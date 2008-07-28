if 0

 Copyright 2008  Serge

 The below code is a rework from code in
 xf86-video-radeonhd/src/r5xx_accel.c, xf86-video-radeonhd/src/r5xx_xaa.c

 Copyright 2008  Luc Verhaegen <lverhaegen@novell.com>
 Copyright 2008  Matthias Hopf <mhopf@novell.com>
 Copyright 2008  Egbert Eich   <eich@novell.com>
 Copyright 2008  Advanced Micro Devices, Inc.

 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 The below code is a rework from code in xf86-video-ati/src/radeon_accel.c
 The original license is included below, it has the messed up disclaimer and
 an all rights reserved statement.


 Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
                VA Linux Systems Inc., Fremont, California.

 All Rights Reserved.

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation on the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice (including the
 next paragraph) shall be included in all copies or substantial
 portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.

 Authors:
   Kevin E. Martin <martin@xfree86.org>
   Rickard E. Faith <faith@valinux.com>
   Alan Hourihane <alanh@fairlite.demon.co.uk>

end if

RADEON_CP_ME_RAM_ADDR            equ 0x07d4
RADEON_CP_ME_RAM_RADDR           equ 0x07d8
RADEON_CP_ME_RAM_DATAH           equ 0x07dc
RADEON_CP_ME_RAM_DATAL           equ 0x07e0

RADEON_CP_RB_BASE                equ 0x0700
RADEON_CP_RB_CNTL                equ 0x0704
  RADEON_RB_NO_UPDATE            equ (1 shl 27)
RADEON_CP_RB_RPTR_ADDR           equ 0x070c
RADEON_CP_RB_RPTR                equ 0x0710
RADEON_CP_RB_WPTR                equ 0x0714

RADEON_CP_CSQ_CNTL               equ 0x0740
  RADEON_CSQ_CNT_PRIMARY_MASK    equ (0xff shl 0)
  RADEON_CSQ_PRIDIS_INDDIS       equ (0    shl 28)
  RADEON_CSQ_PRIPIO_INDDIS       equ (1    shl 28)
  RADEON_CSQ_PRIBM_INDDIS        equ (2    shl 28)
  RADEON_CSQ_PRIPIO_INDBM        equ (3    shl 28)
  RADEON_CSQ_PRIBM_INDBM         equ (4    shl 28)
  RADEON_CSQ_PRIPIO_INDPIO       equ (15   shl 28)

RADEON_CP_RB_WPTR_DELAY          equ 0x0718

RADEON_SCRATCH_UMSK              equ 0x0770
RADEON_SCRATCH_ADDR              equ 0x0774

RADEON_ISYNC_CNTL                equ 0x1724
  RADEON_ISYNC_ANY2D_IDLE3D      equ (1 shl 0)
  RADEON_ISYNC_ANY3D_IDLE2D      equ (1 shl 1)
  RADEON_ISYNC_TRIG2D_IDLE3D     equ (1 shl 2)
  RADEON_ISYNC_TRIG3D_IDLE2D     equ (1 shl 3)
  RADEON_ISYNC_WAIT_IDLEGUI      equ (1 shl 4)
  RADEON_ISYNC_CPSCRATCH_IDLEGUI equ (1 shl 5)

RADEON_AIC_CNTL                  equ 0x01d0
  RADEON_PCIGART_TRANSLATE_EN    equ (1 shl 0)
RADEON_AIC_STAT                  equ 0x01d4
RADEON_AIC_PT_BASE               equ 0x01d8
RADEON_AIC_LO_ADDR               equ 0x01dc
RADEON_AIC_HI_ADDR               equ 0x01e0
RADEON_AIC_TLB_ADDR              equ 0x01e4
RADEON_AIC_TLB_DATA              equ 0x01e8

RADEON_WAIT_UNTIL                equ 0x1720
  RADEON_WAIT_CRTC_PFLIP         equ (1 shl 0)
  RADEON_WAIT_2D_IDLE            equ (1 shl 14)
  RADEON_WAIT_3D_IDLE            equ (1 shl 15)
  RADEON_WAIT_2D_IDLECLEAN       equ (1 shl 16)
  RADEON_WAIT_3D_IDLECLEAN       equ (1 shl 17)
  RADEON_WAIT_HOST_IDLECLEAN     equ (1 shl 18)

D1GRPH_PITCH                     equ 0x6120
D1GRPH_X_END                     equ 0x6134
D1GRPH_Y_END                     equ 0x6138


R5XX_DATATYPE_ARGB8888           equ  6

R5XX_RB3D_CNTL                   equ  0x1c3c

R5XX_RBBM_STATUS                 equ  0x0e40
  R5XX_RBBM_FIFOCNT_MASK         equ  0x007f
  R5XX_RBBM_ACTIVE               equ  (1 shl 31)

R5XX_RBBM_SOFT_RESET             equ  0x00f0
  R5XX_SOFT_RESET_CP             equ  (1 shl 0)
  R5XX_SOFT_RESET_HI             equ  (1 shl 1)
  R5XX_SOFT_RESET_SE             equ  (1 shl 2)
  R5XX_SOFT_RESET_RE             equ  (1 shl 3)
  R5XX_SOFT_RESET_PP             equ  (1 shl 4)
  R5XX_SOFT_RESET_E2             equ  (1 shl 5)
  R5XX_SOFT_RESET_RB             equ  (1 shl 6)
  R5XX_SOFT_RESET_HDP            equ  (1 shl 7)

R5XX_SRC_PITCH_OFFSET            equ  0x1428
R5XX_DST_PITCH_OFFSET            equ  0x142c

R5XX_DP_DATATYPE                 equ  0x16c4
  R5XX_HOST_BIG_ENDIAN_EN        equ  (1 shl 29)

R5XX_DP_CNTL                     equ  0x16c0
  R5XX_DST_X_LEFT_TO_RIGHT       equ (1 shl  0)
  R5XX_DST_Y_TOP_TO_BOTTOM       equ (1 shl  1)
  R5XX_DP_DST_TILE_LINEAR        equ (0 shl  3)
  R5XX_DP_DST_TILE_MACRO         equ (1 shl  3)
  R5XX_DP_DST_TILE_MICRO         equ (2 shl  3)
  R5XX_DP_DST_TILE_BOTH          equ (3 shl  3)

RADEON_RB3D_ZCACHE_CTLSTAT       equ 0x3254
  RADEON_RB3D_ZC_FLUSH           equ (1 shl 0)
  RADEON_RB3D_ZC_FREE            equ (1 shl 2)
  RADEON_RB3D_ZC_FLUSH_ALL       equ 0x5
  RADEON_RB3D_ZC_BUSY            equ (1 shl 31)

R5XX_RB3D_DSTCACHE_CTLSTAT       equ  0x325C
  R5XX_RB3D_DC_FLUSH             equ  (3 shl 0)
  R5XX_RB3D_DC_FREE              equ  (3 shl 2)
  R5XX_RB3D_DC_FLUSH_ALL         equ  0xf
  R5XX_RB3D_DC_BUSY              equ  (1 shl 31)

R5XX_SURFACE_CNTL                equ  0x0b00
  R5XX_SURF_TRANSLATION_DIS      equ  (1 shl 8)
  R5XX_NONSURF_AP0_SWP_16BPP     equ  (1 shl 20)
  R5XX_NONSURF_AP0_SWP_32BPP     equ  (1 shl 21)
  R5XX_NONSURF_AP1_SWP_16BPP     equ  (1 shl 22)
  R5XX_NONSURF_AP1_SWP_32BPP     equ  (1 shl 23)

R5XX_DEFAULT_SC_BOTTOM_RIGHT     equ  0x16e8
  R5XX_DEFAULT_SC_RIGHT_MAX      equ  (0x1fff shl  0)
  R5XX_DEFAULT_SC_BOTTOM_MAX     equ  (0x1fff shl 16)

R5XX_SC_TOP_LEFT                 equ  0x16ec
  R5XX_SC_BOTTOM_RIGHT           equ  0x16f0
  R5XX_SC_SIGN_MASK_LO           equ  0x8000
  R5XX_SC_SIGN_MASK_HI           equ  0x80000000

R5XX_DP_GUI_MASTER_CNTL          equ  0x146c
R5XX_GMC_SRC_PITCH_OFFSET_CNTL   equ  (1 shl 0)
R5XX_GMC_DST_PITCH_OFFSET_CNTL   equ  (1 shl 1)
R5XX_GMC_SRC_CLIPPING            equ  (1 shl 2)
R5XX_GMC_DST_CLIPPING            equ  (1    shl 3)
R5XX_GMC_BRUSH_DATATYPE_MASK     equ  (0x0f shl 4)
R5XX_GMC_BRUSH_8X8_MONO_FG_BG    equ  (0    shl 4)
R5XX_GMC_BRUSH_8X8_MONO_FG_LA    equ  (1    shl 4)
R5XX_GMC_BRUSH_1X8_MONO_FG_BG    equ  (4    shl 4)
R5XX_GMC_BRUSH_1X8_MONO_FG_LA    equ  (5    shl 4)
R5XX_GMC_BRUSH_32x1_MONO_FG_BG   equ  (6    shl 4)
R5XX_GMC_BRUSH_32x1_MONO_FG_LA   equ  (7    shl 4)
R5XX_GMC_BRUSH_32x32_MONO_FG_BG  equ  (8    shl 4)
R5XX_GMC_BRUSH_32x32_MONO_FG_LA  equ  (9    shl 4)
R5XX_GMC_BRUSH_8x8_COLOR         equ  (10   shl 4)
R5XX_GMC_BRUSH_1X8_COLOR         equ  (12   shl 4)
R5XX_GMC_BRUSH_SOLID_COLOR       equ  (13   shl 4)
R5XX_GMC_BRUSH_NONE              equ  (15   shl 4)
R5XX_GMC_DST_8BPP_CI             equ  (2    shl 8)
R5XX_GMC_DST_15BPP               equ  (3    shl 8)
R5XX_GMC_DST_16BPP               equ  (4    shl 8)
R5XX_GMC_DST_24BPP               equ  (5    shl 8)
R5XX_GMC_DST_32BPP               equ  (6    shl 8)
R5XX_GMC_DST_8BPP_RGB            equ  (7    shl 8)
R5XX_GMC_DST_Y8                  equ  (8    shl 8)
R5XX_GMC_DST_RGB8                equ  (9    shl 8)
R5XX_GMC_DST_VYUY                equ  (11   shl 8)
R5XX_GMC_DST_YVYU                equ  (12   shl 8)
R5XX_GMC_DST_AYUV444             equ  (14   shl 8)
R5XX_GMC_DST_ARGB4444            equ  (15   shl 8)
R5XX_GMC_DST_DATATYPE_MASK       equ  (0x0f shl 8)
R5XX_GMC_DST_DATATYPE_SHIFT      equ  8
R5XX_GMC_SRC_DATATYPE_MASK       equ  (3    shl 12)
R5XX_GMC_SRC_DATATYPE_MONO_FG_BG equ  (0    shl 12)
R5XX_GMC_SRC_DATATYPE_MONO_FG_LA equ  (1    shl 12)
R5XX_GMC_SRC_DATATYPE_COLOR      equ  (3    shl 12)
R5XX_GMC_BYTE_PIX_ORDER          equ  (1    shl 14)
R5XX_GMC_BYTE_MSB_TO_LSB         equ  (0    shl 14)
R5XX_GMC_BYTE_LSB_TO_MSB         equ  (1    shl 14)
R5XX_GMC_CONVERSION_TEMP         equ  (1    shl 15)
R5XX_GMC_CONVERSION_TEMP_6500    equ  (0    shl 15)
R5XX_GMC_CONVERSION_TEMP_9300    equ  (1    shl 15)
R5XX_GMC_ROP3_MASK               equ  (0xff shl 16)
R5XX_DP_SRC_SOURCE_MASK          equ  (7    shl 24)
R5XX_DP_SRC_SOURCE_MEMORY        equ  (2    shl 24)
R5XX_DP_SRC_SOURCE_HOST_DATA     equ  (3    shl 24)
R5XX_GMC_3D_FCN_EN               equ  (1    shl 27)
R5XX_GMC_CLR_CMP_CNTL_DIS        equ  (1    shl 28)
R5XX_GMC_AUX_CLIP_DIS            equ  (1    shl 29)
R5XX_GMC_WR_MSK_DIS              equ  (1    shl 30)
R5XX_GMC_LD_BRUSH_Y_X            equ  (1    shl 31)
R5XX_ROP3_ZERO                   equ  0x00000000
R5XX_ROP3_DSa                    equ  0x00880000
R5XX_ROP3_SDna                   equ  0x00440000
R5XX_ROP3_S                      equ  0x00cc0000
R5XX_ROP3_DSna                   equ  0x00220000
R5XX_ROP3_D                      equ  0x00aa0000
R5XX_ROP3_DSx                    equ  0x00660000
R5XX_ROP3_DSo                    equ  0x00ee0000
R5XX_ROP3_DSon                   equ  0x00110000
R5XX_ROP3_DSxn                   equ  0x00990000
R5XX_ROP3_Dn                     equ  0x00550000
R5XX_ROP3_SDno                   equ  0x00dd0000
R5XX_ROP3_Sn                     equ  0x00330000
R5XX_ROP3_DSno                   equ  0x00bb0000
R5XX_ROP3_DSan                   equ  0x00770000
R5XX_ROP3_ONE                    equ  0x00ff0000
R5XX_ROP3_DPa                    equ  0x00a00000
R5XX_ROP3_PDna                   equ  0x00500000
R5XX_ROP3_P                      equ  0x00f00000
R5XX_ROP3_DPna                   equ  0x000a0000
R5XX_ROP3_D                      equ  0x00aa0000
R5XX_ROP3_DPx                    equ  0x005a0000
R5XX_ROP3_DPo                    equ  0x00fa0000
R5XX_ROP3_DPon                   equ  0x00050000
R5XX_ROP3_PDxn                   equ  0x00a50000
R5XX_ROP3_PDno                   equ  0x00f50000
R5XX_ROP3_Pn                     equ  0x000f0000
R5XX_ROP3_DPno                   equ  0x00af0000
R5XX_ROP3_DPan                   equ  0x005f0000

R5XX_HOST_PATH_CNTL              equ  0x0130
R5XX_HDP_SOFT_RESET              equ  (1 shl 26)
R5XX_HDP_APER_CNTL               equ  (1 shl 23)

R5XX_RB3D_DSTCACHE_MODE             equ  0x3258
R5XX_RB3D_DC_CACHE_ENABLE           equ      (0)
R5XX_RB3D_DC_2D_CACHE_DISABLE       equ      (1)
R5XX_RB3D_DC_3D_CACHE_DISABLE       equ      (2)
R5XX_RB3D_DC_CACHE_DISABLE          equ      (3)
R5XX_RB3D_DC_2D_CACHE_LINESIZE_128  equ  (1 shl  2)
R5XX_RB3D_DC_3D_CACHE_LINESIZE_128  equ  (2 shl  2)
R5XX_RB3D_DC_2D_CACHE_AUTOFLUSH     equ  (1 shl  8)
R5XX_RB3D_DC_3D_CACHE_AUTOFLUSH     equ  (2 shl  8)
R200_RB3D_DC_2D_CACHE_AUTOFREE      equ  (1 shl 10)
R200_RB3D_DC_3D_CACHE_AUTOFREE      equ  (2 shl 10)
R5XX_RB3D_DC_FORCE_RMW              equ  (1 shl 16)
R5XX_RB3D_DC_DISABLE_RI_FILL        equ  (1 shl 24)
R5XX_RB3D_DC_DISABLE_RI_READ        equ  (1 shl 25)

R5XX_BRUSH_Y_X                      equ  0x1474
R5XX_DP_BRUSH_BKGD_CLR              equ  0x1478
R5XX_DP_BRUSH_FRGD_CLR              equ  0x147c
R5XX_BRUSH_DATA0                    equ  0x1480
R5XX_BRUSH_DATA1                    equ  0x1484

R5XX_SRC_Y_X                        equ  0x1434

R5XX_DST_Y_X                        equ  0x1438
R5XX_DST_HEIGHT_WIDTH               equ  0x143c
R5XX_DST_WIDTH_HEIGHT               equ  0x1598

R5XX_DST_LINE_START                 equ  0x1600
R5XX_DST_LINE_END                   equ  0x1604
R5XX_DST_LINE_PATCOUNT              equ  0x1608
  R5XX_BRES_CNTL_SHIFT              equ  8


R5XX_DP_SRC_BKGD_CLR                equ  0x15dc
R5XX_DP_SRC_FRGD_CLR                equ  0x15d8

R5XX_DP_WRITE_MASK                  equ  0x16cc


RADEON_CP_PACKET0                   equ 0x00000000

struc RHD
{
  .control           rd 1
  .control_saved     rd 1
  .datatype          rd 1
  .surface_cntl      rd 1
  .dst_pitch_offset  rd 1
  .ring_base         rd 1
  .ring_rp           rd 1
  .ring_wp           rd 1
};

R5XX_LOOP_COUNT equ 2000000



align 4
R5xxFIFOWaitLocal:

           mov ecx, R5XX_LOOP_COUNT
@@:
           rdr ebx, R5XX_RBBM_STATUS
           and ebx, R5XX_RBBM_FIFOCNT_MASK

           cmp eax, ebx
           jbe .done
           loop @B

           mov esi, msgR5xxFIFOWaitLocaltimeout
           call SysMsgBoardStr
           xor eax, eax
           ret
.done:
           mov eax, 1
           ret

align 4
R5xxFIFOWait:
           call R5xxFIFOWaitLocal
           test eax, eax
           jz .reset

           ret
.reset:
           call R5xx2DReset
           call R5xx2DSetup

           ret


; Wait for the graphics engine to be completely idle: the FIFO has
; drained, the Pixel Cache is flushed, and the engine is idle.  This is
; a standard "sync" function that will make the hardware "quiescent".

align 4
R5xx2DIdleLocal:

           mov ecx, R5XX_LOOP_COUNT
@@:
           rdr eax, R5XX_RBBM_STATUS
           and eax, R5XX_RBBM_FIFOCNT_MASK
           cmp eax, 0x40
           je @F
           loop @B

           mov esi, msgR5xx2DIdleLocaltimeout
           call SysMsgBoardStr
           xor eax, eax
           ret
@@:
           mov ecx, R5XX_LOOP_COUNT
@@:
           rdr eax, R5XX_RBBM_STATUS
           test eax, R5XX_RBBM_ACTIVE
           jz .done
           loop @B

           mov esi, msgR5xx2DIdleLocaltimeout
           call SysMsgBoardStr
           xor eax, eax
           ret
.done:
           call R5xx2DFlush
           ret

align 4
R5xx2DFlush:
           rmask R5XX_RB3D_DSTCACHE_CTLSTAT, R5XX_RB3D_DC_FLUSH_ALL, R5XX_RB3D_DC_FLUSH_ALL

           mov ecx, R5XX_LOOP_COUNT
@@:
           rdr eax, R5XX_RB3D_DSTCACHE_CTLSTAT
           test eax, R5XX_RB3D_DC_BUSY
           jz .done
           loop @B
.fail:
           mov esi, msgR5xx2DFlushtimeout
           call SysMsgBoardStr
           xor eax, eax
           ret
.done:
           mov eax, 1
           ret

align 4
proc R5xx2DReset
           locals
             save    rd 1
             tmp     rd 1
           endl

 ; The following RBBM_SOFT_RESET sequence can help un-wedge
 ; an R300 after the command processor got stuck.

           rdr eax, R5XX_RBBM_SOFT_RESET
           mov [save], eax

           or eax, R5XX_SOFT_RESET_CP or \
                   R5XX_SOFT_RESET_HI or R5XX_SOFT_RESET_SE or \
                   R5XX_SOFT_RESET_RE or R5XX_SOFT_RESET_PP or \
                   R5XX_SOFT_RESET_E2 or R5XX_SOFT_RESET_RB
           mov [tmp], eax

           wrr R5XX_RBBM_SOFT_RESET, eax

           rdr ebx, R5XX_RBBM_SOFT_RESET
           and eax, not (R5XX_SOFT_RESET_CP or R5XX_SOFT_RESET_HI or \
                         R5XX_SOFT_RESET_SE or R5XX_SOFT_RESET_RE or \
                         R5XX_SOFT_RESET_PP or R5XX_SOFT_RESET_E2 or \
                         R5XX_SOFT_RESET_RB)
           wrr R5XX_RBBM_SOFT_RESET, eax
           rdr ebx, R5XX_RBBM_SOFT_RESET
           mov eax, [save]
           wrr R5XX_RBBM_SOFT_RESET, eax
           rdr ebx, R5XX_RBBM_SOFT_RESET
           call R5xx2DFlush

; Soft resetting HDP thru RBBM_SOFT_RESET register can cause some
; unexpected behaviour on some machines.  Here we use
; R5XX_HOST_PATH_CNTL to reset it.

           rdr edx, R5XX_HOST_PATH_CNTL

           rdr ebx, R5XX_RBBM_SOFT_RESET

           or ebx, R5XX_SOFT_RESET_CP or R5XX_SOFT_RESET_HI or R5XX_SOFT_RESET_E2

           wrr R5XX_RBBM_SOFT_RESET, ebx

           rdr eax, R5XX_RBBM_SOFT_RESET

           wrr R5XX_RBBM_SOFT_RESET, 0

           rdr ebx, R5XX_RB3D_DSTCACHE_MODE

           or ebx, (1 shl 17)
           wrr R5XX_RB3D_DSTCACHE_MODE, ebx

           lea eax, [edx+R5XX_HDP_SOFT_RESET]
           wrr R5XX_HOST_PATH_CNTL, eax

           rdr ebx, R5XX_HOST_PATH_CNTL

           wrr R5XX_HOST_PATH_CNTL, edx

           ret
