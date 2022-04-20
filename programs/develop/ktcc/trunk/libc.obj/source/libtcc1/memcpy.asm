format ELF

section '.text' executable
include '../../../../../../proc32.inc'

public memcpy

proc memcpy c, to:dword,from:dword,count:dword
    push    esi
    push    edi
    mov     ecx, [count]
    test    ecx, ecx
    jz      .no_copy_block
    mov     esi, [from]
    mov     edi, [to]
    cld
    rep     movsb
.no_copy_block:
    pop     edi
    pop     esi
    mov     eax, [to]
    ret
endp
