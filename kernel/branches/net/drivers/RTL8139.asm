;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;; Realtek 8139 driver for KolibriOS                               ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;    v0.1 - march 2009                                            ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

format MS COFF

	API_VERSION		equ 0x01000100

	DEBUG			equ 1
	__DEBUG__		equ 1
	__DEBUG_LEVEL__ 	equ 1

include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'

OS_BASE 	equ 0;
new_app_base	equ 0x60400000
PROC_BASE	equ OS_BASE+0x0080000

public START
public service_proc
public version

struc IOCTL {
      .handle		dd ?
      .io_code		dd ?
      .input		dd ?
      .inp_size 	dd ?
      .output		dd ?
      .out_size 	dd ?
}

virtual at 0
  IOCTL IOCTL
end virtual

struc ETH_DEVICE {
; pointers to procedures
      .unload		dd ?
      .reset		dd ?
      .transmit 	dd ?
      .set_MAC		dd ?
      .get_MAC		dd ?
      .set_mode 	dd ?
      .get_mode 	dd ?
; status & variables
      .bytes_tx 	dq ?
      .bytes_rx 	dq ?
      .packets_tx	dd ?
      .packets_rx	dd ?
      .mode		dd ?  ; This dword contains cable status (10mbit/100mbit, full/half duplex, auto negotiation or not,..)
      .name		dd ?
      .mac		dp ?
; device specific
      .rx_buffer	dd ?
      .tx_buffer	dd ?
      .rx_data_offset	dd ?
      .io_addr		dd ?
      .curr_tx_desc	db ?
      .pci_bus		db ?
      .pci_dev		db ?
      .irq_line 	db ?
      .hw_ver_id	db ?
      .size:

}

virtual at 0
  device ETH_DEVICE
end virtual

; PCI Bus defines

	PCI_HEADER_TYPE 		equ	0x0e  ;8 bit
	PCI_BASE_ADDRESS_0		equ	0x10  ;32 bit
	PCI_BASE_ADDRESS_5		equ	0x24  ;32 bits
	PCI_BASE_ADDRESS_SPACE_IO	equ	0x01
	PCI_VENDOR_ID			equ	0x00  ;16 bit
	PCI_BASE_ADDRESS_IO_MASK	equ	0xFFFFFFFC

; RTL8139 specific defines

	MAX_RTL8139		equ 16	 ; Max number of devices this driver may handle
	TX_TIMEOUT		equ 30	 ; 300 milliseconds timeout

	PCI_REG_CMD		equ 0x04 ; command register
	PCI_BIT_PIO		equ 0	 ; bit0: io space control
	PCI_BIT_MMIO		equ 1	 ; bit1: memory space control
	PCI_BIT_MASTER		equ 2	 ; bit2: device acts as a PCI master

	REG_IDR0		equ 0x00
	REG_MAR0		equ 0x08 ; multicast filter register 0
	REG_MAR4		equ 0x0c ; multicast filter register 4
	REG_TSD0		equ 0x10 ; transmit status of descriptor
	REG_TSAD0		equ 0x20 ; transmit start address of descriptor
	REG_RBSTART		equ 0x30 ; RxBuffer start address
	REG_COMMAND		equ 0x37 ; command register
	REG_CAPR		equ 0x38 ; current address of packet read (word) R/W
	REG_IMR 		equ 0x3c ; interrupt mask register
	REG_ISR 		equ 0x3e ; interrupt status register
	REG_TXCONFIG		equ 0x40 ; transmit configuration register
	REG_RXCONFIG		equ 0x44 ; receive configuration register 0
	REG_MPC 		equ 0x4c ; missed packet counter
	REG_9346CR		equ 0x50 ; serial eeprom 93C46 command register
	REG_CONFIG1		equ 0x52 ; configuration register 1
	REG_MSR 		equ 0x58
	REG_CONFIG4		equ 0x5a ; configuration register 4
	REG_HLTCLK		equ 0x5b ; undocumented halt clock register
	REG_BMCR		equ 0x62 ; basic mode control register
	REG_ANAR		equ 0x66 ; auto negotiation advertisement register
	REG_9346CR_WE		equ 11b SHL 6

	BIT_RUNT		equ 4 ; total packet length < 64 bytes
	BIT_LONG		equ 3 ; total packet length > 4k
	BIT_CRC 		equ 2 ; crc error occured
	BIT_FAE 		equ 1 ; frame alignment error occured
	BIT_ROK 		equ 0 ; received packet is ok

	BIT_RST 		equ 4 ; reset bit
	BIT_RE			equ 3 ; receiver enabled
	BIT_TE			equ 2 ; transmitter enabled
	BUFE			equ 1 ; rx buffer is empty, no packet stored

	BIT_ISR_TOK		equ 2 ; transmit ok
	BIT_ISR_RER		equ 1 ; receive error interrupt
	BIT_ISR_ROK		equ 0 ; receive ok

	BIT_TX_MXDMA		equ 8 ; Max DMA burst size per Tx DMA burst
	BIT_TXRR		equ 4 ; Tx Retry count 16+(TXRR*16)

	BIT_RXFTH		equ 13 ; Rx fifo threshold
	BIT_RBLEN		equ 11 ; Ring buffer length indicator
	BIT_RX_MXDMA		equ 8 ; Max DMA burst size per Rx DMA burst
	BIT_NOWRAP		equ 7 ; transfered data wrapping
	BIT_9356SEL		equ 6 ; eeprom selector 9346/9356
	BIT_AER 		equ 5 ; accept error packets
	BIT_AR			equ 4 ; accept runt packets
	BIT_AB			equ 3 ; accept broadcast packets
	BIT_AM			equ 2 ; accept multicast packets
	BIT_APM 		equ 1 ; accept physical match packets
	BIT_AAP 		equ 0 ; accept all packets

	BIT_93C46_EEM1		equ 7 ; RTL8139 eeprom operating mode1
	BIT_93C46_EEM0		equ 6 ; RTL8139 eeprom operating mode0
	BIT_93C46_EECS		equ 3 ; chip select
	BIT_93C46_EESK		equ 2 ; serial data clock
	BIT_93C46_EEDI		equ 1 ; serial data input
	BIT_93C46_EEDO		equ 0 ; serial data output

	BIT_LWACT		equ 4 ; see REG_CONFIG1
	BIT_SLEEP		equ 1 ; sleep bit at older chips
	BIT_PWRDWN		equ 0 ; power down bit at older chips
	BIT_PMEn		equ 0 ; power management enabled

	BIT_LWPTN		equ 2 ; see REG_CONFIG4

	BIT_ERTXTH		equ 16 ; early TX threshold
	BIT_TOK 		equ 15 ; transmit ok
	BIT_OWN 		equ 13 ; tx DMA operation is completed

	BIT_ANE 		equ 12 ; auto negotiation enable

	BIT_TXFD		equ 8 ; 100base-T full duplex
	BIT_TX			equ 7 ; 100base-T
	BIT_10FD		equ 6 ; 10base-T full duplex
	BIT_10			equ 5 ; 10base-T
	BIT_SELECTOR		equ 0 ; binary encoded selector CSMA/CD=00001

	BIT_IFG1		equ 25
	BIT_IFG0		equ 24

	RBLEN			equ 2 ; Receive buffer size: 0==8K 1==16k 2==32k 3==64k
	TXRR			equ 8 ; total retries = 16+(TXRR*16)
	TX_MXDMA		equ 6 ; 0=16 1=32 2=64 3=128 4=256 5=512 6=1024 7=2048
	ERTXTH			equ 8 ; in unit of 32 bytes e.g:(8*32)=256
	RX_MXDMA		equ 7 ; 0=16 1=32 2=64 3=128 4=256 5=512 6=1024 7=unlimited
	RXFTH			equ 7 ; 0=16 1=32 2=64 3=128 4=256 5=512 6=1024 7=no threshold

	RX_CONFIG		equ (RBLEN shl BIT_RBLEN) or \
				    (RX_MXDMA shl BIT_RX_MXDMA) or \
				    (1 shl BIT_NOWRAP) or \
				    (RXFTH shl BIT_RXFTH) or\
				    (1 shl BIT_AB) or \
				    (1 shl BIT_APM) or \
				    (1 shl BIT_AER) or \
				    (1 shl BIT_AR) or \
				    (1 shl BIT_AM)

	RX_BUFFER_SIZE		equ (8192 shl RBLEN)
	MAX_ETH_FRAME_SIZE	equ 1516 ; exactly 1514 wthout CRC
	NUM_TX_DESC		equ 4
	TX_BUF_SIZE		equ 4096 ; size of one tx buffer (set to 4kb because of KolibriOS's page size)

	EE_93C46_REG_ETH_ID	equ 7 ; MAC offset
	EE_93C46_READ_CMD	equ (6 shl 6) ; 110b + 6bit address
	EE_93C56_READ_CMD	equ (6 shl 8) ; 110b + 8bit address
	EE_93C46_CMD_LENGTH	equ 9  ; start bit + cmd + 6bit address
	EE_93C56_CMD_LENGTH	equ 11 ; start bit + cmd + 8bit ddress

	VER_RTL8139		equ 1100000b
	VER_RTL8139A		equ 1110000b
	VER_RTL8139AG		equ 1110100b
	VER_RTL8139B		equ 1111000b
	VER_RTL8130		equ VER_RTL8139B
	VER_RTL8139C		equ 1110100b
	VER_RTL8100		equ 1111010b
	VER_RTL8100B		equ 1110101b
	VER_RTL8139D		equ VER_RTL8100B
	VER_RTL8139CP		equ 1110110b
	VER_RTL8101		equ 1110111b

	IDX_RTL8139		equ 0
	IDX_RTL8139A		equ 1
	IDX_RTL8139B		equ 2
	IDX_RTL8139C		equ 3
	IDX_RTL8100		equ 4
	IDX_RTL8139D		equ 5
	IDX_RTL8139D		equ 6
	IDX_RTL8101		equ 7

	ISR_SERR		equ 1 SHL 15
	ISR_TIMEOUT		equ 1 SHL 14
	ISR_LENCHG		equ 1 SHL 13
	ISR_FIFOOVW		equ 1 SHL 6
	ISR_PUN 		equ 1 SHL 5
	ISR_RXOVW		equ 1 SHL 4
	ISR_TER 		equ 1 SHL 3
	ISR_TOK 		equ 1 SHL 2
	ISR_RER 		equ 1 SHL 1
	ISR_ROK 		equ 1 SHL 0

	INTERRUPT_MASK		equ ISR_ROK or \
				    ISR_RXOVW or \
				    ISR_PUN or \
				    ISR_FIFOOVW or \
				    ISR_LENCHG or \
				    ISR_TOK or \
				    ISR_TER

	TSR_OWN 		equ 1 SHL 13
	TSR_TUN 		equ 1 SHL 14
	TSR_TOK 		equ 1 SHL 15

	TSR_CDH 		equ 1 SHL 28
	TSR_OWC 		equ 1 SHL 29
	TSR_TABT		equ 1 SHL 30
	TSR_CRS 		equ 1 SHL 31



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

	DEBUGF 1,"Loading rtl8139 driver\n"
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

	mov	edx, [ioctl]
	mov	eax, [ebx+IOCTL.io_code]

;------------------------------------------------------

	cmp	eax, 0 ;SRV_GETVERSION
	jne	@F

	cmp	[edx+IOCTL.out_size], 4
	jl	.fail
	mov	eax, [edx+IOCTL.output]
	mov	[eax], dword API_VERSION

	xor	eax, eax
	ret

;------------------------------------------------------
  @@:
	cmp	eax, 1 ;SRV_HOOK
	jne	.fail

	cmp	[edx + IOCTL.inp_size], 3		; Data input must be at least 3 bytes
	jl	.fail

	mov	eax, [edx + IOCTL.input]
	cmp	byte [eax], 1				; 1 means device number and bus number (pci) are given
	jne	.fail					; other types arent supported for this card yet

; check if the device is already listed

	mov	esi, RTL8139_LIST
	mov	ecx, [RTL8139_DEV]
	test	ecx, ecx
	jz	.firstdevice
;        mov     eax, [edx+IOCTL.input]                  ; get the pci bus and device numbers
	mov	bx , [eax+1]				;
  .nextdevice:
	lodsd
	cmp	bx , word [eax + device.pci_bus]	; compare with pci and device num in RTL8139 list (notice the usage of word instead of byte)
	je	.find_devicenum 			; Device is already loaded, let's find it's device number

	loop	.nextdevice

; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
	cmp	[RTL8139_DEV], MAX_RTL8139		; First check if the driver can handle one more card
	jge	.fail

	push	edx
	stdcall KernelAlloc, dword device.size		; Allocate the buffer for eth_device structure
	pop	edx
	test	eax, eax
	jz	.fail
	mov	ebx, eax				; ebx is always used as a pointer to the structure (in driver, but also in kernel code)

; Fill in the direct call addresses into the struct

	mov	dword [ebx+device.reset], reset
	mov	dword [ebx+device.transmit], transmit
	mov	dword [ebx+device.get_MAC], read_mac
	mov	dword [ebx+device.set_MAC], write_mac
	mov	dword [ebx+device.unload], unload
	mov	dword [ebx+device.name], my_service

; save the pci bus and device numbers

	mov	eax, [edx+IOCTL.input]
	mov	cl , [eax+1]
	mov	[ebx+device.pci_bus], cl
	mov	cl , [eax+2]
	mov	[ebx+device.pci_dev], cl

; Now, it's time to find the base io addres of the PCI device
; TODO: implement check if bus and dev exist on this machine

	mov	edx, PCI_BASE_ADDRESS_0
  .reg_check:
	movzx	eax, byte [ebx+device.pci_bus]
	movzx	ecx, byte [ebx+device.pci_dev]

	push	edx ecx
	stdcall PciRead16, eax ,ecx ,edx
	pop	ecx edx

	mov	[ebx+device.io_addr], eax
	and	eax, PCI_BASE_ADDRESS_IO_MASK
	test	eax, eax
	jz	.inc_reg
	mov	eax, [ebx+device.io_addr]
	and	eax, PCI_BASE_ADDRESS_SPACE_IO
	test	eax, eax
	jz	.inc_reg

	mov	eax, [ebx+device.io_addr]
	and	eax, PCI_BASE_ADDRESS_IO_MASK
	mov	[ebx+device.io_addr], eax
	jmp	.got_io

  .inc_reg:
	add	edx, 4
	cmp	edx, PCI_BASE_ADDRESS_5
	jbe	.reg_check

  .got_io:

; We've found the io address, find IRQ now

	movzx	eax, byte [ebx+device.pci_bus]
	movzx	ecx, byte [ebx+device.pci_dev]
	push	ebx
	stdcall PciRead8, eax ,ecx ,0x3c				; 0x3c is the offset where irq can be found
	pop	ebx
	mov	byte [ebx+device.irq_line], al

	DEBUGF	1,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
	[ebx+device.pci_dev]:1,[ebx+device.pci_bus]:1,[ebx+device.irq_line]:1,[ebx+device.io_addr]:4

; Allocate the Receive buffer

	stdcall KernelAlloc, dword (RX_BUFFER_SIZE+MAX_ETH_FRAME_SIZE)
	test	eax, eax
	jz	.err
	mov	[ebx+device.rx_buffer], eax				; Save the address to it into the device struct

; Now, Clear the allocated buffer

	cld
	mov	edi, eax
	mov	ecx, (RX_BUFFER_SIZE)/4 				; divide by 4 because we are going to use DWORD
	xor	eax, eax
	rep	stosd

; Allocate the Transmit Buffer

	stdcall KernelAlloc, dword (TX_BUF_SIZE*NUM_TX_DESC)
	test	eax, eax
	jz	.err
	mov	[ebx+device.tx_buffer], eax

; This one needs to be cleared too..

	mov	edi, eax
	mov	ecx, (TX_BUF_SIZE*NUM_TX_DESC)/4
	xor	eax, eax
	rep	stosd

; Ok, the eth_device structure is ready, let's probe the device

	call	probe							; this function will output in eax
	test	eax, eax
	jnz	.err							; If an error occured, exit

	mov	eax, [RTL8139_DEV]					; Add the device structure to our device list
	mov	[RTL8139_LIST+4*eax], ebx				; (IRQ handler uses this list to find device)
	inc	[RTL8139_DEV]						;


	call	EthRegDev
	cmp	eax, -1
	je	.destroy

	ret

; If the device was already loaded, find the device number and return it in eax

  .find_devicenum:
	DEBUGF	1,"Trying to find device number of already registered device\n"
	mov	ebx, eax
	call	EthStruc2Dev						; This kernel procedure converts a pointer to device struct in ebx
									; into a device number in edi
	mov	eax, edi						; Application wants it in eax instead
	DEBUGF	1,"Kernel says: %u\n", eax
	ret

; If an error occured, remove all allocated data and exit (returning -1 in eax)

  .destroy:
	; todo: reset device into virgin state

  .err:
	stdcall KernelFree, dword [ebx+device.rx_buffer]
	stdcall KernelFree, dword [ebx+device.tx_buffer]
	stdcall KernelFree, ebx


  .fail:
	or	eax, -1
	ret

;------------------------------------------------------
endp


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;

align 4
unload:
	; TODO: (in this particular order)
	;
	; - Stop the device
	; - Detach int handler
	; - Remove device from local list (RTL8139_LIST)
	; - call unregister function in kernel
	; - Remove all allocated structures and buffers the card used

	or	eax,-1

ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  probe: enables the device (if it really is RTL8139)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:
	DEBUGF	2,"Probing rtl8139 device: "

; enable the device

	movzx	eax, byte [ebx+device.pci_bus]
	movzx	ecx, byte [ebx+device.pci_dev]
	stdcall PciRead32, eax ,ecx ,PCI_REG_CMD

	mov	cx , ax
	or	cl , (1 shl PCI_BIT_MASTER) or (1 shl PCI_BIT_PIO)
	and	cl , not (1 shl PCI_BIT_MMIO)
	movzx	eax, byte [ebx+device.pci_bus]
	movzx	edx, byte [ebx+device.pci_dev]
	stdcall PciWrite32, eax ,edx ,PCI_REG_CMD, ecx

; get chip version

	mov	edx, [ebx+device.io_addr]
	add	edx, REG_TXCONFIG + 2
	in	ax , dx
	shr	ah , 2
	shr	ax , 6
	and	al , 01111111b
	mov	ecx, HW_VER_ARRAY_SIZE-1
  .chip_ver_loop:
	cmp	al , [hw_ver_array+ecx]
	je	.chip_ver_found
	dec	ecx
	jns	.chip_ver_loop
	xor	cl , cl ; default RTL8139
  .chip_ver_found:
	mov	[ebx+device.hw_ver_id], cl

	shl	ecx, 2
	add	ecx, name_crosslist
	mov	ecx, [ecx]
	mov	dword [ebx+device.name], ecx

	DEBUGF	1,"Chip version: %s\n",ecx

; wake up the chip

	mov	edx, [ebx+device.io_addr]
	add	edx, REG_HLTCLK
	mov	al , 'R' ; run the clock
	out	dx , al

; unlock config and BMCR registers

	add	edx, REG_9346CR - REG_HLTCLK
	mov	al , (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EEM0)
	out	dx , al

; enable power management

	add	edx, REG_CONFIG1 - REG_9346CR
	in	al , dx
	cmp	byte [ebx+device.hw_ver_id], IDX_RTL8139B
	jl	.old_chip

; set LWAKE pin to active high (default value).
; it is for Wake-On-LAN functionality of some motherboards.
; this signal is used to inform the motherboard to execute a wake-up process.
; only at newer chips.

	or	al , (1 shl BIT_PMEn)
	and	al , not (1 shl BIT_LWACT)
	out	dx , al
	add	edx, REG_CONFIG4 - REG_CONFIG1
	in	al , dx
	and	al , not (1 shl BIT_LWPTN)
	out	dx , al
	jmp	.finish_wake_up
  .old_chip:

; wake up older chips

	and	al , not ((1 shl BIT_SLEEP) or (1 shl BIT_PWRDWN))
	out	dx , al
  .finish_wake_up:

; lock config and BMCR registers

	xor	al , al
	mov	edx, [ebx+device.io_addr]
	add	edx, REG_9346CR
	out	dx , al
	DEBUGF	2,"done!\n"


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   reset: Set up all registers and descriptors, clear some values
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

reset:
	DEBUGF	2,"Resetting rtl8139: "

; attach int handler

	movzx	eax, [ebx+device.irq_line]
	DEBUGF	1,"Attaching int handler to irq %x, ",eax:1
	stdcall AttachIntHandler, eax, int_handler, dword 0
	test	eax, eax
	jnz	@f
	DEBUGF	1,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
  @@:

; reset chip

	DEBUGF	1,"Resetting chip\n"
	mov	edx, [ebx+device.io_addr]
	add	edx, REG_COMMAND
	mov	al , 1 shl BIT_RST
	out	dx , al
	mov	cx , 1000		; wait no longer for the reset
  .wait_for_reset:
	in	al , dx
	test	al , 1 shl BIT_RST
	jz	.reset_completed	; RST remains 1 during reset
	dec	cx
	jns	.wait_for_reset
  .reset_completed:

; unlock config and BMCR registers

	mov	edx, [ebx+device.io_addr]
	add	edx, REG_9346CR
	mov	al , (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EEM0)
	out	dx , al

; initialize multicast registers (no filtering)

	mov	eax, 0xffffffff
	add	edx, REG_MAR0 - REG_9346CR
	out	dx , eax
	add	edx, REG_MAR4 - REG_MAR0
	out	dx , eax

; enable Rx/Tx

	mov	al , (1 shl BIT_RE) or (1 shl BIT_TE)
	add	edx, REG_COMMAND - REG_MAR4
	out	dx , al

; 32k Rxbuffer, unlimited dma burst, no wrapping, no rx threshold
; accept broadcast packets, accept physical match packets

	mov	ax , RX_CONFIG
	add	edx, REG_RXCONFIG - REG_COMMAND
	out	dx , ax

; 1024 bytes DMA burst, total retries = 16 + 8 * 16 = 144

	mov	eax , (TX_MXDMA shl BIT_TX_MXDMA) or (TXRR shl BIT_TXRR) or BIT_IFG1 or BIT_IFG0
	add	edx, REG_TXCONFIG - REG_RXCONFIG
	out	dx , eax

; enable auto negotiation

	add	edx, REG_BMCR - REG_TXCONFIG
	in	ax , dx
	or	ax , (1 shl BIT_ANE)
	out	dx , ax

; set auto negotiation advertisement

	add	edx, REG_ANAR - REG_BMCR
	in	ax , dx
	or	ax , (1 shl BIT_SELECTOR) or (1 shl BIT_10) or (1 shl BIT_10FD) or (1 shl BIT_TX) or (1 shl BIT_TXFD)
	out	dx , ax

; lock config and BMCR registers

	xor	eax, eax
	add	edx, REG_9346CR - REG_ANAR
	out	dx , al

; init RX/TX pointers

	mov	[ebx+device.rx_data_offset], eax
	mov	[ebx+device.curr_tx_desc], al

; clear packet/byte counters

	lea	edi, [ebx+device.bytes_tx] ; TODO: check if destroying edi, ecx doesnt harm anything
	mov	ecx, 6
	rep	stosd

; clear missing packet counter

	add	edx, REG_MPC - REG_9346CR
	out	dx , eax

; Set up the 4 Txbuffer descriptors

	add	edx, REG_TSAD0 - REG_MPC
	mov	eax, [ebx+device.tx_buffer]
	mov	ecx, 4
  .loop:
	push	eax
	call	GetPgAddr
	DEBUGF	2,"Desc: %x ", eax
	out	dx , eax
	add	dx , 4
	pop	eax
	add	eax, TX_BUF_SIZE
	loop	.loop

; set RxBuffer address, init RX buffer offset, init TX ring

	mov	eax, [ebx+device.rx_buffer]
	call	GetPgAddr
	mov	edx, [ebx+device.io_addr]
	add	edx, REG_RBSTART
	out	dx , eax

; enable interrupts

	mov	eax, INTERRUPT_MASK
	add	edx, REG_IMR - REG_RBSTART
	out	dx , ax

; Read MAC address

	call	read_mac

; Indicate that we have successfully reset the card

	DEBUGF	2,"Done!\n"
	xor	eax, eax

	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: buffer pointer in [esp]             ;;
;;     size of buffer in [esp+4]           ;;
;;     pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
transmit:
	DEBUGF	1,"Transmitting packet, buffer:%x, size:%u\n",[esp],[esp+4]
	mov	eax, [esp]
	DEBUGF	1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
	[eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
	[eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
	[eax+13]:2,[eax+12]:2

	cmp	dword [esp+4], MAX_ETH_FRAME_SIZE
	jg	.finish 			; packet is too long
	cmp	dword [esp+4], 60
	jl	.finish 			; packet is too short

; check descriptor
;        DEBUGF  1,"Checking descriptor, "
	movzx	ecx, [ebx+device.curr_tx_desc]
	mov	edx, [ebx+device.io_addr]
	lea	edx, [edx+ecx*4+REG_TSD0]
	in	ax, dx
	test	ax, 0x1fff ; or no size given
	jz	.send_packet
	and	ax, (1 shl BIT_TOK) or (1 shl BIT_OWN)
	cmp	ax, (1 shl BIT_TOK) or (1 shl BIT_OWN)
	jz	.send_packet
; wait for timeout
;        DEBUGF  1,"Waiting for timeout, "

	push	edx ebx 			 ; TODO : rtl8139 internal timer should be used instead
	stdcall Sleep, TX_TIMEOUT		 ; ? What registers does this destroy ?
	pop	ebx edx

	in	ax, dx
	and	ax, (1 shl BIT_TOK) or (1 shl BIT_OWN)
	cmp	ax, (1 shl BIT_TOK) or (1 shl BIT_OWN)
	jz	.send_packet			 ; if chip hung, reset it
	push	dx
	call	reset				 ; reset the card
	pop	dx
.send_packet:
;        DEBUGF  1,"Sending packet, "

	push	edx
	movzx	eax, [ebx+device.curr_tx_desc]	 ; calculate the current tx_buffer address
	mov	edx, TX_BUF_SIZE ;MAX_ETH_FRAME_SIZE          ;
	mul	edx				 ;
	mov	edi, [ebx+device.tx_buffer]	 ;
	add	edi, eax			 ; Store it in edi
	pop	edx

	mov	esi, [esp]			 ; Copy data to that address
	mov	ecx, [esp+4]			 ;
	shr	ecx, 2				 ;
	rep	movsd				 ;
	mov	ecx, [esp+4]			 ;
	and	ecx, 3				 ;
	rep	movsb				 ;

	inc	[ebx+device.packets_tx] 	 ;
	mov	eax, [esp+4]			 ; Get packet size in eax

	add	dword [ebx + device.bytes_tx], eax
	adc	dword [ebx + device.bytes_tx + 4], 0

;        or      eax, (ERTXTH shl BIT_ERTXTH)     ; Set descriptor size and the early tx treshold into the correct Transmission status register (TSD0, TSD1, TSD2 or TSD3)
	out	dx , eax			 ;

; get next descriptor 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, ...
	inc	[ebx+device.curr_tx_desc]
	and	[ebx+device.curr_tx_desc], 3

	DEBUGF	2," - Packet Sent! "
.finish:
	DEBUGF	2," - Done!\n"
	call	KernelFree
	add	esp, 4 ; pop (balance stack)

	ret





;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

	DEBUGF	1,"IRQ %x ",eax:2		    ; no, you cant replace 'eax:2' with 'al', this must be a bug in FDO

; find pointer of device wich made IRQ occur

	mov	esi, RTL8139_LIST
	mov	ecx, [RTL8139_DEV]
.nextdevice:
	mov	ebx, dword [esi]

	mov	edx, dword [ebx+device.io_addr]     ; get IRQ reason
	add	edx, REG_ISR
	in	ax , dx
	out	dx , ax 			    ; send it back to ACK

	add	esi, 4

	test	ax , ax
	jnz	.got_it

	loop	.nextdevice

	ret					    ; If no device was found, abort (The irq was probably for a device, not registered to this driver)

  .got_it:

; looks like we've found it!

; Lets found out why the irq occured then..

;----------------------------------------------------
; Received packet ok?

	test	ax, ISR_ROK
	jz	@f
	push	ax

  .receive:
	mov	edx, dword [ebx+device.io_addr]     ; get IRQ reason
	add	edx, REG_COMMAND		    ;
	in	al , dx 			    ;
	test	al , BUFE			    ; test if RX buffer is empty
	jnz	.finish 			    ;

	DEBUGF	2,"RX: "

	mov	eax, dword [ebx+device.rx_buffer]
	add	eax, dword [ebx+device.rx_data_offset]
	test	byte [eax], (1 shl BIT_ROK)	    ; check if packet is ok
	jz	.reset_rx
						    ; packet is ok, copy it
	movzx	ecx, word [eax+2]		    ; packet length
	add	dword [ebx + device.bytes_rx], ecx  ; Update stats
	adc	dword [ebx + device.bytes_rx + 4], 0
	inc	dword [ebx + device.packets_rx]     ;
	sub	ecx, 4				    ; don't copy CRC
	DEBUGF	1,"Received %u bytes\n", ecx

	push	ebx eax ecx
	stdcall KernelAlloc, ecx		    ; Allocate a buffer to put packet into
	pop	ecx
	test	eax, eax			    ; Test if we allocated succesfully
	jz	.abort				    ;

	mov	edi, eax			    ; Set up registers to copy the packet
	mov	esi, [esp]			    ;
	add	esi, 4				    ; Dont copy CRC

	push	dword .abort			    ; Kernel will return to this address after EthReceiver
	push	ecx edi 			    ; Save buffer pointer and size, to pass to kernel

	shr	ecx, 2
	cld
	rep	movsd				    ; copy the dwords
	mov	ecx, [esp+4]
	and	ecx, 3
	rep	movsb				    ; copy the rest bytes

	jmp	EthReceiver			    ; Send it to kernel

  .abort:
	pop	eax ebx
						    ; update eth_data_start_offset
	movzx	eax, word [eax+2]		    ; packet length
	add	eax, [ebx+device.rx_data_offset]
	add	eax, 4+3			    ; packet header is 4 bytes long + dword alignment
	and	eax, not 3			    ; dword alignment
	cmp	eax, RX_BUFFER_SIZE
	jl	.no_wrap
	sub	eax, RX_BUFFER_SIZE
  .no_wrap:
	mov	[ebx+device.rx_data_offset], eax
	DEBUGF	1,"New RX ptr: %u", eax

	mov	edx, dword [ebx+device.io_addr]
	add	edx, REG_CAPR			    ; update 'Current Address of Packet Read register'
	sub	eax, 0x10			    ; value 0x10 is a constant for CAPR
	out	dx , ax

	jmp	.receive			    ; check for multiple packets

  .reset_rx:
	test	byte [eax], (1 shl BIT_CRC)
	jz	.no_crc_error
	DEBUGF	2,"\nCRC error!\n"

  .no_crc_error:
	test	byte [eax], (1 shl BIT_FAE)
	jz	.no_fae_error
	DEBUGF	1,"\nFrame alignment error!\n"

  .no_fae_error:
	DEBUGF	1,"Reset RX\n"
	in	al , dx 			    ; read command register
	push	ax

	and	al , not (1 shl BIT_RE) 	    ; Clear the RE bit
	out	dx , al

	pop	ax
	out	dx , al 			    ; write original command back

	add	edx, REG_RXCONFIG - REG_COMMAND     ; Restore RX configuration
	mov	ax , RX_CONFIG
	out	dx , ax

  .finish:
	pop	ax

;----------------------------------------------------
; Transmit error ?

  @@:
	test	ax, ISR_TER
	jz	@f

	push	ax
	cmp	[ebx+device.curr_tx_desc], 4
	jz	.notxd

	mov	edx, [ebx+device.io_addr]
	movzx	ecx, [ebx+device.curr_tx_desc]
	lea	edx, [edx+ecx*4+REG_TSD0]
	in	eax, dx

  .notxd:
	test	eax, TSR_TUN
	jz	.nobun
	DEBUGF	1, "TX: FIFO Buffer underrun!\n"

  .nobun:
	test	eax, TSR_OWC
	jz	.noowc
	DEBUGF	1, "TX: OWC!\n"

  .noowc:
	test	eax, TSR_TABT
	jz	.notabt
	DEBUGF	1, "TX: TABT!\n"

  .notabt:
	test	eax, TSR_CRS
	jz	.nocsl
	DEBUGF	1, "TX: Carrier Sense Lost!\n"

  .nocsl:
;                test    eax, TSR_OWN or TSR_TOK
;                jz      .nofd
;                DEBUGF  1, "TX: Transmit OK (desc: %u)\n", ecx
;
;               .nofd:
	pop	ax

;----------------------------------------------------
; Transmit ok ?

  @@:
	test	ax, ISR_TOK
	jz	@f

	DEBUGF	1, "TX: Transmit OK (desc: %u)\n", [ebx+device.curr_tx_desc]:1

;----------------------------------------------------
; Rx buffer overflow ?

  @@:
	test	ax, ISR_RXOVW
	jz	@f

	push	ax
	DEBUGF	1,"RX-buffer overflow!\n"

	mov	edx, [ebx+device.io_addr]
	add	edx, REG_ISR
	mov	ax , ISR_FIFOOVW or ISR_RXOVW
	out	dx , ax
	pop	ax

;----------------------------------------------------
; Packet underrun? ?


  @@:
	test	ax, ISR_PUN
	jz	@f

	DEBUGF	1,"Packet underrun!\n"

;----------------------------------------------------
; Receive FIFO overflow ?

  @@:
	test	ax, ISR_FIFOOVW
	jz	@f

	push	ax
	DEBUGF	2,"RX fifo overflox!\n"

	mov	edx, [ebx+device.io_addr]
	add	edx, REG_ISR
	mov	ax , ISR_FIFOOVW or ISR_RXOVW
	out	dx , ax
	pop	ax

;----------------------------------------------------
; Something about Cable changed ?

  @@:
	test	ax, ISR_LENCHG
	jz	.fail

	DEBUGF	2,"Cable changed!\n"
	call	cable

; If none of the above events happened, just exit clearing int

  .fail:

	DEBUGF	2,"\n"
	ret




;;;;;;;;;;;;;;;;;;;;;;;;;
;;                     ;;
;; Update Cable status ;;
;;                     ;;
;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
cable:
	DEBUGF	1,"Checking Cable status: "

	mov	edx, dword [ebx+device.io_addr]
	add	edx, REG_MSR
	in	al , dx

;        test    al , 1 SHL 2     ; 0 = link ok 1 = link fail
;        jnz     .notconnected

;        test    al , 1 SHL 3     ; 0 = 100 Mbps 1 = 10 Mbps
;        jnz     .10mbps

	shr	al, 2
	and	al, 3

	mov	byte [ebx+device.mode+3], al
	DEBUGF	1,"Done!\n"
ret



;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Write MAC address ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
write_mac:	; in: mac pushed onto stack (as 3 words)

	DEBUGF	1,"Writing MAC: "

; disable all in command registers

	mov	edx, [ebx+device.io_addr]
	add	edx, REG_9346CR
	xor	eax, eax
	out	dx , al

	add	edx, REG_IMR - REG_9346CR
	xor	eax, eax
	out	dx , ax

	add	edx, REG_ISR - REG_IMR
	mov	eax, -1
	out	dx , ax

; enable writing


	add	edx, REG_9346CR - REG_ISR
	mov	eax, REG_9346CR_WE
	out	dx , al

 ; write the mac ...

	add	edx, REG_IDR0 - REG_9346CR
	pop	eax
	out	dx , eax

	add	edx, 4
	xor	eax, eax
	pop	ax
	out	dx , eax

; disable writing

	add	edx, REG_9346CR -REG_IDR0
	xor	eax, eax
	out	dx , al

	DEBUGF	1,"ok!\n"

; Notice this procedure does not ret, but continues to read_mac instead.


;;;;;;;;;;;;;;;;;;;;;;
;;                  ;;
;; Read MAC address ;;
;;                  ;;
;;;;;;;;;;;;;;;;;;;;;;

read_mac:
	DEBUGF	1,"Reading MAC: "

	mov	edx, [ebx + device.io_addr]
	lea	edi, [ebx + device.mac]
	in	eax, dx
	stosd
	add	edx, 4
	in	ax, dx
	stosw

	DEBUGF	1,"%x-%x-%x-%x-%x-%x\n",[edi-6]:2,[edi-5]:2,[edi-4]:2,[edi-3]:2,[edi-2]:2,[edi-1]:2

	ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                      ;;
;; Read eeprom (type 93c46 and 93c56)                                   ;;
;;                                                                      ;;
;; In: word to be read in al (6bit in case of 93c46 and 8bit otherwise) ;;
;;     pointer to device structure in ebx                               ;;
;;                                                                      ;;
;; OUT: word read in ax                                                 ;;
;;                                                                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
read_eeprom:
	DEBUGF	2,"Reading eeprom, "

	mov	edx, [ebx+device.io_addr]
	push	ebx
	movzx	ebx, al
	add	edx, REG_RXCONFIG
	in	al, dx
	test	al, (1 shl BIT_9356SEL)
	jz	.type_93c46
;       and     bl, 01111111b ; don't care first bit
	or	bx, EE_93C56_READ_CMD		; it contains start bit
	mov	cx, EE_93C56_CMD_LENGTH-1	; cmd_loop counter
	jmp	.read_eeprom
.type_93c46:
	and	bl, 00111111b
	or	bx, EE_93C46_READ_CMD		; it contains start bit
	mov	cx, EE_93C46_CMD_LENGTH-1	; cmd_loop counter
.read_eeprom:
	add	edx, REG_9346CR - REG_RXCONFIG
;       mov     al, (1 shl BIT_93C46_EEM1)
;       out     dx, al
	mov	al, (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EECS) ; wake up the eeprom
	out	dx, al
.cmd_loop:
	mov	al, (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EECS)
	bt	bx, cx
	jnc	.zero_bit
	or	al, (1 shl BIT_93C46_EEDI)
.zero_bit:
	out	dx, al
;       push    eax
;       in      eax, dx ; eeprom delay
;       pop     eax
	or	al, (1 shl BIT_93C46_EESK)
	out	dx, al
;       in      eax, dx ; eeprom delay
	dec	cx
	jns	.cmd_loop
;       in      eax, dx ; eeprom delay
	mov	al, (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EECS)
	out	dx, al
	mov	cl, 0xf
.read_loop:
	shl	ebx, 1
	mov	al, (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EECS) or (1 shl BIT_93C46_EESK)
	out	dx, al
;       in      eax, dx ; eeprom delay
	in	al, dx
	and	al, (1 shl BIT_93C46_EEDO)
	jz	.dont_set
	inc	ebx
.dont_set:
	mov	al, (1 shl BIT_93C46_EEM1) or (1 shl BIT_93C46_EECS)
	out	dx, al
;       in      eax, dx ; eeprom delay
	dec	cl
	jns	.read_loop
	xor	al, al
	out	dx, al
	mov	ax, bx
	pop	ebx

	ret


; End of code

align 4 					; Place all initialised data here

RTL8139_DEV   dd 0
version       dd (5 shl 16) or (API_VERSION and 0xFFFF)
my_service    db 'RTL8139',0			; max 16 chars include zero

device_1      db 'Realtek 8139',0
device_2      db 'Realtek 8139A',0
device_3      db 'Realtek 8139B',0
device_4      db 'Realtek 8139C',0
device_5      db 'Realtek 8100',0
device_6      db 'Realtek 8139D',0
device_7      db 'Realtek 8139CP',0
device_8      db 'Realtek 8101',0

name_crosslist dd device_1
	       dd device_2
	       dd device_3
	       dd device_4
	       dd device_5
	       dd device_6
	       dd device_7
	       dd device_8

hw_ver_array  db VER_RTL8139			; This array is used by the probe routine to find out wich version of the RTL8139 we are working with
	      db VER_RTL8139A
	      db VER_RTL8139B
	      db VER_RTL8139C
	      db VER_RTL8100
	      db VER_RTL8139D
	      db VER_RTL8139CP
	      db VER_RTL8101

HW_VER_ARRAY_SIZE = $-hw_ver_array

include_debug_strings				; All data wich FDO uses will be included here

section '.data' data readable writable align 16 ; place all uninitialized data place here

RTL8139_LIST rd MAX_RTL8139			; This list contains all pointers to device structures the driver is handling

