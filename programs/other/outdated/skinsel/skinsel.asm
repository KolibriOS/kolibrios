include '..\..\..\macros.inc'
use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      mem, mem
        dd      0, 0

start:
        push    70
        pop     eax
        mov     ebx, fileinfo
        mcall
        cmp     ebx, max_file_size
        jz      exit
        cmp     ebx, -1
        jnz     @f
exit:
        or      eax, -1
        mcall
@@:
        mov     esi, filebuf
        xor     ecx, ecx
scan1l1:
        or      edi, -1
scan1:
        inc     edi
        lodsb
        test    al, al
        jz      scan1done
        cmp     al, 13
        jz      newline
        cmp     al, 10
        jnz     scan1
newline:
        test    edi, edi
        jz      scan1l1
        inc     ecx
        jmp     scan1l1
scan1done:
        test    edi, edi
        jz      @f
        inc     ecx
@@:
        jecxz   exit
        push    3
        pop     eax
        mcall
        ror     eax, 16
        mov     edx, 1024
@@:
        sub     eax, 0x43ab45b5
        ror     eax, 1
        xor     eax, 0x32c4324f
        ror     eax, 1
        dec     edx
        jnz     @b
        div     ecx
; use edx as random index
        mov     esi, filebuf
scan2l1:
        push    esi
        or      edi, -1
scan2:
        inc     edi
        lodsb
        test    al, al
        jz      newline2
        cmp     al, 13
        jz      newline2
        cmp     al, 10
        jnz     scan2
newline2:
        pop     eax
        test    edi, edi
        jz      scan2l1
        dec     edx
        jns     scan2l1
        mov     byte [esi-1], 0
; set eax (-> ASCIIZ string) as skin
        mov     ecx, eax
        push    48
        pop     eax
        mov     bl, 8
        mcall
        jmp     exit

fileinfo:
        dd      0
        dq      0
        dd      max_file_size
        dd      filebuf
        db      '/sys/skinsel.dat',0
i_end:
max_file_size = 0x1000 - 0x40 - $
filebuf:
        rb      max_file_size
; stack
        rb      0x40
mem:
