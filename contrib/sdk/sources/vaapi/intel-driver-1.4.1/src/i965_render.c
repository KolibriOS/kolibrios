/*
 * Copyright © 2006 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Keith Packard <keithp@keithp.com>
 *    Xiang Haihao <haihao.xiang@intel.com>
 *
 */

/*
 * Most of rendering codes are ported from xf86-video-intel/src/i965_video.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <va/va_drmcommon.h>

#include "intel_batchbuffer.h"
#include "intel_driver.h"
#include "i965_defines.h"
#include "i965_drv_video.h"
#include "i965_structs.h"

#include "i965_render.h"

#define SF_KERNEL_NUM_GRF       16
#define SF_MAX_THREADS          1

static const uint32_t sf_kernel_static[][4] = 
{
#include "shaders/render/exa_sf.g4b"
};

#define PS_KERNEL_NUM_GRF       48
#define PS_MAX_THREADS          32

#define I965_GRF_BLOCKS(nreg)	((nreg + 15) / 16 - 1)

static const uint32_t ps_kernel_static[][4] = 
{
#include "shaders/render/exa_wm_xy.g4b"
#include "shaders/render/exa_wm_src_affine.g4b"
#include "shaders/render/exa_wm_src_sample_planar.g4b"
#include "shaders/render/exa_wm_yuv_color_balance.g4b"
#include "shaders/render/exa_wm_yuv_rgb.g4b"
#include "shaders/render/exa_wm_write.g4b"
};
static const uint32_t ps_subpic_kernel_static[][4] = 
{
#include "shaders/render/exa_wm_xy.g4b"
#include "shaders/render/exa_wm_src_affine.g4b"
#include "shaders/render/exa_wm_src_sample_argb.g4b"
#include "shaders/render/exa_wm_write.g4b"
};

/* On IRONLAKE */
static const uint32_t sf_kernel_static_gen5[][4] = 
{
#include "shaders/render/exa_sf.g4b.gen5"
};

static const uint32_t ps_kernel_static_gen5[][4] = 
{
#include "shaders/render/exa_wm_xy.g4b.gen5"
#include "shaders/render/exa_wm_src_affine.g4b.gen5"
#include "shaders/render/exa_wm_src_sample_planar.g4b.gen5"
#include "shaders/render/exa_wm_yuv_color_balance.g4b.gen5"
#include "shaders/render/exa_wm_yuv_rgb.g4b.gen5"
#include "shaders/render/exa_wm_write.g4b.gen5"
};
static const uint32_t ps_subpic_kernel_static_gen5[][4] = 
{
#include "shaders/render/exa_wm_xy.g4b.gen5"
#include "shaders/render/exa_wm_src_affine.g4b.gen5"
#include "shaders/render/exa_wm_src_sample_argb.g4b.gen5"
#include "shaders/render/exa_wm_write.g4b.gen5"
};

/* programs for Sandybridge */
static const uint32_t sf_kernel_static_gen6[][4] = 
{
};

static const uint32_t ps_kernel_static_gen6[][4] = {
#include "shaders/render/exa_wm_src_affine.g6b"
#include "shaders/render/exa_wm_src_sample_planar.g6b"
#include "shaders/render/exa_wm_yuv_color_balance.g6b"
#include "shaders/render/exa_wm_yuv_rgb.g6b"
#include "shaders/render/exa_wm_write.g6b"
};

static const uint32_t ps_subpic_kernel_static_gen6[][4] = {
#include "shaders/render/exa_wm_src_affine.g6b"
#include "shaders/render/exa_wm_src_sample_argb.g6b"
#include "shaders/render/exa_wm_write.g6b"
};

/* programs for Ivybridge */
static const uint32_t sf_kernel_static_gen7[][4] = 
{
};

static const uint32_t ps_kernel_static_gen7[][4] = {
#include "shaders/render/exa_wm_src_affine.g7b"
#include "shaders/render/exa_wm_src_sample_planar.g7b"
#include "shaders/render/exa_wm_yuv_color_balance.g7b"
#include "shaders/render/exa_wm_yuv_rgb.g7b"
#include "shaders/render/exa_wm_write.g7b"
};

static const uint32_t ps_subpic_kernel_static_gen7[][4] = {
#include "shaders/render/exa_wm_src_affine.g7b"
#include "shaders/render/exa_wm_src_sample_argb.g7b"
#include "shaders/render/exa_wm_write.g7b"
};

/* Programs for Haswell */
static const uint32_t ps_kernel_static_gen7_haswell[][4] = {
#include "shaders/render/exa_wm_src_affine.g7b"
#include "shaders/render/exa_wm_src_sample_planar.g7b.haswell"
#include "shaders/render/exa_wm_yuv_color_balance.g7b.haswell"
#include "shaders/render/exa_wm_yuv_rgb.g7b"
#include "shaders/render/exa_wm_write.g7b"
};


#define SURFACE_STATE_PADDED_SIZE       MAX(SURFACE_STATE_PADDED_SIZE_GEN6, SURFACE_STATE_PADDED_SIZE_GEN7)

#define SURFACE_STATE_OFFSET(index)     (SURFACE_STATE_PADDED_SIZE * index)
#define BINDING_TABLE_OFFSET            SURFACE_STATE_OFFSET(MAX_RENDER_SURFACES)

static uint32_t float_to_uint (float f) 
{
    union {
        uint32_t i; 
        float f;
    } x;

    x.f = f;
    return x.i;
}

enum 
{
    SF_KERNEL = 0,
    PS_KERNEL,
    PS_SUBPIC_KERNEL
};

static struct i965_kernel render_kernels_gen4[] = {
    {
        "SF",
        SF_KERNEL,
        sf_kernel_static,
        sizeof(sf_kernel_static),
        NULL
    },
    {
        "PS",
        PS_KERNEL,
        ps_kernel_static,
        sizeof(ps_kernel_static),
        NULL
    },

    {
        "PS_SUBPIC",
        PS_SUBPIC_KERNEL,
        ps_subpic_kernel_static,
        sizeof(ps_subpic_kernel_static),
        NULL
    }
};

static struct i965_kernel render_kernels_gen5[] = {
    {
        "SF",
        SF_KERNEL,
        sf_kernel_static_gen5,
        sizeof(sf_kernel_static_gen5),
        NULL
    },
    {
        "PS",
        PS_KERNEL,
        ps_kernel_static_gen5,
        sizeof(ps_kernel_static_gen5),
        NULL
    },

    {
        "PS_SUBPIC",
        PS_SUBPIC_KERNEL,
        ps_subpic_kernel_static_gen5,
        sizeof(ps_subpic_kernel_static_gen5),
        NULL
    }
};

static struct i965_kernel render_kernels_gen6[] = {
    {
        "SF",
        SF_KERNEL,
        sf_kernel_static_gen6,
        sizeof(sf_kernel_static_gen6),
        NULL
    },
    {
        "PS",
        PS_KERNEL,
        ps_kernel_static_gen6,
        sizeof(ps_kernel_static_gen6),
        NULL
    },

    {
        "PS_SUBPIC",
        PS_SUBPIC_KERNEL,
        ps_subpic_kernel_static_gen6,
        sizeof(ps_subpic_kernel_static_gen6),
        NULL
    }
};

static struct i965_kernel render_kernels_gen7[] = {
    {
        "SF",
        SF_KERNEL,
        sf_kernel_static_gen7,
        sizeof(sf_kernel_static_gen7),
        NULL
    },
    {
        "PS",
        PS_KERNEL,
        ps_kernel_static_gen7,
        sizeof(ps_kernel_static_gen7),
        NULL
    },

    {
        "PS_SUBPIC",
        PS_SUBPIC_KERNEL,
        ps_subpic_kernel_static_gen7,
        sizeof(ps_subpic_kernel_static_gen7),
        NULL
    }
};

static struct i965_kernel render_kernels_gen7_haswell[] = {
    {
        "SF",
        SF_KERNEL,
        sf_kernel_static_gen7,
        sizeof(sf_kernel_static_gen7),
        NULL
    },
    {
        "PS",
        PS_KERNEL,
        ps_kernel_static_gen7_haswell,
        sizeof(ps_kernel_static_gen7_haswell),
        NULL
    },

    {
        "PS_SUBPIC",
        PS_SUBPIC_KERNEL,
        ps_subpic_kernel_static_gen7,
        sizeof(ps_subpic_kernel_static_gen7),
        NULL
    }
};

#define URB_VS_ENTRIES	      8
#define URB_VS_ENTRY_SIZE     1

#define URB_GS_ENTRIES	      0
#define URB_GS_ENTRY_SIZE     0

#define URB_CLIP_ENTRIES      0
#define URB_CLIP_ENTRY_SIZE   0

#define URB_SF_ENTRIES	      1
#define URB_SF_ENTRY_SIZE     2

#define URB_CS_ENTRIES	      4
#define URB_CS_ENTRY_SIZE     4

static float yuv_to_rgb_bt601[3][4] = {
{1.164,		0,	1.596,		-0.06275,},
{1.164,		-0.392,	-0.813,		-0.50196,},
{1.164,		2.017,	0,		-0.50196,},
};

static float yuv_to_rgb_bt709[3][4] = {
{1.164,		0,	1.793,		-0.06275,},
{1.164,		-0.213,	-0.533,		-0.50196,},
{1.164,		2.112,	0,		-0.50196,},
};

static float yuv_to_rgb_smpte_240[3][4] = {
{1.164,		0,	1.794,		-0.06275,},
{1.164,		-0.258,	-0.5425,	-0.50196,},
{1.164,		2.078,	0,		-0.50196,},
};

static void
i965_render_vs_unit(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct i965_vs_unit_state *vs_state;

    dri_bo_map(render_state->vs.state, 1);
    assert(render_state->vs.state->virtual);
    vs_state = render_state->vs.state->virtual;
    memset(vs_state, 0, sizeof(*vs_state));

    if (IS_IRONLAKE(i965->intel.device_info))
        vs_state->thread4.nr_urb_entries = URB_VS_ENTRIES >> 2;
    else
        vs_state->thread4.nr_urb_entries = URB_VS_ENTRIES;

    vs_state->thread4.urb_entry_allocation_size = URB_VS_ENTRY_SIZE - 1;
    vs_state->vs6.vs_enable = 0;
    vs_state->vs6.vert_cache_disable = 1;
    
    dri_bo_unmap(render_state->vs.state);
}

static void
i965_render_sf_unit(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct i965_sf_unit_state *sf_state;

    dri_bo_map(render_state->sf.state, 1);
    assert(render_state->sf.state->virtual);
    sf_state = render_state->sf.state->virtual;
    memset(sf_state, 0, sizeof(*sf_state));

    sf_state->thread0.grf_reg_count = I965_GRF_BLOCKS(SF_KERNEL_NUM_GRF);
    sf_state->thread0.kernel_start_pointer = render_state->render_kernels[SF_KERNEL].bo->offset >> 6;

    sf_state->sf1.single_program_flow = 1; /* XXX */
    sf_state->sf1.binding_table_entry_count = 0;
    sf_state->sf1.thread_priority = 0;
    sf_state->sf1.floating_point_mode = 0; /* Mesa does this */
    sf_state->sf1.illegal_op_exception_enable = 1;
    sf_state->sf1.mask_stack_exception_enable = 1;
    sf_state->sf1.sw_exception_enable = 1;

    /* scratch space is not used in our kernel */
    sf_state->thread2.per_thread_scratch_space = 0;
    sf_state->thread2.scratch_space_base_pointer = 0;

    sf_state->thread3.const_urb_entry_read_length = 0; /* no const URBs */
    sf_state->thread3.const_urb_entry_read_offset = 0; /* no const URBs */
    sf_state->thread3.urb_entry_read_length = 1; /* 1 URB per vertex */
    sf_state->thread3.urb_entry_read_offset = 0;
    sf_state->thread3.dispatch_grf_start_reg = 3;

    sf_state->thread4.max_threads = SF_MAX_THREADS - 1;
    sf_state->thread4.urb_entry_allocation_size = URB_SF_ENTRY_SIZE - 1;
    sf_state->thread4.nr_urb_entries = URB_SF_ENTRIES;
    sf_state->thread4.stats_enable = 1;

    sf_state->sf5.viewport_transform = 0; /* skip viewport */

    sf_state->sf6.cull_mode = I965_CULLMODE_NONE;
    sf_state->sf6.scissor = 0;

    sf_state->sf7.trifan_pv = 2;

    sf_state->sf6.dest_org_vbias = 0x8;
    sf_state->sf6.dest_org_hbias = 0x8;

    dri_bo_emit_reloc(render_state->sf.state,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      sf_state->thread0.grf_reg_count << 1,
                      offsetof(struct i965_sf_unit_state, thread0),
                      render_state->render_kernels[SF_KERNEL].bo);

    dri_bo_unmap(render_state->sf.state);
}

static void 
i965_render_sampler(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct i965_sampler_state *sampler_state;
    int i;
    
    assert(render_state->wm.sampler_count > 0);
    assert(render_state->wm.sampler_count <= MAX_SAMPLERS);

    dri_bo_map(render_state->wm.sampler, 1);
    assert(render_state->wm.sampler->virtual);
    sampler_state = render_state->wm.sampler->virtual;
    for (i = 0; i < render_state->wm.sampler_count; i++) {
        memset(sampler_state, 0, sizeof(*sampler_state));
        sampler_state->ss0.min_filter = I965_MAPFILTER_LINEAR;
        sampler_state->ss0.mag_filter = I965_MAPFILTER_LINEAR;
        sampler_state->ss1.r_wrap_mode = I965_TEXCOORDMODE_CLAMP;
        sampler_state->ss1.s_wrap_mode = I965_TEXCOORDMODE_CLAMP;
        sampler_state->ss1.t_wrap_mode = I965_TEXCOORDMODE_CLAMP;
        sampler_state++;
    }

    dri_bo_unmap(render_state->wm.sampler);
}
static void
i965_subpic_render_wm_unit(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct i965_wm_unit_state *wm_state;

    assert(render_state->wm.sampler);

    dri_bo_map(render_state->wm.state, 1);
    assert(render_state->wm.state->virtual);
    wm_state = render_state->wm.state->virtual;
    memset(wm_state, 0, sizeof(*wm_state));

    wm_state->thread0.grf_reg_count = I965_GRF_BLOCKS(PS_KERNEL_NUM_GRF);
    wm_state->thread0.kernel_start_pointer = render_state->render_kernels[PS_SUBPIC_KERNEL].bo->offset >> 6;

    wm_state->thread1.single_program_flow = 1; /* XXX */

    if (IS_IRONLAKE(i965->intel.device_info))
        wm_state->thread1.binding_table_entry_count = 0; /* hardware requirement */
    else
        wm_state->thread1.binding_table_entry_count = 7;

    wm_state->thread2.scratch_space_base_pointer = 0;
    wm_state->thread2.per_thread_scratch_space = 0; /* 1024 bytes */

    wm_state->thread3.dispatch_grf_start_reg = 2; /* XXX */
    wm_state->thread3.const_urb_entry_read_length = 4;
    wm_state->thread3.const_urb_entry_read_offset = 0;
    wm_state->thread3.urb_entry_read_length = 1; /* XXX */
    wm_state->thread3.urb_entry_read_offset = 0; /* XXX */

    wm_state->wm4.stats_enable = 0;
    wm_state->wm4.sampler_state_pointer = render_state->wm.sampler->offset >> 5; 

    if (IS_IRONLAKE(i965->intel.device_info)) {
        wm_state->wm4.sampler_count = 0;        /* hardware requirement */
    } else {
        wm_state->wm4.sampler_count = (render_state->wm.sampler_count + 3) / 4;
    }

    wm_state->wm5.max_threads = i965->intel.device_info->max_wm_threads - 1;
    wm_state->wm5.thread_dispatch_enable = 1;
    wm_state->wm5.enable_16_pix = 1;
    wm_state->wm5.enable_8_pix = 0;
    wm_state->wm5.early_depth_test = 1;

    dri_bo_emit_reloc(render_state->wm.state,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      wm_state->thread0.grf_reg_count << 1,
                      offsetof(struct i965_wm_unit_state, thread0),
                      render_state->render_kernels[PS_SUBPIC_KERNEL].bo);

    dri_bo_emit_reloc(render_state->wm.state,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      wm_state->wm4.sampler_count << 2,
                      offsetof(struct i965_wm_unit_state, wm4),
                      render_state->wm.sampler);

    dri_bo_unmap(render_state->wm.state);
}


static void
i965_render_wm_unit(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct i965_wm_unit_state *wm_state;

    assert(render_state->wm.sampler);

    dri_bo_map(render_state->wm.state, 1);
    assert(render_state->wm.state->virtual);
    wm_state = render_state->wm.state->virtual;
    memset(wm_state, 0, sizeof(*wm_state));

    wm_state->thread0.grf_reg_count = I965_GRF_BLOCKS(PS_KERNEL_NUM_GRF);
    wm_state->thread0.kernel_start_pointer = render_state->render_kernels[PS_KERNEL].bo->offset >> 6;

    wm_state->thread1.single_program_flow = 1; /* XXX */

    if (IS_IRONLAKE(i965->intel.device_info))
        wm_state->thread1.binding_table_entry_count = 0;        /* hardware requirement */
    else
        wm_state->thread1.binding_table_entry_count = 7;

    wm_state->thread2.scratch_space_base_pointer = 0;
    wm_state->thread2.per_thread_scratch_space = 0; /* 1024 bytes */

    wm_state->thread3.dispatch_grf_start_reg = 2; /* XXX */
    wm_state->thread3.const_urb_entry_read_length = 4;
    wm_state->thread3.const_urb_entry_read_offset = 0;
    wm_state->thread3.urb_entry_read_length = 1; /* XXX */
    wm_state->thread3.urb_entry_read_offset = 0; /* XXX */

    wm_state->wm4.stats_enable = 0;
    wm_state->wm4.sampler_state_pointer = render_state->wm.sampler->offset >> 5; 

    if (IS_IRONLAKE(i965->intel.device_info)) {
        wm_state->wm4.sampler_count = 0;        /* hardware requirement */
    } else {
        wm_state->wm4.sampler_count = (render_state->wm.sampler_count + 3) / 4;
    }

    wm_state->wm5.max_threads = i965->intel.device_info->max_wm_threads - 1;
    wm_state->wm5.thread_dispatch_enable = 1;
    wm_state->wm5.enable_16_pix = 1;
    wm_state->wm5.enable_8_pix = 0;
    wm_state->wm5.early_depth_test = 1;

    dri_bo_emit_reloc(render_state->wm.state,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      wm_state->thread0.grf_reg_count << 1,
                      offsetof(struct i965_wm_unit_state, thread0),
                      render_state->render_kernels[PS_KERNEL].bo);

    dri_bo_emit_reloc(render_state->wm.state,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      wm_state->wm4.sampler_count << 2,
                      offsetof(struct i965_wm_unit_state, wm4),
                      render_state->wm.sampler);

    dri_bo_unmap(render_state->wm.state);
}

static void 
i965_render_cc_viewport(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct i965_cc_viewport *cc_viewport;

    dri_bo_map(render_state->cc.viewport, 1);
    assert(render_state->cc.viewport->virtual);
    cc_viewport = render_state->cc.viewport->virtual;
    memset(cc_viewport, 0, sizeof(*cc_viewport));
    
    cc_viewport->min_depth = -1.e35;
    cc_viewport->max_depth = 1.e35;

    dri_bo_unmap(render_state->cc.viewport);
}

static void 
i965_subpic_render_cc_unit(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct i965_cc_unit_state *cc_state;

    assert(render_state->cc.viewport);

    dri_bo_map(render_state->cc.state, 1);
    assert(render_state->cc.state->virtual);
    cc_state = render_state->cc.state->virtual;
    memset(cc_state, 0, sizeof(*cc_state));

    cc_state->cc0.stencil_enable = 0;   /* disable stencil */
    cc_state->cc2.depth_test = 0;       /* disable depth test */
    cc_state->cc2.logicop_enable = 0;   /* disable logic op */
    cc_state->cc3.ia_blend_enable = 0 ;  /* blend alpha just like colors */
    cc_state->cc3.blend_enable = 1;     /* enable color blend */
    cc_state->cc3.alpha_test = 0;       /* disable alpha test */
    cc_state->cc3.alpha_test_format = 0;//0:ALPHATEST_UNORM8;       /*store alpha value with UNORM8 */
    cc_state->cc3.alpha_test_func = 5;//COMPAREFUNCTION_LESS;       /*pass if less than the reference */
    cc_state->cc4.cc_viewport_state_offset = render_state->cc.viewport->offset >> 5;

    cc_state->cc5.dither_enable = 0;    /* disable dither */
    cc_state->cc5.logicop_func = 0xc;   /* WHITE */
    cc_state->cc5.statistics_enable = 1;
    cc_state->cc5.ia_blend_function = I965_BLENDFUNCTION_ADD;
    cc_state->cc5.ia_src_blend_factor = I965_BLENDFACTOR_DST_ALPHA;
    cc_state->cc5.ia_dest_blend_factor = I965_BLENDFACTOR_DST_ALPHA;

    cc_state->cc6.clamp_post_alpha_blend = 0; 
    cc_state->cc6.clamp_pre_alpha_blend  =0; 
    
    /*final color = src_color*src_blend_factor +/- dst_color*dest_color_blend_factor*/
    cc_state->cc6.blend_function = I965_BLENDFUNCTION_ADD;
    cc_state->cc6.src_blend_factor = I965_BLENDFACTOR_SRC_ALPHA;
    cc_state->cc6.dest_blend_factor = I965_BLENDFACTOR_INV_SRC_ALPHA;
   
    /*alpha test reference*/
    cc_state->cc7.alpha_ref.f =0.0 ;


    dri_bo_emit_reloc(render_state->cc.state,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      0,
                      offsetof(struct i965_cc_unit_state, cc4),
                      render_state->cc.viewport);

    dri_bo_unmap(render_state->cc.state);
}


static void 
i965_render_cc_unit(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct i965_cc_unit_state *cc_state;

    assert(render_state->cc.viewport);

    dri_bo_map(render_state->cc.state, 1);
    assert(render_state->cc.state->virtual);
    cc_state = render_state->cc.state->virtual;
    memset(cc_state, 0, sizeof(*cc_state));

    cc_state->cc0.stencil_enable = 0;   /* disable stencil */
    cc_state->cc2.depth_test = 0;       /* disable depth test */
    cc_state->cc2.logicop_enable = 1;   /* enable logic op */
    cc_state->cc3.ia_blend_enable = 0;  /* blend alpha just like colors */
    cc_state->cc3.blend_enable = 0;     /* disable color blend */
    cc_state->cc3.alpha_test = 0;       /* disable alpha test */
    cc_state->cc4.cc_viewport_state_offset = render_state->cc.viewport->offset >> 5;

    cc_state->cc5.dither_enable = 0;    /* disable dither */
    cc_state->cc5.logicop_func = 0xc;   /* WHITE */
    cc_state->cc5.statistics_enable = 1;
    cc_state->cc5.ia_blend_function = I965_BLENDFUNCTION_ADD;
    cc_state->cc5.ia_src_blend_factor = I965_BLENDFACTOR_ONE;
    cc_state->cc5.ia_dest_blend_factor = I965_BLENDFACTOR_ONE;

    dri_bo_emit_reloc(render_state->cc.state,
                      I915_GEM_DOMAIN_INSTRUCTION, 0,
                      0,
                      offsetof(struct i965_cc_unit_state, cc4),
                      render_state->cc.viewport);

    dri_bo_unmap(render_state->cc.state);
}

static void
i965_render_set_surface_tiling(struct i965_surface_state *ss, unsigned int tiling)
{
    switch (tiling) {
    case I915_TILING_NONE:
        ss->ss3.tiled_surface = 0;
        ss->ss3.tile_walk = 0;
        break;
    case I915_TILING_X:
        ss->ss3.tiled_surface = 1;
        ss->ss3.tile_walk = I965_TILEWALK_XMAJOR;
        break;
    case I915_TILING_Y:
        ss->ss3.tiled_surface = 1;
        ss->ss3.tile_walk = I965_TILEWALK_YMAJOR;
        break;
    }
}

static void
i965_render_set_surface_state(
    struct i965_surface_state *ss,
    dri_bo                    *bo,
    unsigned long              offset,
    unsigned int               width,
    unsigned int               height,
    unsigned int               pitch,
    unsigned int               format,
    unsigned int               flags
)
{
    unsigned int tiling;
    unsigned int swizzle;

    memset(ss, 0, sizeof(*ss));

    switch (flags & (I965_PP_FLAG_TOP_FIELD|I965_PP_FLAG_BOTTOM_FIELD)) {
    case I965_PP_FLAG_BOTTOM_FIELD:
        ss->ss0.vert_line_stride_ofs = 1;
        /* fall-through */
    case I965_PP_FLAG_TOP_FIELD:
        ss->ss0.vert_line_stride = 1;
        height /= 2;
        break;
    }

    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = format;
    ss->ss0.color_blend = 1;

    ss->ss1.base_addr = bo->offset + offset;

    ss->ss2.width = width - 1;
    ss->ss2.height = height - 1;

    ss->ss3.pitch = pitch - 1;

    dri_bo_get_tiling(bo, &tiling, &swizzle);
    i965_render_set_surface_tiling(ss, tiling);
}

static void
gen7_render_set_surface_tiling(struct gen7_surface_state *ss, uint32_t tiling)
{
   switch (tiling) {
   case I915_TILING_NONE:
      ss->ss0.tiled_surface = 0;
      ss->ss0.tile_walk = 0;
      break;
   case I915_TILING_X:
      ss->ss0.tiled_surface = 1;
      ss->ss0.tile_walk = I965_TILEWALK_XMAJOR;
      break;
   case I915_TILING_Y:
      ss->ss0.tiled_surface = 1;
      ss->ss0.tile_walk = I965_TILEWALK_YMAJOR;
      break;
   }
}

/* Set "Shader Channel Select" */
void
gen7_render_set_surface_scs(struct gen7_surface_state *ss)
{
    ss->ss7.shader_chanel_select_r = HSW_SCS_RED;
    ss->ss7.shader_chanel_select_g = HSW_SCS_GREEN;
    ss->ss7.shader_chanel_select_b = HSW_SCS_BLUE;
    ss->ss7.shader_chanel_select_a = HSW_SCS_ALPHA;
}

static void
gen7_render_set_surface_state(
    struct gen7_surface_state *ss,
    dri_bo                    *bo,
    unsigned long              offset,
    int                        width,
    int                        height,
    int                        pitch,
    int                        format,
    unsigned int               flags
)
{
    unsigned int tiling;
    unsigned int swizzle;

    memset(ss, 0, sizeof(*ss));

    switch (flags & (I965_PP_FLAG_TOP_FIELD|I965_PP_FLAG_BOTTOM_FIELD)) {
    case I965_PP_FLAG_BOTTOM_FIELD:
        ss->ss0.vert_line_stride_ofs = 1;
        /* fall-through */
    case I965_PP_FLAG_TOP_FIELD:
        ss->ss0.vert_line_stride = 1;
        height /= 2;
        break;
    }

    ss->ss0.surface_type = I965_SURFACE_2D;
    ss->ss0.surface_format = format;

    ss->ss1.base_addr = bo->offset + offset;

    ss->ss2.width = width - 1;
    ss->ss2.height = height - 1;

    ss->ss3.pitch = pitch - 1;

    dri_bo_get_tiling(bo, &tiling, &swizzle);
    gen7_render_set_surface_tiling(ss, tiling);
}


static void
i965_render_src_surface_state(
    VADriverContextP ctx, 
    int              index,
    dri_bo          *region,
    unsigned long    offset,
    int              w,
    int              h,
    int              pitch,
    int              format,
    unsigned int     flags
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);  
    struct i965_render_state *render_state = &i965->render_state;
    void *ss;
    dri_bo *ss_bo = render_state->wm.surface_state_binding_table_bo;

    assert(index < MAX_RENDER_SURFACES);

    dri_bo_map(ss_bo, 1);
    assert(ss_bo->virtual);
    ss = (char *)ss_bo->virtual + SURFACE_STATE_OFFSET(index);

    if (IS_GEN7(i965->intel.device_info)) {
        gen7_render_set_surface_state(ss,
                                      region, offset,
                                      w, h,
                                      pitch, format, flags);
        if (IS_HASWELL(i965->intel.device_info))
            gen7_render_set_surface_scs(ss);
        dri_bo_emit_reloc(ss_bo,
                          I915_GEM_DOMAIN_SAMPLER, 0,
                          offset,
                          SURFACE_STATE_OFFSET(index) + offsetof(struct gen7_surface_state, ss1),
                          region);
    } else {
        i965_render_set_surface_state(ss,
                                      region, offset,
                                      w, h,
                                      pitch, format, flags);
        dri_bo_emit_reloc(ss_bo,
                          I915_GEM_DOMAIN_SAMPLER, 0,
                          offset,
                          SURFACE_STATE_OFFSET(index) + offsetof(struct i965_surface_state, ss1),
                          region);
    }

    ((unsigned int *)((char *)ss_bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(ss_bo);
    render_state->wm.sampler_count++;
}

static void
i965_render_src_surfaces_state(
    VADriverContextP ctx,
    struct object_surface *obj_surface,
    unsigned int     flags
)
{
    int region_pitch;
    int rw, rh;
    dri_bo *region;

    region_pitch = obj_surface->width;
    rw = obj_surface->orig_width;
    rh = obj_surface->orig_height;
    region = obj_surface->bo;

    i965_render_src_surface_state(ctx, 1, region, 0, rw, rh, region_pitch, I965_SURFACEFORMAT_R8_UNORM, flags);     /* Y */
    i965_render_src_surface_state(ctx, 2, region, 0, rw, rh, region_pitch, I965_SURFACEFORMAT_R8_UNORM, flags);

    if (obj_surface->fourcc == VA_FOURCC_Y800) /* single plane for grayscale */
        return;

    if (obj_surface->fourcc == VA_FOURCC_NV12) {
        i965_render_src_surface_state(ctx, 3, region,
                                      region_pitch * obj_surface->y_cb_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8G8_UNORM, flags); /* UV */
        i965_render_src_surface_state(ctx, 4, region,
                                      region_pitch * obj_surface->y_cb_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8G8_UNORM, flags);
    } else {
        i965_render_src_surface_state(ctx, 3, region,
                                      region_pitch * obj_surface->y_cb_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8_UNORM, flags); /* U */
        i965_render_src_surface_state(ctx, 4, region,
                                      region_pitch * obj_surface->y_cb_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8_UNORM, flags);
        i965_render_src_surface_state(ctx, 5, region,
                                      region_pitch * obj_surface->y_cr_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8_UNORM, flags); /* V */
        i965_render_src_surface_state(ctx, 6, region,
                                      region_pitch * obj_surface->y_cr_offset,
                                      obj_surface->cb_cr_width, obj_surface->cb_cr_height, obj_surface->cb_cr_pitch,
                                      I965_SURFACEFORMAT_R8_UNORM, flags);
    }
}

static void
i965_subpic_render_src_surfaces_state(VADriverContextP ctx,
                                      struct object_surface *obj_surface)
{
    dri_bo *subpic_region;
    unsigned int index = obj_surface->subpic_render_idx;
    struct object_subpic *obj_subpic = obj_surface->obj_subpic[index];
    struct object_image *obj_image = obj_subpic->obj_image;

    assert(obj_surface);
    assert(obj_surface->bo);
    subpic_region = obj_image->bo;
    /*subpicture surface*/
    i965_render_src_surface_state(ctx, 1, subpic_region, 0, obj_subpic->width, obj_subpic->height, obj_subpic->pitch, obj_subpic->format, 0);     
    i965_render_src_surface_state(ctx, 2, subpic_region, 0, obj_subpic->width, obj_subpic->height, obj_subpic->pitch, obj_subpic->format, 0);     
}

static void
i965_render_dest_surface_state(VADriverContextP ctx, int index)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);  
    struct i965_render_state *render_state = &i965->render_state;
    struct intel_region *dest_region = render_state->draw_region;
    void *ss;
    dri_bo *ss_bo = render_state->wm.surface_state_binding_table_bo;
    int format;
    assert(index < MAX_RENDER_SURFACES);

    if (dest_region->cpp == 2) {
	format = I965_SURFACEFORMAT_B5G6R5_UNORM;
    } else {
	format = I965_SURFACEFORMAT_B8G8R8A8_UNORM;
    }

    dri_bo_map(ss_bo, 1);
    assert(ss_bo->virtual);
    ss = (char *)ss_bo->virtual + SURFACE_STATE_OFFSET(index);

    if (IS_GEN7(i965->intel.device_info)) {
        gen7_render_set_surface_state(ss,
                                      dest_region->bo, 0,
                                      dest_region->width, dest_region->height,
                                      dest_region->pitch, format, 0);
        if (IS_HASWELL(i965->intel.device_info))
            gen7_render_set_surface_scs(ss);
        dri_bo_emit_reloc(ss_bo,
                          I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
                          0,
                          SURFACE_STATE_OFFSET(index) + offsetof(struct gen7_surface_state, ss1),
                          dest_region->bo);
    } else {
        i965_render_set_surface_state(ss,
                                      dest_region->bo, 0,
                                      dest_region->width, dest_region->height,
                                      dest_region->pitch, format, 0);
        dri_bo_emit_reloc(ss_bo,
                          I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
                          0,
                          SURFACE_STATE_OFFSET(index) + offsetof(struct i965_surface_state, ss1),
                          dest_region->bo);
    }

    ((unsigned int *)((char *)ss_bo->virtual + BINDING_TABLE_OFFSET))[index] = SURFACE_STATE_OFFSET(index);
    dri_bo_unmap(ss_bo);
}

static void
i965_fill_vertex_buffer(
    VADriverContextP ctx,
    float tex_coords[4], /* [(u1,v1);(u2,v2)] */
    float vid_coords[4]  /* [(x1,y1);(x2,y2)] */
)
{
    struct i965_driver_data * const i965 = i965_driver_data(ctx);
    float vb[12];

    enum { X1, Y1, X2, Y2 };

    static const unsigned int g_rotation_indices[][6] = {
        [VA_ROTATION_NONE] = { X2, Y2, X1, Y2, X1, Y1 },
        [VA_ROTATION_90]   = { X2, Y1, X2, Y2, X1, Y2 },
        [VA_ROTATION_180]  = { X1, Y1, X2, Y1, X2, Y2 },
        [VA_ROTATION_270]  = { X1, Y2, X1, Y1, X2, Y1 },
    };

    const unsigned int * const rotation_indices =
        g_rotation_indices[i965->rotation_attrib->value];

    vb[0]  = tex_coords[rotation_indices[0]]; /* bottom-right corner */
    vb[1]  = tex_coords[rotation_indices[1]];
    vb[2]  = vid_coords[X2];
    vb[3]  = vid_coords[Y2];

    vb[4]  = tex_coords[rotation_indices[2]]; /* bottom-left corner */
    vb[5]  = tex_coords[rotation_indices[3]];
    vb[6]  = vid_coords[X1];
    vb[7]  = vid_coords[Y2];

    vb[8]  = tex_coords[rotation_indices[4]]; /* top-left corner */
    vb[9]  = tex_coords[rotation_indices[5]];
    vb[10] = vid_coords[X1];
    vb[11] = vid_coords[Y1];

    dri_bo_subdata(i965->render_state.vb.vertex_buffer, 0, sizeof(vb), vb);
}

static void 
i965_subpic_render_upload_vertex(VADriverContextP ctx,
                                 struct object_surface *obj_surface,
                                 const VARectangle *output_rect)
{    
    unsigned int index = obj_surface->subpic_render_idx;
    struct object_subpic     *obj_subpic   = obj_surface->obj_subpic[index];
    float tex_coords[4], vid_coords[4];
    VARectangle dst_rect;

    if (obj_subpic->flags & VA_SUBPICTURE_DESTINATION_IS_SCREEN_COORD)
        dst_rect = obj_subpic->dst_rect;
    else {
        const float sx  = (float)output_rect->width  / obj_surface->orig_width;
        const float sy  = (float)output_rect->height / obj_surface->orig_height;
        dst_rect.x      = output_rect->x + sx * obj_subpic->dst_rect.x;
        dst_rect.y      = output_rect->y + sy * obj_subpic->dst_rect.y;
        dst_rect.width  = sx * obj_subpic->dst_rect.width;
        dst_rect.height = sy * obj_subpic->dst_rect.height;
    }

    tex_coords[0] = (float)obj_subpic->src_rect.x / obj_subpic->width;
    tex_coords[1] = (float)obj_subpic->src_rect.y / obj_subpic->height;
    tex_coords[2] = (float)(obj_subpic->src_rect.x + obj_subpic->src_rect.width) / obj_subpic->width;
    tex_coords[3] = (float)(obj_subpic->src_rect.y + obj_subpic->src_rect.height) / obj_subpic->height;

    vid_coords[0] = dst_rect.x;
    vid_coords[1] = dst_rect.y;
    vid_coords[2] = (float)(dst_rect.x + dst_rect.width);
    vid_coords[3] = (float)(dst_rect.y + dst_rect.height);

    i965_fill_vertex_buffer(ctx, tex_coords, vid_coords);
}

static void 
i965_render_upload_vertex(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct intel_region *dest_region = render_state->draw_region;
    float tex_coords[4], vid_coords[4];
    int width, height;

    width  = obj_surface->orig_width;
    height = obj_surface->orig_height;

    tex_coords[0] = (float)src_rect->x / width;
    tex_coords[1] = (float)src_rect->y / height;
    tex_coords[2] = (float)(src_rect->x + src_rect->width) / width;
    tex_coords[3] = (float)(src_rect->y + src_rect->height) / height;

    vid_coords[0] = dest_region->x + dst_rect->x;
    vid_coords[1] = dest_region->y + dst_rect->y;
    vid_coords[2] = vid_coords[0] + dst_rect->width;
    vid_coords[3] = vid_coords[1] + dst_rect->height;

    i965_fill_vertex_buffer(ctx, tex_coords, vid_coords);
}

#define PI  3.1415926

static void
i965_render_upload_constants(VADriverContextP ctx,
                             struct object_surface *obj_surface,
                             unsigned int flags)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    unsigned short *constant_buffer;
    float *color_balance_base;
    float contrast = (float)i965->contrast_attrib->value / DEFAULT_CONTRAST;
    float brightness = (float)i965->brightness_attrib->value / 255; /* YUV is float in the shader */
    float hue = (float)i965->hue_attrib->value / 180 * PI;
    float saturation = (float)i965->saturation_attrib->value / DEFAULT_SATURATION;
    float *yuv_to_rgb;
    unsigned int color_flag;

    dri_bo_map(render_state->curbe.bo, 1);
    assert(render_state->curbe.bo->virtual);
    constant_buffer = render_state->curbe.bo->virtual;

    if (obj_surface->subsampling == SUBSAMPLE_YUV400) {
        assert(obj_surface->fourcc == VA_FOURCC_Y800);

        constant_buffer[0] = 2;
    } else {
        if (obj_surface->fourcc == VA_FOURCC_NV12)
            constant_buffer[0] = 1;
        else
            constant_buffer[0] = 0;
    }

    if (i965->contrast_attrib->value == DEFAULT_CONTRAST &&
        i965->brightness_attrib->value == DEFAULT_BRIGHTNESS &&
        i965->hue_attrib->value == DEFAULT_HUE &&
        i965->saturation_attrib->value == DEFAULT_SATURATION)
        constant_buffer[1] = 1; /* skip color balance transformation */
    else
        constant_buffer[1] = 0;

    color_balance_base = (float *)constant_buffer + 4;
    *color_balance_base++ = contrast;
    *color_balance_base++ = brightness;
    *color_balance_base++ = cos(hue) * contrast * saturation;
    *color_balance_base++ = sin(hue) * contrast * saturation;

    color_flag = flags & VA_SRC_COLOR_MASK;
    yuv_to_rgb = (float *)constant_buffer + 8;
    if (color_flag == VA_SRC_BT709)
        memcpy(yuv_to_rgb, yuv_to_rgb_bt709, sizeof(yuv_to_rgb_bt709));
    else if (color_flag == VA_SRC_SMPTE_240)
        memcpy(yuv_to_rgb, yuv_to_rgb_smpte_240, sizeof(yuv_to_rgb_smpte_240));
    else
        memcpy(yuv_to_rgb, yuv_to_rgb_bt601, sizeof(yuv_to_rgb_bt601));

    dri_bo_unmap(render_state->curbe.bo);
}

static void
i965_subpic_render_upload_constants(VADriverContextP ctx,
                                    struct object_surface *obj_surface)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    float *constant_buffer;
    float global_alpha = 1.0;
    unsigned int index = obj_surface->subpic_render_idx;
    struct object_subpic *obj_subpic = obj_surface->obj_subpic[index];
    
    if (obj_subpic->flags & VA_SUBPICTURE_GLOBAL_ALPHA) {
        global_alpha = obj_subpic->global_alpha;
    }

    dri_bo_map(render_state->curbe.bo, 1);

    assert(render_state->curbe.bo->virtual);
    constant_buffer = render_state->curbe.bo->virtual;
    *constant_buffer = global_alpha;

    dri_bo_unmap(render_state->curbe.bo);
}
 
static void
i965_surface_render_state_setup(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags
)
{
    i965_render_vs_unit(ctx);
    i965_render_sf_unit(ctx);
    i965_render_dest_surface_state(ctx, 0);
    i965_render_src_surfaces_state(ctx, obj_surface, flags);
    i965_render_sampler(ctx);
    i965_render_wm_unit(ctx);
    i965_render_cc_viewport(ctx);
    i965_render_cc_unit(ctx);
    i965_render_upload_vertex(ctx, obj_surface, src_rect, dst_rect);
    i965_render_upload_constants(ctx, obj_surface, flags);
}

static void
i965_subpic_render_state_setup(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    i965_render_vs_unit(ctx);
    i965_render_sf_unit(ctx);
    i965_render_dest_surface_state(ctx, 0);
    i965_subpic_render_src_surfaces_state(ctx, obj_surface);
    i965_render_sampler(ctx);
    i965_subpic_render_wm_unit(ctx);
    i965_render_cc_viewport(ctx);
    i965_subpic_render_cc_unit(ctx);
    i965_subpic_render_upload_constants(ctx, obj_surface);
    i965_subpic_render_upload_vertex(ctx, obj_surface, dst_rect);
}


static void
i965_render_pipeline_select(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
 
    BEGIN_BATCH(batch, 1);
    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_3D);
    ADVANCE_BATCH(batch);
}

static void
i965_render_state_sip(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, CMD_STATE_SIP | 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
i965_render_state_base_address(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    if (IS_IRONLAKE(i965->intel.device_info)) {
        BEGIN_BATCH(batch, 8);
        OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | 6);
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
        OUT_RELOC(batch, render_state->wm.surface_state_binding_table_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY);
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
        ADVANCE_BATCH(batch);
    } else {
        BEGIN_BATCH(batch, 6);
        OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | 4);
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
        OUT_RELOC(batch, render_state->wm.surface_state_binding_table_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY);
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
        OUT_BATCH(batch, 0 | BASE_ADDRESS_MODIFY);
        ADVANCE_BATCH(batch);
    }
}

static void
i965_render_binding_table_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 6);
    OUT_BATCH(batch, CMD_BINDING_TABLE_POINTERS | 4);
    OUT_BATCH(batch, 0); /* vs */
    OUT_BATCH(batch, 0); /* gs */
    OUT_BATCH(batch, 0); /* clip */
    OUT_BATCH(batch, 0); /* sf */
    OUT_BATCH(batch, BINDING_TABLE_OFFSET);
    ADVANCE_BATCH(batch);
}

static void 
i965_render_constant_color(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 5);
    OUT_BATCH(batch, CMD_CONSTANT_COLOR | 3);
    OUT_BATCH(batch, float_to_uint(1.0));
    OUT_BATCH(batch, float_to_uint(0.0));
    OUT_BATCH(batch, float_to_uint(1.0));
    OUT_BATCH(batch, float_to_uint(1.0));
    ADVANCE_BATCH(batch);
}

static void
i965_render_pipelined_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 7);
    OUT_BATCH(batch, CMD_PIPELINED_POINTERS | 5);
    OUT_RELOC(batch, render_state->vs.state, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
    OUT_BATCH(batch, 0);  /* disable GS */
    OUT_BATCH(batch, 0);  /* disable CLIP */
    OUT_RELOC(batch, render_state->sf.state, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
    OUT_RELOC(batch, render_state->wm.state, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
    OUT_RELOC(batch, render_state->cc.state, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
    ADVANCE_BATCH(batch);
}

static void
i965_render_urb_layout(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    int urb_vs_start, urb_vs_size;
    int urb_gs_start, urb_gs_size;
    int urb_clip_start, urb_clip_size;
    int urb_sf_start, urb_sf_size;
    int urb_cs_start, urb_cs_size;

    urb_vs_start = 0;
    urb_vs_size = URB_VS_ENTRIES * URB_VS_ENTRY_SIZE;
    urb_gs_start = urb_vs_start + urb_vs_size;
    urb_gs_size = URB_GS_ENTRIES * URB_GS_ENTRY_SIZE;
    urb_clip_start = urb_gs_start + urb_gs_size;
    urb_clip_size = URB_CLIP_ENTRIES * URB_CLIP_ENTRY_SIZE;
    urb_sf_start = urb_clip_start + urb_clip_size;
    urb_sf_size = URB_SF_ENTRIES * URB_SF_ENTRY_SIZE;
    urb_cs_start = urb_sf_start + urb_sf_size;
    urb_cs_size = URB_CS_ENTRIES * URB_CS_ENTRY_SIZE;

    BEGIN_BATCH(batch, 3);
    OUT_BATCH(batch, 
              CMD_URB_FENCE |
              UF0_CS_REALLOC |
              UF0_SF_REALLOC |
              UF0_CLIP_REALLOC |
              UF0_GS_REALLOC |
              UF0_VS_REALLOC |
              1);
    OUT_BATCH(batch, 
              ((urb_clip_start + urb_clip_size) << UF1_CLIP_FENCE_SHIFT) |
              ((urb_gs_start + urb_gs_size) << UF1_GS_FENCE_SHIFT) |
              ((urb_vs_start + urb_vs_size) << UF1_VS_FENCE_SHIFT));
    OUT_BATCH(batch,
              ((urb_cs_start + urb_cs_size) << UF2_CS_FENCE_SHIFT) |
              ((urb_sf_start + urb_sf_size) << UF2_SF_FENCE_SHIFT));
    ADVANCE_BATCH(batch);
}

static void 
i965_render_cs_urb_layout(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, CMD_CS_URB_STATE | 0);
    OUT_BATCH(batch,
              ((URB_CS_ENTRY_SIZE - 1) << 4) |          /* URB Entry Allocation Size */
              (URB_CS_ENTRIES << 0));                /* Number of URB Entries */
    ADVANCE_BATCH(batch);
}

static void
i965_render_constant_buffer(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, CMD_CONSTANT_BUFFER | (1 << 8) | (2 - 2));
    OUT_RELOC(batch, render_state->curbe.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              URB_CS_ENTRY_SIZE - 1);
    ADVANCE_BATCH(batch);    
}

static void
i965_render_drawing_rectangle(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;
    struct intel_region *dest_region = render_state->draw_region;

    BEGIN_BATCH(batch, 4);
    OUT_BATCH(batch, CMD_DRAWING_RECTANGLE | 2);
    OUT_BATCH(batch, 0x00000000);
    OUT_BATCH(batch, (dest_region->width - 1) | (dest_region->height - 1) << 16);
    OUT_BATCH(batch, 0x00000000);         
    ADVANCE_BATCH(batch);
}

static void
i965_render_vertex_elements(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    if (IS_IRONLAKE(i965->intel.device_info)) {
        BEGIN_BATCH(batch, 5);
        OUT_BATCH(batch, CMD_VERTEX_ELEMENTS | 3);
        /* offset 0: X,Y -> {X, Y, 1.0, 1.0} */
        OUT_BATCH(batch, (0 << VE0_VERTEX_BUFFER_INDEX_SHIFT) |
                  VE0_VALID |
                  (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
                  (0 << VE0_OFFSET_SHIFT));
        OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT) |
                  (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT) |
                  (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT) |
                  (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT));
        /* offset 8: S0, T0 -> {S0, T0, 1.0, 1.0} */
        OUT_BATCH(batch, (0 << VE0_VERTEX_BUFFER_INDEX_SHIFT) |
                  VE0_VALID |
                  (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
                  (8 << VE0_OFFSET_SHIFT));
        OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT) |
                  (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT) |
                  (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT) |
                  (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT));
        ADVANCE_BATCH(batch);
    } else {
        BEGIN_BATCH(batch, 5);
        OUT_BATCH(batch, CMD_VERTEX_ELEMENTS | 3);
        /* offset 0: X,Y -> {X, Y, 1.0, 1.0} */
        OUT_BATCH(batch, (0 << VE0_VERTEX_BUFFER_INDEX_SHIFT) |
                  VE0_VALID |
                  (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
                  (0 << VE0_OFFSET_SHIFT));
        OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT) |
                  (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT) |
                  (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT) |
                  (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT) |
                  (0 << VE1_DESTINATION_ELEMENT_OFFSET_SHIFT));
        /* offset 8: S0, T0 -> {S0, T0, 1.0, 1.0} */
        OUT_BATCH(batch, (0 << VE0_VERTEX_BUFFER_INDEX_SHIFT) |
                  VE0_VALID |
                  (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
                  (8 << VE0_OFFSET_SHIFT));
        OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT) |
                  (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT) |
                  (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT) |
                  (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT) |
                  (4 << VE1_DESTINATION_ELEMENT_OFFSET_SHIFT));
        ADVANCE_BATCH(batch);
    }
}

static void
i965_render_upload_image_palette(
    VADriverContextP ctx,
    struct object_image *obj_image,
    unsigned int     alpha
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    unsigned int i;

    assert(obj_image);

    if (!obj_image)
        return;

    if (obj_image->image.num_palette_entries == 0)
        return;

    BEGIN_BATCH(batch, 1 + obj_image->image.num_palette_entries);
    OUT_BATCH(batch, CMD_SAMPLER_PALETTE_LOAD | (obj_image->image.num_palette_entries - 1));
    /*fill palette*/
    //int32_t out[16]; //0-23:color 23-31:alpha
    for (i = 0; i < obj_image->image.num_palette_entries; i++)
        OUT_BATCH(batch, (alpha << 24) | obj_image->palette[i]);
    ADVANCE_BATCH(batch);
}

