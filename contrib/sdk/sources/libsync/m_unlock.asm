format MS COFF
use32

MUTEX.lock      equ 0
MUTEX.handle    equ 4

section '.text' align 16 code readable executable

public @mutex_unlock@4

@mutex_unlock@4:
        xor     eax, eax
        xchg    eax, [ecx]
        cmp     eax, 1
        jnz     .wake

        ret
.wake:
        push    ebx
        mov     edx, 1
        mov     ecx, [ecx+MUTEX.handle]
        mov     ebx, 3
        mov     eax, 77
        int     0x40
        pop     ebx
        retn
