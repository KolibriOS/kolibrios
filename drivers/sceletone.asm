;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;driver sceletone

format PE DLL native 0.05
entry START

DEBUG        equ 1

API_VERSION     equ 0  ;debug

STRIDE       equ 4      ;size of row in devices table

SRV_GETVERSION  equ 0

section '.flat' code readable writable executable
include 'proc32.inc'
include 'struct.inc'
include 'macros.inc'
include 'peimport.inc'

proc START c, state:dword, cmdline:dword

        cmp     [state], 1
        jne     .exit
.entry:

        push    esi
     if DEBUG
        mov     esi, msgInit
        invoke  SysMsgBoardStr
     end if
        call    detect
        pop     esi
        test    eax, eax
        jz      .fail

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
        xor     eax, eax
        inc     eax
        pop     ebx
        ret
.err:
        mov     esi, msgFail
        invoke  SysMsgBoardStr
        xor     eax, eax
        pop     ebx
        ret
endp

DEVICE_ID    equ  1234;  pci device id
VENDOR_ID    equ  5678;  device vendor id


;all initialized data place here

align 4
devices      dd (DEVICE_ID shl 16)+VENDOR_ID
             dd 0    ;terminator

my_service   db 'MY_SERVICE',0  ;max 16 chars include zero

msgInit      db 'detect hardware...',13,10,0
msgFail      db 'device not found',13,10,0

align 4
data fixups
end data
