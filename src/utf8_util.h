/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file utf8_util.h
 *
 * @author Juuso Alasuutari
 */
#ifndef CUTF8_SRC_UTF8_UTIL_H_
#define CUTF8_SRC_UTF8_UTIL_H_

#include "utf8_compat.h"

#ifndef _MSC_VER

/** @brief Instruct the compiler to always inline a function.
 */
# define utf8_force_inline __attribute__((always_inline)) inline

/** @brief Instruct the compiler to always inline a function
 *         and to assume its return value is determined only
 *         by its arguments.
 */
# define utf8_const_inline __attribute__((always_inline,const)) inline

/** @brief Assume that input pointer arguments are not null.
 */
# define utf8_nonnull_in __attribute__((nonnull))

/** @brief Assume that the return value of a function is not null.
 */
# define utf8_nonnull_out __attribute__((returns_nonnull))

#else /* _MSC_VER */
# define utf8_force_inline __forceinline
# define utf8_const_inline __forceinline
# define utf8_nonnull_in
# define utf8_nonnull_out
#endif /* _MSC_VER */

/** @brief Calculate the element count of an array.
 */
#define utf8_array_size(x) (sizeof(x) / sizeof((x)[0]))

#endif /* CUTF8_SRC_UTF8_UTIL_H_ */
