; --------------------------------------------------------------------------
; FILE: TFormat.Asm
; DATE: October 7, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Input:
;   EDI = ANSI buffer
;   EAX = value to format
; Output:
;   ECX = length of text
; NOTE:
;   All registers are safe
; --------------------------------------------------------------------------
virtual at 0
loc18:
    .buffer CHARS 12
    .nChars INT32 ?
    .bSigned BOOL ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TFormat_Int32:
    push    edx eax esi edi
    mcBeginLocals loc18.size

    mcLoadLocalRef esi, loc18.nChars
    mcLoad8bitsToReg32 ecx, 10
    mcZeroBits edx
    mcStoreLocal loc18.nChars, edx
    mcStoreLocal loc18.bSigned, edx

    add     eax, edx
    jns     .div_10

    neg     eax
    inc     [esp + loc18.bSigned]

.div_10:
    mcZeroBits edx
    div     ecx
    add     dl, '0'
    dec     esi
    mov     [esi], dl
    inc     [esp + loc18.nChars]
    mcOnRegNotZero eax, .div_10

    cmp     [esp + loc18.bSigned], 0
    je      .copy

    dec     esi
    mov     byte [esi], '-'
    inc     [esp + loc18.nChars]

.copy:
    mcLoadLocal ecx, loc18.nChars
    push    ecx
    rep     movsb

    stosb
    pop     ecx

    mcEndLocals loc18.size
    pop     edi esi eax edx
    ret

; --------------------------------------------------------------------------
; Input:
;   ST(0) = value to format
;   EDI = ANSI buffer
;   CL = number of digits after dot
; Output:
;   ECX = length of text
; --------------------------------------------------------------------------
virtual at 0
loc17:
    .dbl_Value DOUBLE ?
    .buf_AllDigits CHARS 10
    .nValue INT32 ?
    .nDigits INT32 ?
    .pszBuffer PCHAR ?
    .bSigned BOOL ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TFormat_Double:
    push    eax ebx esi edi
    mcBeginLocals loc17.size

.prepare:
    mcStoreLocal loc17.pszBuffer, edi
    movzx   ecx, cl
    mcStoreLocal loc17.nDigits, ecx

    fld1

.make_factor:
    fld     [glb_dbl_Ten]
    fmulp
    loop    .make_factor

    fmulp
    frndint

    call    TCommon_FP_Truncate
    mcStoreLocal loc17.nValue, eax

    mcLoadLocalRef edi, loc17.buf_AllDigits
    call    TFormat_Int32

    cmp     ecx, [esp + loc17.nDigits]
    ja      .print_all

    mcLoadLocal ebx, loc17.pszBuffer
    mov     byte [ebx], '0'
    mov     byte [ebx + 1], '.'
    je      .copy_all_at_EBX

    mov     al, [edi]
    mov     byte [ebx + 2], '0'
    mov     byte [ebx + 3], al
    mov     byte [ebx + 4], 0
    jmp     .done

.copy_all_at_EBX:
    mov     esi, edi
    lea     edi, [ebx + 2]
    inc     ecx
    rep     movsb
    jmp     .done

.print_all:
    mov     esi, edi
    mcLoadLocal edi, loc17.pszBuffer
    sub     ecx, [esp + loc17.nDigits]
    rep     movsb

    mov     al, '.'
    stosb

    mov     ecx, [esp + loc17.nDigits]
    inc     ecx
    rep     movsb

.done:
    mcLoadLocal esi, loc17.pszBuffer
    call    TCmdBuf_AnsiStrLen

    mcEndLocals loc17.size
    pop     edi esi ebx eax
    ret

; --- EOF ---
