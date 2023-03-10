;
; Ghost Monitor - ??????? (??????) ???????????? ? ??????????? ???????
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

; Integrated Technology Express
;       Chip    Temp    Volt    Fan   ISA   SMBus
;       it8705   3       8+1*    3     +      +
;       it8712   3       8+1*    3     +      +

; SiS
;       Chip    Temp    Volt    Fan   ISA   SMBus
;       sis950   3       8+1*    3     +      +
;
; * - VBAT

IT87_REGCHIP	equ	0x58
IT87_CHIPID	equ	0x90
IT87_FANDIV	equ	0x0B
it8705		db	'IT8705F/SiS 950', 0
it8712		db	'IT8712F', 0
it8716		db	'IT8716F', 0
ite_unk 	db	'Unknown ITE', 0

ite_coeff:	dd 0.016		; Vcore
		dd 0.016		; Vin0
		dd 0.016		; Vin1 (+3.3V)
		dd 0.02688		; AVcc (+5V)
		dd 0.0608		; Vin2 (+12V)
		dd -0.055632		; -12V
		dd -0.02408		; -5V
;-----------------------------------
it87_init:
; ???????? ??????? ? ?????????????
; OUT - CF = 1 - error
	cmp	byte[acc_type], 2	; Only ISA and SMBus
	jae	.no_io

	mov	ecx, 0x2e
	mov	edx, 0x2f
	call	ReservePorts
	jc	.no_io

	mov	eax, 0x55550187 	; ??????? ? MB PnP Mode
	out	0x2e, al
	shr	eax, 8
	out	0x2e, al
	shr	eax, 8
	out	0x2e, al
	shr	eax, 8
	out	0x2e, al

	mov	al, 0x20
	out	0x2e, al
	in	al, 0x2f		; al = 0x87
	push	eax

	mov	al, 0x21
	out	0x2e, al
	in	al, 0x2f		; al = model
	push	eax

	mov	al, 0x02		; ????? ?? ??????
	out	0x2E, al
	out	0x2F, al

	mov	ecx, 0x2e
	mov	edx, 0x2f
	call	FreePorts

	pop	eax
	pop	ebx
	cmp	bl, 0x87
	jne	.no_io

	mov	edx, it8705
	cmp	al, 0x05
	je	@f
	mov	edx, it8712
	cmp	al, 0x12
	je	@f
	mov	edx, it8716
	cmp	al, 0x16
	je	@f
	mov	edx, ite_unk
@@:	mov	[hwm_chip_name], edx
	clc
	ret

.no_io: stc
	ret

	; cmp   byte[acc_type], 2       ; Only ISA and SMBus
	; jae   it87_no
	;;--- ????????? IT87* --------
	; mov   al, IT87_REGCHIP
	; call  [IO_Read]
	; cmp   al, IT87_CHIPID
	; jne   it87_no         ; ??? ?? it87 !!!
	;;-~- not tested ~-~-
	; mov   al, 0x21        ; --- ?????? ????????????? ???? --
	; call  [IO_Read]
	; mov   edx, it8705
	; cmp   al, 0x05
	; je    @f
	; mov   edx, it8712
	; cmp   al, 0x12
	; je    @f
	; mov   edx, it8716
	; cmp   al, 0x16
	; je    @f
	; mov   edx, ite_unk
; @@:   mov     [hwm_chip_name], edx
	;;-~-~-~-~-~-~-~-~-~-
	; clc
	; ret
; it87_no:stc
	; ret

;-----------------------------------
it87_getparam:
	call	it87_get_temp
	call	it87_get_fan_speed
	mov	edi, ite_coeff
	call	wb_get_volt
	ret
;-----------------------------------
it87_get_temp:
	xor	ecx, ecx
	mov	esi, hwm_temps
@@:	mov	eax, ecx
	add	al, 0x29
	call	[IO_Read]
	mov	[esi + ecx * 2], al
	inc	ecx
	cmp	ecx, 3
	jb	@b
	ret
;-----------------------------------
it87_fan_div	db	1, 1, 1
it87_get_fan_speed:
; ?????? ????????
	mov	al, IT87_FANDIV
	call	[IO_Read]

	mov	ah, al
	and	al, 0x07
	mov	[it87_fan_div], al
	shr	ah, 3
	and	ah, 0x07
	mov	[it87_fan_div + 1], ah

	xor	ecx, ecx
@@:	mov	al, 0x0D
	add	al, cl
	call	[IO_Read]

	test	al, al
	jz	@f
	cmp	al, 0xff
	je	@f

	movzx	ebx, al
	push	ecx
	mov	cl, [it87_fan_div + ecx]
	shl	ebx, cl
	pop	ecx
	mov	eax, 1350000
	xor	edx, edx
	test	ebx, ebx
	jz	@f
	div	ebx
	mov	[hwm_rpms + 4 * ecx], eax
	inc	ecx
	cmp	ecx, 3
	jb	@b
@@:
	ret
;--------------------------------------------------------------------------
;
; Presets for IT8712
;
; Volt = A * read_val + B
;
;       A, B
; --- 0 ---
; dd 0.01565, 0.0               ; VIN0 (Index = 0x20)
; dd 0.016, 0.0                 ; VIN1 (Index = 0x21)
; dd 0.016, 0.0                 ; VIN2 (Index = 0x22)
; dd 0.0, 0.0                   ; VIN3 (Index = 0x23)
; dd 0.0608, 0.0                ; VIN4 (Index = 0x24)
; dd 0.0, 0.0                   ; VIN5 (Index = 0x25)
; dd 0.0, 0.0                   ; VIN6 (Index = 0x26)
; dd 0.08224, -22.104           ; VIN7 (Index = 0x27)
; dd 0.016, 0.0                 ; VBAT (Index = 0x28)

; --- 1 ---
; dd 0.01614, 0.0
; dd 0.01614, 0.0
; dd 0.01614, 0.0
; dd 0.01614, 0.0
; dd 0.062946, 0.0
; dd 0.0, 0.0
; dd 0.0, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0

; --- 2 ---
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.062946, 0.0
; dd 0.016, 0.0
; dd 0.01614, 0.0
; dd 0.0, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0

; --- 3 ---
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.027, 0.0
; dd 0.06564, 0.0
; dd 0.084, -17.408
; dd 0.0512, -9.0112
; dd 0.016, 0.0
; dd 0.016, 0.0

; --- 4 ---
; dd 0.01653, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.02684, 0.0
; dd 0.06398, 0.0
; dd 0.0, 0.0
; dd 0.0, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0

; --- 5 ---
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.027, 0.0
; dd 0.06369, 0.0
; dd 0.082285714288, -16.9691428598999
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0

; --- 6 ---
; dd 0.01565, 0.0
; dd 0.06564, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.02688, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0

; --- 7 ---
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.027, 0.0
; dd 0.06369, 0.0
; dd 0.082285714288, -16.9691428598999
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0

; --- 8 ---
; dd 0.01653, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.0265, 0.0
; dd 0.06398, 0.0
; dd 0.0, 0.0
; dd 0.0, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0

; --- 9 ---
; dd 0.016, 0.0
; dd 0.01565, 0.0
; dd 0.016, 0.0
; dd 0.0265, 0.0
; dd 0.06398, 0.0
; dd 0.09152, -20.592
; dd 0.03728, -8.388
; dd 0.016, 0.0
; dd 0.016, 0.0

; --- 10 ---
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.0265, 0.0
; dd 0.06398, 0.0
; dd 0.016, 0.0
; dd 0.0512, -9.0112
; dd 0.0, 0.0
; dd 0.0, 0.0

; --- 11 ---
; dd 0.016, 0.0
; dd 0.0, 0.0
; dd 0.016, 0.0
; dd 0.016, 0.0
; dd 0.06564, 0.0
; dd 0.0, 0.0
; dd 0.0, 0.0
; dd 0.0, 0.0
; dd 0.0, 0.0
