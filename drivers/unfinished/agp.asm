;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved.    ;;
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


format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

        FAST_WRITE              = 0     ; may cause problems with some motherboards

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../pci.inc'

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START c, reason:dword, cmdline:dword

        cmp     [reason], DRV_ENTRY
        jne     .fail

        mov     esi, msgInit
        invoke  SysMsgBoardStr
        invoke  RegService, my_service, service_proc

        call    detect

        ret

  .fail:
        xor     eax, eax
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc SERVICE_PROC      ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc service_proc stdcall, ioctl:dword

        mov     edx, [ioctl]
        mov     eax, [edx + IOCTL.io_code]

;------------------------------------------------------

        cmp     eax, 0 ;SRV_GETVERSION
        jne     .fail

        cmp     [edx + IOCTL.out_size], 4
        jb      .fail
        mov     eax, [edx + IOCTL.output]
        mov     [eax], dword API_VERSION

        xor     eax, eax
        ret

  .fail:
        or      eax, -1
        ret

endp

align 4
proc detect

        mov     esi, msgSearch
        invoke  SysMsgBoardStr

        invoke  GetPCIList
        mov     edx, eax

  .loop:
        mov     ebx, [eax + PCIDEV.class]
        cmp     bx, 0x0300      ; display controller - vga compatible controller
        je      .found
        cmp     bx, 0x0302      ; display controller - 3d controller
        je      .found
        cmp     bx, 0x0380      ; display controller - other display controller
        je      .found

  .next:
        mov     eax, [eax + PCIDEV.fd]
        cmp     eax, edx
        jne     .loop

        mov     esi, msgDone
        invoke  SysMsgBoardStr

        or      eax, -1
        ret

  .found:
        push    eax edx
        movzx   ebx, [eax + PCIDEV.bus]
        mov     [bus], ebx
        movzx   ebx, [eax + PCIDEV.devfn]
        mov     [devfn], ebx
        invoke  PciRead8, [bus], [devfn], PCI_header00.prog_if
        test    al, 1 shl 4                             ; got capabilities list?
        jnz     .got_capabilities_list

        ; TODO: Do it the old way: detect device and check with a list of known capabilities
        ; stupid pre PCI 2.2 board....

        pop     edx eax
        jmp     .next

  .got_capabilities_list:
        invoke  PciRead8, [bus], [devfn], PCI_header00.cap_ptr
        and     eax, 11111100b                          ; always dword aligned
        mov     edi, eax

  .read_capability:
        invoke  PciRead32, [bus], [devfn], edi          ; read capability
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
        invoke  SysMsgBoardStr

        invoke  PciRead32, [bus], [devfn], edi          ; read AGP status
  .agp_2_:
        test    al, 100b
        jnz     .100b
        test    al, 10b
        jnz     .010b
        test    al, 1b

        pop     edx eax
        jz      .next

  .001b:
        mov     [cmd], 001b
        mov     esi, msg1
        invoke  SysMsgBoardStr
        jmp     .agp_go

  .010b:
        mov     [cmd], 010b
        mov     esi, msg2
        invoke  SysMsgBoardStr
        jmp     .agp_go

  .100b:
        mov     [cmd], 100b
        mov     esi, msg4
        invoke  SysMsgBoardStr
        jmp     .agp_go

  .agp_2m:
        mov     esi, msgAGP2m
        invoke  SysMsgBoardStr
        jmp     .agp_2_

  .agp_3:
        mov     esi, msgAGP3
        invoke  SysMsgBoardStr

        invoke  PciRead32, [bus], [devfn], edi          ; read AGP status
        test    al, 1 shl 3
        jz      .agp_2m

        test    eax, 10b
        jnz     .8x
        mov     [cmd], 01b
        mov     esi, msg4
        invoke  SysMsgBoardStr
        jmp     .agp_go

  .8x:
        mov     [cmd], 10b
        mov     esi, msg8
        invoke  SysMsgBoardStr

  .agp_go:

if FAST_WRITE
        test    ax, 1 shl 4
        jz      @f
        or      [cmd], 1 shl 4
        mov     esi, msgfast
        invoke  SysMsgBoardStr
  @@:
end if

        test    ax, 1 shl 9     ; Side band addressing
        jz      @f
        or      [cmd], 1 shl 9
        mov     esi, msgside
        invoke  SysMsgBoardStr
  @@:

        add     edi, 4
        mov     eax, [cmd]
        or      eax, 1 shl 8                            ; enable AGP
        invoke  PciWrite32, [bus], [devfn], edi, eax    ; write AGP cmd

        mov     esi, msgOK
        invoke  SysMsgBoardStr

        pop     edx eax
        jmp     .next

endp


; End of code

data fixups
end data

include '../peimport.inc'

my_service      db 'AGP', 0                             ; max 16 chars include zero

msgInit         db 'AGP driver loaded.', 13, 10, 0
msgSearch       db 'Searching for AGP card...', 13, 10, 0
msgDone         db 'Done', 13, 10, 0
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

; uninitialized data

revision        db ?
cmd             dd ?
bus             dd ?
devfn           dd ?

