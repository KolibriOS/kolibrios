;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;driver sceletone

format MS COFF

DEBUG        equ 1

API_VERSION     equ 0  ;debug

include 'proc32.inc'
include 'imports.inc'

struc IOCTL
{  .handle      dd ?
   .io_code     dd ?
   .input       dd ?
   .inp_size    dd ?
   .output      dd ?
   .out_size    dd ?
}

virtual at 0
  IOCTL IOCTL
end virtual

public START
public service_proc
public version

DRV_ENTRY    equ 1
DRV_EXIT     equ -1
STRIDE       equ 4      ;size of row in devices table

SRV_GETVERSION  equ 0

section '.flat' code readable align 16

proc START stdcall, state:dword

           cmp [state], 1
           jne .exit
.entry:

     if DEBUG
           mov esi, msgInit
           call SysMsgBoardStr
     end if

           stdcall RegService, my_service, service_proc
           ret
.fail:
.exit:
           xor eax, eax
           ret
endp

handle     equ  IOCTL.handle
io_code    equ  IOCTL.io_code
input      equ  IOCTL.input
inp_size   equ  IOCTL.inp_size
output     equ  IOCTL.output
out_size   equ  IOCTL.out_size

align 4
proc service_proc stdcall, ioctl:dword

           mov ebx, [ioctl]
           mov eax, [ebx+io_code]
           cmp eax, SRV_GETVERSION
           jne @F

           mov eax, [ebx+output]
           cmp [ebx+out_size], 4
           jne .fail
           mov [eax], dword API_VERSION
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

align 4
proc detect
           locals
            last_bus dd ?
           endl

           xor eax, eax
           mov [bus], eax
           inc eax
           call PciApi
           cmp eax, -1
           je .err

           mov [last_bus], eax

.next_bus:
           and [devfn], 0
.next_dev:
           stdcall PciRead32, [bus], [devfn], dword 0
           test eax, eax
           jz .next
           cmp eax, -1
           je .next

           mov edi, devices
@@:
           mov ebx, [edi]
           test ebx, ebx
           jz .next

           cmp eax, ebx
           je .found

           add edi, STRIDE
           jmp @B
.next:
           inc [devfn]
           cmp [devfn], 256
           jb  .next_dev
           mov eax, [bus]
           inc eax
           mov [bus], eax
           cmp eax, [last_bus]
           jna .next_bus
           xor eax, eax
           ret
.found:
           xor eax, eax
           inc eax
           ret
.err:
           xor eax, eax
           ret
endp

DEVICE_ID    equ  1234;  pci device id
VENDOR_ID    equ  5678;  device vendor id


;all initialized data place here

align 4
devices      dd (DEVICE_ID shl 16)+VENDOR_ID
             dd 0    ;terminator

version      dd (5 shl 16) or (API_VERSION and 0xFFFF)

my_service   db 'MY_SERVICE',0  ;max 16 chars include zero

msgInit      db 'detect hardware...',13,10,0
msgPCI       db 'PCI accsess not supported',13,10,0
msgFail      db 'device not found',13,10,0

section '.data' data readable writable align 16

;all uninitialized data place here

