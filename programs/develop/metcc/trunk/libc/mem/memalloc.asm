format ELF

include "proc32.inc"
section '.text' executable
public malloc
public free
public realloc

align 4
proc malloc stdcall, size:dword

        mov eax,68
        mov ebx,12
	mov ecx,[size]
        int 0x40

        ret 
endp

align 4
proc free stdcall, pointer:dword

        mov eax,68
        mov ebx,13
	mov ecx,[pointer]
        int 0x40

        ret 
endp

align 4
proc realloc stdcall, pointer:dword, size:dword

        mov ebx,20
        mov eax,68
	mov ecx,[size]
	mov edx,[pointer]
        int 0x40

        ret
endp
