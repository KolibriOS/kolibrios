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
 *        Author : Zhao Yakui <yakui.zhao@intel.com>
 */
// Modual name: mpeg2_inter_haswell.asm
//
// Make MPEG2 inter predition estimation for Inter-frame on Haswell
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

shl  (2) vme_m0.8<1>:UW         orig_xy_ub<2,2,1>:UB 4:UW {align1};    /* (x, y) * 16 */
mov  (1) vme_m0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */

mul  (1) obw_m0.8<1>:UD         w_in_mb_uw<0,1,0>:UW orig_y_ub<0,1,0>:UB {align1};
add  (1) obw_m0.8<1>:UD         obw_m0.8<0,1,0>:UD orig_x_ub<0,1,0>:UB {align1};
mul  (1) obw_m0.8<1>:UD         obw_m0.8<0,1,0>:UD 24:UD {align1};
mov  (1) obw_m0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */
        
shl	(2) pic_ref.0<1>:uw	r4.24<2,2,1>:uw 4:uw	{align1};
mov	(2) pic_ref.16<1>:uw	r4.20<2,2,1>:uw	{align1};
mov  (8) mb_mvp_ref.0<1>:ud	0:ud		{align1};
mov  (8) mb_ref_win.0<1>:ud	0:ud		{align1};
mov  (8) mba_result.0<1>:ud	0x0:ud		{align1};
mov  (8) mbb_result.0<1>:ud	0x0:ud		{align1};
mov  (8) mbc_result.0<1>:ud	0x0:ud		{align1};

and.z.f0.0 (1)		null:uw	mb_hwdep<0,1,0>:uw		0x04:uw   {align1};
(f0.0) jmpi (1) __mb_hwdep_end;
/* read back the data for MB A */
/* the layout of MB result is: rx.0(Available). rx.4(MVa), rX.8(MVb), rX.16(Pred_L0 flag),
*  rX.18 (Pred_L1 flag), rX.20(Forward reference ID), rX.22(Backwared reference ID)
*/
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
mul  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD 24:UD {align1};
mov  (1) mb_msg0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */

/* bind index 3, read 4 oword (64bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_wb.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_4,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 2
        {align1};

/* TODO: RefID is required after multi-references are added */
cmp.l.f0.0 (1)		null:w	mb_intra_wb.16<0,1,0>:uw	mb_inter_wb.8<0,1,0>:uw {align1};
(f0.0)   mov (2)	mba_result.20<1>:w			-1:w	{align1};
(f0.0)   jmpi	(1)	mbb_start;

add   (1) mb_msg0.8<1>:UD	mb_msg0.8<0,1,0>:ud	3:ud {align1};
/* Read MV for MB A */
/* bind index 3, read 2 oword (16bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_mv0.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_2,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 1
        {align1};
/* TODO: RefID is required after multi-references are added */
/* MV */
mov	   (2)		mba_result.4<1>:ud		mb_mv0.0<2,2,1>:ud	{align1};
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
mul  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD 24:UD {align1};
mov  (1) mb_msg0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */

/* bind index 3, read 4 oword (64bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_wb.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_4,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 2
        {align1};

/* TODO: RefID is required after multi-references are added */
cmp.l.f0.0 (1)		null:w	mb_intra_wb.16<0,1,0>:uw	mb_inter_wb.8<0,1,0>:uw {align1};
(f0.0)   mov (2)	mbb_result.20<1>:w			-1:w	{align1};
(f0.0)   jmpi	(1)	mbc_start;
add   (1) mb_msg0.8<1>:UD	mb_msg0.8<0,1,0>:ud	3:ud {align1};
/* Read MV for MB B */
/* bind index 3, read 8 oword (128bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_mv0.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_2,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 1
        {align1};
/* TODO: RefID is required after multi-references are added */
mov	   (2)		mbb_result.4<1>:ud		mb_mv0.0<2,2,1>:ud	{align1};
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
mul  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD 24:UD {align1};
mov  (1) mb_msg0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */

/* bind index 3, read 4 oword (64bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_wb.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_4,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 2
        {align1};

/* TODO: RefID is required after multi-references are added */
cmp.l.f0.0 (1)		null:w	mb_intra_wb.16<0,1,0>:uw	mb_inter_wb.8<0,1,0>:uw {align1};
(f0.0)   mov (2)	mbc_result.20<1>:w			-1:w	{align1};
(f0.0)   jmpi	(1)	mb_mvp_start;
add   (1) mb_msg0.8<1>:UD	mb_msg0.8<0,1,0>:ud	3:ud {align1};
/* Read MV for MB C */
/* bind index 3, read 8 oword (128bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_mv0.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_2,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 1
        {align1};
/* TODO: RefID is required after multi-references are added */
/* Forward MV */
mov	   (2)		mbc_result.4<1>:ud		mb_mv0.0<2,2,1>:ud	{align1};
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
mul  (1) mb_msg0.8<1>:UD       mb_msg0.8<0,1,0>:UD 24:UD {align1};
mov  (1) mb_msg0.20<1>:UB        thread_id_ub {align1};                  /* dispatch id */

/* bind index 3, read 4 oword (64bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_wb.0<1>:ud
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_4,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 2
        {align1};

cmp.l.f0.0 (1)		null:w	mb_intra_wb.16<0,1,0>:uw	mb_inter_wb.8<0,1,0>:uw {align1};
(f0.0)   mov (2)	mbc_result.20<1>:w			-1:w	{align1};
(f0.0)   jmpi	(1)	mb_mvp_start;

add   (1) mb_msg0.8<1>:UD	mb_msg0.8<0,1,0>:ud	3:ud {align1};
/* Read MV for MB D */
/* bind index 3, read 8 oword (128bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_mv0.0<1>:ub
	NULL
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_2,
                OBR_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 1
        {align1};

/* TODO: RefID is required after multi-references are added */

/* Forward MV */
mov	   (2)		mbc_result.4<1>:ud		mb_mv0.0<2,2,1>:ud	{align1};
mov	   (1)		mbc_result.16<1>:w		MB_PRED_FLAG		{align1};
	
mb_mvp_start:
/*TODO: Add the skip prediction */
/* Check whether both MB B and C are inavailable */
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
        
mov     (2)     mv_cc_ref.0<1>:w	mba_result.4<2,2,1>:w	{align1};

/* Calibrate the ref window for MPEG2 */
mov  (1) vme_m0.0<1>:W		-16:W			{align1};
mov  (1) vme_m0.2<1>:W		-12:W			{align1};

mov  (1) INPUT_ARG0.0<1>:ud	vme_m0.0<0,1,0>:ud	{align1};
mov  (1) INPUT_ARG0.8<1>:ud	vme_m0.8<0,1,0>:ud	{align1};
mov  (8) INPUT_ARG1.0<1>:ud	pic_ref.0<8,8,1>:ud	{align1};

SAVE_RET	{align1};
jmpi	(1)	ref_boundary_check;
mov  (2) vme_m0.0<1>:w		RET_ARG<2,2,1>:w	{align1};

/* m2, get the MV/Mb cost passed from constant buffer when
spawning thread by MEDIA_OBJECT */       
mov (8) vme_m2<1>:UD            r1.0<8,8,1>:UD {align1};

mov (8) vme_msg_2<1>:UD		vme_m2.0<8,8,1>:UD {align1};

/* m3 */
mov (8) vme_msg_3<1>:UD		0x0:UD {align1};	        

/* the neighbour pixel is zero for MPEG2 Intra-prediction */

/* m4 */
mov  (8) vme_msg_4<1>:UD         0:UD {align1};
mov  (1) tmp_reg0.0<1>:UW	LUMA_INTRA_MODE:UW {align1};
/* Use the Luma mode */
mov  (1) vme_msg_4.5<1>:UB	tmp_reg0.0<0,1,0>:UB {align1};
mov  (1) tmp_reg0.0<1>:UW	INTRA16_DC_PRED:UW {align1};
mov  (1) vme_msg_4.4<1>:ub	tmp_reg0.0<0,1,0>:UB {align1};

/* m5 */        
mov  (8) vme_msg_5<1>:UD         0x0:UD {align1};
mov  (1) vme_msg_5.16<1>:UD      INTRA_PREDICTORE_MODE {align1};

/* the penalty for Intra mode */
mov  (1) vme_msg_5.28<1>:UD	0x010101:UD {align1};


/* m6 */
mov  (8) vme_msg_6.0<1>:UD       0:Ud {align1};

/*
 * SIC VME message
 */
