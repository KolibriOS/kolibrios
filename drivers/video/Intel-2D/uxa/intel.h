/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright © 2002 David Dawes

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Keith Whitwell <keith@tungstengraphics.com>
 *   David Dawes <dawes@xfree86.org>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if 0
#define I830DEBUG
#endif

#include <stdint.h>

#ifndef REMAP_RESERVED
#define REMAP_RESERVED 0
#endif

#include "picture.h"
#include <pciaccess.h>
#include "intel_bufmgr.h"
#include "i915_drm.h"

#include "intel_driver.h"
#include "intel_list.h"

# define ENTER() printf("ENTER %s\n", __FUNCTION__)
# define LEAVE() printf("LEAVE %s\n", __FUNCTION__)
# define FAIL() printf("FAIL %s\n", __FUNCTION__)

typedef int ScrnInfoPtr;
typedef int ScreenPtr;

typedef int Bool;

#define  TRUE  (Bool)(1)
#define  FALSE (Bool)(0)

typedef struct pixman_transform PictTransform, *PictTransformPtr;
typedef struct _Pixmap  *PixmapPtr;
typedef struct _Picture *PicturePtr;

/* remain compatible to xorg-server 1.6 */
#ifndef MONITOR_EDID_COMPLETE_RAWDATA
#define MONITOR_EDID_COMPLETE_RAWDATA EDID_COMPLETE_RAWDATA
#endif

#if XF86_CRTC_VERSION >= 5
#define INTEL_PIXMAP_SHARING 1
#endif

struct intel_pixmap {
	dri_bo *bo;

	struct list batch;

	uint16_t stride;
	uint8_t tiling;
	int8_t busy :2;
	uint8_t dirty :1;
	uint8_t offscreen :1;
	uint8_t pinned :3;
#define PIN_SCANOUT 0x1
#define PIN_DRI 0x2
#define PIN_GLAMOR 0x4
};

#if HAS_DEVPRIVATEKEYREC
extern DevPrivateKeyRec uxa_pixmap_index;
#else
extern int uxa_pixmap_index;
#endif

/*
static inline struct intel_pixmap *intel_get_pixmap_private(PixmapPtr pixmap)
{
#if HAS_DEVPRIVATEKEYREC
	return dixGetPrivate(&pixmap->devPrivates, &uxa_pixmap_index);
#else
	return dixLookupPrivate(&pixmap->devPrivates, &uxa_pixmap_index);
#endif
}
*/

static inline Bool intel_pixmap_is_busy(struct intel_pixmap *priv)
{
	if (priv->busy == -1)
		priv->busy = drm_intel_bo_busy(priv->bo);
	return priv->busy;
}

static inline void intel_set_pixmap_private(PixmapPtr pixmap, struct intel_pixmap *intel)
{
	dixSetPrivate(&pixmap->devPrivates, &uxa_pixmap_index, intel);
}

static inline Bool intel_pixmap_is_dirty(PixmapPtr pixmap)
{
	return pixmap && pixmap->private->dirty;
}

static inline Bool intel_pixmap_tiled(PixmapPtr pixmap)
{
	return pixmap->private->tiling != I915_TILING_NONE;
}

dri_bo *intel_get_pixmap_bo(PixmapPtr pixmap);
void intel_set_pixmap_bo(PixmapPtr pixmap, dri_bo * bo);

#include "common.h"

#define PITCH_NONE 0

/** enumeration of 3d consumers so some can maintain invariant state. */
enum last_3d {
    LAST_3D_OTHER,
    LAST_3D_VIDEO,
    LAST_3D_RENDER,
    LAST_3D_ROTATION
};

enum dri_type {
	DRI_DISABLED,
	DRI_NONE,
	DRI_DRI2
};

typedef struct intel_screen_private {
	int scrn;
    int cpp;

#define RENDER_BATCH                I915_EXEC_RENDER
#define BLT_BATCH                   I915_EXEC_BLT
	unsigned int current_batch;

	void *modes;
	drm_intel_bo *front_buffer, *back_buffer;
	PixmapPtr back_pixmap;
	unsigned int back_name;
	long front_pitch, front_tiling;

    dri_bufmgr *bufmgr;

    uint32_t batch_ptr[4096];
    /** Byte offset in batch_ptr for the next dword to be emitted. */
    unsigned int batch_used;
    /** Position in batch_ptr at the start of the current BEGIN_BATCH */
    unsigned int batch_emit_start;
    /** Number of bytes to be emitted in the current BEGIN_BATCH. */
    uint32_t batch_emitting;
    dri_bo *batch_bo, *last_batch_bo[2];
    /** Whether we're in a section of code that can't tolerate flushing */
    Bool in_batch_atomic;
    /** Ending batch_used that was verified by intel_start_batch_atomic() */
    int batch_atomic_limit;
    struct list batch_pixmaps;
    drm_intel_bo *wa_scratch_bo;

    unsigned int tiling;
#define INTEL_TILING_FB     0x1
#define INTEL_TILING_2D     0x2
#define INTEL_TILING_3D     0x4
#define INTEL_TILING_ALL   (~0)

	Bool swapbuffers_wait;
    Bool has_relaxed_fencing;

    int Chipset;
	struct pci_device *PciInfo;
	const struct intel_device_info *info;

    unsigned int BR[20];

	void (*context_switch) (struct intel_screen_private *intel,
				int new_mode);
    void (*vertex_flush) (struct intel_screen_private *intel);
    void (*batch_flush) (struct intel_screen_private *intel);
    void (*batch_commit_notify) (struct intel_screen_private *intel);

    Bool need_sync;
    int accel_pixmap_offset_alignment;
    int accel_max_x;
    int accel_max_y;
    int max_bo_size;
    int max_gtt_map_size;
    int max_tiling_size;

    struct {
        drm_intel_bo *gen4_vs_bo;
        drm_intel_bo *gen4_sf_bo;
        drm_intel_bo *gen4_wm_packed_bo;
        drm_intel_bo *gen4_wm_planar_bo;
        drm_intel_bo *gen4_cc_bo;
        drm_intel_bo *gen4_cc_vp_bo;
        drm_intel_bo *gen4_sampler_bo;
        drm_intel_bo *gen4_sip_kernel_bo;
        drm_intel_bo *wm_prog_packed_bo;
        drm_intel_bo *wm_prog_planar_bo;
        drm_intel_bo *gen6_blend_bo;
        drm_intel_bo *gen6_depth_stencil_bo;
    } video;

    /* Render accel state */
    float scale_units[2][2];
    /** Transform pointers for src/mask, or NULL if identity */
    PictTransform *transform[2];

    PixmapPtr render_source, render_mask, render_dest;
    PicturePtr render_source_picture, render_mask_picture, render_dest_picture;
    Bool needs_3d_invariant;
    Bool needs_render_state_emit;
    Bool needs_render_vertex_emit;

    /* i830 render accel state */
    uint32_t render_dest_format;
    uint32_t cblend, ablend, s8_blendctl;

    /* i915 render accel state */
    PixmapPtr texture[2];
    uint32_t mapstate[6];
    uint32_t samplerstate[6];

    struct {
        int op;
        uint32_t dst_format;
    } i915_render_state;

    struct {
        int num_sf_outputs;
        int drawrect;
        uint32_t blend;
        dri_bo *samplers;
        dri_bo *kernel;
    } gen6_render_state;

    uint32_t prim_offset;
    void (*prim_emit)(struct intel_screen_private *intel,
              int srcX, int srcY,
              int maskX, int maskY,
              int dstX, int dstY,
              int w, int h);
    int floats_per_vertex;
    int last_floats_per_vertex;
    uint16_t vertex_offset;
    uint16_t vertex_count;
    uint16_t vertex_index;
    uint16_t vertex_used;
    uint32_t vertex_id;
    float vertex_ptr[4*1024];
    dri_bo *vertex_bo;

    uint8_t surface_data[16*1024];
    uint16_t surface_used;
    uint16_t surface_table;
    uint32_t surface_reloc;
    dri_bo *surface_bo;

    /* 965 render acceleration state */
    struct gen4_render_state *gen4_render_state;

    Bool use_pageflipping;
    Bool use_triple_buffer;
    Bool force_fallback;
    Bool has_kernel_flush;
    Bool needs_flush;

    enum last_3d last_3d;

    /**
     * User option to print acceleration fallback info to the server log.
     */
    Bool fallback_debug;
    unsigned debug_flush;
    Bool has_prime_vmap_flush;
} intel_screen_private;

#define INTEL_INFO(intel) ((intel)->info)
#define IS_GENx(intel, X) (INTEL_INFO(intel)->gen >= 8*(X) && INTEL_INFO(intel)->gen < 8*((X)+1))
#define IS_GEN1(intel) IS_GENx(intel, 1)
#define IS_GEN2(intel) IS_GENx(intel, 2)
#define IS_GEN3(intel) IS_GENx(intel, 3)
#define IS_GEN4(intel) IS_GENx(intel, 4)
#define IS_GEN5(intel) IS_GENx(intel, 5)
#define IS_GEN6(intel) IS_GENx(intel, 6)
#define IS_GEN7(intel) IS_GENx(intel, 7)
#define IS_HSW(intel) (INTEL_INFO(intel)->gen == 075)

/* Some chips have specific errata (or limits) that we need to workaround. */
#define IS_I830(intel) ((intel)->PciInfo->device_id == PCI_CHIP_I830_M)
#define IS_845G(intel) ((intel)->PciInfo->device_id == PCI_CHIP_845_G)
#define IS_I865G(intel) ((intel)->PciInfo->device_id == PCI_CHIP_I865_G)

#define IS_I915G(pI810) ((intel)->PciInfo->device_id == PCI_CHIP_I915_G || (intel)->PciInfo->device_id == PCI_CHIP_E7221_G)
#define IS_I915GM(pI810) ((intel)->PciInfo->device_id == PCI_CHIP_I915_GM)

#define IS_965_Q(pI810) ((intel)->PciInfo->device_id == PCI_CHIP_I965_Q)

/* supports Y tiled surfaces (pre-965 Mesa isn't ready yet) */
#define SUPPORTS_YTILING(pI810) (INTEL_INFO(intel)->gen >= 040)
#define HAS_BLT(pI810) (INTEL_INFO(intel)->gen >= 060)

#ifndef I915_PARAM_HAS_PRIME_VMAP_FLUSH
#define I915_PARAM_HAS_PRIME_VMAP_FLUSH 21
#endif

enum {
	DEBUG_FLUSH_BATCHES = 0x1,
	DEBUG_FLUSH_CACHES = 0x2,
	DEBUG_FLUSH_WAIT = 0x4,
};

extern intel_screen_private *driverPrivate;

static inline intel_screen_private *
intel_get_screen_private()
{
	return driverPrivate;
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef ALIGN
#define ALIGN(i,m)	(((i) + (m) - 1) & ~((m) - 1))
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

static inline unsigned long intel_pixmap_pitch(PixmapPtr pixmap)
{
	return (unsigned long)pixmap->devKind;
}

/* Batchbuffer support macros and functions */
#include "intel_batchbuffer.h"
void i965_vertex_flush(intel_screen_private *intel);
void i965_batch_flush(intel_screen_private *intel);
void i965_batch_commit_notify(intel_screen_private *intel);
/**
 * Little wrapper around drm_intel_bo_reloc to return the initial value you
 * should stuff into the relocation entry.
 *
 * If only we'd done this before settling on the library API.
 */
static inline uint32_t
intel_emit_reloc(drm_intel_bo * bo, uint32_t offset,
		 drm_intel_bo * target_bo, uint32_t target_offset,
		 uint32_t read_domains, uint32_t write_domain)
{
	drm_intel_bo_emit_reloc(bo, offset, target_bo, target_offset,
				read_domains, write_domain);

	return target_bo->offset + target_offset;
}

static inline drm_intel_bo *intel_bo_alloc_for_data(intel_screen_private *intel,
						    const void *data,
						    unsigned int size,
						    const char *name)
{
	drm_intel_bo *bo;
	int ret;

	bo = drm_intel_bo_alloc(intel->bufmgr, name, size, 4096);
	assert(bo);

	ret = drm_intel_bo_subdata(bo, 0, size, data);
	assert(ret == 0);

	return bo;
	(void)ret;
}
