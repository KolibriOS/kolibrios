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

// Module name: YUVX_Save_RGBX_Float.asm
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
    #define nTEMP18         52       // temp used for repeat U/V in NV12_Load_8x4.asm
    
    #define nTEMP24         58       // it is not safe to use in my case. I try to use it for repeat U/V in NV12_Load_8x4.asm, Y data is taint in row 4/5
#endif

#define ONE_ROW_DEBUG                      0

#if (ONE_ROW_DEBUG)
    // if you want to debug a row which is not the first one, try the following:
    // 1. define ROW_NUM_READ to the row you want to debug
    // 2. ROW_NUM_WRITE can be same to DBG_ROWNUM_READ to overwrite original YUV data, or define it to a new row
    // 3. change (DBG_ROWNUM_BASE+?)=ROW_NUM_READ or ROW_NUM_WRITE to DBG_ROWNUM_0, to not conflict with others
    #define ROW_NUM_READ                   0
    #define ROW_NUM_WRITE                  0
    #define DBG_ROWNUM_BASE                1
    #define DBG_ROWNUM_0                   0
#else
    #define ROW_NUM_READ                   %1
    #define ROW_NUM_WRITE                  %1
    $for(0; <nY_NUM_OF_ROWS; 1) {
#endif    
    // YUVX | Y | U | V | X |
    // XRGB | B | G | R | X |
    // ###### save one row of pixel to temp grf with float format (required by dp4)
    // C = Y' - 16          D = U - 128         E = V - 128

    // the follow sentence doesn't work, I have to split it into two step
    // add (4) REG(r, nTEMP0)<1>:f       r[SRC_RGBA_OFFSET_1,ROW_NUM_READ*32 +  0]<4,1>:ub           REG2(r, nTEMP10,  16)<0;4,1>:b

    add (16)     REG2(r,nTEMP12,0)<1>:w           ubDEST_RGBX(0,ROW_NUM_READ*64   )<0;16,1>           bYUV_OFF<0;4,1>:b
    add (16)     REG2(r,nTEMP14,0)<1>:w           ubDEST_RGBX(0,ROW_NUM_READ*64+16)<0;16,1>           bYUV_OFF<0;4,1>:b
    add (16)     REG2(r,nTEMP16,0)<1>:w           ubDEST_RGBX(0,ROW_NUM_READ*64+32)<0;16,1>           bYUV_OFF<0;4,1>:b
    add (16)     REG2(r,nTEMP17,0)<1>:w           ubDEST_RGBX(0,ROW_NUM_READ*64+48)<0;16,1>           bYUV_OFF<0;4,1>:b
    
    mov (8)      fROW_YUVA(0,0)<1>            REG2(r, nTEMP12, 0)<0;8,1>:w
    mov (8)      fROW_YUVA(1,0)<1>            REG2(r, nTEMP12, 8)<0;8,1>:w
    mov (8)      fROW_YUVA(2,0)<1>            REG2(r, nTEMP14, 0)<0;8,1>:w
    mov (8)      fROW_YUVA(3,0)<1>            REG2(r, nTEMP14, 8)<0;8,1>:w
    mov (8)      fROW_YUVA(4,0)<1>            REG2(r, nTEMP16, 0)<0;8,1>:w
    mov (8)      fROW_YUVA(5,0)<1>            REG2(r, nTEMP16, 8)<0;8,1>:w
    mov (8)      fROW_YUVA(6,0)<1>            REG2(r, nTEMP17, 0)<0;8,1>:w
    mov (8)      fROW_YUVA(7,0)<1>            REG2(r, nTEMP17, 8)<0;8,1>:w

  #if (ONE_ROW_DEBUG)
    mov.sat (8)  REG2(r, nTEMP14,  0)<1>:ud     fROW_YUVA(0,0)<0;8,1>:f
    mov (8)  REG2(r, nTEMP12,   0)<1>:ub        REG2(r, nTEMP14, 0)<0;8,4>:ub
    
        // write Y-16, U-128, V-128 to the 2nd row of RGB (convert float to int first, write whole ud): 1st half, 2 pixels
        mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE)*64   )<1>  REG2(r,nTEMP14,  0)<0;16,1>:ub
        mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE)*64+16)<1>  REG2(r,nTEMP14, 16)<0;16,1>:ub

    mov.sat (8)  REG2(r, nTEMP14,  0)<1>:ud     fROW_YUVA(1,0)<0;8,1>:f
    mov (8)  REG2(r, nTEMP12,   8)<1>:ub        REG2(r, nTEMP14, 0)<0;8,4>:ub
    
        // write Y-16, U-128, V-128 to the 2nd row of RGB (convert float to int first, write whole ud): 2nd half, 2 pixels
        mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE)*64+32)<1>  REG2(r,nTEMP14,  0)<0;16,1>:ub
        mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE)*64+48)<1>  REG2(r,nTEMP14, 16)<0;16,1>:ub

    mov.sat (8)  REG2(r, nTEMP14,  0)<1>:ud     fROW_YUVA(2,0)<0;8,1>:f
    mov (8)  REG2(r, nTEMP12,  16)<1>:ub        REG2(r, nTEMP14, 0)<0;8,4>:ub
    
    mov.sat (8)  REG2(r, nTEMP14,  0)<1>:ud     fROW_YUVA(3,0)<0;8,1>:f
    mov (8)  REG2(r, nTEMP12,  24)<1>:ub        REG2(r, nTEMP14, 0)<0;8,4>:ub

        // write Y-16, U-128, V-128 to the 3rd row of RGB (convert float to int first, only LSB is used): 1st half, 8 pixels
        mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+1)*64   )<1>  REG2(r,nTEMP12,  0)<0;16,1>:ub
        mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+1)*64+16)<1>  REG2(r,nTEMP12, 16)<0;16,1>:ub
    
    mov.sat (8)  REG2(r, nTEMP14,  0)<1>:ud     fROW_YUVA(4,0)<0;8,1>:f
    mov (8)  REG2(r, nTEMP12,   0)<1>:ub        REG2(r, nTEMP14, 0)<0;8,4>:ub
    
    mov.sat (8)  REG2(r, nTEMP14,  0)<1>:ud     fROW_YUVA(5,0)<0;8,1>:f
    mov (8)  REG2(r, nTEMP12,   8)<1>:ub        REG2(r, nTEMP14, 0)<0;8,4>:ub

    mov.sat (8)  REG2(r, nTEMP14,  0)<1>:ud     fROW_YUVA(6,0)<0;8,1>:f
    mov (8)  REG2(r, nTEMP12,  16)<1>:ub        REG2(r, nTEMP14, 0)<0;8,4>:ub
    
    mov.sat (8)  REG2(r, nTEMP14,  0)<1>:ud     fROW_YUVA(7,0)<0;8,1>:f
    mov (8)  REG2(r, nTEMP12,  24)<1>:ub        REG2(r, nTEMP14, 0)<0;8,4>:ub

        // write Y-16, U-128, V-128 to the 3rd row of RGB (convert float to int first, only LSB is used): 2nd half, 8 pixels
        mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+1)*64+32)<1>  REG2(r,nTEMP12,  0)<0;16,1>:ub
        mov (16) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+1)*64+48)<1>  REG2(r,nTEMP12, 16)<0;16,1>:ub
  #endif

        // ######## do one row for Red ########
    #define fCOEF_REG  fYUV_to_RGB_CH2_Coef_Float
    #define CHANNEL   2
    // ##### dp4(nTEMP16) and save result to uw format(nTEMP12)
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(0, 0)<0;8,1>   fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  0)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub

  #if (ONE_ROW_DEBUG)
    // write dp4 (raw float) of 2 pixel to the 4/5th row
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+2)*64+CHANNEL   )<4>  REG2(r,nTEMP16,  0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+2)*64+CHANNEL+32)<4>  REG2(r,nTEMP16,  8)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+3)*64+CHANNEL   )<4>  REG2(r,nTEMP16, 16)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+3)*64+CHANNEL+32)<4>  REG2(r,nTEMP16, 24)<0;8,1>:ub

    // write dp4 (convert float to ud first, write whole ud) of 2 pixel to the 6/7th row
    mov (8)  REG2(r, nTEMP17,  0)<1>:d     REG2(r, nTEMP16, 0)<0;8,1>:f
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+4)*64+CHANNEL   )<4>  REG2(r,nTEMP17,  0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+4)*64+CHANNEL+32)<4>  REG2(r,nTEMP17,  8)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+5)*64+CHANNEL   )<4>  REG2(r,nTEMP17, 16)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+5)*64+CHANNEL+32)<4>  REG2(r,nTEMP17, 24)<0;8,1>:ub
  #endif    

    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(1, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  2)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(2, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  4)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(3, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  6)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(4, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  8)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(5, 0)<5;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 10)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(6, 0)<6;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 12)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(7, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 14)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub

    // ####  write this channel
    mov (8) ubDEST_RGBX(0,ROW_NUM_WRITE*64+CHANNEL   )<4>  REG2(r,nTEMP12, 0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,ROW_NUM_WRITE*64+CHANNEL+32)<4>  REG2(r,nTEMP12, 8)<0;8,1>:ub
    
        // ######## do one row for Green ########
    #define fCOEF_REG  fYUV_to_RGB_CH1_Coef_Float  // reg for green coefficient
    #define CHANNEL   1
    // ##### dp4(nTEMP16) and save result to uw format(nTEMP12)
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(0, 0)<0;8,1>   fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  0)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub

  #if (ONE_ROW_DEBUG)
    // write dp4 (raw float) of 2 pixel to the 4/5th row
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+2)*64+CHANNEL   )<4>  REG2(r,nTEMP16,  0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+2)*64+CHANNEL+32)<4>  REG2(r,nTEMP16,  8)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+3)*64+CHANNEL   )<4>  REG2(r,nTEMP16, 16)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+3)*64+CHANNEL+32)<4>  REG2(r,nTEMP16, 24)<0;8,1>:ub

    // write dp4 (convert float to ud first, write whole ud) of 2 pixel to the 6/7th row
    mov (8)  REG2(r, nTEMP17,  0)<1>:d     REG2(r, nTEMP16, 0)<0;8,1>:f
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+4)*64+CHANNEL   )<4>  REG2(r,nTEMP17,  0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+4)*64+CHANNEL+32)<4>  REG2(r,nTEMP17,  8)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+5)*64+CHANNEL   )<4>  REG2(r,nTEMP17, 16)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+5)*64+CHANNEL+32)<4>  REG2(r,nTEMP17, 24)<0;8,1>:ub
  #endif    

    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(1, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  2)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(2, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  4)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(3, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  6)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(4, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  8)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(5, 0)<5;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 10)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(6, 0)<6;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 12)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(7, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 14)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub

    // ####  write this channel
    mov (8) ubDEST_RGBX(0,ROW_NUM_WRITE*64+CHANNEL   )<4>  REG2(r,nTEMP12, 0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,ROW_NUM_WRITE*64+CHANNEL+32)<4>  REG2(r,nTEMP12, 8)<0;8,1>:ub
    
    // ###### do one row for Blue channel
    #define fCOEF_REG  fYUV_to_RGB_CH0_Coef_Float  // reg for Blue coefficient
    #define CHANNEL   0
    // ##### dp4(nTEMP16) and save result to uw format(nTEMP12)
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(0, 0)<0;8,1>   fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  0)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub

  #if (ONE_ROW_DEBUG)
    // write dp4 (raw float) of 2 pixel to the 4/5th row
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+2)*64+CHANNEL   )<4>  REG2(r,nTEMP16,  0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+2)*64+CHANNEL+32)<4>  REG2(r,nTEMP16,  8)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+3)*64+CHANNEL   )<4>  REG2(r,nTEMP16, 16)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+3)*64+CHANNEL+32)<4>  REG2(r,nTEMP16, 24)<0;8,1>:ub

    // write dp4 (convert float to ud first, write whole ud) of 2 pixel to the 6/7th row
    mov (8)  REG2(r, nTEMP17,  0)<1>:d     REG2(r, nTEMP16, 0)<0;8,1>:f
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+4)*64+CHANNEL   )<4>  REG2(r,nTEMP17,  0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+4)*64+CHANNEL+32)<4>  REG2(r,nTEMP17,  8)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+5)*64+CHANNEL   )<4>  REG2(r,nTEMP17, 16)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,(DBG_ROWNUM_BASE+5)*64+CHANNEL+32)<4>  REG2(r,nTEMP17, 24)<0;8,1>:ub
  #endif    

    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(1, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  2)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(2, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  4)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(3, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  6)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(4, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12,  8)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(5, 0)<5;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 10)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(6, 0)<6;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 12)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub
    
    dp4 (8)  REG2(r, nTEMP16,  0)<1>:f          fROW_YUVA(7, 0)<0;8,1>         fCOEF_REG<0;4,1>:f 
    mov.sat (2)  REG2(r, nTEMP14,  0)<1>:ud     REG2(r, nTEMP16, 0)<0;2,4>:f
    mov (2)  REG2(r, nTEMP12, 14)<1>:ub         REG2(r, nTEMP14, 0)<0;2,4>:ub

    // ####  write this channel
    mov (8) ubDEST_RGBX(0,ROW_NUM_WRITE*64+CHANNEL   )<4>  REG2(r,nTEMP12, 0)<0;8,1>:ub
    mov (8) ubDEST_RGBX(0,ROW_NUM_WRITE*64+CHANNEL+32)<4>  REG2(r,nTEMP12, 8)<0;8,1>:ub
#if (!ONE_ROW_DEBUG)
    }
#endif    
