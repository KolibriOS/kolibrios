/*
 * Interpolation kernel for chrominance 4x4 motion compensation
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
//	Kernel name: Interpolate_C_4x4_Func.asm
//
//	Interpolation kernel for chrominance 4x4 motion compensation
//
//  $Revision: 8 $
//  $Date: 10/09/06 4:00p $
//


//#if !defined(__Interpolate_C_4x4_Func__)		// Make sure this is only included once
//#define __Interpolate_C_4x4_Func__


INTERLABEL(Interpolate_C_4x4_Func):


	// (8-xFrac) and (8-yFrac)
    add (2)		gW0<1>:w			gMVX_FRACC<2;2,1>:w				-0x08:w

	// Compute the GRF address of the starting position of the reference area
    mov (1)		pREF0:w				nOFFSET_REFC:w		{NoDDClr}
	mov (1)		pREF1:uw			nOFFSET_REFC+16:w	{NoDDChk,NoDDClr}
	mov (1)		pRESULT:uw			gpINTPC:uw			{NoDDChk}

	// gCOEFA = (8-xFrac)*(8-yFrac)
    // gCOEFB = xFrac*(8-yFrac)  
    // gCOEFC = (8-xFrac)*yFrac
    // gCOEFD = xFrac*yFrac 
    mul (1)		gCOEFD:w	        gMVX_FRACC:w					gMVY_FRACC:w	{NoDDClr}
    mul (1)		gCOEFA:w			-gW0:w							-gW1:uw		{NoDDClr,NoDDChk}
    mul (1)		gCOEFB:w			gMVX_FRACC:w					-gW1:uw		{NoDDClr,NoDDChk}
    mul (1)		gCOEFC:w		    -gW0:w							gMVY_FRACC:w {NoDDChk}

	add (2)		gW0<1>:uw			pREF0<2;2,1>:uw					16:w

    // (8-xFrac)*(8-yFrac)*A
    // ---------------------
    mul (16)	acc0<1>:uw			r[pREF0,0]<16;8,1>:ub			gCOEFA:uw
    mul (16)	acc1<1>:uw			r[pREF0,nGRFWIB]<16;8,1>:ub		gCOEFA:uw
        
    // xFrac*(8-yFrac)*B
    // -------------------
    mac (16)	acc0<1>:uw          r[pREF0,2]<16;8,1>:ub			gCOEFB:uw
    mac (16)	acc1<1>:uw          r[pREF0,nGRFWIB+2]<16;8,1>:ub	gCOEFB:uw

    // (8-xFrac)*yFrac*C
    // -------------------
    mov (2)		pREF0<1>:uw			gW0<2;2,1>:uw
    mac (16)	acc0<1>:uw          r[pREF0,0]<8,1>:ub				gCOEFC:uw
    mac (16)	acc1<1>:uw          r[pREF0,nGRFWIB]<8,1>:ub		gCOEFC:uw
            
    // xFrac*yFrac*D
    // -----------------
    mac (16)	r[pRESULT]<1>:uw	r[pREF0,2]<8,1>:ub				gCOEFD:uw
    mac (16)	r[pRESULT,GRFWIB]<1>:uw r[pREF0,nGRFWIB+2]<8,1>:ub gCOEFD:uw {SecHalf}

   
//#endif	// !defined(__Interpolate_C_4x4_Func__)
