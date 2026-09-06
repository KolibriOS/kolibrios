;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  sky2.inc  -  Marvell Yukon-2 (88E805x and friends) registers
;;               for KolibriOS
;;
;;  Minimal subset needed to probe the chip and read the permanent
;;  MAC address.  Expand later for rings, BMU, status LE, etc.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; PCI Power Management (capability; standard offset when present)
; We poke the common location used by bge: config offset 0x4C is often PMCSR
; on these chips (capability linked). OpenBSD uses BGE_PCI_PWRMGMT_CMD.
PCI_PWRMGMT_CMD             = 0x4C
PCI_PWR_D0                  = 0x00
PCI_PWR_D3hot               = 0x03
PCI_PWR_PME_ENABLE          = 0x0100

;----------------------------------------------------------------------
;  PCI vendor / device IDs (subset of the Yukon-2 family)
;----------------------------------------------------------------------
PCI_VENDOR_MARVELL      = 0x11AB

; Common 88E805x / Yukon-2 device IDs
PCI_DEVICE_88E8021      = 0x4340
PCI_DEVICE_88E8022      = 0x4341
PCI_DEVICE_88E8035      = 0x4350
PCI_DEVICE_88E8036      = 0x4351
PCI_DEVICE_88E8038      = 0x4352
PCI_DEVICE_88E8039      = 0x4353
PCI_DEVICE_88E8040      = 0x4354
PCI_DEVICE_88E8040T     = 0x4355
PCI_DEVICE_88E8052      = 0x4360
PCI_DEVICE_88E8053      = 0x4361
PCI_DEVICE_88E8055      = 0x4363          ; the one we care about first
PCI_DEVICE_88E8056      = 0x4364
PCI_DEVICE_88E8057      = 0x4362
PCI_DEVICE_88E8058      = 0x436A
PCI_DEVICE_88E8071      = 0x436B
PCI_DEVICE_88E8072      = 0x436C
PCI_DEVICE_88E8075      = 0x4370

;----------------------------------------------------------------------
;  Control / Status registers (block 0)
;----------------------------------------------------------------------
B0_RAP                  = 0x0000
B0_CTST                 = 0x0004          ; Control/Status (16-bit)
B0_POWER_CTRL           = 0x0007
B0_ISRC                 = 0x0008
B0_IMSK                 = 0x000C
B0_HWE_ISRC             = 0x0010
B0_HWE_IMSK             = 0x0014

; Yukon-2 special ISR registers
B0_Y2_SP_ISRC2          = 0x001C
B0_Y2_SP_ISRC3          = 0x0020
B0_Y2_SP_EISR           = 0x0024
B0_Y2_SP_LISR           = 0x0028
B0_Y2_SP_ICR            = 0x002C

; CTST bits we care about for a minimal bring-up
CS_RST_SET              = 0x0001
CS_RST_CLR              = 0x0002
CS_MRST_SET             = 0x0004
CS_MRST_CLR             = 0x0008
CS_STOP_MASTER          = 0x0020
CS_STOP_DONE            = 0x0040
CS_SW_IRQ               = 0x0200
CS_CL_SW_IRQ            = 0x0100

;----------------------------------------------------------------------
;  Block 2 – MAC addresses, chip ID, clocks
;----------------------------------------------------------------------
B2_MAC_1                = 0x0100          ; port 0 permanent MAC (6 bytes)
B2_MAC_2                = 0x0108          ; port 1 (if dual)
B2_MAC_3                = 0x0110
B2_CONN_TYP             = 0x0118
B2_PMD_TYP              = 0x0119
B2_MAC_CFG              = 0x011A          ; chip revision in bits 7:4
B2_CHIP_ID              = 0x011B
B2_E_0                  = 0x011C
B2_Y2_CLK_GATE          = 0x011D
B2_Y2_HW_RES            = 0x011E
B2_E_3                  = 0x011F
B2_Y2_CLK_CTRL          = 0x0120

; Chip ID values (read from B2_CHIP_ID)
CHIP_ID_YUKON_XL        = 0xB3
CHIP_ID_YUKON_EC_U      = 0xB4            ; 88E8055 is often this
CHIP_ID_YUKON_EX        = 0xB5
CHIP_ID_YUKON_EC        = 0xB6
CHIP_ID_YUKON_FE        = 0xB7
CHIP_ID_YUKON_FE_P      = 0xB8
CHIP_ID_YUKON_SUPR      = 0xB9
CHIP_ID_YUKON_UL_2      = 0xBA
CHIP_ID_YUKON_OPT       = 0xBC
CHIP_ID_YUKON_PRM       = 0xBD
CHIP_ID_YUKON_OP_2      = 0xBE

; B2_MAC_CFG
CFG_CHIP_R_MSK          = 0xF0            ; revision in bits 7:4
CFG_SNG_MAC             = 0x01            ; 1 = single MAC

;----------------------------------------------------------------------
;  ASF / test-control / RAM-interface (used during reset)
;----------------------------------------------------------------------
B28_Y2_ASF_STAT_CMD     = 0x0E68          ; 32 bit  ASF Status and Command Reg
Y2_ASF_RESET            = 1 shl 3         ; ASF system in reset state
Y2_ASF_DISABLE          = 1 shl 12        ; ASF unit disable (in B0_CTST)

B2_TST_CTRL1            = 0x0158          ; 8 bit  Test Control Register 1
TST_CFG_WRITE_ON        = 1 shl 1         ; Enable  Config Reg WR
TST_CFG_WRITE_OFF       = 1 shl 0         ; Disable Config Reg WR

; Standard PCI config space status register (offset 0x06) - sticky error bits,
; write-1-to-clear. Cleared during reset in case a previous driver load / BIOS
; left an error latched (matches Linux sky2_reset()).
PCI_STATUS_ERROR_BITS   = 0xF900          ; PARITY | SIG_SYSERR | REC_MABORT |
                                           ; REC_TABORT | SIG_TABORT | DET_PARITY

Y2_CLK_DIV_DIS          = 1 shl 0         ; disable core clock division (B2_Y2_CLK_CTRL)
Y2_LED_STAT_ON          = 1 shl 9         ; "driver loaded" status LED on (B0_CTST)

;----------------------------------------------------------------------
;  Block 8 / Tx Arbiter, GMAC control (SK_REG-style: stride 0x80/port)
;----------------------------------------------------------------------
GMAC_CTRL               = 0x0F00          ; 32 bit  GMAC Control Reg
GMAC_IRQ_MSK            = 0x0F0C          ;  8 bit  GMAC Interrupt Mask Reg
GMAC_LINK_CTRL          = 0x0F10          ; 16 bit  GMAC Link Control Reg

GMLC_RST_CLR            = 1 shl 1         ; Clear GMAC Link Reset
GMLC_RST_SET            = 1 shl 0         ; Set   GMAC Link Reset

; GMAC_CTRL bits (this is the GMAC core's OWN reset - distinct from
; GMAC_LINK_CTRL above, and from GPHY_CTRL below. All three are separate
; reset domains and Linux/OpenBSD both toggle all three during bring-up.
GMC_LOOP_OFF            = 1 shl 4
GMC_PAUSE_ON            = 1 shl 3
GMC_RST_CLR             = 1 shl 1         ; Clear GMAC (core) Reset
GMC_RST_SET             = 1 shl 0         ; Set   GMAC (core) Reset

; GPHY_CTRL - the internal PHY block's own reset (separate register,
; offset 0xf04, SK_REG-style stride 0x80/port like GMAC_CTRL/LINK_CTRL).
; Missing this entirely is what left the PHY permanently unresponsive
; to MDIO on first bring-up - see field log from 2026-08-08.
GPHY_CTRL               = 0x0F04
GPC_RST_CLR             = 1 shl 1         ; Clear GPHY Reset
GPC_RST_SET             = 1 shl 0         ; Set   GPHY Reset

;----------------------------------------------------------------------
;  MMIO-mirrored PCI config space. The chip exposes a window of its own
;  PCI config space directly in MMIO (NOT the same thing as a real PCI
;  config cycle via PciRead/PciWrite) - used for the per-chip "OUR_REG"
;  power-control registers below. See sky2_pci_read32/if_msk.c
;  SK_Y2_PCI_REG() in the reference drivers.
;----------------------------------------------------------------------
Y2_CFG_SPC              = 0x1C00

PCI_DEV_REG1            = 0x40
PCI_DEV_REG2            = 0x44
PCI_DEV_REG3            = 0x80
PCI_DEV_REG4            = 0x84
PCI_DEV_REG5            = 0x88
PCI_CFG_REG_1           = 0x94

; PCI_DEV_REG2 bits
PCI_EN_DUMMY_RD         = 1 shl 3         ; Enable dummy read
PCI_REV_DESC            = 1 shl 2         ; Reverse descriptor bytes (BE CPUs only)
PCI_USEDATA64           = 1 shl 0         ; Use 64-bit data bus extension

; PCI_DEV_REG1 bits - per-port PHY power-down / "coma" mode. These must
; be CLEARED or the internal PHY stays powered down and MDIO will never
; respond, no matter how long you poll (matches the observed symptom).
PCI_Y2_PHY1_POWD        = 1 shl 26
PCI_Y2_PHY2_POWD        = 1 shl 27
PCI_Y2_PHY1_COMA        = 1 shl 28
PCI_Y2_PHY2_COMA        = 1 shl 29

; PCI_DEV_REG4 ASPM control bits, masked off during the EC-Ultra/EX/FE+
; "advanced power control" bring-up sequence (SKY2_HW_ADV_POWER_CTL)
P_FORCE_ASPM_REQUEST    = 1 shl 15
P_ASPM_GPHY_LINK_DOWN   = 1 shl 14
P_ASPM_INT_FIFO_EMPTY   = 1 shl 13
P_ASPM_CLKRUN_REQUEST   = 1 shl 12
P_ASPM_CONTROL_MSK      = P_FORCE_ASPM_REQUEST or P_ASPM_GPHY_LINK_DOWN \
                           or P_ASPM_CLKRUN_REQUEST or P_ASPM_INT_FIFO_EMPTY

; PCI_DEV_REG5 Vmain_av timer bits, similarly masked
P_CTL_TIM_VMAIN_AV_MSK  = 3 shl 27

B2_GP_IO                = 0x015C
GLB_GPIO_STAT_RACE_DIS  = 1 shl 13        ; workaround for dev 4.107 on EC-U/EX

Y2_HW_WOL_ON            = 1 shl 15

;----------------------------------------------------------------------
;  GMAC / PHY block (per-port, stride 0x1000, base 0x2800 for port 0)
;----------------------------------------------------------------------
BASE_GMAC_1             = 0x2800          ; port 0 GMAC/PHY register block
BASE_GMAC_2             = 0x3800          ; port 1 GMAC/PHY register block

; Offsets relative to the per-port GMAC block (add to BASE_GMAC_n)
GM_GP_STAT              = 0x0000          ; 16 bit r/o  General Purpose Status
GM_GP_CTRL              = 0x0004          ; 16 bit r/w  General Purpose Control
GM_TX_CTRL              = 0x0008
GM_RX_CTRL              = 0x000C
GM_SERIAL_MODE	        = 0x0018          ; 16 bit r/w	Serial Mode Register
GM_MC_ADDR_H1           = 0x0034
GM_MC_ADDR_H2           = 0x0038
GM_MC_ADDR_H3           = 0x003C
GM_MC_ADDR_H4           = 0x0040
GM_SMI_CTRL             = 0x0080          ; 16 bit r/w  SMI (MDIO) Control
GM_SMI_DATA             = 0x0084          ; 16 bit r/w  SMI Data
GM_PHY_ADDR             = 0x0088          ; 16 bit r/w  GPHY Address

; GM_GP_STAT bits
GM_GPSR_SPEED           = 1 shl 15        ; 1 = 100 Mbps (see GIG_SPEED for 1000)
GM_GPSR_DUPLEX          = 1 shl 14        ; 1 = Full duplex
GM_GPSR_FC_TX_DIS       = 1 shl 13
GM_GPSR_LINK_UP         = 1 shl 12        ; Link Up Status
GM_GPSR_GIG_SPEED       = 1 shl 4         ; 1 = 1000 Mbps

; GM_GP_CTRL bits
GM_GPCR_TX_ENA          = 1 shl 12
GM_GPCR_RX_ENA          = 1 shl 11

; GM_RX_CTRL bits
GM_RXCR_UCF_ENA         = 1 shl 15        ; enable unicast filtering
GM_RXCR_MCF_ENA         = 1 shl 14        ; enable multicast filtering

; GM_SMI_CTRL bits/fields
GM_SMI_CT_PHY_A_MSK     = 0x1F shl 11
GM_SMI_CT_REG_A_MSK     = 0x1F shl 6
GM_SMI_CT_OP_RD         = 1 shl 5         ; opcode read (0 = write)
GM_SMI_CT_RD_VAL        = 1 shl 4         ; read completed, data valid
GM_SMI_CT_BUSY          = 1 shl 3         ; operation in progress

PHY_ADDR_MARV           = 0               ; internal PHY is always MDIO addr 0
PHY_RETRIES             = 20              ; MDIO poll retries; each retry sleeps
                                           ; ~1ms (Sleep() is ms, not centiseconds -
                                           ; confirmed empirically), so 20 = ~20ms
                                           ; worst-case before giving up

;----------------------------------------------------------------------
;  Marvell internal PHY (88E1xxx) register numbers, via GM_SMI_*
;----------------------------------------------------------------------
PHY_MARV_CTRL           = 0x00            ; PHY Control
PHY_MARV_AUNE_ADV       = 0x04            ; Auto-Neg. Advertisement
PHY_MARV_1000T_CTRL     = 0x09            ; 1000Base-T Control
PHY_MARV_PHY_CTRL       = 0x10            ; PHY Specific Control
PHY_MARV_PHY_STAT       = 0x11            ; PHY Specific Status
PHY_MARV_INT_MASK       = 0x12            ; Interrupt Mask
PHY_MARV_INT_STAT       = 0x13            ; Interrupt Status (r/o, clear-on-read)
PHY_MARV_EXT_CTRL       = 0x14            ; Ext. PHY Specific Control

; PHY_MARV_CTRL bits
PHY_CT_RESET            = 1 shl 15        ; (sc) reset all PHY regs
PHY_CT_SPS_LSB          = 1 shl 13        ; speed select, low bit  -> PHY_CT_SP100
PHY_CT_ANE              = 1 shl 12        ; Auto-Negotiation Enabled
PHY_CT_RE_CFG           = 1 shl 9         ; (sc) Restart Auto-Negotiation
PHY_CT_DUP_MD           = 1 shl 8         ; Duplex Mode (forced)
PHY_CT_SPS_MSB          = 1 shl 6         ; speed select, high bit -> PHY_CT_SP1000
PHY_CT_SP1000           = PHY_CT_SPS_MSB
PHY_CT_SP100            = PHY_CT_SPS_LSB

; PHY_MARV_AUNE_ADV bits
PHY_AN_CSMA             = 1 shl 0         ; selector = IEEE 802.3 (always set)
PHY_M_AN_10_HD          = 1 shl 5
PHY_M_AN_10_FD          = 1 shl 6
PHY_M_AN_100_HD         = 1 shl 7
PHY_M_AN_100_FD         = 1 shl 8

; PHY_MARV_1000T_CTRL bits
PHY_M_1000C_AHD         = 1 shl 8         ; advertise 1000Base-T half duplex
PHY_M_1000C_AFD         = 1 shl 9         ; advertise 1000Base-T full duplex

; PHY_MARV_PHY_CTRL bits (copper, "newer PHY" variants incl. EC Ultra)
PHY_M_PC_MDIX_MSK       = 3 shl 5
PHY_M_PC_ENA_AUTO       = 3               ; enable automatic MDI/MDIX crossover
PHY_M_PC_EN_DET_MSK     = 3 shl 8         ; energy detect mask
PHY_M_PC_DSC_MSK        = 7 shl 12        ; downshift counter mask
PHY_M_PC_DOWN_S_ENA     = 1 shl 11        ; downshift enable

; PHY_MARV_PHY_STAT bits
PHY_M_PS_SPEED_MSK      = 3 shl 14
PHY_M_PS_SPEED_1000     = 1 shl 15
PHY_M_PS_SPEED_100      = 1 shl 14
PHY_M_PS_FULL_DUP       = 1 shl 13
PHY_M_PS_LINK_UP        = 1 shl 10
PHY_M_PS_MDI_X_STAT     = 1 shl 6

; PHY_MARV_INT_MASK / PHY_MARV_INT_STAT bits. Reading INT_STAT clears the
; pending condition at the PHY - no separate ack is needed once this is
; serviced (confirmed against both reference drivers: neither writes
; back to INT_STAT, and the top-level B0_ISRC bit below is just a live
; passthrough of the PHY's own interrupt line).
PHY_M_IS_LSP_CHANGE     = 1 shl 14         ; Link Speed Changed
PHY_M_IS_DUP_CHANGE     = 1 shl 13         ; Duplex Mode Changed
PHY_M_IS_LST_CHANGE     = 1 shl 10         ; Link Status Changed
PHY_M_DEF_MSK           = PHY_M_IS_LSP_CHANGE or PHY_M_IS_DUP_CHANGE \
                           or PHY_M_IS_LST_CHANGE

;----------------------------------------------------------------------
;  Chip-level (not per-port) interrupt status/mask. B0_ISRC/B0_IMSK are
;  already declared above (block 0 registers); B0_ISRC is a live,
;  read-only OR of every currently-asserting enabled source - nothing
;  needs to be written back to it, only to the underlying source that
;  set it (here, PHY_MARV_INT_STAT above).
;----------------------------------------------------------------------
Y2_IS_IRQ_PHY1           = 1 shl 4        ; interrupt from PHY on port 0
Y2_IS_STAT_BMU           = 1 shl 30       ; status BMU (RX/TX completion) interrupt

;----------------------------------------------------------------------
;  List-element (LE) descriptor rings. Yukon-2 uses a uniform 8-byte
;  descriptor for TX, RX, AND the status/completion ring: addr(4),
;  length(2), ctrl/css(1), opcode(1). Cross-checked byte-for-byte
;  against OpenBSD's msk_tx_desc/msk_rx_desc (if_skreg.h) as well as
;  Linux's sky2_tx_le/sky2_rx_le/sky2_status_le - full agreement.
;
;  Deliberately out of scope for this pass: 64-bit DMA addressing
;  (OP_ADDR64 - not needed, this is a 32-bit OS so every physical
;  address already fits in the low 32 bits), scatter-gather/multi-
;  fragment packets, VLAN tagging, checksum offload, RSS, jumbo
;  frames, and TX/RX interrupt-coalescing timers (STAT_*_TIMER_INI -
;  left at their power-on-reset default, which just means the status
;  ring reports completions eagerly rather than batched).
;----------------------------------------------------------------------
OP_ADDR64               = 0x21
OP_BUFFER                = 0x40
OP_PACKET               = 0x41
OP_RXSTAT                = 0x60
OP_TXINDEXLE             = 0x68
HW_OWNER                = 0x80            ; opcode-byte bit: entry owned by hardware
LE_EOP                   = 0x80           ; ctrl-byte bit: end of packet (TX only)

; Receive Frame Status bits (in a status-ring RX entry's "status" field)
GMR_FS_LEN               = 0x7FFF shl 16
GMR_FS_RX_OK             = 1 shl 8
GMR_FS_GOOD_FC           = 1 shl 7
GMR_FS_BAD_FC            = 1 shl 6
GMR_FS_MII_ERR           = 1 shl 5
GMR_FS_LONG_ERR          = 1 shl 4
GMR_FS_FRAGMENT          = 1 shl 3
GMR_FS_CRC_ERR           = 1 shl 1
GMR_FS_RX_FF_OV          = 1 shl 0
GMR_FS_ANY_ERR           = GMR_FS_RX_FF_OV or GMR_FS_CRC_ERR or GMR_FS_FRAGMENT \
                            or GMR_FS_LONG_ERR or GMR_FS_MII_ERR or GMR_FS_BAD_FC

CSS_LINK_BIT             = 1 shl 0        ; port number, within a status entry's css byte

;----------------------------------------------------------------------
;  Queue block (Q_CSR/Q_WM/Q_AL/Q_TEST/Q_DONE) - stride B8_Q_REGS-based,
;  one block per queue (RX1/RX2/sync-TX1/sync-TX2/async-TX1/async-TX2).
;  We only ever use RX1 (port 0 receive) and async-TX1 (port 0 transmit).
;----------------------------------------------------------------------
B8_Q_REGS                = 0x0400
Q_R1                     = 0x0000         ; RX queue, port 0
Q_XS1                     = 0x0200        ; sync TX queue, port 0 (kept disabled)
Q_XA1                     = 0x0280        ; async TX queue, port 0 - the one we use

Q_DONE                   = 0x24           ; 16 bit - Done Index
Q_CSR                     = 0x34          ; 32 bit - BMU Control/Status
Q_TEST                   = 0x38           ; 32 bit - Test/Control
Q_WM                     = 0x40           ; 16 bit - FIFO Watermark
Q_AL                     = 0x42           ;  8 bit - FIFO Alignment

BMU_CLR_IRQ_PAR           = 1 shl 11
BMU_CLR_IRQ_CHK           = 1 shl 10
BMU_START                = 1 shl 8
BMU_FIFO_ENA              = 1 shl 5
BMU_FIFO_RST              = 1 shl 4
BMU_OP_ON                = 1 shl 3
BMU_OP_OFF               = 1 shl 2
BMU_FIFO_OP_ON            = 1 shl 7
BMU_RST_CLR              = 1 shl 1
BMU_CLR_RESET             = BMU_FIFO_RST or BMU_OP_OFF or BMU_RST_CLR
BMU_OPER_INIT             = BMU_CLR_IRQ_PAR or BMU_CLR_IRQ_CHK or BMU_START \
                            or BMU_FIFO_ENA or BMU_OP_ON
BMU_WM_DEFAULT            = 0x600

F_TX_CHK_AUTO_OFF         = 1 shl 31      ; Q_TEST: disable TX descriptor auto-check
F_TX_CHK_AUTO_ON          = 1 shl 30
F_M_RX_RAM_DIS            = 1 shl 24      ; Q_TEST bit: MAC Rx RAM read port
                                           ; disabled - required on EC-Ultra
                                           ; chip_rev > A0 (ours is rev 3), see
                                           ; Linux sky2_rx_start()
CHIP_REV_YU_EC_U_A0        = 1

;----------------------------------------------------------------------
;  Prefetch unit - the interface between the driver's LE ring and the
;  hardware. Separate register block from Q_CSR above despite similar
;  per-queue addressing (base + queue-offset + reg).
;----------------------------------------------------------------------
Y2_B8_PREF_REGS           = 0x0450

PREF_UNIT_CTRL            = 0x00          ; 32 bit
PREF_UNIT_LAST_IDX        = 0x04          ; 16 bit
PREF_UNIT_ADDR_LO         = 0x08          ; 32 bit
PREF_UNIT_ADDR_HI         = 0x0C          ; 32 bit
PREF_UNIT_GET_IDX         = 0x10          ; 16 bit
PREF_UNIT_PUT_IDX         = 0x14          ; 16 bit

PREF_UNIT_RST_SET         = 1 shl 0
PREF_UNIT_RST_CLR         = 1 shl 1
PREF_UNIT_OP_ON            = 1 shl 3

;----------------------------------------------------------------------
;  RAM buffer arbiter - per-queue FIFO sizing within the chip's internal
;  RAM. Sizes are in "qwords" (8-byte units) per the hardware register
;  definition; sky2_ramset-equivalent code below converts from KB.
;----------------------------------------------------------------------
B16_RAM_REGS              = 0x0800

RB_START                 = 0x00           ; 32 bit
RB_END                    = 0x04          ; 32 bit
RB_WP                    = 0x08           ; 32 bit
RB_RP                    = 0x0C           ; 32 bit
RB_RX_UTPP                = 0x10          ; 32 bit - Rx upper thresh, pause pkt
RB_RX_LTPP                = 0x14          ; 32 bit - Rx lower thresh, pause pkt
RB_RX_UTHP                = 0x18          ; 32 bit - Rx upper thresh, high prio
RB_RX_LTHP                = 0x1C          ; 32 bit - Rx lower thresh, high prio
RB_CTRL                   = 0x28          ; 32 bit

RB_ENA_STFWD              = 1 shl 5       ; enable store & forward (Tx queues)
RB_ENA_OP_MD              = 1 shl 3
RB_DIS_OP_MD              = 1 shl 2
RB_RST_CLR                = 1 shl 1
RB_RST_SET                = 1 shl 0

B2_E_0                    = 0x011C        ; 8 bit - internal RAM size, in 4K blocks

;----------------------------------------------------------------------
;  TX arbiter - per-port, SK_REG-style addressing (stride 0x80/port,
;  same block as GMAC_CTRL/GPHY_CTRL/GMAC_LINK_CTRL).
;----------------------------------------------------------------------
TXA_ITI_INI               = 0x0200        ; 32 bit
TXA_LIM_INI               = 0x0208        ; 32 bit
TXA_CTRL                  = 0x0210        ; 8 bit
TXA_ENA_FSYNC             = 1 shl 7
TXA_DIS_FSYNC             = 1 shl 6
TXA_ENA_ALLOC             = 1 shl 5
TXA_DIS_ALLOC             = 1 shl 4
TXA_START_RC              = 1 shl 3
TXA_STOP_RC               = 1 shl 2
TXA_ENA_ARB               = 1 shl 1
TXA_DIS_ARB               = 1 shl 0

;----------------------------------------------------------------------
;  Status/completion ring (card-wide, not per-port). The BMU writes
;  completion entries here as RX/TX work finishes; the driver polls
;  each entry's HW_OWNER bit rather than tracking a separate hardware
;  "put index" register, which keeps the interrupt handler simple and
;  is exactly the same safety check Linux does per-entry regardless.
;----------------------------------------------------------------------
STAT_FIFO_ISR_WM= 0x0ead        ;  8 bit	Status FIFO ISR Watermark Reg


STAT_LEV_TIMER_INI        = 0x0eb0        ; 32 bit	Level Timer Init. Value Reg
STAT_LEV_TIMER_CTRL       = 0x0eb8        ;  8 bit	Level Timer Control Reg
STAT_CTRL                 = 0x0E80        ; 32 bit
STAT_LAST_IDX             = 0x0E84        ; 16 bit
STAT_LIST_ADDR_LO         = 0x0E88        ; 32 bit
STAT_LIST_ADDR_HI         = 0x0E8C        ; 32 bit
STAT_TX_IDX_TH            = 0x0E98        ; 16 bit
STAT_PUT_IDX              = 0x0E9C        ; 16 bit
STAT_FIFO_WM              = 0x0EAC        ;  8 bit
STAT_TX_TIMER_INI         = 0x0ec0        ; 32 bit	Tx Timer Init. Value Reg
STAT_ISR_TIMER_CNT        = 0x0ed4        ; 32 bit	ISR Timer Counter Reg
STAT_ISR_TIMER_CTRL       = 0x0ed8        ;  8 bit	ISR Timer Control Reg
STAT_TX_TIMER_CTRL        = 0x0ec8        ;  8 bit	Tx Timer Control Reg
STAT_ISR_TIMER_INI        = 0x0ed0        ; 32 bit	ISR Timer Init. Value Reg

SC_STAT_OP_ON              = 1 shl 3
SC_STAT_RST_CLR            = 1 shl 1
SC_STAT_RST_SET            = 1 shl 0
SC_STAT_CLR_IRQ           = 1 shl 4

;----------------------------------------------------------------------
;  Ring sizing. Both counts must be powers of two (ring-wrap uses a
;  mask, not a divide). Kept modest and identical to what RTL8169/
;  i8254x already use in this tree.
;----------------------------------------------------------------------
TX_LE_SIZE                = 128                 ; must be a power of 2, between 128 and 2048
RX_LE_SIZE                = 128                 ; must be a power of 2, between 128 and 2048
ST_RING_SIZE              = 512                 ; comfortably more than TX+RX combined
RX_BUF_SIZE               = 1536                ; MTU 1500 + Ethernet header + slack

;----------------------------------------------------------------------
;  Generic 8-byte list-element layout, shared by TX/RX/status rings
;  (see the LE ring comment above for the cross-reference).
;----------------------------------------------------------------------
struct le
        addr            dd ?
        length          dw ?
        ctrl            db ?              ; "css" on a status-ring entry
        opcode          db ?
ends

;----------------------------------------------------------------------
;  Device structure (extends ETH_DEVICE)
;----------------------------------------------------------------------
struct device      ETH_DEVICE

        ; PCI / MMIO
        mmio_addr       dd ?
        pci_bus         dd ?
        pci_dev         dd ?
        irq_line        db ?
                        rb 3

        ; Chip identification
        chip_id         db ?              ; CHIP_ID_YUKON_*
        chip_rev        db ?
        port            db ?              ; 0 for single-port cards
                        rb 1

        chip_name       dd ?              ; pointer to ASCII name

        ; Link state, as last read from GM_GP_STAT (see link_status).
        ; device.state (in ETH_DEVICE, standard kernel-facing field) is
        ; kept in sync with these and is what NetLinkChanged reports.
        link_up         db ?              ; 0 = down, 1 = up
        link_duplex     db ?              ; 0 = half, 1 = full
        link_speed      dw ?              ; Mbps: 10 / 100 / 1000 (0 if link down)

        ; ---- TX/RX/status rings. Embedded directly in the device
        ; struct, same pattern RTL8169 uses in this tree: the struct
        ; itself is allocated via allocate_and_clear, which guarantees
        ; physical contiguity up to 8 pages (32K) - our rings total
        ; well under 2K, so this is safe. Raw byte reservation (not a
        ; typed struct array) matching RTL8169's own tx_ring/rx_ring
        ; convention - this struct.inc doesn't support array-of-custom-
        ; struct fields, only single-instance nesting; per-entry access
        ; is done via sizeof.le * slot_index offset math instead.

        rb 0x1000-($ and 0xfff)   ; align 256
        tx_le           rb TX_LE_SIZE * sizeof.le

        rb 0x1000-($ and 0xfff)   ; align 256
        rx_le           rb RX_LE_SIZE * sizeof.le

        rb 0x1000-($ and 0xfff)   ; align 256
        st_le           rb ST_RING_SIZE * sizeof.le

        ; Software-only bookkeeping, parallel to the rings above but
        ; NOT hardware-visible - just our own buffer-pointer tracking.
        ; (NetAlloc'd pointer per slot; 0 = slot has no owned buffer.)
        tx_buf          rd TX_LE_SIZE
        rx_buf          rd RX_LE_SIZE

        ; Ring indices. All are plain 0-based slot numbers, wrapped
        ; with a power-of-two mask (SIZE-1), not divided.
        tx_prod         dw ?              ; next free TX slot to fill
        tx_cons         dw ?              ; oldest TX slot still in flight
        rx_put          dw ?              ; next RX slot to (re)submit
        rx_next          dw ?              ; oldest RX slot awaiting completion
        st_idx          dw ?              ; next status-ring slot to read

ends



RX_GMF_CTRL_T = 0x0C48
TX_GMF_CTRL_T = 0x0D48
RX_GMF_LP_THR = 0x0C4A          ; 8 bit  Rx GMAC FIFO low pause threshold
RX_GMF_UP_THR = 0x0C4B          ; 8 bit  Rx GMAC FIFO upper pause threshold
GMF_RST_SET     = 1 shl 0
GMF_RST_CLR     = 1 shl 1
GMF_OPER_ON     = 1 shl 3
GMF_RX_F_FL_ON  = 1 shl 7
; TX GMF high bits (Yukon-EC Ultra) - store & forward required when no RAM
TX_STFW_DIS     = 1 shl 31
TX_STFW_ENA     = 1 shl 30
TX_JUMBO_ENA    = 1 shl 23
TX_JUMBO_DIS    = 1 shl 22

; RAM Interface (B3_*) - initialized during reset even when there is no
; external RAM. Still required for the internal FIFO/GMF data path.
B3_RI_WTO_R1    = 0x0190
B3_RI_WTO_XA1   = 0x0191
B3_RI_WTO_XS1   = 0x0192
B3_RI_RTO_R1    = 0x0193
B3_RI_RTO_XA1   = 0x0194
B3_RI_RTO_XS1   = 0x0195
B3_RI_WTO_R2    = 0x0196
B3_RI_WTO_XA2   = 0x0197
B3_RI_WTO_XS2   = 0x0198
B3_RI_RTO_R2    = 0x0199
B3_RI_RTO_XA2   = 0x019a
B3_RI_RTO_XS2   = 0x019b
B3_RI_CTRL      = 0x01a0
RI_RST_CLR      = 1 shl 1
RI_RST_SET      = 1 shl 0
SK_RI_TO_53     = 36            ; Linux/OpenBSD default timeout value

; GM_GP_CTRL extra bits (speed/duplex force)
GM_GPCR_GIGS_ENA        = 1 shl 7
GM_GPCR_DUP_FULL        = 1 shl 5
GM_GPCR_SPEED_100       = 1 shl 3
GM_GPCR_SPEED_1000      = (GM_GPCR_GIGS_ENA or GM_GPCR_SPEED_100)
GM_GPCR_AU_DUP_DIS      = 1 shl 2
GM_GPCR_AU_FCT_DIS      = 1 shl 1
GM_GPCR_AU_SPD_DIS      = 1 shl 0

TIM_START	= 1 shl 2 ;	; Start Timer
TIM_STOP	= 1 shl 1 ; 	; Stop  Timer
TIM_CLR_IRQ	= 1 shl 0 ;	; Clear Timer IRQ (!IRQM)



                  GM_SRC_ADDR_1L	= 0x001c;,	; 16 bit r/w	Source Address 1 (low)
                  GM_SRC_ADDR_1M	= 0x0020;,	; 16 bit r/w	Source Address 1 (middle)
                  GM_SRC_ADDR_1H	= 0x0024;,	; 16 bit r/w	Source Address 1 (high)
                  GM_SRC_ADDR_2L	= 0x0028;,	; 16 bit r/w	Source Address 2 (low)
                  GM_SRC_ADDR_2M	= 0x002c;,	; 16 bit r/w	Source Address 2 (middle)
                  GM_SRC_ADDR_2H	= 0x0030;,	; 16 bit r/w	Source Address 2 (high)



                  Y2_IS_HW_ERR	= 1 shl 31	; Interrupt HW Error
                  Y2_IS_STAT_BMU	= 1 shl 30	; Status BMU Interrupt
                  Y2_IS_ASF	= 1 shl 29	; ASF subsystem Interrupt
                  Y2_IS_CPU_TO	= 1 shl 28	; CPU Timeout
                  Y2_IS_POLL_CHK	= 1 shl 27	; Check IRQ from polling unit
                  Y2_IS_TWSI_RDY	= 1 shl 26	; IRQ on end of TWSI Tx
                  Y2_IS_IRQ_SW	= 1 shl 25	; SW forced IRQ
                  Y2_IS_TIMINT	= 1 shl 24	; IRQ from Timer

                  Y2_IS_IRQ_PHY2	= 1 shl 12	; Interrupt from PHY 2
                  Y2_IS_IRQ_MAC2	= 1 shl 11	; Interrupt from MAC 2
                  Y2_IS_CHK_RX2	= 1 shl 10	; Descriptor error Rx 2
                  Y2_IS_CHK_TXS2	= 1 shl 9		; Descriptor error TXS 2
                  Y2_IS_CHK_TXA2	= 1 shl 8		; Descriptor error TXA 2

                  Y2_IS_PSM_ACK	= 1 shl 7		; PSM Acknowledge (Yukon-Optima only)
                  Y2_IS_PTP_TIST	= 1 shl 6		; PTP Time Stamp (Yukon-Optima only)
                  Y2_IS_PHY_QLNK	= 1 shl 5		; PHY Quick Link (Yukon-Optima only)

                  Y2_IS_IRQ_PHY1	= 1 shl 4		; Interrupt from PHY 1
                  Y2_IS_IRQ_MAC1	= 1 shl 3		; Interrupt from MAC 1
                  Y2_IS_CHK_RX1	= 1 shl 2		; Descriptor error Rx 1
                  Y2_IS_CHK_TXS1	= 1 shl 1		; Descriptor error TXS 1
                  Y2_IS_CHK_TXA1	= 1 shl 0		; Descriptor error TXA 1
