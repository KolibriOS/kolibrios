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

#define DI_ENABLE

    #include "DNDI.inc"
    
    #undef  nY_NUM_OF_ROWS
    #define nY_NUM_OF_ROWS      8       // Number of Y rows per block (4 rows for each frame) 
    #undef  nUV_NUM_OF_ROWS
    #define nUV_NUM_OF_ROWS     8       // Number of U/V rows per block

    #undef  nSMPL_RESP_LEN
    #define nSMPL_RESP_LEN          nSMPL_RESP_LEN_DNDI               // set the number of GRF 
    #undef  nDPW_BLOCK_SIZE_HIST
    #define nDPW_BLOCK_SIZE_HIST    nBLOCK_WIDTH_4+nBLOCK_HEIGHT_1    // HIST Block Size for Write is 4x2
    #undef  nDPW_BLOCK_SIZE_DN
    #define nDPW_BLOCK_SIZE_DN      nBLOCK_WIDTH_16+nBLOCK_HEIGHT_4   // DN Block Size for Write is 16x4
    #undef  nDPR_BLOCK_SIZE_UV
    #define nDPR_BLOCK_SIZE_UV			nBLOCK_WIDTH_8+nBLOCK_HEIGHT_2   //  DN Block Size for UV Write/Read is 8x2
   
////////////////////////////////////// Run the DN Algorithm ///////////////////////////////////////
    #include "DNDI_Command.asm"

////////////////////////////////////// Rearrange for Internal Planar //////////////////////////////
    // move the previous frame Y component to internal planar format
    $for (0; <nY_NUM_OF_ROWS/2; 1) {
        mov (16) uwDEST_Y(%1,0)<1>    ubRESP(nDI_PREV_FRAME_LUMA_OFFSET,%1*16)
    }
    // move the previous frame U,V components to internal planar format
    $for (0; <nUV_NUM_OF_ROWS/2; 1) {
        mov (8) uwDEST_U(0,%1*8)<1>   ubRESP(nDI_PREV_FRAME_CHROMA_OFFSET,%1*16+1)<16;8,2>  //U pixels
        mov (8) uwDEST_V(0,%1*8)<1>   ubRESP(nDI_PREV_FRAME_CHROMA_OFFSET,%1*16)<16;8,2>    //V pixels
    }
    // move the current frame Y component to internal planar format
    $for (0; <nY_NUM_OF_ROWS/2; 1) {
        mov (16) uwDEST_Y(%1+4,0)<1>  ubRESP(nDI_CURR_FRAME_LUMA_OFFSET,%1*16)
    }
    // move the current frame U,V components to internal planar format
    $for (0; <nUV_NUM_OF_ROWS/2; 1) {
        mov (8) uwDEST_U(2,%1*8)<1>   ubRESP(nDI_CURR_FRAME_CHROMA_OFFSET,%1*16+1)<16;8,2>  //U pixels
        mov (8) uwDEST_V(2,%1*8)<1>   ubRESP(nDI_CURR_FRAME_CHROMA_OFFSET,%1*16)<16;8,2>    //V pixels
    }

////////////////////////////////////// Save the STMM Data for Next Run /////////////////////////
    // Write STMM to memory
    shr (1)     rMSGSRC.0<1>:ud        wORIX<0;1,0>:w            1:w     // X origin / 2
    mov (1)     rMSGSRC.1<1>:ud        wORIY<0;1,0>:w                    // Y origin
    mov (1)     rMSGSRC.2<1>:ud        nDPW_BLOCK_SIZE_STMM:ud           // block width and height (8x4)
    mov (8)     mudMSGHDR_STMM(0)<1>   rMSGSRC.0<8;8,1>:ud               // message header   
    mov (8)     mudMSGHDR_STMM(1)<1>   udRESP(nDI_STMM_OFFSET,0)         // Move STMM to MRF 
    send (8)    dNULLREG               mMSGHDR_STMM              udDUMMY_NULL    nDATAPORT_WRITE     nDPMW_MSGDSC+nDPMW_MSG_LEN_STMM+nBI_STMM_HISTORY_OUTPUT:ud      

////////////////////////////////////// Save the History Data for Next Run /////////////////////////
    #include "DI_Hist_Save.asm"

////////////////////////////////////// Save the DN Curr Frame for Next Run ////////////////////////
    add (4)     pCF_Y_OFFSET<1>:uw          ubSRC_CF_OFFSET<4;4,1>:ub  npDN_YUV:w
    // check top/bottom field first
    cmp.e.f0.0 (1)  null<1>:w               ubTFLD_FIRST<0;1,0>:ub     1:w
    (f0.0) jmpi (1) TOP_FIELD_FIRST

BOTTOM_FIELD_FIRST:
    $for (0,0; <nY_NUM_OF_ROWS/2; 2,1) {
        mov (4)     mudMSGHDR_DN(1,%1*4)<1>     udRESP(nDI_CURR_2ND_FIELD_LUMA_OFFSET,%2*4)<4;4,1> // 2nd field luma from current frame (line 0,2)
        mov (4)     mudMSGHDR_DN(1,%1*4+4)<1>   udRESP(nDI_CURR_FRAME_LUMA_OFFSET+%2,4)<4;4,1> // 1st field luma from current frame (line 1,3)
    }
    jmpi (1) SAVE_DN_CURR
    
TOP_FIELD_FIRST:
    $for (0,0; <nY_NUM_OF_ROWS/2; 2,1) {
        mov (4)     mudMSGHDR_DN(1,%1*4)<1>     udRESP(nDI_CURR_FRAME_LUMA_OFFSET+%2,0)<4;4,1> // 2nd field luma from current frame (line 0,2)
        mov (4)     mudMSGHDR_DN(1,%1*4+4)<1>   udRESP(nDI_CURR_2ND_FIELD_LUMA_OFFSET,%2*4)<4;4,1> // 1st field luma from current frame (line 1,3)
    }
SAVE_DN_CURR:
    mov (2)     rMSGSRC.0<1>:ud        wORIX<2;2,1>:w               // X origin and Y origin
    mov (1)     rMSGSRC.2<1>:ud        nDPW_BLOCK_SIZE_DN:ud        // block width and height (16x4)
    mov (8)     mudMSGHDR_DN(0)<1>     rMSGSRC.0<8;8,1>:ud
    send (8)    dNULLREG    mMSGHDR_DN   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nDPMW_MSG_LEN_PL_DN_DI+nBI_DESTINATION_Y:ud


/////////////////////////////IMC3 UV Copy 422/////////////////////////////////////////////////////
		//Read UV through DATAPORT    
    add  (2) rMSGSRC.0<1>:d     wORIX<2;2,1>:w    wSRC_H_ORI_OFFSET<2;2,1>:w       // Source Y Block origin
    asr (2)  rMSGSRC.0<1>:d     rMSGSRC.0<2;2,1>:d       1:w   // U/V block origin should be half of Y's
    mov (1)  rMSGSRC.2<1>:ud    nDPR_BLOCK_SIZE_UV:ud          // U/V block width and height (8x2)  
    mov  (8) mudMSGHDR_DN<1>     rMSGSRC<8;8,1>:ud
    send (4) udBOT_U_IO(0)<1>       mMSGHDR_DN    udDUMMY_NULL    nDATAPORT_READ    nDPMR_MSGDSC+nRESLEN_1+nBI_CURRENT_SRC_U:ud
    send (4) udBOT_V_IO(0)<1>       mMSGHDR_DN    udDUMMY_NULL    nDATAPORT_READ    nDPMR_MSGDSC+nRESLEN_1+nBI_CURRENT_SRC_V:ud

 		//Write UV through DATAPORT
		mov (2)     rMSGSRC.0<1>:ud        wORIX<2;2,1>:w               // X origin and Y origin
		asr  (2)    rMSGSRC.0<1>:d     wORIX<2;2,1>:w    1:w   // U/V block origin should be half of Y's
    mov (1)     rMSGSRC.2<1>:ud        nDPR_BLOCK_SIZE_UV:ud        // block width and height (8x2)
    mov (8)     mudMSGHDR_DN(0)<1>     rMSGSRC.0<8;8,1>:ud
    mov (4)			mudMSGHDR_DN(1)<1>		 udBOT_U_IO(0)<4;4,1>
    send (4)    dNULLREG    mMSGHDR_DN   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nMSGLEN_1+nBI_DESTINATION_U:ud
    mov (4)			mudMSGHDR_DN(1)<1>		 udBOT_V_IO(0)<4;4,1>
    send (4)    dNULLREG    mMSGHDR_DN   udDUMMY_NULL    nDATAPORT_WRITE    nDPMW_MSGDSC+nMSGLEN_1+nBI_DESTINATION_V:ud