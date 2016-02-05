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
// Module Name: Load_Cur_UV_Right_Most_2X8.Asm

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xDDD0:w
#endif

#if defined(_PROGRESSIVE) 
	// Read U+V, (UV MB size = 16x8)
    add (1)	MSGSRC.0:ud		ORIX_CUR:w			12:w			{ NoDDClr }		// Block origin
    asr (1)	MSGSRC.1:ud		ORIY_CUR:w			1:w				{ NoDDClr, NoDDChk }		// NV12 U+V block origin y = half of Y comp
    mov (1)	MSGSRC.2:ud		0x00070003:ud						{ NoDDChk }		// NV12 U+V block width and height (4x8)
	send (8) LEFT_TEMP_D(0)<1>	MSGHDRU	MSGSRC<8;8,1>:ud	DAPREAD	RESP_LEN(1)+DWBRMSGDSC_RC+BI_DEST_UV	
#endif

#if defined(_FIELD) || defined(_MBAFF)

    // FieldModeCurrentMbFlag determines how to access left MB
	and.z.f0.0 (1) 	null:w		r[ECM_AddrReg, BitFlags]:ub		FieldModeCurrentMbFlag:w		

    and.nz.f0.1 (1)	NULLREGW 		BitFields:w  	BotFieldFlag:w				// Get bottom field flag

	// Read U+V
    add (1)	MSGSRC.0:ud		ORIX_CUR:w			12:w				{ NoDDClr }		// Block origin
    asr (1)	MSGSRC.1:ud		ORIY_CUR:w			1:w				{ NoDDClr, NoDDChk }		// NV12 U+V block origin y = half of Y comp
    mov (1)	MSGSRC.2:ud		0x00070003:ud						{ NoDDChk }		// NV12 U+V block width and height (4x8)

	// Load NV12 U+V 
	
    // Set message descriptor

	(f0.0)	if	(1)		ILDB_LABEL(ELSE_Y_2x8T)

    // Frame picture
    mov (1)	MSGDSC	RESP_LEN(1)+DWBRMSGDSC_RC+BI_DEST_UV:ud			// Read 1 GRF from SRC_UV

	(f0.1) add (1)	MSGSRC.1:d		MSGSRC.1:d		8:w		// Add vertical offset 8 for bot MB in MBAFF mode

ILDB_LABEL(ELSE_Y_2x8T): 
	else 	(1)		ILDB_LABEL(ENDIF_Y_2x8T)

	// Field picture
    (f0.1) mov (1)	MSGDSC	RESP_LEN(1)+DWBRMSGDSC_RC_BF+BI_DEST_UV:ud  // Read 1 GRF from SRC_Y bottom field
    (-f0.1) mov (1)	MSGDSC	RESP_LEN(1)+DWBRMSGDSC_RC_TF+BI_DEST_UV:ud  // Read 1 GRF from SRC_Y top field

	endif
ILDB_LABEL(ENDIF_Y_2x8T):

	// Read 1 GRF from DEST surface as the above MB has been deblocked.
//	send (8) BUF_D(0)<1>	MSGHDRU	MSGSRC<8;8,1>:ud	MSGDSC	
	send (8) LEFT_TEMP_D(0)<1>	MSGHDRU	MSGSRC<8;8,1>:ud	DAPREAD	MSGDSC	

#endif

