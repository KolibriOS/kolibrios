;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

;==============================================================================
;     public functions 
;==============================================================================

window.BORDER_SIZE = 5

uglobal
  common_colours rd 48
  draw_limits    RECT
endg
;------------------------------------------------------------------------------


align 4
syscall_draw_window: ;     system function 0
        mov     eax, edx
        shr     eax, 24
        and     al, 0x0f
        cmp     al, 5
        jae     .exit

        push    eax
        call    window._.sys_set_window
        pop     eax

        or      al, al
        jnz     @f

; type I - original style
        call    drawwindow_I
        jmp     window._.draw_window_caption.2
;--------------------------------------
@@:
        dec     al
        jnz     @f

; type II - only reserve area, no draw
        call    __sys_draw_pointer
        jmp     .exit
;--------------------------------------
@@:
        dec     al
        jnz     @f

; type III  - new style
        call    drawwindow_III
        jmp     window._.draw_window_caption.2
;--------------------------------------
@@:
; type IV & V - skinned window (resizable & not)
        mov     eax, [thread_count]
        movzx   eax, word[WIN_POS + eax * 2]
        cmp     eax, [current_slot_idx]
        setz    al
        movzx   eax, al
        push    eax
        call    drawwindow_IV
        jmp     window._.draw_window_caption.2
;--------------------------------------
.exit:
        ret
;------------------------------------------------------------------------------


align 4
; system function 1
syscall_setpixel:  
        mov     eax, ebx
        mov     ebx, ecx
        mov     ecx, edx
        mov     edx, [current_slot]
        mov     edx, [edx + APPDATA.window]
        add     eax, [edx + WDATA.box.left]
        add     ebx, [edx + WDATA.box.top]
        add     eax, [edx + WDATA.clientbox.left]
        add     ebx, [edx + WDATA.clientbox.top]
        xor     edi, edi ; no force
        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 save to mouseunder area
        jmp     __sys_putpixel
;------------------------------------------------------------------------------


align 4
; system function 4
syscall_writetext: 

        push    esi    ;check pointer on kernel address.
        test    ecx, 0x80000000
        jz      @f
        xor     esi, esi
@@:
        stdcall is_region_userspace, edx, esi
        pop     esi
        jnz     .err

        mov     eax, [current_slot]
        mov     eax, [eax + APPDATA.window]
        mov     ebp, [eax + WDATA.box.left]
        add     ebp, [eax + WDATA.clientbox.left]
        shl     ebp, 16
        add     ebp, [eax + WDATA.box.top]
        add     bp, word[eax + WDATA.clientbox.top]
        test    ecx, 0x08000000  ; redirect the output to the user area
        jnz     @f
        add     ebx, ebp
align 4
@@:
        mov     eax, edi
        test    ecx, 0x08000000  ; redirect the output to the user area
        jnz     @f
        xor     edi, edi
        jmp     dtext

@@:     ;  check pointer
        stdcall is_region_userspace, edi, 0
        jnz     .err
        jmp     dtext
.err:
        ret
;------------------------------------------------------------------------------


align 4
; system function 13
syscall_drawrect: 
        mov     edi, edx ; color + gradient
        and     edi, 0x80FFFFFF
        test    bx, bx  ; x.size
        je      .drectr
        test    cx, cx ; y.size
        je      .drectr

        mov     eax, ebx ; bad idea
        mov     ebx, ecx

        movzx   ecx, ax ; ecx - x.size
        shr     eax, 16 ; eax - x.coord
        movzx   edx, bx ; edx - y.size
        shr     ebx, 16 ; ebx - y.coord
        mov     esi, [current_slot]
        mov     esi, [esi + APPDATA.window]
        add     eax, [esi + WDATA.clientbox.left]
        add     ebx, [esi + WDATA.clientbox.top]
        add     ecx, eax
        add     edx, ebx
        jmp     vesa20_drawbar
.drectr:
        ret
;------------------------------------------------------------------------------

align 4
; system function 38
syscall_drawline:
        mov     edi, [current_slot]
        mov     edi, [edi + APPDATA.window]
        movzx   eax, word[edi + WDATA.box.left]
        mov     ebp, eax
        add     ebp, [edi + WDATA.clientbox.left]
        add     ax, word[edi + WDATA.clientbox.left]
        add     ebp, ebx
        shl     eax, 16
        movzx   ebx, word[edi + WDATA.box.top]
        add     eax, ebp
        mov     ebp, ebx
        add     ebp, [edi + WDATA.clientbox.top]
        add     bx, word[edi + WDATA.clientbox.top]
        add     ebp, ecx
        shl     ebx, 16
        xor     edi, edi
        add     ebx, ebp
        mov     ecx, edx
        jmp     __sys_draw_line
;------------------------------------------------------------------------------


align 4
; system function 48 
syscall_display_settings: 
        cmp     ebx, 13
        ja      .ret
        jmp     dword[.ftable + ebx*4]

align 4
.ftable:
dd  .redrawWholeScreen
dd  .setButtonStyle
dd  .setSystemColors
dd  .getSystemColors
dd  .getCaptionHeight
dd  .getScreenWorkingArea
dd  .setScreenWorkingArea
dd  .getSkinMargins
dd  .setSkin
dd  .getFontSmoothing
dd  .setFontSmoothing
dd  .getFontSize
dd  .setFontSize
dd  .setSkinUnicode

.redrawWholeScreen:
        xor     eax, eax
        inc     ebx
        cmp     [windowtypechanged], ebx
        jne     .ret
        mov     [windowtypechanged], eax
.redrawScreen:
        xor     eax, eax
        mov     [draw_limits.left], eax
        mov     [draw_limits.top], eax
        mov     eax, [_display.width]
        dec     eax
        mov     [draw_limits.right], eax
        mov     eax, [_display.height]
        dec     eax
        mov     [draw_limits.bottom], eax
        mov     eax, window_data
        jmp     redrawscreen

.setButtonStyle:
; in: ecx: 0 = flat, 1 = with gradient
        and     ecx, 1
        cmp     ecx, [buttontype]
        je      .ret
        mov     [buttontype], ecx
        mov     [windowtypechanged], ebx
.ret:
        ret

.setSystemColors:
; in: ecx = pointer to color table, edx = size of color table
        dec     ebx
        mov     esi, ecx
        cmp     edx, 192
        jnae    @f
        mov     edx, 192 ; max size
@@:
        stdcall is_region_userspace, esi, edx    ;
        jz      @f                               ;
        ret                                      ;
@@:
        mov     edi, common_colours
        mov     ecx, edx
        rep movsb
        mov     [windowtypechanged], ebx
        ret

.getSystemColors:
; in: ecx = pointer to color table, edx = size of color table
        mov     edi, ecx
        cmp     edx, 192
        jnae    @f
        mov     edx, 192 ; max size
@@:
        stdcall is_region_userspace, edi, edx
        jz      @f
        ret
@@:
        mov     esi, common_colours
        mov     ecx, edx
        rep movsb
        ret

.getCaptionHeight:
        mov     eax, [_skinh]
        mov     [esp + SYSCALL_STACK.eax], eax
        ret

.getScreenWorkingArea:
; out: eax = pack[left, right], ebx = pack[top, bottom]
        mov     eax, [screen_workarea.left - 2]
        mov     ax, word[screen_workarea.right]
        mov     [esp + SYSCALL_STACK.eax], eax
        mov     eax, [screen_workarea.top - 2]
        mov     ax, word[screen_workarea.bottom]
        mov     [esp + SYSCALL_STACK.ebx], eax
        ret

.setScreenWorkingArea:
; in: ecx = pack[left, right], edx = pack[top, bottom]
        xor     esi, esi
        mov     edi, [_display.width]
        dec     edi
        mov     eax, ecx
        movsx   ebx, ax
        sar     eax, 16
        cmp     eax, ebx
        jge     .check_horizontal
        inc     esi
        or      eax, eax
        jge     @f
        xor     eax, eax
@@:
        mov     [screen_workarea.left], eax
        cmp     ebx, edi
        jle     @f
        mov     ebx, edi
@@:
        mov     [screen_workarea.right], ebx
.check_horizontal:
        mov     edi, [_display.height]
        dec     edi
        mov     eax, edx
        movsx   ebx, ax
        sar     eax, 16
        cmp     eax, ebx
        jge     .check_if_redraw_needed
        inc     esi
        or      eax, eax
        jge     @f
        xor     eax, eax
@@:
        mov     [screen_workarea.top], eax
        cmp     ebx, edi
        jle     @f
        mov     ebx, edi
@@:
        mov     [screen_workarea.bottom], ebx
.check_if_redraw_needed:
        or      esi, esi
        jz      @f
        call    repos_windows
.calculateScreen:
        xor     eax, eax
        xor     ebx, ebx
        mov     ecx, [_display.width]
        mov     edx, [_display.height]
        dec     ecx
        dec     edx
        jmp     calculatescreen

.getSkinMargins:
; out: eax = pack[left, right], ebx = pack[top, bottom]
        mov     eax, [_skinmargins + 0]
        mov     [esp + SYSCALL_STACK.eax], eax
        mov     eax, [_skinmargins + 4]
        mov     [esp + SYSCALL_STACK.ebx], eax
        ret

.setSkin:
; in: ecx -> file path string
        mov     ebx, ecx
        call    read_skin_file
        mov     [esp + SYSCALL_STACK.eax], eax
        test    eax, eax
        jnz     .ret
        call    .calculateScreen
        jmp     .redrawScreen

.getFontSmoothing:
        xor     eax, eax
        mov     al, [fontSmoothing]
        mov     [esp + SYSCALL_STACK.eax], eax
        ret

.setFontSmoothing:
        mov     [fontSmoothing], cl
        ret

.getFontSize:
        xor     eax, eax
        mov     al, [fontSize]
        mov     [esp + SYSCALL_STACK.eax], eax
        ret

.setFontSize:
        mov     [fontSize], cl
        ret

.setSkinUnicode:
; in: ecx -> file path string, edx = string encoding
        push    ecx edx
        stdcall kernel_alloc, maxPathLength
        mov     edi, eax
        pop     eax esi
        push    edi
        call    getFullPath
        test    eax, eax
        jz      @f
        mov     ebx, [esp]
        call    read_skin_file
        mov     [esp + SYSCALL_STACK.eax + 4], eax
@@:
        call    kernel_free
        call    .calculateScreen
        jmp     .redrawScreen

;------------------------------------------------------------------------------

align 4
; system function 50
;; Set window shape address:
;> ebx = 0
;> ecx = shape data address
; --------------------------
;; Set window shape scale:
;> ebx = 1
;> ecx = scale power (resulting scale is 2^ebx)
syscall_set_window_shape: 
        mov     edi, [current_slot_idx]
        shl     edi, BSF sizeof.WDATA

        test    ebx, ebx
        jne     .shape_scale
        mov     [window_data + edi + WDATA.shape], ecx
;--------------------------------------
align 4
.shape_scale:
        dec     ebx
        jnz     .exit
        mov     [window_data + edi + WDATA.shape_scale], ecx
;--------------------------------------
align 4
.exit:
        ret
;------------------------------------------------------------------------------

align 4
;     system function 67 
syscall_move_window: 
        mov     edi, [current_slot_idx]
        shl     edi, BSF sizeof.WDATA
        add     edi, window_data

        test    [edi + WDATA.fl_wdrawn], 1
        jz      .exit

        test    [edi + WDATA.fl_wstate], WSTATE_MAXIMIZED
        jnz     .exit

        cmp     ebx, -1
        jne     @f
        mov     ebx, [edi + WDATA.box.left]
;--------------------------------------
align 4
@@:
        cmp     ecx, -1
        jne     @f
        mov     ecx, [edi + WDATA.box.top]
;--------------------------------------
align 4
@@:
        cmp     edx, -1
        jne     @f
        mov     edx, [edi + WDATA.box.width]
;--------------------------------------
align 4
@@:
        cmp     esi, -1
        jne     @f
        mov     esi, [edi + WDATA.box.height]
;--------------------------------------
align 4
@@:
        push    esi edx ecx ebx
        mov     eax, esp
        mov     bl, [edi + WDATA.fl_wstate]
;--------------------------------------
align 4
@@:
        cmp     [REDRAW_BACKGROUND], byte 0
        jz      @f
        call    change_task
        jmp     @b
;--------------------------------------
align 4
@@:
        call    window._.set_window_box
        add     esp, sizeof.BOX

.exit:
        ret
;------------------------------------------------------------------------------

; system function 71
syscall_window_settings: 
        mov     edi, [current_slot]
        mov     edi, [edi + APPDATA.window]
        or      [edi + WDATA.fl_wstyle], WSTYLE_HASCAPTION
        cmp     ebx, 2
        jz      @f
        xor     edx, edx
@@:
        cmp     dl, 4
        jc      @f
        xor     edx, edx
@@:
        mov     [edi + WDATA.caption], ecx
        mov     [edi + WDATA.captionEncoding], dl
        jmp     window._.draw_window_caption
;------------------------------------------------------------------------------

align 4
set_window_defaults:
        mov     byte [background_window + WDATA.cl_titlebar + 3], 1 ; desktop is not movable
        push    eax ecx
        xor     eax, eax
        mov     ecx, WIN_STACK
;--------------------------------------
align 4
@@:
        inc     eax
        add     ecx, 2
        ; process no
        mov     [ecx + 0x000], ax
        ; positions in stack
        mov     [ecx + 0x400], ax
        cmp     ecx, WIN_POS - 2
        jne     @b
        pop     ecx eax
        ret
;------------------------------------------------------------------------------

align 4
;? Scan all windows from bottom to top, calling `setscreen` for each one
;? intersecting given screen area
;> eax = left
;> ebx = top
;> ecx = right
;> edx = bottom
calculatescreen:
        push    esi
        pushfd
        cli

        mov     esi, 1
        call    window._.set_screen

        push    ebp

        mov     ebp, [thread_count]
        cmp     ebp, 1
        jbe     .exit

        push    eax ;for num layout

        push    edx ecx ebx eax

        mov     dword[esp+10h], ZPOS_DESKTOP
;--------------------------------------
align 4
.layout:
        mov     esi, 1          ; = num in window stack
        mov     ebp, [thread_count]
;--------------------------------------
align 4
.next_window:
        movzx   edi, word[WIN_POS + esi * 2]
        shl     edi, BSF sizeof.WDATA

        test    byte [window_data + edi + WDATA.fl_wstate], WSTATE_USED
        jz      .skip_window

        add     edi, window_data
        test    [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz     .skip_window

        mov     eax, [esp+10h]
        cmp     [edi + WDATA.z_modif], al
        jne     .skip_window

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
;--------------------------------------
align 4
@@:
        cmp     ebx, [esp + RECT.top]
        jae     @f
        mov     ebx, [esp + RECT.top]
;--------------------------------------
align 4
@@:
        cmp     ecx, [esp + RECT.right]
        jbe     @f
        mov     ecx, [esp + RECT.right]
;--------------------------------------
align 4
@@:
        cmp     edx, [esp + RECT.bottom]
        jbe     @f
        mov     edx, [esp + RECT.bottom]
;--------------------------------------
align 4
@@:
        push    esi
        movzx   esi, word[WIN_POS + esi * 2]
        call    window._.set_screen
        pop     esi
;--------------------------------------
align 4
.skip_window:
        inc     esi
        dec     ebp
        jnz     .next_window
;---------------------------------------------
        inc     dword[esp+10h]
        cmp     dword[esp+10h], ZPOS_ALWAYS_TOP
        jle     .layout
;---------------------------------------------
        mov     esi, [thread_count]
        movzx   edi, word[WIN_POS + esi * 2]
        shl     edi, BSF sizeof.WDATA
        add     edi, window_data

        pop     eax ebx ecx edx
        pop     ebp     ;del num layout
;--------------------------------------
align 4
.exit:
        pop     ebp
        inc     [_display.mask_seqno]
        popfd
        pop     esi
        ret
;------------------------------------------------------------------------------


align 4
repos_windows:
        mov     ecx, [thread_count]
        mov     edi, window_data + sizeof.WDATA * 2
        call    force_redraw_background
        dec     ecx
        jle     .exit
;--------------------------------------
align 4
.next_window:
        mov     [edi + WDATA.fl_redraw], 1
        test    [edi + WDATA.fl_wstate], WSTATE_MAXIMIZED
        jnz     .fix_maximized

        mov     eax, [edi + WDATA.box.left]
        add     eax, [edi + WDATA.box.width]
        mov     ebx, [_display.width]
        cmp     eax, ebx
        jl      .fix_vertical
        mov     eax, [edi + WDATA.box.width]
        sub     eax, ebx
        jl      @f
        mov     [edi + WDATA.box.width], ebx
;--------------------------------------
align 4
@@:
        sub     ebx, [edi + WDATA.box.width]
        mov     [edi + WDATA.box.left], ebx
;--------------------------------------
align 4
.fix_vertical:
        mov     eax, [edi + WDATA.box.top]
        add     eax, [edi + WDATA.box.height]
        mov     ebx, [_display.height]
        cmp     eax, ebx
        jl      .fix_client_box
        mov     eax, [edi + WDATA.box.height]
        sub     eax, ebx
        jl      @f
        mov     [edi + WDATA.box.height], ebx
;--------------------------------------
align 4
@@:
        sub     ebx, [edi + WDATA.box.height]
        mov     [edi + WDATA.box.top], ebx
;--------------------------------------
align 4
.fix_client_box:
        call    window._.set_window_clientbox
        add     edi, sizeof.WDATA
        loop    .next_window
;--------------------------------------
align 4
.exit:
        ret
;--------------------------------------
align 4
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
        jmp     .fix_client_box
;------------------------------------------------------------------------------


align 4
;> eax = pack[16(left), 16(right)]
;> ebx = pack[16(top), 16(bottom)]
;> esi = color
;       ?? RR GG BB    ; 0x01000000 negation
;                      ; 0x02000000 used for draw_rectangle without top line
;                      ;           for example drawwindow_III and drawwindow_IV
draw_rectangle:
        push    eax ebx ecx edi

        xor     edi, edi
;--------------------------------------
align 4
.flags_set:
        push    ebx

        ; set line color
        mov     ecx, esi
        ; draw top border
        rol     ebx, 16
        push    ebx
        rol     ebx, 16
        pop     bx
        test    ecx, 1 shl 25
        jnz     @f
        sub     ecx, 1 shl 25
;        call    [draw_line]
        call    __sys_draw_line
;--------------------------------------
align 4
@@:
        ; draw bottom border
        mov     ebx, [esp - 2]
        pop     bx
;        call    [draw_line]
        call    __sys_draw_line

        pop     ebx
        add     ebx, 1 * 65536 - 1

        ; draw left border
        rol     eax, 16
        push    eax
        rol     eax, 16
        pop     ax
;        call    [draw_line]
        call    __sys_draw_line

        ; draw right border
        mov     eax, [esp - 2]
        pop     ax
;        call    [draw_line]
        call    __sys_draw_line

        pop     edi ecx ebx eax
        ret
;--------------------------------------
align 4
.forced:
        push    eax ebx ecx edi
        xor     edi, edi
        inc     edi
        jmp     .flags_set
;------------------------------------------------------------------------------


align 4
drawwindow_I_caption:
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
;--------------------------------------
align 4
@@:
        push    ebx

        xor     edi, edi
;--------------------------------------
align 4
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
;--------------------------------------
align 4
@@:
        and     ecx, 0x00ffffff
;        call    [draw_line]
        call    __sys_draw_line
        inc     edx
        cmp     edx, [esp]
        jb      .next_line

        add     esp, 4
        pop     [esi + WDATA.cl_titlebar]
        ret
;------------------------------------------------------------------------------


align 4
drawwindow_I:
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
        cmp     [esi + WDATA.box.height], 21
        jle     .exit

        ; okay, let's draw it
        mov     eax, 1
        mov     ebx, 21
        mov     ecx, [esi + WDATA.box.width]
        mov     edx, [esi + WDATA.box.height]
;        call    [drawbar]
        call    vesa20_drawbar
;--------------------------------------
align 4
.exit:
        popad
        ret
;------------------------------------------------------------------------------


align 4
drawwindow_III_caption:
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
;--------------------------------------
align 4
@@:
        push    ebx

        xor     edi, edi
;--------------------------------------
align 4
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
;--------------------------------------
align 4
@@:
        test    ecx, 0x80000000
        jz      @f
        sub     ecx, 0x00040404
;--------------------------------------
align 4
@@:
        mov     [esi + WDATA.cl_titlebar], ecx
        and     ecx, 0x00ffffff
;        call    [draw_line]
        call    __sys_draw_line
        inc     edx
        cmp     edx, [esp]
        jb      .next_line

        add     esp, 4
        pop     [esi + WDATA.cl_titlebar]
        ret
;------------------------------------------------------------------------------


align 4
drawwindow_III:
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
;--------------------------------------
align 4
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
;        call    [drawbar]
        call    vesa20_drawbar
;--------------------------------------
align 4
.exit:
        popad
        ret
;------------------------------------------------------------------------------


align 4
;? Activate window, redrawing if necessary
waredraw:
        push    -1
        mov     eax, [thread_count]
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
        call    window._.window_activate

        pushad
        mov     edi, [thread_count]
        movzx   esi, word[WIN_POS + edi * 2]
        shl     esi, BSF sizeof.WDATA
        add     esi, window_data

        mov     eax, [esi + WDATA.box.left]
        mov     ebx, [esi + WDATA.box.top]
        mov     ecx, [esi + WDATA.box.width]
        mov     edx, [esi + WDATA.box.height]

        add     ecx, eax
        add     edx, ebx

        mov     edi, [thread_count]
        movzx   esi, word[WIN_POS + edi * 2]
        call    window._.set_screen

        call    window._.set_top_wnd

        inc     [_display.mask_seqno]
        popad

        ; tell application to redraw itself
        mov     [edi + WDATA.fl_redraw], 1
        xor     eax, eax
        jmp     .exit
;--------------------------------------
align 4
.do_not_draw:
        ; no it's not, just activate the window
        call    window._.window_activate
        xor     eax, eax
        ret

;--------------------------------------
align 4
.exit:
        inc     eax
        ret
;------------------------------------------------------------------------------


align 4
minimize_all_window:
        push    ebx ecx edx esi edi
        pushfd
        cli
        xor     edx, edx
        mov     eax, 2  ; we do not minimize the kernel thread N1
        mov     ebx, [thread_count]
;--------------------------------------
align 4
.loop:
        movzx   edi, word[WIN_POS + eax * 2]
        shl     edi, BSF sizeof.WDATA
; it is a unused slot?
        test    byte [window_data + edi + WDATA.fl_wstate], WSTATE_USED
        jz      @f
; it is a hidden thread?
        lea     esi, [SLOT_BASE + edi*(sizeof.APPDATA/sizeof.WDATA) + APPDATA.app_name]
        cmp     [esi], byte '@'
        je      @f
; is it already minimized?
        test    [window_data + edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz     @f
; no it's not, let's do that
        or      [window_data + edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        inc     edx
;--------------------------------------
align 4
@@:
        inc     eax
        cmp     eax, ebx
        jbe     .loop
; If nothing has changed
        test    edx, edx
        jz      @f

        push    edx
        call    syscall_display_settings.calculateScreen
        call    syscall_display_settings.redrawScreen
        pop     edx
;--------------------------------------
align 4
@@:
        mov     eax, edx
        popfd
        pop     edi esi edx ecx ebx
        ret
;------------------------------------------------------------------------------
align 4
;------------------------------------------------------------------------------

;> eax = window number on screen
;# corrupts [dl*]
minimize_window:
        push    edi
        pushfd
        cli

        ; is it already minimized?
        movzx   edi, word[WIN_POS + eax * 2]
        shl     edi, BSF sizeof.WDATA
        add     edi, window_data
        test    [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz     .exit

        push    eax ebx ecx edx esi

        ; no it's not, let's do that
        or      [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
; If the window width is 0, then the action is not needed.
        cmp     [edi + WDATA.box.width], dword 0
        je      @f
; If the window height is 0, then the action is not needed.
        cmp     [edi + WDATA.box.height], dword 0
        je      @f

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

;        DEBUGF  1, "K : minimize_window\n"
;        DEBUGF  1, "K : dl_left %x\n",[draw_limits.left]
;        DEBUGF  1, "K : dl_right %x\n",[draw_limits.right]
;        DEBUGF  1, "K : dl_top %x\n",[draw_limits.top]
;        DEBUGF  1, "K : dl_bottom %x\n",[draw_limits.bottom]
        call    calculatescreen
;        xor     esi, esi
;        xor     eax, eax
        mov     eax, edi
        call    redrawscreen
;--------------------------------------
align 4
@@:
        pop     esi edx ecx ebx eax
;--------------------------------------
align 4
.exit:
        popfd
        pop     edi
        ret
;------------------------------------------------------------------------------


align 4
;> eax = window number on screen
;# corrupts [dl*]
restore_minimized_window:
        pushad
        pushfd
        cli

        ; is it already restored?
        movzx   esi, word[WIN_POS + eax * 2]
        mov     edi, esi
        shl     edi, BSF sizeof.WDATA
        add     edi, window_data
        test    [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jz      .exit

        ; no it's not, let's do that
        mov     [edi + WDATA.fl_redraw], 1
        and     [edi + WDATA.fl_wstate], not WSTATE_MINIMIZED
        mov     ebp, window._.set_screen
        cmp     eax, [thread_count]
        jz      @f
        mov     ebp, calculatescreen
;--------------------------------------
align 4
@@:
        mov     eax, [edi + WDATA.box.left]
        mov     ebx, [edi + WDATA.box.top]
        mov     ecx, [edi + WDATA.box.width]
        mov     edx, [edi + WDATA.box.height]
        add     ecx, eax
        add     edx, ebx
        call    ebp

        cmp     ebp, window._.set_screen
        jne     @f
        call    window._.set_top_wnd
    @@:
        inc     [_display.mask_seqno]
;--------------------------------------
align 4
.exit:
        popfd
        popad
        ret
;------------------------------------------------------------------------------
align 4
; TODO: remove this proc
;------------------------------------------------------------------------------


window_check_events:
        ; do we have window minimize/restore request?
        cmp     [window_minimize], 0
        je      .exit

        ; okay, minimize or restore top-most window and exit
        mov     eax, [thread_count]
        mov     bl, 0
        xchg    [window_minimize], bl
        dec     bl
        jnz     @f
        call    minimize_window
        jmp     .exit
;--------------------------------------
align 4
@@:
        call    restore_minimized_window
;--------------------------------------
align 4
.exit:
        ret
;------------------------------------------------------------------------------


align 4
;> esi = process slot
sys_window_maximize_handler:
        mov     edi, esi
        shl     edi, BSF sizeof.WDATA
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
;--------------------------------------
align 4
.restore_size:
        push    [edi + WDATA.saved_box.height] \
                [edi + WDATA.saved_box.width] \
                [edi + WDATA.saved_box.top] \
                [edi + WDATA.saved_box.left]
        mov     eax, esp
;--------------------------------------
align 4
.set_box:
        test    bl, WSTATE_ROLLEDUP
        jz      @f

        xchg    eax, ecx
        call    window._.get_rolledup_height
        mov     [ecx + BOX.height], eax
        xchg    eax, ecx
;--------------------------------------
align 4
@@:
        call    window._.set_window_box
        add     esp, sizeof.BOX
;--------------------------------------
align 4
.exit:
        inc     [_display.mask_seqno]
        ret
;------------------------------------------------------------------------------


align 4
;> esi = process slot
sys_window_rollup_handler:
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
;--------------------------------------
align 4
.restore_size:
        test    bl, WSTATE_MAXIMIZED
        jnz     @f
        add     esp, -sizeof.BOX
        lea     eax, [edi + WDATA.saved_box]
        jmp     .set_box
;--------------------------------------
align 4
@@:
        mov     eax, [screen_workarea.top]
        push    [screen_workarea.bottom] \
                [edi + WDATA.box.width] \
                eax \
                [edi + WDATA.box.left]
        sub     [esp + BOX.height], eax
        mov     eax, esp
;--------------------------------------
align 4
.set_box:
        call    window._.set_window_box
        add     esp, sizeof.BOX
        ret
;------------------------------------------------------------------------------
align 4
;------------------------------------------------------------------------------
;sys_window_start_moving_handler:
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> eax = old (original) window box
;> esi = process slot
;------------------------------------------------------------------------------
;        mov     edi, eax
;        call    window._.draw_negative_box
;        ret
;------------------------------------------------------------------------------


align 4
;> eax = old (original) window box
;> ebx = new (final) window box
;> esi = process slot
sys_window_end_moving_handler:
;        mov     edi, ebx
;        call    window._.end_moving__box

        mov     edi, esi
        shl     edi, BSF sizeof.WDATA
        add     edi, window_data

        test    [fl_moving], 1
        jz      @f

        push    edi
        mov     edi, ebx
        call    window._.draw_negative_box
        pop     edi
    @@:

        mov     eax, ebx
        mov     bl, [edi + WDATA.fl_wstate]
        call    window._.set_window_box
        ret
;------------------------------------------------------------------------------


align 4
;> eax = old (from previous call) window box
;> ebx = new (current) window box
;> esi = process_slot
sys_window_moving_handler:
        mov     edi, eax
        call    window._.draw_negative_box
        mov     edi, ebx
        call    window._.draw_negative_box
        ret




;==============================================================================
;      private functions
;==============================================================================

iglobal
align 4
window_topleft  dd \
    1, 21, \ ;type 0
    0,  0, \ ;type 1
    5, 20, \ ;type 2
    5,  ?, \ ;type 3 {set by skin}
    5,  ?    ;type 4 {set by skin}
endg

;------------------------------------------------------------------------------


align 4
;> eax = old (original) window box
;> ebx = new (final) window box
;> edi = pointer to WDATA struct
window._.invalidate_screen:
        push    eax ebx

        ; TODO: do we really need `draw_limits`?
        ; Yes, they are used by background drawing code.

; we need only to restore the background windows at the old place!
        mov     ecx, [ebx + BOX.left]
        mov     [draw_limits.left], ecx
        add     ecx, [ebx + BOX.width]
        mov     [draw_limits.right], ecx
        mov     ecx, [ebx + BOX.top]
        mov     [draw_limits.top], ecx
        add     ecx, [ebx + BOX.height]
        mov     [draw_limits.bottom], ecx
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
;------------------------------------------------------------------------------


align 4
;> eax = pointer to BOX struct
;> bl = new window state flags
;> edi = pointer to WDATA struct
window._.set_window_box:
        push    eax ebx esi

; don't do anything if the new box is identical to the old
        cmp     bl, [edi + WDATA.fl_wstate]
        jnz     @f
        mov     esi, eax
        push    edi
if WDATA.box
        add     edi, WDATA.box
end if
        mov     ecx, 4
        repz cmpsd
        pop     edi
        jz      .exit
;--------------------------------------
align 4
@@:
        add     esp, -sizeof.BOX
        mov     ebx, esp
if WDATA.box
        lea     esi, [edi + WDATA.box]
else
        mov     esi, edi ; optimization for WDATA.box = 0
end if
        xchg    eax, esi
        mov     ecx, sizeof.BOX
        call    memmove
        xchg    eax, esi
        xchg    ebx, esi
        call    memmove
        mov     eax, ebx
        mov     ebx, esi
        call    window._.check_window_position
        call    window._.set_window_clientbox
        call    window._.invalidate_screen

        add     esp, sizeof.BOX

        mov     cl, [esp + 4]
        mov     ch, cl
        xchg    cl, [edi + WDATA.fl_wstate]

        or      cl, ch
        test    cl, WSTATE_MAXIMIZED
        jnz     .exit

        lea     ebx, [edi + WDATA.box]
        xchg    esp, ebx

        pop     [edi + WDATA.saved_box.left] \
                [edi + WDATA.saved_box.top] \
                [edi + WDATA.saved_box.width] \
                edx

        xchg    esp, ebx

        test    ch, WSTATE_ROLLEDUP
        jnz     .exit

        mov     [edi + WDATA.saved_box.height], edx
;--------------------------------------
align 4
.exit:
        pop     esi ebx eax
        ret
;------------------------------------------------------------------------------


align 4
;> edi = pointer to WDATA struct
window._.set_window_clientbox:
        push    eax ecx edi

        mov     eax, [_skinh]
        mov     [window_topleft + 8 * 3 + 4], eax
        mov     [window_topleft + 8 * 4 + 4], eax

        mov     ecx, edi
        test    [ecx + WDATA.fl_wstyle], WSTYLE_CLIENTRELATIVE
        jz      .whole_window

        movzx   eax, [ecx + WDATA.fl_wstyle]
        and     eax, 0x0F
        mov     eax, [eax * 8 + window_topleft + 0]
        mov     [ecx + WDATA.clientbox.left], eax
        shl     eax, 1
        neg     eax
        add     eax, [ecx + WDATA.box.width]
        inc     eax  ;Leency: as window is created width+1 so client the same
        mov     [ecx + WDATA.clientbox.width], eax

        movzx   eax, [ecx + WDATA.fl_wstyle]
        and     eax, 0x0F
        push    [eax * 8 + window_topleft + 0]
        mov     eax, [eax * 8 + window_topleft + 4]
        mov     [ecx + WDATA.clientbox.top], eax
        neg     eax
        sub     eax, [esp]
        add     eax, [ecx + WDATA.box.height]
        inc     eax  ;Leency: as window is created height+1 so client the same
        mov     [ecx + WDATA.clientbox.height], eax
        add     esp, 4
        jmp     .exit
;--------------------------------------
align 4
.whole_window:
        xor     eax, eax
        mov     [ecx + WDATA.clientbox.left], eax
        mov     [ecx + WDATA.clientbox.top], eax
        mov     eax, [ecx + WDATA.box.width]
        mov     [ecx + WDATA.clientbox.width], eax
        mov     eax, [ecx + WDATA.box.height]
        mov     [ecx + WDATA.clientbox.height], eax
;--------------------------------------
align 4
.exit:
        pop     edi ecx eax
        ret
;------------------------------------------------------------------------------


align 4
;< edx = pointer to WDATA struct
window._.sys_set_window:
        mov     eax, [current_slot_idx]
        shl     eax, BSF sizeof.WDATA
        add     eax, window_data
; save window colors
        mov     [eax + WDATA.cl_workarea], edx
        mov     [eax + WDATA.cl_titlebar], esi
        mov     [eax + WDATA.cl_frames], edi
        mov     edi, eax
; Was it already defined before?
        test    [edi + WDATA.fl_wdrawn], 1
        jnz     .set_client_box
; No, it wasn't. After first draw_window we need redraw mouse necessarily!
; Otherwise the user can see cursor specified by f.37.5 from another window.
; He will be really unhappy! Usually, he will be enraged!
        or      [edi + WDATA.fl_wdrawn], 1
        mov     [redrawmouse_unconditional], 1
        call    wakeup_osloop
; performing initial window definition
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

        mov     esi, [esp]

        and     cl, 0x0F
        cmp     cl, 3
        je      @f
        cmp     cl, 4
        je      @f

        xor     eax, eax
;--------------------------------------
align 4
@@:
        mov     [esi + WDATA.caption], eax

        add     edi, WDATA.saved_box
        movsd
        movsd
        movsd
        movsd

        pop     edi ecx

        mov     esi, [current_slot_idx]
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
;--------------------------------------
align 4
.set_client_box:
        ; update window client box coordinates
        call    window._.set_window_clientbox

        ; reset window redraw flag and exit
        mov     [edi + WDATA.fl_redraw], 0
        mov     edx, edi
        ret
;------------------------------------------------------------------------------


align 4
;? Check if window is inside screen area
;> edi = pointer to WDATA
window._.check_window_position:
        push    eax ebx ecx edx esi

        mov     eax, [edi + WDATA.box.left]
        mov     ebx, [edi + WDATA.box.top]
        mov     ecx, [edi + WDATA.box.width]
        mov     edx, [edi + WDATA.box.height]

        mov     esi, [_display.width]
        cmp     ecx, esi
        jae     .fix_width_high
;--------------------------------------
align 4
.check_left:
        or      eax, eax
        jl      .fix_left_low
        add     eax, ecx
        cmp     eax, esi
        jge     .fix_left_high
;--------------------------------------
align 4
.check_height:
        mov     esi, [_display.height]
        cmp     edx, esi
        jae     .fix_height_high
;--------------------------------------
align 4
.check_top:
        or      ebx, ebx
        jl      .fix_top_low
        add     ebx, edx
        cmp     ebx, esi
        jge     .fix_top_high
;--------------------------------------
align 4
.exit:
        pop     esi edx ecx ebx eax
        ret
;--------------------------------------
align 4
.fix_width_high:
        mov     ecx, esi
        dec     ecx
        mov     [edi + WDATA.box.width], ecx
        jmp     .check_left
;--------------------------------------
align 4
.fix_left_low:
        xor     eax, eax
        mov     [edi + WDATA.box.left], eax
        jmp     .check_height
;--------------------------------------
align 4
.fix_left_high:
        mov     eax, esi
        sub     eax, ecx
        dec     eax
        mov     [edi + WDATA.box.left], eax
        jmp     .check_height
;--------------------------------------
align 4
.fix_height_high:
        mov     edx, esi
        dec     edx
        mov     [edi + WDATA.box.height], edx
        jmp     .check_top
;--------------------------------------
align 4
.fix_top_low:
        xor     ebx, ebx
        mov     [edi + WDATA.box.top], ebx
        jmp     .exit
;--------------------------------------
align 4
.fix_top_high:
        mov     ebx, esi
        sub     ebx, edx
        dec     ebx
        mov     [edi + WDATA.box.top], ebx
        jmp     .exit
;------------------------------------------------------------------------------


align 4
;> edi = pointer to WDATA
window._.get_titlebar_height:
        mov     al, [edi + WDATA.fl_wstyle]
        and     al, 0x0f
        cmp     al, 0x03
        jne     @f
        mov     eax, [_skinh]
        ret
;--------------------------------------
align 4
@@:
        mov     eax, 21
        ret
;------------------------------------------------------------------------------


align 4
;> edi = pointer to WDATA
window._.get_rolledup_height:
        mov     al, [edi + WDATA.fl_wstyle]
        and     al, 0x0f
        cmp     al, 0x03
        jb      @f
        mov     eax, [_skinh]
        add     eax, 3
        ret
;--------------------------------------
align 4
@@:
        or      al, al
        jnz     @f
        mov     eax, 21
        ret
;--------------------------------------
align 4
@@:
        mov     eax, 21 + 2
        ret
;------------------------------------------------------------------------------


align 4
;? Reserve window area in screen buffer
;> eax = left
;> ebx = top
;> ecx = right
;> edx = bottom
;> esi = process number
window._.set_screen:
virtual at esp
  ff_x     dd ?
  ff_y     dd ?
  ff_width dd ?
  ff_xsz   dd ?
  ff_ysz   dd ?
  ff_scale dd ?
end virtual

        pushad

        mov     edi, esi
        shl     edi, BSF sizeof.WDATA

        cmp     esi, 1
        jz      .check_for_shaped_window

        cmp     [window_data + edi + WDATA.box.width], 0
        jnz     .check_for_shaped_window
        cmp     [window_data + edi + WDATA.box.height], 0
        jz      .exit
;--------------------------------------
align 4
.check_for_shaped_window:
        cmp     [window_data + edi + WDATA.shape], 0
        jne     .shaped_window

        ; get x&y size
        sub     ecx, eax
        sub     edx, ebx
        inc     ecx
        inc     edx

        ; get WinMap start
        push    esi
        mov     esi, [_display.width]
        mov     edi, [d_width_calc_area + ebx*4]

        add     edi, eax
        add     edi, [_display.win_map]
        pop     eax
        mov     ah, al
        push    ax
        shl     eax, 16
        pop     ax
;--------------------------------------
align 4
.next_line:
        push    ecx
        shr     ecx, 2
        rep stosd
        mov     ecx, [esp]
        and     ecx, 3
        rep stosb
        pop     ecx
        add     edi, esi
        sub     edi, ecx
        dec     edx
        jnz     .next_line

        jmp     .exit
;--------------------------------------
align 4
.shaped_window:
        ;  for (y=0; y <= x_size; y++)
        ;      for (x=0; x <= x_size; x++)
        ;          if (shape[coord(x,y,scale)]==1)
        ;             set_pixel(x, y, process_number);

        sub     ecx, eax
        sub     edx, ebx
        inc     ecx
        inc     edx

        push    [window_data + edi + WDATA.shape_scale]  ; push scale first -> for loop

        ; get WinMap start  -> ebp
        push    eax
        mov     eax, [d_width_calc_area + ebx*4]

        add     eax, [esp]
        add     eax, [_display.win_map]
        mov     ebp, eax

        mov     edi, [window_data + edi + WDATA.shape]
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
        shl     ecx, BSF sizeof.WDATA
        mov     edx, [window_data + ecx + WDATA.box.top]
        push    [window_data + ecx + WDATA.box.width]           ; for loop - width
        mov     ecx, [window_data + ecx + WDATA.box.left]
        sub     ebx, edx
        sub     eax, ecx
        push    ebx eax ; for loop - x,y

        add     [ff_xsz], eax
        add     [ff_ysz], ebx

        mov     ebx, [ff_y]
;--------------------------------------
align 4
.ff_new_y:
        mov     edx, [ff_x]
;--------------------------------------
align 4
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
        test    al, al
        jz      @f
        mov     eax, esi
        mov     [ebp], al
        ; -- end body --
;--------------------------------------
align 4
@@:
        inc     ebp
        inc     edx
        cmp     edx, [ff_xsz]
        jb      .ff_new_x

        sub     ebp, [ff_xsz]
        add     ebp, [ff_x]
        add     ebp, [_display.width]  ; screen.x
        inc     ebx
        cmp     ebx, [ff_ysz]
        jb      .ff_new_y

        add     esp, 24
;--------------------------------------
align 4
.exit:
        popad
        inc     [_display.mask_seqno]
        ret
;--------------------------------------
align 4
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
;------------------------------------------------------------------------------


align 4
; Activate window
; esi = pointer to WIN_POS+ window data
window._.window_activate:
        push    eax ebx

        ; if type of current active window is 3 or 4, it must be redrawn
        mov     ebx, [thread_count]

;       DEBUGF  1, "K : thread_count (0x%x)\n", ebx

        movzx   ebx, word[WIN_POS + ebx * 2]
        shl     ebx, BSF sizeof.WDATA
        add     eax, window_data
        mov     al, [window_data + ebx + WDATA.fl_wstyle]
        and     al, 0x0f
        cmp     al, 0x03
        je      .set_window_redraw_flag
        cmp     al, 0x04
        jne     .move_others_down
;--------------------------------------
align 4
.set_window_redraw_flag:
        mov     [window_data + ebx + WDATA.fl_redraw], 1
;--------------------------------------
align 4
.move_others_down:
        ; bx <- process no
        movzx   ebx, word[esi]
        ; bx <- position in window stack
        movzx   ebx, word[WIN_STACK + ebx * 2]

        ; drop others
        xor     eax, eax
;--------------------------------------
align 4
.next_stack_window:
        cmp     eax, [thread_count]
        jae     .move_self_up
        inc     eax

;       push    ebx
;       xor     ebx,ebx
;       mov     bx,[WIN_STACK + eax * 2]
;       DEBUGF  1, "K : DEC WIN_STACK (0x%x)\n",ebx
;       pop     ebx

        cmp     [WIN_STACK + eax * 2], bx
        jbe     .next_stack_window
        dec     word[WIN_STACK + eax * 2]
        jmp     .next_stack_window
;--------------------------------------
align 4
.move_self_up:
        movzx   ebx, word[esi]
        ; number of thread
        mov     ax, word [thread_count]
        ; this is the last (and the upper)
        mov     [WIN_STACK + ebx * 2], ax

        ; update on screen - window stack
        xor     eax, eax
;--------------------------------------
align 4
.next_window_pos:
        cmp     eax, [thread_count]
        jae     .reset_vars
        inc     eax
        movzx   ebx, word[WIN_STACK + eax * 2]
        mov     [WIN_POS + ebx * 2], ax
        jmp     .next_window_pos
;--------------------------------------
align 4
.reset_vars:
        mov     byte[KEY_COUNT], 0
        mov     byte[BTN_COUNT], 0
        mov     word[MOUSE_SCROLL_H], 0
        mov     word[MOUSE_SCROLL_V], 0

        pop     ebx eax
        ret
;------------------------------------------------------------------------------


; Deactivate window
; esi = pointer to WIN_POS+ window data
window._.window_deactivate:
        push    eax ebx
;--------------------------------------
align 4
.move_others_up:
        ; ax <- process no
        movzx   ebx, word[esi]
        ; ax <- position in window stack
        movzx   ebx, word[WIN_STACK + ebx * 2]
        ; up others
        xor     eax, eax
;--------------------------------------
align 4
.next_stack_window:
        cmp     eax, [thread_count]
        jae     .move_self_down
        inc     eax
        cmp     [WIN_STACK + eax * 2], bx
        jae     .next_stack_window
        inc     word[WIN_STACK + eax * 2]
        jmp     .next_stack_window
;--------------------------------------
align 4
.move_self_down:
        movzx   ebx, word[esi]
        ; this is the last (and the low)
        mov     [WIN_STACK + ebx * 2], word 1
        ; update on screen - window stack
        xor     eax, eax
;--------------------------------------
align 4
.next_window_pos:
        cmp     eax, [thread_count]
        jae     .reset_vars
        inc     eax
        movzx   ebx, word[WIN_STACK + eax * 2]
        mov     [WIN_POS + ebx * 2], ax
        jmp     .next_window_pos
;--------------------------------------
align 4
.reset_vars:
        mov     byte[KEY_COUNT], 0
        mov     byte[BTN_COUNT], 0
        mov     word[MOUSE_SCROLL_H], 0
        mov     word[MOUSE_SCROLL_V], 0
        pop     ebx eax
        ret
;------------------------------------------------------------------------------


align 4
; Check if window is necessary to draw
; edi = pointer to WDATA
window._.check_window_draw:
        mov     cl, [edi + WDATA.fl_wstyle]
        and     cl, 0x0f
        cmp     cl, 3
        je      .exit.redraw      ; window type 3
        cmp     cl, 4
        je      .exit.redraw      ; window type 4

        push    eax ebx edx esi

        mov     eax, edi
        sub     eax, window_data
        shr     eax, BSF sizeof.WDATA

        movzx   eax, word[WIN_STACK + eax * 2]  ; get value of the curr process
        lea     esi, [WIN_POS + eax * 2]        ; get address of this process at 0xC400
;--------------------------------------
align 4
.next_window:
        add     esi, 2

        mov     eax, [thread_count]
        lea     eax, word[WIN_POS + eax * 2] ; number of the upper window

        cmp     esi, eax
        ja      .exit.no_redraw

        movzx   edx, word[esi]
        shl     edx, BSF sizeof.WDATA
        test    byte [window_data + edx + WDATA.fl_wstate], WSTATE_USED
        jz      .next_window

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
;--------------------------------------
align 4
.exit.redraw:
        xor     ecx, ecx
        inc     ecx
        ret
;--------------------------------------
align 4
.exit.no_redraw:
        pop     esi edx ebx eax
        xor     ecx, ecx
        ret


;------------------------------------------------------------------------------


align 4
window._.draw_window_caption:
        xor     eax, eax
        mov     edx, [thread_count]
        movzx   edx, word[WIN_POS + edx * 2]
        cmp     edx, [current_slot_idx]
        jne     @f
        inc     eax
;--------------------------------------
align 4
@@:
        mov     edx, [current_slot_idx]
        shl     edx, BSF sizeof.WDATA
        add     edx, window_data
        movzx   ebx, [edx + WDATA.fl_wstyle]
        and     bl, 0x0F
        cmp     bl, 3
        je      .draw_caption_style_3
        cmp     bl, 4
        je      .draw_caption_style_3

        jmp     .not_style_3
;--------------------------------------
align 4
.draw_caption_style_3:
        push    edx
        call    drawwindow_IV_caption
        add     esp, 4
        jmp     .2
;--------------------------------------
align 4
.not_style_3:
        cmp     bl, 2
        jne     .not_style_2

        call    drawwindow_III_caption
        jmp     .2
;--------------------------------------
align 4
.not_style_2:
        cmp     bl, 0
        jne     .2

        call    drawwindow_I_caption
;--------------------------------------
align 4
.2:
        mov     edi, [current_slot_idx]
        shl     edi, BSF sizeof.WDATA
        test    [window_data + edi + WDATA.fl_wstyle], WSTYLE_HASCAPTION
        jz      .exit
        mov     edx, [window_data + edi + WDATA.caption]
        or      edx, edx
        jz      .exit

        mov     ebp, [edi + window_data + WDATA.box.left - 2]
        mov     bp, word[edi + window_data + WDATA.box.top]
        movzx   eax, [edi + window_data + WDATA.fl_wstyle]
        and     al, 0x0F
        cmp     al, 3
        je      .skinned
        cmp     al, 4
        je      .skinned

        jmp     .not_skinned
;--------------------------------------
align 4
.skinned:
        movzx   eax, word[edi + window_data + WDATA.box.width]
        sub     ax, [_skinmargins.left]
        sub     ax, [_skinmargins.right]
        js      .exit
        mov     ebx, dword[_skinmargins.left - 2]
        mov     bx, word[_skinh]
        sub     bx, [_skinmargins.bottom]
        sub     bx, [_skinmargins.top]
        sar     bx, 1
        add     bx, [_skinmargins.top]
        sub     bx, 8
        jmp     .dodraw
;--------------------------------------
align 4
.not_skinned:
        cmp     al, 1
        je      .exit
        movzx   eax, word[edi + window_data + WDATA.box.width]
        sub     eax, 16
        js      .exit
        mov     ebx, 80002h
.dodraw:
        shr     eax, 3
        mov     esi, eax
        add     ebx, ebp
        mov     ecx, [common_colours + 16]
        mov     al, [window_data + edi + WDATA.captionEncoding]
        test    al, al
        jnz     @f
        mov     al, 1
        cmp     byte [edx], 4
        jnc     @f
        mov     al, [edx]
        test    al, al
        jz      .exit
        inc     edx
@@:
        shl     eax, 28
        or      ecx, eax        
        xor     edi, edi
        stc
        call    dtext._
.exit:
        jmp     __sys_draw_pointer
;------------------------------------------------------------------------------


align 4
; Draw negative box
; edi = pointer to BOX struct
window._.draw_negative_box:
        push    eax ebx esi
        mov     esi, 0x01000000
;--------------------------------------
align 4
.1:
        mov     eax, [edi + BOX.left - 2]
        mov     ax, word[edi + BOX.left]
        add     ax, word[edi + BOX.width]
        mov     ebx, [edi + BOX.top - 2]
        mov     bx, word[edi + BOX.top]
        add     bx, word[edi + BOX.height]
        call    draw_rectangle.forced
        pop     esi ebx eax
        ret
;------------------------------------------------------------------------------
;align 4
;------------------------------------------------------------------------------
;window._.end_moving__box: ;//////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Draw positive box
;------------------------------------------------------------------------------
;> edi = pointer to BOX struct
;------------------------------------------------------------------------------
;        push    eax ebx esi
;        xor     esi, esi
;        jmp     window._.draw_negative_box.1
;------------------------------------------------------------------------------


align 4
; void __fastcall get_window_rect(struct RECT* rc);
; ecx = pointer to RECT
window._.get_rect:
        mov     eax, [current_slot_idx]
        shl     eax, BSF sizeof.WDATA

        mov     edx, [eax + window_data + WDATA.box.left]
        mov     [ecx+RECT.left], edx

        add     edx, [eax + window_data + WDATA.box.width]
        mov     [ecx+RECT.right], edx

        mov     edx, [eax +window_data + WDATA.box.top]
        mov     [ecx+RECT.top], edx

        add     edx, [eax + window_data + WDATA.box.height]
        mov     [ecx+RECT.bottom], edx
        ret
;------------------------------------------------------------------------------


align 4
; redraw all windows one above the window
;> eax = left
;> ebx = top
;> ecx = right
;> edx = bottom
;> esi = process number
;! corrupted edi
window._.redraw_top_wnd:
        push    0
        jmp     window._.set_top_wnd.go
;------------------------------------------------------------------------------


align 4
; call set_screen for all windows one above the window
;> eax = left
;> ebx = top
;> ecx = right
;> edx = bottom
;> esi = process number
;! corrupted edi
window._.set_top_wnd:
        push    1
.go:
        push    esi
        pushfd
        cli

        push    ebp
        mov     ebp, [thread_count]
        cmp     ebp, 1
        jbe     .exit

        shl     esi, BSF sizeof.WDATA
        cmp     [esi + window_data + WDATA.z_modif], ZPOS_ALWAYS_TOP
        je      .exit

        push    eax ;for num layout
        push    edx ecx ebx eax

        movsx   eax, byte [esi + window_data + WDATA.z_modif]
        inc     eax
        mov     dword[esp+10h], eax
;--------------------------------------
align 4
.layout:
        mov     esi, 1        ; = num in window stack
        mov     ebp, [thread_count]
;--------------------------------------
align 4
.next_window:
        movzx   edi, word[WIN_POS + esi * 2]
        shl     edi, BSF sizeof.WDATA

        test    byte [window_data + edi + WDATA.fl_wstate], WSTATE_USED
        jz      .skip_window

        add     edi, window_data
        test    [edi + WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz     .skip_window

        mov     eax, [esp+10h]
        cmp     [edi + WDATA.z_modif], al
        jne     .skip_window

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
;--------------------------------------
align 4
@@:
        cmp     ebx, [esp + RECT.top]
        jae     @f
        mov     ebx, [esp + RECT.top]
;--------------------------------------
align 4
@@:
        cmp     ecx, [esp + RECT.right]
        jbe     @f
        mov     ecx, [esp + RECT.right]
;--------------------------------------
align 4
@@:
        cmp     edx, [esp + RECT.bottom]
        jbe     @f
        mov     edx, [esp + RECT.bottom]
;--------------------------------------
align 4
@@:
        cmp     dword[esp+32], 0
        je      .set_fl_redraw

        push    esi
        movzx   esi, word[WIN_POS + esi * 2]
        call    window._.set_screen
        pop     esi
        jmp     @f
.set_fl_redraw:
        mov     [edi + WDATA.fl_redraw], 1      ;set redraw flag
     @@:
;--------------------------------------
align 4
.skip_window:
        inc     esi
        dec     ebp
        jnz     .next_window
;--------------------------------------
        inc     dword[esp+10h]
        cmp     byte[esp+10h], ZPOS_ALWAYS_TOP
        jle     .layout
;-------------------------------------

        pop     eax ebx ecx edx
        pop     ebp     ;del num layout
;-------------------------------------
align 4
.exit:

        pop     ebp
        popfd
        pop     esi

        add     esp, 4   ;dword for 0/1 - set_screen/fl_redraw
        ret
