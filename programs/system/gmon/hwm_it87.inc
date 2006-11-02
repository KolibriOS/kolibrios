; Integrated Technology Express
;	Chip	Temp	Volt	Fan
;	it8705	 3	 8	 3
;	it8712	 3	 8	 3

; SiS
;	Chip	Temp	Volt	Fan
;	sis950	 3	 8	 3

IT87_REGCHIP	equ	0x58
IT87_CHIPID	equ	0x90
IT87_FANDIV	equ	0x0B
uglobal
it_chip_id	db	0
endg
it8705		db	15, 'IT8705F/SiS 950'
it8712		db	 7, 'IT8712F'
it8716		db	 7, 'IT8716F'
ite_unk		db	11, 'Unknown ITE'

ite_coeff:	dd 0.016		; Vcore
		dd 0.016		; Vin0
		dd 0.016		; Vin1 (+3.3V)
		dd 0.02688		; AVcc (+5V)
		dd 0.0608		; Vin2 (+12V)
		dd -0.055632		; -12V
		dd -0.02408		; -5V
;-----------------------------------
it87_init:
; Проверка наличия и инициализация
; OUT - CF = 1 - error
	;--- Проверяем IT87* --------
	mov	dx, 0x295
	mov	al, IT87_REGCHIP
	out	dx, al
	inc	edx
	in	al, dx
	cmp	al, IT87_CHIPID
	jne	it87_no		; это не it87 !!!
	; -~- not tested ~-~-
	dec	edx		; --- узнаём идентификатор чипа --
	mov	al, 0x21
	out	dx, al
	inc	edx
	in	al, dx
	mov	byte[it_chip_id], al
	; -~-~-~-~-~-~-~-~-~-
	clc
	ret
it87_no:stc
	ret
;-----------------------------------
it87_get_temp:
	mov	dx, 0x296
	xor	ecx, ecx
	mov	esi, temps
@@:	dec	edx
	mov	eax, ecx
	add	al, 0x29
	out	dx, al
	inc	edx
	in	al, dx
	mov	[esi + ecx * 2], al
	inc	ecx
	cmp	ecx, 3
	jb	@b
	ret
;-----------------------------------
it87_fan_div	db	1, 1, 1
it87_get_fan_speed:
; читаем делители
	mov	dx, 0x295
	mov	al, IT87_FANDIV
	out	dx, al
	inc	dx
	in	al, dx
	mov	ah, al
	and	al, 0x07
	mov	[it87_fan_div], al
	shr	ah, 3
	and	ah, 0x07
	mov	[it87_fan_div + 1], ah
	
	xor	ecx, ecx
@@:	mov	dx, 0x295
	mov	al, 0x0D
	add	al, cl
	out	dx, al
	inc	edx
	in	al, dx
	movzx	ebx, al
	push	ecx
	mov	cl, [it87_fan_div + ecx]
	shl	ebx, cl
	pop	ecx
	mov	eax, 1350000
	xor	edx, edx
	div	ebx
	mov	[rpms + 4 * ecx], eax
	inc	ecx
	cmp	ecx, 3
	jb	@b

	ret
;--------------------------------------------------------------------------
it87_get_name:
	mov	edx, it8705
	cmp	byte[it_chip_id], 0x05
	je	@f
	mov	edx, it8712
	cmp	byte[it_chip_id], 0x12
	je	@f
	mov	edx, it8716
	cmp	byte[it_chip_id], 0x16
	je	@f
	mov	edx, ite_unk
@@:	ret
