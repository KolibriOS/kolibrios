/*
 * Copyright © 2011 Intel Corporation
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

#ifndef BRW_PROGRAM_H
#define BRW_PROGRAM_H

/**
 * Program key structures.
 *
 * When drawing, we look for the currently bound shaders in the program
 * cache.  This is essentially a hash table lookup, and these are the keys.
 *
 * Sometimes OpenGL features specified as state need to be simulated via
 * shader code, due to a mismatch between the API and the hardware.  This
 * is often referred to as "non-orthagonal state" or "NOS".  We store NOS
 * in the program key so it's considered when searching for a program.  If
 * we haven't seen a particular combination before, we have to recompile a
 * new specialized version.
 *
 * Shader compilation should not look up state in gl_context directly, but
 * instead use the copy in the program key.  This guarantees recompiles will
 * happen correctly.
 *
 *  @{
 */

enum PACKED gen6_gather_sampler_wa {
   WA_SIGN = 1,      /* whether we need to sign extend */
   WA_8BIT = 2,      /* if we have an 8bit format needing wa */
   WA_16BIT = 4,     /* if we have a 16bit format needing wa */
};

/**
 * Sampler information needed by VS, WM, and GS program cache keys.
 */
struct brw_sampler_prog_key_data {
   /**
    * EXT_texture_swizzle and DEPTH_TEXTURE_MODE swizzles.
    */
   uint16_t swizzles[MAX_SAMPLERS];

   uint32_t gl_clamp_mask[3];

   /**
    * For RG32F, gather4's channel select is broken.
    */
   uint32_t gather_channel_quirk_mask;

   /**
    * Whether this sampler uses the compressed multisample surface layout.
    */
   uint32_t compressed_multisample_layout_mask;

   /**
    * For Sandybridge, which shader w/a we need for gather quirks.
    */
   enum gen6_gather_sampler_wa gen6_gather_wa[MAX_SAMPLERS];
};


struct brw_vue_prog_key {
   unsigned program_string_id;

   /**
    * True if at least one clip flag is enabled, regardless of whether the
    * shader uses clip planes or gl_ClipDistance.
    */
   bool userclip_active:1;

   /**
    * How many user clipping planes are being uploaded to the vertex shader as
    * push constants.
    */
   unsigned nr_userclip_plane_consts:4;

   struct brw_sampler_prog_key_data tex;
};

/** The program key for Vertex Shaders. */
struct brw_vs_prog_key {
   struct brw_vue_prog_key base;

   /*
    * Per-attribute workaround flags
    */
   uint8_t gl_attrib_wa_flags[VERT_ATTRIB_MAX];

   bool copy_edgeflag:1;

   bool clamp_vertex_color:1;

   /**
    * For pre-Gen6 hardware, a bitfield indicating which texture coordinates
    * are going to be replaced with point coordinates (as a consequence of a
    * call to glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE)).  Because
    * our SF thread requires exact matching between VS outputs and FS inputs,
    * these texture coordinates will need to be unconditionally included in
    * the VUE, even if they aren't written by the vertex shader.
    */
   uint8_t point_coord_replace;
};

/** The program key for Geometry Shaders. */
struct brw_gs_prog_key
{
   struct brw_vue_prog_key base;

   uint64_t input_varyings;
};

/** The program key for Fragment/Pixel Shaders. */
struct brw_wm_prog_key {
   uint8_t iz_lookup;
   bool stats_wm:1;
   bool flat_shade:1;
   bool persample_shading:1;
   bool persample_2x:1;
   unsigned nr_color_regions:5;
   bool replicate_alpha:1;
   bool render_to_fbo:1;
   bool clamp_fragment_color:1;
   bool compute_pos_offset:1;
   bool compute_sample_id:1;
   unsigned line_aa:2;
   bool high_quality_derivatives:1;

   uint16_t drawable_height;
   uint64_t input_slots_valid;
   unsigned program_string_id;
   GLenum alpha_test_func;          /* < For Gen4/5 MRT alpha test */
   float alpha_test_ref;

   struct brw_sampler_prog_key_data tex;
};

/** @} */

#ifdef __cplusplus
extern "C" {
#endif

void brw_populate_sampler_prog_key_data(struct gl_context *ctx,
				        const struct gl_program *prog,
                                        unsigned sampler_count,
				        struct brw_sampler_prog_key_data *key);
bool brw_debug_recompile_sampler_key(struct brw_context *brw,
                                     const struct brw_sampler_prog_key_data *old_key,
                                     const struct brw_sampler_prog_key_data *key);
void brw_add_texrect_params(struct gl_program *prog);

void
brw_mark_surface_used(struct brw_stage_prog_data *prog_data,
                      unsigned surf_index);

bool
brw_stage_prog_data_compare(const struct brw_stage_prog_data *a,
                            const struct brw_stage_prog_data *b);

void
brw_stage_prog_data_free(const void *prog_data);

void
brw_dump_ir(const char *stage, struct gl_shader_program *shader_prog,
            struct gl_shader *shader, struct gl_program *prog);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
