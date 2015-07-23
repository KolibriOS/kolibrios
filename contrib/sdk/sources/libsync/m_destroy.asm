format MS COFF
use32

MUTEX.lock      equ 0
MUTEX.handle    equ 4

section '.text' align 16 code readable executable

public @mutex_destroy@4

@mutex_destroy@4:
        push    ebx
        mov     ecx, [ecx+MUTEX.handle]
        mov     eax, 77
        mov     ebx, 1
        int     0x40
        pop     ebx
        ret
