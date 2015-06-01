/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 *    Rob Clark <robclark@freedesktop.org>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>

#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_text.h"
#include "tgsi/tgsi_dump.h"

#include "freedreno_util.h"

#include "ir3_compiler.h"
#include "instr-a3xx.h"
#include "ir3.h"

static void dump_reg(const char *name, uint32_t r)
{
	if (r != regid(63,0))
		debug_printf("; %s: r%d.%c\n", name, r >> 2, "xyzw"[r & 0x3]);
}

static void dump_semantic(struct ir3_shader_variant *so,
		unsigned sem, const char *name)
{
	uint32_t regid;
	regid = ir3_find_output_regid(so, ir3_semantic_name(sem, 0));
	dump_reg(name, regid);
}

static void dump_info(struct ir3_shader_variant *so, const char *str)
{
	uint32_t *bin;
	const char *type = (so->type == SHADER_VERTEX) ? "VERT" : "FRAG";

	// for debug, dump some before/after info:
	// TODO make gpu_id configurable on cmdline
	bin = ir3_shader_assemble(so, 320);
	if (fd_mesa_debug & FD_DBG_DISASM) {
		struct ir3_block *block = so->ir->block;
		struct ir3_register *reg;
		uint8_t regid;
		unsigned i;

		debug_printf("; %s: %s\n", type, str);

		for (i = 0; i < block->ninputs; i++) {
			if (!block->inputs[i]) {
				debug_printf("; in%d unused\n", i);
				continue;
			}
			reg = block->inputs[i]->regs[0];
			regid = reg->num;
			debug_printf("@in(%sr%d.%c)\tin%d\n",
					(reg->flags & IR3_REG_HALF) ? "h" : "",
					(regid >> 2), "xyzw"[regid & 0x3], i);
		}

		for (i = 0; i < block->noutputs; i++) {
			if (!block->outputs[i]) {
				debug_printf("; out%d unused\n", i);
				continue;
			}
			/* kill shows up as a virtual output.. skip it! */
			if (is_kill(block->outputs[i]))
				continue;
			reg = block->outputs[i]->regs[0];
			regid = reg->num;
			debug_printf("@out(%sr%d.%c)\tout%d\n",
					(reg->flags & IR3_REG_HALF) ? "h" : "",
					(regid >> 2), "xyzw"[regid & 0x3], i);
		}

		for (i = 0; i < so->immediates_count; i++) {
			debug_printf("@const(c%d.x)\t", so->first_immediate + i);
			debug_printf("0x%08x, 0x%08x, 0x%08x, 0x%08x\n",
					so->immediates[i].val[0],
					so->immediates[i].val[1],
					so->immediates[i].val[2],
					so->immediates[i].val[3]);
		}

		disasm_a3xx(bin, so->info.sizedwords, 0, so->type);

		debug_printf("; %s: outputs:", type);
		for (i = 0; i < so->outputs_count; i++) {
			uint8_t regid = so->outputs[i].regid;
			ir3_semantic sem = so->outputs[i].semantic;
			debug_printf(" r%d.%c (%u:%u)",
					(regid >> 2), "xyzw"[regid & 0x3],
					sem2name(sem), sem2idx(sem));
		}
		debug_printf("\n");
		debug_printf("; %s: inputs:", type);
		for (i = 0; i < so->inputs_count; i++) {
			uint8_t regid = so->inputs[i].regid;
			ir3_semantic sem = so->inputs[i].semantic;
			debug_printf(" r%d.%c (%u:%u,cm=%x,il=%u,b=%u)",
					(regid >> 2), "xyzw"[regid & 0x3],
					sem2name(sem), sem2idx(sem),
					so->inputs[i].compmask,
					so->inputs[i].inloc,
					so->inputs[i].bary);
		}
		debug_printf("\n");
	}

	/* print generic shader info: */
	debug_printf("; %s: %u instructions, %d half, %d full\n", type,
			so->info.instrs_count,
			so->info.max_half_reg + 1,
			so->info.max_reg + 1);

	/* print shader type specific info: */
	switch (so->type) {
	case SHADER_VERTEX:
		dump_semantic(so, TGSI_SEMANTIC_POSITION, "pos");
		dump_semantic(so, TGSI_SEMANTIC_PSIZE, "psize");
		break;
	case SHADER_FRAGMENT:
		dump_reg("pos (bary)", so->pos_regid);
		dump_semantic(so, TGSI_SEMANTIC_POSITION, "posz");
		dump_semantic(so, TGSI_SEMANTIC_COLOR, "color");
		/* these two are hard-coded since we don't know how to
		 * program them to anything but all 0's...
		 */
		if (so->frag_coord)
			debug_printf("; fragcoord: r0.x\n");
		if (so->frag_face)
			debug_printf("; fragface: hr0.x\n");
		break;
	case SHADER_COMPUTE:
		break;
	}
	free(bin);

	debug_printf("\n");
}


static int
read_file(const char *filename, void **ptr, size_t *size)
{
	int fd, ret;
	struct stat st;

	*ptr = MAP_FAILED;

	fd = open(filename, O_RDONLY);
	if (fd == -1) {
		warnx("couldn't open `%s'", filename);
		return 1;
	}

	ret = fstat(fd, &st);
	if (ret)
		errx(1, "couldn't stat `%s'", filename);

	*size = st.st_size;
	*ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (*ptr == MAP_FAILED)
		errx(1, "couldn't map `%s'", filename);

	close(fd);

	return 0;
}

static void reset_variant(struct ir3_shader_variant *v, const char *msg)
{
	printf("; %s\n", msg);
	v->inputs_count = 0;
	v->outputs_count = 0;
	v->total_in = 0;
	v->has_samp = false;
	v->immediates_count = 0;
}

static void print_usage(void)
{
	printf("Usage: ir3_compiler [OPTIONS]... FILE\n");
	printf("    --verbose         - verbose compiler/debug messages\n");
	printf("    --binning-pass    - generate binning pass shader (VERT)\n");
	printf("    --color-two-side  - emulate two-sided color (FRAG)\n");
	printf("    --half-precision  - use half-precision\n");
	printf("    --saturate-s MASK - bitmask of samplers to saturate S coord\n");
	printf("    --saturate-t MASK - bitmask of samplers to saturate T coord\n");
	printf("    --saturate-r MASK - bitmask of samplers to saturate R coord\n");
	printf("    --nocp            - disable copy propagation\n");
	printf("    --nir             - use NIR compiler\n");
	printf("    --help            - show this message\n");
}

int main(int argc, char **argv)
{
	int ret = 0, n = 1;
	const char *filename;
	struct tgsi_token toks[65536];
	struct tgsi_parse_context parse;
	struct ir3_shader_variant v;
	struct ir3_shader_key key = {};
	const char *info;
	void *ptr;
	size_t size;
	int use_nir = 0;

	fd_mesa_debug |= FD_DBG_DISASM;

	/* cmdline args which impact shader variant get spit out in a
	 * comment on the first line..  a quick/dirty way to preserve
	 * that info so when ir3test recompiles the shader with a new
	 * compiler version, we use the same shader-key settings:
	 */
	debug_printf("; options:");

	while (n < argc) {
		if (!strcmp(argv[n], "--verbose")) {
			fd_mesa_debug |=  FD_DBG_OPTDUMP | FD_DBG_MSGS | FD_DBG_OPTMSGS;
			n++;
			continue;
		}

		if (!strcmp(argv[n], "--binning-pass")) {
			debug_printf(" %s", argv[n]);
			key.binning_pass = true;
			n++;
			continue;
		}

		if (!strcmp(argv[n], "--color-two-side")) {
			debug_printf(" %s", argv[n]);
			key.color_two_side = true;
			n++;
			continue;
		}

		if (!strcmp(argv[n], "--half-precision")) {
			debug_printf(" %s", argv[n]);
			key.half_precision = true;
			n++;
			continue;
		}

		if (!strcmp(argv[n], "--saturate-s")) {
			debug_printf(" %s %s", argv[n], argv[n+1]);
			key.vsaturate_s = key.fsaturate_s = strtol(argv[n+1], NULL, 0);
			n += 2;
			continue;
		}

		if (!strcmp(argv[n], "--saturate-t")) {
			debug_printf(" %s %s", argv[n], argv[n+1]);
			key.vsaturate_t = key.fsaturate_t = strtol(argv[n+1], NULL, 0);
			n += 2;
			continue;
		}

		if (!strcmp(argv[n], "--saturate-r")) {
			debug_printf(" %s %s", argv[n], argv[n+1]);
			key.vsaturate_r = key.fsaturate_r = strtol(argv[n+1], NULL, 0);
			n += 2;
			continue;
		}

		if (!strcmp(argv[n], "--nocp")) {
			fd_mesa_debug |= FD_DBG_NOCP;
			n++;
			continue;
		}
		if (!strcmp(argv[n], "--nir")) {
			use_nir = true;
			n++;
			continue;
		}

		if (!strcmp(argv[n], "--help")) {
			print_usage();
			return 0;
		}

		break;
	}
	debug_printf("\n");

	filename = argv[n];

	memset(&v, 0, sizeof(v));
	v.key = key;

	ret = read_file(filename, &ptr, &size);
	if (ret) {
		print_usage();
		return ret;
	}

	if (fd_mesa_debug & FD_DBG_OPTMSGS)
		debug_printf("%s\n", (char *)ptr);

	if (!tgsi_text_translate(ptr, toks, Elements(toks)))
		errx(1, "could not parse `%s'", filename);

	tgsi_parse_init(&parse, toks);
	switch (parse.FullHeader.Processor.Processor) {
	case TGSI_PROCESSOR_FRAGMENT:
		v.type = SHADER_FRAGMENT;
		break;
	case TGSI_PROCESSOR_VERTEX:
		v.type = SHADER_VERTEX;
		break;
	case TGSI_PROCESSOR_COMPUTE:
		v.type = SHADER_COMPUTE;
		break;
	}

	if (use_nir) {
		info = "NIR compiler";
		ret = ir3_compile_shader_nir(&v, toks, key);
	} else {
		info = "TGSI compiler";
		ret = ir3_compile_shader(&v, toks, key, true);
	}

	if (ret) {
		reset_variant(&v, "compiler failed, trying without copy propagation!");
		info = "compiler (no copy propagation)";
		ret = ir3_compile_shader(&v, toks, key, false);
	}

	if (ret) {
		fprintf(stderr, "compiler failed!\n");
		return ret;
	}
	dump_info(&v, info);
}

void _mesa_error_no_memory(const char *caller);

void
_mesa_error_no_memory(const char *caller)
{
	fprintf(stderr, "Mesa error: out of memory in %s", caller);
}
