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

#if defined(_PROGRESSIVE) 
	// Read Y
    mov (2)	MSGSRC.0<1>:ud	ORIX_TOP<2;2,1>:w		{ NoDDClr }		// Block origin
    mov (1)	MSGSRC.2<1>:ud	0x0003000F:ud			{ NoDDChk }		// Block width and height (16x4)

    mov (1)	MSGDSC	RESP_LEN(2)+DWBRMSGDSC_RC+BI_DEST_Y:ud			// Read 2 GRFs from SRC_Y
#endif

#if defined(_FIELD)

//    cmp.z.f0.0 (1)  NULLREGW 	PicTypeC:w  	0:w						// Get pic type flag
    and.nz.f0.1 (1) NULLREGW 	BitFields:w  	BotFieldFlag:w			// Get bottom field flag
	// they are used later in this file
	
    mov (2)	MSGSRC.0<1>:ud	ORIX_TOP<2;2,1>:w		{ NoDDClr }		// Block origin
    mov (1)	MSGSRC.2<1>:ud	0x0003000F:ud			{ NoDDChk }		// Block width and height (16x4)
   
    // Set message descriptor

    // Frame picture
//	(f0.0) mov (1)	MSGDSC	DWBRMSGDSC_RC+0x00020000+BI_DEST_Y:ud			// Read 2 GRFs from SRC_Y
//	(f0.0) jmpi		load_Y_16x4

	// Field picture
    (f0.1) mov (1)	MSGDSC	RESP_LEN(2)+DWBRMSGDSC_RC_BF+BI_DEST_Y:ud  // Read 2 GRFs from SRC_Y bottom field
    (-f0.1) mov (1)	MSGDSC	RESP_LEN(2)+DWBRMSGDSC_RC_TF+BI_DEST_Y:ud  // Read 2 GRFs from SRC_Y top field

//load_Y_16x4:
    // Read 2 GRFs from DEST surface, as the above MB has been deblocked
//    send (8) PREV_MB_YD(0)<1>	MSGHDRY		MSGSRC<8;8,1>:ud	MSGDSC
    
#endif
    
    send (8) TOP_MB_YD(0)<1>	MSGHDRT		MSGSRC<8;8,1>:ud	DAPREAD	MSGDSC
    	    
// End of load_Y_16x4.asm