/* m0 */        
mov  (8) vme_msg_0.0<1>:UD      vme_m0.0<8,8,1>:UD {align1};

/* Disable Intra8x8/Intra4x4 Intra-prediction */
/* m1 */
mov  (8) vme_m1.0<1>:ud		0x0:UD	{align1};

mov  (1) intra_flag<1>:UW       0x0:UW {align1}                     ;
mov   (1) tmp_reg0.0<1>:uw	LUMA_INTRA_8x8_DISABLE:uw		{align1};
add   (1) tmp_reg0.0<1>:uw	tmp_reg0.0<0,1,0>:uw	LUMA_INTRA_4x4_DISABLE:uw {align1};
mov  (1) intra_part_mask_ub<1>:UB  tmp_reg0.0<0,1,0>:ub {align1};

/* assign MB intra struct from the thread payload*/
mov (1) mb_intra_struct_ub<1>:UB input_mb_intra_ub<0,1,0>:UB {align1}; 

/* Enable DC HAAR component when calculating HARR SATD block */
mov  (1) tmp_reg0.0<1>:UW	DC_HARR_ENABLE:UW		{align1};
mov  (1) vme_m1.30<1>:UB	tmp_reg0.0<0,1,0>:UB  {align1};

mov  (1) vme_m0.12<1>:UD        INTRA_SAD_HAAR:UD {align1};    /* 16x16 Source, Intra_harr */
/* m0 */        
mov  (8) vme_msg_0.0<1>:UD      vme_m0.0<8,8,1>:UD {align1};
mov  (8) vme_msg_1<1>:UD        vme_m1.0<8,8,1>:UD {align1};

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

/* IME search */
mov  (1) vme_m0.12<1>:UD        SEARCH_CTRL_SINGLE + INTER_PART_MASK + INTER_SAD_HAAR:UD {align1};    /* 16x16 Source, harr */
mov  (1) vme_m0.22<1>:UW        REF_REGION_SIZE {align1};         /* Reference Width&Height, 48x40 */

mov  (1) vme_m0.4<1>:UD		vme_m0.0<0,1,0>:UD	{align1};
 
mov  (8) vme_msg_0.0<1>:UD      vme_m0.0<8,8,1>:UD {align1};

mov  (1) vme_m1.0<1>:UD         ADAPTIVE_SEARCH_ENABLE:ud {align1} ;
/* the Max MV number is passed by constant buffer */
mov  (1) vme_m1.4<1>:UB         r4.28<0,1,0>:UB {align1};          
mov  (1) vme_m1.8<1>:UD         START_CENTER + SEARCH_PATH_LEN:UD {align1};
/* Set the MV cost center */ 
mov  (1) vme_m1.16<1>:ud	mv_cc_ref.0<0,1,0>:ud	{align1};
mov  (1) vme_m1.20<1>:ud	mv_cc_ref.0<0,1,0>:ud	{align1};

mov  (8) vme_msg_1.0<1>:UD      vme_m1.0<8,8,1>:UD {align1};

mov (8) vme_msg_2<1>:UD		vme_m2.0<8,8,1>:UD {align1};
/* M3/M4 search path */

mov  (1) vme_msg_3.0<1>:UD	0x01010101:UD {align1};
mov  (1) vme_msg_3.4<1>:UD	0x10010101:UD {align1};
mov  (1) vme_msg_3.8<1>:UD	0x0F0F0F0F:UD {align1};
mov  (1) vme_msg_3.12<1>:UD	0x100F0F0F:UD {align1};
mov  (1) vme_msg_3.16<1>:UD	0x01010101:UD {align1};
mov  (1) vme_msg_3.20<1>:UD	0x10010101:UD {align1};
mov  (1) vme_msg_3.24<1>:UD	0x0F0F0F0F:UD {align1};
mov  (1) vme_msg_3.28<1>:UD	0x100F0F0F:UD {align1};

mov  (1) vme_msg_4.0<1>:UD	0x01010101:UD {align1};
mov  (1) vme_msg_4.4<1>:UD	0x10010101:UD {align1};
mov  (1) vme_msg_4.8<1>:UD	0x0F0F0F0F:UD {align1};
mov  (1) vme_msg_4.12<1>:UD	0x000F0F0F:UD {align1};

mov  (4) vme_msg_4.16<1>:UD	0x0:UD {align1};

send (8)
        vme_msg_ind
        vme_wb<1>:UD
        null
        vme(
                BIND_IDX_VME,
                0,
                0,
                VME_IME_MESSAGE_TYPE
        )
        mlen ime_vme_msg_length
        rlen vme_wb_length {align1};

/* Set Macroblock-shape/mode for FBR */

mov  (1) vme_m2.20<1>:UD	0x0:UD {align1};
mov  (1) vme_m2.21<1>:UB	vme_wb.25<0,1,0>:UB	{align1};
mov  (1) vme_m2.22<1>:UB	vme_wb.26<0,1,0>:UB	{align1};

and  (1) tmp_reg0.0<1>:UW	vme_wb.0<0,1,0>:UW	0x03:UW {align1};
mov  (1) vme_m2.20<1>:UB	tmp_reg0.0<0,1,0>:UB    {align1};

/* Send FBR message into CRE */

mov  (8) vme_msg_3.0<1>:UD       vme_wb1.0<8,8,1>:UD {align1};
mov  (8) vme_msg_4.0<1>:ud       vme_wb2.0<8,8,1>:ud {align1};
mov  (8) vme_msg_5.0<1>:ud       vme_wb3.0<8,8,1>:ud {align1};
mov  (8) vme_msg_6.0<1>:ud       vme_wb4.0<8,8,1>:ud {align1};                

mov  (1) vme_m0.12<1>:UD	INTER_SAD_HAAR + SUB_PEL_MODE_HALF + FBR_BME_DISABLE:UD {align1};    /* 16x16 Source, 1/2 pixel, harr, BME disable */

/* Bilinear filter */
mov  (1) tmp_reg0.0<1>:uw	0x04:uw	{align1};
add  (1) vme_m1.30<1>:ub	vme_m1.30<0,1,0>:ub	tmp_reg0.0<0,1,0>:ub	{align1};

mov  (8) vme_msg_0.0<1>:UD	vme_m0.0<8,8,1>:UD  {align1};
mov  (8) vme_msg_1.0<1>:UD	vme_m1.0<8,8,1>:UD  {align1};

mov  (8) vme_msg_2.0<1>:UD		vme_m2.0<8,8,1>:UD	{align1};

/* after verification it will be passed by using payload */
send (8)
        vme_msg_ind
        vme_wb<1>:UD
        null
        cre(
                BIND_IDX_VME,
                VME_FBR_MESSAGE_TYPE
        )
        mlen fbr_vme_msg_length
        rlen vme_wb_length
        {align1};

and.z.f0.0 (1)		null:uw	mb_hwdep<0,1,0>:uw		0x04:uw   {align1};
(-f0.0) jmpi (1) vme_run_again;
nop;
vme_mv_output:

add  (1) obw_m0.8<1>:UD         obw_m0.8<0,1,0>:UD 0x02:UD {align1};
mov  (8) msg_reg0.0<1>:UD       obw_m0<8,8,1>:UD {align1};
/* write FME info */
mov  (1) msg_reg1.0<1>:UD       vme_wb.0<0,1,0>:UD      {align1};

mov  (1) msg_reg1.4<1>:UD       vme_wb.24<0,1,0>:UD     {align1};
/* Inter distortion of FME */
mov  (1) msg_reg1.8<1>:UD       vme_wb.8<0,1,0>:UD     {align1};

mov  (1) msg_reg1.12<1>:UD	vme_m2.20<0,1,0>:UD {align1};

/* bind index 3, write  oword (16bytes), msg type: 8(OWord Block Write) */
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


/* Write FME/BME MV */
add  (1) obw_m0.8<1>:UD         obw_m0.8<0,1,0>:UD 0x01:UD {align1};
mov  (8) msg_reg0.0<1>:UD       obw_m0.0<8,8,1>:UD {align1};


mov  (8) msg_reg1.0<1>:UD       vme_wb1.0<8,8,1>:UD {align1};
mov  (8) msg_reg2.0<1>:ud       vme_wb2.0<8,8,1>:ud {align1};
mov  (8) msg_reg3.0<1>:ud       vme_wb3.0<8,8,1>:ud {align1};
mov  (8) msg_reg4.0<1>:ud       vme_wb4.0<8,8,1>:ud {align1};                
/* bind index 3, write  8 oword (128 bytes), msg type: 8(OWord Block Write) */
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

/* Write FME/BME RefID */
add  (1) obw_m0.8<1>:UD         obw_m0.8<0,1,0>:UD 0x08:UD {align1};
mov  (8) msg_reg0.0<1>:UD       obw_m0<8,8,1>:UD {align1};

mov  (8) msg_reg1.0<1>:UD	vme_wb6.0<8,8,1>:UD {align1};

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

nop;
nop;

ref_boundary_check:

/* The left/up coordinate of reference window */
add  (2) TEMP_VAR0.0<1>:w	INPUT_ARG0.8<2,2,1>:w	INPUT_ARG0.0<2,2,1>:w	{align1};
/* The right/bottom coordinate of reference window */
add  (1) TEMP_VAR0.16<1>:w	TEMP_VAR0.0<0,1,0>:w	48:w			{align1};
add  (1) TEMP_VAR0.18<1>:w	TEMP_VAR0.2<0,1,0>:w	40:w			{align1};

/* Firstly the MV range is checked */
mul  (2) TEMP_VAR1.16<1>:w	INPUT_ARG1.16<2,2,1>:w	-1:w	{align1};
add  (2) TEMP_VAR1.0<1>:w	INPUT_ARG0.8<2,2,1>:w	TEMP_VAR1.16<2,2,1>:w	{align1};
add  (2) TEMP_VAR1.4<1>:w	INPUT_ARG0.8<2,2,1>:w	INPUT_ARG1.16<2,2,1>:w	{align1};

cmp.l.f0.0 (1)	null:w	TEMP_VAR0.0<0,1,0>:w	TEMP_VAR1.0<0,1,0>:w	{align1};
(f0.0)	mov	(1) TEMP_VAR0.0<1>:w	TEMP_VAR1.0<0,1,0>:w	{align1};
cmp.g.f0.0 (1)	null:w	TEMP_VAR0.16<0,1,0>:w	TEMP_VAR1.4<0,1,0>:w	{align1};
(f0.0)	add	(1) TEMP_VAR0.0<1>:w	TEMP_VAR1.4<0,1,0>:w	-48:w	{align1};
cmp.l.f0.0 (1)	null:w	TEMP_VAR0.2<0,1,0>:w	TEMP_VAR1.2<0,1,0>:w	{align1};
(f0.0)	mov	(1) TEMP_VAR0.2<1>:w	TEMP_VAR1.2<0,1,0>:w	{align1};
cmp.g.f0.0 (1)	null:w	TEMP_VAR0.18<0,1,0>:w	TEMP_VAR1.6<0,1,0>:w	{align1};
(f0.0)	add	(1) TEMP_VAR0.2<1>:w	TEMP_VAR1.6<0,1,0>:w	-40:w	{align1};

x_left_cmp:
	cmp.l.f0.0 (1)		null:w		TEMP_VAR0.0<0,1,0>:w	0:w	{align1};
	(-f0.0)	jmpi	(1)	x_right_cmp;
	(f0.0)	mov	(1)	TEMP_VAR0.0<1>:w	0:w		{align1};
	jmpi	(1)	y_top_cmp;
x_right_cmp:
	cmp.g.f0.0 (1)	null:w	TEMP_VAR0.16<0,1,0>:w	INPUT_ARG1.0<0,1,0>:w	{align1};
	(-f0.0)	jmpi	(1)	y_top_cmp;	
	(f0.0)	add	(1)	TEMP_VAR0.0<1>:w	INPUT_ARG1.0<0,1,0>:w	-48:w	{align1};
y_top_cmp:
	cmp.l.f0.0  (1)		null:w	TEMP_VAR0.2<0,1,0>:w	0:w		{align1};
	(-f0.0)	jmpi	(1)	y_bottom_cmp;
	(f0.0)	mov	(1)	TEMP_VAR0.2<1>:w	0:w		{align1};
	jmpi	(1)	y_bottom_end;
y_bottom_cmp:
	cmp.g.f0.0  (1)		null:w	TEMP_VAR0.18<0,1,0>:w	INPUT_ARG1.2<0,1,0>:w {align1};
	(f0.0)	add	(1)	TEMP_VAR0.2<1>:w		INPUT_ARG1.2<0,1,0>:w	-40:w	{align1};

