;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  BOOTCODE.INC                                                ;;
;;                                                              ;;
;;  Kolibri-A auxiliary 16-bit code,                            ;;
;;                        based on bootcode for KolibriOS       ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;==========================================================================
;
;                           16 BIT FUNCTIONS
;
;==========================================================================


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
 if 0
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
end if

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
;         call    print_vesa_info
;         call    calc_vmodes_table
;         call    check_first_parm  ;check and enable cursor_pos

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

;       call    set_vmode

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


; SET GRAPHICS

	xor	ax, ax
	mov	es, ax

;        mov     bx, [es:0x9008]         ; vga & 320x200
	mov	ax, 0xA000		; AtomBIOS Fn00
	mov	cx, 0x550A		; 1024x768, 32bpp, ARGB8888
setgr:
	int	0x10
	test	ah, ah
	jnz	$
	mov	ax, 0xA006		; AtomBIOS Fn06
	int	0x10
	mov	[es:0x9018], ebx	; LFB

gmok2:
	push	ds
	pop	es
