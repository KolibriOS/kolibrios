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
        
__PAK_OBJECT:   
        mul             (1)     tmp_vme_output.8<1>:ud          width_in_mb<0,1,0>:uw           mb_y<0,1,0>:ub {align1};
        add             (1)     tmp_vme_output.8<1>:ud          tmp_vme_output.8<0,1,0>:ud      mb_x<0,1,0>:ub {align1};
        
        mov             (16)    pak_object_ud<1>:ud             0x0:ud {align1} ;
        mov             (1)     pak_object0_ud<1>:ud            MFC_AVC_PAK_OBJECT_INTRA_DW0 ;
        mov             (1)     pak_object5_ud<1>:ud            MFC_AVC_PAK_OBJECT_INTRA_DW5 ;

        and.z.f0.1      (1)     null<1>:uw                      flags<0,1,0>:uw         FLAG_MASK_LAST_OBJECT {align1};
        
__PAK_OBJECT_LOOP:
        mov             (8)     msg_reg0.0<1>:ud                tmp_vme_output<8,8,1>:ud {align1} ;
        
send (16)
        msg_ind
        ob_read_wb
        null
        data_port(
                OB_CACHE_TYPE,
                OB_READ,
                OB_CONTROL_0,
                BIND_IDX_VME_OUTPUT,
                OB_WRITE_COMMIT_CATEGORY,
                OB_HEADER_PRESENT
        )
        mlen 1
        rlen ob_read_wb_len_vme_intra
        {align1};

        /* DW4 */
        add             (1)     pak_object4_ud<1>:ud            mb_xy<0,1,0>:uw                 MFC_AVC_PAK_OBJECT_INTRA_DW4 {align1} ;
        add             (1)     mb_x<1>:ub                      mb_x<0,1,0>:ub                  1:uw {align1};
        cmp.e.f0.0      (1)     null<1>:uw                      width_in_mb<0,1,0>:uw           mb_x<0,1,0>:ub {align1};
        (f0.0)mov       (1)     mb_x<1>:ub                      0:uw {align1} ;
        (f0.0)add       (1)     mb_y<1>:ub                      mb_y<0,1,0>:ub                  1:uw {align1} ;

        /* DW6 */
        mov             (1)     pak_object6_ud<1>:ud            0x0:ud {align1} ;
        (-f0.1)mov      (1)     pak_object6_ud<1>:ud            MFC_AVC_PAK_OBJECT_INTRA_DW6 {align1} ;
        cmp.e.f0.0      (1)     null<1>:uw                      total_mbs<0,1,0>:uw             1:uw {align1};        
        (-f0.0)mov      (1)     pak_object6_ud<1>:ud            0x0:ud {align1} ;
        add             (1)     pak_object6_ud<1>:ud            pak_object6_ud<0,1,0>:ud        qp<0,1,0>:ub {align1} ;

        /* DW3 */
        and             (1)     pak_object3_ud<1>:ud            ob_read_wb0.0<0,1,0>:ud         0xFFFF {align1} ;
        add             (1)     pak_object3_ud<1>:ud            pak_object3_ud<0,1,0>:ud        MFC_AVC_PAK_OBJECT_INTRA_DW3 {align1} ;

        /* DW7 */
        mov             (1)     pak_object7_ud<1>:ud            ob_read_wb0.4<0,1,0>:ud {align1} ;

        /* DW8 */
        mov             (1)     pak_object8_ud<1>:ud            ob_read_wb0.8<0,1,0>:ud {align1} ;

        /* DW9 */
        and             (1)     pak_object9_ud<1>:ud            ob_read_wb0.12<0,1,0>:ud        0xFC:ud {align1} ;
        
        mov             (8)     msg_reg0.0<1>:ud                tmp_mfc_batchbuffer<8,8,1>:ud {align1} ;
        mov             (8)     msg_reg1.0<1>:ud                pak_object_ud<8,8,1>:ud {align1} ;
        mov             (8)     msg_reg2.0<1>:ud                pak_object8_ud<8,8,1>:ud {align1} ;        

        /* the new offset */
        add             (1)     tmp_vme_output.8<1>:ud          tmp_vme_output.8<0,1,0>:ud      1:ud {align1} ;
        
send (16)
        msg_ind
        ob_write_wb
        null
        data_port(
                OB_CACHE_TYPE,
                OB_WRITE,
                OB_CONTROL_3,
                BIND_IDX_MFC_BATCHBUFFER,
                OB_WRITE_COMMIT_CATEGORY,
                OB_HEADER_PRESENT
        )
        mlen 3
        rlen ob_write_wb_length
        {align1};

        /* the new offset */
        add             (1)     tmp_mfc_batchbuffer.8<1>:ud     tmp_mfc_batchbuffer.8<0,1,0>:ud 4:ud {align1} ;

        add.z.f0.0      (1)	total_mbs<1>:w                  total_mbs<0,1,0>:w      -1:w {align1};
        (-f0.0)jmpi     (1)     __PAK_OBJECT_LOOP ;
        