y_bottom_end:
mul	(2)	TEMP_VAR1.0<1>:w	INPUT_ARG0.8<2,2,1>:w	-1:w	{align1};
add	(2)	RET_ARG<1>:w		TEMP_VAR0.0<2,2,1>:w	TEMP_VAR1.0<2,2,1>:w	{align1};	
 	RETURN	{align1};
nop;
nop;

vme_run_again:

asr	(2)	mb_ref_win.0<1>:w	mb_mvp_ref.0<2,2,1>:w	2:w	{align1};
mov	(2)	tmp_reg0.0<1>:w		mb_ref_win.0<2,2,1>:w		{align1};
add	(2)	mb_ref_win.8<1>:w	mb_ref_win.0<2,2,1>:w	3:w	{align1};
and	(2)	mb_ref_win.16<1>:uw	mb_ref_win.8<2,2,1>:uw	0xFFFC:uw {align1};

cmp.l.f0.0	(1) null:w	tmp_reg0.0<0,1,0>:w	0:w	{align1};
(f0.0)	mul	(1) tmp_reg0.0<1>:w	tmp_reg0.0<0,1,0>:w	-1:w	{align1};
cmp.l.f0.0	(1) null:w	tmp_reg0.2<0,1,0>:w	0:w	{align1};
(f0.0)	mul	(1) tmp_reg0.2<1>:w	tmp_reg0.2<0,1,0>:w	-1:w	{align1};

cmp.ge.f0.0	(1) null:w	tmp_reg0.0<0,1,0>:w	4:w	{align1};
(f0.0)	jmpi (1)	vme_start;
cmp.ge.f0.0	(1) null:w	tmp_reg0.2<0,1,0>:w	4:w	{align1};
(f0.0)	jmpi (1)	vme_start;

jmpi (1) vme_done;

vme_start:
	mov (8)	tmp_vme_wb0.0<1>:ud	vme_wb0.0<8,8,1>:ud	{align1};
	mov (8)	tmp_vme_wb1.0<1>:ud	vme_wb1.0<8,8,1>:ud	{align1};

/* Calibrate the ref window for MPEG2 */
mov  (1) vme_m0.0<1>:W		-16:W			{align1};
mov  (1) vme_m0.2<1>:W		-12:W			{align1};
mov  (4) INPUT_ARG0.0<1>:ud	vme_m0.0<4,4,1>:ud	{align1};
add  (2) INPUT_ARG0.0<1>:w	INPUT_ARG0.0<2,2,1>:w	mb_ref_win.16<2,2,1>:w	{align1};
mov  (8) INPUT_ARG1.0<1>:ud	pic_ref.0<8,8,1>:ud	{align1};

SAVE_RET	{align1};
jmpi	(1)	ref_boundary_check;
mov  (2) vme_m0.0<1>:w		RET_ARG<2,2,1>:w	{align1};

/* IME search */
mov  (1) vme_m0.12<1>:UD        SEARCH_CTRL_SINGLE + INTER_PART_MASK + INTER_SAD_HAAR:UD {align1};    /* 16x16 Source, harr */
mov  (1) vme_m0.22<1>:UW        REF_REGION_SIZE {align1};         /* Reference Width&Height, 48x40 */

mov  (1) vme_m0.4<1>:UD		vme_m0.0<0,1,0>:UD	{align1};
 
mov  (8) vme_msg_0.0<1>:UD      vme_m0.0<8,8,1>:UD {align1};

mov  (8) vme_m1.0<1>:ud		0x0:UD	{align1};

mov  (1) vme_m1.0<1>:UD         ADAPTIVE_SEARCH_ENABLE:ud {align1} ;
/* the Max MV number is passed by constant buffer */
mov  (1) vme_m1.4<1>:UB         r4.28<0,1,0>:UB {align1};          
mov  (1) vme_m1.8<1>:UD         START_CENTER + SEARCH_PATH_LEN:UD {align1};
/* Set the MV cost center */ 
mov  (1) vme_m1.16<1>:ud	mv_cc_ref.0<0,1,0>:ud	{align1};
mov  (1) vme_m1.20<1>:ud	mv_cc_ref.0<0,1,0>:ud	{align1};
mov  (8) vme_msg_1.0<1>:UD      vme_m1.0<8,8,1>:UD {align1};

mov (8) vme_msg_2<1>:UD		vme_m2.0<8,8,1>:UD {align1};
/* M3/M4 search path */

mov  (1) vme_msg_3.0<1>:UD	0x01010101:UD {align1};
mov  (1) vme_msg_3.4<1>:UD	0x10010101:UD {align1};
mov  (1) vme_msg_3.8<1>:UD	0x0F0F0F0F:UD {align1};
mov  (1) vme_msg_3.12<1>:UD	0x100F0F0F:UD {align1};
mov  (1) vme_msg_3.16<1>:UD	0x01010101:UD {align1};
mov  (1) vme_msg_3.20<1>:UD	0x10010101:UD {align1};
mov  (1) vme_msg_3.24<1>:UD	0x0F0F0F0F:UD {align1};
mov  (1) vme_msg_3.28<1>:UD	0x100F0F0F:UD {align1};

mov  (1) vme_msg_4.0<1>:UD	0x01010101:UD {align1};
mov  (1) vme_msg_4.4<1>:UD	0x10010101:UD {align1};
mov  (1) vme_msg_4.8<1>:UD	0x0F0F0F0F:UD {align1};
mov  (1) vme_msg_4.12<1>:UD	0x000F0F0F:UD {align1};

mov  (4) vme_msg_4.16<1>:UD	0x0:UD {align1};

send (8)
        vme_msg_ind
        vme_wb<1>:UD
        null
        vme(
                BIND_IDX_VME,
                0,
                0,
                VME_IME_MESSAGE_TYPE
        )
        mlen ime_vme_msg_length
        rlen vme_wb_length {align1};

/* Set Macroblock-shape/mode for FBR */

mov  (1) vme_m2.20<1>:UD	0x0:UD {align1};
mov  (1) vme_m2.21<1>:UB	vme_wb.25<0,1,0>:UB	{align1};
mov  (1) vme_m2.22<1>:UB	vme_wb.26<0,1,0>:UB	{align1};

and  (1) tmp_reg0.0<1>:UW	vme_wb.0<0,1,0>:UW	0x03:UW {align1};
mov  (1) vme_m2.20<1>:UB	tmp_reg0.0<0,1,0>:UB    {align1};

/* Send FBR message into CRE */

mov  (8) vme_msg_3.0<1>:UD       vme_wb1.0<8,8,1>:UD {align1};
mov  (8) vme_msg_4.0<1>:ud       vme_wb2.0<8,8,1>:ud {align1};
mov  (8) vme_msg_5.0<1>:ud       vme_wb3.0<8,8,1>:ud {align1};
mov  (8) vme_msg_6.0<1>:ud       vme_wb4.0<8,8,1>:ud {align1};                

mov  (1) vme_m0.12<1>:UD	INTER_SAD_HAAR + SUB_PEL_MODE_HALF + FBR_BME_DISABLE:UD {align1};    /* 16x16 Source, 1/2 pixel, harr, BME disable */

/* Bilinear filter */
mov  (1) tmp_reg0.0<1>:uw	0x04:uw	{align1};
add  (1) vme_m1.30<1>:ub	vme_m1.30<0,1,0>:ub	tmp_reg0.0<0,1,0>:ub	{align1};

mov  (8) vme_msg_0.0<1>:UD	vme_m0.0<8,8,1>:UD  {align1};
mov  (8) vme_msg_1.0<1>:UD	vme_m1.0<8,8,1>:UD  {align1};

mov  (8) vme_msg_2.0<1>:UD		vme_m2.0<8,8,1>:UD	{align1};

/* after verification it will be passed by using payload */
send (8)
        vme_msg_ind
        vme_wb<1>:UD
        null
        cre(
                BIND_IDX_VME,
                VME_FBR_MESSAGE_TYPE
        )
        mlen fbr_vme_msg_length
        rlen vme_wb_length
        {align1};

cmp.l.f0.0 (1)	null:uw	vme_wb0.8<0,1,0>:uw	tmp_vme_wb0.8<0,1,0>:uw	{align1};
(f0.0)	jmpi (1) vme_done;
mov	(8)	vme_wb0.0<1>:ud	tmp_vme_wb0.0<8,8,1>:ud	{align1};
mov	(8)	vme_wb1.0<1>:ud	tmp_vme_wb1.0<8,8,1>:ud	{align1};

vme_done:
       jmpi (1) vme_mv_output;
nop;
nop;
nop;

