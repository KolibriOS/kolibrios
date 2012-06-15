; KolibriOS bootloader
; bootsector for loading from FAT32 flash (or hard) drive
; intended for use with mtldr_f file in root folder
; this code has been written by diamond in 2005,2006,2007 specially for KolibriOS

; this code is loaded by BIOS to 0000:7C00
	org	0x7C00
	jmp	@f
	nop
	times 57h db 0
;	file 'bt2.dat':3,57h
@@:
	xor	eax, eax
	mov	ds, ax
	mov	ss, ax
	mov	sp, 7C00h
	mov	[boot_drive], dl
	cld
	sti
	push	800h
	pop	es
	movzx	ebx, word [7C0Eh]	; reserved_sect
	mov	[fat_start], ebx
	mov	al, byte [7C10h]	; num_fats
	mul	dword [7C24h]		; sect_fat
	add	eax, ebx
; cluster 2 begins from sector eax
	movzx	ebx, byte [7C0Dh]	; sects_per_clust
	add	bx, bx
	sub	eax, ebx
	mov	[data_start], eax
	mov	eax, [7C2Ch]		; root_cluster
	and	eax, 0FFFFFFFh
fat32_parse_dir:
	xor	bx, bx
	mov	di, bx
	push	eax
	call	read_cluster
	movzx	cx, byte [7C0Dh]	; sects_per_clust
	shl	cx, 4			; *0x200/0x20
scan_cluster:
	cmp	byte [es:di], 0
	jz	file_not_found
	push	cx di
	mov	cx, 11
	mov	si, mtldr_f
	repz	cmpsb
	pop	di cx
	jz	file_found
	add	di, 20h
	loop	scan_cluster
	pop	eax
	call	next_cluster
	jnc	file_not_found
	jc	fat32_parse_dir
file_found:
	pop	eax
	mov	ax, [es:di+14h]
	and	ax, 0FFFh
	shl	eax, 10h
	mov	ax, [es:di+1Ah]
; eax contains first cluster
@@:
	xor	bx, bx
	push	eax
	call	read_cluster
	mov	ax, es
	movzx	cx, byte [7C0Dh]
	shl	cx, 5
	add	ax, cx
	mov	es, ax
	pop	eax
	call	next_cluster
	jc	@b
	jmp	0:8000h

file_not_found:
	mov	si, file_not_found_msg
sayerr:
	call	out_string
	jmp	$

read_cluster:
; in: eax = cluster, bx->buffer
	movzx	ecx, byte [7C0Dh]
	mul	ecx
	add	eax, [data_start]

; read procedure
; in: eax = absolute sector
;     cx = number of sectors
;     es:bx -> buffer
read:
	add	eax, [7C1Ch]	; hidden sectors
	push	es
read_loop:
	pushad
; allocate disk address packet on the stack
; qword +8: absolute block number
	push	0
	push	0 		; dword +C is high dword
	push	eax		; dword +8 is low dword
; dword +4: buffer address
	push	es		; word +6 is segment
	push	bx		; word +4 is offset
; word +2: number of blocks, limited to 7Fh
	sub	cx, 7Fh
	sbb	ax, ax
	and	ax, cx
	add	ax, 7Fh
	push	ax
	shl	ax, 5
	mov	cx, es
	add	cx, ax
	mov	es, cx
; word +0: size of packet = 10h
	push	10h
; now pair ss:sp contain address of disk address packet
	mov	ax, 4200h
	mov	dl, [boot_drive]
	mov	si, sp
	int	13h
	mov	si, disk_read_err
	jc	sayerr
	popaw
	popad
	add	eax, 7Fh
	sub	cx, 7Fh
	ja	read_loop
	pop	es
	ret

next_cluster:
	push	es
	push	ds
	pop	es
	mov	bx, 7E00h
; sector is 200h bytes long, one entry in FAT occupies 4 bytes
; => 80h entries in sector
	push	eax
	shr	eax, 7		; div 80h
	cmp	eax, [fat_cur_sector]
	jz	@f
	mov	[fat_cur_sector], eax
	add	eax, [fat_start]
	mov	cx, 1
	call	read
@@:
	pop	eax
	and	eax, 7Fh
	mov	eax, [7E00h+eax*4]
	and	eax, 0FFFFFFFh
	cmp	eax, 0FFFFFF7h
	mov	si, bad_cluster
	jz	sayerr
	pop	es
	ret

out_string:
	lodsb
	test	al, al
	jz	.xxx
	mov	ah, 0Eh
	mov	bx, 7
	int	10h
	jmp	out_string
.xxx:	ret

file_not_found_msg	db	'Cannot find file '
mtldr_f			db	'MTLD_F32   '
			db	13,10,0
disk_read_err		db	'Disk read error',13,10,0
bad_cluster		db	'Bad cluster',13,10,0
fat_cur_sector	dd	-1

	times (7DFEh - $) db 0
	db	55h, 0AAh

virtual at 7A00h
fat_start	dd	?
data_start	dd	?
boot_drive	db	?
end virtual
