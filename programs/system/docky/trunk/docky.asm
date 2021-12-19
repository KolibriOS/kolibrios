    use32
    org     0x0
;-------------------------------------------------------------------------------
    db	    "MENUET01"
    dd	    1, main, __dataend, __memend, __stackend, 0, sys_path
;-------------------------------------------------------------------------------
    include "../../../macros.inc"
    include "../../../proc32.inc"
    include "../../../dll.inc"
	include "../../../KOSfuncs.inc"
    include "../../../load_lib.mac"
    ;include "../../../debug.inc"

    include "DATA.INC"
    include "NAME.INC"

    @use_library	 \
	    mem.Alloc,	 \
	    mem.Free,	 \
	    mem.ReAlloc, \
	    dll.Load
;================================================================================
main:
; ==== Init ====
    mcall   SF_SYSTEM, SSF_GET_ACTIVE_WINDOW
    mov     [win.psid], eax

    mcall   SF_SET_EVENTS_MASK, EVM_REDRAW+EVM_BUTTON+EVM_MOUSE ;+EVM_DEKSTOP to update colors on skin change

; ==== Load libs ====
    load_libraries load_lib_start, load_lib_end

; ==== Config LibINI ====
    invoke  ini.get_int, ini_data.file_name, ini_data.settings_name, ini_data.location_name, 2
    test    eax,eax
	jz      exit
    mov     [dock_items.location], eax
    invoke  ini.get_int, ini_data.file_name, ini_data.settings_name, ini_data.fsize_name, 0
    mov     [dock_items.fsize], eax
    invoke  ini.get_int, ini_data.file_name, ini_data.settings_name, ini_data.ashow_name, 0
    mov     [dock_items.ashow], eax

    invoke  ini.sections, ini_data.file_name, sections_callback

; ==== Load colors ====
    mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, color
    or	    dword[color.bg],	0x10000000
    or	    dword[color.frame], 0x10000000
    or	    dword[color.text],	0x80000000

; ==== Config LibIMG ====
    mov     dword[fi.p00], SSF_GET_INFO
    mov     dword[fi.p16], buf_128
    mov     dword[fi.p21], img_data.file_name

    mcall   SF_FILE, fi

    mov     edx, [buf_128 + 32]
    imul    edx, 10

    stdcall mem.Alloc, edx
    mov     [img_data.rgb_object], eax

    mov     dword[fi.p00], SSF_READ_FILE
    mov     dword[fi.p12], edx
    m2m     dword[fi.p16], dword[img_data.rgb_object]
    mov     dword[fi.p21], img_data.file_name

    mcall   SF_FILE, fi

    cmp     ebx, 0xFFFFFFFF
    je	    @f

    stdcall dword[img.decode], dword[img_data.rgb_object], ebx, 0
    mov     dword[img_data.object], eax

  ; === ALPHA ===
    mov     edi, eax
    add     edi, 8
    mov     edi, [edi]
    imul    edi, 128
    sub     edi, 4

    add     eax, 24
    mov     eax, [eax]
 .setalpha:
    mov     ebx, [eax + edi]
    shr     ebx, 24
    cmp     ebx, 0
    jne     .nonalpha

    mov     ecx, [color.bg]
    mov     [eax + edi], ecx
 .nonalpha:
    sub     edi, 4
    cmp     edi, 0
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

;-------------------------------------------------------------------------------
 .HORZ_WIDTH:
    cmp     [dock_items.fsize], byte 1
    je	    @f
    mov     eax, BUTTON_SIZE
    mov     ebx, [dock_items.count]
    imul    eax, ebx
    add     eax, 24
    dec     eax
    jmp     .set_hw
  @@:
    mcall   SF_GET_SCREEN_SIZE
    shr     eax, 16
 .set_hw:
    mov     [win.width_opn], eax
    mov     [win.width_hdn], eax

    ret

;-------------------------------------------------------------------------------
 .HORZ_X:
    mcall   SF_GET_SCREEN_SIZE
    shr     eax, 17
    mov     ecx, [win.width_opn]
    shr     ecx, 1
    sub     eax, ecx
    mov     [win.x_opn], eax
    mov     [win.x_hdn], eax

    ret

;-------------------------------------------------------------------------------
 .HORZ_HEIGHT:
    mov     dword[win.height_hdn], 3
    mov     dword[win.height_opn], BUTTON_SIZE

    ret

;-------------------------------------------------------------------------------
 .HORZ_Y_BOTTOM:
    mcall   SF_GET_SCREEN_SIZE
    and     eax, 0xFFFF
    dec     eax
    mov     [win.y_hdn], eax
    sub     eax, 43
    mov     [win.y_opn], eax

    ret

 .HORZ_Y_TOP:
    mov     dword[win.y_opn], 0
    mov     dword[win.y_hdn], 0

    ret

;-------------------------------------------------------------------------------
 .VERT_WIDTH:
    mov     dword[win.width_opn], BUTTON_SIZE
    mov     dword[win.width_hdn], 3

    ret

;-------------------------------------------------------------------------------
 .VERT_X_LEFT:
    mov     dword[win.x_opn], 0
    mov     dword[win.x_hdn], 0

    ret

 .VERT_X_RIGHT:
    mcall   SF_GET_SCREEN_SIZE
    and     eax, 0xFFFF0000
    shr     eax, 16
    mov     [win.x_hdn], eax
    sub     eax, BUTTON_SIZE
    mov     [win.x_opn], eax

    ret

;-------------------------------------------------------------------------------
 .VERT_HEIGHT:
    cmp     [dock_items.fsize], byte 1
    je	    @f
    mov     eax, BUTTON_SIZE
    mov     ebx, [dock_items.count]
    imul    eax, ebx
    dec     eax
    jmp     .set_vh
  @@:
    mcall   SF_GET_SCREEN_SIZE
    and     eax, 0xFFFF
 .set_vh:
    mov     [win.height_opn], eax
    mov     [win.height_hdn], eax

    ret

;-------------------------------------------------------------------------------
 .VERT_Y:
    mcall   SF_GET_SCREEN_SIZE
    and     eax, 0xFFFF
    shr     eax, 1

    mov     esi, [win.height_opn]
    shr     esi, 1
    sub     eax, esi

    mov     [win.y_hdn], eax
    mov     [win.y_opn], eax

    ret

;-------------------------------------------------------------------------------
 .SETDEF:
    mov     eax, [win.width_hdn]
    mov     [win.width], eax

    mov     eax, [win.x_hdn]
    mov     [win.x], eax

    mov     eax, [win.height_hdn]
    mov     [win.height], eax

    mov     eax, [win.y_hdn]
    mov     [win.y], eax
	
    cmp     byte[dock_items.ashow],1
    jne     .not_ashow

    mov     eax, [win.width_opn]
    mov     [win.width], eax

    mov     eax, [win.x_opn]
    mov     [win.x], eax

    mov     eax, [win.height_opn]
    mov     [win.height], eax

    mov     eax, [win.y_opn]
    mov     [win.y], eax

 .not_ashow:


;-------------------------------------------------------------------------------
; ==== START ====
    mcall   SF_THREAD_INFO, win.procinfo, -1
    mov     ecx, [win.procinfo + 30]
    mcall   SF_SYSTEM, SSF_GET_THREAD_SLOT
    and     eax, 0xFFFF
    mov     [win.sid], eax

    call    main_loop
;-------------------------------------------------------------------------------
exit:
    stdcall mem.Free, [img_data.rgb_object]
    mcall   SF_SYSTEM, SSF_TERMINATE_THREAD, [nwin.sid]
    mcall   SF_TERMINATE_PROCESS
;-------------------------------------------------------------------------------
align 4
main_loop:
    mcall   SF_WAIT_EVENT

    cmp     eax, EV_REDRAW
    je	    event_redraw

    cmp     eax, EV_BUTTON
    je	    event_button

    cmp     eax, EV_MOUSE
    je	    event_mouse

    jmp     main_loop
;-------------------------------------------------------------------------------
event_redraw:
    call    DRAW_WINDOW
    jmp     main_loop
;-------------------------------------------------------------------------------
DRAW_WINDOW:
    mcall   SF_REDRAW, SSF_BEGIN_DRAW

    mov     esi, [color.bg]
    or	    esi, 0x01000000
    mcall   SF_CREATE_WINDOW, <[win.x], [win.width]>, <[win.y], [win.height]>, [color.bg], , [color.frame]

    xor     edi, edi
  @@:
    cmp     edi, [dock_items.count]
    je	    @f

    push    edi
    mov     eax, SF_DEFINE_BUTTON
    mov     edx, 0x60000002
    mov     esi, [color.bg]
    imul    edi, BUTTON_SIZE
    add     edi, 12
    shl     edi, 16
    add     edi, BUTTON_SIZE
    cmp     byte[win.isvert], 1
    je	    .vert_btn
    mcall   , edi, <0, BUTTON_SIZE>
    jmp     .endbtn
 .vert_btn:
    sub     edi, 12 shl 16
    mcall   , <0, BUTTON_SIZE>, edi
 .endbtn:
    pop     edi

    cmp     byte[dock_items.separator + edi], 1
    jne     .end_separator

 .draw_separator:
    push    ebx
    push    ecx

    mov     eax, SF_DRAW_RECT
    mov     ebx, edi
    imul    ebx, BUTTON_SIZE
    add     ebx, BUTTON_SIZE
    add     ebx, 12-1
    shl     ebx, 16
    add     ebx, 2

    cmp     byte[win.isvert], 1
    je	    .vert_draw_sep
    mcall   , , <4,  36>, [color.frame]
    jmp     .end_inner_sep
 .vert_draw_sep:
    sub     ebx, 12 shl 16
    mov     ecx, ebx
    mcall   , <4, 36>, , [color.frame]
 .end_inner_sep:
    pop     ecx
    pop     ebx
 .end_separator:

    cmp     byte[win.isvert], 1
    je	    .vert_dig
    mov     edx, ebx
    and     edx, 0xFFFF0000
    add     edx, 0x00060006
    jmp     .digend
 .vert_dig:
    mov     edx, ecx
    and     edx, 0xFFFF0000
    shr     edx, 16
    add     edx, 0x00060006
 .digend:

    imul    ebx, edi, 4
    add     ebx, dock_items.icon
    mov     ebx, [ebx]
    imul    ebx, ICON_SIZE_BGR
    add     ebx, [img_data.rgb_object]

    mcall   SF_PUT_IMAGE, , <32, 32>

    inc     edi
    jmp     @b
  @@:

    mcall   SF_REDRAW, SSF_END_DRAW

    ret
;-------------------------------------------------------------------------------
event_button:
    mcall   SF_GET_BUTTON

	;; it must not be possible to close dock
    ;cmp     ah, 1
    ;je	    .button_close

    cmp     ah, 2
    je	    .button_dock

    jmp     @f

 ;.button_close:
 ;   jmp     exit

 .button_dock:
    mov     edi, [win.button_index]
    imul    edi, 256

    mov     dword[fi.p00], SSF_START_APP

    mov     esi, edi
    add     esi, dock_items.path
    mov     dword[fi.p21], esi

    mov     esi, edi
    add     esi, dock_items.param
    mov     dword[fi.p08], esi

    mcall   SF_FILE, fi

    mov     ecx, eax
    mcall   SF_SYSTEM, SSF_GET_THREAD_SLOT
    and     eax, 0xFFFF
    mov     [win.psid], eax

    jmp     wnd_hide

  @@:
    jmp     main_loop
;-------------------------------------------------------------------------------
event_mouse:
  ; ==== IS MOUSE INNER ====
    mcall   SF_MOUSE_GET, SSF_WINDOW_POSITION
    mov     edi, eax
    mov     esi, eax
    shr     edi, 16
    and     esi, 0xFFFF

    cmp     edi, 0
    jl	    wnd_hide
    dec     edi
    cmp     edi, [win.width]
    jg	    wnd_hide
    cmp     esi, 0
    jl	    wnd_hide
    dec     esi
    cmp     esi, [win.height]
    jg	    wnd_hide

  ; ==== COUNT INDEX ====

    mov     eax, [dock_items.location]
    and     eax, 1b
    cmp     eax, 1
    jne     .vert
    mov     eax, edi
    jmp     .nxt

 .vert:
    mov     eax, esi
    add     eax, 12

 .nxt:
    sub     eax, 12
    mov     edx, 0
    mov     ebx, BUTTON_SIZE
    div     ebx

    cmp     eax, [dock_items.count]
    jge     .set0
    jmp     .nxtcmp

 .set0:
    mov     eax, 100

 .nxtcmp:
    cmp     [win.button_index], eax
    je	    .nxt2

    push    dword[win.button_index]
    pop     dword[win.prev_index]

    mov     [win.button_index], eax

 ; ==== DRAW SELECTION ====
    call    DRAW_SELECTION

 .nxt2:
    mov     eax, [win.button_index]
    imul    eax, BUTTON_SIZE
    cmp     byte[win.isvert], 1
    je	    .vert_name
    add     eax, [win.x]
    mov     [nwin.x], eax
    mov     byte[nwin.change_shape], 1
    mcall   SF_DRAW_RECT, <0, [win.width]>, <[win.height], 1>, [color.frame]
    jmp     .vert_end
 .vert_name:
    add     eax, [win.y]
    add     eax, 14
    mov     [nwin.y], eax
    mov     byte[nwin.change_shape], 1
    mcall   SF_DRAW_RECT, <[win.width], 1>, <0, [win.height]>, [color.frame]
 .vert_end:

 ; ==== OPEN/CLOSE WINDOW ====
    cmp     byte[win.state], 1
    je	    main_loop

    mov     edx, esp
    add     edx, 512
    mcall   SF_CREATE_THREAD, 1, n_main

    mcall   SF_SYSTEM, SSF_GET_ACTIVE_WINDOW
    mov     [win.psid], eax

    mcall   SF_SYSTEM, SSF_FOCUS_WINDOW, [win.sid]

    mov     byte[win.state], 1

    mov     eax, [win.width_opn]
    mov     [win.width], eax

    mov     eax, [win.x_opn]
    mov     [win.x], eax

    mov     eax, [win.height_opn]
    mov     [win.height], eax

    mov     eax, [win.y_opn]
    mov     [win.y], eax


    cmp     byte[dock_items.ashow],1
    je	   .change_nothing
    mcall   SF_CHANGE_WINDOW, [win.x], [win.y], [win.width], [win.height]

  .change_nothing:
    call    DRAW_WINDOW
    call    DRAW_SELECTION
    jmp     main_loop

;-------------------------------------------------------------------------------
wnd_hide:
    cmp     byte[win.state], 0
    je	    main_loop

    mov     byte[nwin.close], 1

    mcall   SF_SYSTEM, SSF_FOCUS_WINDOW, [win.psid]

    mov     byte[win.state], 0
    mov     byte[win.button_index], -1

    cmp     byte[dock_items.ashow],1
    je	   .do_no_hide

    mov     eax, [win.width_hdn]
    mov     [win.width], eax

    mov     eax, [win.x_hdn]
    mov     [win.x], eax

    mov     eax, [win.height_hdn]
    mov     [win.height], eax

    mov     eax, [win.y_hdn]
    mov     [win.y], eax

    mcall   SF_CHANGE_WINDOW, [win.x], [win.y], [win.width], [win.height]

  .do_no_hide:
    call    DRAW_WINDOW
    jmp     main_loop
;-------------------------------------------------------------------------------
DRAW_SELECTION:
    mov     ebx, [win.prev_index]
    imul    ebx, BUTTON_SIZE
    add     ebx, 14
    shl     ebx, 16
    add     ebx, 40
    mov     ecx, 0x00020028

    cmp     byte[win.isvert], 1
    jne     @f
    xchg    ebx, ecx
    sub     ecx, 0x000C0000
  @@:

    mcall   SF_DRAW_RECT, , , [color.bg]

    mov     edx, ebx
    shr     ecx, 16
    mov     dx, cx
    add     edx, 0x00040004

    mov     ebx, [win.prev_index]
    imul    ebx, 4
    add     ebx, dock_items.icon
    mov     ebx, [ebx]
    imul    ebx, ICON_SIZE_BGR
    add     ebx, [img_data.rgb_object]

    mcall   SF_PUT_IMAGE, , <32, 32>

    mov     ebx, [win.button_index]
    imul    ebx, BUTTON_SIZE
    add     ebx, 14
    shl     ebx, 16
    add     ebx, 40
    mov     ecx, 0x00020028

    cmp     byte[win.isvert], 1
    jne     @f
    xchg    ebx, ecx
    sub     ecx, 0x000C0000
  @@:

    mcall   SF_DRAW_RECT, , , [color.bt]

    mov     edx, ebx
    shr     ecx, 16
    mov     dx, cx
    add     edx, 0x00040004

    mov     ecx, [win.button_index]
    imul    ecx, 4
    add     ecx, dock_items.icon
    mov     ecx, [ecx]
    imul    ecx, ICON_SIZE_BGR
    add     ecx, [img_data.rgb_object]

    mov     ebx, sel_img

    mov     edi, 0
  @@:
    mov     al, byte[ecx + 2]
    shl     eax, 8
    mov     al, byte[ecx + 1]
    shl     eax, 8
    mov     al, byte[ecx + 0]

    or	    eax, 0x10000000
    cmp     eax, [color.bg]
    jne     .notbg
    mov     eax, [color.bt]
 .notbg:

    mov     byte[ebx + 0], al
    shr     eax, 8
    mov     byte[ebx + 1], al
    shr     eax, 8
    mov     byte[ebx + 2], al

    add     ebx, 3
    add     ecx, 3

    add     edi, 3

    cmp     edi, 1024 * 3
    jne     @b

    mcall   SF_PUT_IMAGE, sel_img, <32, 32>

    ret
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
    include "MEMORY.INC"