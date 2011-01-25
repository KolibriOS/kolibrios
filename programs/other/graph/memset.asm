format MS COFF
section '.text' code readable executable
public _memset
_memset:
	push	edi
	mov	edi, [esp+8]
	mov	al, [esp+12]
	mov	ecx, [esp+16]
	rep	stosb
	pop	edi
	ret
