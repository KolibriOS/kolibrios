/*
 * Load all reference U/V samples from neighboring macroblocks
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
#if !defined(__LOAD_INTRA_REF_UV__)		// Make sure this is only included once
#define __LOAD_INTRA_REF_UV__

// Module name: load_Intra_Ref_UV.asm
//
// Load all reference U/V samples from neighboring macroblocks
//
// Note: Since loading of U/V data always follows writing of Y, the message descriptor is manipulated
// to avoid recalculating due to frame/field variztions.

// First load top 20x1 row U/V reference samples
// 4 from macroblock D (actually use 2), 16 from macroblock B
//
    shr	(1)	I_ORIY<1>:w		I_ORIY<0;1,0>:w	1:w		// Adjust I_ORIY for NV12 format
    add	(2)	MSGSRC.0<1>:d	I_ORIX<2;2,1>:w	TOP_REF_OFFSET<2;2,1>:b	{NoDDClr}	// Reference samples positioned at (-4, -1)
    mov (1)	MSGSRC.2:ud		0x00000013:ud {NoDDChk}			// Block width and height (20x1)

//  Update message descriptor based on previous Y block write
//
#ifdef DEV_ILK
    add (1)	MSGDSC	MSGDSC	RESP_LEN(1)+DWBRMSGDSC_RC+DESTUV-DWBWMSGDSC_WC-0x10000000-DESTY:ud  // Set message descriptor
#else
    add (1)	MSGDSC	MSGDSC	RESP_LEN(1)+DWBRMSGDSC_RC+DESTUV-DWBWMSGDSC_WC-0x00800000-DESTY:ud  // Set message descriptor
#endif	// DEV_ILK

    send (8)	INTRA_REF_TOP_D(0)	MSGHDR	MSGSRC<8;8,1>:ud	DAPREAD	MSGDSC

// Then load left 4x8 reference samples (actually use 1x8 column)
//
    add	(1)	MSGSRC.1<1>:d	MSGSRC.1<0;1,0>:d	1:w {NoDDClr}	// Reference samples positioned next row
    mov (1)	MSGSRC.2:ud		0x00070003:ud {NoDDChk}			// Block width and height (4x8)
    send (8)	INTRA_REF_LEFT_D(0)	MSGHDRUV	MSGSRC<8;8,1>:ud	DAPREAD	MSGDSC

// End of load_Intra_Ref_UV
#endif	// !defined(__LOAD_INTRA_REF_UV__)
