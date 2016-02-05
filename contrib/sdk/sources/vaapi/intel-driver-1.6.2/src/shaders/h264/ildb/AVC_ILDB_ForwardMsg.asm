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
// Each child thread write a byte into the root GRF r50 defiend in open Gataway.

#if defined(_DEBUG) 
mov		(1)		EntrySignatureC:w			0x7777:w
#endif

// Init payload to r0
mov (8) 	GatewayPayload<1>:ud 	0:w								//{ NoDDClr } 

// Forward a message:
// Offset = x relative to r50 (defiend in open gataway), x = ORIX >> 4 [bit 28:16]
// Need to shift left 16

// shift 2 more bits for byte to word offset

//shl	(1)		Offset_Length:ud		GateWayOffsetC:w	 	16:w		{ NoDDClr, NoDDChk }
shl	(1)		Offset_Length:ud		GateWayOffsetC:w	 	18:w		

// 2 bytes offset
add	(1)		Offset_Length:ud			Offset_Length:ud		0x00020000:d	{ NoDDClr }
	
// Length = 1 byte,	[bit 10:8 = 000]
//000 xxxxxxxxxxxxx 00000 000 00000000 ==> 000x xxxx xxxx xxxx 0000 0000 0000 0000

//mov (1) 	DispatchID:ub 			r0.20:ub		// Dispatch ID

//Move in EUid and Thread ID that we received from the PARENT thread
mov (1) 	EUID_TID:uw 			r0.6:uw								{ NoDDClr, NoDDChk }

mov (1) 	GatewayPayloadKey:uw 	0x1212:uw							{ NoDDClr, NoDDChk }	// Key

//mov	(4)		GatewayPayload<1>:ud	0:ud								{ NoDDClr, NoDDChk }	// Init payload low 4 dword

// Write back one byte (value = 0xFF) to root thread GRF to indicate this child thread is finished
// All lower 4 bytes must be assigned to the same byte value.
mov	(4)		GatewayPayload<1>:ub	0xFFFF:uw							{ NoDDChk }

// msg descriptor bit 15 set to '1' for notification
#ifdef GW_DCN
// For ILK, EOT bit should also be set to terminate the thread. This is to fix a timing related HW issue.
//
send (8)  	null:ud 		m0	  		GatewayPayload<8;8,1>:ud    MSG_GW_EOT	FWDMSGDSC+NOTIFYMSG
#else
send (8)  	null:ud 		m0	  		GatewayPayload<8;8,1>:ud    MSG_GW	FWDMSGDSC+NOTIFYMSG
#endif	// GW_DCN

//========== Forward Msg Done ========================================

