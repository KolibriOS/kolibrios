/*
 * Copyright © 2008 Intel Corporation
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
 *    Keith Packard <keithp@keithp.com>
 *
 */

#include <linux/i2c.h>
//#include <linux/slab.h>
#include "drmP.h"
#include "drm.h"
#include "drm_crtc.h"
#include "drm_crtc_helper.h"
#include "intel_drv.h"
//#include "i915_drm.h"
#include "i915_drv.h"
#include "drm_dp_helper.h"


#define DP_LINK_STATUS_SIZE 6
#define DP_LINK_CHECK_TIMEOUT   (10 * 1000)

#define DP_LINK_CONFIGURATION_SIZE  9

struct intel_dp {
    struct intel_encoder base;
    uint32_t output_reg;
    uint32_t DP;
    uint8_t  link_configuration[DP_LINK_CONFIGURATION_SIZE];
    bool has_audio;
    int force_audio;
    uint32_t color_range;
    int dpms_mode;
    uint8_t link_bw;
    uint8_t lane_count;
    uint8_t dpcd[8];
    struct i2c_adapter adapter;
    struct i2c_algo_dp_aux_data algo;
    bool is_pch_edp;
    uint8_t train_set[4];
    uint8_t link_status[DP_LINK_STATUS_SIZE];
};

/**
 * is_edp - is the given port attached to an eDP panel (either CPU or PCH)
 * @intel_dp: DP struct
 *
 * If a CPU or PCH DP output is attached to an eDP panel, this function
 * will return true, and false otherwise.
 */
static bool is_edp(struct intel_dp *intel_dp)
{
	return intel_dp->base.type == INTEL_OUTPUT_EDP;
}

/**
 * is_pch_edp - is the port on the PCH and attached to an eDP panel?
 * @intel_dp: DP struct
 *
 * Returns true if the given DP struct corresponds to a PCH DP port attached
 * to an eDP panel, false otherwise.  Helpful for determining whether we
 * may need FDI resources for a given DP output or not.
 */
static bool is_pch_edp(struct intel_dp *intel_dp)
{
	return intel_dp->is_pch_edp;
}

static struct intel_dp *enc_to_intel_dp(struct drm_encoder *encoder)
{
	return container_of(encoder, struct intel_dp, base.base);
}





/**
 * intel_encoder_is_pch_edp - is the given encoder a PCH attached eDP?
 * @encoder: DRM encoder
 *
 * Return true if @encoder corresponds to a PCH attached eDP panel.  Needed
 * by intel_display.c.
 */
bool intel_encoder_is_pch_edp(struct drm_encoder *encoder)
{
    struct intel_dp *intel_dp;

    if (!encoder)
        return false;

    intel_dp = enc_to_intel_dp(encoder);

    return is_pch_edp(intel_dp);
}

void
intel_edp_link_config (struct intel_encoder *intel_encoder,
		       int *lane_num, int *link_bw)
{
	struct intel_dp *intel_dp = container_of(intel_encoder, struct intel_dp, base);

	*lane_num = intel_dp->lane_count;
	if (intel_dp->link_bw == DP_LINK_BW_1_62)
		*link_bw = 162000;
	else if (intel_dp->link_bw == DP_LINK_BW_2_7)
		*link_bw = 270000;
}














struct intel_dp_m_n {
	uint32_t	tu;
	uint32_t	gmch_m;
	uint32_t	gmch_n;
	uint32_t	link_m;
	uint32_t	link_n;
};

static void
intel_reduce_ratio(uint32_t *num, uint32_t *den)
{
	while (*num > 0xffffff || *den > 0xffffff) {
		*num >>= 1;
		*den >>= 1;
	}
}

static void
intel_dp_compute_m_n(int bpp,
		     int nlanes,
		     int pixel_clock,
		     int link_clock,
		     struct intel_dp_m_n *m_n)
{
	m_n->tu = 64;
	m_n->gmch_m = (pixel_clock * bpp) >> 3;
	m_n->gmch_n = link_clock * nlanes;
	intel_reduce_ratio(&m_n->gmch_m, &m_n->gmch_n);
	m_n->link_m = pixel_clock;
	m_n->link_n = link_clock;
	intel_reduce_ratio(&m_n->link_m, &m_n->link_n);
}

void
intel_dp_set_m_n(struct drm_crtc *crtc, struct drm_display_mode *mode,
         struct drm_display_mode *adjusted_mode)
{
    struct drm_device *dev = crtc->dev;
    struct drm_mode_config *mode_config = &dev->mode_config;
    struct drm_encoder *encoder;
    struct drm_i915_private *dev_priv = dev->dev_private;
    struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
    int lane_count = 4;
    struct intel_dp_m_n m_n;
    int pipe = intel_crtc->pipe;

    /*
     * Find the lane count in the intel_encoder private
     */
    list_for_each_entry(encoder, &mode_config->encoder_list, head) {
        struct intel_dp *intel_dp;

        if (encoder->crtc != crtc)
            continue;

        intel_dp = enc_to_intel_dp(encoder);
        if (intel_dp->base.type == INTEL_OUTPUT_DISPLAYPORT) {
            lane_count = intel_dp->lane_count;
            break;
        } else if (is_edp(intel_dp)) {
            lane_count = dev_priv->edp.lanes;
            break;
        }
    }

    /*
     * Compute the GMCH and Link ratios. The '3' here is
     * the number of bytes_per_pixel post-LUT, which we always
     * set up for 8-bits of R/G/B, or 3 bytes total.
     */
    intel_dp_compute_m_n(intel_crtc->bpp, lane_count,
                 mode->clock, adjusted_mode->clock, &m_n);

    if (HAS_PCH_SPLIT(dev)) {
        I915_WRITE(TRANSDATA_M1(pipe),
               ((m_n.tu - 1) << PIPE_GMCH_DATA_M_TU_SIZE_SHIFT) |
               m_n.gmch_m);
        I915_WRITE(TRANSDATA_N1(pipe), m_n.gmch_n);
        I915_WRITE(TRANSDPLINK_M1(pipe), m_n.link_m);
        I915_WRITE(TRANSDPLINK_N1(pipe), m_n.link_n);
    } else {
        I915_WRITE(PIPE_GMCH_DATA_M(pipe),
               ((m_n.tu - 1) << PIPE_GMCH_DATA_M_TU_SIZE_SHIFT) |
               m_n.gmch_m);
        I915_WRITE(PIPE_GMCH_DATA_N(pipe), m_n.gmch_n);
        I915_WRITE(PIPE_DP_LINK_M(pipe), m_n.link_m);
        I915_WRITE(PIPE_DP_LINK_N(pipe), m_n.link_n);
    }
}
















































/* Return which DP Port should be selected for Transcoder DP control */
int
intel_trans_dp_port_sel (struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct drm_mode_config *mode_config = &dev->mode_config;
	struct drm_encoder *encoder;

	list_for_each_entry(encoder, &mode_config->encoder_list, head) {
		struct intel_dp *intel_dp;

		if (encoder->crtc != crtc)
			continue;

		intel_dp = enc_to_intel_dp(encoder);
		if (intel_dp->base.type == INTEL_OUTPUT_DISPLAYPORT)
			return intel_dp->output_reg;
	}

	return -1;
}
