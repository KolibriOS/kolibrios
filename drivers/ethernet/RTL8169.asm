;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2018. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  RTL8169 driver for KolibriOS                                   ;;
;;                                                                 ;;
;;  Copyright 2007 mike.dld,                                       ;;
;;   mike.dld@gmail.com                                            ;;
;;                                                                 ;;
;;  Port to the new network stack by hidnplayr                     ;;
;;                                                                 ;;
;;  References:                                                    ;;
;;    r8169.c - linux driver                                       ;;
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

        MAX_DEVICES             = 16

        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2     ; 1 = verbose, 2 = errors only

        NUM_TX_DESC             = 4
        NUM_RX_DESC             = 4

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'

        REG_MAC0                = 0x0 ; Ethernet hardware address
        REG_MAR0                = 0x8 ; Multicast filter
        REG_TxDescStartAddr     = 0x20
        REG_TxHDescStartAddr    = 0x28
        REG_FLASH               = 0x30
        REG_ERSR                = 0x36
        REG_ChipCmd             = 0x37
        REG_TxPoll              = 0x38
        REG_IntrMask            = 0x3C
        REG_IntrStatus          = 0x3E
        REG_TxConfig            = 0x40
        REG_RxConfig            = 0x44
        REG_RxMissed            = 0x4C
        REG_Cfg9346             = 0x50
        REG_Config0             = 0x51
        REG_Config1             = 0x52
        REG_Config2             = 0x53
        REG_Config3             = 0x54
        REG_Config4             = 0x55
        REG_Config5             = 0x56
        REG_MultiIntr           = 0x5C
        REG_PHYAR               = 0x60
        REG_TBICSR              = 0x64
        REG_TBI_ANAR            = 0x68
        REG_TBI_LPAR            = 0x6A
        REG_PHYstatus           = 0x6C
        REG_RxMaxSize           = 0xDA
        REG_CPlusCmd            = 0xE0
        REG_RxDescStartAddr     = 0xE4
        REG_ETThReg             = 0xEC
        REG_FuncEvent           = 0xF0
        REG_FuncEventMask       = 0xF4
        REG_FuncPresetState     = 0xF8
        REG_FuncForceEvent      = 0xFC

        ; InterruptStatusBits
        ISB_SYSErr              = 0x8000
        ISB_PCSTimeout          = 0x4000
        ISB_SWInt               = 0x0100
        ISB_TxDescUnavail       = 0x80
        ISB_RxFIFOOver          = 0x40
        ISB_LinkChg             = 0x20
        ISB_RxOverflow          = 0x10
        ISB_TxErr               = 0x08
        ISB_TxOK                = 0x04
        ISB_RxErr               = 0x02
        ISB_RxOK                = 0x01

        ; RxStatusDesc
        SD_RxRES                = 0x00200000
        SD_RxCRC                = 0x00080000
        SD_RxRUNT               = 0x00100000
        SD_RxRWT                = 0x00400000

        ; ChipCmdBits
        CMD_Reset               = 0x10
        CMD_RxEnb               = 0x08
        CMD_TxEnb               = 0x04
        CMD_RxBufEmpty          = 0x01

        ; Cfg9346Bits
        CFG_9346_Lock           = 0x00
        CFG_9346_Unlock         = 0xC0

        ; rx_mode_bits
        RXM_AcceptErr           = 0x20
        RXM_AcceptRunt          = 0x10
        RXM_AcceptBroadcast     = 0x08
        RXM_AcceptMulticast     = 0x04
        RXM_AcceptMyPhys        = 0x02
        RXM_AcceptAllPhys       = 0x01

        ; RxConfigBits
        RXC_FIFOShift           = 13
        RXC_DMAShift            = 8

        ; TxConfigBits
        TXC_InterFrameGapShift  = 24
        TXC_DMAShift            = 8    ; DMA burst value (0-7) is shift this many bits

        ; PHYstatus
        PHYS_TBI_Enable         = 0x80
        PHYS_TxFlowCtrl         = 0x40
        PHYS_RxFlowCtrl         = 0x20
        PHYS_1000bpsF           = 0x10
        PHYS_100bps             = 0x08
        PHYS_10bps              = 0x04
        PHYS_LinkStatus         = 0x02
        PHYS_FullDup            = 0x01

        ; GIGABIT_PHY_registers
        PHY_CTRL_REG            = 0
        PHY_STAT_REG            = 1
        PHY_AUTO_NEGO_REG       = 4
        PHY_1000_CTRL_REG       = 9

        ; GIGABIT_PHY_REG_BIT
        PHY_Restart_Auto_Nego   = 0x0200
        PHY_Enable_Auto_Nego    = 0x1000

        ; PHY_STAT_REG = 1
        PHY_Auto_Neco_Comp      = 0x0020

        ; PHY_AUTO_NEGO_REG = 4
        PHY_Cap_10_Half         = 0x0020
        PHY_Cap_10_Full         = 0x0040
        PHY_Cap_100_Half        = 0x0080
        PHY_Cap_100_Full        = 0x0100

        ; PHY_1000_CTRL_REG = 9
        PHY_Cap_1000_Full       = 0x0200
        PHY_Cap_1000_Half       = 0x0100

        PHY_Cap_PAUSE           = 0x0400
        PHY_Cap_ASYM_PAUSE      = 0x0800

        PHY_Cap_Null            = 0x0

        ; _MediaType
        MT_10_Half              = 0x01
        MT_10_Full              = 0x02
        MT_100_Half             = 0x04
        MT_100_Full             = 0x08
        MT_1000_Full            = 0x10

        ; _TBICSRBit
        TBI_RESET               = 0x80000000
        TBI_LOOPBACK            = 0x40000000
        TBI_NW_ENABLE           = 0x20000000
        TBI_NW_RESTART          = 0x10000000
        TBI_LINK_OK             = 0x02000000
        TBI_NW_COMPLETE         = 0x01000000

        ; _DescStatusBit
        DSB_OWNbit              = 0x80000000
        DSB_EORbit              = 0x40000000
        DSB_FSbit               = 0x20000000
        DSB_LSbit               = 0x10000000

        RX_BUF_SIZE             = 1514          ; Rx Buffer size

; max supported gigabit ethernet frame size -- must be at least (dev->mtu+14+4)
        MAX_ETH_FRAME_SIZE      = 1514

        TX_FIFO_THRESH          = 256           ; In bytes

        RX_FIFO_THRESH          = 7             ; 7 means NO threshold, Rx buffer level before first PCI xfer
        RX_DMA_BURST            = 7             ; Maximum PCI burst, '6' is 1024
        TX_DMA_BURST            = 7             ; Maximum PCI burst, '6' is 1024
        ETTh                    = 0x3F          ; 0x3F means NO threshold

        EarlyTxThld             = 0x3F          ; 0x3F means NO early transmit
        RxPacketMaxSize         = 0x0800        ; Maximum size supported is 16K-1
        InterFrameGap           = 0x03          ; 3 means InterFrameGap = the shortest one

        HZ                      = 1000

        RTL_MIN_IO_SIZE         = 0x80
        TX_TIMEOUT              = (6*HZ)

        TIMER_EXPIRE_TIME       = 100

        ETH_HDR_LEN             = 14
        DEFAULT_MTU             = 1500
        DEFAULT_RX_BUF_LEN      = 1514


;ifdef   JUMBO_FRAME_SUPPORT
;        MAX_JUMBO_FRAME_MTU     = 10000
;        MAX_RX_SKBDATA_SIZE     = (MAX_JUMBO_FRAME_MTU + ETH_HDR_LEN )
;else
        MAX_RX_SKBDATA_SIZE     = 1600
;end if

        MCFG_METHOD_01          = 0x01
        MCFG_METHOD_02          = 0x02
        MCFG_METHOD_03          = 0x03
        MCFG_METHOD_04          = 0x04
        MCFG_METHOD_05          = 0x05
        MCFG_METHOD_11          = 0x0b
        MCFG_METHOD_12          = 0x0c
        MCFG_METHOD_13          = 0x0d
        MCFG_METHOD_14          = 0x0e
        MCFG_METHOD_15          = 0x0f

        PCFG_METHOD_1           = 0x01          ; PHY Reg 0x03 bit0-3 == 0x0000
        PCFG_METHOD_2           = 0x02          ; PHY Reg 0x03 bit0-3 == 0x0001
        PCFG_METHOD_3           = 0x03          ; PHY Reg 0x03 bit0-3 == 0x0002

struct  tx_desc
        status    dd ?
        vlan_tag  dd ?
        buf_addr  dq ?
ends
        tx_desc.buf_soft_addr = NUM_TX_DESC*sizeof.tx_desc

struct  rx_desc
        status    dd ?
        vlan_tag  dd ?
        buf_addr  dq ?
ends
        rx_desc.buf_soft_addr = NUM_RX_DESC*sizeof.rx_desc

struct  device          ETH_DEVICE

        io_addr         dd ?
        pci_bus         dd ?
        pci_dev         dd ?
        irq_line        db ?
                        rb 3 ; align 4
        pcfg            dd ?
        mcfg            dd ?

        cur_rx          dd ? ; Index into the Rx descriptor buffer of next Rx pkt
        cur_tx          dd ? ; Index into the Tx descriptor buffer of next Rx pkt
        last_tx         dd ?
        mac_version     dd ?

        rb 0x100-($ and 0xff)   ; align 256
        tx_ring         rb NUM_TX_DESC * sizeof.tx_desc * 2

        rb 0x100-($ and 0xff)   ; align 256
        rx_ring         rb NUM_RX_DESC * sizeof.rx_desc * 2

ends

        intr_mask = ISB_LinkChg or ISB_RxOverflow or ISB_RxFIFOOver or ISB_TxErr or ISB_TxOK or ISB_RxErr or ISB_RxOK
        rx_config = (RX_FIFO_THRESH shl RXC_FIFOShift) or (RX_DMA_BURST shl RXC_DMAShift) or 0x0000000E


macro   udelay msec {

        push    esi ecx
        mov     esi, msec
        invoke  Sleep
        pop     ecx esi

}

macro   WRITE_GMII_REG  RegAddr, value {

        set_io  [ebx + device.io_addr], REG_PHYAR
        if      value eq ax
        and     eax, 0x0000ffff
        or      eax, 0x80000000 + (RegAddr shl 16)
        else
        mov     eax, 0x80000000 + (RegAddr shl 16) + value
        end if
        out     dx, eax

        call    PHY_WAIT_WRITE
}

macro   READ_GMII_REG  RegAddr {

local   .error, .done

        set_io  [ebx + device.io_addr], REG_PHYAR
        mov     eax, RegAddr shl 16
        out     dx, eax

        call    PHY_WAIT_READ
        jz      .error

        in      eax, dx
        and     eax, 0xFFFF
        jmp     .done

  .error:
        or      eax, -1
  .done:
}

align 4
PHY_WAIT_READ:       ; io addr must already be set to REG_PHYAR

        udelay  1        ;;;1000

        push    ecx
        mov     ecx, 2000
        ; Check if the RTL8169 has completed writing/reading to the specified MII register
    @@:
        in      eax, dx
        test    eax, 0x80000000
        jnz     .exit
        udelay  1        ;;;100
        loop    @b
  .exit:
        pop     ecx
        ret

align 4
PHY_WAIT_WRITE:       ; io addr must already be set to REG_PHYAR

        udelay  1        ;;;1000

        push    ecx
        mov     ecx, 2000
        ; Check if the RTL8169 has completed writing/reading to the specified MII register
    @@:
        in      eax, dx
        test    eax, 0x80000000
        jz      .exit
        udelay  1        ;;;100
        loop    @b
  .exit:
        pop     ecx
        ret



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

;        mov     eax, [edx + IOCTL.input]                ; get the pci bus and device numbers
        mov     ax, [eax+1]                             ;
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

        allocate_and_clear ebx, sizeof.device, .fail    ; Allocate memory to put the device structure in

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

; Now, it's time to find the base io addres of the PCI device

        stdcall PCI_find_io, [ebx + device.pci_bus], [ebx + device.pci_dev]
        mov     [ebx + device.io_addr], eax

; We've found the io address, find IRQ now

        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.interrupt_line
        mov     [ebx + device.irq_line], al

        DEBUGF  2,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [ebx + device.pci_dev]:1,[ebx + device.pci_bus]:1,[ebx + device.irq_line]:1,[ebx + device.io_addr]:8

; Ok, the eth_device structure is ready, let's probe the device
; Because initialization fires IRQ, IRQ handler must be aware of this device
        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list + 4*eax], ebx                              ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err2                                                   ; If an error occured, exit

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
        DEBUGF  2,"removing device structure\n"
        invoke  KernelFree, ebx
  .fail:
        or      eax, -1
        ret

;------------------------------------------------------
endp


align 4
unload:

        ret


align 4
init_board:

        DEBUGF  1,"init_board\n"

; Make the device a bus master
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER or PCI_CMD_PIO
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

        ; Soft reset the chip
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_ChipCmd
        mov     al, CMD_Reset
        out     dx, al

        ; Check that the chip has finished the reset
        mov     ecx, 1000
        set_io  [ebx + device.io_addr], REG_ChipCmd
  @@:
        in      al, dx
        test    al, CMD_Reset
        jz      @f
        udelay  10
        loop    @b
        DEBUGF  2,"chip reset timeout\n"
        or      eax, -1
        ret
  @@:


        set_io  [ebx + device.io_addr], REG_TxConfig
        in      eax, dx
        mov     esi, MAC_VERSION_LIST
  @@:
        mov     ecx, eax
        and     ecx, dword[esi]
        cmp     ecx, dword[esi+4]
        je      @f
        add     esi, 4*4
        jmp     @r
  @@:

        mov     ecx, [esi+8]
        mov     [ebx + device.mac_version], ecx
        mov     ecx, [esi+12]
        mov     [ebx + device.name], ecx
        DEBUGF  2, "Detected chip: %s\n", ecx
        cmp     dword[esi], 0
        jne     @f
        DEBUGF  1, "TxConfig = 0x%x\n", eax
  @@:

        xor     eax, eax
        ret



;***************************************************************************
;   Function
;      probe
;   Description
;      Searches for an ethernet card, enables it and clears the rx buffer
;      If a card was found, it enables the ethernet -> TCPIP link
;   Destroyed registers
;      eax, ebx, ecx, edx
;
;***************************************************************************
align 4
probe:

        DEBUGF  1,"probe\n"

        call    init_board
        call    read_mac
        call    PHY_config

        DEBUGF  1,"Set MAC Reg C+CR Offset 0x82h = 0x01h\n"
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], 0x82
        mov     al, 0x01
        out     dx, al
        cmp     [ebx + device.mcfg], MCFG_METHOD_03
        jae     @f
        DEBUGF  1,"Set PCI Latency=0x40\n"
; Adjust PCI latency to be at least 64
        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.max_latency
        cmp     al, 64
        jae     @f
        mov     al, 64
        invoke  PciWrite8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.max_latency, eax
  @@:
        cmp     [ebx + device.mcfg], MCFG_METHOD_02
        jne     @f
        DEBUGF  1,"Set MAC Reg C+CR Offset 0x82h = 0x01h\n"
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], 0x82
        mov     al, 0x01
        out     dx, al
        DEBUGF  1,"Set PHY Reg 0x0bh = 0x00h\n"
        WRITE_GMII_REG 0x0b, 0x0000      ; w 0x0b 15 0 0
    @@:
        ; if TBI is not enabled
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_PHYstatus
        in      al, dx
        test    al, PHYS_TBI_Enable
        jz      .tbi_dis
        READ_GMII_REG PHY_AUTO_NEGO_REG

        ; enable 10/100 Full/Half Mode, leave PHY_AUTO_NEGO_REG bit4:0 unchanged
        and     eax, 0x0C1F
        or      eax, PHY_Cap_10_Half or PHY_Cap_10_Full or PHY_Cap_100_Half or PHY_Cap_100_Full
        WRITE_GMII_REG PHY_AUTO_NEGO_REG, ax

        ; enable 1000 Full Mode
        WRITE_GMII_REG PHY_1000_CTRL_REG, PHY_Cap_1000_Full or PHY_Cap_1000_Half ; rtl8168

        ; Enable auto-negotiation and restart auto-nigotiation
        WRITE_GMII_REG PHY_CTRL_REG, PHY_Enable_Auto_Nego or PHY_Restart_Auto_Nego

        udelay  1                       ; 100
        mov     ecx, 200                ; 10000
        DEBUGF  1, "Waiting for auto-negotiation to complete\n"
        ; wait for auto-negotiation process
    @@: dec     ecx
        jz      @f
        set_io  [ebx + device.io_addr], 0
        READ_GMII_REG PHY_STAT_REG
        udelay  1                       ; 100
        test    eax, PHY_Auto_Neco_Comp
        jz      @b
        set_io  [ebx + device.io_addr], REG_PHYstatus
        in      al, dx
        jmp     @f
  .tbi_dis:
        udelay  1                       ; 100
    @@:
        DEBUGF  1, "auto-negotiation complete\n"

;***************************************************************************
;   Function
;      rt8169_reset
;   Description
;      Place the chip (ie, the ethernet card) into a virgin state
;   Destroyed registers
;      eax, ebx, ecx, edx
;
;***************************************************************************
align 4
reset:

        DEBUGF  1,"resetting\n"

        call    init_ring
        test    eax, eax
        jnz     .err

        call    hw_start

; clear packet/byte counters

        xor     eax, eax
        lea     edi, [ebx + device.bytes_tx]
        mov     ecx, 6
        rep     stosd

        mov     [ebx + device.mtu], 1500
        call    detect_link

        DEBUGF  2,"init OK!\n"
        xor     eax, eax
        ret

  .err:
        DEBUGF  2,"reset failed!\n"
        or      eax, -1
        ret



align 4
PHY_config:

        DEBUGF  1,"hw_PHY_config: priv.mcfg=%d, priv.pcfg=%d\n", [ebx + device.mcfg], [ebx + device.pcfg]

        cmp     [ebx + device.mcfg], MCFG_METHOD_04
        jne     .not_4
        set_io  [ebx + device.io_addr], 0
;       WRITE_GMII_REG 0x1F, 0x0001
;       WRITE_GMII_REG 0x1b, 0x841e
;       WRITE_GMII_REG 0x0e, 0x7bfb
;       WRITE_GMII_REG 0x09, 0x273a
        WRITE_GMII_REG 0x1F, 0x0002
        WRITE_GMII_REG 0x01, 0x90D0
        WRITE_GMII_REG 0x1F, 0x0000
        jmp     .exit
  .not_4:
        cmp     [ebx + device.mcfg], MCFG_METHOD_02
        je      @f
        cmp     [ebx + device.mcfg], MCFG_METHOD_03
        jne     .not_2_or_3
    @@:
        set_io  [ebx + device.io_addr], 0
        WRITE_GMII_REG 0x1F, 0x0001
        WRITE_GMII_REG 0x15, 0x1000
        WRITE_GMII_REG 0x18, 0x65C7
        WRITE_GMII_REG 0x04, 0x0000
        WRITE_GMII_REG 0x03, 0x00A1
        WRITE_GMII_REG 0x02, 0x0008
        WRITE_GMII_REG 0x01, 0x1020
        WRITE_GMII_REG 0x00, 0x1000
        WRITE_GMII_REG 0x04, 0x0800
        WRITE_GMII_REG 0x04, 0x0000
        WRITE_GMII_REG 0x04, 0x7000
        WRITE_GMII_REG 0x03, 0xFF41
        WRITE_GMII_REG 0x02, 0xDE60
        WRITE_GMII_REG 0x01, 0x0140
        WRITE_GMII_REG 0x00, 0x0077
        WRITE_GMII_REG 0x04, 0x7800
        WRITE_GMII_REG 0x04, 0x7000
        WRITE_GMII_REG 0x04, 0xA000
        WRITE_GMII_REG 0x03, 0xDF01
        WRITE_GMII_REG 0x02, 0xDF20
        WRITE_GMII_REG 0x01, 0xFF95
        WRITE_GMII_REG 0x00, 0xFA00
        WRITE_GMII_REG 0x04, 0xA800
        WRITE_GMII_REG 0x04, 0xA000
        WRITE_GMII_REG 0x04, 0xB000
        WRITE_GMII_REG 0x03, 0xFF41
        WRITE_GMII_REG 0x02, 0xDE20
        WRITE_GMII_REG 0x01, 0x0140
        WRITE_GMII_REG 0x00, 0x00BB
        WRITE_GMII_REG 0x04, 0xB800
        WRITE_GMII_REG 0x04, 0xB000
        WRITE_GMII_REG 0x04, 0xF000
        WRITE_GMII_REG 0x03, 0xDF01
        WRITE_GMII_REG 0x02, 0xDF20
        WRITE_GMII_REG 0x01, 0xFF95
        WRITE_GMII_REG 0x00, 0xBF00
        WRITE_GMII_REG 0x04, 0xF800
        WRITE_GMII_REG 0x04, 0xF000
        WRITE_GMII_REG 0x04, 0x0000
        WRITE_GMII_REG 0x1F, 0x0000
        WRITE_GMII_REG 0x0B, 0x0000
        jmp     .exit
  .not_2_or_3:
        DEBUGF  1,"mcfg=%d, discard hw PHY config\n", [ebx + device.mcfg]
  .exit:
        ret



align 4
set_rx_mode:

        DEBUGF  1,"set_rx_mode\n"

        ; IFF_ALLMULTI
        ; Too many to filter perfectly -- accept all multicasts
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_RxConfig
        in      eax, dx
        and     eax, 0xff7e1880
        or      eax, rx_config or (RXM_AcceptBroadcast or RXM_AcceptMulticast or RXM_AcceptMyPhys)
        out     dx, eax

        ; Multicast hash filter
        set_io  [ebx + device.io_addr], REG_MAR0 + 0
        or      eax, -1
        out     dx, eax
        set_io  [ebx + device.io_addr], REG_MAR0 + 4
        out     dx, eax

        ret


align 4
init_ring:

        DEBUGF  1,"init_ring\n"

        xor     eax, eax
        mov     [ebx + device.cur_rx], eax
        mov     [ebx + device.cur_tx], eax
        mov     [ebx + device.last_tx], eax

        lea     edi, [ebx + device.tx_ring]
        mov     ecx, (NUM_TX_DESC * sizeof.tx_desc) / 4 * 2
        rep     stosd

        lea     edi, [ebx + device.rx_ring]
        mov     ecx, (NUM_RX_DESC * sizeof.rx_desc) / 4
        rep     stosd

        lea     edi, [ebx + device.rx_ring]
        mov     ecx, NUM_RX_DESC
  .loop:
        push    ecx
        invoke  NetAlloc, RX_BUF_SIZE+NET_BUFF.data
        test    eax, eax
        jz      .err
        mov     dword [edi + rx_desc.buf_soft_addr], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     dword [edi + rx_desc.buf_addr], eax
        mov     [edi + rx_desc.status], DSB_OWNbit or RX_BUF_SIZE
        add     edi, sizeof.rx_desc
        pop     ecx
        dec     ecx
        jnz     .loop
        or      [edi - sizeof.rx_desc + rx_desc.status], DSB_EORbit

        xor     eax, eax
        ret

  .err:
        pop     eax
        or      eax, -1
        ret

align 4
hw_start:

        DEBUGF  1,"hw_start\n"

; attach int handler
        movzx   eax, [ebx + device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n", eax:1
        invoke  AttachIntHandler, eax, int_handler, ebx
        test    eax, eax
        jnz     @f
        DEBUGF  2,"Could not attach int handler!\n"
        or      eax, -1
        ret
       @@:

        ; Soft reset the chip
        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_ChipCmd
        mov     al, CMD_Reset
        out     dx, al

        DEBUGF  1,"Waiting for chip to reset... "
        ; Check that the chip has finished the reset
        mov     ecx, 1000
        set_io  [ebx + device.io_addr], REG_ChipCmd
    @@: in      al, dx
        test    al, CMD_Reset
        jz      @f
        udelay  10
        loop    @b
    @@:
        DEBUGF  1,"done!\n"

        set_io  [ebx + device.io_addr], REG_Cfg9346
        mov     al, CFG_9346_Unlock
        out     dx, al

        set_io  [ebx + device.io_addr], REG_ChipCmd
        mov     al, CMD_TxEnb or CMD_RxEnb
        out     dx, al

        set_io  [ebx + device.io_addr], REG_ETThReg
        mov     al, ETTh
        out     dx, al

        ; For gigabit rtl8169
        set_io  [ebx + device.io_addr], REG_RxMaxSize
        mov     ax, RxPacketMaxSize
        out     dx, ax

        ; Set Rx Config register
        set_io  [ebx + device.io_addr], REG_RxConfig
        in      ax, dx
        and     eax, 0xff7e1880
        or      eax, rx_config
        out     dx, eax

        ; Set DMA burst size and Interframe Gap Time
        set_io  [ebx + device.io_addr], REG_TxConfig
        mov     eax, (TX_DMA_BURST shl TXC_DMAShift) or (InterFrameGap shl TXC_InterFrameGapShift)
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_CPlusCmd
        in      ax, dx
        out     dx, ax

        in      ax, dx
        or      ax, 1 shl 3
        cmp     [ebx + device.mcfg], MCFG_METHOD_02
        jne     @f
        cmp     [ebx + device.mcfg], MCFG_METHOD_03
        jne     @f
        or      ax,1 shl 14
        DEBUGF  1,"Set MAC Reg C+CR Offset 0xE0: bit-3 and bit-14\n"
        jmp     .set
    @@:
        DEBUGF  1,"Set MAC Reg C+CR Offset 0xE0: bit-3\n"
  .set:
        set_io  [ebx + device.io_addr], REG_CPlusCmd
        out     dx, ax

        set_io  [ebx + device.io_addr], 0xE2
;        mov     ax, 0x1517
;        out     dx, ax
;        mov     ax, 0x152a
;        out     dx, ax
;        mov     ax, 0x282a
;        out     dx, ax
        xor     ax, ax
        out     dx, ax

        xor     eax, eax
        mov     [ebx + device.cur_rx], eax
        lea     eax, [ebx + device.tx_ring]
        invoke  GetPhysAddr
        set_io  [ebx + device.io_addr], REG_TxDescStartAddr
        out     dx, eax
        set_io  [ebx + device.io_addr], REG_TxDescStartAddr + 4
        xor     eax, eax
        out     dx, eax

        lea     eax, [ebx + device.rx_ring]
        invoke  GetPhysAddr
        set_io  [ebx + device.io_addr], REG_RxDescStartAddr
        out     dx, eax
        xor     eax, eax
        set_io  [ebx + device.io_addr], REG_RxDescStartAddr + 4
        out     dx, eax

        set_io  [ebx + device.io_addr], REG_Cfg9346
        mov     al, CFG_9346_Lock
        out     dx, al

        udelay  10

        xor     eax, eax
        set_io  [ebx + device.io_addr], REG_RxMissed
        out     dx, eax

        call    set_rx_mode

        set_io  [ebx + device.io_addr], 0
        ; no early-rx interrupts
        set_io  [ebx + device.io_addr], REG_MultiIntr
        in      ax, dx
        and     ax, 0xF000
        out     dx, ax

        ; set interrupt mask
        set_io  [ebx + device.io_addr], REG_IntrMask
        mov     ax, intr_mask
        out     dx, ax

        xor     eax, eax
        ret


align 4
read_mac:

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_MAC0
        xor     ecx, ecx
        lea     edi, [ebx + device.mac]
        mov     ecx, 6

        ; Get MAC address. FIXME: read EEPROM
    @@:
        in      al, dx
        stosb
        inc     edx
        loop    @r

        DEBUGF  1,"MAC = %x-%x-%x-%x-%x-%x\n",\
        [ebx + device.mac+0]:2,[ebx + device.mac+1]:2,[ebx + device.mac+2]:2,[ebx + device.mac+3]:2,[ebx + device.mac+4]:2,[ebx + device.mac+5]:2

        ret

align 4
write_mac:

        ret     6


;***************************************************************************
;   Function
;      transmit
;   Description
;      Transmits a packet of data via the ethernet card
;
;   Destroyed registers
;      eax, edx, esi, edi
;
;***************************************************************************

proc transmit stdcall bufferptr

        pushf
        cli

        mov     esi, [bufferptr]
        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n", [bufferptr], [esi + NET_BUFF.length]
        lea     eax, [esi + NET_BUFF.data]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     [esi + NET_BUFF.length], 1514
        ja      .fail
        cmp     [esi + NET_BUFF.length], 60
        jb      .fail

;----------------------------------
; Find currentTX descriptor address

        mov     eax, sizeof.tx_desc
        mul     [ebx + device.cur_tx]
        lea     esi, [ebx + device.tx_ring + eax]

        DEBUGF  1,"Using TX desc: %x\n", esi

;----------------------------------
; Check if the descriptor is in use

        test    [esi + tx_desc.status], DSB_OWNbit
        jnz     .desc

;---------------------------
; Program the packet pointer

        mov     eax, [bufferptr]
        mov     ecx, [eax + NET_BUFF.length]
        mov     [esi + tx_desc.buf_soft_addr], eax
        add     eax, [eax + NET_BUFF.offset]
        invoke  GetPhysAddr
        mov     dword [esi + tx_desc.buf_addr], eax

;------------------------
; Program the packet size

        mov     eax, ecx
        or      eax, DSB_OWNbit or DSB_FSbit or DSB_LSbit
        cmp     [ebx + device.cur_tx], NUM_TX_DESC - 1
        jne     @f
        or      eax, DSB_EORbit
    @@:
        mov     [esi + tx_desc.status], eax

;-----------------------------------------
; Set the polling bit (start transmission)

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_TxPoll
        mov     al, 0x40     ; set polling bit
        out     dx, al

;-----------------------
; Update TX descriptor

        inc     [ebx + device.cur_tx]
        and     [ebx + device.cur_tx], NUM_TX_DESC - 1

;-------------
; Update stats

        inc     [ebx + device.packets_tx]
        add     dword [ebx + device.bytes_tx], ecx
        adc     dword [ebx + device.bytes_tx + 4], 0

        popf
        xor     eax, eax
        ret

  .desc:
        DEBUGF  2,"Descriptor is still in use!\n"
  .fail:
        DEBUGF  2,"Transmit failed\n"
        invoke  NetFree, [bufferptr]
        popf
        or      eax, -1
        ret

endp



;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

        push    ebx esi edi

        DEBUGF  1,"INT\n"

; find pointer of device wich made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

        set_io  [ebx + device.io_addr], 0
        set_io  [ebx + device.io_addr], REG_IntrStatus
        in      ax, dx
        out     dx, ax                                  ; ACK all interrupts
        cmp     ax, 0xffff                              ; if so, hardware is no longer present
        je      .nothing
        test    ax, ax
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret                                             ; If no device was found, abort (The irq was probably for a device, not registered to this driver)

  .got_it:
        DEBUGF  1,"Device: %x Status: %x\n", ebx, ax

;--------
; Receive
        test    ax, ISB_RxOK
        jz      .no_rx

        push    ax
        push    ebx

  .rx_loop:
        pop     ebx
        mov     eax, sizeof.rx_desc
        mul     [ebx + device.cur_rx]
        lea     esi, [ebx + device.rx_ring + eax]

        DEBUGF  1,"RxDesc.status = 0x%x\n", [esi + rx_desc.status]
        mov     ecx, [esi + rx_desc.status]
        test    ecx, DSB_OWNbit
        jnz     .rx_return

        DEBUGF  1,"cur_rx = %u\n", [ebx + device.cur_rx]
        test    ecx, SD_RxRES
        jnz     .rx_reuse

        push    ebx
        push    .rx_loop
        and     ecx, 0x00001FFF
        add     ecx, -4                         ; we dont need CRC
        DEBUGF  1,"data length = %u\n", ecx
        mov     eax, [esi + rx_desc.buf_soft_addr]
        push    eax
        mov     [eax + NET_BUFF.length], ecx
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data

;-------------
; Update stats

        add     dword [ebx + device.bytes_rx], ecx
        adc     dword [ebx + device.bytes_rx + 4], 0
        inc     [ebx + device.packets_rx]

;----------------------
; Allocate a new buffer

        mov     [esi + rx_desc.status], 0
        invoke  NetAlloc, RX_BUF_SIZE+NET_BUFF.data
        test    eax, eax
        jz      .no_more_buffers
        mov     [esi + rx_desc.buf_soft_addr], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     dword [esi + rx_desc.buf_addr], eax

;---------------
; re set OWN bit

        mov     eax, DSB_OWNbit or RX_BUF_SIZE
        cmp     [ebx + device.cur_rx], NUM_RX_DESC - 1
        jne     @f
        or      eax, DSB_EORbit
    @@:
        mov     [esi + rx_desc.status], eax

  .no_more_buffers:
;--------------
; Update rx ptr

        inc     [ebx + device.cur_rx]
        and     [ebx + device.cur_rx], NUM_RX_DESC - 1

        jmp     [EthInput]

  .rx_reuse:
        mov     eax, DSB_OWNbit or RX_BUF_SIZE
        cmp     [ebx + device.cur_rx], NUM_RX_DESC - 1
        jne     @f
        or      eax, DSB_EORbit
    @@:
        mov     [esi + rx_desc.status], eax
        push    ebx
        jmp     .rx_loop

  .rx_return:
        pop     ax
  .no_rx:

;-----------------
; Transmit cleanup

        test    ax, ISB_TxOK or ISB_TxErr or ISB_TxDescUnavail
        jz      .no_tx
        push    ax

        DEBUGF  1,"TX done!\n"

        mov     ecx, NUM_TX_DESC
        lea     esi, [ebx + device.tx_ring]
  .txloop:
        cmp     dword [esi + tx_desc.buf_soft_addr], 0
        jz      .maybenext

        test    [esi + tx_desc.status], DSB_OWNbit
        jnz     .maybenext

        push    ecx
        DEBUGF  1,"Freeing up TX desc: %x\n", esi
        invoke  NetFree, [esi + tx_desc.buf_soft_addr]
        pop     ecx
        and     dword [esi + tx_desc.buf_soft_addr], 0

  .maybenext:
        add     esi, sizeof.tx_desc
        dec     ecx
        jnz     .txloop

        pop     ax
  .no_tx:

        test    ax, ISB_LinkChg
        jz      .no_linkchange
        DEBUGF  2, "Link change detected\n"
        call    detect_link
  .no_linkchange:

        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret



align 4
detect_link:

        set_io  [ebx + device.io_addr], 0

;        set_io  [ebx + device.io_addr], REG_TBICSR
;        in      eax, dx
;        test    eax, TBI_LinkOK
;        jz      .down

;        mov     [ebx + device.state], ETH_LINK_UNKNOWN
;        invoke  NetLinkChanged
;        ret

        set_io  [ebx + device.io_addr], REG_PHYstatus
        in      al, dx
        test    al, PHYS_LinkStatus
        jz      .down
        DEBUGF  2, "Link is up, phystatus=0x%x\n", al
        xor     ecx, ecx
        test    al, PHYS_10bps
        jz      @f
        or      cl, ETH_LINK_10M
  @@:
        test    al, PHYS_100bps
        jz      @f
        or      cl, ETH_LINK_100M
  @@:
        test    al, PHYS_1000bpsF
        jz      @f
        or      cl, ETH_LINK_1G ;or ETH_LINK_FD
  @@:
        test    al, PHYS_FullDup
        jz      @f
        or      cl, ETH_LINK_FD
  @@:
        mov     [ebx + device.state], ecx
        invoke  NetLinkChanged
        ret

  .down:
        DEBUGF  2, "Link is down\n"
        mov     [ebx + device.state], ETH_LINK_DOWN
        invoke  NetLinkChanged
        ret



; End of code

data fixups
end data

include '../peimport.inc'

my_service    db 'RTL8169',0                    ; max 16 chars include zero

include_debug_strings                           ; All data wich FDO uses will be included here

MAC_VERSION_LIST:

; 8168E+ family
dd 0x7cf00000, 0x50200000, 51, sz_rtl8168ep
dd 0x7cf00000, 0x50100000, 50, sz_rtl8168ep
dd 0x7cf00000, 0x50000000, 49, sz_rtl8168ep

; 8168H family
dd 0x7cf00000, 0x54100000, 46, sz_rtl8168h
dd 0x7cf00000, 0x54000000, 45, sz_rtl8168h

; 8168G family
dd 0x7cf00000, 0x5c800000, 44, sz_rtl8411
dd 0x7cf00000, 0x50900000, 42, sz_rtl8168g
dd 0x7cf00000, 0x4c100000, 41, sz_rtl8168g
dd 0x7cf00000, 0x4c000000, 40, sz_rtl8168g

; 8168F family
dd 0x7c800000, 0x48800000, 38, sz_rtl8411
dd 0x7cf00000, 0x48100000, 36, sz_rtl8168f
dd 0x7cf00000, 0x48000000, 35, sz_rtl8168f

; 8168E family
dd 0x7c800000, 0x2c800000, 34, sz_rtl8168evl
dd 0x7cf00000, 0x2c200000, 33, sz_rtl8168e
dd 0x7cf00000, 0x2c100000, 32, sz_rtl8168e
dd 0x7c800000, 0x2c000000, 33, sz_rtl8168e

; 8168D family
dd 0x7cf00000, 0x28300000, 26, sz_rtl8168d
dd 0x7cf00000, 0x28100000, 25, sz_rtl8168d
dd 0x7c800000, 0x28000000, 26, sz_rtl8168d

; 8168D+ family
dd 0x7cf00000, 0x28800000, 27, sz_rtl8168dp
dd 0x7cf00000, 0x28a00000, 28, sz_rtl8168dp

; 8168C family
dd 0x7cf00000, 0x3cb00000, 24, sz_rtl8168p
dd 0x7cf00000, 0x3c900000, 23, sz_rtl8168p
dd 0x7cf00000, 0x3c800000, 18, sz_rtl8168p
dd 0x7c800000, 0x3c800000, 24, sz_rtl8168p
dd 0x7cf00000, 0x3c000000, 19, sz_rtl8168c
dd 0x7cf00000, 0x3c200000, 20, sz_rtl8168c
dd 0x7cf00000, 0x3c300000, 21, sz_rtl8168c
dd 0x7cf00000, 0x3c400000, 22, sz_rtl8168c
dd 0x7c800000, 0x3c000000, 22, sz_rtl8168c

; 8168B family
dd 0x7cf00000, 0x38000000, 12, sz_rtl8168b
dd 0x7cf00000, 0x38500000, 17, sz_rtl8101e
dd 0x7c800000, 0x38000000, 17, sz_rtl8101e
dd 0x7c800000, 0x30000000, 11, sz_rtl8168b

; 8101 family
dd 0x7cf00000, 0x44900000, 39, sz_rtl8106e
dd 0x7c800000, 0x44800000, 39, sz_rtl8106e
dd 0x7c800000, 0x44000000, 37, sz_rtl8402
dd 0x7cf00000, 0x40b00000, 30, sz_rtl8105e
dd 0x7cf00000, 0x40a00000, 30, sz_rtl8105e
dd 0x7cf00000, 0x40900000, 29, sz_rtl8105e
dd 0x7c800000, 0x40800000, 30, sz_rtl8105e
dd 0x7cf00000, 0x34a00000, 09, sz_rtl8102e
dd 0x7cf00000, 0x24a00000, 09, sz_rtl8102e
dd 0x7cf00000, 0x34900000, 08, sz_rtl8102e
dd 0x7cf00000, 0x24900000, 08, sz_rtl8102e
dd 0x7cf00000, 0x34800000, 07, sz_rtl8102e
dd 0x7cf00000, 0x24800000, 07, sz_rtl8102e
dd 0x7cf00000, 0x34000000, 13, sz_rtl8101e
dd 0x7cf00000, 0x34300000, 10, sz_rtl8101e
dd 0x7cf00000, 0x34200000, 16, sz_rtl8168b
dd 0x7c800000, 0x34800000, 09, sz_rtl8102e
dd 0x7c800000, 0x24800000, 09, sz_rtl8102e
dd 0x7c800000, 0x34000000, 16, sz_rtl8168b
dd 0xfc800000, 0x38800000, 15, sz_rtl8100e
dd 0xfc800000, 0x30800000, 14, sz_rtl8100e

; 8110 family
dd 0xfc800000, 0x98000000, 06, sz_rtl8169sc
dd 0xfc800000, 0x18000000, 05, sz_rtl8169sc
dd 0xfc800000, 0x10000000, 04, sz_rtl8169sb
dd 0xfc800000, 0x04000000, 03, sz_rtl8110s
dd 0xfc800000, 0x00800000, 02, sz_rtl8169s
dd 0xfc800000, 0x00000000, 01, sz_rtl8169

; Catch-all
dd 0x00000000, 0x00000000, 0, sz_unknown

sz_rtl8169      db "Realtek 8169",0
sz_rtl8169s     db "Realtek 8169s",0
sz_rtl8110s     db "Realtek 8110s",0
sz_rtl8169sb    db "Realtek 8169sb/8110sb",0
sz_rtl8169sc    db "Realtek 8169sc/8110sc",0
sz_rtl8102e     db "Realtek 8102e",0
sz_rtl8101e     db "Realtek 8101e",0
sz_rtl8168b     db "Realtek 8168b/8111b",0
sz_rtl8100e     db "Realtek 8100e",0
sz_rtl8168p     db "Realtek 8168c+/8111c+",0
sz_rtl8168c     db "Realtek 8168c/8111c",0
sz_rtl8168d     db "Realtek 8168d/8111d",0
sz_rtl8168dp    db "Realtek 8168d+/8111d+",0
sz_rtl8105e     db "Realtek 8105e",0
sz_rtl8168e     db "Realtek 8168e/8111e",0
sz_rtl8168evl   db "Realtek 8168evl/8111evl",0
sz_rtl8168f     db "Realtek 8168f/8111f",0
sz_rtl8402      db "Realtek 8402",0
sz_rtl8411      db "Realtek 8411",0
sz_rtl8106e     db "Realtek 8106e",0
sz_rtl8168ep    db "Realtek 8168e+",0
sz_rtl8168g     db "Realtek 8168g/8111g",0
sz_rtl8168h     db "Realtek 8168h/8111h",0
sz_unknown      db "unknown RTL8169 rev",0

align 4
devices         dd 0
device_list rd MAX_DEVICES                     ; This list contains all pointers to device structures the driver is handling


