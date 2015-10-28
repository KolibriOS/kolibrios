;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
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

; TODO:
;
; Use GetPCIList instead of reading directly from PCI bus to detect bridge.
; (See #5544 agp.asm for example).
; Export a function in kernel to re-scan PCI device list (fix 'dirty hack').
; Fix bugs (currently not working on all PCMCIA bridges).

format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

        CARDBUS_IO              = 0xFC00

        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 1

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../pci.inc'
include '../fdo.inc'

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START c, reason:dword, cmdline:dword

        cmp     [reason], DRV_ENTRY
        jne     .fail

        DEBUGF  1, "Loading cardbus driver\n"
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
        invoke  PciApi
        cmp     eax, -1
        je      .err
        mov     [last_bus], eax

        inc     eax
        mov     [card_bus], eax

  .next_bus:
        and     [devfn], 0
  .next_dev:
        invoke  PciRead32, [bus], [devfn], PCI_header.vendor_id
        test    eax, eax
        jz      .next
        cmp     eax, -1
        je      .next

        invoke  PciRead16, [bus], [devfn], PCI_header.subclass  ; class & subclass
        cmp     ax, 0x0607
        je      .found

  .next:
        test    [devfn], 7
        jnz     .next_fn
        invoke  PciRead8, [bus], [devfn], PCI_header.header_type
        test    al, al
        js      .next_fn
        or      [devfn], 7

  .next_fn:
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

        invoke  PciRead8, [bus], [devfn], PCI_header.header_type
        and     al, not 0x80                                            ; Mask the multifunction device bit
        DEBUGF  1, "Header type=0x%x\n", eax:2
        cmp     al, 2
        jne     .next

; Write PCI and cardbus numbers

        invoke  PciRead32, [bus], [devfn], PCI_header02.pci_bus_nr      ; PCcard latency settings + Card bus number, PCI bus number
        and     eax, 0xff000000                                         ; Keep original latency setting, clear the rest
        mov     al, byte[bus]
        mov     ah, byte[card_bus]
        mov     ebx, [card_bus]
        shl     ebx, 16
        or      eax, ebx
        DEBUGF  1, "Latency, bus,.. 0x%x\n", eax
        invoke  PciWrite32, [bus], [devfn], PCI_header02.pci_bus_nr, eax

; set ExCA legacy mode base

        invoke  PciWrite32, [bus], [devfn], 0x44, 1

; Enable power

        invoke  PciRead8, [bus], [devfn], 0x14                  ; get capabilities offset
        movzx   eax, al                                         ; (A0 for TI bridges)
        DEBUGF  1, "Capabilities offset=0x%x\n", eax:2
        add     al, 4                                           ; Power management control/status
        invoke  PciWrite16, [bus], [devfn], eax, 0x0100         ; Enable PME signaling, power state=D0

; Enable Bus master, io space, memory space

        invoke  PciWrite16, [bus], [devfn], PCI_header02.command, 0x0007

; Write CardBus Socket/ExCA base address

        mov     eax, 0x7f000000
        push    eax
        invoke  PciWrite32, [bus], [devfn], PCI_header02.base_addr, eax ; base is 4 Kbyte aligned
        pop     ebx
        invoke  MapIoMem, ebx, 4096, 0x1b
        mov     ecx, eax

; Check if a card is present in the socket

        mov     eax, [ecx + 8]                                  ; Socket present state register
        DEBUGF  1, "Socket present state reg: 0x%x\n", eax
        and     al, 10110110b                                   ; NotACard | CBCard | 16bitCard | CDetect1 | CDetect2
        cmp     al, 00100000b                                   ; Check for inserted cardbus card
        je      .CardbusInserted

; No card found... set PCI command back to 0

        invoke  PciWrite16, [bus], [devfn], PCI_header02.command, 0  ; To avoid conflicts with other sockets
        DEBUGF  1, "Cardbus KO\n"
        jmp     .next

  .CardbusInserted:
        DEBUGF  1, "Card inserted\n"
        ;mov     word[ecx + 0x802], 0x00F9       ; Assert reset, output enable, vcc=vpp=3.3V
        mov     dword[ecx + 0x10], 0x33         ; Request 3.3V for Vcc and Vpp (Control register)
        ;push    ecx
        ;mov     esi, 10
        ;invoke  Sleep
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
        invoke  PciWrite32, [bus], [devfn], reg, regvalue
        DEBUGF  1, "Writing 0x%x to 0x%x\n", regvalue, reg
        reg = reg + 4
}

        invoke  PciWrite8, [bus], [devfn], PCI_header02.interrupt_line, 0xc    ; IRQ line

        invoke  PciRead16, [bus], [devfn], PCI_header02.bridge_ctrl                 ; Bridge control
        or      ax, 0x0700                                      ; Enable write posting, both memory windows prefetchable
        invoke  PciWrite16, [bus], [devfn], PCI_header02.bridge_ctrl, eax
        DEBUGF  1, "Write posting enabled\n"


        DEBUGF  1, "Bridge PCI registers:\n"
rept    17 reg
{
        invoke  PciRead32, [bus], [devfn], 4*(reg-1)
        DEBUGF  1, "0x%x\n", eax
}

        inc     byte[0x80009021]                                ; LAST PCI bus count in kernel (dirty HACK!)


        mov     ecx, 100
  .waitactive:
        push    ecx
        invoke  PciRead32, [card_bus], 0, PCI_header02.vendor_id         ; Check if the card is awake yet
        inc     eax
        jnz     .got_it
        mov     esi, 2
        invoke  Sleep
        pop     ecx
        dec     ecx
        jnz     .waitactive

        DEBUGF  1, "Timeout!\n"
        ; TODO: disable card/bridge again ?
        jmp     .next

  .got_it:
        pop     eax
        DEBUGF  1, "Card is enabled!\n"

        invoke  PciWrite32, [card_bus], 0, PCI_header02.base_addr, CARDBUS_IO       ; Supposing it's IO space that is needed
        invoke  PciWrite8, [card_bus], 0, PCI_header02.interrupt_line, 0xC                      ; FIXME
        invoke  PciWrite16, [card_bus], 0, PCI_header02.command, PCI_CMD_PIO or PCI_CMD_MMIO

        DEBUGF  1, "done\n"

        jmp     .next

  .err:
        DEBUGF  1, "Error\n"
        xor     eax, eax

        ret

endp


; End of code

data fixups
end data

include '../peimport.inc'

my_service      db 'CARDBUS',0                  ; max 16 chars include zero

include_debug_strings                           ; All data wich FDO uses will be included here