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

