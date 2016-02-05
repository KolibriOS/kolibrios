/*
 * All Video Processing kernels 
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


.declare SRC_B		Base=REG(r,10)	ElementSize=2 SrcRegion=REGION(8,1) DstRegion=<1> Type=uw
.declare SRC_G		Base=REG(r,18)	ElementSize=2 SrcRegion=REGION(8,1) DstRegion=<1> Type=uw
.declare SRC_R		Base=REG(r,26)	ElementSize=2 SrcRegion=REGION(8,1) DstRegion=<1> Type=uw
.declare SRC_A		Base=REG(r,34)	ElementSize=2 SrcRegion=REGION(8,1) DstRegion=<1> Type=uw

#define DEST_ARGB		ubBOT_ARGB

#undef 	nSRC_REGION
#define nSRC_REGION		nREGION_2


//Pack directly to mrf as optimization - vK

$for(0, 0; <8; 1, 2) {
//	mov	(16) 	DEST_ARGB(%2,0)<4>		SRC_B(%1) 					{ Compr, NoDDClr }			// 16 B
//	mov	(16) 	DEST_ARGB(%2,1)<4>		SRC_G(%1)					{ Compr, NoDDClr, NoDDChk }	// 16 G
//	mov	(16) 	DEST_ARGB(%2,2)<4>		SRC_R(%1)					{ Compr, NoDDClr, NoDDChk }	// 16 R	//these 2 inst can be merged - vK
//	mov	(16) 	DEST_ARGB(%2,3)<4>		SRC_A(%1)					{ Compr, NoDDChk }			//DEST_RGB_FORMAT<0;1,0>:ub	{ Compr, NoDDChk }			// 16 A

	mov	(8) 	DEST_ARGB(%2,  0)<4>		SRC_B(%1) 					{ NoDDClr }				// 8 B
	mov	(8) 	DEST_ARGB(%2,  1)<4>		SRC_G(%1)					{ NoDDClr, NoDDChk }	// 8 G
	mov	(8) 	DEST_ARGB(%2,  2)<4>		SRC_R(%1)					{ NoDDClr, NoDDChk }	// 8 R
	mov	(8) 	DEST_ARGB(%2,  3)<4>		SRC_A(%1)					{ NoDDChk }				// 8 A

	mov	(8) 	DEST_ARGB(%2+1,0)<4>		SRC_B(%1,8) 				{ NoDDClr }				// 8 B
	mov	(8) 	DEST_ARGB(%2+1,1)<4>		SRC_G(%1,8)					{ NoDDClr, NoDDChk }	// 8 G
	mov	(8) 	DEST_ARGB(%2+1,2)<4>		SRC_R(%1,8)					{ NoDDClr, NoDDChk }	// 8 R
	mov	(8) 	DEST_ARGB(%2+1,3)<4>		SRC_A(%1,8)					{ NoDDChk }				// 8 A
}
