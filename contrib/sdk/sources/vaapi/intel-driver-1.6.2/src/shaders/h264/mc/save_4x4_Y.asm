/*
 * Save Intra_4x4 decoded Y picture data to frame buffer
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
#if !defined(__SAVE_4X4_Y__)		// Make sure this is only included once
#define __SAVE_4X4_Y__

// Module name: save_4x4_Y.asm
//
// Save Intra_4x4 decoded Y picture data to frame buffer
// Note: Each 4x4 block is stored in 1 GRF register in the order of block raster scan order,
// i.e. 0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15

save_4x4_Y:

    mov (1) MSGSRC.2:ud		0x000F000F:ud {NoDDClr}		// Block width and height (16x16)
    mov (2) MSGSRC.0:ud		I_ORIX<2;2,1>:w {NoDDChk}	// X, Y offset
#ifdef DEV_ILK
    add (1)		MSGDSC	MSGDSC MSG_LEN(8)+DWBWMSGDSC_WC-DWBRMSGDSC_RC-0x00200000:ud  // Set message descriptor
#else
    add (1)		MSGDSC	MSGDSC MSG_LEN(8)+DWBWMSGDSC_WC-DWBRMSGDSC_RC-0x00020000:ud  // Set message descriptor
#endif	// DEV_ILK

    $for(0; <8; 2) {
	mov (16)	MSGPAYLOAD(%1)<1>		DEC_Y(%1)<16;4,1>
	mov (16)	MSGPAYLOAD(%1,16)<1>	DEC_Y(%1,4)<16;4,1>
	mov (16)	MSGPAYLOAD(%1+1)<1>		DEC_Y(%1,8)<16;4,1>
	mov (16)	MSGPAYLOAD(%1+1,16)<1>	DEC_Y(%1,12)<16;4,1>
    }

//  Update message descriptor based on previous read setup
//
    send (8)	REG_WRITE_COMMIT_Y<1>:ud	MSGHDR	MSGSRC<8;8,1>:ud	DAPWRITE	MSGDSC

    RETURN
// End of save_4x4_Y

#endif	// !defined(__SAVE_4X4_Y__)
