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

//////////////////////////////////////////////////////////////////////////////////
// Multiple_Loop_Head.asm
// This code sets up the loop control for multiple blocks per thread

	mul (1)	wFRAME_ENDX:w	ubBLK_CNT_X:ub	16:uw	{ NoDDClr }				// Build multi-block loop counters
	mov (1) wNUM_BLKS:w		ubNUM_BLKS:ub			{ NoDDClr, NoDDChk }	// Copy num blocks to word variable
	mov (1) wCOPY_ORIX:w	wORIX:w					{ NoDDChk }				// Copy multi-block origin in pixel 
	mov (2) fFRAME_VID_ORIX<1>:f			fSRC_VID_H_ORI<4;2,2>:f			// Copy src video origin for scaling, and alpha origin for blending
	add (1)	wFRAME_ENDX:w	wFRAME_ENDX:w	wORIX:w							// Continue building multi-block loop counters

VIDEO_PROCESSING_LOOP:		// Loop back entry point as the biginning of the loop for multiple blocks
	
// Beginning of the loop
