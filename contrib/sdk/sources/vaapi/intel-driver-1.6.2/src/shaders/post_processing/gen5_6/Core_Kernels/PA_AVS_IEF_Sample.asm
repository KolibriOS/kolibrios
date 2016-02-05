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

//---------- PA_AVS_IEF_Sample.asm ----------

//------------------------------------------------------------------------------
// 2 sampler reads for 8x8 YUV packed
//------------------------------------------------------------------------------
        
    // 1st 8x8 setup
    #include "AVS_SetupFirstBlock.asm"

    // Enable RGB(YUV) channels
    mov (1)  rAVS_8x8_HDR.2:ud      nAVS_RGB_CHANNELS:ud   

    mov (16) mAVS_8x8_HDR.0:ud      rAVS_8x8_HDR.0<8;8,1>:ud    // Copy msg header and payload mirrors to MRFs
    send (1) uwAVS_RESPONSE(0)<1>   mAVS_8x8_HDR    udDUMMY_NULL    nSMPL_ENGINE    nAVS_MSG_DSC_3CH+nSI_SRC_YUV+nBI_CURRENT_SRC_YUV
    // Return YUV in 12 GRFs

    // 2nd 8x8 setup
    #include "AVS_SetupSecondBlock.asm"

    mov (16) mAVS_8x8_HDR_2.0:ud      rAVS_8x8_HDR.0<8;8,1>:ud    // Copy msg header and payload mirrors to MRFs
    send (1) uwAVS_RESPONSE_2(0)<1> mAVS_8x8_HDR_2    udDUMMY_NULL    nSMPL_ENGINE    nAVS_MSG_DSC_3CH+nSI_SRC_YUV+nBI_CURRENT_SRC_YUV
    // Return YUV in 12 GRFs
        

