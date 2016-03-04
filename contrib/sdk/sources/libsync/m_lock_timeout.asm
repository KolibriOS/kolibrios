format MS COFF
use32

MUTEX.lock      equ 0
MUTEX.handle    equ 4

section '.text' align 16 code readable executable

@mutex_lock_timeout@8
        push    esi
        mov     esi, edx
        push    ebx
        mov     edx, 1
        lock xadd [ecx+MUTEX.lock], edx
        mov     eax, 1
        test    edx, edx
        jnz     .slow
.ok:
        pop     ebx
        pop     esi
        retn
align 4
.slow:
        mov     edx, 2
        xchg    edx, [ecx+MUTEX.lock]
        xor     eax, eax
        test    edx, edx
        jz      .ok

        mov     edx, 2
        mov     ecx, [ecx+MUTEX.handle]
        mov     al, 77
        mov     ebx, edx
        int     0x40
        add     eax, 1
        pop     ebx
        pop     esi
        ret






























