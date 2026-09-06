;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  mar_yuk.asm  -  Marvell Yukon 88E8001/8003 (and related) driver
;;               for KolibriOS
;;
;;  Based on OpenBSD if_sk.c / if_skreg.h (preferred structure)
;;  and the KolibriOS netdrv / i8254x pattern.
;;
;;  Copyright (C) KolibriOS team 2026. All rights reserved.
;;  Distributed under terms of the GNU General Public License v2.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

        MAX_DEVICES             = 8

        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 1             ; 1 = verbose while developing

        MAX_PKT_SIZE            = 1514

        ; Start with modest rings; can grow later
        RX_RING_SIZE            = 64
        TX_RING_SIZE            = 64

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'
include 'mar_yuk.inc'

;----------------------------------------------------------------------
;  Local helpers – MMIO access (BAR0 is a linear 16 KB region on Yukon)
;  Classic GEnesis needs RAP windowing; Yukon maps the full space.
;----------------------------------------------------------------------

; ebx = device,  eax = offset  ->  eax = value
align 4
proc sk_read32
        mov     edx, [ebx + SK_DEVICE.mmio_addr]
        mov     eax, [edx + eax]
        ret
endp

align 4
proc sk_write32
        ; ebx = device, eax = offset, ecx = value
        mov     edx, [ebx + SK_DEVICE.mmio_addr]
        mov     [edx + eax], ecx
        ret
endp

align 4
proc sk_read16
        mov     edx, [ebx + SK_DEVICE.mmio_addr]
        movzx   eax, word [edx + eax]
        ret
endp

align 4
proc sk_write16
        mov     edx, [ebx + SK_DEVICE.mmio_addr]
        mov     [edx + eax], cx
        ret
endp

align 4
proc sk_read8
        mov     edx, [ebx + SK_DEVICE.mmio_addr]
        movzx   eax, byte [edx + eax]
        ret
endp

align 4
proc sk_write8
        mov     edx, [ebx + SK_DEVICE.mmio_addr]
        mov     [edx + eax], cl
        ret
endp

;----------------------------------------------------------------------
;  START – standard driver entry
;----------------------------------------------------------------------

proc START c, reason:dword, cmdline:dword

        cmp     [reason], DRV_ENTRY
        jne     .fail

        DEBUGF  1, "Loading Marvell Yukon / SysKonnect driver\n"
        invoke  RegService, my_service, service_proc
        ret

  .fail:
        xor     eax, eax
        ret

endp

;----------------------------------------------------------------------
;  service_proc – IOCTL dispatcher
;----------------------------------------------------------------------

align 4
proc service_proc stdcall, ioctl:dword

        mov     edx, [ioctl]
        mov     eax, [edx + IOCTL.io_code]

;----- SRV_GETVERSION -------------------------------------------------
        cmp     eax, 0
        jne     .not_ver

        cmp     [edx + IOCTL.out_size], 4
        jb      .fail
        mov     eax, [edx + IOCTL.output]
        mov     dword [eax], API_VERSION
        xor     eax, eax
        ret

  .not_ver:
;----- SRV_HOOK -------------------------------------------------------
        cmp     eax, 1
        jne     .fail

        cmp     [edx + IOCTL.inp_size], 3
        jb      .fail

        mov     eax, [edx + IOCTL.input]
        cmp     byte [eax], 1                   ; PCI bus/dev
        jne     .fail

; Already registered?
        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

        mov     ax, [eax + 1]                   ; bus in al, dev in ah
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte [ebx + SK_DEVICE.pci_bus]
        jne     .next
        cmp     ah, byte [ebx + SK_DEVICE.pci_dev]
        je      .find_devicenum
  .next:
        add     esi, 4
        loop    .nextdevice

  .firstdevice:
        cmp     [devices], MAX_DEVICES
        jae     .fail

        allocate_and_clear ebx, sizeof.SK_DEVICE, .fail

; Fill standard ETH_DEVICE callbacks
        mov     [ebx + SK_DEVICE.reset], reset
        mov     [ebx + SK_DEVICE.transmit], transmit
        mov     [ebx + SK_DEVICE.unload], unload
        mov     [ebx + SK_DEVICE.name], my_service
        mov     [ebx + SK_DEVICE.type], NET_TYPE_ETH
        mov     [ebx + SK_DEVICE.mtu], 1500

; Save PCI coordinates
        mov     eax, [edx + IOCTL.input]
        movzx   ecx, byte [eax + 1]
        mov     [ebx + SK_DEVICE.pci_bus], ecx
        movzx   ecx, byte [eax + 2]
        mov     [ebx + SK_DEVICE.pci_dev], ecx

; Enable bus-master + memory space
        invoke  PciRead16, [ebx + SK_DEVICE.pci_bus], \
                           [ebx + SK_DEVICE.pci_dev], PCI_header00.command
        or      ax, PCI_CMD_MASTER or PCI_CMD_MMIO
        invoke  PciWrite16, [ebx + SK_DEVICE.pci_bus], \
                            [ebx + SK_DEVICE.pci_dev], PCI_header00.command, eax

; Locate MMIO BAR (BAR0)
        stdcall PCI_find_mmio, [ebx + SK_DEVICE.pci_bus], [ebx + SK_DEVICE.pci_dev]
        test    eax, eax
        jz      .destroy

; Map 16 KB (full GEnesis/Yukon register space)
        invoke  MapIoMem, eax, 0x4000, PG_SW + PG_NOCACHE
        test    eax, eax
        jz      .destroy
        mov     [ebx + SK_DEVICE.mmio_addr], eax

; IRQ line
        invoke  PciRead8, [ebx + SK_DEVICE.pci_bus], \
                          [ebx + SK_DEVICE.pci_dev], PCI_header00.interrupt_line
        mov     [ebx + SK_DEVICE.irq_line], al

        DEBUGF  1, "bus:%u dev:%u irq:%u mmio:%x\n", \
                [ebx + SK_DEVICE.pci_bus]:1, \
                [ebx + SK_DEVICE.pci_dev]:1, \
                [ebx + SK_DEVICE.irq_line]:1, \
                [ebx + SK_DEVICE.mmio_addr]

; Probe hardware (chip ID, MAC, …)
        call    probe
        test    eax, eax
        jnz     .err

; Add to our list
        mov     eax, [devices]
        mov     [device_list + 4*eax], ebx
        inc     [devices]

; Register with the network stack
        invoke  NetRegDev
        cmp     eax, -1
        je      .destroy

        DEBUGF  1, "device registered, number %u\n", eax

; Attach IRQ handler (PHY link events come via EXTERNAL_REG)
        movzx   eax, [ebx + SK_DEVICE.irq_line]
        DEBUGF  1, "attaching IRQ %u\n", eax
        invoke  AttachIntHandler, eax, int_handler, ebx
        test    eax, eax
        jnz     .irq_ok
        DEBUGF  2, "AttachIntHandler failed\n"
        ; continue anyway – polling not implemented yet
  .irq_ok:
        xor     eax, eax
        ret

  .find_devicenum:
        invoke  NetPtrToNum
        mov     eax, edi
        ret

  .destroy:
  .err:
        ; TODO: unmap, free rings, etc.
        invoke  KernelFree, ebx
  .fail:
        DEBUGF  2, "HOOK failed\n"
        or      eax, -1
        ret

endp

;----------------------------------------------------------------------
;  probe – detect chip, read permanent MAC address
;----------------------------------------------------------------------

align 4
probe:
        push    ebx

; Make sure the ASIC is out of reset (16-bit CSR)
        mov     eax, SK_CSR
        mov     ecx, SK_CSR_SW_UNRESET or SK_CSR_MASTER_UNRESET
        call    sk_write16
        ; small delay
        mov     ecx, 1000
@@:     loop    @b

;;----- Chip type & revision ------------------------------------------
; sk_type = SK_CHIPVER,  sk_rev = (SK_CONFIG >> 4)
; Look up the human-readable name in chip_table (same style as 3c59x).

        mov     eax, SK_CHIPVER
        call    sk_read8
        mov     [ebx + SK_DEVICE.chip_id], al
        DEBUGF  1, "CHIPVER = 0x%x\n", eax:2

        mov     eax, SK_CONFIG
        call    sk_read8
        shr     al, 4
        mov     [ebx + SK_DEVICE.chip_rev], al

; Search chip_table for matching CHIPVER value
        mov     ecx, CHIP_TABLE_COUNT - 1
  .chip_loop:
        mov     al, [chip_table + ecx*8]         ; chip_id byte
        cmp     al, [ebx + SK_DEVICE.chip_id]
        je      .chip_found
        dec     ecx
        jns     .chip_loop

        DEBUGF  2, "unsupported chip ID 0x%x\n", \
                [ebx + SK_DEVICE.chip_id]:2
        mov     eax, -1
        pop     ebx
        ret

  .chip_found:
        mov     esi, [chip_table + ecx*8 + 4]    ; name pointer
        mov     [ebx + SK_DEVICE.chip_name], esi
        mov     [ebx + SK_DEVICE.name], esi      ; also expose via ETH_DEVICE
        mov     [ebx + SK_DEVICE.rev_name], 0

; Yukon / Yukon-LP may actually be Yukon-Lite Rev A0
; (Flash-Address register test from OpenBSD / sk98lin)
        mov     al, [ebx + SK_DEVICE.chip_id]
        cmp     al, SK_YUKON
        je      .test_lite_a0
        cmp     al, SK_YUKON_LP
        jne     .rev_string

  .test_lite_a0:
        mov     eax, SK_EP_ADDR
        call    sk_read32
        push    eax                             ; save original

        mov     eax, SK_EP_ADDR + 3
        mov     cl, 0xFF
        call    sk_write8
        mov     eax, SK_EP_ADDR + 3
        call    sk_read8
        test    al, al
        jnz     .restore_ep                     ; stuck -> not A0

        mov     [ebx + SK_DEVICE.chip_id], SK_YUKON_LITE
        mov     [ebx + SK_DEVICE.chip_rev], SK_YUKON_LITE_REV_A0
        mov     esi, name_yukon_lite
        mov     [ebx + SK_DEVICE.chip_name], esi
        mov     [ebx + SK_DEVICE.name], esi
        mov     [ebx + SK_DEVICE.rev_name], name_rev_a0

  .restore_ep:
        pop     ecx
        mov     eax, SK_EP_ADDR
        call    sk_write32

  .rev_string:
; Yukon-Lite revision string
        cmp     byte [ebx + SK_DEVICE.chip_id], SK_YUKON_LITE
        jne     .print_id

        mov     al, [ebx + SK_DEVICE.chip_rev]
        cmp     al, SK_YUKON_LITE_REV_A0
        jne     @f
        mov     [ebx + SK_DEVICE.rev_name], name_rev_a0
        jmp     .print_id
@@:     cmp     al, SK_YUKON_LITE_REV_A1
        jne     @f
        mov     [ebx + SK_DEVICE.rev_name], name_rev_a1
        jmp     .print_id
@@:     cmp     al, SK_YUKON_LITE_REV_A3
        jne     .print_id
        mov     [ebx + SK_DEVICE.rev_name], name_rev_a3

  .print_id:
        mov     esi, [ebx + SK_DEVICE.chip_name]
        mov     edi, [ebx + SK_DEVICE.rev_name]
        test    edi, edi
        jz      .print_no_rev
        DEBUGF  1, "%s rev. %s (rev=0x%x)\n", \
                esi, edi, [ebx + SK_DEVICE.chip_rev]:2
        jmp     .id_done
  .print_no_rev:
        DEBUGF  1, "%s (rev=0x%x)\n", \
                esi, [ebx + SK_DEVICE.chip_rev]:2

  .id_done:
; Default to port 0, single MAC for now
        mov     [ebx + SK_DEVICE.port], 0
        mov     [ebx + SK_DEVICE.macs], 1

; PMD type → copper vs fiber
        mov     eax, SK_PMDTYPE
        call    sk_read8
        cmp     al, 'T'
        je      .copper
        cmp     al, '1'
        je      .copper
        mov     [ebx + SK_DEVICE.copper], 0
        jmp     .pmd_done
  .copper:
        mov     [ebx + SK_DEVICE.copper], 1
  .pmd_done:

;----- Read permanent MAC address ------------------------------------
; OpenBSD does:
;   for (i = 0; i < 6; i++)
;       eaddr[i] = sk_win_read_1(sc, SK_MAC0_0 + (port * 8) + i);
;
; On a linear Yukon mapping this is simply a byte read from
; SK_MAC0_0 … SK_MAC0_0+5.

        lea     edi, [ebx + SK_DEVICE.mac]      ; ETH_DEVICE.mac is 6 bytes
        xor     esi, esi                        ; offset 0..5
  .mac_loop:
        mov     eax, SK_MAC0_0
        add     eax, esi
        call    sk_read8
        mov     [edi + esi], al
        inc     esi
        cmp     esi, 6
        jb      .mac_loop

        DEBUGF  1, "MAC %x:%x:%x:%x:%x:%x\n", \
                [ebx + SK_DEVICE.mac + 0]:2, \
                [ebx + SK_DEVICE.mac + 1]:2, \
                [ebx + SK_DEVICE.mac + 2]:2, \
                [ebx + SK_DEVICE.mac + 3]:2, \
                [ebx + SK_DEVICE.mac + 4]:2, \
                [ebx + SK_DEVICE.mac + 5]:2

; Sanity-check: reject an all-zero or all-FF address
        mov     eax, dword [ebx + SK_DEVICE.mac]
        cmp     eax, 0
        je      .bad_mac
        cmp     eax, 0xFFFFFFFF
        je      .bad_mac
        movzx   eax, word [ebx + SK_DEVICE.mac + 4]
        cmp     ax, 0
        je      .check_ff
        cmp     ax, 0xFFFF
        je      .bad_mac
        jmp     .mac_ok

  .check_ff:
        ; first 4 bytes already non-zero, accept
        jmp     .mac_ok

  .bad_mac:
        DEBUGF  2, "invalid MAC address read from chip\n"
        ; Fall through – still return success so we can debug further;
        ; a real driver would try the EEPROM/VPD path here.

  .mac_ok:
; Bring the hardware out of reset so link LEDs can light
        call    reset
        xor     eax, eax                        ; success
        pop     ebx
        ret

;----------------------------------------------------------------------
;  Placeholder callbacks – filled in later
;----------------------------------------------------------------------


;----------------------------------------------------------------------
;  PHY access via Yukon SMI (YUKON_SMICR / YUKON_SMIDR)
;----------------------------------------------------------------------
; ebx = device
; eax = PHY register number
; returns eax = 16-bit value (phy_read) / writes cx (phy_write)

align 4
phy_read:
        push    ecx edx
        ; Build SMICR: PHYAD=0, REGAD=eax, OP_READ
        mov     ecx, eax
        and     ecx, 0x1F
        shl     ecx, YU_SMICR_REGAD_SHIFT
        or      ecx, YU_SMICR_OP_READ
        ; PHY address 0 already
        mov     eax, YUKON_REG_BASE + YUKON_SMICR
        call    sk_write32

        ; Wait for READ_VALID
        mov     edx, 1000
  .pr_wait:
        mov     eax, YUKON_REG_BASE + YUKON_SMICR
        call    sk_read32
        test    eax, YU_SMICR_READ_VALID
        jnz     .pr_ok
        dec     edx
        jnz     .pr_wait
        DEBUGF  2, "phy_read timeout\n"
        xor     eax, eax
        pop     edx ecx
        ret
  .pr_ok:
        mov     eax, YUKON_REG_BASE + YUKON_SMIDR
        call    sk_read32
        and     eax, 0xFFFF
        pop     edx ecx
        ret

align 4
phy_write:
        ; eax = reg, cx = value
        push    ecx edx
        mov     edx, ecx                        ; value
        and     edx, 0xFFFF

        ; Write data first
        push    eax
        mov     ecx, edx
        mov     eax, YUKON_REG_BASE + YUKON_SMIDR
        call    sk_write32
        pop     eax

        ; Build SMICR: REGAD + OP_WRITE
        mov     ecx, eax
        and     ecx, 0x1F
        shl     ecx, YU_SMICR_REGAD_SHIFT
        ; OP_WRITE = 0
        mov     eax, YUKON_REG_BASE + YUKON_SMICR
        call    sk_write32

        mov     edx, 1000
  .pw_wait:
        mov     eax, YUKON_REG_BASE + YUKON_SMICR
        call    sk_read32
        test    eax, YU_SMICR_BUSY
        jz      .pw_ok
        dec     edx
        jnz     .pw_wait
        DEBUGF  2, "phy_write timeout\n"
  .pw_ok:
        pop     edx ecx
        ret

;----------------------------------------------------------------------
;  detect_link – read Marvell PHY specific status, update ETH_DEVICE.state
;----------------------------------------------------------------------

align 4
detect_link:
        mov     eax, PHY_MARV_PHY_STAT
        call    phy_read
        DEBUGF  1, "PHY_STAT = 0x%x\n", eax:4

        test    eax, PHY_M_PS_LINK_UP
        jz      .down

        xor     ecx, ecx
        test    eax, PHY_M_PS_SPEED_1000
        jz      .not_1g
        or      ecx, ETH_LINK_SPEED_1G
        jmp     .speed_done
  .not_1g:
        test    eax, PHY_M_PS_SPEED_100
        jz      .is_10
        or      ecx, ETH_LINK_SPEED_100M
        jmp     .speed_done
  .is_10:
        or      ecx, ETH_LINK_SPEED_10M
  .speed_done:
        test    eax, PHY_M_PS_FULL_DUP
        jz      .half
        or      ecx, ETH_LINK_FULL_DUPLEX
  .half:
        mov     [ebx + SK_DEVICE.state], ecx
        mov     [ebx + SK_DEVICE.link], 1
        DEBUGF  1, "link UP  state=0x%x\n", ecx
        invoke  NetLinkChanged
        ret

  .down:
        mov     [ebx + SK_DEVICE.state], ETH_LINK_DOWN
        mov     [ebx + SK_DEVICE.link], 0
        DEBUGF  1, "link DOWN\n"
        invoke  NetLinkChanged
        ret

;----------------------------------------------------------------------
;  int_handler – Yukon IRQ
;  Linux routes PHY events via IS_EXT_REG -> yukon_phy_intr
;----------------------------------------------------------------------

align 4
int_handler:
        push    ebx esi edi

        mov     ebx, [esp + 4*4]                ; softc from AttachIntHandler

        ; Read interrupt source
        mov     eax, SK_ISR
        call    sk_read32
        test    eax, eax
        jz      .none
        DEBUGF  1, "IRQ status = 0x%x\n", eax

	push	eax

; ---- RX done? ----
        test    eax, SK_ISR_RX1_EOF
        jz      .no_rx_ISR
  .rx_loop:
        mov     esi, [ebx + SK_DEVICE.cur_rx]
        mov     edi, [ebx + SK_DEVICE.rx_ring]
        push    eax
        imul    eax, esi, sizeof.SK_RX_DESC
        add     edi, eax
        pop     eax
        test    [edi + SK_RX_DESC.sk_ctl], SK_RXCTL_OWN
        jnz     .no_rx_ISR                        ; caught up with HW

        push    eax
        mov     eax, [edi + SK_RX_DESC.sk_ctl]
        and     eax, SK_RXCTL_LEN
        DEBUGF  1, "RX slot %u len=%u\n", esi, eax

        mov     ecx, [ebx + SK_DEVICE.rx_buffers]
        mov     edx, [ecx + esi*4]
        mov     [edx + NET_BUFF.length], eax
        mov     [edx + NET_BUFF.device], ebx
        mov     [edx + NET_BUFF.offset], NET_BUFF.data

        inc     [ebx + SK_DEVICE.packets_rx]
        add     dword[ebx + SK_DEVICE.bytes_rx], eax
        adc     dword[ebx + SK_DEVICE.bytes_rx + 4], 0

        push    esi edx
        invoke  NetAlloc, MAX_PKT_SIZE + NET_BUFF.data
        pop     edx esi
        test    eax, eax
        jz      .rx_oom

        mov     ecx, [ebx + SK_DEVICE.rx_buffers]
        mov     [ecx + esi*4], eax
        push    eax
        add     eax, NET_BUFF.data
        invoke  GetPhysAddr
        mov     ecx, eax
        pop     eax

        mov     edi, [ebx + SK_DEVICE.rx_ring]
        push    eax
        imul    eax, esi, sizeof.SK_RX_DESC
        add     edi, eax
        pop     eax
        mov     [edi + SK_RX_DESC.sk_data_lo], ecx
        mov     [edi + SK_RX_DESC.sk_data_hi], 0
        mov     [edi + SK_RX_DESC.sk_ctl], MAX_PKT_SIZE or SK_RXSTAT

        inc     esi
        cmp     esi, RX_RING_SIZE
        jb      @f
        xor     esi, esi
  @@:   mov     [ebx + SK_DEVICE.cur_rx], esi

        pop     eax

        push    eax ecx ebx
        push    .retaddr
        push    edx
        jmp     [EthInput]
  .retaddr:
        pop     ebx ecx eax

        jmp     .rx_loop

  .rx_oom:
        DEBUGF  2, "RX out of memory - leaving slot parked\n"
        pop     eax

  .no_rx_ISR:
; Always clear EOF + encoding-error and re-kick RX
        mov     eax, SK_RXQ1_BMU_CSR
        mov     ecx, SK_RXBMU_CLR_IRQ_EOF or SK_RXBMU_CLR_IRQ_ERR or SK_RXBMU_RX_START
        call    sk_write32

        ; ---- TX done? ----
        mov     eax, [esp]                        ; original ISR value, still on stack
        test    eax, SK_ISR_TX1_S_EOF or SK_ISR_TX1_S_CHECK or SK_ISR_TX1_S_EOB
        jz      .no_tx_ISR
        call    clean_tx
        mov     eax, SK_TXQS1_BMU_CSR
        mov     ecx, SK_TXBMU_CLR_IRQ_EOF or SK_TXBMU_CLR_IRQ_ERR
        call    sk_write32
  .no_tx_ISR:

        pop     eax                                ; discard the pushed ISR value

        ; PHY / external register interrupt
        test    eax, SK_ISR_EXTERNAL_REG
        jz      .no_phy

        ; Clear PHY interrupt by reading INT_STAT
        push    eax
        mov     eax, PHY_MARV_INT_STAT
        call    phy_read
        DEBUGF  1, "PHY_INT_STAT = 0x%x\n", eax:4
        test    eax, PHY_M_IS_LST_CHANGE or PHY_M_IS_AN_COMPL or PHY_M_IS_LSP_CHANGE
        jz      .phy_done
        call    detect_link
  .phy_done:
        pop     eax

  .no_phy:
        ; GMAC interrupt (FIFO over/under etc.) – just ack for now
        test    eax, SK_ISR_MAC1
        jz      .no_mac
        push    eax
        mov     eax, SK_GMAC_ISR
        call    sk_read8                        ; read to clear
        pop     eax
  .no_mac:

        ; Re-enable interrupt mask
        mov     eax, SK_IMR
        mov     ecx, [ebx + SK_DEVICE.intr_mask]
        call    sk_write32

        pop     edi esi ebx
        xor     eax, eax
        inc     eax                             ; handled
        ret

  .none:
        pop     edi esi ebx
        xor     eax, eax                        ; not ours
        ret

align 4
reset:
; ebx = SK_DEVICE
        DEBUGF  1, "reset()\n"

;----- Controller soft reset (OpenBSD skc_reset) ---------------------
        mov     eax, SK_CSR
        mov     ecx, SK_CSR_SW_RESET
        call    sk_write16

        mov     eax, SK_CSR
        mov     ecx, SK_CSR_MASTER_RESET
        call    sk_write16

; Link reset (Yukon only)
        cmp     byte [ebx + SK_DEVICE.chip_id], SK_GENESIS
        je      .no_link_rst_set
        mov     eax, SK_LINK_CTRL
        mov     ecx, SK_LINK_RESET_SET
        call    sk_write16
  .no_link_rst_set:

; Delay ~1 ms
        mov     ecx, 200000
@@:     loop    @b

        mov     eax, SK_CSR
        mov     ecx, SK_CSR_SW_UNRESET
        call    sk_write16

        mov     ecx, 1000
@@:     loop    @b

        mov     eax, SK_CSR
        mov     ecx, SK_CSR_MASTER_UNRESET
        call    sk_write16

        cmp     byte [ebx + SK_DEVICE.chip_id], SK_GENESIS
        je      .no_link_rst_clr
        mov     eax, SK_LINK_CTRL
        mov     ecx, SK_LINK_RESET_CLEAR
        call    sk_write16
  .no_link_rst_clr:

; Enable RAM interface
        mov     eax, SK_RAMCTL
        mov     ecx, SK_RAMCTL_UNRESET
        call    sk_write32

; Driver-loaded LED on
        mov     eax, SK_LED
        mov     ecx, SK_LED_GREEN_ON
        call    sk_write16

;----- Yukon GPHY + GMAC bring-up ------------------------------------
        cmp     byte [ebx + SK_DEVICE.chip_id], SK_GENESIS
        je      .reset_done

; Put GPHY/GMAC into reset
        mov     eax, SK_GPHY_CTRL
        mov     ecx, SK_GPHY_RESET_SET
        call    sk_write32
        mov     eax, SK_GMAC_CTRL
        mov     ecx, SK_GMAC_RESET_SET
        call    sk_write32

; Build GPHY control value
        mov     ecx, SK_GPHY_INT_POL_HI or SK_GPHY_DIS_FC or \
                     SK_GPHY_DIS_SLEEP or SK_GPHY_ENA_XC or \
                     SK_GPHY_ANEG_ALL or SK_GPHY_ENA_PAUSE
        cmp     byte [ebx + SK_DEVICE.copper], 0
        je      .fiber
        or      ecx, SK_GPHY_COPPER
        jmp     .phy_cfg
  .fiber:
        or      ecx, SK_GPHY_FIBER
  .phy_cfg:
        push    ecx

        mov     eax, SK_GPHY_CTRL
        or      ecx, SK_GPHY_RESET_SET
        call    sk_write32

        mov     ecx, 200000
@@:     loop    @b

        pop     ecx
        mov     eax, SK_GPHY_CTRL
        or      ecx, SK_GPHY_RESET_CLEAR
        call    sk_write32

; Clear GMAC reset
        mov     eax, SK_GMAC_CTRL
        mov     ecx, SK_GMAC_LOOP_OFF or SK_GMAC_PAUSE_ON or SK_GMAC_RESET_CLEAR
        call    sk_write32

        call    ram_init

; Enable TX arbiter (without this frames never leave the chip)
        mov     eax, SK_TXAR1_COUNTERCTL
        mov     ecx, SK_TXARCTL_ON or SK_TXARCTL_FSYNC_ON
        call    sk_write8
        DEBUGF  1, "TX arbiter on\n"

        call    init_rx_ring
        test    eax, eax
        jnz     .ring_fail
        call    init_tx_ring
        test    eax, eax
        jnz     .ring_fail

; Program MAC into Yukon source-address registers
        lea     esi, [ebx + SK_DEVICE.mac]
        movzx   ecx, word [esi]
        mov     eax, YUKON_REG_BASE + YUKON_SAL1
        call    sk_write32
        movzx   ecx, word [esi + 2]
        mov     eax, YUKON_REG_BASE + YUKON_SAM1
        call    sk_write32
        movzx   ecx, word [esi + 4]
        mov     eax, YUKON_REG_BASE + YUKON_SAH1
        call    sk_write32

; Receive control: strip CRC, enable unicast + multicast filters
        mov     eax, YUKON_REG_BASE + YUKON_RCR
        mov     ecx, YU_RCR_CRCR or YU_RCR_UFLEN or YU_RCR_MUFLEN
        call    sk_write32

; Transmit parameters + serial mode (required for GMAC to clock frames)
        mov     eax, YUKON_REG_BASE + YUKON_TPR
        mov     ecx, YU_TPR_JAM_LEN or YU_TPR_JAM_IPG or YU_TPR_JAM2DATA_IPG
        call    sk_write32

        mov     eax, YUKON_REG_BASE + YUKON_SMR
        mov     ecx, YU_SMR_DATA_BLIND or YU_SMR_MFL_VLAN or YU_SMR_IPG_DATA
        call    sk_write32

; Enable RX + TX
        mov     eax, YUKON_REG_BASE + YUKON_GPCR
        call    sk_read32
        or      eax, YU_GPCR_TXEN or YU_GPCR_RXEN
        mov     ecx, eax
        mov     eax, YUKON_REG_BASE + YUKON_GPCR
        call    sk_write32

        DEBUGF  1, "GPHY/GMAC out of reset, RX+TX enabled\n"

; Enable PHY interrupts (link / speed / AN error / FIFO)
        mov     eax, PHY_MARV_INT_MASK
        mov     ecx, PHY_M_IS_DEF_MSK
        call    phy_write

; Unmask EXTERNAL_REG (PHY) + MAC1 in the controller IMR
        mov     ecx, SK_ISR_EXTERNAL_REG or SK_INTRS1  ; SK_ISR_EXTERNAL_REG or SK_ISR_MAC1
        mov     [ebx + SK_DEVICE.intr_mask], ecx
        mov     eax, SK_IMR
        call    sk_write32

; Initial link status
        call    detect_link

  .reset_done:
        xor     eax, eax
        ret

  .ring_fail:
        or      eax, -1
        ret

;----------------------------------------------------------------------
;  ram_init - partition on-chip SRAM RX/TX 50-50, bring MAC FIFOs online
;
;  CAVEAT: the EPROM0 size table and the ">> 3" unit assumed for the
;  RAMbuffer START/END registers are transcribed from if_skreg.h /
;  general SK docs, not confirmed against a real EPROM0 dump on your
;  boards. Print the raw byte first; if it doesn't match one of the
;  four known codes, we skip straight to just enabling the FIFOs
;  (correct behaviour for Yukon Lite LOM parts with no external SRAM).
;----------------------------------------------------------------------
align 4
ram_init:
; Yukon (OpenBSD/Linux): if EPROM0==0 → 128KB, else size = code * 4096
; Genesis uses a different table (kept for completeness).
        mov     eax, SK_EPROM0
        call    sk_read8
        DEBUGF  1, "EPROM0 ramsize code = 0x%x\n", eax:2
        mov     ecx, eax

        cmp     byte [ebx + SK_DEVICE.chip_id], SK_GENESIS
        je      .genesis_size

; ---- Yukon family ----
        test    cl, cl
        jnz     .yuk_scaled
        mov     [ebx + SK_DEVICE.ramsize], 0x20000   ; 128 KB special case
        jmp     .have_size
  .yuk_scaled:
        movzx   eax, cl
        shl     eax, 12                              ; * 4096
        mov     [ebx + SK_DEVICE.ramsize], eax
        jmp     .have_size

  .genesis_size:
        cmp     cl, SK_RAMSIZE_512K_64
        je      .sz512
        cmp     cl, SK_RAMSIZE_1024K_128
        je      .sz1024
        cmp     cl, SK_RAMSIZE_1024K_64
        je      .sz1024
        cmp     cl, SK_RAMSIZE_2048K_128
        je      .sz2048
        DEBUGF  2, "Genesis: unknown RAMbuffer code, FIFO-only\n"
        mov     [ebx + SK_DEVICE.ramsize], 0
        jmp     .fifo_only

  .sz512:
        mov     [ebx + SK_DEVICE.ramsize], 512*1024
        jmp     .have_size
  .sz1024:
        mov     [ebx + SK_DEVICE.ramsize], 1024*1024
        jmp     .have_size
  .sz2048:
        mov     [ebx + SK_DEVICE.ramsize], 2048*1024

  .have_size:
        mov     eax, [ebx + SK_DEVICE.ramsize]
        shr     eax, 1
        mov     [ebx + SK_DEVICE.rx_ramstart], 0
        mov     [ebx + SK_DEVICE.rx_ramend], eax
        mov     [ebx + SK_DEVICE.tx_ramstart], eax
        mov     ecx, [ebx + SK_DEVICE.ramsize]
        mov     [ebx + SK_DEVICE.tx_ramend], ecx
        DEBUGF  1, "RAM split: rx=[0-0x%x) tx=[0x%x-0x%x)\n", \
                [ebx + SK_DEVICE.rx_ramend], \
                [ebx + SK_DEVICE.tx_ramstart], [ebx + SK_DEVICE.tx_ramend]

        mov     eax, SK_RXRB1_CTLTST
        mov     ecx, SK_RBCTL_RESET
        call    sk_write32
        mov     eax, SK_RXRB1_START
        xor     ecx, ecx
        call    sk_write32
        mov     eax, SK_RXRB1_WR_PTR
        call    sk_write32
        mov     eax, SK_RXRB1_RD_PTR
        call    sk_write32
        mov     eax, SK_RXRB1_END
        mov     ecx, [ebx + SK_DEVICE.rx_ramend]
        shr     ecx, 3
        dec     ecx
        call    sk_write32
        mov     eax, SK_RXRB1_CTLTST
        mov     ecx, SK_RBCTL_UNRESET
        call    sk_write32
        mov     eax, SK_RXRB1_CTLTST
        mov     ecx, SK_RBCTL_ON
        call    sk_write32

        mov     eax, SK_TXRBS1_CTLTST
        mov     ecx, SK_RBCTL_RESET
        call    sk_write32
        mov     eax, SK_TXRBS1_START
        mov     ecx, [ebx + SK_DEVICE.tx_ramstart]
        shr     ecx, 3
        call    sk_write32
        mov     eax, SK_TXRBS1_WR_PTR
        call    sk_write32
        mov     eax, SK_TXRBS1_RD_PTR
        call    sk_write32
        mov     eax, SK_TXRBS1_END
        mov     ecx, [ebx + SK_DEVICE.tx_ramend]
        shr     ecx, 3
        dec     ecx
        call    sk_write32
        mov     eax, SK_TXRBS1_CTLTST
        mov     ecx, SK_RBCTL_UNRESET
        call    sk_write32
        mov     eax, SK_TXRBS1_CTLTST
        mov     ecx, SK_RBCTL_STORENFWD_ON
        call    sk_write32
        mov     eax, SK_TXRBS1_CTLTST
        mov     ecx, SK_RBCTL_ON
        call    sk_write32

  .fifo_only:
; Yukon uses GMAC FIFO (GMF), Genesis uses MFF – branch on chip family
        cmp     byte [ebx + SK_DEVICE.chip_id], SK_GENESIS
        je      .genesis_fifo

; ---- Yukon GMF ----
        mov     eax, RX_GMF_FL_MSK
        mov     ecx, RX_GMF_FL_DEF_MSK
        call    sk_write32

        mov     eax, RX_GMF_CTRL_T
        mov     ecx, GMF_RST_CLR
        call    sk_write8

        mov     eax, RX_GMF_CTRL_T
        mov     ecx, GMF_OPER_ON or GMF_RX_F_FL_ON
        call    sk_write16

        mov     eax, RX_GMF_FL_THR
        mov     ecx, RX_GMF_FL_THR_DEF + 1
        call    sk_write32

        mov     eax, TX_GMF_CTRL_T
        mov     ecx, GMF_RST_CLR
        call    sk_write8

        mov     eax, TX_GMF_CTRL_T
        mov     ecx, GMF_OPER_ON
        call    sk_write16

        DEBUGF  1, "Yukon GMF FIFOs online\n"
        ret

  .genesis_fifo:
        mov     eax, SK_RXF1_CTL
        mov     ecx, SK_RFCTL_RESET_SET
        call    sk_write32
        mov     eax, SK_RXF1_CTL
        mov     ecx, SK_RFCTL_RESET_CLEAR
        call    sk_write32
        mov     eax, SK_RXF1_CTL
        mov     ecx, SK_RFCTL_OPERATION_ON
        call    sk_write32

        mov     eax, SK_TXF1_CTL
        mov     ecx, SK_TFCTL_RESET_SET
        call    sk_write32
        mov     eax, SK_TXF1_CTL
        mov     ecx, SK_TFCTL_RESET_CLEAR
        call    sk_write32
        mov     eax, SK_TXF1_CTL
        mov     ecx, SK_TFCTL_OPERATION_ON
        call    sk_write32
        ret



;----------------------------------------------------------------------
;  init_rx_ring - allocate + post RX_RING_SIZE descriptors, start RX BMU
;----------------------------------------------------------------------
align 4
init_rx_ring:
        push    esi edi

        invoke  KernelAlloc, sizeof.SK_RX_DESC * RX_RING_SIZE
        test    eax, eax
        jz      .fail
        mov     [ebx + SK_DEVICE.rx_ring], eax
        invoke  GetPhysAddr
        mov     [ebx + SK_DEVICE.rx_ring_phys], eax

        invoke  KernelAlloc, 4 * RX_RING_SIZE
        test    eax, eax
        jz      .fail
        mov     [ebx + SK_DEVICE.rx_buffers], eax
        mov     [ebx + SK_DEVICE.cur_rx], 0

        xor     esi, esi
  .loop:
        invoke  NetAlloc, MAX_PKT_SIZE + NET_BUFF.data
        test    eax, eax
        jz      .fail
        mov     edi, [ebx + SK_DEVICE.rx_buffers]
        mov     [edi + esi*4], eax

        push    eax
        add     eax, NET_BUFF.data
        invoke  GetPhysAddr
        mov     ecx, eax
        pop     eax

        mov     edi, [ebx + SK_DEVICE.rx_ring]
        push    eax
        imul    eax, esi, sizeof.SK_RX_DESC
        add     edi, eax
        pop     eax
        mov     [edi + SK_RX_DESC.sk_data_lo], ecx
        mov     [edi + SK_RX_DESC.sk_data_hi], 0
        mov     [edi + SK_RX_DESC.sk_ctl], MAX_PKT_SIZE or SK_RXSTAT

        mov     eax, esi
        inc     eax
        cmp     eax, RX_RING_SIZE
        jb      @f
        xor     eax, eax
  @@:   imul    eax, sizeof.SK_RX_DESC
        add     eax, [ebx + SK_DEVICE.rx_ring_phys]
        mov     [edi + SK_RX_DESC.sk_next], eax

        inc     esi
        cmp     esi, RX_RING_SIZE
        jb      .loop

        mov     eax, SK_RXQ1_BMU_CSR
        mov     ecx, SK_RXBMU_ONLINE
        call    sk_write32
        mov     eax, SK_RXQ1_CURADDR_LO
        mov     ecx, [ebx + SK_DEVICE.rx_ring_phys]
        call    sk_write32
        mov     eax, SK_RXQ1_CURADDR_HI
        xor     ecx, ecx
        call    sk_write32
        mov     eax, SK_RXQ1_BMU_CSR
        mov     ecx, SK_RXBMU_RX_START
        call    sk_write32

        DEBUGF  1, "RX ring posted, %u descriptors\n", RX_RING_SIZE
        pop     edi esi
        xor     eax, eax
        ret
  .fail:
        DEBUGF  2, "init_rx_ring: allocation failed\n"
        pop     edi esi
        or      eax, -1
        ret

;----------------------------------------------------------------------
;  init_tx_ring - allocate empty TX_RING_SIZE descriptor ring
;----------------------------------------------------------------------
align 4
init_tx_ring:
        push    esi edi

        invoke  KernelAlloc, sizeof.SK_TX_DESC * TX_RING_SIZE
        test    eax, eax
        jz      .fail
        mov     [ebx + SK_DEVICE.tx_ring], eax
        invoke  GetPhysAddr
        mov     [ebx + SK_DEVICE.tx_ring_phys], eax

        invoke  KernelAlloc, 4 * TX_RING_SIZE
        test    eax, eax
        jz      .fail
        mov     [ebx + SK_DEVICE.tx_buffers], eax
        mov     edi, eax
        mov     ecx, TX_RING_SIZE
        xor     eax, eax
        rep     stosd                            ; 0 == free slot, checked by clean_tx

        mov     [ebx + SK_DEVICE.cur_tx], 0
        mov     [ebx + SK_DEVICE.dirty_tx], 0

        xor     esi, esi
  .loop:
        mov     edi, [ebx + SK_DEVICE.tx_ring]
        imul    eax, esi, sizeof.SK_TX_DESC
        add     edi, eax
        mov     [edi + SK_TX_DESC.sk_ctl], 0    ; OWN clear = owned by driver

        mov     eax, esi
        inc     eax
        cmp     eax, TX_RING_SIZE
        jb      @f
        xor     eax, eax
  @@:
        imul    eax, sizeof.SK_TX_DESC
        add     eax, [ebx + SK_DEVICE.tx_ring_phys]
        mov     [edi + SK_TX_DESC.sk_next], eax

        inc     esi
        cmp     esi, TX_RING_SIZE
        jb      .loop

        mov     eax, SK_TXQS1_BMU_CSR
        mov     ecx, SK_TXBMU_ONLINE
        call    sk_write32

        mov     eax, SK_TXQS1_CURADDR_LO
        mov     ecx, [ebx + SK_DEVICE.tx_ring_phys]
        call    sk_write32
        mov     eax, SK_TXQS1_CURADDR_HI
        xor     ecx, ecx
        call    sk_write32

        DEBUGF  1, "TX ring initialised, %u descriptors\n", TX_RING_SIZE
        pop     edi esi
        xor     eax, eax
        ret
  .fail:
        DEBUGF  2, "init_tx_ring: allocation failed\n"
        pop     edi esi
        or      eax, -1
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Transmit                                 ;;
;; In:  ebx = device, [bufferptr] = NET_BUFF ;;
;; Out: eax = 0 on success                  ;;
;;
;; Must be stdcall - the old bare-label version left bufferptr on the
;; stack on every call, which is what corrupted state and produced the
;; page faults in BOARDLOG.TXT.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
proc transmit stdcall, bufferptr:dword

        spin_lock_irqsave

        mov     esi, [bufferptr]
        DEBUGF  1, "transmit() buffer=%x size=%u\n", esi, [esi + NET_BUFF.length]

        cmp     [esi + NET_BUFF.length], MAX_PKT_SIZE
        ja      .fail
        cmp     [esi + NET_BUFF.length], 60
        jae     @f
        mov     [esi + NET_BUFF.length], 60
  @@:
        mov     eax, [ebx + SK_DEVICE.cur_tx]
        inc     eax
        and     eax, TX_RING_SIZE-1
        cmp     eax, [ebx + SK_DEVICE.dirty_tx]
        je      .fail                            ; ring full

        mov     edi, [ebx + SK_DEVICE.tx_ring]
        imul    eax, [ebx + SK_DEVICE.cur_tx], sizeof.SK_TX_DESC
        add     edi, eax

        mov     eax, [ebx + SK_DEVICE.cur_tx]
        mov     ecx, [ebx + SK_DEVICE.tx_buffers]
        mov     [ecx + eax*4], esi               ; clean_tx frees this later

        mov     eax, esi
        add     eax, [esi + NET_BUFF.offset]
        invoke  GetPhysAddr
        mov     [edi + SK_TX_DESC.sk_data_lo], eax
        mov     [edi + SK_TX_DESC.sk_data_hi], 0

        mov     eax, [esi + NET_BUFF.length]
        or      eax, SK_TXSTAT                   ; OWN bit set last -> hands off to HW
        mov     [edi + SK_TX_DESC.sk_ctl], eax

        mov     eax, [ebx + SK_DEVICE.cur_tx]
        inc     eax
        and     eax, TX_RING_SIZE-1
        mov     [ebx + SK_DEVICE.cur_tx], eax

        mov     eax, SK_TXQS1_BMU_CSR
        mov     ecx, SK_TXBMU_TX_START
        call    sk_write32

        inc     [ebx + SK_DEVICE.packets_tx]
        mov     eax, [esi + NET_BUFF.length]
        add     dword[ebx + SK_DEVICE.bytes_tx], eax
        adc     dword[ebx + SK_DEVICE.bytes_tx + 4], 0

        ;call    clean_tx

        spin_unlock_irqrestore
        xor     eax, eax
        ret

  .fail:
        ;call    clean_tx
        DEBUGF  2, "transmit: send failed\n"
        invoke  NetFree, [bufferptr]
        spin_unlock_irqrestore
        or      eax, -1
        ret

endp

;----------------------------------------------------------------------
;  clean_tx - reclaim descriptors the TX BMU has finished with
;----------------------------------------------------------------------
clean_tx:
        DEBUGF  1, "Clean TX\n"

        push    eax ecx edx esi edi
  .loop:
        mov     eax, [ebx + SK_DEVICE.dirty_tx]
        cmp     eax, [ebx + SK_DEVICE.cur_tx]
        je      .done

        mov     edi, [ebx + SK_DEVICE.tx_ring]
        imul    ecx, eax, sizeof.SK_TX_DESC
        add     edi, ecx
        test    [edi + SK_TX_DESC.sk_ctl], SK_TXCTL_OWN
        jnz     .done                             ; HW still owns it

        mov     esi, [ebx + SK_DEVICE.tx_buffers]
        mov     edx, [esi + eax*4]
        test    edx, edx
        jz      @f
        DEBUGF  1, "Cleaning TX buffer 0x%x\n", edx
        push    eax
        invoke  NetFree, edx
        pop     eax
        mov     esi, [ebx + SK_DEVICE.tx_buffers]
        mov     dword[esi + eax*4], 0
  @@:
        inc     eax
        and     eax, TX_RING_SIZE-1
        mov     [ebx + SK_DEVICE.dirty_tx], eax
        jmp     .loop
  .done:
        pop     edi esi edx ecx eax
        ret


align 4
unload:
        DEBUGF  1, "unload() called\n"
        xor     eax, eax
        ret

;----------------------------------------------------------------------
;  Data
;----------------------------------------------------------------------
; End of code


data fixups
end data

include '../peimport.inc'

my_service      db '88E80XX',0

;----------------------------------------------------------------------
; Chip ID -> name table  (CHIPVER byte, pad, name pointer)
; Same idea as 3c59x hw_versions / hw_str tables.
;----------------------------------------------------------------------
align 4
chip_table:
        db SK_GENESIS,  0,0,0
        dd name_genesis
        db SK_YUKON,    0,0,0
        dd name_yukon
        db SK_YUKON_LITE,0,0,0
        dd name_yukon_lite
        db SK_YUKON_LP, 0,0,0
        dd name_yukon_lp
CHIP_TABLE_COUNT = ($ - chip_table) / 8

name_genesis    db 'Marvell GEnesis',0
name_yukon      db 'Marvell Yukon',0
name_yukon_lite db 'Marvell Yukon Lite',0
name_yukon_lp   db 'Marvell Yukon LP',0

; Revision strings for Yukon Lite
name_rev_a0     db 'A0',0
name_rev_a1     db 'A1',0
name_rev_a3     db 'A3',0

include_debug_strings

align 4
devices         dd 0
device_list     rd MAX_DEVICES
