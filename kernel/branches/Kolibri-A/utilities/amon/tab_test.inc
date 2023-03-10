;
; Ghost Monitor - ??????? (??????) ???????????? ? ??????????? ???????
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

;
;
;
; ??????? ??????
;
;
;
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
	; MMX ????
	; add	ecx, 27 * 65536
	; inc	edx
	; mov	esi, tcol
	; cmp	byte[CPU_mmx], 1
	; jne	mmxn
	; mov	esi, atcol
; mmxn:	int	0x40
	; ????? ???????? ??????
	mov	eax, 4
	mov	ebx, 30 * 65536 + 307
	mov	esi, 7
	mov	edx, tmsg_p	; P5
	mov	ecx, 0x10000000
	cmp	byte[test_id], 6
	jne	nr1
	mov	ecx, 0x10FF0000
nr1:	int	0x40
	mov	ebx, 30 * 65536 + 307 + 27
	mov	esi, 20		; P6
	mov	ecx, 0x10000000
	cmp	byte[test_id], 7
	jne	nr2
	mov	ecx, 0x10FF0000
nr2:	int	0x40
	mov	ebx, 30 * 65536 + 307 + 2 * 27
	mov	edx, tmsg_k6	; K6
	mov	esi, 13
	mov	ecx, 0x10000000
	cmp	byte[test_id], 8
	jne	nr3
	mov	ecx, 0x10FF0000
nr3:	int	0x40
	mov	ebx, 30 * 65536 + 307 + 3 * 27
	mov	edx, tmsg_k7	; K7
	mov	esi, 15
	mov	ecx, 0x10000000
	cmp	byte[test_id], 9
	jne	nr4
	mov	ecx, 0x10FF0000
nr4:	int	0x40
	; mov	ebx, 30 * 65536 + 307 + 4 * 27
	; mov	edx, tmsg_mmx	; MMX
	; mov	esi, 12
	; mov	ecx, 0x10000000
	; cmp	byte[test_id], 10
	; jne	nr5
	; mov	ecx, 0x10FF0000
; nr5:	int	0x40
	; recommendate
	mov	ecx, 0x10000000
	mov	ebx, 190 * 65536 + 420
	mov	edx, tmsg_rec
	mov	esi, 14
	int	0x40
	; read about
	mov	ebx, 30 * 65536 + 282
	mov	edx, tmsg_war
	mov	esi, 34
	mov	ecx, 0x10FF0000
	int	0x40
	; recommendate color
	mov	eax, 13
	mov	ebx, 170 * 65536 + 10
	mov	ecx, 418 * 65536 + 10
	mov	edx, atcol
	int	0x40
	
	
	jmp	end_drow_tab
	
tmsg_p	db	'Pentium (Pro/II/III)'	; 7/20
tmsg_k6	db	'K6 (I/II/III)'		; 13
tmsg_k7	db	'K7/Athlon/Duron'	; 15
;tmsg_mmx db	'MMX (memory)'		; 12
tmsg_rec db	'--------------'	; 14
tmsg_war db	'May permanent damage your system' ; 34