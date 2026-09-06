;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  skge.inc  -  Marvell Yukon / SysKonnect GEnesis register & descriptor
;;               definitions for KolibriOS
;;
;;  Faithful port of OpenBSD sys/dev/pci/if_skreg.h + if_skvar.h
;;  (plus the Yukon-specific portions).  Hardware constants are NOT
;;  simplified; the silicon does not care about our aesthetic preferences.
;;
;;  Copyright notices from the original sources are retained below.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;----------------------------------------------------------------------
;  Original copyrights (OpenBSD / FreeBSD / Bill Paul / Nathan Binkert)
;----------------------------------------------------------------------
;
; Copyright (c) 1997, 1998, 1999, 2000 Bill Paul <wpaul@ctr.columbia.edu>
; Copyright (c) 2003 Nathan L. Binkert <binkertn@umich.edu>
; Copyright (c) 2003 The NetBSD Foundation, Inc.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the conditions in the
; original BSD-style licenses are met.  See the OpenBSD sources for
; the full text.
;
;----------------------------------------------------------------------

;======================================================================
;  Windowing helpers (GEnesis has a 256-byte I/O window into a 16 KB
;  internal register space, divided into 128-byte blocks)
;======================================================================

SK_WIN_BASE             = 0x0080
SK_WIN_LEN              = 0x80
SK_WIN_MASK             = 0x3F80
SK_REG_MASK             = 0x7F

; Compute RAP window number and offset inside the window
; (used when the RAP must be programmed)
; SK_WIN(reg)  = ((reg) & SK_WIN_MASK) / SK_WIN_LEN
; SK_REG(reg)  =  (reg) & SK_REG_MASK

SK_PORT_A               = 0
SK_PORT_B               = 1

;----------------------------------------------------------------------
;  Block 0 – permanently mapped at the start of the BAR
;----------------------------------------------------------------------

SK_RAP                  = 0x0000
SK_CSR                  = 0x0004
SK_LED                  = 0x0006
SK_ISR                  = 0x0008                ; interrupt source
SK_IMR                  = 0x000C                ; interrupt mask
SK_IESR                 = 0x0010                ; hw error source
SK_IEMR                 = 0x0014                ; hw error mask
SK_ISSR                 = 0x0018                ; special interrupt source

; Yukon-2 extras (present in later silicon, harmless on classic Yukon)
SK_Y2_ISSR2             = 0x001C
SK_Y2_ISSR3             = 0x0020
SK_Y2_EISR              = 0x0024
SK_Y2_LISR              = 0x0028
SK_Y2_ICR               = 0x002C

; XMAC PHY access windows (Genesis)
SK_XM_IMR0              = 0x0020
SK_XM_ISR0              = 0x0028
SK_XM_PHYADDR0          = 0x0030
SK_XM_PHYDATA0          = 0x0034
SK_XM_IMR1              = 0x0040
SK_XM_ISR1              = 0x0048
SK_XM_PHYADDR1          = 0x0050
SK_XM_PHYDATA1          = 0x0054

; BMU control/status (Block 0 shortcuts)
SK_BMU_RX_CSR0          = 0x0060
SK_BMU_RX_CSR1          = 0x0064
SK_BMU_TXS_CSR0         = 0x0068
SK_BMU_TXA_CSR0         = 0x006C
SK_BMU_TXS_CSR1         = 0x0070
SK_BMU_TXA_CSR1         = 0x0074

;----- SK_CSR bits ----------------------------------------------------
SK_CSR_SW_RESET         = 0x0001
SK_CSR_SW_UNRESET       = 0x0002
SK_CSR_MASTER_RESET     = 0x0004
SK_CSR_MASTER_UNRESET   = 0x0008
SK_CSR_MASTER_STOP      = 0x0010
SK_CSR_MASTER_DONE      = 0x0020
SK_CSR_SW_IRQ_CLEAR     = 0x0040
SK_CSR_SW_IRQ_SET       = 0x0080
SK_CSR_SLOTSIZE         = 0x0100                ; 1 = 64-bit, 0 = 32-bit
SK_CSR_BUSCLOCK         = 0x0200                ; 1 = 33/66, 0 = 33
SK_CSR_ASF_OFF          = 0x1000
SK_CSR_ASF_ON           = 0x2000

;----- SK_LED ---------------------------------------------------------
SK_LED_GREEN_OFF        = 0x01
SK_LED_GREEN_ON         = 0x02

;----- SK_ISR / SK_IMR ------------------------------------------------
SK_ISR_TX2_AS_CHECK     = 0x00000001
SK_ISR_TX2_AS_EOF       = 0x00000002
SK_ISR_TX2_AS_EOB       = 0x00000004
SK_ISR_TX2_S_CHECK      = 0x00000008
SK_ISR_TX2_S_EOF        = 0x00000010
SK_ISR_TX2_S_EOB        = 0x00000020
SK_ISR_TX1_AS_CHECK     = 0x00000040
SK_ISR_TX1_AS_EOF       = 0x00000080
SK_ISR_TX1_AS_EOB       = 0x00000100
SK_ISR_TX1_S_CHECK      = 0x00000200
SK_ISR_TX1_S_EOF        = 0x00000400
SK_ISR_TX1_S_EOB        = 0x00000800
SK_ISR_RX2_CHECK        = 0x00001000
SK_ISR_RX2_EOF          = 0x00002000
SK_ISR_RX2_EOB          = 0x00004000
SK_ISR_RX1_CHECK        = 0x00008000
SK_ISR_RX1_EOF          = 0x00010000
SK_ISR_RX1_EOB          = 0x00020000
SK_ISR_LINK2_OFLOW      = 0x00040000
SK_ISR_MAC2             = 0x00080000
SK_ISR_LINK1_OFLOW      = 0x00100000
SK_ISR_MAC1             = 0x00200000
SK_ISR_TIMER            = 0x00400000
SK_ISR_EXTERNAL_REG     = 0x00800000
SK_ISR_SW               = 0x01000000
SK_ISR_I2C_RDY          = 0x02000000
SK_ISR_TX2_TIMEO        = 0x04000000
SK_ISR_TX1_TIMEO        = 0x08000000
SK_ISR_RX2_TIMEO        = 0x10000000
SK_ISR_RX1_TIMEO        = 0x20000000
SK_ISR_RSVD             = 0x40000000
SK_ISR_HWERR            = 0x80000000

; Convenience masks used by the driver
SK_INTRS1 = (SK_ISR_RX1_EOF or SK_ISR_TX1_S_EOF or SK_ISR_MAC1)
SK_INTRS2 = (SK_ISR_RX2_EOF or SK_ISR_TX2_S_EOF or SK_ISR_MAC2)

; Yukon-2 interrupt bits (for completeness; classic Yukon ignores them)
SK_Y2_IMR_TX1_AS_CHECK  = 0x00000001
SK_Y2_IMR_TX1_S_CHECK   = 0x00000002
SK_Y2_IMR_RX1_CHECK     = 0x00000004
SK_Y2_IMR_MAC1          = 0x00000008
SK_Y2_IMR_PHY1          = 0x00000010
SK_Y2_IMR_TX2_AS_CHECK  = 0x00000100
SK_Y2_IMR_TX2_S_CHECK   = 0x00000200
SK_Y2_IMR_RX2_CHECK     = 0x00000400
SK_Y2_IMR_MAC2          = 0x00000800
SK_Y2_IMR_PHY2          = 0x00001000
SK_Y2_IMR_TIMER         = 0x01000000
SK_Y2_IMR_SW            = 0x02000000
SK_Y2_IMR_ASF           = 0x20000000
SK_Y2_IMR_BMU           = 0x40000000
SK_Y2_IMR_HWERR         = 0x80000000

;----- SK_IESR / SK_IEMR ----------------------------------------------
SK_IESR_PAR_RX2         = 0x00000001
SK_IESR_PAR_RX1         = 0x00000002
SK_IESR_PAR_MAC2        = 0x00000004
SK_IESR_PAR_MAC1        = 0x00000008
SK_IESR_PAR_WR_RAM      = 0x00000010
SK_IESR_PAR_RD_RAM      = 0x00000020
SK_IESR_NO_TSTAMP_MAC2  = 0x00000040
SK_IESR_NO_TSTAMO_MAC1  = 0x00000080
SK_IESR_NO_STS_MAC2     = 0x00000100
SK_IESR_NO_STS_MAC1     = 0x00000200
SK_IESR_IRQ_STS         = 0x00000400
SK_IESR_MASTERERR       = 0x00000800

;----------------------------------------------------------------------
;  Block 2 – MAC addresses, chip ID, EEPROM, timers, GPIO, I2C
;----------------------------------------------------------------------

SK_MAC0_0               = 0x0100
SK_MAC0_1               = 0x0104
SK_MAC1_0               = 0x0108
SK_MAC1_1               = 0x010C
SK_MAC2_0               = 0x0110
SK_MAC2_1               = 0x0114
SK_CONNTYPE             = 0x0118
SK_PMDTYPE              = 0x0119
SK_CONFIG               = 0x011A
SK_CHIPVER              = 0x011B
SK_EPROM0               = 0x011C
SK_EPROM1               = 0x011D                ; yukon/genesis
SK_Y2_CLKGATE           = 0x011D                ; yukon-2
SK_EPROM2               = 0x011E
SK_Y2_HWRES             = 0x011E
SK_EPROM3               = 0x011F
SK_EP_ADDR              = 0x0120
SK_EP_DATA              = 0x0124
SK_EP_LOADCTL           = 0x0128
SK_EP_LOADTST           = 0x0129
SK_TIMERINIT            = 0x0130
SK_TIMER                = 0x0134
SK_TIMERCTL             = 0x0138
SK_TIMERTST             = 0x0139
SK_IMTIMERINIT          = 0x0140
SK_IMTIMER              = 0x0144
SK_IMTIMERCTL           = 0x0148
SK_IMTIMERTST           = 0x0149
SK_IMMR                 = 0x014C
SK_IHWEMR               = 0x0150
SK_TESTCTL1             = 0x0158
SK_TESTCTL2             = 0x0159
SK_GPIO                 = 0x015C
SK_I2CHWCTL             = 0x0160
SK_I2CHWDATA            = 0x0164
SK_I2CHWIRQ             = 0x0168
SK_I2CSW                = 0x016C
SK_BLNKINIT             = 0x0170
SK_BLNKCOUNT            = 0x0174
SK_BLNKCTL              = 0x0178
SK_BLNKSTS              = 0x0179
SK_BLNKTST              = 0x017A

; Chip ID values (read from SK_CHIPVER)
SK_GENESIS              = 0x0A
SK_YUKON                = 0xB0
SK_YUKON_LITE           = 0xB1
SK_YUKON_LP             = 0xB2
SK_YUKON_XL             = 0xB3
SK_YUKON_EC_U           = 0xB4
SK_YUKON_EX             = 0xB5
SK_YUKON_EC             = 0xB6
SK_YUKON_FE             = 0xB7
SK_YUKON_FE_P           = 0xB8
SK_YUKON_SUPR           = 0xB9
SK_YUKON_ULTRA2         = 0xBA
SK_YUKON_OPTIMA         = 0xBC
SK_YUKON_PRM            = 0xBD
SK_YUKON_OPTIMA2        = 0xBE

; Yukon-Lite revision numbers
SK_YUKON_LITE_REV_A0    = 0x0
SK_YUKON_LITE_REV_A1    = 0x3
SK_YUKON_LITE_REV_A3    = 0x7

; PHY type identifiers
SK_PHYTYPE_XMAC         = 0                     ; integrated XMAC II PHY
SK_PHYTYPE_BCOM         = 1                     ; Broadcom BCM5400
SK_PHYTYPE_LONE         = 2                     ; Level One LXT1000
SK_PHYTYPE_NAT          = 3                     ; National DP83891
SK_PHYTYPE_MARV_COPPER  = 4                     ; Marvell 88E1011S copper
SK_PHYTYPE_MARV_FIBER   = 5                     ; Marvell 88E1011S fiber


;----------------------------------------------------------------------
;  Yukon GPHY / GMAC control (block near 0x0F00)
;----------------------------------------------------------------------

SK_GMAC_CTRL            = 0x0F00
SK_GPHY_CTRL            = 0x0F04
SK_GMAC_ISR             = 0x0F08
SK_GMAC_IMR             = 0x0F0C
SK_LINK_CTRL            = 0x0F10

SK_GMAC_LOOP_ON         = 0x00000020
SK_GMAC_LOOP_OFF        = 0x00000010
SK_GMAC_PAUSE_ON        = 0x00000008
SK_GMAC_PAUSE_OFF       = 0x00000004
SK_GMAC_RESET_CLEAR     = 0x00000002
SK_GMAC_RESET_SET       = 0x00000001

SK_GPHY_INT_POL_HI      = 0x08000000
SK_GPHY_DIS_FC          = 0x02000000
SK_GPHY_DIS_SLEEP       = 0x01000000
SK_GPHY_HWCFG_M_3       = 0x00800000
SK_GPHY_HWCFG_M_2       = 0x00400000
SK_GPHY_HWCFG_M_1       = 0x00200000
SK_GPHY_HWCFG_M_0       = 0x00100000
SK_GPHY_ANEG_0          = 0x00080000
SK_GPHY_ENA_XC          = 0x00040000
SK_GPHY_ANEG_3          = 0x00010000
SK_GPHY_ANEG_2          = 0x00008000
SK_GPHY_ANEG_1          = 0x00004000
SK_GPHY_ENA_PAUSE       = 0x00002000
SK_GPHY_RESET_CLEAR     = 0x00000002
SK_GPHY_RESET_SET       = 0x00000001

SK_GPHY_COPPER          = (SK_GPHY_HWCFG_M_0 or SK_GPHY_HWCFG_M_1 or SK_GPHY_HWCFG_M_2 or SK_GPHY_HWCFG_M_3)
SK_GPHY_FIBER           = (SK_GPHY_HWCFG_M_0 or SK_GPHY_HWCFG_M_1 or SK_GPHY_HWCFG_M_2)
SK_GPHY_ANEG_ALL        = (SK_GPHY_ANEG_0 or SK_GPHY_ANEG_1 or SK_GPHY_ANEG_2 or SK_GPHY_ANEG_3)

SK_LINK_RESET_CLEAR     = 0x0002
SK_LINK_RESET_SET       = 0x0001

SK_RAMCTL               = 0x01A0
SK_RAMCTL_RESET         = 0x0001
SK_RAMCTL_UNRESET       = 0x0002

; Yukon GMAC register space base (port 0)
YUKON_REG_BASE          = 0x2800

; GPCR bits (relative to YUKON_REG_BASE)
YU_GPCR_TXEN            = 0x1000
YU_GPCR_RXEN            = 0x0800
YU_GPCR_GIG             = 0x0080
YU_GPCR_DUPLEX          = 0x0020

YU_RCR_UFLEN            = 0x8000                ; unicast filter enable
YU_RCR_MUFLEN           = 0x4000                ; multicast filter enable
YU_RCR_CRCR             = 0x2000                ; strip CRC
YU_RCR_PASSFC           = 0x1000                ; pass flow-control frames

;----------------------------------------------------------------------
;  Descriptor formats (classic GEnesis / Yukon-1)
;----------------------------------------------------------------------

SK_RING_ALIGN           = 64

; RX descriptor (32 bytes)
struct SK_RX_DESC
        sk_ctl          dd ?
        sk_next         dd ?
        sk_data_lo      dd ?
        sk_data_hi      dd ?
        sk_xmac_rxstat  dd ?
        sk_timestamp    dd ?
        sk_csum         dd ?
        sk_csum_start   dd ?
ends

SK_OPCODE_DEFAULT       = 0x00550000
SK_OPCODE_CSUM          = 0x00560000

SK_RXCTL_LEN            = 0x0000FFFF
SK_RXCTL_OPCODE         = 0x00FF0000
SK_RXCTL_TSTAMP_VALID   = 0x01000000
SK_RXCTL_STATUS_VALID   = 0x02000000
SK_RXCTL_DEV0           = 0x04000000
SK_RXCTL_EOF_INTR       = 0x08000000
SK_RXCTL_EOB_INTR       = 0x10000000
SK_RXCTL_LASTFRAG       = 0x20000000
SK_RXCTL_FIRSTFRAG      = 0x40000000
SK_RXCTL_OWN            = 0x80000000

SK_RXSTAT = (SK_OPCODE_DEFAULT or SK_RXCTL_EOF_INTR or \
             SK_RXCTL_LASTFRAG or SK_RXCTL_FIRSTFRAG or SK_RXCTL_OWN)

; TX descriptor (32 bytes)
struct SK_TX_DESC
        sk_ctl          dd ?
        sk_next         dd ?
        sk_data_lo      dd ?
        sk_data_hi      dd ?
        sk_xmac_txstat  dd ?
        sk_rsvd0        dw ?
        sk_csum_startval dw ?
        sk_csum_startpos dw ?
        sk_csum_writepos dw ?
        sk_rsvd1        dd ?
ends

SK_TXCTL_LEN            = 0x0000FFFF
SK_TXCTL_OPCODE         = 0x00FF0000
SK_TXCTL_SW             = 0x01000000
SK_TXCTL_NOCRC          = 0x02000000
SK_TXCTL_STORENFWD      = 0x04000000
SK_TXCTL_EOF_INTR       = 0x08000000
SK_TXCTL_EOB_INTR       = 0x10000000
SK_TXCTL_LASTFRAG       = 0x20000000
SK_TXCTL_FIRSTFRAG      = 0x40000000
SK_TXCTL_OWN            = 0x80000000

SK_TXSTAT = (SK_OPCODE_DEFAULT or SK_TXCTL_EOF_INTR or SK_TXCTL_STORENFWD or \
             SK_TXCTL_FIRSTFRAG or SK_TXCTL_LASTFRAG or SK_TXCTL_OWN)

; Default ring sizes (OpenBSD values; may be reduced for KolibriOS memory)
SK_TX_RING_CNT          = 512
SK_RX_RING_CNT          = 256

;----------------------------------------------------------------------
;  Yukon (Marvell) GMAC registers
;  Base addresses inside the windowed space:
;    Port A : 0x2800
;    Port B : 0x3800
;----------------------------------------------------------------------

YUKON_GPSR              = 0x0000                ; General Purpose Status
YUKON_GPCR              = 0x0004                ; General Purpose Control
YUKON_TCR               = 0x0008                ; Transmit Control
YUKON_RCR               = 0x000C                ; Receive Control
YUKON_TFCR              = 0x0010                ; Tx Flow Control
YUKON_TPR               = 0x0014                ; Tx Parameter
YUKON_SMR               = 0x0018                ; Serial Mode
YUKON_SAL1              = 0x001C                ; Source Address 1 low
YUKON_SAM1              = 0x0020
YUKON_SAH1              = 0x0024
YUKON_SAL2              = 0x0028
YUKON_SAM2              = 0x002C
YUKON_SAH2              = 0x0030
YUKON_MCAH1             = 0x0034                ; Multicast hash
YUKON_MCAH2             = 0x0038
YUKON_MCAH3             = 0x003C
YUKON_MCAH4             = 0x0040
YUKON_TIR               = 0x0044                ; Tx Interrupt
YUKON_RIR               = 0x0048                ; Rx Interrupt
YUKON_TRIR              = 0x004C
YUKON_TIMR              = 0x0050
YUKON_RIMR              = 0x0054
YUKON_TRIMR             = 0x0058
YUKON_SMICR             = 0x0080                ; SMI Control
YUKON_SMIDR             = 0x0084                ; SMI Data
YUKON_PAR               = 0x0088                ; PHY Address / MIB

; SMI control bits
YU_SMICR_PHYAD_SHIFT    = 11
YU_SMICR_REGAD_SHIFT    = 6
YU_SMICR_OP_READ        = 0x0020
YU_SMICR_OP_WRITE       = 0x0000
YU_SMICR_READ_VALID     = 0x0010
YU_SMICR_BUSY           = 0x0008

; Receive status bits (inside descriptor / status word)
YU_RXSTAT_FOFL          = 0x00000001
YU_RXSTAT_CRCERR        = 0x00000002
YU_RXSTAT_FRAGMENT      = 0x00000008
YU_RXSTAT_LONGERR       = 0x00000010
YU_RXSTAT_MIIERR        = 0x00000020
YU_RXSTAT_BADFC         = 0x00000040
YU_RXSTAT_GOODFC        = 0x00000080
YU_RXSTAT_RXOK          = 0x00000100
YU_RXSTAT_BROADCAST     = 0x00000200
YU_RXSTAT_MULTICAST     = 0x00000400
YU_RXSTAT_RUNT          = 0x00000800
YU_RXSTAT_JABBER        = 0x00001000
YU_RXSTAT_VLAN          = 0x00002000
YU_RXSTAT_LENSHIFT      = 16

;----------------------------------------------------------------------
;  Queue / BMU register blocks (relative offsets)
;  Actual address = base + (port * factor * SK_WIN_LEN)
;----------------------------------------------------------------------

; These are the classic GEnesis queue registers; the exact base addresses
; appear in the full OpenBSD header (Q_R1, Q_R2, Q_XS1, Q_XA1, …).
; They will be filled in as we implement the BMU init path.


;----------------------------------------------------------------------
;  Marvell 88E1011 integrated PHY (Yukon)
;----------------------------------------------------------------------

PHY_ADDR_MARV           = 0

PHY_MARV_CTRL           = 0x00
PHY_MARV_STAT           = 0x01
PHY_MARV_ID0            = 0x02
PHY_MARV_ID1            = 0x03
PHY_MARV_AUNE_ADV       = 0x04
PHY_MARV_AUNE_LP        = 0x05
PHY_MARV_1000T_CTRL     = 0x09
PHY_MARV_1000T_STAT     = 0x0A
PHY_MARV_PHY_CTRL       = 0x10
PHY_MARV_PHY_STAT       = 0x11
PHY_MARV_INT_MASK       = 0x12
PHY_MARV_INT_STAT       = 0x13

; PHY specific status bits
PHY_M_PS_SPEED_1000     = 1 shl 15
PHY_M_PS_SPEED_100      = 1 shl 14
PHY_M_PS_FULL_DUP       = 1 shl 13
PHY_M_PS_SPDUP_RES      = 1 shl 11
PHY_M_PS_LINK_UP        = 1 shl 10

; PHY interrupt status / mask bits
PHY_M_IS_AN_ERROR       = 1 shl 15
PHY_M_IS_LSP_CHANGE     = 1 shl 14
PHY_M_IS_DUP_CHANGE     = 1 shl 13
PHY_M_IS_AN_COMPL       = 1 shl 11
PHY_M_IS_LST_CHANGE     = 1 shl 10
PHY_M_IS_FIFO_ERROR     = 1 shl 7

PHY_M_IS_DEF_MSK        = (PHY_M_IS_AN_ERROR or PHY_M_IS_LSP_CHANGE or PHY_M_IS_LST_CHANGE or PHY_M_IS_FIFO_ERROR)


;----------------------------------------------------------------------
;  KolibriOS device private data (extends ETH_DEVICE)
;----------------------------------------------------------------------
;
; The OpenBSD softc is split into sk_softc (controller) and
; sk_if_softc (per-port).  For KolibriOS we keep a single device
; structure per port and store the shared controller state inside it
; (most boards are single-port).

struct SK_DEVICE        ETH_DEVICE

        ; PCI / MMIO
        mmio_addr       dd ?                    ; mapped BAR0
        pci_bus         dd ?
        pci_dev         dd ?
        irq_line        db ?
                        rb 3

        ; Chip identification
        chip_id         db ?                    ; SK_YUKON / SK_GENESIS …
        chip_rev        db ?
        phy_type        db ?                    ; SK_PHYTYPE_*
        port            db ?                    ; 0 or 1

        copper          db ?                    ; 1 = copper, 0 = fiber
        macs            db ?                    ; number of MACs on ASIC
                        rb 2

        chip_name       dd ?                    ; ptr to ASCII chip name
        rev_name        dd ?                    ; ptr to revision string (or NULL)

        ; RAM buffer layout (filled by probe)
        ramsize         dd ?
        rx_ramstart     dd ?
        rx_ramend       dd ?
        tx_ramstart     dd ?
        tx_ramend       dd ?

        ; Ring indices
        cur_rx          dd ?
        cur_tx          dd ?
        dirty_tx        dd ?

        ; Descriptor rings (physically contiguous, aligned)
        rx_ring         dd ?                    ; virtual address
        tx_ring         dd ?
        rx_ring_phys    dd ?
        tx_ring_phys    dd ?

        ; Packet buffers (one-to-one with descriptors for simplicity)
        rx_buffers      dd ?                    ; array of buffer pointers
        tx_buffers      dd ?

        ; Link state
        link            dd ?                    ; 0 = down, 1 = up
        speed           dd ?                    ; 10/100/1000
        duplex          dd ?                    ; 0 = half, 1 = full

        ; Interrupt mask currently programmed into SK_IMR
        intr_mask       dd ?

ends

;----------------------------------------------------------------------
;  Convenience macros for register access (to be used after mmio_addr
;  is known).  These mirror the OpenBSD CSR_READ_*/CSR_WRITE_* style.
;----------------------------------------------------------------------
;
;  In the .asm file we will implement:
;
;    sk_read32  / sk_write32
;    sk_read16  / sk_write16
;    sk_read8   / sk_write8
;
;  and the windowed variants that program SK_RAP when necessary.
;
;----------------------------------------------------------------------





;----------------------------------------------------------------------
;  Port-A queue / BMU / RAMbuffer blocks (if_skreg.h blocks 8,12,16,20,24,26)
;  Port B would need the +0x80/+0x100 skip described in if_skreg.h;
;  not implemented since both boards you're testing report macs=1.
;----------------------------------------------------------------------

; Block 8 - RX queue 1
SK_RXQ1_CURADDR_LO      = 0x0420
SK_RXQ1_CURADDR_HI      = 0x0424
SK_RXQ1_BMU_CSR         = 0x0434

SK_RXBMU_CLR_IRQ_EOF            = 0x00000002
SK_RXBMU_RX_START               = 0x00000010
SK_RXBMU_TRANSFER_SM_UNRESET    = 0x00000200
SK_RXBMU_DESCWR_SM_UNRESET      = 0x00000800
SK_RXBMU_DESCRD_SM_UNRESET      = 0x00002000
SK_RXBMU_SUPERVISOR_SM_UNRESET  = 0x00008000
SK_RXBMU_PFI_SM_UNRESET         = 0x00020000
SK_RXBMU_FIFO_UNRESET           = 0x00080000
SK_RXBMU_DESC_UNRESET           = 0x00200000

SK_RXBMU_ONLINE = (SK_RXBMU_TRANSFER_SM_UNRESET or SK_RXBMU_DESCWR_SM_UNRESET or \
SK_RXBMU_DESCRD_SM_UNRESET or SK_RXBMU_SUPERVISOR_SM_UNRESET or \
SK_RXBMU_PFI_SM_UNRESET or SK_RXBMU_FIFO_UNRESET or \
SK_RXBMU_DESC_UNRESET)

; Block 12 - TX sync queue 1 (only TX queue we use)
SK_TXQS1_CURADDR_LO     = 0x0620
SK_TXQS1_CURADDR_HI     = 0x0624
SK_TXQS1_BMU_CSR        = 0x0634

SK_TXBMU_CLR_IRQ_EOF             = 0x00000002
SK_TXBMU_TX_START                = 0x00000010
SK_TXBMU_TRANSFER_SM_UNRESET     = 0x00000200
SK_TXBMU_DESCWR_SM_UNRESET       = 0x00000800
SK_TXBMU_DESCRD_SM_UNRESET       = 0x00002000
SK_TXBMU_SUPERVISOR_SM_UNRESET   = 0x00008000
SK_TXBMU_PFI_SM_UNRESET          = 0x00020000
SK_TXBMU_FIFO_UNRESET            = 0x00080000
SK_TXBMU_DESC_UNRESET            = 0x00200000
SK_TXBMU_POLL_ON                 = 0x00000080

SK_TXBMU_ONLINE = (SK_TXBMU_TRANSFER_SM_UNRESET or SK_TXBMU_DESCWR_SM_UNRESET or \
SK_TXBMU_DESCRD_SM_UNRESET or SK_TXBMU_SUPERVISOR_SM_UNRESET or \
SK_TXBMU_PFI_SM_UNRESET or SK_TXBMU_FIFO_UNRESET or \
SK_TXBMU_DESC_UNRESET or SK_TXBMU_POLL_ON)

; Block 16 - RX RAMbuffer 1 / Block 20 - sync TX RAMbuffer 1
SK_RXRB1_START          = 0x0800
SK_RXRB1_END            = 0x0804
SK_RXRB1_WR_PTR         = 0x0808
SK_RXRB1_RD_PTR         = 0x080C
SK_RXRB1_CTLTST         = 0x0828

SK_TXRBS1_START         = 0x0A00
SK_TXRBS1_END           = 0x0A04
SK_TXRBS1_WR_PTR        = 0x0A08
SK_TXRBS1_RD_PTR        = 0x0A0C
SK_TXRBS1_CTLTST        = 0x0A28

SK_RBCTL_RESET          = 0x00000001
SK_RBCTL_UNRESET        = 0x00000002
SK_RBCTL_ON             = 0x00000008

; Block 24 / 26 - RX/TX MAC FIFO 1 basic control
SK_RXF1_CTL             = 0x0C1C
SK_TXF1_CTL             = 0x0D1C

SK_RFCTL_OPERATION_ON   = 0x00000008
SK_RFCTL_RESET_CLEAR    = 0x00000002
SK_RFCTL_RESET_SET      = 0x00000001
SK_TFCTL_OPERATION_ON   = 0x00000008
SK_TFCTL_RESET_CLEAR    = 0x00000002
SK_TFCTL_RESET_SET      = 0x00000001

; SK_EPROM0 -> SRAM size table (UNVERIFIED against real 88E8001/8003 EPROM0 - DEBUGF it and check)
SK_RAMSIZE_512K_64      = 0x1
SK_RAMSIZE_1024K_128    = 0x2
SK_RAMSIZE_1024K_64     = 0x3
SK_RAMSIZE_2048K_128    = 0x4


;----------------------------------------------------------------------
;  Yukon GMAC FIFO (NOT the Genesis MFF at 0x0C1C)
;----------------------------------------------------------------------
RX_GMF_CTRL_T           = 0x0C48
RX_GMF_FL_MSK           = 0x0C4C
RX_GMF_FL_THR           = 0x0C50
TX_GMF_CTRL_T           = 0x0D48

GMF_RX_F_FL_ON          = 1 shl 7
GMF_OPER_ON             = 1 shl 3
GMF_RST_CLR             = 1 shl 1
GMF_RST_SET             = 1 shl 0

; Default flush mask: CRC err | FIFO OV | MII err | bad FC | good FC | too long | fragment
RX_GMF_FL_DEF_MSK       = 0x000000B7
RX_GMF_FL_THR_DEF       = 0x0A

SK_RBCTL_STORENFWD_ON   = 0x00000020
SK_RBCTL_STORENFWD_OFF  = 0x00000010

; BMU clear encoding-error IRQ
SK_RXBMU_CLR_IRQ_ERR    = 0x00000001
SK_TXBMU_CLR_IRQ_ERR    = 0x00000001


; Transmit parameter / serial mode (OpenBSD sk_init_yukon)
YU_TPR_JAM_LEN          = (0x3 shl 14)
YU_TPR_JAM_IPG          = (0xB shl 9)
YU_TPR_JAM2DATA_IPG     = (0x1A shl 4)
YU_SMR_DATA_BLIND       = (0x1C shl 11)
YU_SMR_MFL_VLAN         = 0x0200
YU_SMR_IPG_DATA         = 0x001E

; TX Arbiter MAC 1 (Block 4)
SK_TXAR1_COUNTERCTL     = 0x0210
SK_TXARCTL_ON           = 0x02
SK_TXARCTL_FSYNC_ON     = 0x80

; End of skge.inc
