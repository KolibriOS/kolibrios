; Copyright (c) 2008-2009, diamond
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;       * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;       * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;       * Neither the name of the <organization> nor the
;       names of its contributors may be used to endorse or promote products
;       derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Alexey Teplov aka <Lrz> ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************

; KordOS bootloader, based on mtldr, KolibriOS bootloader, by diamond
; It is used when main bootloader is Windows loader.

; this code is loaded:
; NT/2k/XP: by ntldr to 0D00:0000
; 9x: by io.sys from config.sys to xxxx:0100
; Vista: by bootmgr to 0000:7C00
	format binary
	use16

; in any case, we relocate this code to 0000:0600
	org 0x600
; entry point for 9x and Vista booting
	call	@f
	db	'NTFS'
@@:
	pop	si
	sub	si, 3
	cmp	si, 100h
	jnz	boot_vista
	mov	si, load_question + 100h - 600h
	call	out_string
;	mov	si, answer + 100h - 0600h		; already is
xxy:	mov	ah, 0
	int	16h
	or	al, 20h
	mov	[si], al
	cmp	al, 'y'
	jz	xxz
	cmp	al, 'n'
	jnz	xxy
; continue load Windows
;	call	out_string
;	ret
out_string:
	push	bx
@@:
	lodsb
	test	al, al
	jz	@f
	mov	ah, 0Eh
	mov	bx, 7
	int	10h
	jmp	@b
@@:
	pop	bx
	ret
xxz:
; boot KordOS
	call	out_string
; 9x bootloader has already hooked some interrupts; to correctly remove all DOS handlers,
; issue int 19h (reboot interrupt) and trace its DOS handler until original BIOS handler is reached
	xor	di, di
	mov	ds, di
	mov	word [di+4], new01handler + 100h - 600h
	mov	[di+6], cs
	pushf
	pop	ax
	or	ah, 1
	push	ax
	popf
; we cannot issue INT 19h directly, because INT command clears TF
;	int	19h	; don't issue it directly, because INT command clears TF
; so instead we use direct call
;	pushf		; there will be no IRET
	call	far [di + 19h*4]
xxt:
	xor	di, di
	mov	ds, di
	cmp	word [di + 8*4+2], 0F000h
	jz	@f
	les	bx, [di + 8*4]
	mov	eax, [es:bx+1]
	mov	[di + 8*4], eax
@@:
	mov	si, 100h
boot_vista:
; relocate cs:si -> 0000:0600
	push	cs
	pop	ds
	xor	ax, ax
	mov	es, ax
	mov	di, 0x600
	mov	cx, 2000h/2
	rep	movsw
	jmp	0:real_entry

load_question	db	'Load KordOS? [y/n]: ',0
answer	db	?
	db	13,10,0

new01handler:
; [sp]=ip, [sp+2]=cs, [sp+4]=flags
	push	bp
	mov	bp, sp
	push	ds
	lds	bp, [bp+2]
	cmp	word [ds:bp], 19cdh
	jz	xxt
	pop	ds
	pop	bp
	iret

; read from hard disk
; in: eax = absolute sector
;     cx = number of sectors
;     es:bx -> buffer
; out: CF=1 if error
read:
	pushad
	add	eax, [bp + partition_start - dat]
	cmp	[bp + use_lba - dat], 0
	jz	.chs
; LBA read
	push	ds
.lbado:
	push	ax
	push	cx
	cmp	cx, 0x7F
	jbe	@f
	mov	cx, 0x7F
@@:
; create disk address packet on the stack
; dq starting LBA
	push	0
	push	0
	push	eax
; dd buffer
	push	es
	push	bx
; dw number of blocks to transfer (no more than 0x7F)
	push	cx
; dw packet size in bytes
	push	10h
; issue BIOS call
	push	ss
	pop	ds
	mov	si, sp
	mov	dl, [bp + boot_drive - dat]
	mov	ah, 42h
	int	13h
	jc	.disk_error_lba
	add	sp, 10h		; restore stack
; increase current sector & buffer; decrease number of sectors
	movzx	esi, cx
	mov	ax, es
	shl	cx, 5
	add	ax, cx
	mov	es, ax
	pop	cx
	pop	ax
	add	eax, esi
	sub	cx, si
	jnz	.lbado
	pop	ds
	popad
	ret
.disk_error_lba:
	add	sp, 14h
	pop	ds
	popad
	stc
	ret

.chs:
	pusha
	pop	edi		; loword(edi) = di, hiword(edi) = si
	push	bx

; eax / (SectorsPerTrack) -> eax, remainder bx
	movzx	esi, [bp + sectors - dat]
	xor	edx, edx
	div	esi
	mov	bx, dx		; bx = sector-1

; eax -> dx:ax
	push	eax
	pop	ax
	pop	dx
; (dword in dx:ax) / (NumHeads) -> (word in ax), remainder dx
	div	[bp + heads - dat]

; number of sectors: read no more than to end of track
	sub	si, bx
	cmp	cx, si
	jbe	@f
	mov	cx, si
@@:

	inc	bx
; now ax=track, dl=head, dh=0, cl=number of sectors, ch=0, bl=sector
; convert to int13 format
	movzx	edi, cx
	mov	dh, dl
	mov	dl, [bp + boot_drive - dat]
	shl	ah, 6
	mov	ch, al
	mov	al, cl
	mov	cl, bl
	or	cl, ah
	pop	bx
	mov	si, 3
	mov	ah, 2
@@:
	push	ax
	int	13h
	jnc	@f
	xor	ax, ax
	int	13h	; reset drive
	pop	ax
	dec	si
	jnz	@b
	add	sp, 12
	popad
	stc
	ret
@@:
	pop	ax
	mov	ax, es
	mov	cx, di
	shl	cx, 5
	add	ax, cx
	mov	es, ax
	push	edi
	popa
	add	eax, edi
	sub	cx, di
	jnz	.chs
	popad
	ret

disk_error2	db	'Fatal: cannot read partitions info: '
disk_error_msg	db	'disk read error',0
disk_params_msg	db	'Fatal: cannot get drive parameters',0
start_msg	db	2,' KordOS bootloader',13,10,0
part_msg	db	'looking at partition '
part_char	db	'0'	; will be incremented before writing message
		db	' ... ',0
errfs_msg	db	'unknown filesystem',13,10,0
fatxx_msg	db	'FATxx'
newline		db	13,10,0
ntfs_msg	db	'NTFS',13,10,0
error_msg	db	'Error'
colon		db	': ',0
root_string	db	'\',0
nomem_msg	db	'No memory',0
filesys_string	db	'(filesystem)',0
directory_string db	'is a directory',0
notdir_string	db	'not a directory',0

; entry point for NT/2k/XP booting
; ntldr loads our code to 0D00:0000 and jumps to 0D00:0256
	repeat	600h + 256h - $
		db	1	; any data can be here; 1 in ASCII is a nice face :)
	end repeat
; cs=es=0D00, ds=07C0, ss=0
; esi=edi=ebp=0, esp=7C00
	xor	si, si
	jmp	boot_vista

real_entry:
; ax = 0
	mov	ds, ax
	mov	es, ax
; our stack is 4 Kb: memory range 2000-3000
	mov	ss, ax
	mov	sp, 3000h
	mov	bp, dat
	sti	; just for case
; say hi to user
	mov	si, start_msg
	call	out_string
; we are booting from hard disk identified by [boot_drive]
	mov	dl, [bp + boot_drive - dat]
; is LBA supported?
	mov	[bp + use_lba - dat], 0
	mov	ah, 41h
	mov	bx, 55AAh
	int	13h
	jc	.no_lba
	cmp	bx, 0AA55h
	jnz	.no_lba
	test	cl, 1
	jz	.no_lba
	inc	[bp + use_lba - dat]
	jmp	disk_params_ok
.no_lba:
; get drive geometry
	mov	ah, 8
	mov	dl, [bp + boot_drive - dat]
	int	13h
	jnc	@f
	mov	si, disk_params_msg
	call	out_string
	jmp	$
@@:
	movzx	ax, dh
	inc	ax
	mov	[bp + heads - dat], ax
	and	cx, 3Fh
	mov	[bp + sectors - dat], cx
disk_params_ok:
; determine size of cache for folders
	int	12h	; ax = size of available base memory in Kb
	sub	ax, 94000h / 1024
	jc	nomem
	shr	ax, 3
	mov	[bp + cachelimit - dat], ax	; size of cache - 1
; scan all partitions
new_partition_ex:
	xor	eax, eax	; read first sector of current disk area
	mov	[bp + extended_part_cur - dat], eax	; no extended partition yet
	mov	[bp + cur_partition_ofs - dat], 31BEh	; start from first partition
	push	es
	mov	cx, 1
	mov	bx, 3000h
	call	read
	pop	es
	jnc	new_partition
	mov	si, disk_error2
	call	out_string
	jmp	$
new_partition:
	mov	bx, [bp + cur_partition_ofs - dat]
	mov	al, [bx+4]	; partition type
	test	al, al
	jz	next_partition
	cmp	al, 5
	jz	@f
	cmp	al, 0xF
	jnz	not_extended
@@:
; extended partition
	mov	eax, [bx+8]	; partition start
	add	eax, [bp + extended_part_start - dat]
	mov	[bp + extended_part_cur - dat], eax
next_partition:
	add	[bp + cur_partition_ofs - dat], 10h
	cmp	[bp + cur_partition_ofs - dat], 31FEh
	jb	new_partition
	mov	eax, [bp + extended_part_cur - dat]
	test	eax, eax
	jz	partitions_done
	cmp	[bp + extended_part_start - dat], 0
	jnz	@f
	mov	[bp + extended_part_start - dat], eax
@@:
	mov	[bp + extended_parent - dat], eax
	mov	[bp + partition_start - dat], eax
	jmp	new_partition_ex
partitions_done:
	mov	si, total_kaput
	call	out_string
	jmp	$
not_extended:
	mov	eax, [bx+8]
	add	eax, [bp + extended_parent - dat]
	mov	[bp + partition_start - dat], eax
; try to load from current partition
; inform user
	mov	si, part_msg
	inc	[si + part_char - part_msg]
	call	out_string
; read bootsector
	xor	eax, eax
	mov	[bp + cur_obj - dat], filesys_string
	push	es
	mov	cx, 1
	mov	bx, 3200h
	call	read
	pop	es
	mov	si, disk_error_msg
	jc	find_error_si
	movzx	si, byte [bx+13]
	mov	word [bp + sect_per_clust - dat], si
	test	si, si
	jz	unknown_fs
	lea	ax, [si-1]
	test	si, ax
	jnz	unknown_fs
; determine file system
; Number of bytes per sector == 0x200 (this loader assumes that physical sector size is 200h)
	cmp	word [bx+11], 0x200
	jnz	unknown_fs
; is it NTFS?
	cmp	dword [bx+3], 'NTFS'
	jnz	not_ntfs
	cmp	byte [bx+16], bl
	jz	ntfs
not_ntfs:
; is it FAT? FAT12/FAT16/FAT32?
; get count of sectors to dword in cx:si
	mov	si, [bx+19]
	xor	cx, cx
	test	si, si
	jnz	@f
	mov	si, [bx+32]
	mov	cx, [bx+34]
@@:
	xor	eax, eax
; subtract size of system area
	sub	si, [bx+14]	; BPB_ResvdSecCnt
	sbb	cx, ax
	mov	ax, [bx+17]	; BPB_RootEntCnt
	add	ax, 0xF
	rcr	ax, 1
	shr	ax, 3
	sub	si, ax
	sbb	cx, 0
	push	cx
	push	si
	mov	ax, word [bx+22]
	test	ax, ax
	jnz	@f
	mov	eax, [bx+36]
@@:
	movzx	ecx, byte [bx+16]
	imul	ecx, eax
	pop	eax
	sub	eax, ecx
; now eax = count of sectors in the data region
	xor	edx, edx
	div	[bp + sect_per_clust - dat]
; now eax = count of clusters in the data region
	mov	si, fatxx_msg
	cmp	eax, 0xFFF5
	jae	test_fat32
; test magic value in FAT bootsector - FAT12/16 bootsector has it at the offset +38
	cmp	byte [bx+38], 0x29
	jnz	not_fat
	cmp	ax, 0xFF5
	jae	fat16
fat12:
	mov	[bp + get_next_cluster_ptr - dat], fat12_get_next_cluster
	mov	di, cx		; BPB_NumFATs
	mov	ax, '12'
	push	ax		; save for secondary loader
	mov	word [si+3], ax
	call	out_string
	movzx	ecx, word [bx+22]	; BPB_FATSz16
; FAT12: read entire FAT table (it is no more than 0x1000*3/2 = 0x1800 bytes)
.fatloop:
; if first copy is not readable, try to switch to other copies
	push	0x6000
	pop	es
	xor	bx, bx
	movzx	eax, word [0x320E]	; BPB_RsvdSecCnt
	push	cx
	cmp	cx, 12
	jb	@f
	mov	cx, 12
@@:
	call	read
	pop	cx
	jnc	fat1x_common
	add	eax, ecx	; switch to next copy of FAT
	dec	di
	jnz	.fatloop
	mov	si, disk_error_msg
	jmp	find_error_si
fat16:
	mov	[bp + get_next_cluster_ptr - dat], fat16_get_next_cluster
	mov	ax, '16'
	push	ax		; save for secondary loader
	mov	word [si+3], ax
	call	out_string
; FAT16: init FAT cache - no sectors loaded
	mov	di, 0x3400
	xor	ax, ax
	mov	cx, 0x100/2
	rep	stosw
fat1x_common:
	mov	bx, 0x3200
	movzx	eax, word [bx+22]	; BPB_FATSz16
	xor	esi, esi	; no root cluster
	jmp	fat_common
test_fat32:
; FAT32 bootsector has it at the offset +66
	cmp	byte [bx+66], 0x29
	jnz	not_fat
	mov	[bp + get_next_cluster_ptr - dat], fat32_get_next_cluster
	mov	ax, '32'
	push	ax		; save for secondary loader
	mov	word [si+3], ax
	call	out_string
; FAT32 - init cache for FAT table: no sectors loaded
	lea	si, [bp + cache1head - dat]
	mov	[si], si		; no sectors in cache:
	mov	[si+2], si		; 'prev' & 'next' links point to self
	mov	[bp + cache1end - dat], 3400h	; first free item = 3400h
	mov	[bp + cache1limit - dat], 3C00h
	mov	eax, [bx+36]	; BPB_FATSz32
	mov	esi, [bx+44]	; BPB_RootClus
	jmp	fat_common
not_fat:
unknown_fs:
	mov	si, errfs_msg
	call	out_string
	jmp	next_partition
fat_common:
	push	ss
	pop	es
	movzx	edx, byte [bx+16]	; BPB_NumFATs
	mul	edx
	mov	[bp + root_start - dat], eax	; this is for FAT1x
; eax = total size of all FAT tables, in sectors
	movzx	ecx, word [bx+17]	; BPB_RootEntCnt
	add	ecx, 0xF
	shr	ecx, 4
	add	eax, ecx
	mov	cx, word [bx+14]	; BPB_RsvdSecCnt
	add	[bp + root_start - dat], ecx	; this is for FAT1x
	add	eax, ecx
; cluster 2 begins from sector eax
	movzx	ebx, byte [bx+13]	; BPB_SecPerClus
	sub	eax, ebx
	sub	eax, ebx
	mov	[bp + data_start - dat], eax
; no clusters in folders cache
	mov	di, foldcache_clus - 2
	xor	ax, ax
	mov	cx, 7*8/2 + 1
	rep	stosw
	mov	[bp + root_clus - dat], esi
; load secondary loader
	mov	[bp + load_file_ptr - dat], load_file_fat
load_secondary:
	push	0x1000
	pop	es
	xor	bx, bx
	mov	si, kernel_name
	mov	cx, 0x30000 / 0x200
	call	[bp + load_file_ptr - dat]
; say error if needed
	mov	si, error_too_big
	dec	bx
	js	@f
	jz	find_error_si
	mov	si, disk_error_msg
	jmp	find_error_si
@@:
; fill loader information and jump to secondary loader
	mov	al, 'h'		; boot device: hard drive
	mov	ah, [bp + boot_drive - dat]
	sub	ah, 80h		; boot device: identifier
	pop	bx		; restore file system ID ('12'/'16'/'32'/'nt')
	mov	si, callback
	jmp	1000h:0000h

nomem:
	mov	si, nomem_msg
	call	out_string
	jmp	$

ntfs:
	push	'nt'		; save for secondary loader
	mov	si, ntfs_msg
	call	out_string
	xor	eax, eax
	mov	[bp + data_start - dat], eax
	mov	ecx, [bx+40h]	; frs_size
	cmp	cl, al
	jg	.1
	neg	cl
	inc	ax
	shl	eax, cl
	jmp	.2
.1:
	mov	eax, ecx
	shl	eax, 9
.2:
	mov	[bp + frs_size - dat], ax
; standard value for frs_size is 0x400 bytes = 1 Kb, and it cannot be set different
; (at least with standard tools)
; we allow extra size, but no more than 0x1000 bytes = 4 Kb
	mov	si, invalid_volume_msg
	cmp	eax, 0x1000
	ja	find_error_si
; must be multiple of sector size
	test	ax, 0x1FF
	jnz	find_error_si
	shr	ax, 9
	xchg	cx, ax
; initialize cache - no data loaded
	lea	si, [bp + cache1head - dat]
	mov	[si], si
	mov	[si+2], si
	mov	word [si+4], 3400h	; first free item = 3400h
	mov	word [si+6], 3400h + 8*8	; 8 items in this cache
; read first MFT record - description of MFT itself
	mov	[bp + cur_obj - dat], mft_string
	mov	eax, [bx+30h]	; mft_cluster
	mul	[bp + sect_per_clust - dat]
	push	0x8000
	pop	es
	xor	bx, bx
	push	es
	call	read
	pop	ds
	call	restore_usa
; scan for unnamed $DATA attribute
	mov	[bp + freeattr - dat], 4000h
	mov	ax, 80h
	call	load_attr
	push	ss
	pop	ds
	mov	si, nodata_string
	jc	find_error_si
; load secondary loader
	mov	[bp + load_file_ptr - dat], load_file_ntfs
	jmp	load_secondary

find_error_si:
	push	si
find_error_sp:
	cmp	[bp + in_callback - dat], 0
	jnz	error_in_callback
	push	ss
	pop	ds
	push	ss
	pop	es
	mov	si, error_msg
	call	out_string
	mov	si, [bp + cur_obj - dat]
@@:
	lodsb
	test	al, al
	jz	@f
	cmp	al, '/'
	jz	@f
	mov	ah, 0Eh
	mov	bx, 7
	int	10h
	jmp	@b
@@:
	mov	si, colon
	call	out_string
	pop	si
	call	out_string
	mov	si, newline
	call	out_string
	mov	sp, 0x3000
	jmp	next_partition
error_in_callback:
; return status: file not found, except for read errors
	mov	bx, 2
	cmp	si, disk_error_msg
	jnz	@f
	inc	bx
@@:
	mov	ax, 0xFFFF
	mov	dx, ax
	mov	sp, 3000h - 6
	ret

callback:
; in: ax = function number; only functions 1 and 2 are defined for now
; save caller's stack
	mov	dx, ss
	mov	cx, sp
; set our stack (required because we need ss=0)
	xor	si, si
	mov	ss, si
	mov	sp, 3000h
	mov	bp, dat
	mov	[bp + in_callback - dat], 1
	push	dx
	push	cx
; set ds:si -> ASCIIZ name
	lea	si, [di+6]
; set cx = limit in sectors; 4Kb = 8 sectors
	movzx	ecx, word [di+4]
	shl	cx, 3
; set es:bx = pointer to buffer
	les	bx, [di]
; call our function
	stc	; unsupported function
	dec	ax
	jz	callback_readfile
	dec	ax
	jnz	callback_ret
	call	continue_load_file
	jmp	callback_ret_succ
callback_readfile:
; function 1: read file
; in: ds:di -> information structure
;	dw:dw	address
;	dw	limit in 4Kb blocks (0x1000 bytes) (must be non-zero and not greater than 0x100)
;	ASCIIZ	name
; out: bx=0 - ok, bx=1 - file is too big, only part of file was loaded, bx=2 - file not found, bx=3 - read error
; out: dx:ax = file size (0xFFFFFFFF if file was not found)
	call	[bp + load_file_ptr - dat]
callback_ret_succ:
	clc
callback_ret:
; restore caller's stack
	pop	cx
	pop	ss
	mov	sp, cx
; return to caller
	retf

read_file_chunk.resident:
; auxiliary label for read_file_chunk procedure
	mov	di, bx
	lodsw
read_file_chunk.resident.continue:
	mov	dx, ax
	add	dx, 0x1FF
	shr	dx, 9
	cmp	dx, cx
	jbe	@f
	mov	ax, cx
	shl	ax, 9
@@:
	xchg	ax, cx
	rep	movsb
	xchg	ax, cx
	clc	; no disk error if no disk requests
	mov	word [bp + num_sectors - dat], ax
	ret

read_file_chunk:
; in: ds:si -> file chunk
; in: es:bx -> buffer for output
; in: ecx = maximum number of sectors to read (high word must be 0)
; out: CF=1 <=> disk read error
	lodsb
	mov	[bp + cur_chunk_resident - dat], al
	test	al, al
	jz	.resident
; normal case: load (non-resident) attribute from disk
.read_block:
	lodsd
	xchg	eax, edx
	test	edx, edx
	jz	.ret
	lodsd
; eax = start cluster, edx = number of clusters, cx = limit in sectors
	imul	eax, [bp + sect_per_clust - dat]
	add	eax, [bp + data_start - dat]
	mov	[bp + cur_cluster - dat], eax
	imul	edx, [bp + sect_per_clust - dat]
	mov	[bp + num_sectors - dat], edx
	and	[bp + cur_delta - dat], 0
.nonresident.continue:
	cmp	edx, ecx
	jb	@f
	mov	edx, ecx
@@:
	test	dx, dx
	jz	.read_block
	add	[bp + cur_delta - dat], edx
	sub	[bp + num_sectors - dat], edx
	sub	ecx, edx
	push	cx
	mov	cx, dx
	call	read
	pop	cx
	jc	.ret
	test	cx, cx
	jnz	.read_block
.ret:
	ret

cache_lookup:
; in: eax = value to look, si = pointer to cache structure
; out: di->cache entry; CF=1 <=> the value was not found
	push	ds bx
	push	ss
	pop	ds
	mov	di, [si+2]
.look:
	cmp	di, si
	jz	.not_in_cache
	cmp	eax, [di+4]
	jz	.in_cache
	mov	di, [di+2]
	jmp	.look
.not_in_cache:
; cache miss
; cache is full?
	mov	di, [si+4]
	cmp	di, [si+6]
	jnz	.cache_not_full
; yes, delete the oldest entry
	mov	di, [si]
	mov	bx, [di]
	mov	[si], bx
	push	word [di+2]
	pop	word [bx+2]
	jmp	.cache_append
.cache_not_full:
; no, allocate new item
	add	word [si+4], 8
.cache_append:
	mov	[di+4], eax
	stc
	jmp	@f
.in_cache:
; delete this sector from the list
	push	si
	mov	si, [di]
	mov	bx, [di+2]
	mov	[si+2], bx
	mov	[bx], si
	pop	si
@@:
; add new sector to the end of list
	mov	bx, di
	xchg	bx, [si+2]
	push	word [bx]
	pop	word [di]
	mov	[bx], di
	mov	[di+2], bx
	pop	bx ds
	ret

include 'fat.inc'
include 'ntfs.inc'

total_kaput	db	13,10,'Fatal error: cannot load the secondary loader',0
error_too_big	db	'file is too big',0
nodata_string	db	'$DATA '
error_not_found	db	'not found',0
noindex_string	db	'$INDEX_ROOT not found',0
badname_msg	db	'bad name for FAT',0
invalid_volume_msg db	'invalid volume',0
mft_string	db	'$MFT',0
fragmented_string db	'too fragmented file',0
invalid_read_request_string db 'cannot read attribute',0

kernel_name	db	'kernel.mnt',0

align 4
dat:

extended_part_start	dd	0	; start sector for main extended partition
extended_part_cur	dd	?	; start sector for current extended child
extended_parent		dd	0	; start sector for current extended parent
partition_start		dd	0	; start sector for current logical disk
cur_partition_ofs	dw	?	; offset in MBR data for current partition
sect_per_clust		dd	0
; change this variable if you want to boot from other physical drive
boot_drive	db	80h
in_callback	db	0

; uninitialized data
use_lba		db	?
cur_chunk_resident db	?
align 2
heads		dw	?
sectors		dw	?
cache1head	rw	2
cache1end	dw	?
cache1limit	dw	?
data_start	dd	?
cachelimit	dw	?
load_file_ptr	dw	?
cur_obj		dw	?
missing_slash	dw	?
root_clus	dd	?
root_start	dd	?
get_next_cluster_ptr	dw	?
frs_size	dw	?
freeattr	dw	?
index_root	dw	?
index_alloc	dw	?
cur_index_seg	dw	?
cur_index_cache	dw	?
filesize	dd	?
filesize_sectors dd	?
cur_cluster	dd	?
cur_delta	dd	?
num_sectors	dd	?
sectors_read	dd	?
cur_chunk_ptr	dw	?

rootcache_size	dw	?	; must be immediately before foldcache_clus
if $-dat >= 0x80
warning: unoptimal data displacement!
end if
foldcache_clus	rd	7
foldcache_mark	rw	7
foldcache_size	rw	7
fat_filename	rb	11

if $ > 2000h
error: file is too big
end if

; for NT/2k/XP, file must be 16 sectors = 0x2000 bytes long
repeat 0x2600 - $
	db	2	; any data can be here; 2 is another nice face in ASCII :)
end repeat
