/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file utf8_compat.h
 *
 * @author Juuso Alasuutari
 */
#ifndef CUTF8_SRC_UTF8_COMPAT_H_
#define CUTF8_SRC_UTF8_COMPAT_H_

#include "utf8_compiler.h"
#include "utf8_pragma.h"

#ifdef _WIN32
# define _CRT_SECURE_NO_WARNINGS
# define WIN32_LEAN_AND_MEAN
#endif

#ifndef __cplusplus

# undef HAVE_C23_BOOL
# undef HAVE_C23_CONSTEXPR
# undef HAVE_C23_NULLPTR

# if __STDC_VERSION__ >= 202000L && !defined __INTELLISENSE__
#  if utf8_gcc_at_least_version(13,1) || utf8_clang_at_least_version(15)
#   define HAVE_C23_BOOL
#  endif
#  if utf8_gcc_at_least_version(13,1) || utf8_clang_at_least_version(19)
#   define HAVE_C23_CONSTEXPR
#  endif
#  if utf8_gcc_at_least_version(13,1) || utf8_clang_at_least_version(16)
#   define HAVE_C23_NULLPTR
#  endif
# endif /* __STDC_VERSION__ >= 202000L && !__INTELLISENSE__ */

# ifndef HAVE_C23_BOOL
#  include <stdbool.h>
# endif

# ifndef HAVE_C23_CONSTEXPR
#  define constexpr
# endif

# ifndef HAVE_C23_NULLPTR
#  include <stddef.h>
#  define nullptr NULL
# endif

# undef HAVE_C23_BOOL
# undef HAVE_C23_CONSTEXPR
# undef HAVE_C23_NULLPTR

# if utf8_clang_older_than_version(8)   \
  || utf8_gcc_older_than_version(13,1)  \
  || defined(__INTELLISENSE__)
#  define utf8_fixed_enum(name, T) enum name
# elif utf8_clang_older_than_version(18)
#  define utf8_fixed_enum(name, T)      \
    utf8_diag_clang(push)               \
    utf8_diag_clang(ignored             \
      "-Wfixed-enum-extension")         \
    enum name : T                       \
    utf8_diag_clang(pop)
# else
#  define utf8_fixed_enum(name, T) enum name : T
# endif /* clang < 8 || gcc < 13.1 || __INTELLISENSE__ */

# if utf8_clang_older_than_version(16)
#  ifndef typeof
#   define typeof __typeof__
#  endif /* !typeof */
# endif /* clang < 16 */

/* Old Clang versions don't know new Doxygen commands */
# if utf8_clang_older_than_version(10)
utf8_diag_clang(ignored "-Wdocumentation-unknown-command")
# endif /* clang < 10 */

/* Complains about C99 syntax */
# if utf8_clang_at_least_version(14)
utf8_diag_clang(ignored "-Wdeclaration-after-statement")
# endif /* clang >= 14 */

/* Mostly false positives in C */
# if utf8_clang_at_least_version(16)
utf8_diag_clang(ignored "-Wunsafe-buffer-usage")
# endif /* clang >= 16 */

/* These whine about C23 when compiling C23 */
# if __STDC_VERSION__ >= 202000L
#  if utf8_clang_at_least_version(16) && utf8_clang_older_than_version(18)
utf8_diag_clang(ignored "-Wpre-c2x-compat")
#  endif /* 16 <= clang < 18 */
#  if utf8_clang_at_least_version(18)
utf8_diag_clang(ignored "-Wpre-c23-compat")
#  endif /* clang >= 18 */
#  if utf8_clang_at_least_version(19)
utf8_diag_clang(ignored "-Wc++98-compat")
#  endif /* clang >= 19 */
# endif /* __STDC_VERSION__ >= 202000L */

/* Complains about C11 when compiling C11 */
# if utf8_clang_at_least_version(19)
utf8_diag_clang(ignored "-Wpre-c11-compat")
# endif /* clang >= 19 */

#endif /* !__cplusplus */

#endif /* CUTF8_SRC_UTF8_COMPAT_H_ */
