; --------------------------------------------------------------------------
; FILE: TApp.Asm
; DATE: September 21, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; DESTRCT
; --------------------------------------------------------------------------
virtual at 0
loc84:
    .nCountDown COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_SelfDestruct:
    mcBeginLocals loc84.size

    mov     cl, DEV_COMPUTER
    call    TArray_IsDamaged
    jnc     .L5

    call    TConsole_ScrollUp
    mov     ecx, 686
    call    TConsole_ProutGameMsg
    call    TConsole_ScrollUp
    jmp     .done

.L5:
    call    TConsole_ScrollUp
    mov     ecx, 290
    call    TConsole_ProutGameMsg
    mov     ecx, 687
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    call    TConsole_ScrollUp

    mcLoad8bitsToReg32 ecx, 10
    mcStoreLocal loc84.nCountDown, ecx

.pre_sequence:
    mov     ecx, 694
    call    TConsole_Cram
    mcLoadLocal eax, loc84.nCountDown
    call    TConsole_CramInt
    call    TConsole_RefreshCaretLine
    invoke  Sleep, 1000

    dec     [esp + loc84.nCountDown]
    cmp     [esp + loc84.nCountDown], 6
    jae     .pre_sequence

    call    TConsole_ScrollUp
    call    TConsole_ScrollUp

    mov     cl, ATTR_PROMPT_TEXT
    call    TConsole_SetAttr

    mov     ecx, 689
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    inc     ecx
    call    TGame_Prompt
    call    TConsole_ScrollUp

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    je      .password_rejected

    mcLoadGameDataPtr edi
    add     edi, TREKDATA.PASSWD
    call    TString_AnsiEqual
    jnc     .password_rejected

    mov     ecx, 692
    call    TConsole_ProutGameMsg
    mov     cl, 22
    call    TConsole_RepeatBlank

.final_sequence:
    mov     ecx, 694
    call    TConsole_Cram
    mcLoadLocal eax, loc84.nCountDown
    call    TConsole_CramInt
    call    TConsole_RefreshCaretLine
    invoke  Sleep, 1000

    dec     [esp + loc84.nCountDown]
    jnz     .final_sequence

    call    TConsole_ScrollUp

    fld     [glb_dbl_0dot05]
    call    TRandom_Ranf
    mc_CMP_ST0_ST1
    jnc     .everyone_wasted

    mov     ecx, 693
    call    TConsole_ProutGameMsg
    invoke  Sleep, 1000

.everyone_wasted:
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp
    call    TPlanet_KaBoom
    jmp     .done

.password_rejected:
    mov     ecx, 695
    call    TConsole_ProutGameMsg
    inc     ecx
    call    TConsole_Prout
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp

.done:
    mcEndLocals loc84.size
    ret

; --------------------------------------------------------------------------
; PLAQUE
; --------------------------------------------------------------------------
virtual at 0
loc112:
    .pNextInByte PCHAR ?
    .pNextOutByte PCHAR ?
    .bufAnsiValue CHARS 16
    .bufAnsiKirkAlias CHARS 32
    .pAllocatedOutBuffer PCHAR ?
    .timeNow rb SYSTIME.size
    .nLenName COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_PlaqueProcessor:
    mcBeginLocals loc112.size

    mov     ecx, 727
    call    TGame_Prompt

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    je      .done

    mcLoadLocalRef edi, loc112.bufAnsiKirkAlias
    call    TString_AnsiCopy

    mov     esi, glb_Plaque
    invoke  HeapAlloc, [glb_Allocator], HEAP_NO_SERIALIZE, 8192 ; <-- 2 pages is enough!
    mcStoreLocal loc112.pAllocatedOutBuffer, eax
    mcStoreLocal loc112.pNextOutByte, eax
    mcStoreLocal loc112.pNextInByte, esi

.load_input_byte:
    mcLoadLocal esi, loc112.pNextInByte
    inc     [esp + loc112.pNextInByte]
    mov     al, [esi]

    mcOnRegEqu al, PLAQUE_STRING, .dump_string
    mcOnRegEqu al, PLAQUE_INSERT_DATE, .dump_date
    mcOnRegEqu al, PLAQUE_INSERT_KLRATE, .dump_klrate
    mcOnRegEqu al, PLAQUE_INSERT_LEVEL, .dump_level
    mcOnRegEqu al, PLAQUE_INSERT_NAME, .dump_captain_name
    mcOnRegEqu al, PLAQUE_INSERT_SCORE, .dump_score
    mcOnRegEqu al, PLAQUE_END_OF_LINE, .dump_CR_LF
    mcOnRegEqu al, PLAQUE_END_OF_ALL, .save
    ;
    ; This byte is a count of charactes defined by next byte
    ;
    movzx   ecx, al
    mov     al, [esi + 1]
    inc     [esp + loc112.pNextInByte]
    mcLoadLocal edi, loc112.pNextOutByte
    add     [esp + loc112.pNextOutByte], ecx
    rep     stosb
    jmp     .load_input_byte

