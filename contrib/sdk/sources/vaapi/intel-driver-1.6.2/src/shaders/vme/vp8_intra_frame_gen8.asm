/*
 * Copyright © 2014 Intel Corporation
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
 * Authors:
 *    Zhao Yakui <yakui.zhao@intel.com>
 *    Xiang Haihao <haihao.xiang@intel.com>
 *    Li Zhong <zhong.li@intel.com>
 *
 */

/*
 * __START
 */
__INTRA_START:
mov  (16) tmp_reg0.0<1>:UD      0x0:UD {align1};
mov  (16) tmp_reg2.0<1>:UD      0x0:UD {align1};
mov  (16) tmp_reg4.0<1>:UD      0x0:UD {align1} ;
mov  (16) tmp_reg6.0<1>:UD      0x0:UD {align1} ;

shl  (2) read0_header.0<1>:D    orig_xy_ub<2,2,1>:UB 4:UW {align1};    /* (x, y) * 16 */
add  (1) read0_header.0<1>:D    read0_header.0<0,1,0>:D -8:W {align1};     /* X offset */
add  (1) read0_header.4<1>:D    read0_header.4<0,1,0>:D -1:W {align1};     /* Y offset */ 
mov  (1) read0_header.8<1>:UD   BLOCK_32X1 {align1};
mov  (1) read0_header.20<1>:UB  thread_id_ub {align1};                  /* dispatch id */

shl  (2) read1_header.0<1>:D    orig_xy_ub<2,2,1>:UB 4:UW {align1};    /* (x, y) * 16 */
add  (1) read1_header.0<1>:D    read1_header.0<0,1,0>:D -4:W {align1};     /* X offset */
mov  (1) read1_header.8<1>:UD   BLOCK_4X16 {align1};
mov  (1) read1_header.20<1>:UB  thread_id_ub {align1};                  /* dispatch id */
        
shl  (2) vme_m0.8<1>:UW         orig_xy_ub<2,2,1>:UB 4:UW {align1};    /* (x, y) * 16 */
mov  (1) vme_m0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */

mul  (1) obw_m0.8<1>:UD         w_in_mb_uw<0,1,0>:UW orig_y_ub<0,1,0>:UB {align1};
add  (1) obw_m0.8<1>:UD         obw_m0.8<0,1,0>:UD orig_x_ub<0,1,0>:UB {align1};
mul  (1) obw_m0.8<1>:UD         obw_m0.8<0,1,0>:UD 0x02:UD {align1};
mov  (1) obw_m0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */
        
/*
 * Media Read Message -- fetch Luma neighbor edge pixels 
 */
/* ROW */
mov  (8) msg_reg0.0<1>:UD       read0_header.0<8,8,1>:UD {align1};        
send (8) msg_ind INEP_ROW<1>:UB null read(BIND_IDX_INEP, 0, 0, 4) mlen 1 rlen 1 {align1};

/* COL */
mov  (8) msg_reg0.0<1>:UD       read1_header.0<8,8,1>:UD {align1};                
send (8) msg_ind INEP_COL0<1>:UB null read(BIND_IDX_INEP, 0, 0, 4) mlen 1 rlen 2 {align1};
        
/*
 * Media Read Message -- fetch Chroma neighbor edge pixels 
 */
/* ROW */
shl  (2) read0_header.0<1>:D    orig_xy_ub<2,2,1>:UB 3:UW {align1};    /* x * 16 , y * 8 */
mul  (1) read0_header.0<1>:D    read0_header.0<0,1,0>:D  2:W {align1};
add  (1) read0_header.0<1>:D    read0_header.0<0,1,0>:D -8:W {align1};     /* X offset */
add  (1) read0_header.4<1>:D    read0_header.4<0,1,0>:D -1:W {align1};     /* Y offset */ 
mov  (8) msg_reg0.0<1>:UD       read0_header.0<8,8,1>:UD {align1};        
send (8) msg_ind CHROMA_ROW<1>:UB null read(BIND_IDX_CBCR, 0, 0, 4) mlen 1 rlen 1 {align1};

/* COL */
shl  (2) read1_header.0<1>:D    orig_xy_ub<2,2,1>:UB 3:UW {align1};    /* x * 16, y * 8 */
mul  (1) read1_header.0<1>:D    read1_header.0<0,1,0>:D  2:W {align1};
add  (1) read1_header.0<1>:D    read1_header.0<0,1,0>:D -4:W {align1};     /* X offset */
mov  (1) read1_header.8<1>:UD   BLOCK_8X4 {align1};
mov  (8) msg_reg0.0<1>:UD       read1_header.0<8,8,1>:UD {align1};                
send (8) msg_ind CHROMA_COL<1>:UB null read(BIND_IDX_CBCR, 0, 0, 4) mlen 1 rlen 1 {align1};

/* m2, get the MV/Mb cost passed by constant buffer 
when creating EU thread by MEDIA_OBJECT */       
mov (8) vme_msg_2<1>:UD         r1.0<8,8,1>:UD {align1};

/* m3. This is changed for FWD/BWD cost center */
mov (8) vme_msg_3<1>:UD		0x0:UD {align1};	        

/* m4.*/
mov (8) vme_msg_4<1>:ud		0x0:ud	{align1};

/* m5 */
mov  (1) INEP_ROW.0<1>:UD       0x0:UD {align1};
and  (1) INEP_ROW.4<1>:UD       INEP_ROW.4<0,1,0>:UD            0xFF000000:UD {align1};
mov  (8) vme_msg_5<1>:UD        INEP_ROW.0<8,8,1>:UD {align1};

mov  (1) tmp_reg0.0<1>:UB	INTRA_PLANAR_MODE_MASK {align1}; /* vp8 don't support planar intra mode */
mov  (1) tmp_reg0.1<1>:UB	LUMA_CHROMA_MODE {align1}; /* Intra type: Luma + Chroma */

/* Intra mode mask && Intra compute type */
mov  (1) vme_msg_5.4<1>:UW	tmp_reg0.0<0,1,0>:UW {align1};

/* m6 */        
mov  (8) vme_msg_6<1>:UD         0x0:UD {align1};
mov (16) vme_msg_6.0<1>:UB       INEP_COL0.3<32,8,4>:UB {align1};
mov  (1) vme_msg_6.16<1>:UD      INTRA_PREDICTORE_MODE {align1};

/* the penalty for Intra mode */
mov  (1) vme_msg_6.28<1>:UD	0x010101:UD {align1};
mov  (1) vme_msg_6.20<1>:UW      CHROMA_ROW.6<0,1,0>:UW {align1};


/* m7 */

mov  (4) vme_msg_7.16<1>:UD      CHROMA_ROW.8<4,4,1>:UD {align1};
mov  (8) vme_msg_7.0<1>:UW       CHROMA_COL.2<16,8,2>:UW {align1};

/*
 * VME message
 */

/* m1 */
mov  (1) intra_flag<1>:UW       0x0:UW {align1};
mov  (1) intra_part_mask_ub<1>:UB  LUMA_INTRA_8x8_DISABLE {align1}; /* vp8 don't support intra_8x8 mode*/

/* assign MB intra struct from the thread payload*/
mov (1) mb_intra_struct_ub<1>:UB input_mb_intra_ub<0,1,0>:UB {align1}; 
                           
/* Disable DC HAAR component when calculating HARR SATD block */
mov  (1) tmp_reg0.0<1>:UW	DC_HARR_DISABLE:UW		{align1};
mov  (1) vme_m1.30<1>:UB	tmp_reg0.0<0,1,0>:UB  {align1};

mov  (8) vme_msg_1<1>:UD        vme_m1.0<8,8,1>:UD {align1};

/* m0 */        
add  (1) vme_m0.12<1>:UD        vme_m0.12<0,1,0>:ud	INTRA_SAD_HAAR:UD {align1};/* 16x16 Source, Intra_harr */
mov  (1) vme_m0.15<1>:UB        SUB_PART_8x4_DISABLE + SUB_PART_4x8_DISABLE {align1}; /* vp8 don't support 8x4 and 4x8 partion */
mov  (8) vme_msg_0<1>:UD        vme_m0.0<8,8,1>:UD {align1};

/* after verification it will be passed by using payload */
send (8)
        vme_msg_ind
        vme_wb<1>:UD
        null
        cre(
                BIND_IDX_VME,
                VME_SIC_MESSAGE_TYPE
        )
        mlen sic_vme_msg_length
        rlen vme_wb_length
        {align1};
/*
 * Oword Block Write message
 */
mov  (8) msg_reg0.0<1>:UD       obw_m0<8,8,1>:UD {align1};
        
mov  (1) msg_reg1.0<1>:UD       vme_wb.0<0,1,0>:UD      {align1};
mov  (1) msg_reg1.4<1>:UD       vme_wb.16<0,1,0>:UD     {align1};
mov  (1) msg_reg1.8<1>:UD       vme_wb.20<0,1,0>:UD     {align1};
mov  (1) msg_reg1.12<1>:UD      vme_wb.24<0,1,0>:UD     {align1};

/* Distortion, Intra (17-16), */
mov  (1) msg_reg1.16<1>:UW      vme_wb.12<0,1,0>:UW     {align1};

mov  (1) msg_reg1.20<1>:UD      vme_wb.8<0,1,0>:UD     {align1};
/* VME clock counts */
mov  (1) msg_reg1.24<1>:UD      vme_wb.28<0,1,0>:UD     {align1};

mov  (1) msg_reg1.28<1>:UD      obw_m0.8<0,1,0>:UD     {align1};

/* bind index 3, write 2 oword (32bytes), msg type: 8(OWord Block Write) */
send (16)
        msg_ind
        obw_wb
        null
        data_port(
                OBW_CACHE_TYPE,
                OBW_MESSAGE_TYPE,
                OBW_CONTROL_2,
                OBW_BIND_IDX,
                OBW_WRITE_COMMIT_CATEGORY,
                OBW_HEADER_PRESENT
        )
        mlen 2
        rlen obw_wb_length
        {align1};

__EXIT: 
/*
 * kill thread
 */        
mov  (8) ts_msg_reg0<1>:UD         r0<8,8,1>:UD {align1};
send (16) ts_msg_ind acc0<1>UW null thread_spawner(0, 0, 1) mlen 1 rlen 0 {align1 EOT};
