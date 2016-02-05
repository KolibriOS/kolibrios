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
// Kernel name: WriteRecon_C_8x4.asm
//
//  $Revision: 10 $
//  $Date: 10/03/06 5:28p $
//


//#if !defined(__WRITERECON_C_8x4__)		// Make sure this is only included once
//#define __WRITERECON_C_8x4__


	// TODO: Why did I use p0?
#ifndef MONO
    add (1)		p0:w					pERRORC:w				-16:w
	mov (16)	mbMSGPAYLOADC(0,0)<2>	r[p0,0]<32;16,2>:ub				{NoDDClr}
	mov (16)	mbMSGPAYLOADC(0,1)<2>	r[p0,128]<32;16,2>:ub			{NoDDChk}
	mov (16)	mbMSGPAYLOADC(1,0)<2>	r[p0,32]<32;16,2>:ub			{NoDDClr}
	mov (16)	mbMSGPAYLOADC(1,1)<2>	r[p0,128+32]<32;16,2>:ub		{NoDDChk}
#else	// defined(MONO)
	mov (16)	mbMSGPAYLOADC(0)<1>		0x80808080:ud {Compr}
#endif	// !defined(MONO)

 #if defined(MBAFF)
	add (1)		pMSGDSC:ud				gFIELDFLAGS:uw			MSG_LEN(2)+nDWBWMSGDSC+nBDIX_DESTC+ENWRCOM:ud
 #elif defined(FIELD)
	add (1)		pMSGDSC:ud				gFIELDFLAGS:uw			MSG_LEN(2)+nDWBWMSGDSC_TF+nBDIX_DESTC+ENWRCOM:ud
 #endif

    asr (1)		gMSGSRC.1:d				gMSGSRC.1:d					1:w	{NoDDClr}
    mov (1)		gMSGSRC.2:ud			0x0003000f:ud					{NoDDChk} // NV12 (16x4)

#if defined(FRAME)
    send (8)	gREG_WRITE_COMMIT_UV<1>:ud		mMSGHDRCW				gMSGSRC<8;8,1>:ud		DAPWRITE	MSG_LEN(2)+nDWBWMSGDSC+nBDIX_DESTC+ENWRCOM
#else
    send (8)	gREG_WRITE_COMMIT_UV<1>:ud		mMSGHDRCW				gMSGSRC<8;8,1>:ud		DAPWRITE	pMSGDSC:ud
#endif	// defined(FRAME)

//#endif	// !defined(__WRITERECON_C_8x4__)
