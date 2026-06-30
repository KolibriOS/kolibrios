; === Universal Cirrus Logic GD54xx driver for KolibriOS ===
;
; The Cirrus VCLK PLL formula, VRAM-size decode and per-chip mode limits for the
; GD54xx family were derived from and verified against the Linux cirrusfb driver.
;
; Reference (Linux kernel):
;     drivers/video/fbdev/cirrusfb.c
;     Copyright 1999-2001 Jeff Garzik <jgarzik@pobox.com>
;     original cirrusfb author: Frank Neumann; based on earlier clgen.c (Frank
;     Neumann) and retz3fb.c (Jes Sorensen). Licensed under the GNU GPL.
;
; This file is a derivative work and is distributed under the GPL.
;
; VERSION 18.0 (FINAL): 16bpp and 24bpp.
; Dynamic PLL core + strict filter by hardware VRAM size!

format PE DLL native 0.05
entry START

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'

PCI_VENDOR_CIRRUS  = 0x1013
PCI_DEVICE_GD5430  = 0x00A0   ; Alpine (32-bit memory)
PCI_DEVICE_GD5434  = 0x00A8   ; Alpine (64-bit memory)
PCI_DEVICE_GD5446  = 0x00B8   ; Laguna (64-bit memory)

VGA_CRTC_INDEX     = 0x3D4
VGA_SEQ_INDEX      = 0x3C4
VGA_GC_INDEX       = 0x3CE
VGA_MISC_W         = 0x3C2
VGA_DAC_MASK       = 0x3C6

MO_WIDTH    = 0
MO_HEIGHT   = 4
MO_BPP      = 8
MO_PITCH    = 12
MO_ROWOFF   = 16
MO_MISC     = 20
MO_SR0E     = 21
MO_SR1E     = 22
MO_CRTC_PTR = 24
MO_CRTC_LEN = 28

REASON_INIT   = 1
IOCTL_io_code = 4
IOCTL_input   = 8
IOCTL_output  = 16
VBE_GET_MODES = 1
VBE_SET_MODE  = 2
VBE_GET_INFO  = 3
MAX_MODES     = 6
REC_SIZE      = 24

proc START c uses ebx esi edi, reason:dword, cmdline:dword
        cmp     [reason], REASON_INIT
        jne     .stop
        mov     esi, msg_start
        call    dword [SysMsgBoardStr]

        call    FindCirrus
        test    eax, eax
        jz      .stop

        mov     esi, msg_found
        call    dword [SysMsgBoardStr]
        mov     esi, msg_devid
        call    dword [SysMsgBoardStr]
        movzx   eax, word [found_devid]
        call    LogHexDword
        mov     esi, msg_bar0
        call    dword [SysMsgBoardStr]
        mov     eax, [lfb_phys]
        call    LogHexDword

        call    DetectVRAM
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
        mov     eax, [lfb_phys]
        mov     [ebx + 4], eax
        xor     eax, eax
        ret
.bad:
        or      eax, -1
        ret
endp

proc BuildModeTable
        mov     edi, mode_table
        mov     dword [mode_count], 0

        ; Select candidate table by chip ID
        mov     ax, [found_devid]
        cmp     ax, PCI_DEVICE_GD5430
        je      .use_5430_table

.use_5434_table:
        mov     esi, cand_table_5434
        jmp     .loop

.use_5430_table:
        mov     esi, cand_table_5430

.loop:
        mov     eax, [esi + 0]
        test    eax, eax
        jz      .done
        mov     ebx, [esi + 4]
        mov     ecx, [esi + 8]

.check_mem:
        ; Physical VRAM requirement: (width * height * bpp) / 8192 = KB
        push    eax
        imul    eax, ebx
        imul    eax, ecx
        shr     eax, 13
        cmp     eax, [vram_kb]
        pop     eax
        ja      .skip                   ; Does not fit in memory - hide the mode!

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
        mov     edx, [lfb_phys]
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

proc SwitchMode
        pushad

        ; 1. Unlock extended registers
        mov     dx, VGA_SEQ_INDEX
        mov     ax, 0x1206
        out     dx, ax

        ; 2. Frequency synthesizer (VCLK)
        mov     al, 0x0E
        mov     ah, [esi + MO_SR0E]
        out     dx, ax

        mov     al, 0x1E
        mov     ah, [esi + MO_SR1E]
        out     dx, ax

        ; Reset extended address
        mov     al, 0x14
        mov     ah, 0x00
        out     dx, ax

        ; 3. Misc Output
        mov     dx, VGA_MISC_W
        mov     al, [esi + MO_MISC]
        out     dx, al

        ; 4. Sequencer
        mov     dx, VGA_SEQ_INDEX
        mov     ax, 0x0100
        out     dx, ax
        mov     ax, 0x2101
        out     dx, ax
        mov     ax, 0x0F02
        out     dx, ax
        mov     ax, 0x0003
        out     dx, ax
        mov     ax, 0x0E04
        out     dx, ax
        mov     ax, 0x0300
        out     dx, ax

        ; 5. Graphics Controller
        mov     dx, VGA_GC_INDEX
        mov     ax, 0x0000
        out     dx, ax
        mov     ax, 0x0001
        out     dx, ax
        mov     ax, 0x0003
        out     dx, ax
        mov     ax, 0x4005
        out     dx, ax
        mov     ax, 0x0106
        out     dx, ax
        mov     ax, 0xFF07
        out     dx, ax
        mov     ax, 0xFF08
        out     dx, ax
        mov     ax, 0x010B
        out     dx, ax

        ; 6. Unlock CRTC
        mov     dx, VGA_CRTC_INDEX
        mov     al, 0x11
        out     dx, al
        inc     dx
        in      al, dx
        and     al, 0x7F
        out     dx, al
        dec     dx

        ; 7. Load CRTC timings
        mov     edi, [esi + MO_CRTC_PTR]
        mov     ecx, [esi + MO_CRTC_LEN]
.crtc_loop:
        mov     ax, [edi]
        out     dx, ax
        add     edi, 2
        loop    .crtc_loop

        ; 8. Row stride (offset)
        mov     eax, [esi + MO_ROWOFF]
        mov     ebx, eax

        mov     ah, al
        mov     al, 0x13
        out     dx, ax

        mov     al, 0x1B
        out     dx, al
        inc     dx
        in      al, dx
        dec     edx

        and     al, 0xCD

        shr     ebx, 8
        and     bl, 3
        shl     bl, 4
        or      al, bl
        or      al, 0x02

        mov     ah, al
        mov     al, 0x1B
        out     dx, ax

        mov     ax, 0x000C
        out     dx, ax
        mov     ax, 0x000D
        out     dx, ax

        ; === 9. Enable the requested color depth (16bpp or 24bpp) ===
        mov     eax, [esi + MO_BPP]
        cmp     eax, 24
        je      .bpp24

        ; --- 16 bpp (565 HighColor) ---
        mov     dx, VGA_SEQ_INDEX
        mov     ax, 0x1707
        out     dx, ax
        mov     al, 0xC1
        call    WriteHDR
        jmp     .bpp_done

.bpp24:
        ; --- 24 bpp (888 TrueColor) ---
        mov     dx, VGA_SEQ_INDEX
        mov     ax, 0x1507
        out     dx, ax
        mov     al, 0xC5
        call    WriteHDR

.bpp_done:
        mov     dx, VGA_DAC_MASK
        mov     al, 0xFF
        out     dx, al

        mov     dx, VGA_SEQ_INDEX
        mov     ax, 0x0101
        out     dx, ax

        push    dword [lfb_phys]
        push    dword [esi + MO_PITCH]
        push    dword [esi + MO_BPP]
        push    dword [esi + MO_HEIGHT]
        push    dword [esi + MO_WIDTH]
        call    dword [SetLfbMode]

        popad
        ret
endp

proc WriteHDR
        push    eax edx
        mov     ah, al
        mov     dx, 0x3DA
        in      al, dx
        mov     dx, VGA_DAC_MASK
        in      al, dx
        in      al, dx
        in      al, dx
        in      al, dx
        mov     al, ah
        out     dx, al
        pop     edx eax
        ret
endp

proc FindCirrus
        push    ebx ecx edx esi edi
        xor     esi, esi
.scan_bus:
        xor     edi, edi
.scan_dev:
        push    0
        push    edi
        push    esi
        call    dword [PciRead32]
        cmp     ax, PCI_VENDOR_CIRRUS
        jne     .next
        shr     eax, 16
        cmp     ax, PCI_DEVICE_GD5430
        je      .found
        cmp     ax, PCI_DEVICE_GD5434
        je      .found
        cmp     ax, PCI_DEVICE_GD5446
        je      .found
        jmp     .next
.found:
        mov     [found_devid], ax
        push    0x10
        push    edi
        push    esi
        call    dword [PciRead32]
        and     eax, 0xFFFFFFF0
        mov     [lfb_phys], eax
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

proc DetectVRAM
        mov     dx, VGA_SEQ_INDEX
        mov     al, 0x0F
        out     dx, al
        inc     dx
        in      al, dx
        dec     dx

        mov     bl, al
        and     al, 0x18

        mov     ecx, 0
        cmp     al, 0x08
        je      .mem_512k
        cmp     al, 0x10
        je      .mem_1m
        cmp     al, 0x18
        je      .mem_2m
        jmp     .mem_done

