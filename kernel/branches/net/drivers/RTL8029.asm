;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;; ne2000 driver for KolibriOS                                     ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;; current status (september 2009) - INCOMPLETE                    ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

API_VERSION	equ 0x01000100

DEBUG equ 1
__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 1

include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'

OS_BASE 	equ 0x80000000
new_app_base	equ 0x0
PROC_BASE	equ OS_BASE+0x0080000

; PCI Bus defines
PCI_HEADER_TYPE 	    equ     0x0e  ;8 bit
PCI_BASE_ADDRESS_0	    equ     0x10  ;32 bit
PCI_BASE_ADDRESS_5	    equ     0x24  ;32 bits
PCI_BASE_ADDRESS_SPACE_IO   equ     0x01
PCI_VENDOR_ID		    equ     0x00  ;16 bit
PCI_BASE_ADDRESS_IO_MASK    equ     0xFFFFFFFC

struc IOCTL {
      .handle	   dd ?
      .io_code	   dd ?
      .input	   dd ?
      .inp_size    dd ?
      .output	   dd ?
      .out_size    dd ?
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
      .io_addr		dw ?
      .irq_line 	db ?
      .pci_bus		db ?
      .pci_dev		db ?

      .flags		db ?
      .vendor		db ?
      .asic_base	dw ?
      .memsize		db ?
      .rx_start 	db ?
      .tx_start 	db ?
      .bmem		dd ?
      .rmem		dd ?
      .romdata		rb 16
      .size:

}

virtual at 0
  device ETH_DEVICE
end virtual

public START
public service_proc
public version

	MAX_ne2000		  equ 16	  ; Max number of devices this driver may handle

	P0_PSTART		  equ 0x01
	P0_PSTOP		  equ 0x02
	P0_BOUND		  equ 0x03
	P0_TSR			  equ 0x04
	P0_TPSR 		  equ 0x04
	P0_TBCR0		  equ 0x05
	P0_TBCR1		  equ 0x06
	P0_ISR			  equ 0x07
	P0_RSAR0		  equ 0x08
	P0_RSAR1		  equ 0x09
	P0_RBCR0		  equ 0x0A
	P0_RBCR1		  equ 0x0B
	P0_RSR			  equ 0x0C
	P0_RCR			  equ 0x0C
	P0_TCR			  equ 0x0D
	P0_DCR			  equ 0x0E
	P0_IMR			  equ 0x0F

	P1_PAR0 		  equ 0x01
	P1_PAR1 		  equ 0x02
	P1_PAR2 		  equ 0x03
	P1_PAR3 		  equ 0x04
	P1_PAR4 		  equ 0x05
	P1_PAR5 		  equ 0x06
	P1_CURR 		  equ 0x07
	P1_MAR0 		  equ 0x08

	CMD_PS0 		  equ 0x00	  ;  Page 0 select
	CMD_PS1 		  equ 0x40	  ;  Page 1 select
	CMD_PS2 		  equ 0x80	  ;  Page 2 select
	CMD_RD2 		  equ 0x20	  ;  Remote DMA control
	CMD_RD1 		  equ 0x10
	CMD_RD0 		  equ 0x08
	CMD_TXP 		  equ 0x04	  ;  transmit packet
	CMD_STA 		  equ 0x02	  ;  start
	CMD_STP 		  equ 0x01	  ;  stop

	RCR_MON 		  equ 0x20	  ;  monitor mode

	DCR_FT1 		  equ 0x40
	DCR_LS			  equ 0x08	  ;  Loopback select
	DCR_WTS 		  equ 0x01	  ;  Word transfer select

	ISR_PRX 		  equ 0x01	  ;  successful recv
	ISR_PTX 		  equ 0x02	  ;  successful xmit
	ISR_RXE 		  equ 0x04	  ;  receive error
	ISR_TXE 		  equ 0x08	  ;  transmit error
	ISR_OVW 		  equ 0x10	  ;  Overflow
	ISR_CNT 		  equ 0x20	  ;  Counter overflow
	ISR_RDC 		  equ 0x40	  ;  Remote DMA complete
	ISR_RST 		  equ 0x80	  ;  reset

	IRQ_MASK		  equ ISR_PRX ; + ISR_PTX + ISR_TXE

	RSTAT_PRX		  equ 0x01	  ;  successful recv
	RSTAT_CRC		  equ 0x02	  ;  CRC error
	RSTAT_FAE		  equ 0x04	  ;  Frame alignment error
	RSTAT_OVER		  equ 0x08	  ;  FIFO overrun

	TXBUF_SIZE		  equ 6
	RXBUF_END		  equ 32
	PAGE_SIZE		  equ 256

	ETH_ALEN		  equ 6
	ETH_HLEN		  equ 14
	ETH_ZLEN		  equ 60
	ETH_FRAME_LEN		  equ 1514

	FLAG_PIO		  equ 0x01
	FLAG_16BIT		  equ 0x02
	ASIC_PIO		  equ 0

	VENDOR_NONE		  equ 0
	VENDOR_WD		  equ 1
	VENDOR_NOVELL		  equ 2
	VENDOR_3COM		  equ 3

	NE_ASIC_OFFSET		  equ 0x10
	NE_RESET		  equ 0x0F	  ; Used to reset card
	NE_DATA 		  equ 0x00	  ; Used to read/write NIC mem

	MEM_8192		  equ 32
	MEM_16384		  equ 64
	MEM_32768		  equ 128

	ISA_MAX_ADDR		  equ 0x400

;------------------------------------------------

	LAST_IO = 0

macro set_io addr {

	if	addr = 0
		mov	dx, [ebp + device.io_addr]
	else
		add	edx, addr - LAST_IO
	end if

	LAST_IO = addr


}

;-------------------------------------------------


section '.flat' code readable align 16

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; proc START
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START stdcall, state:dword

	cmp	[state], 1
	jne	.exit
  .entry:
	DEBUGF 2,"Registering rtl8029 service \n"
	stdcall RegService, my_service, service_proc
	ret
  .fail:
  .exit:
	xor	eax, eax
	ret

endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; proc SERVICE_PROC
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
proc service_proc ;stdcall, ioctl:dword
	push	ebp

	mov	edx, [esp+8];[ioctl]
	mov	eax, [edx+IOCTL.io_code]

;------------------------------------------------------
		       ;---------------
	cmp	eax, 0 ;SRV_GETVERSION
	jne	@F     ;---------------

	cmp	[edx+IOCTL.out_size], 4
	jl	.fail
	mov	eax, [edx+IOCTL.output]
	mov	[eax], dword API_VERSION

	xor	eax, eax
	pop	ebp
	ret	4

;------------------------------------------------------
  @@:		       ;---------
	cmp	eax, 1 ;SRV_HOOK
	jne	@F     ;---------

	DEBUGF	2,"Checking if device is already listed..\n"

	mov	eax, [edx+IOCTL.input]

	cmp	[edx+IOCTL.inp_size], 3
	jl	.fail
	cmp	byte [eax], 1
	je	.pci

	cmp	[edx+IOCTL.inp_size], 4
	jl	.fail
	cmp	byte [eax], 0
	je	.isa

	jmp	.fail

  .pci:

	mov	esi, ne2000_LIST
	mov	ecx, [ne2000_DEV]
	test	ecx, ecx
	jz	.firstdevice_pci
	mov	bx , [eax+1]
  .nextdevice:
	lodsd
	cmp	bx , word [eax + device.pci_bus] ; compare with pci and device num in ne2000 list
	je	find_device_num
	loop	.nextdevice

  .firstdevice_pci:
	call	create_new_struct

	mov	eax, [edx+IOCTL.input]	       ; save the pci bus and device numbers
	mov	cx , [eax+1]			 ;
	mov	[ebx+device.pci_bus], cl       ;
	mov	[ebx+device.pci_dev], ch       ;

	mov	edx, PCI_BASE_ADDRESS_0        ; find the base io address
  .sb_reg_check:
  ;
	movzx	eax, byte [ebx+device.pci_bus] ;
	movzx	ecx, byte [ebx+device.pci_dev] ;
					       ;
	push	edx ecx
	stdcall PciRead16, eax ,ecx ,edx       ;
	pop	ecx edx
					       ;
	mov	[ebx+device.io_addr], ax       ;
	and	eax, PCI_BASE_ADDRESS_IO_MASK  ;
	test	eax, eax		       ;
	jz	.sb_inc_reg		       ;
	movzx	eax, [ebx+device.io_addr]      ;
	and	eax, PCI_BASE_ADDRESS_SPACE_IO ;
	test	eax, eax		       ;
	jz	.sb_inc_reg		       ;
					       ;
	movzx	eax, [ebx+device.io_addr]      ;
	and	eax, PCI_BASE_ADDRESS_IO_MASK  ;
	mov	[ebx+device.io_addr], ax       ;
					       ;
	jmp	.got_io 		       ;
					       ;
  .sb_inc_reg:				   ;
	add	edx, 4			       ;
	cmp	edx, PCI_BASE_ADDRESS_5        ;
	jbe	.sb_reg_check		       ;

  .got_io:
	movzx	eax, byte [ebx+device.pci_bus] ; find IRQ line
	movzx	ecx, byte [ebx+device.pci_dev] ;
	push	ebx
	stdcall PciRead8, eax ,ecx ,0x3c       ; 0x3c is the offset where irq can be found
	pop	ebx
	mov	byte [ebx+device.irq_line], al ;

	jmp	.hook

  .isa:

	mov	esi, ne2000_LIST
	mov	ecx, [ne2000_DEV]
	test	ecx, ecx
	jz	.firstdevice_isa
	mov	bx , [eax+1]
	mov	dl , [eax+3]
  .nextdevice_isa:
	lodsd
	cmp	bx , [eax + device.io_addr]
	jne	.maybenext
	cmp	dl , [eax + device.irq_line]
	je	find_device_num
  .maybenext:
	loop	.nextdevice_isa



  .firstdevice_isa:
	call	create_new_struct

	mov	eax, [edx+IOCTL.input]
	mov	cx , [eax+1]
	mov	[ebx+device.io_addr], cx
	mov	cl, [eax+3]
	mov	[ebx+device.irq_line], cl

  .hook:

	DEBUGF	2,"Hooking into device, dev:%x, bus:%x, irq:%x, addr:%x\n",[ebx+device.pci_dev]:1,[ebx+device.pci_bus]:1,[ebx+device.irq_line]:1,[ebx+device.io_addr]:4

	call	probe							; this function will output in eax
	test	eax, eax
	jnz	.err							; If an error occured, exit

	mov	eax, [ne2000_DEV]
	mov	[ne2000_LIST+4*eax], ebx
	inc	[ne2000_DEV]

	call	EthRegDev				     ; Register the device to kernel (ebx points to device struct)
	cmp	eax, -1
	jz	.err
	pop	ebp
	ret	4

  .err:
	stdcall KernelFree, ebx

	jmp	.fail

;------------------------------------------------------
  @@:
.fail:
	or	eax, -1
	pop	ebp
	ret	4

;------------------------------------------------------
endp


create_new_struct:

	cmp	[ne2000_DEV], MAX_ne2000
	jge	.fail

	push	edx
	stdcall KernelAlloc, device.size
	pop	edx
	test	eax, eax
	jz	.fail
	mov	ebx, eax

	mov	dword [ebx+device.reset], reset
	mov	dword [ebx+device.transmit], transmit
	mov	dword [ebx+device.get_MAC], read_mac
	mov	dword [ebx+device.set_MAC], write_mac
	mov	dword [ebx+device.unload], unload

	ret

.fail:
	add	esp, 4
	or	eax, -1
	ret

find_device_num:

	DEBUGF	1,"Trying to find device number of already registered device\n"
	mov	ebx, eax
	call	EthStruc2Dev						; This kernel procedure converts a pointer to device struct in ebx
									; into a device number in edi
	mov	eax, edi						; Application wants it in eax instead
	DEBUGF	1,"Kernel says: %u\n", eax
	ret


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Actual Hardware dependent code starts here                      ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;


unload:   ; TODO
	or	eax, -1
	ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;  probe: enables the device and clears the rx buffer
;;
;;  Destroys: eax, ebx, ecx, edx
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

probe:
	mov	ebp, ebx	;---

	mov	[ebp + device.vendor], VENDOR_NONE
	mov	[ebp + device.bmem], 0
	mov	ax, [ebp + device.io_addr]
	add	ax, NE_ASIC_OFFSET
	mov	[ebp + device.asic_base], ax

	DEBUGF	2,"Trying 16-bit mode\n"

	or	[ebp + device.flags], FLAG_16BIT or FLAG_PIO
	mov	[ebp + device.memsize], MEM_32768
	mov	[ebp + device.tx_start], 64
	mov	[ebp + device.rx_start], TXBUF_SIZE + 64

	set_io	0
	set_io	P0_DCR
	mov	al, DCR_WTS + DCR_FT1 + DCR_LS
	out	dx, al

	set_io	P0_PSTART
	mov	al, MEM_16384
	out	dx, al

	set_io	P0_PSTOP
	mov	al, MEM_32768
	out	dx, al

	mov	esi, test_data
	mov	bx, 16384
	mov	cx, 14
	call	eth_pio_write

	mov	bx, 16384
	mov	cx, 14
	lea	edi, [ebp + device.romdata]
	call	eth_pio_read

	lea	esi, [ebp + device.romdata]
	mov	edi, test_data
	mov	ecx, 13

     repz    cmpsb
     jz      ep_set_vendor


	DEBUGF	2,"Trying 8-bit mode\n"

	mov	[ebp + device.flags], FLAG_PIO
	mov	[ebp + device.memsize], MEM_16384
	mov	[ebp + device.tx_start], 32
	mov	[ebp + device.rx_start], TXBUF_SIZE + 32

	mov	dx, [ebp + device.asic_base]
	add	dx, NE_RESET

	in	al, dx
	out	dx, al

	in	al, 0x84

	set_io	0
	mov	al, CMD_RD2 + CMD_STP
	out	dx, al

	set_io	P0_RCR
	mov	al, RCR_MON
	out	dx, al

	set_io	P0_DCR
	mov	al, DCR_FT1 + DCR_LS
	out	dx, al

	set_io	P0_PSTART
	mov	al, MEM_8192
	out	dx, al

	set_io	P0_PSTOP
	mov	al, MEM_16384
	out	dx, al

	mov	esi, test_data
	mov	bx, 8192
	mov	cx, 14
	call	eth_pio_write

	mov	bx, 8192
	mov	cx, 14
	lea	edi, [ebp + device.romdata]
	call	eth_pio_read

	mov	esi, test_data
	lea	edi, [ebp + device.romdata]
	mov	ecx, 13

    repz      cmpsb
    jz	      ep_set_vendor

	DEBUGF	2,"This is not a valid ne2000 device!\n"
	or	eax, -1
	ret


ep_set_vendor:

	cmp	[ebp + device.io_addr], ISA_MAX_ADDR
	jbe	ep_001

	DEBUGF	2,"Card is using PCI bus\n"

;;;        or      [ebp + device.flags], FLAG_16BIT

ep_001:
	mov	[ebp + device.vendor], VENDOR_NOVELL

ep_check_have_vendor:

	mov	ebx, ebp	;----

	mov	al, [ebp + device.vendor]
	cmp	al, VENDOR_NONE
  ;;;;      je      rtl8029_exit

	cmp	al, VENDOR_3COM
	je	reset

	mov	eax, [ebp + device.bmem]
	mov	[ebp + device.rmem], eax

	;-- hack
	mov	ebx, ebp
	call	read_mac


	push	.hack
	sub	esp, 6
	mov	edi, esp
	lea	esi, [ebp + device.mac]
	movsd
	movsw

	jmp	write_mac
       .hack:
	;--- hack


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;   reset: Place the chip into a virgin state
;;
;;   Destroys: eax, ebx, ecx, edx
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

reset:
	mov	ebp, ebx	;---


	DEBUGF	2,"Resetting rtl8029\n"

; attach int handler
	movzx	eax, [ebp+device.irq_line]
	DEBUGF	1,"Attaching int handler to irq %x\n",eax:1
	stdcall AttachIntHandler, eax, int_handler, dword 0

; Stop mode

	set_io	0
	mov	al, CMD_PS0 + CMD_RD2 + CMD_STP
	out	dx, al

	set_io	P0_DCR
	test	[ebp + device.flags], FLAG_16BIT
	jz	nsr_001

	mov	al, 0x49
	jmp	nsr_002

nsr_001:
	mov	al, 0x48

nsr_002:
	out	dx, al


;clear remote bytes count
	set_io	0

	xor	al, al

	set_io	P0_RBCR0
	out	dx, al

	set_io	P0_RBCR1
	out	dx, al


;initialize Receive configuration register
	set_io	P0_RCR
	mov	al, 0x20	; monitor mode
	out	dx, al


; transmit configuration register
	set_io	P0_TCR
	mov	al, 2		; internal loopback
	out	dx, al


; transmit page stuff
	set_io	P0_TPSR
	mov	al, [ebp + device.tx_start]
	out	dx, al

; set receive control register ;;;;
	set_io	P0_RCR
	mov	al, 4		; accept broadcast
	out	dx, al

; pagestart
	set_io	P0_PSTART
	mov	al, [ebp + device.rx_start]
	out	dx, al

; pagestop
	set_io	P0_PSTOP
	mov	al, [ebp + device.memsize]
	out	dx, al

; page boundary
	set_io	P0_BOUND
	mov	al, [ebp + device.memsize]
	dec	al
	out	dx, al


;;clear IRQ mask
;        set_io  P0_IMR
;        xor     al, al
;        out     dx, al

	set_io	0
	mov	al, CMD_PS1 + CMD_RD2 + CMD_STP ; page 1, stop mode
	out	dx, al

	set_io	P1_CURR
	mov	al, [ebp + device.rx_start]
	out	dx, al

	set_io	0
	mov	al, CMD_PS0 + CMD_RD2 + CMD_STA ; go to page 0
	out	dx, al

; Read MAC address
	mov	ebx, ebp	;----
	call	read_mac

; clear interupt status
	set_io	0
	set_io	P0_ISR
	mov	al, 0xff
	out	dx, al

; set IRQ mask
	set_io	P0_IMR
	mov	al, IRQ_MASK
	out	dx, al

;; start mode
;        set_io  0
;        mov     al, CMD_STA
;        out     dx, al

; clear transmit control register
	set_io	P0_TCR
	mov	al, 0		; no loopback
	out	dx, al


; Indicate that we have successfully reset the card
	DEBUGF	2,"Done!\n"
	xor	eax, eax

	mov	ebx, ebp	;------

	ret



;***************************************************************************
;   Function
;      transmit
; buffer in [esp], size in [esp+4], pointer to device struct in ebx
;***************************************************************************

align 4
transmit:
	mov	ebp, ebx

	mov	esi, [esp]
	mov	ecx, [esp + 4]
	DEBUGF	2,"Transmitting packet, buffer:%x, size:%u\n",esi, ecx
	DEBUGF	2,"To: %x-%x-%x-%x-%x-%x From: %x-%x-%x-%x-%x-%x Type:%x%x\n",[esi+0]:2,[esi+1]:2,[esi+2]:2,[esi+3]:2,[esi+4]:2,[esi+5]:2,[esi+6]:2,[esi+7]:2,[esi+8]:2,[esi+9]:2,[esi+10]:2,[esi+11]:2,[esi+13]:2,[esi+12]:2

	cmp	dword [esp+4], ETH_FRAME_LEN
	jg	.finish ; packet is too long
	cmp	dword [esp+4], 60
	jl	.finish ; packet is too short

	xor	bl, bl
	mov	bh, [ebp + device.tx_start]
	push	cx
	call	eth_pio_write
	pop	cx

	set_io	0
	mov	al, CMD_PS0 + CMD_RD2 + CMD_STA
	out	dx, al

	set_io	P0_TPSR
	mov	al, [ebp + device.tx_start]
	out	dx, al

	set_io	P0_TBCR0
	mov	al, cl
	out	dx, al

	set_io	P0_TBCR1
	mov	al, ch
	out	dx, al

	set_io	0
	mov	al, CMD_PS0 + CMD_TXP + CMD_RD2 + CMD_STA
	out	dx, al

	DEBUGF	2," - Packet Sent!\n"
.finish:
	mov	ebx, ebp

	call	KernelFree
	add	esp, 4 ; pop (balance stack)
	xor	eax, eax

	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Interrupt handler
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
int_handler:

	DEBUGF	2,"IRQ %x ",eax:2

; find pointer of device wich made INT occur
	mov	esi, ne2000_LIST
	mov	ecx, [ne2000_DEV]
.nextdevice:
	mov	ebp, dword [esi]

	set_io	0		; We chould check ISR instead..
	set_io	P0_ISR
	in	al, dx

	DEBUGF	2,"isr %x ",eax:2

	test	al, ISR_PRX
	jnz	.rx

	add	esi, 4

	loop	.nextdevice
	ret

; looks like we've found it!
.rx:


	sub	esp, 14+8			  ; allocate memory for temp variables in stack

	eth_type	equ esp
	pkthdr		equ esp + 2
	pktoff		equ esp + 6
	eth_rx_data_ptr equ esp + 8
	eth_tmp_len	equ esp + 12

	pointer 	equ esp + 14
	size		equ esp + 18

	stdcall KernelAlloc, ETH_FRAME_LEN
	mov	[pointer], eax
	mov	[eth_rx_data_ptr], eax

	set_io	P0_BOUND
	in	al, dx
	inc	al

	cmp	al, [ebp + device.memsize]
	jb	.nsp_001

	mov	al, [ebp + device.rx_start]

.nsp_001:
	mov	ch, al

	set_io	0
	mov	al, CMD_PS1
	out	dx, al

	set_io	P1_CURR
	in	al, dx		     ; get current page
	mov	cl, al

	set_io	0
	mov	al, CMD_PS0
	out	dx, al

	cmp	cl, [ebp + device.memsize]
	jb	.nsp_002

	mov	cl, [ebp + device.rx_start]

.nsp_002:
	cmp	cl, ch
	je	.fail

	xor	ax, ax
	mov	ah, ch

	mov	[pktoff], ax

	mov	al, [ebp + device.flags]
	test	al, FLAG_PIO
	jz	.nsp_003

	mov	bx, word [pktoff]
	lea	edi, [pkthdr]
	mov	cx, 4
	call	eth_pio_read
	jmp	.nsp_004

.nsp_003:
	mov	edi, [ebp + device.rmem]
	movzx	eax, word [pktoff]
	add	edi, eax
	mov	eax, [edi]
	mov	[pkthdr], eax

.nsp_004:
	add	word[pktoff] , 4

	xor	eax, eax
	mov	ax, [pkthdr + 2]
	sub	ax, 4

	DEBUGF	2,"Received %u bytes\n",eax

	mov	[eth_tmp_len], ax
	mov	dword[size], eax

	cmp	ax, ETH_ZLEN
	jb	.fail

	cmp	ax, ETH_FRAME_LEN
	ja	.fail

	mov	al, [pkthdr]
	test	al, RSTAT_PRX
	jz	.fail

   ; Right, we can now get the data

	mov	bh, [ebp + device.memsize]
	sub	bx, [pktoff]

	cmp	[eth_tmp_len], bx
	jbe	.nsp_005

	mov	al, [ebp + device.flags]
	test	al, FLAG_PIO
	jz	.nsp_006

	push	bx
	mov	cx, bx
	mov	bx, [pktoff]
	mov	edi, [eth_rx_data_ptr]
	call	eth_pio_read
	pop	bx
	jmp	.nsp_007

.nsp_006:
   ; Not implemented, as we are using PIO mode on this card

.nsp_007:
	xor	al, al
	mov	ah, [ebp + device.rx_start]
	mov	[pktoff], ax

	add	[eth_rx_data_ptr], ebx

	mov	ax, [eth_tmp_len]
	sub	ax, bx
	mov	[eth_tmp_len], ax

.nsp_005:
	test	[ebp + device.flags], FLAG_PIO
	jz	.nsp_008

	xor	ebx, ebx
	mov	bx, [pktoff]
	xor	ecx, ecx
	mov	cx, [eth_tmp_len]
	mov	edi, [eth_rx_data_ptr]
	call	eth_pio_read
	jmp	.nsp_009

.nsp_008:
   ; Not implemented, as we are using PIO mode on this card

.nsp_009:
	mov	al, [pkthdr+1]
	cmp	al, [ebp + device.rx_start]
	jne	.nsp_010

	mov	al, [ebp + device.memsize]

.nsp_010:
	set_io	0
	set_io	P0_BOUND
	dec	al
	out	dx, al

	add	esp, 14

	mov	ebx, ebp
	jmp	EthReceiver	;;;

.fail:
	add	esp, 14+8
	DEBUGF	2,"done\n"
ret





;;;;;;;;;;;;;;;;;;;;;;;
;;                   ;;
;; Write MAC address ;;
;;                   ;;
;;;;;;;;;;;;;;;;;;;;;;;

align 4
write_mac:	; in: mac on stack

	mov	ebp, ebx	;---

	DEBUGF	1,"Writing MAC: "

	set_io	0
	mov	al, CMD_PS1; + CMD_RD2 + CMD_STP
	out	dx, al

	set_io	P1_PAR0
	mov	esi, esp
	mov	cx, 6
 @@:
	lodsb
	out	dx, al
	inc	dx
	loopw	@r

	add	esp, 6

	mov	ebx, ebp	;---

; Notice this procedure does not ret, but continues to read_mac instead.

;;;;;;;;;;;;;;;;;;;;;;
;;                  ;;
;; Read MAC address ;;
;;                  ;;
;;;;;;;;;;;;;;;;;;;;;;

read_mac:

	mov	ebp, ebx	;-----

	DEBUGF	1,"Reading MAC: "

;        set_io  0
;        mov     al, CMD_PS1; + CMD_RD2 + CMD_STP ; select page 1
;        out     dx, al
;
;        set_io  P1_PAR0
;        lea     edi, [ebp + device.mac]
;
;        mov     cx, 6
; .loop:
;        in      al, dx
;        stosb
;        inc     dx
;        loopw   .loop
;
;        lea     edi, [ebp + device.mac]
;        DEBUGF  1,"%x-%x-%x-%x-%x-%x\n",[edi+0]:2,[edi+1]:2,[edi+2]:2,[edi+3]:2,[edi+4]:2,[edi+5]:2
;
;        set_io  0
;        mov     al, CMD_PS0; + CMD_RD2 + CMD_STA  ; set page back to 0
;        out     dx, al


	mov	bx, 0
	mov	cx, 16
	lea	edi, [ebp + device.romdata]
	call	eth_pio_read

	lea	esi, [ebp + device.romdata]
	lea	edi, [ebp + device.mac]
	mov	ecx, 6

  .loop:
	movsb
	test	[ebp + device.flags], FLAG_16BIT
	jz	.8bit
	inc	esi
  .8bit:
	loop	.loop


	mov	ebx, ebp	;---

	ret


;***************************************************************************
;   Function
;      eth_pio_read
;
;   Description
;       Read a frame from the ethernet card via Programmed I/O
;      src in bx
;      cnt in cx
;       dst in edi
;***************************************************************************
eth_pio_read:

	DEBUGF	1,"Eth PIO Read from %x to %x, %u bytes ",bx,edi,cx

	set_io	0
	mov	al, CMD_RD2 + CMD_STA
	out	dx, al

	mov	al, cl
	set_io	P0_RBCR0
	out	dx, al

	mov	al, ch
	set_io	P0_RBCR1
	out	dx, al

	mov	al, bl
	set_io	P0_RSAR0
	out	dx, al

	mov	al, bh
	set_io	P0_RSAR1
	out	dx, al

	mov	al, CMD_RD0 + CMD_STA
	set_io	0
	out	dx, al

	mov	dx, [ebp + device.asic_base]

	test	[ebp+device.flags], FLAG_16BIT
	jz	epr_003

	DEBUGF	1,"in 16-bits mode"

	shr	cx, 1	; note that if the number was odd, carry flag will be set
	pushf		; save the flags for later

epr_002:
	in	ax, dx
	stosw
	loopw	epr_002

	inc	cx
	popf
	jnc	epr_004

epr_003:
	in	al, dx
	stosb
	loopw	epr_003


epr_004:
	set_io	0
	set_io	P0_ISR

epr_005:				; Wait for Remote DMA Complete
	in	al, dx
	test	al, ISR_RDC
	jz	epr_005
;        and     al, not ISR_RDC
	out	dx, al			; clear the bit


	DEBUGF	1,"\n"
	ret




;***************************************************************************
;   Function
;      eth_pio_write
;
;   Description
;       writes a frame to the ethernet card via Programmed I/O
;      dst in bx
;      cnt in cx
;       src in esi
;***************************************************************************
eth_pio_write:

	DEBUGF	1,"Eth PIO Write from %x to %x, %u bytes ",esi,bx,cx

	set_io	0
	mov	al, CMD_RD2 + CMD_STA
	out	dx, al

	set_io	P0_ISR
	mov	al, ISR_RDC
	out	dx, al

	set_io	P0_RBCR0
	mov	al, cl
	out	dx, al

	set_io	P0_RBCR1
	mov	al, ch
	out	dx, al

	set_io	P0_RSAR0
	mov	al,   bl
	out	dx, al

	set_io	P0_RSAR1
	mov	al,   bh
	out	dx, al

	set_io	0
	mov	al, CMD_RD1 + CMD_STA
	out	dx, al

	mov	dx, [ebp + device.asic_base]
	test	[ebp + device.flags], FLAG_16BIT
	jz	epw_003

	DEBUGF	1,"in 16-bits mode"

	shr	cx, 1	; note that if the number was odd, carry flag will be set
	pushf		; save the flags for later

epw_002:
	lodsw
	out	dx, ax
	loopw	epw_002

	popf
	jnc	epw_004
	inc	cx

epw_003:
	lodsb
	out	dx, al
	loopw	epw_003

epw_004:
	set_io	0
	set_io	P0_ISR

epw_005:				; Wait for Remote DMA Complete
	in	al, dx
	test	al, ISR_RDC
	jz	epw_005
;        and     al, not ISR_RDC
	out	dx, al			; clear the bit


	DEBUGF	1,"\n"
	ret



;all initialized data place here
align 4

ne2000_DEV	dd 0
version 	dd (5 shl 16) or (API_VERSION and 0xFFFF)
my_service	db 'ne2000',0  ;max 16 chars include zero
devicename	db 'Realtek 8029',0
		db 'Realtek 8019',0
		db 'Realtek 8019AS',0
		db 'ne2000',0
		db 'DP8390',0

test_data	db 'NE*000 memory',0
;test_buffer     db '             ',0

include_debug_strings

section '.data' data readable writable align 16

;place all uninitialized data place here

ne2000_LIST:
rd MAX_ne2000





