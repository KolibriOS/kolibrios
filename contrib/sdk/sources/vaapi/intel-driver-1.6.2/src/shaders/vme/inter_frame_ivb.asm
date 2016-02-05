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
 * Authors: Zhao Yakui <yakui.zhao@intel.com>
 *
 */
// Modual name: InterFrame_ivy.asm
//
// Make inter predition estimation for Inter frame on Ivy
//

//
//  Now, begin source code....
//

#define SAVE_RET	add (1) RETURN_REG<1>:ud   ip:ud	32:ud
#define	RETURN		mov (1)	ip:ud	RETURN_REG<0,1,0>:ud

/*
 * __START
 */
__INTER_START:
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
mul  (1) obw_m0.8<1>:UD         obw_m0.8<0,1,0>:UD INTER_VME_OUTPUT_IN_OWS:UD {align1};
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
        
mov  (8) mb_mvp_ref.0<1>:ud	0:ud		{align1};
mov  (8) mb_ref_win.0<1>:ud	0:ud		{align1};
and.z.f0.0 (1)		null:uw	mb_hwdep<0,1,0>:uw		0x04:uw   {align1};
(f0.0) jmpi (1) __mb_hwdep_end;
/* read back the data for MB A */
/* the layout of MB result is: rx.0(Available). rx.4(MVa), rX.8(MVb), rX.16(Pred_L0 flag),
*  rX.18 (Pred_L1 flag), rX.20(Forward reference ID), rX.22(Backwared reference ID)
*/
mov  (8) mba_result.0<1>:ud	0x0:ud		{align1};
mov  (8) mbb_result.0<1>:ud	0x0:ud		{align1};
mov  (8) mbc_result.0<1>:ud	0x0:ud		{align1};
mba_start:
mov  (8) mb_msg0.0<1>:ud	0:ud		{align1};
and.z.f0.0 (1)		null:uw	input_mb_intra_ub<0,1,0>:ub	INTRA_PRED_AVAIL_FLAG_AE:uw   {align1};
/* MB A doesn't exist. Zero MV. mba_flag is zero and ref ID = -1 */
(f0.0)  mov  (2)    	mba_result.20<1>:w	-1:w	{align1};
(f0.0)  jmpi (1)	mbb_start;
mov  (1) mba_result.0<1>:d	MB_AVAIL		{align1};	
mov  (2) tmp_reg0.0<1>:UW	orig_xy_ub<2,2,1>:UB	{align1};
add  (1) tmp_reg0.0<1>:w	tmp_reg0.0<0,1,0>:w	-1:w	{align1};
mul  (1) mb_msg0.8<1>:UD       w_in_mb_uw<0,1,0>:UW tmp_reg0.2<0,1,0>:UW {align1};
add  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD   tmp_reg0.0<0,1,0>:uw {align1};
mul  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD INTER_VME_OUTPUT_IN_OWS:UD {align1};
mov  (1) mb_msg0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */
mov  (1) mb_msg_tmp.8<1>:ud	mb_msg0.8<0,1,0>:ud	{align1};

add  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD      INTER_VME_OUTPUT_MV_IN_OWS:UD {align1};
/* bind index 3, read 1 oword (16bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_wb.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_0,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 1
        {align1};

/* TODO: RefID is required after multi-references are added */
and.z.f0.0      (1)     null<1>:ud        mb_mode_wb.0<0,1,0>:ud     INTRAMBFLAG_MASK:ud {align1} ;
(-f0.0)   mov (2)	mba_result.20<1>:w			-1:w	{align1};
(-f0.0)   jmpi	(1)	mbb_start;

mov   (1) mb_msg0.8<1>:UD	mb_msg_tmp.8<0,1,0>:ud	{align1};
/* Read MV for MB A */
/* bind index 3, read 8 oword (128bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_mv0.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_8,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 4
        {align1};
/* TODO: RefID is required after multi-references are added */
/* MV */
mov	   (2)		mba_result.4<1>:ud		mb_mv1.8<2,2,1>:ud	{align1};
mov	   (1)		mba_result.16<1>:w		MB_PRED_FLAG		{align1};

mbb_start:
mov  (8) mb_msg0.0<1>:ud	0:ud		{align1};
and.z.f0.0 (1)		null:uw	input_mb_intra_ub<0,1,0>:ub	INTRA_PRED_AVAIL_FLAG_B:uw   {align1};
/* MB B doesn't exist. Zero MV. mba_flag is zero */
/* If MB B doesn't exist, neither MB C nor D exists */
(f0.0)  mov  (2)    	mbb_result.20<1>:w	-1:w		{align1};
(f0.0)  mov  (2)    	mbc_result.20<1>:w	-1:w		{align1};
(f0.0)  jmpi (1)	mb_mvp_start;
mov  (1) mbb_result.0<1>:d	MB_AVAIL		{align1};	
mov  (2) tmp_reg0.0<1>:UW	orig_xy_ub<2,2,1>:UB	{align1};
add  (1) tmp_reg0.2<1>:w	tmp_reg0.2<0,1,0>:w	-1:w	{align1};
mul  (1) mb_msg0.8<1>:UD       w_in_mb_uw<0,1,0>:UW tmp_reg0.2<0,1,0>:UW {align1};
add  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD   tmp_reg0.0<0,1,0>:uw {align1};
mul  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD INTER_VME_OUTPUT_IN_OWS:UD {align1};
mov  (1) mb_msg0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */
mov  (1) mb_msg_tmp.8<1>:ud	mb_msg0.8<0,1,0>:ud	{align1};

add  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD      INTER_VME_OUTPUT_MV_IN_OWS:UD {align1};

/* bind index 3, read 4 oword (64bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_wb.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_0,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 1
        {align1};

/* TODO: RefID is required after multi-references are added */
and.z.f0.0      (1)     null<1>:ud        mb_mode_wb.0<0,1,0>:ud     INTRAMBFLAG_MASK:ud {align1} ;
(-f0.0)   mov (2)	mbb_result.20<1>:w			-1:w	{align1};
(-f0.0)   jmpi	(1)	mbc_start;

mov   (1) mb_msg0.8<1>:UD	mb_msg_tmp.8<0,1,0>:ud	{align1};
/* Read MV for MB B */
/* bind index 3, read 8 oword (128bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_mv0.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_8,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 4
        {align1};
/* TODO: RefID is required after multi-references are added */
mov	   (2)		mbb_result.4<1>:ud		mb_mv2.16<2,2,1>:ud	{align1};
mov	   (1)		mbb_result.16<1>:w		MB_PRED_FLAG		{align1};

mbc_start:
mov  (8) mb_msg0.0<1>:ud	0:ud		{align1};
and.z.f0.0 (1)		null:uw	input_mb_intra_ub<0,1,0>:ub	INTRA_PRED_AVAIL_FLAG_C:uw   {align1};
/* MB C doesn't exist. Zero MV. mba_flag is zero */
/* Based on h264 spec the MB D will be replaced if MB C doesn't exist */
(f0.0)  jmpi (1)	mbd_start;
mov  (1) mbc_result.0<1>:d	MB_AVAIL		{align1};	
mov  (2) tmp_reg0.0<1>:UW	orig_xy_ub<2,2,1>:UB	{align1};
add  (1) tmp_reg0.2<1>:w	tmp_reg0.2<0,1,0>:w	-1:w	{align1};
add  (1) tmp_reg0.0<1>:w	tmp_reg0.0<0,1,0>:w	1:w	{align1};
mul  (1) mb_msg0.8<1>:UD       w_in_mb_uw<0,1,0>:UW tmp_reg0.2<0,1,0>:UW {align1};
add  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD   tmp_reg0.0<0,1,0>:uw {align1};
mul  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD INTER_VME_OUTPUT_IN_OWS:UD {align1};
mov  (1) mb_msg0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */

mov  (1) mb_msg_tmp.8<1>:ud	mb_msg0.8<0,1,0>:ud	{align1};

add  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD      INTER_VME_OUTPUT_MV_IN_OWS:UD {align1};
/* bind index 3, read 4 oword (64bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_wb.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_0,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 1
        {align1};

/* TODO: RefID is required after multi-references are added */
and.z.f0.0      (1)     null<1>:ud        mb_mode_wb.0<0,1,0>:ud     INTRAMBFLAG_MASK:ud {align1} ;
(-f0.0)   mov (2)	mbc_result.20<1>:w			-1:w	{align1};
(-f0.0)   jmpi	(1)	mb_mvp_start;
mov   (1) mb_msg0.8<1>:UD	mb_msg_tmp.8<0,1,0>:ud {align1};
/* Read MV for MB C */
/* bind index 3, read 8 oword (128bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_mv0.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_8,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 4
        {align1};
/* TODO: RefID is required after multi-references are added */
/* Forward MV */
mov	   (2)		mbc_result.4<1>:ud		mb_mv2.16<2,2,1>:ud	{align1};
mov	   (1)		mbc_result.16<1>:w		MB_PRED_FLAG		{align1};

jmpi   (1)    mb_mvp_start;
mbd_start:
mov  (8) mb_msg0.0<1>:ud	0:ud		{align1};
and.z.f0.0 (1)		null:uw	input_mb_intra_ub<0,1,0>:ub	INTRA_PRED_AVAIL_FLAG_D:uw   {align1};
(f0.0)  jmpi (1)	mb_mvp_start;
mov  (1) mbc_result.0<1>:d	MB_AVAIL		{align1};	
mov  (2) tmp_reg0.0<1>:UW	orig_xy_ub<2,2,1>:UB	{align1};
add  (2) tmp_reg0.0<1>:w	tmp_reg0.0<2,2,1>:w	-1:w	{align1};
mul  (1) mb_msg0.8<1>:UD       w_in_mb_uw<0,1,0>:UW tmp_reg0.2<0,1,0>:UW {align1};
add  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD   tmp_reg0.0<0,1,0>:uw {align1};

mul  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD INTER_VME_OUTPUT_IN_OWS:UD {align1};
mov  (1) mb_msg0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */
mov  (1) mb_msg_tmp.8<1>:ud	mb_msg0.8<0,1,0>:ud	{align1};

add  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD      INTER_VME_OUTPUT_MV_IN_OWS:UD {align1};
/* bind index 3, read 4 oword (64bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_wb.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_0,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 1
        {align1};

and.z.f0.0      (1)     null<1>:ud        mb_mode_wb.0<0,1,0>:ud     INTRAMBFLAG_MASK:ud {align1} ;
(-f0.0)   mov (2)	mbc_result.20<1>:w			-1:w	{align1};
(-f0.0)   jmpi	(1)	mb_mvp_start;

mov   (1) mb_msg0.8<1>:UD	mb_msg_tmp.8<0,1,0>:ud	{align1};
/* Read MV for MB D */
/* bind index 3, read 8 oword (128bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_mv0.0<1>:ub
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_8,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 4
        {align1};

/* TODO: RefID is required after multi-references are added */

/* Forward MV */
mov	   (2)		mbc_result.4<1>:ud		mb_mv3.24<2,2,1>:ud	{align1};
mov	   (1)		mbc_result.16<1>:w		MB_PRED_FLAG		{align1};
	
mb_mvp_start:
/*TODO: Add the skip prediction */
/* Check whether both MB B and C are invailable */
add	(1)	tmp_reg0.0<1>:d		mbb_result.0<0,1,0>:d	mbc_result.0<0,1,0>:d	{align1};
cmp.z.f0.0 (1)	null:d			tmp_reg0.0<0,1,0>:d	0:d	{align1};
(-f0.0)	jmpi (1)	mb_median_start;
cmp.nz.f0.0 (1)	null:d	mba_result.0<0,1,0>:d		0:d		{align1};
(f0.0)	mov	(1)	mbb_result.4<1>:ud		mba_result.4<0,1,0>:ud	{align1};	
(f0.0)	mov	(1)	mbc_result.4<1>:ud		mba_result.4<0,1,0>:ud	{align1};	
(f0.0)	mov	(1)	mbb_result.20<1>:uw		mba_result.20<0,1,0>:uw	{align1};	
(f0.0)	mov	(1)	mbc_result.20<1>:uw		mba_result.20<0,1,0>:uw	{align1};	
(f0.0)  mov     (1)	mb_mvp_ref.0<1>:ud		mba_result.4<0,1,0>:ud	{align1};
(-f0.0) mov	(1)	mb_mvp_ref.0<1>:ud		0:ud			{align1};
jmpi	(1)	__mb_hwdep_end;
	
mb_median_start:
/* check whether only one neighbour MB has the same ref ID with the current MB */
mov (8)	tmp_reg0.0<1>:ud		0:ud		{align1};
cmp.z.f0.0	(1)	null:d	mba_result.20<1>:w	0:w	{align1};
(f0.0)	add	(1)	tmp_reg0.0<1>:w		tmp_reg0.0<1>:w	1:w	{align1};
(f0.0)	mov	(1)	tmp_reg0.4<1>:ud	mba_result.4<0,1,0>:ud	{align1};
cmp.z.f0.0	(1)	null:d	mbb_result.20<1>:w	0:w	{align1};
(f0.0)	add	(1)	tmp_reg0.0<1>:w		tmp_reg0.0<1>:w	1:w	{align1};
(f0.0)	mov	(1)	tmp_reg0.4<1>:ud	mbb_result.4<0,1,0>:ud	{align1};
cmp.z.f0.0	(1)	null:d	mbc_result.20<1>:w	0:w	{align1};
(f0.0)	add	(1)	tmp_reg0.0<1>:w		tmp_reg0.0<1>:w	1:w	{align1};
(f0.0)	mov	(1)	tmp_reg0.4<1>:ud	mbc_result.4<0,1,0>:ud	{align1};
cmp.e.f0.0	(1)	null:d	tmp_reg0.0<1>:w	 1:w	{align1};
(f0.0)	mov	(1)     mb_mvp_ref.0<1>:ud	tmp_reg0.4<0,1,0>:ud	{align1};
(f0.0)	jmpi (1)  __mb_hwdep_end;

mov	(1)	INPUT_ARG0.0<1>:w	mba_result.4<0,1,0>:w	{align1};
mov	(1)	INPUT_ARG0.4<1>:w	mbb_result.4<0,1,0>:w	{align1};
mov	(1)	INPUT_ARG0.8<1>:w	mbc_result.4<0,1,0>:w	{align1};
SAVE_RET	{align1};
 jmpi	(1)	word_imedian;
mov	(1)	mb_mvp_ref.0<1>:w		RET_ARG<0,1,0>:w	{align1};
mov	(1)	INPUT_ARG0.0<1>:w	mba_result.6<0,1,0>:w	{align1};
mov	(1)	INPUT_ARG0.4<1>:w	mbb_result.6<0,1,0>:w	{align1};
mov	(1)	INPUT_ARG0.8<1>:w	mbc_result.6<0,1,0>:w	{align1};
SAVE_RET	{align1};
jmpi	(1)	word_imedian; 
mov	(1)	mb_mvp_ref.2<1>:w		RET_ARG<0,1,0>:w	{align1};

__mb_hwdep_end:
asr	(2)	mb_ref_win.0<1>:w	mb_mvp_ref.0<2,2,1>:w	2:w	{align1};
add	(2)	mb_ref_win.8<1>:w	mb_ref_win.0<2,2,1>:w	3:w	{align1};
and	(2)	mb_ref_win.16<1>:uw	mb_ref_win.8<2,2,1>:uw	0xFFFC:uw {align1};

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


/* m1 */
mov  (8) vme_m1.0<1>:ud		0x0:ud	{align1};
and.z.f0.0 (1) null<1>:UW transform_8x8_ub<0,1,0>:UB 1:UW {align1};
(f0.0) mov  (1) intra_part_mask_ub<1>:UB  LUMA_INTRA_8x8_DISABLE:uw {align1};

/* assign MB intra struct from the thread payload*/
mov (1) mb_intra_struct_ub<1>:UB input_mb_intra_ub<0,1,0>:UB {align1}; 


/* M0 */
/* IME search */
cmp.z.f0.0 (1)		null<1>:uw	quality_level_ub<0,1,0>:ub		LOW_QUALITY_LEVEL:uw   {align1};
(f0.0) jmpi (1) __low_quality_search;

__high_quality_search:
mov  (1) vme_m0.12<1>:UD   SEARCH_CTRL_SINGLE + INTER_PART_MASK + INTER_SAD_HAAR + SUB_PEL_MODE_QUARTER:UD {align1};  
/* 16x16 Source, 1/4 pixel, harr */
mov  (1) vme_m0.22<1>:UW        REF_REGION_SIZE {align1};         /* Reference Width&Height, 48x40 */

mov  (1) vme_m0.0<1>:W		-16:W			{align1};
mov  (1) vme_m0.2<1>:W		-12:W			{align1};

and.z.f0.0 (1)		null:uw	input_mb_intra_ub<0,1,0>:ub	INTRA_PRED_AVAIL_FLAG_AE:uw   {align1};
(f0.0)	add 	(1)	vme_m0.0<1>:w	vme_m0.0<0,1,0>:w	12:w	{align1};
and.z.f0.0 (1)		null:uw	input_mb_intra_ub<0,1,0>:ub	INTRA_PRED_AVAIL_FLAG_B:uw   {align1};
(f0.0)	add 	(1)	vme_m0.2<1>:w	vme_m0.2<0,1,0>:w	8:w	{align1};

jmpi __vme_msg;

__low_quality_search:
mov  (1) vme_m0.12<1>:UD   SEARCH_CTRL_SINGLE + INTER_PART_MASK + INTER_SAD_HAAR + SUB_PEL_MODE_HALF:UD {align1};  
/* 16x16 Source, 1/2 pixel, harr */
mov  (1) vme_m0.22<1>:UW        MIN_REF_REGION_SIZE {align1};         /* Reference Width&Height, 32x32 */

mov  (1) vme_m0.0<1>:W		-8:W			{align1};
mov  (1) vme_m0.2<1>:W		-8:W			{align1};

and.z.f0.0 (1)		null:uw	input_mb_intra_ub<0,1,0>:ub	INTRA_PRED_AVAIL_FLAG_AE:uw   {align1};
(f0.0)	add 	(1)	vme_m0.0<1>:w	vme_m0.0<0,1,0>:w	4:w	{align1};
and.z.f0.0 (1)		null:uw	input_mb_intra_ub<0,1,0>:ub	INTRA_PRED_AVAIL_FLAG_B:uw   {align1};
(f0.0)	add 	(1)	vme_m0.2<1>:w	vme_m0.2<0,1,0>:w	4:w	{align1};

__vme_msg:
mov  (1) vme_m0.4<1>:UD		vme_m0.0<0,1,0>:UD	{align1};
add  (2) vme_m0.0<1>:w		vme_m0.0<2,2,1>:w	mb_ref_win.16<2,2,1>:w	{align1};
add  (2) vme_m0.4<1>:w		vme_m0.4<2,2,1>:w	mb_ref_win.16<2,2,1>:w	{align1};
mov  (8) vme_msg_0.0<1>:UD      vme_m0.0<8,8,1>:UD {align1};

/* m1 */

mov  (1) vme_m1.0<1>:UD         ADAPTIVE_SEARCH_ENABLE:ud {align1} ;
/* MV num is passed by constant buffer. R4.28 */
mov  (1) vme_m1.4<1>:UB		r4.28<0,1,0>:UB {align1};
add  (1) vme_m1.4<1>:UD         vme_m1.4<0,1,0>:UD	FB_PRUNING_DISABLE:UD {align1};
mov  (1) vme_m1.8<1>:UD         START_CENTER + SEARCH_PATH_LEN:UD {align1};

/* Set the MV cost center */
mov  (1) vme_m1.16<1>:ud	mb_mvp_ref.0<0,1,0>:ud	{align1};
mov  (1) vme_m1.20<1>:ud	mb_mvp_ref.0<0,1,0>:ud	{align1};
mov  (8) vme_msg_1.0<1>:UD      vme_m1.0<8,8,1>:UD {align1};


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

and.z.f0.0      (1)     null<1>:ud              vme_wb0.0<0,1,0>:ud     INTRAMBFLAG_MASK:ud {align1} ;

(-f0.0)jmpi     (1)     __INTRA_INFO ;

__INTER_INFO:
/* Write MV pairs */	
mov  (8) msg_reg0.0<1>:UD       obw_m0.0<8,8,1>:UD {align1};

mov  (8) msg_reg1.0<1>:UD       vme_wb1.0<8,8,1>:UD   {align1};

mov  (8) msg_reg2.0<1>:UD       vme_wb2.0<8,8,1>:UD   {align1};

mov  (8) msg_reg3.0<1>:UD       vme_wb3.0<8,8,1>:UD   {align1};

mov  (8) msg_reg4.0<1>:UD       vme_wb4.0<8,8,1>:UD   {align1};                
/* bind index 3, write  8 oword (128 bytes), msg type: 8(OWord Block Write) */
send (16)
        msg_ind
        obw_wb
        null
        data_port(
                OBW_CACHE_TYPE,
                OBW_MESSAGE_TYPE,
                OBW_CONTROL_8,
                OBW_BIND_IDX,
                OBW_WRITE_COMMIT_CATEGORY,
                OBW_HEADER_PRESENT
        )
        mlen 5
        rlen obw_wb_length
        {align1};

mov             (1)     tmp_uw1<1>:uw           0:uw {align1} ;
mov             (1)     tmp_ud1<1>:ud           0:ud {align1} ;
and     	(1)     tmp_uw1<1>:uw           vme_wb0.2<0,1,0>:uw     MV32_BIT_MASK:uw {align1} ;
shr       	(1)     tmp_uw1<1>:uw           tmp_uw1<1>:uw           MV32_BIT_SHIFT:uw {align1} ;
mul       	(1)     tmp_ud1<1>:ud           tmp_uw1<0,1,0>:uw       96:uw {align1} ;
add       	(1)     tmp_ud1<1>:ud           tmp_ud1<0,1,0>:ud       32:uw {align1} ;
shl       (1)     tmp_uw1<1>:uw           tmp_uw1<0,1,0>:uw       MFC_MV32_BIT_SHIFT:uw {align1} ;
add       (1)     tmp_uw1<1>:uw           tmp_uw1<0,1,0>:uw       MVSIZE_UW_BASE:uw {align1} ;
add             (1)     tmp_uw1<1>:uw           tmp_uw1<0,1,0>:uw       CBP_DC_YUV_UW:uw {align1} ;

mov             (1)     msg_reg1.0<1>:uw        vme_wb0.0<0,1,0>:uw     {align1} ;
mov             (1)     msg_reg1.2<1>:uw        tmp_uw1<0,1,0>:uw       {align1} ;
mov             (1)     msg_reg1.4<1>:UD        vme_wb0.28<0,1,0>:UD    {align1};
mov             (1)     msg_reg1.8<1>:ud        tmp_ud1<0,1,0>:ud       {align1} ;
mov             (1)     msg_reg1.12<1>:ud        vme_wb0.0<0,1,0>:ud     {align1} ;
mov             (1)     msg_reg1.16<1>:ud        0x25:ud     {align1} ;
jmpi		(1) 	__OUTPUT_INFO;

__INTRA_INFO:
mov             (1)     msg_reg1.0<1>:UD        vme_wb.0<0,1,0>:UD      {align1};
mov             (1)     msg_reg1.4<1>:UD        vme_wb.16<0,1,0>:UD     {align1};
mov             (1)     msg_reg1.8<1>:UD        vme_wb.20<0,1,0>:UD     {align1};
mov             (1)     msg_reg1.12<1>:UD       vme_wb.24<0,1,0>:UD     {align1};
mov             (1)     msg_reg1.16<1>:ud        0x35:ud     {align1} ;

__OUTPUT_INFO:

mov	(1)	msg_reg1.20<1>:ud	obw_m0.8<0,1,0>:ud	{align1};
add     (1)     obw_m0.8<1>:UD       obw_m0.8<0,1,0>:UD      INTER_VME_OUTPUT_MV_IN_OWS:UD {align1};
mov	(8)	msg_reg0.0<1>:ud	obw_m0.0<8,8,1>:ud	{align1};


/* bind index 3, write 1 oword, msg type: 8(OWord Block Write) */
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

/* Issue message fence so that the previous write message is committed */
send (16)
        mb_ind
        mb_wb.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_FENCE,
                OBR_MF_COMMIT,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 1
	{align1};

__EXIT: 
/*
 * kill thread
 */        
mov  (8) ts_msg_reg0<1>:UD         r0<8,8,1>:UD {align1};
send (16) ts_msg_ind acc0<1>UW null thread_spawner(0, 0, 1) mlen 1 rlen 0 {align1 EOT};


	nop		;
	nop		;
/* Compare three word data to get the min value */
word_imin:
	cmp.le.f0.0 (1)		null:w		INPUT_ARG0.0<0,1,0>:w	INPUT_ARG0.4<0,1,0>:w {align1};
	(f0.0) mov  (1)		TEMP_VAR0.0<1>:w INPUT_ARG0.0<0,1,0>:w			  {align1};
	(-f0.0) mov (1)		TEMP_VAR0.0<1>:w INPUT_ARG0.4<0,1,0>:w			  {align1};
	cmp.le.f0.0 (1)		null:w		TEMP_VAR0.0<0,1,0>:w	INPUT_ARG0.8<0,1,0>:w {align1};
	(f0.0) mov  (1)		RET_ARG<1>:w TEMP_VAR0.0<0,1,0>:w			  {align1};
	(-f0.0) mov (1)		RET_ARG<1>:w INPUT_ARG0.8<0,1,0>:w			  {align1};
	RETURN		{align1};	
	
/* Compare three word data to get the max value */
word_imax:
	cmp.ge.f0.0 (1)		null:w		INPUT_ARG0.0<0,1,0>:w	INPUT_ARG0.4<0,1,0>:w {align1};
	(f0.0) mov  (1)		TEMP_VAR0.0<1>:w INPUT_ARG0.0<0,1,0>:w			  {align1};
	(-f0.0) mov (1)		TEMP_VAR0.0<1>:w INPUT_ARG0.4<0,1,0>:w			  {align1};
	cmp.ge.f0.0 (1)		null:w		TEMP_VAR0.0<0,1,0>:w	INPUT_ARG0.8<0,1,0>:w {align1};
	(f0.0) mov  (1)		RET_ARG<1>:w TEMP_VAR0.0<0,1,0>:w			  {align1};
	(-f0.0) mov (1)		RET_ARG<1>:w INPUT_ARG0.8<0,1,0>:w			  {align1};
	RETURN		{align1};	
	
word_imedian:
	cmp.ge.f0.0 (1) null:w INPUT_ARG0.0<0,1,0>:w INPUT_ARG0.4<0,1,0>:w {align1};
	(f0.0)	jmpi (1) cmp_a_ge_b;
	cmp.ge.f0.0 (1) null:w INPUT_ARG0.0<0,1,0>:w INPUT_ARG0.8<0,1,0>:w {align1};
	(f0.0) mov (1) RET_ARG<1>:w INPUT_ARG0.0<0,1,0>:w {align1};
	(f0.0) jmpi (1) cmp_end;
	cmp.ge.f0.0 (1) null:w INPUT_ARG0.4<0,1,0>:w INPUT_ARG0.8<0,1,0>:w {align1};
	(f0.0) mov (1) RET_ARG<1>:w INPUT_ARG0.8<0,1,0>:w {align1};
	(-f0.0) mov (1) RET_ARG<1>:w INPUT_ARG0.4<0,1,0>:w {align1};
	jmpi (1) cmp_end;
cmp_a_ge_b:
	cmp.ge.f0.0 (1) null:w INPUT_ARG0.4<0,1,0>:w INPUT_ARG0.8<0,1,0>:w {align1};
	(f0.0) mov (1) RET_ARG<1>:w INPUT_ARG0.4<0,1,0>:w {align1};
	(f0.0) jmpi (1) cmp_end;
	cmp.ge.f0.0 (1) null:w INPUT_ARG0.0<0,1,0>:w INPUT_ARG0.8<0,1,0>:w {align1};
	(f0.0) mov (1) RET_ARG<1>:w INPUT_ARG0.8<0,1,0>:w {align1};
	(-f0.0) mov (1) RET_ARG<1>:w INPUT_ARG0.0<0,1,0>:w {align1};
cmp_end:
 	RETURN	{align1};

