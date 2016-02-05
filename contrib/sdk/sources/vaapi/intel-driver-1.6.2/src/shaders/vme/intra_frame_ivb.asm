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
        
/* m0 */        
mov  (8) vme_msg_0.0<1>:UD      vme_m0.0<8,8,1>:UD {align1};

/* m2 */        
mov  (8) vme_msg_2<1>:UD        0x0:UD {align1};

/*
 * VME message
 */
/* m0 */        
mov  (8) vme_msg_0.0<1>:UD      vme_m0.0<8,8,1>:UD {align1};

/* m1 */
mov  (8) vme_m1.0<1>:ud		0x0:ud	{align1};
and.z.f0.0 (1) null<1>:UW transform_8x8_ub<0,1,0>:UB 1:UW {align1};
(f0.0) mov  (1) intra_part_mask_ub<1>:UB  LUMA_INTRA_8x8_DISABLE:uw {align1};

/* assign MB intra struct from the thread payload*/
mov (1) mb_intra_struct_ub<1>:UB input_mb_intra_ub<0,1,0>:UB {align1}; 
                           
mov  (8) vme_msg_1<1>:UD        vme_m1.0<8,8,1>:UD {align1};

/* m2 */        
mov (8) vme_msg_2<1>:UD         0x0:UD {align1};

/* m3 */
mov  (1) INEP_ROW.0<1>:UD       0x0:UD {align1};
and  (1) INEP_ROW.4<1>:UD       INEP_ROW.4<0,1,0>:UD            0xFF000000:UD {align1};
mov  (8) vme_msg_3<1>:UD         INEP_ROW.0<8,8,1>:UD {align1};

/* m4 */        
mov  (8) vme_msg_4<1>:UD         0x0 {align1};
mov (16) vme_msg_4.0<1>:UB       INEP_COL0.3<32,8,4>:UB {align1};
mov  (1) vme_msg_4.16<1>:UD      INTRA_PREDICTORE_MODE {align1};

send (8)
        vme_msg_ind
        vme_wb
        null
        vme(
                BIND_IDX_VME,
                0,
                0,
                VME_MESSAGE_TYPE_INTRA
        )
        mlen vme_msg_length
        rlen vme_intra_wb_length
        {align1};


/*
 * Oword Block Write message
 */
mov  (8) msg_reg0.0<1>:UD       obw_m0<8,8,1>:UD {align1};
        
mov  (1) msg_reg1.0<1>:UD       vme_wb.0<0,1,0>:UD      {align1};
mov  (1) msg_reg1.4<1>:UD       vme_wb.16<0,1,0>:UD     {align1};
mov  (1) msg_reg1.8<1>:UD       vme_wb.20<0,1,0>:UD     {align1};
mov  (1) msg_reg1.12<1>:UD      vme_wb.24<0,1,0>:UD     {align1};

/* bind index 3, write 2 oword (32bytes), msg type: 8(OWord Block Write) */
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

__EXIT: 
/*
 * kill thread
 */        
mov  (8) ts_msg_reg0<1>:UD         r0<8,8,1>:UD {align1};
send (16) ts_msg_ind acc0<1>UW null thread_spawner(0, 0, 1) mlen 1 rlen 0 {align1 EOT};
