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
 * Authors:
 *    Halley Zhao <halley.zhao@intel.com>
 */

// Module name: PL16x8_PL8x4.asm
//----------------------------------------------------------------

#include "RGBX_Load_16x8.inc"

#if (0)
    #define nTEMP0          34        // transformation coefficient
    #define nTEMP1          35        // one row of Y (first half register is used)
    #define nTEMP2          36        // first  half of one row
    #define nTEMP3          37        // second half of one row
    #define nTEMP4          38        // mul and add
    #define nTEMP5          39        // mul and add
    #define nTEMP6          40        // mul and add
    #define nTEMP7          41        // mul and add
    #define nTEMP8          42        // sum of mul
    #define nTEMP10         44
    #define nTEMP12         46
    #define nTEMP14         48
    #define nTEMP16         50
    #define nTEMP17         51
    #define nTEMP18         52
    
    #define nTEMP24         58
#endif

$for(0; <nY_NUM_OF_ROWS; 1) {
    // BGRX | B | G | R | X |
    // ###### do on row for Y
    // #### mul and add
    mul (16)  REG2(r, nTEMP4, 0)<1>:uw      r[SRC_RGBA_OFFSET_1, %1*32 +  0]<0; 16,1>:ub        ubRGB_to_Y_Coef_Fix<0;4,1>:ub
    mul (16)  REG2(r, nTEMP5, 0)<1>:uw      r[SRC_RGBA_OFFSET_1, %1*32 + 16]<0; 16,1>:ub        ubRGB_to_Y_Coef_Fix<0;4,1>:ub
    mul (16)  REG2(r, nTEMP6, 0)<1>:uw      r[SRC_RGBA_OFFSET_2, %1*32 +  0]<0; 16,1>:ub        ubRGB_to_Y_Coef_Fix<0;4,1>:ub
    mul (16)  REG2(r, nTEMP7, 0)<1>:uw      r[SRC_RGBA_OFFSET_2, %1*32 + 16]<0; 16,1>:ub        ubRGB_to_Y_Coef_Fix<0;4,1>:ub

    add (4)   REG2(r, nTEMP4, 0)<4>:uw      REG2(r, nTEMP4, 0)<0;4,4>:uw      REG2(r, nTEMP4, 1)<0;4,4>:uw      
    add (4)   REG2(r, nTEMP5, 0)<4>:uw      REG2(r, nTEMP5, 0)<0;4,4>:uw      REG2(r, nTEMP5, 1)<0;4,4>:uw      
    add (4)   REG2(r, nTEMP6, 0)<4>:uw      REG2(r, nTEMP6, 0)<0;4,4>:uw      REG2(r, nTEMP6, 1)<0;4,4>:uw      
    add (4)   REG2(r, nTEMP7, 0)<4>:uw      REG2(r, nTEMP7, 0)<0;4,4>:uw      REG2(r, nTEMP7, 1)<0;4,4>:uw      
    add (4)   REG2(r, nTEMP4, 0)<4>:uw      REG2(r, nTEMP4, 0)<0;4,4>:uw      REG2(r, nTEMP4, 2)<0;4,4>:uw      
    add (4)   REG2(r, nTEMP5, 0)<4>:uw      REG2(r, nTEMP5, 0)<0;4,4>:uw      REG2(r, nTEMP5, 2)<0;4,4>:uw      
    add (4)   REG2(r, nTEMP6, 0)<4>:uw      REG2(r, nTEMP6, 0)<0;4,4>:uw      REG2(r, nTEMP6, 2)<0;4,4>:uw      
    add (4)   REG2(r, nTEMP7, 0)<4>:uw      REG2(r, nTEMP7, 0)<0;4,4>:uw      REG2(r, nTEMP7, 2)<0;4,4>:uw      

    // ####  write Y to the 1 row
    mov (4)  REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP4, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8,  4)<1>:uw    REG2(r, nTEMP5, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8,  8)<1>:uw    REG2(r, nTEMP6, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8, 12)<1>:uw    REG2(r, nTEMP7, 0)<0; 4, 4>:uw
    add (16) REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP8, 0)<0; 16, 1>:uw    0x1080:uw
    mov (16) REG2(r, nTEMP8,  0)<1>:ub    REG2(r, nTEMP8, 1)<0; 16, 2>:ub
    mov (16) uwDEST_Y(%1)<1>  REG2(r,nTEMP8, 0)<0;16,1>:ub

    // ######  do one row for U
    // #### mul and add
    mul (16)  REG2(r, nTEMP4, 0)<1>:w      r[SRC_RGBA_OFFSET_1, %1*32 +  0]<0; 16,1>:ub        bRGB_to_U_Coef_Fix<0;4,1>:b
    mul (16)  REG2(r, nTEMP5, 0)<1>:w      r[SRC_RGBA_OFFSET_1, %1*32 + 16]<0; 16,1>:ub        bRGB_to_U_Coef_Fix<0;4,1>:b
    mul (16)  REG2(r, nTEMP6, 0)<1>:w      r[SRC_RGBA_OFFSET_2, %1*32 +  0]<0; 16,1>:ub        bRGB_to_U_Coef_Fix<0;4,1>:b
    mul (16)  REG2(r, nTEMP7, 0)<1>:w      r[SRC_RGBA_OFFSET_2, %1*32 + 16]<0; 16,1>:ub        bRGB_to_U_Coef_Fix<0;4,1>:b
    
    add (4)   REG2(r, nTEMP4, 0)<4>:w      REG2(r, nTEMP4, 0)<0;4,4>:w      REG2(r, nTEMP4, 1)<0;4,4>:w      
    add (4)   REG2(r, nTEMP5, 0)<4>:w      REG2(r, nTEMP5, 0)<0;4,4>:w      REG2(r, nTEMP5, 1)<0;4,4>:w      
    add (4)   REG2(r, nTEMP6, 0)<4>:w      REG2(r, nTEMP6, 0)<0;4,4>:w      REG2(r, nTEMP6, 1)<0;4,4>:w      
    add (4)   REG2(r, nTEMP7, 0)<4>:w      REG2(r, nTEMP7, 0)<0;4,4>:w      REG2(r, nTEMP7, 1)<0;4,4>:w      
    add (4)   REG2(r, nTEMP4, 0)<4>:w      REG2(r, nTEMP4, 0)<0;4,4>:w      REG2(r, nTEMP4, 2)<0;4,4>:w      
    add (4)   REG2(r, nTEMP5, 0)<4>:w      REG2(r, nTEMP5, 0)<0;4,4>:w      REG2(r, nTEMP5, 2)<0;4,4>:w      
    add (4)   REG2(r, nTEMP6, 0)<4>:w      REG2(r, nTEMP6, 0)<0;4,4>:w      REG2(r, nTEMP6, 2)<0;4,4>:w      
    add (4)   REG2(r, nTEMP7, 0)<4>:w      REG2(r, nTEMP7, 0)<0;4,4>:w      REG2(r, nTEMP7, 2)<0;4,4>:w      

    // #### write U to the 1 row
    mov (4)  REG2(r, nTEMP8,  0)<1>:w    REG2(r, nTEMP4, 0)<0; 4, 4>:w
    mov (4)  REG2(r, nTEMP8,  4)<1>:w    REG2(r, nTEMP5, 0)<0; 4, 4>:w
    mov (4)  REG2(r, nTEMP8,  8)<1>:w    REG2(r, nTEMP6, 0)<0; 4, 4>:w
    mov (4)  REG2(r, nTEMP8, 12)<1>:w    REG2(r, nTEMP7, 0)<0; 4, 4>:w
    add (16) REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP8, 0)<0; 16, 1>:w    0x8080:uw
    mov (16) REG2(r, nTEMP8,  0)<1>:ub    REG2(r, nTEMP8, 1)<0; 16, 2>:ub
    mov (16) uwDEST_U(%1)<1>  REG2(r,nTEMP8, 0)<0;16,1>:ub

    // ###### do one row for V
    // #### mul and add
    mul (16)  REG2(r, nTEMP4, 0)<1>:w      r[SRC_RGBA_OFFSET_1, %1*32 +  0]<0; 16,1>:ub        bRGB_to_V_Coef_Fix<0;4,1>:b
    mul (16)  REG2(r, nTEMP5, 0)<1>:w      r[SRC_RGBA_OFFSET_1, %1*32 + 16]<0; 16,1>:ub        bRGB_to_V_Coef_Fix<0;4,1>:b
    mul (16)  REG2(r, nTEMP6, 0)<1>:w      r[SRC_RGBA_OFFSET_2, %1*32 +  0]<0; 16,1>:ub        bRGB_to_V_Coef_Fix<0;4,1>:b
    mul (16)  REG2(r, nTEMP7, 0)<1>:w      r[SRC_RGBA_OFFSET_2, %1*32 + 16]<0; 16,1>:ub        bRGB_to_V_Coef_Fix<0;4,1>:b
    
    add (4)   REG2(r, nTEMP4, 0)<4>:w      REG2(r, nTEMP4, 0)<0;4,4>:w      REG2(r, nTEMP4, 1)<0;4,4>:w      
    add (4)   REG2(r, nTEMP5, 0)<4>:w      REG2(r, nTEMP5, 0)<0;4,4>:w      REG2(r, nTEMP5, 1)<0;4,4>:w      
    add (4)   REG2(r, nTEMP6, 0)<4>:w      REG2(r, nTEMP6, 0)<0;4,4>:w      REG2(r, nTEMP6, 1)<0;4,4>:w      
    add (4)   REG2(r, nTEMP7, 0)<4>:w      REG2(r, nTEMP7, 0)<0;4,4>:w      REG2(r, nTEMP7, 1)<0;4,4>:w      
    add (4)   REG2(r, nTEMP4, 0)<4>:w      REG2(r, nTEMP4, 0)<0;4,4>:w      REG2(r, nTEMP4, 2)<0;4,4>:w      
    add (4)   REG2(r, nTEMP5, 0)<4>:w      REG2(r, nTEMP5, 0)<0;4,4>:w      REG2(r, nTEMP5, 2)<0;4,4>:w      
    add (4)   REG2(r, nTEMP6, 0)<4>:w      REG2(r, nTEMP6, 0)<0;4,4>:w      REG2(r, nTEMP6, 2)<0;4,4>:w      
    add (4)   REG2(r, nTEMP7, 0)<4>:w      REG2(r, nTEMP7, 0)<0;4,4>:w      REG2(r, nTEMP7, 2)<0;4,4>:w      

    // #### write V to the 1 row
    mov (4)  REG2(r, nTEMP8,  0)<1>:w    REG2(r, nTEMP4, 0)<0; 4, 4>:w
    mov (4)  REG2(r, nTEMP8,  4)<1>:w    REG2(r, nTEMP5, 0)<0; 4, 4>:w
    mov (4)  REG2(r, nTEMP8,  8)<1>:w    REG2(r, nTEMP6, 0)<0; 4, 4>:w
    mov (4)  REG2(r, nTEMP8, 12)<1>:w    REG2(r, nTEMP7, 0)<0; 4, 4>:w
    add (16) REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP8, 0)<0; 16, 1>:w    0x8080:uw
    mov (16) REG2(r, nTEMP8,  0)<1>:ub    REG2(r, nTEMP8, 1)<0; 16, 2>:ub
    mov (16) uwDEST_V(%1)<1>  REG2(r,nTEMP8, 0)<0;16,1>:ub
}

