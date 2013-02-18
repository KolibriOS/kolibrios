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


#include "intel_driver.h"
#include "pciaccess.h"

#include "compiler.h"

//#define DBG(x)
//#define DBG(x) ErrorF x

#define assert(x)


typedef struct
{
  unsigned      handle;
  unsigned      io_code;
  void          *input;
  int           inp_size;
  void          *output;
  int           out_size;
}ioctl_t;

#define SRV_GET_INFO            20
#define SRV_GET_PARAM           21
#define SRV_I915_GEM_CREATE     22
#define SRV_DRM_GEM_CLOSE       23
#define SRV_I915_GEM_PIN        24

static int call_service(ioctl_t *io)
{
  int retval;

  asm volatile("int $0x40"
      :"=a"(retval)
      :"a"(68),"b"(17),"c"(io)
      :"memory","cc");

  return retval;
};


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
    PIXMAN_a2b10g10r10 = PIXMAN_FORMAT(32,PIXMAN_TYPE_ABGR,2,10,10,10)

} pixman_format_code_t;


typedef unsigned long   Picture;
typedef unsigned long   PictFormat;

typedef struct _Pixmap  *PixmapPtr;
typedef struct _Picture *PicturePtr;

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
//        struct gen3_render_state gen3;
//        struct gen4_render_state gen4;
//        struct gen5_render_state gen5;
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

static inline int vertex_space(struct sna *sna)
{
    return sna->render.vertex_size - sna->render.vertex_used;
}

static inline void vertex_emit(struct sna *sna, float v)
{
    assert(sna->render.vertex_used < sna->render.vertex_size);
    sna->render.vertices[sna->render.vertex_used++] = v;
}

static inline void vertex_emit_2s(struct sna *sna, int16_t x, int16_t y)
{
    int16_t *v = (int16_t *)&sna->render.vertices[sna->render.vertex_used++];
    assert(sna->render.vertex_used <= sna->render.vertex_size);
    v[0] = x;
    v[1] = y;
}

static inline void batch_emit(struct sna *sna, uint32_t dword)
{
    assert(sna->kgem.mode != KGEM_NONE);
    assert(sna->kgem.nbatch + KGEM_BATCH_RESERVED < sna->kgem.surface);
    sna->kgem.batch[sna->kgem.nbatch++] = dword;
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
