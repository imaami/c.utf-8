/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file utf8_priv.h
 * @brief Including this private header before utf8.h prevents
 *        `UTF8_PARSER_DESCRIPTOR` from being undefined at the
 *        end of utf8.h.
 * @author Juuso Alasuutari
 */
#ifndef CUTF8_SRC_UTF8_PRIV_H_
#define CUTF8_SRC_UTF8_PRIV_H_

/* Cleaning up private macros from the namespace during module
 * use relies on the private macros being undefined inside the
 * guarded section but before the public header is included.
 */
#undef UTF8_CMP_V_
#undef UTF8_GEN_V
#undef UTF8_GEN_V_
#undef UTF8_PARSER_DESCRIPTOR
#undef utf8_clang_at_least_version
#undef utf8_clang_older_than_version
#undef utf8_const_inline
#undef utf8_diag
#undef utf8_diag_apple_clang
#undef utf8_diag_clang
#undef utf8_diag_gcc
#undef utf8_fixed_enum
#undef utf8_force_inline
#undef utf8_gcc_at_least_version
#undef utf8_gcc_older_than_version
#undef utf8_ligma
#undef utf8_ligma_
#undef utf8_nonnull_in
#undef utf8_nonnull_out
#undef utf8_pragma_apple_clang
#undef utf8_pragma_clang
#undef utf8_pragma_gcc
#undef utf8_pragma_msvc
#undef utf8_v_clang
#undef utf8_v_gcc

#include "utf8.h"

#endif /* CUTF8_SRC_UTF8_PRIV_H_ */
