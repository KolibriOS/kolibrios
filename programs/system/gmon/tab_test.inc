;
; Ghost Monitor - утилита (стресс) тестирования и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved
;
; Вкладка тестов
;
; "GenuineIntel" - International Electronics
; "GenuineTMx86" - Transmeta Processor
; "AuthenticAMD" - Advanced Micro Devices
; "AMD ISBETTER" - Advanced Micro Devices
; "UMC UMC UMC " - United Microelectronics Corporation
; "CyrixInstead" - Cyrix Processor
; "Geode by NSC" - National Semiconductor Processor
; "SiS SiS SiS " - SiS Processor
; "RiseRiseRise" - Rise Processor
; "NexGenDriven" - NexGen Processor  (acquired by AMD)
; "CentaurHauls" - IDT/Centaur, now VIA Processor

	; Pentium (P5) button
	mov	eax, 8
	mov	ebx, 17 * 65536 + 145
	mov	ecx, 297 * 65536 + 25
	mov	edx, 6
	mov	esi, tcol
	cmp	byte[Vendor + 11], 'l'
	jne	p5n
	cmp	byte[CPU_fam], 5
	jne	p5n
	mov	esi, atcol
p5n:	int	0x40
	; Pentium Pro / II / III (P6) button
	add	ecx, 27 * 65536
	inc	edx
	mov	esi, tcol
	cmp	byte[Vendor + 11], 'l'
	jne	p6n
	cmp	byte[CPU_fam], 6
	jne	p6n
	mov	esi, atcol
p6n:	int	0x40
	; AMD K6 button
	add	ecx, 27 * 65536
	inc	edx
	mov	esi, tcol
	cmp	byte[Vendor], 'A'
	jne	k6n
	cmp	byte[CPU_fam], 5
	jne	k6n
	mov	esi, atcol
k6n:	int	0x40
	; AMD K7 (Athlon / Duron)
	add	ecx, 27 * 65536
	inc	edx
	mov	esi, tcol
	cmp	byte[Vendor], 'A'
	jne	k7n
	cmp	byte[CPU_fam], 6
	jne	k7n
	mov	esi, atcol
k7n:	int	0x40
	; Пишем названия кнопок
	mov	eax, 4
	mov	ebx, 30 * 65536 + 307
	mov	esi, 7
	mov	edx, tmsg_p	; P5
	xor	ecx, ecx
	cmp	byte[test_id], 6
	jne	nr1
	mov	ecx, 0xFF0000
nr1:	int	0x40
	add	ebx, 27
	mov	esi, 20		; P6
	xor	ecx, ecx
	cmp	byte[test_id], 7
	jne	nr2
	mov	ecx, 0xFF0000
nr2:	int	0x40
	add	ebx, 27
	mov	edx, tmsg_k6	; K6
	mov	esi, 13
	xor	ecx, ecx
	cmp	byte[test_id], 8
	jne	nr3
	mov	ecx, 0xFF0000
nr3:	int	0x40
	add	ebx, 27
	mov	edx, tmsg_k7	; K7
	mov	esi, 15
	xor	ecx, ecx
	cmp	byte[test_id], 9
	jne	nr4
	mov	ecx, 0xFF0000
nr4:	int	0x40
	xor	ecx, ecx
	mov	ebx, 190 * 65536 + 420
	mov	edx, tmsg_rec
	mov	esi, 11
	int	0x40
	; read about
	mov	ebx, 30 * 65536 + 282
	mov	edx, tmsg_war
	mov	esi, 35
	mov	ecx, 0xFF0000
	int	0x40
	; recommendate color
	mov	eax, 13
	mov	ebx, 170 * 65536 + 10
	mov	ecx, 418 * 65536 + 10
	mov	edx, atcol
	int	0x40

	jmp	redraw

tmsg_p	db	'Pentium (Pro/II/III)'	; 7/20
tmsg_k6	db	'K6 (I/II/III)'		; 13
tmsg_k7	db	'K7/Athlon/Duron'	; 15
tmsg_rec db	'recommended'		; 11
if lang eq it
	tmsg_war db	'Potrebbe danneggiare lo hardware   '
else
	tmsg_war db	'May permanently damage the hardware'
end if
