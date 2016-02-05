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
/////////////////////////////////////////////////////////////////////////////////////
// Kernel name: AVC_ILDB_Root_Mbaff.asm
//
//  Root kernel serves as a scheduler for child threads.
//
//
//	***** Note *****
//	Initial design bundle MB pair for each thread, and share AVC_ILDB_MB_Dep_Check.asm
//	with non mbaff kernels.
//
//	Optimization will be done later, putting top and bottom MBs on separate threads.
//
//
/////////////////////////////////////////////////////////////////////////////////////
//
//  $Revision: 1 $
//  $Date: 10/19/06 5:06p $
//

// ----------------------------------------------------
//  AVC_ILDB_ROOT_MBAFF_Y
// ----------------------------------------------------
#define AVC_ILDB

.kernel AVC_ILDB_ROOT_MBAFF_Y
#if defined(COMBINED_KERNEL)
ILDB_LABEL(AVC_ILDB_ROOT_Y):
#endif

#include "SetupVPKernel.asm"
#include "AVC_ILDB.inc"


#if defined(_DEBUG) 

/////////////////////////////////////////////////////////////////////////////////////
// Init URB space for running on RTL.  It satisfies reading an unwritten URB entries.  
// Will remove it for production release.


//mov (8) m1:ud 		0x11111111:ud
//mov (8) m2:ud 		0x22222222:ud 
//mov (8) m3:ud 		0x33333333:ud
//mov (8) m4:ud 		0x44444444:ud 

//mov (1)	Temp1_W:w	0:w

//ILDB_INIT_URB:
//mul (1)	URBOffset:w				Temp1_W:w		4:w
//shl (1) URBWriteMsgDescLow:uw 	URBOffset:w		4:w		// Msg descriptor: URB write dest offset (9:4)
//mov (1) URBWriteMsgDescHigh:uw 	0x0650:uw				// Msg descriptor: URB write 5 MRFs (m0 - m4)
//#include "writeURB.asm"

//add		(1)		Temp1_W:w	Temp1_W:w	1:w				// Increase block count
//cmp.l.f0.0 (1) 	null		Temp1_W:w	MBsCntY:w		// Check the block count limit
//(f0.0) jmpi		ILDB_INIT_URB							// Loop back

/////////////////////////////////////////////////////////////////////////////////////


mov		(1)		EntrySignature:w			0xEFF0:w

#endif
//----------------------------------------------------------------------------------------------------------------

// Set global variable
mov (32) 	ChildParam:uw			0:uw								// Reset local variables
//mul	(1)	 	TotalBlocks:w			MBsCntX:w		MBsCntY:w			// Total # of MB pairs
//add	(1)	 	GatewayApertureE:w		MBsCntY:w 		GatewayApertureB:w	// Aperture End = aperture Head + BlockCntY


// 2 URB entries for Y:
// Entry 0 - Child thread R0Hdr
// Entry 1 - input parameter to child kernel (child r1)

#undef 		URB_ENTRIES_PER_MB
#define 	URB_ENTRIES_PER_MB	 	2

// URB_ENTRIES_PER_MB in differnt form, the final desired format is (URB_ENTRIES_PER_MB-1) << 10
mov (1) 	URB_EntriesPerMB_2:w		URB_ENTRIES_PER_MB-1:w
shl (1) 	URB_EntriesPerMB_2:w		URB_EntriesPerMB_2:w	10:w

mov	(1)		ChildThreadsID:uw		1:uw					// ChildThreadsID for chroma root

shr (1)		ThreadLimit:w		MaxThreads:w		1:w		// Initial luma thread limit to 50%
mul	(1)	 	TotalBlocks:w		MBsCntX:w		MBsCntY:w	// MBs to be processed count down from TotalBlocks

//***** Init CT_R0Hdr fields that are common to all threads *************************
mov (8)		CT_R0Hdr.0:ud			r0.0<8;8,1>:ud				// Init to root R0 header
mov (1)		CT_R0Hdr.7:ud			r0.6:ud						// Copy Parent Thread Cnt; JJ did the change on 06/20/2006
mov (1) 	CT_R0Hdr.31:ub			0:w							// Reset the highest byte
mov (1) 	CT_R0Hdr.3:ud 			0x00000000	 
mov (1) 	CT_R0Hdr.6:uw 			sr0.0:uw					// sr0.0: state reg contains general thread states, e.g. EUID/TID.

//***** Init ChildParam fields that are common to all threads ***********************
mov (8) 	ChildParam<1>:ud	RootParam<8;8,1>:ud		// Copy all root parameters
mov (4)		CurCol<1>:w			0:w						// Reset CurCol, CurRow
add	(2)		LastCol<1>:w		MBsCntX<2;2,1>:w		-1:w	// Get LastCol and LastRow

mov (1) 	URBWriteMsgDesc:ud		MSG_LEN(2)+URBWMSGDSC:ud

//===================================================================================

#include "AVC_ILDB_OpenGateway.asm"		// Open root thread gateway for receiving notification 

#if defined(DEV_CL)	
	mov	(1)		URBOffset:uw		240:uw	// Use chroma URB offset to spawn chroma root
#else
	mov	(1)		URBOffset:uw		320:uw	// Use chroma URB offset to spawn chroma root
#endif

#include "AVC_ILDB_SpawnChromaRoot.asm"	// Spawn chroma root

mov	(1)		URBOffset:uw		0:uw	// Use luma URB offset to spawn luma child 
mov	(1)		ChildThreadsID:uw	2:uw	// Starting ChildThreadsID for luma child threads

#include "AVC_ILDB_Dep_Check.asm"  	// Check dependency and spawn all MBs

// Wait for UV root thread to finish
ILDB_LABEL(WAIT_FOR_UV):
cmp.l.f0.0 (1) null:w	ThreadLimit:w		MaxThreads:w
(f0.0) 	jmpi 	ILDB_LABEL(WAIT_FOR_UV)

#include "AVC_ILDB_CloseGateway.asm"	// Close root thread gateway 

END_THREAD								// End of root thread

#if !defined(COMBINED_KERNEL)		// For standalone kernel only
.end_code

.end_kernel
#endif
