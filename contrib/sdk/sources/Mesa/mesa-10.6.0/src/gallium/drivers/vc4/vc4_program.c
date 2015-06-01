/*
 * Copyright (c) 2014 Scott Mansell
 * Copyright © 2014 Broadcom
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <inttypes.h>
#include "pipe/p_state.h"
#include "util/u_format.h"
#include "util/u_hash.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_pack_color.h"
#include "util/format_srgb.h"
#include "util/ralloc.h"
#include "util/hash_table.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_info.h"
#include "tgsi/tgsi_lowering.h"
#include "tgsi/tgsi_parse.h"
#include "nir/tgsi_to_nir.h"

#include "vc4_context.h"
#include "vc4_qpu.h"
#include "vc4_qir.h"
#ifdef USE_VC4_SIMULATOR
#include "simpenrose/simpenrose.h"
#endif

struct vc4_key {
        struct vc4_uncompiled_shader *shader_state;
        struct {
                enum pipe_format format;
                unsigned compare_mode:1;
                unsigned compare_func:3;
                unsigned wrap_s:3;
                unsigned wrap_t:3;
                uint8_t swizzle[4];
        } tex[VC4_MAX_TEXTURE_SAMPLERS];
        uint8_t ucp_enables;
};

struct vc4_fs_key {
        struct vc4_key base;
        enum pipe_format color_format;
        bool depth_enabled;
        bool stencil_enabled;
        bool stencil_twoside;
        bool stencil_full_writemasks;
        bool is_points;
        bool is_lines;
        bool alpha_test;
        bool point_coord_upper_left;
        bool light_twoside;
        uint8_t alpha_test_func;
        uint8_t logicop_func;
        uint32_t point_sprite_mask;

        struct pipe_rt_blend_state blend;
};

struct vc4_vs_key {
        struct vc4_key base;

        /**
         * This is a proxy for the array of FS input semantics, which is
         * larger than we would want to put in the key.
         */
        uint64_t compiled_fs_id;

        enum pipe_format attr_formats[8];
        bool is_coord;
        bool per_vertex_point_size;
};

static void
resize_qreg_array(struct vc4_compile *c,
                  struct qreg **regs,
                  uint32_t *size,
                  uint32_t decl_size)
{
        if (*size >= decl_size)
                return;

        uint32_t old_size = *size;
        *size = MAX2(*size * 2, decl_size);
        *regs = reralloc(c, *regs, struct qreg, *size);
        if (!*regs) {
                fprintf(stderr, "Malloc failure\n");
                abort();
        }

        for (uint32_t i = old_size; i < *size; i++)
                (*regs)[i] = c->undef;
}

static struct qreg
indirect_uniform_load(struct vc4_compile *c,
                      struct qreg indirect_offset,
                      unsigned offset)
{
        struct vc4_compiler_ubo_range *range = NULL;
        unsigned i;
        for (i = 0; i < c->num_uniform_ranges; i++) {
                range = &c->ubo_ranges[i];
                if (offset >= range->src_offset &&
                    offset < range->src_offset + range->size) {
                        break;
                }
        }
        /* The driver-location-based offset always has to be within a declared
         * uniform range.
         */
        assert(range);
        if (!range->used) {
                range->used = true;
                range->dst_offset = c->next_ubo_dst_offset;
                c->next_ubo_dst_offset += range->size;
                c->num_ubo_ranges++;
        };

        offset -= range->src_offset;
        /* Translate the user's TGSI register index from the TGSI register
         * base to a byte offset.
         */
        indirect_offset = qir_SHL(c, indirect_offset, qir_uniform_ui(c, 4));

        /* Adjust for where we stored the TGSI register base. */
        indirect_offset = qir_ADD(c, indirect_offset,
                                  qir_uniform_ui(c, (range->dst_offset +
                                                     offset)));
        indirect_offset = qir_MIN(c, indirect_offset,
                                  qir_uniform_ui(c, (range->dst_offset +
                                                     range->size - 4)));

        qir_TEX_DIRECT(c, indirect_offset, qir_uniform(c, QUNIFORM_UBO_ADDR, 0));
        struct qreg r4 = qir_TEX_RESULT(c);
        c->num_texture_samples++;
        return qir_MOV(c, r4);
}

static struct qreg *
ntq_get_dest(struct vc4_compile *c, nir_dest dest)
{
        assert(!dest.is_ssa);
        nir_register *reg = dest.reg.reg;
        struct hash_entry *entry = _mesa_hash_table_search(c->def_ht, reg);
        assert(reg->num_array_elems == 0);
        assert(dest.reg.base_offset == 0);

        struct qreg *qregs = entry->data;
        return qregs;
}

static struct qreg
ntq_get_src(struct vc4_compile *c, nir_src src, int i)
{
        struct hash_entry *entry;
        if (src.is_ssa) {
                entry = _mesa_hash_table_search(c->def_ht, src.ssa);
                assert(i < src.ssa->num_components);
        } else {
                nir_register *reg = src.reg.reg;
                entry = _mesa_hash_table_search(c->def_ht, reg);
                assert(reg->num_array_elems == 0);
                assert(src.reg.base_offset == 0);
                assert(i < reg->num_components);
        }

        struct qreg *qregs = entry->data;
        return qregs[i];
}

static struct qreg
ntq_get_alu_src(struct vc4_compile *c, nir_alu_instr *instr,
                unsigned src)
{
        assert(util_is_power_of_two(instr->dest.write_mask));
        unsigned chan = ffs(instr->dest.write_mask) - 1;
        struct qreg r = ntq_get_src(c, instr->src[src].src,
                                    instr->src[src].swizzle[chan]);

        assert(!instr->src[src].abs);
        assert(!instr->src[src].negate);

        return r;
};

static struct qreg
get_swizzled_channel(struct vc4_compile *c,
                     struct qreg *srcs, int swiz)
{
        switch (swiz) {
        default:
        case UTIL_FORMAT_SWIZZLE_NONE:
                fprintf(stderr, "warning: unknown swizzle\n");
                /* FALLTHROUGH */
        case UTIL_FORMAT_SWIZZLE_0:
                return qir_uniform_f(c, 0.0);
        case UTIL_FORMAT_SWIZZLE_1:
                return qir_uniform_f(c, 1.0);
        case UTIL_FORMAT_SWIZZLE_X:
        case UTIL_FORMAT_SWIZZLE_Y:
        case UTIL_FORMAT_SWIZZLE_Z:
        case UTIL_FORMAT_SWIZZLE_W:
                return srcs[swiz];
        }
}

static inline struct qreg
qir_SAT(struct vc4_compile *c, struct qreg val)
{
        return qir_FMAX(c,
                        qir_FMIN(c, val, qir_uniform_f(c, 1.0)),
                        qir_uniform_f(c, 0.0));
}

static struct qreg
ntq_rcp(struct vc4_compile *c, struct qreg x)
{
        struct qreg r = qir_RCP(c, x);

        /* Apply a Newton-Raphson step to improve the accuracy. */
        r = qir_FMUL(c, r, qir_FSUB(c,
                                    qir_uniform_f(c, 2.0),
                                    qir_FMUL(c, x, r)));

        return r;
}

static struct qreg
ntq_rsq(struct vc4_compile *c, struct qreg x)
{
        struct qreg r = qir_RSQ(c, x);

        /* Apply a Newton-Raphson step to improve the accuracy. */
        r = qir_FMUL(c, r, qir_FSUB(c,
                                    qir_uniform_f(c, 1.5),
                                    qir_FMUL(c,
                                             qir_uniform_f(c, 0.5),
                                             qir_FMUL(c, x,
                                                      qir_FMUL(c, r, r)))));

        return r;
}

static struct qreg
qir_srgb_decode(struct vc4_compile *c, struct qreg srgb)
{
        struct qreg low = qir_FMUL(c, srgb, qir_uniform_f(c, 1.0 / 12.92));
        struct qreg high = qir_POW(c,
                                   qir_FMUL(c,
                                            qir_FADD(c,
                                                     srgb,
                                                     qir_uniform_f(c, 0.055)),
                                            qir_uniform_f(c, 1.0 / 1.055)),
                                   qir_uniform_f(c, 2.4));

        qir_SF(c, qir_FSUB(c, srgb, qir_uniform_f(c, 0.04045)));
        return qir_SEL_X_Y_NS(c, low, high);
}

static struct qreg
qir_srgb_encode(struct vc4_compile *c, struct qreg linear)
{
        struct qreg low = qir_FMUL(c, linear, qir_uniform_f(c, 12.92));
        struct qreg high = qir_FSUB(c,
                                    qir_FMUL(c,
                                             qir_uniform_f(c, 1.055),
                                             qir_POW(c,
                                                     linear,
                                                     qir_uniform_f(c, 0.41666))),
                                    qir_uniform_f(c, 0.055));

        qir_SF(c, qir_FSUB(c, linear, qir_uniform_f(c, 0.0031308)));
        return qir_SEL_X_Y_NS(c, low, high);
}

static struct qreg
ntq_umul(struct vc4_compile *c, struct qreg src0, struct qreg src1)
{
        struct qreg src0_hi = qir_SHR(c, src0,
                                      qir_uniform_ui(c, 24));
        struct qreg src1_hi = qir_SHR(c, src1,
                                      qir_uniform_ui(c, 24));

        struct qreg hilo = qir_MUL24(c, src0_hi, src1);
        struct qreg lohi = qir_MUL24(c, src0, src1_hi);
        struct qreg lolo = qir_MUL24(c, src0, src1);

        return qir_ADD(c, lolo, qir_SHL(c,
                                        qir_ADD(c, hilo, lohi),
                                        qir_uniform_ui(c, 24)));
}

static void
ntq_emit_tex(struct vc4_compile *c, nir_tex_instr *instr)
{
        struct qreg s, t, r, lod, proj, compare;
        bool is_txb = false, is_txl = false, has_proj = false;
        unsigned unit = instr->sampler_index;

        for (unsigned i = 0; i < instr->num_srcs; i++) {
                switch (instr->src[i].src_type) {
                case nir_tex_src_coord:
                        s = ntq_get_src(c, instr->src[i].src, 0);
                        if (instr->sampler_dim != GLSL_SAMPLER_DIM_1D)
                                t = ntq_get_src(c, instr->src[i].src, 1);
                        if (instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE)
                                r = ntq_get_src(c, instr->src[i].src, 2);
                        break;
                case nir_tex_src_bias:
                        lod = ntq_get_src(c, instr->src[i].src, 0);
                        is_txb = true;
                        break;
                case nir_tex_src_lod:
                        lod = ntq_get_src(c, instr->src[i].src, 0);
                        is_txl = true;
                        break;
                case nir_tex_src_comparitor:
                        compare = ntq_get_src(c, instr->src[i].src, 0);
                        break;
                case nir_tex_src_projector:
                        proj = qir_RCP(c, ntq_get_src(c, instr->src[i].src, 0));
                        s = qir_FMUL(c, s, proj);
                        t = qir_FMUL(c, t, proj);
                        has_proj = true;
                        break;
                default:
                        unreachable("unknown texture source");
                }
        }

        struct qreg texture_u[] = {
                qir_uniform(c, QUNIFORM_TEXTURE_CONFIG_P0, unit),
                qir_uniform(c, QUNIFORM_TEXTURE_CONFIG_P1, unit),
                qir_uniform(c, QUNIFORM_CONSTANT, 0),
                qir_uniform(c, QUNIFORM_CONSTANT, 0),
        };
        uint32_t next_texture_u = 0;

        /* There is no native support for GL texture rectangle coordinates, so
         * we have to rescale from ([0, width], [0, height]) to ([0, 1], [0,
         * 1]).
         */
        if (instr->sampler_dim == GLSL_SAMPLER_DIM_RECT) {
                s = qir_FMUL(c, s,
                             qir_uniform(c, QUNIFORM_TEXRECT_SCALE_X, unit));
                t = qir_FMUL(c, t,
                             qir_uniform(c, QUNIFORM_TEXRECT_SCALE_Y, unit));
        }

        if (instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE || is_txl) {
                texture_u[2] = qir_uniform(c, QUNIFORM_TEXTURE_CONFIG_P2,
                                           unit | (is_txl << 16));
        }

        if (instr->sampler_dim == GLSL_SAMPLER_DIM_CUBE) {
                struct qreg ma = qir_FMAXABS(c, qir_FMAXABS(c, s, t), r);
                struct qreg rcp_ma = qir_RCP(c, ma);
                s = qir_FMUL(c, s, rcp_ma);
                t = qir_FMUL(c, t, rcp_ma);
                r = qir_FMUL(c, r, rcp_ma);

                qir_TEX_R(c, r, texture_u[next_texture_u++]);
        } else if (c->key->tex[unit].wrap_s == PIPE_TEX_WRAP_CLAMP_TO_BORDER ||
                   c->key->tex[unit].wrap_s == PIPE_TEX_WRAP_CLAMP ||
                   c->key->tex[unit].wrap_t == PIPE_TEX_WRAP_CLAMP_TO_BORDER ||
                   c->key->tex[unit].wrap_t == PIPE_TEX_WRAP_CLAMP) {
                qir_TEX_R(c, qir_uniform(c, QUNIFORM_TEXTURE_BORDER_COLOR, unit),
                          texture_u[next_texture_u++]);
        }

        if (c->key->tex[unit].wrap_s == PIPE_TEX_WRAP_CLAMP) {
                s = qir_SAT(c, s);
        }

        if (c->key->tex[unit].wrap_t == PIPE_TEX_WRAP_CLAMP) {
                t = qir_SAT(c, t);
        }

        qir_TEX_T(c, t, texture_u[next_texture_u++]);

        if (is_txl || is_txb)
                qir_TEX_B(c, lod, texture_u[next_texture_u++]);

        qir_TEX_S(c, s, texture_u[next_texture_u++]);

        c->num_texture_samples++;
        struct qreg r4 = qir_TEX_RESULT(c);

        enum pipe_format format = c->key->tex[unit].format;

        struct qreg unpacked[4];
        if (util_format_is_depth_or_stencil(format)) {
                struct qreg depthf = qir_ITOF(c, qir_SHR(c, r4,
                                                         qir_uniform_ui(c, 8)));
                struct qreg normalized = qir_FMUL(c, depthf,
                                                  qir_uniform_f(c, 1.0f/0xffffff));

                struct qreg depth_output;

                struct qreg one = qir_uniform_f(c, 1.0f);
                if (c->key->tex[unit].compare_mode) {
                        if (has_proj)
                                compare = qir_FMUL(c, compare, proj);

                        switch (c->key->tex[unit].compare_func) {
                        case PIPE_FUNC_NEVER:
                                depth_output = qir_uniform_f(c, 0.0f);
                                break;
                        case PIPE_FUNC_ALWAYS:
                                depth_output = one;
                                break;
                        case PIPE_FUNC_EQUAL:
                                qir_SF(c, qir_FSUB(c, compare, normalized));
                                depth_output = qir_SEL_X_0_ZS(c, one);
                                break;
                        case PIPE_FUNC_NOTEQUAL:
                                qir_SF(c, qir_FSUB(c, compare, normalized));
                                depth_output = qir_SEL_X_0_ZC(c, one);
                                break;
                        case PIPE_FUNC_GREATER:
                                qir_SF(c, qir_FSUB(c, compare, normalized));
                                depth_output = qir_SEL_X_0_NC(c, one);
                                break;
                        case PIPE_FUNC_GEQUAL:
                                qir_SF(c, qir_FSUB(c, normalized, compare));
                                depth_output = qir_SEL_X_0_NS(c, one);
                                break;
                        case PIPE_FUNC_LESS:
                                qir_SF(c, qir_FSUB(c, compare, normalized));
                                depth_output = qir_SEL_X_0_NS(c, one);
                                break;
                        case PIPE_FUNC_LEQUAL:
                                qir_SF(c, qir_FSUB(c, normalized, compare));
                                depth_output = qir_SEL_X_0_NC(c, one);
                                break;
                        }
                } else {
                        depth_output = normalized;
                }

                for (int i = 0; i < 4; i++)
                        unpacked[i] = depth_output;
        } else {
                for (int i = 0; i < 4; i++)
                        unpacked[i] = qir_R4_UNPACK(c, r4, i);
        }

        const uint8_t *format_swiz = vc4_get_format_swizzle(format);
        struct qreg texture_output[4];
        for (int i = 0; i < 4; i++) {
                texture_output[i] = get_swizzled_channel(c, unpacked,
                                                         format_swiz[i]);
        }

        if (util_format_is_srgb(format)) {
                for (int i = 0; i < 3; i++)
                        texture_output[i] = qir_srgb_decode(c,
                                                            texture_output[i]);
        }

        struct qreg *dest = ntq_get_dest(c, instr->dest);
        for (int i = 0; i < 4; i++) {
                dest[i] = get_swizzled_channel(c, texture_output,
                                               c->key->tex[unit].swizzle[i]);
        }
}

/**
 * Computes x - floor(x), which is tricky because our FTOI truncates (rounds
 * to zero).
 */
static struct qreg
ntq_ffract(struct vc4_compile *c, struct qreg src)
{
        struct qreg trunc = qir_ITOF(c, qir_FTOI(c, src));
        struct qreg diff = qir_FSUB(c, src, trunc);
        qir_SF(c, diff);
        return qir_SEL_X_Y_NS(c,
                              qir_FADD(c, diff, qir_uniform_f(c, 1.0)),
                              diff);
}

/**
 * Computes floor(x), which is tricky because our FTOI truncates (rounds to
 * zero).
 */
static struct qreg
ntq_ffloor(struct vc4_compile *c, struct qreg src)
{
        struct qreg trunc = qir_ITOF(c, qir_FTOI(c, src));

        /* This will be < 0 if we truncated and the truncation was of a value
         * that was < 0 in the first place.
         */
        qir_SF(c, qir_FSUB(c, src, trunc));

        return qir_SEL_X_Y_NS(c,
                              qir_FSUB(c, trunc, qir_uniform_f(c, 1.0)),
                              trunc);
}

/**
 * Computes ceil(x), which is tricky because our FTOI truncates (rounds to
 * zero).
 */
static struct qreg
ntq_fceil(struct vc4_compile *c, struct qreg src)
{
        struct qreg trunc = qir_ITOF(c, qir_FTOI(c, src));

        /* This will be < 0 if we truncated and the truncation was of a value
         * that was > 0 in the first place.
         */
        qir_SF(c, qir_FSUB(c, trunc, src));

        return qir_SEL_X_Y_NS(c,
                              qir_FADD(c, trunc, qir_uniform_f(c, 1.0)),
                              trunc);
}

static struct qreg
ntq_fsin(struct vc4_compile *c, struct qreg src)
{
        float coeff[] = {
                -2.0 * M_PI,
                pow(2.0 * M_PI, 3) / (3 * 2 * 1),
                -pow(2.0 * M_PI, 5) / (5 * 4 * 3 * 2 * 1),
                pow(2.0 * M_PI, 7) / (7 * 6 * 5 * 4 * 3 * 2 * 1),
                -pow(2.0 * M_PI, 9) / (9 * 8 * 7 * 6 * 5 * 4 * 3 * 2 * 1),
        };

        struct qreg scaled_x =
                qir_FMUL(c,
                         src,
                         qir_uniform_f(c, 1.0f / (M_PI * 2.0f)));

        struct qreg x = qir_FADD(c,
                                 ntq_ffract(c, scaled_x),
                                 qir_uniform_f(c, -0.5));
        struct qreg x2 = qir_FMUL(c, x, x);
        struct qreg sum = qir_FMUL(c, x, qir_uniform_f(c, coeff[0]));
        for (int i = 1; i < ARRAY_SIZE(coeff); i++) {
                x = qir_FMUL(c, x, x2);
                sum = qir_FADD(c,
                               sum,
                               qir_FMUL(c,
                                        x,
                                        qir_uniform_f(c, coeff[i])));
        }
        return sum;
}

static struct qreg
ntq_fcos(struct vc4_compile *c, struct qreg src)
{
        float coeff[] = {
                -1.0f,
                pow(2.0 * M_PI, 2) / (2 * 1),
                -pow(2.0 * M_PI, 4) / (4 * 3 * 2 * 1),
                pow(2.0 * M_PI, 6) / (6 * 5 * 4 * 3 * 2 * 1),
                -pow(2.0 * M_PI, 8) / (8 * 7 * 6 * 5 * 4 * 3 * 2 * 1),
                pow(2.0 * M_PI, 10) / (10 * 9 * 8 * 7 * 6 * 5 * 4 * 3 * 2 * 1),
        };

        struct qreg scaled_x =
                qir_FMUL(c, src,
                         qir_uniform_f(c, 1.0f / (M_PI * 2.0f)));
        struct qreg x_frac = qir_FADD(c,
                                      ntq_ffract(c, scaled_x),
                                      qir_uniform_f(c, -0.5));

        struct qreg sum = qir_uniform_f(c, coeff[0]);
        struct qreg x2 = qir_FMUL(c, x_frac, x_frac);
        struct qreg x = x2; /* Current x^2, x^4, or x^6 */
        for (int i = 1; i < ARRAY_SIZE(coeff); i++) {
                if (i != 1)
                        x = qir_FMUL(c, x, x2);

                struct qreg mul = qir_FMUL(c,
                                           x,
                                           qir_uniform_f(c, coeff[i]));
                if (i == 0)
                        sum = mul;
                else
                        sum = qir_FADD(c, sum, mul);
        }
        return sum;
}

static struct qreg
ntq_fsign(struct vc4_compile *c, struct qreg src)
{
        qir_SF(c, src);
        return qir_SEL_X_Y_NC(c,
                              qir_SEL_X_0_ZC(c, qir_uniform_f(c, 1.0)),
                              qir_uniform_f(c, -1.0));
}

static struct qreg
get_channel_from_vpm(struct vc4_compile *c,
                     struct qreg *vpm_reads,
                     uint8_t swiz,
                     const struct util_format_description *desc)
{
        const struct util_format_channel_description *chan =
                &desc->channel[swiz];
        struct qreg temp;

        if (swiz > UTIL_FORMAT_SWIZZLE_W)
                return get_swizzled_channel(c, vpm_reads, swiz);
        else if (chan->size == 32 &&
                 chan->type == UTIL_FORMAT_TYPE_FLOAT) {
                return get_swizzled_channel(c, vpm_reads, swiz);
        } else if (chan->size == 32 &&
                   chan->type == UTIL_FORMAT_TYPE_SIGNED) {
                if (chan->normalized) {
                        return qir_FMUL(c,
                                        qir_ITOF(c, vpm_reads[swiz]),
                                        qir_uniform_f(c,
                                                      1.0 / 0x7fffffff));
                } else {
                        return qir_ITOF(c, vpm_reads[swiz]);
                }
        } else if (chan->size == 8 &&
                   (chan->type == UTIL_FORMAT_TYPE_UNSIGNED ||
                    chan->type == UTIL_FORMAT_TYPE_SIGNED)) {
                struct qreg vpm = vpm_reads[0];
                if (chan->type == UTIL_FORMAT_TYPE_SIGNED) {
                        temp = qir_XOR(c, vpm, qir_uniform_ui(c, 0x80808080));
                        if (chan->normalized) {
                                return qir_FSUB(c, qir_FMUL(c,
                                                            qir_UNPACK_8_F(c, temp, swiz),
                                                            qir_uniform_f(c, 2.0)),
                                                qir_uniform_f(c, 1.0));
                        } else {
                                return qir_FADD(c,
                                                qir_ITOF(c,
                                                         qir_UNPACK_8_I(c, temp,
                                                                        swiz)),
                                                qir_uniform_f(c, -128.0));
                        }
                } else {
                        if (chan->normalized) {
                                return qir_UNPACK_8_F(c, vpm, swiz);
                        } else {
                                return qir_ITOF(c, qir_UNPACK_8_I(c, vpm, swiz));
                        }
                }
        } else if (chan->size == 16 &&
                   (chan->type == UTIL_FORMAT_TYPE_UNSIGNED ||
                    chan->type == UTIL_FORMAT_TYPE_SIGNED)) {
                struct qreg vpm = vpm_reads[swiz / 2];

                /* Note that UNPACK_16F eats a half float, not ints, so we use
                 * UNPACK_16_I for all of these.
                 */
                if (chan->type == UTIL_FORMAT_TYPE_SIGNED) {
                        temp = qir_ITOF(c, qir_UNPACK_16_I(c, vpm, swiz % 2));
                        if (chan->normalized) {
                                return qir_FMUL(c, temp,
                                                qir_uniform_f(c, 1/32768.0f));
                        } else {
                                return temp;
                        }
                } else {
                        /* UNPACK_16I sign-extends, so we have to emit ANDs. */
                        temp = vpm;
                        if (swiz == 1 || swiz == 3)
                                temp = qir_UNPACK_16_I(c, temp, 1);
                        temp = qir_AND(c, temp, qir_uniform_ui(c, 0xffff));
                        temp = qir_ITOF(c, temp);

                        if (chan->normalized) {
                                return qir_FMUL(c, temp,
                                                qir_uniform_f(c, 1 / 65535.0));
                        } else {
                                return temp;
                        }
                }
        } else {
                return c->undef;
        }
}

static void
emit_vertex_input(struct vc4_compile *c, int attr)
{
        enum pipe_format format = c->vs_key->attr_formats[attr];
        uint32_t attr_size = util_format_get_blocksize(format);
        struct qreg vpm_reads[4];

        c->vattr_sizes[attr] = align(attr_size, 4);
        for (int i = 0; i < align(attr_size, 4) / 4; i++) {
                struct qreg vpm = { QFILE_VPM, attr * 4 + i };
                vpm_reads[i] = qir_MOV(c, vpm);
                c->num_inputs++;
        }

        bool format_warned = false;
        const struct util_format_description *desc =
                util_format_description(format);

        for (int i = 0; i < 4; i++) {
                uint8_t swiz = desc->swizzle[i];
                struct qreg result = get_channel_from_vpm(c, vpm_reads,
                                                          swiz, desc);

                if (result.file == QFILE_NULL) {
                        if (!format_warned) {
                                fprintf(stderr,
                                        "vtx element %d unsupported type: %s\n",
                                        attr, util_format_name(format));
                                format_warned = true;
                        }
                        result = qir_uniform_f(c, 0.0);
                }
                c->inputs[attr * 4 + i] = result;
        }
}

static void
emit_fragcoord_input(struct vc4_compile *c, int attr)
{
        c->inputs[attr * 4 + 0] = qir_FRAG_X(c);
        c->inputs[attr * 4 + 1] = qir_FRAG_Y(c);
        c->inputs[attr * 4 + 2] =
                qir_FMUL(c,
                         qir_ITOF(c, qir_FRAG_Z(c)),
                         qir_uniform_f(c, 1.0 / 0xffffff));
        c->inputs[attr * 4 + 3] = qir_RCP(c, qir_FRAG_W(c));
}

static void
emit_point_coord_input(struct vc4_compile *c, int attr)
{
        if (c->point_x.file == QFILE_NULL) {
                c->point_x = qir_uniform_f(c, 0.0);
                c->point_y = qir_uniform_f(c, 0.0);
        }

        c->inputs[attr * 4 + 0] = c->point_x;
        if (c->fs_key->point_coord_upper_left) {
                c->inputs[attr * 4 + 1] = qir_FSUB(c,
                                                   qir_uniform_f(c, 1.0),
                                                   c->point_y);
        } else {
                c->inputs[attr * 4 + 1] = c->point_y;
        }
        c->inputs[attr * 4 + 2] = qir_uniform_f(c, 0.0);
        c->inputs[attr * 4 + 3] = qir_uniform_f(c, 1.0);
}

static struct qreg
emit_fragment_varying(struct vc4_compile *c, uint8_t semantic,
                      uint8_t index, uint8_t swizzle)
{
        uint32_t i = c->num_input_semantics++;
        struct qreg vary = {
                QFILE_VARY,
                i
        };

        if (c->num_input_semantics >= c->input_semantics_array_size) {
                c->input_semantics_array_size =
                        MAX2(4, c->input_semantics_array_size * 2);

                c->input_semantics = reralloc(c, c->input_semantics,
                                              struct vc4_varying_semantic,
                                              c->input_semantics_array_size);
        }

        c->input_semantics[i].semantic = semantic;
        c->input_semantics[i].index = index;
        c->input_semantics[i].swizzle = swizzle;

        return qir_VARY_ADD_C(c, qir_FMUL(c, vary, qir_FRAG_W(c)));
}

static void
emit_fragment_input(struct vc4_compile *c, int attr,
                    unsigned semantic_name, unsigned semantic_index)
{
        for (int i = 0; i < 4; i++) {
                c->inputs[attr * 4 + i] =
                        emit_fragment_varying(c,
                                              semantic_name,
                                              semantic_index,
                                              i);
                c->num_inputs++;
        }
}

static void
emit_face_input(struct vc4_compile *c, int attr)
{
        c->inputs[attr * 4 + 0] = qir_FSUB(c,
                                           qir_uniform_f(c, 1.0),
                                           qir_FMUL(c,
                                                    qir_ITOF(c, qir_FRAG_REV_FLAG(c)),
                                                    qir_uniform_f(c, 2.0)));
        c->inputs[attr * 4 + 1] = qir_uniform_f(c, 0.0);
        c->inputs[attr * 4 + 2] = qir_uniform_f(c, 0.0);
        c->inputs[attr * 4 + 3] = qir_uniform_f(c, 1.0);
}

static void
add_output(struct vc4_compile *c,
           uint32_t decl_offset,
           uint8_t semantic_name,
           uint8_t semantic_index,
           uint8_t semantic_swizzle)
{
        uint32_t old_array_size = c->outputs_array_size;
        resize_qreg_array(c, &c->outputs, &c->outputs_array_size,
                          decl_offset + 1);

        if (old_array_size != c->outputs_array_size) {
                c->output_semantics = reralloc(c,
                                               c->output_semantics,
                                               struct vc4_varying_semantic,
                                               c->outputs_array_size);
        }

        c->output_semantics[decl_offset].semantic = semantic_name;
        c->output_semantics[decl_offset].index = semantic_index;
        c->output_semantics[decl_offset].swizzle = semantic_swizzle;
}

static void
declare_uniform_range(struct vc4_compile *c, uint32_t start, uint32_t size)
{
        unsigned array_id = c->num_uniform_ranges++;
        if (array_id >= c->ubo_ranges_array_size) {
                c->ubo_ranges_array_size = MAX2(c->ubo_ranges_array_size * 2,
                                                array_id + 1);
                c->ubo_ranges = reralloc(c, c->ubo_ranges,
                                         struct vc4_compiler_ubo_range,
                                         c->ubo_ranges_array_size);
        }

        c->ubo_ranges[array_id].dst_offset = 0;
        c->ubo_ranges[array_id].src_offset = start;
        c->ubo_ranges[array_id].size = size;
        c->ubo_ranges[array_id].used = false;
}

static void
ntq_emit_alu(struct vc4_compile *c, nir_alu_instr *instr)
{
        /* Vectors are special in that they have non-scalarized writemasks,
         * and just take the first swizzle channel for each argument in order
         * into each writemask channel.
         */
        if (instr->op == nir_op_vec2 ||
            instr->op == nir_op_vec3 ||
            instr->op == nir_op_vec4) {
                struct qreg srcs[4];
                for (int i = 0; i < nir_op_infos[instr->op].num_inputs; i++)
                        srcs[i] = ntq_get_src(c, instr->src[i].src,
                                              instr->src[i].swizzle[0]);
                struct qreg *dest = ntq_get_dest(c, instr->dest.dest);
                for (int i = 0; i < nir_op_infos[instr->op].num_inputs; i++)
                        dest[i] = srcs[i];
                return;
        }

        /* General case: We can just grab the one used channel per src. */
        struct qreg src[nir_op_infos[instr->op].num_inputs];
        for (int i = 0; i < nir_op_infos[instr->op].num_inputs; i++) {
                src[i] = ntq_get_alu_src(c, instr, i);
        }

        /* Pick the channel to store the output in. */
        assert(!instr->dest.saturate);
        struct qreg *dest = ntq_get_dest(c, instr->dest.dest);
        assert(util_is_power_of_two(instr->dest.write_mask));
        dest += ffs(instr->dest.write_mask) - 1;

        switch (instr->op) {
        case nir_op_fmov:
        case nir_op_imov:
                *dest = qir_MOV(c, src[0]);
                break;
        case nir_op_fmul:
                *dest = qir_FMUL(c, src[0], src[1]);
                break;
        case nir_op_fadd:
                *dest = qir_FADD(c, src[0], src[1]);
                break;
        case nir_op_fsub:
                *dest = qir_FSUB(c, src[0], src[1]);
                break;
        case nir_op_fmin:
                *dest = qir_FMIN(c, src[0], src[1]);
                break;
        case nir_op_fmax:
                *dest = qir_FMAX(c, src[0], src[1]);
                break;

        case nir_op_f2i:
        case nir_op_f2u:
                *dest = qir_FTOI(c, src[0]);
                break;
        case nir_op_i2f:
        case nir_op_u2f:
                *dest = qir_ITOF(c, src[0]);
                break;
        case nir_op_b2f:
                *dest = qir_AND(c, src[0], qir_uniform_f(c, 1.0));
                break;
        case nir_op_b2i:
                *dest = qir_AND(c, src[0], qir_uniform_ui(c, 1));
                break;
        case nir_op_i2b:
        case nir_op_f2b:
                qir_SF(c, src[0]);
                *dest = qir_SEL_X_0_ZC(c, qir_uniform_ui(c, ~0));
                break;

        case nir_op_iadd:
                *dest = qir_ADD(c, src[0], src[1]);
                break;
        case nir_op_ushr:
                *dest = qir_SHR(c, src[0], src[1]);
                break;
        case nir_op_isub:
                *dest = qir_SUB(c, src[0], src[1]);
                break;
        case nir_op_ishr:
                *dest = qir_ASR(c, src[0], src[1]);
                break;
        case nir_op_ishl:
                *dest = qir_SHL(c, src[0], src[1]);
                break;
        case nir_op_imin:
                *dest = qir_MIN(c, src[0], src[1]);
                break;
        case nir_op_imax:
                *dest = qir_MAX(c, src[0], src[1]);
                break;
        case nir_op_iand:
                *dest = qir_AND(c, src[0], src[1]);
                break;
        case nir_op_ior:
                *dest = qir_OR(c, src[0], src[1]);
                break;
        case nir_op_ixor:
                *dest = qir_XOR(c, src[0], src[1]);
                break;
        case nir_op_inot:
                *dest = qir_NOT(c, src[0]);
                break;

        case nir_op_imul:
                *dest = ntq_umul(c, src[0], src[1]);
                break;

        case nir_op_seq:
                qir_SF(c, qir_FSUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_ZS(c, qir_uniform_f(c, 1.0));
                break;
        case nir_op_sne:
                qir_SF(c, qir_FSUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_ZC(c, qir_uniform_f(c, 1.0));
                break;
        case nir_op_sge:
                qir_SF(c, qir_FSUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_NC(c, qir_uniform_f(c, 1.0));
                break;
        case nir_op_slt:
                qir_SF(c, qir_FSUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_NS(c, qir_uniform_f(c, 1.0));
                break;
        case nir_op_feq:
                qir_SF(c, qir_FSUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_ZS(c, qir_uniform_ui(c, ~0));
                break;
        case nir_op_fne:
                qir_SF(c, qir_FSUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_ZC(c, qir_uniform_ui(c, ~0));
                break;
        case nir_op_fge:
                qir_SF(c, qir_FSUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_NC(c, qir_uniform_ui(c, ~0));
                break;
        case nir_op_flt:
                qir_SF(c, qir_FSUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_NS(c, qir_uniform_ui(c, ~0));
                break;
        case nir_op_ieq:
                qir_SF(c, qir_SUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_ZS(c, qir_uniform_ui(c, ~0));
                break;
        case nir_op_ine:
                qir_SF(c, qir_SUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_ZC(c, qir_uniform_ui(c, ~0));
                break;
        case nir_op_ige:
                qir_SF(c, qir_SUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_NC(c, qir_uniform_ui(c, ~0));
                break;
        case nir_op_ilt:
                qir_SF(c, qir_SUB(c, src[0], src[1]));
                *dest = qir_SEL_X_0_NS(c, qir_uniform_ui(c, ~0));
                break;

        case nir_op_bcsel:
                qir_SF(c, src[0]);
                *dest = qir_SEL_X_Y_NS(c, src[1], src[2]);
                break;
        case nir_op_fcsel:
                qir_SF(c, src[0]);
                *dest = qir_SEL_X_Y_ZC(c, src[1], src[2]);
                break;

        case nir_op_frcp:
                *dest = ntq_rcp(c, src[0]);
                break;
        case nir_op_frsq:
                *dest = ntq_rsq(c, src[0]);
                break;
        case nir_op_fexp2:
                *dest = qir_EXP2(c, src[0]);
                break;
        case nir_op_flog2:
                *dest = qir_LOG2(c, src[0]);
                break;

        case nir_op_ftrunc:
                *dest = qir_ITOF(c, qir_FTOI(c, src[0]));
                break;
        case nir_op_fceil:
                *dest = ntq_fceil(c, src[0]);
                break;
        case nir_op_ffract:
                *dest = ntq_ffract(c, src[0]);
                break;
        case nir_op_ffloor:
                *dest = ntq_ffloor(c, src[0]);
                break;

        case nir_op_fsin:
                *dest = ntq_fsin(c, src[0]);
                break;
        case nir_op_fcos:
                *dest = ntq_fcos(c, src[0]);
                break;

        case nir_op_fsign:
                *dest = ntq_fsign(c, src[0]);
                break;

        case nir_op_fabs:
                *dest = qir_FMAXABS(c, src[0], src[0]);
                break;
        case nir_op_iabs:
                *dest = qir_MAX(c, src[0],
                                qir_SUB(c, qir_uniform_ui(c, 0), src[0]));
                break;

        default:
                fprintf(stderr, "unknown NIR ALU inst: ");
                nir_print_instr(&instr->instr, stderr);
                fprintf(stderr, "\n");
                abort();
        }
}

static struct qreg
vc4_blend_channel(struct vc4_compile *c,
                  struct qreg *dst,
                  struct qreg *src,
                  struct qreg val,
                  unsigned factor,
                  int channel)
{
        switch(factor) {
        case PIPE_BLENDFACTOR_ONE:
                return val;
        case PIPE_BLENDFACTOR_SRC_COLOR:
                return qir_FMUL(c, val, src[channel]);
        case PIPE_BLENDFACTOR_SRC_ALPHA:
                return qir_FMUL(c, val, src[3]);
        case PIPE_BLENDFACTOR_DST_ALPHA:
                return qir_FMUL(c, val, dst[3]);
        case PIPE_BLENDFACTOR_DST_COLOR:
                return qir_FMUL(c, val, dst[channel]);
        case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
                if (channel != 3) {
                        return qir_FMUL(c,
                                        val,
                                        qir_FMIN(c,
                                                 src[3],
                                                 qir_FSUB(c,
                                                          qir_uniform_f(c, 1.0),
                                                          dst[3])));
                } else {
                        return val;
                }
        case PIPE_BLENDFACTOR_CONST_COLOR:
                return qir_FMUL(c, val,
                                qir_uniform(c, QUNIFORM_BLEND_CONST_COLOR,
                                            channel));
        case PIPE_BLENDFACTOR_CONST_ALPHA:
                return qir_FMUL(c, val,
                                qir_uniform(c, QUNIFORM_BLEND_CONST_COLOR, 3));
        case PIPE_BLENDFACTOR_ZERO:
                return qir_uniform_f(c, 0.0);
        case PIPE_BLENDFACTOR_INV_SRC_COLOR:
                return qir_FMUL(c, val, qir_FSUB(c, qir_uniform_f(c, 1.0),
                                                 src[channel]));
        case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
                return qir_FMUL(c, val, qir_FSUB(c, qir_uniform_f(c, 1.0),
                                                 src[3]));
        case PIPE_BLENDFACTOR_INV_DST_ALPHA:
                return qir_FMUL(c, val, qir_FSUB(c, qir_uniform_f(c, 1.0),
                                                 dst[3]));
        case PIPE_BLENDFACTOR_INV_DST_COLOR:
                return qir_FMUL(c, val, qir_FSUB(c, qir_uniform_f(c, 1.0),
                                                 dst[channel]));
        case PIPE_BLENDFACTOR_INV_CONST_COLOR:
                return qir_FMUL(c, val,
                                qir_FSUB(c, qir_uniform_f(c, 1.0),
                                         qir_uniform(c,
                                                     QUNIFORM_BLEND_CONST_COLOR,
                                                     channel)));
        case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
                return qir_FMUL(c, val,
                                qir_FSUB(c, qir_uniform_f(c, 1.0),
                                         qir_uniform(c,
                                                     QUNIFORM_BLEND_CONST_COLOR,
                                                     3)));

        default:
        case PIPE_BLENDFACTOR_SRC1_COLOR:
        case PIPE_BLENDFACTOR_SRC1_ALPHA:
        case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
        case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
                /* Unsupported. */
                fprintf(stderr, "Unknown blend factor %d\n", factor);
                return val;
        }
}

static struct qreg
vc4_blend_func(struct vc4_compile *c,
               struct qreg src, struct qreg dst,
               unsigned func)
{
        switch (func) {
        case PIPE_BLEND_ADD:
                return qir_FADD(c, src, dst);
        case PIPE_BLEND_SUBTRACT:
                return qir_FSUB(c, src, dst);
        case PIPE_BLEND_REVERSE_SUBTRACT:
                return qir_FSUB(c, dst, src);
        case PIPE_BLEND_MIN:
                return qir_FMIN(c, src, dst);
        case PIPE_BLEND_MAX:
                return qir_FMAX(c, src, dst);

        default:
                /* Unsupported. */
                fprintf(stderr, "Unknown blend func %d\n", func);
                return src;

        }
}

/**
 * Implements fixed function blending in shader code.
 *
 * VC4 doesn't have any hardware support for blending.  Instead, you read the
 * current contents of the destination from the tile buffer after having
 * waited for the scoreboard (which is handled by vc4_qpu_emit.c), then do
 * math using your output color and that destination value, and update the
 * output color appropriately.
 */
static void
vc4_blend(struct vc4_compile *c, struct qreg *result,
          struct qreg *dst_color, struct qreg *src_color)
{
        struct pipe_rt_blend_state *blend = &c->fs_key->blend;

        if (!blend->blend_enable) {
                for (int i = 0; i < 4; i++)
                        result[i] = src_color[i];
                return;
        }

        struct qreg clamped_src[4];
        struct qreg clamped_dst[4];
        for (int i = 0; i < 4; i++) {
                clamped_src[i] = qir_SAT(c, src_color[i]);
                clamped_dst[i] = qir_SAT(c, dst_color[i]);
        }
        src_color = clamped_src;
        dst_color = clamped_dst;

        struct qreg src_blend[4], dst_blend[4];
        for (int i = 0; i < 3; i++) {
                src_blend[i] = vc4_blend_channel(c,
                                                 dst_color, src_color,
                                                 src_color[i],
                                                 blend->rgb_src_factor, i);
                dst_blend[i] = vc4_blend_channel(c,
                                                 dst_color, src_color,
                                                 dst_color[i],
                                                 blend->rgb_dst_factor, i);
        }
        src_blend[3] = vc4_blend_channel(c,
                                         dst_color, src_color,
                                         src_color[3],
                                         blend->alpha_src_factor, 3);
        dst_blend[3] = vc4_blend_channel(c,
                                         dst_color, src_color,
                                         dst_color[3],
                                         blend->alpha_dst_factor, 3);

        for (int i = 0; i < 3; i++) {
                result[i] = vc4_blend_func(c,
                                           src_blend[i], dst_blend[i],
                                           blend->rgb_func);
        }
        result[3] = vc4_blend_func(c,
                                   src_blend[3], dst_blend[3],
                                   blend->alpha_func);
}

static void
clip_distance_discard(struct vc4_compile *c)
{
        for (int i = 0; i < PIPE_MAX_CLIP_PLANES; i++) {
                if (!(c->key->ucp_enables & (1 << i)))
                        continue;

                struct qreg dist = emit_fragment_varying(c,
                                                         TGSI_SEMANTIC_CLIPDIST,
                                                         i,
                                                         TGSI_SWIZZLE_X);

                qir_SF(c, dist);

                if (c->discard.file == QFILE_NULL)
                        c->discard = qir_uniform_ui(c, 0);

                c->discard = qir_SEL_X_Y_NS(c, qir_uniform_ui(c, ~0),
                                            c->discard);
        }
}

static void
alpha_test_discard(struct vc4_compile *c)
{
        struct qreg src_alpha;
        struct qreg alpha_ref = qir_uniform(c, QUNIFORM_ALPHA_REF, 0);

        if (!c->fs_key->alpha_test)
                return;

        if (c->output_color_index != -1)
                src_alpha = c->outputs[c->output_color_index + 3];
        else
                src_alpha = qir_uniform_f(c, 1.0);

        if (c->discard.file == QFILE_NULL)
                c->discard = qir_uniform_ui(c, 0);

        switch (c->fs_key->alpha_test_func) {
        case PIPE_FUNC_NEVER:
                c->discard = qir_uniform_ui(c, ~0);
                break;
        case PIPE_FUNC_ALWAYS:
                break;
        case PIPE_FUNC_EQUAL:
                qir_SF(c, qir_FSUB(c, src_alpha, alpha_ref));
                c->discard = qir_SEL_X_Y_ZS(c, c->discard,
                                            qir_uniform_ui(c, ~0));
                break;
        case PIPE_FUNC_NOTEQUAL:
                qir_SF(c, qir_FSUB(c, src_alpha, alpha_ref));
                c->discard = qir_SEL_X_Y_ZC(c, c->discard,
                                            qir_uniform_ui(c, ~0));
                break;
        case PIPE_FUNC_GREATER:
                qir_SF(c, qir_FSUB(c, src_alpha, alpha_ref));
                c->discard = qir_SEL_X_Y_NC(c, c->discard,
                                            qir_uniform_ui(c, ~0));
                break;
        case PIPE_FUNC_GEQUAL:
                qir_SF(c, qir_FSUB(c, alpha_ref, src_alpha));
                c->discard = qir_SEL_X_Y_NS(c, c->discard,
                                            qir_uniform_ui(c, ~0));
                break;
        case PIPE_FUNC_LESS:
                qir_SF(c, qir_FSUB(c, src_alpha, alpha_ref));
                c->discard = qir_SEL_X_Y_NS(c, c->discard,
                                            qir_uniform_ui(c, ~0));
                break;
        case PIPE_FUNC_LEQUAL:
                qir_SF(c, qir_FSUB(c, alpha_ref, src_alpha));
                c->discard = qir_SEL_X_Y_NC(c, c->discard,
                                            qir_uniform_ui(c, ~0));
                break;
        }
}

static struct qreg
vc4_logicop(struct vc4_compile *c, struct qreg src, struct qreg dst)
{
        switch (c->fs_key->logicop_func) {
        case PIPE_LOGICOP_CLEAR:
                return qir_uniform_f(c, 0.0);
        case PIPE_LOGICOP_NOR:
                return qir_NOT(c, qir_OR(c, src, dst));
        case PIPE_LOGICOP_AND_INVERTED:
                return qir_AND(c, qir_NOT(c, src), dst);
        case PIPE_LOGICOP_COPY_INVERTED:
                return qir_NOT(c, src);
        case PIPE_LOGICOP_AND_REVERSE:
                return qir_AND(c, src, qir_NOT(c, dst));
        case PIPE_LOGICOP_INVERT:
                return qir_NOT(c, dst);
        case PIPE_LOGICOP_XOR:
                return qir_XOR(c, src, dst);
        case PIPE_LOGICOP_NAND:
                return qir_NOT(c, qir_AND(c, src, dst));
        case PIPE_LOGICOP_AND:
                return qir_AND(c, src, dst);
        case PIPE_LOGICOP_EQUIV:
                return qir_NOT(c, qir_XOR(c, src, dst));
        case PIPE_LOGICOP_NOOP:
                return dst;
        case PIPE_LOGICOP_OR_INVERTED:
                return qir_OR(c, qir_NOT(c, src), dst);
        case PIPE_LOGICOP_OR_REVERSE:
                return qir_OR(c, src, qir_NOT(c, dst));
        case PIPE_LOGICOP_OR:
                return qir_OR(c, src, dst);
        case PIPE_LOGICOP_SET:
                return qir_uniform_ui(c, ~0);
        case PIPE_LOGICOP_COPY:
        default:
                return src;
        }
}

static void
emit_frag_end(struct vc4_compile *c)
{
        clip_distance_discard(c);
        alpha_test_discard(c);

        enum pipe_format color_format = c->fs_key->color_format;
        const uint8_t *format_swiz = vc4_get_format_swizzle(color_format);
        struct qreg tlb_read_color[4] = { c->undef, c->undef, c->undef, c->undef };
        struct qreg dst_color[4] = { c->undef, c->undef, c->undef, c->undef };
        struct qreg linear_dst_color[4] = { c->undef, c->undef, c->undef, c->undef };
        struct qreg packed_dst_color = c->undef;

        if (c->fs_key->blend.blend_enable ||
            c->fs_key->blend.colormask != 0xf ||
            c->fs_key->logicop_func != PIPE_LOGICOP_COPY) {
                struct qreg r4 = qir_TLB_COLOR_READ(c);
                for (int i = 0; i < 4; i++)
                        tlb_read_color[i] = qir_R4_UNPACK(c, r4, i);
                for (int i = 0; i < 4; i++) {
                        dst_color[i] = get_swizzled_channel(c,
                                                            tlb_read_color,
                                                            format_swiz[i]);
                        if (util_format_is_srgb(color_format) && i != 3) {
                                linear_dst_color[i] =
                                        qir_srgb_decode(c, dst_color[i]);
                        } else {
                                linear_dst_color[i] = dst_color[i];
                        }
                }

                /* Save the packed value for logic ops.  Can't reuse r4
                 * because other things might smash it (like sRGB)
                 */
                packed_dst_color = qir_MOV(c, r4);
        }

        struct qreg blend_color[4];
        struct qreg undef_array[4] = {
                c->undef, c->undef, c->undef, c->undef
        };
        vc4_blend(c, blend_color, linear_dst_color,
                  (c->output_color_index != -1 ?
                   c->outputs + c->output_color_index :
                   undef_array));

        if (util_format_is_srgb(color_format)) {
                for (int i = 0; i < 3; i++)
                        blend_color[i] = qir_srgb_encode(c, blend_color[i]);
        }

        /* Debug: Sometimes you're getting a black output and just want to see
         * if the FS is getting executed at all.  Spam magenta into the color
         * output.
         */
        if (0) {
                blend_color[0] = qir_uniform_f(c, 1.0);
                blend_color[1] = qir_uniform_f(c, 0.0);
                blend_color[2] = qir_uniform_f(c, 1.0);
                blend_color[3] = qir_uniform_f(c, 0.5);
        }

        struct qreg swizzled_outputs[4];
        for (int i = 0; i < 4; i++) {
                swizzled_outputs[i] = get_swizzled_channel(c, blend_color,
                                                           format_swiz[i]);
        }

        if (c->discard.file != QFILE_NULL)
                qir_TLB_DISCARD_SETUP(c, c->discard);

        if (c->fs_key->stencil_enabled) {
                qir_TLB_STENCIL_SETUP(c, qir_uniform(c, QUNIFORM_STENCIL, 0));
                if (c->fs_key->stencil_twoside) {
                        qir_TLB_STENCIL_SETUP(c, qir_uniform(c, QUNIFORM_STENCIL, 1));
                }
                if (c->fs_key->stencil_full_writemasks) {
                        qir_TLB_STENCIL_SETUP(c, qir_uniform(c, QUNIFORM_STENCIL, 2));
                }
        }

        if (c->fs_key->depth_enabled) {
                struct qreg z;
                if (c->output_position_index != -1) {
                        z = qir_FTOI(c, qir_FMUL(c, c->outputs[c->output_position_index + 2],
                                                 qir_uniform_f(c, 0xffffff)));
                } else {
                        z = qir_FRAG_Z(c);
                }
                qir_TLB_Z_WRITE(c, z);
        }

        struct qreg packed_color = c->undef;
        for (int i = 0; i < 4; i++) {
                if (swizzled_outputs[i].file == QFILE_NULL)
                        continue;
                if (packed_color.file == QFILE_NULL) {
                        packed_color = qir_PACK_8888_F(c, swizzled_outputs[i]);
                } else {
                        packed_color = qir_PACK_8_F(c,
                                                    packed_color,
                                                    swizzled_outputs[i],
                                                    i);
                }
        }

        if (packed_color.file == QFILE_NULL)
                packed_color = qir_uniform_ui(c, 0);

        if (c->fs_key->logicop_func != PIPE_LOGICOP_COPY) {
                packed_color = vc4_logicop(c, packed_color, packed_dst_color);
        }

        /* If the bit isn't set in the color mask, then just return the
         * original dst color, instead.
         */
        uint32_t colormask = 0xffffffff;
        for (int i = 0; i < 4; i++) {
                if (format_swiz[i] < 4 &&
                    !(c->fs_key->blend.colormask & (1 << format_swiz[i]))) {
                        colormask &= ~(0xff << (i * 8));
                }
        }
        if (colormask != 0xffffffff) {
                packed_color = qir_OR(c,
                                      qir_AND(c, packed_color,
                                              qir_uniform_ui(c, colormask)),
                                      qir_AND(c, packed_dst_color,
                                              qir_uniform_ui(c, ~colormask)));
        }

        qir_emit(c, qir_inst(QOP_TLB_COLOR_WRITE, c->undef,
                             packed_color, c->undef));
}

static void
emit_scaled_viewport_write(struct vc4_compile *c, struct qreg rcp_w)
{
        struct qreg xyi[2];

        for (int i = 0; i < 2; i++) {
                struct qreg scale =
                        qir_uniform(c, QUNIFORM_VIEWPORT_X_SCALE + i, 0);

                xyi[i] = qir_FTOI(c, qir_FMUL(c,
                                              qir_FMUL(c,
                                                       c->outputs[c->output_position_index + i],
                                                       scale),
                                              rcp_w));
        }

        qir_VPM_WRITE(c, qir_PACK_SCALED(c, xyi[0], xyi[1]));
}

static void
emit_zs_write(struct vc4_compile *c, struct qreg rcp_w)
{
        struct qreg zscale = qir_uniform(c, QUNIFORM_VIEWPORT_Z_SCALE, 0);
        struct qreg zoffset = qir_uniform(c, QUNIFORM_VIEWPORT_Z_OFFSET, 0);

        qir_VPM_WRITE(c, qir_FADD(c, qir_FMUL(c, qir_FMUL(c,
                                                          c->outputs[c->output_position_index + 2],
                                                          zscale),
                                              rcp_w),
                                  zoffset));
}

static void
emit_rcp_wc_write(struct vc4_compile *c, struct qreg rcp_w)
{
        qir_VPM_WRITE(c, rcp_w);
}

static void
emit_point_size_write(struct vc4_compile *c)
{
        struct qreg point_size;

        if (c->output_point_size_index != -1)
                point_size = c->outputs[c->output_point_size_index + 3];
        else
                point_size = qir_uniform_f(c, 1.0);

        /* Workaround: HW-2726 PTB does not handle zero-size points (BCM2835,
         * BCM21553).
         */
        point_size = qir_FMAX(c, point_size, qir_uniform_f(c, .125));

        qir_VPM_WRITE(c, point_size);
}

/**
 * Emits a VPM read of the stub vertex attribute set up by vc4_draw.c.
 *
 * The simulator insists that there be at least one vertex attribute, so
 * vc4_draw.c will emit one if it wouldn't have otherwise.  The simulator also
 * insists that all vertex attributes loaded get read by the VS/CS, so we have
 * to consume it here.
 */
static void
emit_stub_vpm_read(struct vc4_compile *c)
{
        if (c->num_inputs)
                return;

        c->vattr_sizes[0] = 4;
        struct qreg vpm = { QFILE_VPM, 0 };
        (void)qir_MOV(c, vpm);
        c->num_inputs++;
}

static void
emit_ucp_clipdistance(struct vc4_compile *c)
{
        unsigned cv;
        if (c->output_clipvertex_index != -1)
                cv = c->output_clipvertex_index;
        else if (c->output_position_index != -1)
                cv = c->output_position_index;
        else
                return;

        for (int plane = 0; plane < PIPE_MAX_CLIP_PLANES; plane++) {
                if (!(c->key->ucp_enables & (1 << plane)))
                        continue;

                /* Pick the next outputs[] that hasn't been written to, since
                 * there are no other program writes left to be processed at
                 * this point.  If something had been declared but not written
                 * (like a w component), we'll just smash over the top of it.
                 */
                uint32_t output_index = c->num_outputs++;
                add_output(c, output_index,
                           TGSI_SEMANTIC_CLIPDIST,
                           plane,
                           TGSI_SWIZZLE_X);


                struct qreg dist = qir_uniform_f(c, 0.0);
                for (int i = 0; i < 4; i++) {
                        struct qreg pos_chan = c->outputs[cv + i];
                        struct qreg ucp =
                                qir_uniform(c, QUNIFORM_USER_CLIP_PLANE,
                                            plane * 4 + i);
                        dist = qir_FADD(c, dist, qir_FMUL(c, pos_chan, ucp));
                }

                c->outputs[output_index] = dist;
        }
}

static void
emit_vert_end(struct vc4_compile *c,
              struct vc4_varying_semantic *fs_inputs,
              uint32_t num_fs_inputs)
{
        struct qreg rcp_w = qir_RCP(c, c->outputs[c->output_position_index + 3]);

        emit_stub_vpm_read(c);
        emit_ucp_clipdistance(c);

        emit_scaled_viewport_write(c, rcp_w);
        emit_zs_write(c, rcp_w);
        emit_rcp_wc_write(c, rcp_w);
        if (c->vs_key->per_vertex_point_size)
                emit_point_size_write(c);

        for (int i = 0; i < num_fs_inputs; i++) {
                struct vc4_varying_semantic *input = &fs_inputs[i];
                int j;

                for (j = 0; j < c->num_outputs; j++) {
                        struct vc4_varying_semantic *output =
                                &c->output_semantics[j];

                        if (input->semantic == output->semantic &&
                            input->index == output->index &&
                            input->swizzle == output->swizzle) {
                                qir_VPM_WRITE(c, c->outputs[j]);
                                break;
                        }
                }
                /* Emit padding if we didn't find a declared VS output for
                 * this FS input.
                 */
                if (j == c->num_outputs)
                        qir_VPM_WRITE(c, qir_uniform_f(c, 0.0));
        }
}

static void
emit_coord_end(struct vc4_compile *c)
{
        struct qreg rcp_w = qir_RCP(c, c->outputs[c->output_position_index + 3]);

        emit_stub_vpm_read(c);

        for (int i = 0; i < 4; i++)
                qir_VPM_WRITE(c, c->outputs[c->output_position_index + i]);

        emit_scaled_viewport_write(c, rcp_w);
        emit_zs_write(c, rcp_w);
        emit_rcp_wc_write(c, rcp_w);
        if (c->vs_key->per_vertex_point_size)
                emit_point_size_write(c);
}

static void
vc4_optimize_nir(struct nir_shader *s)
{
        bool progress;

        do {
                progress = false;

                nir_lower_vars_to_ssa(s);
                nir_lower_alu_to_scalar(s);

                progress = nir_copy_prop(s) || progress;
                progress = nir_opt_dce(s) || progress;
                progress = nir_opt_cse(s) || progress;
                progress = nir_opt_peephole_select(s) || progress;
                progress = nir_opt_algebraic(s) || progress;
                progress = nir_opt_constant_folding(s) || progress;
        } while (progress);
}

static int
driver_location_compare(const void *in_a, const void *in_b)
{
        const nir_variable *const *a = in_a;
        const nir_variable *const *b = in_b;

        return (*a)->data.driver_location - (*b)->data.driver_location;
}

static void
ntq_setup_inputs(struct vc4_compile *c)
{
        unsigned num_entries = 0;
        foreach_list_typed(nir_variable, var, node, &c->s->inputs)
                num_entries++;

        nir_variable *vars[num_entries];

        unsigned i = 0;
        foreach_list_typed(nir_variable, var, node, &c->s->inputs)
                vars[i++] = var;

        /* Sort the variables so that we emit the input setup in
         * driver_location order.  This is required for VPM reads, whose data
         * is fetched into the VPM in driver_location (TGSI register index)
         * order.
         */
        qsort(&vars, num_entries, sizeof(*vars), driver_location_compare);

        for (unsigned i = 0; i < num_entries; i++) {
                nir_variable *var = vars[i];
                unsigned array_len = MAX2(glsl_get_length(var->type), 1);
                /* XXX: map loc slots to semantics */
                unsigned semantic_name = var->data.location;
                unsigned semantic_index = var->data.index;
                unsigned loc = var->data.driver_location;

                assert(array_len == 1);
                resize_qreg_array(c, &c->inputs, &c->inputs_array_size,
                                  (loc + 1) * 4);

                if (c->stage == QSTAGE_FRAG) {
                        if (semantic_name == TGSI_SEMANTIC_POSITION) {
                                emit_fragcoord_input(c, loc);
                        } else if (semantic_name == TGSI_SEMANTIC_FACE) {
                                emit_face_input(c, loc);
                        } else if (semantic_name == TGSI_SEMANTIC_GENERIC &&
                                   (c->fs_key->point_sprite_mask &
                                    (1 << semantic_index))) {
                                emit_point_coord_input(c, loc);
                        } else {
                                emit_fragment_input(c, loc,
                                                    semantic_name,
                                                    semantic_index);
                        }
                } else {
                        emit_vertex_input(c, loc);
                }
        }
}

static void
ntq_setup_outputs(struct vc4_compile *c)
{
        foreach_list_typed(nir_variable, var, node, &c->s->outputs) {
                unsigned array_len = MAX2(glsl_get_length(var->type), 1);
                /* XXX: map loc slots to semantics */
                unsigned semantic_name = var->data.location;
                unsigned semantic_index = var->data.index;
                unsigned loc = var->data.driver_location * 4;

                assert(array_len == 1);

                for (int i = 0; i < 4; i++) {
                        add_output(c,
                                   loc + i,
                                   semantic_name,
                                   semantic_index,
                                   i);
                }

                switch (semantic_name) {
                case TGSI_SEMANTIC_POSITION:
                        c->output_position_index = loc;
                        break;
                case TGSI_SEMANTIC_CLIPVERTEX:
                        c->output_clipvertex_index = loc;
                        break;
                case TGSI_SEMANTIC_COLOR:
                        c->output_color_index = loc;
                        break;
                case TGSI_SEMANTIC_PSIZE:
                        c->output_point_size_index = loc;
                        break;
                }

        }
}

static void
ntq_setup_uniforms(struct vc4_compile *c)
{
        foreach_list_typed(nir_variable, var, node, &c->s->uniforms) {
                unsigned array_len = MAX2(glsl_get_length(var->type), 1);
                unsigned array_elem_size = 4 * sizeof(float);

                declare_uniform_range(c, var->data.driver_location * array_elem_size,
                                      array_len * array_elem_size);

        }
}

/**
 * Sets up the mapping from nir_register to struct qreg *.
 *
 * Each nir_register gets a struct qreg per 32-bit component being stored.
 */
static void
ntq_setup_registers(struct vc4_compile *c, struct exec_list *list)
{
        foreach_list_typed(nir_register, nir_reg, node, list) {
                unsigned array_len = MAX2(nir_reg->num_array_elems, 1);
                struct qreg *qregs = ralloc_array(c->def_ht, struct qreg,
                                                  array_len *
                                                  nir_reg->num_components);

                _mesa_hash_table_insert(c->def_ht, nir_reg, qregs);

                for (int i = 0; i < array_len * nir_reg->num_components; i++)
                        qregs[i] = qir_uniform_ui(c, 0);
        }
}

static void
ntq_emit_load_const(struct vc4_compile *c, nir_load_const_instr *instr)
{
        struct qreg *qregs = ralloc_array(c->def_ht, struct qreg,
                                          instr->def.num_components);
        for (int i = 0; i < instr->def.num_components; i++)
                qregs[i] = qir_uniform_ui(c, instr->value.u[i]);

        _mesa_hash_table_insert(c->def_ht, &instr->def, qregs);
}

static void
ntq_emit_intrinsic(struct vc4_compile *c, nir_intrinsic_instr *instr)
{
        const nir_intrinsic_info *info = &nir_intrinsic_infos[instr->intrinsic];
        struct qreg *dest = NULL;

        if (info->has_dest) {
                dest = ntq_get_dest(c, instr->dest);
        }

        switch (instr->intrinsic) {
        case nir_intrinsic_load_uniform:
                assert(instr->const_index[1] == 1);

                for (int i = 0; i < instr->num_components; i++) {
                        dest[i] = qir_uniform(c, QUNIFORM_UNIFORM,
                                              instr->const_index[0] * 4 + i);
                }
                break;

        case nir_intrinsic_load_uniform_indirect:
                assert(instr->const_index[1] == 1);

                for (int i = 0; i < instr->num_components; i++) {
                        dest[i] = indirect_uniform_load(c,
                                                        ntq_get_src(c, instr->src[0], 0),
                                                        (instr->const_index[0] *
                                                         4 + i) * sizeof(float));
                }

                break;

        case nir_intrinsic_load_input:
                assert(instr->const_index[1] == 1);

                for (int i = 0; i < instr->num_components; i++)
                        dest[i] = c->inputs[instr->const_index[0] * 4 + i];

                break;

        case nir_intrinsic_store_output:
                for (int i = 0; i < instr->num_components; i++) {
                        c->outputs[instr->const_index[0] * 4 + i] =
                                qir_MOV(c, ntq_get_src(c, instr->src[0], i));
                }
                c->num_outputs = MAX2(c->num_outputs,
                                      instr->const_index[0] * 4 +
                                      instr->num_components + 1);
                break;

        case nir_intrinsic_discard:
                c->discard = qir_uniform_ui(c, ~0);
                break;

        case nir_intrinsic_discard_if:
                if (c->discard.file == QFILE_NULL)
                        c->discard = qir_uniform_ui(c, 0);
                c->discard = qir_OR(c, c->discard,
                                    ntq_get_src(c, instr->src[0], 0));
                break;

        default:
                fprintf(stderr, "Unknown intrinsic: ");
                nir_print_instr(&instr->instr, stderr);
                fprintf(stderr, "\n");
                break;
        }
}

static void
ntq_emit_if(struct vc4_compile *c, nir_if *if_stmt)
{
        fprintf(stderr, "general IF statements not handled.\n");
}

static void
ntq_emit_instr(struct vc4_compile *c, nir_instr *instr)
{
        switch (instr->type) {
        case nir_instr_type_alu:
                ntq_emit_alu(c, nir_instr_as_alu(instr));
                break;

        case nir_instr_type_intrinsic:
                ntq_emit_intrinsic(c, nir_instr_as_intrinsic(instr));
                break;

        case nir_instr_type_load_const:
                ntq_emit_load_const(c, nir_instr_as_load_const(instr));
                break;

        case nir_instr_type_tex:
                ntq_emit_tex(c, nir_instr_as_tex(instr));
                break;

        default:
                fprintf(stderr, "Unknown NIR instr type: ");
                nir_print_instr(instr, stderr);
                fprintf(stderr, "\n");
                abort();
        }
}

static void
ntq_emit_block(struct vc4_compile *c, nir_block *block)
{
        nir_foreach_instr(block, instr) {
                ntq_emit_instr(c, instr);
        }
}

static void
ntq_emit_cf_list(struct vc4_compile *c, struct exec_list *list)
{
        foreach_list_typed(nir_cf_node, node, node, list) {
                switch (node->type) {
                        /* case nir_cf_node_loop: */
                case nir_cf_node_block:
                        ntq_emit_block(c, nir_cf_node_as_block(node));
                        break;

                case nir_cf_node_if:
                        ntq_emit_if(c, nir_cf_node_as_if(node));
                        break;

                default:
                        assert(0);
                }
        }
}

static void
ntq_emit_impl(struct vc4_compile *c, nir_function_impl *impl)
{
        ntq_setup_registers(c, &impl->registers);
        ntq_emit_cf_list(c, &impl->body);
}

static void
nir_to_qir(struct vc4_compile *c)
{
        ntq_setup_inputs(c);
        ntq_setup_outputs(c);
        ntq_setup_uniforms(c);
        ntq_setup_registers(c, &c->s->registers);

        /* Find the main function and emit the body. */
        nir_foreach_overload(c->s, overload) {
                assert(strcmp(overload->function->name, "main") == 0);
                assert(overload->impl);
                ntq_emit_impl(c, overload->impl);
        }
}

static const nir_shader_compiler_options nir_options = {
        .lower_ffma = true,
        .lower_flrp = true,
        .lower_fpow = true,
        .lower_fsat = true,
        .lower_fsqrt = true,
        .lower_negate = true,
};

static bool
count_nir_instrs_in_block(nir_block *block, void *state)
{
        int *count = (int *) state;
        nir_foreach_instr(block, instr) {
                *count = *count + 1;
        }
        return true;
}

static int
count_nir_instrs(nir_shader *nir)
{
        int count = 0;
        nir_foreach_overload(nir, overload) {
                if (!overload->impl)
                        continue;
                nir_foreach_block(overload->impl, count_nir_instrs_in_block, &count);
        }
        return count;
}

static struct vc4_compile *
vc4_shader_ntq(struct vc4_context *vc4, enum qstage stage,
                       struct vc4_key *key)
{
        struct vc4_compile *c = qir_compile_init();

        c->stage = stage;
        c->shader_state = &key->shader_state->base;
        c->program_id = key->shader_state->program_id;
        c->variant_id = key->shader_state->compiled_variant_count++;

        c->key = key;
        switch (stage) {
        case QSTAGE_FRAG:
                c->fs_key = (struct vc4_fs_key *)key;
                if (c->fs_key->is_points) {
                        c->point_x = emit_fragment_varying(c, ~0, ~0, 0);
                        c->point_y = emit_fragment_varying(c, ~0, ~0, 0);
                } else if (c->fs_key->is_lines) {
                        c->line_x = emit_fragment_varying(c, ~0, ~0, 0);
                }
                break;
        case QSTAGE_VERT:
                c->vs_key = (struct vc4_vs_key *)key;
                break;
        case QSTAGE_COORD:
                c->vs_key = (struct vc4_vs_key *)key;
                break;
        }

        const struct tgsi_token *tokens = key->shader_state->base.tokens;
        if (c->fs_key && c->fs_key->light_twoside) {
                if (!key->shader_state->twoside_tokens) {
                        const struct tgsi_lowering_config lowering_config = {
                                .color_two_side = true,
                        };
                        struct tgsi_shader_info info;
                        key->shader_state->twoside_tokens =
                                tgsi_transform_lowering(&lowering_config,
                                                        key->shader_state->base.tokens,
                                                        &info);

                        /* If no transformation occurred, then NULL is
                         * returned and we just use our original tokens.
                         */
                        if (!key->shader_state->twoside_tokens) {
                                key->shader_state->twoside_tokens =
                                        key->shader_state->base.tokens;
                        }
                }
                tokens = key->shader_state->twoside_tokens;
        }

        if (vc4_debug & VC4_DEBUG_TGSI) {
                fprintf(stderr, "%s prog %d/%d TGSI:\n",
                        qir_get_stage_name(c->stage),
                        c->program_id, c->variant_id);
                tgsi_dump(tokens, 0);
        }

        c->s = tgsi_to_nir(tokens, &nir_options);
        nir_opt_global_to_local(c->s);
        nir_convert_to_ssa(c->s);
        nir_lower_idiv(c->s);

        vc4_optimize_nir(c->s);

        nir_remove_dead_variables(c->s);

        nir_convert_from_ssa(c->s);

        if (vc4_debug & VC4_DEBUG_SHADERDB) {
                fprintf(stderr, "SHADER-DB: %s prog %d/%d: %d NIR instructions\n",
                        qir_get_stage_name(c->stage),
                        c->program_id, c->variant_id,
                        count_nir_instrs(c->s));
        }

        if (vc4_debug & VC4_DEBUG_NIR) {
                fprintf(stderr, "%s prog %d/%d NIR:\n",
                        qir_get_stage_name(c->stage),
                        c->program_id, c->variant_id);
                nir_print_shader(c->s, stderr);
        }

        nir_to_qir(c);

        switch (stage) {
        case QSTAGE_FRAG:
                emit_frag_end(c);
                break;
        case QSTAGE_VERT:
                emit_vert_end(c,
                              vc4->prog.fs->input_semantics,
                              vc4->prog.fs->num_inputs);
                break;
        case QSTAGE_COORD:
                emit_coord_end(c);
                break;
        }

        if (vc4_debug & VC4_DEBUG_QIR) {
                fprintf(stderr, "%s prog %d/%d pre-opt QIR:\n",
                        qir_get_stage_name(c->stage),
                        c->program_id, c->variant_id);
                qir_dump(c);
        }

        qir_optimize(c);
        qir_lower_uniforms(c);

        if (vc4_debug & VC4_DEBUG_QIR) {
                fprintf(stderr, "%s prog %d/%d QIR:\n",
                        qir_get_stage_name(c->stage),
                        c->program_id, c->variant_id);
                qir_dump(c);
        }
        qir_reorder_uniforms(c);
        vc4_generate_code(vc4, c);

        if (vc4_debug & VC4_DEBUG_SHADERDB) {
                fprintf(stderr, "SHADER-DB: %s prog %d/%d: %d instructions\n",
                        qir_get_stage_name(c->stage),
                        c->program_id, c->variant_id,
                        c->qpu_inst_count);
                fprintf(stderr, "SHADER-DB: %s prog %d/%d: %d uniforms\n",
                        qir_get_stage_name(c->stage),
                        c->program_id, c->variant_id,
                        c->num_uniforms);
        }

        ralloc_free(c->s);

        return c;
}

static void *
vc4_shader_state_create(struct pipe_context *pctx,
                        const struct pipe_shader_state *cso)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_uncompiled_shader *so = CALLOC_STRUCT(vc4_uncompiled_shader);
        if (!so)
                return NULL;

        so->base.tokens = tgsi_dup_tokens(cso->tokens);
        so->program_id = vc4->next_uncompiled_program_id++;

        return so;
}

static void
copy_uniform_state_to_shader(struct vc4_compiled_shader *shader,
                             struct vc4_compile *c)
{
        int count = c->num_uniforms;
        struct vc4_shader_uniform_info *uinfo = &shader->uniforms;

        uinfo->count = count;
        uinfo->data = ralloc_array(shader, uint32_t, count);
        memcpy(uinfo->data, c->uniform_data,
               count * sizeof(*uinfo->data));
        uinfo->contents = ralloc_array(shader, enum quniform_contents, count);
        memcpy(uinfo->contents, c->uniform_contents,
               count * sizeof(*uinfo->contents));
        uinfo->num_texture_samples = c->num_texture_samples;
}

static struct vc4_compiled_shader *
vc4_get_compiled_shader(struct vc4_context *vc4, enum qstage stage,
                        struct vc4_key *key)
{
        struct hash_table *ht;
        uint32_t key_size;
        if (stage == QSTAGE_FRAG) {
                ht = vc4->fs_cache;
                key_size = sizeof(struct vc4_fs_key);
        } else {
                ht = vc4->vs_cache;
                key_size = sizeof(struct vc4_vs_key);
        }

        struct vc4_compiled_shader *shader;
        struct hash_entry *entry = _mesa_hash_table_search(ht, key);
        if (entry)
                return entry->data;

        struct vc4_compile *c = vc4_shader_ntq(vc4, stage, key);
        shader = rzalloc(NULL, struct vc4_compiled_shader);

        shader->program_id = vc4->next_compiled_program_id++;
        if (stage == QSTAGE_FRAG) {
                bool input_live[c->num_input_semantics];
                struct simple_node *node;

                memset(input_live, 0, sizeof(input_live));
                foreach(node, &c->instructions) {
                        struct qinst *inst = (struct qinst *)node;
                        for (int i = 0; i < qir_get_op_nsrc(inst->op); i++) {
                                if (inst->src[i].file == QFILE_VARY)
                                        input_live[inst->src[i].index] = true;
                        }
                }

                shader->input_semantics = ralloc_array(shader,
                                                       struct vc4_varying_semantic,
                                                       c->num_input_semantics);

                for (int i = 0; i < c->num_input_semantics; i++) {
                        struct vc4_varying_semantic *sem = &c->input_semantics[i];

                        if (!input_live[i])
                                continue;

                        /* Skip non-VS-output inputs. */
                        if (sem->semantic == (uint8_t)~0)
                                continue;

                        if (sem->semantic == TGSI_SEMANTIC_COLOR ||
                            sem->semantic == TGSI_SEMANTIC_BCOLOR) {
                                shader->color_inputs |= (1 << shader->num_inputs);
                        }

                        shader->input_semantics[shader->num_inputs] = *sem;
                        shader->num_inputs++;
                }
        } else {
                shader->num_inputs = c->num_inputs;

                shader->vattr_offsets[0] = 0;
                for (int i = 0; i < 8; i++) {
                        shader->vattr_offsets[i + 1] =
                                shader->vattr_offsets[i] + c->vattr_sizes[i];

                        if (c->vattr_sizes[i])
                                shader->vattrs_live |= (1 << i);
                }
        }

        copy_uniform_state_to_shader(shader, c);
        shader->bo = vc4_bo_alloc_mem(vc4->screen, c->qpu_insts,
                                      c->qpu_inst_count * sizeof(uint64_t),
                                      "code");

        /* Copy the compiler UBO range state to the compiled shader, dropping
         * out arrays that were never referenced by an indirect load.
         *
         * (Note that QIR dead code elimination of an array access still
         * leaves that array alive, though)
         */
        if (c->num_ubo_ranges) {
                shader->num_ubo_ranges = c->num_ubo_ranges;
                shader->ubo_ranges = ralloc_array(shader, struct vc4_ubo_range,
                                                  c->num_ubo_ranges);
                uint32_t j = 0;
                for (int i = 0; i < c->num_uniform_ranges; i++) {
                        struct vc4_compiler_ubo_range *range =
                                &c->ubo_ranges[i];
                        if (!range->used)
                                continue;

                        shader->ubo_ranges[j].dst_offset = range->dst_offset;
                        shader->ubo_ranges[j].src_offset = range->src_offset;
                        shader->ubo_ranges[j].size = range->size;
                        shader->ubo_size += c->ubo_ranges[i].size;
                        j++;
                }
        }
        if (shader->ubo_size) {
                fprintf(stderr, "SHADER-DB: %s prog %d/%d: %d UBO uniforms\n",
                        qir_get_stage_name(c->stage),
                        c->program_id, c->variant_id,
                        shader->ubo_size / 4);
        }

        qir_compile_destroy(c);

        struct vc4_key *dup_key;
        dup_key = ralloc_size(shader, key_size);
        memcpy(dup_key, key, key_size);
        _mesa_hash_table_insert(ht, dup_key, shader);

        return shader;
}

static void
vc4_setup_shared_key(struct vc4_context *vc4, struct vc4_key *key,
                     struct vc4_texture_stateobj *texstate)
{
        for (int i = 0; i < texstate->num_textures; i++) {
                struct pipe_sampler_view *sampler = texstate->textures[i];
                struct pipe_sampler_state *sampler_state =
                        texstate->samplers[i];

                if (sampler) {
                        key->tex[i].format = sampler->format;
                        key->tex[i].swizzle[0] = sampler->swizzle_r;
                        key->tex[i].swizzle[1] = sampler->swizzle_g;
                        key->tex[i].swizzle[2] = sampler->swizzle_b;
                        key->tex[i].swizzle[3] = sampler->swizzle_a;
                        key->tex[i].compare_mode = sampler_state->compare_mode;
                        key->tex[i].compare_func = sampler_state->compare_func;
                        key->tex[i].wrap_s = sampler_state->wrap_s;
                        key->tex[i].wrap_t = sampler_state->wrap_t;
                }
        }

        key->ucp_enables = vc4->rasterizer->base.clip_plane_enable;
}

static void
vc4_update_compiled_fs(struct vc4_context *vc4, uint8_t prim_mode)
{
        struct vc4_fs_key local_key;
        struct vc4_fs_key *key = &local_key;

        if (!(vc4->dirty & (VC4_DIRTY_PRIM_MODE |
                            VC4_DIRTY_BLEND |
                            VC4_DIRTY_FRAMEBUFFER |
                            VC4_DIRTY_ZSA |
                            VC4_DIRTY_RASTERIZER |
                            VC4_DIRTY_FRAGTEX |
                            VC4_DIRTY_TEXSTATE |
                            VC4_DIRTY_UNCOMPILED_FS))) {
                return;
        }

        memset(key, 0, sizeof(*key));
        vc4_setup_shared_key(vc4, &key->base, &vc4->fragtex);
        key->base.shader_state = vc4->prog.bind_fs;
        key->is_points = (prim_mode == PIPE_PRIM_POINTS);
        key->is_lines = (prim_mode >= PIPE_PRIM_LINES &&
                         prim_mode <= PIPE_PRIM_LINE_STRIP);
        key->blend = vc4->blend->rt[0];
        if (vc4->blend->logicop_enable) {
                key->logicop_func = vc4->blend->logicop_func;
        } else {
                key->logicop_func = PIPE_LOGICOP_COPY;
        }
        if (vc4->framebuffer.cbufs[0])
                key->color_format = vc4->framebuffer.cbufs[0]->format;

        key->stencil_enabled = vc4->zsa->stencil_uniforms[0] != 0;
        key->stencil_twoside = vc4->zsa->stencil_uniforms[1] != 0;
        key->stencil_full_writemasks = vc4->zsa->stencil_uniforms[2] != 0;
        key->depth_enabled = (vc4->zsa->base.depth.enabled ||
                              key->stencil_enabled);
        if (vc4->zsa->base.alpha.enabled) {
                key->alpha_test = true;
                key->alpha_test_func = vc4->zsa->base.alpha.func;
        }

        if (key->is_points) {
                key->point_sprite_mask =
                        vc4->rasterizer->base.sprite_coord_enable;
                key->point_coord_upper_left =
                        (vc4->rasterizer->base.sprite_coord_mode ==
                         PIPE_SPRITE_COORD_UPPER_LEFT);
        }

        key->light_twoside = vc4->rasterizer->base.light_twoside;

        struct vc4_compiled_shader *old_fs = vc4->prog.fs;
        vc4->prog.fs = vc4_get_compiled_shader(vc4, QSTAGE_FRAG, &key->base);
        if (vc4->prog.fs == old_fs)
                return;

        vc4->dirty |= VC4_DIRTY_COMPILED_FS;
        if (vc4->rasterizer->base.flatshade &&
            old_fs && vc4->prog.fs->color_inputs != old_fs->color_inputs) {
                vc4->dirty |= VC4_DIRTY_FLAT_SHADE_FLAGS;
        }
}

static void
vc4_update_compiled_vs(struct vc4_context *vc4, uint8_t prim_mode)
{
        struct vc4_vs_key local_key;
        struct vc4_vs_key *key = &local_key;

        if (!(vc4->dirty & (VC4_DIRTY_PRIM_MODE |
                            VC4_DIRTY_RASTERIZER |
                            VC4_DIRTY_VERTTEX |
                            VC4_DIRTY_TEXSTATE |
                            VC4_DIRTY_VTXSTATE |
                            VC4_DIRTY_UNCOMPILED_VS |
                            VC4_DIRTY_COMPILED_FS))) {
                return;
        }

        memset(key, 0, sizeof(*key));
        vc4_setup_shared_key(vc4, &key->base, &vc4->verttex);
        key->base.shader_state = vc4->prog.bind_vs;
        key->compiled_fs_id = vc4->prog.fs->program_id;

        for (int i = 0; i < ARRAY_SIZE(key->attr_formats); i++)
                key->attr_formats[i] = vc4->vtx->pipe[i].src_format;

        key->per_vertex_point_size =
                (prim_mode == PIPE_PRIM_POINTS &&
                 vc4->rasterizer->base.point_size_per_vertex);

        vc4->prog.vs = vc4_get_compiled_shader(vc4, QSTAGE_VERT, &key->base);
        key->is_coord = true;
        vc4->prog.cs = vc4_get_compiled_shader(vc4, QSTAGE_COORD, &key->base);
}

void
vc4_update_compiled_shaders(struct vc4_context *vc4, uint8_t prim_mode)
{
        vc4_update_compiled_fs(vc4, prim_mode);
        vc4_update_compiled_vs(vc4, prim_mode);
}

static uint32_t
fs_cache_hash(const void *key)
{
        return _mesa_hash_data(key, sizeof(struct vc4_fs_key));
}

static uint32_t
vs_cache_hash(const void *key)
{
        return _mesa_hash_data(key, sizeof(struct vc4_vs_key));
}

static bool
fs_cache_compare(const void *key1, const void *key2)
{
        return memcmp(key1, key2, sizeof(struct vc4_fs_key)) == 0;
}

static bool
vs_cache_compare(const void *key1, const void *key2)
{
        return memcmp(key1, key2, sizeof(struct vc4_vs_key)) == 0;
}

static void
delete_from_cache_if_matches(struct hash_table *ht,
                             struct hash_entry *entry,
                             struct vc4_uncompiled_shader *so)
{
        const struct vc4_key *key = entry->key;

        if (key->shader_state == so) {
                struct vc4_compiled_shader *shader = entry->data;
                _mesa_hash_table_remove(ht, entry);
                vc4_bo_unreference(&shader->bo);
                ralloc_free(shader);
        }
}

static void
vc4_shader_state_delete(struct pipe_context *pctx, void *hwcso)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        struct vc4_uncompiled_shader *so = hwcso;

        struct hash_entry *entry;
        hash_table_foreach(vc4->fs_cache, entry)
                delete_from_cache_if_matches(vc4->fs_cache, entry, so);
        hash_table_foreach(vc4->vs_cache, entry)
                delete_from_cache_if_matches(vc4->vs_cache, entry, so);

        if (so->twoside_tokens != so->base.tokens)
                free((void *)so->twoside_tokens);
        free((void *)so->base.tokens);
        free(so);
}

static uint32_t translate_wrap(uint32_t p_wrap, bool using_nearest)
{
        switch (p_wrap) {
        case PIPE_TEX_WRAP_REPEAT:
                return 0;
        case PIPE_TEX_WRAP_CLAMP_TO_EDGE:
                return 1;
        case PIPE_TEX_WRAP_MIRROR_REPEAT:
                return 2;
        case PIPE_TEX_WRAP_CLAMP_TO_BORDER:
                return 3;
        case PIPE_TEX_WRAP_CLAMP:
                return (using_nearest ? 1 : 3);
        default:
                fprintf(stderr, "Unknown wrap mode %d\n", p_wrap);
                assert(!"not reached");
                return 0;
        }
}

static void
write_texture_p0(struct vc4_context *vc4,
                 struct vc4_texture_stateobj *texstate,
                 uint32_t unit)
{
        struct pipe_sampler_view *texture = texstate->textures[unit];
        struct vc4_resource *rsc = vc4_resource(texture->texture);

        cl_reloc(vc4, &vc4->uniforms, rsc->bo,
                 VC4_SET_FIELD(rsc->slices[0].offset >> 12, VC4_TEX_P0_OFFSET) |
                 VC4_SET_FIELD(texture->u.tex.last_level -
                               texture->u.tex.first_level, VC4_TEX_P0_MIPLVLS) |
                 VC4_SET_FIELD(texture->target == PIPE_TEXTURE_CUBE,
                               VC4_TEX_P0_CMMODE) |
                 VC4_SET_FIELD(rsc->vc4_format & 15, VC4_TEX_P0_TYPE));
}

static void
write_texture_p1(struct vc4_context *vc4,
                 struct vc4_texture_stateobj *texstate,
                 uint32_t unit)
{
        struct pipe_sampler_view *texture = texstate->textures[unit];
        struct vc4_resource *rsc = vc4_resource(texture->texture);
        struct pipe_sampler_state *sampler = texstate->samplers[unit];
        static const uint8_t minfilter_map[6] = {
                VC4_TEX_P1_MINFILT_NEAR_MIP_NEAR,
                VC4_TEX_P1_MINFILT_LIN_MIP_NEAR,
                VC4_TEX_P1_MINFILT_NEAR_MIP_LIN,
                VC4_TEX_P1_MINFILT_LIN_MIP_LIN,
                VC4_TEX_P1_MINFILT_NEAREST,
                VC4_TEX_P1_MINFILT_LINEAR,
        };
        static const uint32_t magfilter_map[] = {
                [PIPE_TEX_FILTER_NEAREST] = VC4_TEX_P1_MAGFILT_NEAREST,
                [PIPE_TEX_FILTER_LINEAR] = VC4_TEX_P1_MAGFILT_LINEAR,
        };

        bool either_nearest =
                (sampler->mag_img_filter == PIPE_TEX_MIPFILTER_NEAREST ||
                 sampler->min_img_filter == PIPE_TEX_MIPFILTER_NEAREST);

        cl_aligned_u32(&vc4->uniforms,
               VC4_SET_FIELD(rsc->vc4_format >> 4, VC4_TEX_P1_TYPE4) |
               VC4_SET_FIELD(texture->texture->height0 & 2047,
                             VC4_TEX_P1_HEIGHT) |
               VC4_SET_FIELD(texture->texture->width0 & 2047,
                             VC4_TEX_P1_WIDTH) |
               VC4_SET_FIELD(magfilter_map[sampler->mag_img_filter],
                             VC4_TEX_P1_MAGFILT) |
               VC4_SET_FIELD(minfilter_map[sampler->min_mip_filter * 2 +
                                           sampler->min_img_filter],
                             VC4_TEX_P1_MINFILT) |
               VC4_SET_FIELD(translate_wrap(sampler->wrap_s, either_nearest),
                             VC4_TEX_P1_WRAP_S) |
               VC4_SET_FIELD(translate_wrap(sampler->wrap_t, either_nearest),
                             VC4_TEX_P1_WRAP_T));
}

static void
write_texture_p2(struct vc4_context *vc4,
                 struct vc4_texture_stateobj *texstate,
                 uint32_t data)
{
        uint32_t unit = data & 0xffff;
        struct pipe_sampler_view *texture = texstate->textures[unit];
        struct vc4_resource *rsc = vc4_resource(texture->texture);

        cl_aligned_u32(&vc4->uniforms,
               VC4_SET_FIELD(VC4_TEX_P2_PTYPE_CUBE_MAP_STRIDE,
                             VC4_TEX_P2_PTYPE) |
               VC4_SET_FIELD(rsc->cube_map_stride >> 12, VC4_TEX_P2_CMST) |
               VC4_SET_FIELD((data >> 16) & 1, VC4_TEX_P2_BSLOD));
}


#define SWIZ(x,y,z,w) {          \
        UTIL_FORMAT_SWIZZLE_##x, \
        UTIL_FORMAT_SWIZZLE_##y, \
        UTIL_FORMAT_SWIZZLE_##z, \
        UTIL_FORMAT_SWIZZLE_##w  \
}

static void
write_texture_border_color(struct vc4_context *vc4,
                           struct vc4_texture_stateobj *texstate,
                           uint32_t unit)
{
        struct pipe_sampler_state *sampler = texstate->samplers[unit];
        struct pipe_sampler_view *texture = texstate->textures[unit];
        struct vc4_resource *rsc = vc4_resource(texture->texture);
        union util_color uc;

        const struct util_format_description *tex_format_desc =
                util_format_description(texture->format);

        float border_color[4];
        for (int i = 0; i < 4; i++)
                border_color[i] = sampler->border_color.f[i];
        if (util_format_is_srgb(texture->format)) {
                for (int i = 0; i < 3; i++)
                        border_color[i] =
                                util_format_linear_to_srgb_float(border_color[i]);
        }

        /* Turn the border color into the layout of channels that it would
         * have when stored as texture contents.
         */
        float storage_color[4];
        util_format_unswizzle_4f(storage_color,
                                 border_color,
                                 tex_format_desc->swizzle);

        /* Now, pack so that when the vc4_format-sampled texture contents are
         * replaced with our border color, the vc4_get_format_swizzle()
         * swizzling will get the right channels.
         */
        if (util_format_is_depth_or_stencil(texture->format)) {
                uc.ui[0] = util_pack_z(PIPE_FORMAT_Z24X8_UNORM,
                                       sampler->border_color.f[0]) << 8;
        } else {
                switch (rsc->vc4_format) {
                default:
                case VC4_TEXTURE_TYPE_RGBA8888:
                        util_pack_color(storage_color,
                                        PIPE_FORMAT_R8G8B8A8_UNORM, &uc);
                        break;
                case VC4_TEXTURE_TYPE_RGBA4444:
                        util_pack_color(storage_color,
                                        PIPE_FORMAT_A8B8G8R8_UNORM, &uc);
                        break;
                case VC4_TEXTURE_TYPE_RGB565:
                        util_pack_color(storage_color,
                                        PIPE_FORMAT_B8G8R8A8_UNORM, &uc);
                        break;
                case VC4_TEXTURE_TYPE_ALPHA:
                        uc.ui[0] = float_to_ubyte(storage_color[0]) << 24;
                        break;
                case VC4_TEXTURE_TYPE_LUMALPHA:
                        uc.ui[0] = ((float_to_ubyte(storage_color[1]) << 24) |
                                    (float_to_ubyte(storage_color[0]) << 0));
                        break;
                }
        }

        cl_aligned_u32(&vc4->uniforms, uc.ui[0]);
}

static uint32_t
get_texrect_scale(struct vc4_texture_stateobj *texstate,
                  enum quniform_contents contents,
                  uint32_t data)
{
        struct pipe_sampler_view *texture = texstate->textures[data];
        uint32_t dim;

        if (contents == QUNIFORM_TEXRECT_SCALE_X)
                dim = texture->texture->width0;
        else
                dim = texture->texture->height0;

        return fui(1.0f / dim);
}

static struct vc4_bo *
vc4_upload_ubo(struct vc4_context *vc4, struct vc4_compiled_shader *shader,
               const uint32_t *gallium_uniforms)
{
        if (!shader->ubo_size)
                return NULL;

        struct vc4_bo *ubo = vc4_bo_alloc(vc4->screen, shader->ubo_size, "ubo");
        uint32_t *data = vc4_bo_map(ubo);
        for (uint32_t i = 0; i < shader->num_ubo_ranges; i++) {
                memcpy(data + shader->ubo_ranges[i].dst_offset,
                       gallium_uniforms + shader->ubo_ranges[i].src_offset,
                       shader->ubo_ranges[i].size);
        }

        return ubo;
}

void
vc4_write_uniforms(struct vc4_context *vc4, struct vc4_compiled_shader *shader,
                   struct vc4_constbuf_stateobj *cb,
                   struct vc4_texture_stateobj *texstate)
{
        struct vc4_shader_uniform_info *uinfo = &shader->uniforms;
        const uint32_t *gallium_uniforms = cb->cb[0].user_buffer;
        struct vc4_bo *ubo = vc4_upload_ubo(vc4, shader, gallium_uniforms);

        cl_ensure_space(&vc4->uniforms, (uinfo->count +
                                         uinfo->num_texture_samples) * 4);

        cl_start_shader_reloc(&vc4->uniforms, uinfo->num_texture_samples);

        for (int i = 0; i < uinfo->count; i++) {

                switch (uinfo->contents[i]) {
                case QUNIFORM_CONSTANT:
                        cl_aligned_u32(&vc4->uniforms, uinfo->data[i]);
                        break;
                case QUNIFORM_UNIFORM:
                        cl_aligned_u32(&vc4->uniforms,
                                       gallium_uniforms[uinfo->data[i]]);
                        break;
                case QUNIFORM_VIEWPORT_X_SCALE:
                        cl_aligned_f(&vc4->uniforms, vc4->viewport.scale[0] * 16.0f);
                        break;
                case QUNIFORM_VIEWPORT_Y_SCALE:
                        cl_aligned_f(&vc4->uniforms, vc4->viewport.scale[1] * 16.0f);
                        break;

                case QUNIFORM_VIEWPORT_Z_OFFSET:
                        cl_aligned_f(&vc4->uniforms, vc4->viewport.translate[2]);
                        break;
                case QUNIFORM_VIEWPORT_Z_SCALE:
                        cl_aligned_f(&vc4->uniforms, vc4->viewport.scale[2]);
                        break;

                case QUNIFORM_USER_CLIP_PLANE:
                        cl_aligned_f(&vc4->uniforms,
                                     vc4->clip.ucp[uinfo->data[i] / 4][uinfo->data[i] % 4]);
                        break;

                case QUNIFORM_TEXTURE_CONFIG_P0:
                        write_texture_p0(vc4, texstate, uinfo->data[i]);
                        break;

                case QUNIFORM_TEXTURE_CONFIG_P1:
                        write_texture_p1(vc4, texstate, uinfo->data[i]);
                        break;

                case QUNIFORM_TEXTURE_CONFIG_P2:
                        write_texture_p2(vc4, texstate, uinfo->data[i]);
                        break;

                case QUNIFORM_UBO_ADDR:
                        cl_aligned_reloc(vc4, &vc4->uniforms, ubo, 0);
                        break;

                case QUNIFORM_TEXTURE_BORDER_COLOR:
                        write_texture_border_color(vc4, texstate, uinfo->data[i]);
                        break;

                case QUNIFORM_TEXRECT_SCALE_X:
                case QUNIFORM_TEXRECT_SCALE_Y:
                        cl_aligned_u32(&vc4->uniforms,
                                       get_texrect_scale(texstate,
                                                         uinfo->contents[i],
                                                         uinfo->data[i]));
                        break;

                case QUNIFORM_BLEND_CONST_COLOR:
                        cl_aligned_f(&vc4->uniforms,
                                     CLAMP(vc4->blend_color.color[uinfo->data[i]], 0, 1));
                        break;

                case QUNIFORM_STENCIL:
                        cl_aligned_u32(&vc4->uniforms,
                                       vc4->zsa->stencil_uniforms[uinfo->data[i]] |
                                       (uinfo->data[i] <= 1 ?
                                        (vc4->stencil_ref.ref_value[uinfo->data[i]] << 8) :
                                        0));
                        break;

                case QUNIFORM_ALPHA_REF:
                        cl_aligned_f(&vc4->uniforms,
                                     vc4->zsa->base.alpha.ref_value);
                        break;
                }
#if 0
                uint32_t written_val = *(uint32_t *)(vc4->uniforms.next - 4);
                fprintf(stderr, "%p: %d / 0x%08x (%f)\n",
                        shader, i, written_val, uif(written_val));
#endif
        }
}

static void
vc4_fp_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->prog.bind_fs = hwcso;
        vc4->dirty |= VC4_DIRTY_UNCOMPILED_FS;
}

static void
vc4_vp_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct vc4_context *vc4 = vc4_context(pctx);
        vc4->prog.bind_vs = hwcso;
        vc4->dirty |= VC4_DIRTY_UNCOMPILED_VS;
}

void
vc4_program_init(struct pipe_context *pctx)
{
        struct vc4_context *vc4 = vc4_context(pctx);

        pctx->create_vs_state = vc4_shader_state_create;
        pctx->delete_vs_state = vc4_shader_state_delete;

        pctx->create_fs_state = vc4_shader_state_create;
        pctx->delete_fs_state = vc4_shader_state_delete;

        pctx->bind_fs_state = vc4_fp_state_bind;
        pctx->bind_vs_state = vc4_vp_state_bind;

        vc4->fs_cache = _mesa_hash_table_create(pctx, fs_cache_hash,
                                                fs_cache_compare);
        vc4->vs_cache = _mesa_hash_table_create(pctx, vs_cache_hash,
                                                vs_cache_compare);
}

void
vc4_program_fini(struct pipe_context *pctx)
{
        struct vc4_context *vc4 = vc4_context(pctx);

        struct hash_entry *entry;
        hash_table_foreach(vc4->fs_cache, entry) {
                struct vc4_compiled_shader *shader = entry->data;
                vc4_bo_unreference(&shader->bo);
                ralloc_free(shader);
                _mesa_hash_table_remove(vc4->fs_cache, entry);
        }

        hash_table_foreach(vc4->vs_cache, entry) {
                struct vc4_compiled_shader *shader = entry->data;
                vc4_bo_unreference(&shader->bo);
                ralloc_free(shader);
                _mesa_hash_table_remove(vc4->vs_cache, entry);
        }
}
