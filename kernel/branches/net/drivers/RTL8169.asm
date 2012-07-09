;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  RTL8169 driver for KolibriOS                                   ;;
;;                                                                 ;;
;;  Copyright 2007 mike.dld,                                       ;;
;;   mike.dld@gmail.com                                            ;;
;;                                                                 ;;
;;  Version 0.1  11 February 2007                                  ;;
;;  Version 0.2  3 August 2010 - port to net branch by hidnplayr   ;;
;;  Version 0.3  31 Januari 2011 - bugfixes by hidnplayr           ;;
;;                                                                 ;;
;;  References:                                                    ;;
;;    r8169.c - linux driver (etherboot project)                   ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

        API_VERSION             =   0x01000100
        DRIVER_VERSION          =   5

        MAX_DEVICES             =   16

        DEBUG                   =   1
        __DEBUG__               =   1
        __DEBUG_LEVEL__         =   1

        NUM_TX_DESC             =   4
        NUM_RX_DESC             =   4

include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'
include 'netdrv.inc'

public START
public service_proc
public version


        REG_MAC0               =   0x0 ; Ethernet hardware address
        REG_MAR0               =   0x8 ; Multicast filter
        REG_TxDescStartAddr    =   0x20
        REG_TxHDescStartAddr   =   0x28
        REG_FLASH              =   0x30
        REG_ERSR               =   0x36
        REG_ChipCmd            =   0x37
        REG_TxPoll             =   0x38
        REG_IntrMask           =   0x3C
        REG_IntrStatus         =   0x3E
        REG_TxConfig           =   0x40
        REG_RxConfig           =   0x44
        REG_RxMissed           =   0x4C
        REG_Cfg9346            =   0x50
        REG_Config0            =   0x51
        REG_Config1            =   0x52
        REG_Config2            =   0x53
        REG_Config3            =   0x54
        REG_Config4            =   0x55
        REG_Config5            =   0x56
        REG_MultiIntr          =   0x5C
        REG_PHYAR              =   0x60
        REG_TBICSR             =   0x64
        REG_TBI_ANAR           =   0x68
        REG_TBI_LPAR           =   0x6A
        REG_PHYstatus          =   0x6C
        REG_RxMaxSize          =   0xDA
        REG_CPlusCmd           =   0xE0
        REG_RxDescStartAddr    =   0xE4
        REG_ETThReg            =   0xEC
        REG_FuncEvent          =   0xF0
        REG_FuncEventMask      =   0xF4
        REG_FuncPresetState    =   0xF8
        REG_FuncForceEvent     =   0xFC

        ; InterruptStatusBits
        ISB_SYSErr             =   0x8000
        ISB_PCSTimeout         =   0x4000
        ISB_SWInt              =   0x0100
        ISB_TxDescUnavail      =   0x80
        ISB_RxFIFOOver         =   0x40
        ISB_LinkChg            =   0x20
        ISB_RxOverflow         =   0x10
        ISB_TxErr              =   0x08
        ISB_TxOK               =   0x04
        ISB_RxErr              =   0x02
        ISB_RxOK               =   0x01

        ; RxStatusDesc
        SD_RxRES               =   0x00200000
        SD_RxCRC               =   0x00080000
        SD_RxRUNT              =   0x00100000
        SD_RxRWT               =   0x00400000

        ; ChipCmdBits
        CMD_Reset              =   0x10
        CMD_RxEnb              =   0x08
        CMD_TxEnb              =   0x04
        CMD_RxBufEmpty         =   0x01

        ; Cfg9346Bits
        CFG_9346_Lock          =   0x00
        CFG_9346_Unlock        =   0xC0

        ; rx_mode_bits
        RXM_AcceptErr          =   0x20
        RXM_AcceptRunt         =   0x10
        RXM_AcceptBroadcast    =   0x08
        RXM_AcceptMulticast    =   0x04
        RXM_AcceptMyPhys       =   0x02
        RXM_AcceptAllPhys      =   0x01

        ; RxConfigBits
        RXC_FIFOShift          =   13
        RXC_DMAShift           =   8

        ; TxConfigBits
        TXC_InterFrameGapShift =   24
        TXC_DMAShift           =   8    ; DMA burst value (0-7) is shift this many bits

        ; PHYstatus
        PHYS_TBI_Enable        =   0x80
        PHYS_TxFlowCtrl        =   0x40
        PHYS_RxFlowCtrl        =   0x20
        PHYS_1000bpsF          =   0x10
        PHYS_100bps            =   0x08
        PHYS_10bps             =   0x04
        PHYS_LinkStatus        =   0x02
        PHYS_FullDup           =   0x01

        ; GIGABIT_PHY_registers
        PHY_CTRL_REG           =   0
        PHY_STAT_REG           =   1
        PHY_AUTO_NEGO_REG      =   4
        PHY_1000_CTRL_REG      =   9

        ; GIGABIT_PHY_REG_BIT
        PHY_Restart_Auto_Nego  =   0x0200
        PHY_Enable_Auto_Nego   =   0x1000

        ; PHY_STAT_REG = 1;
        PHY_Auto_Neco_Comp     =   0x0020

        ; PHY_AUTO_NEGO_REG = 4;
        PHY_Cap_10_Half        =   0x0020
        PHY_Cap_10_Full        =   0x0040
        PHY_Cap_100_Half       =   0x0080
        PHY_Cap_100_Full       =   0x0100

        ; PHY_1000_CTRL_REG = 9;
        PHY_Cap_1000_Full      =   0x0200
        PHY_Cap_1000_Half      =   0x0100

        PHY_Cap_PAUSE          =   0x0400
        PHY_Cap_ASYM_PAUSE     =   0x0800

        PHY_Cap_Null           =   0x0

        ; _MediaType
        MT_10_Half             =   0x01
        MT_10_Full             =   0x02
        MT_100_Half            =   0x04
        MT_100_Full            =   0x08
        MT_1000_Full           =   0x10

        ; _TBICSRBit
        TBI_LinkOK             =   0x02000000

        ; _DescStatusBit
        DSB_OWNbit             =   0x80000000
        DSB_EORbit             =   0x40000000
        DSB_FSbit              =   0x20000000
        DSB_LSbit              =   0x10000000

        RX_BUF_SIZE             =   1536    ; Rx Buffer size


ETH_ALEN               =   6
ETH_HLEN               =   (2 * ETH_ALEN + 2)
ETH_ZLEN               =   60 ; 60 + 4bytes auto payload for
                                      ; mininmum 64bytes frame length

; MAC address length
MAC_ADDR_LEN        =   6

; max supported gigabit ethernet frame size -- must be at least (dev->mtu+14+4)
MAX_ETH_FRAME_SIZE  =   1536

TX_FIFO_THRESH      =   256     ; In bytes

RX_FIFO_THRESH      =   7       ; 7 means NO threshold, Rx buffer level before first PCI xfer
RX_DMA_BURST        =   7       ; Maximum PCI burst, '6' is 1024
TX_DMA_BURST        =   7       ; Maximum PCI burst, '6' is 1024
ETTh                =   0x3F    ; 0x3F means NO threshold

EarlyTxThld         =   0x3F    ; 0x3F means NO early transmit
RxPacketMaxSize     =   0x0800  ; Maximum size supported is 16K-1
InterFrameGap       =   0x03    ; 3 means InterFrameGap = the shortest one

HZ                  =   1000

RTL_MIN_IO_SIZE     =   0x80
TX_TIMEOUT          =   (6*HZ)

TIMER_EXPIRE_TIME =   100

ETH_HDR_LEN         =   14
DEFAULT_MTU         =   1500
DEFAULT_RX_BUF_LEN  =   1536


;#ifdef JUMBO_FRAME_SUPPORT
;#define MAX_JUMBO_FRAME_MTU    ( 10000 )
;#define MAX_RX_SKBDATA_SIZE    ( MAX_JUMBO_FRAME_MTU + ETH_HDR_LEN )
;#else
MAX_RX_SKBDATA_SIZE =   1600
;#endif                         //end #ifdef JUMBO_FRAME_SUPPORT

MCFG_METHOD_01       =   0x01
MCFG_METHOD_02       =   0x02
MCFG_METHOD_03       =   0x03
MCFG_METHOD_04       =   0x04
MCFG_METHOD_05       =   0x05
MCFG_METHOD_11       =   0x0b
MCFG_METHOD_12       =   0x0c
MCFG_METHOD_13       =   0x0d
MCFG_METHOD_14       =   0x0e
MCFG_METHOD_15       =   0x0f

PCFG_METHOD_1       =   0x01    ; PHY Reg 0x03 bit0-3 == 0x0000
PCFG_METHOD_2       =   0x02    ; PHY Reg 0x03 bit0-3 == 0x0001
PCFG_METHOD_3       =   0x03    ; PHY Reg 0x03 bit0-3 == 0x0002

virtual at 0
  tx_desc:
  .status    dd ?
  .vlan_tag  dd ?
  .buf_addr  dq ?
  .size = $
  rb    (NUM_TX_DESC-1)*tx_desc.size
  .buf_soft_addr        dd ?
end virtual

virtual at 0
  rx_desc:
  .status    dd ?
  .vlan_tag  dd ?
  .buf_addr  dq ?
  .size = $
  rb    (NUM_RX_DESC-1)*rx_desc.size
  .buf_soft_addr        dd ?
end virtual

virtual at ebx

        device:

        ETH_DEVICE

        .io_addr        dd ?
        .pci_bus        db ?
        .pci_dev        db ?
        .irq_line       db ?

        tpc:
        .mmio_addr      dd ? ; memory map physical address
        .chipset        dd ?
        .pcfg           dd ?
        .mcfg           dd ?
        .cur_rx         dd ? ; Index into the Rx descriptor buffer of next Rx pkt
        .cur_tx         dd ? ; Index into the Tx descriptor buffer of next Rx pkt
        .TxDescArrays   dd ? ; Index of Tx Descriptor buffer
        .RxDescArrays   dd ? ; Index of Rx Descriptor buffer
        .TxDescArray    dd ? ; Index of 256-alignment Tx Descriptor buffer
        .RxDescArray    dd ? ; Index of 256-alignment Rx Descriptor buffer

        rb 256-(($ - device) and 255)              ;        align 256
        tx_ring rb NUM_TX_DESC * tx_desc.size * 2

        rb 256-(($ - device) and 255)              ;        align 256
        rx_ring rb NUM_RX_DESC * rx_desc.size * 2

        device_size = $ - device

end virtual

intr_mask = ISB_LinkChg or ISB_RxOverflow or ISB_RxFIFOOver or ISB_TxErr or ISB_TxOK or ISB_RxErr or ISB_RxOK
rx_config = (RX_FIFO_THRESH shl RXC_FIFOShift) or (RX_DMA_BURST shl RXC_DMAShift) or 0x0000000E


macro   udelay msec {

        push    esi
        mov     esi, msec
        call    Sleep
        pop     esi

}

macro   WRITE_GMII_REG  RegAddr, value {

        set_io  REG_PHYAR
        if      value eq ax
        and     eax, 0x0000ffff
        or      eax, 0x80000000 + (RegAddr shl 16)
        else
        mov     eax, 0x80000000 + (RegAddr shl 16) + value
        end if
        out     dx, eax

        call    PHY_WAIT
}

macro   READ_GMII_REG  RegAddr {

local   .error, .done

        set_io  REG_PHYAR
        mov     eax, RegAddr shl 16
        out     dx, eax

        call    PHY_WAIT
        jz      .error

        in      eax, dx
        and     eax, 0xFFFF
        jmp     .done

  .error:
        or      eax, -1
  .done:
}

align 4
PHY_WAIT:       ; io addr must already be set to REG_PHYAR

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

        DEBUGF  2,"Loading rtl8169 driver\n"
        stdcall RegService, my_service, service_proc
        ret

  .fail:
  .exit:
        xor eax, eax
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
        mov     eax, [IOCTL.io_code]

;------------------------------------------------------

        cmp     eax, 0 ;SRV_GETVERSION
        jne     @F

        cmp     [IOCTL.out_size], 4
        jl      .fail
        mov     eax, [IOCTL.output]
        mov     [eax], dword API_VERSION

        xor     eax, eax
        ret

;------------------------------------------------------
  @@:
        cmp     eax, 1 ;SRV_HOOK
        jne     .fail

        cmp     [IOCTL.inp_size], 3                     ; Data input must be at least 3 bytes
        jl      .fail

        mov     eax, [IOCTL.input]
        cmp     byte [eax], 1                           ; 1 means device number and bus number (pci) are given
        jne     .fail                                   ; other types arent supported for this card yet

; check if the device is already listed

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

;        mov     eax, [IOCTL.input]                     ; get the pci bus and device numbers
        mov     ax , [eax+1]                            ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     ax , word [device.pci_bus]              ; compare with pci and device num in device list (notice the usage of word instead of byte)
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
        add     esi, 4
        loop    .nextdevice


; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
        cmp     [devices], MAX_DEVICES                  ; First check if the driver can handle one more card
        jge     .fail

        allocate_and_clear ebx, device_size, .fail      ; Allocate memory to put the device structure in

; Fill in the direct call addresses into the struct

        mov     [device.reset], reset
        mov     [device.transmit], transmit
        mov     [device.get_MAC], read_mac
        mov     [device.set_MAC], write_mac
        mov     [device.unload], unload
        mov     [device.name], my_service

; save the pci bus and device numbers

        mov     eax, [IOCTL.input]
        mov     cl , [eax+1]
        mov     [device.pci_bus], cl
        mov     cl , [eax+2]
        mov     [device.pci_dev], cl

; Now, it's time to find the base io addres of the PCI device

        find_io [device.pci_bus], [device.pci_dev], [device.io_addr]
        mov     eax, [device.io_addr]
        mov     [tpc.mmio_addr], eax

; We've found the io address, find IRQ now

        find_irq [device.pci_bus], [device.pci_dev], [device.irq_line]

        DEBUGF  2,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [device.pci_dev]:1,[device.pci_bus]:1,[device.irq_line]:1,[device.io_addr]:8

; Ok, the eth_device structure is ready, let's probe the device
; Because initialization fires IRQ, IRQ handler must be aware of this device
        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err2                                                   ; If an error occured, exit


        mov     [device.type], NET_TYPE_ETH
        call    NetRegDev

        cmp     eax, -1
        je      .destroy

        ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  2,"Trying to find device number of already registered device\n"
        call    NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
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
        stdcall KernelFree, ebx


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

        make_bus_master [device.pci_bus], [device.pci_dev]

        ; Soft reset the chip
        set_io  0
        set_io  REG_ChipCmd
        mov     al, CMD_Reset
        out     dx, al

        ; Check that the chip has finished the reset
        mov     ecx, 1000
        set_io  REG_ChipCmd
    @@: in      al, dx
        test    al, CMD_Reset
        jz      @f
        udelay  10
        loop    @b
    @@:
        ; identify config method
        set_io  REG_TxConfig
        in      eax, dx
        and     eax, 0x7c800000
        DEBUGF  1,"init_board: TxConfig & 0x7c800000 = 0x%x\n", eax
        mov     esi, mac_info-8
    @@: add     esi, 8
        mov     ecx, eax
        and     ecx, [esi]
        cmp     ecx, [esi]
        jne     @b
        mov     eax, [esi+4]
        mov     [tpc.mcfg], eax

        mov     [tpc.pcfg], PCFG_METHOD_3
        READ_GMII_REG 3
        and     al, 0x0f
        or      al, al
        jnz     @f
        mov     [tpc.pcfg], PCFG_METHOD_1
        jmp     .pconf
    @@: dec     al
        jnz     .pconf
        mov     [tpc.pcfg], PCFG_METHOD_2
  .pconf:

        ; identify chip attached to board
        mov     ecx, 10
        mov     eax, [tpc.mcfg]
    @@: dec     ecx
        js      @f
        cmp     eax, [rtl_chip_info+ecx*8]
        jne     @b
        mov     [tpc.chipset], ecx
        jmp     .match
    @@:
        ; if unknown chip, assume array element #0, original RTL-8169 in this case
        DEBUGF  1,"init_board: PCI device: unknown chip version, assuming RTL-8169\n"
        set_io  REG_TxConfig
        in      eax, dx
        DEBUGF  1,"init_board: PCI device: TxConfig = 0x%x\n", eax

        mov     [tpc.chipset],  0

        xor     eax, eax
        inc     eax
        ret

  .match:
        DEBUGF  1,"init_board: chipset=%u\n", ecx
        xor     eax,eax
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

;       DEBUGF  1,"K :   Set MAC Reg C+CR Offset 0x82h = 0x01h\n"
        set_io  0
        set_io  0x82
        mov     al, 0x01
        out     dx, al
        cmp     [tpc.mcfg], MCFG_METHOD_03
        jae     @f
;       DEBUGF  1,"K :   Set PCI Latency=0x40\n"
;       stdcall pci_write_config_byte,PCI_LATENCY_TIMER,0x40
   @@:
        cmp     [tpc.mcfg], MCFG_METHOD_02
        jne     @f
;       DEBUGF  1,"K :   Set MAC Reg C+CR Offset 0x82h = 0x01h\n"
        set_io  0x82
        mov     al, 0x01
        out     dx, al
;       DEBUGF  1,"K :   Set PHY Reg 0x0bh = 0x00h\n"
        WRITE_GMII_REG 0x0b, 0x0000      ; w 0x0b 15 0 0
    @@:
        ; if TBI is not enabled
        set_io  0
        set_io  REG_PHYstatus
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

        udelay  100
        mov     ecx, 10000
        ; wait for auto-negotiation process
    @@: dec     ecx
        jz      @f
        set_io  0
        READ_GMII_REG PHY_STAT_REG
        udelay  100
        test    eax, PHY_Auto_Neco_Comp
        jz      @b
        set_io  REG_PHYstatus
        in      al, dx
        jmp     @f
  .tbi_dis:
        udelay  100
    @@:


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

        DEBUGF  1,"reset\n"

        lea     eax, [tx_ring]
        mov     [tpc.TxDescArrays], eax
        mov     [tpc.TxDescArray], eax

        lea     eax, [rx_ring]
        mov     [tpc.RxDescArrays], eax
        mov     [tpc.RxDescArray], eax

        call    init_ring
        call    hw_start

; clear packet/byte counters

        xor     eax, eax
        lea     edi, [device.bytes_tx]
        mov     ecx, 6
        rep     stosd

        mov     [device.mtu], 1500

        xor     eax, eax
        ret





align 4
PHY_config:

        DEBUGF  1,"hw_PHY_config: priv.mcfg=%d, priv.pcfg=%d\n",[tpc.mcfg],[tpc.pcfg]

        cmp     [tpc.mcfg], MCFG_METHOD_04
        jne     .not_4
        set_io  0
;       WRITE_GMII_REG 0x1F, 0x0001
;       WRITE_GMII_REG 0x1b, 0x841e
;       WRITE_GMII_REG 0x0e, 0x7bfb
;       WRITE_GMII_REG 0x09, 0x273a
        WRITE_GMII_REG 0x1F, 0x0002
        WRITE_GMII_REG 0x01, 0x90D0
        WRITE_GMII_REG 0x1F, 0x0000
        jmp     .exit
  .not_4:
        cmp     [tpc.mcfg], MCFG_METHOD_02
        je      @f
        cmp     [tpc.mcfg], MCFG_METHOD_03
        jne     .not_2_or_3
    @@:
        set_io  0
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
        DEBUGF  1,"tpc.mcfg=%d, discard hw PHY config\n", [tpc.mcfg]
  .exit:
        ret



align 4
set_rx_mode:

        DEBUGF  1,"set_rx_mode\n"

        ; IFF_ALLMULTI
        ; Too many to filter perfectly -- accept all multicasts
        set_io  0
        set_io  REG_RxConfig
        in      eax, dx
        mov     ecx, [tpc.chipset]
        and     eax, [rtl_chip_info + ecx * 8 + 4] ; RxConfigMask
        or      eax, rx_config or (RXM_AcceptBroadcast or RXM_AcceptMulticast or RXM_AcceptMyPhys)
        out     dx, eax

        ; Multicast hash filter
        set_io  REG_MAR0 + 0
        or      eax, -1
        out     dx, eax
        set_io  REG_MAR0 + 4
        out     dx, eax

        ret


