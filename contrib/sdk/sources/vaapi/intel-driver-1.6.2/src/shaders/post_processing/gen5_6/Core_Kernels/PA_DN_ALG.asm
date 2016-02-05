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

#define DI_DISABLE

#include "DNDI.inc"

#undef  nY_NUM_OF_ROWS
#define nY_NUM_OF_ROWS         8                                 // Number of Y rows per block
#undef  nUV_NUM_OF_ROWS
#define nUV_NUM_OF_ROWS        8                                 // Number of U/V rows per block

#undef   nSMPL_RESP_LEN
#define  nSMPL_RESP_LEN        nSMPL_RESP_LEN_DN_PA              // Set the Number of GRFs in DNDI response 
#undef   nDPW_BLOCK_SIZE_DN
#define  nDPW_BLOCK_SIZE_DN    nBLOCK_WIDTH_32+nBLOCK_HEIGHT_8   // DN Curr Block Size for Write is 32x8
#undef   nDPW_BLOCK_SIZE_HIST
#define  nDPW_BLOCK_SIZE_HIST  nBLOCK_WIDTH_4+nBLOCK_HEIGHT_2    // HIST Block Size for Write is 4x2

////////////////////////////////////// Run the DN Algorithm ///////////////////////////////////////
#include "DNDI_COMMAND.asm"

////////////////////////////////////// Save the History Data for Next Run /////////////////////////
#include "DNDI_Hist_Save.asm"

////////////////////////////////////// Pack and Save the DN Curr Frame for Next Run ///////////////
add (4)     pCF_Y_OFFSET<1>:uw    ubDEST_CF_OFFSET<4;4,1>:ub    npDN_YUV:w 
$for (0; <nY_NUM_OF_ROWS; 1) {
    mov (16)    r[pCF_Y_OFFSET,  %1*32]<2>:ub   ubRESP(nNODI_LUMA_OFFSET,%1*16)<16;16,1>       // copy line of Y
}
$for (0; <nUV_NUM_OF_ROWS; 1) {
    mov (8)     r[pCF_U_OFFSET,  %1*32]<4>:ub   ubRESP(nNODI_CHROMA_OFFSET,%1*16+1)<16;8,2>    // copy line of U
    mov (8)     r[pCF_V_OFFSET,  %1*32]<4>:ub   ubRESP(nNODI_CHROMA_OFFSET,%1*16)<16;8,2>      // copy line of V
}

shl (1)     rMSGSRC.0<1>:ud     wORIX<0;1,0>:w     1:w       // X origin * 2 (422 output)
mov (1)     rMSGSRC.1<1>:ud     wORIY<0;1,0>:w               // Y origin
mov (1)     rMSGSRC.2<1>:ud     nDPW_BLOCK_SIZE_DN:ud        // block width and height (32x8)
mov (8)     mMSGHDR_DN<1>:ud    rMSGSRC<8;8,1>:ud            // message header   

$for(0; <nY_NUM_OF_ROWS; 2) {
        mov (16) mudMSGHDR_DN(1+%1)<1>  udDN_YUV(%1)REGION(8,1)    // Move DN Curr to MRF
}
send (8)    dNULLREG    mMSGHDR_DN   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPMW_MSG_LEN_PA_DN_NODI+nBI_DESTINATION_YUV:ud     



