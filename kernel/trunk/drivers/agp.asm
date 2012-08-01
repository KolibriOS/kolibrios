;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


format MS COFF

DEBUG           equ 1
API_VERSION     equ 1

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

SRV_GETVERSION  equ 0
SRV_DETECT      equ 1

section '.flat' code readable align 16

proc START stdcall, state:dword

        cmp     [state], 1
        jne     .exit
.entry:

     if DEBUG
        mov     esi, msgInit
        call    SysMsgBoardStr
     end if

        stdcall RegService, my_service, service_proc
        ret
.fail:
.exit:
        xor     eax, eax
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

        mov     ebx, [ioctl]
        mov     eax, [ebx+io_code]
        cmp     eax, SRV_GETVERSION
        jne     @F

        mov     eax, [ebx+output]
        cmp     [ebx+out_size], 4
        jne     .fail
        mov     [eax], dword API_VERSION
        xor     eax, eax
        ret
@@:
        mov     ebx, [ioctl]
        mov     eax, [ebx+io_code]
        cmp     eax, SRV_DETECT
        jne     @F
        call    detect
@@:
.fail:
        or      eax, -1
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

        xor     eax, eax
        mov     [bus], eax
        inc     eax
        call    PciApi          ; get last bus
        cmp     eax, -1
        je      .error

        mov     [last_bus], eax

  .next_bus:
        and     [devfn], 0
  .next_dev:
        stdcall PciRead16, [bus], [devfn], dword 0x08   ; read class/subclass
        test    eax, eax
        jz      .next
        cmp     eax, -1
        je      .next

        cmp     ax, 0x0302      ; display controller - 3d controller
        je      .found

        cmp     ax, 0x0380      ; display controller - other display controller
        je      .found

  .next:
        inc     [devfn]
        cmp     [devfn], 256
        jb      .next_dev
        mov     eax, [bus]
        inc     eax
        mov     [bus], eax
        cmp     eax, [last_bus]
        jna     .next_bus
        xor     eax, eax
        ret

  .found:
        stdcall PciRead8, [bus], [devfn], dword 0x06    ; read prog IF
        test    al, 1 shl 4                             ; got capabilities list?
        jnz     .got_capabilities_list

  .got_capabilities_list:
        stdcall PciRead8, [bus], [devfn], dword 0x34    ; read capabilities offset
        and     eax, 11111100b                          ; always dword aligned
        mov     esi, eax

  .read_capability:
        stdcall PciRead32, [bus], [devfn], esi          ; read capability
        cmp     al, 0x02                                ; AGP
        je      .got_agp
        movzx   esi, ah                                 ; pointer to next capability
        test    esi, esi
        jnz     .read_capability
  .error:
        xor     eax, eax
        inc     eax
        ret

  .got_agp:
        shl     eax, 16
        mov     [revision], al                          ; high nibble = major revision
                                                        ; low nibble = minor revision
        add     esi, 4
        and     al, 0xf0
        cmp     al, 0x30
        je      .agp_3

  .agp_2:
        stdcall PciRead32, [bus], [devfn], esi          ; read AGP status
        test    al, 100b
        jnz     .100b

        test    al, 10b
        jnz     .010b

        test    al, 1b
        jz      .error

  .001b:
        mov     [speed], 001b
        jmp     .agp_go

  .010b:
        mov     [speed], 010b
        jmp     .agp_go

  .100b:
        mov     [speed], 100b
        jmp     .agp_go

  .agp_3:
        stdcall PciRead32, [bus], [devfn], esi          ; read AGP status
        test    al, 1 shl 3
        jz      .agp_2
        and     al, 11b
        mov     [speed], al
        cmp     al, 11b
        jne     .agp_go
        mov     [speed], 10b

  .agp_go:
        add     esi, 4
        stdcall PciRead32, [bus], [devfn], esi          ; read AGP cmd
        and     al, not 111b                            ; set max speed
        or      al, [speed]
        or      eax, 1 shl 8                            ; enable AGP
        stdcall PciWrite32, [bus], [devfn], esi, eax    ; write AGP cmd

        ret

endp


;all initialized data place here

align 4
version         dd (5 shl 16) or (API_VERSION and 0xFFFF)

my_service      db 'AGP', 0                             ; max 16 chars include zero

msgInit         db 'detect hardware...', 13, 10, 0
msgPCI          db 'PCI acces not supported', 13, 10, 0
msgFail         db 'device not found', 13, 10, 0

section '.data' data readable writable align 16

;all uninitialized data place here

revision        db ?
speed           db ?
bus             dd ?
devfn           dd ?

