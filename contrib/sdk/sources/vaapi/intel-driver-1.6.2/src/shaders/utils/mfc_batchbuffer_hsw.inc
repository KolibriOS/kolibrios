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

/* GRF registers
 * r0 header
 * r1~r4 constant buffer (reserved)
 * r5 inline data
 * r6~r7 reserved      
 * r8~r15 temporary registers
 * r16 write back of Oword Block Write
 */

/*
 * GRF 0 -- header       
 */        
define(`thread_id_ub',          `r0.20<0,1,0>:UB')  /* thread id in payload */

define(`inline_reg0',           `r5')
define(`buffer_offset',		`inline_reg0.0') /* :ud, in units of Owords */
/* :ub,
 * bit0 indicates the frame type. 1 is the I-frame. 0 is P-B frame
 */
define(`mb_flag',		`inline_reg0.4')
define(`qp_flag',		`inline_reg0.6') /* :ub */

define(`mb_x',                  `inline_reg0.8') /* :ub, */
define(`mb_y',                  `inline_reg0.9') /* :ub, */
define(`mb_xy',                 `inline_reg0.8') /* :uw, */      
/* :uw, the picture width in macroblocks */
define(`width_in_mbs',           `inline_reg0.10') 
/* :w, the number of macroblock commands being processed by the kernel */
define(`total_mbs',             `inline_reg0.12')
/* ub, the mb x/y of the last mb in slice */
define(`slice_end_x',           `inline_reg0.16')
define(`slice_end_y',           `inline_reg0.17')

/* :ud the forward reference picture list */
define(`fwd_ref',		`inline_reg0.20')
/* :ud the backward reference picture list */
define(`bwd_ref',		`inline_reg0.24')

/*
 * GRF 8~15 -- temporary registers
 */
define(`tmp_reg0',              `r8')
define(`tmp_reg1',              `r9')
define(`tmp_reg2',              `r10')
define(`tmp_reg3',              `r11')
define(`tmp_reg4',              `r12')
define(`tmp_reg5',              `r13')
define(`tmp_reg6',              `r14')
define(`tmp_reg7',              `r15')

define(`obw_m0',                `tmp_reg7')

define(`obw_wb',                `null<1>:W')
define(`obw_wb_length',         `0')

/*
 * GRF 26~27 
 */
define(`pak_object_reg0',     `r26')
define(`pak_object0_ud',      `r26.0')
define(`pak_object1_ud',      `r26.4')
define(`pak_object2_ud',      `r26.8')
define(`pak_object3_ud',      `r26.12')
define(`pak_object4_ud',      `r26.16')
define(`pak_object5_ud',      `r26.20')
define(`pak_object6_ud',      `r26.24')
define(`pak_object7_ud',      `r26.28')

define(`pak_object_reg1',     `r27')
define(`pak_object8_ud',      `r27.0')
define(`pak_object9_ud',      `r27.4')
define(`pak_object10_ud',     `r27.8')
define(`pak_object11_ud',     `r27.12')

/*
 * Message Payload registers
 */
define(`msg_ind',               `64')
define(`msg_reg0',              `g64')
define(`msg_reg1',              `g65')
define(`msg_reg2',              `g66')
define(`msg_reg3',              `g67')
define(`msg_reg4',              `g68')
define(`msg_reg5',              `g69')
define(`msg_reg6',              `g70')
define(`msg_reg7',              `g71')
define(`msg_reg8',              `g72')

define(`MV_BIND_IDX',           `0')
define(`MFC_BIND_IDX',          `2')

define(`ts_msg_ind',               `112')
define(`ts_msg_reg0',               `r112')


define(`MFC_AVC_PAK_OBJECT_DW0',  `0x7149000a')
define(`MFC_AVC_PAK_OBJECT_DW4',  `0xFFFF0000')        /* CBP for Y */
define(`MFC_AVC_PAK_OBJECT_DW5',  `0x000F000F')
define(`MFC_AVC_PAK_OBJECT_DW10', `0x0000000')

define(`OBR_MESSAGE_TYPE',              `0')
define(`OBR_CACHE_TYPE',                `10')

define(`OBR_MESSAGE_FENCE',              `7')
define(`OBR_MF_NOCOMMIT',		 `0')
define(`OBR_MF_COMMIT',			 `0x20')

define(`OBR_CONTROL_0',                 `0')    /* 1 OWord, low 128 bits */
define(`OBR_CONTROL_1',                 `1')    /* 1 OWord, high 128 bits */
define(`OBR_CONTROL_2',                 `2')    /* 2 OWords */
define(`OBR_CONTROL_4',                 `3')    /* 4 OWords */
define(`OBR_CONTROL_8',                 `4')    /* 8 OWords */

define(`OBR_HEADER_PRESENT',            `1')
define(`OBR_WRITE_COMMIT_CATEGORY',     `0')    /* category on Ivybridge */

define(`OBW_WRITE_COMMIT_CATEGORY',     `0')    /* category on Ivybridge */

define(`OBW_CACHE_TYPE',                `10')


define(`OBW_MESSAGE_TYPE',              `8')

define(`OBW_CONTROL_0',                 `0')    /* 1 OWord, low 128 bits */
define(`OBW_CONTROL_1',                 `1')    /* 1 OWord, high 128 bits */
define(`OBW_CONTROL_2',                 `2')    /* 2 OWords */
define(`OBW_CONTROL_4',                 `3')    /* 4 OWords */
define(`OBW_CONTROL_8',                 `4')    /* 8 OWords */
define(`OBW_HEADER_PRESENT',            `1')

define(`INTER_MASK',               	`0x03')
define(`INTER_16X16MODE',		`0x0')
define(`INTER_16X8MODE',		`0x01')
define(`INTER_8X16MODE',		`0x02')
define(`INTER_8X8MODE',			`0x03')
define(`SUBSHAPE_MASK',               	`0xFF00')

define(`mb_ind',                `90')
define(`mb_msg0',		`r90')
define(`mb_wb',			`r91')
define(`mb_intra_wb',		`r91')
define(`mb_inter_wb',		`r92')
define(`mb_mv0',		`r93')
define(`mb_mv1',		`r94')
define(`mb_mv2',		`r95')
define(`mb_mv3',		`r96')

define(`mb_temp',		`r86')
define(`cur_mb_x',              `mb_temp.0') /* :uw, */
define(`cur_mb_y',              `mb_temp.2') /* :uw, */
define(`cur_loop_count',        `mb_temp.4') /* :uw, */
define(`mb_end',		`r87')
define(`end_mb_x',              `mb_end.0') /* :uw, */
define(`end_mb_y',              `mb_end.2') /* :uw, */
define(`end_loop_count',        `mb_end.4') /* :uw, */
/* :ud the length of VME predict result for every mb. Units in owords */
define(`vme_len',               `mb_end.8') 
define(`mb_cur_msg',		`r88')

define(`INTRA_SLICE',		`0x0001')
define(`MFC_AVC_PAK_LAST_MB',	`0x0400')

define(`MFC_AVC_INTER_MASK_DW3',	`0x1F00FFFF')
define(`MFC_AVC_INTRA_MASK_DW3',	`0x0000C0FF')
define(`INTER_MV8',		`0x00400000')
define(`INTER_MV32',		`0x00600000')
define(`MFC_AVC_PAK_CBP',	`0x000E0000')
define(`MFC_AVC_INTRA_FLAG',	`0x00002000')
define(`AVC_INTRA_MASK',	`0x1F00')