.mem_512k:
        mov     ecx, 512
        jmp     .mem_bank
.mem_1m:
        mov     ecx, 1024
        jmp     .mem_bank
.mem_2m:
        mov     ecx, 2048

.mem_bank:
        test    bl, 0x80
        jz      .mem_done
        shl     ecx, 1

.mem_done:
        mov     [vram_kb], ecx
        cmp     ecx, 1024
        je      .print_1m
        cmp     ecx, 2048
        je      .print_2m
        cmp     ecx, 4096
        je      .print_4m

        mov     esi, msg_vram_unk
        jmp     .print_it
.print_1m:
        mov     esi, msg_vram_1m
        jmp     .print_it
.print_2m:
        mov     esi, msg_vram_2m
        jmp     .print_it
.print_4m:
        mov     esi, msg_vram_4m
.print_it:
        call    dword [SysMsgBoardStr]
        ret
endp

; ==========================================
; === CANDIDATE TABLES AND MODE DATA =======
; ==========================================

align 4

; --- CANDIDATES FOR GD5430 (32-bit RAM, VCLKx3) ---
cand_table_5430:
        dd 640,  480, 16, mode_640x480_16
        dd 640,  480, 24, mode_640x480_24_5430
        dd 800,  600, 16, mode_800x600_16
        dd 800,  600, 24, mode_800x600_24_5430
        dd 1024, 768, 16, mode_1024x768_16
        dd 1024, 768, 24, mode_1024x768_24_5430
        dd 0

; --- CANDIDATES FOR GD5434 / GD5446 (64-bit RAM, VCLKx1) ---
cand_table_5434:
        dd 640,  480, 16, mode_640x480_16
        dd 640,  480, 24, mode_640x480_24_5434
        dd 800,  600, 16, mode_800x600_16
        dd 800,  600, 24, mode_800x600_24_5434
        dd 1024, 768, 16, mode_1024x768_16
        dd 1024, 768, 24, mode_1024x768_24_5434
        dd 0

; ==========================================
; === MODE STRUCTURES AND CORRECT CLOCKS
; ==========================================

; --- 640x480 ---
align 4
mode_640x480_16:
        dd 640, 480, 16, 640 * 2, (640 * 2) / 8
        db 0xE3, 0x00, 0x00, 0
        dd crtc_640x480_60_16, crtc_640x480_60_16_len

mode_640x480_24_5430:
        dd 640, 480, 24, 640 * 3, (640 * 3) / 8
        db 0xEF, 0x4F, 0x1E, 0
        dd crtc_640x480_60_24, crtc_640x480_60_24_len

mode_640x480_24_5434:
        dd 640, 480, 24, 640 * 3, (640 * 3) / 8
        db 0xE3, 0x00, 0x00, 0
        dd crtc_640x480_60_24, crtc_640x480_60_24_len

; --- 800x600 ---
align 4
mode_800x600_16:
        dd 800, 600, 16, 800 * 2, (800 * 2) / 8
        db 0x0F, 0x2A, 0x1E, 0
        dd crtc_800x600_60_16, crtc_800x600_60_16_len

mode_800x600_24_5430:
        dd 800, 600, 24, 800 * 3, (800 * 3) / 8
        db 0x0F, 0x54, 0x14, 0
        dd crtc_800x600_60_24, crtc_800x600_60_24_len

mode_800x600_24_5434:
        dd 800, 600, 24, 800 * 3, (800 * 3) / 8
        db 0x0F, 0x2A, 0x1E, 0
        dd crtc_800x600_60_24, crtc_800x600_60_24_len

; --- 1024x768 ---
align 4
mode_1024x768_16:
        dd 1024, 768, 16, 1024 * 2, (1024 * 2) / 8
        db 0xEF, 0x64, 0x2C, 0
        dd crtc_1024x768_60_16, crtc_1024x768_60_16_len

mode_1024x768_24_5430:
        dd 1024, 768, 24, 1024 * 3, (1024 * 3) / 8
        db 0xEF, 0x6D, 0x10, 0
        dd crtc_1024x768_60_24, crtc_1024x768_60_24_len

mode_1024x768_24_5434:
        dd 1024, 768, 24, 1024 * 3, (1024 * 3) / 8
        db 0xEF, 0x64, 0x2C, 0
        dd crtc_1024x768_60_24, crtc_1024x768_60_24_len

; ==========================================
; === CRTC SCAN TIMINGS ==================
; ==========================================

; --- 640x480 ---
align 4
crtc_640x480_60_16:
        db 0x00, 0x5F, 0x01, 0x4F, 0x02, 0x50, 0x03, 0x82, 0x04, 0x54, 0x05, 0x80
        db 0x06, 0x0B, 0x07, 0x3E, 0x08, 0x00, 0x09, 0x40, 0x10, 0xEA, 0x11, 0x0C
        db 0x12, 0xDF, 0x14, 0x40, 0x15, 0xE7, 0x16, 0x04, 0x17, 0xE3
crtc_640x480_60_16_len = ($ - crtc_640x480_60_16) / 2

align 4
crtc_640x480_60_24:
        db 0x00, 0x5F, 0x01, 0x4F, 0x02, 0x50, 0x03, 0x82, 0x04, 0x54, 0x05, 0x80
        db 0x06, 0x0B, 0x07, 0x3E, 0x08, 0x00, 0x09, 0x40, 0x10, 0xEA, 0x11, 0x0C
        db 0x12, 0xDF, 0x14, 0x00, 0x15, 0xE7, 0x16, 0x04, 0x17, 0xE3
crtc_640x480_60_24_len = ($ - crtc_640x480_60_24) / 2

; --- 800x600 ---
align 4
crtc_800x600_60_16:
        db 0x00, 0x7F, 0x01, 0x63, 0x02, 0x63, 0x03, 0x80, 0x04, 0x6B, 0x05, 0x14
        db 0x06, 0x72, 0x07, 0xF0, 0x08, 0x00, 0x09, 0x60, 0x10, 0x58, 0x11, 0x8C
        db 0x12, 0x57, 0x14, 0x40, 0x15, 0x58, 0x16, 0x72, 0x17, 0xE3
crtc_800x600_60_16_len = ($ - crtc_800x600_60_16) / 2

align 4
crtc_800x600_60_24:
        db 0x00, 0x7F, 0x01, 0x63, 0x02, 0x63, 0x03, 0x80, 0x04, 0x6B, 0x05, 0x14
        db 0x06, 0x72, 0x07, 0xF0, 0x08, 0x00, 0x09, 0x60, 0x10, 0x58, 0x11, 0x8C
        db 0x12, 0x57, 0x14, 0x00, 0x15, 0x58, 0x16, 0x72, 0x17, 0xE3
crtc_800x600_60_24_len = ($ - crtc_800x600_60_24) / 2

; --- 1024x768 ---
align 4
crtc_1024x768_60_16:
        db 0x00, 0xA3, 0x01, 0x7F, 0x02, 0x7F, 0x03, 0x86, 0x04, 0x83, 0x05, 0x9A
        db 0x06, 0x24, 0x07, 0xFD, 0x08, 0x00, 0x09, 0x60, 0x10, 0x03, 0x11, 0x89
        db 0x12, 0xFF, 0x14, 0x40, 0x15, 0x58, 0x16, 0x72, 0x17, 0xE3
crtc_1024x768_60_16_len = ($ - crtc_1024x768_60_16) / 2

align 4
crtc_1024x768_60_24:
        db 0x00, 0xA3, 0x01, 0x7F, 0x02, 0x7F, 0x03, 0x86, 0x04, 0x83, 0x05, 0x9A
        db 0x06, 0x24, 0x07, 0xFD, 0x08, 0x00, 0x09, 0x60, 0x10, 0x03, 0x11, 0x89
        db 0x12, 0xFF, 0x14, 0x00, 0x15, 0x58, 0x16, 0x72, 0x17, 0xE3
crtc_1024x768_60_24_len = ($ - crtc_1024x768_60_24) / 2


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

align 4
found_devid        dw ?
lfb_phys           dd ?
vram_kb            dd ?
mode_count         dd ?
hexbuf             rb 12
mode_table         rb MAX_MODES * REC_SIZE
srv_name           db 'clgd54xx', 0
msg_start          db 'CirrusVGA: driver started', 13, 10, 0
msg_found          db 'CirrusVGA: Cirrus card found on PCI', 13, 10, 0
msg_devid          db 'CirrusVGA: Device ID = 0x', 0
msg_bar0           db 'CirrusVGA: BAR0 (LFB phys) = 0x', 0
msg_err_pci        db 'CirrusVGA: ERROR - Cirrus GD5430/GD5446 not found!', 13, 10, 0
msg_vram_1m        db 'CirrusVGA: VRAM = 1 MB', 13, 10, 0
msg_vram_2m        db 'CirrusVGA: VRAM = 2 MB', 13, 10, 0
msg_vram_4m        db 'CirrusVGA: VRAM = 4 MB', 13, 10, 0
msg_vram_unk       db 'CirrusVGA: VRAM = unknown', 13, 10, 0

include '../peimport.inc'
data fixups
end data
