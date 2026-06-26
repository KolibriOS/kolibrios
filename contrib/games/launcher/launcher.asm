; SPDX-License-Identifier: GPL-2.0-only
;
; Launcher - Launcher for Wolf3D, Doom, Quake
; Copyright (C) 2025-2026 KolibriOS team
;
; Contributor mxlgv - Idea
; Contributor Burer - Main code

; ================================================================

use32
org 0

; ================================================================

db      "MENUET01"
dd      0x01
dd      START
dd      I_END
dd      MEM
dd      STACK_TOP
dd      CMD_LINE
dd      0

; ================================================================

include "../../../programs/macros.inc"
include "../../../programs/KOSfuncs.inc"
include "../../../programs/encoding.inc"
include "../../../programs/proc32.inc"
include "../../../programs/dll.inc"
include "../../../programs/develop/libraries/box_lib/box_lib.mac"
include "cmdline.inc"

; ================================================================

LIBS:
        library gui,    "box_lib.obj"
        import  gui,    optionbox.draw, "option_box_draw", \
                        optionbox.mouse, "option_box_mouse"

; ================================================================

START:
        ; Parse the command line FIRST: heap init and dll.Load below reuse the
        ; memory the OS placed it in, so reading it later loses the --game arg.
        call    select_game

        mcall   SF_SET_EVENTS_MASK, EVM_REDRAW + EVM_BUTTON + EVM_MOUSE
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors
        mcall   SF_SYS_MISC, SSF_HEAP_INIT

        ; load gamepad icon from reshare (32 bpp); eax = 0 if ICONS18W absent
        mcall   SF_SYS_MISC, SSF_MEM_OPEN, icon_mem, , 0
        test    eax, eax
        jz      @f
        lea     eax, [eax + 23 * IC_PLAY.W * IC_PLAY.H * 4]
        
        @@:
        mov     [icon_play], eax

        ; load dynamic libraries
        stdcall dll.Load, LIBS
        test    eax, eax
        jnz     button.exit

        call    draw_window

        jmp     still

; ================================================================

still:
        mcall   SF_WAIT_EVENT

        cmp     al, EV_REDRAW
        je      redraw

        cmp     al, EV_BUTTON
        je      button

        call    options_mouse
        jmp     still

; ================================================================

redraw:
        call    draw_window
        jmp     still

; ================================================================

button:
        mcall   SF_GET_BUTTON

        cmp     ah, 1
        je      .exit

        cmp     ah, BTN_PLAY
        je      run_game

        jmp     still

        .exit:
                mcall   SF_TERMINATE_PROCESS

; ================================================================

draw_window:

        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mov     edx, [sc.work]
        add     edx, 0x34000000
        mcall   SF_CREATE_WINDOW, <WN_RECT.X, WN_RECT.W>, <WN_RECT.Y, WN_RECT.H>, , , [title_ptr]

        ; mode, resolutions and scale headers
        mov     ecx, FONT_BIG
        add     ecx, [sc.work_text]

        mcall   SF_DRAW_TEXT, <LB_MODE.X,      LB_MODE.Y> , , label_mode
        mcall               , <LB_MODE.X + 1,  LB_MODE.Y> , ,
        mcall               , <LB_RES.X,       LB_RES.Y>  , , label_res
        mcall               , <LB_RES.X + 1,   LB_RES.Y>  , ,
        mcall               , <LB_SCALE.X,     LB_SCALE.Y>, , label_scale
        mcall               , <LB_SCALE.X + 1, LB_SCALE.Y>, ,

        ; options groups
        call    options_set_colors

        invoke  optionbox.draw, op_list_mode
        invoke  optionbox.draw, op_list_res
        invoke  optionbox.draw, op_list_scale

        call   draw_option_labels

        ; play button
        mcall   SF_DEFINE_BUTTON, <BT_PLAY.X, BT_PLAY.W>, <BT_PLAY.Y, BT_PLAY.H>, BTN_PLAY, [sc.work_button]

        mov     ecx, [sc.work_button_text]
        add     ecx, FONT_BIG
        mcall   SF_DRAW_TEXT, <LB_PLAY.X, LB_PLAY.Y>, , text_play

        call    draw_icon_transparent

        mcall   SF_REDRAW, SSF_END_DRAW

        ret


draw_option_labels:
        mov     ecx, [sc.work_text]
        add     ecx, FONT_BIG
        mcall   SF_DRAW_TEXT, <LB_MODE_WIN.X,  LB_MODE_WIN.Y> , , text_win
        mcall               , <LB_MODE_WINF.X, LB_MODE_WINF.Y>, , text_winf
        mcall               , <LB_MODE_FULL.X, LB_MODE_FULL.Y>, , text_full

        cmp     [op_group_mode], op_mode_win
        je      .windowed

        ; dimmed labels for resolution and scale in fullscreen mode
        mov     ecx, [sc.work_graph]
        add     ecx, FONT_BIG

        .windowed:
        mcall   SF_DRAW_TEXT, <LB_RES_320_200.X, LB_RES_320_200.Y>, , text_320_200
        mcall               , <LB_RES_320_240.X, LB_RES_320_240.Y>, , text_320_240
        mcall               , <LB_SCALE_1.X,     LB_SCALE_1.Y>    , , text_x1
        mcall               , <LB_SCALE_2.X,     LB_SCALE_2.Y>    , , text_x2
        mcall               , <LB_SCALE_3.X,     LB_SCALE_3.Y>    , , text_x3
        mcall               , <LB_SCALE_4.X,     LB_SCALE_4.Y>    , , text_x4

        ret


draw_icon_transparent:

        mov     esi, [icon_play]
        test    esi, esi        ; ICONS18W unavailable -> skip icon
        jz      .ret
        mov     edi, [sc.work]
        and     edi, 0x00FFFFFF
        mov     ecx, IC_PLAY.Y
        mov     ebp, IC_PLAY.H

        .row:
                mov     ebx, IC_PLAY.X
                mov     eax, IC_PLAY.W

        .col:
                mov     edx, [esi]
                and     edx, 0x00FFFFFF
                cmp     edx, edi
                je      .skip
                cmp     edx, 0x00C5BDB9
                je      .skip
                push    eax
                mcall   SF_PUT_PIXEL
                pop     eax

        .skip:
                add     esi, 4
                inc     ebx
                dec     eax
                jnz     .col
                inc     ecx
                dec     ebp
                jnz     .row

        .ret:
        ret

; ================================================================

options_mouse:

        invoke  optionbox.mouse, op_list_mode

        cmp     [op_group_mode], op_mode_win
        jne     .fullscreen

        invoke  optionbox.mouse, op_list_res
        invoke  optionbox.mouse, op_list_scale

        .fullscreen:
                call    options_sync
                test    al, al
                jnz     draw_window

        ret


options_sync:

        xor     eax, eax

        mov     ebx, MODE_WIN
        cmp     [op_group_mode], op_mode_win
        je      .mode_set
        mov     ebx, MODE_WINF
        cmp     [op_group_mode], op_mode_winf
        je      .mode_set
        mov     ebx, MODE_FULL
        .mode_set:
        cmp     [mode], ebx
        je      .mode_done
        mov     [mode], ebx
        or      al, 1
        .mode_done:

        xor     ebx, ebx
        cmp     [op_group_res], op_res_320_200
        setne   bl
        cmp     [base_res_index], ebx
        je      .res_done
        mov     [base_res_index], ebx
        or      al, 1
        .res_done:

        xor     ebx, ebx
        cmp     [op_group_scale], op_scale_1
        je      .scale_set
        inc     ebx
        cmp     [op_group_scale], op_scale_2
        je      .scale_set
        inc     ebx
        cmp     [op_group_scale], op_scale_3
        je      .scale_set
        inc     ebx
        .scale_set:
        cmp     [scale_index], ebx
        je      .scale_done
        mov     [scale_index], ebx
        or      al, 1
        .scale_done:

        ret


options_set_colors:

        mov     eax, [sc.work_button]
        mov     ebx, [sc.work_light]
        mov     ecx, [sc.work]
        add     ecx, FONT_BIG

        mov     esi, op_list_mode
        call    options_set_group_colors

        cmp     [mode], dword MODE_WIN
        jne     .disabled

        mov     esi, op_list_res
        call    options_set_group_colors

        mov     esi, op_list_scale
        call    options_set_group_colors

        ret

        .disabled:
        mov     eax, [sc.work_graph]
        mov     ebx, [sc.work]
        mov     ecx, [sc.work]
        add     ecx, FONT_BIG

        mov     esi, op_list_res
        call    options_set_group_colors

        mov     esi, op_list_scale
        call    options_set_group_colors

        ret


options_set_group_colors:

        .loop:
                mov     edi, [esi]
                test    edi, edi
                jz      .end
                mov     op_border_color, eax
                mov     op_color, ebx
                mov     op_text_color, ecx
                add     esi, 4
                jmp     .loop
        .end:

        ret

; ================================================================

select_game:
        mov     dword [title_ptr], title_wolf
        mov     dword [args_res_ptr], args_res_wolf
        mov     dword [run_info.file], run_name_wolf

        cld
        mov     esi, CMD_LINE
        mov     edi, argv_buf
        mov     edx, argv_ptrs
        call    parse_cmdline

        mov     ecx, ebx
        mov     esi, argv_ptrs

        .loop:
                test    ecx, ecx
                jz      .done
                mov     eax, [esi]
                add     esi, 4
                dec     ecx
                mov     edi, token_game
                call    streq
                jnz     .loop

                test    ecx, ecx
                jz      .done
                mov     eax, [esi]
                add     esi, 4
                dec     ecx

                mov     edi, arg_wolf
                call    streq
                jz      .set_wolf
                mov     edi, arg_quake
                call    streq
                jz      .set_quake
                mov     edi, arg_doom
                call    streq
                jz      .set_doom
                jmp     .loop

        .set_wolf:
                mov     dword [title_ptr], title_wolf
                mov     dword [args_res_ptr], args_res_wolf
                mov     dword [args_fs_ptr], arg_fullscreen_wolf
                mov     dword [run_info.file], run_name_wolf
                jmp     .loop

        .set_doom:
                mov     dword [title_ptr], title_doom
                mov     dword [args_res_ptr], args_res_doom
                mov     dword [args_fs_ptr], arg_fullscreen
                mov     dword [run_info.file], run_name_doom
                jmp     .loop

        .set_quake:
                mov     dword [title_ptr], title_quake
                mov     dword [args_res_ptr], args_res_quake
                mov     dword [args_fs_ptr], arg_fullscreen
                mov     dword [run_info.file], run_name_quake
                jmp     .loop

        .done:
                ret

streq:
        push    esi
        push    edi
        mov     esi, eax
        .cmp:
                mov     bl, [esi]
                mov     bh, [edi]
                cmp     bl, bh
                jne     .no
                test    bl, bl
                jz      .yes
                inc     esi
                inc     edi
                jmp     .cmp
        .yes:
                pop     edi
                pop     esi
                xor     eax, eax
                ret
        .no:
                pop     edi
                pop     esi
                or      eax, 1
                ret

; ================================================================

args_build:
        mov     edi, args_buffer
        mov     ecx, ARGS_BUF_SIZE
        xor     eax, eax
        rep     stosb
        mov     edi, args_buffer

        mov     esi, [args_res_ptr]
        call    args_append_str

        cmp     [mode], dword MODE_WIN
        je      .windowed
        cmp     [mode], dword MODE_FULL
        je      .exclusive

        ; windowed fullscreen mode, screen_size - consts
        call    get_screen_size
        call    get_skin_height
        mov     eax, [screen_w]
        sub     eax, UI_PADDING * 2
        mov     edx, [screen_h]
        sub     edx, [skin_h]
        sub     edx, UI_PADDING
        jmp     .emit

        ; exclusive fullscreen, full screen size (borderless via -fullscreen)
        .exclusive:
        call    get_screen_size
        mov     eax, [screen_w]
        mov     edx, [screen_h]
        jmp     .emit

        ; windowed mode, res * scale
        .windowed:
        mov     eax, [base_res_index]
        mov     ebx, [base_w + eax * 4]
        mov     edx, [base_h + eax * 4]
        mov     eax, [scale_index]
        mov     ecx, [scale_vals + eax * 4]
        imul    ebx, ecx
        imul    edx, ecx
        mov     eax, ebx

        .emit:
        push    edx
        call    args_append_uint
        mov     al, ' '
        stosb
        pop     edx
        mov     eax, edx
        call    args_append_uint

        cmp     [mode], dword MODE_FULL
        jne     .no_fs
        mov     esi, [args_fs_ptr]
        test    esi, esi
        jz      .no_fs
        call    args_append_str
        .no_fs:

        ret


args_append_str:

        lodsb
        test    al, al
        jz      .done
        stosb
        jmp     args_append_str

        .done:
        ret


args_append_uint:

        mov     esi, num_buf + 15
        mov     byte [esi], 0
        mov     ebx, 10

        cmp     eax, 0
        jne     .loop

        dec     esi
        mov     byte [esi], '0'
        jmp     .copy

        .loop:
        xor     edx, edx
        div     ebx
        add     dl, '0'
        dec     esi
        mov     [esi], dl
        test    eax, eax
        jne     .loop

        .copy:
        mov     al, [esi]
        test    al, al
        jz      .done

        stosb
        inc     esi
        jmp     .copy

        .done:
        ret

; ================================================================

run_game:

        call    args_build
        mov     dword [run_info.args], args_buffer
        mcall   SF_FILE, run_info

        jmp     still

; ================================================================

get_screen_size:

        mcall   SF_GET_SCREEN_SIZE
        movzx   ecx, ax
        inc     ecx
        mov     [screen_h], ecx
        shr     eax, 16
        inc     eax
        mov     [screen_w], eax

        ret


get_skin_height:

        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
        mov     [skin_h], eax

        ret

; ================================================================

WN_RECT         RECT    128, 128, 220, 293

LB_MODE         RECT      7,  10,   0,   0

LB_MODE_WIN     RECT     30,  35,   0,   0
LB_MODE_WINF    RECT     30,  58,   0,   0
LB_MODE_FULL    RECT     30,  81,   0,   0

BT_MODE_WIN     RECT      8,  34,  15,  15
BT_MODE_WINF    RECT      8,  57,  15,  15
BT_MODE_FULL    RECT      8,  80,  15,  15

LB_RES          RECT      7, 109,   0,   0

LB_RES_320_200  RECT     30, 134,   0,   0
LB_RES_320_240  RECT     30, 158,   0,   0

BT_RES_320_200  RECT      8, 133,  15,  15
BT_RES_320_240  RECT      8, 157,  15,  15

LB_SCALE        RECT    134, 109,   0,   0

LB_SCALE_1      RECT    157, 134,   0,   0
LB_SCALE_2      RECT    157, 157,   0,   0
LB_SCALE_3      RECT    157, 180,   0,   0
LB_SCALE_4      RECT    157, 203,   0,   0

BT_SCALE_1      RECT    135, 133,  15,  15
BT_SCALE_2      RECT    135, 156,  15,  15
BT_SCALE_3      RECT    135, 179,  15,  15
BT_SCALE_4      RECT    135, 202,  15,  15

BT_PLAY         RECT      8, 233, 196,  23
LB_PLAY         RECT    100, 237,   0,   0

IC_PLAY         RECT     76, 235,  18,  18

FONT_BIG        = 0x90000000
UI_MARGIN       = 9
UI_PADDING      = 5

; ================================================================

if lang eq ru_RU

        title_wolf      cp866 "Лаунчер Wolf3D", 0
        title_doom      cp866 "Лаунчер Doom", 0
        title_quake     cp866 "Лаунчер Quake", 0

        label_mode      cp866 "Режим отображения:", 0
        label_res       cp866 "Разрешение:", 0
        label_scale     cp866 "Масштаб:", 0
        text_win        cp866 "Оконный", 0
        text_win_len    = $ - text_win - 1
        text_winf       cp866 "Полноэкр. в окне", 0
        text_winf_len   = $ - text_winf - 1
        text_full       cp866 "Весь экран", 0
        text_full_len   = $ - text_full - 1
        text_play       cp866 "Играть", 0

else if lang eq es_ES

        title_wolf      cp850 "Wolf3D Launcher", 0
        title_doom      cp850 "Doom Launcher", 0
        title_quake     cp850 "Quake Launcher", 0

        label_mode      cp850 "Modo de pantalla:", 0
        label_res       cp850 "Resolucion:", 0
        label_scale     cp850 "Escala:", 0
        text_win        cp850 "En ventana", 0
        text_win_len    = $ - text_win - 1
        text_winf       cp850 "Ventana completa", 0
        text_winf_len   = $ - text_winf - 1
        text_full       cp850 "Pantalla completa", 0
        text_full_len   = $ - text_full - 1
        text_play       cp850 "Jugar", 0

else

        title_wolf      db "Wolf3D Launcher", 0
        title_doom      db "Doom Launcher", 0
        title_quake     db "Quake Launcher", 0

        label_mode      db "Display mode:", 0
        label_res       db "Resolution:", 0
        label_scale     db "Scale:", 0
        text_win        db "Windowed", 0
        text_win_len    = $ - text_win - 1
        text_winf       db "Windowed fullscreen", 0
        text_winf_len   = $ - text_winf - 1
        text_full       db "Exclusive fullscreen", 0
        text_full_len   = $ - text_full - 1
        text_play       db "Play", 0

endf

text_320_200            db "320x200", 0
text_320_200_len        = $ - text_320_200 - 1
text_320_240            db "320x240", 0
text_320_240_len        = $ - text_320_240 - 1
text_x1                 db "x1", 0
text_x1_len             = $ - text_x1 - 1
text_x2                 db "x2", 0
text_x2_len             = $ - text_x2 - 1
text_x3                 db "x3", 0
text_x3_len             = $ - text_x3 - 1
text_x4                 db "x4", 0
text_x4_len             = $ - text_x4 - 1

; ================================================================

op_mode_win     option_box op_group_mode, BT_MODE_WIN.X, BT_MODE_WIN.Y, UI_MARGIN, BT_MODE_WIN.W, 0, 0, 0, text_win, text_win_len, 0
op_mode_winf    option_box op_group_mode, BT_MODE_WINF.X, BT_MODE_WINF.Y, UI_MARGIN, BT_MODE_WINF.W, 0, 0, 0, text_winf, text_winf_len, 0
op_mode_full    option_box op_group_mode, BT_MODE_FULL.X, BT_MODE_FULL.Y, UI_MARGIN, BT_MODE_FULL.W, 0, 0, 0, text_full, text_full_len, 0

op_res_320_200  option_box op_group_res,  BT_RES_320_200.X, BT_RES_320_200.Y, UI_MARGIN, BT_RES_320_200.W, 0, 0, 0, text_320_200, text_320_200_len, 0
op_res_320_240  option_box op_group_res,  BT_RES_320_240.X, BT_RES_320_240.Y, UI_MARGIN, BT_RES_320_240.W, 0, 0, 0, text_320_240, text_320_240_len, 0

op_scale_1      option_box op_group_scale, BT_SCALE_1.X, BT_SCALE_1.Y, UI_MARGIN, BT_SCALE_1.W, 0, 0, 0, text_x1, text_x1_len, 0
op_scale_2      option_box op_group_scale, BT_SCALE_2.X, BT_SCALE_2.Y, UI_MARGIN, BT_SCALE_2.W, 0, 0, 0, text_x2, text_x2_len, 0
op_scale_3      option_box op_group_scale, BT_SCALE_3.X, BT_SCALE_3.Y, UI_MARGIN, BT_SCALE_3.W, 0, 0, 0, text_x3, text_x3_len, 0
op_scale_4      option_box op_group_scale, BT_SCALE_4.X, BT_SCALE_4.Y, UI_MARGIN, BT_SCALE_4.W, 0, 0, 0, text_x4, text_x4_len, 0

op_list_mode    dd op_mode_win, op_mode_winf, op_mode_full, 0
op_list_res     dd op_res_320_200, op_res_320_240, 0
op_list_scale   dd op_scale_1, op_scale_2, op_scale_3, op_scale_4, 0

op_group_mode   dd op_mode_win
op_group_res    dd op_res_320_200
op_group_scale  dd op_scale_3

; ================================================================

MODE_WIN        = 0 ; windowed
MODE_WINF       = 1 ; windowed fullsceen
MODE_FULL       = 2 ; exclusive (borderless) fullscreen

RES_320_200     = 0 ; 320x200
RES_320_240     = 1 ; 320x240

SCALE_X1        = 0 ; x1
SCALE_X2        = 1 ; x2
SCALE_X3        = 2 ; x3
SCALE_X4        = 3 ; x4

mode            dd MODE_WIN
base_res_index  dd RES_320_200
scale_index     dd SCALE_X3


base_w          dd 320, 320
base_h          dd 200, 240
scale_vals      dd 1, 2, 3, 4

screen_w        dd 0
screen_h        dd 0
skin_h          dd 0

BTN_PLAY        = 0x40

icon_play       dd 0x00000000
icon_mem        db "ICONS18W", 0

token_game      db "--game", 0
arg_wolf        db "wolf3d", 0
arg_doom        db "doom", 0
arg_quake       db "quake", 0

; game-specific data
title_ptr       dd title_wolf
args_res_ptr    dd args_res_wolf
args_fs_ptr     dd arg_fullscreen_wolf

args_res_wolf   db "--resf ", 0
args_res_doom   db "-winsize ", 0
args_res_quake  db "-winsize ", 0
arg_fullscreen      db " -fullscreen", 0
arg_fullscreen_wolf db " --fullscreen", 0

run_info:
        .func   dd SSF_START_APP
        .flags  dd 0
        .args   dd 0
        .res0   dd 0
        .res1   dd 0
        .zero   db 0
        .file   dd run_name_wolf

run_name_wolf   db "/kolibrios/games/wolf3d/wolf3d", 0
run_name_doom   db "/kolibrios/games/doom1/doom", 0
run_name_quake  db "/kolibrios/games/quake/sdlquake", 0

; ================================================================

I_END:

; The kernel zeroes an app's BSS only from the first page boundary past I_END
; (map_process_image rounds file_size UP to a page before clearing), so the tail
; of the last loaded page holds garbage. Page-align these reserved buffers into
; the zeroed region. The padding and buffers are reserved (not stored in the
; binary), so this keeps the file small while guaranteeing zeroed buffers.
align   4096

sc              system_colors

ARGS_BUF_SIZE   = 64
args_buffer     rb ARGS_BUF_SIZE
num_buf         rb 16

CMD_LINE        rb 256
argv_ptrs       rd 16
argv_buf        rb 256

rb      1024            ; stack
align   512

STACK_TOP:
MEM:
