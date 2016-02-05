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

// Module name: RGBX_Save_YUV_Float.asm
//----------------------------------------------------------------

#include "RGBX_Load_16x8.inc"

#if (0)
    // 8 grf reg for one row of pixel (2 pixel per grf)
    #define nTEMP0          34
    #define nTEMP1          35
    #define nTEMP2          36
    #define nTEMP3          37
    #define nTEMP4          38
    #define nTEMP5          39
    #define nTEMP6          40
    #define nTEMP7          41
    
    #define nTEMP8          42        // transformation coefficient
    #define nTEMP10         44        // transformation coefficient
    
    #define nTEMP12         46        // save Y/U/V in ub format
    #define nTEMP14         48        // save YUV in ud format
    #define nTEMP16         50        // dp4 result
    #define nTEMP17         51        
    #define nTEMP18         52
    
    #define nTEMP24         58
#endif

$for(0; <nY_NUM_OF_ROWS; 1) {
    // BGRX | B | G | R | X |
    // ###### save one row of pixel to temp grf with float format (required by dp4)
    // mov (8) doesn't work, puzzle
    mov (4) REG(r, nTEMP0)<1>:f       r[SRC_RGBA_OFFSET_1,%1*32 +  0]<4,1>:ub
    mov (4) REG(r, nTEMP1)<1>:f       r[SRC_RGBA_OFFSET_1,%1*32 +  8]<4,1>:ub
    mov (4) REG(r, nTEMP2)<1>:f       r[SRC_RGBA_OFFSET_1,%1*32 + 16]<4,1>:ub
    mov (4) REG(r, nTEMP3)<1>:f       r[SRC_RGBA_OFFSET_1,%1*32 + 24]<4,1>:ub
    mov (4) REG(r, nTEMP4)<1>:f       r[SRC_RGBA_OFFSET_2,%1*32 +  0]<4,1>:ub
    mov (4) REG(r, nTEMP5)<1>:f       r[SRC_RGBA_OFFSET_2,%1*32 +  8]<4,1>:ub
    mov (4) REG(r, nTEMP6)<1>:f       r[SRC_RGBA_OFFSET_2,%1*32 + 16]<4,1>:ub
    mov (4) REG(r, nTEMP7)<1>:f       r[SRC_RGBA_OFFSET_2,%1*32 + 24]<4,1>:ub
    mov (4) REG2(r, nTEMP0, 4)<1>:f   r[SRC_RGBA_OFFSET_1,%1*32 +  4]<4,1>:ub
    mov (4) REG2(r, nTEMP1, 4)<1>:f   r[SRC_RGBA_OFFSET_1,%1*32 + 12]<4,1>:ub
    mov (4) REG2(r, nTEMP2, 4)<1>:f   r[SRC_RGBA_OFFSET_1,%1*32 + 20]<4,1>:ub
    mov (4) REG2(r, nTEMP3, 4)<1>:f   r[SRC_RGBA_OFFSET_1,%1*32 + 28]<4,1>:ub
    mov (4) REG2(r, nTEMP4, 4)<1>:f   r[SRC_RGBA_OFFSET_2,%1*32 +  4]<4,1>:ub
    mov (4) REG2(r, nTEMP5, 4)<1>:f   r[SRC_RGBA_OFFSET_2,%1*32 + 12]<4,1>:ub
    mov (4) REG2(r, nTEMP6, 4)<1>:f   r[SRC_RGBA_OFFSET_2,%1*32 + 20]<4,1>:ub
    mov (4) REG2(r, nTEMP7, 4)<1>:f   r[SRC_RGBA_OFFSET_2,%1*32 + 24]<4,1>:ub

    // ###### do one row for Y
    // ##### dp4(nTEMP16) and save result to uw format(nTEMP12)
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f      fROW_BGRX(0, 0)<0;8,1>       fRGB_to_Y_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  0)<1>:ub     REG2(r, nTEMP14, 0)<0;2,4>:ub
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f      fROW_BGRX(1, 0)<0;8,1>       fRGB_to_Y_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  2)<1>:ub     REG2(r, nTEMP14, 0)<0;2,4>:ub
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f      fROW_BGRX(2, 0)<0;8,1>       fRGB_to_Y_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  4)<1>:ub     REG2(r, nTEMP14, 0)<0;2,4>:ub
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f      fROW_BGRX(3, 0)<0;8,1>       fRGB_to_Y_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  6)<1>:ub     REG2(r, nTEMP14, 0)<0;2,4>:ub
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f      fROW_BGRX(4, 0)<0;8,1>       fRGB_to_Y_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  8)<1>:ub     REG2(r, nTEMP14, 0)<0;2,4>:ub
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f      fROW_BGRX(5, 0)<0;8,1>       fRGB_to_Y_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 10)<1>:ub     REG2(r, nTEMP14, 0)<0;2,4>:ub
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f      fROW_BGRX(6, 0)<0;8,1>       fRGB_to_Y_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 12)<1>:ub     REG2(r, nTEMP14, 0)<0;2,4>:ub
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f      fROW_BGRX(7, 0)<0;8,1>       fRGB_to_Y_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 14)<1>:ub     REG2(r, nTEMP14, 0)<0;2,4>:ub

    // ####  write Y to the 1 row
    mov (16) uwDEST_Y(%1)<1>  REG2(r,nTEMP12,  0)<0;16,1>:ub
    
    // ###### do one row for U
    // ##### dp4(nTEMP16) and save result to uw format(nTEMP12)
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(0, 0)<0;8,1>        fRGB_to_U_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  0)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(1, 0)<0;8,1>        fRGB_to_U_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  2)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(2, 0)<0;8,1>        fRGB_to_U_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  4)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(3, 0)<0;8,1>        fRGB_to_U_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  6)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(4, 0)<0;8,1>        fRGB_to_U_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  8)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(5, 0)<0;8,1>        fRGB_to_U_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 10)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(6, 0)<0;8,1>        fRGB_to_U_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 12)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(7, 0)<0;8,1>        fRGB_to_U_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 14)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    add (16) REG2(r, nTEMP12,  0)<1>:w     REG2(r, nTEMP12, 0)<0;16,1>:w   128:w
    // ####  write U to the 1 row
    mov (16) uwDEST_U(%1)<1>  REG2(r,nTEMP12,  0)<0;16,2>:ub
    
    // ###### do one row for V
    // ##### dp4(nTEMP16) and save result to uw format(nTEMP12)
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(0, 0)<0;8,1>        fRGB_to_V_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  0)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(1, 0)<0;8,1>        fRGB_to_V_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  2)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(2, 0)<0;8,1>        fRGB_to_V_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  4)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(3, 0)<0;8,1>        fRGB_to_V_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  6)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(4, 0)<0;8,1>        fRGB_to_V_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  8)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(5, 0)<0;8,1>        fRGB_to_V_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 10)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(6, 0)<0;8,1>        fRGB_to_V_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 12)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f     fROW_BGRX(7, 0)<0;8,1>        fRGB_to_V_Coef_Float<0;4,1>:f 
    mov (2)  REG2(r, nTEMP14,  0)<1>:d     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 14)<1>:w     REG2(r, nTEMP14, 0)<0;2,2>:w
    add (16) REG2(r, nTEMP12,  0)<1>:w     REG2(r, nTEMP12, 0)<0;16,1>:w   128:w

    // ####  write V to the 1 row
    mov (16) uwDEST_V(%1)<1>  REG2(r,nTEMP12,  0)<0;16,2>:ub
}
