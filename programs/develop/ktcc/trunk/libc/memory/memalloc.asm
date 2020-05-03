format ELF

;include "../proc32.inc"
section '.text' executable
public sysmalloc
public sysfree
public sysrealloc

align 4
sysmalloc:
    push ebx
    push ecx
    mov eax,68
    mov ebx,12
    mov ecx,[esp+12] ;size
    int 0x40
    pop ecx
    pop ebx
    ret 4

align 4
sysfree:
    push ebx
    push ecx
    mov eax,68
    mov ebx,13
    mov ecx,[esp+12]
    int 0x40
    pop ecx
    pop ebx
    ret 4

align 4
sysrealloc:
    push ebx
    push ecx
    push edx
    mov eax,68
    mov ebx,20
    mov ecx,[esp+20]  ; size
    mov edx,[esp+16]  ; pointer
    int 0x40
    pop edx
    pop ecx
    pop ebx
    ret 8
