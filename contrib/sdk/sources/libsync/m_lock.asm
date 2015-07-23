format MS COFF
use32

MUTEX.lock      equ 0
MUTEX.handle    equ 4

section '.text' align 16 code readable executable

public @mutex_lock@4

@mutex_lock@4:

        mov     eax, 1
        lock xadd [ecx+MUTEX.lock], eax
        test    eax, eax
        jnz     .slow
        ret
.slow:
        push    ebx
        push    esi
        push    edi
        mov     edi, ecx
        mov     ecx, [edi+MUTEX.handle]
        mov     edx, 2
        mov     ebx, edx
        xor     esi, esi
align 4
.again:
        mov     eax, edx
        xchg    eax, [edi+MUTEX.lock]
        test    eax, eax
        jz      .ok

        mov     eax, 77
        int     0x40
        jmp     .again
.ok:
        pop     edi
        pop     esi
        pop     ebx
        ret
