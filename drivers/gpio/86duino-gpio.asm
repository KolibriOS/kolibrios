;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;driver sceletone

format PE DLL native 0.05
entry START

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 1             ; 1 = verbose, 2 = errors only


        API_VERSION             = 0  ;debug

        STRIDE                  = 4      ;size of row in devices table

        SRV_GETVERSION          = 0

section '.flat' code readable writable executable

include 'proc32.inc'
include 'struct.inc'
include 'macros.inc'
include 'peimport.inc'
include 'fdo.inc'

proc START c, state:dword, cmdline:dword

        cmp     [state], 1
        jne     .exit
.entry:

        push    esi
        DEBUGF  1,"Loading vortex86EX GPIO driver\n"
        call    detect
        pop     esi
        test    eax, eax
        jz      .fail

; Set crossbar base address register in southbridge
        invoke  PciWrite16, [bus], [dev], 64h, 0x0A00 or 1

; Set GPIO base address register in southbridge
        invoke  PciWrite16, [bus], [dev], 62h, 0xF100 or 1

; Enable GPIO0-9
        mov     dx, 0xf100
        mov     eax, 0x000001ff
        out     dx, eax

        mov     ecx, 10
        mov     dx, 0xf104
        mov     ax, 0xf200
  .gpio_init:
; Set GPIO data port base address
        out     dx, ax
        add     ax, 2
        add     dx, 2
; Set GPIO direction base address
        out     dx, ax
        add     ax, 2
        add     dx, 2
; loop
        dec     ecx
        jnz     .gpio_init

; Set GPIO0 pin 0 as output
        mov     al, 0x01
        mov     dx, 0xf202
        out     dx, al

; Set GPIO4 pin 0 as output
        mov     al, 0x01
        mov     dx, 0xf212
        out     dx, al

        invoke  RegService, my_service, service_proc
        ret
.fail:
.exit:
        xor     eax, eax
        ret
endp

proc service_proc stdcall, ioctl:dword

        mov     ebx, [ioctl]
        mov     eax, [ebx+IOCTL.io_code]
        cmp     eax, SRV_GETVERSION
        jne     @F

        mov     eax, [ebx+IOCTL.output]
        cmp     [ebx+IOCTL.out_size], 4
        jne     .fail
        mov     dword [eax], API_VERSION
        xor     eax, eax
        ret
@@:
        cmp     eax, 1  ; read GPIO P0
        jne     @f
        mov     dx, 0xf200
        in      al, dx
        ret
@@:
        cmp     eax, 2  ; write GPIO P0
        jne     @f

        mov     eax, [ebx + IOCTL.input]
        mov     dx, 0xf200
        out     dx, al
        xor     eax, eax
        ret
@@:
.fail:
        or      eax, -1
        ret
endp


proc detect
        push    ebx
        invoke  GetPCIList
        mov     ebx, eax
.next_dev:
        mov     eax, [eax+PCIDEV.fd]
        cmp     eax, ebx
        jz      .err
        mov     edx, [eax+PCIDEV.vendor_device_id]

        mov     esi, devices
@@:
        cmp     dword [esi], 0
        jz      .next_dev
        cmp     edx, [esi]
        jz      .found

        add     esi, STRIDE
        jmp     @B

.found:
        movzx   ebx, [eax+PCIDEV.devfn]
        mov     [dev], ebx
        movzx   ebx, [eax+PCIDEV.bus]
        mov     [bus], ebx
        xor     eax, eax
        inc     eax
        pop     ebx
        ret
.err:
        DEBUGF  1,"Could not find vortex86EX south bridge!\n"
        xor     eax, eax
        pop     ebx
        ret
endp

DEVICE_ID    = 6011h
VENDOR_ID    = 17F3h

;all initialized data place here

align 4
devices      dd (DEVICE_ID shl 16)+VENDOR_ID
             dd 0    ;terminator

my_service   db '86DUINO-GPIO',0  ;max 16 chars include zero

include_debug_strings                           ; All data wich FDO uses will be included here

dev     dd ?
bus     dd ?

align 4
data fixups
end data
