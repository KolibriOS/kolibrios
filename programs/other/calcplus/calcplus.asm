    VERSION equ "0.3Å"
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

    sz_head db "Calc+ [v", VERSION, "]", 0
    buttons db "123456789()%^-+/*0"
    edb1    edit_box 184, 8, 12, 0, 0, 0, 0, 0, 500, \
		     exp, group, ed_always_focus + ed_focus, 0, 0

 imports:
    library gui, "box_lib.obj"
    import  gui, editbox.draw,	"edit_box",	\
		 editbox.key,	"edit_box_key", \
		 editbox.mouse, "edit_box_mouse"

 ;===============================

 main:
    mov     [ans.buffer], word "0"
    mov     [ans.size], 15

    mcall   40, 100111b
    mcall   48, 3, scn, 192

    m2m     [edb1.color],	       [scn.gui_face]
    m2m     [edb1.shift_color],        [scn.gui_select]
    m2m     [edb1.focus_border_color], [scn.gui_face]
    m2m     [edb1.text_color],	       [scn.gui_fctext]

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

    dec     [timer]
    cmp     [timer], 0
    je	    calc

    jmp     update

 ;----------------------

 ev_redraw:
    mov     edx, [scn.win_body]
    or	    edx, 0x34 shl 24
    mcall    0, <100, 236>, <100, 164>, , , sz_head

    call    draw_textbox

    mov     [but_id], 0x4000000A
    mov     [txt_id], buttons
    mov     [txt_x], 16
    mov     [but_w], 38
    stdcall draw_button,   4,  42
    stdcall draw_button,  46,  42
    stdcall draw_button,  88,  42
    stdcall draw_button,   4,  66
    stdcall draw_button,  46,  66
    stdcall draw_button,  88,  66
    stdcall draw_button,   4,  90
    stdcall draw_button,  46,  90
    stdcall draw_button,  88,  90
    stdcall draw_button,  88, 114

    stdcall draw_button, 144,  42
    stdcall draw_button, 186,  42
    stdcall draw_button, 144,  66
    stdcall draw_button, 186,  66
    stdcall draw_button, 144,  90
    stdcall draw_button, 186,  90
    stdcall draw_button, 144, 114
    stdcall draw_button, 186, 114

    mov     [txt_x], 37
    mov     [but_w], 80
    stdcall draw_button,   4, 114

    jmp     update

 ;----------------------

 ev_key:
    mov     [timer], 10

    mcall   2
    cmp     ah, 27
    je	    exit
    invoke  editbox.key, edb1
    jmp     update

 ;----------------------

 ev_button:
    mov     [timer], 10

    mcall   17

    cmp     ah, 1
    je	    exit

    cmp     ah, 2
    jne     .not_copy

 .copy:

    mov     eax, exp
    mov     ebx, ans.buffer
    mov     ecx, 0
  @@:
    mov     dl, [ebx]
    mov     [eax], dl
    inc     eax
    inc     ebx
    inc     ecx
    cmp     [eax - 1], byte 0
    jne     @b

    dec     ecx
    mov     [edb1.size], ecx
    m2m     [edb1.pos], [edb1.size]

    jmp     .redraw

 .not_copy:

    cmp     ah, 19
    jne     .not_del

 .del:

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

    cmp     ah, 10
    jl	    update
    cmp     ah, 50
    jg	    update

    movzx   eax, ah
    add     eax, buttons
    sub     eax, 10
    mov     al, [eax]

    mov     ebx, exp
    add     ebx, [edb1.size]
    mov     ecx, exp
    add     ecx, [edb1.pos]
  @@:
    cmp     ebx, ecx
    je	    @f
    mov     dl, [ebx - 1]
    mov     [ebx], dl
    dec     ebx
    jmp     @b
  @@:

    mov     [ebx], al
    inc     [edb1.size]
    inc     [edb1.pos]

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

    stdcall convert_to_str, eax, ans.buffer
    imul    eax, 6
    add     eax, 9
    mov     [ans.size], eax
    jmp     .redraw

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

 .redraw:
    call    draw_textbox
    jmp     update

 ;----------------------

 proc draw_button, x, y
    mcall   13, <[x], [but_w]>, <[y], 20>, [scn.gui_frame]

    add     ebx, 1 shl 16
    add     ecx, 1 shl 16
    sub     ebx, 2
    sub     ecx, 2
    mcall   , , , [scn.3d_light]

    add     ebx, 1 shl 16
    add     ecx, 1 shl 16
    dec     ebx
    dec     ecx
    mcall   , , , [scn.3d_dark]

    dec     ebx
    dec     ecx
    mcall   , , , [scn.win_face]

    mcall   1, [x], [y], [scn.win_body]

    add     ebx, [but_w]
    dec     ebx
    mcall

    add     ecx, 20
    dec     ecx
    mcall

    sub     ebx, [but_w]
    inc     ebx
    mcall

    mov     ebx, [x]
    inc     ebx
    shl     ebx, 16
    add     ebx, [but_w]
    sub     ebx, 3
    mov     ecx, [y]
    inc     ecx
    shl     ecx, 16
    add     ecx, 20
    sub     ecx, 3
    mov     edx, [but_id]
    mcall   8

    mov     ebx, [x]
    add     ebx, [txt_x]
    shl     ebx, 16
    add     ebx, [y]
    add     ebx, 7
    mcall   4, , [scn.win_text], [txt_id], 1

    inc     dword [txt_id]
    inc     dword [but_id]

    ret
 endp

 ;----------------------

 proc draw_textbox
    mcall   13, <  4, 220>, <  8,  23>, [scn.gui_frame]
    mov     edx, [scn.gui_face]
    cmp     [error_n], 0
    je	    @f
    mov     edx, 0xFFAAAA
  @@:
    mcall   13, <  5, 218>, <  9, 21>
    mcall     , <  5, 218>, <  9,  1>, [scn.3d_face]
    mcall     , <  5,	1>, < 10, 20>
    mcall     , <  5, 218>, < 31,  1>, [scn.3d_light]

    mov     ebx, 224
    sub     ebx, [ans.size]
    shl     ebx, 16
    inc     ebx
    mcall     , 	  , < 9, 21>, [scn.gui_frame]
    add     ebx, [ans.size]
    add     ebx, 1 shl 16
    sub     ebx, 3
    mcall     , 	  ,	    , [scn.3d_light]
    cmp     [error_n], 0
    jne     .btn_not
    mcall   8, , , 0x40000002
 .btn_not:
    add     ebx, 1 shl 16
    add     ecx, 1 shl 16
    dec     ebx
    dec     ecx
    mcall   13, 	  ,	    , [scn.win_face]
    shr     ecx, 16
    mov     bx, cx
    add     ebx, 3 shl 16 + 6
    mov     ecx, [scn.win_text]
    or	    ecx, 1b shl 31
    mcall   4, , , ans.buffer

    mcall    1,   4,  8, [scn.win_body]
    mcall     , 223
    mcall     ,    , 30, [scn.3d_light]
    mcall     ,   4

    mov     ebx, 214
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

 dataend:

 ;===============================

	    rb 2048
 stacktop:

 exp	    rb 512
 exp_pos    rd 1
 exp_lvl    rd 1
 group	    rd 1

 ans.buffer:rb 512
 ans.size   rd 1
 error_n    rd 1

 scn	    sys_colors_new
 timer	    rd 1
 but_id     rd 1
 but_w	    rd 1
 txt_id     rd 1
 txt_x	    rd 1

 memory:
