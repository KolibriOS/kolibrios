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
        
/*
 * __START
 */
__START:
        mov             (16)    tmp_reg0<1>:ud                  0x0:ud {align1} ;
        mov             (16)    tmp_reg2<1>:ud                  0x0:ud {align1} ;
        mov             (1)     obw_header.20<1>:ub             thread_id_ub {align1};                  /* dispatch id */

        mov             (8)     media_object_ud<1>:ud           0x0:ud {align1} ;
        mov             (1)     media_object0_ud<1>:ud          CMD_MEDIA_OBJECT {align1} ;
        mov             (1)     media_object1_ud<1>:ud          mtype_ub<0,1,0>ub {align1};
        mov             (1)     media_object6_width<1>:uw       width_in_mb<0,1,0>:uw {align1};
        mov             (1)     media_object7_flag<1>:uw        transform_8x8_ub<0,1,0>ub {align1};
        mov             (1)     media_object7_num_mbs<1>:uw     NUM_MACROBLOCKS_PER_COMMAND:uw {align1} ;

        mov             (1)     width_per_row<1>:ud             width_in_mb<0,1,0>:uw {align1} ;
        and.z.f0.1      (1)     remainder_cmds<1>:ud            total_mbs<0,1,0>:ud     (NUM_MACROBLOCKS_PER_COMMAND - 1):ud {align1} ;
        and.z.f0.0      (1)     total_mbs<1>:ud                 total_mbs<0,1,0>:ud     -NUM_MACROBLOCKS_PER_COMMAND:ud {align1} ;

        (f0.0)jmpi      (1)     __REMAINDER ;
        
__CMD_LOOP:
        mov             (8)     msg_reg0.0<1>:ud                obw_header<8,8,1>:ud {align1};
        mov             (8)     msg_reg1<1>:ud                  media_object_ud<8,8,1>:ud {align1};
        
/* bind index 5, write 2 oword, msg type: 8(OWord Block Write) */
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

        /* (x, y) of the first macroblock */
        add             (1)     count<1>:ud                     count<0,1,0>:ud         NUM_MACROBLOCKS_PER_COMMAND:uw {align1} ;
        math            (1)     quotient<1>:ud                  count<0,1,0>:ud         width_per_row<0,1,0>:ud intdivmod {align1} ;
        shl             (1)     quotient<1>:ud                  quotient<0,1,0>:ud      8:uw {align1} ;
        add             (1)     quotient<1>:ud                  quotient<0,1,0>:ud      remainder<0,1,0>:ud {align1} ;
        mov             (1)     media_object6_xy<1>:uw          quotient<0,1,0>:uw {align1} ;
        
        /* the new offset */
        add             (1)     obw_header.8<1>:ud              obw_header.8<0,1,0>:ud  2:uw {align1} ;

        add.z.f0.0      (1)     total_mbs<1>:w                  total_mbs<0,1,0>:w      -NUM_MACROBLOCKS_PER_COMMAND:w {align1} ;
        (-f0.0)jmpi     (1)     __CMD_LOOP ;

__REMAINDER:
        (f0.1)jmpi      (1)     __DONE ;

        mov             (1)     media_object7_num_mbs<1>:uw     remainder_cmds<0,1,0>:uw {align1} ;        
        mov             (8)     msg_reg0.0<1>:ud                obw_header<8,8,1>:ud {align1};
        mov             (8)     msg_reg1<1>:ud                  media_object_ud<8,8,1>:ud {align1};
        
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

        /* the new offset */
        add             (1)     obw_header.8<1>:ud              obw_header.8<0,1,0>:ud  2:uw {align1} ;
        
__DONE:

/* bind index 5, write 1 oword, msg type: 8(OWord Block Write) */
        mov             (8)     msg_reg0.0<1>:ud                obw_header<8,8,1>:ud {align1} ;
        mov             (4)     msg_reg1.0<1>:ud                0x0:ud {align1} ;
        mov             (1)     msg_reg1.4<1>:ud                MI_BATCH_BUFFER_END {align1} ;
        
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
        mov             (8)     msg_reg0<1>:ud                  r0<8,8,1>:ud {align1} ;
        send            (16)    msg_ind acc0<1>ud null thread_spawner(0, 0, 1) mlen 1 rlen 0 {align1 EOT} ;
