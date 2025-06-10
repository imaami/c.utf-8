/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file utf8_pragma.h
 * @brief Cross-compiler pragma wrapper macros
 * @author Juuso Alasuutari
 */
#ifndef CUTF8_SRC_UTF8_PRAGMA_H_
#define CUTF8_SRC_UTF8_PRAGMA_H_

#define utf8_diag_apple_clang(...)      utf8_pragma_apple_clang(diagnostic __VA_ARGS__)
#define utf8_diag_clang(...)            utf8_pragma_clang(diagnostic __VA_ARGS__)
#define utf8_diag_gcc(...)              utf8_pragma_gcc(diagnostic __VA_ARGS__)
#define utf8_diag(...)                  utf8_diag_clang(__VA_ARGS__) utf8_diag_gcc(__VA_ARGS__)

#ifdef __apple_build_version__
# define utf8_pragma_apple_clang        utf8_pragma_clang
#else
# define utf8_pragma_apple_clang(...)
#endif

#ifdef __clang__
# define utf8_pragma_clang(...) utf8_ligma(clang __VA_ARGS__)
#else
# define utf8_pragma_clang(...)
#endif

#if !defined __clang__ && defined __GNUC__
# define utf8_pragma_gcc(...)   utf8_ligma(GCC __VA_ARGS__)
#else
# define utf8_pragma_gcc(...)
#endif

#ifdef _MSC_VER
# define utf8_pragma_msvc(...)  utf8_ligma(__VA_ARGS__)
#else
# define utf8_pragma_msvc(...)
#endif

#define utf8_ligma_(...)        # __VA_ARGS__
#define utf8_ligma(...)         _Pragma(utf8_ligma_(__VA_ARGS__))

#endif /* CUTF8_SRC_UTF8_PRAGMA_H_ */
