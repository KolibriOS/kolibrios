    use32
    org     0
    db	    'MENUET01'
    dd	    1, @entry, @end, @memory, @stack, @params, 0

    include "../../macros.inc"
    include "../../proc32.inc"
    include "../../dll.inc"

 macro cmpe a, b, c  {
    cmp     a, b
    je	    c	     }

 macro cmpl a, b, c  {
    cmp     a, b
    jl	    c	     }

 macro cmpne a, b, c {
    cmp     a, b
    jne     c	     }

 macro cmple a, b, c {
    cmp     a, b
    jle     c	     }

 macro cmpge a, b, c {
    cmp     a, b
    jge     c	     }

 LINEH	    equ 12
 ICONS	    equ 11

;=====================================================================

 @entry:

    mcall   40, 101b

 ;----------------------------

  ;; CHECK FOR PARAMS
    cmpne   [@params], byte 0, parse
    mov     eax, @params
    mov     ebx, sz_std
  @@:
    mov     cl, [ebx]
    mov     [eax], cl
    inc     eax
    inc     ebx
    cmpne   [ebx - 1], byte 0, @b

  ;; TEXT

 parse:
    mov     [text.lines], dword 1
    mov     [text.max_len], dword 1

    mov     eax, @params
    mov     ebx, text.buffer
    mov     edx, 0
    mov     esi, 0

    cmpne   [eax], byte "'", @f
    mov     dl, "'"
    mov     eax, @params + 1
    jmp     .text

  @@:
    cmpne   [eax], byte '"', .text
    mov     dl, '"'
    mov     eax, @params + 1

  .text:
    cmpe    [eax], dl, .text.end
    cmpe    [eax], byte 0, .text.end
    mov     cl, [eax]

    cmpe    cl, "\", .char
    cmpne   cl, 10, .copy
    cmple   esi, dword [text.max_len], @f
    mov     [text.max_len], esi
  @@:
    mov     esi, 0
    mov     cl, 0
    inc     dword [text.lines]
    jmp     .copy

  .char:
    cmpe    [eax + 1], byte "n", .newline
    cmpe    [eax + 1], dl, .quote
    jmp     .copy

  .newline:
    cmple   esi, dword [text.max_len], @f
    mov     [text.max_len], esi
  @@:
    mov     esi, 0
    mov     cl, 0
    inc     dword [text.lines]
    inc     eax
    jmp     .copy

  .quote:
    mov     cl, dl
    inc     eax

  .copy:
    mov     [ebx], cl
    inc     eax
    inc     ebx
    inc     esi
    jmp     .text
  .text.end:

    cmple   esi, dword [text.max_len], @f
    mov     [text.max_len], esi
  @@:

    mov     [ebx], byte 0

    cmpge   [text.max_len], dword 25, @f
    mov     [text.max_len], dword 25
  @@:

 ;; PARAMS

  .params:
    cmpe    [eax], byte 0, .params.end
    cmpe    [eax], byte "d", .set_atcl
    cmpe    [eax], byte "t", .set_title
    mov     ebx, 1
    cmpe    [eax], byte "A", .set_icon
    inc     ebx
    cmpe    [eax], byte "E", .set_icon
    inc     ebx
    cmpe    [eax], byte "W", .set_icon
    inc     ebx
    cmpe    [eax], byte "O", .set_icon
    inc     ebx
    cmpe    [eax], byte "N", .set_icon
    inc     ebx
    cmpe    [eax], byte "I", .set_icon
    inc     ebx
    cmpe    [eax], byte "F", .set_icon
    inc     ebx
    cmpe    [eax], byte "C", .set_icon
    inc     ebx
    cmpe    [eax], byte "M", .set_icon
    inc     ebx
    cmpe    [eax], byte "D", .set_icon
    inc     ebx
    cmpe    [eax], byte "S", .set_icon

    jmp     .next_char

  .set_atcl:
    mov     [params.atcl], byte 1
    jmp     .next_char

  .set_title:
    mov     [params.title], byte 1
    jmp     .next_char

  .set_icon:
    mov     [params.icon], ebx

  .next_char:
    inc     eax
    jmp     .params

  .params.end:

 ;----------------------------

    mcall   68, 11
    stdcall dll.Load, @imports

    mov     dword [fi + 00], 5
    mov     dword [fi + 16], buffer
    mov     dword [fi + 21], sz_ifile
    mcall   70, fi

    mov     edx, dword [buffer + 32]
    shl     edx, 2
    stdcall mem.Alloc, edx
    mov     [img_data.rgb_obj], eax

    mov     dword [fi + 00], 0
    mov     dword [fi + 12], edx
    m2m     dword [fi + 16], [img_data.rgb_obj]
    mov     dword [fi + 21], sz_ifile
    mcall   70, fi

    stdcall dword [img.decode], dword [img_data.rgb_obj], ebx, 0
    mov     dword [img_data.obj], eax

 ;; alpha
    add     eax, 24
    mov     eax, [eax] ;; eax - data [argb]

    mov     ecx, 24 * 24 * ICONS
 alpha:
    mov     ebx, [eax]
    shr     ebx, 24
    cmpne   bl, 0x00, @f
    mov     [eax], dword 0x222222

  @@:
    add     eax, 4
    loop    alpha

 ;; end alpha

    stdcall dword [img.to_rgb], dword [img_data.obj], dword [img_data.rgb_obj]
    stdcall dword [img.destroy], dword [img_data.obj]

 ;----------------------------

    mov     [text.offset], LINEH

    mov     eax, [text.lines]
    add     eax, 2
    imul    eax, LINEH
    mov     [window.height], eax

    mov     eax, [text.max_len]
    imul    eax, 6
    add     eax, LINEH * 2
    cmpe    [params.icon], dword 0, @f
    add     eax, 24 + LINEH
    add     [text.offset], 24 + LINEH
  @@:
    mov     [window.width], eax

    mcall   14
    mov     ebx, eax
    and     ebx, 0xFFFF
    mov     [scr.height], ebx
    shr     eax, 16
    mov     [scr.width], eax
    sub     eax, [window.width]
    sub     eax, LINEH
    mov     [window.x], eax

 ;; CALC WINDOW.Y

    mcall   68, 22, sz_shname, 256 + 512, 4 + 1 ;OPEN_ALWAYS and WRITE
    add     eax, 512
    mov     [shm], eax

 s_search:
    mov     eax, [shm]

    mov     ebx, 0
    mov     ecx, [text.lines]
    add     ecx, 3

    push    eax ebx

    mov     eax, [scr.height]
    mov     edx, 0
    mov     ebx, LINEH
    div     ebx
    mov     edx, eax
    add     edx, [shm]
    sub     edx, ecx
    inc     edx

    pop     ebx eax

 s_area:
    cmpe    [eax], byte 1, .is_1

  .is_0:
    inc     ebx
    cmpe    ebx, ecx, s_ok
    jmp     .next

  .is_1:
    mov     ebx, 0

  .next:
    inc     eax
    cmple   eax, edx, s_area

    mcall   5, 10
    jmp     s_search

 s_ok:
    sub     eax, ecx
    inc     eax
    mov     [shm.our], eax

    mov     edx, eax
    sub     edx, [shm]
    inc     edx
    imul    edx, LINEH
    mov     [window.y], edx

  @@:
    mov     [eax], byte 1
    inc     eax
    loop    @b

 ;----------------------------

    mov     eax, 60
    imul    eax, [text.lines]
    mov     [timer], eax

    mov     [timer.step], dword 1
    cmpne   [params.atcl], byte 1, @f
    mov     [timer.step], dword 0
  @@:

 ;----------------------------

 update:
    mcall   23, 10
    cmpe    al, EV_REDRAW, redraw
    cmpe    al, EV_BUTTON, exit

    mov     eax, [timer.step]
    sub     [timer], eax
    cmpne   [timer], dword 0, update

 ;----------------------------

 exit:
    mov     eax, [shm.our]
    mov     ecx, [text.lines]
    add     ecx, 3
  @@:
    mov     [eax], byte 0
    inc     eax
    loop    @b

    mcall   68, 23, sz_shname

    mcall   -1

 ;----------------------------

 redraw:
    call    draw_window
    call    draw_text

    jmp     update

 ;----------------------------

 draw_window:
    dec     dword [window.width]
    dec     dword [window.height]
    mcall   0, <[window.x], [window.width]>, <[window.y], [window.height]>, 0x61000000
    inc     dword [window.width]
    inc     dword [window.height]

    and     ebx, 0xFFFF
    and     ecx, 0xFFFF
    inc     ebx
    inc     ecx
    mcall   8, , , 0x61000001

    mov     eax, 13
    mov     ebx, [window.width]
    mov     edx, 0x222222
    cmpe    [first_draw], byte 1, .draw_full
    mov     [first_draw], byte 1
    mov     esi, [window.height]
    mov     ecx, LINEH
  @@:
    mcall
    add     ecx, LINEH shl 16
    sub     esi, LINEH
    push    eax ebx
    mcall   5, 1
    pop     ebx eax
    cmpne   esi, 0, @b
  .draw_full:
    mcall

    mcall   , , 1, 0x121212

    mov     ecx, [window.height]
    dec     ecx
    shl     ecx, 16
    inc     ecx
    mcall

    mcall   , 1, [window.width]

    mov     ebx, [window.width]
    dec     ebx
    shl     ebx, 16
    inc     ebx
    mcall

    mcall   1, 1, 1
    mov     ebx, [window.width]
    sub     ebx, 2
    mcall
    mov     ecx, [window.height]
    sub     ecx, 2
    mcall
    mov     ebx, 1
    mcall

 ;-----

    mov     ecx, [scr.width]
    inc     ecx

    mov     eax, 35
    mov     ebx, ecx
    imul    ebx, [window.y]
    add     ebx, [window.x]
    dec     ebx
    mcall
    push    eax

    mov     eax, 35
    add     ebx, [window.width]
    add     ebx, 2
    mcall
    push    eax

    mov     eax, 35
    mov     edx, ecx
    mov     esi, [window.height]
    dec     esi
    imul    edx, esi
    add     ebx, edx
    mcall
    push    eax

    mov     eax, 35
    sub     ebx, [window.width]
    sub     ebx, 2
    mcall
    push    eax

 ;-----

    mov     eax, 1

    pop     edx
    mov     ecx, [window.height]
    dec     ecx
    mcall   , 0

    pop     edx
    mov     ebx, [window.width]
    dec     ebx
    mcall

    pop     edx
    mcall   , , 0

    pop     edx
    mcall   , 0

 ;-----

    cmpe    [params.icon], dword 0, @f

    mov     ebx, [params.icon]
    dec     ebx
    imul    ebx, 24 * 24 * 3
    add     ebx, [img_data.rgb_obj]

    mov     edx, [window.height]
    shr     edx, 1
    sub     edx, 12
    add     edx, LINEH shl 16

    mcall   7, , <24, 24>

  @@:

    ret

 ;----------------------------

 draw_text:
    mov     esi, [text.lines]

    mov     eax, 4
    mov     ebx, [text.offset]
    shl     ebx, 16
    add     ebx, LINEH + (LINEH - 6) / 2
    mov     edx, text.buffer

  .draw_lines:
    mov     ecx, 0x80111111

    add     ebx, 0x00010000
    dec     ebx
    mcall
    add     ebx, 2
    mcall
    sub     ebx, 0x00020000
    mcall
    sub     ebx, 2
    mcall

    add     ebx, 0x00010001
    mov     ecx, 0x80D0D0D0
    mcall

    add     ebx, LINEH
    dec     esi
    cmpe    esi, 0, .draw_lines.end

    inc     edx
  @@:
    cmpe    [edx], byte 0, @f
    inc     edx
    jmp     @b
  @@:
    inc     edx
    jmp     .draw_lines

  .draw_lines.end:

    cmpne   [params.title], byte 1, @f
    mov     edx, text.buffer
    mov     ecx, 0x80111111
    and     ebx, 0xFFFF0000
    add     ebx, 1 shl 16 + LINEH + (LINEH - 6) / 2
    mcall

    mov     ecx, 0x80FFFFFF
    sub     ebx, 0x00010000
    mcall
  @@:

    ret
 ;----------------------------

 @imports:
    library img, "libimg.obj"
    import  img, img.init,    "lib_init",     \
		 img.to_rgb,  "img_to_rgb2",  \
		 img.decode,  "img_decode",   \
		 img.destroy, "img_destroy"

 ;----------------------------

 sz_ifile   db "/sys/notify3.png", 0
 sz_shname  db "notify-mem-v01", 0
 sz_std     db "'NOTIFY 3\n",                 \
		"d - disable auto-closing\n", \
		"t - title\n",		      \
		"A - application\n",	      \
		"E - error\n",		      \
		"W - warning\n",	      \
		"O - ok\n",		      \
		"N - network\n",	      \
		"I - info\n",		      \
		"F - folder\n", 	      \
		"C - component\n",	      \
		"M - mail\n",		      \
		"D - download\n",	      \
		"P - audio player",	      \
		"' -td", 0

 ;----------------------------

 @end:

;=====================================================================

 window:
  .x	    rd 1
  .y	    rd 1
  .width    rd 1
  .height   rd 1

 scr:
  .width    rd 1
  .height   rd 1

 text:
  .buffer   rb 256
  .lines    rd 1
  .max_len  rd 1
  .offset   rd 1

 params:
  .atcl     rb 1
  .title    rb 1
  .icon     rd 1

 img_data:
  .rgb_obj  rd 1
  .obj	    rd 1

 timer:
  .value    rd 1
  .step     rd 1

 shm:
  .addr     rd 1
  .our	    rd 1

 fi	    rb 26
 buffer     rb 1024
 first_draw rb 1

;=====================================================================
	    rb 2048
 @stack:
 @params    rb 256

 @memory:
