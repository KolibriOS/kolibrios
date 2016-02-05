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
//----- Open a Message Gateway -----
// The parent thread is the recipient thread

#if defined(_DEBUG) 
	mov		(1)		EntrySignature:w			0x1111:w
#endif

mov (8) 	GatewayPayload<1>:ud 		r0.0<8;8,1>:ud			// Init payload to r0

// r50- (16 GRFs) are the GRFs child thread can wtite to. 

// Reg base is at bit 28:21, Gateway size is at [bit 10:8]
// r6: 6 = 00000110
//000 00000110 0000000000 100 00000000 ==> 0000 0000 1100 0000 0000 0100 0000 0000
mov (1) 	RegBase_GatewaySize:ud 	0x00C00400:ud	// Reg base + Gateway size (16 GRFs)


//000 00110010 0000000000 100 00000000 ==> 0000 0110 0100 0000 0000 0100 0000 0000
//mov (1) 	RegBase_GatewaySize:ud 	0x06400400:ud	// Reg base (r50 = 0x640 byte offset) + Gateway size (16 GRFs)

//mov (1) 	DispatchID:ub 			r0.20:ub		// Dispatch ID
mov (1) 	GatewayPayloadKey:uw 	0x1212:uw		// Key=0x1212

// Message descriptor
// bit 31	EOD
// 27:24	FFID = 0x0011 for msg gateway
// 23:20	msg length = 1 MRF
// 19:16	Response length	= 0
// 14		AckReg = 1
// 1:0		SubFuncID = 00 for OpenGateway
// Message descriptor: 0 000 0011 0001 0000 + 0 1 000000000000 00 ==> 0000 0011 0001 0000 0100 0000 0000 0000
// Send message to gateway: the ack message is put into response GRF r49 ==> Good for debugging
send (8)  	GatewayResponse:ud	m7	  GatewayPayload<8;8,1>:ud    MSG_GW	OGWMSGDSC

//----- End of Open a Message Gateway -----
