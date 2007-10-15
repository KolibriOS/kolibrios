format ELF

;include "proc32.inc"
section '.text' executable
public malloc
public free
public realloc

align 4
malloc:

        mov eax,68
        mov ebx,12
	mov ecx,[esp+4] ;size
        int 0x40

        ret 4

align 4
free:

        mov eax,68
        mov ebx,13
	mov ecx,[esp+4]
        int 0x40

        ret 4

align 4
realloc:

        mov ebx,20
        mov eax,68
	mov ecx,[esp+4]
	mov edx,[esp+8]
        int 0x40

        ret 8
