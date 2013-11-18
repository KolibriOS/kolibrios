/*
 * Copyright (c) 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#ifndef _SNA_COMPILER_H_
#define _SNA_COMPILER_H_

#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define likely(expr) (__builtin_expect (!!(expr), 1))
#define unlikely(expr) (__builtin_expect (!!(expr), 0))
#define noinline __attribute__((noinline))
#define force_inline inline __attribute__((always_inline))
#define fastcall __attribute__((regparm(3)))
#define must_check __attribute__((warn_unused_result))
#define constant __attribute__((const))
#define pure __attribute__((pure))
#define tightly_packed __attribute__((__packed__))
#define flatten __attribute__((flatten))
#define page_aligned __attribute__((aligned(4096)))
#else
#define likely(expr) (expr)
#define unlikely(expr) (expr)
#define noinline
#define force_inline inline
#define fastcall
#define must_check
#define constant
#define pure
#define tighly_packed
#define flatten
#define page_aligned
#endif

#define HAS_GCC(major, minor) defined(__GNUC__) && (__GNUC__ > (major) || __GNUC__ == (major) && __GNUC_MINOR__ >= (minor))

#if HAS_GCC(4, 5)
#define sse2 __attribute__((target("sse2,fpmath=sse")))
#define sse4_2 __attribute__((target("sse4.2,sse2,fpmath=sse")))
#endif

#if HAS_GCC(4, 7)
#define avx2 __attribute__((target("avx2,sse4.2,sse2,fpmath=sse")))
#endif

#if HAS_GCC(4, 6) && defined(__OPTIMIZE__)
#define fast __attribute__((optimize("Ofast")))
#else
#define fast
#endif

#if HAS_GCC(4, 6) && defined(__OPTIMIZE__)
#define fast_memcpy __attribute__((optimize("Ofast"))) __attribute__((target("inline-all-stringops")))
#elif HAS_GCC(4, 5) && defined(__OPTIMIZE__)
#define fast_memcpy __attribute__((target("inline-all-stringops")))
#else
#define fast_memcpy
#endif

#ifdef HAVE_VALGRIND
#define VG(x) x
#else
#define VG(x)
#endif

#define VG_CLEAR(s) VG(memset(&s, 0, sizeof(s)))

#define COMPILE_TIME_ASSERT(E) ((void)sizeof(char[1 - 2*!(E)]))

#endif /* _SNA_COMPILER_H_ */
