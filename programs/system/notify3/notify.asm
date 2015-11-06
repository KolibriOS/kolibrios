    use32
    org     0
    db	    'MENUET01'
    dd	    1, @entry, @end, @memory, @stack, @params, 0

    include "../../macros.inc"
    include "../../proc32.inc"
    include "../../dll.inc"
;    include "../../debug.inc"

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

  ;; GET PID OF ACTIVE

    mcall   18, 7

    mov     ecx, eax
    mcall   9, buffer

    m2m     dword[prev_pid], dword[buffer + 30]

  ;; ALWAYS TOP

    mcall   18, 25, 2, -1, 1

  ;; CHECK FOR PARAMS

    mov     eax, @params
    cmpne   [@params], byte 0, @f
    mov     eax, sz_std
  @@:

  ;; TEXT

 parse:
    call    parse_text

 ;; PARAMS

  .params:
    mov     dl, [eax]

    cmpe    dl, 0, .params.end
    cmpe    dl, "d", .set_atcl
    cmpe    dl, "t", .set_title
    cmpe    dl, "c", .set_ctrl

    mov     ebx, 1
    mov     ecx, sz_icons
  @@:
    cmpe    dl, [ecx], .set_icon
    inc     ebx
    inc     ecx
    cmpne   [ecx], byte 0, @b

    jmp     .next_char

  .set_atcl:
    mov     [params.atcl], byte 1
    jmp     .next_char

  .set_title:
    mov     [params.title], byte 1
    jmp     .next_char

  .set_ctrl:
    mov     [params.ctrl], byte 1
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

    push    eax ebx
    mcall   48, 5
    shr     ebx, 16
    add     [window.y], ebx
    pop     ebx eax

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

    call    init_window

    mcall   18, 21, [prev_pid]
    mov     ecx, eax
    mcall   18, 3

    jmp     redraw

 ;----------------------------

 update:
    mcall   23, 10
    cmpe    al, EV_REDRAW, redraw
    cmpe    al, EV_KEY, key
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

 key:
    mcall   2
    cmpne   ah, 27, update
    jmp     exit

 ;----------------------------

 redraw:
    call    draw_window
    call    draw_text

    jmp     update

 ;----------------------------

 draw_window:
    call    init_window

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

 init_window:
    dec     dword [window.width]
    dec     dword [window.height]
    mcall   0, <[window.x], [window.width]>, <[window.y], [window.height]>, 0x61000000
    inc     dword [window.width]
    inc     dword [window.height]

    ret

 ;----------------------------

 parse_text:
    mov     dword [text.max_len], 0
    mov     dword [text.lines], 1
    mov     ebx, text.buffer
    mov     ecx, 0
    mov     dl, 0
    mov     dh, 0

    cmpne   byte [eax], "'", @f
    mov     dl, "'"
    mov     dh, 1
  @@:
    cmpne   byte [eax], '"', @f
    mov     dl, '"'
    mov     dh, 1
  @@:
    cmpne   dh, 1, @f
    inc     eax
  @@:

  .parse_loop:
    cmpe    byte [eax],  0, .parse_loop.end
    cmpe    byte [eax], dl, .parse_loop.end
    mov     dh, [eax]

    cmpe    byte [eax], 10, .newline

    cmpne   byte [eax + 0], "\", @f
    cmpne   byte [eax + 1], "n", @f
    inc     eax

  .newline:
    mov     byte [ebx], 0
    cmple   ecx, dword [text.max_len], .skip_max_len
    mov     dword [text.max_len], ecx
  .skip_max_len:
    mov     ecx, 0
    inc     dword [text.lines]
    jmp     .next
  @@:

    mov     [ebx], dh

  .next:
    inc     eax
    inc     ebx
    inc     ecx
    jmp     .parse_loop
  .parse_loop.end:

    cmple   ecx, dword [text.max_len], @f
    mov     dword [text.max_len], ecx
  @@:

    cmpge   [text.max_len], dword 25, @f
    mov     [text.max_len], dword 25
  @@:

    mov     [ebx], byte 0

    ret

 ;----------------------------

 @imports:
    library img, "libimg.obj"
    import  img, img.init,    "lib_init",     \
		 img.to_rgb,  "img_to_rgb2",  \
		 img.decode,  "img_decode",   \
		 img.destroy, "img_destroy"

 ;----------------------------

 sz_icons   db "AEWONIFCMDS", 0
 sz_ifile   db "/sys/notify3.png", 0
 sz_shname  db "notify-mem-v01", 0
 sz_std     db "'NOTIFY 3\n",                \
	       "d - disable auto-closing\n", \
	       "t - title\n",		     \
	       " \n",			     \
	       "ICONS:\n",		     \
	       "A - application\n",	     \
	       "E - error\n",		     \
	       "W - warning\n", 	     \
	       "O - ok\n",		     \
	       "N - network\n", 	     \
	       "I - info\n",		     \
	       "F - folder\n",		     \
	       "C - component\n",	     \
	       "M - mail\n",		     \
	       "D - download\n",	     \
	       "S - audio player",	     \
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
  .buffer   rb 2048
  .lines    rd 1
  .max_len  rd 1
  .offset   rd 1

 params:
  .atcl     rb 1
  .title    rb 1
  .icon     rd 1
  .ctrl     rb 1

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
 prev_pid   rd 1

;=====================================================================
	    rb 2048
 @stack:
 @params    rb 2048

 @memory:
