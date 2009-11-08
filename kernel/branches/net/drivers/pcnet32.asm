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
; status
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

      .io_addr		dd ?
      .irq_line 	db ?
      .pci_bus		db ?
      .pci_dev		db ?

	; The following fields up to .tx_ring_phys inclusive form
	; initialization block for hardware; do not modify
	align 4 ; initialization block must be dword-aligned
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

      .access_read_csr		dd ?
      .access_write_csr 	dd ?
      .access_read_bcr		dd ?
      .access_write_bcr 	dd ?
      .access_read_rap		dd ?
      .access_write_rap 	dd ?
      .access_reset		dd ?

      .size:

}

virtual at 0
 device ETH_DEVICE
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

proc START stdcall, state:dword

	cmp [state], 1
	jne .exit

  .entry:

	DEBUGF 1,"Loading PCnet driver\n"
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

	mov	eax, [esp]

	cmp	[edx + IOCTL.inp_size], 3		; Data input must be at least 3 bytes
	jl	.fail

	mov	eax, [edx + IOCTL.input]
	cmp	byte [eax], 1				; 1 means device number and bus number (pci) are given
	jne	.fail					; other types arent supported for this card yet

; check if the device is already listed

	mov	esi, PCNET_LIST
	mov	ecx, [PCNET_DEV]
	test	ecx, ecx
	jz	.firstdevice
	mov	eax, [edx+IOCTL.input]			; get the pci bus and device numbers
	mov	bx , [eax+1]				;
  .nextdevice:
	lodsd
	cmp	bx , word [eax + device.pci_bus]	; compare with pci and device num in RTL8139 list (notice the usage of word instead of byte)
	je	.find_devicenum 			; Device is already loaded, let's find it's device number

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

	stdcall KernelAlloc, PCNET_RX_RING_SIZE * PCNET_PKT_BUF_SZ
	test	eax, eax
	jz	.err
	mov	[ebx+device.rx_buffer], eax				; Save the address to it into the device struct

; Allocate the Transmit Buffer

	stdcall KernelAlloc, PCNET_TX_RING_SIZE * PCNET_PKT_BUF_SZ
	test	eax, eax
	jz	.err
	mov	[ebx+device.tx_buffer], eax

; Allocate the RX Ring

	stdcall KernelAlloc, PCNET_RX_RING_SIZE * buf_head.size
	test	eax, eax
	jz	.err
	mov	dword [ebx + device.rx_ring], eax
	call	GetPgAddr
	mov	dword [ebx + device.rx_ring_phys], eax

; Allocate the TX ring

	stdcall KernelAlloc, PCNET_TX_RING_SIZE * buf_head.size
	test	eax, eax
	jz	.err
	mov	dword [ebx + device.tx_ring], eax
	call	GetPgAddr
	mov	dword [ebx + device.tx_ring_phys], eax

; fill in some of the structure variables

	call	switch_to_wio

	mov	edi, [ebx + device.rx_ring]
	mov	ecx, PCNET_RX_RING_SIZE
	mov	eax, [ebx + device.rx_buffer]
	call	GetPgAddr
  .rx_init:
	mov	[edi + buf_head.base], eax
	mov	[edi + buf_head.length], PCNET_PKT_BUF_SZ_NEG
	mov	[edi + buf_head.status], 0x8000
	add	eax, PCNET_PKT_BUF_SZ
;        inc     eax
	add	 edi, buf_head.size
	loop	 .rx_init

	mov	edi, [ebx + device.tx_ring]
	mov	ecx, PCNET_TX_RING_SIZE
	mov	eax, [ebx + device.tx_buffer]
	call	GetPgAddr
  .tx_init:
	mov	[edi + buf_head.base], eax
	add	eax, PCNET_PKT_BUF_SZ
	add	edi, buf_head.size
	loop	.tx_init

	mov	[ebx + device.tlen_rlen],(PCNET_TX_RING_LEN_BITS or PCNET_RX_RING_LEN_BITS)

; Ok, the eth_device structure is ready, let's probe the device
; Because initialization fires IRQ, IRQ handler must be aware of this device
	mov	eax, [PCNET_DEV]					; Add the device structure to our device list
	mov	[PCNET_LIST+4*eax], ebx 				; (IRQ handler uses this list to find device)
	inc	[PCNET_DEV]						;

	call	probe							; this function will output in eax
	test	eax, eax
	jnz	.destroy							; If an error occured, exit

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
;;  probe: enables the device (if it really is a PCnet device)
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
probe:
	mov	edx, [ebx + device.io_addr]

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

	xor	ecx, ecx
	call	dwio_read_csr
	cmp	eax, 4
	jne	.no_dev

	; Try Dword I/O
	add	edx, PCNET_DWIO_RAP
	mov	eax, 88
	out	dx , eax
	nop
	nop
	in	eax, dx
	sub	edx, PCNET_DWIO_RAP
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
	; TODO: remember to use WORD or DWORD operations

;;;        stdcall Sleep, 10

;---------------------------------------------
; Switch to dword operations

 ;       DEBUGF 1,"Switching to 32\n"
 ;
 ;       mov     ecx, PCNET_DWIO_RDP
 ;       mov     eax, 0
 ;       call    wio_write_csr

;---------------------------------------------

	mov	ecx, PCNET_CSR_CHIPID0
	call	[ebx + device.access_read_csr]
	mov	esi, eax

	mov	ecx, PCNET_CSR_CHIPID1
	call	[ebx + device.access_read_csr]
	shl	eax, 16
	or	eax, esi

	mov	ecx, eax
	and	ecx, 0xfff
	cmp	ecx, 3
	jne	.no_dev

	shr	eax, 12
	and	eax, 0xffff
	mov	[ebx + device.chip_version], eax

	DEBUGF 1,"chip version ok\n"
	mov	[ebx + device.fdx], 0
	mov	[ebx + device.mii], 0
	mov	[ebx + device.fset], 0
	mov	[ebx + device.dxsuflo], 0
	mov	[ebx + device.ltint], 0

	cmp	eax, 0x2420
	je	.L2
	cmp	eax, 0x2430
	je	.L2

	mov	[ebx + device.fdx], 1

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
	mov	[ebx + device.name], device_l2
	jmp	.L10
  .L4:
	mov	[ebx + device.name], device_l4
;        mov     [ebx + device.fdx], 1
	jmp	.L10
  .L5:
	mov	[ebx + device.name], device_l5
;        mov     [ebx + device.fdx], 1
	mov	[ebx + device.mii], 1
	mov	[ebx + device.fset], 1
	mov	[ebx + device.ltint], 1
	jmp	.L10
  .L6:
	mov	[ebx + device.name], device_l6
;        mov     [ebx + device.fdx], 1
	mov	[ebx + device.mii], 1
	mov	[ebx + device.fset], 1
	jmp	.L10
  .L7:
	mov	[ebx + device.name], device_l7
;        mov     [ebx + device.fdx], 1
	mov	[ebx + device.mii], 1
	jmp	.L10
  .L8:
	mov	[ebx + device.name], device_l8
;        mov     [ebx + device.fdx], 1
	mov	ecx, PCNET_CSR_RXPOLL
	call	dword [ebx + device.access_read_bcr]
	call	dword [ebx + device.access_write_bcr]
	jmp	.L10
  .L9:
	mov	[ebx + device.name], device_l9
;        mov     [ebx + device.fdx], 1
	mov	[ebx + device.mii], 1
  .L10:
	DEBUGF 1,"device name: %s\n",[ebx + device.name]

	cmp	[ebx + device.fset], 1
	jne	.L11
	mov	ecx, PCNET_BCR_BUSCTL
	call	[ebx + device.access_read_bcr]
	or	eax, 0x800
	call	[ebx + device.access_write_bcr]

	mov	ecx, PCNET_CSR_DMACTL
	call	[ebx + device.access_read_csr]
;        and     eax, 0xc00
;        or      eax, 0xc00
	mov	eax, 0xc00
	call	[ebx + device.access_write_csr]

	mov	[ebx + device.dxsuflo],1
	mov	[ebx + device.ltint],1
  .L11:

	push	ebx
	call	adjust_pci_device
	pop	ebx

	DEBUGF 1,"PCI done\n"
	mov	eax, PCNET_PORT_ASEL
	mov	[ebx + device.options], eax
	mov	[ebx + device.mode_], word 0x0003
	mov	[ebx + device.tlen_rlen], word (PCNET_TX_RING_LEN_BITS or PCNET_RX_RING_LEN_BITS)

	mov	dword [ebx + device.filter], 0
	mov	dword [ebx + device.filter+4], 0

	mov	eax, PCNET_IMR
	mov	ecx, PCNET_CSR_IMR			; Write interrupt mask
	call	[ebx + device.access_write_csr]

if 0

	mov	ecx, PCNET_BCR_SSTYLE		; Select Software style 2       TODO: freebsd driver uses style 3, why?
	mov	eax, 2
	call	[ebx + device.access_write_bcr]


; ------------ really nescessary??? ----------------
	lea	eax, [ebx + device.private]
	mov	ecx, eax
	and	ecx, 0xFFF ; KolibriOS PAGE SIZE
	call	GetPgAddr
	add	eax, ecx

	and	eax, 0xffff
	mov	ecx, PCNET_CSR_IAB0
	call	[ebx + device.access_write_csr]


	lea	eax, [ebx + device.private]
	mov	ecx, eax
	and	ecx, 0xFFF ; KolibriOS PAGE SIZE
	call	GetPgAddr
	add	eax, ecx

	shr	eax,16
	mov	ecx, PCNET_CSR_IAB1
	call	[ebx + device.access_write_csr]

	mov	ecx, PCNET_CSR_CSR
	mov	eax, 1
	call	[ebx + device.access_write_csr]
; ------------------------------------------------
end if

;       mov     esi, 1
;       call    Sleep


reset:

; attach int handler

	movzx	eax, [ebx+device.irq_line]
	DEBUGF	1,"Attaching int handler to irq %x\n",eax:1
	stdcall AttachIntHandler, eax, int_handler, dword 0
	test	eax, eax
	jnz	@f
	DEBUGF	1,"\nCould not attach int handler!\n"
;        or      eax, -1
;        ret
  @@:

	mov	edx, [ebx + device.io_addr]
	call	[ebx + device.access_reset]

	; Switch pcnet32 to 32bit mode
	mov	ecx, PCNET_BCR_SSTYLE
	mov	eax, 2
	call	[ebx + device.access_write_bcr]

	; set/reset autoselect bit
	mov	ecx, PCNET_BCR_MISCCFG
	call	[ebx + device.access_read_bcr]
	and	eax,not 2
	test	[ebx + device.options], PCNET_PORT_ASEL
	jz	.L1
	or	eax, 2
  .L1:
	call	[ebx + device.access_write_bcr]


	; Handle full duplex setting
	cmp	byte [ebx + device.full_duplex], 0
	je	.L2
	mov	ecx, PCNET_BCR_DUPLEX
	call	[ebx + device.access_read_bcr]
	and	eax, not 3
	test	[ebx + device.options], PCNET_PORT_FD
	jz	.L3
	or	eax, 1
	cmp	[ebx + device.options], PCNET_PORT_FD or PCNET_PORT_AUI
	jne	.L4
	or	eax, 2
	jmp	.L4
  .L3:
	test	[ebx + device.options], PCNET_PORT_ASEL
	jz	.L4
	cmp	[ebx + device.chip_version], 0x2627
	jne	.L4
	or	eax, 3
  .L4:
	mov	ecx, PCNET_BCR_DUPLEX
	call	[ebx + device.access_write_bcr]
  .L2:


	; set/reset GPSI bit in test register
	mov	ecx, 124
	call	[ebx + device.access_read_csr]
	mov	ecx, [ebx + device.options]
	and	ecx, PCNET_PORT_PORTSEL
	cmp	ecx, PCNET_PORT_GPSI
	jne	.L5
	or	eax, 0x10
  .L5:
	call	[ebx + device.access_write_csr]
	cmp	[ebx + device.mii], 0
	je	.L6
	test	[ebx + device.options], PCNET_PORT_ASEL
	jnz	.L6
	mov	ecx, PCNET_BCR_MIICTL
	call	[ebx + device.access_read_bcr]
	and	eax,not 0x38
	test	[ebx + device.options], PCNET_PORT_FD
	jz	.L7
	or	eax, 0x10
  .L7:
	test	[ebx + device.options], PCNET_PORT_100
	jz	.L8
	or	eax, 0x08
  .L8:
	call	[ebx + device.access_write_bcr]
	jmp	.L9
.L6:
	test	[ebx + device.options], PCNET_PORT_ASEL
	jz	.L9
	mov	ecx, PCNET_BCR_MIICTL
	DEBUGF 1,"ASEL, enable auto-negotiation\n"
	call	[ebx + device.access_read_bcr]
	and	eax, not 0x98
	or	eax, 0x20
	call	[ebx + device.access_write_bcr]
.L9:
	cmp	[ebx + device.ltint],0
	je	.L10
	mov	ecx,5
	call	[ebx + device.access_read_csr]
	or	eax,(1 shl 14)
	call	[ebx + device.access_write_csr]
.L10:
	mov	eax,[ebx  + device.options]
	and	eax,PCNET_PORT_PORTSEL
	shl	eax,7
	mov	[ebx + device.mode_],ax
	mov	dword [ebx + device.filter], -1
	mov	dword [ebx + device.filter+4], -1

	call	read_mac

	lea	esi, [ebx + device.mac]
	lea	edi, [ebx + device.phys_addr]
	movsd
	movsw

	lea	eax, [ebx + device.private]
	mov	ecx, eax
	and	ecx, 0xFFF ; KolibriOS PAGE SIZE
	call	GetPgAddr
	add	eax, ecx

	push	eax
	and	eax, 0xffff
	mov	ecx, 1
	call	[ebx + device.access_write_csr]
	pop	eax
	shr	eax,16
	mov	ecx,2
	call	[ebx + device.access_write_csr]

	mov	ecx,4
	mov	eax,0x0915
	call	[ebx + device.access_write_csr]

	mov	ecx,0
	mov	eax,1
	call	[ebx + device.access_write_csr]

	mov	[ebx + device.tx_full],0
	mov	[ebx + device.cur_rx],0
	mov	[ebx + device.cur_tx],0
	mov	[ebx + device.dirty_rx],0
	mov	[ebx + device.dirty_tx],0

	mov	ecx,100
.L11:
	push	ecx
	xor	ecx,ecx
	call	[ebx + device.access_read_csr]
	pop	ecx
	test	ax,0x100
	jnz	.L12
	loop	.L11
.L12:

	DEBUGF 1,"hardware reset\n"
	xor	ecx, ecx
	mov	eax, 0x0002
	call	[ebx + device.access_write_csr]

	xor	ecx, ecx
	call	[ebx + device.access_read_csr]

	xor	ecx, ecx
	mov	eax, PCNET_CSR_INTEN or PCNET_CSR_START
	call	[ebx + device.access_write_csr]

	DEBUGF 1,"PCNET reset complete\n"
	xor	eax, eax
	ret




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                         ;;
;; Transmit                                ;;
;;                                         ;;
;; In: buffer pointer in [esp+4]             ;;
;;     size of buffer in [esp+8]           ;;
;;     pointer to device structure in ebx  ;;
;;                                         ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
transmit:
	DEBUGF	1,"Transmitting packet, buffer:%x, size:%u\n",[esp],[esp+4]
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
	movzx	eax, [ebx + device.cur_tx]
	imul	edi, eax, PCNET_PKT_BUF_SZ
	shl	eax, 4
	add	edi, [ebx + device.tx_buffer]
	add	eax, [ebx + device.tx_ring]
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
	call	[ebx + device.access_read_csr]
	or	eax, PCNET_CSR_TX
	call	[ebx + device.access_write_csr]

; get next descriptor 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, ...
	inc	[ebx + device.cur_tx]
	and	[ebx + device.cur_tx], 3
	DEBUGF	2," - Packet Sent! "

.finish:
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

;       DEBUGF  1,"IRQ %x ",eax:2                   ; no, you cant replace 'eax:2' with 'al', this must be a bug in FDO

; find pointer of device wich made IRQ occur

	mov	esi, PCNET_LIST
	mov	ecx, [PCNET_DEV]
	test	ecx, ecx
	jz	.abort
  .nextdevice:
	mov	ebx, dword [esi]
	mov	edx, [ebx + device.io_addr]	; get IRQ reason

	push	ecx
	xor	ecx, ecx ; CSR0
	call	[ebx + device.access_read_csr]
	pop	ecx

	test	al , al
	js	.got_it

	add	esi, 4
	loop	.nextdevice

	ret					    ; If no device was found, abort (The irq was probably for a device, not registered to this driver

  .got_it:
;-------------------------------------------------------
; Possible reasons:
; initialization done - ignore
; transmit done - ignore
; packet received - handle
; Clear ALL IRQ reasons.
; N.B. One who wants to handle more than one reason must be ready
; to two or more reasons in one IRQ.
	xor	ecx, ecx
	call	[ebx + device.access_write_csr]
; Received packet ok?

	test	ax, PCNET_CSR_RINT
	jz	@f

.receiver_test_loop:
	movzx	eax, [ebx + device.cur_rx]
;        and     eax, PCNET_RX_RING_MOD_MASK
	mov	edi, eax

	imul	esi, eax, PCNET_PKT_BUF_SZ	;
	add	esi, [ebx + device.rx_buffer]	; esi now points to rx buffer

	shl	edi, 4				; desc * 16 (16 is size of one ring entry)
	add	edi, [ebx + device.rx_ring]	; edi now points to current rx ring entry

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

	xchg	edi, eax
	push	ecx
	shr	ecx, 2
	cld
	rep	movsd
	pop	ecx
	and	ecx, 3
	rep	movsb

;       mov     word [eax + buf_head.length], PCNET_PKT_BUF_SZ_NEG
	mov	word [eax + buf_head.status], PCNET_RXSTAT_OWN	    ; Set OWN bit back to 1 (controller may write to tx-buffer again now)

	inc	[ebx + device.cur_rx]		; update descriptor
	and	[ebx + device.cur_rx], 3	;

	jmp	EthReceiver			; Send the copied packet to kernel

  .abort:

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

	mov	edx, [ebx + device.io_addr]
	add	dx, 2
	xor	eax, eax

	mov	ecx, PCNET_CSR_PAR0
       @@:
	pop	ax
	call	[ebx + device.access_write_csr]
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

read_mac:				; T- OK
	DEBUGF	1,"Reading MAC"

	mov	edx, [ebx + device.io_addr]
	add	dx, 6
       @@:
	dec	dx
	dec	dx
	in	ax, dx
	push	ax
	DEBUGF	1,"."
	cmp	edx, [ebx + device.io_addr]
	jg	@r

	DEBUGF	1," %x-%x-%x-%x-%x-%x\n",[esp+0]:2,[esp+1]:2,[esp+2]:2,[esp+3]:2,[esp+4]:2,[esp+5]:2

	lea	edi, [ebx + device.mac]
	pop	ax
	stosw
	pop	ax
	stosw
	pop	ax
	stosw

	ret


switch_to_wio:

	mov	[ebx + device.access_read_csr], wio_read_csr
	mov	[ebx + device.access_write_csr], wio_write_csr
	mov	[ebx + device.access_read_bcr], wio_read_bcr
	mov	[ebx + device.access_write_bcr], wio_write_bcr
	mov	[ebx + device.access_read_rap], wio_read_rap
	mov	[ebx + device.access_write_rap], wio_write_rap
	mov	[ebx + device.access_reset], wio_reset

	ret

switch_to_dwio:

	mov	[ebx + device.access_read_csr], dwio_read_csr
	mov	[ebx + device.access_write_csr], dwio_write_csr
	mov	[ebx + device.access_read_bcr], dwio_read_bcr
	mov	[ebx + device.access_write_bcr], dwio_write_bcr
	mov	[ebx + device.access_read_rap], dwio_read_rap
	mov	[ebx + device.access_write_rap], dwio_write_rap
	mov	[ebx + device.access_reset], dwio_reset

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



adjust_pci_device:
	;*******Get current setting************************
	movzx	 edx, byte [ebx + device.pci_dev]
	movzx	 ecx, byte [ebx + device.pci_bus]
	push	ecx edx
	stdcall  PciRead16, ecx ,edx ,0x04
	pop	edx ecx
;        ;******see if its already set as bus master********
;        and      ax,5
;        cmp      ax,5
;        je       .Latency
	;******Make card a bus master*******
	or	 al, 5
	stdcall  PciWrite16, ecx ,edx ,0x04, eax
	;******Check latency setting***********
  .Latency:
   ;*******Get current latency setting************************
;   mov     al, 1                                       ;read a byte
;   mov     bh, [pci_dev]
;   mov     ah, [pci_bus]
;   mov     bl, 0x0D                                ;from Lantency Timer Register
;   call    pci_read_reg
   ;******see if its aat least 64 clocks********
;   cmp      ax,64
;   jge      PCNET_adjust_pci_device_Done
   ;******Set latency to 32 clocks*******
;   mov     cx, 64                              ;value to write
;   mov     bh, [pci_dev]
;   mov     al, 1                               ;write a byte
;   mov     ah, [pci_bus]
;   mov     bl, 0x0D                            ;to Lantency Timer Register
;   call    pci_write_reg
   ;******Check latency setting***********
  .Done:
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
