;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  FORCEDETH.INC                                                  ;;
;;                                                                 ;;
;;  Ethernet driver for Kolibri OS                                 ;;
;;                                                                 ;;
;;  Driver for chips of NVIDIA nForce2                             ;;
;;  References:                                                    ;;
;;    forcedeth.c - linux driver (etherboot project)               ;;
;;    ethernet driver template by Mike Hibbett                     ;;
;;                                                                 ;;
;;  The copyright statement is                                     ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;  Copyright 2008 shurf,                                          ;;
;;   cit.utc@gmail.com                                             ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


format MS COFF

        API_VERSION             = 0x01000100
        DRIVER_VERSION          = 5

        MAX_DEVICES             = 16

        RBLEN                   = 0     ; Receive buffer size: 0=4K 1=8k 2=16k 3=32k 4=64k
                                        ; FIXME: option 1 and 2 will not allocate buffer correctly causing data loss!

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 1

        RX_RING                 = 4
        TX_RING                 = 4

include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'
include '../struct.inc'
include 'netdrv.inc'

public START
public service_proc
public version

;**************************************************************************
; forcedeth Register Definitions
;**************************************************************************

PCI_DEVICE_ID_NVIDIA_NVENET_1   = 0x01c3
PCI_DEVICE_ID_NVIDIA_NVENET_2   = 0x0066
PCI_DEVICE_ID_NVIDIA_NVENET_4   = 0x0086
PCI_DEVICE_ID_NVIDIA_NVENET_5   = 0x008c
PCI_DEVICE_ID_NVIDIA_NVENET_3   = 0x00d6
PCI_DEVICE_ID_NVIDIA_NVENET_7   = 0x00df
PCI_DEVICE_ID_NVIDIA_NVENET_6   = 0x00e6
PCI_DEVICE_ID_NVIDIA_NVENET_8   = 0x0056
PCI_DEVICE_ID_NVIDIA_NVENET_9   = 0x0057
PCI_DEVICE_ID_NVIDIA_NVENET_10  = 0x0037
PCI_DEVICE_ID_NVIDIA_NVENET_11  = 0x0038
PCI_DEVICE_ID_NVIDIA_NVENET_12  = 0x0268
PCI_DEVICE_ID_NVIDIA_NVENET_13  = 0x0269
PCI_DEVICE_ID_NVIDIA_NVENET_14  = 0x0372
PCI_DEVICE_ID_NVIDIA_NVENET_15  = 0x0373

UNKSETUP1_VAL             = 0x16070f
UNKSETUP2_VAL             = 0x16
UNKSETUP3_VAL1            = 0x200010
UNKSETUP4_VAL             = 8
UNKSETUP5_BIT31           = (1 shl 31)
UNKSETUP6_VAL             = 3

TXRXCTL_RXCHECK           = 0x0400
MIISTAT_ERROR             = 0x0001
MIISTAT_MASK              = 0x000f
MIISTAT_MASK2             = 0x000f
MIICTL_INUSE              = 0x08000
MIICTL_WRITE              = 0x00400
MIICTL_ADDRSHIFT          = 5

MIISPEED_BIT8             = (1 shl 8)
MIIDELAY                  = 5

IRQ_RX_ERROR              = 0x0001
IRQ_RX                    = 0x0002
IRQ_RX_NOBUF              = 0x0004
IRQ_LINK                  = 0x0040
IRQ_TIMER                 = 0x0020
IRQMASK_WANTED_2          = 0x0147

IRQ_RX_ALL                = IRQ_RX_ERROR or IRQ_RX or IRQ_RX_NOBUF
IRQ_TX_ALL                = 0       ; ???????????
IRQ_OTHER_ALL             = IRQ_LINK ;or IRQ_TIMER

IRQSTAT_MASK              = 0x1ff

TXRXCTL_KICK              = 0x0001
TXRXCTL_BIT1              = 0x0002
TXRXCTL_BIT2              = 0x0004
TXRXCTL_IDLE              = 0x0008
TXRXCTL_RESET             = 0x0010
TXRXCTL_RXCHECK           = 0x0400

MCASTADDRA_FORCE          = 0x01

MAC_RESET_ASSERT          = 0x0F3

MISC1_HD                  = 0x02
MISC1_FORCE               = 0x3b0f3c

PFF_ALWAYS                = 0x7F0008
PFF_PROMISC               = 0x80
PFF_MYADDR                = 0x20

OFFLOAD_HOMEPHY           = 0x601
OFFLOAD_NORMAL            = 4096 shl RBLEN

RNDSEED_MASK              = 0x00ff
RNDSEED_FORCE             = 0x7f00
RNDSEED_FORCE2            = 0x2d00
RNDSEED_FORCE3            = 0x7400

; POLL_DEFAULT is the interval length of the timer source on the nic
; POLL_DEFAULT=97 would result in an interval length of 1 ms
POLL_DEFAULT              = 970

ADAPTCTL_START            = 0x02
ADAPTCTL_LINKUP           = 0x04
ADAPTCTL_PHYVALID         = 0x40000
ADAPTCTL_RUNNING          = 0x100000
ADAPTCTL_PHYSHIFT         = 24

WAKEUPFLAGS_VAL           = 0x7770

POWERSTATE_POWEREDUP      = 0x8000
POWERSTATE_VALID          = 0x0100
POWERSTATE_MASK           = 0x0003
POWERSTATE_D0             = 0x0000
POWERSTATE_D1             = 0x0001
POWERSTATE_D2             = 0x0002
POWERSTATE_D3             = 0x0003

POWERSTATE2_POWERUP_MASK  = 0x0F11
POWERSTATE2_POWERUP_REV_A3= 0x0001

RCVCTL_START              = 0x01
RCVSTAT_BUSY              = 0x01

XMITCTL_START             = 0x01

LINKSPEED_FORCE           = 0x10000
LINKSPEED_10              = 1000
LINKSPEED_100             = 100
LINKSPEED_1000            = 50

RINGSZ_TXSHIFT            = 0
RINGSZ_RXSHIFT            = 16

LPA_1000FULL                    = 0x0800

; Link partner ability register.
LPA_SLCT                        = 0x001f  ; Same as advertise selector
LPA_10HALF                      = 0x0020  ; Can do 10mbps half-duplex
LPA_10FULL                      = 0x0040  ; Can do 10mbps full-duplex
LPA_100HALF                     = 0x0080  ; Can do 100mbps half-duplex
LPA_100FULL                     = 0x0100  ; Can do 100mbps full-duplex
LPA_100BASE4                    = 0x0200  ; Can do 100mbps 4k packets
LPA_RESV                        = 0x1c00  ; Unused...
LPA_RFAULT                      = 0x2000  ; Link partner faulted
LPA_LPACK                       = 0x4000  ; Link partner acked us
LPA_NPAGE                       = 0x8000  ; Next page bit

MII_READ                        = (-1)
MII_PHYSID1                     = 0x02    ; PHYS ID 1
MII_PHYSID2                     = 0x03    ; PHYS ID 2
MII_BMCR                        = 0x00    ; Basic mode control register
MII_BMSR                        = 0x01    ; Basic mode status register
MII_ADVERTISE                   = 0x04    ; Advertisement control reg
MII_LPA                         = 0x05    ; Link partner ability reg
MII_SREVISION                   = 0x16    ; Silicon revision
MII_RESV1                       = 0x17    ; Reserved...
MII_NCONFIG                     = 0x1c    ; Network interface config

; PHY defines
PHY_OUI_MARVELL                 = 0x5043
PHY_OUI_CICADA                  = 0x03f1
PHYID1_OUI_MASK                 = 0x03ff
PHYID1_OUI_SHFT                 = 6
PHYID2_OUI_MASK                 = 0xfc00
PHYID2_OUI_SHFT                 = 10
PHY_INIT1                       = 0x0f000
PHY_INIT2                       = 0x0e00
PHY_INIT3                       = 0x01000
PHY_INIT4                       = 0x0200
PHY_INIT5                       = 0x0004
PHY_INIT6                       = 0x02000
PHY_GIGABIT                     = 0x0100

PHY_TIMEOUT                     = 0x1
PHY_ERROR                       = 0x2

PHY_100                         = 0x1
PHY_1000                        = 0x2
PHY_HALF                        = 0x100

PHY_RGMII                       = 0x10000000

; desc_ver values:
; This field has two purposes:
; - Newer nics uses a different ring layout. The layout is selected by
;   comparing np->desc_ver with DESC_VER_xy.
; - It contains bits that are forced on when writing to TxRxControl.
DESC_VER_1                      = 0x0
DESC_VER_2                      = (0x02100 or TXRXCTL_RXCHECK)

MAC_ADDR_LEN                    = 6

NV_TX_LASTPACKET                = (1 shl 16)
NV_TX_RETRYERROR                = (1 shl 19)
NV_TX_LASTPACKET1               = (1 shl 24)
NV_TX_DEFERRED                  = (1 shl 26)
NV_TX_CARRIERLOST               = (1 shl 27)
NV_TX_LATECOLLISION             = (1 shl 28)
NV_TX_UNDERFLOW                 = (1 shl 29)
NV_TX_ERROR                     = (1 shl 30)
NV_TX_VALID                     = (1 shl 31)

NV_TX2_LASTPACKET               = (1 shl 29)
NV_TX2_RETRYERROR               = (1 shl 18)
NV_TX2_LASTPACKET1              = (1 shl 23)
NV_TX2_DEFERRED                 = (1 shl 25)
NV_TX2_CARRIERLOST              = (1 shl 26)
NV_TX2_LATECOLLISION            = (1 shl 27)
NV_TX2_UNDERFLOW                = (1 shl 28)
; error and valid are the same for both
NV_TX2_ERROR                    = (1 shl 30)
NV_TX2_VALID                    = (1 shl 31)

NV_RX_DESCRIPTORVALID           = (1 shl 16)
NV_RX_AVAIL                     = (1 shl 31)

NV_RX2_DESCRIPTORVALID          = (1 shl 29)

FLAG_MASK_V1                    = 0xffff0000
FLAG_MASK_V2                    = 0xffffc000
LEN_MASK_V1                     = (0xffffffff xor FLAG_MASK_V1)
LEN_MASK_V2                     = (0xffffffff xor FLAG_MASK_V2)

; Miscelaneous hardware related defines:
NV_PCI_REGSZ_VER1               = 0x270
NV_PCI_REGSZ_VER2               = 0x604
; various timeout delays: all in usec
NV_TXRX_RESET_DELAY             = 4
NV_TXSTOP_DELAY1                = 10
NV_TXSTOP_DELAY1MAX             = 500000
NV_TXSTOP_DELAY2                = 100
NV_RXSTOP_DELAY1                = 10
NV_RXSTOP_DELAY1MAX             = 500000
NV_RXSTOP_DELAY2                = 100
NV_SETUP5_DELAY                 = 5
NV_SETUP5_DELAYMAX              = 50000
NV_POWERUP_DELAY                = 5
NV_POWERUP_DELAYMAX             = 5000
NV_MIIBUSY_DELAY                = 50
NV_MIIPHY_DELAY                 = 10
NV_MIIPHY_DELAYMAX              = 10000
NV_MAC_RESET_DELAY              = 64
NV_WAKEUPPATTERNS               = 5
NV_WAKEUPMASKENTRIES            = 4

; Advertisement control register.
ADVERTISE_SLCT                  = 0x001f  ; Selector bits
ADVERTISE_CSMA                  = 0x0001  ; Only selector supported
ADVERTISE_10HALF                = 0x0020  ; Try for 10mbps half-duplex
ADVERTISE_10FULL                = 0x0040  ; Try for 10mbps full-duplex
ADVERTISE_100HALF               = 0x0080  ; Try for 100mbps half-duplex
ADVERTISE_100FULL               = 0x0100  ; Try for 100mbps full-duplex
ADVERTISE_100BASE4              = 0x0200  ; Try for 100mbps 4k packets
ADVERTISE_RESV                  = 0x1c00  ; Unused...
ADVERTISE_RFAULT                = 0x2000  ; Say we can detect faults
ADVERTISE_LPACK                 = 0x4000  ; Ack link partners response
ADVERTISE_NPAGE                 = 0x8000  ; Next page bit

ADVERTISE_FULL                  = (ADVERTISE_100FULL or ADVERTISE_10FULL or ADVERTISE_CSMA)
ADVERTISE_ALL                   = (ADVERTISE_10HALF or ADVERTISE_10FULL or ADVERTISE_100HALF or ADVERTISE_100FULL)

MII_1000BT_CR                   = 0x09
MII_1000BT_SR                   = 0x0a
ADVERTISE_1000FULL              = 0x0200
ADVERTISE_1000HALF              = 0x0100

BMCR_ANRESTART                  = 0x0200  ; Auto negotiation restart
BMCR_ANENABLE                   = 0x1000  ; Enable auto negotiation
BMCR_SPEED100                   = 0x2000  ; Select 100Mbps
BMCR_LOOPBACK                   = 0x4000  ; TXD loopback bits
BMCR_RESET                      = 0x8000  ; Reset the DP83840

; Basic mode status register.
BMSR_ERCAP                      = 0x0001  ; Ext-reg capability
BMSR_JCD                        = 0x0002  ; Jabber detected
BMSR_LSTATUS                    = 0x0004  ; Link status
BMSR_ANEGCAPABLE                = 0x0008  ; Able to do auto-negotiation
BMSR_RFAULT                     = 0x0010  ; Remote fault detected
BMSR_ANEGCOMPLETE               = 0x0020  ; Auto-negotiation complete
BMSR_RESV                       = 0x07c0  ; Unused...
BMSR_10HALF                     = 0x0800  ; Can do 10mbps, half-duplex
BMSR_10FULL                     = 0x1000  ; Can do 10mbps, full-duplex
BMSR_100HALF                    = 0x2000  ; Can do 100mbps, half-duplex
BMSR_100FULL                    = 0x4000  ; Can do 100mbps, full-duplex
BMSR_100BASE4                   = 0x8000  ; Can do 100mbps, 4k packets

struct  TxDesc
        PacketBuffer            dd ?
        FlagLen                 dd ?
ends

struct  RxDesc
        PacketBuffer            dd ?
        FlagLen                 dd ?
ends

virtual at ebx

        device:

        ETH_DEVICE

        .pci_bus                dd ?
        .pci_dev                dd ?

        .mmio_addr              dd ?
        .vendor_id              dw ?
        .device_id              dw ?
        .txflags                dd ?
        .desc_ver               dd ?
        .irqmask                dd ?
        .wolenabled             dd ?
        .in_shutdown            dd ?
        .cur_rx                 dd ?
        .phyaddr                dd ?
        .phy_oui                dd ?
        .gigabit                dd ?
        .needs_mac_reset        dd ?
        .linkspeed              dd ?
        .duplex                 dd ?
        .next_tx                dd ?
        .nocable                dd ?

                                rb 0x100 - (($ - device) and 0xff)
        .tx_ring                rd (TX_RING * sizeof.TxDesc) /4*2

                                rb 0x100 - (($ - device) and 0xff)
        .rx_ring                rd (RX_RING * sizeof.RxDesc) /4*2

        sizeof.device_struct = $ - device
end virtual


virtual at edi
        IrqStatus               dd ?
        IrqMask                 dd ?
        UnknownSetupReg6        dd ?
        PollingInterval         dd ?
end virtual

virtual at edi + 0x3c
        MacReset                dd ?
end virtual

virtual at edi + 0x80
        Misc1                   dd ?
        TransmitterControl      dd ?
        TransmitterStatus       dd ?
        PacketFilterFlags       dd ?
        OffloadConfig           dd ?
        ReceiverControl         dd ?
        ReceiverStatus          dd ?
        RandomSeed              dd ?
        UnknownSetupReg1        dd ?
        UnknownSetupReg2        dd ?
        MacAddrA                dd ?
        MacAddrB                dd ?
        MulticastAddrA          dd ?
        MulticastAddrB          dd ?
        MulticastMaskA          dd ?
        MulticastMaskB          dd ?
        PhyInterface            dd ?
end virtual

virtual at edi + 0x100
        TxRingPhysAddr          dd ?
        RxRingPhysAddr          dd ?
        RingSizes               dd ?
        UnknownTransmitterReg   dd ?
        LinkSpeed               dd ?
end virtual

virtual at edi + 0x130
        UnknownSetupReg5        dd ?
end virtual

virtual at edi + 0x13c
        UnknownSetupReg3        dd ?
end virtual

virtual at edi + 0x144
        TxRxControl             dd ?
end virtual

virtual at edi + 0x180
        MIIStatus               dd ?
        UnknownSetupReg4        dd ?
        AdapterControl          dd ?
        MIISpeed                dd ?
        MIIControl              dd ?
        MIIData                 dd ?
end virtual

virtual at edi + 0x200
        WakeUpFlags             dd ?
end virtual

virtual at edi + 0x26c
        PowerState              dd ?
end virtual

virtual at edi + 0x600
        PowerState2             dd ?
end virtual


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

        DEBUGF  2,"Loading %s driver\n", my_service
        stdcall RegService, my_service, service_proc
        ret

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
        jb      .fail
        mov     eax, [IOCTL.output]
        mov     [eax], dword API_VERSION

        xor     eax, eax
        ret

;------------------------------------------------------
  @@:
        cmp     eax, 1 ;SRV_HOOK
        jne     .fail

        cmp     [IOCTL.inp_size], 3                     ; Data input must be at least 3 bytes
        jb      .fail

        mov     eax, [IOCTL.input]
        cmp     byte [eax], 1                           ; 1 means device number and bus number (pci) are given
        jne     .fail                                   ; other types arent supported for this card yet

; check if the device is already listed

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

;        mov     eax, [IOCTL.input]                      ; get the pci bus and device numbers
        mov     ax, [eax+1]
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte [device.pci_bus]               ; compare with pci and device num in device list (notice the usage of word instead of byte)
        jne     @f
        cmp     ah, byte [device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
  @@:
        add     esi, 4
        loop    .nextdevice


; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
        cmp     [devices], MAX_DEVICES                  ; First check if the driver can handle one more card
        jae     .fail

        allocate_and_clear ebx, sizeof.device_struct, .fail      ; Allocate the buffer for device structure

; Fill in the direct call addresses into the struct

        mov     [device.reset], reset
        mov     [device.transmit], transmit
        mov     [device.get_MAC], read_mac
        mov     [device.set_MAC], .fail
        mov     [device.unload], .fail
        mov     [device.name], my_service

; save the pci bus and device numbers

        mov     eax, [IOCTL.input]
        movzx   ecx, byte [eax+1]
        mov     [device.pci_bus], ecx
        movzx   ecx, byte [eax+2]
        mov     [device.pci_dev], ecx

        DEBUGF  1,"Hooking into device, dev:%x, bus:%x\n", [device.pci_dev], [device.pci_bus]

; Ok, the eth_device structure is ready, let's probe the device
        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err                                                    ; If an error occured, exit

        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        mov     [device.type], NET_TYPE_ETH
        call    NetRegDev

        cmp     eax, -1
        je      .destroy

        ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  1,"Trying to find device number of already registered device\n"
        call    NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  1,"Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        ; todo: reset device into virgin state

  .err:
        stdcall KernelFree, ebx
  .fail:

        ret

;------------------------------------------------------
endp


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;




;***************************************************************************
;   Function
;      probe
;   Description
;      Searches for an ethernet card, enables it and clears the rx buffer
;
;***************************************************************************
align 4
probe:

        DEBUGF  1,"probe\n"

        mov     [device.needs_mac_reset], 0

        PCI_make_bus_master
        PCI_adjust_latency  32
        PCI_find_mmio32

        DEBUGF 1,"mmio_addr= 0x%x\n", [device.mmio_addr]:8

        stdcall MapIoMem, [device.mmio_addr], 2048, (PG_SW + PG_NOCACHE)
        test    eax, eax
        jz      fail
        mov     [device.mmio_addr], eax
        mov     edi, eax

        DEBUGF 1,"mapped mmio_addr= 0x%x\n", [device.mmio_addr]:8

;-------------------------------------
; handle different descriptor versions
        mov     [device.desc_ver], DESC_VER_1
        movzx   eax, [device.device_id]
        cmp     eax, PCI_DEVICE_ID_NVIDIA_NVENET_1
        je      .ver1
        cmp     eax, PCI_DEVICE_ID_NVIDIA_NVENET_2
        je      .ver1
        cmp     eax, PCI_DEVICE_ID_NVIDIA_NVENET_3
        je      .ver1
        mov     [device.desc_ver], DESC_VER_2
  .ver1:

        call    read_mac

        ; disable WOL
        mov     [WakeUpFlags], 0
        mov     [device.wolenabled], 0
        
        mov     [device.txflags], (NV_TX2_LASTPACKET or NV_TX2_VALID)
        cmp     [device.desc_ver], DESC_VER_1
        jne     @f
        mov     [device.txflags], (NV_TX_LASTPACKET or NV_TX_VALID)
      @@:

; BEGIN of switch (pci->dev_id)

        cmp     [device.device_id], 0x01C3
        jne     .next_0x0066
        ; nforce
        mov     [device.irqmask], 0    ;;;;;;;;;;;;;;;(IRQMASK_WANTED_2 or IRQ_TIMER)
        jmp     .find_phy

  .next_0x0066:
        cmp     [device.device_id], 0x0066
        je      @f
        cmp     [device.device_id], 0x00D6
        je      @f
        jmp     .next_0x0086
  @@:
        mov     [device.irqmask], 0    ;;;;;;;;;;;;;;;;(IRQMASK_WANTED_2 or IRQ_TIMER)
        cmp     [device.desc_ver], DESC_VER_1
        jne     @f
        or      [device.txflags], NV_TX_LASTPACKET1
        jmp     .find_phy
  @@:
        or      [device.txflags], NV_TX2_LASTPACKET1
        jmp     .find_phy

  .next_0x0086:
        cmp     [device.device_id], 0x0086
        je      @f
        cmp     [device.device_id], 0x008c
        je      @f
        cmp     [device.device_id], 0x00e6
        je      @f
        cmp     [device.device_id], 0x00df
        je      @f
        cmp     [device.device_id], 0x0056
        je      @f
        cmp     [device.device_id], 0x0057
        je      @f
        cmp     [device.device_id], 0x0037
        je      @f
        cmp     [device.device_id], 0x0038
        je      @f
        jmp     .find_phy

      @@:
        mov     [device.irqmask], 0    ;;;;;;;;;;;;;;;;(IRQMASK_WANTED_2 or IRQ_TIMER)

        cmp     [device.desc_ver], DESC_VER_1
        jne     @f
        or      [device.txflags], NV_TX_LASTPACKET1
        jmp     .find_phy
       @@:
        or      [device.txflags], NV_TX2_LASTPACKET1
        jmp     .find_phy

.next_0x0268:
;       cmp     word [device_id], 0x0268
;       je      @f
;       cmp     word [device_id], 0x0269
;       je      @f
;       cmp     word [device_id], 0x0372
;       je      @f
;       cmp     word [device_id], 0x0373
;       je      @f
;       jmp     .default_switch
;@@:
        cmp     [device.device_id], 0x0268
        jb      .undefined

; Get device revision

        stdcall PciRead8, [device.pci_bus], [device.pci_dev], PCI_REVISION_ID

; take phy and nic out of low power mode
        mov     ecx, [PowerState2]
        and     ecx, not POWERSTATE2_POWERUP_MASK
        cmp     [device.device_id], PCI_DEVICE_ID_NVIDIA_NVENET_12
        jne     @f
        cmp     [device.device_id], PCI_DEVICE_ID_NVIDIA_NVENET_13
        jne     @f
        cmp     al, 0xA3
        jb      @f
        or      ecx, POWERSTATE2_POWERUP_REV_A3
       @@:
        mov     [PowerState2], ecx

        ; DEV_NEED_LASTPACKET1|DEV_IRQMASK_2|DEV_NEED_TIMERIRQ
        mov     [device.irqmask], 0    ;;;;;;;;;;;;;;;;(IRQMASK_WANTED_2 or IRQ_TIMER)
        
        mov     [device.needs_mac_reset], 1
        cmp     [device.desc_ver], DESC_VER_1
        jne     @f
        or      [device.txflags], NV_TX_LASTPACKET1
        jmp     .find_phy
       @@:
        cmp     [device.desc_ver], DESC_VER_2
        jne     .undefined
        or      [device.txflags], NV_TX2_LASTPACKET1
        jmp     .find_phy

  .undefined:
        DEBUGF 1,"Your card was undefined in this driver.\n"
        DEBUGF 1,"Review driver_data in Kolibri driver and send a patch\n"

; Find a suitable phy
; Start with address 1 to 31, then do 0, then fail

  .find_phy:
        xor     edx, edx
  .phy_loop:
        inc     edx
        and     edx, 0x1f       ; phyaddr = i & 0x1f
        mov     eax, MII_PHYSID1
        mov     ecx, MII_READ
        call    mii_rw          ; EDX - addr, EAX - miireg, ECX - value

        cmp     eax, 0xffff
        je      .try_next
        cmp     eax, 0
        jl      .try_next
        mov     esi, eax

        mov     eax, MII_PHYSID2
        mov     ecx, MII_READ
        call    mii_rw

        cmp     eax, 0xffff
        je      .try_next
        cmp     eax, 0
        jl      .try_next
        jmp     .got_it

  .try_next:
        test    edx, edx
        jnz     .phy_loop

        ; PHY in isolate mode? No phy attached and user wants to test loopback?
        ; Very odd, but can be correct.
        
        DEBUGF 1,"Could not find a valid PHY.\n"
        jmp     .no_phy

  .got_it:

        and     esi, PHYID1_OUI_MASK
        shl     esi, PHYID1_OUI_SHFT

        and     eax, PHYID2_OUI_MASK
        shr     eax, PHYID2_OUI_SHFT

        DEBUGF 1,"Found PHY 0x%x:0x%x at address 0x%x\n", esi:8, eax:8, edx

        mov     [device.phyaddr], edx
        or      eax, esi
        mov     [device.phy_oui], eax

        call    phy_init

  .no_phy:

        cmp     [device.needs_mac_reset], 0
        je      @f
        call    mac_reset
  @@:
        
;***************************************************************************
;   Function
;      reset
;   Description
;      Place the chip (ie, the ethernet card) into a virgin state
;      No inputs
;      All registers destroyed
;
;***************************************************************************
reset:

        DEBUGF  1,"Resetting\n"

        stdcall PciRead8, [device.pci_bus], [device.pci_dev], PCI_REG_IRQ
        movzx   eax, al
        stdcall AttachIntHandler, eax, int_handler, dword 0
        test    eax, eax
        jnz     @f
        DEBUGF  1,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
       @@:

; erase previous misconfiguration

        mov     edi, [device.mmio_addr]
        mov     [MulticastAddrA], MCASTADDRA_FORCE
        mov     [MulticastAddrB], 0
        mov     [MulticastMaskA], 0
        mov     [MulticastMaskB], 0
        mov     [PacketFilterFlags], 0
        mov     [TransmitterControl], 0
        mov     [ReceiverControl], 0
        mov     [AdapterControl], 0

; initialize descriptor rings

        call    init_ring

        mov     [LinkSpeed], 0
        mov     [UnknownTransmitterReg], 0

        call    txrx_reset
        
        mov     [UnknownSetupReg6], 0
        mov     [device.in_shutdown], 0

; give hw rings

        lea     eax, [device.rx_ring]
        GetRealAddr
        mov     [RxRingPhysAddr], eax

        lea     eax, [device.tx_ring]
        GetRealAddr
        mov     [TxRingPhysAddr], eax

        mov     [RingSizes], (((RX_RING - 1) shl RINGSZ_RXSHIFT) + ((TX_RING - 1) shl RINGSZ_TXSHIFT))

;

        mov     [device.linkspeed], (LINKSPEED_FORCE or LINKSPEED_10)
        mov     [device.duplex], 0
        mov     [LinkSpeed], (LINKSPEED_FORCE or LINKSPEED_10)
        mov     [UnknownSetupReg3], UNKSETUP3_VAL1

        mov     eax, [device.desc_ver]
        mov     [TxRxControl], eax
        call    pci_push
        or      eax, TXRXCTL_BIT1
        mov     [TxRxControl], eax

        stdcall reg_delay, UnknownSetupReg5-edi, UNKSETUP5_BIT31, UNKSETUP5_BIT31, NV_SETUP5_DELAY, NV_SETUP5_DELAYMAX, 0

        mov     [UnknownSetupReg4], 0
        mov     [MIIStatus], MIISTAT_MASK2

;
        
        mov     [Misc1], (MISC1_FORCE or MISC1_HD)

        mov     eax, [TransmitterStatus]
        mov     [TransmitterStatus], eax

        mov     [PacketFilterFlags], PFF_ALWAYS

        mov     [OffloadConfig], OFFLOAD_NORMAL

        mov     eax, [ReceiverStatus]
        mov     [ReceiverStatus], eax

; set random seed
        push    ebx
        stdcall GetTimerTicks   ; bad idea, driver is started at system startup in 90% of cases..
        pop     ebx

        mov     edi, [device.mmio_addr]

        and     eax, RNDSEED_MASK
        or      eax, RNDSEED_FORCE
        mov     [RandomSeed], eax

        mov     [UnknownSetupReg1], UNKSETUP1_VAL
        mov     [UnknownSetupReg2], UNKSETUP2_VAL
        mov     [PollingInterval], POLL_DEFAULT
        mov     [UnknownSetupReg6], UNKSETUP6_VAL

        mov     eax, [device.phyaddr]
        shl     eax, ADAPTCTL_PHYSHIFT
        or      eax, (ADAPTCTL_PHYVALID or ADAPTCTL_RUNNING)
        mov     [AdapterControl], eax

        mov     [MIISpeed], (MIISPEED_BIT8 or MIIDELAY)
        mov     [UnknownSetupReg4], UNKSETUP4_VAL
        mov     [WakeUpFlags], WAKEUPFLAGS_VAL
        
        or      [PowerState], POWERSTATE_POWEREDUP
        call    pci_push

        mov     esi, 10
        call    Sleep

        or      [PowerState], POWERSTATE_VALID
        mov     [IrqMask], 0

;;;     ; ??? Mask RX interrupts
        mov      [IrqMask], IRQ_RX_ALL + IRQ_TX_ALL
;;;     ; ??? Mask TX interrupts
;;;     mov      [IrqMask], IRQ_TX_ALL
;;;     ; ??? Mask OTHER interrupts
;;;     mov      [IrqMask], IRQ_OTHER_ALL
        call    pci_push

        mov     [MIIStatus], MIISTAT_MASK2
        mov     [IrqStatus], IRQSTAT_MASK
        call    pci_push

        mov     [MulticastAddrA], MCASTADDRA_FORCE
        mov     [MulticastAddrB], 0
        mov     [MulticastMaskA], 0
        mov     [MulticastMaskB], 0

        mov     [PacketFilterFlags], (PFF_ALWAYS or PFF_MYADDR)

        call    set_multicast
        
        ; One manual link speed update: Interrupts are enabled, future link
        ; speed changes cause interrupts and are handled by nv_link_irq().

        mov     eax, [MIIStatus]
        mov     [MIIStatus], MIISTAT_MASK
        DEBUGF  1,"startup: got 0x%x\n", eax

        call    update_linkspeed

        mov     [TransmitterControl], XMITCTL_START       ; start TX
        call    pci_push

        mov     [device.nocable], 0
        test    eax, eax
        jnz     .return
        DEBUGF  1,"no link during initialization.\n"
        mov     [device.nocable], 1

  .return:
        xor     eax, eax        ; Indicate that we have successfully reset the card
        mov     [device.mtu], 1514 ;;; FIXME

        ret


fail:
        or      eax, -1

        ret

;--------------------------------------------------------
;
; MII_RW
;
; read/write a register on the PHY.
; Caller must guarantee serialization
; Input:  EAX - miireg, EDX - addr, ECX - value
; Output: EAX - retval
;
;--------------------------------------------------------

mii_rw:

        DEBUGF  1,"mii_rw: 0x%x to reg %d at PHY %d\n", ecx, eax, edx

        push    edx

        mov     edi, [device.mmio_addr]
        mov     [MIIStatus], MIISTAT_MASK

        test    [MIIControl], MIICTL_INUSE
        jz      @f
        mov     [MIIControl], MIICTL_INUSE

        mov     esi, NV_MIIBUSY_DELAY
        call    Sleep
       @@:

        shl     edx, MIICTL_ADDRSHIFT
        or      edx, eax

        cmp     ecx, MII_READ
        je      @f

        mov     [MIIData], ecx
        or      edx, MIICTL_WRITE
       @@:
        mov     [MIIControl], edx

        stdcall reg_delay, MIIControl-edi, MIICTL_INUSE, 0, NV_MIIPHY_DELAY, NV_MIIPHY_DELAYMAX, 0

        test    eax, eax
        jz      @f
        DEBUGF  1,"mii_rw timed out.\n"
        or      eax, -1
        jmp     .return
       @@:

        cmp     ecx, MII_READ
        je      @f
; it was a write operation - fewer failures are detectable
        DEBUGF  1,"mii_rw write: ok\n"
        xor     eax, eax
        jmp     .return
       @@:

        mov     eax, [MIIStatus]
        test    eax, MIISTAT_ERROR
        jz      @f
        DEBUGF  1,"mii read: failed.\n"
        or      eax, -1
        jmp     .return
       @@:

        mov     eax, [MIIData]
        DEBUGF  1,"mii read: 0x%x.\n", eax

  .return:
        pop     edx
        ret





; Input:  offset:word, mask:dword, target:dword, delay:word, delaymax:word, msg:dword
; Output: EAX - 0|1

proc    reg_delay, offset:dword, mask:dword, target:dword, delay:dword, delaymax:dword, msg:dword

;        DEBUGF  1,"reg_delay\n"

        push    esi
        call    pci_push

  .loop:
        mov     esi, [delay]
        call    Sleep
        mov     eax, [delaymax]
        sub     eax, [delay]
        mov     [delaymax], eax

        cmp     eax, 0
        jl      .fail

        mov     eax, [offset]
        mov     eax, [edi + eax]
        and     eax, [mask]
        cmp     eax, [target]
        jne     .loop

        pop     esi
        xor     eax, eax
        ret

  .fail:
        pop     esi
        xor     eax, eax
        inc     eax
        ret

endp





; Input:  none
; Output: EAX - result (0 = OK, other = error)
phy_init:

        push    ebx ecx
        
        ; set advertise register
        mov     edx, [device.phyaddr]
        mov     eax, MII_ADVERTISE
        mov     ecx, MII_READ
        call    mii_rw

        or      eax, (ADVERTISE_10HALF or ADVERTISE_10FULL or ADVERTISE_100HALF or ADVERTISE_100FULL or 0x800 or 0x400)

        mov     ecx, eax
        mov     eax, MII_ADVERTISE
        call    mii_rw

        test    eax, eax
        jz      @f

        DEBUGF  1,"phy write to advertise failed.\n"

        mov     eax, PHY_ERROR
        jmp     .return
       @@:

        ; get phy interface type
        mov     edi, [device.mmio_addr]
        mov     eax, [PhyInterface]
        DEBUGF  1,"phy interface type = 0x%x\n", eax:8

        ; see if gigabit phy
        mov     eax, MII_BMSR
        mov     ecx, MII_READ
        call    mii_rw
        
        test    eax, PHY_GIGABIT
        jnz     .gigabit
        mov     [device.gigabit], 0
        jmp     .next_if

  .gigabit:
        mov     [device.gigabit], PHY_GIGABIT

        mov     eax, MII_1000BT_CR
        mov     ecx, MII_READ
        call    mii_rw
        
        and     eax, (not ADVERTISE_1000HALF)

        test    [PhyInterface], PHY_RGMII
        jz      @f
        or      eax, ADVERTISE_1000FULL
        jmp     .next
       @@:

        and     eax, (not ADVERTISE_1000FULL)

  .next:
        mov     ecx, eax
        mov     eax, MII_1000BT_CR
        call    mii_rw

        test    eax, eax
        jz      .next_if

        DEBUGF 1,"phy init failed.\n"

        mov     eax, PHY_ERROR
        jmp     .return

  .next_if:

        call    phy_reset
        test    eax, eax
        jz      @f

        DEBUGF 1,"phy reset failed.\n"

        mov     eax, PHY_ERROR
        jmp     .return
       @@:

        ; phy vendor specific configuration
        cmp     [device.phy_oui], PHY_OUI_CICADA
        jne     .next_if2
        test    [PhyInterface], PHY_RGMII
        jz      .next_if2

        mov     eax, MII_RESV1
        mov     ecx, MII_READ
        call    mii_rw

        and     eax, (not (PHY_INIT1 or PHY_INIT2))
        or      eax, (PHY_INIT3 or PHY_INIT4)
        mov     ecx, eax
        mov     eax, MII_RESV1
        call    mii_rw

        test    eax, eax
        jz      @f

        DEBUGF 1,"phy init failed.\n"

        mov     eax, PHY_ERROR
        jmp     .return
       @@:

        mov     eax, MII_NCONFIG
        mov     ecx, MII_READ
        call    mii_rw

        or      eax, PHY_INIT5
        mov     ecx, eax
        mov     eax, MII_NCONFIG
        call    mii_rw
        test    eax, eax
        jz      .next_if2

        DEBUGF 1,"phy init failed.\n"

        mov     eax, PHY_ERROR
        jmp     .return



  .next_if2:

        cmp     [device.phy_oui], PHY_OUI_CICADA
        jne     .restart
        
        mov     eax, MII_SREVISION
        mov     ecx, MII_READ
        call    mii_rw
        
        or      eax, PHY_INIT6
        mov     ecx, eax
        mov     eax, MII_SREVISION
        call    mii_rw
        test    eax, eax
        jz      .restart

        DEBUGF 1,"phy init failed.\n"

        jmp     .return

  .restart:
        ; restart auto negotiation

        mov     eax, MII_BMCR
        mov     ecx, MII_READ
        call    mii_rw

        or      eax, (BMCR_ANRESTART or BMCR_ANENABLE)
        mov     ecx, eax
        mov     eax, MII_BMCR
        call    mii_rw
        test    eax, eax
        jz      .ok

        mov     eax, PHY_ERROR
        jmp     .return

  .ok:
        xor     eax, eax
  .return:
        pop     ecx ebx

        ret


; Input:  none
; Output: EAX - result (0 = OK, other = error)
phy_reset:

        DEBUGF  1,"phy_reset\n"

        push    ebx ecx edx

        mov     edx, [device.phyaddr]
        mov     eax, MII_BMCR
        mov     ecx, MII_READ
        call    mii_rw

        or      eax, BMCR_RESET
        push    eax
        mov     ecx, eax
        mov     eax, MII_BMCR
        call    mii_rw

        test    eax, eax
        jz      @f

        pop     eax
        mov     eax, 0xffffffff
        jmp     .return
       @@:

        pop     eax

        mov     esi, 500
        call    Sleep

        ; must wait till reset is deasserted
        mov     esi, 100        ; FIXME: 100 tries seem excessive
  .while_loop:
        test    eax, BMCR_RESET
        jz      .while_loop_exit

        push    esi
        mov     esi, 10
        call    Sleep
        pop     esi

        mov     eax, MII_BMCR
        mov     ecx, MII_READ
        call    mii_rw

        dec     esi
        jnz     .while_loop

        mov     eax, 0xffffffff
        jmp     .return

  .while_loop_exit:
        xor     eax, eax
  .return:
        pop     edx ecx ebx

        ret


align 4
pci_push:

        push    eax
        mov     eax, [edi]
        pop     eax

        ret




align 4
mac_reset:

        push    esi edi

        DEBUGF  1,"mac_reset.\n"

        mov     edi, [device.mmio_addr]
        mov     eax, [device.desc_ver]
        or      eax, (TXRXCTL_BIT2 or TXRXCTL_RESET)
        mov     [TxRxControl], eax
        call    pci_push

        mov     [MacReset], MAC_RESET_ASSERT
        call    pci_push

        mov     esi, NV_MAC_RESET_DELAY
        call    Sleep

        mov     [MacReset], 0
        call    pci_push

        mov     esi, NV_MAC_RESET_DELAY
        call    Sleep

        mov     eax, [device.desc_ver]
        or      eax, TXRXCTL_BIT2
        mov     [TxRxControl], eax
        call    pci_push

        pop     edi esi

        ret





align 4
init_ring:

        DEBUGF  1,"init rings\n"
        push    eax esi ecx

        mov     [device.next_tx], 0

        mov     ecx, TX_RING
        lea     esi, [device.tx_ring]
  .tx_loop:
        mov     [esi + TxDesc.FlagLen], 0
        add     esi, sizeof.TxDesc
        dec     ecx
        jnz     .tx_loop

        mov     [device.cur_rx], 0

        mov     ecx, RX_RING
        lea     esi, [device.rx_ring]
  .rx_loop:
        push    ecx esi
        stdcall KernelAlloc, 4096 shl RBLEN             ; push/pop esi not needed, but just in case...
        pop     esi
        mov     [esi + RX_RING*sizeof.RxDesc], eax
        GetRealAddr
        mov     [esi + RxDesc.PacketBuffer], eax
        mov     [esi + RxDesc.FlagLen], (4096 shl RBLEN or NV_RX_AVAIL)
        add     esi, sizeof.RxDesc
        pop     ecx
        dec     ecx
        jnz     .rx_loop
        
        pop     ecx esi eax

        ret





; Input:  none
; Output: none
align 4
txrx_reset:

        push    eax esi

        DEBUGF 1,"txrx_reset\n"

        mov     edi, [device.mmio_addr]
        mov     eax, [device.desc_ver]
        or      eax, (TXRXCTL_BIT2 or TXRXCTL_RESET)
        mov     [TxRxControl], eax
        call    pci_push

        mov     esi, NV_TXRX_RESET_DELAY
        call    Sleep

        mov     eax, [device.desc_ver]
        or      eax, TXRXCTL_BIT2
        mov     [TxRxControl], eax
        call    pci_push

        pop     esi eax

        ret





; Input:  none
; Output: none
set_multicast:

        ; u32 addr[2];
        ; u32 mask[2];
        ; u32 pff;
        ; u32 alwaysOff[2];
        ; u32 alwaysOn[2];
        ;
        ; memset(addr, 0, sizeof(addr));
        ; memset(mask, 0, sizeof(mask));
        ;
        ; pff = PFF_MYADDR;
        ;
        ; alwaysOn[0] = alwaysOn[1] = alwaysOff[0] = alwaysOff[1] = 0;
        ;
        ; addr[0] = alwaysOn[0];
        ; addr[1] = alwaysOn[1];
        ; mask[0] = alwaysOn[0] | alwaysOff[0];
        ; mask[1] = alwaysOn[1] | alwaysOff[1];
        ;
        ; addr[0] |= MCASTADDRA_FORCE;
        ; pff |= PFF_ALWAYS;

        call    stop_rx

        mov     edi, [device.mmio_addr]
        mov     [MulticastAddrA], MCASTADDRA_FORCE

        mov     [MulticastAddrB], 0
        mov     [MulticastMaskA], 0
        mov     [MulticastMaskB], 0
        mov     [PacketFilterFlags], (PFF_MYADDR or PFF_ALWAYS)

        call    start_rx

        ret





; Input:  none
; Output: none
start_rx:

        push    edi

        DEBUGF  1,"start_rx\n"

        ; Already running? Stop it.
        mov     edi, [device.mmio_addr]
        mov     eax, [ReceiverControl]
        test    eax, RCVCTL_START
        jz      @f
        mov     [ReceiverControl], 0
        call    pci_push
       @@:

        mov     eax, [device.linkspeed]
        mov     [LinkSpeed], eax
        call    pci_push

        mov     [ReceiverControl], RCVCTL_START
        call    pci_push

        pop     edi

        ret




; Input:  none
; Output: none
stop_rx:

        push    esi edi

        DEBUGF 1,"stop_rx.\n"

        mov     edi, [device.mmio_addr]
        mov     [ReceiverControl], 0

        push    ebx edx edi
        stdcall reg_delay, ReceiverStatus-edi, RCVSTAT_BUSY, 0, NV_RXSTOP_DELAY1, NV_RXSTOP_DELAY1MAX, 0
        pop     edi edx ebx

        mov     esi, NV_RXSTOP_DELAY2
        call    Sleep

        mov     [LinkSpeed], 0

        pop     edi esi

        ret




; Input:  none
; Output: EAX
update_linkspeed:

        DEBUGF  1,"update linkspeed\n"

; BMSR_LSTATUS is latched, read it twice: we want the current value.
        
        mov     edx, [device.phyaddr]
        mov     eax, MII_BMSR
        mov     ecx, MII_READ
        call    mii_rw

        mov     eax, MII_BMSR
        mov     ecx, MII_READ
        call    mii_rw
        
        test    eax, BMSR_LSTATUS               ; Link up?
        jz      .10mbit_hd

        test    eax, BMSR_ANEGCOMPLETE          ; still in autonegotiation?
        jz      .10mbit_hd

        cmp     [device.gigabit], PHY_GIGABIT
        jne     .no_gigabit

        ;mov     edx, [device.phyaddr]
        mov     eax, MII_1000BT_CR
        mov     ecx, MII_READ
        call    mii_rw
        push    eax

        ;mov     edx, [device.phyaddr]
        mov     eax, MII_1000BT_SR
        mov     ecx, MII_READ
        call    mii_rw
        pop     ecx

        test    eax, LPA_1000FULL
        jz      .no_gigabit
        test    ecx, ADVERTISE_1000FULL
        jz      .no_gigabit

        DEBUGF  1,"update_linkspeed: GBit ethernet detected.\n"
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_1000)
        xor     eax, eax
        inc     eax
        jmp     set_speed
  .no_gigabit:

        ;mov     edx, [device.phyaddr]
        mov     eax, MII_ADVERTISE
        mov     ecx, MII_READ
        call    mii_rw        ; adv = eax
        push    eax

        ;mov     edx, [device.phyaddr]
        mov     eax, MII_LPA
        mov     ecx, MII_READ
        call    mii_rw        ; lpa = eax
        pop     ecx

        DEBUGF  1,"PHY advertises 0x%x, lpa 0x%x\n", ecx, eax
        and     eax, ecx                ; FIXME: handle parallel detection properly, handle gigabit ethernet

        test    eax, LPA_100FULL
        jz      @f
        DEBUGF  1,"update_linkspeed: 100 mbit full duplex\n"
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_100)
        xor     eax, eax
        inc     eax
        jmp     set_speed
       @@:

        test    eax, LPA_100HALF
        jz      @f
        DEBUGF  1,"update_linkspeed: 100 mbit half duplex\n"
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_100)
        xor     eax, eax
        jmp     set_speed
       @@:

        test    eax, LPA_10FULL
        jz      @f
        DEBUGF  1,"update_linkspeed: 10 mbit full duplex\n"
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_10)
        xor     eax, eax
        inc     eax
        jmp     set_speed
       @@:

  .10mbit_hd:
        DEBUGF  1,"update_linkspeed: 10 mbit half duplex\n"
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_10)
        xor     eax, eax
        jmp     set_speed


