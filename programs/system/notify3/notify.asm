    use32
    org     0
    db	    'MENUET01'
    dd	    1, @ENTRY, @end, @memory, @stack, @params, 0

    include "../../macros.inc"
    include "../../proc32.inc"
    include "../../dll.inc"
;    include "../../debug.inc"
    include "../../notify.inc"
    include "../../string.inc"

 LINEH	    equ 12
 MARGIN     equ 12
 ICONS	    equ 11

;-------------------------------------------------------------------------------

 @ENTRY:
;; INIT HEAP
    mcall   68, 11

;; SAVE FOCUSED WINDOW
    mcall   18, 7
    mov     ecx, eax
    mcall   9, buffer
    m2m     dword [prev_pid], dword [buffer + 30]

;; MAKE IT ALWAYS ON TOP
    mcall   18, 25, 2, -1, 1

;; SET STD PARAMS, IF IT NEEDS
    mov     eax, @params
    cmpne   byte [@params], 0, @f
    mov     eax, sz_std
  @@:
    mov     [params.source], eax

;; PARSE ARGUMENTS
    mov     esi, 0
    call    parse_text
    call    parse_flags
    movzx   ebx, [params.icon]

;; GET PID
    mcall   9, buffer, -1
    mov     eax, dword [buffer + 30]

;; CONVERT PID TO STR
    mov     ebx, 10
    mov     ecx, 0
  @@:
    mov     edx, 0
    div     ebx
    push    edx
    inc     ecx
    cmpne   eax, 0, @b

    mov     ebx, ctrl.name
  @@:
    pop     eax
    add     al, "0"
    mov     [ebx], al
    inc     ebx
    loop    @b

    mov     dword [ebx + 0], "-NOT"
    mov     dword [ebx + 4], "IFY"

;; LOAD LIBRARIES
    stdcall dll.Load, @imports

;; GET SIZE OF ICONS
    mcall   70, fi
    mov     edx, dword [buffer + 32]
    shl     edx, 2

;; ALLOCATE MEMORY FOR THIS
    stdcall mem.Alloc, edx
    mov     [img_data.rgb_obj], eax

;; READ ICONS
    mov     dword [fi + 00], 0
    mov     dword [fi + 12], edx
    mov     dword [fi + 16], eax
    mcall   70, fi

;; DECODE ICONS
    stdcall dword [img.decode], dword [img_data.rgb_obj], ebx, 0
    mov     dword [img_data.obj], eax
    stdcall dword [img.to_rgb], dword [img_data.obj], dword [img_data.rgb_obj]
    stdcall dword [img.destroy], dword [img_data.obj]



;; CALC HEIGHT
    mov     eax, [text.lines]
    add     eax, 2
    imul    eax, LINEH
    mov     [window.height], eax

;; CALC OFFSET OF TEXT
    mov     dword [text.offset], MARGIN
    cmpe    byte [params.icon], 0, @f
    add     dword [text.offset], MARGIN + 24
  @@:

;; CALC WIDTH
    mov     eax, [text.max_len]
    imul    eax, 6
    add     eax, MARGIN
    add     eax, [text.offset]
    mov     [window.width], eax

    mcall   14
    mov     ebx, eax
    movzx   ebx, bx
    mov     [scr.height], ebx
    shr     eax, 16
    mov     [scr.width], eax
    sub     eax, [window.width]
    sub     eax, LINEH
    mov     [window.x], eax

;; CALC PBAR
    mov     eax, [window.width]
    sub     eax, [text.offset]
    sub     eax, MARGIN
    mov     [pbar.width], eax

;; CALC Y
    mcall   68, 22, sz_shname, 256 + 512, 4 + 1 ;OPEN_ALWAYS AND WRITE
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

;; SET EVENT MASK
    mcall   40, 101b

;; INIT TIMER
    mov     eax, 60
    imul    eax, [text.lines]
    mov     [timer], eax
    mov     dword [timer.step], 1
    cmpne   byte [params.atcl], 1, @f
    mov     dword [timer.step], 0
  @@:

;; INIT WINDOW
    call    init_window

;; RESTORE FOCUS
    mcall   18, 21, [prev_pid]
    mov     ecx, eax
    mcall   18, 3

    jmp     redraw

 ;----------------------------

 update:
    mcall   23, 10
    cmpe    al, EV_REDRAW, redraw
    cmpe    al, EV_BUTTON, button

    mov     edi, update

;; TRY OPEN CONTROLLER
    cmpe    byte [params.ctrl], 1, .fail_controller_open
    mcall   68, 22, ctrl.name, , 0x01
    cmpe    eax, 0, .fail_controller_open
    mov     byte [params.ctrl], 1
    mov     [ctrl.addr], eax

;; COPY TEXT TO CTRL
    add     eax, NTCTRL_TEXT
    mov     ebx, text.buffer

    mov     ecx, [text.lines]
    cmpne   byte [params.pbar], 1, @f
    dec     ecx
  @@:

 .copy_start:
    cmpe     ecx, 0, .copy_end
    mov      dl, [ebx]
    cmpne    dl, 0, @f
    mov      dl, 10
    dec      ecx
  @@:
    mov      [eax], dl
    inc      eax
    inc      ebx
    jmp      .copy_start
 .copy_end:
    mov      byte [eax - 1], 0

;; COPY FLAGS TO CTRL
    mov      eax, [ctrl.addr]
    add      eax, NTCTRL_ICON
    mov      dl, [params.icon]
    mov      [eax], dl

    mov      eax, [ctrl.addr]
    add      eax, NTCTRL_TITLE
    mov      dl,  [params.title]
    mov      [eax], dl

;; SET CONTROLLER READY
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_READY
    mov     byte [eax], 1
 .fail_controller_open:


    cmpe    [params.ctrl], 0, .no_ctrl
;; TEST TEXT
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_TEXT
    cmpne   byte [eax], 1, @f
    mov     byte [eax], 0

    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_TEXT
    mov     esi, 1
    call    parse_text

    mov     edi, redraw
  @@:

;; TEST ICON
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_ICON
    cmpne   byte [eax], 1, @f
    mov     byte [eax], 0

    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_ICON
    mov     dl, [eax]
    mov     [params.icon], dl

    mov     edi, redraw
  @@:

;; TEST TITLE
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_TITLE
    cmpne   byte [eax], 1, @f
    mov     byte [eax], 0

    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_TITLE
    mov     dl, [eax]
    mov     [params.title], dl

    mov     edi, redraw
  @@:

;; TEST PBAR
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_APPLY_PBAR
    cmpne   byte [eax], 1, @f
    mov     byte [eax], 0

    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_PBAR_CUR
    mov     edx, [eax]
    mov     [pbar.cur_val], edx

    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_PBAR_MAX
    mov     edx, [eax]
    mov     [pbar.max_val], edx

    call    calc_fill_width

    mov     edi, redraw
  @@:

;; TEST CLOSE
    mov     eax, [ctrl.addr]
    add     eax, NTCTRL_CLOSE
    cmpe    byte [eax], 1, exit

 .no_ctrl:

    mov     eax, [timer.step]
    sub     [timer], eax
    cmpe    [timer], dword 0, exit
    jmp     edi

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

 button:
    mcall   17
    cmpe    byte [params.clcl], 0, exit
    jmp     update

 ;----------------------------

 redraw:
    call    draw_window
    call    draw_text
    call    draw_pbar

    jmp     update

 ;----------------------------

 draw_window:
    call    init_window

    movzx   ebx, bx
    movzx   ecx, cx
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

    cmpe    byte [params.icon], 0, @f
    movzx   ebx, byte [params.icon]
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

    cmpne   byte [params.pbar], 1, @f
    dec     esi
  @@:

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

    cmpne   byte [params.title], 1, @f
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

 draw_pbar:
    cmpne   byte [params.pbar], 1, .exit
    mov     esi, 0xFF0000
    mov     ecx, LINEH
    imul    ecx, [text.lines]
    shl     ecx, 16
    add     ecx, (LINEH / 2) shl 16 + 8
    mcall   13, <[text.offset], [pbar.width]>, , 0x555555

    sub     ecx, -(1 shl 16) + 2

    mov     bx, word [pbar.f_width]
    add     ebx, 1 shl 16

    mcall     , , , 0x999999
    mov     cx, 1
    mcall     , , , 0xAAAAAA
    mov     cx, 6
    mov     bx, 1
    mcall
 .exit:
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
    mov     dword [text.lines], 1
    cmpne   byte [params.pbar], 1, @f
    inc     dword [text.lines]
  @@:

    mov     dword [text.max_len], 0
    mov     ebx, text.buffer
    mov     ecx, 0
    mov     dl, 0
    mov     dh, 0

    cmpe    esi, 1, .parse_loop
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

    cmpe    byte [eax], 10, .newline
    cmpe    esi, 1, .next_set_char
    cmpne   byte [eax], "\", @f
    cmpe    byte [eax + 1], dl, .quote
    cmpe    byte [eax + 1], "n", .newline_esc
    jmp     @f

 .quote:
    inc     eax
    jmp     .next_set_char

 .newline_esc:
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

 .next_set_char:
    mov     dh, [eax]
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

 parse_flags:
    mov     byte [params.atcl], 0
    mov     byte [params.title], 0
    mov     byte [params.icon], 0

 .loop:
    mov     dl, [eax]

    cmpe    dl, 0, .exit
    cmpe    dl, "d", .set_atcl
    cmpe    dl, "c", .set_clcl
    cmpe    dl, "t", .set_title
    cmpe    dl, "p", .set_pbar

    mov     bl, 1
    mov     ecx, sz_icons
  @@:
    cmpe    dl, [ecx], .set_icon
    inc     bl
    inc     ecx
    cmpne   [ecx], byte 0, @b
    jmp     .next_char

 .set_atcl:
    mov     byte [params.atcl], 1
    jmp     .next_char

 .set_clcl:
    mov     byte [params.clcl], 1
    jmp     .next_char

 .set_title:
    mov     byte [params.title], 1
    jmp     .next_char

 .set_pbar:
    mov     byte [params.pbar], 1
    inc     dword [text.lines]
    jmp     .next_char

 .set_icon:
    mov     [params.icon], bl

 .next_char:
    inc     eax
    jmp     .loop

 .exit:
    ret

 ;----------------------------

 calc_fill_width:
    mov     eax, [pbar.cur_val]
    cmpng   eax, [pbar.max_val], @f
    mov     eax, [pbar.max_val]
  @@:
    cmpnl   eax, 0, @f
    mov     eax, 0
  @@:

    mov     ebx, [pbar.width]
    sub     ebx, 2
    imul    eax, ebx
    mov     edx, 0
    mov     ebx, [pbar.max_val]
    div     ebx
    mov     [pbar.f_width], eax
    ret

 ;----------------------------

 @imports:
    library img, "libimg.obj"
    import  img, img.to_rgb,  "img_to_rgb2", \
		 img.decode,  "img_decode",  \
		 img.destroy, "img_destroy"

 ;----------------------------

 sz_icons   db "AEWONIFCMDS", 0
 sz_ifile   db "/sys/notify3.png", 0
 sz_shname  db "notify-mem-v01", 0
 sz_std     db "'NOTIFY 3\n",                 \
	       "d - disable auto-closing\n",  \
	       "c - disable click-closing\n", \
	       "p - use progressbar\n",       \
	       "t - title\n",		      \
	       " \n",			      \
	       "ICONS:\n",		      \
	       "A - application\n",	      \
	       "E - error\n",		      \
	       "W - warning\n", 	      \
	       "O - ok\n",		      \
	       "N - network\n", 	      \
	       "I - info\n",		      \
	       "F - folder\n",		      \
	       "C - component\n",	      \
	       "M - mail\n",		      \
	       "D - download\n",	      \
	       "S - audio player",	      \
	       "' -td", 0

 fi:
	    dd 5
	    dd 0, 0, 0
	    dd buffer
	    db 0
	    dd sz_ifile

 ;----------------------------

 @end:

;=====================================================================

 window:
 .x	   rd 1
 .y	   rd 1
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
 .source   rd 1
 .atcl	   rb 1
 .clcl	   rb 1
 .title    rb 1
 .pbar	   rb 1
 .icon	   rb 1
 .ctrl	   rb 1

 img_data:
 .rgb_obj  rd 1
 .obj	   rd 1

 timer:
 .value    rd 1
 .step	   rd 1

 shm:
 .addr	   rd 1
 .our	   rd 1

 ctrl:
 .name	   rb 31
 .addr	   rd 1

 pbar:
 .width    rd 1
 .cur_val  rd 1
 .max_val  rd 1
 .f_width  rd 1

 buffer     rb 1024
 first_draw rb 1
 prev_pid   rd 1

;=====================================================================
	    rb 2048
 @stack:
 @params    rb 2048

 @memory:
