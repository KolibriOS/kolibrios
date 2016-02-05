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
// Modual name: ME_header.inc
//
// Global symbols define
//

/*
 * Constant
 */
define(`VME_MESSAGE_TYPE_INTER',        `1')
define(`VME_MESSAGE_TYPE_INTRA',        `2')
define(`VME_MESSAGE_TYPE_MIXED',        `3')
        
define(`BLOCK_32X1',                    `0x0000001F')
define(`BLOCK_4X16',                    `0x000F0003')
        
define(`LUMA_INTRA_16x16_DISABLE',      `0x1')
define(`LUMA_INTRA_8x8_DISABLE',        `0x2')
define(`LUMA_INTRA_4x4_DISABLE',        `0x4')

define(`INTRA_PRED_AVAIL_FLAG_AE',      `0x60')
define(`INTRA_PRED_AVAIL_FLAG_B',       `0x10')
define(`INTRA_PRED_AVAIL_FLAG_C',       `0x8')
define(`INTRA_PRED_AVAIL_FLAG_D',       `0x4')

define(`BIND_IDX_VME',                  `0')
define(`BIND_IDX_VME_REF0',             `1')
define(`BIND_IDX_VME_REF1',             `2')
define(`BIND_IDX_OUTPUT',               `3')
define(`BIND_IDX_INEP',                 `4')

define(`SUB_PEL_MODE_INTEGER',          `0x00000000')
define(`SUB_PEL_MODE_HALF',             `0x00001000')
define(`SUB_PEL_MODE_QUARTER',          `0x00003000')

define(`INTER_SAD_NONE',                `0x00000000')
define(`INTER_SAD_HAAR',                `0x00200000')

define(`INTRA_SAD_NONE',                `0x00000000')
define(`INTRA_SAD_HAAR',                `0x00800000')

define(`INTER_PART_MASK',               `0x00000000')

define(`SEARCH_CTRL_SINGLE',            `0x00000000')
define(`SEARCH_CTRL_DUAL_START',        `0x00000100')
define(`SEARCH_CTRL_DUAL_RECORD',       `0x00000300')
define(`SEARCH_CTRL_DUAL_REFERENCE',    `0x00000700')

define(`REF_REGION_SIZE',               `0x2830:UW')
define(`MIN_REF_REGION_SIZE',           `0x2020:UW')

define(`BI_SUB_MB_PART_MASK',           `0x0c000000')
define(`MAX_NUM_MV',                    `0x00000020')
define(`FB_PRUNING_ENABLE',             `0x40000000')

define(`SEARCH_PATH_LEN',               `0x00003030')
define(`START_CENTER',                  `0x30000000')

define(`ADAPTIVE_SEARCH_ENABLE',        `0x00000002') 
define(`INTRA_PREDICTORE_MODE',         `0x11111111:UD')

define(`INTER_VME_OUTPUT_IN_OWS',       `10')
define(`INTER_VME_OUTPUT_MV_IN_OWS',    `8')

define(`INTRAMBFLAG_MASK',              `0x00002000')
define(`MVSIZE_UW_BASE',                `0x0040')
define(`MFC_MV32_BIT_SHIFT',            `5')
define(`CBP_DC_YUV_UW',                 `0x000E')

#ifdef DEV_SNB

define(`MV32_BIT_MASK',                 `0x0010')
define(`MV32_BIT_SHIFT',                `4')

define(`OBW_CACHE_TYPE',                `5')

#else

define(`MV32_BIT_MASK',                 `0x0020')
define(`MV32_BIT_SHIFT',                `5')

define(`OBW_CACHE_TYPE',                `10')

#endif

define(`OBW_MESSAGE_TYPE',              `8')

define(`OBW_BIND_IDX',                  `BIND_IDX_OUTPUT')

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

/* GRF registers
 * r0 header
 * r1~r4 constant buffer (reserved)
 * r5 inline data
 * r6~r11 reserved        
 * r12 write back of VME message
 * r13 write back of Oword Block Write        
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
define(`w_in_mb_uw',            `inline_reg0.2')
define(`orig_xy_ub',            `inline_reg0.0')
define(`orig_x_ub',             `inline_reg0.0')    /* in macroblock */    
define(`orig_y_ub',             `inline_reg0.1')
define(`transform_8x8_ub',      `inline_reg0.4')
define(`slice_edge_ub',         `inline_reg0.4')
define(`num_macroblocks',       `inline_reg0.6')
define(`quality_level_ub',      `inline_reg0.8')

/*
 * GRF 6~11 -- reserved
 */

/*
 * GRF 12~15 -- write back for VME message 
 */
define(`vme_wb',                `r12')
define(`vme_wb0',               `r12')
define(`vme_wb1',               `r13')
define(`vme_wb2',               `r14')
define(`vme_wb3',               `r15')

#ifdef DEV_SNB        
/*
 * GRF 16 -- write back for Oword Block Write message with write commit bit
 */        
define(`obw_wb',                `r16')
define(`obw_wb_length',         `1')

#else

/*
 * GRF 16 -- write back for VME message
 */
define(`vme_wb4',               `r16')
define(`obw_wb',                `null<1>:W')
define(`obw_wb_length',         `0')

#endif

/*
 * GRF 18~21 -- Intra Neighbor Edge Pixels
 */
define(`INEP_ROW',              `r18')
define(`INEP_COL0',             `r20')
define(`INEP_COL1',             `r21')
        
/*
 * temporary registers
 */
define(`tmp_reg0',              `r32')
define(`read0_header',          `tmp_reg0')
define(`tmp_reg1',              `r33')
define(`read1_header',          `tmp_reg1')
define(`tmp_reg2',              `r34')
define(`vme_m0',                `tmp_reg2')
define(`tmp_reg3',              `r35')                                
define(`vme_m1',                `tmp_reg3')
define(`intra_flag',            `vme_m1.28')
define(`intra_part_mask_ub',    `vme_m1.28')        
define(`mb_intra_struct_ub',    `vme_m1.29')
define(`tmp_reg4',              `r36')
define(`obw_m0',                `tmp_reg4')
define(`tmp_reg5',              `r37')
define(`obw_m1',                `tmp_reg5')
define(`tmp_reg6',              `r38')
define(`obw_m2',                `tmp_reg6')
define(`tmp_reg7',              `r39')
define(`obw_m3',                `tmp_reg7')
define(`tmp_reg8',              `r40')
define(`obw_m4',                `tmp_reg8')
define(`tmp_reg9',              `r41')
define(`tmp_x_w',               `tmp_reg9.0')
define(`tmp_rega',              `r42')
define(`tmp_ud0',               `tmp_rega.0')
define(`tmp_ud1',               `tmp_rega.4')
define(`tmp_ud2',               `tmp_rega.8')
define(`tmp_ud3',               `tmp_rega.12')
define(`tmp_uw0',               `tmp_rega.0')
define(`tmp_uw1',               `tmp_rega.2')
define(`tmp_uw2',               `tmp_rega.4')
define(`tmp_uw3',               `tmp_rega.6')
define(`tmp_uw4',               `tmp_rega.8')
define(`tmp_uw5',               `tmp_rega.10')
define(`tmp_uw6',               `tmp_rega.12')
define(`tmp_uw7',               `tmp_rega.14')

/*
 * MRF registers
 */        
#ifdef DEV_SNB

define(`msg_ind',               `0')
define(`msg_reg0',              `m0')               /* m0 */
define(`msg_reg1',              `m1')               /* m1 */
define(`msg_reg2',              `m2')               /* m2 */
define(`msg_reg3',              `m3')               /* m3 */
define(`msg_reg4',              `m4')               /* m4 */

#else

define(`msg_ind',               `64')
define(`msg_reg0',              `g64')
define(`msg_reg1',              `g65')
define(`msg_reg2',              `g66')
define(`msg_reg3',              `g67')
define(`msg_reg4',              `g68')

#endif

/*
 * VME message payload
 */

#ifdef DEV_SNB

define(`vme_msg_length',        `4')
define(`vme_inter_wb_length',   `4')

#else

define(`vme_msg_length',        `5')
define(`vme_inter_wb_length',   `6')

#endif

define(`vme_intra_wb_length',   `1')

define(`vme_msg_ind',           `msg_ind')
define(`vme_msg_0',             `msg_reg0')
define(`vme_msg_1',             `msg_reg1')
define(`vme_msg_2',             `msg_reg2')

#ifdef DEV_SNB

define(`vme_msg_3',             `vme_msg_2')
define(`vme_msg_4',             `msg_reg3')

#else

define(`vme_msg_3',             `msg_reg3')
define(`vme_msg_4',             `msg_reg4')

#endif

define(`DEFAULT_QUALITY_LEVEL',           `0x01')
define(`HIGH_QUALITY_LEVEL',              `DEFAULT_QUALITY_LEVEL')
define(`LOW_QUALITY_LEVEL',               `0x02')
