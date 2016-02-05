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
//	Module name: Transpose_Y_16x16.asm
//	
//	Transpose Y 16x16 block.
//
//----------------------------------------------------------------------------------------
//  Symbols need to be defined before including this module
//
//	Source region is :ub
//	SRC_YB:			SRC_YB Base=rxx ElementSize=1 SrcRegion=REGION(16,1) Type=ub	// 8 GRFs
//
//  Temp buffer:
//	CUR_TEMP_B:		BUF_B Base=rxx ElementSize=1 SrcRegion=REGION(16,1) Type=ub		// 8 GRFs
//
//////////////////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xDDDA:w
#endif


// Transpose Y (16x16 bytes)

// The first step
mov (16)	CUR_TEMP_B(0,0)<1>		SRC_YB(0,0)<16;4,1>		{ NoDDClr } 
mov (16)	CUR_TEMP_B(0,16)<1>		SRC_YB(2,0)<16;4,1>		{ NoDDChk }
mov (16)	CUR_TEMP_B(1,0)<1>		SRC_YB(4,0)<16;4,1>		{ NoDDClr }
mov (16)	CUR_TEMP_B(1,16)<1>		SRC_YB(6,0)<16;4,1>		{ NoDDChk }

mov (16)	CUR_TEMP_B(2,0)<1>		SRC_YB(0,4)<16;4,1>		{ NoDDClr }
mov (16)	CUR_TEMP_B(2,16)<1>		SRC_YB(2,4)<16;4,1>		{ NoDDChk }
mov (16)	CUR_TEMP_B(3,0)<1>		SRC_YB(4,4)<16;4,1>		{ NoDDClr }
mov (16)	CUR_TEMP_B(3,16)<1>		SRC_YB(6,4)<16;4,1>		{ NoDDChk }

mov (16)	CUR_TEMP_B(4,0)<1>		SRC_YB(0,8)<16;4,1>		{ NoDDClr }
mov (16)	CUR_TEMP_B(4,16)<1>		SRC_YB(2,8)<16;4,1>		{ NoDDChk }
mov (16)	CUR_TEMP_B(5,0)<1>		SRC_YB(4,8)<16;4,1>		{ NoDDClr }
mov (16)	CUR_TEMP_B(5,16)<1>		SRC_YB(6,8)<16;4,1>		{ NoDDChk }

mov (16)	CUR_TEMP_B(6,0)<1>		SRC_YB(0,12)<16;4,1>	{ NoDDClr }
mov (16)	CUR_TEMP_B(6,16)<1>		SRC_YB(2,12)<16;4,1>	{ NoDDChk }
mov (16)	CUR_TEMP_B(7,0)<1>		SRC_YB(4,12)<16;4,1>	{ NoDDClr }
mov (16)	CUR_TEMP_B(7,16)<1>		SRC_YB(6,12)<16;4,1>	{ NoDDChk }

// The second step
mov (16)	SRC_YB(0,0)<1>		CUR_TEMP_B(0,0)<32;8,4>		{ NoDDClr }
mov (16)	SRC_YB(0,16)<1>		CUR_TEMP_B(0,1)<32;8,4>		{ NoDDChk }
mov (16)	SRC_YB(1,0)<1>		CUR_TEMP_B(0,2)<32;8,4>		{ NoDDClr }
mov (16)	SRC_YB(1,16)<1>		CUR_TEMP_B(0,3)<32;8,4>		{ NoDDChk }

mov (16)	SRC_YB(2,0)<1>		CUR_TEMP_B(2,0)<32;8,4>		{ NoDDClr }
mov (16)	SRC_YB(2,16)<1>		CUR_TEMP_B(2,1)<32;8,4>		{ NoDDChk }
mov (16)	SRC_YB(3,0)<1>		CUR_TEMP_B(2,2)<32;8,4>		{ NoDDClr }
mov (16)	SRC_YB(3,16)<1>		CUR_TEMP_B(2,3)<32;8,4>		{ NoDDChk }

mov (16)	SRC_YB(4,0)<1>		CUR_TEMP_B(4,0)<32;8,4>		{ NoDDClr }
mov (16)	SRC_YB(4,16)<1>		CUR_TEMP_B(4,1)<32;8,4>		{ NoDDChk }
mov (16)	SRC_YB(5,0)<1>		CUR_TEMP_B(4,2)<32;8,4>		{ NoDDClr }
mov (16)	SRC_YB(5,16)<1>		CUR_TEMP_B(4,3)<32;8,4>		{ NoDDChk }

mov (16)	SRC_YB(6,0)<1>		CUR_TEMP_B(6,0)<32;8,4>		{ NoDDClr }
mov (16)	SRC_YB(6,16)<1>		CUR_TEMP_B(6,1)<32;8,4>		{ NoDDChk }
mov (16)	SRC_YB(7,0)<1>		CUR_TEMP_B(6,2)<32;8,4>		{ NoDDClr }
mov (16)	SRC_YB(7,16)<1>		CUR_TEMP_B(6,3)<32;8,4>		{ NoDDChk }

// Y is transposed.
