; KolibriOS bootloader
; this code has been written by diamond in 2005,2006,2007 specially for KolibriOS

; this code is loaded by our bootsector to 0000:8000
	format	binary
	use16

out_string = 0x7DA2
read_cluster = 0x7D0A
relative_read = 0x7D18
next_cluster = 0x7D5C

	org	0x8000
start:
; cs=ds=0, es undefined, ss=0, sp=7C00
	movzx	esp, sp
	push	1000h
	pop	es
; say hi to user
	mov	si, start_msg
	call	out_string
; parse image name
	mov	eax, [7C2Ch]	; root_cluster
	and	eax, 0xFFFFFFF
	mov	[cur_obj], root_string
.parsedir:
	push	ax
	mov	si, [imgnameofs]
	push	si
@@:
	lodsb
	cmp	al, 0
	jz	@f
	cmp	al, '\'
	jnz	@b
	dec     si
	mov     [missing_slash], si
	inc     si
@@:
	xchg	ax, [esp+2]
	mov	byte [si-1], 0
	mov	[imgnameofs], si
	call	fat32_parse_dir
	call    restore_slash
	pop	cx
	test	cl, cl
	jz	.end
	test	byte [es:di+0Bh], 10h
	mov	si, notdir_string
	jz	find_error_si
	jmp	.parsedir
.end:
	test	byte [es:di+0Bh], 10h
	mov	si, directory_string
	jnz	find_error_si
; parse FAT chunk
; runlist at 5000:0000
	mov	di, 4
	push	5000h
	pop	es
	mov	dword [es:di-4], 1
	stosd
.parsefat:
	call	next_cluster
	jnc	.done
	mov	ecx, [es:di-8]
	add	ecx, [es:di-4]
	cmp	eax, ecx
	jz	.contc
	mov	dword [es:di], 1
	scasd
	stosd
	jmp	.parsefat
.contc:
	inc	dword [es:di-8]
	jmp	.parsefat
.done:
	xor	eax, eax
	stosd
read_img_file:
	xor	si, si
	push	es
	pop	fs
; yes! Now read file to 0x100000
	xor	edi, edi
; read buffer to 1000:0000 and move it to extended memory
	push	1000h
	pop	es
	xor	bx, bx
.img_read_block:
	lods dword [fs:si]		; eax=length
	xchg	eax, ecx
	jecxz	.img_read_done
	lods dword [fs:si]		; eax=disk cluster
.img_read_cluster:
	pushad
; read part of file
	movzx	esi, byte [7C0Dh]
	mul	esi
	add	eax, [7A04h]
	push	ax
	mov	ax, 0x200
	div	si
	cmp	cx, ax
	jb	@f
	mov	cx, ax
@@:
	pop	ax
	add	[esp+1Ch], ecx
	sub	[esp+18h], cx
	imul	cx, si
	push	cx
	call	relative_read
	pop	cx
; move it to extended memory
	mov	byte [sou_addr+2], 1
.move_loop:
	push	cx
	cmp	cx, 80h
	jbe	@f
	mov	cx, 80h
@@:
	mov	ah, 87h
	xchg	cl, ch
	mov	si, movedesc
	push	cx es
	push	ds
	pop	es
	int	15h
	pop	es cx
	test    ah, ah
	mov	si, exmem_string
	jnz	find_error_si
	add	[dest_addr], ecx
	add	[dest_addr], ecx
	inc	byte [sou_addr+2]
	mov	al, ch
	mov	ah, cl
	pop	cx
	sub	cx, ax
	jnz	.move_loop
	popad
	test	cx, cx
	jnz	.img_read_cluster
	jmp	.img_read_block
.img_read_done:
; kolibri.img loaded; now load kernel.mnt
load_kernel:
	push	ds
	pop	es
	mov	[cur_obj], kernel_mnt_name
; read boot sector
	xor	eax, eax
	mov	bx, 500h
	mov	cx, 1
	call	read_img
; init vars
	mov	ax, [50Eh]	; reserved_sect
	add	ax, [51Ch]	; hidden
	mov	word [fat_start], ax
	xchg	ax, bx
	movzx	ax, byte [510h]		; num_fats
	mul	word [516h]		; fat_length
	add	ax, bx
; read root dir
	mov	bx, 700h
	mov	cx, [511h]	; dir_entries
	add	cx, 0Fh
	shr	cx, 4
	call	read_img
	add	ax, cx
	mov	[img_data_start], ax
	shl	cx, 9
	mov	di, bx
	add	bx, cx
	mov	byte [bx], 0
.scan_loop:
	cmp	byte [di], 0
	mov	si, notfound_string
	jz	find_error_si
	mov	si, kernel_mnt_name
	call	fat_compare_name
	jz	.found
	and	di, not 1Fh
	add	di, 20h
	jmp	.scan_loop
.found:
	and	di, not 1Fh
	mov	si, directory_string
	test	byte [di+0Bh], 10h
	jnz	find_error_si
; found, now load it to 1000h:0000h
	mov	ax, [di+1Ah]
; first cluster of kernel.mnt in ax
; translate it to sector on disk in kolibri.img
	push	ax
	dec	ax
	dec	ax
	movzx	cx, byte [50Dh]
	mul	cx
	add	ax, [img_data_start]
; now ax is sector in kolibri.img
	mov	[kernel_mnt_in_img], ax
	movzx	cx, byte [7C0Dh]
	div	cx
; now ax is cluster in kolibri.img and
; dx is offset from the beginning of cluster
	movzx	eax, ax
	push	5000h
	pop	ds
	xor	si, si
	mov	si, 1
.scani:
	sub	eax, [si]
	jb	.scanidone
; sanity check
	cmp	dword [si], 0
	push	data_error_msg
	jz	find_error_sp
	pop	cx
; next chunk
	add	si, 8
	jmp	.scani
.scanidone:
	add	eax, [si]	; undo last subtract
	add	eax, [si+4]	; get cluster
	push	0
	pop	ds
	movzx	ecx, byte [7C0Dh]
	push	dx
	mul	ecx		; get sector
	pop	dx
	movzx	edx, dx
	add	eax, edx
	add	eax, [7A04h]
	mov	[kernel_mnt_1st], eax
	pop	ax
	push	1000h
	pop	es
.read_loop:
	push	ax
	xor	bx, bx
	call	img_read_cluster
	shl	cx, 9-4
	mov	ax, es
	add	ax, cx
	mov	es, ax
	pop	ax
	call	img_next_cluster
	jc	.read_loop
	mov	ax, 'KL'
	mov	si, loader_block
	jmp	1000h:0000h

img_next_cluster:
	mov	bx, 700h
	push	ax
	shr	ax, 1
	add	ax, [esp]
	mov	dx, ax
	shr	ax, 9
	add	ax, word [fat_start]
	mov	cx, 2
	push	es
	push	ds
	pop	es
	call	read_img
	pop	es
	and	dx, 1FFh
	add	bx, dx
	mov	ax, [bx]
	pop	cx
	test	cx, 1
	jz	.1
	shr	ax, 4
.1:
	and	ax, 0FFFh
	mov	si, bad_cluster_string
	cmp	ax, 0FF7h
	jz	find_error_si
	ret
img_read_cluster:
	dec	ax
	dec	ax
	movzx	cx, byte [50Dh]	; sects_per_clust
	mul	cx
	add	ax, [img_data_start]
	movzx	eax, ax
;	call	read_img
;	ret
read_img:
; in: ax = sector, es:bx->buffer, cx=length in sectors
	pushad
	movzx	ebx, bx
	mov	si, movedesc
	shl	eax, 9
	add	eax, 93100000h
	mov	dword [si+sou_addr-movedesc], eax
	mov	eax, 9300000h
	mov	ax, es
	shl	eax, 4
	add	eax, ebx
	mov	[si+dest_addr-movedesc], eax
	mov	ah, 87h
	shl	cx, 8	; mul 200h/2
	push	es
	push	0
	pop	es
	int	15h
	pop	es
	cmp	ah, 0
	mov	si, exmem_string
	jnz	find_error_si
	popad
	ret

movedesc:
	times 16 db 0
; source
	dw	0xFFFF		; segment length
sou_addr dw	0000h		; linear address
	db	1		; linear address
	db	93h		; access rights
	dw	0
; destination
	dw	0xFFFF		; segment length
dest_addr dd	93100000h	; high byte contains access rights
				; three low bytes contains linear address (updated when reading)
	dw	0
	times 32 db 0

find_error_si:
	push	si
find_error_sp:
	mov	si, error_msg
	call	out_string
	mov	si, [cur_obj]
	call	out_string
	mov	si, colon
	call	out_string
	pop	si
	call	out_string
	mov     si, newline
	call    out_string
	jmp	$

file_not_found:
	mov	si, [esp+2]
	mov	[cur_obj], si
	push	notfound_string
	jmp	find_error_sp

restore_slash:
	mov     si, [missing_slash]
	test    si, si
	jz      @f
	and     [missing_slash], 0
	mov     byte [si], '\'
@@:     ret

	include 'fat32.inc'

if 0
write1st:
; callback from kernel.mnt
; write first sector of kernel.mnt from 1000:0000 back to disk
	push	cs
	pop	ds
	push	cs
	pop	es
; sanity check
	mov	bx, 500h
	mov	si, bx
	mov	cx, 1
	push	cx
	mov	eax, [kernel_mnt_1st]
	push	eax
	call	relative_read
	push	1000h
	pop	es
	xor	di, di
	mov	cx, 8
	repz	cmpsw
	mov	si, data_error_msg
	jnz	find_error_si
; ok, now write back to disk
	or	byte [read.patch1+2], 1
	or	byte [read.patch2+2], 1
	xor	bx, bx
	pop	eax
	pop	cx
	call	relative_read
	and	byte [read.patch1+1], not 1
	and	byte [read.patch2+2], not 2
; and to image in memory (probably this may be done by kernel.mnt itself?)
	mov	dword [sou_addr], 93010000h
	movzx	eax, [kernel_mnt_in_img]
	shl	eax, 9
	add	eax, 93100000h
	mov	dword [dest_addr], eax
	mov	si, movedesc
	push	ds
	pop	es
	mov	ah, 87h
	mov	cx, 100h
	int	15h
	cmp	ah, 0
	mov	si, exmem_string
	jnz	find_error_si
	retf
else
write1st = 0
end if

loader_block:
	db	1	; version
	dw	1	; flags - image is loaded
	dw	write1st	; offset
	dw	0		; segment

imgnameofs dw kolibri_img_name

; -----------------------------------------------
; ------------------ Settings -------------------
; -----------------------------------------------

; must be in lowercase, see ntfs_parse_dir.scan, fat32_parse_dir.scan
kernel_mnt_name 	db	'kernel.mnt',0
kolibri_img_name 	db	'kolibri.img',0

missing_slash   dw      0

start_msg	db	2,' KolibriOS bootloader, FAT32 flash version'
newline		db	13,10,0
error_msg	db	'Error'
colon		db	': ',0
root_string	db	'\',0
nodata_string	db	'$DATA '
notfound_string db	'not found',0
directory_string db	'is a directory',0
notdir_string	db	'not a directory',0
exmem_string	db	'extended memory error',0
bad_cluster_string db	'bad cluster',0
data_error_msg db	'data error',0

        align 2

; uninitialized data follows
cur_obj 		dw	?
img_data_start		dw	?
kernel_mnt_in_img	dw	?
fat_start		dw	?
kernel_mnt_1st		dd	?
