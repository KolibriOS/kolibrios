/*
 * Copyright (C) 2014 Intel Corporation
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
 *    Jason Ekstrand (jason@jlekstrand.net)
 */

#include <math.h>
#include "main/core.h"
#include "util/rounding.h" /* for _mesa_roundeven */
#include "nir_constant_expressions.h"

#if defined(_MSC_VER) && (_MSC_VER < 1800)
static int isnormal(double x)
{
   return _fpclass(x) == _FPCLASS_NN || _fpclass(x) == _FPCLASS_PN;
}
#elif defined(__SUNPRO_CC)
#include <ieeefp.h>
static int isnormal(double x)
{
   return fpclass(x) == FP_NORMAL;
}
#endif

#if defined(_MSC_VER)
static double copysign(double x, double y)
{
   return _copysign(x, y);
}
#endif

/**
 * Evaluate one component of packSnorm4x8.
 */
static uint8_t
pack_snorm_1x8(float x)
{
    /* From section 8.4 of the GLSL 4.30 spec:
     *
     *    packSnorm4x8
     *    ------------
     *    The conversion for component c of v to fixed point is done as
     *    follows:
     *
     *      packSnorm4x8: round(clamp(c, -1, +1) * 127.0)
     *
     * We must first cast the float to an int, because casting a negative
     * float to a uint is undefined.
     */
   return (uint8_t) (int)
          _mesa_roundevenf(CLAMP(x, -1.0f, +1.0f) * 127.0f);
}

/**
 * Evaluate one component of packSnorm2x16.
 */
static uint16_t
pack_snorm_1x16(float x)
{
    /* From section 8.4 of the GLSL ES 3.00 spec:
     *
     *    packSnorm2x16
     *    -------------
     *    The conversion for component c of v to fixed point is done as
     *    follows:
     *
     *      packSnorm2x16: round(clamp(c, -1, +1) * 32767.0)
     *
     * We must first cast the float to an int, because casting a negative
     * float to a uint is undefined.
     */
   return (uint16_t) (int)
          _mesa_roundevenf(CLAMP(x, -1.0f, +1.0f) * 32767.0f);
}

/**
 * Evaluate one component of unpackSnorm4x8.
 */
static float
unpack_snorm_1x8(uint8_t u)
{
    /* From section 8.4 of the GLSL 4.30 spec:
     *
     *    unpackSnorm4x8
     *    --------------
     *    The conversion for unpacked fixed-point value f to floating point is
     *    done as follows:
     *
     *       unpackSnorm4x8: clamp(f / 127.0, -1, +1)
     */
   return CLAMP((int8_t) u / 127.0f, -1.0f, +1.0f);
}

/**
 * Evaluate one component of unpackSnorm2x16.
 */
static float
unpack_snorm_1x16(uint16_t u)
{
    /* From section 8.4 of the GLSL ES 3.00 spec:
     *
     *    unpackSnorm2x16
     *    ---------------
     *    The conversion for unpacked fixed-point value f to floating point is
     *    done as follows:
     *
     *       unpackSnorm2x16: clamp(f / 32767.0, -1, +1)
     */
   return CLAMP((int16_t) u / 32767.0f, -1.0f, +1.0f);
}

/**
 * Evaluate one component packUnorm4x8.
 */
static uint8_t
pack_unorm_1x8(float x)
{
    /* From section 8.4 of the GLSL 4.30 spec:
     *
     *    packUnorm4x8
     *    ------------
     *    The conversion for component c of v to fixed point is done as
     *    follows:
     *
     *       packUnorm4x8: round(clamp(c, 0, +1) * 255.0)
     */
   return (uint8_t) (int)
          _mesa_roundevenf(CLAMP(x, 0.0f, 1.0f) * 255.0f);
}

/**
 * Evaluate one component packUnorm2x16.
 */
static uint16_t
pack_unorm_1x16(float x)
{
    /* From section 8.4 of the GLSL ES 3.00 spec:
     *
     *    packUnorm2x16
     *    -------------
     *    The conversion for component c of v to fixed point is done as
     *    follows:
     *
     *       packUnorm2x16: round(clamp(c, 0, +1) * 65535.0)
     */
   return (uint16_t) (int)
          _mesa_roundevenf(CLAMP(x, 0.0f, 1.0f) * 65535.0f);
}

/**
 * Evaluate one component of unpackUnorm4x8.
 */
static float
unpack_unorm_1x8(uint8_t u)
{
    /* From section 8.4 of the GLSL 4.30 spec:
     *
     *    unpackUnorm4x8
     *    --------------
     *    The conversion for unpacked fixed-point value f to floating point is
     *    done as follows:
     *
     *       unpackUnorm4x8: f / 255.0
     */
   return (float) u / 255.0f;
}

/**
 * Evaluate one component of unpackUnorm2x16.
 */
static float
unpack_unorm_1x16(uint16_t u)
{
    /* From section 8.4 of the GLSL ES 3.00 spec:
     *
     *    unpackUnorm2x16
     *    ---------------
     *    The conversion for unpacked fixed-point value f to floating point is
     *    done as follows:
     *
     *       unpackUnorm2x16: f / 65535.0
     */
   return (float) u / 65535.0f;
}

/**
 * Evaluate one component of packHalf2x16.
 */
static uint16_t
pack_half_1x16(float x)
{
   return _mesa_float_to_half(x);
}

/**
 * Evaluate one component of unpackHalf2x16.
 */
static float
unpack_half_1x16(uint16_t u)
{
   return _mesa_half_to_float(u);
}

/* Some typed vector structures to make things like src0.y work */
struct float_vec {
   float x;
   float y;
   float z;
   float w;
};
struct int_vec {
   int x;
   int y;
   int z;
   int w;
};
struct unsigned_vec {
   unsigned x;
   unsigned y;
   unsigned z;
   unsigned w;
};
struct bool_vec {
   bool x;
   bool y;
   bool z;
   bool w;
};

static nir_const_value
evaluate_b2f(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               bool src0 = _src[0].u[_i] != 0;

            float dst = src0 ? 1.0f : 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_b2i(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               bool src0 = _src[0].u[_i] != 0;

            int dst = src0 ? 1 : 0;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ball2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct bool_vec src0 = {
            _src[0].u[0] != 0,
            _src[0].u[1] != 0,
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x) && (src0.y));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_ball3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct bool_vec src0 = {
            _src[0].u[0] != 0,
            _src[0].u[1] != 0,
            _src[0].u[2] != 0,
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x) && (src0.y) && (src0.z));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_ball4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct bool_vec src0 = {
            _src[0].u[0] != 0,
            _src[0].u[1] != 0,
            _src[0].u[2] != 0,
            _src[0].u[3] != 0,
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x) && (src0.y) && (src0.z) && (src0.w));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_ball_fequal2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x == src1.x) && (src0.y == src1.y));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_ball_fequal3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
            _src[1].f[2],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x == src1.x) && (src0.y == src1.y) && (src0.z == src1.z));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_ball_fequal4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
            _src[0].f[3],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
            _src[1].f[2],
            _src[1].f[3],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x == src1.x) && (src0.y == src1.y) && (src0.z == src1.z) && (src0.w == src1.w));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_ball_iequal2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct int_vec src0 = {
            _src[0].i[0],
            _src[0].i[1],
      };

      struct int_vec src1 = {
            _src[1].i[0],
            _src[1].i[1],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x == src1.x) && (src0.y == src1.y));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_ball_iequal3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct int_vec src0 = {
            _src[0].i[0],
            _src[0].i[1],
            _src[0].i[2],
      };

      struct int_vec src1 = {
            _src[1].i[0],
            _src[1].i[1],
            _src[1].i[2],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x == src1.x) && (src0.y == src1.y) && (src0.z == src1.z));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_ball_iequal4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct int_vec src0 = {
            _src[0].i[0],
            _src[0].i[1],
            _src[0].i[2],
            _src[0].i[3],
      };

      struct int_vec src1 = {
            _src[1].i[0],
            _src[1].i[1],
            _src[1].i[2],
            _src[1].i[3],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x == src1.x) && (src0.y == src1.y) && (src0.z == src1.z) && (src0.w == src1.w));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_bany2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct bool_vec src0 = {
            _src[0].u[0] != 0,
            _src[0].u[1] != 0,
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x) || (src0.y));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_bany3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct bool_vec src0 = {
            _src[0].u[0] != 0,
            _src[0].u[1] != 0,
            _src[0].u[2] != 0,
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x) || (src0.y) || (src0.z));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_bany4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct bool_vec src0 = {
            _src[0].u[0] != 0,
            _src[0].u[1] != 0,
            _src[0].u[2] != 0,
            _src[0].u[3] != 0,
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x) || (src0.y) || (src0.z) || (src0.w));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_bany_fnequal2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != src1.x) || (src0.y != src1.y));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_bany_fnequal3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
            _src[1].f[2],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != src1.x) || (src0.y != src1.y) || (src0.z != src1.z));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_bany_fnequal4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
            _src[0].f[3],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
            _src[1].f[2],
            _src[1].f[3],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != src1.x) || (src0.y != src1.y) || (src0.z != src1.z) || (src0.w != src1.w));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_bany_inequal2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct int_vec src0 = {
            _src[0].i[0],
            _src[0].i[1],
      };

      struct int_vec src1 = {
            _src[1].i[0],
            _src[1].i[1],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != src1.x) || (src0.y != src1.y));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_bany_inequal3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct int_vec src0 = {
            _src[0].i[0],
            _src[0].i[1],
            _src[0].i[2],
      };

      struct int_vec src1 = {
            _src[1].i[0],
            _src[1].i[1],
            _src[1].i[2],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != src1.x) || (src0.y != src1.y) || (src0.z != src1.z));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_bany_inequal4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct int_vec src0 = {
            _src[0].i[0],
            _src[0].i[1],
            _src[0].i[2],
            _src[0].i[3],
      };

      struct int_vec src1 = {
            _src[1].i[0],
            _src[1].i[1],
            _src[1].i[2],
            _src[1].i[3],
      };

      struct bool_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != src1.x) || (src0.y != src1.y) || (src0.z != src1.z) || (src0.w != src1.w));

            _dst_val.u[0] = dst.x ? NIR_TRUE : NIR_FALSE;

   return _dst_val;
}
static nir_const_value
evaluate_bcsel(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                           
      for (unsigned _i = 0; _i < num_components; _i++) {
               bool src0 = _src[0].u[_i] != 0;
               unsigned src1 = _src[1].u[_i];
               unsigned src2 = _src[2].u[_i];

            unsigned dst = src0 ? src1 : src2;

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_bfi(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                           
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];
               unsigned src2 = _src[2].u[_i];

            unsigned dst;
            
unsigned mask = src0, insert = src1 & mask, base = src2;
if (mask == 0) {
   dst = base;
} else {
   unsigned tmp = mask;
   while (!(tmp & 1)) {
      tmp >>= 1;
      insert <<= 1;
   }
   dst = (base & ~mask) | insert;
}


            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_bfm(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            unsigned dst;
            
int offset = src0, bits = src1;
if (offset < 0 || bits < 0 || offset + bits > 32)
   dst = 0; /* undefined per the spec */
else
   dst = ((1 << bits)- 1) << offset;


            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_bit_count(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];

            unsigned dst;
            
dst = 0;
for (unsigned bit = 0; bit < 32; bit++) {
   if ((src0 >> bit) & 1)
      dst++;
}


            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_bitfield_insert(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      struct int_vec src2 = {
            _src[2].i[0],
      };

      struct int_vec src3 = {
            _src[3].i[0],
      };

      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];
                              
            unsigned dst;
            
unsigned base = src0, insert = src1;
int offset = src2.x, bits = src3.x;
if (bits == 0) {
   dst = 0;
} else if (offset < 0 || bits < 0 || bits + offset > 32) {
   dst = 0;
} else {
   unsigned mask = ((1 << bits) - 1) << offset;
   dst = (base & ~mask) | ((insert << bits) & mask);
}


            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_bitfield_reverse(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];

            unsigned dst;
            
/* we're not winning any awards for speed here, but that's ok */
dst = 0;
for (unsigned bit = 0; bit < 32; bit++)
   dst |= ((src0 >> bit) & 1) << (31 - bit);


            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_f2b(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            bool dst = src0 != 0.0f;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_f2i(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            int dst = src0;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_f2u(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            unsigned dst = src0;

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fabs(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = fabsf(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fadd(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = src0 + src1;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fall2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != 0.0f) && (src0.y != 0.0f)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fall3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != 0.0f) && (src0.y != 0.0f) && (src0.z != 0.0f)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fall4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
            _src[0].f[3],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != 0.0f) && (src0.y != 0.0f) && (src0.z != 0.0f) && (src0.w != 0.0f)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fall_equal2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x == src1.x) && (src0.y == src1.y)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fall_equal3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
            _src[1].f[2],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x == src1.x) && (src0.y == src1.y) && (src0.z == src1.z)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fall_equal4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
            _src[0].f[3],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
            _src[1].f[2],
            _src[1].f[3],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x == src1.x) && (src0.y == src1.y) && (src0.z == src1.z) && (src0.w == src1.w)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fand(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = ((src0 != 0.0f) && (src1 != 0.0f)) ? 1.0f : 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fany2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != 0.0f) || (src0.y != 0.0f)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fany3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != 0.0f) || (src0.y != 0.0f) || (src0.z != 0.0f)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fany4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
            _src[0].f[3],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != 0.0f) || (src0.y != 0.0f) || (src0.z != 0.0f) || (src0.w != 0.0f)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fany_nequal2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != src1.x) || (src0.y != src1.y)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fany_nequal3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
            _src[1].f[2],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != src1.x) || (src0.y != src1.y) || (src0.z != src1.z)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fany_nequal4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
            _src[0].f[3],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
            _src[1].f[2],
            _src[1].f[3],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x != src1.x) || (src0.y != src1.y) || (src0.z != src1.z) || (src0.w != src1.w)) ? 1.0f : 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fceil(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = ceilf(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fcos(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = cosf(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fcsel(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                           
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];
               float src2 = _src[2].f[_i];

            float dst = (src0 != 0.0f) ? src1 : src2;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fddx(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               
            float dst = 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fddx_coarse(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               
            float dst = 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fddx_fine(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               
            float dst = 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fddy(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               
            float dst = 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fddy_coarse(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               
            float dst = 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fddy_fine(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               
            float dst = 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fdiv(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = src0 / src1;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fdot2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x * src1.x) + (src0.y * src1.y));

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fdot3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
            _src[1].f[2],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x * src1.x) + (src0.y * src1.y) + (src0.z * src1.z));

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fdot4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
            _src[0].f[3],
      };

      struct float_vec src1 = {
            _src[1].f[0],
            _src[1].f[1],
            _src[1].f[2],
            _src[1].f[3],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = ((src0.x * src1.x) + (src0.y * src1.y) + (src0.z * src1.z) + (src0.w * src1.w));

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_feq(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            bool dst = src0 == src1;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fexp2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = exp2f(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ffloor(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = floorf(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ffma(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                           
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];
               float src2 = _src[2].f[_i];

            float dst = src0 * src1 + src2;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ffract(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = src0 - floorf(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fge(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            bool dst = src0 >= src1;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_find_lsb(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];

            int dst;
            
dst = -1;
for (unsigned bit = 0; bit < 32; bit++) {
   if ((src0 >> bit) & 1) {
      dst = bit;
      break;
   }
}


            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_flog2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = log2f(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_flrp(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                           
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];
               float src2 = _src[2].f[_i];

            float dst = src0 * (1 - src2) + src1 * src2;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_flt(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            bool dst = src0 < src1;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fmax(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = fmaxf(src0, src1);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fmin(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = fminf(src0, src1);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fmod(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = src0 - src1 * floorf(src0 / src1);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fmov(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = src0;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fmul(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = src0 * src1;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fne(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            bool dst = src0 != src1;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fneg(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = -src0;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fnoise1_1(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise1_2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise1_3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise1_4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise2_1(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise2_2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise2_3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise2_4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise3_1(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;
            _dst_val.f[2] = dst.z;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise3_2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;
            _dst_val.f[2] = dst.z;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise3_3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;
            _dst_val.f[2] = dst.z;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise3_4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;
            _dst_val.f[2] = dst.z;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise4_1(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;
            _dst_val.f[2] = dst.z;
            _dst_val.f[3] = dst.w;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise4_2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;
            _dst_val.f[2] = dst.z;
            _dst_val.f[3] = dst.w;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise4_3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;
            _dst_val.f[2] = dst.z;
            _dst_val.f[3] = dst.w;

   return _dst_val;
}
static nir_const_value
evaluate_fnoise4_4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = 0.0f;

            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;
            _dst_val.f[2] = dst.z;
            _dst_val.f[3] = dst.w;

   return _dst_val;
}
static nir_const_value
evaluate_fnot(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = (src0 == 0.0f) ? 1.0f : 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_for(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = ((src0 != 0.0f) || (src1 != 0.0f)) ? 1.0f : 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fpow(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = powf(src0, src1);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_frcp(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = 1.0f / src0;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fround_even(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = _mesa_roundevenf(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_frsq(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = 1.0f / sqrtf(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fsat(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = (src0 > 1.0f) ? 1.0f : ((src0 <= 0.0f) ? 0.0f : src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fsign(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = (src0 == 0.0f) ? 0.0f : ((src0 > 0.0f) ? 1.0f : -1.0f);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fsin(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = sinf(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fsqrt(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = sqrtf(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fsub(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = src0 - src1;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ftrunc(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];

            float dst = truncf(src0);

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_fxor(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = (src0 != 0.0f && src1 == 0.0f) || (src0 == 0.0f && src1 != 0.0f) ? 1.0f : 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_i2b(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];

            bool dst = src0 != 0;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_i2f(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];

            float dst = src0;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_iabs(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];

            int dst = (src0 < 0) ? -src0 : src0;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_iadd(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            int dst = src0 + src1;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_iand(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            unsigned dst = src0 & src1;

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ibitfield_extract(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct int_vec src1 = {
            _src[1].i[0],
      };

      struct int_vec src2 = {
            _src[2].i[0],
      };

      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
                              
            int dst;
            
int base = src0;
int offset = src1.x, bits = src2.x;
if (bits == 0) {
   dst = 0;
} else if (offset < 0 || bits < 0 || offset + bits > 32) {
   dst = 0;
} else {
   dst = (base << (32 - offset - bits)) >> offset; /* use sign-extending shift */
}


            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_idiv(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            int dst = src0 / src1;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ieq(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            bool dst = src0 == src1;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ifind_msb(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];

            int dst;
            
dst = -1;
for (int bit = 31; bit >= 0; bit--) {
   /* If src0 < 0, we're looking for the first 0 bit.
    * if src0 >= 0, we're looking for the first 1 bit.
    */
   if ((((src0 >> bit) & 1) && (src0 >= 0)) ||
      (!((src0 >> bit) & 1) && (src0 < 0))) {
      dst = bit;
      break;
   }
}


            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ige(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            bool dst = src0 >= src1;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ilt(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            bool dst = src0 < src1;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_imax(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            int dst = src1 > src0 ? src1 : src0;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_imin(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            int dst = src1 > src0 ? src0 : src1;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_imov(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];

            int dst = src0;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_imul(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            int dst = src0 * src1;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_imul_high(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            int dst = (int32_t)(((int64_t) src0 * (int64_t) src1) >> 32);

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ine(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            bool dst = src0 != src1;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ineg(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];

            int dst = -src0;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_inot(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];

            int dst = ~src0;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ior(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            unsigned dst = src0 | src1;

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ishl(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            int dst = src0 << src1;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ishr(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            int dst = src0 >> src1;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_isign(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];

            int dst = (src0 == 0) ? 0 : ((src0 > 0) ? 1 : -1);

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_isub(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               int src0 = _src[0].i[_i];
               int src1 = _src[1].i[_i];

            int dst = src0 - src1;

            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ixor(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            unsigned dst = src0 ^ src1;

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ldexp(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               int src1 = _src[1].i[_i];

            float dst;
            
dst = ldexp(src0, src1);
/* flush denormals to zero. */
if (!isnormal(dst))
   dst = copysign(0.0f, src0);


            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_pack_half_2x16(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
      };

      struct unsigned_vec dst;

         
dst.x = (uint32_t) pack_half_1x16(src0.x);
dst.x |= ((uint32_t) pack_half_1x16(src0.y)) << 16;


            _dst_val.u[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_pack_half_2x16_split(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
      };

      struct float_vec src1 = {
            _src[1].f[0],
      };

      struct unsigned_vec dst;

         dst.x = dst.y = dst.z = dst.w = pack_half_1x16(src0.x) | (pack_half_1x16(src1.x) << 16);

            _dst_val.u[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_pack_snorm_2x16(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
      };

      struct unsigned_vec dst;

         
dst.x = (uint32_t) pack_snorm_1x16(src0.x);
dst.x |= ((uint32_t) pack_snorm_1x16(src0.y)) << 16;


            _dst_val.u[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_pack_snorm_4x8(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
            _src[0].f[3],
      };

      struct unsigned_vec dst;

         
dst.x = (uint32_t) pack_snorm_1x8(src0.x);
dst.x |= ((uint32_t) pack_snorm_1x8(src0.y)) << 8;
dst.x |= ((uint32_t) pack_snorm_1x8(src0.z)) << 16;
dst.x |= ((uint32_t) pack_snorm_1x8(src0.w)) << 24;


            _dst_val.u[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_pack_unorm_2x16(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
      };

      struct unsigned_vec dst;

         
dst.x = (uint32_t) pack_unorm_1x16(src0.x);
dst.x |= ((uint32_t) pack_unorm_1x16(src0.y)) << 16;


            _dst_val.u[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_pack_unorm_4x8(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct float_vec src0 = {
            _src[0].f[0],
            _src[0].f[1],
            _src[0].f[2],
            _src[0].f[3],
      };

      struct unsigned_vec dst;

         
dst.x = (uint32_t) pack_unorm_1x8(src0.x);
dst.x |= ((uint32_t) pack_unorm_1x8(src0.y)) << 8;
dst.x |= ((uint32_t) pack_unorm_1x8(src0.z)) << 16;
dst.x |= ((uint32_t) pack_unorm_1x8(src0.w)) << 24;


            _dst_val.u[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_seq(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = (src0 == src1) ? 1.0f : 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_sge(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = (src0 >= src1) ? 1.0f : 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_slt(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = (src0 < src1) ? 1.0f : 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_sne(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               float src0 = _src[0].f[_i];
               float src1 = _src[1].f[_i];

            float dst = (src0 != src1) ? 1.0f : 0.0f;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_u2f(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];

            float dst = src0;

            _dst_val.f[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_uadd_carry(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            bool dst = src0 + src1 < src0;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ubitfield_extract(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      struct int_vec src1 = {
            _src[1].i[0],
      };

      struct int_vec src2 = {
            _src[2].i[0],
      };

      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
                              
            unsigned dst;
            
unsigned base = src0;
int offset = src1.x, bits = src2.x;
if (bits == 0) {
   dst = 0;
} else if (bits < 0 || offset < 0 || offset + bits > 32) {
   dst = 0; /* undefined per the spec */
} else {
   dst = (base >> offset) & ((1 << bits) - 1);
}


            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_udiv(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            unsigned dst = src0 / src1;

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ufind_msb(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

         
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];

            int dst;
            
dst = -1;
for (int bit = 31; bit > 0; bit--) {
   if ((src0 >> bit) & 1) {
      dst = bit;
      break;
   }
}


            _dst_val.i[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_uge(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            bool dst = src0 >= src1;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_ult(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            bool dst = src0 < src1;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_umax(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            unsigned dst = src1 > src0 ? src1 : src0;

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_umin(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            unsigned dst = src1 > src0 ? src0 : src1;

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_umod(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            unsigned dst = src1 == 0 ? 0 : src0 % src1;

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_umul_high(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            unsigned dst = (uint32_t)(((uint64_t) src0 * (uint64_t) src1) >> 32);

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_unpack_half_2x16(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct unsigned_vec src0 = {
            _src[0].u[0],
      };

      struct float_vec dst;

         
dst.x = unpack_half_1x16((uint16_t)(src0.x & 0xffff));
dst.y = unpack_half_1x16((uint16_t)(src0.x << 16));


            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;

   return _dst_val;
}
static nir_const_value
evaluate_unpack_half_2x16_split_x(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct unsigned_vec src0 = {
            _src[0].u[0],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = unpack_half_1x16((uint16_t)(src0.x & 0xffff));

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_unpack_half_2x16_split_y(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct unsigned_vec src0 = {
            _src[0].u[0],
      };

      struct float_vec dst;

         dst.x = dst.y = dst.z = dst.w = unpack_half_1x16((uint16_t)(src0.x >> 16));

            _dst_val.f[0] = dst.x;

   return _dst_val;
}
static nir_const_value
evaluate_unpack_snorm_2x16(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct unsigned_vec src0 = {
            _src[0].u[0],
      };

      struct float_vec dst;

         
dst.x = unpack_snorm_1x16((uint16_t)(src0.x & 0xffff));
dst.y = unpack_snorm_1x16((uint16_t)(src0.x << 16));


            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;

   return _dst_val;
}
static nir_const_value
evaluate_unpack_snorm_4x8(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct unsigned_vec src0 = {
            _src[0].u[0],
      };

      struct float_vec dst;

         
dst.x = unpack_snorm_1x8((uint8_t)(src0.x & 0xff));
dst.y = unpack_snorm_1x8((uint8_t)((src0.x >> 8) & 0xff));
dst.z = unpack_snorm_1x8((uint8_t)((src0.x >> 16) & 0xff));
dst.w = unpack_snorm_1x8((uint8_t)(src0.x >> 24));


            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;
            _dst_val.f[2] = dst.z;
            _dst_val.f[3] = dst.w;

   return _dst_val;
}
static nir_const_value
evaluate_unpack_unorm_2x16(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct unsigned_vec src0 = {
            _src[0].u[0],
      };

      struct float_vec dst;

         
dst.x = unpack_unorm_1x16((uint16_t)(src0.x & 0xffff));
dst.y = unpack_unorm_1x16((uint16_t)(src0.x << 16));


            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;

   return _dst_val;
}
static nir_const_value
evaluate_unpack_unorm_4x8(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct unsigned_vec src0 = {
            _src[0].u[0],
      };

      struct float_vec dst;

         
dst.x = unpack_unorm_1x8((uint8_t)(src0.x & 0xff));
dst.y = unpack_unorm_1x8((uint8_t)((src0.x >> 8) & 0xff));
dst.z = unpack_unorm_1x8((uint8_t)((src0.x >> 16) & 0xff));
dst.w = unpack_unorm_1x8((uint8_t)(src0.x >> 24));


            _dst_val.f[0] = dst.x;
            _dst_val.f[1] = dst.y;
            _dst_val.f[2] = dst.z;
            _dst_val.f[3] = dst.w;

   return _dst_val;
}
static nir_const_value
evaluate_ushr(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            unsigned dst = src0 >> src1;

            _dst_val.u[_i] = dst;
      }

   return _dst_val;
}
static nir_const_value
evaluate_usub_borrow(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };

                  
      for (unsigned _i = 0; _i < num_components; _i++) {
               unsigned src0 = _src[0].u[_i];
               unsigned src1 = _src[1].u[_i];

            bool dst = src1 < src0;

            _dst_val.u[_i] = dst ? NIR_TRUE : NIR_FALSE;
      }

   return _dst_val;
}
static nir_const_value
evaluate_vec2(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct unsigned_vec src0 = {
            _src[0].u[0],
      };

      struct unsigned_vec src1 = {
            _src[1].u[0],
      };

      struct unsigned_vec dst;

         
dst.x = src0.x;
dst.y = src1.x;


            _dst_val.u[0] = dst.x;
            _dst_val.u[1] = dst.y;

   return _dst_val;
}
static nir_const_value
evaluate_vec3(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct unsigned_vec src0 = {
            _src[0].u[0],
      };

      struct unsigned_vec src1 = {
            _src[1].u[0],
      };

      struct unsigned_vec src2 = {
            _src[2].u[0],
      };

      struct unsigned_vec dst;

         
dst.x = src0.x;
dst.y = src1.x;
dst.z = src2.x;


            _dst_val.u[0] = dst.x;
            _dst_val.u[1] = dst.y;
            _dst_val.u[2] = dst.z;

   return _dst_val;
}
static nir_const_value
evaluate_vec4(unsigned num_components, nir_const_value *_src)
{
   nir_const_value _dst_val = { { {0, 0, 0, 0} } };


      struct unsigned_vec src0 = {
            _src[0].u[0],
      };

      struct unsigned_vec src1 = {
            _src[1].u[0],
      };

      struct unsigned_vec src2 = {
            _src[2].u[0],
      };

      struct unsigned_vec src3 = {
            _src[3].u[0],
      };

      struct unsigned_vec dst;

         
dst.x = src0.x;
dst.y = src1.x;
dst.z = src2.x;
dst.w = src3.x;


            _dst_val.u[0] = dst.x;
            _dst_val.u[1] = dst.y;
            _dst_val.u[2] = dst.z;
            _dst_val.u[3] = dst.w;

   return _dst_val;
}

nir_const_value
nir_eval_const_opcode(nir_op op, unsigned num_components,
                      nir_const_value *src)
{
   switch (op) {
   case nir_op_b2f: {
      return evaluate_b2f(num_components, src);
      break;
   }
   case nir_op_b2i: {
      return evaluate_b2i(num_components, src);
      break;
   }
   case nir_op_ball2: {
      return evaluate_ball2(num_components, src);
      break;
   }
   case nir_op_ball3: {
      return evaluate_ball3(num_components, src);
      break;
   }
   case nir_op_ball4: {
      return evaluate_ball4(num_components, src);
      break;
   }
   case nir_op_ball_fequal2: {
      return evaluate_ball_fequal2(num_components, src);
      break;
   }
   case nir_op_ball_fequal3: {
      return evaluate_ball_fequal3(num_components, src);
      break;
   }
   case nir_op_ball_fequal4: {
      return evaluate_ball_fequal4(num_components, src);
      break;
   }
   case nir_op_ball_iequal2: {
      return evaluate_ball_iequal2(num_components, src);
      break;
   }
   case nir_op_ball_iequal3: {
      return evaluate_ball_iequal3(num_components, src);
      break;
   }
   case nir_op_ball_iequal4: {
      return evaluate_ball_iequal4(num_components, src);
      break;
   }
   case nir_op_bany2: {
      return evaluate_bany2(num_components, src);
      break;
   }
   case nir_op_bany3: {
      return evaluate_bany3(num_components, src);
      break;
   }
   case nir_op_bany4: {
      return evaluate_bany4(num_components, src);
      break;
   }
   case nir_op_bany_fnequal2: {
      return evaluate_bany_fnequal2(num_components, src);
      break;
   }
   case nir_op_bany_fnequal3: {
      return evaluate_bany_fnequal3(num_components, src);
      break;
   }
   case nir_op_bany_fnequal4: {
      return evaluate_bany_fnequal4(num_components, src);
      break;
   }
   case nir_op_bany_inequal2: {
      return evaluate_bany_inequal2(num_components, src);
      break;
   }
   case nir_op_bany_inequal3: {
      return evaluate_bany_inequal3(num_components, src);
      break;
   }
   case nir_op_bany_inequal4: {
      return evaluate_bany_inequal4(num_components, src);
      break;
   }
   case nir_op_bcsel: {
      return evaluate_bcsel(num_components, src);
      break;
   }
   case nir_op_bfi: {
      return evaluate_bfi(num_components, src);
      break;
   }
   case nir_op_bfm: {
      return evaluate_bfm(num_components, src);
      break;
   }
   case nir_op_bit_count: {
      return evaluate_bit_count(num_components, src);
      break;
   }
   case nir_op_bitfield_insert: {
      return evaluate_bitfield_insert(num_components, src);
      break;
   }
   case nir_op_bitfield_reverse: {
      return evaluate_bitfield_reverse(num_components, src);
      break;
   }
   case nir_op_f2b: {
      return evaluate_f2b(num_components, src);
      break;
   }
   case nir_op_f2i: {
      return evaluate_f2i(num_components, src);
      break;
   }
   case nir_op_f2u: {
      return evaluate_f2u(num_components, src);
      break;
   }
   case nir_op_fabs: {
      return evaluate_fabs(num_components, src);
      break;
   }
   case nir_op_fadd: {
      return evaluate_fadd(num_components, src);
      break;
   }
   case nir_op_fall2: {
      return evaluate_fall2(num_components, src);
      break;
   }
   case nir_op_fall3: {
      return evaluate_fall3(num_components, src);
      break;
   }
   case nir_op_fall4: {
      return evaluate_fall4(num_components, src);
      break;
   }
   case nir_op_fall_equal2: {
      return evaluate_fall_equal2(num_components, src);
      break;
   }
   case nir_op_fall_equal3: {
      return evaluate_fall_equal3(num_components, src);
      break;
   }
   case nir_op_fall_equal4: {
      return evaluate_fall_equal4(num_components, src);
      break;
   }
   case nir_op_fand: {
      return evaluate_fand(num_components, src);
      break;
   }
   case nir_op_fany2: {
      return evaluate_fany2(num_components, src);
      break;
   }
   case nir_op_fany3: {
      return evaluate_fany3(num_components, src);
      break;
   }
   case nir_op_fany4: {
      return evaluate_fany4(num_components, src);
      break;
   }
   case nir_op_fany_nequal2: {
      return evaluate_fany_nequal2(num_components, src);
      break;
   }
   case nir_op_fany_nequal3: {
      return evaluate_fany_nequal3(num_components, src);
      break;
   }
   case nir_op_fany_nequal4: {
      return evaluate_fany_nequal4(num_components, src);
      break;
   }
   case nir_op_fceil: {
      return evaluate_fceil(num_components, src);
      break;
   }
   case nir_op_fcos: {
      return evaluate_fcos(num_components, src);
      break;
   }
   case nir_op_fcsel: {
      return evaluate_fcsel(num_components, src);
      break;
   }
   case nir_op_fddx: {
      return evaluate_fddx(num_components, src);
      break;
   }
   case nir_op_fddx_coarse: {
      return evaluate_fddx_coarse(num_components, src);
      break;
   }
   case nir_op_fddx_fine: {
      return evaluate_fddx_fine(num_components, src);
      break;
   }
   case nir_op_fddy: {
      return evaluate_fddy(num_components, src);
      break;
   }
   case nir_op_fddy_coarse: {
      return evaluate_fddy_coarse(num_components, src);
      break;
   }
   case nir_op_fddy_fine: {
      return evaluate_fddy_fine(num_components, src);
      break;
   }
   case nir_op_fdiv: {
      return evaluate_fdiv(num_components, src);
      break;
   }
   case nir_op_fdot2: {
      return evaluate_fdot2(num_components, src);
      break;
   }
   case nir_op_fdot3: {
      return evaluate_fdot3(num_components, src);
      break;
   }
   case nir_op_fdot4: {
      return evaluate_fdot4(num_components, src);
      break;
   }
   case nir_op_feq: {
      return evaluate_feq(num_components, src);
      break;
   }
   case nir_op_fexp2: {
      return evaluate_fexp2(num_components, src);
      break;
   }
   case nir_op_ffloor: {
      return evaluate_ffloor(num_components, src);
      break;
   }
   case nir_op_ffma: {
      return evaluate_ffma(num_components, src);
      break;
   }
   case nir_op_ffract: {
      return evaluate_ffract(num_components, src);
      break;
   }
   case nir_op_fge: {
      return evaluate_fge(num_components, src);
      break;
   }
   case nir_op_find_lsb: {
      return evaluate_find_lsb(num_components, src);
      break;
   }
   case nir_op_flog2: {
      return evaluate_flog2(num_components, src);
      break;
   }
   case nir_op_flrp: {
      return evaluate_flrp(num_components, src);
      break;
   }
   case nir_op_flt: {
      return evaluate_flt(num_components, src);
      break;
   }
   case nir_op_fmax: {
      return evaluate_fmax(num_components, src);
      break;
   }
   case nir_op_fmin: {
      return evaluate_fmin(num_components, src);
      break;
   }
   case nir_op_fmod: {
      return evaluate_fmod(num_components, src);
      break;
   }
   case nir_op_fmov: {
      return evaluate_fmov(num_components, src);
      break;
   }
   case nir_op_fmul: {
      return evaluate_fmul(num_components, src);
      break;
   }
   case nir_op_fne: {
      return evaluate_fne(num_components, src);
      break;
   }
   case nir_op_fneg: {
      return evaluate_fneg(num_components, src);
      break;
   }
   case nir_op_fnoise1_1: {
      return evaluate_fnoise1_1(num_components, src);
      break;
   }
   case nir_op_fnoise1_2: {
      return evaluate_fnoise1_2(num_components, src);
      break;
   }
   case nir_op_fnoise1_3: {
      return evaluate_fnoise1_3(num_components, src);
      break;
   }
   case nir_op_fnoise1_4: {
      return evaluate_fnoise1_4(num_components, src);
      break;
   }
   case nir_op_fnoise2_1: {
      return evaluate_fnoise2_1(num_components, src);
      break;
   }
   case nir_op_fnoise2_2: {
      return evaluate_fnoise2_2(num_components, src);
      break;
   }
   case nir_op_fnoise2_3: {
      return evaluate_fnoise2_3(num_components, src);
      break;
   }
   case nir_op_fnoise2_4: {
      return evaluate_fnoise2_4(num_components, src);
      break;
   }
   case nir_op_fnoise3_1: {
      return evaluate_fnoise3_1(num_components, src);
      break;
   }
   case nir_op_fnoise3_2: {
      return evaluate_fnoise3_2(num_components, src);
      break;
   }
   case nir_op_fnoise3_3: {
      return evaluate_fnoise3_3(num_components, src);
      break;
   }
   case nir_op_fnoise3_4: {
      return evaluate_fnoise3_4(num_components, src);
      break;
   }
   case nir_op_fnoise4_1: {
      return evaluate_fnoise4_1(num_components, src);
      break;
   }
   case nir_op_fnoise4_2: {
      return evaluate_fnoise4_2(num_components, src);
      break;
   }
   case nir_op_fnoise4_3: {
      return evaluate_fnoise4_3(num_components, src);
      break;
   }
   case nir_op_fnoise4_4: {
      return evaluate_fnoise4_4(num_components, src);
      break;
   }
   case nir_op_fnot: {
      return evaluate_fnot(num_components, src);
      break;
   }
   case nir_op_for: {
      return evaluate_for(num_components, src);
      break;
   }
   case nir_op_fpow: {
      return evaluate_fpow(num_components, src);
      break;
   }
   case nir_op_frcp: {
      return evaluate_frcp(num_components, src);
      break;
   }
   case nir_op_fround_even: {
      return evaluate_fround_even(num_components, src);
      break;
   }
   case nir_op_frsq: {
      return evaluate_frsq(num_components, src);
      break;
   }
   case nir_op_fsat: {
      return evaluate_fsat(num_components, src);
      break;
   }
   case nir_op_fsign: {
      return evaluate_fsign(num_components, src);
      break;
   }
   case nir_op_fsin: {
      return evaluate_fsin(num_components, src);
      break;
   }
   case nir_op_fsqrt: {
      return evaluate_fsqrt(num_components, src);
      break;
   }
   case nir_op_fsub: {
      return evaluate_fsub(num_components, src);
      break;
   }
   case nir_op_ftrunc: {
      return evaluate_ftrunc(num_components, src);
      break;
   }
   case nir_op_fxor: {
      return evaluate_fxor(num_components, src);
      break;
   }
   case nir_op_i2b: {
      return evaluate_i2b(num_components, src);
      break;
   }
   case nir_op_i2f: {
      return evaluate_i2f(num_components, src);
      break;
   }
   case nir_op_iabs: {
      return evaluate_iabs(num_components, src);
      break;
   }
   case nir_op_iadd: {
      return evaluate_iadd(num_components, src);
      break;
   }
   case nir_op_iand: {
      return evaluate_iand(num_components, src);
      break;
   }
   case nir_op_ibitfield_extract: {
      return evaluate_ibitfield_extract(num_components, src);
      break;
   }
   case nir_op_idiv: {
      return evaluate_idiv(num_components, src);
      break;
   }
   case nir_op_ieq: {
      return evaluate_ieq(num_components, src);
      break;
   }
   case nir_op_ifind_msb: {
      return evaluate_ifind_msb(num_components, src);
      break;
   }
   case nir_op_ige: {
      return evaluate_ige(num_components, src);
      break;
   }
   case nir_op_ilt: {
      return evaluate_ilt(num_components, src);
      break;
   }
   case nir_op_imax: {
      return evaluate_imax(num_components, src);
      break;
   }
   case nir_op_imin: {
      return evaluate_imin(num_components, src);
      break;
   }
   case nir_op_imov: {
      return evaluate_imov(num_components, src);
      break;
   }
   case nir_op_imul: {
      return evaluate_imul(num_components, src);
      break;
   }
   case nir_op_imul_high: {
      return evaluate_imul_high(num_components, src);
      break;
   }
   case nir_op_ine: {
      return evaluate_ine(num_components, src);
      break;
   }
   case nir_op_ineg: {
      return evaluate_ineg(num_components, src);
      break;
   }
   case nir_op_inot: {
      return evaluate_inot(num_components, src);
      break;
   }
   case nir_op_ior: {
      return evaluate_ior(num_components, src);
      break;
   }
   case nir_op_ishl: {
      return evaluate_ishl(num_components, src);
      break;
   }
   case nir_op_ishr: {
      return evaluate_ishr(num_components, src);
      break;
   }
   case nir_op_isign: {
      return evaluate_isign(num_components, src);
      break;
   }
   case nir_op_isub: {
      return evaluate_isub(num_components, src);
      break;
   }
   case nir_op_ixor: {
      return evaluate_ixor(num_components, src);
      break;
   }
   case nir_op_ldexp: {
      return evaluate_ldexp(num_components, src);
      break;
   }
   case nir_op_pack_half_2x16: {
      return evaluate_pack_half_2x16(num_components, src);
      break;
   }
   case nir_op_pack_half_2x16_split: {
      return evaluate_pack_half_2x16_split(num_components, src);
      break;
   }
   case nir_op_pack_snorm_2x16: {
      return evaluate_pack_snorm_2x16(num_components, src);
      break;
   }
   case nir_op_pack_snorm_4x8: {
      return evaluate_pack_snorm_4x8(num_components, src);
      break;
   }
   case nir_op_pack_unorm_2x16: {
      return evaluate_pack_unorm_2x16(num_components, src);
      break;
   }
   case nir_op_pack_unorm_4x8: {
      return evaluate_pack_unorm_4x8(num_components, src);
      break;
   }
   case nir_op_seq: {
      return evaluate_seq(num_components, src);
      break;
   }
   case nir_op_sge: {
      return evaluate_sge(num_components, src);
      break;
   }
   case nir_op_slt: {
      return evaluate_slt(num_components, src);
      break;
   }
   case nir_op_sne: {
      return evaluate_sne(num_components, src);
      break;
   }
   case nir_op_u2f: {
      return evaluate_u2f(num_components, src);
      break;
   }
   case nir_op_uadd_carry: {
      return evaluate_uadd_carry(num_components, src);
      break;
   }
   case nir_op_ubitfield_extract: {
      return evaluate_ubitfield_extract(num_components, src);
      break;
   }
   case nir_op_udiv: {
      return evaluate_udiv(num_components, src);
      break;
   }
   case nir_op_ufind_msb: {
      return evaluate_ufind_msb(num_components, src);
      break;
   }
   case nir_op_uge: {
      return evaluate_uge(num_components, src);
      break;
   }
   case nir_op_ult: {
      return evaluate_ult(num_components, src);
      break;
   }
   case nir_op_umax: {
      return evaluate_umax(num_components, src);
      break;
   }
   case nir_op_umin: {
      return evaluate_umin(num_components, src);
      break;
   }
   case nir_op_umod: {
      return evaluate_umod(num_components, src);
      break;
   }
   case nir_op_umul_high: {
      return evaluate_umul_high(num_components, src);
      break;
   }
   case nir_op_unpack_half_2x16: {
      return evaluate_unpack_half_2x16(num_components, src);
      break;
   }
   case nir_op_unpack_half_2x16_split_x: {
      return evaluate_unpack_half_2x16_split_x(num_components, src);
      break;
   }
   case nir_op_unpack_half_2x16_split_y: {
      return evaluate_unpack_half_2x16_split_y(num_components, src);
      break;
   }
   case nir_op_unpack_snorm_2x16: {
      return evaluate_unpack_snorm_2x16(num_components, src);
      break;
   }
   case nir_op_unpack_snorm_4x8: {
      return evaluate_unpack_snorm_4x8(num_components, src);
      break;
   }
   case nir_op_unpack_unorm_2x16: {
      return evaluate_unpack_unorm_2x16(num_components, src);
      break;
   }
   case nir_op_unpack_unorm_4x8: {
      return evaluate_unpack_unorm_4x8(num_components, src);
      break;
   }
   case nir_op_ushr: {
      return evaluate_ushr(num_components, src);
      break;
   }
   case nir_op_usub_borrow: {
      return evaluate_usub_borrow(num_components, src);
      break;
   }
   case nir_op_vec2: {
      return evaluate_vec2(num_components, src);
      break;
   }
   case nir_op_vec3: {
      return evaluate_vec3(num_components, src);
      break;
   }
   case nir_op_vec4: {
      return evaluate_vec4(num_components, src);
      break;
   }
   default:
      unreachable("shouldn't get here");
   }
}
