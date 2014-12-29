format COFF

;include "proc32.inc"
section '.text' code
public _malloc
public _free
public _realloc

align 4
_malloc:

        push ebx
        mov eax,68
        mov ebx,12
	mov ecx,[esp+8] ;size
        int 0x40
        pop ebx

        ret 4

align 4
_free:

        push ebx
        mov eax,68
        mov ebx,13
	mov ecx,[esp+8]
        int 0x40
        pop ebx

        ret 4

align 4
_realloc:

        push ebx
        mov ebx,20
        mov eax,68
	mov ecx,[esp+8]
	mov edx,[esp+12]
        int 0x40
        pop ebx

        ret 8
