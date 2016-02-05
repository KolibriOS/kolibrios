/*
 * Load reference 6x3 area for chroma NV12 4x4 MC
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
// Kernel name: LoadRef_C_6x3.asm
//
// Load reference 6x3 area for chroma NV12 4x4 MC


//#if !defined(__LOADREF_C_6x3__)		// Make sure this is only included once
//#define __LOADREF_C_6x3__


#ifdef DEV_ILK
	add (1)		pMSGDSC:ud			gMSGDSC_R:ud					0x00100010:ud
#else
	add (1)		pMSGDSC:ud			gMSGDSC_R:ud					0x00010010:ud
#endif	// DEV_ILK

	// Compute integer and fractional components of MV
    asr (2)		gMVX_INT<1>:w		r[pMV,0]<2;2,1>:w				0x03:w {NoDDClr}
    and (2)		gMVX_FRACC<1>:w		r[pMV,0]<2;2,1>:w				0x07:w {NoDDChk}
	
	// Compute top-left corner position to be loaded
    mov (2)		gMSGSRC.0<1>:d		gMVX_INT<2;2,1>:w
	shl (1)		gMSGSRC.0:d			gMSGSRC.0:d						1:w

    // Read 8x3 pixels
    mov (1)		gMSGSRC.2:ud		0x00020005:ud
    send (8)	gudREFC(0)<1>	    mMSGHDRC						gMSGSRC<8;8,1>:ud	DAPREAD	pMSGDSC:ud

        
//#endif	// !defined(__LOADREF_C_6x3__)
