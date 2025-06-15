/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file utf8_graph.c
 *
 * @author Juuso Alasuutari
 */
#ifndef __cplusplus

#ifdef _WIN32
# define _CRT_SECURE_NO_WARNINGS
# define WIN32_LEAN_AND_MEAN
#endif

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "utf8_impl.h"

#include "utf8_graph.h"

#if utf8_clang_at_least_version(21)
utf8_diag_clang(ignored "-Wimplicit-void-ptr-cast")
#endif /* clang >= 21 */

static char *
string_copy (char                 *dst,
             char const *const     end,
             char const *restrict  src,
             int        *const     err)
{
	if (!dst)
		return nullptr;

	if (dst >= end) {
		*err = ENOBUFS;
		return nullptr;
	}

	size_t sz = (size_t)(end - dst);
	char *ret = memccpy(dst, src, '\0', sz);
	if (!ret) {
		dst[sz - 1] = '\0';
		*err = E2BIG;
		return nullptr;
	}

	return ret - 1;
}

char *
utf8_graph (char              *dst,
            char const *const  end,
            int        *const  err)
{
	constexpr const uint8_t utf8_range[][4] = {
		#define F(n,m,l,...) [n] = {__VA_ARGS__},
		UTF8_PARSER_DESCRIPTOR(F)
		#undef F
		#undef UTF8_PARSER_DESCRIPTOR
	};

	constexpr const UTF8_PARSER_STATE_MAP(utf8_dst);
	#undef UTF8_PARSER_STATE_MAP

	if (!err || !dst)
		return nullptr;

	if (!end) {
		*err = EFAULT;
		dst = nullptr;
	}

	dst = string_copy(dst, end, "digraph {\n", err);
	if (!dst)
		return nullptr;

	for (size_t i = 0; i < array_size(utf8_range); ++i) {
		unsigned a = utf8_range[i][0];
		unsigned b = utf8_range[i][1];
		unsigned c = utf8_range[i][2];
		unsigned d = utf8_range[i][3];
		if (!c) {
			b += d;
			d = 0;
		} else if (!d)
			c = 0;
		if (!(b + d))
			continue;

		size_t sz = (size_t)(end - dst);
		int n = snprintf(dst, sz, "\tx%02zx [label=\"", i);
		if (n < 0) {
			*err = EIO;
			return nullptr;
		}
		if ((size_t)n >= sz) {
			*err = E2BIG;
			return nullptr;
		}
		dst += n;

		if (b) {
			sz = (size_t)(end - dst);
			n = b == 1
			    ? snprintf(dst, sz, "0x%02x", a)
			    : snprintf(dst, sz, "0x%02x...0x%02x",
			               a, a + b - 1);
			if (n < 0) {
				*err = EIO;
				return nullptr;
			}
			if ((size_t)n >= sz) {
				*err = E2BIG;
				return nullptr;
			}
			dst += n;
		}

		if (d) {
			sz = (size_t)(end - dst);
			n = d == 1
			    ? snprintf(dst, sz, "%s0x%02x", &", "[!b << 1],
			               a + b + c)
			    : snprintf(dst, sz, "%s0x%02x...0x%02x",
			               &", "[!b << 1], a + b + c,
			               a + b + c + d - 1);
			if (n < 0) {
				*err = EIO;
				return nullptr;
			}
			if ((size_t)n >= sz) {
				*err = E2BIG;
				return nullptr;
			}
			dst += n;
		}

		dst = string_copy(dst, end, "\"];\n", err);
		if (!dst)
			return nullptr;
	}

	for (size_t src_idx = 0; src_idx < array_size(utf8_dst); ++src_idx) {
		if (!(utf8_range[src_idx][1] +
		      utf8_range[src_idx][3]))
			continue;
		for (unsigned bits = utf8_dst[src_idx],
		     dst_idx = 0; bits; bits >>= 1U, ++dst_idx) {
			if (!(bits & 1U))
				continue;

			size_t sz = (size_t)(end - dst);
			int n = snprintf(dst, sz, "\tx%02zx -> x%02x;\n",
			                 src_idx, dst_idx);
			if (n < 0) {
				*err = EIO;
				return nullptr;
			}
			if ((size_t)n >= sz) {
				*err = E2BIG;
				return nullptr;
			}
			dst += n;
		}
	}

	return string_copy(dst, end, "}\n", err);
}

#endif /* !__cplusplus */
