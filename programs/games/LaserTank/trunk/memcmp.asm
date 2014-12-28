format MS COFF
section '.text' code readable executable
public _memcmp
_memcmp:
	push	esi edi
	mov	esi, [esp+12]
	mov	edi, [esp+16]
	mov	ecx, [esp+20]
	repz	cmpsb
	pop	edi esi
	setb	ah
	seta	al
	sub	al, ah
	movsx	eax, al
	ret
