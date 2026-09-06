;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  mar_yuk2.asm  -  Marvell Yukon-2 (88E8055 and friends) driver
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
        __DEBUG_LEVEL__         = 2             ; 1 = verbose while developing

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'
include 'mar_yuk2.inc'

;----------------------------------------------------------------------
;  Local MMIO helpers  (BAR0 is a linear 16 KB region on Yukon-2)
;----------------------------------------------------------------------

; ebx = device, eax = offset  ->  eax = value
align 4
proc sky2_read32
        mov     edx, [ebx + device.mmio_addr]
        mov     eax, [edx + eax]
        ret
endp

align 4
proc sky2_write32
        ; ebx = device, eax = offset, ecx = value
        mov     edx, [ebx + device.mmio_addr]
        mov     [edx + eax], ecx
        ret
endp

align 4
proc sky2_read16
        mov     edx, [ebx + device.mmio_addr]
        movzx   eax, word [edx + eax]
        ret
endp

align 4
proc sky2_write16
        mov     edx, [ebx + device.mmio_addr]
        mov     [edx + eax], cx
        ret
endp

align 4
proc sky2_read8
        mov     edx, [ebx + device.mmio_addr]
        movzx   eax, byte [edx + eax]
        ret
endp

align 4
proc sky2_write8
        mov     edx, [ebx + device.mmio_addr]
        mov     [edx + eax], cl
        ret
endp

macro udelay u {
local .loop
        push    ecx
        mov     ecx, u
  .loop:
        in      al, 0x80
        dec     ecx
        jnz     .loop
        pop     ecx
}

;----------------------------------------------------------------------
;  Per-port register helpers.
;
;  The Yukon-2 register map has two different per-port strides:
;    - GMAC/PHY block (GM_*, GM_SMI_*): stride 0x1000, base BASE_GMAC_1
;    - "SK_REG" block  (GMAC_CTRL, GMAC_LINK_CTRL, TXA_CTRL, ...): stride 0x80
;
;  These wrap the plain sky2_read*/write* helpers above so callers just
;  pass a port-relative offset and device.port is folded in automatically.
;  None of them touch ebx, so ebx (device ptr) survives across calls.
;----------------------------------------------------------------------

; ebx = device, eax = GM_* offset (relative to per-port GMAC block) -> eax = value
align 4
proc sky2_gma_read16
        push    edx
        movzx   edx, [ebx + device.port]
        shl     edx, 12                          ; port stride = BASE_GMAC_2-BASE_GMAC_1
        add     eax, edx
        add     eax, BASE_GMAC_1
        pop     edx
        call    sky2_read16
        ret
endp

; ebx = device, eax = GM_* offset, ecx = value
align 4
proc sky2_gma_write16
        push    edx
        movzx   edx, [ebx + device.port]
        shl     edx, 12
        add     eax, edx
        add     eax, BASE_GMAC_1
        pop     edx
        call    sky2_write16
        ret
endp

; ebx = device, eax = SK_REG-style offset -> eax = value
align 4
proc sky2_sk_read8
        push    edx
        movzx   edx, [ebx + device.port]
        shl     edx, 7                            ; port stride = 0x80
        add     eax, edx
        pop     edx
        call    sky2_read8
        ret
endp

; ebx = device, eax = SK_REG-style offset, ecx = value
align 4
proc sky2_sk_write8
        push    edx
        movzx   edx, [ebx + device.port]
        shl     edx, 7
        add     eax, edx
        pop     edx
        call    sky2_write8
        ret
endp

align 4
proc sky2_sk_write16
        push    edx
        movzx   edx, [ebx + device.port]
        shl     edx, 7
        add     eax, edx
        pop     edx
        call    sky2_write16
        ret
endp

align 4
proc sky2_sk_write32
        push    edx
        movzx   edx, [ebx + device.port]
        shl     edx, 7
        add     eax, edx
        pop     edx
        call    sky2_write32
        ret
endp

; ebx = device, eax = SK_REG-style offset -> eax = value
align 4
proc sky2_sk_read32
        push    edx
        movzx   edx, [ebx + device.port]
        shl     edx, 7
        add     eax, edx
        pop     edx
        call    sky2_read32
        ret
endp

;----------------------------------------------------------------------
;  MMIO-mirrored PCI config space (Y2_CFG_SPC window) - not a real PCI
;  config cycle, just another MMIO offset. Used only for the per-chip
;  "OUR_REG"/PCI_DEV_REG* power-control registers, which aren't
;  per-port, so no port-stride folding here.
;----------------------------------------------------------------------

; ebx = device, eax = PCI_DEV_REG* offset -> eax = value
align 4
proc sky2_cfg_read32
        add     eax, Y2_CFG_SPC
        call    sky2_read32
        ret
endp

; ebx = device, eax = PCI_DEV_REG* offset, ecx = value
align 4
proc sky2_cfg_write32
        add     eax, Y2_CFG_SPC
        call    sky2_write32
        ret
endp

;----------------------------------------------------------------------
;  Queue (Q_CSR/Q_WM/Q_AL/Q_TEST), prefetch-unit (PREF_UNIT_*), and
;  RAM-buffer (RB_*) register helpers. All three are per-queue blocks
;  (queue = Q_R1 or Q_XA1 for us) but with different bases and strides
;  from the per-port SK_REG block used elsewhere - queue offset IS the
;  stride here, not multiplied by anything, since Q_R1/Q_XA1/etc are
;  already absolute offsets within their respective block.
;----------------------------------------------------------------------

; ebx = device, eax = Q_CSR-style offset, ecx = queue (Q_R1/Q_XA1) -> eax = B8_Q_REGS+queue+offset, then read/write
align 4
proc sky2_q_read32
        add     eax, ecx
        add     eax, B8_Q_REGS
        call    sky2_read32
        ret
endp

; ebx = device, eax = offset, ecx = queue, edx = value
align 4
proc sky2_q_write32
        push    ecx
        add     eax, ecx
        add     eax, B8_Q_REGS
        mov     ecx, edx
        call    sky2_write32
        pop     ecx
        ret
endp

; ebx = device, eax = offset, ecx = queue, edx = value
align 4
proc sky2_q_write16
        push    ecx
        add     eax, ecx
        add     eax, B8_Q_REGS
        mov     ecx, edx
        call    sky2_write16
        pop     ecx
        ret
endp

; ebx = device, eax = offset, ecx = queue -> eax = prefetch-unit register value
align 4
proc sky2_pref_read32
        add     eax, ecx
        add     eax, Y2_B8_PREF_REGS
        call    sky2_read32
        ret
endp

; ebx = device, eax = offset, ecx = queue, edx = value
align 4
proc sky2_pref_write32
        push    ecx
        add     eax, ecx
        add     eax, Y2_B8_PREF_REGS
        mov     ecx, edx
        call    sky2_write32
        pop     ecx
        ret
endp

; ebx = device, eax = offset, ecx = queue, edx = value
align 4
proc sky2_pref_write16
        push    ecx
        add     eax, ecx
        add     eax, Y2_B8_PREF_REGS
        mov     ecx, edx
        call    sky2_write16
        pop     ecx
        ret
endp

; ebx = device, eax = offset, ecx = queue, edx = value
align 4
proc sky2_rb_write32
        push    ecx
        add     eax, ecx
        add     eax, B16_RAM_REGS
        mov     ecx, edx
        call    sky2_write32
        pop     ecx
        ret
endp

; ebx = device, eax = offset, ecx = queue, edx = value (low byte used)
align 4
proc sky2_rb_write8
        push    ecx
        add     eax, ecx
        add     eax, B16_RAM_REGS
        mov     ecx, edx
        call    sky2_write8
        pop     ecx
        ret
endp

;----------------------------------------------------------------------
;  PHY (MDIO/SMI) access.  The Yukon-2 GMAC has a built-in serial
;  management interface state machine (GM_SMI_CTRL/GM_SMI_DATA) that
;  talks to the internal Marvell 88E1xxx PHY at fixed MDIO address 0.
;----------------------------------------------------------------------

; ebx = device, eax = PHY register (0..31), ecx = 16-bit value to write
; destroys eax/ecx/edx; returns eax = 0 on success, -1 on timeout/IO error
align 4
proc sky2_phy_write
        push    esi
        push    edi

        mov     edi, ecx                          ; stash value
        mov     esi, eax                          ; stash phy register #

        mov     eax, GM_SMI_DATA
        mov     ecx, edi
        call    sky2_gma_write16

        mov     eax, esi
        shl     eax, 6
        and     eax, GM_SMI_CT_REG_A_MSK           ; PHY_AD field is 0 (PHY_ADDR_MARV)
        mov     ecx, eax
        mov     eax, GM_SMI_CTRL
        call    sky2_gma_write16

        mov     edi, PHY_RETRIES                  ; retry counter (esi is needed for Sleep below)
  .wait:
        mov     eax, GM_SMI_CTRL
        call    sky2_gma_read16
        cmp     ax, 0xFFFF
        je      .err
        test    ax, GM_SMI_CT_BUSY
        jz      .ok

        ; A raw cycle-count spin here is not a reliable time unit across
        ; different CPU speeds - that's what caused the read timeout seen
        ; in the field log. Use the kernel's real delay instead, same as
        ; RTL8169/i8254x/ar81xx do for their MDIO/reset polling.
        xor     esi, esi
        inc     esi                                ; Sleep(1) == ~1ms (units are ms, not cs - confirmed empirically)
        invoke  Sleep

        dec     edi
        jnz     .wait

  .err:
        DEBUGF  2, "PHY write timeout/error\n"
        pop     edi
        pop     esi
        or      eax, -1
        ret

  .ok:
        pop     edi
        pop     esi
        xor     eax, eax
        ret
endp

; ebx = device, eax = PHY register (0..31)
; destroys eax/ecx/edx; returns eax = 16-bit value (zero-extended),
; edx = 0 on success, -1 on timeout/IO error (value is undefined then)
align 4
proc sky2_phy_read
        push    esi
        push    edi

        mov     esi, eax                          ; stash phy register #
        shl     eax, 6
        and     eax, GM_SMI_CT_REG_A_MSK
        or      eax, GM_SMI_CT_OP_RD
        mov     ecx, eax
        mov     eax, GM_SMI_CTRL
        call    sky2_gma_write16

        mov     edi, PHY_RETRIES
  .wait:
        mov     eax, GM_SMI_CTRL
        call    sky2_gma_read16
        cmp     ax, 0xFFFF
        je      .err
        test    ax, GM_SMI_CT_RD_VAL
        jnz     .ready

        xor     esi, esi
        inc     esi                                ; Sleep(1) == ~1ms (units are ms, not cs - confirmed empirically)
        invoke  Sleep

        dec     edi
        jnz     .wait

  .err:
        DEBUGF  2, "PHY read timeout/error\n"
        pop     edi
        pop     esi
        xor     eax, eax
        or      edx, -1
        ret

  .ready:
        mov     eax, GM_SMI_DATA
        call    sky2_gma_read16
        pop     edi
        pop     esi
        xor     edx, edx
        ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START c, reason:dword, cmdline:dword

        cmp     [reason], DRV_ENTRY
        jne     .fail

        DEBUGF  1,"Loading driver\n"
        invoke  RegService, my_service, service_proc
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

align 4
proc service_proc stdcall, ioctl:dword

        mov     edx, [ioctl]
        mov     eax, [edx + IOCTL.io_code]

;------------------------------------------------------

        cmp     eax, 0 ;SRV_GETVERSION
        jne     @F

        cmp     [edx + IOCTL.out_size], 4
        jb      .fail
        mov     eax, [edx + IOCTL.output]
        mov     dword[eax], API_VERSION

        xor     eax, eax
        ret

;------------------------------------------------------
  @@:
        cmp     eax, 1 ;SRV_HOOK
        jne     .fail

        cmp     [edx + IOCTL.inp_size], 3               ; Data input must be at least 3 bytes
        jb      .fail

        mov     eax, [edx + IOCTL.input]
        cmp     byte[eax], 1                            ; 1 means device number and bus number (pci) are given
        jne     .fail                                   ; other types aren't supported for this card yet

; check if the device is already listed

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

;        mov     eax, [edx + IOCTL.input]                ; get the pci bus and device numbers
        mov     ax, [eax+1]                             ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte[ebx + device.pci_bus]
        jne     .next
        cmp     ah, byte[ebx + device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find its device number
  .next:
        add     esi, 4
        loop    .nextdevice


; This device doesn't have its own eth_device structure yet, let's create one
  .firstdevice:
        cmp     [devices], MAX_DEVICES                  ; First check if the driver can handle one more card
        jae     .fail

        allocate_and_clear ebx, sizeof.device, .fail    ; Allocate the buffer for device structure

; Fill in the direct call addresses into the struct

        mov     [ebx + device.reset], reset
        mov     [ebx + device.transmit], transmit
        mov     [ebx + device.unload], unload
        mov     [ebx + device.name], my_service

; save the pci bus and device numbers

        mov     eax, [edx + IOCTL.input]
        movzx   ecx, byte[eax+1]
        mov     [ebx + device.pci_bus], ecx
        movzx   ecx, byte[eax+2]
        mov     [ebx + device.pci_dev], ecx

; Now, it's time to find the base mmio address of the PCI device

        stdcall PCI_find_mmio, [ebx + device.pci_bus], [ebx + device.pci_dev] ; returns in eax
        test    eax, eax
        jz      .destroy

; Create virtual mapping of the physical memory

        invoke  MapIoMem, eax, 10000h, PG_SW+PG_NOCACHE
        mov     [ebx + device.mmio_addr], eax

; We've found the mmio address, find IRQ now

        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.interrupt_line
        mov     [ebx + device.irq_line], al

        DEBUGF  1,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [ebx + device.pci_dev]:1,[ebx + device.pci_bus]:1,[ebx + device.irq_line]:1,[ebx + device.mmio_addr]:8

; Ok, the eth_device structure is ready, let's probe the device
        call    probe                                   ; this function will output in eax
        test    eax, eax
        jnz     .err                                    ; If an error occured, exit

        mov     eax, [devices]                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                ; (IRQ handler uses this list to find device)
        inc     [devices]                               ;

        call    reset
        test    eax, eax
        jnz     .destroy

        mov     [ebx + device.type], NET_TYPE_ETH
        mov     [ebx + device.mtu], 1500
        invoke  NetRegDev
        cmp     eax, -1
        je      .destroy

        ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  1,"Trying to find device number of already registered device\n"
        invoke  NetPtrToNum                             ; This kernel procedure converts a pointer to device struct in ebx
                                                        ; into a device number in edi
        mov     eax, edi                                ; Application wants it in eax instead
        DEBUGF  1,"Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        ; todo: reset device into virgin state

  .err:
        invoke  KernelFree, ebx

  .fail:
        DEBUGF  2,"Loading driver failed\n"
        or      eax, -1
        ret

;------------------------------------------------------
endp

;----------------------------------------------------------------------
;  probe – identify Yukon-2 chip and read permanent MAC address
;----------------------------------------------------------------------

align 4
probe:
        push    ebx

        ; PCI bus-mastering must be explicitly enabled or the chip is
        ; not permitted to DMA to/from system memory at all - without
        ; this, the prefetch units can never fetch our LE rings and the
        ; BMU can never write status entries back, so RX/TX (and their
        ; interrupts) silently never happen even though register-level
        ; MMIO, the PHY, and link detection all keep working fine (none
        ; of that needs DMA). Missing this was the root cause of "no
        ; frames on the wire, no RX/TX interrupts" - caught by spotting
        ; it never gets touched anywhere else in this file either.
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER or PCI_CMD_MMIO
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

        ; Take the ASIC out of reset (same sequence used by Linux sky2)
        mov     eax, B0_CTST
        mov     cx, CS_RST_CLR
        call    sky2_write16

        udelay 1000

        ; Also clear master reset
        mov     eax, B0_CTST
        mov     cx, CS_MRST_CLR
        call    sky2_write16

        udelay 1000

        ;---- Chip ID -------------------------------------------------
        mov     eax, B2_CHIP_ID
        call    sky2_read8
        mov     [ebx + device.chip_id], al
        DEBUGF  1, "CHIP_ID = 0x%x\n", eax:2

        ; Revision lives in the high nibble of B2_MAC_CFG
        mov     eax, B2_MAC_CFG
        call    sky2_read8
        shr     al, 4
        mov     [ebx + device.chip_rev], al
        DEBUGF  1, "chip revision = %u\n", al

        ; Look up a friendly name
        mov     esi, name_unknown
        mov     al, [ebx + device.chip_id]
        cmp     al, CHIP_ID_YUKON_EC_U
        jne     @f
        mov     esi, name_ec_u
        jmp     .name_done
@@:     cmp     al, CHIP_ID_YUKON_EC
        jne     @f
        mov     esi, name_ec
        jmp     .name_done
@@:     cmp     al, CHIP_ID_YUKON_XL
        jne     @f
        mov     esi, name_xl
        jmp     .name_done
@@:     cmp     al, CHIP_ID_YUKON_EX
        jne     @f
        mov     esi, name_ex
        jmp     .name_done
@@:     cmp     al, CHIP_ID_YUKON_FE
        jne     @f
        mov     esi, name_fe
        jmp     .name_done
@@:     cmp     al, CHIP_ID_YUKON_FE_P
        jne     @f
        mov     esi, name_fe_p
        jmp     .name_done
@@:     cmp     al, CHIP_ID_YUKON_SUPR
        jne     @f
        mov     esi, name_supr
        jmp     .name_done
@@:     cmp     al, CHIP_ID_YUKON_UL_2
        jne     @f
        mov     esi, name_ul2
        jmp     .name_done
@@:
        ; Accept any other Yukon-2 ID for now; we only need the MAC
        cmp     al, 0xB3
        jb      .unsupported
        cmp     al, 0xBE
        ja      .unsupported
        mov     esi, name_yukon2
  .name_done:
        mov     [ebx + device.chip_name], esi
        mov     [ebx + device.name], esi
        DEBUGF  1, "chip: %s\n", esi
        jmp     .read_mac

  .unsupported:
        DEBUGF  2, "unsupported CHIP_ID 0x%x\n", \
                [ebx + device.chip_id]:2
        mov     eax, -1
        pop     ebx
        ret

  .read_mac:




        DEBUGF  1, "=== MMIO 0x0000-0x007F ===\n"
        xor     esi, esi
@@:
        mov     eax, esi
        call    sky2_read32
        DEBUGF  1, "0x%x: 0x%x\n", esi, eax
        add     esi, 4
        cmp     esi, 0x80
        jb      @b


        ;---- Permanent MAC address (port 0) --------------------------
        ; Linux: memcpy_fromio(dev->dev_addr, hw->regs + B2_MAC_1, 6);
        lea     edi, [ebx + device.mac]
        xor     esi, esi
  .mac_loop:
        mov     eax, B2_MAC_1
        add     eax, esi
        call    sky2_read8
        mov     [edi + esi], al
        inc     esi
        cmp     esi, 6
        jb      .mac_loop

        DEBUGF  1, "MAC %x:%x:%x:%x:%x:%x\n", \
                [ebx + device.mac + 0]:2, \
                [ebx + device.mac + 1]:2, \
                [ebx + device.mac + 2]:2, \
                [ebx + device.mac + 3]:2, \
                [ebx + device.mac + 4]:2, \
                [ebx + device.mac + 5]:2

        ; Sanity check: reject all-zero or all-FF
        mov     eax, dword [ebx + device.mac]
        test    eax, eax
        jz      .bad_mac
        cmp     eax, 0xFFFFFFFF
        je      .bad_mac
        movzx   eax, word [ebx + device.mac + 4]
        test    ax, ax
        jz      .mac_ok                     ; first 4 non-zero is enough
        cmp     ax, 0xFFFF
        je      .bad_mac
        jmp     .mac_ok

  .bad_mac:
        DEBUGF  2, "invalid MAC address read from B2_MAC_1\n"
        ; Still return success so we can inspect further;
        ; a production driver would fall back to VPD/EEPROM here.

  .mac_ok:
        mov     [ebx + device.port], 0
        xor     eax, eax                        ; success
        pop     ebx
        ret
;endp

;----------------------------------------------------------------------
;  reset – bring the ASIC out of reset and configure the PHY for
;          auto-negotiation.  Deliberately stops short of RAM buffer /
;          BMU / Tx-arbiter setup: no rings exist yet, so none of that
;          is needed just to get a link up. That comes with the TX/RX
;          ring milestone.
;----------------------------------------------------------------------

align 4
reset:
        ; ebx = device, per the convention already used by probe/transmit/unload
        DEBUGF  1, "reset()\n"


; --- 0. enable cfg-space mirror writes ---
mov     eax, B2_TST_CTRL1          ; 0x0158
mov     cl, TST_CFG_WRITE_ON
call    sky2_write8

; --- 1. kill ASF hard (PXE left it running) ---
mov     eax, B28_Y2_ASF_STAT_CMD   ; 0x0E68
mov     ecx, Y2_ASF_RESET          ; bit 3
call    sky2_write32

mov     eax, B0_CTST
mov     cx, Y2_ASF_DISABLE         ; bit 12
call    sky2_write16

; --- 2. clocks ---
mov     eax, B2_Y2_CLK_GATE        ; 0x011D
xor     cl, cl                     ; 0 = all clocks on
call    sky2_write8

mov     eax, B2_Y2_CLK_CTRL        ; 0x0120
mov     cl, Y2_CLK_DIV_DIS
call    sky2_write8

; --- 3. clear PHY power-down / coma in PCI_DEV_REG1 ---
mov     eax, PCI_DEV_REG1          ; 0x40 via cfg window
call    sky2_cfg_read32
and     eax, not (PCI_Y2_PHY1_POWD or PCI_Y2_PHY1_COMA or PCI_Y2_PHY2_POWD or PCI_Y2_PHY2_COMA)
mov     ecx, eax
mov     eax, PCI_DEV_REG1
call    sky2_cfg_write32

; --- 4. clear any ASPM / clock-request bits that lock the core (EC-U) ---
mov     eax, PCI_DEV_REG4          ; 0x84
call    sky2_cfg_read32
and     eax, not P_ASPM_CONTROL_MSK
mov     ecx, eax
mov     eax, PCI_DEV_REG4
call    sky2_cfg_write32

mov     eax, PCI_DEV_REG5          ; 0x88
call    sky2_cfg_read32
and     eax, not P_CTL_TIM_VMAIN_AV_MSK
mov     ecx, eax
mov     eax, PCI_DEV_REG5
call    sky2_cfg_write32

; --- 5. done with cfg writes ---
mov     eax, B2_TST_CTRL1
mov     cl, TST_CFG_WRITE_OFF
call    sky2_write8

; --- 6. CTST: soft reset clear + master reset clear ---
mov     eax, B0_CTST
mov     cx, CS_RST_CLR
call    sky2_write16
udelay  1000

mov     eax, B0_CTST
mov     cx, CS_MRST_CLR
call    sky2_write16
udelay  1000

; --- 7. NOW the DEADBEEF test on PREF ---


mov     eax, B0_CTST
call    sky2_read16
DEBUGF  1, "CTST=%x\n", eax

mov     eax, B2_Y2_CLK_GATE
call    sky2_read8
DEBUGF  1, "CLK_GATE=%x\n", eax

mov     eax, B2_Y2_CLK_CTRL
call    sky2_read8
DEBUGF  1, "CLK_CTRL=%x\n", eax

mov     eax, B28_Y2_ASF_STAT_CMD
call    sky2_read32
DEBUGF  1, "ASF_STAT_CMD=%x\n", eax

mov     eax, PCI_DEV_REG1
call    sky2_cfg_read32
DEBUGF  1, "PCI_DEV_REG1=%x\n", eax

mov     eax, PCI_DEV_REG3
call    sky2_cfg_read32
DEBUGF  1, "PCI_DEV_REG3=%x\n", eax

mov     eax, PCI_DEV_REG4
call    sky2_cfg_read32
DEBUGF  1, "PCI_DEV_REG4=%x\n", eax

mov     eax, PCI_DEV_REG5
call    sky2_cfg_read32
DEBUGF  1, "PCI_DEV_REG5=%x\n", eax

        ; ---- allow writes to PCI config space, clear any latched PCI errors ----
        mov     eax, B2_TST_CTRL1
        mov     cl, TST_CFG_WRITE_ON
        call    sky2_write8

        invoke  PciRead16, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.status
        movzx   ecx, ax
        or      ecx, PCI_STATUS_ERROR_BITS
        invoke  PciWrite16, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.status, ecx

        mov     eax, B0_CTST
        mov     cl, CS_MRST_CLR
        call    sky2_write8

        ; ---- power on: VCC (not VAUX), disable core clock division ----
        mov     eax, B0_POWER_CTRL
        mov     cl, 0xA6                           ; PC_VAUX_ENA|PC_VCC_ENA|PC_VAUX_OFF|PC_VCC_ON
        call    sky2_write8

        mov     eax, B2_Y2_CLK_CTRL
        mov     ecx, Y2_CLK_DIV_DIS
        call    sky2_write32

        mov     eax, B2_Y2_CLK_GATE
        xor     ecx, ecx
        call    sky2_write8

        ; ---- "advanced power control" dance (EC-Ultra/EX/FE+ only).
        ; Not just power tuning as I'd assumed earlier - the GPIO status
        ; race workaround here matters for these chips, and this whole
        ; block runs immediately before the PHY power-up bits get
        ; cleared below. Matches Linux SKY2_HW_ADV_POWER_CTL and
        ; OpenBSD's equivalent EC_U/EX/FE_P branch.
        ; Ensure descriptor byte order is native (little-endian).
        ; PCI_REV_DESC reverses LE bytes in hardware; if left set by
        ; firmware/BIOS the BMU sees garbage opcodes and raises CHK on
        ; every list element. Linux only sets this on big-endian CPUs.
        mov     eax, PCI_DEV_REG2
        call    sky2_cfg_read32
        DEBUGF  1, "PCI_DEV_REG2 was %x\n", eax
        and     eax, not (PCI_REV_DESC or PCI_USEDATA64)
        or      eax, PCI_EN_DUMMY_RD
        mov     ecx, eax
        mov     eax, PCI_DEV_REG2
        call    sky2_cfg_write32

        mov     eax, PCI_DEV_REG3
        xor     ecx, ecx
        call    sky2_cfg_write32

        mov     eax, PCI_DEV_REG4
        call    sky2_cfg_read32
        and     eax, P_ASPM_CONTROL_MSK
        mov     ecx, eax
        mov     eax, PCI_DEV_REG4
        call    sky2_cfg_write32

        mov     eax, PCI_DEV_REG5
        call    sky2_cfg_read32
        and     eax, P_CTL_TIM_VMAIN_AV_MSK
        mov     ecx, eax
        mov     eax, PCI_DEV_REG5
        call    sky2_cfg_write32

        mov     eax, PCI_CFG_REG_1
        xor     ecx, ecx
        call    sky2_cfg_write32

        mov     eax, B0_CTST
        mov     cx, Y2_HW_WOL_ON
        call    sky2_write16

        mov     eax, B2_GP_IO
        call    sky2_read32
        or      eax, GLB_GPIO_STAT_RACE_DIS
        mov     ecx, eax
        mov     eax, B2_GP_IO
        call    sky2_write32
        mov     eax, B2_GP_IO
        call    sky2_read32                       ; read back, matches reference drivers

        ; turn on "driver loaded" status LED, just for visual confirmation
        mov     eax, B0_CTST
        mov     cx, Y2_LED_STAT_ON
        call    sky2_write16

        mov     eax, B2_TST_CTRL1
        mov     cl, TST_CFG_WRITE_OFF
        call    sky2_write8

        ; ---- RAM interface timeouts (Linux sky2_reset, always done) ----
        ; Even chips with no external RAM (EC Ultra, ramsize=0) need the
        ; RI block out of reset and the per-queue timeouts programmed,
        ; otherwise the BMU never hands frames to the MAC FIFO.
        mov     eax, B3_RI_CTRL
        mov     cl, RI_RST_CLR
        call    sky2_write8
        mov     esi, B3_RI_WTO_R1
  .ri_to_loop:
        mov     eax, esi
        mov     cl, SK_RI_TO_53
        call    sky2_write8
        inc     esi
        cmp     esi, B3_RI_RTO_XS2 + 1
        jb      .ri_to_loop

        ; ---- per-port GMAC link reset (single port only, for now) ----
        mov     eax, GMAC_LINK_CTRL
        mov     cl, GMLC_RST_SET
        call    sky2_sk_write8
        mov     eax, GMAC_LINK_CTRL
        mov     cl, GMLC_RST_CLR
        call    sky2_sk_write8

        ; small settle delay - chip needs a moment after the link reset
        ; before the GMAC/PHY block responds to MDIO cleanly. A raw cycle
        ; count isn't a reliable time unit across CPU speeds, so use the
        ; kernel's real delay here too (see the note in sky2_phy_read).
        push    esi
        xor     esi, esi
        inc     esi                                ; Sleep(1) == ~1ms (units are ms, not cs - confirmed empirically)
        invoke  Sleep
        pop     esi

        ; ---- GMAC core reset + GPHY (internal PHY) block reset ----
        ; GMAC_LINK_CTRL above is a DIFFERENT reset domain from these two.
        ; Missing the GPHY_CTRL reset specifically is what left the PHY
        ; permanently deaf to MDIO on first bring-up - confirmed against
        ; both Linux's sky2_mac_init() and OpenBSD's msk_reset().
        mov     eax, GMAC_CTRL
        mov     cl, GMC_RST_SET
        call    sky2_sk_write8
        mov     eax, GPHY_CTRL
        mov     cl, GPC_RST_SET
        call    sky2_sk_write8

        push    esi
        xor     esi, esi
        inc     esi                                ; Sleep(1) == ~1ms (units are ms, not cs - confirmed empirically)
        invoke  Sleep
        pop     esi

        mov     eax, GPHY_CTRL
        mov     cl, GPC_RST_CLR
        call    sky2_sk_write8
        mov     eax, GMAC_CTRL
        mov     cl, GMC_LOOP_OFF or GMC_PAUSE_ON or GMC_RST_CLR
        call    sky2_sk_write8

        ; ---- PHY power-up: clear the per-port power-down/coma bits in
        ; the PCI_DEV_REG1 mirror, then release the GPHY reset once more
        ; (matches Linux sky2_phy_power_up(), redundant-looking second
        ; GPC_RST_CLR included since that's what the reference does)
        mov     eax, PCI_DEV_REG1
        call    sky2_cfg_read32
        and     eax, not (PCI_Y2_PHY1_POWD or PCI_Y2_PHY2_POWD or \
                          PCI_Y2_PHY1_COMA or PCI_Y2_PHY2_COMA)
        mov     ecx, eax
        mov     eax, PCI_DEV_REG1
        call    sky2_cfg_write32

        mov     eax, GPHY_CTRL
        mov     cl, GPC_RST_CLR
        call    sky2_sk_write8

        ; ---- GMF (MAC FIFO) bring-up ---------------------------------
        ; Required by Linux sky2_mac_init(), OpenBSD msk, and FreeBSD
        ; msk. Without GMF_OPER_ON the MAC never hands frames to the
        ; BMU, so the status unit stays silent even though the prefetch
        ; units happily walk the LE rings (the exact symptom in the
        ; field log: GET_IDX advances, STAT_PUT_IDX stays 0).
        ;
        ; Sequence matches all three reference drivers:
        ;   RST_SET → RST_CLR → OPER_ON (+ RX flush-on for RX)

        ; RX GMF
        mov     eax, RX_GMF_CTRL_T
        mov     cl, GMF_RST_SET
        call    sky2_sk_write8
        mov     eax, RX_GMF_CTRL_T
        mov     cl, GMF_RST_CLR
        call    sky2_sk_write8
        mov     eax, RX_GMF_CTRL_T
        mov     ecx, GMF_OPER_ON or GMF_RX_F_FL_ON
        call    sky2_sk_write32

        ; TX GMF — OPER_ON first, then OR in STFW for no-RAM chips.
        ; Earlier code wrote TX_STFW_ENA alone and relied on sticky bits;
        ; make the OR explicit so OPER_ON cannot be lost.
        mov     eax, TX_GMF_CTRL_T
        mov     cl, GMF_RST_SET
        call    sky2_sk_write8
        mov     eax, TX_GMF_CTRL_T
        mov     cl, GMF_RST_CLR
        call    sky2_sk_write8
        ; For EC Ultra (no RAM): OPER_ON | STFW in one write.
        ; For others: OPER_ON alone.
        movzx   eax, [ebx + device.chip_id]
        cmp     eax, CHIP_ID_YUKON_EC_U
        jne     .tx_gmf_plain
        mov     eax, RX_GMF_LP_THR
        mov     cl, 768 / 8
        call    sky2_sk_write8
        mov     eax, RX_GMF_UP_THR
        mov     cl, 1024 / 8
        call    sky2_sk_write8
        mov     eax, TX_GMF_CTRL_T
        mov     ecx, GMF_OPER_ON or TX_STFW_ENA
        call    sky2_sk_write32
        jmp     .gmf_done
  .tx_gmf_plain:
        mov     eax, TX_GMF_CTRL_T
        mov     ecx, GMF_OPER_ON
        call    sky2_sk_write32
  .gmf_done:

        ; collision threshold (Linux TX_COL_THR(TX_COL_DEF) = 4<<10)
        mov     eax, GM_TX_CTRL
        mov     ecx, 4 shl 10
        call    sky2_gma_write16

        ; serial mode: data-blind + default IPG for 1000
        mov     eax, GM_SERIAL_MODE
        mov     ecx, (4 shl 11) or 0x1e
        call    sky2_gma_write16

        ; ---- station address (permanent MAC → GM_SRC_ADDR_1/2) -------
        ; Linux gma_set_addr() / OpenBSD equivalent. Required on some
        ; Yukon-EC-U revisions before the MAC will TX or accept unicast.
        movzx   ecx, word [ebx + device.mac]        ; bytes 0-1
        mov     eax, GM_SRC_ADDR_1L
        call    sky2_gma_write16
        movzx   ecx, word [ebx + device.mac + 2]    ; bytes 2-3
        mov     eax, GM_SRC_ADDR_1M
        call    sky2_gma_write16
        movzx   ecx, word [ebx + device.mac + 4]    ; bytes 4-5
        mov     eax, GM_SRC_ADDR_1H
        call    sky2_gma_write16

        movzx   ecx, word [ebx + device.mac]
        mov     eax, GM_SRC_ADDR_2L
        call    sky2_gma_write16
        movzx   ecx, word [ebx + device.mac + 2]
        mov     eax, GM_SRC_ADDR_2M
        call    sky2_gma_write16
        movzx   ecx, word [ebx + device.mac + 4]
        mov     eax, GM_SRC_ADDR_2H
        call    sky2_gma_write16

        ; ---- basic GMAC bring-up: no IRQs yet, accept our own MAC ----
        mov     eax, GMAC_IRQ_MSK
        xor     ecx, ecx
        call    sky2_sk_write8

        mov     eax, GM_MC_ADDR_H1
        xor     ecx, ecx
        call    sky2_gma_write16
        mov     eax, GM_MC_ADDR_H2
        xor     ecx, ecx
        call    sky2_gma_write16
        mov     eax, GM_MC_ADDR_H3
        xor     ecx, ecx
        call    sky2_gma_write16
        mov     eax, GM_MC_ADDR_H4
        xor     ecx, ecx
        call    sky2_gma_write16

        mov     eax, GM_RX_CTRL
        call    sky2_gma_read16
        or      ax, GM_RXCR_UCF_ENA or GM_RXCR_MCF_ENA
        mov     ecx, eax
        mov     eax, GM_RX_CTRL
        call    sky2_gma_write16

        ; enable MAC RX/TX (all three reference drivers do this)
        mov     eax, GM_GP_CTRL
        call    sky2_gma_read16
        or      ax, GM_GPCR_RX_ENA or GM_GPCR_TX_ENA
        mov     ecx, eax
        mov     eax, GM_GP_CTRL
        call    sky2_gma_write16

        ; ---- PHY: reset it and kick off auto-negotiation ----
        call    phy_init

        ; ---- enable PHY link-change interrupt ----
        ; PHY_MARV_INT_MASK enables the PHY's *own* interrupt sources;
        ; the chip-wide B0_IMSK mask (which also has to let this through
        ; to the shared IRQ line) is set once, combined with the status-
        ; BMU bit, inside hw_start below - no point unmasking anything
        ; at the top level before the interrupt handler is registered.
        mov     eax, PHY_MARV_INT_MASK
        mov     ecx, PHY_M_DEF_MSK
        call    sky2_phy_write

        movzx   eax, [ebx + device.irq_line]
        invoke  AttachIntHandler, eax, int_handler, ebx
        test    eax, eax
        jnz     @f
        DEBUGF  2, "could not attach interrupt handler\n"
  @@:

        ; ---- bring up the TX/RX rings, BMU, and status ring, and only
        ; now unmask interrupts at the chip level (handler is already
        ; registered above, so nothing can be missed) ----
        call    hw_start

        ; Auto-negotiation isn't instant (IEEE 802.3 FLP/parallel-detect
        ; can take several hundred ms - empirically, several *seconds*
        ; on at least one real link partner), so a single synchronous
        ; read here is likely to catch it mid-negotiation - that's
        ; almost certainly what produced the bogus 10Mbps/half-duplex
        ; reading in an earlier field log. This bounded poll is a
        ; load-time diagnostic only, to show the negotiation settling
        ; in the boot log; the PHY IRQ just wired up above is the real,
        ; ongoing mechanism (also covers cable unplug/replug after boot,
        ; which this one-shot poll never will).
;mov     edi, 10                           ; up to ~5s total (10 * 500ms)
;  .link_poll:
;        call    link_status
;        push    esi
;        mov     esi, 500                          ; Sleep(500) == ~500ms (units are ms, not cs)
;        invoke  Sleep
;        pop     esi
;        dec     edi
;        jnz     .link_poll


mov     ecx, Q_XA1
mov     eax, Q_CSR
mov     edx, BMU_CLR_IRQ_CHK
call    sky2_q_write32

mov     ecx, Q_R1
mov     eax, Q_CSR
mov     edx, BMU_CLR_IRQ_CHK
call    sky2_q_write32

        xor     eax, eax
        ret

;----------------------------------------------------------------------
;  phy_init – configure the internal Marvell copper PHY for
;             auto-negotiation across 10/100/1000 full & half duplex,
;             then restart auto-neg.
;
;             Scope note: this covers the CHIP_ID_YUKON_EC_U / "newer
;             PHY" gigabit-copper case only (our current bring-up
;             target).  Fiber (88E1112) and non-gigabit (FE/FE+) PHYs
;             need the extra setup Linux does in sky2_phy_init() - not
;             handled here yet.
;----------------------------------------------------------------------

align 4
phy_init:
        ; disable energy detect, enable automatic MDI/MDIX crossover,
        ; enable 3x downshift (gigabit copper, "newer PHY" variant)
        mov     eax, PHY_MARV_PHY_CTRL
        call    sky2_phy_read
        test    edx, edx
        jnz     .mdio_dead                        ; PHY not answering - don't
                                                    ; write derived garbage back
        and     ax, not PHY_M_PC_EN_DET_MSK
        or      ax, (PHY_M_PC_ENA_AUTO shl 5) and PHY_M_PC_MDIX_MSK
        and     ax, not PHY_M_PC_DSC_MSK
        or      ax, (2 shl 12) or PHY_M_PC_DOWN_S_ENA
        mov     ecx, eax
        mov     eax, PHY_MARV_PHY_CTRL
        call    sky2_phy_write

        ; advertise all copper speeds/duplex modes
        mov     eax, PHY_MARV_1000T_CTRL
        mov     ecx, PHY_M_1000C_AFD or PHY_M_1000C_AHD
        call    sky2_phy_write

        mov     eax, PHY_MARV_AUNE_ADV
        mov     ecx, PHY_AN_CSMA or PHY_M_AN_100_FD or PHY_M_AN_100_HD \
                     or PHY_M_AN_10_FD or PHY_M_AN_10_HD
        call    sky2_phy_write

        ; enable auto-negotiation, ask it to restart, and reset the PHY
        mov     eax, PHY_MARV_CTRL
        mov     ecx, PHY_CT_RESET or PHY_CT_ANE or PHY_CT_RE_CFG
        call    sky2_phy_write

        DEBUGF  1, "phy_init() done, auto-neg restarted\n"
        ret

  .mdio_dead:
        DEBUGF  2, "phy_init() aborted, MDIO bus not responding\n"
        ret

;----------------------------------------------------------------------
;  link_status – read the GMAC's resolved link state (post auto-neg)
;                and cache it in the device struct. Cheap enough to
;                poll from a timer once the driver has one; for now
;                it's just called once after reset() so we can see
;                what the chip thinks the link is doing.
;----------------------------------------------------------------------

align 4
link_status:
        ; PHY_MARV_PHY_STAT (via MDIO) is what Linux's IRQ-driven link
        ; decode actually reads - GM_GP_STAT (plain MMIO, what this used
        ; to read) apparently doesn't reliably track live PHY state; on
        ; at least one board it kept reporting stale "1000Mbps link up"
        ; after a physical disconnect. Switched to match the reference.
        mov     eax, PHY_MARV_PHY_STAT
        call    sky2_phy_read
        test    edx, edx
        jnz     .mdio_dead

        test    ax, PHY_M_PS_LINK_UP
        jz      .down

        mov     byte [ebx + device.link_up], 1

        test    ax, PHY_M_PS_FULL_DUP
        setnz   [ebx + device.link_duplex]

        mov     ecx, eax                          ; speed field survives past setnz
        and     cx, PHY_M_PS_SPEED_MSK
        mov     edx, ETH_LINK_SPEED_10M
        cmp     cx, PHY_M_PS_SPEED_1000
        je      .is_1000
        cmp     cx, PHY_M_PS_SPEED_100
        je      .is_100

        mov     word [ebx + device.link_speed], 10
        jmp     .report

  .is_100:
        mov     edx, ETH_LINK_SPEED_100M
        mov     word [ebx + device.link_speed], 100
        jmp     .report

  .is_1000:
        mov     edx, ETH_LINK_SPEED_1G
        mov     word [ebx + device.link_speed], 1000

  .report:
        cmp     byte [ebx + device.link_duplex], 0
        jz      @f
        or      edx, ETH_LINK_FULL_DUPLEX
  @@:
        ; device.state is the standard ETH_DEVICE field the kernel network
        ; stack actually reads - our own link_up/link_speed/link_duplex
        ; fields above are just for the DEBUGF text below.
        mov     [ebx + device.state], edx
        invoke  NetLinkChanged

        movzx   eax, byte [ebx + device.link_duplex]
        mov     esi, str_half
        test    eax, eax
        jz      @f
        mov     esi, str_full
  @@:
        movzx   ecx, word [ebx + device.link_speed]
        DEBUGF  1, "link UP, %u Mbps, %s duplex\n", ecx, esi


; after setting device.state / NetLinkChanged on link UP:
; Force speed/duplex into GM_GP_CTRL (disable auto-update) so the MAC
; matches the PHY. Observed 0x18CA had GIGS+SPEED_100 but no DUP_FULL.
        mov     eax, GM_GP_CTRL
        call    sky2_gma_read16
        and     ax, not (GM_GPCR_SPEED_1000 or GM_GPCR_DUP_FULL or \
                         GM_GPCR_AU_SPD_DIS or GM_GPCR_AU_DUP_DIS or \
                         GM_GPCR_AU_FCT_DIS)
        or      ax, GM_GPCR_TX_ENA or GM_GPCR_RX_ENA or \
                    GM_GPCR_AU_SPD_DIS or GM_GPCR_AU_DUP_DIS or GM_GPCR_AU_FCT_DIS
        cmp     word [ebx + device.link_speed], 1000
        jne     .gp_spd100
        or      ax, GM_GPCR_SPEED_1000
        jmp     .gp_dup
  .gp_spd100:
        cmp     word [ebx + device.link_speed], 100
        jne     .gp_dup
        or      ax, GM_GPCR_SPEED_100
  .gp_dup:
        cmp     byte [ebx + device.link_duplex], 0
        je      .gp_write
        or      ax, GM_GPCR_DUP_FULL
  .gp_write:
        mov     ecx, eax
        mov     eax, GM_GP_CTRL
        call    sky2_gma_write16
        DEBUGF  1, "GM_GP_CTRL after link-up force=%x\n", ecx
        ret

  .mdio_dead:
        DEBUGF  2, "link_status() MDIO read failed\n"
        ret

  .down:
mov     eax, GM_GP_CTRL
call    sky2_gma_read16
and     ax, not (GM_GPCR_TX_ENA or GM_GPCR_RX_ENA)
mov     ecx, eax
mov     eax, GM_GP_CTRL
call    sky2_gma_write16


        mov     byte [ebx + device.link_up], 0
        mov     word [ebx + device.link_speed], 0
        mov     dword [ebx + device.state], ETH_LINK_DOWN
        invoke  NetLinkChanged
        DEBUGF  1, "link DOWN\n"
        ret

;----------------------------------------------------------------------
;  sky2_phy_intr – service a PHY interrupt: read (and thereby clear)
;                  the PHY's own interrupt status register, then
;                  refresh the cached link state. ebx = device.
;----------------------------------------------------------------------

align 4
sky2_phy_intr:
        mov     eax, PHY_MARV_INT_STAT
        call    sky2_phy_read                     ; clear-on-read ack
        test    edx, edx
        jnz     .mdio_dead

        call    link_status
        ret

  .mdio_dead:
        DEBUGF  2, "PHY interrupt fired but MDIO read failed\n"
        ret

