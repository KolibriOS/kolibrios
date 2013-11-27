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

#include <stdint.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "compiler.h"


#include <memory.h>
#include <malloc.h>
#include <errno.h>
#include <kos32sys.h>

#include "intel_driver.h"
#include "pciaccess.h"

#include <drm.h>
#include <i915_drm.h>

#ifdef HAVE_DRI2_H
#include <dri2.h>
#endif

#if HAVE_UDEV
#include <libudev.h>
#endif

#if 0
#include <xorg-server.h>

#include <xf86Crtc.h>
#if XF86_CRTC_VERSION >= 5
#define HAS_PIXMAP_SHARING 1
#endif

#include <xf86str.h>
#include <windowstr.h>
#include <glyphstr.h>
#include <picturestr.h>
#include <gcstruct.h>
#include <xvdix.h>

#include <pciaccess.h>

#include <xf86drmMode.h>

#include "../compat-api.h"

#endif

#include <assert.h>

#define ErrorF printf

#if HAS_DEBUG_FULL
#define DBG(x) ErrorF x
#else
#define DBG(x)
#endif

#define DEBUG_NO_BLT 0

#define DEBUG_FLUSH_BATCH 0

#define TEST_ALL 0
#define TEST_ACCEL (TEST_ALL || 0)
#define TEST_BATCH (TEST_ALL || 0)
#define TEST_BLT (TEST_ALL || 0)
#define TEST_COMPOSITE (TEST_ALL || 0)
#define TEST_DAMAGE (TEST_ALL || 0)
#define TEST_GRADIENT (TEST_ALL || 0)
#define TEST_GLYPHS (TEST_ALL || 0)
#define TEST_IO (TEST_ALL || 0)
#define TEST_KGEM (TEST_ALL || 0)
#define TEST_RENDER (TEST_ALL || 0)

int drmIoctl(int fd, unsigned long request, void *arg);


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

/* sRGB formats */
    PIXMAN_a8r8g8b8_sRGB = PIXMAN_FORMAT(32,PIXMAN_TYPE_ARGB_SRGB,8,8,8,8),

/* 24bpp formats */
    PIXMAN_r8g8b8 =	 PIXMAN_FORMAT(24,PIXMAN_TYPE_ARGB,0,8,8,8),
    PIXMAN_b8g8r8 =	 PIXMAN_FORMAT(24,PIXMAN_TYPE_ABGR,0,8,8,8),

/* 16bpp formats */
    PIXMAN_r5g6b5 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_ARGB,0,5,6,5),
    PIXMAN_b5g6r5 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_ABGR,0,5,6,5),

    PIXMAN_a1r5g5b5 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_ARGB,1,5,5,5),
    PIXMAN_x1r5g5b5 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_ARGB,0,5,5,5),
    PIXMAN_a1b5g5r5 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_ABGR,1,5,5,5),
    PIXMAN_x1b5g5r5 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_ABGR,0,5,5,5),
    PIXMAN_a4r4g4b4 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_ARGB,4,4,4,4),
    PIXMAN_x4r4g4b4 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_ARGB,0,4,4,4),
    PIXMAN_a4b4g4r4 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_ABGR,4,4,4,4),
    PIXMAN_x4b4g4r4 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_ABGR,0,4,4,4),

/* 8bpp formats */
    PIXMAN_a8 =		 PIXMAN_FORMAT(8,PIXMAN_TYPE_A,8,0,0,0),
    PIXMAN_r3g3b2 =	 PIXMAN_FORMAT(8,PIXMAN_TYPE_ARGB,0,3,3,2),
    PIXMAN_b2g3r3 =	 PIXMAN_FORMAT(8,PIXMAN_TYPE_ABGR,0,3,3,2),
    PIXMAN_a2r2g2b2 =	 PIXMAN_FORMAT(8,PIXMAN_TYPE_ARGB,2,2,2,2),
    PIXMAN_a2b2g2r2 =	 PIXMAN_FORMAT(8,PIXMAN_TYPE_ABGR,2,2,2,2),

    PIXMAN_c8 =		 PIXMAN_FORMAT(8,PIXMAN_TYPE_COLOR,0,0,0,0),
    PIXMAN_g8 =		 PIXMAN_FORMAT(8,PIXMAN_TYPE_GRAY,0,0,0,0),

    PIXMAN_x4a4 =	 PIXMAN_FORMAT(8,PIXMAN_TYPE_A,4,0,0,0),

    PIXMAN_x4c4 =	 PIXMAN_FORMAT(8,PIXMAN_TYPE_COLOR,0,0,0,0),
    PIXMAN_x4g4 =	 PIXMAN_FORMAT(8,PIXMAN_TYPE_GRAY,0,0,0,0),

/* 4bpp formats */
    PIXMAN_a4 =		 PIXMAN_FORMAT(4,PIXMAN_TYPE_A,4,0,0,0),
    PIXMAN_r1g2b1 =	 PIXMAN_FORMAT(4,PIXMAN_TYPE_ARGB,0,1,2,1),
    PIXMAN_b1g2r1 =	 PIXMAN_FORMAT(4,PIXMAN_TYPE_ABGR,0,1,2,1),
    PIXMAN_a1r1g1b1 =	 PIXMAN_FORMAT(4,PIXMAN_TYPE_ARGB,1,1,1,1),
    PIXMAN_a1b1g1r1 =	 PIXMAN_FORMAT(4,PIXMAN_TYPE_ABGR,1,1,1,1),

    PIXMAN_c4 =		 PIXMAN_FORMAT(4,PIXMAN_TYPE_COLOR,0,0,0,0),
    PIXMAN_g4 =		 PIXMAN_FORMAT(4,PIXMAN_TYPE_GRAY,0,0,0,0),

/* 1bpp formats */
    PIXMAN_a1 =		 PIXMAN_FORMAT(1,PIXMAN_TYPE_A,1,0,0,0),

    PIXMAN_g1 =		 PIXMAN_FORMAT(1,PIXMAN_TYPE_GRAY,0,0,0,0),

/* YUV formats */
    PIXMAN_yuy2 =	 PIXMAN_FORMAT(16,PIXMAN_TYPE_YUY2,0,0,0,0),
    PIXMAN_yv12 =	 PIXMAN_FORMAT(12,PIXMAN_TYPE_YV12,0,0,0,0)

} pixman_format_code_t;

typedef enum _PictFormatShort {

    PICT_a2r10g10b10 = PIXMAN_a2r10g10b10,
    PICT_x2r10g10b10 = PIXMAN_x2r10g10b10,
    PICT_a2b10g10r10 = PIXMAN_a2b10g10r10,
    PICT_x2b10g10r10 = PIXMAN_x2b10g10r10,

    PICT_a8r8g8b8 = PIXMAN_a8r8g8b8,
    PICT_x8r8g8b8 = PIXMAN_x8r8g8b8,
    PICT_a8b8g8r8 = PIXMAN_a8b8g8r8,
    PICT_x8b8g8r8 = PIXMAN_x8b8g8r8,
    PICT_b8g8r8a8 = PIXMAN_b8g8r8a8,
    PICT_b8g8r8x8 = PIXMAN_b8g8r8x8,

/* 24bpp formats */
    PICT_r8g8b8 = PIXMAN_r8g8b8,
    PICT_b8g8r8 = PIXMAN_b8g8r8,

/* 16bpp formats */
    PICT_r5g6b5 = PIXMAN_r5g6b5,
    PICT_b5g6r5 = PIXMAN_b5g6r5,

    PICT_a1r5g5b5 = PIXMAN_a1r5g5b5,
    PICT_x1r5g5b5 = PIXMAN_x1r5g5b5,
    PICT_a1b5g5r5 = PIXMAN_a1b5g5r5,
    PICT_x1b5g5r5 = PIXMAN_x1b5g5r5,
    PICT_a4r4g4b4 = PIXMAN_a4r4g4b4,
    PICT_x4r4g4b4 = PIXMAN_x4r4g4b4,
    PICT_a4b4g4r4 = PIXMAN_a4b4g4r4,
    PICT_x4b4g4r4 = PIXMAN_x4b4g4r4,

/* 8bpp formats */
    PICT_a8 = PIXMAN_a8,
    PICT_r3g3b2 = PIXMAN_r3g3b2,
    PICT_b2g3r3 = PIXMAN_b2g3r3,
    PICT_a2r2g2b2 = PIXMAN_a2r2g2b2,
    PICT_a2b2g2r2 = PIXMAN_a2b2g2r2,

    PICT_c8 = PIXMAN_c8,
    PICT_g8 = PIXMAN_g8,

    PICT_x4a4 = PIXMAN_x4a4,

    PICT_x4c4 = PIXMAN_x4c4,
    PICT_x4g4 = PIXMAN_x4g4,

/* 4bpp formats */
    PICT_a4 = PIXMAN_a4,
    PICT_r1g2b1 = PIXMAN_r1g2b1,
    PICT_b1g2r1 = PIXMAN_b1g2r1,
    PICT_a1r1g1b1 = PIXMAN_a1r1g1b1,
    PICT_a1b1g1r1 = PIXMAN_a1b1g1r1,

    PICT_c4 = PIXMAN_c4,
    PICT_g4 = PIXMAN_g4,

/* 1bpp formats */
    PICT_a1 = PIXMAN_a1,

    PICT_g1 = PIXMAN_g1
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


#define SNA_CURSOR_X			64
#define SNA_CURSOR_Y			SNA_CURSOR_X

struct sna_client {
	int is_compositor; /* only 4 bits used */
};


//#define assert(x)


struct sna {
	struct kgem kgem;

	unsigned scrn;

    unsigned flags;
#define SNA_NO_WAIT		0x1
#define SNA_NO_FLIP		0x2
#define SNA_TRIPLE_BUFFER	0x4
#define SNA_TEAR_FREE		0x10
#define SNA_FORCE_SHADOW	0x20
#define SNA_FLUSH_GTT		0x40
#define SNA_IS_HOSTED		0x80
#define SNA_PERFORMANCE		0x100
#define SNA_POWERSAVE		0x200
#define SNA_REPROBE		0x80000000

	unsigned cpu_features;
#define MMX 0x1
#define SSE 0x2
#define SSE2 0x4
#define SSE3 0x8
#define SSSE3 0x10
#define SSE4_1 0x20
#define SSE4_2 0x40
#define AVX 0x80
#define AVX2 0x100

	struct list flush_pixmaps;
	struct list active_pixmaps;





    unsigned int tiling;
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

    struct sna_render render;

#if DEBUG_MEMORY
	struct {
		int pixmap_allocs;
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
static inline bool
_sna_transform_point(const PictTransform *transform,
		     int64_t x, int64_t y, int64_t result[3])
{
	int j;

	for (j = 0; j < 3; j++)
		result[j] = (transform->matrix[j][0] * x +
			     transform->matrix[j][1] * y +
			     transform->matrix[j][2]);

	return result[2] != 0;
}

static inline void
_sna_get_transformed_coordinates(int x, int y,
				 const PictTransform *transform,
				 float *x_out, float *y_out)
{

	int64_t result[3];

	_sna_transform_point(transform, x, y, result);
	*x_out = result[0] / (double)result[2];
	*y_out = result[1] / (double)result[2];
}

static inline void
_sna_get_transformed_scaled(int x, int y,
			    const PictTransform *transform, const float *sf,
			    float *x_out, float *y_out)
{
	*x_out = sf[0] * (transform->matrix[0][0] * x +
			  transform->matrix[0][1] * y +
			  transform->matrix[0][2]);

	*y_out = sf[1] * (transform->matrix[1][0] * x +
			  transform->matrix[1][1] * y +
			  transform->matrix[1][2]);
}

void
sna_get_transformed_coordinates(int x, int y,
				const PictTransform *transform,
				float *x_out, float *y_out);

void
sna_get_transformed_coordinates_3d(int x, int y,
				   const PictTransform *transform,
				   float *x_out, float *y_out, float *z_out);

bool sna_transform_is_affine(const PictTransform *t);
bool sna_transform_is_integer_translation(const PictTransform *t,
					  int16_t *tx, int16_t *ty);
bool sna_transform_is_translation(const PictTransform *t,
				  pixman_fixed_t *tx, pixman_fixed_t *ty);
static inline bool
sna_affine_transform_is_rotation(const PictTransform *t)
{
	assert(sna_transform_is_affine(t));
	return t->matrix[0][1] | t->matrix[1][0];
}

static inline bool
sna_transform_equal(const PictTransform *a, const PictTransform *b)
{
	if (a == b)
		return true;

	if (a == NULL || b == NULL)
		return false;

	return memcmp(a, b, sizeof(*a)) == 0;
}
#endif /* _SNA_H */
