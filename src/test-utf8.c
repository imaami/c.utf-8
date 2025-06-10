/* SPDX-License-Identifier: LGPL-3.0-or-later */
/** @file test-utf8.c
 *
 * @author Juuso Alasuutari
 */
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define PROGNAME "test-utf8"
#define SYNOPSIS "[OPTION]... [--] [STRING]..."
#define PURPOSE  "Inspect UTF-8 encoded strings"

#define OPTIONS(X)                              \
	X(boolean, help, 'h', "help",           \
	  "print this help text and exit")      \
	                                        \
	X(boolean, bytes, 'b', "bytes",         \
	  "count bytes inside valid UTF-8")     \
	                                        \
	X(boolean, chars, 'c', "chars",         \
	  "count valid unicode characters")     \
	                                        \
	X(boolean, join, 'j', "join",           \
	  "treat arguments as one string")      \
	                                        \
	X(boolean, print, 'p', "print",         \
	  "print strings even when counting")   \
	                                        \
	X(boolean, skip, 's', "skip",           \
	  "skip invalid UTF-8, don't replace")  \
	                                        \
	X(boolean, quiet, 'q', "quiet",         \
	  "report invalid UTF-8 via exit code") \
	                                        \
	X(boolean, graph, 'g', "graph",         \
	  "dump UTF-8 state machine graph (DOT)")

#define DETAILS \
 "The default behaviour without option arguments is to\n" \
 "print input strings on different lines, substituting\n" \
 "the replacement symbol U+FFFD for invalid sequences."

#include "letopt/src/letopt.h"

#undef DETAILS
#undef OPTIONS
#undef PURPOSE
#undef SYNOPSIS
#undef PROGNAME

#include "utf8_priv.h"

#include "utf8_graph.h"

static utf8_const_inline size_t
saturated_add_uz (size_t a,
                  size_t b)
{
	if ((a += b) < b)
		a = SIZE_MAX;
	return a;
}

static struct uz2 {
	size_t max;
	size_t sum;
} arg_sizes (struct letopt const *opt)
{
	struct uz2 ret = {0};
	for (int i = 0; i < letopt_nargs(opt); ++i) {
		size_t n = strlen(letopt_arg(opt, i));
		ret.sum = saturated_add_uz(ret.sum, n);
		if (n > ret.max)
			ret.max = n;
	}
	return ret;
}

static int
arg_conflict (struct letopt *opt);

static int
dump_utf8_graph (struct letopt *opt);

