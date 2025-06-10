/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file utf8.c
 *
 * @author Juuso Alasuutari
 */
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#ifdef DEBUG
# include <stdio.h>
#endif

#include "utf8_priv.h"
#include "utf8_util.h"

#ifdef _MSC_VER
# define __attribute__(...)
#endif /* _MSC_VER */

#ifndef __has_builtin
# define __has_builtin(...) 0
#endif /* !__has_builtin */

/** @brief Assume that a value is within a certain range.
 */
#if __has_builtin(__builtin_assume)
# define assume_value_bits(x, mask) \
        __builtin_assume((x) == ((x) & (typeof(x))(mask)))
#else
# define assume_value_bits(x, mask)
#endif /* __builtin_assume */

/**
 * @brief Check if an integer value is negative without getting warning
 *        spam if the type of the value is unsigned.
 *
 * If `x` is an unsigned integer type the macro expands to what should
 * be a compile-time constant expression 0, otherwise to `x < 0`. In
 * the latter case `x` is assumed to be a signed integer.
 *
 * @note The `_Generic` expression used to implement this macro only has
 *       explicit cases for the old school C unsigned integer types and
 *       a default case for everything else.
 *
 * @param x An integral-typed value.
 */
#define is_negative(x) (_Generic((x), \
        default:(x), unsigned char:1, \
        unsigned short:1, unsigned:1, \
        typeof(1UL):1,typeof(1ULL):1) < (typeof(x))0)

#ifndef DEBUG
# define pr_(...)
#else /* DEBUG */
# define pr_(fmt, ...) (void)fprintf(stderr, fmt "\n", __VA_ARGS__)

static utf8_const_inline char const *
utf8_st8_name (enum utf8_st8 st8)
{
	constexpr static char const name[][8] = {
		#define F(n,m,...) [n] = #m,
		UTF8_PARSER_DESCRIPTOR(F)
		#undef F
	};

	if (!is_negative(st8) && st8 < utf8_array_size(name))
		return name[st8];

	return nullptr;
}
#endif /* DEBUG */

#define X1(x)   x
#define X2(x)   x,x
#define X3(x)   x,x,x
#define X8(x)   x,x,x,x,x,x,x,x
#define X11(x)  X8(x),x,x,x
#define X12(x)  X11(x),x
#define X16(x)  X8(x),X8(x)
#define X30(x)  X16(x),X12(x),x,x
#define X32(x)  X16(x),X16(x)
#define X128(x) X32(x),X32(x),X32(x),X32(x)
#define X(n, x) X##n(x)

constexpr static const uint16_t utf8_lut[] = {
	X(128,utf8_bit(asc)  ), /* 0x00-0x7f */

	/* 0x80-0x8f */
	X(16,utf8_bit(cb3)   \
	    |utf8_bit(cb3_f4)\
	    |utf8_bit(cb2)   \
	    |utf8_bit(cb2_ed)\
	    |utf8_bit(cb1)   ),

	/* 0x90-0x9f */
	X(16,utf8_bit(cb3)   \
	    |utf8_bit(cb3_f0)\
	    |utf8_bit(cb2)   \
	    |utf8_bit(cb2_ed)\
	    |utf8_bit(cb1)   ),

	/* 0xa0-0xbf */
	X(32,utf8_bit(cb3)   \
	    |utf8_bit(cb3_f0)\
	    |utf8_bit(cb2)   \
	    |utf8_bit(cb2_e0)\
	    |utf8_bit(cb1)   ),

	X( 2,0               ), /* 0xc0-0xc1 */
	X(30,utf8_bit(lb2)   ), /* 0xc2-0xdf */
	X( 1,utf8_bit(lb3_e0)), /* 0xe0      */
	X(12,utf8_bit(lb3)   ), /* 0xe1-0xec */
	X( 1,utf8_bit(lb3_ed)), /* 0xed      */
	X( 2,utf8_bit(lb3)   ), /* 0xee-0xef */
	X( 1,utf8_bit(lb4_f0)), /* 0xf0      */
	X( 3,utf8_bit(lb4)   ), /* 0xf1-0xf3 */
	X( 1,utf8_bit(lb4_f4)), /* 0xf4      */
	X(11,0               ), /* 0xf5-0xff */
};

#undef X
#undef X128
#undef X32
#undef X30
#undef X16
#undef X12
#undef X11
#undef X8
#undef X3
#undef X2
#undef X1

constexpr static const UTF8_PARSER_STATE_MAP(utf8_dst);

/**
 * @brief Convert a parser state from a bit flag representation
 *        to the corresponding parser state enumeration.
 *
 * If the return value is not negative, it is guaranteed to be
 * the `int` conversion of a valid @ref utf8_st8 enumeration.
 *
 * @param bit The flag to convert. Must be a non-zero power of 2.
 * @return An enumeration if `bit` is a valid parser state flag,
 *         otherwise -1.
 */
static utf8_const_inline int
utf8_state_from_bit (uint16_t bit)
{
	return bit && !(bit & (bit - 1U))
		? __builtin_ctz(bit) : -1;
}

/**
 * @brief Get the parser's current state as a state enumeration.
 *
 * Note that this is not a pure getter, as @ref utf8::error is
 * clobbered on failure. Take precautions to save it if needed.
 *
 * @param u8p The UTF-8 parser. The error field is set on failure
 *            without regard to its current state.
 * @param st8 Where to store the state enumeration on success.
 *            Not modified on failure. Must not be `nullptr`.
 * @return `true` on success. On failure sets @ref ut8::error to
 *         `ENOTRECOVERABLE` and returns `false`.
 */
utf8_nonnull_in
static utf8_force_inline bool
utf8_get_state (struct utf8 *const   u8p,
                enum utf8_st8 *const st8)
{
	int e = utf8_state_from_bit(u8p->state);
	if (e < 0) {
		u8p->error = ENOTRECOVERABLE;
		return false;
	}

	assume_value_bits(e, 0xf);

	*st8 = (enum utf8_st8)e;
	return true;
}

/**
 * @brief Get a mask of state flags comprising the states allowed after `st8`.
 *
 * @param st8 A parser state enumeration.
 * @return Mask of state flags representing the allowed states, or 0 if `st8`
 *         is not a valid state enumeration.
 */
static utf8_const_inline uint16_t
utf8_get_allowed_next_states (enum utf8_st8 st8)
{
	if (!is_negative(st8) && st8 <= utf8_ini)
		return utf8_dst[st8];
	assert(!is_negative(st8) && st8 <= utf8_ini);
	return 0;
}

/**
 * @brief Check if the UTF-8 parser has finished processing a character.
 *
 * This function checks the state of the UTF-8 parser to determine if it
 * has successfully parsed a complete character. The parser is considered
 * done if the current state represents an ASCII byte or a continuation byte.
 *
 * @param u8p A pointer to the UTF-8 parser.
 * @return `true` if the parser has finished processing a character,
 *         otherwise `false`.
 */
utf8_nonnull_in
static utf8_const_inline bool
utf8_done (struct utf8 const *const u8p)
{
	return u8p->state & (utf8_bit(asc) | utf8_bit(cb1));
}

/**
 * @brief Check if a parser state enumeration represents a UTF-8 leading
 *        byte or an ASCII byte.
 *
 * @param st8 The state enumeration to check.
 * @return `true` if `st8` is a leading byte or ASCII, otherwise `false`.
 */
static utf8_const_inline bool
utf8_st8_is_leading_byte (enum utf8_st8 st8)
{
	return st8 < (enum utf8_st8)8;
}

/**
 * @brief Write a byte to a specific position inside the parser cache.
 *
 * This function writes a byte into the parser cache at the position
 * determined by the current parser state (as indicated by the `st8`
 * parameter).
 *
 * If the state represents a leading byte or an ASCII byte, the cache
 * is reset, the length of the expected UTF-8 sequence is written to
 * the length field, and the byte value is written to the first buffer
 * position.
 *
 * If the state represents a continuation byte, the destination position
 * is calculated from the state in the context of the length field value.
 * (This is necessary because some parser states are shared between state
 * transition paths; for example, all multi-byte sequences terminate in
 * the @ref utf8_cb1 state, so determining the correct buffer position
 * requires knowing the expected sequence length as well.)
 *
 * @param u8p The UTF-8 parser to update.
 * @param st8 The current parser state expressed as a state enumeration.
 * @param byte The byte to push into the cache.
 */
