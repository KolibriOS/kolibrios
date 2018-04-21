;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2018. All rights reserved.    ;;
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


format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

        MAX_DEVICES             = 16

        RBLEN                   = 0     ; Receive buffer size: 0=4K 1=8k 2=16k 3=32k 4=64k
                                        ; FIXME: option 1 and 2 may allocate a non contiguous buffer causing data loss!

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2

        RX_RING                 = 4
        TX_RING                 = 4

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'
include '../mii.inc'

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
IRQ_TX_ERROR              = 0x0008
IRQ_TX_OK                 = 0x0010
IRQ_TIMER                 = 0x0020
IRQ_LINK                  = 0x0040
IRQ_RX_FORCED             = 0x0080
IRQ_TX_FORCED             = 0x0100
IRQ_RECOVER_ERROR         = 0x8200                                           ;
IRQMASK_WANTED_2          = IRQ_TX_FORCED + IRQ_LINK + IRQ_RX_ERROR + IRQ_RX + IRQ_TX_OK + IRQ_TX_ERROR

IRQ_RX_ALL                = IRQ_RX_ERROR or IRQ_RX or IRQ_RX_NOBUF or IRQ_RX_FORCED
IRQ_TX_ALL                = IRQ_TX_ERROR or IRQ_TX_OK or IRQ_TX_FORCED
IRQ_OTHER                 = IRQ_LINK or IRQ_TIMER or IRQ_RECOVER_ERROR

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

struct  TxDesc
        PacketBuffer            dd ?
        FlagLen                 dd ?
ends

struct  RxDesc
        PacketBuffer            dd ?
        FlagLen                 dd ?
ends

struct  device                  ETH_DEVICE

        pci_bus                 dd ?
        pci_dev                 dd ?

        mmio_addr               dd ?
        vendor_id               dw ?
        device_id               dw ?
        txflags                 dd ?
        desc_ver                dd ?
        irqmask                 dd ?
        wolenabled              dd ?
        in_shutdown             dd ?
        cur_rx                  dd ?
        cur_tx                  dd ?
        last_tx                 dd ?
        phyaddr                 dd ?
        phy_oui                 dd ?
        gigabit                 dd ?
        needs_mac_reset         dd ?
        linkspeed               dd ?
        duplex                  dd ?
        nocable                 dd ?

                                rb 0x100 - ($ and 0xff)
        tx_ring                 rd (TX_RING * sizeof.TxDesc) /4*2

                                rb 0x100 - ($ and 0xff)
        rx_ring                 rd (RX_RING * sizeof.RxDesc) /4*2

ends



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
        mov     ax, [eax+1]
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte [ebx + device.pci_bus]               ; compare with pci and device num in device list (notice the usage of word instead of byte)
        jne     @f
        cmp     ah, byte [ebx + device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
  @@:
        add     esi, 4
        loop    .nextdevice


; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
        cmp     [devices], MAX_DEVICES                  ; First check if the driver can handle one more card
        jae     .fail

        allocate_and_clear ebx, sizeof.device, .fail      ; Allocate the buffer for device structure

; Fill in the direct call addresses into the struct

        mov     [ebx + device.reset], reset
        mov     [ebx + device.transmit], transmit
        mov     [ebx + device.unload], .fail
        mov     [ebx + device.name], my_service

; save the pci bus and device numbers

        mov     eax, [edx + IOCTL.input]
        movzx   ecx, byte [eax+1]
        mov     [ebx + device.pci_bus], ecx
        movzx   ecx, byte [eax+2]
        mov     [ebx + device.pci_dev], ecx

        DEBUGF  1,"Hooking into device, dev:%x, bus:%x\n", [ebx + device.pci_dev], [ebx + device.pci_bus]

; Ok, the eth_device structure is ready, let's probe the device
        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err                                                    ; If an error occured, exit

        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        mov     [ebx + device.type], NET_TYPE_ETH
        invoke  NetRegDev

        cmp     eax, -1
        je      .destroy

        ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
        DEBUGF  1,"Trying to find device number of already registered device\n"
        invoke  NetPtrToNum                                             ; This kernel procedure converts a pointer to device struct in ebx
                                                                        ; into a device number in edi
        mov     eax, edi                                                ; Application wants it in eax instead
        DEBUGF  1,"Kernel says: %u\n", eax
        ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
        ; todo: reset device into virgin state

  .err:
        invoke  KernelFree, ebx
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

        mov     [ebx + device.needs_mac_reset], 0

; Make the device a bus master and enable response in I/O space
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      al, PCI_CMD_MASTER + PCI_CMD_PIO ; + PCI_CMD_MMIO
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

; Adjust PCI latency to be at least 32
        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.max_latency
        cmp     al, 32
        jae     @f
        mov     al, 32
        invoke  PciWrite8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.max_latency, eax
  @@:

; Now, it's time to find the base mmio addres of the PCI device
        stdcall PCI_find_mmio, [ebx + device.pci_bus], [ebx + device.pci_dev] ; returns in eax
        test    eax, eax
        jnz     @f
        DEBUGF 1, "No useable MMIO addresses found!\n"
        dec     eax
        ret
  @@:
        DEBUGF 1,"mmio_addr= 0x%x\n", eax

; Create virtual mapping of the physical memory
        invoke  MapIoMem, eax, 10000h, PG_SW + PG_NOCACHE
        test    eax, eax
        jz      fail
        mov     [ebx + device.mmio_addr], eax
        DEBUGF 1,"mapped mmio_addr= 0x%x\n", eax

; Read PCI vendor/device ID
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.vendor_id
        mov     dword[ebx + device.vendor_id], eax
        DEBUGF 1,"vendor = 0x%x\n", [ebx + device.vendor_id]:4
        DEBUGF 1,"device = 0x%x\n", [ebx + device.device_id]:4

;-------------------------------------
; handle different descriptor versions
        mov     [ebx + device.desc_ver], DESC_VER_1
        mov     ax, [ebx + device.device_id]
        cmp     ax, PCI_DEVICE_ID_NVIDIA_NVENET_1
        je      .ver1
        cmp     ax, PCI_DEVICE_ID_NVIDIA_NVENET_2
        je      .ver1
        cmp     ax, PCI_DEVICE_ID_NVIDIA_NVENET_3
        je      .ver1
        mov     [ebx + device.desc_ver], DESC_VER_2
  .ver1:

        call    read_mac

        ; disable WOL
        mov     [WakeUpFlags], 0
        mov     [ebx + device.wolenabled], 0
        
        mov     [ebx + device.txflags], (NV_TX2_LASTPACKET or NV_TX2_VALID)
        cmp     [ebx + device.desc_ver], DESC_VER_1
        jne     @f
        mov     [ebx + device.txflags], (NV_TX_LASTPACKET or NV_TX_VALID)
      @@:

; BEGIN of switch (pci->dev_id)

        cmp     [ebx + device.device_id], 0x01C3
        jne     .not_0x01c3
        ; nforce
        mov     [ebx + device.irqmask], (IRQMASK_WANTED_2 or IRQ_TIMER)         ;;; Was 0
        jmp     .find_phy
  .not_0x01c3:

        cmp     [ebx + device.device_id], 0x0066
        je      @f
        cmp     [ebx + device.device_id], 0x00D6
        jne     .not_0x0066
  @@:
        mov     [ebx + device.irqmask], (IRQMASK_WANTED_2 or IRQ_TIMER)         ;;;; was 0
        cmp     [ebx + device.desc_ver], DESC_VER_1
        jne     @f
        or      [ebx + device.txflags], NV_TX_LASTPACKET1
        jmp     .find_phy
  @@:
        or      [ebx + device.txflags], NV_TX2_LASTPACKET1
        jmp     .find_phy
  .not_0x0066:

        cmp     [ebx + device.device_id], 0x0086
        je      @f
        cmp     [ebx + device.device_id], 0x008c
        je      @f
        cmp     [ebx + device.device_id], 0x00e6
        je      @f
        cmp     [ebx + device.device_id], 0x00df
        je      @f
        cmp     [ebx + device.device_id], 0x0056
        je      @f
        cmp     [ebx + device.device_id], 0x0057
        je      @f
        cmp     [ebx + device.device_id], 0x0037
        je      @f
        cmp     [ebx + device.device_id], 0x0038
        jne     .not_0x0086

      @@:
        mov     [ebx + device.irqmask], (IRQMASK_WANTED_2 or IRQ_TIMER) ;;; was 0

        cmp     [ebx + device.desc_ver], DESC_VER_1
        jne     @f
        or      [ebx + device.txflags], NV_TX_LASTPACKET1
        jmp     .find_phy
       @@:
        or      [ebx + device.txflags], NV_TX2_LASTPACKET1
        jmp     .find_phy
  .not_0x0086:

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
        cmp     [ebx + device.device_id], 0x0268
        jb      .undefined

; Get device revision
        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header.revision_id
        mov     edi, [ebx + device.mmio_addr]   ;;;;;

; take phy and nic out of low power mode
        mov     ecx, [PowerState2]
        and     ecx, not POWERSTATE2_POWERUP_MASK
        cmp     [ebx + device.device_id], PCI_DEVICE_ID_NVIDIA_NVENET_12
        je      @f
        cmp     [ebx + device.device_id], PCI_DEVICE_ID_NVIDIA_NVENET_13
        jne     .set_powerstate
  @@:
        cmp     al, 0xA3
        jb      .set_powerstate
        or      ecx, POWERSTATE2_POWERUP_REV_A3
  .set_powerstate:
        mov     [PowerState2], ecx

        ; DEV_NEED_LASTPACKET1|DEV_IRQMASK_2|DEV_NEED_TIMERIRQ
        mov     [ebx + device.irqmask], (IRQMASK_WANTED_2 or IRQ_TIMER)         ; was 0
        
        mov     [ebx + device.needs_mac_reset], 1
        cmp     [ebx + device.desc_ver], DESC_VER_1
        jne     @f
        or      [ebx + device.txflags], NV_TX_LASTPACKET1
        jmp     .find_phy

       @@:
        cmp     [ebx + device.desc_ver], DESC_VER_2
        jne     .undefined
        or      [ebx + device.txflags], NV_TX2_LASTPACKET1
        jmp     .find_phy

  .undefined:
        DEBUGF  2,"Your card was undefined in this driver.\n"
        or      eax, -1
        ret

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

        cmp     eax, 0x0000ffff
        je      .try_next
        test    eax, 0x80000000
        jnz     .try_next
        mov     esi, eax

        mov     eax, MII_PHYSID2
        mov     ecx, MII_READ
        call    mii_rw

        cmp     eax, 0x0000ffff
        je      .try_next
        test    eax, 0x80000000
        jnz     .try_next
        jmp     .got_it

  .try_next:
        test    edx, edx
        jnz     .phy_loop

        ; PHY in isolate mode? No phy attached and user wants to test loopback?
        ; Very odd, but can be correct.
        
        DEBUGF  2,"Could not find a valid PHY.\n"
        jmp     .no_phy

  .got_it:
        and     esi, PHYID1_OUI_MASK
        shl     esi, PHYID1_OUI_SHFT
        and     eax, PHYID2_OUI_MASK
        shr     eax, PHYID2_OUI_SHFT
        or      eax, esi

        mov     [ebx + device.phyaddr], edx
        mov     [ebx + device.phy_oui], eax

        DEBUGF 1,"Found PHY with OUI:0x%x at address:0x%x\n", eax, edx

        call    phy_init

  .no_phy:

        cmp     [ebx + device.needs_mac_reset], 0
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

        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.interrupt_line
        movzx   eax, al
        invoke  AttachIntHandler, eax, int_handler, ebx
        test    eax, eax
        jnz     @f
        DEBUGF  2,"Could not attach int handler!\n"
        or      eax, -1
        ret
       @@:

; erase previous misconfiguration

        mov     edi, [ebx + device.mmio_addr]
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
        mov     [ebx + device.in_shutdown], 0

; give hw rings

        lea     eax, [ebx + device.rx_ring]
        invoke  GetPhysAddr
        mov     [RxRingPhysAddr], eax

        lea     eax, [ebx + device.tx_ring]
        invoke  GetPhysAddr
        mov     [TxRingPhysAddr], eax

        mov     [RingSizes], (((RX_RING - 1) shl RINGSZ_RXSHIFT) + ((TX_RING - 1) shl RINGSZ_TXSHIFT))

;

        mov     [ebx + device.linkspeed], (LINKSPEED_FORCE or LINKSPEED_10)
        mov     [ebx + device.duplex], 0
        mov     [LinkSpeed], (LINKSPEED_FORCE or LINKSPEED_10)
        mov     [UnknownSetupReg3], UNKSETUP3_VAL1

        mov     eax, [ebx + device.desc_ver]
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
        invoke  GetTimerTicks   ; bad idea, driver is started at system startup in 90% of cases..
        pop     ebx

        mov     edi, [ebx + device.mmio_addr]

        and     eax, RNDSEED_MASK
        or      eax, RNDSEED_FORCE
        mov     [RandomSeed], eax

        mov     [UnknownSetupReg1], UNKSETUP1_VAL
        mov     [UnknownSetupReg2], UNKSETUP2_VAL
        mov     [PollingInterval], POLL_DEFAULT
        mov     [UnknownSetupReg6], UNKSETUP6_VAL

        mov     eax, [ebx + device.phyaddr]
        shl     eax, ADAPTCTL_PHYSHIFT
        or      eax, (ADAPTCTL_PHYVALID or ADAPTCTL_RUNNING)
        mov     [AdapterControl], eax

        mov     [MIISpeed], (MIISPEED_BIT8 or MIIDELAY)
        mov     [UnknownSetupReg4], UNKSETUP4_VAL
        mov     [WakeUpFlags], WAKEUPFLAGS_VAL
        
        or      [PowerState], POWERSTATE_POWEREDUP
        call    pci_push

        mov     esi, 10
        invoke  Sleep

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

        mov     [ebx + device.nocable], 0
        test    eax, eax
        jnz     .return
        DEBUGF  1,"no link during initialization.\n"
        mov     [ebx + device.nocable], 1

  .return:
        xor     eax, eax        ; Indicate that we have successfully reset the card
        mov     [ebx + device.mtu], 1514 ;;; FIXME
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
; Input:  EAX - miireg, EDX - phy addr, ECX - value to write (or -1 to read)
; Output: EAX - retval (lower 16 bits)
;
;--------------------------------------------------------

mii_rw:

        DEBUGF  1,"mii_rw: 0x%x to reg %d at PHY %d\n", ecx, eax, edx

        push    edx

        mov     edi, [ebx + device.mmio_addr]

; Check if MII interface is busy
        mov     [MIIStatus], MIISTAT_MASK
       @@:
        test    [MIIControl], MIICTL_INUSE
        jz      @f
        mov     [MIIControl], MIICTL_INUSE

        DEBUGF  1,"mii_rw: in use!\n"
        pusha
        mov     esi, NV_MIIBUSY_DELAY
        invoke  Sleep
        popa
        jmp     @r
       @@:

; Set the address we want to access
        shl     edx, MIICTL_ADDRSHIFT
        or      edx, eax

        ; When writing, write the data first.
        cmp     ecx, MII_READ
        je      @f
        mov     [MIIData], ecx
        or      edx, MIICTL_WRITE
       @@:

        mov     [MIIControl], edx

; Wait for read/write to complete
        stdcall reg_delay, MIIControl-edi, MIICTL_INUSE, 0, NV_MIIPHY_DELAY, NV_MIIPHY_DELAYMAX, 0

        test    eax, eax
        jz      @f
        DEBUGF  1,"mii_rw timed out.\n"
        or      eax, -1
        jmp     .return

       @@:
        cmp     ecx, MII_READ
        je      .read
; it was a write operation - fewer failures are detectable
        DEBUGF  1,"mii_rw write: ok\n"
        xor     eax, eax
        jmp     .return

  .read:
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
        invoke  Sleep
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
        mov     edx, [ebx + device.phyaddr]
        mov     eax, MII_ADVERTISE
        mov     ecx, MII_READ
        call    mii_rw

        or      eax, (ADVERTISE_10HALF or ADVERTISE_10FULL or ADVERTISE_100HALF or ADVERTISE_100FULL or 0x800 or 0x400)

        mov     ecx, eax
        mov     eax, MII_ADVERTISE
        call    mii_rw

        test    eax, eax
        jz      @f

        DEBUGF  2,"phy write to advertise failed.\n"

        mov     eax, PHY_ERROR
        jmp     .return
       @@:

        ; get phy interface type
        mov     edi, [ebx + device.mmio_addr]
        mov     eax, [PhyInterface]
        DEBUGF  1,"phy interface type = 0x%x\n", eax:8

        ; see if gigabit phy
        mov     eax, MII_BMSR
        mov     ecx, MII_READ
        call    mii_rw
        
        test    eax, PHY_GIGABIT
        jnz     .gigabit
        mov     [ebx + device.gigabit], 0
        jmp     .next_if

  .gigabit:
        mov     [ebx + device.gigabit], PHY_GIGABIT

        mov     eax, MII_CTRL1000
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
        mov     eax, MII_CTRL1000
        call    mii_rw

        test    eax, eax
        jz      .next_if

        DEBUGF  2,"phy init failed.\n"

        mov     eax, PHY_ERROR
        jmp     .return

  .next_if:

        call    phy_reset
        test    eax, eax
        jz      @f

        DEBUGF  2,"phy reset failed.\n"

        mov     eax, PHY_ERROR
        jmp     .return
       @@:

        ; phy vendor specific configuration
        cmp     [ebx + device.phy_oui], PHY_OUI_CICADA
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

        DEBUGF  2,"phy init failed.\n"

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

        DEBUGF  2,"phy init failed.\n"

        mov     eax, PHY_ERROR
        jmp     .return



  .next_if2:

        cmp     [ebx + device.phy_oui], PHY_OUI_CICADA
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

        DEBUGF  2,"phy init failed.\n"

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

        mov     edx, [ebx + device.phyaddr]
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
        invoke  Sleep

        ; must wait till reset is deasserted
        mov     esi, 100        ; FIXME: 100 tries seem excessive
  .while_loop:
        test    eax, BMCR_RESET
        jz      .while_loop_exit

        push    esi
        mov     esi, 10
        invoke  Sleep
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

        mov     edi, [ebx + device.mmio_addr]
        mov     eax, [ebx + device.desc_ver]
        or      eax, (TXRXCTL_BIT2 or TXRXCTL_RESET)
        mov     [TxRxControl], eax
        call    pci_push

        mov     [MacReset], MAC_RESET_ASSERT
        call    pci_push

        mov     esi, NV_MAC_RESET_DELAY
        invoke  Sleep

        mov     [MacReset], 0
        call    pci_push

        mov     esi, NV_MAC_RESET_DELAY
        invoke  Sleep

        mov     eax, [ebx + device.desc_ver]
        or      eax, TXRXCTL_BIT2
        mov     [TxRxControl], eax
        call    pci_push

        pop     edi esi

        ret



align 4
init_ring:

        DEBUGF  1,"init rings\n"
        push    esi ecx

        mov     [ebx + device.cur_tx], 0
        mov     [ebx + device.last_tx], 0

        mov     ecx, TX_RING
        lea     esi, [ebx + device.tx_ring]
  .tx_loop:
        mov     [esi + TxDesc.FlagLen], 0
        mov     [esi + TxDesc.PacketBuffer], 0
        add     esi, sizeof.TxDesc
        dec     ecx
        jnz     .tx_loop


        mov     [ebx + device.cur_rx], 0

        mov     ecx, RX_RING
        lea     esi, [ebx + device.rx_ring]
  .rx_loop:
        push    ecx esi
        invoke  NetAlloc, (4096 shl RBLEN) + NET_BUFF.data             ; push/pop esi not needed, but just in case...
        pop     esi
        test    eax, eax
        jz      .out_of_mem
        mov     [esi + RX_RING*sizeof.RxDesc], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [esi + RxDesc.PacketBuffer], eax
        mov     [esi + RxDesc.FlagLen], (4096 shl RBLEN or NV_RX_AVAIL)
        add     esi, sizeof.RxDesc
        pop     ecx
        dec     ecx
        jnz     .rx_loop
        
        pop     ecx esi

        xor     eax, eax
        ret

  .out_of_mem:
        add     esp, 12
        or      eax, -1
        ret





; Input:  none
; Output: none
align 4
txrx_reset:

        push    eax esi

        DEBUGF  1,"txrx_reset\n"

        mov     edi, [ebx + device.mmio_addr]
        mov     eax, [ebx + device.desc_ver]
        or      eax, (TXRXCTL_BIT2 or TXRXCTL_RESET)
        mov     [TxRxControl], eax
        call    pci_push

        mov     esi, NV_TXRX_RESET_DELAY
        invoke  Sleep

        mov     eax, [ebx + device.desc_ver]
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

        mov     edi, [ebx + device.mmio_addr]
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
        mov     edi, [ebx + device.mmio_addr]
        mov     eax, [ReceiverControl]
        test    eax, RCVCTL_START
        jz      @f
        mov     [ReceiverControl], 0
        call    pci_push
       @@:

        mov     eax, [ebx + device.linkspeed]
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

        DEBUGF  1,"stop_rx.\n"

        mov     edi, [ebx + device.mmio_addr]
        mov     [ReceiverControl], 0

        push    ebx edx edi
        stdcall reg_delay, ReceiverStatus-edi, RCVSTAT_BUSY, 0, NV_RXSTOP_DELAY1, NV_RXSTOP_DELAY1MAX, 0
        pop     edi edx ebx

        mov     esi, NV_RXSTOP_DELAY2
        invoke  Sleep

        mov     [LinkSpeed], 0

        pop     edi esi

        ret




; Input:  none
; Output: EAX
update_linkspeed:

        DEBUGF  1,"update linkspeed\n"

; BMSR_LSTATUS is latched, read it twice: we want the current value.
        
        mov     edx, [ebx + device.phyaddr]
        mov     eax, MII_BMSR
        mov     ecx, MII_READ
        call    mii_rw

        mov     eax, MII_BMSR
        mov     ecx, MII_READ
        call    mii_rw
        
        test    ax, BMSR_LSTATUS               ; Link up?
        jz      .no_link

        DEBUGF  1,"link is up\n"

        test    ax, BMSR_ANEGCOMPLETE          ; still in autonegotiation?
        jz      .10mbit_hd

        DEBUGF  1,"autonegotiation is complete\n"

        cmp     [ebx + device.gigabit], PHY_GIGABIT
        jne     .no_gigabit

        ;mov     edx, [ebx + device.phyaddr]
        mov     eax, MII_CTRL1000
        mov     ecx, MII_READ
        call    mii_rw
        push    eax

        ;mov     edx, [ebx + device.phyaddr]
        mov     eax, MII_STAT1000
        mov     ecx, MII_READ
        call    mii_rw
        pop     ecx

        test    eax, LPA_1000FULL
        jz      .no_gigabit
        test    ecx, ADVERTISE_1000FULL
        jz      .no_gigabit

        DEBUGF  1,"update_linkspeed: GBit ethernet detected.\n"
        mov     [ebx + device.state], ETH_LINK_1G
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_1000)
        xor     eax, eax
        inc     eax
        jmp     set_speed
  .no_gigabit:

        ;mov     edx, [ebx + device.phyaddr]
        mov     eax, MII_ADVERTISE
        mov     ecx, MII_READ
        call    mii_rw        ; adv = eax
        push    eax

        ;mov     edx, [ebx + device.phyaddr]
        mov     eax, MII_LPA
        mov     ecx, MII_READ
        call    mii_rw        ; lpa = eax
        pop     ecx

        DEBUGF  1,"PHY advertises 0x%x, lpa 0x%x\n", cx, ax
        and     eax, ecx                ; FIXME: handle parallel detection properly, handle gigabit ethernet

        test    eax, LPA_100FULL
        jz      @f
        DEBUGF  1,"update_linkspeed: 100 mbit full duplex\n"
        mov     [ebx + device.state], ETH_LINK_100M + ETH_LINK_FD
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_100)
        xor     eax, eax
        inc     eax
        jmp     set_speed
       @@:

        test    eax, LPA_100HALF
        jz      @f
        DEBUGF  1,"update_linkspeed: 100 mbit half duplex\n"
        mov     [ebx + device.state], ETH_LINK_100M
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_100)
        xor     eax, eax
        jmp     set_speed
       @@:

        test    eax, LPA_10FULL
        jz      @f
        DEBUGF  1,"update_linkspeed: 10 mbit full duplex\n"
        mov     [ebx + device.state], ETH_LINK_10M + ETH_LINK_FD
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_10)
        xor     eax, eax
        inc     eax
        jmp     set_speed
       @@:

  .10mbit_hd:
        DEBUGF  1,"update_linkspeed: 10 mbit half duplex\n"
        mov     [ebx + device.state], ETH_LINK_10M
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_10)
        xor     eax, eax
        jmp     set_speed

  .no_link:
        DEBUGF  1,"update_linkspeed: link is down\n"
        mov     [ebx + device.state], ETH_LINK_DOWN
        mov     ecx, (LINKSPEED_FORCE or LINKSPEED_10)
        xor     eax, eax
        jmp     set_speed


align 4
set_speed:

        cmp     eax, [ebx + device.duplex]
        jne     .update
        cmp     ecx, [ebx + device.linkspeed]
        jne     .update

        ret

  .update:
        DEBUGF  1,"update_linkspeed: changing link to 0x%x/XD.\n", ecx
        
        mov     [ebx + device.duplex], eax
        mov     [ebx + device.linkspeed], ecx
        
        cmp     [ebx + device.gigabit], PHY_GIGABIT
        jne     .no_gigabit

        mov     edi, [ebx + device.mmio_addr]
        mov     eax, [RandomSeed]

        and     eax, not (0x3FF00)
        mov     ecx, eax                ; phyreg = ecx

        mov     eax, [ebx + device.linkspeed]
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

        cmp     [ebx + device.duplex], 0
        jne     @f
        or      ecx, PHY_HALF
       @@:

        mov     eax, [ebx + device.linkspeed]
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
                
        cmp     [ebx + device.duplex], 0
        je      @f
        xor     ecx, ecx
        jmp     .next
       @@:

        mov     ecx, MISC1_HD
  .next:
        or      ecx, MISC1_FORCE
        mov     [Misc1], ecx

        call    pci_push

        mov     eax, [ebx + device.linkspeed]
        mov     [LinkSpeed], eax

        call    pci_push

        ret






align 4
read_mac:

        mov     edi, [ebx + device.mmio_addr]
        mov     eax, [MacAddrA]
        mov     ecx, [MacAddrB]

        mov     dword [ebx + device.mac], eax
        mov     word [ebx + device.mac + 4], cx

        cmp     [ebx + device.device_id], 0x03E5
        jae     @f
        bswap   eax
        xchg    cl, ch
        mov     dword [ebx + device.mac + 2], eax
        mov     word [ebx + device.mac], cx
       @@:

        DEBUGF  1,"MAC = %x-%x-%x-%x-%x-%x\n", \
        [ebx + device.mac+0]:2,[ebx + device.mac+1]:2,[ebx + device.mac+2]:2,[ebx + device.mac+3]:2,[ebx + device.mac+4]:2,[ebx + device.mac+5]:2

        ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: buffer pointer in [esp+4]           ;;
;;     pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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

; get the descriptor address
        mov     eax, [ebx + device.cur_tx]
        shl     eax, 3                                  ; TX descriptor is 8 bytes.
        lea     edi, [ebx + device.tx_ring + eax]

        mov     eax, [bufferptr]
        mov     [edi + TX_RING*sizeof.TxDesc], eax
        add     eax, [eax + NET_BUFF.offset]
        invoke  GetPhysAddr                             ; Does not change esi/ebx :)
        mov     [edi + TxDesc.PacketBuffer], eax

        mov     ecx, [esi + NET_BUFF.length]
        or      ecx, [ebx + device.txflags]
        mov     [edi + TxDesc.FlagLen], ecx

        mov     edi, [ebx + device.mmio_addr]
        mov     eax, [ebx + device.desc_ver]
        or      eax, TXRXCTL_KICK
        mov     [TxRxControl], eax

        call    pci_push

        inc     [ebx + device.cur_tx]
        and     [ebx + device.cur_tx], (TX_RING-1)

; Update stats
        inc     [ebx + device.packets_tx]
        add     dword[ebx + device.bytes_tx], ecx
        adc     dword[ebx + device.bytes_tx + 4], 0

        popf
        xor     eax, eax
        ret

  .fail:
        DEBUGF  2,"Send failed\n"
        invoke  NetFree, [bufferptr]
        popf
        or      eax, -1
        ret

endp






; Interrupt handler
align 4
int_handler:

        push    ebx esi edi

        DEBUGF  1,"INT\n"

;-------------------------------------------
; Find pointer of device wich made IRQ occur

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .fail
  .nextdevice:
        mov     ebx, dword [esi]
        add     esi, 4

        mov     edi, [ebx + device.mmio_addr]
        mov     eax, [IrqStatus]
        test    eax, eax
        jnz     .got_it
        dec     ecx
        jnz     .nextdevice
  .nothing:
        pop     edi esi ebx
        xor     eax, eax

        ret

  .got_it:
        mov     [IrqStatus], eax
        DEBUGF  1,"IrqStatus = %x\n", eax

        test    eax, IRQ_RX ;+ IRQ_TIMER ;;;;
        jz      .no_rx

        push    ebx
  .more_rx:
        pop     ebx
        mov     eax, [ebx + device.cur_rx]
        mov     cx, sizeof.RxDesc
        mul     cx
        lea     esi, [ebx + device.rx_ring + eax]
        mov     ecx, [esi + RxDesc.FlagLen]

        test    ecx, NV_RX_AVAIL        ; still owned by hardware
        jnz     .no_rx

        cmp     [ebx + device.desc_ver], DESC_VER_1
        jne     @f
        test    ecx, NV_RX_DESCRIPTORVALID
        jz      .no_rx
        jmp     .next
  @@:
        test    ecx, NV_RX2_DESCRIPTORVALID
        jz      .no_rx

  .next:
        cmp     dword[ebx + device.desc_ver], DESC_VER_1
        jne     @f
        and     ecx, LEN_MASK_V1
        jmp     .next2
   @@:
        and     ecx, LEN_MASK_V2

  .next2:
        DEBUGF  1,"Received %u bytes\n", ecx

        ; Update stats
        add     dword[ebx + device.bytes_rx], ecx
        adc     dword[ebx + device.bytes_rx + 4], 0
        inc     dword[ebx + device.packets_rx]

        ; Prepare to give packet to kernel
        push    ebx
        push    .more_rx

        mov     eax, [esi + RX_RING*sizeof.RxDesc]
        push    eax
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.length], ecx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data
        DEBUGF  1,"packet ptr=0x%x\n", [esi + RX_RING*sizeof.RxDesc]

        ; Allocate new buffer for this descriptor
        invoke  NetAlloc, (4096 shl RBLEN) + NET_BUFF.data
        mov     [esi + RX_RING*sizeof.RxDesc], eax
        invoke  GetPhysAddr
        add     eax, NET_BUFF.data
        mov     [esi + RxDesc.PacketBuffer], eax
        mov     [esi + RxDesc.FlagLen], (4096 shl RBLEN or NV_RX_AVAIL)

        ; update current RX descriptor
        inc     [ebx + device.cur_rx]
        and     [ebx + device.cur_rx], (RX_RING-1)

        jmp     [EthInput]

  .no_rx:
        test    eax, IRQ_RX_ERROR
        jz      .no_rx_err

        push    eax
        DEBUGF  2,"RX error!\n"

        mov     eax, [ebx + device.cur_rx]
        mov     cx, sizeof.RxDesc
        mul     cx
        lea     esi, [ebx + device.rx_ring + eax]
        mov     eax, [esi + RxDesc.FlagLen]

        DEBUGF  1,"Flaglen=%x\n", eax

        ; TODO: allocate new buff ?
        pop     eax

  .no_rx_err:
        test    eax, IRQ_TX_ERROR
        jz      .no_tx_err

        DEBUGF  2,"TX error!\n"
        ; TODO

  .no_tx_err:
        test    eax, IRQ_LINK
        jz      .no_link

        push    eax
        call    update_linkspeed
        pop     eax

  .no_link:
        test    eax, IRQ_TX_OK
        jz      .no_tx

        DEBUGF  1, "TX completed\n"
      .loop_tx:
        mov     esi, [ebx + device.last_tx]
        shl     esi, 3                                  ; TX descriptor is 8 bytes.
        lea     esi, [ebx + device.tx_ring + esi]

        DEBUGF  1,"Flaglen = 0x%x\n", [esi + TxDesc.FlagLen]
        test    [esi + TxDesc.FlagLen], NV_TX_VALID
        jnz     .no_tx
        cmp     dword[esi + TX_RING*sizeof.TxDesc], 0
        je      .no_tx

        DEBUGF  1,"Freeing buffer 0x%x\n", [esi + TX_RING*sizeof.TxDesc]:8
        push    dword[esi + TX_RING*sizeof.TxDesc]
        mov     dword[esi + TX_RING*sizeof.TxDesc], 0
        invoke  NetFree

        inc     [ebx + device.last_tx]
        and     [ebx + device.last_tx], TX_RING - 1

        jmp     .loop_tx

  .no_tx:
  .fail:
        pop     edi esi ebx
        xor     eax, eax
        inc     eax

        ret


; End of code

data fixups
end data

include '../peimport.inc'

my_service      db 'FORCEDETH',0                ; max 16 chars include zero

include_debug_strings

align 4
devices         dd 0
device_list     rd MAX_DEVICES                  ; This list contains all pointers to device structures the driver is handling

