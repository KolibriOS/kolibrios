;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;==============================================================================
;///// public functions ///////////////////////////////////////////////////////
;==============================================================================

macro FuncTable name, [label]
{
  common
    align 4
    \label name#.ftable dword
  forward
    dd name#.#label
  common
    name#.sizeof.ftable = $ - name#.ftable
}

iglobal
  FuncTable syscall_display_settings, \
    00, 01, 02, 03, 04, 05, 06, 07, 08
endg

uglobal
  common_colours            rd 32
  new_window_starting       dd ?
  latest_window_touch       dd ?
  latest_window_touch_delta dd ?
  old_window_pos            BOX
  new_window_pos            BOX
  draw_limits               RECT
  bPressedMouseXY_W         db ?
  do_resize                 db ?
  do_resize_from_corner     db ?
  reposition                db ?
endg

align 4
;------------------------------------------------------------------------------
syscall_display_settings: ;///// system function 48 ///////////////////////////
;------------------------------------------------------------------------------
;; Redraw screen:
;< ebx = 0
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;; Set button style:
;< ebx = 1
;< ecx = 0 (flat) or 1 (with gradient)
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;; Set system color palette:
;< ebx = 2
;< ecx = pointer to color table
;< edx = size of color table
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;; Get system color palette:
;< ebx = 3
;< ecx = pointer to color table buffer
;< edx = size of color table buffer
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;; Get skinned caption height:
;< ebx = 4
;> eax = height in pixels
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;; Get screen working area:
;< ebx = 5
;> eax = pack[16(left), 16(right)]
;> ebx = pack[16(top), 16(bottom)]
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;; Set screen working area:
;< ebx = 6
;< ecx = pack[16(left), 16(right)]
;< edx = pack[16(top), 16(bottom)]
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;; Get skin margins:
;< ebx = 7
;> eax = pack[16(left), 16(right)]
;> ebx = pack[16(top), 16(bottom)]
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;; Set skin:
;< ebx = 8
;< ecx = pointer to FileInfoBlock struct
;> eax = FS error code
;------------------------------------------------------------------------------
        cmp     ebx, .sizeof.ftable / 4
        ja      @f
        jmp     [.ftable + ebx * 4]
    @@: ret


align 4
syscall_display_settings.00:
        xor     eax, eax
        inc     ebx
        cmp     [windowtypechanged], ebx
        jne     .exit
        mov     [windowtypechanged], eax

        jmp     syscall_display_settings._.redraw_whole_screen

  .exit:
        ret

align 4
syscall_display_settings.01:
        and     ecx, 1
        cmp     ecx, [buttontype]
        je      .exit
        mov     [buttontype], ecx
        mov     [windowtypechanged], ebx

  .exit:
        ret

align 4
syscall_display_settings.02:
        dec     ebx
        mov     esi, ecx
        and     edx, 127
        mov     edi, common_colours
        mov     ecx, edx
        rep     movsb
        mov     [windowtypechanged], ebx
        ret

align 4
syscall_display_settings.03:
        mov     edi, ecx
        and     edx, 127
        mov     esi, common_colours
        mov     ecx, edx
        rep     movsb
        ret

align 4
syscall_display_settings.04:
        mov     eax, [_skinh]
        mov     [esp + 32], eax
        ret

align 4
syscall_display_settings.05:
        mov     eax, [screen_workarea.left - 2]
        mov     ax, word[screen_workarea.right]
        mov     [esp + 32], eax
        mov     eax, [screen_workarea.top - 2]
        mov     ax, word[screen_workarea.bottom]
        mov     [esp + 20], eax
        ret

align 4
syscall_display_settings.06:
        xor     esi, esi

        mov     edi, [Screen_Max_X]
        mov     eax, ecx
        movsx   ebx, ax
        sar     eax, 16
        cmp     eax, ebx
        jge     .check_horizontal
        inc     esi
        or      eax, eax
        jge     @f
        xor     eax, eax
    @@: mov     [screen_workarea.left], eax
        cmp     ebx, edi
        jle     @f
        mov     ebx, edi
    @@: mov     [screen_workarea.right], ebx

  .check_horizontal:
        mov     edi, [Screen_Max_Y]
        mov     eax, edx
        movsx   ebx, ax
        sar     eax, 16
        cmp     eax, ebx
        jge     .check_if_redraw_needed
        inc     esi
        or      eax, eax
        jge     @f
        xor     eax, eax
    @@: mov     [screen_workarea.top], eax
        cmp     ebx, edi
        jle     @f
        mov     ebx, edi
    @@: mov     [screen_workarea.bottom], ebx

  .check_if_redraw_needed:
        or      esi, esi
        jz      .exit

        call    repos_windows
        jmp     syscall_display_settings._.calculate_whole_screen

  .exit:
        ret

align 4
syscall_display_settings.07:
        mov     eax, [_skinmargins + 0]
        mov     [esp + 32], eax
        mov     eax, [_skinmargins + 4]
        mov     [esp + 20], eax
        ret

align 4
syscall_display_settings.08:
        mov     ebx, ecx
        call    read_skin_file
        mov     [esp + 32], eax
        test    eax, eax
        jnz     .exit

        call    syscall_display_settings._.calculate_whole_screen
        jmp     syscall_display_settings._.redraw_whole_screen

  .exit:
        ret

syscall_display_settings._.calculate_whole_screen:
        xor     eax, eax
        xor     ebx, ebx
        mov     ecx, [Screen_Max_X]
        mov     edx, [Screen_Max_Y]
        jmp     calculatescreen

syscall_display_settings._.redraw_whole_screen:
        xor     eax, eax
        mov     [draw_limits.left], eax
        mov     [draw_limits.top], eax
        mov     eax, [Screen_Max_X]
        mov     [draw_limits.right], eax
        mov     eax, [Screen_Max_Y]
        mov     [draw_limits.bottom], eax
        mov     eax, window_data
        jmp     redrawscreen

align 4
;------------------------------------------------------------------------------
syscall_set_window_shape: ;///// system function 50 ///////////////////////////
;------------------------------------------------------------------------------
;; Set window shape address:
;> ebx = 0
;> ecx = shape data address
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;; Set window shape scale:
;> ebx = 1
;> ecx = scale power (resulting scale is 2^ebx)
;------------------------------------------------------------------------------
        mov     edi, [current_slot]

        test    ebx, ebx
        jne     .shape_scale
        mov     [edi + APPDATA.wnd_shape], ecx

  .shape_scale:
        dec     ebx
        jnz     .exit
        mov     [edi + APPDATA.wnd_shape_scale], ecx

  .exit:
        ret

align 4
;------------------------------------------------------------------------------
set_window_defaults: ;/////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        push    eax ecx
        xor     eax, eax
        mov     ecx, WIN_STACK
    @@: inc     eax
        add     ecx, 2
        ; process no
        mov     [ecx + 0x000], ax
        ; positions in stack
        mov     [ecx + 0x400], ax
        cmp     ecx, WIN_POS - 2
        jne     @b
        pop     ecx eax
        ret

align 4
;------------------------------------------------------------------------------
calculatescreen: ;/////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Scan all windows from bottom to top, calling `setscreen` for each one
;? intersecting given screen area
;------------------------------------------------------------------------------
;> eax = left
;> ebx = top
;> ecx = right
;> edx = bottom
;------------------------------------------------------------------------------
        push    esi
        pushfd
        cli

        mov     esi, 1
        call    window._.set_screen

        push    ebp

        mov     ebp, [TASK_COUNT]
        cmp     ebp, 1
        jbe     .exit

        push    edx ecx ebx eax

  .next_window:
        movzx   edi, word[WIN_POS + esi * 2]
        shl     edi, 5

        cmp     [CURRENT_TASK + edi + TASKDATA.state], TSTATE_FREE
        je      .skip_window

        add     edi, window_data
        test    [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz     .skip_window

        mov     eax, [edi + WDATA.box.left]
        cmp     eax, [esp + RECT.right]
        jg      .skip_window
        mov     ebx, [edi + WDATA.box.top]
        cmp     ebx, [esp + RECT.bottom]
        jg      .skip_window
        mov     ecx, [edi + WDATA.box.width]
        add     ecx, eax
        cmp     ecx, [esp + RECT.left]
        jl      .skip_window
        mov     edx, [edi + WDATA.box.height]
        add     edx, ebx
        cmp     edx, [esp + RECT.top]
        jl      .skip_window

        cmp     eax, [esp + RECT.left]
        jae     @f
        mov     eax, [esp + RECT.left]
    @@: cmp     ebx, [esp + RECT.top]
        jae     @f
        mov     ebx, [esp + RECT.top]
    @@: cmp     ecx, [esp + RECT.right]
        jbe     @f
        mov     ecx, [esp + RECT.right]
    @@: cmp     edx, [esp + RECT.bottom]
        jbe     @f
        mov     edx, [esp + RECT.bottom]

    @@: push    esi
        movzx   esi, word[WIN_POS + esi * 2]
        call    window._.set_screen
        pop     esi

  .skip_window:
        inc     esi
        dec     ebp
        jnz     .next_window

        pop     eax ebx ecx edx

  .exit:
        pop     ebp
        popfd
        pop     esi
        ret

align 4
;------------------------------------------------------------------------------
repos_windows: ;///////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        mov     ecx, [TASK_COUNT]
        mov     edi, window_data + WDATA.sizeof * 2
        call    force_redraw_background
        dec     ecx
        jle     .exit

  .next_window:
        mov     [edi + WDATA.fl_redraw], 1
        test    [edi + WDATA.fl_wstate], WSTATE_MAXIMIZED
        jnz     .fix_maximized

        mov     eax, [edi + WDATA.box.left]
        add     eax, [edi + WDATA.box.width]
        mov     ebx, [Screen_Max_X]
        cmp     eax, ebx
        jle     .fix_vertical
        mov     eax, [edi + WDATA.box.width]
        sub     eax, ebx
        jle     @f
        mov     [edi + WDATA.box.width], ebx
    @@: sub     ebx, [edi + WDATA.box.width]
        mov     [edi + WDATA.box.left], ebx

  .fix_vertical:
        mov     eax, [edi + WDATA.box.top]
        add     eax, [edi + WDATA.box.height]
        mov     ebx, [Screen_Max_Y]
        cmp     eax, ebx
        jle     .fix_client_box
        mov     eax, [edi + WDATA.box.height]
        sub     eax, ebx
        jle     @f
        mov     [edi + WDATA.box.height], ebx
    @@: sub     ebx, [edi + WDATA.box.height]
        mov     [edi + WDATA.box.top], ebx
        jmp     .fix_client_box

  .fix_maximized:
        mov     eax, [screen_workarea.left]
        mov     [edi + WDATA.box.left], eax
        sub     eax, [screen_workarea.right]
        neg     eax
        mov     [edi + WDATA.box.width], eax
        mov     eax, [screen_workarea.top]
        mov     [edi + WDATA.box.top], eax
        test    [edi + WDATA.fl_wstate], WSTATE_ROLLEDUP
        jnz     .fix_client_box
        sub     eax, [screen_workarea.bottom]
        neg     eax
        mov     [edi + WDATA.box.height], eax

  .fix_client_box:
        call    set_window_clientbox

        add     edi, WDATA.sizeof
        loop    .next_window

  .exit:
        ret

align 4
;------------------------------------------------------------------------------
check_window_position: ;///////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Check if window is inside screen area
;------------------------------------------------------------------------------
;> edi = pointer to WDATA
;------------------------------------------------------------------------------
        push    eax ebx ecx edx esi

        mov     eax, [edi + WDATA.box.left]
        mov     ebx, [edi + WDATA.box.top]
        mov     ecx, [edi + WDATA.box.width]
        mov     edx, [edi + WDATA.box.height]

        mov     esi, [Screen_Max_X]
        cmp     ecx, esi
        ja      .fix_width

  .check_left:
        or      eax, eax
        jl      .fix_left_low
        add     eax, ecx
        cmp     eax, esi
        jg      .fix_left_high

  .check_height:
        mov     esi, [Screen_Max_Y]
        cmp     edx, esi
        ja      .fix_height

  .check_top:
        or      ebx, ebx
        jl      .fix_top_low
        add     ebx, edx
        cmp     ebx, esi
        jg      .fix_top_high

  .exit:
        pop     esi edx ecx ebx eax
        ret

  .fix_width:
        mov     ecx, esi
        mov     [edi + WDATA.box.width], esi
        jmp     .check_left

  .fix_left_low:
        xor     eax, eax
        mov     [edi + WDATA.box.left], eax
        jmp     .check_height

  .fix_left_high:
        mov     eax, esi
        sub     eax, ecx
        mov     [edi + WDATA.box.left], eax
        jmp     .check_height

  .fix_height:
        mov     edx, esi
        mov     [edi + WDATA.box.height], esi
        jmp     .check_top

  .fix_top_low:
        xor     ebx, ebx
        mov     [edi + WDATA.box.top], ebx
        jmp     .exit

  .fix_top_high:
        mov     ebx, esi
        sub     ebx, edx
        mov     [edi + WDATA.box.top], ebx
        jmp     .exit

align 4
;------------------------------------------------------------------------------
sys_window_mouse: ;////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        push    eax

        mov     eax, [timer_ticks]
        cmp     [new_window_starting], eax
        jb      .exit

        mov     byte[MOUSE_BACKGROUND], 0
        mov     byte[DONT_DRAW_MOUSE], 0

        mov     [new_window_starting], eax

  .exit:
        pop     eax
        ret

align 4
;------------------------------------------------------------------------------
draw_rectangle: ;//////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;> eax = pack[16(left), 16(right)]
;> ebx = pack[16(top), 16(bottom)]
;> esi = color
;------------------------------------------------------------------------------
        push    eax ebx ecx edi

        xor     edi, edi

  .flags_set:
        push    ebx

        ; set line color
        mov     ecx, esi

        ; draw top border
        rol     ebx, 16
        push    ebx
        rol     ebx, 16
        pop     bx
        call    [draw_line]

        ; draw bottom border
        mov     ebx, [esp - 2]
        pop     bx
        call    [draw_line]

        pop     ebx
        add     ebx, 1 * 65536 - 1

        ; draw left border
        rol     eax, 16
        push    eax
        rol     eax, 16
        pop     ax
        call    [draw_line]

        ; draw right border
        mov     eax, [esp - 2]
        pop     ax
        call    [draw_line]

        pop     edi ecx ebx eax
        ret

  .forced:
        push    eax ebx ecx edi
        xor     edi, edi
        inc     edi
        jmp     .flags_set

align 4
;------------------------------------------------------------------------------
drawwindow_I_caption: ;////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        push    [edx + WDATA.cl_titlebar]
        mov     esi, edx

        mov     edx, [esi + WDATA.box.top]
        mov     eax, edx
        lea     ebx, [edx + 21]
        inc     edx
        add     eax, [esi + WDATA.box.height]

        cmp     ebx, eax
        jbe     @f
        mov     ebx, eax
    @@: push    ebx

        xor     edi, edi

  .next_line:
        mov     ebx, edx
        shl     ebx, 16
        add     ebx, edx
        mov     eax, [esi + WDATA.box.left]
        inc     eax
        shl     eax, 16
        add     eax, [esi + WDATA.box.left]
        add     eax, [esi + WDATA.box.width]
        dec     eax
        mov     ecx, [esi + WDATA.cl_titlebar]
        test    ecx, 0x80000000
        jz      @f
        sub     ecx, 0x00040404
        mov     [esi + WDATA.cl_titlebar], ecx
    @@: and     ecx, 0x00ffffff
        call    [draw_line]
        inc     edx
        cmp     edx, [esp]
        jb      .next_line

        add     esp, 4
        pop     [esi + WDATA.cl_titlebar]
        ret

align 4
;------------------------------------------------------------------------------
drawwindow_I: ;////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        pushad

        ; window border

        mov     eax, [edx + WDATA.box.left - 2]
        mov     ax, word[edx + WDATA.box.left]
        add     ax, word[edx + WDATA.box.width]
        mov     ebx, [edx + WDATA.box.top - 2]
        mov     bx, word[edx + WDATA.box.top]
        add     bx, word[edx + WDATA.box.height]

        mov     esi, [edx + WDATA.cl_frames]
        call    draw_rectangle

        ; window caption

        call    drawwindow_I_caption

        ; window client area

        ; do we need to draw it?
        mov     edi, [esi + WDATA.cl_workarea]
        test    edi, 0x40000000
        jnz     .exit

        ; does client area have a positive size on screen?
        mov     edx, [esi + WDATA.box.top]
        add     edx, 21 + 5
        mov     ebx, [esi + WDATA.box.top]
        add     ebx, [esi + WDATA.box.height]
        cmp     edx, ebx
        jg      .exit

        ; okay, let's draw it
        mov     eax, 1
        mov     ebx, 21
        mov     ecx, [esi + WDATA.box.width]
        mov     edx, [esi + WDATA.box.height]
        call    [drawbar]

  .exit:
        popad
        ret

align 4
;------------------------------------------------------------------------------
drawwindow_III_caption: ;/////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        mov     ecx, [edx + WDATA.cl_titlebar]
        push    ecx
        mov     esi, edx
        mov     edx, [esi + WDATA.box.top]
        add     edx, 4
        mov     ebx, [esi + WDATA.box.top]
        add     ebx, 20
        mov     eax, [esi + WDATA.box.top]
        add     eax, [esi + WDATA.box.height]

        cmp     ebx, eax
        jb      @f
        mov     ebx, eax
    @@: push    ebx

        xor     edi, edi

  .next_line:
        mov     ebx, edx
        shl     ebx, 16
        add     ebx, edx
        mov     eax, [esi + WDATA.box.left]
        shl     eax, 16
        add     eax, [esi + WDATA.box.left]
        add     eax, [esi + WDATA.box.width]
        add     eax, 4 * 65536 - 4
        mov     ecx, [esi + WDATA.cl_titlebar]
        test    ecx, 0x40000000
        jz      @f
        add     ecx, 0x00040404
    @@: test    ecx, 0x80000000
        jz      @f
        sub     ecx, 0x00040404
    @@: mov     [esi + WDATA.cl_titlebar], ecx
        and     ecx, 0x00ffffff
        call    [draw_line]
        inc     edx
        cmp     edx, [esp]
        jb      .next_line

        add     esp, 4
        pop     [esi + WDATA.cl_titlebar]
        ret

align 4
;------------------------------------------------------------------------------
drawwindow_III: ;//////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        pushad

        ; window border

        mov     eax, [edx + WDATA.box.left - 2]
        mov     ax, word[edx + WDATA.box.left]
        add     ax, word[edx + WDATA.box.width]
        mov     ebx, [edx + WDATA.box.top - 2]
        mov     bx, word[edx + WDATA.box.top]
        add     bx, word[edx + WDATA.box.height]

        mov     esi, [edx + WDATA.cl_frames]
        shr     esi, 1
        and     esi, 0x007f7f7f
        call    draw_rectangle

        push    esi
        mov     ecx, 3
        mov     esi, [edx + WDATA.cl_frames]

  .next_frame:
        add     eax, 1 * 65536 - 1
        add     ebx, 1 * 65536 - 1
        call    draw_rectangle
        dec     ecx
        jnz     .next_frame

        pop     esi
        add     eax, 1 * 65536 - 1
        add     ebx, 1 * 65536 - 1
        call    draw_rectangle

        ; window caption

        call    drawwindow_III_caption

        ; window client area

        ; do we need to draw it?
        mov     edi, [esi + WDATA.cl_workarea]
        test    edi, 0x40000000
        jnz     .exit

        ; does client area have a positive size on screen?
        mov     edx, [esi + WDATA.box.top]
        add     edx, 21 + 5
        mov     ebx, [esi + WDATA.box.top]
        add     ebx, [esi + WDATA.box.height]
        cmp     edx, ebx
        jg      .exit

        ; okay, let's draw it
        mov     eax, 5
        mov     ebx, 20
        mov     ecx, [esi + WDATA.box.width]
        mov     edx, [esi + WDATA.box.height]
        sub     ecx, 4
        sub     edx, 4
        call    [drawbar]

  .exit:
        popad
        ret

align 4
;------------------------------------------------------------------------------
waredraw: ;////////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Activate window, redrawing if necessary
;------------------------------------------------------------------------------
        ; is it overlapped by another window now?
        push    ecx
        call    window._.check_window_draw
        test    ecx, ecx
        pop     ecx
        jz      .do_not_draw

        ; yes it is, activate and update screen buffer
        mov     byte[MOUSE_DOWN], 1
        call    window._.window_activate

        pushad
        mov     edi, [TASK_COUNT]
        movzx   esi, word[WIN_POS + edi * 2]
        shl     esi, 5
        add     esi, window_data

        mov     eax, [esi + WDATA.box.left]
        mov     ebx, [esi + WDATA.box.top]
        mov     ecx, [esi + WDATA.box.width]
        mov     edx, [esi + WDATA.box.height]

        add     ecx, eax
        add     edx, ebx

        mov     edi, [TASK_COUNT]
        movzx   esi, word[WIN_POS + edi * 2]
        call    window._.set_screen
        popad

        ; tell application to redraw itself
        mov     [edi + WDATA.fl_redraw], 1
        mov     byte[MOUSE_DOWN], 0
        ret

  .do_not_draw:
        ; no it's not, just activate the window
        call    window._.window_activate
        mov     byte[MOUSE_DOWN], 0
        mov     byte[MOUSE_BACKGROUND], 0
        mov     byte[DONT_DRAW_MOUSE], 0
        ret

align 4
;------------------------------------------------------------------------------
minimize_window: ;/////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;> eax = window number on screen
;------------------------------------------------------------------------------
;# corrupts [dl*]
;------------------------------------------------------------------------------
        push    edi
        pushfd
        cli

        ; is it already minimized?
        movzx   edi, word[WIN_POS + eax * 2]
        shl     edi, 5
        add     edi, window_data
        test    [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz     .exit

        push    eax ebx ecx edx esi

        ; no it's not, let's do that
        or      [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        mov     eax, [edi + WDATA.box.left]
        mov     [draw_limits.left], eax
        mov     ecx, eax
        add     ecx, [edi + WDATA.box.width]
        mov     [draw_limits.right], ecx
        mov     ebx, [edi + WDATA.box.top]
        mov     [draw_limits.top], ebx
        mov     edx, ebx
        add     edx, [edi + WDATA.box.height]
        mov     [draw_limits.bottom], edx
        call    calculatescreen
        xor     esi, esi
        xor     eax, eax
        call    redrawscreen

        pop     esi edx ecx ebx eax

  .exit:
        popfd
        pop     edi
        ret

align 4
;------------------------------------------------------------------------------
restore_minimized_window: ;////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;> eax = window number on screen
;------------------------------------------------------------------------------
;# corrupts [dl*]
;------------------------------------------------------------------------------
        pushad
        pushfd
        cli

        ; is it already restored?
        movzx   esi, word[WIN_POS + eax * 2]
        mov     edi, esi
        shl     edi, 5
        add     edi, window_data
        test    [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jz      .exit

        ; no it's not, let's do that
        mov     [edi + WDATA.fl_redraw], 1
        and     [edi + WDATA.fl_wstate], not WSTATE_MINIMIZED
        mov     ebp, window._.set_screen
        cmp     eax, [TASK_COUNT]
        jz      @f
        mov     ebp, calculatescreen
    @@: mov     eax, [edi + WDATA.box.left]
        mov     ebx, [edi + WDATA.box.top]
        mov     ecx, [edi + WDATA.box.width]
        mov     edx, [edi + WDATA.box.height]
        add     ecx, eax
        add     edx, ebx
        call    ebp

        mov     byte[MOUSE_BACKGROUND], 0

  .exit:
        popfd
        popad
        ret

align 4
;------------------------------------------------------------------------------
checkwindows: ;////////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Check for user-initiated window operations
;------------------------------------------------------------------------------
        pushad

        ; do we have window minimize/restore request?
        cmp     [window_minimize], 0
        je      .check_for_mouse_buttons_state

        ; okay, minimize or restore top-most window and exit
        mov     eax, [TASK_COUNT]
        mov     bl, 0
        xchg    [window_minimize], bl
        dec     bl
        jnz     @f
        call    minimize_window
        jmp     .check_for_mouse_buttons_state
    @@: call    restore_minimized_window

  .check_for_mouse_buttons_state:
        ; do we have any mouse buttons pressed?
        cmp     byte[BTN_DOWN], 0
        jne     .mouse_buttons_pressed

        mov     [bPressedMouseXY_W], 0
        jmp     .exit

  .mouse_buttons_pressed:
        ; yes we do, iterate and ...
        mov     esi, [TASK_COUNT]
        inc     esi

        cmp     [bPressedMouseXY_W], 1
        ja      .next_window
        inc     [bPressedMouseXY_W]
        jnc     .next_window
        push    dword[MOUSE_X]
        pop     dword[mx]

  .next_window:
        cmp     esi, 2
        jb      .exit

        dec     esi

        ; is that window not minimized?
        movzx   edi, word[WIN_POS + esi * 2]
        shl     edi, 5
        add     edi, window_data
        test    [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz     .next_window

        movzx   eax, [mx]
        movzx   ebx, [my]

        ; is the cursor inside screen bounds of that window?
        mov     ecx, [edi + WDATA.box.left]
        mov     edx, [edi + WDATA.box.top]
        cmp     eax, ecx
        jl      .next_window
        cmp     ebx, edx
        jl      .next_window
        add     ecx, [edi + WDATA.box.width]
        add     edx, [edi + WDATA.box.height]
        cmp     eax, ecx
        jge     .next_window
        cmp     ebx, edx
        jge     .next_window

        ; is that a top-most (which means active) window?
        cmp     esi, [TASK_COUNT]
        je      .check_for_moving_or_resizing

        ; no it's not, did we just press mouse button down above it or was it
        ; already pressed before?
        cmp     [bPressedMouseXY_W], 1
        ja      .exit

        ; okay, we just pressed the button, activate this window and exit
        lea     esi, [WIN_POS + esi * 2]
        call    waredraw
        jmp     .exit

  .check_for_moving_or_resizing:
        ; is that window movable?
        test    byte[edi + WDATA.cl_titlebar + 3], 0x01
        jnz     .exit

        ; yes it is, is it rolled up?
        test    [edi + WDATA.fl_wstate], WSTATE_ROLLEDUP
        jnz     .check_for_cursor_on_caption

        ; no it's not, can it be resized then?
        mov     [do_resize_from_corner], 0
        mov     dl, [edi + WDATA.fl_wstyle]
        and     dl, 0x0f
        cmp     dl, 0x00
        je      .check_for_cursor_on_caption
        cmp     dl, 0x01
        je      .check_for_cursor_on_caption
        cmp     dl, 0x04
        je      .check_for_cursor_on_caption

        ; are we going to resize it?
        mov     edx, [edi + WDATA.box.top]
        add     edx, [edi + WDATA.box.height]
        sub     edx, 6
        cmp     ebx, edx
        jl      .check_for_cursor_on_caption

        ; yes we do, remember that
        mov     [do_resize_from_corner], 1
        jmp     .set_move_resize_flag

  .check_for_cursor_on_caption:
        ; is the cursor inside window titlebar?
        push    eax
        call    window._.get_titlebar_height
        add     eax, [edi + WDATA.box.top]
        cmp     ebx, eax
        pop     eax
        jge     .exit

        ; calculate duration between two clicks
        mov     ecx, [timer_ticks]
        mov     edx, ecx
        sub     edx, [latest_window_touch]
        mov     [latest_window_touch], ecx
        mov     [latest_window_touch_delta], edx

  .set_move_resize_flag:
        mov     cl, [BTN_DOWN]
        mov     [do_resize], cl

        mov     ecx, [edi + WDATA.box.left]
        mov     edx, [edi + WDATA.box.top]

        push    ecx edx
        mov     [draw_limits.left], ecx
        mov     [draw_limits.top], edx
        add     ecx, [edi + WDATA.box.width]
        add     edx, [edi + WDATA.box.height]
        mov     [draw_limits.right], ecx
        mov     [draw_limits.bottom], edx
        pop     edx ecx

        ; calculate window-relative cursor coordinates
        sub     eax, ecx
        sub     ebx, edx

        push    dword[MOUSE_X]
        pop     dword[WIN_TEMP_XY]

        ; save old window coordinates
        push    eax
        mov     eax, [edi + WDATA.box.left]
        mov     [old_window_pos.left], eax
        mov     [new_window_pos.left], eax
        mov     eax, [edi + WDATA.box.top]
        mov     [old_window_pos.top], eax
        mov     [new_window_pos.top], eax
        mov     eax, [edi + WDATA.box.width]
        mov     [old_window_pos.width], eax
        mov     [new_window_pos.width], eax
        mov     eax, [edi + WDATA.box.height]
        mov     [old_window_pos.height], eax
        mov     [new_window_pos.height], eax
        pop     eax

        ; draw negative moving/sizing frame
        call    window._.draw_window_frames

        mov     [reposition], 0
        mov     byte[MOUSE_DOWN], 1

  .next_mouse_state_check:
        ; process OS events
        mov     byte[DONT_DRAW_MOUSE], 1
        call    checkidle
        call    checkVga_N13
        mov     byte[MOUSE_BACKGROUND], 0
        call    [draw_pointer]
        pushad
        call    stack_handler
        popad

        ; did cursor position change?
        mov     esi, [WIN_TEMP_XY]
        cmp     esi, [MOUSE_X]
        je      .check_for_new_mouse_buttons_state

        ; yes it did, calculate window-relative cursor coordinates
        movzx   ecx, word[MOUSE_X]
        movzx   edx, word[MOUSE_Y]
        sub     ecx, eax
        sub     edx, ebx

        push    eax ebx

        ; we're going to draw new frame, erasing the old one
        call    window._.draw_window_frames

        ; are we moving it right now?
        cmp     [do_resize_from_corner], 0
        jne     .resize_window

        ; yes we do, check if it's inside the screen area
        mov     eax, [Screen_Max_X]
        mov     ebx, [Screen_Max_Y]

        mov     [new_window_pos.left], 0
        or      ecx, ecx
        jle     .check_for_new_vert_cursor_pos
        mov     [reposition], 1
        sub     eax, [new_window_pos.width]
        mov     [new_window_pos.left], eax
        cmp     ecx, eax
        jge     .check_for_new_vert_cursor_pos
        mov     [new_window_pos.left], ecx

  .check_for_new_vert_cursor_pos:
        mov     [new_window_pos.top], 0
        or      edx, edx
        jle     .draw_new_window_frame
        mov     [reposition], 1
        sub     ebx, [new_window_pos.height]
        mov     [new_window_pos.top], ebx
        cmp     edx, ebx
        jge     .draw_new_window_frame
        mov     [new_window_pos.top], edx
        jmp     .draw_new_window_frame

  .resize_window:
        push    eax ebx edx

        mov     edx, edi
        sub     edx, window_data
        lea     edx, [SLOT_BASE + edx * 8]

        movzx   eax, word[MOUSE_X]
        cmp     eax, [edi + WDATA.box.left]
        jb      .fix_new_vert_size
        sub     eax, [edi + WDATA.box.left]
        cmp     eax, 32
        jge     @f
        mov     eax, 32
    @@: mov     [new_window_pos.width], eax

  .fix_new_vert_size:
        call    window._.get_rolledup_height
        mov     ebx, eax
        movzx   eax, word[MOUSE_Y]
        cmp     eax, [edi + WDATA.box.top]
        jb      .set_reposition_flag
        sub     eax, [edi + WDATA.box.top]
        cmp     eax, ebx
        jge     @f
        mov     eax, ebx
    @@: mov     [new_window_pos.height], eax

  .set_reposition_flag:
        mov     [reposition], 1

        pop     edx ebx eax

  .draw_new_window_frame:
        pop     ebx eax

        ; draw new window moving/sizing frame
        call    window._.draw_window_frames

        mov     esi, [MOUSE_X]
        mov     [WIN_TEMP_XY], esi

  .check_for_new_mouse_buttons_state:
        ; did user release mouse button(s)?
        cmp     byte[BTN_DOWN], 0
        jne     .next_mouse_state_check

        ; yes he did, moving/sizing is over
        mov     byte[DONT_DRAW_MOUSE], 1
        mov     cl, 0
        test    [edi + WDATA.fl_wstate], WSTATE_MAXIMIZED
        jnz     .check_other_actions

        mov     cl, [reposition]

        ; draw negative frame once again to hide it
        call    window._.draw_window_frames

        ; save new window bounds
        mov     eax, [new_window_pos.left]
        mov     [edi + WDATA.box.left], eax
        mov     eax, [new_window_pos.top]
        mov     [edi + WDATA.box.top], eax
        mov     eax, [new_window_pos.width]
        mov     [edi + WDATA.box.width], eax
        mov     eax, [new_window_pos.height]
        mov     [edi + WDATA.box.height], eax
        call    set_window_clientbox

        cmp     cl, 1
        jne     .check_other_actions
        push    esi edi ecx
        mov     esi, edi
        mov     ecx, 2
        test    [edi + WDATA.fl_wstate], WSTATE_ROLLEDUP or WSTATE_MAXIMIZED
        jnz     @f
        add     ecx, 2
    @@: sub     edi, window_data
        shr     edi, 5
        shl     edi, 8
        add     edi, SLOT_BASE + APPDATA.saved_box
        cld
        rep     movsd
        pop     ecx edi esi

  .check_other_actions:
        mov     [reposition], cl

        pushad

        mov     dl, [edi + WDATA.fl_wstyle]
        and     dl, 0x0f
        cmp     dl, 0x00
        je      .check_if_window_fits_screen
        cmp     dl, 0x01
        je      .check_if_window_fits_screen

        cmp     cl, 1
        je      .no_window_sizing
        mov     edx, edi
        sub     edx, window_data
        shr     edx, 5
        shl     edx, 8
        add     edx, SLOT_BASE

        ; did we right-click on titlebar?
        cmp     [do_resize], 2
        jne     .check_maximization_request

        ; yes we did, toggle normal/rolled up window state
        xor     [edi + WDATA.fl_wstate], WSTATE_ROLLEDUP
        mov     [reposition], 1

        ; calculate and set appropriate window height
        test    [edi + WDATA.fl_wstate], WSTATE_ROLLEDUP
        jz      @f
        call    window._.get_rolledup_height
        jmp     .set_new_window_height
    @@: mov     eax, [edx + APPDATA.saved_box.height]
        test    [edi + WDATA.fl_wstate], WSTATE_MAXIMIZED
        jz      .set_new_window_height
        mov     eax, [screen_workarea.bottom]
        sub     eax, [screen_workarea.top]

  .set_new_window_height:
        mov     [edi + WDATA.box.height], eax
        add     eax, [edi + WDATA.box.top]
        cmp     eax, [Screen_Max_Y]
        jbe     @f
        mov     eax, [Screen_Max_Y]
        sub     eax, [edi + WDATA.box.height]
        mov     [edi + WDATA.box.top], eax
    @@: call    check_window_position
        call    set_window_clientbox

  .check_maximization_request:
        ; can window change its height?
        push    edx
        mov     dl, [edi + WDATA.fl_wstyle]
        and     dl, 0x0f
        cmp     dl, 0x04
        pop     edx
        je      .check_if_window_fits_screen

        ; was it really a maximize/restore request?
        cmp     [do_resize], 1
        jne     .check_if_window_fits_screen
        cmp     [do_resize_from_corner], 0
        jne     .check_if_window_fits_screen
        cmp     [latest_window_touch_delta], 50
        jg      .check_if_window_fits_screen

        ; yes is was, toggle normal/maximized window state
        xor     [edi + WDATA.fl_wstate], WSTATE_MAXIMIZED
        mov     [reposition], 1

        ; calculate and set appropriate window bounds
        test    [edi + WDATA.fl_wstate], WSTATE_MAXIMIZED
        jz      .restore_normal_window_size
        mov     eax, [screen_workarea.left]
        mov     [edi + WDATA.box.left], eax
        sub     eax, [screen_workarea.right]
        neg     eax
        mov     [edi + WDATA.box.width], eax
        mov     eax, [screen_workarea.top]
        mov     [edi + WDATA.box.top], eax
        test    [edi + WDATA.fl_wstate], WSTATE_ROLLEDUP
        jnz     .calculate_window_client_area
        sub     eax, [screen_workarea.bottom]
        neg     eax
        mov     [edi + WDATA.box.height], eax
        jmp     .calculate_window_client_area

  .restore_normal_window_size:
        push    [edi + WDATA.box.height]
        push    edi
        lea     esi, [edx + APPDATA.saved_box]
        mov     ecx, 4
        cld
        rep     movsd
        pop     edi
        pop     eax
        test    [edi + WDATA.fl_wstate], WSTATE_ROLLEDUP
        jz      .calculate_window_client_area
        mov     [edi + WDATA.box.height], eax

  .calculate_window_client_area:
        call    set_window_clientbox

  .check_if_window_fits_screen:
        ; does window fit into screen area?
        mov     eax, [edi + WDATA.box.top]
        add     eax, [edi + WDATA.box.height]
        cmp     eax, [Screen_Max_Y]
        jbe     .no_window_sizing
        mov     eax, [edi + WDATA.box.left]
        add     eax, [edi + WDATA.box.width]
        cmp     eax, [Screen_Max_X]
        jbe     .no_window_sizing

        ; no it doesn't, fix that
        mov     eax, [Screen_Max_X]
        sub     eax, [edi + WDATA.box.width]
        mov     [edi + WDATA.box.left], eax
        mov     eax, [Screen_Max_Y]
        sub     eax, [edi + WDATA.box.height]
        mov     [edi + WDATA.box.top], eax
        call    set_window_clientbox

  .no_window_sizing:
        popad

        ; did somethins actually change its place?
        cmp     [reposition], 0
        je      .reset_vars

        mov     byte[DONT_DRAW_MOUSE], 1

        push    eax ebx ecx edx

        ; recalculate screen buffer at new position
        mov     eax, [edi + WDATA.box.left]
        mov     ebx, [edi + WDATA.box.top]
        mov     ecx, [edi + WDATA.box.width]
        mov     edx, [edi + WDATA.box.height]
        add     ecx, eax
        add     edx, ebx
        call    calculatescreen

        ; recalculate screen buffer at old position
        mov     eax, [old_window_pos.left]
        mov     ebx, [old_window_pos.top]
        mov     ecx, [old_window_pos.width]
        mov     edx, [old_window_pos.height]
        add     ecx, eax
        add     edx, ebx
        call    calculatescreen

        pop     edx ecx ebx eax

        mov     eax, edi
        call    redrawscreen

        ; tell window to redraw itself
        mov     [edi + WDATA.fl_redraw], 1

        ; wait a bit for window to redraw itself
        mov     ecx, 100

  .next_idle_cycle:
        mov     byte[DONT_DRAW_MOUSE], 1
        call    checkidle
        cmp     [edi + WDATA.fl_redraw], 0
        jz      .reset_vars
        loop    .next_idle_cycle

  .reset_vars:
        mov     byte[DONT_DRAW_MOUSE], 0 ; mouse pointer
        mov     byte[MOUSE_BACKGROUND], 0 ; no mouse under
        mov     byte[MOUSE_DOWN], 0 ; react to mouse up/down

  .exit:
        popad
        ret

;==============================================================================
;///// private functions //////////////////////////////////////////////////////
;==============================================================================

align 4
;------------------------------------------------------------------------------
window._.get_titlebar_height: ;////////////////////////////////////////////////
;------------------------------------------------------------------------------
;> edi = pointer to WDATA
;------------------------------------------------------------------------------
        mov     al, [edi + WDATA.fl_wstyle]
        and     al, 0x0f
        cmp     al, 0x03
        jne     @f
        mov     eax, [_skinh]
        ret
    @@: mov     eax, 21
        ret

align 4
;------------------------------------------------------------------------------
window._.get_rolledup_height: ;////////////////////////////////////////////////
;------------------------------------------------------------------------------
;> edi = pointer to WDATA
;------------------------------------------------------------------------------
        mov     al, [edi + WDATA.fl_wstyle]
        and     al, 0x0f
        cmp     al, 0x03
        jb      @f
        mov     eax, [_skinh]
        add     eax, 3
        ret
    @@: or      al, al
        jnz     @f
        mov     eax, 21
        ret
    @@: mov     eax, 21 + 2
        ret

align 4
;------------------------------------------------------------------------------
window._.set_screen: ;/////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Reserve window area in screen buffer
;------------------------------------------------------------------------------
;> eax = left
;> ebx = top
;> ecx = right
;> edx = bottom
;> esi = process number
;------------------------------------------------------------------------------
virtual at esp
  ff_x     dd ?
  ff_y     dd ?
  ff_width dd ?
  ff_xsz   dd ?
  ff_ysz   dd ?
  ff_scale dd ?
end virtual

        pushad

        cmp     esi, 1
        jz      .check_for_shaped_window
        mov     edi, esi
        shl     edi, 5
        cmp     [window_data + edi + WDATA.box.width], 0
        jnz     .check_for_shaped_window
        cmp     [window_data + edi + WDATA.box.height], 0
        jz      .exit

  .check_for_shaped_window:
        mov     edi, esi
        shl     edi, 8
        add     edi, SLOT_BASE
        cmp     [edi + APPDATA.wnd_shape], 0
        jne     .shaped_window

        ; get x&y size
        sub     ecx, eax
        sub     edx, ebx
        inc     ecx
        inc     edx

        ; get WinMap start
        push    esi
        mov     edi, [Screen_Max_X]
        inc     edi
        mov     esi, edi
        imul    edi, ebx
        add     edi, eax
        add     edi, [_WinMapAddress]
        pop     eax
        mov     ah, al
        push    ax
        shl     eax, 16
        pop     ax

  .next_line:
        push    ecx
        shr     ecx, 2
        rep     stosd
        mov     ecx, [esp]
        and     ecx, 3
        rep     stosb
        pop     ecx
        add     edi, esi
        sub     edi, ecx
        dec     edx
        jnz     .next_line

        jmp     .exit

  .shaped_window:
        ;  for (y=0; y <= x_size; y++)
        ;      for (x=0; x <= x_size; x++)
        ;          if (shape[coord(x,y,scale)]==1)
        ;             set_pixel(x, y, process_number);

        sub     ecx, eax
        sub     edx, ebx
        inc     ecx
        inc     edx

        push    [edi + APPDATA.wnd_shape_scale]  ; push scale first -> for loop

        ; get WinMap start  -> ebp
        push    eax
        mov     eax, [Screen_Max_X] ; screen_sx
        inc     eax
        imul    eax, ebx
        add     eax, [esp]
        add     eax, [_WinMapAddress]
        mov     ebp, eax

        mov     edi, [edi + APPDATA.wnd_shape]
        pop     eax

        ; eax = x_start
        ; ebx = y_start
        ; ecx = x_size
        ; edx = y_size
        ; esi = process_number
        ; edi = &shape
        ;       [scale]
        push    edx ecx ; for loop - x,y size

        mov     ecx, esi
        shl     ecx, 5
        mov     edx, [window_data + ecx + WDATA.box.top]
        push    [window_data + ecx + WDATA.box.width]           ; for loop - width
        mov     ecx, [window_data + ecx + WDATA.box.left]
        sub     ebx, edx
        sub     eax, ecx
        push    ebx eax ; for loop - x,y

        add     [ff_xsz], eax
        add     [ff_ysz], ebx

        mov     ebx, [ff_y]

  .ff_new_y:
        mov     edx, [ff_x]

  .ff_new_x:
        ; -- body --
        mov     ecx, [ff_scale]
        mov     eax, [ff_width]
        inc     eax
        shr     eax, cl
        push    ebx edx
        shr     ebx, cl
        shr     edx, cl
        imul    eax, ebx
        add     eax, edx
        pop     edx ebx
        add     eax, edi
        call    .read_byte
        test    al,al
        jz      @f
        mov     eax, esi
        mov     [ebp], al
        ; -- end body --
    @@: inc     ebp
        inc     edx
        cmp     edx, [ff_xsz]
        jb      .ff_new_x

        sub     ebp, [ff_xsz]
        add     ebp, [ff_x]
        add     ebp, [Screen_Max_X]  ; screen.x
        inc     ebp
        inc     ebx
        cmp     ebx, [ff_ysz]
        jb      .ff_new_y

        add     esp, 24

  .exit:
        popad
        ret

  .read_byte:
        ; eax - address
        ; esi - slot
        push    eax ecx edx esi
        xchg    eax, esi
        lea     ecx, [esp + 12]
        mov     edx, 1
        call    read_process_memory
        pop     esi edx ecx eax
        ret

align 4
;------------------------------------------------------------------------------
window._.window_activate: ;////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Activate window
;------------------------------------------------------------------------------
;> esi = pointer to WIN_POS+ window data
;------------------------------------------------------------------------------
        push    eax ebx

        ; if type of current active window is 3 or 4, it must be redrawn
        mov     ebx, [TASK_COUNT]
        movzx   ebx, word[WIN_POS + ebx * 2]
        shl     ebx, 5
        add     eax, window_data
        mov     al, [window_data + ebx + WDATA.fl_wstyle]
        and     al, 0x0f
        cmp     al, 0x03
        je      .set_window_redraw_flag
        cmp     al, 0x04
        jne     .move_others_down

  .set_window_redraw_flag:
        mov     [window_data + ebx + WDATA.fl_redraw], 1

  .move_others_down:
        ; ax <- process no
        movzx   ebx, word[esi]
        ; ax <- position in window stack
        movzx   ebx, word[WIN_STACK + ebx * 2]

        ; drop others
        xor     eax, eax

  .next_stack_window:
        cmp     eax, [TASK_COUNT]
        jae     .move_self_up
        inc     eax
        cmp     [WIN_STACK + eax * 2], bx
        jbe     .next_stack_window
        dec     word[WIN_STACK + eax * 2]
        jmp     .next_stack_window

  .move_self_up:
        movzx   ebx, word[esi]
        ; number of processes
        mov     ax, [TASK_COUNT]
        ; this is the last (and the upper)
        mov     [WIN_STACK + ebx * 2], ax

        ; update on screen - window stack
        xor     eax, eax

  .next_window_pos:
        cmp     eax, [TASK_COUNT]
        jae     .reset_vars
        inc     eax
        movzx   ebx, word[WIN_STACK + eax * 2]
        mov     [WIN_POS + ebx * 2], ax
        jmp     .next_window_pos

  .reset_vars:
        mov     byte[KEY_COUNT], 0
        mov     byte[BTN_COUNT], 0
        mov     word[MOUSE_SCROLL_H], 0
        mov     word[MOUSE_SCROLL_V], 0

        pop     ebx eax
        ret

align 4
;------------------------------------------------------------------------------
window._.check_window_draw: ;//////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Check if window is necessary to draw
;------------------------------------------------------------------------------
;> edi = pointer to WDATA
;------------------------------------------------------------------------------
        mov     cl, [edi + WDATA.fl_wstyle]
        and     cl, 0x0f
        cmp     cl, 0x03
        je      .exit.redraw      ; window type 3
        cmp     cl, 0x04
        je      .exit.redraw      ; window type 4

        push    eax ebx edx esi

        mov     eax, edi
        sub     eax, window_data
        shr     eax, 5

        ; esi = process number

        movzx   eax, word[WIN_STACK + eax * 2]  ; get value of the curr process
        lea     esi, [WIN_POS + eax * 2]        ; get address of this process at 0xC400

  .next_window:
        add     esi, 2

        mov     eax, [TASK_COUNT]
        lea     eax, word[WIN_POS + eax * 2] ; number of the upper window

        cmp     esi, eax
        ja      .exit.no_redraw

        movzx   edx, word[esi]
        shl     edx, 5
        cmp     [CURRENT_TASK + edx + TASKDATA.state], TSTATE_FREE
        je      .next_window

        mov     eax, [edi + WDATA.box.top]
        mov     ebx, [edi + WDATA.box.height]
        add     ebx, eax

        mov     ecx, [window_data + edx + WDATA.box.top]
        cmp     ecx, ebx
        jge     .next_window
        add     ecx, [window_data + edx + WDATA.box.height]
        cmp     eax, ecx
        jge     .next_window

        mov     eax, [edi + WDATA.box.left]
        mov     ebx, [edi + WDATA.box.width]
        add     ebx, eax

        mov     ecx, [window_data + edx + WDATA.box.left]
        cmp     ecx, ebx
        jge     .next_window
        add     ecx, [window_data + edx + WDATA.box.width]
        cmp     eax, ecx
        jge     .next_window

        pop     esi edx ebx eax

  .exit.redraw:
        xor     ecx, ecx
        inc     ecx
        ret

  .exit.no_redraw:
        pop     esi edx ebx eax
        xor     ecx, ecx
        ret

align 4
;------------------------------------------------------------------------------
window._.draw_window_frames: ;/////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Draw negative window frames
;------------------------------------------------------------------------------
;> edi = pointer to WDATA
;------------------------------------------------------------------------------
        push    eax
        cli

        test    [edi + WDATA.fl_wstate], WSTATE_MAXIMIZED
        jnz     .exit
        mov     eax, [new_window_pos.left]
        cmp     eax, [edi + WDATA.box.left]
        jnz     .draw
        mov     eax, [new_window_pos.width]
        cmp     eax, [edi + WDATA.box.width]
        jnz     .draw
        mov     eax, [new_window_pos.top]
        cmp     eax, [edi + WDATA.box.top]
        jnz     .draw
        mov     eax, [new_window_pos.height]
        cmp     eax, [edi + WDATA.box.height]
        jnz     .draw
        xor     [edi + WDATA.fl_wdrawn], 2

  .draw:
        push    ebx esi
        mov     eax, [new_window_pos.left - 2]
        mov     ax, word[new_window_pos.left]
        add     ax, word[new_window_pos.width]
        mov     ebx, [new_window_pos.top - 2]
        mov     bx, word[new_window_pos.top]
        add     bx, word[new_window_pos.height]
        mov     esi, 0x01000000
        call    draw_rectangle.forced
        pop     esi ebx

  .exit:
        sti
        pop     eax
        ret

  .forced:
        push    eax
        cli
        jmp     .draw
