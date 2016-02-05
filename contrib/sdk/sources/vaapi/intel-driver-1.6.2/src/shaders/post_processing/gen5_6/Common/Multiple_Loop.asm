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

/////////////////////////////////////////////////////////////////////////////////
// Multiple_Loop.asm


// This lable is for satisfying component kernel build.
// DL will remove this label and reference the real one in Multiple_Loop_Head.asm.
#if defined(COMPONENT)
VIDEO_PROCESSING_LOOP:
#endif


//===== Possible build flags for component kernels
// 1) INC_SCALING
// 2) INC_BLENDING
// 3) INC_BLENDING and INC_SCALING
// 4) (no flags)


#define MxN_MULTIPLE_BLOCKS

//------------------------------------------------------------------------------
#if defined(MxN_MULTIPLE_BLOCKS)
// Do Multiple Block Processing ------------------------------------------------

	// The 1st block has been processed before entering the loop

	// Processed all blocks?
	add.z.f0.0	(1)	wNUM_BLKS:w	wNUM_BLKS:w	-1:w

	// Reached multi-block width?
	add			(1)	wORIX:w		wORIX:w		16:w
	cmp.l.f0.1	(1)	null:w		wORIX:w	wFRAME_ENDX:w	// acc0.0 has wORIX

	#if defined(INC_SCALING)
	// Update SRC_VID_H_ORI for scaling
		mul	(1)	REG(r,nTEMP0):f		fVIDEO_STEP_X:f		16.0:f
		add	(1)	fSRC_VID_H_ORI:f	REG(r,nTEMP0):f		fSRC_VID_H_ORI:f
	#endif

	#if defined(INC_BLENDING)
	// Update SRC_ALPHA_H_ORI for blending
		mul	(1)	REG(r,nTEMP0):f		fALPHA_STEP_X:f		16.0:f
		add	(1)	fSRC_ALPHA_H_ORI:f	REG(r,nTEMP0):f		fSRC_ALPHA_H_ORI:f
	#endif

	(f0.0)jmpi	(1)	END_VIDEO_PROCESSING	// All blocks are done - Exit loop

    // blocks in the middle of the loop (neither the first nor the last one)?
    // it may be on the left edge (Mx1) or not (1xN)
    mov (1) uwBLOCK_MASK_H<1>:uw            uwBLOCK_MASK_H_MIDDLE:uw

    // the last block?
    cmp.e.f0.0  (1) null:w      wNUM_BLKS:w     1:w
    (f0.0)  mov (1) uwBLOCK_MASK_H<1>:uw  uwBLOCK_MASK_H_RIGHT:uw
    (f0.0)  mov (1) ubBLOCK_MASK_V<1>:ub  ubBLOCK_MASK_V_BOTTOM:ub
    
	(f0.1)jmpi	(1)	VIDEO_PROCESSING_LOOP	// If not the end of row, goto the beginning of the loop

	//If end of row, restart Horizontal offset and calculate Vertical offsets next row.
	mov	(1)		wORIX:w		wCOPY_ORIX:w
	add	(1)		wORIY:w		wORIY:w			8:w

	#if defined(INC_SCALING)
	// Update SRC_VID_H_ORI and SRC_VID_V_ORI for scaling
		mov	(1)		fSRC_VID_H_ORI:f	fFRAME_VID_ORIX:f	// Reset normalised X origin to 0 for video and alpha
		mul	(1)		REG(r,nTEMP0):f		fVIDEO_STEP_Y:f		8.0:f
		add	(1)		fSRC_VID_V_ORI:f	REG(r,nTEMP0):f		fSRC_VID_V_ORI:f
	#endif

	#if defined(INC_BLENDING)
	// Update SRC_ALPHA_H_ORI and SRC_ALPHA_V_ORI for blending
		mov	(1)		fSRC_ALPHA_H_ORI:f	fFRAME_ALPHA_ORIX:f	// Reset normalised X origin to 0 for video and alpha
		mul	(1)		REG(r,nTEMP0):f		fALPHA_STEP_Y:f		8.0:f
		add	(1)		fSRC_ALPHA_V_ORI:f	REG(r,nTEMP0):f		fSRC_ALPHA_V_ORI:f
	#endif

	jmpi (1)	VIDEO_PROCESSING_LOOP	// Continue Loop

END_VIDEO_PROCESSING:
	nop

#endif
END_THREAD	// End of Thread
