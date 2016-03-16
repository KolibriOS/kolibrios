;
; Ghost Monitor - утилита (стресс) тестирования и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved
;
; k6, k7, p5 & p6 tests based on Robert J. Redelmeier cpuburn-1.4
; Copyright 1999, 2000  Robert J. Redelmeier.  All Right Reserved 
; Licensed under GNU General Public Licence 2.0.  No warrantee. 

;  adapted by Mihailov Ilia
;
; Заметки :
;	переписть реакцию на ошибки

k6_:
	finit
	push	ebp
	mov	ebp, esp
	and	ebp, -32
	sub	esp, 96
	fldpi
	fld	qword [rtt]
	fstp	qword [ebp - 24]
	fld	qword [e]
	fstp	qword [ebp - 32]
	mov	edx, [half]
	mov	[ebp - 8], edx
k6_after_check:
	xor	eax, eax
	mov	ebx, eax
	lea	esi, [eax - 1]
	mov	ecx, 400000000
	mov	[ebp - 4], ecx
align	32
	;rev. eng. win. ver. (lea esi, [esi]) ; Ghost
k6_crunch:				
	fld	qword [ebp+esi*8-16]		; CALC BLOCK
	fmul	qword [ebp+esi*8-24]
	add	edx, [half + 9 + esi + esi * 8]	;!!!
	jnz	$ + 2
	faddp	
	fld	qword [ebp+esi*8-16]
	dec	ebx
	sub	edx, [half + 9 + esi + esi * 8]	;!!!
	jmp	$ + 2
	fmul	qword [ebp+esi*8-24]
	inc	ebx
	dec	dword [ebp+esi*8+4]
	fsubp	
	jnz	k6_crunch			; time for testing ?

	test	ebx, ebx			; TEST BLOCK
	jnz	k6_int_exit
	cmp	edx, [half]
	jnz	k6_int_exit
	fldpi
	fcomp	st1
	fstsw	ax
	sahf
	jz	k6_after_check
	dec	ebx
k6_int_exit:
	dec	ebx
	add	esp, 96
	pop	ebp
	mov	eax, -1
	int	0x40
;#######################################################################
;#######################################################################
k7_:	finit
	mov	ebp, esp
	and	ebp,-32
	sub	esp,96
	fld	qword [rtt]
	fstp	qword [ebp-40]
	fld	qword [e]
	fstp	qword [ebp-32]
	fldpi
	fldpi
	xor	eax, eax
	xor	ebx, ebx
	xor	ecx, ecx
	mov	edx, half
	lea	esi,[eax-1] 
	mov	[ebp-12], eax 
	mov	[ebp-8], edx

k7_after_check:
	mov	dword[ebp-4], 0x32A9F880;0x850000000	;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
ALIGN 16
k7_crunch:
	fxch					; CALC BLOCK
	fld	qword [ebp+esi*8-24]		; 17 instr / 6.0 cycles
	fmul	qword [ebp+esi*8-32]
	faddp
	dec	ecx
	fld	qword [ebp+esi*8-24]
	dec	ebx
	inc	dword [ebp+esi*8-12]
	inc	ecx
	fmul	qword [ebp+esi*8-32]
	inc	ebx
	dec	dword [ebp+esi*8-4]
	jmp     $+2
	fsubp    st2, st0
	jnz	k7_crunch			; time for testing ?
	test	ebx, ebx			; TEST BLOCK
	jnz	k7_int_exit
	test	ecx, ecx
	jnz	k7_int_exit
	cmp	edx, half
	jnz	k7_int_exit
	fcom	st1
	fstsw	ax
	sahf
	jz	k7_after_check
	dec	ebx
k7_int_exit:
	dec	ebx
	add	esp,96
	mov	eax, -1
	int	0x40
;#######################################################################
;#######################################################################
p5:	finit
	push	ebp
	mov	ebp, esp
	and	ebp, -32
	sub	esp, 96
	fld	qword [half]
	fstp	qword [ebp - 24]
	fld	qword [rtt]
	fst	qword [ebp - 16]
	fld	st
	fld	st
p5_after_check:
	xor	eax, eax
	mov	eax, ebx
	mov	ecx, 200000000
align	32
				;# MAIN LOOP  16 flops / 18 cycles
p5_crunch:	   
	fmul	qword [ebp - 24]
	fxch	st1
	fadd	qword [ebp - 16]
	fxch	st2
	fmul	qword [ebp - 24]
	fxch	st1
	fadd	qword [ebp - 16]
	fxch	st2

	fmul	qword [ebp - 24]
	fxch	st1
	fadd	qword [ebp - 16]
	fxch	st2
	fmul	qword [ebp - 24]
	fxch	st1
	fadd	qword [ebp - 16]
	fxch	st2
 
	fmul	qword [ebp - 24]
	fxch	st1
	fadd	qword [ebp - 16]
	fxch	st2
	fmul	qword [ebp - 24]
	fxch	st1
	fadd	qword [ebp - 16]
	fxch	st2

	fmul	qword [ebp - 24]
	fxch	st1
	fadd	qword [ebp - 16]
	fxch	st2
	fmul	qword [ebp - 24]
	fxch	st1
	fadd	qword [ebp - 16]
	fxch	st2
 
	dec	ecx
	jnz	p5_crunch

	jmp	p5_after_check
;#######################################################################
;#######################################################################	
p6:	finit
	push	ebp
	mov	ebp, esp
	and	ebp, -32
	sub	esp, 96
	fldpi
	fld	qword [rtt]
	fstp	qword [ebp - 24]
	fld	qword [e]
	fstp	qword [ebp - 32]
	mov	edx, [half]
	mov	[ebp - 8], edx
p6_after_check:
	xor	eax, eax
	mov	ebx, eax
	lea	esi, [eax - 1]
	mov	ecx, 539000000		;# check after this count  
	mov	[ebp - 4], ecx
align	32
p6_crunch:				;# MAIN LOOP	21uops / 8.0 clocks
	fld	qword [ebp+esi*8-16]
	fmul	qword [ebp+esi*8-24]
	add	edx, [half]
	jnz	$ + 2
	faddp	
	fld	qword [ebp - 24]
	dec	ebx
	sub	edx, [half + 9 + esi + esi*8]
	jmp	$ + 2
	fmul	qword [ebp+esi*8-24]
	inc	ebx
	dec	dword [ebp+esi*8+4]
	fsubp	
	jnz	p6_crunch		

	test	ebx, ebx		;# Testing block
	mov	ebx, 0
	jnz	p6_int_exit
	cmp	edx, [half]
	jnz	p6_int_exit
	fldpi
	fcomp	st1
	fstsw	ax
	sahf
	jz	p6_after_check		;# fp result = pi ?
	dec	ebx
p6_int_exit:				;# error abort
	dec	ebx
	add	esp, 96
	pop	ebp
	mov	eax, -1
	int	0x40
;---------------------------------------------
align	32
	half	dd	0x7fffffff, 0
	e	dd	0xffffffff, 0x3fdfffff
	rtt	dd	0xffffffff, 0x3fefffff
	