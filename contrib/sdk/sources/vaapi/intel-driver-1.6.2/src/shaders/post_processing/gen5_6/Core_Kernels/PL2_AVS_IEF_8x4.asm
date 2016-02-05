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

//---------- PL2_AVS_IEF_8x4.asm ----------

#include "AVS_IEF.inc"

//------------------------------------------------------------------------------
// 2 sampler reads for 8x8 Y each
// 1 sampler read for 8x8 U and 8x8 V (NV11\NV12 input surface)
//------------------------------------------------------------------------------

    // 1st 8x8 setup
    #include "AVS_SetupFirstBlock.asm"

    // Enable green channel only
    mov (1) rAVS_8x8_HDR.2:ud      nAVS_GREEN_CHANNEL_ONLY:ud               

    mov (16) mAVS_8x8_HDR.0:ud      rAVS_8x8_HDR.0<8;8,1>:ud    // Copy msg header and payload mirrors to MRFs
    send (1) uwAVS_RESPONSE(0)<1>   mAVS_8x8_HDR   udDUMMY_NULL      nSMPL_ENGINE        nAVS_MSG_DSC_1CH+nSI_SRC_Y+nBI_CURRENT_SRC_Y
    // Return Y in 4 GRFs

    // 8x8 U and V sampling 
    // Enable red and blue channels  
    //Only 8x4 wil be used  
    mov (1) rAVS_8x8_HDR.2:ud  nAVS_RED_BLUE_CHANNELS:ud                   

    // Calculate Chroma Step Size:
    // for H direction: 16 Luma samples are covered by 8 Chroma samples. Thus Chroma_Step_X = 2 * Luma_Step_X 
    // for V direction: 8  Luma samples are covered by 8 Chroma samples. Thus Chroma_Step_Y = Luma_Step_Y
    mul  (1)  rAVS_PAYLOAD.1:f      fVIDEO_STEP_X:f    2.0:f             // Step X for chroma

    mov (16) mAVS_8x8_HDR_UV.0:ud      rAVS_8x8_HDR.0<8;8,1>:ud    // Copy msg header and payload mirrors to MRFs
    send (1) uwAVS_RESPONSE(4)<1> mAVS_8x8_HDR_UV   udDUMMY_NULL  nSMPL_ENGINE    nAVS_MSG_DSC_2CH+nSI_SRC_UV+nBI_CURRENT_SRC_UV
    // Return U and V in 8 GRFs

    // 2nd 8x8 setup
    #include "AVS_SetupSecondBlock.asm"

    // 2nd 8x8 Y sampling
    // Enable green channel only
    mov (1) rAVS_8x8_HDR.2:ud      nAVS_GREEN_CHANNEL_ONLY:ud                           

    mov (16) mAVS_8x8_HDR.0:ud      rAVS_8x8_HDR.0<8;8,1>:ud    // Copy msg header and payload mirrors to MRFs
    send (1) uwAVS_RESPONSE_2(0)<1>    mAVS_8x8_HDR    udDUMMY_NULL    nSMPL_ENGINE    nAVS_MSG_DSC_1CH+nSI_SRC_Y+nBI_CURRENT_SRC_Y 

//------------------------------------------------------------------------------
// Unpacking sampler reads to 4:2:0 internal planar 
//------------------------------------------------------------------------------
    #include "PL2_AVS_IEF_Unpack_8x4.asm"

