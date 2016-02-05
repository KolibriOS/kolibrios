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

// Module name: RGB_to_YUV_Coef.asm
//----------------------------------------------------------------

// is src surface |R|G|B|X| layout? otherwise it is |B|G|R|X|
and.nz.f0.1 (1) dNULLREG     r1.1:ud         0xFF000000:ud      
#ifdef FIX_POINT_CONVERSION
        // Y = ( (  66 * R + 129 * G +  25 * B + 128 ) >> 8) +  16
    (-f0.1) mov (1) REG2(r, nTEMP0, 0):ud       0x00428119:ud      // used as unsigned byte
    ( f0.1) mov (1) REG2(r, nTEMP0, 0):ud       0x00198142:ud      // used as unsigned byte
        // U = ( ( -38 * R -  74 * G + 112 * B + 128 ) >> 8) + 128
    (-f0.1) mov (1) REG2(r, nTEMP0, 1):ud       0x00DAB670:ud      // used as signed byte
    ( f0.1) mov (1) REG2(r, nTEMP0, 1):ud       0x0070B6DA:ud      // used as signed byte
        // V = ( ( 112 * R -  94 * G -  18 * B + 128 ) >> 8) + 128
    (-f0.1) mov (1) REG2(r, nTEMP0, 2):ud       0x0070A2EEud      // used as signed byte
    ( f0.1) mov (1) REG2(r, nTEMP0, 2):ud       0x00EEA270ud      // used as signed byte

    #define ubRGB_to_Y_Coef_Fix        REG2(r, nTEMP0, 0)
    #define  bRGB_to_U_Coef_Fix        REG2(r, nTEMP0, 4)
    #define  bRGB_to_V_Coef_Fix        REG2(r, nTEMP0, 8)
#else
        // Y =  0.299R + 0.587G + 0.114B
    (-f0.1) mov (1) REG2(r, nTEMP8, 0):f       0.114f       // B coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 2):f       0.114f       // R coef
            mov (1) REG2(r, nTEMP8, 1):f       0.587f       // G coef
    (-f0.1) mov (1) REG2(r, nTEMP8, 2):f       0.299f       // R coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 0):f       0.299f       // B coef
            mov (1) REG2(r, nTEMP8, 3):f       0.000f       // A coef
    
    // Cb= -0.169R - 0.331G + 0.499B + 128
        // U = -0.147R - 0.289G + 0.436B + 128
    (-f0.1) mov (1) REG2(r, nTEMP8, 4):f       0.436f       // B coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 6):f       0.436f       // R coef
            mov (1) REG2(r, nTEMP8, 5):f      -0.289f       // G coef
    (-f0.1) mov (1) REG2(r, nTEMP8, 6):f      -0.147f       // R coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 4):f      -0.147f       // B coef
            mov (1) REG2(r, nTEMP8, 7):f       0.000f       // A coef

        // Cr= 0.499R - 0.418G - 0.0813B+ 128
        // V = 0.615R - 0.515G - 0.100B + 128
    (-f0.1) mov (1) REG2(r, nTEMP10, 0):f      -0.100f       // B coef
    ( f0.1) mov (1) REG2(r, nTEMP10, 2):f      -0.100f       // R coef
            mov (1) REG2(r, nTEMP10, 1):f      -0.515f       // G coef
    (-f0.1) mov (1) REG2(r, nTEMP10, 2):f       0.615f       // R coef
    ( f0.1) mov (1) REG2(r, nTEMP10, 0):f       0.615f       // B coef
            mov (1) REG2(r, nTEMP10, 3):f       0.000f       // A coef

    #define fRGB_to_Y_Coef_Float        REG2(r, nTEMP8, 0)
    #define fRGB_to_U_Coef_Float        REG2(r, nTEMP8, 4)
    #define fRGB_to_V_Coef_Float        REG2(r, nTEMP10, 0)
    .declare fROW_BGRX       Base=REG(r,nTEMP0) ElementSize=4 SrcRegion=REGION(8,8) Type=f    // r nTEMP0 - r nTEMP7
#endif

