use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,0,0

include '../../../proc32.inc'
include '../../../macros.inc'

BUTTON_SIDE                     equ     16      ; button are squares
BUTTON_SPACE                    equ     19      ; space between cols and rows
BUTTON_ID_SHIFT                 equ     2       ; button_id = character + BUTTON_ID_SHIFT
TABLE_BEGIN_X                   equ     2
TABLE_BEGIN_Y                   equ     2

FOCUS_SQUARE_COLOR              equ     0x000080FF
PAGE_SWITCHER_BLINK_COLOR       equ     0x00808080


start:
still:
        mcall   10
        dec     eax
        jz      redraw
        dec     eax
        jz      key

button:
        mcall   17
        shr     eax, 8

        cmp     eax, 1
        je      quit
        cmp     ax, 0xFFAA                      ; page switcher
        je      .switch_page                    ; any button with a character
  .change_focus:
        mov     bl, [symbol_focused]
        mov     [symbol_unfocused], bl
        sub     ax, BUTTON_ID_SHIFT             ; get the corresponding character
        mov     [symbol_focused], al
        stdcall draw_table, 0
        call    draw_codes
        jmp     still
  .switch_page:
        movzx   bx, [symbol_start]
        add     bx, BUTTON_ID_SHIFT
        mov     cx, 128                         ; half of page
        mov     edx, 0x80000000
        mov     dx, bx
    @@: mcall   8
        inc     edx
        dec     cx
        jnz     @b

    @@: add     [symbol_start], 128             ; change page
        add     [symbol_focused], 128
        stdcall draw_table, 1                   ; 1 means redraw the whole table
        call    draw_codes
        stdcall draw_page_switcher, 1           ; 1 means dark color, for blinking
        mcall   5, 10
        stdcall draw_page_switcher, 0           ; 0 means usual light color
        jmp     still

redraw:
        mcall   9, proc_info, -1
        mcall   48, 3, sc, 40

        mcall   12, 1

        mcall   48, 4                           ; get skin height
        mov     ecx, 300*0x10000+184
        add     ecx, eax
        mov     edx, 0x34000000
        or      edx, [sc.work]
        mov     esi, 0x80000000
        or      esi, [sc.grab_text]
        mcall   0, <300,315>, , , , window_title
        test    [proc_info.wnd_state], 0x04
        jnz     @f

        stdcall draw_table, 1
        call    draw_codes
        stdcall draw_page_switcher, 0

    @@:
        mcall   12, 2
        jmp     still

key:
        mcall   2
        cmp     ah, 0x09                        ; TAB key
        je      button.switch_page

        cmp     ah, 0xB0                        ; left
        jne     @f
        mov     bl, [symbol_focused]
        mov     [symbol_unfocused], bl
        dec     bl
        and     bl, 0x0f
        and     [symbol_focused], 0xf0
        or      [symbol_focused], bl
        stdcall draw_table, 0
        call    draw_codes
        jmp     still

    @@: cmp     ah, 0xB1                        ; down
        jne     @f
        mov     bl, [symbol_focused]
        mov     [symbol_unfocused], bl
        add     bl, 16
        and     bl, 0x70
        and     [symbol_focused], 0x8f
        or      [symbol_focused], bl
        stdcall draw_table, 0
        call    draw_codes
        jmp     still

    @@: cmp     ah, 0xB2                        ; up
        jne     @f
        mov     bl, [symbol_focused]
        mov     [symbol_unfocused], bl
        sub     bl, 16
        and     bl, 0x70
        and     [symbol_focused], 0x8f
        or      [symbol_focused], bl
        stdcall draw_table, 0
        call    draw_codes
        jmp     still

    @@: cmp     ah, 0xB3                        ; righ
        jne     @f
        mov     bl, [symbol_focused]
        mov     [symbol_unfocused], bl
        inc     bl
        and     bl, 0x0f
        and     [symbol_focused], 0xf0
        or      [symbol_focused], bl
        stdcall draw_table, 0
        call    draw_codes
        jmp     still
        jne     @f

    @@:
        jmp     still



proc    draw_table _full_redraw

        mov     al, [symbol_start]
        mov     [symbol_current], al

  .next_button:

        xor     edi, edi                        ; character focus flag
        mov     al, [symbol_current]
        cmp     al, [symbol_focused]
        jne     @f
        inc     edi
    @@: cmp     [_full_redraw], 1
        je      .draw
        cmp     al, [symbol_focused]
        je      .draw
        cmp     al, [symbol_unfocused]          ; previously focused, should redraw to clear focus
        je      .draw
        jmp     .skip                           ; skip button if it isn't (un)focused

  .draw:
        call    draw_button
  .skip:
        mov     bl, [symbol_start]
        add     bl, 127                         ; end of current page
        cmp     [symbol_current], bl            ; the last on page?
        jne     @f
        mov     [button_x], TABLE_BEGIN_X
        mov     [button_y], TABLE_BEGIN_Y
        ret
    @@: inc     [symbol_current]
        add     [button_x], BUTTON_SPACE
        cmp     [button_x], 306                 ; the last in row?
        jne     .next_button
        add     [button_y], BUTTON_SPACE        ; next row
        mov     [button_x], TABLE_BEGIN_X
        jmp     .next_button
        ret
endp


proc    draw_button
        mov     ebx, [button_x]
        shl     ebx, 16
        mov     bx, BUTTON_SIDE
        mov     ecx, [button_y]
        shl     ecx, 16
        mov     cx, BUTTON_SIDE
        mov     edx, 0x80000000
        mov     dl, [symbol_current]
        add     edx, BUTTON_ID_SHIFT
        mcall   8, , ,
        and     edx, 0x7FFFFFFF
        or      edx, 0x20000000
        mcall   , , , , [sc.work_button]

        test    edi, edi                        ; is focused?
        jz      .symbol                         ; draw only character, not selection square
  .focus_frame:                                 ; draw a blue square (selection), 8 segments
        mov     esi, [button_x]
        mov     edi, [button_y]

        mov     bx, si
        shl     ebx, 16
        mov     bx, si
        add     bx, BUTTON_SIDE
        mov     cx, di
        shl     ecx, 16
        mov     cx, di
        mcall   38, , , FOCUS_SQUARE_COLOR
        add     ecx, 0x00010001
        mcall
        add     ecx, (BUTTON_SIDE-2)*0x10000+(BUTTON_SIDE-2)
        mcall
        add     ecx, 0x00010001
        mcall

        mov     bx, si
        shl     ebx, 16
        mov     bx, si
        mov     cx, di
        shl     ecx, 16
        mov     cx, di
        add     ecx, 2*0x10000+(BUTTON_SIDE-2)
        mcall   38, , ,
        add     ebx, 0x00010001
        mcall
        add     ebx, (BUTTON_SIDE-2)*0x10000+(BUTTON_SIDE-2)
        mcall
        add     ebx, 0x00010001
        mcall

  .symbol:
        mov     ebx, [button_x]
        add     ebx, 6
        shl     ebx, 16
        add     ebx, [button_y]
        add     ebx, 5
        mcall   4, , [sc.work_button_text], symbol_current, 1

        ret
endp


proc    draw_page_switcher _blinking

        mcall   8, , , 0x8000FFAA

        mov     esi, [sc.work_button]
        cmp     [_blinking], 1                  ; blinking?
        jne     @f
        mov     esi, PAGE_SWITCHER_BLINK_COLOR
    @@: mcall   , <2,60>, <157,19>, 0x2000FFAA

        mov     ecx, 0x80000000
        or      ecx, [sc.work_button_text]
        mov     edx, string_000_127
        cmp     [symbol_start], 0               ; first page?
        je      @f
        mov     edx, string_128_255             ; ok, the second one
    @@: mcall   4, <11,164>,
        ret
endp


proc    draw_codes

        mov     ecx, 0x80000000
        or      ecx, [sc.work_text]
        mcall   4, <80,164>, , string_ASCII_CODE
        mcall   , <180,164>, , string_ASCII_HEX_CODE
        movzx   ecx, [symbol_focused]
        mov     esi, 0x40000000
        or      esi, [sc.work_text]
        mcall   47, 0x00030000, , <152,164>, , [sc.work]
        mcall   , 0x00020100, , <276,164>,

        ret
endp


quit:
        mcall   -1


szZ window_title                ,'ASCIIVju v0.4'
szZ string_000_127              ,'000-127'
szZ string_128_255              ,'128-255'
szZ string_ASCII_CODE           ,'ASCII Code:    '
szZ string_ASCII_HEX_CODE       ,'ASCII Hex-Code:   '

button_x                dd 2
button_y                dd 2

symbol_current          db 0
symbol_start            db 0

symbol_unfocused        db 0
symbol_focused          db 0
i_end:
proc_info               process_information
sc                      system_colors
rb 0x400                                        ;stack
e_end:
