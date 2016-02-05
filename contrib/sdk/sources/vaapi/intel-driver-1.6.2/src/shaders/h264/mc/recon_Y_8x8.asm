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
// Kernel name: Recon_Y_8x8.asm
//
//  $Revision: 10 $
//  $Date: 9/22/06 2:50p $
//


//#if !defined(__RECON_Y_8x8__)		// Make sure this is only included once
//#define __RECON_Y_8x8__


	add.sat (16)		r[pERRORY,0]<2>:ub			r[pERRORY,0]<16;16,1>:w				gubYPRED(0)
	add.sat (16)		r[pERRORY,nGRFWIB]<2>:ub	r[pERRORY,nGRFWIB]<16;16,1>:w		gubYPRED(1)
	add.sat (16)		r[pERRORY,nGRFWIB*2]<2>:ub	r[pERRORY,nGRFWIB*2]<16;16,1>:w		gubYPRED(2)
	add.sat (16)		r[pERRORY,nGRFWIB*3]<2>:ub	r[pERRORY,nGRFWIB*3]<16;16,1>:w		gubYPRED(3)
	
	add (1)				pERRORY:w					pERRORY:w							128:w

//#endif	// !defined(__RECON_Y_8x8__)
