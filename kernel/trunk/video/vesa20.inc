;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2023. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  VESA20.INC                                                  ;;
;;                                                              ;;
;;  Vesa 2.0 functions for MenuetOS                             ;;
;;                                                              ;;
;;  Copyright 2002 Ville Turjanmaa                              ;;
;;  Alexey, kgaz@crosswindws.net                                ;;
;;  - Voodoo compatible graphics                                ;;
;;  Juan M. Caravaca                                            ;;
;;  - Graphics optimimizations eg. drawline                     ;;
;;                                                              ;;
;;  See file COPYING for details                                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

uglobal
align 4
        bgr_cur_line    rd MAX_SCREEN_WIDTH
        bgr_next_line   rd MAX_SCREEN_WIDTH
endg

iglobal
align 4
        overlapping_of_points_ptr       dd overlapping_of_points
endg


;-----------------------------------------------------------------------------
; eax = x
; ebx = y

align 4
Vesa20_getpixel16:

        ; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     .no_mouseunder

        ; check mouse area for putpixel
        test    ecx, 0x04000000         ; don't load to mouseunder area
        jnz     .no_mouseunder
        call    [_display.check_m_pixel]
        test    ecx, ecx                ; 0xff000000
        jnz     @f

  .no_mouseunder:
;        imul    ebx, [BytesPerScanLine] ; ebx = y * y multiplier
        mov     ebx, [BPSLine_calc_area + ebx*4]
        lea     edi, [eax*2]            ; edi = x*2
        add     edi, ebx                ; edi = x*2+(y*y multiplier)

        movzx   ecx, word[LFB_BASE + edi]
        shl     ecx, 3
        ror     ecx, 8
        shl     cx, 2
        ror     ecx, 8
        shl     cl, 3
        rol     ecx, 16
  @@:
        and     ecx, 0x00ffffff
        ret

;-----------------------------------------------------------------------------
; eax = x
; ebx = y

align 4
Vesa20_getpixel24:

        ; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     .no_mouseunder

        ; check mouse area for putpixel
        test    ecx, 0x04000000         ; don't load to mouseunder area
        jnz     .no_mouseunder
        call    [_display.check_m_pixel]
        test    ecx, ecx                ; 0xff000000
        jnz     @f

  .no_mouseunder:
;        imul    ebx, [BytesPerScanLine] ; ebx = y * y multiplier
        mov     ebx, [BPSLine_calc_area + ebx*4]
        lea     edi, [eax + eax*2]        ; edi = x*3
        add     edi, ebx                ; edi = x*3+(y*y multiplier)

        mov     ecx, [LFB_BASE + edi]
  @@:
        and     ecx, 0x00ffffff
        ret

;-----------------------------------------------------------------------------
; eax = x
; ebx = y

align 4
Vesa20_getpixel32:

        ; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     .no_mouseunder

        ; check mouse area for putpixel
        test    ecx, 0x04000000         ; don't load to mouseunder area
        jnz     .no_mouseunder
        call    [_display.check_m_pixel]
        test    ecx, ecx                ; 0xff000000
        jnz     @f

  .no_mouseunder:
;        imul    ebx, [BytesPerScanLine] ; ebx = y * y multiplier
        mov     ebx, [BPSLine_calc_area + ebx*4]
        lea     edi, [ebx + eax*4]        ; edi = x*4+(y*y multiplier)

        mov     ecx, [LFB_BASE + edi]
  @@:
        and     ecx, 0x00ffffff
        ret

;-----------------------------------------------------------------------------
; ebx = pointer
; ecx = size [x|y]
; edx = coordinates [x|y]
; ebp = pointer to 'get' function
; esi = pointer to 'init' function
; edi = parameter for 'get' function

align 16
vesa20_putimage:

virtual at esp
 putimg:
   .real_sx        dd ?
   .real_sy        dd ?
   .image_sx       dd ?
   .image_sy       dd ?
   .image_cx       dd ?
   .image_cy       dd ?
   .pti            dd ?
   .abs_cx         dd ?
   .abs_cy         dd ?
   .line_increment dd ?
   .winmap_newline dd ?
   .screen_newline dd ?
   .real_sx_and_abs_cx dd ?
   .real_sy_and_abs_cy dd ?
   .stack_data = 4*14
   .edi         dd      ?
   .esi         dd      ?
   .ebp         dd      ?
   .esp         dd      ?
   .ebx         dd      ?
   .edx         dd      ?
   .ecx         dd      ?
   .eax         dd      ?
   .ret_addr    dd      ?
   .arg_0       dd      ?
end virtual

        pushad
        sub     esp, putimg.stack_data
; save pointer to image
        mov     [putimg.pti], ebx
; unpack the size
        mov     eax, ecx
        and     ecx, 0xFFFF
        shr     eax, 16
        mov     [putimg.image_sx], eax
        mov     [putimg.image_sy], ecx
; unpack the coordinates
        mov     eax, edx
        and     edx, 0xFFFF
        shr     eax, 16
        mov     [putimg.image_cx], eax
        mov     [putimg.image_cy], edx
; calculate absolute (i.e. screen) coordinates
        mov     eax, [current_slot]
        mov     eax, [eax + APPDATA.window]
        mov     ebx, [eax + WDATA.box.left]
        add     ebx, [putimg.image_cx]
        mov     [putimg.abs_cx], ebx
        mov     ebx, [eax + WDATA.box.top]
        add     ebx, [putimg.image_cy]
        mov     [putimg.abs_cy], ebx
; real_sx = MIN(wnd_sx-image_cx, image_sx);
        mov     ebx, [eax + WDATA.box.width]       ; ebx = wnd_sx
        inc     ebx                                     ; WDATA.box.width is one pixel less than real window x-size
        sub     ebx, [putimg.image_cx]
        ja      @f
        add     esp, putimg.stack_data
        popad
        ret

  @@:
        cmp     ebx, [putimg.image_sx]
        jbe     .end_x
        mov     ebx, [putimg.image_sx]
  .end_x:
        mov     [putimg.real_sx], ebx
; init real_sy
        mov     ebx, [eax + WDATA.box.height]      ; ebx = wnd_sy
        inc     ebx
        sub     ebx, [putimg.image_cy]
        ja      @f
        add     esp, putimg.stack_data
        popad
        ret

  @@:
        cmp     ebx, [putimg.image_sy]
        jbe     .end_y
        mov     ebx, [putimg.image_sy]
  .end_y:
        mov     [putimg.real_sy], ebx
; line increment
        mov     eax, [putimg.image_sx]
        mov     ecx, [putimg.real_sx]
        sub     eax, ecx
;;     imul    eax, [putimg.source_bpp]
;     lea     eax, [eax + eax * 2]
        call    esi
        add     eax, [putimg.arg_0]
        mov     [putimg.line_increment], eax
; winmap new line increment
        mov     eax, [_display.width]
        sub     eax, [putimg.real_sx]
        mov     [putimg.winmap_newline], eax
; screen new line increment
        mov     eax, [_display.lfb_pitch]
        mov     ebx, [_display.bytes_per_pixel]
        imul    ecx, ebx
        sub     eax, ecx
        mov     [putimg.screen_newline], eax
; pointer to image
        mov     esi, [putimg.pti]
; pointer to screen
        mov     edx, [putimg.abs_cy]
;        imul    edx, [BytesPerScanLine]
        mov     edx, [BPSLine_calc_area + edx*4]
        mov     eax, [putimg.abs_cx]
        imul    eax, ebx
        add     edx, eax
; pointer to pixel map
        mov     eax, [putimg.abs_cy]
;        imul    eax, [Screen_Max_X]
;        add     eax, [putimg.abs_cy]
        mov     eax, [d_width_calc_area + eax*4]

        add     eax, [putimg.abs_cx]
        add     eax, [_display.win_map]
        xchg    eax, ebp

        mov     ecx, [putimg.real_sx]
        add     ecx, [putimg.abs_cx]
        mov     [putimg.real_sx_and_abs_cx], ecx
        mov     ecx, [putimg.real_sy]
        add     ecx, [putimg.abs_cy]
        mov     [putimg.real_sy_and_abs_cy], ecx

; get process number
        mov     ebx, [current_slot_idx]

        cmp     byte [_display.bits_per_pixel], 16
        je      put_image_end_16
        cmp     byte [_display.bits_per_pixel], 24
        je      put_image_end_24
        cmp     byte [_display.bits_per_pixel], 32
        je      put_image_end_32

;------------------------------------------------------------------------------

put_image_end_16:

        mov     edi, [putimg.real_sy]

; check for hardware cursor
        mov     ecx, [_display.select_cursor]
        cmp     ecx, select_cursor
        je      put_image_end_16_new

  .new_line:
        mov     ecx, [putimg.real_sx]
  .new_x:
        push    [putimg.edi]
        mov     eax, [putimg.ebp + 4]
        call    eax
        cmp     [ebp], bl
        jne     .skip
; convert to 16 bpp and store to LFB
        and     eax, 00000000111110001111110011111000b
        shr     ah, 2
        shr     ax, 3
        ror     eax, 8
        add     al, ah
        rol     eax, 8
        mov     [LFB_BASE + edx], ax
  .skip:
        add     edx, 2
        inc     ebp
        dec     ecx
        jnz     .new_x

        add     esi, [putimg.line_increment]
        add     edx, [putimg.screen_newline]
        add     ebp, [putimg.winmap_newline]

        cmp     [putimg.ebp], putimage_get1bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get2bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get4bpp
        jnz     @f
  .correct:
        mov     eax, [putimg.edi]
        mov     byte [eax], 80h
  @@:
        dec     edi
        jnz     .new_line
  .finish:
        add     esp, putimg.stack_data
        popad
        ret

;------------------------------------------------------------------------------

align 4
put_image_end_16_new:

  .new_line:
        mov     ecx, [putimg.real_sx]

  .new_x:
        push    [putimg.edi]
        mov     eax, [putimg.ebp + 4]
        call    eax

        cmp     [ebp], bl
        jne     .skip

        push    ecx
  .sh:
        neg     ecx
        add     ecx, [putimg.real_sx_and_abs_cx + 4]

; check for X
        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     .no_mouse_area

        sub     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      .no_mouse_area

        shl     ecx, 16
        add     ecx, [putimg.real_sy_and_abs_cy + 4]
        sub     ecx, edi

; check for Y
        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     .no_mouse_area

        sub     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      .no_mouse_area

; check mouse area for putpixel
        call    check_mouse_area_for_putpixel_new.1
        cmp     ecx, -1                 ; SHIT HAPPENS?
        jne     .no_mouse_area

        mov     ecx, [esp]
        jmp     .sh

  .no_mouse_area:
        pop     ecx
; convert to 16 bpp and store to LFB
        and     eax, 00000000111110001111110011111000b
        shr     ah, 2
        shr     ax, 3
        ror     eax, 8
        add     al, ah
        rol     eax, 8
        mov     [LFB_BASE+edx], ax

  .skip:
        add     edx, 2
        inc     ebp
        dec     ecx
        jnz     .new_x

        add     esi, [putimg.line_increment]
        add     edx, [putimg.screen_newline]
        add     ebp, [putimg.winmap_newline]

        cmp     [putimg.ebp], putimage_get1bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get2bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get4bpp
        jnz     @f

  .correct:
        mov     eax, [putimg.edi]
        mov     byte [eax], 80h

  @@:
        dec     edi
        jnz     .new_line
        jmp     put_image_end_16.finish

;------------------------------------------------------------------------------

align 4
put_image_end_24:

        mov     edi, [putimg.real_sy]

; check for hardware cursor
        mov     ecx, [_display.select_cursor]
        cmp     ecx, select_cursor
        je      put_image_end_24_new

  .new_line:
        mov     ecx, [putimg.real_sx]
  .new_x:
        push    [putimg.edi]
        mov     eax, [putimg.ebp + 4]
        call    eax
        cmp     [ebp], bl
        jne     .skip

; store to LFB
        mov     [LFB_BASE + edx], ax
        shr     eax, 16
        mov     [LFB_BASE + edx + 2], al

  .skip:
        add     edx, 3
        inc     ebp
        dec     ecx
        jnz     .new_x

        add     esi, [putimg.line_increment]
        add     edx, [putimg.screen_newline]
        add     ebp, [putimg.winmap_newline]

        cmp     [putimg.ebp], putimage_get1bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get2bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get4bpp
        jnz     @f
  .correct:
        mov     eax, [putimg.edi]
        mov     byte [eax], 80h
  @@:
        dec     edi
        jnz     .new_line
  .finish:
        add     esp, putimg.stack_data
        popad
        ret

;------------------------------------------------------------------------------

align 4
put_image_end_24_new:

  .new_line:
        mov     ecx, [putimg.real_sx]

  .new_x:
        push    [putimg.edi]
        mov     eax, [putimg.ebp + 4]
        call    eax
        cmp     [ebp], bl
        jne     .skip

        push    ecx
  .sh:
        neg     ecx
        add     ecx, [putimg.real_sx_and_abs_cx + 4]

; check for X
        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     .no_mouse_area

        sub     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      .no_mouse_area

        shl     ecx, 16

        add     ecx, [putimg.real_sy_and_abs_cy + 4]
        sub     ecx, edi

; check for Y
        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     .no_mouse_area

        sub     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      .no_mouse_area

; check mouse area for putpixel
        call    check_mouse_area_for_putpixel_new.1
        cmp     ecx, -1         ; SHIT HAPPENS?
        jne     .no_mouse_area

        mov     ecx, [esp]
        jmp     .sh

  .no_mouse_area:
        pop     ecx

; store to LFB
        mov     [LFB_BASE + edx], ax
        shr     eax, 16
        mov     [LFB_BASE + edx + 2], al

  .skip:
        add     edx, 3
        inc     ebp
        dec     ecx
        jnz     .new_x

        add     esi, [putimg.line_increment]
        add     edx, [putimg.screen_newline]
        add     ebp, [putimg.winmap_newline]

        cmp     [putimg.ebp], putimage_get1bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get2bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get4bpp
        jnz     @f

  .correct:
        mov     eax, [putimg.edi]
        mov     byte [eax], 80h

  @@:
        dec     edi
        jnz     .new_line
        jmp     put_image_end_24.finish

;------------------------------------------------------------------------------

align 4
put_image_end_32:

        mov     edi, [putimg.real_sy]

; check for hardware cursor
        mov     ecx, [_display.select_cursor]
        cmp     ecx, select_cursor
        je      put_image_end_32_new

  .new_line:
        mov     ecx, [putimg.real_sx]

  .new_x:
        push    [putimg.edi]
        mov     eax, [putimg.ebp + 4]
        call    eax
        cmp     [ebp], bl
        jne     .skip

; store to LFB
        mov     [LFB_BASE + edx], eax

  .skip:
        add     edx, 4
        inc     ebp
        dec     ecx
        jnz     .new_x

        add     esi, [putimg.line_increment]
        add     edx, [putimg.screen_newline]
        add     ebp, [putimg.winmap_newline]

        cmp     [putimg.ebp], putimage_get1bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get2bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get4bpp
        jnz     @f

  .correct:
        mov     eax, [putimg.edi]
        mov     byte [eax], 80h

  @@:
        dec     edi
        jnz     .new_line

  .finish:
        add     esp, putimg.stack_data
        popad
        cmp     [SCR_MODE], 0x12
        jne     @f
        call    VGA__putimage
  @@:
        mov     [EGA_counter], 1
        ret

;------------------------------------------------------------------------------

align 4
put_image_end_32_new:

  .new_line:
        mov     ecx, [putimg.real_sx]

  .new_x:
        push    [putimg.edi]
        mov     eax, [putimg.ebp + 4]
        call    eax
        cmp     [ebp], bl
        jne     .skip

        push    ecx

  .sh:
        neg     ecx
        add     ecx, [putimg.real_sx_and_abs_cx + 4]

; check for X
        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     .no_mouse_area
        sub     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      .no_mouse_area
        shl     ecx, 16

        add     ecx, [putimg.real_sy_and_abs_cy + 4]
        sub     ecx, edi

; check for Y
        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     .no_mouse_area
        sub     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      .no_mouse_area

; check mouse area for putpixel
        call    check_mouse_area_for_putpixel_new.1
        cmp     ecx, -1         ; SHIT HAPPENS?
        jne     .no_mouse_area

        mov     ecx, [esp]
        jmp     .sh

  .no_mouse_area:
        pop     ecx

; store to LFB
        mov     [LFB_BASE+edx], eax

  .skip:
        add     edx, 4
        inc     ebp
        dec     ecx
        jnz     .new_x

        add     esi, [putimg.line_increment]
        add     edx, [putimg.screen_newline]
        add     ebp, [putimg.winmap_newline]

        cmp     [putimg.ebp], putimage_get1bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get2bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get4bpp
        jnz     @f

  .correct:
        mov     eax, [putimg.edi]
        mov     byte [eax], 80h

  @@:
        dec     edi
        jnz     .new_line
        jmp     put_image_end_32.finish

;------------------------------------------------------------------------------
; eax = x coordinate
; ebx = y coordinate
; ecx = xx RR GG BB
; xx flags:
; 0x01000000 color inversion
; 0x02000000 used for draw_rectangle without top line (for drawwindow_III and drawwindow_IV)
; edi = 0x00000001 force

align 4
__sys_putpixel:

        pushad
        cmp     eax, [_display.width]
        jae     .exit
        cmp     ebx, [_display.height]
        jae     .exit
        test    edi, 1          ; force ?
        jnz     .forced

; not forced
        mov     edx, [d_width_calc_area + ebx*4]
        add     edx, [_display.win_map]
        movzx   edx, byte [eax+edx]
        cmp     edx, [current_slot_idx]
        jne     .exit

  .forced:
; check for color inversion
        test    ecx, 0x01000000
        jz      .no_inv

        push    eax ebx edx edi
        call    [GETPIXEL]
        pop     edi edx ebx eax

        not     ecx
        rol     ecx, 8
        mov     cl, [esp + 32-8+3]
        ror     ecx, 8
        mov     [esp + 32-8], ecx
  .no_inv:
        call    [PUTPIXEL]      ; call the real put_pixel function
  .exit:
        popad
        ret

;-----------------------------------------------------------------------------
; eax = x
; ebx = y

align 4
Vesa20_putpixel16_new:

        mov     ecx, eax
        shl     ecx, 16
        mov     cx, bx

;        imul    ebx, [BytesPerScanLine]  ; ebx = y * y multiplier
        mov     ebx, [BPSLine_calc_area + ebx*4]
        lea     edi, [eax*2]; edi = x*2
        mov     eax, [esp + 32-8+4]

; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     @f
; check mouse area for putpixel
        test    eax, 0x04000000
        jnz     @f

; check for Y
        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     @f
        sub     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      @f
        rol     ecx, 16

; check for X
        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     @f
        sub     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      @f
        ror     ecx, 16

        call    check_mouse_area_for_putpixel_new.1
  @@:
; store to LFB
        and     eax, 00000000111110001111110011111000b
        shr     ah, 2
        shr     ax, 3
        ror     eax, 8
        add     al, ah
        rol     eax, 8

        mov     [LFB_BASE + ebx + edi], ax
        ret

;-----------------------------------------------------------------------------
; eax = x
; ebx = y

align 4
Vesa20_putpixel24_new:

        mov     ecx, eax
        shl     ecx, 16
        mov     cx, bx

;        imul    ebx, [BytesPerScanLine]  ; ebx = y * y multiplier
        mov     ebx, [BPSLine_calc_area+ebx*4]
        lea     edi, [eax + eax*2]; edi = x*3
        mov     eax, [esp + 32-8+4]

; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     @f
; check mouse area for putpixel
        test    eax, 0x04000000
        jnz     @f

; check for Y
        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     @f
        sub     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      @f
        rol     ecx, 16

; check for X
        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     @f
        sub     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      @f
        ror     ecx, 16

        call    check_mouse_area_for_putpixel_new.1
  @@:
; store to LFB
        mov     [LFB_BASE + ebx + edi], ax
        shr     eax, 16
        mov     [LFB_BASE + ebx + edi + 2], al
        ret

;-----------------------------------------------------------------------------
; eax = x
; ebx = y

align 4
Vesa20_putpixel32_new:

        mov     ecx, eax
        shl     ecx, 16
        mov     cx, bx

