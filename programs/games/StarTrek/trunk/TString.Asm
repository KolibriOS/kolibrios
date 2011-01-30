; --------------------------------------------------------------------------
; FILE: TString.Asm
; DATE: September 21, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Input:
;   ESI = source string
;   EDI = destination string
; --------------------------------------------------------------------------
align PROC_ALIGN
TString_Copy:
    mcZeroBits eax

.more:
    lodsw
    stosw
    mcOnRegNotZero eax, .more

    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = source string
;   EDI = destination string
; --------------------------------------------------------------------------
align PROC_ALIGN
TString_AnsiCopy:
    mcZeroBits eax

.more:
    lodsb
    stosb
    mcOnRegNotZero eax, .more

    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = string #1
;   EDI = string #2
; Output:
;   CF=1 if strings are same
; --------------------------------------------------------------------------
align PROC_ALIGN
TString_AnsiEqual:
    push    esi edi

.check_both:
    cmpsb
    jne     .ret_false

    cmp     byte [esi - 1], 0
    jne     .check_both

    pop     edi esi
    stc
    ret

.ret_false:
    pop     edi esi
    clc
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = source string
;   EDI = destination string
; Output:
;   EDI = set to concatenate another string
; --------------------------------------------------------------------------
align PROC_ALIGN
TString_CopyEx:
    call    TString_Copy
    sub     edi, 2
    ret

; --- EOF ---
