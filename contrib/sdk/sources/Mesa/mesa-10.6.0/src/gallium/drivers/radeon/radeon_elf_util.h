/*
 * Copyright 2014 Advanced Micro Devices, Inc.
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
 * Authors: Tom Stellard <thomas.stellard@amd.com>
 *
 */

#ifndef RADEON_ELF_UTIL_H
#define RADEON_ELF_UTIL_H

#include <stdint.h>

struct radeon_shader_binary;
struct radeon_shader_reloc;

/*
 * Parse the elf binary stored in \p elf_data and create a
 * radeon_shader_binary object.
 */
void radeon_elf_read(const char *elf_data, unsigned elf_size,
		struct radeon_shader_binary *binary, unsigned debug);

/**
 * @returns A pointer to the start of the configuration information for
 * the function starting at \p symbol_offset of the binary.
 */
const unsigned char *radeon_shader_binary_config_start(
	const struct radeon_shader_binary *binary,
	uint64_t symbol_offset);

/**
 * Free all memory allocated for members of \p binary.  This function does
 * not free \p binary.
 *
 * @param free_relocs If false, reolc information will not be freed.
 */
void radeon_shader_binary_free_members(struct radeon_shader_binary *binary,
	unsigned free_relocs);

/**
 * Free \p relocs and all member data.
 */
void radeon_shader_binary_free_relocs(struct radeon_shader_reloc *relocs,
					unsigned reloc_count);
#endif /* RADEON_ELF_UTIL_H */
