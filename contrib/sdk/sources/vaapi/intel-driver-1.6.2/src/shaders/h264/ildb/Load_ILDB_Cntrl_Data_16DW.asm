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
// Module name: load_ILDB_Cntrl_Data_16DW.asm
//
// This module loads AVC ILDB 64DW control data for one MB CTG. 
// Dataport expands from 16DW to 64DW.
//
//----------------------------------------------------------------
//  Symbols need to be defined before including this module
//
//	Source region in :ud
//	CNTRL_DATA_D:	CNTRL_DATA_D Base=rxx ElementSize=4 SrcRegion=REGION(8,1) Type=ud			// 8 GRFs
//
//	Binding table index: 
//	BI_CNTRL_DATA:	Binding table index of control data surface
//
//----------------------------------------------------------------

// On CTG, MB control data in memory occupy 16 DWs. Data port returns 8 GRFs with expanded control data.

#if defined(_MBAFF) 
	// We need to get control data offset for the bottom MB in mbaff mode.
	// That is, get f0.1=1 if MbaffFlag==1 && BotFieldFlag==1
//	and (1)	CTemp1_W:uw 		BitFields:uw  	MbaffFlag+BotFieldFlag:uw	// Mute all other bits
//	cmp.e.f0.1 (1) NULLREGW 	CTemp1_W:uw  	MbaffFlag+BotFieldFlag:uw	// Check mbaff and bot flags
		
	and.ne.f0.1 (1)	NULLREGW 	BitFields:uw  	BotFieldFlag:uw

	// Global offset
	mov (1)	MSGSRC.2:ud		CntrlDataOffsetY:ud
		
	(f0.1) add (1) MSGSRC.2:ud		MSGSRC.2:ud		64:w	// +64 to the next MB control data (bot MB)
#endif

    send (8) CNTRL_DATA_D(0)<1>	MSGHDRY	MSGSRC<8;8,1>:ud	DAPREAD	RESP_LEN(8)+ILDBRMSGDSC+BI_CNTRL_DATA	// Receive 8 GRFs

// End of load_ILDB_Cntrl_Data_16DW.asm



// AVC ILDB control data message header format

//DWord	Bit	Description
//M0.7	31:0	Debug 
//M0.6	31:0	Debug
//M0.5	31:8	Ignored
//		7:0		Dispatch ID. // This ID is assigned by the fixed function unit and is a unique identifier for the thread.  It is used to free up resources used by the thread upon thread completion.
//M0.4	31:0	Ignored
//M0.3	31:0	Ignored
//M0.2	31:0	Global Offset. Specifies the global byte offset into the buffer.
				//	This offset must be OWord aligned (bits 3:0 MBZ) Format = U32 Range = [0,FFFFFFF0h]
//M0.1	31:0	Ignored
//M0.0	31:0	Ignored



