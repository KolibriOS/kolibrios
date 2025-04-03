; SPDX-License-Identifier: GPL-2.0-only
;
; Magnify - Screen Magnifier
; Copyright (C) 2005 MenuetOS
; Copyright (C) 2005-2025 KolibriOS team

;---------------------------------------------------------------------

use32
org     0x0

db      'MENUET01'
dd      1
dd      START
dd      I_END
dd      MEM
dd      STACKTOP
dd      0, 0

;---------------------------------------------------------------------

include '../../../macros.inc'
include "../../../KOSfuncs.inc"
include "../../../encoding.inc"

;---------------------------------------------------------------------

START:
        mcall   SF_SET_EVENTS_MASK, EVM_REDRAW or EVM_BUTTON or EVM_MOUSE

        mcall   SF_SYS_MISC, SSF_HEAP_INIT

        ; initializing MAG_H * MAG_W rect of pixels from screen
        mcall   SF_SYS_MISC, SSF_MEM_ALLOC, MAG_H * MAG_W * 3
        mov     [screen_rect], eax

        call    logic_read_screen

still:
        mcall   SF_WAIT_EVENT_TIMEOUT, DELAY

        cmp     eax, EV_REDRAW
        je      redraw

        cmp     eax, EV_BUTTON
        je      button

        cmp     eax, EV_MOUSE
        je      mouse

        jmp     redraw

redraw:
        call    draw_window
        call    draw_magnify

        jmp still

;---------------------------------------------------------------------

button:
        ; we have only one button, close
        or      eax, SF_TERMINATE_PROCESS
        mcall

mouse:
        mcall   SF_MOUSE_GET, SSF_BUTTON

        test    ax, 0x0001
        jnz     still

        call    logic_read_screen
        call    draw_magnify

        jmp     still

;---------------------------------------------------------------------
;   *******  WINDOW DEFINITIONS AND DRAW ********
;---------------------------------------------------------------------

draw_window:

        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT

        mov     ecx, eax
        add     ecx, WIN.Y shl 16 + WIN.H

        mcall   SF_CREATE_WINDOW, <WIN.X, WIN.W>, , 0x34181818, , labelt

        mcall   SF_REDRAW, SSF_END_DRAW

        ret


; MAG_H * MAG_W pixels grid
draw_magnify:

        mcall   SF_THREAD_INFO, procinfo, -1
        mov     al, byte [procinfo.wnd_state]
        test    al, 0x04
        jne     .du_loop_end

        mov     eax, SF_DRAW_RECT
        mov     ebx, MAG_S - 1
        mov     ecx, MAG_S - 1
        mov     esi, [screen_rect]
        mov     edi, MAG_W * MAG_H

        .du_loop_rect:
                mcall   , , , dword [esi]
                add     ebx, MAG_S shl 16
                cmp     ebx, MAG_S * MAG_W shl 16
                jle     .du_loop_rect_row
                mov     ebx, MAG_S - 1
                add     ecx, MAG_S shl 16

                .du_loop_rect_row:
                add     esi, 3
                dec     edi
                jne     .du_loop_rect

        .du_loop_end:
                ret

;---------------------------------------------------------------------
; LOGIC AREA
;---------------------------------------------------------------------

; read array of pixels from screen by mouse coords
logic_read_screen:

        mcall   SF_MOUSE_GET, SSF_SCREEN_POSITION
        mov     edx, eax

        ; clamping mouse coords to stay within the screen
        mcall   SF_GET_SCREEN_SIZE
        mov     ebx, eax
        call    logic_clamp_pixels

        sub     edx, (MAG_W / 2) shl 16 + (MAG_H / 2)
        mcall   SF_GET_IMAGE, [screen_rect], <MAG_W, MAG_H>,

        ret


; clamping mouse coords to stay within the screen
logic_clamp_pixels:

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
                cmp     cx, MAG_W / 2
                jge     .check_min_y
                mov     cx, MAG_W / 2
        .check_min_y:
                cmp     ax, MAG_H / 2
                jge     .check_max_x
                mov     ax, MAG_H / 2
        .check_max_x:
                mov     dx, si
                sub     dx, MAG_W / 2 - 1
                cmp     cx, dx
                jle     .check_max_y
                mov     cx, dx
        .check_max_y:
                mov     dx, di
                sub     dx, MAG_H / 2 - 1
                cmp     ax, dx
                jle     .combine_coords
                mov     ax, dx

        .combine_coords:
                mov     dx, cx
                shl     edx, 16
                or      dx, ax

        ret

;---------------------------------------------------------------------
; DATA AREA
;---------------------------------------------------------------------

DELAY           = 5

MAG_W           = 40
MAG_H           = 30
MAG_S           = 8

WIN             RECT    100, 100, MAG_W * MAG_S + 8, MAG_H * MAG_S + 3

;---------------------------------------------------------------------

if lang eq ru_RU
        labelt  cp866   'Magnify - Экранная лупа', 0
else if lang eq es_ES
        labelt  db      'Magnify - Lupa de Pantalla', 0
else
        labelt  db      'Magnify - Screen Magnifier', 0
endf

;---------------------------------------------------------------------

screen_rect     dd 0x00000000

;---------------------------------------------------------------------

I_END:
        rb      512
        align   512

STACKTOP:
        procinfo        process_information
MEM:
