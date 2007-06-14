; Сортировка dword'ов в количестве ecx по адресу edx, функция сравнения в ebx
; Разрушает eax, ecx, esi, edi
sort:
        jecxz   .done
        mov     eax, ecx
@@:
        push    eax
        call    .restore
        pop     eax
        dec     eax
        jnz     @b
@@:
        cmp     ecx, 1
        jz      .done
        mov     esi, 1
        mov     edi, ecx
        call    .exchange
        dec     ecx
        mov     eax, 1
        call    .restore
        jmp     @b
.done:
        ret

.exchange:
        push    eax ecx
        mov     eax, [edx+esi*4-4]
        mov     ecx, [edx+edi*4-4]
        mov     [edx+esi*4-4], ecx
        mov     [edx+edi*4-4], eax
        pop     ecx eax
        ret

.restore:
        lea     esi, [eax+eax]
        cmp     esi, ecx
        ja      .doner
        push    esi
        mov     esi, [edx+esi*4-4]
        mov     edi, [edx+eax*4-4]
        call    ebx
        pop     esi
        ja      .need_xchg
        cmp     esi, ecx
        jae     .doner
        push    esi
        mov     esi, [edx+esi*4]
        mov     edi, [edx+eax*4-4]
        call    ebx
        pop     esi
        jbe     .doner
.need_xchg:
        cmp     esi, ecx
        jz      .do_xchg
        push    esi
        mov     edi, [edx+esi*4-4]
        mov     esi, [edx+esi*4]
        call    ebx
        pop     esi
        sbb     esi, -1
.do_xchg:
        mov     edi, eax
        call    .exchange
        mov     eax, esi
        jmp     .restore
.doner:
        ret
