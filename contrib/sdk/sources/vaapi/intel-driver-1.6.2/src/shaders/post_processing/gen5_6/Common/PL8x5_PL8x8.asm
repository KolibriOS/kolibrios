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

// Module name: PL8x5_PL8x8.asm

#include "Expansion.inc"

//------------------------------- Vertical Upconversion ------------------------------
    avg.sat (8) uwDEST_U(0, 3*16+8)<1>   uwDEST_U(0, 3*8)    uwDEST_U(0, (1+3)*8)    // Optimization
    avg.sat (8) uwDEST_V(0, 3*16+8)<1>   uwDEST_V(0, 3*8)    uwDEST_V(0, (1+3)*8)    // Optimization

    $for(nUV_NUM_OF_ROWS/2-2; >-1; -1) {
        mov     (8) uwDEST_U(0, (1+%1)*16)<1>    uwDEST_U(0, (1+%1)*8)
        avg.sat (8) uwDEST_U(0, %1*16+8)<1>   uwDEST_U(0, %1*8)    uwDEST_U(0, (1+%1)*8)

        mov     (8) uwDEST_V(0, (1+%1)*16)<1>    uwDEST_V(0, (1+%1)*8)
        avg.sat (8) uwDEST_V(0, %1*16+8)<1>   uwDEST_V(0, %1*8)    uwDEST_V(0, (1+%1)*8)
    }

// End of PL8x5_PL8x8
