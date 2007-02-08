;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;;  RTL8139.INC                                                    ;;
;;                                                                 ;;
;;  Ethernet driver for Menuet OS                                  ;;
;;                                                                 ;;
;;  Version 0.2  11 August 2003                                    ;;
;;                                                                 ;;
;;  Driver for chips of RealTek 8139 family                        ;;
;;  References:                                                    ;;
;;    www.realtek.com.hw - data sheets                             ;;
;;    rtl8139.c - linux driver                                     ;;
;;    8139too.c - linux driver                                     ;;
;;    ethernet driver template by Mike Hibbett                     ;;
;;                                                                 ;;
;;  The copyright statement is                                     ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;  Copyright 2003 Endre Kozma,                                    ;;
;;   endre.kozma@axelero.hu                                        ;;
;;                                                                 ;;
;;  See file COPYING for details                                   ;;
;;                                                                 ;;
;;  10.01.2007 Bugfix for l8139_transmit from Paolo Franchetti     ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	ETH_ALEN	       equ 6
	ETH_HLEN	       equ (2 * ETH_ALEN + 2)
	ETH_ZLEN	       equ 60 ; 60 + 4bytes auto payload for
				      ; mininmum 64bytes frame length

	PCI_REG_COMMAND        equ 0x04 ; command register
	PCI_BIT_PIO	       equ 0 ; bit0: io space control
	PCI_BIT_MMIO	       equ 1 ; bit1: memory space control
	PCI_BIT_MASTER	       equ 2 ; bit2: device acts as a PCI master

	RTL8139_REG_MAR0       equ 0x08 ; multicast filter register 0
	RTL8139_REG_MAR4       equ 0x0c ; multicast filter register 4
	RTL8139_REG_TSD0       equ 0x10 ; transmit status of descriptor
	RTL8139_REG_TSAD0      equ 0x20 ; transmit start address of descriptor
	RTL8139_REG_RBSTART    equ 0x30 ; RxBuffer start address
	RTL8139_REG_COMMAND    equ 0x37 ; command register
	RTL8139_REG_CAPR       equ 0x38 ; current address of packet read
	RTL8139_REG_IMR        equ 0x3c ; interrupt mask register
	RTL8139_REG_ISR        equ 0x3e ; interrupt status register
	RTL8139_REG_TXCONFIG   equ 0x40 ; transmit configuration register
	RTL8139_REG_TXCONFIG_0 equ 0x40 ; transmit configuration register 0
	RTL8139_REG_TXCONFIG_1 equ 0x41 ; transmit configuration register 1
	RTL8139_REG_TXCONFIG_2 equ 0x42 ; transmit configuration register 2
	RTL8139_REG_TXCONFIG_3 equ 0x43 ; transmit configuration register 3
	RTL8139_REG_RXCONFIG   equ 0x44 ; receive configuration register 0
	RTL8139_REG_RXCONFIG_0 equ 0x44 ; receive configuration register 0
	RTL8139_REG_RXCONFIG_1 equ 0x45 ; receive configuration register 1
	RTL8139_REG_RXCONFIG_2 equ 0x46 ; receive configuration register 2
	RTL8139_REG_RXCONFIG_3 equ 0x47 ; receive configuration register 3
	RTL8139_REG_MPC        equ 0x4c ; missed packet counter
	RTL8139_REG_9346CR     equ 0x50 ; serial eeprom 93C46 command register
	RTL8139_REG_CONFIG1    equ 0x52 ; configuration register 1
	RTL8139_REG_CONFIG4    equ 0x5a ; configuration register 4
	RTL8139_REG_HLTCLK     equ 0x5b ; undocumented halt clock register
	RTL8139_REG_BMCR       equ 0x62 ; basic mode control register
	RTL8139_REG_ANAR       equ 0x66 ; auto negotiation advertisement register

; 5.1 packet header
	RTL8139_BIT_RUNT       equ 4 ; total packet length < 64 bytes
	RTL8139_BIT_LONG       equ 3 ; total packet length > 4k
	RTL8139_BIT_CRC        equ 2 ; crc error occured
	RTL8139_BIT_FAE        equ 1 ; frame alignment error occured
	RTL8139_BIT_ROK        equ 0 ; received packet is ok
; 5.4 command register
	RTL8139_BIT_RST        equ 4 ; reset bit
	RTL8139_BIT_RE	       equ 3 ; receiver enabled
	RTL8139_BIT_TE	       equ 2 ; transmitter enabled
	RTL8139_BIT_BUFE       equ 0 ; rx buffer is empty, no packet stored
; 5.6 interrupt status register
	RTL8139_BIT_ISR_TOK    equ 2 ; transmit ok
	RTL8139_BIT_ISR_RER    equ 1 ; receive error interrupt
	RTL8139_BIT_ISR_ROK    equ 0 ; receive ok
; 5.7 transmit configyration register
	RTL8139_BIT_TX_MXDMA   equ 8 ; Max DMA burst size per Tx DMA burst
	RTL8139_BIT_TXRR       equ 4 ; Tx Retry count 16+(TXRR*16)
; 5.8 receive configuration register
	RTL8139_BIT_RXFTH      equ 13 ; Rx fifo threshold
	RTL8139_BIT_RBLEN      equ 11 ; Ring buffer length indicator
	RTL8139_BIT_RX_MXDMA   equ 8 ; Max DMA burst size per Rx DMA burst
	RTL8139_BIT_NOWRAP     equ 7 ; transfered data wrapping
	RTL8139_BIT_9356SEL    equ 6 ; eeprom selector 9346/9356
	RTL8139_BIT_AER        equ 5 ; accept error packets
	RTL8139_BIT_AR	       equ 4 ; accept runt packets
	RTL8139_BIT_AB	       equ 3 ; accept broadcast packets
	RTL8139_BIT_AM	       equ 2 ; accept multicast packets
	RTL8139_BIT_APM        equ 1 ; accept physical match packets
	RTL8139_BIT_AAP        equ 0 ; accept all packets
; 5.9 93C46/93C56 command register
	RTL8139_BIT_93C46_EEM1 equ 7 ; RTL8139 eeprom operating mode1
	RTL8139_BIT_93C46_EEM0 equ 6 ; RTL8139 eeprom operating mode0
	RTL8139_BIT_93C46_EECS equ 3 ; chip select
	RTL8139_BIT_93C46_EESK equ 2 ; serial data clock
	RTL8139_BIT_93C46_EEDI equ 1 ; serial data input
	RTL8139_BIT_93C46_EEDO equ 0 ; serial data output
; 5.11 configuration register 1
	RTL8139_BIT_LWACT      equ 4 ; see RTL8139_REG_CONFIG1
	RTL8139_BIT_SLEEP      equ 1 ; sleep bit at older chips
	RTL8139_BIT_PWRDWN     equ 0 ; power down bit at older chips
	RTL8139_BIT_PMEn       equ 0 ; power management enabled
; 5.14 configuration register 4
	RTL8139_BIT_LWPTN      equ 2 ; see RTL8139_REG_CONFIG4
; 6.2 transmit status register
	RTL8139_BIT_ERTXTH     equ 16 ; early TX threshold
	RTL8139_BIT_TOK        equ 15 ; transmit ok
	RTL8139_BIT_OWN        equ 13 ; tx DMA operation is completed
; 6.18 basic mode control register
	RTL8139_BIT_ANE        equ 12 ; auto negotiation enable
; 6.20 auto negotiation advertisement register
	RTL8139_BIT_TXFD       equ 8 ; 100base-T full duplex
	RTL8139_BIT_TX	       equ 7 ; 100base-T
	RTL8139_BIT_10FD       equ 6 ; 10base-T full duplex
	RTL8139_BIT_10	       equ 5 ; 10base-T
	RTL8139_BIT_SELECTOR   equ 0 ; binary encoded selector CSMA/CD=00001
; RX/TX buffer size
	RTL8139_RBLEN	       equ 0 ; 0==8K 1==16k 2==32k 3==64k
	RTL8139_RX_BUFFER_SIZE equ (8192 shl RTL8139_RBLEN)
	MAX_ETH_FRAME_SIZE     equ 1516 ; exactly 1514 wthout CRC
	RTL8139_NUM_TX_DESC    equ 4
	RTL8139_TX_BUFFER_SIZE equ (MAX_ETH_FRAME_SIZE * RTL8139_NUM_TX_DESC)
	RTL8139_TXRR	       equ 8 ; total retries = 16+(TXRR*16)
	RTL8139_TX_MXDMA       equ 6 ; 0==16 1==32 2==64 3==128
				     ; 4==256 5==512 6==1024 7==2048
	RTL8139_ERTXTH	       equ 8 ; in unit of 32 bytes e.g:(8*32)=256
	RTL8139_RX_MXDMA       equ 7 ; 0==16 1==32 2==64 3==128
				     ; 4==256 5==512 6==1024 7==unlimited
	RTL8139_RXFTH	       equ 7 ; 0==16 1==32 2==64 3==128
				     ; 4==256 5==512 6==1024 7==no threshold
	RTL8139_RX_CONFIG      equ ((RTL8139_RBLEN shl RTL8139_BIT_RBLEN) \
				    or (RTL8139_RX_MXDMA shl RTL8139_BIT_RX_MXDMA) \
				    or (1 shl RTL8139_BIT_NOWRAP) \
				    or (RTL8139_RXFTH shl RTL8139_BIT_RXFTH) \
				    or (1 shl RTL8139_BIT_AB) or (1 shl RTL8139_BIT_APM) \
				    or (1 shl RTL8139_BIT_AER) or (1 shl RTL8139_BIT_AR) \
				    or (1 shl RTL8139_BIT_AM))
	RTL8139_TX_TIMEOUT     equ 30 ; 300 milliseconds timeout

	EE_93C46_REG_ETH_ID    equ 7 ; MAC offset
	EE_93C46_READ_CMD      equ (6 shl 6) ; 110b + 6bit address
	EE_93C56_READ_CMD      equ (6 shl 8) ; 110b + 8bit address
	EE_93C46_CMD_LENGTH    equ 9 ; start bit + cmd + 6bit address
	EE_93C56_CMD_LENGTH    equ 11 ; start bit + cmd + 8bit ddress

	VER_RTL8139	       equ 1100000b
	VER_RTL8139A	       equ 1110000b
;       VER_RTL8139AG          equ 1110100b
	VER_RTL8139B	       equ 1111000b
	VER_RTL8130	       equ VER_RTL8139B
	VER_RTL8139C	       equ 1110100b
	VER_RTL8100	       equ 1111010b
	VER_RTL8100B	       equ 1110101b
	VER_RTL8139D	       equ VER_RTL8100B
	VER_RTL8139CP	       equ 1110110b
	VER_RTL8101	       equ 1110111b

	IDX_RTL8139	       equ 0
	IDX_RTL8139A	       equ 1
	IDX_RTL8139B	       equ 2
	IDX_RTL8139C	       equ 3
	IDX_RTL8100	       equ 4
	IDX_RTL8139D	       equ 5
	IDX_RTL8139D	       equ 6
	IDX_RTL8101	       equ 7


; These two must be 4 byte aligned ( which they are )
rtl8139_rx_buff     equ     eth_data_start
rtl8139_tx_buff     equ     rtl8139_rx_buff + (RTL8139_RX_BUFFER_SIZE + MAX_ETH_FRAME_SIZE)

uglobal
	align	4
rtl8139_rx_buff_offset: dd 0
curr_tx_desc: dd 0
endg

iglobal
hw_ver_array: db VER_RTL8139, VER_RTL8139A, VER_RTL8139B, VER_RTL8139C
	      db VER_RTL8100, VER_RTL8139D, VER_RTL8139CP, VER_RTL8101
HW_VER_ARRAY_SIZE = $-hw_ver_array
endg

uglobal
hw_ver_id: db 0
endg

