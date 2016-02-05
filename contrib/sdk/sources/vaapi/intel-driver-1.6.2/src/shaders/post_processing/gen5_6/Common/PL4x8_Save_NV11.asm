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


#include "PL4x8_Save_NV11.inc"

    mov  (8) mMSGHDR<1>:ud      rMSGSRC<8;8,1>:ud

#if !defined(SAVE_UV_ONLY)
// Save current planar frame Y block data (16x8) -------------------------------

    mov  (2) mMSGHDR.0<1>:d     wORIX<2;2,1>:w          // Block origin
    mov  (1) mMSGHDR.2<1>:ud    nDPW_BLOCK_SIZE_Y:ud    // Block width and height (16x8)

///* Yoni - masking is not relevant for ILK?!? 
//
//        //Use the mask to determine which pixels shouldn't be over-written
//        cmp.ge.f0.0     (1)             NULLREG         BLOCK_MASK_D:ud         0x00FFFFFF:ud   //Check if all pixels in the block need to be modified
//        (f0.0)  jmpi WritePlanarToDataPort
//
//        //If mask is not all 1's, then load the entire 16x8 block
//        //so that only those bytes may be modified that need to be (using the mask)
//    send (8)    SRC_YD(0)<1>    MSGHDR  MSGSRC<8;8,1>:ud        DWBRMSGDSC+0x00040000+BI_DEST_Y:ud         //16x8
//        
//    asr  (2)    MSGSRC.0<1>:ud  ORIX<2;2,1>:w   1:w     // U/V block origin should be half of Y's
//    mov  (1)    MSGSRC.2<1>:ud  0x00030007:ud           // Block width and height (8x4)
//    send (8)    SRC_UD(0)<1>    MSGHDR  MSGSRC<8;8,1>:ud        DWBRMSGDSC+0x00010000+BI_DEST_U:ud
//    send (8)    SRC_VD(0)<1>    MSGHDR  MSGSRC<8;8,1>:ud        DWBRMSGDSC+0x00010000+BI_DEST_V:ud
//        
//    //Restore the origin information
//    mov (2)     MSGSRC.0<1>:ud  ORIX<2;2,1>:w           // Block origin
//    mov (1)     MSGSRC.2<1>:ud  0x0007000F:ud           // Block width and height (16x8)
//
//        //expand U and V to be aligned on word boundary
//        mov     (16)    SRC_UW(1)<1>            SRC_U(0,16)
//        mov     (16)    SRC_UW(0)<1>            SRC_U(0, 0)
//        mov (16)        SRC_VW(1)<1>            SRC_V(0,16)
//        mov (16)        SRC_VW(0)<1>            SRC_V(0, 0)
//        
//        //Merge the data
//        mov  (1)        f0.1:uw                 BLOCK_MASK_V:uw                 //Load the mask on flag reg
//        (f0.1)  mov     (8)     TEMP0<1>:uw     BLOCK_MASK_H:uw
//        (-f0.1) mov     (8)     TEMP0<1>:uw     0:uw
//                
//        // Destination is Word aligned
//                $for(0; <Y_ROW_SIZE; 2) {
//                        mov     (1)     f0.1:uw         TEMP(0,%1)<0;1,0>
//                        (-f0.1) mov  (16)       DEST_Y(0, %1*32)<2>             SRC_Y(0, %1*16)
//                        (-f0.1) mov  (16)       DEST_U(0, %1*8)<1>              SRC_U(0, %1*8)  //only works for Word aligned Byte data
//                        (-f0.1) mov  (16)       DEST_V(0, %1*8)<1>              SRC_V(0, %1*8)  //only works for Word aligned Byte data
//
//                        mov     (1)     f0.1:uw         TEMP(0,1+%1)<0;1,0>
//                        (-f0.1) mov  (16)       DEST_Y(0, 1+%1*32)<2>   SRC_Y(0, 1+%1*16)
//
//                }
//
//*/ Yoni - masking is not relevant for ILK?!? 
        
WritePlanarToDataPort:
    $for(0,0; <nY_NUM_OF_ROWS; 2,1) {
            mov (16) mubMSGPAYLOAD(%2,0)<1>     ub2DEST_Y(%1)REGION(16,2)
            mov (16) mubMSGPAYLOAD(%2,16)<1>    ub2DEST_Y(%1+1)REGION(16,2)
    } 
    send (8)    dNULLREG    mMSGHDR   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_Y+nBI_DESTINATION_Y:ud
#endif

// Save U/V data block in planar format (4x8) ----------------------------------
    mov (2)  rMSGSRC.0<1>:d    wORIX<2;2,1>:w             // Block origin
    asr (1)  rMSGSRC.0<1>:d    rMSGSRC.0<0;1,0>:d    1:w  // U/V block origin should be half of Y's
    mov (8)  mMSGHDR<1>:ud     rMSGSRC<8;8,1>:ud

    $for(0,0; <nY_NUM_OF_ROWS;4,1) {
        mov (16) mubMSGPAYLOAD(%2,0)<2>     ub2DEST_U(%2)REGION(16,2) 
        mov (16) mubMSGPAYLOAD(%2,1)<2>     ub2DEST_V(%2)REGION(16,2) 
    }
    send (8)    dNULLREG    mMSGHDR    udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_UV+nBI_DESTINATION_UV:ud

// End of PL4x8_Save_NV11

