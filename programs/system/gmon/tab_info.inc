;
; Ghost Monitor - утилита (стресс) тестирования и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

;
;
; Вкладка информации о системе
;
; Дописать ещё чтоить (маловато будет)
;
;

	; Пишем параметры
	mov	edx, lab_info_tab
	mov	ebx, 17 * 65536 + 285
	call	show_text

	; Пишем производителя процессора
	mov	edx, Vendor
	mov	esi, 12
	mov	ebx, 160 * 65536 + 285
	int	0x40
	; Поддержка MMX
	mov	ebx, 160 * 65536 + 305
	mov	esi, 3
	mov	edx, msg_yes
	cmp	byte[CPU_mmx], 1
	je	mmx_en
	mov	edx, msg_no
mmx_en:	int	0x40
	; Чип мониторинга
	mov	edx, [hwm_chip_name]
	or	ecx, 0x80000000
	mov	ebx, 160 * 65536 + 325
	mov	al, 4
	int	0x40
	; Частота процессора
	mov	eax, 18
	mov	ebx, 5
	int	0x40

	xor	edx, edx
	mov	ebx, 1000000
	div	ebx
	mov	ecx, eax
	call	digit_len
	mov	eax, 47
	mov	edx, 160 * 65536 + 295
	xor	esi, esi
	int	0x40

	; push	eax			; Вернуть код после исправления ftoa
	; fild	dword[esp]
	; push	1000;000
	; fidiv	dword[esp]
	; fidiv	dword[esp]
	; fstp	dword[esp]
	; mov	esi, esp
	; call	ftoa
	; mov	ebx, 160 * 65536 + 295
	; mov	edx, Data_String
	; mov	eax, 4
	; mov	esi, 7
	; xor	ecx, ecx
	; int	0x40
	; pop	eax
	; pop	eax

	; Семейство и модель процессора
	mov	eax, 47
	mov	ebx, 0x10100
	xor	ecx, ecx
	mov	cl, [CPU_fam]
	mov	edx, 250 * 65536 + 285
	xor	esi, esi
	int	0x40
	mov	cl, [CPU_mod]
	add	edx, 10 * 65536
	int	0x40

	; коэффициент умножения
	call	multiplier
	test	ecx, ecx
	jz	@f
	movzx	ecx, cl
	push	ecx
	fild	dword[esp]
	push	10
	fidiv	dword[esp]
	fstp	dword[esp]
	mov	esi, esp
	call	ftoa
	mov	ebx, 160 * 65536 + 315
	mov	edx, Data_String
	mov	eax, 4
	mov	esi, eax
	xor	ecx, ecx
	int	0x40
	pop	eax
	pop	eax
@@:

	; Расставляем двоеточия
	mov	eax, 4
	mov	ebx, 152 * 65536 + 285
	mov	edx, dub_p
	mov	esi, 1
	mov	ecx, 6		; <- количество строк
nxt_dp:	push	ecx
	xor	ecx, ecx
	int	0x40
	add	ebx, 10
	pop	ecx
	loop	nxt_dp
;---------------------------------------------------------------------
	jmp	redraw
	
free_mem:
	; Свободная память
if (OS_version >= 0x0530)
	mov	eax, 18
	mov	ebx, 16
	int	0x40
	mov	ecx, eax
	call	digit_len
	mov	eax, 47
	mov	edx, 160 * 65536 + 335
	xor	esi, esi
	clear_prev_no edx
	int	0x40
end if
	jmp end_drow_tab
	
dub_p	db	':'

if lang eq it
	lab_info_tab mls \
		'Vendor CPU',\
		'Frequenza CPU (MHz)',\
		'MMX tehnology',\
		'Multiplier',\
		'Monitoring chip',\
		'Memoria libera (Kb)'
else
	lab_info_tab mls \
		'CPU vendor',\
		'CPU frequency (MHz)',\
		'MMX technology',\
		'Multiplier',\
		'Monitoring chip',\
		'Free memory (Kb)'
end if
