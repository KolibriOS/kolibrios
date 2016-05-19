format ELF

;include "proc32.inc"
section '.text' executable
public malloc
public free
public realloc

align 4
malloc:
    push ebx
        mov eax,68
        mov ebx,12
	mov ecx,[esp+8] ;size
        int 0x40
    pop ebx
        ret 4

align 4
free:
    push ebx
        mov eax,68
        mov ebx,13
	mov ecx,[esp+8]
        int 0x40
    pop ebx
        ret 4

align 4
realloc:
    push ebx
        mov ebx,20
        mov eax,68
	mov edx,[esp+8]  ; pointer
	mov ecx,[esp+12]  ; size
        int 0x40
    pop ebx
        ret 8
