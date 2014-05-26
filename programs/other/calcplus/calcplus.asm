    VERSION equ "0.5"

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

    LIST_ITEM_SIZE   equ 16
    LIST_ITEM_COUNT  equ 6
    LIST_SIZE	     equ LIST_ITEM_SIZE * LIST_ITEM_COUNT
    LIST_ITEM_COLOR1 equ dword [scn.btn_face]
    LIST_ITEM_COLOR2 equ dword [scn.win_face]
    LIST_ITEM_TEXT1  equ dword [scn.btn_text]
    LIST_ITEM_TEXT2  equ dword [scn.win_text]
    LIST_ITEM_Y      equ LIST_ITEM_SIZE / 2 - 3

    KEYB_SIZE	     equ 140

    sz_cont db "Keyboard ", 0x10
    sz_head db "Calc+ [v", VERSION, "]", 0
    btn_clr db ""
    buttons db "|%^*/-+)(=7894561230"
    edb1    edit_box 0, 8, 12, 0, 0, 0, 0, 0, 480, \
		     exp, group, ed_always_focus + ed_focus, 0, 0

 imports:
    library gui, "box_lib.obj"
    import  gui, editbox.draw,	"edit_box",	\
		 editbox.key,	"edit_box_key", \
		 editbox.mouse, "edit_box_mouse"

 ;===============================

 main:
    mov     [ans.buffer], dword "= 0"
    mov     [ans.size], 3 * 6 + 9

    mcall   40, 100111b
    mcall   48, 3, scn, 192


    m2m     [edb1.color],	       [scn.gui_face]
    m2m     [edb1.shift_color],        [scn.gui_select]
    m2m     [edb1.focus_border_color], [scn.gui_face]
    m2m     [edb1.text_color],	       [scn.gui_text]

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

 ; WINDOW
    mov     edx, [scn.win_face]
    or	    edx, 0x34 shl 24
    mcall   0, <100, 236 + 100 - 50 - 25>, <100, 66 + LIST_SIZE>, , , sz_head

 ; TOOLBAR
    mov     ebx, (275 - 50 - 25 - 50) shl 16 + 64
    cmp     [keyb], byte 1
    jne     @f
    add     ebx, KEYB_SIZE shl 16
  @@:
    mcall   8, , <-17, 12>, 2 + 1 shl 30
    add     ebx, 4 shl 16 - (64 + 14)
    mcall   4, , [scn.win_title], sz_cont, 10

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

 ; DELETE
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

 ; SHOW/HIDE KEYBOARD
    cmp     ah, 2
    jne     .not_keyb

    cmp     [keyb], byte 0
    je	    .open

    mcall   67, -1, -1, 286 - 25, -1
    mov     [keyb], 0
    mov     [sz_cont + 9], byte 0x10
    jmp     ev_redraw

 .open:
    mcall   67, -1, -1, 286 - 25 + KEYB_SIZE, -1
    mov     [keyb], 1
    mov     [sz_cont + 9], byte 0x11
    jmp     ev_redraw

    jmp     update
 .not_keyb:

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
    imul    eax, 6
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
    not     [his_even]

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
    mov     [ans.size], 81

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
    mov     [ans.size], 63
    jmp     .redraw
 .err_4:
    mov     [ans.buffer +  0], dword "Inpu"
    mov     [ans.buffer +  4], dword "t er"
    mov     [ans.buffer +  8], dword "rror"
    mov     [ans.buffer + 12], byte 0
    mov     [ans.size], 81
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

 proc draw_button, x, y
    mcall   8, <[x], 30>, <[y], 21>, [but_id], [but_c]

    mov     ebx, [x]
    mov     esi, [txt_size]
    imul    esi, 3
    mov     edi, 16
    sub     edi, esi
    add     ebx, edi
    shl     ebx, 16
    add     ebx, [y]
    add     ebx, 7
    mcall   4, , [but_tc], [txt_id], [txt_size]

    mov     eax, [txt_size]
    add     [txt_id], eax
    inc     dword [but_id]

    ret
 endp

 ;----------------------

 proc draw_textbox
    mcall   13, <4, 320 - 50 - 25>, <  8,  23>, [scn.gui_frame]
    mov     edx, [scn.gui_face]
    cmp     [error_n], 0
    je	    @f
    mov     edx, 0xFFAAAA
  @@:
    mcall   13, <  5, 318 - 50 - 25>, <  9, 21>
    mcall     , <  5, 318 - 50 - 25>, <  9,  1>, [scn.3d_face]
    mcall     , <  5,	1>, < 10, 20>
    mcall     , <  5, 318 - 50 - 25>, < 31,  1>, [scn.3d_light]

    mov      ebx, 328 - 16 - 50 - 25
    sub      ebx, [ans.size]
    shl      ebx, 16
    add      ebx, 16
    mov      ecx, [scn.gui_intext]
    or	     ecx, 1 shl 31
    mcall    4, , , ans.buffer

    mov      ecx, [scn.gui_text]
    mcall     , <310 - 50 - 25, 16>, , btn_clr, 1
    add      ebx, 1 shl 16
    mcall

    mcall    8, <305 - 50 - 25, 17>, <9, 20>, 0x40000004

    mcall    1,   4,  8, [scn.win_body]
    mcall     , 323 - 50 - 25
    mcall     ,    , 30, [scn.3d_light]
    mcall     ,   4

    mov     ebx, 318 - 16 - 50 - 25
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

 proc draw_keyb
    cmp     [keyb], byte 0
    je	    @f

    mov     [txt_size], 1
    mov     [but_id], 0x0000000A
    mov     [txt_id], buttons

    mov     eax, [scn.win_face]
    mov     [but_c], eax
    mov     eax, [scn.win_text]
    mov     [but_tc], eax
    stdcall draw_button,   4 + 278 - 25,  42 - 25 - 8
    stdcall draw_button,  37 + 278 - 25,  42 - 25 - 8
    stdcall draw_button,  70 + 278 - 25,  42 - 25 - 8
    stdcall draw_button, 103 + 278 - 25,  42 - 25 - 8
    stdcall draw_button, 103 + 278 - 25,  66 - 25 - 8
    stdcall draw_button, 103 + 278 - 25,  90 - 25 - 8
    stdcall draw_button, 103 + 278 - 25, 114 - 25 - 8
    stdcall draw_button,  70 + 278 - 25, 139 - 25 - 8
    stdcall draw_button,  37 + 278 - 25, 139 - 25 - 8

    mov     eax, [scn.btn_inface]
    mov     [but_c], eax
    mov     eax, [scn.btn_intext]
    mov     [but_tc], eax
    stdcall draw_button, 103 + 278 - 25, 139 - 25 - 8

    mov     eax, [scn.btn_face]
    mov     [but_c], eax
    mov     eax, [scn.btn_text]
    mov     [but_tc], eax
    stdcall draw_button,   4 + 278 - 25,  66 - 25 - 8
    stdcall draw_button,  37 + 278 - 25,  66 - 25 - 8
    stdcall draw_button,  70 + 278 - 25,  66 - 25 - 8
    stdcall draw_button,   4 + 278 - 25,  90 - 25 - 8
    stdcall draw_button,  37 + 278 - 25,  90 - 25 - 8
    stdcall draw_button,  70 + 278 - 25,  90 - 25 - 8
    stdcall draw_button,   4 + 278 - 25, 114 - 25 - 8
    stdcall draw_button,  37 + 278 - 25, 114 - 25 - 8
    stdcall draw_button,  70 + 278 - 25, 114 - 25 - 8
    stdcall draw_button,   4 + 278 - 25, 139 - 25 - 8

  @@:
    ret
 endp

 ;----------------------

 proc draw_list
  ; BACKGROUND
    mov     edi, LIST_ITEM_COUNT
    mov     eax, 13
    mov     ebx, 4 shl 16 + 320 - 50 - 25
    mov     ecx, 37 shl 16 + LIST_ITEM_SIZE
    mov     edx, LIST_ITEM_COLOR1
    cmp     [his_even], byte 0
    je	    @f
    mov     edx, LIST_ITEM_COLOR2
  @@:
    mcall
    add     ecx, LIST_ITEM_SIZE shl 16
    cmp     edx, LIST_ITEM_COLOR1
    je	    .set_color_to_2
    mov     edx, LIST_ITEM_COLOR1
    jmp     .next
 .set_color_to_2:
    mov     edx, LIST_ITEM_COLOR2
 .next:
    dec     edi
    cmp     edi, 0
    jne     @b

  ; BUTTONS
    mov     edi, LIST_ITEM_COUNT
    mov     eax, 8
    mov     ebx, 4 shl 16 + 320 - 50 - 25
    mov     ecx, 37 shl 16 + LIST_ITEM_SIZE
    mov     edx, 0x40000060
  @@:
    mcall
    add     ecx, LIST_ITEM_SIZE shl 16
    inc     edx
    dec     edi
    cmp     edi, 0
    jne     @b

  ; TEXT
    mov     edi, LIST_ITEM_COUNT
    mov     eax, 4
    mov     ebx, 8 shl 16 + 37 + LIST_ITEM_Y
    mov     ecx, LIST_ITEM_TEXT1
    cmp     [his_even], byte 0
    je	    @f
    mov     ecx, LIST_ITEM_TEXT2
  @@:
    or	    ecx, 1 shl 31
    mov     edx, history
  @@:
    mcall

    push    ebx
    add     ebx, (320 - 50 - 25) shl 16
    mov     esi, [edx + 508]
    shl     esi, 16
    sub     ebx, esi
    add     edx, 480
    mcall
    pop     ebx

    add     edx, 32
    add     ebx, LIST_ITEM_SIZE

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

 scn	    sys_colors_new
 timer	    rd 1
 but_id     rd 1
 but_c	    rd 1
 but_tc     rd 1
 txt_id     rd 1
 txt_size   rd 1
 keyb	    rb 1

 his_even   rb 1
 history    rb 512 * LIST_ITEM_COUNT
	    rb 512

 memory:
