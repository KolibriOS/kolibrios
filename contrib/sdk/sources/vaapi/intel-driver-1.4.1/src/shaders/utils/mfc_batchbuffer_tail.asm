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
        
__TAIL:
        (f0.1)jmpi      (1)     __EXIT ;
        
__TAIL_LOOP:
        mov             (8)     msg_reg0.0<1>:ud                tmp_slice_header<8,8,1>:ud {align1} ;

send (16)
        msg_ind
        ob_read_wb
        null
        data_port(
                OB_CACHE_TYPE,
                OB_READ,
                OB_CONTROL_0,
                BIND_IDX_MFC_SLICE_HEADER,
                OB_WRITE_COMMIT_CATEGORY,
                OB_HEADER_PRESENT
        )
        mlen 1
        rlen ob_read_wb_len_slice_header
        {align1};

        mov             (8)     msg_reg0.0<1>:ud                tmp_mfc_batchbuffer<8,8,1>:ud {align1} ;        
        mov             (8)     msg_reg1.0<1>:ud                ob_read_wb0<8,8,1>:ud {align1} ;

send (16)
        msg_ind
        ob_write_wb
        null
        data_port(
                OB_CACHE_TYPE,
                OB_WRITE,
                OB_CONTROL_0,
                BIND_IDX_MFC_BATCHBUFFER,
                OB_WRITE_COMMIT_CATEGORY,
                OB_HEADER_PRESENT
        )
        mlen 2
        rlen ob_write_wb_length
        {align1};

        /* the new offset */
        add             (1)     tmp_slice_header.8<1>:ud        tmp_slice_header.8<0,1,0>:ud    1:ud {align1} ;
        add             (1)     tmp_mfc_batchbuffer.8<1>:ud     tmp_mfc_batchbuffer.8<0,1,0>:ud    1:ud {align1} ;

        add.z.f0.0      (1)     tail_size<1>:w                  tail_size<0,1,0>:w      -1:w {align1};
        (-f0.0)jmpi     (1)     __TAIL_LOOP ;

        
__DONE:

        and.z.f0.0      (1)     null<1>:uw                      flags<0,1,0>:uw         FLAG_MASK_LAST_SLICE {align1};
        (f0.0)jmpi      (1)     __EXIT ;
        
/* bind index 5, write 1 oword, msg type: 8(OWord Block Write) */
        mov             (8)     msg_reg0.0<1>:ud                tmp_mfc_batchbuffer<8,8,1>:ud {align1} ;
        mov             (4)     msg_reg1.0<1>:ud                0x0:ud {align1} ;
        mov             (1)     msg_reg1.4<1>:ud                MI_BATCH_BUFFER_END {align1} ;
        
send (16)
        msg_ind
        ob_write_wb
        null
        data_port(
                OB_CACHE_TYPE,
                OB_WRITE,
                OB_CONTROL_0,
                BIND_IDX_MFC_BATCHBUFFER,
                OB_WRITE_COMMIT_CATEGORY,
                OB_HEADER_PRESENT
        )
        mlen 2
        rlen ob_write_wb_length
        {align1};
