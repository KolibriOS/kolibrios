;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  rhine.asm                                                      ;;
;;                                                                 ;;
;;  Ethernet driver for Kolibri OS                                 ;;
;;                                                                 ;;
;;  This driver is based on the via-rhine driver from              ;;
;;  the etherboot 5.0.6 project. The copyright statement is        ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;    Rewritten in flat assembler by Asper (asper.85@mail.ru)      ;;
;;            and hidnplayr (hidnplayr@gmail.com)                  ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;                                                                 ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

        API_VERSION             = 0x01000100
        DRIVER_VERSION          = 5

        MAX_DEVICES             = 16

        DEBUG                   = 1
        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 1

        TX_RING_SIZE            = 4
        RX_RING_SIZE            = 4

        ; max time out delay time
        W_MAX_TIMEOUT           = 0x0FFF

        ; Size of the in-memory receive ring.
        RX_BUF_LEN_IDX          = 3     ; 0==8K, 1==16K, 2==32K, 3==64K
        RX_BUF_LEN              = (8192 shl RX_BUF_LEN_IDX)

        ; PCI Tuning Parameters
        ;   Threshold is bytes transferred to chip before transmission starts.
        TX_FIFO_THRESH          = 256   ; In bytes, rounded down to 32 byte units.

        ; The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024.
        RX_FIFO_THRESH          = 4     ; Rx buffer level before first PCI xfer.
        RX_DMA_BURST            = 4     ; Maximum PCI burst, '4' is 256 bytes
        TX_DMA_BURST            = 4


include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'
include 'netdrv.inc'

public START
public service_proc
public version


;**************************************************************************
; VIA Rhine Register Definitions
;**************************************************************************
byPAR0                          = 0x00
byRCR                           = 0x06
byTCR                           = 0x07
byCR0                           = 0x08
byCR1                           = 0x09
byISR0                          = 0x0c
byISR1                          = 0x0d
byIMR0                          = 0x0e
byIMR1                          = 0x0f
byMAR0                          = 0x10
byMAR1                          = 0x11
byMAR2                          = 0x12
byMAR3                          = 0x13
byMAR4                          = 0x14
byMAR5                          = 0x15
byMAR6                          = 0x16
byMAR7                          = 0x17
dwCurrentRxDescAddr             = 0x18
dwCurrentTxDescAddr             = 0x1c
dwCurrentRDSE0                  = 0x20
dwCurrentRDSE1                  = 0x24
dwCurrentRDSE2                  = 0x28
dwCurrentRDSE3                  = 0x2c
dwNextRDSE0                     = 0x30
dwNextRDSE1                     = 0x34
dwNextRDSE2                     = 0x38
dwNextRDSE3                     = 0x3c
dwCurrentTDSE0                  = 0x40
dwCurrentTDSE1                  = 0x44
dwCurrentTDSE2                  = 0x48
dwCurrentTDSE3                  = 0x4c
dwNextTDSE0                     = 0x50
dwNextTDSE1                     = 0x54
dwNextTDSE2                     = 0x58
dwNextTDSE3                     = 0x5c
dwCurrRxDMAPtr                  = 0x60
dwCurrTxDMAPtr                  = 0x64
byMPHY                          = 0x6c
byMIISR                         = 0x6d
byBCR0                          = 0x6e
byBCR1                          = 0x6f
byMIICR                         = 0x70
byMIIAD                         = 0x71
wMIIDATA                        = 0x72
byEECSR                         = 0x74
byTEST                          = 0x75
byGPIO                          = 0x76
byCFGA                          = 0x78
byCFGB                          = 0x79
byCFGC                          = 0x7a
byCFGD                          = 0x7b
wTallyCntMPA                    = 0x7c
wTallyCntCRC                    = 0x7d
bySTICKHW                       = 0x83
byWOLcrClr                      = 0xA4
byWOLcgClr                      = 0xA7
byPwrcsrClr                     = 0xAC

;---------------------  Exioaddr Definitions -------------------------

; Bits in the RCR register
RCR_RRFT2               = 0x80
RCR_RRFT1               = 0x40
RCR_RRFT0               = 0x20
RCR_PROM                = 0x10
RCR_AB                  = 0x08
RCR_AM                  = 0x04
RCR_AR                  = 0x02
RCR_SEP                 = 0x01
; Bits in the TCR register
TCR_RTSF                = 0x80
TCR_RTFT1               = 0x40
TCR_RTFT0               = 0x20
TCR_OFSET               = 0x08
TCR_LB1                 = 0x04    ; loopback[1]
TCR_LB0                 = 0x02    ; loopback[0]
; Bits in the CR0 register
CR0_RDMD                = 0x40    ; rx descriptor polling demand
CR0_TDMD                = 0x20    ; tx descriptor polling demand
CR0_TXON                = 0x10
CR0_RXON                = 0x08
CR0_STOP                = 0x04    ; stop NIC, default = 1
CR0_STRT                = 0x02    ; start NIC
CR0_INIT                = 0x01    ; start init process
; Bits in the CR1 register
CR1_SFRST               = 0x80    ; software reset
CR1_RDMD1               = 0x40    ; RDMD1
CR1_TDMD1               = 0x20    ; TDMD1
CR1_KEYPAG              = 0x10    ; turn on par/key
CR1_DPOLL               = 0x08    ; disable rx/tx auto polling
CR1_FDX                 = 0x04    ; full duplex mode
CR1_ETEN                = 0x02    ; early tx mode
CR1_EREN                = 0x01    ; early rx mode
; Bits in the CR register
CR_RDMD                 = 0x0040  ; rx descriptor polling demand
CR_TDMD                 = 0x0020  ; tx descriptor polling demand
CR_TXON                 = 0x0010
CR_RXON                 = 0x0008
CR_STOP                 = 0x0004  ; stop NIC, default = 1
CR_STRT                 = 0x0002  ; start NIC
CR_INIT                 = 0x0001  ; start init process
CR_SFRST                = 0x8000  ; software reset
CR_RDMD1                = 0x4000  ; RDMD1
CR_TDMD1                = 0x2000  ; TDMD1
CR_KEYPAG               = 0x1000  ; turn on par/key
CR_DPOLL                = 0x0800  ; disable rx/tx auto polling
CR_FDX                  = 0x0400  ; full duplex mode
CR_ETEN                 = 0x0200  ; early tx mode
CR_EREN                 = 0x0100  ; early rx mode
; Bits in the IMR0 register
IMR0_CNTM               = 0x80
IMR0_BEM                = 0x40
IMR0_RUM                = 0x20
IMR0_TUM                = 0x10
IMR0_TXEM               = 0x08
IMR0_RXEM               = 0x04
IMR0_PTXM               = 0x02
IMR0_PRXM               = 0x01
; define imrshadow
IMRShadow               = 0x5AFF
; Bits in the IMR1 register
IMR1_INITM              = 0x80
IMR1_SRCM               = 0x40
IMR1_NBFM               = 0x10
IMR1_PRAIM              = 0x08
IMR1_RES0M              = 0x04
IMR1_ETM                = 0x02
IMR1_ERM                = 0x01
; Bits in the ISR register
ISR_INITI               = 0x8000
ISR_SRCI                = 0x4000
ISR_ABTI                = 0x2000
ISR_NORBF               = 0x1000
ISR_PKTRA               = 0x0800
ISR_RES0                = 0x0400
ISR_ETI                 = 0x0200
ISR_ERI                 = 0x0100
ISR_CNT                 = 0x0080
ISR_BE                  = 0x0040
ISR_RU                  = 0x0020
ISR_TU                  = 0x0010
ISR_TXE                 = 0x0008
ISR_RXE                 = 0x0004
ISR_PTX                 = 0x0002
ISR_PRX                 = 0x0001
; Bits in the ISR0 register
ISR0_CNT                = 0x80
ISR0_BE                 = 0x40
ISR0_RU                 = 0x20
ISR0_TU                 = 0x10
ISR0_TXE                = 0x08
ISR0_RXE                = 0x04
ISR0_PTX                = 0x02
ISR0_PRX                = 0x01
; Bits in the ISR1 register
ISR1_INITI              = 0x80
ISR1_SRCI               = 0x40
ISR1_NORBF              = 0x10
ISR1_PKTRA              = 0x08
ISR1_ETI                = 0x02
ISR1_ERI                = 0x01
; ISR ABNORMAL CONDITION
ISR_ABNORMAL            = ISR_BE+ISR_RU+ISR_TU+ISR_CNT+ISR_NORBF+ISR_PKTRA
; Bits in the MIISR register
MIISR_MIIERR            = 0x08
MIISR_MRERR             = 0x04
MIISR_LNKFL             = 0x02
MIISR_SPEED             = 0x01
; Bits in the MIICR register
MIICR_MAUTO             = 0x80
MIICR_RCMD              = 0x40
MIICR_WCMD              = 0x20
MIICR_MDPM              = 0x10
MIICR_MOUT              = 0x08
MIICR_MDO               = 0x04
MIICR_MDI               = 0x02
MIICR_MDC               = 0x01
; Bits in the EECSR register
EECSR_EEPR              = 0x80    ; eeprom programed status, 73h means programed
EECSR_EMBP              = 0x40    ; eeprom embeded programming
EECSR_AUTOLD            = 0x20    ; eeprom content reload
EECSR_DPM               = 0x10    ; eeprom direct programming
EECSR_CS                = 0x08    ; eeprom CS pin
EECSR_SK                = 0x04    ; eeprom SK pin
EECSR_DI                = 0x02    ; eeprom DI pin
EECSR_DO                = 0x01    ; eeprom DO pin
; Bits in the BCR0 register
BCR0_CRFT2              = 0x20
BCR0_CRFT1              = 0x10
BCR0_CRFT0              = 0x08
BCR0_DMAL2              = 0x04
BCR0_DMAL1              = 0x02
BCR0_DMAL0              = 0x01
; Bits in the BCR1 register
BCR1_CTSF               = 0x20
BCR1_CTFT1              = 0x10
BCR1_CTFT0              = 0x08
BCR1_POT2               = 0x04
BCR1_POT1               = 0x02
BCR1_POT0               = 0x01
; Bits in the CFGA register
CFGA_EELOAD             = 0x80    ; enable eeprom embeded and direct programming
CFGA_JUMPER             = 0x40
CFGA_MTGPIO             = 0x08
CFGA_T10EN              = 0x02
CFGA_AUTO               = 0x01
; Bits in the CFGB register
CFGB_PD                 = 0x80
CFGB_POLEN              = 0x02
CFGB_LNKEN              = 0x01
; Bits in the CFGC register
CFGC_M10TIO             = 0x80
CFGC_M10POL             = 0x40
CFGC_PHY1               = 0x20
CFGC_PHY0               = 0x10
CFGC_BTSEL              = 0x08
CFGC_BPS2               = 0x04    ; bootrom select[2]
CFGC_BPS1               = 0x02    ; bootrom select[1]
CFGC_BPS0               = 0x01    ; bootrom select[0]
; Bits in the CFGD register
CFGD_GPIOEN             = 0x80
CFGD_DIAG               = 0x40
CFGD_MAGIC              = 0x10
CFGD_RANDOM             = 0x08
CFGD_CFDX               = 0x04
CFGD_CEREN              = 0x02
CFGD_CETEN              = 0x01
; Bits in RSR
RSR_RERR                = 0x00000001
RSR_CRC                 = 0x00000002
RSR_FAE                 = 0x00000004
RSR_FOV                 = 0x00000008
RSR_LONG                = 0x00000010
RSR_RUNT                = 0x00000020
RSR_SERR                = 0x00000040
RSR_BUFF                = 0x00000080
RSR_EDP                 = 0x00000100
RSR_STP                 = 0x00000200
RSR_CHN                 = 0x00000400
RSR_PHY                 = 0x00000800
RSR_BAR                 = 0x00001000
RSR_MAR                 = 0x00002000
RSR_RXOK                = 0x00008000
RSR_ABNORMAL            = RSR_RERR+RSR_LONG+RSR_RUNT
; Bits in TSR
TSR_NCR0                = 0x00000001
TSR_NCR1                = 0x00000002
TSR_NCR2                = 0x00000004
TSR_NCR3                = 0x00000008
TSR_COLS                = 0x00000010
TSR_CDH                 = 0x00000080
TSR_ABT                 = 0x00000100
TSR_OWC                 = 0x00000200
TSR_CRS                 = 0x00000400
TSR_UDF                 = 0x00000800
TSR_TBUFF               = 0x00001000
TSR_SERR                = 0x00002000
TSR_JAB                 = 0x00004000
TSR_TERR                = 0x00008000
TSR_ABNORMAL            = TSR_TERR+TSR_OWC+TSR_ABT+TSR_JAB+TSR_CRS
TSR_OWN_BIT             = 0x80000000

CB_DELAY_LOOP_WAIT      = 10      ; 10ms
; enabled mask value of irq
W_IMR_MASK_VALUE        = 0x1BFF  ; initial value of IMR

; Ethernet address filter type
PKT_TYPE_DIRECTED       = 0x0001  ; obsolete, directed address is always accepted
PKT_TYPE_MULTICAST      = 0x0002
PKT_TYPE_ALL_MULTICAST  = 0x0004
PKT_TYPE_BROADCAST      = 0x0008
PKT_TYPE_PROMISCUOUS    = 0x0020
PKT_TYPE_LONG           = 0x2000
PKT_TYPE_RUNT           = 0x4000
PKT_TYPE_ERROR          = 0x8000  ; accept error packets, e.g. CRC error

; Loopback mode

NIC_LB_NONE             = 0x00
NIC_LB_INTERNAL         = 0x01
NIC_LB_PHY              = 0x02    ; MII or Internal-10BaseT loopback

PKT_BUF_SZ              = 1536    ; Size of each temporary Rx buffer.

PCI_REG_MODE3           = 0x53
MODE3_MIION             = 0x04    ; in PCI_REG_MOD3 OF PCI space

; VIA Rhine revisions
VT86C100A       = 0x00
VTunknown0      = 0x20
VT6102          = 0x40
VT8231          = 0x50 ; Integrated MAC
VT8233          = 0x60 ; Integrated MAC
VT8235          = 0x74 ; Integrated MAC
VT8237          = 0x78 ; Integrated MAC
VTunknown1      = 0x7C
VT6105          = 0x80
VT6105_B0       = 0x83
VT6105L         = 0x8A
VT6107          = 0x8C
VTunknown2      = 0x8E
VT6105M         = 0x90

; Rx status bits
RX_SBITS_RERR                   = 1 shl 0
RX_SBITS_CRC_ERROR              = 1 shl 1
RX_SBITS_FAE                    = 1 shl 2
RX_SBITS_FOV                    = 1 shl 3
RX_SBITS_TOOLONG                = 1 shl 4
RX_SBITS_RUNT                   = 1 shl 5
RX_SBITS_SERR                   = 1 shl 6
RX_SBITS_BUFF                   = 1 shl 7
RX_SBITS_EDP                    = 1 shl 8
RX_SBITS_STP                    = 1 shl 9
RX_SBITS_CHN                    = 1 shl 10
RX_SBITS_PHY                    = 1 shl 11
RX_SBITS_BAR                    = 1 shl 12
RX_SBITS_MAR                    = 1 shl 13
RX_SBITS_RESERVED_1             = 1 shl 14
RX_SBITS_RXOK                   = 1 shl 15
RX_SBITS_FRAME_LENGTH           = 0x7FF shl 16
RX_SBITS_RESERVED_2             = 0xF shl 27
RX_SBITS_OWN_BIT                = 1 shl 31

; Rx control bits
RX_CBITS_RX_BUF_SIZE            = 0x7FF
RX_CBITS_EXTEND_RX_BUF_SIZE     = 0xF shl 11
RX_CBITS_RESERVED_1             = 0x1FFFF shl 15

; Tx status bits
TX_SBITS_NCR0                   = 1 shl 0
TX_SBITS_NCR1                   = 1 shl 1
TX_SBITS_NCR2                   = 1 shl 2
TX_SBITS_NCR3                   = 1 shl 3
TX_SBITS_COLS                   = 1 shl 4
TX_SBITS_RESERVED_1             = 1 shl 5
TX_SBITS_CDH                    = 1 shl 7
TX_SBITS_ABT                    = 1 shl 8
TX_SBITS_OWC                    = 1 shl 9
TX_SBITS_CRS                    = 1 shl 10
TX_SBITS_UDF                    = 1 shl 11
TX_SBITS_TBUFF                  = 1 shl 12
TX_SBITS_SERR                   = 1 shl 13
TX_SBITS_JAB                    = 1 shl 14
TX_SBITS_TERR                   = 1 shl 15
TX_SBITS_RESERVED_2             = 0x7FFF shl 16
TX_SBITS_OWN_BIT                = 1 shl 31

; Tx control bits
TX_CBITS_TX_BUF_SIZE            = 0x7FF
TX_CBITS_EXTEND_TX_BUF_SIZE     = 0xF shl 11
TX_CBITS_CHN                    = 1 shl 15
TX_CBITS_CRC                    = 1 shl 16
TX_CBITS_RESERVED_1             = 0xF shl 17
TX_CBITS_STP                    = 1 shl 21
TX_CBITS_EDP                    = 1 shl 22
TX_CBITS_IC                     = 1 shl 23
TX_CBITS_RESERVED_2             = 0xFF shl 24



; Offsets to the device registers.
        StationAddr             = 0x00
        RxConfig                = 0x06
        TxConfig                = 0x07
        ChipCmd                 = 0x08
        IntrStatus              = 0x0C
        IntrEnable              = 0x0E
        MulticastFilter0        = 0x10
        MulticastFilter1        = 0x14
        RxRingPtr               = 0x18
        TxRingPtr               = 0x1C
        GFIFOTest               = 0x54
        MIIPhyAddr              = 0x6C
        MIIStatus               = 0x6D
        PCIBusConfig            = 0x6E
        MIICmd                  = 0x70
        MIIRegAddr              = 0x71
        MIIData                 = 0x72
        MACRegEEcsr             = 0x74
        ConfigA                 = 0x78
        ConfigB                 = 0x79
        ConfigC                 = 0x7A
        ConfigD                 = 0x7B
        RxMissed                = 0x7C
        RxCRCErrs               = 0x7E
        MiscCmd                 = 0x81
        StickyHW                = 0x83
        IntrStatus2             = 0x84
        WOLcrClr                = 0xA4
        WOLcgClr                = 0xA7
        PwrcsrClr               = 0xAC

; Bits in the interrupt status/mask registers.
        IntrRxDone              = 0x0001
        IntrRxErr               = 0x0004
        IntrRxEmpty             = 0x0020
        IntrTxDone              = 0x0002
        IntrTxError             = 0x0008
        IntrTxUnderrun          = 0x0010
        IntrPCIErr              = 0x0040
        IntrStatsMax            = 0x0080
        IntrRxEarly             = 0x0100
        IntrRxOverflow          = 0x0400
        IntrRxDropped           = 0x0800
        IntrRxNoBuf             = 0x1000
        IntrTxAborted           = 0x2000
        IntrLinkChange          = 0x4000
        IntrRxWakeUp            = 0x8000
        IntrNormalSummary       = 0x0003
        IntrAbnormalSummary     = 0xC260
        IntrTxDescRace          = 0x080000        ; mapped from IntrStatus2
        IntrTxErrSummary        = 0x082218

DEFAULT_INTR  = (IntrRxDone or IntrRxErr or IntrRxEmpty or IntrRxOverflow or IntrRxDropped or IntrRxNoBuf)


virtual at ebx

        device:

        ETH_DEVICE

        .io_addr                dd ?
        .pci_dev                dd ?
        .pci_bus                dd ?
        .revision               db ?
        .irq_line               db ?
        .chip_id                dw ?

        .cur_rx                 dw ?
        .cur_tx                 dw ?
        .last_tx                dw ?

                                rb 0x100-(($ - device) and 0xff)        ;        align 256
        .tx_ring                rb tx_head.sizeof*TX_RING_SIZE

                                rb 0x100-(($ - device) and 0xff)        ;        align 256
        .rx_ring                rb rx_head.sizeof*RX_RING_SIZE

        .size                   = $ - device

end virtual

virtual at 0
        rx_head:
        .status         dd ?
        .control        dd ?
        .buff_addr      dd ?    ; address
        .next_desc      dd ?    ;

        .buff_addr_virt dd ?
                        rd 3    ; alignment
        .sizeof:
end virtual


virtual at 0
        tx_head:
        .status         dd ?
        .control        dd ?
        .buff_addr      dd ?    ; address
        .next_desc      dd ?    ;

        .buff_addr_virt dd ?
                        rd 3    ; alignment
        .sizeof:
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

  .entry:

        DEBUGF  2,"Loading %s driver\n", my_service
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
        mov     ax , [eax+1]                            ;
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte[device.pci_bus]
        jne     @f
        cmp     ah, byte[device.pci_dev]
        je      .find_devicenum                         ; Device is already loaded, let's find it's device number
       @@:
        add     esi, 4
        loop    .nextdevice


; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
        cmp     [devices], MAX_DEVICES                  ; First check if the driver can handle one more card
        jae     .fail

        allocate_and_clear ebx, device.size, .fail      ; Allocate the buffer for device structure

; Fill in the direct call addresses into the struct

        mov     [device.reset], reset
        mov     [device.transmit], transmit
        mov     [device.get_MAC], read_mac
        mov     [device.set_MAC], .fail
        mov     [device.unload], unload
        mov     [device.name], my_service

; save the pci bus and device numbers

        mov     eax, [IOCTL.input]
        movzx   ecx, byte[eax+1]
        mov     [device.pci_bus], ecx
        movzx   ecx, byte[eax+2]
        mov     [device.pci_dev], ecx

; Now, it's time to find the base io addres of the PCI device

        PCI_find_io

; We've found the io address, find IRQ now

        PCI_find_irq

        DEBUGF  1,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
        [device.pci_dev]:1,[device.pci_bus]:1,[device.irq_line]:1,[device.io_addr]:4

; Ok, the eth_device structure is ready, let's probe the device
    ;;;    cli

        call    probe                                                   ; this function will output in eax
        test    eax, eax
        jnz     .err_sti                                                ; If an error occured, exit

        mov     eax, [devices]                                          ; Add the device structure to our device list
        mov     [device_list+4*eax], ebx                                ; (IRQ handler uses this list to find device)
        inc     [devices]                                               ;

        mov     [device.type], NET_TYPE_ETH
        call    NetRegDev
   ;;;     sti

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

  .err_sti:
        sti

  .err:
        stdcall KernelFree, ebx

  .fail:
        or      eax, -1
        ret

;------------------------------------------------------
endp


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;



probe:

        mov     eax, [device.io_addr]
        DEBUGF  1, "Probing card at 0x%x\n", eax

; make the card a bus master
        PCI_make_bus_master

; get device id
        stdcall PciRead16, [device.pci_bus], [device.pci_dev], PCI_DEVICE_ID
        mov     [device.chip_id], ax

; get revision id.
        PCI_find_rev

        movzx   eax, [device.revision]
        DEBUGF  1, "Card revision = 0x%x\n", eax

; D-Link provided reset code (with comment additions)
        cmp     al, 0x40
        jb      .below_x40

        mov     ax, [device.chip_id]
        DEBUGF  1, "Enabling Sticky Bit Workaround for Chip_id: 0x%x\n", ax

        ; clear sticky bit before reset & read ethernet address
        set_io  0
        set_io  bySTICKHW
        in      al, dx
        and     al, 0xFC
        out     dx, al

        ; (bits written are cleared?)
        ; disable force PME-enable
        set_io  byWOLcgClr
        mov     al, 0x80
        out     dx, al

        ; disable power-event config bit
        mov     al, 0xFF
        out     dx, al

        ; clear power status (undocumented in vt6102 docs?)
        set_io  byPwrcsrClr
        out     dx, al

  .below_x40:

; Reset the chip to erase previous misconfiguration.
        set_io  0
        set_io  byCR0
        mov     ax, CR_SFRST
        out     dx, ax

; if vt3043 delay after reset
        cmp     [device.revision], 0x40
        jae     @f
        mov     esi, 2000 ; 2000ms
        call    Sleep
   @@:

; polling till software reset complete
        mov     ecx, W_MAX_TIMEOUT
   .poll_again:
        in      ax, dx
        test    ax, CR_SFRST
        jz      @f
        loop    .poll_again
        DEBUGF  1, "Soft reset timeout!\n"
   @@:

; issue AUTOLoad in EECSR to reload eeprom
        set_io  byEECSR
        mov     al, 0x20
        out     dx, al

; if vt3065 delay after reset
        cmp     [device.revision], 0x40
        jb      .not_vt3065

        ; delay 8ms to let MAC stable
        mov     esi, 8 ; 8ms
        call    Sleep

        ; for 3065D, EEPROM reloaded will cause bit 0 in MAC_REG_CFGA
        ; turned on.  it makes MAC receive magic packet
        ; automatically. So, we turn it off. (D-Link)

        set_io  byCFGA
        in      al, dx
        and     al, 0xFE
        out     dx, al

        ; turn on bit2 in PCI configuration register 0x53 , only for 3065
        stdcall PciRead8, [device.pci_bus], [device.pci_dev], PCI_REG_MODE3
        or      al,  MODE3_MIION
        stdcall PciWrite8, [device.pci_bus], [device.pci_dev], PCI_REG_MODE3, eax
  .not_vt3065:

; back off algorithm, disable the right-most 4-bit off CFGD
        set_io  0
        set_io  byCFGD
        in      al, dx
        and     al, not (CFGD_RANDOM or CFGD_CFDX or CFGD_CEREN or CFGD_CETEN)
        out     dx, al

; reload eeprom
        call    reload_eeprom

; read MAC
        call    read_mac

; restart MII auto-negotiation
        stdcall WriteMII, 0, 1 shl 9, 1

        DEBUGF  1, "Analyzing Media type, this may take several seconds"

        mov     ecx, 5
     .read_again:
        DEBUGF  1, "."
        mov     esi, 1
        call    Sleep

        stdcall ReadMII, 1
        test    eax, 0x0020
        jnz     .read_done
        loop    .read_again
        DEBUGF  1, "timeout!\n"
     .read_done:
        DEBUGF  1, " OK\n"

if DEBUG

        set_io  0
        set_io  0x6C
        in      al, dx
        and     eax, 0xFF
        DEBUGF  1, "MII : Address %x\n", ax

        stdcall ReadMII, 1
        DEBUGF  1, "status 0x%x\n", ax

        stdcall ReadMII, 4
        DEBUGF  1, "advertising 0x%x\n", ax

        stdcall ReadMII, 5
        DEBUGF  1, "link 0x%x\n", ax

end if

; query MII to know LineSpeed, duplex mode
        set_io  0
        set_io  MIIStatus
        in      al, dx
        test    al, MIISR_SPEED
        jz      .100mbps
        DEBUGF  1, "Linespeed=10Mbs\n"
        jmp     @f

    .100mbps:
        DEBUGF  1, "Linespeed=100Mbs\n"
    @@:

        call    QueryAuto

        test    eax, 1
        jz      .halfduplex

        DEBUGF  1, "Fullduplex\n"
        set_io  0
        set_io  byCR0
        mov     ax, CR_FDX
        out     dx, ax
        jmp     @f

    .halfduplex:
        DEBUGF  1, "Halfduplex\n"
    @@:

; set MII 10 FULL ON, only apply in vt3043
        cmp     [device.chip_id], 0x3043
        jne     @f
        stdcall WriteMII, 0x17, 1 shl 1, 1
    @@:

; turn on MII link change
        set_io  0
        set_io  byMIICR
        in      al, dx
        and     al, 0x7F
        out     dx, al
        push    eax

        call    MIIDelay

        set_io  byMIIAD
        mov     al, 0x41
        out     dx, al

        call    MIIDelay

        pop     eax
        or      al, 0x80
        set_io  byMIICR
        out     dx, al

;**************************************************************************;
;* ETH_RESET - Reset adapter                                              *;
;**************************************************************************;

reset:

        DEBUGF  1, "reset\n"

; attach int handler

        movzx   eax, [device.irq_line]
        DEBUGF  2,"Attaching int handler to irq %x\n", eax:1
        stdcall AttachIntHandler, eax, int_handler, dword 0
        test    eax, eax
        jnz     @f
        DEBUGF  2,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
       @@:

; Soft reset the chip.
        set_io  0
        set_io  byCR0
        mov     ax, CR_SFRST
        out     dx, ax

        call    MIIDelay

; Initialize rings
        call    init_ring

; Setup Multicast
        call    set_rx_mode

; set TCR RCR threshold to store and forward
        set_io  0
        set_io  byBCR0
        mov     al, 0x3E
        out     dx, al

        set_io  byBCR1
        mov     al, 0x38
        out     dx, al

        set_io  byRCR
        mov     al, 0x2C
        out     dx, al

        set_io  byTCR
        mov     al, 0x60
        out     dx, al

; Set Fulldupex

        call    QueryAuto
        test    eax, eax        ; full duplex?
        jz      @f

        set_io  0
        set_io  byCFGD
        mov     al, CFGD_CFDX
        out     dx, al

        set_io  byCR0
        mov     ax, CR_FDX
        out     dx, ax
    @@:

; ENABLE interrupts
        set_io  0
        set_io  byIMR0
        mov     ax, DEFAULT_INTR
        out     dx, ax

; KICK NIC to WORK

        set_io  byCR0
        in      ax, dx
        and     ax, not CR_STOP
        or      ax, CR_STRT or CR_TXON or CR_RXON or CR_DPOLL
        out     dx, ax

; Set the mtu, kernel will be able to send now
        mov     [device.mtu], 1514

; say reset was successfull
        xor     eax, eax

        ret



align 4
unload:

        call    reset
        push    eax edx
        DEBUGF  1, "rhine disable\n"

        ; Switch to loopback mode to avoid hardware races.
        set_io  0
        set_io  byTCR
        mov     al, 0x61
        out     dx, al

        ; Stop the chip's Tx and Rx processes.
        set_io  byCR0
        mov     ax, CR_STOP
        out     dx, ax
        pop     edx eax

        ret




align 4
reload_eeprom:

        DEBUGF  1, "Reload eeprom\n"

        set_io  0
        set_io  byEECSR
        mov     al, 0x20
        out     dx, al
        ; Typically 2 cycles to reload.
        mov     ecx, 150
  .reload:
        in      al, dx
        test    al, 0x20
        jz      @f
        loop    .reload
        DEBUGF  1, "Reload timeout!\n"
  @@:

        ret

; Initialize the Rx and Tx rings, along with various 'dev' bits.
align 4
init_ring:

        DEBUGF  1, "Init ring\n"

        lea     edi, [device.rx_ring]
        mov     eax, edi
        GetRealAddr
        mov     esi, eax
        push    esi
        mov     ecx, RX_RING_SIZE
   .rx_init:
        add     esi, rx_head.sizeof
        mov     [edi + rx_head.status], RX_SBITS_OWN_BIT
        mov     [edi + rx_head.control], PKT_BUF_SZ
        push    ecx
        stdcall KernelAlloc, PKT_BUF_SZ
        pop     ecx
        mov     [edi + rx_head.buff_addr_virt], eax
        GetRealAddr
        mov     [edi + rx_head.buff_addr], eax                        ; buffer ptr
        mov     [edi + rx_head.next_desc], esi                        ; next head
        add     edi, rx_head.sizeof
        dec     ecx
        jnz     .rx_init
        pop     [edi - rx_head.sizeof + rx_head.next_desc]            ; Mark the last entry as wrapping the ring.


        lea     edi, [device.tx_ring]
        mov     eax, edi
        GetRealAddr
        mov     esi, eax
        push    esi
        mov     ecx, TX_RING_SIZE
   .tx_init:
        add     esi, tx_head.sizeof
        mov     [edi + tx_head.status], 0
        mov     [edi + tx_head.control], 0x00E08000
        mov     [edi + tx_head.buff_addr], 0
        mov     [edi + tx_head.next_desc], esi
        mov     [edi + tx_head.buff_addr_virt], 0
        add     edi, tx_head.sizeof
        dec     ecx
        jnz     .tx_init
        pop     [edi - tx_head.sizeof + tx_head.next_desc]              ; Mark the last entry as wrapping the ring.

; write Descriptors to MAC
        lea     eax, [device.rx_ring]
        GetRealAddr
        set_io  0
        set_io  dwCurrentRxDescAddr
        out     dx, eax

        lea     eax, [device.tx_ring]
        GetRealAddr
        set_io  dwCurrentTxDescAddr
        out     dx, eax

        xor     eax, eax
        mov     [device.cur_rx], ax
        mov     [device.cur_tx], ax
        mov     [device.last_tx], ax

        ret


align 4
QueryAuto:

        DEBUGF  1, "Query Auto\n"

        push    ecx
        stdcall ReadMII, 0x04   ; advertised
        mov     ecx, eax
        stdcall ReadMII, 0x05
        and     ecx, eax

        xor     eax, eax
        test    ecx, 0x100
        jnz     .one

        and     ecx, 0x1C0
        cmp     ecx, 0x40
        jne     .zero
   .one:
        inc     eax
        DEBUGF  1, "AutoNego OK!\n"
   .zero:
        pop     ecx

        ret


proc    ReadMII stdcall, byMIIIndex:dword

;        DEBUGF  1, "ReadMII Index=%x\n", [byMIIIndex]

        push    esi ebx ecx edx

        set_io  0
        set_io  byMIIAD
        in      al, dx
        mov     bl, al

        set_io  byMIICR
        in      al, dx
        mov     bh, al
        and     al, 0x7F
        out     dx, al

        call    MIIDelay

        mov     al, byte [byMIIIndex]
        set_io  byMIIAD
        out     dx, al

        call    MIIDelay

        set_io  byMIICR
        in      al, dx
        or      al, 0x40
        out     dx, al

        mov     ecx, 200
  .read_again:
        in      al, dx
        test    al, 0x40
        jz      @f

        mov     esi, 10
        call    Sleep
        dec     ecx
        jnz     .read_again
        DEBUGF  1, "\nReadMII timeout!\n"
  @@:

        call    MIIDelay

        set_io  byMIIAD
        in      ax, dx

        push    eax
        mov     ax, bx
        set_io  byMIIAD
        out     dx, al

        shr     ax, 8
        set_io  byMIICR
        out     dx, al

        call    MIIDelay

        pop     eax
        and     eax, 0xFFFF
        rol     ax, 8           ;;;;; I dont know how or why but it seems needed...

        pop     edx ecx ebx esi
        ret
endp

proc    WriteMII stdcall, byMIISetByte:dword, byMIISetBit:dword, byMIIOP:dword

;        DEBUGF  1, "WriteMII SetByte=%x SetBit=%x OP=%x\n", [byMIISetByte], [byMIISetBit], [byMIIOP]

        push    ebx eax ecx edx

        set_io  0
        set_io  byMIIAD
        in      al, dx
        mov     bl, al

        set_io  byMIICR
        in      al, dx
        mov     bh, al
        and     al, 0x7F
        out     dx, al

        call    MIIDelay

        mov     al, byte [byMIISetByte]
        set_io  byMIIAD
        out     dx, al

        call    MIIDelay

        set_io  byMIICR
        in      al, dx
        or      al, 0x40
        out     dx, al

        mov     ecx, 200
  .read_again0:
        in      al, dx
        test    al, 0x40
        jz      .done

        mov     esi, 10
        call    Sleep
        dec     ecx
        jnz     .read_again0
        DEBUGF  1, "WriteMII timeout 1\n"
  .done:

        call    MIIDelay

        set_io  wMIIDATA
        in      ax, dx

        mov     ecx, [byMIISetBit]
        rol     cx, 8                   ;;;;;;;;;;;;;;;;; CHECKME

        cmp     byte [byMIIOP], 0
        jne     @f
        not     ecx
        and     ax, cx
        jmp     .end_mascarad
     @@:
        or      ax, cx
     .end_mascarad:

        set_io  wMIIDATA
        out     dx, ax

        call    MIIDelay

        set_io  byMIICR
        in      al, dx
        or      al, 0x20
        out     dx, al

        mov     ecx, 200
    .read_again1:
        in      al, dx
        test    al, 0x20
        jz      @f

        mov     esi, 10
        call    Sleep
        dec     ecx
        jnz     .read_again1
        DEBUGF  1, "WriteMII timeout 2\n"
    @@:

        call    MIIDelay

        mov     ax, bx
        and     al, 0x7F
        set_io  byMIIAD
        out     dx, al

        shr     ax, 8
        set_io  byMIICR
        out     dx, al

        call    MIIDelay

        pop     edx ecx eax ebx
        ret
endp


align 4
MIIDelay:

        mov     ecx, 0x7FFF
    @@:
        in      al, 0x61
        in      al, 0x61
        in      al, 0x61
        in      al, 0x61
        loop    @b

        ret


align 4
set_rx_mode:

        DEBUGF  1, "Set RX mode\n"

        ; ! IFF_PROMISC
        mov     eax, 0xffffffff
        set_io  0
        set_io  byMAR0
        out     dx, eax

        set_io  byMAR4
        out     dx, eax

        set_io  byRCR
        mov     al, 0x6C                ;rx_mode = 0x0C;
        out     dx, al          ;outb(0x60 /* thresh */ | rx_mode, byRCR );

        ret





; Beware of PCI posted writes
macro IOSYNC
{
        set_io  StationAddr
        in      al, dx
}



align 4
read_mac:

        DEBUGF  1, "Ethernet Address: "

        lea     edi, [device.mac]
        set_io  0
        set_io  byPAR0
        mov     ecx, 6
  .next:
        in      al, dx
        stosb
        DEBUGF  1, "-%x", al
        inc     edx
        dec     ecx
        jnz     .next
        DEBUGF  1, "\n"

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

        cmp     dword [esp+8], 1514
        ja      .fail
        cmp     dword [esp+8], 60
        jb      .fail

        movzx   eax, [device.cur_tx]
        mov     ecx, tx_head.sizeof
        mul     ecx
        lea     edi, [device.tx_ring]
        add     edi, eax

        cmp     [edi + tx_head.buff_addr_virt], 0
        jne     .fail

        mov     eax, [esp+4]
        mov     [edi + tx_head.buff_addr_virt], eax
        GetRealAddr
        mov     [edi + tx_head.buff_addr], eax
        mov     ecx, [esp+8]
        and     ecx, TX_CBITS_TX_BUF_SIZE
        or      ecx,  0x00E08000
        mov     [edi + tx_head.control], ecx
        or      [edi + tx_head.status], TX_SBITS_OWN_BIT

        set_io  0
        set_io  byCR1
        in      al, dx
        or      al, CR1_TDMD1
        out     dx, al

        inc     [device.cur_tx]
        and     [device.cur_tx], TX_RING_SIZE-1

        ;outw(IMRShadow,byIMR0); ;

; Update stats
        inc     [device.packets_tx]
        mov     ecx, [esp+8]    ;;;;;
        add     dword [device.bytes_tx], ecx
        adc     dword [device.bytes_tx + 4], 0

        ret     8

  .fail:
        DEBUGF  1, "Failed!\n"

        ret     8








;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

        DEBUGF  1,"\n%s int ", my_service

; Find pointer of device wich made IRQ occur

        mov     ecx, [devices]
        test    ecx, ecx
        jz      .nothing
        mov     esi, device_list
  .nextdevice:
        mov     ebx, [esi]

        set_io  0
        set_io  IntrStatus
        in      ax, dx
        out     dx, ax                  ; send it back to ACK
        test    ax, ax
        jnz     .got_it
  .continue:
        add     esi, 4
        dec     ecx
        jnz     .nextdevice
  .nothing:                             ; If no device was found, abort (The irq was probably for a device, not registered to this driver)
        DEBUGF  1, "not for me\n"
        ret


  .got_it:

        DEBUGF  1, "status=0x%x\n", ax

        push    ax

        test    ax, IntrRxDone
        jz      .not_RX

        push    ebx
  .more_RX:
        pop     ebx

; Get the current descripter pointer

        movzx   eax, [device.cur_rx]
        mov     ecx, rx_head.sizeof
        mul     ecx
        lea     edi, [device.rx_ring]
        add     edi, eax

; Check it's status

        test    [edi + rx_head.status], RX_SBITS_OWN_BIT
        jnz     .not_bit_own

        DEBUGF  1, "Packet status = 0x%x\n", [edi + rx_head.status]

; TODO: check error bits

; get length

        mov     ecx, [edi + rx_head.status]
        and     ecx, RX_SBITS_FRAME_LENGTH
        shr     ecx, 16
        sub     ecx, 4  ; We dont want CRC

; Update stats

        add     dword [device.bytes_rx], ecx
        adc     dword [device.bytes_rx + 4], 0
        inc     [device.packets_rx]

; Push packet size and pointer, kernel will need it..

        push    ebx
        push    .more_RX        ; return ptr

        push    ecx             ; full packet size
        push    [edi + rx_head.buff_addr_virt]

; reset the RX descriptor

        push    edi
        stdcall KernelAlloc, PKT_BUF_SZ
        pop     edi
        mov     [edi + rx_head.buff_addr_virt], eax
        GetRealAddr
        mov     [edi + rx_head.buff_addr], eax
        mov     [edi + rx_head.status], RX_SBITS_OWN_BIT

; Use next descriptor next time

        inc     [device.cur_rx]
        and     [device.cur_rx], RX_RING_SIZE - 1

; At last, send packet to kernel

        jmp     Eth_input

  .not_bit_own:
  .not_RX:

        pop     ax

        test    ax, IntrTxDone
        jz      .not_TX

      .loop_tx:
        movzx   eax, [device.last_tx]
        mov     ecx, tx_head.sizeof
        mul     ecx
        lea     edi, [device.tx_ring]
        add     edi, eax

        test    [edi + tx_head.status], TX_SBITS_OWN_BIT
        jnz     .not_TX

        cmp     [edi + tx_head.buff_addr_virt], 0
        je      .not_TX

        DEBUGF  1,"Freeing buffer 0x%x\n", [edi + tx_head.buff_addr_virt]

        push    [edi + tx_head.buff_addr_virt]
        mov     [edi + tx_head.buff_addr_virt], 0
        call    KernelFree

        inc     [device.last_tx]
        and     [device.last_tx], TX_RING_SIZE - 1

        jmp     .loop_tx

  .not_TX:

        ; On Rhine-II, Bit 3 indicates Tx descriptor write-back race.
if 0
        cmp     [device.chip_id], 0x3065 ;if (tp->chip_id == 0x3065)
        jne     @f
        push    ax
        xor     eax, eax
        set_io  IntrStatus2
        in      al, dx ;  intr_status |= inb(nic->ioaddr + IntrStatus2) << 16;
        shl     eax, 16
        pop     ax
    @@:
end if

if 0

        ; Acknowledge all of the current interrupt sources ASAP.
        xor     ecx, ecx
        test    eax, IntrTxDescRace
        jz      @f
        set_io  0
        set_io  IntrStatus2
        push    ax
        mov     al, 0x08
        out     dx, al
        pop     ax
    @@:
        set_io  0
        set_io  IntrStatus
        out     dx, ax
        IOSYNC

end if

        ret





; End of code

section '.data' data readable writable align 16 ; place all uninitialized data here
align 4                                         ; Place all initialised data here

devices         dd 0
version         dd (DRIVER_VERSION shl 16) or (API_VERSION and 0xFFFF)
my_service      db 'RHINE',0                    ; max 16 chars including zero

devicelist:
                dd 0x30431106, rhine_3043;, RHINE_IOTYPE, RHINE_I_IOSIZE, CanHaveMII or ReqTxAlign or HasV1TxStat
                dd 0x61001106, rhine_6100;, RHINE_IOTYPE, RHINE_I_IOSIZE, CanHaveMII or ReqTxAlign or HasV1TxStat
                dd 0x30651106, rhine_6102;, RHINE_IOTYPE, RHINEII_IOSIZE, CanHaveMII or HasWOL
                dd 0x31061106, rhine_6105;, RHINE_IOTYPE, RHINEII_IOSIZE, CanHaveMII or HasWOL
                ; Duplicate entry, with 'M' features enabled.
                dd 0x31061106, rhine_6105;, RHINE_IOTYPE, RHINEII_IOSIZE, CanHaveMII or HasWOL or HasIPChecksum or HasVLAN
                dd 0x30531106, rhine_3053;, RHINE_IOTYPE, RHINEII_IOSIZE, CanHaveMII or HasWOL
                dd 0

rhine_3043      db "VIA VT3043 Rhine", 0
rhine_6100      db "VIA VT86C100A Rhine", 0
rhine_6102      db "VIA VT6102 Rhine-II", 0
rhine_6105      db "VIA VT6105LOM Rhine-III (3106)", 0
rhine_3053      db "VIA VT6105M Rhine-III (3053 prototype)", 0

include_debug_strings                           ; All data wich FDO uses will be included here

device_list     rd MAX_DEVICES                  ; This list contains all pointers to device structures the driver is handling

