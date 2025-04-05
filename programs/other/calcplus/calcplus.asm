; SPDX-License-Identifier: GPL-2.0-only
;
; Calc+ - Multifunctional calculator
; Copyright (C) 2014-2025 KolibriOS team
;
; Contributor eAndrew - Main code
; Contributor Leency  - Code refactoring and UX/UI update
; Contributor Burer   - Code refactoring and UI update

; ================================================================

use32
org     0

db      'MENUET01'
dd      1
dd      START
dd      I_END
dd      MEM
dd      STACKTOP
dd      0, 0

; ====================================================================

include "../../proc32.inc"
include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../encoding.inc"
include "../../dll.inc"
include "../../develop/libraries/box_lib/trunk/box_lib.mac"
include "parser.inc"

imports:

        library gui,    "box_lib.obj"
        import  gui,    editbox.draw,   "edit_box", \
                        editbox.key,    "edit_box_key", \
                        editbox.mouse,  "edit_box_mouse"

; ====================================================================

LIST_X          = 8
LIST_Y          = 47
LIST_ITEM_H     = 26
LIST_ITEM_W     = 295
LIST_ITEM_COUNT = 8
LIST_H          = LIST_ITEM_H * LIST_ITEM_COUNT
LIST_TEXT_Y     = LIST_ITEM_H / 2 - 8

KEYB_BTN_W      = 37
KEYB_BTN_H      = 37
GAP             = 5

KEYBOARD_X      = LIST_ITEM_W + 16
KEYBOARD_Y      = LIST_Y
KEYBOARD_W      = 190

LIST_ITEM_TEXT1 equ dword [sc.work_text]
LIST_ITEM_TEXT2 equ dword [sc.work_text]

WIN_X           = 200
WIN_Y           = 200
WIN_W           = LIST_ITEM_W + KEYBOARD_W + 7
WIN_H           = LIST_H + 60

sz_head         db "Calc+", 0
btn_clr         db ""
buttons         db "|%^*/-+)(=7894561230"
edb1            edit_box 0, 11, 12, 0xFFFFFF, 0x94AECE, 0xFFC90E, 0xCACACA, 0x10000000, \
                         480, exp, group, ed_always_focus + ed_focus, 0, 0

; ====================================================================

START:
        mov     [ans.buffer], dword "= 0"
        mov     [ans.size], 3 * 8 + 9

        mcall   SF_SET_EVENTS_MASK, EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE

        m2m     [edb1.color], [scn.gui_face]

        mcall   SF_SYS_MISC, SSF_HEAP_INIT
        stdcall dll.Load, imports

; ====================================================================

update:
        mcall   SF_WAIT_EVENT_TIMEOUT, 5

        cmp     eax, EV_REDRAW
        je      ev_redraw

        cmp     eax, EV_KEY
        je      ev_key

        cmp     eax, EV_BUTTON
        je      ev_button

        cmp     eax, EV_MOUSE
        je      ev_mouse

        jmp     update

; ====================================================================

ev_redraw:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors

        ; WINDOW
        mov     edx, [sc.work]
        or      edx, 0x34 shl 24

        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
        mov     ecx, WIN_Y shl 16 + WIN_H
        add     ecx, eax

        mcall   SF_GET_SCREEN_SIZE
        shr     eax, 16
        mov     ebx, eax
        sub     ebx, WIN_W
        shr     ebx, 1

        mcall   0, <ebx, WIN_W>, , , , sz_head

        ; CONTENT
        call    draw_textbox
        call    draw_keyb
        call    draw_list

        mcall   SF_REDRAW, SSF_END_DRAW

        jmp     update

; ====================================================================

ev_key:
        mcall   SF_GET_KEY

        cmp     ah, 27
        je      exit

        cmp     ah, 13
        je      calc

        invoke  editbox.key, edb1

        jmp     update

; ====================================================================

ev_button:
        mcall   SF_GET_BUTTON

        ; EXIT
        cmp     ah, 1
        je      exit

        cmp     ah, 250
        jne     @f
        mcall   SF_FILE, f70_calc
        jmp     update

        ; DELETE
        @@:
        cmp     ah, 4
        jne     .not_del

        cmp     [edb1.pos], 0
        je      update
        mov     eax, exp
        add     eax, [edb1.pos]
        dec     eax
        mov     ebx, exp
        add     ebx, [edb1.size]
        inc     ebx

        @@:
        cmp     eax, ebx
        je      @f
        mov     cl, [eax + 1]
        mov     [eax], cl
        inc     eax
        jmp     @b
        
        @@:
        dec     [edb1.pos]
        dec     [edb1.size]
        m2m     [edb1.shift], [edb1.pos]
        jmp     .redraw
        
        
        .not_del:

        ; CALCULATE
        cmp     ah, 19
        je      calc

        ; LIST
        cmp     ah, 0x60
        jl      .not_list

        sub     ah, 0x60
        movzx   ebx, ah
        imul    ebx, 512
        add     ebx, history
        add     ebx, 482
        stdcall str_len, ebx

        mov     edx, exp
        add     edx, [edb1.size]
        add     edx, eax
        mov     edi, exp
        add     edi, [edb1.pos]

        @@:
        cmp     edx, edi
        je      @f
        mov     esi, edx
        sub     esi, eax
        push    eax
        mov     al, [esi]
        mov     [edx], al
        pop     eax
        dec     edx
        jmp     @b

        @@:
        add     [edb1.size], eax

        @@:
        cmp     eax, 0
        je      @f
        mov     cl, [ebx]
        mov     [edi], cl
        inc     edi
        inc     ebx
        dec     eax
        inc     [edb1.pos]
        jmp     @b
        
        @@:
        jmp     .redraw
        
        .not_list:

        ; KEYBOARD
        cmp     ah, 10
        jl      update
        cmp     ah, 50
        jg      update
        mov     dh, ah

        mov     ebx, exp
        add     ebx, [edb1.size]
        mov     ecx, exp
        add     ecx, [edb1.pos]
        cmp     dh, 30
        jl      @f
        add     ebx, 2
  
        @@:
        cmp     ebx, ecx
        je      @f
        mov     dl, [ebx - 1]
        mov     [ebx], dl
        dec     ebx
        jmp     @b

        @@:
        movzx   eax, dh
        add     eax, buttons
        sub     eax, 10
        mov     al, [eax]

        mov     [ebx], al
        inc     [edb1.size]
        inc     [edb1.pos]
        cmp     dh, 30
        jl      @f
        add     [edb1.size], 2
        add     [edb1.pos], 2

        @@:
        .redraw:
        call    draw_textbox
        jmp     update

; ====================================================================

 ev_mouse:
        mcall   SF_GET_KEY

        invoke  editbox.mouse, edb1
        jmp     update

; ====================================================================

exit:
        mcall   SF_TERMINATE_PROCESS

; ====================================================================

calc:
        stdcall parse
        cmp     [error_n], 0
        jne     .error

        mov     [ans.buffer], word " = "

        stdcall convert_to_str, eax, ans.buffer + 2
        add     eax, 2
        mov     edi, eax
        imul    eax, 8                          ; char_w
        add     eax, 9
        mov     [ans.size], eax

        ; HISTORY
        mov     ecx, LIST_ITEM_COUNT - 1
        mov     eax, history
        add     eax, (LIST_ITEM_COUNT - 1) * 512

        @@:
        mov     ebx, eax
        sub     ebx, 512
        stdcall str_cpy, ebx, eax
        add     ebx, 480
        add     eax, 480
        stdcall str_cpy, ebx, eax
        sub     ebx, 480
        sub     eax, 480
        mov     esi, [ebx + 508]
        mov     [eax + 508], esi
        sub     eax, 512
        loop    @b

        stdcall str_cpy, exp, history
        stdcall str_cpy, ans.buffer, history + 480
        mov     esi, [ans.size]
        mov     dword[history + 508], esi

        ; Check length
        mov     esi, 37
        sub     esi, edi
        stdcall str_len, exp
        sub     esi, eax
        cmp     esi, 0
        jg      .redraw

        mov     ebx, history
        add     ebx, eax
        add     ebx, esi
        mov     [ebx], dword ".."

        jmp     .redraw

        ; ERRORS
        .error:
        cmp     [error_n], 1
        je      .err_1
        cmp     [error_n], 4
        je      .err_4

        mov     [ans.buffer +  0], dword "Expe"
        mov     [ans.buffer +  4], dword "cted"
        mov     [ans.buffer +  8], dword " ')'"
        mov     [ans.buffer + 12], byte 0
        mov     [ans.size], 105

        cmp     [error_n], 2
        je      .redraw
        cmp     [error_n], 3
        je      .err_3
        cmp     [error_n], 5
        je      .err_5
        
        .err_1:
                mov     [ans.buffer +  0], dword "Div."
                mov     [ans.buffer +  4], dword " by "
                mov     [ans.buffer +  8], byte  "0"
                mov     [ans.buffer +  9], byte 0
                mov     [ans.size], 81
                jmp     .redraw

        .err_4:
                mov     [ans.buffer +  0], dword "Inpu"
                mov     [ans.buffer +  4], dword "t er"
                mov     [ans.buffer +  8], dword "rror"
                mov     [ans.buffer + 12], byte 0
                mov     [ans.size], 105
                jmp     .redraw

        .err_3:
                mov     [ans.buffer + 10], byte "("
                jmp     .redraw

        .err_5:
                mov     [ans.buffer + 10], byte "|"
                jmp     .redraw

        .redraw:

        call    draw_textbox
        call    draw_list
        jmp     update

; ====================================================================

proc draw_textbox

        ; border
        mcall   SF_DRAW_RECT, <LIST_X, LIST_ITEM_W>, <  8,  30>, [sc.work_graph]
    
        ; background
        mov     edx, [scn.gui_face]
        cmp     [error_n], 0
        je      @f
        mov     edx, 0xFFAAAA

        @@:
        mcall   SF_DRAW_RECT, <LIST_X + 1, LIST_ITEM_W - 2>, <  9,  28>
        mcall               , <LIST_X + 1, LIST_ITEM_W - 2>, <  9,   1>, [scn.gui_tb_in_shd]
        mcall               , <LIST_X + 1, 1              >, < 10,  27>

        mov     ebx, LIST_X + LIST_ITEM_W - 9
        sub     ebx, [ans.size]
        shl     ebx, 16
        add     ebx, 16
        mov     ecx, [scn.gui_intext]
        add     ecx, 0x10000000
        or      ecx, 1 shl 31
        mcall   SF_DRAW_TEXT, , , ans.buffer

        mcall               , <LIST_ITEM_W - 8, 19>, [scn.gui_intext], btn_clr, 1
        add     ebx, 1 shl 16
        mcall

        mcall   SF_DEFINE_BUTTON, <LIST_ITEM_W - 14, 17>, <9, 27>, 4 + BT_HIDE

        mov     ebx, LIST_ITEM_W - 18
        sub     ebx, [ans.size]
        cmp     ebx, 24
        jg      @f
        mov     ebx, 24
  
        @@:
        mov     [edb1.width], ebx
        m2m     [edb1.color]             , [scn.gui_face]
        m2m     [edb1.focus_border_color], [scn.gui_face]
        cmp     [error_n], 0
        je      @f
        mov     [edb1.color]             , 0x00FFAAAA
        mov     [edb1.focus_border_color], 0x00FFAAAA

        @@:
        invoke  editbox.draw, edb1

        ret

endp

; ====================================================================

proc draw_button, x, y

        mcall   SF_DEFINE_BUTTON, <[x], KEYB_BTN_W>, <[y], KEYB_BTN_H>, [but_id], [but_c]

        mov     ebx, [x]
        add     ebx, KEYB_BTN_W/2-4

        shl     ebx, 16
        add     ebx, [y]
        add     ebx, KEYB_BTN_H/2 - 6
        or      [but_tc], 0x01000000                            ;use bigger font
        mcall   SF_DRAW_TEXT, , [but_tc], [txt_id], [txt_size]

        mov     eax, [txt_size]
        add     [txt_id], eax
        inc     dword [but_id]

        ret

endp

; ====================================================================

proc draw_keyb

        mcall   SF_DEFINE_BUTTON, <KEYBOARD_X, (KEYB_BTN_W + GAP)*4 - GAP>, <8, 29>, 250, [sc.work_light]
        mov     ecx, [sc.work_text]
        or      ecx, 0x10000000
        mcall   SF_DRAW_TEXT, <(KEYB_BTN_W + GAP - engineering_len*2)*2 - GAP/2 + KEYBOARD_X, KEYB_BTN_H / 2 - 10 + 8>, , engineering_str, engineering_len

        mov     [txt_size], 1
        mov     [but_id], 0x0000000A
        mov     [txt_id], buttons

        m2m     [but_c], [sc.work]
        m2m     [but_tc], [sc.work_text]
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*0, KEYBOARD_Y
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*1, KEYBOARD_Y
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*2, KEYBOARD_Y
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*3, KEYBOARD_Y
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*3, KEYBOARD_Y + (KEYB_BTN_H + GAP)*1
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*3, KEYBOARD_Y + (KEYB_BTN_H + GAP)*2
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*3, KEYBOARD_Y + (KEYB_BTN_H + GAP)*3
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*2, KEYBOARD_Y + (KEYB_BTN_H + GAP)*4
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*1, KEYBOARD_Y + (KEYB_BTN_H + GAP)*4

        mov     [but_c] , 0x00F0969D
        mov     [but_tc], 0x00FFFFFF
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*3, KEYBOARD_Y +(KEYB_BTN_H + GAP)*4

        m2m     [but_c] , [sc.work_button]
        m2m     [but_tc], [sc.work_button_text]
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*0, KEYBOARD_Y + KEYB_BTN_H + GAP
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*1, KEYBOARD_Y + KEYB_BTN_H + GAP
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*2, KEYBOARD_Y + KEYB_BTN_H + GAP
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*0, KEYBOARD_Y + (KEYB_BTN_H + GAP)*2
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*1, KEYBOARD_Y + (KEYB_BTN_H + GAP)*2
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*2, KEYBOARD_Y + (KEYB_BTN_H + GAP)*2
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*0, KEYBOARD_Y + (KEYB_BTN_H + GAP)*3
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*1, KEYBOARD_Y + (KEYB_BTN_H + GAP)*3
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*2, KEYBOARD_Y + (KEYB_BTN_H + GAP)*3
        stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*0, KEYBOARD_Y + (KEYB_BTN_H + GAP)*4

        ret

endp

; ====================================================================

proc draw_list

        ; BACKGROUND
        mov     eax, SF_DRAW_RECT
        mov     ebx, LIST_X shl 16 + LIST_ITEM_W
        mov     ecx, LIST_Y shl 16 + LIST_ITEM_H
        mov     edx, [sc.work_light]
        mov     edi, LIST_ITEM_COUNT
        
        @@:
        mcall
        ; draw separator {
        push    ecx edx
        sub     ecx, LIST_ITEM_H - 1
        mov     edx, [sc.work_dark]
        mcall
        ; }
        pop     edx ecx
        add     ecx, LIST_ITEM_H shl 16
        
        .next:
        dec     edi
        cmp     edi, 0
        jne     @b

        mcall     , <LIST_X                  , 1>, <LIST_Y, LIST_ITEM_H * 8>, [sc.work_dark]
        mcall     , <LIST_X + LIST_ITEM_W - 1, 1>,
        mcall     , <LIST_X, LIST_ITEM_W>, <LIST_Y + LIST_ITEM_H * 8, 1>

        ; BUTTONS
        mov     eax, SF_DEFINE_BUTTON
        mov     ebx, LIST_X shl 16 + LIST_ITEM_W
        mov     ecx, 43 shl 16 + LIST_ITEM_H
        mov     edx, 60 + BT_HIDE
        mov     edi, LIST_ITEM_COUNT

        @@:
        mcall
        add     ecx, LIST_ITEM_H shl 16
        inc     edx
        dec     edi
        cmp     edi, 0
        jne     @b

        ; TEXT
        mov     eax, SF_DRAW_TEXT
        mov     ebx, (LIST_X + LIST_X) shl 16 + LIST_Y + LIST_TEXT_Y + 1
        mov     ecx, [sc.work_text]
        mov     edi, LIST_ITEM_COUNT
  
        @@:
        or      ecx, 1 shl 31
        mov     edx, history

        @@:
        add     ecx, 0x10000000
        mcall

        push    ebx
        add     ebx, (LIST_ITEM_W - 8) shl 16
        mov     esi, [edx + 508]
        shl     esi, 16
        sub     ebx, esi
        add     edx, 480
        mcall
        pop     ebx

        add     edx, 32
        add     ebx, LIST_ITEM_H

        and     ecx, 0xFFFFFF
        cmp     ecx, LIST_ITEM_TEXT2
        je      .set_color_to_2_txt
        mov     ecx, LIST_ITEM_TEXT2
        jmp     .next_txt

        .set_color_to_2_txt:
                mov     ecx, LIST_ITEM_TEXT1

        .next_txt:
                or      ecx, 1 shl 31

        dec     edi
        cmp     edi, 0
        jne     @b

        ret

endp

; ====================================================================

proc str_len uses ebx, str

        xor     eax, eax
        mov     ebx, [str]
        
        @@:
        cmp     [ebx], byte 0
        je      @f
        inc     eax
        inc     ebx
        jmp     @b
        
        @@:

        ret

endp

; ====================================================================

proc str_cpy uses eax ebx ecx, from, to

        mov     eax, [from]
        mov     ebx, [to]

        @@:
        cmp     [eax], byte 0
        je      @f
        mov     cl, [eax]
        mov     [ebx], cl
        inc     eax
        inc     ebx
        jmp     @b
        
        @@:
        mov     [ebx], byte 0
        ret

endp

; ====================================================================

if lang eq ru_RU

        engineering_str db      "Engineering calc"
        engineering_len         = 16

else if lang eq es_ES

        engineering_str db      " Modo ingeniero "
        engineering_len         = 16
else

        engineering_str cp866   "Инженерный режим"
        engineering_len         = 16

endf

; ====================================================================

I_END:

rb 2048

STACKTOP:

exp             rb 480
exp_pos         rd 1
exp_lvl         rd 1
abs_lvl         rd 1
group           rd 1

ans.buffer:     rb 480
ans.size        rd 1
error_n         rd 1

timer           rd 1
but_id          rd 1
but_c           rd 1
but_tc          rd 1
txt_id          rd 1
txt_size        rd 1

history         rb 512 * LIST_ITEM_COUNT
                rb 512

struc system_colors_internal {
        .gui_tb_in_shd  dd 0x00CED0D0
        .gui_face       dd 0x00FAF8FA
        .gui_text       dd 0x10373C42
        .gui_intext     dd 0x005F5F5F
        .gui_select     dd 0x00C7C9C9
}

scn             system_colors_internal
sc              system_colors

f70_calc:
        dd      7
        dd      0
        dd      0
        dd      0
        dd      0
        db      '/sys/calc', 0

MEM:
