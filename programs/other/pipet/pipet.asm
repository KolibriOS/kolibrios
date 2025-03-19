; SPDX-License-Identifier: GPL-2.0-only
; SPDX-FileCopyrightText: 2025 KolibriOS Team
; FileContributor: Pipet 1.0 in C-- by Leency
; FileContributor: Pipet 2.0 in FASM by Burer

;---------------------------------------------------------------------

use32
org     0

        db      'MENUET01'
        dd      1
        dd      START
        dd      I_END
        dd      MEM
        dd      STACKTOP
        dd      0
        dd      0

include "../../macros.inc"
include "../../KOSfuncs.inc"

;---------------------------------------------------------------------

START:

        mcall   SF_SET_EVENTS_MASK, 0x00000027
        mcall   SF_SYSTEM, SSF_WINDOW_BEHAVIOR, SSSF_SET_WB, -1, 1
        mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1

        mcall   SF_SYS_MISC, SSF_HEAP_INIT

        ; initializing 7*7 rect of pixels from screen
        mcall   SF_SYS_MISC, SSF_MEM_ALLOC, 7*7*3
        mov     [sel_rect], eax

        ; loading and converting pipet icon from ICONS18W
        mcall   SF_SYS_MISC,   , 18*18*4
        mov     [pip_icon], eax

        mcall   SF_SYS_MISC, SSF_MEM_OPEN, win_icons_name, , 0
        add     eax, 39*18*18*4

        mov     esi, eax
        mov     edi, [pip_icon]
        mov     ecx, 18*18

        cld
        rep movsd

        call    get_pixels

;---------------------------------------------------------------------

still:

        mcall   SF_WAIT_EVENT

        cmp     eax, 1
        je      redraw
        cmp     eax, 2
        je      key
        cmp     eax, 3
        je      button
        cmp     eax, 6
        je      mouse

        jmp     still

;---------------------------------------------------------------------

redraw:
        call    draw_window

key:
        mcall   SF_GET_KEY

        cmp     ah, 1
        je      button.exit
        cmp     ah, 25
        je      make_pick_active
        cmp     ah, 19
        je      copy_col_rgb
        cmp     ah, 46
        je      copy_col_hex
        jmp     still

button:
        mcall   SF_GET_BUTTON

        cmp     ah, 11
        je      make_pick_active
        cmp     ah, 12
        je      copy_col_hex            ; copy HEX color
        cmp     ah, 13
        je      copy_col_rgb            ; copy RGB color
        cmp     ah, 14
        je      pick_col_cell           ; make pick active again
        cmp     ah, 1
        jne     still

        .exit:
                mcall   SF_TERMINATE_PROCESS

mouse:
        mcall   SF_MOUSE_GET, SSF_BUTTON
        test    ax, 0x0001
        jz      .move

        mov     [pick_act], 0x00        ; left mouse button click

        .move:
                cmp     [pick_act], 0x00
                je      still

                call    get_pixels
                call    draw_update

        jmp     still

;---------------------------------------------------------------------

; window redraw function
draw_window:

        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, win_cols, sizeof.system_colors
        mcall                    , SSF_GET_SKIN_HEIGHT,

        mov     ecx, eax
        add     ecx, WIN.Y * 65536 + WIN.H

        mov     edx, [win_cols.work]
        add     edx, 0x34000000
        mcall   SF_CREATE_WINDOW, <WIN.X, WIN.W>, , , , header

        call    draw_base
        call    draw_update

        mcall   SF_REDRAW, SSF_END_DRAW

        ret

;---------------------------------------------------------------------

