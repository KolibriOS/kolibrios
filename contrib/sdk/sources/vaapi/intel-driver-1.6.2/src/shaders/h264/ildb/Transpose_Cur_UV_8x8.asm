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
//	Module name: Transpose_UV_8x8.asm
//	
//	Transpose a 8x8 UV block. (8x8U + 8x8V)  The output is also in NV12
//
//----------------------------------------------------------------------------------------
//  Symbols need to be defined before including this module
//
//	Source region is :ub
//	SRC_UW:			SRC_UW Base=rxx ElementSize=2 SrcRegion=REGION(8,1) Type=uw	// 4 GRFs
//
//  Temp buffer:
//	BUF_W:			BUF_W Base=rxx ElementSize=2 SrcRegion=REGION(8,1) Type=uw		// 4 GRFs
//
//////////////////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xDDDA:w
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Src U and V are mixed in NV12 format. U on even bytes, V on odd bytes.
// Transpose by treating UV pair as a word.


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

//  First step 		(16)	<1>:w <==== <8;4,1>:w
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|33 33 32 32 31 31 30 30 23 23 22 22 21 21 20 20 13 13 12 12 11 11 10 10 03 03 02 02 01 01 00 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|73 73 72 72 71 71 70 70 63 63 62 62 61 61 60 60 53 53 52 52 51 51 50 50 43 43 42 42 41 41 40 40|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|37 37 36 36 35 35 34 34 27 27 26 26 25 25 24 24 17 17 16 16 15 15 14 14 07 07 06 06 05 05 04 04|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|77 77 76 76 75 75 74 74 67 67 66 66 65 65 64 64 57 57 56 56 55 55 54 54 47 47 46 46 45 45 44 44|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//
// Transpose UV (8x8 words), The first step
mov (16)	CUR_TEMP_W(0,0)<1>		SRC_UW(0,0)<8;4,1>
mov (16)	CUR_TEMP_W(1,0)<1>		SRC_UW(2,0)<8;4,1>
mov (16)	CUR_TEMP_W(2,0)<1>		SRC_UW(0,4)<8;4,1>
mov (16)	CUR_TEMP_W(3,0)<1>		SRC_UW(2,4)<8;4,1>


//	Second step		(16)	<1>:w <=== <16;4,4>:w
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|71 71 61 61 51 51 41 41 31 31 21 21 11 11 01 01 70 70 60 60 50 50 40 40 30 30 20 20 10 10 00 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|73 73 63 63 53 53 43 43 33 33 23 23 13 13 03 03 72 72 62 62 52 52 42 42 32 32 22 22 12 12 02 02|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|75 75 65 65 55 55 45 45 35 35 25 25 15 15 05 05 74 74 64 64 54 54 44 44 34 34 24 24 14 14 04 04|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|77 77 67 67 57 57 47 47 37 37 27 27 17 17 07 07 76 76 66 66 56 56 46 46 36 36 26 26 16 16 06 06|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//
// Transpose UV (8x8 words), The second step
mov (8)		SRC_UW(0,0)<1>		CUR_TEMP_W(0,0)<16;4,4>		{ NoDDClr }
mov (8)		SRC_UW(0,8)<1>		CUR_TEMP_W(0,1)<16;4,4>		{ NoDDChk }
mov (8)		SRC_UW(1,0)<1>		CUR_TEMP_W(0,2)<16;4,4>		{ NoDDClr }
mov (8)		SRC_UW(1,8)<1>		CUR_TEMP_W(0,3)<16;4,4>		{ NoDDChk }
mov (8)		SRC_UW(2,0)<1>		CUR_TEMP_W(2,0)<16;4,4>		{ NoDDClr }
mov (8)		SRC_UW(2,8)<1>		CUR_TEMP_W(2,1)<16;4,4>		{ NoDDChk }
mov (8)		SRC_UW(3,0)<1>		CUR_TEMP_W(2,2)<16;4,4>		{ NoDDClr }
mov (8)		SRC_UW(3,8)<1>		CUR_TEMP_W(2,3)<16;4,4>		{ NoDDChk }

// U and V are now transposed and separated.
