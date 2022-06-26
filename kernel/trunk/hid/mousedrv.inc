;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2022. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;   check mouse
;
;
;   FB00  ->   FB0F   mouse memory 00 chunk count - FB0A-B x - FB0C-D y
;   FB10  ->   FB17   mouse color mem
;   FB21              x move
;   FB22              y move
;   FB30              color temp
;   FB28              high bits temp
;   FB4A  ->   FB4D   FB4A-B x-under - FB4C-D y-under
;   FC00  ->   FCFE   com1/ps2 buffer
;   FCFF              com1/ps2 buffer count starting from FC00

uglobal
;--------------------------------------
align 4
mousecount                      dd ?
mousedata                       dd ?
Y_UNDER_sub_CUR_hot_y_add_curh  dw ?
Y_UNDER_subtraction_CUR_hot_y   dw ?
X_UNDER_sub_CUR_hot_x_add_curh  dw ?
X_UNDER_subtraction_CUR_hot_x   dw ?
endg

iglobal
;--------------------------------------
align 4
mouse_speed_factor      dw 4
mouse_delay             db 3
mouse_doubleclick_delay db 64
endg

;-----------------------------------------------------------------------------

align 4
draw_mouse_under:

        ; return old picture
        cmp     [_display.restore_cursor], 0
        je      @F

        pushad
        movzx   eax, word [X_UNDER]
        movzx   ebx, word [Y_UNDER]
        stdcall [_display.restore_cursor], eax, ebx
        popad
        ret

  @@:
        pushad
        xor     ecx, ecx
        xor     edx, edx

  mres:
        movzx   eax, word [X_UNDER]
        movzx   ebx, word [Y_UNDER]
        add     eax, ecx
        add     ebx, edx
        push    ecx
        push    edx
        push    eax
        push    ebx
        mov     eax, edx
        shl     eax, 6
        shl     ecx, 2
        add     eax, ecx
        add     eax, mouseunder
        mov     ecx, [eax]
        pop     ebx
        pop     eax
        mov     edi, 1 ; force
        or      ecx, 0x04000000  ; don't save to mouseunder area
;        call    [putpixel]
        call    __sys_putpixel
        pop     edx
        pop     ecx
        inc     ecx
        cmp     ecx, 16
        jnz     mres
        xor     ecx, ecx
        inc     edx
        cmp     edx, 24
        jnz     mres
        popad
        ret

;-----------------------------------------------------------------------------

align 4
save_draw_mouse:
        cmp     [_display.move_cursor], 0
        je      .no_hw_cursor
        pushad

        mov     [X_UNDER], ax
        mov     [Y_UNDER], bx
        movzx   eax, word [MOUSE_Y]
        movzx   ebx, word [MOUSE_X]
        push    eax
        push    ebx

        mov     eax, [d_width_calc_area + eax*4]

        add     eax, [_display.win_map]
        movzx   edx, byte [ebx + eax]
        shl     edx, BSF sizeof.APPDATA
        ; edx - thread slot of window under cursor
        mov     esi, [SLOT_BASE + edx + APPDATA.cursor] ; cursor of window under cursor

        ; if cursor of window under cursor already equal to the
        ; current_cursor then just draw it
        cmp     esi, [current_cursor]
        je      .draw

        ; eax = thread slot of current active window
        mov     eax, [thread_count]
        movzx   eax, word [WIN_POS + eax*2]
        shl     eax, BSF sizeof.APPDATA

        ; window under cursor == active window ?
        cmp     eax, edx
        je      @f

        ; check whether active window is being resized now:
        mov     bl, [mouse.active_sys_window.action]
        and     bl, mouse.WINDOW_RESIZE_S_FLAG or mouse.WINDOW_RESIZE_W_FLAG or mouse.WINDOW_RESIZE_E_FLAG
        test    bl, bl
        jz      .active_isnt_resizing
        mov     esi, [SLOT_BASE + eax + APPDATA.cursor] ; esi = cursor of active window, it is resizing cursor
        jmp     @f
        .active_isnt_resizing:

        ; if cursor of window under the cursor is resizing cursor then draw it.
        cmp     esi, [def_cursor_hresize]
        je      @f
        cmp     esi, [def_cursor_vresize]
        je      @f
        cmp     esi, [def_cursor_dresize1]
        je      @f
        cmp     esi, [def_cursor_dresize2]
        je      @f

        ; set cursor of window under cursor
        mov     esi, [SLOT_BASE + edx + APPDATA.cursor]
        cmp     esi, [current_cursor]
        je      .draw

@@:
        push    esi
        call    [_display.select_cursor]
        mov     [current_cursor], esi
;--------------------------------------
align 4
.draw:
        stdcall [_display.move_cursor], esi
        popad
        ret
;--------------------------------------
;align 4
;.fail:
;        mov     ecx, [def_cursor]
;        mov     [edx+SLOT_BASE+APPDATA.cursor], ecx
;        stdcall [_display.move_cursor], ecx        ; stdcall: [esp]=ebx,eax
;        popad
;        ret
;--------------------------------------
align 4
.no_hw_cursor:
        pushad
        ; save & draw
        mov     [X_UNDER], ax
        mov     [Y_UNDER], bx
        push    eax
        push    ebx
        mov     ecx, 0
        mov     edx, 0
;--------------------------------------
align 4
drm:
        push    eax
        push    ebx
        push    ecx
        push    edx
        ; helloworld
        push    ecx
        add     eax, ecx        ; save picture under mouse
        add     ebx, edx
        push    ecx
        or      ecx, 0x04000000 ; don't load to mouseunder area
        push    eax ebx edx edi
        call    [GETPIXEL]
        pop     edi edx ebx eax
        mov     [COLOR_TEMP], ecx
        pop     ecx
        mov     eax, edx
        shl     eax, 6
        shl     ecx, 2
        add     eax, ecx
        add     eax, mouseunder
        mov     ebx, [COLOR_TEMP]
        and     ebx, 0xffffff
        mov     [eax], ebx
        pop     ecx
        mov     edi, edx        ; y cycle
        shl     edi, 4          ; *16 bytes per row
        add     edi, ecx        ; x cycle
        mov     esi, edi
        add     edi, esi
        add     edi, esi        ; *3
        add     edi, [MOUSE_PICTURE]    ; we have our str address
        mov     esi, edi
        add     esi, 16*24*3
        push    ecx
        mov     ecx, [COLOR_TEMP]
        call    combine_colors
        and     ecx, 0xffffff
        mov     [MOUSE_COLOR_MEM], ecx
        pop     ecx
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        add     eax, ecx        ; we have x coord+cycle
        add     ebx, edx        ; and y coord+cycle
        push    ecx
        mov     ecx, [MOUSE_COLOR_MEM]
        mov     edi, 1 ; force
        or      ecx, 0x04000000 ; don't save to mouseunder area
;        call    [putpixel]
        call    __sys_putpixel
        pop     ecx
        mov     ebx, [esp+0]    ; pure y coord again
        mov     eax, [esp+4]    ; and x
        inc     ecx             ; +1 cycle
        cmp     ecx, 16         ; if more than 16
        jnz     drm
        xor     ecx, ecx
        inc     edx
        cmp     edx, 24
        jnz     drm
        add     esp, 8
        popad
        ret

;-----------------------------------------------------------------------------

align 4
combine_colors:
      ; in
      ; ecx - color ( 00 RR GG BB )
      ; edi - ref to new color byte
      ; esi - ref to alpha byte
      ;
      ; out
      ; ecx - new color ( roughly (ecx*[esi]>>8)+([edi]*[esi]>>8) )
        push    eax
        push    ebx
        push    edx
        push    ecx
        xor     ecx, ecx
         ; byte 0
        mov     eax, 0xff
        sub     al, [esi+0]
        mov     ebx, [esp]
        shr     ebx, 16
        and     ebx, 0xff
        mul     ebx
        shr     eax, 8
        add     ecx, eax
        xor     eax, eax
        xor     ebx, ebx
        mov     al, [edi+0]
        mov     bl, [esi+0]
        mul     ebx
        shr     eax, 8
        add     ecx, eax
        shl     ecx, 8
         ; byte 1
        mov     eax, 0xff
        sub     al, [esi+1]
        mov     ebx, [esp]
        shr     ebx, 8
        and     ebx, 0xff
        mul     ebx
        shr     eax, 8
        add     ecx, eax
        xor     eax, eax
        xor     ebx, ebx
        mov     al, [edi+1]
        mov     bl, [esi+1]
        mul     ebx
        shr     eax, 8
        add     ecx, eax
        shl     ecx, 8
         ; byte 2
        mov     eax, 0xff
        sub     al, [esi+2]
        mov     ebx, [esp]
        and     ebx, 0xff
        mul     ebx
        shr     eax, 8
        add     ecx, eax
        xor     eax, eax
        xor     ebx, ebx
        mov     al, [edi+2]
        mov     bl, [esi+2]
        mul     ebx
        shr     eax, 8
        add     ecx, eax
        pop     eax
        pop     edx
        pop     ebx
        pop     eax
        ret

;-----------------------------------------------------------------------------

align 4
check_mouse_area_for_getpixel:
; in:
; eax = x
; ebx = y
; out:
; ecx = new color
        push    eax ebx
; check for Y
        xor     ecx, ecx
        mov     cx, [Y_UNDER]   ; [MOUSE_Y]

        cmp     ebx, ecx
        jb      .no_mouse_area
        add     ecx, 23         ; mouse cursor Y size
        cmp     ebx, ecx
        ja      .no_mouse_area
; offset Y
        sub     bx, [Y_UNDER]   ; [MOUSE_Y]
;--------------------------------------
; check for X
        xor     ecx, ecx
        mov     cx, [X_UNDER]   ; [MOUSE_X]
        cmp     eax, ecx
        jb      .no_mouse_area
        add     ecx, 15         ; mouse cursor X size
        cmp     eax, ecx
        ja      .no_mouse_area
; offset X
        sub     ax, [X_UNDER]   ; [MOUSE_X]
;--------------------------------------
; eax = offset x
; ebx = offset y
        shl     ebx, 6  ;y
        shl     eax, 2  ;x
        add     eax, ebx
        add     eax, mouseunder
        mov     ecx, [eax]
        and     ecx, 0xffffff
        or      ecx, 0xff000000
        pop     ebx eax
        ret

  .no_mouse_area:
        xor     ecx, ecx
        pop     ebx eax
        ret

;-----------------------------------------------------------------------------

align 4
check_mouse_area_for_putpixel:
; in:
; ecx = x shl 16 + y
; eax = color
; out:
; eax = new color
        push    eax
; check for Y
        mov     ax, [Y_UNDER]   ; [MOUSE_Y]
        cmp     cx, ax
        jb      .no_mouse_area
        add     ax, 23          ; mouse cursor Y size
        cmp     cx, ax
        ja      .no_mouse_area
; offset Y
        sub     cx, [Y_UNDER]   ; [MOUSE_Y]
        mov     ax, cx
        shl     eax, 16

; check for X
        mov     ax, [X_UNDER]   ; [MOUSE_X]
        shr     ecx, 16
        cmp     cx, ax
        jb      .no_mouse_area
        add     ax, 15          ; mouse cursor X size
        cmp     cx, ax
        ja      .no_mouse_area
; offset X
        sub     cx, [X_UNDER]   ; [MOUSE_X]
        mov     ax, cx

; eax = (offset y) shl 16 + (offset x)

        pop     ecx

        push    eax ebx

        mov     ebx, eax
        shr     ebx, 16         ; y
        and     eax, 0xffff     ; x

        shl     ebx, 6
        shl     eax, 2
        add     eax, ebx
        add     eax, mouseunder
        and     ecx, 0xFFFFFF
        mov     [eax], ecx

        pop     ebx eax

        push    esi edi
        rol     eax, 16
        movzx   edi, ax         ; y cycle
        shl     edi, 4          ; *16 bytes per row
        shr     eax, 16
        add     edi, eax        ; x cycle
        lea     edi, [edi*3]
        add     edi, [MOUSE_PICTURE]    ; we have our str address
        mov     esi, edi
        add     esi, 16*24*3
        call    combine_colors
        pop     edi esi
        mov     eax, ecx
        ret

  .no_mouse_area:
        pop     eax
        ret

;-----------------------------------------------------------------------------

align 4
__sys_draw_pointer:
        pushad
        movzx   ecx, word [X_UNDER]
        movzx   edx, word [Y_UNDER]
        movzx   ebx, word [MOUSE_Y]
        movzx   eax, word [MOUSE_X]
        cmp     [redrawmouse_unconditional], 0
        je      @f
        mov     [redrawmouse_unconditional], 0
        jmp     redrawmouse
  @@:
        cmp     eax, ecx
        jne     redrawmouse
        cmp     ebx, edx
        je      nodmp

;--------------------------------------

align 4
redrawmouse:
        pushfd
        cli
        call    draw_mouse_under
        call    save_draw_mouse

;        mov     eax, [_display.select_cursor]
;        test    eax, eax
;        jz      @f
        cmp     [_display.select_cursor], select_cursor
        jne     @f

        xor     eax, eax
        mov     esi, [current_cursor]

        mov     ax, [Y_UNDER]
        sub     eax, [esi + CURSOR.hot_y]
        mov     [Y_UNDER_subtraction_CUR_hot_y], ax
        add     eax, [cur.h]
        mov     [Y_UNDER_sub_CUR_hot_y_add_curh], ax

        mov     ax, [X_UNDER]
        sub     eax, [esi + CURSOR.hot_x]
        mov     [X_UNDER_subtraction_CUR_hot_x], ax
        add     eax, [cur.w]
        mov     [X_UNDER_sub_CUR_hot_x_add_curh], ax
  @@:
        popfd
  nodmp:
        popad
        ret

;-----------------------------------------------------------------------------

align 4
proc set_mouse_data stdcall uses ecx edx, BtnState:dword, XMoving:dword, YMoving:dword, VScroll:dword, HScroll:dword

        mov     eax, [BtnState]
        and     eax, 0x3FFFFFFF         ; Top 2 bits are used to flag absolute movements
        mov     [BTN_DOWN], eax
;--------------------------------------
        mov     eax, [XMoving]
        test    [BtnState], 0x80000000
        jnz     .absolute_x
        test    eax, eax
        jz      @f
        call    mouse_acceleration
        add     ax, [MOUSE_X]
        jns     .check_x
        xor     eax, eax
        jmp     .set_x
 .absolute_x:
        mov     edx, [_display.width]
        mul     edx
        shr     eax, 15
 .check_x:
        cmp     ax, word[_display.width]
        jl      .set_x
        mov     ax, word[_display.width]
        dec     ax
 .set_x:
        mov     [MOUSE_X], ax
;--------------------------------------
@@:
        mov     eax, [YMoving]
        test    [BtnState], 0x40000000
        jnz     .absolute_y
        test    eax, eax
        jz      @f
        neg     eax
        call    mouse_acceleration
        add     ax, [MOUSE_Y]
        jns     .check_y
        xor     eax, eax
        jmp     .set_y
 .absolute_y:
        mov     edx, [_display.height]
        mul     edx
        shr     eax, 15
 .check_y:
        cmp     ax, word[_display.height]
        jl      .set_y
        mov     ax, word[_display.height]
        dec     ax
 .set_y:
        mov     [MOUSE_Y], ax
;--------------------------------------
@@:
        mov     eax, [VScroll]
        test    eax, eax
        jz      @f
        add     [MOUSE_SCROLL_V], ax
        bts     word [BTN_DOWN], 15
@@:
        mov     eax, [HScroll]
        test    eax, eax
        jz      @f
        add     [MOUSE_SCROLL_H], ax
        bts     dword [BTN_DOWN], 23
@@:
        mov     [mouse_active], 1
        call    wakeup_osloop
        ret
endp

;-----------------------------------------------------------------------------
mouse_acceleration:
        neg     ax
        jl      mouse_acceleration
        add     al, [mouse_delay]
        mul     al
        mov     cx, [mouse_speed_factor]
        dec     ax
        shr     ax, cl
        inc     ax
        test    eax, eax
        jns     @f
        neg     ax
@@:
        ret
