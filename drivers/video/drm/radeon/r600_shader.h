/*
 * RadeonHD R6xx, R7xx DRI driver
 *
 * Copyright (C) 2008-2009  Alexander Deucher
 * Copyright (C) 2008-2009  Matthias Hopf
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Shader macros
 */

#ifndef __SHADER_H__
#define __SHADER_H__

//#include "radeon.h"

/* Restrictions of ALU instructions
 * order of scalar ops is always x,y,z,w,t(rans), last to be indicated by last==1.
 * max of 3 different src GPRs per instr.
 * max of 4 different cfile constant components per instr.
 * max of 2 (different) constants (any type) for t.
 * bank swizzle (see below).
 * GPR write stalls read of same register. Auto-replaced by PV/PS, NOP needed if registers are relative to
 * different indices (gpr,loop,nothing).
 * may use constant registers or constant cache, but not both.
 */

/* Bank_swizzle: (pp. 297ff)
 * Only one of each x,y,z,w GPR component can be loaded per cycle (3 cycles per instr, called 0-2).
 * per scalar instruction bank_swizzle can select which cycle each operand comes from. e.g.:
 *   SRC0 SRC1 SRC2  SWIZZLE  cycle0 cycle1 cycle2
 *   1.x  2.x          012     1.x    2.x     -
 *   3.x  1.y          201     1.y     -     3.x
 *   2.x  1.y          102    (1.y)  (2.x)    -
 * If data is read in a cycle, multiple scalar instructions can reference it.
 * Special case: square() - i.e. same component in src0+src1 doesn't need read port -> ignores swizzle for src1.
 * No restrictions for constants or PV/PS.
 * t can load multiple components in a single cycle slot, but has to share cycles with xyzw.
 * t with single constant may not load GPRs or PV/PS in cycle 0 (carefull with ALU_TRANS_210).
 * t with two constants may only load GPRs or PV/PS in cycle 2.
 */


/* Oder of instructions: All CF, All ALU, All Tex/Vtx fetches */


// CF insts
// addr
#define ADDR(x)  (x)
// pc
#define POP_COUNT(x)      (x)
// const
#define CF_CONST(x)       (x)
// cond
#define COND(x)        (x)		// SQ_COND_*
// count
#define I_COUNT(x)        ((x) ? ((x) - 1) : 0)
//r7xx
#define COUNT_3(x)        (x)
// call count
#define CALL_COUNT(x)     (x)
// eop
#define END_OF_PROGRAM(x)   (x)
// vpm
#define VALID_PIXEL_MODE(x) (x)
// cf inst
#define CF_INST(x)        (x)		// SQ_CF_INST_*

// wqm
#define WHOLE_QUAD_MODE(x)  (x)
// barrier
#define BARRIER(x)          (x)
//kb0
#define KCACHE_BANK0(x)          (x)
//kb1
#define KCACHE_BANK1(x)          (x)
// km0/1
#define KCACHE_MODE0(x)          (x)
#define KCACHE_MODE1(x)          (x)	// SQ_CF_KCACHE_*
//
#define KCACHE_ADDR0(x)          (x)
#define KCACHE_ADDR1(x)          (x)
// uw
#define USES_WATERFALL(x)        (x)

#define ARRAY_BASE(x)        (x)
// export pixel
#define CF_PIXEL_MRT0         0
#define CF_PIXEL_MRT1         1
#define CF_PIXEL_MRT2         2
#define CF_PIXEL_MRT3         3
#define CF_PIXEL_MRT4         4
#define CF_PIXEL_MRT5         5
#define CF_PIXEL_MRT6         6
#define CF_PIXEL_MRT7         7
// *_FOG: r6xx only
#define CF_PIXEL_MRT0_FOG     16
#define CF_PIXEL_MRT1_FOG     17
#define CF_PIXEL_MRT2_FOG     18
#define CF_PIXEL_MRT3_FOG     19
#define CF_PIXEL_MRT4_FOG     20
#define CF_PIXEL_MRT5_FOG     21
#define CF_PIXEL_MRT6_FOG     22
#define CF_PIXEL_MRT7_FOG     23
#define CF_PIXEL_Z            61
// export pos
#define CF_POS0               60
#define CF_POS1               61
#define CF_POS2               62
#define CF_POS3               63
// export param
// 0...31
#define TYPE(x)              (x)	// SQ_EXPORT_*
#if 0
// type export
#define SQ_EXPORT_PIXEL              0
#define SQ_EXPORT_POS                1
#define SQ_EXPORT_PARAM              2
// reserved 3
// type mem
#define SQ_EXPORT_WRITE              0
#define SQ_EXPORT_WRITE_IND          1
#define SQ_EXPORT_WRITE_ACK          2
#define SQ_EXPORT_WRITE_IND_ACK      3
#endif

#define RW_GPR(x)            (x)
#define RW_REL(x)            (x)
#define ABSOLUTE                  0
#define RELATIVE                  1
#define INDEX_GPR(x)            (x)
#define ELEM_SIZE(x)            (x ? (x - 1) : 0)
#define COMP_MASK(x)            (x)
#define R6xx_ELEM_LOOP(x)            (x)
#define BURST_COUNT(x)          (x ? (x - 1) : 0)

// swiz
#define SRC_SEL_X(x)    (x)		// SQ_SEL_* each
#define SRC_SEL_Y(x)    (x)
#define SRC_SEL_Z(x)    (x)
#define SRC_SEL_W(x)    (x)

#define CF_DWORD0(addr) cpu_to_le32((addr))
// R7xx has another entry (COUNT3), but that is only used for adding a bit to count.
// We allow one more bit for count in the argument of the macro on R7xx instead.
// R6xx: [0,7]  R7xx: [1,16]
#define CF_DWORD1(pc, cf_const, cond, count, call_count, eop, vpm, cf_inst, wqm, b) \
    cpu_to_le32((((pc) << 0) | ((cf_const) << 3) | ((cond) << 8) | (((count) & 7) << 10) | (((count) >> 3) << 19) | \
		 ((call_count) << 13) | ((eop) << 21) | ((vpm) << 22) | ((cf_inst) << 23) | ((wqm) << 30) | ((b) << 31)))

#define CF_ALU_DWORD0(addr, kb0, kb1, km0) cpu_to_le32((((addr) << 0) | ((kb0) << 22) | ((kb1) << 26) | ((km0) << 30)))
#define CF_ALU_DWORD1(km1, kcache_addr0, kcache_addr1, count, uw, cf_inst, wqm, b) \
    cpu_to_le32((((km1) << 0) | ((kcache_addr0) << 2) | ((kcache_addr1) << 10) | \
		 ((count) << 18) | ((uw) << 25) | ((cf_inst) << 26) | ((wqm) << 30) | ((b) << 31)))

#define CF_ALLOC_IMP_EXP_DWORD0(array_base, type, rw_gpr, rr, index_gpr, es) \
    cpu_to_le32((((array_base) << 0) | ((type) << 13) | ((rw_gpr) << 15) | ((rr) << 22) | ((index_gpr) << 23) | \
		 ((es) << 30)))
// R7xx apparently doesn't have the ELEM_LOOP entry any more
// We still expose it, but ELEM_LOOP is explicitely R6xx now.
// TODO: is this just forgotten in the docs, or really not available any more?
#define CF_ALLOC_IMP_EXP_DWORD1_BUF(array_size, comp_mask, el, bc, eop, vpm, cf_inst, wqm, b) \
    cpu_to_le32((((array_size) << 0) | ((comp_mask) << 12) | ((el) << 16) | ((bc) << 17) | \
		 ((eop) << 21) | ((vpm) << 22) | ((cf_inst) << 23) | ((wqm) << 30) | ((b) << 31)))
#define CF_ALLOC_IMP_EXP_DWORD1_SWIZ(sel_x, sel_y, sel_z, sel_w, el, bc, eop, vpm, cf_inst, wqm, b) \
    cpu_to_le32((((sel_x) << 0) | ((sel_y) << 3) | ((sel_z) << 6) | ((sel_w) << 9) | ((el) << 16) | \
		 ((bc) << 17) | ((eop) << 21) | ((vpm) << 22) | ((cf_inst) << 23) | \
		 ((wqm) << 30) | ((b) << 31)))

// ALU clause insts
#define SRC0_SEL(x)        (x)
#define SRC1_SEL(x)        (x)
#define SRC2_SEL(x)        (x)
// src[0-2]_sel
//   0-127 GPR
// 128-159 kcache constants bank 0
// 160-191 kcache constants bank 1
// 248-255 special SQ_ALU_SRC_* (0, 1, etc.)
#define ALU_SRC_GPR_BASE        0
#define ALU_SRC_KCACHE0_BASE  128
#define ALU_SRC_KCACHE1_BASE  160
#define ALU_SRC_CFILE_BASE    256

