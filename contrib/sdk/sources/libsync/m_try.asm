format MS COFF
use32

MUTEX.lock      equ 0
MUTEX.handle    equ 4

section '.text' align 16 code readable executable

public @mutex_trylock@4

@mutex_trylock@4:
        mov     edx, 1
        xor     eax, eax
        lock cmpxchg [ecx+MUTEX.lock], edx
        setz    al
        movzx   eax, al
        ret
