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

define(`BIND_IDX_VME_OUTPUT',           `0')
define(`BIND_IDX_MFC_SLICE_HEADER',     `1')
define(`BIND_IDX_MFC_BATCHBUFFER',      `2')

define(`INTRAMBFLAG_MASK',              `0x00002000')

#ifdef DEV_SNB

define(`OB_CACHE_TYPE',                 `5')

#else

define(`OB_CACHE_TYPE',                 `10')

#endif

define(`OB_READ',                       `0')
define(`OB_WRITE',                      `8')

define(`OB_CONTROL_0',                  `0')    /* 1 OWord, low 128 bits */
define(`OB_CONTROL_1',                  `1')    /* 1 OWord, high 128 bits */
define(`OB_CONTROL_2',                  `2')    /* 2 OWords */
define(`OB_CONTROL_3',                  `3')    /* 4 OWords */
define(`OB_CONTROL_4',                  `4')    /* 8 OWords */

#ifdef DEV_SNB

define(`OB_WRITE_COMMIT_CATEGORY',      `1')    /* write commit on Sandybrige */

#else

define(`OB_WRITE_COMMIT_CATEGORY',      `0')    /* category on Ivybridge */

#endif

define(`OB_HEADER_PRESENT',             `1')

define(`INTER_VME_OUTPUT_IN_BYTES',     `160')
define(`INTER_VME_OUTPUT_IN_OWS',       `10')
define(`INTER_VME_OUTPUT_MV_IN_OWS',    `8')

define(`MFC_AVC_PAK_OBJECT_INTRA_DW0',  `0x71490009:UD')
define(`MFC_AVC_PAK_OBJECT_INTRA_DW3',  `0x000e0000:UD')        /* CbpDC (1 << 19 | 1 << 18 | 1 << 17) */
define(`MFC_AVC_PAK_OBJECT_INTRA_DW4',  `0xFFFF0000:UD')        /* CBP for Y */
define(`MFC_AVC_PAK_OBJECT_INTRA_DW5',  `0x000F000F:UD')
define(`MFC_AVC_PAK_OBJECT_INTRA_DW6',  `0x04000000:UD')        /* the flag of the last macroblock */

define(`MFC_AVC_PAK_OBJECT_INTER_DW0',  `MFC_AVC_PAK_OBJECT_INTRA_DW0')
define(`MFC_AVC_PAK_OBJECT_INTER_DW1',  `0x20:UD')              /* 32 MVs */
define(`MFC_AVC_PAK_OBJECT_INTER_DW2',  `INTER_VME_OUTPUT_IN_BYTES:UD') /* offset, in bytes */
define(`MFC_AVC_PAK_OBJECT_INTER_DW3',  `0x014e0000:UD')        /* 
                                                                 * (1 << 24) |     PackedMvNum, Debug
                                                                 * (4 << 20) |     8 MV, SNB don't use it
                                                                 * (1 << 19) |     CbpDcY
                                                                 * (1 << 18) |     CbpDcU
                                                                 * (1 << 17) |     CbpDcV
                                                                 * (0 << 15) |     Transform8x8Flag = 0
                                                                 * (0 << 14) |     Frame based
                                                                 * (0 << 13) |     Inter MB
                                                                 * (1 << 8)  |     MbType = P_L0_16x16
                                                                 * (0 << 7)  |     MBZ for frame
                                                                 * (0 << 6)  |     MBZ
                                                                 * (2 << 4)  |     MBZ for inter
                                                                 * (0 << 3)  |     MBZ
                                                                 * (0 << 2)  |     SkipMbFlag
                                                                 * (0 << 0)        InterMbMode
                                                                 */
define(`MFC_AVC_PAK_OBJECT_INTER_DW4',  `MFC_AVC_PAK_OBJECT_INTRA_DW4')
define(`MFC_AVC_PAK_OBJECT_INTER_DW5',  `MFC_AVC_PAK_OBJECT_INTRA_DW5')
define(`MFC_AVC_PAK_OBJECT_INTER_DW6',  `MFC_AVC_PAK_OBJECT_INTRA_DW6')

define(`MI_BATCH_BUFFER_END',           `0x05000000:UD')

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

/*
 * GRF 1~4 -- Constant Buffer (reserved)
 */

define(`FLAG_MASK_LAST_SLICE',          `0x0001:uw')
define(`FLAG_MASK_LAST_OBJECT',         `0x0002:uw')
define(`FLAG_MASK_FIRST_OBJECT',        `0x0004:uw')        

/*
 * GRF 5 -- inline data
 */        
define(`inline_reg0',           `r5')
define(`head_offset',           `inline_reg0.0') /* :ud, in units of Owords */
define(`batchbuffer_offset',    `inline_reg0.4') /* :ud, in units of Owords */
define(`tail_size',             `inline_reg0.8') /* :w, in units of Owords */
define(`head_size',             `inline_reg0.10') /* :w, in units of Owords */
define(`flags',                 `inline_reg0.12') /* :uw,
                                                   * bit0 the flag of the last slice
                                                   * bit1 the flag of the last object in a slice
                                                   * bit2 the flag of the first object in a slice 
                                                   */
define(`total_mbs',             `inline_reg0.14') /* :w, the number of macroblock commands 
                                                   * being processed by the kernel
                                                   */
define(`mb_x',                  `inline_reg0.16') /* :ub, */
define(`mb_y',                  `inline_reg0.17') /* :ub, */
define(`mb_xy',                 `inline_reg0.16') /* :uw, */      
define(`width_in_mb',           `inline_reg0.20') /* :uw, the picture width in macroblocks */
define(`qp',                    `inline_reg0.22') /* :ub, */
define(`ref_idx0',              `inline_reg0.24') /* :ud */
define(`ref_idx1',              `inline_reg0.28') /* :ud */

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

define(`tmp_vme_output',        `tmp_reg0')
define(`tmp_slice_header',      `tmp_reg1')
define(`tmp_mfc_batchbuffer',   `tmp_reg2')
define(`tmp_offset',            `tmp_reg7')
/* 
 * GRF 16~23 write back for Oword Block Read message 
 */
define(`ob_read_wb',            `r16<1>:uw')
define(`ob_read_wb0',           `r16')
define(`ob_read_wb1',           `r17')
define(`ob_read_wb2',           `r18')
define(`ob_read_wb3',           `r19')
define(`ob_read_wb4',           `r20')
define(`ob_read_wb5',           `r21')
define(`ob_read_wb6',           `r22')
define(`ob_read_wb7',           `r23')

define(`ob_read_wb_len_slice_header',   `1')
define(`ob_read_wb_len_vme_intra',      `1')
define(`ob_read_wb_len_vme_inter',      `1')

#ifdef DEV_SNB

/* 
 * GRF 24~25 write back for Oword Block Write message 
 */

define(`ob_write_wb',           `r24')
define(`ob_write_wb_length',    `1')

#else

/* 
 * GRF 24~25 -- reserved
 */
define(`ob_write_wb',           `null<1>:W')
define(`ob_write_wb_length',    `0')

#endif

/*
 * GRF 26~27 
 */
define(`pak_object_ud',       `r26.0')
define(`pak_object0_ud',      `r26.0')
define(`pak_object1_ud',      `r26.4')
define(`pak_object2_ud',      `r26.8')
define(`pak_object3_ud',      `r26.12')
define(`pak_object4_ud',      `r26.16')
define(`pak_object5_ud',      `r26.20')
define(`pak_object6_ud',      `r26.24')
define(`pak_object7_ud',      `r26.28')
define(`pak_object8_ud',      `r27.0')
define(`pak_object9_ud',      `r27.4')
define(`pak_object10_ud',     `r27.8')
define(`pak_object11_ud',     `r27.12')

#ifdef DEV_SNB

/*
 * Message Payload registers
 */
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

#endif
