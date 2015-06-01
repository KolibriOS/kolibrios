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

#include "radeon_elf_util.h"
#include "r600_pipe_common.h"

#include "util/u_memory.h"

#include <gelf.h>
#include <libelf.h>
#include <stdio.h>

static void parse_symbol_table(Elf_Data *symbol_table_data,
				const GElf_Shdr *symbol_table_header,
				struct radeon_shader_binary *binary)
{
	GElf_Sym symbol;
	unsigned i = 0;
	unsigned symbol_count =
		symbol_table_header->sh_size / symbol_table_header->sh_entsize;

	/* We are over allocating this list, because symbol_count gives the
 	 * total number of symbols, and we will only be filling the list
 	 * with offsets of global symbols.  The memory savings from
 	 * allocating the correct size of this list will be small, and
 	 * I don't think it is worth the cost of pre-computing the number
 	 * of global symbols.
 	 */
	binary->global_symbol_offsets = CALLOC(symbol_count, sizeof(uint64_t));

	while (gelf_getsym(symbol_table_data, i++, &symbol)) {
		unsigned i;
		if (GELF_ST_BIND(symbol.st_info) != STB_GLOBAL ||
		    symbol.st_shndx == 0 /* Undefined symbol */) {
			continue;
		}

		binary->global_symbol_offsets[binary->global_symbol_count] =
					symbol.st_value;

		/* Sort the list using bubble sort.  This list will usually
		 * be small. */
		for (i = binary->global_symbol_count; i > 0; --i) {
			uint64_t lhs = binary->global_symbol_offsets[i - 1];
			uint64_t rhs = binary->global_symbol_offsets[i];
			if (lhs < rhs) {
				break;
			}
			binary->global_symbol_offsets[i] = lhs;
			binary->global_symbol_offsets[i - 1] = rhs;
		}
		++binary->global_symbol_count;
	}
}

static void parse_relocs(Elf *elf, Elf_Data *relocs, Elf_Data *symbols,
			unsigned symbol_sh_link,
			struct radeon_shader_binary *binary)
{
	unsigned i;

	if (!relocs || !symbols || !binary->reloc_count) {
		return;
	}
	binary->relocs = CALLOC(binary->reloc_count,
			sizeof(struct radeon_shader_reloc));
	for (i = 0; i < binary->reloc_count; i++) {
		GElf_Sym symbol;
		GElf_Rel rel;
		char *symbol_name;
		struct radeon_shader_reloc *reloc = &binary->relocs[i];

		gelf_getrel(relocs, i, &rel);
		gelf_getsym(symbols, GELF_R_SYM(rel.r_info), &symbol);
		symbol_name = elf_strptr(elf, symbol_sh_link, symbol.st_name);

		reloc->offset = rel.r_offset;
		reloc->name = strdup(symbol_name);
	}
}

void radeon_elf_read(const char *elf_data, unsigned elf_size,
					struct radeon_shader_binary *binary,
					unsigned debug)
{
	char *elf_buffer;
	Elf *elf;
	Elf_Scn *section = NULL;
	Elf_Data *symbols = NULL, *relocs = NULL;
	size_t section_str_index;
	unsigned symbol_sh_link = 0;

	/* One of the libelf implementations
	 * (http://www.mr511.de/software/english.htm) requires calling
	 * elf_version() before elf_memory().
	 */
	elf_version(EV_CURRENT);
	elf_buffer = MALLOC(elf_size);
	memcpy(elf_buffer, elf_data, elf_size);

	elf = elf_memory(elf_buffer, elf_size);

	elf_getshdrstrndx(elf, &section_str_index);
	binary->disassembled = 0;

	while ((section = elf_nextscn(elf, section))) {
		const char *name;
		Elf_Data *section_data = NULL;
		GElf_Shdr section_header;
		if (gelf_getshdr(section, &section_header) != &section_header) {
			fprintf(stderr, "Failed to read ELF section header\n");
			return;
		}
		name = elf_strptr(elf, section_str_index, section_header.sh_name);
		if (!strcmp(name, ".text")) {
			section_data = elf_getdata(section, section_data);
			binary->code_size = section_data->d_size;
			binary->code = MALLOC(binary->code_size * sizeof(unsigned char));
			memcpy(binary->code, section_data->d_buf, binary->code_size);
		} else if (!strcmp(name, ".AMDGPU.config")) {
			section_data = elf_getdata(section, section_data);
			binary->config_size = section_data->d_size;
			binary->config = MALLOC(binary->config_size * sizeof(unsigned char));
			memcpy(binary->config, section_data->d_buf, binary->config_size);
		} else if (debug && !strcmp(name, ".AMDGPU.disasm")) {
			binary->disassembled = 1;
			section_data = elf_getdata(section, section_data);
			fprintf(stderr, "\nShader Disassembly:\n\n");
			fprintf(stderr, "%.*s\n", (int)section_data->d_size,
						  (char *)section_data->d_buf);
		} else if (!strncmp(name, ".rodata", 7)) {
			section_data = elf_getdata(section, section_data);
			binary->rodata_size = section_data->d_size;
			binary->rodata = MALLOC(binary->rodata_size * sizeof(unsigned char));
			memcpy(binary->rodata, section_data->d_buf, binary->rodata_size);
		} else if (!strncmp(name, ".symtab", 7)) {
			symbols = elf_getdata(section, section_data);
			symbol_sh_link = section_header.sh_link;
			parse_symbol_table(symbols, &section_header, binary);
		} else if (!strcmp(name, ".rel.text")) {
			relocs = elf_getdata(section, section_data);
			binary->reloc_count = section_header.sh_size /
					section_header.sh_entsize;
		}
	}

	parse_relocs(elf, relocs, symbols, symbol_sh_link, binary);

	if (elf){
		elf_end(elf);
	}
	FREE(elf_buffer);

	/* Cache the config size per symbol */
	if (binary->global_symbol_count) {
		binary->config_size_per_symbol =
			binary->config_size / binary->global_symbol_count;
	} else {
		binary->global_symbol_count = 1;
		binary->config_size_per_symbol = binary->config_size;
	}
}

const unsigned char *radeon_shader_binary_config_start(
	const struct radeon_shader_binary *binary,
	uint64_t symbol_offset)
{
	unsigned i;
	for (i = 0; i < binary->global_symbol_count; ++i) {
		if (binary->global_symbol_offsets[i] == symbol_offset) {
			unsigned offset = i * binary->config_size_per_symbol;
			return binary->config + offset;
		}
	}
	return binary->config;
}

void radeon_shader_binary_free_relocs(struct radeon_shader_reloc *relocs,
					unsigned reloc_count)
{
	unsigned i;
	for (i = 0; i < reloc_count; i++) {
		FREE(relocs[i].name);
	}
	FREE(relocs);
}

void radeon_shader_binary_free_members(struct radeon_shader_binary *binary,
					unsigned free_relocs)
{
	FREE(binary->code);
	FREE(binary->config);
	FREE(binary->rodata);

	if (free_relocs) {
		radeon_shader_binary_free_relocs(binary->relocs,
						binary->reloc_count);
	}
}