;***************************************************************************
;   Function
;      rtl8139_probe
;   Description
;      Searches for an ethernet card, enables it and clears the rx buffer
;      If a card was found, it enables the ethernet -> TCPIP link
;   Destroyed registers
;      eax, ebx, ecx, edx
;
;***************************************************************************
rtl8139_probe:
; enable the device
	mov	al, 2
	mov	ah, [pci_bus]
	mov	bh, [pci_dev]
	mov	bl, PCI_REG_COMMAND
	call	pci_read_reg
	mov	cx, ax
	or	cl, (1 shl PCI_BIT_MASTER) or (1 shl PCI_BIT_PIO)
	and	cl, not (1 shl PCI_BIT_MMIO)
	mov	al, 2
	mov	ah, [pci_bus]
	mov	bh, [pci_dev]
	mov	bl, PCI_REG_COMMAND
	call	pci_write_reg
; get chip version
	mov	edx, [io_addr]
	add	edx, RTL8139_REG_TXCONFIG_2
	in	ax, dx
	shr	ah, 2
	shr	ax, 6
	and	al, 01111111b
	mov	ecx, HW_VER_ARRAY_SIZE-1
.chip_ver_loop:
	cmp	al, [hw_ver_array+ecx]
	je	.chip_ver_found
	dec	ecx
	jns	.chip_ver_loop
	xor	cl, cl ; default RTL8139
.chip_ver_found:
	mov	[hw_ver_id], cl
; wake up the chip
	mov	edx, [io_addr]
	add	edx, RTL8139_REG_HLTCLK
	mov	al, 'R' ; run the clock
	out	dx, al
; unlock config and BMCR registers
	add	edx, RTL8139_REG_9346CR - RTL8139_REG_HLTCLK
	mov	al, (1 shl RTL8139_BIT_93C46_EEM1) or (1 shl RTL8139_BIT_93C46_EEM0)
	out	dx, al
; enable power management
	add	edx, RTL8139_REG_CONFIG1 - RTL8139_REG_9346CR
	in	al, dx
	cmp	byte [hw_ver_id], IDX_RTL8139B
	jl	.old_chip
; set LWAKE pin to active high (default value).
; it is for Wake-On-LAN functionality of some motherboards.
; this signal is used to inform the motherboard to execute a wake-up process.
; only at newer chips.
	or	al, (1 shl RTL8139_BIT_PMEn)
	and	al, not (1 shl RTL8139_BIT_LWACT)
	out	dx, al
	add	edx, RTL8139_REG_CONFIG4 - RTL8139_REG_CONFIG1
	in	al, dx
	and	al, not (1 shl RTL8139_BIT_LWPTN)
	out	dx, al
	jmp	.finish_wake_up
.old_chip:
; wake up older chips
	and	al, not ((1 shl RTL8139_BIT_SLEEP) or (1 shl RTL8139_BIT_PWRDWN))
	out	dx, al
.finish_wake_up:
; lock config and BMCR registers
	xor	al, al
	mov	edx, [io_addr]
	add	edx, RTL8139_REG_9346CR
	out	dx, al
;***************************************************************************
;   Function
;      rt8139_reset
;   Description
;      Place the chip (ie, the ethernet card) into a virgin state
;   Destroyed registers
;      eax, ebx, ecx, edx
;
;***************************************************************************
rtl8139_reset:
	mov	edx, [io_addr]
	add	edx, RTL8139_REG_COMMAND
	mov	al, 1 shl RTL8139_BIT_RST
	out	dx, al
	mov	cx, 1000 ; wait no longer for the reset
.wait_for_reset:
	in	al, dx
	test	al, 1 shl RTL8139_BIT_RST
	jz	.reset_completed ; RST remains 1 during reset
	dec	cx
	jns	.wait_for_reset
.reset_completed:
; get MAC (hardware address)
	mov	ecx, 2
.mac_read_loop:
	lea	eax, [EE_93C46_REG_ETH_ID+ecx]
	push	ecx
	call	rtl8139_read_eeprom
	pop	ecx
	mov	[node_addr+ecx*2], ax
	dec	ecx
	jns	.mac_read_loop
; unlock config and BMCR registers
	mov	edx, [io_addr]
	add	edx, RTL8139_REG_9346CR
	mov	al, (1 shl RTL8139_BIT_93C46_EEM1) or (1 shl RTL8139_BIT_93C46_EEM0)
	out	dx, al
; initialize multicast registers (no filtering)
	mov	eax, 0xffffffff
	add	edx, RTL8139_REG_MAR0 - RTL8139_REG_9346CR
	out	dx, eax
	add	edx, RTL8139_REG_MAR4 - RTL8139_REG_MAR0
	out	dx, eax
; enable Rx/Tx
	mov	al, (1 shl RTL8139_BIT_RE) or (1 shl RTL8139_BIT_TE)
	add	edx, RTL8139_REG_COMMAND - RTL8139_REG_MAR4
	out	dx, al
; 32k Rxbuffer, unlimited dma burst, no wrapping, no rx threshold
; accept broadcast packets, accept physical match packets
	mov	ax, RTL8139_RX_CONFIG
	add	edx, RTL8139_REG_RXCONFIG - RTL8139_REG_COMMAND
	out	dx, ax
; 1024 bytes DMA burst, total retries = 16 + 8 * 16 = 144
	mov	ax, (RTL8139_TX_MXDMA shl RTL8139_BIT_TX_MXDMA) \
		    or (RTL8139_TXRR shl RTL8139_BIT_TXRR)
	add	edx, RTL8139_REG_TXCONFIG - RTL8139_REG_RXCONFIG
	out	dx, ax
; enable auto negotiation
	add	edx, RTL8139_REG_BMCR - RTL8139_REG_TXCONFIG
	in	ax, dx
	or	ax, (1 shl RTL8139_BIT_ANE)
	out	dx, ax
; set auto negotiation advertisement
	add	edx, RTL8139_REG_ANAR - RTL8139_REG_BMCR
	in	ax, dx
	or	ax, (1 shl RTL8139_BIT_SELECTOR) or (1 shl RTL8139_BIT_10) \
		    or (1 shl RTL8139_BIT_10FD) or (1 shl RTL8139_BIT_TX) \
		    or (1 shl RTL8139_BIT_TXFD)
	out	dx, ax
; lock config and BMCR registers
	xor	eax, eax
	add	edx, RTL8139_REG_9346CR - RTL8139_REG_ANAR
	out	dx, al
; init RX/TX pointers
	mov	[rtl8139_rx_buff_offset], eax
	mov	[curr_tx_desc], eax
; clear missing packet counter
	add	edx, RTL8139_REG_MPC - RTL8139_REG_9346CR
	out	dx, eax
; disable all interrupts
	add	edx, RTL8139_REG_IMR - RTL8139_REG_MPC
	out	dx, ax
; set RxBuffer address, init RX buffer offset, init TX ring
	mov	eax, rtl8139_rx_buff
	add	edx, RTL8139_REG_RBSTART - RTL8139_REG_IMR
	out	dx, eax
; Indicate that we have successfully reset the card
	mov	eax, [pci_data]
	mov	[eth_status], eax
	ret

;***************************************************************************
;   Function
;      rtl8139_read_eeprom
;   Description
;      reads eeprom type 93c46 and 93c56
;   Parameters
;      al - word to be read (6bit in case of 93c46 and 8bit otherwise)
;   Return value
;      ax - word read in
;   Destroyed register(s)
;      eax, cx, ebx, edx
;
;***************************************************************************
rtl8139_read_eeprom:
	movzx	ebx, al
	mov	edx, [io_addr]
	add	edx, RTL8139_REG_RXCONFIG
	in	al, dx
	test	al, (1 shl RTL8139_BIT_9356SEL)
	jz	.type_93c46
;       and     bl, 01111111b ; don't care first bit
	or	bx, EE_93C56_READ_CMD ; it contains start bit
	mov	cx, EE_93C56_CMD_LENGTH-1 ; cmd_loop counter
	jmp	.read_eeprom
.type_93c46:
	and	bl, 00111111b
	or	bx, EE_93C46_READ_CMD ; it contains start bit
	mov	cx, EE_93C46_CMD_LENGTH-1 ; cmd_loop counter
.read_eeprom:
	add	edx, RTL8139_REG_9346CR - RTL8139_REG_RXCONFIG_0
;       mov     al, (1 shl RTL8139_BIT_93C46_EEM1)
;       out     dx, al
	mov	al, (1 shl RTL8139_BIT_93C46_EEM1) \
		    or (1 shl RTL8139_BIT_93C46_EECS) ; wake up the eeprom
	out	dx, al
.cmd_loop:
	mov	al, (1 shl RTL8139_BIT_93C46_EEM1) or (1 shl RTL8139_BIT_93C46_EECS)
	bt	bx, cx
	jnc	.zero_bit
	or	al, (1 shl RTL8139_BIT_93C46_EEDI)
.zero_bit:
	out	dx, al
;       push    eax
;       in      eax, dx ; eeprom delay
;       pop     eax
	or	al, (1 shl RTL8139_BIT_93C46_EESK)
	out	dx, al
;       in      eax, dx ; eeprom delay
	dec	cx
	jns	.cmd_loop
;       in      eax, dx ; eeprom delay
	mov	al, (1 shl RTL8139_BIT_93C46_EEM1) or (1 shl RTL8139_BIT_93C46_EECS)
	out	dx, al
	mov	cl, 0xf
.read_loop:
	shl	ebx, 1
	mov	al, (1 shl RTL8139_BIT_93C46_EEM1) \
		    or (1 shl RTL8139_BIT_93C46_EECS) \
		    or (1 shl RTL8139_BIT_93C46_EESK)
	out	dx, al
;       in      eax, dx ; eeprom delay
	in	al, dx
	and	al, (1 shl RTL8139_BIT_93C46_EEDO)
	jz	.dont_set
	inc	ebx
.dont_set:
	mov	al, (1 shl RTL8139_BIT_93C46_EEM1) \
		    or (1 shl RTL8139_BIT_93C46_EECS)
	out	dx, al
;       in      eax, dx ; eeprom delay
	dec	cl
	jns	.read_loop
	xor	al, al
	out	dx, al
	mov	ax, bx
	ret

;***************************************************************************
;   Function
;      rtl8139_transmit
;   Description
;      Transmits a packet of data via the ethernet card
;         Pointer to 48 bit destination address in edi
;         Type of packet in bx
;         size of packet in ecx
;         pointer to packet data in esi
;   Destroyed registers
;      eax, edx, esi, edi
;   ToDo
;      for waiting of timeout the rtl8139 internal timer
;      should be used
;
;***************************************************************************
rtl8139_transmit:
	cmp	ecx, MAX_ETH_FRAME_SIZE
	jg	.finish ; packet is too long
	push	ecx
; check descriptor
	mov	ecx, [curr_tx_desc]
	mov	edx, [io_addr]
	lea	edx, [edx+ecx*4+RTL8139_REG_TSD0]
	push	edx ebx
	in	ax, dx
	test    ax, 0x1fff ; or no size given
      jz      .send_packet
      and	ax, (1 shl RTL8139_BIT_TOK) or (1 shl RTL8139_BIT_OWN)
	cmp	ax, (1 shl RTL8139_BIT_TOK) or (1 shl RTL8139_BIT_OWN)
	jz	.send_packet
; wait for timeout
	mov	ebx, RTL8139_TX_TIMEOUT
	mov	eax, 0x5 ; delay x/100 secs
	int	0x40
	in	ax, dx
	and	ax, (1 shl RTL8139_BIT_TOK) or (1 shl RTL8139_BIT_OWN)
	cmp	ax, (1 shl RTL8139_BIT_TOK) or (1 shl RTL8139_BIT_OWN)
	jz	.send_packet
; chip hung, reset it
	call	rtl8139_reset
; reset the card
.send_packet:
; calculate tx_buffer address
	pop	ebx
	push	esi
	mov	eax, MAX_ETH_FRAME_SIZE
	mul	dword [curr_tx_desc]
	mov	esi, edi
	lea	edi, [rtl8139_tx_buff+eax]
	mov	eax, edi
	cld
; copy destination address
	movsd
	movsw
; copy source address
	mov	esi, node_addr
	movsd
	movsw
; copy packet type
	mov	[edi], bx
	add	edi, 2
; copy the packet data
	pop	esi edx ecx
	push	ecx
	shr	ecx, 2
	rep	movsd
	pop	ecx
	push	ecx
	and	ecx, 3
	rep	movsb
; set address
	add	edx, RTL8139_REG_TSAD0 - RTL8139_REG_TSD0
	out	dx, eax
; set size and early threshold
	pop	eax ; pick up the size
	add	eax, ETH_HLEN
	cmp	eax, ETH_ZLEN
	jnc	.no_pad
	mov	eax, ETH_ZLEN
.no_pad:
	or	eax, (RTL8139_ERTXTH shl RTL8139_BIT_ERTXTH)
	add	edx, RTL8139_REG_TSD0 - RTL8139_REG_TSAD0
	out	dx, eax
; get next descriptor 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, ...
	inc	dword [curr_tx_desc]
	and	dword [curr_tx_desc], 3
.finish:
	ret

;***************************************************************************
; Function
;    rtl8139_poll
;
; Description
;    Polls the ethernet card for a received packet
;    Received data, if any, ends up in Ether_buffer
; Destroyed register(s)
;    eax, edx, ecx
;
;***************************************************************************
rtl8139_poll:
	mov	word [eth_rx_data_len], 0
	mov	edx, [io_addr]
	add	edx, RTL8139_REG_COMMAND
	in	al, dx
	test	al, (1 shl RTL8139_BIT_BUFE)
	jnz	.finish
; new packet received copy it from rx_buffer into Ether_buffer
	mov	eax, rtl8139_rx_buff
	add	eax, [rtl8139_rx_buff_offset]
; check if packet is ok
	test	byte [eax], (1 shl RTL8139_BIT_ROK)
	jz	.reset_rx
; packet is ok copy it into the Ether_buffer
	movzx	ecx, word [eax+2] ; packet length
	sub	ecx, 4 ; don't copy CRC
	mov	word [eth_rx_data_len], cx
	push	ecx
	shr	ecx, 2 ; first copy dword-wise
	lea	esi, [eax+4] ; don't copy the packet header
	mov	edi, Ether_buffer
	cld
	rep	movsd ; copy the dwords
	pop	ecx
	and	ecx, 3
	rep	movsb ; copy the rest bytes
; update rtl8139_rx_buff_offset
	movzx	eax, word [eax+2] ; packet length
	add	eax, [rtl8139_rx_buff_offset]
	add	eax, 4+3 ; packet header is 4 bytes long + dword alignment
	and	eax, not 3 ; dword alignment
	cmp	eax, RTL8139_RX_BUFFER_SIZE
	jl	.no_wrap
	sub	eax, RTL8139_RX_BUFFER_SIZE
.no_wrap:
	mov	[rtl8139_rx_buff_offset], eax
; update CAPR register
	sub	eax, 0x10 ; value 0x10 is a constant for CAPR
	add	edx, RTL8139_REG_CAPR - RTL8139_REG_COMMAND
	out	dx, ax
.finish:
; clear active interrupt sources
	mov	edx, [io_addr]
	add	edx, RTL8139_REG_ISR
	in	ax, dx
	out	dx, ax
	ret
.reset_rx:
	in	al, dx ; read command register
	push	eax
	and	al, not (1 shl RTL8139_BIT_RE)
	out	dx, al
	pop	eax
	out	dx, al
	add	edx, RTL8139_REG_RXCONFIG - RTL8139_REG_COMMAND
	mov	ax, RTL8139_RX_CONFIG
	out	dx, ax
	ret

rtl8139_cable:
	pusha
	mov	edx, [io_addr]
	add	edx, 0x58
	in	al,dx
	test	al,1 SHL 2
	jnz	.notconnected
	popa
	xor	al,al
	inc	al
	ret
       .notconnected:
	popa
	xor	al,al
	ret
