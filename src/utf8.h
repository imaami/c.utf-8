/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file utf8.h
 *
 * @author Juuso Alasuutari
 */
#ifndef CUTF8_SRC_UTF8_H_
#define CUTF8_SRC_UTF8_H_

#include <stddef.h>
#include <stdint.h>

#include "utf8_compat.h"
#include "utf8_util.h"

/** @brief UTF-8 parser states.
 *
 * The parser is implemented as a state machine with 15 operational
 * states and an initial state. The states are encoded as bit flags
 * in the byte value lookup table to track multi-byte sequences. In
 * other situations, state enumeration values are used directly.
 *
 * State enumerations 0 through 7 represent leading bytes, 8 through
 * 14 represent continuation bytes, and enumeration 15 is the initial
 * state. State enumerations 0, 14, and 15 can be followed by any one
 * of the 8 leading byte state enumerations. The remaining 13 states
 * (1 through 13) can be followed by exactly one state.
 *
 * Due to explicit restrictions in the UTF-8 encoding scheme, not all
 * leading byte values can be followed by any continuation byte value.
 * This is reflected in the presence of overlapping state transitions
 * in the table below.
 *
 * There are three distinct types of restrictions at play:
 *
 * 1. Overlong encodings are avoided by rejecting the leading bytes
 *    0xc0-0xc1 and 0xf5-0xff, and limiting the range of the first
 *    continuation byte following the leading bytes 0xe0 and 0xf0.
 *
 * 2. Surrogate code points are avoided by restricting the range of
 *    the first continuation byte following the leading byte 0xed.
 *
 * 3. Code points above the Unicode maximum of 0x10ffff are avoided
 *    by lowering the maximum value of the first continuation byte
 *    following the leading byte 0xf4.
 */
#define UTF8_PARSER_DESCRIPTOR(F)                                                                      \
        /* ╭───────────────────────────────enumeration                                              */ \
        /* │      ╭────────────────────────label                                                    */ \
        /* │      │    ╭───────────────────size                                                     */ \
        /* │      │    │     ╭─────────────start                                                    */ \
        /* │      │    │     │    ╭────────run                                                      */ \
        /* │      │    │     │    │  ╭─────skip                                                     */ \
        /* │      │    │     │    │  │  ╭──run                                                      */ \
        F( 0,  asc,    1, 0x01, 127, 0, 0) /* ASCII - never followed by continuation byte 0x80-0xbf */ \
        F( 1,  lb2,    2, 0xc2,  30, 0, 0) /* start of 2-byte sequence, any continuation may follow */ \
        F( 2,  lb3_e0, 3, 0xe0,   1, 0, 0) /* start of 3-byte sequence, next byte must be 0xa0-0xbf */ \
        F( 3,  lb3,    3, 0xe1,  12, 1, 2) /* start of 3-byte sequence, any continuation may follow */ \
        F( 4,  lb3_ed, 3, 0xed,   1, 0, 0) /* start of 3-byte sequence, next byte must be 0x80-0x9f */ \
        F( 5,  lb4_f0, 4, 0xf0,   1, 0, 0) /* start of 4-byte sequence, next byte must be 0x90-0xbf */ \
        F( 6,  lb4,    4, 0xf1,   3, 0, 0) /* start of 4-byte sequence, any continuation may follow */ \
        F( 7,  lb4_f4, 4, 0xf4,   1, 0, 0) /* start of 4-byte sequence, next byte must be 0x80-0x8f */ \
        F( 8,  cb3_f4, 3, 0x80,  16, 0, 0) /* 3rd-to-last continuation, follows 0xf4                */ \
        F( 9,  cb3,    3, 0x80,  64, 0, 0) /* 3rd-to-last continuation, follows 0xf1-0xf3           */ \
        F(10,  cb3_f0, 3, 0x90,  48, 0, 0) /* 3rd-to-last continuation, follows 0xf0                */ \
        F(11,  cb2_ed, 2, 0x80,  32, 0, 0) /* 2nd-to-last continuation, follows 0xed                */ \
        F(12,  cb2,    2, 0x80,  64, 0, 0) /* 2nd-to-last continuation, follows 0xe1-0xec,0xee-0xef */ \
        F(13,  cb2_e0, 2, 0xa0,  32, 0, 0) /* 2nd-to-last continuation, follows 0xe0                */ \
        F(14,  cb1,    1, 0x80,  64, 0, 0) /* last continuation, common to all multi-byte sequences */ \
        F(15,  ini,    0,    0,   0, 0 ,0) /* initial state, not used as a flag in the lookup table */

/**
 * @brief UTF-8 parser state enumeration.
 */
utf8_fixed_enum(utf8_st8, uint8_t) {
	#define F(n,m,...) utf8_##m = n,
	UTF8_PARSER_DESCRIPTOR(F)
	#undef F
};

/**
 * @brief UTF-8 parser state bit flags.
 * @param label State label (2nd column in the descriptor).
 * @return Bit flag value for the state.
 */
#define utf8_bit(label) (uint16_t)(1U << utf8_##label)

/**
 * @brief Define a lookup table describing allowed state
 *        transitions from each state, using state enums
 *        for keys and sets of state flags for values.
 *
 * @note The baked-in type declaration is `uint16_t[16]`
 *       with no type qualifiers; prepend more as needed
 *       (e.g. `const`, `static`, etc.).
 *
 * @note This macro is not defined (i.e. gets undefined)
 *       when `utf8.h` is included as a public interface.
 *
 * @param sym Symbol name for the lookup table.
 */
#define UTF8_PARSER_STATE_MAP(sym) uint16_t sym[16] = { \
        [utf8_asc   ] = utf8_bit(asc)                   \
                      | utf8_bit(lb2)                   \
                      | utf8_bit(lb3_e0)                \
                      | utf8_bit(lb3)                   \
                      | utf8_bit(lb3_ed)                \
                      | utf8_bit(lb4_f0)                \
                      | utf8_bit(lb4)                   \
                      | utf8_bit(lb4_f4),               \
        [utf8_lb2   ] = utf8_bit(cb1),                  \
        [utf8_lb3_e0] = utf8_bit(cb2_e0),               \
        [utf8_lb3   ] = utf8_bit(cb2),                  \
        [utf8_lb3_ed] = utf8_bit(cb2_ed),               \
        [utf8_lb4_f0] = utf8_bit(cb3_f0),               \
        [utf8_lb4   ] = utf8_bit(cb3),                  \
        [utf8_lb4_f4] = utf8_bit(cb3_f4),               \
        [utf8_cb3_f4] = utf8_bit(cb2),                  \
        [utf8_cb3   ] = utf8_bit(cb2),                  \
        [utf8_cb3_f0] = utf8_bit(cb2),                  \
        [utf8_cb2_ed] = utf8_bit(cb1),                  \
        [utf8_cb2   ] = utf8_bit(cb1),                  \
        [utf8_cb2_e0] = utf8_bit(cb1),                  \
        [utf8_cb1   ] = utf8_bit(asc)                   \
                      | utf8_bit(lb2)                   \
                      | utf8_bit(lb3_e0)                \
                      | utf8_bit(lb3)                   \
                      | utf8_bit(lb3_ed)                \
                      | utf8_bit(lb4_f0)                \
                      | utf8_bit(lb4)                   \
                      | utf8_bit(lb4_f4),               \
        [utf8_ini   ] = utf8_bit(asc)                   \
                      | utf8_bit(lb2)                   \
                      | utf8_bit(lb3_e0)                \
                      | utf8_bit(lb3)                   \
                      | utf8_bit(lb3_ed)                \
                      | utf8_bit(lb4_f0)                \
                      | utf8_bit(lb4)                   \
                      | utf8_bit(lb4_f4),               \
}

/**
 * @brief UTF-8 parser object.
 */
struct utf8 {
	uint16_t state;
	uint8_t  cache[5];
	uint8_t  error;
};

/**
 * @brief UTF-8 parser object RAII initializer.
 * @return A UTF-8 parser object by value.
 */
static utf8_const_inline
struct utf8 utf8 (void)
{
	return (struct utf8) {
		.state = utf8_bit(ini),
		.cache = {0,0,0,0,0},
		.error = 0,
	};
}

/**
 * @brief Clear a UTF-8 parser object.
 * @param u8p Pointer to parser object.
 */
utf8_nonnull_in
static utf8_force_inline void
utf8_reset (struct utf8 *const u8p)
{
	*u8p = utf8();
}

/**
 * @brief Parse the next UTF-8 code point from a byte buffer.
 *
 * Consumes up to 4 bytes of UTF-8 encoded input until a code
 * point is assembled or an error occurs. Returns the address
 * of the next byte after the consumed bytes.
 *
 * On valid input `u8p->error` is assigned 0 and the address
 * of the next byte after the parsed code point is returned.
 * The parsed code point's value (as UTF-8) can be retrieved
 * by calling @ref utf8_result() and its byte size with
 * @ref utf8_size().
 *
 * On invalid input `u8p->error` is assigned `EILSEQ` and the
 * address of the first invalid byte is returned (implying that
 * invalid input is never consumed).
 *
 * A null byte is treated like any ASCII value, i.e. valid if
 * @ref utf8_expects_leading_byte() returns `true` before the
 * null byte is encountered. More specifically:
 * - in the initial parser state (@ref utf8_ini), or
 * - after consuming an ASCII byte (@ref utf8_asc), or
 * - after consuming the last continuation byte of a multi-byte
 *   sequence (@ref utf8_cb1).
 * In this situation the null byte is consumed, `u8p->error` is
 * assigned 0, and the address of the next byte after the null
 * byte is returned.
 *
 * If a null byte is encountered in any other situation it is
 * an illegal byte inside a multi-byte sequence. In this case
 * `u8p->error` is assigned `EILSEQ` and the address of the
 * null byte is returned (i.e. the null byte is not consumed).
 *
 * If the content of `u8p->state` is corrupt, `u8p->error` is
 * assigned `ENOTRECOVERABLE` and the original value of `ptr`
 * is returned. This is a serious error that should not occur
 * under normal circumstances. If this happens there is a bug
 * in the caller code or a memory corruption issue. No parsing
 * is performed in this case.
 *
 * @param u8p A pointer to the UTF-8 parser object. Must not
 *            be null.
 * @param ptr A pointer to the input buffer. Must not be null.
 * @return A pointer to the byte immediately after the parsed
 *         code point, or a pointer to the first invalid byte
 *         if malformed input is encountered.
 */
utf8_nonnull_out
extern uint8_t const *
utf8_parse_next_code_point (struct utf8   *u8p,
                            uint8_t const *ptr) utf8_nonnull_in;

/**
 * @brief Get the result of the last UTF-8 code point parsing operation.
 *
 * The result is a pointer to the UTF-8 encoded code point. The result
 * is only valid if the last parsing operation was successful, i.e. if
 * `u8p->error` is 0. Otherwise the result may be incomplete.
 *
 * @param u8p Pointer to the UTF-8 parser object.
 * @return A pointer to the UTF-8 encoded code point.
 */
utf8_nonnull_in utf8_nonnull_out
static utf8_force_inline char const *
utf8_result (struct utf8 const *const u8p)
{
	return (char const *)&u8p->cache[1];
}

/**
 * @brief Get the intended size of the last UTF-8 code point.
 *
 * The size is the number of bytes that the last UTF-8 code point
 * was intended to occupy in the input buffer. The size is only
 * equal to the actual number of bytes consumed if the last parsing
 * operation was successful, i.e. if `u8p->error` is 0. Otherwise
 * the size is larger than the actual number of bytes successfully
 * consumed.
 *
 * @param u8p Pointer to the UTF-8 parser object.
 * @return The intended size of the last UTF-8 code point.
 */
utf8_nonnull_in
static utf8_force_inline size_t
utf8_size (struct utf8 const *const u8p)
{
	return u8p->cache[0];
}

/**
 * @brief Check if the parser expects a leading byte or an ASCII byte.
 *
 * Succeeds if the parser:
 *
 * - is in the initial parser state (@ref utf8_ini), or
 * - has successfully consumed an ASCII byte (@ref utf8_asc), or
 * - has successfully consumed the last continuation byte of a multi-byte
 *   sequence (@ref utf8_cb1).
 *
 * @param u8p Pointer to the UTF-8 parser object.
 * @return `true` if the parser expects a leading byte or an ASCII byte.
 */
utf8_nonnull_in
static utf8_const_inline bool
utf8_expects_leading_byte (struct utf8 const *const u8p)
{
	return u8p->state & (utf8_bit(asc) | utf8_bit(cb1) | utf8_bit(ini));
}

/* Private macro cleanup logic depends on this include being here,
 * right above the closing endif of the header guard. DO NOT MOVE.
 */
#include "utf8_priv.h"

#endif /* CUTF8_SRC_UTF8_H_ */
