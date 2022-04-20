format ELF

section '.text' executable
include '../../../../../../proc32.inc'

public memmove

proc memmove c, to:dword,from:dword,count:dword
    push    esi
    push    edi
    mov     ecx,[count]
    test    ecx,ecx
    jz      .no_copy_block
    mov     esi,[from]
    mov     edi,[to]
    cmp     esi, edi
    je      .no_copy_block
    jg      .copy
    add     esi, ecx
    add     edi, ecx
    dec     esi
    dec     edi
    std
.copy:
    rep     movsb
    cld
.no_copy_block:
    pop     edi
    pop     esi
    mov     eax,[to]
    ret
endp
