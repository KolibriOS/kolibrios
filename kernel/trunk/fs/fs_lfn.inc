; System function 70 - files with long names (LFN)
; diamond, 2006

iglobal
; in this table names must be in lowercase
rootdirs:
	db	2,'rd'
	dd	fs_OnRamdisk
	db	7,'ramdisk'
	dd	fs_OnRamdisk
	db	2,'fd'
	dd	fs_OnFloppy
	db	10,'floppydisk'
	dd	fs_OnFloppy
	db	3,'hd0'
	dd	fs_OnHd0
	db	3,'hd1'
	dd	fs_OnHd1
	db	3,'hd2'
	dd	fs_OnHd2
	db	3,'hd3'
	dd	fs_OnHd3
	db	0
endg

file_system_lfn:
; in: eax->fileinfo block
; operation codes:
; 0 : read file
; 1 : rewrite file - not implemented yet
; 2 : delete file - not implemented yet
; 3 : write/append to file - not implemented yet
; 4 : create directory - not implemented yet
; 5 : rename file/directory - not implemented yet
; 6 : get file attributes structure - not implemented yet
; 7 : start application - not implemented yet
; 8 : find file with mask - not implemented yet

; parse file name
	xchg	ebx, eax
	lea	esi, [ebx+20]
	lodsb
	cmp	al, '/'
	jz	@f
.notfound:
	mov	dword [esp+36], 5	; file not found
	ret
@@:
	cmp	byte [esi], 0
	jz	.rootdir
	mov	edi, rootdirs-4
	xor	ecx, ecx
	push	esi
.scan1:
	pop	esi
	add	edi, ecx
	scasd
	mov	cl, byte [edi]
	jecxz	.notfound
	inc	edi
	push	esi
@@:
	lodsb
	or	al, 20h
	scasb
	loopz	@b
	jnz	.scan1
	pop	eax
	lodsb
	cmp	al, '/'
	jz	.found1
	test	al, al
	jnz	.scan1
; directory /xxx
.maindir:
; directory /
.rootdir:
	mov	dword [esp+36], 10	; access denied
	ret

.found1:
	cmp	byte [esi], 0
	jz	.maindir
	mov	ebp, dword [edi]	; handler for this device
; read partition number
	xor	ecx, ecx
	xor	eax, eax
@@:
	lodsb
	cmp	al, '/'
	jz	.done1
	test	al, al
	jz	.done1
	sub	al, '0'
	cmp	al, 9
	ja	.notfound
	imul	ecx, 10
	add	ecx, eax
	jmp	@b
.done1:
	test	ecx, ecx
	jz	.notfound
	test	al, al
	jnz	@f
	dec	esi
@@:
; now ebp contains handler address, ecx - partition number, esi points to ASCIIZ string - rest of name
	jmp	ebp

; handlers for devices
; in: ecx = partition number
;     esi -> relative (for device) name
;     ebx -> fileinfo
; out: [esp+36]=image of eax, [esp+24]=image of ebx

fs_OnRamdisk:
	cmp	ecx, 1
	jnz	file_system_lfn.notfound
	movzx	eax, byte [ebx]
	test	eax, eax
	jnz	.not_impl
	mov	ecx, [ebx+12]
	mov	edx, [ebx+16]
	add	edx, std_application_base_address
	add	ebx, 4
	call	dword [fs_RamdiskServices + eax*4]
	mov	[esp+36], eax
	mov	[esp+24], ebx
	ret
.not_impl:
	mov	dword [esp+36], 2	; not implemented
	ret

fs_RamdiskServices:
	dd	fs_RamdiskRead

fs_OnFloppy:
	cmp	ecx, 2
	ja	file_system_lfn.notfound
	movzx	eax, byte [ebx]
	test	eax, eax
	jnz	fs_OnRamdisk.not_impl
	call	reserve_flp
	mov	[flp_number], cl
	mov	ecx, [ebx+12]
	mov	edx, [ebx+16]
	add	edx, std_application_base_address
	add	ebx, 4
	call	dword [fs_FloppyServices + eax*4]
	and	[flp_status], 0
	mov	[esp+36], eax
	mov	[esp+24], ebx
	ret

fs_FloppyServices:
	dd	fs_FloppyRead

fs_OnHd0:
	call	reserve_hd1
	mov	[hdbase], 0x1F0
	mov	[hdid], 0
	push	1
	jmp	fs_OnHd
fs_OnHd1:
	call	reserve_hd1
	mov	[hdbase], 0x1F0
	mov	[hdid], 0x10
	push	2
	jmp	fs_OnHd
fs_OnHd2:
	call	reserve_hd1
	mov	[hdbase], 0x170
	mov	[hdid], 0
	push	3
	jmp	fs_OnHd
fs_OnHd3:
	call	reserve_hd1
	mov	[hdbase], 0x170
	mov	[hdid], 0x10
	push	4
fs_OnHd:
	pop	eax
	mov	[hdpos], eax
	cmp	ecx, [0x40001+eax]
	jbe	@f
	and	[hd1_status], 0
	mov	dword [esp+36], 5	; not found
	ret
@@:
	mov	[fat32part], ecx
	push	ebx esi
	call	choice_necessity_partition_1
	pop	esi ebx
	mov	ecx, [ebx+12]
	mov	edx, [ebx+16]
	add	edx, std_application_base_address
	movzx	eax, byte [ebx]
	add	ebx, 4
	call	dword [fs_HdServices + eax*4]
	and	[hd1_status], 0
	mov	[esp+36], eax
	mov	[esp+24], ebx
	ret

fs_HdServices:
	dd	fs_HdRead
