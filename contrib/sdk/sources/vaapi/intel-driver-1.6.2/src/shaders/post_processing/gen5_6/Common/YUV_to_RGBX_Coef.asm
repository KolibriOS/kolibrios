/*
 * All Video Processing kernels 
 * Copyright © <2010>, Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file was originally licensed under the following license
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 * Authors:
 *    Halley Zhao <halley.zhao@intel.com>
 */

// Module name: YUV_to_RGBX_Coef.asm
//----------------------------------------------------------------
#define ubDEST_RGBX          ubTOP_Y       // I'd like use them for color conversion

// is dst surface |R|G|B|X| layout? otherwise, it is |B|G|R|X| layout
and.nz.f0.1 (1) dNULLREG     r1.2:ud         0xFF000000:ud      
#ifdef FIX_POINT_CONVERSION
    // ###### set up transformation coefficient
        // R = clip((   298 * C +   0 * D +    409 * E + 128) >> 8)
        // R = clip((0x012A * C +   0 * D + 0x0199 * E + 128) >> 8)
    (-f0.1) mov (1) REG2(r, nTEMP0, 0):ud       0x0000012A:ud      
    (-f0.1) mov (1) REG2(r, nTEMP0, 1):ud       0x00000199:ud      
    ( f0.1) mov (1) REG2(r, nTEMP0, 4):ud       0x0000012A:ud      
    ( f0.1) mov (1) REG2(r, nTEMP0, 5):ud       0x00000199:ud      

        // G = clip((    298 * C -    100 * D -    208 * E + 128) >> 8)
        // G = clip(( 0x012A * C -   0x64 * D -   0xD0 * E + 128) >> 8)
        // G = clip(( 0x012A * C + 0xFF9C * D + 0xFF30 * E + 128) >> 8)
    mov (1) REG2(r, nTEMP0, 2):ud       0xFF9C012A:ud      
    mov (1) REG2(r, nTEMP0, 3):ud       0x0000FF30:ud      

        // B = clip((  298 * C +    516 * D +   0 * E + 128) >> 8) 
        // B = clip((0x012A* C + 0x0204 * D +   0 * E + 128) >> 8) 
    (-f0.1) mov (1) REG2(r, nTEMP0, 4):ud       0x0204012A:ud
    (-f0.1) mov (1) REG2(r, nTEMP0, 5):ud       0x00000000:ud
    ( f0.1) mov (1) REG2(r, nTEMP0, 0):ud       0x0204012A:ud
    ( f0.1) mov (1) REG2(r, nTEMP0, 1):ud       0x00000000:ud

    // asr.sat (24) REG2(r,nTEMP0,0)<1>    REG2(r,nTEMP0,0)<0;24,1>    1:w
    asr.sat (8) REG2(r,nTEMP0, 0)<1>:w    REG2(r,nTEMP0, 0)<0;8,1>:w    1:w
    asr.sat (4)  REG2(r,nTEMP0,8)<1>:w    REG2(r,nTEMP0,8)<0;4,1>:w    1:w
    
        // C = Y' - 16          D = U - 128         E = V - 128
    mov (1) REG2(r, nTEMP0, 6):ud       0x008080F0:ud

    #define wYUV_to_RGB_CH2_Coef_Fix        REG2(r, nTEMP0, 0)
    #define wYUV_to_RGB_CH1_Coef_Fix        REG2(r, nTEMP0, 4)
    #define wYUV_to_RGB_CH0_Coef_Fix        REG2(r, nTEMP0, 8)
    #define bYUV_OFF                        REG2(r,nTEMP0,24)

    // debug use
    #define bYUV_to_RGB_CH2_Coef_Fix        REG2(r, nTEMP0, 0)
    #define bYUV_to_RGB_CH1_Coef_Fix        REG2(r, nTEMP0, 8)
    #define bYUV_to_RGB_CH0_Coef_Fix        REG2(r, nTEMP0, 16)

#else
        // R = Y             + 1.13983*V
        // R = clip( Y                  + 1.402*(Cr-128))  // ITU-R
    (-f0.1) mov (1) REG2(r, nTEMP8, 3):f       0.000f       // A coef
    (-f0.1) mov (1) REG2(r, nTEMP8, 2):f       1.402f       // V coef
    (-f0.1) mov (1) REG2(r, nTEMP8, 1):f       0.0f         // U coef
    (-f0.1) mov (1) REG2(r, nTEMP8, 0):f       1.0f         // Y coef

    ( f0.1) mov (1) REG2(r, nTEMP10, 3):f       0.000f       // A coef
    ( f0.1) mov (1) REG2(r, nTEMP10, 2):f       1.402f       // V coef
    ( f0.1) mov (1) REG2(r, nTEMP10, 1):f       0.0f         // U coef
    ( f0.1) mov (1) REG2(r, nTEMP10, 0):f       1.0f         // Y coef
    
        // G = Y - 0.39465*U - 0.58060*V
        // G = clip( Y - 0.344*(Cb-128) - 0.714*(Cr-128))
    mov (1) REG2(r, nTEMP8, 7):f       0.000f       // A coef
    mov (1) REG2(r, nTEMP8, 6):f      -0.714f       // V coef
    mov (1) REG2(r, nTEMP8, 5):f      -0.344f       // U coef
    mov (1) REG2(r, nTEMP8, 4):f       1.0f         // Y coef

        // B = Y + 2.03211*U
        // B = clip( Y + 1.772*(Cb-128))
    (-f0.1) mov (1) REG2(r, nTEMP10, 3):f       0.000f      // A coef
    (-f0.1) mov (1) REG2(r, nTEMP10, 2):f       0.0f        // V coef
    (-f0.1) mov (1) REG2(r, nTEMP10, 1):f       1.772f      // U coef
    (-f0.1) mov (1) REG2(r, nTEMP10, 0):f       1.0f        // Y coef

    ( f0.1) mov (1) REG2(r, nTEMP8, 3):f       0.000f      // A coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 2):f       0.0f        // V coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 1):f       1.772f      // U coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 0):f       1.0f        // Y coef

    mov (1) REG2(r, nTEMP10,  4):ud         0x008080F0:ud

    #define fYUV_to_RGB_CH2_Coef_Float          REG2(r, nTEMP8, 0)
    #define fYUV_to_RGB_CH1_Coef_Float          REG2(r, nTEMP8, 4)
    #define fYUV_to_RGB_CH0_Coef_Float          REG2(r, nTEMP10, 0)
    #define bYUV_OFF                            REG2(r,nTEMP10,16)

    .declare fROW_YUVA       Base=REG(r,nTEMP0) ElementSize=4 SrcRegion=REGION(8,8) Type=f    // r nTEMP0 - r nTEMP7

#endif
