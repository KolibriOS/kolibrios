; --------------------------------------------------------------------------
; FILE: TLrScan.Asm
; DATE: October 11, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Enable/Disable log file
; --------------------------------------------------------------------------
align PROC_ALIGN
TLrScan_Log:
    call    TCmdBuf_Scan

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_ALPHA
    je      .check_yes_no

    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    je      .print_log_state

.print_log_format:
    mov     ecx, 712

.print:
    call    TConsole_ProutGameMsg
    ret

.check_yes_no:
    mcLoad8bitsToReg32 ecx, 159
    call    TCmdBuf_Crop
    jc      .yes

    mcLoad8bitsToReg32 ecx, 122
    call    TCmdBuf_Crop
    jnc     .print_log_format
    ;
    ; Switch logging OFF
    ;
    cmp     [glb_LogEnabled], 0
    je      .msg710

    call    TLog_Disable
    mov     [glb_LogEnabled], 0

.msg710:
    mov     ecx, 710
    jmp     .print

.yes:
    ;
    ; Switch logging ON
    ;
    cmp     [glb_LogEnabled], 1
    je      .msg711

    mov     [glb_LogEnabled], 1
    call    TLog_Enable

.msg711:
    mov     ecx, 711
    jmp     .print

.print_log_state:
    movzx   ecx, [glb_LogEnabled]
    add     ecx, 710
    jmp     .print

; --------------------------------------------------------------------------
; Input:
;   EAX = X quadrant coordinate (may be out of range)
;   EDX = Y quadrant coordinate (may be out of range)
; --------------------------------------------------------------------------
align PROC_ALIGN
TLrScan_DumpContents:
    push    eax edx

    cmp     eax, 1
    jb      .out_of_range
    cmp     edx, 1
    jb      .out_of_range
    cmp     eax, 8
    ja      .out_of_range
    cmp     edx, 8
    ja      .out_of_range

    mcLoad1 ecx
    call    TArray_SetStarChartValue

    mov     cl, ATTR_REPORT_VALUE
    call    TConsole_SetAttr

    call    TArray_GetGalaxyValue
    mov     eax, ecx
    mov     cl, 5
    call    TConsole_CramIntWidth
    jmp     .done

.out_of_range:
    mov     cl, ATTR_GALAXY_EDGE
    call    TConsole_SetAttr
    mcLoad8bitsToReg32 ecx, 134
    call    TConsole_Cram

.done:
    pop     edx eax
    ret

; --------------------------------------------------------------------------
; LRSCAN
; --------------------------------------------------------------------------
align PROC_ALIGN
TLrScan_Main:
    mov     cl, DEV_LR_SENSORS
    call    TArray_IsDamaged
    jnc     .proceed

    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .not_available

.proceed:
    call    TConsole_ScrollUp
    mov     cl, ATTR_REPORT_TEXT
    call    TConsole_SetAttr
    mcLoad8bitsToReg32 ecx, 135
    call    TConsole_Cram
    mov     cl, 1
    mcLoadGameDataPtr ebx
    mov     al, [ebx + TREKDATA.QUADX]
    mov     dl, [ebx + TREKDATA.QUADY]
    call    TConsole_CramLoc
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp
    ;
    ; EAX = QUADX-1
    ; EDX = QUADY-1
    ;
    mcLoadGameDataPtr ebx
    movzx   eax, [ebx + TREKDATA.QUADX]
    movzx   edx, [ebx + TREKDATA.QUADY]
    dec     eax
    dec     edx

    call    TLrScan_DumpContents
    inc     edx
    call    TLrScan_DumpContents
    inc     edx
    call    TLrScan_DumpContents

    push    edx eax
    call    TConsole_ScrollUp
    pop     eax edx

    inc     eax
    sub     edx, 2
    call    TLrScan_DumpContents
    inc     edx
    call    TLrScan_DumpContents
    inc     edx
    call    TLrScan_DumpContents

    push    edx eax
    call    TConsole_ScrollUp
    pop     eax edx

    inc     eax
    sub     edx, 2
    call    TLrScan_DumpContents
    inc     edx
    call    TLrScan_DumpContents
    inc     edx
    call    TLrScan_DumpContents
    call    TConsole_ScrollUp
    jmp     .done

.not_available:
    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 133
    call    TConsole_Prout

.done:
    call    TConsole_ScrollUp
    ret

; --- EOF ---
