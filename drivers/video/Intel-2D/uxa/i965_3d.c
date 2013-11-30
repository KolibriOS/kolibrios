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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include "intel.h"
#include "i965_reg.h"
#include "brw_defines.h"
#include "brw_structs.h"

void
gen6_upload_invariant_states(intel_screen_private *intel)
{
	Bool ivb = INTEL_INFO(intel)->gen >= 070;

	OUT_BATCH(BRW_PIPE_CONTROL | (4 - 2));
	OUT_BATCH(BRW_PIPE_CONTROL_IS_FLUSH |
		BRW_PIPE_CONTROL_WC_FLUSH |
		BRW_PIPE_CONTROL_DEPTH_CACHE_FLUSH |
		BRW_PIPE_CONTROL_NOWRITE);
	OUT_BATCH(0); /* write address */
	OUT_BATCH(0); /* write data */

	OUT_BATCH(NEW_PIPELINE_SELECT | PIPELINE_SELECT_3D);

	OUT_BATCH(GEN6_3DSTATE_MULTISAMPLE | ((ivb ? 4 : 3) - 2));
	OUT_BATCH(GEN6_3DSTATE_MULTISAMPLE_PIXEL_LOCATION_CENTER |
		GEN6_3DSTATE_MULTISAMPLE_NUMSAMPLES_1); /* 1 sample/pixel */
	OUT_BATCH(0);
	if (ivb)
		OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_SAMPLE_MASK | (2 - 2));
	OUT_BATCH(1);

	/* Set system instruction pointer */
	OUT_BATCH(BRW_STATE_SIP | 0);
	OUT_BATCH(0);
}

void
gen6_upload_viewport_state_pointers(intel_screen_private *intel,
				    drm_intel_bo *cc_vp_bo)
{
	OUT_BATCH(GEN6_3DSTATE_VIEWPORT_STATE_POINTERS |
		GEN6_3DSTATE_VIEWPORT_STATE_MODIFY_CC |
		(4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_RELOC(cc_vp_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
}

void
gen7_upload_viewport_state_pointers(intel_screen_private *intel,
				    drm_intel_bo *cc_vp_bo)
{
	OUT_BATCH(GEN7_3DSTATE_VIEWPORT_STATE_POINTERS_CC | (2 - 2));
	OUT_RELOC(cc_vp_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);

	OUT_BATCH(GEN7_3DSTATE_VIEWPORT_STATE_POINTERS_SF_CL | (2 - 2));
	OUT_BATCH(0);
}

void
gen6_upload_urb(intel_screen_private *intel)
{
	OUT_BATCH(GEN6_3DSTATE_URB | (3 - 2));
	OUT_BATCH(((1 - 1) << GEN6_3DSTATE_URB_VS_SIZE_SHIFT) |
		(24 << GEN6_3DSTATE_URB_VS_ENTRIES_SHIFT)); /* at least 24 on GEN6 */
	OUT_BATCH((0 << GEN6_3DSTATE_URB_GS_SIZE_SHIFT) |
		(0 << GEN6_3DSTATE_URB_GS_ENTRIES_SHIFT)); /* no GS thread */
}

/*
 * URB layout on GEN7
 * ----------------------------------------
 * | PS Push Constants (8KB) | VS entries |
 * ----------------------------------------
 */
void
gen7_upload_urb(intel_screen_private *intel)
{
	unsigned int num_urb_entries = 32;

	if (IS_HSW(intel))
		num_urb_entries = 64;

	OUT_BATCH(GEN7_3DSTATE_PUSH_CONSTANT_ALLOC_PS | (2 - 2));
	OUT_BATCH(8); /* in 1KBs */

	OUT_BATCH(GEN7_3DSTATE_URB_VS | (2 - 2));
	OUT_BATCH(
		(num_urb_entries << GEN7_URB_ENTRY_NUMBER_SHIFT) |
		(2 - 1) << GEN7_URB_ENTRY_SIZE_SHIFT |
		(1 << GEN7_URB_STARTING_ADDRESS_SHIFT));

	OUT_BATCH(GEN7_3DSTATE_URB_GS | (2 - 2));
	OUT_BATCH((0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
		(1 << GEN7_URB_STARTING_ADDRESS_SHIFT));

	OUT_BATCH(GEN7_3DSTATE_URB_HS | (2 - 2));
	OUT_BATCH((0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
		(2 << GEN7_URB_STARTING_ADDRESS_SHIFT));

	OUT_BATCH(GEN7_3DSTATE_URB_DS | (2 - 2));
	OUT_BATCH((0 << GEN7_URB_ENTRY_SIZE_SHIFT) |
		(2 << GEN7_URB_STARTING_ADDRESS_SHIFT));
}

void
gen6_upload_cc_state_pointers(intel_screen_private *intel,
			      drm_intel_bo *blend_bo,
			      drm_intel_bo *cc_bo,
			      drm_intel_bo *depth_stencil_bo,
			      uint32_t blend_offset)
{
	OUT_BATCH(GEN6_3DSTATE_CC_STATE_POINTERS | (4 - 2));
	if (blend_bo)
		OUT_RELOC(blend_bo, I915_GEM_DOMAIN_INSTRUCTION, 0,
			  blend_offset | 1);
	else
		OUT_BATCH(0);

	if (depth_stencil_bo)
		OUT_RELOC(depth_stencil_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 1);
	else
		OUT_BATCH(0);

	if (cc_bo)
		OUT_RELOC(cc_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 1);
	else
		OUT_BATCH(0);
}

void
gen7_upload_cc_state_pointers(intel_screen_private *intel,
			      drm_intel_bo *blend_bo,
			      drm_intel_bo *cc_bo,
			      drm_intel_bo *depth_stencil_bo,
			      uint32_t blend_offset)
{
	OUT_BATCH(GEN7_3DSTATE_BLEND_STATE_POINTERS | (2 - 2));
	if (blend_bo)
		OUT_RELOC(blend_bo, I915_GEM_DOMAIN_INSTRUCTION, 0,
			  blend_offset | 1);
	else
		OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_CC_STATE_POINTERS | (2 - 2));
	if (cc_bo)
		OUT_RELOC(cc_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 1);
	else
		OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_DEPTH_STENCIL_STATE_POINTERS | (2 - 2));
	if (depth_stencil_bo)
		OUT_RELOC(depth_stencil_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 1);
	else
		OUT_BATCH(0);
}

void
gen6_upload_sampler_state_pointers(intel_screen_private *intel,
				   drm_intel_bo *sampler_bo)
{
	OUT_BATCH(GEN6_3DSTATE_SAMPLER_STATE_POINTERS |
		GEN6_3DSTATE_SAMPLER_STATE_MODIFY_PS |
		(4 - 2));
	OUT_BATCH(0); /* VS */
	OUT_BATCH(0); /* GS */
	OUT_RELOC(sampler_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
}

void
gen7_upload_sampler_state_pointers(intel_screen_private *intel,
				   drm_intel_bo *sampler_bo)
{
	OUT_BATCH(GEN7_3DSTATE_SAMPLER_STATE_POINTERS_PS | (2 - 2));
	OUT_RELOC(sampler_bo, I915_GEM_DOMAIN_INSTRUCTION, 0, 0);
}

void
gen7_upload_bypass_states(intel_screen_private *intel)
{
	/* bypass GS */
	OUT_BATCH(GEN6_3DSTATE_CONSTANT_GS | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_GS | (7 - 2));
	OUT_BATCH(0); /* without GS kernel */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */

	OUT_BATCH(GEN7_3DSTATE_BINDING_TABLE_POINTERS_GS | (2 - 2));
	OUT_BATCH(0);

	/* disable HS */
	OUT_BATCH(GEN7_3DSTATE_CONSTANT_HS | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_HS | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_BINDING_TABLE_POINTERS_HS | (2 - 2));
	OUT_BATCH(0);

	/* Disable TE */
	OUT_BATCH(GEN7_3DSTATE_TE | (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	/* Disable DS */
	OUT_BATCH(GEN7_3DSTATE_CONSTANT_DS | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_DS | (6 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_BINDING_TABLE_POINTERS_DS | (2 - 2));
	OUT_BATCH(0);

	/* Disable STREAMOUT */
	OUT_BATCH(GEN7_3DSTATE_STREAMOUT | (3 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
}

void
gen6_upload_vs_state(intel_screen_private *intel)
{
	Bool ivb = INTEL_INFO(intel)->gen >= 070;
	/* disable VS constant buffer */
	OUT_BATCH(GEN6_3DSTATE_CONSTANT_VS | ((ivb ? 7 : 5) - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	if (ivb) {
		OUT_BATCH(0);
		OUT_BATCH(0);
	}

	OUT_BATCH(GEN6_3DSTATE_VS | (6 - 2));
	OUT_BATCH(0); /* without VS kernel */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */
}

void
gen6_upload_gs_state(intel_screen_private *intel)
{
	/* disable GS constant buffer */
	OUT_BATCH(GEN6_3DSTATE_CONSTANT_GS | (5 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_GS | (7 - 2));
	OUT_BATCH(0); /* without GS kernel */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */
}

void
gen6_upload_clip_state(intel_screen_private *intel)
{
	OUT_BATCH(GEN6_3DSTATE_CLIP | (4 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0); /* pass-through */
	OUT_BATCH(0);
}

void
gen6_upload_sf_state(intel_screen_private *intel,
		     int num_sf_outputs,
		     int read_offset)
{
	OUT_BATCH(GEN6_3DSTATE_SF | (20 - 2));
	OUT_BATCH((num_sf_outputs << GEN6_3DSTATE_SF_NUM_OUTPUTS_SHIFT) |
		(1 << GEN6_3DSTATE_SF_URB_ENTRY_READ_LENGTH_SHIFT) |
		(read_offset << GEN6_3DSTATE_SF_URB_ENTRY_READ_OFFSET_SHIFT));
	OUT_BATCH(0);
	OUT_BATCH(GEN6_3DSTATE_SF_CULL_NONE);
	OUT_BATCH(2 << GEN6_3DSTATE_SF_TRIFAN_PROVOKE_SHIFT); /* DW4 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW9 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW14 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW19 */
}

void
gen7_upload_sf_state(intel_screen_private *intel,
		     int num_sf_outputs,
		     int read_offset)
{
	OUT_BATCH(GEN7_3DSTATE_SBE | (14 - 2));
	OUT_BATCH((num_sf_outputs << GEN7_SBE_NUM_OUTPUTS_SHIFT) |
		(1 << GEN7_SBE_URB_ENTRY_READ_LENGTH_SHIFT) |
		(read_offset << GEN7_SBE_URB_ENTRY_READ_OFFSET_SHIFT));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW4 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0); /* DW9 */
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN6_3DSTATE_SF | (7 - 2));
	OUT_BATCH(0);
	OUT_BATCH(GEN6_3DSTATE_SF_CULL_NONE);
	OUT_BATCH(2 << GEN6_3DSTATE_SF_TRIFAN_PROVOKE_SHIFT);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
}

void
gen6_upload_binding_table(intel_screen_private *intel,
			  uint32_t ps_binding_table_offset)
{
	/* Binding table pointers */
	OUT_BATCH(BRW_3DSTATE_BINDING_TABLE_POINTERS |
		  GEN6_3DSTATE_BINDING_TABLE_MODIFY_PS |
		  (4 - 2));
	OUT_BATCH(0); /* VS */
	OUT_BATCH(0); /* GS */
	/* Only the PS uses the binding table */
	OUT_BATCH(ps_binding_table_offset);
}

void
gen7_upload_binding_table(intel_screen_private *intel,
			  uint32_t ps_binding_table_offset)
{
	OUT_BATCH(GEN7_3DSTATE_BINDING_TABLE_POINTERS_PS | (2 - 2));
	OUT_BATCH(ps_binding_table_offset);
}

void
gen6_upload_depth_buffer_state(intel_screen_private *intel)
{
	OUT_BATCH(BRW_3DSTATE_DEPTH_BUFFER | (7 - 2));
	OUT_BATCH((BRW_SURFACE_NULL << BRW_3DSTATE_DEPTH_BUFFER_TYPE_SHIFT) |
		  (BRW_DEPTHFORMAT_D32_FLOAT << BRW_3DSTATE_DEPTH_BUFFER_FORMAT_SHIFT));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(BRW_3DSTATE_CLEAR_PARAMS | (2 - 2));
	OUT_BATCH(0);
}

void
gen7_upload_depth_buffer_state(intel_screen_private *intel)
{
	OUT_BATCH(GEN7_3DSTATE_DEPTH_BUFFER | (7 - 2));
	OUT_BATCH((BRW_DEPTHFORMAT_D32_FLOAT << 18) | (BRW_SURFACE_NULL << 29));
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);
	OUT_BATCH(0);

	OUT_BATCH(GEN7_3DSTATE_CLEAR_PARAMS | (3 - 2));
	OUT_BATCH(0);
	OUT_BATCH(0);
}
