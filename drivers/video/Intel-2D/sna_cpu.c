/*
 * Copyright (c) 2013 Intel Corporation
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sna.h"
#include "sna_cpuid.h"

#define xgetbv(index,eax,edx)                                   \
	__asm__ ("xgetbv" : "=a"(eax), "=d"(edx) : "c" (index))

#define has_YMM 0x1

unsigned sna_cpu_detect(void)
{
	unsigned max = __get_cpuid_max(BASIC_CPUID, NULL);
	unsigned int eax, ebx, ecx, edx;
	unsigned features = 0;
	unsigned extra = 0;

	if (max >= 1) {
		__cpuid(1, eax, ebx, ecx, edx);
		if (ecx & bit_SSE3)
			features |= SSE3;

		if (ecx & bit_SSSE3)
			features |= SSSE3;

		if (ecx & bit_SSE4_1)
			features |= SSE4_1;

		if (ecx & bit_SSE4_2)
			features |= SSE4_2;

		if (ecx & bit_OSXSAVE) {
			unsigned int bv_eax, bv_ecx;
			xgetbv(0, bv_eax, bv_ecx);
			if ((bv_eax & 6) == 6)
				extra |= has_YMM;
		}

		if ((extra & has_YMM) && (ecx & bit_AVX))
			features |= AVX;

		if (edx & bit_MMX)
			features |= MMX;

		if (edx & bit_SSE)
			features |= SSE;

		if (edx & bit_SSE2)
			features |= SSE2;
	}

	if (max >= 7) {
		__cpuid_count(7, 0, eax, ebx, ecx, edx);
		if ((extra & has_YMM) && (ebx & bit_AVX2))
			features |= AVX2;
	}

	return features;
}

char *sna_cpu_features_to_string(unsigned features, char *line)
{
	char *ret = line;

#ifdef __x86_64__
	line += sprintf (line, "x86-64");
#else
	line += sprintf (line, "x86");
#endif

	if (features & SSE2)
		line += sprintf (line, ", sse2");
	if (features & SSE3)
		line += sprintf (line, ", sse3");
	if (features & SSSE3)
		line += sprintf (line, ", ssse3");
	if (features & SSE4_1)
		line += sprintf (line, ", sse4.1");
	if (features & SSE4_2)
		line += sprintf (line, ", sse4.2");
	if (features & AVX)
		line += sprintf (line, ", avx");
	if (features & AVX2)
		line += sprintf (line, ", avx2");

	return ret;
}
