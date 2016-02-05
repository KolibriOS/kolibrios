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

//---------- PA_AVS_IEF_Unpack_8x8.asm ----------

// Yoni: In order to optimize unpacking, 3 methods are being checked:
//  1. AVS_ORIGINAL
//  2. AVS_ROUND_TO_8_BITS  
//  3. AVS_INDIRECT_ACCESS  
//
// Only 1 method should stay in the code 


//#define AVS_ROUND_TO_8_BITS
//#define AVS_INDIRECT_ACCESS


    // Move first 8x8 words of Y to dest GRF
    mov (8)  uwDEST_Y(0)<1>     ubAVS_RESPONSE(2,1)<16;4,2>                 
    mov (8)  uwDEST_Y(1)<1>     ubAVS_RESPONSE(2,8+1)<16;4,2>               
    mov (8)  uwDEST_Y(2)<1>     ubAVS_RESPONSE(3,1)<16;4,2>                 
    mov (8)  uwDEST_Y(3)<1>     ubAVS_RESPONSE(3,8+1)<16;4,2>               
    mov (8)  uwDEST_Y(4)<1>     ubAVS_RESPONSE(8,1)<16;4,2>                 
    mov (8)  uwDEST_Y(5)<1>     ubAVS_RESPONSE(8,8+1)<16;4,2>               
    mov (8)  uwDEST_Y(6)<1>     ubAVS_RESPONSE(9,1)<16;4,2>                 
    mov (8)  uwDEST_Y(7)<1>     ubAVS_RESPONSE(9,8+1)<16;4,2>               

    // Move first 4x8 words of V to dest GRF  
    mov (4) uwDEST_V(0)<1>      ubAVS_RESPONSE(0,1)<16;2,4>                 
    mov (4) uwDEST_V(0,8)<1>    ubAVS_RESPONSE(0,8+1)<16;2,4>               
    mov (4) uwDEST_V(1)<1>      ubAVS_RESPONSE(1,1)<16;2,4>                 
    mov (4) uwDEST_V(1,8)<1>    ubAVS_RESPONSE(1,8+1)<16;2,4>               
    mov (4) uwDEST_V(2)<1>      ubAVS_RESPONSE(6,1)<16;2,4>                 
    mov (4) uwDEST_V(2,8)<1>    ubAVS_RESPONSE(6,8+1)<16;2,4>               
    mov (4) uwDEST_V(3)<1>      ubAVS_RESPONSE(7,1)<16;2,4>                 
    mov (4) uwDEST_V(3,8)<1>    ubAVS_RESPONSE(7,8+1)<16;2,4>               

    // Move first 4x8 words of U to dest GRF        
    mov (4) uwDEST_U(0)<1>      ubAVS_RESPONSE(4,1)<16;2,4>           
    mov (4) uwDEST_U(0,8)<1>    ubAVS_RESPONSE(4,8+1)<16;2,4>                 
    mov (4) uwDEST_U(1)<1>      ubAVS_RESPONSE(5,1)<16;2,4>           
    mov (4) uwDEST_U(1,8)<1>    ubAVS_RESPONSE(5,8+1)<16;2,4>                 
    mov (4) uwDEST_U(2)<1>      ubAVS_RESPONSE(10,1)<16;2,4>          
    mov (4) uwDEST_U(2,8)<1>    ubAVS_RESPONSE(10,8+1)<16;2,4>                
    mov (4) uwDEST_U(3)<1>      ubAVS_RESPONSE(11,1)<16;2,4>          
    mov (4) uwDEST_U(3,8)<1>    ubAVS_RESPONSE(11,8+1)<16;2,4>                

    // Move second 8x8 words of Y to dest GRF
    mov (8) uwDEST_Y(0,8)<1>    ubAVS_RESPONSE_2(2,1)<16;4,2>    
    mov (8) uwDEST_Y(1,8)<1>    ubAVS_RESPONSE_2(2,8+1)<16;4,2>
    mov (8) uwDEST_Y(2,8)<1>    ubAVS_RESPONSE_2(3,1)<16;4,2>     
    mov (8) uwDEST_Y(3,8)<1>    ubAVS_RESPONSE_2(3,8+1)<16;4,2>
    mov (8) uwDEST_Y(4,8)<1>    ubAVS_RESPONSE_2(8,1)<16;4,2>  
    mov (8) uwDEST_Y(5,8)<1>    ubAVS_RESPONSE_2(8,8+1)<16;4,2>
    mov (8) uwDEST_Y(6,8)<1>    ubAVS_RESPONSE_2(9,1)<16;4,2>     
    mov (8) uwDEST_Y(7,8)<1>    ubAVS_RESPONSE_2(9,8+1)<16;4,2>

    // Move second 4x8 words of V to dest GRF        
    mov (4) uwDEST_V(0,4)<1>    ubAVS_RESPONSE_2(0,1)<16;2,4>           
    mov (4) uwDEST_V(0,12)<1>   ubAVS_RESPONSE_2(0,8+1)<16;2,4>                 
    mov (4) uwDEST_V(1,4)<1>    ubAVS_RESPONSE_2(1,1)<16;2,4>           
    mov (4) uwDEST_V(1,12)<1>   ubAVS_RESPONSE_2(1,8+1)<16;2,4>                 
    mov (4) uwDEST_V(2,4)<1>    ubAVS_RESPONSE_2(6,1)<16;2,4>           
    mov (4) uwDEST_V(2,12)<1>   ubAVS_RESPONSE_2(6,8+1)<16;2,4>                 
    mov (4) uwDEST_V(3,4)<1>    ubAVS_RESPONSE_2(7,1)<16;2,4>           
    mov (4) uwDEST_V(3,12)<1>   ubAVS_RESPONSE_2(7,8+1)<16;2,4>                 

    // Move second 4x8 words of U to dest GRF        
    mov (4) uwDEST_U(0,4)<1>    ubAVS_RESPONSE_2(4,1)<16;2,4>             
    mov (4) uwDEST_U(0,12)<1>   ubAVS_RESPONSE_2(4,8+1)<16;2,4>           
    mov (4) uwDEST_U(1,4)<1>    ubAVS_RESPONSE_2(5,1)<16;2,4>             
    mov (4) uwDEST_U(1,12)<1>   ubAVS_RESPONSE_2(5,8+1)<16;2,4>           
    mov (4) uwDEST_U(2,4)<1>    ubAVS_RESPONSE_2(10,1)<16;2,4>            
    mov (4) uwDEST_U(2,12)<1>   ubAVS_RESPONSE_2(10,8+1)<16;2,4>          
    mov (4) uwDEST_U(3,4)<1>    ubAVS_RESPONSE_2(11,1)<16;2,4>            
    mov (4) uwDEST_U(3,12)<1>   ubAVS_RESPONSE_2(11,8+1)<16;2,4>          

//------------------------------------------------------------------------------

       // Re-define new number of lines
       #undef nUV_NUM_OF_ROWS
       #undef nY_NUM_OF_ROWS
       
       #define nY_NUM_OF_ROWS      8
       #define nUV_NUM_OF_ROWS     8

