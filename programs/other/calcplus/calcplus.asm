    ; VERSION equ "v1.0"

    use32
    org     0
    db	    'MENUET01'
    dd	    1, main, dataend, memory, stacktop, 0, 0

    include "../../proc32.inc"
    include "../../macros.inc"
    include "../../dll.inc"
    include "../../develop/libraries/box_lib/trunk/box_lib.mac"
  ;  include "../../debug.inc"
    include "parser.inc"

 ;===============================

    LIST_X           = 5
    LIST_Y           = 43
    LIST_ITEM_H      = 26
    LIST_ITEM_W      = 320 - 25
    LIST_ITEM_COUNT  = 8
    LIST_H           = LIST_ITEM_H * LIST_ITEM_COUNT
    LIST_TEXT_Y      = LIST_ITEM_H / 2 - 8
    
    KEYB_BTN_W       = 37
    KEYB_BTN_H       = 37
    GAP              = 5
    
    KEYBOARD_X       = LIST_ITEM_W + 20
    KEYBOARD_Y       = LIST_Y
    KEYBOARD_W       = 190
    WIN_W            = LIST_ITEM_W + 16 + KEYBOARD_W
    WIN_H            = LIST_H + 58
    
    LIST_ITEM_TEXT1  equ dword [sc.work_text]
    LIST_ITEM_TEXT2  equ dword [sc.work_text]


    sz_head db "Calc+", 0
    btn_clr db ""
    buttons db "|%^*/-+)(=7894561230"
    edb1    edit_box 0, 8, 12, 0xffffff,0x94AECE,0xffc90E,0xCACACA, 0x10000000, 480, \
		     exp, group, ed_always_focus + ed_focus, 0, 0

 imports:
    library gui, "box_lib.obj"
    import  gui, editbox.draw,	"edit_box",	\
		 editbox.key,	"edit_box_key", \
		 editbox.mouse, "edit_box_mouse"

 ;===============================

 main:
    mov     [ans.buffer], dword "= 0"
    mov     [ans.size], 3 * 8 + 9

    mcall   40, EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE

    m2m     [edb1.color],              [scn.gui_face]
    ;m2m     [edb1.shift_color],        [scn.gui_select]
    ;m2m     [edb1.focus_border_color], [scn.gui_face]
    ;m2m     [edb1.text_color],         [scn.gui_text]

    mcall   68, 11
    stdcall dll.Load, imports

 ;----------------------

 update:
    mcall   23, 5

    cmp     eax, EV_REDRAW
    je	    ev_redraw
    cmp     eax, EV_KEY
    je	    ev_key
    cmp     eax, EV_BUTTON
    je	    ev_button
    cmp     eax, EV_MOUSE
    je	    ev_mouse

    jmp     update

 ;----------------------

 ev_redraw:
    mcall   12, 1
    mcall   48, 3, sc, sizeof.system_colors

 ; WINDOW
    mov     edx, [sc.work]
    or      edx, 0x34 shl 24
    
    mcall   48, 4 ;get skin_h
    mov     ecx, 200 shl 16 + WIN_H
    add     ecx, eax ; add skin_h to WIN_H

    mcall   14 ; get screen size
    shr     eax, 16
    mov     ebx, eax
    sub     ebx, WIN_W
    shr     ebx, 1
    
    mcall   0, <ebx, WIN_W>, , , , sz_head

 ; CONTENT
    call    draw_textbox
    call    draw_keyb
    call    draw_list

    mcall   12, 2

    jmp     update

 ;----------------------

 ev_key:
    mcall   2
    cmp     ah, 27
    je	    exit
    cmp     ah, 13
    je	    calc
    invoke  editbox.key, edb1
    jmp     update

 ;----------------------

 ev_button:
    mcall   17

 ; EXIT
    cmp     ah, 1
    je	    exit

    cmp     ah, 250
    jne     @f
    mcall   70, f70_calc
    jmp     update
 ; DELETE
 @@:
    cmp     ah, 4
    jne     .not_del
    cmp     [edb1.pos], 0
    je	    update
    mov     eax, exp
    add     eax, [edb1.pos]
    dec     eax
    mov     ebx, exp
    add     ebx, [edb1.size]
    inc     ebx
  @@:
    cmp     eax, ebx
    je	    @f
    mov     cl, [eax + 1]
    mov     [eax], cl
    inc     eax
    jmp     @b
  @@:
    dec     [edb1.pos]
    dec     [edb1.size]
    m2m     [edb1.shift], [edb1.pos]
    jmp     .redraw
 .not_del:

 ; CALCULATE
    cmp     ah, 19
    je	    calc

 ; LIST
    cmp     ah, 0x60
    jl	    .not_list

    sub     ah, 0x60
    movzx   ebx, ah
    imul    ebx, 512
    add     ebx, history
    add     ebx, 482
    stdcall str_len, ebx

    mov     edx, exp
    add     edx, [edb1.size]
    add     edx, eax
    mov     edi, exp
    add     edi, [edb1.pos]
  @@:
    cmp     edx, edi
    je	    @f
    mov     esi, edx
    sub     esi, eax
    push    eax
    mov     al, [esi]
    mov     [edx], al
    pop     eax
    dec     edx
    jmp     @b
  @@:
    add     [edb1.size], eax

  @@:
    cmp     eax, 0
    je	    @f
    mov     cl, [ebx]
    mov     [edi], cl
    inc     edi
    inc     ebx
    dec     eax
    inc     [edb1.pos]
    jmp     @b
  @@:

    jmp     .redraw
 .not_list:

 ; KEYBOARD
    cmp     ah, 10
    jl	    update
    cmp     ah, 50
    jg	    update
    mov     dh, ah

    mov     ebx, exp
    add     ebx, [edb1.size]
    mov     ecx, exp
    add     ecx, [edb1.pos]
    cmp     dh, 30
    jl	    @f
    add     ebx, 2
  @@:
    cmp     ebx, ecx
    je	    @f
    mov     dl, [ebx - 1]
    mov     [ebx], dl
    dec     ebx
    jmp     @b
  @@:

    movzx   eax, dh
    add     eax, buttons
    sub     eax, 10
    mov     al, [eax]

    mov     [ebx], al
    inc     [edb1.size]
    inc     [edb1.pos]
    cmp     dh, 30
    jl	    @f
    add     [edb1.size], 2
    add     [edb1.pos], 2
  @@:
 .redraw:
    call    draw_textbox
    jmp     update

 ;----------------------

 ev_mouse:
    mcall   2

    invoke  editbox.mouse, edb1
    jmp     update

 ;----------------------

 exit:
    mcall   -1

 ;----------------------

 calc:
    stdcall parse
    cmp     [error_n], 0
    jne     .error

    mov     [ans.buffer], word "= "

    stdcall convert_to_str, eax, ans.buffer + 2
    add     eax, 2
    mov     edi, eax
    imul    eax, 8 ;char_w
    add     eax, 9
    mov     [ans.size], eax

 ; HISTORY
    mov     ecx, LIST_ITEM_COUNT - 1
    mov     eax, history
    add     eax, (LIST_ITEM_COUNT - 1) * 512
  @@:
    mov     ebx, eax
    sub     ebx, 512
    stdcall str_cpy, ebx, eax
    add     ebx, 480
    add     eax, 480
    stdcall str_cpy, ebx, eax
    sub     ebx, 480
    sub     eax, 480
    mov     esi, [ebx + 508]
    mov     [eax + 508], esi
    sub     eax, 512
    loop    @b

    stdcall str_cpy, exp, history
    stdcall str_cpy, ans.buffer, history + 480
    mov     esi, [ans.size]
    mov     dword[history + 508], esi

 ; Check length
    mov     esi, 37
    sub     esi, edi
    stdcall str_len, exp
    sub     esi, eax
    cmp     esi, 0
    jg	    .redraw

    mov     ebx, history
    add     ebx, eax
    add     ebx, esi
    mov     [ebx], dword ".."

    jmp     .redraw

 ; ERRORS

 .error:
    cmp     [error_n], 1
    je	    .err_1
    cmp     [error_n], 4
    je	    .err_4

    mov     [ans.buffer +  0], dword "Expe"
    mov     [ans.buffer +  4], dword "cted"
    mov     [ans.buffer +  8], dword " ')'"
    mov     [ans.buffer + 12], byte 0
    mov     [ans.size], 105

    cmp     [error_n], 2
    je	    .redraw
    cmp     [error_n], 3
    je	    .err_3
    cmp     [error_n], 5
    je	    .err_5
 .err_1:
    mov     [ans.buffer +  0], dword "Div."
    mov     [ans.buffer +  4], dword " by "
    mov     [ans.buffer +  8], byte  "0"
    mov     [ans.buffer +  9], byte 0
    mov     [ans.size], 81
    jmp     .redraw
 .err_4:
    mov     [ans.buffer +  0], dword "Inpu"
    mov     [ans.buffer +  4], dword "t er"
    mov     [ans.buffer +  8], dword "rror"
    mov     [ans.buffer + 12], byte 0
    mov     [ans.size], 105
    jmp     .redraw
 .err_3:
    mov     [ans.buffer + 10], byte "("
    jmp     .redraw
 .err_5:
    mov     [ans.buffer + 10], byte "|"
    jmp     .redraw

 .redraw:
    call    draw_textbox
    call    draw_list
    jmp     update

 ;----------------------

 proc draw_textbox
    ; border
    mcall   13, <LIST_X, LIST_ITEM_W>, <  8,  30>, [sc.work_graph]
    
    ; background
    mov     edx, [scn.gui_face]
    cmp     [error_n], 0
    je	    @f
    mov     edx, 0xFFAAAA
  @@:
    mcall   13, <  LIST_X+1, LIST_ITEM_W - 2>, <  9, 28>
    mcall     , <  LIST_X+1, LIST_ITEM_W - 2>, <  9,  1>, [scn.text_box_inner_shadow]
    mcall     , <  LIST_X+1,	1>, < 10, 27>
    mcall     , <  LIST_X, LIST_ITEM_W - 1>, < 38,  1>, [sc.work_light]

    mov      ebx, LIST_X + LIST_ITEM_W - 12
    sub      ebx, [ans.size]
    shl      ebx, 16
    add      ebx, 16
    mov      ecx, [scn.gui_intext]
    add      ecx, 0x10000000
    or       ecx, 1 shl 31
    mcall    4, , , ans.buffer

    mcall     , <LIST_ITEM_W - 8, 19>, [scn.gui_intext], btn_clr, 1
    add      ebx, 1 shl 16
    mcall

    mcall    8, <LIST_ITEM_W - 14, 17>, <9, 27>, 4 + BT_HIDE

    ; dots to fake border radius
    ; mcall    1,   4,  8, [sc.work]
    ; mcall     , LIST_ITEM_W + 3
    ; mcall     ,    , 37, [sc.work_light]
    ; mcall     ,   4

    mov     ebx, LIST_ITEM_W - 18
    sub     ebx, [ans.size]
    cmp     ebx, 24
    jg	    @f
    mov     ebx, 24
  @@:
    mov     [edb1.width], ebx
    m2m     [edb1.color],	       [scn.gui_face]
    m2m     [edb1.focus_border_color], [scn.gui_face]
    cmp     [error_n],	0
    je	    @f
    mov     [edb1.color],	       0xFFAAAA
    mov     [edb1.focus_border_color], 0xFFAAAA
  @@:
    invoke  editbox.draw, edb1

    ret
 endp
 
 ;----------------------

 proc draw_button, x, y
    mcall   8, <[x], KEYB_BTN_W>, <[y], KEYB_BTN_H>, [but_id], [but_c]

    mov     ebx, [x]
    add     ebx, KEYB_BTN_W/2-4
    ;mov     esi, [txt_size]  ;the text potentially could have valiable len
    ;imul    esi, 4
    ;mov     edi, 16
    ;add     ebx, edi
    shl     ebx, 16
    add     ebx, [y]
    add     ebx, KEYB_BTN_H/2 - 6
    or      [but_tc], 0x01000000 ;use bigger font
    mcall   4, , [but_tc], [txt_id], [txt_size]

    mov     eax, [txt_size]
    add     [txt_id], eax
    inc     dword [but_id]

    ret
 endp

 ;----------------------

if lang eq ru_RU
  engineering_str db "Engineering calc"
  engineering_len = 16
else
  engineering_str db "Инженерный режим"
  engineering_len = 16
endf

 proc draw_keyb

    mcall   8, <KEYBOARD_X, (KEYB_BTN_W + GAP)*4 - GAP>, <8, 29>, 250, [sc.work_light]
    mov     ecx, [sc.work_text]
    or      ecx, 0x10000000
    mcall   4, <(KEYB_BTN_W + GAP - engineering_len*2)*2 - GAP/2 + KEYBOARD_X, KEYB_BTN_H / 2 - 10 + 8>, , engineering_str, engineering_len

    mov     [txt_size], 1
    mov     [but_id], 0x0000000A
    mov     [txt_id], buttons

    m2m     [but_c], [sc.work]
    m2m     [but_tc], [sc.work_text]
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*0, KEYBOARD_Y
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*1, KEYBOARD_Y
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*2, KEYBOARD_Y
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*3, KEYBOARD_Y
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*3, KEYBOARD_Y + (KEYB_BTN_H + GAP)*1
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*3, KEYBOARD_Y + (KEYB_BTN_H + GAP)*2
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*3, KEYBOARD_Y + (KEYB_BTN_H + GAP)*3
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*2, KEYBOARD_Y + (KEYB_BTN_H + GAP)*4
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*1, KEYBOARD_Y + (KEYB_BTN_H + GAP)*4

    mov     [but_c], 0xF0969D
    mov     [but_tc], 0x00FFFfff
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*3, KEYBOARD_Y +(KEYB_BTN_H + GAP)*4

    m2m     [but_c], [sc.work_button]
    m2m     [but_tc], [sc.work_button_text]
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*0, KEYBOARD_Y + KEYB_BTN_H + GAP
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*1, KEYBOARD_Y + KEYB_BTN_H + GAP
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*2, KEYBOARD_Y + KEYB_BTN_H + GAP
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*0, KEYBOARD_Y + (KEYB_BTN_H + GAP)*2
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*1, KEYBOARD_Y + (KEYB_BTN_H + GAP)*2
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*2, KEYBOARD_Y + (KEYB_BTN_H + GAP)*2
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*0, KEYBOARD_Y + (KEYB_BTN_H + GAP)*3
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*1, KEYBOARD_Y + (KEYB_BTN_H + GAP)*3
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*2, KEYBOARD_Y + (KEYB_BTN_H + GAP)*3
    stdcall draw_button, KEYBOARD_X + (KEYB_BTN_W + GAP)*0, KEYBOARD_Y + (KEYB_BTN_H + GAP)*4

    ret
 endp

 ;----------------------

 proc draw_list
  ; === BACKGROUND === 
    mov     edi, LIST_ITEM_COUNT
    mov     eax, 13
    mov     ebx, LIST_X shl 16 + LIST_ITEM_W
    mov     ecx, LIST_Y shl 16 + LIST_ITEM_H
    mov     edx, [sc.work_light]
  @@:
    mcall
    ; draw separator {
    push    ecx edx
    sub     ecx, LIST_ITEM_H-1
    mov     edx, [sc.work_dark]
    mcall
    ; }
    pop     edx ecx
    add     ecx, LIST_ITEM_H shl 16
 .next:
    dec     edi
    cmp     edi, 0
    jne     @b

  ; === BUTTONS === 
    mov     edi, LIST_ITEM_COUNT
    mov     eax, 8
    mov     ebx, LIST_X shl 16 + LIST_ITEM_W
    mov     ecx, 43 shl 16 + LIST_ITEM_H
    mov     edx, 60 + BT_HIDE
  @@:
    mcall
    add     ecx, LIST_ITEM_H shl 16
    inc     edx
    dec     edi
    cmp     edi, 0
    jne     @b

  ; === TEXT === 
    mov     edi, LIST_ITEM_COUNT
    mov     eax, 4
    mov     ebx, (LIST_X + LIST_X) shl 16 + LIST_Y + LIST_TEXT_Y + 1
    mov     ecx, [sc.work_text]
  @@:
    or	    ecx, 1 shl 31
    mov     edx, history
  @@:
    add     ecx, 0x10000000
    mcall

    push    ebx
    add     ebx, LIST_ITEM_W shl 16
    mov     esi, [edx + 508]
    shl     esi, 16
    sub     ebx, esi
    add     edx, 480
    mcall
    pop     ebx

    add     edx, 32
    add     ebx, LIST_ITEM_H

    and     ecx, 0xFFFFFF
    cmp     ecx, LIST_ITEM_TEXT2
    je	    .set_color_to_2_txt
    mov     ecx, LIST_ITEM_TEXT2
    jmp     .next_txt
 .set_color_to_2_txt:
    mov     ecx, LIST_ITEM_TEXT1
 .next_txt:
    or	    ecx, 1 shl 31

    dec     edi
    cmp     edi, 0
    jne     @b

    ret
 endp

 ;----------------------

 proc str_len uses ebx, str
    xor     eax, eax
    mov     ebx, [str]
  @@:
    cmp     [ebx], byte 0
    je	    @f
    inc     eax
    inc     ebx
    jmp     @b
  @@:

    ret
 endp

 ;----------------------

 proc str_cpy uses eax ebx ecx, from, to
    mov     eax, [from]
    mov     ebx, [to]
  @@:
    cmp     [eax], byte 0
    je	    @f
    mov     cl, [eax]
    mov     [ebx], cl
    inc     eax
    inc     ebx
    jmp     @b
  @@:
    mov     [ebx], byte 0
    ret
 endp

 ;----------------------

 dataend:

 ;===============================

	    rb 2048
 stacktop:

 exp	    rb 480
 exp_pos    rd 1
 exp_lvl    rd 1
 abs_lvl    rd 1
 group	    rd 1

 ans.buffer:rb 480
 ans.size   rd 1
 error_n    rd 1

 timer	    rd 1
 but_id     rd 1
 but_c	    rd 1
 but_tc     rd 1
 txt_id     rd 1
 txt_size   rd 1

 history    rb 512 * LIST_ITEM_COUNT
	    rb 512
		
struc system_colors_internal {
  .text_box_inner_shadow        dd 0xCED0D0
  .gui_face       dd 0xFAF8FA
  .gui_text       dd 0x10373C42
  .gui_intext     dd 0x5F5F5F
  .gui_select     dd 0xC7C9C9
}

 scn system_colors_internal
 sc              system_colors
 
 f70_calc:
        dd      7
        dd      0
        dd      0
        dd      0
        dd      0
        db      '/sys/calc',0

 memory:
