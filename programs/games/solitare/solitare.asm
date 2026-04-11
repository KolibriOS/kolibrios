;   SPDX-License-Identifier: GPL-2.0-only
;   Program - Solitaire card game
;   Copyright (C) 2026 KolibriOS team
;   Author: Max <snapemax@wp.pl>
;   Graphics: Designed by rawpixel.com / Freepik (see 'License free.txt')
;   http://www.freepik.com

use32
org 0x0

db 'MENUET01'
dd 0x01
dd start
dd i_end
dd mem_end
dd stacktop
dd 0, 0

include '../../macros.inc'
include '../../KOSfuncs.inc'

; --- CONSTANTS ---
CARD_W           equ 70
CARD_H           equ 100
STACK_OFFSET     equ 25
WIN_W            equ 760
WIN_H            equ 560
BPP              equ 3
STRIDE           equ WIN_W * BPP
BMP_W            equ 40
BMP_H            equ 60
BMP_BPP          equ 3
BMP_LINE         equ (BMP_W * BMP_BPP)
COL_TABLE        equ 0x207020
COL_REVERSE      equ 0x0000AA
COL_WHITE        equ 0xFFFFFF
COL_RED          equ 0xCC0000
COL_BLACK        equ 0x000000
COL_SLOT         equ 0x185018 ; Dark gray/green for slots
CARD_STRUCT_SIZE equ 12
CARD_VALUE       equ 0
CARD_SUIT        equ 1
CARD_STATE       equ 2
CARD_LOC         equ 3
CARD_X           equ 4
CARD_Y           equ 6
CARD_OLD_X       equ 8
CARD_OLD_Y       equ 10
STOCK_X          equ 20
WASTE_X          equ 100
FOUNDATION_X     equ 420
FOUND_Y          equ 20
SCORE_Y          equ 525
BTN_REPLAY       equ 100
BTN_EXIT         equ 101

start:
    mcall SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
    mov [skin_h], eax
    call init_game_state

event_loop:
    cmp byte [exit_request], 1
    je .force_exit
    mcall SF_CHECK_EVENT
    cmp eax, 1
    je on_redraw
    cmp eax, 2
    je on_key
    cmp eax, 3
    je on_button
    cmp byte [game_won], 1
    je .win_animation
    call handle_mouse
    jmp .check_redraw

.win_animation:
    call update_cascade
    mov byte [force_redraw], 1
    cmp byte [win_thread_active], 0
    jne .check_reset
    inc dword [win_timer]
    cmp dword [win_timer], 120
    jl .check_reset
    mov byte [win_thread_active], 1
    call create_win_thread

.check_reset:
    cmp byte [should_reset], 1
    jne .no_reset
    call init_game_state
    call draw_main_window
.no_reset:
    mcall SF_SLEEP, 1
    jmp update_screen_only

.check_redraw:
    cmp dword [active_card_count], 0
    ja update_screen_only
    cmp byte [force_redraw], 1
    je update_screen_only
    mcall SF_SLEEP, 1
    jmp event_loop

.force_exit:
    mcall SF_TERMINATE_PROCESS

on_key:
    mcall SF_GET_KEY
    jmp event_loop

on_button:
    mcall SF_GET_BUTTON
    shr eax, 8
    cmp eax, 1
    jne event_loop
    mcall SF_TERMINATE_PROCESS

on_redraw:
    call draw_main_window
    mov byte [force_redraw], 1
    jmp update_screen_only

update_screen_only:
    mov byte [force_redraw], 0
    call prepare_buffer
    mov edx, 4 shl 16
    add dx, word [skin_h]
    mcall SF_PUT_IMAGE, screen_buffer, WIN_W shl 16 + WIN_H
    jmp event_loop

init_game_state:
    call init_deck
    call shuffle_deck
    call layout_tableau
    mov dword [score], 0
    mov byte [game_won], 0
    mov dword [win_timer], 0
    mov byte [win_thread_active], 0
    mov byte [should_reset], 0
    mov byte [exit_request], 0
    mov dword [active_cascade_idx], -1
    mov dword [cascade_timer], 0
    ret

draw_main_window:
    mcall SF_REDRAW, SSF_BEGIN_DRAW
    mov ebx, 100 shl 16 + (WIN_W + 9)
    mov eax, [skin_h]
    add eax, WIN_H + 5
    mov ecx, 100 shl 16
    mov cx, ax
    mcall SF_CREATE_WINDOW, , , 0x14000000 or COL_TABLE, 0, title
    mcall SF_REDRAW, SSF_END_DRAW
    ret

create_win_thread:
    mcall SF_CREATE_THREAD, SSF_CREATE_THREAD, win_thread_entry, win_stack_top
    ret

win_thread_entry:
    mcall SF_THREAD_INFO, win_proc_info, -1
    mov eax, [win_proc_info + 34]
    add eax, 200
    shl eax, 16
    add eax, 300
    mov [win_pos_x], eax
    mov eax, [win_proc_info + 38]
    add eax, 180
    shl eax, 16
    add eax, 180
    mov [win_pos_y], eax
.redr:
    mcall SF_REDRAW, SSF_BEGIN_DRAW
    mcall SF_CREATE_WINDOW, [win_pos_x], [win_pos_y], 0x34FFFFFF, 0x80505050, title_win
    mcall SF_DRAW_TEXT, 70 shl 16 + 35, 0x80000000, msg_congrats
    mcall SF_DRAW_TEXT, 100 shl 16 + 65, 0x80000000, msg_score_lbl
    mov eax, [score]
    mov edi, score_str
    call int_to_string
    mcall SF_DRAW_TEXT, 160 shl 16 + 65, 0x80000000, score_str
    mcall SF_DEFINE_BUTTON, 40 shl 16 + 100, 100 shl 16 + 30, BTN_REPLAY, 0xCCCCCC
    mcall SF_DRAW_TEXT, 66 shl 16 + 110, 0x80000000, txt_replay
    mcall SF_DEFINE_BUTTON, 160 shl 16 + 100, 100 shl 16 + 30, BTN_EXIT, 0xCCCCCC
    mcall SF_DRAW_TEXT, 194 shl 16 + 110, 0x80000000, txt_exit
    mcall SF_REDRAW, SSF_END_DRAW
.wait_ev:
    mcall SF_WAIT_EVENT
    cmp eax, 1
    je .redr
    cmp eax, 3
    jne .wait_ev
    mcall SF_GET_BUTTON
    shr eax, 8
    cmp eax, BTN_EXIT
    je .trigger_exit
    cmp eax, BTN_REPLAY
    jne .wait_ev
.replay_game:
    mov byte [should_reset], 1
    mcall SF_TERMINATE_PROCESS
    jmp .wait_ev
.trigger_exit:
    mov byte [exit_request], 1
    mcall SF_TERMINATE_PROCESS

int_to_string:
    pushad
    mov ebx, 10
    xor ecx, ecx
    test eax, eax
    jnz .l1
    mov byte [edi], '0'
    inc edi
    jmp .fin
.l1: xor edx, edx
    div ebx
    add dl, '0'
    push edx
    inc ecx
    test eax, eax
    jnz .l1
.l2: pop edx
    mov [edi], dl
    inc edi
    loop .l2
.fin:
    mov byte [edi], 0
    popad
    ret

update_cascade:
    pushad
    inc dword [cascade_timer]
    cmp dword [cascade_timer], 3
    jl .move_active
    mov dword [cascade_timer], 0
    cmp dword [active_cascade_idx], 51
    jge .move_active
    inc dword [active_cascade_idx]
.move_active:
    mov ecx, [active_cascade_idx]
    cmp ecx, 0
    jl .done
.l:
    push ecx
    imul esi, ecx, CARD_STRUCT_SIZE
    add esi, deck_data
    imul edi, ecx, 4
    add edi, card_vectors
    inc word [edi+2]
    mov ax, [edi+2]
    add [esi+CARD_Y], ax
    mov ax, [edi]
    add [esi+CARD_X], ax
    mov ax, [esi+CARD_Y]
    add ax, CARD_H
    cmp ax, WIN_H
    jl .no_y_bounce
    mov word [esi+CARD_Y], WIN_H - CARD_H
    mov ax, [edi+2]
    neg ax
    sar ax, 1
    mov [edi+2], ax
.no_y_bounce:
    mov ax, [esi+CARD_X]
    cmp ax, 0
    jg .not_left
    mov word [esi+CARD_X], 0
    neg word [edi]
.not_left:
    add ax, CARD_W
    cmp ax, WIN_W
    jl .no_x_bounce
    mov word [esi+CARD_X], WIN_W - CARD_W
    neg word [edi]
.no_x_bounce:
    pop ecx
    dec ecx
    jns .l
.done:
    popad
    ret

init_cascade_vectors:
    pushad
    xor ecx, ecx
.l:
    imul edi, ecx, 4
    add edi, card_vectors
    mcall SF_GET_SYS_TIME
    push eax
    and eax, 0x0F
    sub eax, 7
    jnz .x_ok
    inc eax
.x_ok:
    mov [edi], ax
    pop eax
    shr eax, 4
    and eax, 0x07
    neg eax
    sub eax, 4
    mov [edi+2], ax
    inc ecx
    cmp ecx, 52
    jl .l
    popad
    ret

handle_mouse:
    pushad
    mcall SF_MOUSE_GET, SSF_WINDOW_POSITION
    mov ebp, eax
    shr ebp, 16
    sub ebp, 5
    and eax, 0xFFFF
    sub eax, [skin_h]
    mov edi, eax
    mcall SF_MOUSE_GET, SSF_BUTTON
    test eax, 1
    jnz .dragging_state
    cmp dword [mouse_lock], 1
    jne .exit
    mov dword [mouse_lock], 0
    cmp dword [active_card_count], 0
    je .exit
    call check_stack_move
    mov dword [active_card_count], 0
    mov byte [force_redraw], 1
    popad
    ret
.dragging_state:
    cmp dword [mouse_lock], 1
    je .no_deal_click
    cmp ebp, STOCK_X
    jb .find_card
    cmp ebp, STOCK_X+CARD_W
    jae .find_card
    cmp edi, FOUND_Y
    jb .find_card
    cmp edi, FOUND_Y+CARD_H
    jae .find_card
    mov dword [mouse_lock], 1
    call deal_from_stock
    mov byte [force_redraw], 1
    popad
    ret
.find_card:
    mov ebx, 51
.f_loop:
    imul edx, ebx, CARD_STRUCT_SIZE
    add edx, deck_data
    movsx esi, word [edx+CARD_X]
    movsx ecx, word [edx+CARD_Y]
    cmp ebp, esi
    jb .next_s
    add esi, CARD_W
    cmp ebp, esi
    jae .next_s
    cmp edi, ecx
    jb .next_s
    add ecx, CARD_H
    cmp edi, ecx
    jae .next_s
    test byte [edx+CARD_STATE], 1
    jz .next_s
    mov dword [mouse_lock], 1
    mov [active_stack], ebx
    mov dword [active_card_count], 1
    mov ax, word [edx+CARD_X]
    sub ax, bp
    mov [mouse_off_x], ax
    mov ax, word [edx+CARD_Y]
    sub ax, di
    mov [mouse_off_y], ax
    mov eax, ebx
    call build_active_stack
    popad
    ret
.next_s:
    dec ebx
    jns .f_loop
    popad
    ret
.no_deal_click:
    cmp dword [active_card_count], 0
    je .exit_drag
    xor ecx, ecx
.d_lp:
    cmp ecx, [active_card_count]
    jae .exit_drag
    mov eax, [active_stack + ecx*4]
    imul eax, CARD_STRUCT_SIZE
    add eax, deck_data
    mov dx, [mouse_off_x]
    add dx, bp
    mov [eax+CARD_X], dx
    mov dx, [mouse_off_y]
    add dx, di
    push eax
    mov eax, ecx
    imul eax, STACK_OFFSET
    add dx, ax
    pop eax
    mov [eax+CARD_Y], dx
    inc ecx
    jmp .d_lp
.exit_drag:
    popad
    ret
.exit:
    popad
    ret

prepare_buffer:
    mov edi, screen_buffer
    mov ecx, WIN_W * WIN_H
    mov eax, COL_TABLE
.clear_loop:
    mov [edi], al
    mov [edi+1], ah
    push eax
    shr eax, 16
    mov byte [edi+2], al
    pop eax
    add edi, 3
    dec ecx
    jnz .clear_loop
    call draw_slots
    call render_score_ui
    xor ecx, ecx
.draw_loop:
    call is_card_active
    jc .skip
    push ecx
    imul esi, ecx, CARD_STRUCT_SIZE
    add esi, deck_data
    call draw_card_to_buffer
    pop ecx
.skip:
    inc ecx
    cmp ecx, 52
    jl .draw_loop
    xor ecx, ecx
.draw_active:
    cmp ecx, [active_card_count]
    jae .done_rendering
    push ecx
    mov eax, [active_stack + ecx*4]
    imul esi, eax, CARD_STRUCT_SIZE
    add esi, deck_data
    call draw_card_to_buffer
    pop ecx
    inc ecx
    jmp .draw_active
.done_rendering:
    ret

draw_card_to_buffer:
    pushad
    movsx ebx, word [esi+CARD_X]
    movsx ecx, word [esi+CARD_Y]
    mov edx, COL_BLACK
    mov edi, CARD_W
    mov ebp, CARD_H
    call fill_rect_buf
    inc ebx
    inc ecx
    mov edx, COL_WHITE
    mov edi, CARD_W-2
    mov ebp, CARD_H-2
    call fill_rect_buf
    test byte [esi+CARD_STATE], 1
    jnz .face_up
    add ebx, 4
    add ecx, 4
    mov edx, COL_REVERSE
    mov edi, CARD_W-10
    mov ebp, CARD_H-10
    call fill_rect_buf
    popad
    ret

.face_up:
    mov edx, COL_RED
    cmp byte [esi+CARD_SUIT], 2
    jb .is_red
    mov edx, COL_BLACK
.is_red:
    push edx
    push esi
    movzx eax, byte [esi+CARD_VALUE]
    dec eax
    push esi
    mov esi, eax
    push ebx
    push ecx
    add ebx, 4
    add ecx, 4
    call draw_char_to_buf
    pop ecx
    pop ebx
    pop esi
    push esi
    movzx eax, byte [esi+CARD_SUIT]
    add eax, 13
    push ebx
    push ecx
    add ebx, 10
    add ecx, 4
    mov esi, eax
    call draw_char_to_buf
    pop ecx
    pop ebx
    pop esi
    push esi
    movzx eax, byte [esi+CARD_SUIT]
    add eax, 13
    push ebx
    push ecx
    add ebx, CARD_W - 15
    add ecx, CARD_H - 11
    mov esi, eax
    call draw_char_to_buf
    pop ecx
    pop ebx
    pop esi
    movzx eax, byte [esi+CARD_VALUE]
    dec eax
    push esi
    mov esi, eax
    push ebx
    push ecx
    add ebx, CARD_W - 9
    add ecx, CARD_H - 11
    call draw_char_to_buf
    pop ecx
    pop ebx
    pop esi
    pop esi
    pop edx

    pushad
    movzx eax, byte [esi+CARD_VALUE]
    movzx edx, byte [esi+CARD_SUIT]

    movsx ebx, word [esi+CARD_X]
    add ebx, 15
    movsx ecx, word [esi+CARD_Y]
    add ecx, 20

    cmp eax, 11
    je .face_jack
    cmp eax, 12
    je .face_queen
    cmp eax, 13
    je .face_king
    jmp .patterns

.face_jack:
    cmp edx, 0
    je .set_jp
    cmp edx, 1
    je .set_jd
    cmp edx, 2
    je .set_jh
    mov esi, db_jt
    jmp .render_face
    .set_jp: mov esi, db_jp
    jmp .render_face
    .set_jd: mov esi, db_jd
    jmp .render_face
    .set_jh: mov esi, db_jh
    jmp .render_face

.face_queen:
    cmp edx, 0
    je .set_qp
    cmp edx, 1
    je .set_qd
    cmp edx, 2
    je .set_qh
    mov esi, db_queen
    jmp .render_face
    .set_qp: mov esi, db_qp
    jmp .render_face
    .set_qd: mov esi, db_qd
    jmp .render_face
    .set_qh: mov esi, db_qh
    jmp .render_face

.face_king:
    cmp edx, 0
    je .set_kp
    cmp edx, 1
    je .set_kd
    cmp edx, 2
    je .set_kh
    mov esi, db_kt
    jmp .render_face
    .set_kp: mov esi, db_kp
    jmp .render_face
    .set_kd: mov esi, db_kkaro
    jmp .render_face
    .set_kh: mov esi, db_kh
    jmp .render_face

.render_face:
    call draw_bitmap_to_buf
    jmp .face_done

.patterns:
    mov edx, COL_RED
    cmp byte [esi+CARD_SUIT], 2
    jb .set_center_color
    mov edx, COL_BLACK
.set_center_color:
    dec eax
    mov eax, [pattern_offsets + eax*4]
    add eax, card_patterns
    mov ebp, eax
.pat_loop:
    mov al, [ebp]
    cmp al, 0xFF
    je .face_done
    push edx
    movzx ebx, byte [ebp]
    add bx, word [esi+CARD_X]
    movzx ecx, byte [ebp+1]
    add cx, word [esi+CARD_Y]
    movzx eax, byte [esi+CARD_SUIT]
    add eax, 13
    push esi
    mov esi, eax
    call draw_char_to_buf
    pop esi
    pop edx
    add ebp, 2
    jmp .pat_loop

.face_done:
    popad
    popad
    ret

draw_bitmap_to_buf:
    pushad
    add ecx, BMP_H
    dec ecx
    mov dword [temp_counter2], BMP_H
.row_loop:
    cmp ecx, 0
    jl .skip_row
    cmp ecx, WIN_H
    jge .skip_row
    push ebx
    mov eax, ecx
    imul eax, STRIDE
    add eax, screen_buffer
    mov ebp, BMP_W
.pixel_loop:
    cmp ebx, 0
    jl .next_p
    cmp ebx, WIN_W
    jge .next_p
    lea edi, [ebx*2 + ebx]
    mov dl, [esi]
    mov [eax+edi], dl
    mov dl, [esi+1]
    mov [eax+edi+1], dl
    mov dl, [esi+2]
    mov [eax+edi+2], dl
.next_p:
    add esi, 3
    inc ebx
    dec ebp
    jnz .pixel_loop
    pop ebx
    jmp .row_finish
.skip_row:
    add esi, BMP_LINE
.row_finish:
    dec ecx
    dec dword [temp_counter2]
    jnz .row_loop
    popad
    ret

fill_rect_buf:
    pushad
    mov esi, ebp
.yl:
    cmp ecx, 0
    jl .skip_line
    cmp ecx, WIN_H
    jge .skip_line
    push ecx
    push ebx
    mov eax, ecx
    imul eax, STRIDE
    add eax, screen_buffer
    mov dword [temp_counter], edi
.xl:
    cmp ebx, 0
    jl .skip_pixel
    cmp ebx, WIN_W
    jge .skip_pixel
    push eax
    mov ecx, ebx
    imul ecx, 3
    add eax, ecx
    mov [eax], dl
    mov [eax+1], dh
    push edx
    shr edx, 16
    mov [eax+2], dl
    pop edx
    pop eax
.skip_pixel:
    inc ebx
    dec dword [temp_counter]
    jnz .xl
    pop ebx
    pop ecx
.skip_line:
    inc ecx
    dec esi
    jnz .yl
    popad
    ret

draw_char_to_buf:
    pushad
    imul esi, 7
    add esi, font_5x7
    mov ebp, 7
.y_loop:
    push ebx
    push ecx
    mov al, [esi]
    mov ah, 10000000b
    mov dword [temp_counter], 5
.x_loop:
    test al, ah
    jz .skip_pixel
    cmp ebx, 0
    jl .skip_pixel
    cmp ebx, WIN_W
    jge .skip_pixel
    cmp ecx, 0
    jl .skip_pixel
    cmp ecx, WIN_H
    jge .skip_pixel
    push eax
    mov eax, ecx
    imul eax, STRIDE
    push ebx
    imul ebx, 3
    add eax, ebx
    pop ebx
    add eax, screen_buffer
    mov [eax], dl
    mov [eax+1], dh
    push edx
    shr edx, 16
    mov [eax+2], dl
    pop edx
    pop eax
.skip_pixel:
    shr ah, 1
    inc ebx
    dec dword [temp_counter]
    jnz .x_loop
    pop ecx
    pop ebx
    inc esi
    inc ecx
    dec ebp
    jnz .y_loop
    popad
    ret

draw_large_char_to_buf:
    pushad
    imul esi, 12
    add esi, font_8x12
    mov ebp, 12
.y_loop:
    push ebx
    push ecx
    mov al, [esi]
    mov ah, 10000000b
    mov dword [temp_counter], 8
.x_loop:
    test al, ah
    jz .skip_pixel
    cmp ebx, 0
    jl .skip_pixel
    cmp ebx, WIN_W
    jge .skip_pixel
    cmp ecx, 0
    jl .skip_pixel
    cmp ecx, WIN_H
    jge .skip_pixel
    push eax
    mov eax, ecx
    imul eax, STRIDE
    push ebx
    imul ebx, 3
    add eax, ebx
    pop ebx
    add eax, screen_buffer
    mov [eax], dl
    mov [eax+1], dh
    push edx
    shr edx, 16
    mov [eax+2], dl
    pop edx
    pop eax
.skip_pixel:
    shr ah, 1
    inc ebx
    dec dword [temp_counter]
    jnz .x_loop
    pop ecx
    pop ebx
    inc esi
    inc ecx
    dec ebp
    jnz .y_loop
    popad
    ret

render_score_ui:
    mov edx, COL_BLACK
    mov ebx, 480
    mov ecx, SCORE_Y
    xor esi, esi
    call draw_large_char_to_buf
    add ebx, 10
    mov esi, 1
    call draw_large_char_to_buf
    add ebx, 10
    mov esi, 2
    call draw_large_char_to_buf
    add ebx, 10
    mov esi, 3
    call draw_large_char_to_buf
    add ebx, 10
    mov esi, 4
    call draw_large_char_to_buf
    add ebx, 20
    mov eax, [score]
    test eax, eax
    jns .pos
    neg eax
    push eax
    mov ecx, SCORE_Y
    mov esi, 5
    call draw_large_char_to_buf
    add ebx, 10
    pop eax
.pos:
    mov edi, 10
    xor ecx, ecx
.dec_loop:
    xor edx, edx
    div edi
    push edx
    inc ecx
    test eax, eax
    jnz .dec_loop
.print_dec:
    pop edx
    add edx, 6
    push ecx
    mov ecx, SCORE_Y
    mov esi, edx
    mov edx, COL_BLACK
    call draw_large_char_to_buf
    add ebx, 10
    pop ecx
    loop .print_dec
    ret

draw_slots:
    pushad
    ; Foundation slots (Top Right)
    mov ebx, FOUNDATION_X
    mov ecx, FOUND_Y
    mov dword [temp_counter2], 4
.f_loop:
    mov edx, COL_SLOT
    mov edi, CARD_W
    mov ebp, CARD_H
    call fill_rect_buf
    add ebx, 80
    dec dword [temp_counter2]
    jnz .f_loop

    ; Stock slot (Top Left)
    mov ebx, STOCK_X
    mov ecx, FOUND_Y
    mov edx, COL_SLOT
    mov edi, CARD_W
    mov ebp, CARD_H
    call fill_rect_buf

    ; Waste slot (Next to Stock)
    mov ebx, WASTE_X
    mov ecx, FOUND_Y
    mov edx, COL_SLOT
    mov edi, CARD_W
    mov ebp, CARD_H
    call fill_rect_buf

    ; Tableau slots (Bottom rows)
    mov ebx, 40
    mov ecx, 140
    mov dword [temp_counter2], 7
.t_loop:
    mov edx, COL_SLOT
    mov edi, CARD_W
    mov ebp, CARD_H
    call fill_rect_buf
    add ebx, 105
    dec dword [temp_counter2]
    jnz .t_loop
    popad
    ret

is_card_active:
    push eax
    push ebx
    xor ebx, ebx
.l:
    cmp ebx, [active_card_count]
    jae .no
    mov eax, [active_stack + ebx*4]
    cmp eax, ecx
    je .yes
    inc ebx
    jmp .l
.yes:
    pop ebx
    pop eax
    stc
    ret
.no:
    pop ebx
    pop eax
    clc
    ret

build_active_stack:
    pushad
    imul esi, eax, CARD_STRUCT_SIZE
    add esi, deck_data
    mov bx, [esi+CARD_X]
    mov dx, [esi+CARD_Y]
    movzx ebp, dx
    add ebp, STACK_OFFSET
.y_search:
    xor ecx, ecx
.l:
    imul edi, ecx, CARD_STRUCT_SIZE
    add edi, deck_data
    cmp word [edi+CARD_X], bx
    jne .nx
    cmp word [edi+CARD_Y], bp
    jne .nx
    mov edx, [active_card_count]
    mov [active_stack + edx*4], ecx
    inc dword [active_card_count]
    add ebp, STACK_OFFSET
    jmp .y_search
.nx:
    inc ecx
    cmp ecx, 52
    jl .l
.done:
    xor ecx, ecx
.save:
    cmp ecx, [active_card_count]
    jae .fin
    mov eax, [active_stack + ecx*4]
    imul eax, CARD_STRUCT_SIZE
    add eax, deck_data
    mov dx, [eax+CARD_X]
    mov [eax+CARD_OLD_X], dx
    mov dx, [eax+CARD_Y]
    mov [eax+CARD_OLD_Y], dx
    inc ecx
    jmp .save
.fin:
    popad
    ret

check_stack_move:
    pushad
    mov esi, [active_stack]
    imul esi, CARD_STRUCT_SIZE
    add esi, deck_data
    cmp dword [active_card_count], 1
    jne .tableau_check
    mov ebx, FOUNDATION_X
    xor ecx, ecx
.f_lp:
    movsx eax, word [esi+CARD_X]
    sub eax, ebx
    add eax, 35
    cmp eax, 70
    ja .next_f
    movsx eax, word [esi+CARD_Y]
    sub eax, FOUND_Y
    add eax, 50
    cmp eax, 100
    ja .next_f
    call check_foundation_logic
    jc .found_match
.next_f:
    add ebx, 80
    inc ecx
    cmp ecx, 4
    jl .f_lp
.tableau_check:
    mov ebx, 40
.col_lp:
    xor ecx, ecx
    mov dword [temp_val], -1
    mov dword [temp_counter], 0
.find_l:
    imul edi, ecx, CARD_STRUCT_SIZE
    add edi, deck_data
    push ecx
    call is_card_active
    pop ecx
    jc .sk_l
    cmp word [edi+CARD_X], bx
    jne .sk_l
    movzx eax, word [edi+CARD_Y]
    cmp eax, [temp_counter]
    jl .sk_l
    mov [temp_counter], eax
    mov [temp_val], ecx
.sk_l:
    inc ecx
    cmp ecx, 52
    jl .find_l
    cmp dword [temp_val], -1
    je .empty_col
    mov ecx, [temp_val]
    imul edi, ecx, CARD_STRUCT_SIZE
    add edi, deck_data
    movsx eax, word [esi+CARD_X]
    movsx edx, word [edi+CARD_X]
    sub eax, edx
    add eax, 35
    cmp eax, 70
    ja .nx_col
    movsx eax, word [esi+CARD_Y]
    movsx edx, word [edi+CARD_Y]
    sub eax, edx
    add eax, 50
    cmp eax, 100
    ja .nx_col
    mov al, [esi+CARD_SUIT]
    mov dl, [edi+CARD_SUIT]
    shr al, 1
    shr dl, 1
    cmp al, dl
    je .nx_col
    mov al, [esi+CARD_VALUE]
    mov dl, [edi+CARD_VALUE]
    inc al
    cmp al, dl
    jne .nx_col
    mov dx, word [edi+CARD_Y]
    add dx, STACK_OFFSET
    jmp .apply_move
.empty_col:
    movsx eax, word [esi+CARD_X]
    sub eax, ebx
    add eax, 35
    cmp eax, 70
    ja .nx_col
    movsx eax, word [esi+CARD_Y]
    sub eax, 140
    add eax, 50
    cmp eax, 100
    ja .nx_col
    cmp byte [esi+CARD_VALUE], 13
    jne .nx_col
    mov dx, 140
    jmp .apply_move
.nx_col:
    add ebx, 105
    cmp ebx, 750
    jl .col_lp
    xor ecx, ecx
.back:
    cmp ecx, [active_card_count]
    jae .out
    mov eax, [active_stack + ecx*4]
    imul eax, CARD_STRUCT_SIZE
    add eax, deck_data
    mov dx, [eax+CARD_OLD_X]
    mov [eax+CARD_X], dx
    mov dx, [eax+CARD_OLD_Y]
    mov [eax+CARD_Y], dx
    inc ecx
    jmp .back
.out:
    popad
    ret
.found_match:
    test byte [esi+CARD_STATE], 0x80
    jnz .already_scored
    add dword [score], 10
    or byte [esi+CARD_STATE], 0x80
.already_scored:
    mov word [esi+CARD_X], bx
    mov word [esi+CARD_Y], FOUND_Y
    mov byte [esi+CARD_LOC], 8
    pushad
    mov eax, [active_stack]
    mov [temp_active], eax
    call move_single_to_top
    popad
    call auto_reveal_all
    call check_win
    popad
    ret
.apply_move:
    xor ecx, ecx
    push eax
    push edx
    mov eax, ebx
    sub eax, 40
    xor edx, edx
    mov edi, 105
    div edi
    mov [temp_val], eax
    pop edx
    pop eax
    mov [temp_counter2], edx
.sticky_loop:
    cmp ecx, [active_card_count]
    jae .sticky_done
    mov eax, [active_stack + ecx*4]
    imul eax, CARD_STRUCT_SIZE
    add eax, deck_data
    mov word [eax+CARD_X], bx
    mov dx, word [temp_counter2]
    mov word [eax+CARD_Y], dx
    mov dl, byte [temp_val]
    mov byte [eax+CARD_LOC], dl
    add word [temp_counter2], STACK_OFFSET
    inc ecx
    jmp .sticky_loop
.sticky_done:
    xor ecx, ecx
.top_loop:
    cmp ecx, [active_card_count]
    jae .ok
    push ecx
    mov eax, [active_stack + ecx*4]
    mov [temp_active], eax
    call move_single_to_top
    call update_active_stack_after_move
    pop ecx
    inc ecx
    jmp .top_loop
.ok:
    call auto_reveal_all
    popad
    ret

update_active_stack_after_move:
    pushad
    mov edx, [temp_active]
    xor ecx, ecx
.up_lp:
    cmp ecx, [active_card_count]
    jae .up_dn
    mov eax, [active_stack + ecx*4]
    cmp eax, edx
    jbe .no_chg
    dec eax
    mov [active_stack + ecx*4], eax
.no_chg:
    inc ecx
    jmp .up_lp
.up_dn:
    popad
    ret

check_foundation_logic:
    pushad
    xor eax, eax
    mov dword [temp_val], 0
    mov dword [temp_counter], -1
    xor ebp, ebp
.find_top:
    imul edi, ebp, CARD_STRUCT_SIZE
    add edi, deck_data
    cmp word [edi+CARD_X], bx
    jne .nx_c
    cmp word [edi+CARD_Y], FOUND_Y
    jne .nx_c
    movzx edx, byte [edi+CARD_VALUE]
    cmp edx, [temp_val]
    jle .nx_c
    mov [temp_val], edx
    mov [temp_counter], ebp
.nx_c:
    inc ebp
    cmp ebp, 52
    jl .find_top
    mov al, [esi+CARD_VALUE]
    mov dl, [esi+CARD_SUIT]
    cmp dword [temp_counter], -1
    jne .not_empty
    cmp al, 1
    je .valid
    jmp .invalid
.not_empty:
    mov edi, [temp_counter]
    imul edi, CARD_STRUCT_SIZE
    add edi, deck_data
    cmp dl, [edi+CARD_SUIT]
    jne .invalid
    mov dl, [edi+CARD_VALUE]
    inc dl
    cmp al, dl
    je .valid
.invalid:
    popad
    clc
    ret
.valid:
    popad
    stc
    ret

check_win:
    pushad
    xor ecx, ecx
    xor eax, eax
.l:
    imul esi, ecx, CARD_STRUCT_SIZE
    add esi, deck_data
    cmp byte [esi+CARD_LOC], 8
    jne .no
    inc eax
.no:
    inc ecx
    cmp ecx, 52
    jl .l
    cmp eax, 52
    jne .out
    cmp byte [game_won], 0
    jne .out
    add dword [score], 100
    mov byte [game_won], 1
    call init_cascade_vectors
.out:
    popad
    ret

auto_reveal_all:
    pushad
    mov ebx, 40
.col_loop:
    xor esi, esi
    mov dword [temp_val], -1
    xor ecx, ecx
.find_lowest:
    imul edi, ecx, CARD_STRUCT_SIZE
    add edi, deck_data
    cmp word [edi+CARD_X], bx
    jne .next_card
    movzx eax, word [edi+CARD_Y]
    cmp eax, [temp_val]
    jle .next_card
    mov [temp_val], eax
    mov esi, edi
.next_card:
    inc ecx
    cmp ecx, 52
    jl .find_lowest
    cmp dword [temp_val], -1
    je .next_col
    or byte [esi+CARD_STATE], 1
.next_col:
    add ebx, 105
    cmp ebx, 750
    jl .col_loop
    popad
    ret

move_single_to_top:
    mov eax, [temp_active]
    cmp eax, 51
    je .done
    imul esi, eax, CARD_STRUCT_SIZE
    add esi, deck_data
    mov edi, temp_card_buf
    mov ecx, 3
    cld
    rep movsd
    mov eax, [temp_active]
    inc eax
    imul esi, eax, CARD_STRUCT_SIZE
    add esi, deck_data
    imul edi, [temp_active], CARD_STRUCT_SIZE
    add edi, deck_data
    mov ecx, 51
    sub ecx, [temp_active]
    imul ecx, 3
    rep movsd
    mov esi, temp_card_buf
    mov edi, deck_data + (51 * CARD_STRUCT_SIZE)
    mov ecx, 3
    rep movsd
.done: ret

deal_from_stock:
    pushad
    mov ecx, 51
.f:
    imul esi, ecx, CARD_STRUCT_SIZE
    add esi, deck_data
    cmp byte [esi+CARD_LOC], 7
    jne .n
    test byte [esi+CARD_STATE], 1
    jnz .n
.o:
    or byte [esi+CARD_STATE], 1
    mov byte [esi+CARD_LOC], 9
    mov word [esi+CARD_X], WASTE_X
    mov word [esi+CARD_Y], FOUND_Y
    mov [temp_active], ecx
    call move_single_to_top
    popad
    ret
.n: dec ecx
    jns .f
    call reset_stock
    popad
    ret

reset_stock:
    sub dword [score], 20
    xor ecx, ecx
.l:
    imul esi, ecx, CARD_STRUCT_SIZE
    add esi, deck_data
    cmp byte [esi+CARD_LOC], 9
    jne .s
    and byte [esi+CARD_STATE], 0xFE
    mov byte [esi+CARD_LOC], 7
    mov word [esi+CARD_X], STOCK_X
    mov word [esi+CARD_Y], FOUND_Y
.s: inc ecx
    cmp ecx, 52
    jl .l
    ret

init_deck:
    mov edi, deck_data
    xor eax, eax
.s: xor ebx, ebx
.r: inc ebx
    mov byte [edi+CARD_VALUE], bl
    mov byte [edi+CARD_SUIT], al
    mov byte [edi+CARD_STATE], 0
    mov byte [edi+CARD_LOC], 7
    add edi, CARD_STRUCT_SIZE
    cmp ebx, 13
    jne .r
    inc eax
    cmp eax, 4
    jne .s
    ret

; --- PRNG (Linear Congruential Generator) ---
; Uses constants from Numerical Recipes: a=1103515245, c=12345
seed_rng:
    mcall SF_GET_SYS_TIME
    imul eax, eax, 1103515245
    add eax, 12345
    mov [rng_state], eax
    ret

; Returns pseudo-random value in eax
rng_next:
    mov eax, [rng_state]
    imul eax, eax, 1103515245
    add eax, 12345
    mov [rng_state], eax
    shr eax, 16
    and eax, 0x7FFF
    ret

shuffle_deck:
    call seed_rng
    mov dword [temp_counter2], 51
.sh:
    call rng_next
    xor edx, edx
    mov ebx, [temp_counter2]
    inc ebx
    div ebx
    imul esi, [temp_counter2], CARD_STRUCT_SIZE
    add esi, deck_data
    imul edi, edx, CARD_STRUCT_SIZE
    add edi, deck_data
    mov ecx, 3
.sw:
    mov eax, [esi]
    mov edx, [edi]
    mov [esi], edx
    mov [edi], eax
    add esi, 4
    add edi, 4
    loop .sw
    dec dword [temp_counter2]
    jnz .sh
    ret

layout_tableau:
    mov edi, deck_data
    mov ebx, 40
    xor edx, edx
.cols:
    mov ecx, 140
    mov eax, edx
    inc eax
    mov ebp, eax
.cards:
    mov word [edi+CARD_X], bx
    mov word [edi+CARD_Y], cx
    mov byte [edi+CARD_LOC], dl
    dec ebp
    jnz .h
    or byte [edi+CARD_STATE], 1
    jmp .nx
.h:
    and byte [edi+CARD_STATE], 0xFE
.nx:
    add edi, CARD_STRUCT_SIZE
    add ecx, STACK_OFFSET
    cmp edx, 6
    je .cl
    cmp ebp, 0
    jne .cards
    add ebx, 105
    inc edx
    jmp .cols
.cl:
    cmp ebp, 0
    jne .cards
.sf:
    cmp edi, deck_data + (52 * CARD_STRUCT_SIZE)
    jae .fin
    mov word [edi+CARD_X], STOCK_X
    mov word [edi+CARD_Y], FOUND_Y
    mov byte [edi+CARD_LOC], 7
    and byte [edi+CARD_STATE], 0xFE
    add edi, CARD_STRUCT_SIZE
    jmp .sf
.fin: ret

; --- DATA ---
title            db 'Solitare V1.6', 0
title_win        db 'Winner!', 0
msg_congrats     db 'Congrats You Win', 0
msg_score_lbl    db 'SCORE: ', 0
txt_replay       db 'REPLAY', 0
txt_exit         db 'EXIT', 0


; ... Font Data ...
font_8x12:
db 01111100b,10000010b,10000000b,10000000b,01111100b,00000010b,00000010b,10000010b,01111100b,00000000b,00000000b,00000000b
db 01111100b,10000010b,10000000b,10000000b,10000000b,10000000b,10000000b,10000010b,01111100b,00000000b,00000000b,00000000b
db 01111100b,10000010b,10000010b,10000010b,10000010b,10000010b,10000010b,10000010b,01111100b,00000000b,00000000b,00000000b
db 11111100b,10000010b,10000010b,10000010b,11111100b,10010000b,10001000b,10000100b,10000010b,00000000b,00000000b,00000000b
db 11111110b,10000000b,10000000b,10000000b,11111100b,10000000b,10000000b,10000000b,11111110b,00000000b,00000000b,00000000b
db 00000000b,00000000b,00000000b,00000000b,11111111b,00000000b,00000000b,00000000b,00000000b,00000000b,00000000b,00000000b
db 00111100b,01000010b,10000001b,10000001b,10000001b,10000001b,10000001b,01000010b,00111100b,00000000b,00000000b,00000000b
db 00011000b,00111000b,00001000b,00001000b,00001000b,00001000b,00001000b,00001000b,00111100b,00000000b,00000000b,00000000b
db 01111100b,10000010b,00000010b,00000010b,01111100b,10000000b,10000000b,10000000b,11111110b,00000000b,00000000b,00000000b
db 01111100b,10000010b,00000010b,00000010b,00111100b,00000010b,00000010b,10000010b,01111100b,00000000b,00000000b,00000000b
db 00001000b,00011000b,00101000b,01001000b,10001000b,11111111b,00001000b,00001000b,00001000b,00000000b,00000000b,00000000b
db 11111110b,10000000b,10000000b,11111100b,00000010b,00000010b,00000010b,10000010b,01111100b,00000000b,00000000b,00000000b
db 01111100b,10000010b,10000000b,11111100b,10000010b,10000010b,10000010b,10000010b,01111100b,00000000b,00000000b,00000000b
db 11111110b,00000010b,00000100b,00001000b,00010000b,00100000b,00100000b,00100000b,00100000b,00000000b,00000000b,00000000b
db 01111100b,10000010b,10000010b,10000010b,01111100b,10000010b,10000010b,10000010b,01111100b,00000000b,00000000b,00000000b
db 01111100b,10000010b,10000010b,10000010b,10000010b,01111110b,00000010b,10000010b,01111100b,00000000b,00000000b,00000000b

font_5x7:
db 01110000b,10001000b,10001000b,11111000b,10001000b,10001000b,10001000b
db 11110000b,00001000b,00010000b,00100000b,01000000b,10000000b,11111000b
db 11110000b,00001000b,00001000b,01110000b,00001000b,00001000b,11111000b
db 10001000b,10001000b,10001000b,11111000b,00001000b,00001000b,00001000b
db 11111000b,10000000b,10000000b,11110000b,00001000b,00001000b,11111000b
db 01110000b,10000000b,10000000b,11110000b,10001000b,10001000b,01110000b
db 11111000b,00001000b,00010000b,00100000b,01000000b,01000000b,01000000b
db 01110000b,10001000b,10001000b,01110000b,10001000b,10001000b,01110000b
db 01110000b,10001000b,10001000b,01111000b,00001000b,00001000b,01110000b
db 10111000b,10101000b,10101000b,10101000b,10101000b,10101000b,10111000b
db 00011000b,00001000b,00001000b,00001000b,00001000b,10001000b,01110000b
db 01110000b,10001000b,10001000b,10001000b,10101000b,10010000b,01101000b
db 10001000b,10010000b,10100000b,11000000b,10100000b,10010000b,10001000b
db 01010000b,11111000b,11111000b,01110000b,00100000b,00000000b,00000000b
db 00100000b,01110000b,11111000b,11111000b,01110000b,00100000b,00000000b
db 00100000b,00100000b,11111000b,11111000b,00100000b,00100000b,01110000b
db 00100000b,01110000b,11111000b,11111000b,01110000b,00100000b,01110000b

pattern_offsets:
dd p1-card_patterns, p2-card_patterns, p3-card_patterns, p4-card_patterns, p5-card_patterns
dd p6-card_patterns, p7-card_patterns, p8-card_patterns, p9-card_patterns, p10-card_patterns
card_patterns:
p1  db 32,46, 0xFF
p2  db 32,20, 32,72, 0xFF
p3  db 32,20, 32,46, 32,72, 0xFF
p4  db 18,20, 47,20, 18,72, 47,72, 0xFF
p5  db 18,20, 47,20, 18,72, 47,72, 32,46, 0xFF
p6  db 18,20, 47,20, 18,46, 47,46, 18,72, 47,72, 0xFF
p7  db 18,20, 47,20, 18,46, 47,46, 18,72, 47,72, 32,33, 0xFF
p8  db 18,20, 47,20, 18,46, 47,46, 18,72, 47,72, 32,33, 32,59, 0xFF
p9  db 18,20, 47,20, 18,40, 47,40, 18,60, 47,60, 18,80, 47,80, 32,50, 0xFF
p10 db 18,20, 47,20, 18,40, 47,40, 18,60, 47,60, 18,80, 47,80, 32,30, 32,70, 0xFF

align 4
; --- Bitmap Data ---
; Suit mapping: 0=hearts(red), 1=diamonds(red), 2=clubs(black), 3=spades(black)
; File naming: h=hearts, d=diamonds, t=trefl/clubs, p=pik/spades
db_jp:      file 'jh.bmp':54
db_jh:      file 'jt.bmp':54
db_jd:      file 'jd.bmp':54
db_jt:      file 'jp.bmp':54
db_qp:      file 'qh.bmp':54
db_qh:      file 'qt.bmp':54
db_qd:      file 'qd.bmp':54
db_queen:   file 'qp.bmp':54
db_kp:      file 'kh.bmp':54
db_kh:      file 'kt.bmp':54
db_kkaro:   file 'kd.bmp':54
db_kt:      file 'kp.bmp':54


i_end:

score            rd 1
game_won         rb 1
win_timer        rd 1
win_thread_active rb 1
should_reset     rb 1
exit_request     rb 1
mouse_lock       rd 1
force_redraw     rb 1
temp_counter     rd 1
temp_counter2    rd 1
temp_val         rd 1
mouse_off_x      rw 1
mouse_off_y      rw 1
temp_active      rd 1
temp_card_buf    rd 3
active_card_count rd 1
active_stack     rd 13
skin_h           rd 1
cascade_timer    rd 1
active_cascade_idx rd 1
win_pos_x        rd 1
win_pos_y        rd 1
rng_state        rd 1

align 4
deck_data:       rb 52 * CARD_STRUCT_SIZE
card_vectors:    rw 52 * 2
win_proc_info:   rb 1024
score_str        rb 16

align 64
stack_bottom:    rb 16384
stacktop:

align 64
win_stack_bottom: rb 8192
win_stack_top:

screen_buffer:   rb WIN_W * WIN_H * 3 + 32768
mem_end:
