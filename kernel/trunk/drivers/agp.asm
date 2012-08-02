;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;; simple AGP driver for KolibriOS                                 ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


format MS COFF

DEBUG                   equ 1
FAST_WRITE              equ 0           ; may cause problems with some motherboards

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

DRV_ENTRY       equ 1
DRV_EXIT        equ -1

SRV_GETVERSION  equ 0
SRV_DETECT      equ 1

API_VERSION     equ 1

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

        mov     esi, msgSearch
        call    SysMsgBoardStr

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
        stdcall PciRead16, [bus], [devfn], dword 0x0a   ; read class/subclass

        cmp     ax, 0x0300      ; display controller - vga compatable controller
        je      .found

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

  .error:
        mov     esi, msgFail
        call    SysMsgBoardStr

        xor     eax, eax
        inc     eax
        ret

  .found:
        stdcall PciRead8, [bus], [devfn], dword 0x06    ; read prog IF
        test    al, 1 shl 4                             ; got capabilities list?
        jnz     .got_capabilities_list

        ; TODO: Do it the old way: detect device and check with a list of known capabilities
        ; stupid pre PCI 2.2 board....

        jmp     .next

  .got_capabilities_list:
        stdcall PciRead8, [bus], [devfn], dword 0x34    ; read capabilities offset
        and     eax, 11111100b                          ; always dword aligned
        mov     edi, eax

  .read_capability:
        stdcall PciRead32, [bus], [devfn], edi          ; read capability
        cmp     al, 0x02                                ; AGP
        je      .got_agp
        movzx   edi, ah                                 ; pointer to next capability
        test    edi, edi
        jnz     .read_capability
        jmp     .next

  .got_agp:
        shr     eax, 16
        mov     [revision], al                          ; high nibble = major revision
                                                        ; low nibble = minor revision
        add     edi, 4
        and     al, 0xf0
        cmp     al, 0x30
        je      .agp_3

  .agp_2:
        mov     esi, msgAGP2
        call    SysMsgBoardStr

        stdcall PciRead32, [bus], [devfn], edi          ; read AGP status
  .agp_2_:
        test    al, 100b
        jnz     .100b

        test    al, 10b
        jnz     .010b

        test    al, 1b
        jz      .error

  .001b:
        mov     [cmd], 001b
        mov     esi, msg1
        call    SysMsgBoardStr
        jmp     .agp_go

  .010b:
        mov     [cmd], 010b
        mov     esi, msg2
        call    SysMsgBoardStr
        jmp     .agp_go

  .100b:
        mov     [cmd], 100b
        mov     esi, msg4
        call    SysMsgBoardStr
        jmp     .agp_go

  .agp_2m:
        mov     esi, msgAGP2m
        call    SysMsgBoardStr
        jmp     .agp_2_

  .agp_3:
        mov     esi, msgAGP3
        call    SysMsgBoardStr

        stdcall PciRead32, [bus], [devfn], edi          ; read AGP status
        test    al, 1 shl 3
        jz      .agp_2m

        test    eax, 10b
        jnz     .8x
        mov     [cmd], 01b
        mov     esi, msg4
        call    SysMsgBoardStr
        jmp     .agp_go

  .8x:
        mov     [cmd], 10b
        mov     esi, msg8
        call    SysMsgBoardStr

  .agp_go:

if FAST_WRITE
        test    ax, 1 shl 4
        jz      @f
        or      [cmd], 1 shl 4
        mov     esi, msgfast
        call    SysMsgBoardStr
  @@:
end if

        test    ax, 1 shl 9     ; Side band addressing
        jz      @f
        or      [cmd], 1 shl 9
        mov     esi, msgside
        call    SysMsgBoardStr
  @@:

        add     edi, 4
        mov     eax, [cmd]
        or      eax, 1 shl 8                            ; enable AGP
        stdcall PciWrite32, [bus], [devfn], edi, eax    ; write AGP cmd

        mov     esi, msgOK
        call    SysMsgBoardStr

        ret

endp


; initialized data

align 4
version         dd (5 shl 16) or (API_VERSION and 0xFFFF)

my_service      db 'AGP', 0                             ; max 16 chars include zero

msgInit         db 'AGP driver loaded.', 13, 10, 0
msgSearch       db 'Searching for AGP card...', 13, 10, 0
msgFail         db 'device not found', 13, 10, 0
msgOK           db 'AGP device enabled', 13, 10, 0
msgAGP2         db 'AGP2 device found', 13, 10, 0
msgAGP3         db 'AGP3 device found', 13, 10, 0
msgAGP2m        db 'Running in AGP2 mode', 13, 10, 0
msg8            db '8x speed', 13, 10, 0
msg4            db '4x speed', 13, 10, 0
msg2            db '2x speed', 13, 10, 0
msg1            db '1x speed', 13, 10, 0
msgfast         db 'Fast Write', 13, 10, 0
msgside         db 'Side band addressing', 13, 10, 0

section '.data' data readable writable align 16

; uninitialized data

revision        db ?
cmd             dd ?
bus             dd ?
devfn           dd ?