;        imul    ebx, [BytesPerScanLine]  ; ebx = y * y multiplier
        mov     ebx, [BPSLine_calc_area+ebx*4]
        lea     edi, [ebx + eax*4]        ; edi = x*4+(y*y multiplier)
        mov     eax, [esp + 32-8+4]       ; eax = color

; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     @f
; check mouse area for putpixel
        test    eax, 0x04000000
        jnz     @f

; check for Y
        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     @f
        sub     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      @f
        rol     ecx, 16

; check for X
        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     @f
        sub     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      @f
        ror     ecx, 16

        call    check_mouse_area_for_putpixel_new.1
  @@:
        and     eax, 0x00ffffff
; store to LFB
        mov     [LFB_BASE + edi], eax
        ret

;-----------------------------------------------------------------------------

align 4
calculate_edi:
;        mov     edi, ebx
;        imul    edi, [Screen_Max_X]
;        add     edi, ebx
        mov     edi, [d_width_calc_area + ebx*4]
        add     edi, eax
        ret


;-----------------------------------------------------------------------------
; DRAWLINE
;-----------------------------------------------------------------------------
; eax = x1 shl 16 + x2
; ebx = y1 shl 16 + y2
; ecx = color
; edi = force ?

align 4
__sys_draw_line:

dl_x1 equ esp+20
dl_y1 equ esp+16
dl_x2 equ esp+12
dl_y2 equ esp+8
dl_dx equ esp+4
dl_dy equ esp+0

        pusha

        xor     edx, edx        ; clear edx
        xor     esi, esi        ; unpack arguments
        xor     ebp, ebp
        mov     si, ax          ; esi = x2
        mov     bp, bx          ; ebp = y2
        shr     eax, 16         ; eax = x1
        shr     ebx, 16         ; ebx = y1
        push    eax             ; save x1
        push    ebx             ; save y1
        push    esi             ; save x2
        push    ebp             ; save y2
; checking x-axis...
        sub     esi, eax        ; esi = x2-x1
        push    esi             ; save y2-y1
        jl      .x2lx1          ; is x2 less than x1 ?
        jg      .no_vline       ; x1 > x2 ?
        mov     edx, ebp        ; else (if x1=x2)
        call    vline
        push    edx             ; necessary to rightly restore stack frame at .exit
        jmp     .exit

  .x2lx1:
        neg     esi             ; get esi absolute value

  .no_vline:
; checking y-axis...
        sub     ebp, ebx        ; ebp = y2-y1
        push    ebp             ; save y2-y1
        jl      .y2ly1          ; is y2 less than y1 ?
        jg      .no_hline       ; y1 > y2 ?
        mov     edx, [dl_x2]    ; else (if y1=y2)
        call    hline
        jmp     .exit

  .y2ly1:
        neg     ebp             ; get ebp absolute value

  .no_hline:
        cmp     ebp, esi
        jle     .x_rules        ; |y2-y1| < |x2-x1|  ?
        cmp     [dl_y2], ebx    ; make sure y1 is at the begining
        jge     .no_reverse1
        neg     dword [dl_dx]
        mov     edx, [dl_x2]
        mov     [dl_x2], eax
        mov     [dl_x1], edx
        mov     edx, [dl_y2]
        mov     [dl_y2], ebx
        mov     [dl_y1], edx

  .no_reverse1:
        mov     eax, [dl_dx]
        cdq                     ; extend eax sing to edx
        shl     eax, 16         ; using 16bit fix-point maths
        idiv    ebp             ; eax = ((x2-x1)*65536)/(y2-y1)

; correction for the remainder of the division
        shl     edx, 1
        cmp     ebp, edx
        jb      @f
        inc     eax
  @@:
        mov     edx, ebp        ; edx = counter (number of pixels to draw)
        mov     ebp, 1 shl 16   ; ebp = dy = 1.0
        mov     esi, eax        ; esi = dx
        jmp     .y_rules
  .x_rules:
        cmp     [dl_x2], eax    ; make sure x1 is at the begining
        jge     .no_reverse2
        neg     dword [dl_dy]
        mov     edx, [dl_x2]
        mov     [dl_x2], eax
        mov     [dl_x1], edx
        mov     edx, [dl_y2]
        mov     [dl_y2], ebx
        mov     [dl_y1], edx
  .no_reverse2:
        xor     edx, edx
        mov     eax, [dl_dy]
        cdq                     ; extend eax sing to edx
        shl     eax, 16         ; using 16bit fix-point maths
        idiv    esi             ; eax = ((y2-y1)*65536)/(x2-x1)
; correction for the remainder of the division
        shl     edx, 1
        cmp     esi, edx
        jb      @f
        inc     eax
  @@:
        mov     edx, esi        ; edx = counter (number of pixels to draw)
        mov     esi, 1 shl 16   ; esi = dx = 1.0
        mov     ebp, eax        ; ebp = dy

  .y_rules:
        mov     eax, [dl_x1]
        mov     ebx, [dl_y1]
        shl     eax, 16
        shl     ebx, 16

        and     ecx, 0xFBFFFFFF ; negate 0x04000000 save to mouseunder area
  .draw:
        push    eax ebx

; correction for the remainder of the division
        test    ah, 0x80
        jz      @f
        add     eax, 1 shl 16
  @@:
        shr     eax, 16
; correction for the remainder of the division
        test    bh, 0x80
        jz      @f
        add     ebx, 1 shl 16
  @@:
        shr     ebx, 16
;        and     ecx, 0xFBFFFFFF  ; negate 0x04000000 save to mouseunder area
;        call    [putpixel]
        call    __sys_putpixel
        pop     ebx eax
        add     ebx, ebp     ; y = y+dy
        add     eax, esi     ; x = x+dx
        dec     edx
        jnz     .draw
; force last drawn pixel to be at (x2,y2)
        mov     eax, [dl_x2]
        mov     ebx, [dl_y2]
;        and     ecx, 0xFBFFFFFF ;n egate 0x04000000 save to mouseunder area
;        call    [putpixel]
        call    __sys_putpixel

  .exit:
        add     esp, 6*4
        popa
;        call    [draw_pointer]
        ret

;------------------------------------------------------------------------------
; draw an horizontal line
; eax = x1
; edx = x2
; ebx = y
; ecx = color
; edi = force ?

align 4
hline:

        push    eax edx
        cmp     edx, eax   ; make sure x2 is above x1
        jge     @f
        xchg    eax, edx
  @@:
        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 save to mouseunder area
  @@:
;        call    [putpixel]
        call    __sys_putpixel
        inc     eax
        cmp     eax, edx
        jle     @b
        pop     edx eax
        ret

;------------------------------------------------------------------------------
; draw a vertical line
; eax = x
; ebx = y1
; edx = y2
; ecx = color
; edi = force ?

align 4
vline:

        push    ebx edx
        cmp     edx, ebx   ; make sure y2 is above y1
        jge     @f
        xchg    ebx, edx
  @@:
        and     ecx, 0xFBFFFFFF ;negate 0x04000000 save to mouseunder area
  @@:
;        call    [putpixel]
        call    __sys_putpixel
        inc     ebx
        cmp     ebx, edx
        jle     @b
        pop     edx ebx
        ret

;------------------------------------------------------------------------------
; eax   cx
; ebx   cy
; ecx   xe
; edx   ye
; edi   color

align 4
vesa20_drawbar:

virtual at esp
drbar:
     .bar_sx       dd ?
     .bar_sy       dd ?
     .bar_cx       dd ?
     .bar_cy       dd ?
     .abs_cx       dd ?
     .abs_cy       dd ?
     .real_sx      dd ?
     .real_sy      dd ?
     .color        dd ?
     .line_inc_scr dd ?
     .line_inc_map dd ?
     .real_sx_and_abs_cx dd ?
     .real_sy_and_abs_cy dd ?
     .stack_data = 4*13
end virtual

        pushad
        sub     esp, drbar.stack_data
        mov     [drbar.color], edi
        sub     edx, ebx
        jle     .exit
        sub     ecx, eax
        jle     .exit
        mov     [drbar.bar_sy], edx
        mov     [drbar.bar_sx], ecx
        mov     [drbar.bar_cx], eax
        mov     [drbar.bar_cy], ebx
        mov     edi, [current_slot]
        mov     edi, [edi + APPDATA.window]
        add     eax, [edi + WDATA.box.left]        ; win_cx
        add     ebx, [edi + WDATA.box.top]         ; win_cy
        mov     [drbar.abs_cx], eax
        mov     [drbar.abs_cy], ebx
; real_sx = MIN(wnd_sx-bar_cx, bar_sx);
        mov     ebx, [edi + WDATA.box.width]       ; ebx = wnd_sx
        inc     ebx                                     ; WDATA.box.width is one pixel less than real window x-size
        sub     ebx, [drbar.bar_cx]
        ja      @f
  .exit:
        add     esp, drbar.stack_data
        popad
        xor     eax, eax
        inc     eax
        ret
  @@:
        cmp     ebx, [drbar.bar_sx]
        jbe     .end_x
        mov     ebx, [drbar.bar_sx]
  .end_x:
        mov     [drbar.real_sx], ebx
; real_sy = MIN(wnd_sy-bar_cy, bar_sy);
        mov     ebx, [edi + WDATA.box.height]      ; ebx = wnd_sy
        inc     ebx
        sub     ebx, [drbar.bar_cy]
        ja      @f
        add     esp, drbar.stack_data
        popad
        xor     eax, eax
        inc     eax
        ret
  @@:
        cmp     ebx, [drbar.bar_sy]
        jbe     .end_y
        mov     ebx, [drbar.bar_sy]
  .end_y:
        mov     [drbar.real_sy], ebx
; line_inc_map
        mov     eax, [_display.width]
        sub     eax, [drbar.real_sx]
        mov     [drbar.line_inc_map], eax
; line_inc_scr
        mov     eax, [drbar.real_sx]
        mov     ebx, [_display.bytes_per_pixel]
        imul    eax, ebx
        neg     eax
        add     eax, [_display.lfb_pitch]
        mov     [drbar.line_inc_scr], eax
; pointer to screen
        mov     edx, [drbar.abs_cy]
;        imul    edx, [BytesPerScanLine]
        mov     edx, [BPSLine_calc_area+edx*4]
        mov     eax, [drbar.abs_cx]
        imul    eax, ebx
        add     edx, eax
; pointer to pixel map
        mov     eax, [drbar.abs_cy]
;        imul    eax, [Screen_Max_X]
;        add     eax, [drbar.abs_cy]
        mov     eax, [d_width_calc_area + eax*4]

        add     eax, [drbar.abs_cx]
        add     eax, [_display.win_map]
        xchg    eax, ebp

        mov     ebx, [drbar.real_sx]
        add     ebx, [drbar.abs_cx]
        mov     [drbar.real_sx_and_abs_cx], ebx
        mov     ebx, [drbar.real_sy]
        add     ebx, [drbar.abs_cy]
        mov     [drbar.real_sy_and_abs_cy], ebx

        add     edx, LFB_BASE

; get process number
        mov     ebx, [current_slot_idx]  ; bl - process num
        mov     esi, [drbar.real_sy]
        mov     eax, [drbar.color] ; BBGGRR00
        rol     eax, 8
        mov     bh, al  ; 0x80 drawing gradient bars
        ror     eax, 8

        cmp     byte [_display.bits_per_pixel], 16
        je      draw_bar_end_16
        cmp     byte [_display.bits_per_pixel], 24
        je      draw_bar_end_24
        cmp     byte [_display.bits_per_pixel], 32
        je      draw_bar_end_32

;--------------------------------------
; eax - color high   RRGGBB
; bl - process num
; ecx - temp
; edx - pointer to screen
; esi - counter
; edi - counter

align 4
draw_bar_end_24:

; check for hardware cursor
        mov     ecx, [_display.select_cursor]
        cmp     ecx, select_cursor
        je      draw_bar_end_24_new

  .new_y:
        mov     edi, [drbar.real_sx]
  .new_x:
        cmp     byte [ebp], bl
        jne     .skip

; store to LFB
        mov     [edx], ax
        shr     eax, 16
        mov     [edx + 2], al
  .skip:
; add pixel
        add     edx, 3
        inc     ebp
        dec     edi
        jnz     .new_x
; add line
        add     edx, [drbar.line_inc_scr]
        add     ebp, [drbar.line_inc_map]
; drawing gradient bars
        test    bh, 0x80
        jz      @f
        test    al, al
        jz      @f
        dec     al
  @@:
        dec     esi
        jnz     .new_y
  .end:
        add     esp, drbar.stack_data
        popad
        xor     eax, eax
        ret

;------------------------------------------------------------------------------

align 4
draw_bar_end_24_new:

  .new_y:
        mov     edi, [drbar.real_sx]
  .new_x:
        cmp     byte [ebp], bl
        jne     .skip

        mov     ecx, [drbar.real_sy_and_abs_cy]
        sub     ecx, esi

; check for Y
        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     .no_mouse_area
        sub     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      .no_mouse_area
        rol     ecx, 16
        add     ecx, [drbar.real_sx_and_abs_cx]
        sub     ecx, edi

; check for X
        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     .no_mouse_area
        sub     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      .no_mouse_area
        ror     ecx, 16

; check mouse area for putpixel
        push    eax
        call    check_mouse_area_for_putpixel_new.1
        mov     [edx], ax
        shr     eax, 16
        mov     [edx + 2], al
        pop     eax
        jmp     .skip

  .no_mouse_area:
; store to LFB
        mov     [edx], ax
        ror     eax, 16
        mov     [edx + 2], al
        rol     eax, 16
  .skip:

; add pixel
        add     edx, 3
        inc     ebp
        dec     edi
        jnz     .new_x

; add line
        add     edx, [drbar.line_inc_scr]
        add     ebp, [drbar.line_inc_map]

; drawing gradient bars
        test    bh, 0x80
        jz      @f
        test    al, al
        jz      @f
        dec     al
  @@:
        dec     esi
        jnz     .new_y
        jmp     draw_bar_end_24.end

;------------------------------------------------------------------------------
; eax - color high   RRGGBB
; bl - process num
; ecx - temp
; edx - pointer to screen
; esi - counter
; edi - counter

draw_bar_end_32:

; check for hardware cursor
        mov     ecx, [_display.select_cursor]
        cmp     ecx, select_cursor
        je      draw_bar_end_32_new

  .new_y:
        mov     edi, [drbar.real_sx]
  .new_x:
        cmp     byte [ebp], bl
        jne     .skip

; store to LFB
        mov     [edx], eax
        mov     eax, [drbar.color]
  .skip:

; add pixel
        add     edx, 4
        inc     ebp
        dec     edi
        jnz     .new_x

; add line
        add     edx, [drbar.line_inc_scr]
        add     ebp, [drbar.line_inc_map]

; drawing gradient bars
        test    bh, 0x80
        jz      @f
        test    al, al
        jz      @f
        dec     al
  @@:
        dec     esi
        jnz     .new_y
  .end:
        add     esp, drbar.stack_data
        popad
        cmp     [SCR_MODE], 0x12
        jne     @f
        call    VGA_draw_bar
  @@:
        xor     eax, eax
        mov     [EGA_counter], 1
        ret

;------------------------------------------------------------------------------

align 4
draw_bar_end_32_new:

  .new_y:
        mov     edi, [drbar.real_sx]
  .new_x:
        cmp     byte [ebp], bl
        jne     .skip

        mov     ecx, [drbar.real_sy_and_abs_cy]
        sub     ecx, esi

; check for Y
        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     .no_mouse_area
        sub     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      .no_mouse_area
        rol     ecx, 16
        add     ecx, [drbar.real_sx_and_abs_cx]
        sub     ecx, edi

; check for X
        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     .no_mouse_area
        sub     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      .no_mouse_area
        ror     ecx, 16

; check mouse area for putpixel
        push    eax
        call    check_mouse_area_for_putpixel_new.1
        mov     [edx], eax
        pop     eax
        jmp     .skip
  .no_mouse_area:

; store to LFB
        mov     [edx], eax
  .skip:

; add pixel
        add     edx, 4
        inc     ebp
        dec     edi
        jnz     .new_x

; add line
        add     edx, [drbar.line_inc_scr]
        add     ebp, [drbar.line_inc_map]

; drawing gradient bars
        test    bh, 0x80
        jz      @f
        test    al, al
        jz      @f
        dec     al
  @@:
        dec     esi
        jnz     .new_y
        jmp     draw_bar_end_32.end

;------------------------------------------------------------------------------
; eax - color high   RRGGBB
; bl - process num
; ecx - temp
; edx - pointer to screen
; esi - counter
; edi - counter

align 4
draw_bar_end_16:

; check for hardware cursor
        mov     ecx, [_display.select_cursor]
        cmp     ecx, select_cursor
        je      draw_bar_end_16_new

  .new_y:
        mov     edi, [drbar.real_sx]
  .new_x:
        cmp     byte [ebp], bl
        jne     .skip
; convert to 16 bpp and store to LFB
        and     eax, 00000000111110001111110011111000b
        shr     ah, 2
        shr     ax, 3
        ror     eax, 8
        add     al, ah
        rol     eax, 8
        mov     [edx], ax
        mov     eax, [drbar.color]
  .skip:

; add pixel
        add     edx, 2
        inc     ebp
        dec     edi
        jnz     .new_x
; add line
        add     edx, [drbar.line_inc_scr]
        add     ebp, [drbar.line_inc_map]
; drawing gradient bars
        test    bh, 0x80
        jz      @f
        test    al, al
        jz      @f
        dec     al
  @@:
        dec     esi
        jnz     .new_y
  .end:
        add     esp, drbar.stack_data
        popad
        cmp     [SCR_MODE], 0x12
        jne     @f
        call    VGA_draw_bar
  @@:
        xor     eax, eax
        mov     [EGA_counter], 1
        ret

;------------------------------------------------------------------------------

align 4
draw_bar_end_16_new:

  .new_y:
        mov     edi, [drbar.real_sx]
  .new_x:
        cmp     byte [ebp], bl
        jne     .skip

        mov     ecx, [drbar.real_sy_and_abs_cy]
        sub     ecx, esi

; check for Y
        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     .no_mouse_area
        sub     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      .no_mouse_area
        rol     ecx, 16
        add     ecx, [drbar.real_sx_and_abs_cx]
        sub     ecx, edi

; check for X
        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     .no_mouse_area
        sub     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      .no_mouse_area
        ror     ecx, 16

; check mouse area for putpixel
        push    eax
        call    check_mouse_area_for_putpixel_new.1
        push    eax
        and     eax, 00000000111110001111110011111000b
        shr     ah, 2
        shr     ax, 3
        ror     eax, 8
        add     al, ah
        rol     eax, 8
        mov     [edx], ax
        pop     eax
        pop     eax
        jmp     .skip

  .no_mouse_area:
; convert to 16 bpp and store to LFB
        push    eax
        and     eax, 00000000111110001111110011111000b
        shr     ah, 2
        shr     ax, 3
        ror     eax, 8
        add     al, ah
        rol     eax, 8
        mov     [edx], ax
        pop     eax
  .skip:

; add pixel
        add     edx, 2
        inc     ebp
        dec     edi
        jnz     .new_x

; add line
        add     edx, [drbar.line_inc_scr]
        add     ebp, [drbar.line_inc_map]

; drawing gradient bars
        test    bh, 0x80
        jz      @f
        test    al, al
        jz      @f
        dec     al
  @@:
        dec     esi
        jnz     .new_y
        jmp     draw_bar_end_16.end

;------------------------------------------------------------------------------

align 4
vesa20_drawbackground_tiled:

        pushad
; External loop for all y from start to end
        mov     ebx, [background_window + WDATA.draw_data.top]    ; y start
  .dp2:
        mov     ebp, [background_window + WDATA.draw_data.left]   ; x start
; 1) Calculate pointers in WinMapAddress (does pixel belong to OS thread?) [ebp]
;    and LFB data (output for our function) [edi]
;        mov     eax, [BytesPerScanLine]
;        mul     ebx
        mov     eax, [BPSLine_calc_area+ebx*4]
        xchg    ebp, eax
        add     ebp, eax
        add     ebp, eax
        cmp     byte [_display.bytes_per_pixel], 2
        je      @f
        add     ebp, eax
        cmp     byte [_display.bytes_per_pixel], 3
        je      @f
        add     ebp, eax

  @@:
        add     ebp, LFB_BASE
; ebp:=Y*BytesPerScanLine+X*BytesPerPixel+AddrLFB
        call    calculate_edi
        xchg    edi, ebp
        add     ebp, [_display.win_map]
; Now eax=x, ebx=y, edi->output, ebp=offset in WinMapAddress
; 2) Calculate offset in background memory block
        push    eax
        xor     edx, edx
        mov     eax, ebx
        div     dword [BgrDataHeight]   ; edx := y mod BgrDataHeight
        pop     eax
        push    eax
        mov     ecx, [BgrDataWidth]
        mov     esi, edx
        imul    esi, ecx                ; esi := (y mod BgrDataHeight) * BgrDataWidth
        xor     edx, edx
        div     ecx             ; edx := x mod BgrDataWidth
        sub     ecx, edx
        add     esi, edx        ; esi := (y mod BgrDataHeight)*BgrDataWidth + (x mod BgrDataWidth)
        pop     eax
        lea     esi, [esi*3]
        add     esi, [img_background]
        xor     edx, edx
        inc     edx
; 3) Loop through redraw rectangle and copy background data
; Registers meaning:
; eax = x, ebx = y (screen coordinates)
; ecx = deltax - number of pixels left in current tile block
; edx = 1
; esi -> bgr memory, edi -> output
; ebp = offset in WinMapAddress
  .dp3:
        cmp     [ebp], dl
        jnz     .next_pix

        push    eax ecx
        mov     ecx, eax
        shl     ecx, 16
        add     ecx, ebx

        mov     eax, [esi]

; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     .no_mouseunder

        and     eax, 0xffffff
; check mouse area for putpixel
        call    [_display.check_mouse]
  .no_mouseunder:

        cmp     byte [_display.bits_per_pixel], 16
        je      .16bpp
; store to LFB
        mov     [edi], ax
        shr     eax, 16
        mov     [edi+2], al
        pop     ecx eax
        jmp     .next_pix

  .16bpp:
; convert to 16 bpp and store to LFB
        and     eax, 00000000111110001111110011111000b
        shr     ah, 2
        shr     ax, 3
        ror     eax, 8
        add     al, ah
        rol     eax, 8
        mov     [edi], ax
        pop     ecx eax

; Advance to next pixel
  .next_pix:
        add     esi, 3
        add     edi, [_display.bytes_per_pixel]

        add     ebp, edx
        add     eax, edx
        cmp     eax, [background_window + WDATA.draw_data.right]
        ja      .dp4
        sub     ecx, edx
        jnz     .dp3

; next tile block on x-axis
        mov     ecx, [BgrDataWidth]
        sub     esi, ecx
        sub     esi, ecx
        sub     esi, ecx
        jmp     .dp3

  .dp4:
; next scan line
        inc     ebx
        cmp     ebx, [background_window + WDATA.draw_data.bottom]
        jbe     .dp2
        popad
        mov     [EGA_counter], 1
        cmp     [SCR_MODE], 0x12
        jne     @f
        call    VGA_drawbackground
  @@:
        ret

;------------------------------------------------------------------------------

align 4
vesa20_drawbackground_stretch:

        pushad
; Helper variables
; calculate 2^32*(BgrDataWidth-1) mod (ScreenWidth-1)
        mov     eax, [BgrDataWidth]
        dec     eax
        xor     edx, edx
        div     dword [_display.width]
        push    eax     ; high
        xor     eax, eax
        div     dword [_display.width]
        push    eax     ; low

; the same for height
        mov     eax, [BgrDataHeight]
        dec     eax
        xor     edx, edx
        div     dword [_display.height]
        push    eax     ; high
        xor     eax, eax
        div     dword [_display.height]
        push    eax     ; low

; External loop for all y from start to end
        mov     ebx, [background_window + WDATA.draw_data.top]    ; y start
        mov     ebp, [background_window + WDATA.draw_data.left]   ; x start
; 1) Calculate pointers in WinMapAddress (does pixel belong to OS thread?) [ebp]
;                       and LFB data (output for our function) [edi]
;        mov     eax, [BytesPerScanLine]
;        mul     ebx
        mov     eax, [BPSLine_calc_area+ebx*4]
        xchg    ebp, eax
        add     ebp, eax
        add     ebp, eax
        cmp     byte [_display.bytes_per_pixel], 2
        jz      @f
        add     ebp, eax
        cmp     byte [_display.bytes_per_pixel], 3
        jz      @f
        add     ebp, eax
  @@:

; ebp:=Y*BytesPerScanLine+X*BytesPerPixel+AddrLFB
        call    calculate_edi
        xchg    edi, ebp

; Now eax=x, ebx=y, edi->output, ebp=offset in WinMapAddress
        push    ebx
        push    eax
; 2) Calculate offset in background memory block
        mov     eax, ebx
        imul    ebx, dword [esp+12]
        mul     dword [esp+8]
        add     edx, ebx        ; edx:eax = y * 2^32*(BgrDataHeight-1)/(ScreenHeight-1)
        mov     esi, edx
        imul    esi, [BgrDataWidth]
        push    edx
        push    eax
        mov     eax, [esp+8]
        mul     dword [esp+28]
        push    eax
        mov     eax, [esp+12]
        mul     dword [esp+28]
        add     [esp], edx
        pop     edx             ; edx:eax = x * 2^32*(BgrDataWidth-1)/(ScreenWidth-1)
        add     esi, edx
        lea     esi, [esi*3]
        add     esi, [img_background]
        push    eax
        push    edx
        push    esi

; 3) Smooth horizontal
  bgr_resmooth0:
        mov     ecx, [esp+8]
        mov     edx, [esp+4]
        mov     esi, [esp]
        push    edi
        mov     edi, bgr_cur_line
        call    smooth_line

  bgr_resmooth1:
        mov     eax, [esp+16+4]
        inc     eax
        cmp     eax, [BgrDataHeight]
        jae     bgr.no2nd
        mov     ecx, [esp+8+4]
        mov     edx, [esp+4+4]
        mov     esi, [esp+4]
        add     esi, [BgrDataWidth]
        add     esi, [BgrDataWidth]
        add     esi, [BgrDataWidth]
        mov     edi, bgr_next_line
        call    smooth_line

  bgr.no2nd:
        pop     edi

  sdp3:
        xor     esi, esi
        mov     ecx, [esp+12]

; 4) Loop through redraw rectangle and copy background data
; Registers meaning:
; esi = offset in current line, edi -> output
; ebp = offset in WinMapAddress
; dword [esp] = offset in bgr data
; qword [esp+4] = x * 2^32 * (BgrDataWidth-1) / (ScreenWidth-1)
; qword [esp+12] = y * 2^32 * (BgrDataHeight-1) / (ScreenHeight-1)
; dword [esp+20] = x
; dword [esp+24] = y
; precalculated constants:
; qword [esp+28] = 2^32*(BgrDataHeight-1)/(ScreenHeight-1)
; qword [esp+36] = 2^32*(BgrDataWidth-1)/(ScreenWidth-1)

  sdp3a:
        mov     eax, [_display.win_map]
        cmp     [ebp+eax], byte 1
        jnz     snbgp
        mov     eax, [bgr_cur_line+esi]
        test    ecx, ecx
        jz      .novert
        mov     ebx, [bgr_next_line+esi]
        call    [overlapping_of_points_ptr]

  .novert:
        push    ecx
; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     .no_mouseunder

        mov     ecx, [esp+20+4]        ;x
        shl     ecx, 16
        add     ecx, [esp+24+4]        ;y
; check mouse area for putpixel
        call    [_display.check_mouse]
  .no_mouseunder:

        cmp     [_display.bits_per_pixel], 16
        jne     .not_16bpp
; convert to 16 bpp and store to LFB
        and     eax, 00000000111110001111110011111000b
        shr     ah, 2
        shr     ax, 3
        ror     eax, 8
        add     al, ah
        rol     eax, 8
        mov     [LFB_BASE+edi], ax
        pop     ecx
        jmp     snbgp
  .not_16bpp:

; store to LFB
        mov     [LFB_BASE+edi], ax
        shr     eax, 16
        mov     [LFB_BASE+edi+2], al
        pop     ecx

  snbgp:
        add     edi, [_display.bytes_per_pixel]
        add     ebp, 1
        mov     eax, [esp+20]
        add     eax, 1
        mov     [esp+20], eax
        add     esi, 4
        cmp     eax, [background_window + WDATA.draw_data.right]
        jbe     sdp3a

  sdp4:
; next y
        mov     ebx, [esp+24]
        add     ebx, 1
        mov     [esp+24], ebx
        cmp     ebx, [background_window + WDATA.draw_data.bottom]
        ja      sdpdone

; advance edi, ebp to next scan line
        sub     eax, [background_window + WDATA.draw_data.left]
        sub     ebp, eax
        add     ebp, [_display.width]
        sub     edi, eax
        sub     edi, eax
        cmp     byte [_display.bytes_per_pixel], 2
        jz      @f
        sub     edi, eax
        cmp     byte [_display.bytes_per_pixel], 3
        jz      @f
        sub     edi, eax

  @@:
        add     edi, [_display.lfb_pitch]
; restore ecx,edx; advance esi to next background line
        mov     eax, [esp+28]
        mov     ebx, [esp+32]
        add     [esp+12], eax
        mov     eax, [esp+16]
        adc     [esp+16], ebx
        sub     eax, [esp+16]
        mov     ebx, eax
        lea     eax, [eax*3]
        imul    eax, [BgrDataWidth]
        sub     [esp], eax
        mov     eax, [background_window + WDATA.draw_data.left]
        mov     [esp+20], eax
        test    ebx, ebx
        jz      sdp3
        cmp     ebx, -1
        jnz     bgr_resmooth0
        push    edi
        mov     esi, bgr_next_line
        mov     edi, bgr_cur_line
        mov     ecx, [_display.width]
        rep movsd
        jmp     bgr_resmooth1

  sdpdone:
        add     esp, 44
        popad
        mov     [EGA_counter], 1
        cmp     [SCR_MODE], 0x12
        jne     @f
        call    VGA_drawbackground
  @@:
        ret

;--------------------------------------

align 4
smooth_line:
        mov     al, [esi+2]
        shl     eax, 16
        mov     ax, [esi]
        test    ecx, ecx
        jz      @f
        mov     ebx, [esi+2]
        shr     ebx, 8
        call    [overlapping_of_points_ptr]
  @@:
        stosd
        mov     eax, [esp+20+8]
        add     eax, 1
        mov     [esp+20+8], eax
        cmp     eax, [background_window + WDATA.draw_data.right]
        ja      @f
        add     ecx, [esp+36+8]
        mov     eax, edx
        adc     edx, [esp+40+8]
        sub     eax, edx
        lea     eax, [eax*3]
        sub     esi, eax
        jmp     smooth_line
  @@:
        mov     eax, [background_window + WDATA.draw_data.left]
        mov     [esp+20+8], eax
        ret

;------------------------------------------------------------------------------

align 16
overlapping_of_points:
if 0
; this version of procedure works, but is slower than next version
        push    ecx edx
        mov     edx, eax
        push    esi
        shr     ecx, 24
        mov     esi, ecx
        mov     ecx, ebx
        movzx   ebx, dl
        movzx   eax, cl
        sub     eax, ebx
        movzx   ebx, dh
        imul    eax, esi
        add     dl, ah
        movzx   eax, ch
        sub     eax, ebx
        imul    eax, esi
        add     dh, ah
        ror     ecx, 16
        ror     edx, 16
        movzx   eax, cl
        movzx   ebx, dl
        sub     eax, ebx
        imul    eax, esi
        pop     esi
        add     dl, ah
        mov     eax, edx
        pop     edx
        ror     eax, 16
        pop     ecx
        ret
else
        push    ecx edx
        mov     edx, eax
        push    esi
        shr     ecx, 26
        mov     esi, ecx
        mov     ecx, ebx
        shl     esi, 9
        movzx   ebx, dl
        movzx   eax, cl
        sub     eax, ebx
        movzx   ebx, dh
        add     dl, [BgrAuxTable + (eax + 0x100) + esi]
        movzx   eax, ch
        sub     eax, ebx
        add     dh, [BgrAuxTable + (eax+0x100) + esi]
        ror     ecx, 16
        ror     edx, 16
        movzx   eax, cl
        movzx   ebx, dl
        sub     eax, ebx
        add     dl, [BgrAuxTable + (eax + 0x100) + esi]
        pop     esi
        mov     eax, edx
        pop     edx
        ror     eax, 16
        pop     ecx
        ret
end if


;------------------------------------------------------------------------------

align 4
init_background:

        mov     edi, BgrAuxTable
        xor     edx, edx

  .loop2:
        mov     eax, edx
        shl     eax, 8
        neg     eax
        mov     ecx, 0x200

  .loop1:
        mov     byte [edi], ah
        inc     edi
        add     eax, edx
        loop    .loop1
        add     dl, 4
        jnz     .loop2
        test    byte [cpu_caps + (CAPS_MMX/8)], 1 shl (CAPS_MMX mod 8)
        jz      @f
        mov     [overlapping_of_points_ptr], overlapping_of_points_mmx
  @@:
        ret

;------------------------------------------------------------------------------

align 16
overlapping_of_points_mmx:

        movd    mm0, eax
        movd    mm4, eax
        movd    mm1, ebx
        pxor    mm2, mm2
        punpcklbw mm0, mm2
        punpcklbw mm1, mm2
        psubw   mm1, mm0
        movd    mm3, ecx
        psrld   mm3, 24
        packuswb mm3, mm3
        packuswb mm3, mm3
        pmullw  mm1, mm3
        psrlw   mm1, 8
        packuswb mm1, mm2
        paddb   mm4, mm1
        movd    eax, mm4

        ret

;------------------------------------------------------------------------------
