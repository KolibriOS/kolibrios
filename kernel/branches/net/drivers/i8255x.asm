;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;; I8255x (Intel eepro 100) driver for KolibriOS                   ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;    v0.0 - march 2009                                            ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;; current status (september 2009) - INCOMPLETE                    ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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
; status
      .bytes_tx 	dd ?
      .bytes_rx 	dd ?
      .packets_tx	dd ?
      .packets_rx	dd ?
      .mode		dd ?  ; This dword contains cable status (10mbit/100mbit, full/half duplex, auto negotiation or not,..)
      .name		dd ?
; device specific
      .io_addr		dd ?
      .pci_bus		db ?
      .pci_dev		db ?
      .irq_line 	db ?
      .status		dw ? ;
      .command		dw ? ;
      .link		dd ? ;
      .rx_buf_addr	dd ? ;
      .count		dw ? ;
      .size		dw ? ;
      .packet		dd ? ;
      .eeprom_data	rd 16

	.txfd:
	.txfd_status		 dw  ?
	.txfd_command		 dw  ?
	.txfd_link		 dd  ?
	.txfd_tx_desc_addr	 dd  ?
	.txfd_count		 dd  ?
	.txfd_tx_buf_addr0	 dd  ?
	.txfd_tx_buf_size0	 dd  ?
	.txfd_tx_buf_addr1	 dd  ?
	.txfd_tx_buf_size1	 dd  ?

      .size_:

}

virtual at 0
  device ETH_DEVICE
end virtual


lstats:
tx_good_frames: 	dd 0
tx_coll16_errs: 	dd 0
tx_late_colls:		dd 0
tx_underruns:		dd 0
tx_lost_carrier:	dd 0
tx_deferred:		dd 0
tx_one_colls:		dd 0
tx_multi_colls: 	dd 0
tx_total_colls: 	dd 0

rx_good_frames: 	dd 0
rx_crc_errs:		dd 0
rx_align_errs:		dd 0
rx_resource_errs:	dd 0
rx_overrun_errs:	dd 0
rx_colls_errs:		dd 0
rx_runt_errs:		dd 0

done_marker:		dd 0


confcmd:
	.status:	 dw  0
	.command:	 dw  0
	.link:		 dd  0

confcmd_data:		db  22, 0x08, 0, 0, 0, 0x80, 0x32, 0x03, 1
			db  0, 0x2e, 0, 0x60, 0, 0xf2, 0x48, 0, 0x40, 0xf2
			db  0x80, 0x3f, 0x05


	MAX_I8255x		equ 16	 ; Max number of devices this driver may handle


; PCI Bus defines

	PCI_HEADER_TYPE 		equ	0x0e  ;8 bit
	PCI_BASE_ADDRESS_0		equ	0x10  ;32 bit
	PCI_BASE_ADDRESS_5		equ	0x24  ;32 bits
	PCI_BASE_ADDRESS_SPACE_IO	equ	0x01
	PCI_VENDOR_ID			equ	0x00  ;16 bit
	PCI_BASE_ADDRESS_IO_MASK	equ	0xFFFFFFFC



;/***********************************************************************/
;/*                       I82557 related defines                        */
;/***********************************************************************/

; Serial EEPROM section.
;   A "bit" grungy, but we work our way through bit-by-bit :->.
;  EEPROM_Ctrl bits.
EE_SHIFT_CLK   equ   0x01    ; EEPROM shift clock.
EE_CS	       equ   0x02    ; EEPROM chip select.
EE_DATA_WRITE  equ   0x04    ; EEPROM chip data in.
EE_DATA_READ   equ   0x08    ; EEPROM chip data out.
EE_WRITE_0     equ   0x4802
EE_WRITE_1     equ   0x4806
EE_ENB	       equ   0x4802


; The EEPROM commands include the alway-set leading bit.
EE_READ_CMD    equ   6

; The SCB accepts the following controls for the Tx and Rx units:
CU_START       equ   0x0010
CU_RESUME      equ   0x0020
CU_STATSADDR   equ   0x0040
CU_SHOWSTATS   equ   0x0050   ; Dump statistics counters.
CU_CMD_BASE    equ   0x0060   ; Base address to add to add CU commands.
CU_DUMPSTATS   equ   0x0070   ; Dump then reset stats counters.

RX_START       equ   0x0001
RX_RESUME      equ   0x0002
RX_ABORT       equ   0x0004
RX_ADDR_LOAD   equ   0x0006
RX_RESUMENR    equ   0x0007
INT_MASK       equ   0x0100
DRVR_INT       equ   0x0200   ; Driver generated interrupt.






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

	DEBUGF 1,"Loading I8255x driver\n"
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
	DEBUGF 1,"esp=%x [esp]=%x\n",esp,eax

	cmp	[edx + IOCTL.inp_size], 3		; Data input must be at least 3 bytes
	jl	.fail

	mov	eax, [edx + IOCTL.input]
	cmp	byte [eax], 1				; 1 means device number and bus number (pci) are given
	jne	.fail					; other types arent supported for this card yet

; check if the device is already listed

	mov	esi, I8255x_LIST
	mov	ecx, [I8255x_DEV]
	test	ecx, ecx
	jz	.firstdevice
	mov	eax, [edx+IOCTL.input]			; get the pci bus and device numbers
	mov	bx , [eax+1]				;
  .nextdevice:
	DEBUGF 1,"1"
	lodsd
	cmp	bx , word [eax + device.pci_bus]	; compare with pci and device num in RTL8139 list (notice the usage of word instead of byte)
	je	.find_devicenum 			; Device is already loaded, let's find it's device number

	loop	.nextdevice

; This device doesnt have its own eth_device structure yet, lets create one
  .firstdevice:
	cmp	[I8255x_DEV], MAX_I8255x		; First check if the driver can handle one more card
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
;        mov     dword [ebx+device.get_MAC], read_mac
;        mov     dword [ebx+device.set_MAC], write_mac
	mov	dword [ebx+device.unload], unload
	mov	dword [ebx+device.name], devicename

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

	stdcall KernelAlloc, dword (TX_BUFFER_SIZE)
	test	eax, eax
	jz	.err
	mov	[ebx+device.tx_buffer], eax

; This one needs to be cleared too..

	mov	edi, eax
	mov	ecx, (TX_BUFFER_SIZE)/4
	xor	eax, eax
	rep	stosd

; Ok, the eth_device structure is ready, let's probe the device

	call	probe							; this function will output in eax
	test	eax, eax
	jnz	.err							; If an error occured, exit

	mov	eax, [I8255x_DEV]					; Add the device structure to our device list
	mov	[I8255x_LIST+4*eax], ebx				; (IRQ handler uses this list to find device)
	inc	[I8255x_DEV]						;

	jmp	EthRegDev						; Register the device to kernel (ebx points to device struct)
									; Notice the jump instead of call, it is the same as
									;
									; call EthRegDev
									; ret
									;
									; Also notice that the value EthRegDev returned in eax, will be returned to
									; the caller application

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

;***************************************************************************
;   Function
;      I8255x_probe
;   Description
;      Searches for an ethernet card, enables it and clears the rx buffer
;      If a card was found, it enables the ethernet -> TCPIP link
;
;***************************************************************************

probe:

	DEBUGF 1," Probing i8255x device: "

; enable the device

	movzx	eax, byte [ebx+device.pci_bus]
	movzx	ecx, byte [ebx+device.pci_dev]
	stdcall PciRead16, eax ,ecx ,PCI_REG_CMD

	mov	cx , ax
	or	cx , 0x05
	movzx	eax, byte [ebx+device.pci_bus]
	movzx	edx, byte [ebx+device.pci_dev]
	stdcall PciWrite16, eax ,edx ,PCI_REG_CMD, ecx

; do something else TODO

	mov	ebx, 0x6000000
	mov	ecx, 27
	call	do_eeprom_cmd
	and	eax, 0xffe0000
	cmp	eax, 0xffe0000
	je	bige

	mov	ebx, 0x1800000
	mov	ecx, 0x40
	jmp	doread

bige:
	mov	ebx, 0x6000000
	mov	ecx, 0x100

doread:

	; do-eeprom-cmd will destroy all registers
	; we have eesize in ecx
	; read_cmd in ebx

	; Ignore full eeprom - just load the mac address
	mov	ecx, 0

drlp:


	push	ecx	 ; save count
	push	ebx
	mov	eax, ecx
	shl	eax, 16
	or	ebx, eax
	mov	ecx, 27
	call	do_eeprom_cmd

	pop	ebx
	pop	ecx

	mov	edx, ecx
	shl	edx, 2
	mov	esi, eeprom_data
	add	esi, edx
	mov	[esi], eax

	inc	ecx
	cmp	ecx, 16
	jne	drlp

	; OK, we have the MAC address.

;***************************************************************************
;   Function
;      I8255x_reset
;   Description
;      Place the chip (ie, the ethernet card) into a virgin state
;      No inputs
;      All registers destroyed
;
;***************************************************************************

reset:

	; Now reset the card

	mov	edx, [ebx+device.io_addr]
	add	dx, 8	      ; SCBPort
	xor	eax, eax      ; The reset cmd == 0
	out	dx, eax

	mov	esi, 10
	call	delay_ms      ; Give the card time to warm up.

	mov	eax, lstats
	mov	edx, [ebx+device.io_addr]
	add	edx, 4		  ; SCBPointer
	out	dx, eax

	mov	eax, 0x0140	    ; INT_MASK | CU_STATSADDR
	mov	edx, [ebx+device.io_addr]
	add	edx, 2		  ; SCBCmd
	out	dx, ax

	call	wait_for_cmd_done

	mov	eax, 0
	mov	edx, [ebx+device.io_addr]
	add	edx, 4		  ; SCBPointer
	out	dx, eax

	mov	eax, 0x0106	    ; INT_MASK | RX_ADDR_LOAD
	mov	edx, [ebx+device.io_addr]
	add	edx, 2		  ; SCBCmd
	out	dx, ax

	call	wait_for_cmd_done
   ; build rxrd structure
	mov	ax, 0x0001
	mov	[ebx+device.status], ax
	mov	ax, 0x0000
	mov	[rxfd_command], ax

	mov	eax, rxfd_status
	sub	eax, OS_BASE
	mov	[ebx+device.link], eax

	mov	eax, Ether_buffer
	sub	eax, OS_BASE
	mov	[ebx+device.rx_buf_addr], eax

	mov	ax, 0
	mov	[ebx+device.count], ax

	mov	ax, 1528
	mov	[ebx+device.size], ax

	mov	edx, [ebx+device.io_addr]
	add	edx, 4		 ; SCBPointer

	mov	eax, rxfd_status
	sub	eax, OS_BASE
	out	dx, eax

	mov	edx, [ebx+device.io_addr]
	add	edx, 2		 ; SCBCmd

	mov	ax, 0x0101	   ; INT_MASK | RX_START
	out	dx, ax

	call	wait_for_cmd_done

   ; start the receiver

	mov	ax, 0
	mov	[ebx+device.status], ax

	mov	ax, 0xc000
	mov	[ebx+device.command], ax

	mov	edx, [ebx+device.io_addr]
	add	edx, 4		 ; SCBPointer

	mov	eax, rxfd_status
	sub	eax, OS_BASE
	out	dx, eax

	mov	edx, [ebx+device.io_addr]
	add	edx, 2		 ; SCBCmd

	mov	ax, 0x0101	   ; INT_MASK | RX_START
	out	dx, ax

   ; Init TX Stuff

	mov	edx, [ebx+device.io_addr]
	add	edx, 4		 ; SCBPointer

	mov	eax, 0
	out	dx, eax

	mov	edx, [ebx+device.io_addr]
	add	edx, 2		 ; SCBCmd

	mov	ax, 0x0160	   ; INT_MASK | CU_CMD_BASE
	out	dx, ax

	call	wait_for_cmd_done

   ; Set TX Base address

   ; First, set up confcmd values

	mov	ax, 2
	mov	[confcmd_command], ax
	mov	eax, txfd
	sub	eax, OS_BASE
	mov	[confcmd_link], eax

	mov	ax, 1
	mov	[txfd_command], ax	   ; CmdIASetup

	mov	ax, 0
	mov	[txfd_status], ax

	mov	eax, confcmd
	sub	eax, OS_BASE
	mov	[txfd_link], eax



   ; ETH_ALEN is 6 bytes

	mov	esi, eeprom_data
	mov	edi, node_addr
	mov	ecx, 3

drp000:

	mov	eax, [esi]
	mov	[edi], al
	shr	eax, 8
	inc	edi
	mov	[edi], al
	inc	edi
	add	esi, 4
	loop	drp000

   ; Hard code your MAC address into node_addr at this point,
   ; If you cannot read the MAC address from the eeprom in the previous step.
   ; You also have to write the mac address into txfd_tx_desc_addr, rather
   ; than taking data from eeprom_data

	mov	esi, eeprom_data
	mov	edi, txfd_tx_desc_addr
	mov	ecx, 3

drp001:

	mov	eax, [esi]
	mov	[edi], al
	shr	eax, 8
	inc	edi
	mov	[edi], al
	inc	edi
	add	esi, 4
	loop	drp001

	mov	esi, eeprom_data + (6 * 4)
	mov	eax, [esi]
	shr	eax, 8
	and	eax, 0x3f
	cmp	eax, 4		  ; DP83840
	je	drp002
	cmp	eax, 10 	   ; DP83840A
	je	drp002
	jmp	drp003

drp002:

	mov	ebx, [esi]
	and	ebx, 0x1f
	push	ebx
	mov	ecx, 23
	call	mdio_read
	pop	ebx
	or	eax, 0x0422
	mov	ecx, 23
	mov	edx, eax
	call	mdio_write

drp003:

	mov	ax, 0x4002	   ; Cmdsuspend | CmdConfigure
	mov	[confcmd_command], ax
	mov	ax, 0
	mov	[confcmd_status], ax
	mov	eax, txfd
	mov	[confcmd_link], eax
	mov	ebx, confcmd_data
	mov	al, 0x88	 ; fifo of 8 each
	mov	[ebx + 1], al
	mov	al, 0
	mov	[ebx + 4], al
	mov	al, 0x80
	mov	[ebx + 5], al
	mov	al, 0x48
	mov	[ebx + 15], al
	mov	al, 0x80
	mov	[ebx + 19], al
	mov	al, 0x05
	mov	[ebx + 21], al

	mov	eax, txfd
	sub	eax, OS_BASE
	mov	edx, [ebx+device.io_addr]
	add	edx, 4		  ; SCBPointer
	out	dx, eax

	mov	eax, 0x0110	    ; INT_MASK | CU_START
	mov	edx, [ebx+device.io_addr]
	add	edx, 2		  ; SCBCmd
	out	dx, ax

	call	wait_for_cmd_done

jmp skip

   ; wait for thing to start
drp004:

	mov	ax, [txfd_status]
	test	ax, ax
	jz	drp004

skip:
   ; Indicate that we have successfully reset the card

	xor	eax, eax

	ret


;***************************************************************************
;   Function
;      I8255x_transmit
;
;   Description
;       Transmits a packet of data via the ethernet card
;          Pointer to 48 bit destination address in edi
;         Type of packet in bx
;         size of packet in ecx
;         pointer to packet data in esi
;
;***************************************************************************

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

;        mov     [hdr_type], bx

;        mov     eax, [edi]
;        mov     [hdr_dst_addr], eax
;        mov     ax, [edi+4]
;        mov     [hdr_dst_addr+4], ax

;        mov     eax, [node_addr]
;        mov     [hdr_src_addr], eax
;        mov     ax, [node_addr+4]
;        mov     [hdr_src_addr+4], ax

	mov	edx, [ebx+device.io_addr]
	in	ax, dx
	and	ax, 0xfc00
	out	dx, ax

	mov	[ebx+device.txfd_status], 0
	mov	[ebx+device.txfd_command], 0x400C		  ; Cmdsuspend | CmdTx | CmdTxFlex
	lea	eax, [ebx+device.txfd]
	mov	[txfd_link], eax
	mov	[txfd_count], 0x02208000
	mov	eax, txfd_tx_buf_addr0
	call	GetPgAddr
	mov	[txfd_tx_desc_addr], eax
	mov	eax, hdr
	call	GetPgAddr
	mov	[txfd_tx_buf_addr0], eax
	mov	eax, 14   ; sizeof hdr
	mov	[txfd_tx_buf_size0], eax

	; Copy the buffer address and size in
;        mov     eax, esi
;        sub     eax, OS_BASE
;        mov     [txfd_tx_buf_addr1], eax
;        mov     eax, ecx
;        mov     [txfd_tx_buf_size1], eax

	mov	eax, ebx
	call	GetPgAddr
	add	eax, device.txfd
	mov	edx, [ebx+device.io_addr]
	add	edx, 4		  ; SCBPointer
	out	dx, eax

	mov	ax, 0x0110	   ; INT_MASK | CU_START
	mov	edx, [ebx+device.io_addr]
	add	edx, 2		  ; SCBCmd
	out	dx, ax

	call	wait_for_cmd_done

	mov	edx, [ebx+device.io_addr]
	in	ax, dx

I8t_001:

	mov	ax, [ebx+device.txfd_status]
	cmp	ax, 0
	je	I8t_001

	mov	edx, [ebx+device.io_addr]
	in	ax, dx

  .finish:

	ret



;***************************************************************************
; Function
;    I8255x_poll
;
; Description
;    Polls the ethernet card for a received packet
;    Received data, if any, ends up in Ether_buffer
;
;***************************************************************************
I8255x_poll:

	mov	ax, 0	   ; assume no data
	mov	[eth_rx_data_len], ax

	mov	ax, [rxfd_status]
	cmp	ax, 0
	je	i8p_exit

	mov	ax, 0
	mov	[rxfd_status], ax

	mov	ax, 0xc000
	mov	[rxfd_command], ax

	mov	edx, [io_addr]
	add	edx, 4		 ; SCBPointer

	mov	eax, rxfd_status
	sub	eax, OS_BASE
	out	dx, eax

	mov	edx, [ebx+device.io_addr]
	add	edx, 2		 ; SCBCmd

	mov	ax, 0x0101	   ; INT_MASK | RX_START
	out	dx, ax

	call	wait_for_cmd_done

	mov	esi, rxfd_packet
	mov	edi, Ether_buffer
	mov	ecx, 1518
	cld
	rep	movsb

	mov	ax, [rxfd_count]
	and	ax, 0x3fff
	mov	[eth_rx_data_len], ax

i8p_exit:
   ret




;***************************************************************************
;   Function
;      wait_for_cmd_done
;
;   Description
;       waits for the hardware to complete a command
;       port address in edx
;
;       al destroyed
;***************************************************************************

wait_for_cmd_done:

;        mov     edx, [ebx + device.io_addr]

  .loop:
	in	al , dx
	test	al , al
	jnz	.loop

	ret



;***************************************************************************
;   Function
;      mdio_read
;
;   Description
;       This probably reads a register in the "physical media interface chip"
;         Phy_id in ebx   NOW EAX
;         location in ecx
;
;       Data returned in eax
;
;***************************************************************************

mdio_read:

	mov	edx, [ebx + device.io_addr]
	add	edx, 16 	; SCBCtrlMDI

;        mov     eax, 0x08000000
;        shl     ecx, 16
;        or      eax, ecx
;        shl     ebx, 21
;        or      eax, ebx
				   ;
	shl	ecx, 16 	   ;
	shl	eax, 21 	   ;
	or	eax, ecx	   ;
	or	eax, 0x08000000    ;
	out	dx , eax	   ;
				   ;
mrlp:
	call	delay_us
	in	eax, dx
	mov	ecx, eax
	and	ecx, 0x10000000
	jz	mrlp

	and	eax, 0xffff
	ret



;***************************************************************************
;   Function
;      mdio_write
;
;   Description
;       This probably writes a register in the "physical media interface chip"
;         Phy_id in ebx   NOW EAX
;         location in ecx
;         data in edx
;       Data returned in eax
;
;***************************************************************************

mdio_write:

;        mov     eax, 0x04000000
;        shl     ecx, 16
;        or      eax, ecx
;        shl     ebx, 21
;        or      eax, ebx
;        or      eax, edx

	shl	ecx, 16 	  ;
	shl	ebx, 21 	  ;
	or	eax, ecx	  ;
	or	eax, edx	  ;
	or	eax, 0x04000000   ;

	mov	edx, [ebx + device.io_addr]
	add	edx, 16 	; SCBCtrlMDI
	out	dx, eax

mwlp:
	call	delay_us
	in	eax, dx
	mov	ecx, eax
	and	ecx, 0x10000000
	jz	mwlp

	and	eax, 0xffff
	ret


;***************************************************************************
;   Function
;      do_eeprom_cmd
;
;   Description
;       writes a cmd to the ethernet cards eeprom, by bit bashing
;       cmd in ebx        NOW EAX
;       cmd length in ecx
;       return in eax
;***************************************************************************

do_eeprom_cmd:

	push	eax
	mov	edx, [ebx + device.io_addr]
	add	dx, 14		  ; the value SCBeeprom

	mov	ax, EE_ENB
	out	dx, ax
	call	delay_us

	mov	ax, 0x4803	  ; EE_ENB | EE_SHIFT_CLK
	out	dx, ax
	call	delay_us

	 ; dx holds ee_addr
	 ; ecx holds count
	 ; eax holds cmd
	xor	edi, edi	  ; this will be the receive data

dec_001:

	mov	esi, 1

	dec	ecx
	shl	esi, cl
	inc	ecx
	and	esi, [esp]
	mov	eax, EE_WRITE_0   ; I am assuming this doesnt affect the flags..
	cmp	esi, 0
	jz	dec_002
	mov	eax, EE_WRITE_1

dec_002:

	out	dx, ax
	call	delay_us

	or	ax, EE_SHIFT_CLK
	out	dx, ax
	call	delay_us

	shl	edi,1

	in	ax, dx
	and	ax, EE_DATA_READ
	cmp	ax,0
	jz	dec_003
	inc	edi

dec_003:

	loop	dec_001

	mov	ax, EE_ENB
	out	dx, ax
	call	delay_us

	mov	ax, 0x4800
	out	dx, ax
	call	delay_us

	add	esp, 4
	mov	eax, edi

	ret





; End of code

align 4 					; Place all initialised data here

I8255x_DEV    dd 0
version       dd (5 shl 16) or (API_VERSION and 0xFFFF)
my_service    db 'I8255x',0			; max 16 chars include zero
devicename    db 'Intel Etherexpress pro/100',0



include_debug_strings				; All data wich FDO uses will be included here

section '.data' data readable writable align 16 ; place all uninitialized data place here

I8255x_LIST rd I8255x			  ; This list contains all pointers to device structures the driver is handling

