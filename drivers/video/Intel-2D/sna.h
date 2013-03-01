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

#ifndef _SNA_H_
#define _SNA_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <memory.h>
#include <malloc.h>
#include <errno.h>

#include "intel_driver.h"
#include "pciaccess.h"

#include "compiler.h"

//#define DBG(x)
//#define DBG(x) ErrorF x

#define assert(x)


int drmIoctl(int fd, unsigned long request, void *arg);


#define SRV_GET_PCI_INFO            20
#define SRV_GET_PARAM               21
#define SRV_I915_GEM_CREATE         22
#define SRV_DRM_GEM_CLOSE           23
#define SRV_I915_GEM_PIN            24
#define SRV_I915_GEM_SET_CACHEING   25
#define SRV_I915_GEM_GET_APERTURE   26
#define SRV_I915_GEM_PWRITE         27
#define SRV_I915_GEM_BUSY           28
#define SRV_I915_GEM_SET_DOMAIN     29
#define SRV_I915_GEM_MMAP           30
#define SRV_I915_GEM_THROTTLE       32
#define SRV_FBINFO                  33
#define SRV_I915_GEM_EXECBUFFER2    34 
#define SRV_MASK_UPDATE             35

#define SRV_I915_GEM_MMAP_GTT       31


#define DRM_IOCTL_GEM_CLOSE         SRV_DRM_GEM_CLOSE

#define PIXMAN_FORMAT(bpp,type,a,r,g,b) (((bpp) << 24) |    \
                                        ((type) << 16) |    \
                                        ((a) << 12)    |    \
                                        ((r) << 8) |        \
                                        ((g) << 4) |        \
                                        ((b)))
#define PIXMAN_TYPE_OTHER	0
#define PIXMAN_TYPE_A		1
#define PIXMAN_TYPE_ARGB	2
#define PIXMAN_TYPE_ABGR	3
#define PIXMAN_TYPE_COLOR	4
#define PIXMAN_TYPE_GRAY	5
#define PIXMAN_TYPE_YUY2	6
#define PIXMAN_TYPE_YV12	7
#define PIXMAN_TYPE_BGRA	8
#define PIXMAN_TYPE_RGBA	9
#define PIXMAN_TYPE_ARGB_SRGB	10

/* 32bpp formats */
typedef enum {
    PIXMAN_a8r8g8b8 =    PIXMAN_FORMAT(32,PIXMAN_TYPE_ARGB,8,8,8,8),
    PIXMAN_x8r8g8b8 =    PIXMAN_FORMAT(32,PIXMAN_TYPE_ARGB,0,8,8,8),
    PIXMAN_a8b8g8r8 =    PIXMAN_FORMAT(32,PIXMAN_TYPE_ABGR,8,8,8,8),
    PIXMAN_x8b8g8r8 =    PIXMAN_FORMAT(32,PIXMAN_TYPE_ABGR,0,8,8,8),
    PIXMAN_b8g8r8a8 =    PIXMAN_FORMAT(32,PIXMAN_TYPE_BGRA,8,8,8,8),
    PIXMAN_b8g8r8x8 =    PIXMAN_FORMAT(32,PIXMAN_TYPE_BGRA,0,8,8,8),
    PIXMAN_r8g8b8a8 =    PIXMAN_FORMAT(32,PIXMAN_TYPE_RGBA,8,8,8,8),
    PIXMAN_r8g8b8x8 =    PIXMAN_FORMAT(32,PIXMAN_TYPE_RGBA,0,8,8,8),
    PIXMAN_x14r6g6b6 =   PIXMAN_FORMAT(32,PIXMAN_TYPE_ARGB,0,6,6,6),
    PIXMAN_x2r10g10b10 = PIXMAN_FORMAT(32,PIXMAN_TYPE_ARGB,0,10,10,10),
    PIXMAN_a2r10g10b10 = PIXMAN_FORMAT(32,PIXMAN_TYPE_ARGB,2,10,10,10),
    PIXMAN_x2b10g10r10 = PIXMAN_FORMAT(32,PIXMAN_TYPE_ABGR,0,10,10,10),
    PIXMAN_a2b10g10r10 = PIXMAN_FORMAT(32,PIXMAN_TYPE_ABGR,2,10,10,10),

    PIXMAN_a8 =		 PIXMAN_FORMAT(8,PIXMAN_TYPE_A,8,0,0,0)
    
} pixman_format_code_t;

typedef enum _PictFormatShort {

    PICT_a8r8g8b8 = PIXMAN_a8r8g8b8,
    PICT_x8r8g8b8 = PIXMAN_x8r8g8b8,
    PICT_a8b8g8r8 = PIXMAN_a8b8g8r8,
    PICT_x8b8g8r8 = PIXMAN_x8b8g8r8,
    PICT_b8g8r8a8 = PIXMAN_b8g8r8a8,
    PICT_b8g8r8x8 = PIXMAN_b8g8r8x8,

/* 8bpp formats */
    PICT_a8 = PIXMAN_a8,

/* 4bpp formats */
} PictFormatShort;

#define PIXMAN_FORMAT_A(f)	(((f) >> 12) & 0x0f)
#define PIXMAN_FORMAT_RGB(f)	(((f)      ) & 0xfff)

#define PICT_FORMAT_A(f)	PIXMAN_FORMAT_A(f)

#define RepeatNone                          0
#define RepeatNormal                        1
#define RepeatPad                           2
#define RepeatReflect                       3

#define PictFilterNearest	0
#define PictFilterBilinear	1

#define PictFilterFast		2
#define PictFilterGood		3
#define PictFilterBest		4

#define PictFilterConvolution	5

typedef int32_t			pixman_fixed_16_16_t;
typedef pixman_fixed_16_16_t	pixman_fixed_t;

struct pixman_transform
{
    pixman_fixed_t	matrix[3][3];
};

typedef unsigned long   Picture;
typedef unsigned long   PictFormat;

typedef struct _Pixmap  *PixmapPtr;
typedef struct _Picture *PicturePtr;
typedef struct _Drawable *DrawablePtr;
typedef struct _PictFormat *PictFormatPtr;

typedef struct pixman_transform PictTransform, *PictTransformPtr;



typedef struct _Drawable {
    unsigned char type;         /* DRAWABLE_<type> */
    unsigned char class;        /* specific to type */
    unsigned char depth;
    unsigned char bitsPerPixel;
    unsigned int   id;           /* resource id */
    short x;                    /* window: screen absolute, pixmap: 0 */
    short y;                    /* window: screen absolute, pixmap: 0 */
    unsigned short width;
    unsigned short height;
} DrawableRec;

/*
 * PIXMAP -- device dependent
 */

typedef struct _Pixmap {
    DrawableRec drawable;
//    PrivateRec *devPrivates;
    int refcnt;
    int devKind;                /* This is the pitch of the pixmap, typically width*bpp/8. */
//    DevUnion devPrivate;        /* When !NULL, devPrivate.ptr points to the raw pixel data. */
#ifdef COMPOSITE
    short screen_x;
    short screen_y;
#endif
    unsigned usage_hint;        /* see CREATE_PIXMAP_USAGE_* */

    PixmapPtr master_pixmap;    /* pointer to master copy of pixmap for pixmap sharing */
} PixmapRec;

typedef struct _PictFormat {
    uint32_t id;
    uint32_t format;              /* except bpp */
    unsigned char type;
    unsigned char depth;
//    DirectFormatRec direct;
//   IndexFormatRec index;
} PictFormatRec;

typedef struct _Picture {
    DrawablePtr pDrawable;
//    PictFormatPtr pFormat;
    PictFormatShort format;     /* PICT_FORMAT */
    int refcnt;
    uint32_t id;
    unsigned int repeat:1;
    unsigned int graphicsExposures:1;
    unsigned int subWindowMode:1;
    unsigned int polyEdge:1;
    unsigned int polyMode:1;
    unsigned int freeCompClip:1;
    unsigned int clientClipType:2;
    unsigned int componentAlpha:1;
    unsigned int repeatType:2;
    unsigned int filter:3;
//    unsigned int stateChanges:CPLastBit;
//    unsigned int unused:18 - CPLastBit;

//    PicturePtr alphaMap;

//    PictTransform *transform;

//    SourcePictPtr pSourcePict;
//    xFixed *filter_params;
//    int filter_nparams;
} PictureRec;

#define PolyModePrecise			    0
#define PolyModeImprecise		    1


struct sna_fb
{
    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch;
    uint32_t  tiling;
    
    struct kgem_bo *fb_bo;
};

struct pixman_box16
{
    int16_t x1, y1, x2, y2;
};

typedef struct pixman_box16 BoxRec;
typedef unsigned int   CARD32;
typedef unsigned short CARD16;

#include "sna_render.h"
#include "kgem.h"

#define GXclear                 0x0
#define GXcopy                  0x3

#define PictOpClear             0
#define PictOpSrc               1
#define PictOpDst               2
#define PictOpOver              3
#define PictOpOverReverse       4
#define PictOpIn                5
#define PictOpInReverse         6
#define PictOpOut               7
#define PictOpOutReverse        8
#define PictOpAtop              9
#define PictOpAtopReverse       10
#define PictOpXor               11
#define PictOpAdd               12
#define PictOpSaturate          13
#define PictOpMaximum           13



struct sna {
    unsigned flags;
#define SNA_NO_WAIT		0x1
#define SNA_NO_FLIP		0x2
#define SNA_TRIPLE_BUFFER	0x4
#define SNA_TEAR_FREE		0x10
#define SNA_FORCE_SHADOW	0x20

	struct list flush_pixmaps;
	struct list active_pixmaps;



//    int vblank_interval;

//    struct list deferred_free;
//    struct list dirty_pixmaps;
//    struct list active_pixmaps;
//    struct list inactive_clock[2];

    unsigned int tiling;
#define SNA_TILING_DISABLE  0x0
#define SNA_TILING_FB       0x1
#define SNA_TILING_2D       0x2
#define SNA_TILING_ALL     (~0)

	struct pci_device *PciInfo;
	const struct intel_device_info *info;

//    PicturePtr clear;
    struct {
        uint32_t fill_bo;
        uint32_t fill_pixel;
        uint32_t fill_alu;
    } blt_state;
    union {
//        struct gen2_render_state gen2;
        struct gen3_render_state gen3;
        struct gen4_render_state gen4;
        struct gen5_render_state gen5;
        struct gen6_render_state gen6;
		struct gen7_render_state gen7;
    } render_state;


    /* Broken-out options. */
//    OptionInfoPtr Options;

    /* Driver phase/state information */
//    Bool suspended;

    struct kgem kgem;
    struct sna_render render;

#if DEBUG_MEMORY
	struct {
	       int shadow_pixels_allocs;
	       int cpu_bo_allocs;
	       size_t shadow_pixels_bytes;
	       size_t cpu_bo_bytes;
	} debug_memory;
#endif
};

static inline struct sna *
to_sna_from_kgem(struct kgem *kgem)
{
	return container_of(kgem, struct sna, kgem);
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef ALIGN
#define ALIGN(i,m)	(((i) + (m) - 1) & ~((m) - 1))
#endif

#ifndef MIN
#define MIN(a,b)	((a) <= (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)	((a) >= (b) ? (a) : (b))
#endif
#endif /* _SNA_H */