#define SRC0_REL(x)        (x)
#define SRC1_REL(x)        (x)
#define SRC2_REL(x)        (x)
// elem
#define SRC0_ELEM(x)        (x)
#define SRC1_ELEM(x)        (x)
#define SRC2_ELEM(x)        (x)
#define ELEM_X        0
#define ELEM_Y        1
#define ELEM_Z        2
#define ELEM_W        3
// neg
#define SRC0_NEG(x)        (x)
#define SRC1_NEG(x)        (x)
#define SRC2_NEG(x)        (x)
// im
#define INDEX_MODE(x)    (x)		// SQ_INDEX_*
// ps
#define PRED_SEL(x)      (x)		// SQ_PRED_SEL_*
// last
#define LAST(x)          (x)
// abs
#define SRC0_ABS(x)       (x)
#define SRC1_ABS(x)       (x)
// uem
#define UPDATE_EXECUTE_MASK(x) (x)
// up
#define UPDATE_PRED(x)      (x)
// wm
#define WRITE_MASK(x)   (x)
// fm
#define FOG_MERGE(x)    (x)
// omod
#define OMOD(x)        (x)		// SQ_ALU_OMOD_*
// alu inst
#define ALU_INST(x)        (x)		// SQ_ALU_INST_*
//bs
#define BANK_SWIZZLE(x)        (x)	// SQ_ALU_VEC_*
#define DST_GPR(x)        (x)
#define DST_REL(x)        (x)
#define DST_ELEM(x)       (x)
#define CLAMP(x)          (x)

#define ALU_DWORD0(src0_sel, s0r, s0e, s0n, src1_sel, s1r, s1e, s1n, im, ps, last) \
    cpu_to_le32((((src0_sel) << 0) | ((s0r) << 9) | ((s0e) << 10) | ((s0n) << 12) | \
		 ((src1_sel) << 13) | ((s1r) << 22) | ((s1e) << 23) | ((s1n) << 25) | \
		 ((im) << 26) | ((ps) << 29) | ((last) << 31)))
// R7xx has alu_inst at a different slot, and no fog merge any more (no fix function fog any more)
#define R6xx_ALU_DWORD1_OP2(s0a, s1a, uem, up, wm, fm, omod, alu_inst, bs, dst_gpr, dr, de, clamp) \
    cpu_to_le32((((s0a) << 0) | ((s1a) << 1) | ((uem) << 2) | ((up) << 3) | ((wm) << 4) | \
		 ((fm) << 5) | ((omod) << 6) | ((alu_inst) << 8) | ((bs) << 18) | ((dst_gpr) << 21) | \
		 ((dr) << 28) | ((de) << 29) | ((clamp) << 31)))
#define R7xx_ALU_DWORD1_OP2(s0a, s1a, uem, up, wm, omod, alu_inst, bs, dst_gpr, dr, de, clamp) \
    cpu_to_le32((((s0a) << 0) | ((s1a) << 1) | ((uem) << 2) | ((up) << 3) | ((wm) << 4) | \
		 ((omod) << 5) | ((alu_inst) << 7) | ((bs) << 18) | ((dst_gpr) << 21) | \
		 ((dr) << 28) | ((de) << 29) | ((clamp) << 31)))
// This is a general chipset macro, but due to selection by chipid typically not usable in static arrays
// Fog is NOT USED on R7xx, even if specified.
#define ALU_DWORD1_OP2(chipfamily, s0a, s1a, uem, up, wm, fm, omod, alu_inst, bs, dst_gpr, dr, de, clamp) \
    ((chipfamily) < CHIP_FAMILY_RV770 ? \
     R6xx_ALU_DWORD1_OP2(s0a, s1a, uem, up, wm, fm, omod, alu_inst, bs, dst_gpr, dr, de, clamp) : \
     R7xx_ALU_DWORD1_OP2(s0a, s1a, uem, up, wm, omod, alu_inst, bs, dst_gpr, dr, de, clamp))
#define ALU_DWORD1_OP3(src2_sel, s2r, s2e, s2n, alu_inst, bs, dst_gpr, dr, de, clamp) \
    cpu_to_le32((((src2_sel) << 0) | ((s2r) << 9) | ((s2e) << 10) | ((s2n) << 12) | \
		 ((alu_inst) << 13) | ((bs) << 18) | ((dst_gpr) << 21) | ((dr) << 28) | \
		 ((de) << 29) | ((clamp) << 31)))

// VTX clause insts
// vxt insts
#define VTX_INST(x)        (x)		// SQ_VTX_INST_*

// fetch type
#define FETCH_TYPE(x)        (x)	// SQ_VTX_FETCH_*

#define FETCH_WHOLE_QUAD(x)        (x)
#define BUFFER_ID(x)        (x)
#define SRC_GPR(x)          (x)
#define SRC_REL(x)          (x)
#define MEGA_FETCH_COUNT(x)        ((x) ? ((x) - 1) : 0)

#define SEMANTIC_ID(x)        (x)
#define DST_SEL_X(x)          (x)
#define DST_SEL_Y(x)          (x)
#define DST_SEL_Z(x)          (x)
#define DST_SEL_W(x)          (x)
#define USE_CONST_FIELDS(x)   (x)
#define DATA_FORMAT(x)        (x)
// num format
#define NUM_FORMAT_ALL(x)     (x)	// SQ_NUM_FORMAT_*
// format comp
#define FORMAT_COMP_ALL(x)     (x)	// SQ_FORMAT_COMP_*
// sma
#define SRF_MODE_ALL(x)     (x)
#define SRF_MODE_ZERO_CLAMP_MINUS_ONE      0
#define SRF_MODE_NO_ZERO                   1
#define OFFSET(x)     (x)
// endian swap
#define ENDIAN_SWAP(x)     (x)		// SQ_ENDIAN_*
#define CONST_BUF_NO_STRIDE(x)     (x)
// mf
#define MEGA_FETCH(x)     (x)

#define VTX_DWORD0(vtx_inst, ft, fwq, buffer_id, src_gpr, sr, ssx, mfc) \
    cpu_to_le32((((vtx_inst) << 0) | ((ft) << 5) | ((fwq) << 7) | ((buffer_id) << 8) | \
		 ((src_gpr) << 16) | ((sr) << 23) | ((ssx) << 24) | ((mfc) << 26)))
#define VTX_DWORD1_SEM(semantic_id, dsx, dsy, dsz, dsw, ucf, data_format, nfa, fca, sma) \
    cpu_to_le32((((semantic_id) << 0) | ((dsx) << 9) | ((dsy) << 12) | ((dsz) << 15) | ((dsw) << 18) | \
		 ((ucf) << 21) | ((data_format) << 22) | ((nfa) << 28) | ((fca) << 30) | ((sma) << 31)))
#define VTX_DWORD1_GPR(dst_gpr, dr, dsx, dsy, dsz, dsw, ucf, data_format, nfa, fca, sma) \
    cpu_to_le32((((dst_gpr) << 0) | ((dr) << 7) | ((dsx) << 9) | ((dsy) << 12) | ((dsz) << 15) | ((dsw) << 18) | \
		 ((ucf) << 21) | ((data_format) << 22) | ((nfa) << 28) | ((fca) << 30) | ((sma) << 31)))
#define VTX_DWORD2(offset, es, cbns, mf) \
    cpu_to_le32((((offset) << 0) | ((es) << 16) | ((cbns) << 18) | ((mf) << 19)))
#define VTX_DWORD_PAD cpu_to_le32(0x00000000)

// TEX clause insts
// tex insts
#define TEX_INST(x)     (x)		// SQ_TEX_INST_*

#define BC_FRAC_MODE(x)         (x)
#define FETCH_WHOLE_QUAD(x)     (x)
#define RESOURCE_ID(x)          (x)
#define R7xx_ALT_CONST(x)            (x)

#define LOD_BIAS(x)     (x)
//ct
#define COORD_TYPE_X(x)     (x)
#define COORD_TYPE_Y(x)     (x)
#define COORD_TYPE_Z(x)     (x)
#define COORD_TYPE_W(x)     (x)
#define TEX_UNNORMALIZED                0
#define TEX_NORMALIZED                  1
#define OFFSET_X(x) (((int)(x) * 2) & 0x1f) /* 4:1-bits 2's-complement fixed-point: [-8.0..7.5] */
#define OFFSET_Y(x) (((int)(x) * 2) & 0x1f)
#define OFFSET_Z(x) (((int)(x) * 2) & 0x1f)
#define SAMPLER_ID(x)     (x)

// R7xx has an additional parameter ALT_CONST. We always expose it, but ALT_CONST is R7xx only
#define TEX_DWORD0(tex_inst, bfm, fwq, resource_id, src_gpr, sr, ac) \
    cpu_to_le32((((tex_inst) << 0) | ((bfm) << 5) | ((fwq) << 7) | ((resource_id) << 8) | \
		 ((src_gpr) << 16) | ((sr) << 23) | ((ac) << 24)))
#define TEX_DWORD1(dst_gpr, dr, dsx, dsy, dsz, dsw, lod_bias, ctx, cty, ctz, ctw) \
    cpu_to_le32((((dst_gpr) << 0) | ((dr) << 7) | ((dsx) << 9) | ((dsy) << 12) | ((dsz) << 15) | ((dsw) << 18) | \
		 ((lod_bias) << 21) | ((ctx) << 28) | ((cty) << 29) | ((ctz) << 30) | ((ctw) << 31)))
#define TEX_DWORD2(offset_x, offset_y, offset_z, sampler_id, ssx, ssy, ssz, ssw) \
    cpu_to_le32((((offset_x) << 0) | ((offset_y) << 5) | ((offset_z) << 10) | ((sampler_id) << 15) | \
		 ((ssx) << 20) | ((ssy) << 23) | ((ssz) << 26) | ((ssw) << 29)))
#define TEX_DWORD_PAD cpu_to_le32(0x00000000)

#endif
