ALX_VEN_ID                                      = 0x1969

ALX_DEV_ID_AR8131                               = 0x1063
ALX_DEV_ID_AR8132                               = 0x1062
ALX_DEV_ID_AR8151_1                             = 0x1073
ALX_DEV_ID_AR8151_2                             = 0x1083
ALX_DEV_ID_AR8152_1                             = 0x2060
ALX_DEV_ID_AR8152_2                             = 0x2062
ALX_DEV_ID_AR8161                               = 0x1091
ALX_DEV_ID_E2200                                = 0xe091
ALX_DEV_ID_E2400                                = 0xe0a1
ALX_DEV_ID_E2500                                = 0xe0b1
ALX_DEV_ID_AR8162                               = 0x1090
ALX_DEV_ID_AR8171                               = 0x10A1
ALX_DEV_ID_AR8172                               = 0x10A0

ALX_PCI_REVID_WITH_XD                           = 1 shl 0
ALX_PCI_REVID_WITH_CR                           = 1 shl 1       ; With Card Reader
ALX_PCI_REVID_SHIFT                             = 3
ALX_REV_A0                                      = 0
ALX_REV_A1                                      = 1
ALX_REV_B0                                      = 2
ALX_REV_C0                                      = 3

ALX_DEV_CTRL                                    = 0x0060
ALX_DEV_CTRL_MAXRRS_MIN                         = 2

ALX_MSIX_MASK                                   = 0x0090

ALX_UE_SVRT                                     = 0x010C
ALX_UE_SVRT_FCPROTERR                           = (1 shl 13)
ALX_UE_SVRT_DLPROTERR                           = (1 shl 4)

; eeprom & flash load register
ALX_EFLD                                        = 0x0204
ALX_EFLD_F_EXIST                                = (1 shl 10)
ALX_EFLD_E_EXIST                                = (1 shl 9)
ALX_EFLD_STAT                                   = (1 shl 5)
ALX_EFLD_START                                  = (1 shl 0)

; eFuse load register
ALX_SLD                                         = 0x0218
ALX_SLD_STAT                                    = (1 shl 12)
ALX_SLD_START                                   = (1 shl 11)
ALX_SLD_MAX_TO                                  = 100

ALX_PDLL_TRNS1                                  = 0x1104
ALX_PDLL_TRNS1_D3PLLOFF_EN                      = (1 shl 11)

ALX_PMCTRL                                      = 0x12F8
ALX_PMCTRL_HOTRST_WTEN                          = (1 shl 31)
; bit30: L0s/L1 controlled by MAC based on throughput(setting in 15A0)
ALX_PMCTRL_ASPM_FCEN                            = (1 shl 30)
ALX_PMCTRL_SADLY_EN                             = (1 shl 29)
ALX_PMCTRL_LCKDET_TIMER_MASK                    = 0xF
ALX_PMCTRL_LCKDET_TIMER_SHIFT                   = 24
ALX_PMCTRL_LCKDET_TIMER_DEF                     = 0xC
; bit[23:20] if pm_request_l1 time > @, then enter L0s not L1
ALX_PMCTRL_L1REQ_TO_MASK                        = 0xF
ALX_PMCTRL_L1REQ_TO_SHIFT                       = 20
ALX_PMCTRL_L1REG_TO_DEF                         = 0xF
ALX_PMCTRL_TXL1_AFTER_L0S                       = (1 shl 19)
ALX_PMCTRL_L1_TIMER_MASK                        = 0x7
ALX_PMCTRL_L1_TIMER_SHIFT                       = 16
ALX_PMCTRL_L1_TIMER_16US                        = 4
ALX_PMCTRL_RCVR_WT_1US                          = (1 shl 15)
; bit13: enable pcie clk switch in L1 state
ALX_PMCTRL_L1_CLKSW_EN                          = (1 shl 13)
ALX_PMCTRL_L0S_EN                               = (1 shl 12)
ALX_PMCTRL_RXL1_AFTER_L0S                       = (1 shl 11)
ALX_PMCTRL_L1_BUFSRX_EN                         = (1 shl 7)
; bit6: power down serdes RX
ALX_PMCTRL_L1_SRDSRX_PWD                        = (1 shl 6)
ALX_PMCTRL_L1_SRDSPLL_EN                        = (1 shl 5)
ALX_PMCTRL_L1_SRDS_EN                           = (1 shl 4)
ALX_PMCTRL_L1_EN                                = (1 shl 3)

;******************************************************
; following registers are mapped only to memory space
;******************************************************

ALX_MASTER                                      = 0x1400
; bit12: 1:alwys select pclk from serdes, not sw to 25M
ALX_MASTER_PCLKSEL_SRDS                         = (1 shl 12)
; bit11: irq moduration for rx
ALX_MASTER_IRQMOD2_EN                           = (1 shl 11)
; bit10: irq moduration for tx/rx
ALX_MASTER_IRQMOD1_EN                           = (1 shl 10)
ALX_MASTER_SYSALVTIMER_EN                       = (1 shl 7)
ALX_MASTER_OOB_DIS                              = (1 shl 6)
; bit5: wakeup without pcie clk
ALX_MASTER_WAKEN_25M                            = (1 shl 5)
; bit0: MAC & DMA reset
ALX_MASTER_DMA_MAC_RST                          = (1 shl 0)
ALX_DMA_MAC_RST_TO                              = 50

ALX_IRQ_MODU_TIMER                              = 0x1408
ALX_IRQ_MODU_TIMER1_MASK                        = 0x0000FFFF
ALX_IRQ_MODU_TIMER1_SHIFT                       = 0
ALX_IRQ_MODU_TIMER2_MASK                        = 0xFFFF0000
ALX_IRQ_MODU_TIMER2_SHIFT                       = 16

ALX_PHY_CTRL                                    = 0x140C
ALX_PHY_CTRL_100AB_EN                           = (1 shl 17)
; bit14: affect MAC & PHY, go to low power sts
ALX_PHY_CTRL_POWER_DOWN                         = (1 shl 14)
; bit13: 1:pll always ON, 0:can switch in lpw
ALX_PHY_CTRL_PLL_ON                             = (1 shl 13)
ALX_PHY_CTRL_RST_ANALOG                         = (1 shl 12)
ALX_PHY_CTRL_HIB_PULSE                          = (1 shl 11)
ALX_PHY_CTRL_HIB_EN                             = (1 shl 10)
ALX_PHY_CTRL_IDDQ                               = (1 shl 7)
ALX_PHY_CTRL_GATE_25M                           = (1 shl 5)
ALX_PHY_CTRL_LED_MODE                           = (1 shl 2)
; bit0: out of dsp RST state
ALX_PHY_CTRL_DSPRST_OUT                         = (1 shl 0)
ALX_PHY_CTRL_DSPRST_TO                          = 80
ALX_PHY_CTRL_CLS                                = (ALX_PHY_CTRL_LED_MODE or ALX_PHY_CTRL_100AB_EN or ALX_PHY_CTRL_PLL_ON)

ALX_MAC_STS                                     = 0x1410
ALX_MAC_STS_TXQ_BUSY                            = (1 shl 3)
ALX_MAC_STS_RXQ_BUSY                            = (1 shl 2)
ALX_MAC_STS_TXMAC_BUSY                          = (1 shl 1)
ALX_MAC_STS_RXMAC_BUSY                          = (1 shl 0)
ALX_MAC_STS_IDLE                                = (ALX_MAC_STS_TXQ_BUSY or ALX_MAC_STS_RXQ_BUSY or ALX_MAC_STS_TXMAC_BUSY or ALX_MAC_STS_RXMAC_BUSY)

ALX_MDIO                                        = 0x1414
ALX_MDIO_MODE_EXT                               = (1 shl 30)
ALX_MDIO_BUSY                                   = (1 shl 27)
ALX_MDIO_CLK_SEL_MASK                           = 0x7
ALX_MDIO_CLK_SEL_SHIFT                          = 24
ALX_MDIO_CLK_SEL_25MD4                          = 0
ALX_MDIO_CLK_SEL_25MD128                        = 7
ALX_MDIO_START                                  = (1 shl 23)
ALX_MDIO_SPRES_PRMBL                            = (1 shl 22)
; bit21: 1:read,0:write
ALX_MDIO_OP_READ                                = (1 shl 21)
ALX_MDIO_REG_MASK                               = 0x1F
ALX_MDIO_REG_SHIFT                              = 16
ALX_MDIO_DATA_MASK                              = 0xFFFF
ALX_MDIO_DATA_SHIFT                             = 0
ALX_MDIO_MAX_AC_TO                              = 120

ALX_MDIO_EXTN                                   = 0x1448
ALX_MDIO_EXTN_DEVAD_MASK                        = 0x1F
ALX_MDIO_EXTN_DEVAD_SHIFT                       = 16
ALX_MDIO_EXTN_REG_MASK                          = 0xFFFF
ALX_MDIO_EXTN_REG_SHIFT                         = 0

ALX_SERDES                                      = 0x1424
ALX_SERDES_PHYCLK_SLWDWN                        = (1 shl 18)
ALX_SERDES_MACCLK_SLWDWN                        = (1 shl 17)

ALX_LPI_CTRL                                    = 0x1440
ALX_LPI_CTRL_EN                                 = (1 shl 0)

; for B0+, bit[13..] for C0+
ALX_HRTBT_EXT_CTRL                              = 0x1AD0
L1F_HRTBT_EXT_CTRL_PERIOD_HIGH_MASK             = 0x3F
L1F_HRTBT_EXT_CTRL_PERIOD_HIGH_SHIFT            = 24
L1F_HRTBT_EXT_CTRL_SWOI_STARTUP_PKT_EN          = (1 shl 23)
L1F_HRTBT_EXT_CTRL_IOAC_2_FRAGMENTED            = (1 shl 22)
L1F_HRTBT_EXT_CTRL_IOAC_1_FRAGMENTED            = (1 shl 21)
L1F_HRTBT_EXT_CTRL_IOAC_1_KEEPALIVE_EN          = (1 shl 20)
L1F_HRTBT_EXT_CTRL_IOAC_1_HAS_VLAN              = (1 shl 19)
L1F_HRTBT_EXT_CTRL_IOAC_1_IS_8023               = (1 shl 18)
L1F_HRTBT_EXT_CTRL_IOAC_1_IS_IPV6               = (1 shl 17)
L1F_HRTBT_EXT_CTRL_IOAC_2_KEEPALIVE_EN          = (1 shl 16)
L1F_HRTBT_EXT_CTRL_IOAC_2_HAS_VLAN              = (1 shl 15)
L1F_HRTBT_EXT_CTRL_IOAC_2_IS_8023               = (1 shl 14)
L1F_HRTBT_EXT_CTRL_IOAC_2_IS_IPV6               = (1 shl 13)
ALX_HRTBT_EXT_CTRL_NS_EN                        = (1 shl 12)
ALX_HRTBT_EXT_CTRL_FRAG_LEN_MASK                = 0xFF
ALX_HRTBT_EXT_CTRL_FRAG_LEN_SHIFT               = 4
ALX_HRTBT_EXT_CTRL_IS_8023                      = (1 shl 3)
ALX_HRTBT_EXT_CTRL_IS_IPV6                      = (1 shl 2)
ALX_HRTBT_EXT_CTRL_WAKEUP_EN                    = (1 shl 1)
ALX_HRTBT_EXT_CTRL_ARP_EN                       = (1 shl 0)

ALX_HRTBT_REM_IPV4_ADDR                         = 0x1AD4
ALX_HRTBT_HOST_IPV4_ADDR                        = 0x1478
ALX_HRTBT_REM_IPV6_ADDR3                        = 0x1AD8
ALX_HRTBT_REM_IPV6_ADDR2                        = 0x1ADC
ALX_HRTBT_REM_IPV6_ADDR1                        = 0x1AE0
ALX_HRTBT_REM_IPV6_ADDR0                        = 0x1AE4

; 1B8C ~ 1B94 for C0+
ALX_SWOI_ACER_CTRL                              = 0x1B8C
ALX_SWOI_ORIG_ACK_NAK_EN                        = (1 shl 20)
ALX_SWOI_ORIG_ACK_NAK_PKT_LEN_MASK              = 0xFF
ALX_SWOI_ORIG_ACK_NAK_PKT_LEN_SHIFT             = 12
ALX_SWOI_ORIG_ACK_ADDR_MASK                     = 0xFFF
ALX_SWOI_ORIG_ACK_ADDR_SHIFT                    = 0

ALX_SWOI_IOAC_CTRL_2                            = 0x1B90
ALX_SWOI_IOAC_CTRL_2_SWOI_1_FRAG_LEN_MASK       = 0xFF
ALX_SWOI_IOAC_CTRL_2_SWOI_1_FRAG_LEN_SHIFT      = 24
ALX_SWOI_IOAC_CTRL_2_SWOI_1_PKT_LEN_MASK        = 0xFFF
ALX_SWOI_IOAC_CTRL_2_SWOI_1_PKT_LEN_SHIFT       = 12
ALX_SWOI_IOAC_CTRL_2_SWOI_1_HDR_ADDR_MASK       = 0xFFF
ALX_SWOI_IOAC_CTRL_2_SWOI_1_HDR_ADDR_SHIFT      = 0

ALX_SWOI_IOAC_CTRL_3                            = 0x1B94
ALX_SWOI_IOAC_CTRL_3_SWOI_2_FRAG_LEN_MASK       = 0xFF
ALX_SWOI_IOAC_CTRL_3_SWOI_2_FRAG_LEN_SHIFT      = 24
ALX_SWOI_IOAC_CTRL_3_SWOI_2_PKT_LEN_MASK        = 0xFFF
ALX_SWOI_IOAC_CTRL_3_SWOI_2_PKT_LEN_SHIFT       = 12
ALX_SWOI_IOAC_CTRL_3_SWOI_2_HDR_ADDR_MASK       = 0xFFF
ALX_SWOI_IOAC_CTRL_3_SWOI_2_HDR_ADDR_SHIFT      = 0

; for B0
ALX_IDLE_DECISN_TIMER                           = 0x1474
; 1ms
ALX_IDLE_DECISN_TIMER_DEF                       = 0x400

ALX_MAC_CTRL                                    = 0x1480
ALX_MAC_CTRL_FAST_PAUSE                         = (1 shl 31)
ALX_MAC_CTRL_WOLSPED_SWEN                       = (1 shl 30)
; bit29: 1:legacy(hi5b), 0:marvl(lo5b)
ALX_MAC_CTRL_MHASH_ALG_HI5B                     = (1 shl 29)
ALX_MAC_CTRL_BRD_EN                             = (1 shl 26)
ALX_MAC_CTRL_MULTIALL_EN                        = (1 shl 25)
ALX_MAC_CTRL_SPEED_MASK                         = 0x3
ALX_MAC_CTRL_SPEED_SHIFT                        = 20
ALX_MAC_CTRL_SPEED_10_100                       = 1
ALX_MAC_CTRL_SPEED_1000                         = 2
ALX_MAC_CTRL_PROMISC_EN                         = (1 shl 15)
ALX_MAC_CTRL_VLANSTRIP                          = (1 shl 14)
ALX_MAC_CTRL_PRMBLEN_MASK                       = 0xF
ALX_MAC_CTRL_PRMBLEN_SHIFT                      = 10
ALX_MAC_CTRL_PCRCE                              = (1 shl 7)
ALX_MAC_CTRL_CRCE                               = (1 shl 6)
ALX_MAC_CTRL_FULLD                              = (1 shl 5)
ALX_MAC_CTRL_RXFC_EN                            = (1 shl 3)
ALX_MAC_CTRL_TXFC_EN                            = (1 shl 2)
ALX_MAC_CTRL_RX_EN                              = (1 shl 1)
ALX_MAC_CTRL_TX_EN                              = (1 shl 0)

ALX_STAD0                                       = 0x1488
ALX_STAD1                                       = 0x148C

ALX_HASH_TBL0                                   = 0x1490
ALX_HASH_TBL1                                   = 0x1494

ALX_MTU                                         = 0x149C
ALX_MTU_JUMBO_TH                                = 1514
ALX_MTU_STD_ALGN                                = 1536

ALX_SRAM5                                       = 0x1524
ALX_SRAM_RXF_LEN_MASK                           = 0xFFF
ALX_SRAM_RXF_LEN_SHIFT                          = 0
ALX_SRAM_RXF_LEN_8K                             = (8*1024)

ALX_SRAM9                                       = 0x1534
ALX_SRAM_LOAD_PTR                               = (1 shl 0)

ALX_RX_BASE_ADDR_HI                             = 0x1540

ALX_TX_BASE_ADDR_HI                             = 0x1544

ALX_RFD_ADDR_LO                                 = 0x1550
ALX_RFD_RING_SZ                                 = 0x1560
ALX_RFD_BUF_SZ                                  = 0x1564

ALX_RRD_ADDR_LO                                 = 0x1568
ALX_RRD_RING_SZ                                 = 0x1578

; pri3: highest, pri0: lowest
ALX_TPD_PRI3_ADDR_LO                            = 0x14E4
ALX_TPD_PRI2_ADDR_LO                            = 0x14E0
ALX_TPD_PRI1_ADDR_LO                            = 0x157C
ALX_TPD_PRI0_ADDR_LO                            = 0x1580

; producer index is 16bit
ALX_TPD_PRI3_PIDX                               = 0x1618
ALX_TPD_PRI2_PIDX                               = 0x161A
ALX_TPD_PRI1_PIDX                               = 0x15F0
ALX_TPD_PRI0_PIDX                               = 0x15F2

; consumer index is 16bit
ALX_TPD_PRI3_CIDX                               = 0x161C
ALX_TPD_PRI2_CIDX                               = 0x161E
ALX_TPD_PRI1_CIDX                               = 0x15F4
ALX_TPD_PRI0_CIDX                               = 0x15F6

ALX_TPD_RING_SZ                                 = 0x1584

ALX_TXQ0                                        = 0x1590
ALX_TXQ0_TXF_BURST_PREF_MASK                    = 0xFFFF
ALX_TXQ0_TXF_BURST_PREF_SHIFT                   = 16
ALX_TXQ_TXF_BURST_PREF_DEF                      = 0x200
ALX_TXQ0_LSO_8023_EN                            = (1 shl 7)
ALX_TXQ0_MODE_ENHANCE                           = (1 shl 6)
ALX_TXQ0_EN                                     = (1 shl 5)
ALX_TXQ0_SUPT_IPOPT                             = (1 shl 4)
ALX_TXQ0_TPD_BURSTPREF_MASK                     = 0xF
ALX_TXQ0_TPD_BURSTPREF_SHIFT                    = 0
ALX_TXQ_TPD_BURSTPREF_DEF                       = 5

ALX_TXQ1                                        = 0x1594
; bit11:  drop large packet, len > (rfd buf)
ALX_TXQ1_ERRLGPKT_DROP_EN                       = (1 shl 11)
ALX_TXQ1_JUMBO_TSO_TH                           = (7*1024)

ALX_RXQ0                                        = 0x15A0
ALX_RXQ0_EN                                     = (1 shl 31)
ALX_RXQ0_RSS_HASH_EN                            = (1 shl 29)
ALX_RXQ0_RSS_MODE_MASK                          = 0x3
ALX_RXQ0_RSS_MODE_SHIFT                         = 26
ALX_RXQ0_RSS_MODE_DIS                           = 0
ALX_RXQ0_RSS_MODE_MQMI                          = 3
ALX_RXQ0_NUM_RFD_PREF_MASK                      = 0x3F
ALX_RXQ0_NUM_RFD_PREF_SHIFT                     = 20
ALX_RXQ0_NUM_RFD_PREF_DEF                       = 8
ALX_RXQ0_IDT_TBL_SIZE_MASK                      = 0x1FF
ALX_RXQ0_IDT_TBL_SIZE_SHIFT                     = 8
ALX_RXQ0_IDT_TBL_SIZE_DEF                       = 0x100
ALX_RXQ0_IDT_TBL_SIZE_NORMAL                    = 128
ALX_RXQ0_IPV6_PARSE_EN                          = (1 shl 7)
ALX_RXQ0_RSS_HSTYP_MASK                         = 0xF
ALX_RXQ0_RSS_HSTYP_SHIFT                        = 2
ALX_RXQ0_RSS_HSTYP_IPV6_TCP_EN                  = (1 shl 5)
ALX_RXQ0_RSS_HSTYP_IPV6_EN                      = (1 shl 4)
ALX_RXQ0_RSS_HSTYP_IPV4_TCP_EN                  = (1 shl 3)
ALX_RXQ0_RSS_HSTYP_IPV4_EN                      = (1 shl 2)
ALX_RXQ0_RSS_HSTYP_ALL                          = (ALX_RXQ0_RSS_HSTYP_IPV6_TCP_EN or ALX_RXQ0_RSS_HSTYP_IPV4_TCP_EN or ALX_RXQ0_RSS_HSTYP_IPV6_EN or ALX_RXQ0_RSS_HSTYP_IPV4_EN)
ALX_RXQ0_ASPM_THRESH_MASK                       = 0x3
ALX_RXQ0_ASPM_THRESH_SHIFT                      = 0
ALX_RXQ0_ASPM_THRESH_100M                       = 3

ALX_RXQ2                                        = 0x15A8
ALX_RXQ2_RXF_XOFF_THRESH_MASK                   = 0xFFF
ALX_RXQ2_RXF_XOFF_THRESH_SHIFT                  = 16
ALX_RXQ2_RXF_XON_THRESH_MASK                    = 0xFFF
ALX_RXQ2_RXF_XON_THRESH_SHIFT                   = 0
; Size = tx-packet(1522) + IPG(12) + SOF(8) + 64(Pause) + IPG(12) + SOF(8) +
;        rx-packet(1522) + delay-of-link(64)
;      = 3212.

ALX_RXQ2_RXF_FLOW_CTRL_RSVD                     = 3212

ALX_DMA                                         = 0x15C0
ALX_DMA_RCHNL_SEL_MASK                          = 0x3
ALX_DMA_RCHNL_SEL_SHIFT                         = 26
ALX_DMA_WDLY_CNT_MASK                           = 0xF
ALX_DMA_WDLY_CNT_SHIFT                          = 16
ALX_DMA_WDLY_CNT_DEF                            = 4
ALX_DMA_RDLY_CNT_MASK                           = 0x1F
ALX_DMA_RDLY_CNT_SHIFT                          = 11
ALX_DMA_RDLY_CNT_DEF                            = 15
; bit10: 0:tpd with pri, 1: data
ALX_DMA_RREQ_PRI_DATA                           = (1 shl 10)
ALX_DMA_RREQ_BLEN_MASK                          = 0x7
ALX_DMA_RREQ_BLEN_SHIFT                         = 4
ALX_DMA_RORDER_MODE_MASK                        = 0x7
ALX_DMA_RORDER_MODE_SHIFT                       = 0
ALX_DMA_RORDER_MODE_OUT                         = 4

ALX_WOL0                                        = 0x14A0
ALX_WOL0_PME_LINK                               = (1 shl 5)
ALX_WOL0_LINK_EN                                = (1 shl 4)
ALX_WOL0_PME_MAGIC_EN                           = (1 shl 3)
ALX_WOL0_MAGIC_EN                               = (1 shl 2)

; RFD Producer index
ALX_RFD_PIDX                                    = 0x15E0

; RFD Consumer indef
ALX_RFD_CIDX                                    = 0x15F8

; MIB
ALX_MIB_BASE                                    = 0x1700

ALX_MIB_RX_OK                                   = (ALX_MIB_BASE + 0)
ALX_MIB_RX_BCAST                                = (ALX_MIB_BASE + 4)
ALX_MIB_RX_MCAST                                = (ALX_MIB_BASE + 8)
ALX_MIB_RX_PAUSE                                = (ALX_MIB_BASE + 12)
ALX_MIB_RX_CTRL                                 = (ALX_MIB_BASE + 16)
ALX_MIB_RX_FCS_ERR                              = (ALX_MIB_BASE + 20)
ALX_MIB_RX_LEN_ERR                              = (ALX_MIB_BASE + 24)
ALX_MIB_RX_BYTE_CNT                             = (ALX_MIB_BASE + 28)
ALX_MIB_RX_RUNT                                 = (ALX_MIB_BASE + 32)
ALX_MIB_RX_FRAG                                 = (ALX_MIB_BASE + 36)
ALX_MIB_RX_SZ_64B                               = (ALX_MIB_BASE + 40)
ALX_MIB_RX_SZ_127B                              = (ALX_MIB_BASE + 44)
ALX_MIB_RX_SZ_255B                              = (ALX_MIB_BASE + 48)
ALX_MIB_RX_SZ_511B                              = (ALX_MIB_BASE + 52)
ALX_MIB_RX_SZ_1023B                             = (ALX_MIB_BASE + 56)
ALX_MIB_RX_SZ_1518B                             = (ALX_MIB_BASE + 60)
ALX_MIB_RX_SZ_MAX                               = (ALX_MIB_BASE + 64)
ALX_MIB_RX_OV_SZ                                = (ALX_MIB_BASE + 68)
ALX_MIB_RX_OV_RXF                               = (ALX_MIB_BASE + 72)
ALX_MIB_RX_OV_RRD                               = (ALX_MIB_BASE + 76)
ALX_MIB_RX_ALIGN_ERR                            = (ALX_MIB_BASE + 80)
ALX_MIB_RX_BCCNT                                = (ALX_MIB_BASE + 84)
ALX_MIB_RX_MCCNT                                = (ALX_MIB_BASE + 88)
ALX_MIB_RX_ERRADDR                              = (ALX_MIB_BASE + 92)

ALX_MIB_TX_OK                                   = (ALX_MIB_BASE + 96)
ALX_MIB_TX_BCAST                                = (ALX_MIB_BASE + 100)
ALX_MIB_TX_MCAST                                = (ALX_MIB_BASE + 104)
ALX_MIB_TX_PAUSE                                = (ALX_MIB_BASE + 108)
ALX_MIB_TX_EXC_DEFER                            = (ALX_MIB_BASE + 112)
ALX_MIB_TX_CTRL                                 = (ALX_MIB_BASE + 116)
ALX_MIB_TX_DEFER                                = (ALX_MIB_BASE + 120)
ALX_MIB_TX_BYTE_CNT                             = (ALX_MIB_BASE + 124)
ALX_MIB_TX_SZ_64B                               = (ALX_MIB_BASE + 128)
ALX_MIB_TX_SZ_127B                              = (ALX_MIB_BASE + 132)
ALX_MIB_TX_SZ_255B                              = (ALX_MIB_BASE + 136)
ALX_MIB_TX_SZ_511B                              = (ALX_MIB_BASE + 140)
ALX_MIB_TX_SZ_1023B                             = (ALX_MIB_BASE + 144)
ALX_MIB_TX_SZ_1518B                             = (ALX_MIB_BASE + 148)
ALX_MIB_TX_SZ_MAX                               = (ALX_MIB_BASE + 152)
ALX_MIB_TX_SINGLE_COL                           = (ALX_MIB_BASE + 156)
ALX_MIB_TX_MULTI_COL                            = (ALX_MIB_BASE + 160)
ALX_MIB_TX_LATE_COL                             = (ALX_MIB_BASE + 164)
ALX_MIB_TX_ABORT_COL                            = (ALX_MIB_BASE + 168)
ALX_MIB_TX_UNDERRUN                             = (ALX_MIB_BASE + 172)
ALX_MIB_TX_TRD_EOP                              = (ALX_MIB_BASE + 176)
ALX_MIB_TX_LEN_ERR                              = (ALX_MIB_BASE + 180)
ALX_MIB_TX_TRUNC                                = (ALX_MIB_BASE + 184)
ALX_MIB_TX_BCCNT                                = (ALX_MIB_BASE + 188)
ALX_MIB_TX_MCCNT                                = (ALX_MIB_BASE + 192)
ALX_MIB_UPDATE                                  = (ALX_MIB_BASE + 196)


ALX_ISR                                         = 0x1600
ALX_ISR_DIS                                     = (1 shl 31)
ALX_ISR_RX_Q7                                   = (1 shl 30)
ALX_ISR_RX_Q6                                   = (1 shl 29)
ALX_ISR_RX_Q5                                   = (1 shl 28)
ALX_ISR_RX_Q4                                   = (1 shl 27)
ALX_ISR_PCIE_LNKDOWN                            = (1 shl 26)
ALX_ISR_RX_Q3                                   = (1 shl 19)
ALX_ISR_RX_Q2                                   = (1 shl 18)
ALX_ISR_RX_Q1                                   = (1 shl 17)
ALX_ISR_RX_Q0                                   = (1 shl 16)
ALX_ISR_TX_Q0                                   = (1 shl 15)
ALX_ISR_PHY                                     = (1 shl 12)
ALX_ISR_DMAW                                    = (1 shl 10)
ALX_ISR_DMAR                                    = (1 shl 9)
ALX_ISR_TXF_UR                                  = (1 shl 8)
ALX_ISR_TX_Q3                                   = (1 shl 7)
ALX_ISR_TX_Q2                                   = (1 shl 6)
ALX_ISR_TX_Q1                                   = (1 shl 5)
ALX_ISR_RFD_UR                                  = (1 shl 4)
ALX_ISR_RXF_OV                                  = (1 shl 3)
ALX_ISR_MANU                                    = (1 shl 2)
ALX_ISR_TIMER                                   = (1 shl 1)
ALX_ISR_SMB                                     = (1 shl 0)

ALX_IMR                                         = 0x1604

; re-send assert msg if SW no response
ALX_INT_RETRIG                                  = 0x1608
; 40ms
ALX_INT_RETRIG_TO                               = 20000

ALX_SMB_TIMER                                   = 0x15C4

ALX_TINT_TPD_THRSHLD                            = 0x15C8

ALX_TINT_TIMER                                  = 0x15CC

ALX_CLK_GATE                                    = 0x1814
ALX_CLK_GATE_RXMAC                              = (1 shl 5)
ALX_CLK_GATE_TXMAC                              = (1 shl 4)
ALX_CLK_GATE_RXQ                                = (1 shl 3)
ALX_CLK_GATE_TXQ                                = (1 shl 2)
ALX_CLK_GATE_DMAR                               = (1 shl 1)
ALX_CLK_GATE_DMAW                               = (1 shl 0)
ALX_CLK_GATE_ALL                                = (ALX_CLK_GATE_RXMAC or ALX_CLK_GATE_TXMAC or ALX_CLK_GATE_RXQ or ALX_CLK_GATE_TXQ or ALX_CLK_GATE_DMAR or ALX_CLK_GATE_DMAW)

; interop between drivers
ALX_DRV                                         = 0x1804
ALX_DRV_PHY_AUTO                                = (1 shl 28)
ALX_DRV_PHY_1000                                = (1 shl 27)
ALX_DRV_PHY_100                                 = (1 shl 26)
ALX_DRV_PHY_10                                  = (1 shl 25)
ALX_DRV_PHY_DUPLEX                              = (1 shl 24)
; bit23: adv Pause
ALX_DRV_PHY_PAUSE                               = (1 shl 23)
; bit22: adv Asym Pause
ALX_DRV_PHY_MASK                                = 0xFF
ALX_DRV_PHY_SHIFT                               = 21
ALX_DRV_PHY_UNKNOWN                             = 0

; flag of phy inited
ALX_PHY_INITED                                  = 0x003F

; reg 1830 ~ 186C for C0+, 16 bit map patterns and wake packet detection
ALX_WOL_CTRL2                                   = 0x1830
ALX_WOL_CTRL2_DATA_STORE                        = (1 shl 3)
ALX_WOL_CTRL2_PTRN_EVT                          = (1 shl 2)
ALX_WOL_CTRL2_PME_PTRN_EN                       = (1 shl 1)
ALX_WOL_CTRL2_PTRN_EN                           = (1 shl 0)

ALX_WOL_CTRL3                                   = 0x1834
ALX_WOL_CTRL3_PTRN_ADDR_MASK                    = 0xFFFFF
ALX_WOL_CTRL3_PTRN_ADDR_SHIFT                   = 0

ALX_WOL_CTRL4                                   = 0x1838
ALX_WOL_CTRL4_PT15_MATCH                        = (1 shl 31)
ALX_WOL_CTRL4_PT14_MATCH                        = (1 shl 30)
ALX_WOL_CTRL4_PT13_MATCH                        = (1 shl 29)
ALX_WOL_CTRL4_PT12_MATCH                        = (1 shl 28)
ALX_WOL_CTRL4_PT11_MATCH                        = (1 shl 27)
ALX_WOL_CTRL4_PT10_MATCH                        = (1 shl 26)
ALX_WOL_CTRL4_PT9_MATCH                         = (1 shl 25)
ALX_WOL_CTRL4_PT8_MATCH                         = (1 shl 24)
ALX_WOL_CTRL4_PT7_MATCH                         = (1 shl 23)
ALX_WOL_CTRL4_PT6_MATCH                         = (1 shl 22)
ALX_WOL_CTRL4_PT5_MATCH                         = (1 shl 21)
ALX_WOL_CTRL4_PT4_MATCH                         = (1 shl 20)
ALX_WOL_CTRL4_PT3_MATCH                         = (1 shl 19)
ALX_WOL_CTRL4_PT2_MATCH                         = (1 shl 18)
ALX_WOL_CTRL4_PT1_MATCH                         = (1 shl 17)
ALX_WOL_CTRL4_PT0_MATCH                         = (1 shl 16)
ALX_WOL_CTRL4_PT15_EN                           = (1 shl 15)
ALX_WOL_CTRL4_PT14_EN                           = (1 shl 14)
ALX_WOL_CTRL4_PT13_EN                           = (1 shl 13)
ALX_WOL_CTRL4_PT12_EN                           = (1 shl 12)
ALX_WOL_CTRL4_PT11_EN                           = (1 shl 11)
ALX_WOL_CTRL4_PT10_EN                           = (1 shl 10)
ALX_WOL_CTRL4_PT9_EN                            = (1 shl 9)
ALX_WOL_CTRL4_PT8_EN                            = (1 shl 8)
ALX_WOL_CTRL4_PT7_EN                            = (1 shl 7)
ALX_WOL_CTRL4_PT6_EN                            = (1 shl 6)
ALX_WOL_CTRL4_PT5_EN                            = (1 shl 5)
ALX_WOL_CTRL4_PT4_EN                            = (1 shl 4)
ALX_WOL_CTRL4_PT3_EN                            = (1 shl 3)
ALX_WOL_CTRL4_PT2_EN                            = (1 shl 2)
ALX_WOL_CTRL4_PT1_EN                            = (1 shl 1)
ALX_WOL_CTRL4_PT0_EN                            = (1 shl 0)

ALX_WOL_CTRL5                                   = 0x183C
ALX_WOL_CTRL5_PT3_LEN_MASK                      = 0xFF
ALX_WOL_CTRL5_PT3_LEN_SHIFT                     = 24
ALX_WOL_CTRL5_PT2_LEN_MASK                      = 0xFF
ALX_WOL_CTRL5_PT2_LEN_SHIFT                     = 16
ALX_WOL_CTRL5_PT1_LEN_MASK                      = 0xFF
ALX_WOL_CTRL5_PT1_LEN_SHIFT                     = 8
ALX_WOL_CTRL5_PT0_LEN_MASK                      = 0xFF
ALX_WOL_CTRL5_PT0_LEN_SHIFT                     = 0

ALX_WOL_CTRL6                                   = 0x1840
ALX_WOL_CTRL5_PT7_LEN_MASK                      = 0xFF
ALX_WOL_CTRL5_PT7_LEN_SHIFT                     = 24
ALX_WOL_CTRL5_PT6_LEN_MASK                      = 0xFF
ALX_WOL_CTRL5_PT6_LEN_SHIFT                     = 16
ALX_WOL_CTRL5_PT5_LEN_MASK                      = 0xFF
ALX_WOL_CTRL5_PT5_LEN_SHIFT                     = 8
ALX_WOL_CTRL5_PT4_LEN_MASK                      = 0xFF
ALX_WOL_CTRL5_PT4_LEN_SHIFT                     = 0

ALX_WOL_CTRL7                                   = 0x1844
ALX_WOL_CTRL5_PT11_LEN_MASK                     = 0xFF
ALX_WOL_CTRL5_PT11_LEN_SHIFT                    = 24
ALX_WOL_CTRL5_PT10_LEN_MASK                     = 0xFF
ALX_WOL_CTRL5_PT10_LEN_SHIFT                    = 16
ALX_WOL_CTRL5_PT9_LEN_MASK                      = 0xFF
ALX_WOL_CTRL5_PT9_LEN_SHIFT                     = 8
ALX_WOL_CTRL5_PT8_LEN_MASK                      = 0xFF
ALX_WOL_CTRL5_PT8_LEN_SHIFT                     = 0

ALX_WOL_CTRL8                                   = 0x1848
ALX_WOL_CTRL5_PT15_LEN_MASK                     = 0xFF
ALX_WOL_CTRL5_PT15_LEN_SHIFT                    = 24
ALX_WOL_CTRL5_PT14_LEN_MASK                     = 0xFF
ALX_WOL_CTRL5_PT14_LEN_SHIFT                    = 16
ALX_WOL_CTRL5_PT13_LEN_MASK                     = 0xFF
ALX_WOL_CTRL5_PT13_LEN_SHIFT                    = 8
ALX_WOL_CTRL5_PT12_LEN_MASK                     = 0xFF
ALX_WOL_CTRL5_PT12_LEN_SHIFT                    = 0

ALX_ACER_FIXED_PTN0                             = 0x1850
ALX_ACER_FIXED_PTN0_MASK                        = 0xFFFFFFFF
ALX_ACER_FIXED_PTN0_SHIFT                       = 0

ALX_ACER_FIXED_PTN1                             = 0x1854
ALX_ACER_FIXED_PTN1_MASK                        = 0xFFFF
ALX_ACER_FIXED_PTN1_SHIFT                       = 0

ALX_ACER_RANDOM_NUM0                            = 0x1858
ALX_ACER_RANDOM_NUM0_MASK                       = 0xFFFFFFFF
ALX_ACER_RANDOM_NUM0_SHIFT                      = 0

ALX_ACER_RANDOM_NUM1                            = 0x185C
ALX_ACER_RANDOM_NUM1_MASK                       = 0xFFFFFFFF
ALX_ACER_RANDOM_NUM1_SHIFT                      = 0

ALX_ACER_RANDOM_NUM2                            = 0x1860
ALX_ACER_RANDOM_NUM2_MASK                       = 0xFFFFFFFF
ALX_ACER_RANDOM_NUM2_SHIFT                      = 0

ALX_ACER_RANDOM_NUM3                            = 0x1864
ALX_ACER_RANDOM_NUM3_MASK                       = 0xFFFFFFFF
ALX_ACER_RANDOM_NUM3_SHIFT                      = 0

ALX_ACER_MAGIC                                  = 0x1868
ALX_ACER_MAGIC_EN                               = (1 shl 31)
ALX_ACER_MAGIC_PME_EN                           = (1 shl 30)
ALX_ACER_MAGIC_MATCH                            = (1 shl 29)
ALX_ACER_MAGIC_FF_CHECK                         = (1 shl 10)
ALX_ACER_MAGIC_RAN_LEN_MASK                     = 0x1F
ALX_ACER_MAGIC_RAN_LEN_SHIFT                    = 5
ALX_ACER_MAGIC_FIX_LEN_MASK                     = 0x1F
ALX_ACER_MAGIC_FIX_LEN_SHIFT                    = 0

ALX_ACER_TIMER                                  = 0x186C
ALX_ACER_TIMER_EN                               = (1 shl 31)
ALX_ACER_TIMER_PME_EN                           = (1 shl 30)
ALX_ACER_TIMER_MATCH                            = (1 shl 29)
ALX_ACER_TIMER_THRES_MASK                       = 0x1FFFF
ALX_ACER_TIMER_THRES_SHIFT                      = 0
ALX_ACER_TIMER_THRES_DEF                        = 1

; RSS definitions
ALX_RSS_KEY0                                    = 0x14B0
ALX_RSS_KEY1                                    = 0x14B4
ALX_RSS_KEY2                                    = 0x14B8
ALX_RSS_KEY3                                    = 0x14BC
ALX_RSS_KEY4                                    = 0x14C0
ALX_RSS_KEY5                                    = 0x14C4
ALX_RSS_KEY6                                    = 0x14C8
ALX_RSS_KEY7                                    = 0x14CC
ALX_RSS_KEY8                                    = 0x14D0
ALX_RSS_KEY9                                    = 0x14D4

ALX_RSS_IDT_TBL0                                = 0x1B00

ALX_MSI_MAP_TBL1                                = 0x15D0
ALX_MSI_MAP_TBL1_TXQ1_SHIFT                     = 20
ALX_MSI_MAP_TBL1_TXQ0_SHIFT                     = 16
ALX_MSI_MAP_TBL1_RXQ3_SHIFT                     = 12
ALX_MSI_MAP_TBL1_RXQ2_SHIFT                     = 8
ALX_MSI_MAP_TBL1_RXQ1_SHIFT                     = 4
ALX_MSI_MAP_TBL1_RXQ0_SHIFT                     = 0

ALX_MSI_MAP_TBL2                                = 0x15D8
ALX_MSI_MAP_TBL2_TXQ3_SHIFT                     = 20
ALX_MSI_MAP_TBL2_TXQ2_SHIFT                     = 16
ALX_MSI_MAP_TBL2_RXQ7_SHIFT                     = 12
ALX_MSI_MAP_TBL2_RXQ6_SHIFT                     = 8
ALX_MSI_MAP_TBL2_RXQ5_SHIFT                     = 4
ALX_MSI_MAP_TBL2_RXQ4_SHIFT                     = 0

ALX_MSI_ID_MAP                                  = 0x15D4

ALX_MSI_RETRANS_TIMER                           = 0x1920
; bit16: 1:line,0:standard
ALX_MSI_MASK_SEL_LINE                           = (1 shl 16)
ALX_MSI_RETRANS_TM_MASK                         = 0xFFFF
ALX_MSI_RETRANS_TM_SHIFT                        = 0

; CR DMA ctrl

; TX QoS
ALX_WRR                                         = 0x1938
ALX_WRR_PRI_MASK                                = 0x3
ALX_WRR_PRI_SHIFT                               = 29
ALX_WRR_PRI_RESTRICT_NONE                       = 3
ALX_WRR_PRI3_MASK                               = 0x1F
ALX_WRR_PRI3_SHIFT                              = 24
ALX_WRR_PRI2_MASK                               = 0x1F
ALX_WRR_PRI2_SHIFT                              = 16
ALX_WRR_PRI1_MASK                               = 0x1F
ALX_WRR_PRI1_SHIFT                              = 8
ALX_WRR_PRI0_MASK                               = 0x1F
ALX_WRR_PRI0_SHIFT                              = 0

ALX_HQTPD                                       = 0x193C
ALX_HQTPD_BURST_EN                              = (1 shl 31)
ALX_HQTPD_Q3_NUMPREF_MASK                       = 0xF
ALX_HQTPD_Q3_NUMPREF_SHIFT                      = 8
ALX_HQTPD_Q2_NUMPREF_MASK                       = 0xF
ALX_HQTPD_Q2_NUMPREF_SHIFT                      = 4
ALX_HQTPD_Q1_NUMPREF_MASK                       = 0xF
ALX_HQTPD_Q1_NUMPREF_SHIFT                      = 0

ALX_MISC                                        = 0x19C0
ALX_MISC_PSW_OCP_MASK                           = 0x7
ALX_MISC_PSW_OCP_SHIFT                          = 21
ALX_MISC_PSW_OCP_DEF                            = 0x7
ALX_MISC_ISO_EN                                 = (1 shl 12)
ALX_MISC_INTNLOSC_OPEN                          = (1 shl 3)

ALX_MSIC2                                       = 0x19C8
ALX_MSIC2_CALB_START                            = (1 shl 0)

ALX_MISC3                                       = 0x19CC
; bit1: 1:Software control 25M
ALX_MISC3_25M_BY_SW                             = (1 shl 1)
; bit0: 25M switch to intnl OSC
ALX_MISC3_25M_NOTO_INTNL                        = (1 shl 0)

; MSIX tbl in memory space
ALX_MSIX_ENTRY_BASE                             = 0x2000

;******************** PHY regs definition;**************************

; PHY Specific Status Register
ALX_MII_GIGA_PSSR                               = 0x11
ALX_GIGA_PSSR_SPD_DPLX_RESOLVED                 = 0x0800
ALX_GIGA_PSSR_DPLX                              = 0x2000
ALX_GIGA_PSSR_SPEED                             = 0xC000
ALX_GIGA_PSSR_10MBS                             = 0x0000
ALX_GIGA_PSSR_100MBS                            = 0x4000
ALX_GIGA_PSSR_1000MBS                           = 0x8000

; PHY Interrupt Enable Register
ALX_MII_IER                                     = 0x12
ALX_IER_LINK_UP                                 = 0x0400
ALX_IER_LINK_DOWN                               = 0x0800

; PHY Interrupt Status Register
ALX_MII_ISR                                     = 0x13

ALX_MII_DBG_ADDR                                = 0x1D
ALX_MII_DBG_DATA                                = 0x1E

;**************************** debug port;************************************

ALX_MIIDBG_ANACTRL                              = 0x00
ALX_ANACTRL_DEF                                 = 0x02EF

ALX_MIIDBG_SYSMODCTRL                           = 0x04
; en half bias
ALX_SYSMODCTRL_IECHOADJ_DEF                     = 0xBB8B

ALX_MIIDBG_SRDSYSMOD                            = 0x05
ALX_SRDSYSMOD_DEEMP_EN                          = 0x0040
ALX_SRDSYSMOD_DEF                               = 0x2C46

ALX_MIIDBG_HIBNEG                               = 0x0B
ALX_HIBNEG_PSHIB_EN                             = 0x8000
ALX_HIBNEG_HIB_PSE                              = 0x1000
ALX_HIBNEG_DEF                                  = 0xBC40
ALX_HIBNEG_NOHIB                                = (ALX_HIBNEG_DEF and not(ALX_HIBNEG_PSHIB_EN or ALX_HIBNEG_HIB_PSE))

ALX_MIIDBG_TST10BTCFG                           = 0x12
ALX_TST10BTCFG_DEF                              = 0x4C04

ALX_MIIDBG_AZ_ANADECT                           = 0x15
ALX_AZ_ANADECT_DEF                              = 0x3220
ALX_AZ_ANADECT_LONG                             = 0x3210

ALX_MIIDBG_MSE16DB                              = 0x18
ALX_MSE16DB_UP                                  = 0x05EA
ALX_MSE16DB_DOWN                                = 0x02EA

ALX_MIIDBG_MSE20DB                              = 0x1C
ALX_MSE20DB_TH_MASK                             = 0x7F
ALX_MSE20DB_TH_SHIFT                            = 2
ALX_MSE20DB_TH_DEF                              = 0x2E
ALX_MSE20DB_TH_HI                               = 0x54

ALX_MIIDBG_AGC                                  = 0x23
ALX_AGC_2_VGA_MASK                              = 0x3F
ALX_AGC_2_VGA_SHIFT                             = 8
ALX_AGC_LONG1G_LIMT                             = 40
ALX_AGC_LONG100M_LIMT                           = 44

ALX_MIIDBG_LEGCYPS                              = 0x29
ALX_LEGCYPS_EN                                  = 0x8000
ALX_LEGCYPS_DEF                                 = 0x129D

ALX_MIIDBG_TST100BTCFG                          = 0x36
ALX_TST100BTCFG_DEF                             = 0xE12C

ALX_MIIDBG_GREENCFG                             = 0x3B
ALX_GREENCFG_DEF                                = 0x7078

ALX_MIIDBG_GREENCFG2                            = 0x3D
ALX_GREENCFG2_BP_GREEN                          = 0x8000
ALX_GREENCFG2_GATE_DFSE_EN                      = 0x0080

;****** dev 3;********
ALX_MIIEXT_PCS                                  = 3

ALX_MIIEXT_CLDCTRL3                             = 0x8003
ALX_CLDCTRL3_BP_CABLE1TH_DET_GT                 = 0x8000

ALX_MIIEXT_CLDCTRL5                             = 0x8005
ALX_CLDCTRL5_BP_VD_HLFBIAS                      = 0x4000

ALX_MIIEXT_CLDCTRL6                             = 0x8006
ALX_CLDCTRL6_CAB_LEN_MASK                       = 0xFF
ALX_CLDCTRL6_CAB_LEN_SHIFT                      = 0
ALX_CLDCTRL6_CAB_LEN_SHORT1G                    = 116
ALX_CLDCTRL6_CAB_LEN_SHORT100M                  = 152

ALX_MIIEXT_VDRVBIAS                             = 0x8062
ALX_VDRVBIAS_DEF                                = 0x3

;******** dev 7;*********
ALX_MIIEXT_ANEG                                 = 7

ALX_MIIEXT_LOCAL_EEEADV                         = 0x3C
ALX_LOCAL_EEEADV_1000BT                         = 0x0004
ALX_LOCAL_EEEADV_100BT                          = 0x0002

ALX_MIIEXT_AFE                                  = 0x801A
ALX_AFE_10BT_100M_TH                            = 0x0040

ALX_MIIEXT_S3DIG10                              = 0x8023
; bit0: 1:bypass 10BT rx fifo, 0:original 10BT rx
ALX_MIIEXT_S3DIG10_SL                           = 0x0001
ALX_MIIEXT_S3DIG10_DEF                          = 0

ALX_MIIEXT_NLP78                                = 0x8027
ALX_MIIEXT_NLP78_120M_DEF                       = 0x8A05


; tpd word 1
TPD_CXSUMSTART_MASK                             = 0x00FF
TPD_CXSUMSTART_SHIFT                            = 0
TPD_L4HDROFFSET_MASK                            = 0x00FF
TPD_L4HDROFFSET_SHIFT                           = 0
TPD_CXSUM_EN_MASK                               = 0x0001
TPD_CXSUM_EN_SHIFT                              = 8
TPD_IP_XSUM_MASK                                = 0x0001
TPD_IP_XSUM_SHIFT                               = 9
TPD_TCP_XSUM_MASK                               = 0x0001
TPD_TCP_XSUM_SHIFT                              = 10
TPD_UDP_XSUM_MASK                               = 0x0001
TPD_UDP_XSUM_SHIFT                              = 11
TPD_LSO_EN_MASK                                 = 0x0001
TPD_LSO_EN_SHIFT                                = 12
TPD_LSO_V2_MASK                                 = 0x0001
TPD_LSO_V2_SHIFT                                = 13
TPD_VLTAGGED_MASK                               = 0x0001
TPD_VLTAGGED_SHIFT                              = 14
TPD_INS_VLTAG_MASK                              = 0x0001
TPD_INS_VLTAG_SHIFT                             = 15
TPD_IPV4_MASK                                   = 0x0001
TPD_IPV4_SHIFT                                  = 16
TPD_ETHTYPE_MASK                                = 0x0001
TPD_ETHTYPE_SHIFT                               = 17
TPD_CXSUMOFFSET_MASK                            = 0x00FF
TPD_CXSUMOFFSET_SHIFT                           = 18
TPD_MSS_MASK                                    = 0x1FFF
TPD_MSS_SHIFT                                   = 18
TPD_EOP_MASK                                    = 0x0001
TPD_EOP_SHIFT                                   = 31


; rrd word 0
RRD_XSUM_MASK   = 0xFFFF
RRD_XSUM_SHIFT  = 0
RRD_NOR_MASK    = 0x000F
RRD_NOR_SHIFT   = 16
RRD_SI_MASK     = 0x0FFF
RRD_SI_SHIFT    = 20

; rrd word 2 
RRD_VLTAG_MASK  = 0xFFFF
RRD_VLTAG_SHIFT = 0
RRD_PID_MASK    = 0x00FF
RRD_PID_SHIFT   = 16
; non-ip packet 
RRD_PID_NONIP   = 0
; ipv4(only) 
RRD_PID_IPV4    = 1
; tcp/ipv6 
RRD_PID_IPV6TCP = 2
; tcp/ipv4 
RRD_PID_IPV4TCP = 3
; udp/ipv6 
RRD_PID_IPV6UDP = 4
; udp/ipv4 
RRD_PID_IPV4UDP = 5
; ipv6(only) 
RRD_PID_IPV6    = 6
; LLDP packet 
RRD_PID_LLDP    = 7
; 1588 packet 
RRD_PID_1588    = 8
RRD_RSSQ_MASK   = 0x0007
RRD_RSSQ_SHIFT  = 25
RRD_RSSALG_MASK = 0x000F
RRD_RSSALG_SHIFT        = 28
RRD_RSSALG_TCPV6        = 0x1
RRD_RSSALG_IPV6 = 0x2
RRD_RSSALG_TCPV4        = 0x4
RRD_RSSALG_IPV4 = 0x8

; rrd word 3 
RRD_PKTLEN_MASK = 0x3FFF
RRD_PKTLEN_SHIFT        = 0
RRD_ERR_L4_MASK = 0x0001
RRD_ERR_L4_SHIFT        = 14
RRD_ERR_IPV4_MASK       = 0x0001
RRD_ERR_IPV4_SHIFT      = 15
RRD_VLTAGGED_MASK       = 0x0001
RRD_VLTAGGED_SHIFT      = 16
RRD_OLD_PID_MASK        = 0x0007
RRD_OLD_PID_SHIFT       = 17
RRD_ERR_RES_MASK        = 0x0001
RRD_ERR_RES_SHIFT       = 20
RRD_ERR_FCS_MASK        = 0x0001
RRD_ERR_FCS_SHIFT       = 21
RRD_ERR_FAE_MASK        = 0x0001
RRD_ERR_FAE_SHIFT       = 22
RRD_ERR_TRUNC_MASK      = 0x0001
RRD_ERR_TRUNC_SHIFT     = 23
RRD_ERR_RUNT_MASK       = 0x0001
RRD_ERR_RUNT_SHIFT      = 24
RRD_ERR_ICMP_MASK       = 0x0001
RRD_ERR_ICMP_SHIFT      = 25
RRD_BCAST_MASK  = 0x0001
RRD_BCAST_SHIFT = 26
RRD_MCAST_MASK  = 0x0001
RRD_MCAST_SHIFT = 27
RRD_ETHTYPE_MASK        = 0x0001
RRD_ETHTYPE_SHIFT       = 28
RRD_ERR_FIFOV_MASK      = 0x0001
RRD_ERR_FIFOV_SHIFT     = 29
RRD_ERR_LEN_MASK        = 0x0001
RRD_ERR_LEN_SHIFT       = 30
RRD_UPDATED_MASK        = 0x0001
RRD_UPDATED_SHIFT       = 31


ALX_ISR_MISC = ALX_ISR_PCIE_LNKDOWN or ALX_ISR_DMAW or ALX_ISR_DMAR or ALX_ISR_SMB or ALX_ISR_MANU or ALX_ISR_TIMER

ALX_ISR_FATAL = ALX_ISR_PCIE_LNKDOWN or ALX_ISR_DMAW or ALX_ISR_DMAR

ALX_ISR_ALERT = ALX_ISR_RXF_OV or ALX_ISR_TXF_UR or ALX_ISR_RFD_UR

ALX_ISR_ALL_QUEUES = ALX_ISR_TX_Q0 or ALX_ISR_TX_Q1 or ALX_ISR_TX_Q2 or ALX_ISR_TX_Q3 or ALX_ISR_RX_Q0 or ALX_ISR_RX_Q1 or ALX_ISR_RX_Q2 or ALX_ISR_RX_Q3 or ALX_ISR_RX_Q4 or ALX_ISR_RX_Q5 or ALX_ISR_RX_Q6 or ALX_ISR_RX_Q7
