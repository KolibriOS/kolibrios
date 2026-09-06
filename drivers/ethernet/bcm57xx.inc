;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2026. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  bcm57xx.inc - Broadcom NetXtreme register definitions          ;;
;;                                                                 ;;
;;  Register names and values follow OpenBSD's if_bgereg.h         ;;
;;  (Bill Paul / Wind River) with the BGE_ prefix dropped.         ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;---------------------------------------------------------------------
; PCI configuration space (vendor specific part)
;---------------------------------------------------------------------

PCI_CMD_PIO                     = 0x0001
PCI_CMD_MMIO                    = 0x0002
PCI_CMD_MASTER                  = 0x0004

PCI_CACHESZ                     = 0x0c            ; cacheline size / latency
PCI_PWRMGMT_CMD                 = 0x4c            ; PMCSR (fixed on all bge)
PCI_PWR_STATE_MASK              = 0x0003
PCI_PWR_PME_ENABLE              = 0x0100

PCI_MISC_CTL                    = 0x68
PCI_DMA_RW_CTL                  = 0x6c
PCI_PCISTATE                    = 0x70
PCI_CLKCTL                      = 0x74
PCI_REG_BASEADDR                = 0x78            ; indirect CSR window
PCI_MEMWIN_BASEADDR             = 0x7c            ; NIC memory window
PCI_REG_DATA                    = 0x80
PCI_MEMWIN_DATA                 = 0x84
PCI_PRODID_ASICREV              = 0xbc

; PCI misc host control bits
PCIMISCCTL_CLEAR_INTA           = 0x00000001
PCIMISCCTL_MASK_PCI_INTR        = 0x00000002
PCIMISCCTL_ENDIAN_BYTESWAP      = 0x00000004
PCIMISCCTL_ENDIAN_WORDSWAP      = 0x00000008
PCIMISCCTL_PCISTATE_RW          = 0x00000010
PCIMISCCTL_CLOCKCTL_RW          = 0x00000020
PCIMISCCTL_REG_WORDSWAP         = 0x00000040
PCIMISCCTL_INDIRECT_ACCESS      = 0x00000080
PCIMISCCTL_TAGGED_STATUS        = 0x00000200
PCIMISCCTL_ASICREV_SHIFT        = 16

; Value kept in PCI_MISC_CTL for the whole driver lifetime.
; MASK_PCI_INTR is cleared again once the rings are live.
BGE_INIT        = PCIMISCCTL_CLEAR_INTA \
                or PCIMISCCTL_MASK_PCI_INTR \
                or PCIMISCCTL_INDIRECT_ACCESS \
                or PCIMISCCTL_ENDIAN_WORDSWAP \
                or PCIMISCCTL_PCISTATE_RW \
                or PCIMISCCTL_CLOCKCTL_RW

; PCI DMA read/write control
PCIDMARWCTL_USE_MRM             = 0x00400000
PCIDMARWCTL_ASRT_ALL_BE         = 0x00800000
PCIDMARWCTL_DEFAULT             = 0x763f0000      ; RD/WR cmd + watermarks

; PCI state
PCISTATE_INTR_NOT_ACTIVE        = 0x00000002

;---------------------------------------------------------------------
; Memory mapped registers (BAR0)
;---------------------------------------------------------------------

; --- Mailboxes (0x0200 - 0x03ff) --------------------------------------
MBX_IRQ0_HI                     = 0x0200
MBX_IRQ0_LO                     = 0x0204
MBX_RX_STD_PROD_LO              = 0x026c
MBX_RX_JUMBO_PROD_LO            = 0x0274
MBX_RX_MINI_PROD_LO             = 0x027c
MBX_RX_CONS0_LO                 = 0x0284
MBX_TX_HOST_PROD0_LO            = 0x0304

; --- Ethernet MAC (0x0400) --------------------------------------------
MAC_MODE                        = 0x0400
MAC_STS                         = 0x0404
MAC_EVT_ENB                     = 0x0408
MAC_LED_CTL                     = 0x040c
MAC_ADDR0_HI                    = 0x0410
MAC_ADDR0_LO                    = 0x0414
MAC_TX_BACKOFF_SEED             = 0x0438
MAC_RX_MTU                      = 0x043c
MI_COMM                         = 0x044c
MI_STS                          = 0x0450
MI_MODE                         = 0x0454
MAC_TX_MODE                     = 0x045c
MAC_TX_LENGTHS                  = 0x046c
MAC_RX_MODE                     = 0x0468
RX_RULES_CFG                    = 0x0500

MAC_MODE_RESET                  = 0x00000001
MAC_MODE_HALF_DUPLEX            = 0x00000002
MAC_MODE_PORT_MODE_MII          = 0x00000004
MAC_MODE_PORT_MODE_GMII         = 0x00000008
MAC_MODE_PORT_MODE_TBI          = 0x0000000c
MAC_MODE_PORT_MODE_MASK         = 0x0000000c
MAC_MODE_RX_STATS_EN            = 0x00000800
MAC_MODE_RX_STATS_CLEAR         = 0x00001000
MAC_MODE_TX_STATS_EN            = 0x00004000
MAC_MODE_TX_STATS_CLEAR         = 0x00008000
MAC_MODE_TXDMA_EN               = 0x00200000
MAC_MODE_RXDMA_EN               = 0x00400000
MAC_MODE_FRMHDR_DMA_EN          = 0x00800000

MACSTAT_CFG_CHANGED             = 0x00000004
MACSTAT_SYNC_CHANGED            = 0x00000008
MACSTAT_LINK_CHANGED            = 0x00001000
MACSTAT_MI_COMPLETE             = 0x00400000
MACSTAT_STICKY                  = MACSTAT_LINK_CHANGED or MACSTAT_SYNC_CHANGED \
                                  or MACSTAT_CFG_CHANGED or MACSTAT_MI_COMPLETE
EVTENB_LINK_CHANGED             = 0x00001000

RX_MODE_RESET                   = 0x00000001
RX_MODE_ENABLE                  = 0x00000002
RX_MODE_FLOW_CTRL_ENABLE        = 0x00000004
RX_MODE_PROMISC                 = 0x00000100
RX_MODE_KEEP_VLAN_TAG           = 0x00000400

TX_MODE_RESET                   = 0x00000001
TX_MODE_ENABLE                  = 0x00000002
TX_MODE_FLOW_CTRL_ENABLE        = 0x00000010
TXMODE_MBUF_LOCKUP_FIX          = 0x00000100

TX_LENGTHS_DEFAULT              = 0x00002620      ; inter packet gap etc.
TX_BACKOFF_SEED_MASK            = 0x000003ff

; MI (PHY) access
MIMODE_CLK_10MHZ                = 0x000c0000      ; "base" value
MIMODE_AUTOPOLL                 = 0x00000010
MIMODE_PHYADDR_1                = 0x00000020

MICOMM_DATA_MASK                = 0x0000ffff
MICOMM_REG_SHIFT                = 16
MICOMM_PHY_SHIFT                = 21
MICOMM_CMD_WRITE                = 0x04000000
MICOMM_CMD_READ                 = 0x08000000
MICOMM_READFAIL                 = 0x10000000
MICOMM_BUSY                     = 0x20000000

MISTS_LINK                      = 0x00000001
MISTS_10MBPS                    = 0x00000002

; --- Send Data Initiator (0x0c00) -------------------------------------
SDI_MODE                        = 0x0c00
SDI_STATS_ENABLE_MASK           = 0x0c08
SDI_STATS_CTL                   = 0x0c0c
SDIMODE_ENABLE                  = 0x00000002

; --- Send Data Completion (0x1000) ------------------------------------
SDC_MODE                        = 0x1000
SDCMODE_ENABLE                  = 0x00000002

; --- Send BD Ring Selector (0x1400) -----------------------------------
SRS_MODE                        = 0x1400
SRSMODE_ENABLE                  = 0x00000002

; --- Send BD Initiator (0x1800) ---------------------------------------
SBDI_MODE                       = 0x1800
SBDIMODE_ENABLE                 = 0x00000002

; --- Send BD Completion (0x1c00) --------------------------------------
SBDC_MODE                       = 0x1c00
SBDCMODE_ENABLE                 = 0x00000002

; --- Receive List Placement (0x2000) ----------------------------------
RXLP_MODE                       = 0x2000
RXLP_CFG                        = 0x2010
RXLP_STATS_CTL                  = 0x2014
RXLP_STATS_ENABLE_MASK          = 0x2018
RXLPMODE_ENABLE                 = 0x00000002
RXLP_CFG_DEFAULT                = 0x00000181

; --- Receive Data & BD Initiator (0x2400) -----------------------------
RDBDI_MODE                      = 0x2400
RDBDIMODE_ENABLE                = 0x00000002
RDBDIMODE_ATTN                  = 0x00000004

; Receive ring control blocks live in this block
RX_JUMBO_RCB_HADDR_HI           = 0x2440
RX_JUMBO_RCB_HADDR_LO           = 0x2444
RX_JUMBO_RCB_MAXLEN_FLAGS       = 0x2448
RX_JUMBO_RCB_NICADDR            = 0x244c
RX_STD_RCB_HADDR_HI             = 0x2450
RX_STD_RCB_HADDR_LO             = 0x2454
RX_STD_RCB_MAXLEN_FLAGS         = 0x2458
RX_STD_RCB_NICADDR              = 0x245c
RX_MINI_RCB_HADDR_HI            = 0x2460
RX_MINI_RCB_HADDR_LO            = 0x2464
RX_MINI_RCB_MAXLEN_FLAGS        = 0x2468
RX_MINI_RCB_NICADDR             = 0x246c

; --- Receive Data Completion (0x2800) ---------------------------------
RDC_MODE                        = 0x2800
RDCMODE_ENABLE                  = 0x00000002
RDCMODE_ATTN                    = 0x00000004

; --- Receive BD Initiator (0x2c00) ------------------------------------
RBDI_MODE                       = 0x2c00
RBDI_STD_REPL_THRESH            = 0x2c18
RBDI_JUMBO_REPL_THRESH          = 0x2c1c
RBDIMODE_ENABLE                 = 0x00000002
RBDIMODE_ATTN                   = 0x00000004

; --- Receive BD Completion (0x3000) -----------------------------------
RBDC_MODE                       = 0x3000
RBDCMODE_ENABLE                 = 0x00000002
RBDCMODE_ATTN                   = 0x00000004

; --- Receive List Selector (0x3400), pre-5705 only --------------------
RXLS_MODE                       = 0x3400
RXLSMODE_ENABLE                 = 0x00000002

; --- Host Coalescing (0x3c00) -----------------------------------------
HCC_MODE                        = 0x3c00
HCC_STATUS                      = 0x3c04
HCC_RX_COAL_TICKS               = 0x3c08
HCC_TX_COAL_TICKS               = 0x3c0c
HCC_RX_MAX_COAL_BDS             = 0x3c10
HCC_TX_MAX_COAL_BDS             = 0x3c14
HCC_RX_COAL_TICKS_INT           = 0x3c18
HCC_TX_COAL_TICKS_INT           = 0x3c1c
HCC_RX_MAX_COAL_BDS_INT         = 0x3c20
HCC_TX_MAX_COAL_BDS_INT         = 0x3c24
HCC_STATS_TICKS                 = 0x3c28
HCC_STATS_ADDR_HI               = 0x3c30
HCC_STATS_ADDR_LO               = 0x3c34
HCC_STATUSBLK_ADDR_HI           = 0x3c38
HCC_STATUSBLK_ADDR_LO           = 0x3c3c
HCC_STATS_BASEADDR              = 0x3c40
HCC_STATUSBLK_BASEADDR          = 0x3c44

HCCMODE_RESET                   = 0x00000001
HCCMODE_ENABLE                  = 0x00000002
HCCMODE_ATTN                    = 0x00000004
HCCMODE_COAL_NOW                = 0x00000008
HCCMODE_STATBLKSZ_32BYTE        = 0x00000100

; --- Memory Arbiter (0x4000) ------------------------------------------
MARB_MODE                       = 0x4000
MARBMODE_ENABLE                 = 0x00000002

; --- Buffer Manager (0x4400), pre-5705 needs explicit setup -----------
BMAN_MODE                       = 0x4400
BMAN_STATUS                     = 0x4404
BMAN_MBUFPOOL_BASEADDR          = 0x4408
BMAN_MBUFPOOL_LEN               = 0x440c
BMAN_MBUFPOOL_READDMA_LOWAT     = 0x4410
BMAN_MBUFPOOL_MACRX_LOWAT       = 0x4414
BMAN_MBUFPOOL_HIWAT             = 0x4418
BMAN_DMA_DESCPOOL_BASEADDR      = 0x442c
BMAN_DMA_DESCPOOL_LEN           = 0x4430
BMAN_DMA_DESCPOOL_LOWAT         = 0x4434
BMAN_DMA_DESCPOOL_HIWAT         = 0x4438

BMANMODE_RESET                  = 0x00000001
BMANMODE_ENABLE                 = 0x00000002
BMANMODE_ATTN                   = 0x00000004
BMANMODE_LOMBUF_ATTN            = 0x00000010

BUFFPOOL_1                      = 0x00010000      ; NIC SRAM mbuf pool
BUFFPOOL_1_LEN                  = 0x00018000
DMA_DESCRIPTORS                 = 0x00002000
DMA_DESCRIPTORS_LEN             = 0x00002000

; --- Read / Write DMA -------------------------------------------------
RDMA_MODE                       = 0x4800
RDMA_RSRVCTRL                   = 0x4900
WDMA_MODE                       = 0x4c00

RDMAMODE_ENABLE                 = 0x00000002
RDMAMODE_ALL_ATTNS              = 0x000003fc
RDMAMODE_BD_SBD_CRPT_ATTN       = 0x00000800
RDMAMODE_MBUF_RBD_CRPT_ATTN     = 0x00001000
RDMAMODE_MBUF_SBD_CRPT_ATTN     = 0x00002000
RDMAMODE_FIFO_LONG_BURST        = 0x00030000
RDMA_RSRVCTRL_FIFO_OFLW_FIX     = 0x00000004

WDMAMODE_ENABLE                 = 0x00000002
WDMAMODE_ALL_ATTNS              = 0x000003fc
WDMAMODE_STATUS_TAG_FIX         = 0x20000000

; --- Flow Through Queues (0x5c00) -------------------------------------
FTQ_RESET                       = 0x5c00

; --- Mode / misc control (0x6800) -------------------------------------
MODE_CTRL                       = 0x6800
MISC_CFG                        = 0x6804
MISC_LOCAL_CTL                  = 0x6808
EE_ADDR                         = 0x6838
EE_DATA                         = 0x683c

MODECTL_BYTESWAP_NONFRAME       = 0x00000002
MODECTL_WORDSWAP_NONFRAME       = 0x00000004
MODECTL_BYTESWAP_DATA           = 0x00000010
MODECTL_WORDSWAP_DATA           = 0x00000020
MODECTL_NO_TX_INTR              = 0x00002000
MODECTL_NO_RX_INTR              = 0x00004000
MODECTL_FORCE_PCI32             = 0x00008000
MODECTL_HOST_STACKUP            = 0x00010000
MODECTL_HOST_SENDBDS            = 0x00020000
MODECTL_TX_ATTN_INTR            = 0x01000000
MODECTL_RX_ATTN_INTR            = 0x02000000
MODECTL_MAC_ATTN_INTR           = 0x04000000
MODECTL_DMA_ATTN_INTR           = 0x08000000
MODECTL_FLOW_ATTN_INTR          = 0x10000000

MISCCFG_RESET_CORE_CLOCKS       = 0x00000001
MISCCFG_TIMER_PRESCALE_66MHZ    = (65 shl 1)

MLC_INTR_ONATTN                 = 0x00000008
MLC_AUTO_EEPROM                 = 0x01000000

; Serial EEPROM auto access
EEADDR_ADDRESS                  = 0x0000fffc
EEADDR_START                    = 0x02000000
EEADDR_RESET                    = 0x20000000
EEADDR_DONE                     = 0x40000000
EEADDR_RW                       = 0x80000000      ; 1 = read
HALFCLK_384SCL                  = 0x60
EE_READCMD      = (HALFCLK_384SCL shl 16) or EEADDR_START or EEADDR_RW or EEADDR_DONE
EE_MAC_OFFSET                   = 0x7c

;---------------------------------------------------------------------
; NIC internal memory (reached through PCI_MEMWIN_BASEADDR/DATA)
;---------------------------------------------------------------------

NICMEM_SEND_RING_RCB            = 0x0100          ; 16 RCBs, 16 bytes each
NICMEM_RX_RETURN_RING_RCB       = 0x0200          ; 16 RCBs, 16 bytes each
NICMEM_SOFTWARE_MAC             = 0x0c14          ; "HK" signature + MAC
NICMEM_SEND_RING_1_TO_4         = 0x4000          ; TX ring mirror, pre-5705
NICMEM_STD_RX_RINGS             = 0x6000          ; RX std ring mirror

STATS_BLOCK_NIC                 = 0x00000300
STATUS_BLOCK_NIC                = 0x00000b00

; Ring control block layout (both in NIC memory and as CSRs)
RCB_HADDR_HI                    = 0x00
RCB_HADDR_LO                    = 0x04
RCB_MAXLEN_FLAGS                = 0x08
RCB_NICADDR                     = 0x0c
sizeof.rcb                      = 0x10

RCB_FLAG_USE_EXT_RX_BD          = 0x0001
RCB_FLAG_RING_DISABLED          = 0x0002

;---------------------------------------------------------------------
; Chip identification
;---------------------------------------------------------------------

; The chip id is the top half of PCI_MISC_CTL, and the ASIC revision is
; its top nibble-and-a-bit: asic_rev = chip_id shr 12.
;
; Note that the revision numbering is NOT chronological. The BCM5700,
; the oldest part in the family, has revision 0x07, which is larger than
; the 0x03 of the 5705. Anything deciding "is this a 5705 or later" by
; comparing revisions therefore gets the 5700 wrong - see is_5705_plus
; in the driver.

ASICREV_BCM5701                 = 0x00
ASICREV_BCM5703                 = 0x01
ASICREV_BCM5704                 = 0x02
ASICREV_BCM5705                 = 0x03
ASICREV_BCM5750                 = 0x04
ASICREV_BCM5714_A0              = 0x05
ASICREV_BCM5752                 = 0x06
ASICREV_BCM5700                 = 0x07
ASICREV_BCM5780                 = 0x08
ASICREV_BCM5714                 = 0x09
ASICREV_BCM5755                 = 0x0a
ASICREV_BCM5787                 = 0x0b
ASICREV_BCM5906                 = 0x0c
ASICREV_USE_PRODID_REG          = 0x0f
ASICREV_BCM57780                = 0x57780

;--- 5700 class, pre-5705 programming model ---------------------------
CHIPID_BCM5700_A0               = 0x7000
CHIPID_BCM5700_B0               = 0x7100
CHIPID_BCM5700_B2               = 0x7102
CHIPID_BCM5700_B3               = 0x7103
CHIPID_BCM5700_ALTIMA           = 0x7104
CHIPID_BCM5700_C0               = 0x7200

CHIPID_BCM5701_A0               = 0x0000
CHIPID_BCM5701_B0               = 0x0100
CHIPID_BCM5701_B2               = 0x0102
CHIPID_BCM5701_B5               = 0x0105

CHIPID_BCM5703_A0               = 0x1000
CHIPID_BCM5703_A1               = 0x1001
CHIPID_BCM5703_A2               = 0x1002
CHIPID_BCM5703_A3               = 0x1003
CHIPID_BCM5703_B0               = 0x1100

CHIPID_BCM5704_A0               = 0x2000
CHIPID_BCM5704_A1               = 0x2001
CHIPID_BCM5704_A2               = 0x2002
CHIPID_BCM5704_A3               = 0x2003
CHIPID_BCM5704_B0               = 0x2100

;--- 5705 and later ---------------------------------------------------
CHIPID_BCM5705_A0               = 0x3000
CHIPID_BCM5705_A1               = 0x3001
CHIPID_BCM5705_A2               = 0x3002
CHIPID_BCM5705_A3               = 0x3003

CHIPID_BCM5750_A0               = 0x4000
CHIPID_BCM5750_A1               = 0x4001
CHIPID_BCM5750_B0               = 0x4010
CHIPID_BCM5750_B1               = 0x4101
CHIPID_BCM5750_C0               = 0x4200
CHIPID_BCM5750_C1               = 0x4201
CHIPID_BCM5750_C2               = 0x4202

CHIPID_BCM57780_A0              = 0x57780000
CHIPID_BCM57780_A1              = 0x57780001

; Later members of the family. These are recognised well enough to be
; named, but none of them has been tested with this driver and several
; want setup it does not do yet (clock power management on the 5784 and
; 5761 class, the PHY quirks on the 5906, jumbo support on the 5714 and
; 5780). Left here so an unknown card can at least be identified from
; the log; uncomment the matching chip_table entries once one has
; actually been tried.
;
; CHIPID_BCM5714_A0             = 0x5000
; CHIPID_BCM5752_A0             = 0x6000
; CHIPID_BCM5752_A1             = 0x6001
; CHIPID_BCM5752_A2             = 0x6002
; CHIPID_BCM5714_B0             = 0x8000
; CHIPID_BCM5714_B3             = 0x8003
; CHIPID_BCM5715_A0             = 0x9000
; CHIPID_BCM5715_A1             = 0x9001
; CHIPID_BCM5715_A3             = 0x9003
; CHIPID_BCM5755_A0             = 0xa000
; CHIPID_BCM5755_A1             = 0xa001
; CHIPID_BCM5755_A2             = 0xa002
; CHIPID_BCM5722_A0             = 0xa200
; CHIPID_BCM5787_A0             = 0xb000
; CHIPID_BCM5787_A1             = 0xb001
; CHIPID_BCM5787_A2             = 0xb002
; CHIPID_BCM5906_A1             = 0xc001
; CHIPID_BCM5906_A2             = 0xc002

;---------------------------------------------------------------------
; DMA structures
;---------------------------------------------------------------------

; Status block. The chip DMA-writes it whenever something happened.
; Only the first 32 bytes are used when STATBLKSZ_32BYTE is selected;
; the tail is reserved so the hardware never writes past the struct.
struct  status_block
        status_word         dd ?          ; bit0 = UPDATED
        tag                 dd ?
        rx_jumbo_consumer   dw ?
        rx_std_consumer     dw ?
        rx_mini_consumer    dw ?
        reserved0           dw ?
        rx_prod_idx         dw ?          ; return ring 0 producer
        tx_cons_idx         dw ?          ; send ring 0 consumer
                            rb 12         ; idx[1..3], written in 32 byte mode
                            rb 48         ; idx[4..15], 80 byte mode
ends

STATUSFLAG_UPDATED              = 0x00000001
STATUSFLAG_LINKSTATE_CHANGED    = 0x00000002
STATUSFLAG_ERROR                = 0x00000004

struct  tx_bd
        addr_hi             dd ?
        addr_lo             dd ?
        len_flags           dd ?          ; length in high word, flags in low
        vlan_tag            dd ?
ends

struct  rx_bd
        addr_hi             dd ?
        addr_lo             dd ?
        len                 dw ?          ; frame length, FCS included
        idx                 dw ?          ; opaque, we store the std ring slot
        flags               dw ?
        type                dw ?
        tcp_udp_cksum       dw ?
        ip_cksum            dw ?
        vlan_tag            dw ?
        error_flag          dw ?
        reserved            dd ?
        opaque              dd ?
ends

TXBDFLAG_TCP_UDP_CKSUM          = 0x0001
TXBDFLAG_IP_CKSUM               = 0x0002
TXBDFLAG_END                    = 0x0004
TXBDFLAG_VLAN_TAG               = 0x0040
TXBDFLAG_COAL_NOW               = 0x0080

RXBDFLAG_END                    = 0x0004
RXBDFLAG_VLAN_TAG               = 0x0040
RXBDFLAG_ERROR                  = 0x0400

;---------------------------------------------------------------------
; Broadcom PHY
;---------------------------------------------------------------------

PHY_ADDR                        = 1
PHY_BMCR                        = 0x00
PHY_BMSR                        = 0x01
PHY_AUX_STAT                    = 0x19

; Basic mode status register. The link and remote fault bits latch low,
; so read it twice to get the current state.
PHY_BMSR_LINK                   = 0x0004
PHY_BMSR_AUTONEG_ABLE           = 0x0008
PHY_BMSR_REMOTE_FAULT           = 0x0010
PHY_BMSR_AUTONEG_COMP           = 0x0020

; Auto-negotiation result, bits 10:8 of the auxiliary status register.
; Note that code 4 is 100BASE-T4 - leaving it out shifts everything
; above it by one and reports gigabit links as 100 Mbit.
AUX_SPEED_MASK                  = 0x0700
AUX_SPEED_10HALF                = 0x0100
AUX_SPEED_10FULL                = 0x0200
AUX_SPEED_100HALF               = 0x0300
AUX_SPEED_100T4                 = 0x0400
AUX_SPEED_100FULL               = 0x0500
AUX_SPEED_1000HALF              = 0x0600
AUX_SPEED_1000FULL              = 0x0700
