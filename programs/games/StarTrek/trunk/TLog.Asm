; --------------------------------------------------------------------------
; FILE: TLog.Asm
; DATE: February 21, 2009
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
align PROC_ALIGN
TLog_Create:
    invoke  HeapAlloc, [glb_Allocator], HEAP_NO_SERIALIZE, TLog.size
    mov     [glb_pLog], eax

    mov     edi, eax
    mcZeroBits eax

    mov     [edi + TLog.nRemainChars], eax
    mov     [edi + TLog.nStoredChars], eax
    mov     [edi + TLog.pCurrentBufPos], eax

    invoke  VirtualAlloc, 0, MAX_LOG_ROOM, MEM_COMMIT or MEM_RESERVE, PAGE_READWRITE
    mov     [edi + TLog.pBuffer], eax
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TLog_Enable:
    invoke  CreateFile, str_LogFileName, GENERIC_WRITE, \
            0, 0, CREATE_ALWAYS, FILE_FLAG_SEQ_SCAN, 0

    invoke  CloseHandle, eax

    mov     edi, [glb_pLog]
    mcZeroBits eax
    mov     edx, MAX_LOG_ROOM
    mov     ebx, [edi + TLog.pBuffer]

    mov     [edi + TLog.nRemainChars], edx
    mov     [edi + TLog.nStoredChars], eax
    mov     [edi + TLog.pCurrentBufPos], ebx
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TLog_Backspace:
    cmp     [glb_LogEnabled], 1
    je      .do_it
    ret

.do_it:
    mov     esi, [glb_pLog]
    cmp     [esi + TLog.nStoredChars], 0
    je      .print_backspace

    dec     [esi + TLog.nStoredChars]
    dec     [esi + TLog.pCurrentBufPos]
    inc     [esi + TLog.nRemainChars]
    ret

.print_backspace:
    mov     al, '\'
    call    TLog_DumpChar
    mov     al, 'B'
    call    TLog_DumpChar
    mov     al, 'S'
    call    TLog_DumpChar
    mov     al, 'P'
    call    TLog_DumpChar
    mov     al, '\'
    call    TLog_DumpChar
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TLog_Disable:
    cmp     [glb_LogEnabled], 1
    je      .do_it
    ret

.do_it:
    call    TLog_Flush
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TLog_Flush:
    mov     esi, [glb_pLog]
    mov     ecx, [esi + TLog.nStoredChars]
    jecxz   .done

    invoke  CreateFile, str_LogFileName, GENERIC_WRITE, \
            0, 0, OPEN_EXISTING, FILE_FLAG_SEQ_SCAN, 0

    mov     ebx, eax
    invoke  SetFilePointer, eax, 0, 0, 2

    invoke  WriteFile, ebx, [esi + TLog.pBuffer], [esi + TLog.nStoredChars], glb_FPU_Int32, 0
    invoke  CloseHandle, ebx

    mcZeroBits eax
    mov     edx, MAX_LOG_ROOM
    mov     ebx, [esi + TLog.pBuffer]

    mov     [esi + TLog.nRemainChars], edx
    mov     [esi + TLog.nStoredChars], eax
    mov     [esi + TLog.pCurrentBufPos], ebx

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = character to append into log buffer.
; --------------------------------------------------------------------------
align PROC_ALIGN
TLog_DumpChar:
    cmp     [glb_LogEnabled], 1
    je      .do_it
    ret

.do_it:
    mcOnRegZero al, .do_backspace

    mov     esi, [glb_pLog]
    cmp     [esi + TLog.nRemainChars], 0
    jne     .dump

    push    eax
    call    TLog_Flush
    pop     eax
    mov     esi, [glb_pLog]

.dump:
    mov     edi, [esi + TLog.pCurrentBufPos]
    stosb
    mov     [esi + TLog.pCurrentBufPos], edi
    inc     [esi + TLog.nStoredChars]
    dec     [esi + TLog.nRemainChars]
    ret

.do_backspace:
    call    TLog_Backspace
    ret

; --- EOF ---
