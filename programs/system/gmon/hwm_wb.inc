;
; Библиотека для чипов WinBond
;
;
;    Дописать получение инфы с других датчиков
;  + 16.03.06 Исправлена ошибка в wb_get_cpu_temper с десятых градуса (было 30.5 30.0 31.5 ...)
;
;
uglobal
wb_chip_id:	db 0    		; Идентификатор чипа (если нашли)
endg
;		      | Chip name	| ID		Vin	Fanin	PWM	Temp
msg_wb27HF:	db 8, 'W83627HF'	; 0x20|0x21	 9	  3	 2	 3
msg_wb27THF:	db 9, 'W83627THF'	; 0x90|0x91	 7	  3	 3	 3
msg_wb37THF:	db 9, 'W83637THF'	; 0x80		 7	  3	 3	 3
msg_wb97HF:	db 8, 'W83697HF'	; 0x60|0x61	 8	  2	 2	 2
msg_wb27EHF:	db 11,'W83627EHF-A'	; 0x88
msg_wb27THFA:	db 11,'W83627THF-A'	; 0x1A
;		db 7, 'W83781D'		; 0x10|0x11	 7	  	 3	 3
;		db 7, 'W83782D'		; 0x30		 9	   	 3	 3
;		db 8, 'AS99127F'	; 0x31
;		db 7, 'W83783S'		; 0x40|0x41	 5-6	   	 3	 1-2
;		db 7, 'W83791D'		; 0x70|0x71	 10	   	 5	 3
msg_wbunk:	db 15,'Unknown Winbond'	; other, non zero ;-)

;-----------------------------------
wb_init:
; Проверка наличия и инициализация
; OUT - CF = 1 - error
	mov	dx, 0x295
	mov	al, 0x4e
	out	dx, al
	inc	edx
	xor	al, al
	out	dx, al
	dec	edx
	mov	al, 0x4f
	out	dx, al
	inc	edx
	in	al, dx
	mov	ah, al		;!
	dec	edx
	mov	al, 0x4e
	out	dx, al
	inc	edx
	mov	al, 0x80
	out	dx, al
	dec	edx
	mov	al, 0x4f
	out	dx, al
	inc	edx
	in	al, dx
	cmp	ax, 0xa35c
	jne	wb_no		; это не Winbond !!!
	dec	edx		; --- узнаём идентификатор чипа --
	mov	al, 0x58
	out	dx, al
	inc	edx
	in	al, dx
	mov	byte[wb_chip_id], al
	clc
	ret
wb_no:	stc
	ret

;-----------------------------------
wb_get_temp:
	; temp 1
	mov	dx, 0x295	; Выбираем bank 1
	mov	al, 0x4e
	out	dx, al
	inc	edx
	mov	al, 1
	out	dx, al

	dec	edx		; Получаем старший байт температуры (градусы)
	mov	al, 0x50
	out	dx, al
	inc	edx
	in	al, dx
	mov	[temps], al
	dec	edx		; Получаем младший байт температуры  (0.5`C)
	mov	al, 0x51
	out	dx, al
	inc	edx
	in	al, dx		; if al == 0 then 0.0, else 0.5
	cmp	al, 0
	je	@f
	mov	al, 5
@@:	mov	[temps + 1], al
	; temp 2 (3 SYSTIN)
	dec	edx		; Выбираем bank 0
	mov	al, 0x4e
	out	dx, al
	inc	edx
	mov	al, 0
	out	dx, al

	dec	edx		; Получаем старший байт температуры (градусы)
	mov	al, 0x27
	out	dx, al
	inc	edx
	in	al, dx
	mov	[temps + 2], al
	; temp 3 (VTIN)
	dec	edx		; Выбираем bank 2
	mov	al, 0x4e
	out	dx, al
	inc	edx
	mov	al, 2
	out	dx, al

	dec	edx		; Получаем старший байт температуры (градусы)
	mov	al, 0x50
	out	dx, al
	inc	edx
	in	al, dx
	mov	[temps + 4], al
	dec	edx		; Получаем младший байт температуры  (0.5`C)
	mov	al, 0x51
	out	dx, al
	inc	edx
	in	al, dx		; if al == 0 then 0.0, else 0.5
	cmp	al, 0
	je	@f
	mov	al, 5
@@:	mov	[temps + 5], al
	; Проверка температуры, датчики с 127.5`C не используются
	mov	ecx, 3
	mov	esi, temps
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
	mov	dx, 0x295
	out	dx, al
	inc	edx
	in	al, dx
	and	al, 0x30
	shr	al, 4
	mov	ebx, 1
	mov	cl, al
	shl	ebx, cl	; <- div1
	dec	edx
	mov	al, 0x28
	out	dx, al
	inc	edx
	xor	eax, eax
	in	al, dx
	cmp	al, 255
	jne	@f
	xor	eax, eax
	ret
@@:	mul	ebx
	mov	ebx, eax
	mov	eax, 1350000
	xor	edx, edx
	div	ebx
	mov	[rpms], eax

	mov	al, 0x47
	mov	dx, 0x295
	out	dx, al
	inc	edx
	in	al, dx
	shr	al, 6
	mov	ebx, 1
	mov	cl, al
	shl	ebx, cl	; <- div2
	dec	edx
	mov	al, 0x29
	out	dx, al
	inc	edx
	xor	eax, eax
	in	al, dx
	cmp	al, 255
	jne	@f
	xor	eax, eax
	ret
@@:	mul	ebx
	mov	ebx, eax
	mov	eax, 1350000
	xor	edx, edx
	div	ebx
	mov	[rpms + 4], eax
	ret
;-----------------------------------
wb_get_volt:
	mov	dx, 0x295
	;mov	edi, vcoeff		; <-- possible differences for miscellaneous chip
	mov	esi, Vcore
	xor	ecx, ecx	
@@:	mov	eax, ecx
	add	al, 0x20
	out	dx, al
	inc	edx
	in	al, dx
	fld	dword [edi + ecx * 4]
	push	eax
	fild	dword [esp]
	fmulp	st1, st0
	fstp	dword [esi + ecx * 4]
	pop	eax
	inc	ecx
	dec	edx
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
;-----------------------------------
wb_get_name:
; Возвращает имя чипа
; Out : EDX - pointer to string (size:string - Pascal style)
	mov	al, [wb_chip_id]
	and	al, 0xFE	; 0x20 = 0x21, 0x60 = 0x61, etc
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
	mov	edx, msg_wb27EHF
	cmp	al, 0x88
	je	@f
	mov	edx, msg_wb27THFA
	cmp	al, 0x1A
	je	@f
	mov	edx, msg_wbunk
@@:	ret
	
	
