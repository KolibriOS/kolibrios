; SPDX-License-Identifier: GPL-2.0-only
;
; Program - Freecell card game
; Copyright (C) 2026 KolibriOS team
;
; Authors: Max <snapemax@wp.pl>

use32
org 0x0

db      "MENUET01"
dd      0x01
dd      start
dd      i_end
dd      mem_end
dd      stacktop
dd      0, 0

include "../../macros.inc"
include "../../KOSfuncs.inc"
include "../../encoding.inc"

; --- CONSTANTS ---

NUM_COLS        = 8
NUM_SLOTS       = 4
NUM_SUITS       = 4
NUM_VALUES      = 13
NUM_CARDS       = NUM_SUITS * NUM_VALUES

BORDER          = 5
PADDING         = 16
OFFSET          = 25

CARD_W          = 72
CARD_H          = 96

SPACING         = PADDING + CARD_W
TABLEAU_Y       = PADDING * 2 + CARD_H
FC_X            = PADDING
FOUNDATION_X    = PADDING + 4 * SPACING

SCREEN_W        = PADDING * 9 + CARD_W * 8
SCREEN_H        = PADDING * 3 + CARD_H * 2 + OFFSET * 12
SCREEN_BYTES    = SCREEN_W * SCREEN_H * 4

COL_TABLE       = 0x207020
COL_WHITE       = 0xFFFFFF
COL_RED         = 0xCC0000
COL_BLACK       = 0x000000
COL_SLOT        = 0x185018

TEXT_FONT       = 0x89000000    ; A=1, font=6x9 cp866, C=2
WIN_TYPE        = 0x34000000    ; skinned, fixed size
struct CARD
        VALUE   db ?
        SUIT    db ?
        STATE   db ? ; 0: Tableau, 1: FreeCell, 2: Foundation
        SLOT_ID db ?
        X       dw ?
        Y       dw ?
ends

start:
        cld
        mcall   SF_SET_EVENTS_MASK, EVM_REDRAW or EVM_KEY or EVM_BUTTON or EVM_MOUSE or EVM_MOUSE_FILTER
        mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1
        call    restart_game
        jmp     draw_base

event_loop:
        mcall   SF_WAIT_EVENT
        cmp     al, EV_REDRAW
        je      draw_base
        cmp     al, EV_BUTTON
        je      quit_app
        cmp     al, EV_KEY
        je      handle_key
        cmp     al, EV_MOUSE
        jne     redraw_if_needed
        cmp     byte [game_won], 1
        je      redraw_if_needed
        call    handle_mouse
        jmp     redraw_if_needed

handle_key:
        mcall   SF_GET_KEY
        test    ah, 0x80               ; ignore key release
        jnz     redraw_if_needed
        cmp     ah, 0x13               ; R scancode
        jne     redraw_if_needed
        call    restart_game
        jmp     redraw_if_needed

draw_base:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
        add     eax, SCREEN_H + BORDER
        mov     esi, eax

        mcall   SF_GET_SCREEN_SIZE
        movzx   ecx, ax
        shr     eax, 16
        sub     eax, SCREEN_W + BORDER * 2
        shr     eax, 1
        shl     eax, 16
        add     eax, SCREEN_W + BORDER * 2
        mov     ebx, eax
        sub     ecx, esi
        shr     ecx, 1
        shl     ecx, 16
        add     ecx, esi
        
        mcall   SF_CREATE_WINDOW, , , WIN_TYPE or COL_TABLE, 0, title
        mcall   SF_REDRAW, SSF_END_DRAW
        or      byte [force_redraw], 1

redraw_if_needed:
        cmp     byte [force_redraw], 0
        je      event_loop

        mov     byte [force_redraw], 0
        call    prepare_buffer
        mcall   SF_PUT_IMAGE_EXT, screen_pixels, <SCREEN_W, SCREEN_H>, 0, 32, 0, 0
        jmp     event_loop

quit_app:
        mcall   SF_TERMINATE_PROCESS

restart_game:
        xor     eax, eax
        mov     dword [active_count], eax
        call    init_game_state
        call    shuffle_deck
        or      byte [force_redraw], 1
        ret

; --- GRAPHICS ENGINE ---

prepare_buffer:
        pushad
        mov     edi, screen_pixels
        mov     eax, COL_TABLE
        mov     ecx, SCREEN_W * SCREEN_H
        rep     stosd

        cmp     byte [game_won], 1
        je      .draw_victory

        call    draw_slots
        mov     eax, dword [active_count]
        xor     ecx, ecx
        mov     esi, deck_data
        test    eax, eax
        jz      .draw_all

        .draw_deck:
        xor     edx, edx
        .chk_active:
        cmp     cl, byte [active_indices + edx]
        je      .skip
        inc     edx
        cmp     edx, eax
        jl      .chk_active
        call    draw_card_to_buffer

        .skip:
        inc     ecx
        add     esi, sizeof.CARD
        cmp     ecx, NUM_CARDS
        jl      .draw_deck
        jmp     .base_done

        .draw_all:
        call    draw_card_to_buffer
        inc     ecx
        add     esi, sizeof.CARD
        cmp     ecx, NUM_CARDS
        jl      .draw_all

        .base_done:
        xor     ebx, ebx
        cmp     dword [active_count], 0
        je      .done

        .draw_act:
        movzx   eax, byte [active_indices + ebx]
        lea     esi, [deck_data + eax*8]
        call    draw_card_to_buffer
        inc     ebx
        cmp     ebx, dword [active_count]
        jl      .draw_act
        jmp     .done

        .draw_victory:
        mov     ebx, (SCREEN_W - 144) / 2
        shl     ebx, 16
        or      ebx, (SCREEN_H - 72) / 2
        mov     ecx, COL_WHITE or 0x8F000000   ; A=1, 6x9, C=1, size x8
        mov     edx, won_text
        mcall   SF_DRAW_TEXT, , , , , screen_buffer

        .done:
        popad
        ret

; in: ebx=x, ecx=y, edx=color, edi=w, ebp=h
fill_rect_buf:
        pushad
        mov     eax, edx                ; color
        mov     esi, ebp                ; row count
        mov     ebp, edi                ; width
        imul    ecx, SCREEN_W * 4       ; y -> bytes
        lea     edx, [screen_pixels + ecx + ebx*4]

        .row:
        mov     edi, edx
        mov     ecx, ebp
        rep     stosd
        add     edx, SCREEN_W * 4
        dec     esi
        jnz     .row
        popad
        ret

draw_card_to_buffer:
        pushad
        movzx   ebx, word [esi+CARD.X]
        movzx   ecx, word [esi+CARD.Y]
        mov     edx, COL_BLACK
        mov     edi, CARD_W
        mov     ebp, CARD_H
        call    fill_rect_buf
        inc     ebx
        inc     ecx
        mov     edx, COL_WHITE
        mov     edi, CARD_W-2
        mov     ebp, CARD_H-2
        call    fill_rect_buf
        movzx   eax, byte [esi+CARD.VALUE]
        movzx   edi, byte [esi+CARD.SUIT]
        mov     edx, [suit_colors + edi*4]
        mov     ebp, edx
        dec     eax
        lea     edx, [rank_pairs + eax*2]
        mov     ax, [edx]
        mov     word [rank_buf], ax
        mov     byte [rank_buf+2], 0

        .draw_rank:
        movzx   ebx, word [esi+CARD.X]
        movzx   ecx, word [esi+CARD.Y]
        add     ebx, CARD_W-14-6
        add     ecx, 6
        mov     edx, ebp
        push    esi
        mov     esi, edi
        call    draw_suit_to_buf_x2
        pop     esi

        mov     edx, rank_buf
        sub     ebx, CARD_W-26
        shl     ebx, 16
        movzx   eax, cx
        or      ebx, eax
        mov     ecx, ebp
        or      ecx, TEXT_FONT
        mcall   SF_DRAW_TEXT, , , , , screen_buffer
        popad
        ret

draw_suit_to_buf_x2:
        pushad
        imul    esi, 7
        add     esi, suit_7x7
        mov     ebp, ebx                ; base x
        mov     ebx, edx                ; color
        mov     edi, ecx                ; base y
        mov     eax, 7                  ; rows

        .cy:
        push    eax
        mov     edx, edi
        imul    edx, SCREEN_W * 4
        add     edx, screen_pixels
        lea     edx, [edx + ebp*4]      ; row 1 ptr
        lea     ecx, [edx + SCREEN_W*4] ; row 2 ptr
        mov     eax, 01000000b          ; 7-bit mask

        .cx:
        test    byte [esi], al
        jz      .s

        mov     [edx], ebx
        mov     [edx+4], ebx
        mov     [ecx], ebx
        mov     [ecx+4], ebx

        .s:
        add     edx, 8
        add     ecx, 8
        shr     eax, 1
        jnz     .cx

        .next_row:
        inc     esi
        add     edi, 2
        pop     eax
        dec     eax
        jnz     .cy
        popad
        ret

; in:  eax = card index
; out: eax = pointer to card struct in deck_data
card_ptr_from_eax:
        lea     eax, [deck_data + eax * sizeof.CARD]
        ret

; in:  eax = x (center), ecx = slot group base x
; out: eax = slot index (0..NUM_SLOTS-1), CF=0 on success, CF=1 if outside
slot_index_from_x:
        sub     eax, ecx
        jl      .fail
        cdq
        mov     ecx, SPACING
        div     ecx
        cmp     eax, NUM_SLOTS
        jae     .fail
        cmp     edx, CARD_W
        jae     .fail
        clc
        ret

        .fail:
        stc
        ret

; --- MOUSE AND GAME LOGIC ---

handle_mouse:
        pushad
        mcall   SF_MOUSE_GET, SSF_WINDOW_POSITION
        mov     ebp, eax
        sar     ebp, 16
        movsx   edi, ax
        mcall   SF_MOUSE_GET, SSF_BUTTON
        test    eax, 1
        jz      handle_mouse_release
        cmp     dword [active_count], 0
        jne     handle_mouse_drag
        jmp     handle_mouse_press

handle_mouse_release:
        cmp     dword [active_count], 0
        je      handle_mouse_exit
        movzx   eax, byte [active_indices]
        call    card_ptr_from_eax
        mov     dword [active_card_ptr], eax
        movzx   edx, word [eax+CARD.Y]
        cmp     edx, TABLEAU_Y-10
        jle     .top_drop
        movzx   esi, word [eax+CARD.X]
        add     esi, CARD_W/2
        sub     esi, PADDING
        jl      .return_all
        mov     eax, esi
        xor     edx, edx
        mov     ecx, SPACING
        div     ecx
        cmp     eax, NUM_COLS
        jae     .return_all
        mov     ebx, eax
        call    find_last_in_col        ; eax=target, edx=max_y
        cmp     eax, -1
        jne     .has_target
        mov     ebp, TABLEAU_Y - OFFSET ; no target: first slot is TABLEAU_Y
        jmp     .move_ok

        .has_target:
        mov     ebp, edx        ; base y = max_y (first iter adds OFFSET)
        mov     esi, dword [active_card_ptr]
        call    card_ptr_from_eax
        movzx   ecx, byte [esi+CARD.VALUE]
        movzx   edx, byte [eax+CARD.VALUE]
        dec     edx
        cmp     ecx, edx
        jne     .return_all
        movzx   ecx, byte [esi+CARD.SUIT]
        and     ecx, 1
        movzx   edx, byte [eax+CARD.SUIT]
        and     edx, 1
        cmp     ecx, edx
        je      .return_all

        .move_ok:
        ; Limit tableau column height to 13 cards to keep Y positions valid.
        mov     eax, ebp
        sub     eax, TABLEAU_Y - OFFSET
        xor     edx, edx
        mov     ecx, OFFSET
        div     ecx                    ; eax = cards already in target column
        add     eax, dword [active_count]
        cmp     eax, NUM_VALUES
        ja      .return_all

        mov     edx, ebx
        imul    edx, SPACING
        add     edx, PADDING    ; target x for whole moved stack
        xor     ecx, ecx
        mov     edi, dword [active_count]

        .move_loop:
        movzx   eax, byte [active_indices + ecx]
        call    card_ptr_from_eax
        mov     byte [eax+CARD.STATE], 0
        mov     [eax+CARD.SLOT_ID], bl
        mov     [eax+CARD.X], dx
        add     ebp, OFFSET
        mov     [eax+CARD.Y], bp
        inc     ecx
        cmp     ecx, edi
        jl      .move_loop
        call    restack_deck
        jmp     .reset_all

        .top_drop:
        cmp     dword [active_count], 1
        jne     .return_all
        mov     eax, dword [active_card_ptr]
        movzx   esi, word [eax+CARD.X]
        add     esi, CARD_W/2
        movzx   edi, word [eax+CARD.Y]
        add     edi, CARD_H/2
        sub     edi, PADDING
        cmp     edi, CARD_H
        jae     .return_all
        mov     eax, esi
        mov     ecx, FC_X
        call    slot_index_from_x
        jc      .try_found_slot
        jmp     .drop_fc

        .try_found_slot:
        mov     eax, esi
        mov     ecx, FOUNDATION_X
        call    slot_index_from_x
        jc      .return_all
        mov     ebx, eax

        .dfn:
        mov     edx, dword [active_card_ptr]
        movzx   ecx, byte [edx+CARD.SUIT]
        cmp     cl, bl
        jne     .return_all
        movzx   edi, byte [edx+CARD.VALUE]
        xor     eax, eax
        mov     esi, deck_data
        mov     edx, NUM_CARDS

        .gftl:
        cmp     byte [esi+CARD.STATE], 2
        jne     .gftn
        cmp     byte [esi+CARD.SLOT_ID], bl
        jne     .gftn
        movzx   ecx, byte [esi+CARD.VALUE]
        cmp     ecx, eax
        jbe     .gftn
        mov     eax, ecx

        .gftn:
        add     esi, sizeof.CARD
        dec     edx
        jnz     .gftl
        mov     ecx, edi
        test    eax, eax
        jnz     .n_e_f
        cmp     ecx, 1
        jne     .return_all
        jmp     .dfn_ok

        .n_e_f:
        inc     eax
        cmp     ecx, eax
        jne     .return_all

        .dfn_ok:
        mov     dl, 2
        mov     ecx, FOUNDATION_X
        call    place_active_to_slot
        call    restack_deck
        call    check_victory
        jmp     .reset_all

        .drop_fc:
        mov     ebx, eax
        mov     esi, deck_data
        mov     ecx, NUM_CARDS

        .fc_l:
        cmp     byte [esi+CARD.STATE], 1
        jne     .fc_n
        cmp     byte [esi+CARD.SLOT_ID], bl
        je      .return_all

        .fc_n:
        add     esi, sizeof.CARD
        loop    .fc_l
        mov     dl, 1
        mov     ecx, FC_X
        call    place_active_to_slot
        call    restack_deck
        jmp     .reset_all

        .return_all:
        xor     ecx, ecx
        mov     ebx, dword [active_count]
        .ret_l:
        movzx   eax, byte [active_indices + ecx]
        call    card_ptr_from_eax
        mov     edx, dword [drag_old_x]
        mov     [eax+CARD.X], dx
        mov     esi, dword [drag_old_y]
        movzx   edx, cx
        imul    edx, OFFSET
        add     edx, esi
        mov     [eax+CARD.Y], dx
        inc     ecx
        cmp     ecx, ebx
        jl      .ret_l

        .reset_all:
        xor     eax, eax
        mov     dword [active_count], eax
        or      byte [force_redraw], 1
        jmp     handle_mouse_exit

handle_mouse_press:
        mov     ecx, NUM_CARDS-1
        mov     edx, deck_data + (NUM_CARDS-1) * sizeof.CARD

        .find:
        movzx   esi, word [edx+CARD.X]
        movzx   eax, word [edx+CARD.Y]
        mov     ebx, ebp
        sub     ebx, esi
        cmp     ebx, CARD_W
        jae     .next
        mov     dword [drag_off_x], ebx
        mov     esi, edi
        sub     esi, eax
        cmp     esi, CARD_H
        jae     .next
        mov     dword [drag_off_y], esi
        mov     byte [active_indices], cl
        mov     dword [active_count], 1
        cmp     byte [edx+CARD.STATE], 0
        jne     .grab
        mov     esi, ecx
        push    edx                     ; preserve scan pointer for .next on fail

        .build:
        call    find_card_below
        cmp     eax, -1
        je      .limit
        push    eax esi
        mov     eax, edx        ; below card ptr (from find_card_below)
        lea     esi, [deck_data + esi*8]
        movzx   ebx, byte [eax+CARD.VALUE]
        movzx   edx, byte [esi+CARD.VALUE]
        dec     edx
        cmp     ebx, edx
        jne     .inv
        movzx   ebx, byte [eax+CARD.SUIT]
        and     ebx, 1
        movzx   edx, byte [esi+CARD.SUIT]
        and     edx, 1
        cmp     ebx, edx
        je      .inv
        pop     esi eax
        mov     ebx, dword [active_count]
        cmp     ebx, NUM_CARDS
        jae     .limit
        mov     byte [active_indices+ebx], al
        inc     dword [active_count]
        mov     esi, eax
        jmp     .build

        .inv:
        pop     esi eax
        jmp     .cancel_build

        .limit:
        push    ecx
        call    count_move_limit
        pop     ecx
        cmp     dword [active_count], eax
        jbe     .grab_build
        .cancel_build:
        xor     eax, eax
        mov     dword [active_count], eax
        pop     edx
        jmp     .next

        .grab_build:
        pop     edx

        .grab:
        xor     ecx, ecx
        mov     ebx, dword [active_count]
        .save:
        movzx   eax, byte [active_indices+ecx]
        call    card_ptr_from_eax
        test    ecx, ecx
        jne     .save_next
        movzx   edx, word [eax+CARD.X]
        mov     dword [drag_old_x], edx
        movzx   edx, word [eax+CARD.Y]
        mov     dword [drag_old_y], edx
        .save_next:
        inc     ecx
        cmp     ecx, ebx
        jl      .save
        jmp     handle_mouse_exit

        .next:
        sub     edx, sizeof.CARD
        dec     ecx
        jns     .find
        jmp     handle_mouse_exit

handle_mouse_drag:
        mov     ebx, ebp
        mov     edx, dword [drag_off_x]
        sub     ebx, edx
        test    ebx, ebx
        jge     .x_min_ok
        xor     ebx, ebx

        .x_min_ok:
        cmp     ebx, SCREEN_W - CARD_W
        jle     .x_max_ok
        mov     ebx, SCREEN_W - CARD_W

        .x_max_ok:
        mov     ebp, ebx        ; ebp = final_x
        mov     ebx, edi
        mov     edx, dword [drag_off_y]
        sub     ebx, edx
        test    ebx, ebx
        jge     .y_min_ok
        xor     ebx, ebx

        .y_min_ok:
        mov     edx, dword [active_count]
        dec     edx
        imul    edx, OFFSET
        mov     ecx, SCREEN_H - CARD_H
        sub     ecx, edx
        cmp     ebx, ecx
        jle     .y_max_ok
        mov     ebx, ecx

        .y_max_ok:
        mov     edx, ebx        ; current y
        xor     ecx, ecx        ; ebp = final_x
        mov     edi, dword [active_count]

        .m_upd:
        movzx   eax, byte [active_indices+ecx]
        call    card_ptr_from_eax
        mov     [eax+CARD.X], bp
        mov     [eax+CARD.Y], dx
        add     edx, OFFSET
        inc     ecx
        cmp     ecx, edi
        jl      .m_upd
        or      byte [force_redraw], 1

handle_mouse_exit:
        popad
        ret

place_active_to_slot:
        mov     eax, dword [active_card_ptr]
        mov     [eax+CARD.STATE], dl
        mov     [eax+CARD.SLOT_ID], bl
        imul    ebx, SPACING
        add     ebx, ecx
        mov     [eax+CARD.X], bx
        mov     word [eax+CARD.Y], PADDING
        ret

restack_deck:
        mov     ecx, dword [active_count]
        test    ecx, ecx
        jz      .done
        xor     ebx, ebx

        .loop_act:
        movzx   eax, byte [active_indices + ebx]
        call    move_to_last
        inc     ebx
        dec     ecx
        jnz     .loop_act
        .done:
        ret

move_to_last:
        pushad
        mov     edx, eax
        lea     esi, [deck_data + eax*8]
        mov     eax, [esi]
        mov     ebx, [esi+4]
        mov     edi, esi
        lea     esi, [edi + sizeof.CARD]
        mov     ecx, NUM_CARDS-1
        sub     ecx, edx
        jz      .is_last
        add     ecx, ecx        ; cards * 2 dwords
        rep     movsd

        .is_last:
        mov     edi, deck_data + (NUM_CARDS-1) * sizeof.CARD
        mov     [edi], eax
        mov     [edi+4], ebx
        cmp     dword [active_count], 1
        jle     .done
        xor     ebx, ebx

        .upd:
        movzx   eax, byte [active_indices + ebx]
        cmp     eax, edx
        jb      .skip_u         ; < moved idx: unchanged
        je      .was_moved      ; == moved idx: now at NUM_CARDS-1
        dec     eax             ; > moved idx: shifted down by 1
        jmp     .store

        .was_moved:
        mov     eax, NUM_CARDS-1

        .store:
        mov     byte [active_indices + ebx], al

        .skip_u:
        inc     ebx
        cmp     ebx, dword [active_count]
        jl      .upd
        .done:
        popad
        ret

find_card_below:
        push    esi
        shl     esi, 3          ; sizeof.CARD = 8, index * 8
        add     esi, deck_data
        mov     bl, [esi+CARD.SLOT_ID]
        mov     dx, [esi+CARD.Y]
        add     dx, OFFSET
        xor     ecx, ecx
        mov     eax, deck_data

        .fbl:
        cmp     byte [eax+CARD.STATE], 0
        jne     .fbn
        cmp     byte [eax+CARD.SLOT_ID], bl
        jne     .fbn
        cmp     word [eax+CARD.Y], dx
        je      .fbf

        .fbn:
        add     eax, sizeof.CARD
        inc     ecx
        cmp     ecx, NUM_CARDS
        jl      .fbl
        pop     esi
        mov     eax, -1
        ret

        .fbf:
        mov     edx, eax        ; pointer to found card
        mov     eax, ecx
        pop     esi
        ret

; out: eax = (free_slots + 1) << empty_cols
count_move_limit:
        push    ebp
        mov     eax, NUM_SLOTS  ; free slots
        mov     ecx, NUM_COLS   ; empty tableau columns
        xor     edx, edx        ; occupied freecells bitmask
        xor     ebx, ebx        ; occupied tableau columns bitmask
        mov     esi, deck_data
        mov     edi, NUM_CARDS

        .scan:
        movzx   ebp, byte [esi+CARD.STATE]
        cmp     ebp, 1
        je      .fc
        test    ebp, ebp
        jne     .next
        movzx   ebp, byte [esi+CARD.SLOT_ID]
        bts     ebx, ebp
        adc     ecx, -1
        jmp     .next

        .fc:
        movzx   ebp, byte [esi+CARD.SLOT_ID]
        bts     edx, ebp
        adc     eax, -1

        .next:
        add     esi, sizeof.CARD
        dec     edi
        jnz     .scan

        ; eax = (free_slots + 1) << empty_cols
        inc     eax
        shl     eax, cl
        pop     ebp
        ret

; in:  ebx = col index
; out: eax = target card idx (-1 if none), edx = max_y
find_last_in_col:
        mov     edi, -1         ; target
        xor     ebp, ebp        ; max_y
        xor     ecx, ecx
        mov     eax, deck_data

        .fli:
        cmp     byte [eax+CARD.STATE], 0
        jne     .flin
        cmp     byte [eax+CARD.SLOT_ID], bl
        jne     .flin
        movzx   edx, word [eax+CARD.Y]
        cmp     edx, ebp
        jl      .flin
        mov     ebp, edx
        mov     edi, ecx

        .flin:
        add     eax, sizeof.CARD
        inc     ecx
        cmp     ecx, NUM_CARDS
        jl      .fli
        mov     eax, edi
        mov     edx, ebp
        ret

check_victory:
        mov     ecx, NUM_CARDS
        mov     esi, deck_data

        .loop:
        cmp     byte [esi+CARD.STATE], 2
        jne     .done
        add     esi, sizeof.CARD
        loop    .loop
        mov     byte [game_won], 1
        or      byte [force_redraw], 1

        .done:
        ret

draw_slots:
        pushad
        mov     edi, CARD_W
        mov     ebp, CARD_H
        xor     esi, esi
        mov     ebx, FC_X

        .slot:
        mov     edx, COL_SLOT
        mov     ecx, PADDING
        call    fill_rect_buf
        cmp     esi, NUM_SLOTS
        jl      .next
        push    esi
        sub     esi, NUM_SLOTS
        mov     edx, COL_BLACK
        test    esi, 1
        jz      .draw_suit
        mov     edx, COL_RED

        .draw_suit:
        push    ebx ecx
        add     ebx, 29
        add     ecx, 41
        call    draw_suit_to_buf_x2
        pop     ecx ebx
        pop     esi

        .next:
        add     ebx, SPACING
        inc     esi
        cmp     esi, NUM_SLOTS * 2
        jl      .slot
        popad
        ret

shuffle_deck:
        pushad
        call    seed_rng
        mov     ecx, NUM_CARDS-1

        .sh:
        call    rng_next
        cdq
        mov     ebx, ecx
        inc     ebx
        div     ebx
        lea     esi, [deck_data + ecx*8]
        lea     edi, [deck_data + edx*8]
        mov     ax, [esi]
        mov     bx, [edi]
        mov     [esi], bx
        mov     [edi], ax
        loop    .sh
        call    init_card_positions
        popad
        ret

seed_rng:
        mcall   SF_GET_SYS_TIME
        mov     edx, eax
        mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
        xor     eax, edx
        rol     eax, 7
        imul    eax, eax, 1103515245
        add     eax, 12345
        mov     dword [rand_seed], eax
        ret

rng_next:
        mov     eax, dword [rand_seed]
        imul    eax, 1103515245
        add     eax, 12345
        mov     dword [rand_seed], eax
        shr     eax, 16
        and     eax, 0x7FFF
        ret

init_game_state:
        mov     edi, deck_data
        xor     eax, eax
        mov     byte [game_won], 0

        .s:
        xor     ebx, ebx

        .v:
        inc     ebx
        mov     [edi+CARD.VALUE], bl
        mov     [edi+CARD.SUIT], al
        mov     byte [edi+CARD.STATE], 0
        mov     byte [edi+CARD.SLOT_ID], 0
        add     edi, sizeof.CARD
        cmp     ebx, NUM_VALUES
        jne     .v
        inc     eax
        cmp     eax, NUM_SUITS
        jne     .s
        ret

init_card_positions:
        mov     edi, deck_data
        xor     edx, edx

        .cols:
        mov     ebx, edx
        imul    ebx, SPACING
        add     ebx, PADDING
        mov     ecx, TABLEAU_Y
        mov     eax, 6
        cmp     edx, NUM_SLOTS
        jge     .cards
        inc     eax

        .cards:
        mov     [edi+CARD.X], bx
        mov     [edi+CARD.Y], cx
        mov     [edi+CARD.SLOT_ID], dl
        add     edi, sizeof.CARD
        add     ecx, OFFSET
        dec     eax
        jnz     .cards
        inc     edx
        cmp     edx, NUM_COLS
        jl      .cols
        ret

; --- DATA SECTION ---

if lang eq ru_RU
        title   cp866 "FreeCell 1.0 [R - Перезапуск]", 0
else if lang eq es_ES
        title   cp850 "FreeCell 1.0 [R - Reiniciar]", 0
else
        title   db    "FreeCell 1.0 [R - Restart]", 0
end if

won_text        db "WON", 0

rank_pairs      db "A ","2 ","3 ","4 ","5 ","6 ","7 ","8 ","9 ","10","J ","Q ","K "
suit_colors     dd COL_BLACK, COL_RED, COL_BLACK, COL_RED

suit_7x7:
                db 0001000b, \ ; Spades
                   0011100b, \
                   0111110b, \
                   1111111b, \
                   0110110b, \
                   0001000b, \
                   0011100b

                db 0001000b, \ ; Diamonds
                   0011100b, \
                   0111110b, \
                   1111111b, \
                   0111110b, \
                   0011100b, \
                   0001000b

                db 0011100b, \ ; Clubs
                   0011100b, \
                   1101011b, \
                   1111111b, \
                   0110110b, \
                   0001000b, \
                   0011100b

                db 0110110b, \ ; Hearts
                   1111111b, \
                   1111111b, \
                   1111111b, \
                   0111110b, \
                   0011100b, \
                   0001000b

i_end:

align 4
deck_data       rb NUM_CARDS * sizeof.CARD
game_won        rb 1
active_indices  rb NUM_CARDS
active_count    rb 4
active_card_ptr rb 4
force_redraw    rb 1
rand_seed       rb 4
drag_off_x      rb 4
drag_off_y      rb 4
drag_old_x      rb 4
drag_old_y      rb 4
rank_buf        rb 3

align   16
screen_buffer   dd SCREEN_W, SCREEN_H
screen_pixels   rb SCREEN_BYTES

align   16
rb      4096            ; stack
stacktop:
mem_end:
