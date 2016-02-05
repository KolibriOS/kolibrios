/*
 * Decode Intra_PCM macroblock
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
// Kernel name: Intra_PCM.asm
//
// Decoding of I_PCM macroblock
//
//  $Revision: 8 $
//  $Date: 10/18/06 4:10p $
//

// ----------------------------------------------------
//  Main: Intra_PCM
// ----------------------------------------------------

.kernel Intra_PCM
INTRA_PCM:

#ifdef _DEBUG
// WA for FULSIM so we'll know which kernel is being debugged
mov (1) acc0:ud 0x03aa55a5:ud
#endif

#include "SetupForHWMC.asm"

// Not actually needed here but just want to slow down the Intra-PCM to avoid race condition
//
#ifdef SW_SCOREBOARD
	and (1)	  REG_INTRA_PRED_AVAIL_FLAG_WORD<1>:w	REG_INTRA_PRED_AVAIL_FLAG_WORD<0;1,0>:w	0xffe0:w	// Ensure all neighbor avail flags are "0"
    CALL(scoreboard_start_intra,1)
	wait	n0:ud		//	Now wait for scoreboard to response
#endif

//
//  Decoding Y blocks
//
//	In I_PCM mode, the samples are already arranged in raster scan order within the macroblock.
//	We just need to save them to picture buffers
//
    #include "save_I_PCM.asm"		    // Save to destination picture buffers

#ifdef SW_SCOREBOARD    
    #include "scoreboard_update.asm"
#endif

// Terminate the thread
//
    #include "EndIntraThread.asm"

// End of Intra_PCM
