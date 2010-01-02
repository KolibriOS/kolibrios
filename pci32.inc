;   Error codes
;       eax = -1 : user access to PCI blocked,
;       eax = -2 : an invalid BAR register referred
;       eax = -3 : no i/o space on that BAR
;       eax = -4 : a port i/o BAR register referred
;       eax = -5 : dynamic userspace allocation problem
;***************************************************************************

align 4
pci_mmio_map:
    and     edx,0x0ffff
    cmp     ah,6
    jc	    @f
    mov     eax,-2
    ret
@@:
    push    ecx
    add     ebx, 4095
    and     ebx,-4096
    push    ebx
    mov     bl, ah	; bl = BAR# (0..5)
    shl     bl, 1
    shl     bl, 1
    add     bl, 0x10	; bl = BARs offset in PCI config. space
    mov     ax,word [mmio_pci_addr]
    mov     bh, al	; bh = dddddfff
    mov     al, 2	; al : DW to read
    call    pci_read_reg
    or	    eax, eax
    jnz     @f
    mov     eax,-3	; empty I/O space
    jmp     mmio_ret_fail
@@:
    test    eax, 1
    jz	    @f
    mov     eax,-4	; damned ports (not MMIO space)
    jmp     mmio_ret_fail
@@:
    pop     ecx 	; ecx = block size, bytes (expanded to whole page)
    mov     ebx, ecx	; user_alloc destroys eax, ecx, edx, but saves ebx
    push    eax 	; store MMIO physical address + keep 2DWords in the stack
    stdcall user_alloc, ecx
    or	    eax, eax
    jnz     mmio_map_over
    mov     eax,-5	; problem with page allocation

mmio_ret_fail:
    pop     ecx
    pop     edx
    ret

mmio_map_over:
    mov     ecx, ebx	; ecx = size (bytes, expanded to whole page)
    shr     ecx, 12	; ecx = number of pages
    mov     ebx, eax	; ebx = linear address
    pop     eax 	; eax = MMIO start
    pop     edx 	; edx = MMIO shift (pages)
    shl     edx, 12	; edx = MMIO shift (bytes)
    add     eax, edx	; eax = uMMIO physical address
    or	    eax, PG_SHARED
    or	    eax, PG_UW
    or	    eax, PG_NOCACHE
    mov     edi, ebx
    call    commit_pages
    mov     eax, edi
    ret

;***************************************************************************
;   Function
;      pci_mmio_unmap_page ; NEW!
;
;   Description
;       unmaps the linear space previously tied to a PCI memory block
;
;       IN: ebx = linear address of space previously allocated by pci_mmio_map
;       returns eax = 1 if successfully unmapped
;
;   Error codes
;       eax = -1 if no user PCI access allowed,
;       eax =  0 if unmapping failed
;***************************************************************************

align 4
pci_mmio_unmap:
    stdcall user_free, ebx
    ret


;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

; VendID (2), DevID (2), Revision = 0 (1), Class Code (3), FNum (1), Bus (1)
pci_emu_dat:	times	30*10 db 0

;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
align 4
sys_pcibios:
	xchg	ebx, eax
	xchg	ecx, eax
	xchg	edx, eax
	xchg	esi, eax
	xchg	edi, eax
	cmp	[pci_access_enabled], 1
	jne	.unsupported_func
	cmp	[pci_bios_entry], 0
	jz	.emulate_bios

	push	ds
	mov	ax, pci_data_sel
	mov	ds, ax
	mov	eax, ebp
	mov	ah, 0B1h
	call	pword [cs:pci_bios_entry]
	pop	ds

	jmp	.return
	;-=-=-=-=-=-=-=-=
.emulate_bios:
	cmp	ebp, 1			; PCI_FUNCTION_ID
	jnz	.not_PCI_BIOS_PRESENT
	mov	edx, 'PCI '
	mov	al, [OS_BASE+0x2F0000 + 0x9020]
	mov	bx, [OS_BASE+0x2F0000 + 0x9022]
	mov	cl, [OS_BASE+0x2F0000 + 0x9021]
	xor	ah, ah
	jmp	.return_abcd

.not_PCI_BIOS_PRESENT:
	cmp	ebp, 2			; FIND_PCI_DEVICE
	jne	.not_FIND_PCI_DEVICE
	mov	ebx, pci_emu_dat
..nxt:	cmp	[ebx], dx
	jne	..no
	cmp	[ebx + 2], cx
	jne	..no
	dec	si
	jns	..no
	mov	bx, [ebx + 4]
	xor	ah, ah
	jmp	.return_ab
..no:	cmp	word[ebx], 0
	je	..dev_not_found
	add	ebx, 10
	jmp	..nxt
..dev_not_found:
	mov	ah, 0x86		; DEVICE_NOT_FOUND
	jmp	.return_a

.not_FIND_PCI_DEVICE:
	cmp	ebp, 3			; FIND_PCI_CLASS_CODE
	jne	.not_FIND_PCI_CLASS_CODE
	mov	esi, pci_emu_dat
	shl	ecx, 8
..nxt2: cmp	[esi], ecx
	jne	..no2
	mov	bx, [esi]
	xor	ah, ah
	jmp	.return_ab
..no2:	cmp	dword[esi], 0
	je	..dev_not_found
	add	esi, 10
	jmp	..nxt2

.not_FIND_PCI_CLASS_CODE:
	cmp	ebp, 8			; READ_CONFIG_*
	jb	.not_READ_CONFIG
	cmp	ebp, 0x0A
	ja	.not_READ_CONFIG
	mov	eax, ebp
	mov	ah, bh
	mov	edx, edi
	mov	bh, bl
	mov	bl, dl
	call	pci_read_reg
	mov	ecx, eax
	xor	ah, ah			; SUCCESSFUL
	jmp	.return_abc
.not_READ_CONFIG:
	cmp	ebp, 0x0B		; WRITE_CONFIG_*
	jb	.not_WRITE_CONFIG
	cmp	ebp, 0x0D
	ja	.not_WRITE_CONFIG
	lea	eax, [ebp+1]
	mov	ah, bh
	mov	edx, edi
	mov	bh, bl
	mov	bl, dl
	call	pci_write_reg
	xor	ah, ah			; SUCCESSFUL
	jmp	.return_abc
.not_WRITE_CONFIG:
.unsupported_func:
	mov	ah, 0x81		; FUNC_NOT_SUPPORTED
.return:mov	dword[esp + 8 ], edi
	mov	dword[esp + 12], esi
.return_abcd:
	mov	dword[esp + 28], edx
.return_abc:
	mov	dword[esp + 32], ecx
.return_ab:
	mov	dword[esp + 24], ebx
.return_a:
	mov	dword[esp + 36], eax
	ret
