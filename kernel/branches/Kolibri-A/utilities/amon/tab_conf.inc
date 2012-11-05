;
; Ghost Monitor - утилита (стресс) тестирования и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

;
;
; Вкладка настроек
;

	; Время обновления
	mov	eax, 8
	mov	ebx, 200 * 65536 + 15
	mov	ecx, 280 * 65536 + 15
	mov	edx, 11
	mov	esi, tcol
	int	0x40
	
	mov	ebx, 217 * 65536 + 15
	inc	edx
	int	0x40
	
	mov	eax, 4
	mov	ebx, 17 * 65536 + 285
	mov	edx, msg_cf_utime
	mov	ecx, 0x10000000
	mov	esi, 11
	int	0x40
	
	mov	ebx, 205 * 65536 + 285
	mov	edx, msg_cf_pm
	xor	ecx, ecx
	mov	esi, 4
	int	0x40
	
	mov	ecx, [update_time]
	call	digit_len
	mov	edx, 160 * 65536 + 285
	xor	esi, esi
	mov	eax, 47
	int	0x40
	
	; Расставляем двоеточия
	mov	eax, 4
	mov	ebx, 152 * 65536 + 285
	mov	edx, dub_p
	mov	esi, 1
	mov	ecx, 0x10000000
	int	0x40
	
	jmp	end_drow_tab
	
msg_cf_utime	db	'Update time'	; 11
msg_cf_pm	db	'+  -' ; 4