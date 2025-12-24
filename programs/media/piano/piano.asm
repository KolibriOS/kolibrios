; SPDX-License-Identifier: GPL-2.0-only
;
; Piano - Toy Piano
; Copyright (C) 2019-2025 KolibriOS team
;
; Contributor ???       - Initial code
; Contributor Antonio   - Refactoring and new functionality
; Contributor Burer     - Refactoring and Spanish translation
; Contributor qullarwee - New keyboard layout

; ====================================================================

use32
org 0

; ====================================================================

db      'MENUET01'
dd      1
dd      START
dd      I_END
dd      MEM
dd      STACKTOP
dd      0
dd      0

; ====================================================================

include '../../macros.inc'
include "../../KOSfuncs.inc"
include "../../encoding.inc"

; ====================================================================

START:

        mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1
        jmp     redraw

; ====================================================================

still:

        mcall   SF_WAIT_EVENT

        cmp     al, 1
        je      redraw
        cmp     al, 2
        je      key
        cmp     al, 3
        je      button

        jmp     still

; ====================================================================

key:

        mcall   SF_GET_KEY

        ; drop keys release
        test    ah, 0x80
        jnz     still

        ; open help window on Escape
        cmp     ah, KEY_ESC
        je      .help

        ; check if Shift is pressed
        ; increase note_id by 0x40
        mov     bl, ah
        mcall   SF_KEYBOARD, SSF_GET_CONTROL_KEYS
        xor     dl, dl
        test    al, KEY_SHIFT
        jz      .key_map_setup
        mov     dl, INC_SHIFT

        ; loop to find note_id by key_scancode
        .key_map_setup:

        mov     edi, key_note_map

        .key_note_loop:

                cmp     edi, key_note_map_end
                jae     still
                cmp     bl, [edi]
                jne     .key_note_next
                mov     ah, [edi + 1]
                add     ah, dl
                jmp     play

        .key_note_next:

                add     edi, KM_SIZE
                jmp     .key_note_loop

        ; help window thread creation
        .help:

                cmp     [hp_tid], 0
                jne     still
                mcall   SF_CREATE_THREAD, 1, help_thread, help_stack_top
                cmp     eax, -1
                je      still
                mov     [hp_tid], eax
                jmp     still

button:

        mcall   SF_GET_BUTTON

        cmp     ah, 1
        je      .exit

        ; note_id is always button_id - 0x10
        sub     ah, 0x10
        jmp     play

        .exit:
                mcall   SF_TERMINATE_PROCESS

; =======================================================================

play:

        mov     [melody + 1], ah
        mcall   SF_SPEAKER_PLAY, SF_SPEAKER_PLAY, , , , melody

        jmp     still

; =======================================================================

redraw:

        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS,  sc, sizeof.system_colors
        mcall                    , SSF_GET_SKIN_HEIGHT,

        mov     ecx, eax
        add     ecx, WN_MAIN.Y shl 16 + WN_MAIN.H
        mov     edx, [sc.work]
        or      edx, 0x34000000
        mcall   SF_CREATE_WINDOW, <WN_MAIN.X, WN_MAIN.W>, , , , cp_main

        ; white buttons rows
        mov     eax, SF_DEFINE_BUTTON                   ; constant syscall number
        mov     ebx, BT_WHITE.X shl 16 + BT_WHITE.W     ; packed <x, width>, x starts at 0
        mov     ecx, BT_WHITE.Y shl 16 + BT_WHITE.H     ; packed <y, height>, constant
        mov     edi, white_top_buttons
        mov     ebp, white_bottom_buttons

        .white_buttons_loop:

                cmp     edi, white_top_buttons_end
                jae     .black_buttons_setup

                mov     edx, [edi]
                mov     esi, [edi + 4]
                mcall

                mov     ecx, (BT_WHITE.Y + BT_WHITE.H) shl 16 + BT_WHITE.H
                mov     edx, [ebp]
                mov     esi, [ebp + 4]
                mcall

                mov     ecx, BT_WHITE.Y shl 16 + BT_WHITE.H
                add     ebx, BT_WHITE.W shl 16
                add     edi, DT_SIZE
                add     ebp, DT_SIZE
                jmp     .white_buttons_loop

        ; black buttons rows
        .black_buttons_setup:

        mov     esi, 0x00221100         ; constant color
        mov     edi, black_buttons

        .black_buttons_loop:

                cmp     edi, black_buttons_end
                jae     .black_buttons_loop_end

                movzx   ebx, byte [edi]
                imul    ebx, BT_WHITE.W
                add     ebx, BT_BLACK.X
                shl     ebx, 16
                mov      bx, BT_BLACK.W

                movzx   edx, byte [edi + 1]

                mov     ecx, (BT_WHITE.Y + BT_BLACK.Y) shl 16 + BT_BLACK.H              ; top row
                add     edx, 0x20
                mcall

                mov     ecx, (BT_WHITE.Y + BT_WHITE.H + BT_BLACK.Y) shl 16 + BT_BLACK.H ; bottom row
                sub     edx, 0x20
                mcall

                add     edi, BL_SIZE
                jmp     .black_buttons_loop

        .black_buttons_loop_end:

        ; notes labels
        mcall   SF_DRAW_TEXT, <RT_NOTES.X, RT_NOTES.Y>, 0x90FFFFFF, lb_notes

        mcall   SF_REDRAW, SSF_END_DRAW

        jmp     still

; =======================================================================

help_thread:

        jmp    .help_redraw

        .help_still:

                mcall   SF_WAIT_EVENT

                cmp     al, 1
                je      .help_redraw
                cmp     al, 3
                je      .help_button

                jmp     .help_still

        .help_button:

                mcall   SF_GET_BUTTON
                cmp     ah, 1
                jne     .help_still

                mov     dword [hp_tid], 0
                mcall   SF_TERMINATE_PROCESS

        .help_redraw:

                mcall   SF_REDRAW, SSF_BEGIN_DRAW
                mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS,  sc, sizeof.system_colors
                mcall                    , SSF_GET_SKIN_HEIGHT,

                mov     ecx, eax
                add     ecx, WN_HELP.Y shl 16 + WN_HELP.H
                mov     edx, [sc.work]
                or      edx, 0x34000000
                mcall   SF_CREATE_WINDOW, <WN_HELP.X, WN_HELP.W>, , , , cp_help

                mov     eax, SF_DRAW_TEXT
                mov     ecx, [sc.work_text]
                or      ecx, 0x90000000
                mov     ebx, RT_HELP.X shl 16 + RT_HELP.Y
                mov     edi, help_texts

        ; draw all help text in rows
        .help_text_loop:

                cmp     edi, help_texts_end
                jae     .help_text_loop_done

                mov     edx, [edi]
                mcall

                add     ebx, RT_HELP.H
                add     edi, 4
                jmp     .help_text_loop

        .help_text_loop_done:

                mcall   SF_REDRAW, SSF_END_DRAW
                jmp     help_thread.help_still

; =======================================================================

sc              system_colors

WN_MAIN         RECT   32, 32, BT_WHITE.W * 15 + 10, BT_WHITE.H * 2 + 9

BT_WHITE        RECT    0,  4,  48, 100
BT_BLACK        RECT   33,  0,  30,  50

RT_HELP         RECT    8,  8,   0,  24

KEY_ESC         = 0x01
KEY_SHIFT       = 0x03
INC_SHIFT       = 0x40

; =======================================================================

if lang eq ru_RU

        cp_main  cp866  "Детское Пианино [Escape - Открыть Справку]", 0
        cp_help  cp866  "Детское Пианино - Справка", 0
        lb_help1 cp866  "Клавиши от 1 до = и от Q до ] - верхний ряд.", 0
        lb_help2 cp866  "Клавиши от A до Enter и от Z до /, \ и Backspace - нижний ряд.", 0
        lb_help3 cp866  "Удерживайте Shift для повышения октавы на 4.", 0
        lb_help4 cp866  "Должен звучать встроенный динамик ПК (не колонки).", 0
        lb_notes cp866  "ДО    РЕ    МИ    ФА   СОЛЬ   ЛЯ    СИ    ДО", 0

        WN_HELP  RECT   64,  64, 521, 106
        RT_NOTES RECT   16, 183,   0,   0

else if lang eq es_ES

        cp_main  db     "Piano de Juguete [Escape - Abrir Ayuda]", 0
        cp_help  db     "Piano de Juguete - Ayuda", 0
        lb_help1 db     "Teclas de 1 a = y de Q a ] - fila superior.", 0
        lb_help2 db     "Teclas de A a Enter y de Z a /, \ y Backspace - fila inferior.", 0
        lb_help3 db     "Mantenga Shift para subir 4 octavas.", 0
        lb_help4 db     "Deberias oir el altavoz interno del PC (no los externos).", 0
        lb_notes db     "DO    RE    MI    FA    SOL   LA    SI    DO", 0

        WN_HELP  RECT   64,  64, 521, 106
        RT_NOTES RECT   16, 183,   0,   0

else

        cp_main  db     "Toy Piano [Escape - Open Help]", 0
        cp_help  db     "Toy Piano - Help", 0
        lb_help1 db     "Keys from 1 to = and from Q to ] - top row.", 0
        lb_help2 db     "Keys from A to Enter and from Z to /, \ and Backspace - bottom row.", 0
        lb_help3 db     "Hold Shift to raise the octave by 4.", 0
        lb_help4 db     "You should hear the built-in PC speaker (not external speakers).", 0
        lb_notes db     "C     D     E     F     G     A     B     C", 0

        WN_HELP  RECT   64,  64, 561, 106
        RT_NOTES RECT   20, 183,   0,   0

end if

; =======================================================================

DT_SIZE =  8 ; white buttons entry size
BL_SIZE =  2 ; black buttons entry size

; button id and color
white_top_buttons:
        dd 0x31, 0xFF7A74
        dd 0x33, 0x907040
        dd 0x35, 0xA08050
        dd 0x36, 0xB09060
        dd 0x38, 0xC0A070
        dd 0x3A, 0xD0B080
        dd 0x3C, 0xE0C090
        dd 0x41, 0xFFA97C
        dd 0x43, 0xAF8D8D
        dd 0x45, 0xBF9D9D
        dd 0x46, 0xCFADAD
        dd 0x48, 0xDFBDBD
        dd 0x4A, 0xEFCDCD
        dd 0x4C, 0xFFDDDD
        dd 0x51, 0xFFE558
white_top_buttons_end:

; button id and color
white_bottom_buttons:
        dd 0x11, 0x702050
        dd 0x13, 0x683638
        dd 0x15, 0x784648
        dd 0x16, 0x885658
        dd 0x18, 0x986668
        dd 0x1A, 0xA87678
        dd 0x1C, 0xB88688
        dd 0x21, 0x880040
        dd 0x23, 0x90622B
        dd 0x25, 0xA0723B
        dd 0x26, 0xB0824B
        dd 0x28, 0xC0925B
        dd 0x2A, 0xD0A26B
        dd 0x2C, 0xE0B27B
        dd 0x31, 0xFF7A74
white_bottom_buttons_end:

; parent white button id and button id
black_buttons:
        db 0x00, 0x12
        db 0x01, 0x14
        db 0x03, 0x17
        db 0x04, 0x19
        db 0x05, 0x1B
        db 0x07, 0x22
        db 0x08, 0x24
        db 0x0A, 0x27
        db 0x0B, 0x29
        db 0x0C, 0x2B
black_buttons_end:

; corrensponding help string label
help_texts:
        dd lb_help1
        dd lb_help2
        dd lb_help3
        dd lb_help4
help_texts_end:

; =======================================================================

KM_SIZE =  2 ; key map entry size

; key scancode and note id
key_note_map:
        db 0x02, 0x31 ; '1' -> note 0x31
        db 0x03, 0x32 ; '2' -> note 0x32
        db 0x04, 0x33 ; '3' -> note 0x33
        db 0x05, 0x34 ; '4' -> note 0x34
        db 0x06, 0x35 ; '5' -> note 0x35
        db 0x07, 0x36 ; '6' -> note 0x36
        db 0x08, 0x37 ; '7' -> note 0x37
        db 0x09, 0x38 ; '8' -> note 0x38
        db 0x0A, 0x39 ; '9' -> note 0x39
        db 0x0B, 0x3A ; '0' -> note 0x3A
        db 0x0C, 0x3B ; '-' -> note 0x3B
        db 0x0D, 0x3C ; '=' -> note 0x3C

        db 0x10, 0x41 ; 'q' -> note 0x41
        db 0x11, 0x42 ; 'w' -> note 0x42
        db 0x12, 0x43 ; 'e' -> note 0x43
        db 0x13, 0x44 ; 'r' -> note 0x44
        db 0x14, 0x45 ; 't' -> note 0x45
        db 0x15, 0x46 ; 'y' -> note 0x46
        db 0x16, 0x47 ; 'u' -> note 0x47
        db 0x17, 0x48 ; 'i' -> note 0x48
        db 0x18, 0x49 ; 'o' -> note 0x49
        db 0x19, 0x4A ; 'p' -> note 0x4A
        db 0x1A, 0x4B ; '[' -> note 0x4B
        db 0x1B, 0x4C ; ']' -> note 0x4C

        db 0x1E, 0x11 ; 'a' -> note 0x11
        db 0x1F, 0x12 ; 's' -> note 0x12
        db 0x20, 0x13 ; 'd' -> note 0x13
        db 0x21, 0x14 ; 'f' -> note 0x14
        db 0x22, 0x15 ; 'g' -> note 0x15
        db 0x23, 0x16 ; 'h' -> note 0x16
        db 0x24, 0x17 ; 'j' -> note 0x17
        db 0x25, 0x18 ; 'k' -> note 0x18
        db 0x26, 0x19 ; 'l' -> note 0x19
        db 0x27, 0x1A ; ';' -> note 0x1A
        db 0x28, 0x1B ; ''' -> note 0x1B
        db 0x1C, 0x1C ; Enter -> note 0x1C

        db 0x2C, 0x21 ; 'z' -> note 0x21
        db 0x2D, 0x22 ; 'x' -> note 0x22
        db 0x2E, 0x23 ; 'c' -> note 0x23
        db 0x2F, 0x24 ; 'v' -> note 0x24
        db 0x30, 0x25 ; 'b' -> note 0x25
        db 0x31, 0x26 ; 'n' -> note 0x26
        db 0x32, 0x27 ; 'm' -> note 0x27
        db 0x33, 0x28 ; ',' -> note 0x28
        db 0x34, 0x29 ; '.' -> note 0x29
        db 0x35, 0x2A ; '/' -> note 0x2A
        db 0x2B, 0x2B ; '\' -> note 0x2B
        db 0x0E, 0x2C ; Backspace -> note 0x2C
key_note_map_end:

; =======================================================================

melody  db 0x90, 0x30, 0
hp_tid  dd 0

; =======================================================================

I_END:
help_stack:
        rb 512
help_stack_top:
        rb 1024
        align 512
STACKTOP:
MEM:
