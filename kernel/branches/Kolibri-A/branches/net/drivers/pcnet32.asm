;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                  ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved.     ;;
;; Distributed under terms of the GNU General Public License        ;;
;;                                                                  ;;
;;          GNU GENERAL PUBLIC LICENSE                              ;;
;;             Version 2, June 1991                                 ;;
;;                                                                  ;;
;; Status: under construction                                       ;;
;;                                                                  ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; $Revision$

format MS COFF

	API_VERSION		equ 0x01000100

	DEBUG			equ 1
	__DEBUG__		equ 1
	__DEBUG_LEVEL__ 	equ 1

MAX_PCNET equ 4
MAX_ETH_FRAME_SIZE equ 1514

include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'
include 'netdrv.inc'


OS_BASE 	equ 0
new_app_base	equ 0x60400000
PROC_BASE	equ OS_BASE+0x0080000

public START
public service_proc
public version


virtual at ebx

	device:

	ETH_DEVICE

      .rx_buffer	dd ?
      .tx_buffer	dd ?

      .io_addr		dd ?
      .irq_line 	db ?
      .pci_bus		db ?
      .pci_dev		db ?
			db ?	; align 4

      .access_read_csr		dd ?
      .access_write_csr 	dd ?
      .access_read_bcr		dd ?
      .access_write_bcr 	dd ?
      .access_read_rap		dd ?
      .access_write_rap 	dd ?
      .access_reset		dd ?

	; The following fields up to .tx_ring_phys inclusive form
	; initialization block for hardware; do not modify  (must be 4-aligned)

      .private:
      .mode_		dw ?
      .tlen_rlen	dw ?
      .phys_addr	dp ?
      .reserved 	dw ?
      .filter		dq ?
      .rx_ring_phys	dd ?
      .tx_ring_phys	dd ?
      .rx_ring		dd ?
      .tx_ring		dd ?
      .cur_rx		db ?
      .cur_tx		db ?
      .dirty_rx 	dd ?
      .dirty_tx 	dd ?
      .tx_full		db ?
      .options		dd ?
      .full_duplex	db ?
      .chip_version	dd ?
      .mii		db ?
      .ltint		db ?
      .dxsuflo		db ?
      .fset		db ?
      .fdx		db ?

      .size = $ - device

end virtual

struc buf_head {
	.base		dd ?
	.length 	dw ?
	.status 	dw ?
	.msg_length	dw ?
	.misc		dw ?
	.reserved	dd ?

	.size:
}

virtual at 0
 buf_head buf_head
end virtual

struc rx_desc_2 { ; Swstyle 2

	.rbadr		dd ?
	.status 	dd ?
	.rfrtag 	dd ?

; +--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
; |Address |  31 |  30 |  29 |  28 |  27 |  26 |  25 |  24 |  23 |  22 |  21 |  20 |19-16|15-12|11-0 |
; +--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
; |CRDA+00h|                                       RBADR[31:0]                                       |
; +--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
; |CRDA+04h| OWN | ERR |FRAM |OFLO | CRC |BUFF | STP | ENP | BPE | PAM |LAFM | BAM | RES |1111 |BCNT |
; +--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
; |CRDA+08h| RES |                              RFRTAG[14:0]                             |0000 |MCNT |
; +--------+-----+-----------------------------------------------------------------------+-----+-----+
; |CRDA+0Ch|                                    USER SPACE                               |     |     |
; +--------+-----------------------------------------------------------------------------+-----+-----+

}

struc rx_desc_3 {  ; Swstyle 3

	.mcnt		dd ?
	.status 	dd ?
	.rbadr		dd ?

; +--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
; |Address |  31 |  30 |  29 |  28 |  27 |  26 |  25 |  24 |  23 |22-16|15-12|11-0 |
; +--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
; |CRDA+00h|                      RES                      | RES | RES |0000 |MCNT |
; +--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
; |CRDA+04h| OWN | ERR |FRAM |OFLO | CRC |BUFF | STP | ENP | BPE | RES |1111 |BCNT |
; +--------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
; |CRDA+08h|                            RBADR[31:0]                                |
; +--------+-----------------------------------------------------------------------+
; |CRDA+0Ch|                             USER SPACE                                |
; +--------+-----------------------------------------------------------------------+

}

virtual at 0
 rx_desc rx_desc_2
end virtual


; PCI Bus defines

	PCI_HEADER_TYPE 	      equ 0x0e	;8 bit
	PCI_BASE_ADDRESS_0	      equ 0x10	;32 bit
	PCI_BASE_ADDRESS_5	      equ 0x24	;32 bits
	PCI_BASE_ADDRESS_SPACE_IO     equ 0x01
	PCI_VENDOR_ID		      equ 0x00	;16 bit
	PCI_BASE_ADDRESS_IO_MASK      equ 0xFFFFFFFC

	PCNET_PORT_AUI		      equ 0x00
	PCNET_PORT_10BT 	      equ 0x01
	PCNET_PORT_GPSI 	      equ 0x02
	PCNET_PORT_MII		      equ 0x03
	PCNET_PORT_PORTSEL	      equ 0x03
	PCNET_PORT_ASEL 	      equ 0x04
	PCNET_PORT_100		      equ 0x40
	PCNET_PORT_FD		      equ 0x80

	PCNET_DMA_MASK		      equ 0xffffffff

	PCNET_LOG_TX_BUFFERS	      equ 2
	PCNET_LOG_RX_BUFFERS	      equ 2

	PCNET_TX_RING_SIZE	      equ 4
	PCNET_TX_RING_MOD_MASK	      equ (PCNET_TX_RING_SIZE-1)
	PCNET_TX_RING_LEN_BITS	      equ (PCNET_LOG_TX_BUFFERS shl 12)

	PCNET_RX_RING_SIZE	      equ 4
	PCNET_RX_RING_MOD_MASK	      equ (PCNET_RX_RING_SIZE-1)
	PCNET_RX_RING_LEN_BITS	      equ (PCNET_LOG_RX_BUFFERS shl 4)

	PCNET_PKT_BUF_SZ	      equ 1544
	PCNET_PKT_BUF_SZ_NEG	      equ 0xf9f8

	PCNET_WIO_RDP		      equ 0x10
	PCNET_WIO_RAP		      equ 0x12
	PCNET_WIO_RESET 	      equ 0x14
	PCNET_WIO_BDP		      equ 0x16
	PCNET_DWIO_RDP		      equ 0x10
	PCNET_DWIO_RAP		      equ 0x14
	PCNET_DWIO_RESET	      equ 0x18
	PCNET_DWIO_BDP		      equ 0x1C
	PCNET_TOTAL_SIZE	      equ 0x20

; CSR registers

	PCNET_CSR_CSR		      equ 0x00
	PCNET_CSR_IAB0		      equ 0x01
	PCNET_CSR_IAB1		      equ 0x02
	PCNET_CSR_IMR		      equ 0x03
	PCNET_CSR_TFEAT 	      equ 0x04
	PCNET_CSR_EXTCTL1	      equ 0x05
	PCNET_CSR_DTBLLEN	      equ 0x06
	PCNET_CSR_EXTCTL2	      equ 0x07
	PCNET_CSR_MAR0		      equ 0x08
	PCNET_CSR_MAR1		      equ 0x09
	PCNET_CSR_MAR2		      equ 0x0A
	PCNET_CSR_MAR3		      equ 0x0B
	PCNET_CSR_PAR0		      equ 0x0C
	PCNET_CSR_PAR1		      equ 0x0D
	PCNET_CSR_PAR2		      equ 0x0E
	PCNET_CSR_MODE		      equ 0x0F
	PCNET_CSR_RXADDR0	      equ 0x18
	PCNET_CSR_RXADDR1	      equ 0x19
	PCNET_CSR_TXADDR0	      equ 0x1E
	PCNET_CSR_TXADDR1	      equ 0x1F
	PCNET_CSR_TXPOLL	      equ 0x2F
	PCNET_CSR_RXPOLL	      equ 0x31
	PCNET_CSR_RXRINGLEN	      equ 0x4C
	PCNET_CSR_TXRINGLEN	      equ 0x4E
	PCNET_CSR_DMACTL	      equ 0x50
	PCNET_CSR_BUSTIMER	      equ 0x52
	PCNET_CSR_MEMERRTIMEO	      equ 0x64
	PCNET_CSR_ONNOWMISC	      equ 0x74
	PCNET_CSR_ADVFEAT	      equ 0x7A
	PCNET_CSR_MACCFG	      equ 0x7D
	PCNET_CSR_CHIPID0	      equ 0x58
	PCNET_CSR_CHIPID1	      equ 0x59

; Control and Status Register (CSR0)

	PCNET_CSR_INIT		      equ 1 shl 0
	PCNET_CSR_START 	      equ 1 shl 1
	PCNET_CSR_STOP		      equ 1 shl 2
	PCNET_CSR_TX		      equ 1 shl 3
	PCNET_CSR_TXON		      equ 1 shl 4
	PCNET_CSR_RXON		      equ 1 shl 5
	PCNET_CSR_INTEN 	      equ 1 shl 6
	PCNET_CSR_INTR		      equ 1 shl 7
	PCNET_CSR_IDONE 	      equ 1 shl 8
	PCNET_CSR_TINT		      equ 1 shl 9
	PCNET_CSR_RINT		      equ 1 shl 10
	PCNET_CSR_MERR		      equ 1 shl 11
	PCNET_CSR_MISS		      equ 1 shl 12
	PCNET_CSR_CERR		      equ 1 shl 13

; Interrupt masks and deferral control (CSR3)

	PCNET_IMR_BSWAP 	      equ 0x0004
	PCNET_IMR_ENMBA 	      equ 0x0008  ; enable modified backoff alg
	PCNET_IMR_DXMT2PD	      equ 0x0010
	PCNET_IMR_LAPPEN	      equ 0x0020  ; lookahead packet processing enb
	PCNET_IMR_DXSUFLO	      equ 0x0040  ; disable TX stop on underflow
	PCNET_IMR_IDONE 	      equ 0x0100
	PCNET_IMR_TINT		      equ 0x0200
	PCNET_IMR_RINT		      equ 0x0400
	PCNET_IMR_MERR		      equ 0x0800
	PCNET_IMR_MISS		      equ 0x1000

	PCNET_IMR		      equ PCNET_IMR_TINT+PCNET_IMR_RINT+PCNET_IMR_IDONE+PCNET_IMR_MERR+PCNET_IMR_MISS

; Test and features control (CSR4)

	PCNET_TFEAT_TXSTRTMASK	      equ 0x0004
	PCNET_TFEAT_TXSTRT	      equ 0x0008
	PCNET_TFEAT_RXCCOFLOWM	      equ 0x0010  ; Rx collision counter oflow
	PCNET_TFEAT_RXCCOFLOW	      equ 0x0020
	PCNET_TFEAT_UINT	      equ 0x0040
	PCNET_TFEAT_UINTREQ	      equ 0x0080
	PCNET_TFEAT_MISSOFLOWM	      equ 0x0100
	PCNET_TFEAT_MISSOFLOW	      equ 0x0200
	PCNET_TFEAT_STRIP_FCS	      equ 0x0400
	PCNET_TFEAT_PAD_TX	      equ 0x0800
	PCNET_TFEAT_TXDPOLL	      equ 0x1000
	PCNET_TFEAT_DMAPLUS	      equ 0x4000

; Extended control and interrupt 1 (CSR5)

	PCNET_EXTCTL1_SPND	      equ 0x0001  ; suspend
	PCNET_EXTCTL1_MPMODE	      equ 0x0002  ; magic packet mode
	PCNET_EXTCTL1_MPENB	      equ 0x0004  ; magic packet enable
	PCNET_EXTCTL1_MPINTEN	      equ 0x0008  ; magic packet interrupt enable
	PCNET_EXTCTL1_MPINT	      equ 0x0010  ; magic packet interrupt
	PCNET_EXTCTL1_MPPLBA	      equ 0x0020  ; magic packet phys. logical bcast
	PCNET_EXTCTL1_EXDEFEN	      equ 0x0040  ; excessive deferral interrupt enb.
	PCNET_EXTCTL1_EXDEF	      equ 0x0080  ; excessive deferral interrupt
	PCNET_EXTCTL1_SINTEN	      equ 0x0400  ; system interrupt enable
	PCNET_EXTCTL1_SINT	      equ 0x0800  ; system interrupt
	PCNET_EXTCTL1_LTINTEN	      equ 0x4000  ; last TX interrupt enb
	PCNET_EXTCTL1_TXOKINTD	      equ 0x8000  ; TX OK interrupt disable

; RX/TX descriptor len (CSR6)

	PCNET_DTBLLEN_RLEN	      equ 0x0F00
	PCNET_DTBLLEN_TLEN	      equ 0xF000

; Extended control and interrupt 2 (CSR7)

	PCNET_EXTCTL2_MIIPDTINTE      equ 0x0001
	PCNET_EXTCTL2_MIIPDTINT       equ 0x0002
	PCNET_EXTCTL2_MCCIINTE	      equ 0x0004
	PCNET_EXTCTL2_MCCIINT	      equ 0x0008
	PCNET_EXTCTL2_MCCINTE	      equ 0x0010
	PCNET_EXTCTL2_MCCINT	      equ 0x0020
	PCNET_EXTCTL2_MAPINTE	      equ 0x0040
	PCNET_EXTCTL2_MAPINT	      equ 0x0080
	PCNET_EXTCTL2_MREINTE	      equ 0x0100
	PCNET_EXTCTL2_MREINT	      equ 0x0200
	PCNET_EXTCTL2_STINTE	      equ 0x0400
	PCNET_EXTCTL2_STINT	      equ 0x0800
	PCNET_EXTCTL2_RXDPOLL	      equ 0x1000
	PCNET_EXTCTL2_RDMD	      equ 0x2000
	PCNET_EXTCTL2_RXFRTG	      equ 0x4000
	PCNET_EXTCTL2_FASTSPNDE       equ 0x8000

; Mode (CSR15)

	PCNET_MODE_RXD		      equ 0x0001  ; RX disable
	PCNET_MODE_TXD		      equ 0x0002  ; TX disable
	PCNET_MODE_LOOP 	      equ 0x0004  ; loopback enable
	PCNET_MODE_TXCRCD	      equ 0x0008
	PCNET_MODE_FORCECOLL	      equ 0x0010
	PCNET_MODE_RETRYD	      equ 0x0020
	PCNET_MODE_INTLOOP	      equ 0x0040
	PCNET_MODE_PORTSEL	      equ 0x0180
	PCNET_MODE_RXVPAD	      equ 0x2000
	PCNET_MODE_RXNOBROAD	      equ 0x4000
	PCNET_MODE_PROMISC	      equ 0x8000

; BCR (Bus Control Registers)

	PCNET_BCR_MMRA		      equ 0x00	  ; Master Mode Read Active
	PCNET_BCR_MMW		      equ 0x01	  ; Master Mode Write Active
	PCNET_BCR_MISCCFG	      equ 0x02
	PCNET_BCR_LED0		      equ 0x04
	PCNET_BCR_LED1		      equ 0x05
	PCNET_BCR_LED2		      equ 0x06
	PCNET_BCR_LED3		      equ 0x07
	PCNET_BCR_DUPLEX	      equ 0x09
	PCNET_BCR_BUSCTL	      equ 0x12
	PCNET_BCR_EECTL 	      equ 0x13
	PCNET_BCR_SSTYLE	      equ 0x14
	PCNET_BCR_PCILAT	      equ 0x16
	PCNET_BCR_PCISUBVENID	      equ 0x17
	PCNET_BCR_PCISUBSYSID	      equ 0x18
	PCNET_BCR_SRAMSIZE	      equ 0x19
	PCNET_BCR_SRAMBOUND	      equ 0x1A
	PCNET_BCR_SRAMCTL	      equ 0x1B
	PCNET_BCR_MIICTL	      equ 0x20
	PCNET_BCR_MIIADDR	      equ 0x21
	PCNET_BCR_MIIDATA	      equ 0x22
	PCNET_BCR_PCIVENID	      equ 0x23
	PCNET_BCR_PCIPCAP	      equ 0x24
	PCNET_BCR_DATA0 	      equ 0x25
	PCNET_BCR_DATA1 	      equ 0x26
	PCNET_BCR_DATA2 	      equ 0x27
	PCNET_BCR_DATA3 	      equ 0x28
	PCNET_BCR_DATA4 	      equ 0x29
	PCNET_BCR_DATA5 	      equ 0x2A
	PCNET_BCR_DATA6 	      equ 0x2B
	PCNET_BCR_DATA7 	      equ 0x2C
	PCNET_BCR_ONNOWPAT0	      equ 0x2D
	PCNET_BCR_ONNOWPAT1	      equ 0x2E
	PCNET_BCR_ONNOWPAT2	      equ 0x2F
	PCNET_BCR_PHYSEL	      equ 0x31

; RX status register

	PCNET_RXSTAT_BPE	      equ 0x0080  ; bus parity error
	PCNET_RXSTAT_ENP	      equ 0x0100  ; end of packet
	PCNET_RXSTAT_STP	      equ 0x0200  ; start of packet
	PCNET_RXSTAT_BUFF	      equ 0x0400  ; buffer error
	PCNET_RXSTAT_CRC	      equ 0x0800  ; CRC error
	PCNET_RXSTAT_OFLOW	      equ 0x1000  ; rx overrun
	PCNET_RXSTAT_FRAM	      equ 0x2000  ; framing error
	PCNET_RXSTAT_ERR	      equ 0x4000  ; error summary
	PCNET_RXSTAT_OWN	      equ 0x8000

; TX status register

	PCNET_TXSTAT_TRC	      equ 0x0000000F	  ; transmit retries
	PCNET_TXSTAT_RTRY	      equ 0x04000000	  ; retry
	PCNET_TXSTAT_LCAR	      equ 0x08000000	  ; lost carrier
	PCNET_TXSTAT_LCOL	      equ 0x10000000	  ; late collision
	PCNET_TXSTAT_EXDEF	      equ 0x20000000	  ; excessive deferrals
	PCNET_TXSTAT_UFLOW	      equ 0x40000000	  ; transmit underrun
	PCNET_TXSTAT_BUFF	      equ 0x80000000	  ; buffer error

	PCNET_TXCTL_OWN 	      equ 0x80000000
	PCNET_TXCTL_ERR 	      equ 0x40000000	  ; error summary
	PCNET_TXCTL_ADD_FCS	      equ 0x20000000	  ; add FCS to pkt
	PCNET_TXCTL_MORE_LTINT	      equ 0x10000000
	PCNET_TXCTL_ONE 	      equ 0x08000000
	PCNET_TXCTL_DEF 	      equ 0x04000000
	PCNET_TXCTL_STP 	      equ 0x02000000
	PCNET_TXCTL_ENP 	      equ 0x01000000
	PCNET_TXCTL_BPE 	      equ 0x00800000
	PCNET_TXCTL_MBO 	      equ 0x0000F000
	PCNET_TXCTL_BUFSZ	      equ 0x00000FFF




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

	DEBUGF	1,"Loading PCnet driver\n"
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
	mov	eax, [IOCTL.io_code]

;------------------------------------------------------

	cmp	eax, 0 ;SRV_GETVERSION
	jne	@F

	cmp	[IOCTL.out_size], 4
	jl	.fail
	mov	eax, [IOCTL.output]
	mov	[eax], dword API_VERSION

	xor	eax, eax
	ret

;------------------------------------------------------
  @@:
	cmp	eax, 1 ;SRV_HOOK
	jne	.fail

	cmp	[IOCTL.inp_size], 3			; Data input must be at least 3 bytes
	jl	.fail

	mov	eax, [IOCTL.input]
	cmp	byte [eax], 1				; 1 means device number and bus number (pci) are given
	jne	.fail					; other types arent supported for this card yet

; check if the device is already listed

	mov	ecx, [PCNET_DEV]
	test	ecx, ecx
	jz	.firstdevice

	mov	esi, PCNET_LIST
;        mov     eax, [IOCTL.input]                      ; get the pci bus and device numbers
	mov	ax , [eax+1]				;
  .nextdevice:
	mov	ebx, [esi]
	cmp	ax , word [device.pci_bus]		; compare with pci and device num in device list (notice the usage of word instead of byte)
	je	.find_devicenum 			; Device is already loaded, let's find it's device number
	add	esi, 4
	loop	.nextdevice

; This device doesnt have its own eth_device structure yet, lets create one

  .firstdevice:
	cmp	[PCNET_DEV], MAX_PCNET			; First check if the driver can handle one more card
	jge	.fail

	push	edx
	stdcall KernelAlloc, device.size		; Allocate the buffer for eth_device structure
	pop	edx
	test	eax, eax
	jz	.fail
	mov	ebx, eax				; ebx is always used as a pointer to the structure (in driver, but also in kernel code)

; Fill in the direct call addresses into the struct

	mov	[device.reset], reset
	mov	[device.transmit], transmit
	mov	[device.get_MAC], read_mac
	mov	[device.set_MAC], write_mac
	mov	[device.unload], unload
	mov	[device.name], my_service

; save the pci bus and device numbers

	mov	eax, [IOCTL.input]
	mov	cl , [eax+1]
	mov	[device.pci_bus], cl
	mov	cl , [eax+2]
	mov	[device.pci_dev], cl

; Now, it's time to find the base io addres of the PCI device

	find_io [device.pci_bus], [device.pci_dev], [device.io_addr]

; We've found the io address, find IRQ now

	find_irq [device.pci_bus], [device.pci_dev], [device.irq_line]

	DEBUGF	1,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",\
	[device.pci_dev]:1,[device.pci_bus]:1,[device.irq_line]:1,[device.io_addr]:4

	allocate_and_clear [device.tx_buffer], (PCNET_RX_RING_SIZE * PCNET_PKT_BUF_SZ), .err
	allocate_and_clear [device.rx_buffer], (PCNET_TX_RING_SIZE * PCNET_PKT_BUF_SZ), .err
	allocate_and_clear [device.rx_ring], (PCNET_RX_RING_SIZE * buf_head.size), .err

	mov	eax, [device.rx_ring]
	call	GetPgAddr
	mov	[device.rx_ring_phys], eax

	allocate_and_clear [device.tx_ring], (PCNET_TX_RING_SIZE * buf_head.size), .err

	mov	eax, [device.tx_ring]
	call	GetPgAddr
	mov	[device.tx_ring_phys], eax

; Ok, the eth_device structure is ready, let's probe the device
; Because initialization fires IRQ, IRQ handler must be aware of this device
	mov	eax, [PCNET_DEV]					; Add the device structure to our device list
	mov	[PCNET_LIST+4*eax], ebx 				; (IRQ handler uses this list to find device)
	inc	[PCNET_DEV]						;

	call	probe							; this function will output in eax
	test	eax, eax
	jnz	.destroy						; If an error occured, exit

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
	dec	[PCNET_DEV]
  .err:
	DEBUGF	1,"Error, removing all data !\n"
	stdcall KernelFree, [device.rx_buffer]
	stdcall KernelFree, [device.tx_buffer]
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
;;  probe: enables the device (if it really is a PCnet device)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:

	make_bus_master [device.pci_bus], [device.pci_dev]

; first, fill in some of the structure variables

	mov	edi, [device.rx_ring]
	mov	ecx, PCNET_RX_RING_SIZE
	mov	eax, [device.rx_buffer]
	call	GetPgAddr
  .rx_init:
	mov	[edi + buf_head.base], eax
	mov	[edi + buf_head.length], PCNET_PKT_BUF_SZ_NEG
	mov	[edi + buf_head.status], 0x8000
	and	dword [edi + buf_head.msg_length], 0
	and	dword [edi + buf_head.reserved], 0
	add	eax, PCNET_PKT_BUF_SZ
;        inc     eax
	add	 edi, buf_head.size
	loop	 .rx_init

	mov	edi, [device.tx_ring]
	mov	ecx, PCNET_TX_RING_SIZE
	mov	eax, [device.tx_buffer]
	call	GetPgAddr
  .tx_init:
	mov	[edi + buf_head.base], eax
	and	dword [edi + buf_head.length], 0
	and	dword [edi + buf_head.msg_length], 0
	and	dword [edi + buf_head.reserved], 0
	add	eax, PCNET_PKT_BUF_SZ
	add	edi, buf_head.size
	loop	.tx_init

	mov	[device.tlen_rlen], (PCNET_TX_RING_LEN_BITS or PCNET_RX_RING_LEN_BITS)

	; First, we must try to use Word operations
	call	switch_to_wio
	set_io	0
	call	wio_reset

	xor	ecx, ecx
	call	wio_read_csr
	cmp	eax, 4
	jne	.try_dwio

	; Try Word I/O
	mov	ax , 88
	add	edx, PCNET_WIO_RAP
	out	dx , ax
	nop
	nop
	in	ax , dx
	sub	edx, PCNET_WIO_RAP
	cmp	ax , 88
	jne	.try_dwio

	DEBUGF 1,"Using WIO\n"

	call	switch_to_wio

	jmp	.L1

  .try_dwio:
	call	dwio_reset

	set_io	0
	xor	ecx, ecx
	call	dwio_read_csr
	cmp	eax, 4
	jne	.no_dev

	; Try Dword I/O
	set_io	PCNET_DWIO_RAP
	mov	eax, 88
	out	dx , eax
	nop
	nop
	in	eax, dx
	set_io	0
	and	eax, 0xffff
	cmp	eax, 88
	jne	.no_dev

	DEBUGF 1,"Using DWIO\n"

	call	switch_to_dwio

	jmp	.L1

  .no_dev:
	DEBUGF 1,"PCnet device not found!\n"
	mov	eax, 1
	ret
  .L1:

	mov	ecx, PCNET_CSR_CHIPID0
	call	[device.access_read_csr]
	mov	esi, eax

	mov	ecx, PCNET_CSR_CHIPID1
	call	[device.access_read_csr]
	shl	eax, 16
	or	eax, esi

	mov	ecx, eax
	and	ecx, 0xfff
	cmp	ecx, 3
	jne	.no_dev

	shr	eax, 12
	and	eax, 0xffff
	mov	[device.chip_version], eax

	DEBUGF 1,"chip version ok\n"
	mov	[device.fdx], 0
	mov	[device.mii], 0
	mov	[device.fset], 0
	mov	[device.dxsuflo], 0
	mov	[device.ltint], 0

	cmp	eax, 0x2420
	je	.L2
	cmp	eax, 0x2430
	je	.L2

	mov	[device.fdx], 1

	cmp	eax, 0x2621
	je	.L4
	cmp	eax, 0x2623
	je	.L5
	cmp	eax, 0x2624
	je	.L6
	cmp	eax, 0x2625
	je	.L7
	cmp	eax, 0x2626
	je	.L8
	cmp	eax, 0x2627
	je	.L9

	DEBUGF 1,"Invalid chip rev\n"
	jmp	.no_dev
  .L2:
	mov	[device.name], device_l2
	jmp	.L10
  .L4:
	mov	[device.name], device_l4
;        mov     [device.fdx], 1
	jmp	.L10
  .L5:
	mov	[device.name], device_l5
;        mov     [device.fdx], 1
	mov	[device.mii], 1
	mov	[device.fset], 1
	mov	[device.ltint], 1
	jmp	.L10
  .L6:
	mov	[device.name], device_l6
;        mov     [device.fdx], 1
	mov	[device.mii], 1
	mov	[device.fset], 1
	jmp	.L10
  .L7:
	mov	[device.name], device_l7
;        mov     [device.fdx], 1
	mov	[device.mii], 1
	jmp	.L10
  .L8:
	mov	[device.name], device_l8
;        mov     [device.fdx], 1
	mov	ecx, PCNET_CSR_RXPOLL
	call	[device.access_read_bcr]
	call	[device.access_write_bcr]
	jmp	.L10
  .L9:
	mov	[device.name], device_l9
;        mov     [device.fdx], 1
	mov	[device.mii], 1
  .L10:
	DEBUGF 1,"device name: %s\n",[device.name]

	cmp	[device.fset], 1
	jne	.L11
	mov	ecx, PCNET_BCR_BUSCTL
	call	[device.access_read_bcr]
	or	eax, 0x800
	call	[device.access_write_bcr]

	mov	ecx, PCNET_CSR_DMACTL
	call	[device.access_read_csr]
;        and     eax, 0xc00
;        or      eax, 0xc00
	mov	eax, 0xc00
	call	[device.access_write_csr]

	mov	[device.dxsuflo],1
	mov	[device.ltint],1
  .L11:

	DEBUGF 1,"PCI done\n"
	mov	eax, PCNET_PORT_ASEL
	mov	[device.options], eax
	mov	[device.mode_], word 0x0003
	mov	[device.tlen_rlen], word (PCNET_TX_RING_LEN_BITS or PCNET_RX_RING_LEN_BITS)

	mov	dword [device.filter], 0
	mov	dword [device.filter+4], 0

	mov	eax, PCNET_IMR
	mov	ecx, PCNET_CSR_IMR			; Write interrupt mask
	call	[device.access_write_csr]





align 4
reset:

; attach int handler

	movzx	eax, [device.irq_line]
	DEBUGF	1,"Attaching int handler to irq %x\n",eax:1
	stdcall AttachIntHandler, eax, int_handler, dword 0
	test	eax, eax
	jnz	@f
	DEBUGF	1,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
  @@:

	set_io	0
	call	[device.access_reset]		; after a reset, device will be in WIO mode!

; Switch to dword operations

	DEBUGF 1,"Switching to 32-bit mode\n"

	mov	ecx, PCNET_DWIO_RDP
	mov	eax, 0
	call	wio_write_csr

	call	switch_to_dwio

; Lets find out if we are really in 32-bit mode now..

	set_io	0
	set_io	PCNET_DWIO_RAP
	mov	eax, 88
	out	dx , eax
	nop
	nop
	in	eax, dx
	set_io	0
	and	eax, 0xffff
	cmp	eax, 88
	je	.yes_dwio

	call	switch_to_wio			; it seem to have failed, reset device again and use wio
	set_io	0
	call	[device.access_reset]

  .yes_dwio:

	; set/reset autoselect bit
	mov	ecx, PCNET_BCR_MISCCFG
	call	[device.access_read_bcr]
	and	eax,not 2
	test	[device.options], PCNET_PORT_ASEL
	jz	.L1
	or	eax, 2
  .L1:
	call	[device.access_write_bcr]


	; Handle full duplex setting
	cmp	byte [device.full_duplex], 0
	je	.L2
	mov	ecx, PCNET_BCR_DUPLEX
	call	[device.access_read_bcr]
	and	eax, not 3
	test	[device.options], PCNET_PORT_FD
	jz	.L3
	or	eax, 1
	cmp	[device.options], PCNET_PORT_FD or PCNET_PORT_AUI
	jne	.L4
	or	eax, 2
	jmp	.L4
  .L3:
	test	[device.options], PCNET_PORT_ASEL
	jz	.L4
	cmp	[device.chip_version], 0x2627
	jne	.L4
	or	eax, 3
  .L4:
	mov	ecx, PCNET_BCR_DUPLEX
	call	[device.access_write_bcr]
  .L2:


	; set/reset GPSI bit in test register
	mov	ecx, 124
	call	[device.access_read_csr]
	mov	ecx, [device.options]
	and	ecx, PCNET_PORT_PORTSEL
	cmp	ecx, PCNET_PORT_GPSI
	jne	.L5
	or	eax, 0x10
  .L5:
	call	[device.access_write_csr]
	cmp	[device.mii], 0
	je	.L6
	test	[device.options], PCNET_PORT_ASEL
	jnz	.L6
	mov	ecx, PCNET_BCR_MIICTL
	call	[device.access_read_bcr]
	and	eax,not 0x38
	test	[device.options], PCNET_PORT_FD
	jz	.L7
	or	eax, 0x10
  .L7:
	test	[device.options], PCNET_PORT_100
	jz	.L8
	or	eax, 0x08
  .L8:
	call	[device.access_write_bcr]
	jmp	.L9
.L6:
	test	[device.options], PCNET_PORT_ASEL
	jz	.L9
	mov	ecx, PCNET_BCR_MIICTL
	DEBUGF 1,"ASEL, enable auto-negotiation\n"
	call	[device.access_read_bcr]
	and	eax, not 0x98
	or	eax, 0x20
	call	[device.access_write_bcr]
.L9:
	cmp	[device.ltint],0
	je	.L10
	mov	ecx,5
	call	[device.access_read_csr]
	or	eax,(1 shl 14)
	call	[device.access_write_csr]
.L10:
	mov	eax, [device.options]
	and	eax, PCNET_PORT_PORTSEL
	shl	eax, 7
	mov	[device.mode_], ax
	mov	dword [device.filter], -1
	mov	dword [device.filter+4], -1

	call	read_mac

	lea	esi, [device.mac]
	lea	edi, [device.phys_addr]
	movsd
	movsw

	lea	eax, [device.private]
	mov	ecx, eax
	and	ecx, 0xFFF ; KolibriOS PAGE SIZE
	call	GetPgAddr
	add	eax, ecx

	push	eax
	and	eax, 0xffff
	mov	ecx, 1
	call	[device.access_write_csr]
	pop	eax
	shr	eax,16
	mov	ecx,2
	call	[device.access_write_csr]

	mov	ecx,4
	mov	eax,0x0915
	call	[device.access_write_csr]

	mov	ecx,0
	mov	eax,1
	call	[device.access_write_csr]

	mov	[device.tx_full],0
	mov	[device.cur_rx],0
	mov	[device.cur_tx],0
	mov	[device.dirty_rx],0
	mov	[device.dirty_tx],0

	mov	ecx,100
.L11:
	push	ecx
	xor	ecx, ecx
	call	[device.access_read_csr]
	pop	ecx
	test	ax,0x100
	jnz	.L12
	loop	.L11
.L12:

	DEBUGF 1,"Starting up device\n"
	xor	ecx, ecx
	mov	eax, 0x0002
	call	[device.access_write_csr]

	xor	ecx, ecx
	call	[device.access_read_csr]

	xor	ecx, ecx
	mov	eax, PCNET_CSR_INTEN or PCNET_CSR_START
	call	[device.access_write_csr]

	DEBUGF 1,"PCNET reset complete\n"
	xor	eax, eax
; clear packet/byte counters
	lea	edi, [device.bytes_tx]
	mov	ecx, 6
	rep	stosd

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
	DEBUGF	1,"Transmitting packet, buffer:%x, size:%u\n",[esp+4],[esp+8]
	mov	eax, [esp+4]
	DEBUGF	1,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",\
	[eax+00]:2,[eax+01]:2,[eax+02]:2,[eax+03]:2,[eax+04]:2,[eax+05]:2,\
	[eax+06]:2,[eax+07]:2,[eax+08]:2,[eax+09]:2,[eax+10]:2,[eax+11]:2,\
	[eax+13]:2,[eax+12]:2

	cmp	dword [esp+8], 1514
	jg	.finish 			; packet is too long
	cmp	dword [esp+8], 60
	jl	.finish 			; packet is too short

; check descriptor
	movzx	eax, [device.cur_tx]
	imul	edi, eax, PCNET_PKT_BUF_SZ
	shl	eax, 4
	add	edi, [device.tx_buffer]
	add	eax, [device.tx_ring]
	test	byte [eax + buf_head.status + 1], 80h
	jnz	.nospace
; descriptor is free, copy data
	mov	esi, [esp+4]
	mov	ecx, [esp+8]
	mov	edx, ecx
	shr	ecx, 2
	and	edx, 3
	rep	movsd
	mov	ecx, edx
	rep	movsb
; set length
	mov	ecx, [esp+8]
	neg	ecx
	mov	[eax + buf_head.length], cx
; put to transfer queue
	mov	[eax + buf_head.status], 0x8300

; trigger an immediate send
	xor	ecx, ecx	 ; CSR0
	call	[device.access_read_csr]
	or	eax, PCNET_CSR_TX
	call	[device.access_write_csr]

; get next descriptor 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, ...
	inc	[device.cur_tx]
	and	[device.cur_tx], 3
	DEBUGF	2," - Packet Sent! "

.finish:
; update statistics
	inc	[device.packets_tx]

	mov	ecx, [esp+8]
	add	dword [device.bytes_tx], ecx
	adc	dword [device.bytes_tx + 4], 0
	DEBUGF	2," - Done!\n"
	ret

.nospace:
	DEBUGF	1, 'ERROR: no free transmit descriptors\n'
; todo: maybe somehow notify the kernel about the error?
	ret



;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Interrupt handler ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
int_handler:

	DEBUGF	1,"IRQ %x ", eax:2		; no, you cant replace 'eax:2' with 'al', this must be a bug in FDO

; find pointer of device wich made IRQ occur

	mov	esi, PCNET_LIST
	mov	ecx, [PCNET_DEV]
	test	ecx, ecx
	jz	.abort
  .nextdevice:
	mov	ebx, [esi]
	DEBUGF	1,"device=%x? ", ebx
	set_io	0

	push	ecx
	xor	ecx, ecx ; CSR0
	call	[device.access_read_csr]       ; get IRQ reason
	pop	ecx

	test	ax , ax
	jnz	.got_it

	add	esi, 4
	loop	.nextdevice

	ret					; If no device was found, abort (The irq was probably for a device, not registered to this driver

  .got_it:
	DEBUGF	1,"yes, reason=%x ", ax
;-------------------------------------------------------
; Possible reasons:
; initialization done - ignore
; transmit done - ignore
; packet received - handle
; Clear ALL IRQ reasons.
; N.B. One who wants to handle more than one reason must be ready
; to two or more reasons in one IRQ.
	xor	ecx, ecx
	call	[device.access_write_csr]
; Received packet ok?

	test	ax, PCNET_CSR_RINT
	jz	@f

.receiver_test_loop:
	movzx	eax, [device.cur_rx]
;        and     eax, PCNET_RX_RING_MOD_MASK
	mov	edi, eax

	imul	esi, eax, PCNET_PKT_BUF_SZ	;
	add	esi, [device.rx_buffer] 	; esi now points to rx buffer

	shl	edi, 4				; desc * 16 (16 is size of one ring entry)
	add	edi, [device.rx_ring]		; edi now points to current rx ring entry

	mov	cx , [edi + buf_head.status]

	test	cx , PCNET_RXSTAT_OWN		; If this bit is set, the controller OWN's the packet, if not, we do
	jnz	.abort

	test	cx , PCNET_RXSTAT_ENP
	jz	.abort

	test	cx , PCNET_RXSTAT_STP
	jz	.abort

	movzx	ecx, [edi + buf_head.msg_length]	; get packet length in ecx
	sub	ecx, 4				;

	push	ecx
	stdcall KernelAlloc, ecx		; Allocate a buffer to put packet into
	pop	ecx
	test	eax, eax			; Test if we allocated succesfully
	jz	.abort				;

	push	.receiver_test_loop		;
	push	ecx				; for eth_receiver
	push	eax				;

; update statistics
	inc	[device.packets_rx]

	add	dword [device.bytes_rx], ecx
	adc	dword [device.bytes_rx + 4], 0

	xchg	edi, eax

; copy packet data
	shr	cx , 1
	jnc	.nb
	movsb
  .nb:
	shr	cx , 1
	jnc	.nw
	movsw
  .nw:
	rep	movsd

;       mov     word [eax + buf_head.length], PCNET_PKT_BUF_SZ_NEG
	mov	word [eax + buf_head.status], PCNET_RXSTAT_OWN	    ; Set OWN bit back to 1 (controller may write to tx-buffer again now)

	inc	[device.cur_rx] 	  ; update descriptor
	and	[device.cur_rx], 3	  ;

	DEBUGF	1,"Inserting packet\n"
	jmp	EthReceiver			; Send the copied packet to kernel

  .abort:
	DEBUGF	1,"done \n"
  @@:

	ret




;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Write MAC address ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
write_mac:	; in: mac pushed onto stack (as 3 words)

	DEBUGF	1,"Writing MAC: %x-%x-%x-%x-%x-%x",[esp+0]:2,[esp+1]:2,[esp+2]:2,[esp+3]:2,[esp+4]:2,[esp+5]:2

	mov	edx, [device.io_addr]
	add	edx, 2
	xor	eax, eax

	mov	ecx, PCNET_CSR_PAR0
       @@:
	pop	ax
	call	[device.access_write_csr]
	DEBUGF	1,"."
	inc	ecx
	cmp	ecx, PCNET_CSR_PAR2
	jl	@r

	DEBUGF	1,"\n"

; Notice this procedure does not ret, but continues to read_mac instead.

;;;;;;;;;;;;;;;;;;;;;;
;;                  ;;
;; Read MAC address ;;
;;                  ;;
;;;;;;;;;;;;;;;;;;;;;;

read_mac:
	DEBUGF	1,"Reading MAC"

	mov	edx, [device.io_addr]
	add	edx, 6
       @@:
	dec	dx
	dec	dx
	in	ax, dx
	push	ax
	DEBUGF	1,"."
	cmp	edx, [device.io_addr]
	jg	@r

	DEBUGF	1," %x-%x-%x-%x-%x-%x\n",[esp+0]:2,[esp+1]:2,[esp+2]:2,[esp+3]:2,[esp+4]:2,[esp+5]:2

	lea	edi, [device.mac]
	pop	ax
	stosw
	pop	ax
	stosw
	pop	ax
	stosw

	ret


switch_to_wio:
	DEBUGF	1,"Switch to WIO\n"

	mov	[device.access_read_csr], wio_read_csr
	mov	[device.access_write_csr], wio_write_csr
	mov	[device.access_read_bcr], wio_read_bcr
	mov	[device.access_write_bcr], wio_write_bcr
	mov	[device.access_read_rap], wio_read_rap
	mov	[device.access_write_rap], wio_write_rap
	mov	[device.access_reset], wio_reset

	ret

switch_to_dwio:
	DEBUGF	1,"Switch to DWIO\n"

	mov	[device.access_read_csr], dwio_read_csr
	mov	[device.access_write_csr], dwio_write_csr
	mov	[device.access_read_bcr], dwio_read_bcr
	mov	[device.access_write_bcr], dwio_write_bcr
	mov	[device.access_read_rap], dwio_read_rap
	mov	[device.access_write_rap], dwio_write_rap
	mov	[device.access_reset], dwio_reset

	ret





; ecx - index
; return:
; eax - data
wio_read_csr:

	add	edx, PCNET_WIO_RAP
	mov	ax , cx
	out	dx , ax
	add	edx, PCNET_WIO_RDP - PCNET_WIO_RAP
	in	ax , dx
	and	eax, 0xffff
	sub	edx, PCNET_WIO_RDP

	ret


; eax - data
; ecx - index
wio_write_csr:

	add	edx, PCNET_WIO_RAP
	xchg	eax, ecx
	out	dx , ax
	xchg	eax, ecx
	add	edx, PCNET_WIO_RDP - PCNET_WIO_RAP
	out	dx , ax
	sub	edx, PCNET_WIO_RDP

	ret


; ecx - index
; return:
; eax - data
wio_read_bcr:

	add	edx, PCNET_WIO_RAP
	mov	ax , cx
	out	dx , ax
	add	edx, PCNET_WIO_BDP - PCNET_WIO_RAP
	in	ax , dx
	and	eax, 0xffff
	sub	edx, PCNET_WIO_BDP

	ret


; eax - data
; ecx - index
wio_write_bcr:

	add	edx, PCNET_WIO_RAP
	xchg	eax, ecx
	out	dx , ax
	xchg	eax, ecx
	add	edx, PCNET_WIO_BDP - PCNET_WIO_RAP
	out	dx , ax
	sub	edx, PCNET_WIO_BDP

	ret


wio_read_rap:

	add	edx, PCNET_WIO_RAP
	in	ax , dx
	and	eax, 0xffff
	sub	edx, PCNET_WIO_RAP

	ret

; eax - val
wio_write_rap:

	add	edx, PCNET_WIO_RAP
	out	dx , ax
	sub	edx, PCNET_WIO_RAP

	ret

wio_reset:

	push	eax
	add	edx, PCNET_WIO_RESET
	in	ax , dx
	pop	eax
	sub	edx, PCNET_WIO_RESET

	ret


; ecx - index
; return:
; eax - data
dwio_read_csr:

	add	edx, PCNET_DWIO_RAP
	mov	eax, ecx
	out	dx , eax
	add	edx, PCNET_DWIO_RDP - PCNET_DWIO_RAP
	in	eax, dx
	and	eax, 0xffff
	sub	edx, PCNET_DWIO_RDP

	ret


; ecx - index
; eax - data
dwio_write_csr:

	add	edx, PCNET_DWIO_RAP
	xchg	eax, ecx
	out	dx , eax
	add	edx, PCNET_DWIO_RDP - PCNET_DWIO_RAP
	xchg	eax, ecx
	out	dx , eax
	sub	edx, PCNET_DWIO_RDP

	ret

; ecx - index
; return:
; eax - data
dwio_read_bcr:

	add	edx, PCNET_DWIO_RAP
	mov	eax, ecx
	out	dx , eax
	add	edx, PCNET_DWIO_BDP - PCNET_DWIO_RAP
	in	eax, dx
	and	eax, 0xffff
	sub	edx, PCNET_DWIO_BDP

	ret


; ecx - index
; eax - data
dwio_write_bcr:

	add	edx, PCNET_DWIO_RAP
	xchg	eax, ecx
	out	dx , eax
	add	edx, PCNET_DWIO_BDP - PCNET_DWIO_RAP
	xchg	eax, ecx
	out	dx , eax
	sub	edx, PCNET_DWIO_BDP

	ret


dwio_read_rap:

	add	edx, PCNET_DWIO_RAP
	in	eax, dx
	and	eax, 0xffff
	sub	edx, PCNET_DWIO_RAP

	ret


; eax - val
dwio_write_rap:

	add	edx, PCNET_DWIO_RAP
	out	dx , eax
	sub	edx, PCNET_DWIO_RAP

	ret


dwio_reset:

	push	eax
	add	edx, PCNET_DWIO_RESET
	in	eax, dx
	pop	eax
	sub	edx, PCNET_DWIO_RESET

	ret



; End of code

align 4 				      ; Place all initialised data here

PCNET_DEV     dd 0
version       dd (5 shl 16) or (API_VERSION and 0xFFFF)
my_service    db 'PCnet',0		      ; max 16 chars include zero

device_l2     db "PCnet/PCI 79C970",0
device_l4     db "PCnet/PCI II 79C970A",0
device_l5     db "PCnet/FAST 79C971",0
device_l6     db "PCnet/FAST+ 79C972",0
device_l7     db "PCnet/FAST III 79C973",0
device_l8     db "PCnet/Home 79C978",0
device_l9     db "PCnet/FAST III 79C975",0

options_mapping:
dd PCNET_PORT_ASEL					;  0 Auto-select
dd PCNET_PORT_AUI					;  1 BNC/AUI
dd PCNET_PORT_AUI					;  2 AUI/BNC
dd PCNET_PORT_ASEL					;  3 not supported
dd PCNET_PORT_10BT or PCNET_PORT_FD			;  4 10baseT-FD
dd PCNET_PORT_ASEL					;  5 not supported
dd PCNET_PORT_ASEL					;  6 not supported
dd PCNET_PORT_ASEL					;  7 not supported
dd PCNET_PORT_ASEL					;  8 not supported
dd PCNET_PORT_MII					;  9 MII 10baseT
dd PCNET_PORT_MII or PCNET_PORT_FD			; 10 MII 10baseT-FD
dd PCNET_PORT_MII					; 11 MII (autosel)
dd PCNET_PORT_10BT					; 12 10BaseT
dd PCNET_PORT_MII or PCNET_PORT_100			; 13 MII 100BaseTx
dd PCNET_PORT_MII or PCNET_PORT_100 or PCNET_PORT_FD	; 14 MII 100BaseTx-FD
dd PCNET_PORT_ASEL					; 15 not supported

include_debug_strings					; All data wich FDO uses will be included here

section '.data' data readable writable align 16 	; place all uninitialized data place here

PCNET_LIST rd MAX_PCNET 				; This list contains all pointers to device structures the driver is handling
