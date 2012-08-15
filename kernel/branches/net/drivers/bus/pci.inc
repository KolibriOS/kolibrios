;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; PCI Bus defines

        PCI_HEADER_TYPE                 = 0x0e          ; 8 bit
        PCI_BASE_ADDRESS_0              = 0x10          ; 32 bit
        PCI_BASE_ADDRESS_5              = 0x24          ; 32 bits
        PCI_BASE_ADDRESS_SPACE_IO       = 0x01
        PCI_VENDOR_ID                   = 0x00          ; 16 bit
        PCI_BASE_ADDRESS_IO_MASK        = 0xFFFFFFFC

; PCI programming

        PCI_REG_COMMAND                 = 0x4           ; command register
        PCI_REG_STATUS                  = 0x6           ; status register
        PCI_REG_LATENCY                 = 0xd           ; latency timer register
        PCI_REG_CAP_PTR                 = 0x34          ; capabilities pointer
        PCI_REG_CAPABILITY_ID           = 0x0           ; capapility ID in pm register block
        PCI_REG_PM_STATUS               = 0x4           ; power management status register
        PCI_REG_PM_CTRL                 = 0x4           ; power management control register
        PCI_BIT_PIO                     = 1             ; bit0: io space control
        PCI_BIT_MMIO                    = 2             ; bit1: memory space control
        PCI_BIT_MASTER                  = 4             ; bit2: device acts as a PCI master


macro   find_io bus, dev, io {

        local   .check, .inc, .got

        xor     eax, eax
        mov     esi, PCI_BASE_ADDRESS_0
        movzx   ecx, bus
        movzx   edx, dev
  .check:
        stdcall PciRead32, ecx ,edx ,esi

        test    eax, PCI_BASE_ADDRESS_IO_MASK
        jz      .inc

        test    eax, PCI_BASE_ADDRESS_SPACE_IO
        jz      .inc

        and     eax, PCI_BASE_ADDRESS_IO_MASK
        jmp     .got

  .inc:
        add     esi, 4
        cmp     esi, PCI_BASE_ADDRESS_5
        jbe     .check
        xor     eax, eax

  .got:
        mov     io, eax

}


macro   find_mmio32 bus, dev, io {

        local   .check, .inc, .got

        mov     esi, PCI_BASE_ADDRESS_0
  .check:
        stdcall PciRead32, bus, dev, esi

        test    eax, PCI_BASE_ADDRESS_SPACE_IO  ; mmio address?
        jnz     .inc

        test    eax, 100b       ; 64 bit?
        jnz     .inc
        and     eax, not 1111b
        jmp     .got

  .inc:
        add     esi, 4
        cmp     esi, PCI_BASE_ADDRESS_5
        jbe     .check
        xor     eax, eax

  .got:
        mov     io, eax

}

macro   find_irq bus, dev, irq {

        push    eax edx ecx
        movzx   ecx, bus
        movzx   edx, dev
        stdcall PciRead8, ecx ,edx ,0x3c                                ; 0x3c is the offset where irq can be found
        mov     irq, al
        pop     ecx edx eax

}

macro   find_rev bus, dev, rev {

        push    eax edx ecx
        movzx   ecx, bus
        movzx   edx, dev
        stdcall PciRead8, ecx ,edx ,0x8
        mov     rev, al
        pop     ecx edx eax

}

macro   make_bus_master bus, dev {

        movzx   ecx, bus
        movzx   edx, dev
        stdcall PciRead32, ecx ,edx, PCI_REG_COMMAND
        or      al, PCI_BIT_MASTER
        stdcall PciWrite32, ecx, edx, PCI_REG_COMMAND, eax

}

macro   adjust_latency bus, dev, min {

        movzx   ecx, bus
        movzx   edx, dev
        stdcall PciRead8, ecx ,edx, PCI_REG_LATENCY
        cmp     al, min
        ja      @f
        mov     al, min
        stdcall PciWrite8, ecx, edx, PCI_REG_LATENCY, eax
  @@:

}
