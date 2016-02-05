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

// Module name: PL8x8_Save_PA.asm
//
// Save planar YUV422 to packed YUV422 format data
//
// Note: SRC_* must reference to regions with data type "BYTE"
//               in order to save to byte-aligned byte location

#include "PL8x8_Save_PA.inc"

    add (4) pCF_Y_OFFSET<1>:uw   ubDEST_CF_OFFSET<4;4,1>:ub   nDEST_YUV_REG*nGRFWIB:w    // Initial Y,U,V offset in YUV422 block

    // Pack Y
    $for(0; <nY_NUM_OF_ROWS; 1) {
        mov (16) r[pCF_Y_OFFSET, %1*nGRFWIB]<2>    ubSRC_Y(0,%1*32)
    }

    // Pack U/V
    $for(0; <nUV_NUM_OF_ROWS; 1) {
        mov (8)  r[pCF_U_OFFSET, %1*nGRFWIB]<4>    ubSRC_U(0, %1*16)
        mov (8)  r[pCF_V_OFFSET, %1*nGRFWIB]<4>    ubSRC_V(0, %1*16)
    }

    shl (1) rMSGSRC.0<1>:d      wORIX<0;1,0>:w            1:w  { NoDDClr }             // H. block origin need to be doubled
    mov (1) rMSGSRC.1<1>:d      wORIY<0;1,0>:w                 { NoDDClr, NoDDChk }    // Block origin
    mov (1) rMSGSRC.2<1>:ud     nDPW_BLOCK_SIZE_YUV:ud         { NoDDChk }             // Block width and height (32x8)

    mov (8) mMSGHDR<1>:ud       rMSGSRC<8;8,1>:ud

//Use the mask to determine which pixels shouldn't be over-written
    and (1)        acc0.0<1>:ud udBLOCK_MASK<0;1,0>:ud   0x00FFFFFF:ud
    cmp.ge.f0.0(1) dNULLREG     acc0.0<0;1,0>:ud         0x00FFFFFF:ud   //Check if all pixels in the block need to be modified
    (f0.0)  jmpi WritePackedToDataPort

    //If mask is not all 1's, then load the entire 32x8 block
    //so that only those bytes may be modified that need to be (using the mask)

    // Load 32x8 packed YUV 422 ----------------------------------------------------
    send (8) udSRC_YUV(0)<1>    mMSGHDR     udDUMMY_NULL    nDATAPORT_READ    nDPMR_MSGDSC+nDPR_MSG_SIZE_YUV+nBI_DESTINATION_YUV:ud
    mov  (8) mMSGHDR<1>:ud      rMSGSRC<8;8,1>:ud

    //Merge the data
    mov (1)           f0.0:uw             ubBLOCK_MASK_V:ub    //Load the mask on flag reg
    (f0.0)  mov (8)   rMASK_TEMP<1>:uw    uwBLOCK_MASK_H:uw
    (-f0.0) mov (8)   rMASK_TEMP<1>:uw    0:uw

    // Destination is Byte aligned
    $for(0; <nY_NUM_OF_ROWS; 1) {
        mov (1)             f0.1:uw                   uwMASK_TEMP(0,%1)<0;1,0>
        (-f0.1) mov (16)    uwDEST_YUV(%1)<1>         uwSRC_YUV(%1)        //check the UV merge - vK
    }

WritePackedToDataPort:
    //  Packed YUV data are stored in one of the I/O regions before moving to MRF
    //  Note: This is necessary since indirect addressing is not supported for MRF. 
    //  Packed data block should be saved as 32x8 pixel block
    $for(0; <nY_NUM_OF_ROWS; 1) {
        mov (8) mudMSGPAYLOAD(%1)<1>       udDEST_YUV(%1)REGION(8,1)
    }
    send (8)    dNULLREG    mMSGHDR   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_YUV+nBI_DESTINATION_YUV:ud

// End of PL8x8_Save_PA