int
main (int    c,
      char **v)
{
	struct letopt opt = letopt_init(c, v);

	if (arg_conflict(&opt))
		return letopt_fini(&opt);

	if (opt.m_graph)
		return dump_utf8_graph(&opt);

	if (letopt_nargs(&opt) < 1 || opt.m_help)
		letopt_helpful_exit(&opt);

	struct uz2 sz = arg_sizes(&opt);
	size_t buf_sz = opt.m_join ? sz.sum : sz.max;
	buf_sz = saturated_add_uz(
		saturated_add_uz(buf_sz, 2U),
		saturated_add_uz(buf_sz, buf_sz));

	if (buf_sz == SIZE_MAX) {
		(void)fprintf(stderr, "error: %s\n", strerror(EOVERFLOW));
		(void)letopt_fini(&opt);
		return EXIT_FAILURE;
	}

	char *buf = malloc(buf_sz);
	if (!buf) {
		(void)fprintf(stderr, "error: malloc(%zu): %s\n", buf_sz,
		              strerror(errno));
		(void)letopt_fini(&opt);
		return EXIT_FAILURE;
	}

	const bool count = opt.m_bytes || opt.m_chars;
	const bool print = !opt.m_quiet && (opt.m_print || !count);
	size_t n_chars = 0, n_bytes = 0;
	struct utf8 u8p = utf8();
	char *out = buf;

	for (int i = 0; i < letopt_nargs(&opt); ++i) {
		uint8_t const *p = (uint8_t const *)letopt_arg(&opt, i);
		for (uint8_t const *q = p; *q; q = p) {
			p = utf8_parse_next_code_point(&u8p, q);
			if (u8p.error) {
				if (print &&
				    !opt.m_skip && (*p || !opt.m_join)) {
					*out++ = (char)(unsigned char)0xefU;
					*out++ = (char)(unsigned char)0xbfU;
					*out++ = (char)(unsigned char)0xbdU;
				}
				if (!*p)
					break;
				if (p == q && utf8_expects_leading_byte(&u8p))
					++p;
				utf8_reset(&u8p);
				continue;
			}

			if (opt.m_bytes)
				n_bytes += utf8_size(&u8p);
			if (opt.m_chars)
				n_chars += 1U;

			if (print) {
				for (char const *s = utf8_result(&u8p); *s;) {
					*out++ = *s++;
				}
			}
		}

		if (!opt.m_join) {
			if (print) {
				if (out == buf || *(out - 1) != '\n')
					*out++ = '\n';
				*out = '\0';
				out = buf;

				if (!count)
					(void)fputs(buf, stdout);
				else if (!opt.m_bytes || !opt.m_chars)
					(void)printf("%zu\t%s", opt.m_bytes
					             ? n_bytes : n_chars, buf);
				else
					(void)printf("%zu\t%zu\t%s", n_chars,
					             n_bytes, buf);

			} else if (count) {
				if (!opt.m_bytes || !opt.m_chars)
					(void)printf("%zu\n", opt.m_bytes
					             ? n_bytes : n_chars);
				else
					(void)printf("%zu\t%zu\n", n_chars,
					             n_bytes);
			}

			n_chars = 0;
			n_bytes = 0;
			utf8_reset(&u8p);
		}
	}

	if (opt.m_join) {
		if (print) {
			if (out == buf || *(out - 1) != '\n')
				*out++ = '\n';
			*out = '\0';

			if (!count)
				(void)fputs(buf, stdout);
			else if (!opt.m_bytes || !opt.m_chars)
				(void)printf("%zu\t%s", opt.m_bytes
				             ? n_bytes : n_chars, buf);
			else
				(void)printf("%zu\t%zu\t%s", n_chars,
				             n_bytes, buf);

		} else if (count) {
			if (!opt.m_bytes || !opt.m_chars)
				(void)printf("%zu\n", opt.m_bytes
				             ? n_bytes : n_chars);
			else
				(void)printf("%zu\t%zu\n", n_chars,
				             n_bytes);
		}
	}

	free(buf);
	return letopt_fini(&opt);
}

static int
arg_conflict (struct letopt *opt)
{
	bool count = opt->m_bytes || opt->m_chars;
	int e = 0;

	if (opt->m_graph && (letopt_nargs(opt) > 0 || count
	                     || opt->m_help || opt->m_print
	                     || opt->m_join || opt->m_quiet
	                     || opt->m_skip)) {
		(void)fputs("error: graph dump option is exclusive\n",
		            stderr);
		e = EINVAL;

	} else {
		if (opt->m_quiet) {
			if (count) {
				(void)fputs("error: can't count quietly\n",
				            stderr);
				e = EINVAL;
			}

			if (opt->m_print) {
				(void)fputs("error: can't print quietly\n",
				            stderr);
				e = EINVAL;
			}
		}

		if (opt->m_skip &&
		    (opt->m_quiet || (!opt->m_print && count)))
			(void)fputs("warning: skip option ignored\n", stderr);
	}

	opt->p.e = e;
	return e;
}

static int
dump_utf8_graph (struct letopt *opt)
{
	static char buf[1024] = {0};
	char const *end = utf8_graph(buf, &buf[sizeof buf], &opt->p.e);

	if (end)
		(void)fputs(buf, stdout);
	else if (opt->p.e)
		(void)fprintf(stderr, "error: utf8_graph: %s\n",
		              strerror(opt->p.e));

	return letopt_fini(opt);
}
