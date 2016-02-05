/*
 * Add macroblock correction Y data blocks to predicted picture
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
        
// Module name: add_Error_16x16_Y.asm
//
// Add macroblock correction Y data blocks to predicted picture
//

//  Every line of predicted Y data is added to Y error data if CBP bit is set

    mov (1) PERROR_UD<1>:ud	0x10001*ERRBUF*GRFWIB+0x00100000:ud	// Pointers to first and second row of error block

    and.z.f0.1 (1)	NULLREG    REG_CBPCY	CBP_Y_MASK
    (f0.1) jmpi (1) End_add_Error_16x16_Y	// Skip all blocks

//  Block Y0
//
    $for(0,0; <8; 2,1) {
	add.sat (16)	DEC_Y(%1)<2>	r[PERROR,%2*GRFWIB]REGION(8,1):w	PRED_Y(%1)REGION(8,2) {Compr}
    }

//  Block Y1
//
    $for(0,0; <8; 2,1) {
	add.sat (16)	DEC_Y(%1,16)<2>		r[PERROR,%2*GRFWIB+0x80]REGION(8,1):w	PRED_Y(%1,16)REGION(8,2) {Compr}
    }

//  Block Y2
//
    $for(8,0; <16; 2,1) {
	add.sat (16)	DEC_Y(%1)<2>	r[PERROR,%2*GRFWIB+0x100]REGION(8,1):w	PRED_Y(%1)REGION(8,2) {Compr}
    }

//  Block Y3
//
    $for(8,0; <16; 2,1) {
	add.sat (16)	DEC_Y(%1,16)<2>		r[PERROR,%2*GRFWIB+0x180]REGION(8,1):w	PRED_Y(%1,16)REGION(8,2) {Compr}
    }

End_add_Error_16x16_Y:
    add (1) PERROR_UD<1>:ud	PERROR_UD:ud	0x01800180:ud	// Pointers to Y3 error block

//  End of add_Error_16x16_Y

