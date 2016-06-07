include 'cfg_bios.inc'
use16
org	0

rom_header:
;	PnP Option ROM header
rom_signature	dw	0xAA55			; +0	: magic
rom_length	db	1			; +2	: number of 512byte blocks
rom_entry:
		jmp	init_entry		; +3	: initialization entry point
;		 retf				 ; +5
@@:
	times (24-$) db 0			;	: left blank
rom_pci_struc	dw	pci_header		; +18h	: offset to PCI data structure

	times (32-$) db 0

; +32
pci_header:
;	PCI Data Structure
pci_magic	db	'PCIR'			; +0	: magic
pci_vendor	dw	BIOS_PCI_VENDOR 	; +4	: 10EE
pci_device	dw	BIOS_PCI_DEVICE 	; +6	: 0007
pci_vdata	dw	0			; +8	: 0 = no vital product data
pci_length	dw	0x18			; +A	: PCI data structure length
pci_classrev	dd	BIOS_PCI_CLASS		; +C	: rev.00 + class 05.00.00
pci_size	dw	1			; +10	: image length (512byte blocks)
pci_rev 	dw	0			; +12
pci_codetype	db	0			; +14	: x86
pci_indicator	db	0x80			; +15	: last image
pci_maxlen	dw	1			; +16	: max length (512b blocks)
pci_hdrptr	dw	0; manstr		   ; +18   : ExROM's config utility code header
pci_dmtf_entry	dw	0			; +1A	: DMTP CLP entry point

	times (64-$) db 0

manstr:
	db	1,'Kolibri-A Operating System',13,10,0
						; utility code header size = 42 bytes
	times (64+41-$) db  0			;
	db	1				; ver.1

utility_entry:
	jmp boot_failure
	times (128-$) db 0

;align 128

boot_failure:
	int	18h				; return to BIOS Boot sequence

align	4
init_entry:
; note x86 "unreal" mode!
; ax	=  b:d.f
; bx	=  run-time segment
; cx	=  PMM segment
; dx	=  PMM entry offset

	mov	dx, 0x0cf8			; PCI config address
;	 mov	 eax, 0x80010010		 ; BAR0
	and	eax, 0x0FFFF
	shl	eax, 8
	or	eax, 0x80000010 		;BAR0
	out	dx, eax
	out	dx, eax
	add	dl, 4
	in	eax, dx
	and	eax, 0xFFFFFF00
	add	eax, 0x0400			; LPC control reg
	mov	dword[eax], 0x90050000		; LED port
	mov	dword[eax+4], 0x055
	xor	eax, eax
	sub	dl, 4
	out	dx, eax
	mov	bx, ax
	retf


;	 mov	 edx,[esp]			 ; get the origin
;	 and	 dl, 0xF0
;	 shl	 ebx,  4			 ; correct run-time address
;	 mov	 cx, 0				 ; words counter
;@@:
;	 mov	 ax, word[edx+ecx]
;	 mov	 [ebx+ecx],ax
;	 inc	 cx
;	 inc	 cx
;	 test	 cx,0x200
;	 jz  @b
;	 xor	 ax, ax
;	 mov	 bx, ax
;	 mov	 al, 0x20			 ; IPL device's attached

;	 retd					 ; double !


	times (0xDF-$) db 0		; checksum position

sum = 0
repeat	$-$$
	load a byte from $$+% -1
	sum = a + sum
end repeat
check = sum mod 256
check_byte   db      (0x100 - check ) mod 256

times	(512-$) db 0