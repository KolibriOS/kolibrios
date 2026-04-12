; ==================
; ARKANOID INFINITE
; ==================
; SPDX-License-Identifier: GPL-2.0-only
; Program - Lbreakout clone game
; Copyright (C) 2026 KolibriOS team
; Author: Max, <snapemax@wp.pl>

use32
org 0x0

db      'MENUET01'
dd      0x01
dd      start
dd      i_end
dd      mem_end
dd      stacktop
dd      0, 0

include '../../macros.inc'
include '../../KOSfuncs.inc'

; --- CONSTANTS ---
WIN_W           equ 550
WIN_H           equ 553
CLR_WINDOW_BG   equ 0xACE1AF
GAME_W          equ 380
GAME_H          equ 500
GAME_X          equ 13
GAME_Y          equ 13
BPP             equ 3
STRIDE          equ GAME_W * BPP
BRICK_W         equ 40
BRICK_H         equ 15
MAP_X_OFF       equ 30
MAP_Y_OFF       equ 80
PADDLE_W        equ 80
PADDLE_Y        equ 465
BALL_SIZE       equ 8


start:
        mov     esp, stacktop
        mcall   SF_SET_EVENTS_MASK, EVM_REDRAW + EVM_BUTTON + EVM_KEY
        call    load_highscores
        call    generate_random_level
        call    draw_full_window

event_loop:
        mcall   SF_WAIT_EVENT_TIMEOUT, 1
        test    eax, eax
        jz      no_more_events
        cmp     eax, EV_REDRAW
        je      on_redraw
        cmp     eax, EV_BUTTON
        je      on_button
        cmp     eax, EV_KEY
        je      on_key
        jmp     event_loop

on_key:
        mcall   SF_GET_KEY
        shr     eax, 8
        cmp     dword [input_mode], 1
        je      handle_name_input
        cmp     dword [lives], 0
        jg      event_loop
        cmp     al, 'y'
        je      reset_game
        cmp     al, 'Y'
        je      reset_game
        cmp     al, 'n'
        je      close_app
        cmp     al, 'N'
        je      close_app
        jmp     event_loop

draw_input_area:
        pushad
        mcall   13, <420, 100>, <205, 25>, CLR_WINDOW_BG
        mcall    4, <420, 190>, 0xB0000000 + 0xFF0000, new_hs_txt, 14
        mcall    4, <420, 215>, 0xB0000000 + 0xFF00FF, new_name, 8
        popad
        ret

handle_name_input:
        cmp     al, 8 ; Backspace
        je      .backspace
        cmp     al, 13 ; Enter
        je      .save_hs
        cmp     al, ' '
        jl      event_loop
        cmp     al, 'z'
        jg      event_loop
        mov     ecx, [name_ptr]
        cmp     ecx, 8
        jae     event_loop
        mov     [new_name + ecx], al
        inc     dword [name_ptr]
        mov     byte [new_name + ecx + 1], 0
        call    draw_input_area
        jmp     event_loop

        .backspace:
        cmp     dword [name_ptr], 0
        je      event_loop
        dec     dword [name_ptr]
        mov     ecx, [name_ptr]
        mov     byte [new_name + ecx], '-'
        call    draw_input_area
        jmp     event_loop

        .save_hs:
        call    insert_highscore
        mov     dword [input_mode], 0
        call    draw_full_window
        jmp     event_loop

on_button:
        mcall   SF_GET_BUTTON
        shr     eax, 8
        cmp     eax, 1
        jne     event_loop

close_app:
        mcall   SF_TERMINATE_PROCESS

on_redraw:
        call    draw_full_window
        jmp     event_loop

reset_game:
        xor     eax, eax
        mov     [score], eax
        mov     [ball_active], eax
        mov     [old_score], eax
        mov     [old_lives], eax
        mov     [old_level], eax
        push    4
        pop     [lives]
        inc     eax
        mov     [current_level], eax
        mov     edi, score_buffer
        mov     ecx, 24
        xor     eax, eax
        rep     stosd
        call    generate_random_level
        call    draw_full_window
        jmp     event_loop

no_more_events:
        cmp     [lives], 0
        jle     .render_only
        cmp     [input_mode], 1
        je      .render_only
        mcall   37, 1
        shr     eax, 16
        movsx   ebx, ax
        sub     ebx, GAME_X + (PADDLE_W / 2)
        test    ebx, ebx
        jns     .check_right
        xor     ebx, ebx

        .check_right:
        cmp     ebx, GAME_W - PADDLE_W
        jle     .apply_p
        mov     ebx, GAME_W - PADDLE_W

        .apply_p:
        mov     [paddle_x], ebx
        cmp     [ball_active], 0
        je      .ball_on_paddle
        call    move_ball
        call    check_bricks
        cmp     dword [bricks_left], 0
        jne     .render_only
        call    next_level
        call    draw_full_window
        jmp     .render_only

        .ball_on_paddle:
        mcall   37, 2
        test    eax, 1
        jz      .stay_still
        mov     [ball_active], 1
        mov     dword [ball_dx], 2
        mov     dword [ball_dy], -4
        jmp     .render_only

        .stay_still:
        mov     eax, [paddle_x]
        add     eax, (PADDLE_W / 2) - (BALL_SIZE / 2)
        mov     [ball_x], eax
        mov     [ball_y], PADDLE_Y - BALL_SIZE - 1

        .render_only:
        call    render_buffer
        jmp     event_loop

; --- HIGH SCORE LOGIC ---
load_highscores:
        pushad
        mov     [file_info.op], 0
        mov     [file_info.path], HS_FILE
        mov     [file_info.buf], hs_data
        mov     [file_info.size], 60
        mcall   70, file_info
        test    eax, eax
        jnz     .create_default
        popad
        ret

        .create_default:
        mov     edi, hs_data
        mov     ecx, 5

        .init_loop:
        mov     eax, 'PLAY'
        stosd
        mov     eax, 'ER__'
        stosd
        xor     eax, eax
        stosd
        loop    .init_loop
        call    save_highscores
        popad
        ret

save_highscores:
        pushad
        mov     [file_info.op], 2
        mov     [file_info.path], HS_FILE
        mov     [file_info.buf], hs_data
        mov     [file_info.size], 60
        mcall   70, file_info
        popad
        ret

check_if_highscore:
        mov     eax, [score]
        test    eax, eax
        jz      .no
        mov     esi, hs_data + 8 ; point value of the first record
        mov     ecx, 5

        .l:
        cmp     eax, [esi]
        jg      .yes
        add     esi, 12
        loop    .l

        .no:
        ret

        .yes:
        mov     dword [input_mode], 1
        mov     dword [name_ptr], 0
        mov     edi, new_name
        mov     al, ' '
        mov     ecx, 8
        rep     stosb
        call    draw_input_area
        ret

insert_highscore:
        pushad
        mov     eax, [score]
        mov     edi, hs_data
        mov     ecx, 5

        .find_pos:
        cmp     eax, [edi+8]
        jg      .found
        add     edi, 12
        loop    .find_pos
        popad
        ret

        .found:
        push    edi ecx
        mov     esi, hs_data + 48 ; start of the last (5th) record

        .move_loop:
        cmp     esi, edi
        je      .done_move
        push    esi edi
        mov     edi, esi
        sub     esi, 12
        mov     ecx, 12
        rep     movsb
        pop     edi esi
        sub     esi, 12
        jmp     .move_loop

        .done_move:
        pop     ecx edi
        mov     esi, new_name
        mov     ecx, 8
        rep     movsb
        mov     eax, [score]
        stosd
        call    save_highscores
        popad
        ret

; --- BALL MECHANICS ---
move_ball:
        mov     eax, [ball_x]
        add     eax, [ball_dx]
        mov     [ball_x], eax
        test    eax, eax
        jg      .check_right_wall
        mov     dword [ball_x], 0
        neg     dword [ball_dx]
        jmp     .move_y

        .check_right_wall:
        cmp     eax, GAME_W - BALL_SIZE
        jl      .move_y
        mov     dword [ball_x], GAME_W - BALL_SIZE
        neg     dword [ball_dx]

        .move_y:
        mov     eax, [ball_y]
        add     eax, [ball_dy]
        mov     [ball_y], eax
        test    eax, eax
        jg      .check_paddle
        mov     dword [ball_y], 0
        neg     dword [ball_dy]
        ret

        .check_paddle:
        cmp     eax, PADDLE_Y - BALL_SIZE
        jl      .check_out
        cmp     eax, PADDLE_Y + 5
        jg      .check_out
        mov     edx, [ball_x]
        add     edx, BALL_SIZE / 2
        mov     ebx, [paddle_x]
        mov     ecx, edx
        sub     ecx, ebx
        js      .check_out
        cmp     ecx, PADDLE_W
        jg      .check_out
        sub     ecx, PADDLE_W / 2
        mov     eax, ecx
        cdq
        mov     esi, 8
        idiv    esi
        add     eax, [ball_dx]
        sar     eax, 1
        test    eax, eax
        jnz     .apply_dx
        mov     eax, 1

        .apply_dx:
        mov     [ball_dx], eax
        neg     dword [ball_dy]
        mov     dword [ball_y], PADDLE_Y - BALL_SIZE - 1
        ret

        .check_out:
        cmp     eax, GAME_H
        jg      .die
        ret

        .die:
        mov     dword [ball_active], 0
        dec     dword [lives]
        jnz     .not_over
        call    check_if_highscore

        .not_over:
        call    update_dynamic_stats
        ret

check_bricks:
        pushad
        mov     esi, active_map
        xor     edi, edi

        .loop_y:
        xor     ebx, ebx

        .loop_x:
        mov     al, [esi]
        test    al, al
        jz      .next
        mov     ecx, ebx
        imul    ecx, BRICK_W
        add     ecx, MAP_X_OFF
        mov     edx, edi
        imul    edx, BRICK_H
        add     edx, MAP_Y_OFF
        mov     eax, [ball_x]
        add     eax, BALL_SIZE
        cmp     eax, ecx
        jl      .next
        sub     eax, BALL_SIZE
        mov     ebp, ecx
        add     ebp, BRICK_W
        cmp     eax, ebp
        jg      .next
        mov     eax, [ball_y]
        add     eax, BALL_SIZE
        cmp     eax, edx
        jl      .next
        sub     eax, BALL_SIZE
        mov     ebp, edx
        add     ebp, BRICK_H
        cmp     eax, ebp
        jg      .next
        dec     byte [esi]
        jnz     .brick_reflect
        dec     dword [bricks_left]

        .brick_reflect:
        neg     dword [ball_dy]
        mov     eax, [ball_dy]
        test    eax, eax
        js      .snap_up
        mov     [ball_y], ebp
        jmp     .done_collision

        .snap_up:
        mov     [ball_y], edx
        sub     dword [ball_y], BALL_SIZE

        .done_collision:
        add     dword [score], 10
        call    update_dynamic_stats
        popad
        ret

        .next:
        inc     esi
        inc     ebx
        cmp     ebx, 8
        jl      .loop_x
        inc     edi
        cmp     edi, 4
        jl      .loop_y
        popad
        ret

; --- RENDERING ---
draw_full_window:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_CREATE_WINDOW, <100, WIN_W>, <100, WIN_H>, 0x33000000 + CLR_WINDOW_BG, 0, title

        ; SCORE
        mcall   4, <420, 30>, 0xB0000000 + 0x0000FF, score_txt, 5

        ; LIVES
        mcall    , <420, 90>, 0xB0000000 + 0x000000, lives_txt, 5

        ; LEVEL
        mcall    , <420, 140>, 0xB0000000 + 0x555555, level_txt, 5

        ; HIGH SCORES
        mcall    , <420, 300>, 0xB0000000 + 0x8B0000, hs_title, 11

        .hs_draw:
        mov     esi, hs_data
        mov     dword [temp_y], 340
        mov     ecx, 5

        .hs_loop:
        push    ecx
        push    esi
        mov     ebx, 420 shl 16
        add     ebx, [temp_y]
        mcall   4, , 0xB0000000 + 0x000000, esi, 8
        pop     esi
        push    esi
        mov     eax, [esi+8]
        mov     edi, hs_temp_buf
        call    int_to_string
        mov     esi, eax
        mov     ebx, 495 shl 16
        add     ebx, [temp_y]
        mcall   4, , 0xB0000000 + 0x000000, hs_temp_buf
        pop     esi
        pop     ecx
        add     esi, 12
        add     dword [temp_y], 25
        dec     ecx
        jnz     .hs_loop

        .hs_finish_up:
        mov     dword [old_score], -1
        mov     dword [old_lives], -1
        mov     dword [old_level], -1
        call    update_dynamic_stats
        call    render_buffer
        mcall   SF_REDRAW, SSF_END_DRAW
        ret

update_dynamic_stats:
        pushad
        ; --- SCORE VALUE ---
        mov     eax, [score]
        cmp     eax, [old_score]
        je      .skip_score
        mov     edi, score_buffer
        push    eax
        mov     eax, [old_score]
        cmp     eax, -1
        je      .no_old_s
        call    int_to_string
        mcall   4, <420, 55>, 0xB0000000 + CLR_WINDOW_BG, score_buffer, 10

        .no_old_s:
        pop     eax
        mov     [old_score], eax
        mov     edi, score_buffer
        call    int_to_string
        mov     esi, eax
        mcall   4, <420, 55>, 0xB0000000 + 0x0000FF, score_buffer

        .skip_score:
        ; --- LIVES VALUE ---
        mov     eax, [lives]
        cmp     eax, [old_lives]
        je      .skip_lives
        mov     edi, lives_buffer
        push    eax
        mov     eax, [old_lives]
        cmp     eax, -1
        je      .no_old_l
        call    int_to_string
        mcall   4, <420, 110>, 0xB0000000 + CLR_WINDOW_BG, lives_buffer, 2

        .no_old_l:
        pop     eax
        mov     [old_lives], eax
        cmp     eax, 0
        jle     .over
        mov     edi, lives_buffer
        call    int_to_string
        mov     esi, eax
        mcall   4, <420, 110>, 0xB0000000 + 0x000000, lives_buffer

        .skip_lives:
        ; --- LEVEL VALUE ---
        mov     eax, [current_level]
        cmp     eax, [old_level]
        je      .skip_level
        mov     edi, level_buffer
        push    eax
        mov     eax, [old_level]
        cmp     eax, -1
        je      .no_old_v
        call    int_to_string
        mcall   4, <420, 165>, 0xB0000000 + CLR_WINDOW_BG, level_buffer, 2

        .no_old_v:
        pop     eax
        mov     [old_level], eax
        mov     edi, level_buffer
        call    int_to_string
        mov     esi, eax
        mcall   4, <420, 165>, 0xB0000000 + 0x555555, level_buffer

        .skip_level:
        popad
        ret

        .over:
        ; GAME OVER messages
        mcall   4, <420, 110>, 0xB0000000 + 0xFF0000, over_txt, 9
        mcall    , <420, 250>, 0xB0000000 + 0x000000, again_txt, 11
        mcall    , <420, 270>,                      , y_n_txt, 3
        popad
        ret

; --- HELPERS ---
int_to_string:
        push    ebx
        push    ecx
        push    edi
        mov     ebx, edi        ; ebx = buffer
        xor     ecx, ecx        ; digit counter
        ; If number is 0
        test    eax, eax
        jnz     .conv_loop
        mov     byte [ebx], '0'
        mov     byte [ebx+1], 0
        mov     eax, 1          ; Return length 1
        jmp     .done_int

        .conv_loop:
        xor     edx, edx
        mov     esi, 10
        div     esi
        add     dl, '0'
        push    dx             ; Save digit on stack
        inc     ecx
        test    eax, eax
        jnz     .conv_loop
        mov     eax, ecx        ; Return length in EAX
        mov     edi, ebx

        .copy_loop:
        pop     dx
        mov     [edi], dl
        inc     edi
        loop    .copy_loop
        mov     byte [edi], 0

        .done_int:
        pop     edi
        pop     ecx
        pop     ebx
        ret

generate_random_level:
        pushad
        mov     edi, active_map
        mov     ecx, 32
        mov     dword [bricks_left], 0
        mcall   26
        add     [random_seed], eax

        .gen_loop:
        mov     eax, [random_seed]
        imul    eax, 1103515245
        add     eax, 12345
        mov     [random_seed], eax
        shr     eax, 16
        xor     edx, edx
        mov     ebx, 3
        div     ebx
        mov     [edi], dl
        test    dl, dl
        jz      .skip
        inc     dword [bricks_left]

        .skip:
        inc     edi
        loop    .gen_loop
        cmp     dword [bricks_left], 0
        jne     .done
        mov     byte [active_map], 1
        mov     dword [bricks_left], 1

        .done:
        popad
        ret

next_level:
        inc     dword [current_level]
        mov     dword [ball_active], 0
        call    generate_random_level
        ret

render_buffer:
        pushad
        mov     edi, screen_buffer
        mov     ecx, (GAME_W * GAME_H * 3) / 4
        xor     eax, eax
        rep     stosd
        mov     esi, active_map
        xor     ebp, ebp
        
        .r_y:
        xor     edi, edi

        .r_x:
        lodsb
        test    al, al
        jz      .r_next
        push    edi ebp eax
        imul    ebx, edi, BRICK_W
        add     ebx, MAP_X_OFF
        imul    ecx, ebp, BRICK_H
        add     ecx, MAP_Y_OFF
        cmp     al, 2
        je      .c_s
        mov     edx, 0xFF0000
        cmp     ebp, 1
        je      .c_o
        cmp     ebp, 2
        je      .c_y
        cmp     ebp, 3
        je      .c_g
        jmp     .dr

        .c_o:
        mov     edx, 0xFF8C00
        jmp     .dr

        .c_y:
        mov     edx, 0xFFFF00
        jmp     .dr

        .c_g:
        mov     edx, 0x00FF00
        jmp     .dr

        .c_s:
        mov     edx, 0xC0C0C0

        .dr:
        mov     edi, BRICK_W - 2
        mov     ebp, BRICK_H - 2
        call    fill_rect
        pop     eax ebp edi

        .r_next:
        inc     edi
        cmp     edi, 8
        jl      .r_x
        inc     ebp
        cmp     ebp, 4
        jl      .r_y
        mov     ebx, [paddle_x]
        mov     ecx, PADDLE_Y
        call    draw_paddle
        call    draw_ball
        mcall   SF_PUT_IMAGE, screen_buffer, <GAME_W, GAME_H>, <GAME_X, GAME_Y>
        popad
        ret

fill_rect:
        pushad
        imul    eax, ecx, STRIDE
        imul    ecx, ebx, 3
        add     eax, ecx
        add     eax, screen_buffer

        .yl:
        push    eax
        mov     ecx, edi

        .xl:
        mov     [eax], dl
        mov     [eax+1], dh
        push    edx
        shr     edx, 16
        mov     [eax+2], dl
        pop     edx
        add     eax, 3
        loop    .xl
        pop     eax
        add     eax, STRIDE
        dec     ebp
        jnz     .yl
        popad
        ret

draw_paddle:
        pushad
        imul    eax, ecx, STRIDE
        imul    ecx, ebx, 3
        add     eax, ecx
        add     eax, screen_buffer
        xor     ebp, ebp
        
        .yl:
        push    eax
        mov     esi, 20
        mov     ecx, ebp
        shl     ecx, 1
        sub     esi, ecx
        js      .sk
        imul    ecx, esi, 3
        add     eax, ecx
        mov     edi, PADDLE_W
        sub     edi, esi
        sub     edi, esi
        jle     .sk

        .xl:
        mov     byte [eax+1], 0xFF
        add     eax, 3
        dec     edi
        jnz     .xl

        .sk:
        pop     eax
        add     eax, STRIDE
        inc     ebp
        cmp     ebp, 10
        jl      .yl
        popad
        ret

draw_ball:
        pushad
        mov     edx, [ball_y]
        test    edx, edx
        js      .done
        mov     ebx, edx
        add     ebx, BALL_SIZE
        cmp     ebx, GAME_H
        jg      .done
        mov     ebx, [ball_x]
        imul    eax, edx, STRIDE
        imul    ecx, ebx, 3
        add     eax, ecx
        add     eax, screen_buffer
        xor     edx, edx

        .yl:
        push    eax
        xor     ecx, ecx

        .xl:
        cmp     edx, 0
        je      .bc
        cmp     edx, BALL_SIZE - 1
        je      .bc
        jmp     .bp

        .bc:
        cmp     ecx, 0
        je      .bs
        cmp     ecx, BALL_SIZE - 1
        je      .bs

        .bp:
        mov     byte [eax], 0xFF
        mov     byte [eax+1], 0xFF
        mov     byte [eax+2], 0xFF

        .bs:
        add     eax, 3
        inc     ecx
        cmp     ecx, BALL_SIZE
        jl      .xl
        pop     eax
        add     eax, STRIDE
        inc     edx
        cmp     edx, BALL_SIZE
        jl      .yl

        .done:
        popad
        ret

; --- DATA ---
DATA
        title           db 'Arkanoid Infinite', 0
        score_txt       db 'SCORE', 0
        lives_txt       db 'LIVES', 0
        level_txt       db 'LEVEL', 0
        over_txt        db 'GAME OVER', 0
        again_txt       db 'PLAY AGAIN?', 0
        y_n_txt         db 'Y/N', 0
        hs_title        db 'HIGH SCORES', 0
        new_hs_txt      db 'NEW HIGHSCORE!', 0
        paddle_x        dd 150
        ball_active     dd 0
        ball_x          dd 0
        ball_y          dd 0
        ball_dx         dd 3
        ball_dy         dd -4
        score           dd 0
        old_score       dd -1
        lives           dd 4
        old_lives       dd -1
        current_level   dd 1
        old_level       dd -1
        bricks_left     dd 0
        random_seed     dd 0x42069
        temp_y          dd 0
        input_mode      dd 0
        name_ptr        dd 0

file_info:
        .op             dd 0
        .pos            dd 0
        .pos_hi         dd 0
        .size           dd 0
        .buf            dd 0
        .rsvd           db 0
        .path           dd 0

        HS_FILE         db '/rd/1/GAMES/arkanoid.hs',0

i_end:
virtual at $
        hs_data:        rb 60
        active_map:     rb 256
        screen_buffer:  rb (GAME_W * GAME_H * 3) + 1024
        hs_temp_buf     rb 32
        score_buffer    rb 32
        lives_buffer    rb 32
        level_buffer    rb 32
        new_name        rb 16
        rb 4096
        stacktop:
        mem_end:
end virtual
