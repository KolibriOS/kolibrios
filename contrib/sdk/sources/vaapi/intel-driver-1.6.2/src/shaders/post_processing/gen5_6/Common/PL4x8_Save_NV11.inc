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

//Module name: PL8x4_Save_NV11.inc
//
// Setup for storing planar data
//

#include "undefall.inc"                 //Undefine the SRC and DEST sysmbols

#define nDPW_BLOCK_SIZE_Y   nBLOCK_WIDTH_16+nBLOCK_HEIGHT_8   // Y block size 16x8
#define nDPW_MSG_SIZE_Y     nMSGLEN_4                         // # of MRF's to hold Y block data (4)
#define nDPW_BLOCK_SIZE_UV  nBLOCK_WIDTH_8+nBLOCK_HEIGHT_8    // U/V interleaved block width and height (8x8)
#define nDPW_MSG_SIZE_UV    nMSGLEN_2                         // # of MRF's to hold U/V block data (2)

#if (nSRC_REGION==nREGION_1)
        #define udSRC_Y                 udBOT_Y_IO
        #define udSRC_U                 udBOT_U_IO
        #define udSRC_V                 udBOT_V_IO
        #define ubSRC_Y                 ubBOT_Y
        #define ubSRC_U                 ubBOT_U
        #define ubSRC_V                 ubBOT_V

        #define uwSRC_U                 uwBOT_U  //For masking operation
        #define uwSRC_V                 uwBOT_V

        #define ub2DEST_Y               ub2TOP_Y
        #define ub2DEST_U               ub2TOP_U
        #define ub2DEST_V               ub2TOP_V
        
#elif (nSRC_REGION==nREGION_2)
        #define udSRC_Y                 udTOP_Y_IO
        #define udSRC_U                 udTOP_U_IO
        #define udSRC_V                 udTOP_V_IO
        #define ubSRC_Y                 ubTOP_Y
        #define ubSRC_U                 ubTOP_U
        #define ubSRC_V                 ubTOP_V

        #define uwSRC_U                 uwTOP_U  //For masking operation
        #define uwSRC_V                 uwTOP_V

        #define ub2DEST_Y               ub2BOT_Y
        #define ub2DEST_U               ub2BOT_U
        #define ub2DEST_V               ub2BOT_V
        
#endif

///* Yoni - masking is not relevant for ILK?!? 
//#define         TEMP0   REG(r,54)
//.declare    TEMP        Base=TEMP0      ElementSize=2   SrcRegion=<8;8,1>       Type=uw
///* Yoni - masking is not relevant for ILK?!? 


