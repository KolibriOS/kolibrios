;-----------------------------------------------------------------------------+
; ?㭪??? ??????樨 ???砩???? ??᫠ ? ????????? 0..99999                     |
;-----------------------------------------------------------------------------+
; ?? ??室?  : eax - ???砩??? ??᫮                                          |
;-----------------------------------------------------------------------------+
_random:
    push    edx ecx ebx
    mov     eax,ebx
    or      eax,eax
    jnz     @f
    rdtsc
    xor     eax,edx
    mov     ebx,eax
@@:
    xor     edx,edx
    mov     ecx,127773
    div     ecx
    mov     ecx,eax
    mov     eax,16807
    mul     edx
    mov     edx,ecx
    mov     ecx,eax
    mov     eax,2836
    mul     edx
    sub     ecx,eax
    xor     edx,edx
    mov     eax,ecx
    mov     ebx,ecx
    mov     ecx,100000
    div     ecx
    mov     eax,edx
    pop     ebx ecx edx
    ret
;->
