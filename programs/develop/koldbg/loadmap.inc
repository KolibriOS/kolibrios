uglobal
LastNames       dq      ?
ShowNames       db      ?
endg

FindEAX:push    esi
        and     [LastNames],0
        cmp     [ShowNames],0
        jnz     .NoSymb
        cmp     eax,0x80
        jl      .NoSymb
        lea     esi,[eax-1]
        test    eax,esi
        jz      .NoSymb
        call    FindSymbol
        jc      .NoSymb
        mov     [LastNames],esi
        cmp     rdi,[TheBufferO]
        jz      .SetCol                 ;Skip starting address
        push    edi
    @@:
        lodsb
        test    al,al
        jz      @F
        stosb
        jmp     @B
    .SetCol:
        mov     [BufferO-1],byte 31h    ;Label Color
    .NoSymb:
        stc
        pop     esi
        ret
    @@: mov     ecx,edi
        sub     ecx,[esp]
        cmp     ecx,255
ifa     mov     ecx,255
        pop     edi
        add     edi,ecx
        clc
        pop     esi
        ret

include "symbols.inc"
