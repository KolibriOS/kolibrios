; Sorting bunch of dwords, count = ecx, locating at address = edx,
; comparison function at ebx
; Destroy content of eax, ecx, esi, edi
Sort:
        jecxz   .Done
        mov     eax,ecx
    @@:
        push    eax
        call    .Restore
        pop     eax
        dec     eax
        jnz     @B
    @@:
        cmp     ecx,1
        jz      .Done
        mov     esi,1
        mov     edi,ecx
        call    .Exchange
        dec     ecx
        mov     eax,1
        call    .Restore
        jmp     @B
    .Done:
        ret

    .Exchange:
        push    eax ecx
        mov     eax,[edx+esi*4-4]
        mov     ecx,[edx+edi*4-4]
        mov     [edx+esi*4-4],ecx
        mov     [edx+edi*4-4],eax
        pop     ecx eax
        ret

    .Restore:
        lea     esi,[eax+eax]
        cmp     esi,ecx
        ja      .DonErr
        push    esi
        mov     esi,[edx+esi*4-4]
        mov     edi,[edx+eax*4-4]
        call    ebx
        pop     esi
        ja      .NeedXchg
        cmp     esi,ecx
        jae     .DonErr
        push    esi
        mov     esi,[edx+esi*4]
        mov     edi,[edx+eax*4-4]
        call    ebx
        pop     esi
        jbe     .DonErr
    .NeedXchg:
        cmp     esi,ecx
        jz      .DoXchg
        push    esi
        mov     edi,[edx+esi*4-4]
        mov     esi,[edx+esi*4]
        call    ebx
        pop     esi
        sbb     esi,-1
    .DoXchg:
        mov     edi,eax
        call    .Exchange
        mov     eax,esi
        jmp     .Restore
    .DonErr:
        ret