;----------------------------------------------------------------------
;  int_handler – shared IRQ entry point. Handles both the PHY link-
;                change source and the status BMU (RX/TX completion)
;                source. Follows the same convention as the other in-
;                tree drivers (RTL8169/i8254x/ar81xx): device ptr
;                arrives on the stack, return eax=1 if we handled
;                something so the kernel knows not to try the next
;                handler on a shared IRQ line, eax=0 otherwise.
;----------------------------------------------------------------------

align 4
int_handler:
        push    ebx esi edi

        mov     ebx, [esp + 4*4]

        mov     eax, B0_ISRC
        call    sky2_read32
        test    eax, eax
        jz      .nothing
        cmp     eax, -1
        je      .nothing                           ; card gone (all-ones read)

        mov     esi, eax                            ; keep the raw status around
        xor     edi, edi                            ; edi = "did we handle anything"

        test    esi, Y2_IS_IRQ_PHY1
        jz      @f
        call    sky2_phy_intr
        inc     edi
  @@:
        test    esi, Y2_IS_STAT_BMU
        jz      @f
        call    sky2_status_intr
        inc     edi
  @@:
        test    edi, edi
        jz      .nothing

        pop     edi esi ebx
        xor     eax, eax
        inc     eax
        ret

  .nothing:
        pop     edi esi ebx
        xor     eax, eax
        ret

;----------------------------------------------------------------------
;  sky2_ramset – configure one queue's slice of the chip's internal
;                RAM buffer. stdcall: queue, start_kb, space_kb.
;                ebx = device throughout, as everywhere else.
;----------------------------------------------------------------------

align 4
proc sky2_ramset stdcall, queue:dword, start_kb:dword, space_kb:dword
        push    esi edi

        mov     eax, [start_kb]
        shl     eax, 7                              ; KB -> qwords (*1024/8)
        mov     esi, eax
        mov     eax, [space_kb]
        shl     eax, 7
        mov     edi, eax

        mov     ecx, [queue]
        mov     eax, RB_CTRL
        mov     edx, RB_RST_CLR
        call    sky2_rb_write8

        mov     ecx, [queue]
        mov     eax, RB_START
        mov     edx, esi
        call    sky2_rb_write32

        mov     ecx, [queue]
        mov     eax, RB_END
        lea     edx, [esi + edi - 1]
        call    sky2_rb_write32

        mov     ecx, [queue]
        mov     eax, RB_WP
        mov     edx, esi
        call    sky2_rb_write32

        mov     ecx, [queue]
        mov     eax, RB_RP
        mov     edx, esi
        call    sky2_rb_write32

        cmp     dword [queue], Q_R1
        jne     .tx_queue

        ; RX queue: give receiver priority when > 3/4 full, ask the link
        ; partner to pause when down to 2K free (all thresholds in
        ; qwords, matching edi = space in qwords)
        mov     eax, edi
        mov     ecx, edi
        shr     ecx, 2
        sub     eax, ecx                            ; tp = space - space/4
        push    eax
        mov     ecx, [queue]
        mov     eax, RB_RX_UTHP
        pop     edx
        call    sky2_rb_write32

        mov     ecx, [queue]
        mov     eax, RB_RX_LTHP
        mov     edx, edi
        shr     edx, 1
        call    sky2_rb_write32

        mov     eax, edi
        sub     eax, 1024                           ; space - 8192/8
        push    eax
        mov     ecx, [queue]
        mov     eax, RB_RX_UTPP
        pop     edx
        call    sky2_rb_write32

        mov     ecx, [queue]
        mov     eax, RB_RX_LTPP
        mov     edx, edi
        shr     edx, 2
        call    sky2_rb_write32

        jmp     .enable

  .tx_queue:
        ; Tx FIFO is only 1K on this hardware, so force store & forward
        mov     ecx, [queue]
        mov     eax, RB_CTRL
        mov     edx, RB_ENA_STFWD
        call    sky2_rb_write8

  .enable:
        mov     ecx, [queue]
        mov     eax, RB_CTRL
        mov     edx, RB_ENA_OP_MD
        call    sky2_rb_write8

        pop     edi esi
        ret
endp

;----------------------------------------------------------------------
;  sky2_qset – bring one queue's BMU (Buffer Management Unit) out of
;              reset and operational. stdcall: queue.
;----------------------------------------------------------------------

align 4
proc sky2_qset stdcall, queue:dword
        mov     ecx, [queue]
        mov     eax, Q_CSR
        mov     edx, BMU_CLR_RESET
        call    sky2_q_write32

        mov     ecx, [queue]
        mov     eax, Q_CSR
        mov     edx, BMU_OPER_INIT
        call    sky2_q_write32

        mov     ecx, [queue]
        mov     eax, Q_CSR
        mov     edx, BMU_FIFO_OP_ON
        call    sky2_q_write32

        mov     ecx, [queue]
        mov     eax, Q_WM
        mov     edx, BMU_WM_DEFAULT
        call    sky2_q_write16
        ret
endp

;----------------------------------------------------------------------
;  sky2_prefetch_init – point a queue's prefetch unit at our LE ring
;                        and switch it on. stdcall: queue, phys_addr,
;                        last_idx (= ring size - 1).
;----------------------------------------------------------------------

align 4
proc sky2_prefetch_init stdcall, queue:dword, phys_addr:dword, last_idx:dword
        DEBUGF  1, "prefetch_init q=%x phys=%x last=%u\n", [queue]:4, [phys_addr]:8, [last_idx]

        ; ---- raw absolute addresses ----
        mov     eax, [queue]
        add     eax, Y2_B8_PREF_REGS          ; base for this queue

        ; CTRL = RST_SET
        mov     edx, [ebx + device.mmio_addr]
        mov     dword [edx + eax + PREF_UNIT_CTRL], PREF_UNIT_RST_SET

        ; CTRL = RST_CLR
        mov     dword [edx + eax + PREF_UNIT_CTRL], PREF_UNIT_RST_CLR

        ; ADDR_HI = 0
        mov     dword [edx + eax + PREF_UNIT_ADDR_HI], 0

        ; ADDR_LO
        mov     ecx, [phys_addr]
        mov     dword [edx + eax + PREF_UNIT_ADDR_LO], ecx

        ; LAST_IDX (16-bit)
        mov     cx, word [last_idx]
        mov     word [edx + eax + PREF_UNIT_LAST_IDX], cx

        ; CTRL = OP_ON
        mov     dword [edx + eax + PREF_UNIT_CTRL], PREF_UNIT_OP_ON

        ; ---- immediate read-back of the same absolute locations ----
        mov     ecx, [edx + eax + PREF_UNIT_ADDR_LO]
        movzx   edx, word [edx + eax + PREF_UNIT_LAST_IDX]
        DEBUGF  1, "raw after write: ADDR_LO=%x LAST_IDX=%u (abs base=%x)\n", ecx, edx, eax

        ret
endp

;----------------------------------------------------------------------
;  sky2_rx_alloc_one – allocate one new RX buffer and write its LE at
;                      the current rx_put slot (HW_OWNER set, ready for
;                      hardware), advancing rx_put. Does NOT ring the
;                      doorbell (PREF_UNIT_PUT_IDX) - callers batch
;                      that after one or more calls. ebx = device.
;                      Returns eax = 0 on success, -1 if NetAlloc
;                      failed (slot left as it was - a failed refill
;                      after a completed RX just means that slot stays
;                      empty rather than being resubmitted).
;----------------------------------------------------------------------

align 4
sky2_rx_alloc_one:
        push    esi edi

        invoke  NetAlloc, RX_BUF_SIZE + NET_BUFF.data
        test    eax, eax
        jz      .fail

        mov     [eax + NET_BUFF.type], NET_TYPE_ETH

        movzx   ecx, [ebx + device.rx_put]
        mov     [ebx + device.rx_buf + ecx*4], eax

        push    eax
        add     eax, NET_BUFF.data
        invoke  GetPhysAddr
        mov     esi, eax
        pop     eax

        imul    edi, ecx, sizeof.le
        add     edi, ebx
        add     edi, device.rx_le
        mov     [edi + le.addr], esi
        mov     [edi + le.length], RX_BUF_SIZE
        mov     [edi + le.ctrl], 0
        mov     [edi + le.opcode], OP_PACKET or HW_OWNER

        DEBUGF  1, "rx_alloc_one put=%u buf=0x%x phys=0x%x desc=0x%x\n", ecx, eax, esi, edi

        inc     ecx
        and     ecx, RX_LE_SIZE - 1
        mov     [ebx + device.rx_put], cx

        pop     edi esi
        xor     eax, eax
        ret

  .fail:
        pop     edi esi
        or      eax, -1
        ret

;----------------------------------------------------------------------
;  hw_start – bring up the RAM buffer, both BMU queues, both prefetch
;             units, the TX arbiter, and the status ring; fill the RX
;             ring with buffers; unmask the status-BMU interrupt.
;             Called once from reset(), after phy_init.
;----------------------------------------------------------------------

align 4
hw_start:
        ; Linux sky2 tx_init(): seed OP_ADDR64 at slot 0, prod=cons=1.
        ; Do NOT doorbell yet — first transmit() PUT publishes ADDR64
        ; and the first OP_PACKET together (matches Linux/BSD).
        mov     dword [ebx + device.tx_le + le.addr], 0
        mov     word  [ebx + device.tx_le + le.length], 0
        mov     byte  [ebx + device.tx_le + le.ctrl], 0
        mov     byte  [ebx + device.tx_le + le.opcode], OP_PACKET or HW_OWNER
        mov     word  [ebx + device.tx_prod], 0 ;;;1
        mov     word  [ebx + device.tx_cons], 0 ;;;1

        ; ---- RAM buffer sizing ----
        ; Linux: ramsize = B2_E_0 * 4 (KB). EC Ultra / EX / FE+ have no
        ; external RAM; B2_E_0 is typically 0 and the whole block is skipped.
        ; Programming RB_* on a chip with no RAM breaks the GMF path, so
        ; only call sky2_ramset when ramsize is non-zero.
        mov     eax, B2_E_0
        call    sky2_read8
        movzx   esi, al
        shl     esi, 2                              ; esi = ramsize (KB)
        DEBUGF  1, "B2_E_0 ramsize=%u KB\n", esi
        test    esi, esi
        jz      .skip_ramset

        cmp     esi, 16
        jae     .big_ram
        mov     eax, esi
        shr     eax, 1                              ; rxspace = ramsize/2
        jmp     .have_rxspace
  .big_ram:
        mov     eax, esi
        sub     eax, 16
        add     eax, eax
        cdq
        mov     ecx, 3
        idiv    ecx                                 ; (ramsize-16)*2/3
        add     eax, 8
  .have_rxspace:
        mov     edi, eax                            ; edi = rxspace (KB)

        stdcall sky2_ramset, Q_R1, 0, edi
        mov     eax, esi
        sub     eax, edi                            ; txspace = ramsize - rxspace
        stdcall sky2_ramset, Q_XA1, edi, eax

        ; make sure the (unused) sync-TX queue stays disabled
        mov     ecx, Q_XS1
        mov     eax, RB_CTRL
        mov     edx, RB_RST_SET
        call    sky2_rb_write8

  .skip_ramset:

        ; ---- test whether PREF registers are writable at all ----
        mov     edx, [ebx + device.mmio_addr]
        mov     eax, Y2_B8_PREF_REGS + Q_XA1          ; 0x06D0

        mov     dword [edx + eax + 0x08], 0xDEADBEEF  ; ADDR_LO
        mov     word  [edx + eax + 0x04], 0x1234      ; LAST_IDX

        mov     ecx, [edx + eax + 0x08]
        movzx   esi, word [edx + eax + 0x04]
        DEBUGF  1, "PREF write test: wrote DEADBEEF/1234, read %x / %x\n", ecx, esi

        ; ---- TX queue + prefetch unit ----
        stdcall sky2_qset, Q_XA1

        ; Linux sky2_up() TX quirks (do NOT conflate these):
        ;   Q_AL = ECU_TXFF_LEV (0x1a0)  — only EC Ultra A0
        ;   F_TX_CHK_AUTO_OFF            — only Yukon EX B0 (TX checksum
        ;                                  auto-calc, NOT descriptor CHK IRQ)
        ; Our chip is EC Ultra B0 (id=0xB4, rev=3): neither applies.
        movzx   eax, [ebx + device.chip_id]
        cmp     eax, CHIP_ID_YUKON_EC_U
        jne     .tx_pref
        movzx   eax, [ebx + device.chip_rev]
        cmp     eax, CHIP_REV_YU_EC_U_A0
        jne     .tx_pref
        mov     ecx, Q_XA1
        mov     eax, Q_AL
        mov     edx, 0x1a0                    ; ECU_TXFF_LEV
        call    sky2_q_write16
  .tx_pref:

        lea     eax, [ebx + device.tx_le]
        invoke  GetPhysAddr
        stdcall sky2_prefetch_init, Q_XA1, eax, TX_LE_SIZE - 1
        ; PUT_IDX left at 0 until first transmit() — Linux behaviour.

        ; TX Arbiter (Linux sky2 / OpenBSD msk / skge). ITI/LIM are
        ; 32-bit registers; writing them as 8-bit left the high bytes
        ; undefined. Full control: enable arbiter + force-sync + alloc.
        mov     eax, TXA_ITI_INI
        xor     ecx, ecx
        call    sky2_sk_write32
        mov     eax, TXA_LIM_INI
        mov     ecx, 0x00ffffff
        call    sky2_sk_write32
        mov     eax, TXA_CTRL
        mov     cl, TXA_ENA_ARB or TXA_ENA_FSYNC or TXA_ENA_ALLOC or TXA_START_RC
        call    sky2_sk_write8

        ; ---- RX queue + prefetch unit ----
        stdcall sky2_qset, Q_R1

        ; EC-Ultra silicon newer than rev A0 (ours is rev 3) bypasses the
        ; RAM buffer for RX reads - hardware controls it directly. See
        ; Linux sky2_rx_start(); confirmed our board's chip_rev via the
        ; earlier "chip revision = 3" field log.
        movzx   eax, [ebx + device.chip_id]
        cmp     eax, CHIP_ID_YUKON_EC_U
        jne     @f
        movzx   eax, [ebx + device.chip_rev]
        cmp     eax, CHIP_REV_YU_EC_U_A0
        jbe     @f
        mov     ecx, Q_R1
        mov     eax, Q_TEST
        mov     edx, F_M_RX_RAM_DIS
        call    sky2_q_write32
  @@:

        lea     eax, [ebx + device.rx_le]
        invoke  GetPhysAddr
        stdcall sky2_prefetch_init, Q_R1, eax, RX_LE_SIZE - 1

        ; fill the RX ring
        mov     word [ebx + device.rx_put], 0
        mov     esi, RX_LE_SIZE - 1
  .rx_fill:
        call    sky2_rx_alloc_one
        test    eax, eax
        jz      @f
        DEBUGF  2, "RX buffer alloc failed during ring fill\n"
  @@:
        dec     esi
        jnz     .rx_fill

        movzx   eax, [ebx + device.rx_put]
        stdcall sky2_pref_write16_ext, Q_R1, PREF_UNIT_PUT_IDX, eax

        ; ---- status ring ----
        mov     eax, STAT_CTRL
        mov     ecx, SC_STAT_RST_SET
        call    sky2_write32
        mov     eax, STAT_CTRL
        mov     ecx, SC_STAT_RST_CLR
        call    sky2_write32

        lea     eax, [ebx + device.st_le]
        invoke  GetPhysAddr
        mov     ecx, eax
        mov     eax, STAT_LIST_ADDR_LO
        call    sky2_write32
        mov     eax, STAT_LIST_ADDR_HI
        xor     ecx, ecx
        call    sky2_write32

        mov     eax, STAT_LAST_IDX
        mov     cx, ST_RING_SIZE - 1
        call    sky2_write16

        mov     eax, STAT_TX_IDX_TH
        mov     cx, 10
        call    sky2_write16

        mov     eax, STAT_FIFO_WM
        mov     cl, 16
        call    sky2_write8

        mov     eax, STAT_CTRL
        mov     ecx, SC_STAT_OP_ON
        call    sky2_write32

        mov     word [ebx + device.st_idx], 0

        mov     eax, STAT_TX_TIMER_INI
        mov     ecx, 125*1000        ; ~1ms
        call    sky2_write32
        mov     eax, STAT_ISR_TIMER_INI
        mov     ecx, 125*20          ; ~20µs
        call    sky2_write32
        mov     eax, STAT_LEV_TIMER_INI
        mov     ecx, 125*100
        call    sky2_write32

        mov     eax, STAT_FIFO_ISR_WM
        mov     cl, 16
        call    sky2_write8

        mov     eax, STAT_TX_TIMER_CTRL
        mov     cl, TIM_START
        call    sky2_write8
        mov     eax, STAT_LEV_TIMER_CTRL
        mov     cl, TIM_START
        call    sky2_write8
        mov     eax, STAT_ISR_TIMER_CTRL
        mov     cl, TIM_START
        call    sky2_write8

        ; ---- unmask PHY-link and status-BMU interrupts together ----
        mov     eax, B0_IMSK
        mov     ecx, Y2_IS_IRQ_PHY1 or Y2_IS_STAT_BMU
        call    sky2_write32

        ret

;----------------------------------------------------------------------
;  sky2_status_intr – walk the status ring for as long as entries have
;                      HW_OWNER set, dispatching RX completions to
;                      EthInput and TX completions to buffer reclaim.
;                      ebx = device.
;
;  EthInput has a non-standard calling convention: it's reached via
;  jmp (not call) with the return address and the single argument
;  pushed in swapped order (arg on top), and it may clobber every
;  register on the way back - so anything we need afterward (here,
;  just ebx) has to be saved on the stack ourselves, underneath the
;  return address, and restored the moment control comes back. This
;  matches the pattern already used for the same reason in RTL8169.asm.
;----------------------------------------------------------------------

align 4
sky2_status_intr:
        push    esi edi

  .next:
        movzx   ecx, [ebx + device.st_idx]
        imul    edi, ecx, sizeof.le
        add     edi, ebx
        add     edi, device.st_le

        movzx   eax, byte [edi + le.opcode]
        test    al, HW_OWNER
        jz      .done

        mov     byte [edi + le.opcode], 0            ; ack: release this slot

        inc     ecx
        and     ecx, ST_RING_SIZE - 1
        mov     [ebx + device.st_idx], cx

        and     al, not HW_OWNER
        cmp     al, OP_RXSTAT
        je      .do_rx
        cmp     al, OP_TXINDEXLE
        je      .do_tx
        jmp     .next                                ; unhandled opcode, skip

  .do_tx:
        mov     eax, [edi + le.addr]                  ; TX index, low 12 bits
        and     eax, 0xFFF
        push    eax
        DEBUGF  1, "TX complete, idx=%u\n", eax
        pop     eax
        call    sky2_tx_reclaim
        jmp     .next

  .do_rx:
        mov     edx, [edi + le.addr]                  ; status dword (GMR_FS_*)
        movzx   ecx, word [edi + le.length]            ; frame length
        push    edx ecx
        DEBUGF  1, "RX complete, len=%u, status=%x\n", ecx, edx
        pop     ecx edx

        movzx   eax, [ebx + device.rx_next]
        push    eax                                    ; slot index, for later
        mov     eax, [ebx + device.rx_buf + eax*4]
        pop     esi
        mov     dword [ebx + device.rx_buf + esi*4], 0

        inc     esi
        and     esi, RX_LE_SIZE - 1
        mov     [ebx + device.rx_next], si

        test    eax, eax
        jz      .rx_refill                            ; slot was already empty

        test    edx, GMR_FS_ANY_ERR
        jnz     .rx_drop
        test    edx, GMR_FS_RX_OK
        jz      .rx_drop

        mov     [eax + NET_BUFF.length], ecx
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data

        add     dword [ebx + device.bytes_rx], ecx
        adc     dword [ebx + device.bytes_rx + 4], 0
        inc     dword [ebx + device.packets_rx]

        ; refill the ring BEFORE handing off, so a failed refill can't
        ; be blamed on (or confused with) the packet we're delivering
        push    eax                                    ; the NET_BUFF ptr, survives - stack, not a register
        call    sky2_rx_alloc_one
        test    eax, eax
        jnz     @f
        movzx   eax, [ebx + device.rx_put]
        stdcall sky2_pref_write16_ext, Q_R1, PREF_UNIT_PUT_IDX, eax
  @@:
        pop     eax                                    ; the NET_BUFF ptr back

        push    ebx
        push    .eth_input_return
        push    eax
        jmp     [EthInput]

  .eth_input_return:
        pop     ebx
        jmp     .next

  .rx_drop:
        inc     dword [ebx + device.packets_rx_err]
        invoke  NetFree, eax

  .rx_refill:
        call    sky2_rx_alloc_one
        test    eax, eax
        jnz     .next
        movzx   eax, [ebx + device.rx_put]
        stdcall sky2_pref_write16_ext, Q_R1, PREF_UNIT_PUT_IDX, eax
        jmp     .next

  .done:
        mov     eax, STAT_CTRL
        mov     ecx, SC_STAT_CLR_IRQ
        call    sky2_write32

        pop     edi esi
        ret

;----------------------------------------------------------------------
;  sky2_tx_reclaim – free every TX buffer from tx_cons up to (but not
;                     including) the index hardware just reported done.
;                     ebx = device, eax = done index.
;----------------------------------------------------------------------

align 4
sky2_tx_reclaim:
        push    esi edi
        mov     edi, eax                            ; edi = done index

  .loop:
        movzx   ecx, [ebx + device.tx_cons]
        cmp     ecx, edi
        je      .finish

        mov     eax, [ebx + device.tx_buf + ecx*4]
        mov     [ebx + device.tx_buf + ecx*4], 0

        inc     ecx
        and     ecx, TX_LE_SIZE - 1
        mov     [ebx + device.tx_cons], cx

        test    eax, eax
        jz      .loop                                ; placeholder LE, no buffer

        inc     dword [ebx + device.packets_tx]
        invoke  NetFree, eax
        jmp     .loop

  .finish:
        pop     edi esi
        ret

; ebx=device, stdcall: queue, offset, value - same as sky2_pref_write16
; but with offset as an argument too, needed because PREF_UNIT_PUT_IDX
; is written from more than one call site above with the same pattern
align 4
proc sky2_pref_write16_ext stdcall, queue:dword, offset:dword, value:dword
        mov     ecx, [queue]
        mov     eax, [offset]
        mov     edx, [value]
        call    sky2_pref_write16
        ret
endp

;----------------------------------------------------------------------
;  transmit – hand one outgoing packet to the TX ring. Single LE per
;             packet (no scatter-gather / TSO / VLAN / checksum
;             offload - see the ring comment in sky2.inc for the full
;             list of what's deferred). Standard stdcall ETH_DEVICE
;             signature; ebx = device is set by the kernel before the
;             call, same convention as reset/probe/unload.
;----------------------------------------------------------------------

align 4
proc transmit stdcall, bufferptr:dword
        push    esi edi

        mov     esi, [bufferptr]
        mov     ecx, [esi + NET_BUFF.length]

        push    esi ecx
        DEBUGF  1, "transmit() len=%u\n", ecx
        pop     ecx esi

        cmp     ecx, 1514
        ja      .drop
        cmp     ecx, 60
        jb      .drop

        ; Protects tx_prod/tx_cons/tx_buf/tx_le against a concurrent
        ; sky2_tx_reclaim() from int_handler. KolibriOS doesn't run
        ; driver/kernel code on more than one core yet, so plain
        ; cli/sti (what this macro actually is) is sufficient here -
        ; it wouldn't be enough on a real SMP kernel, but that's not
        ; what this is.
        spin_lock_irqsave

        ; is there a free slot? tx_prod must not catch up to tx_cons
        movzx   eax, [ebx + device.tx_prod]
        mov     edx, eax
        inc     edx
        and     edx, TX_LE_SIZE - 1
        movzx   edi, [ebx + device.tx_cons]
        cmp     edx, edi
        je      .overrun

        mov     [ebx + device.tx_buf + eax*4], esi

        push    eax
        mov     eax, esi
        add     eax, [esi + NET_BUFF.offset]
        invoke  GetPhysAddr
        mov     edx, eax
        pop     eax

        imul    edi, eax, sizeof.le
        push    eax
        lea     edi, [ebx + device.tx_le + edi]
        mov     [edi + le.addr], edx
        mov     word [edi + le.length], cx
        mov     byte [edi + le.ctrl], LE_EOP
        mov     byte [edi + le.opcode], OP_PACKET or HW_OWNER
        pop     eax

        inc     eax
        and     eax, TX_LE_SIZE - 1
        mov     [ebx + device.tx_prod], ax

        add     dword [ebx + device.bytes_tx], ecx
        adc     dword [ebx + device.bytes_tx + 4], 0

        ; Linux: only write PREF_UNIT_PUT_IDX. BMU already started in qset.
        stdcall sky2_pref_write16_ext, Q_XA1, PREF_UNIT_PUT_IDX, eax

        spin_unlock_irqrestore

        DEBUGF  1, "transmit() queued ok\n"

        pop     edi esi
        xor     eax, eax
        ret

  .overrun:
        spin_unlock_irqrestore
        DEBUGF  2, "transmit() TX ring full, dropping packet\n"
        inc     dword [ebx + device.packets_tx_drop]
        invoke  NetFree, esi
        pop     edi esi
        or      eax, -1
        ret

  .drop:
        DEBUGF  2, "transmit() bad packet length %u, dropping\n", ecx
        inc     dword [ebx + device.packets_tx_err]
        invoke  NetFree, esi
        pop     edi esi
        or      eax, -1
        ret
endp

;----------------------------------------------------------------------
;  unload – free every outstanding NetAlloc'd RX/TX buffer before the
;           driver goes away.
;----------------------------------------------------------------------

align 4
unload:
        push    esi

        mov     esi, RX_LE_SIZE
        xor     ecx, ecx
  .free_rx:
        mov     eax, [ebx + device.rx_buf + ecx*4]
        test    eax, eax
        jz      @f
        invoke  NetFree, eax
  @@:
        inc     ecx
        dec     esi
        jnz     .free_rx

        mov     esi, TX_LE_SIZE
        xor     ecx, ecx
  .free_tx:
        mov     eax, [ebx + device.tx_buf + ecx*4]
        test    eax, eax
        jz      @f
        invoke  NetFree, eax
  @@:
        inc     ecx
        dec     esi
        jnz     .free_tx

        pop     esi
        DEBUGF  1, "unload()\n"
        xor     eax, eax
        ret

;----------------------------------------------------------------------
;  Data
;----------------------------------------------------------------------

data fixups
end data

include '../peimport.inc'

include_debug_strings

my_service      db 'MARVYUK2',0

name_ec_u       db 'Marvell Yukon-2 EC Ultra',0
name_ec         db 'Marvell Yukon-2 EC',0
name_xl         db 'Marvell Yukon-2 XL',0
name_ex         db 'Marvell Yukon-2 Extreme',0
name_fe         db 'Marvell Yukon-2 FE',0
name_fe_p       db 'Marvell Yukon-2 FE+',0
name_supr       db 'Marvell Yukon-2 Supreme',0
name_ul2        db 'Marvell Yukon-2 Ultra 2',0
name_yukon2     db 'Marvell Yukon-2',0
name_unknown    db 'unknown Yukon-2',0

str_full        db 'full',0
str_half        db 'half',0

; Local device list
align 4
devices         dd 0
device_list     rd MAX_DEVICES
