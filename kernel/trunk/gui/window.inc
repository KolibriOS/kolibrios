;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

;==============================================================================
;///// public functions ///////////////////////////////////////////////////////
;==============================================================================

window.BORDER_SIZE = 5

macro FuncTable name, table_name, [label]
{
  common
    align 4
    \label name#.#table_name dword
  forward
    dd name#.#label
  common
    name#.sizeof.#table_name = $ - name#.#table_name
}

uglobal
  common_colours rd 32
  draw_limits    RECT
endg

align 4
;------------------------------------------------------------------------------
syscall_draw_window: ;///// system function 0 /////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        mov     eax, edx
        shr     eax, 24
        and     al, 0x0f
        cmp     al, 5
        jae     .exit

        push    eax
        inc     [mouse_pause]
        call    [_display.disable_mouse]
        call    window._.sys_set_window
        call    [_display.disable_mouse]
        pop     eax

        or      al, al
        jnz     @f

        ; type I - original style
        call    drawwindow_I
        jmp     window._.draw_window_caption.2

    @@: dec     al
        jnz     @f

        ; type II - only reserve area, no draw
        call    sys_window_mouse
        dec     [mouse_pause]
        call    [draw_pointer]
        jmp     .exit

    @@: dec     al
        jnz     @f

        ; type III  - new style
        call    drawwindow_III
        jmp     window._.draw_window_caption.2

        ; type IV & V - skinned window (resizable & not)
    @@: mov     eax, [TASK_COUNT]
        movzx   eax, word[WIN_POS + eax * 2]
        cmp     eax, [CURRENT_TASK]
        setz    al
        movzx   eax, al
        push    eax
        call    drawwindow_IV
        jmp     window._.draw_window_caption.2

  .exit:
        ret

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
syscall_move_window: ;///// system function 67 ////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        mov     edi, [CURRENT_TASK]
        shl     edi, 5
        add     edi, window_data

        test    [edi + WDATA.fl_wdrawn], 1
        jz      .exit

        test    [edi + WDATA.fl_wstate], WSTATE_MAXIMIZED
        jnz     .exit

        cmp     ebx, -1
        jne     @f
        mov     ebx, [edi + WDATA.box.left]
    @@: cmp     ecx, -1
        jne     @f
        mov     ecx, [edi + WDATA.box.top]
    @@: cmp     edx, -1
        jne     @f
        mov     edx, [edi + WDATA.box.width]
    @@: cmp     esi, -1
        jne     @f
        mov     esi, [edi + WDATA.box.height]

    @@: push    esi edx ecx ebx
        mov     eax, esp
        mov     bl, [edi + WDATA.fl_wstate]
        call    window._.set_window_box
        add     esp, BOX.sizeof

        ; NOTE: do we really need this? to be reworked
;       mov     byte[DONT_DRAW_MOUSE], 0 ; mouse pointer
;       mov     byte[MOUSE_BACKGROUND], 0 ; no mouse under
;       mov     byte[MOUSE_DOWN], 0 ; react to mouse up/down

        ; NOTE: do we really need this? to be reworked
;       call    [draw_pointer]

  .exit:
        ret

align 4
;------------------------------------------------------------------------------
syscall_window_settings: ;///// system function 71 /////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        dec     ebx     ; subfunction #1 - set window caption
        jnz     .exit_fail

        ; NOTE: only window owner thread can set its caption,
        ;       so there's no parameter for PID/TID

        mov     edi, [CURRENT_TASK]
        shl     edi, 5

        mov     [edi * 8 + SLOT_BASE + APPDATA.wnd_caption], ecx
        or      [edi + window_data + WDATA.fl_wstyle], WSTYLE_HASCAPTION

        call    window._.draw_window_caption

        xor     eax, eax ; eax = 0 (success)
        ret

;  .get_window_caption:
;        dec     eax     ; subfunction #2 - get window caption
;        jnz     .exit_fail

        ; not implemented yet

  .exit_fail:
        xor     eax, eax
        inc     eax     ; eax = 1 (fail)
        ret

align 4
;------------------------------------------------------------------------------
set_window_defaults: ;/////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        mov     byte [window_data + 0x20 + WDATA.cl_titlebar + 3], 1 ; desktop is not movable
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
        call    window._.set_window_clientbox

        add     edi, WDATA.sizeof
        loop    .next_window

  .exit:
        ret

align 4
;------------------------------------------------------------------------------
sys_window_mouse: ;////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        ; NOTE: commented out since doesn't provide necessary functionality
        ;       anyway, to be reworked
;       push    eax
;
;       mov     eax, [timer_ticks]
;       cmp     [new_window_starting], eax
;       jb      .exit
;
;       mov     byte[MOUSE_BACKGROUND], 0
;       mov     byte[DONT_DRAW_MOUSE], 0
;
;       mov     [new_window_starting], eax
;
; .exit:
;       pop     eax
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
        push    -1
        mov     eax, [TASK_COUNT]
        lea     eax, [WIN_POS + eax * 2]
        cmp     eax, esi
        pop     eax
        je      .exit

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
        xor     eax, eax
        jmp     .exit

  .do_not_draw:
        ; no it's not, just activate the window
        call    window._.window_activate
        xor     eax, eax
        mov     byte[MOUSE_BACKGROUND], al
        mov     byte[DONT_DRAW_MOUSE], al


  .exit:
        mov     byte[MOUSE_DOWN], 0
        inc     eax
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
; TODO: remove this proc
;------------------------------------------------------------------------------
window_check_events: ;/////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        ; do we have window minimize/restore request?
        cmp     [window_minimize], 0
        je      .exit

        ; okay, minimize or restore top-most window and exit
        mov     eax, [TASK_COUNT]
        mov     bl, 0
        xchg    [window_minimize], bl
        dec     bl
        jnz     @f
        call    minimize_window
        jmp     .exit

    @@: call    restore_minimized_window

  .exit:
        ret

align 4
;------------------------------------------------------------------------------
sys_window_maximize_handler: ;/////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> esi = process slot
;------------------------------------------------------------------------------
        mov     edi, esi
        shl     edi, 5
        add     edi, window_data

        ; can window change its height?
        ; only types 2 and 3 can be resized
        mov     dl, [edi + WDATA.fl_wstyle]
        test    dl, 2
        jz      .exit

        ; toggle normal/maximized window state
        mov     bl, [edi + WDATA.fl_wstate]
        xor     bl, WSTATE_MAXIMIZED

        ; calculate and set appropriate window bounds
        test    bl, WSTATE_MAXIMIZED
        jz      .restore_size

        mov     eax, [screen_workarea.left]
        mov     ecx, [screen_workarea.top]
        push    [screen_workarea.bottom] \
                [screen_workarea.right] \
                ecx \
                eax
        sub     [esp + BOX.width], eax
        sub     [esp + BOX.height], ecx
        mov     eax, esp
        jmp     .set_box

  .restore_size:
        mov     eax, esi
        shl     eax, 8
        add     eax, SLOT_BASE + APPDATA.saved_box
        push    [eax + BOX.height] \
                [eax + BOX.width] \
                [eax + BOX.top] \
                [eax + BOX.left]
        mov     eax, esp

  .set_box:
        test    bl, WSTATE_ROLLEDUP
        jz      @f

        xchg    eax, ecx
        call    window._.get_rolledup_height
        mov     [ecx + BOX.height], eax
        xchg    eax, ecx

    @@: call    window._.set_window_box
        add     esp, BOX.sizeof

  .exit:
        ret

align 4
;------------------------------------------------------------------------------
sys_window_rollup_handler: ;///////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> esi = process slot
;------------------------------------------------------------------------------
        mov     edx, esi
        shl     edx, 8
        add     edx, SLOT_BASE

        ; toggle normal/rolled up window state
        mov     bl, [edi + WDATA.fl_wstate]
        xor     bl, WSTATE_ROLLEDUP

        ; calculate and set appropriate window bounds
        test    bl, WSTATE_ROLLEDUP
        jz      .restore_size

        call    window._.get_rolledup_height
        push    eax \
                [edi + WDATA.box.width] \
                [edi + WDATA.box.top] \
                [edi + WDATA.box.left]
        mov     eax, esp
        jmp     .set_box

  .restore_size:
        test    bl, WSTATE_MAXIMIZED
        jnz     @f
        add     esp, -BOX.sizeof
        lea     eax, [edx + APPDATA.saved_box]
        jmp     .set_box

    @@: mov     eax, [screen_workarea.top]
        push    [screen_workarea.bottom] \
                [edi + WDATA.box.width] \
                eax \
                [edi + WDATA.box.left]
        sub     [esp + BOX.height], eax
        mov     eax, esp

  .set_box:
        call    window._.set_window_box
        add     esp, BOX.sizeof

        ret

align 4
;------------------------------------------------------------------------------
sys_window_start_moving_handler: ;/////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> eax = old (original) window box
;> esi = process slot
;------------------------------------------------------------------------------
        mov     edi, eax
        call    window._.draw_negative_box

        ret

align 4
;------------------------------------------------------------------------------
sys_window_end_moving_handler: ;///////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> eax = old (original) window box
;> ebx = new (final) window box
;> esi = process slot
;------------------------------------------------------------------------------
        mov     edi, ebx
        call    window._.draw_negative_box

        mov     edi, esi
        shl     edi, 5
        add     edi, window_data

        mov     eax, ebx
        mov     bl, [edi + WDATA.fl_wstate]
        call    window._.set_window_box
        ret

align 4
;------------------------------------------------------------------------------
sys_window_moving_handler: ;///////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> eax = old (from previous call) window box
;> ebx = new (current) window box
;> esi = process_slot
;------------------------------------------------------------------------------
        mov     edi, eax
        call    window._.draw_negative_box
        mov     edi, ebx
        call    window._.draw_negative_box
        ret

;==============================================================================
;///// private functions //////////////////////////////////////////////////////
;==============================================================================

iglobal
  FuncTable syscall_display_settings, ftable, \
    00, 01, 02, 03, 04, 05, 06, 07, 08

  align 4
  window_topleft dd \
    1, 21, \ ;type 0
    0,  0, \ ;type 1
    5, 20, \ ;type 2
    5,  ?, \ ;type 3 {set by skin}
    5,  ?    ;type 4 {set by skin}
endg

;uglobal
  ; NOTE: commented out since doesn't provide necessary functionality anyway,
  ;       to be reworked
; new_window_starting       dd ?
;endg

align 4
;------------------------------------------------------------------------------
window._.invalidate_screen: ;//////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> eax = old (original) window box
;> ebx = new (final) window box
;> edi = pointer to WDATA struct
;------------------------------------------------------------------------------
        push    eax ebx

        ; TODO: do we really need `draw_limits`?
        ; Yes, they are used by background drawing code.
        mov     ecx, [eax + BOX.left]
        mov     edx, [ebx + BOX.left]
        cmp     ecx, edx
        jle     @f
        mov     ecx, edx
    @@: mov     [draw_limits.left], ecx
        mov     ecx, [eax + BOX.left]
        add     ecx, [eax + BOX.width]
        add     edx, [ebx + BOX.width]
        cmp     ecx, edx
        jae     @f
        mov     ecx, edx
    @@: mov     [draw_limits.right], ecx
        mov     ecx, [eax + BOX.top]
        mov     edx, [ebx + BOX.top]
        cmp     ecx, edx
        jle     @f
        mov     ecx, edx
    @@: mov     [draw_limits.top], ecx
        mov     ecx, [eax + BOX.top]
        add     ecx, [eax + BOX.height]
        add     edx, [ebx + BOX.height]
        cmp     ecx, edx
        jae     @f
        mov     ecx, edx
    @@: mov     [draw_limits.bottom], ecx

        ; recalculate screen buffer at old position
        push    ebx
        mov     edx, [eax + BOX.height]
        mov     ecx, [eax + BOX.width]
        mov     ebx, [eax + BOX.top]
        mov     eax, [eax + BOX.left]
        add     ecx, eax
        add     edx, ebx
        call    calculatescreen
        pop     eax

        ; recalculate screen buffer at new position
        mov     edx, [eax + BOX.height]
        mov     ecx, [eax + BOX.width]
        mov     ebx, [eax + BOX.top]
        mov     eax, [eax + BOX.left]
        add     ecx, eax
        add     edx, ebx
        call    calculatescreen

        mov     eax, edi
        call    redrawscreen

        ; tell window to redraw itself
        mov     [edi + WDATA.fl_redraw], 1

        pop     ebx eax
        ret

align 4
;------------------------------------------------------------------------------
window._.set_window_box: ;/////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> eax = pointer to BOX struct
;> bl = new window state flags
;> edi = pointer to WDATA struct
;------------------------------------------------------------------------------
        push    eax ebx esi

; don't do anything if the new box is identical to the old
	cmp	bl, [edi + WDATA.fl_wstate]
	jnz	@f
	mov	esi, eax
	push	edi
if WDATA.box
	add	edi, WDATA.box
end if
	mov	ecx, 4
	repz	cmpsd
	pop	edi
	jnz	@f
	
	mov	edi,eax
        mov     eax, [edi + BOX.left - 2]
        mov     ax, word[edi + BOX.left]
        add     ax, word[edi + BOX.width]
        mov     ebx, [edi + BOX.top - 2]
        mov     bx, word[edi + BOX.top]
        add     bx, word[edi + BOX.height]
        xor     esi,esi
        call    draw_rectangle.forced
	jmp	.exit
@@:

        add     esp, -BOX.sizeof

        mov     ebx, esp
if WDATA.box
        lea     esi, [edi + WDATA.box]
else
	mov	esi, edi ; optimization for WDATA.box = 0
end if
        xchg    eax, esi
        mov     ecx, BOX.sizeof
        call    memmove
        xchg    eax, esi
        xchg    ebx, esi
        call    memmove
        mov     eax, ebx
        mov     ebx, esi

        call    window._.check_window_position
        call    window._.set_window_clientbox
        call    window._.invalidate_screen

        add     esp, BOX.sizeof

        mov     cl, [esp + 4]
        mov     ch, cl
        xchg    cl, [edi + WDATA.fl_wstate]

        or      cl, ch
        test    cl, WSTATE_MAXIMIZED
        jnz     .exit

        mov     eax, edi
        sub     eax, window_data
        shl     eax, 3
        add     eax, SLOT_BASE

        lea     ebx, [edi + WDATA.box]
        xchg    esp, ebx

        pop     [eax + APPDATA.saved_box.left] \
                [eax + APPDATA.saved_box.top] \
                [eax + APPDATA.saved_box.width] \
                edx

        xchg    esp, ebx

        test    ch, WSTATE_ROLLEDUP
        jnz     .exit

        mov     [eax + APPDATA.saved_box.height], edx

  .exit:
        pop     esi ebx eax
        ret

align 4
;------------------------------------------------------------------------------
window._.set_window_clientbox: ;///////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> edi = pointer to WDATA struct
;------------------------------------------------------------------------------
        push    eax ecx edi

        mov     eax, [_skinh]
        mov     [window_topleft + 8 * 3 + 4], eax
        mov     [window_topleft + 8 * 4 + 4], eax

        mov     ecx, edi
        sub     edi, window_data
        shl     edi, 3
        test    [ecx + WDATA.fl_wstyle], WSTYLE_CLIENTRELATIVE
        jz      .whole_window

        movzx   eax, [ecx + WDATA.fl_wstyle]
        and     eax, 0x0F
        mov     eax, [eax * 8 + window_topleft + 0]
        mov     [edi + SLOT_BASE + APPDATA.wnd_clientbox.left], eax
        shl     eax, 1
        neg     eax
        add     eax, [ecx + WDATA.box.width]
        mov     [edi + SLOT_BASE + APPDATA.wnd_clientbox.width], eax

        movzx   eax, [ecx + WDATA.fl_wstyle]
        and     eax, 0x0F
        push    [eax * 8 + window_topleft + 0]
        mov     eax, [eax * 8 + window_topleft + 4]
        mov     [edi + SLOT_BASE + APPDATA.wnd_clientbox.top], eax
        neg     eax
        sub     eax, [esp]
        add     eax, [ecx + WDATA.box.height]
        mov     [edi + SLOT_BASE + APPDATA.wnd_clientbox.height], eax
        add     esp, 4
        jmp     .exit

  .whole_window:
        xor     eax, eax
        mov     [edi + SLOT_BASE + APPDATA.wnd_clientbox.left], eax
        mov     [edi + SLOT_BASE + APPDATA.wnd_clientbox.top], eax
        mov     eax, [ecx + WDATA.box.width]
        mov     [edi + SLOT_BASE + APPDATA.wnd_clientbox.width], eax
        mov     eax, [ecx + WDATA.box.height]
        mov     [edi + SLOT_BASE + APPDATA.wnd_clientbox.height], eax

  .exit:
        pop     edi ecx eax
        ret

align 4
;------------------------------------------------------------------------------
window._.sys_set_window: ;/////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;< edx = pointer to WDATA struct
;------------------------------------------------------------------------------
        mov     eax, [CURRENT_TASK]
        shl     eax, 5
        add     eax, window_data

        ; save window colors
        mov     [eax + WDATA.cl_workarea], edx
        mov     [eax + WDATA.cl_titlebar], esi
        mov     [eax + WDATA.cl_frames], edi

        mov     edi, eax

        ; was it already defined before?
        test    [edi + WDATA.fl_wdrawn], 1
        jnz     .set_client_box
        or      [edi + WDATA.fl_wdrawn], 1

        ; NOTE: commented out since doesn't provide necessary functionality
        ;       anyway, to be reworked
;       mov     eax, [timer_ticks] ; [0xfdf0]
;       add     eax, 100
;       mov     [new_window_starting], eax

        ; no it wasn't, performing initial window definition
        movzx   eax, bx
        mov     [edi + WDATA.box.width], eax
        movzx   eax, cx
        mov     [edi + WDATA.box.height], eax
        sar     ebx, 16
        sar     ecx, 16
        mov     [edi + WDATA.box.left], ebx
        mov     [edi + WDATA.box.top], ecx

        call    window._.check_window_position

        push    ecx edi

        mov     cl, [edi + WDATA.fl_wstyle]
        mov     eax, [edi + WDATA.cl_frames]

        sub     edi, window_data
        shl     edi, 3
        add     edi, SLOT_BASE

        and     cl, 0x0F
        cmp     cl, 3
        je      @f
        cmp     cl, 4
        je      @f

        xor     eax, eax

    @@: mov     [edi + APPDATA.wnd_caption], eax

        mov     esi, [esp]
        add     edi, APPDATA.saved_box
        movsd
        movsd
        movsd
        movsd

        pop     edi ecx

        mov     esi, [CURRENT_TASK]
        movzx   esi, word[WIN_STACK + esi * 2]
        lea     esi, [WIN_POS + esi * 2]
        call    waredraw

        mov     eax, [edi + WDATA.box.left]
        mov     ebx, [edi + WDATA.box.top]
        mov     ecx, [edi + WDATA.box.width]
        mov     edx, [edi + WDATA.box.height]
        add     ecx, eax
        add     edx, ebx
        call    calculatescreen

        mov     byte[KEY_COUNT], 0           ; empty keyboard buffer
        mov     byte[BTN_COUNT], 0           ; empty button buffer

  .set_client_box:
        ; update window client box coordinates
        call    window._.set_window_clientbox

        ; reset window redraw flag and exit
        mov     [edi + WDATA.fl_redraw], 0
        mov     edx, edi
        ret

align 4
;------------------------------------------------------------------------------
window._.check_window_position: ;//////////////////////////////////////////////
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
        ja      .fix_width_high

  .check_left:
        or      eax, eax
        jl      .fix_left_low
        add     eax, ecx
        cmp     eax, esi
        jg      .fix_left_high

  .check_height:
        mov     esi, [Screen_Max_Y]
        cmp     edx, esi
        ja      .fix_height_high

  .check_top:
        or      ebx, ebx
        jl      .fix_top_low
        add     ebx, edx
        cmp     ebx, esi
        jg      .fix_top_high

  .exit:
        pop     esi edx ecx ebx eax
        ret

  .fix_width_high:
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

  .fix_height_high:
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
window._.get_titlebar_height: ;////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
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
;? <description>
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
        cmp     cl, 3
        je      .exit.redraw      ; window type 3
        cmp     cl, 4
        je      .exit.redraw      ; window type 4

        push    eax ebx edx esi

        mov     eax, edi
        sub     eax, window_data
        shr     eax, 5

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
window._.draw_window_caption: ;////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        inc     [mouse_pause]
        call    [_display.disable_mouse]

        xor     eax, eax
        mov     edx, [TASK_COUNT]
        movzx   edx, word[WIN_POS + edx * 2]
        cmp     edx, [CURRENT_TASK]
        jne     @f
        inc     eax
    @@: mov     edx, [CURRENT_TASK]
        shl     edx, 5
        add     edx, window_data
        movzx   ebx, [edx + WDATA.fl_wstyle]
        and     bl, 0x0F
        cmp     bl, 3
        je      .draw_caption_style_3
        cmp     bl, 4
        je      .draw_caption_style_3

        jmp     .not_style_3

  .draw_caption_style_3:
        push    edx
        call    drawwindow_IV_caption
        add     esp, 4
        jmp     .2

  .not_style_3:
        cmp     bl, 2
        jne     .not_style_2

        call    drawwindow_III_caption
        jmp     .2

  .not_style_2:
        cmp     bl, 0
        jne     .2

        call    drawwindow_I_caption

  .2:   mov     edi, [CURRENT_TASK]
        shl     edi, 5
        test    [edi + window_data + WDATA.fl_wstyle], WSTYLE_HASCAPTION
        jz      .exit
        mov     edx, [edi * 8 + SLOT_BASE + APPDATA.wnd_caption]
        or      edx, edx
        jz      .exit

        movzx   eax, [edi + window_data + WDATA.fl_wstyle]
        and     al, 0x0F
        cmp     al, 3
        je      .skinned
        cmp     al, 4
        je      .skinned

        jmp     .not_skinned

  .skinned:
        mov     ebp, [edi + window_data + WDATA.box.left - 2]
        mov     bp, word[edi + window_data + WDATA.box.top]
        movzx   eax, word[edi + window_data + WDATA.box.width]
        sub     ax, [_skinmargins.left]
        sub     ax, [_skinmargins.right]
        push    edx
        cwde
        cdq
        mov     ebx, 6
        idiv    ebx
        pop     edx
        or      eax, eax
        js      .exit

        mov     esi, eax
        mov     ebx, dword[_skinmargins.left - 2]
        mov     bx, word[_skinh]
        sub     bx, [_skinmargins.bottom]
        sub     bx, [_skinmargins.top]
        sar     bx, 1
        adc     bx, 0
        add     bx, [_skinmargins.top]
        add     bx, -3
        add     ebx, ebp
        jmp     .dodraw

  .not_skinned:
        cmp     al, 1
        je      .exit

        mov     ebp, [edi + window_data + WDATA.box.left - 2]
        mov     bp, word[edi + window_data + WDATA.box.top]
        movzx   eax, word[edi + window_data + WDATA.box.width]
        sub     eax, 16
        push    edx
        cwde
        cdq
        mov     ebx, 6
        idiv    ebx
        pop     edx
        or      eax, eax
        js      .exit

        mov     esi, eax
        mov     ebx, 0x00080007
        add     ebx, ebp

  .dodraw:
        mov     ecx, [common_colours + 16]
        or      ecx, 0x80000000
        xor     edi, edi
        call    dtext_asciiz_esi

  .exit:
        dec     [mouse_pause]
        call    [draw_pointer]
        ret

align 4
;------------------------------------------------------------------------------
window._.draw_negative_box: ;//////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Draw negative box
;------------------------------------------------------------------------------
;> edi = pointer to BOX struct
;------------------------------------------------------------------------------
        push    eax ebx esi
        mov     eax, [edi + BOX.left - 2]
        mov     ax, word[edi + BOX.left]
        add     ax, word[edi + BOX.width]
        mov     ebx, [edi + BOX.top - 2]
        mov     bx, word[edi + BOX.top]
        add     bx, word[edi + BOX.height]
        mov     esi, 0x01000000
        call    draw_rectangle.forced
        pop     esi ebx eax
        ret
