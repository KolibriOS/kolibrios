; --------------------------------------------------------------------------
; FILE: TCmdBuf.Asm
; DATE: September 27, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Input:
;   ESI = ANSI token scanned
;   CL = ANSI text to match (1-based index into message table)
; Output:
;   CF=TRUE if first token exactly matches text at index in CL
;   CF=FALSE if no match (message given to the user)
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_VerifyToken:
    movzx   ecx, cl

TCmdBuf_VerifyToken_ECX:
    push    esi
    call    TMsgTable_GetItem
    pop     edi

    call    TString_AnsiEqual
    jnc     .ret_false
    ret

.ret_false:
    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 93
    call    TConsole_Prout
    call    TConsole_ScrollUp

    clc
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = ANSI token scanned
;   ECX = ANSI text to match (1-based index into message table)
; Output:
;   CF=TRUE if text at EDI has a prefix like ESI
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_Crop:
    push    esi esi
    call    TMsgTable_GetItem
    mov     edi, esi
    pop     esi

    mcZeroBits edx
    mcZeroBits eax

.load_chars:
    lodsb
    mov     dl, [edi]
    inc     edi

    cmp     dl, al
    je      .check_zero

    mcOnRegZero eax, .match

    pop     esi
    clc
    ret

.check_zero:
    mcOnRegNotZero eax, .load_chars

.match:
    pop     esi
    stc
    ret

; --------------------------------------------------------------------------
; Input:
;   AX = WCHAR to append
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_PutChar:
    mov     ebx, [glb_pCmdBuf]
    cmp     [ebx + TCmdBuf.cmdbuf_CmdColumn], CMDBUF_MAX_CHARS
    je      .done

    mcLoadMember ecx, TCmdBuf.cmdbuf_CmdColumn
    mov     [ebx + ecx*2], ax
    inc     [ebx + TCmdBuf.cmdbuf_CmdColumn]

.done:
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_Parse:
    mcZeroBits eax
    call    TCmdBuf_PutChar

    mov     ebx, [glb_pCmdBuf]
    mcStoreMember TCmdBuf.cmdbuf_NumTokens, eax
    mcStoreMember TCmdBuf.cmdbuf_CmdColumn, eax

    dec     eax
    mcStoreMember TCmdBuf.cmdbuf_TokenIterator, eax
    inc     eax

    mov     esi, ebx
    mov     edx, eax
    mcLoadMemberRef edi, TCmdBuf.cmdbuf_CmdTokens

.load_char:
    lodsw
    mcOnRegZero eax, .done

    cmp     al, CHAR_BLANK
    je      .separator

    mcOnRegNotZero edx, .load_char

    lea     edx, [esi - 2]
    mov     [edi], edx
    add     edi, 4
    inc     [ebx + TCmdBuf.cmdbuf_NumTokens]
    cmp     [ebx + TCmdBuf.cmdbuf_NumTokens], CMDBUF_MAX_TOKENS
    je      .done
    jmp     .load_char

.separator:
    mcZeroBits edx
    mov     [esi - 2], dx
    jmp     .load_char

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = character
; Output:
;   CF = TRUE if AL is a digit
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_IsDigit:
    cmp     al, '0'
    jb      .false

    cmp     al, '9'
    ja      .false

    stc
    ret

.false:
    clc
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = token address
; Output:
;   CF = TRUE if token is a real value: "-100" or "5.6"
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_IsRealToken:
    mcZeroBits ecx
    mcZeroBits eax
    mcZeroBits edi

.load_char:
    mov     ax, [esi + ecx*2]
    inc     ecx
    mcOnRegZero eax, .true

    mcOnRegEqu al, '-', .minus
    mcOnRegEqu al, '.', .period
    ;
    ; Check if digit
    ;
    call    TCmdBuf_IsDigit
    jc      .load_char

.false:
    clc
    ret

.minus:
    mcOnRegEqu ecx, 1, .load_char
    jmp     .false

.period:
    mcOnRegNotZero edi, .false
    mov     edi, esi
    jmp     .load_char

.true:
    stc
    ret

; --------------------------------------------------------------------------
; Output:
;   ESI = token address or NULL
;   EAX = token type (CMD_TOKEN_ALPHA, CMD_TOKEN_REAL, CMD_TOKEN_EOL)
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_GetNextToken:
    mov     ebx, [glb_pCmdBuf]
    inc     [ebx + TCmdBuf.cmdbuf_TokenIterator]

    mcLoadMember ecx, TCmdBuf.cmdbuf_TokenIterator
    cmp     ecx, [ebx + TCmdBuf.cmdbuf_NumTokens]
    jae     .end_of_line

    mov     esi, [ebx + ecx*4 + TCmdBuf.cmdbuf_CmdTokens]
    call    TCmdBuf_IsRealToken
    jc      .real_value

    mcLoad8bitsToReg32 eax, CMD_TOKEN_ALPHA
    ret

.real_value:
    mcLoad8bitsToReg32 eax, CMD_TOKEN_REAL
    ret

.end_of_line:
    mcZeroBits eax
    mcZeroBits esi
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = ANSI text
;   AL = character to find
; Output:
;   ECX = character offset (CF=TRUE)
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_AnsiFindChar:
    mcZeroBits ecx

.test_char:
    cmp     byte [esi + ecx], 0
    je      .not_found

    cmp     [esi + ecx], al
    je      .found

    inc     ecx
    jmp     .test_char

.found:
    stc
    ret

.not_found:
    clc
    ret

; --------------------------------------------------------------------------
; Output:
;   EBX = TCmdBuf instance
;   ESI = ANSI token scanned
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_Scan:
    call    TCmdBuf_GetNextToken
    mcStoreMember TCmdBuf.cmdbuf_KEY, al

    mcZeroBits edx
    mcStoreMember TCmdBuf.cmdbuf_INUM, edx
    mcStoreMember TCmdBuf.cmdbuf_AnsiTokenBuf, dl

    fldz
    fstp    [ebx + TCmdBuf.cmdbuf_FNUM]

    mcOnRegZero esi, .done
    ;
    ; Convert the token into ANSI
    ;
    mcLoadMemberRef edi, TCmdBuf.cmdbuf_AnsiTokenBuf
    mov     eax, edx

.convert_char:
    lodsw
    stosb
    mcOnRegNotZero eax, .convert_char

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_ALPHA
    je      .done

    mcLoadMemberRef esi, TCmdBuf.cmdbuf_AnsiTokenBuf
    mov     al, '.'
    call    TCmdBuf_AnsiFindChar
    jc      .set_fp_num
    ;
    ; Integer value scanned
    ;
    call    TCmdBuf_AnsiStr2Int32
    mcStoreMember TCmdBuf.cmdbuf_INUM, eax

    fild    [ebx + TCmdBuf.cmdbuf_INUM]
    fstp    [ebx + TCmdBuf.cmdbuf_FNUM]
    jmp     .done

.set_fp_num:
    ;
    ; FP value scanned
    ;
    mcLoadMemberRef edi, TCmdBuf.cmdbuf_FNUM
    call    TCmdBuf_AnsiStr2Double

.done:
    mcLoadMemberRef esi, TCmdBuf.cmdbuf_AnsiTokenBuf
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = ANSI text
; Output:
;   ECX = number of characters in the string
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_AnsiStrLen:
    push    edx

    mcZeroBits ecx
    mcZeroBits edx

.check_char:
    cmp     [esi + ecx], dl
    je      .done

    inc     ecx
    jmp     .check_char

.done:
    pop     edx
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = ANSI text
; Output:
;   EAX = converted value
; --------------------------------------------------------------------------
virtual at 0
loc5:
    .bNegative BOOL ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_AnsiStr2Int32:
    mcBeginLocals loc5.size

    mcZeroBits eax
    mcZeroBits edx
    mcStoreLocal loc5.bNegative, eax

    cmp     byte [esi], '-'
    jne     .convert

    inc     [esp + loc5.bNegative]
    inc     esi

.convert:
    mov     dl, [esi]
    mcOnRegZero edx, .apply_sign

    sub     dl, '0'
    inc     esi
    imul    eax, 10
    add     eax, edx
    jmp     .convert

.apply_sign:
    cmp     byte [esp + loc5.bNegative], 0
    je      .done

    neg     eax

.done:
    mcEndLocals loc5.size
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = text
;   EDI = pointer to DOUBLE value
; --------------------------------------------------------------------------
virtual at 0
loc6:
    .pszDot PCHAR ?
    .pszValue PCHAR ?
    .bNegative BOOL ?
    .iPart1 INT32 ?
    .iPart2 INT32 ?
    .pDblResult PDOUBLE ?
    .dblPart1 DOUBLE ?
    .dblPart2 DOUBLE ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_AnsiStr2Double:
    mcBeginLocals loc6.size
    mcStoreLocal loc6.pszValue, esi
    mcStoreLocal loc6.pDblResult, edi

    mcZeroBits eax
    mcStoreLocal loc6.bNegative, eax
    mcStoreLocal loc6.pszDot, eax

    cmp     byte [esi], '-'
    jne     .parse

    inc     [esp + loc6.bNegative]
    inc     [esp + loc6.pszValue]
    inc     esi

.parse:
    lodsb
    mcOnRegZero eax, .analysis
    mcOnRegEqu al, '.', .dot
    jmp     .parse

.dot:
    lea     edx, [esi - 1]
    mcStoreLocal loc6.pszDot, edx
    jmp     .parse

.analysis:
    cmp     byte [esp + loc6.pszDot], 0
    jne     .combine_parts
    ;
    ; No decimal point
    ;
    mcLoadLocal esi, loc6.pszValue
    call    TCmdBuf_AnsiStr2Int32
    mcStoreLocal loc6.iPart1, eax

    fild    [esp + loc6.iPart1]
    jmp     .apply_sign

.combine_parts:
    mcLoadLocal edi, loc6.pszDot
    mcZeroBits al
    stosb

    fldz
    fldz
    fstp    [esp + loc6.dblPart1]
    fstp    [esp + loc6.dblPart2]

    mov     esi, edi
    call    TCmdBuf_AnsiStr2Int32
    mcStoreLocal loc6.iPart2, eax

    mcLoadLocal esi, loc6.pszValue
    call    TCmdBuf_AnsiStr2Int32
    mcStoreLocal loc6.iPart1, eax

    mcLoadLocal esi, loc6.pszDot
    inc     esi
    call    TCmdBuf_AnsiStrLen
    jecxz   .combine

    fild    [esp + loc6.iPart2]
@@:
    fld     [glb_dbl_Ten]
    fdivp
    loop    @r

    fstp    [esp + loc6.dblPart2]

.combine:
    fild    [esp + loc6.iPart1]
    fld     [esp + loc6.dblPart2]
    faddp

.apply_sign:
    cmp     byte [esp + loc6.bNegative], 0
    je      .done

    fchs

.done:
    mcLoadLocal edi, loc6.pDblResult
    fstp    tbyte [edi]

    mcEndLocals loc6.size
    ret

; --------------------------------------------------------------------------
; Output:
;   CF = TRUE if buffer has no characters
; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_IsEmpty:
    mov     ebx, [glb_pCmdBuf]
    cmp     [ebx + TCmdBuf.cmdbuf_CmdColumn], 0
    sete    cl
    shr     cl, 1
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_Backspace:
    mov     ebx, [glb_pCmdBuf]
    cmp     [ebx + TCmdBuf.cmdbuf_CmdColumn], 0
    je      .done

    dec     [ebx + TCmdBuf.cmdbuf_CmdColumn]

.done:
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TCmdBuf_Create:
    invoke  HeapAlloc, [glb_Allocator], HEAP_NO_SERIALIZE, TCmdBuf.size
    mov     [glb_pCmdBuf], eax

    mov     edi, eax
    mcZeroBits eax
    mov     ecx, TCmdBuf.size
    rep     stosb
    ret

; --- EOF ---
