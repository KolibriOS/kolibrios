    VERSION equ "0.1Å"

    use32
    org     0
    db	    'MENUET01'
    dd	    1, main, dataend, memory, stacktop, 0, 0

    include "../../proc32.inc"
    include "../../macros.inc"
    include "../../dll.inc"
    include "../../develop/libraries/box_lib/trunk/box_lib.mac"
    include "parser.inc"

 ;===============================

    sz_head db "Calc+ [v", VERSION, "]", 0
    buttons db "1234567890.%-+*/()"
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
    mcall   23, 50

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
    add     ebx, 1 shl 16
    add     ecx, 1 shl 16
    dec     ebx
    dec     ecx
    mcall     , 	  ,	    , [scn.win_face]
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

    mov     [but_id], 0x4000000A
    stdcall draw_button,   4, 38,  42, buttons + 00, 15
    stdcall draw_button,  46, 38,  42, buttons + 01, 15
    stdcall draw_button,  88, 38,  42, buttons + 02, 15
    stdcall draw_button,   4, 38,  66, buttons + 03, 15
    stdcall draw_button,  46, 38,  66, buttons + 04, 15
    stdcall draw_button,  88, 38,  66, buttons + 05, 15
    stdcall draw_button,   4, 38,  90, buttons + 06, 15
    stdcall draw_button,  46, 38,  90, buttons + 07, 15
    stdcall draw_button,  88, 38,  90, buttons + 08, 15
    stdcall draw_button,   4, 80, 114, buttons + 09, 35
    stdcall draw_button,  88, 38, 114, buttons + 10, 15

    stdcall draw_button, 144, 38,  42, buttons + 11, 15
    stdcall draw_button, 186, 38,  42, buttons + 12, 15
    stdcall draw_button, 144, 38,  66, buttons + 13, 15
    stdcall draw_button, 186, 38,  66, buttons + 14, 15
    stdcall draw_button, 144, 38,  90, buttons + 15, 15
    stdcall draw_button, 186, 38,  90, buttons + 16, 15
    stdcall draw_button, 144, 38, 114, buttons + 17, 15
    stdcall draw_button, 186, 38, 114, buttons + 18, 15

    jmp     update

 ;----------------------

 ev_key:
    mov     [timer], 1

    mcall   2
    cmp     ah, 27
    je	    exit
    invoke  editbox.key, edb1
    jmp     update

 ;----------------------

 ev_button:
    mov     [timer], 1

    mcall   17

    cmp     ah, 1
    je	    exit

    cmp     ah, 22
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
    jmp     ev_redraw

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

    jmp     ev_redraw

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
    jmp     ev_redraw

 .error:
    cmp     [error_n], 1
    je	    .err_1
    jmp     ev_redraw
 .err_1:
    mov     [ans.buffer +  0], dword "Div."
    mov     [ans.buffer +  4], dword " by "
    mov     [ans.buffer +  8], dword "zero"
    mov     [ans.buffer + 12], byte 0
    mov     [ans.size], 81
    jmp     ev_redraw

 ;----------------------

 proc draw_button, x, w, y, txt, txtx
    mcall   13, <[x], [w]>, <[y], 20>, [scn.gui_frame]

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

    add     ebx, [w]
    dec     ebx
    mcall

    add     ecx, 20
    dec     ecx
    mcall

    sub     ebx, [w]
    inc     ebx
    mcall

    mov     ebx, [x]
    inc     ebx
    shl     ebx, 16
    add     ebx, [w]
    sub     ebx, 3
    mov     ecx, [y]
    inc     ecx
    shl     ecx, 16
    add     ecx, 20
    sub     ecx, 3
    mov     edx, [but_id]
    mcall   8
    inc     dword[but_id]

    mov     ebx, [x]
    add     ebx, [txtx]
    shl     ebx, 16
    add     ebx, [y]
    add     ebx, 7
    mcall   4, , [scn.win_text], [txt], 1

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

 memory:
