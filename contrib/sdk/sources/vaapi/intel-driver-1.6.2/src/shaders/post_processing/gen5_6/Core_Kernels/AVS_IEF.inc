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

// Module name: AVS_IEF.inc

#ifndef _AVS_INF_INC_
#define _AVS_INF_INC_

#include "undefall.inc"             //Undefine the SRC and DEST sysmbols

        // Message Header
        // m0.7         31:0    Debug
        // m0.6         31:0    Debug
        // m0.5         31:0    Ignored
        // m0.4         31:0    Ignored
        // m0.3         31:0    Ignored
        // m0.2         31:16   Ignored
        //              15      Alpha Write Channel Mask        enable=0, disable=1
        //              14      Blue Write Channel Mask  (V)    
        //              13      Green Write Channel Mask (Y)
        //              12      Red Write Channel Mask   (U)
        //              11:0    Ignored
        // m0.1                 Ignored
        // m0.0                 Ignored

#define mAVS_8x8_HDR   m0               // Message Header
#define mAVS_PAYLOAD   m1               // Message Payload Header

#define mAVS_8x8_HDR_2   m2               // Message Header
#define mAVS_PAYLOAD_2   m3               // Message Payload Header

#define mAVS_8x8_HDR_UV   m2               // Message Header
#define mAVS_PAYLOAD_UV   m3               // Message Payload Header

#define rAVS_8x8_HDR   rMSGSRC          // Mirror of Message Header 
#define rAVS_PAYLOAD   r9               // Mirror of Message Payload Header
        
        // AVS payload
        // m1.7                 Ignored
        // m1.6                 Pixel 0 V Address       ---> ORIY (Y0)
        // m1.5                 Delta V                 ---> Step Y
        // m1.4                 Ignored
        // m1.3                 Ignored
        // m1.2                 Pixel 0 U Address       ---> ORIX (X0)
        // m1.1                 U 2nd Derivative        ---> NLAS dx 
        // m1.0                 Delta U                 ---> Step X

        // Sampler Message Descriptor
        // 31:29        Reserved                        000
        // 28:25        Message length                  0010
        // 24:20        Response length                 xxxxx   ---> 4GRFs for each enabled channel
        // 19           Header Present                  1
        // 18           MBZ                             0
        // 17:16        SIMD Mode                       11      ---> SIMD64
        // 15:12        Message Type                    0011    ---> sample_8x8
        // 11:8         Sampler Index                   xxxx
        // 7:0          Binding Table Index             xxxxxxxx
#define nAVS_MSG_DSC_1CH        0x044BB000  
#define nAVS_MSG_DSC_2CH        0x048BB000
#define nAVS_MSG_DSC_3CH        0x04CBB000      
#define nAVS_MSG_DSC_4CH        0x050BB000 

#define nAVS_RED_CHANNEL_ONLY   0x0000E000      // Enable Red channel only
#define nAVS_GREEN_CHANNEL_ONLY 0x0000D000      // Enable Green channel only
#define nAVS_RED_BLUE_CHANNELS  0x0000A000      // Enable Red and Blue channels
#define nAVS_RGB_CHANNELS       0x00008000      // Enable RGB(YUV) channels
#define nAVS_ALL_CHANNELS       0x00000000      // Enable all channels (ARGB\AYUV)

        

.declare     ubAVS_RESPONSE  Base=REG(r,nTEMP8) ElementSize=1  SrcRegion=REGION(16,1) Type=ub
.declare     uwAVS_RESPONSE  Base=REG(r,nTEMP8) ElementSize=2  SrcRegion=REGION(16,1) Type=uw

.declare     ubAVS_RESPONSE_2  Base=REG(r,nTEMP24) ElementSize=1  SrcRegion=REGION(16,1) Type=ub
.declare     uwAVS_RESPONSE_2  Base=REG(r,nTEMP24) ElementSize=2  SrcRegion=REGION(16,1) Type=uw


#if (nSRC_REGION==nREGION_2)
    #define uwDEST_Y        uwBOT_Y
    #define uwDEST_U        uwBOT_U
    #define uwDEST_V        uwBOT_V

    #define ubDEST_Y        ubBOT_Y
    
    #undef  nSRC_REGION
    #define nSRC_REGION nREGION_2

#else //(nSRC_REGION==nREGION_1)
    #define uwDEST_Y        uwTOP_Y
    #define uwDEST_U        uwTOP_U
    #define uwDEST_V        uwTOP_V

    #define ubDEST_Y        ubTOP_Y
    
    #undef  nSRC_REGION
    #define nSRC_REGION     nREGION_1

#endif


#endif //_AVS_INF_INC_