; draw basic elements of window
draw_base:

        mcall   SF_DRAW_RECT, <BUT_PIP.X, BUT_PIP.H>, <BUT_PIP.Y, BUT_PIP.H>, [win_cols.work_graph]
        mcall               , <BUT_COL.X, BUT_COL.W>,                       ,
        mcall               , <BUT_HEX.X, BUT_HEX.W>, <BUT_HEX.Y, BUT_HEX.H>,
        mcall               ,                       , <BUT_RGB.Y, BUT_HEX.H>,
        mcall               , <BUT_REC.X, BUT_REC.W>, <BUT_REC.Y, BUT_REC.H>,

        mcall               , <BUT_PIP.X, BUT_PIP.H - 1>, <BUT_PIP.Y, BUT_PIP.H - 1>, [win_cols.work_dark]
        mcall               , <BUT_COL.X, BUT_COL.W - 1>,                           ,
        mcall               , <BUT_HEX.X, BUT_HEX.W - 1>, <BUT_HEX.Y, BUT_HEX.H - 1>,
        mcall               ,                           , <BUT_RGB.Y, BUT_HEX.H - 1>,
        mcall               , <BUT_REC.X, BUT_REC.W - 1>, <BUT_REC.Y, BUT_REC.H - 1>,

        mcall               , <BUT_PIP.X + 1, BUT_PIP.H - 2>, <BUT_PIP.Y + 1, BUT_PIP.H - 2>, [col_white]
        mcall               , <BUT_COL.X + 1, BUT_COL.W - 2>,                               ,
        mcall               , <BUT_HEX.X + 1, BUT_HEX.W - 2>, <BUT_HEX.Y + 1, BUT_HEX.H - 2>,
        mcall               ,                               , <BUT_RGB.Y + 1, BUT_HEX.H - 2>,
        mcall               , <BUT_REC.X + 1, BUT_REC.W - 2>, <BUT_REC.Y + 1, BUT_REC.H - 2>,


        ; buttons 11, 12, 13 and 14
        mcall   SF_DEFINE_BUTTON, <BUT_PIP.X + 1, BUT_PIP.W - 3>, <BUT_PIP.Y + 1, BUT_PIP.H - 3>, 0x4000000B
        mcall                   , <BUT_HEX.X + 1, BUT_HEX.W - 3>, <BUT_HEX.Y + 1, BUT_HEX.H - 3>, 0x4000000C
        mcall                   ,                               , <BUT_RGB.Y + 1, BUT_RGB.H - 3>, 0x4000000D
        mcall                   , <BUT_REC.X + 2, BUT_REC.W - 4>, <BUT_REC.Y + 2, BUT_REC.H - 4>, 0x6000000E

        ; 18*18 pixels icon
        mcall   SF_PUT_IMAGE_EXT, [pip_icon], 0x00120012, 0x000B000F, 32

        ret


; drawing text on buttons and colorful rect
draw_update:

        ; current color rect
        mcall   SF_DRAW_RECT, <BUT_COL.X + 2, BUT_COL.W - 4>, <BUT_COL.Y + 2, BUT_COL.H - 4>, [sel_color]

        ; color codes
        mcall               , <BUT_RGB.X + 1, BUT_HEX.W - 2>, <BUT_RGB.Y + 1, BUT_HEX.H - 2>, [col_white]

        mcall   SF_DRAW_NUMBER, 0x00060100, [sel_color], <BUT_HEX.X + 26, BUT_HEX.Y + 5>, 0x50000000, [col_white]

        mov     ebx, 0x00030000
        xor     ecx, ecx
        mov     edx, 65536 * 78 + 81
        xor     edi, edi

        dr_loop:
                mov     cl, byte [sel_color + edi]
                mov     esi, [rgb_cols + edi * 4]
                or      esi, 0x10000000
                mcall   , , , , ,
                sub     edx, 0x00200000
                inc     edi
                cmp     edi, 3
                jb      dr_loop

        ; 7*7 pixels grid
        mov     eax, SF_DRAW_RECT
        mov     ebx, 118 * 65536 + 12
        mov     ecx,  14 * 65536 + 12
        mov     esi, [sel_rect]
        mov     edi, 49

        .du_loop_rect:
                mcall   , , , dword [esi]
                add     ebx,  12 * 65536
                cmp     ebx, 196 * 65536
                jle     .du_loop_rect_row
                mov     ebx, 118 * 65536 + 12
                add     ecx,  12 * 65536

                .du_loop_rect_row:
                add     esi, 3
                dec     edi
                jne     .du_loop_rect

        ; selection of one pixel from 7*7 grid
        mov     cl, [cell_act_y]
        mov     al, 12
        mul     cl
        add     ax, 14
        shl     eax, 16
        mov     ax, 12
        mov     ecx, eax

        mov     bl, [cell_act_x]
        mov     al, 12
        mul     bl
        add     ax, 118
        shl     eax, 16
        mov     ax, 12
        mov     ebx, eax

        mcall   SF_DRAW_RECT, , , 0x00FF0000
        add     ebx, 2 * 65536 - 4
        add     ecx, 2 * 65536 - 4
        mcall               , , , [sel_color]

        ret

;---------------------------------------------------------------------

; making pipet active again
make_pick_active:

        mov     [pick_act],   0x01
        mov     [cell_act_x], 0x03
        mov     [cell_act_y], 0x03

        mcall   SF_DRAW_RECT, <BUT_REC.X + 1, BUT_REC.W - 2>, <BUT_REC.Y + 1, BUT_REC.H - 2>, [col_white]
        mcall   SF_DRAW_TEXT, <BUT_REC.X + 28, BUT_REC.Y + 37>, 0x10000000, mes_pick, 4

        mcall   SF_SLEEP, 50

        mcall   SF_DRAW_RECT, <BUT_PIP.X + 1, BUT_PIP.W - 2>, <BUT_PIP.Y + 1, BUT_PIP.H - 2>, [win_cols.work_light]
        mcall   SF_PUT_IMAGE_EXT, [pip_icon], 0x00120012, 0x000B000F, 32

        call    draw_update
        jmp     still


; read array of pixels from screen by mouse coords
get_pixels:

        mcall   SF_MOUSE_GET, SSF_SCREEN_POSITION
        mov     edx, eax

        mcall   SF_GET_SCREEN_SIZE
        mov     ebx, eax

        ; clamping mouse coords to stay within the screen
        call    clamp_pixels

        mcall   SF_GET_IMAGE, sel_color, <1, 1>,

        sub     edx, 0x00030003
        mcall   SF_GET_IMAGE, [sel_rect], <7, 7>,

        ret


; clamping mouse coords to stay within the screen
clamp_pixels:

        mov     eax, edx
        shr     eax, 16
        mov     cx, ax
        mov     ax, dx

        push    ax
        push    cx
        mov     ax, bx
        mov     di, ax
        mov     eax, ebx
        shr     eax, 16
        mov     si, ax

        pop     cx
        pop     ax

        .check_min_x:
                cmp     cx, 3
                jge     .check_min_y
                mov     cx, 3
        .check_min_y:
                cmp     ax, 3
                jge     .check_max_x
                mov     ax, 3
        .check_max_x:
                mov     dx, si
                sub     dx, 3
                cmp     cx, dx
                jle     .check_max_y
                mov     cx, dx
        .check_max_y:
                mov     dx, di
                sub     dx, 3
                cmp     ax, dx
                jle     .combine_coords
                mov     ax, dx

        .combine_coords:
                mov     dx, cx
                shl     edx, 16
                or     	dx, ax

        ret


; copy color HEX code
copy_col_hex:

        mcall   SF_DRAW_RECT, <BUT_HEX.X + 2, BUT_HEX.W - 4>, <BUT_HEX.Y + 2, BUT_HEX.H - 4>, [col_white]

        mov     ebx, [sel_color]
        mov     ecx, 6

        ch_loop:                                ; iterate over all hex-digits of color
                mov     al, bl
                and     al, 0x0F
                add     al, 0x30
                cmp     al, 0x39
                jbe     ch_loop.cont            ; if digit

                add     al, 0x07                ; if letter

                .cont:
                        mov     [color_hex + 11 + ecx], byte al
                        shr     ebx, 4
                        loop    ch_loop

        mcall   SF_CLIPBOARD, 2, color_hex.end - color_hex, color_hex
        mcall   SF_DRAW_TEXT, <BUT_HEX.X + 1, BUT_HEX.Y + 5>, 0x10000000, mes_copy, 12
        mcall   SF_SLEEP, 50

        mcall   SF_DRAW_RECT, <BUT_HEX.X + 1, BUT_HEX.W - 2>, <BUT_HEX.Y + 1, BUT_HEX.H - 2>, [col_white]

        call    draw_update
        jmp     still


; copy color RGB code
copy_col_rgb:

        mcall   SF_DRAW_RECT, <BUT_HEX.X + 2, BUT_HEX.W - 4>, <BUT_RGB.Y + 2, BUT_HEX.H - 4>, [col_white]

        mov     bl, 10
        mov     edx, [sel_color]
        mov     esi, 19
        mov     edi, 3

        cr_loop:

                mov     al, dl
                mov     ecx, 3

                .cr_loop_in:

                        and     eax, 0x000000FF
                        div     bl
                        add     ah, 0x30
                        mov     [color_rgb + esi + ecx], byte ah

                        loop    cr_loop.cr_loop_in

                shr     edx, 8
                sub     esi, 4
                dec     edi
                jg      cr_loop


        mcall   SF_CLIPBOARD, 2, color_rgb.end - color_rgb, color_rgb
        mcall   SF_DRAW_TEXT, <BUT_HEX.X + 1, BUT_RGB.Y + 5>, 0x10000000, mes_copy, 12
        mcall   SF_SLEEP, 50

        mcall   SF_DRAW_RECT, <BUT_HEX.X + 1, BUT_HEX.W - 2>, <BUT_RGB.Y + 1, BUT_HEX.H - 2>, [col_white]

        call    draw_update
        jmp     still


; picking one color cell from 7*7 grid
pick_col_cell:

        mcall   SF_MOUSE_GET, 1

        push    eax
        sub     ax, 14
        mov     bl, 12
        div     bl
        mov     [cell_act_y], al

        pop     eax
        shr     eax, 16
        sub     ax, 118
        div     bl
        mov     [cell_act_x], al

        xor     ebx, ebx
        mov     bl, [cell_act_y]
        mov     al, 7
        mul     bl
        add     al, [cell_act_x]
        mov     bx, 3
        mul     bx
        add     eax, [sel_rect]

        mov     ebx, dword [eax]
        and     ebx, 0x00FFFFFF
        mov     [sel_color], ebx

        call    draw_update
        jmp     still

;---------------------------------------------------------------------

WIN     RECT 100,100,221,112

BUT_PIP RECT   8, 12, 24, 24
BUT_COL RECT  40, 12, 68, 24
BUT_HEX RECT   8, 44,100, 24
BUT_RGB RECT   8, 76,100, 24
BUT_REC RECT 116, 12, 88, 88

;---------------------------------------------------------------------

win_cols        system_colors
win_icons_name  db 'ICONS18', 0
pip_icon        dd 0x00000000
col_white       dd 0x00FFFFFF

if lang eq ru_RU
                header  db 'Пипетка', 0
else if lang eq es_ES
                header  db 'Pipeta', 0
else
                header  db 'Pipet', 0
endf

mes_copy        db '   Copied   '
mes_pick        db 'Pick'

rgb_cols:
                dd 0x000000FF
                dd 0x00008000
                dd 0x00FF0000

pick_act        db 0x01
cell_act_x      db 0x03
cell_act_y      db 0x03

sel_rect        dd 0x00000000
sel_color:
                db 0xCF
                db 0xD7
                db 0xDD
                db 0x00

color_hex:
                dd color_hex.end - color_hex
                dd 0
                dd 1
                db '000000'
.end:

color_rgb:
                dd color_rgb.end - color_rgb
                dd 0
                dd 1
                db '000,000,000'
.end:

;---------------------------------------------------------------------

I_END:
        rb      0
        align   1024
STACKTOP:
MEM: