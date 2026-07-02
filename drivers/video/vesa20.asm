; === VBE resolution-switching service for KolibriOS ===
; On load: queries VBE via the kernel V86 monitor (int 10h), builds
; a table of LFB modes (>=15bpp) and registers the "vesa20" service.
; IOCTL (sysfn 68.17):
;   io_code = 1 (GET_MODES): output <- [dd count][record x count], record =
;             {dd w, dd h, dd bpp, dd mode, dd phys, dd pitch} = 24 bytes.
;   io_code = 2 (SET_MODE):  input  -> [dd index]; sets the mode, calls SetLfbMode.
; Requires kernel exports: RegService, V86start, V86machine, SetLfbMode.

format PE DLL native 0.05
entry START

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'

REASON_INIT   = 1

; ---- IOCTL ----
IOCTL_handle   = 0
IOCTL_io_code  = 4
IOCTL_input    = 8
IOCTL_inp_size = 12
IOCTL_output   = 16
IOCTL_out_size = 20

VBE_GET_MODES  = 1
VBE_SET_MODE   = 2
VBE_GET_INFO   = 3      ; output <- [dd vbe_version][dd pci_lfb]

MAX_MODES      = 48
REC_SIZE       = 24

; ---- addresses in the first megabyte (visible to V86: 0x98000-0x9FFFF is mapped) ----
OS_BASE       = 0x80000000
SVGA_SEG      = 0x9A00
SVGA_OFF      = 0x0000
MODE_SEG      = 0x9A00
MODE_OFF      = 0x0200
SVGA_LIN      = OS_BASE + 0x9A000
MODE_LIN      = OS_BASE + 0x9A200

; ---- V86 int10 stub (set up by v86_create): 0x503 = int 10h; 0x505 = hlt ----
V86_EIP       = 0x503
V86_END       = 0x505
V86_SS        = 0x9000
V86_SP        = 0x9000

; ---- v86_regs offsets (order from core/v86.inc) ----
VR_edi=0
VR_esi=4
VR_ebp=8
VR_ebx=16
VR_edx=20
VR_ecx=24
VR_eax=28
VR_eip=32
VR_cs=36
VR_eflags=40
VR_esp=44
VR_ss=48
VR_es=52
VR_ds=56
VR_SIZE=68

; ---- VBE offsets ----
SI_SIG     = 0x00
SI_VER     = 0x04      ; VbeVersion (word): 0x0200=2.0, 0x0102=1.2
SI_MODEPTR = 0x0E
MI_ATTR    = 0x00
MI_PITCH   = 0x10
MI_XRES    = 0x12
MI_YRES    = 0x14
MI_BPP     = 0x19
MI_PHYS    = 0x28

; =====================================================================
proc START c uses ebx esi edi, reason:dword, cmdline:dword
; IMPORTANT: "uses ebx esi edi" is mandatory - load_pe_driver keeps the START address in esi
; AROUND the START call and writes it into SRV.entry AFTERWARDS. Without saving esi,
; garbage ends up there -> stop_all_services on shutdown does call [garbage] -> hang.
        cmp     [reason], REASON_INIT
        jne     .stop
        mov     esi, msg_start
        call    dword [SysMsgBoardStr]

        call    BuildModeTable

        mov     esi, msg_count
        call    dword [SysMsgBoardStr]
        mov     eax, [mode_count]
        call    LogHexDword

        invoke  RegService, srv_name, service_proc
        ret
.stop:
        xor     eax, eax
        ret
endp

; =====================================================================
; IOCTL handler.  stdcall(ioctl_ptr)
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

.get_info:
        ; output <- [dd vbe_version][dd pci_lfb]
        mov     ebx, [edi + IOCTL_output]
        test    ebx, ebx
        jz      .bad
        movzx   eax, word [vbe_ver]
        mov     [ebx], eax
        mov     eax, [pci_lfb]
        mov     [ebx + 4], eax
        xor     eax, eax
        ret

.get_modes:
        ; output <- [dd count][records]
        mov     ebx, [edi + IOCTL_output]
        test    ebx, ebx
        jz      .bad
        call    KmsOwnsGpu
        jne     .no_modes               ; KMS owns the GPU -> report 0 modes (hide from picker)
        mov     eax, [mode_count]
        mov     [ebx], eax
        ; copy count*REC_SIZE bytes (count*6 dwords)
        push    esi edi
        lea     ecx, [eax + eax*2]      ; count*3
        shl     ecx, 1                  ; count*6
        mov     esi, mode_table
        lea     edi, [ebx + 4]
        rep     movsd
        pop     edi esi
        xor     eax, eax
        ret

.no_modes:
        mov     dword [ebx], 0          ; count = 0 -> vidmode won't list vesa20 while KMS is active
        xor     eax, eax
        ret

.set_mode:
        call    KmsOwnsGpu
        jne     .bad                    ; KMS owns the GPU -> refuse (VBE modeset would hang)
        mov     esi, [edi + IOCTL_input]
        test    esi, esi
        jz      .bad
        mov     eax, [esi]              ; index
        cmp     eax, [mode_count]
        jae     .bad
        call    DoSetMode               ; eax = 0 ok / -1 fail
        ret
.bad:
        or      eax, -1
        ret
endp

; =====================================================================
; KMS present?  After the call, `jne` means a DRM/KMS driver owns the display
; (display_t.ddev != 0) -> vesa20 must NOT VBE-modeset it (would fight KMS and
; hang, like VESA vs KMS in Linux). Clobbers eax; leaves the cmp flags for the caller.
; ddev offset: x/y/w/h/bpp/vrefresh/current_lfb/lfb_pitch (32) + win_map_lock
; RWSEM(12) + win_map/pitch/size(12) + modes(4) = +60.
KmsOwnsGpu:
        call    dword [GetDisplay]      ; eax = _display
        cmp     dword [eax + 60], 0     ; display_t.ddev
        ret

; =====================================================================
; Set the mode by index eax. eax=0 ok / -1 fail
DoSetMode:
        mov     ebx, eax
        shl     eax, 4                  ; index*16
        lea     eax, [eax + ebx*8]      ; + index*8 = index*24
        lea     esi, [mode_table + eax] ; esi -> record
        ; cache the record fields (so we no longer depend on esi after v86/logs)
        mov     eax, [esi + 0]
        mov     [set_w], eax
        mov     eax, [esi + 4]
        mov     [set_h], eax
        mov     eax, [esi + 8]
        mov     [set_bpp], eax
        movzx   eax, word [esi + 12]
        mov     [found_mode], ax
        mov     eax, [esi + 16]
        mov     [set_phys], eax
        mov     eax, [esi + 20]
        mov     [set_pitch], eax

        call    SaveIVT                 ; save the IVT around the v86 calls

        call    VbeSetModeByNumber
        test    eax, eax
        jz      .fail

        ; compute pitch ourselves: width * (bpp/8). 4F06 is only a sanity-check:
        ; take it ONLY if it is larger than the computed value (legal padding).
        ; If 4F06 is smaller/zero (BIOS bug, like 2048 for 640x480x32) - ignore it.
        mov     eax, [set_bpp]
        shr     eax, 3                  ; bytes/pixel: 16->2, 24->3, 32->4
        imul    eax, [set_w]
        mov     [set_pitch], eax        ; computed pitch

        call    VbeGetPitch             ; eax = pitch from 4F06 (or 0)
        cmp     eax, [set_pitch]
        jbe     .nopitch                ; 4F06 <= computed -> take the computed one
        mov     [set_pitch], eax        ; 4F06 larger -> padding, take it
.nopitch:
        call    RestoreIVT              ; restore the IVT before handing control to the kernel

        push    dword [set_phys]        ; phys
        push    dword [set_pitch]       ; pitch
        push    dword [set_bpp]         ; bpp
        push    dword [set_h]           ; height
        push    dword [set_w]           ; width
        call    dword [SetLfbMode]

        xor     eax, eax
        ret
.fail:
        call    RestoreIVT              ; restore the IVT on the error path too
        or      eax, -1
        ret

; =====================================================================
; Build the mode table via VBE
proc BuildModeTable
        mov     dword [mode_count], 0
        mov     dword [pci_lfb], 0
        call    SaveIVT                 ; save the IVT around the v86 calls
        call    VbeGetInfo
        test    eax, eax
        jz      .done

        ; determine the VBE version
        mov     ax, word [SVGA_LIN + SI_VER]
        mov     [vbe_ver], ax
        cmp     ax, 0x0200
        jae     .vbe2_ok
        ; VBE 1.x: the generic path (LFB from PCI BAR) does NOT work on real cards -
        ; chip-specific linear-enable is needed (verified on S3 Vision968 and Cirrus:
        ; BAR is correct, but banked 4F02 does not enable the linear aperture -> garbage/hang).
        ; Safe choice: do not expose any modes. Such cards use native modules (LAYER 3):
        ; clgd54xx (Cirrus), s3vid (S3).
        mov     esi, msg_native
        call    dword [SysMsgBoardStr]
        jmp     .done                   ; mode_count stays 0
.vbe2_ok:
        mov     eax, [SVGA_LIN + SI_MODEPTR]
        movzx   ebx, ax
        shr     eax, 16
        shl     eax, 4
        add     eax, ebx
        add     eax, OS_BASE
        mov     esi, eax                ; esi -> list of words
.next:
        movzx   ecx, word [esi]
        cmp     cx, 0xFFFF
        je      .done
        add     esi, 2
        mov     [cur_mode], cx

        mov     eax, [mode_count]
        cmp     eax, MAX_MODES
        jae     .done

        push    esi
        call    VbeGetModeInfo          ; cx = mode
        pop     esi
        test    eax, eax
        jz      .next

        ; mode acceptance (VBE 2.0): need the LFB attribute, phys from PhysBasePtr
        test    byte [MODE_LIN + MI_ATTR], 0x80     ; LFB available?
        jz      .next
        mov     edx, [MODE_LIN + MI_PHYS]           ; phys from PhysBasePtr
        movzx   eax, byte [MODE_LIN + MI_BPP]       ; only 16/24/32 (no 15bpp/555)
        cmp     eax, 16
        jb      .next

        ; store into mode_table + count*REC_SIZE
        mov     eax, [mode_count]
        mov     ebx, eax
        shl     eax, 4
        lea     eax, [eax + ebx*8]      ; count*24
        lea     edi, [mode_table + eax]
        movzx   eax, word [MODE_LIN + MI_XRES]
        mov     [edi + 0], eax
        movzx   eax, word [MODE_LIN + MI_YRES]
        mov     [edi + 4], eax
        movzx   eax, byte [MODE_LIN + MI_BPP]
        mov     [edi + 8], eax
        movzx   eax, word [cur_mode]
        mov     [edi + 12], eax
        mov     [edi + 16], edx         ; phys: PhysBasePtr (layer 1) or PCI BAR (layer 2)
        movzx   eax, word [MODE_LIN + MI_PITCH]
        mov     [edi + 20], eax
        inc     dword [mode_count]
        jmp     .next
.done:
        call    RestoreIVT              ; restore the IVT (video-BIOS int15h hook removed)
        ret
endp

; =====================================================================
; ---- low-level VBE calls via V86 ----
ZeroRegsIn:
        push    eax ecx edi
        mov     edi, v86_in
        xor     eax, eax
        mov     ecx, VR_SIZE/4
        rep     stosd
        pop     edi ecx eax
        ret

V86Int10:
        push    ebx ecx edx esi
        mov     dword [v86_in + VR_eip], V86_EIP
        mov     dword [v86_in + VR_eflags], 0x20200
        mov     dword [v86_in + VR_esp], V86_SP
        mov     dword [v86_in + VR_ss], V86_SS
        mov     ebx, v86_in
        mov     esi, [V86machine]
        mov     esi, [esi]
        mov     ecx, V86_END
        mov     edx, -1
        push    fs
        call    dword [V86start]
        pop     fs
        pop     esi edx ecx ebx
        ret

VbeOk:                          ; ZF=1 if AX==0x004F
        mov     ax, word [v86_out + VR_eax]
        cmp     ax, 0x004F
        ret

VbeGetInfo:
        mov     dword [SVGA_LIN + SI_SIG], 'VBE2'
        call    ZeroRegsIn
        mov     word [v86_in + VR_eax], 0x4F00
        mov     word [v86_in + VR_es], SVGA_SEG
        mov     word [v86_in + VR_edi], SVGA_OFF
        call    V86Int10
        call    VbeOk
        jne     .fail
        cmp     dword [SVGA_LIN + SI_SIG], 'VESA'
        jne     .fail
        mov     eax, 1
        ret
.fail:
        xor     eax, eax
        ret

VbeGetModeInfo:                 ; cx = mode
        push    ecx
        call    ZeroRegsIn
        pop     ecx
        mov     word [v86_in + VR_eax], 0x4F01
        mov     word [v86_in + VR_ecx], cx
        mov     word [v86_in + VR_es], MODE_SEG
        mov     word [v86_in + VR_edi], MODE_OFF
        call    V86Int10
        call    VbeOk
        jne     .fail
        mov     eax, 1
        ret
.fail:
        xor     eax, eax
        ret

VbeSetModeByNumber:             ; [found_mode] = mode number
        call    ZeroRegsIn
        mov     word [v86_in + VR_eax], 0x4F02
        mov     ax, [found_mode]
        or      ax, 0xC000              ; LFB + no-clear
        mov     word [v86_in + VR_ebx], ax
        call    V86Int10
        call    VbeOk
        jne     .fail
        mov     eax, 1
        ret
.fail:
        xor     eax, eax
        ret

VbeGetPitch:                    ; eax = bytes/line or 0
        call    ZeroRegsIn
        mov     word [v86_in + VR_eax], 0x4F06
        mov     byte [v86_in + VR_ebx], 0x01
        call    V86Int10
        call    VbeOk
        jne     .fail
        movzx   eax, word [v86_out + VR_ebx]
        ret
.fail:
        xor     eax, eax
        ret

