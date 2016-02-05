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
// Module name: WriteURB_Child.asm
//
// General purpose module to write data to URB using the URB handle/offset in r0
//
//----------------------------------------------------------------
//	Assume:
//	- a0.0 and a0.1 is meg desc, has been assign with URB offset and msg size
//	- MRFs are alrady assigned with data.
//----------------------------------------------------------------
//
//  16x16 byte pixel block can be saved using just 1 "send" instruction.

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0x3535:w
#endif

// URB write header:
//mov (8) MSGSRC.0:ud 	r0.0<8;8,1>:ud			// Copy parent R0 header

//shr (1)	Temp2_W:uw	URBOffsetC:uw	1:w	// divide by 2, because URB entry is counted by 512bits.  Offset is counted by 256bits.
//add (1) MSGSRC.0:uw		r0.0:uw		Temp2_W:uw	

shr (1)	MSGSRC.0:uw		URBOffsetC:uw	1:w	// divide by 2, because URB entry is counted by 512bits.  Offset is counted by 256bits.

//mov (1) MSGSRC.1:ud 	0:ud					// Reset Handle 1

	// URB write 1 MRFs, 
	// Current MB offset is in URBOffset, use it as write origin
	// Add 2 to offset to store data be be passed to the right MB

send  null:uw 	m0	  MSGSRC<8;8,1>:uw		URBWRITE	MSG_LEN(1)+URBWMSGDSC+0x20 // URB write
