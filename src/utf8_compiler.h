/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file utf8_compiler.h
 * @brief Compiler identification convenience macros
 * @author Juuso Alasuutari
 */
#ifndef CUTF8_SRC_UTF8_COMPILER_H_
#define CUTF8_SRC_UTF8_COMPILER_H_

#ifdef __clang_major__
# define utf8_v_clang() UTF8_GEN_V(__clang_major__,    \
                                   __clang_minor__,    \
                                   __clang_patchlevel__)
# define utf8_clang_equal_to_version(...) UTF8_CMP_V_(==,clang,__VA_ARGS__)
# define utf8_clang_at_least_version(...) UTF8_CMP_V_(>=,clang,__VA_ARGS__)
# define utf8_clang_older_than_version(...) UTF8_CMP_V_(<,clang,__VA_ARGS__)
# define utf8_clang_not_version(...) UTF8_CMP_V_(!=,clang,__VA_ARGS__)
#else
# define utf8_clang_equal_to_version(...) 0
# define utf8_clang_at_least_version(...) 0
# define utf8_clang_older_than_version(...) 0
# define utf8_clang_not_version(...) 0
#endif

#if !defined __clang_major__ && defined __GNUC__
# define utf8_v_gcc() UTF8_GEN_V(__GNUC__,          \
                                 __GNUC_MINOR__,    \
                                 __GNUC_PATCHLEVEL__)
# define utf8_gcc_equal_to_version(...) UTF8_CMP_V_(==,gcc,__VA_ARGS__)
# define utf8_gcc_at_least_version(...) UTF8_CMP_V_(>=,gcc,__VA_ARGS__)
# define utf8_gcc_older_than_version(...) UTF8_CMP_V_(<,gcc,__VA_ARGS__)
# define utf8_gcc_not_version(...) UTF8_CMP_V_(!=,gcc,__VA_ARGS__)
#else
# define utf8_gcc_equal_to_version(...) 0
# define utf8_gcc_at_least_version(...) 0
# define utf8_gcc_older_than_version(...) 0
# define utf8_gcc_not_version(...) 0
#endif

#if defined utf8_v_clang || defined utf8_v_gcc
# define UTF8_CMP_V_(op, id, ...) (utf8_v_##id() op UTF8_GEN_V(__VA_ARGS__))
# define UTF8_GEN_V(...) UTF8_GEN_V_(__VA_ARGS__+0,0,0,)
# define UTF8_GEN_V_(a, b, c, ...) \
        ( (((a) & 0x3ffU) << 21U)  \
        | (((b) & 0x3ffU) << 11U)  \
        |  ((c) & 0x7ffU)          )
#endif

#endif /* CUTF8_SRC_UTF8_COMPILER_H_ */
