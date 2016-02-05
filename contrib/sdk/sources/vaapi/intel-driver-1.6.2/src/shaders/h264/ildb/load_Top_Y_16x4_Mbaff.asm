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
// Module Name: Load_Y_16X4.asm
//
// Load Y 16X4 Block to PREV_MB_YD
//
//----------------------------------------------------------------
//  Symbols Need To Be Defined Before Including This Module
//
//	Source Region In :Ud
//	Src_YD:			Src_Yd Base=Rxx Elementsize=4 Srcregion=Region(8,1) Type=Ud			// 3 Grfs (2 For Y, 1 For U+V)
//
//	Source Region Is :Ub.  The Same Region As :Ud Region
//	Src_YB:			Src_Yb Base=Rxx Elementsize=1 Srcregion=Region(16,1) Type=Ub		// 2 Grfs
//
//	Binding Table Index: 
//	Bi_Src_Y:		Binding Table Index Of Y Surface
//
//	Temp Buffer:
//	Buf_D:			Buf_D Base=Rxx Elementsize=4 Srcregion=Region(8,1) Type=Ud
//	Buf_B:			Buf_B Base=Rxx Elementsize=1 Srcregion=Region(16,1) Type=Ub
//
//----------------------------------------------------------------

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xDDD2:w
#endif
    // FieldModeCurrentMbFlag determines how to access above MB
	and.z.f0.0 (1) 	null:w		r[ECM_AddrReg, BitFlags]:ub		FieldModeCurrentMbFlag:w		

    and.nz.f0.1 (1) NULLREGW 	BitFields:w   BotFieldFlag:w

	// Read Y
    mov (2)	MSGSRC.0<1>:ud	ORIX_TOP<2;2,1>:w		{ NoDDClr }		// Block origin
    mov (1)	MSGSRC.2<1>:ud	0x0003000F:ud			{ NoDDChk }		// Block width and height (16x4)
   
    // Set message descriptor

	(f0.0)	if	(1)		ELSE_Y_16x4

    // Frame picture
    mov (1)	MSGDSC	RESP_LEN(2)+DWBRMSGDSC_RC+BI_DEST_Y:ud			// Read 2 GRFs from SRC_Y

	// Add vertical offset 16 for bot MB in MBAFF mode
	(f0.1) add (1)	MSGSRC.1:d	MSGSRC.1:d		16:w		
	
	// Dual field mode setup
	and.z.f0.1 (1) NULLREGW		DualFieldMode:w		1:w
	(f0.1) jmpi NOT_DUAL_FIELD

    add (1)	MSGSRC.1:d		MSGSRC.1:d		-4:w	{ NoDDClr }		// Load 8 lines in above MB
	mov (1)	MSGSRC.2:ud		0x0007000F:ud			{ NoDDChk }		// New block width and height (16x8)
	
	add (1) MSGDSC			MSGDSC			RESP_LEN(2):ud	// 2 more GRF to receive

NOT_DUAL_FIELD:

ELSE_Y_16x4: 
	else 	(1)		ENDIF_Y_16x4

	asr (1)	MSGSRC.1:d		ORIY_CUR:w		1:w		// Reduce y by half in field access mode

	// Field picture
    (f0.1) mov (1)	MSGDSC	RESP_LEN(2)+DWBRMSGDSC_RC_BF+BI_DEST_Y:ud  // Read 2 GRFs from SRC_Y bottom field
    (-f0.1) mov (1)	MSGDSC	RESP_LEN(2)+DWBRMSGDSC_RC_TF+BI_DEST_Y:ud  // Read 2 GRFs from SRC_Y top field

	add (1)	MSGSRC.1:d		MSGSRC.1:d		-4:w	// for last 4 rows of above MB

	endif
ENDIF_Y_16x4:
        
    // Read 2 GRFs from DEST surface, as the above MB has been deblocked
    send (8) PREV_MB_YD(0)<1>	MSGHDRY	MSGSRC<8;8,1>:ud	DAPREAD	MSGDSC

// End of load_Y_16x4.asm
