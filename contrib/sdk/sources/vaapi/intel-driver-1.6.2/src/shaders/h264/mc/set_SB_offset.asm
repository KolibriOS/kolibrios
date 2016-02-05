/*
 * Common module to set offset into the scoreboard
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
// Module name: set_SB_offset.asm
//
// Common module to set offset into the scoreboard
//	Note: This is to encapsulate the way M0.5:ud in ForwardMsg is filled.
//
//  $Revision: 2 $
//  $Date: 10/16/06 5:19p $
//
	add (1)		MSGHDRY1.10<1>:uw r0.20:ub	0x0200:uw			// Message length = 1 DWORD

	add	(16)	acc0<1>:w	r0.12<0;1,0>:uw	-LEADING_THREAD:w	// 0-based thread count derived from r0.6:ud
	shl (1)		M05_STORE<1>:uw		acc0<0;1,0>:uw	0x2:uw		// Store for future "update" use, in DWORD unit
	and	(16)	acc0<1>:w	acc0<16;16,1>:uw	SB_MASK:uw		// Wrap around scoreboard
	shl (1)		MSGHDRY1.11<1>:uw	acc0<0;1,0>:uw	0x2:uw		// Convert to DWORD offset

// End of set_SB_offset