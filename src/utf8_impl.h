/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file utf8_impl.h
 *
 * @author Juuso Alasuutari
 */
#ifndef CUTF8_SRC_UTF8_IMPL_H_
#define CUTF8_SRC_UTF8_IMPL_H_

#ifdef DEBUG
# include <stdio.h>
#endif /* DEBUG */

#include "utf8_priv.h"
#include "utf8_util.h"

#undef HAVE_C23_CONSTEXPR
#undef HAVE_C23_NULLPTR

#if __STDC_VERSION__ >= 202000L && !defined __INTELLISENSE__
# if utf8_gcc_at_least_version(13,1) || utf8_clang_at_least_version(19)
#  define HAVE_C23_CONSTEXPR
# endif
# if utf8_gcc_at_least_version(13,1) || utf8_clang_at_least_version(16)
#  define HAVE_C23_NULLPTR
# endif
#endif /* __STDC_VERSION__ >= 202000L && !__INTELLISENSE__ */

#ifndef HAVE_C23_CONSTEXPR
# define constexpr
#endif

#ifndef HAVE_C23_NULLPTR
# include <stddef.h>
# define nullptr NULL
#endif

#undef HAVE_C23_CONSTEXPR
#undef HAVE_C23_NULLPTR

#if utf8_clang_older_than_version(16)
# ifndef typeof
#  define typeof __typeof__
# endif /* !typeof */
#endif /* clang < 16 */

#ifdef _MSC_VER
# define __attribute__(...)
#endif /* _MSC_VER */

#ifndef __has_builtin
# define __has_builtin(...) 0
#endif /* !__has_builtin */

/** @brief Calculate the element count of an array.
 */
#define array_size(x) (sizeof(x) / sizeof((x)[0]))

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

#ifdef DEBUG
# define pr_(fmt, ...) (void)fprintf(stderr, fmt "\n", __VA_ARGS__)
#else /* DEBUG */
# define pr_(...)
#endif /* DEBUG */

#endif /* CUTF8_SRC_UTF8_IMPL_H_ */
