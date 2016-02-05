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


// Module name: PL8x8_Save_P208.asm
//
// Save entire current planar frame data block of size 16x8
//---------------------------------------------------------------
//  Symbols needed to be defined before including this module
//
//      DWORD_ALIGNED_DEST:     only if DEST_Y, DEST_U, DEST_V data are DWord aligned
//      ORIX:
//---------------------------------------------------------------

#include "PL8x8_Save_P208.inc"

    mov  (8) mMSGHDR<1>:ud      rMSGSRC<8;8,1>:ud

#if !defined(SAVE_UV_ONLY)
// Save current planar frame Y block data (16x8) -------------------------------

    mov  (2) mMSGHDR.0<1>:d     wORIX<2;2,1>:w          // Block origin
    mov  (1) mMSGHDR.2<1>:ud    nDPW_BLOCK_SIZE_Y:ud    // Block width and height (16x8)

WritePlanarToDataPort:
    $for(0,0; <nY_NUM_OF_ROWS; 2,1) {
            mov (16) mubMSGPAYLOAD(%2,0)<1>     ub2DEST_Y(%1)REGION(16,2)
            mov (16) mubMSGPAYLOAD(%2,16)<1>    ub2DEST_Y(%1+1)REGION(16,2)
    } 
    send (8)    dNULLREG    mMSGHDR   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_Y+nBI_DESTINATION_Y:ud
#endif
    
//** Save  8x8 packed U and V -----------------------------------------------------
// we could write directly wORIX to mMSGHDR and then execute asr on it, that way we could
// avoid using rMSGSRC as a buffer and have one command less in code, but it is unknown whether
//it is possible to do asr on mMSGHDR so we use rMSGSRC.
    mov (2)  rMSGSRC.0<1>:d    wORIX<2;2,1>:w             // Block origin
                                                                                                        
    mov (1)  rMSGSRC.2<1>:ud   nDPW_BLOCK_SIZE_UV:ud      // U/V block width and height (16x4)
    mov (8)  mMSGHDR<1>:ud     rMSGSRC<8;8,1>:ud

    $for(0,0; <nY_NUM_OF_ROWS;2,1) {
        mov (16) mubMSGPAYLOAD(%2,0)<2>     ub2DEST_U(%2)REGION(16,2) 
        mov (16) mubMSGPAYLOAD(%2,1)<2>     ub2DEST_V(%2)REGION(16,2) 
    }
    send (8)    dNULLREG    mMSGHDR    udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_UV+nBI_DESTINATION_UV:ud

//End of PL8x8_Save_P208.asm  