.dump_string:
    ;
    ; The next byte is a length of data to be copied
    ;
    movzx   ecx, byte [esi + 1]
    inc     [esp + loc112.pNextInByte]
    ;
    ; Copy data
    ;
    mcLoadLocal edi, loc112.pNextOutByte
    mcLoadLocal esi, loc112.pNextInByte
    add     [esp + loc112.pNextInByte], ecx
    add     [esp + loc112.pNextOutByte], ecx
    rep     movsb
    jmp     .load_input_byte

.dump_date:
    mcLoadLocalRef edi, loc112.timeNow
    invoke  GetLocalTime, edi

    movzx   ecx, word [edi + SYSTIME.wMonth]
    add     ecx, 714
    call    TMsgTable_GetItem

    call    TCmdBuf_AnsiStrLen
    mcLoadLocal edi, loc112.pNextOutByte
    add     [esp + loc112.pNextOutByte], ecx
    rep     movsb

    mov     al, ' '
    stosb
    inc     [esp + loc112.pNextOutByte]

    mcLoadLocalRef esi, loc112.timeNow
    movzx   eax, word [esi + SYSTIME.wDay]
    mcLoadLocalRef edi, loc112.bufAnsiValue
    call    TFormat_Int32

    mov     esi, edi
    mcLoadLocal edi, loc112.pNextOutByte
    add     [esp + loc112.pNextOutByte], ecx
    rep     movsb

    mov     al, ','
    stosb
    mov     al, ' '
    stosb
    add     [esp + loc112.pNextOutByte], 2

    mcLoadLocalRef esi, loc112.timeNow
    movzx   eax, word [esi + SYSTIME.wYear]
    mcLoadLocalRef edi, loc112.bufAnsiValue
    call    TFormat_Int32

    mov     esi, edi
    mcLoadLocal edi, loc112.pNextOutByte
    add     [esp + loc112.pNextOutByte], ecx
    rep     movsb
    jmp     .load_input_byte

.dump_klrate:
    mov     cl, 2
    mcLoadLocalRef edi, loc112.bufAnsiValue
    fld     [glb_dbl_KlingonsPerDate]
    call    TFormat_Double

    mov     esi, edi
    mcLoadLocal edi, loc112.pNextOutByte
    add     [esp + loc112.pNextOutByte], ecx
    rep     movsb
    jmp     .load_input_byte

.dump_level:
    mcLoadGameDataPtr esi
    movzx   ecx, [esi + TREKDATA.SKILL]
    add     cl, 12
    call    TMsgTable_GetItem

    mcLoadLocal edi, loc112.pNextOutByte
    mcZeroBits eax
    mcZeroBits ecx

.next_level_char:
    lodsb
    mcOnRegZero eax, .level_copied

    stosb
    inc     ecx
    jmp     .next_level_char

.level_copied:
    add     [esp + loc112.pNextOutByte], ecx
    jmp     .load_input_byte

.dump_captain_name:
    ;
    ; NSKIP = 65 - LEN (NAME)/2
    ;
    mcLoadLocalRef esi, loc112.bufAnsiKirkAlias
    call    TCmdBuf_AnsiStrLen
    mcStoreLocal loc112.nLenName, ecx
    mcLoad8bitsToReg32 eax, 65
    shr     ecx, 1
    sub     eax, ecx

    mov     cl, ' '
    xchg    eax, ecx
    mcLoadLocal edi, loc112.pNextOutByte
    add     [esp + loc112.pNextOutByte], ecx
    rep     stosb

    mcLoadLocalRef esi, loc112.bufAnsiKirkAlias
    mcLoadLocal ecx, loc112.nLenName
    add     [esp + loc112.pNextOutByte], ecx
    rep     movsb
    jmp     .dump_CR_LF

.dump_score:
    mcLoadLocalRef edi, loc112.bufAnsiValue
    mov     eax, [glb_Score]
    call    TFormat_Int32

    mov     esi, edi
    mcLoadLocal edi, loc112.pNextOutByte
    add     [esp + loc112.pNextOutByte], ecx
    rep     movsb
    jmp     .load_input_byte

.dump_CR_LF:
    mcLoadLocal edi, loc112.pNextOutByte
    mov     al, 0Dh
    stosb
    mov     al, 0Ah
    stosb
    mcStoreLocal loc112.pNextOutByte, edi
    jmp     .load_input_byte

.save:
    invoke  CreateFileAnsi, str_PlaqueFile, GENERIC_WRITE, 0, 0,\
            CREATE_ALWAYS, FILE_FLAG_SEQ_SCAN, 0

    mov     ebx, eax
    mcOnRegEqu ebx, -1, .done

    mcLoadLocal esi, loc112.pAllocatedOutBuffer
    mcLoadLocalRef edi, loc112.pNextInByte
    mcLoadLocal ecx, loc112.pNextOutByte
    sub     ecx, esi
    invoke  WriteFile, ebx, esi, ecx, edi, 0
    invoke  CloseHandle, ebx

    call    TConsole_SetGameMsgAttr
    mov     cl, 3
    call    TConsole_Skip

    mov     ecx, 728
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout
    inc     ecx
    call    TConsole_Prout

    mov     cl, 3
    call    TConsole_Skip

.done:
    mcEndLocals loc112.size
    ret

; --------------------------------------------------------------------------
; Output:
;   CF = TRUE if directory has been set
; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_SetLocalDir:
    mcBeginLocals 260*2

    mov     esi, esp
    invoke  GetModuleFileName, 0, esi, 260

    mcZeroBits eax
    mcZeroBits edi

.next_char:
    lodsw
    mcOnRegZero eax, .set_dir

    cmp     al, '\'
    jne     .next_char

    mov     edi, esi
    jmp     .next_char

.set_dir:
    mcOnRegZero edi, .failed

    stosw
    invoke  SetCurrentDir, esp
    mcOnRegZero eax, .failed

.done:
    mcEndLocals 260*2
    stc
    ret

.failed:
    invoke  MessageBox, 0, str_SetDirFailed, str_MainWndTitle, 10h
    mcEndLocals 260*2
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = HDC
; --------------------------------------------------------------------------
virtual at 0
loc1:
    .hDC HDC ?
    .hMemDC HDC ?
    .hSurfaceBmp HBITMAP ?
    .hDefaultBmp HBITMAP ?
    .hDefaultFont HFONT ?
    .rect_Client rb RECT.size
    .bkMode INT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_WmPaint:
    mcBeginLocals loc1.size
    mcStoreLocal loc1.hDC, eax

    mcLoadLocalRef edi, loc1.rect_Client
    invoke  GetClientRect, [glb_MainWnd], edi

    mcLoadLocal edx, loc1.hDC
    invoke  CreateCompatibleDC, edx
    mcStoreLocal loc1.hMemDC, eax

    mov     ebx, esp
    invoke  CreateCompatibleBitmap, [ebx + loc1.hDC], \
            dword [ebx + loc1.rect_Client + RECT.rc_Right], \
            dword [ebx + loc1.rect_Client + RECT.rc_Bottom]

    mcStoreLocal loc1.hSurfaceBmp, eax

    invoke  SelectObject, [ebx + loc1.hMemDC], eax
    mcStoreLocal loc1.hDefaultBmp, eax

    invoke  GetStockObject, BLACK_BRUSH
    mcLoadLocalRef esi, loc1.rect_Client
    invoke  FillRect, [ebx + loc1.hMemDC], esi, eax

    invoke  SetBkMode, [ebx + loc1.hMemDC], TRANSPARENT
    mcStoreLocal loc1.bkMode, eax

    invoke  SelectObject, [ebx + loc1.hMemDC], [glb_Font]
    mcStoreLocal loc1.hDefaultFont, eax

    mcLoadLocal edx, loc1.hMemDC
    call    TConsole_DrawAllLines

    mov     ebx, esp
    invoke  SetBkMode, [ebx + loc1.hMemDC], [ebx + loc1.bkMode]
    invoke  SelectObject, [ebx + loc1.hMemDC], [ebx + loc1.hDefaultFont]

    mcZeroBits ecx

    invoke  BitBlt, [ebx + loc1.hDC], ecx, ecx, \
            dword [ebx + loc1.rect_Client + RECT.rc_Right], \
            dword [ebx + loc1.rect_Client + RECT.rc_Bottom], \
            [ebx + loc1.hMemDC], ecx, ecx, SRCCOPY

    invoke  SelectObject, [ebx + loc1.hMemDC], [ebx + loc1.hDefaultBmp]
    invoke  DeleteObject, [ebx + loc1.hSurfaceBmp]
    invoke  DeleteDC, [ebx + loc1.hMemDC]

    mcEndLocals loc1.size
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = WCHAR received from user
; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_WmChar:
    and     eax, 0FFh

    cmp     al, 20h
    jb      .control_char

    cmp     al, 'a'
    jb      .draw_symbol
    cmp     al, 'z'
    ja      .draw_symbol

    sub     al, 20h

