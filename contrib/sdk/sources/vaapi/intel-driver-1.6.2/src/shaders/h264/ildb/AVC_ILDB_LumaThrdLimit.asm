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
//========== Forward message to root thread through gateway ==========

// Chroma root kenrel updates luma thread limit.

#if defined(_DEBUG) 
mov		(1)		EntrySignatureC:w			0x7788:w
#endif

// Init payload to r0
mov (8) 	GatewayPayload<1>:ud 	0:w								{ NoDDClr } 

// Forward a message:
// Offset = x relative to r50 (defiend in open gataway), x = ORIX >> 4 [bit 28:16]
// Need to shift left 16

mov	(1)		Offset_Length:ud		THREAD_LIMIT_OFFSET:ud	 			{ NoDDClr, NoDDChk }

// Length = 1 byte,	[bit 10:8 = 000]
//000 xxxxxxxxxxxxx 00000 000 00000000 ==> 000x xxxx xxxx xxxx 0000 0000 0000 0000

//mov (1) 	DispatchID:ub 			r0.20:ub		// Dispatch ID

//  Copy EUid and Thread ID that we received from the PARENT thread
mov (1) 	EUID_TID:uw 			r0.6:uw								{ NoDDClr, NoDDChk }

mov (1) 	GatewayPayloadKey:uw 	0x1212:uw							{ NoDDChk }	// Key

//mov	(4)		GatewayPayload<1>:ud	0:ud								{ NoDDClr, NoDDChk }	// Init payload low 4 dword

// Write back one byte (value = 0xFF) to root thread GRF to indicate this child thread is finished
// All lower 4 bytes must be assigned to the same byte value.
add	(1)		Temp1_W:w				MaxThreads:uw	-OutstandingThreads:uw
mov	(4)		GatewayPayload<1>:ub	Temp1_B<0;1,0>:ub 

send (8)  	GatewayResponse:ud 		m0	  		GatewayPayload<8;8,1>:ud    MSG_GW	FWDMSGDSC

//========== Forward Msg Done ========================================

