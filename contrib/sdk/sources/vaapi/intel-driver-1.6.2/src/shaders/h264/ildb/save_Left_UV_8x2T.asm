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
// Module name: save_Left_UV_8x2T.asm
//
// Transpose 8x2 to 2x8 UV data and write to memory 
//
//----------------------------------------------------------------
//  Symbols need to be defined before including this module
//
//	Left MB region:
//	PREV_MB_UW: 	Base=ryy 	ElementSize=2 SrcRegion=REGION(8,1) Type=uw

//	Binding table index: 
//	BI_SRC_UV:		Binding table index of UV surface (NV12)
//
//	Temp buffer:
//	BUF_W:			BUF_W Base=rxx ElementSize=1 SrcRegion=REGION(8,1) Type=uw
//
//
#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xDDD6:w
#endif

#if defined(_FIELD)
    and.nz.f0.1 (1)  NULLREGW 	BitFields:w  	BotFieldFlag:w			// Get bottom field flag
#endif

	// Transpose U/V, save them to MRFs in NV12 format
    mov (1)	MSGSRC.0:ud		ORIX_LEFT:w						{ NoDDClr }			// Block origin
    asr (1)	MSGSRC.1:ud		ORIY_LEFT:w			1:w			{ NoDDClr, NoDDChk }	// NV12 U+V block origin y = half of Y comp
    mov (1)	MSGSRC.2:ud		0x00070003:ud					{ NoDDChk }			// NV12 U+V block width and height (4x8)


//	16x2 UV src in GRF (each pix is specified as yx)
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|71 71 61 61 51 51 41 41 31 31 21 21 11 11 01 01 70 70 60 60 50 50 40 40 30 30 20 20 10 10 00 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//
//	First step		(8)		<1>	<=== <8;4,1>:w
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|71 71 61 61 51 51 41 41 70 70 60 60 50 50 40 40 31 31 21 21 11 11 01 01 30 30 20 20 10 10 00 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
	mov (8)	LEFT_TEMP_W(0,0)<1>		PREV_MB_UW(0,0)<8;4,1>		{ NoDDClr }
	mov (8)	LEFT_TEMP_W(0,8)<1>		PREV_MB_UW(0,4)<8;4,1>		{ NoDDChk }

//	Second step		(8)		<1>	<=== <1;2,4>
//	+-----------------------+-----------------------+-----------------------+-----------------------+
//	|71 71 70 70 61 61 60 60 51 51 50 50 41 41 40 40 31 31 30 30 21 21 20 20 11 11 10 10 01 01 00 00|
//	+-----------------------+-----------------------+-----------------------+-----------------------+
	mov (8)	MSGPAYLOADW(0,0)<1>		LEFT_TEMP_W(0,0)<1;2,4>
	mov (8)	MSGPAYLOADW(0,8)<1>		LEFT_TEMP_W(0,8)<1;2,4>

//  Transposed U+V in NV12 in 4x8 is ready for writting to dataport.
 
#if defined(_PROGRESSIVE) 
	mov (1)	MSGDSC	MSG_LEN(1)+DWBWMSGDSC+BI_DEST_UV:ud
//    send (8)	NULLREG		MSGHDR		MSGSRC<8;8,1>:ud	DWBWMSGDSC+0x00100000+BI_DEST_UV
#endif

#if defined(_FIELD)
	// Field picture
    (f0.1) mov (1)	MSGDSC	MSG_LEN(1)+DWBWMSGDSC+ENMSGDSCBF+BI_DEST_UV:ud  // Write 1 GRF to DEST_UV bottom field
    (-f0.1) mov (1)	MSGDSC	MSG_LEN(1)+DWBWMSGDSC+ENMSGDSCTF+BI_DEST_UV:ud  // Write 1 GRF to DEST_UV top field

#endif
    send (8)	null:ud		MSGHDR		MSGSRC<8;8,1>:ud	DAPWRITE	MSGDSC
