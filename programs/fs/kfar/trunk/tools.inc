xmalloc:
; in: eax=size
; out: eax=pointer or NULL
        call    mf_alloc
.common:
        test    eax, eax
        jnz     @f
        call    SayNoMem
        xor     eax, eax
@@:
        ret

xrealloc:
; in: eax=pointer, ebx=new size
; out: eax=pointer or NULL
        call    mf_realloc
        jmp     xmalloc.common

get_error_msg:
; in: eax=error code
; out: eax=pointer to message (in static buffer)
        push    esi edi
        mov     edi, error_msg
        cmp     eax, 11
        ja      .no1
        mov     esi, [errors1+eax*4]
        jmp     .copy
.no1:
        cmp     eax, 30
        jb      .no2
        cmp     eax, 32
        ja      .no2
        mov     esi, [errors2+(eax-30)*4]
.copy:
        lodsb
        stosb
        test    al, al
        jnz     .copy
.ret:
        mov     eax, error_msg
        pop     edi esi
        ret
.no2:
        mov     esi, aUnknownError
        push    eax
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     eax
        push    edx ecx
        test    eax, eax
        jns     @f
        mov     byte [edi], '-'
        inc     edi
        neg     eax
@@:
        xor     edx, edx
        mov     ecx, 10
        div     ecx
        add     edx, '0'
        mov     byte [edi], dl
        inc     edi
        test    eax, eax
        jnz     @b
        pop     ecx edx
        stosb
        jmp     .ret
