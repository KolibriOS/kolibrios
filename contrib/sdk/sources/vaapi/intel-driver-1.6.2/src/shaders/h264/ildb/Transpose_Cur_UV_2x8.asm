/*
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
//////////////////////////////////////////////////////////////////////////////////////////
//	Module name: Transpose_UV_2x8.asm
//	
//	Transpose UV 2x8 to 8x2 block (2x8U + 2x8V in NV12)
//
//----------------------------------------------------------------------------------------
//  Symbols need to be defined before including this module
//
//	Source region is :ub
//	SRC_UW:			SRC_UB Base=rxx ElementSize=2 SrcRegion=REGION(8,1) Type=uw		// 4 GRFs
//
//  Temp buffer:
//	BUF_W:			BUF_W Base=rxx ElementSize=2 SrcRegion=REGION(8,1) Type=uw		// 4 GRFs
//
//////////////////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xDDDB:w
#endif

// Transpose UV (4x8),  right most 2 columns in word
// Use BUF_W(0) as temp buf

// Src U 8x8 and V 8x8 are mixed. (each pix is specified as yx)
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|17 17 16 16 15 15 14 14 13 13 12 12 11 11 10 10 07 07 06 06 05 05 04 04 03 03 02 02 01 01 00 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|37 37 36 36 35 35 34 34 33 33 32 32 31 31 30 30 27 27 26 26 25 25 24 24 23 23 22 22 21 21 20 20|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|57 57 56 56 55 55 54 54 53 53 52 52 51 51 50 50 47 47 46 46 45 45 44 44 43 43 42 42 41 41 40 40|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|77 77 76 76 75 75 74 74 73 73 72 72 71 71 70 70 67 67 66 66 65 65 64 64 63 63 62 62 61 61 60 60|
//	+-----------------------+-----------------------+-----------------------+-----------------------+

//  First step 		(8)	<1>:w <==== <8;2,1>:w
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|77 77 76 76 67 67 66 66 57 57 56 56 47 47 46 46 37 37 36 36 27 27 26 26 17 17 16 16 07 07 06 06|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
mov (8)		LEFT_TEMP_W(0,0)<1>		SRC_UW(0,6)<8;2,1>		{ NoDDClr }
mov (8)		LEFT_TEMP_W(0,8)<1>		SRC_UW(2,6)<8;2,1>		{ NoDDChk }

//	Second step		(16) <1>:w <==== <1;8,2>:w
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|77 77 67 67 57 57 47 47 37 37 27 27 17 17 07 07 76 76 66 66 56 56 46 46 36 36 26 26 16 16 06 06|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
mov (16)	LEFT_TEMP_W(1,0)<1>		LEFT_TEMP_W(0,0)<1;8,2>

// UV are now transposed.  the result is in BUF_W(1)
