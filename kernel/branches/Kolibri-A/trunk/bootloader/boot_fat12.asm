; FAT12 boot sector for KolibriOS
;
; Copyright (C) Alex Nogueira Teixeira
; Copyright (C) Diamond
; Copyright (C) Dmitry Kartashov aka shurf
;
; Distributed under GPL, see file COPYING for details
;
; Version 1.0

lf		equ	0ah
cr		equ	0dh

pos_read_tmp	equ	0700h			;position for temporary read
boot_program	equ	07c00h			;position for boot code
seg_read_kernel	equ	01000h			;segment to kernel read

	jmp	start_program
	nop

; Boot Sector and BPB Structure
include 'floppy1440.inc'
;include 'floppy2880.inc'

start_program:

	xor	ax,ax
	mov	ss,ax
	mov	sp,boot_program
	push	ss
	pop	ds

	; print loading string
	mov	si,loading+boot_program
loop_loading:
	lodsb
	or	al,al
	jz	read_root_directory
	mov	ah,0eh
	mov	bx,7
	int	10h
	jmp	loop_loading

read_root_directory:
	push	ss
	pop	es

	; reading root directory
	; al=count root dir sectors !!!! TODO: al, max 255 sectors !!!!
	mov	ah,2				; read
	push	ax

	mov	ax,word [FirstRootDirSecNum+boot_program]
	call	conv_abs_to_THS			; convert abs sector (AX) to BIOS T:H:S (track:head:sector)
	pop	ax
	mov	bx,pos_read_tmp			; es:bx read buffer
	call	read_sector

	mov	si,bx				; read buffer address: es:si
	mov	ax,[RootDirSecs+boot_program]
	mul	word [BPB_BytsPerSec+boot_program]
	add	ax,si				; AX = end of root dir. in buffer pos_read_tmp

	; find kernel file in root directory
loop_find_dir_entry:
	push	si
	mov	cx,11
	mov	di,kernel_name+boot_program
	rep	cmpsb				; compare es:si and es:di, cx bytes long
	pop	si
	je	found_kernel_file
	add	si,32				; next dir. entry
	cmp	si,ax				; end of directory
	jb	loop_find_dir_entry
        jmp     $

	; === KERNEL FOUND. LOADING... ===

found_kernel_file:
	mov	bp,[si+01ah]			; first cluster of kernel file
	; <diamond>
	mov	[cluster1st+boot_program],bp	; starting cluster of kernel file
	; <\diamond>

	; reading first FAT table
	mov	ax,word [BPB_RsvdSecCnt+boot_program]	; begin first FAT abs sector number
	call	conv_abs_to_THS			; convert abs sector (AX) to BIOS T:H:S (track:head:sector)
	mov	bx,pos_read_tmp			; es:bx read position
	mov	ah,2				; ah=2 (read)
	mov	al, byte [BPB_FATSz16+boot_program]	; FAT size in sectors (TODO: max 255 sectors)
	call	read_sector

	mov	ax,seg_read_kernel
	mov	es,ax
	xor	bx,bx				; es:bx = 1000h:0000h

	; reading kernel file
loop_obtains_kernel_data:
	call	obtain_cluster	                ; read one cluster of file

	mov	ax,es
	add	ax,seg_inc_per_cluster          ; << =32(1.44M) or 64(2.88) >>
	mov	es,ax	                ; add one cluster length to segment:offset

	mov	di,bp
	shr	di,1
	pushf
	add	di,bp				; di = bp * 1.5
	add	di,pos_read_tmp
	mov	ax,[di]				; read next entry from FAT-chain
	popf
	jc	move_4_right
	and	ax,0fffh
	jmp	verify_end_sector
move_4_right:
	shr	ax,4
verify_end_sector:
	cmp	ax,0ff8h			; last cluster
	jae	execute_kernel
	mov	bp,ax
	jmp	loop_obtains_kernel_data

execute_kernel:
	; <diamond>
	mov	ax,'KL'
	push	0
	pop	ds
	mov	si,loader_block+boot_program
	; </diamond>
	push	word	seg_read_kernel
	push	word	0
	retf					; jmp far 1000:0000


;------------------------------------------
	; loading cluster from file to es:bx
obtain_cluster:
	; bp - cluster number to read
	; carry = 0 -> read OK
	; carry = 1 -> read ERROR

	; print one dot
	push	bx
	mov	ax,0e2eh			; ah=0eh (teletype), al='.'
	xor	bh,bh
	int	10h
;	jc	$               		; error
        pop     bx

writesec:
	; convert cluster number to sector number
	mov	ax,bp				; data cluster to read
	sub	ax,2
;>>	add	ax,ax                           ; << only for 2.88M disks! >>
	add	ax,word [data_start+boot_program]
	call	conv_abs_to_THS			; convert abs sector (AX) to BIOS T:H:S (track:head:sector)

patchhere:
	mov	ax,0x201			; ah=2 (read)
;>>                    	                        ; al=1(1.44) or 2(2.88)
	; read sector from disk

read_sector:
	push	ax bx cx dx
	int	13h
	jc	$                               ; error: stop here
	pop	dx cx bx ax
	retn

;------------------------------------------
	; converts abs. sector number (AX) to BIOS T:H:S
	; Returns: {pre.track number = (AX / BPB_SecPerTrk)}
	;	ch - track number  = pre.track number/2
        ;       cl - sector number = (abs.sector%BPB_SecPerTrk)+1
	;	dh - head number   = pre.track number%BPB_NumHeads
	;	dl - drive number (0 = a:)
conv_abs_to_THS:
	mov	cx,word [BPB_SecPerTrk+boot_program]    ; << 18 or 36 >>
	xor	dx,dx
	div	cx
	inc	dx
	mov	cl, dl				; cl = sector number
        xor     dx, dx
        shr     ax, 1
        rol     dx, 1           		; dh = head number
        mov     ch, al				; ch = track number
	retn
;------------------------------------------

loading		db	cr,lf,'Starting system ',00h
;error_message	db	13,10
kernel_name	db	'KERNEL  MNT',00h
FirstRootDirSecNum	dw	19      ; 1st sector of the root dir
RootDirSecs	dw	14              ; << = 14(1.44M) or 15(2.88M) >>
data_start	dw	33		; << = 33(1.44M) or 34(2.88M) >>

; <diamond>
write1st:
	push	cs
	pop	ds
	mov	byte [patchhere+1+boot_program], 3	; change ah=2 to ah=3
	mov	bp,[cluster1st+boot_program]
	push	1000h
	pop	es
	xor	bx,bx
	call	writesec
	mov	byte [patchhere+1+boot_program], 2	; change back ah=3 to ah=2
	retf

cluster1st	dw	?       ; 1st cluster of kernel.mnt
loader_block:
		db	1
		dw	0
		dw	write1st+boot_program
		dw	0
; <\diamond>

times	0x1fe-$ db 00h

	db	55h,0aah 			;boot signature
