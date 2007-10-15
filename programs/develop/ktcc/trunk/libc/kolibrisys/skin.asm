format ELF

section '.text' executable

public _ksys_get_skin_height

_ksys_get_skin_height:

	mov eax,48
	mov ebx,4
	int 0x40

	ret