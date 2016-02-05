/*
 * Save decoded U/V picture data to frame buffer
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
#if !defined(__SAVE_8x8_UV__)		// Make sure this is only included once
#define __SAVE_8x8_UV__

// Module name: save_8x8_UV.asm
//
// Save decoded U/V picture data to frame buffer
//

    mov (1) MSGSRC.2:ud	    0x0007000F:ud {NoDDClr}		// Block width and height (16x8)
    mov (2) MSGSRC.0<1>:ud  I_ORIX<2;2,1>:w	{NoDDChk}	// I_ORIX has already been adjusted for NV12

//  Update message descriptor based on previous read setup
//
#ifdef DEV_ILK
    add (1)		MSGDSC	MSGDSC MSG_LEN(4)+DWBWMSGDSC_WC-DWBRMSGDSC_RC-0x00100000:ud  // Set message descriptor
#else
    add (1)		MSGDSC	MSGDSC MSG_LEN(4)+DWBWMSGDSC_WC-DWBRMSGDSC_RC-0x00010000:ud  // Set message descriptor
#endif	// DEV_ILK

// Write U/V picture data
//
#ifndef MONO
    mov	    MSGPAYLOAD(0,0)<1>	DEC_UV(0)REGION(16,2)	// U/V row 0
    mov	    MSGPAYLOAD(0,16)<1>	DEC_UV(1)REGION(16,2)	// U/V row 1
    mov	    MSGPAYLOAD(1,0)<1>	DEC_UV(2)REGION(16,2)	// U/V row 2
    mov	    MSGPAYLOAD(1,16)<1>	DEC_UV(3)REGION(16,2)	// U/V row 3
    mov	    MSGPAYLOAD(2,0)<1>	DEC_UV(4)REGION(16,2)	// U/V row 4
    mov	    MSGPAYLOAD(2,16)<1>	DEC_UV(5)REGION(16,2)	// U/V row 5
    mov	    MSGPAYLOAD(3,0)<1>	DEC_UV(6)REGION(16,2)	// U/V row 6
    mov	    MSGPAYLOAD(3,16)<1>	DEC_UV(7)REGION(16,2)	// U/V row 7
#else	// defined(MONO)
    $for(0; <4; 2) {
	mov (16)	MSGPAYLOADD(%1)<1>		0x80808080:ud {Compr}
	}

#endif	// !defined(MONO)

	send (8)	REG_WRITE_COMMIT_UV<1>:ud	MSGHDR	MSGSRC<8;8,1>:ud	DAPWRITE	MSGDSC

// End of save_8x8_UV

#endif	// !defined(__SAVE_8x8_UV__)
