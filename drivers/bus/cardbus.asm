;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  PCMCIA aka cardbus driver for KolibriOS                     ;;
;;  Written by hidnplayr@gmail.com                              ;;
;;                                                              ;;
;;  Many credits go to Paolo Franchetti for his HWTEST program  ;;
;;  (https://sites.google.com/site/pfranz73/) from which large  ;;
;;  parts of code have been borrowed.                           ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; This module detects and initialises all Cardbus/pc-card/PCMCIA cards.

; WARNING: Cards must be inserted before the driver starts, and shouldn't be removed.
; This module doesn't handle insertions and removals.

format MS COFF

        API_VERSION             = 0x01000100
        DRIVER_VERSION          = 5

        CARDBUS_IO              = 0xFC00

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 1


include '../struct.inc'
include '../macros.inc'
include '../proc32.inc'
include '../imports.inc'
include '../pci.inc'
include '../fdo.inc'

public START
public service_proc
public version

section '.flat' code readable align 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc START stdcall, state:dword

        cmp [state], 1
        jne .exit

  .entry:

        DEBUGF  1, "Loading cardbus driver\n"
        stdcall RegService, my_service, service_proc

        call    detect

        ret

  .fail:
  .exit:
        xor     eax, eax
        ret

endp



;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc SERVICE_PROC      ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
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

           locals
             last_bus dd ?
             card_bus dd ?
             bus      dd ?
             devfn    dd ?
           endl

        DEBUGF  1, "Searching for cardbus bridges...\n"

        xor     eax, eax
        mov     [bus], eax
        inc     eax
        call    PciApi
        cmp     eax, -1
        je      .err
        mov     [last_bus], eax

        inc     eax
        mov     [card_bus], eax

  .next_bus:
        and     [devfn], 0
  .next_dev:
        stdcall PciRead32, [bus], [devfn], PCI_VENDOR_ID
        test    eax, eax
        jz      .next
        cmp     eax, -1
        je      .next

        stdcall PciRead16, [bus], [devfn], 0x0a ; class & subclass
        cmp     ax, 0x0607
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

        DEBUGF  1, "Search complete\n"
        xor     eax, eax
        inc     eax
        ret

  .found:
        DEBUGF  1, "Found cardbus bridge: bus=0x%x, dev=0x%x\n", [bus], [devfn]

        stdcall PciRead8, [bus], [devfn], 0x0e                  ; get header type
        DEBUGF  1, "Header type=0x%x\n", eax:2

        test    al, al
        jz      .next

; Write PCI and cardbus numbers

        stdcall PciRead32, [bus], [devfn], 0x18                 ; PCcard latency settings + Card bus number, PCI bus number
        and     eax, 0xff000000                                 ; Keep original latency setting, clear the rest
        mov     al, byte[bus]
        mov     ah, byte[card_bus]
        mov     ebx, [card_bus]
        shl     ebx, 16
        or      eax, ebx
        DEBUGF  1, "Latency, bus,.. 0x%x\n", eax
        stdcall PciWrite32, [bus], [devfn], 0x18, eax

; set ExCA legacy mode base

        stdcall PciWrite32, [bus], [devfn], 0x44, 1

; Enable power

        stdcall PciRead8, [bus], [devfn], 0x14                  ; get capabilities offset
        movzx   eax, al                                         ; (A0 for TI bridges)
        DEBUGF  1, "Capabilities offset=0x%x\n", eax:2
        add     al, 4                                           ; Power management control/status
        stdcall PciWrite16, [bus], [devfn], eax, 0x0100         ; Enable PME signaling, power state=D0

; Enable Bus master, io space, memory space

        stdcall PciWrite16, [bus], [devfn], PCI_REG_COMMAND, 0x0007

; Write CardBus Socket/ExCA base address

        mov     eax, 0x7f000000
        push    eax
        stdcall PciWrite32, [bus], [devfn], 0x10, eax           ; base is 4 Kbyte aligned
        pop     ebx
        stdcall MapIoMem, ebx, 4096, 0x1b
        mov     ecx, eax

; Check if a card is present in the socket

        mov     eax, [ecx + 8]                                  ; Socket present state register
        DEBUGF  1, "Socket present state reg: 0x%x\n", eax
        and     al, 10110110b                                   ; NotACard | CBCard | 16bitCard | CDetect1 | CDetect2
        cmp     al, 00100000b                                   ; Check for inserted cardbus card
        je      .CardbusInserted

; No card found... set PCI command back to 0

        stdcall PciWrite16, [bus], [devfn], PCI_REG_COMMAND, 0  ; To avoid conflicts with other sockets
        DEBUGF  1, "Cardbus KO\n"
        jmp     .next

  .CardbusInserted:
        DEBUGF  1, "Card inserted\n"
        ;mov     word[ecx + 0x802], 0x00F9       ; Assert reset, output enable, vcc=vpp=3.3V
        mov     dword[ecx + 0x10], 0x33         ; Request 3.3V for Vcc and Vpp (Control register)
        ;push    ecx
        ;mov     esi, 10
        ;call    Sleep
        ;pop     ecx
        ;mov     byte[ecx + 0x803], 0x40         ; stop reset
        mov     dword[ecx + 0xC], 0x4000        ; force Card CV test (Force register)   ;;; WHY???
        DEBUGF  1, "Resetting card\n"

; Next power up test can be deferred until before writing to Bridge control PCI reg 0x3E
  .waitpower:                                   ; For TI, you can check that bits 8-11 in PCI reg 80h are all 0
        test    dword[ecx + 8], 1 shl 3         ; Test PWRCYCLE bit
        jz      .waitpower                      ; Wait for power to go up

        DEBUGF  1, "Interface is powered up\n"

; Write MemBase-Limit 0 and 1, then IOBase-Limit 0 and 1
; mem0 space limit = base => size is 4 kilobytes
; set to 0 the second interval (mem1 and IO1)
; IO0: size is 256 bytes

irp     regvalue,   0x7efff000, 0x7effffff, 0x7effe000, 0x7effe000, CARDBUS_IO, CARDBUS_IO + 0xFF, 0, 0
{
common
        reg = 0x1C
forward
        stdcall PciWrite32, [bus], [devfn], reg, regvalue
        DEBUGF  1, "Writing 0x%x to 0x%x\n", regvalue, reg
        reg = reg + 4
}

        stdcall PciWrite8, [bus], [devfn], 0x3c, 0xc    ; IRQ line

        stdcall PciRead16, [bus], [devfn], 0x3e                 ; Bridge control
        or      ax, 0x0700                                      ; Enable write posting, both memory windows prefetchable
        stdcall PciWrite16, [bus], [devfn], 0x3e, eax
        DEBUGF  1, "Write posting enabled\n"


        DEBUGF  1, "Bridge PCI registers:\n"
rept    17 reg
{
        stdcall PciRead32, [bus], [devfn], 4*(reg-1)
        DEBUGF  1, "0x%x\n", eax
}

        inc     byte[0x80009021]                                ; LAST PCI bus count in kernel (dirty HACK!)


        mov     ecx, 100
  .waitactive:
        push    ecx
        stdcall PciRead32, [card_bus], 0, PCI_VENDOR_ID         ; Check if the card is awake yet
        inc     eax
        jnz     .got_it
        mov     esi, 2
        call    Sleep
        pop     ecx
        dec     ecx
        jnz     .waitactive

        DEBUGF  1, "Timeout!\n"
        ; TODO: disable card/bridge again ?
        jmp     .next

  .got_it:
        pop     eax
        DEBUGF  1, "Card is enabled!\n"

        stdcall PciWrite32, [card_bus], 0, PCI_BASE_ADDRESS_0, CARDBUS_IO       ; Supposing it's IO space that is needed
        stdcall PciWrite8, [card_bus], 0, PCI_REG_IRQ, 0xC                      ; FIXME
        stdcall PciWrite16, [card_bus], 0, PCI_REG_COMMAND, PCI_BIT_PIO or PCI_BIT_MMIO

        DEBUGF  1, "done\n"

        jmp     .next

  .err:
        DEBUGF  1, "Error\n"
        xor     eax, eax

        ret



endp




; End of code

section '.data' data readable writable align 16

version         dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service      db 'CARDBUS',0                  ; max 16 chars include zero

include_debug_strings                           ; All data wich FDO uses will be included here