align 4
init_ring:

        DEBUGF  1,"init_ring\n"

        xor     eax, eax
        mov     [tpc.cur_rx], eax
        mov     [tpc.cur_tx], eax

        lea     edi, [tx_ring]
        mov     ecx, (NUM_TX_DESC * tx_desc.size) / 4
        rep     stosd

        lea     edi, [rx_ring]
        mov     ecx, (NUM_RX_DESC * rx_desc.size) / 4
        rep     stosd

        mov     edi, [tpc.RxDescArray]
        mov     ecx, NUM_RX_DESC
  .loop:
        push    ecx
        stdcall KernelAlloc, RX_BUF_SIZE
        mov     [edi + rx_desc.buf_soft_addr], eax
        call    GetPgAddr
        mov     dword [edi + rx_desc.buf_addr], eax
        mov     [edi + rx_desc.status], DSB_OWNbit or RX_BUF_SIZE
        add     edi, rx_desc.size
        pop     ecx
        loop    .loop
        or      [edi - rx_desc.size + rx_desc.status], DSB_EORbit

        ret


align 4
hw_start:

        DEBUGF  1,"hw_start\n"

; attach int handler
        movzx   eax, [device.irq_line]
        DEBUGF  1,"Attaching int handler to irq %x\n", eax:1
        stdcall AttachIntHandler, eax, int_handler, dword 0

        ; Soft reset the chip
        set_io  0
        set_io  REG_ChipCmd
        mov     al, CMD_Reset
        out     dx, al

        ; Check that the chip has finished the reset
        mov     ecx, 1000
        set_io  REG_ChipCmd
    @@: in      al, dx
        test    al, CMD_Reset
        jz      @f
        udelay  10
        loop    @b
    @@:

        set_io  REG_Cfg9346
        mov     al, CFG_9346_Unlock
        out     dx, al

        set_io  REG_ChipCmd
        mov     al, CMD_TxEnb or CMD_RxEnb
        out     dx, al

        set_io  REG_ETThReg
        mov     al, ETTh
        out     dx, al

        ; For gigabit rtl8169
        set_io  REG_RxMaxSize
        mov     ax, RxPacketMaxSize
        out     dx, ax

        ; Set Rx Config register
        set_io  REG_RxConfig
        in      ax, dx
        mov     ecx, [tpc.chipset]
        and     eax, [rtl_chip_info + ecx * 8 + 4] ; RxConfigMask
        or      eax, rx_config
        out     dx, eax

        ; Set DMA burst size and Interframe Gap Time
        set_io  REG_TxConfig
        mov     eax, (TX_DMA_BURST shl TXC_DMAShift) or (InterFrameGap shl TXC_InterFrameGapShift)
        out     dx, eax

        set_io  REG_CPlusCmd
        in      ax, dx
        out     dx, ax

        in      ax, dx
        or      ax, 1 shl 3
        cmp     [tpc.mcfg], MCFG_METHOD_02
        jne     @f
        cmp     [tpc.mcfg], MCFG_METHOD_03
        jne     @f
        or      ax,1 shl 14
        DEBUGF  1,"Set MAC Reg C+CR Offset 0xE0: bit-3 and bit-14\n"
        jmp     .set
    @@:
        DEBUGF  1,"Set MAC Reg C+CR Offset 0xE0: bit-3\n"
  .set:
        set_io  REG_CPlusCmd
        out     dx, ax

        set_io  0xE2
;        mov     ax, 0x1517
;        out     dx, ax
;        mov     ax, 0x152a
;        out     dx, ax
;        mov     ax, 0x282a
;        out     dx, ax
        xor     ax, ax
        out     dx, ax

        xor     eax, eax
        mov     [tpc.cur_rx], eax
        lea     eax, [tx_ring]
        GetRealAddr
        set_io  REG_TxDescStartAddr
        out     dx, eax

        lea     eax, [rx_ring]
        GetRealAddr
        set_io  REG_RxDescStartAddr
        out     dx, eax

        set_io  REG_Cfg9346
        mov     al, CFG_9346_Lock
        out     dx, al

        udelay  10

        xor     eax, eax
        set_io  REG_RxMissed
        out     dx, eax

        call    set_rx_mode

        set_io  0
        ; no early-rx interrupts
        set_io  REG_MultiIntr
        in      ax, dx
        and     ax, 0xF000
        out     dx, ax

        ; set interrupt mask
        set_io  REG_IntrMask
        mov     ax, intr_mask
        out     dx, ax

        xor     eax, eax
        ret


align 4
read_mac:

        set_io  0
        set_io  REG_MAC0
        xor     ecx, ecx
        lea     edi, [device.mac]
        mov     ecx, MAC_ADDR_LEN

        ; Get MAC address. FIXME: read EEPROM
    @@: in      al, dx
        stosb
        inc     edx
        loop    @r

        DEBUGF  1,"MAC = %x-%x-%x-%x-%x-%x\n",[device.mac+0]:2,[device.mac+1]:2,[device.mac+2]:2,[device.mac+3]:2,[device.mac+4]:2,[device.mac+5]:2

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
align 4
transmit:

        DEBUGF  1,"Transmitting packet, buffer:%x, size:%u\n", [esp+4], [esp+8]
        mov     eax, [esp+4]
        DEBUGF  1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     dword [esp+8], MAX_ETH_FRAME_SIZE
        ja      .fail

;----------------------------------
; Find currentTX descriptor address

        mov     eax, tx_desc.size
        mul     [tpc.cur_tx]
        lea     esi, [eax + tx_ring]

        DEBUGF  1,"Using TX desc: %x\n", esi

;---------------------------
; Program the packet pointer

        mov     eax, [esp + 4]
        mov     [esi + tx_desc.buf_soft_addr], eax
        GetRealAddr
        mov     dword [esi + tx_desc.buf_addr], eax

;------------------------
; Program the packet size

        mov     eax, [esp + 8]
    @@: or      eax, DSB_OWNbit or DSB_FSbit or DSB_LSbit
        cmp     [tpc.cur_tx], NUM_TX_DESC - 1
        jne     @f
        or      eax, DSB_EORbit
    @@: mov     [esi + tx_desc.status], eax

;-----------------------------------------
; Set the polling bit (start transmission)

        set_io  0
        set_io  REG_TxPoll
        mov     al, 0x40     ; set polling bit
        out     dx, al

;-----------------------
; Update TX descriptor

        inc     [tpc.cur_tx]
        and     [tpc.cur_tx], NUM_TX_DESC - 1

;-------------
; Update stats

        inc     [device.packets_tx]
        mov     eax, [esp+8]
        add     dword [device.bytes_tx], eax
        adc     dword [device.bytes_tx + 4], 0

        xor     eax, eax
        ret     8

  .fail:
        DEBUGF  1,"transmit failed\n"
        or      eax, -1
        stdcall KernelFree, [esp+4]
        ret     8


;;;DSB_OWNbit


;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

        DEBUGF  1,"IRQ %x ",eax:2

; find pointer of device wich made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .fail
        mov     esi, device_list
  .nextdevice:
        mov     ebx, dword [esi]

        set_io  0
        set_io  REG_IntrStatus
        in      ax, dx

        test    ax, ax
        jnz     .got_it

  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice

        ret                                             ; If no device was found, abort (The irq was probably for a device, not registered to this driver)

  .got_it:
        DEBUGF  1,"IntrStatus = 0x%x\n",ax

        cmp     ax, 0xFFFF      ; if so, hardware is no longer present
        je      .fail

;--------
; Receive

        test    ax, ISB_RxOK
        jz      .no_rx

        push    ax
        push    ebx

  .check_more:
        pop     ebx
        DEBUGF  1,"ebx = 0x%x\n", ebx
        mov     eax, rx_desc.size
        mul     [tpc.cur_rx]
        lea     esi, [eax + rx_ring]

        DEBUGF  1,"RxDesc.status = 0x%x\n", [esi + rx_desc.status]

        mov     eax, [esi + rx_desc.status]
        test    eax, DSB_OWNbit ;;;
        jnz     .rx_return

        DEBUGF  1,"tpc.cur_rx = %u\n", [tpc.cur_rx]

        test    eax, SD_RxRES
        jnz     .rx_return      ;;;;; RX error!

        push    ebx
        push    .check_more
        and     eax, 0x00001FFF
        add     eax, -4                         ; we dont need CRC
        push    eax
        DEBUGF  1,"data length = %u\n", ax

;-------------
; Update stats

        add     dword [device.bytes_rx], eax
        adc     dword [device.bytes_rx + 4], 0
        inc     dword [device.packets_rx]

        push    [esi + rx_desc.buf_soft_addr]

;----------------------
; Allocate a new buffer

        stdcall KernelAlloc, RX_BUF_SIZE
        mov     [esi + rx_desc.buf_soft_addr], eax
        GetRealAddr
        mov     dword [esi + rx_desc.buf_addr], eax

;---------------
; re set OWN bit

        mov     eax, DSB_OWNbit or RX_BUF_SIZE
        cmp     [tpc.cur_rx], NUM_RX_DESC - 1
        jne     @f
        or      eax, DSB_EORbit
    @@: mov     [esi + rx_desc.status], eax

;--------------
; Update rx ptr

        inc     [tpc.cur_rx]
        and     [tpc.cur_rx], NUM_RX_DESC - 1

        jmp     EthReceiver
  .rx_return:

        pop     ax
  .no_rx:

;---------
; Transmit

        test    ax, ISB_TxOK
        jz      .no_tx
        push    ax

        DEBUGF  1,"TX ok!\n"

        mov     ecx, NUM_TX_DESC
        lea     esi, [tx_ring]
  .txloop:
        cmp     [esi+tx_desc.buf_soft_addr], 0
        jz      .maybenext

        test    [esi+tx_desc.status], DSB_OWNbit
        jnz     .maybenext

        push    ecx
        DEBUGF  1,"Freeing up TX desc: %x\n", esi
        stdcall KernelFree, [esi+tx_desc.buf_soft_addr]
        pop     ecx
        and     [esi+tx_desc.buf_soft_addr], 0

  .maybenext:
        add     esi, tx_desc.size
        dec     ecx
        jnz     .txloop

        pop     ax
  .no_tx:

;-------
; Finish

        set_io  0
        set_io  REG_IntrStatus
        out     dx, ax                  ; ACK all interrupts

  .fail:
        ret









; End of code
align 4                                         ; Place all initialised data here

devices       dd 0
version       dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service    db 'RTL8169',0                    ; max 16 chars include zero

include_debug_strings                           ; All data wich FDO uses will be included here

rtl_chip_info dd \
  MCFG_METHOD_01, 0xff7e1880, \ ; RTL8169
  MCFG_METHOD_02, 0xff7e1880, \ ; RTL8169s/8110s
  MCFG_METHOD_03, 0xff7e1880, \ ; RTL8169s/8110s
  MCFG_METHOD_04, 0xff7e1880, \ ; RTL8169sb/8110sb
  MCFG_METHOD_05, 0xff7e1880, \ ; RTL8169sc/8110sc
  MCFG_METHOD_11, 0xff7e1880, \ ; RTL8168b/8111b   // PCI-E
  MCFG_METHOD_12, 0xff7e1880, \ ; RTL8168b/8111b   // PCI-E
  MCFG_METHOD_13, 0xff7e1880, \ ; RTL8101e         // PCI-E 8139
  MCFG_METHOD_14, 0xff7e1880, \ ; RTL8100e         // PCI-E 8139
  MCFG_METHOD_15, 0xff7e1880    ; RTL8100e         // PCI-E 8139

mac_info dd \
  0x38800000, MCFG_METHOD_15, \
  0x38000000, MCFG_METHOD_12, \
  0x34000000, MCFG_METHOD_13, \
  0x30800000, MCFG_METHOD_14, \
  0x30000000, MCFG_METHOD_11, \
  0x18000000, MCFG_METHOD_05, \
  0x10000000, MCFG_METHOD_04, \
  0x04000000, MCFG_METHOD_03, \
  0x00800000, MCFG_METHOD_02, \
  0x00000000, MCFG_METHOD_01    ; catch-all

name_01         db "RTL8169", 0
name_02_03      db "RTL8169s/8110s", 0
name_04         db "RTL8169sb/8110sb", 0
name_05         db "RTL8169sc/8110sc", 0
name_11_12      db "RTL8168b/8111b", 0  ; PCI-E
name_13         db "RTL8101e", 0        ; PCI-E 8139
name_14_15      db "RTL8100e", 0        ; PCI-E 8139


section '.data' data readable writable align 16 ; place all uninitialized data place here

device_list rd MAX_DEVICES                     ; This list contains all pointers to device structures the driver is handling


