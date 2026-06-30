; === kms.asm — KMS/DRM -> vidmode bridge service for KolibriOS ===
;
; Purpose: make an already-loaded DRM/KMS display driver (i915 / radeon /
; vmwgfx, all of which register the kernel service "DISPLAY") appear in the
; vidmode mode-picker as just another driver.
;
; The bridge speaks TWO protocols:
;   * Outward (to the vidmode GUI) it is a normal vidmode service:
;       VBE_GET_MODES (1): output = [dword count][count * 24-byte records]
;       VBE_SET_MODE  (2): input  = [dword index]
;       VBE_GET_INFO  (3): output = [word version][dword lfb]
;   * Inward (to the DRM driver) it calls the "DISPLAY" service handler
;     directly via SRV.srv_proc, using the DRM ABI:
;       SRV_ENUM_MODES (1): input = videomode_t[], output = &count (2-pass)
;       SRV_SET_MODE   (2): input = videomode_t{w,h,bpp,freq}
;     where videomode_t = { int width, height, bpp, freq }  (16 bytes).
;
; It does NOT touch hardware or SetLfbMode itself — the DRM driver owns the
; framebuffer/modeset. The bridge only translates and forwards.
;
; If no "DISPLAY" service is registered (no KMS driver loaded) the bridge
; refuses to register, so the picker simply will not list it.

format PE DLL native 0.05
entry START

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'

; --- driver entry reason ---
REASON_INIT    = 1

; --- vidmode-facing ioctl_t field offsets (the picker's ABI) ---
IOCTL_io_code  = 4
IOCTL_input    = 8
IOCTL_output   = 16

; --- vidmode IO codes ---
VBE_GET_MODES  = 1
VBE_SET_MODE   = 2
VBE_GET_INFO   = 3

REC_SIZE       = 24            ; vidmode mode record size
MAX_MODES      = 48            ; fits vidmode buffer (4 + 48*24)

; --- DRM "DISPLAY" service IO codes ---
SRV_GETVERSION = 0
SRV_ENUM_MODES = 1
SRV_SET_MODE   = 2

; --- kernel SRV descriptor: user-mode handler pointer ---
SRV_PROC       = 0x28

VM_SIZE        = 16            ; sizeof(videomode_t)

; =====================================================================
; START(reason, cmdline) — locate DISPLAY, register ourselves
proc START c uses ebx esi edi, reason:dword, cmdline:dword
        cmp     [reason], REASON_INIT
        jne     .stop
        mov     esi, msg_start
        call    dword [SysMsgBoardStr]

        ; Find the DRM display service and cache its handler.
        invoke  GetService, sz_display
        test    eax, eax
        jz      .nodisp
        mov     ecx, [eax + SRV_PROC]
        test    ecx, ecx
        jz      .nodisp
        mov     [disp_proc], ecx

        mov     esi, msg_found
        call    dword [SysMsgBoardStr]

        invoke  RegService, srv_name, service_proc
        ret
.nodisp:
        mov     esi, msg_nodisp
        call    dword [SysMsgBoardStr]
.stop:
        xor     eax, eax
        ret
endp

; =====================================================================
; service_proc(ioctl) — vidmode IOCTL dispatcher
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

; ---- VBE_GET_MODES: enumerate via DISPLAY, repack to 24-byte records ----
.get_modes:
        mov     ebx, [edi + IOCTL_output]   ; caller output buffer
        test    ebx, ebx
        jz      .bad

        ; pass 1: ask DISPLAY for the total mode count (count==0 => total)
        mov     dword [d_count], 0
        mov     dword [di_iocode], SRV_ENUM_MODES
        mov     dword [di_input], 0
        mov     dword [di_inpsize], 0
        mov     dword [di_output], d_count
        mov     dword [di_outsize], 4
        call    call_display

        mov     eax, [d_count]
        cmp     eax, MAX_MODES
        jbe     @f
        mov     eax, MAX_MODES
@@:
        mov     [d_count], eax              ; desired count (clamped)
        test    eax, eax
        jz      .gm_zero

        ; pass 2: fill the videomode_t array
        mov     ecx, eax
        shl     ecx, 4                      ; count*16 bytes
        mov     dword [di_iocode], SRV_ENUM_MODES
        mov     dword [di_input], vm_buf
        mov     [di_inpsize], ecx
        mov     dword [di_output], d_count
        mov     dword [di_outsize], 4
        call    call_display

        mov     eax, [d_count]              ; actual filled count
        cmp     eax, MAX_MODES              ; paranoia clamp
        jbe     @f
        mov     eax, MAX_MODES
@@:
        mov     [mode_count], eax           ; cache for SET_MODE by index

        ; repack videomode_t{w,h,bpp,freq} -> vidmode 24-byte record
        mov     esi, vm_buf
        lea     edi, [ebx + 4]
        mov     ecx, eax
        test    ecx, ecx
        jz      .gm_done
.gm_pack:
        mov     eax, [esi + 0]              ; width
        mov     [edi + 0], eax
        mov     eax, [esi + 4]              ; height
        mov     [edi + 4], eax
        mov     eax, [esi + 8]              ; bpp (32 from DRM)
        mov     [edi + 8], eax
        mov     word [edi + 12], 0          ; vbe_mode = 0 (n/a for KMS)
        mov     eax, [esi + 12]             ; refresh rate (Hz) from videomode_t
        mov     [edi + 14], ax              ; -> record reserved word = freq
        mov     dword [edi + 16], 0         ; phys=0  (KMS owns fb; also the freq marker)
        mov     dword [edi + 20], 0         ; pitch=0 (KMS owns the framebuffer)
        add     esi, VM_SIZE
        add     edi, REC_SIZE
        dec     ecx
        jnz     .gm_pack
.gm_done:
        mov     eax, [mode_count]
        mov     [ebx], eax                  ; [output+0] = count
        xor     eax, eax
        ret
.gm_zero:
        mov     dword [ebx], 0
        mov     dword [mode_count], 0
        xor     eax, eax
        ret

; ---- VBE_SET_MODE: index -> cached videomode_t -> DISPLAY SRV_SET_MODE ----
.set_mode:
        mov     esi, [edi + IOCTL_input]
        test    esi, esi
        jz      .bad
        mov     eax, [esi]                  ; mode index
        cmp     eax, [mode_count]
        jae     .bad                        ; also covers empty cache (count==0)
        shl     eax, 4                      ; index*16
        add     eax, vm_buf                 ; &vm_buf[index]
        mov     [di_input], eax
        mov     dword [di_iocode], SRV_SET_MODE
        mov     dword [di_inpsize], VM_SIZE
        mov     dword [di_output], 0
        mov     dword [di_outsize], 0
        call    call_display                ; eax = DISPLAY retval (0 = ok)
        ret

; ---- VBE_GET_INFO: report "native" (version 0), no VBE LFB ----
.get_info:
        mov     ebx, [edi + IOCTL_output]
        test    ebx, ebx
        jz      .bad
        mov     dword [ebx], 0              ; version 0 -> picker shows "native"
        mov     dword [ebx + 4], 0          ; lfb (unused by the picker)
        xor     eax, eax
        ret

.bad:
        or      eax, -1
        ret
endp

; =====================================================================
; Invoke the cached DISPLAY service handler with d_ioctl. stdcall: the
; callee cleans the single argument. Returns the handler's eax.
call_display:
        push    d_ioctl
        call    dword [disp_proc]
        ret

; =====================================================================
align 4
disp_proc      dd 0            ; DISPLAY service srv_proc (set at init)
mode_count     dd 0            ; cached number of modes (for SET_MODE index)
d_count        dd 0            ; in/out count for SRV_ENUM_MODES

; ioctl_t passed to the DISPLAY handler (handle, io_code, input, inp_size,
; output, out_size) — same layout as the kernel ioctl_t.
d_ioctl:
di_handle      dd 0
di_iocode      dd 0
di_input       dd 0
di_inpsize     dd 0
di_output      dd 0
di_outsize     dd 0

vm_buf         rb MAX_MODES * VM_SIZE      ; cached videomode_t[] from DISPLAY

srv_name       db 'kms', 0                 ; name the picker probes
sz_display     db 'DISPLAY', 0             ; the DRM service we bridge to

msg_start      db 'kms: bridge init', 13, 10, 0
msg_found      db 'kms: DISPLAY service found, registering', 13, 10, 0
msg_nodisp     db 'kms: no DISPLAY (KMS) service present, skipping', 13, 10, 0

include '../peimport.inc'
data fixups
end data
