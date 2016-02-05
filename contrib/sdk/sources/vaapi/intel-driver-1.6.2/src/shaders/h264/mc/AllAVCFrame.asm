/*
 * All frame picture HWMC kernels 
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
//	2857702934	// 0xAA551616 - GUID for Intra_16x16 luma prediction mode offsets
//    0    // Offset to Intra_16x16 luma prediction mode 0
//    9    // Offset to Intra_16x16 luma prediction mode 1
//   19    // Offset to Intra_16x16 luma prediction mode 2
//   42    // Offset to Intra_16x16 luma prediction mode 3
//	2857699336	// 0xAA550808 - GUID for Intra_8x8 luma prediction mode offsets
//    0    // Offset to Intra_8x8 luma prediction mode 0
//    5    // Offset to Intra_8x8 luma prediction mode 1
//   10    // Offset to Intra_8x8 luma prediction mode 2
//   26    // Offset to Intra_8x8 luma prediction mode 3
//   36    // Offset to Intra_8x8 luma prediction mode 4
//   50    // Offset to Intra_8x8 luma prediction mode 5
//   68    // Offset to Intra_8x8 luma prediction mode 6
//   85    // Offset to Intra_8x8 luma prediction mode 7
//   95    // Offset to Intra_8x8 luma prediction mode 8
//	2857698308	// 0xAA550404 - GUID for Intra_4x4 luma prediction mode offsets
//    0    // Offset to Intra_4x4 luma prediction mode 0
//    2    // Offset to Intra_4x4 luma prediction mode 1
//    4    // Offset to Intra_4x4 luma prediction mode 2
//   16    // Offset to Intra_4x4 luma prediction mode 3
//   23    // Offset to Intra_4x4 luma prediction mode 4
//   32    // Offset to Intra_4x4 luma prediction mode 5
//   45    // Offset to Intra_4x4 luma prediction mode 6
//   59    // Offset to Intra_4x4 luma prediction mode 7
//   66    // Offset to Intra_4x4 luma prediction mode 8
//	2857700364	// 0xAA550C0C - GUID for intra chroma prediction mode offsets
//    0    // Offset to intra chroma prediction mode 0
//   30    // Offset to intra chroma prediction mode 1
//   36    // Offset to intra chroma prediction mode 2
//   41    // Offset to intra chroma prediction mode 3

// Kernel name: AllAVCFrame.asm
//
// All frame picture HWMC kernels merged into this file
//
//  $Revision: 1 $
//  $Date: 4/13/06 4:35p $
//

// ----------------------------------------------------
//  Main: AllAVCFrame
// ----------------------------------------------------

#define	ALLHWMC
#define	COMBINED_KERNEL

.kernel AllAVCFrame

    #include "Intra_PCM.asm"
    #include "Intra_16x16.asm"
    #include "Intra_8x8.asm"
    #include "Intra_4x4.asm"
    #include "scoreboard.asm"

	#include "AVCMCInter.asm"

// End of AllAVCFrame

.end_kernel

