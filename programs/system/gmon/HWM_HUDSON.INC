;
; Ghost Monitor - утилита (стресс) тестирования и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

;
; Библиотека для чипов Hudson
;
;
;
;                  | Chip name           | ID           Vin     Fanin   PWM     Temp   ISA   SMBus
msg_A50:	db 'AMD Hudson A50', 0	  ; 0x20|0x21      9        3      2       3      +      +
msg_A55:	db 'AMD Hudson A55', 0	  ; 0x20|0x21      9        3      2       3      +      +

msg_amdunk:	db 'Unknown AMD', 0 ; other, non zero ;-)

uglobal
wb_fans_num	db	0
endg

;-----------------------------------
hudson_init:
; OUT - CF = 1 - error

	mov	ebx, 0xF00C3000
	mov	eax, [ebx]
	cmp	eax, 0x17031022
	jne	.not_found	     ; not a Fusion!

	mov	byte[wb_fans_num], 3
	mov	edx, msg_A55
@@:	mov	[hwm_chip_name], edx
	mov	dword[hwm_enable],1
	mov	al, 0xE6
	mov	bl, 0x0A
	call	pm2write     ; init PWM_Control register


	clc
	ret
.not_found:  stc
	ret

;-----------------------------------
hudson_getparam:
	call	hudson_get_temp
	call	hudson_get_fan_speed
	mov	edi, hudson_coeff
	call	hudson_get_volt
	fld	dword[V12]
	fld	dword[hudson_n12v_const]
	faddp	st1, st0
	fstp	dword[V12]
	ret
;-----------------------------------
hudson_get_temp:
	; temp 1
	mov	ebx, 0xF00C3000
	mov	eax, [ebx+0xA4]
	mov	edx, eax
	shr	edx, 24
	mov	[hwm_temps], dl ; integer degrees
	mov	edx, eax
	shr	edx, 21
	and	dl, 7		; 1/8th fractions of degree
	cmp	dl, 3		; round 3/8 upto 0.4
	jb	.corrected
	inc	dl
	cmp	dl, 8		; 7/8 ~ 0.9
	jb	.corrected
	inc	dl
.corrected:
	mov	[hwm_temps + 1], dl

	; temp 2 (3 SYSTIN)
	xor	ebx, ebx
	mov	al, 0x95
	call	pm2read
	mov	bl, al
	mov	al, 0x96	 ; SB internal sensor
	call	pm2read
	mov	bh, al
	mov	[hudson_temp_read], ebx
	fild	dword[hudson_temp_read]
	fmul	dword[hudson_temp_coef]
	fidiv	dword[hudson_int_64]
	fadd	dword[hudson_temp_offs]
	fimul	dword[hudson_int_2]
	fistp	dword[hudson_temp_read]
	mov	bx, word[hudson_temp_read]
	mov	ax, bx
	xor	bh, bh
	shr	eax, 1
	mov	byte[hwm_temps + 2], al
	or	bl, 1
	jz	@f
	mov	bh, 5
	mov	byte[hwm_temps + 3], bh


	; temp 3 (VTIN)
	xor	ebx, ebx
	mov	al, 0xA1
	call	pm2read
	mov	bl, al
	mov	al, 0xA2	; temp sensor #2
	call	pm2read
	mov	bh, al
	mov	[hudson_temp_read], ebx
	fild	dword[hudson_temp_read]
	fmul	dword[hudson_temp_coef]
	fidiv	dword[hudson_int_64]
	fadd	dword[hudson_temp_offs]
	fimul	dword[hudson_int_2]
	fistp	dword[hudson_temp_read]
	mov	bx, word[hudson_temp_read]
	mov	ax, bx
	xor	bh, bh
	shr	eax, 1
	mov	byte[hwm_temps + 4], al
	or	bl, 1
	jz	@f
	mov	bh, 5
	mov	byte[hwm_temps + 5], bh


	mov	ecx, 3
	mov	esi, hwm_temps
hudson_check_temp:
	cmp	word[esi + ecx * 2 - 2], 0x057F
	jne	hudson_temp_ok
	mov	word[esi + ecx * 2 - 2], 0
hudson_temp_ok:
	loop	hudson_check_temp

	ret
;-----------------------------------
hudson_get_fan_speed:
	; fan1
	mov	al, 0x47
	call	[IO_Read]
	and	al, 0x30
	shr	al, 4
	mov	ebx, 1
	mov	cl, al
	shl	ebx, cl ; <- div1
	xor	eax, eax
	mov	al,  0x28
	call	[IO_Read]
	cmp	al, 255
	jne	@f
	xor	eax, eax	; ???
	ret			; ???
@@:	mul	ebx
	mov	ebx, eax
	mov	eax, 1350000
	xor	edx, edx
	test	ebx, ebx
	jz	.div0
	div	ebx
	mov	[hwm_rpms], eax

	mov	al, 0x47
	call	[IO_Read]
	shr	al, 6
	mov	ebx, 1
	mov	cl, al
	shl	ebx, cl ; <- div2
	xor	eax, eax
	mov	al,  0x29
	call	[IO_Read]
	cmp	al, 255
	jne	@f
	xor	eax, eax
.div0:
	ret
@@:	mul	ebx
	mov	ebx, eax
	mov	eax, 1350000
	xor	edx, edx
	test	ebx, ebx
	jz	.div0
	div	ebx
	mov	[hwm_rpms + 4], eax

	mov	al, 0x4B
	call	[IO_Read]
	shr	al, 6
	mov	ebx, 1
	mov	cl, al
	shl	ebx, cl ; <- div3
	xor	eax, eax
	mov	al,  0x2A
	call	[IO_Read]
	cmp	al, 255
	jne	@f
	xor	eax, eax
	ret
@@:	mul	ebx
	mov	ebx, eax
	mov	eax, 1350000
	xor	edx, edx
	test	ebx, ebx
	jz	.div0
	div	ebx
	mov	[hwm_rpms + 8], eax

	cmp	byte[wb_fans_num], 3
	jna	.wb_f_e

	mov	al, 0x59
	call	[IO_Read]
	and	al, 3
	mov	ebx, 1
	mov	cl, al
	shl	ebx, cl ; <- div4, дописать учёт DIV_B2
	xor	eax, eax
	mov	al,  0x3F
	call	[IO_Read]
	cmp	al, 255
	jne	@f
	xor	eax, eax
	ret
@@:	mul	ebx
	mov	ebx, eax
	mov	eax, 1350000
	xor	edx, edx
	test	ebx, ebx
	jz	.div0
	div	ebx
	mov	[hwm_rpms + 12], eax

	mov	al, 0x59
	call	[IO_Read]
	shr	al, 2
	and	al, 3
	mov	ebx, 1
	mov	cl, al
	shl	ebx, cl ; <- div5, дописать учёт DIV_B2
	xor	eax, eax
	mov	al, 0x4e	; Выбираем bank 5
	mov	bl, 5
	call	[IO_Write]
	mov	al,  0x53
	call	[IO_Read]
	cmp	al, 255
	jne	@f
	xor	eax, eax
	ret
@@:	mul	ebx
	mov	ebx, eax
	mov	eax, 1350000
	xor	edx, edx
	test	ebx, ebx
	jz	.wb_f_e
	div	ebx
	mov	[hwm_rpms + 16], eax

.wb_f_e:
	ret
;-----------------------------------
hudson_get_volt:
	;mov    edi, wb_coeff           ; <-- possible differences for miscellaneous chip
	mov	esi, hwm_voltages
	xor	ecx, ecx
@@:	mov	eax, ecx
	add	al, 0x20
	call	[IO_Read]
	fld	dword [edi + ecx * 4]
	push	eax
	fild	dword [esp]
	fmulp	st1, st0
	fstp	dword [esi + ecx * 4]
	pop	eax
	inc	ecx
	cmp	ecx, 7
	jb	@b
	ret

align 4
hudson_coeff:	    dd 0.016		    ; Vcore
		dd 0.016		; Vin0
		dd 0.016		; Vin1 (+3.3V)
		dd 0.02688		; AVcc (+5V)
		dd 0.0608		; Vin2 (+12V)
		dd 0.0822857142857145	; -12V
		dd -0.02408		; -5V ; false

hudson_n12v_const   dd -14.9142857142857

hudson_temp_offs    dd -273.65
hudson_temp_coef    dd 0.517
hudson_temp_read    dd 0
hudson_int_2	    dd 2
hudson_int_64	    dd 64



