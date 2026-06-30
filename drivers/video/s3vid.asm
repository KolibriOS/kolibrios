; === Universal S3 Trio32/Trio64/ViRGE driver for KolibriOS ===
;
; This driver is a port of the Linux kernel s3fb framebuffer driver together with
; the svgalib helper routines (svga_compute_pll / svga_set_timings / svga_wcrt_multi
; and the S3 CRTC regset tables), adapted to KolibriOS (FASM PE driver, runtime
; mode switching via the kernel SetLfbMode export).
;
; Ported from the Linux kernel:
;     drivers/video/fbdev/s3fb.c
;     drivers/video/fbdev/core/svgalib.c
;     Copyright (c) 2006-2007 Ondrej Zajicek <santiago@crfreenet.org>
;     (s3fb is based on David Boucher's viafb, itself based on neofb)
;     Licensed under the GNU General Public License (GPL).
;
; S3 register/timing behaviour was cross-checked against the 86Box emulator
;     (src/video/vid_s3.c, GPLv2) used as a hardware reference.
;
; This file is a derivative work and is distributed under the GPL.
;
; VERSION 30.0: PORT OF Linux kernel s3fb (svga_compute_pll + svga_set_timings +
; regset tables + full s3fb_set_par sequence). Timings/PLL/CRTC are computed
; at runtime from VESA parameters; 16bpp goes through hmul=2 (2:1-mux).
; Service framework (IOCTL/SetLfbMode/detect) preserved from the previous version.

format PE DLL native 0.05
entry START

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'

PCI_VENDOR_S3      = 0x5333

; --- Offsets of the VESA mode structure ---
MO_WIDTH    = 0
MO_HEIGHT   = 4
MO_BPP      = 8
MO_PITCH    = 12        ; bytes per line (width*bpp/8)
MO_PIXCLK   = 16        ; pixel clock, kHz
MO_HFP      = 20        ; right margin (front porch), pixels
MO_HSYNC    = 24        ; hsync_len
MO_HBP      = 28        ; left margin (back porch)
MO_VFP      = 32        ; lower margin
MO_VSYNC    = 36        ; vsync_len
MO_VBP      = 40        ; upper margin
MO_SYNC     = 44        ; bit0: hsync negative; bit1: vsync negative
MO_SIZE     = 48

; --- REFERENCE IOCTL OFFSETS ---
REASON_INIT   = 1
IOCTL_io_code = 4
IOCTL_input   = 8
IOCTL_output  = 16

VBE_GET_MODES = 1
VBE_SET_MODE  = 2
VBE_GET_INFO  = 3
MAX_MODES     = 6
REC_SIZE      = 24

; --- Register access macros (clobber dx, ax/al) ---
macro WCRT idx, val {
        mov     dx, 0x3D4
        mov     ax, ((val) shl 8) + (idx)
        out     dx, ax
}
macro WCRTMASK idx, val, mask {
        mov     dx, 0x3D4
        mov     al, idx
        out     dx, al
        inc     dx
        in      al, dx
        and     al, (mask) xor 0FFh
        or      al, (val) and (mask)
        out     dx, al
}
macro WSEQ idx, val {
        mov     dx, 0x3C4
        mov     ax, ((val) shl 8) + (idx)
        out     dx, ax
}
macro WSEQMASK idx, val, mask {
        mov     dx, 0x3C4
        mov     al, idx
        out     dx, al
        inc     dx
        in      al, dx
        and     al, (mask) xor 0FFh
        or      al, (val) and (mask)
        out     dx, al
}
macro WGFX idx, val {
        mov     dx, 0x3CE
        mov     ax, ((val) shl 8) + (idx)
        out     dx, ax
}
macro WATTR idx, val {
        mov     dx, 0x3DA
        in      al, dx
        mov     dx, 0x3C0
        mov     al, idx
        out     dx, al
        mov     al, val
        out     dx, al
}
; CRTC[idx] = al
macro WCRTAL idx {
        mov     ah, al
        mov     al, idx
        mov     dx, 0x3D4
        out     dx, ax
}

proc START c uses ebx esi edi, reason:dword, cmdline:dword
        cmp     [reason], REASON_INIT
        jne     .stop
        mov     esi, msg_start
        call    dword [SysMsgBoardStr]

        call    FindS3
        test    eax, eax
        jz      .stop

        mov     esi, msg_found
        call    dword [SysMsgBoardStr]
        mov     esi, msg_bar0
        call    dword [SysMsgBoardStr]
        mov     eax, [s3_lfb_phys]
        call    LogHexDword

        call    DetectVRAM
        call    LogVRAM

        call    BuildModeTable

        invoke  RegService, srv_name, service_proc
        ret
.stop:
        xor     eax, eax
        ret
endp

proc service_proc stdcall uses ebx esi edi, ioctl:dword
        mov     edi, [ioctl]
        mov     eax, [edi + IOCTL_io_code]
        cmp     eax, VBE_GET_MODES
        je      .get_modes
        cmp     eax, VBE_SET_MODE
        je      .set_mode
        cmp     eax, VBE_GET_INFO
        je      .get_info
        or      eax, -1
        ret
.get_modes:
        mov     ebx, [edi + IOCTL_output]
        test    ebx, ebx
        jz      .bad
        mov     eax, [mode_count]
        mov     [ebx], eax
        lea     ecx, [eax + eax*2]
        shl     ecx, 1
        mov     esi, mode_table
        lea     edi, [ebx + 4]
        rep     movsd
        xor     eax, eax
        ret
.set_mode:
        mov     esi, [edi + IOCTL_input]
        test    esi, esi
        jz      .bad
        mov     eax, [esi]
        cmp     eax, [mode_count]
        jae     .bad
        mov     ebx, eax
        shl     eax, 4
        lea     eax, [eax + ebx*8]
        mov     esi, dword [mode_table + eax + 12]
        call    SwitchMode
        xor     eax, eax
        ret
.get_info:
        mov     ebx, [edi + IOCTL_output]
        test    ebx, ebx
        jz      .bad
        mov     dword [ebx], 0
        mov     eax, [s3_lfb_phys]
        mov     [ebx + 4], eax
        xor     eax, eax
        ret
.bad:
        or      eax, -1
        ret
endp

proc BuildModeTable
        mov     edi, mode_table
        mov     esi, cand_table
        mov     dword [mode_count], 0
.loop:
        mov     eax, [esi + 0]
        test    eax, eax
        jz      .done
        mov     ebx, [esi + 4]
        mov     ecx, [esi + 8]
        cmp     eax, 640
        jne     .check_mem
        cmp     ebx, 480
        jne     .check_mem
        cmp     ecx, 16
        je      .add_mode
.check_mem:
        push    eax
        imul    eax, ebx
        imul    eax, ecx
        shr     eax, 13
        cmp     eax, [vram_kb]
        pop     eax
        ja      .skip
.add_mode:
        mov     edx, [mode_count]
        cmp     edx, MAX_MODES
        jae     .done
        push    edi
        imul    edx, REC_SIZE
        add     edi, edx
        mov     [edi + 0], eax
        mov     [edi + 4], ebx
        mov     [edi + 8], ecx
        mov     edx, [esi + 12]
        mov     [edi + 12], edx
        mov     edx, [s3_lfb_phys]
        mov     [edi + 16], edx
        mov     edx, eax
        imul    edx, ecx
        shr     edx, 3
        mov     [edi + 20], edx
        pop     edi
        inc     dword [mode_count]
.skip:
        add     esi, 16
        jmp     .loop
.done:
        ret
endp

; ============================================================
; === PORT of s3fb_set_par: set mode (esi = mode ptr) ===
; ============================================================
proc SwitchMode
        pushad

        ; hmul: 16bpp = 2 (2:1 multiplex), 32bpp = 1 (as in s3fb).
        cmp     dword [esi + MO_BPP], 16
        jne     .b32
        mov     dword [g_hmul], 2
        jmp     .bdone
.b32:
        mov     dword [g_hmul], 1
.bdone:
        ; offset_value = width*bpp/64
        mov     eax, [esi + MO_WIDTH]
        imul    eax, [esi + MO_BPP]
        shr     eax, 6
        mov     [g_offset], eax

        ; --- Unlock ---
        WCRT    0x38, 0x48
        WCRT    0x39, 0xA5
        WSEQ    0x08, 0x06
        WCRTMASK 0x11, 0x00, 0x80

        ; --- Blank screen + sync off ---
        WSEQMASK 0x01, 0x20, 0x20
        WCRTMASK 0x17, 0x00, 0x80

        ; --- Default registers (svga_set_default_*) ---
        call    set_default_gfx
        call    set_default_atc
        call    set_default_seq
        call    set_default_crt
        mov     edi, rs_linecmp
        mov     eax, 0xFFFFFFFF
        call    wcrt_multi
        mov     edi, rs_startaddr
        xor     eax, eax
        call    wcrt_multi

        ; --- S3-specific initialization ---
        WCRTMASK 0x58, 0x10, 0x10       ; linear framebuffer
        WCRTMASK 0x31, 0x08, 0x08       ; sequencer access >256KB
        WCRTMASK 0x33, 0x00, 0x08
        WCRTMASK 0x43, 0x00, 0x05        ; CR43 bit0=0 (DDR) + bit2=0 (high bit of rowoffset)
        WCRTMASK 0x5D, 0x00, 0x28       ; clear stray HSlen bits

        ; LFB base (CR59:CR5A) from PCI BAR (KolibriOS-specific)
        mov     eax, [s3_lfb_phys]
        shr     eax, 24
        WCRTAL  0x59
        mov     eax, [s3_lfb_phys]
        shr     eax, 16
        and     eax, 0xFF
        WCRTAL  0x5A

        ; offset register
        mov     edi, rs_offset
        mov     eax, [g_offset]
        call    wcrt_multi

        ; FIFO (M/N/L parameters) - stock s3fb values (conservative;
        ; CR54=0xF8/M=31 turned out to be LESS stable on real hardware than the stock 0x18).
        WCRT    0x54, 0x18
        WCRT    0x60, 0xFF
        WCRT    0x61, 0xFF
        WCRT    0x62, 0xFF

        WCRT    0x3A, 0x35
        WATTR   0x33, 0x00

        WCRTMASK 0x09, 0x00, 0x80       ; no double scan
        WCRTMASK 0x42, 0x00, 0x20       ; no interlace
        WCRTMASK 0x45, 0x00, 0x01       ; HW cursor off
        WCRTMASK 0x67, 0x00, 0x0C       ; streams off
        WCRT    0x34, 0x10              ; DTPC enable

        WCRTMASK 0x31, 0x00, 0x40
        WCRTMASK 0x08, 0x00, 0x60
        WCRTMASK 0x05, 0x00, 0x60

        ; --- bpp switch (Trio64 branch) ---
        cmp     dword [esi + MO_BPP], 16
        jne     .s32
        WCRTMASK 0x50, 0x10, 0x30
        WCRTMASK 0x67, 0x50, 0xF0       ; 16bpp 5-6-5
        jmp     .sdone
.s32:
        WCRTMASK 0x50, 0x30, 0x30
        WCRTMASK 0x67, 0xD0, 0xF0       ; 32bpp 8-8-8-8
.sdone:
        ; PEL mask
        mov     dx, 0x3C6
        mov     al, 0xFF
        out     dx, al

        ; multiplex = 0 (for our modes)
        WSEQMASK 0x15, 0x00, 0x10
        WSEQMASK 0x18, 0x00, 0x80

        ; PLL
        mov     eax, [esi + MO_PIXCLK]
        call    set_pixclock

        ; CRTC timings (hmul applied inside)
        call    set_timings

        ; CR3C: interlace start/end = (htotal+1)/2
        mov     eax, [esi + MO_WIDTH]
        add     eax, [esi + MO_HBP]
        add     eax, [esi + MO_HFP]
        add     eax, [esi + MO_HSYNC]
        imul    eax, [g_hmul]
        shr     eax, 3
        sub     eax, 5
        mov     [g_htotal], eax
        inc     eax
        shr     eax, 1
        WCRTAL  0x3C

        ; DTPC (Start Display FIFO):
        ;   hsstart = (W+HFP)*hmul/8
        ;   value = clamp((htotal+hsstart+1)/2+2, hsstart+4, htotal+1)
        mov     eax, [esi + MO_WIDTH]
        add     eax, [esi + MO_HFP]
        imul    eax, [g_hmul]
        shr     eax, 3
        mov     ebx, eax                ; ebx = hsstart
        mov     eax, [g_htotal]
        add     eax, ebx
        inc     eax
        shr     eax, 1
        add     eax, 2
        mov     ecx, ebx
        add     ecx, 4                  ; low = hsstart+4
        cmp     eax, ecx
        jae     @f
        mov     eax, ecx
@@:
        mov     ecx, [g_htotal]
        inc     ecx                     ; high = htotal+1
        cmp     eax, ecx
        jbe     @f
        mov     eax, ecx
@@:
        mov     edi, rs_dtpc
        call    wcrt_multi

        ; --- Screen and sync back on ---
        WCRTMASK 0x17, 0x80, 0x80
        WSEQMASK 0x01, 0x00, 0x20

        ; --- Hand off to kernel ---
        push    dword [s3_lfb_phys]
        push    dword [esi + MO_PITCH]
        push    dword [esi + MO_BPP]
        push    dword [esi + MO_HEIGHT]
        push    dword [esi + MO_WIDTH]
        call    dword [SetLfbMode]

        popad
        ret
endp

; --- svga_wcrt_multi: edi = regset, eax = value ---
wcrt_multi:
        push    eax ebx ecx edx esi edi
        mov     [mwval], eax
.next:
        mov     al, [edi]
        cmp     al, 0xFF
        je      .done
        mov     [mw_reg], al
        mov     al, [edi + 1]
        mov     [mw_low], al
        mov     al, [edi + 2]
        mov     [mw_high], al
        ; read the current register value
        mov     dx, 0x3D4
        mov     al, [mw_reg]
        out     dx, al
        inc     dx
        in      al, dx
        mov     bl, al                  ; bl = working copy
        mov     cl, [mw_low]
.bit:
        cmp     cl, [mw_high]
        ja      .write
        mov     bh, 1
        shl     bh, cl                  ; bh = 1<<bitnum
        mov     al, bh
        xor     al, 0xFF
        and     bl, al                  ; clear the bit
        test    dword [mwval], 1
        jz      .noset
        or      bl, bh
.noset:
        shr     dword [mwval], 1
        inc     cl
        jmp     .bit
.write:
        mov     dx, 0x3D4
        mov     al, [mw_reg]
        out     dx, al
        inc     dx
        mov     al, bl
        out     dx, al
        add     edi, 3
        jmp     .next
.done:
        pop     edi esi edx ecx ebx eax
        ret

; --- svga_compute_pll: eax = f_wanted (kHz) -> [pll_m]/[pll_n]/[pll_r] ---
; s3_pll = {m 3..129, n 3..33, r 0..3, fvco 35000..240000, fbase 14318}
compute_pll:
        push    eax ebx ecx edx
        mov     ecx, 3                  ; ar = r_max
        mov     ebx, eax
        shl     ebx, 3                  ; fvco = f<<3
.shr:
        cmp     ecx, 0
        jle     .vd
        cmp     ebx, 240000
        jbe     .vd
        dec     ecx
        shr     ebx, 1
        jmp     .shr
.vd:
        mov     [pll_r], ecx
        mov     [cp_fvco], ebx
        mov     dword [cp_dbest], 0xFFFFFFFF
        mov     dword [pll_m], 0
        mov     dword [pll_n], 0
        mov     dword [cp_am], 3
        mov     dword [cp_an], 3
.search:
        mov     eax, [cp_am]
        cmp     eax, 129
        jg      .sd
        mov     eax, [cp_an]
        cmp     eax, 33
        jg      .sd
        ; f_current = 14318*am/an
        mov     eax, 14318
        imul    eax, [cp_am]
        xor     edx, edx
        div     dword [cp_an]
        ; delta = abs(f_current - fvco)
        mov     ebx, eax
        sub     ebx, [cp_fvco]
        jns     .pos
        neg     ebx
.pos:
        cmp     ebx, [cp_dbest]
        jae     .nob
        mov     [cp_dbest], ebx
        mov     edx, [cp_am]
        mov     [pll_m], edx
        mov     edx, [cp_an]
        mov     [pll_n], edx
.nob:
        cmp     eax, [cp_fvco]
        ja      .incn
        inc     dword [cp_am]
        jmp     .search
.incn:
        inc     dword [cp_an]
        jmp     .search
.sd:
        pop     edx ecx ebx eax
        ret

; --- s3_set_pixclock: eax = f_wanted (kHz) ---
set_pixclock:
        call    compute_pll
        ; MISC |= 0x0C (select PLL clock), RMW preserving sync polarity
        mov     dx, 0x3CC
        in      al, dx
        or      al, 0x0C
        mov     dx, 0x3C2
        out     dx, al
        ; SR12 = (n-2) | (r<<5)
        mov     eax, [pll_n]
        sub     eax, 2
        mov     ebx, [pll_r]
        shl     ebx, 5
        or      eax, ebx
        mov     ah, al
        mov     al, 0x12
        mov     dx, 0x3C4
        out     dx, ax
        ; SR13 = m-2
        mov     eax, [pll_m]
        sub     eax, 2
        mov     ah, al
        mov     al, 0x13
        mov     dx, 0x3C4
        out     dx, ax
        ; pause ~1ms (busy-wait, sleep is not allowed in the driver)
        mov     ecx, 0x300000
.dl:
        dec     ecx
        jnz     .dl
        ; latch: SR15 bit5 = 0 -> 1 -> 0
        mov     dx, 0x3C4
        mov     al, 0x15
        out     dx, al
        inc     dx
        in      al, dx
        and     al, not 0x20
        mov     bl, al
        out     dx, al
        or      al, 0x20
        out     dx, al
        mov     al, bl
        out     dx, al
        ret

; --- svga_set_timings: esi = mode ptr, [g_hmul] = hmul (hdiv=vmul=vdiv=1, hborder=hmul) ---
set_timings:
        ; h_total = (W+HBP+HFP+HSYNC)*hmul/8 - 5
        mov     eax, [esi + MO_WIDTH]
        add     eax, [esi + MO_HBP]
        add     eax, [esi + MO_HFP]
        add     eax, [esi + MO_HSYNC]
        imul    eax, [g_hmul]
        shr     eax, 3
        sub     eax, 5
        mov     edi, rs_htotal
        call    wcrt_multi
        ; h_display = W*hmul/8 - 1
        mov     eax, [esi + MO_WIDTH]
        imul    eax, [g_hmul]
        shr     eax, 3
        dec     eax
        mov     edi, rs_hdisp
        call    wcrt_multi
        ; h_blank_start = W*hmul/8 - 1 + hborder(=hmul)
        mov     eax, [esi + MO_WIDTH]
        imul    eax, [g_hmul]
        shr     eax, 3
        dec     eax
        add     eax, [g_hmul]
        mov     edi, rs_hbstart
        call    wcrt_multi
        ; h_blank_end = (W+HBP+HFP+HSYNC)*hmul/8 - 1 - hborder(=hmul)
        mov     eax, [esi + MO_WIDTH]
        add     eax, [esi + MO_HBP]
        add     eax, [esi + MO_HFP]
        add     eax, [esi + MO_HSYNC]
        imul    eax, [g_hmul]
        shr     eax, 3
        dec     eax
        sub     eax, [g_hmul]
        mov     edi, rs_hbend
        call    wcrt_multi
        ; h_sync_start = (W+HFP)*hmul/8
        mov     eax, [esi + MO_WIDTH]
        add     eax, [esi + MO_HFP]
        imul    eax, [g_hmul]
        shr     eax, 3
        mov     edi, rs_hsstart
        call    wcrt_multi
        ; h_sync_end = (W+HFP+HSYNC)*hmul/8
        mov     eax, [esi + MO_WIDTH]
        add     eax, [esi + MO_HFP]
        add     eax, [esi + MO_HSYNC]
        imul    eax, [g_hmul]
        shr     eax, 3
        mov     edi, rs_hsend
        call    wcrt_multi
        ; v_total = (H+VBP+VFP+VSYNC) - 2
        mov     eax, [esi + MO_HEIGHT]
        add     eax, [esi + MO_VBP]
        add     eax, [esi + MO_VFP]
        add     eax, [esi + MO_VSYNC]
        sub     eax, 2
        mov     edi, rs_vtotal
        call    wcrt_multi
        ; v_display = H - 1
        mov     eax, [esi + MO_HEIGHT]
        dec     eax
        mov     edi, rs_vdisp
        call    wcrt_multi
        ; v_blank_start = H
        mov     eax, [esi + MO_HEIGHT]
        mov     edi, rs_vbstart
        call    wcrt_multi
        ; v_blank_end = (H+VBP+VFP+VSYNC) - 2
        mov     eax, [esi + MO_HEIGHT]
        add     eax, [esi + MO_VBP]
        add     eax, [esi + MO_VFP]
        add     eax, [esi + MO_VSYNC]
        sub     eax, 2
        mov     edi, rs_vbend
        call    wcrt_multi
        ; v_sync_start = H + VFP
        mov     eax, [esi + MO_HEIGHT]
        add     eax, [esi + MO_VFP]
        mov     edi, rs_vsstart
        call    wcrt_multi
        ; v_sync_end = H + VFP + VSYNC
        mov     eax, [esi + MO_HEIGHT]
        add     eax, [esi + MO_VFP]
        add     eax, [esi + MO_VSYNC]
        mov     edi, rs_vsend
        call    wcrt_multi
        ; sync polarity in MISC
        mov     dx, 0x3CC
        in      al, dx
        mov     bl, [esi + MO_SYNC]
        test    bl, 1
        jz      .hpos
        or      al, 0x80
        jmp     .hd
.hpos:
        and     al, not 0x80
.hd:
        test    bl, 2
        jz      .vpos
        or      al, 0x40
        jmp     .vd2
.vpos:
        and     al, not 0x40
.vd2:
        mov     dx, 0x3C2
        out     dx, al
        ret

; --- Default VGA registers (svga_set_default_*) ---
set_default_gfx:
        WGFX    0x00, 0x00
        WGFX    0x01, 0x00
        WGFX    0x02, 0x00
        WGFX    0x03, 0x00
        WGFX    0x04, 0x00
        WGFX    0x05, 0x00
        WGFX    0x06, 0x05
        WGFX    0x07, 0x0F
        WGFX    0x08, 0xFF
        ret

set_default_atc:
        mov     dx, 0x3DA
        in      al, dx
        mov     dx, 0x3C0
        xor     al, al
        out     dx, al                  ; index 0, PAS=0 (video off)
        xor     ecx, ecx
.al:
        mov     dx, 0x3DA
        in      al, dx
        mov     dx, 0x3C0
        mov     al, cl
        out     dx, al
        mov     al, cl
        out     dx, al
        inc     cl
        cmp     cl, 0x10
        jb      .al
        WATTR   0x10, 0x01
        WATTR   0x11, 0x00
        WATTR   0x12, 0x0F
        WATTR   0x13, 0x00
        WATTR   0x14, 0x00
        mov     dx, 0x3DA
        in      al, dx
        mov     dx, 0x3C0
        mov     al, 0x20                ; PAS=1 (video on)
        out     dx, al
        ret

set_default_seq:
        WSEQ    0x01, 0x21              ; 8 dots/char + screen off (bit5)
        WSEQ    0x02, 0x0F
        WSEQ    0x03, 0x00
        WSEQ    0x04, 0x06              ; EXT_MEM | SEQ_MODE
        ret

set_default_crt:
        WCRTMASK 0x03, 0x80, 0x80
        WCRT    0x08, 0x00
        WCRTMASK 0x09, 0x00, 0x1F
        WCRT    0x14, 0x00
        WCRT    0x17, 0xE3
        ret

proc FindS3
        push    ebx ecx edx esi edi
        xor     esi, esi
.scan_bus:
        xor     edi, edi
.scan_dev:
        push    0
        push    edi
        push    esi
        call    dword [PciRead32]
        cmp     ax, PCI_VENDOR_S3
        jne     .next
        push    0x10
        push    edi
        push    esi
        call    dword [PciRead32]
        and     eax, 0xFFFFFFF0
        mov     [s3_lfb_phys], eax
        mov     eax, 1
        jmp     .done
.next:
        inc     edi
        cmp     edi, 256
        jb      .scan_dev
        inc     esi
        cmp     esi, 8
        jb      .scan_bus
        xor     eax, eax
.done:
        pop     edi esi edx ecx ebx
        ret
endp

proc LogHexDword
        pushad
        mov     edi, hexbuf
        mov     ecx, 8
.nib:   rol     eax, 4
        mov     bl, al
        and     bl, 0x0F
        add     bl, '0'
        cmp     bl, '9'
        jbe     @f
        add     bl, 7
@@:     mov     [edi], bl
        inc     edi
        loop    .nib
        mov     byte [edi],   13
        mov     byte [edi+1], 10
        mov     byte [edi+2], 0
        mov     esi, hexbuf
        call    dword [SysMsgBoardStr]
        popad
        ret
endp

; === SAFE VRAM DETECTION ===
proc DetectVRAM
        pushad
        mov     dx, 0x3D4
        in      al, dx
        mov     cl, al
        mov     ax, 0x4838
        out     dx, ax
        mov     ax, 0xA539
        out     dx, ax
        mov     al, 0x36
        out     dx, al
        inc     dx
        in      al, dx
        dec     dx
        mov     ch, al
        mov     al, cl
        out     dx, al
        mov     al, ch
        shr     al, 5
        movzx   eax, al
        mov     eax, [s3_memsizes + eax*4]
        mov     [vram_kb], eax
        popad
        ret
endp

proc LogVRAM
        pushad
        mov     eax, [vram_kb]
        xor     edx, edx
        mov     ecx, 512
        div     ecx
        mov     ebp, eax
        and     ebp, 1
        shr     eax, 1
        mov     edi, membuf
        call    PutDecEAX
        mov     byte [edi], '.'
        inc     edi
        test    ebp, ebp
        jz      .zero
        mov     byte [edi], '5'
        jmp     .suffix
.zero:
        mov     byte [edi], '0'
.suffix:
        inc     edi
        mov     byte [edi],   ' '
        mov     byte [edi+1], 'M'
        mov     byte [edi+2], 'B'
        mov     byte [edi+3], 13
        mov     byte [edi+4], 10
        mov     byte [edi+5], 0
        mov     esi, msg_vram
        call    dword [SysMsgBoardStr]
        mov     esi, membuf
        call    dword [SysMsgBoardStr]
        popad
        ret
endp

PutDecEAX:
        push    ebx ecx edx
        mov     ebx, 10
        xor     ecx, ecx
        test    eax, eax
        jnz     .loop
        mov     byte [edi], '0'
        inc     edi
        jmp     .done
.loop:
        xor     edx, edx
        div     ebx
        push    edx
        inc     ecx
        test    eax, eax
        jnz     .loop
.pop:
        pop     edx
        add     dl, '0'
        mov     [edi], dl
        inc     edi
        dec     ecx
        jnz     .pop
.done:
        pop     edx ecx ebx
        ret

; ============================================================
; === DATA ===
; ============================================================
align 4
cand_table:
        dd 640,  480, 16, mode_640x480_16
        dd 640,  480, 32, mode_640x480_32
        dd 800,  600, 16, mode_800x600_16
        dd 800,  600, 32, mode_800x600_32
        dd 1024, 768, 16, mode_1024x768_16
        dd 1024, 768, 32, mode_1024x768_32
        dd 0

; VESA timings @60Hz: pixclk(kHz), HFP(right),HSYNC,HBP(left), VFP(lower),VSYNC,VBP(upper), sync
align 4
mode_640x480_16:
        dd 640, 480, 16, 640*2,  25175,  16, 96, 48,   10, 2, 33,  3
mode_640x480_32:
        dd 640, 480, 32, 640*4,  25175,  16, 96, 48,   10, 2, 33,  3
mode_800x600_16:
        dd 800, 600, 16, 800*2,  40000,  40, 128, 88,   1, 4, 23,  0
mode_800x600_32:
        dd 800, 600, 32, 800*4,  40000,  40, 128, 88,   1, 4, 23,  0
mode_1024x768_16:
        dd 1024, 768, 16, 1024*2, 65000, 24, 136, 160,  3, 6, 29,  3
mode_1024x768_32:
        dd 1024, 768, 32, 1024*4, 65000, 24, 136, 160,  3, 6, 29,  3

; s3fb regset tables (db regnum, lowbit, highbit, ..., 0xFF)
align 4
rs_htotal:    db 0x00,0,7, 0x5D,0,0, 0xFF
rs_hdisp:     db 0x01,0,7, 0x5D,1,1, 0xFF
rs_hbstart:   db 0x02,0,7, 0x5D,2,2, 0xFF
rs_hbend:     db 0x03,0,4, 0x05,7,7, 0xFF
rs_hsstart:   db 0x04,0,7, 0x5D,4,4, 0xFF
rs_hsend:     db 0x05,0,4, 0xFF
rs_vtotal:    db 0x06,0,7, 0x07,0,0, 0x07,5,5, 0x5E,0,0, 0xFF
rs_vdisp:     db 0x12,0,7, 0x07,1,1, 0x07,6,6, 0x5E,1,1, 0xFF
rs_vbstart:   db 0x15,0,7, 0x07,3,3, 0x09,5,5, 0x5E,2,2, 0xFF
rs_vbend:     db 0x16,0,7, 0xFF
rs_vsstart:   db 0x10,0,7, 0x07,2,2, 0x07,7,7, 0x5E,4,4, 0xFF
rs_vsend:     db 0x11,0,3, 0xFF
rs_linecmp:   db 0x18,0,7, 0x07,4,4, 0x09,6,6, 0x5E,6,6, 0xFF
rs_startaddr: db 0x0D,0,7, 0x0C,0,7, 0x69,0,4, 0xFF
rs_offset:    db 0x13,0,7, 0x51,4,5, 0xFF
rs_dtpc:      db 0x3B,0,7, 0x5D,6,6, 0xFF

align 4
s3_memsizes        dd 4096, 0, 3072, 8192, 2048, 6144, 1024, 512

align 4
s3_lfb_phys        dd ?
vram_kb            dd ?
mode_count         dd ?
g_hmul             dd ?
g_offset           dd ?
g_htotal           dd ?
pll_m              dd ?
pll_n              dd ?
pll_r              dd ?
cp_fvco            dd ?
cp_dbest           dd ?
cp_am              dd ?
cp_an              dd ?
mwval              dd ?
mw_reg             db ?
mw_low             db ?
mw_high            db ?
hexbuf             rb 12
membuf             rb 16
mode_table         rb MAX_MODES * REC_SIZE
srv_name           db 's3vid', 0

msg_start          db 'S3vid: Driver started', 13, 10, 0
msg_found          db 'S3vid: S3 card identified', 13, 10, 0
msg_bar0           db 'S3vid: BAR0 (LFB phys) = 0x', 0
msg_vram           db 'S3vid: VRAM = ', 0

include '../peimport.inc'
data fixups
end data
