 ;;; Docky v0.4 by eAndrew

    use32
    org     0x0
;-------------------------------------------------------------------------------
    db	    "MENUET01"
    dd	    1, main, __dataend, __memend, __stackend, 0, sys_path
;-------------------------------------------------------------------------------
    include "../../../macros.inc"
    include "../../../proc32.inc"
    include "../../../dll.inc"
    include "../../../develop/libraries/box_lib/load_lib.mac"

    @use_library_mem	 \
	    mem.Alloc,	 \
	    mem.Free,	 \
	    mem.ReAlloc, \
	    dll.Load
;-------------------------------------------------------------------------------
ICON_SIZE	 equ  32 * 32 * 3
IMAGE_FILE_SIZE  equ  ICON_SIZE * 29
IMAGE_DATA_SIZE  equ  32 * 32 * 4 * 29
;================================================================================
proc main
; ==== Init ====
    mcall   18, 7
    mov     [win.psid], eax

    mcall   40, 100101b

    mov     dword[file_exec.proc], 7

; ==== Load libs ====
    load_libraries load_lib_start, load_lib_end

; ==== Config LibINI ====
    invoke  ini.get_int, ini_data.file_name, ini_data.settings_name, ini_data.location_name, -1
    mov     [dock_items.location], eax

    invoke  ini.get_str, ini_data.file_name, ini_data.settings_name, ini_data.skin_name, ini_data.skin_file, 32, 0

    invoke  ini.get_color, ini_data.skin_dir, ini_data.skin_name, ini_data.color_bg, 0x0
    mov     [color.bg], eax
    invoke  ini.get_color, ini_data.skin_dir, ini_data.skin_name, ini_data.color_frame, 0xFFFFFF
    mov     [color.frame], eax
    invoke  ini.get_color, ini_data.skin_dir, ini_data.skin_name, ini_data.color_framein, 0x888888
    mov     [color.framein], eax
    invoke  ini.get_color, ini_data.skin_dir, ini_data.skin_name, ini_data.color_text, 0xFFFFFF
    or	    eax, 0x80000000
    mov     [color.text], eax

    invoke  ini.sections, ini_data.file_name, sections_callback

; ==== Config LibIMG ====
    stdcall mem.Alloc, dword IMAGE_FILE_SIZE
    mov     [img_data.rgb_object], eax

    mov     dword[img_data.file.proc], 0
    mov     dword[img_data.file.position], 0
    mov     dword[img_data.file.size], dword IMAGE_FILE_SIZE
    m2m     dword[img_data.file.buffer], dword[img_data.rgb_object]
    mov     byte[img_data.file + 20], 0
    mov     dword[img_data.file.name], img_data.file_name

    mcall   70, img_data.file

    cmp     ebx, 0xFFFFFFFF
    je	    @f

    stdcall dword[img.decode], dword[img_data.rgb_object], ebx, 0
    mov     dword[img_data.object], eax

  ; === ALPHA ===
    mov     edi, 0
    add     eax, 24
    mov     eax, [eax]
    mov     ecx, [color.bg]
 .setalpha:
    mov     ebx, [eax + edi]
    shr     ebx, 24
    cmp     ebx, 0
    jne     .nonalpha
    mov     [eax + edi], ecx
 .nonalpha:
    add     edi, 4
    cmp     edi, IMAGE_DATA_SIZE
    jne     .setalpha

  ; === CONVERTING TO BGR
    stdcall dword[img.toRGB], dword[img_data.object], dword[img_data.rgb_object]
    stdcall dword[img.destroy], dword[img_data.object]

; ==== Config window ====
    mov     eax, dword[dock_items.location]
    and     eax, 1b
    cmp     eax, 0
    je	    .vert
    jmp     .setshape

 .vert:
    mov     byte[win.isvert], 1

 .setshape:
    cmp     byte[win.isvert], 1
    je	    .vert_sp

 .horz_sp:
    call    .HORZ_WIDTH
    call    .HORZ_X
    call    .HORZ_HEIGHT
    cmp     dword[dock_items.location], 1
    je	    .settop

 .setbottom:
    call    .HORZ_Y_BOTTOM
    jmp     .SETDEF

 .settop:
    call    .HORZ_Y_TOP
    jmp     .SETDEF


 .vert_sp:
    call    .VERT_WIDTH
    call    .VERT_HEIGHT
    call    .VERT_Y
    cmp     dword[dock_items.location], 2
    je	    .setleft

 .setright:
    call    .VERT_X_RIGHT
    jmp     .SETDEF

 .setleft:
    call    .VERT_X_LEFT
    jmp     .SETDEF

 .HORZ_WIDTH:
    mov     eax, 42
    mov     ebx, [dock_items.count]
    imul    eax, ebx
    dec     eax
    mov     [win.width_opn], eax
    mov     [win.width_hdn], eax

    ret

 .HORZ_X:
    mcall   14
    shr     eax, 16
    mov     ebx, 2
    mov     edx, 0
    div     ebx
    mov     edx, 0
    mov     ecx, eax

    mov     eax, [win.width_opn]
    div     ebx
    sub     ecx, eax
    mov     [win.x_opn], ecx
    mov     [win.x_hdn], ecx

    ret

 .HORZ_HEIGHT:
    mov     dword[win.height_opn], 42
    mov     dword[win.height_hdn], 0

    ret

 .HORZ_Y_BOTTOM:
    mcall   14
    and     eax, 0xFFFF
    dec     eax
    mov     [win.y_hdn], eax
    sub     eax, 42
    mov     [win.y_opn], eax

    ret

 .HORZ_Y_TOP:
    mov     dword[win.y_opn], 0
    mov     dword[win.y_hdn], 0

    ret

 .VERT_WIDTH:
    mov     dword[win.width_opn], 42
    mov     dword[win.width_hdn], 0

    ret

 .VERT_X_LEFT:

    mov     dword[win.x_opn], 0
    mov     dword[win.x_hdn], 0

    ret

 .VERT_X_RIGHT:
    mcall   14
    and     eax, 0xFFFF0000
    shr     eax, 16
    mov     [win.x_hdn], eax
    sub     eax, 42
    mov     [win.x_opn], eax

    ret

 .VERT_HEIGHT:
    mov     eax, 42
    mov     ebx, [dock_items.count]
    imul    eax, ebx
    dec     eax
    mov     [win.height_opn], eax
    mov     [win.height_hdn], eax

    ret

 .VERT_Y:
    mcall   14
    and     eax, 0xFFFF
    mov     edx, 0
    mov     ebx, 2
    div     ebx
    mov     esi, eax

    mov     eax, [win.height_opn]
    mov     edx, 0
    mov     ebx, 2
    div     ebx
    sub     esi, eax

    mov     [win.y_hdn], esi
    mov     [win.y_opn], esi

    ret

 .SETDEF:
    mov     eax, [win.width_hdn]
    mov     [win.width], eax

    mov     eax, [win.x_hdn]
    mov     [win.x], eax

    mov     eax, [win.height_hdn]
    mov     [win.height], eax

    mov     eax, [win.y_hdn]
    mov     [win.y], eax

; ==== START ====
    mcall   9, win.procinfo, -1
    mov     ecx, [win.procinfo + 30]
    mcall   18, 21
    and     eax, 0xFFFF
    mov     [win.sid], eax

    call    main_loop

exit:
    stdcall mem.Free, [img_data.rgb_object]
    mcall   18, 2, [nwin.sid]
    mcall   -1
endp
;-------------------------------------------------------------------------------
proc main_loop
    mcall   10

    cmp     eax, EV_REDRAW
    je	    event_redraw

    cmp     eax, EV_BUTTON
    je	    event_button

    cmp     eax, EV_MOUSE
    je	    event_mouse

    jmp     main_loop

 .end:
    ret
endp
;-------------------------------------------------------------------------------
proc event_redraw
    mcall   12, 1

    mcall   0, <[win.x], [win.width]>, <[win.y], [win.height]>, [color.frame], [color.frame], [color.frame]

    and     ebx, 0x0000FFFF
    add     ebx, 0x00010000
    sub     ebx, 0x00000001

    and     ecx, 0x0000FFFF
    add     ecx, 0x00010000
    sub     ecx, 0x00000001

    mcall   13, , , [color.framein]


    add     ebx, 0x00010000
    sub     ebx, 0x00000002

    add     ecx, 0x00010000
    sub     ecx, 0x00000002

    mcall   13, , , [color.bg]

    mov     edi, 0
  @@:
    cmp     edi, [dock_items.count]
    je	    @f

    push    edi
    imul    esi, edi, 42
    shl     esi, 16
    add     esi, 41
    cmp     byte[win.isvert], 1
    je	    .vert_btn
    mcall   8, esi, <0, 42>, 0x60000002, [color.bg]
    jmp     .endbtn
 .vert_btn:
    mcall   8, <0, 42>, esi, 0x60000002, [color.bg]
 .endbtn:
    pop     edi

    cmp     byte[dock_items.separator + edi], 1
    je	    .draw_separator
    jmp     .end_separator

 .draw_separator:
    push    ebx
    push    ecx
    mov     ebx, edi
    imul    ebx, 42
    add     ebx, 41
    shl     ebx, 16
    add     ebx, 1
    cmp     byte[win.isvert], 1
    je	    .vert_draw_sep
    mcall   13, , <0, 43>, [color.frame]
    sub     ebx, 0x00010000
    mcall   13, , <1, 41>, [color.framein]
    add     ebx, 0x00020000
    mcall   13, , <1, 41>, [color.framein]
    jmp     .end_inner_sep
 .vert_draw_sep:
    mov     ecx, ebx
    mcall   13, <0, 43>, , [color.frame]
    sub     ecx, 0x00010000
    mcall   13, <1, 41>, , [color.framein]
    add     ecx, 0x00020000
    mcall   13, <1, 41>, , [color.framein]
 .end_inner_sep:
    pop     ecx
    pop     ebx
 .end_separator:

    cmp     byte[win.isvert], 1
    je	    .vert_dig
    mov     edx, ebx
    and     edx, 0xFFFF0000
    add     edx, 0x00050005
    jmp     .digend
 .vert_dig:
    mov     edx, ecx
    and     edx, 0xFFFF0000
    shr     edx, 16
    add     edx, 0x00050005
 .digend:

    imul    ebx, edi, 4
    add     ebx, dock_items.icon
    mov     ebx, [ebx]
    imul    ebx, ICON_SIZE
    add     ebx, [img_data.rgb_object]

    mcall   7, , <32, 32>

    inc     edi
    jmp     @b
  @@:

    mcall   12, 2

    jmp     main_loop
endp
;-------------------------------------------------------------------------------
proc event_button
    mcall   17

    cmp     ah, 1
    je	    .button_close

    cmp     ah, 2
    je	    .button_dock

    jmp     @f

 .button_close:
    jmp     exit

 .button_dock:
    mov     edi, [win.button_index]
    imul    edi, 256

    mov     esi, edi
    add     esi, dock_items.path
    mov     dword[file_exec.file], esi

    mov     esi, edi
    add     esi, dock_items.param
    mov     dword[file_exec.param], esi

    mcall   70, file_exec

  @@:
    jmp     main_loop
endp
;-------------------------------------------------------------------------------
proc event_mouse
    mcall   37, 1
    mov     edi, eax
    mov     esi, eax
    shr     edi, 16
    and     esi, 0xFFFF

    cmp     edi, 0
    jl	    @f
    dec     edi
    cmp     edi, [win.width]
    jg	    @f
    cmp     esi, 0
    jl	    @f
    dec     esi
    cmp     esi, [win.height]
    jg	    @f

    mov     eax, [dock_items.location]
    and     eax, 1b
    cmp     eax, 1
    jne     .vert
    mov     eax, edi
    jmp     .nxt

 .vert:
    mov     eax, esi

 .nxt:
    mov     edx, 0
    mov     ebx, 42
    div     ebx

    cmp     eax, [dock_items.count]
    jge     .set0
    jmp     .nxtcmp

 .set0:
    mov     eax, 100

 .nxtcmp:
    cmp     [win.button_index], eax
    je	    .nxt2

    mov     [win.button_index], eax

 .nxt2:
    mov     eax, [win.button_index]
    imul    eax, 42
    cmp     byte[win.isvert], 1
    je	    .vert_name
    sub     eax, 13
    add     eax, [win.x]
    mov     [nwin.x], eax
    jmp     .vert_end
 .vert_name:
    add     eax, 13
    add     eax, [win.y]
    mov     [nwin.y], eax
 .vert_end:
    mov     byte[nwin.change_shape], 1

    cmp     byte[win.state], 1
    je	    .end_cmp

    mov     edx, esp
    add     edx, 512
    mcall   51, 1, n_main

    mcall   18, 7
    mov     [win.psid], eax

    mcall   18, 3, [win.sid]

    mov     byte[win.state], 1

    mov     eax, [win.width_opn]
    mov     [win.width], eax

    mov     eax, [win.x_opn]
    mov     [win.x], eax

    mov     eax, [win.height_opn]
    mov     [win.height], eax

    mov     eax, [win.y_opn]
    mov     [win.y], eax

    mcall   67, [win.x], [win.y], [win.width], [win.height]

    call     event_redraw

  @@:
    cmp     byte[win.state], 0
    je	    .end_cmp

    mov     byte[nwin.close], 1

    mcall   18, 3, [win.psid]

    mov     byte[win.state], 0
    mov     byte[win.button_index], -1

    mov     eax, [win.width_hdn]
    mov     [win.width], eax

    mov     eax, [win.x_hdn]
    mov     [win.x], eax

    mov     eax, [win.height_hdn]
    mov     [win.height], eax

    mov     eax, [win.y_hdn]
    mov     [win.y], eax

    mcall   67, [win.x], [win.y], [win.width], [win.height]

    call     event_redraw

  .end_cmp:
    jmp     main_loop
endp
;-------------------------------------------------------------------------------
proc sections_callback, _file_name, _section_name
    mov     eax, [_section_name]
    cmp     byte[eax], '@'
    jne     @f

    dec     dword[dock_items.count]
    jmp     .endproc

  @@:
    ; ==== GET NAME ====
    mov     ebx, [dock_items.count]
    imul    ebx, 16
    add     ebx, dock_items.name

    mov     eax, [_section_name]

    mov     edi, 0
  @@:
    mov     cl, byte[eax]
    mov     byte[ebx + edi], cl

    inc     eax
    inc     edi
    cmp     edi, 10
    jne     @b

  ; ==== GET PATH ====
    mov     ebx, [dock_items.count]
    imul    ebx, 256
    add     ebx, dock_items.path

    invoke  ini.get_str, [_file_name], [_section_name], ini_data.path_name, ebx, 256, 0

  ; === GET  PARAM ===
    mov     ebx, [dock_items.count]
    imul    ebx, 256
    add     ebx, dock_items.param

    invoke  ini.get_str, [_file_name], [_section_name], ini_data.param_name, ebx, 256, 0

  ; ==== GET ICON ====
    invoke  ini.get_int, [_file_name], [_section_name], ini_data.icon_name, 0

    mov     ebx, [dock_items.count]
    imul    ebx, 4
    mov     [dock_items.icon + ebx], eax

  ; ==== GET SEPARATOR ====
    invoke  ini.get_int, [_file_name], [_section_name], ini_data.separator_name, 0

    mov     ebx, [dock_items.count]
    mov     byte[dock_items.separator + ebx], al

  ; ====== END =======
 .endproc:
    mov     eax, 1
    inc     dword[dock_items.count]
    ret
endp
;-------------------------------------------------------------------------------
n_main:
    cmp     dword[dock_items.location], 1
    je	    .top
    cmp     dword[dock_items.location], 4
    je	    .right
    cmp     dword[dock_items.location], 3
    je	    .bottom
    jmp     .left
 .top:
    mov     eax, [win.height_opn]
    add     eax, 5
    mov     [nwin.y], eax
    jmp     @f
 .right:
    mov     eax, [win.x_opn]
    sub     eax, 73
    mov     [nwin.x], eax
    jmp     @f
 .bottom:
    mov     eax, [win.y_opn]
    sub     eax, 21
    mov     [nwin.y], eax
    jmp     @f
 .left:
    mov     eax, [win.width_opn]
    add     eax, 5
    mov     [nwin.x], eax
  @@:
    mov     dword[nwin.width], 68
    mov     dword[nwin.height], 16

    mcall   40, 1b

    mcall   9, win.procinfo, -1
    mov     ecx, [win.procinfo + 30]
    mcall   18, 21
    and     eax, 0xFFFF
    mov     [nwin.sid], eax
;-------------------------------------------------------------------------------
n_main_loop:
    mcall   23, 1

    cmp     eax, EV_IDLE
    je	    n_event_idle
    cmp     eax, EV_REDRAW
    je	    n_event_redraw

    jmp     n_main_loop
;-------------------------------------------------------------------------------
n_event_idle:
    cmp     byte[nwin.close], 1
    jne     @f

    mov     byte[nwin.close], 0
    mcall   -1

  @@:
    cmp     byte[win.button_index], 100
    jne     @f

    mcall   67, 0, 0, 0, 0
    jmp     .end

  @@:
    cmp     byte[nwin.change_shape], 1
    jne     .end

    mov     byte[nwin.change_shape], 0
    mcall   67, [nwin.x], [nwin.y], [nwin.width], [nwin.height]

 .end:
    jmp     n_main_loop
;-------------------------------------------------------------------------------
n_event_redraw:
    mcall   12, 1

    mcall   0, <[nwin.x], [nwin.width]>, <[nwin.y], [nwin.height]>, [color.frame], [color.frame], [color.frame]

    and     ebx, 0x0000FFFF
    add     ebx, 0x00010000
    sub     ebx, 0x00000001

    and     ecx, 0x0000FFFF
    add     ecx, 0x00010000
    sub     ecx, 0x00000001

    mcall   13, , , [color.framein]


    add     ebx, 0x00010000
    sub     ebx, 0x00000002

    add     ecx, 0x00010000
    sub     ecx, 0x00000002

    mcall   13, , , [color.bg]

    mov     edx, [win.button_index]
    imul    edx, 16
    add     edx, dock_items.name

    mov     eax, 0
  @@:
    inc     eax
    cmp     byte[edx+eax], 0
    jne     @b

    imul    eax, 3
    mov     ebx, 34
    sub     ebx, eax
    inc     ebx
    shl     ebx, 16
    add     ebx, 5

    mcall   4, , [color.text]

    mcall   12, 2

    jmp     n_main_loop
;-------------------------------------------------------------------------------
img_data:
 .file_name:
    db	    "/sys/iconstrp.png", 0
 .cfg_text:
    db	    "R", 0
 .ext_text:
    db	    "X", 0
;-------------------------------------------------------------------------------
ini_data:
 .file_name:
    db	    "/sys/settings/Docky.ini", 0
 .skin_dir:
    db	    "/sys/settings/Docky skins/"
 .skin_file:
    db	    32 dup(0)
 .path_name:
    db	    "path", 0
 .param_name:
    db	    "param", 0
 .icon_name:
    db	    "icon", 0
 .separator_name:
    db	    "separator", 0

 .settings_name:
    db	    "@SETTINGS", 0
 .location_name:
    db	    "location", 0
 .skin_name:
    db	    "skin", 0
 .color_bg:
    db	    "bg", 0
 .color_frame:
    db	    "frame", 0
 .color_framein:
    db	    "framein", 0
 .color_text:
    db	    "text", 0
;-------------------------------------------------------------------------------
load_lib_start:
    lib1    l_libs img.name,	     \
		   sys_path,	     \
		   file_name,	     \
		   img.dir,	     \
		   error,	     \
		   error,	     \
		   img, 	     \
		   error,	     \
		   error

    lib2    l_libs ini.name,	     \
		   sys_path,	     \
		   file_name,	     \
		   ini.dir,	     \
		   error,	     \
		   error,	     \
		   ini, 	     \
		   error,	     \
		   error
load_lib_end:
;-------------------------------------------------------------------------------
img:
 .init	   \
    dd	    .init_T
 .toRGB    \
    dd	    .toRGB_T
 .decode   \
    dd	    .decode_T
 .destroy  \
    dd	    .destroy_T

    dd	    0, 0

 .init_T:
    db	    "lib_init", 0
 .toRGB_T:
    db	    "img_to_rgb2", 0
 .decode_T:
    db	    "img_decode", 0
 .destroy_T:
    db	    "img_destroy", 0

 .dir:
    db	    "/sys/lib/"
 .name:
    db	    "libimg.obj", 0
;-------------------------------------------------------------------------------
ini:
 .init	   \
    dd	    .init_T
 .sections \
    dd	    .sections_T
 .get_int  \
    dd	    .get_int_T
 .get_str  \
    dd	    .get_str_T
 .get_color\
    dd	    .get_color_T

    dd	    0, 0

 .init_T:
    db	    "lib_init", 0
 .sections_T:
    db	    "ini_enum_sections", 0
 .get_int_T:
    db	    "ini_get_int", 0
 .get_str_T:
    db	    "ini_get_str", 0
 .get_color_T:
    db	    "ini_get_color", 0

 .dir:
    db	    "/sys/lib/"
 .name:
    db	    "libini.obj", 0
;-------------------------------------------------------------------------------
error:
    db	    0
;-------------------------------------------------------------------------------
__dataend:
;================================================================================
    rb	    1024
__stackend:
;================================================================================
color:
 .bg:
    rd	    1
 .frame:
    rd	    1
 .framein:
    rd	    1
 .text:
    rd	    1
;-------------------------------------------------------------------------------
win:
 .x:
    rd	    1
 .y:
    rd	    1
 .width:
    rd	    1
 .height:
    rd	    1

 .x_hdn:
    rd	    1
 .y_hdn:
    rd	    1
 .width_hdn:
    rd	    1
 .height_hdn:
    rd	    1

 .x_opn:
    rd	    1
 .y_opn:
    rd	    1
 .width_opn:
    rd	    1
 .height_opn:
    rd	    1

 .sid:
    rd	    1
 .psid:
    rd	    1
 .procinfo:
    rb	    1024
 .state:
    rb	    1
 .button_index:
    rd	    1

 .isvert:
    rb	    1
;-------------------------------------------------------------------------------
nwin:
 .x:
    rd	    1
 .y:
    rd	    1
 .width:
    rd	    1
 .height:
    rd	    1

 .visible:
    rd	    1
 .sid:
    rd	    1
 .change_shape:
    rb	    1
 .close:
    rb	    1
;-------------------------------------------------------------------------------
img_data.object:
    rd	    1
img_data.rgb_object:
    rd	    1
img_data.file:
 .proc:
    rd	    1
 .position:
    rd	    1
    rd	    1
 .size:
    rd	    1
 .buffer:
    rd	    1
    rb	    1
 .name:
    rd	    1
;-------------------------------------------------------------------------------
file_exec:
 .proc:
    rd	    1
    rd	    1
 .param:
    rd	    1
    rd	    1
    rd	    1
    rb	    1
 .file:
    rd	    1
;-------------------------------------------------------------------------------
dock_items:
 .count:
    rd	    1
 .name:
    rb	    16	* 20
 .path:
    rb	    256 * 20
 .param:
    rb	    256 * 20
 .icon:
    rd	    1	* 20
 .separator:
    rb	    1	* 20
 .location:
    rd	    1
;-------------------------------------------------------------------------------
sys_path:
    rb	    4096
file_name:
    rb	    4096
;-------------------------------------------------------------------------------
__memend:
;================================================================================