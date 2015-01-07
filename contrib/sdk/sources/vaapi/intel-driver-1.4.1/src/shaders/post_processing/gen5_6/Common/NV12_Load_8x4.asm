/*
 * All Video Processing kernels 
 * Copyright © <2010>, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Eclipse Public License (EPL), version 1.0.  The full text of the EPL is at
 * http://www.opensource.org/licenses/eclipse-1.0.php.
 *
 */

// Module name: NV12_Load_8x4.asm
//----------------------------------------------------------------

#define  NV12_LOAD_8x4
#include "PL2_Load.inc"

// Load 16x8 planar Y ----------------------------------------------------------
    add  (2) rMSGSRC.0<1>:d     wORIX<2;2,1>:w    wSRC_H_ORI_OFFSET<2;2,1>:w       // Source Y Block origin
#if !defined(LOAD_UV_ONLY)
    mov  (1) rMSGSRC.2<1>:ud    nDPR_BLOCK_SIZE_Y:ud                               // Y block width and height (16x8)
    mov  (8) mMSGHDRY<1>:ud     rMSGSRC<8;8,1>:ud
    send (8) udSRC_Y(0)<1>      mMSGHDRY    udDUMMY_NULL    nDATAPORT_READ    nDPMR_MSGDSC+nDPR_MSG_SIZE_Y+nBI_CURRENT_SRC_Y:ud
#endif

// Load 8x4 planar U and V -----------------------------------------------------
    asr (1)  rMSGSRC.1<1>:d     rMSGSRC.1<0;1,0>:d       1:w   // U/V block origin should be half of Y's
    mov (1)  rMSGSRC.2<1>:ud    nDPR_BLOCK_SIZE_UV:ud          // U/V block width and height (16x4)
    mov  (8) mMSGHDRU<1>:ud     rMSGSRC<8;8,1>:ud
    send (8) udSRC_U(0)<1>      mMSGHDRU    udDUMMY_NULL    nDATAPORT_READ    nDPMR_MSGDSC+nDPR_MSG_SIZE_UV+nBI_CURRENT_SRC_UV:ud

// Convert to word-aligned format ----------------------------------------------
#if defined(FIX_POINT_CONVERSION) || defined(FLOAT_POINT_CONVERSION)
    // load NV12 and save it as packed AYUV to dst (64x8)

    $for (nY_NUM_OF_ROWS-1; >-1; -1) {
        // #### Y
        mov (8)  ubDEST_Y(0,%1*16*4)<4>             ubSRC_Y(0,%1*16)<0;8,1>
        mov (8)  ubDEST_Y(0,(%1*16+8)*4)<4>         ubSRC_Y(0,%1*16+8)<0;8,1>

        // #### U/V
        // error from compile: "Invalid horiz size 8", so I have to repeat UV first
        // mov (4)  ubDEST_Y(0,%1*16*4+1)<8>                   ubSRC_U(0,%1/2*16)<0;4,2>
        // mov (4)  ubDEST_Y(0,%1*16*4+1+32)<8>                ubSRC_U(0,%1/2*16+8)<0;4,2>
	
        // repeate U/V for each one
        mov (8)     REG2(r,nTEMP18,0)<2>:uw	            uwSRC_U(0,%1/2*8)<0;8,1>
        mov (8)     REG2(r,nTEMP18,1)<2>:uw	            uwSRC_U(0,%1/2*8)<0;8,1>
        
        // mov U/V to ubDEST
        mov (8)    ubDEST_Y(0,%1*16*4+1)<4>             REG2(r,nTEMP18,0)<0;8,2>:ub
        mov (8)    ubDEST_Y(0,%1*16*4+1+32)<4>          REG2(r,nTEMP18,16)<0;8,2>:ub

        mov (8)    ubDEST_Y(0,%1*16*4+2)<4>             REG2(r,nTEMP18,1)<0;8,2>:ub
        mov (8)    ubDEST_Y(0,%1*16*4+2+32)<4>          REG2(r,nTEMP18,17)<0;8,2>:ub
    }
#else
  #if !defined(LOAD_UV_ONLY)
    $for (nY_NUM_OF_ROWS-1; >-1; -1) {
        mov (16)  uwDEST_Y(0,%1*16)<1>      ubSRC_Y(0,%1*16)
    }
  #endif
    $for (nUV_NUM_OF_ROWS/2-1; >-1; -1) {
        // why "mov (16)"? should it be 8?
        mov (16)  uwDEST_U(0,%1*16)<1>      ubSRC_U(0,%1*32)<32;16,2>
        mov (16)  uwDEST_V(0,%1*16)<1>      ubSRC_U(0,%1*32+1)<32;16,2>
    }

#endif    

// End of NV12_Load_8x4
