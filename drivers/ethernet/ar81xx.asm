;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2018-2021. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  AR81XX driver for KolibriOS                                    ;;
;;                                                                 ;;
;;  based on alx driver from TI-OpenLink                           ;;
;;                                                                 ;;
;;  Written by hidnplayr (hidnplayr@gmail.com)                     ;;
;;                                                                 ;;
;;  Thanks to: floppy121 for kindly providing me with the hardware ;;
;;              that made the development of this driver possible. ;;
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

; configureable area

        MAX_DEVICES             = 16    ; Maximum number of devices this driver may handle

        __DEBUG__               = 1     ; 1 = on, 0 = off
        __DEBUG_LEVEL__         = 2     ; 1 = verbose, 2 = errors only

        TX_RING_SIZE            = 128   ; Number of packets in send ring buffer
        RX_RING_SIZE            = 128   ; Number of packets in receive ring buffer

        RX_BUFFER_SIZE          = 1536

        SMB_TIMER               = 400
        IMT                     = 200                   ; IRQ Modulo Timer
        ITH_TPD                 = TX_RING_SIZE / 3      ; Interrupt Threshold TPD

; end configureable area

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'

include 'ar81xx.inc'

if (bsr TX_RING_SIZE)>(bsf TX_RING_SIZE)
  display 'TX_RING_SIZE must be a power of two'
  err
end if

if (bsr RX_RING_SIZE)>(bsf RX_RING_SIZE)
  display 'RX_RING_SIZE must be a power of two'
  err
end if

; Transmit Packet Descriptor
struct alx_tpd
        length          dw ?
        vlan_tag        dw ?
        word1           dd ?
        addr_l          dd ?
        addr_h          dd ?
ends

; Receive Return Descriptor
struct alx_rrd
        word0           dd ?    ; IP payload cksum + number of RFDs + start index of RFD-ring
        rss_hash        dd ?
        word2           dd ?    ; VLAN tag + Protocol ID + RSS Q num + RSS Hash algorithm
        word3           dd ?    ; Packet length + status
ends

; Receive Free Descriptor
struct alx_rfd
        addr_l          dd ?
        addr_h          dd ?
ends

struct  device          ETH_DEVICE

        io_addr         dd ?
        pci_bus         dd ?
        pci_dev         dd ?
        pci_vid         dw ?    ; Vendor ID
        pci_did         dw ?    ; Device ID
        irq_line        dd ?
        pci_rev         dd ?
        chip_rev        dd ?
        mmio_addr       dd ?

        max_dma_chnl    dd ?

        int_mask        dd ?
        rx_ctrl         dd ?

        rxq_read_idx            dd ?
        rxq_write_idx           dd ?
;        rxq_rrd_read_idx        dd ?
        txq_read_idx            dd ?
        txq_write_idx           dd ?

        rb 0x100 - ($ and 0xff) ; align 256
        tpd_ring        rb ((TX_RING_SIZE*sizeof.alx_tpd+16) and 0xfffffff0)
        rrd_ring        rb ((RX_RING_SIZE*sizeof.alx_rrd+16) and 0xfffffff0)
        rfd_ring        rb ((RX_RING_SIZE*sizeof.alx_rfd+16) and 0xfffffff0)
        tpd_ring_virt   rd TX_RING_SIZE
        rfd_ring_virt   rd RX_RING_SIZE

ends

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;; (standard driver proc) ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START c, reason:dword, cmdline:dword

        cmp     [reason], DRV_ENTRY
        jne     .fail

        DEBUGF  2,"Loading driver\n"
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

proc service_proc stdcall, ioctl:dword

        mov     edx, [ioctl]
        mov     eax, [edx + IOCTL.io_code]

;------------------------------------------------------

        cmp     eax, 0 ;SRV_GETVERSION
        jne     @F

        cmp     [edx + IOCTL.out_size], 4
        jb      .fail
        mov     eax, [edx + IOCTL.output]
        mov     [eax], dword API_VERSION

        xor     eax, eax
        ret

;------------------------------------------------------
  @@:
        cmp     eax, 1 ;SRV_HOOK
        jne     .fail

        cmp     [edx + IOCTL.inp_size], 3               ; Data input must be at least 3 bytes
        jb      .fail

        mov     eax, [edx + IOCTL.input]
        cmp     byte [eax], 1                           ; 1 means device number and bus number (pci) are given
        jne     .fail                                   ; other types arent supported for this card yet

; check if the device is already listed

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

;        mov     eax, [edx + IOCTL.input]               ; get the pci bus and device numbers
        mov     ax , [eax+1]                            ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte[ebx + device.pci_bus]
        jne     @f
        cmp     ah, byte[ebx + device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
       @@:
        add     esi, 4
        loop    .nextdevice

; This device doesnt have its own eth_device structure yet, lets create one
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

; Now, it's time to find the base mmio addres of the PCI device

        stdcall PCI_find_mmio, [ebx + device.pci_bus], [ebx + device.pci_dev] ; returns in eax
        test    eax, eax
        jz      .destroy

; Create virtual mapping of the physical memory

        invoke  MapIoMem, eax, 10000h, PG_SW+PG_NOCACHE
        mov     [ebx + device.mmio_addr], eax

; We've found the mmio address, find IRQ now
        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.interrupt_line
        and     eax, 0xff
        mov     [ebx + device.irq_line], eax

        DEBUGF  1,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [ebx + device.pci_dev]:1, [ebx + device.pci_bus]:1, [ebx + device.irq_line]:1, [ebx + device.mmio_addr]:8

; Ok, the eth_device structure is ready, let's probe the device

        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err2

        DEBUGF  2,"Initialised OK\n"

        mov     [ebx + device.type], NET_TYPE_ETH
        invoke  NetRegDev

        cmp     eax, -1
        je      .destroy

        ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  2,"Trying to find device number of already registered device\n"
        invoke  NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  2,"Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        ; todo: reset device into virgin state

  .err2:
        dec     [devices]
  .err:
        invoke  KernelFree, ebx
  .fail:
        DEBUGF  2, "Failed to load\n"
        or      eax, -1
        ret

;------------------------------------------------------
endp

;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;

align 4
unload:
        ; TODO: (in this particular order)
        ;
        ; - Stop the device
        ; - Detach int handler
        ; - Remove device from local list (device_list)
        ; - call unregister function in kernel
        ; - Remove all allocated structures and buffers the card used

        or      eax, -1
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  probe: enables the device (if it really is AR81XX)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:
        DEBUGF  1,"Probing\n"

; Make the device a bus master
        invoke  PciRead16, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header.command
        or      al, PCI_CMD_MASTER + PCI_CMD_MMIO + PCI_CMD_PIO
        and     ax, not(PCI_CMD_INTX_DISABLE)
        invoke  PciWrite16, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header.command, eax

; get device ID
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header.vendor_id
        mov     dword[ebx + device.pci_vid], eax

        mov     esi, chiplist
  .loop:
        cmp     dword[esi], eax
        je      .got_it
        add     esi, 8
        cmp     dword[esi], 0
        jne     .loop
        DEBUGF  2, "Unknown chip: 0x%x, continuing anyway\n", eax
        mov     [ebx + device.name], my_service
        jmp     .done
  .got_it:
        mov     eax, dword[esi+4]
        mov     [ebx + device.name], eax
        DEBUGF  1, "Chip type = %s\n", eax
  .done:

; get revision ID
        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header.revision_id
        and     eax, 0xff
        DEBUGF  1,"PCI Revision: %u\n", eax
        mov     [ebx + device.pci_rev], eax
        shr     al, ALX_PCI_REVID_SHIFT
        mov     [ebx + device.chip_rev], eax
        DEBUGF  1,"ALX Revision: %u\n", eax

        stdcall alx_reset_pcie

        mov     ecx, (ALX_PMCTRL_L0S_EN or ALX_PMCTRL_L1_EN or ALX_PMCTRL_ASPM_FCEN)
        stdcall alx_enable_aspm

        stdcall alx_reset_phy

        stdcall alx_reset_mac

; Setup link to put it in a known good starting state
        stdcall alx_write_phy_reg, ALX_MII_DBG_ADDR, 0

        mov     esi, [ebx + device.mmio_addr]
        mov     eax, dword[esi + ALX_DRV]

        stdcall alx_write_phy_reg, MII_ADVERTISE, ADVERTISE_CSMA or ADVERTISE_10HALF or ADVERTISE_10FULL or ADVERTISE_100HALF or ADVERTISE_100FULL or ADVERTISE_PAUSE_CAP

        xor     eax, eax
        test    [ebx + device.pci_did], 1       ;;; FIXME: is gigabit device?
        jz      @f
        mov     eax, ADVERTISE_1000XFULL
  @@:
        stdcall alx_write_phy_reg, MII_CTRL1000, eax

        stdcall alx_write_phy_reg, MII_BMCR, BMCR_RESET or BMCR_ANENABLE or BMCR_ANRESTART

        stdcall alx_get_perm_macaddr

align 4
reset:

        DEBUGF  1,"Resetting\n"

; alx init_sw

        stdcall alx_identify_hw

        mov     [ebx + device.int_mask], ALX_ISR_MISC
        mov     [ebx + device.rx_ctrl], ALX_MAC_CTRL_WOLSPED_SWEN or ALX_MAC_CTRL_MHASH_ALG_HI5B or ALX_MAC_CTRL_BRD_EN or ALX_MAC_CTRL_PCRCE or ALX_MAC_CTRL_CRCE or ALX_MAC_CTRL_RXFC_EN or ALX_MAC_CTRL_TXFC_EN or (7 shl ALX_MAC_CTRL_PRMBLEN_SHIFT)

        stdcall alx_alloc_rings

        stdcall alx_configure

        stdcall alx_request_irq

; attach interrupt handler

        mov     eax, [ebx + device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n", eax:1
        invoke  AttachIntHandler, eax, int_handler, ebx
        test    eax, eax
        jnz     @f
        DEBUGF  2,"Could not attach int handler!\n"
        or      eax, -1
        ret
  @@:

; Clear old interrupts
        mov     edi, [ebx + device.mmio_addr]
        mov     eax, not ALX_ISR_DIS
        mov     [edi + ALX_ISR], eax

        stdcall alx_irq_enable

; Set the MTU, kernel will be able to send now
        mov     [ebx + device.mtu], 1514

        stdcall alx_check_link

        DEBUGF  1,"Reset ok\n"
        xor     eax, eax
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: pointer to device structure in ebx  ;;
;; Out: eax = 0 on success                 ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 16
proc transmit stdcall bufferptr

        spin_lock_irqsave

        mov     esi, [bufferptr]
        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n", [bufferptr], [esi + NET_BUFF.length]
        lea     eax, [esi + NET_BUFF.data]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     [esi + NET_BUFF.length], 1514
        ja      .error
        cmp     [esi + NET_BUFF.length], 60
        jb      .error

; Program the descriptor
        mov     edi, [ebx + device.txq_write_idx]
        DEBUGF  1, "Using TPD: %u\n", edi
        cmp     dword[ebx + device.tpd_ring_virt + edi*4], 0
        jne     .overrun
        mov     dword[ebx + device.tpd_ring_virt + edi*4], esi
        shl     edi, 4
        lea     edi, [ebx + device.tpd_ring + edi]
        mov     eax, esi
        add     eax, [esi + NET_BUFF.offset]
        invoke  GetPhysAddr
        mov     [edi + alx_tpd.addr_l], eax
        mov     [edi + alx_tpd.addr_h], 0

        mov     ecx, [esi + NET_BUFF.length]
        mov     [edi + alx_tpd.length], cx

        mov     [edi + alx_tpd.word1], 1 shl TPD_EOP_SHIFT

; Update Producer Index
        mov     eax, [ebx + device.txq_write_idx]
        inc     eax
        and     eax, TX_RING_SIZE - 1
        mov     [ebx + device.txq_write_idx], eax

        mov     edi, [ebx + device.mmio_addr]
        mov     word[edi + ALX_TPD_PRI0_PIDX], ax

; Update stats
        inc     [ebx + device.packets_tx]
        mov     eax, [esi + NET_BUFF.length]
        add     dword[ebx + device.bytes_tx], eax
        adc     dword[ebx + device.bytes_tx + 4], 0

        spin_unlock_irqrestore
        xor     eax, eax
        ret

  .error:
        DEBUGF  2, "TX packet error\n"
        inc     [ebx + device.packets_tx_err]
        invoke  NetFree, [bufferptr]

        spin_unlock_irqrestore
        or      eax, -1
        ret

  .overrun:
        DEBUGF  2, "TX overrun\n"
        inc     [ebx + device.packets_tx_ovr]
        invoke  NetFree, [bufferptr]

        spin_unlock_irqrestore
        or      eax, -1
        ret

endp

;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;
align 16
int_handler:

        push    ebx esi edi

        mov     ebx, [esp+4*4]
        DEBUGF  1,"INT for 0x%x\n", ebx

; TODO? if we are paranoid, we can check that the value from ebx is present in the current device_list

        mov     edi, [ebx + device.mmio_addr]
        mov     eax, [edi + ALX_ISR]
        test    eax, eax
        jz      .nothing

        or      eax, ALX_ISR_DIS
        mov     [edi + ALX_ISR], eax    ; ACK interrupt
        DEBUGF  1,"Status: %x\n", eax

        test    eax, ALX_ISR_TX_Q0
        jz      .no_tx
        DEBUGF  1,"TX interrupt\n"
        pusha
        stdcall alx_clean_tx_irq
        popa
  .no_tx:

        test    eax, ALX_ISR_RX_Q0
        jz      .no_rx
        DEBUGF  1,"RX interrupt\n"
        pusha
        stdcall alx_clean_rx_irq
        popa

  .no_rx:
        test    eax, ALX_ISR_PHY
        jz      .no_phy
        DEBUGF  1,"PHY interrupt\n"
        pusha
; TODO: queue link check and disable this interrupt cause meanwhile??
        stdcall alx_check_link
        popa

  .no_phy:
        mov     dword[edi + ALX_ISR], 0
        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret

  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret

proc alx_identify_hw stdcall

        cmp     [ebx + device.pci_did], ALX_DEV_ID_AR8131
        je      .alc

        cmp     [ebx + device.chip_rev], ALX_REV_C0
        ja      .einval

        mov     [ebx + device.max_dma_chnl], 2
        cmp     [ebx + device.chip_rev], ALX_REV_B0
        jb      @f
        mov     [ebx + device.max_dma_chnl], 4
  @@:
        xor     eax, eax
        ret

  .einval:
        DEBUGF  1, "Invalid revision 0x%x\n", [ebx + device.chip_rev]

        xor     eax, eax
        dec     eax
        ret

  .alc:
        mov     [ebx + device.max_dma_chnl], 2
        xor     eax, eax
        ret

endp

proc udelay stdcall microseconds

; FIXME

        push    esi ecx edx
        xor     esi, esi
        inc     esi
        invoke  Sleep
        pop     edx ecx esi

        ret

endp

proc alx_reset_pcie stdcall

        DEBUGF  1,"alx_reset_pcie\n"

; Make the device a bus master
        invoke  PciRead16, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER or PCI_CMD_MMIO or PCI_CMD_PIO
        and     ax, not(PCI_CMD_INTX_DISABLE)
        invoke  PciWrite16, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

; Clear any powersaving setting
        invoke  PciWrite16, [ebx + device.pci_bus], [ebx + device.pci_dev], 0x44, 0x0000        ;; FIXME

; Mask some pcie error bits
        mov     esi, [ebx + device.mmio_addr]
        mov     eax, [esi + ALX_UE_SVRT]
        and     eax, not(ALX_UE_SVRT_DLPROTERR or ALX_UE_SVRT_FCPROTERR)
        mov     [esi + ALX_UE_SVRT], eax

; pclk
        mov     eax, [esi + ALX_MASTER]
        or      eax, ALX_MASTER_WAKEN_25M
        and     eax, not (ALX_MASTER_PCLKSEL_SRDS)
        cmp     [ebx + device.chip_rev], ALX_REV_A1
        ja      @f
        test    [ebx + device.pci_rev], ALX_PCI_REVID_WITH_CR
        jz      @f
        or      eax, ALX_MASTER_PCLKSEL_SRDS
  @@:
        mov     [esi + ALX_MASTER], eax

        xor     eax, eax
        ret

endp

proc alx_clean_tx_irq stdcall

        mov     eax, [ebx + device.txq_read_idx]
        movzx   ecx, word[edi + ALX_TPD_PRI0_CIDX]

  .loop:
        cmp     eax, ecx
        je      .done

        DEBUGF  1,"Cleaning TX desc %u buffer 0x%x\n", eax, [ebx + device.tpd_ring_virt + eax*4]
        push    eax ecx
        invoke  NetFree, [ebx + device.tpd_ring_virt + eax*4]
        pop     ecx eax
        mov     [ebx + device.tpd_ring_virt + eax*4], 0

        inc     eax
        and     eax, TX_RING_SIZE-1
        jmp     .loop
  .done:
        mov     [ebx + device.txq_read_idx], eax

        ret

endp

proc alx_clean_rx_irq stdcall

        mov     ecx, [ebx + device.rxq_read_idx]
  .loop:
        shl     ecx, 2
        lea     esi, [ebx + device.rrd_ring + ecx*4]
        test    [esi + alx_rrd.word3], 1 shl RRD_UPDATED_SHIFT
        jz      .done
        and     [esi + alx_rrd.word3], not(1 shl RRD_UPDATED_SHIFT)
        DEBUGF  1,"RRD=%u updated\n", [ebx + device.rxq_read_idx]

        mov     eax, [esi + alx_rrd.word0]
        shr     eax, RRD_SI_SHIFT
        and     eax, RRD_SI_MASK
        cmp     eax, [ebx + device.rxq_read_idx]
;        jne     .error
        DEBUGF  1,"RFD=%u\n", eax

        mov     eax, [esi + alx_rrd.word0]
        shr     eax, RRD_NOR_SHIFT
        and     eax, RRD_NOR_MASK
        cmp     eax, 1
;        jne     .error

        mov     eax, [esi + alx_rrd.word3]
;        shr     eax, RRD_PKTLEN_SHIFT
        and     eax, RRD_PKTLEN_MASK
        sub     eax, 4                          ;;;;;
        mov     edx, [ebx + device.rfd_ring_virt + ecx]
        DEBUGF  1,"Received %u bytes in buffer 0x%x\n", eax, edx

        mov     [edx + NET_BUFF.length], eax
        mov     [edx + NET_BUFF.device], ebx
        mov     [edx + NET_BUFF.offset], NET_BUFF.data

; Update stats
        add     dword[ebx + device.bytes_rx], eax
        adc     dword[ebx + device.bytes_rx + 4], 0
        inc     [ebx + device.packets_rx]

; Allocate new descriptor
        push    esi ecx edx
        invoke  NetAlloc, RX_BUFFER_SIZE+NET_BUFF.data
        pop     edx ecx esi
        test    eax, eax
        jz      .rx_overrun
        mov     [ebx + device.rfd_ring_virt + ecx], eax
        add     eax, NET_BUFF.data
        invoke  GetPhysAddr
        mov     dword[ebx + device.rfd_ring + ecx*2], eax

        push    ecx ebx
        push    .retaddr
        push    edx
        jmp     [EthInput]
  .retaddr:
        pop     ebx ecx

        shr     ecx, 2
        inc     ecx
        and     ecx, RX_RING_SIZE-1
        jmp     .loop

  .rx_overrun:
        DEBUGF  2,"RX FIFO overrun\n"
        inc     [ebx + device.packets_rx_ovr]
        shr     ecx, 2
        inc     ecx
        and     ecx, RX_RING_SIZE-1
        jmp     .loop

  .done:
        shr     ecx, 2
        mov     [ebx + device.rxq_read_idx], ecx

; Update producer index
        mov     esi, [ebx + device.mmio_addr]
        mov     [esi + ALX_RFD_PIDX], cx

        ret

endp

; IN: ecx = additional bit flags (ALX_PMCTRL_L0S_EN, ALX_PMCTRL_L1_EN, ALX_PMCTRL_ASPM_FCEN)
proc alx_enable_aspm stdcall

        DEBUGF  1,"alx_enable_aspm (0x%x)\n", ecx

        mov     esi, [ebx + device.mmio_addr]

        cmp     [ebx + device.pci_did], ALX_DEV_ID_AR8131
        je      .alc_l1c

        cmp     [ebx + device.chip_rev], ALX_REV_A1
        ja      @f
        test    [ebx + device.pci_rev], ALX_PCI_REVID_WITH_CR
        jz      @f
        or      ecx, ALX_PMCTRL_L1_SRDS_EN or ALX_PMCTRL_L1_SRDSPLL_EN
  @@:

        mov     eax, dword[esi + ALX_PMCTRL]
        and     eax, not((ALX_PMCTRL_LCKDET_TIMER_MASK shl ALX_PMCTRL_LCKDET_TIMER_SHIFT) or \
                         (ALX_PMCTRL_L1REQ_TO_MASK shl ALX_PMCTRL_L1REQ_TO_SHIFT) or \
                         (ALX_PMCTRL_L1_TIMER_MASK shl ALX_PMCTRL_L1_TIMER_SHIFT) or \
                         ALX_PMCTRL_L1_SRDS_EN or \
                         ALX_PMCTRL_L1_SRDSPLL_EN or \
                         ALX_PMCTRL_L1_BUFSRX_EN or \
                         ALX_PMCTRL_SADLY_EN or \
                         ALX_PMCTRL_HOTRST_WTEN or \
                         ALX_PMCTRL_L0S_EN or \
                         ALX_PMCTRL_L1_EN or \
                         ALX_PMCTRL_ASPM_FCEN or \
                         ALX_PMCTRL_TXL1_AFTER_L0S or \
                         ALX_PMCTRL_RXL1_AFTER_L0S)
        or      eax, (ALX_PMCTRL_LCKDET_TIMER_DEF shl ALX_PMCTRL_LCKDET_TIMER_SHIFT) or \
                     (ALX_PMCTRL_RCVR_WT_1US or ALX_PMCTRL_L1_CLKSW_EN or ALX_PMCTRL_L1_SRDSRX_PWD) or \
                     (ALX_PMCTRL_L1REG_TO_DEF shl ALX_PMCTRL_L1REQ_TO_SHIFT) or \
                     (ALX_PMCTRL_L1_TIMER_16US shl ALX_PMCTRL_L1_TIMER_SHIFT)
        or      eax, ecx
        mov     dword[esi + ALX_PMCTRL], eax

        ret

  .alc_l1c:

        DEBUGF  1, "aspm for L1C\n"

        mov     esi, [ebx + device.mmio_addr]
        mov     eax, dword[esi + ALX_PMCTRL]

        and     eax, not(ALX_PMCTRL_L0S_EN or ALX_PMCTRL_L1_EN or ALX_PMCTRL_ASPM_FCEN)
        or      eax, (ALX_PMCTRL_LCKDET_TIMER_DEF shl ALX_PMCTRL_LCKDET_TIMER_SHIFT) ;\
                ;     or (0 shl ALX_PMCTRL_L1_TIMER_SHIFT)

        or      eax, ecx

;;; FIXME if(linkon)

        or      eax, (ALX_PMCTRL_L1_SRDS_EN or ALX_PMCTRL_L1_SRDSPLL_EN or ALX_PMCTRL_L1_BUFSRX_EN)
        and     eax, not(ALX_PMCTRL_L1_SRDSRX_PWD or ALX_PMCTRL_L1_CLKSW_EN or ALX_PMCTRL_L0S_EN or ALX_PMCTRL_L1_EN)

;;

        mov     dword[esi + ALX_PMCTRL], eax

        ret

endp

proc alx_reset_mac stdcall

        DEBUGF  1, "reset mac\n"

; disable all interrupts, RXQ/TXQ
        mov     esi, [ebx + device.mmio_addr]
        mov     dword[esi + ALX_MSIX_MASK], 0xffffffff
        mov     dword[esi + ALX_IMR], 0x0
        mov     dword[esi + ALX_ISR], ALX_ISR_DIS

        stdcall alx_stop_mac

; mac reset workaround
        mov     dword[esi + ALX_RFD_PIDX], 1

; disable l0s/l1 before MAC reset on some chips
        cmp     [ebx + device.chip_rev], ALX_REV_A1
        ja      @f
        test    [ebx + device.pci_rev], ALX_PCI_REVID_WITH_CR
        jz      @f
        mov     eax, [esi + ALX_PMCTRL]
        mov     edx, eax
        test    eax, ALX_PMCTRL_L1_EN or ALX_PMCTRL_L0S_EN
        jz      @f
        and     eax, not(ALX_PMCTRL_L1_EN or ALX_PMCTRL_L0S_EN)
        mov     [esi + ALX_PMCTRL], eax
  @@:

; reset whole MAC safely
        mov     eax, [esi + ALX_MASTER]
        or      eax, ALX_MASTER_DMA_MAC_RST or ALX_MASTER_OOB_DIS
        mov     [esi + ALX_MASTER], eax

; make sure it's real idle
        stdcall udelay, 10

        mov     ecx, ALX_DMA_MAC_RST_TO ; timeout
  .loop1:
        mov     eax, dword[esi + ALX_RFD_PIDX]
        test    eax, eax
        jz      @f

        stdcall udelay, 10

        dec     ecx
        jnz     .loop1
        jmp     .error
  @@:

  .loop2:
        mov     eax, dword[esi + ALX_MASTER]
        test    eax, ALX_MASTER_DMA_MAC_RST
        jz      @f

        stdcall udelay, 10

        dec     ecx
        jnz     .loop2
        jmp     .error
  @@:

; restore l0s/l1
        cmp     [ebx + device.chip_rev], ALX_REV_A1
        ja      @f
        test    [ebx + device.pci_rev], ALX_PCI_REVID_WITH_CR
        jz      @f
        or      eax, ALX_MASTER_PCLKSEL_SRDS
        mov     [esi + ALX_MASTER], eax
        mov     [esi + ALX_PMCTRL], edx
  @@:

        stdcall alx_reset_osc

; clear Internal OSC settings, switching OSC by hw itself, disable isolate for rev A devices

        mov     eax, [esi + ALX_MISC3]
        and     eax, not (ALX_MISC3_25M_BY_SW)
        or      eax, ALX_MISC3_25M_NOTO_INTNL
        mov     [esi + ALX_MISC3], eax

        mov     eax, [esi + ALX_MISC]
        and     eax, not (ALX_MISC_INTNLOSC_OPEN)

        cmp     [ebx + device.chip_rev], ALX_REV_A1
        ja      @f
        and     eax, not ALX_MISC_ISO_EN
  @@:
        mov     [esi + ALX_MISC], eax

        stdcall udelay, 20

; driver control speed/duplex, hash-alg
        mov     eax, [ebx + device.rx_ctrl]
        mov     [esi + ALX_MAC_CTRL], eax

; clk sw
        mov     eax, dword[esi + ALX_SERDES]
        or      eax, ALX_SERDES_MACCLK_SLWDWN or ALX_SERDES_PHYCLK_SLWDWN
        mov     dword[esi + ALX_SERDES], eax

        DEBUGF  1, "OK\n"
        xor     eax, eax
        ret

  .error:
        DEBUGF  1, "error\n"
        xor     eax, eax
        dec     eax
        ret

endp

proc alx_reset_phy stdcall

        DEBUGF  1, "Reset phy\n"

        mov     esi, [ebx + device.mmio_addr]

        mov     eax, dword [esi + ALX_PHY_CTRL]
        DEBUGF 1, "read ALX_PHY_CTRL = %x\n", eax
        and     eax, not (ALX_PHY_CTRL_DSPRST_OUT or ALX_PHY_CTRL_IDDQ or ALX_PHY_CTRL_GATE_25M or ALX_PHY_CTRL_POWER_DOWN or ALX_PHY_CTRL_CLS)
        or      eax, ALX_PHY_CTRL_RST_ANALOG
        or      eax, ALX_PHY_CTRL_HIB_PULSE or ALX_PHY_CTRL_HIB_EN ; assume pws is enabled

        DEBUGF 1, "write ALX_PHY_CTRL = %x\n", eax
        mov     [esi + ALX_PHY_CTRL], eax

        stdcall udelay, 5

        or      eax, ALX_PHY_CTRL_DSPRST_OUT
        mov     [esi + ALX_PHY_CTRL], eax

        stdcall udelay, 10

        or      eax, ALX_PHY_CTRL_DSPRST_OUT
        DEBUGF 1, "write ALX_PHY_CTRL = %x\n", eax
        mov     dword [esi + ALX_PHY_CTRL], eax

        stdcall udelay, 800

; PHY power saving & hibernate
        stdcall alx_write_phy_dbg, ALX_MIIDBG_LEGCYPS, ALX_LEGCYPS_DEF
        stdcall alx_write_phy_dbg, ALX_MIIDBG_SYSMODCTRL, ALX_SYSMODCTRL_IECHOADJ_DEF
        stdcall alx_write_phy_ext, ALX_MIIEXT_PCS, ALX_MIIEXT_VDRVBIAS, ALX_VDRVBIAS_DEF

; EEE advertisement
        mov     eax, [esi + ALX_LPI_CTRL]
        and     eax, not (ALX_LPI_CTRL_EN)
        mov     [esi + ALX_LPI_CTRL], eax
        stdcall alx_write_phy_ext, ALX_MIIEXT_ANEG, ALX_MIIEXT_LOCAL_EEEADV, 0

; PHY power saving
        stdcall alx_write_phy_dbg, ALX_MIIDBG_TST10BTCFG, ALX_TST10BTCFG_DEF
        stdcall alx_write_phy_dbg, ALX_MIIDBG_SRDSYSMOD, ALX_SRDSYSMOD_DEF
        stdcall alx_write_phy_dbg, ALX_MIIDBG_TST100BTCFG, ALX_TST100BTCFG_DEF
        stdcall alx_write_phy_dbg, ALX_MIIDBG_ANACTRL, ALX_ANACTRL_DEF
        stdcall alx_read_phy_dbg, ALX_MIIDBG_GREENCFG2
        and     eax, not ALX_GREENCFG2_GATE_DFSE_EN
        stdcall alx_write_phy_dbg, ALX_MIIDBG_GREENCFG2, eax
; rtl8139c, 120m issue */
        stdcall alx_write_phy_ext, ALX_MIIEXT_ANEG, ALX_MIIEXT_NLP78, ALX_MIIEXT_NLP78_120M_DEF
        stdcall alx_write_phy_ext, ALX_MIIEXT_ANEG, ALX_MIIEXT_S3DIG10, ALX_MIIEXT_S3DIG10_DEF

; TODO: link patch ?

; set PHY interrupt mask
        stdcall alx_read_phy_reg, ALX_MII_IER
        or      eax, ALX_IER_LINK_UP or ALX_IER_LINK_DOWN
        stdcall alx_write_phy_reg, ALX_MII_IER , eax

        DEBUGF  1, "OK\n"
        xor     eax, eax
        ret

  .error:
        DEBUGF  1, "error\n"
        xor     eax, eax
        dec     eax
        ret

endp

proc alx_set_macaddr stdcall

        mov     esi, [ebx + device.mmio_addr]

        mov     eax, dword[ebx + device.mac+2]
        bswap   eax
        mov     [esi + ALX_STAD0], eax
        mov     ax, word[ebx + device.mac]
        xchg    al, ah
        mov     [esi + ALX_STAD1], ax


        ret
endp

proc alx_enable_osc stdcall

        mov     esi, [ebx + device.mmio_addr]

; rising edge
        mov     eax, dword[esi + ALX_MISC]
        and     eax, not ALX_MISC_INTNLOSC_OPEN
        mov     dword[esi + ALX_MISC], eax
        or      eax, ALX_MISC_INTNLOSC_OPEN
        mov     dword[esi + ALX_MISC], eax

        ret

endp

proc alx_reset_osc stdcall

        mov     esi, [ebx + device.mmio_addr]

; clear Internal OSC settings, switching OSC by hw itself
        mov     eax, dword[esi + ALX_MISC3]
        and     eax, not ALX_MISC3_25M_BY_SW
        or      eax, ALX_MISC3_25M_NOTO_INTNL
        mov     dword[esi + ALX_MISC3], eax

; clk from chipset may be unstable 1s after de-assert of
; PERST, driver need re-calibrate before enter Sleep for WoL
        mov     eax, dword[esi + ALX_MISC]
        cmp     [ebx + device.chip_rev], ALX_REV_B0
        jb      .rev_A

; restore over current protection def-val, this val could be reset by MAC-RST
        and     eax, not (ALX_MISC_PSW_OCP_MASK shl ALX_MISC_PSW_OCP_SHIFT)
        or      eax, ALX_MISC_PSW_OCP_DEF shl ALX_MISC_PSW_OCP_SHIFT
; a 0->1 change will update the internal val of osc
        and     eax, not ALX_MISC_INTNLOSC_OPEN
        mov     dword[esi + ALX_MISC], eax
        or      eax, ALX_MISC_INTNLOSC_OPEN
        mov     dword[esi + ALX_MISC], eax

; hw will automatically dis OSC after cab
        mov     eax, dword[esi + ALX_MSIC2]
        and     eax, not ALX_MSIC2_CALB_START
        mov     dword[esi + ALX_MSIC2], eax
        or      eax, ALX_MSIC2_CALB_START
        mov     dword[esi + ALX_MSIC2], eax

        stdcall udelay, 20

        ret

  .rev_A:

;  disable isolate for rev A devices
        and     eax, not (ALX_MISC_ISO_EN)
        or      eax, ALX_MISC_INTNLOSC_OPEN
        mov     dword[esi + ALX_MISC], eax
        and     eax, not ALX_MISC_INTNLOSC_OPEN
        mov     dword[esi + ALX_MISC], eax

        stdcall udelay, 20

        ret

endp

proc alx_read_macaddr stdcall

        mov     esi, [ebx + device.mmio_addr]
        mov     eax, dword[esi + ALX_STAD0]
        bswap   eax
        mov     dword[ebx + device.mac + 2], eax
        mov     ax, word[esi + ALX_STAD1]
        xchg    al, ah
        mov     word[ebx + device.mac], ax

        DEBUGF  1,"MAC = %x-%x-%x-%x-%x-%x\n",\
        [ebx + device.mac+0]:2, [ebx + device.mac+1]:2, [ebx + device.mac+2]:2, [ebx + device.mac+3]:2, [ebx + device.mac+4]:2, [ebx + device.mac+5]:2

; check if it is a valid MAC
        cmp     dword[ebx + device.mac], 0x0
        jne     @f
        cmp     word[ebx + device.mac + 4], 0x0
        je      .invalid
  @@:
        cmp     dword[ebx + device.mac], 0xffffffff
        jne     @f
        cmp     word[ebx + device.mac + 4], 0xffff
        je      .invalid
  @@:
        test    byte[ebx + device.mac + 5], 0x01        ; Multicast
        jnz     .invalid
  @@:
        xor     eax, eax
        ret

  .invalid:
        DEBUGF  1, "Invalid MAC!\n"
        xor     eax, eax
        inc     eax
        ret

endp

proc alx_get_perm_macaddr stdcall

; try to get it from register first
        stdcall alx_read_macaddr
        test    eax, eax
        jz      .done

; try to load from efuse
        mov     esi, [ebx + device.mmio_addr]
        mov     ecx, ALX_SLD_MAX_TO
  .loop1:
        mov     eax, dword[esi + ALX_SLD]
        test    eax, ALX_SLD_STAT or ALX_SLD_START
        jz      @f

        dec     ecx
        jz      .error

        push    esi ecx
        xor     esi, esi
        inc     esi
        invoke  Sleep
        pop     ecx esi
        jmp     .loop1
  @@:
        or      eax, ALX_SLD_START
        mov     dword[esi + ALX_SLD], eax

        mov     ecx, ALX_SLD_MAX_TO
  .loop2:
        mov     eax, dword[esi + ALX_SLD]
        test    eax, ALX_SLD_START
        jz      @f

        dec     ecx
        jz      .error

        push    esi ecx
        xor     esi, esi
        inc     esi
        invoke  Sleep
        pop     ecx esi
        jmp     .loop2
  @@:

        stdcall alx_read_macaddr
        test    eax, eax
        jz      .done

; try to load from flash/eeprom (if present)
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], ALX_EFLD
        in      eax, dx
        test    eax, ALX_EFLD_F_EXIST or ALX_EFLD_E_EXIST
        jz      .error

        mov     ecx, ALX_SLD_MAX_TO
  .loop3:
        in      eax, dx
        test    eax, ALX_EFLD_STAT or ALX_EFLD_START
        jz      @f

        dec     ecx
        jz      .error

        push    esi edx ecx
        xor     esi, esi
        inc     esi
        invoke  Sleep
        pop     ecx edx esi
        jmp     .loop3
  @@:
        or      eax, ALX_EFLD_START
        out     dx, eax

        mov     ecx, ALX_SLD_MAX_TO
  .loop4:
        in      eax, dx
        test    eax, ALX_EFLD_START
        jz      @f

        dec     ecx
        jz      .error

        push    esi edx ecx
        xor     esi, esi
        inc     esi
        invoke  Sleep
        pop     ecx edx esi
        jmp     .loop4
  @@:

        stdcall alx_read_macaddr
        test    eax, eax
        jz      .done

  .error:
        DEBUGF  1, "error obtaining MAC\n"
        xor     eax, eax
        dec     eax
        ret

  .done:
        DEBUGF  1, "MAC OK\n"
        xor     eax, eax
        ret

endp

proc alx_stop_mac stdcall

        DEBUGF  1,"alx_stop_mac\n"

        mov     esi, [ebx + device.mmio_addr]

        mov     eax, dword[esi + ALX_RXQ0]
        and     eax, not ALX_RXQ0_EN
        mov     dword[esi + ALX_RXQ0], eax

        mov     eax, dword[esi + ALX_TXQ0]
        and     eax, not ALX_TXQ0_EN
        mov     dword[esi + ALX_TXQ0], eax

        stdcall udelay, 40

        mov     eax, [ebx + device.rx_ctrl]
        and     eax, not(ALX_MAC_CTRL_TX_EN or ALX_MAC_CTRL_RX_EN)
        mov     [ebx + device.rx_ctrl], eax
        mov     [esi + ALX_MAC_CTRL], eax

        mov     ecx, ALX_DMA_MAC_RST_TO
  .loop:
        mov     eax, [esi + ALX_MAC_STS]
        test    eax, ALX_MAC_STS_IDLE
        jz      .done

        stdcall udelay, 10

        dec     ecx
        jnz     .loop

        DEBUGF  1,"alx_stop_mac timeout!\n"
        xor     eax, eax
        dec     eax
        ret

  .done:
        DEBUGF  1,"alx_stop_mac ok\n"
        xor     eax, eax
        ret

endp

proc alx_start_mac stdcall

        DEBUGF  1,"alx_start_mac\n"

        mov     esi, [ebx + device.mmio_addr]

        mov     eax, dword[esi + ALX_RXQ0]
        or      eax, ALX_RXQ0_EN
        mov     dword[esi + ALX_RXQ0], eax

        mov     eax, dword[esi + ALX_TXQ0]
        or      eax, ALX_TXQ0_EN
        mov     dword[esi + ALX_TXQ0], eax

        mov     eax, [ebx + device.rx_ctrl]
        or      eax, ALX_MAC_CTRL_TX_EN or ALX_MAC_CTRL_RX_EN
        and     eax, not ALX_MAC_CTRL_FULLD
        test    [ebx + device.state], ETH_LINK_FULL_DUPLEX
        jz      .no_fd
        or      eax, ALX_MAC_CTRL_FULLD
  .no_fd:
        and     eax, not (ALX_MAC_CTRL_SPEED_MASK shl ALX_MAC_CTRL_SPEED_SHIFT)
        mov     ecx, [ebx + device.state]
        and     ecx, ETH_LINK_SPEED_MASK
        cmp     ecx, ETH_LINK_SPEED_1G
        jne      .10_100
        or      eax, (ALX_MAC_CTRL_SPEED_1000 shl ALX_MAC_CTRL_SPEED_SHIFT)
        jmp     .done

  .10_100:
        or      eax, (ALX_MAC_CTRL_SPEED_10_100 shl ALX_MAC_CTRL_SPEED_SHIFT)

  .done:
        DEBUGF  1,"mac ctrl=0x%x\n", eax
        mov     [ebx + device.rx_ctrl], eax
        mov     [esi + ALX_MAC_CTRL], eax

        ret

endp

proc alx_init_ring_ptrs stdcall

        DEBUGF  1,"alx_init_ring_ptrs\n"

        mov     esi, [ebx + device.mmio_addr]

; Receive descriptors
        mov     [ebx + device.rxq_read_idx], 0
        mov     [ebx + device.rxq_write_idx], 0
;        mov     [ebx + device.rxq_rrd_read_idx], 0

        mov     dword[esi + ALX_RX_BASE_ADDR_HI], 0

        lea     eax, [ebx + device.rrd_ring]
        invoke  GetPhysAddr
        mov     dword[esi + ALX_RRD_ADDR_LO], eax
        mov     dword[esi + ALX_RRD_RING_SZ], RX_RING_SIZE

        lea     eax, [ebx + device.rfd_ring]
        invoke  GetPhysAddr
        mov     dword[esi + ALX_RFD_ADDR_LO], eax
        mov     dword[esi + ALX_RFD_RING_SZ], RX_RING_SIZE
        mov     dword[esi + ALX_RFD_BUF_SZ], RX_BUFFER_SIZE

; Transmit descriptors
        mov     [ebx + device.txq_read_idx], 0
        mov     [ebx + device.txq_write_idx], 0

        mov     dword[esi + ALX_TX_BASE_ADDR_HI], 0

        lea     eax, [ebx + device.tpd_ring]
        invoke  GetPhysAddr
        mov     dword[esi + ALX_TPD_PRI0_ADDR_LO], eax
        mov     dword[esi + ALX_TPD_RING_SZ], TX_RING_SIZE

; Load these pointers into the chip
        mov     dword[esi + ALX_SRAM9], ALX_SRAM_LOAD_PTR

        xor     eax, eax

        ret

endp

proc alx_alloc_rings stdcall

        DEBUGF  1,"alx_alloc_rings\n"

        and     [ebx + device.int_mask], not ALX_ISR_ALL_QUEUES
        or      [ebx + device.int_mask], ALX_ISR_TX_Q0 or ALX_ISR_RX_Q0
        stdcall alx_reinit_rings

        ret

endp

proc alx_reinit_rings stdcall

        DEBUGF  1,"alx_reinit_rings\n"

        stdcall alx_free_rx_ring
        stdcall alx_init_ring_ptrs
        stdcall alx_refill_rx_ring

        ret

endp

proc alx_refill_rx_ring stdcall

        DEBUGF  1,"alx_refill_rx_ring\n"

        mov     ecx, [ebx + device.rxq_write_idx]
  .loop:
        cmp     dword[ebx + device.rfd_ring+ecx*sizeof.alx_rfd + alx_rfd.addr_l], 0
        jne     .done

        invoke  NetAlloc, NET_BUFF.data+RX_BUFFER_SIZE
        test    eax, eax
        jz      .done
        mov     [ebx + device.rfd_ring_virt + ecx*4], eax
        add     eax, NET_BUFF.data
        invoke  GetPhysAddr
        mov     dword[ebx + device.rfd_ring+ecx*sizeof.alx_rfd + alx_rfd.addr_l], eax
        mov     dword[ebx + device.rfd_ring+ecx*sizeof.alx_rfd + alx_rfd.addr_h], 0

        mov     eax, ecx
        inc     ecx
        and     ecx, RX_RING_SIZE - 1

        cmp     ecx, [ebx + device.rxq_read_idx]
        jne     .loop

        mov     ecx, eax

  .done:
        cmp     ecx, [ebx + device.rxq_write_idx]
        je      .none

        mov     [ebx + device.rxq_write_idx], ecx
        mov     esi, [ebx + device.mmio_addr]
        mov     [esi + ALX_RFD_PIDX], cx

  .none:
        xor     eax, eax

        ret

endp

proc alx_free_rx_ring stdcall

        DEBUGF  1,"alx_free_rx_ring\n"

        xor     ecx, ecx
  .loop:
        mov     eax, [ebx + device.rfd_ring_virt + ecx*4]
        test    eax, eax
        jz      .next

        invoke  NetFree, eax

        xor     eax, eax
        mov     dword[ebx + device.rfd_ring+ecx*sizeof.alx_rfd + alx_rfd.addr_l], eax
        mov     dword[ebx + device.rfd_ring+ecx*sizeof.alx_rfd + alx_rfd.addr_h], eax
        mov     [ebx + device.rfd_ring_virt + ecx*4], eax

  .next:
        inc     ecx
        cmp     ecx, RX_RING_SIZE
        jb      .loop

        ret

endp

proc alx_configure stdcall

        DEBUGF  1,"alx_configure\n"

        stdcall alx_configure_basic
        stdcall alx_disable_rss
        call    alx_set_rx_mode

        mov     esi, [ebx + device.mmio_addr]
        mov     eax, [ebx + device.rx_ctrl]
        mov     [esi + ALX_MAC_CTRL], eax

        xor     eax, eax
        ret

endp

proc alx_irq_enable stdcall

        DEBUGF  1,"alx_irq_enable\n"

        mov     esi, [ebx + device.mmio_addr]
        mov     dword[esi + ALX_ISR], 0
        mov     eax, [ebx + device.int_mask]
        mov     [esi + ALX_IMR], eax

        stdcall alx_post_write

        ret

endp

proc alx_irq_disable stdcall

        DEBUGF  1,"alx_irq_disable\n"

        mov     esi, [ebx + device.mmio_addr]
        mov     dword[esi + ALX_ISR], ALX_ISR_DIS
        mov     dword[esi + ALX_IMR], 0

        stdcall alx_post_write

        ret

endp

proc alx_post_write stdcall

        push    eax
        mov     esi, [ebx + device.mmio_addr]
        mov     eax, [esi]
        pop     eax

        ret

endp

proc alx_configure_basic stdcall

        DEBUGF  1,"alx_configure_basic\n"

        mov     esi, [ebx + device.mmio_addr]

        stdcall alx_set_macaddr

        mov     dword[esi + ALX_CLK_GATE], ALX_CLK_GATE_ALL

; idle timeout to switch clk_125M
        cmp     [ebx + device.chip_rev], ALX_REV_B0
        jb      @f
        mov     dword[esi + ALX_IDLE_DECISN_TIMER], ALX_IDLE_DECISN_TIMER_DEF
  @@:

        mov     dword[esi + ALX_SMB_TIMER], SMB_TIMER * 500

; Interrupt moderation
        mov     eax, [esi + ALX_MASTER]
        or      eax, ALX_MASTER_IRQMOD2_EN or ALX_MASTER_IRQMOD1_EN or ALX_MASTER_SYSALVTIMER_EN
        mov     [esi + ALX_MASTER], eax

; Set interupt moderator timer (max interupts per second)
        mov     dword[esi + ALX_IRQ_MODU_TIMER], ((IMT) shl ALX_IRQ_MODU_TIMER1_SHIFT) or ((IMT / 2) shl ALX_IRQ_MODU_TIMER2_SHIFT)

; Interrupt re-trigger timeout
        mov     dword[esi + ALX_INT_RETRIG], ALX_INT_RETRIG_TO

; tpd threshold to trig int
        mov     dword[esi + ALX_TINT_TPD_THRSHLD], ITH_TPD
        mov     dword[esi + ALX_TINT_TIMER], IMT

        mov     dword[esi + ALX_MTU], RX_BUFFER_SIZE + 8 ;;;;

        mov     dword[esi + ALX_TXQ1], (((RX_BUFFER_SIZE + 8 + 7) shr 3) or ALX_TXQ1_ERRLGPKT_DROP_EN)

; rxq, flow control

; TODO set ALX_RXQ2

; RXQ0
        mov     eax, (ALX_RXQ0_NUM_RFD_PREF_DEF shl ALX_RXQ0_NUM_RFD_PREF_SHIFT) \
                or (ALX_RXQ0_RSS_MODE_DIS shl ALX_RXQ0_RSS_MODE_SHIFT) \
                or (ALX_RXQ0_IDT_TBL_SIZE_DEF shl ALX_RXQ0_IDT_TBL_SIZE_SHIFT) \
                or ALX_RXQ0_RSS_HSTYP_ALL or ALX_RXQ0_RSS_HASH_EN or ALX_RXQ0_IPV6_PARSE_EN

        test    [ebx + device.pci_did], 1       ;;; FIXME: is gigabit device?
        jz      @f
        or      eax, ALX_RXQ0_ASPM_THRESH_100M shl ALX_RXQ0_ASPM_THRESH_SHIFT
  @@:
        mov     [esi + ALX_RXQ0], eax

; DMA
        max_payload equ 2               ;;;; FIXME

        mov     eax, [esi + ALX_DMA]            ; Read and ignore?
        ; Pre-B0 devices have 2 DMA channels
        mov     eax, (ALX_DMA_RORDER_MODE_OUT shl ALX_DMA_RORDER_MODE_SHIFT) \
                or ALX_DMA_RREQ_PRI_DATA \
                or (max_payload shl ALX_DMA_RREQ_BLEN_SHIFT) \
                or (ALX_DMA_WDLY_CNT_DEF shl ALX_DMA_WDLY_CNT_SHIFT ) \
                or (ALX_DMA_RDLY_CNT_DEF shl ALX_DMA_RDLY_CNT_SHIFT ) \
                or ((2-1) shl ALX_DMA_RCHNL_SEL_SHIFT)

        cmp     [ebx + device.chip_rev], ALX_REV_B0
        jb      @f
        ; B0 and newer have 4 DMA channels
        mov     eax, (ALX_DMA_RORDER_MODE_OUT shl ALX_DMA_RORDER_MODE_SHIFT) \
                or ALX_DMA_RREQ_PRI_DATA \
                or (max_payload shl ALX_DMA_RREQ_BLEN_SHIFT) \
                or (ALX_DMA_WDLY_CNT_DEF shl ALX_DMA_WDLY_CNT_SHIFT ) \
                or (ALX_DMA_RDLY_CNT_DEF shl ALX_DMA_RDLY_CNT_SHIFT ) \
                or ((4-1) shl ALX_DMA_RCHNL_SEL_SHIFT)
  @@:
        mov     [esi + ALX_DMA], eax

; default multi-tx-q weights
        mov      eax, (ALX_WRR_PRI_RESTRICT_NONE shl ALX_WRR_PRI_SHIFT) \
                 or (4 shl ALX_WRR_PRI0_SHIFT) \
                 or (4 shl ALX_WRR_PRI1_SHIFT) \
                 or (4 shl ALX_WRR_PRI2_SHIFT) \
                 or (4 shl ALX_WRR_PRI3_SHIFT)
        mov      [esi + ALX_WRR], eax

        ret

endp

proc alx_disable_rss stdcall

        DEBUGF  1,"alx_disable_rss\n"

        mov     esi, [ebx + device.mmio_addr]

        mov     eax, [esi + ALX_RXQ0]
        and     eax, not (ALX_RXQ0_RSS_HASH_EN)
        mov     [esi + ALX_RXQ0] , eax

        ret

endp

proc alx_set_rx_mode stdcall

        DEBUGF  1,"__alx_set_rx_mode\n"

        mov     esi, [ebx + device.mmio_addr]

; TODO: proper multicast

;        if (!(netdev->flags & IFF_ALLMULTI)) {
;                netdev_for_each_mc_addr(ha, netdev)
;                        alx_add_mc_addr(hw, ha->addr, mc_hash);
;
;                alx_write_mem32(hw, ALX_HASH_TBL0, mc_hash[0]);
;                alx_write_mem32(hw, ALX_HASH_TBL1, mc_hash[1]);
;        }

        mov     eax, [ebx + device.rx_ctrl]
        or      eax, ALX_MAC_CTRL_PROMISC_EN or ALX_MAC_CTRL_MULTIALL_EN        ; FIXME: dont force promiscous mode..
        mov     [ebx + device.rx_ctrl], eax
        mov     dword[esi + ALX_MAC_CTRL], eax

        ret

endp

proc alx_check_link stdcall

        stdcall alx_clear_phy_intr

        mov     edx, [ebx + device.state]

        stdcall alx_get_phy_link
        cmp     eax, 0
        jl      .reset

        mov     esi, [ebx + device.mmio_addr]

        spin_lock_irqsave
        or      [ebx + device.int_mask], ALX_ISR_PHY
        mov     eax, [ebx + device.int_mask]
        mov     [esi + ALX_IMR], eax
        spin_unlock_irqrestore

        cmp     edx, [ebx + device.state]
        je      .no_change

        cmp     [ebx + device.state], ETH_LINK_DOWN
        je      .link_down

        stdcall alx_post_phy_link
        mov     ecx, (ALX_PMCTRL_L0S_EN or ALX_PMCTRL_L1_EN or ALX_PMCTRL_ASPM_FCEN)
        stdcall alx_enable_aspm
        stdcall alx_start_mac

        invoke  NetLinkChanged

        ret

  .no_change:
        DEBUGF  1, "link state unchanged\n"

        ret

  .link_down:
; Link is now down

        stdcall alx_reset_mac
        test    eax, eax
        jnz     .reset

        stdcall alx_irq_disable

; MAC reset causes all HW settings to be lost, restore all
        stdcall alx_reinit_rings
        test    eax, eax
        jnz     .reset

        stdcall alx_configure
        mov     ecx, (ALX_PMCTRL_L1_EN or ALX_PMCTRL_ASPM_FCEN)
        stdcall alx_enable_aspm
        stdcall alx_post_phy_link
        stdcall alx_irq_enable

        invoke  NetLinkChanged

        ret

  .reset:
        DEBUGF  1, "alx_schedule_reset\n"
;;;        stdcall alx_schedule_reset

        ret

endp

proc alx_post_phy_link stdcall

        DEBUGF  1, "alx_post_phy_link\n"

        cmp     [ebx + device.chip_rev], ALX_REV_B0
        ja      .done

        cmp     [ebx + device.state], ETH_LINK_UNKNOWN
        jae     @f

; TODO: vendor hocus-pocus to tune the PHY according the detected cable length
        stdcall alx_write_phy_dbg, ALX_MIIDBG_AZ_ANADECT, ALX_AZ_ANADECT_DEF
        stdcall alx_read_phy_ext, ALX_MIIEXT_AFE, ALX_MIIEXT_ANEG
        and     eax, not (ALX_AFE_10BT_100M_TH)
        stdcall alx_write_phy_ext, ALX_MIIEXT_AFE, ALX_MIIEXT_ANEG, eax

        ret
  @@:

  .done:

        ret

endp

proc alx_clear_phy_intr stdcall

        DEBUGF  1,"alx_clear_phy_intr\n"
        stdcall alx_read_phy_reg, ALX_MII_ISR

        ret

endp

proc alx_get_phy_link stdcall

        DEBUGF  1,"alx_get_phy_link\n"

        stdcall alx_read_phy_reg, MII_BMSR
        stdcall alx_read_phy_reg, MII_BMSR

        mov     [ebx + device.state], ETH_LINK_DOWN

        test    ax, BMSR_LSTATUS
        jnz     @f
        DEBUGF  1,"link is down\n"
        xor     eax, eax
        ret
  @@:
        stdcall alx_read_phy_reg, ALX_MII_GIGA_PSSR
        test    ax, ALX_GIGA_PSSR_SPD_DPLX_RESOLVED
        jz      .wrong_speed

        DEBUGF  1,"link is up\n"

        test    ax, ALX_GIGA_PSSR_DPLX
        jz      @f
        or      [ebx + device.state], ETH_LINK_FULL_DUPLEX
        DEBUGF  1,"full duplex\n"
  @@:

        and     ax, ALX_GIGA_PSSR_SPEED
        cmp     ax, ALX_GIGA_PSSR_1000MBS
        jne     @f
        or      [ebx + device.state], ETH_LINK_SPEED_1G
        DEBUGF  1,"1 gigabit\n"
        ret

  @@:
        cmp     ax, ALX_GIGA_PSSR_100MBS
        jne     @f
        or      [ebx + device.state], ETH_LINK_SPEED_100M
        DEBUGF  1,"100 Mbit\n"
        ret

  @@:
        cmp     ax, ALX_GIGA_PSSR_10MBS
        jne     @f
        or      [ebx + device.state], ETH_LINK_SPEED_10M
        DEBUGF  1,"10 Mbit\n"
        ret

  @@:
        mov     [ebx + device.state], ETH_LINK_UNKNOWN
        DEBUGF  1,"speed unknown\n"
        ret

  .wrong_speed:
        DEBUGF  1,"wrong speed\n"
        xor     eax, eax
        dec     eax
        ret

endp

proc alx_read_phy_reg stdcall, reg:dword

; FIXME: fixed clock

        DEBUGF  1,"alx_read_phy_reg reg=0x%x\n", [reg]:4

        mov     esi, [ebx + device.mmio_addr]

        mov     eax, [reg]
        shl     eax, ALX_MDIO_REG_SHIFT
        or      eax, ALX_MDIO_SPRES_PRMBL or (ALX_MDIO_CLK_SEL_25MD4 shl ALX_MDIO_CLK_SEL_SHIFT) or ALX_MDIO_START or ALX_MDIO_OP_READ
        mov     dword[esi + ALX_MDIO], eax

        mov     ecx, ALX_MDIO_MAX_AC_TO
  .loop:
        mov     eax, dword[esi + ALX_MDIO]
        test    eax, ALX_MDIO_BUSY
        jz      .ready

        stdcall udelay, 10

        dec     ecx
        jnz     .loop

        DEBUGF  1,"alx_read_phy_reg read timeout!\n"
        xor     eax, eax
        dec     eax
        ret

  .ready:
;        shr     eax, ALX_MDIO_DATA_SHIFT
        and     eax, ALX_MDIO_DATA_MASK

        DEBUGF  1,"alx_read_phy_reg data=0x%x\n", eax:4

        ret

endp

proc  alx_read_phy_ext stdcall, dev:dword, reg:dword

; FIXME: fixed clock

        DEBUGF  1,"alx_read_phy_ext dev=0x%x reg=0x%x\n", [dev]:4, [reg]:4

        mov     esi, [ebx + device.mmio_addr]

        mov     eax, [dev]
        shl     eax, ALX_MDIO_EXTN_DEVAD_SHIFT
        mov     ax, word[reg]
;        shl     eax, ALX_MDIO_EXTN_REG_SHIFT
        mov     dword[esi + ALX_MDIO_EXTN], eax

        mov     eax, ALX_MDIO_SPRES_PRMBL or (ALX_MDIO_CLK_SEL_25MD4 shl ALX_MDIO_CLK_SEL_SHIFT) or ALX_MDIO_START or ALX_MDIO_OP_READ or ALX_MDIO_MODE_EXT
        mov     dword[esi + ALX_MDIO], eax

        mov     ecx, ALX_MDIO_MAX_AC_TO
  .loop:
        mov     eax, dword[esi + ALX_MDIO]
        test    eax, ALX_MDIO_BUSY
        jz      .ready

        stdcall udelay, 10

        dec     ecx
        jnz     .loop

        DEBUGF  1,"alx_read_phy_ext read timeout!\n"
        xor     eax, eax
        dec     eax
        ret

  .ready:
;        shr     eax, ALX_MDIO_DATA_SHIFT
        and     eax, ALX_MDIO_DATA_MASK

        DEBUGF  1,"alx_read_phy_ext data=0x%x\n", eax:4

        ret

endp

proc  alx_write_phy_reg stdcall, reg:dword, val:dword

; FIXME: fixed clock

        DEBUGF  1,"alx_write_phy_reg reg=0x%x data=0x%x\n", [reg]:4, [val]:4

        mov     esi, [ebx + device.mmio_addr]

        mov     eax, [reg]
        shl     eax, ALX_MDIO_REG_SHIFT
        mov     ax, word[val]                   ; data must be in 16 lower bits :)
        or      eax, ALX_MDIO_SPRES_PRMBL or (ALX_MDIO_CLK_SEL_25MD4 shl ALX_MDIO_CLK_SEL_SHIFT) or ALX_MDIO_START
        mov     dword[esi + ALX_MDIO], eax

        mov     ecx, ALX_MDIO_MAX_AC_TO
  .loop:
        mov     eax, dword[esi + ALX_MDIO]
        test    eax, ALX_MDIO_BUSY
        jz      .ready

        stdcall udelay, 10

        dec     ecx
        jnz     .loop

        DEBUGF  1,"alx_write_phy_reg timeout!\n"
        xor     eax, eax
        dec     eax
        ret

  .ready:
        DEBUGF  1,"alx_write_phy_reg OK\n"
        xor     eax, eax

        ret
endp

proc  alx_write_phy_dbg stdcall, reg:dword, val:dword

        DEBUGF  1,"alx_write_phy_dbg\n"

        stdcall alx_write_phy_reg, ALX_MII_DBG_ADDR, [reg]
        test    eax, eax
        jnz     @f
        stdcall alx_write_phy_reg, ALX_MII_DBG_DATA, [val]

        ret
  @@:
        DEBUGF  1,"alx_write_phy_dbg ERROR\n"

        ret

endp

proc  alx_read_phy_dbg stdcall, reg:dword

        DEBUGF  1,"alx_read_phy_dbg\n"

        stdcall alx_write_phy_reg, ALX_MII_DBG_ADDR, [reg]
        test    eax, eax
        jnz     @f
        stdcall alx_read_phy_reg, ALX_MII_DBG_DATA

        ret
  @@:
        DEBUGF  1,"alx_read_phy_dbg ERROR\n"

        ret

endp

proc  alx_write_phy_ext stdcall, dev:dword, reg:dword, val:dword

; FIXME: fixed clock

        DEBUGF  1,"alx_write_phy_ext dev=0x%x reg=0x%x, data=0x%x\n", [dev]:4, [reg]:4, [val]:4

        mov     esi, [ebx + device.mmio_addr]

        mov     eax, [dev]
        shl     eax, ALX_MDIO_EXTN_DEVAD_SHIFT
        mov     ax, word[reg]
;        shl     eax, ALX_MDIO_EXTN_REG_SHIFT
        mov     dword[esi + ALX_MDIO_EXTN], eax

        movzx   eax, word[val]                   ; data must be in 16 lower bits :)
        or      eax, ALX_MDIO_SPRES_PRMBL or (ALX_MDIO_CLK_SEL_25MD4 shl ALX_MDIO_CLK_SEL_SHIFT) or ALX_MDIO_START or ALX_MDIO_MODE_EXT
        mov     dword[esi + ALX_MDIO], eax

        mov     ecx, ALX_MDIO_MAX_AC_TO
  .loop:
        mov     eax, dword[esi + ALX_MDIO]
        test    eax, ALX_MDIO_BUSY
        jz      .ready

        stdcall udelay, 10

        dec     ecx
        jnz     .loop

        DEBUGF  1,"alx_write_phy_ext timeout!\n"
        xor     eax, eax
        dec     eax
        ret

  .ready:
        DEBUGF  1,"alx_write_phy_ext OK\n"
        xor     eax, eax

        ret
endp

alx_request_irq:

        DEBUGF  1,"Request IRQ\n"

        mov     esi, [ebx + device.mmio_addr]

; Only legacy interrupts supported for now.
        mov     dword[esi + ALX_MSI_RETRANS_TIMER], 0

        ret

; End of code

data fixups
end data

include '../peimport.inc'

my_service      db 'AR81XX',0                    ; max 16 chars include zero

chiplist:
                dd (ALX_DEV_ID_AR8131 shl 16) or ALX_VEN_ID, ar8131_sz
                dd (ALX_DEV_ID_AR8132 shl 16) or ALX_VEN_ID, ar8132_sz
                dd (ALX_DEV_ID_AR8151_1 shl 16) or ALX_VEN_ID, ar8151_1_sz
                dd (ALX_DEV_ID_AR8151_2 shl 16) or ALX_VEN_ID, ar8151_2_sz
                dd (ALX_DEV_ID_AR8152_1 shl 16) or ALX_VEN_ID, ar8152_1_sz
                dd (ALX_DEV_ID_AR8152_2 shl 16) or ALX_VEN_ID, ar8152_2_sz
                dd (ALX_DEV_ID_AR8161 shl 16) or ALX_VEN_ID, ar8161_sz
                dd (ALX_DEV_ID_E2200 shl 16) or ALX_VEN_ID, e2200_sz
                dd (ALX_DEV_ID_E2400 shl 16) or ALX_VEN_ID, e2400_sz
                dd (ALX_DEV_ID_E2500 shl 16) or ALX_VEN_ID, e2500_sz
                dd (ALX_DEV_ID_AR8162 shl 16) or ALX_VEN_ID, ar8162_sz
                dd (ALX_DEV_ID_AR8171 shl 16) or ALX_VEN_ID, ar8171_sz
                dd (ALX_DEV_ID_AR8172 shl 16) or ALX_VEN_ID, ar8172_sz
                dd 0

ar8131_sz       db "AR8131", 0
ar8132_sz       db "AR8132", 0
ar8151_1_sz     db "AR8151 rev1", 0
ar8151_2_sz     db "AR8151 rev2", 0
ar8152_1_sz     db "AR8152 rev1", 0
ar8152_2_sz     db "AR8152 rev2", 0
ar8161_sz       db "AR8161", 0
ar8162_sz       db "AR8162", 0
ar8171_sz       db "QCA8171", 0
ar8172_sz       db "QCA8172", 0
e2200_sz        db "Killer E2200", 0
e2400_sz        db "Killer E2400", 0
e2500_sz        db "Killer E2500", 0

include_debug_strings

align 4
devices         dd 0
device_list     rd MAX_DEVICES                  ; This list contains all pointers to device structures the driver is handling

