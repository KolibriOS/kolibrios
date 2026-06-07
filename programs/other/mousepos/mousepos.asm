; SPDX-License-Identifier: GPL-2.0-only
; SPDX-FileCopyrightText:  2023-2026 KolibriOS Team

;---------------------------------------------------------------------

include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../encoding.inc"

;---------------------------------------------------------------------

KOS_APP_START

CODE

mcall   SF_SYSTEM, SSF_WINDOW_BEHAVIOR, SSSF_SET_WB, -1, 1
mcall   SF_SET_EVENTS_MASK, EVM_REDRAW + EVM_BUTTON + EVM_MOUSE

;---------------------------------------------------------------------

wait_event:
        mcall   SF_WAIT_EVENT

        cmp     eax, EV_REDRAW
        jz      redraw_event

        cmp     eax, EV_BUTTON
        jz      button_event

        cmp     eax, EV_MOUSE
        jz      mouse_event

        jmp     wait_event

;---------------------------------------------------------------------

button_event:
        mcall   SF_GET_BUTTON
        cmp     ah, BT_EXIT
        jne     wait_event
        mcall   SF_TERMINATE_PROCESS

;---------------------------------------------------------------------

mouse_event:
        call    draw_data
        jmp     wait_event

;---------------------------------------------------------------------

redraw_event:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors
        mcall                    , SSF_GET_SKIN_HEIGHT

        mov     ecx, eax
        add     ecx, WIN.Y shl 16 + WIN.H
        mov     edx, [sc.work]
        add     edx, WN_TYPE
        mcall   SF_CREATE_WINDOW, <WIN.X, WIN.W>, , , , header

        call    draw_labels
        call    draw_data

        mcall   SF_REDRAW, SSF_END_DRAW

        jmp     wait_event

;---------------------------------------------------------------------

draw_labels:
        mov     ecx, FONT_LB
        or      ecx, [sc.work_text]
        mcall   SF_DRAW_TEXT, <TXT.X, TXT.Y>,                   , lb_gx
        mcall               , <TXT.X, TXT.Y + TXT.H>,           , lb_gy
        mcall               , <TXT.X, TXT.Y + TXT.H*2 + GAP>,   , lb_pid
        mcall               , <TXT.X, TXT.Y + TXT.H*3 + GAP*2>, , lb_lx
        mcall               , <TXT.X, TXT.Y + TXT.H*4 + GAP*2>, , lb_ly

        mcall   SF_DRAW_RECT, <TXT.X, WIN.W - TXT.X*2 - BORD*2>, <TXT.Y + TXT.H*2,       1>, [sc.work_dark]
        mcall               ,                                  , <TXT.Y + TXT.H*3 + GAP, 1>,

        ret

;---------------------------------------------------------------------

draw_data:
        ; cleanup numbers backgrounds
        mcall   SF_DRAW_RECT, <TXT.W, CHAR_W*4 + 1>, <TXT.Y,                   TXT.H*2>, [sc.work]
        mcall               ,                      , <TXT.Y + TXT.H*2 + GAP,   TXT.H>,
        mcall               ,                      , <TXT.Y + TXT.H*3 + GAP*2, TXT.H*2>,

        ; save mouse position (it is needed several times below)
        mcall   SF_MOUSE_GET, SSF_SCREEN_POSITION
        movzx   ecx, ax
        mov     [mouse_y], ecx
        shr     eax, 16
        mov     [mouse_x], eax

        mov     esi, FONT_NM
        or      esi, [sc.work_text]

        ; Global X / Y
        mov     ecx, [mouse_x]
        mcall   SF_DRAW_NUMBER, FORM_NM, , <TXT.W,     TXT.Y>
        mcall                 ,        , , <TXT.W + 1, TXT.Y>
        mov     ecx, [mouse_y]
        mcall                 ,        , , <TXT.W,     TXT.Y + TXT.H>
        mcall                 ,        , , <TXT.W + 1, TXT.Y + TXT.H>

        ; PID of the window under the cursor
        mov     ebx, [mouse_x]
        mov     ecx, [mouse_y]
        mcall   SF_GET_PIXEL_OWNER
        mov     ecx, eax
        mcall   SF_DRAW_NUMBER, FORM_NM, , <TXT.W,     TXT.Y + TXT.H*2 + GAP>
        mcall                 ,        , , <TXT.W + 1, TXT.Y + TXT.H*2 + GAP>

        ; Local X / Y (relative to that window)
        mcall   SF_THREAD_INFO, pi
        mov     ecx, [mouse_x]
        sub     ecx, [pi.box.left]
        mcall   SF_DRAW_NUMBER, FORM_NM, , <TXT.W,     TXT.Y + TXT.H*3 + GAP*2>
        mcall                 ,        , , <TXT.W + 1, TXT.Y + TXT.H*3 + GAP*2>
        mov     ecx, [mouse_y]
        sub     ecx, [pi.box.top]
        mcall                 ,        , , <TXT.W,     TXT.Y + TXT.H*4 + GAP*2>
        mcall                 ,        , , <TXT.W + 1, TXT.Y + TXT.H*4 + GAP*2>

        ret

;---------------------------------------------------------------------

DATA

WN_TYPE =       0x34000000  ; window: skinned, fixed, caption + relative coords
FONT_LB =       0x90000000  ; label  text flags
FONT_NM =       0x10000000  ; number text flags
FORM_NM =       0x00040000  ; SF_DRAW_NUMBER: 4 decimal digits

BT_EXIT =       1           ; skin close button

header  db      "Mousepos", 0


; TXT: X,Y - first label pos, W - numbers column X, H - row step
CHAR_W  =       8
PAD     =       13
BORD    =       5
GAP     =       9

TXT     RECT    PAD, PAD, PAD   + CHAR_W*13,              24 ; reserve 13 chars width
WIN     RECT    100, 100, PAD*2 + CHAR_W*17 + BORD*2 + 1, PAD*2 + TXT.H*5 + GAP*2 - 8


if lang eq ru_RU
        lb_gx   cp866   "Экран X:", 0
        lb_gy   cp866   "Экран Y:", 0
        lb_lx   cp866   "Окно X:", 0
        lb_ly   cp866   "Окно Y:", 0
        lb_pid  cp866   "PID окна:", 0

else if lang eq es_ES
        lb_gx   cp850   "Pantalla X:", 0
        lb_gy   cp850   "Pantalla Y:", 0
        lb_lx   cp850   "Ventana X:", 0
        lb_ly   cp850   "Ventana Y:", 0
        lb_pid  cp850   "PID ventana:", 0
else
        lb_gx   db      "Screen X:", 0
        lb_gy   db      "Screen Y:", 0
        lb_lx   db      "Window X:", 0
        lb_ly   db      "Window Y:", 0
        lb_pid  db      "Window PID:", 0

end if

sc      system_colors
pi      process_information

UDATA

mouse_x rd      1
mouse_y rd      1

KOS_APP_END