; #####################################################################
; ##                                                                 ##
; ##  LAYER 2 (DISABLED, NOT CALLED): generic VBE 1.x via PCI BAR    ##
; ##  Verified: does not work on S3/Cirrus (needs linear-enable).    ##
; ##  Kept for reference. VBE 1.x -> native modules (LAYER 3).       ##
; ##                                                                 ##
; ##  VBE 1.2 does not return PhysBasePtr and cannot do LFB in 4F02. ##
; ##  But on PCI cards all video memory is visible linearly via the  ##
; ##  memory BAR. We set the mode with banked 4F02 (no LFB bit) and  ##
; ##  draw linearly through that BAR (display start = 0, frame sits  ##
; ##  at BAR+0).                                                      ##
; ##  Limitation: some chips need a register-level linear-enable     ##
; ##  (that is already LAYER 3 / native module). Not applicable to   ##
; ##  ISA/VLB.                                                        ##
; ##                                                                 ##
; #####################################################################

; Looks for the framebuffer linear aperture on PCI. Result -> [pci_lfb] (0 if none).
; Takes the first display controller (base class 0x03) and its memory BAR:
; prefers prefetchable, otherwise the first memory BAR.
FindPciFramebuffer:
        push    ebx ecx edx esi edi
        xor     esi, esi                ; esi = bus
.scan_bus:
        xor     edi, edi                ; edi = devfn (dev<<3|func)
.scan_dev:
        push    0x08                    ; reg 0x08: rev + class code
        push    edi
        push    esi
        call    dword [PciRead32]
        shr     eax, 24                 ; eax = base class
        cmp     al, 0x03                ; display controller?
        jne     .next_dev

        ; found a video controller - scan BAR0..BAR5 (0x10..0x24)
        mov     ebx, 0x10
.scan_bar:
        push    ebx
        push    edi
        push    esi
        call    dword [PciRead32]       ; eax = BARn
        test    al, 1                   ; I/O BAR? (bit0=1) - skip
        jnz     .next_bar
        mov     ecx, eax
        and     ecx, 0xFFFFFFF0         ; memory BAR base
        jz      .next_bar               ; not implemented
        test    al, 0x08                ; prefetchable? (bit3) - this is the framebuffer
        jz      .bar_fallback
        mov     [pci_lfb], ecx          ; found prefetchable - best candidate
        jmp     .done
.bar_fallback:
        cmp     dword [pci_lfb], 0      ; remember the first non-pref memory BAR
        jne     .next_bar
        mov     [pci_lfb], ecx
.next_bar:
        add     ebx, 4
        cmp     ebx, 0x28               ; past BAR5
        jb      .scan_bar
        jmp     .done                   ; take the first video controller found
.next_dev:
        inc     edi
        cmp     edi, 256
        jb      .scan_dev
        inc     esi
        cmp     esi, 8
        jb      .scan_bus
.done:
        pop     edi esi edx ecx ebx
        ret

; #####################################################################
; ##  end of LAYER 2                                                 ##
; #####################################################################

; =====================================================================
; Save/restore the real-mode IVT around v86 BIOS calls.
; On int10 the video BIOS hooks int15h in the SHARED (phys page 0) IVT -
; this breaks APM shutdown (real-mode int15h goes into the video hook). We save
; the IVT (256 vectors = 1KB at phys 0 = OS_BASE) before and restore it after.
SaveIVT:
        push    esi edi ecx
        cld
        mov     esi, OS_BASE
        mov     edi, ivt_save
        mov     ecx, 256
        rep     movsd
        pop     ecx edi esi
        ret

RestoreIVT:
        push    esi edi ecx
        cld
        mov     esi, ivt_save
        mov     edi, OS_BASE
        mov     ecx, 256
        rep     movsd
        pop     ecx edi esi
        ret

; =====================================================================
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

; =====================================================================
align 4
srv_name      db 'vesa20', 0
msg_start     db 'vesa20: service init', 13, 10, 0
msg_count     db 'vesa20: mode count = 0x', 0
msg_native    db 'vesa20: VESA 1.x - native driver should be used', 13, 10, 0

align 4
mode_count    dd ?
vbe2          db ?          ; 1 = VBE 2.0+ (LAYER 1), 0 = VBE 1.x (LAYER 2)
vbe_ver       dw ?          ; raw VbeVersion value (0x0200, 0x0102, ...)
pci_lfb       dd ?          ; LAYER 2: phys address of the framebuffer aperture from the PCI BAR
found_mode    dw ?
cur_mode      dw ?
set_w         dd ?
set_h         dd ?
set_bpp       dd ?
set_phys      dd ?
set_pitch     dd ?
hexbuf        rb 12
ivt_save      rb 1024
v86_in        rb VR_SIZE
v86_out       rb VR_SIZE
mode_table    rb MAX_MODES * REC_SIZE

include '../peimport.inc'
data fixups
end data
