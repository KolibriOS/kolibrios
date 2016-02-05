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

// Module name: RGB16x8_Save_Y416.asm
//
// Save packed ARGB 444 frame data block of size 16x8
//
// To save 16x8 block (128x8 byte layout for ARGB 16bit per component) we need 4 send instructions
//  -----------------
//  | 1 | 2 | 3 | 4 |
//  ----------------- 

#include "RGB16x8_Save_RGB.inc"

    shl (1) rMSGSRC.0<1>:d      wORIX<0;1,0>:w            3:w  { NoDDClr }             // H. block origin need to become 8 times
    mov (1) rMSGSRC.1<1>:d      wORIY<0;1,0>:w                 { NoDDClr, NoDDChk }    // Block origin (1st quadrant)
    mov (1) rMSGSRC.2<1>:ud     nDPW_BLOCK_SIZE_ARGB:ud        { NoDDChk }             // Block width and height (32x8)

    mov (8) mMSGHDR<1>:ud       rMSGSRC<8;8,1>:ud
/*	Not needed for validation kernels for now -vK
//Use the mask to determine which pixels shouldn't be over-written
    and (1)        acc0.0<1>:ud udBLOCK_MASK<0;1,0>:ud   0x00FFFFFF:ud
    cmp.ge.f0.0(1) dNULLREG     acc0.0<0;1,0>:ud         0x00FFFFFF:ud   //Check if all pixels in the block need to be modified
    (f0.0)  jmpi WriteARGBToDataPort

    //If mask is not all 1's, then load the entire 64x8 block
    //so that only those bytes may be modified that need to be (using the mask)

    // Load first block 16x8 packed ARGB 444 ---------------------------------------
    or (1)         acc0.0<1>:ud udBLOCK_MASK<0;1,0>:ud   0xFF00FF00:ud   //Check first block
    cmp.e.f0.0 (1) dNULLREG     acc0.0<0;1,0>:ud         0xFFFFFFFF:ud   
    (f0.0)  jmpi SkipFirstBlockMerge                                     //If full mask then skip this block

    send (8) udSRC_ARGB(0)<1>   mMSGHDR     udDUMMY_NULL    nDATAPORT_READ    nDPMR_MSGDSC+nDPR_MSG_SIZE_ARGB+nBI_DESTINATION_RGB:ud
    mov  (8) mMSGHDR<1>:ud      rMSGSRC<8;8,1>:ud

    //Merge the data
    mov (1)           f0.0:uw             ubBLOCK_MASK_V:ub    //Load the mask on flag reg
    (f0.0)  mov (8)   rMASK_TEMP<1>:uw    uwBLOCK_MASK_H:uw    //use sel instruction - vK
    (-f0.0) mov (8)   rMASK_TEMP<1>:uw    0:uw

    $for(0, 0; <nY_NUM_OF_ROWS; 1, 2) {               //take care of the lines in the block, they are different in the src and dest
        mov (1)             f0.1:uw                   uwMASK_TEMP(0,%1)<0;1,0>
        (-f0.1) mov (8)     udDEST_ARGB(%2)<1>        udSRC_ARGB(%1) 
    }

SkipFirstBlockMerge:
    // Load second block 16x8 packed ARGB 444 ---------------------------------------
    or (1)         acc0.0<1>:ud udBLOCK_MASK<0;1,0>:ud   0xFF0000FF:ud   //Check second block
    cmp.e.f0.0 (1) dNULLREG     acc0.0<0;1,0>:ud         0xFFFFFFFF:ud   
    (f0.0)  jmpi WriteARGBToDataPort                                     //If full mask then skip this block

    add  (1) mMSGHDR.0<1>:d     rMSGSRC.0<0;1,0>:d       32:d     // Point to 2nd part
    send (8) udSRC_ARGB(0)<1>   mMSGHDR    udDUMMY_NULL  nDATAPORT_READ    nDPMR_MSGDSC+nDPR_MSG_SIZE_ARGB+nBI_DESTINATION_RGB:ud
    mov  (8) mMSGHDR<1>:ud      rMSGSRC<8;8,1>:ud                 // Point to 1st part again

    //Merge the data
    mov (1)           f0.0:uw             ubBLOCK_MASK_V:ub    //Load the mask on flag reg
    (f0.0)  shr (8)   rMASK_TEMP<1>:uw    uwBLOCK_MASK_H:uw    8:uw    //load the mask for second block
    (-f0.0) mov (8)   rMASK_TEMP<1>:uw    0:uw

    $for(0, 1; <nY_NUM_OF_ROWS; 1, 2) {               //take care of the lines in the block, they are different in the src and dest
        mov (1)             f0.1:uw                   uwMASK_TEMP(0,%1)<0;1,0>
        (-f0.1) mov (8)     udDEST_ARGB(%2)<1>        udSRC_ARGB(%1) 
    }
*/
WriteARGBToDataPort:
    // Move packed data to MRF and output
    
    //Write 1st 4X8 pixels  
    $for(0; <nY_NUM_OF_ROWS; 1) {
        mov (8) mudMSGPAYLOAD(%1)<1>       udDEST_ARGB(%1*4)
    }
    send (8)    dNULLREG    mMSGHDR   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_ARGB+nBI_DESTINATION_RGB:ud

	//Write 2nd 4X8 pixels  
    mov  (8)    mMSGHDR<1>:ud         rMSGSRC<8;8,1>:ud
    add  (1)    mMSGHDR.0<1>:d        rMSGSRC.0<0;1,0>:d       32:d   // Point to 2nd part
    $for(0; <nY_NUM_OF_ROWS; 1) {
        mov (8) mudMSGPAYLOAD(%1)<1>       udDEST_ARGB(%1*4+1)
    }
    send (8)    dNULLREG    mMSGHDR   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_ARGB+nBI_DESTINATION_RGB:ud

	//Write 3rd 4X8 pixels  
    mov  (8)    mMSGHDR<1>:ud         rMSGSRC<8;8,1>:ud
    add  (1)    mMSGHDR.0<1>:d        rMSGSRC.0<0;1,0>:d       64:d   // Point to 2nd part
    $for(0; <nY_NUM_OF_ROWS; 1) {
        mov (8) mudMSGPAYLOAD(%1)<1>       udDEST_ARGB(%1*4+2)
    }
    send (8)    dNULLREG    mMSGHDR   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_ARGB+nBI_DESTINATION_RGB:ud

	//Write 4th 4X8 pixels  
    mov  (8)    mMSGHDR<1>:ud         rMSGSRC<8;8,1>:ud
    add  (1)    mMSGHDR.0<1>:d        rMSGSRC.0<0;1,0>:d       96:d   // Point to 2nd part
    $for(0; <nY_NUM_OF_ROWS; 1) {
        mov (8) mudMSGPAYLOAD(%1)<1>       udDEST_ARGB(%1*4+3)
    }
    send (8)    dNULLREG    mMSGHDR   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_ARGB+nBI_DESTINATION_RGB:ud

// End of RGB16x8_Save_Y416
