PGAlloc:
; in: ecx=size
; out: eax=pointer or NULL
        push    ebx
        push    68
        pop     eax
        push    12
        pop     ebx
        int     40h
        pop     ebx
        ret
PGFree:
; in: ecx=pointer
; destroys eax
        push    ebx
        push    68
        pop     eax
        push    13
        pop     ebx
        int     40h
        pop     ebx
        ret
