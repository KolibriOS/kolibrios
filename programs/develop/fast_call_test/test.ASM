;
; Kolibri Fast Calls test
;
;   Compile with FASM for Kolibri
;
;
SYSENTER_VAR	equ	0
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
include 'debug.inc'

START:	print	'Please wait'
	; через быстрый вызов (SYSENTER)
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
	mov	dword[SYSENTER_VAR], .ret_p
	mov	[SYSENTER_VAR + 4], esp
	align	32
.nxt:	mov	eax, 19 	; функция пустышка
	sysenter		; портятся ecx, edx
.ret_p: dec	ebx
	jnz	.nxt

	xor	eax, eax
	cpuid
	rdtsc
	cmp	eax, [old_tsc]
	jnb	@f
	dec	edx
@@:	sub	eax, [old_tsc]
	sub	edx, [old_tsc + 4]
	debug_print_hex edx
	debug_print_hex eax
.end:	print	' <- Fast call (SYSENTER)'
	
	;----------------------------------------------
	; через быстрый вызов (SYSCALL)
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
.nxt:	mov	eax, 19
	push	ecx
	syscall
	pop	ecx
	
.ret_p: dec	ebx
	jnz	.nxt

	xor	eax, eax
	cpuid
	rdtsc
	cmp	eax, [old_tsc]
	jnb	@f
	dec	edx
@@:	sub	eax, [old_tsc]
	sub	edx, [old_tsc + 4]
	debug_print_hex edx
	debug_print_hex eax
.end:	print	' <- Fast call (SYSCALL)'
	;----------------------------------------------
	; через шлюз прерывания
	xor	eax, eax
	cpuid
	rdtsc
	mov	[old_tsc], eax
	mov	[old_tsc + 4], edx
	
test3:	mov	ebx, 0x100000
	align	32
.nxt:	mov	eax, 19 	; функция пустышка
	int	0x40
	dec	ebx
	jnz	.nxt
	
	xor	eax, eax
	cpuid
	rdtsc
	cmp	eax, [old_tsc]
	jnb	@f
	dec	edx
@@:	sub	eax, [old_tsc]
	sub	edx, [old_tsc + 4]
	debug_print_hex edx
	debug_print_hex eax
	print	' <- Interrupt'

	call	show_alive
	mov	eax, -1
	int	0x40
;---------------------------------------------
show_alive:
	; через быстрый вызов, настраиваем регистры для возврата
	mov	eax, 63
	mov	ebx, 1
	mov	esi, msg_Ok
.nxt:	mov	cl, [esi]
	test	cl, cl
	jz	.end

	mov	dword[SYSENTER_VAR], .ret_p
	mov	[SYSENTER_VAR + 4], esp
	sysenter		; портятся ecx, edx

.ret_p: inc	esi
	jmp	.nxt
.end:	ret
	; через шлюз прерывания
	; mov   eax, 63
	; mov   ebx, 1
	; mov   esi, msg_Ok
; @@:   mov     cl, [esi]
	; test  cl, cl
	; jz    @f

	; int   0x40

	; inc   esi
	; jmp   @b
; @@:   ret


old_tsc:	dd	0, 0

msg_Ok	db	'Alive!', 10, 13, 0
I_END: