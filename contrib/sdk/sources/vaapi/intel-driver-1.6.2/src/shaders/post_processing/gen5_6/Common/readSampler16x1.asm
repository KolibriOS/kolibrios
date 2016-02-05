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

// Module name: readSampler16x1.asm
//
// Read one row of pix through sampler
//



//#define SAMPLER_MSG_DSC		0x166A0000	// ILK Sampler Message Descriptor



// Send Message [DevILK]                                Message Descriptor
//  MBZ MsgL=5 MsgR=8                            H MBZ   SIMD     MsgType   SmplrIndx BindTab
//  000 0 101 0 1000                             1  0     10     0000         0000    00000000
//    0     A    8                                     A             0             0     0     0

//     MsgL=1+2*2(u,v)=5 MsgR=8
 
#define SAMPLER_MSG_DSC		0x0A8A0000	// ILK Sampler Message Descriptor





                                                                                

	// Assume MSGSRC is set already in the caller
        //mov (8)		rMSGSRC.0<1>:ud			0:ud	// Unused fileds



	// Read 16 sampled pixels and stored them in float32 in 8 GRFs
	// 422 data is expanded to 444, return 8 GRF in the order of RGB- (UYV-).
	// 420 data has three surfaces, return 8 GRF. Valid is always in the 1st GRF when in R8.  Make sure no overwrite the following 3 GRFs.
	// alpha data is expanded to 4444, return 8 GRF in the order of RGBA (UYVA).

    mov(16)     mMSGHDR<1>:uw   rMSGSRC<16;16,1>:uw
    send (16)	DATABUF(0)<1>	mMSGHDR		udDUMMY_NULL	0x2 SAMPLER_MSG_DSC+SAMPLER_IDX+BINDING_IDX:ud




    


