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

boot_dev  db	  0  ; 0=floppy, 1=hd


;=========================================================================
;
;                           16 BIT CODE
;
;=========================================================================

diff16 "start_of_code: ",0,$

start_of_code:
	cld

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
        xor     ax, ax
	push	ax
	popf
	sti

; set up esp
	movzx	esp, sp

	push	ax
	pop	es
	mov	[es:0x9031], ax

; --------------- APM - removed--------------------
	and	word [es:0x9044], 0	; ver = 0.0 (APM not found)

cfgmanager:


; DMA ACCESS TO HD

	mov	al, 1
	mov	[es:0x901F], al

; GRAPHICS ACCELERATION
; force yes
	mov	[es:0x901C], al


; VRR_M USE

	mov	[es:0x9030], byte 2
	mov	[es:0x901E], al

; BOOT DEVICE

	xor	ax, ax
	mov	[boot_dev], al
	mov	es, ax


; SET GRAPHICS

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