endp

align 4
R5xx2DSetup:

; Setup engine location. This shouldn't be necessary since we
; set them appropriately before any accel ops, but let's avoid
; random bogus DMA in case we inadvertently trigger the engine
; in the wrong place (happened).

           mov eax, 2
           call R5xxFIFOWaitLocal

           mov eax, [rhd.dst_pitch_offset]
           wrr R5XX_DST_PITCH_OFFSET, eax

           wrr R5XX_SRC_PITCH_OFFSET, eax

           mov eax, 1
           call R5xxFIFOWaitLocal

           rmask R5XX_DP_DATATYPE, 0, R5XX_HOST_BIG_ENDIAN_EN

           mov eax, [rhd.surface_cntl]
           wrr R5XX_SURFACE_CNTL, eax

           mov eax, 1
           call R5xxFIFOWaitLocal

           wrr R5XX_DEFAULT_SC_BOTTOM_RIGHT,\
               (R5XX_DEFAULT_SC_RIGHT_MAX or R5XX_DEFAULT_SC_BOTTOM_MAX)

           mov eax, 1
           call R5xxFIFOWaitLocal

           mov eax, [rhd.control]
           or eax, (R5XX_GMC_BRUSH_SOLID_COLOR or R5XX_GMC_SRC_DATATYPE_COLOR)
           wrr R5XX_DP_GUI_MASTER_CNTL, eax

           mov eax, 5
           call R5xxFIFOWaitLocal

           wrr R5XX_DP_BRUSH_FRGD_CLR, 0xFFFFFFFF

           wrr R5XX_DP_BRUSH_BKGD_CLR, 0x00000000

           wrr R5XX_DP_SRC_FRGD_CLR, 0xFFFFFFFF
           wrr R5XX_DP_SRC_BKGD_CLR, 0x00000000
           wrr R5XX_DP_WRITE_MASK, 0xFFFFFFFF

           call R5xx2DIdleLocal
           ret

align 4
R5xx2DPreInit:

           mov [rhd.control],\
               (R5XX_DATATYPE_ARGB8888 shl R5XX_GMC_DST_DATATYPE_SHIFT) or\
                R5XX_GMC_CLR_CMP_CNTL_DIS or R5XX_GMC_DST_PITCH_OFFSET_CNTL

           mov [rhd.datatype], R5XX_DATATYPE_ARGB8888
           mov [rhd.surface_cntl],0

           rdr eax, D1GRPH_PITCH
           shl eax, 18

           mov ebx, [r500_LFB]
           shr ebx, 10
           or eax, ebx

           mov [rhd.dst_pitch_offset], eax

           ret

RADEON_BUS_CNTL            equ 0x0030
  RADEON_BUS_MASTER_DIS    equ (1 shl 6)

align 4
R5xxCpInit:
           stdcall CreateRingBuffer, 0x8000, PG_SW+PG_NOCACHE
           test eax, eax
           jz .fail

           mov [rhd.ring_base], eax
           call GetPgAddr

           wrr RADEON_CP_RB_BASE, eax

           wrr RADEON_CP_RB_WPTR_DELAY, 0

           rdr ebx, RADEON_CP_RB_RPTR
           wrr RADEON_CP_RB_WPTR, ebx

           mov [rhd.ring_rp], ebx
           mov [rhd.ring_wp], ebx

           wrr RADEON_CP_RB_RPTR_ADDR, 0       ;ring buffer read pointer
                                               ;no update

           wrr RADEON_CP_RB_CNTL, RADEON_RB_NO_UPDATE + 12
           wrr RADEON_SCRATCH_UMSK, 0          ;no scratch update

           rdr ebx, RADEON_BUS_CNTL
           and ebx, not RADEON_BUS_MASTER_DIS

           wrr RADEON_BUS_CNTL, ebx

         ;  wrr RADEON_LAST_FRAME_REG, 0
         ;  wrr RADEON_LAST_DISPATCH_REG, 0
         ;  wrr RADEON_LAST_CLEAR_REG, 0

           call R5xx2DIdleLocal

           wrr RADEON_ISYNC_CNTL, RADEON_ISYNC_ANY2D_IDLE3D + \
                                  RADEON_ISYNC_ANY3D_IDLE2D + \
                                  RADEON_ISYNC_WAIT_IDLEGUI + \
                                  RADEON_ISYNC_CPSCRATCH_IDLEGUI
.fail:
           ret

align 4
load_microcode:

           pushfd
           cli

           call R5xx2DIdleLocal

           wrr RADEON_CP_ME_RAM_ADDR, 0

           lea esi, [R520_cp_microcode]
           mov ecx, 256
@@:
           mov eax, [esi]
           mov ebx, [esi+4]
           wrr RADEON_CP_ME_RAM_DATAH, ebx
           wrr RADEON_CP_ME_RAM_DATAL, eax
           add esi, 8
           loop @B

           popfd
           ret


align 4
R5xx2DInit:

           call R5xx2DPreInit
           wrr R5XX_RB3D_CNTL, 0

           call R5xx2DReset
           call R5xx2DSetup

           rdr eax, RADEON_AIC_CNTL                     ;disable GART
           and eax, not RADEON_PCIGART_TRANSLATE_EN
           wrr RADEON_AIC_CNTL, eax

           call load_microcode

           call R5xxCpInit

           rdr eax, D1GRPH_X_END
           rdr ebx, D1GRPH_Y_END
           dec eax
           dec ebx

           mov [__xmin], 0         ;set clip
           mov [__ymin], 0
           mov [__xmax], eax
           mov [__ymax], ebx

           wrr RADEON_CP_CSQ_CNTL, RADEON_CSQ_PRIBM_INDBM

       ;    BEGIN_RING
       ;    RADEON_PURGE_CACHE
       ;    RADEON_PURGE_ZCACHE
       ;    RADEON_WAIT_UNTIL_IDLE
       ;    COMMIT_RING

           ret

proc R5xxSetupForSolidFill stdcall,color:dword, rop:dword, planemask:dword

           mov edx, [rop]
           mov edx, [R5xxRops+4+edx*8]
           or edx, [rhd.control]
           or edx, (R5XX_GMC_BRUSH_SOLID_COLOR or R5XX_GMC_SRC_DATATYPE_COLOR)

; Save for later clipping */
           mov [rhd.control_saved], edx

           mov eax, 4
           call R5xxFIFOWait

           wrr R5XX_DP_GUI_MASTER_CNTL, edx

           mov eax, [color]
           wrr R5XX_DP_BRUSH_FRGD_CLR, eax

           mov ebx, [planemask]
           wrr R5XX_DP_WRITE_MASK, ebx

           wrr R5XX_DP_CNTL, (R5XX_DST_X_LEFT_TO_RIGHT or R5XX_DST_Y_TOP_TO_BOTTOM)

           ret
 endp

align 4
proc R5xxSolidFillRect stdcall, x:dword, y:dword, w:dword, h:dword

           mov eax, 3
           call R5xxFIFOWait

           mov eax, [rhd.dst_pitch_offset]
           wrr R5XX_DST_PITCH_OFFSET, eax

           mov ebx, [y]
           shl ebx, 16
           mov bx, word [x]
           wrr R5XX_DST_Y_X, ebx

           mov ecx, [w]
           shl ecx, 16
           mov cx, word [h]
           wrr R5XX_DST_WIDTH_HEIGHT, ecx

           ret
endp

handle     equ  IOCTL.handle
io_code    equ  IOCTL.io_code
input      equ  IOCTL.input
inp_size   equ  IOCTL.inp_size
output     equ  IOCTL.output
out_size   equ  IOCTL.out_size

SRV_GETVERSION  equ 0
SOLID_FILL      equ 1
LINE_2P         equ 2

align 4
proc r500_entry stdcall, state:dword

.close:
          ; call r500_close

           xor eax, eax
           ret
endp

CURRENT_TASK	    equ (OS_BASE+0x0003000)
TASK_COUNT	    equ (OS_BASE+0x0003004)
WIN_STACK 	    equ (OS_BASE+0x000C000)


align 4
proc r500_HDraw stdcall, ioctl:dword

           mov ebx, [ioctl]
           mov eax, [ebx+io_code]
           cmp eax, LINE_2P
           ja .fail

           cmp eax, SRV_GETVERSION
           jne @F

           mov eax, [ebx+output]
           cmp [ebx+out_size], 4
           jne .fail
           mov [eax], dword API_VERSION
           xor eax, eax
           ret
@@:
           mov edx, [CURRENT_TASK]
           movzx edx, word [WIN_STACK+edx*2]
           cmp edx, [TASK_COUNT]
           jne .skip                                   ;skip if window inactive

           cmp eax, SOLID_FILL
           jne @F

           cmp [ebx+inp_size], 5
           jne .fail

           mov esi, [ebx+input]
           call solid_fill
.skip:
           xor eax, eax
           ret
@@:
           cmp eax, LINE_2P
           jne @F

           cmp [ebx+inp_size], 5
           jne .fail

           mov esi, [ebx+input]
           call solid_line
           xor eax, eax
           ret
@@:

.fail:
           or eax, -1
           ret
endp

restore   handle
restore   io_code
restore   input
restore   inp_size
restore   output
restore   out_size

struc FILL
{
  .color  rd 1
  .x      rd 1
  .y      rd 1
  .w      rd 1
  .h      rd 1
}

virtual at 0
  FILL FILL
end virtual

struc LINE2P
{
  .color  rd 1
  .x1      rd 1
  .y1      rd 1
  .x2      rd 1
  .y2      rd 1
}

virtual at 0
  LINE2P LINE2P
end virtual

GXcopy  equ 3

RADEON_CP_PACKET3         equ 0xC0000000

PAINT_MULTI               equ 0xC0009A00

DST_PITCH_OFFSET_CNTL     equ (  1 shl  1)
BRUSH_SOLID_COLOR         equ ( 13 shl  4)
COLOR_ARGB                equ (  6 shl  8)
SRC_DATATYPE_COLOR        equ (  3 shl 12)

;RADEON_ROP3_P             equ

; esi= input params
align 4
solid_fill:

           mov ebx, [esi+FILL.x]
           mov ecx, [esi+FILL.y]
           mov eax, [esi+FILL.w]
           mov edx, [esi+FILL.h]

           lea eax, [eax+ebx-1]     ;x2
           lea edx, [edx+ecx-1]     ;y2

           push edx                 ;y2
           push eax                 ;x2

           mov eax, esp             ;&x2
           lea ebx, [esp+4]         ;&y2

           lea ecx, [esi+FILL.x]
           lea edx, [esi+FILL.y]

           push ebx                 ;&y2
           push eax                 ;&x2
           push edx                 ;&y1
           push ecx                 ;&x1

           call _BlockClip
           add esp, 16
           test eax, eax
           jnz .exit

           ;mov edx, [R5xxRops+4+GXcopy*8]
           ;or edx, [rhd.control]
           ;or edx, (R5XX_GMC_BRUSH_SOLID_COLOR or R5XX_GMC_SRC_DATATYPE_COLOR)

           pushfd
           cli


       BEGIN_RING
         OUT_PACKET3 PAINT_MULTI, 4
         OUT_RING (DST_PITCH_OFFSET_CNTL + \
                     BRUSH_SOLID_COLOR     + \
                     COLOR_ARGB            + \
                     SRC_DATATYPE_COLOR    + \
                     (1 shl 28)+(1 shl 30) + \
                     R5XX_ROP3_P)

         OUT_RING [rhd.dst_pitch_offset]
         OUT_RING [esi+FILL.color]

           mov ebx, [esi+FILL.y]
           shl ebx, 16
           mov bx, word [esi+FILL.x]
         OUT_RING ebx

           mov ecx, [esp+4]             ;x2
           sub ecx, [esi+FILL.x]
           inc ecx                      ;w

           mov eax, [esp+8]             ;y2
           sub eax, [esi+FILL.y]
           inc eax                      ;h

           shl ecx, 16
           mov cx, ax                   ;w|h

         OUT_RING ecx
       COMMIT_RING

if 0
;           mov eax, 7
;           call R5xxFIFOWait

;           wrr R5XX_DP_GUI_MASTER_CNTL, edx

;           mov eax, [esi+FILL.color]
;           wrr R5XX_DP_BRUSH_FRGD_CLR, eax

;           wrr R5XX_DP_WRITE_MASK, 0xFFFFFFFF

;           wrr R5XX_DP_CNTL, (R5XX_DST_X_LEFT_TO_RIGHT or R5XX_DST_Y_TOP_TO_BOTTOM)

;           mov eax, [rhd.dst_pitch_offset]
;           wrr R5XX_DST_PITCH_OFFSET, eax

;           mov ebx, [esi+FILL.y]
;           shl ebx, 16
;           mov bx, word [esi+FILL.x]
;           wrr R5XX_DST_Y_X, ebx

;           mov ecx, [esp+4]             ;x2
;           sub ecx, [esi+FILL.x]
;           inc ecx                      ;w

;           mov eax, [esp+8]             ;y2
;           sub eax, [esi+FILL.y]
;           inc eax                      ;h

;           shl ecx, 16
;           mov cx, ax                        ;w|h
;           wrr R5XX_DST_WIDTH_HEIGHT, ecx
end if
           popfd
.exit:
           add esp, 8
           ret

align 4
solid_line:

           lea eax, [esi+LINE2P.y2]
           lea ebx, [esi+LINE2P.x2]
           lea ecx, [esi+LINE2P.y1]
           lea edx, [esi+LINE2P.x1]

           push eax
           push ebx
           push ecx
           push edx

           call _LineClip
           add esp, 16
           test eax, eax
           jnz .exit

           mov edx, [R5xxRops+4+GXcopy*8]
           or edx, [rhd.control]
           or edx, (R5XX_GMC_BRUSH_SOLID_COLOR or R5XX_GMC_SRC_DATATYPE_COLOR)

           pushfd
           cli

           mov eax, 7
           call R5xxFIFOWait

           wrr R5XX_DST_LINE_PATCOUNT, (0x55 shl R5XX_BRES_CNTL_SHIFT)
           wrr R5XX_DP_GUI_MASTER_CNTL, edx

           mov eax, [esi+FILL.color]
           wrr R5XX_DP_BRUSH_FRGD_CLR, eax

           wrr R5XX_DP_WRITE_MASK, 0xFFFFFFFF

           mov eax, [rhd.dst_pitch_offset]
           wrr R5XX_DST_PITCH_OFFSET, eax

           mov ebx, [esi+LINE2P.y1]
           shl ebx, 16
           mov bx, word [esi+LINE2P.x1]
           wrr R5XX_DST_LINE_START, ebx

           mov ecx, [esi+LINE2P.y2]
           shl ecx, 16
           mov cx, word [esi+LINE2P.x2]
           wrr R5XX_DST_LINE_END, ecx
           popfd
.exit:
           ret

align 4
__L1OutCode:
           cmp     eax, [__xmin]
           mov     ecx, edx
           setl    dl
           sal     edx, 3
           cmp     eax, [__xmax]
           jle     L9
           or      edx, 4
L9:
           cmp     ecx, [__ymin]
           jge     L11
           or      edx, 1
L11:
           cmp     ecx, [__ymax]
           jle     L13
           or      edx, 2
L13:
           movzx   eax, dl
           ret

align 4
_line_inter:
           push    ebp
           mov     ebp, edx
           push    edi
           push    esi
           push    ebx
           sub     esp, 4
           mov     ebx, [eax]
           mov     [esp], eax
           mov     edx, [esp+24]
           mov     edi, [ebp]
           sub     ecx, ebx
           mov     eax, ecx
           sar     eax, 31
           sub     edx, edi
           mov     esi, eax
           xor     esi, ecx
           sub     esi, eax
           mov     eax, [esp+28]
           lea     ecx, [edx+edx]
           sub     eax, ebx
           cdq
           xor     eax, edx
           sub     eax, edx
           imul    ecx, eax
           test    ecx, ecx
           jle     L17
           add     ecx, esi
           jmp     L19
L17:
           sub     ecx, esi
L19:
           lea     edx, [esi+esi]
           mov     eax, ecx
           mov     ebx, edx
           cdq
           idiv    ebx
           lea     eax, [eax+edi]
           mov     [ebp], eax
           mov     eax, [esp]
           mov     edx, [esp+28]
           mov     [eax], edx
           pop     eax
           pop     ebx
           pop     esi
           pop     edi
           pop     ebp
           ret

_LineClip:
           push    ebp
           push    edi
           push    esi
           push    ebx
           mov     eax, [esp+24]
           mov     ecx, [esp+20]
           mov     ebp, [esp+28]
           mov     edi, [esp+32]
           mov     edx, [eax]
           mov     eax, [ecx]
           call    __L1OutCode
           mov     edx, [edi]
           mov     bl, al
           mov     eax, [ebp]
           call    __L1OutCode
L48:
           mov     esi, eax
L47:
           mov     eax, esi
           and     al, bl
           jne     L23
           mov     edx, esi
           cmp     bl, dl
           je      L23
           test    bl, bl
           jne     L26
           movsx   eax, dl
           test    al, 1
           je      L28
           push    [__ymin]
           mov     ecx, [esp+24]
           push    dword [ecx]
           jmp     L51
L28:
           test    al, 2
           je      L31
           push    [__ymax]
           mov     edx, [esp+24]
           push    dword [edx]
L51:
           mov     eax, [esp+32]
           mov     edx, ebp
           mov     ecx, [eax]
           mov     eax, edi
           jmp     L49
L31:
           test    al, 4
           je      L33
           push    [__xmax]
           jmp     L52
L33:
           test    al, 8
           je      L30
           push    [__xmin]
L52:
           mov     edx, [esp+28]
           push    dword [edx]
           mov     edx, edi
           mov     eax, [esp+28]
           mov     ecx, [eax]
           mov     eax, ebp
L49:
           call    _line_inter
           pop     esi
           pop     eax
L30:
           mov     edx, [edi]
           mov     eax, [ebp]
           call    __L1OutCode
           jmp     L48
L26:
           movsx   eax, bl
           test    al, 1
           je      L36
           push    [__ymin]
           jmp     L53
L36:
           test    al, 2
           je      L39
           push    [__ymax]
L53:
           push    dword [ebp]
           mov     ecx, [edi]
           mov     edx, [esp+28]
           mov     eax, [esp+32]
           jmp     L50
L39:
           test    al, 4
           je      L41
           push    [__xmax]
           jmp     L54
L41:
           test    al, 8
           je      L38
           push    [__xmin]
L54:
           push    dword [edi]
           mov     ecx, [ebp]
           mov     edx, [esp+32]
           mov     eax, [esp+28]
L50:
           call    _line_inter
           pop     edx
           pop     ecx
L38:
           mov     ecx, [esp+24]
           mov     edx, [ecx]
           mov     ecx, [esp+20]
           mov     eax, [ecx]
           call    __L1OutCode
           mov     bl, al
           jmp     L47
L23:
           pop     ebx
           movsx   eax, al
           pop     esi
           pop     edi
           pop     ebp
           ret

align 4
_block_inter:
           test    cl, 1
           push    ebx
           mov     ebx, eax
           je      L57
           mov     eax, [__ymin]
           jmp     L66
L57:
           test    cl, 2
           je      L60
           mov     eax, [__ymax]
L66:
           mov     [edx], eax
           jmp     L65
L60:
           test    cl, 4
           je      L62
           mov     eax, [__xmax]
           jmp     L67
L62:
           and     cl, 8
           je      L65
           mov     eax, [__xmin]
L67:
           mov     [ebx], eax
L65:
           pop     ebx
           ret

align 4
_BlockClip:
           push    ebp
           push    edi
           push    esi
           push    ebx
           mov     eax, [esp+24]
           mov     ecx, [esp+20]
           mov     ebp, [esp+28]
           mov     edi, [esp+32]
           mov     edx, [eax]
           mov     eax, [ecx]
           call    __L1OutCode
           mov     edx, [edi]
           mov     ebx, eax
           mov     eax, [ebp]
           call    __L1OutCode
L80:
           mov     esi, eax
L79:
           test    esi, ebx
           jne     L70
           cmp     ebx, esi
           je      L72
           test    ebx, ebx
           jne     L74
           mov     edx, edi
           mov     eax, ebp
           mov     ecx, esi
           call    _block_inter
           mov     edx, [edi]
           mov     eax, [ebp]
           call    __L1OutCode
           jmp     L80
L74:
           mov     edx, [esp+24]
           mov     ecx, ebx
           mov     eax, [esp+20]
           call    _block_inter
           mov     eax, [esp+24]
           mov     ecx, [esp+20]
           mov     edx, [eax]
           mov     eax, [ecx]
           call    __L1OutCode
           mov     ebx, eax
           jmp     L79
L72:
           mov     esi, ebx
L70:
           mov     eax, esi
           and     eax, ebx
           pop     ebx
           cwde
           pop     esi
           pop     edi
           pop     ebp
           ret