align 4
set_speed:

        cmp     eax, [device.duplex]
        jne     .update
        cmp     ecx, [device.linkspeed]
        jne     .update

        ret

  .update:
        DEBUGF 1,"update_linkspeed: changing link to 0x%x/XD.\n", ecx
        
        mov     [device.duplex], eax
        mov     [device.linkspeed], ecx
        
        cmp     [device.gigabit], PHY_GIGABIT
        jne     .no_gigabit

        mov     edi, [device.mmio_addr]
        mov     eax, [RandomSeed]

        and     eax, not (0x3FF00)
        mov     ecx, eax                ; phyreg = ecx

        mov     eax, [device.linkspeed]
        and     eax, 0xFFF
        cmp     eax, LINKSPEED_10
        jne     @f
        or      ecx, RNDSEED_FORCE3
        jmp     .end_if4
       @@:

        cmp     eax, LINKSPEED_100
        jne     @f
        or      ecx, RNDSEED_FORCE2
        jmp     .end_if4
       @@:

        cmp     eax, LINKSPEED_1000
        jne     .end_if4
        or      ecx, RNDSEED_FORCE
  .end_if4:
        mov     [RandomSeed], ecx
  .no_gigabit:

        mov     ecx, [PhyInterface]
        and     ecx, not (PHY_HALF or PHY_100 or PHY_1000)

        cmp     [device.duplex], 0
        jne     @f
        or      ecx, PHY_HALF
       @@:

        mov     eax, [device.linkspeed]
        and     eax, 0xFFF
        cmp     eax, LINKSPEED_100
        jne     @f
        or      ecx, PHY_100
        jmp     .end_if5
       @@:

        cmp     eax, LINKSPEED_1000
        jne     .end_if5
        or      ecx, PHY_1000

  .end_if5:
        mov     [PhyInterface], ecx
                
        cmp     [device.duplex], 0
        je      @f
        xor     ecx, ecx
        jmp     .next
       @@:

        mov     ecx, MISC1_HD
  .next:
        or      ecx, MISC1_FORCE
        mov     [Misc1], ecx

        call    pci_push

        mov     eax, [device.linkspeed]
        mov     [LinkSpeed], eax

        call    pci_push

        ret






align 4
read_mac:

        mov     edi, [device.mmio_addr]

        mov     eax, [MacAddrA]
        mov     ecx, [MacAddrB]

        mov     dword [device.mac], eax
        mov     word [device.mac + 4], cx

        cmp     [device.device_id], 0x03E5
        jae     @f
        bswap   eax
        xchg    cl, ch
        mov     dword [device.mac + 2], eax
        mov     word [device.mac], cx
       @@:

        DEBUGF 1,"MAC = %x-%x-%x-%x-%x-%x\n", \
        [device.mac+0]:2,[device.mac+1]:2,[device.mac+2]:2,[device.mac+3]:2,[device.mac+4]:2,[device.mac+5]:2

        ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: buffer pointer in [esp+4]           ;;
;;     size of buffer in [esp+8]           ;;
;;     pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
transmit:
        DEBUGF  2,"\nTransmitting packet, buffer:%x, size:%u\n", [esp+4], [esp+8]
        mov     eax, [esp+4]
        DEBUGF  2,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
        [eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
        [eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
        [eax+13]:2,[eax+12]:2

        cmp     dword [esp + 8], 1514
        ja      .fail
        cmp     dword [esp + 8], 60
        jb      .fail

; get the descriptor address
        mov     eax, [device.next_tx]
        mov     cl, sizeof.TxDesc
        mul     cl
        lea     esi, [device.tx_ring + eax]
        mov     eax, [esp + 4]
        mov     [esi + TX_RING*sizeof.TxDesc], eax
        GetRealAddr
        mov     [esi + TxDesc.PacketBuffer], eax

        mov     ecx, [esp + 8]
        or      ecx, [device.txflags]
        mov     [esi + TxDesc.FlagLen], eax

        mov     edi, [device.mmio_addr]
        mov     eax, [device.desc_ver]
        or      eax, TXRXCTL_KICK
        mov     [TxRxControl], eax

        call    pci_push

        inc     [device.next_tx]
        and     [device.next_tx], (TX_RING-1)

; Update stats
        inc     [device.packets_tx]
        mov     eax, [esp + 8]
        add     dword [device.bytes_tx], eax
        adc     dword [device.bytes_tx + 4], 0

        xor     eax, eax
        ret     8

  .fail:
        xor     eax, eax
        inc     eax
        ret     8






; Interrupt handler
align 4
int_handler:
        DEBUGF  2,"\n%s INT\n", my_service

;-------------------------------------------
; Find pointer of device wich made IRQ occur

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .fail
  .nextdevice:
        mov     ebx, dword [esi]
        add     esi, 4

        mov     edi, [device.mmio_addr]
        mov     eax, [IrqStatus]
        test    eax, eax
        jnz     .got_it
        dec     ecx
        jnz     .nextdevice
        ret

  .got_it:
        mov     [IrqStatus], eax
        DEBUGF  2,"IrqStatus = %x\n", eax

        test    eax, IRQ_RX
        jz      .no_rx

  .top:
        mov     eax, [device.cur_rx]
        mov     cx, sizeof.RxDesc
        mul     cx
        lea     esi, [device.rx_ring + eax]
        mov     eax, [esi + RxDesc.FlagLen]

        test    eax, NV_RX_AVAIL        ; still owned by hardware
        jnz     .return0

        cmp     [device.desc_ver], DESC_VER_1
        jne     @f
        test    eax, NV_RX_DESCRIPTORVALID
        jz      .return0
        jmp     .next
  @@:
        test    eax, NV_RX2_DESCRIPTORVALID
        jz      .return0

  .next:

        cmp     dword [device.desc_ver], DESC_VER_1
        jne     @f
        and     eax, LEN_MASK_V1
        jmp     .next2
   @@:
        and     eax, LEN_MASK_V2
  .next2:

        ; got a valid packet - forward it to the network core
        push    .top
        push    eax
        push    dword [esi + RX_RING*sizeof.RxDesc]

        inc     [device.cur_rx]
        and     [device.cur_rx], (RX_RING-1)

; Allocate new buffer

        stdcall KernelAlloc, 4096 shl RBLEN
        mov     [esi + RX_RING*sizeof.RxDesc], eax
        GetRealAddr
        mov     [esi + RxDesc.PacketBuffer], eax
        mov     [esi + RxDesc.FlagLen], (4096 shl RBLEN or NV_RX_AVAIL)

        jmp     Eth_input

  .return0:


  .no_rx:
        test    eax, IRQ_RX_ERROR
        jz      .no_rx_err

        push    eax
        DEBUGF  2,"RX error!\n"

        mov     eax, [device.cur_rx]
        mov     cx, sizeof.RxDesc
        mul     cx
        lea     esi, [device.rx_ring + eax]
        mov     eax, [esi + RxDesc.FlagLen]

        DEBUGF  2,"Flaglen=%x\n", eax

        ; TODO: allocate new buff
        pop     eax

  .no_rx_err:
        test    eax, IRQ_LINK
        jz      .no_link

        push    eax
        call    update_linkspeed
        pop     eax

  .no_link:
  .fail:

        ret




; End of code

section '.data' data readable writable align 16 ; place all uninitialized data place here
align 4                                         ; Place all initialised data here

devices         dd 0
version         dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service      db 'FORCEDETH',0                ; max 16 chars include zero

include_debug_strings                           ; All data wich FDO uses will be included here

device_list     rd MAX_DEVICES                  ; This list contains all pointers to device structures the driver is handling
