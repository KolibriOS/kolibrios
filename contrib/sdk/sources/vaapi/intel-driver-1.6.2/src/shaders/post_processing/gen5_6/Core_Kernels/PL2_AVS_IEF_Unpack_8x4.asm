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
        
    // Move first 8x8 words of Y to dest GRF at lower 8 words of each RGF.
    $for(0; <8/2; 1) {
        mov (8) uwDEST_Y(%1*2)<1>        ubAVS_RESPONSE(%1,1)<16;4,2>      // Copy high byte in a word
        mov (8) uwDEST_Y(%1*2+1)<1>      ubAVS_RESPONSE(%1,8+1)<16;4,2>    // Copy high byte in a word
    } 

    // Move 8x4 words of U to dest GRF  (Copy high byte in a word)
    mov (8) uwDEST_U(0)<1>            ubAVS_RESPONSE(4,1)<16;4,2>      
    mov (8) uwDEST_U(0,8)<1>          ubAVS_RESPONSE(5,1)<16;4,2>    
    mov (8) uwDEST_U(1)<1>            ubAVS_RESPONSE(8,1)<16;4,2>      
    mov (8) uwDEST_U(1,8)<1>          ubAVS_RESPONSE(9,1)<16;4,2>    

    // Move 8x4 words of V to dest GRF  
    mov (8) uwDEST_V(0)<1>            ubAVS_RESPONSE(6,1)<16;4,2>      
    mov (8) uwDEST_V(0,8)<1>          ubAVS_RESPONSE(7,1)<16;4,2>    
    mov (8) uwDEST_V(1)<1>            ubAVS_RESPONSE(10,1)<16;4,2>      
    mov (8) uwDEST_V(1,8)<1>          ubAVS_RESPONSE(11,1)<16;4,2>    

    // Move 2nd 8x8 words of Y to dest GRF at higher 8 words of each GRF.
    $for(0; <8/2; 1) {
        mov (8) uwDEST_Y(%1*2,8)<1>      ubAVS_RESPONSE_2(%1,1)<16;4,2>    // Copy high byte in a word
        mov (8) uwDEST_Y(%1*2+1,8)<1>    ubAVS_RESPONSE_2(%1,8+1)<16;4,2>  // Copy high byte in a word
    } 

//------------------------------------------------------------------------------

    // Re-define new # of lines
    #undef nUV_NUM_OF_ROWS
    #undef nY_NUM_OF_ROWS
   
    #define nY_NUM_OF_ROWS      8
    #define nUV_NUM_OF_ROWS     4

