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

// Module name: PA_Load.inc
//
// YUV422 data are first loaded to bottom I/O REGION_2, then unpacked to planar data
// and stored in top I/O REGION_1

#undef 	nY_NUM_OF_ROWS
#undef 	nUV_NUM_OF_ROWS

#define nY_NUM_OF_ROWS      8       // Number of Y rows per block
#define nUV_NUM_OF_ROWS     8       // Number of U/V rows per block

#if defined(PA_LOAD_8x8)
        #define nDPR_BLOCK_SIZE_YUV           nBLOCK_WIDTH_32+nBLOCK_HEIGHT_8   // Y block size 32x8
        #define nDPR_MSG_SIZE_YUV             nRESLEN_8                         // # of MRF's to hold Y block data (8)
#endif
#if defined(PA_LOAD_9x8)
        #define nDPR_BLOCK_SIZE_YUV_MAIN      nBLOCK_WIDTH_32+nBLOCK_HEIGHT_8   // Main YUV block size 32x8
        #define nDPR_MSG_SIZE_YUV_MAIN        nRESLEN_8                         // # of MRF's to hold Y block data (8)
        #define nDPR_BLOCK_SIZE_YUV_ADDITION  nBLOCK_WIDTH_4+nBLOCK_HEIGHT_8    // Additional YUV block size 4x8
        #define nDPR_MSG_SIZE_YUV_ADDITION    nRESLEN_1                         // # of MRF's to hold Y block data (8)
#endif

#define udSRC_YUV               udBOT_Y_IO
#define nSRC_YUV_REG            nBOT_Y

#define uwDEST_Y                uwTOP_Y
#define uwDEST_U                uwTOP_U
#define uwDEST_V                uwTOP_V

#define nSRC_REGION nREGION_1    // REGION_1 will be the source region for first kernel

// End of PA_Load.inc
