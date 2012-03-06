;
; Ghost Monitor - утилита (стресс) тестирования и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

;
; Библиотека для чипов WinBond
;
;
;    Дописать получение инфы с других датчиков
;    W83627DHG мониторинг аналогичен W83627EHF, дописать учёт DIV_B2
;  + 16.03.06 Исправлена ошибка в wb_get_cpu_temper с десятых градуса (было 30.5 30.0 31.5 ...)
;
;
;    О маркировке (?) : H - H/W Monitor, G - Pb-free package, T - ACPI, S - Smart Card Reader Interface
;      D - SuperIO supporting Intel* PECI* & SST* interfaces, U - UART
;
;                  | Chip name           | ID           Vin     Fanin   PWM     Temp   ISA   SMBus
msg_wb27HF:	db 'W83627HF', 0	; 0x20|0x21      9        3      2       3      +      +
msg_wb27THF:	db 'W83627THF', 0	; 0x90|0x91      7        3      3       3      +      +
msg_wb37THF:	db 'W83637THF', 0	; 0x80           7        3      3       3      +      +
msg_wb97HF:	db 'W83697HF', 0	; 0x60|0x61      8        2      2       2      +      -
msg_wb27THFA:	db 'W83627THF-A', 0	; 0x1A

msg_w83627DHG:	db 'W83627DHG', 0	; 0xC1           ?        5      ?       3      +      +
msg_w83627EHF:	db 'W83627EHF', 0	; 0x88           10       5      3       3      +      +
msg_w83627EHG:	db 'W83627EHG', 0	; 0xA1           10       5      3       3      +      +


;               db 'W83781D', 0         ; 0x10|0x11      7               3       3      +      +
;               db 'W83782D', 0         ; 0x30           9               3       3      +      +
;               db 'AS99127F', 0        ; 0x31           7               3       3      -      +
;               db 'W83783S', 0         ; 0x40|0x41      5-6             3       1-2    -      +
;               db 'W83791D', 0         ; 0x70|0x71      10              5       3      -      +
msg_wbunk:	db 'Unknown Winbond', 0 ; other, non zero ;-)

uglobal
wb_fans_num	db	0
endg

;-----------------------------------
wb_init:
; Проверка наличия и инициализация
; OUT - CF = 1 - error
	cmp	byte[acc_type], 2	; Only ISA and SMBus
	jae	wb_no

	mov	al, 0x4e
	xor	bl, bl
	call	[IO_Write]
	mov	al, 0x4f
	call	[IO_Read]
	mov	ah, al		;!
	mov	al, 0x4e
	mov	bl, 0x80
	call	[IO_Write]
	mov	al, 0x4f
	call	[IO_Read]
	cmp	ax, 0xa35c
	jne	wb_no		; это не Winbond !!!

	; --- узнаём идентификатор чипа --
	mov	al, 0x58
	call	[IO_Read]
	and	al, 0xFE	; 0x20 = 0x21, 0x60 = 0x61, etc
	mov	byte[wb_fans_num], 3
	mov	edx, msg_wb27HF
	cmp	al, 0x20
	je	@f
	mov	edx, msg_wb27THF
	cmp	al, 0x90
	je	@f
	mov	edx, msg_wb37THF
	cmp	al, 0x80
	je	@f
	mov	edx, msg_wb97HF
	cmp	al, 0x60
	je	@f
	mov	edx, msg_wb27THFA
	cmp	al, 0x1A
	je	@f
	mov	byte[wb_fans_num], 5
	mov	edx, msg_w83627EHF
	cmp	al, 0x88
	je	@f
	mov	edx, msg_w83627EHG
	cmp	al, 0xA0
	je	@f
	mov	edx, msg_w83627DHG
	cmp	al, 0xC0
	je	@f
	mov	byte[wb_fans_num], 3
	mov	edx, msg_wbunk
@@:	mov	[hwm_chip_name], edx
	clc
	ret
wb_no:	stc
	ret

;-----------------------------------
wb_getparam:
	call	wb_get_temp
	call	wb_get_fan_speed
	mov	edi, wb_coeff
	call	wb_get_volt
	fld	dword[V12]
	fld	dword[wb_n12v_const]
	faddp	st1, st0
	fstp	dword[V12]
	ret
;-----------------------------------
wb_get_temp:
	; temp 1
	mov	al, 0x4e	; Выбираем bank 1
	mov	bl, 1
	call	[IO_Write]
	mov	al, 0x50	; Получаем старший байт температуры (градусы)
	call	[IO_Read]
	mov	[hwm_temps], al
	mov	al, 0x51	; Получаем младший байт температуры  (x.5`C)
	call	[IO_Read]
	cmp	al, 0		; if al == 0 then x.0, else x.5
	je	@f
	mov	al, 5
@@:	mov	[hwm_temps + 1], al

	; temp 2 (3 SYSTIN)
	mov	al, 0x4e	; Выбираем bank 0
	xor	bl, bl
	call	[IO_Write]
	mov	al, 0x27	; Получаем старший байт температуры (градусы)
	call	[IO_Read]
	mov	[hwm_temps + 2], al

	; temp 3 (VTIN)
	mov	al, 0x4e	; Выбираем bank 2
	mov	bl, 2
	call	[IO_Write]
	mov	al, 0x50	; Получаем старший байт температуры (градусы)
	call	[IO_Read]
	mov	[hwm_temps + 4], al
	mov	al, 0x51	; Получаем младший байт температуры  (x.5`C)
	call	[IO_Read]
	cmp	al, 0		; if al == 0 then x.0, else x.5
	je	@f
	mov	al, 5
@@:	mov	[hwm_temps + 5], al

	; Проверка температуры, датчики с 127.5`C не используются
	mov	ecx, 3
	mov	esi, hwm_temps
wb_check_temp:
	cmp	word[esi + ecx * 2 - 2], 0x057F
	jne	wb_temp_ok
	mov	word[esi + ecx * 2 - 2], 0
wb_temp_ok:
	loop	wb_check_temp

	ret
;-----------------------------------
wb_get_fan_speed:
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
wb_get_volt:
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

wb_coeff:	dd 0.016		; Vcore
		dd 0.016		; Vin0
		dd 0.016		; Vin1 (+3.3V)
		dd 0.02688		; AVcc (+5V)
		dd 0.0608		; Vin2 (+12V)
		dd 0.0822857142857145	; -12V
		dd -0.02408		; -5V ; false

wb_n12v_const	dd -14.9142857142857



