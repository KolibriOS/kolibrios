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

// Module name: YUVX_Save_RGBX_Fix.asm
//----------------------------------------------------------------

#include "RGBX_Load_16x8.inc"

#if (0)
    #define nTEMP0          34        // transformation coefficient
    #define nTEMP1          35        // one row of R (first half register is used)
    #define nTEMP2          36        // one row of G (first half register is used)
    #define nTEMP3          37        // one row of B (first half register is used)
    #define nTEMP4          38        // mul and add
    #define nTEMP5          39        // mul and add
    #define nTEMP6          40        // mul and add
    #define nTEMP7          41        // mul and add
    #define nTEMP8          42        // sum of mul
    #define nTEMP10         44        
    #define nTEMP10         44        // split ub pixel to word width 1st quarter
    #define nTEMP12         46        // split ub pixel to word width 2nd quarter
    #define nTEMP14         48        // split ub pixel to word width 3rd quarter
    #define nTEMP16         50        // split ub pixel to word width 4th quarter
    #define nTEMP17         51
    #define nTEMP18         52
    
    #define nTEMP24         58        // temp using for repeat U/V in NV12_Load_8x4.asm
#endif

#define ONE_ROW_DEBUG                      0

#if (ONE_ROW_DEBUG)
    #define ROW_NUM                        0
    #define DBG_ROWNUM_BASE                1
    CHANNEL_2                              2
#else
    #define ROW_NUM                        %1
    $for(0; <nY_NUM_OF_ROWS; 1) {
#endif    
    // C = Y' - 16          D = U - 128         E = V - 128
    add (16)     REG2(r,nTEMP10,0)<1>:w           ubDEST_RGBX(0,ROW_NUM*64   )<0;16,1>           bYUV_OFF<0;4,1>:b
    add (16)     REG2(r,nTEMP12,0)<1>:w           ubDEST_RGBX(0,ROW_NUM*64+16)<0;16,1>           bYUV_OFF<0;4,1>:b
    add (16)     REG2(r,nTEMP14,0)<1>:w           ubDEST_RGBX(0,ROW_NUM*64+32)<0;16,1>           bYUV_OFF<0;4,1>:b
    add (16)     REG2(r,nTEMP16,0)<1>:w           ubDEST_RGBX(0,ROW_NUM*64+48)<0;16,1>           bYUV_OFF<0;4,1>:b

#if (ONE_ROW_DEBUG)
    mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE)*64   )<1>  REG2(r,nTEMP10, 0)<0;16,2>:ub
    mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE)*64+16)<1>  REG2(r,nTEMP12, 0)<0;16,2>:ub
    mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE)*64+32)<1>  REG2(r,nTEMP14, 0)<0;16,2>:ub
    mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE)*64+48)<1>  REG2(r,nTEMP16, 0)<0;16,2>:ub
