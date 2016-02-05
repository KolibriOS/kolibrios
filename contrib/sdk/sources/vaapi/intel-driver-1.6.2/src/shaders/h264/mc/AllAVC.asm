/*
 * All HWMC kernels 
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

// Kernel name: AllAVC.asm
//
// All HWMC kernels merged into this file
//
//  $Revision: 2 $
//  $Date: 9/10/06 2:02a $
//

// Note: To enable SW scoreboard for ILK AVC kernels, simply toggle the HW_SCOREBOARD 
//		 and SW_SCOREBOARD definition as described below.
//
// ----------------------------------------------------
//  Main: ALLINTRA
// ----------------------------------------------------

#define	COMBINED_KERNEL
#define	ENABLE_ILDB

//	WA for *Stim tool issue, should be removed later

#ifdef DEV_ILK
#define INSTFACTOR	2	// 128-bit count as 2 instructions
#else
#define INSTFACTOR	1	// 128-bit is 1 instruction
#endif	// DEV_ILK

#ifdef DEV_CTG
  #define SW_SCOREBOARD		// SW Scoreboard should be enabled for CTG and earlier
  #undef HW_SCOREBOARD		// HW Scoreboard should be disabled for CTG and earlier
#else
  #define HW_SCOREBOARD		// HW Scoreboard should be enabled for ILK and beyond
  #undef SW_SCOREBOARD		// SW Scoreboard should be disabled for ILK and beyond
#endif	// DEV_CTG
#ifdef BOOTSTRAP
#  ifdef ENABLE_ILDB
#    define ALL_SPAWNED_UV_ILDB_FRAME_IP	0
#    define SLEEP_ENTRY_UV_ILDB_FRAME_IP	0
#    define POST_SLEEP_UV_ILDB_FRAME_IP	        0
#    define ALL_SPAWNED_Y_ILDB_FRAME_IP	        0
#    define SLEEP_ENTRY_Y_ILDB_FRAME_IP	        0
#    define POST_SLEEP_Y_ILDB_FRAME_IP	        0
#  endif
#elif defined(DEV_ILK)
# include "export.inc.gen5"
#elif defined(DEV_CTG)
# include "export.inc"
#endif
#if defined(_EXPORT)
	#include "AllAVC_Export.inc"
#elif defined(_BUILD)
	#include "AllAVC.ich"			// ISAasm dumped .exports
	#include "AllAVC_Export.inc"	// Keep jumping targets aligned, only for CTG and beyond
	#include "AllAVC_Build.inc"
#else
#endif

.kernel AllAVC

// Build all intra prediction kernels
//
#ifdef INTRA_16x16_PAD_NENOP
    $for(0; <INTRA_16x16_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef INTRA_16x16_PAD_NOP
    $for(0; <INTRA_16x16_PAD_NOP; 1) {
	nop
	}
#endif
    #include "Intra_16x16.asm"

#ifdef INTRA_8x8_PAD_NENOP
    $for(0; <INTRA_8x8_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef INTRA_8x8_PAD_NOP
    $for(0; <INTRA_8x8_PAD_NOP; 1) {
	nop
	}
#endif
    #include "Intra_8x8.asm"

#ifdef INTRA_4x4_PAD_NENOP
    $for(0; <INTRA_4x4_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef INTRA_4x4_PAD_NOP
    $for(0; <INTRA_4x4_PAD_NOP; 1) {
	nop
	}
#endif
    #include "Intra_4x4.asm"

#ifdef INTRA_PCM_PAD_NENOP
    $for(0; <INTRA_PCM_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef INTRA_PCM_PAD_NOP
    $for(0; <INTRA_PCM_PAD_NOP; 1) {
	nop
	}
#endif
    #include "Intra_PCM.asm"

// Build FrameMB_Motion kernel
//
#define FRAME

  #ifdef FRAME_MB_PAD_NENOP
    $for(0; <FRAME_MB_PAD_NENOP; 1) {
	nenop
	}
  #endif
  #ifdef FRAME_MB_PAD_NOP
    $for(0; <FRAME_MB_PAD_NOP; 1) {
	nop
	}
  #endif
    #include "AVCMCInter.asm"
#undef  FRAME

// Build FieldMB_Motion kernel
//
#define FIELD

  #ifdef FIELD_MB_PAD_NENOP
    $for(0; <FIELD_MB_PAD_NENOP; 1) {
	nenop
	}
  #endif
  #ifdef FIELD_MB_PAD_NOP
    $for(0; <FIELD_MB_PAD_NOP; 1) {
	nop
	}
  #endif
    #include "AVCMCInter.asm"
#undef  FIELD

// Build MBAff_Motion kernel
//
#define MBAFF

  #ifdef MBAFF_MB_PAD_NENOP
    $for(0; <MBAFF_MB_PAD_NENOP; 1) {
	nenop
	}
  #endif
  #ifdef MBAFF_MB_PAD_NOP
    $for(0; <MBAFF_MB_PAD_NOP; 1) {
	nop
	}
  #endif
    #include "AVCMCInter.asm"
#undef  MBAFF

#ifdef SW_SCOREBOARD    

// SW scoreboard kernel for non-MBAFF
//
#ifdef SCOREBOARD_PAD_NENOP
    $for(0; <SCOREBOARD_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef SCOREBOARD_PAD_NOP
    $for(0; <SCOREBOARD_PAD_NOP; 1) {
	nop
	}
#endif
    #include "scoreboard.asm"

//	SW scoreboard kernel for MBAFF

#ifdef SCOREBOARD_MBAFF_PAD_NENOP
    $for(0; <SCOREBOARD_MBAFF_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef SCOREBOARD_MBAFF_PAD_NOP
    $for(0; <SCOREBOARD_MBAFF_PAD_NOP; 1) {
	nop
	}
#endif
    #include "scoreboard_MBAFF.asm"

#elif defined(HW_SCOREBOARD)
 
// SetHWscoreboard kernel for non-MBAFF
//
#ifdef SETHWSCOREBOARD_PAD_NENOP
    $for(0; <SETHWSCOREBOARD_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef SETHWSCOREBOARD_PAD_NOP
    $for(0; <SETHWSCOREBOARD_PAD_NOP; 1) {
	nop
	}
#endif
    #include "SetHWScoreboard.asm"

//	SetHWscoreboard kernel for MBAFF

#ifdef SETHWSCOREBOARD_MBAFF_PAD_NENOP
    $for(0; <SETHWSCOREBOARD_MBAFF_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef SETHWSCOREBOARD_MBAFF_PAD_NOP
    $for(0; <SETHWSCOREBOARD_MBAFF_PAD_NOP; 1) {
	nop
	}
#endif
    #include "SetHWScoreboard_MBAFF.asm"

#endif	// SW_SCOREBOARD

#ifdef BSDRESET_PAD_NENOP
    $for(0; <BSDRESET_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef BSDRESET_PAD_NOP
    $for(0; <BSDRESET_PAD_NOP; 1) {
	nop
	}
#endif
    #include "BSDReset.asm"

#ifdef DCRESETDUMMY_PAD_NENOP
    $for(0; <DCRESETDUMMY_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef DCRESETDUMMY_PAD_NOP
    $for(0; <DCRESETDUMMY_PAD_NOP; 1) {
	nop
	}
#endif
    #include "DCResetDummy.asm"

#ifdef ENABLE_ILDB

// Build all ILDB kernels
//
//	Undefine some previous defined symbols since they will be re-defined/re-declared in ILDB kernels
#undef	A
#undef	B
#undef	p0
#undef	p1

#define MSGPAYLOADB MSGPAYLOADB_ILDB
#define MSGPAYLOADW MSGPAYLOADW_ILDB
#define MSGPAYLOADD MSGPAYLOADD_ILDB
#define MSGPAYLOADF MSGPAYLOADF_ILDB

//				< Frame ILDB >
#define _PROGRESSIVE
#define ILDB_LABEL(x)	x##_ILDB_FRAME
#ifdef AVC_ILDB_ROOT_Y_ILDB_FRAME_PAD_NENOP
    $for(0; <AVC_ILDB_ROOT_Y_ILDB_FRAME_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_ROOT_Y_ILDB_FRAME_PAD_NOP
    $for(0; <AVC_ILDB_ROOT_Y_ILDB_FRAME_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Root_Y.asm"

#ifdef AVC_ILDB_CHILD_Y_ILDB_FRAME_PAD_NENOP
    $for(0; <AVC_ILDB_CHILD_Y_ILDB_FRAME_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_CHILD_Y_ILDB_FRAME_PAD_NOP
    $for(0; <AVC_ILDB_CHILD_Y_ILDB_FRAME_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Child_Y.asm"

#ifdef AVC_ILDB_ROOT_UV_ILDB_FRAME_PAD_NENOP
    $for(0; <AVC_ILDB_ROOT_UV_ILDB_FRAME_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_ROOT_UV_ILDB_FRAME_PAD_NOP
    $for(0; <AVC_ILDB_ROOT_UV_ILDB_FRAME_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Root_UV.asm"

#ifdef AVC_ILDB_CHILD_UV_ILDB_FRAME_PAD_NENOP
    $for(0; <AVC_ILDB_CHILD_UV_ILDB_FRAME_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_CHILD_UV_ILDB_FRAME_PAD_NOP
    $for(0; <AVC_ILDB_CHILD_UV_ILDB_FRAME_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Child_UV.asm"
#undef ILDB_LABEL
#undef _PROGRESSIVE

//				< Field ILDB >
#define _FIELD
#define ILDB_LABEL(x)	x##_ILDB_FIELD
#ifdef AVC_ILDB_ROOT_Y_ILDB_FIELD_PAD_NENOP
    $for(0; <AVC_ILDB_ROOT_Y_ILDB_FIELD_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_ROOT_Y_ILDB_FIELD_PAD_NOP
    $for(0; <AVC_ILDB_ROOT_Y_ILDB_FIELD_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Root_Field_Y.asm"

#ifdef AVC_ILDB_CHILD_Y_ILDB_FIELD_PAD_NENOP
    $for(0; <AVC_ILDB_CHILD_Y_ILDB_FIELD_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_CHILD_Y_ILDB_FIELD_PAD_NOP
    $for(0; <AVC_ILDB_CHILD_Y_ILDB_FIELD_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Child_Field_Y.asm"

#ifdef AVC_ILDB_ROOT_UV_ILDB_FIELD_PAD_NENOP
    $for(0; <AVC_ILDB_ROOT_UV_ILDB_FIELD_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_ROOT_UV_ILDB_FIELD_PAD_NOP
    $for(0; <AVC_ILDB_ROOT_UV_ILDB_FIELD_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Root_Field_UV.asm"

#ifdef AVC_ILDB_CHILD_UV_ILDB_FIELD_PAD_NENOP
    $for(0; <AVC_ILDB_CHILD_UV_ILDB_FIELD_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_CHILD_UV_ILDB_FIELD_PAD_NOP
    $for(0; <AVC_ILDB_CHILD_UV_ILDB_FIELD_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Child_Field_UV.asm"
#undef ILDB_LABEL
#undef _FIELD

//				< MBAFF Frame ILDB >
#define _MBAFF
#define ILDB_LABEL(x)	x##_ILDB_MBAFF
#ifdef AVC_ILDB_ROOT_Y_ILDB_MBAFF_PAD_NENOP
    $for(0; <AVC_ILDB_ROOT_Y_ILDB_MBAFF_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_ROOT_Y_ILDB_MBAFF_PAD_NOP
    $for(0; <AVC_ILDB_ROOT_Y_ILDB_MBAFF_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Root_Mbaff_Y.asm"

#ifdef AVC_ILDB_CHILD_Y_ILDB_MBAFF_PAD_NENOP
    $for(0; <AVC_ILDB_CHILD_Y_ILDB_MBAFF_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_CHILD_Y_ILDB_MBAFF_PAD_NOP
    $for(0; <AVC_ILDB_CHILD_Y_ILDB_MBAFF_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Child_Mbaff_Y.asm"

#ifdef AVC_ILDB_ROOT_UV_ILDB_MBAFF_PAD_NENOP
    $for(0; <AVC_ILDB_ROOT_UV_ILDB_MBAFF_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_ROOT_UV_ILDB_MBAFF_PAD_NOP
    $for(0; <AVC_ILDB_ROOT_UV_ILDB_MBAFF_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Root_Mbaff_UV.asm"

#ifdef AVC_ILDB_CHILD_UV_ILDB_MBAFF_PAD_NENOP
    $for(0; <AVC_ILDB_CHILD_UV_ILDB_MBAFF_PAD_NENOP; 1) {
	nenop
	}
#endif
#ifdef AVC_ILDB_CHILD_UV_ILDB_MBAFF_PAD_NOP
    $for(0; <AVC_ILDB_CHILD_UV_ILDB_MBAFF_PAD_NOP; 1) {
	nop
	}
#endif
    #include "AVC_ILDB_Child_Mbaff_UV.asm"
#undef ILDB_LABEL
#undef _MBAFF

#endif		// ENABLE_ILDB

AllAVC_END:
nop
// End of AllAVC

.end_code

.end_kernel

