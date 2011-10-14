;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  R6040 driver for KolibriOS                                     ;;
;;                                                                 ;;
;;  based on R6040.c from linux                                    ;;
;;                                                                 ;;
;;    Written by Asper (asper.85@mail.ru)                          ;;
;;            and hidnplayr (hidnplayr@gmail.com)                  ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



;********************************************************************
;   Interface
;      r6040_reset
;      r6040_probe
;      r6040_poll
;      r6040_transmit
;
;      These functions are referenced in ethernet.inc
;
;********************************************************************

;; A few user-configurable values.

TX_RING_SIZE    equ 4
RX_RING_SIZE    equ 4

; ethernet address length
ETH_ALEN        equ 6
ETH_HLEN        equ (2 * ETH_ALEN + 2)
ETH_ZLEN        equ     60      ; 60 + 4bytes auto payload for
                                ; mininmum 64bytes frame length
; system timer frequency
HZ              equ 1000

; max time out delay time
W_MAX_TIMEOUT   equ 0x0FFF

;; Size of the in-memory receive ring.
RX_BUF_LEN_IDX  equ 3       ;; 0==8K, 1==16K, 2==32K, 3==64K
RX_BUF_LEN      equ (8192 << RX_BUF_LEN_IDX)

;-; Size of the Tx bounce buffers -- must be at least (dev->mtu+14+4).
;-TX_BUF_SIZE     equ 1536
;-RX_BUF_SIZE     equ 1536

;; PCI Tuning Parameters
;   Threshold is bytes transferred to chip before transmission starts.
TX_FIFO_THRESH  equ 256     ;; In bytes, rounded down to 32 byte units.

;; The following settings are log_2(bytes)-4:  0 == 16 bytes .. 6==1024.
RX_FIFO_THRESH  equ 4       ;; Rx buffer level before first PCI xfer.
RX_DMA_BURST    equ 4       ;; Maximum PCI burst, '4' is 256 bytes
TX_DMA_BURST    equ 4

;; Operational parameters that usually are not changed.
PHY1_ADDR       equ 1       ;For MAC1
PHY2_ADDR       equ 3       ;For MAC2
PHY_MODE        equ 0x3100  ;PHY CHIP Register 0
PHY_CAP         equ 0x01E1  ;PHY CHIP Register 4

;; Time in jiffies before concluding the transmitter is hung.
TX_TIMEOUT      equ ((6000*HZ)/1000)

R6040_IO_SIZE   equ 256     ; RDC MAC I/O Size
MAX_MAC         equ 2       ; MAX RDC MAC

;**************************************************************************
; RDC R6040 Register Definitions
;**************************************************************************
MCR0            equ 0x00    ;Control register 0
MCR1            equ 0x01    ;Control register 1
MAC_RST         equ 0x0001  ;Reset the MAC
MBCR            equ 0x08    ;Bus control
MT_ICR          equ 0x0C    ;TX interrupt control
MR_ICR          equ 0x10    ;RX interrupt control
MTPR            equ 0x14    ;TX poll command register
MR_BSR          equ 0x18    ;RX buffer size
MR_DCR          equ 0x1A    ;RX descriptor control
MLSR            equ 0x1C    ;Last status
MMDIO           equ 0x20    ;MDIO control register
MDIO_WRITE      equ 0x4000  ;MDIO write
MDIO_READ       equ 0x2000  ;MDIO read
MMRD            equ 0x24    ;MDIO read data register
MMWD            equ 0x28    ;MDIO write data register
MTD_SA0         equ 0x2C    ;TX descriptor start address 0
MTD_SA1         equ 0x30    ;TX descriptor start address 1
MRD_SA0         equ 0x34    ;RX descriptor start address 0
MRD_SA1         equ 0x38    ;RX descriptor start address 1
MISR            equ 0x3C    ;Status register
MIER            equ 0x40    ;INT enable register
MSK_INT         equ 0x0000  ;Mask off interrupts
RX_FINISH       equ 0x0001  ;RX finished
RX_NO_DESC      equ 0x0002  ;No RX descriptor available
RX_FIFO_FULL    equ 0x0004  ;RX FIFO full
RX_EARLY        equ 0x0008  ;RX early
TX_FINISH       equ 0x0010  ;TX finished
TX_EARLY        equ 0x0080  ;TX early
EVENT_OVRFL     equ 0x0100  ;Event counter overflow
LINK_CHANGED    equ 0x0200  ;PHY link changed
ME_CISR         equ 0x44    ;Event counter INT status
ME_CIER         equ 0x48    ;Event counter INT enable
MR_CNT          equ 0x50    ;Successfully received packet counter
ME_CNT0         equ 0x52    ;Event counter 0
ME_CNT1         equ 0x54    ;Event counter 1
ME_CNT2         equ 0x56    ;Event counter 2
ME_CNT3         equ 0x58    ;Event counter 3
MT_CNT          equ 0x5A    ;Successfully transmit packet counter
ME_CNT4         equ 0x5C    ;Event counter 4
MP_CNT          equ 0x5E    ;Pause frame counter register
MAR0            equ 0x60    ;Hash table 0
MAR1            equ 0x62    ;Hash table 1
MAR2            equ 0x64    ;Hash table 2
MAR3            equ 0x66    ;Hash table 3
MID_0L          equ 0x68    ;Multicast address MID0 Low
MID_0M          equ 0x6A    ;Multicast address MID0 Medium
MID_0H          equ 0x6C    ;Multicast address MID0 High
MID_1L          equ 0x70    ;MID1 Low
MID_1M          equ 0x72    ;MID1 Medium
MID_1H          equ 0x74    ;MID1 High
MID_2L          equ 0x78    ;MID2 Low
MID_2M          equ 0x7A    ;MID2 Medium
MID_2H          equ 0x7C    ;MID2 High
MID_3L          equ 0x80    ;MID3 Low
MID_3M          equ 0x82    ;MID3 Medium
MID_3H          equ 0x84    ;MID3 High
PHY_CC          equ 0x88    ;PHY status change configuration register
PHY_ST          equ 0x8A    ;PHY status register
MAC_SM          equ 0xAC    ;MAC status machine
MAC_ID          equ 0xBE    ;Identifier register

MAX_BUF_SIZE    equ 0x600   ;1536

MBCR_DEFAULT    equ 0x012A  ;MAC Bus Control Register
MCAST_MAX       equ 3       ;Max number multicast addresses to filter

;Descriptor status
DSC_OWNER_MAC   equ 0x8000  ;MAC is the owner of this descriptor
DSC_RX_OK       equ 0x4000  ;RX was successfull
DSC_RX_ERR      equ 0x0800  ;RX PHY error
DSC_RX_ERR_DRI  equ 0x0400  ;RX dribble packet
DSC_RX_ERR_BUF  equ 0x0200  ;RX length exceeds buffer size
DSC_RX_ERR_LONG equ 0x0100  ;RX length > maximum packet length
DSC_RX_ERR_RUNT equ 0x0080  ;RX packet length < 64 byte
DSC_RX_ERR_CRC  equ 0x0040  ;RX CRC error
DSC_RX_BCAST    equ 0x0020  ;RX broadcast (no error)
DSC_RX_MCAST    equ 0x0010  ;RX multicast (no error)
DSC_RX_MCH_HIT  equ 0x0008  ;RX multicast hit in hash table (no error)
DSC_RX_MIDH_HIT equ 0x0004  ;RX MID table hit (no error)
DSC_RX_IDX_MID_MASK  equ 3  ;RX mask for the index of matched MIDx

;PHY settings
ICPLUS_PHY_ID   equ 0x0243

RX_INTS         equ RX_FIFO_FULL or RX_NO_DESC or RX_FINISH
TX_INTS         equ TX_FINISH
INT_MASK        equ RX_INTS or TX_INTS


r6040_txb     equ (eth_data_start)
r6040_rxb     equ ((r6040_txb+(MAX_BUF_SIZE*TX_RING_SIZE)+32) and 0xfffffff0)
r6040_tx_ring equ ((r6040_rxb+(MAX_BUF_SIZE*RX_RING_SIZE)+32) and 0xfffffff0)
r6040_rx_ring equ ((r6040_tx_ring+(r6040_x_head.sizeof*TX_RING_SIZE)+32) and 0xfffffff0)

virtual at ((r6040_rx_ring+(r6040_x_head.sizeof*RX_RING_SIZE)+32) and 0xfffffff0)
r6040_private:
  .rx_ring                dd ?
  .tx_ring                dd ?
  .cur_rx                 dw ?
  .cur_tx                 dw ?
  .phy_addr               dw ?
  .phy_mode               dw ?
  .mcr0                   dw ?
  .mcr1                   dw ?
  .switch_sig             dw ?
end virtual

