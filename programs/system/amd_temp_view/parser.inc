pars_cmd:
        mov     edi, PATH
        mov     ecx, 512
.still:
        mov     al, ' '
        repz    scasb

        test    ecx, ecx
        jz      .end_parser

        dec     edi
        mov     ebx, edi  ;save edi
        cmp     byte[edi],'"'
        jnz     @f ;.skip_string
        mov     eax, [save_file]
        cmp     eax, [.flag_drv]
        jae     .str_save

        cmp     dword[drv_ptr], 0
        jnz     .err
        mov     [drv_ptr],edi
        inc     dword[drv_ptr]
        jmp     .skip_string
.str_save:
        cmp     dword[save_ptr], 0
        jnz     .err
        mov     [save_ptr],edi
        inc     dword[save_ptr]
        jmp     .skip_string
@@:

        cmp     word[edi], 's-'
        jnz     @f
        cmp     dword[save_file], 0
        jnz     @f
        mov     [save_file], edi
@@:
        cmp     word[edi], 'd-'
        jnz     @f
        cmp     dword[.flag_drv], 0
        jnz     @f
        mov     [.flag_drv], edi
        add     edi, 2
@@:
        cmp     edi, ebx
        jz      .err
        jmp     .still

.skip_string:
        inc     edi
        mov     al, '"'
        rep     scasb
        test    ecx, ecx
        jz      .err
        mov     byte[edi-1], 0x00
        jmp     .still
.err:
        or      eax, -1
        ret
.end_parser:
        xor     eax, eax
        ret
.flag_drv:      dd 0;don`t using in other code