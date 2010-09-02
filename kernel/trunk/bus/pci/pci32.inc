;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;                                                              ;;
;;  PCI32.INC                                                   ;;
;;                                                              ;;
;;  32 bit PCI driver code                                      ;;
;;                                                              ;;
;;  Version 0.3  April 9, 2007                                  ;;
;;  Version 0.2  December 21st, 2002                            ;;
;;                                                              ;;
;;  Author: Victor Prodan, victorprodan@yahoo.com               ;;
;;          Mihailov Ilia, ghost.nsk@gmail.com                  ;;
;;    Credits:                                                  ;;
;;          Ralf Brown                                          ;;
;;          Mike Hibbett, mikeh@oceanfree.net                   ;;
;;                                                              ;;
;;  See file COPYING for details                                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

;***************************************************************************
;   Function
;      pci_api:
;
;   Description
;       entry point for system PCI calls
;***************************************************************************
;mmio_pci_addr	equ  0x400		 ; set actual PCI address here to activate user-MMIO

iglobal
align 4
f62call:
	dd	pci_api.0
	dd	pci_api.1
	dd	pci_api.2
	dd	pci_api.not_support	;3
	dd	pci_read_reg		;4 byte
	dd	pci_read_reg            ;5 word
	dd	pci_read_reg            ;6 dword
	dd	pci_api.not_support     ;7 
	dd	pci_write_reg           ;8 byte
	dd	pci_write_reg           ;9 word
	dd	pci_write_reg           ;10 dword
if defined mmio_pci_addr
	dd	pci_mmio_init		;11
	dd	pci_mmio_map		;12
	dd	pci_mmio_unmap		;13
end if
f62_rcall:
	dd	pci_read_reg.0		;4 byte
	dd	pci_read_reg.1		;5 word
	dd	pci_read_reg.2		;6 dword
f62_rcall2:
	dd	pci_read_reg_2.0	;4 byte
	dd	pci_read_reg_2.1	;5 word
	dd	pci_read_reg_2.2	;6 dword
f62_wcall:
	dd	pci_write_reg.0		;4 byte
	dd	pci_write_reg.1		;5 word
	dd	pci_write_reg.2		;6 dword
f62_wcall2:
	dd	pci_write_reg_2.0	;4 byte
	dd	pci_write_reg_2.1	;5 word
	dd	pci_write_reg_2.2	;6 dword
endg


align 4
pci_api:
        movzx	eax,bl
	cmp  [pci_access_enabled],1
	jne  .no_pci_access_for_applications

if defined mmio_pci_addr
        cmp eax, 13
        jb .not_support
else
        cmp eax, 10
        jb .not_support
end if	
        call dword [f62call+eax*4]
        mov	dword [esp+32],eax
        ret



;	or al,al
;	jnz pci_fn_1
	; PCI function 0: get pci version (AH.AL)
.0:
	movzx	eax, word [BOOT_VAR+0x9022]
	ret

;pci_fn_1:
;	cmp al,1
;	jnz pci_fn_2

	; PCI function 1: get last bus in AL
.1:
	movzx	eax, byte [BOOT_VAR+0x9021]
	ret

;pci_fn_2:
;	cmp al,2
;	jne pci_fn_3
	; PCI function 2: get pci access mechanism
.2:
	movzx	eax, byte [BOOT_VAR+0x9020]
	ret
;pci_fn_3:

;	cmp al,4
;	jz pci_read_reg   ;byte
;	cmp al,5
;	jz pci_read_reg   ;word
;	cmp al,6
;	jz pci_read_reg   ;dword

;	cmp al,8
;	jz pci_write_reg  ;byte
;	cmp al,9
;	jz pci_write_reg  ;word
;	cmp al,10
;	jz pci_write_reg  ;dword

;if defined mmio_pci_addr
;	cmp al,11	    ;  user-level MMIO functions 
;	jz pci_mmio_init
;	cmp al,12
;	jz pci_mmio_map
;	cmp al,13
;	jz pci_mmio_unmap
;end if

.not_support:
.no_pci_access_for_applications:
	or 	eax,-1
	ret

;***************************************************************************
;   Function
;      pci_make_config_cmd
;
;   Description
;       creates a command dword  for use with the PCI bus
;       bus # in bh;ah
;       device+func in ch;bh (dddddfff)
;       register in cl;bl
;
;      command dword returned in ebx;eax ( 10000000 bbbbbbbb dddddfff rrrrrr00 )
;***************************************************************************

align 4

pci_make_config_cmd:
    shl     ebx,8;eax,8	   ; move bus to bits 16-23
    mov     bx,cx;ax,bx	   ; combine all
    and     ebx,0xffffff;eax,0xffffff
    or	    ebx,0x80000000;eax,0x80000000
    ret

;***************************************************************************
;   Function
;      pci_read_reg:
;
;   Description
;       read a register from the PCI config space into EAX/AX/AL
;       IN: bh=bus,device+func=ch,register address=cl
;           number of bytes to read (1,2,4) coded into BL, bits 0-1
;           (0 - byte, 1 - word, 2 - dword)
;***************************************************************************

align 4

pci_read_reg:
	cmp	byte [BOOT_VAR+0x9020],2 ;what mechanism will we use?
	je	pci_read_reg_2

		; mechanism 1
;	push	esi   ; save register size into ESI
	mov	esi,ebx;eax
	and	esi,3

	call	pci_make_config_cmd
	mov	eax,ebx;ebx,eax
		; get current state
	mov	dx,0xcf8
	in	eax, dx
	push	eax
		; set up addressing to config data
	mov	eax,ebx
	and	al,0xfc ; make address dword-aligned
	out	dx,eax
		; get requested DWORD of config data
	mov	dl,0xfc
	and	bl,3
	or	dl,bl	 ; add to port address first 2 bits of register address

;	or	esi,esi
;	jz	pci_read_byte1
;	cmp	esi,1
;	jz	pci_read_word1
;	cmp	esi,2
;	jz	pci_read_dword1
;	jmp	pci_fin_read1
	jmp	dword [f62_rcall+esi*4]

.0:
	in	al,dx
	jmp .pci_fin_read1
.1:
	in	ax,dx
	jmp .pci_fin_read1
.2:
	in	eax,dx
;	jmp	pci_fin_read1
.pci_fin_read1:
		; restore configuration control
	xchg	eax,[esp]
	mov	dx,0xcf8
	out	dx,eax

	pop	eax
	;pop	esi
	ret
pci_read_reg_2:

	test	ch,128;bh,128	;mech#2 only supports 16 devices per bus
	jnz	pci_api.not_support

;	push esi   ; save register size into ESI
	mov esi,ebx;eax
	and esi,3

	push	ebx;eax
	mov	eax,ebx
		;store current state of config space
	mov	dx,0xcf8
	in	al,dx
	mov	ah,al
	mov	dl,0xfa
	in	al,dx

	xchg	eax,[esp]
		; out 0xcfa,bus
	mov	al,ah
	out	dx,al
		; out 0xcf8,0x80
	mov	dl,0xf8
	mov	al,0x80
	out	dx,al
		; compute addr
	shr	ch,3;bh,3 ; func is ignored in mechanism 2
	or	ch,0xc0;bh,0xc0
	mov	dx,cx;bx

;	or	esi,esi
;	jz	pci_read_byte2
;	cmp	esi,1
;	jz	pci_read_word2
;	cmp	esi,2
;	jz	pci_read_dword2
;	jmp	pci_fin_read2
	jmp	dword [f62_rcall2+esi*4]

.0:
	in	al,dx
	jmp .pci_fin_read2
.1:
	in	ax,dx
	jmp .pci_fin_read2
.2:
	in	eax,dx
;       jmp pci_fin_read2

.pci_fin_read2:

		; restore configuration space
	xchg	eax,[esp]
	mov	dx,0xcfa
	out	dx,al
	mov	dl,0xf8
	mov	al,ah
	out	dx,al

	pop	eax
;	pop	esi
	ret

;pci_read_reg_err:
;	or 	dword [esp+32],-1
;	ret


;***************************************************************************
;   Function
;      pci_write_reg:
;
;   Description
;       write a register from ECX/CX/CL into the PCI config space
;       IN: bh=bus,device+func=ch,register address (dword aligned)=cl,
;           value to write in edx
;           number of bytes to write (1,2,4) coded into BL, bits 0-1
;           (0 - byte, 1 - word, 2 - dword)
;***************************************************************************

align 4

pci_write_reg:
	cmp byte [BOOT_VAR+0x9020],2 ;what mechanism will we use?
	je pci_write_reg_2

		; mechanism 1
;	push	esi   ; save register size into ESI
	mov	esi,ebx;eax
	and	esi,3	;not need

	call	pci_make_config_cmd
	mov	eax,ebx;ebx,eax
	mov	ecx,edx		;cross registers
		; get current state into ecx
	mov	dx,0xcf8
	in	eax, dx
	push	eax
		; set up addressing to config data
	mov	eax,ebx
	and	al,0xfc ; make address dword-aligned
	out	dx,eax
		; write DWORD of config data
	mov	dl,0xfc
	and	bl,3
	or	dl,bl
	mov	eax,ecx

;	or	esi,esi
;	jz	pci_write_byte1
;	cmp	esi,1
;	jz	pci_write_word1
;	cmp	esi,2
;	jz	pci_write_dword1
;	jmp	pci_fin_write1
	jmp	dword [f62_wcall+esi*4]
.0:
	out	dx,al
	jmp .pci_fin_write1
.1:
	out	dx,ax
	jmp .pci_fin_write1
.2:
	out	dx,eax
.pci_fin_write1:

		; restore configuration control
	pop	eax
	mov	dl,0xf8
	out	dx,eax

	xor	eax,eax
	;pop	esi
	ret
pci_write_reg_2:

	test	ch,128;bh,128	;mech#2 only supports 16 devices per bus
	jnz	pci_api.not_support


;	push esi   ; save register size into ESI
	mov esi,eax
	and esi,3	;not need

	push	eax
	mov	ecx,edx		;cross registers
		;store current state of config space
	mov	dx,0xcf8
	in	al,dx
	mov	ah,al
	mov	dl,0xfa
	in	al,dx
	xchg	eax,[esp]
		; out 0xcfa,bus
	mov	al,ah
	out	dx,al
		; out 0xcf8,0x80
	mov	dl,0xf8
	mov	al,0x80
	out	dx,al
		; compute addr
	shr	bh,3 ; func is ignored in mechanism 2
	or	bh,0xc0
	mov	dx,bx
		; write register
	mov	eax,ecx

;	or	esi,esi
;	jz	pci_write_byte2
;	cmp	esi,1
;	jz	pci_write_word2
;	cmp	esi,2
;	jz	pci_write_dword2
;	jmp	pci_fin_write2
	jmp	dword [f62_wcall2+esi*4]
.0:
	out	dx,al
	jmp .pci_fin_write2
.1:
	out	dx,ax
	jmp .pci_fin_write2
.2:
	out	dx,eax
.pci_fin_write2:
		; restore configuration space
	pop	eax
	mov	dx,0xcfa
	out	dx,al
	mov	dl,0xf8
	mov	al,ah
	out	dx,al

	xor	eax,eax
	;pop	esi
	ret

;pci_write_reg_err:
;	xor	eax,eax
;	dec	eax
;	ret

if defined mmio_pci_addr	; must be set above
;***************************************************************************
;   Function
;      pci_mmio_init 
;
;   Description
;       IN:  cx = device's PCI bus address (bbbbbbbbdddddfff)
;   Returns  eax = user heap space available (bytes)
;   Error codes
;       eax = -1 : PCI user access blocked,
;       eax = -2 : device not registered for uMMIO service
;       eax = -3 : user heap initialization failure
;***************************************************************************
pci_mmio_init:
    cmp     cx, mmio_pci_addr	
    jz	    @f
    mov     eax,-2
    ret
@@:
    call    init_heap	   ; (if not initialized yet)
    or	    eax,eax
    jz	    @f
    ret
@@:
    mov     eax,-3
    ret


;***************************************************************************
;   Function
;      pci_mmio_map 
;
;   Description
;       maps a block of PCI memory to user-accessible linear address
;
;       WARNING! This VERY EXPERIMENTAL service is for one chosen PCI device only!
;       The target device address should be set in kernel var mmio_pci_addr
;
;       IN:  bh = BAR#;
;       IN: ecx = block size (bytes);
;       IN: edx = offset in MMIO block (in 4K-pages, to avoid misaligned pages);
;
;   Returns eax = MMIO block's linear address in the userspace (if no error)
;
;
;   Error codes
;       eax = -1 : user access to PCI blocked,
;       eax = -2 : an invalid BAR register referred
;       eax = -3 : no i/o space on that BAR
;       eax = -4 : a port i/o BAR register referred
;       eax = -5 : dynamic userspace allocation problem
;***************************************************************************

pci_mmio_map:
    and     edx,0x0ffff
    cmp     bh, 6
    jc     .bar_0_5
    jz     .bar_rom
    mov     eax,-2
    ret
.bar_rom:
    mov    ah, 8	; bar6 = Expansion ROM base address
.bar_0_5:
    push    edx
    add     ecx, 4095
    and     ecx, 0xFFFFF000	; 4k-alignment
    push    ecx
    mov     cl, bh	; cl = BAR# (0..5), however cl=8 for BAR6
    shl     cl, 1
    shl     cl, 1
    add     cl, 0x10	; now cl = BAR offset in PCI config. space
    mov     ax, mmio_pci_addr
    mov     ch, al	; ch = dddddfff
    mov     bl, 2		; bl : DW to read
    call    pci_read_reg	; new call
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
    pop     ecx 		; ecx = block size, bytes (expanded to whole page)
    mov     ebx, ecx	; user_alloc destroys eax, ecx, edx, but saves ebx
    and     al, 0xF0	; clear flags 
    push    eax 	      ; store MMIO physical address + keep 2DWords in the stack
    stdcall user_alloc, ecx
    or	eax, eax
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
    pop     eax 		; eax = MMIO start
    pop     edx 		; edx = MMIO shift (pages)
    shl     edx, 12	; edx = MMIO shift (bytes)
    add     eax, edx	; eax = uMMIO physical address
    or	eax, PG_SHARED+PG_UW+PG_NOCACHE
    mov     edi, ebx
    call    commit_pages
    mov     eax, edi
    ret

;***************************************************************************
;   Function
;      pci_mmio_unmap_page 
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

pci_mmio_unmap:
    stdcall user_free, ecx;ebx
    ret

end if

;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
uglobal
align 4
; VendID (2), DevID (2), Revision = 0 (1), Class Code (3), FNum (1), Bus (1)
pci_emu_dat:	times	30*10 db 0
endg
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
align 4
sys_pcibios:
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
	mov	edx, ecx
	cmp	ebp, 8			; READ_CONFIG_*
	jb	.not_READ_CONFIG
	cmp	ebp, 0x0A
	ja	.not_READ_CONFIG
	mov	eax, ebp	; -- ?? 
;	mov	ah, bh	; bus
	mov	ecx, edi
	mov	ch, bl	; dev+fn
;	mov	cl, dl	; reg#
	call	pci_read_reg	
	mov	edx, eax
	xor	ah, ah			; SUCCESSFUL
	jmp	.return_abc
.not_READ_CONFIG:
	cmp	ebp, 0x0B		; WRITE_CONFIG_*
	jb	.not_WRITE_CONFIG
	cmp	ebp, 0x0D
	ja	.not_WRITE_CONFIG
	lea	eax, [ebp+1]
;	mov	ah, bh	; bus
	mov	ecx, edi	
	mov	ch, bl
;	mov	cl, dl
	call	pci_write_reg
	xor	ah, ah			; SUCCESSFUL
	jmp	.return_abc
.not_WRITE_CONFIG:
.unsupported_func:
	mov	ah, 0x81		; FUNC_NOT_SUPPORTED
.return:mov	dword[esp + 4 ], edi
	mov	dword[esp + 8], esi
.return_abcd:
	mov	dword[esp + 24], edx
.return_abc:
	mov	dword[esp + 28], ecx
.return_ab:
	mov	dword[esp + 20], ebx
.return_a:
	mov	dword[esp + 32], eax
	ret