static void
i965_render_startup(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 11);
    OUT_BATCH(batch, CMD_VERTEX_BUFFERS | 3);
    OUT_BATCH(batch, 
              (0 << VB0_BUFFER_INDEX_SHIFT) |
              VB0_VERTEXDATA |
              ((4 * 4) << VB0_BUFFER_PITCH_SHIFT));
    OUT_RELOC(batch, render_state->vb.vertex_buffer, I915_GEM_DOMAIN_VERTEX, 0, 0);

    if (IS_IRONLAKE(i965->intel.device_info))
        OUT_RELOC(batch, render_state->vb.vertex_buffer, I915_GEM_DOMAIN_VERTEX, 0, 12 * 4);
    else
        OUT_BATCH(batch, 3);

    OUT_BATCH(batch, 0);

    OUT_BATCH(batch, 
              CMD_3DPRIMITIVE |
              _3DPRIMITIVE_VERTEX_SEQUENTIAL |
              (_3DPRIM_RECTLIST << _3DPRIMITIVE_TOPOLOGY_SHIFT) |
              (0 << 9) |
              4);
    OUT_BATCH(batch, 3); /* vertex count per instance */
    OUT_BATCH(batch, 0); /* start vertex offset */
    OUT_BATCH(batch, 1); /* single instance */
    OUT_BATCH(batch, 0); /* start instance location */
    OUT_BATCH(batch, 0); /* index buffer offset, ignored */
    ADVANCE_BATCH(batch);
}

static void 
i965_clear_dest_region(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;
    struct intel_region *dest_region = render_state->draw_region;
    unsigned int blt_cmd, br13;
    int pitch;

    blt_cmd = XY_COLOR_BLT_CMD;
    br13 = 0xf0 << 16;
    pitch = dest_region->pitch;

    if (dest_region->cpp == 4) {
        br13 |= BR13_8888;
        blt_cmd |= (XY_COLOR_BLT_WRITE_RGB | XY_COLOR_BLT_WRITE_ALPHA);
    } else {
        assert(dest_region->cpp == 2);
        br13 |= BR13_565;
    }

    if (dest_region->tiling != I915_TILING_NONE) {
        blt_cmd |= XY_COLOR_BLT_DST_TILED;
        pitch /= 4;
    }

    br13 |= pitch;

    if (IS_GEN6(i965->intel.device_info) ||
        IS_GEN7(i965->intel.device_info)) {
        intel_batchbuffer_start_atomic_blt(batch, 24);
        BEGIN_BLT_BATCH(batch, 6);
    } else {
        intel_batchbuffer_start_atomic(batch, 24);
        BEGIN_BATCH(batch, 6);
    }

    OUT_BATCH(batch, blt_cmd);
    OUT_BATCH(batch, br13);
    OUT_BATCH(batch, (dest_region->y << 16) | (dest_region->x));
    OUT_BATCH(batch, ((dest_region->y + dest_region->height) << 16) |
              (dest_region->x + dest_region->width));
    OUT_RELOC(batch, dest_region->bo, 
              I915_GEM_DOMAIN_RENDER, I915_GEM_DOMAIN_RENDER,
              0);
    OUT_BATCH(batch, 0x0);
    ADVANCE_BATCH(batch);
    intel_batchbuffer_end_atomic(batch);
}

static void
i965_surface_render_pipeline_setup(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    i965_clear_dest_region(ctx);
    intel_batchbuffer_start_atomic(batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(batch);
    i965_render_pipeline_select(ctx);
    i965_render_state_sip(ctx);
    i965_render_state_base_address(ctx);
    i965_render_binding_table_pointers(ctx);
    i965_render_constant_color(ctx);
    i965_render_pipelined_pointers(ctx);
    i965_render_urb_layout(ctx);
    i965_render_cs_urb_layout(ctx);
    i965_render_constant_buffer(ctx);
    i965_render_drawing_rectangle(ctx);
    i965_render_vertex_elements(ctx);
    i965_render_startup(ctx);
    intel_batchbuffer_end_atomic(batch);
}

static void
i965_subpic_render_pipeline_setup(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    intel_batchbuffer_start_atomic(batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(batch);
    i965_render_pipeline_select(ctx);
    i965_render_state_sip(ctx);
    i965_render_state_base_address(ctx);
    i965_render_binding_table_pointers(ctx);
    i965_render_constant_color(ctx);
    i965_render_pipelined_pointers(ctx);
    i965_render_urb_layout(ctx);
    i965_render_cs_urb_layout(ctx);
    i965_render_constant_buffer(ctx);
    i965_render_drawing_rectangle(ctx);
    i965_render_vertex_elements(ctx);
    i965_render_startup(ctx);
    intel_batchbuffer_end_atomic(batch);
}


static void 
i965_render_initialize(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    dri_bo *bo;

    /* VERTEX BUFFER */
    dri_bo_unreference(render_state->vb.vertex_buffer);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vertex buffer",
                      4096,
                      4096);
    assert(bo);
    render_state->vb.vertex_buffer = bo;

    /* VS */
    dri_bo_unreference(render_state->vs.state);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vs state",
                      sizeof(struct i965_vs_unit_state),
                      64);
    assert(bo);
    render_state->vs.state = bo;

    /* GS */
    /* CLIP */
    /* SF */
    dri_bo_unreference(render_state->sf.state);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "sf state",
                      sizeof(struct i965_sf_unit_state),
                      64);
    assert(bo);
    render_state->sf.state = bo;

    /* WM */
    dri_bo_unreference(render_state->wm.surface_state_binding_table_bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      (SURFACE_STATE_PADDED_SIZE + sizeof(unsigned int)) * MAX_RENDER_SURFACES,
                      4096);
    assert(bo);
    render_state->wm.surface_state_binding_table_bo = bo;

    dri_bo_unreference(render_state->wm.sampler);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "sampler state",
                      MAX_SAMPLERS * sizeof(struct i965_sampler_state),
                      64);
    assert(bo);
    render_state->wm.sampler = bo;
    render_state->wm.sampler_count = 0;

    dri_bo_unreference(render_state->wm.state);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "wm state",
                      sizeof(struct i965_wm_unit_state),
                      64);
    assert(bo);
    render_state->wm.state = bo;

    /* COLOR CALCULATOR */
    dri_bo_unreference(render_state->cc.state);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "color calc state",
                      sizeof(struct i965_cc_unit_state),
                      64);
    assert(bo);
    render_state->cc.state = bo;

    dri_bo_unreference(render_state->cc.viewport);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "cc viewport",
                      sizeof(struct i965_cc_viewport),
                      64);
    assert(bo);
    render_state->cc.viewport = bo;
}

static void
i965_render_put_surface(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    i965_render_initialize(ctx);
    i965_surface_render_state_setup(ctx, obj_surface, src_rect, dst_rect, flags);
    i965_surface_render_pipeline_setup(ctx);
    intel_batchbuffer_flush(batch);
}

static void
i965_render_put_subpicture(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    unsigned int index = obj_surface->subpic_render_idx;
    struct object_subpic *obj_subpic = obj_surface->obj_subpic[index];

    assert(obj_subpic);

    i965_render_initialize(ctx);
    i965_subpic_render_state_setup(ctx, obj_surface, src_rect, dst_rect);
    i965_subpic_render_pipeline_setup(ctx);
    i965_render_upload_image_palette(ctx, obj_subpic->obj_image, 0xff);
    intel_batchbuffer_flush(batch);
}

/*
 * for GEN6+
 */
static void 
gen6_render_initialize(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    dri_bo *bo;

    /* VERTEX BUFFER */
    dri_bo_unreference(render_state->vb.vertex_buffer);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vertex buffer",
                      4096,
                      4096);
    assert(bo);
    render_state->vb.vertex_buffer = bo;

    /* WM */
    dri_bo_unreference(render_state->wm.surface_state_binding_table_bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      (SURFACE_STATE_PADDED_SIZE + sizeof(unsigned int)) * MAX_RENDER_SURFACES,
                      4096);
    assert(bo);
    render_state->wm.surface_state_binding_table_bo = bo;

    dri_bo_unreference(render_state->wm.sampler);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "sampler state",
                      MAX_SAMPLERS * sizeof(struct i965_sampler_state),
                      4096);
    assert(bo);
    render_state->wm.sampler = bo;
    render_state->wm.sampler_count = 0;

    /* COLOR CALCULATOR */
    dri_bo_unreference(render_state->cc.state);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "color calc state",
                      sizeof(struct gen6_color_calc_state),
                      4096);
    assert(bo);
    render_state->cc.state = bo;

    /* CC VIEWPORT */
    dri_bo_unreference(render_state->cc.viewport);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "cc viewport",
                      sizeof(struct i965_cc_viewport),
                      4096);
    assert(bo);
    render_state->cc.viewport = bo;

    /* BLEND STATE */
    dri_bo_unreference(render_state->cc.blend);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "blend state",
                      sizeof(struct gen6_blend_state),
                      4096);
    assert(bo);
    render_state->cc.blend = bo;

    /* DEPTH & STENCIL STATE */
    dri_bo_unreference(render_state->cc.depth_stencil);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "depth & stencil state",
                      sizeof(struct gen6_depth_stencil_state),
                      4096);
    assert(bo);
    render_state->cc.depth_stencil = bo;
}

static void
gen6_render_color_calc_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen6_color_calc_state *color_calc_state;
    
    dri_bo_map(render_state->cc.state, 1);
    assert(render_state->cc.state->virtual);
    color_calc_state = render_state->cc.state->virtual;
    memset(color_calc_state, 0, sizeof(*color_calc_state));
    color_calc_state->constant_r = 1.0;
    color_calc_state->constant_g = 0.0;
    color_calc_state->constant_b = 1.0;
    color_calc_state->constant_a = 1.0;
    dri_bo_unmap(render_state->cc.state);
}

static void
gen6_render_blend_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen6_blend_state *blend_state;
    
    dri_bo_map(render_state->cc.blend, 1);
    assert(render_state->cc.blend->virtual);
    blend_state = render_state->cc.blend->virtual;
    memset(blend_state, 0, sizeof(*blend_state));
    blend_state->blend1.logic_op_enable = 1;
    blend_state->blend1.logic_op_func = 0xc;
    dri_bo_unmap(render_state->cc.blend);
}

static void
gen6_render_depth_stencil_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen6_depth_stencil_state *depth_stencil_state;
    
    dri_bo_map(render_state->cc.depth_stencil, 1);
    assert(render_state->cc.depth_stencil->virtual);
    depth_stencil_state = render_state->cc.depth_stencil->virtual;
    memset(depth_stencil_state, 0, sizeof(*depth_stencil_state));
    dri_bo_unmap(render_state->cc.depth_stencil);
}

static void
gen6_render_setup_states(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags
)
{
    i965_render_dest_surface_state(ctx, 0);
    i965_render_src_surfaces_state(ctx, obj_surface, flags);
    i965_render_sampler(ctx);
    i965_render_cc_viewport(ctx);
    gen6_render_color_calc_state(ctx);
    gen6_render_blend_state(ctx);
    gen6_render_depth_stencil_state(ctx);
    i965_render_upload_constants(ctx, obj_surface, flags);
    i965_render_upload_vertex(ctx, obj_surface, src_rect, dst_rect);
}

static void
gen6_emit_invarient_states(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_3D);

    OUT_BATCH(batch, GEN6_3DSTATE_MULTISAMPLE | (3 - 2));
    OUT_BATCH(batch, GEN6_3DSTATE_MULTISAMPLE_PIXEL_LOCATION_CENTER |
              GEN6_3DSTATE_MULTISAMPLE_NUMSAMPLES_1); /* 1 sample/pixel */
    OUT_BATCH(batch, 0);

    OUT_BATCH(batch, GEN6_3DSTATE_SAMPLE_MASK | (2 - 2));
    OUT_BATCH(batch, 1);

    /* Set system instruction pointer */
    OUT_BATCH(batch, CMD_STATE_SIP | 0);
    OUT_BATCH(batch, 0);
}

static void
gen6_emit_state_base_address(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | (10 - 2));
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* General state base address */
    OUT_RELOC(batch, render_state->wm.surface_state_binding_table_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY); /* Surface state base address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Dynamic state base address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Indirect object base address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Instruction base address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* General state upper bound */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Dynamic state upper bound */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Indirect object upper bound */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Instruction access upper bound */
}

static void
gen6_emit_viewport_state_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    OUT_BATCH(batch, GEN6_3DSTATE_VIEWPORT_STATE_POINTERS |
              GEN6_3DSTATE_VIEWPORT_STATE_MODIFY_CC |
              (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_RELOC(batch, render_state->cc.viewport, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
}

static void
gen6_emit_urb(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    OUT_BATCH(batch, GEN6_3DSTATE_URB | (3 - 2));
    OUT_BATCH(batch, ((1 - 1) << GEN6_3DSTATE_URB_VS_SIZE_SHIFT) |
              (24 << GEN6_3DSTATE_URB_VS_ENTRIES_SHIFT)); /* at least 24 on GEN6 */
    OUT_BATCH(batch, (0 << GEN6_3DSTATE_URB_GS_SIZE_SHIFT) |
              (0 << GEN6_3DSTATE_URB_GS_ENTRIES_SHIFT)); /* no GS thread */
}

static void
gen6_emit_cc_state_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    OUT_BATCH(batch, GEN6_3DSTATE_CC_STATE_POINTERS | (4 - 2));
    OUT_RELOC(batch, render_state->cc.blend, I915_GEM_DOMAIN_INSTRUCTION, 0, 1);
    OUT_RELOC(batch, render_state->cc.depth_stencil, I915_GEM_DOMAIN_INSTRUCTION, 0, 1);
    OUT_RELOC(batch, render_state->cc.state, I915_GEM_DOMAIN_INSTRUCTION, 0, 1);
}

static void
gen6_emit_sampler_state_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    OUT_BATCH(batch, GEN6_3DSTATE_SAMPLER_STATE_POINTERS |
              GEN6_3DSTATE_SAMPLER_STATE_MODIFY_PS |
              (4 - 2));
    OUT_BATCH(batch, 0); /* VS */
    OUT_BATCH(batch, 0); /* GS */
    OUT_RELOC(batch,render_state->wm.sampler, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
}

static void
gen6_emit_binding_table(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    /* Binding table pointers */
    OUT_BATCH(batch, CMD_BINDING_TABLE_POINTERS |
              GEN6_BINDING_TABLE_MODIFY_PS |
              (4 - 2));
    OUT_BATCH(batch, 0);		/* vs */
    OUT_BATCH(batch, 0);		/* gs */
    /* Only the PS uses the binding table */
    OUT_BATCH(batch, BINDING_TABLE_OFFSET);
}

static void
gen6_emit_depth_buffer_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    OUT_BATCH(batch, CMD_DEPTH_BUFFER | (7 - 2));
    OUT_BATCH(batch, (I965_SURFACE_NULL << CMD_DEPTH_BUFFER_TYPE_SHIFT) |
              (I965_DEPTHFORMAT_D32_FLOAT << CMD_DEPTH_BUFFER_FORMAT_SHIFT));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);

    OUT_BATCH(batch, CMD_CLEAR_PARAMS | (2 - 2));
    OUT_BATCH(batch, 0);
}

static void
gen6_emit_drawing_rectangle(VADriverContextP ctx)
{
    i965_render_drawing_rectangle(ctx);
}

static void 
gen6_emit_vs_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    /* disable VS constant buffer */
    OUT_BATCH(batch, GEN6_3DSTATE_CONSTANT_VS | (5 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
	
    OUT_BATCH(batch, GEN6_3DSTATE_VS | (6 - 2));
    OUT_BATCH(batch, 0); /* without VS kernel */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* pass-through */
}

static void 
gen6_emit_gs_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    /* disable GS constant buffer */
    OUT_BATCH(batch, GEN6_3DSTATE_CONSTANT_GS | (5 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
	
    OUT_BATCH(batch, GEN6_3DSTATE_GS | (7 - 2));
    OUT_BATCH(batch, 0); /* without GS kernel */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* pass-through */
}

static void 
gen6_emit_clip_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    OUT_BATCH(batch, GEN6_3DSTATE_CLIP | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* pass-through */
    OUT_BATCH(batch, 0);
}

static void 
gen6_emit_sf_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    OUT_BATCH(batch, GEN6_3DSTATE_SF | (20 - 2));
    OUT_BATCH(batch, (1 << GEN6_3DSTATE_SF_NUM_OUTPUTS_SHIFT) |
              (1 << GEN6_3DSTATE_SF_URB_ENTRY_READ_LENGTH_SHIFT) |
              (0 << GEN6_3DSTATE_SF_URB_ENTRY_READ_OFFSET_SHIFT));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, GEN6_3DSTATE_SF_CULL_NONE);
    OUT_BATCH(batch, 2 << GEN6_3DSTATE_SF_TRIFAN_PROVOKE_SHIFT); /* DW4 */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* DW9 */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* DW14 */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* DW19 */
}

static void 
gen6_emit_wm_state(VADriverContextP ctx, int kernel)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    OUT_BATCH(batch, GEN6_3DSTATE_CONSTANT_PS |
              GEN6_3DSTATE_CONSTANT_BUFFER_0_ENABLE |
              (5 - 2));
    OUT_RELOC(batch, 
              render_state->curbe.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              (URB_CS_ENTRY_SIZE-1));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);

    OUT_BATCH(batch, GEN6_3DSTATE_WM | (9 - 2));
    OUT_RELOC(batch, render_state->render_kernels[kernel].bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              0);
    OUT_BATCH(batch, (1 << GEN6_3DSTATE_WM_SAMPLER_COUNT_SHITF) |
              (5 << GEN6_3DSTATE_WM_BINDING_TABLE_ENTRY_COUNT_SHIFT));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, (6 << GEN6_3DSTATE_WM_DISPATCH_START_GRF_0_SHIFT)); /* DW4 */
    OUT_BATCH(batch, ((i965->intel.device_info->max_wm_threads - 1) << GEN6_3DSTATE_WM_MAX_THREADS_SHIFT) |
              GEN6_3DSTATE_WM_DISPATCH_ENABLE |
              GEN6_3DSTATE_WM_16_DISPATCH_ENABLE);
    OUT_BATCH(batch, (1 << GEN6_3DSTATE_WM_NUM_SF_OUTPUTS_SHIFT) |
              GEN6_3DSTATE_WM_PERSPECTIVE_PIXEL_BARYCENTRIC);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
}

static void
gen6_emit_vertex_element_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    /* Set up our vertex elements, sourced from the single vertex buffer. */
    OUT_BATCH(batch, CMD_VERTEX_ELEMENTS | (5 - 2));
    /* offset 0: X,Y -> {X, Y, 1.0, 1.0} */
    OUT_BATCH(batch, (0 << GEN6_VE0_VERTEX_BUFFER_INDEX_SHIFT) |
              GEN6_VE0_VALID |
              (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
              (0 << VE0_OFFSET_SHIFT));
    OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT) |
              (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT));
    /* offset 8: S0, T0 -> {S0, T0, 1.0, 1.0} */
    OUT_BATCH(batch, (0 << GEN6_VE0_VERTEX_BUFFER_INDEX_SHIFT) |
              GEN6_VE0_VALID |
              (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
              (8 << VE0_OFFSET_SHIFT));
    OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT) | 
              (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT));
}

static void
gen6_emit_vertices(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 11);
    OUT_BATCH(batch, CMD_VERTEX_BUFFERS | 3);
    OUT_BATCH(batch, 
              (0 << GEN6_VB0_BUFFER_INDEX_SHIFT) |
              GEN6_VB0_VERTEXDATA |
              ((4 * 4) << VB0_BUFFER_PITCH_SHIFT));
    OUT_RELOC(batch, render_state->vb.vertex_buffer, I915_GEM_DOMAIN_VERTEX, 0, 0);
    OUT_RELOC(batch, render_state->vb.vertex_buffer, I915_GEM_DOMAIN_VERTEX, 0, 12 * 4);
    OUT_BATCH(batch, 0);

    OUT_BATCH(batch, 
              CMD_3DPRIMITIVE |
              _3DPRIMITIVE_VERTEX_SEQUENTIAL |
              (_3DPRIM_RECTLIST << _3DPRIMITIVE_TOPOLOGY_SHIFT) |
              (0 << 9) |
              4);
    OUT_BATCH(batch, 3); /* vertex count per instance */
    OUT_BATCH(batch, 0); /* start vertex offset */
    OUT_BATCH(batch, 1); /* single instance */
    OUT_BATCH(batch, 0); /* start instance location */
    OUT_BATCH(batch, 0); /* index buffer offset, ignored */
    ADVANCE_BATCH(batch);
}

static void
gen6_render_emit_states(VADriverContextP ctx, int kernel)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    intel_batchbuffer_start_atomic(batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(batch);
    gen6_emit_invarient_states(ctx);
    gen6_emit_state_base_address(ctx);
    gen6_emit_viewport_state_pointers(ctx);
    gen6_emit_urb(ctx);
    gen6_emit_cc_state_pointers(ctx);
    gen6_emit_sampler_state_pointers(ctx);
    gen6_emit_vs_state(ctx);
    gen6_emit_gs_state(ctx);
    gen6_emit_clip_state(ctx);
    gen6_emit_sf_state(ctx);
    gen6_emit_wm_state(ctx, kernel);
    gen6_emit_binding_table(ctx);
    gen6_emit_depth_buffer_state(ctx);
    gen6_emit_drawing_rectangle(ctx);
    gen6_emit_vertex_element_state(ctx);
    gen6_emit_vertices(ctx);
    intel_batchbuffer_end_atomic(batch);
}

static void
gen6_render_put_surface(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    gen6_render_initialize(ctx);
    gen6_render_setup_states(ctx, obj_surface, src_rect, dst_rect, flags);
    i965_clear_dest_region(ctx);
    gen6_render_emit_states(ctx, PS_KERNEL);
    intel_batchbuffer_flush(batch);
}

static void
gen6_subpicture_render_blend_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen6_blend_state *blend_state;

    dri_bo_unmap(render_state->cc.state);    
    dri_bo_map(render_state->cc.blend, 1);
    assert(render_state->cc.blend->virtual);
    blend_state = render_state->cc.blend->virtual;
    memset(blend_state, 0, sizeof(*blend_state));
    blend_state->blend0.dest_blend_factor = I965_BLENDFACTOR_INV_SRC_ALPHA;
    blend_state->blend0.source_blend_factor = I965_BLENDFACTOR_SRC_ALPHA;
    blend_state->blend0.blend_func = I965_BLENDFUNCTION_ADD;
    blend_state->blend0.blend_enable = 1;
    blend_state->blend1.post_blend_clamp_enable = 1;
    blend_state->blend1.pre_blend_clamp_enable = 1;
    blend_state->blend1.clamp_range = 0; /* clamp range [0, 1] */
    dri_bo_unmap(render_state->cc.blend);
}

static void
gen6_subpicture_render_setup_states(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    i965_render_dest_surface_state(ctx, 0);
    i965_subpic_render_src_surfaces_state(ctx, obj_surface);
    i965_render_sampler(ctx);
    i965_render_cc_viewport(ctx);
    gen6_render_color_calc_state(ctx);
    gen6_subpicture_render_blend_state(ctx);
    gen6_render_depth_stencil_state(ctx);
    i965_subpic_render_upload_constants(ctx, obj_surface);
    i965_subpic_render_upload_vertex(ctx, obj_surface, dst_rect);
}

static void
gen6_render_put_subpicture(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    unsigned int index = obj_surface->subpic_render_idx;
    struct object_subpic *obj_subpic = obj_surface->obj_subpic[index];

    assert(obj_subpic);
    gen6_render_initialize(ctx);
    gen6_subpicture_render_setup_states(ctx, obj_surface, src_rect, dst_rect);
    gen6_render_emit_states(ctx, PS_SUBPIC_KERNEL);
    i965_render_upload_image_palette(ctx, obj_subpic->obj_image, 0xff);
    intel_batchbuffer_flush(batch);
}

/*
 * for GEN7
 */
static void 
gen7_render_initialize(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    dri_bo *bo;

    /* VERTEX BUFFER */
    dri_bo_unreference(render_state->vb.vertex_buffer);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "vertex buffer",
                      4096,
                      4096);
    assert(bo);
    render_state->vb.vertex_buffer = bo;

    /* WM */
    dri_bo_unreference(render_state->wm.surface_state_binding_table_bo);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "surface state & binding table",
                      (SURFACE_STATE_PADDED_SIZE + sizeof(unsigned int)) * MAX_RENDER_SURFACES,
                      4096);
    assert(bo);
    render_state->wm.surface_state_binding_table_bo = bo;

    dri_bo_unreference(render_state->wm.sampler);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "sampler state",
                      MAX_SAMPLERS * sizeof(struct gen7_sampler_state),
                      4096);
    assert(bo);
    render_state->wm.sampler = bo;
    render_state->wm.sampler_count = 0;

    /* COLOR CALCULATOR */
    dri_bo_unreference(render_state->cc.state);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "color calc state",
                      sizeof(struct gen6_color_calc_state),
                      4096);
    assert(bo);
    render_state->cc.state = bo;

    /* CC VIEWPORT */
    dri_bo_unreference(render_state->cc.viewport);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "cc viewport",
                      sizeof(struct i965_cc_viewport),
                      4096);
    assert(bo);
    render_state->cc.viewport = bo;

    /* BLEND STATE */
    dri_bo_unreference(render_state->cc.blend);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "blend state",
                      sizeof(struct gen6_blend_state),
                      4096);
    assert(bo);
    render_state->cc.blend = bo;

    /* DEPTH & STENCIL STATE */
    dri_bo_unreference(render_state->cc.depth_stencil);
    bo = dri_bo_alloc(i965->intel.bufmgr,
                      "depth & stencil state",
                      sizeof(struct gen6_depth_stencil_state),
                      4096);
    assert(bo);
    render_state->cc.depth_stencil = bo;
}

/*
 * for GEN8
 */
#define ALIGNMENT       64

static void
gen7_render_color_calc_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen6_color_calc_state *color_calc_state;
    
    dri_bo_map(render_state->cc.state, 1);
    assert(render_state->cc.state->virtual);
    color_calc_state = render_state->cc.state->virtual;
    memset(color_calc_state, 0, sizeof(*color_calc_state));
    color_calc_state->constant_r = 1.0;
    color_calc_state->constant_g = 0.0;
    color_calc_state->constant_b = 1.0;
    color_calc_state->constant_a = 1.0;
    dri_bo_unmap(render_state->cc.state);
}

static void
gen7_render_blend_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen6_blend_state *blend_state;
    
    dri_bo_map(render_state->cc.blend, 1);
    assert(render_state->cc.blend->virtual);
    blend_state = render_state->cc.blend->virtual;
    memset(blend_state, 0, sizeof(*blend_state));
    blend_state->blend1.logic_op_enable = 1;
    blend_state->blend1.logic_op_func = 0xc;
    blend_state->blend1.pre_blend_clamp_enable = 1;
    dri_bo_unmap(render_state->cc.blend);
}

static void
gen7_render_depth_stencil_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen6_depth_stencil_state *depth_stencil_state;
    
    dri_bo_map(render_state->cc.depth_stencil, 1);
    assert(render_state->cc.depth_stencil->virtual);
    depth_stencil_state = render_state->cc.depth_stencil->virtual;
    memset(depth_stencil_state, 0, sizeof(*depth_stencil_state));
    dri_bo_unmap(render_state->cc.depth_stencil);
}

static void 
gen7_render_sampler(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen7_sampler_state *sampler_state;
    int i;
    
    assert(render_state->wm.sampler_count > 0);
    assert(render_state->wm.sampler_count <= MAX_SAMPLERS);

    dri_bo_map(render_state->wm.sampler, 1);
    assert(render_state->wm.sampler->virtual);
    sampler_state = render_state->wm.sampler->virtual;
    for (i = 0; i < render_state->wm.sampler_count; i++) {
        memset(sampler_state, 0, sizeof(*sampler_state));
        sampler_state->ss0.min_filter = I965_MAPFILTER_LINEAR;
        sampler_state->ss0.mag_filter = I965_MAPFILTER_LINEAR;
        sampler_state->ss3.r_wrap_mode = I965_TEXCOORDMODE_CLAMP;
        sampler_state->ss3.s_wrap_mode = I965_TEXCOORDMODE_CLAMP;
        sampler_state->ss3.t_wrap_mode = I965_TEXCOORDMODE_CLAMP;
        sampler_state++;
    }

    dri_bo_unmap(render_state->wm.sampler);
}


static void
gen7_render_setup_states(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags
)
{
    i965_render_dest_surface_state(ctx, 0);
    i965_render_src_surfaces_state(ctx, obj_surface, flags);
    gen7_render_sampler(ctx);
    i965_render_cc_viewport(ctx);
    gen7_render_color_calc_state(ctx);
    gen7_render_blend_state(ctx);
    gen7_render_depth_stencil_state(ctx);
    i965_render_upload_constants(ctx, obj_surface, flags);
    i965_render_upload_vertex(ctx, obj_surface, src_rect, dst_rect);
}


static void
gen7_emit_invarient_states(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 1);
    OUT_BATCH(batch, CMD_PIPELINE_SELECT | PIPELINE_SELECT_3D);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 4);
    OUT_BATCH(batch, GEN6_3DSTATE_MULTISAMPLE | (4 - 2));
    OUT_BATCH(batch, GEN6_3DSTATE_MULTISAMPLE_PIXEL_LOCATION_CENTER |
              GEN6_3DSTATE_MULTISAMPLE_NUMSAMPLES_1); /* 1 sample/pixel */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN6_3DSTATE_SAMPLE_MASK | (2 - 2));
    OUT_BATCH(batch, 1);
    ADVANCE_BATCH(batch);

    /* Set system instruction pointer */
    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, CMD_STATE_SIP | 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen7_emit_state_base_address(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    OUT_BATCH(batch, CMD_STATE_BASE_ADDRESS | (10 - 2));
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* General state base address */
    OUT_RELOC(batch, render_state->wm.surface_state_binding_table_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, BASE_ADDRESS_MODIFY); /* Surface state base address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Dynamic state base address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Indirect object base address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Instruction base address */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* General state upper bound */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Dynamic state upper bound */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Indirect object upper bound */
    OUT_BATCH(batch, BASE_ADDRESS_MODIFY); /* Instruction access upper bound */
}

static void
gen7_emit_viewport_state_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_VIEWPORT_STATE_POINTERS_CC | (2 - 2));
    OUT_RELOC(batch,
              render_state->cc.viewport,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CL | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

/*
 * URB layout on GEN7 
 * ----------------------------------------
 * | PS Push Constants (8KB) | VS entries |
 * ----------------------------------------
 */
static void
gen7_emit_urb(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    unsigned int num_urb_entries = 32;

    if (IS_HASWELL(i965->intel.device_info))
        num_urb_entries = 64;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_PUSH_CONSTANT_ALLOC_PS | (2 - 2));
    OUT_BATCH(batch, 8); /* in 1KBs */
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_URB_VS | (2 - 2));
    OUT_BATCH(batch, 
              (num_urb_entries << GEN7_URB_ENTRY_NUMBER_SHIFT) |
              (2 - 1) << GEN7_URB_ENTRY_SIZE_SHIFT |
              (1 << GEN7_URB_STARTING_ADDRESS_SHIFT));
   ADVANCE_BATCH(batch);

   BEGIN_BATCH(batch, 2);
   OUT_BATCH(batch, GEN7_3DSTATE_URB_GS | (2 - 2));
   OUT_BATCH(batch,
             (0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
             (1 << GEN7_URB_STARTING_ADDRESS_SHIFT));
   ADVANCE_BATCH(batch);

   BEGIN_BATCH(batch, 2);
   OUT_BATCH(batch, GEN7_3DSTATE_URB_HS | (2 - 2));
   OUT_BATCH(batch,
             (0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
             (2 << GEN7_URB_STARTING_ADDRESS_SHIFT));
   ADVANCE_BATCH(batch);

   BEGIN_BATCH(batch, 2);
   OUT_BATCH(batch, GEN7_3DSTATE_URB_DS | (2 - 2));
   OUT_BATCH(batch,
             (0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
             (2 << GEN7_URB_STARTING_ADDRESS_SHIFT));
   ADVANCE_BATCH(batch);
}

static void
gen7_emit_cc_state_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN6_3DSTATE_CC_STATE_POINTERS | (2 - 2));
    OUT_RELOC(batch,
              render_state->cc.state,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              1);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BLEND_STATE_POINTERS | (2 - 2));
    OUT_RELOC(batch,
              render_state->cc.blend,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              1);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_DEPTH_STENCIL_STATE_POINTERS | (2 - 2));
    OUT_RELOC(batch,
              render_state->cc.depth_stencil,
              I915_GEM_DOMAIN_INSTRUCTION, 0, 
              1);
    ADVANCE_BATCH(batch);
}

static void
gen7_emit_sampler_state_pointers(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_SAMPLER_STATE_POINTERS_PS | (2 - 2));
    OUT_RELOC(batch,
              render_state->wm.sampler,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              0);
    ADVANCE_BATCH(batch);
}

static void
gen7_emit_binding_table(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BINDING_TABLE_POINTERS_PS | (2 - 2));
    OUT_BATCH(batch, BINDING_TABLE_OFFSET);
    ADVANCE_BATCH(batch);
}

static void
gen7_emit_depth_buffer_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 7);
    OUT_BATCH(batch, GEN7_3DSTATE_DEPTH_BUFFER | (7 - 2));
    OUT_BATCH(batch,
              (I965_DEPTHFORMAT_D32_FLOAT << 18) |
              (I965_SURFACE_NULL << 29));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 3);
    OUT_BATCH(batch, GEN7_3DSTATE_CLEAR_PARAMS | (3 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen7_emit_drawing_rectangle(VADriverContextP ctx)
{
    i965_render_drawing_rectangle(ctx);
}

static void 
gen7_emit_vs_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    /* disable VS constant buffer */
    OUT_BATCH(batch, GEN6_3DSTATE_CONSTANT_VS | (7 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
	
    OUT_BATCH(batch, GEN6_3DSTATE_VS | (6 - 2));
    OUT_BATCH(batch, 0); /* without VS kernel */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* pass-through */
}

static void 
gen7_emit_bypass_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    /* bypass GS */
    BEGIN_BATCH(batch, 7);
    OUT_BATCH(batch, GEN6_3DSTATE_CONSTANT_GS | (7 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 7);	
    OUT_BATCH(batch, GEN6_3DSTATE_GS | (7 - 2));
    OUT_BATCH(batch, 0); /* without GS kernel */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* pass-through */
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BINDING_TABLE_POINTERS_GS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* disable HS */
    BEGIN_BATCH(batch, 7);
    OUT_BATCH(batch, GEN7_3DSTATE_CONSTANT_HS | (7 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 7);
    OUT_BATCH(batch, GEN7_3DSTATE_HS | (7 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BINDING_TABLE_POINTERS_HS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* Disable TE */
    BEGIN_BATCH(batch, 4);
    OUT_BATCH(batch, GEN7_3DSTATE_TE | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* Disable DS */
    BEGIN_BATCH(batch, 7);
    OUT_BATCH(batch, GEN7_3DSTATE_CONSTANT_DS | (7 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 6);
    OUT_BATCH(batch, GEN7_3DSTATE_DS | (6 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 2);
    OUT_BATCH(batch, GEN7_3DSTATE_BINDING_TABLE_POINTERS_DS | (2 - 2));
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    /* Disable STREAMOUT */
    BEGIN_BATCH(batch, 3);
    OUT_BATCH(batch, GEN7_3DSTATE_STREAMOUT | (3 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void 
gen7_emit_clip_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    OUT_BATCH(batch, GEN6_3DSTATE_CLIP | (4 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* pass-through */
    OUT_BATCH(batch, 0);
}

static void 
gen7_emit_sf_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    BEGIN_BATCH(batch, 14);
    OUT_BATCH(batch, GEN7_3DSTATE_SBE | (14 - 2));
    OUT_BATCH(batch,
              (1 << GEN7_SBE_NUM_OUTPUTS_SHIFT) |
              (1 << GEN7_SBE_URB_ENTRY_READ_LENGTH_SHIFT) |
              (0 << GEN7_SBE_URB_ENTRY_READ_OFFSET_SHIFT));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* DW4 */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0); /* DW9 */
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 7);
    OUT_BATCH(batch, GEN6_3DSTATE_SF | (7 - 2));
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, GEN6_3DSTATE_SF_CULL_NONE);
    OUT_BATCH(batch, 2 << GEN6_3DSTATE_SF_TRIFAN_PROVOKE_SHIFT);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void 
gen7_emit_wm_state(VADriverContextP ctx, int kernel)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;
    unsigned int max_threads_shift = GEN7_PS_MAX_THREADS_SHIFT_IVB;
    unsigned int num_samples = 0;

    if (IS_HASWELL(i965->intel.device_info)) {
        max_threads_shift = GEN7_PS_MAX_THREADS_SHIFT_HSW;
        num_samples = 1 << GEN7_PS_SAMPLE_MASK_SHIFT_HSW;
    }

    BEGIN_BATCH(batch, 3);
    OUT_BATCH(batch, GEN6_3DSTATE_WM | (3 - 2));
    OUT_BATCH(batch,
              GEN7_WM_DISPATCH_ENABLE |
              GEN7_WM_PERSPECTIVE_PIXEL_BARYCENTRIC);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 7);
    OUT_BATCH(batch, GEN6_3DSTATE_CONSTANT_PS | (7 - 2));
    OUT_BATCH(batch, URB_CS_ENTRY_SIZE);
    OUT_BATCH(batch, 0);
    OUT_RELOC(batch, 
              render_state->curbe.bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 8);
    OUT_BATCH(batch, GEN7_3DSTATE_PS | (8 - 2));
    OUT_RELOC(batch, 
              render_state->render_kernels[kernel].bo,
              I915_GEM_DOMAIN_INSTRUCTION, 0,
              0);
    OUT_BATCH(batch, 
              (1 << GEN7_PS_SAMPLER_COUNT_SHIFT) |
              (5 << GEN7_PS_BINDING_TABLE_ENTRY_COUNT_SHIFT));
    OUT_BATCH(batch, 0); /* scratch space base offset */
    OUT_BATCH(batch, 
              ((i965->intel.device_info->max_wm_threads - 1) << max_threads_shift) | num_samples |
              GEN7_PS_PUSH_CONSTANT_ENABLE |
              GEN7_PS_ATTRIBUTE_ENABLE |
              GEN7_PS_16_DISPATCH_ENABLE);
    OUT_BATCH(batch, 
              (6 << GEN7_PS_DISPATCH_START_GRF_SHIFT_0));
    OUT_BATCH(batch, 0); /* kernel 1 pointer */
    OUT_BATCH(batch, 0); /* kernel 2 pointer */
    ADVANCE_BATCH(batch);
}

static void
gen7_emit_vertex_element_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    /* Set up our vertex elements, sourced from the single vertex buffer. */
    OUT_BATCH(batch, CMD_VERTEX_ELEMENTS | (5 - 2));
    /* offset 0: X,Y -> {X, Y, 1.0, 1.0} */
    OUT_BATCH(batch, (0 << GEN6_VE0_VERTEX_BUFFER_INDEX_SHIFT) |
              GEN6_VE0_VALID |
              (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
              (0 << VE0_OFFSET_SHIFT));
    OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT) |
              (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT));
    /* offset 8: S0, T0 -> {S0, T0, 1.0, 1.0} */
    OUT_BATCH(batch, (0 << GEN6_VE0_VERTEX_BUFFER_INDEX_SHIFT) |
              GEN6_VE0_VALID |
              (I965_SURFACEFORMAT_R32G32_FLOAT << VE0_FORMAT_SHIFT) |
              (8 << VE0_OFFSET_SHIFT));
    OUT_BATCH(batch, (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_0_SHIFT) | 
              (I965_VFCOMPONENT_STORE_SRC << VE1_VFCOMPONENT_1_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_2_SHIFT) |
              (I965_VFCOMPONENT_STORE_1_FLT << VE1_VFCOMPONENT_3_SHIFT));
}

static void
gen7_emit_vertices(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    struct i965_render_state *render_state = &i965->render_state;

    BEGIN_BATCH(batch, 5);
    OUT_BATCH(batch, CMD_VERTEX_BUFFERS | (5 - 2));
    OUT_BATCH(batch, 
              (0 << GEN6_VB0_BUFFER_INDEX_SHIFT) |
              GEN6_VB0_VERTEXDATA |
              GEN7_VB0_ADDRESS_MODIFYENABLE |
              ((4 * 4) << VB0_BUFFER_PITCH_SHIFT));
    OUT_RELOC(batch, render_state->vb.vertex_buffer, I915_GEM_DOMAIN_VERTEX, 0, 0);
    OUT_RELOC(batch, render_state->vb.vertex_buffer, I915_GEM_DOMAIN_VERTEX, 0, 12 * 4);
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);

    BEGIN_BATCH(batch, 7);
    OUT_BATCH(batch, CMD_3DPRIMITIVE | (7 - 2));
    OUT_BATCH(batch,
              _3DPRIM_RECTLIST |
              GEN7_3DPRIM_VERTEXBUFFER_ACCESS_SEQUENTIAL);
    OUT_BATCH(batch, 3); /* vertex count per instance */
    OUT_BATCH(batch, 0); /* start vertex offset */
    OUT_BATCH(batch, 1); /* single instance */
    OUT_BATCH(batch, 0); /* start instance location */
    OUT_BATCH(batch, 0);
    ADVANCE_BATCH(batch);
}

static void
gen7_render_emit_states(VADriverContextP ctx, int kernel)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    intel_batchbuffer_start_atomic(batch, 0x1000);
    intel_batchbuffer_emit_mi_flush(batch);
    gen7_emit_invarient_states(ctx);
    gen7_emit_state_base_address(ctx);
    gen7_emit_viewport_state_pointers(ctx);
    gen7_emit_urb(ctx);
    gen7_emit_cc_state_pointers(ctx);
    gen7_emit_sampler_state_pointers(ctx);
    gen7_emit_bypass_state(ctx);
    gen7_emit_vs_state(ctx);
    gen7_emit_clip_state(ctx);
    gen7_emit_sf_state(ctx);
    gen7_emit_wm_state(ctx, kernel);
    gen7_emit_binding_table(ctx);
    gen7_emit_depth_buffer_state(ctx);
    gen7_emit_drawing_rectangle(ctx);
    gen7_emit_vertex_element_state(ctx);
    gen7_emit_vertices(ctx);
    intel_batchbuffer_end_atomic(batch);
}


static void
gen7_render_put_surface(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,    
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;

    gen7_render_initialize(ctx);
    gen7_render_setup_states(ctx, obj_surface, src_rect, dst_rect, flags);
    i965_clear_dest_region(ctx);
    gen7_render_emit_states(ctx, PS_KERNEL);
    intel_batchbuffer_flush(batch);
}


static void
gen7_subpicture_render_blend_state(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    struct gen6_blend_state *blend_state;

    dri_bo_unmap(render_state->cc.state);    
    dri_bo_map(render_state->cc.blend, 1);
    assert(render_state->cc.blend->virtual);
    blend_state = render_state->cc.blend->virtual;
    memset(blend_state, 0, sizeof(*blend_state));
    blend_state->blend0.dest_blend_factor = I965_BLENDFACTOR_INV_SRC_ALPHA;
    blend_state->blend0.source_blend_factor = I965_BLENDFACTOR_SRC_ALPHA;
    blend_state->blend0.blend_func = I965_BLENDFUNCTION_ADD;
    blend_state->blend0.blend_enable = 1;
    blend_state->blend1.post_blend_clamp_enable = 1;
    blend_state->blend1.pre_blend_clamp_enable = 1;
    blend_state->blend1.clamp_range = 0; /* clamp range [0, 1] */
    dri_bo_unmap(render_state->cc.blend);
}

static void
gen7_subpicture_render_setup_states(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    i965_render_dest_surface_state(ctx, 0);
    i965_subpic_render_src_surfaces_state(ctx, obj_surface);
    i965_render_sampler(ctx);
    i965_render_cc_viewport(ctx);
    gen7_render_color_calc_state(ctx);
    gen7_subpicture_render_blend_state(ctx);
    gen7_render_depth_stencil_state(ctx);
    i965_subpic_render_upload_constants(ctx, obj_surface);
    i965_subpic_render_upload_vertex(ctx, obj_surface, dst_rect);
}

static void
gen7_render_put_subpicture(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct intel_batchbuffer *batch = i965->batch;
    unsigned int index = obj_surface->subpic_render_idx;
    struct object_subpic *obj_subpic = obj_surface->obj_subpic[index];

    assert(obj_subpic);
    gen7_render_initialize(ctx);
    gen7_subpicture_render_setup_states(ctx, obj_surface, src_rect, dst_rect);
    gen7_render_emit_states(ctx, PS_SUBPIC_KERNEL);
    i965_render_upload_image_palette(ctx, obj_subpic->obj_image, 0xff);
    intel_batchbuffer_flush(batch);
}


void
intel_render_put_surface(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect,
    unsigned int       flags
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    int has_done_scaling = 0;
    VASurfaceID out_surface_id = i965_post_processing(ctx,
                                                      obj_surface,
                                                      src_rect,
                                                      dst_rect,
                                                      flags,
                                                      &has_done_scaling);

    assert((!has_done_scaling) || (out_surface_id != VA_INVALID_ID));

    if (out_surface_id != VA_INVALID_ID) {
        struct object_surface *new_obj_surface = SURFACE(out_surface_id);
        
        if (new_obj_surface && new_obj_surface->bo)
            obj_surface = new_obj_surface;

        if (has_done_scaling)
            src_rect = dst_rect;
    }

    render_state->render_put_surface(ctx, obj_surface, src_rect, dst_rect, flags);

    if (out_surface_id != VA_INVALID_ID)
        i965_DestroySurfaces(ctx, &out_surface_id, 1);
}

void
intel_render_put_subpicture(
    VADriverContextP   ctx,
    struct object_surface *obj_surface,
    const VARectangle *src_rect,
    const VARectangle *dst_rect
)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;

    render_state->render_put_subpicture(ctx, obj_surface, src_rect, dst_rect);
}

static void
genx_render_terminate(VADriverContextP ctx)
{
    int i;
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;

    dri_bo_unreference(render_state->curbe.bo);
    render_state->curbe.bo = NULL;

    for (i = 0; i < NUM_RENDER_KERNEL; i++) {
        struct i965_kernel *kernel = &render_state->render_kernels[i];

        dri_bo_unreference(kernel->bo);
        kernel->bo = NULL;
    }

    dri_bo_unreference(render_state->vb.vertex_buffer);
    render_state->vb.vertex_buffer = NULL;
    dri_bo_unreference(render_state->vs.state);
    render_state->vs.state = NULL;
    dri_bo_unreference(render_state->sf.state);
    render_state->sf.state = NULL;
    dri_bo_unreference(render_state->wm.sampler);
    render_state->wm.sampler = NULL;
    dri_bo_unreference(render_state->wm.state);
    render_state->wm.state = NULL;
    dri_bo_unreference(render_state->wm.surface_state_binding_table_bo);
    dri_bo_unreference(render_state->cc.viewport);
    render_state->cc.viewport = NULL;
    dri_bo_unreference(render_state->cc.state);
    render_state->cc.state = NULL;
    dri_bo_unreference(render_state->cc.blend);
    render_state->cc.blend = NULL;
    dri_bo_unreference(render_state->cc.depth_stencil);
    render_state->cc.depth_stencil = NULL;

    if (render_state->draw_region) {
        dri_bo_unreference(render_state->draw_region->bo);
        free(render_state->draw_region);
        render_state->draw_region = NULL;
    }
}

bool 
genx_render_init(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;
    int i;

    /* kernel */
    assert(NUM_RENDER_KERNEL == (sizeof(render_kernels_gen5) / 
                                 sizeof(render_kernels_gen5[0])));
    assert(NUM_RENDER_KERNEL == (sizeof(render_kernels_gen6) / 
                                 sizeof(render_kernels_gen6[0])));

    if (IS_GEN7(i965->intel.device_info)) {
        memcpy(render_state->render_kernels,
               (IS_HASWELL(i965->intel.device_info) ? render_kernels_gen7_haswell : render_kernels_gen7),
               sizeof(render_state->render_kernels));
        render_state->render_put_surface = gen7_render_put_surface;
        render_state->render_put_subpicture = gen7_render_put_subpicture;
    } else if (IS_GEN6(i965->intel.device_info)) {
        memcpy(render_state->render_kernels, render_kernels_gen6, sizeof(render_state->render_kernels));
        render_state->render_put_surface = gen6_render_put_surface;
        render_state->render_put_subpicture = gen6_render_put_subpicture;
    } else if (IS_IRONLAKE(i965->intel.device_info)) {
        memcpy(render_state->render_kernels, render_kernels_gen5, sizeof(render_state->render_kernels));
        render_state->render_put_surface = i965_render_put_surface;
        render_state->render_put_subpicture = i965_render_put_subpicture;
    } else {
        memcpy(render_state->render_kernels, render_kernels_gen4, sizeof(render_state->render_kernels));
        render_state->render_put_surface = i965_render_put_surface;
        render_state->render_put_subpicture = i965_render_put_subpicture;
    }

    render_state->render_terminate = genx_render_terminate;

    for (i = 0; i < NUM_RENDER_KERNEL; i++) {
        struct i965_kernel *kernel = &render_state->render_kernels[i];

        if (!kernel->size)
            continue;

        kernel->bo = dri_bo_alloc(i965->intel.bufmgr, 
                                  kernel->name, 
                                  kernel->size, 0x1000);
        assert(kernel->bo);
        dri_bo_subdata(kernel->bo, 0, kernel->size, kernel->bin);
    }

    /* constant buffer */
    render_state->curbe.bo = dri_bo_alloc(i965->intel.bufmgr,
                      "constant buffer",
                      4096, 64);
    assert(render_state->curbe.bo);

    return true;
}

bool
i965_render_init(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);

    return i965->codec_info->render_init(ctx);
}

void
i965_render_terminate(VADriverContextP ctx)
{
    struct i965_driver_data *i965 = i965_driver_data(ctx);
    struct i965_render_state *render_state = &i965->render_state;

    render_state->render_terminate(ctx);
}