.draw_symbol:
    mov     cl, ATTR_PROMPT_VALUE
    call    TConsole_SetAttr
    call    TCmdBuf_PutChar
    call    TConsole_PutChar
    call    TConsole_RefreshCaretLine
    ret

.backspace:
    call    TCmdBuf_IsEmpty
    jnc     .del_last_char
    ret

.del_last_char:
    call    TCmdBuf_Backspace
    call    TConsole_Backspace
    ret

.command_complete:
    call    TConsole_ScrollUp
    call    TCmdBuf_Parse
    lock    inc [glb_GameThreadBusy]
    ret

.key_escape:
    ret

.control_char:
    cmp     al, CHAR_BACKSPACE
    je      .backspace
    cmp     al, CHAR_ENTER
    je      .command_complete
    cmp     al, CHAR_ESC
    je      .key_escape
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_QueryTextSize:
    invoke  GetDC, 0
    mov     esi, eax

    invoke  SelectObject, eax, [glb_Font]
    push    eax

    invoke  GetTextExtentPoint32, esi, str_CharSizeText, 2, glb_CharSize

    pop     edx
    invoke  SelectObject, esi, edx

    invoke  ReleaseDC, 0, esi

    shr     dword [glb_CharSize + SIZE.size_Width], 1
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = address of rectangle to redraw
; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_RefreshMainWndRect:
    mov     ebx, [glb_MainWnd]
    invoke  InvalidateRect, ebx, esi, 1
    invoke  UpdateWindow, ebx
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_AdjustWndRect:
    mcBeginLocals RECT.size

    mov     esi, esp

    mcZeroBits eax
    mov     [esi + RECT.rc_Left], eax
    mov     [esi + RECT.rc_Top], eax

    mov     ecx, dword [glb_CharSize + SIZE.size_Width]
    imul    ecx, CONSOLE_COLS
    add     ecx, 2
    mov     [esi + RECT.rc_Right], ecx

    mov     ecx, dword [glb_CharSize + SIZE.size_Height]
    imul    ecx, CONSOLE_ROWS
    add     ecx, 2
    mov     [esi + RECT.rc_Bottom], ecx

    invoke  OffsetRect, esi, 200, 32
    invoke  AdjustWindowRect, esi, WS_NOSIZEWND, 0

    mcRectWidth ecx, esi
    mcRectHeight edx, esi

    invoke  MoveWindow, [glb_MainWnd], \
            [esi + RECT.rc_Left], \
            [esi + RECT.rc_Top], \
            ecx, edx, 0

    mcEndLocals RECT.size
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_WmCreate:
    call    TCommon_Allocate
    call    TMsgTable_Load
    call    TCmdBuf_Create
    call    TApp_QueryTextSize
    call    TApp_AdjustWndRect

    invoke  PostMessage, [glb_MainWnd], WM_CUSTOM_DO_PRELIM, 0, 0
    ret

; --------------------------------------------------------------------------
; Input:
;   hWnd, uiMsg, WParam, LParam
; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_WndProc:
    mcProlog

    mov     eax, uiMsg
    mcOnRegEqu eax, WM_CREATE, .OnCreate
    mcOnRegEqu eax, WM_DESTROY, .OnDestroy
    mcOnRegEqu eax, WM_ERASEBKGND, .OnEraseBkGnd
    mcOnRegEqu eax, WM_PAINT, .OnPaint
    mcOnRegEqu eax, WM_CHAR, .OnChar
    mcOnRegEqu eax, WM_CUSTOM_DO_PRELIM, .StartGameThread

    invoke  DefWindowProc, hWnd, uiMsg, WParam, LParam

.exit:
    mcEpilog
    ret     10h

.StartGameThread:
    invoke  CreateThread, 0, 0, TGame_Thread, 0, 0, 0

.ret_zero:
    mcZeroBits eax
    jmp     .exit

.OnCreate:
    mov     eax, hWnd
    mov     [glb_MainWnd], eax
    call    TApp_WmCreate
    jmp     .ret_zero

.OnDestroy:
    invoke  PostQuitMessage, 0
    jmp     .ret_zero

.OnEraseBkGnd:
    mcLoad1 eax
    jmp     .exit

.OnPaint:
    mcBeginLocals PAINTSTRUCT.size
    invoke  BeginPaint, [glb_MainWnd], esp
    call    TApp_WmPaint
    invoke  EndPaint, [glb_MainWnd], esp
    mcEndLocals PAINTSTRUCT.size
    jmp     .ret_zero

.OnChar:
    mcZeroBits eax
    lock add [glb_GameThreadBusy], eax
    jnz     .ret_zero

    mov     eax, WParam
    call    TApp_WmChar
    jmp     .ret_zero

; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_CreateFont:
    mcBeginLocals LOGFONT.size

    mov     edi, esp
    mov     ecx, LOGFONT.size
    mcZeroBits eax
    rep     stosb

    mov     edx, FW_NORMAL
    mov     [esp + LOGFONT.lf_Weight], edx

    mov     dl, DEFAULT_CHARSET
    mov     [esp + LOGFONT.lf_CharSet], dl

    lea     edi, [esp + LOGFONT.lf_FaceName]
    mov     esi, str_FontFace
    call    TString_Copy

    invoke  GetDC, 0
    mov     esi, eax
    invoke  GetDeviceCaps, esi, LOGPIXELSY
    invoke  MulDiv, 12, eax, 72

    neg     eax
    mov     [esp + LOGFONT.lf_Height], eax
    invoke  ReleaseDC, 0, esi

    invoke  CreateFontIndirect, esp
    mov     [glb_Font], eax

    mcEndLocals LOGFONT.size
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_RegisterWnd:
    mcBeginLocals WNDCLASS.size

    mov     ebx, esp

    mcZeroBits ecx
    mcStoreMember WNDCLASS.wc_lpszMenuName, ecx
    mcStoreMember WNDCLASS.wc_cbClsExtra, ecx
    mcStoreMember WNDCLASS.wc_cbWndExtra, ecx
    mcStoreMember WNDCLASS.wc_hbrBackground, ecx

    invoke  LoadCursor, 0, IDC_ARROW
    mcStoreMember WNDCLASS.wc_hCursor, eax

    invoke  LoadIcon, 0, IDI_EXCLAMATION
    mcStoreMember WNDCLASS.wc_hIcon, eax

    mov     eax, [glb_Instance]
    mcStoreMember WNDCLASS.wc_hInstance, eax

    mov     eax, TApp_WndProc
    mcStoreMember WNDCLASS.wc_lpfnWndProc, eax

    mov     eax, str_MainWndClass
    mcStoreMember WNDCLASS.wc_lpszClassName, eax

    invoke  RegisterClass, ebx

    mcEndLocals WNDCLASS.size
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_CreateMainWnd:
    invoke  CreateWindowEx, WS_EX_APPWINDOW, str_MainWndClass, \
            str_MainWndTitle, WS_NOSIZEWND, \
            80, 40, 500, 500, \
            0, 0, [glb_Instance], 0
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_Init:
    invoke  GetModuleHandle, 0
    mov     [glb_Instance], eax

    invoke  HeapCreate, HEAP_NO_SERIALIZE, 0x20000, 0
    mov     [glb_Allocator], eax

    call    TRandom_Create
    call    TApp_RegisterWnd
    call    TApp_CreateFont
    call    TApp_CreateMainWnd
    call    TConsole_Create
    call    TLog_Create
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TApp_Run:
    mcBeginLocals MSG.size

    mov     esi, [glb_MainWnd]
    invoke  ShowWindow, esi, SW_SHOWNORMAL
    invoke  UpdateWindow, esi

@@:
    mcZeroBits eax
    mov     edi, esp
    invoke  GetMessage, edi, eax, eax, eax
    mcOnRegZero eax, .done

    invoke  TranslateMessage, edi
    invoke  DispatchMessage, edi
    jmp     @r

.done:
    call    TLog_Disable

    mcEndLocals MSG.size
    ret

; --- EOF ---
