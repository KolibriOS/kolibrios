; SPDX-License-Identifier: GPL-2.0-only
;
; Charsets - Charsets Viewer
; Copyright (C) 2025 KolibriOS team
;
; Contributor Burer  - Main code
; Contributor Doczom - UTF-8 to UTF-16 convertor

; ================================================================

use32
org 0

; ================================================================

db      'MENUET01'
dd      1
dd      START
dd      I_END
dd      MEM
dd      STACKTOP
dd      0
dd      0

; ================================================================

include '../../macros.inc'
include "../../KOSfuncs.inc"
include "../../encoding.inc"

; ================================================================

START:                                  ; start of execution
        call    draw_window             ; draw the window

event_wait:

        mcall   SF_WAIT_EVENT

        cmp     al, 1
        je      red
        cmp     al, 2
        je      key
        cmp     al, 3
        je      button

        jmp     event_wait

red:
        call    draw_window

        jmp     event_wait

key:
        mcall   SF_GET_KEY

        ; no input
        cmp     al, 1
        je      event_wait

        ; if not reading - skip
        cmp     [reading], 0x01
        je      .read_input

        ; moving scancode to ah
        shr     eax, 16

        .key_pagedown:
                cmp     al, 81
                jne     .key_pageup

                call    logic_charpage_dec

                jmp     .key_end

        .key_pageup:
                cmp     al, 73
                jne     .key_pagezero

                call    logic_charpage_inc

                jmp     .key_end

        .key_pagezero:
                cmp     al, 82
                jne     .key_arr_left

                call    logic_charpage_zero

                jmp     .key_end

        .key_arr_left:
                cmp     al, 75
                jne     .key_arr_right

                call    logic_char_left

                jmp     .key_end

        .key_arr_right:
                cmp     al, 77
                jne     .key_arr_down

                call    logic_char_right

                jmp     .key_end

        .key_arr_down:
                cmp     al, 72
                jne     .key_arr_up

                call    logic_char_down

                jmp     .key_end

        .key_arr_up:
                cmp     al, 80
                jne     .key_scale_down

                call    logic_char_up

                jmp     .key_end

        .key_scale_down:
                cmp     al, 74
                jne     .key_scale_up

                call    logic_scale_down

                jmp     .key_end

        .key_scale_up:
                cmp     al, 78
                jne     .key_home

                call    logic_scale_up

                jmp     .key_end

        .key_home:
                cmp     al, 71
                jne     .key_end

                call    logic_charpage_zero

                jmp     .key_end

        ; reading inputted char
        .read_input:
                mov     [char], ah
                mov     [page], 0x00
                push    eax
                shr     ax, 8
                xor     ah, ah
                mov     [char_ascii], ax
                pop     eax
                shr     eax, 16
                mov     [char_scan], ax
                mov     [letter], 0x0000

        .key_end:
                call    logic_update
                jmp     event_wait


button:
        mcall   SF_GET_BUTTON

        .close:                         ; window close button
                cmp     ah, 1
                jne     .button_a

                mcall   SF_TERMINATE_PROCESS

        .button_a:                      ; 0A - select character
                cmp     ah, 0x0A
                jne     .button_b

                mcall   SF_MOUSE_GET, SSF_WINDOW_POSITION
                sub     eax, 0x00220022
                mov     bl, 24
                div     bl
                mov     cl, al
                shl     cl, 4
                shr     eax, 16
                div     bl
                add     cl, al
                mov     [char], cl
                mov     [char_scan], 0x0000

                jmp     .button_end

        .button_b:                      ; 0B - charset CP866 6x9
                cmp     ah, 0x0B
                jne     .button_c

                call    draw_charpage_delete

                mov     [page], 0x00

                mov     [charset], 0x80
                mov     [lb_curr], lb_cp6x9

                jmp    .button_end

        .button_c:                      ; 0C - charset CP866 8x16
                cmp     ah, 0x0C
                jne     .button_d

                call    draw_charpage_delete

                mov     [page], 0x00

                mov     [charset], 0x90
                mov     [lb_curr], lb_cp8x16

                jmp    .button_end

        .button_d:                      ; 0D - charset UTF-16 8x16
                cmp     ah, 0x0D
                jne     .button_e

                call    draw_charpage_buttons

                mov     al, [page_utf]
                mov     [page], al

                mov     [charset], 0xA0
                mov     [lb_curr], lb_utf16

                jmp    .button_end

        .button_e:                      ; 0E - charset UTF-8 8x16
                cmp     ah, 0x0E
                jne     .button_f

                call    draw_charpage_buttons

                mov     al, [page_utf]
                mov     [page], al

                mov     [charset], 0xB0
                mov     [lb_curr], lb_utf8

                call    .button_end

        .button_f:                      ; 0F - charpage reset
                cmp     ah, 0x0F
                jne     .button_10

                call    logic_charpage_zero

                jmp     event_wait

        .button_10:                     ; 10 - charpage decrement
                cmp     ah, 0x10
                jne     .button_11

                call    logic_charpage_dec

                jmp     event_wait

        .button_11:                     ; 11 - charpage increment
                cmp     ah, 0x11
                jne     .button_12

                call    logic_charpage_inc

                jmp     event_wait

        .button_12:                     ; 12 - read/stop keyboard input
                cmp     ah, 0x12
                jne     .button_13

                mov     al, 0x01
                sub     al, [reading]
                mov     [reading], al

                call    draw_toggle

                jmp     event_wait

        .button_13:                     ; 13 - character scale down
                cmp     ah, 0x13
                jne     .button_14

                call    logic_scale_down

                jmp     event_wait

        .button_14:                     ; 14 - character scale up
                cmp     ah, 0x14
                jne     .button_15

                call    logic_scale_up

                jmp     event_wait

        .button_15:                     ; 15 - copy ascii dec
                cmp     ah, 0x15
                jne     .button_16

                mov     dx, [char_ascii]
                call    logic_copy_dec

                jmp     event_wait

        .button_16:                     ; 18 - copy scan dec
                cmp     ah, 0x16
                jne     .button_17

                mov     dx, [char_scan]
                call    logic_copy_dec

                jmp     event_wait

        .button_17:                     ; 17 - copy ascii hex
                cmp     ah, 0x17
                jne     .button_18

                mov     dx, [char_ascii]
                call    logic_copy_hex

                jmp     event_wait

        .button_18:                     ; 18 - copy scan hex
                cmp     ah, 0x18
                jne     event_wait

                mov     dx, [char_scan]
                call    logic_copy_hex

                jmp     event_wait

        .button_end:
                call    logic_update
                jmp     event_wait

; ===================================================================

; set charpage number to zero
logic_charpage_zero:

        cmp     [charset], 0x90
        jle     event_wait

        mov     [page], 0x00
        mov     [page_utf], 0x00

        call    logic_update

        ret


; decrement charpage number
logic_charpage_dec:

        cmp     [charset], 0x90
        jle     event_wait

        dec     byte [page]
        dec     byte [page_utf]

        call    logic_update

        ret


; increment charpage number
logic_charpage_inc:

        cmp     [charset], 0x90
        jle     event_wait

        inc     byte [page]
        inc     byte [page_utf]

        call    logic_update

        ret


; moving in charpage
; dec/inc lower nibble mod 16
; input: al = +1 or -1
logic_char_modify_lower_nibble:

        mov     bl, [char]
        mov     cl, bl
        and     cl, 0x0F
        add     cl, al
        and     cl, 0x0F
        and     bl, 0xF0
        or      bl, cl
        mov     [char], bl

        ret


; dec/inc upper nibble mod 16
; input: al = +1 or -1
logic_char_modify_upper_nibble:

        mov     bl, [char]
        mov     cl, bl
        and     cl, 0xF0
        shr     cl, 4
        add     cl, al
        and     cl, 0x0F
        shl     cl, 4
        and     bl, 0x0F
        or      bl, cl
        mov     [char], bl

        ret


; move left in charpage
logic_char_left:

        mov     al, -1
        call    logic_char_modify_lower_nibble

        ret


; move right in charpage
logic_char_right:

        mov     al, 1
        call    logic_char_modify_lower_nibble

        ret


; move down in charpage
logic_char_down:

        mov     al, -1
        call    logic_char_modify_upper_nibble

        ret


; move up in charpage
logic_char_up:

        mov     al, 1
        call    logic_char_modify_upper_nibble

        ret


; update all dependant values
logic_update:

        ; update [letter] value (first char on current page)
        mov     ax, [letter]
        mov     ah, [page]
        mov     [letter], ax

        ; update [char_ascii] value (selected single char)
        mov     ah, [page]
        mov     al, [char]
        mov     [char_ascii], ax

        call    draw_update_table
        call    draw_update_single

        ret


; edx = num, esi -> buffer of size 4
logic_utf16to8:

        push    eax ecx edx
        xor     ecx, ecx
        mov     dword [esi], 0
        or      ecx, 3
        mov     eax, 0x80808000 + 11110000b

        cmp     edx, 0x00010000
        jae     @f
        mov     eax, 0x00808000 + 11100000b
        dec     ecx

        cmp     edx, 0x00000800
        jae     @f
        mov     eax, 0x00008000 + 11000000b
        dec     ecx

        cmp     edx, 0x00000080
        jae     @f
        mov     eax, edx
        dec     ecx

        @@:
                mov     [esi], eax

        @@:
                mov     eax, edx
                and     eax, 0x3F
                shr     edx, 6
                or      byte[esi + ecx], al
                dec     ecx
                jns     @b

        pop     edx ecx eax

        ret


; scale down single character display
logic_scale_down:

        cmp     [scale], 0x00
        jle     event_wait

        mov     bl, [scale]
        dec     bl
        mov     [scale], bl

        call    draw_update_single

        ret


; scale up single character display
logic_scale_up:

        cmp     [scale], 0x07
        jge     event_wait

        mov     bl, [scale]
        inc     bl
        mov     [scale], bl

        call    draw_update_single

        ret


; char codes copying functions
; edx - code
logic_copy_dec:

        mov     ax, dx
        mov     bx, 10
        mov     ecx, 5

        .copy_d_loop:

                xor     dx, dx
                div     bx
                add     dl, 0x30
                mov     [char_code_dec + 11 + ecx], byte dl

                loop    .copy_d_loop

        mcall   SF_CLIPBOARD, SSF_WRITE_CB, char_code_dec.end - char_code_dec, char_code_dec

        ret


; edx - code
logic_copy_hex:

        mov     ecx, 4

        .copy_ah_loop:                   ; iterate over all hex-digits of color
                mov     al, dl
                and     al, 0x0F
                add     al, 0x30        ; 0x30 = ASCII code of "0"

                cmp     al, 0x39
                jbe     .cont           ; if digit

                add     al, 0x07        ; if letter

                .cont:
                        mov     [char_code_hex + 11 + ecx], byte al
                        shr     edx, 4
                        loop    .copy_ah_loop

        mcall   SF_CLIPBOARD, SSF_WRITE_CB, char_code_hex.end - char_code_hex, char_code_hex

        ret

; ===================================================================

draw_window:

        mcall   SF_REDRAW, SSF_BEGIN_DRAW

        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, win_cols, sizeof.system_colors
        mcall                    , SSF_GET_SKIN_HEIGHT,

        mov     ecx, eax
        add     ecx, WN_RECT.Y shl 16 + WN_RECT.H

        mov     edx, [win_cols.work]
        add     edx, 0x34000000
        mcall   SF_CREATE_WINDOW, <WN_RECT.X, WN_RECT.W>, , , , title

        call    draw_base
        call    draw_update_table
        call    draw_update_single
        call    draw_toggle

        mcall   SF_REDRAW, SSF_END_DRAW

        ret

; ===================================================================

; unchangeble base - table, headers and buttons
draw_base:

        ; both tables background
        mcall   SF_DRAW_RECT, <TB_PAGE.X, TB_PAGE.W>, <TB_PAGE.Y, TB_PAGE.H>, [win_cols.work_text]
        mcall               , <TB_SING.X, TB_SING.W>,                       ,
        mcall               , <TB_PAGE.X + 1, TB_PAGE.W - 2>, <TB_PAGE.Y + 1, TB_PAGE.H - 2>, [win_cols.work_light]
        mcall               , <TB_SING.X + 1, TB_SING.W - 2>,

        ; buttons
        ; button on charpage table to pick single character
        mcall   SF_DEFINE_BUTTON,   <BT_TABL.X, BT_TABL.W>,   <BT_TABL.Y, BT_TABL.H>, 0x6000000A,

        ; charsets change buttons
        mcall                   , <BT_CS_01.X, BT_CS_01.W>, <BT_CS_01.Y, BT_CS_01.H>, 0x0000000B, [win_cols.work_button]
        mcall                   , <BT_CS_02.X, BT_CS_02.W>,                         , 0x0000000C,
        mcall                   , <BT_CS_03.X, BT_CS_03.W>,                         , 0x0000000D,
        mcall                   , <BT_CS_04.X, BT_CS_04.W>,                         , 0x0000000E,

        ; character scale buttons
        mcall                   , <BT_SC_DN.X, BT_SC_DN.W>, <BT_SC_DN.Y, BT_SC_DN.H>, 0x00000013,
        mcall                   , <BT_SC_UP.X, BT_SC_UP.W>,                         , 0x00000014,

        ; tables
        ; 16x16 characters table
        mcall   SF_DRAW_RECT, <LN_PG_01.X, LN_PG_01.W>, <LN_PG_01.Y, LN_PG_01.H>, [win_cols.work_text]
        mcall               , <LN_PG_02.X, LN_PG_02.W>, <LN_PG_02.Y, LN_PG_02.H>,

        ; single character table
        mcall   SF_DRAW_RECT, <LN_SN_01.X, LN_SN_01.W>,                         ,
        mcall               , <LN_SN_02.X, LN_SN_02.W>, <LN_SN_02.Y, LN_SN_02.H>,
        mcall               ,                         , <LN_SN_03.Y, LN_SN_03.H>,
        mcall               ,                         , <LN_SN_04.Y, LN_SN_04.H>,

        ; headers
        ; horizontal page table headers
        mov     eax, SF_DRAW_TEXT
        mov     ebx, HD_TABL.X shl 16 + HD_TABL.Y
        mov     ecx, [win_cols.work_text]
        add     ecx, 0x90000000
        xor     edx, edx
        mov     esi, 16

        .loop_hx:
                mov     edx, header
                mcall

                mov     dx, [header]
                add     dx, 0x0100

                cmp     dx, 0x3AFF
                jne     .hx_af
                add     dx, 0x0700

                .hx_af:
                mov     [header], dx
                add     ebx, 24 shl 16
                dec     esi
                jnz     .loop_hx

        ; vertical headers
        mov     ebx, 13 shl 16 + 39
        mov     esi, 16
        mov     [header], 0xFF30

        .loop_hy:
                mov     edx, header
                mcall

                mov     dx, [header]
                add     dx, 0x0001

                cmp     dx, 0xFF3A
                jne     .hy_af
                add     dx, 0x0007

                .hy_af:
                mov     [header], dx
                add     ebx, 24
                dec     esi
                jnz     .loop_hy

        ; reset headers
        mov     [header], 0x30FF

        ; single character table headers
        mcall     , <LB_SN_SCL.X, LB_SN_SCL.Y>, , lb_size,
        mcall     , <LB_SN_DEC.X, LB_SN_DEC.Y>, ,  lb_dec,
        mcall     , <LB_SN_HEX.X, LB_SN_HEX.Y>, ,  lb_hex,
        mcall     , <LB_SN_ASC.X, LB_SN_ASC.Y>, , lb_asci,
        mcall     , <LB_SN_SCN.X, LB_SN_SCN.Y>, , lb_scan,

        ; buttons subscriptions
        ; charsets change buttons subscriptions
        mov      ecx, [win_cols.work_button_text]
        add      ecx, 0xB0000000
        mcall     , <LB_CS_01.X, LB_CS_01.Y>, , lb_cp6x9
        mcall     , <LB_CS_02.X, LB_CS_02.Y>, , lb_cp8x16
        mcall     , <LB_CS_03.X, LB_CS_03.Y>, , lb_utf16
        mcall     , <LB_CS_04.X, LB_CS_04.Y>, , lb_utf8

        ; page swap buttons subscriptions
        mcall     , <LB_CP_ZR.X, LB_CP_ZR.Y>, , bt_res
        mcall     , <LB_CP_DN.X, LB_CP_DN.Y>, , bt_dec
        mcall     , <LB_CP_UP.X, LB_CP_UP.Y>, , bt_inc

        ; character scale buttons subscriptions
        mcall     , <LB_SC_DN.X, LB_SC_DN.Y>, , bt_smaller
        mcall     , <LB_SC_UP.X, LB_SC_UP.Y>, , bt_bigger

        cmp     [charset], 0x90
        jle     .cp_bt_cp

        .cp_bt_utf:
                call    draw_charpage_buttons
                jmp     .cp_bt_end

        .cp_bt_cp:
                call    draw_charpage_delete

        .cp_bt_end:
                ret


; changable data: current charset, charpage, chars, selection
draw_update_table:

        ; current charset and charpage
        mov     ecx, [win_cols.work_text]
        add     ecx, 0xD0000000
        mcall   SF_DRAW_TEXT, <TB_PAGE.X, TB_PAGE.Y + TB_PAGE.H + 13>, , [lb_curr], , [win_cols.work]

        mov     esi, [win_cols.work_text]
        add     esi, 0x50000000

        mcall   SF_DRAW_NUMBER, 0x00020101, page, <TB_PAGE.X + 5, TB_PAGE.Y + 6>, , [win_cols.work_light]

        ; 16x16 table of letters
        ; background for letters
        mcall   SF_DRAW_RECT, <TB_PAGE.X + 26, TB_PAGE.W - 27>, <TB_PAGE.Y + 26, TB_PAGE.H - 27>, [win_cols.work_light]

        ;different coordinates for 6x9 charset
        mov     bl, [charset]
        cmp     bl, 0x80
        jne     .char_big

        .char_sm:
                mov     ebx, 44 shl 16 + 42
                jmp     .char_draw

        .char_big:
                mov     ebx, 42 shl 16 + 39

        .char_draw:
                mov     eax, SF_DRAW_TEXT
                mov     cl,  [charset]
                shl     ecx, 24
                add     ecx, [win_cols.work_text]
                mov     esi, 16

        ; letters draw loop
        .loop_ly:
                mov     edi, 16

                .loop_lx:
                        mov     edx, letter

                        cmp     [charset], 0xB0
                        jne     .skip_conversion

                        ;utf 8 to 16
                        xor     edx, edx
                        mov     dx, [letter]
                        push    esi
                        mov     esi, letutf
                        call    logic_utf16to8
                        pop     esi
                        mov     edx, letutf

                        .skip_conversion:

                        mcall

                        mov     dx, [letter]
                        add     dx, 0x01
                        mov     [letter], dx

                        add     ebx, 24 shl 16

                        dec     edi
                        jnz     .loop_lx

                ; start new row of letters
                sub     ebx, 383 shl 16 + 65512

                dec     esi
                jnz     .loop_ly

        ; reset letter from 0x0100 to 0x0000
        mov     dx, [letter]
        dec     dh
        mov     [letter], dx

        ; highlight of current character in table
        mov     al, [char]
        shr     al, 4
        mov     bl, 24
        mul     bl
        add     ax, 34
        shl     eax, 16
        mov     al, 0x01
        mov     ecx, eax
        push    ecx

        mov     eax, SF_DRAW_RECT
        mov     ebx, 34 shl 16 + 384
        mov     edx, [win_cols.work_button]

        ; skip horizontal lines aligned to table borders
        cmp     ecx, 34 shl 16 + 65536
        jle     .hl_next_hr
        mcall

        .hl_next_hr:

                add     ecx, 23 shl 16
                cmp     ecx, 417 shl 16
                jge     .hl_end_hr
                mcall

        .hl_end_hr:

        mov     al, [char]
        and     al, 0x0F
        mov     bl, 24
        mul     bl
        add     ax, 34
        shl     eax, 16
        mov     al, 0x01
        mov     ebx, eax

        mov     eax, SF_DRAW_RECT
        mov     ecx, 34 shl 16 + 384
        mov     edx, [win_cols.work_button]

        ; skip vertical lines aligned to table borders
        cmp     ebx, 34 shl 16 + 65536
        jle     .hl_next_vr
        mcall

        .hl_next_vr:
                add     ebx, 23 shl 16
                cmp     ebx, 417 shl 16
                jge     .hl_end_vr
                mcall

        .hl_end_vr:

        pop     ecx
        add     ecx, 23
        sub     ebx, 23 * 65535
        mcall

        ; redraw active symbol
        shr     ecx, 16
        mov     bx, cx
        add     ebx, 8 shl 16 + 5

        mov     cl, [charset]
        cmp     cl, 0xB0
        jne     .check_6x9
        mov     cl, 0xA0
        jmp     .char_draw_act

        .check_6x9:
                cmp     cl, 0x80
                jne     .char_draw_act
                add     ebx, 2 shl 16 + 3

        .char_draw_act:
                shl     ecx, 24
                add     ecx, [win_cols.work_button_text]
                mcall   SF_DRAW_TEXT, , , char_ascii

        ret


; single character big display, it's scale and codes
draw_update_single:

        ; background for single char
        mcall   SF_DRAW_RECT, <TB_SING.X + 1, TB_SING.W - 2>, <TB_SING.Y + 26, TB_SING.H - 102>, [win_cols.work_light]

        mov     ah, [page]
        mov     al, [char]
        mov     [char_ascii], ax

        ; shifting letter position depenging on scale
        mov     bl, 7
        sub     bl, [scale]
        mov     al, 7
        mul     bl
        add     ax, 136
        push    ax

        mov     bl, 7
        sub     bl, [scale]
        mov     al, 4
        mul     bl
        add     ax, 516

        shl     eax, 16
        pop     ax
        mov     ebx, eax

        cmp     [charset], 0xB0
        jne     .not_utf8

        ;utf 8 to 16 conversion
        .utf_8:
                xor     edx, edx
                mov     dx, [char_ascii]
                push    esi
                mov     esi, char_utf
                call    logic_utf16to8
                pop     esi

                mov     cl, 0xF0
                add     cl, [scale]
                shl     ecx, 24
                add     ecx, [win_cols.work_text]

                mcall   SF_DRAW_TEXT, , , char_utf, 0, [win_cols.work_light]

                jmp     .codes

        .not_utf8:
                mov     cl, [charset]
                cmp     cl, 0x80
                jne     .not_80

                ; shifting letter position depenging on scale
                xor     eax, eax
                mov     al, [scale]
                add     al, [scale]
                shl     eax, 16
                add     ebx, eax
                push    ebx

                mov     bl, 7
                sub     bl, [scale]
                mov     al, 3
                mul     bl
                pop     ebx
                sub     bx, ax
                add     ebx, 0 shl 16 + 25

        .not_80:
                add     cl, 0x40
                add     cl, [scale]
                shl     ecx, 24
                add     ecx, [win_cols.work_text]

        mcall   SF_DRAW_TEXT, , , char_ascii, 1, [win_cols.work_light]

        ; singe character codes
        .codes:

                ; character codes copy buttons
                mcall   SF_DEFINE_BUTTON, <BT_CP_AD.X, BT_CP_AD.W>, <BT_CP_AD.Y, BT_CP_AD.H>, 0x00000015, [win_cols.work_button]
                mcall                   ,                         , <BT_CP_SD.Y, BT_CP_SD.H>, 0x00000016,
                mcall                   , <BT_CP_AH.X, BT_CP_AH.W>, <BT_CP_AH.Y, BT_CP_AH.H>, 0x00000017,
                mcall                   ,                         , <BT_CP_SH.Y, BT_CP_SH.H>, 0x00000018,

                ; lines to overlap button's borders
                mcall   SF_DRAW_RECT, <LN_SN_02.X, LN_SN_02.W>, <LN_SN_05.Y, LN_SN_05.H>,
                mcall               ,                         , <LN_SN_06.Y, LN_SN_06.H>,
                mcall               ,                         , <LN_SN_06.Y + BT_CP_SH.H, LN_SN_06.H>,
                mcall               , <LN_SN_07.X, LN_SN_07.W>, <LN_SN_07.Y, LN_SN_07.H>,
                mcall               , <LN_SN_08.X, LN_SN_08.W>,                         ,
                mcall               , <LN_SN_08.X + BT_CP_SH.W, LN_SN_08.W>,                         ,

                xor     ecx, ecx
                mov     esi, [win_cols.work_button_text]
                add     esi, 0x10000000

                mov     cx, [char_ascii]
                mcall   SF_DRAW_NUMBER, 0x00050000, , <DT_CD_AD.X, DT_CD_AD.Y>,

                mov     cx, [char_scan]
                mcall                 ,           , , <DT_CD_SD.X, DT_CD_SD.Y>,

                mov     cx, [char_ascii]
                mcall                 , 0x00040100, , <DT_CD_AH.X, DT_CD_AH.Y>,

                mov     cx, [char_scan]
                mcall                 ,           , , <DT_CD_SH.X, DT_CD_SH.Y>,

        ; single character scale
        xor     ecx, ecx
        mov     cl, [scale]
        inc     cl
        mov     esi, [win_cols.work_text]
        add     esi, 0x50000000
        mcall   SF_DRAW_NUMBER, 0x80010000, , <DT_CH_SC.X, DT_CH_SC.Y>, , [win_cols.work_light]

        ret


; redraw keyboard input toggle button
draw_toggle:

        cmp     [reading], 0x01

        je      .stop

                mcall   SF_DEFINE_BUTTON, <BT_RD_ST.X, BT_RD_ST.W>, <BT_RD_ST.Y,  BT_RD_ST.H>, 0x00000012, [win_cols.work_button]

                mov     ecx, [win_cols.work_button_text]
                add     ecx, 0x90000000
                mcall   SF_DRAW_TEXT, <BT_RD_ST.X + 17, BT_RD_ST.Y + 5>, , bt_read

                ret

        .stop:
                mov     esi, [win_cols.work_button]
                or      esi, 0x00FF0000
                mcall   SF_DEFINE_BUTTON, <BT_RD_ST.X, BT_RD_ST.W>, <BT_RD_ST.Y,  BT_RD_ST.H>, 0x00000012,

                mov     ecx, [win_cols.work_button_text]
                add     ecx, 0x90000000
                mcall   SF_DRAW_TEXT, <BT_RD_ST.X + 17, BT_RD_ST.Y + 5>, , bt_stop

                ret


; draw charpage change buttons
draw_charpage_buttons:

        mcall   SF_DEFINE_BUTTON, <BT_CP_ZR.X, BT_CP_ZR.W>, <BT_CP_ZR.Y, BT_CP_ZR.H>, 0x0000000F, [win_cols.work_button]
        mcall                   , <BT_CP_DN.X, BT_CP_DN.W>,                         , 0x00000010,
        mcall                   , <BT_CP_UP.X, BT_CP_UP.W>,                         , 0x00000011,

        mov      ecx, [win_cols.work_button_text]
        add      ecx, 0xB0000000

        ; page swap buttons subscriptions
        mcall   SF_DRAW_TEXT, <LB_CP_ZR.X, LB_CP_ZR.Y>, , bt_res
        mcall               , <LB_CP_DN.X, LB_CP_DN.Y>, , bt_dec
        mcall               , <LB_CP_UP.X, LB_CP_UP.Y>, , bt_inc

        ret

; delete charpage change buttons
draw_charpage_delete:

        mcall   SF_DEFINE_BUTTON, , , 0x8000000F
        mcall                   , , , 0x80000010
        mcall                   , , , 0x80000011

        mcall   SF_DRAW_RECT, <BT_CP_ZR.X, BT_CP_ZR.W + BT_CP_DN.W + BT_CP_UP.W + 19>, <BT_CP_ZR.Y, BT_CP_ZR.H + 1>, [win_cols.work]

        ret

; ===================================================================

WN_RECT         RECT    100, 100, 686, 494      ; window

TB_PAGE         RECT      8,   8, 411, 411      ; tables background
TB_SING         RECT    427,   8, 242, 411

BT_TABL         RECT     34,  34, 384, 384      ; button to pick char

BT_CS_01        RECT      8, 459,  95,  23      ; charsets buttons
BT_CS_02        RECT    113, 459,  95,  23
BT_CS_03        RECT    218, 459,  95,  23
BT_CS_04        RECT    323, 459,  95,  23

BT_CP_ZR        RECT    323, 427,  31,  23      ; charpage buttons
BT_CP_DN        RECT    363, 427,  23,  23
BT_CP_UP        RECT    395, 427,  23,  23

BT_SC_DN        RECT    618,   8,  25,  25      ; char scale buttons
BT_SC_UP        RECT    643,   8,  25,  25

BT_CP_AD        RECT    562, 368,  57,  25      ; char code copy buttons
BT_CP_SD        RECT    562, 393,  57,  25
BT_CP_AH        RECT    619, 368,  49,  25
BT_CP_SH        RECT    619, 393,  49,  25

BT_RD_ST        RECT    427, 459, 241,  23      ; key input read/stop button

LN_PG_01        RECT      8,  33, 410,   1      ; page table lines
LN_PG_02        RECT     33,   8,   1, 410

LN_SN_01        RECT    668,   8,   1, 410      ; singe table lines
LN_SN_02        RECT    427,   8, 242,   1
LN_SN_03        RECT    427,  33, 242,   1
LN_SN_04        RECT    427, 343, 242,   1
LN_SN_05        RECT    427, 368, 242,   1
LN_SN_06        RECT    427, 393, 242,   1
LN_SN_07        RECT    562, 343,   1,  75
LN_SN_08        RECT    619, 343,   1,  75

LB_SN_SCL       RECT    436,  14,   0,   0      ; single table subscriptions
LB_SN_DEC       RECT    579, 349,   0,   0
LB_SN_HEX       RECT    632, 349,   0,   0
LB_SN_ASC       RECT    436, 374,   0,   0
LB_SN_SCN       RECT    436, 399,   0,   0

LB_CS_01        RECT     20, 464,   0,   0      ; charsets buttons subscriptions
LB_CS_02        RECT    121, 464,   0,   0
LB_CS_03        RECT    222, 464,   0,   0
LB_CS_04        RECT    331, 464,   0,   0

LB_CP_ZR        RECT    331, 432,   0,   0      ; charpage buttons subscriptions
LB_CP_DN        RECT    370, 432,   0,   0
LB_CP_UP        RECT    403, 432,   0,   0

LB_SC_DN        RECT    627,  13,   0,   0      ; char scale subscriptions
LB_SC_UP        RECT    652,  13,   0,   0

DT_CD_AD        RECT    571, 374,   0,   0      ; char codes
DT_CD_SD        RECT    571, 399,   0,   0
DT_CD_AH        RECT    628, 374,   0,   0
DT_CD_SH        RECT    628, 399,   0,   0
DT_CH_SC        RECT    508,  14,   0,   0      ; char scale

HD_TABL         RECT     38,  14,   0,   0      ; char table headers

; ===================================================================

if lang eq ru_RU

        title   cp866 "Таблицы Символов 0.4.2", 0
        lb_size cp866 "Масштаб:", 0
        lb_asci cp866 "ASCII-код", 0
        lb_scan cp866 "Скан-код", 0
        bt_read cp866 " Читать ввод с клавиатуры ", 0
        bt_stop cp866 "  Прекратить читать ввод  ", 0

else if lang eq es_ES

        title   db "Tablas de Simbolos 0.4.2", 0
        lb_size db "Escala:", 0
        lb_asci db "Codigo ASCII", 0
        lb_scan db "Codigo scan", 0
        bt_read db " Leer entrada del teclano ", 0
        bt_stop db "Detener entrada de teclano", 0
else

        title   db "Charsets Viewer 0.4.2", 0
        lb_size db "Scale:", 0
        lb_asci db "ASCII-code", 0
        lb_scan db "Scan-code", 0
        bt_read db "   Start keyboard input   ", 0
        bt_stop db "    End keyboard input    ", 0

endf

lb_cp6x9        db "CP866 6x9  ", 0
lb_cp8x16       db "CP866 8x16 ", 0
lb_utf16        db "UTF-16 8x16", 0
lb_utf8         db "UTF-8 8x16 ", 0

lb_curr         dd lb_utf8

lb_hex          db "HEX", 0
lb_dec          db "DEC", 0

bt_res          db "00", 0
bt_dec          db "<", 0
bt_inc          db ">", 0

bt_smaller      db "-", 0
bt_bigger       db "+", 0

; ===================================================================

win_cols        system_colors

; ===================================================================

reading         db 0x00

header          dw 0x30FF, 0            ; "-0" symbols
letter          dw 0x0000, 0            ; buffer for chars in page table
letutf          dd 0x00000000, 0        ; same, but for UTF-8 chars

charset         db 0xB0
page            db 0x00
page_utf        db 0x00
char            db 0x00
scale           db 0x07

char_ascii      dw 0x0000, 0            ; current char ASCII/CP866 code
char_scan       dw 0x0000, 0            ; current char SCAN-code
char_utf        dd 0x00000000, 0        ; current char UTF-8 code

char_code_dec:
                dd char_code_dec.end - char_code_dec
                dd 0
                dd 1
                db '00000'
.end:

char_code_hex:
                dd char_code_hex.end - char_code_hex
                dd 0
                dd 1
                db '0000'
.end:

; ===================================================================

I_END:
        rb      512
        align   16
STACKTOP:

MEM: