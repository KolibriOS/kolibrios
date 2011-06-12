;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  BOOTCODE.INC                                                ;;
;;                                                              ;;
;;  KolibriOS 16-bit loader,                                    ;;
;;                        based on bootcode for MenuetOS        ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;==========================================================================
;
;                           16 BIT FUNCTIONS
;
;==========================================================================

if 0
putchar:
; in: al=character
	mov	ah, 0Eh
	mov	bh, 0
	int	10h
	ret

print:
; in: si->string
	mov	al, 186
	call	putchar
	mov	al, ' '
	call	putchar

printplain:
; in: si->string
	pusha
	lodsb
@@:
	call	putchar
	lodsb
	test	al,al
	jnz	@b
	popa
	ret


setcursor:
; in: dl=column, dh=row
	mov	ah, 2
	mov	bh, 0
	int	10h
	ret

macro _setcursor row,column
{
	mov	dx, row*256 + column
	call	setcursor
}

end if

boot_read_floppy:
	push	si
	xor	si, si
	mov	ah, 2	; read
@@:
	push	ax
	int	0x13
	pop	ax
	jnc	@f
	inc	si
	cmp	si, 10
	jnb	$
@@:
	pop	si
	ret

sayerr_plain:
	jmp	$

sayerr:
	jmp	$


; convert abs. sector number (AX) to BIOS T:H:S
; sector number = (abs.sector%BPB_SecPerTrk)+1
; pre.track number = (abs.sector/BPB_SecPerTrk)
; head number = pre.track number%BPB_NumHeads
; track number = pre.track number/BPB_NumHeads
; Return: cl - sector number
;         ch - track number
;         dl - drive number (0 = a:)
;         dh - head number
conv_abs_to_THS:
	push	bx
	mov	bx,word [BPB_SecPerTrk]
	xor	dx,dx
	div	bx
	inc	dx
	mov	cl, dl				; cl = sector number
	mov	bx,word [BPB_NumHeads]
	xor	dx,dx
	div	bx
	; !!!!!!! ax = track number, dx = head number
	mov	ch,al				; ch=track number
	xchg	dh,dl				; dh=head number
	mov	dl,0				; dl=0 (drive 0 (a:))
	pop	bx
	retn
; needed variables
BPB_SecPerTrk	dw	0			; sectors per track
BPB_NumHeads	dw	0			; number of heads
BPB_FATSz16	dw	0			; size of FAT
BPB_RootEntCnt	dw	0			; count of root dir. entries
BPB_BytsPerSec	dw	0			; bytes per sector
BPB_RsvdSecCnt	dw	0			; number of reserved sectors
BPB_TotSec16	dw	0			; count of the sectors on the volume
BPB_SecPerClus	db	0			; number of sectors per cluster
BPB_NumFATs	db	0			; number of FAT tables
abs_sector_adj	dw	0			; adjustment to make abs. sector number
end_of_FAT	dw	0			; end of FAT table
FirstDataSector dw	0			; begin of data

;=========================================================================
;
;                           16 BIT CODE
;
;=========================================================================

include 'bootvesa.inc'		       ;Include source for boot vesa

start_of_code:
	cld
; \begin{diamond}[02.12.2005]
; if bootloader sets ax = 'KL', then ds:si points to loader block
	cmp	ax, 'KL'
	jnz	@f
	mov	word [cs:cfgmanager.loader_block], si
	mov	word [cs:cfgmanager.loader_block+2], ds
@@:
; \end{diamond}[02.12.2005]

; if bootloader sets cx = 'HA' and dx = 'RD', then bx contains identifier of source hard disk
; (see comment to bx_from_load)
	cmp	cx, 'HA'
	jnz	no_hd_load
	cmp	dx,'RD'
	jnz	no_hd_load
	mov	word [cs:bx_from_load], bx		; {SPraid}[13.03.2007]
no_hd_load:

; set up stack
	mov	ax, 3000h
	mov	ss, ax
	mov	sp, 0EC00h
; set up segment registers
	push	cs
	pop	ds
	push	cs
	pop	es

; set videomode
	mov	ax, 3
	int	0x10

; draw frames
	push	0xb800
	pop	es
	xor	di, di
	mov	ah, 1*16+15

cpugood:

	push	0
	popf
	sti

; set up esp
	movzx	esp, sp

	push	0
	pop	es
	and	word [es:0x9031], 0
; \begin{Mario79}
; find HDD IDE DMA PCI device
; check for PCI BIOS
	mov	ax, 0xB101
	int	0x1A
	jc	.nopci
	cmp	edx, 'PCI '
	jnz	.nopci
; find PCI class code
; class 1 = mass storage
; subclass 1 = IDE controller
; a) class 1, subclass 1, programming interface 0x80
	mov	ax, 0xB103
	mov	ecx, 1*10000h + 1*100h + 0x80
	xor	si, si	; device index = 0
	int	0x1A
	jnc	.found
; b) class 1, subclass 1, programming interface 0x8A
	mov	ax, 0xB103
	mov	ecx, 1*10000h + 1*100h + 0x8A
	xor	si, si	; device index = 0
	int	0x1A
	jnc	.found
; c) class 1, subclass 1, programming interface 0x85
	mov	ax, 0xB103
	mov	ecx, 1*10000h + 1*100h + 0x85
	xor	si, si
	int	0x1A
	jc	.nopci
.found:
; get memory base
	mov	ax, 0xB10A
	mov	di, 0x20	; memory base is config register at 0x20
	int	0x1A
	jc	.nopci
	and	cx, 0xFFF0	; clear address decode type
	mov	[es:0x9031], cx
.nopci:
; \end{Mario79}

; --------------- APM ---------------------
	and	word [es:0x9044], 0	; ver = 0.0 (APM not found)
	mov	ax, 0x5300
	xor	bx, bx
	int	0x15
	jc	apm_end 		; APM not found
	test	cx, 2
	jz	apm_end 		; APM 32-bit protected-mode interface not supported
	mov	[es:0x9044], ax 	; Save APM Version
	mov	[es:0x9046], cx 	; Save APM flags

	mov	ax, 0x5304		; Disconnect interface
	xor	bx, bx
	int	0x15
	mov	ax, 0x5303		; Connect 32 bit mode interface
	xor	bx, bx
	int	0x15

	mov	[es:0x9040], ebx
	mov	[es:0x9050], ax
	mov	[es:0x9052], cx
	mov	[es:0x9054], dx

apm_end:

;CHECK current of code
	cmp	[cfgmanager.loader_block], -1
	jz	noloaderblock
	les	bx, [cfgmanager.loader_block]
	cmp	byte [es:bx], 1
	jnz	sayerr
	push	0
	pop	es

noloaderblock:
; DISPLAY VESA INFORMATION
	 call	 print_vesa_info
	 call	 calc_vmodes_table
	 call	 check_first_parm  ;check and enable cursor_pos

; \begin{diamond}[30.11.2005]
cfgmanager:
; settings:
; a) preboot_graph = graphical mode
;    preboot_gprobe = probe this mode?
; b) preboot_dma  = use DMA access?
; c) preboot_vrrm = use VRR?
; d) preboot_device = from what boot?

; determine default settings
;        mov     [.bSettingsChanged], 0

;.preboot_gr_end:
	mov	di, preboot_device
; if image in memory is present and [preboot_device] is uninitialized,
; set it to use this preloaded image
	cmp	byte [di], 0
	jnz	.preboot_device_inited
	cmp	[.loader_block], -1
	jz	@f
	les	bx, [.loader_block]
	test	byte [es:bx+1], 1
	jz	@f
	mov	byte [di], 3
	jmp	.preboot_device_inited
@@:
; otherwise, set [preboot_device] to 1 (default value - boot from floppy)
	mov	byte [di], 1
.preboot_device_inited:
; following 4 lines set variables to 1 if its current value is 0
	cmp	byte [di+preboot_dma-preboot_device], 1
	adc	byte [di+preboot_dma-preboot_device], 0
	cmp	byte [di+preboot_biosdisk-preboot_device], 1
	adc	byte [di+preboot_biosdisk-preboot_device], 0

;        pop     ax              ; <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ??
	jmp	.continue
.loader_block dd -1
.continue:
	sti
	jmp	 .load

.loadc:
	pop	eax
.cont:
	push	cs
	pop	ds
.load:

; ASK GRAPHICS MODE

	call	set_vmode

; GRAPHICS ACCELERATION
; force yes
	mov	[es:0x901C], byte 1

; DMA ACCESS TO HD

	mov	al, [preboot_dma]
	mov	[es:0x901F], al

; VRR_M USE

	mov	al,[preboot_vrrm]
	mov	[es:0x9030], al
	mov	[es:0x901E], byte 1

; BOOT DEVICE

	mov	al, [preboot_device]
	dec	al
	mov	[boot_dev], al

; GET MEMORY MAP
include 'detect/biosmem.inc'

; READ DISKETTE TO MEMORY

if 0
----
	cmp	[boot_dev],0
	jne	no_sys_on_floppy
	xor	ax, ax		  ; reset drive
	xor	dx, dx
	int	0x13
; do we boot from CD-ROM?
	mov	ah, 41h
	mov	bx, 55AAh
	xor	dx, dx
	int	0x13
	jc	.nocd
	cmp	bx, 0AA55h
	jnz	.nocd
	mov	ah, 48h
	push	ds
	push	es
	pop	ds
	mov	si, 0xa000
	mov	word [si], 30
	int	0x13
	pop	ds
	jc	.nocd
	push	ds
	lds	si, [es:si+26]
	test	byte [ds:si+10], 40h
	pop	ds
	jz	.nocd
; yes - read all floppy by 18 sectors

; TODO: !!!! read only first sector and set variables !!!!!
; ...
; TODO: !!! then read flippy image track by track

	mov	cx, 0x0001	; startcyl,startsector
.a1:
	push	cx dx
	mov	al, 18
	mov	bx, 0xa000
	call	boot_read_floppy
	mov	si, movedesc
	push	es
	push	ds
	pop	es
	mov	cx, 256*18
	mov	ah, 0x87
	int	0x15
	pop	es
	pop	dx cx
	test	ah, ah
	jnz	sayerr_floppy
	add	dword [si+8*3+2], 512*18
	inc	dh
	cmp	dh, 2
	jnz	.a1
	mov	dh, 0
	inc	ch
	cmp	ch, 80
	jae	ok_sys_on_floppy
	mov	al, ch
	shr	ch, 2
	jmp	.a1
.nocd:
; no - read only used sectors from floppy
; now load floppy image to memory
; at first load boot sector and first FAT table

; read only first sector and fill variables
	mov	cx, 0x0001	; first logical sector
	xor	dx, dx		; head = 0, drive = 0 (a:)
	mov	al, 1		; read one sector
	mov	bx, 0xB000	; es:bx -> data area
	call	boot_read_floppy
; fill the necessary parameters to work with a floppy
	mov	ax, word [es:bx+24]
	mov	word [BPB_SecPerTrk], ax
	mov	ax, word [es:bx+26]
	mov	word [BPB_NumHeads], ax
	mov	ax, word [es:bx+17]
	mov	word [BPB_RootEntCnt], ax
	mov	ax, word [es:bx+14]
	mov	word [BPB_RsvdSecCnt], ax
	mov	ax, word [es:bx+19]
	mov	word [BPB_TotSec16], ax
	mov	al, byte [es:bx+13]
	mov	byte [BPB_SecPerClus], al
	mov	al, byte [es:bx+16]
	mov	byte [BPB_NumFATs], al
;<Lrz> 18.11.2008
	mov	ax, word [es:bx+22]
	mov	word [BPB_FATSz16], ax
	mov	cx, word [es:bx+11]
	mov	word [BPB_BytsPerSec], cx

; count of clusters in FAT12 ((size_of_FAT*2)/3)
;        mov     ax, word [BPB_FATSz16]
;        mov     cx, word [BPB_BytsPerSec]
;end <Lrz> 18.11.2008
	xor	dx, dx
	mul	cx
	shl	ax, 1
	mov	cx, 3
	div	cx		; now ax - number of clusters in FAT12
	mov	word [end_of_FAT], ax

; load first FAT table
	mov	cx, 0x0002	; startcyl,startsector          ; TODO!!!!!
	xor	dx, dx		; starthead,drive
	mov	al, byte [BPB_FATSz16]	   ; no of sectors to read
	add	bx, word [BPB_BytsPerSec]  ; es:bx -> data area
	call	boot_read_floppy
	mov	bx, 0xB000

; and copy them to extended memory
	mov	si, movedesc
	mov	[si+8*2+3], bh		; from

	mov	ax, word [BPB_BytsPerSec]
	shr	ax, 1			; words per sector
	mov	cx, word [BPB_RsvdSecCnt]
	add	cx, word [BPB_FATSz16]
	mul	cx
	push	ax			; save to stack count of words in boot+FAT
	xchg	ax, cx

	push	es
	push	ds
	pop	es
	mov	ah, 0x87
	int	0x15
	pop	es
	test	ah, ah
	jz	@f
sayerr_floppy:
	mov	dx, 0x3f2
	mov	al, 0
	out	dx, al
;        mov     si, memmovefailed
	jmp	$
@@:
	pop	ax			; restore from stack count of words in boot+FAT
	shl	ax, 1			; make bytes count from count of words
	and	eax, 0ffffh
	add	dword [si+8*3+2], eax

; copy first FAT to second copy
; TODO: BPB_NumFATs !!!!!
	add	bx, word [BPB_BytsPerSec]	; !!! TODO: may be need multiply by BPB_RsvdSecCnt !!!
	mov	byte [si+8*2+3], bh	; bx - begin of FAT

	mov	ax, word [BPB_BytsPerSec]
	shr	ax, 1			; words per sector
	mov	cx, word [BPB_FATSz16]
	mul	cx
	mov	cx, ax			; cx - count of words in FAT

	push	es
	push	ds
	pop	es
	mov	ah, 0x87
	int	0x15
	pop	es
	test	ah, ah
	jnz	sayerr_floppy

	mov	ax, cx
	shl	ax, 1
	and	eax, 0ffffh		; ax - count of bytes in FAT
	add	dword [si+8*3+2], eax

; reading RootDir
; TODO: BPB_NumFATs
	add	bx, ax
	add	bx, 100h
	and	bx, 0ff00h			; bx - place in buffer to write RootDir
	push	bx

	mov	bx, word [BPB_BytsPerSec]
	shr	bx, 5				; divide bx by 32
	mov	ax, word [BPB_RootEntCnt]
	xor	dx, dx
	div	bx
	push	ax				; ax - count of RootDir sectors

	mov	ax, word [BPB_FATSz16]
	xor	cx, cx
	mov	cl, byte [BPB_NumFATs]
	mul	cx
	add	ax, word [BPB_RsvdSecCnt]	; ax - first sector of RootDir

	mov	word [FirstDataSector], ax
	pop	bx
	push	bx
	add	word [FirstDataSector], bx	; Begin of data region of floppy

; read RootDir
	call	conv_abs_to_THS
	pop	ax
	pop	bx				; place in buffer to write
	push	ax
	call	boot_read_floppy		; read RootDir into buffer
; copy RootDir
	mov	byte [si+8*2+3], bh		; from buffer
	pop	ax				; ax = count of RootDir sectors
	mov	cx, word [BPB_BytsPerSec]
	mul	cx
	shr	ax, 1
	mov	cx, ax				; count of words to copy
	push	es
	push	ds
	pop	es
	mov	ah, 0x87
	int	0x15
	pop	es

	mov	ax, cx
	shl	ax, 1
	and	eax, 0ffffh		; ax - count of bytes in RootDir
	add	dword [si+8*3+2], eax	; add count of bytes copied

; Reading data clusters from floppy
	mov	byte [si+8*2+3], bh
	push	bx

	mov	di, 2			; First data cluster
.read_loop:
	mov	bx, di
	shr	bx, 1			; bx+di = di*1.5
	jnc	.even
	test	word [es:bx+di+0xB200], 0xFFF0	; TODO: may not be 0xB200 !!!
	jmp	@f
.even:
	test	word [es:bx+di+0xB200], 0xFFF	; TODO: may not be 0xB200 !!!

@@:
	jz	.skip
; read cluster di
;.read:
	;conv cluster di to abs. sector ax
	; ax = (N-2) * BPB_SecPerClus + FirstDataSector
	mov	ax, di
	sub	ax, 2
	xor	bx, bx
	mov	bl, byte [BPB_SecPerClus]
	mul	bx
	add	ax, word [FirstDataSector]
	call	conv_abs_to_THS
	pop	bx
	push	bx
	mov	al, byte [BPB_SecPerClus]	; number of sectors in cluster
	call	boot_read_floppy
	push	es
	push	ds
	pop	es
	pusha
;
	mov	ax, word [BPB_BytsPerSec]
	xor	cx, cx
	mov	cl, byte [BPB_SecPerClus]
	mul	cx
	shr	ax, 1				; ax = (BPB_BytsPerSec * BPB_SecPerClus)/2
	mov	cx, ax				; number of words to copy (count words in cluster)
;
	mov	ah, 0x87
	int	0x15				; copy data
	test	ah, ah
	popa
	pop	es
	jnz	sayerr_floppy
; skip cluster di
.skip:
	mov	ax, word [BPB_BytsPerSec]
	xor	cx, cx
	mov	cl, byte [BPB_SecPerClus]
	mul	cx
	and	eax, 0ffffh		; ax - count of bytes in cluster
	add	dword [si+8*3+2], eax

	mov	ax, word [end_of_FAT]	; max cluster number
	pusha
; draw percentage
; total clusters: ax
; read clusters: di
	xchg	ax, di
	mov	cx, 100
	mul	cx
	div	di
	aam
	xchg	al, ah
	add	ax, '00'
@@:
	popa
	inc	di
	cmp	di, word [end_of_FAT]	; max number of cluster
	jnz	.read_loop
	pop	bx			; clear stack

ok_sys_on_floppy:
no_sys_on_floppy:
	xor	ax, ax		; reset drive
	xor	dx, dx
	int	0x13
	mov	dx, 0x3f2	; floppy motor off
	mov	al, 0
	out	dx, al
---
end if


; SET GRAPHICS

	xor	ax, ax
	mov	es, ax

	mov	bx, [es:0x9008] 	; vga & 320x200
	mov	ax, 0x4f02		; Vesa
setgr:
	int	0x10
	test	ah, ah
	jnz	$
gmok2:
	push	ds
	pop	es
