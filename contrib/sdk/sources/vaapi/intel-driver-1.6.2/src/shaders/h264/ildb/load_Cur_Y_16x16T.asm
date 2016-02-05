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
// Module name: load_Y_16x16T.asm
//
// Load and transpose Y 16x16 block 
//
//----------------------------------------------------------------
//  Symbols need to be defined before including this module
//
//	Source region in :ud
//	SRC_YD:			SRC_YD Base=rxx ElementSize=4 SrcRegion=REGION(8,1) Type=ud			// 8 GRFs
//
//	Binding table index: 
//	BI_SRC_Y:		Binding table index of Y surface
//
//----------------------------------------------------------------

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xDDD1:w
#endif
	// Read Y
	
#if defined(_PROGRESSIVE) 
    mov (2)	MSGSRC.0<1>:ud	ORIX_CUR<2;2,1>:w		{ NoDDClr }			// Block origin
	mov (1)	MSGSRC.2<1>:ud	0x000F000F:ud			{ NoDDChk }			// Block width and height (16x16)

    //send (8) SRC_YD(0)<1>	MSGHDRC		MSGSRC<8;8,1>:ud	DWBRMSGDSC_SMPLR+0x00080000+BI_SRC_Y
	mov (1)	MSGDSC	RESP_LEN(8)+DWBRMSGDSC_SC+BI_SRC_Y:ud    	
#endif

    
#if defined(_FIELD)
//    cmp.z.f0.0 (1)  NULLREGW 	PicTypeC:w  	0:w						// Get pic type flag
    and.nz.f0.1 (1) NULLREGW 	BitFields:w  	BotFieldFlag:w			// Get bottom field flag
	// they are used later in this file

    mov (2)	MSGSRC.0<1>:ud	ORIX_CUR<2;2,1>:w		{ NoDDClr }			// Block origin
    mov (1)	MSGSRC.2<1>:ud	0x000F000F:ud			{ NoDDChk }			// Block width and height (16x16)
    
    // Set message descriptor
    // Frame picture
//	(f0.0) mov (1)	MSGDSC	RESP_LEN(8)+DWBRMSGDSC_SC+BI_SRC_Y:ud			// Read 8 GRFs from SRC_Y
//	(f0.0) jmpi		load_Y_16x16T

	// Non frame picture
    (f0.1) mov (1)	MSGDSC	RESP_LEN(8)+DWBRMSGDSC_SC_BF+BI_SRC_Y:ud  // Read 8 GRFs from SRC_Y bottom field
    (-f0.1) mov (1)	MSGDSC	RESP_LEN(8)+DWBRMSGDSC_SC_TF+BI_SRC_Y:ud  // Read 8 GRFs from SRC_Y top field

//load_Y_16x16T:

#endif

    send (8) SRC_YD(0)<1>	MSGHDRC		MSGSRC<8;8,1>:ud	DAPREAD	MSGDSC
    	
//	#include "Transpose_Cur_Y_16x16.asm"

// End of load_Y_16x16T
