/*
 * Mesa 3-D graphics library
 *
 * Copyright (c) 2011 VMware, Inc.
 * Copyright (c) 2014 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * Color, depth, stencil packing functions.
 * Used to pack basic color, depth and stencil formats to specific
 * hardware formats.
 *
 * There are both per-pixel and per-row packing functions:
 * - The former will be used by swrast to write values to the color, depth,
 *   stencil buffers when drawing points, lines and masked spans.
 * - The later will be used for image-oriented functions like glDrawPixels,
 *   glAccum, and glTexImage.
 */

#include <stdint.h>

#include "format_pack.h"
#include "format_utils.h"
#include "macros.h"
#include "../../gallium/auxiliary/util/u_format_rgb9e5.h"
#include "../../gallium/auxiliary/util/u_format_r11g11b10f.h"
#include "util/format_srgb.h"

#define UNPACK(SRC, OFFSET, BITS) (((SRC) >> (OFFSET)) & MAX_UINT(BITS))
#define PACK(SRC, OFFSET, BITS) (((SRC) & MAX_UINT(BITS)) << (OFFSET))



/* ubyte packing functions */


static inline void
pack_ubyte_a8b8g8r8_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);

      uint32_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_x8b8g8r8_unorm(const GLubyte src[4], void *dst)
{
      
               

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);

      uint32_t d = 0;
                     d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r8g8b8a8_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 8);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);

      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
         d |= PACK(a, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r8g8b8x8_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 8);
      
         
      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_b8g8r8a8_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);

      uint32_t d = 0;
         d |= PACK(b, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(r, 16, 8);
         d |= PACK(a, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_b8g8r8x8_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);
      
         
      uint32_t d = 0;
         d |= PACK(b, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(r, 16, 8);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_a8r8g8b8_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 8);

      uint32_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(r, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(b, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_x8r8g8b8_unorm(const GLubyte src[4], void *dst)
{
      
               

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 8);

      uint32_t d = 0;
                     d |= PACK(r, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(b, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_l16a16_unorm(const GLubyte src[4], void *dst)
{
      

      uint16_t l =
            _mesa_unorm_to_unorm(src[0], 8, 16);
      

      uint16_t a =
            _mesa_unorm_to_unorm(src[3], 8, 16);

      uint32_t d = 0;
         d |= PACK(l, 0, 16);
         d |= PACK(a, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_a16l16_unorm(const GLubyte src[4], void *dst)
{
      

      uint16_t a =
            _mesa_unorm_to_unorm(src[3], 8, 16);
      

      uint16_t l =
            _mesa_unorm_to_unorm(src[0], 8, 16);

      uint32_t d = 0;
         d |= PACK(a, 0, 16);
         d |= PACK(l, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_b5g6r5_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 5);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 6);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 5);

      uint16_t d = 0;
         d |= PACK(b, 0, 5);
         d |= PACK(g, 5, 6);
         d |= PACK(r, 11, 5);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_r5g6b5_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 5);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 6);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 5);

      uint16_t d = 0;
         d |= PACK(r, 0, 5);
         d |= PACK(g, 5, 6);
         d |= PACK(b, 11, 5);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_b4g4r4a4_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 4);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 4);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 4);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 4);

      uint16_t d = 0;
         d |= PACK(b, 0, 4);
         d |= PACK(g, 4, 4);
         d |= PACK(r, 8, 4);
         d |= PACK(a, 12, 4);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_b4g4r4x4_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 4);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 4);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 4);
      
         
      uint16_t d = 0;
         d |= PACK(b, 0, 4);
         d |= PACK(g, 4, 4);
         d |= PACK(r, 8, 4);
                  (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_a4r4g4b4_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 4);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 4);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 4);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 4);

      uint16_t d = 0;
         d |= PACK(a, 0, 4);
         d |= PACK(r, 4, 4);
         d |= PACK(g, 8, 4);
         d |= PACK(b, 12, 4);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_a1b5g5r5_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 1);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 5);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 5);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 5);

      uint16_t d = 0;
         d |= PACK(a, 0, 1);
         d |= PACK(b, 1, 5);
         d |= PACK(g, 6, 5);
         d |= PACK(r, 11, 5);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_b5g5r5a1_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 5);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 5);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 5);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 1);

      uint16_t d = 0;
         d |= PACK(b, 0, 5);
         d |= PACK(g, 5, 5);
         d |= PACK(r, 10, 5);
         d |= PACK(a, 15, 1);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_b5g5r5x1_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 5);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 5);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 5);
      
         
      uint16_t d = 0;
         d |= PACK(b, 0, 5);
         d |= PACK(g, 5, 5);
         d |= PACK(r, 10, 5);
                  (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_a1r5g5b5_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 1);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 5);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 5);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 5);

      uint16_t d = 0;
         d |= PACK(a, 0, 1);
         d |= PACK(r, 1, 5);
         d |= PACK(g, 6, 5);
         d |= PACK(b, 11, 5);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_l8a8_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t l =
            _mesa_unorm_to_unorm(src[0], 8, 8);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);

      uint16_t d = 0;
         d |= PACK(l, 0, 8);
         d |= PACK(a, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_a8l8_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);
      

      uint8_t l =
            _mesa_unorm_to_unorm(src[0], 8, 8);

      uint16_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(l, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_r8g8_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);

      uint16_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_g8r8_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);

      uint16_t d = 0;
         d |= PACK(g, 0, 8);
         d |= PACK(r, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_l4a4_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t l =
            _mesa_unorm_to_unorm(src[0], 8, 4);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 4);

      uint8_t d = 0;
         d |= PACK(l, 0, 4);
         d |= PACK(a, 4, 4);
      (*(uint8_t *)dst) = d;
}

static inline void
pack_ubyte_b2g3r3_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 2);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 3);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 3);

      uint8_t d = 0;
         d |= PACK(b, 0, 2);
         d |= PACK(g, 2, 3);
         d |= PACK(r, 5, 3);
      (*(uint8_t *)dst) = d;
}

static inline void
pack_ubyte_r16g16_unorm(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 16);
      

      uint16_t g =
            _mesa_unorm_to_unorm(src[1], 8, 16);

      uint32_t d = 0;
         d |= PACK(r, 0, 16);
         d |= PACK(g, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_g16r16_unorm(const GLubyte src[4], void *dst)
{
      

      uint16_t g =
            _mesa_unorm_to_unorm(src[1], 8, 16);
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 16);

      uint32_t d = 0;
         d |= PACK(g, 0, 16);
         d |= PACK(r, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_b10g10r10a2_unorm(const GLubyte src[4], void *dst)
{
      

      uint16_t b =
            _mesa_unorm_to_unorm(src[2], 8, 10);
      

      uint16_t g =
            _mesa_unorm_to_unorm(src[1], 8, 10);
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 10);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 2);

      uint32_t d = 0;
         d |= PACK(b, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(r, 20, 10);
         d |= PACK(a, 30, 2);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_b10g10r10x2_unorm(const GLubyte src[4], void *dst)
{
      

      uint16_t b =
            _mesa_unorm_to_unorm(src[2], 8, 10);
      

      uint16_t g =
            _mesa_unorm_to_unorm(src[1], 8, 10);
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 10);
      
         
      uint32_t d = 0;
         d |= PACK(b, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(r, 20, 10);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r10g10b10a2_unorm(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 10);
      

      uint16_t g =
            _mesa_unorm_to_unorm(src[1], 8, 10);
      

      uint16_t b =
            _mesa_unorm_to_unorm(src[2], 8, 10);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 2);

      uint32_t d = 0;
         d |= PACK(r, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(b, 20, 10);
         d |= PACK(a, 30, 2);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r10g10b10x2_unorm(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 10);
      

      uint16_t g =
            _mesa_unorm_to_unorm(src[1], 8, 10);
      

      uint16_t b =
            _mesa_unorm_to_unorm(src[2], 8, 10);
      
         
      uint32_t d = 0;
         d |= PACK(r, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(b, 20, 10);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r3g3b2_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 3);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 3);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 2);

      uint8_t d = 0;
         d |= PACK(r, 0, 3);
         d |= PACK(g, 3, 3);
         d |= PACK(b, 6, 2);
      (*(uint8_t *)dst) = d;
}

static inline void
pack_ubyte_a4b4g4r4_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 4);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 4);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 4);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 4);

      uint16_t d = 0;
         d |= PACK(a, 0, 4);
         d |= PACK(b, 4, 4);
         d |= PACK(g, 8, 4);
         d |= PACK(r, 12, 4);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_r4g4b4a4_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 4);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 4);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 4);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 4);

      uint16_t d = 0;
         d |= PACK(r, 0, 4);
         d |= PACK(g, 4, 4);
         d |= PACK(b, 8, 4);
         d |= PACK(a, 12, 4);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_r5g5b5a1_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 5);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 5);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 5);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 1);

      uint16_t d = 0;
         d |= PACK(r, 0, 5);
         d |= PACK(g, 5, 5);
         d |= PACK(b, 10, 5);
         d |= PACK(a, 15, 1);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_a2b10g10r10_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 2);
      

      uint16_t b =
            _mesa_unorm_to_unorm(src[2], 8, 10);
      

      uint16_t g =
            _mesa_unorm_to_unorm(src[1], 8, 10);
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 10);

      uint32_t d = 0;
         d |= PACK(a, 0, 2);
         d |= PACK(b, 2, 10);
         d |= PACK(g, 12, 10);
         d |= PACK(r, 22, 10);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_a2r10g10b10_unorm(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 2);
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 10);
      

      uint16_t g =
            _mesa_unorm_to_unorm(src[1], 8, 10);
      

      uint16_t b =
            _mesa_unorm_to_unorm(src[2], 8, 10);

      uint32_t d = 0;
         d |= PACK(a, 0, 2);
         d |= PACK(r, 2, 10);
         d |= PACK(g, 12, 10);
         d |= PACK(b, 22, 10);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_a_unorm8(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_a_unorm16(const GLubyte src[4], void *dst)
{
      

      uint16_t a =
            _mesa_unorm_to_unorm(src[3], 8, 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_l_unorm8(const GLubyte src[4], void *dst)
{
      

      uint8_t l =
            _mesa_unorm_to_unorm(src[0], 8, 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_l_unorm16(const GLubyte src[4], void *dst)
{
      

      uint16_t l =
            _mesa_unorm_to_unorm(src[0], 8, 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_i_unorm8(const GLubyte src[4], void *dst)
{
      

      uint8_t i =
            _mesa_unorm_to_unorm(src[0], 8, 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_i_unorm16(const GLubyte src[4], void *dst)
{
      

      uint16_t i =
            _mesa_unorm_to_unorm(src[0], 8, 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_r_unorm8(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_r_unorm16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_bgr_unorm8(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = b;
         d[1] = g;
         d[2] = r;
}

static inline void
pack_ubyte_rgb_unorm8(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            _mesa_unorm_to_unorm(src[0], 8, 8);
      

      uint8_t g =
            _mesa_unorm_to_unorm(src[1], 8, 8);
      

      uint8_t b =
            _mesa_unorm_to_unorm(src[2], 8, 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_ubyte_rgba_unorm16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 16);
      

      uint16_t g =
            _mesa_unorm_to_unorm(src[1], 8, 16);
      

      uint16_t b =
            _mesa_unorm_to_unorm(src[2], 8, 16);
      

      uint16_t a =
            _mesa_unorm_to_unorm(src[3], 8, 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_ubyte_rgbx_unorm16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_unorm(src[0], 8, 16);
      

      uint16_t g =
            _mesa_unorm_to_unorm(src[1], 8, 16);
      

      uint16_t b =
            _mesa_unorm_to_unorm(src[2], 8, 16);
      
         
      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_ubyte_a8b8g8r8_snorm(const GLubyte src[4], void *dst)
{
      

      int8_t a =
         _mesa_unorm_to_snorm(src[3], 8, 8);
      

      int8_t b =
         _mesa_unorm_to_snorm(src[2], 8, 8);
      

      int8_t g =
         _mesa_unorm_to_snorm(src[1], 8, 8);
      

      int8_t r =
         _mesa_unorm_to_snorm(src[0], 8, 8);

      uint32_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_x8b8g8r8_snorm(const GLubyte src[4], void *dst)
{
      
               

      int8_t b =
         _mesa_unorm_to_snorm(src[2], 8, 8);
      

      int8_t g =
         _mesa_unorm_to_snorm(src[1], 8, 8);
      

      int8_t r =
         _mesa_unorm_to_snorm(src[0], 8, 8);

      uint32_t d = 0;
                     d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r8g8b8a8_snorm(const GLubyte src[4], void *dst)
{
      

      int8_t r =
         _mesa_unorm_to_snorm(src[0], 8, 8);
      

      int8_t g =
         _mesa_unorm_to_snorm(src[1], 8, 8);
      

      int8_t b =
         _mesa_unorm_to_snorm(src[2], 8, 8);
      

      int8_t a =
         _mesa_unorm_to_snorm(src[3], 8, 8);

      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
         d |= PACK(a, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r8g8b8x8_snorm(const GLubyte src[4], void *dst)
{
      

      int8_t r =
         _mesa_unorm_to_snorm(src[0], 8, 8);
      

      int8_t g =
         _mesa_unorm_to_snorm(src[1], 8, 8);
      

      int8_t b =
         _mesa_unorm_to_snorm(src[2], 8, 8);
      
         
      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r16g16_snorm(const GLubyte src[4], void *dst)
{
      

      int16_t r =
         _mesa_unorm_to_snorm(src[0], 8, 16);
      

      int16_t g =
         _mesa_unorm_to_snorm(src[1], 8, 16);

      uint32_t d = 0;
         d |= PACK(r, 0, 16);
         d |= PACK(g, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_g16r16_snorm(const GLubyte src[4], void *dst)
{
      

      int16_t g =
         _mesa_unorm_to_snorm(src[1], 8, 16);
      

      int16_t r =
         _mesa_unorm_to_snorm(src[0], 8, 16);

      uint32_t d = 0;
         d |= PACK(g, 0, 16);
         d |= PACK(r, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r8g8_snorm(const GLubyte src[4], void *dst)
{
      

      int8_t r =
         _mesa_unorm_to_snorm(src[0], 8, 8);
      

      int8_t g =
         _mesa_unorm_to_snorm(src[1], 8, 8);

      uint16_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_g8r8_snorm(const GLubyte src[4], void *dst)
{
      

      int8_t g =
         _mesa_unorm_to_snorm(src[1], 8, 8);
      

      int8_t r =
         _mesa_unorm_to_snorm(src[0], 8, 8);

      uint16_t d = 0;
         d |= PACK(g, 0, 8);
         d |= PACK(r, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_l8a8_snorm(const GLubyte src[4], void *dst)
{
      

      int8_t l =
         _mesa_unorm_to_snorm(src[0], 8, 8);
      

      int8_t a =
         _mesa_unorm_to_snorm(src[3], 8, 8);

      uint16_t d = 0;
         d |= PACK(l, 0, 8);
         d |= PACK(a, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_a8l8_snorm(const GLubyte src[4], void *dst)
{
      

      int8_t a =
         _mesa_unorm_to_snorm(src[3], 8, 8);
      

      int8_t l =
         _mesa_unorm_to_snorm(src[0], 8, 8);

      uint16_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(l, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_a_snorm8(const GLubyte src[4], void *dst)
{
      

      int8_t a =
         _mesa_unorm_to_snorm(src[3], 8, 8);

      int8_t *d = (int8_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_a_snorm16(const GLubyte src[4], void *dst)
{
      

      int16_t a =
         _mesa_unorm_to_snorm(src[3], 8, 16);

      int16_t *d = (int16_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_l_snorm8(const GLubyte src[4], void *dst)
{
      

      int8_t l =
         _mesa_unorm_to_snorm(src[0], 8, 8);

      int8_t *d = (int8_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_l_snorm16(const GLubyte src[4], void *dst)
{
      

      int16_t l =
         _mesa_unorm_to_snorm(src[0], 8, 16);

      int16_t *d = (int16_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_i_snorm8(const GLubyte src[4], void *dst)
{
      

      int8_t i =
         _mesa_unorm_to_snorm(src[0], 8, 8);

      int8_t *d = (int8_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_i_snorm16(const GLubyte src[4], void *dst)
{
      

      int16_t i =
         _mesa_unorm_to_snorm(src[0], 8, 16);

      int16_t *d = (int16_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_r_snorm8(const GLubyte src[4], void *dst)
{
      

      int8_t r =
         _mesa_unorm_to_snorm(src[0], 8, 8);

      int8_t *d = (int8_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_r_snorm16(const GLubyte src[4], void *dst)
{
      

      int16_t r =
         _mesa_unorm_to_snorm(src[0], 8, 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_la_snorm16(const GLubyte src[4], void *dst)
{
      

      int16_t l =
         _mesa_unorm_to_snorm(src[0], 8, 16);
      

      int16_t a =
         _mesa_unorm_to_snorm(src[3], 8, 16);

      int16_t *d = (int16_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_ubyte_rgb_snorm16(const GLubyte src[4], void *dst)
{
      

      int16_t r =
         _mesa_unorm_to_snorm(src[0], 8, 16);
      

      int16_t g =
         _mesa_unorm_to_snorm(src[1], 8, 16);
      

      int16_t b =
         _mesa_unorm_to_snorm(src[2], 8, 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_ubyte_rgba_snorm16(const GLubyte src[4], void *dst)
{
      

      int16_t r =
         _mesa_unorm_to_snorm(src[0], 8, 16);
      

      int16_t g =
         _mesa_unorm_to_snorm(src[1], 8, 16);
      

      int16_t b =
         _mesa_unorm_to_snorm(src[2], 8, 16);
      

      int16_t a =
         _mesa_unorm_to_snorm(src[3], 8, 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_ubyte_rgbx_snorm16(const GLubyte src[4], void *dst)
{
      

      int16_t r =
         _mesa_unorm_to_snorm(src[0], 8, 16);
      

      int16_t g =
         _mesa_unorm_to_snorm(src[1], 8, 16);
      

      int16_t b =
         _mesa_unorm_to_snorm(src[2], 8, 16);
      
         
      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_ubyte_a8b8g8r8_srgb(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);
      

      uint8_t b =
            
            util_format_linear_to_srgb_8unorm(src[2]);
      

      uint8_t g =
            
            util_format_linear_to_srgb_8unorm(src[1]);
      

      uint8_t r =
            
            util_format_linear_to_srgb_8unorm(src[0]);

      uint32_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_b8g8r8a8_srgb(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            
            util_format_linear_to_srgb_8unorm(src[2]);
      

      uint8_t g =
            
            util_format_linear_to_srgb_8unorm(src[1]);
      

      uint8_t r =
            
            util_format_linear_to_srgb_8unorm(src[0]);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);

      uint32_t d = 0;
         d |= PACK(b, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(r, 16, 8);
         d |= PACK(a, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_a8r8g8b8_srgb(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);
      

      uint8_t r =
            
            util_format_linear_to_srgb_8unorm(src[0]);
      

      uint8_t g =
            
            util_format_linear_to_srgb_8unorm(src[1]);
      

      uint8_t b =
            
            util_format_linear_to_srgb_8unorm(src[2]);

      uint32_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(r, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(b, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_b8g8r8x8_srgb(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            
            util_format_linear_to_srgb_8unorm(src[2]);
      

      uint8_t g =
            
            util_format_linear_to_srgb_8unorm(src[1]);
      

      uint8_t r =
            
            util_format_linear_to_srgb_8unorm(src[0]);
      
         
      uint32_t d = 0;
         d |= PACK(b, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(r, 16, 8);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_x8r8g8b8_srgb(const GLubyte src[4], void *dst)
{
      
               

      uint8_t r =
            
            util_format_linear_to_srgb_8unorm(src[0]);
      

      uint8_t g =
            
            util_format_linear_to_srgb_8unorm(src[1]);
      

      uint8_t b =
            
            util_format_linear_to_srgb_8unorm(src[2]);

      uint32_t d = 0;
                     d |= PACK(r, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(b, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r8g8b8a8_srgb(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            
            util_format_linear_to_srgb_8unorm(src[0]);
      

      uint8_t g =
            
            util_format_linear_to_srgb_8unorm(src[1]);
      

      uint8_t b =
            
            util_format_linear_to_srgb_8unorm(src[2]);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);

      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
         d |= PACK(a, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r8g8b8x8_srgb(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
            
            util_format_linear_to_srgb_8unorm(src[0]);
      

      uint8_t g =
            
            util_format_linear_to_srgb_8unorm(src[1]);
      

      uint8_t b =
            
            util_format_linear_to_srgb_8unorm(src[2]);
      
         
      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_x8b8g8r8_srgb(const GLubyte src[4], void *dst)
{
      
               

      uint8_t b =
            
            util_format_linear_to_srgb_8unorm(src[2]);
      

      uint8_t g =
            
            util_format_linear_to_srgb_8unorm(src[1]);
      

      uint8_t r =
            
            util_format_linear_to_srgb_8unorm(src[0]);

      uint32_t d = 0;
                     d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_l8a8_srgb(const GLubyte src[4], void *dst)
{
      

      uint8_t l =
            _mesa_unorm_to_unorm(src[0], 8, 8);
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);

      uint16_t d = 0;
         d |= PACK(l, 0, 8);
         d |= PACK(a, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_a8l8_srgb(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
            _mesa_unorm_to_unorm(src[3], 8, 8);
      

      uint8_t l =
            _mesa_unorm_to_unorm(src[0], 8, 8);

      uint16_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(l, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_ubyte_l_srgb8(const GLubyte src[4], void *dst)
{
      

      uint8_t l =
            _mesa_unorm_to_unorm(src[0], 8, 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_bgr_srgb8(const GLubyte src[4], void *dst)
{
      

      uint8_t b =
            
            util_format_linear_to_srgb_8unorm(src[2]);
      

      uint8_t g =
            
            util_format_linear_to_srgb_8unorm(src[1]);
      

      uint8_t r =
            
            util_format_linear_to_srgb_8unorm(src[0]);

      uint8_t *d = (uint8_t *)dst;
         d[0] = b;
         d[1] = g;
         d[2] = r;
}
            
static inline void
pack_ubyte_a_float16(const GLubyte src[4], void *dst)
{
      

      uint16_t a =
            _mesa_unorm_to_half(src[3], 8);

      uint16_t *d = (uint16_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_a_float32(const GLubyte src[4], void *dst)
{
      

      float a =
            _mesa_unorm_to_float(src[3], 8);

      float *d = (float *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_l_float16(const GLubyte src[4], void *dst)
{
      

      uint16_t l =
            _mesa_unorm_to_half(src[0], 8);

      uint16_t *d = (uint16_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_l_float32(const GLubyte src[4], void *dst)
{
      

      float l =
            _mesa_unorm_to_float(src[0], 8);

      float *d = (float *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_la_float16(const GLubyte src[4], void *dst)
{
      

      uint16_t l =
            _mesa_unorm_to_half(src[0], 8);
      

      uint16_t a =
            _mesa_unorm_to_half(src[3], 8);

      uint16_t *d = (uint16_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_ubyte_la_float32(const GLubyte src[4], void *dst)
{
      

      float l =
            _mesa_unorm_to_float(src[0], 8);
      

      float a =
            _mesa_unorm_to_float(src[3], 8);

      float *d = (float *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_ubyte_i_float16(const GLubyte src[4], void *dst)
{
      

      uint16_t i =
            _mesa_unorm_to_half(src[0], 8);

      uint16_t *d = (uint16_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_i_float32(const GLubyte src[4], void *dst)
{
      

      float i =
            _mesa_unorm_to_float(src[0], 8);

      float *d = (float *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_r_float16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_half(src[0], 8);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_r_float32(const GLubyte src[4], void *dst)
{
      

      float r =
            _mesa_unorm_to_float(src[0], 8);

      float *d = (float *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_rg_float16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_half(src[0], 8);
      

      uint16_t g =
            _mesa_unorm_to_half(src[1], 8);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_ubyte_rg_float32(const GLubyte src[4], void *dst)
{
      

      float r =
            _mesa_unorm_to_float(src[0], 8);
      

      float g =
            _mesa_unorm_to_float(src[1], 8);

      float *d = (float *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_ubyte_rgb_float16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_half(src[0], 8);
      

      uint16_t g =
            _mesa_unorm_to_half(src[1], 8);
      

      uint16_t b =
            _mesa_unorm_to_half(src[2], 8);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_ubyte_rgb_float32(const GLubyte src[4], void *dst)
{
      

      float r =
            _mesa_unorm_to_float(src[0], 8);
      

      float g =
            _mesa_unorm_to_float(src[1], 8);
      

      float b =
            _mesa_unorm_to_float(src[2], 8);

      float *d = (float *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_ubyte_rgba_float16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_half(src[0], 8);
      

      uint16_t g =
            _mesa_unorm_to_half(src[1], 8);
      

      uint16_t b =
            _mesa_unorm_to_half(src[2], 8);
      

      uint16_t a =
            _mesa_unorm_to_half(src[3], 8);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_ubyte_rgba_float32(const GLubyte src[4], void *dst)
{
      

      float r =
            _mesa_unorm_to_float(src[0], 8);
      

      float g =
            _mesa_unorm_to_float(src[1], 8);
      

      float b =
            _mesa_unorm_to_float(src[2], 8);
      

      float a =
            _mesa_unorm_to_float(src[3], 8);

      float *d = (float *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_ubyte_rgbx_float16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
            _mesa_unorm_to_half(src[0], 8);
      

      uint16_t g =
            _mesa_unorm_to_half(src[1], 8);
      

      uint16_t b =
            _mesa_unorm_to_half(src[2], 8);
      
         
      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_ubyte_rgbx_float32(const GLubyte src[4], void *dst)
{
      

      float r =
            _mesa_unorm_to_float(src[0], 8);
      

      float g =
            _mesa_unorm_to_float(src[1], 8);
      

      float b =
            _mesa_unorm_to_float(src[2], 8);
      
         
      float *d = (float *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_ubyte_b10g10r10a2_uint(const GLubyte src[4], void *dst)
{
      

      uint16_t b =
              _mesa_unsigned_to_unsigned(src[2], 10);
      

      uint16_t g =
              _mesa_unsigned_to_unsigned(src[1], 10);
      

      uint16_t r =
              _mesa_unsigned_to_unsigned(src[0], 10);
      

      uint8_t a =
              _mesa_unsigned_to_unsigned(src[3], 2);

      uint32_t d = 0;
         d |= PACK(b, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(r, 20, 10);
         d |= PACK(a, 30, 2);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_r10g10b10a2_uint(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
              _mesa_unsigned_to_unsigned(src[0], 10);
      

      uint16_t g =
              _mesa_unsigned_to_unsigned(src[1], 10);
      

      uint16_t b =
              _mesa_unsigned_to_unsigned(src[2], 10);
      

      uint8_t a =
              _mesa_unsigned_to_unsigned(src[3], 2);

      uint32_t d = 0;
         d |= PACK(r, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(b, 20, 10);
         d |= PACK(a, 30, 2);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_a2b10g10r10_uint(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
              _mesa_unsigned_to_unsigned(src[3], 2);
      

      uint16_t b =
              _mesa_unsigned_to_unsigned(src[2], 10);
      

      uint16_t g =
              _mesa_unsigned_to_unsigned(src[1], 10);
      

      uint16_t r =
              _mesa_unsigned_to_unsigned(src[0], 10);

      uint32_t d = 0;
         d |= PACK(a, 0, 2);
         d |= PACK(b, 2, 10);
         d |= PACK(g, 12, 10);
         d |= PACK(r, 22, 10);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_a2r10g10b10_uint(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
              _mesa_unsigned_to_unsigned(src[3], 2);
      

      uint16_t r =
              _mesa_unsigned_to_unsigned(src[0], 10);
      

      uint16_t g =
              _mesa_unsigned_to_unsigned(src[1], 10);
      

      uint16_t b =
              _mesa_unsigned_to_unsigned(src[2], 10);

      uint32_t d = 0;
         d |= PACK(a, 0, 2);
         d |= PACK(r, 2, 10);
         d |= PACK(g, 12, 10);
         d |= PACK(b, 22, 10);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_ubyte_a_uint8(const GLubyte src[4], void *dst)
{
      

      uint8_t a =
              _mesa_unsigned_to_unsigned(src[3], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_a_uint16(const GLubyte src[4], void *dst)
{
      

      uint16_t a =
              _mesa_unsigned_to_unsigned(src[3], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_a_uint32(const GLubyte src[4], void *dst)
{
      

      uint32_t a =
              _mesa_unsigned_to_unsigned(src[3], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_a_sint8(const GLubyte src[4], void *dst)
{
      

      int8_t a =
              _mesa_unsigned_to_signed(src[3], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_a_sint16(const GLubyte src[4], void *dst)
{
      

      int16_t a =
              _mesa_unsigned_to_signed(src[3], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_a_sint32(const GLubyte src[4], void *dst)
{
      

      int32_t a =
              _mesa_unsigned_to_signed(src[3], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = a;
}

static inline void
pack_ubyte_i_uint8(const GLubyte src[4], void *dst)
{
      

      uint8_t i =
              _mesa_unsigned_to_unsigned(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_i_uint16(const GLubyte src[4], void *dst)
{
      

      uint16_t i =
              _mesa_unsigned_to_unsigned(src[0], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_i_uint32(const GLubyte src[4], void *dst)
{
      

      uint32_t i =
              _mesa_unsigned_to_unsigned(src[0], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_i_sint8(const GLubyte src[4], void *dst)
{
      

      int8_t i =
              _mesa_unsigned_to_signed(src[0], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_i_sint16(const GLubyte src[4], void *dst)
{
      

      int16_t i =
              _mesa_unsigned_to_signed(src[0], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_i_sint32(const GLubyte src[4], void *dst)
{
      

      int32_t i =
              _mesa_unsigned_to_signed(src[0], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = i;
}

static inline void
pack_ubyte_l_uint8(const GLubyte src[4], void *dst)
{
      

      uint8_t l =
              _mesa_unsigned_to_unsigned(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_l_uint16(const GLubyte src[4], void *dst)
{
      

      uint16_t l =
              _mesa_unsigned_to_unsigned(src[0], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_l_uint32(const GLubyte src[4], void *dst)
{
      

      uint32_t l =
              _mesa_unsigned_to_unsigned(src[0], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_l_sint8(const GLubyte src[4], void *dst)
{
      

      int8_t l =
              _mesa_unsigned_to_signed(src[0], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_l_sint16(const GLubyte src[4], void *dst)
{
      

      int16_t l =
              _mesa_unsigned_to_signed(src[0], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_l_sint32(const GLubyte src[4], void *dst)
{
      

      int32_t l =
              _mesa_unsigned_to_signed(src[0], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = l;
}

static inline void
pack_ubyte_la_uint8(const GLubyte src[4], void *dst)
{
      

      uint8_t l =
              _mesa_unsigned_to_unsigned(src[0], 8);
      

      uint8_t a =
              _mesa_unsigned_to_unsigned(src[3], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_ubyte_la_uint16(const GLubyte src[4], void *dst)
{
      

      uint16_t l =
              _mesa_unsigned_to_unsigned(src[0], 16);
      

      uint16_t a =
              _mesa_unsigned_to_unsigned(src[3], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_ubyte_la_uint32(const GLubyte src[4], void *dst)
{
      

      uint32_t l =
              _mesa_unsigned_to_unsigned(src[0], 32);
      

      uint32_t a =
              _mesa_unsigned_to_unsigned(src[3], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_ubyte_la_sint8(const GLubyte src[4], void *dst)
{
      

      int8_t l =
              _mesa_unsigned_to_signed(src[0], 8);
      

      int8_t a =
              _mesa_unsigned_to_signed(src[3], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_ubyte_la_sint16(const GLubyte src[4], void *dst)
{
      

      int16_t l =
              _mesa_unsigned_to_signed(src[0], 16);
      

      int16_t a =
              _mesa_unsigned_to_signed(src[3], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_ubyte_la_sint32(const GLubyte src[4], void *dst)
{
      

      int32_t l =
              _mesa_unsigned_to_signed(src[0], 32);
      

      int32_t a =
              _mesa_unsigned_to_signed(src[3], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_ubyte_r_uint8(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
              _mesa_unsigned_to_unsigned(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_r_uint16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
              _mesa_unsigned_to_unsigned(src[0], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_r_uint32(const GLubyte src[4], void *dst)
{
      

      uint32_t r =
              _mesa_unsigned_to_unsigned(src[0], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_r_sint8(const GLubyte src[4], void *dst)
{
      

      int8_t r =
              _mesa_unsigned_to_signed(src[0], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_r_sint16(const GLubyte src[4], void *dst)
{
      

      int16_t r =
              _mesa_unsigned_to_signed(src[0], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_r_sint32(const GLubyte src[4], void *dst)
{
      

      int32_t r =
              _mesa_unsigned_to_signed(src[0], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = r;
}

static inline void
pack_ubyte_rg_uint8(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
              _mesa_unsigned_to_unsigned(src[0], 8);
      

      uint8_t g =
              _mesa_unsigned_to_unsigned(src[1], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_ubyte_rg_uint16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
              _mesa_unsigned_to_unsigned(src[0], 16);
      

      uint16_t g =
              _mesa_unsigned_to_unsigned(src[1], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_ubyte_rg_uint32(const GLubyte src[4], void *dst)
{
      

      uint32_t r =
              _mesa_unsigned_to_unsigned(src[0], 32);
      

      uint32_t g =
              _mesa_unsigned_to_unsigned(src[1], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_ubyte_rg_sint8(const GLubyte src[4], void *dst)
{
      

      int8_t r =
              _mesa_unsigned_to_signed(src[0], 8);
      

      int8_t g =
              _mesa_unsigned_to_signed(src[1], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_ubyte_rg_sint16(const GLubyte src[4], void *dst)
{
      

      int16_t r =
              _mesa_unsigned_to_signed(src[0], 16);
      

      int16_t g =
              _mesa_unsigned_to_signed(src[1], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_ubyte_rg_sint32(const GLubyte src[4], void *dst)
{
      

      int32_t r =
              _mesa_unsigned_to_signed(src[0], 32);
      

      int32_t g =
              _mesa_unsigned_to_signed(src[1], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_ubyte_rgb_uint8(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
              _mesa_unsigned_to_unsigned(src[0], 8);
      

      uint8_t g =
              _mesa_unsigned_to_unsigned(src[1], 8);
      

      uint8_t b =
              _mesa_unsigned_to_unsigned(src[2], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_ubyte_rgb_uint16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
              _mesa_unsigned_to_unsigned(src[0], 16);
      

      uint16_t g =
              _mesa_unsigned_to_unsigned(src[1], 16);
      

      uint16_t b =
              _mesa_unsigned_to_unsigned(src[2], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_ubyte_rgb_uint32(const GLubyte src[4], void *dst)
{
      

      uint32_t r =
              _mesa_unsigned_to_unsigned(src[0], 32);
      

      uint32_t g =
              _mesa_unsigned_to_unsigned(src[1], 32);
      

      uint32_t b =
              _mesa_unsigned_to_unsigned(src[2], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_ubyte_rgb_sint8(const GLubyte src[4], void *dst)
{
      

      int8_t r =
              _mesa_unsigned_to_signed(src[0], 8);
      

      int8_t g =
              _mesa_unsigned_to_signed(src[1], 8);
      

      int8_t b =
              _mesa_unsigned_to_signed(src[2], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_ubyte_rgb_sint16(const GLubyte src[4], void *dst)
{
      

      int16_t r =
              _mesa_unsigned_to_signed(src[0], 16);
      

      int16_t g =
              _mesa_unsigned_to_signed(src[1], 16);
      

      int16_t b =
              _mesa_unsigned_to_signed(src[2], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_ubyte_rgb_sint32(const GLubyte src[4], void *dst)
{
      

      int32_t r =
              _mesa_unsigned_to_signed(src[0], 32);
      

      int32_t g =
              _mesa_unsigned_to_signed(src[1], 32);
      

      int32_t b =
              _mesa_unsigned_to_signed(src[2], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_ubyte_rgba_uint8(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
              _mesa_unsigned_to_unsigned(src[0], 8);
      

      uint8_t g =
              _mesa_unsigned_to_unsigned(src[1], 8);
      

      uint8_t b =
              _mesa_unsigned_to_unsigned(src[2], 8);
      

      uint8_t a =
              _mesa_unsigned_to_unsigned(src[3], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_ubyte_rgba_uint16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
              _mesa_unsigned_to_unsigned(src[0], 16);
      

      uint16_t g =
              _mesa_unsigned_to_unsigned(src[1], 16);
      

      uint16_t b =
              _mesa_unsigned_to_unsigned(src[2], 16);
      

      uint16_t a =
              _mesa_unsigned_to_unsigned(src[3], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_ubyte_rgba_uint32(const GLubyte src[4], void *dst)
{
      

      uint32_t r =
              _mesa_unsigned_to_unsigned(src[0], 32);
      

      uint32_t g =
              _mesa_unsigned_to_unsigned(src[1], 32);
      

      uint32_t b =
              _mesa_unsigned_to_unsigned(src[2], 32);
      

      uint32_t a =
              _mesa_unsigned_to_unsigned(src[3], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_ubyte_rgba_sint8(const GLubyte src[4], void *dst)
{
      

      int8_t r =
              _mesa_unsigned_to_signed(src[0], 8);
      

      int8_t g =
              _mesa_unsigned_to_signed(src[1], 8);
      

      int8_t b =
              _mesa_unsigned_to_signed(src[2], 8);
      

      int8_t a =
              _mesa_unsigned_to_signed(src[3], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_ubyte_rgba_sint16(const GLubyte src[4], void *dst)
{
      

      int16_t r =
              _mesa_unsigned_to_signed(src[0], 16);
      

      int16_t g =
              _mesa_unsigned_to_signed(src[1], 16);
      

      int16_t b =
              _mesa_unsigned_to_signed(src[2], 16);
      

      int16_t a =
              _mesa_unsigned_to_signed(src[3], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_ubyte_rgba_sint32(const GLubyte src[4], void *dst)
{
      

      int32_t r =
              _mesa_unsigned_to_signed(src[0], 32);
      

      int32_t g =
              _mesa_unsigned_to_signed(src[1], 32);
      

      int32_t b =
              _mesa_unsigned_to_signed(src[2], 32);
      

      int32_t a =
              _mesa_unsigned_to_signed(src[3], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_ubyte_rgbx_uint8(const GLubyte src[4], void *dst)
{
      

      uint8_t r =
              _mesa_unsigned_to_unsigned(src[0], 8);
      

      uint8_t g =
              _mesa_unsigned_to_unsigned(src[1], 8);
      

      uint8_t b =
              _mesa_unsigned_to_unsigned(src[2], 8);
      
         
      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_ubyte_rgbx_uint16(const GLubyte src[4], void *dst)
{
      

      uint16_t r =
              _mesa_unsigned_to_unsigned(src[0], 16);
      

      uint16_t g =
              _mesa_unsigned_to_unsigned(src[1], 16);
      

      uint16_t b =
              _mesa_unsigned_to_unsigned(src[2], 16);
      
         
      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_ubyte_rgbx_uint32(const GLubyte src[4], void *dst)
{
      

      uint32_t r =
              _mesa_unsigned_to_unsigned(src[0], 32);
      

      uint32_t g =
              _mesa_unsigned_to_unsigned(src[1], 32);
      

      uint32_t b =
              _mesa_unsigned_to_unsigned(src[2], 32);
      
         
      uint32_t *d = (uint32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_ubyte_rgbx_sint8(const GLubyte src[4], void *dst)
{
      

      int8_t r =
              _mesa_unsigned_to_signed(src[0], 8);
      

      int8_t g =
              _mesa_unsigned_to_signed(src[1], 8);
      

      int8_t b =
              _mesa_unsigned_to_signed(src[2], 8);
      
         
      int8_t *d = (int8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_ubyte_rgbx_sint16(const GLubyte src[4], void *dst)
{
      

      int16_t r =
              _mesa_unsigned_to_signed(src[0], 16);
      

      int16_t g =
              _mesa_unsigned_to_signed(src[1], 16);
      

      int16_t b =
              _mesa_unsigned_to_signed(src[2], 16);
      
         
      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_ubyte_rgbx_sint32(const GLubyte src[4], void *dst)
{
      

      int32_t r =
              _mesa_unsigned_to_signed(src[0], 32);
      

      int32_t g =
              _mesa_unsigned_to_signed(src[1], 32);
      

      int32_t b =
              _mesa_unsigned_to_signed(src[2], 32);
      
         
      int32_t *d = (int32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }
                                                                                                                                                                                                      
static inline void
pack_ubyte_r9g9b9e5_float(const GLubyte src[4], void *dst)
{
   GLuint *d = (GLuint *) dst;
   GLfloat rgb[3];
   rgb[0] = _mesa_unorm_to_float(src[RCOMP], 8);
   rgb[1] = _mesa_unorm_to_float(src[GCOMP], 8);
   rgb[2] = _mesa_unorm_to_float(src[BCOMP], 8);
   *d = float3_to_rgb9e5(rgb);
}

static inline void
pack_ubyte_r11g11b10_float(const GLubyte src[4], void *dst)
{
   GLuint *d = (GLuint *) dst;
   GLfloat rgb[3];
   rgb[0] = _mesa_unorm_to_float(src[RCOMP], 8);
   rgb[1] = _mesa_unorm_to_float(src[GCOMP], 8);
   rgb[2] = _mesa_unorm_to_float(src[BCOMP], 8);
   *d = float3_to_r11g11b10f(rgb);
}

/* uint packing functions */

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
static inline void
pack_uint_b10g10r10a2_uint(const GLuint src[4], void *dst)
{
      

      uint16_t b =
         _mesa_unsigned_to_unsigned(src[2], 10);
      

      uint16_t g =
         _mesa_unsigned_to_unsigned(src[1], 10);
      

      uint16_t r =
         _mesa_unsigned_to_unsigned(src[0], 10);
      

      uint8_t a =
         _mesa_unsigned_to_unsigned(src[3], 2);

      uint32_t d = 0;
         d |= PACK(b, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(r, 20, 10);
         d |= PACK(a, 30, 2);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_uint_r10g10b10a2_uint(const GLuint src[4], void *dst)
{
      

      uint16_t r =
         _mesa_unsigned_to_unsigned(src[0], 10);
      

      uint16_t g =
         _mesa_unsigned_to_unsigned(src[1], 10);
      

      uint16_t b =
         _mesa_unsigned_to_unsigned(src[2], 10);
      

      uint8_t a =
         _mesa_unsigned_to_unsigned(src[3], 2);

      uint32_t d = 0;
         d |= PACK(r, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(b, 20, 10);
         d |= PACK(a, 30, 2);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_uint_a2b10g10r10_uint(const GLuint src[4], void *dst)
{
      

      uint8_t a =
         _mesa_unsigned_to_unsigned(src[3], 2);
      

      uint16_t b =
         _mesa_unsigned_to_unsigned(src[2], 10);
      

      uint16_t g =
         _mesa_unsigned_to_unsigned(src[1], 10);
      

      uint16_t r =
         _mesa_unsigned_to_unsigned(src[0], 10);

      uint32_t d = 0;
         d |= PACK(a, 0, 2);
         d |= PACK(b, 2, 10);
         d |= PACK(g, 12, 10);
         d |= PACK(r, 22, 10);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_uint_a2r10g10b10_uint(const GLuint src[4], void *dst)
{
      

      uint8_t a =
         _mesa_unsigned_to_unsigned(src[3], 2);
      

      uint16_t r =
         _mesa_unsigned_to_unsigned(src[0], 10);
      

      uint16_t g =
         _mesa_unsigned_to_unsigned(src[1], 10);
      

      uint16_t b =
         _mesa_unsigned_to_unsigned(src[2], 10);

      uint32_t d = 0;
         d |= PACK(a, 0, 2);
         d |= PACK(r, 2, 10);
         d |= PACK(g, 12, 10);
         d |= PACK(b, 22, 10);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_uint_a_uint8(const GLuint src[4], void *dst)
{
      

      uint8_t a =
         _mesa_unsigned_to_unsigned(src[3], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = a;
}

static inline void
pack_uint_a_uint16(const GLuint src[4], void *dst)
{
      

      uint16_t a =
         _mesa_unsigned_to_unsigned(src[3], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = a;
}

static inline void
pack_uint_a_uint32(const GLuint src[4], void *dst)
{
      

      uint32_t a =
         _mesa_unsigned_to_unsigned(src[3], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = a;
}

static inline void
pack_uint_a_sint8(const GLuint src[4], void *dst)
{
      

      int8_t a =
         _mesa_signed_to_signed(src[3], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = a;
}

static inline void
pack_uint_a_sint16(const GLuint src[4], void *dst)
{
      

      int16_t a =
         _mesa_signed_to_signed(src[3], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = a;
}

static inline void
pack_uint_a_sint32(const GLuint src[4], void *dst)
{
      

      int32_t a =
         _mesa_signed_to_signed(src[3], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = a;
}

static inline void
pack_uint_i_uint8(const GLuint src[4], void *dst)
{
      

      uint8_t i =
         _mesa_unsigned_to_unsigned(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = i;
}

static inline void
pack_uint_i_uint16(const GLuint src[4], void *dst)
{
      

      uint16_t i =
         _mesa_unsigned_to_unsigned(src[0], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = i;
}

static inline void
pack_uint_i_uint32(const GLuint src[4], void *dst)
{
      

      uint32_t i =
         _mesa_unsigned_to_unsigned(src[0], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = i;
}

static inline void
pack_uint_i_sint8(const GLuint src[4], void *dst)
{
      

      int8_t i =
         _mesa_signed_to_signed(src[0], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = i;
}

static inline void
pack_uint_i_sint16(const GLuint src[4], void *dst)
{
      

      int16_t i =
         _mesa_signed_to_signed(src[0], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = i;
}

static inline void
pack_uint_i_sint32(const GLuint src[4], void *dst)
{
      

      int32_t i =
         _mesa_signed_to_signed(src[0], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = i;
}

static inline void
pack_uint_l_uint8(const GLuint src[4], void *dst)
{
      

      uint8_t l =
         _mesa_unsigned_to_unsigned(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = l;
}

static inline void
pack_uint_l_uint16(const GLuint src[4], void *dst)
{
      

      uint16_t l =
         _mesa_unsigned_to_unsigned(src[0], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = l;
}

static inline void
pack_uint_l_uint32(const GLuint src[4], void *dst)
{
      

      uint32_t l =
         _mesa_unsigned_to_unsigned(src[0], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = l;
}

static inline void
pack_uint_l_sint8(const GLuint src[4], void *dst)
{
      

      int8_t l =
         _mesa_signed_to_signed(src[0], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = l;
}

static inline void
pack_uint_l_sint16(const GLuint src[4], void *dst)
{
      

      int16_t l =
         _mesa_signed_to_signed(src[0], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = l;
}

static inline void
pack_uint_l_sint32(const GLuint src[4], void *dst)
{
      

      int32_t l =
         _mesa_signed_to_signed(src[0], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = l;
}

static inline void
pack_uint_la_uint8(const GLuint src[4], void *dst)
{
      

      uint8_t l =
         _mesa_unsigned_to_unsigned(src[0], 8);
      

      uint8_t a =
         _mesa_unsigned_to_unsigned(src[3], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_uint_la_uint16(const GLuint src[4], void *dst)
{
      

      uint16_t l =
         _mesa_unsigned_to_unsigned(src[0], 16);
      

      uint16_t a =
         _mesa_unsigned_to_unsigned(src[3], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_uint_la_uint32(const GLuint src[4], void *dst)
{
      

      uint32_t l =
         _mesa_unsigned_to_unsigned(src[0], 32);
      

      uint32_t a =
         _mesa_unsigned_to_unsigned(src[3], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_uint_la_sint8(const GLuint src[4], void *dst)
{
      

      int8_t l =
         _mesa_signed_to_signed(src[0], 8);
      

      int8_t a =
         _mesa_signed_to_signed(src[3], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_uint_la_sint16(const GLuint src[4], void *dst)
{
      

      int16_t l =
         _mesa_signed_to_signed(src[0], 16);
      

      int16_t a =
         _mesa_signed_to_signed(src[3], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_uint_la_sint32(const GLuint src[4], void *dst)
{
      

      int32_t l =
         _mesa_signed_to_signed(src[0], 32);
      

      int32_t a =
         _mesa_signed_to_signed(src[3], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_uint_r_uint8(const GLuint src[4], void *dst)
{
      

      uint8_t r =
         _mesa_unsigned_to_unsigned(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
}

static inline void
pack_uint_r_uint16(const GLuint src[4], void *dst)
{
      

      uint16_t r =
         _mesa_unsigned_to_unsigned(src[0], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
}

static inline void
pack_uint_r_uint32(const GLuint src[4], void *dst)
{
      

      uint32_t r =
         _mesa_unsigned_to_unsigned(src[0], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = r;
}

static inline void
pack_uint_r_sint8(const GLuint src[4], void *dst)
{
      

      int8_t r =
         _mesa_signed_to_signed(src[0], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = r;
}

static inline void
pack_uint_r_sint16(const GLuint src[4], void *dst)
{
      

      int16_t r =
         _mesa_signed_to_signed(src[0], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
}

static inline void
pack_uint_r_sint32(const GLuint src[4], void *dst)
{
      

      int32_t r =
         _mesa_signed_to_signed(src[0], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = r;
}

static inline void
pack_uint_rg_uint8(const GLuint src[4], void *dst)
{
      

      uint8_t r =
         _mesa_unsigned_to_unsigned(src[0], 8);
      

      uint8_t g =
         _mesa_unsigned_to_unsigned(src[1], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_uint_rg_uint16(const GLuint src[4], void *dst)
{
      

      uint16_t r =
         _mesa_unsigned_to_unsigned(src[0], 16);
      

      uint16_t g =
         _mesa_unsigned_to_unsigned(src[1], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_uint_rg_uint32(const GLuint src[4], void *dst)
{
      

      uint32_t r =
         _mesa_unsigned_to_unsigned(src[0], 32);
      

      uint32_t g =
         _mesa_unsigned_to_unsigned(src[1], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_uint_rg_sint8(const GLuint src[4], void *dst)
{
      

      int8_t r =
         _mesa_signed_to_signed(src[0], 8);
      

      int8_t g =
         _mesa_signed_to_signed(src[1], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_uint_rg_sint16(const GLuint src[4], void *dst)
{
      

      int16_t r =
         _mesa_signed_to_signed(src[0], 16);
      

      int16_t g =
         _mesa_signed_to_signed(src[1], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_uint_rg_sint32(const GLuint src[4], void *dst)
{
      

      int32_t r =
         _mesa_signed_to_signed(src[0], 32);
      

      int32_t g =
         _mesa_signed_to_signed(src[1], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_uint_rgb_uint8(const GLuint src[4], void *dst)
{
      

      uint8_t r =
         _mesa_unsigned_to_unsigned(src[0], 8);
      

      uint8_t g =
         _mesa_unsigned_to_unsigned(src[1], 8);
      

      uint8_t b =
         _mesa_unsigned_to_unsigned(src[2], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_uint_rgb_uint16(const GLuint src[4], void *dst)
{
      

      uint16_t r =
         _mesa_unsigned_to_unsigned(src[0], 16);
      

      uint16_t g =
         _mesa_unsigned_to_unsigned(src[1], 16);
      

      uint16_t b =
         _mesa_unsigned_to_unsigned(src[2], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_uint_rgb_uint32(const GLuint src[4], void *dst)
{
      

      uint32_t r =
         _mesa_unsigned_to_unsigned(src[0], 32);
      

      uint32_t g =
         _mesa_unsigned_to_unsigned(src[1], 32);
      

      uint32_t b =
         _mesa_unsigned_to_unsigned(src[2], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_uint_rgb_sint8(const GLuint src[4], void *dst)
{
      

      int8_t r =
         _mesa_signed_to_signed(src[0], 8);
      

      int8_t g =
         _mesa_signed_to_signed(src[1], 8);
      

      int8_t b =
         _mesa_signed_to_signed(src[2], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_uint_rgb_sint16(const GLuint src[4], void *dst)
{
      

      int16_t r =
         _mesa_signed_to_signed(src[0], 16);
      

      int16_t g =
         _mesa_signed_to_signed(src[1], 16);
      

      int16_t b =
         _mesa_signed_to_signed(src[2], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_uint_rgb_sint32(const GLuint src[4], void *dst)
{
      

      int32_t r =
         _mesa_signed_to_signed(src[0], 32);
      

      int32_t g =
         _mesa_signed_to_signed(src[1], 32);
      

      int32_t b =
         _mesa_signed_to_signed(src[2], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_uint_rgba_uint8(const GLuint src[4], void *dst)
{
      

      uint8_t r =
         _mesa_unsigned_to_unsigned(src[0], 8);
      

      uint8_t g =
         _mesa_unsigned_to_unsigned(src[1], 8);
      

      uint8_t b =
         _mesa_unsigned_to_unsigned(src[2], 8);
      

      uint8_t a =
         _mesa_unsigned_to_unsigned(src[3], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_uint_rgba_uint16(const GLuint src[4], void *dst)
{
      

      uint16_t r =
         _mesa_unsigned_to_unsigned(src[0], 16);
      

      uint16_t g =
         _mesa_unsigned_to_unsigned(src[1], 16);
      

      uint16_t b =
         _mesa_unsigned_to_unsigned(src[2], 16);
      

      uint16_t a =
         _mesa_unsigned_to_unsigned(src[3], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_uint_rgba_uint32(const GLuint src[4], void *dst)
{
      

      uint32_t r =
         _mesa_unsigned_to_unsigned(src[0], 32);
      

      uint32_t g =
         _mesa_unsigned_to_unsigned(src[1], 32);
      

      uint32_t b =
         _mesa_unsigned_to_unsigned(src[2], 32);
      

      uint32_t a =
         _mesa_unsigned_to_unsigned(src[3], 32);

      uint32_t *d = (uint32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_uint_rgba_sint8(const GLuint src[4], void *dst)
{
      

      int8_t r =
         _mesa_signed_to_signed(src[0], 8);
      

      int8_t g =
         _mesa_signed_to_signed(src[1], 8);
      

      int8_t b =
         _mesa_signed_to_signed(src[2], 8);
      

      int8_t a =
         _mesa_signed_to_signed(src[3], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_uint_rgba_sint16(const GLuint src[4], void *dst)
{
      

      int16_t r =
         _mesa_signed_to_signed(src[0], 16);
      

      int16_t g =
         _mesa_signed_to_signed(src[1], 16);
      

      int16_t b =
         _mesa_signed_to_signed(src[2], 16);
      

      int16_t a =
         _mesa_signed_to_signed(src[3], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_uint_rgba_sint32(const GLuint src[4], void *dst)
{
      

      int32_t r =
         _mesa_signed_to_signed(src[0], 32);
      

      int32_t g =
         _mesa_signed_to_signed(src[1], 32);
      

      int32_t b =
         _mesa_signed_to_signed(src[2], 32);
      

      int32_t a =
         _mesa_signed_to_signed(src[3], 32);

      int32_t *d = (int32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_uint_rgbx_uint8(const GLuint src[4], void *dst)
{
      

      uint8_t r =
         _mesa_unsigned_to_unsigned(src[0], 8);
      

      uint8_t g =
         _mesa_unsigned_to_unsigned(src[1], 8);
      

      uint8_t b =
         _mesa_unsigned_to_unsigned(src[2], 8);
      
         
      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_uint_rgbx_uint16(const GLuint src[4], void *dst)
{
      

      uint16_t r =
         _mesa_unsigned_to_unsigned(src[0], 16);
      

      uint16_t g =
         _mesa_unsigned_to_unsigned(src[1], 16);
      

      uint16_t b =
         _mesa_unsigned_to_unsigned(src[2], 16);
      
         
      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_uint_rgbx_uint32(const GLuint src[4], void *dst)
{
      

      uint32_t r =
         _mesa_unsigned_to_unsigned(src[0], 32);
      

      uint32_t g =
         _mesa_unsigned_to_unsigned(src[1], 32);
      

      uint32_t b =
         _mesa_unsigned_to_unsigned(src[2], 32);
      
         
      uint32_t *d = (uint32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_uint_rgbx_sint8(const GLuint src[4], void *dst)
{
      

      int8_t r =
         _mesa_signed_to_signed(src[0], 8);
      

      int8_t g =
         _mesa_signed_to_signed(src[1], 8);
      

      int8_t b =
         _mesa_signed_to_signed(src[2], 8);
      
         
      int8_t *d = (int8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_uint_rgbx_sint16(const GLuint src[4], void *dst)
{
      

      int16_t r =
         _mesa_signed_to_signed(src[0], 16);
      

      int16_t g =
         _mesa_signed_to_signed(src[1], 16);
      

      int16_t b =
         _mesa_signed_to_signed(src[2], 16);
      
         
      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_uint_rgbx_sint32(const GLuint src[4], void *dst)
{
      

      int32_t r =
         _mesa_signed_to_signed(src[0], 32);
      

      int32_t g =
         _mesa_signed_to_signed(src[1], 32);
      

      int32_t b =
         _mesa_signed_to_signed(src[2], 32);
      
         
      int32_t *d = (int32_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }
                                                                                                                                                                                                      
/* float packing functions */


static inline void
pack_float_a8b8g8r8_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);

      uint32_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_x8b8g8r8_unorm(const GLfloat src[4], void *dst)
{
      
               

      uint8_t b =
            _mesa_float_to_unorm(src[2], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);

      uint32_t d = 0;
                     d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r8g8b8a8_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 8);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);

      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
         d |= PACK(a, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r8g8b8x8_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 8);
      
         
      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_float_b8g8r8a8_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);

      uint32_t d = 0;
         d |= PACK(b, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(r, 16, 8);
         d |= PACK(a, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_b8g8r8x8_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);
      
         
      uint32_t d = 0;
         d |= PACK(b, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(r, 16, 8);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_float_a8r8g8b8_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 8);

      uint32_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(r, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(b, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_x8r8g8b8_unorm(const GLfloat src[4], void *dst)
{
      
               

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 8);

      uint32_t d = 0;
                     d |= PACK(r, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(b, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_l16a16_unorm(const GLfloat src[4], void *dst)
{
      

      uint16_t l =
            _mesa_float_to_unorm(src[0], 16);
      

      uint16_t a =
            _mesa_float_to_unorm(src[3], 16);

      uint32_t d = 0;
         d |= PACK(l, 0, 16);
         d |= PACK(a, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_a16l16_unorm(const GLfloat src[4], void *dst)
{
      

      uint16_t a =
            _mesa_float_to_unorm(src[3], 16);
      

      uint16_t l =
            _mesa_float_to_unorm(src[0], 16);

      uint32_t d = 0;
         d |= PACK(a, 0, 16);
         d |= PACK(l, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_b5g6r5_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 5);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 6);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 5);

      uint16_t d = 0;
         d |= PACK(b, 0, 5);
         d |= PACK(g, 5, 6);
         d |= PACK(r, 11, 5);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_r5g6b5_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 5);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 6);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 5);

      uint16_t d = 0;
         d |= PACK(r, 0, 5);
         d |= PACK(g, 5, 6);
         d |= PACK(b, 11, 5);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_b4g4r4a4_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 4);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 4);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 4);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 4);

      uint16_t d = 0;
         d |= PACK(b, 0, 4);
         d |= PACK(g, 4, 4);
         d |= PACK(r, 8, 4);
         d |= PACK(a, 12, 4);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_b4g4r4x4_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 4);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 4);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 4);
      
         
      uint16_t d = 0;
         d |= PACK(b, 0, 4);
         d |= PACK(g, 4, 4);
         d |= PACK(r, 8, 4);
                  (*(uint16_t *)dst) = d;
}

static inline void
pack_float_a4r4g4b4_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 4);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 4);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 4);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 4);

      uint16_t d = 0;
         d |= PACK(a, 0, 4);
         d |= PACK(r, 4, 4);
         d |= PACK(g, 8, 4);
         d |= PACK(b, 12, 4);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_a1b5g5r5_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 1);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 5);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 5);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 5);

      uint16_t d = 0;
         d |= PACK(a, 0, 1);
         d |= PACK(b, 1, 5);
         d |= PACK(g, 6, 5);
         d |= PACK(r, 11, 5);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_b5g5r5a1_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 5);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 5);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 5);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 1);

      uint16_t d = 0;
         d |= PACK(b, 0, 5);
         d |= PACK(g, 5, 5);
         d |= PACK(r, 10, 5);
         d |= PACK(a, 15, 1);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_b5g5r5x1_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 5);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 5);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 5);
      
         
      uint16_t d = 0;
         d |= PACK(b, 0, 5);
         d |= PACK(g, 5, 5);
         d |= PACK(r, 10, 5);
                  (*(uint16_t *)dst) = d;
}

static inline void
pack_float_a1r5g5b5_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 1);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 5);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 5);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 5);

      uint16_t d = 0;
         d |= PACK(a, 0, 1);
         d |= PACK(r, 1, 5);
         d |= PACK(g, 6, 5);
         d |= PACK(b, 11, 5);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_l8a8_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t l =
            _mesa_float_to_unorm(src[0], 8);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);

      uint16_t d = 0;
         d |= PACK(l, 0, 8);
         d |= PACK(a, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_a8l8_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);
      

      uint8_t l =
            _mesa_float_to_unorm(src[0], 8);

      uint16_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(l, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_r8g8_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);

      uint16_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_g8r8_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);

      uint16_t d = 0;
         d |= PACK(g, 0, 8);
         d |= PACK(r, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_l4a4_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t l =
            _mesa_float_to_unorm(src[0], 4);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 4);

      uint8_t d = 0;
         d |= PACK(l, 0, 4);
         d |= PACK(a, 4, 4);
      (*(uint8_t *)dst) = d;
}

static inline void
pack_float_b2g3r3_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 2);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 3);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 3);

      uint8_t d = 0;
         d |= PACK(b, 0, 2);
         d |= PACK(g, 2, 3);
         d |= PACK(r, 5, 3);
      (*(uint8_t *)dst) = d;
}

static inline void
pack_float_r16g16_unorm(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 16);
      

      uint16_t g =
            _mesa_float_to_unorm(src[1], 16);

      uint32_t d = 0;
         d |= PACK(r, 0, 16);
         d |= PACK(g, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_g16r16_unorm(const GLfloat src[4], void *dst)
{
      

      uint16_t g =
            _mesa_float_to_unorm(src[1], 16);
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 16);

      uint32_t d = 0;
         d |= PACK(g, 0, 16);
         d |= PACK(r, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_b10g10r10a2_unorm(const GLfloat src[4], void *dst)
{
      

      uint16_t b =
            _mesa_float_to_unorm(src[2], 10);
      

      uint16_t g =
            _mesa_float_to_unorm(src[1], 10);
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 10);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 2);

      uint32_t d = 0;
         d |= PACK(b, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(r, 20, 10);
         d |= PACK(a, 30, 2);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_b10g10r10x2_unorm(const GLfloat src[4], void *dst)
{
      

      uint16_t b =
            _mesa_float_to_unorm(src[2], 10);
      

      uint16_t g =
            _mesa_float_to_unorm(src[1], 10);
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 10);
      
         
      uint32_t d = 0;
         d |= PACK(b, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(r, 20, 10);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r10g10b10a2_unorm(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 10);
      

      uint16_t g =
            _mesa_float_to_unorm(src[1], 10);
      

      uint16_t b =
            _mesa_float_to_unorm(src[2], 10);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 2);

      uint32_t d = 0;
         d |= PACK(r, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(b, 20, 10);
         d |= PACK(a, 30, 2);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r10g10b10x2_unorm(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 10);
      

      uint16_t g =
            _mesa_float_to_unorm(src[1], 10);
      

      uint16_t b =
            _mesa_float_to_unorm(src[2], 10);
      
         
      uint32_t d = 0;
         d |= PACK(r, 0, 10);
         d |= PACK(g, 10, 10);
         d |= PACK(b, 20, 10);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r3g3b2_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 3);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 3);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 2);

      uint8_t d = 0;
         d |= PACK(r, 0, 3);
         d |= PACK(g, 3, 3);
         d |= PACK(b, 6, 2);
      (*(uint8_t *)dst) = d;
}

static inline void
pack_float_a4b4g4r4_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 4);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 4);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 4);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 4);

      uint16_t d = 0;
         d |= PACK(a, 0, 4);
         d |= PACK(b, 4, 4);
         d |= PACK(g, 8, 4);
         d |= PACK(r, 12, 4);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_r4g4b4a4_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 4);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 4);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 4);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 4);

      uint16_t d = 0;
         d |= PACK(r, 0, 4);
         d |= PACK(g, 4, 4);
         d |= PACK(b, 8, 4);
         d |= PACK(a, 12, 4);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_r5g5b5a1_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 5);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 5);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 5);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 1);

      uint16_t d = 0;
         d |= PACK(r, 0, 5);
         d |= PACK(g, 5, 5);
         d |= PACK(b, 10, 5);
         d |= PACK(a, 15, 1);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_a2b10g10r10_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 2);
      

      uint16_t b =
            _mesa_float_to_unorm(src[2], 10);
      

      uint16_t g =
            _mesa_float_to_unorm(src[1], 10);
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 10);

      uint32_t d = 0;
         d |= PACK(a, 0, 2);
         d |= PACK(b, 2, 10);
         d |= PACK(g, 12, 10);
         d |= PACK(r, 22, 10);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_a2r10g10b10_unorm(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 2);
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 10);
      

      uint16_t g =
            _mesa_float_to_unorm(src[1], 10);
      

      uint16_t b =
            _mesa_float_to_unorm(src[2], 10);

      uint32_t d = 0;
         d |= PACK(a, 0, 2);
         d |= PACK(r, 2, 10);
         d |= PACK(g, 12, 10);
         d |= PACK(b, 22, 10);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_a_unorm8(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = a;
}

static inline void
pack_float_a_unorm16(const GLfloat src[4], void *dst)
{
      

      uint16_t a =
            _mesa_float_to_unorm(src[3], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = a;
}

static inline void
pack_float_l_unorm8(const GLfloat src[4], void *dst)
{
      

      uint8_t l =
            _mesa_float_to_unorm(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = l;
}

static inline void
pack_float_l_unorm16(const GLfloat src[4], void *dst)
{
      

      uint16_t l =
            _mesa_float_to_unorm(src[0], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = l;
}

static inline void
pack_float_i_unorm8(const GLfloat src[4], void *dst)
{
      

      uint8_t i =
            _mesa_float_to_unorm(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = i;
}

static inline void
pack_float_i_unorm16(const GLfloat src[4], void *dst)
{
      

      uint16_t i =
            _mesa_float_to_unorm(src[0], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = i;
}

static inline void
pack_float_r_unorm8(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
}

static inline void
pack_float_r_unorm16(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
}

static inline void
pack_float_bgr_unorm8(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = b;
         d[1] = g;
         d[2] = r;
}

static inline void
pack_float_rgb_unorm8(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            _mesa_float_to_unorm(src[0], 8);
      

      uint8_t g =
            _mesa_float_to_unorm(src[1], 8);
      

      uint8_t b =
            _mesa_float_to_unorm(src[2], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_float_rgba_unorm16(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 16);
      

      uint16_t g =
            _mesa_float_to_unorm(src[1], 16);
      

      uint16_t b =
            _mesa_float_to_unorm(src[2], 16);
      

      uint16_t a =
            _mesa_float_to_unorm(src[3], 16);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_float_rgbx_unorm16(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_unorm(src[0], 16);
      

      uint16_t g =
            _mesa_float_to_unorm(src[1], 16);
      

      uint16_t b =
            _mesa_float_to_unorm(src[2], 16);
      
         
      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_float_a8b8g8r8_snorm(const GLfloat src[4], void *dst)
{
      

      int8_t a =
         _mesa_float_to_snorm(src[3], 8);
      

      int8_t b =
         _mesa_float_to_snorm(src[2], 8);
      

      int8_t g =
         _mesa_float_to_snorm(src[1], 8);
      

      int8_t r =
         _mesa_float_to_snorm(src[0], 8);

      uint32_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_x8b8g8r8_snorm(const GLfloat src[4], void *dst)
{
      
               

      int8_t b =
         _mesa_float_to_snorm(src[2], 8);
      

      int8_t g =
         _mesa_float_to_snorm(src[1], 8);
      

      int8_t r =
         _mesa_float_to_snorm(src[0], 8);

      uint32_t d = 0;
                     d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r8g8b8a8_snorm(const GLfloat src[4], void *dst)
{
      

      int8_t r =
         _mesa_float_to_snorm(src[0], 8);
      

      int8_t g =
         _mesa_float_to_snorm(src[1], 8);
      

      int8_t b =
         _mesa_float_to_snorm(src[2], 8);
      

      int8_t a =
         _mesa_float_to_snorm(src[3], 8);

      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
         d |= PACK(a, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r8g8b8x8_snorm(const GLfloat src[4], void *dst)
{
      

      int8_t r =
         _mesa_float_to_snorm(src[0], 8);
      

      int8_t g =
         _mesa_float_to_snorm(src[1], 8);
      

      int8_t b =
         _mesa_float_to_snorm(src[2], 8);
      
         
      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r16g16_snorm(const GLfloat src[4], void *dst)
{
      

      int16_t r =
         _mesa_float_to_snorm(src[0], 16);
      

      int16_t g =
         _mesa_float_to_snorm(src[1], 16);

      uint32_t d = 0;
         d |= PACK(r, 0, 16);
         d |= PACK(g, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_g16r16_snorm(const GLfloat src[4], void *dst)
{
      

      int16_t g =
         _mesa_float_to_snorm(src[1], 16);
      

      int16_t r =
         _mesa_float_to_snorm(src[0], 16);

      uint32_t d = 0;
         d |= PACK(g, 0, 16);
         d |= PACK(r, 16, 16);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r8g8_snorm(const GLfloat src[4], void *dst)
{
      

      int8_t r =
         _mesa_float_to_snorm(src[0], 8);
      

      int8_t g =
         _mesa_float_to_snorm(src[1], 8);

      uint16_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_g8r8_snorm(const GLfloat src[4], void *dst)
{
      

      int8_t g =
         _mesa_float_to_snorm(src[1], 8);
      

      int8_t r =
         _mesa_float_to_snorm(src[0], 8);

      uint16_t d = 0;
         d |= PACK(g, 0, 8);
         d |= PACK(r, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_l8a8_snorm(const GLfloat src[4], void *dst)
{
      

      int8_t l =
         _mesa_float_to_snorm(src[0], 8);
      

      int8_t a =
         _mesa_float_to_snorm(src[3], 8);

      uint16_t d = 0;
         d |= PACK(l, 0, 8);
         d |= PACK(a, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_a8l8_snorm(const GLfloat src[4], void *dst)
{
      

      int8_t a =
         _mesa_float_to_snorm(src[3], 8);
      

      int8_t l =
         _mesa_float_to_snorm(src[0], 8);

      uint16_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(l, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_a_snorm8(const GLfloat src[4], void *dst)
{
      

      int8_t a =
         _mesa_float_to_snorm(src[3], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = a;
}

static inline void
pack_float_a_snorm16(const GLfloat src[4], void *dst)
{
      

      int16_t a =
         _mesa_float_to_snorm(src[3], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = a;
}

static inline void
pack_float_l_snorm8(const GLfloat src[4], void *dst)
{
      

      int8_t l =
         _mesa_float_to_snorm(src[0], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = l;
}

static inline void
pack_float_l_snorm16(const GLfloat src[4], void *dst)
{
      

      int16_t l =
         _mesa_float_to_snorm(src[0], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = l;
}

static inline void
pack_float_i_snorm8(const GLfloat src[4], void *dst)
{
      

      int8_t i =
         _mesa_float_to_snorm(src[0], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = i;
}

static inline void
pack_float_i_snorm16(const GLfloat src[4], void *dst)
{
      

      int16_t i =
         _mesa_float_to_snorm(src[0], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = i;
}

static inline void
pack_float_r_snorm8(const GLfloat src[4], void *dst)
{
      

      int8_t r =
         _mesa_float_to_snorm(src[0], 8);

      int8_t *d = (int8_t *)dst;
         d[0] = r;
}

static inline void
pack_float_r_snorm16(const GLfloat src[4], void *dst)
{
      

      int16_t r =
         _mesa_float_to_snorm(src[0], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
}

static inline void
pack_float_la_snorm16(const GLfloat src[4], void *dst)
{
      

      int16_t l =
         _mesa_float_to_snorm(src[0], 16);
      

      int16_t a =
         _mesa_float_to_snorm(src[3], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_float_rgb_snorm16(const GLfloat src[4], void *dst)
{
      

      int16_t r =
         _mesa_float_to_snorm(src[0], 16);
      

      int16_t g =
         _mesa_float_to_snorm(src[1], 16);
      

      int16_t b =
         _mesa_float_to_snorm(src[2], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_float_rgba_snorm16(const GLfloat src[4], void *dst)
{
      

      int16_t r =
         _mesa_float_to_snorm(src[0], 16);
      

      int16_t g =
         _mesa_float_to_snorm(src[1], 16);
      

      int16_t b =
         _mesa_float_to_snorm(src[2], 16);
      

      int16_t a =
         _mesa_float_to_snorm(src[3], 16);

      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_float_rgbx_snorm16(const GLfloat src[4], void *dst)
{
      

      int16_t r =
         _mesa_float_to_snorm(src[0], 16);
      

      int16_t g =
         _mesa_float_to_snorm(src[1], 16);
      

      int16_t b =
         _mesa_float_to_snorm(src[2], 16);
      
         
      int16_t *d = (int16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_float_a8b8g8r8_srgb(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);
      

      uint8_t b =
            
            util_format_linear_float_to_srgb_8unorm(src[2]);
      

      uint8_t g =
            
            util_format_linear_float_to_srgb_8unorm(src[1]);
      

      uint8_t r =
            
            util_format_linear_float_to_srgb_8unorm(src[0]);

      uint32_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_b8g8r8a8_srgb(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            
            util_format_linear_float_to_srgb_8unorm(src[2]);
      

      uint8_t g =
            
            util_format_linear_float_to_srgb_8unorm(src[1]);
      

      uint8_t r =
            
            util_format_linear_float_to_srgb_8unorm(src[0]);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);

      uint32_t d = 0;
         d |= PACK(b, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(r, 16, 8);
         d |= PACK(a, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_a8r8g8b8_srgb(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);
      

      uint8_t r =
            
            util_format_linear_float_to_srgb_8unorm(src[0]);
      

      uint8_t g =
            
            util_format_linear_float_to_srgb_8unorm(src[1]);
      

      uint8_t b =
            
            util_format_linear_float_to_srgb_8unorm(src[2]);

      uint32_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(r, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(b, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_b8g8r8x8_srgb(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            
            util_format_linear_float_to_srgb_8unorm(src[2]);
      

      uint8_t g =
            
            util_format_linear_float_to_srgb_8unorm(src[1]);
      

      uint8_t r =
            
            util_format_linear_float_to_srgb_8unorm(src[0]);
      
         
      uint32_t d = 0;
         d |= PACK(b, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(r, 16, 8);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_float_x8r8g8b8_srgb(const GLfloat src[4], void *dst)
{
      
               

      uint8_t r =
            
            util_format_linear_float_to_srgb_8unorm(src[0]);
      

      uint8_t g =
            
            util_format_linear_float_to_srgb_8unorm(src[1]);
      

      uint8_t b =
            
            util_format_linear_float_to_srgb_8unorm(src[2]);

      uint32_t d = 0;
                     d |= PACK(r, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(b, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r8g8b8a8_srgb(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            
            util_format_linear_float_to_srgb_8unorm(src[0]);
      

      uint8_t g =
            
            util_format_linear_float_to_srgb_8unorm(src[1]);
      

      uint8_t b =
            
            util_format_linear_float_to_srgb_8unorm(src[2]);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);

      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
         d |= PACK(a, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_r8g8b8x8_srgb(const GLfloat src[4], void *dst)
{
      

      uint8_t r =
            
            util_format_linear_float_to_srgb_8unorm(src[0]);
      

      uint8_t g =
            
            util_format_linear_float_to_srgb_8unorm(src[1]);
      

      uint8_t b =
            
            util_format_linear_float_to_srgb_8unorm(src[2]);
      
         
      uint32_t d = 0;
         d |= PACK(r, 0, 8);
         d |= PACK(g, 8, 8);
         d |= PACK(b, 16, 8);
                  (*(uint32_t *)dst) = d;
}

static inline void
pack_float_x8b8g8r8_srgb(const GLfloat src[4], void *dst)
{
      
               

      uint8_t b =
            
            util_format_linear_float_to_srgb_8unorm(src[2]);
      

      uint8_t g =
            
            util_format_linear_float_to_srgb_8unorm(src[1]);
      

      uint8_t r =
            
            util_format_linear_float_to_srgb_8unorm(src[0]);

      uint32_t d = 0;
                     d |= PACK(b, 8, 8);
         d |= PACK(g, 16, 8);
         d |= PACK(r, 24, 8);
      (*(uint32_t *)dst) = d;
}

static inline void
pack_float_l8a8_srgb(const GLfloat src[4], void *dst)
{
      

      uint8_t l =
            _mesa_float_to_unorm(src[0], 8);
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);

      uint16_t d = 0;
         d |= PACK(l, 0, 8);
         d |= PACK(a, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_a8l8_srgb(const GLfloat src[4], void *dst)
{
      

      uint8_t a =
            _mesa_float_to_unorm(src[3], 8);
      

      uint8_t l =
            _mesa_float_to_unorm(src[0], 8);

      uint16_t d = 0;
         d |= PACK(a, 0, 8);
         d |= PACK(l, 8, 8);
      (*(uint16_t *)dst) = d;
}

static inline void
pack_float_l_srgb8(const GLfloat src[4], void *dst)
{
      

      uint8_t l =
            _mesa_float_to_unorm(src[0], 8);

      uint8_t *d = (uint8_t *)dst;
         d[0] = l;
}

static inline void
pack_float_bgr_srgb8(const GLfloat src[4], void *dst)
{
      

      uint8_t b =
            
            util_format_linear_float_to_srgb_8unorm(src[2]);
      

      uint8_t g =
            
            util_format_linear_float_to_srgb_8unorm(src[1]);
      

      uint8_t r =
            
            util_format_linear_float_to_srgb_8unorm(src[0]);

      uint8_t *d = (uint8_t *)dst;
         d[0] = b;
         d[1] = g;
         d[2] = r;
}
            
static inline void
pack_float_a_float16(const GLfloat src[4], void *dst)
{
      

      uint16_t a =
            _mesa_float_to_half(src[3]);

      uint16_t *d = (uint16_t *)dst;
         d[0] = a;
}

static inline void
pack_float_a_float32(const GLfloat src[4], void *dst)
{
      

      float a =
            src[3];

      float *d = (float *)dst;
         d[0] = a;
}

static inline void
pack_float_l_float16(const GLfloat src[4], void *dst)
{
      

      uint16_t l =
            _mesa_float_to_half(src[0]);

      uint16_t *d = (uint16_t *)dst;
         d[0] = l;
}

static inline void
pack_float_l_float32(const GLfloat src[4], void *dst)
{
      

      float l =
            src[0];

      float *d = (float *)dst;
         d[0] = l;
}

static inline void
pack_float_la_float16(const GLfloat src[4], void *dst)
{
      

      uint16_t l =
            _mesa_float_to_half(src[0]);
      

      uint16_t a =
            _mesa_float_to_half(src[3]);

      uint16_t *d = (uint16_t *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_float_la_float32(const GLfloat src[4], void *dst)
{
      

      float l =
            src[0];
      

      float a =
            src[3];

      float *d = (float *)dst;
         d[0] = l;
         d[1] = a;
}

static inline void
pack_float_i_float16(const GLfloat src[4], void *dst)
{
      

      uint16_t i =
            _mesa_float_to_half(src[0]);

      uint16_t *d = (uint16_t *)dst;
         d[0] = i;
}

static inline void
pack_float_i_float32(const GLfloat src[4], void *dst)
{
      

      float i =
            src[0];

      float *d = (float *)dst;
         d[0] = i;
}

static inline void
pack_float_r_float16(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_half(src[0]);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
}

static inline void
pack_float_r_float32(const GLfloat src[4], void *dst)
{
      

      float r =
            src[0];

      float *d = (float *)dst;
         d[0] = r;
}

static inline void
pack_float_rg_float16(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_half(src[0]);
      

      uint16_t g =
            _mesa_float_to_half(src[1]);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_float_rg_float32(const GLfloat src[4], void *dst)
{
      

      float r =
            src[0];
      

      float g =
            src[1];

      float *d = (float *)dst;
         d[0] = r;
         d[1] = g;
}

static inline void
pack_float_rgb_float16(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_half(src[0]);
      

      uint16_t g =
            _mesa_float_to_half(src[1]);
      

      uint16_t b =
            _mesa_float_to_half(src[2]);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_float_rgb_float32(const GLfloat src[4], void *dst)
{
      

      float r =
            src[0];
      

      float g =
            src[1];
      

      float b =
            src[2];

      float *d = (float *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
}

static inline void
pack_float_rgba_float16(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_half(src[0]);
      

      uint16_t g =
            _mesa_float_to_half(src[1]);
      

      uint16_t b =
            _mesa_float_to_half(src[2]);
      

      uint16_t a =
            _mesa_float_to_half(src[3]);

      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_float_rgba_float32(const GLfloat src[4], void *dst)
{
      

      float r =
            src[0];
      

      float g =
            src[1];
      

      float b =
            src[2];
      

      float a =
            src[3];

      float *d = (float *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
         d[3] = a;
}

static inline void
pack_float_rgbx_float16(const GLfloat src[4], void *dst)
{
      

      uint16_t r =
            _mesa_float_to_half(src[0]);
      

      uint16_t g =
            _mesa_float_to_half(src[1]);
      

      uint16_t b =
            _mesa_float_to_half(src[2]);
      
         
      uint16_t *d = (uint16_t *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }

static inline void
pack_float_rgbx_float32(const GLfloat src[4], void *dst)
{
      

      float r =
            src[0];
      

      float g =
            src[1];
      

      float b =
            src[2];
      
         
      float *d = (float *)dst;
         d[0] = r;
         d[1] = g;
         d[2] = b;
            }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
static inline void
pack_float_r9g9b9e5_float(const GLfloat src[4], void *dst)
{
   GLuint *d = (GLuint *) dst;
   *d = float3_to_rgb9e5(src);
}

static inline void
pack_float_r11g11b10_float(const GLfloat src[4], void *dst)
{
   GLuint *d = (GLuint *) dst;
   *d = float3_to_r11g11b10f(src);
}

/**
 * Return a function that can pack a GLubyte rgba[4] color.
 */
gl_pack_ubyte_rgba_func
_mesa_get_pack_ubyte_rgba_function(mesa_format format)
{
   switch (format) {

   case MESA_FORMAT_A8B8G8R8_UNORM:
      return pack_ubyte_a8b8g8r8_unorm;

   case MESA_FORMAT_X8B8G8R8_UNORM:
      return pack_ubyte_x8b8g8r8_unorm;

   case MESA_FORMAT_R8G8B8A8_UNORM:
      return pack_ubyte_r8g8b8a8_unorm;

   case MESA_FORMAT_R8G8B8X8_UNORM:
      return pack_ubyte_r8g8b8x8_unorm;

   case MESA_FORMAT_B8G8R8A8_UNORM:
      return pack_ubyte_b8g8r8a8_unorm;

   case MESA_FORMAT_B8G8R8X8_UNORM:
      return pack_ubyte_b8g8r8x8_unorm;

   case MESA_FORMAT_A8R8G8B8_UNORM:
      return pack_ubyte_a8r8g8b8_unorm;

   case MESA_FORMAT_X8R8G8B8_UNORM:
      return pack_ubyte_x8r8g8b8_unorm;

   case MESA_FORMAT_L16A16_UNORM:
      return pack_ubyte_l16a16_unorm;

   case MESA_FORMAT_A16L16_UNORM:
      return pack_ubyte_a16l16_unorm;

   case MESA_FORMAT_B5G6R5_UNORM:
      return pack_ubyte_b5g6r5_unorm;

   case MESA_FORMAT_R5G6B5_UNORM:
      return pack_ubyte_r5g6b5_unorm;

   case MESA_FORMAT_B4G4R4A4_UNORM:
      return pack_ubyte_b4g4r4a4_unorm;

   case MESA_FORMAT_B4G4R4X4_UNORM:
      return pack_ubyte_b4g4r4x4_unorm;

   case MESA_FORMAT_A4R4G4B4_UNORM:
      return pack_ubyte_a4r4g4b4_unorm;

   case MESA_FORMAT_A1B5G5R5_UNORM:
      return pack_ubyte_a1b5g5r5_unorm;

   case MESA_FORMAT_B5G5R5A1_UNORM:
      return pack_ubyte_b5g5r5a1_unorm;

   case MESA_FORMAT_B5G5R5X1_UNORM:
      return pack_ubyte_b5g5r5x1_unorm;

   case MESA_FORMAT_A1R5G5B5_UNORM:
      return pack_ubyte_a1r5g5b5_unorm;

   case MESA_FORMAT_L8A8_UNORM:
      return pack_ubyte_l8a8_unorm;

   case MESA_FORMAT_A8L8_UNORM:
      return pack_ubyte_a8l8_unorm;

   case MESA_FORMAT_R8G8_UNORM:
      return pack_ubyte_r8g8_unorm;

   case MESA_FORMAT_G8R8_UNORM:
      return pack_ubyte_g8r8_unorm;

   case MESA_FORMAT_L4A4_UNORM:
      return pack_ubyte_l4a4_unorm;

   case MESA_FORMAT_B2G3R3_UNORM:
      return pack_ubyte_b2g3r3_unorm;

   case MESA_FORMAT_R16G16_UNORM:
      return pack_ubyte_r16g16_unorm;

   case MESA_FORMAT_G16R16_UNORM:
      return pack_ubyte_g16r16_unorm;

   case MESA_FORMAT_B10G10R10A2_UNORM:
      return pack_ubyte_b10g10r10a2_unorm;

   case MESA_FORMAT_B10G10R10X2_UNORM:
      return pack_ubyte_b10g10r10x2_unorm;

   case MESA_FORMAT_R10G10B10A2_UNORM:
      return pack_ubyte_r10g10b10a2_unorm;

   case MESA_FORMAT_R10G10B10X2_UNORM:
      return pack_ubyte_r10g10b10x2_unorm;

   case MESA_FORMAT_R3G3B2_UNORM:
      return pack_ubyte_r3g3b2_unorm;

   case MESA_FORMAT_A4B4G4R4_UNORM:
      return pack_ubyte_a4b4g4r4_unorm;

   case MESA_FORMAT_R4G4B4A4_UNORM:
      return pack_ubyte_r4g4b4a4_unorm;

   case MESA_FORMAT_R5G5B5A1_UNORM:
      return pack_ubyte_r5g5b5a1_unorm;

   case MESA_FORMAT_A2B10G10R10_UNORM:
      return pack_ubyte_a2b10g10r10_unorm;

   case MESA_FORMAT_A2R10G10B10_UNORM:
      return pack_ubyte_a2r10g10b10_unorm;

   case MESA_FORMAT_A_UNORM8:
      return pack_ubyte_a_unorm8;

   case MESA_FORMAT_A_UNORM16:
      return pack_ubyte_a_unorm16;

   case MESA_FORMAT_L_UNORM8:
      return pack_ubyte_l_unorm8;

   case MESA_FORMAT_L_UNORM16:
      return pack_ubyte_l_unorm16;

   case MESA_FORMAT_I_UNORM8:
      return pack_ubyte_i_unorm8;

   case MESA_FORMAT_I_UNORM16:
      return pack_ubyte_i_unorm16;

   case MESA_FORMAT_R_UNORM8:
      return pack_ubyte_r_unorm8;

   case MESA_FORMAT_R_UNORM16:
      return pack_ubyte_r_unorm16;

   case MESA_FORMAT_BGR_UNORM8:
      return pack_ubyte_bgr_unorm8;

   case MESA_FORMAT_RGB_UNORM8:
      return pack_ubyte_rgb_unorm8;

   case MESA_FORMAT_RGBA_UNORM16:
      return pack_ubyte_rgba_unorm16;

   case MESA_FORMAT_RGBX_UNORM16:
      return pack_ubyte_rgbx_unorm16;

   case MESA_FORMAT_A8B8G8R8_SNORM:
      return pack_ubyte_a8b8g8r8_snorm;

   case MESA_FORMAT_X8B8G8R8_SNORM:
      return pack_ubyte_x8b8g8r8_snorm;

   case MESA_FORMAT_R8G8B8A8_SNORM:
      return pack_ubyte_r8g8b8a8_snorm;

   case MESA_FORMAT_R8G8B8X8_SNORM:
      return pack_ubyte_r8g8b8x8_snorm;

   case MESA_FORMAT_R16G16_SNORM:
      return pack_ubyte_r16g16_snorm;

   case MESA_FORMAT_G16R16_SNORM:
      return pack_ubyte_g16r16_snorm;

   case MESA_FORMAT_R8G8_SNORM:
      return pack_ubyte_r8g8_snorm;

   case MESA_FORMAT_G8R8_SNORM:
      return pack_ubyte_g8r8_snorm;

   case MESA_FORMAT_L8A8_SNORM:
      return pack_ubyte_l8a8_snorm;

   case MESA_FORMAT_A8L8_SNORM:
      return pack_ubyte_a8l8_snorm;

   case MESA_FORMAT_A_SNORM8:
      return pack_ubyte_a_snorm8;

   case MESA_FORMAT_A_SNORM16:
      return pack_ubyte_a_snorm16;

   case MESA_FORMAT_L_SNORM8:
      return pack_ubyte_l_snorm8;

   case MESA_FORMAT_L_SNORM16:
      return pack_ubyte_l_snorm16;

   case MESA_FORMAT_I_SNORM8:
      return pack_ubyte_i_snorm8;

   case MESA_FORMAT_I_SNORM16:
      return pack_ubyte_i_snorm16;

   case MESA_FORMAT_R_SNORM8:
      return pack_ubyte_r_snorm8;

   case MESA_FORMAT_R_SNORM16:
      return pack_ubyte_r_snorm16;

   case MESA_FORMAT_LA_SNORM16:
      return pack_ubyte_la_snorm16;

   case MESA_FORMAT_RGB_SNORM16:
      return pack_ubyte_rgb_snorm16;

   case MESA_FORMAT_RGBA_SNORM16:
      return pack_ubyte_rgba_snorm16;

   case MESA_FORMAT_RGBX_SNORM16:
      return pack_ubyte_rgbx_snorm16;

   case MESA_FORMAT_A8B8G8R8_SRGB:
      return pack_ubyte_a8b8g8r8_srgb;

   case MESA_FORMAT_B8G8R8A8_SRGB:
      return pack_ubyte_b8g8r8a8_srgb;

   case MESA_FORMAT_A8R8G8B8_SRGB:
      return pack_ubyte_a8r8g8b8_srgb;

   case MESA_FORMAT_B8G8R8X8_SRGB:
      return pack_ubyte_b8g8r8x8_srgb;

   case MESA_FORMAT_X8R8G8B8_SRGB:
      return pack_ubyte_x8r8g8b8_srgb;

   case MESA_FORMAT_R8G8B8A8_SRGB:
      return pack_ubyte_r8g8b8a8_srgb;

   case MESA_FORMAT_R8G8B8X8_SRGB:
      return pack_ubyte_r8g8b8x8_srgb;

   case MESA_FORMAT_X8B8G8R8_SRGB:
      return pack_ubyte_x8b8g8r8_srgb;

   case MESA_FORMAT_L8A8_SRGB:
      return pack_ubyte_l8a8_srgb;

   case MESA_FORMAT_A8L8_SRGB:
      return pack_ubyte_a8l8_srgb;

   case MESA_FORMAT_L_SRGB8:
      return pack_ubyte_l_srgb8;

   case MESA_FORMAT_BGR_SRGB8:
      return pack_ubyte_bgr_srgb8;

   case MESA_FORMAT_R9G9B9E5_FLOAT:
      return pack_ubyte_r9g9b9e5_float;

   case MESA_FORMAT_R11G11B10_FLOAT:
      return pack_ubyte_r11g11b10_float;

   case MESA_FORMAT_A_FLOAT16:
      return pack_ubyte_a_float16;

   case MESA_FORMAT_A_FLOAT32:
      return pack_ubyte_a_float32;

   case MESA_FORMAT_L_FLOAT16:
      return pack_ubyte_l_float16;

   case MESA_FORMAT_L_FLOAT32:
      return pack_ubyte_l_float32;

   case MESA_FORMAT_LA_FLOAT16:
      return pack_ubyte_la_float16;

   case MESA_FORMAT_LA_FLOAT32:
      return pack_ubyte_la_float32;

   case MESA_FORMAT_I_FLOAT16:
      return pack_ubyte_i_float16;

   case MESA_FORMAT_I_FLOAT32:
      return pack_ubyte_i_float32;

   case MESA_FORMAT_R_FLOAT16:
      return pack_ubyte_r_float16;

   case MESA_FORMAT_R_FLOAT32:
      return pack_ubyte_r_float32;

   case MESA_FORMAT_RG_FLOAT16:
      return pack_ubyte_rg_float16;

   case MESA_FORMAT_RG_FLOAT32:
      return pack_ubyte_rg_float32;

   case MESA_FORMAT_RGB_FLOAT16:
      return pack_ubyte_rgb_float16;

   case MESA_FORMAT_RGB_FLOAT32:
      return pack_ubyte_rgb_float32;

   case MESA_FORMAT_RGBA_FLOAT16:
      return pack_ubyte_rgba_float16;

   case MESA_FORMAT_RGBA_FLOAT32:
      return pack_ubyte_rgba_float32;

   case MESA_FORMAT_RGBX_FLOAT16:
      return pack_ubyte_rgbx_float16;

   case MESA_FORMAT_RGBX_FLOAT32:
      return pack_ubyte_rgbx_float32;

   case MESA_FORMAT_B10G10R10A2_UINT:
      return pack_ubyte_b10g10r10a2_uint;

   case MESA_FORMAT_R10G10B10A2_UINT:
      return pack_ubyte_r10g10b10a2_uint;

   case MESA_FORMAT_A2B10G10R10_UINT:
      return pack_ubyte_a2b10g10r10_uint;

   case MESA_FORMAT_A2R10G10B10_UINT:
      return pack_ubyte_a2r10g10b10_uint;

   case MESA_FORMAT_A_UINT8:
      return pack_ubyte_a_uint8;

   case MESA_FORMAT_A_UINT16:
      return pack_ubyte_a_uint16;

   case MESA_FORMAT_A_UINT32:
      return pack_ubyte_a_uint32;

   case MESA_FORMAT_A_SINT8:
      return pack_ubyte_a_sint8;

   case MESA_FORMAT_A_SINT16:
      return pack_ubyte_a_sint16;

   case MESA_FORMAT_A_SINT32:
      return pack_ubyte_a_sint32;

   case MESA_FORMAT_I_UINT8:
      return pack_ubyte_i_uint8;

   case MESA_FORMAT_I_UINT16:
      return pack_ubyte_i_uint16;

   case MESA_FORMAT_I_UINT32:
      return pack_ubyte_i_uint32;

   case MESA_FORMAT_I_SINT8:
      return pack_ubyte_i_sint8;

   case MESA_FORMAT_I_SINT16:
      return pack_ubyte_i_sint16;

   case MESA_FORMAT_I_SINT32:
      return pack_ubyte_i_sint32;

   case MESA_FORMAT_L_UINT8:
      return pack_ubyte_l_uint8;

   case MESA_FORMAT_L_UINT16:
      return pack_ubyte_l_uint16;

   case MESA_FORMAT_L_UINT32:
      return pack_ubyte_l_uint32;

   case MESA_FORMAT_L_SINT8:
      return pack_ubyte_l_sint8;

   case MESA_FORMAT_L_SINT16:
      return pack_ubyte_l_sint16;

   case MESA_FORMAT_L_SINT32:
      return pack_ubyte_l_sint32;

   case MESA_FORMAT_LA_UINT8:
      return pack_ubyte_la_uint8;

   case MESA_FORMAT_LA_UINT16:
      return pack_ubyte_la_uint16;

   case MESA_FORMAT_LA_UINT32:
      return pack_ubyte_la_uint32;

   case MESA_FORMAT_LA_SINT8:
      return pack_ubyte_la_sint8;

   case MESA_FORMAT_LA_SINT16:
      return pack_ubyte_la_sint16;

   case MESA_FORMAT_LA_SINT32:
      return pack_ubyte_la_sint32;

   case MESA_FORMAT_R_UINT8:
      return pack_ubyte_r_uint8;

   case MESA_FORMAT_R_UINT16:
      return pack_ubyte_r_uint16;

   case MESA_FORMAT_R_UINT32:
      return pack_ubyte_r_uint32;

   case MESA_FORMAT_R_SINT8:
      return pack_ubyte_r_sint8;

   case MESA_FORMAT_R_SINT16:
      return pack_ubyte_r_sint16;

   case MESA_FORMAT_R_SINT32:
      return pack_ubyte_r_sint32;

   case MESA_FORMAT_RG_UINT8:
      return pack_ubyte_rg_uint8;

   case MESA_FORMAT_RG_UINT16:
      return pack_ubyte_rg_uint16;

   case MESA_FORMAT_RG_UINT32:
      return pack_ubyte_rg_uint32;

   case MESA_FORMAT_RG_SINT8:
      return pack_ubyte_rg_sint8;

   case MESA_FORMAT_RG_SINT16:
      return pack_ubyte_rg_sint16;

   case MESA_FORMAT_RG_SINT32:
      return pack_ubyte_rg_sint32;

   case MESA_FORMAT_RGB_UINT8:
      return pack_ubyte_rgb_uint8;

   case MESA_FORMAT_RGB_UINT16:
      return pack_ubyte_rgb_uint16;

   case MESA_FORMAT_RGB_UINT32:
      return pack_ubyte_rgb_uint32;

   case MESA_FORMAT_RGB_SINT8:
      return pack_ubyte_rgb_sint8;

   case MESA_FORMAT_RGB_SINT16:
      return pack_ubyte_rgb_sint16;

   case MESA_FORMAT_RGB_SINT32:
      return pack_ubyte_rgb_sint32;

   case MESA_FORMAT_RGBA_UINT8:
      return pack_ubyte_rgba_uint8;

   case MESA_FORMAT_RGBA_UINT16:
      return pack_ubyte_rgba_uint16;

   case MESA_FORMAT_RGBA_UINT32:
      return pack_ubyte_rgba_uint32;

   case MESA_FORMAT_RGBA_SINT8:
      return pack_ubyte_rgba_sint8;

   case MESA_FORMAT_RGBA_SINT16:
      return pack_ubyte_rgba_sint16;

   case MESA_FORMAT_RGBA_SINT32:
      return pack_ubyte_rgba_sint32;

   case MESA_FORMAT_RGBX_UINT8:
      return pack_ubyte_rgbx_uint8;

   case MESA_FORMAT_RGBX_UINT16:
      return pack_ubyte_rgbx_uint16;

   case MESA_FORMAT_RGBX_UINT32:
      return pack_ubyte_rgbx_uint32;

   case MESA_FORMAT_RGBX_SINT8:
      return pack_ubyte_rgbx_sint8;

   case MESA_FORMAT_RGBX_SINT16:
      return pack_ubyte_rgbx_sint16;

   case MESA_FORMAT_RGBX_SINT32:
      return pack_ubyte_rgbx_sint32;
                                                                                                                                                                                                         default:
      return NULL;
   }
}

/**
 * Return a function that can pack a GLfloat rgba[4] color.
 */
gl_pack_float_rgba_func
_mesa_get_pack_float_rgba_function(mesa_format format)
{
   switch (format) {

   case MESA_FORMAT_A8B8G8R8_UNORM:
      return pack_float_a8b8g8r8_unorm;

   case MESA_FORMAT_X8B8G8R8_UNORM:
      return pack_float_x8b8g8r8_unorm;

   case MESA_FORMAT_R8G8B8A8_UNORM:
      return pack_float_r8g8b8a8_unorm;

   case MESA_FORMAT_R8G8B8X8_UNORM:
      return pack_float_r8g8b8x8_unorm;

   case MESA_FORMAT_B8G8R8A8_UNORM:
      return pack_float_b8g8r8a8_unorm;

   case MESA_FORMAT_B8G8R8X8_UNORM:
      return pack_float_b8g8r8x8_unorm;

   case MESA_FORMAT_A8R8G8B8_UNORM:
      return pack_float_a8r8g8b8_unorm;

   case MESA_FORMAT_X8R8G8B8_UNORM:
      return pack_float_x8r8g8b8_unorm;

   case MESA_FORMAT_L16A16_UNORM:
      return pack_float_l16a16_unorm;

   case MESA_FORMAT_A16L16_UNORM:
      return pack_float_a16l16_unorm;

   case MESA_FORMAT_B5G6R5_UNORM:
      return pack_float_b5g6r5_unorm;

   case MESA_FORMAT_R5G6B5_UNORM:
      return pack_float_r5g6b5_unorm;

   case MESA_FORMAT_B4G4R4A4_UNORM:
      return pack_float_b4g4r4a4_unorm;

   case MESA_FORMAT_B4G4R4X4_UNORM:
      return pack_float_b4g4r4x4_unorm;

   case MESA_FORMAT_A4R4G4B4_UNORM:
      return pack_float_a4r4g4b4_unorm;

   case MESA_FORMAT_A1B5G5R5_UNORM:
      return pack_float_a1b5g5r5_unorm;

   case MESA_FORMAT_B5G5R5A1_UNORM:
      return pack_float_b5g5r5a1_unorm;

   case MESA_FORMAT_B5G5R5X1_UNORM:
      return pack_float_b5g5r5x1_unorm;

   case MESA_FORMAT_A1R5G5B5_UNORM:
      return pack_float_a1r5g5b5_unorm;

   case MESA_FORMAT_L8A8_UNORM:
      return pack_float_l8a8_unorm;

   case MESA_FORMAT_A8L8_UNORM:
      return pack_float_a8l8_unorm;

   case MESA_FORMAT_R8G8_UNORM:
      return pack_float_r8g8_unorm;

   case MESA_FORMAT_G8R8_UNORM:
      return pack_float_g8r8_unorm;

   case MESA_FORMAT_L4A4_UNORM:
      return pack_float_l4a4_unorm;

   case MESA_FORMAT_B2G3R3_UNORM:
      return pack_float_b2g3r3_unorm;

   case MESA_FORMAT_R16G16_UNORM:
      return pack_float_r16g16_unorm;

   case MESA_FORMAT_G16R16_UNORM:
      return pack_float_g16r16_unorm;

   case MESA_FORMAT_B10G10R10A2_UNORM:
      return pack_float_b10g10r10a2_unorm;

   case MESA_FORMAT_B10G10R10X2_UNORM:
      return pack_float_b10g10r10x2_unorm;

   case MESA_FORMAT_R10G10B10A2_UNORM:
      return pack_float_r10g10b10a2_unorm;

   case MESA_FORMAT_R10G10B10X2_UNORM:
      return pack_float_r10g10b10x2_unorm;

   case MESA_FORMAT_R3G3B2_UNORM:
      return pack_float_r3g3b2_unorm;

   case MESA_FORMAT_A4B4G4R4_UNORM:
      return pack_float_a4b4g4r4_unorm;

   case MESA_FORMAT_R4G4B4A4_UNORM:
      return pack_float_r4g4b4a4_unorm;

   case MESA_FORMAT_R5G5B5A1_UNORM:
      return pack_float_r5g5b5a1_unorm;

   case MESA_FORMAT_A2B10G10R10_UNORM:
      return pack_float_a2b10g10r10_unorm;

   case MESA_FORMAT_A2R10G10B10_UNORM:
      return pack_float_a2r10g10b10_unorm;

   case MESA_FORMAT_A_UNORM8:
      return pack_float_a_unorm8;

   case MESA_FORMAT_A_UNORM16:
      return pack_float_a_unorm16;

   case MESA_FORMAT_L_UNORM8:
      return pack_float_l_unorm8;

   case MESA_FORMAT_L_UNORM16:
      return pack_float_l_unorm16;

   case MESA_FORMAT_I_UNORM8:
      return pack_float_i_unorm8;

   case MESA_FORMAT_I_UNORM16:
      return pack_float_i_unorm16;

   case MESA_FORMAT_R_UNORM8:
      return pack_float_r_unorm8;

   case MESA_FORMAT_R_UNORM16:
      return pack_float_r_unorm16;

   case MESA_FORMAT_BGR_UNORM8:
      return pack_float_bgr_unorm8;

   case MESA_FORMAT_RGB_UNORM8:
      return pack_float_rgb_unorm8;

   case MESA_FORMAT_RGBA_UNORM16:
      return pack_float_rgba_unorm16;

   case MESA_FORMAT_RGBX_UNORM16:
      return pack_float_rgbx_unorm16;

   case MESA_FORMAT_A8B8G8R8_SNORM:
      return pack_float_a8b8g8r8_snorm;

   case MESA_FORMAT_X8B8G8R8_SNORM:
      return pack_float_x8b8g8r8_snorm;

   case MESA_FORMAT_R8G8B8A8_SNORM:
      return pack_float_r8g8b8a8_snorm;

   case MESA_FORMAT_R8G8B8X8_SNORM:
      return pack_float_r8g8b8x8_snorm;

   case MESA_FORMAT_R16G16_SNORM:
      return pack_float_r16g16_snorm;

   case MESA_FORMAT_G16R16_SNORM:
      return pack_float_g16r16_snorm;

   case MESA_FORMAT_R8G8_SNORM:
      return pack_float_r8g8_snorm;

   case MESA_FORMAT_G8R8_SNORM:
      return pack_float_g8r8_snorm;

   case MESA_FORMAT_L8A8_SNORM:
      return pack_float_l8a8_snorm;

   case MESA_FORMAT_A8L8_SNORM:
      return pack_float_a8l8_snorm;

   case MESA_FORMAT_A_SNORM8:
      return pack_float_a_snorm8;

   case MESA_FORMAT_A_SNORM16:
      return pack_float_a_snorm16;

   case MESA_FORMAT_L_SNORM8:
      return pack_float_l_snorm8;

   case MESA_FORMAT_L_SNORM16:
      return pack_float_l_snorm16;

   case MESA_FORMAT_I_SNORM8:
      return pack_float_i_snorm8;

   case MESA_FORMAT_I_SNORM16:
      return pack_float_i_snorm16;

   case MESA_FORMAT_R_SNORM8:
      return pack_float_r_snorm8;

   case MESA_FORMAT_R_SNORM16:
      return pack_float_r_snorm16;

   case MESA_FORMAT_LA_SNORM16:
      return pack_float_la_snorm16;

   case MESA_FORMAT_RGB_SNORM16:
      return pack_float_rgb_snorm16;

   case MESA_FORMAT_RGBA_SNORM16:
      return pack_float_rgba_snorm16;

   case MESA_FORMAT_RGBX_SNORM16:
      return pack_float_rgbx_snorm16;

   case MESA_FORMAT_A8B8G8R8_SRGB:
      return pack_float_a8b8g8r8_srgb;

   case MESA_FORMAT_B8G8R8A8_SRGB:
      return pack_float_b8g8r8a8_srgb;

   case MESA_FORMAT_A8R8G8B8_SRGB:
      return pack_float_a8r8g8b8_srgb;

   case MESA_FORMAT_B8G8R8X8_SRGB:
      return pack_float_b8g8r8x8_srgb;

   case MESA_FORMAT_X8R8G8B8_SRGB:
      return pack_float_x8r8g8b8_srgb;

   case MESA_FORMAT_R8G8B8A8_SRGB:
      return pack_float_r8g8b8a8_srgb;

   case MESA_FORMAT_R8G8B8X8_SRGB:
      return pack_float_r8g8b8x8_srgb;

   case MESA_FORMAT_X8B8G8R8_SRGB:
      return pack_float_x8b8g8r8_srgb;

   case MESA_FORMAT_L8A8_SRGB:
      return pack_float_l8a8_srgb;

   case MESA_FORMAT_A8L8_SRGB:
      return pack_float_a8l8_srgb;

   case MESA_FORMAT_L_SRGB8:
      return pack_float_l_srgb8;

   case MESA_FORMAT_BGR_SRGB8:
      return pack_float_bgr_srgb8;

   case MESA_FORMAT_R9G9B9E5_FLOAT:
      return pack_float_r9g9b9e5_float;

   case MESA_FORMAT_R11G11B10_FLOAT:
      return pack_float_r11g11b10_float;

   case MESA_FORMAT_A_FLOAT16:
      return pack_float_a_float16;

   case MESA_FORMAT_A_FLOAT32:
      return pack_float_a_float32;

   case MESA_FORMAT_L_FLOAT16:
      return pack_float_l_float16;

   case MESA_FORMAT_L_FLOAT32:
      return pack_float_l_float32;

   case MESA_FORMAT_LA_FLOAT16:
      return pack_float_la_float16;

   case MESA_FORMAT_LA_FLOAT32:
      return pack_float_la_float32;

   case MESA_FORMAT_I_FLOAT16:
      return pack_float_i_float16;

   case MESA_FORMAT_I_FLOAT32:
      return pack_float_i_float32;

   case MESA_FORMAT_R_FLOAT16:
      return pack_float_r_float16;

   case MESA_FORMAT_R_FLOAT32:
      return pack_float_r_float32;

   case MESA_FORMAT_RG_FLOAT16:
      return pack_float_rg_float16;

   case MESA_FORMAT_RG_FLOAT32:
      return pack_float_rg_float32;

   case MESA_FORMAT_RGB_FLOAT16:
      return pack_float_rgb_float16;

   case MESA_FORMAT_RGB_FLOAT32:
      return pack_float_rgb_float32;

   case MESA_FORMAT_RGBA_FLOAT16:
      return pack_float_rgba_float16;

   case MESA_FORMAT_RGBA_FLOAT32:
      return pack_float_rgba_float32;

   case MESA_FORMAT_RGBX_FLOAT16:
      return pack_float_rgbx_float16;

   case MESA_FORMAT_RGBX_FLOAT32:
      return pack_float_rgbx_float32;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     default:
      return NULL;
   }
}

/**
 * Pack a row of GLubyte rgba[4] values to the destination.
 */
void
_mesa_pack_ubyte_rgba_row(mesa_format format, GLuint n,
                          const GLubyte src[][4], void *dst)
{
   GLuint i;
   GLubyte *d = dst;

   switch (format) {

   case MESA_FORMAT_A8B8G8R8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a8b8g8r8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_X8B8G8R8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_x8b8g8r8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8A8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r8g8b8a8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8X8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r8g8b8x8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B8G8R8A8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b8g8r8a8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B8G8R8X8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b8g8r8x8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A8R8G8B8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a8r8g8b8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_X8R8G8B8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_x8r8g8b8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_L16A16_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l16a16_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A16L16_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a16l16_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B5G6R5_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b5g6r5_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R5G6B5_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r5g6b5_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_B4G4R4A4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b4g4r4a4_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_B4G4R4X4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b4g4r4x4_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A4R4G4B4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a4r4g4b4_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A1B5G5R5_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a1b5g5r5_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_B5G5R5A1_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b5g5r5a1_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_B5G5R5X1_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b5g5r5x1_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A1R5G5B5_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a1r5g5b5_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L8A8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l8a8_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A8L8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a8l8_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R8G8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r8g8_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_G8R8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_g8r8_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L4A4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l4a4_unorm(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_B2G3R3_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b2g3r3_unorm(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_R16G16_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r16g16_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_G16R16_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_g16r16_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B10G10R10A2_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b10g10r10a2_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B10G10R10X2_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b10g10r10x2_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R10G10B10A2_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r10g10b10a2_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R10G10B10X2_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r10g10b10x2_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R3G3B2_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r3g3b2_unorm(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_A4B4G4R4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a4b4g4r4_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R4G4B4A4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r4g4b4a4_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R5G5B5A1_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r5g5b5a1_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A2B10G10R10_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a2b10g10r10_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A2R10G10B10_UNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a2r10g10b10_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_unorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_A_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_unorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_unorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_L_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_unorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_I_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_unorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_I_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_unorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_unorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_R_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_unorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_BGR_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_bgr_unorm8(src[i], d);
         d += 3;
      }
      break;

   case MESA_FORMAT_RGB_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgb_unorm8(src[i], d);
         d += 3;
      }
      break;

   case MESA_FORMAT_RGBA_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgba_unorm16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBX_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgbx_unorm16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_A8B8G8R8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a8b8g8r8_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_X8B8G8R8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_x8b8g8r8_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8A8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r8g8b8a8_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8X8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r8g8b8x8_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R16G16_SNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r16g16_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_G16R16_SNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_g16r16_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r8g8_snorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_G8R8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_g8r8_snorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L8A8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l8a8_snorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A8L8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a8l8_snorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A_SNORM8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_snorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_A_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_snorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_SNORM8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_snorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_L_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_snorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_I_SNORM8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_snorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_I_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_snorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R_SNORM8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_snorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_R_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_snorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_LA_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_la_snorm16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RGB_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgb_snorm16(src[i], d);
         d += 6;
      }
      break;

   case MESA_FORMAT_RGBA_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgba_snorm16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBX_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgbx_snorm16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_A8B8G8R8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a8b8g8r8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B8G8R8A8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b8g8r8a8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A8R8G8B8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a8r8g8b8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B8G8R8X8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b8g8r8x8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_X8R8G8B8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_ubyte_x8r8g8b8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8A8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r8g8b8a8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8X8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r8g8b8x8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_X8B8G8R8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_ubyte_x8b8g8r8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_L8A8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l8a8_srgb(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A8L8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a8l8_srgb(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_SRGB8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_srgb8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_BGR_SRGB8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_bgr_srgb8(src[i], d);
         d += 3;
      }
      break;

   case MESA_FORMAT_R9G9B9E5_FLOAT:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r9g9b9e5_float(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R11G11B10_FLOAT:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r11g11b10_float(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_float16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_float32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_L_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_float16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_float32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_LA_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_la_float16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_LA_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_la_float32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_I_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_float16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_I_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_float32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_float16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_float32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RG_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rg_float16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RG_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rg_float32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGB_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgb_float16(src[i], d);
         d += 6;
      }
      break;

   case MESA_FORMAT_RGB_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgb_float32(src[i], d);
         d += 12;
      }
      break;

   case MESA_FORMAT_RGBA_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgba_float16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBA_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgba_float32(src[i], d);
         d += 16;
      }
      break;

   case MESA_FORMAT_RGBX_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgbx_float16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBX_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgbx_float32(src[i], d);
         d += 16;
      }
      break;

   case MESA_FORMAT_B10G10R10A2_UINT:
      for (i = 0; i < n; ++i) {
         pack_ubyte_b10g10r10a2_uint(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R10G10B10A2_UINT:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r10g10b10a2_uint(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A2B10G10R10_UINT:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a2b10g10r10_uint(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A2R10G10B10_UINT:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a2r10g10b10_uint(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A_UINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_uint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_A_UINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_uint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A_UINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_uint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A_SINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_sint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_A_SINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_sint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A_SINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_a_sint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_I_UINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_uint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_I_UINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_uint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_I_UINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_uint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_I_SINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_sint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_I_SINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_sint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_I_SINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_i_sint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_L_UINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_uint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_L_UINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_uint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_UINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_uint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_L_SINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_sint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_L_SINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_sint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_SINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_l_sint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_LA_UINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_la_uint8(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_LA_UINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_la_uint16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_LA_UINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_la_uint32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_LA_SINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_la_sint8(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_LA_SINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_la_sint16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_LA_SINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_la_sint32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_R_UINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_uint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_R_UINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_uint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R_UINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_uint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R_SINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_sint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_R_SINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_sint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R_SINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_r_sint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RG_UINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rg_uint8(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_RG_UINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rg_uint16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RG_UINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rg_uint32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RG_SINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rg_sint8(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_RG_SINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rg_sint16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RG_SINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rg_sint32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGB_UINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgb_uint8(src[i], d);
         d += 3;
      }
      break;

   case MESA_FORMAT_RGB_UINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgb_uint16(src[i], d);
         d += 6;
      }
      break;

   case MESA_FORMAT_RGB_UINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgb_uint32(src[i], d);
         d += 12;
      }
      break;

   case MESA_FORMAT_RGB_SINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgb_sint8(src[i], d);
         d += 3;
      }
      break;

   case MESA_FORMAT_RGB_SINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgb_sint16(src[i], d);
         d += 6;
      }
      break;

   case MESA_FORMAT_RGB_SINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgb_sint32(src[i], d);
         d += 12;
      }
      break;

   case MESA_FORMAT_RGBA_UINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgba_uint8(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RGBA_UINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgba_uint16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBA_UINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgba_uint32(src[i], d);
         d += 16;
      }
      break;

   case MESA_FORMAT_RGBA_SINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgba_sint8(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RGBA_SINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgba_sint16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBA_SINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgba_sint32(src[i], d);
         d += 16;
      }
      break;

   case MESA_FORMAT_RGBX_UINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgbx_uint8(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RGBX_UINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgbx_uint16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBX_UINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgbx_uint32(src[i], d);
         d += 16;
      }
      break;

   case MESA_FORMAT_RGBX_SINT8:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgbx_sint8(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RGBX_SINT16:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgbx_sint16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBX_SINT32:
      for (i = 0; i < n; ++i) {
         pack_ubyte_rgbx_sint32(src[i], d);
         d += 16;
      }
      break;
                                                                                                                                                                                                         default:
      assert(!"Invalid format");
   }
}

/**
 * Pack a row of GLuint rgba[4] values to the destination.
 */
void
_mesa_pack_uint_rgba_row(mesa_format format, GLuint n,
                          const GLuint src[][4], void *dst)
{
   GLuint i;
   GLubyte *d = dst;

   switch (format) {
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
   case MESA_FORMAT_B10G10R10A2_UINT:
      for (i = 0; i < n; ++i) {
         pack_uint_b10g10r10a2_uint(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R10G10B10A2_UINT:
      for (i = 0; i < n; ++i) {
         pack_uint_r10g10b10a2_uint(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A2B10G10R10_UINT:
      for (i = 0; i < n; ++i) {
         pack_uint_a2b10g10r10_uint(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A2R10G10B10_UINT:
      for (i = 0; i < n; ++i) {
         pack_uint_a2r10g10b10_uint(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A_UINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_a_uint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_A_UINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_a_uint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A_UINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_a_uint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A_SINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_a_sint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_A_SINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_a_sint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A_SINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_a_sint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_I_UINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_i_uint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_I_UINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_i_uint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_I_UINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_i_uint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_I_SINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_i_sint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_I_SINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_i_sint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_I_SINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_i_sint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_L_UINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_l_uint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_L_UINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_l_uint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_UINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_l_uint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_L_SINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_l_sint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_L_SINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_l_sint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_SINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_l_sint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_LA_UINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_la_uint8(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_LA_UINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_la_uint16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_LA_UINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_la_uint32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_LA_SINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_la_sint8(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_LA_SINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_la_sint16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_LA_SINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_la_sint32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_R_UINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_r_uint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_R_UINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_r_uint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R_UINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_r_uint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R_SINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_r_sint8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_R_SINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_r_sint16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R_SINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_r_sint32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RG_UINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_rg_uint8(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_RG_UINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_rg_uint16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RG_UINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_rg_uint32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RG_SINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_rg_sint8(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_RG_SINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_rg_sint16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RG_SINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_rg_sint32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGB_UINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_rgb_uint8(src[i], d);
         d += 3;
      }
      break;

   case MESA_FORMAT_RGB_UINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_rgb_uint16(src[i], d);
         d += 6;
      }
      break;

   case MESA_FORMAT_RGB_UINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_rgb_uint32(src[i], d);
         d += 12;
      }
      break;

   case MESA_FORMAT_RGB_SINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_rgb_sint8(src[i], d);
         d += 3;
      }
      break;

   case MESA_FORMAT_RGB_SINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_rgb_sint16(src[i], d);
         d += 6;
      }
      break;

   case MESA_FORMAT_RGB_SINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_rgb_sint32(src[i], d);
         d += 12;
      }
      break;

   case MESA_FORMAT_RGBA_UINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_rgba_uint8(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RGBA_UINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_rgba_uint16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBA_UINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_rgba_uint32(src[i], d);
         d += 16;
      }
      break;

   case MESA_FORMAT_RGBA_SINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_rgba_sint8(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RGBA_SINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_rgba_sint16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBA_SINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_rgba_sint32(src[i], d);
         d += 16;
      }
      break;

   case MESA_FORMAT_RGBX_UINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_rgbx_uint8(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RGBX_UINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_rgbx_uint16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBX_UINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_rgbx_uint32(src[i], d);
         d += 16;
      }
      break;

   case MESA_FORMAT_RGBX_SINT8:
      for (i = 0; i < n; ++i) {
         pack_uint_rgbx_sint8(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RGBX_SINT16:
      for (i = 0; i < n; ++i) {
         pack_uint_rgbx_sint16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBX_SINT32:
      for (i = 0; i < n; ++i) {
         pack_uint_rgbx_sint32(src[i], d);
         d += 16;
      }
      break;
                                                                                                                                                                                                         default:
      assert(!"Invalid format");
   }
}

/**
 * Pack a row of GLfloat rgba[4] values to the destination.
 */
void
_mesa_pack_float_rgba_row(mesa_format format, GLuint n,
                          const GLfloat src[][4], void *dst)
{
   GLuint i;
   GLubyte *d = dst;

   switch (format) {

   case MESA_FORMAT_A8B8G8R8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a8b8g8r8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_X8B8G8R8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_x8b8g8r8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8A8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r8g8b8a8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8X8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r8g8b8x8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B8G8R8A8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_b8g8r8a8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B8G8R8X8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_b8g8r8x8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A8R8G8B8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a8r8g8b8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_X8R8G8B8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_x8r8g8b8_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_L16A16_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_l16a16_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A16L16_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a16l16_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B5G6R5_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_b5g6r5_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R5G6B5_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r5g6b5_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_B4G4R4A4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_b4g4r4a4_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_B4G4R4X4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_b4g4r4x4_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A4R4G4B4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a4r4g4b4_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A1B5G5R5_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a1b5g5r5_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_B5G5R5A1_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_b5g5r5a1_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_B5G5R5X1_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_b5g5r5x1_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A1R5G5B5_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a1r5g5b5_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L8A8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_l8a8_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A8L8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a8l8_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R8G8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r8g8_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_G8R8_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_g8r8_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L4A4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_l4a4_unorm(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_B2G3R3_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_b2g3r3_unorm(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_R16G16_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r16g16_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_G16R16_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_g16r16_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B10G10R10A2_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_b10g10r10a2_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B10G10R10X2_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_b10g10r10x2_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R10G10B10A2_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r10g10b10a2_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R10G10B10X2_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r10g10b10x2_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R3G3B2_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r3g3b2_unorm(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_A4B4G4R4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a4b4g4r4_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R4G4B4A4_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r4g4b4a4_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R5G5B5A1_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r5g5b5a1_unorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A2B10G10R10_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a2b10g10r10_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A2R10G10B10_UNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a2r10g10b10_unorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_float_a_unorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_A_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_a_unorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_float_l_unorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_L_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_l_unorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_I_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_float_i_unorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_I_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_i_unorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_float_r_unorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_R_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_r_unorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_BGR_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_float_bgr_unorm8(src[i], d);
         d += 3;
      }
      break;

   case MESA_FORMAT_RGB_UNORM8:
      for (i = 0; i < n; ++i) {
         pack_float_rgb_unorm8(src[i], d);
         d += 3;
      }
      break;

   case MESA_FORMAT_RGBA_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_rgba_unorm16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBX_UNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_rgbx_unorm16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_A8B8G8R8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a8b8g8r8_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_X8B8G8R8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_float_x8b8g8r8_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8A8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r8g8b8a8_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8X8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r8g8b8x8_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R16G16_SNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r16g16_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_G16R16_SNORM:
      for (i = 0; i < n; ++i) {
         pack_float_g16r16_snorm(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_float_r8g8_snorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_G8R8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_float_g8r8_snorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L8A8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_float_l8a8_snorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A8L8_SNORM:
      for (i = 0; i < n; ++i) {
         pack_float_a8l8_snorm(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A_SNORM8:
      for (i = 0; i < n; ++i) {
         pack_float_a_snorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_A_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_a_snorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_SNORM8:
      for (i = 0; i < n; ++i) {
         pack_float_l_snorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_L_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_l_snorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_I_SNORM8:
      for (i = 0; i < n; ++i) {
         pack_float_i_snorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_I_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_i_snorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R_SNORM8:
      for (i = 0; i < n; ++i) {
         pack_float_r_snorm8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_R_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_r_snorm16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_LA_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_la_snorm16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RGB_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_rgb_snorm16(src[i], d);
         d += 6;
      }
      break;

   case MESA_FORMAT_RGBA_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_rgba_snorm16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBX_SNORM16:
      for (i = 0; i < n; ++i) {
         pack_float_rgbx_snorm16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_A8B8G8R8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_float_a8b8g8r8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B8G8R8A8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_float_b8g8r8a8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A8R8G8B8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_float_a8r8g8b8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_B8G8R8X8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_float_b8g8r8x8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_X8R8G8B8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_float_x8r8g8b8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8A8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_float_r8g8b8a8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R8G8B8X8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_float_r8g8b8x8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_X8B8G8R8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_float_x8b8g8r8_srgb(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_L8A8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_float_l8a8_srgb(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A8L8_SRGB:
      for (i = 0; i < n; ++i) {
         pack_float_a8l8_srgb(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_SRGB8:
      for (i = 0; i < n; ++i) {
         pack_float_l_srgb8(src[i], d);
         d += 1;
      }
      break;

   case MESA_FORMAT_BGR_SRGB8:
      for (i = 0; i < n; ++i) {
         pack_float_bgr_srgb8(src[i], d);
         d += 3;
      }
      break;

   case MESA_FORMAT_R9G9B9E5_FLOAT:
      for (i = 0; i < n; ++i) {
         pack_float_r9g9b9e5_float(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R11G11B10_FLOAT:
      for (i = 0; i < n; ++i) {
         pack_float_r11g11b10_float(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_A_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_float_a_float16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_A_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_float_a_float32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_L_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_float_l_float16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_L_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_float_l_float32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_LA_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_float_la_float16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_LA_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_float_la_float32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_I_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_float_i_float16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_I_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_float_i_float32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_R_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_float_r_float16(src[i], d);
         d += 2;
      }
      break;

   case MESA_FORMAT_R_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_float_r_float32(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RG_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_float_rg_float16(src[i], d);
         d += 4;
      }
      break;

   case MESA_FORMAT_RG_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_float_rg_float32(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGB_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_float_rgb_float16(src[i], d);
         d += 6;
      }
      break;

   case MESA_FORMAT_RGB_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_float_rgb_float32(src[i], d);
         d += 12;
      }
      break;

   case MESA_FORMAT_RGBA_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_float_rgba_float16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBA_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_float_rgba_float32(src[i], d);
         d += 16;
      }
      break;

   case MESA_FORMAT_RGBX_FLOAT16:
      for (i = 0; i < n; ++i) {
         pack_float_rgbx_float16(src[i], d);
         d += 8;
      }
      break;

   case MESA_FORMAT_RGBX_FLOAT32:
      for (i = 0; i < n; ++i) {
         pack_float_rgbx_float32(src[i], d);
         d += 16;
      }
      break;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     default:
      assert(!"Invalid format");
   }
}

/**
 * Pack a 2D image of ubyte RGBA pixels in the given format.
 * \param srcRowStride  source image row stride in bytes
 * \param dstRowStride  destination image row stride in bytes
 */
void
_mesa_pack_ubyte_rgba_rect(mesa_format format, GLuint width, GLuint height,
                           const GLubyte *src, GLint srcRowStride,
                           void *dst, GLint dstRowStride)
{
   GLubyte *dstUB = dst;
   GLuint i;

   if (srcRowStride == width * 4 * sizeof(GLubyte) &&
       dstRowStride == _mesa_format_row_stride(format, width)) {
      /* do whole image at once */
      _mesa_pack_ubyte_rgba_row(format, width * height,
                                (const GLubyte (*)[4]) src, dst);
   }
   else {
      /* row by row */
      for (i = 0; i < height; i++) {
         _mesa_pack_ubyte_rgba_row(format, width,
                                   (const GLubyte (*)[4]) src, dstUB);
         src += srcRowStride;
         dstUB += dstRowStride;
      }
   }
}


/** Helper struct for MESA_FORMAT_Z32_FLOAT_S8X24_UINT */
struct z32f_x24s8
{
   float z;
   uint32_t x24s8;
};


/**
 ** Pack float Z pixels
 **/

static void
pack_float_S8_UINT_Z24_UNORM(const GLfloat *src, void *dst)
{
   /* don't disturb the stencil values */
   GLuint *d = ((GLuint *) dst);
   const GLdouble scale = (GLdouble) 0xffffff;
   GLuint s = *d & 0xff;
   GLuint z = (GLuint) (*src * scale);
   assert(z <= 0xffffff);
   *d = (z << 8) | s;
}

static void
pack_float_Z24_UNORM_S8_UINT(const GLfloat *src, void *dst)
{
   /* don't disturb the stencil values */
   GLuint *d = ((GLuint *) dst);
   const GLdouble scale = (GLdouble) 0xffffff;
   GLuint s = *d & 0xff000000;
   GLuint z = (GLuint) (*src * scale);
   assert(z <= 0xffffff);
   *d = s | z;
}

static void
pack_float_Z_UNORM16(const GLfloat *src, void *dst)
{
   GLushort *d = ((GLushort *) dst);
   const GLfloat scale = (GLfloat) 0xffff;
   *d = (GLushort) (*src * scale);
}

static void
pack_float_Z_UNORM32(const GLfloat *src, void *dst)
{
   GLuint *d = ((GLuint *) dst);
   const GLdouble scale = (GLdouble) 0xffffffff;
   *d = (GLuint) (*src * scale);
}

static void
pack_float_Z_FLOAT32(const GLfloat *src, void *dst)
{
   GLfloat *d = (GLfloat *) dst;
   *d = *src;
}

gl_pack_float_z_func
_mesa_get_pack_float_z_func(mesa_format format)
{
   switch (format) {
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
   case MESA_FORMAT_X8_UINT_Z24_UNORM:
      return pack_float_S8_UINT_Z24_UNORM;
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
   case MESA_FORMAT_Z24_UNORM_X8_UINT:
      return pack_float_Z24_UNORM_S8_UINT;
   case MESA_FORMAT_Z_UNORM16:
      return pack_float_Z_UNORM16;
   case MESA_FORMAT_Z_UNORM32:
      return pack_float_Z_UNORM32;
   case MESA_FORMAT_Z_FLOAT32:
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      return pack_float_Z_FLOAT32;
   default:
      _mesa_problem(NULL,
                    "unexpected format in _mesa_get_pack_float_z_func()");
      return NULL;
   }
}



/**
 ** Pack uint Z pixels.  The incoming src value is always in
 ** the range [0, 2^32-1].
 **/

static void
pack_uint_S8_UINT_Z24_UNORM(const GLuint *src, void *dst)
{
   /* don't disturb the stencil values */
   GLuint *d = ((GLuint *) dst);
   GLuint s = *d & 0xff;
   GLuint z = *src & 0xffffff00;
   *d = z | s;
}

static void
pack_uint_Z24_UNORM_S8_UINT(const GLuint *src, void *dst)
{
   /* don't disturb the stencil values */
   GLuint *d = ((GLuint *) dst);
   GLuint s = *d & 0xff000000;
   GLuint z = *src >> 8;
   *d = s | z;
}

static void
pack_uint_Z_UNORM16(const GLuint *src, void *dst)
{
   GLushort *d = ((GLushort *) dst);
   *d = *src >> 16;
}

static void
pack_uint_Z_UNORM32(const GLuint *src, void *dst)
{
   GLuint *d = ((GLuint *) dst);
   *d = *src;
}

static void
pack_uint_Z_FLOAT32(const GLuint *src, void *dst)
{
   GLuint *d = ((GLuint *) dst);
   const GLdouble scale = 1.0 / (GLdouble) 0xffffffff;
   *d = (GLuint) (*src * scale);
   assert(*d >= 0.0f);
   assert(*d <= 1.0f);
}

static void
pack_uint_Z_FLOAT32_X24S8(const GLuint *src, void *dst)
{
   GLfloat *d = ((GLfloat *) dst);
   const GLdouble scale = 1.0 / (GLdouble) 0xffffffff;
   *d = (GLfloat) (*src * scale);
   assert(*d >= 0.0f);
   assert(*d <= 1.0f);
}

gl_pack_uint_z_func
_mesa_get_pack_uint_z_func(mesa_format format)
{
   switch (format) {
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
   case MESA_FORMAT_X8_UINT_Z24_UNORM:
      return pack_uint_S8_UINT_Z24_UNORM;
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
   case MESA_FORMAT_Z24_UNORM_X8_UINT:
      return pack_uint_Z24_UNORM_S8_UINT;
   case MESA_FORMAT_Z_UNORM16:
      return pack_uint_Z_UNORM16;
   case MESA_FORMAT_Z_UNORM32:
      return pack_uint_Z_UNORM32;
   case MESA_FORMAT_Z_FLOAT32:
      return pack_uint_Z_FLOAT32;
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      return pack_uint_Z_FLOAT32_X24S8;
   default:
      _mesa_problem(NULL, "unexpected format in _mesa_get_pack_uint_z_func()");
      return NULL;
   }
}


/**
 ** Pack ubyte stencil pixels
 **/

static void
pack_ubyte_stencil_Z24_S8(const GLubyte *src, void *dst)
{
   /* don't disturb the Z values */
   GLuint *d = ((GLuint *) dst);
   GLuint s = *src;
   GLuint z = *d & 0xffffff00;
   *d = z | s;
}

static void
pack_ubyte_stencil_S8_Z24(const GLubyte *src, void *dst)
{
   /* don't disturb the Z values */
   GLuint *d = ((GLuint *) dst);
   GLuint s = *src << 24;
   GLuint z = *d & 0xffffff;
   *d = s | z;
}

static void
pack_ubyte_stencil_S8(const GLubyte *src, void *dst)
{
   GLubyte *d = (GLubyte *) dst;
   *d = *src;
}

static void
pack_ubyte_stencil_Z32_FLOAT_X24S8(const GLubyte *src, void *dst)
{
   GLfloat *d = ((GLfloat *) dst);
   d[1] = *src;
}


gl_pack_ubyte_stencil_func
_mesa_get_pack_ubyte_stencil_func(mesa_format format)
{
   switch (format) {
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
      return pack_ubyte_stencil_Z24_S8;
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
      return pack_ubyte_stencil_S8_Z24;
   case MESA_FORMAT_S_UINT8:
      return pack_ubyte_stencil_S8;
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      return pack_ubyte_stencil_Z32_FLOAT_X24S8;
   default:
      _mesa_problem(NULL,
                    "unexpected format in _mesa_pack_ubyte_stencil_func()");
      return NULL;
   }
}



void
_mesa_pack_float_z_row(mesa_format format, GLuint n,
                       const GLfloat *src, void *dst)
{
   switch (format) {
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
   case MESA_FORMAT_X8_UINT_Z24_UNORM:
      {
         /* don't disturb the stencil values */
         GLuint *d = ((GLuint *) dst);
         const GLdouble scale = (GLdouble) 0xffffff;
         GLuint i;
         for (i = 0; i < n; i++) {
            GLuint s = d[i] & 0xff;
            GLuint z = (GLuint) (src[i] * scale);
            assert(z <= 0xffffff);
            d[i] = (z << 8) | s;
         }
      }
      break;
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
   case MESA_FORMAT_Z24_UNORM_X8_UINT:
      {
         /* don't disturb the stencil values */
         GLuint *d = ((GLuint *) dst);
         const GLdouble scale = (GLdouble) 0xffffff;
         GLuint i;
         for (i = 0; i < n; i++) {
            GLuint s = d[i] & 0xff000000;
            GLuint z = (GLuint) (src[i] * scale);
            assert(z <= 0xffffff);
            d[i] = s | z;
         }
      }
      break;
   case MESA_FORMAT_Z_UNORM16:
      {
         GLushort *d = ((GLushort *) dst);
         const GLfloat scale = (GLfloat) 0xffff;
         GLuint i;
         for (i = 0; i < n; i++) {
            d[i] = (GLushort) (src[i] * scale);
         }
      }
      break;
   case MESA_FORMAT_Z_UNORM32:
      {
         GLuint *d = ((GLuint *) dst);
         const GLdouble scale = (GLdouble) 0xffffffff;
         GLuint i;
         for (i = 0; i < n; i++) {
            d[i] = (GLuint) (src[i] * scale);
         }
      }
      break;
   case MESA_FORMAT_Z_FLOAT32:
      memcpy(dst, src, n * sizeof(GLfloat));
      break;
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      {
         struct z32f_x24s8 *d = (struct z32f_x24s8 *) dst;
         GLuint i;
         for (i = 0; i < n; i++) {
            d[i].z = src[i];
         }
      }
      break;
   default:
      _mesa_problem(NULL, "unexpected format in _mesa_pack_float_z_row()");
   }
}


/**
 * The incoming Z values are always in the range [0, 0xffffffff].
 */
void
_mesa_pack_uint_z_row(mesa_format format, GLuint n,
                      const GLuint *src, void *dst)
{
   switch (format) {
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
   case MESA_FORMAT_X8_UINT_Z24_UNORM:
      {
         /* don't disturb the stencil values */
         GLuint *d = ((GLuint *) dst);
         GLuint i;
         for (i = 0; i < n; i++) {
            GLuint s = d[i] & 0xff;
            GLuint z = src[i] & 0xffffff00;
            d[i] = z | s;
         }
      }
      break;
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
   case MESA_FORMAT_Z24_UNORM_X8_UINT:
      {
         /* don't disturb the stencil values */
         GLuint *d = ((GLuint *) dst);
         GLuint i;
         for (i = 0; i < n; i++) {
            GLuint s = d[i] & 0xff000000;
            GLuint z = src[i] >> 8;
            d[i] = s | z;
         }
      }
      break;
   case MESA_FORMAT_Z_UNORM16:
      {
         GLushort *d = ((GLushort *) dst);
         GLuint i;
         for (i = 0; i < n; i++) {
            d[i] = src[i] >> 16;
         }
      }
      break;
   case MESA_FORMAT_Z_UNORM32:
      memcpy(dst, src, n * sizeof(GLfloat));
      break;
   case MESA_FORMAT_Z_FLOAT32:
      {
         GLuint *d = ((GLuint *) dst);
         const GLdouble scale = 1.0 / (GLdouble) 0xffffffff;
         GLuint i;
         for (i = 0; i < n; i++) {
            d[i] = (GLuint) (src[i] * scale);
            assert(d[i] >= 0.0f);
            assert(d[i] <= 1.0f);
         }
      }
      break;
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      {
         struct z32f_x24s8 *d = (struct z32f_x24s8 *) dst;
         const GLdouble scale = 1.0 / (GLdouble) 0xffffffff;
         GLuint i;
         for (i = 0; i < n; i++) {
            d[i].z = (GLfloat) (src[i] * scale);
            assert(d[i].z >= 0.0f);
            assert(d[i].z <= 1.0f);
         }
      }
      break;
   default:
      _mesa_problem(NULL, "unexpected format in _mesa_pack_uint_z_row()");
   }
}


void
_mesa_pack_ubyte_stencil_row(mesa_format format, GLuint n,
                             const GLubyte *src, void *dst)
{
   switch (format) {
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
      {
         /* don't disturb the Z values */
         GLuint *d = ((GLuint *) dst);
         GLuint i;
         for (i = 0; i < n; i++) {
            GLuint s = src[i];
            GLuint z = d[i] & 0xffffff00;
            d[i] = z | s;
         }
      }
      break;
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
      {
         /* don't disturb the Z values */
         GLuint *d = ((GLuint *) dst);
         GLuint i;
         for (i = 0; i < n; i++) {
            GLuint s = src[i] << 24;
            GLuint z = d[i] & 0xffffff;
            d[i] = s | z;
         }
      }
      break;
   case MESA_FORMAT_S_UINT8:
      memcpy(dst, src, n * sizeof(GLubyte));
      break;
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      {
         struct z32f_x24s8 *d = (struct z32f_x24s8 *) dst;
         GLuint i;
         for (i = 0; i < n; i++) {
            d[i].x24s8 = src[i];
         }
      }
      break;
   default:
      _mesa_problem(NULL, "unexpected format in _mesa_pack_ubyte_stencil_row()");
   }
}


/**
 * Incoming Z/stencil values are always in uint_24_8 format.
 */
void
_mesa_pack_uint_24_8_depth_stencil_row(mesa_format format, GLuint n,
                                       const GLuint *src, void *dst)
{
   switch (format) {
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
      memcpy(dst, src, n * sizeof(GLuint));
      break;
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
      {
         GLuint *d = ((GLuint *) dst);
         GLuint i;
         for (i = 0; i < n; i++) {
            GLuint s = src[i] << 24;
            GLuint z = src[i] >> 8;
            d[i] = s | z;
         }
      }
      break;
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      {
         const GLdouble scale = 1.0 / (GLdouble) 0xffffff;
         struct z32f_x24s8 *d = (struct z32f_x24s8 *) dst;
         GLuint i;
         for (i = 0; i < n; i++) {
            GLfloat z = (GLfloat) ((src[i] >> 8) * scale);
            d[i].z = z;
            d[i].x24s8 = src[i];
         }
      }
      break;
   default:
      _mesa_problem(NULL, "bad format %s in _mesa_pack_ubyte_s_row",
                    _mesa_get_format_name(format));
      return;
   }
}



/**
 * Convert a boolean color mask to a packed color where each channel of
 * the packed value at dst will be 0 or ~0 depending on the colorMask.
 */
void
_mesa_pack_colormask(mesa_format format, const GLubyte colorMask[4], void *dst)
{
   GLfloat maskColor[4];

   switch (_mesa_get_format_datatype(format)) {
   case GL_UNSIGNED_NORMALIZED:
      /* simple: 1.0 will convert to ~0 in the right bit positions */
      maskColor[0] = colorMask[0] ? 1.0f : 0.0f;
      maskColor[1] = colorMask[1] ? 1.0f : 0.0f;
      maskColor[2] = colorMask[2] ? 1.0f : 0.0f;
      maskColor[3] = colorMask[3] ? 1.0f : 0.0f;
      _mesa_pack_float_rgba_row(format, 1,
                                (const GLfloat (*)[4]) maskColor, dst);
      break;
   case GL_SIGNED_NORMALIZED:
   case GL_FLOAT:
      /* These formats are harder because it's hard to know the floating
       * point values that will convert to ~0 for each color channel's bits.
       * This solution just generates a non-zero value for each color channel
       * then fixes up the non-zero values to be ~0.
       * Note: we'll need to add special case code if we ever have to deal
       * with formats with unequal color channel sizes, like R11_G11_B10.
       * We issue a warning below for channel sizes other than 8,16,32.
       */
      {
         GLuint bits = _mesa_get_format_max_bits(format); /* bits per chan */
         GLuint bytes = _mesa_get_format_bytes(format);
         GLuint i;

         /* this should put non-zero values into the channels of dst */
         maskColor[0] = colorMask[0] ? -1.0f : 0.0f;
         maskColor[1] = colorMask[1] ? -1.0f : 0.0f;
         maskColor[2] = colorMask[2] ? -1.0f : 0.0f;
         maskColor[3] = colorMask[3] ? -1.0f : 0.0f;
         _mesa_pack_float_rgba_row(format, 1,
                                   (const GLfloat (*)[4]) maskColor, dst);

         /* fix-up the dst channels by converting non-zero values to ~0 */
         if (bits == 8) {
            GLubyte *d = (GLubyte *) dst;
            for (i = 0; i < bytes; i++) {
               d[i] = d[i] ? 0xff : 0x0;
            }
         }
         else if (bits == 16) {
            GLushort *d = (GLushort *) dst;
            for (i = 0; i < bytes / 2; i++) {
               d[i] = d[i] ? 0xffff : 0x0;
            }
         }
         else if (bits == 32) {
            GLuint *d = (GLuint *) dst;
            for (i = 0; i < bytes / 4; i++) {
               d[i] = d[i] ? 0xffffffffU : 0x0;
            }
         }
         else {
            _mesa_problem(NULL, "unexpected size in _mesa_pack_colormask()");
            return;
         }
      }
      break;
   default:
      _mesa_problem(NULL, "unexpected format data type in gen_color_mask()");
      return;
   }
}

