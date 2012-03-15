;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  RTL8169.INC                                                    ;;
;;                                                                 ;;
;;  Ethernet driver for Menuet OS                                  ;;
;;                                                                 ;;
;;  Version 0.1  11 February 2007                                  ;;
;;                                                                 ;;
;;  Driver for chips of RealTek 8169 family                        ;;
;;  References:                                                    ;;
;;    r8169.c - linux driver (etherboot project)                   ;;
;;    ethernet driver template by Mike Hibbett                     ;;
;;                                                                 ;;
;;  The copyright statement is                                     ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;  Copyright 2007 mike.dld,                                       ;;
;;   mike.dld@gmail.com                                            ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


        ETH_ALEN               equ 6
        ETH_HLEN               equ (2 * ETH_ALEN + 2)
        ETH_ZLEN               equ 60 ; 60 + 4bytes auto payload for
                                      ; mininmum 64bytes frame length

        RTL8169_REG_MAC0               equ 0x0 ; Ethernet hardware address
        RTL8169_REG_MAR0               equ 0x8 ; Multicast filter
        RTL8169_REG_TxDescStartAddr    equ 0x20
        RTL8169_REG_TxHDescStartAddr   equ 0x28
        RTL8169_REG_FLASH              equ 0x30
        RTL8169_REG_ERSR               equ 0x36
        RTL8169_REG_ChipCmd            equ 0x37
        RTL8169_REG_TxPoll             equ 0x38
        RTL8169_REG_IntrMask           equ 0x3C
        RTL8169_REG_IntrStatus         equ 0x3E
        RTL8169_REG_TxConfig           equ 0x40
        RTL8169_REG_RxConfig           equ 0x44
        RTL8169_REG_RxMissed           equ 0x4C
        RTL8169_REG_Cfg9346            equ 0x50
        RTL8169_REG_Config0            equ 0x51
        RTL8169_REG_Config1            equ 0x52
        RTL8169_REG_Config2            equ 0x53
        RTL8169_REG_Config3            equ 0x54
        RTL8169_REG_Config4            equ 0x55
        RTL8169_REG_Config5            equ 0x56
        RTL8169_REG_MultiIntr          equ 0x5C
        RTL8169_REG_PHYAR              equ 0x60
        RTL8169_REG_TBICSR             equ 0x64
        RTL8169_REG_TBI_ANAR           equ 0x68
        RTL8169_REG_TBI_LPAR           equ 0x6A
        RTL8169_REG_PHYstatus          equ 0x6C
        RTL8169_REG_RxMaxSize          equ 0xDA
        RTL8169_REG_CPlusCmd           equ 0xE0
        RTL8169_REG_RxDescStartAddr    equ 0xE4
        RTL8169_REG_ETThReg            equ 0xEC
        RTL8169_REG_FuncEvent          equ 0xF0
        RTL8169_REG_FuncEventMask      equ 0xF4
        RTL8169_REG_FuncPresetState    equ 0xF8
        RTL8169_REG_FuncForceEvent     equ 0xFC

        ; InterruptStatusBits
        RTL8169_ISB_SYSErr             equ 0x8000
        RTL8169_ISB_PCSTimeout         equ 0x4000
        RTL8169_ISB_SWInt              equ 0x0100
        RTL8169_ISB_TxDescUnavail      equ 0x80
        RTL8169_ISB_RxFIFOOver         equ 0x40
        RTL8169_ISB_LinkChg            equ 0x20
        RTL8169_ISB_RxOverflow         equ 0x10
        RTL8169_ISB_TxErr              equ 0x08
        RTL8169_ISB_TxOK               equ 0x04
        RTL8169_ISB_RxErr              equ 0x02
        RTL8169_ISB_RxOK               equ 0x01

        ; RxStatusDesc
        RTL8169_SD_RxRES               equ 0x00200000
        RTL8169_SD_RxCRC               equ 0x00080000
        RTL8169_SD_RxRUNT              equ 0x00100000
        RTL8169_SD_RxRWT               equ 0x00400000

        ; ChipCmdBits
        RTL8169_CMD_Reset              equ 0x10
        RTL8169_CMD_RxEnb              equ 0x08
        RTL8169_CMD_TxEnb              equ 0x04
        RTL8169_CMD_RxBufEmpty         equ 0x01

        ; Cfg9346Bits
        RTL8169_CFG_9346_Lock          equ 0x00
        RTL8169_CFG_9346_Unlock        equ 0xC0

        ; rx_mode_bits
        RTL8169_RXM_AcceptErr          equ 0x20
        RTL8169_RXM_AcceptRunt         equ 0x10
        RTL8169_RXM_AcceptBroadcast    equ 0x08
        RTL8169_RXM_AcceptMulticast    equ 0x04
        RTL8169_RXM_AcceptMyPhys       equ 0x02
        RTL8169_RXM_AcceptAllPhys      equ 0x01

        ; RxConfigBits
        RTL8169_RXC_FIFOShift          equ 13
        RTL8169_RXC_DMAShift           equ 8

        ; TxConfigBits
        RTL8169_TXC_InterFrameGapShift equ 24
        RTL8169_TXC_DMAShift           equ 8    ; DMA burst value (0-7) is shift this many bits

        ; rtl8169_PHYstatus
        RTL8169_PHYS_TBI_Enable        equ 0x80
        RTL8169_PHYS_TxFlowCtrl        equ 0x40
        RTL8169_PHYS_RxFlowCtrl        equ 0x20
        RTL8169_PHYS_1000bpsF          equ 0x10
        RTL8169_PHYS_100bps            equ 0x08
        RTL8169_PHYS_10bps             equ 0x04
        RTL8169_PHYS_LinkStatus        equ 0x02
        RTL8169_PHYS_FullDup           equ 0x01

        ; GIGABIT_PHY_registers
        RTL8169_PHY_CTRL_REG           equ 0
        RTL8169_PHY_STAT_REG           equ 1
        RTL8169_PHY_AUTO_NEGO_REG      equ 4
        RTL8169_PHY_1000_CTRL_REG      equ 9

        ; GIGABIT_PHY_REG_BIT
        RTL8169_PHY_Restart_Auto_Nego  equ 0x0200
        RTL8169_PHY_Enable_Auto_Nego   equ 0x1000

        ; PHY_STAT_REG = 1;
        RTL8169_PHY_Auto_Neco_Comp     equ 0x0020

        ; PHY_AUTO_NEGO_REG = 4;
        RTL8169_PHY_Cap_10_Half        equ 0x0020
        RTL8169_PHY_Cap_10_Full        equ 0x0040
        RTL8169_PHY_Cap_100_Half       equ 0x0080
        RTL8169_PHY_Cap_100_Full       equ 0x0100

        ; PHY_1000_CTRL_REG = 9;
        RTL8169_PHY_Cap_1000_Full      equ 0x0200
        RTL8169_PHY_Cap_1000_Half      equ 0x0100

        RTL8169_PHY_Cap_PAUSE          equ 0x0400
        RTL8169_PHY_Cap_ASYM_PAUSE     equ 0x0800

        RTL8169_PHY_Cap_Null           equ 0x0

        ; _MediaType
        RTL8169_MT_10_Half             equ 0x01
        RTL8169_MT_10_Full             equ 0x02
        RTL8169_MT_100_Half            equ 0x04
        RTL8169_MT_100_Full            equ 0x08
        RTL8169_MT_1000_Full           equ 0x10

        ; _TBICSRBit
        RTL8169_TBI_LinkOK             equ 0x02000000

        ; _DescStatusBit
        RTL8169_DSB_OWNbit             equ 0x80000000
        RTL8169_DSB_EORbit             equ 0x40000000
        RTL8169_DSB_FSbit              equ 0x20000000
        RTL8169_DSB_LSbit              equ 0x10000000

; MAC address length
MAC_ADDR_LEN        equ 6

; max supported gigabit ethernet frame size -- must be at least (dev->mtu+14+4)
MAX_ETH_FRAME_SIZE  equ 1536

TX_FIFO_THRESH      equ 256     ; In bytes

RX_FIFO_THRESH      equ 7       ; 7 means NO threshold, Rx buffer level before first PCI xfer
RX_DMA_BURST        equ 7       ; Maximum PCI burst, '6' is 1024
TX_DMA_BURST        equ 7       ; Maximum PCI burst, '6' is 1024
ETTh                equ 0x3F    ; 0x3F means NO threshold

EarlyTxThld         equ 0x3F    ; 0x3F means NO early transmit
RxPacketMaxSize     equ 0x0800  ; Maximum size supported is 16K-1
InterFrameGap       equ 0x03    ; 3 means InterFrameGap = the shortest one

NUM_TX_DESC         equ 1       ; Number of Tx descriptor registers
NUM_RX_DESC         equ 4       ; Number of Rx descriptor registers
RX_BUF_SIZE         equ 1536    ; Rx Buffer size

HZ                  equ 1000

RTL_MIN_IO_SIZE     equ 0x80
TX_TIMEOUT          equ (6*HZ)

RTL8169_TIMER_EXPIRE_TIME equ 100

ETH_HDR_LEN         equ 14
DEFAULT_MTU         equ 1500
DEFAULT_RX_BUF_LEN  equ 1536


;#ifdef RTL8169_JUMBO_FRAME_SUPPORT
;#define MAX_JUMBO_FRAME_MTU    ( 10000 )
;#define MAX_RX_SKBDATA_SIZE    ( MAX_JUMBO_FRAME_MTU + ETH_HDR_LEN )
;#else
MAX_RX_SKBDATA_SIZE equ 1600
;#endif                         //end #ifdef RTL8169_JUMBO_FRAME_SUPPORT

;#ifdef RTL8169_USE_IO
;!!!#define RTL_W8(reg, val8)   outb ((val8), ioaddr + (reg))
macro RTL_W8 reg,val8 {
  if ~reg eq dx
        mov     dx, word[rtl8169_tpc.mmio_addr]
        add     dx, reg
  end if
  if ~val8 eq al
        mov     al, val8
  end if
        out     dx, al
}
;!!!#define RTL_W16(reg, val16) outw ((val16), ioaddr + (reg))
macro RTL_W16 reg,val16 {
  if ~reg eq dx
        mov     dx, word[rtl8169_tpc.mmio_addr]
        add     dx, reg
  end if
  if ~val16 eq ax
        mov     ax, val16
  end if
        out     dx, ax
}
;!!!#define RTL_W32(reg, val32) outl ((val32), ioaddr + (reg))
macro RTL_W32 reg,val32 {
  if ~reg eq dx
        mov     dx, word[rtl8169_tpc.mmio_addr]
        add     dx, reg
  end if
  if ~val32 eq eax
        mov     eax, val32
  end if
        out     dx, eax
}
;!!!#define RTL_R8(reg)         inb (ioaddr + (reg))
macro RTL_R8 reg {
  if ~reg eq dx
        mov     dx, word[rtl8169_tpc.mmio_addr]
        add     dx, reg
  end if
        in      al, dx
}
;!!!#define RTL_R16(reg)        inw (ioaddr + (reg))
macro RTL_R16 reg {
  if ~reg eq dx
        mov     dx, word[rtl8169_tpc.mmio_addr]
        add     dx, reg
  end if
        in      ax, dx
}
;!!!#define RTL_R32(reg)        ((unsigned long) inl (ioaddr + (reg)))
macro RTL_R32 reg {
  if ~reg eq dx
        mov     dx, word[rtl8169_tpc.mmio_addr]
        add     dx, reg
  end if
        in      eax, dx
}
;#else
; write/read MMIO register
;#define RTL_W8(reg, val8)      writeb ((val8), ioaddr + (reg))
;#define RTL_W16(reg, val16)    writew ((val16), ioaddr + (reg))
;#define RTL_W32(reg, val32)    writel ((val32), ioaddr + (reg))
;#define RTL_R8(reg)            readb (ioaddr + (reg))
;#define RTL_R16(reg)           readw (ioaddr + (reg))
;#define RTL_R32(reg)           ((unsigned long) readl (ioaddr + (reg)))
;#endif

MCFG_METHOD_01       equ 0x01
MCFG_METHOD_02       equ 0x02
MCFG_METHOD_03       equ 0x03
MCFG_METHOD_04       equ 0x04
MCFG_METHOD_05       equ 0x05
MCFG_METHOD_11       equ 0x0b
MCFG_METHOD_12       equ 0x0c
MCFG_METHOD_13       equ 0x0d
MCFG_METHOD_14       equ 0x0e
MCFG_METHOD_15       equ 0x0f

PCFG_METHOD_1       equ 0x01    ; PHY Reg 0x03 bit0-3 == 0x0000
PCFG_METHOD_2       equ 0x02    ; PHY Reg 0x03 bit0-3 == 0x0001
PCFG_METHOD_3       equ 0x03    ; PHY Reg 0x03 bit0-3 == 0x0002

PCI_COMMAND_IO          equ 0x1   ; Enable response in I/O space
PCI_COMMAND_MEM         equ 0x2   ; Enable response in mem space
PCI_COMMAND_MASTER      equ 0x4   ; Enable bus mastering
PCI_LATENCY_TIMER       equ 0x0d  ; 8 bits
PCI_COMMAND_SPECIAL     equ 0x8   ; Enable response to special cycles
PCI_COMMAND_INVALIDATE  equ 0x10  ; Use memory write and invalidate
PCI_COMMAND_VGA_PALETTE equ 0x20  ; Enable palette snooping
PCI_COMMAND_PARITY      equ 0x40  ; Enable parity checking
PCI_COMMAND_WAIT        equ 0x80  ; Enable address/data stepping
PCI_COMMAND_SERR        equ 0x100 ; Enable SERR
PCI_COMMAND_FAST_BACK   equ 0x200 ; Enable back-to-back writes

struc rtl8169_TxDesc {
  .status    dd ?
  .vlan_tag  dd ?
  .buf_addr  dd ?
  .buf_Haddr dd ?
}
virtual at 0
  rtl8169_TxDesc rtl8169_TxDesc
  sizeof.rtl8169_TxDesc = $ - rtl8169_TxDesc
end virtual

struc rtl8169_RxDesc {
  .status    dd ?
  .vlan_tag  dd ?
  .buf_addr  dd ?
  .buf_Haddr dd ?
}
virtual at 0
  rtl8169_RxDesc rtl8169_RxDesc
  sizeof.rtl8169_RxDesc = $ - rtl8169_RxDesc
end virtual

virtual at eth_data_start

; Define the TX Descriptor
align 256
rtl8169_tx_ring rb NUM_TX_DESC * sizeof.rtl8169_TxDesc

; Create a static buffer of size RX_BUF_SZ for each
; TX Descriptor.  All descriptors point to a
; part of this buffer
align 256
rtl8169_txb rb NUM_TX_DESC * RX_BUF_SIZE

; Define the RX Descriptor
align 256
rtl8169_rx_ring rb NUM_RX_DESC * sizeof.rtl8169_RxDesc

; Create a static buffer of size RX_BUF_SZ for each
; RX Descriptor   All descriptors point to a
; part of this buffer
align 256
rtl8169_rxb rb NUM_RX_DESC * RX_BUF_SIZE

rtl8169_tpc:
  .mmio_addr    dd ? ; memory map physical address
  .chipset      dd ?
  .pcfg         dd ?
  .mcfg         dd ?
  .cur_rx       dd ? ; Index into the Rx descriptor buffer of next Rx pkt
  .cur_tx       dd ? ; Index into the Tx descriptor buffer of next Rx pkt
  .TxDescArrays dd ? ; Index of Tx Descriptor buffer
  .RxDescArrays dd ? ; Index of Rx Descriptor buffer
  .TxDescArray  dd ? ; Index of 256-alignment Tx Descriptor buffer
  .RxDescArray  dd ? ; Index of 256-alignment Rx Descriptor buffer
  .RxBufferRing rd NUM_RX_DESC ; Index of Rx Buffer array
  .Tx_skbuff    rd NUM_TX_DESC

end virtual

rtl8169_intr_mask = RTL8169_ISB_LinkChg or RTL8169_ISB_RxOverflow or RTL8169_ISB_RxFIFOOver or RTL8169_ISB_TxErr or RTL8169_ISB_TxOK or RTL8169_ISB_RxErr or RTL8169_ISB_RxOK
rtl8169_rx_config = (RX_FIFO_THRESH shl RTL8169_RXC_FIFOShift) or (RX_DMA_BURST shl RTL8169_RXC_DMAShift) or 0x0000000E

iglobal

;static struct {
;       const char *name;
;       u8 mcfg;                /* depend on RTL8169 docs */
;       u32 RxConfigMask;       /* should clear the bits supported by this chip */
;}
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

endg

PCI_COMMAND_IO          equ 0x1    ; Enable response in I/O space
PCI_COMMAND_MEM         equ 0x2    ; Enable response in mem space
PCI_COMMAND_MASTER      equ 0x4    ; Enable bus mastering
PCI_LATENCY_TIMER       equ 0x0d   ; 8 bits
PCI_COMMAND_SPECIAL     equ 0x8    ; Enable response to special cycles
PCI_COMMAND_INVALIDATE  equ 0x10   ; Use memory write and invalidate
PCI_COMMAND_VGA_PALETTE equ 0x20   ; Enable palette snooping
PCI_COMMAND_PARITY      equ 0x40   ; Enable parity checking
PCI_COMMAND_WAIT        equ 0x80   ; Enable address/data stepping
PCI_COMMAND_SERR        equ 0x100  ; Enable SERR
PCI_COMMAND_FAST_BACK   equ 0x200  ; Enable back-to-back writes

PCI_VENDOR_ID           equ 0x00   ; 16 bits
PCI_DEVICE_ID           equ 0x02   ; 16 bits
PCI_COMMAND             equ 0x04   ; 16 bits

PCI_BASE_ADDRESS_0      equ 0x10   ; 32 bits
PCI_BASE_ADDRESS_1      equ 0x14   ; 32 bits
PCI_BASE_ADDRESS_2      equ 0x18   ; 32 bits
PCI_BASE_ADDRESS_3      equ 0x1c   ; 32 bits
PCI_BASE_ADDRESS_4      equ 0x20   ; 32 bits
PCI_BASE_ADDRESS_5      equ 0x24   ; 32 bits

PCI_BASE_ADDRESS_MEM_TYPE_MASK equ 0x06
PCI_BASE_ADDRESS_MEM_TYPE_32   equ 0x00 ; 32 bit address
PCI_BASE_ADDRESS_MEM_TYPE_1M   equ 0x02 ; Below 1M [obsolete]
PCI_BASE_ADDRESS_MEM_TYPE_64   equ 0x04 ; 64 bit address

PCI_BASE_ADDRESS_IO_MASK  equ (not 0x03)
PCI_BASE_ADDRESS_MEM_MASK equ (not 0x0f)
PCI_BASE_ADDRESS_SPACE_IO equ 0x01
PCI_ROM_ADDRESS           equ 0x30      ; 32 bits

proc CONFIG_CMD,where:byte
        movzx   eax, byte[pci_bus]
        shl     eax, 8
        mov     al, [pci_dev]
        shl     eax, 8
        mov     al, [where]
        and     al, not 3
        or      eax, 0x80000000
        ret
endp

proc pci_read_config_byte,where:dword
        push    edx
        stdcall CONFIG_CMD, [where]
        mov     dx, 0xCF8
        out     dx, eax
        mov     edx, [where]
        and     edx, 3
        add     edx, 0xCFC
        in      al, dx
        pop     edx
        ret
endp

proc pci_read_config_word,where:dword
        push    edx
        stdcall CONFIG_CMD, [where]
        mov     dx, 0xCF8
        out     dx, eax
        mov     edx, [where]
        and     edx, 2
        add     edx, 0xCFC
        in      ax, dx
        pop     edx
        ret
endp

proc pci_read_config_dword,where:dword
        push    edx
        stdcall CONFIG_CMD, [where]
        mov     edx, 0xCF8
        out     dx, eax
        mov     edx, 0xCFC
        in      eax, dx
        pop     edx
        ret
endp

proc pci_write_config_byte,where:dword,value:byte
        push    edx
        stdcall CONFIG_CMD, [where]
        mov     dx, 0xCF8
        out     dx, eax
        mov     edx, [where]
        and     edx, 3
        add     edx, 0xCFC
        mov     al, [value]
        out     dx, al
        pop     edx
        ret
endp

proc pci_write_config_word,where:dword,value:word
        push    edx
        stdcall CONFIG_CMD, [where]
        mov     dx, 0xCF8
        out     dx, eax
        mov     edx, [where]
        and     edx, 2
        add     edx, 0xCFC
        mov     ax, [value]
        out     dx, ax
        pop     edx
        ret
endp

proc pci_write_config_dword,where:dword,value:dword
        push    edx
        stdcall CONFIG_CMD, [where]
        mov     edx, 0xCF8
        out     dx, eax
        mov     edx, 0xCFC
        mov     eax, [value]
        out     dx, eax
        pop     edx
        ret
endp

; Set device to be a busmaster in case BIOS neglected to do so.
; Also adjust PCI latency timer to a reasonable value, 32.
proc adjust_pci_device

;        DEBUGF  1,"K : adjust_pci_device\n"

        stdcall pci_read_config_word, PCI_COMMAND
        mov     bx, ax
        or      bx, PCI_COMMAND_MASTER or PCI_COMMAND_IO
        cmp     ax, bx
        je      @f
;        DEBUGF  1,"K : adjust_pci_device: The PCI BIOS has not enabled this device!\nK :   Updating PCI command %x->%x. pci_bus %x pci_device_fn %x\n",ax,bx,[pci_bus]:2,[pci_dev]:2
        stdcall pci_write_config_word, PCI_COMMAND, ebx
    @@:
        stdcall pci_read_config_byte, PCI_LATENCY_TIMER
        cmp     al, 32
        jae     @f
;        DEBUGF  1,"K : adjust_pci_device: PCI latency timer (CFLT) is unreasonably low at %d.\nK :   Setting to 32 clocks.\n",al
        stdcall pci_write_config_byte, PCI_LATENCY_TIMER, 32
    @@:
        ret
endp

; Find the start of a pci resource
proc pci_bar_start,index:dword
        stdcall pci_read_config_dword, [index]
        test    eax, PCI_BASE_ADDRESS_SPACE_IO
        jz      @f
        and     eax, PCI_BASE_ADDRESS_IO_MASK
        jmp     .exit
    @@:
        push    eax
        and     eax, PCI_BASE_ADDRESS_MEM_TYPE_MASK
        cmp     eax, PCI_BASE_ADDRESS_MEM_TYPE_64
        jne     .not64
        mov     eax, [index]
        add     eax, 4
        stdcall pci_read_config_dword, eax
        or      eax, eax
        jz      .not64
;        DEBUGF  1,"K : pci_bar_start: Unhandled 64bit BAR\n"
        add     esp, 4
        or      eax, -1
        ret
  .not64:
        pop     eax
        and     eax, PCI_BASE_ADDRESS_MEM_MASK
  .exit:
        ret
endp

proc rtl8169_init_board

;        DEBUGF  1,"K : rtl8169_init_board\n"

        call    adjust_pci_device

        stdcall pci_bar_start, PCI_BASE_ADDRESS_0
        mov     [rtl8169_tpc.mmio_addr], eax
        ; Soft reset the chip
        RTL_W8  RTL8169_REG_ChipCmd,RTL8169_CMD_Reset

        ; Check that the chip has finished the reset
        mov     ecx, 1000
    @@:
        RTL_R8  RTL8169_REG_ChipCmd
        test    al, RTL8169_CMD_Reset
        jz      @f
        stdcall udelay, 10
        loop    @b
    @@:
        ; identify config method
        RTL_R32 RTL8169_REG_TxConfig
        and     eax, 0x7c800000
;        DEBUGF  1,"K : rtl8169_init_board: TxConfig & 0x7c800000 = 0x%x\n",eax
        mov     esi, mac_info-8
    @@:
        add     esi, 8
        mov     ecx, eax
        and     ecx, [esi]
        cmp     ecx, [esi]
        jne     @b
        mov     eax, [esi+4]
        mov     [rtl8169_tpc.mcfg], eax

        mov     [rtl8169_tpc.pcfg], PCFG_METHOD_3
        stdcall RTL8169_READ_GMII_REG, 3
        and     al, 0x0f
        or      al, al
        jnz     @f
        mov     [rtl8169_tpc.pcfg], PCFG_METHOD_1
        jmp     .pconf
    @@:
        dec     al
        jnz     .pconf
        mov     [rtl8169_tpc.pcfg], PCFG_METHOD_2
  .pconf:

        ; identify chip attached to board
        mov     ecx, 10
        mov     eax, [rtl8169_tpc.mcfg]
    @@:
        dec     ecx
        js      @f
        cmp     eax, [rtl_chip_info+ecx*8]
        jne     @b
        mov     [rtl8169_tpc.chipset], ecx
        jmp     .match
    @@:
        ; if unknown chip, assume array element #0, original RTL-8169 in this case
;        DEBUGF  1,"K : rtl8169_init_board: PCI device: unknown chip version, assuming RTL-8169\n"
        RTL_R32 RTL8169_REG_TxConfig
;        DEBUGF  1,"K : rtl8169_init_board: PCI device: TxConfig = 0x%x\n",eax

        mov     [rtl8169_tpc.chipset], 0

        xor     eax, eax
        inc     eax
        ret

  .match:
        xor     eax, eax
        ret
endp

proc rtl8169_hw_PHY_config

;        DEBUGF  1,"K : rtl8169_hw_PHY_config: priv.mcfg=%d, priv.pcfg=%d\n",[rtl8169_tpc.mcfg],[rtl8169_tpc.pcfg]

;       DBG_PRINT("priv->mcfg=%d, priv->pcfg=%d\n", tpc->mcfg, tpc->pcfg);

        cmp     [rtl8169_tpc.mcfg], MCFG_METHOD_04
        jne     .not_4
;       stdcall RTL8169_WRITE_GMII_REG,0x1F,0x0001
;       stdcall RTL8169_WRITE_GMII_REG,0x1b,0x841e
;       stdcall RTL8169_WRITE_GMII_REG,0x0e,0x7bfb
;       stdcall RTL8169_WRITE_GMII_REG,0x09,0x273a
        stdcall RTL8169_WRITE_GMII_REG, 0x1F, 0x0002
        stdcall RTL8169_WRITE_GMII_REG, 0x01, 0x90D0
        stdcall RTL8169_WRITE_GMII_REG, 0x1F, 0x0000
        jmp     .exit
  .not_4:
        cmp     [rtl8169_tpc.mcfg], MCFG_METHOD_02
        je      @f
        cmp     [rtl8169_tpc.mcfg], MCFG_METHOD_03
        jne     .not_2_or_3
    @@:
        stdcall RTL8169_WRITE_GMII_REG, 0x1F, 0x0001
        stdcall RTL8169_WRITE_GMII_REG, 0x15, 0x1000
        stdcall RTL8169_WRITE_GMII_REG, 0x18, 0x65C7
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0x0000
        stdcall RTL8169_WRITE_GMII_REG, 0x03, 0x00A1
        stdcall RTL8169_WRITE_GMII_REG, 0x02, 0x0008
        stdcall RTL8169_WRITE_GMII_REG, 0x01, 0x1020
        stdcall RTL8169_WRITE_GMII_REG, 0x00, 0x1000
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0x0800
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0x0000
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0x7000
        stdcall RTL8169_WRITE_GMII_REG, 0x03, 0xFF41
        stdcall RTL8169_WRITE_GMII_REG, 0x02, 0xDE60
        stdcall RTL8169_WRITE_GMII_REG, 0x01, 0x0140
        stdcall RTL8169_WRITE_GMII_REG, 0x00, 0x0077
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0x7800
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0x7000
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0xA000
        stdcall RTL8169_WRITE_GMII_REG, 0x03, 0xDF01
        stdcall RTL8169_WRITE_GMII_REG, 0x02, 0xDF20
        stdcall RTL8169_WRITE_GMII_REG, 0x01, 0xFF95
        stdcall RTL8169_WRITE_GMII_REG, 0x00, 0xFA00
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0xA800
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0xA000
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0xB000
        stdcall RTL8169_WRITE_GMII_REG, 0x03, 0xFF41
        stdcall RTL8169_WRITE_GMII_REG, 0x02, 0xDE20
        stdcall RTL8169_WRITE_GMII_REG, 0x01, 0x0140
        stdcall RTL8169_WRITE_GMII_REG, 0x00, 0x00BB
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0xB800
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0xB000
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0xF000
        stdcall RTL8169_WRITE_GMII_REG, 0x03, 0xDF01
        stdcall RTL8169_WRITE_GMII_REG, 0x02, 0xDF20
        stdcall RTL8169_WRITE_GMII_REG, 0x01, 0xFF95
        stdcall RTL8169_WRITE_GMII_REG, 0x00, 0xBF00
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0xF800
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0xF000
        stdcall RTL8169_WRITE_GMII_REG, 0x04, 0x0000
        stdcall RTL8169_WRITE_GMII_REG, 0x1F, 0x0000
        stdcall RTL8169_WRITE_GMII_REG, 0x0B, 0x0000
        jmp     .exit
  .not_2_or_3:
;       DBG_PRINT("tpc->mcfg=%d. Discard hw PHY config.\n", tpc->mcfg);
;        DEBUGF  1,"K :   tpc.mcfg=%d, discard hw PHY config\n",[rtl8169_tpc.mcfg]
  .exit:
        ret
endp

;proc pci_write_config_byte
;       ret
;endp

proc RTL8169_WRITE_GMII_REG,RegAddr:byte,value:dword

;;;     DEBUGF  1,"K : RTL8169_WRITE_GMII_REG: 0x%x 0x%x\n",[RegAddr]:2,[value]

        movzx   eax, [RegAddr]
        shl     eax, 16
        or      eax, [value]
        or      eax, 0x80000000
        RTL_W32 RTL8169_REG_PHYAR,eax
        stdcall udelay, 1       ;;;1000

        mov     ecx, 2000
        ; Check if the RTL8169 has completed writing to the specified MII register
    @@:
        RTL_R32 RTL8169_REG_PHYAR
        test    eax, 0x80000000
        jz      .exit
        stdcall udelay, 1       ;;;100
        loop    @b
  .exit:
        ret
endp

proc RTL8169_READ_GMII_REG,RegAddr:byte

;;;     DEBUGF  1,"K : RTL8169_READ_GMII_REG: 0x%x\n",[RegAddr]:2

        push    ecx
        movzx   eax, [RegAddr]
        shl     eax, 16
;       or      eax,0x0
        RTL_W32 RTL8169_REG_PHYAR,eax
        stdcall udelay, 1       ;;;1000

        mov     ecx, 2000
        ; Check if the RTL8169 has completed retrieving data from the specified MII register
    @@:
        RTL_R32 RTL8169_REG_PHYAR
        test    eax, 0x80000000
        jnz     .exit
        stdcall udelay, 1       ;;;100
        loop    @b

        or      eax, -1
        pop     ecx
        ret
  .exit:
        RTL_R32 RTL8169_REG_PHYAR
        and     eax, 0xFFFF
        pop     ecx
        ret
endp

proc rtl8169_set_rx_mode

;        DEBUGF  1,"K : rtl8169_set_rx_mode\n"

        ; IFF_ALLMULTI
        ; Too many to filter perfectly -- accept all multicasts
        RTL_R32 RTL8169_REG_RxConfig
        mov     ecx, [rtl8169_tpc.chipset]
        and     eax, [rtl_chip_info + ecx * 8 + 4]; RxConfigMask
        or      eax, rtl8169_rx_config or (RTL8169_RXM_AcceptBroadcast or RTL8169_RXM_AcceptMulticast or RTL8169_RXM_AcceptMyPhys)
        RTL_W32 RTL8169_REG_RxConfig,eax

        ; Multicast hash filter
        RTL_W32 RTL8169_REG_MAR0 + 0,0xffffffff
        RTL_W32 RTL8169_REG_MAR0 + 4,0xffffffff
        ret
endp

proc rtl8169_init_ring

;        DEBUGF  1,"K : rtl8169_init_ring\n"

        xor     eax, eax
        mov     [rtl8169_tpc.cur_rx], eax
        mov     [rtl8169_tpc.cur_tx], eax

        mov     edi, [rtl8169_tpc.TxDescArray]
        mov     ecx, (NUM_TX_DESC * sizeof.rtl8169_TxDesc) / 4
        cld
        rep stosd
        mov     edi, [rtl8169_tpc.RxDescArray]
        mov     ecx, (NUM_RX_DESC * sizeof.rtl8169_RxDesc) / 4
        rep stosd

        mov     edi, rtl8169_tpc.Tx_skbuff
        mov     eax, rtl8169_txb
        mov     ecx, NUM_TX_DESC
    @@:
        stosd
        inc     eax           ; add eax,RX_BUF_SIZE ???
        loop    @b

;!!!    for (i = 0; i < NUM_RX_DESC; i++) {
;!!!            if (i == (NUM_RX_DESC - 1))
;!!!                    tpc->RxDescArray[i].status = (OWNbit | EORbit) | RX_BUF_SIZE;
;!!!            else
;!!!                    tpc->RxDescArray[i].status = OWNbit | RX_BUF_SIZE;
;!!!            tpc->RxBufferRing[i] = &rxb[i * RX_BUF_SIZE];
;!!!            tpc->RxDescArray[i].buf_addr = virt_to_bus(tpc->RxBufferRing[i]);
;!!!    }
        mov     esi, rtl8169_tpc.RxBufferRing
        mov     edi, [rtl8169_tpc.RxDescArray]
        mov     eax, rtl8169_rxb
        mov     ecx, NUM_RX_DESC
    @@:
        mov     [esi], eax
        mov     [edi+rtl8169_RxDesc.buf_addr], eax
        sub     [edi+rtl8169_RxDesc.buf_addr], OS_BASE                          ; shurf 28.09.2008
        mov     [edi+rtl8169_RxDesc.status], RTL8169_DSB_OWNbit or RX_BUF_SIZE
        add     esi, 4
        add     edi, sizeof.rtl8169_RxDesc
        add     eax, RX_BUF_SIZE
        loop    @b

        or      [edi - sizeof.rtl8169_RxDesc + rtl8169_RxDesc.status], RTL8169_DSB_EORbit

        ret
endp

proc rtl8169_hw_start

;        DEBUGF  1,"K : rtl8169_hw_start\n"

        ; Soft reset the chip
        RTL_W8  RTL8169_REG_ChipCmd,RTL8169_CMD_Reset
        ; Check that the chip has finished the reset
        mov     ecx, 1000
    @@:
        RTL_R8  RTL8169_REG_ChipCmd
        and     al, RTL8169_CMD_Reset
        jz      @f
        stdcall udelay, 10
        loop    @b
    @@:
        RTL_W8  RTL8169_REG_Cfg9346,RTL8169_CFG_9346_Unlock
        RTL_W8  RTL8169_REG_ChipCmd,RTL8169_CMD_TxEnb or RTL8169_CMD_RxEnb
        RTL_W8  RTL8169_REG_ETThReg,ETTh
        ; For gigabit rtl8169
        RTL_W16 RTL8169_REG_RxMaxSize,RxPacketMaxSize
        ; Set Rx Config register
        RTL_R32 RTL8169_REG_RxConfig
        mov     ecx, [rtl8169_tpc.chipset]
        and     eax, [rtl_chip_info + ecx * 8 + 4]; RxConfigMask
        or      eax, rtl8169_rx_config
        RTL_W32 RTL8169_REG_RxConfig,eax
        ; Set DMA burst size and Interframe Gap Time
        RTL_W32 RTL8169_REG_TxConfig,(TX_DMA_BURST shl RTL8169_TXC_DMAShift) or (InterFrameGap shl RTL8169_TXC_InterFrameGapShift)
        RTL_R16 RTL8169_REG_CPlusCmd
        RTL_W16 RTL8169_REG_CPlusCmd,ax

        RTL_R16 RTL8169_REG_CPlusCmd
        or      ax, 1 shl 3
        cmp     [rtl8169_tpc.mcfg], MCFG_METHOD_02
        jne     @f
        cmp     [rtl8169_tpc.mcfg], MCFG_METHOD_03
        jne     @f
        or      ax, 1 shl 14
;       DEBUGF  1,"K :   Set MAC Reg C+CR Offset 0xE0: bit-3 and bit-14\n"
        jmp     .set
    @@:;DEBUGF  1,"K :   Set MAC Reg C+CR Offset 0xE0: bit-3\n"
  .set:
        RTL_W16 RTL8169_REG_CPlusCmd,ax

;       RTL_W16 0xE2,0x1517
;       RTL_W16 0xE2,0x152a
;       RTL_W16 0xE2,0x282a
        RTL_W16 0xE2,0x0000

        MOV     [rtl8169_tpc.cur_rx],0
        push    eax                                                             ; shurf 28.09.2008
        mov     eax, [rtl8169_tpc.TxDescArray]                                  ; shurf 28.09.2008
        sub     eax, OS_BASE                                                    ; shurf 28.09.2008
        RTL_W32 RTL8169_REG_TxDescStartAddr,eax ;[rtl8169_tpc.TxDescArray]      ; shurf 28.09.2008
        mov     eax, [rtl8169_tpc.RxDescArray]                                  ; shurf 28.09.2008
        sub     eax, OS_BASE                                                    ; shurf 28.09.2008
        RTL_W32 RTL8169_REG_RxDescStartAddr,eax ;[rtl8169_tpc.RxDescArray]      ; shurf 28.09.2008
        pop     eax                                                             ; shurf 28.09.2008
        RTL_W8  RTL8169_REG_Cfg9346,RTL8169_CFG_9346_Lock
        stdcall udelay, 10
        RTL_W32 RTL8169_REG_RxMissed,0
        call    rtl8169_set_rx_mode
        ; no early-rx interrupts
        RTL_R16 RTL8169_REG_MultiIntr
        and     ax, 0xF000
        RTL_W16 RTL8169_REG_MultiIntr,ax
        RTL_W16 RTL8169_REG_IntrMask,0 ; rtl8169_intr_mask
        ret
endp

proc udelay,msec:dword
        push    esi
        mov     esi, [msec]
        call    delay_ms
        pop     esi
        ret
endp

;***************************************************************************
;   Function
;      rtl8169_probe
;   Description
;      Searches for an ethernet card, enables it and clears the rx buffer
;      If a card was found, it enables the ethernet -> TCPIP link
;   Destroyed registers
;      eax, ebx, ecx, edx
;
;***************************************************************************
proc rtl8169_probe

;        DEBUGF  1,"K : rtl8169_probe: 0x%x : 0x%x 0x%x\n",[io_addr]:8,[pci_bus]:2,[pci_dev]:2

        call    rtl8169_init_board

        mov     ecx, MAC_ADDR_LEN
        mov     edx, [rtl8169_tpc.mmio_addr]
        add     edx, RTL8169_REG_MAC0
        xor     ebx, ebx
        ; Get MAC address. FIXME: read EEPROM
    @@:
        RTL_R8  dx
        mov     [node_addr+ebx], al
        inc     edx
        inc     ebx
        loop    @b

;        DEBUGF  1,"K : rtl8169_probe: MAC = %x-%x-%x-%x-%x-%x\n",[node_addr+0]:2,[node_addr+1]:2,[node_addr+2]:2,[node_addr+3]:2,[node_addr+4]:2,[node_addr+5]:2

        ; Config PHY
        stdcall rtl8169_hw_PHY_config
;       DEBUGF  1,"K :   Set MAC Reg C+CR Offset 0x82h = 0x01h\n"
        RTL_W8  0x82,0x01
        cmp     [rtl8169_tpc.mcfg], MCFG_METHOD_03
        jae     @f
;       DEBUGF  1,"K :   Set PCI Latency=0x40\n"
;       stdcall pci_write_config_byte,PCI_LATENCY_TIMER,0x40
   @@:
        cmp     [rtl8169_tpc.mcfg], MCFG_METHOD_02
        jne     @f
;       DEBUGF  1,"K :   Set MAC Reg C+CR Offset 0x82h = 0x01h\n"
        RTL_W8  0x82,0x01
;       DEBUGF  1,"K :   Set PHY Reg 0x0bh = 0x00h\n"
        stdcall RTL8169_WRITE_GMII_REG, 0x0b, 0x0000    ; w 0x0b 15 0 0
    @@:
        ; if TBI is not enabled
        RTL_R8  RTL8169_REG_PHYstatus
        test    al, RTL8169_PHYS_TBI_Enable
        jz      .tbi_dis
        stdcall RTL8169_READ_GMII_REG, RTL8169_PHY_AUTO_NEGO_REG
        ; enable 10/100 Full/Half Mode, leave PHY_AUTO_NEGO_REG bit4:0 unchanged
        and     eax, 0x0C1F
        or      eax, RTL8169_PHY_Cap_10_Half or RTL8169_PHY_Cap_10_Full or RTL8169_PHY_Cap_100_Half or RTL8169_PHY_Cap_100_Full
        stdcall RTL8169_WRITE_GMII_REG, RTL8169_PHY_AUTO_NEGO_REG, eax
        ; enable 1000 Full Mode
        stdcall RTL8169_WRITE_GMII_REG, RTL8169_PHY_1000_CTRL_REG, RTL8169_PHY_Cap_1000_Full or RTL8169_PHY_Cap_1000_Half; rtl8168
        ; Enable auto-negotiation and restart auto-nigotiation
        stdcall RTL8169_WRITE_GMII_REG, RTL8169_PHY_CTRL_REG, RTL8169_PHY_Enable_Auto_Nego or RTL8169_PHY_Restart_Auto_Nego
        stdcall udelay, 100
        mov     ecx, 10000
        ; wait for auto-negotiation process
    @@:
        dec     ecx
        jz      @f
        stdcall RTL8169_READ_GMII_REG, RTL8169_PHY_STAT_REG
        stdcall udelay, 100
        test    eax, RTL8169_PHY_Auto_Neco_Comp
        jz      @b
        RTL_R8  RTL8169_REG_PHYstatus
        jmp     @f
  .tbi_dis:
        stdcall udelay, 100
    @@:
        call    rtl8169_reset
        ret
endp

;***************************************************************************
;   Function
;      rt8169_reset
;   Description
;      Place the chip (ie, the ethernet card) into a virgin state
;   Destroyed registers
;      eax, ebx, ecx, edx
;
;***************************************************************************
proc rtl8169_reset

;        DEBUGF  1,"K : rtl8169_reset: 0x%x : 0x%x 0x%x\n",[io_addr]:8,[pci_bus]:2,[pci_dev]:2

        mov     [rtl8169_tpc.TxDescArrays], rtl8169_tx_ring
        ; Tx Desscriptor needs 256 bytes alignment
        mov     [rtl8169_tpc.TxDescArray], rtl8169_tx_ring

        mov     [rtl8169_tpc.RxDescArrays], rtl8169_rx_ring
        ; Rx Desscriptor needs 256 bytes alignment
        mov     [rtl8169_tpc.RxDescArray], rtl8169_rx_ring

        call    rtl8169_init_ring
        call    rtl8169_hw_start
        ; Construct a perfect filter frame with the mac address as first match
        ; and broadcast for all others
        mov     edi, rtl8169_txb
        or      al, -1
        mov     ecx, 192
        cld
        rep stosb

        mov     esi, node_addr
        mov     edi, rtl8169_txb
        movsd
        movsw

        mov     eax, [pci_data]
        mov     [eth_status], eax
        ret
endp

;***************************************************************************
;   Function
;      rtl8169_transmit
;   Description
;      Transmits a packet of data via the ethernet card
;         d - edi - Pointer to 48 bit destination address
;         t -  bx - Type of packet
;         s - ecx - size of packet
;         p - esi - pointer to packet data
;   Destroyed registers
;      eax, edx, esi, edi
;
;***************************************************************************
proc rtl8169_transmit

;        DEBUGF  1,"K : rtl8169_transmit\n" ;: 0x%x : 0x%x 0x%x 0x%x 0x%x\n",[io_addr]:8,edi,bx,ecx,esi

        push    ecx edx esi
        mov     eax, MAX_ETH_FRAME_SIZE
        mul     [rtl8169_tpc.cur_tx]
        mov     esi, edi
        ; point to the current txb incase multiple tx_rings are used
        mov     edi, [rtl8169_tpc.Tx_skbuff + eax * 4]
        mov     eax, edi
        cld
; copy destination address
        movsd
        movsw
; copy source address
        mov     esi, node_addr
        movsd
        movsw
; copy packet type
        mov     [edi], bx
        add     edi, 2
; copy the packet data
        pop     esi edx ecx
        push    ecx
        shr     ecx, 2
        rep movsd
        pop     ecx
        push    ecx
        and     ecx, 3
        rep movsb

;!!!    s += ETH_HLEN;
;!!!    s &= 0x0FFF;
;!!!    while (s < ETH_ZLEN)
;!!!            ptxb[s++] = '\0';
        mov     edi, eax
        pop     ecx
        push    eax
        add     ecx, ETH_HLEN
        and     ecx, 0x0FFF
        xor     al, al
        add     edi, ecx
    @@:
        cmp     ecx, ETH_ZLEN
        jae     @f
        stosb
        inc     ecx
        jmp     @b
    @@:
        pop     eax

        mov     ebx, eax
        mov     eax, sizeof.rtl8169_TxDesc
        mul     [rtl8169_tpc.cur_tx]
        add     eax, [rtl8169_tpc.TxDescArray]
        xchg    eax, ebx
        mov     [ebx + rtl8169_TxDesc.buf_addr], eax
        sub     [ebx + rtl8169_TxDesc.buf_addr], OS_BASE                                ; shurf 28.09.2008

        mov     eax, ecx
        cmp     eax, ETH_ZLEN
        jae     @f
        mov     eax, ETH_ZLEN
    @@:
        or      eax, RTL8169_DSB_OWNbit or RTL8169_DSB_FSbit or RTL8169_DSB_LSbit
        cmp     [rtl8169_tpc.cur_tx], NUM_TX_DESC - 1
        jne     @f
        or      eax, RTL8169_DSB_EORbit
    @@:
        mov     [ebx + rtl8169_TxDesc.status], eax

        RTL_W8  RTL8169_REG_TxPoll,0x40     ; set polling bit

        inc     [rtl8169_tpc.cur_tx]
        and     [rtl8169_tpc.cur_tx], NUM_TX_DESC - 1

;!!!    to = currticks() + TX_TIMEOUT;
;!!!    while ((tpc->TxDescArray[entry].status & OWNbit) && (currticks() < to));        /* wait */
        mov     ecx, TX_TIMEOUT / 10
    @@:
        test    [ebx + rtl8169_TxDesc.status], RTL8169_DSB_OWNbit
        jnz     @f
        stdcall udelay, 10
        loop    @b
;        DEBUGF  1,"K : rtl8169_transmit: TX Time Out\n"
    @@:

        ret
endp

;***************************************************************************
; Function
;    rtl8169_poll
;
; Description
;    Polls the ethernet card for a received packet
;    Received data, if any, ends up in Ether_buffer
; Destroyed register(s)
;    eax, edx, ecx
;
;***************************************************************************
proc rtl8169_poll

;       DEBUGF  1,"K : rtl8169_poll\n" ;: 0x%x : none\n",[io_addr]:8

        mov     word[eth_rx_data_len], 0

        mov     eax, sizeof.rtl8169_RxDesc
        mul     [rtl8169_tpc.cur_rx]
        add     eax, [rtl8169_tpc.RxDescArray]
        mov     ebx, eax

;       DEBUGF  1,"K :   rtl8169_RxDesc.status = 0x%x\n",[ebx + rtl8169_RxDesc.status]

        test    [ebx + rtl8169_RxDesc.status], RTL8169_DSB_OWNbit; 0x80000600
        jnz     .exit

;       DEBUGF  1,"K :   rtl8169_tpc.cur_rx = %u\n",[rtl8169_tpc.cur_rx]

        ; h/w no longer present (hotplug?) or major error, bail
        RTL_R16 RTL8169_REG_IntrStatus

;       DEBUGF  1,"K :   IntrStatus = 0x%x\n",ax

        cmp     ax, 0xFFFF
        je      .exit

        push    eax
        and     ax, not (RTL8169_ISB_RxFIFOOver or RTL8169_ISB_RxOverflow or RTL8169_ISB_RxOK)
        RTL_W16 RTL8169_REG_IntrStatus,ax

        mov     eax, [ebx + rtl8169_RxDesc.status]

;       DEBUGF  1,"K :   RxDesc.status = 0x%x\n",eax

        test    eax, RTL8169_SD_RxRES
        jnz     .else
        and     eax, 0x00001FFF
;       jz      .exit.pop
        add     eax, -4
        mov     [eth_rx_data_len], ax

;        DEBUGF  1,"K : rtl8169_poll: data length = %u\n",ax

        push    eax
        mov     ecx, eax
        shr     ecx, 2
        mov     eax, [rtl8169_tpc.cur_rx]
        mov     edx, [rtl8169_tpc.RxBufferRing + eax * 4]
        mov     esi, edx
        mov     edi, Ether_buffer
        cld
        rep movsd
        pop     ecx
        and     ecx, 3
        rep movsb

        mov     eax, RTL8169_DSB_OWNbit or RX_BUF_SIZE
        cmp     [rtl8169_tpc.cur_rx], NUM_RX_DESC - 1
        jne     @f
        or      eax, RTL8169_DSB_EORbit
    @@:
        mov     [ebx + rtl8169_RxDesc.status], eax

        mov     [ebx + rtl8169_RxDesc.buf_addr], edx
        sub     [ebx + rtl8169_RxDesc.buf_addr], OS_BASE                                ; shurf 28.09.2008
        jmp     @f
  .else:
;        DEBUGF  1,"K : rtl8169_poll: Rx Error\n"
        ; FIXME: shouldn't I reset the status on an error
    @@:
        inc     [rtl8169_tpc.cur_rx]
        and     [rtl8169_tpc.cur_rx], NUM_RX_DESC - 1
  .exit.pop:
        pop     eax
        and     ax, RTL8169_ISB_RxFIFOOver or RTL8169_ISB_RxOverflow or RTL8169_ISB_RxOK
        RTL_W16 RTL8169_REG_IntrStatus,ax
  .exit:
        ret
endp

proc rtl8169_cable
        ret
endp
