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
// Module name: saveNV12_16x4T.asm
//
// Transpose 16x4 to 4x16 YNV12 data and write to memory 
//
//----------------------------------------------------------------
//  Symbols need to be defined before including this module
//
//	Left MB region:
//	PREV_MB_YB:	  	Base=rxx 	ElementSize=1 SrcRegion=REGION(16,1) Type=ub
//	PREV_MB_UW: 	Base=ryy 	ElementSize=2 SrcRegion=REGION(8,1) Type=uw

//	Binding table index: 
//	BI_SRC_Y:		Binding table index of Y surface
//	BI_SRC_UV:		Binding table index of UV surface (NV12)
//
//	Temp buffer:
//	BUF_B:			BUF_B Base=rxx ElementSize=1 SrcRegion=REGION(16,1) Type=ub
//	BUF_W:			BUF_W Base=rxx ElementSize=1 SrcRegion=REGION(8,1) Type=uw
//
//
#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xDDD6:w
#endif

    mov (2)	MSGSRC.0<1>:ud	ORIX_LEFT<2;2,1>:w		// Block origin
    mov (1)	MSGSRC.2<1>:ud	0x000F0003:ud			// 4x16
    
// Transpose Y, save them to MRFs

//	16x4 Y src in GRF (each pix is specified as yx)
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|f1 e1 d1 c1 b1 a1 91 81 71 61 51 41 31 21 11 01 f0 e0 d0 c0 b0 a0 90 80 70 60 50 40 30 20 10 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|f3 e3 d3 c3 b3 a3 93 83 73 63 53 43 33 23 13 03 f2 e2 d2 c2 b2 a2 92 82 72 62 52 42 32 22 12 02|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//
//  First step		(16)	<1>	<=== <16;4,1>
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|73 63 53 43 72 62 52 42 71 61 51 41 70 60 50 40 33 23 13 03 32 22 12 02 31 21 11 01 30 20 10 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|f3 e3 d3 c3 f2 e2 d2 c2 f1 e1 d1 c1 f0 e0 d0 c0 b3 a3 93 83 b2 a2 92 82 b1 a1 91 81 b0 a0 90 80|
//	+-----------------------+-----------------------+-----------------------+-----------------------+

	// The first step
	mov (16)	BUF_B(0,0)<1>			PREV_MB_YB(0,0)<16;4,1>
	mov (16)	BUF_B(0,16)<1>			PREV_MB_YB(0,4)<16;4,1>
	mov (16)	BUF_B(1,0)<1>			PREV_MB_YB(0,8)<16;4,1>
	mov (16)	BUF_B(1,16)<1>			PREV_MB_YB(0,12)<16;4,1>

//
//  Second step		(16)	<1>	<=== <1;4,4>
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|73 72 71 70 63 62 61 60 53 52 51 50 43 42 41 40 33 32 31 30 23 22 21 20 13 12 11 10 03 02 01 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|f3 f2 f1 f0 e3 e2 e1 e0 d3 d2 d1 d0 c3 c2 c1 c0 b3 b2 b1 b0 a3 a2 a1 a0 93 92 91 90 83 82 81 80|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//
	// The second step
//	mov	(16)	MSGPAYLOADB(0,0)<1>		BUF_B(0,0)<32;8,4> 			// Read 2 rows, write 1 row
//	mov (16)	MSGPAYLOADB(0,16)<1>	BUF_B(0,1)<32;8,4>
//	mov (16)	MSGPAYLOADB(1,0)<1>		BUF_B(0,2)<32;8,4>
//	mov (16)	MSGPAYLOADB(1,16)<1>	BUF_B(0,3)<32;8,4>

	mov	(16)	MSGPAYLOADB(0,0)<1>		BUF_B(0,0)<1;4,4>
	mov (16)	MSGPAYLOADB(0,16)<1>	BUF_B(0,16)<1;4,4>
	mov (16)	MSGPAYLOADB(1,0)<1>		BUF_B(1,0)<1;4,4>
	mov (16)	MSGPAYLOADB(1,16)<1>	BUF_B(1,16)<1;4,4>

//  Transposed Y in 4x16 is ready for writting to dataport.
//
    send (8)	NULLREG	MSGHDR	MSGSRC<8;8,1>:ud	DAPWRITE	MSG_LEN(2)+DWBWMSGDSC+BI_DEST_Y				// Write 2 GRFs



/////////////////////////////////////////////////////////////////////////////////////////////////////

	// Transpose U/V, save them to MRFs in NV12 format
    asr (1)	MSGSRC.1:ud		MSGSRC.1:ud			1:w						// NV12 U+V block origin y = half of Y comp
    mov (1)	MSGSRC.2<1>:ud	0x00070003:ud								// NV12 U+V block width and height (4x8)


//	16x2 UV src in GRF (each pix is specified as yx)
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|71 71 61 61 51 51 41 41 31 31 21 21 11 11 01 01 70 70 60 60 50 50 40 40 30 30 20 20 10 10 00 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//
//	First step		(8)		<1>	<=== <8;4,1>:w
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|71 71 61 61 51 51 41 41 70 70 60 60 50 50 40 40 31 31 21 21 11 11 01 01 30 30 20 20 10 10 00 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
	mov (8)	BUF_W(0,0)<1>		PREV_MB_UW(0,0)<8;4,1>
	mov (8)	BUF_W(0,8)<1>		PREV_MB_UW(0,4)<8;4,1>

//	Second step		(8)		<1>	<=== <1;2,4>
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|71 71 70 70 61 61 60 60 51 51 50 50 41 41 40 40 31 31 30 30 21 21 20 20 11 11 10 10 01 01 00 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
	mov (8)	MSGPAYLOADW(0,0)<1>		BUF_W(0,0)<1;2,4>
	mov (8)	MSGPAYLOADW(0,8)<1>		BUF_W(0,8)<1;2,4>

//  Transposed U+V in NV12 in 4x8 is ready for writting to dataport.
 
    send (8)	NULLREG	MSGHDR	MSGSRC<8;8,1>:ud	DAPWRITE	MSG_LEN(1)+DWBWMSGDSC+BI_DEST_UV		// Write 1 GRF
    

