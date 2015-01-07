/*
 * Copyright © 2012 Intel Corporation
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
 *    Xiang Haihao <haihao.xiang@intel.com>
 */

define(`BIND_IDX_VME',                  `0')
define(`BIND_IDX_VME_REF0',             `1')
define(`BIND_IDX_VME_REF1',             `2')
define(`BIND_IDX_OUTPUT',               `3')
define(`BIND_IDX_INEP',                 `4')
define(`BIND_IDX_VME_BATCHBUFFER',      `5')

#ifdef DEV_SNB

define(`OBW_CACHE_TYPE',                `5')

#else

define(`OBW_CACHE_TYPE',                `10')

#endif

define(`OBW_MESSAGE_TYPE',              `8')

define(`OBW_BIND_IDX',                  `BIND_IDX_VME_BATCHBUFFER')

define(`OBW_CONTROL_0',                 `0')    /* 1 OWord, low 128 bits */
define(`OBW_CONTROL_1',                 `1')    /* 1 OWord, high 128 bits */
define(`OBW_CONTROL_2',                 `2')    /* 2 OWords */
define(`OBW_CONTROL_3',                 `3')    /* 4 OWords */
define(`OBW_CONTROL_4',                 `4')    /* 8 OWords */

#ifdef DEV_SNB

define(`OBW_WRITE_COMMIT_CATEGORY',     `1')    /* write commit on Sandybrige */

#else

define(`OBW_WRITE_COMMIT_CATEGORY',     `0')    /* category on Ivybridge */

#endif

define(`OBW_HEADER_PRESENT',            `1')

define(`CMD_MEDIA_OBJECT',              `0x71000006:UD')
define(`MI_BATCH_BUFFER_END',           `0x05000000:UD')

define(`NUM_MACROBLOCKS_PER_COMMAND',   `512')

/* GRF registers
 * r0 header
 * r1~r4 constant buffer (reserved)
 * r5 inline data
 * r6~r7 reserved      
 * r8~r15 temporary registers
 * r16 media object command
 * r17 
 * r18 write back of Oword Block Write
 */
/*
 * GRF 0 -- header       
 */        
define(`thread_id_ub',          `r0.20<0,1,0>:UB')  /* thread id in payload */

/*
 * GRF 1~4 -- Constant Buffer (reserved)
 */
        
/*
 * GRF 5 -- inline data
 */        
define(`inline_reg0',           `r5')
define(`total_mbs',             `inline_reg0.0') /* the number of macroblocks in a picture */
define(`transform_8x8_ub',      `inline_reg0.4') /* transform_8x8 flag */
define(`mtype_ub',              `inline_reg0.5') /* 0: INTRA, 1: INTER */
define(`width_in_mb',           `inline_reg0.6') /* the picture width in macroblocks */

/*
 * GRF 8~15 -- temporary registers
 */
define(`tmp_reg0',              `r8')
define(`obw_header',            `tmp_reg0')
define(`tmp_reg1',              `r9')
define(`count',                 `tmp_reg1.0')
define(`remainder_cmds',        `tmp_reg1.4')
define(`width_per_row',         `tmp_reg1.8')
define(`tmp_reg2',              `r10')
define(`quotient',              `tmp_reg2')
define(`tmp_reg3',              `r11')
define(`remainder',             `tmp_reg3')
define(`tmp_reg4',              `r12')
define(`tmp_reg5',              `r13')
define(`tmp_reg6',              `r14')
define(`tmp_reg7',              `r15')

/*
 * GRF 16
 */
define(`media_object_ud',       `r16.0')
define(`media_object0_ud',      `r16.0')
define(`media_object1_ud',      `r16.4')
define(`media_object2_ud',      `r16.8')
define(`media_object3_ud',      `r16.12')
define(`media_object4_ud',      `r16.16')
define(`media_object5_ud',      `r16.20')
define(`media_object6_ud',      `r16.24')
define(`media_object6_xy',      `r16.24')
define(`media_object6_x',       `r16.24')
define(`media_object6_y',       `r16.25')
define(`media_object6_width',   `r16.26')
define(`media_object7_ud',      `r16.28')
define(`media_object7_flag',    `r16.28')
define(`media_object7_num_mbs', `r16.30')

/* 
 * GRF 18 write back for Oword Block Write message 
 */

#if DEV_SNB

define(`obw_wb',                `r18')
define(`obw_wb_length',         `1')

#else

/*
 * write commit is removed on Ivybridge
 */
define(`obw_wb',                `null<1>:W')
define(`obw_wb_length',         `0')

#endif

/*
 * Message Payload registers
 */
#if DEV_SNB

define(`msg_ind',               `0')
define(`msg_reg0',              `m0')
define(`msg_reg1',              `m1')
define(`msg_reg2',              `m2')
define(`msg_reg3',              `m3')
define(`msg_reg4',              `m4')
define(`msg_reg5',              `m5')
define(`msg_reg6',              `m6')
define(`msg_reg7',              `m7')
define(`msg_reg8',              `m8')

#else

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

#endif
