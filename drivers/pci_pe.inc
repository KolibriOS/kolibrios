;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

struct  PCI_header

        vendor_id       dw ?
        device_id       dw ?
        command         dw ?
        status          dw ?
        revision_id     db ?
        prog_if         db ?
        subclass        db ?
        class_code      db ?
        cache_line_size db ?
        latency_timer   db ?
        header_type     db ?
        bist            db ?

ends

struct  PCI_header00    PCI_header

        base_addr_0     dd ?
        base_addr_1     dd ?
        base_addr_2     dd ?
        base_addr_3     dd ?
        base_addr_4     dd ?
        base_addr_5     dd ?
        cardbus_cis_ptr dd ?
        subsys_vendor   dw ?
        subsys_id       dw ?
        exp_rom_addr    dd ?
        cap_ptr         db ?
        reserved        rb 7
        interrupt_line  db ?
        interrupt_pin   db ?
        min_grant       db ?
        max_latency     db ?

ends

; Base address bits
        PCI_BASE_ADDRESS_SPACE_IO       = 0x01
        PCI_BASE_ADDRESS_IO_MASK        = 0xFFFFFFFC
        PCI_BASE_ADDRESS_MEM_MASK       = 0xFFFFFFF0

; command bits
        PCI_CMD_PIO                     = 1             ; bit0: io space control
        PCI_CMD_MMIO                    = 2             ; bit1: memory space control
        PCI_CMD_MASTER                  = 4             ; bit2: device acts as a PCI master


if used PCI_find_io
proc PCI_find_io stdcall bus, dev

        push    esi
        xor     eax, eax
        mov     esi, PCI_BASE_ADDRESS_0
  .check:
        invoke  PciRead32, [bus], [dev], esi

        test    eax, PCI_BASE_ADDRESS_IO_MASK
        jz      .inc

        test    eax, PCI_BASE_ADDRESS_SPACE_IO
        jz      .inc

        and     eax, PCI_BASE_ADDRESS_IO_MASK
        pop     esi
        ret

  .inc:
        add     esi, 4
        cmp     esi, PCI_BASE_ADDRESS_5
        jbe     .check
        pop     esi
        xor     eax, eax
        ret

endp
end if


if used PCI_find_mmio32
proc PCI_find_mmio32 stdcall bus, dev

        push    esi
        mov     esi, PCI_header00.base_addr_0
  .check:
        invoke  PciRead32, [bus], [dev], esi

        test    eax, PCI_BASE_ADDRESS_SPACE_IO  ; mmio address?
        jnz     .inc

        test    eax, 100b       ; 64 bit?
        jnz     .inc
        and     eax, not 1111b
        pop     esi
        ret

  .inc:
        add     esi, 4
        cmp     esi, PCI_header00.base_addr_5
        jbe     .check
        xor     eax, eax
        pop     esi
        ret

endp
end if