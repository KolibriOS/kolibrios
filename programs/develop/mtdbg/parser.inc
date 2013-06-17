;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;; EXPRESSION PARSER ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


token_end       equ     1
token_reg       equ     2
token_hex       equ     3
token_add       equ     4
token_sub       equ     5
token_mul       equ     6
token_div       equ     7
token_lp        equ     8
token_rp        equ     9
token_err       equ     -1


;-----------------------------------------------------------------------------
;               Check if byte - some kind of instruction prefix

is_prefix:
        cmp     al, 0x64        ; fs:
        jz      .ret
        cmp     al, 0x65        ; gs:
        jz      .ret
        cmp     al, 0x66        ; use16/32
        jz      .ret
        cmp     al, 0x67        ; addr16/32
        jz      .ret
        cmp     al, 0xF0        ; lock
        jz      .ret
        cmp     al, 0xF2        ; repnz
        jz      .ret
        cmp     al, 0xF3        ; rep(z)
        jz      .ret
        cmp     al, 0x2E        ; cs:
        jz      .ret
        cmp     al, 0x36        ; ss:
        jz      .ret
        cmp     al, 0x3E        ; ds:
        jz      .ret
        cmp     al, 0x26        ; es:
   
    .ret:
        ret

;-----------------------------------------------------------------------------
;                   Check if byte is hex digit

is_hex_digit:
        cmp     al, '0'
        jb      .no
        cmp     al, '9'
        jbe     .09
        cmp     al, 'A'
        jb      .no
        cmp     al, 'F'
        jbe     .AF
        cmp     al, 'a'
        jb      .no
        cmp     al, 'f'
        jbe     .af

    .no:
        stc
        ret

    .09:
        sub     al, '0'
;       clc
        ret

    .AF:
        sub     al, 'A'-10
;       clc
        ret

    .af:
        sub     al, 'a'-10
;       clc
        ret

;-----------------------------------------------------------------------------
;                      Find register in the table

find_reg:
        mov     edi, reg_table
    
    .findreg:
        movzx   ecx, byte [edi]
        stc
        jecxz   .regnotfound
        inc     edi
        push    esi edi ecx
    
    @@:
        lodsb
        or      al, 20h
        scasb
        loopz   @b
        pop     ecx edi esi
        lea     edi, [edi+ecx+1]
        jnz     .findreg
        movzx   edi, byte [edi-1]
        add     esi, ecx

    .regnotfound:
        ret

;-----------------------------------------------------------------------------
;                      Tokenize expressions

expr_get_token:
        lodsb
        cmp     al, 0
        jz      .end_token
        cmp     al, ' '
        jbe     expr_get_token
        cmp     al, '+'
        jz      .add
        cmp     al, '-'
        jz      .sub
        cmp     al, '*'
        jz      .mul
        cmp     al, '/'
        jz      .div
        cmp     al, '('
        jz      .lp
        cmp     al, ')'
        jnz     .notsign
    
    .rp:
        mov     al, token_rp
        ret
    
    .div:
        mov     al, token_div
        ret

    .end_token:
        mov     al, token_end
        ret
    
    .add:
        mov     al, token_add
        ret

    .sub:
        mov     al, token_sub
        ret

    .mul:
        mov     al, token_mul
        ret

    .lp:
        mov     al, token_lp
        ret

    .notsign:
        dec     esi
        call    find_reg
        jc      .regnotfound
        mov     al, token_reg
        ret

    .regnotfound:
    ; test for symbol
        push    esi

    @@:
        lodsb
        cmp     al, ' '
        ja      @b
        push    eax
        mov     byte [esi], 0
        xchg    esi, [esp+4]
        call    find_symbol_name
        mov     edi, eax
        pop     eax
        xchg    esi, [esp]
        mov     byte [esi], al
        jc      @f
        add     esp, 4
        mov     al, token_hex
        ret

    @@:
        pop     esi
    ; test for hex number
        xor     ecx, ecx
        xor     edi, edi
        xor     eax, eax

    @@:
        lodsb
        call    is_hex_digit
        jc      @f
        shl     edi, 4
        or      edi, eax
        inc     ecx
        jmp     @b

    @@:
        dec     esi
        jecxz   .err
        cmp     ecx, 8
        ja      .err
        mov     al, token_hex
        ret

    .err:
        mov     al, token_err
        mov     esi, aParseError
        ret

;-----------------------------------------------------------------------------

expr_read2:
        cmp     al, token_hex
        jz      .hex
        cmp     al, token_reg
        jz      .reg
        cmp     al, token_lp
        jz      .lp
        mov     al, token_err
        mov     esi, aParseError
        ret

    .hex:
        mov     ebp, edi

    .ret:
        jmp     expr_get_token

    .reg:
        cmp     edi, 24
        jz      .eip
        sub     edi, 4
        jb      .8lo
        sub     edi, 4
        jb      .8hi
        sub     edi, 8
        jb      .16
        mov     ebp, [_eax+edi*4]
        jmp     .ret

    .16:
        movzx   ebp, word [_eax+(edi+8)*4]
        jmp     .ret

    .8lo:
        movzx   ebp, byte [_eax+(edi+4)*4]
        jmp     .ret

    .8hi:
        movzx   ebp, byte [_eax+(edi+4)*4+1]
        jmp     .ret

    .eip:
        mov     ebp, [_eip]
        jmp     .ret

    .lp:
        call    expr_get_token
        call    expr_read0
        cmp     al, token_err
        jz      @f
        cmp     al, token_rp
        jz      expr_get_token
        mov     al, token_err
        mov     esi, aParseError

    @@:
        ret

;-----------------------------------------------------------------------------

expr_read1:
        call    expr_read2

    .1:
        cmp     al, token_mul
        jz      .mul
        cmp     al, token_div
        jz      .div
        ret

    .mul:
        push    ebp
        call    expr_get_token
        call    expr_read2
        pop     edx
    ; ebp := edx*ebp
        imul    ebp, edx
        jmp     .1

    .div:
        push    ebp
        call    expr_get_token
        call    expr_read2
        pop     edx
    ; ebp := edx/ebp
        test    ebp, ebp
        jz      .div0
        push    eax
        xor     eax, eax
        xchg    eax, edx
        div     ebp
        xchg    eax, ebp
        pop     eax
        jmp     .1

    .div0:
        mov     al, token_err
        mov     esi, aDivByZero
        ret

;-----------------------------------------------------------------------------

expr_read0:
        xor     ebp, ebp
        cmp     al, token_add
        jz      .add
        cmp     al, token_sub
        jz      .sub
        call    expr_read1

    .1:
        cmp     al, token_add
        jz      .add
        cmp     al, token_sub
        jz      .sub
        ret

    .add:
        push    ebp
        call    expr_get_token
        call    expr_read1
        pop     edx
    ; ebp := edx+ebp
        add     ebp, edx
        jmp     .1

    .sub:
        push    ebp
        call    expr_get_token
        call    expr_read1
        pop     edx
    ; ebp := edx-ebp
        xchg    edx, ebp
        sub     ebp, edx
        jmp     .1

;-----------------------------------------------------------------------------

; in: esi->expression
; out: CF=1 if error
;      CF=0 and ebp=value if ok
calc_expression:
        call    expr_get_token
        call    expr_read0
        cmp     al, token_end
        jz      .end
        cmp     al, token_err
        jz      @f
        mov     esi, aParseError

    @@:
        call    put_message
        stc
        ret

    .end:
        clc
        ret

;-----------------------------------------------------------------------------

get_arg:
        lodsb
        cmp     al, ' '
        ja      get_arg
        mov     byte [esi-1], 0
        cmp     al, 0
        jnz     .skip_spaces
        dec     esi

    .skip_spaces:
        lodsb
        cmp     al, 0
        jz      @f
        cmp     al, ' '
        jbe     .skip_spaces

    @@:
        dec     esi
        ret



; vim: ft=fasm tabstop=4

