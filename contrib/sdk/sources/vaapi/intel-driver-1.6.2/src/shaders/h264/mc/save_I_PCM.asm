/*
 * Save I_PCM Y samples to Y picture buffer
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
// Module name: save_I_PCM.asm
//
// First save I_PCM Y samples to Y picture buffer
//
    mov (1) MSGSRC.2:ud		0x000F000F:ud {NoDDClr}			// Block width and height (16x16)
	shl (2) MSGSRC.0:ud		ORIX<2;2,1>:ub	4:w	{NoDDChk}	// Convert MB origin in pixel unit

    add (1)	MSGDSC	REG_MBAFF_FIELD<0;1,0>:uw	MSG_LEN(8)+DWBWMSGDSC_WC+DESTY:ud  // Set message descriptor

    $for(0; <8; 2) {
	mov (32)	MSGPAYLOAD(%1)<1>		I_PCM_Y(%1)REGION(16,1) {Compr,NoDDClr}
	mov (32)	MSGPAYLOAD(%1,16)<1>	I_PCM_Y(%1,16)REGION(16,1) {Compr,NoDDChk}
    }

    send (8)	REG_WRITE_COMMIT_Y<1>:ud	MSGHDR	MSGSRC<8;8,1>:ud	DAPWRITE	MSGDSC

// Then save I_PCM U/V samples to U/V picture buffer
//
    mov (1) MSGHDR.2:ud		0x0007000F:ud			{NoDDClr}	// Block width and height (16x8)
    asr (1) MSGHDR.1:ud		MSGSRC.1<0;1,0>:ud	1:w {NoDDChk}	// Y offset should be halved
    add (1)	MSGDSC			MSGDSC			0x0-MSG_LEN(4)+0x1:d	// Set message descriptor for U/V

#if 0
    and.z.f0.0 (1)  NULLREG REG_CHROMA_FORMAT_IDC  CHROMA_FORMAT_IDC:ud
	(f0.0) jmpi (1) MONOCHROME_I_PCM
#endif

#ifndef MONO
// Non-monochrome picture
//
    $for(0,0; <4; 2,1) {
	mov (16)	MSGPAYLOAD(%1)<2>		I_PCM_UV(%2)REGION(16,1)		// U data
	mov (16)	MSGPAYLOAD(%1,1)<2>		I_PCM_UV(%2+2)REGION(16,1)		// V data
	mov (16)	MSGPAYLOAD(%1+1)<2>		I_PCM_UV(%2,16)REGION(16,1)		// U data
	mov (16)	MSGPAYLOAD(%1+1,1)<2>	I_PCM_UV(%2+2,16)REGION(16,1)	// V data
	}
#else	// defined(MONO)
MONOCHROME_I_PCM:
    $for(0; <4; 2) {
	mov (16)	MSGPAYLOADD(%1)<1>		0x80808080:ud {Compr}
	}

#endif	// !defined(MONO)

    send (8)	REG_WRITE_COMMIT_UV<1>:ud	MSGHDR	null:ud	DAPWRITE	MSGDSC

// End of save_I_PCM
