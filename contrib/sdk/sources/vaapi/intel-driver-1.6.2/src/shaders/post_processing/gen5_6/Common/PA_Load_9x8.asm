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
 */

// Module name: PA_Load_9x8.asm
//----------------------------------------------------------------
//  This module loads 16x8 Y, 9x8 U and 9x8 V planar data blocks for CSC module 
//	and stores it in word-aligned format.
//----------------------------------------------------------------

#define  PA_LOAD_9x8
#include "PA_Load.inc"

//  Load 18x8 packed data block
//  Packed data block should be loaded as 36x8 pixel block
    add  (2) rMSGSRC.0<1>:d     wORIX<2;2,1>:w    wSRC_H_ORI_OFFSET<2;2,1>:w       // Source Block origin
    shl  (1) rMSGSRC.0<1>:d     acc0:w            1:w                              // H. block origin need to be doubled
    mov  (1) rMSGSRC.2<1>:ud    nDPR_BLOCK_SIZE_YUV_MAIN:ud                        // Block width and height (32x8)
    mov  (8) mMSGHDRY<1>:ud     rMSGSRC<8;8,1>:ud
    send (8) udSRC_YUV(0)<1>    mMSGHDRY    udDUMMY_NULL    nDATAPORT_READ    nDPMR_MSGDSC+nDPR_MSG_SIZE_YUV_MAIN+nBI_CURRENT_SRC_YUV:ud

    add  (1) rMSGSRC.0<1>:d     rMSGSRC.0:d       32:w                             //the last 4 pixels are read again for optimization
    mov  (1) rMSGSRC.2<1>:ud    nDPR_BLOCK_SIZE_YUV_ADDITION:ud                    // Block width and height (4x8)
    mov  (8) mMSGHDRY<1>:ud     rMSGSRC<8;8,1>:ud
    send (8) udSRC_YUV(8)<1>    mMSGHDRY    udDUMMY_NULL    nDATAPORT_READ    nDPMR_MSGDSC+nDPR_MSG_SIZE_YUV_ADDITION+nBI_CURRENT_SRC_YUV:ud

//  Unpack to "planar" YUV422 format in word-aligned bytes
    add  (4) pCF_Y_OFFSET<1>:uw    ubSRC_CF_OFFSET<4;4,1>:ub    nSRC_YUV_REG*nGRFWIB:w    // Initial Y,U,V offset in YUV422 block
    $for(0; <nY_NUM_OF_ROWS; 1) {
        mov (16)  uwDEST_Y(0, %1*16)<1>     r[pCF_Y_OFFSET, %1*nGRFWIB]REGION(16,2)
        mov (8)   uwDEST_U(0, %1*16)<1>     r[pCF_U_OFFSET, %1*nGRFWIB]REGION(8,4)
        mov (8)   uwDEST_V(0, %1*16)<1>     r[pCF_V_OFFSET, %1*nGRFWIB]REGION(8,4)
    }

    $for(0; <nUV_NUM_OF_ROWS; 1) {
        mov (1)   uwDEST_U(0, %1*16+8)<1>   r[pCF_U_OFFSET, %1*4+256]REGION(1,0)
        mov (1)   uwDEST_V(0, %1*16+8)<1>   r[pCF_V_OFFSET, %1*4+256]REGION(1,0)
    }
	//UV expansion done in PL9x8_PL16x8.asm module

// End of PA_Load_9x8
