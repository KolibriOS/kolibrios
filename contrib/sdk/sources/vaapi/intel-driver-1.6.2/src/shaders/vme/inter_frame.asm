/*
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
 */
// Modual name: IntraFrame.asm
//
// Make intra predition estimation for Intra frame
//

//
//  Now, begin source code....
//

/*
 * __START
 */
__INTER_START:
mov  (16) tmp_reg0.0<1>:UD      0x0:UD {align1};
mov  (16) tmp_reg2.0<1>:UD      0x0:UD {align1};
mov  (16) tmp_reg3.0<1>:UD      0x0:UD {align1};

shl  (2) read0_header.0<1>:D    orig_xy_ub<2,2,1>:UB 4:UW {align1};    /* (x, y) * 16 */
add  (1) read0_header.0<1>:D    read0_header.0<0,1,0>:D -8:W {align1};     /* X offset */
add  (1) read0_header.4<1>:D    read0_header.4<0,1,0>:D -1:W {align1};     /* Y offset */ 
mov  (1) read0_header.8<1>:UD   BLOCK_32X1 {align1};
mov  (1) read0_header.20<1>:UB  thread_id_ub {align1};                  /* dispatch id */

shl  (2) read1_header.0<1>:D    orig_xy_ub<2,2,1>:UB 4:UW {align1};    /* (x, y) * 16 */
add  (1) read1_header.0<1>:D    read1_header.0<0,1,0>:D -4:W {align1};     /* X offset */
mov  (1) read1_header.8<1>:UD   BLOCK_4X16 {align1};
mov  (1) read1_header.20<1>:UB  thread_id_ub {align1};                  /* dispatch id */
        
shl  (2) vme_m0.8<1>:UW         orig_xy_ub<2,2,1>:UB 4:UW {align1};    /* Source =  (x, y) * 16 */

cmp.z.f0.0 (1)	null<1>:uw	quality_level_ub<0,1,0>:ub		LOW_QUALITY_LEVEL:uw   {align1};
(f0.0) jmpi (1) __low_quality_search;

__high_quality_search:
#ifdef DEV_SNB
shl  (2) vme_m0.0<1>:UW         orig_xy_ub<2,2,1>:UB 4:UW {align1};	
add  (1) vme_m0.0<1>:W          vme_m0.0<0,1,0>:W -16:W {align1};		/* Reference = (x-16,y-12)-(x+32,y+24) */
add  (1) vme_m0.2<1>:W          vme_m0.2<0,1,0>:W -12:W {align1};
#else
mov  (1) vme_m0.0<1>:W          -16:W {align1} ;                /* Reference = (x-16,y-12)-(x+32,y+24) */
mov  (1) vme_m0.2<1>:W          -12:W {align1} ;
#endif
        
mov  (1) vme_m0.12<1>:UD        SEARCH_CTRL_SINGLE + INTER_PART_MASK + INTER_SAD_HAAR + SUB_PEL_MODE_QUARTER:UD {align1};    /* 16x16 Source, 1/4 pixel, harr */
mov  (1) vme_m0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */
mov  (1) vme_m0.22<1>:UW        REF_REGION_SIZE {align1};               /* Reference Width&Height, 48x40 */
jmpi __vme_msg1;


__low_quality_search:
#ifdef DEV_SNB
shl  (2) vme_m0.0<1>:UW         orig_xy_ub<2,2,1>:UB 4:UW {align1};	
add  (1) vme_m0.0<1>:W          vme_m0.0<0,1,0>:W -8:W {align1};
add  (1) vme_m0.2<1>:W          vme_m0.2<0,1,0>:W -8:W {align1};
#else
mov  (1) vme_m0.0<1>:W          -8:W {align1} ;
mov  (1) vme_m0.2<1>:W          -8:W {align1} ;
#endif
        
mov  (1) vme_m0.12<1>:UD        SEARCH_CTRL_SINGLE + INTER_PART_MASK + INTER_SAD_HAAR + SUB_PEL_MODE_HALF:UD {align1};    /* 16x16 Source, 1/2 pixel, harr */
mov  (1) vme_m0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */
mov  (1) vme_m0.22<1>:UW        MIN_REF_REGION_SIZE {align1};               /* Reference Width&Height, 32x32 */

__vme_msg1:
mov  (1) vme_m1.0<1>:UD         ADAPTIVE_SEARCH_ENABLE:ud {align1} ;
mov  (1) vme_m1.4<1>:UD         FB_PRUNING_ENABLE:UD {align1};
/* MV num is passed by constant buffer. R4.28 */
mov  (1) vme_m1.4<1>:UB		r4.28<0,1,0>:UB {align1};
mov  (1) vme_m1.8<1>:UD         START_CENTER + SEARCH_PATH_LEN:UD {align1};

mul  (1) obw_m0.8<1>:UD         w_in_mb_uw<0,1,0>:UW orig_y_ub<0,1,0>:UB {align1};
add  (1) obw_m0.8<1>:UD         obw_m0.8<0,1,0>:UD orig_x_ub<0,1,0>:UB {align1};
mul  (1) obw_m0.8<1>:UD         obw_m0.8<0,1,0>:UD INTER_VME_OUTPUT_IN_OWS:UD {align1};
mov  (1) obw_m0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */

__VME_LOOP:     

/*
 * Media Read Message -- fetch neighbor edge pixels 
 */
/* ROW */
mov  (8) msg_reg0.0<1>:UD       read0_header.0<8,8,1>:UD {align1};        
send (8) msg_ind INEP_ROW<1>:UB null read(BIND_IDX_INEP, 0, 0, 4) mlen 1 rlen 1 {align1};

/* COL */
mov  (8) msg_reg0.0<1>:UD       read1_header.0<8,8,1>:UD {align1};                
send (8) msg_ind INEP_COL0<1>:UB null read(BIND_IDX_INEP, 0, 0, 4) mlen 1 rlen 2 {align1};
        
/*
 * VME message
 */
/* m0 */
mov  (8) vme_msg_0.0<1>:UD      vme_m0.0<8,8,1>:UD {align1};
        
/* m1 */
mov  (1) intra_flag<1>:UW       0x0:UW {align1}                     ;
and.z.f0.0 (1) null<1>:UW transform_8x8_ub<0,1,0>:UB 1:UW {align1};
(f0.0) mov  (1) intra_part_mask_ub<1>:UB  LUMA_INTRA_8x8_DISABLE {align1};

cmp.nz.f0.0 (1) null<1>:UW orig_x_ub<0,1,0>:UB 0:UW {align1};                                                   /* X != 0 */
(f0.0) add (1) mb_intra_struct_ub<1>:UB mb_intra_struct_ub<0,1,0>:UB INTRA_PRED_AVAIL_FLAG_AE {align1};         /* A */

cmp.nz.f0.0 (1) null<1>:UW orig_y_ub<0,1,0>:UB 0:UW {align1};                                                   /* Y != 0 */
(f0.0) add (1) mb_intra_struct_ub<1>:UB mb_intra_struct_ub<0,1,0>:UB INTRA_PRED_AVAIL_FLAG_B {align1};          /* B */

mul.nz.f0.0 (1) null<1>:UW orig_x_ub<0,1,0>:UB orig_y_ub<0,1,0>:UB {align1};                                    /* X * Y != 0 */
(f0.0) add (1) mb_intra_struct_ub<1>:UB mb_intra_struct_ub<0,1,0>:UB INTRA_PRED_AVAIL_FLAG_D {align1};          /* D */

add  (1) tmp_x_w<1>:W orig_x_ub<0,1,0>:UB 1:UW {align1};                                                        /* X + 1 */
add  (1) tmp_x_w<1>:W w_in_mb_uw<0,1,0>:UW -tmp_x_w<0,1,0>:W {align1};                                          /* width - (X + 1) */
mul.nz.f0.0 (1) null<1>:UD tmp_x_w<0,1,0>:W orig_y_ub<0,1,0>:UB {align1};                                       /* (width - (X + 1)) * Y != 0 */
(f0.0) add (1) mb_intra_struct_ub<1>:UB mb_intra_struct_ub<0,1,0>:UB INTRA_PRED_AVAIL_FLAG_C {align1};          /* C */

and.nz.f0.0 (1) null<1>:UW slice_edge_ub<0,1,0>:UB 2:UW {align1};
(f0.0) and (1) mb_intra_struct_ub<1>:UB mb_intra_struct_ub<0,1,0>:UB  0xE0 {align1};                            /* slice edge disable B,C,D*/
        
mov  (8) vme_msg_1<1>:UD        vme_m1.0<8,8,1>:UD {align1};
        
/* m2 */        
mov  (8) vme_msg_2<1>:UD        0x0:UD {align1};

/* m3 */
mov  (1) INEP_ROW.0<1>:UD       0x0:UD {align1};
and  (1) INEP_ROW.4<1>:UD       INEP_ROW.4<0,1,0>:UD            0xFF000000:UD {align1};
mov  (8) vme_msg_3<1>:UD        INEP_ROW.0<8,8,1>:UD {align1};        

/* m4 */
mov  (8) vme_msg_4<1>:UD        0x0 {align1};
mov (16) vme_msg_4.0<1>:UB      INEP_COL0.3<32,8,4>:UB {align1};
mov  (1) vme_msg_4.16<1>:UD     INTRA_PREDICTORE_MODE {align1};

send (8)
        vme_msg_ind
        vme_wb
        null
        vme(
                BIND_IDX_VME,
                0,
                0,
                VME_MESSAGE_TYPE_MIXED
        )
        mlen vme_msg_length
        rlen vme_inter_wb_length
        {align1};
/*
 * Oword Block Write message
 */

/* MV pairs */        
mov  (8) msg_reg0.0<1>:UD       obw_m0.0<8,8,1>:UD {align1};

#ifdef DEV_SNB        
mov  (16) obw_m1.0<1>:UW        vme_wb1.0<16,16,1>:UB  {align1};
add  (8) obw_m1.0<2>:W          obw_m1.0<16,8,2>:W -64:W {align1};
add  (8) obw_m1.2<2>:W          obw_m1.2<16,8,2>:W -48:W {align1};
mov  (16) obw_m2.0<1>:UW        vme_wb1.16<16,16,1>:UB  {align1};
add  (8) obw_m2.0<2>:W          obw_m2.0<16,8,2>:W -64:W {align1};
add  (8) obw_m2.2<2>:W          obw_m2.2<16,8,2>:W -48:W {align1}; 
mov  (16) obw_m3.0<1>:UW        vme_wb2.0<16,16,1>:UB  {align1};
add  (8) obw_m3.0<2>:W          obw_m3.0<16,8,2>:W -64:W {align1};
add  (8) obw_m3.2<2>:W          obw_m3.2<16,8,2>:W -48:W {align1};
mov  (16) obw_m4.0<1>:UW        vme_wb2.16<16,16,1>:UB  {align1};
add  (8) obw_m4.0<2>:W          obw_m4.0<16,8,2>:W -64:W {align1};
add  (8) obw_m4.2<2>:W          obw_m4.2<16,8,2>:W -48:W {align1}; 
#else
mov  (8) obw_m1.0<1>:ud         vme_wb1.0<8,8,1>:ud {align1};
mov  (8) obw_m2.0<1>:ud         vme_wb2.0<8,8,1>:ud {align1};
mov  (8) obw_m3.0<1>:ud         vme_wb3.0<8,8,1>:ud {align1};
mov  (8) obw_m4.0<1>:ud         vme_wb4.0<8,8,1>:ud {align1};                
#endif       
        
mov  (8) msg_reg1.0<1>:UD       obw_m1.0<8,8,1>:UD   {align1};

mov  (8) msg_reg2.0<1>:UD       obw_m2.0<8,8,1>:UD   {align1};

mov  (8) msg_reg3.0<1>:UD       obw_m3.0<8,8,1>:UD   {align1};

mov  (8) msg_reg4.0<1>:UD       obw_m4.0<8,8,1>:UD   {align1};                

/* bind index 3, write 8 oword, msg type: 8(OWord Block Write) */
send (16)
        msg_ind
        obw_wb
        null
        data_port(
                OBW_CACHE_TYPE,
                OBW_MESSAGE_TYPE,
                OBW_CONTROL_4,
                OBW_BIND_IDX,
                OBW_WRITE_COMMIT_CATEGORY,
                OBW_HEADER_PRESENT
        )
        mlen 5
        rlen obw_wb_length
        {align1};

/* other info */        
add             (1)     msg_reg0.8<1>:UD        obw_m0.8<0,1,0>:UD      INTER_VME_OUTPUT_MV_IN_OWS:UD {align1} ;

and.z.f0.0      (1)     null<1>:ud              vme_wb0.0<0,1,0>:ud     INTRAMBFLAG_MASK:ud {align1} ;

(-f0.0)jmpi     (1)     __INTRA_INFO ;

__INTER_INFO:   
mov             (1)     tmp_uw1<1>:uw           0:uw {align1} ;
mov             (1)     tmp_ud1<1>:ud           0:ud {align1} ;
(f0.0)and       (1)     tmp_uw1<1>:uw           vme_wb0.2<0,1,0>:uw     MV32_BIT_MASK:uw {align1} ;
(f0.0)shr       (1)     tmp_uw1<1>:uw           tmp_uw1<1>:uw           MV32_BIT_SHIFT:uw {align1} ;
(f0.0)mul       (1)     tmp_ud1<1>:ud           tmp_uw1<0,1,0>:uw       96:uw {align1} ;
(f0.0)add       (1)     tmp_ud1<1>:ud           tmp_ud1<0,1,0>:ud       32:uw {align1} ;
(f0.0)shl       (1)     tmp_uw1<1>:uw           tmp_uw1<0,1,0>:uw       MFC_MV32_BIT_SHIFT:uw {align1} ;
(f0.0)add       (1)     tmp_uw1<1>:uw           tmp_uw1<0,1,0>:uw       MVSIZE_UW_BASE:uw {align1} ;
add             (1)     tmp_uw1<1>:uw           tmp_uw1<0,1,0>:uw       CBP_DC_YUV_UW:uw {align1} ;

mov             (1)     msg_reg1.0<1>:uw        vme_wb0.0<0,1,0>:uw     {align1} ;
mov             (1)     msg_reg1.2<1>:uw        tmp_uw1<0,1,0>:uw       {align1} ;
mov             (1)     msg_reg1.4<1>:UD        vme_wb0.28<0,1,0>:UD    {align1};
mov             (1)     msg_reg1.8<1>:ud        tmp_ud1<0,1,0>:ud       {align1} ;

jmpi            (1)     __OUTPUT_INFO ;
        
__INTRA_INFO:
mov             (1)     msg_reg1.0<1>:UD        vme_wb.0<0,1,0>:UD      {align1};
mov             (1)     msg_reg1.4<1>:UD        vme_wb.16<0,1,0>:UD     {align1};
mov             (1)     msg_reg1.8<1>:UD        vme_wb.20<0,1,0>:UD     {align1};
mov             (1)     msg_reg1.12<1>:UD       vme_wb.24<0,1,0>:UD     {align1};

__OUTPUT_INFO:  
/* bind index 3, write 1 oword, msg type: 8(OWord Block Write) */
send (16)
        msg_ind
        obw_wb
        null
        data_port(
                OBW_CACHE_TYPE,
                OBW_MESSAGE_TYPE,
                OBW_CONTROL_0,
                OBW_BIND_IDX,
                OBW_WRITE_COMMIT_CATEGORY,
                OBW_HEADER_PRESENT
        )
        mlen 2
        rlen obw_wb_length
        {align1};

add             (1)     orig_x_ub<1>:ub         orig_x_ub<0,1,0>:ub             1:uw {align1} ;
add             (1)     vme_m0.8<1>:UW          vme_m0.8<0,1,0>:UW              16:UW {align1};    /* X += 16 */
#ifdef DEV_SNB        
add             (1)     vme_m0.0<1>:W           vme_m0.0<0,1,0>:W               16:W {align1};	   /* X += 16 */
#endif

cmp.e.f0.0      (1)     null<1>:uw              w_in_mb_uw<0,1,0>:uw            orig_x_ub<0,1,0>:ub {align1};
/* (0, y + 1) */        
(f0.0)mov       (1)     orig_x_ub<1>:ub         0:uw {align1} ;
(f0.0)add       (1)     orig_y_ub<1>:ub orig_y_ub<0,1,0>:ub             1:uw {align1} ;
(f0.0)mov       (1)     vme_m0.8<1>:uw          0:uw {align1} ;
(f0.0)add       (1)     vme_m0.10<1>:uw         vme_m0.10<0,1,0>:uw             16:uw {align1} ;
#ifdef DEV_SNB        
(f0.0)mov       (1)     vme_m0.0<1>:w           -16:W {align1};		        /* Reference = (x-16,y-12)-(x+32,y+24) */
(f0.0)add       (1)     vme_m0.2<1>:w           vme_m0.2<0,1,0>:w               16:w {align1};
#endif

shl  (2) read0_header.0<1>:D    orig_xy_ub<2,2,1>:UB 4:UW {align1};    /* (x, y) * 16 */
add  (1) read0_header.0<1>:D    read0_header.0<0,1,0>:D -8:W {align1};     /* X offset */
add  (1) read0_header.4<1>:D    read0_header.4<0,1,0>:D -1:W {align1};     /* Y offset */ 

shl  (2) read1_header.0<1>:D    orig_xy_ub<2,2,1>:UB 4:UW {align1};    /* (x, y) * 16 */
add  (1) read1_header.0<1>:D    read1_header.0<0,1,0>:D -4:W {align1};     /* X offset */

shl  (2) vme_m0.8<1>:UW         orig_xy_ub<2,2,1>:UB 4:UW {align1};    /* Source =  (x, y) * 16 */

add             (1)     obw_m0.8<1>:UD          obw_m0.8<0,1,0>:UD              INTER_VME_OUTPUT_IN_OWS:UW {align1} ;
        
add.z.f0.1      (1)     num_macroblocks<1>:w    num_macroblocks<0,1,0>:w        -1:w {align1} ;
(-f0.1)jmpi     (1)     __VME_LOOP ;
        
__EXIT: 
        
/*
 * kill thread
 */        
mov  (8) msg_reg0<1>:UD         r0<8,8,1>:UD {align1};
send (16) msg_ind acc0<1>UW null thread_spawner(0, 0, 1) mlen 1 rlen 0 {align1 EOT};
