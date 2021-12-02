;-----------------------------------------------------------------------------
TokenEnd       = 1
TokenReg       = 2
TokenHex       = 3
TokenOr        = 4
TokenAnd       = 5
TokenXor       = 6
TokenAdd       = 7
TokenSub       = 8
TokenMul       = 9
TokenDiv       = 10
TokenLP        = 11
TokenRP        = 12
TokenErr       = -1
;-----------------------------------------------------------------------------
RegTable:
        db 2,'al',0
        db 2,'cl',1
        db 2,'dl',2
        db 2,'bl',3
        db 2,'ah',4
        db 2,'ch',5
        db 2,'dh',6
        db 2,'bh',7
        db 2,'ax',8
        db 2,'cx',9
        db 2,'dx',10
        db 2,'bx',11
        db 2,'sp',12
        db 2,'bp',13
        db 2,'si',14
        db 2,'di',15
        db 3,'eax',16
        db 3,'ecx',17
        db 3,'edx',18
        db 3,'ebx',19
        db 3,'esp',20
        db 3,'ebp',21
        db 3,'esi',22
        db 3,'edi',23
        db 3,'eip',24
        db 0
;-----------------------------------------------------------------------------
;                   Check if byte is hex digit
IsHexDigit:
        cmp     al,'0'
        jb      .No
        cmp     al,'9'
        jbe     .09
        cmp     al,'A'
        jb      .No
        cmp     al,'F'
        jbe     .AF
        cmp     al,'a'
        jb      .No
        cmp     al,'f'
        jbe     .FA
    .No:
        stc
        ret
    .09:
        sub     al,'0'
;       clc
        ret
    .AF:
        sub     al,'A'-10
;       clc
        ret
    .FA:
        sub     al,'a'-10
;       clc
        ret

;-----------------------------------------------------------------------------
;                      Find register in the table
FindReg:
        mov     edi,RegTable
    .FindReg:
        movzx   ecx,byte [edi]
        stc
        jecxz   .RegNotFound
        inc     edi
        push    esi edi ecx
    @@:
        lodsb
        or      al,20h
        scasb
        loopz   @B
        pop     ecx edi esi
        lea     edi,[edi+ecx+1]
        jnz     .FindReg
        movzx   edi,byte [edi-1]
        add     esi,ecx
    .RegNotFound:
        ret

;-----------------------------------------------------------------------------
;                      Tokenize expressions
ExprGetToken:
        lodsb
        cmp     al,0
        jz      .EndToken
        cmp     al,' '
        jbe     ExprGetToken
        cmp     al,'|'
        jz      .Or
        cmp     al,'&'
        jz      .And
        cmp     al,'^'
        jz      .Xor
        cmp     al,'+'
        jz      .Add
        cmp     al,'-'
        jz      .Sub
        cmp     al,'*'
        jz      .Mul
        cmp     al,'/'
        jz      .Div
        cmp     al,'('
        jz      .LP
        cmp     al,')'
        jnz     .NotSign
    .RP:
        mov     al,TokenRP
        ret
    .Or:
        mov     al,TokenOr
        ret
    .And:
        mov     al,TokenAnd
        ret
    .Xor:
        mov     al,TokenXor
        ret
    .Div:
        mov     al,TokenDiv
        ret
    .EndToken:
        mov     al,TokenEnd
        ret
    .Add:
        mov     al,TokenAdd
        ret
    .Sub:
        mov     al,TokenSub
        ret
    .Mul:
        mov     al,TokenMul
        ret
    .LP:
        mov     al,TokenLP
        ret
    .NotSign:
        dec     esi
        call    FindReg
        jc      .RegNotFound
        mov     al,TokenReg
        ret
    .RegNotFound:
    ; test for symbol
        push    esi
    @@:
        lodsb
        cmp     al,' '
        ja      @B
        push    eax
        mov     byte [esi],0
        xchg    esi,[esp+4]
        call    FindSymbolName
        mov     edi,eax
        pop     eax
        xchg    esi,[esp]
        mov     byte [esi],al
        jc      @F
        add     esp,4
        mov     al,TokenHex
        ret
    @@:
        pop     esi
    ; test for hex number
        xor     ecx,ecx
        xor     edi,edi
        xor     eax,eax
    @@:
        lodsb
        call    IsHexDigit
        jc      @F
        shl     edi,4
        or      edi,eax
        inc     ecx
        jmp     @B
    @@:
        dec     esi
        jecxz   .Err
        cmp     ecx,8
        ja      .Err
        mov     al,TokenHex
        ret
    .Err:
        mov     al,TokenErr
        mov     esi,aParseError
        ret

;-----------------------------------------------------------------------------
ExprRead2:
        cmp     al,TokenHex
        jz      .Hex
        cmp     al,TokenReg
        jz      .Reg
        cmp     al,TokenLP
        jz      .LP
        mov     al,TokenErr
        mov     esi,aParseError
        ret
    .Hex:
        mov     ebp,edi
    .Ret:
        jmp     ExprGetToken
    .Reg:
        cmp     edi,24
        jz      .EIP
        sub     edi,4
        jb      .8LO
        sub     edi,4
        jb      .8HI
        sub     edi,8
        jb      .16
        mov     ebp,[_EAX+edi*4]
        jmp     .Ret
    .16:
        movzx   ebp,word [_EAX+(edi+8)*4]
        jmp     .Ret
    .8LO:
        movzx   ebp,byte [_EAX+(edi+4)*4]
        jmp     .Ret
    .8HI:
        movzx   ebp,byte [_EAX+(edi+4)*4+1]
        jmp     .Ret
    .EIP:
        mov     ebp,[_EIP]
        jmp     .Ret
    .LP:
        call    ExprGetToken
        call    ExprRead0
        cmp     al,TokenErr
        jz      @F
        cmp     al,TokenRP
        jz      ExprGetToken
        mov     al,TokenErr
        mov     esi,aParseError
    @@:
        ret

;-----------------------------------------------------------------------------
ExprRead1:
        call    ExprRead2
    .1:
        cmp     al,TokenMul
        jz      .Mul
        cmp     al,TokenDiv
        jz      .Div
        ret
    .Mul:
        push    ebp
        call    ExprGetToken
        call    ExprRead2
        pop     edx
    ; ebp := edx*ebp
        imul    ebp,edx
        jmp     .1
    .Div:
        push    ebp
        call    ExprGetToken
        call    ExprRead2
        pop     edx
    ; ebp := edx/ebp
        test    ebp,ebp
        jz      .Div0
        push    eax
        xor     eax,eax
        xchg    eax,edx
        div     ebp
        xchg    eax,ebp
        pop     eax
        jmp     .1
    .Div0:
        mov     al,TokenErr
        mov     esi,aDivByZero
        ret

;-----------------------------------------------------------------------------
ExprRead0:
        xor     ebp,ebp
        cmp     al,TokenOr
        jz      .Or
        cmp     al,TokenAnd
        jz      .And
        cmp     al,TokenXor
        jz      .Xor
        cmp     al,TokenAdd
        jz      .Add
        cmp     al,TokenSub
        jz      .Sub
        call    ExprRead1
    .1:
        cmp     al,TokenOr
        jz      .Or
        cmp     al,TokenAnd
        jz      .And
        cmp     al,TokenXor
        jz      .Xor
        cmp     al,TokenAdd
        jz      .Add
        cmp     al,TokenSub
        jz      .Sub
        ret
    .Or:
        push    ebp
        call    ExprGetToken
        call    ExprRead1
        pop     edx
    ; ebp := edx | ebp
        or      ebp,edx
        jmp     .1
    .And:
        push    ebp
        call    ExprGetToken
        call    ExprRead1
        pop     edx
    ; ebp := edx & ebp
        and     ebp,edx
        jmp     .1
    .Xor:
        push    ebp
        call    ExprGetToken
        call    ExprRead1
        pop     edx
    ; ebp := edx ^ ebp
        xor     ebp,edx
        jmp     .1
    .Add:
        push    ebp
        call    ExprGetToken
        call    ExprRead1
        pop     edx
    ; ebp := edx+ebp
        add     ebp,edx
        jmp     .1
    .Sub:
        push    ebp
        call    ExprGetToken
        call    ExprRead1
        pop     edx
    ; ebp := edx-ebp
        xchg    edx,ebp
        sub     ebp,edx
        jmp     .1

;-----------------------------------------------------------------------------
; in: esi->expression
; out: CF=1 if error
;      CF=0 and ebp=value if ok
CalcExpression:
        call    ExprGetToken
        call    ExprRead0
        cmp     al,TokenEnd
        jz      .End
        cmp     al,TokenErr
        jz      @F
        mov     esi,aParseError
    @@:
        call    PutMessage
        stc
        ret
    .End:
        clc
        ret

GetArg:
        lodsb
        cmp     al,' '
        ja      GetArg
        mov     byte [esi-1],0
        cmp     al,0
        jnz     .SkipSpaces
        dec     esi

    .SkipSpaces:
        lodsb
        cmp     al,0
        jz      @F
        cmp     al,' '
        jbe     .SkipSpaces
    @@:
        dec     esi
        ret
