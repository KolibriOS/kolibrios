; SPDX-License-Identifier: GPL-2.0-only
; SPDX-FileCopyrightText: 2024-2025 KolibriOS Team

; ================================================================

format binary as ""
use32
org 0

; ================================================================

db 'MENUET01'
dd 1
dd START
dd I_END
dd MEM
dd STACKTOP
dd 0, 0

; ================================================================

include '../../macros.inc'

START:                                  ; start of execution
        call    draw_window             ; draw the window

event_wait:
        mov     eax, 10                 ; function 10 : wait until event
        mcall                           ; event type is returned in eax

        cmp     eax, 1                  ; Event redraw request
        je      red                     ; Expl.: there has been activity on screen and
                                        ; parts of the applications has to be redrawn.
        cmp     eax, 2                  ; Event key in buffer
        je      key                     ; Expl.: User has pressed a key while the
                                        ; app is at the top of the window stack.
        cmp     eax, 3                  ; Event button in buffer
        je      button                  ; Expl.: User has pressed one of the
                                        ; applications buttons.
        jmp     event_wait

red:                                    ; Redraw event handler
        call    draw_window             ; We call the window_draw function and
        jmp     event_wait              ; jump back to event_wait

key:                                    ; Keypress event handler
        mov     eax, 2                  ; The key is returned in ah. The key must be
        mcall                           ; read and cleared from the system queue.
        cmp     eax, 1                  ; Just read the key, ignore it and jump to event_wait.
        je      event_wait
        mov     cl, [reading]
        cmp     cl, 0x00
        je      event_wait
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
        call    draw_update
        jmp     event_wait

button:                                  ; Buttonpress event handler
        mov     eax, 17                  ; The button number defined in window_draw
        mcall

        .close:
                cmp     ah, 1
                jne     .button_a
                mov     eax, -1
                mcall

        .button_a:                        ; select character
                cmp     ah, 0x0A
                jne     .button_b
                mcall   37, 1
                push    eax
                sub     ax, 34
                mov     bl, 24
                div     bl
                mov     cl, al
                shl     cl, 4
                pop     eax
                shr     eax, 16
                sub     ax, 34
                div     bl
                add     cl, al
                mov     [char], cl
                mov     [char_scan], 0x0000
                call    logic_update
                call    draw_update
                jmp     event_wait

        .button_b:                        ; charset CP866 6x9
                cmp     ah, 0x0B
                jne     .button_c
                mov     [charset], 0x80
                mov     [lb_curr], lb_cp6x9
                call    draw_update
                jmp     event_wait

        .button_c:                        ; charset CP866 8x16
                cmp     ah, 0x0C
                jne     .button_d
                mov     [charset], 0x90
                mov     [lb_curr], lb_cp8x16
                call    draw_update
                jmp     event_wait

        .button_d:                        ; charset UTF-16 8x16
                cmp     ah, 0x0D
                jne     .button_e
                mov     [charset], 0xA0
                mov     [lb_curr], lb_utf16
                call    draw_update
                jmp     event_wait

        .button_e:                        ; charset UTF-8 8x16
                cmp     ah, 0x0E
                jne     .button_f
                mov     [charset], 0xB0
                mov     [lb_curr], lb_utf8
                call    draw_update
                jmp     event_wait

        .button_f:                        ; charpage reset
                cmp     ah, 0x0F
                jne     .button_10
                mov     [page], 0x00
                call    logic_update
                call    draw_update
                jmp     event_wait

        .button_10:                        ; charpage decrement
                cmp     ah, 0x10
                jne     .button_11
                mov     ch, [page]
                dec     ch
                mov     [page], ch
                call    logic_update
                call    draw_update
                jmp     event_wait

        .button_11:                        ; charpage increment
                cmp     ah, 0x11
                jne     .button_12
                mov     ch, [page]
                inc     ch
                mov     [page], ch
                call    logic_update
                call    draw_update
                jmp     event_wait

        .button_12:                        ; read/stop keyboard input
                cmp     ah, 0x12
                jne     event_wait
                mov     al, 0x01
                sub     al, [reading]
                mov     [reading], al
                call    draw_toggle


        jmp     event_wait

; ================================================================

draw_window:

        mcall   12, 1

        mcall   48, 3, window_colors, 40

        mcall     , 4
        add       eax, 3
        mov       [win_head], eax

        mov     eax, 0
        mov     ebx, 100 * 65536 + 685
        mov     ecx, 100 * 65536 + 518
        mov     edx, [window_colors.work]
        add     edx, 0x34000000
        mov     edi, title
        mcall

        ; Don't draw rolled up or rolled down window
        mcall   9, proc_info, -1
        mov     eax, [proc_info + 70]
        mov     [win_stat], eax
        test    [win_stat], 100b
        jnz     .draw_end

        ; Draw all app content
        add     [win_head], 492

        call    draw_base
        call    draw_update
        call    draw_toggle

        .draw_end:
                mov     esi, [win_head]
                mcall   67, -1, -1, -1,
                mcall   12, 2

        ret

; ================================================================

; unchangeble base - table, headers and buttons
draw_base:

        .tables:
                ; both tables background
                mcall   13, 65536 *   9 + 410, 65536 *   9 + 410, [window_colors.work_light]
                mcall     , 65536 * 428 + 240, 65536 *   9 + 442,

                ; 16x16 characters table
                mcall     , 65536 *   8 + 411, 65536 *   8 +   1, [window_colors.work_text]
                mcall     ,                  , 65536 *  33 +   1,
                mcall     ,                  , 65536 * 418 +   1,
                mcall     , 65536 *   8 +   1, 65536 *   8 + 410,
                mcall     , 65536 *  33 +   1,                  ,
                mcall     , 65536 * 418 +   1,                  ,

                ; single character table
                mcall     , 65536 * 427 +   1, 65536 *   8 + 443,
                mcall     , 65536 * 668 +   1,                  ,
                mcall     , 65536 * 427 + 242, 65536 *   8 +   1,
                mcall     ,                  , 65536 * 376 +   1,
                mcall     ,                  , 65536 * 401 +   1,
                mcall     ,                  , 65536 * 426 +   1,
                mcall     ,                  , 65536 * 451 +   1,
                mcall     , 65536 * 562 +   1, 65536 * 377 +  75,
                mcall     , 65536 * 619 +   1,                  ,

        .headers:
                ; horizontal table headers
                mov     eax, 4
                mov     ebx, 65536 * 38 + 14
                mov     ecx, [window_colors.work_text]
                add     ecx, 0x90000000
                mov     esi, 16

                .loop_hx:
                        mov     edx, header
                        mcall

                        mov     dx, [header]
                        add     dx, 0x0100

                        cmp     dx, 0x3A2D
                        jne     .hx_af
                        add     dx, 0x0700

                        .hx_af:
                        mov     [header], dx
                        add     ebx, 65536 * 24
                        dec     esi
                        jnz     .loop_hx

                ; vertical headers
                mov     ebx, 65536 * 13 + 39
                mov     esi, 16
                mov     [header], 0x2D30

                .loop_hy:
                        mov     edx, header
                        mcall

                        mov     dx, [header]
                        add     dx, 0x0001

                        cmp     dx, 0x2D3A
                        jne     .hy_af
                        add     dx, 0x0007

                        .hy_af:
                        mov     [header], dx
                        add     ebx, 24
                        dec     esi
                        jnz     .loop_hy

                ; reset headers
                mov     [header], 0x302D

                ; single character table headers
                mcall     , 65536 * 579 + 382, ,  lb_dec,
                mcall     , 65536 * 632 + 382, ,  lb_hex,
                mcall     , 65536 * 436 + 407, , lb_asci,
                mcall     , 65536 * 436 + 432, , lb_scan,

        .buttons:
                ; button on table to pick single character
                mcall    8, 65536 *  34 + 384, 65536 *  34 + 384, 0x6000000A,

                ; charsets change buttons
                mcall     , 65536 *   8 +  95, 65536 * 459 +  23, 0x0000000B, [window_colors.work_button]
                mcall     , 65536 * 113 +  95,                  , 0x0000000C,
                mcall     , 65536 * 218 +  95,                  , 0x0000000D,
                mcall     , 65536 * 323 +  95,                  , 0x0000000E,

                ; page swap buttons
                mcall     , 65536 * 323 +  31, 65536 * 427 +  23, 0x0000000F,
                mcall     , 65536 * 363 +  23,                  , 0x00000010,
                mcall     , 65536 * 395 +  23,                  , 0x00000011,

                ; charsets change buttons subscriptions
                mov      ecx, [window_colors.work_button_text]
                add      ecx, 0xB0000000
                mcall    4, 65536 *  20 + 464, , lb_cp6x9
                mcall     , 65536 * 121 + 464, , lb_cp8x16
                mcall     , 65536 * 222 + 464, , lb_utf16
                mcall     , 65536 * 331 + 464, , lb_utf8

                ; page swap buttons subscriptions
                mcall     , 65536 * 331 + 432, , bt_res
                mcall     , 65536 * 370 + 432, , bt_dec
                mcall     , 65536 * 403 + 432, , bt_inc

        ret

