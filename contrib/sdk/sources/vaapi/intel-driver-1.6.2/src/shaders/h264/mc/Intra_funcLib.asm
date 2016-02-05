/*
 * Library of common modules shared among different intra prediction kernels
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
// Module name: Intra_funcLib.asm
//
// Library of common modules shared among different intra prediction kernels
//
//  Note: Any sub-modules, if they are #included in more than one kernel,
//	  should be moved to this module.
//
#if defined(INTRA_16X16)
#undef INTRA_16X16
    #include "load_Intra_Ref_Y.asm"		// Load intra Y reference data
    #include "Decode_Chroma_Intra.asm"	// Decode chroma blocks
    #include "save_16x16_Y.asm"			// Save to destination Y frame surface
#elif defined(INTRA_8X8)
#undef INTRA_8X8
    #include "load_Intra_Ref_Y.asm"		// Load intra Y reference data
    #include "Decode_Chroma_Intra.asm"	// Decode chroma blocks
    #include "intra_Pred_8x8_Y.asm"		// Intra predict Intra_4x4 blocks
    #include "save_8x8_Y.asm"			// Save to destination Y frame surface
#elif defined(INTRA_4X4)
#undef INTRA_4X4
    #include "load_Intra_Ref_Y.asm"		// Load intra Y reference data
    #include "Decode_Chroma_Intra.asm"	// Decode chroma blocks
    #include "intra_Pred_4x4_Y_4.asm"	// Intra predict Intra_4x4 blocks
    #include "save_4x4_Y.asm"			// Save to destination Y frame surface
#else								// For all merged kernels
#endif

#ifdef SW_SCOREBOARD    
    #include "scoreboard_start_intra.asm"	// scorboard intra start function	
    #include "scoreboard_start_inter.asm"	// scorboard inter start function	
#endif	// SW_SCOREBOARD

// End of Intra_funcLib
