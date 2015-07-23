format MS COFF
use32

MUTEX.lock      equ 0
MUTEX.handle    equ 4

section '.text' align 16 code readable executable

public @mutex_init@4

@mutex_init@4:
        push    ebx
        xor     ebx, ebx
        mov     eax, 77
        mov     [ecx+MUTEX.lock], ebx
        int     0x40
        mov     [ecx+MUTEX.handle], eax
        pop     ebx
        ret