#endif
    
    // |Y|U|V|X|==>|R|G|B|X|  
    // ###### do one row for R
    // #### mul and add
    mul.sat (16)  REG2(r, nTEMP4, 0)<1>:w      REG2(r,nTEMP10,0)<0;16,1>:w        wYUV_to_RGB_CH2_Coef_Fix<0;4,1>:w
    mul.sat (16)  REG2(r, nTEMP5, 0)<1>:w      REG2(r,nTEMP12,0)<0;16,1>:w        wYUV_to_RGB_CH2_Coef_Fix<0;4,1>:w
    mul.sat (16)  REG2(r, nTEMP6, 0)<1>:w      REG2(r,nTEMP14,0)<0;16,1>:w        wYUV_to_RGB_CH2_Coef_Fix<0;4,1>:w
    mul.sat (16)  REG2(r, nTEMP7, 0)<1>:w      REG2(r,nTEMP16,0)<0;16,1>:w        wYUV_to_RGB_CH2_Coef_Fix<0;4,1>:w

  #if (ONE_ROW_DEBUG)
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+1)*64+CHANNEL_2   )<4>  bYUV_to_RGB_CH2_Coef_Fix<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+1)*64+CHANNEL_2+32)<4>  bYUV_to_RGB_CH2_Coef_Fix<0;8,1>:ub

    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+2)*64+CHANNEL_2   )<4>  REG2(r,nTEMP4, 0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+2)*64+CHANNEL_2+32)<4>  REG2(r,nTEMP4, 8)<0;8,1>:ub
  #endif

    add.sat (4)   REG2(r, nTEMP4, 0)<4>:uw      REG2(r, nTEMP4, 0)<0;4,4>:w      REG2(r, nTEMP4, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP5, 0)<4>:uw      REG2(r, nTEMP5, 0)<0;4,4>:w      REG2(r, nTEMP5, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP6, 0)<4>:uw      REG2(r, nTEMP6, 0)<0;4,4>:w      REG2(r, nTEMP6, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP7, 0)<4>:uw      REG2(r, nTEMP7, 0)<0;4,4>:w      REG2(r, nTEMP7, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP4, 0)<4>:uw      REG2(r, nTEMP4, 0)<0;4,4>:uw      REG2(r, nTEMP4, 2)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP5, 0)<4>:uw      REG2(r, nTEMP5, 0)<0;4,4>:uw      REG2(r, nTEMP5, 2)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP6, 0)<4>:uw      REG2(r, nTEMP6, 0)<0;4,4>:uw      REG2(r, nTEMP6, 2)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP7, 0)<4>:uw      REG2(r, nTEMP7, 0)<0;4,4>:uw      REG2(r, nTEMP7, 2)<0;4,4>:w      

    // ####  write one row of R to rnTEMP1
    mov (4)  REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP4, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8,  4)<1>:uw    REG2(r, nTEMP5, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8,  8)<1>:uw    REG2(r, nTEMP6, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8, 12)<1>:uw    REG2(r, nTEMP7, 0)<0; 4, 4>:uw

  #if (ONE_ROW_DEBUG)
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+3)*64+CHANNEL_2   )<4>  REG2(r,nTEMP8, 0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+3)*64+CHANNEL_2+32)<4>  REG2(r,nTEMP8, 8)<0;8,1>:ub
  #endif    

    add.sat (16) REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP8, 0)<0; 16, 1>:uw    0x80:uw 
    shl.sat (16) REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP8, 0)<0; 16, 1>:uw    1:w
    mov (16) REG2(r, nTEMP1,  0)<1>:ub   REG2(r, nTEMP8, 1)<0; 16, 2>:ub
    
  #if (ONE_ROW_DEBUG)
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+4)*64+CHANNEL_2   )<4>  REG2(r,nTEMP8, 0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+4)*64+CHANNEL_2+32)<4>  REG2(r,nTEMP8, 8)<0;8,1>:ub
  #endif    
    // ###### do one row for G
    // #### mul and add
    mul.sat (16)  REG2(r, nTEMP4, 0)<1>:w      REG2(r,nTEMP10,0)<0;16,1>:w        wYUV_to_RGB_CH1_Coef_Fix<0;4,1>:w
    mul.sat (16)  REG2(r, nTEMP5, 0)<1>:w      REG2(r,nTEMP12,0)<0;16,1>:w        wYUV_to_RGB_CH1_Coef_Fix<0;4,1>:w
    mul.sat (16)  REG2(r, nTEMP6, 0)<1>:w      REG2(r,nTEMP14,0)<0;16,1>:w        wYUV_to_RGB_CH1_Coef_Fix<0;4,1>:w
    mul.sat (16)  REG2(r, nTEMP7, 0)<1>:w      REG2(r,nTEMP16,0)<0;16,1>:w        wYUV_to_RGB_CH1_Coef_Fix<0;4,1>:w

    add.sat (4)   REG2(r, nTEMP4, 0)<4>:uw      REG2(r, nTEMP4, 0)<0;4,4>:w      REG2(r, nTEMP4, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP5, 0)<4>:uw      REG2(r, nTEMP5, 0)<0;4,4>:w      REG2(r, nTEMP5, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP6, 0)<4>:uw      REG2(r, nTEMP6, 0)<0;4,4>:w      REG2(r, nTEMP6, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP7, 0)<4>:uw      REG2(r, nTEMP7, 0)<0;4,4>:w      REG2(r, nTEMP7, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP4, 0)<4>:uw      REG2(r, nTEMP4, 0)<0;4,4>:uw      REG2(r, nTEMP4, 2)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP5, 0)<4>:uw      REG2(r, nTEMP5, 0)<0;4,4>:uw      REG2(r, nTEMP5, 2)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP6, 0)<4>:uw      REG2(r, nTEMP6, 0)<0;4,4>:uw      REG2(r, nTEMP6, 2)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP7, 0)<4>:uw      REG2(r, nTEMP7, 0)<0;4,4>:uw      REG2(r, nTEMP7, 2)<0;4,4>:w      

    // ####  write one row of G to rnTEMP2
    mov (4)  REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP4, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8,  4)<1>:uw    REG2(r, nTEMP5, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8,  8)<1>:uw    REG2(r, nTEMP6, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8, 12)<1>:uw    REG2(r, nTEMP7, 0)<0; 4, 4>:uw
    
    add (16) REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP8, 0)<0; 16, 1>:uw    0x80:uw // saturation
    shl.sat (16) REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP8, 0)<0; 16, 1>:uw    1:w
    mov (16) REG2(r, nTEMP2,  0)<1>:ub   REG2(r, nTEMP8, 1)<0; 16, 2>:ub

    // ###### do one row for B
    // #### mul and add
    mul.sat (16)  REG2(r, nTEMP4, 0)<1>:w      REG2(r,nTEMP10,0)<0;16,1>:w        wYUV_to_RGB_CH0_Coef_Fix<0;4,1>:w
    mul.sat (16)  REG2(r, nTEMP5, 0)<1>:w      REG2(r,nTEMP12,0)<0;16,1>:w        wYUV_to_RGB_CH0_Coef_Fix<0;4,1>:w
    mul.sat (16)  REG2(r, nTEMP6, 0)<1>:w      REG2(r,nTEMP14,0)<0;16,1>:w        wYUV_to_RGB_CH0_Coef_Fix<0;4,1>:w
    mul.sat (16)  REG2(r, nTEMP7, 0)<1>:w      REG2(r,nTEMP16,0)<0;16,1>:w        wYUV_to_RGB_CH0_Coef_Fix<0;4,1>:w

    // I had reduced the following add because U coef is zero for B; but in order to support BGR/RGB at the same time, I have to add it back.
    add.sat (4)   REG2(r, nTEMP4, 0)<4>:uw      REG2(r, nTEMP4, 0)<0;4,4>:w      REG2(r, nTEMP4, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP5, 0)<4>:uw      REG2(r, nTEMP5, 0)<0;4,4>:w      REG2(r, nTEMP5, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP6, 0)<4>:uw      REG2(r, nTEMP6, 0)<0;4,4>:w      REG2(r, nTEMP6, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP7, 0)<4>:uw      REG2(r, nTEMP7, 0)<0;4,4>:w      REG2(r, nTEMP7, 1)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP4, 0)<4>:uw      REG2(r, nTEMP4, 0)<0;4,4>:uw      REG2(r, nTEMP4, 2)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP5, 0)<4>:uw      REG2(r, nTEMP5, 0)<0;4,4>:uw      REG2(r, nTEMP5, 2)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP6, 0)<4>:uw      REG2(r, nTEMP6, 0)<0;4,4>:uw      REG2(r, nTEMP6, 2)<0;4,4>:w      
    add.sat (4)   REG2(r, nTEMP7, 0)<4>:uw      REG2(r, nTEMP7, 0)<0;4,4>:uw      REG2(r, nTEMP7, 2)<0;4,4>:w      

    // ####  write one row of B to rnTEMP3
    mov (4)  REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP4, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8,  4)<1>:uw    REG2(r, nTEMP5, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8,  8)<1>:uw    REG2(r, nTEMP6, 0)<0; 4, 4>:uw
    mov (4)  REG2(r, nTEMP8, 12)<1>:uw    REG2(r, nTEMP7, 0)<0; 4, 4>:uw

    add.sat (16) REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP8, 0)<0; 16, 1>:uw    0x80:uw // saturation
    shl.sat (16) REG2(r, nTEMP8,  0)<1>:uw    REG2(r, nTEMP8, 0)<0; 16, 1>:uw    1:w
    mov (16) REG2(r, nTEMP3,  0)<1>:ub   REG2(r, nTEMP8, 1)<0; 16, 2>:ub

    // B
    mov (8) ubDEST_RGBX(0,ROW_NUM*64   )<4>  REG2(r,nTEMP3, 0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,ROW_NUM*64+32)<4>  REG2(r,nTEMP3, 8)<0;8,1>:ub
    // G
    mov (8) ubDEST_RGBX(0,ROW_NUM*64+1   )<4>  REG2(r,nTEMP2, 0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,ROW_NUM*64+1+32)<4>  REG2(r,nTEMP2, 8)<0;8,1>:ub
    // R
    mov (8) ubDEST_RGBX(0,ROW_NUM*64+2   )<4>  REG2(r,nTEMP1, 0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,ROW_NUM*64+2+32)<4>  REG2(r,nTEMP1, 8)<0;8,1>:ub
#if (!ONE_ROW_DEBUG)    
    }
#endif
