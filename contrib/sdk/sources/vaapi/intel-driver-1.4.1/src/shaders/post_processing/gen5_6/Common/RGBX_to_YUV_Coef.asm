/*
 * All Video Processing kernels 
 * Copyright © <2010>, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Eclipse Public License (EPL), version 1.0.  The full text of the EPL is at
 * http://www.opensource.org/licenses/eclipse-1.0.php.
 *
 * Authors:
 *    Halley Zhao <halley.zhao@intel.com>
 */

// Module name: RGB_to_YUV_Coef.asm
//----------------------------------------------------------------

// is src surface |R|G|B|X| layout? otherwise it is |B|G|R|X|
and.nz.f0.1 (1) dNULLREG     r1.1:ud         0xFF000000:ud      
#ifdef FIX_POINT_CONVERSION
        // Y = ( (  66 * R + 129 * G +  25 * B + 128 ) >> 8) +  16
    (-f0.1) mov (1) REG2(r, nTEMP0, 0):ud       0x00428119:ud      // used as unsigned byte
    ( f0.1) mov (1) REG2(r, nTEMP0, 0):ud       0x00198142:ud      // used as unsigned byte
        // U = ( ( -38 * R -  74 * G + 112 * B + 128 ) >> 8) + 128
    (-f0.1) mov (1) REG2(r, nTEMP0, 1):ud       0x00DAB670:ud      // used as signed byte
    ( f0.1) mov (1) REG2(r, nTEMP0, 1):ud       0x0070B6DA:ud      // used as signed byte
        // V = ( ( 112 * R -  94 * G -  18 * B + 128 ) >> 8) + 128
    (-f0.1) mov (1) REG2(r, nTEMP0, 2):ud       0x0070A2EEud      // used as signed byte
    ( f0.1) mov (1) REG2(r, nTEMP0, 2):ud       0x00EEA270ud      // used as signed byte

    #define ubRGB_to_Y_Coef_Fix        REG2(r, nTEMP0, 0)
    #define  bRGB_to_U_Coef_Fix        REG2(r, nTEMP0, 4)
    #define  bRGB_to_V_Coef_Fix        REG2(r, nTEMP0, 8)
#else
        // Y =  0.299R + 0.587G + 0.114B
    (-f0.1) mov (1) REG2(r, nTEMP8, 0):f       0.114f       // B coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 2):f       0.114f       // R coef
            mov (1) REG2(r, nTEMP8, 1):f       0.587f       // G coef
    (-f0.1) mov (1) REG2(r, nTEMP8, 2):f       0.299f       // R coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 0):f       0.299f       // B coef
            mov (1) REG2(r, nTEMP8, 3):f       0.000f       // A coef
    
    // Cb= -0.169R - 0.331G + 0.499B + 128
        // U = -0.147R - 0.289G + 0.436B + 128
    (-f0.1) mov (1) REG2(r, nTEMP8, 4):f       0.436f       // B coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 6):f       0.436f       // R coef
            mov (1) REG2(r, nTEMP8, 5):f      -0.289f       // G coef
    (-f0.1) mov (1) REG2(r, nTEMP8, 6):f      -0.147f       // R coef
    ( f0.1) mov (1) REG2(r, nTEMP8, 4):f      -0.147f       // B coef
            mov (1) REG2(r, nTEMP8, 7):f       0.000f       // A coef

        // Cr= 0.499R - 0.418G - 0.0813B+ 128
        // V = 0.615R - 0.515G - 0.100B + 128
    (-f0.1) mov (1) REG2(r, nTEMP10, 0):f      -0.100f       // B coef
    ( f0.1) mov (1) REG2(r, nTEMP10, 2):f      -0.100f       // R coef
            mov (1) REG2(r, nTEMP10, 1):f      -0.515f       // G coef
    (-f0.1) mov (1) REG2(r, nTEMP10, 2):f       0.615f       // R coef
    ( f0.1) mov (1) REG2(r, nTEMP10, 0):f       0.615f       // B coef
            mov (1) REG2(r, nTEMP10, 3):f       0.000f       // A coef

    #define fRGB_to_Y_Coef_Float        REG2(r, nTEMP8, 0)
    #define fRGB_to_U_Coef_Float        REG2(r, nTEMP8, 4)
    #define fRGB_to_V_Coef_Float        REG2(r, nTEMP10, 0)
    .declare fROW_BGRX       Base=REG(r,nTEMP0) ElementSize=4 SrcRegion=REGION(8,8) Type=f    // r nTEMP0 - r nTEMP7
#endif

