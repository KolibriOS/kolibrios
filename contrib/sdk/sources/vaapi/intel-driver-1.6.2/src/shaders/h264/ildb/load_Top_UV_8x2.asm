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
// Module Name: Load_Top_UV_8X2.Asm
//
// Load UV 8X2 Block 
//
//----------------------------------------------------------------
//  Symbols ceed To be defined before including this module
//
//	Source Region Is :UB
//	BUF_D:			BUF_D Base=Rxx Elementsize=4 Srcregion=Region(8,1) Type=UD

//	Binding Table Index: 
//	BI_SRC_UV:		Binding Table Index Of UV Surface (NV12)
//
//----------------------------------------------------------------

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xDDD2:w
#endif

#if defined(_PROGRESSIVE) 
	// Read U+V
    mov (1)	MSGSRC.0:ud		ORIX_TOP:w						{ NoDDClr }			// Block origin
    asr (1)	MSGSRC.1:ud		ORIY_TOP:w			1:w			{ NoDDClr, NoDDChk }	// NV12 U+V block origin y = half of Y comp
    mov (1)	MSGSRC.2:ud		0x0001000F:ud					{ NoDDChk }			// NV12 U+V block width and height (16x2)

	// Read 1 GRF from DEST surface as the above MB has been deblocked.
	//send (8) TOP_MB_UD(0)<1>	MSGHDRU		MSGSRC<8;8,1>:ud	RESP_LEN(1)+DWBRMSGDSC_RC+BI_DEST_UV	
	mov (1)	MSGDSC	RESP_LEN(1)+DWBRMSGDSC_RC+BI_DEST_UV:ud	
#endif

#if defined(_FIELD)

//    cmp.z.f0.0 (1)  NULLREGW PicTypeC:w  	0:w							// Get pic type flag
    and.nz.f0.1 (1)  NULLREGW 	BitFields:w  	BotFieldFlag:w			// Get bottom field flag
	// They are used later in this file

	// Read U+V
    mov (1)	MSGSRC.0:ud		ORIX_TOP:w						{ NoDDClr }			// Block origin
    asr (1)	MSGSRC.1:ud		ORIY_TOP:w			1:w			{ NoDDClr, NoDDChk }	// NV12 U+V block origin y = half of Y comp
    mov (1)	MSGSRC.2:ud		0x0001000F:ud					{ NoDDChk }			// NV12 U+V block width and height (16x2)

	// Load NV12 U+V 
	
    // Set message descriptor
    // Frame picture
//    (f0.0) mov (1)	MSGDSC	DWBRMSGDSC_RC+0x00010000+BI_DEST_UV:ud			// Read 1 GRF from SRC_UV
//	(f0.0) jmpi		Load_Top_UV_8x2

	// Field picture
    (f0.1) mov (1)	MSGDSC	RESP_LEN(1)+DWBRMSGDSC_RC_BF+BI_DEST_UV:ud  // Read 1 GRF from SRC_Y bottom field
    (-f0.1) mov (1)	MSGDSC	RESP_LEN(1)+DWBRMSGDSC_RC_TF+BI_DEST_UV:ud  // Read 1 GRF from SRC_Y top field

//Load_Top_UV_8x2:

	// Read 1 GRF from DEST surface as the above MB has been deblocked.
//	send (8) PREV_MB_UD(0)<1>	MSGHDRU		MSGSRC<8;8,1>:ud	MSGDSC	

#endif

	send (8) TOP_MB_UD(0)<1>	MSGHDRU		MSGSRC<8;8,1>:ud	DAPREAD	MSGDSC	
		
// End of load_Top_UV_8x2.asm
