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

// File name: Scaling.inc

#ifndef _SCALING_INC_
#define _SCALING_INC_

// Local variables----------------------------------------------------------------------------------
#define MSGHDR_SCALE		m1		// Message Payload Header (Uses m2, m3, m4, m5 implicitly)

//--------------------------------------------------------------------------------------------------
//r10.0 thru r33.0; Primary surface read from sampler (16x8)
#define DEST_Y			uwTOP_Y
#define DEST_U			uwTOP_U
#define DEST_V			uwTOP_V

//r10.0 thru r41.0
.declare DEST_B		Base=REG(r,10)	ElementSize=2 SrcRegion=REGION(8,1) DstRegion=<1> Type=uw
.declare DEST_G		Base=REG(r,18)	ElementSize=2 SrcRegion=REGION(8,1) DstRegion=<1> Type=uw
.declare DEST_R		Base=REG(r,26)	ElementSize=2 SrcRegion=REGION(8,1) DstRegion=<1> Type=uw
.declare DEST_A		Base=REG(r,34)	ElementSize=2 SrcRegion=REGION(8,1) DstRegion=<1> Type=uw

//r56.0 thru r79.0
.declare	SCALE_RESPONSE_YF  	Base=REG(r,nBOT_Y) 	ElementSize=4 SrcRegion=REGION(8,1) Type=f
.declare	SCALE_RESPONSE_UF  	Base=REG(r,nBOT_U) 	ElementSize=4 SrcRegion=REGION(8,1) Type=f
.declare	SCALE_RESPONSE_VF  	Base=REG(r,nBOT_V) 	ElementSize=4 SrcRegion=REGION(8,1) Type=f

.declare	SCALE_RESPONSE_YW  	Base=REG(r,nBOT_Y) 	ElementSize=2 SrcRegion=REGION(16,1) Type=uw
.declare	SCALE_RESPONSE_UW  	Base=REG(r,nBOT_U) 	ElementSize=2 SrcRegion=REGION(16,1) Type=uw
.declare	SCALE_RESPONSE_VW  	Base=REG(r,nBOT_V) 	ElementSize=2 SrcRegion=REGION(16,1) Type=uw

.declare	SCALE_RESPONSE_YD  	Base=REG(r,nBOT_Y) 	ElementSize=4 SrcRegion=REGION(8,1) Type=ud
.declare	SCALE_RESPONSE_UD  	Base=REG(r,nBOT_U) 	ElementSize=4 SrcRegion=REGION(8,1) Type=ud
.declare	SCALE_RESPONSE_VD  	Base=REG(r,nBOT_V) 	ElementSize=4 SrcRegion=REGION(8,1) Type=ud

.declare	SCALE_RESPONSE_YB  	Base=REG(r,nBOT_Y) 	ElementSize=1 SrcRegion=REGION(8,4) Type=ub
.declare	SCALE_RESPONSE_UB  	Base=REG(r,nBOT_U) 	ElementSize=1 SrcRegion=REGION(8,4) Type=ub
.declare	SCALE_RESPONSE_VB  	Base=REG(r,nBOT_V) 	ElementSize=1 SrcRegion=REGION(8,4) Type=ub

.declare	SAMPLER_RAMP   	Base=REG(r,42) ElementSize=4 SrcRegion=<8;8,1> Type=f	// 2 GRFs, 16 elements

//#define	SCALE_STEP_X	REG2(r,43,0)
//#define	SCALE_COORD_X	REG2(r,43,3)

#define SCALE_LINE_P255			REG2(r,43,4)	// = 255.0	Used in 'line' inst to multiply 255, add 0.5, and round to int.
#define SCALE_LINE_P0_5			REG2(r,43,7)	// = 0.5

//r44.0 thru r45.0
#define SCALE_COORD_Y		REG(r,44)	//2GRF


// Send Message [DevILK]                                Message Descriptor
//  MBZ MsgL=5 MsgR=8                            H MBZ   SIMD     MsgType   SmplrIndx BindTab
//  000 0 101 0 1000                             1  0     10     0000         0000    00000000
//    0     A    8                                     A             0             0     0     0
//     MsgL=1+2*2(u,v)=5 MsgR=8
#define SMPLR_MSG_DSC		0x0A8A0000	// ILK Sampler Message Descriptor

// Re-define new number of lines
#undef nY_NUM_OF_ROWS
#undef nUV_NUM_OF_ROWS

#define nY_NUM_OF_ROWS      8
#define nUV_NUM_OF_ROWS     8


#endif 	//_SCALING_INC_
