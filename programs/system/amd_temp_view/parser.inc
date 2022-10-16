; AMDtemp -s/sd0/4/data/temp/log.grf -d"/sd0/4/data/датчик температуры com порт/sensors.sys"
parse_cmd:
        mov     edi, PATH
        mov     ecx, 512
.still:
        mov     al, ' '
        repz    scasb

        test    ecx, ecx
        jz      .end_parser

        dec     edi
        or      word[edi], 0x2020 ; переводим в нижний регистр

        cmp     word[edi], '-s'
        jnz     @f

        add     edi, 2
        sub     ecx, 2
        call    .get_str
        mov     [save_ptr],eax
        jmp     .still
@@:
        cmp     word[edi], '-d'
        jnz     @f

        add     edi, 2
        sub     ecx, 2
        call    .get_str
        mov     [drv_ptr],eax
        jmp     .still
@@:
        cmp     byte[edi], 0
        jnz     @f
.end_parser:
        ret
@@:
        inc     edi
        jmp     .still

.get_str:
        push    edi
        inc     dword[esp]
        mov     al, '"'
        cmp     byte[edi], al
        jz      @f
        dec     dword[esp]
        mov     al, ' '
        dec     edi
@@:
        inc     edi
        repne   scasb
        and     byte[edi - 1], 0
        pop     eax
        ret