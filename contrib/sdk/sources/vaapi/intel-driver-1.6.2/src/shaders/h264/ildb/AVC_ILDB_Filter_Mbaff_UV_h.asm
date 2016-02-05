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
////////// AVC ILDB filter horizontal Mbaff UV ///////////////////////////////////////////////////////
//
//	This filter code prepares the src data and control data for ILDB filtering on all horizontal edges of UV.
//
//	It sssumes the data for horizontal de-blocking is already transposed.  
//
//		Chroma:
//
//		+-------+-------+		H0 Edge
//		|		|		|
//		|		|		|
//		|		|		|
//		+-------+-------+		H1 Edge
//		|		|		|
//		|		|		|
//		|		|		|
//		+-------+-------+
//
/////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0xBBBC:w
#endif	

//=============== Chroma deblocking ================

//---------- Deblock UV external top edge ----------

	and.z.f0.0  (1) null:w		r[ECM_AddrReg, BitFlags]:ub		FilterTopMbEdgeFlag:w		// Check for FilterTopMbEdgeFlag 

	mov	(1)	f0.1:w		DualFieldMode:w		// Check for dual field mode

	// Get Luma maskA and maskB	
	shr (16)	TempRow0(0)<1>		r[ECM_AddrReg, wEdgeCntlMapA_ExtTopHorz0]<0;1,0>:uw		RRampW(0)
	shr (16)	TempRow1(0)<1>		r[ECM_AddrReg, wEdgeCntlMapB_ExtTopHorz0]<0;1,0>:uw		RRampW(0)

    (f0.0)	jmpi	H0_UV_DONE				// Skip H0 UV edge

	(f0.1) jmpi DUAL_FIELD_UV

	// Non dual field mode	

	// Extract UV MaskA and MaskB from every other bit of Y masks
	and.nz.f0.0 (8) null:w			TempRow0(0)<16;8,2>		1:w
	and.nz.f0.1 (8) null:w			TempRow1(0)<16;8,2>		1:w

	// Ext U
	//	p1 = Prev MB U row 0
	//	p0 = Prev MB U row 1
	// 	q0 = Cur MB U row 0
	//	q1 = Cur MB U row 1
	mov (1)	P_AddrReg:w		PREV_MB_U_BASE:w	{ NoDDClr }
	mov (1)	Q_AddrReg:w		SRC_MB_U_BASE:w		{ NoDDChk }
	
	mov	(8) Mbaff_ALPHA(0,0)<1>		r[ECM_AddrReg, bAlphaTop0_Cb]<0;1,0>:ub
	mov	(8) Mbaff_BETA(0,0)<1>		r[ECM_AddrReg, bBetaTop0_Cb]<0;1,0>:ub
	mov (8) Mbaff_TC0(0,0)<1>		r[ECM_AddrReg, bTc0_h00_0_Cb]<1;2,0>:ub

	// Store UV MaskA and MaskB
	mov (2)		MaskA<1>:uw			f0.0<2;2,1>:uw

	CALL(FILTER_UV_MBAFF, 1)	

	// Ext V
	mov (1)	P_AddrReg:w		PREV_MB_V_BASE:w	{ NoDDClr }
	mov (1)	Q_AddrReg:w		SRC_MB_V_BASE:w		{ NoDDChk }

	mov	(8) Mbaff_ALPHA(0,0)<1>		r[ECM_AddrReg, bAlphaTop0_Cr]<0;1,0>:ub
	mov	(8) Mbaff_BETA(0,0)<1>		r[ECM_AddrReg, bBetaTop0_Cr]<0;1,0>:ub
	mov (8) Mbaff_TC0(0,0)<1>		r[ECM_AddrReg, bTc0_h00_0_Cr]<1;2,0>:ub

	// Set UV MaskA and MaskB
	mov (2)		f0.0<1>:uw		MaskA<2;2,1>:uw

	CALL(FILTER_UV_MBAFF, 1)	

	jmpi H0_UV_DONE	
	
DUAL_FIELD_UV:
	// Dual field mode, FieldModeCurrentMbFlag=0 && FieldModeAboveMbFlag=1

	//===== Ext U, Top field

	// Extract UV MaskA and MaskB from every other bit of Y masks
	and.nz.f0.0 (8) null:w			TempRow0(0)<16;8,2>		1:w
	and.nz.f0.1 (8) null:w			TempRow1(0)<16;8,2>		1:w

	mov (1)	P_AddrReg:w		ABOVE_CUR_MB_BASE:w			{ NoDDClr }
	mov (1)	Q_AddrReg:w		ABOVE_CUR_MB_BASE+32:w		{ NoDDChk }

	mov (16) ABOVE_CUR_MB_UW(0)<1>	PREV_MB_UW(0, 0)<16;8,1>	// Copy p1, p0
	mov (16) ABOVE_CUR_MB_UW(1)<1>	SRC_UW(0, 0)<16;8,1>		// Copy q1, q0

	//===== Ext U, top field
	mov	(8) Mbaff_ALPHA(0,0)<1>		r[ECM_AddrReg, bAlphaTop0_Cb]<0;1,0>:ub
	mov	(8) Mbaff_BETA(0,0)<1>		r[ECM_AddrReg, bBetaTop0_Cb]<0;1,0>:ub
	mov (8) Mbaff_TC0(0,0)<1>		r[ECM_AddrReg, bTc0_h00_0_Cb]<1;2,0>:ub

	// Store UV MaskA and MaskB
	mov (2)		MaskA<1>:uw			f0.0<2;2,1>:uw

	CALL(FILTER_UV_MBAFF, 1)	// Ext U, top field

	//===== Ext V, top field
	mov (1)	P_AddrReg:w		ABOVE_CUR_MB_BASE+1:w		{ NoDDClr }
	mov (1)	Q_AddrReg:w		ABOVE_CUR_MB_BASE+33:w		{ NoDDChk }

	mov	(8) Mbaff_ALPHA(0,0)<1>		r[ECM_AddrReg, bAlphaTop0_Cr]<0;1,0>:ub
	mov	(8) Mbaff_BETA(0,0)<1>		r[ECM_AddrReg, bBetaTop0_Cr]<0;1,0>:ub
	mov (8) Mbaff_TC0(0,0)<1>		r[ECM_AddrReg, bTc0_h00_0_Cr]<1;2,0>:ub

	// Set UV MaskA and MaskB
	mov (2)		f0.0<1>:uw		MaskA<2;2,1>:uw

	CALL(FILTER_UV_MBAFF, 1)	// Ext U, top field

	// Prefetch for bottom field
	// Get bot field Luma maskA and maskB	
	shr (16)	TempRow0(0)<1>		r[ECM_AddrReg, wEdgeCntlMapA_ExtTopHorz1]<0;1,0>:uw		RRampW(0)
	shr (16)	TempRow1(0)<1>		r[ECM_AddrReg, wEdgeCntlMapB_ExtTopHorz1]<0;1,0>:uw		RRampW(0)

	// Save deblocked top field rows
	mov (8) PREV_MB_UW(1, 0)<1>		ABOVE_CUR_MB_UW(0, 8)	// Copy p0
	mov (8) SRC_UW(0, 0)<1>			ABOVE_CUR_MB_UW(1, 0)	// Copy q0
	//==========================================================================

	//===== Ext U, Bot field 
	
	// Extract UV MaskA and MaskB from every other bit of Y masks
	and.nz.f0.0 (8) null:w			TempRow0(0)<16;8,2>		1:w
	and.nz.f0.1 (8) null:w			TempRow1(0)<16;8,2>		1:w

	mov (1)	P_AddrReg:w		ABOVE_CUR_MB_BASE:w			{ NoDDClr }
	mov (1)	Q_AddrReg:w		ABOVE_CUR_MB_BASE+32:w		{ NoDDChk }

	mov (16) ABOVE_CUR_MB_UW(0)<1>	PREV_MB_UW(0, 8)<16;8,1>	// Copy p1, p0
	mov (16) ABOVE_CUR_MB_UW(1)<1>	SRC_UW(0, 8)<16;8,1>		// Copy q1, q0

	//===== Ext U, bottom field
	mov	(8) Mbaff_ALPHA(0,0)<1>		r[ECM_AddrReg, bAlphaTop1_Cb]<0;1,0>:ub
	mov	(8) Mbaff_BETA(0,0)<1>		r[ECM_AddrReg, bBetaTop1_Cb]<0;1,0>:ub
	mov (8) Mbaff_TC0(0,0)<1>		r[ECM_AddrReg, bTc0_h00_1_Cb]<1;2,0>:ub

	// Store UV MaskA and MaskB
	mov (2)		MaskA<1>:uw			f0.0<2;2,1>:uw

	CALL(FILTER_UV_MBAFF, 1)	// Ext U, bottom field

	//===== Ext V, bot field
	mov (1)	P_AddrReg:w		ABOVE_CUR_MB_BASE+1:w		{ NoDDClr }
	mov (1)	Q_AddrReg:w		ABOVE_CUR_MB_BASE+33:w		{ NoDDChk }

	mov	(8) Mbaff_ALPHA(0,0)<1>		r[ECM_AddrReg, bAlphaTop1_Cr]<0;1,0>:ub
	mov	(8) Mbaff_BETA(0,0)<1>		r[ECM_AddrReg, bBetaTop1_Cr]<0;1,0>:ub
	mov (8) Mbaff_TC0(0,0)<1>		r[ECM_AddrReg, bTc0_h00_1_Cr]<1;2,0>:ub

	// Set UV MaskA and MaskB
	mov (2)		f0.0<1>:uw		MaskA<2;2,1>:uw

	CALL(FILTER_UV_MBAFF, 1)	// Ext V, bottom field
	
	// Save deblocked bot field rows
	mov (8) PREV_MB_UW(1, 8)<1>		ABOVE_CUR_MB_UW(0, 8)	// Copy p0
	mov (8) SRC_UW(0, 8)<1>			ABOVE_CUR_MB_UW(1, 0)	// Copy q0
	//========================================

H0_UV_DONE:

//---------- Deblock U internal horz middle edge ----------

	//***** Need to take every other bit to form U maskA in core
	shr (16)	TempRow0(0)<1>		r[ECM_AddrReg, wEdgeCntlMap_IntMidHorz]<0;1,0>:uw		RRampW(0)

	//	p1 = Cur MB U row 2
	//	p0 = Cur MB U row 3
	// 	q0 = Cur MB U row 4
	//	q1 = Cur MB U row 5
	mov (1)	P_AddrReg:w		4*UV_ROW_WIDTH+SRC_MB_U_BASE:w		{ NoDDClr }
	mov (1)	Q_AddrReg:w		8*UV_ROW_WIDTH+SRC_MB_U_BASE:w		{ NoDDChk }

	mov	(8) Mbaff_ALPHA(0,0)<1>		r[ECM_AddrReg, bAlphaInternal_Cb]<0;1,0>:ub
	mov	(8) Mbaff_BETA(0,0)<1>		r[ECM_AddrReg, bBetaInternal_Cb]<0;1,0>:ub
	mov (8) Mbaff_TC0(0,0)<1>		r[ECM_AddrReg, bTc0_h20_Cb]<1;2,0>:ub

	and.nz.f0.0 (8) null:w			TempRow0(0)<16;8,2>		1:w

	// Store UV MaskA and MaskB
	mov (1)	f0.1:uw		0:w
	mov (1)	MaskB:uw	0:w			{ NoDDClr }
	mov (1)	MaskA:uw	f0.0:uw		{ NoDDChk }

	CALL(FILTER_UV_MBAFF, 1)	
//-----------------------------------------------


//---------- Deblock V internal horz middle edge ----------

	//	p1 = Cur MB V row 2
	//	p0 = Cur MB V row 3
	// 	q0 = Cur MB V row 4
	//	q1 = Cur MB V row 5
	mov (1)	P_AddrReg:w		4*UV_ROW_WIDTH+SRC_MB_V_BASE:w		{ NoDDClr }
	mov (1)	Q_AddrReg:w		8*UV_ROW_WIDTH+SRC_MB_V_BASE:w		{ NoDDChk }

	mov	(8) Mbaff_ALPHA(0,0)<1>		r[ECM_AddrReg, bAlphaInternal_Cr]<0;1,0>:ub
	mov	(8) Mbaff_BETA(0,0)<1>		r[ECM_AddrReg, bBetaInternal_Cr]<0;1,0>:ub
	mov (8) Mbaff_TC0(0,0)<1>		r[ECM_AddrReg, bTc0_h20_Cr]<1;2,0>:ub

	// Set UV MaskA and MaskB
	mov (2)		f0.0<1>:uw		MaskA<2;2,1>:uw

	CALL(FILTER_UV_MBAFF, 1)	
//-----------------------------------------------