virtual at 0
r6040_x_head:
  .status         dw ?   ;0-1
  .len            dw ?   ;2-3
  .buf            dd ?   ;4-7
  .ndesc          dd ?   ;8-B
  .rev1           dd ?   ;C-F
  .vbufp          dd ?   ;10-13
  .vndescp        dd ?   ;14-17
  .skb_ptr        dd ?   ;18-1B
  .rev2           dd ?   ;1C-1F
  .sizeof:
end virtual



; Read a word data from PHY Chip
proc  r6040_phy_read stdcall, phy_addr:dword, reg:dword
        push    ecx edx
        mov     eax, [phy_addr]
        shl     eax, 8
        add     eax, [reg]
        add     eax, MDIO_READ
        mov     edx, [io_addr]
        add     edx, MMDIO
        out     dx, ax
        ;Wait for the read bit to be cleared.
        mov     ecx, 2048 ;limit
        xor     eax, eax
  .read:
        in      ax, dx
        test    ax, MDIO_READ
        jz      @f
        dec     ecx
        test    ecx, ecx
        jnz     .read
  @@:
        mov     edx, [io_addr]
        add     edx, MMRD
        in      ax, dx
        and     eax, 0xFFFF
        pop     edx ecx
        ret
endp

; Write a word data to PHY Chip
proc  r6040_phy_write stdcall, phy_addr:dword, reg:dword, val:dword
        push    eax ecx edx
        mov     eax, [val]
        mov     edx, [io_addr]
        add     edx, MMWD
        out     dx, ax
        ;Write the command to the MDIO bus
        mov     eax, [phy_addr]
        shl     eax, 8
        add     eax, [reg]
        add     eax, MDIO_WRITE
        mov     edx, [io_addr]
        add     edx, MMDIO
        out     dx, ax
        ;Wait for the write bit to be cleared.
        mov     ecx, 2048 ;limit
        xor     eax, eax
  .write:
        in      ax, dx
        test    ax, MDIO_WRITE
        jz      @f
        dec     ecx
        test    ecx, ecx
        jnz     .write
  @@:
        pop     edx ecx eax
        ret
endp

macro r6040_mdio_write reg, val {
        stdcall r6040_phy_read, [io_addr], [r6040_private.phy_addr], reg
}

macro r6040_mdio_write reg, val {
        stdcall r6040_phy_write, [io_addr], [r6040_private.phy_addr], reg, val
}


proc r6040_init_ring_desc stdcall, desc_ring:dword, size:dword
        push    eax ecx esi
        mov     ecx, [size]
        test    ecx, ecx
        jz      .out

        mov     esi, [desc_ring]
        mov     eax, esi
    .next_desc:
        add     eax, r6040_x_head.sizeof - OS_BASE
        mov     [esi+r6040_x_head.ndesc], eax
        add     eax, OS_BASE
        mov     [esi+r6040_x_head.vndescp], eax
        mov     esi, eax
        dec     ecx
        jnz     .next_desc

        sub     esi, r6040_x_head.sizeof
        mov     eax, [desc_ring]
        mov     [esi+r6040_x_head.vndescp], eax
        sub     eax, OS_BASE
        mov     [esi+r6040_x_head.ndesc], eax
    .out:
        pop     esi ecx eax
        ret
endp




r6040_init_rxbufs:

        stdcall r6040_init_ring_desc, r6040_rx_ring, RX_RING_SIZE

        ; Allocate skbs for the rx descriptors
        mov     esi, r6040_rx_ring
        mov     ebx, r6040_rxb
        mov     ecx, RX_RING_SIZE
        mov     eax, r6040_rx_ring
    .next_desc:
        mov     [esi+r6040_x_head.skb_ptr], ebx
        mov     [esi+r6040_x_head.buf], ebx
        sub     [esi+r6040_x_head.buf], OS_BASE
        mov     [esi+r6040_x_head.status], DSC_OWNER_MAC

        mov     eax, [esi+r6040_x_head.vndescp]
        mov     esi, eax

        add     ebx, MAX_BUF_SIZE
        dec     ecx
        jnz     .next_desc

        xor     eax, eax
    .out:

        ret


r6040_probe:

        DEBUGF  1, "Probing r6040\n"

        call    adjust_pci_device


        ; If PHY status change register is still set to zero
        ; it means the bootloader didn't initialize it
        mov     edx, [io_addr]
        add     edx, PHY_CC
        in      ax, dx
        test    ax, ax
        jnz     @f
        mov     eax, 0x9F07
        out     dx, ax
     @@:
        ; Set MAC address
        mov     ecx, 3
        mov     edi, node_addr
        mov     edx, [io_addr]
        add     edx, MID_0L
     .mac:
        in      ax, dx
        stosw
        add     edx, 2
        dec     ecx
        jnz     .mac
        ; Some bootloaders/BIOSes do not initialize
        ; MAC address, warn about that
        and     eax, 0xFF
        or      eax, [node_addr]
        test    eax, eax
        jnz     @f
        DEBUGF  1, "K : MAC address not initialized\n" ;, generating random"
        ;Asper: Add here generate function call!
        ;       Temporary workaround: init by constant adress
        mov     dword [node_addr], 0x00006000
        mov     word [node_addr+4], 0x0001
     @@:
        ; Init RDC private data
        mov     [r6040_private.mcr0], 0x1002
        ;mov     [r6040_private.phy_addr], 1 ; Asper: Only one network card is supported now.
        mov     [r6040_private.switch_sig], 0

        ; Check the vendor ID on the PHY, if 0xFFFF assume none attached
        stdcall r6040_phy_read, 1, 2
        cmp     ax, 0xFFFF
        jne     @f
        DEBUGF  1, "K : Failed to detect an attached PHY\n" ;, generating random"
        mov     eax, -1
        ret
     @@:

        ; Set MAC address
        call    r6040_mac_address


        ; Initialize and alloc RX/TX buffers
        stdcall r6040_init_ring_desc, r6040_tx_ring, TX_RING_SIZE
        call    r6040_init_rxbufs ;r6040_alloc_rxbufs
        test    eax, eax
        jnz     .out

        ; Read the PHY ID
        mov     [r6040_private.phy_mode], 0x8000
        stdcall r6040_phy_read, 0, 2
        mov     [r6040_private.switch_sig], ax
        cmp     ax, ICPLUS_PHY_ID
        jne     @f
        stdcall r6040_phy_write, 29, 31, 0x175C ; Enable registers
        jmp     .phy_readen
      @@:

        ; PHY Mode Check
        movzx   eax, [r6040_private.phy_addr]
        stdcall r6040_phy_write, eax, 4, PHY_CAP
        stdcall r6040_phy_write, eax, 0, PHY_MODE
;      if PHY_MODE = 0x3100
        call    r6040_phy_mode_chk
        mov     [r6040_private.phy_mode], ax
        jmp     .phy_readen
;      end if
;      if not (PHY_MODE and 0x0100)
        mov     [r6040_private.phy_mode], 0
;      end if
      .phy_readen:

        ; Set duplex mode
        mov     ax, [r6040_private.phy_mode]
        or      [r6040_private.mcr0], ax

        ; improve performance (by RDC guys)
        stdcall r6040_phy_read, 30, 17
        or      ax, 0x4000
        stdcall r6040_phy_write, 30, 17, eax

        stdcall r6040_phy_read, 30, 17
        xor     ax, -1
        or      ax, 0x2000
        xor     ax, -1
        stdcall r6040_phy_write, 30, 17, eax

        stdcall r6040_phy_write, 0, 19, 0x0000
        stdcall r6040_phy_write, 0, 30, 0x01F0

        ; Initialize all Mac registers
        call    r6040_reset

        xor     eax, eax
      .out:
        ret






align 4
r6040_reset:

        DEBUGF  1, "Resetting r6040\n"

        push    eax ecx edx
        ; Mask off Interrupt
        mov     eax, MSK_INT
        mov     edx, [io_addr]
        add     edx, MIER
        out     dx, ax

        ;Reset RDC MAC
        mov     eax, MAC_RST
        mov     edx, [io_addr]
        add     edx, MCR1
        out     dx, ax

        mov     ecx, 2048 ;limit
  .read:
        in      ax, dx
        test    ax, 0x1
        jnz     @f
        dec     ecx
        test    ecx, ecx
        jnz     .read
  @@:
        ;Reset internal state machine
        mov     ax, 2
        mov     edx, [io_addr]
        add     edx, MAC_SM
        out     dx, ax
        xor     ax, ax
        out     dx, ax
        mov     esi, 5
        call    delay_ms

        ;MAC Bus Control Register
        mov     ax, MBCR_DEFAULT
        mov     edx, [io_addr]
        add     edx, MBCR
        out     dx, ax

        ;Buffer Size Register
        mov     ax, MAX_BUF_SIZE
        mov     edx, [io_addr]
        add     edx, MR_BSR
        out     dx, ax

        ;Write TX ring start address
        mov     eax, r6040_tx_ring - OS_BASE  ;Asper: Maybe we can just write dword? Hidnplayr: better use word, as described in datasheet.
        mov     edx, [io_addr]
        add     edx, MTD_SA0
        out     dx, ax
        shr     eax, 16
        add     edx, MTD_SA1 - MTD_SA0
        out     dx, ax

        ;Write RX ring start address
        mov     eax, r6040_rx_ring - OS_BASE  ;Asper: Maybe we can just write dword?
        mov     edx, [io_addr]
        add     edx, MRD_SA0
        out     dx, ax
        shr     eax, 16
        add     edx, MRD_SA1 - MRD_SA0
        out     dx, ax

        ;Set interrupt waiting time and packet numbers
        xor     ax, ax
        mov     edx, [io_addr]
        add     edx, MT_ICR
        out     dx, ax

        ;Asper: ~ Disable ints ;Enable interrupts
        ;mov     ax, MSK_INT ;INT_MASK  ;Asper ~
        ;mov     edx, [io_addr]
        ;add     edx, MIER
        ;out     dx, ax

        ;Enable TX and RX
        mov     ax, [r6040_private.mcr0]
        or      ax, 0x0002
        mov     edx, [io_addr]
        out     dx, ax

        ;Let TX poll the descriptors
        ;we may got called by r6040_tx_timeout which has left
        ;some unset tx buffers
        xor     ax, ax
        inc     ax
        mov     edx, [io_addr]
        add     edx, MTPR
        out     dx, ax

        pop     edx ecx eax

        DEBUGF  1, "reset ok!\n"

        ; Indicate that we have successfully reset the card
        mov     eax, [pci_data]
        mov     [eth_status], eax
        ret



proc r6040_tx_timeout
        push    eax edx
        ;...
        inc     [stats.tx_errors]
        ;Reset MAC and re-init all registers
        call    r6040_init_mac_regs
        pop     edx eax
        ret
endp

proc  r6040_get_stats
        push    eax edx
        mov     edx, [io_addr]
        add     edx, ME_CNT1
        in      al, dx
        add     [stats.rx_crc_errors], al
        mov     edx, [io_addr]
        add     edx, ME_CNT0
        in      al, dx
        add     [stats.multicast], al
        pop     edx eax
        ret
endp

;...

proc r6040_phy_mode_chk
        push    ebx
        ;PHY Link Status Check
        movzx   eax, [r6040_private.phy_addr]
        stdcall r6040_phy_read, eax, 1
        test    eax, 0x4
        jnz     @f
        mov     eax, 0x8000 ;Link Failed, full duplex
  @@:
        ;PHY Chip Auto-Negotiation Status
        movzx   eax, [r6040_private.phy_addr]
        stdcall r6040_phy_read, eax, 1
        test    eax, 0x0020
        jz      .force_mode
        ;Auto Negotuiation Mode
        movzx   eax, [r6040_private.phy_addr]
        stdcall r6040_phy_read, eax, 5
        mov     ebx, eax
        movzx   eax, [r6040_private.phy_addr]
        stdcall r6040_phy_read, eax, 4
        and     eax, ebx
        test    eax, 0x140
        jz      .ret_0
        jmp     .ret_0x8000
  .force_mode:
        ;Force Mode
        movzx   eax, [r6040_private.phy_addr]
        stdcall r6040_phy_read, eax, 0
        test    eax, 0x100
        jz      .ret_0
  .ret_0x8000:
        mov     eax, 0x8000
        pop     ebx
        ret
  .ret_0:
        xor     eax, eax
        pop     ebx
        ret
endp



;***************************************************************************
;   Function
;      r6040_rx
;   Description
;      polls card to see if there is a packet waiting
;
;  Currently only supports one descriptor per packet, if packet is fragmented
;  between multiple descriptors you will lose part of the packet
;***************************************************************************
r6040_poll:
        push    ebx ecx esi edi

        xor     eax, eax
        mov     [eth_rx_data_len], ax

        movzx   eax, [r6040_private.cur_rx]
        mov     ebx, eax
        shl     ebx, 5

        mov     cx, [ebx+r6040_rx_ring+r6040_x_head.status] ; Read the descriptor status
        test    cx, DSC_OWNER_MAC
        jnz     .out

        test    cx, DSC_RX_ERR  ; Global error status set
        jz      .no_dsc_rx_err
        ;...
        jmp     .out

    .no_dsc_rx_err:
        ; Packet successfully received
        movzx   ecx, [ebx+r6040_rx_ring+r6040_x_head.len]
        and     ecx, 0xFFF
        sub     ecx, 4    ; Do not count the CRC
        mov     [eth_rx_data_len], cx
        mov     esi, [ebx+r6040_rx_ring+r6040_x_head.skb_ptr]

        push    ecx
        shr     ecx, 2
        mov     edi, Ether_buffer
        cld
        rep movsd
        pop     ecx
        and     ecx, 3
        rep movsb

        or      [ebx+r6040_rx_ring+r6040_x_head.status], DSC_OWNER_MAC

        inc     [r6040_private.cur_rx]
        and     [r6040_private.cur_rx], RX_RING_SIZE-1

        xor     eax, eax
    .out:
        pop     edi esi ecx ebx
        ret



;***************************************************************************
;   Function
;      r6040_transmit
;   Description
;      Transmits a packet of data via the ethernet card
;         Pointer to 48 bit destination address in edi
;         Type of packet in bx
;         size of packet in ecx
;         pointer to packet data in esi
;
;***************************************************************************
r6040_transmit:
        cmp     ecx, MAX_BUF_SIZE
        jg      .out  ; packet is too long

        push    edi esi ebx ecx

        movzx   eax, [r6040_private.cur_tx]
        shl     eax, 5

;        DEBUGF  1,"R6040: TX buffer status: 0x%x, eax=%u\n", [eax + r6040_tx_ring + r6040_x_head.status]:4, eax

        test    [r6040_tx_ring + eax + r6040_x_head.status], 0x8000  ; check if buffer is available
        jz      .l3

        push    ecx esi
        mov     ecx, [timer_ticks]
        add     ecx, 100
     .l2:
        test    [r6040_tx_ring + eax + r6040_x_head.status], 0x8000
        jz      .l5
        cmp     ecx, [timer_ticks]
        jb      .l4
        mov     esi, 10
        call    delay_ms
        jmp     .l2

     .l4:
        pop     esi ecx
        DEBUGF  1,"R6040: Send timeout\n"
        jmp     .out

     .l5:
        pop     esi ecx
     .l3:
        push    eax

        mov     esi, edi

; point to the current tx buffer
        movzx   edi, [r6040_private.cur_tx]
        imul    edi, MAX_BUF_SIZE
        add     edi, r6040_txb
        lea     eax, [edi - OS_BASE]            ; real buffer address in eax

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

        mov     esi, [esp+8+4]
        mov     ecx, [esp+4]
; copy the packet data
        push    ecx
        shr     ecx, 2
        rep movsd
        pop     ecx
        and     ecx, 3
        rep movsb

        pop     edi

        mov     ecx, [esp]
        add     ecx, ETH_HLEN
        cmp     cx, ETH_ZLEN
        jae     @f
        mov     cx, ETH_ZLEN
    @@:

        mov     [r6040_tx_ring + edi + r6040_x_head.len], cx
        mov     [r6040_tx_ring + edi + r6040_x_head.buf], eax
        mov     [r6040_tx_ring + edi + r6040_x_head.status], 0x8000

        ; Trigger the MAC to check the TX descriptor
        mov     ax, 0x01
        mov     edx, [io_addr]
        add     edx, MTPR
        out     dx, ax

        inc     [r6040_private.cur_tx]
        and     [r6040_private.cur_tx], TX_RING_SIZE-1
        xor     eax, eax

        pop     ecx ebx esi edi
    .out:
        ret



r6040_mac_address:
        push    eax ecx edx esi edi
        ; MAC operation register
        mov     ax, 1
        mov     edx, [io_addr]
        add     edx, MCR1
        out     dx, ax
        ; Reset MAC
        mov     ax, 2
        mov     edx, [io_addr]
        add     edx, MAC_SM
        out     dx, ax
        ; Reset internal state machine
        xor     ax, ax
        out     dx, ax
        mov     esi, 5
        call    delay_ms

        ; Restore MAC Address
        mov     ecx, 3
        mov     edi, node_addr
        mov     edx, [io_addr]
        add     edx, MID_0L
     .mac:
        in      ax, dx
        stosw
        add     edx, 2
        dec     ecx
        jnz     .mac

        pop     edi esi edx ecx eax
        ret

