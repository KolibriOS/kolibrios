/*
 * Scoreboard interrupt handler
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
// Kernel name: scoreboard_sip.asm
//
// scoreboard interrupt handler
//
// Simply send a notification message to scoreboard thread

    mov (8)		m0<1>:ud	0x00000000:ud			// Initialize message header payload with 0
#ifdef	DOUBLE_SB
	mov (1)		m0.5<1>:ud	0x08000200:ud			// Message length = 1 DWORD, sent to GRF offset 64 registers
#else
	mov (1)		m0.5<1>:ud	0x04000200:ud			// Message length = 1 DWORD, sent to GRF offset 32 registers
#endif
	send (8)	null<1>:ud  m0	null:ud    0x03108002	// Send notification message to scoreboard kernel

	and (1)		cr0.1:ud	cr0.1:ud	0x00800000		// Clear preempt exception bit
	and (1)		cr0.0:ud	cr0.0:ud	0x7fffffff:ud	// Exit SIP routine
	nop													// Required by B-spec

.end_code






