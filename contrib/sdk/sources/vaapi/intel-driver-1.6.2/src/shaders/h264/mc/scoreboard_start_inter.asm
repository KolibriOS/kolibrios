/*
 * Scoreboard function for starting inter prediction kernels
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
#if !defined(__SCOREBOARD_START_INTER__)
#define __SCOREBOARD_START_INTER__
//
// Module name: scoreboard_start_inter.asm
//
//	Scoreboard function for starting inter prediction kernels
//	This function is only used by inter prediction kernels to send message to
//	scoreboard in order to announce the inter kernel has started
//
//  $Revision: 5 $
//  $Date: 10/18/06 4:11p $
//
scoreboard_start_inter:

// First open message gateway since intra kernels need wake-up message to resume
// 
    mov (8)	MSGHDRY0<1>:ud	0x00000000:ud			// Initialize message header payload with 0

    // Send a message with register base RegBase = r0 (0x0) and Size = 0x0
    // 000 00000000 00000 00000 000 00000000 ==> 0000 0000 0000 0000 0000 0000 0000 0000
    // ---------------------------------------------------------------------------------
	send (8)	NULLREG  MSGHDRY0	null:ud    MSG_GW	OGWMSGDSC

//	Derive the scoreboard location where the inter thread writes to
//
    mov (8)		MSGHDRY1<1>:ud	0x00000000:ud			// Initialize message header payload with 0

	// Compose M0.5:ud
	#include "set_SB_offset.asm"

	// Compose M0.0:ud, i.e. message payload
	or	(1)		MSGHDRY1.1<1>:uw	sr0.0<0;1,0>:uw		0x0000:uw	// Set EUID/TID bits + inter start bit

	send (8)	NULLREG  MSGHDRY1	null:ud    MSG_GW	FWDMSGDSC+NOTIFYMSG	// Send "Inter start" message to scoreboard kernel

    RETURN

#endif	// !defined(__SCOREBOARD_START_INTER__)
