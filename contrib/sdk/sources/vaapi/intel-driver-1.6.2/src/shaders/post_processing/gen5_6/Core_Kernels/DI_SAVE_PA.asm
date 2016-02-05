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

    shl (1) rMSGSRC.0<1>:ud     wORIX<0;1,0>:w            1:w  NODDCLR             // H. block origin need to be doubled
    mov (1) rMSGSRC.1<1>:ud     wORIY<0;1,0>:w                 NODDCLR_NODDCHK    // Block origin
    mov (1) rMSGSRC.2<1>:ud     nDPW_BLOCK_SIZE_DI:ud          NODDCHK             // Block width and height (32x8)
    
	
	add (4) pCF_Y_OFFSET<1>:uw   ubDEST_CF_OFFSET<4;4,1>:ub   nDEST_YUV_REG*nGRFWIB:w    // Initial Y,U,V offset in YUV422 block

	// Pack 2nd field Y
    $for(0; <nY_NUM_OF_ROWS; 1) {
		mov     (16) r[pCF_Y_OFFSET, %1*nGRFWIB]<2>       ubRESP(nDI_PREV_FRAME_LUMA_OFFSET,%1*16)
    }
	// Pack 1st field Y
    $for(0; <nY_NUM_OF_ROWS; 1) {
		mov     (16) r[pCF_Y_OFFSET, %1+4*nGRFWIB]<2>       ubRESP(nDI_CURR_FRAME_LUMA_OFFSET,%1*16)
    }
	// Pack 2nd field U
    $for(0; <nUV_NUM_OF_ROWS; 1) {
        mov (8) r[pCF_U_OFFSET,   %1*nGRFWIB]<4>  ubRESP(nDI_PREV_FRAME_CHROMA_OFFSET,%1*16+1)<16;8,2>  //U pixels
    }
	 // Pack 1st field U
    $for(0; <nUV_NUM_OF_ROWS; 1) {
        mov (8) r[pCF_U_OFFSET,   %1+4*nGRFWIB]<4>  ubRESP(nDI_CURR_FRAME_CHROMA_OFFSET,%1*16+1)<16;8,2>  //U pixels
    }
	// Pack 2nd field V
    $for(0; <nUV_NUM_OF_ROWS; 1) {
        mov (8) r[pCF_V_OFFSET,   %1*nGRFWIB]<4>  ubRESP(nDI_PREV_FRAME_CHROMA_OFFSET,%1*16)<16;8,2>  //Vpixels
    }
	// Packs1st field V
    $for(0; <nUV_NUM_OF_ROWS; 1) {
        mov (8) r[pCF_V_OFFSET,   %1+4*nGRFWIB]<4>  ubRESP(nDI_CURR_FRAME_CHROMA_OFFSET,%1*16)<16;8,2>  //Vpixels
    }

    //save the previous frame
    mov (8) mMSGHDR<1>:ud       rMSGSRC<8;8,1>:ud
    $for(0; <4; 1) {
            mov (8) mudMSGPAYLOAD(%1)<1>  udDEST_YUV(%1)REGION(8,1)
    }
    send (8)    dNULLREG    mMSGHDR   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_DI+nBI_DESTINATION_1_YUV:ud

    //save the current frame
    mov (8) mMSGHDR<1>:ud       rMSGSRC<8;8,1>:ud
    $for(0; <4; 1) {
            mov (8) mudMSGPAYLOAD(%1)<1>  udDEST_YUV(%1+4)REGION(8,1)
    }
    send (8)    dNULLREG    mMSGHDR   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPW_MSG_SIZE_DI+nBI_DESTINATION_2_YUV:ud
	
