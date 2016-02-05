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

// Module name: PL8x8_Save_PA.inc
//
// Setup for storing packed data
//

#include "undefall.inc"                 //Undefine the SRC and DEST sysmbols

// For saving
#define nDPW_BLOCK_SIZE_YUV      nBLOCK_WIDTH_32+nBLOCK_HEIGHT_8    // YUV block size 32x8
#define nDPW_MSG_SIZE_YUV        nMSGLEN_8                          // # of MRF's to hold YUV block data (8)

// For masking
#undef  nDPR_MSG_SIZE_YUV
#define nDPR_MSG_SIZE_YUV        nRESLEN_8                          // # of MRF's to hold YUV block data (8)
#define     rMASK_TEMP     REG(r,nTEMP0)
.declare    uwMASK_TEMP    Base=rMASK_TEMP    ElementSize=2    SrcRegion=<8;8,1>    Type=uw        //1 GRF

#if (nSRC_REGION==nREGION_1)
    // For saving
    #define udSRC_YUV        udTOP_Y_IO
    #define udDEST_YUV       udBOT_Y_IO
    #define nDEST_YUV_REG    nBOT_Y
    //For masking operation
    #define ubSRC_Y          ub2TOP_Y
    #define ubSRC_U          ub2TOP_U
    #define ubSRC_V          ub2TOP_V
    #define uwSRC_YUV        uwTOP_Y
    #define uwDEST_YUV       uwBOT_Y

#elif (nSRC_REGION==nREGION_2)
    // For saving
    #define udSRC_YUV        udBOT_Y_IO
    #define udDEST_YUV       udTOP_Y_IO
    #define nDEST_YUV_REG    nTOP_Y
    //For masking operation
    #define ubSRC_Y          ub2BOT_Y
    #define ubSRC_U          ub2BOT_U
    #define ubSRC_V          ub2BOT_V
    #define uwSRC_YUV        uwBOT_Y
    #define uwDEST_YUV       uwTOP_Y

#endif
