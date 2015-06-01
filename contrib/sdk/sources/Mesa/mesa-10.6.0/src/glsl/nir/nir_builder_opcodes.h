/* Copyright (C) 2015 Broadcom
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

#ifndef _NIR_BUILDER_OPCODES_
#define _NIR_BUILDER_OPCODES_

ALU1(b2f);
ALU1(b2i);
ALU1(ball2);
ALU1(ball3);
ALU1(ball4);
ALU2(ball_fequal2);
ALU2(ball_fequal3);
ALU2(ball_fequal4);
ALU2(ball_iequal2);
ALU2(ball_iequal3);
ALU2(ball_iequal4);
ALU1(bany2);
ALU1(bany3);
ALU1(bany4);
ALU2(bany_fnequal2);
ALU2(bany_fnequal3);
ALU2(bany_fnequal4);
ALU2(bany_inequal2);
ALU2(bany_inequal3);
ALU2(bany_inequal4);
ALU3(bcsel);
ALU3(bfi);
ALU2(bfm);
ALU1(bit_count);
ALU4(bitfield_insert);
ALU1(bitfield_reverse);
ALU1(f2b);
ALU1(f2i);
ALU1(f2u);
ALU1(fabs);
ALU2(fadd);
ALU1(fall2);
ALU1(fall3);
ALU1(fall4);
ALU2(fall_equal2);
ALU2(fall_equal3);
ALU2(fall_equal4);
ALU2(fand);
ALU1(fany2);
ALU1(fany3);
ALU1(fany4);
ALU2(fany_nequal2);
ALU2(fany_nequal3);
ALU2(fany_nequal4);
ALU1(fceil);
ALU1(fcos);
ALU3(fcsel);
ALU1(fddx);
ALU1(fddx_coarse);
ALU1(fddx_fine);
ALU1(fddy);
ALU1(fddy_coarse);
ALU1(fddy_fine);
ALU2(fdiv);
ALU2(fdot2);
ALU2(fdot3);
ALU2(fdot4);
ALU2(feq);
ALU1(fexp2);
ALU1(ffloor);
ALU3(ffma);
ALU1(ffract);
ALU2(fge);
ALU1(find_lsb);
ALU1(flog2);
ALU3(flrp);
ALU2(flt);
ALU2(fmax);
ALU2(fmin);
ALU2(fmod);
ALU1(fmov);
ALU2(fmul);
ALU2(fne);
ALU1(fneg);
ALU1(fnoise1_1);
ALU1(fnoise1_2);
ALU1(fnoise1_3);
ALU1(fnoise1_4);
ALU1(fnoise2_1);
ALU1(fnoise2_2);
ALU1(fnoise2_3);
ALU1(fnoise2_4);
ALU1(fnoise3_1);
ALU1(fnoise3_2);
ALU1(fnoise3_3);
ALU1(fnoise3_4);
ALU1(fnoise4_1);
ALU1(fnoise4_2);
ALU1(fnoise4_3);
ALU1(fnoise4_4);
ALU1(fnot);
ALU2(for);
ALU2(fpow);
ALU1(frcp);
ALU1(fround_even);
ALU1(frsq);
ALU1(fsat);
ALU1(fsign);
ALU1(fsin);
ALU1(fsqrt);
ALU2(fsub);
ALU1(ftrunc);
ALU2(fxor);
ALU1(i2b);
ALU1(i2f);
ALU1(iabs);
ALU2(iadd);
ALU2(iand);
ALU3(ibitfield_extract);
ALU2(idiv);
ALU2(ieq);
ALU1(ifind_msb);
ALU2(ige);
ALU2(ilt);
ALU2(imax);
ALU2(imin);
ALU1(imov);
ALU2(imul);
ALU2(imul_high);
ALU2(ine);
ALU1(ineg);
ALU1(inot);
ALU2(ior);
ALU2(ishl);
ALU2(ishr);
ALU1(isign);
ALU2(isub);
ALU2(ixor);
ALU2(ldexp);
ALU1(pack_half_2x16);
ALU2(pack_half_2x16_split);
ALU1(pack_snorm_2x16);
ALU1(pack_snorm_4x8);
ALU1(pack_unorm_2x16);
ALU1(pack_unorm_4x8);
ALU2(seq);
ALU2(sge);
ALU2(slt);
ALU2(sne);
ALU1(u2f);
ALU2(uadd_carry);
ALU3(ubitfield_extract);
ALU2(udiv);
ALU1(ufind_msb);
ALU2(uge);
ALU2(ult);
ALU2(umax);
ALU2(umin);
ALU2(umod);
ALU2(umul_high);
ALU1(unpack_half_2x16);
ALU1(unpack_half_2x16_split_x);
ALU1(unpack_half_2x16_split_y);
ALU1(unpack_snorm_2x16);
ALU1(unpack_snorm_4x8);
ALU1(unpack_unorm_2x16);
ALU1(unpack_unorm_4x8);
ALU2(ushr);
ALU2(usub_borrow);
ALU2(vec2);
ALU3(vec3);
ALU4(vec4);

#endif /* _NIR_BUILDER_OPCODES_ */
