; SPDX-License-Identifier: GPL-2.0-only
;
; Piano - Toy Piano
; Copyright (C) 2019-2025 KolibriOS team
;
; Authors: Antonio and Leency

; ====================================================================

use32
org 0

; ====================================================================

db      "MENUET01"
dd      1
dd      START
dd      I_END
dd      MEM
dd      STACKTOP
dd      0
dd      0

; ====================================================================

include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../encoding.inc"

; ====================================================================

START:
        mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1
        call    load_keymap
        jmp     redraw

; ====================================================================

still:
        mcall   SF_WAIT_EVENT

        cmp     al, EV_REDRAW
        je      redraw
        cmp     al, EV_KEY
        je      key
        cmp     al, EV_BUTTON
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

        ; if Shift is pressed - increase note_id by 0x40
        movzx   ecx, ah
        mcall   SF_KEYBOARD, SSF_GET_CONTROL_KEYS
        xor     edx, edx

        test    al, KEY_SHIFT
        jz      .key_map_play

        mov     dl, INC_SHIFT

        ; play note if valid key pressed
        .key_map_play:
                mov     ah, [keymap + ecx]
                cmp     ah, BAD_NOTE
                je      still

                add     ah, dl
                jmp     play

        ; help window thread creation
        .help:
                cmp     [hw_tid], 0
                jne     still

                sub     esp, 1024
                mov     ebx, esp
                mcall   SF_THREAD_INFO, ebx, -1
                mov     eax, [esp + process_information.box.left]
                mov     edx, [esp + process_information.box.top]
                add     esp, 1024
                shl     eax, 16
                mov      ax, dx
                mov     [mw_pos], eax

                mcall   SF_CREATE_THREAD, 1, help_thread, help_stack_top
                cmp     eax, -1
                je      still

                mov     [hw_tid], eax
                jmp     still

button:
        mcall   SF_GET_BUTTON

        cmp     ah, 1
        je      .exit

        ; note_id is always button_id - 0x10
        sub     ah, 0x10
        mov     dl, ah

        ; if Shift is pressed - increase note_id by 0x40
        mcall   SF_KEYBOARD, SSF_GET_CONTROL_KEYS
        test    al, KEY_SHIFT
        jz      .no_shift
        add     dl, INC_SHIFT

        .no_shift:
                mov     ah, dl
                jmp     play

.exit:
        mcall   SF_TERMINATE_PROCESS

; =======================================================================

play:
        mov     [melody + 1], ah
        mcall   SF_SPEAKER_PLAY, SF_SPEAKER_PLAY, , , melody

        jmp     still

; =======================================================================

load_keymap:
        call    load_default_keymap

        mcall   SF_FILE, keymap_file
        test    eax, eax
        jnz     .notify
        cmp     ebx, 256
        je      .done
        .notify:
        call    load_default_keymap
        mcall   SF_FILE, notify_info
        .done:
        ret

load_default_keymap:
        mov     esi, default_keymap
        mov     edi, keymap
        mov     ecx, 256
        rep     movsb
        ret

; =======================================================================

redraw:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS,  sc, sizeof.system_colors
        mcall                    , SSF_GET_SKIN_HEIGHT,

        mov     ecx, eax
        add     ecx, WN_MAIN.Y shl 16 + WN_MAIN.H
        mov     edx, [sc.work]
        or      edx, 0x34000000 ; skin, no resize, caption, client area
        mcall   SF_CREATE_WINDOW, <WN_MAIN.X, WN_MAIN.W>, , , , cp_main

        ; white buttons rows
        mov     eax, SF_DEFINE_BUTTON                   ; constant syscall number
        mov     ebx, BT_WHITE.X shl 16 + BT_WHITE.W     ; packed <x, width>, x starts at 0
        mov     ecx, BT_WHITE.Y shl 16 + BT_WHITE.H     ; packed <y, height>, constant
        mov     edi, white_button_colors
        mov     ebp, white_button_ids

        .white_buttons_loop:
                cmp     edi, end_white_button_colors
                jae     .black_buttons_setup

                movzx   edx, byte[ebp]
                mov     esi, [edi]
                mcall

                mov     ecx, (BT_WHITE.Y + BT_WHITE.H) shl 16 + BT_WHITE.H
                movzx   edx, byte[ebp + 1]
                mov     esi, [edi + 4]
                mcall

                mov     ecx, BT_WHITE.Y shl 16 + BT_WHITE.H
                add     ebx, BT_WHITE.W shl 16

                add     edi, WT_SIZE
                add     ebp, 2

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
        mcall   SF_SET_EVENTS_MASK, EVM_REDRAW + EVM_BUTTON
        jmp    .redraw

        .still:
                mcall   SF_WAIT_EVENT

                cmp     al, EV_REDRAW
                je      .redraw
                cmp     al, EV_BUTTON
                je      .button

                jmp     .still

        .button:
                mcall   SF_GET_BUTTON
                cmp     ah, 1
                jne     .still

                mov     dword [hw_tid], 0
                mcall   SF_TERMINATE_PROCESS

        .redraw:
                mcall   SF_REDRAW, SSF_BEGIN_DRAW
                mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS,  sc, sizeof.system_colors
                mcall                    , SSF_GET_SKIN_HEIGHT,

                mov     ebp, eax
                mov     ebx, [mw_pos]
                mov     ecx, ebx
                shr     ebx, 16
                and     ecx, 0xFFFF

                add     ebx, WN_HELP.X
                shl     ebx, 16
                mov      bx, WN_HELP.W

                add     ecx, WN_HELP.Y
                add     ecx, ebp
                shl     ecx, 16
                mov      cx, WN_HELP.H

                mov     edx, [sc.work]
                or      edx, 0x34000000 ; skin, no resize, caption, client area
                mcall   SF_CREATE_WINDOW, , , , , cp_help

                mov     eax, SF_DRAW_TEXT
                mov     ecx, [sc.work_text]
                or      ecx, 0x90000000 ; null terminator, 8x16 OLE
                mov     ebx, RT_HELP.X shl 16 + RT_HELP.Y
                mov     edi, help_texts

        ; draw all help text in rows
        .help_text_loop:
                cmp     edi, help_texts_end
                jae     .help_text_loop_done

                mov     edx, [edi]
                mcall   ; draw text

                add     ebx, RT_HELP.H
                add     edi, 4
                jmp     .help_text_loop

        .help_text_loop_done:
                mcall   SF_REDRAW, SSF_END_DRAW
                jmp     .still

; =======================================================================

sc              system_colors

WN_MAIN         RECT   32, 32, BT_WHITE.W * 15 + 10, BT_WHITE.H * 2 + (5 + 4)

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
        lb_help4 cp866  "Раскладку клавиатуры можно изменить через файл piano.map.", 0
        lb_help5 cp866  "Он должен содержать 256 байт, каждый индекс - скан-код, а значение - нота.", 0
        lb_help6 cp866  "Должен звучать встроенный динамик ПК (не колонки).", 0

        lb_notes cp866  "ДО    РЕ    МИ    ФА   СОЛЬ   ЛЯ    СИ    ДО", 0

        nf_map   cp866  "'Детское Пианино\npiano.map отсутствует или повреждён, используется раскладка по умолчанию' -tW", 0

        WN_HELP  RECT   14,  13, 617, 178
        RT_NOTES RECT   16, 183,   0,   0

else if lang eq es_ES

        cp_main  cp850  "Piano de Juguete [Escape - Abrir Ayuda]", 0
        cp_help  cp850  "Piano de Juguete - Ayuda", 0

        lb_help1 cp850  "Teclas de 1 a = y de Q a ] - fila superior.", 0
        lb_help2 cp850  "Teclas de A a Enter y de Z a /, \ y Backspace - fila inferior.", 0
        lb_help3 cp850  "Mantenga Shift para subir 4 octavas.", 0
        lb_help4 cp850  "La distribución del teclado se puede cambiar mediante el archivo piano.map.", 0
        lb_help5 cp850  "Debe contener 256 bytes: cada índice es un código de escaneo y el valor es una nota.", 0
        lb_help6 cp850  "Deberias oir el altavoz interno del PC (no los externos).", 0

        lb_notes cp850  "DO    RE    MI    FA    SOL   LA    SI    DO", 0

        nf_map   cp850  "'Piano de Juguete\nFalta piano.map o es inválido; se usa el mapa de teclas predeterminado' -tW", 0

        WN_HELP  RECT   14,  13, 697, 178
        RT_NOTES RECT   16, 183,   0,   0

else

        cp_main  db     "Toy Piano [Escape - Open Help]", 0
        cp_help  db     "Toy Piano - Help", 0

        lb_help1 db     "Keys from 1 to = and from Q to ] - top row.", 0
        lb_help2 db     "Keys from A to Enter and from Z to /, \ and Backspace - bottom row.", 0
        lb_help3 db     "Hold Shift to raise the octave by 4.", 0
        lb_help4 db     "Keyboard layout can be changed via the piano.map file.", 0
        lb_help5 db     "It must contain 256 bytes - each index is scancode, and value is note.", 0
        lb_help6 db     "You should hear the built-in PC speaker (not external speakers).", 0

        lb_notes db     "C     D     E     F     G     A     B     C", 0

        nf_map  db     "'Toy Piano\npiano.map is missing or invalid, using default keymap' -tW", 0

        WN_HELP  RECT   14,  13, 585, 178
        RT_NOTES RECT   20, 183,   0,   0

end if

; =======================================================================

WT_SIZE         =  8 ; white buttons entry size
BL_SIZE         =  2 ; black buttons entry size

; white button colors
white_button_colors: ; 1 top, 1 bottom, 2 top, 2 bottom, N top, N bottom
        dd      0xFF7A74, 0x702050
        dd      0x907040, 0x683638
        dd      0xA08050, 0x784648
        dd      0xB09060, 0x885658
        dd      0xC0A070, 0x986668
        dd      0xD0B080, 0xA87678
        dd      0xE0C090, 0xB88688

        dd      0xFFA97C, 0x880040
        dd      0xAF8D8D, 0x90622B
        dd      0xBF9D9D, 0xA0723B
        dd      0xCFADAD, 0xB0824B
        dd      0xDFBDBD, 0xC0925B
        dd      0xEFCDCD, 0xD0A26B
        dd      0xFFDDDD, 0xE0B27B

        dd      0xFFE558, 0xFF7A74
end_white_button_colors:

; white button ids
white_button_ids: ; 1 top, 1 bottom, 2 top, 2 bottom, N top, N bottom
        db 0x31, 0x11
        db 0x33, 0x13
        db 0x35, 0x15
        db 0x36, 0x16
        db 0x38, 0x18
        db 0x3A, 0x1A
        db 0x3C, 0x1C

        db 0x41, 0x21
        db 0x43, 0x23
        db 0x45, 0x25
        db 0x46, 0x26
        db 0x48, 0x28
        db 0x4A, 0x2A
        db 0x4C, 0x2C

        db 0x51, 0x31  ; ?
end_white_button_ids:

; parent white button index in row and black button ids
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

; help string labels
help_texts:
        dd lb_help1
        dd lb_help2
        dd lb_help3
        dd lb_help4
        dd lb_help5
        dd lb_help6
help_texts_end:

; =======================================================================

BAD_NOTE = 0xFF

default_keymap:
        db BAD_NOTE     ; 0x00
        db BAD_NOTE     ; 0x01
        db 0x21         ; 0x02  '1' -> note 0x31
        db 0x22         ; 0x03  '2' -> note 0x32
        db 0x23         ; 0x04  '3' -> note 0x33
        db 0x24         ; 0x05  '4' -> note 0x34
        db 0x25         ; 0x06  '5' -> note 0x35
        db 0x26         ; 0x07  '6' -> note 0x36
        db 0x27         ; 0x08  '7' -> note 0x37
        db 0x28         ; 0x09  '8' -> note 0x38
        db 0x29         ; 0x0A  '9' -> note 0x39
        db 0x2A         ; 0x0B  '0' -> note 0x3A
        db 0x2B         ; 0x0C  '-' -> note 0x3B
        db 0x2C         ; 0x0D  '=' -> note 0x3C
        db 0x1C         ; 0x0E  Backspace -> note 0x2C
        db BAD_NOTE     ; 0x0F

        db 0x31         ; 0x10  'q' -> note 0x41
        db 0x32         ; 0x11  'w' -> note 0x42
        db 0x33         ; 0x12  'e' -> note 0x43
        db 0x34         ; 0x13  'r' -> note 0x44
        db 0x35         ; 0x14  't' -> note 0x45
        db 0x36         ; 0x15  'y' -> note 0x46
        db 0x37         ; 0x16  'u' -> note 0x47
        db 0x38         ; 0x17  'i' -> note 0x48
        db 0x39         ; 0x18  'o' -> note 0x49
        db 0x3A         ; 0x19  'p' -> note 0x4A
        db 0x3B         ; 0x1A  '[' -> note 0x4B
        db 0x3C         ; 0x1B  ']' -> note 0x4C
        db 0x0C         ; 0x1C  Enter -> note 0x1C
        db BAD_NOTE     ; 0x1D

        db 0x01         ; 0x1E  'a' -> note 0x11
        db 0x02         ; 0x1F  's' -> note 0x12
        db 0x03         ; 0x20  'd' -> note 0x13
        db 0x04         ; 0x21  'f' -> note 0x14
        db 0x05         ; 0x22  'g' -> note 0x15
        db 0x06         ; 0x23  'h' -> note 0x16
        db 0x07         ; 0x24  'j' -> note 0x17
        db 0x08         ; 0x25  'k' -> note 0x18
        db 0x09         ; 0x26  'l' -> note 0x19
        db 0x0A         ; 0x27  ';' -> note 0x1A
        db 0x0B         ; 0x28  ''' -> note 0x1B
        db BAD_NOTE     ; 0x29
        db BAD_NOTE     ; 0x2A

        db 0x1B         ; 0x2B  '\' -> note 0x2B
        db 0x11         ; 0x2C  'z' -> note 0x21
        db 0x12         ; 0x2D  'x' -> note 0x22
        db 0x13         ; 0x2E  'c' -> note 0x23
        db 0x14         ; 0x2F  'v' -> note 0x24
        db 0x15         ; 0x30  'b' -> note 0x25
        db 0x16         ; 0x31  'n' -> note 0x26
        db 0x17         ; 0x32  'm' -> note 0x27
        db 0x18         ; 0x33  ',' -> note 0x28
        db 0x19         ; 0x34  '.' -> note 0x29
        db 0x1A         ; 0x35  '/' -> note 0x2A

        times 256-($-default_keymap) db BAD_NOTE

keymap_file:
        dd SSF_READ_FILE
        dd 0
        dd 0
        dd 256
        dd keymap
        db 'piano.map', 0

; =======================================================================

melody  db 0x90, 0x30, 0
keymap  rb 256

mw_pos  dd 0
hw_tid  dd 0

; =======================================================================

notify_info:
        dd SSF_START_APP
        dd 0
        dd nf_map
        dd 0
        dd 0
        db '/sys/@notify', 0

; =======================================================================

I_END:
help_stack:
        rb 512
help_stack_top:
        rb 1024
        align 512
STACKTOP:
MEM:
