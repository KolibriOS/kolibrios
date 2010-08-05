; PCI Bus defines
	PCI_HEADER_TYPE 		equ	0x0e  ;8 bit
	PCI_BASE_ADDRESS_0		equ	0x10  ;32 bit
	PCI_BASE_ADDRESS_5		equ	0x24  ;32 bits
	PCI_BASE_ADDRESS_SPACE_IO	equ	0x01
	PCI_VENDOR_ID			equ	0x00  ;16 bit
	PCI_BASE_ADDRESS_IO_MASK	equ	0xFFFFFFFC


; PCI programming
	PCI_REG_COMMAND 	equ 0x4 ; command register
	PCI_REG_STATUS		equ 0x6 ; status register
	PCI_REG_LATENCY 	equ 0xd ; latency timer register
	PCI_REG_CAP_PTR 	equ 0x34 ; capabilities pointer
	PCI_REG_CAPABILITY_ID	equ 0x0 ; capapility ID in pm register block
	PCI_REG_PM_STATUS	equ 0x4 ; power management status register
	PCI_REG_PM_CTRL 	equ 0x4 ; power management control register
	PCI_BIT_PIO		equ 1 ; bit0: io space control
	PCI_BIT_MMIO		equ 2 ; bit1: memory space control
	PCI_BIT_MASTER		equ 4 ; bit2: device acts as a PCI master


; Kernel variables

	PAGESIZE		equ 4096
	PG_SW			equ 0x003


; network driver types

	NET_TYPE_ETH		equ 1
	NET_TYPE_SLIP		equ 2



	LAST_IO = 0

macro set_io addr {

	if	addr = 0
	mov	edx, [device.io_addr]
	else if addr = LAST_IO
	else
	add	edx, addr - LAST_IO
	end if

	LAST_IO = addr
}

macro allocate_and_clear dest, size, err {

; We need to allocate at least 8 pages, if we want a continuous memory in ram
	push	edx
    if (size < 8*4096) & (size > 4096)
	stdcall KernelAlloc, 8*4096
    else
	stdcall KernelAlloc, size
    end if
	pop	edx

	test	eax, eax
	jz	err
	mov	dest, eax		; Save the address to it into the device struct
	mov	edi, eax		; look at last part of code!

; Release the unused pages (if any)
    if (size < 8*4096) & (size > 4096)
	add	eax, (size/4096+1)*4096
	mov	ecx, 8-(size/4096+1)
	push	edx
	call	ReleasePages
	pop	edx
    end if

; Clear the allocated buffer
	mov	ecx, size/4		; divide by 4 because of DWORD
	xor	eax, eax
	rep	stosd

}

macro find_io bus, dev, io {

	local	.check, .inc, .got

	xor	eax, eax
	mov	esi, PCI_BASE_ADDRESS_0
	movzx	ecx, bus
	movzx	edx, dev
  .check:
	stdcall PciRead32, ecx ,edx ,esi

	test	eax, PCI_BASE_ADDRESS_IO_MASK
	jz	.inc

	test	eax, PCI_BASE_ADDRESS_SPACE_IO
	jz	.inc

	and	eax, PCI_BASE_ADDRESS_IO_MASK
	mov	io , eax
	jmp	.got

  .inc:
	add	esi, 4
	cmp	esi, PCI_BASE_ADDRESS_5
	jle	.check

  .got:

}

macro find_irq bus, dev, irq {

	push	eax edx ecx
	movzx	ecx, bus
	movzx	edx, dev
	stdcall PciRead8, ecx ,edx ,0x3c				; 0x3c is the offset where irq can be found
	mov	irq, al
	pop	ecx edx eax

}

macro find_rev bus, dev, rev {

	push	eax edx ecx
	movzx	ecx, bus
	movzx	edx, dev
	stdcall PciRead8, ecx ,edx ,0x8
	mov	rev, al
	pop	ecx edx eax

}

macro make_bus_master bus, dev {

	movzx	ecx, bus
	movzx	edx, dev
	stdcall PciRead32, ecx ,edx, PCI_REG_COMMAND

	or	al, PCI_BIT_MASTER ;or PCI_BIT_PIO
;        and     al, not PCI_BIT_MMIO
	stdcall PciWrite32, ecx, edx, PCI_REG_COMMAND, eax

;; TODO: try to switch to PIO, and check if PIO works or not..

}

struc IOCTL {
      .handle		dd ?
      .io_code		dd ?
      .input		dd ?
      .inp_size 	dd ?
      .output		dd ?
      .out_size 	dd ?
}

virtual at edx
  IOCTL IOCTL
end virtual


if used null_op
align 4
null_op:
	or	eax, -1
	ret

end if


macro GetRealAddr { ; input and output is eax

	push	ax
	call	GetPgAddr
	and	word[esp], PAGESIZE - 1
	or	ax, word[esp]
	inc	esp
	inc	esp

}

macro NET_DEVICE {

	.type		dd ?	; Type field
	.mtu		dd ?	; Maximal Transmission Unit
	.name		dd ?	; Ptr to 0 terminated string

	.unload 	dd ?	; Ptrs to driver functions
	.reset		dd ?	;
	.transmit	dd ?	;

	.bytes_tx	dq ?	; Statistics, updated by the driver
	.bytes_rx	dq ?	;
	.packets_tx	dd ?	;
	.packets_rx	dd ?	;

	.end:
}


macro ETH_DEVICE {
      NET_DEVICE

	.set_mode	dd ?
	.get_mode	dd ?

	.set_MAC	dd ?
	.get_MAC	dd ?

	.mode		dd ?
	.mac		dp ?
			dp ?	; qword alignment

}



macro SLIP_DEVICE {
	NET_DEVICE

	.set_mode	dd ?
	.get_mode	dd ?

	.mode		dd ?

}