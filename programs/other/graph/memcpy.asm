format MS COFF
section '.text' code readable executable
public _memcpy
_memcpy:
	push	esi edi
	mov	edi, [esp+12]
	mov	esi, [esp+16]
	mov	ecx, [esp+20]
	rep	movsb
	pop	edi esi
	ret