; changable data: current charset, charpage, chars, single char and it's codes
draw_update:

        ; background for letters
        mcall   13, 65536 * 34 + 384, 65536 * 34 + 384, [window_colors.work_light]

        ; current charset and charpage
        .charpage:
                ; current charpage
                mov     esi, [window_colors.work_text]
                add     esi, 0x50000000
                mcall   47, 65536 * 2 + 257, page, 0x000D000E, , [window_colors.work_light]

                ; current charset
                mov     ecx, [window_colors.work_text]
                add     ecx, 0xD0000000
                mcall   4, 65536 * 8 + 432, , [lb_curr], , [window_colors.work]

        ; 16x16 table of letters
        .letters:
                ;different coordinates for 6x9 charset
                mov     bl, [charset]
                cmp     bl, 0x80
                jne     .char_big

                .char_sm:
                        mov     ebx, 65536 * 44 + 42
                        jmp     .char_draw

                .char_big:
                        mov     ebx, 65536 * 42 + 39

                .char_draw:
                        mov     cl,  [charset]
                        shl     ecx, 24
                        add     ecx, [window_colors.work_text]
                        mov     esi, 16

                ; letters draw loop
                .loop_ly:
                        mov     edi, 16

                        .loop_lx:
                                mov     edx, letter

                                cmp     [charset], 0xB0
                                jne     .skip_lx

                                ;utf 8 to 16
                                xor     edx, edx
                                mov     dx, [letter]
                                push    esi
                                mov     esi, letutf
                                call    logic_utf16to8
                                pop     esi
                                mov     edx, letutf

                                .skip_lx:

                                mcall

                                mov     dx, [letter]
                                add     dx, 0x01
                                mov     [letter], dx

                                add     ebx, 65536 * 24

                                dec     edi
                                jnz     .loop_lx

                        ; start new row of letters
                        sub     ebx, 65536 * 383 + 65512

                        dec     esi
                        jnz     .loop_ly

                ; reset letter from 0x0100 to 0x0000
                mov     dx, [letter]
                dec     dh
                mov     [letter], dx

        ; highlight of current character in table
        .highlight:
                mov     al, [char]
                shr     al, 4
                mov     bl, 24
                mul     bl
                add     ax, 34
                shl     eax, 16
                mov     al, 0x01
                mov     ecx, eax
                push    ecx

                mov             eax, 13
                mov             ebx, 65536 * 34 + 384
                mov             edx, [window_colors.work_button]

                ;skip lines aligned to table borders
                cmp             ecx, 65536 * 34 + 65536
                jle             .hl_next_hr
                mcall
                .hl_next_hr:
                        add     ecx, 65536 * 23
                        cmp     ecx, 65536 * 417
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

                mov     eax, 13
                mov     ecx, 65536 * 34 + 384
                mov     edx, [window_colors.work_button]

                ;skip lines aligned to table borders
                cmp     ebx, 65536 * 34 + 65536
                jle     .hl_next_vr
                mcall
                .hl_next_vr:
                        add     ebx, 65536 * 23
                        cmp     ebx, 65536 * 417
                        jge     .hl_end_vr
                        mcall
                .hl_end_vr:

                pop     ecx
                add     ecx, 23
                sub     ebx, 65535 * 23
                mcall

                ; redraw active symbol
                shr     ecx, 16
                mov     bx, cx
                add     ebx, 65536 * 8 + 5

                mov     cl, [charset]
                cmp     cl, 0xB0
                jne     .check_80
                mov     cl, 0xA0
                jmp     .process

                .check_80:
                cmp     cl, 0x80
                jne     .process
                add     ebx, 65536 * 2 + 3

                .process:
                shl     ecx, 24
                add     ecx, [window_colors.work_button_text]
                mcall   4, , , char_ascii

        ; single character big display
        .single:

                mcall   13, 65536 * 452 + 192, 65536 * 24 + 336, [window_colors.work_light]
                mov     ah, [page]
                mov     al, [char]
                mov     [char_ascii], ax

                cmp     [charset], 0xB0

                jne     .skip_sn
                        ;utf 8 to 16
                        xor     edx, edx
                        mov     dx, [char_ascii]
                        push    esi
                        mov     esi, char_utf
                        call    logic_utf16to8
                        pop     esi

                        mov     ecx, 0xF7000000
                        add     ecx, [window_colors.work_text]
                        mcall   4, 65536 * 516 + 136, , char_utf, 0, [window_colors.work_light]
                        jmp     .codes

                .skip_sn:
                        mov     ebx, 65536 * 516 + 136
                        mov     cl, [charset]

                        cmp     cl, 0x80
                        jne     .not_80
                        add     ebx, 65536 * 12 + 29

                        .not_80:
                        add     cl, 0x07
                        shl     ecx, 24
                        add     ecx, [window_colors.work_text]
                        mcall   4, , , char_ascii, 1, [window_colors.work_light]

        ; singe character codes
        .codes:

                mov     esi, [window_colors.work_text]
                add     esi, 0x50000000

                xor     ecx, ecx

                mov     cx, [char_ascii]
                mcall   47, 0x00050000, , 65536 * 571 + 407, , [window_colors.work_light]
                mov     cx, [char_scan]
                mcall     ,           , , 65536 * 571 + 432,

                mov     cx, [char_ascii]
                mcall     , 0x00040100, , 65536 * 628 + 407,
                mov     cx, [char_scan]
                mcall     ,           , , 65536 * 628 + 432,

        ret

; redraw keyboard input toggle button
draw_toggle:

        mcall   8, 65536 * 427 + 241, 65536 * 459 +  23, 0x00000012, [window_colors.work_button]

        mov     ecx, [window_colors.work_button_text]
        add     ecx, 0xB0000000

        mov     al, [reading]
        cmp     al, 0x01

        je      .stop
                mcall     4, 65536 * 472 + 464, , bt_read
                ret
        .stop:
                mcall     4, 65536 * 472 + 464, , bt_stop
                ret

; ================================================================

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

; ================================================================

title   db "Charsets Viewer 0.3.2", 0

lb_cp6x9        db "CP866 6x9  ", 0
lb_cp8x16       db "CP866 8x16 ", 0
lb_utf16        db "UTF-16 8x16", 0
lb_utf8         db "UTF-8 8x16 ", 0

lb_curr         dd lb_utf8

lb_hex  db "HEX", 0
lb_dec  db "DEC", 0
lb_asci db "ASCII-code", 0
lb_scan db "Scan-code", 0

bt_res  db "00", 0
bt_dec  db "<", 0
bt_inc  db ">", 0

bt_read db "Read keyboard input", 0
bt_stop db "Stop keyboard input", 0

reading db 0x00

header  dw 0x302D, 0        ; "-0" symbols
letter  dw 0x0000, 0
letutf  dd 0x00000000, 0

charset db 0xB0
page    db 0x00
char    db 0x00

char_ascii      dw 0x0000, 0
char_scan       dw 0x0000, 0
char_utf        dd 0x00000000, 0

win_stat        rd 1
win_head        rd 1

window_colors   system_colors
proc_info       process_information

; ================================================================

I_END:
        rb      4096
        align   16
STACKTOP:

MEM:
