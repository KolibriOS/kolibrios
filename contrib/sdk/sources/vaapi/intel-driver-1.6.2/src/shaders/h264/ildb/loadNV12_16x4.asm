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
// Module Name: Loadnv12_16X4.Asm
//
// Load Nv12 16X4 Block 
//
//----------------------------------------------------------------
//  Symbols Need To Be Defined Before Including This Module
//
//	Source Region In :Ud
//	Src_Yd:			Src_Yd Base=Rxx Elementsize=4 Srcregion=Region(8,1) Type=Ud			// 3 Grfs (2 For Y, 1 For U+V)
//
//	Source Region Is :Ub.  The Same Region As :Ud Region
//	Src_Yb:			Src_Yb Base=Rxx Elementsize=1 Srcregion=Region(16,1) Type=Ub		// 2 Grfs
//	Src_Ub:			Src_Ub Base=Rxx Elementsize=1 Srcregion=Region(16,1) Type=Ub		// 0.5 Grf
//	Src_Vb:			Src_Vb Base=Rxx Elementsize=1 Srcregion=Region(16,1) Type=Ub		// 0.5 Grf
//
//	Binding Table Index: 
//	Bi_Src_Y:		Binding Table Index Of Y Surface
//	Bi_Src_UV:		Binding Table Index Of UV Surface (Nv12)
//
//	Temp Buffer:
//	Buf_D:			Buf_D Base=Rxx Elementsize=4 Srcregion=Region(8,1) Type=Ud
//	Buf_B:			Buf_B Base=Rxx Elementsize=1 Srcregion=Region(16,1) Type=Ub
//
//----------------------------------------------------------------

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xDDD2:w
#endif

	// Read Y
    mov (2)	MSGSRC.0<1>:ud	ORIX<2;2,1>:w		// Block origin
    mov (1)	MSGSRC.2<1>:ud	0x0003000F:ud		// Block width and height (16x4)
    send (8) PREV_MB_YD(0)<1>	MSGHDRY	MSGSRC<8;8,1>:ud	DAPREAD	RESP_LEN(2)+DWBRMSGDSC_RC+BI_SRC_Y	// Read 2 GRFs

	// Read U+V
    asr (1)	MSGSRC.1:ud		MSGSRC.1:ud			1:w						// NV12 U+V block origin y = half of Y comp
    mov (1)	MSGSRC.2<1>:ud	0x0001000F:ud		// NV12 U+V block width and height (16x2)

	// Load NV12 U+V tp a temp buf  
	send (8) BUF_D(0)<1>	MSGHDRU	MSGSRC<8;8,1>:ud	DAPREAD	RESP_LEN(1)+DWBRMSGDSC_RC+BI_SRC_UV	// Read 1 GRF

	// Convert NV12 U+V to internal planar U and V and place them right after Y.
//	mov (16)	SRC_UB(0,0)<1>		BUF_B(0,0)<32;16,2>
//	mov (16)	SRC_VB(0,0)<1>		BUF_B(0,1)<32;16,2>	
	
// End of loadNV12_16x4.asm
