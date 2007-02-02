;
; Ghost Monitor - утилита (стресс) тестирования и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

;--------------------------------------------------------------------
; Загрузка процессора
; OUT : AL - загрузка в процентах
cpu_usage:
	pusha
	mov	byte [c_u_t], 0
	mov	eax, 18		; TSC / SEC
	mov	ebx, 5
	int	0x40
	shr	eax, 20
	push	eax

	mov	eax, 18		; IDLE / SEC
	mov	ebx, 4
	int	0x40		; eax - пустых циклов в секунду
	shr	eax, 20		; eax = IDLE / 1048576
	pop	ebx
	cmp	eax, ebx	; BUG : time to time ICPS > TSCPS
	jnl	c_u_o
	push	ebx
	xor	edx, edx
	mov	ebx, 100
	mul	ebx		; eax =(IDLE / 1048576) * 100

	xor	edx, edx
	pop	ebx		; ebx = (TSC/1048576)+1
	inc	ebx
	div	ebx		; eax = ((IDLE / 1048576) * 100) / ((TSC/1048576)+1)

	mov	bl, 100
	sub	bl, al
	cmp	bl, 101
	jnl	c_u_o
	mov	[c_u_t], bl

c_u_o:	popa
	mov	al, [c_u_t]
	ret
uglobal
	c_u_t:	db	0
endg
;--------------------------------------------------------------------
mem_usage:
; Испрользование памяти
; OUT : al - процетн испрользуемой памяти
display nl, 'Memory usage : '
if (OS_version >= 0x0530)
	display 'yes'
	pusha
	mov	byte [m_u_t], 0

	mov	eax, 18		; full mem
	mov	ebx, 17
	int	0x40
	shr	eax, 10
	push	eax

	mov	eax, 18		; free mem
	mov	ebx, 16
	int	0x40		; eax - free mem
	shr	eax, 10		; eax = fmem / 1024
	xor	edx, edx
	mov	ebx, 100
	mul	ebx		; eax =(free mem / 1024) * 100

	xor	edx, edx
	pop	ebx		; ebx = (full mem/1024)+1
	inc	ebx
	div	ebx		; eax = ((free mem / 1024) * 100) / ((full mem/1024)+1)

	mov	bl, 100
	sub	bl, al
	cmp	bl, 101
	jnb	m_u_o
	mov	[m_u_t], bl

m_u_o:	popa
	mov	al, [m_u_t]
	ret
uglobal
	m_u_t:	db	0
endg
else
	display 'no'
	xor	eax, eax
	ret
end if

;#######################################################################
stop_test:
	; портятся регистры
	display nl, 'Kill process type : '
if (OS_version < 0x0400)
	; код для Menuet
	display 'Menuet'
	mov	eax, 18
	mov	ebx, 2
	mov	ecx, [test_pid]
	int	0x40
	mov	byte[test_id], 0
	ret
	; -------------------------------
else if (OS_version >= 0x0400)
if (OS_version < 0x0580)
	display 'old Kolibri'
	; -- следуйший код убивает процесс в Kolibri 4 и старше
	mov	ebx, Buf
	xor	ecx, ecx
find_th:mov	eax, 9
	int	0x40
	mov	edx, dword [Buf + 30]
	cmp	edx, dword [test_pid]
	je	kill_t
	inc	ecx
	cmp	ecx, eax		; eax - количество процессов
	jle	find_th
	jmp	cl_tinf			; ОШИБКА !!!! поток не найден !!!!
kill_t:	mov	ebx, 2			; убиваем тестовый поток
	mov	eax, 18
	int	0x40
cl_tinf:
	mov	byte[test_id], 0
	ret
	uglobal
	Buf:		times	1024 db ?	; Буфер для нахождения PID`а запущенного теста
	endg
else
	; -------------------------------
	; код для Kolibri 0.5.8.0 и старше
	display 'Kolibri 0.5.8.0'
	mov	eax, 18
	mov	ebx, 18
	mov	ecx, [test_pid]
	int	0x40
	mov	byte [test_id], 0
	ret
	; -------------------------------
end if
end if
;#######################################################################

; Выводит строки текста
; in : edx - указатель на mls
;      ebx - координаты : X << 16 + Y
show_text:
	mov	eax, 4
	mov	ecx, 0x10000000
@@:
	movzx	esi, byte[edx]
	inc	edx
	int	0x40
	add	ebx, 10
	add	edx, esi
	cmp	byte[edx], -1
	jne	@b
	ret
;---------------------------------------------------------------------	
multiplier:
;--- вычисление коэффициента умножения -
; портит регистры
; out : CL = коэфф.умножения * 10, или 0
	xor	ecx, ecx
if (OS_version >= 0x0510)
	display nl, 'Multiplier (RDMSR) : yes'
	cmp	dword [Vendor + 8], 'cAMD'	; Check for Advanced Micro Devices CPU
	jne	noAMD
	cmp	byte [CPU_fam], 5
	jne	noAMDK6
	mov	eax, 68		; AMD-K6 (p.30)
	mov	ebx, 3
	mov	edx, 0x0C0000087
	int	0x40		; eax - low dword
	and	eax, 111b
	mov	cl, [athloncoef3 + eax]
	cmp	eax, 6
	jne	@f
	cmp	byte[CPU_mod], 8
	jae	@f
	mov	cl, 20
@@:	ret	

noAMDK6:cmp	byte [CPU_fam], 6
	jne	noAMDK7
	cmp	byte [CPU_mod], 5
	jna	@f
	mov	eax, 0x80000007
	cpuid
	and	edx, 6		;  voltage ID control & frequency ID control
	cmp	edx, 6
	je	AMDK7M
@@:	mov	eax, 68		; Athlon/AthlonXP
	mov	ebx, 3
	mov	edx, 0x0C0010015
	int	0x40
	mov	ebx, eax
	shr	ebx, 24
	and	ebx, 0x0F
	shr	eax, 20
	jnc	@f
	add	bl, 16
@@:	mov	cl, [athloncoef + ebx]
	ret

AMDK7M:	mov	eax, 68		; AthonXP-M
	mov	ebx, 3
	mov	edx, 0xC0010042
	int	0x40
	and	eax, 0x1F
	mov	cl, [athlonmcoef + eax]
	ret

noAMDK7:cmp	byte [CPU_fam], 0xF
	jne	noAMDK8
	mov	eax, 0x80000007
	cpuid
	and	edx, 6		;  voltage ID control & frequency ID control
	cmp	edx, 6
	je	AMDK8M
	mov	eax, 68		; Athon64
	mov	ebx, 3
	mov	edx, 0xC0010015
	int	0x40
	shr	eax, 24
	and	al, 0x3F
	shr	al, 1
	add	al, 4
	mov	dl, 10
	mul	dl
	mov	cl, al
	ret

AMDK8M:	mov	eax, 68		; Athon64-M
	mov	ebx, 3
	mov	edx, 0xC0010042
	int	0x40
	and	al, 0x3F
	shr	al, 1
	add	al, 4
	mov	dl, 10
	mul	dl
	mov	cl, al
	ret
	
noAMD:	cmp	dword [Vendor + 8], 'ntel'	; Check for International Electronics CPU
	jne	noIntel
	cmp	byte[CPU_fam], 0x0F
	jne	noIntelP4
	cmp	byte [CPU_type], 6
	jne	@f
	mov	eax, 68		; Pentium M
	mov	ebx, 3
	mov	edx, 0x2A
	int	0x40
	shr	eax, 22
	and	eax, 0x1F
	mov	dl, 10
	mul	dl
	mov	cl, al
	ret
@@:	cmp	byte [CPU_mod], 2
	jae	@f
	mov	eax, 68		; Pentium 4 / Xeon (model < 2) Willamete
	mov	ebx, 3
	mov	edx, 0x2A
	int	0x40
	shr	eax, 8
	and	eax, 0x0F
	mov	cl, [p4coef + eax]
	ret
@@:	mov	eax, 68		; Pentium 4 / Xeon (model >= 2) NorthWood
	mov	ebx, 3
	mov	edx, 0x2C
	int	0x40
	shr	eax, 24
	and	eax, 0x1F
	mov	dl, 10
	mul	dl
	mov	cl, al
	ret	
noIntelP4:
	cmp	byte[CPU_fam], 6
	jne	noIntelP6
	mov	eax, 68		; Pentium Pro / Pentium II / Pentium III
	mov	ebx, 3
	mov	edx, 0x2A
	int	0x40
	shr	eax, 22
	test	al, 0x20
	jz	@f
	or	al, 0x10
@@:	and	eax, 0x1f

	cmp	byte[CPU_mod], 0x06 ; ? 7
	ja	@f
	and	al, 0x0f
@@:
	mov	cl, [coppercoeff + eax]
	cmp	byte[CPU_mod], 0x0B
	jb	@f
	mov	cl, [tualatcoeff + eax]
	
	cmp	byte[CPU_mod], 0x0B
	je	@f
	mov	dl, 10		; model 0x0C - 0x0F - Dothan / Yonah  / Conroe / Merom
	mul	dl
	mov	cl, al
	
@@:	ret

noIntel:
noIntelP6:
noAMDK8:
	ret

athloncoef	db	110, 115, 120, 125, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100, 105, 120
		db	190, 120, 200, 130, 135, 140, 210, 150, 220, 160, 165, 170, 180, 230, 240
athlonmcoef:	db	110, 115, 120, 125, 50, 55, 60, 65,  70, 75, 80, 85, 90, 95, 100, 105
		db	30, 190, 40, 200, 130, 135, 14, 210, 150, 220, 160, 165, 170, 230, 240
athloncoef3	db	45, 50, 40, 55, 25, 30, 60, 35
p4coef		db	160, 170, 180, 190, 200, 210, 220, 230, 80, 90, 100, 110, 120, 130, 140, 150	; Pentium 4 (Willamete)
coppercoeff	db	 50, 30, 40, 20, 55, 35,  45, 25,  35, 70, 80, 60, 20, 75, 15, 65, 90, 110, 120, 20, 95, 115, 85, 25, 35, 70,  80, 100,  20, 75,  15, 105
tualatcoeff	db	120, 35, 35, 40, 55, 35, 115, 35, 160, 70, 80, 60, 40, 75, 35, 65, 90, 110,  35, 35, 95,  35, 85, 35, 35, 35, 130, 100, 140, 35, 150, 105
else
	display nl, 'Multiplier : no'
	ret
end if
;---------------------------------------------------------------------
digit_len:
	; Вычисление длинны числа для функ. 47
	; ecx - число
	; ebx - длинна * 65536
	pusha
	xor	ebx, ebx
	mov	eax, ecx
	mov	esi, 10
@@:	xor	edx, edx
	inc	ebx
	div	esi
	test	eax, eax
	jnz	@b
	mov	[tdl], ebx
	popa
	mov	ebx, [tdl]
	shl	ebx, 16
	ret
uglobal
	tdl	dd	0
endg


;--------------------------------------------------------------------
ftoa:
; esi - преобразуемое число;
; Data_String - строка-результат.

	pusha
	mov     edi, Data_String
	fninit
	fld	dword [esi]
	fmul	dword [MConst]
	fbstp	[Data_BCD]
	mov	ax, word[Data_BCD + 8]
	cmp	ax, 0xFFFF
	je      @@Overflow
	mov	al, byte[Data_BCD + 9]
	and	al, al
	jz	@@NoSign
	mov	AL,'-'
	stosb
@@NoSign:
	mov	ebx, 8
	mov	ecx, 9
	mov     edx, 18 - 2	; 2 знака после запятой
@@NextPair:
	mov     al, byte[Data_BCD + ebx]
	mov     AH,AL
	shr     AL,4
	add     AL,'0'
	stosb
	dec     edx
	jnz     @@N0
	mov     AL,'.'
	stosb
@@N0:	mov     AL,AH
	and     AL,0Fh
	add     AL,'0'
	stosb
	dec     edx
	jnz     @@N1
	mov     AL,'.'
	stosb
@@N1:	dec     ebx
	loop    @@NextPair
	xor	al, al
	stosb
	mov     edi, Data_String
	mov     esi, Data_String
	cmp     byte[esi], '-'
	jne     @@N2
	inc     esi
	inc     edi
@@N2:	mov     ecx, 18+1+1
@@N3:   cmp	byte[esi], '0'
	jne     @@N4
	inc     esi
	loop    @@N3
	jmp	@@Error
@@N4:   rep movsb
	jmp	@@End
@@Overflow:
@@Error:mov	eax, 'ERR'
	stosd
@@End:	popa
	ret
MConst: dd 1.0E2	; 2 знака после запятой

uglobal
	Data_BCD:	DT ?
	Data_String:	times 20 db 0	; ASCIIZ сторка с преобразованным числом
endg
;--------------------------------------------------------------------
ReservePorts:
	; In  : ecx - first port, edx - last port
	; Out : CF = 1 if error
	pusha
	mov	eax, 46
	xor	ebx, ebx
	int	0x40
	clc
	test	eax, eax
	jz	@f
	stc
@@:	popa
	ret
	
FreePorts:
	; In  : ecx - first port, edx - last port
	pusha
	mov	eax, 46		; освобождаем 0x295 и 0x296 порты
	xor	ebx, ebx
	inc	ebx
	int	0x40
	popa
	ret
