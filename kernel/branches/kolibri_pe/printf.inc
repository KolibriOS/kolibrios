
DWORD equ dword
WORD  equ word
BYTE  equ byte

PTR   equ

_putc:
	mov	ecx, DWORD PTR [esp+4]
.L13:
	mov	edx, 1021
	in al, dx
	test	al, 96
        je      .L13
	mov	dl, -8
	mov	eax, ecx
	out dx, al
	cmp	ecx, 10
        jne     .L7
	mov	cl, 13
        jmp     .L13
.L7:
        ret

_printf:
	push	ebp
	push	edi
	push	esi
	push	ebx
	sub	esp, 40
	lea	eax, [esp+64]
	mov	DWORD PTR [esp+4], eax
.L50:
	mov	ecx, DWORD PTR [esp+60]
	movzx	edx, BYTE PTR [ecx]
	test	dl, dl
        je      .L51
.L17:
	lea	eax, [ecx+1]
	cmp	dl, 37
	mov	DWORD PTR [esp+60], eax
        je      .L18
	movsx	eax, dl
	mov	DWORD PTR [esp], eax
	call	_putc
	mov	ecx, DWORD PTR [esp+60]
	movzx	edx, BYTE PTR [ecx]
	test	dl, dl
        jne     .L17
.L51:
	add	esp, 40
	pop	ebx
	pop	esi
	pop	edi
	pop	ebp
	ret
.L18:
	movzx	edx, BYTE PTR [ecx+1]
	lea	eax, [ecx+2]
	mov	DWORD PTR [esp+60], eax
	cmp	dl, 115
        je      .L22
        jg      .L23
	cmp	dl, 100
        je      .L21
.L20:
	mov	edx, DWORD PTR [esp+4]
	mov	eax, DWORD PTR [edx]
	add	edx, 4
	mov	DWORD PTR [esp+4], edx
	mov	DWORD PTR [esp], eax
	call	_putc
        jmp     .L50
.L23:
	cmp	dl, 117
        je      .L21
	cmp	dl, 120
        jne     .L20
.L21:
	mov	ecx, DWORD PTR [esp+4]
	mov	eax, DWORD PTR [ecx]
	add	ecx, 4
	cmp	dl, 100
	mov	DWORD PTR [esp+4], ecx
	mov	ecx, eax
        je      .L52
.L24:
	cmp	dl, 120
        je      .L53
	lea	ebp, [esp+20]
	mov	esi, 10
	mov	ebx, ebp
	mov	edi, ebp
.L43:
	xor	edx, edx
	mov	eax, ecx
	div	esi
	cmp	edx, 9
	lea	eax, [edx+48]
        jle     .L31
	lea	eax, [edx+87]
.L31:
	mov	BYTE PTR [edi], al
	xor	edx, edx
	mov	eax, ecx
	div	esi
	add	edi, 1
	test	eax, eax
	mov	ecx, eax
        jne     .L43
	lea	eax, [edi-1]
	cmp	eax, ebx
	mov	BYTE PTR [edi], 0
        jbe     .L33
	lea	ecx, [edi-2]
.L35:
	movzx	eax, BYTE PTR [ebx]
	movzx	edx, BYTE PTR [ecx+1]
	mov	BYTE PTR [ebx], dl
	add	ebx, 1
	mov	BYTE PTR [ecx+1], al
	mov	eax, ecx
	sub	ecx, 1
	cmp	ebx, eax
        jb      .L35
.L33:
	mov	edx, ebp
.L36:
	movzx	eax, BYTE PTR [edx]
	test	al, al
        je      .L50
	mov	ebx, edx
.L39:
	movsx	eax, al
	mov	DWORD PTR [esp], eax
	call	_putc
	movzx	eax, BYTE PTR [ebx+1]
	add	ebx, 1
	test	al, al
        jne     .L39
        jmp     .L50
.L22:
	mov	eax, DWORD PTR [esp+4]
	mov	edx, DWORD PTR [eax]
	add	eax, 4
	mov	DWORD PTR [esp+4], eax
	test	edx, edx
        jne     .L36
        mov     edx, .LC0
        jmp     .L36
.L52:
	test	eax, eax
        jns     .L24
	lea	ebx, [esp+21]
	neg	ecx
	mov	edi, ebx
	mov	esi, 10
	lea	ebp, [esp+20]
	mov	BYTE PTR [esp+20], 45
        jmp     .L43
.L53:
	lea	ebp, [esp+20]
	mov	esi, 16
	mov	ebx, ebp
	mov	edi, ebp
        jmp     .L43

.LC0:   db "(null)",0

restore DWORD
restore WORD
restore BYTE

restore PTR
