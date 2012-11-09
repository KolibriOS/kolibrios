;
; Kolibri Fast Calls test
;
;   Compile with FASM for Kolibri
;
;
use32
org	0x0
	db    'MENUET01'
	dd     0x01
	dd     START
	dd     I_END
	dd     0x1000
	dd     0x1000
	dd     0x0, 0x0

include 'macros.inc'
include '../../debug.inc'

START:	print	'Please wait'
	; через быстрый вызов (SYSENTER)
	__CPU_type	equ	p6
test1:	mov	eax, 1
	cpuid
	test	edx, 0x800
	jnz	.ok
	dps	'unsupported     '
	jmp	.end
.ok:	
	xor	eax, eax
	cpuid
	rdtsc
	mov	[old_tsc], eax
	mov	[old_tsc + 4], edx
	
	mov	ebx, 0x100000
	align	32
.nxt:  ;mcall   19              ; функция пустышка
				; портятся ecx, edx
	dec	ebx
	jnz	.nxt

	xor	eax, eax
	cpuid
	rdtsc
        sub     eax, [old_tsc]
        sbb     edx, [old_tsc + 4]
	debug_print_hex edx
	debug_print_hex eax
.end:	print	' <- Fast call (SYSENTER)'
	
	;----------------------------------------------
	; через быстрый вызов (SYSCALL)
	__CPU_type	equ	k6
test2:	xor	eax, eax
	cpuid
	cmp	ecx, "cAMD"
	je	.ok
.nf:	dps	'unsupported     '
	jmp	.end
.ok:	mov	eax, 0x80000001
	cpuid
	test	edx, 0x800  ; bit_11 - SYSCALL/SYSRET support
	jz	.nf
	
	xor	eax, eax
	cpuid
	rdtsc
	mov	[old_tsc], eax
	mov	[old_tsc + 4], edx
	
	mov	ebx, 0x100000
	align	32
.nxt:   ;mcall   19              ; функция пустышка
	
	dec	ebx
	jnz	.nxt

	xor	eax, eax
	cpuid
	rdtsc

        sub     eax, [old_tsc]
        sbb     edx, [old_tsc + 4]
	debug_print_hex edx
	debug_print_hex eax
.end:	print	' <- Fast call (SYSCALL)'
	;----------------------------------------------
	; через шлюз прерывани
	__CPU_type	equ	p5
	xor	eax, eax
	cpuid
	rdtsc
	mov	[old_tsc], eax
	mov	[old_tsc + 4], edx
	
test3:	mov	ebx, 0x100000
	align	32
.nxt:   ;mcall   19              ; функция пустышка
	dec	ebx
	jnz	.nxt
	
	xor	eax, eax
	cpuid
	rdtsc

        sub     eax, [old_tsc]
        sbb     edx, [old_tsc + 4]
	debug_print_hex edx
	debug_print_hex eax
	print	' <- Interrupt'

	mcall -1
;---------------------------------------------
old_tsc:	dd	0, 0
I_END:
