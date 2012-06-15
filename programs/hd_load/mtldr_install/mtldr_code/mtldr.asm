; KolibriOS bootloader
; this code has been written by diamond in 2005,2006 specially for KolibriOS

; this code is loaded by ntldr to 0D00:0000
; and by io.sys from config.sys to xxxx:0100
; and by bootmgr in vista to 0000:7C00
	format	binary
	use16

	org	0xD000

; entry point for 9x booting
	call	@f
;	db	'd' xor 'i' xor 'a' xor 'm' xor 'o' xor 'n' xor 'd'
	db	'NTFS'

; file offset +7
; may be changed by installator
boot_drive	db	80h
partition_start dd	-1
imgnameofs	dw	menuet_img_name

@@:
	pop	si
	sub	si, 3
	cmp	si, 7C00h
	jz	boot_vista
	mov	si, load_question + 100h - 0D000h
	call	out_string
	mov	si, answer + 100h - 0D000h
xxy:	mov	ah, 0
	int	16h
	or	al, 20h
	mov	[si], al
	cmp	al, 'y'
	jz	xxz
	cmp	al, 'n'
	jnz	xxy
; continue load Windows
;       call    out_string
;       ret
out_string:
	lodsb
	test	al, al
	jz	.xxx
	mov	ah, 0Eh
	mov	bx, 7
	int	10h
	jmp	out_string
.xxx:	ret
xxz:
; boot KolibriOS
	call	out_string
	push	0
	pop	ds
	mov	word [4], new01handler + 100h - 0D000h
	mov	[6], cs
	pushf
	pop	ax
	or	ah, 1
	push	ax
	popf
;	int	19h
;	pushf		; there will be no iret
	call	far [19h*4]
xxt:
; TF has been cleared when entered new01handler
;	pushf
;	pop	ax
;	and	ah, not 1
;	push	ax
;	popf
	push	0
	pop	ds
	cmp	word [8*4+2], 0F000h
	jz	@f
	les	bx, [8*4]
	mov	eax, [es:bx+1]
	mov	[8*4], eax
@@:
	mov	si, 100h
boot_vista:
	xor	di, di
	push	cs
	pop	ds
	push	0D00h
	pop	es
	mov	cx, 2000h/2
	rep	movsw
	jmp	0D00h:0256h

new01handler:
; [sp]=ip, [sp+2]=cs, [sp+4]=flags
	push	bp
	mov	bp, sp
	push	bx
	push	ds
	lds	bx, [bp+2]
	cmp	word [bx], 19cdh
	jz	xxt
	pop	ds
	pop	bx
	pop	bp
	iret

relative_read:
	add	eax, [partition_start]

; read from hard disk
; drive_size must be already initialized
; in: eax = absolute sector
;     cx = number of sectors
;     es:bx -> buffer
read:
	pushad
	cmp	eax, [drive_size]
	jb	.old_style
	xor	dx, dx
; new style - LBA, function 42
	cmp	[has_lba], dl
	jz	disk_error
; allocate disk address packet on the stack
; qword +8: absolute block number
	push	dx
	push	dx
;	push	dword 0 	; dword +C is high dword
	push	eax		; dword +8 is low dword
; dword +4: buffer address
	push	es		; word +6 is segment
	push	bx		; word +4 is offset
; word +2: number of blocks = 1
	push	1
; word +0: size of packet = 10h
	push	10h
; now pair ss:sp contain address of disk address packet
.patch1:
	mov	ax, 4200h
	mov	dl, [boot_drive]
	mov	si, sp
	push	ds
	push	ss
	pop	ds
	int	13h
	pop	ds
	lea	sp, [si+10h]
.end:
	popad
	jc	disk_error
	add	bx, 200h
	inc	eax
	dec	cx
	jnz	read
	ret
.old_style:
; old style - CHS, function 2
; convert absolute sector in eax to cylinder-head-sector coordinates
; calculate sector
	xor	edx, edx
	movzx	ecx, [sectors]
	div	ecx
; sectors are counted from 1
	inc	dl
	mov	cl, dl		; low 6 bits of cl = sector number
; calculate head number
;	shld	edx, eax, 10h	; convert eax to dx:ax
	push	eax
	pop	ax
	pop	dx
	div	[heads]
	mov	dh, dl		; dh = head
	mov	ch, al		; ch = low 8 bits of cylinder
	shl	ah, 6
	or	cl, ah		; high 2 bits of cl = high 2 bits of cylinder
.patch2:
	mov	ax, 201h	; function 2, al=1 - number of sectors
	mov	dl, [boot_drive]
	int	13h
	jmp	.end

disk_error:
	mov	si, disk_error_msg
	call	out_string
	jmp	$

answer	db	?
	db	13,10
has_lba	db	0

disk_error_msg	db	'Disk read error!',0
start_msg	db	2,' KolibriOS bootloader, running on ',0
errfs_msg	db	'unknown filesystem, cannot continue',0
fat16_msg	db	'FAT12/FAT16 - unsupported',13,10,0
fat32_msg	db	'FAT32',13,10,0
ntfs_msg	db	'NTFS',13,10,0
error_msg	db	'Error'
colon		db	': ',0
mft_string	db	'MFT',0
root_string	db	'\',0
noindex_string	db	'$INDEX_ROOT not found',0
invalid_read_request_string db 'cannot read attribute',0
nodata_string	db	'$DATA '
notfound_string db	'not found',0
directory_string db	'is a directory',0
notdir_string	db	'not a directory',0
fragmented_string db	'too fragmented file',0
bad_cluster_string db	'bad cluster',0
exmem_string	db	'extended memory error',0

load_question	db	'Load KolibriOS? [y/n]: ',0

	repeat	0D256h - $
		db	1
	end	repeat

start:
	xor	ax, ax
	mov	ds, ax
	mov	es, ax
; our stack is 4Kb-2b!!! (0xFFE)
	mov	ss, ax
	mov	esp, 0FFFEh
	cld
	sti
; calculate drive size
	mov	dl, [boot_drive]
	mov	ah, 8	; 8 = get drive parameters
	int	13h
; now: CF is set on error;
; ch = low 8 bits of maximum cylinder number
; cl : low 6 bits makes maximum sector number, high 2 bits are high 2 bits of maximum cylinder number
; dh = maximum head number
	jnc	@f
	mov	cx, -1
	mov	dh, cl
@@:
	movzx	ax, dh
	inc	ax
; ax = number of heads
	mov	[heads], ax
	mov	dl, cl
	and	dx, 3Fh
; dx = number of sectors
; (note that sectors are counted from 1, and maximum sector number = number of sectors)
	mov	[sectors], dx
	mul	dx
	xchg	cl, ch
	shr	ch, 6
	inc	cx
; cx = number of cylinders
	mov	[cyls], cx
	mul	cx
	mov	word [drive_size], ax
	mov	word [drive_size+2], dx
; this drive supports LBA?
	mov	dl, [boot_drive]
	mov	ah, 41h
	mov	bx, 55AAh
	int	13h
	jc	.no_lba
	cmp	bx, 0AA55h
	jnz	.no_lba
	test	cl, 1
	jz	.no_lba
	inc	[has_lba]
.no_lba:
; say hi to user
	mov	si, start_msg
	call	out_string
	mov	eax, [partition_start]
	cmp	eax, -1
	jnz	@f
; now read first sector to determine file system type
; first sector of disk is MBR sector
	xor	eax, eax
	mov	cx, 1
	mov	bx, 500h
	call	read
	mov	eax, [6C6h]	; first disk
	mov	[partition_start], eax
@@:
	mov	cx, 1
	mov	bx, 500h
	call	read
	movzx	ax, byte [50Dh]
	mov	[sect_per_clust], ax
; determine file system
	cmp	dword [536h], 'FAT1'
	jz	fat1x
	cmp	dword [552h], 'FAT3'
	jz	fat32
	cmp	dword [503h], 'NTFS'
	jz	ntfs
;       mov     si, errfs_msg           ; already is
	call	out_string
	jmp	$
fat1x:
	mov	si, fat16_msg
	call	out_string
	jmp	$
fat32:
	mov	si, fat32_msg
	call	out_string
	movzx	eax, word [50Bh]	; bytes_per_sect
	movzx	ebx, byte [50Dh]	; sects_per_clust
	mul	ebx
	mov	[cluster_size], eax
	movzx	ebx, word [50Eh]	; reserved_sect
	mov	[fat_start], ebx
	movzx	eax, byte [510h]	; num_fats
	mul	dword [524h]		; sect_fat
	add	eax, ebx
; cluster 2 begins from sector eax
	movzx	ebx, byte [50Dh]	; sects_per_clust
	sub	eax, ebx
	sub	eax, ebx
	mov	[data_start], eax
; parse image name
	mov	eax, [52Ch]	; root_cluster
	mov	[cur_obj], root_string
.parsedir:
	push	ax
	mov	si, [imgnameofs]
	push	si
@@:
	lodsb
	cmp	al, '\'
	jz	@f
	cmp	al, 0
	jnz	@b
@@:
	xchg	ax, [esp+2]
	mov	byte [si-1], 0
	mov	[imgnameofs], si
	call	fat32_parse_dir
	pop	cx
	test	cl, cl
	jz	.end
	test	byte [di+0Bh], 10h
	mov	si, notdir_string
	jz	find_error_si
	jmp	.parsedir
.end:
	test	byte [di+0Bh], 10h
	mov	si, directory_string
	jnz	find_error_si
; parse FAT chunk
; runlist at 2000:0000
	mov	di, 5
	push	2000h
	pop	es
	mov	byte [es:di-5], 1	; of course, non-resident
	mov	dword [es:di-4], 1
	stosd
.parsefat:
	push	es
	push	ds
	pop	es
	call	next_cluster
	pop	es
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
	jmp	read_img_file

ntfs:
	mov	si, ntfs_msg
	call	out_string
	movzx	eax, word [50Bh]	; bpb_bytes_per_sect
	push	eax
	movzx	ebx, byte [50Dh]	; bpb_sects_per_clust
	mul	ebx
	mov	[cluster_size], eax
	mov	[data_start], 0
	mov	ecx, [540h]		; frs_size
	cmp	cl, 0
	jg	.1
	neg	cl
	xor	eax, eax
	inc	eax
	shl	eax, cl
	jmp	.2
.1:
	mul	ecx
.2:
	mov	[frs_size], eax
	pop	ebx
	xor	edx, edx
	div	ebx
	mov	[frs_sectors], ax
; read first MFT record - description of MFT itself
	mov	[cur_obj], mft_string
	movzx	eax, byte [50Dh]	; bpb_sects_per_clust
	mul	dword [530h]		; mft_cluster
	mov	cx, [frs_sectors]
	mov	bx, 4000h
	mov	di, bx
	push	bx
	call	relative_read
	call	restore_usa
; scan for unnamed $DATA attribute
	pop	di
	mov	ax, 80h		; $DATA
	mov	bx, 700h
	call	load_attr
	mov	si, nodata_string
	jc	find_error_si
	mov	[free], bx
; load menuet.img
; parse image name
	mov	eax, 5		; root cluster
	mov	[cur_obj], root_string
.parsedir:
	push	ax
	mov	si, [imgnameofs]
	push	si
@@:
	lodsb
	cmp	al, '\'
	jz	@f
	cmp	al, 0
	jnz	@b
@@:
	xchg	ax, [esp+2]
	mov	byte [si-1], 0
	mov	[imgnameofs], si
	call	ntfs_parse_dir
	pop	cx
	test	cl, cl
	jnz	.parsedir
read_img_file:
	xor	si, si
	push	es
	pop	fs
; yes! Now read file to 0x100000
	lods byte [fs:si]
	cmp	al, 0	; assume nonresident attr
	mov	si, invalid_read_request_string
	jz	find_error_si
	mov	si, 1
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
	movzx	ecx, byte [50Dh]
	mul	ecx
	add	eax, [data_start]
	call	relative_read
; move it to extended memory
	mov	ah, 87h
	mov	ecx, [cluster_size]
	push	ecx
	shr	cx, 1
	mov	si, movedesc
	push	es
	push	ds
	pop	es
	int	15h
	pop	es
	test	ah, ah
	mov	si, exmem_string
	jnz	find_error_si
	pop	ecx
	add	[dest_addr], ecx
	popad
	inc	eax
	loop	.img_read_cluster
	jmp	.img_read_block
.img_read_done:
; menuet.img loaded; now load kernel.mnt
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
; translate it to sector on disk in menuet.img
	push	ax
	dec	ax
	dec	ax
	movzx	cx, byte [50Dh]
	mul	cx
	add	ax, [img_data_start]
; now ax is sector in menuet.img
	mov	[kernel_mnt_in_img], ax
	div	[sect_per_clust]
; now ax is cluster in menuet.img and
; dx is offset from the beginning of cluster
	movzx	eax, ax
	push	2000h
	pop	ds
	mov	si, 1
.scani:
	sub	eax, [si]
	jb	.scanidone
; sanity check
	cmp	dword [si], 0
	push	invalid_read_request_string
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
	movzx	ecx, [sect_per_clust]
	push	dx
	mul	ecx		; get sector
	pop	dx
	movzx	edx, dx
	add	eax, edx
	add	eax, [data_start]
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
	jmp	$

file_not_found:
	mov	si, [esp+2]
	mov	[cur_obj], si
	push	notfound_string
	jmp	find_error_sp

	include 'fat32.inc'
	include	'ntfs.inc'

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

loader_block:
	db	1	; version
	dw	1	; flags - image is loaded
	dw	write1st	; offset
	dw	0		; segment

fat_cur_sector dd -1

data_error_msg db	'data error',0

; -----------------------------------------------
; ------------------ Settings -------------------
; -----------------------------------------------

; must be in lowercase, see ntfs_parse_dir.scan, fat32_parse_dir.scan
kernel_mnt_name 	db	'kernel.mnt',0

; will be initialized by installer
menuet_img_name		rb	300

; uninitialized data follows
drive_size		dd	?	; in sectors
heads			dw	?
sectors 		dw	?
cyls			dw	?
free			dw	?
cur_obj 		dw	?
data_start		dd	?
img_data_start		dw	?
sect_per_clust		dw	?
kernel_mnt_in_img	dw	?
kernel_mnt_1st		dd	?
; NTFS data
cluster_size		dd	?	; in bytes
frs_size		dd	?	; in bytes
frs_sectors		dw	?	; in sectors
mft_data_attr		dw	?
index_root		dw	?
index_alloc		dw	?
ofs			dw	?
dir			dw	?
; FAT32 data
fat_start		dd	?
cur_cluster		dd	?
