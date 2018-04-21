;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2018. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

struct  PCI_header

        vendor_id       dw ?    ; 0x00
        device_id       dw ?    ; 0x02
        command         dw ?    ; 0x04
        status          dw ?    ; 0x06
        revision_id     db ?    ; 0x08
        prog_if         db ?    ; 0x09
        subclass        db ?    ; 0x0A
        class_code      db ?    ; 0x0B
        cache_line_size db ?    ; 0x0C
        latency_timer   db ?    ; 0x0D
        header_type     db ?    ; 0x0E
        bist            db ?    ; 0x0F

ends

struct  PCI_header00    PCI_header

        base_addr_0     dd ?    ; 0x10
        base_addr_1     dd ?    ; 0x14
        base_addr_2     dd ?    ; 0x18
        base_addr_3     dd ?    ; 0x1C
        base_addr_4     dd ?    ; 0x20
        base_addr_5     dd ?    ; 0x24
        cardbus_cis_ptr dd ?    ; 0x28
        subsys_vendor   dw ?    ; 0x2C
        subsys_id       dw ?    ; 0x2E
        exp_rom_addr    dd ?    ; 0x30
        cap_ptr         db ?    ; 0x34
                        rb 7    ; reserved
        interrupt_line  db ?    ; 0x3C
        interrupt_pin   db ?    ; 0x3D
        min_grant       db ?    ; 0x3E
        max_latency     db ?    ; 0x3F

ends

struct  PCI_header01    PCI_header

        base_addr_0     dd ?    ; 0x10
        base_addr_1     dd ?    ; 0x14
        prim_bus_nr     db ?    ; 0x18
        sec_bus_nr      db ?    ; 0x19
        sub_bus_nr      db ?    ; 0x1A
        sec_lat_tmr     db ?    ; 0x1B
        io_base         db ?    ; 0x1C
        io_limit        db ?    ; 0x1D
        sec_status      dw ?    ; 0x1E
        mem_base        dw ?    ; 0x20
        mem_limit       dw ?    ; 0x22
        pref_mem_base   dw ?    ; 0x24
        pref_mem_limit  dw ?    ; 0x26
        pref_base_up    dd ?    ; 0x28
        pref_limit_up   dd ?    ; 0x2C
        io_base_up      dw ?    ; 0x30
        io_limit_up     dw ?    ; 0x32
        cap_ptr         db ?    ; 0x34
                        rb 3    ; reserved
        exp_rom_addr    dd ?    ; 0x38
        interrupt_line  db ?    ; 0x3C
        interrupt_pin   db ?    ; 0x3E
        bridge_ctrl     dw ?    ; 0x3F

ends

struct  PCI_header02    PCI_header

        base_addr       dd ?    ; 0x10
        cap_list_offs   db ?    ; 0x14
                        rb 1    ; reserved
        sec_stat        dw ?    ; 0x16
        pci_bus_nr      db ?    ; 0x18
        cardbus_bus_nr  db ?    ; 0x19
        sub_bus_nr      db ?    ; 0x1A
        cardbus_lat_tmr db ?    ; 0x1B
        mbar_0          dd ?    ; 0x1C
        mlimit_0        dd ?    ; 0x20
        mbar_1          dd ?    ; 0x24
        mlimit_1        dd ?    ; 0x28
        iobar_0         dd ?    ; 0x2C
        iolimit_0       dd ?    ; 0x30
        iobar_1         dd ?    ; 0x34
        iolimit_1       dd ?    ; 0x38
        interrupt_line  db ?    ; 0x3C
        interrupt_pin   db ?    ; 0x3D
        bridge_ctrl     dw ?    ; 0x3E
        subs_did        dw ?    ; 0x40
        subs_vid        dw ?    ; 0x42
        legacy_bar      dd ?    ; 0x44

ends

; Base address bits
        PCI_BASE_ADDRESS_SPACE_IO               = 0x01
        PCI_BASE_ADDRESS_IO_MASK                = 0xFFFFFFFC
        PCI_BASE_ADDRESS_MEM_MASK               = 0xFFFFFFF0
        PCI_BASE_ADDRESS_MEM_TYPE_MASK          = 0x00000006
        PCI_BASE_ADDRESS_MEM_TYPE_32            = 0x0
        PCI_BASE_ADDRESS_MEM_TYPE_RESERVED      = 0x02
        PCI_BASE_ADDRESS_MEM_TYPE_64            = 0x4


; command bits
        PCI_CMD_PIO                     = 0x01          ; bit0: io space control
        PCI_CMD_MMIO                    = 0x02          ; bit1: memory space control
        PCI_CMD_MASTER                  = 0x04          ; bit2: device acts as a PCI master
        PCI_CMD_INTX_DISABLE            = 0x400         ; INTx emulation disable

; status bits
        PCI_STATUS_CAPA                 = 0x10          ; bit4: new capabilities available


if used PCI_find_io
proc PCI_find_io stdcall bus, dev

        push    esi
        xor     eax, eax
        mov     esi, PCI_header00.base_addr_0
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
        cmp     esi, PCI_header00.base_addr_5
        jbe     .check
        pop     esi
        xor     eax, eax
        ret

endp
end if


if used PCI_find_mmio
proc PCI_find_mmio stdcall bus, dev

        push    esi ebx
        mov     esi, PCI_header00.base_addr_0
  .check:
        invoke  PciRead32, [bus], [dev], esi
        DEBUGF  1, "BAR: 0x%x\n", eax
        mov     ebx, eax
        test    eax, PCI_BASE_ADDRESS_SPACE_IO  ; MMIO address?
        jnz     .next
        and     ebx, PCI_BASE_ADDRESS_MEM_TYPE_MASK
        cmp     bl, PCI_BASE_ADDRESS_MEM_TYPE_64
        je      .is64
        cmp     bl, PCI_BASE_ADDRESS_MEM_TYPE_32
        jne     .next
        ; Ok, we have a 32-bit BAR.
        and     eax, PCI_BASE_ADDRESS_MEM_MASK
        pop     ebx esi
        DEBUGF  1, "32-bit MMIO address found: 0x%x\n", eax
        ret

  .is64:
        ; Ok, we have a 64-bit BAR, check if the upper 32-bits are 0, then we can use it..
        push    eax
        add     esi, 4
        cmp     esi, PCI_header00.base_addr_5
        ja      .fail
        invoke  PciRead32, [bus], [dev], esi
        test    eax, eax
        pop     eax
        jnz     .next
        and     eax, PCI_BASE_ADDRESS_MEM_MASK
        pop     ebx esi
        DEBUGF  1, "64-bit MMIO address found: 0x00000000%x\n", eax
        ret

  .next:
        add     esi, 4
        cmp     esi, PCI_header00.base_addr_5
        jbe     .check
  .fail:
        xor     eax, eax
        pop     ebx esi
        DEBUGF  1, "No usable MMIO addresses found!\n"
        ret

endp
end if