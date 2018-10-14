    DEBUG = 0

    LIST_WIDTH  = 256
    WIN_WIDTH   = (LIST_WIDTH + 16 + 12)
    LIST_SIZE   = 12
    LINE_SIZE   = 40
    LIST_HEIGHT = (LIST_SIZE * LINE_SIZE / 2)
    WIN_HEIGHT  = (LIST_HEIGHT + 80)

    use32
    org     0
    db	    'MENUET01'
    dd	    1, main, dataend, memory, memory, params, 0

    include "../../proc32.inc"
    include "../../macros.inc"
    include "../../dll.inc"
    include "../../string.inc"
    include "../../develop/libraries/box_lib/trunk/box_lib.mac"

    include "lang.inc"

if DEBUG eq 1
    include "../../debug.inc"
end if

 ;===============================

if lang eq ru
 title db "Открыть с помощью", 0
 browse_txt db "Обзор...", 0
 notify_txt db "'Incorrect ", '"', "/sys/settings/assoc.ini", '"', " file' -tE", 0
 checkbox_txt db "Использовать всегда", 0
else if lang eq et
 title db "Open with", 0
 browse_txt db "Browse...", 0
 notify_txt db "'Incorrect ", '"', "/sys/settings/assoc.ini", '"', " file' -tE", 0
 checkbox_txt db "Always use selected program", 0
else if lang eq it
 title db "Open with", 0
 browse_txt db "Browse...", 0
 notify_txt db "'Incorrect ", '"', "/sys/settings/assoc.ini", '"', " file' -tE", 0
 checkbox_txt db "Always use selected program", 0
else
 title db "Open with", 0
 browse_txt db "Browse...", 0
 notify_txt db "'Incorrect ", '"', "/sys/settings/assoc.ini", '"', " file' -tE", 0
 checkbox_txt db "Always use selected program", 0
end if

 sys_dir db "/sys", 0
 slash db "/", 0
 open_dialog_path db "/sys/file managers/opendial", 0
 app_link db "$", 0
 icons db "/sys/icons32.png", 0
 communication_area_name db "FFFFFF_open_dialog", 0
 assoc_ini db "/sys/settings/assoc.ini", 0
  .sec db "Assoc", 0
  .exec db "exec", 0
  .next db "next", 0
  .icon db "icon", 0

 sb_apps scrollbar \
  13, WIN_WIDTH - 25, LIST_HEIGHT, 10 + 12, \  ;; w, x, h, y
  0, 0, LIST_SIZE / 2, 0, \		       ;; b.h, max, area, pos
  0, 0, 0, 2

 cb_always check_box2 \
  5 shl 16 + 16, (LIST_HEIGHT + 33) shl 16 + 16, 6, 0, 0, 0, checkbox_txt, ch_flag_middle

 opendialog:
  .type dd 0
  .procinfo dd buffer3
  .com_area_name dd communication_area_name
  .com_area dd 0
  .opendir_path dd buffer4
  .dir_default_path dd sys_dir
  .start_path dd open_dialog_path
  .draw_window dd draw_window
  .status dd 0
  .openfile_pach dd buffer
  .filename_area dd buffer5
  .filter_area dd .filter
  .x:
  .x_size dw 420
  .x_start dw 200
  .y:
  .y_size dw 320
  .y_start dw 120

 .filter dd 0

 ps_addres:
  dd 0		;; type
  dw 7		;; y
  dw 4		;; x
  dw 6		;; font.w
  dw LIST_WIDTH ;; w
  dd 0		;; mono
  dd 0		;; without bg
  .color dd 0	;; text color
  dd 0		;; bg color
  .txt dd 0	;; text
  dd buffer2
  dd 0

 is_execute:
  dd 7, 0, 0, 0, 0
  db 0
  dd buffer

 is_openas:
  dd 7, 0, 0, 0, 0
  db 0
  dd buffer

 is_open:
  dd 7, 0, 0, 0, 0
  db 0
  dd buffer

 is_undefined:
  dd 7, 0, notify_txt, 0, 0
  db "/sys/@notify", 0

 is_openimg_info:
  dd 5, 0, 0, 0, img.buf
  db 0
  dd icons

 is_openimg:
  dd 0, 0, 0, 0, 0
  db 0
  dd icons

 is_file_exists:
  dd 0, 0, 0, 0, buffer6
  db 0
  dd buffer

  last_x dd -1
  last_y dd -1

if DEBUG eq 1
    std_param db "~/sys/example.asm", 0
end if

 imports:
    library libini, "libini.obj"
    import  libini, libini.get_str, "ini_get_str", \
		    libini.set_str, "ini_set_str", \
		    libini.get_num, "ini_get_int", \
		    libini.for_each_section, "ini_enum_sections"

 imports_add:
    library libimg, "libimg.obj", \
	    boxlib, "box_lib.obj", \
	    prclib, "proc_lib.obj"
    import  libimg, libimg.init, "lib_init", \
		    libimg.toRGB, "img_to_rgb2", \
		    libimg.decode, "img_decode", \
		    libimg.destroy, "img_destroy"
    import  boxlib, scrollbar.draw, "scrollbar_v_draw", \
		    scrollbar.mouse, "scrollbar_v_mouse", \
		    pathshow.init, "PathShow_prepare", \
		    pathshow.draw, "PathShow_draw", \
		    checkbox.init, "init_checkbox2", \
		    checkbox.draw, "check_box_draw2", \
		    checkbox.mouse, "check_box_mouse2"
    import  prclib, opendialog.lib_init, "lib_init", \
		    opendialog.init, "OpenDialog_init", \
		    opendialog.start, "OpenDialog_start"

 ;===============================

 main:
    mcall   68, 11
    stdcall dll.Load, imports

if DEBUG eq 1
    stdcall string.copy, std_param, params
end if

 ;; trim params
	stdcall string.copy, params, paramorig
    stdcall string.trim_last, paramorig
    stdcall string.trim_first, paramorig
	mov     [param_s], eax

    stdcall string.to_lower_case, params
    stdcall string.trim_last, params
    stdcall string.trim_first, params
    mov     [param_lwr], eax

 ;; if empty - exit
    cmpe    [eax], byte 0, exit

 ;; if folder
    stdcall string.length, [param_s]
    add     eax, [param_s]
    cmpe    [eax - 1], byte '/', open

 ;; if dialog
    mov     eax, [param_s]
    mov     [is_openas + 8], eax
    cmpne   [eax], byte '~', @f
    inc     [param_s]
    mov     eax, [param_s]
    mov     [is_openas + 8], eax
    jmp     start_dialog
  @@:

 ;; if without '.' - execute
    stdcall string.last_index_of, [param_s], '.', 1
    cmpe    eax, -1, execute
 ;; if '.' is part of path - execute
    mov     esi, eax
    stdcall string.last_index_of, [param_s], '/', 1
    cmpg    eax, esi, execute

 ;; if ext == "kex" - execute
    add     esi, [param_lwr]
    mov     [param_e], esi
    cmpe    [esi], dword "kex", execute

 open_as:
    invoke  libini.get_str, assoc_ini, assoc_ini.sec, esi, buffer8, 2048, undefined
    cmpe    byte [buffer8], 0, start_dialog_pre
 .run:
    mov     edi, 0
    cmpne   byte [buffer8], "$", .pre_open
    mov     edi, 1
    invoke  libini.get_str, assoc_ini, buffer8 + 1, assoc_ini.exec, buffer, 2048, undefined
    cmpe    byte [buffer], 0, ini_error
    jmp     @f
 .pre_open:
    stdcall string.copy, buffer8, buffer
  @@:
    mcall   70, is_openas
    cmpge   eax, 0, exit
    cmpe    edi, 0, start_dialog
    invoke  libini.get_str, assoc_ini, buffer8 + 1, assoc_ini.next, buffer8, 2048, undefined
    cmpne   byte [buffer], 0, .run
    jmp     start_dialog

 execute:
    mov     eax, [param_s]
    mov     [is_execute + 21], eax
    mcall   70, is_execute
    jmp     exit

 open:
    invoke  libini.get_str, assoc_ini, assoc_ini.sec, slash, buffer, 2048, undefined
    cmpne   [buffer], byte "$", @f
    invoke  libini.get_str, assoc_ini, buffer + 1, assoc_ini.exec, buffer, 2048, undefined
    cmpe    [buffer], byte 0, ini_error
  @@:
    mov     eax, [param_s]
    mov     [is_open + 8], eax
    mcall   70, is_open
    jmp     exit

 ini_error:
    mcall   70, is_undefined
    jmp     exit

 ;----------------------

 start_dialog_pre:
    or	    [cb_always.flags], ch_flag_en

 start_dialog:
    stdcall dll.Load, imports_add
    invoke  opendialog.lib_init

    mcall   40, 100111b

 ;; get title
    stdcall string.copy, title, win.title

 ;; get positions
    mcall   14
    movzx   ebx, ax
    shr     ebx, 1
    shr     eax, 16 + 1
    sub     eax, WIN_WIDTH / 2 - 1
    sub     ebx, WIN_HEIGHT / 2 - 1
    mov     [win.x], eax
    mov     [win.y], ebx

 ;; get colors
    mcall   48, 3, skin, sizeof.system_colors

 ;; get opendialog
    invoke  opendialog.init, opendialog

 ;; get pathshow
    mov     eax, [param_s]
    mov     ebx, [skin.work_text]
    mov     [ps_addres.txt], eax
    mov     [ps_addres], ebx
	m2m     [ps_addres.color], [skin.work_text]
    invoke  pathshow.init, ps_addres

 ;; get checkbox
    mov     eax, 0xFFFfff
    mov     ebx, [skin.work_graph]
    mov     ecx, [skin.work_text]
    mov     [cb_always.color], eax
    mov     [cb_always.border_color], ebx
    mov     [cb_always.text_color], ecx
    invoke  checkbox.init, cb_always

 ;; get list
    invoke  libini.for_each_section, assoc_ini, section_cb
    stdcall sort_list
    mov     eax, [sb_apps.max_area]
    and     eax, 1b
    shr     [sb_apps.max_area], 1
    add     [sb_apps.max_area], eax

    mov     eax, 0xFFFfff
    mov     ebx, 0xCCCccc ;[skin.3d]
    mov     [sb_apps.bg_color], eax
    mov     [sb_apps.front_color], ebx
    mov     [sb_apps.line_color], ebx

 ;; get icons
    mcall   70, is_openimg_info
    mov     edx, dword [img.buf + 32]
    shl     edx, 4
    stdcall mem.Alloc, edx
    mov     [img.rgb], eax

    mov     [is_openimg + 12], edx
    m2m     dword[is_openimg + 16], dword[img.rgb]
    mcall   70, is_openimg

    invoke  libimg.decode, dword[img.rgb], ebx, 0
    mov     [img], eax

  ;; alpha
    mov     edi, [eax + 8]
    shl     edi, 7
    sub     edi, 4
    mov     eax, [eax + 24]
 .setalpha:
    mov     ebx, [eax + edi]
    shr     ebx, 24
    cmpne   ebx, 0, @f
    mov     ecx, 0xFFFfff
    mov     [eax + edi], ecx
 @@:
    cmpe    edi, 0, @f
    sub     edi, 4
    jmp     .setalpha
 @@:

    invoke  libimg.toRGB, [img], [img.rgb]
    invoke  libimg.destroy, [img]

 ;----------------------

 update:
    mcall   10
    cmpe    eax, EV_REDRAW, event_redraw
    cmpe    eax, EV_KEY, event_key
    cmpe    eax, EV_BUTTON, event_button
    cmpe    eax, EV_MOUSE, event_mouse
    jmp     update

 ;----------------------

 event_redraw:
    call    draw_window
    jmp     update

 ;----------------------

 event_key:
    mcall   2
    cmpe    ah, 27, exit

    cmpe    ah,  13, list_item_activate
    cmpe    ah,   9, event_button.opendialog
    cmpe    ah,  32, .toggle_cb
    cmpe    ah, 179, .go_right
    cmpe    ah, 176, .go_left
    cmpe    ah, 178, .go_up
    cmpe    ah, 177, .go_down

    cmpl    ah, "a", @f
    cmpg    ah, "z", @f
    jmp     .letter
  @@:

    cmpl    ah, "а", @f
    cmpg    ah, "я", @f
    jmp     .letter
  @@:

    cmpl    ah, "A", @f
    cmpg    ah, "Z", @f
    jmp     .letter_big
  @@:

    cmpl    ah, "А", @f
    cmpg    ah, "Я", @f
    jmp     .letter_big
  @@:

    cmpl    ah, "0", @f
    cmpg    ah, "9", @f
    jmp     .letter
  @@:

    jmp     update

 .letter_big:
    movzx   ebx, ah
    stdcall downcase_char, ebx
    mov     ah, al

 .letter:
    push    eax

    stdcall get_index
    inc     eax

    mov     ecx, eax

    imul    ebx, eax, 32
    add     ebx, list

    pop     eax

    mov     edi, 0
    mov     esi, [list.size]
 .search:
    shl     esi, 5
    add     esi, list
 @@:
    cmpe    ebx, esi, @f

    movzx   edx, byte [ebx]
    stdcall downcase_char, edx
    cmpne   ah, al, .next
    stdcall set_index, ecx
    stdcall draw_list
    jmp     update

 .next:
    inc     ecx
    add     ebx, 32
    jmp     @b
  @@:
    cmpe    edi, 1, update
    mov     ebx, list
    mov     ecx, 0
    mov     edi, 1
    push    eax
    stdcall get_index
    mov     esi, eax
    pop     eax
    jmp     .search

 .toggle_cb:
    mov     eax, [cb_always.flags]
    mov     ebx, eax
    and     ebx, ch_flag_en
    cmpe    ebx, 0, @f
    sub     eax, ch_flag_en
    jmp     .toggle_cb.redraw
  @@:
    add     eax, ch_flag_en
 .toggle_cb.redraw:
    mov     [cb_always.flags], eax
    invoke  checkbox.draw, cb_always
    jmp     update

 .go_right:
    mov     esi, 1
    jmp     .go_anyway

 .go_left:
    mov     esi, -1
    jmp     .go_anyway

 .go_up:
    mov     esi, -2
    jmp     .go_anyway

 .go_down:
    mov     esi, 2

 .go_anyway:
;; HIDE OLD SELECTION
    stdcall draw_item, [last_x], [last_y], 0

;; [X, Y] -> INDEX
    stdcall get_index

;; CHANGE INDEX
 .check_p2:
    cmpne   esi, 2, .check_m2
    mov     ebx, [list.size]
    dec     ebx
    sub     ebx, eax
    cmpg    ebx, 1, .add
    jmp     .check

 .check_m2:
    cmpne   esi, -2, .add
    cmpg    eax, 1, .add
    jmp     .check

 .add:
    add     eax, esi

 .check:
    cmpl    eax, [list.size], @f
    mov     eax, [list.size]
    dec     eax
  @@:

    cmpge   eax, 0, @f
    mov     eax, 0
  @@:

;; INDEX -> [X, Y]
    stdcall set_index, eax
    cmpe    eax, 1, @f

;; PARTLY REDRAW
    stdcall draw_item, [last_x], [last_y], 1
    jmp     update

;; FULL REDRAW
  @@:
    stdcall draw_list
    jmp     update

 ;----------------------

 event_button:
    mcall   17
    cmpe    ah, 1, exit
    cmpe    ah, 2, .opendialog
    jmp     list_item_activate

 .opendialog:
    invoke  opendialog.start, opendialog
    cmpne   [opendialog.status], 1, update
    mcall   70, is_openas

    mov     edi, 0
    jmp     save_assoc

 ;----------------------

 list_item_activate:
    stdcall get_index
    shl     eax, 5
    add     eax, list
    cmpe    byte [eax], 0, update
    mov     [param_a], eax
    invoke  libini.get_str, assoc_ini, eax, assoc_ini.exec, buffer, 256, undefined
    cmpe    byte [buffer], 0, ini_error
    mcall   70, is_openas

    mov     edi, 1
    jmp     save_assoc

 ;----------------------

 save_assoc:
    mov     eax, [cb_always.flags]
    and     eax, ch_flag_en
    cmpe    eax, 0, exit

    cmpe    edi, 0, @f
    stdcall string.copy, app_link, buffer
    stdcall string.concatenate, [param_a], buffer
  @@:

    stdcall string.length, buffer
	mov     edi, eax
	
    invoke  libini.set_str, assoc_ini, assoc_ini.sec, [param_e], buffer, edi
    jmp     exit

 ;----------------------

 event_mouse:
    invoke  checkbox.mouse, cb_always
    mov     edi, [sb_apps.position]
    invoke  scrollbar.mouse, sb_apps
    sub     edi, [sb_apps.position]
    jz	    @f
    call    draw_list
  @@:

    mcall   37, 1
    movzx   edi, ax
    shr     eax, 16
    mov     esi, eax
    sub     esi, 4
    sub     edi, 10 + 12
    cmpl    esi, 0, .out
    cmpge   esi, LIST_WIDTH, .out
    cmpl    edi, 0, .out
    cmpge   edi, LIST_HEIGHT, .out

    mcall   37, 7
    and     eax, 0xFFFF
    cmpe    eax, 65535, .scroll_up
    cmpe    eax, 0, @f

 .scroll_down:
    mov     eax, [sb_apps.position]
    add     eax, LIST_SIZE / 2
    shl     eax, 1
    cmpge   eax, [list.size], @f
    inc     [sb_apps.position]
    stdcall draw_list
    jmp     update

 .scroll_up:
    cmpe    [sb_apps.position], 0, @f
    dec     [sb_apps.position]
    stdcall draw_list
    jmp     update

  @@:

    mov     eax, esi
    mov     ebx, LIST_WIDTH / 2
    mov     edx, 0
    div     ebx
    mov     esi, eax

    mov     eax, edi
    mov     ebx, LINE_SIZE
    mov     edx, 0
    div     ebx
    mov     edi, eax

    cmpne   esi, [last_x], .redraw
    cmpne   edi, [last_y], .redraw
    jmp     @f

 .redraw:
    stdcall draw_item, [last_x], [last_y], 0
    mov     [last_x], esi
    mov     [last_y], edi
    stdcall draw_item, esi, edi, 1
    jmp     @f

 .out:
    cmpe    [last_x], -1, @f
    stdcall draw_item, [last_x], [last_y], 0
    mov     [last_x], -1
    mov     [last_y], -1
  @@:

    jmp     update

 ;----------------------

 exit:
    mcall   -1

 ;----------------------

 proc draw_window
    mcall   12, 1

    mov     edx, [skin.work]
    or	    edx, 0x34 shl 24
    mcall   0, <[win.x], WIN_WIDTH>, <[win.y], WIN_HEIGHT>, , , win.title
    stdcall draw_list
    invoke  pathshow.draw, ps_addres
    invoke  checkbox.draw, cb_always

    mcall   8, <208, 63>, <LIST_HEIGHT + 28, 21>, 2, [skin.work_button]
;
    mov     ecx, [skin.work_button_text]
    add     ecx, 0x80 shl 24
    mcall   4, <214, LIST_HEIGHT + 36>, , browse_txt

 ;; buttons
    mov     eax, 8
    mov     ebx, 4 shl 16 + LIST_WIDTH / 2
    mov     ecx, (10 + 12) shl 16 + LINE_SIZE - 1
    mov     edx, 0x60 shl 24 + 10
    mov     edi, LIST_SIZE
    mov     esi, 0
  @@:
    cmpe    edi, 0, @f
    mcall

    cmpe    esi, 0, .doA
 .doB:
    sub     ebx, LIST_WIDTH shl 16
    add     ecx, LINE_SIZE shl 16
 .doA:
    add     ebx, (LIST_WIDTH / 2) shl 16
    sub     ecx, LINE_SIZE shl 16

    not     esi
    add     ecx, LINE_SIZE shl 16
    inc     edx
    dec     edi
    jmp     @b
  @@:

    mcall   12, 2
    ret
  endp

 ;----------------------

 proc draw_list
    mcall   13, <3, LIST_WIDTH + 2 + 12>, <9 + 12, 1>, [skin.work_graph]
    mcall     , 			, <LIST_HEIGHT + 9 + 1 + 12, 1>
    mcall     , <3, 1>, <9 + 12, LIST_HEIGHT + 1>
    mcall     , <3 + LIST_WIDTH + 12 + 1, 1>
    mcall     , <4, LIST_WIDTH>, <10 + 12, LIST_HEIGHT>, 0xFFFfff

    mov     esi, 1
    mov     edi, LIST_SIZE / 2 - 1
 .draw_loop:
    mov     edx, 0
    cmpne   [last_x], esi, @f
    cmpne   [last_y], edi, @f
    mov     edx, 1
  @@:
    stdcall draw_item, esi, edi, edx
    dec     esi
    cmpne   esi, -1, @f
    mov     esi, 1
    dec     edi
  @@:
    cmpne   edi, -1, .draw_loop

    invoke  scrollbar.draw, sb_apps

    ret
 endp

 ;----------------------

 proc draw_item uses edx edi esi, _x, _y, _sel
 ;; get index
    stdcall get_index_cur, [_x], [_y]
    mov     edi, eax

    cmpge   edi, [list.size], .break

 ;; background
    mov     ebx, [_x]
    shl     ebx, 7 + 16
    add     ebx, 4 shl 16 + LIST_WIDTH / 2

    imul    ecx, [_y], LINE_SIZE
    shl     ecx, 16
    add     ecx, (10 + 12) shl 16 + LINE_SIZE

    mov     edx, 0xFFFfff
    cmpe    [_sel], 0, @f
    mov     edx, 0x94AECE
  @@:
    mcall   13

 ;; shadows
    push    ecx
    cmpe    [_sel], 1, .after_shadows
    mov     edx, 0xCCCccc ;[skin.3d]

    cmpne   [_x], 0, @f
    imul     ecx, [_y], LINE_SIZE
    shl      ecx, 16
    add      ecx, (10 + 12) shl 16 + LINE_SIZE
    mcall     , <4, 1>
  @@:

    cmpne   [_y], 0, @f
    imul     ebx, [_x], LIST_WIDTH / 2
    shl      ebx, 16
    add      ebx, 4 shl 16 + LIST_WIDTH / 2
    mcall     , , <10 + 12, 1>
  @@:

 .after_shadows:
    pop     ecx

 ;; icon
    and     ebx, 0xFFFF shl 16
    shr     ecx, 16
    add     ecx, ebx
    mov     edx, ecx
    add     edx, 6 shl 16 + (LINE_SIZE - 32) / 2

    mov     ebx, edi
    shl     ebx, 2
    mov     ebx, [ebx + list.icon]
    imul    ebx, 32 * 32 * 3
    add     ebx, [img.rgb]
    cmpe    [_sel], 0, .draw_icon
 .selected:
    mov     eax, img.sel
    mov     ecx, 32 * 32
    push    edx
  @@:
    mov     edx, [ebx]
    and     edx, 0xFFFFFF
    cmpne   edx, 0xFFFFFF, .not
    mov     edx, 0x94AECE
 .not:
    mov     [eax], edx
    add     eax, 3
    add     ebx, 3
    dec     ecx
    jnz     @b
    pop     edx
    mov     ebx, img.sel

 .draw_icon:
    mcall   7, , <32, 32>

 ;; text
    mov     ebx, edx
    add     ebx, (32 + 6) shl 16 + 32 / 2 - 9 / 2

    mov     ecx, 0x000000 ; inactive item text
    cmpe    [_sel], 0, @f
    mov     ecx, 0x000000 ; active item text
  @@:
    add     ecx, 0x80 shl 24

    mov     edx, edi
    shl     edx, 5
    add     edx, list

    mcall   4

  .break:
    ret
 endp

 ;----------------------

 proc downcase_char uses ebx, _ch
    mov     ebx, [_ch]

    cmpl    bl, "A", @f
    cmpg    bl, "Z", @f
    sub     bl, "A" - "a"
    jmp     .ret
  @@:

    cmpl    bl, "А", @f
    cmpg    bl, "Я", @f
    sub     bl, "А" - "а"
  @@:

 .ret:
    mov     al, bl
    ret
 endp

 ;----------------------

 proc get_index_cur uses ebx, _x, _y
    mov     eax, [_y]
    shl     eax, 1
    add     eax, [_x]
    mov     ebx, [sb_apps.position]
    shl     ebx, 1
    add     eax, ebx

    ret
 endp

 ;----------------------

 proc get_index
    stdcall get_index_cur, [last_x], [last_y]
    ret
 endp

 ;----------------------

 proc set_index uses ebx,_index
    mov     eax, [_index]
    mov     ebx, [sb_apps.position]
    shl     ebx, 1
    sub     eax, ebx

    mov     ebx, eax
    and     ebx, 1b
    mov     [last_x], ebx
    shr     eax, 1
    mov     [last_y], eax

    mov     eax, 0

    cmpl    [last_y], 0xFFFF, @f
    mov     ebx, 0xFFFFFFFF shr 1
    sub     ebx, [last_y]
    inc     ebx
    mov     [last_y], 0
    ;dec     [sb_apps.position]
    sub     [sb_apps.position], ebx
    mov     eax, 1
    jmp     .exit
  @@:

    cmpl    [last_y], LIST_SIZE / 2, @f
    mov     ebx, [last_y]
    sub     ebx, LIST_SIZE / 2 - 1
    mov     [last_y], LIST_SIZE / 2 - 1
    add     [sb_apps.position], ebx
    mov     eax, 1
    jmp     .exit
  @@:

 .exit:
    cmpge   [sb_apps.position], 0, @f
    mov     [sb_apps.position], 0
  @@:

    ret
 endp

 ;----------------------

 proc section_cb, _file, _sec
 ;; CHECK IF EXISTS
    invoke  libini.get_str, assoc_ini, [_sec], assoc_ini.exec, buffer, 2048, undefined
    mcall   70, is_file_exists
    cmpe    ebx, -1, .exit

    mov     ebx, [list.size]
    shl     ebx, 5
    add     ebx, list
    stdcall string.cmp, [_sec], assoc_ini.sec
    cmpe    eax, 0, @f
    stdcall string.copy, [_sec], ebx
    invoke  libini.get_num, [_file], [_sec], assoc_ini.icon, 0
    mov     ecx, [list.size]
    shl     ecx, 2
    mov     [ecx + list.icon], eax
    inc     [list.size]
    inc     [sb_apps.max_area]
  @@:
    mov     eax, 1
  .exit:
    ret
 endp

 ;----------------------

 proc sort_list
    mov     eax, list
    mov     ebx, list.lowercased
  @@:
    stdcall string.copy, eax, ebx
    stdcall string.to_lower_case, ebx
    add     eax, 32
    add     ebx, 32
    cmpne   byte [eax], 0, @b

    mov     edi, 0
    mov     ebx, list.lowercased ;; i = 0
    imul    ecx, [list.size], 32 ;; i < n - 1
    sub     ecx, 32
    add     ecx, list.lowercased

 .loop1:
    mov     edx, list.lowercased ;; j = 0
    mov     esi, [list.size]	 ;; j < n - i - 1
    sub     esi, edi
    dec     esi
    imul    esi, 32
    add     esi, list.lowercased
 .loop2:
    mov     eax, edx
    add     eax, 32
    stdcall string.cmp, edx, eax, 32
    cmpne   eax, 1, .next2
;; swap names lw
    mov     eax, edx
    add     eax, 32
    stdcall string.copy, edx, buffer7
    stdcall string.copy, eax, edx
    stdcall string.copy, buffer7, eax
;; swap names
    mov     eax, edx
    sub     eax, 256 * 32 - 32
    sub     edx, 256 * 32
    stdcall string.copy, edx, buffer7
    stdcall string.copy, eax, edx
    stdcall string.copy, buffer7, eax
    add     edx, 256 * 32
;; swap icons
    mov     eax, edx
    sub     eax, list.lowercased
    shr     eax, 3
    add     eax, list.icon
    push    ebx ecx
    mov     ebx, [eax]
    mov     ecx, [eax + 4]
    mov     [eax], ecx
    mov     [eax + 4], ebx
    pop     ecx ebx
 .next2:
    add     edx, 32
    cmpne   edx, esi, .loop2
 .next1:
    inc     edi
    add     ebx, 32
    cmpne   ebx, ecx, .loop1

    ret
 endp

 ;----------------------

 dataend:

 ;===============================

 skin system_colors
 list rb 32 * 256
  .lowercased rd 32 * 256
  .icon rd 256
  .size rd 1
 img  rd 1
  .rgb rd 1
  .size rd 1
  .buf rb 40
  .sel rb 32 * 32 * 3
 win:
  .x rd 1
  .y rd 1
 win.title rb 256
 param_lwr rd 1
 param_e rd 1
 param_a rd 1
 param_s rd 1
 undefined rb 1
 buffer  rb 2048
 buffer2 rb 2048 ;OD
 buffer3 rb 2048 ;OD
 buffer4 rb 2048 ;OD
 buffer5 rb 2048 ;OD
 buffer6 rb 2048 ;check existance
 buffer7 rb 32	 ;for sorting
 buffer8 rd 2048
 params rb 2048
 paramorig rb 2048
 _stack rb 2048
 memory:
