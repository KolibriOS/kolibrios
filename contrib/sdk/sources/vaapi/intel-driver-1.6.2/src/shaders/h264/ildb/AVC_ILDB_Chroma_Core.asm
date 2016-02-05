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
#if !defined(__AVC_ILDB_CHROMA_CORE__)	// Make sure this file is only included once
#define __AVC_ILDB_CHROMA_CORE__

////////// AVC ILDB Chroma Core /////////////////////////////////////////////////////////////////////////////////
//
//	This core performs AVC U or V ILDB filtering on one horizontal edge (8 pixels) of a MB.
//	If data is transposed, it can also de-block a vertical edge.
//
//	Bafore calling this subroutine, caller needs to set the following parameters.
//
//	- EdgeCntlMap1				//	Edge control map A
//	- EdgeCntlMap2				//	Edge control map B
//	- P_AddrReg					//	Src and dest address register for P pixels
//	- Q_AddrReg					//	Src and dest address register for Q pixels 	
//	- alpha						//  alpha corresponding to the edge to be filtered
//	- beta						//  beta corresponding to the edge to be filtered
//	- tc0						// 	tc0  corresponding to the edge to be filtered
//
//	U or V:
//	+----+----+----+----+
//	| P1 | p0 | q0 | q1 |
//	+----+----+----+----+
//
//	p1 = r[P_AddrReg, 0]<16;8,2> 
//	p0 = r[P_AddrReg, 16]<16;8,2> 
// 	q0 = r[Q_AddrReg, 0]<16;8,2>  
//	q1 = r[Q_AddrReg, 16]<16;8,2> 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The region is both src and dest
// P0-P3 and Q0-Q3 should be only used if they have not been modified to new values  
#undef 	P1
#undef 	P0
#undef 	Q0
#undef 	Q1

#define P1 		r[P_AddrReg,  0]<16;8,2>:ub
#define P0 		r[P_AddrReg, 16]<16;8,2>:ub
#define Q0 		r[Q_AddrReg,  0]<16;8,2>:ub
#define Q1 		r[Q_AddrReg, 16]<16;8,2>:ub

// New region as dest
#undef 	NewP0
#undef 	NewQ0

#define NewP0 	r[P_AddrReg, 16]<2>:ub
#define NewQ0 	r[Q_AddrReg,  0]<2>:ub

// Filter one chroma edge 
FILTER_UV:

#if defined(_DEBUG) 
	mov		(1)		EntrySignatureC:w			0x1112:w
#endif
	//---------- Derive filterSampleflag in AVC spec, equition (8-469) ----------
	// bS is in MaskA

	// Src copy of the p1, p0, q0, q1
//	mov (8) p1(0)<1>		r[P_AddrReg, 0]<16;8,2>:ub
//	mov (8) p0(0)<1>		r[P_AddrReg, 16]<16;8,2>:ub
//	mov (8) q0(0)<1>		r[Q_AddrReg, 0]<16;8,2>:ub
//	mov (8) q1(0)<1>		r[Q_AddrReg, 16]<16;8,2>:ub

//	mov (1)	f0.0:uw		MaskA:uw

	add (8) q0_p0(0)<1>			Q0		-P0				// q0-p0
	add (8) TempRow0(0)<1>		P1		-P0				// p1-p0
	add (8) TempRow1(0)<1>		Q1		-Q0				// q1-q0

	// Build FilterSampleFlag
	// abs(q0-p0) < alpha
	(f0.0) cmp.l.f0.0 (16) null:w		(abs)q0_p0(0)			alpha:w
	// abs(p1-p0) < Beta
	(f0.0) cmp.l.f0.0 (16) null:w		(abs)TempRow0(0)		beta:w
	// abs(q1-q0) < Beta
	(f0.0) cmp.l.f0.0 (16) null:w		(abs)TempRow1(0)		beta:w

	//-----------------------------------------------------------------------------------------

	// if 
    (f0.0)	if	(8)		UV_ENDIF1
		// For channels whose edge control map1 = 1 ---> perform de-blocking

//		mov (1)		f0.1:w		MaskB:w		{NoMask}		// Now check for which algorithm to apply

		(f0.1)	if	(8)		UV_ELSE2

			// For channels whose edge control map2 = 1 ---> bS = 4 algorithm 
			// p0' = (2*p1 + p0 + q1 + 2) >> 2
			// q0' = (2*q1 + q0 + p1 + 2) >> 2

			// Optimized version:
			// A = (p1 + q1 + 2)
			// p0' = (p0 + p1 + A) >> 2
			// q0' = (q0 + q1 + A) >> 2
			//------------------------------------------------------------------------------------
			
			// p0' = (2*p1 + p0 + q1 + 2) >> 2
			add (8) acc0<1>:w		Q1				2:w
			mac (8) acc0<1>:w		P1				2:w
			add (8)	acc0<1>:w		acc0<8;8,1>:w	P0
			shr.sat	(8)	TempRow0B(0)<2>		acc0<8;8,1>:w		2:w
			
			// q0' = (2*q1 + q0 + p1 + 2) >> 2
			add (8) acc0<1>:w		P1				2:w
			mac (8) acc0<1>:w		Q1				2:w
			add (8)	acc0<1>:w		acc0<8;8,1>:w	Q0
			shr.sat	(8)	TempRow1B(0)<2>		acc0<8;8,1>:w		2:w

			mov (8) NewP0		TempRow0B(0)					// p0'
			mov (8) NewQ0		TempRow1B(0)					// q0'
			
			
UV_ELSE2: 
		else 	(8)		UV_ENDIF2
			// For channels whose edge control map2 = 0 ---> bS < 4 algorithm
			
			// Expand tc0	(tc0 has 4 bytes)
//			mov (8)	tc0_exp(0)<1>	tc0<1;2,0>:ub	{NoMask}				// tc0_exp = tc0, each tc0 is duplicated 2 times for 2 adjcent pixels	
			mov (8)	acc0<1>:w	tc0<1;2,0>:ub	{NoMask}				// tc0_exp = tc0, each tc0 is duplicated 2 times for 2 adjcent pixels	
			
			// tc_exp = tc0_exp + 1
//			add (8) tc_exp(0)<1>	tc0_exp(0)		1:w
			add (8) tc_exp(0)<1>	acc0<8;8,1>:w		1:w

			// delta = Clip3(-tc, tc, ((((q0 - p0)<<2) + (p1-q1) + 4) >> 3))
			// 4 * (q0-p0) + p1 - q1 + 4
			add (8)	acc0<1>:w		P1			4:w
			mac (8) acc0<1>:w		q0_p0(0)	4:w	
			add (8) acc0<1>:w		acc0<8;8,1>:w		-Q1
			shr (8) TempRow0(0)<1>	acc0<8;8,1>:w		3:w

			// tc clip
			cmp.g.f0.0	(8) null:w		TempRow0(0)		tc_exp(0)				// Clip if > tc0
			cmp.l.f0.1	(8) null:w		TempRow0(0)		-tc_exp(0)				// Clip if < -tc0
			
			(f0.0) mov (8) TempRow0(0)<1>				tc_exp(0)
			(f0.1) mov (8) TempRow0(0)<1>				-tc_exp(0)
			
			// p0' = Clip1(p0 + delta) = Clip3(0, 0xFF, p0 + delta)
			add.sat (8)	TempRow1B(0)<2>		P0			TempRow0(0)				// p0+delta
		
			// q0' = Clip1(q0 - delta) = Clip3(0, 0xFF, q0 - delta)
			add.sat (8)	TempRow0B(0)<2>		Q0			-TempRow0(0)			// q0-delta

			mov (8) NewP0				TempRow1B(0)			// p0'
			mov (8) NewQ0				TempRow0B(0)			// q0'

		endif
UV_ENDIF2:
UV_ENDIF1:
	endif

RETURN

#endif	// !defined(__AVC_ILDB_CHROMA_CORE__)
