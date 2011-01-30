; --------------------------------------------------------------------------
; FILE: TSrScan.Asm
; DATE: October 11, 2008
; --------------------------------------------------------------------------

; --------------------------------------------------------------------------
; Input:
;   AL = character from QUAD array
; --------------------------------------------------------------------------
align PROC_ALIGN
TSrScan_SetCosmosAttr:
    push    ecx

    mcOnRegEqu al, CHAR_ASTERISK, .attr_star
    mcOnRegEqu al, CHAR_BLACK_HOLE, .attr_bhole
    mcOnRegEqu al, CHAR_COMMANDER, .attr_klingon
    mcOnRegEqu al, CHAR_ENTERPRISE, .attr_ship
    mcOnRegEqu al, CHAR_FQUEENE, .attr_ship
    mcOnRegEqu al, CHAR_KLINGON, .attr_klingon
    mcOnRegEqu al, CHAR_PLANET, .attr_planet
    mcOnRegEqu al, CHAR_ROMULAN, .attr_romulan
    mcOnRegEqu al, CHAR_SCOM, .attr_klingon
    mcOnRegEqu al, CHAR_STARBASE, .attr_base
    mcOnRegEqu al, CHAR_THING, .attr_thing
    mcOnRegEqu al, CHAR_THOLIAN, .attr_tholian
    mcOnRegEqu al, CHAR_WEB, .attr_web

    mov     cl, ATTR_COSMOS
    jmp     .apply

.attr_star:
    mov     cl, ATTR_STAR
    jmp     .apply

.attr_bhole:
    mov     cl, ATTR_BLACK_HOLE
    jmp     .apply

.attr_klingon:
    mov     cl, ATTR_KLINGON
    jmp     .apply

.attr_ship:
    mov     cl, ATTR_SHIP
    jmp     .apply

.attr_planet:
    mov     cl, ATTR_PLANET
    jmp     .apply

.attr_romulan:
    mov     cl, ATTR_ROMULAN
    jmp     .apply

.attr_base:
    mov     cl, ATTR_BASE
    jmp     .apply

.attr_thing:
    mov     cl, ATTR_BLACK_HOLE
    jmp     .apply

.attr_tholian:
    mov     cl, ATTR_THOLIAN
    jmp     .apply

.attr_web:
    mov     cl, ATTR_WEB

.apply:
    call    TConsole_SetAttr
    pop     ecx
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = row index from 1 to 10
; --------------------------------------------------------------------------
align PROC_ALIGN
TSrScan_CramSpaceRow:
    dec     eax
    imul    eax, 10

    mcLoadGameDataPtr edx
    lea     esi, [edx + eax + TREKDATA.QUAD]
    mcLoad8bitsToReg32 ecx, 10

.dump_sector:
    push    ecx

    lodsb
    call    TSrScan_SetCosmosAttr
    call    TConsole_PutChar
    mov     al, CHAR_BLANK
    call    TConsole_PutChar

    pop     ecx
    loop    .dump_sector
    ret

; --------------------------------------------------------------------------
; Input:
;   ECX = index of SRSCAN report item text
; --------------------------------------------------------------------------
align PROC_ALIGN
TSrScan_Helper1:
    call    TConsole_Cram
    mov     cl, ATTR_REPORT_VALUE
    call    TConsole_SetAttr
    mcLoadGameDataPtr ebx
    ret

; --------------------------------------------------------------------------
align PROC_ALIGN
TSrScan_PrintTubesInfo:
    mov     ecx, 705
    call    TConsole_Cram

    mcLoadGameDataPtr esi
    movzx   eax, [esi + TREKDATA.NPTUBES]
    call    TConsole_CramInt

    mcLoadGameDataPtr esi
    cmp     [esi + TREKDATA.QTORPS], 0
    je      .close_bracket
    cmp     [esi + TREKDATA.TORPS], 0
    je      .close_bracket

    mov     ecx, 706
    call    TConsole_Cram

.close_bracket:
    mov     al, ')'
    call    TConsole_PutChar
    ret

; --------------------------------------------------------------------------
; Input:
;   EAX = row index from 1 to 10
; --------------------------------------------------------------------------
align PROC_ALIGN
TSrScan_CramStatusRow:
    mov     cl, ATTR_REPORT_TEXT
    call    TConsole_SetAttr

    mcOnRegEqu eax, 1, .print_stardate
    mcOnRegEqu eax, 2, .print_condition
    mcOnRegEqu eax, 3, .print_position
    mcOnRegEqu eax, 4, .print_lsupport
    mcOnRegEqu eax, 5, .print_warp
    mcOnRegEqu eax, 6, .print_energy
    mcOnRegEqu eax, 7, .print_torpedoes
    mcOnRegEqu eax, 8, .print_shields
    mcOnRegEqu eax, 9, .print_klingons
    ;
    ; Time left
    ;
    mcLoad8bitsToReg32 ecx, 103
    call    TSrScan_Helper1

    fld     [ebx + TREKDATA.REMTIME]
    mov     cl, 2
    call    TConsole_CramFloat
    jmp     .done

.print_stardate:
    mcLoad8bitsToReg32 ecx, 94
    call    TSrScan_Helper1

    fld     [ebx + TREKDATA.DATE]
    mov     cl, 1
    call    TConsole_CramFloat
    jmp     .done

.print_condition:
    mcLoad8bitsToReg32 ecx, 95
    call    TSrScan_Helper1

    cmp     [ebx + TREKDATA.CONDIT], CONDITION_DOCKED
    je      .do_condition

    push    ebx
    call    TCommon_NewCondition
    pop     ebx

.do_condition:
    movzx   eax, [ebx + TREKDATA.CONDIT]
    mov     esi, glb_AttrCondition
    mov     cl, [esi + eax - 1]
    call    TConsole_SetAttr

    lea     ecx, [eax + 103]
    call    TConsole_Cram
    jmp     .done

.print_position:
    mcLoad8bitsToReg32 ecx, 96
    call    TSrScan_Helper1

    mov     al, [ebx + TREKDATA.QUADX]
    mov     dl, [ebx + TREKDATA.QUADY]
    mcZeroBits ecx
    push    ebx
    call    TConsole_CramLoc

    mov     al, ','
    call    TConsole_PutChar
    pop     ebx

    mov     al, [ebx + TREKDATA.SECTX]
    mov     dl, [ebx + TREKDATA.SECTY]
    mcZeroBits ecx
    call    TConsole_CramLoc
    jmp     .done

.print_lsupport:
    mcLoad8bitsToReg32 ecx, 97
    call    TSrScan_Helper1

    mov     cl, DEV_LIFE_SUPPORT
    call    TArray_IsDamaged
    jc      .L44

    mcLoad8bitsToReg32 ecx, 108
    call    TConsole_Cram
    jmp     .done

.L44:
    cmp     [ebx + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .L45

    mcLoad8bitsToReg32 ecx, 109
    call    TConsole_Cram
    jmp     .done

.L45:
    mcLoad8bitsToReg32 ecx, 110
    push    ebx
    call    TConsole_Cram
    pop     ebx

    fld     [ebx + TREKDATA.LSUPRES]
    mov     cl, 2
    mov     dl, 4
    call    TConsole_CramFloatWidth
    jmp     .done

.print_warp:
    mcLoad8bitsToReg32 ecx, 98
    call    TSrScan_Helper1

    fld     [ebx + TREKDATA.WARPFAC]
    mov     cl, 1
    call    TConsole_CramFloat
    jmp     .done

.print_energy:
    mcLoad8bitsToReg32 ecx, 99
    call    TSrScan_Helper1

    fld     [ebx + TREKDATA.ENERGY]
    mov     cl, 2
    call    TConsole_CramFloat
    jmp     .done

.print_torpedoes:
    mcLoad8bitsToReg32 ecx, 100
    call    TSrScan_Helper1

    movzx   eax, [ebx + TREKDATA.TORPS]
    call    TConsole_CramInt
    call    TSrScan_PrintTubesInfo
    jmp     .done

.print_shields:
    mcLoad8bitsToReg32 ecx, 101
    call    TSrScan_Helper1

    mov     cl, DEV_SHIELDS
    call    TArray_IsDamaged
    jc      .shields_damaged

    movzx   ecx, [ebx + TREKDATA.SHLDUP]
    add     cl, 120

.cram_shields_state:
    push    ebx
    call    TConsole_Cram
    pop     ebx

    fld     [ebx + TREKDATA.SHLD]
    fld     [ebx + TREKDATA.INSHLD]
    fdivp
    fld     [glb_dbl_100]
    fmulp
    frndint
    call    TCommon_FP_Truncate
    push    ebx
    call    TConsole_CramInt

    mcLoad8bitsToReg32 ecx, 111
    call    TConsole_Cram

    pop     ebx
    fld     [ebx + TREKDATA.SHLD]
    frndint
    call    TCommon_FP_Truncate
    call    TConsole_CramInt

    mcLoad8bitsToReg32 ecx, 112
    call    TConsole_Cram
    jmp     .done

.shields_damaged:
    mcLoad8bitsToReg32 ecx, 119
    jmp     .cram_shields_state

.print_klingons:
    mcLoad8bitsToReg32 ecx, 102
    call    TSrScan_Helper1

    movzx   eax, [ebx + TREKDATA.REMKL]
    call    TConsole_CramInt

.done:
    ret

; --------------------------------------------------------------------------
; STATUS
; --------------------------------------------------------------------------
align PROC_ALIGN
TSrScan_Status:
    call    TConsole_ScrollUp

    mcLoad1 eax
    mcLoad8bitsToReg32 ecx, 10

.dump_row:
    push    eax ecx

    call    TSrScan_CramStatusRow
    call    TConsole_ScrollUp

    pop     ecx eax
    inc     eax
    loop    .dump_row

    call    TConsole_ScrollUp
    ret

; --------------------------------------------------------------------------
; REQUEST
; --------------------------------------------------------------------------
align PROC_ALIGN
TSrScan_Request:

.L301:
    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_ALPHA
    je      .L303

.L302:
    mcLoad8bitsToReg32 ecx, 115
    call    TGame_Prompt
    jmp     .L301

.L303:
    mcLoad8bitsToReg32 edx, 1

.check_cmd:
    mcLoad8bitsToReg32 ecx, 122
    add     ecx, edx
    push    edx
    call    TCmdBuf_Crop
    pop     edx
    jc      .row_in_EDX

    inc     edx
    cmp     edx, 10
    jbe     .check_cmd
    ;
    ; List possible request items
    ;
    call    TConsole_ScrollUp
    call    TConsole_SetGameMsgAttr
    mcLoad8bitsToReg32 ecx, 116
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 117
    call    TConsole_Prout
    mcLoad8bitsToReg32 ecx, 118
    call    TConsole_Prout
    call    TConsole_ScrollUp
    jmp     .L302

.row_in_EDX:
    push    edx
    call    TConsole_ScrollUp
    pop     eax
    ;
    ; Due to mapping we need to swap places of
    ; TIME and TORPEDOES request. Basic logic here as follows:
    ;
    ; IF (EAX == 7) THEN EAX=10
    ; IF (EAX == 10) THEN EAX=7
    ;
    mcOnRegEqu eax, 7, .load_10
    mcOnRegEqu eax, 10, .load_7

.dump_info:
    call    TSrScan_CramStatusRow
    call    TConsole_ScrollUp
    call    TConsole_ScrollUp
    ret

.load_7:
    sub     eax, 3
    jmp     .dump_info

.load_10:
    add     eax, 3
    jmp     .dump_info

; --------------------------------------------------------------------------
; SRSCAN
; --------------------------------------------------------------------------
virtual at 0
loc24:
    .bRightSide BOOL ?
    .nRow INDEX ?
    .size = $
end virtual
; --------------------------------------------------------------------------
align PROC_ALIGN
TSrScan_Main:
    mcBeginLocals loc24.size

    mov     cl, DEV_SR_SENSORS
    call    TArray_IsDamaged
    jnc     .proceed

    mcLoadGameDataPtr ebx
    cmp     [ebx + TREKDATA.CONDIT], CONDITION_DOCKED
    jne     .not_available

.proceed:
    mcLoad1 eax
    mcStoreLocal loc24.bRightSide, eax
    mcStoreLocal loc24.nRow, eax

    call    TCmdBuf_Scan
    cmp     [ebx + TCmdBuf.cmdbuf_KEY], CMD_TOKEN_EOL
    je      .L3

    mcLoad8bitsToReg32 ecx, 122
    call    TCmdBuf_Crop
    jnc     .L3

    dec     [esp + loc24.bRightSide]

.L3:
    mcLoadLocal ecx, loc24.nRow
    mcLoadGameDataPtr ebx
    mcLoadMember al, TREKDATA.QUADX
    mcLoadMember dl, TREKDATA.QUADY
    call    TArray_SetStarChartValue
    call    TArray_SetScanned

    call    TConsole_ScrollUp

    mov     cl, ATTR_SCAN_SCALE
    call    TConsole_SetAttr
    mcLoad8bitsToReg32 ecx, 113
    call    TConsole_Prout

.dump_row:
    mov     cl, ATTR_SCAN_SCALE
    call    TConsole_SetAttr

    mcLoadLocal eax, loc24.nRow
    mov     cl, 3
    call    TConsole_CramIntWidth

    mov     al, CHAR_BLANK
    call    TConsole_PutChar

    mcLoadLocal eax, loc24.nRow
    call    TSrScan_CramSpaceRow

    cmp     [esp + loc24.bRightSide], 0
    je      .next_row

    mcLoadLocal eax, loc24.nRow
    call    TSrScan_CramStatusRow

.next_row:
    call    TConsole_ScrollUp

    inc     [esp + loc24.nRow]
    cmp     [esp + loc24.nRow], 10
    jbe     .dump_row

    mov     cl, ATTR_SCAN_SCALE
    call    TConsole_SetAttr
    mcLoad8bitsToReg32 ecx, 113
    call    TConsole_Prout
    jmp     .done

.not_available:
    call    TConsole_SetGameMsgAttr
    call    TConsole_ScrollUp
    mcLoad8bitsToReg32 ecx, 114
    call    TConsole_Prout

.done:
    call    TConsole_ScrollUp
    mcEndLocals loc24.size
    ret

; --- EOF ---
