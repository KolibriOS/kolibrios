/*
 * Scoreboard update function for decoding kernels
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
//
// Module name: scoreboard_update.asm
//
//	Scoreboard update function for decoding kernels
//
//	This module is used by decoding kernels to send message to scoreboard to update the
//	"complete" status, thus the dependency of the MB can be cleared.
//
//  $Revision: 6 $
//  $Date: 10/16/06 5:19p $
//
    mov (8)		MSGHDRY1<1>:ud	0x00000000:ud				// Initialize message header payload with 0

	// Compose M0.5:ud information
	add (1)	MSGHDRY1.10<1>:uw	r0.20:ub	0x0200:uw				// Message length = 1 DWORD
	and (1) MSGHDRY1.11<1>:uw	M05_STORE<0;1,0>:uw	SB_MASK*4:uw	// Retrieve stored value and wrap around scoreboard

	or (1)	MSGHDRY1.0<1>:ud	M05_STORE<0;1,0>:uw	0xc0000000:ud	// Set "Completed" bits

#ifndef BSDRESET_ENABLE
#ifdef INTER_KERNEL
	mov	(1)	gREG_WRITE_COMMIT_Y<1>:ud	gREG_WRITE_COMMIT_Y<0;1,0>:ud		// Make sure Y write is committed
	mov	(1)	gREG_WRITE_COMMIT_UV<1>:ud	gREG_WRITE_COMMIT_UV<0;1,0>:ud		// Make sure U/V write is committed
#else
	mov	(1)	REG_WRITE_COMMIT_Y<1>:ud	REG_WRITE_COMMIT_Y<0;1,0>:ud		// Make sure Y write is committed
	mov	(1)	REG_WRITE_COMMIT_UV<1>:ud	REG_WRITE_COMMIT_UV<0;1,0>:ud		// Make sure U/V write is committed
#endif	// INTER_KERNEL
#endif	// BSDRESET_ENABLE

	send (8)	NULLREG  MSGHDRY1	null:ud    MSG_GW	FWDMSGDSC

// End of scoreboard_update
