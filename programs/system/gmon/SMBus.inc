;
; Ghost Monitor - утилита (стресс) тестировани€ и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; SMBus interface
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

include		'smb_nf2.inc'

smb_base	dd	0	; IO port base for SMBus acces
smb_slave	dd	0	; SMBus addres for monitoring chip

;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
smbInit:
	; »зменить, код только дл€ тестировани€ !!!
	; mov	dword[IO_Read ], nf2_readb
	; mov	dword[IO_Write], nf2_writeb
	
	
	; See pci_pm.c -> pci_pm_smb_prob
	

.exit_err:
	stc
	ret
.exit_ok:
	clc
	ret
	
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; pci_bus_num:	db	2	; Ќомер шины
; pci_devfn:	db	255

; find_north_bridg:
	; mov	bl, 6
	; xor	cl, cl
; nbus:	mov	bh, [pci_bus_num]
; ndevfn:	mov	ch, [pci_devfn]
	; mov	eax, 62
	; int	0x40
	; cmp	eax, 0xffffffff
	; je	bd_next
	;;;;---------
	; mov	esi, bd_table_end - bd_table - 12
; @@:	cmp	eax, [bd_table + esi]
	; je	bd_found
	; test	esi, esi
	; jz	bd_next
	; sub	esi, 12
	; jmp	@b
	;;;;---------
; bd_next:dec	byte[pci_devfn]
	; jns	ndevfn
	; mov	byte[pci_devfn], 0
	; dec	byte[pci_bus_num]
	; jns	nbus
	; ret
; bd_found:
	; add	esi, bd_table + 4
	; mov	edi, nb_name
	; mov	ecx, 2
	; rep	movsd

	; mov	[bd_id], eax
	; ret
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
smbClose:
	
	ret
;-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