__attribute__((nonnull(1)))
static utf8_force_inline void
utf8_push_to_cache (struct utf8 *const u8p,
                    enum utf8_st8      st8,
                    uint8_t            byte)
{
	constexpr static const uint8_t utf8_len[16] = {
		#define F(n,m,l,...) [n] = l,
		UTF8_PARSER_DESCRIPTOR(F)
		#undef F
	};

	uint8_t len = utf8_len[st8];
#ifdef DEBUG
	uint8_t k = 0;
#endif /* DEBUG */

	if (utf8_st8_is_leading_byte(st8)) {
		// Leading byte or ASCII
		u8p->cache[0] = len;
		__builtin_memset(&u8p->cache[1], 0, sizeof u8p->cache - 1U);
		pr_(" [\033[1;3%" PRIu8 "m%02" PRIx8 "\033[m]", len, len);
#ifdef DEBUG
		++k;
#endif /* DEBUG */
	}

	uint8_t pos = u8p->cache[0] + 1U - len;
	u8p->cache[pos] = byte;

#ifdef DEBUG
	for (;k < pos; ++k) {
		pr_(" [%02" PRIx8 "]", u8p->cache[k]);
	}
	pr_(" [\033[1;3%" PRIu8 "m%02" PRIx8 "\033[m]", pos, u8p->cache[k]);
	for (; ++k < sizeof u8p->cache;) {
		pr_(" [%02" PRIx8 "]", u8p->cache[k]);
	}
#endif /* DEBUG */
}

#undef UTF8_PARSER_DESCRIPTOR

/**
 * @brief Update the UTF-8 parser state and cache based on an input byte.
 *
 * This function validates the input byte against the state transition rules
 * which apply to the current parser state. If the byte is a valid next byte,
 * the parser state is updated accordingly and the byte is pushed into the
 * parser's byte cache.
 *
 * @param u8p The UTF-8 parser to update. The error field is set on failure.
 * @param st8 On input a pointer to the current parser state expressed as a
 *            state enumeration. On output the updated state enumeration if
 *            the byte is valid, otherwise unchanged. Must not be `nullptr`.
 * @param byte The byte to process and push into the parser cache.
 * @return `true` if the state update was successful. If the update fails,
 *         sets @ref utf8::error to `EILSEQ` and returns `false`.
 */
__attribute__((nonnull(1,2)))
static utf8_force_inline bool
utf8_set_state (struct utf8 *const   u8p,
                enum utf8_st8 *const st8,
                uint8_t              byte)
{
	uint16_t next_bit = utf8_lut[byte] &
	                    utf8_get_allowed_next_states(*st8);

	int e = utf8_state_from_bit(next_bit);
	if (e < 0) {
		u8p->error = EILSEQ;
		return false;
	}

	assume_value_bits(e, 0xf);

	u8p->state = next_bit;
	utf8_push_to_cache(u8p, (enum utf8_st8)e, byte);

	pr_("  %6s -> %-6s", utf8_st8_name(*st8),
	                     utf8_st8_name((enum utf8_st8)e));

	*st8 = (enum utf8_st8)e;
	return true;
}

utf8_nonnull_in utf8_nonnull_out
uint8_t const *
utf8_parse_next_code_point (struct utf8 *const  u8p,
                            uint8_t const      *ptr)
{
	enum utf8_st8 st8 = utf8_ini;

	if (utf8_get_state(u8p, &st8)) {
		while (utf8_set_state(u8p, &st8, *ptr)) {
			++ptr;
			if (utf8_done(u8p)) {
				u8p->error = 0;
				break;
			}
		}
	}

	return ptr;
}
