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

// Module name: PL2_Load.inc

#undef 	nY_NUM_OF_ROWS
#undef 	nUV_NUM_OF_ROWS

#define nY_NUM_OF_ROWS      8                                         // Number of Y rows per block
#define nDPR_BLOCK_SIZE_Y   nBLOCK_WIDTH_16+nBLOCK_HEIGHT_8           // Y block size 16x8
#define nDPR_MSG_SIZE_Y     nRESLEN_4                                 // # of MRF's to hold Y block data (4)


#if defined(NV11_LOAD_4x8)
        #define nUV_NUM_OF_ROWS     8                                 // Number of U/V rows per block
        #define nDPR_BLOCK_SIZE_UV  nBLOCK_WIDTH_8+nBLOCK_HEIGHT_8    // U/V block size 8x8
        #define nDPR_MSG_SIZE_UV    nRESLEN_2                         // # of MRF's to hold U/V block data (2)
#endif

#if defined(NV11_LOAD_5x8)
        #define nUV_NUM_OF_ROWS     8                                 // Number of U/V rows per block
        #define nDPR_BLOCK_SIZE_UV  nBLOCK_WIDTH_12+nBLOCK_HEIGHT_8   // U/V block size 12x8
        #define nDPR_MSG_SIZE_UV    nRESLEN_4                         // # of MRF's to hold U/V block data (4)
#endif
#if defined(NV12_LOAD_8x4)
        #define nUV_NUM_OF_ROWS     4                                 // Number of U/V rows per block
        #define nDPR_BLOCK_SIZE_UV  nBLOCK_WIDTH_16+nBLOCK_HEIGHT_4   // U/V block size 16x4
        #define nDPR_MSG_SIZE_UV    nRESLEN_2                         // # of MRF's to hold U/V block data (2)
#endif
#if defined(NV12_LOAD_8x5)
        #define nUV_NUM_OF_ROWS     6                                 // Number of U/V rows per block (Rounded Up to Nearest Even Number)
        #define nDPR_BLOCK_SIZE_UV  nBLOCK_WIDTH_16+nBLOCK_HEIGHT_5   // U/V block size 16x5
        #define nDPR_MSG_SIZE_UV    nRESLEN_3                         // # of MRF's to hold U/V block data (3)
#endif
#if defined(NV12_LOAD_9x5)
        #define nUV_NUM_OF_ROWS     6                                 // Number of U/V rows per block (Rounded Up to Nearest Even Number)
        #define nDPR_BLOCK_SIZE_UV  nBLOCK_WIDTH_20+nBLOCK_HEIGHT_5   // U/V block size 20x5
        #define nDPR_MSG_SIZE_UV    nRESLEN_5                         // # of MRF's to hold U/V block data (5)
#endif
#if defined(P208_LOAD_8x8)
        #define nUV_NUM_OF_ROWS     8                                 // Number of U/V rows per block
        #define nDPR_BLOCK_SIZE_UV  nBLOCK_WIDTH_16+nBLOCK_HEIGHT_8   // U/V block size 16x8
        #define nDPR_MSG_SIZE_UV    nRESLEN_4                         // # of MRF's to hold U/V block data (4)
#endif
#if defined(P208_LOAD_9x8)
        #define nUV_NUM_OF_ROWS     8                                 // Number of U/V rows per block
        #define nDPR_BLOCK_SIZE_UV  nBLOCK_WIDTH_20+nBLOCK_HEIGHT_8   // U/V block size 20x8
        #define nDPR_MSG_SIZE_UV    nRESLEN_8                         // # of MRF's to hold U/V block data (8)
#endif

// Source/destination region definitions
#if !defined(udSRC_Y)
        #define udSRC_Y  udBOT_Y_IO     // Default Y source region is top Y region
#endif

#if !defined(udSRC_U)
        #define udSRC_U  udBOT_U_IO     // Default U source region is top U region
#endif

#define ubSRC_Y           ubBOT_Y
#define nSRC_Y_REG        nBOT_Y
#define ubSRC_U           ubBOT_U
#define nSRC_U_REG        nBOT_U

#define uwDEST_Y          uwTOP_Y       // However they can be transferred to word-aligned byte if desired
#define uwDEST_U          uwTOP_U
#define uwDEST_V          uwTOP_V

#define ubDEST_Y          ubTOP_Y       // I'd like use them for color conversion
#define uwSRC_U           uwBOT_U

#define nSRC_REGION       nREGION_1     // REGION_1 will be the source region for first kernel

// End of PL2_Load.inc
