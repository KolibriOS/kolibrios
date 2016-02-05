/*
 * Copyright © 2010-2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Zhao Yakui <yakui.zhao@intel.com>
 */
        
START:
	mov	(16)	pak_object_reg0.0<1>:ud	0x0:ud		{align1}; 
	mov	(8)	obw_m0.0<1>:ud		0x0:ud		{align1}; 
	mov	(8)	mb_cur_msg.0<1>:ud	0x0:ud		{align1}; 
	mov     (16)	mb_temp.0<1>:ud		0x0:ud 		{align1}; 
	mov	(1)	cur_mb_x<1>:uw		mb_x<0,1,0>:ub	{align1};
	mov	(1)	cur_mb_y<1>:uw		mb_y<0,1,0>:ub	{align1};
	mov	(1)	end_mb_x<1>:uw	slice_end_x<0,1,0>:ub	{align1};
	mov	(1)	end_mb_y<1>:uw	slice_end_y<0,1,0>:ub	{align1};
	mov	(1)	end_loop_count<1>:uw	total_mbs<0,1,0>:uw	{align1};
	mov	(1)	vme_len<1>:ud		2:ud		{align1};
	and.z.f0.0	(1) null:uw	mb_flag<0,1,0>:ub	INTRA_SLICE:uw {align1};
	(f0.0)	mov	(1) vme_len<1>:ud	24:ud		{align1};

	mov  (1) obw_m0.8<1>:UD         buffer_offset<0,1,0>:ud {align1};
	mov  (1) obw_m0.20<1>:UB        thread_id_ub {align1};    /* dispatch id */

	mul  (1) mb_cur_msg.8<1>:UD       width_in_mbs<0,1,0>:UW   cur_mb_y<0,1,0>:UW {align1};
	add  (1) mb_cur_msg.8<1>:UD       mb_cur_msg.8<0,1,0>:UD   cur_mb_x<0,1,0>:uw {align1};
	mul  (1) mb_cur_msg.8<1>:UD       mb_cur_msg.8<0,1,0>:UD vme_len<0,1,0>:UD {align1};
	mov  (1) mb_cur_msg.20<1>:UB      thread_id_ub {align1};                  /* dispatch id */
	mov  (1) pak_object0_ud<1>:ud	   MFC_AVC_PAK_OBJECT_DW0:ud {align1};
	mov  (1) pak_object5_ud<1>:ud      MFC_AVC_PAK_OBJECT_DW5:ud {align1};
	mov  (1) pak_object10_ud<1>:ud     MFC_AVC_PAK_OBJECT_DW10:ud {align1};
	mov  (1) pak_object6_ud<1>:ub      qp_flag<0,1,0>:ub {align1};

pak_object_loop:
	mov	(8)	mb_msg0.0<1>:ud	 mb_cur_msg.0<8,8,1>:ud	{align1};
	mov     (1) 	pak_object4_ud<1>:ud MFC_AVC_PAK_OBJECT_DW4:ud {align1};
	mov	(1) 	tmp_reg0.0<1>:ub  cur_mb_x<0,1,0>:ub	{align1};
	mov	(1) 	tmp_reg0.1<1>:ub  cur_mb_y<0,1,0>:ub	{align1};
	mov     (1) 	pak_object4_ud<1>:uw tmp_reg0.0<0,1,0>:uw {align1};
	/* pak_object6_ud */
	mov	(1)	pak_object_reg0.26<1>:uw	0x0:uw	{align1};

	cmp.e.f0.0 (1)	null:uw	cur_mb_x<0,1,0>:uw end_mb_x<0,1,0>:uw	{align1};
	(-f0.0)	jmpi	(1) start_mb_flag;	
	cmp.e.f0.0 (1)	null:uw	cur_mb_y<0,1,0>:uw end_mb_y<0,1,0>:uw	{align1};
	(f0.0)	mov	(1)  pak_object_reg0.26<1>:uw MFC_AVC_PAK_LAST_MB:uw {align1};	
start_mb_flag:
	and.z.f0.0	(1) null:uw	mb_flag<0,1,0>:ub	INTRA_SLICE:uw {align1};
	(f0.0)	jmpi	(1) inter_frame_start;
	
/* bind index 0, read 2 oword (32bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_wb.0<1>:ud
        null
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_2,
                MV_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 1
        {align1};	
	jmpi (1) intra_pak_command;

nop;
nop;
inter_frame_start:
/* bind index 0, read 4 oword (64bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_wb.0<1>:ud
        null
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_4,
                MV_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 2
        {align1};
		
/* TODO: RefID is required after multi-references are added */
cmp.l.f0.0 (1)		null:w	mb_intra_wb.16<0,1,0>:uw	mb_inter_wb.8<0,1,0>:uw {align1};
(f0.0)   jmpi	(1)	intra_pak_command;

/* MV len and MV mode */	
	and	(1)   pak_object3_ud<1>:ud mb_inter_wb.0<0,1,0>:ud MFC_AVC_INTER_MASK_DW3:ud {align1};
	add	(1)   pak_object3_ud<1>:ud pak_object3_ud<0,1,0>:ud MFC_AVC_PAK_CBP:ud {align1}; 
	and	(1)   tmp_reg0.0<1>:uw   mb_inter_wb.0<0,1,0>:uw	INTER_MASK:uw	{align1};
	mov	(1)   pak_object1_ud<1>:ud	32:ud	{align1};
	cmp.e.f0.0 (1) null:uw	tmp_reg0.0<0,1,0>:uw	INTER_8X8MODE:uw	{align1};
	(-f0.0) add (1) pak_object3_ud<1>:ud  pak_object3_ud<0,1,0>:ud	INTER_MV8:ud {align1};
	(-f0.0) jmpi (1)	inter_mv_check;
	and.nz.f0.0 (1) null:ud  mb_inter_wb.4<0,1,0>:uw	SUBSHAPE_MASK:uw {align1};
	(f0.0)  mov  (1)	pak_object1_ud<1>:ud	128:ud	{align1};
	(f0.0)  add (1) pak_object3_ud<1>:ud  pak_object3_ud<0,1,0>:ud	INTER_MV32:ud {align1};
	(f0.0)  jmpi	(1) mv_check_end;

	add (1) pak_object3_ud<1>:ud  pak_object3_ud<0,1,0>:ud	INTER_MV8:ud {align1};
		
inter_mv_check:
	and	(1)   tmp_reg0.0<1>:uw   mb_inter_wb.0<0,1,0>:uw	INTER_MASK:uw	{align1};
	cmp.e.f0.0 (1) null:uw	tmp_reg0.0<0,1,0>:uw	INTER_16X16MODE:uw	{align1};
	(f0.0)  jmpi 	(1) mv_check_end;
	
add   (1) mb_msg0.8<1>:UD	mb_msg0.8<0,1,0>:ud	3:ud {align1};
/* Read MV for MB A */
/* bind index 0, read 8 oword (128bytes), msg type: 0(OWord Block Read) */
send (16)
        mb_ind
        mb_mv0.0<1>:ud
        null
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_TYPE,
                OBR_CONTROL_8,
                MV_BIND_IDX,
                OBR_WRITE_COMMIT_CATEGORY,
                OBR_HEADER_PRESENT
        )
        mlen 1
        rlen 4
        {align1};
/* TODO: RefID is required after multi-references are added */

	mov	(2)	mb_mv0.8<1>:ud	mb_mv1.0<2,2,1>:ud	{align1};
	mov	(2)	mb_mv0.16<1>:ud	mb_mv2.0<2,2,1>:ud	{align1};
	mov	(2)	mb_mv0.24<1>:ud	mb_mv3.0<2,2,1>:ud	{align1};

        mov             (8)     msg_reg0.0<1>:ud                mb_msg0.0<8,8,1>:ud {align1} ;
        mov             (8)     msg_reg1.0<1>:ud                mb_mv0.0<8,8,1>:ud {align1} ;
/* Write MV for MB A */
/* bind index 0, write 2 oword (32bytes), msg type: 8(OWord Block Write) */
send (16)
        msg_ind
        obw_wb
        null
        data_port(
                OBW_CACHE_TYPE,
                OBW_MESSAGE_TYPE,
                OBW_CONTROL_2,
                MV_BIND_IDX,
                OBW_WRITE_COMMIT_CATEGORY,
                OBW_HEADER_PRESENT
        )
        mlen 2
        rlen obw_wb_length
        {align1};

mv_check_end:

/* ref list */
	mov	(1)   pak_object8_ud<1>:ud fwd_ref<0,1,0>:ud	{align1};
	mov	(1)   pak_object9_ud<1>:ud bwd_ref<0,1,0>:ud	{align1};
/* inter_mode. pak_object7_ud */
	mov	(1)   pak_object7_ud<1>:ud	0x0:ud	{align1};
	mov	(1)   pak_object_reg0.28<1>:ub mb_inter_wb.5<0,1,0>:ub	{align1};
	mov	(1)   pak_object_reg0.29<1>:ub mb_inter_wb.6<0,1,0>:ub	{align1};

/* mv start address */
	add	(1)   tmp_reg0.4<1>:ud	mb_cur_msg.8<0,1,0>:ud	3:ud {align1};
	mul	(1)   pak_object2_ud<1>:ud tmp_reg0.4<0,1,0>:ud	16:ud {align1};	

        jmpi	(1)	write_pak_command;

intra_pak_command:
	/* object 1/2 is set to zero */
	mov	(2)   pak_object1_ud<1>:ud	0x0:ud	{align1};
	/* object 7/8 intra mode */
	mov	(1)   pak_object7_ud<1>:ud	mb_intra_wb.4<0,1,0>:ud	{align1};
	mov	(1)   pak_object8_ud<1>:ud	mb_intra_wb.8<0,1,0>:ud	{align1};
	/* object 9 Intra structure */
	mov	(1)   pak_object9_ud<1>:ud	0x0:ud			{align1};
	mov	(1)   pak_object9_ud<1>:ub	mb_intra_wb.12<0,1,0>:ub {align1};

	and	(1)   pak_object3_ud<1>:ud mb_intra_wb.0<0,1,0>:ud MFC_AVC_INTRA_MASK_DW3:ud {align1};
	add     (1)   pak_object3_ud<1>:ud pak_object3_ud<0,1,0>:ud MFC_AVC_INTRA_FLAG + MFC_AVC_PAK_CBP:ud {align1};

	mov	(1)   tmp_reg0.0<1>:ud	0:ud	{align1};
	mov	(1)   tmp_reg0.1<1>:ub	mb_intra_wb.2<0,1,0>:ub	{align1};
	and	(1)   tmp_reg0.0<1>:uw	tmp_reg0.0<0,1,0>:uw	AVC_INTRA_MASK:uw {align1};
	add	(1)   pak_object3_ud<1>:ud pak_object3_ud<0,1,0>:ud tmp_reg0.0<0,1,0>:ud {align1};

/* Write the pak command into the batchbuffer */
write_pak_command:
        mov             (8)     msg_reg0.0<1>:ud                obw_m0.0<8,8,1>:ud {align1} ;
        mov             (8)     msg_reg1.0<1>:ud                pak_object_reg0.0<8,8,1>:ud {align1} ;

/* bind index 3, write 2 oword (32bytes), msg type: 8(OWord Block Write) */
send (16)
        msg_ind
        obw_wb
        null
        data_port(
                OBW_CACHE_TYPE,
                OBW_MESSAGE_TYPE,
                OBW_CONTROL_2,
                MFC_BIND_IDX,
                OBW_WRITE_COMMIT_CATEGORY,
                OBW_HEADER_PRESENT
        )
        mlen 2
        rlen obw_wb_length
        {align1};

	add	(1)	msg_reg0.8<1>:ud	msg_reg0.8<0,1,0>:ud	2:ud	{align1};
	mov	(8)	msg_reg1.0<1>:ud	pak_object_reg1.0<8,8,1>:ud {align1};

/* bind index 3, write 1 oword (16bytes), msg type: 8(OWord Block Write) */
send (16)
        msg_ind
        obw_wb
        null
        data_port(
                OBW_CACHE_TYPE,
                OBW_MESSAGE_TYPE,
                OBW_CONTROL_0,
                MFC_BIND_IDX,
                OBW_WRITE_COMMIT_CATEGORY,
                OBW_HEADER_PRESENT
        )
        mlen 2
        rlen obw_wb_length
        {align1};


/* Check the next mb */
add	(1)	cur_loop_count<1>:uw	cur_loop_count<0,1,0>:uw	1:uw	{align1};
cmp.e.f0.0	(1)	null:uw	cur_loop_count<0,1,0>:uw end_loop_count<0,1,0>:uw {align1};
(f0.0)	jmpi	(1)	pak_loop_end;
/* the buffer offset for next block */
add     (1)	obw_m0.8<1>:ud		obw_m0.8<0,1,0>:ud	3:uw	{align1};
add	(1)     mb_cur_msg.8<1>:ud	mb_cur_msg.8<0,1,0>:ud	vme_len<0,1,0>:ud {align1};		
add	(1)	cur_mb_x<1>:uw		cur_mb_x<0,1,0>:uw	1:uw	{align1};
/* Check whether it is already equal to width in mbs */
cmp.e.f0.0	(1)	null:uw		cur_mb_x<0,1,0>:uw	width_in_mbs<0,1,0>:uw	{align1};
(f0.0)	add (1)	cur_mb_y<1>:uw		cur_mb_y<0,1,0>:uw	1:uw	{align1};
(f0.0)	mov	(1) cur_mb_x<1>:uw	0:uw		{align1};		

/* continue the pak command for next mb */
jmpi	(1)	pak_object_loop;
nop;
nop;
pak_loop_end:
/* Issue message fence so that the previous write message is committed */
send (16)
        msg_ind
        mb_wb.0<1>:ud
        null
        data_port(
                OBR_CACHE_TYPE,
                OBR_MESSAGE_FENCE,
                OBR_MF_COMMIT,
                MFC_BIND_IDX,
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
send (1) ts_msg_ind acc0<1>UW null thread_spawner(0, 0, 1) mlen 1 rlen 0 {align1 EOT};

nop;
        
