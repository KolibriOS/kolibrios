; --------------------------------------------------------------------------
; FILE: TConsole.Asm
; DATE: September 21, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; CRAM3AS
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_Cram3Asterisks:
    mcLoad8bitsToReg32 ecx, 181
    call    TConsole_Cram
    ret

; --------------------------------------------------------------------------
; CRAMSHP
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramShip:
    mcLoadGameDataPtr ebx
    mcLoadMember al, TREKDATA.SHIP

    mcOnRegEqu al, CHAR_ENTERPRISE, .good_ship
    mcOnRegEqu al, CHAR_FQUEENE, .crappy_ship
    ret

.good_ship:
    push    144
    jmp     .dump

.crappy_ship:
    push    145

.dump:
    pop     ecx
    call    TConsole_Cram
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_ScrollUp:
    pushad

    mov     ebx, [glb_pConsole]
    mov     edi, ebx
    lea     esi, [edi + TLine.size]
    mov     ecx, TConsole.cons_LastRow
    rep     movsb

    lea     edi, [ebx + TConsole.cons_LastRow]
    mcZeroBits eax
    mov     ecx, TLine.size
    rep     stosb

    mcStoreMember TConsole.cons_CaretColumn, eax

    mov     esi, eax
    call    TApp_RefreshMainWndRect

    mov     al, 0Dh
    call    TLog_DumpChar
    mov     al, 0Ah
    call    TLog_DumpChar

    popad
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_RefreshCaretLine:
    mcBeginLocals RECT.size

    mov     esi, esp
    invoke  GetClientRect, [glb_MainWnd], esi

    mov     eax, [esi + RECT.rc_Bottom]
    sub     eax, dword [glb_CharSize + SIZE.size_Height]
    dec     eax
    mov     [esi + RECT.rc_Top], eax

    call    TApp_RefreshMainWndRect

    mcEndLocals RECT.size
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_Backspace:
    mov     ebx, [glb_pConsole]
    cmp     [ebx + TConsole.cons_CaretColumn], 0
    je      .done

    dec     [ebx + TConsole.cons_CaretColumn]
    mcZeroBits eax
    call    TConsole_PutChar

    dec     [ebx + TConsole.cons_CaretColumn]
    call    TConsole_RefreshCaretLine

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = character code
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_PutChar:
    mov     ebx, [glb_pConsole]
    cmp     [ebx + TConsole.cons_CaretColumn], CONSOLE_LAST_COL
    jae     .done

    mcLoadMember ecx, TConsole.cons_CaretColumn
    inc     [ebx + TConsole.cons_CaretColumn]
    imul    ecx, TCharacter.size
    lea     edi, [ebx + ecx + TConsole.cons_LastRow]

    mcLoadMember edx, TConsole.cons_ActiveColor
    mov     [edi + TCharacter.char_Symbol], al
    mov     [edi + TCharacter.char_Attribute], dl

    pushad
    call    TLog_DumpChar
    popad

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   CL = index of color
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_SetAttr:
    cmp     cl, CONSOLE_ATTRS
    jae     .done

    and     ecx, 0FFh
    push    ebx
    mov     ebx, [glb_pConsole]
    mcStoreMember TConsole.cons_ActiveColor, ecx
    pop     ebx

.done:
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_SetGameMsgAttr:
    mov     cl, ATTR_GAME_MSG
    call    TConsole_SetAttr
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_SetCrewMsgAttr:
    mov     cl, ATTR_CREW_MSG
    call    TConsole_SetAttr
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = character code
;   CL = number of characters
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_RepeatChar:
    and     ecx, 0FFh

.next:
    push    ecx
    call    TConsole_PutChar
    pop     ecx
    loop    .next
    ret

; --------------------------------------------------------------------------
; Input:
;   CL = number of characters
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_RepeatBlank:
    mov     al, CHAR_BLANK
    call    TConsole_RepeatChar
    ret

; --------------------------------------------------------------------------
; Input:
;   CL = number of characters
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_RepeatAsterisk:
    mov     al, CHAR_ASTERISK
    call    TConsole_RepeatChar
    ret

; --------------------------------------------------------------------------
; SUBROUTINE CRAM from original FORTRAN source
; --------------------------------------------------------------------------
; Input:
;   ECX = index of the message table
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_Cram:
    call    TMsgTable_GetItem
    mcZeroBits eax

.get_char:
    lodsb
    mcOnRegZero eax, .done

    call    TConsole_PutChar
    jmp     .get_char

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = UNICODE text
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramWide:
    mcZeroBits eax

.get_char:
    lodsw
    mcOnRegZero eax, .done

    call    TConsole_PutChar
    jmp     .get_char

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   ESI = ANSI text
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramAnsi:
    mcZeroBits eax

.get_char:
    lodsb
    mcOnRegZero eax, .done

    call    TConsole_PutChar
    jmp     .get_char

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = index of the message table
;   DL = field width
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramString:
    movzx   edx, dl
    call    TMsgTable_GetLength

    cmp     eax, edx
    jae     .cram_and_exit

    sub     edx, eax
    push    edx
    call    TConsole_Cram

    pop     ecx
    call    TConsole_RepeatBlank
    ret

.cram_and_exit:
    call    TConsole_Cram
    ret

; --------------------------------------------------------------------------
; Input:
;   CL = 1-based device index (like DEV_IMPULSE_ENGINES)
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramDevice:
    movzx   ecx, cl
    add     ecx, 59
    call    TConsole_Cram
    ret

; --------------------------------------------------------------------------
; CRAMEN
; --------------------------------------------------------------------------
; Input:
;   AL = object symbol
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramEnemy:
    pushad

    mov     edi, glb_ObjectMap
    mcLoad8bitsToReg32 ecx, 9
    mov     edx, edi
    repne   scasb
    jne     .done

.cram_it:
    sub     edi, edx
    lea     ecx, [edi + 202]
    call    TConsole_Cram

.done:
    popad
    ret

; --------------------------------------------------------------------------
; CRMSENA
; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate
;   DL = Y coordinate
;   CL = 1 for "QUADRANT", 2 for "SECTOR", 0 for none
;   BL = Enemy character
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramEnemyAt:
    push    eax ecx edx

    mov     al, bl
    call    TConsole_CramEnemy

    mov     ecx, 257
    call    TConsole_Cram

    pop     edx ecx eax
    call    TConsole_CramLoc
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = sector X coordinate
;   DL = sector Y coordinate
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramEnemyAtEx:
    push    ebx
    call    TArray_QuadPtr
    mov     bl, [ebx]
    mov     cl, 2
    call    TConsole_CramEnemyAt
    pop     ebx
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = Int32 value to print
;   CL = field width or zero (value is padded with blanks on the left side)
; --------------------------------------------------------------------------
virtual at 0
loc19:
    .bufInt32 CHARS 12
    .nWidth COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramIntWidth:
    mcBeginLocals loc19.size

    movzx   ecx, cl
    mcStoreLocal loc19.nWidth, ecx

    mcLoadLocalRef edi, loc19.bufInt32
    call    TFormat_Int32

    cmp     [esp + loc19.nWidth], 0
    je      .dump

    cmp     [esp + loc19.nWidth], ecx
    jbe     .dump

    sub     ecx, [esp + loc19.nWidth]
    neg     ecx
    call    TConsole_RepeatBlank

.dump:
    mcLoadLocalRef esi, loc19.bufInt32
    mcZeroBits eax

.next:
    lodsb
    mcOnRegZero eax, .done
    call    TConsole_PutChar
    jmp     .next

.done:
    mcEndLocals loc19.size
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = Int32 value to print
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramInt:
    pushad
    mcZeroBits ecx
    call    TConsole_CramIntWidth
    popad
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = message table index
;   EAX = Int32 value to print
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramSinglePlural:
    push    eax ecx
    call    TConsole_CramInt

    mov     al, CHAR_BLANK
    call    TConsole_PutChar

    pop     ecx
    call    TConsole_Cram

    pop     eax
    cmp     al, 1
    je      .done

    mov     al, 'S'
    call    TConsole_PutChar

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   AL = X coordinate
;   DL = Y coordinate
;   CL = KEY (1 - print "QUADRANT", 2 - print "SECTOR")
; --------------------------------------------------------------------------
virtual at 0
loc21:
    .nIX INT32 ?
    .nIY INT32 ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramLoc:
    mcBeginLocals loc21.size

    movzx   eax, al
    movzx   edx, dl
    mcStoreLocal loc21.nIX, eax
    mcStoreLocal loc21.nIY, edx

    mcOnRegZero cl, .print_values

    add     cl, 56
    movzx   ecx, cl
    call    TConsole_Cram

.print_values:
    mov     al, CHAR_BLANK
    call    TConsole_PutChar

    mcLoadLocal eax, loc21.nIX
    call    TConsole_CramInt

    mcLoad8bitsToReg32 ecx, 59
    call    TConsole_Cram

    mcLoadLocal eax, loc21.nIY
    call    TConsole_CramInt

    mcEndLocals loc21.size
    ret

; --------------------------------------------------------------------------
; Input:
;   ST(0) = value to print
;   CL = precision
;   DL = field width or zero
; --------------------------------------------------------------------------
virtual at 0
loc20:
    .bufDouble CHARS 16
    .nWidth COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramFloatWidth:
    mcBeginLocals loc20.size

    movzx   edx, dl
    mcStoreLocal loc20.nWidth, edx

    mcLoadLocalRef edi, loc20.bufDouble
    call    TFormat_Double

    cmp     [esp + loc20.nWidth], 0
    je      .dump

    cmp     [esp + loc20.nWidth], ecx
    jbe     .dump

    sub     ecx, [esp + loc20.nWidth]
    neg     ecx
    call    TConsole_RepeatBlank

.dump:
    mcLoadLocalRef esi, loc20.bufDouble
    mcZeroBits eax

.next:
    lodsb
    mcOnRegZero eax, .done
    call    TConsole_PutChar
    jmp     .next

.done:
    mcEndLocals loc20.size
    ret

; --------------------------------------------------------------------------
; Input:
;   ST(0) = value to print
;   CL = precision
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_CramFloat:
    pushad
    mcZeroBits edx
    call    TConsole_CramFloatWidth
    popad
    ret

; --------------------------------------------------------------------------
; SUBROUTINE PROUT from original FORTRAN source
; --------------------------------------------------------------------------
; Input:
;   ECX = index of the message table
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_Prout:
    push    ecx
    call    TConsole_Cram
    call    TConsole_ScrollUp
    pop     ecx
    ret

; --------------------------------------------------------------------------
; Input:
;   CL = number of lines to skip
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_Skip:
    movzx   ecx, cl

.skipping:
    push    ecx
    call    TConsole_ScrollUp
    pop     ecx
    loop    .skipping
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = index of the message table
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_ProutGameMsg:
    push    ecx
    call    TConsole_SetGameMsgAttr
    pop     ecx
    call    TConsole_Prout
    ret

; --------------------------------------------------------------------------
; Input:
;   CL = index of color
; Output:
;   EAX = COLORREF at the specified index
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_GetColorAtIndex:
    mcLoadRGB eax, 255, 0, 0
    cmp     cl, CONSOLE_ATTRS
    jae     .done

    and     ecx, 0FFh
    mov     ebx, [glb_pConsole]
    mov     eax, [ebx + 4*ecx + TConsole.cons_Attributes]

.done:
    ret

; --------------------------------------------------------------------------
; Input:
;   EDX = HDC
;   EAX = Y coordinate
;   ESI = address of TLine structure
;   ECX = number of lines left in the loop
; --------------------------------------------------------------------------
virtual at 0
loc2:
    .hDC HDC ?
    .yText INT32 ?
    .xText INT32 ?
    .pTChar PTCharacter ?
    .rgbNow COLORREF ?
    .strOneChar WCHARS 2
    .nLinesLeft COUNT ?
    .rect_Caret rb RECT.size
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_DrawLine:
    mcBeginLocals loc2.size

    mcStoreLocal loc2.nLinesLeft, ecx
    mcStoreLocal loc2.hDC, edx
    mcStoreLocal loc2.yText, eax
    mcStoreLocal loc2.pTChar, esi

    mcZeroBits eax
    mcStoreLocal loc2.rgbNow, eax
    inc     eax
    mcStoreLocal loc2.xText, eax

.load_char:
    mcLoadLocal esi, loc2.pTChar
    cmp     [esi + TCharacter.char_Symbol], 0
    je      .draw_caret

    movzx   eax, [esi + TCharacter.char_Symbol]
    mov     [esp + loc2.strOneChar], ax

    mov     cl, [esi + TCharacter.char_Attribute]
    call    TConsole_GetColorAtIndex

    mcLoadLocal edi, loc2.hDC
    cmp     [esp + loc2.rgbNow], eax
    je      .draw_char

    mcStoreLocal loc2.rgbNow, eax
    invoke  SetTextColor, edi, eax

.draw_char:
    mov     ebx, esp
    lea     esi, [ebx + loc2.strOneChar]

    invoke  TextOut, edi, \
            [ebx + loc2.xText], \
            [ebx + loc2.yText], esi, 1

    add     [ebx + loc2.pTChar], TCharacter.size

    mov     eax, dword [glb_CharSize + SIZE.size_Width]
    add     [ebx + loc2.xText], eax
    jmp     .load_char

.draw_caret:
    cmp     [esp + loc2.nLinesLeft], 1
    jne     .done
    ;
    ; Paint a rectangular caret
    ;
    mov     ebx, esp
    mcLoadMemberRef edi, loc2.rect_Caret
    mcLoadMember edx, loc2.xText
    mcLoadMember eax, loc2.yText
    mov     [edi + RECT.rc_Left], edx
    mov     [edi + RECT.rc_Top], eax

    mov     esi, glb_CharSize
    add     edx, [esi + SIZE.size_Width]
    add     eax, [esi + SIZE.size_Height]
    mov     [edi + RECT.rc_Right], edx
    mov     [edi + RECT.rc_Bottom], eax

    mov     cl, ATTR_PROMPT_VALUE
    call    TConsole_GetColorAtIndex

    invoke  CreateSolidBrush, eax
    mov     esi, eax
    mcLoadLocal edx, loc2.hDC
    mcLoadLocalRef edi, loc2.rect_Caret
    inc     [edi + RECT.rc_Left]
    inc     [edi + RECT.rc_Right]
    invoke  FillRect, edx, edi, eax
    invoke  DeleteObject, esi

.done:
    mcEndLocals loc2.size
    ret

; --------------------------------------------------------------------------
; Input:
;   EDX = HDC
; --------------------------------------------------------------------------
virtual at 0
loc3:
    .hDC HDC ?
    .yText INT32 ?
    .pLineOfChars PTLine ?
    .nLines COUNT ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_DrawAllLines:
    mcBeginLocals loc3.size

    mcStoreLocal loc3.hDC, edx

    mcLoad1 eax
    mcStoreLocal loc3.yText, eax

    mov     ebx, [glb_pConsole]
    mcStoreLocal loc3.pLineOfChars, ebx

    mcLoad8bitsToReg32 ecx, CONSOLE_ROWS
    mcStoreLocal loc3.nLines, ecx

.paint_row:
    mcLoadLocal edx, loc3.hDC
    mcLoadLocal eax, loc3.yText
    mcLoadLocal esi, loc3.pLineOfChars
    mcLoadLocal ecx, loc3.nLines
    call    TConsole_DrawLine

    add     [esp + loc3.pLineOfChars], TLine.size

    mov     eax, dword [glb_CharSize + SIZE.size_Height]
    add     [esp + loc3.yText], eax

    dec     [esp + loc3.nLines]
    jnz     .paint_row

    mcEndLocals loc3.size
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TConsole_Create:
    invoke  HeapAlloc, [glb_Allocator], HEAP_NO_SERIALIZE, TConsole.size
    mov     [glb_pConsole], eax

    mov     edi, eax
    mov     ecx, TConsole.size
    mcZeroBits eax
    push    edi
    rep     stosb
    pop     esi
    ;
    ; Init palette
    ;
    lea     edi, [esi + TConsole.cons_Attributes]

    mcLoadRGB eax, 128, 191, 255
    stosd ; ATTR_GAME_MSG

    mcLoadRGB eax, 255, 255, 206
    stosd ; ATTR_CREW_MSG

    mcLoadRGB eax, 94, 174, 174
    stosd ; ATTR_KLINGON

    mcLoadRGB eax, 0, 176, 0
    stosd ; ATTR_ROMULAN

    mcLoadRGB eax, 192, 192, 192
    stosd ; ATTR_COSMOS

    mcLoadRGB eax, 255, 255, 255
    stosd ; ATTR_SHIP

    mcLoadRGB eax, 255, 255, 0
    stosd ; ATTR_STAR

    mcLoadRGB eax, 6, 131, 255
    stosd ; ATTR_BASE

    mcLoadRGB eax, 128, 128, 64
    stosd ; ATTR_THOLIAN

    mcLoadRGB eax, 128, 128, 128
    stosd ; ATTR_BLACK_HOLE

    mcLoadRGB eax, 230, 211, 157
    stosd ; ATTR_PLANET

    mcLoadRGB eax, 0, 128, 255
    stosd ; ATTR_REPORT_TEXT

    mcLoadRGB eax, 62, 158, 255
    stosd ; ATTR_REPORT_VALUE

    mcLoadRGB eax, 248, 108, 41
    stosd ; ATTR_GALAXY_EDGE

    mcLoadRGB eax, 149, 255, 202
    stosd ; ATTR_SHIP_QUAD

    mcLoadRGB eax, 174, 194, 247
    stosd ; ATTR_SCANNED_QUAD

    mcLoadRGB eax, 218, 191, 5
    stosd ; ATTR_SCAN_SCALE

    mcLoadRGB eax, 255, 170, 170
    stosd ; ATTR_HIT_DAMAGE

    mcLoadRGB eax, 181, 181, 106
    stosd ; ATTR_WEB

    mcLoadRGB eax, 46, 170, 43
    stosd ; ATTR_COND_GREEN

    mcLoadRGB eax, 255, 255, 130
    stosd ; ATTR_COND_YELLOW

    mcLoadRGB eax, 255, 73, 64
    stosd ; ATTR_COND_RED

    mcLoadRGB eax, 0, 155, 0
    stosd ; ATTR_PROMPT_TEXT

    mcLoadRGB eax, 73, 231, 69
    stosd ; ATTR_PROMPT_VALUE

    ret

; --- EOF ---
