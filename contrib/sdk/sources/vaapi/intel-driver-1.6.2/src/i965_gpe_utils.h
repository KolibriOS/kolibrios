/*
 * Copyright © 2012 Intel Corporation
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
 *
 * Authors:
 *    Xiang Haihao <haihao.xiang@intel.com>
 */

#ifndef _I965_GPE_UTILS_H_
#define _I965_GPE_UTILS_H_

#include <i915_drm.h>
#include <intel_bufmgr.h>

#include "i965_defines.h"
#include "i965_drv_video.h"
#include "i965_structs.h"

#define MAX_GPE_KERNELS    32

struct i965_buffer_surface
{
    dri_bo *bo;
    unsigned int num_blocks;
    unsigned int size_block;
    unsigned int pitch;
};

struct i965_gpe_context
{
    struct {
        dri_bo *bo;
        unsigned int length;            /* in bytes */
    } surface_state_binding_table;

    struct {
        dri_bo *bo;
        unsigned int max_entries;
        unsigned int entry_size;        /* in bytes */
    } idrt;

    struct {
        dri_bo *bo;
        unsigned int length;            /* in bytes */
    } curbe;

    struct {
        unsigned int gpgpu_mode : 1;
        unsigned int pad0 : 7;
        unsigned int max_num_threads : 16;
        unsigned int num_urb_entries : 8;
        unsigned int urb_entry_size : 16;
        unsigned int curbe_allocation_size : 16;
    } vfe_state;
  
    /* vfe_desc5/6/7 is used to determine whether the HW scoreboard is used.
     * If scoreboard is not used, don't touch them
     */ 
    union { 
	unsigned int dword;
    	struct {
		unsigned int mask:8;
		unsigned int pad:22;
		unsigned int type:1;
		unsigned int enable:1;
	} scoreboard0; 
    }vfe_desc5;

    union {
	unsigned int dword;
	struct {
        	int delta_x0:4;
	        int delta_y0:4;
	        int delta_x1:4;
	        int delta_y1:4;
	        int delta_x2:4;
        	int delta_y2:4;
	        int delta_x3:4;
	        int delta_y3:4;
	} scoreboard1;
     } vfe_desc6;

    union {
	unsigned int dword;
	struct {
        	int delta_x4:4;
	        int delta_y4:4;
	        int delta_x5:4;
	        int delta_y5:4;
	        int delta_x6:4;
	        int delta_y6:4;
	        int delta_x7:4;
	        int delta_y7:4;
    	} scoreboard2;
     } vfe_desc7;

    unsigned int num_kernels;
    struct i965_kernel kernels[MAX_GPE_KERNELS];

    struct {
        dri_bo *bo;
        int bo_size;
        unsigned int end_offset;
    } instruction_state;

    struct {
        dri_bo *bo;
    } indirect_state;

    struct {
        dri_bo *bo;
        int bo_size;
        unsigned int end_offset;
    } dynamic_state;

    unsigned int sampler_offset;
    int sampler_size;
    unsigned int idrt_offset;
    int idrt_size;
    unsigned int curbe_offset;
    int curbe_size;
};

void i965_gpe_context_destroy(struct i965_gpe_context *gpe_context);
void i965_gpe_context_init(VADriverContextP ctx,
                           struct i965_gpe_context *gpe_context);
void i965_gpe_load_kernels(VADriverContextP ctx,
                           struct i965_gpe_context *gpe_context,
                           struct i965_kernel *kernel_list,
                           unsigned int num_kernels);
void gen6_gpe_pipeline_setup(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct intel_batchbuffer *batch);
void i965_gpe_surface2_setup(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct object_surface *obj_surface,
                             unsigned long binding_table_offset,
                             unsigned long surface_state_offset);
void i965_gpe_media_rw_surface_setup(VADriverContextP ctx,
                                     struct i965_gpe_context *gpe_context,
                                     struct object_surface *obj_surface,
                                     unsigned long binding_table_offset,
                                     unsigned long surface_state_offset);
void i965_gpe_buffer_suface_setup(VADriverContextP ctx,
                                  struct i965_gpe_context *gpe_context,
                                  struct i965_buffer_surface *buffer_surface,
                                  unsigned long binding_table_offset,
                                  unsigned long surface_state_offset);
void gen7_gpe_surface2_setup(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct object_surface *obj_surface,
                             unsigned long binding_table_offset,
                             unsigned long surface_state_offset);
void gen7_gpe_media_rw_surface_setup(VADriverContextP ctx,
                                     struct i965_gpe_context *gpe_context,
                                     struct object_surface *obj_surface,
                                     unsigned long binding_table_offset,
                                     unsigned long surface_state_offset);
void gen7_gpe_buffer_suface_setup(VADriverContextP ctx,
                                  struct i965_gpe_context *gpe_context,
                                  struct i965_buffer_surface *buffer_surface,
                                  unsigned long binding_table_offset,
                                  unsigned long surface_state_offset);
void gen75_gpe_media_chroma_surface_setup(VADriverContextP ctx,
                                     struct i965_gpe_context *gpe_context,
                                     struct object_surface *obj_surface,
                                     unsigned long binding_table_offset,
                                     unsigned long surface_state_offset);

extern void gen8_gpe_surface2_setup(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct object_surface *obj_surface,
                             unsigned long binding_table_offset,
                             unsigned long surface_state_offset);
extern void gen8_gpe_media_rw_surface_setup(VADriverContextP ctx,
                                     struct i965_gpe_context *gpe_context,
                                     struct object_surface *obj_surface,
                                     unsigned long binding_table_offset,
                                     unsigned long surface_state_offset);
extern void gen8_gpe_buffer_suface_setup(VADriverContextP ctx,
                                  struct i965_gpe_context *gpe_context,
                                  struct i965_buffer_surface *buffer_surface,
                                  unsigned long binding_table_offset,
                                  unsigned long surface_state_offset);
extern void gen8_gpe_media_chroma_surface_setup(VADriverContextP ctx,
                                     struct i965_gpe_context *gpe_context,
                                     struct object_surface *obj_surface,
                                     unsigned long binding_table_offset,
                                     unsigned long surface_state_offset);

void gen8_gpe_pipeline_setup(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct intel_batchbuffer *batch);


void gen8_gpe_context_destroy(struct i965_gpe_context *gpe_context);
void gen8_gpe_context_init(VADriverContextP ctx,
                           struct i965_gpe_context *gpe_context);

void gen8_gpe_load_kernels(VADriverContextP ctx,
                           struct i965_gpe_context *gpe_context,
                           struct i965_kernel *kernel_list,
                           unsigned int num_kernels);

void gen9_gpe_pipeline_setup(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct intel_batchbuffer *batch);

void gen9_gpe_pipeline_end(VADriverContextP ctx,
                             struct i965_gpe_context *gpe_context,
                             struct intel_batchbuffer *batch);

#endif /* _I965_GPE_UTILS_H_ */
