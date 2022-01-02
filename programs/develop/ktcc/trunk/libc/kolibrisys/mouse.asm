
format ELF

section '.text' executable

public _ksys_GetMouseXY
public _ksys_GetMouseButtonsState

align 4
_ksys_GetMouseXY:

	mov eax,37
	mov ebx,1
	int 0x40

	ret

align 4
_ksys_GetMouseButtonsState:

	mov eax,37
	mov ebx,2
	int 0x40

	ret
