;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
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


; If you're planning to write your own video driver I suggest
; you replace the VESA12.INC file and see those instructions.

;Screen_Max_X             equ     0xfe00
;Screen_Max_Y            equ     0xfe04
;BytesPerScanLine        equ     0xfe08
;LFBAddress              equ     0xfe80
;ScreenBPP               equ     0xfbf1



;-----------------------------------------------------------------------------
; getpixel
;
; in:
; eax = x coordinate
; ebx = y coordinate
;
; ret:
; ecx = 00 RR GG BB
;-----------------------------------------------------------------------------
align 4
getpixel:
        push    eax ebx edx edi
        call    dword [GETPIXEL]
        pop     edi edx ebx eax
        ret
;-----------------------------------------------------------------------------
align 4
Vesa20_getpixel24:
; eax = x
; ebx = y
;--------------------------------------
; check mouse area for putpixel
        test    ecx, 0x04000000  ; don't load to mouseunder area
        jnz     .no_mouseunder
        call    [_display.check_m_pixel]
        test    ecx, ecx        ;0xff000000
        jnz     @f
.no_mouseunder:
;--------------------------------------
        imul    ebx, [BytesPerScanLine] ; ebx = y * y multiplier
        lea     edi, [eax+eax*2]; edi = x*3
        add     edi, ebx      ; edi = x*3+(y*y multiplier)
        mov     ecx, [LFB_BASE+edi]
;--------------------------------------
align 4
@@:
        and     ecx, 0xffffff
        ret
;-----------------------------------------------------------------------------
align 4
Vesa20_getpixel32:
;--------------------------------------
; check mouse area for putpixel
        test    ecx, 0x04000000  ; don't load to mouseunder area
        jnz     .no_mouseunder
        call    [_display.check_m_pixel]
        test    ecx, ecx        ;0xff000000
        jnz     @f
.no_mouseunder:
;--------------------------------------
        imul    ebx, [BytesPerScanLine] ; ebx = y * y multiplier
        lea     edi, [ebx+eax*4]; edi = x*4+(y*y multiplier)
        mov     ecx, [LFB_BASE+edi]
;--------------------------------------
align 4
@@:
        and     ecx, 0xffffff
        ret
;-----------------------------------------------------------------------------
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
;-----------------------------------------------------------------------------
align 16
; ebx = pointer
; ecx = size [x|y]
; edx = coordinates [x|y]
; ebp = pointer to 'get' function
; esi = pointer to 'init' function
; edi = parameter for 'get' function
vesa20_putimage:
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
        mov     eax, [TASK_BASE]
        mov     ebx, [eax-twdw + WDATA.box.left]
        add     ebx, [putimg.image_cx]
        mov     [putimg.abs_cx], ebx
        mov     ebx, [eax-twdw + WDATA.box.top]
        add     ebx, [putimg.image_cy]
        mov     [putimg.abs_cy], ebx
; real_sx = MIN(wnd_sx-image_cx, image_sx);
        mov     ebx, [eax-twdw + WDATA.box.width]; ebx = wnd_sx
; \begin{diamond}[20.08.2006]
; note that WDATA.box.width is one pixel less than real window x-size
        inc     ebx
; \end{diamond}[20.08.2006]
        sub     ebx, [putimg.image_cx]
        ja      @f
        add     esp, putimg.stack_data
        popad
        ret
;--------------------------------------
align 4
@@:
        cmp     ebx, [putimg.image_sx]
        jbe     .end_x
        mov     ebx, [putimg.image_sx]
;--------------------------------------
align 4
.end_x:
        mov     [putimg.real_sx], ebx
; init real_sy
        mov     ebx, [eax-twdw + WDATA.box.height]; ebx = wnd_sy
; \begin{diamond}[20.08.2006]
        inc     ebx
; \end{diamond}[20.08.2006]
        sub     ebx, [putimg.image_cy]
        ja      @f
        add     esp, putimg.stack_data
        popad
        ret
;--------------------------------------
align 4
@@:
        cmp     ebx, [putimg.image_sy]
        jbe     .end_y
        mov     ebx, [putimg.image_sy]
;--------------------------------------
align 4
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
        mov     eax, [Screen_Max_X]
        inc     eax
        sub     eax, [putimg.real_sx]
        mov     [putimg.winmap_newline], eax
; screen new line increment
        mov     eax, [BytesPerScanLine]
        movzx   ebx, byte [ScreenBPP]
        shr     ebx, 3
        imul    ecx, ebx
        sub     eax, ecx
        mov     [putimg.screen_newline], eax
; pointer to image
        mov     esi, [putimg.pti]
; pointer to screen
        mov     edx, [putimg.abs_cy]
        imul    edx, [BytesPerScanLine]
        mov     eax, [putimg.abs_cx]
        movzx   ebx, byte [ScreenBPP]
        shr     ebx, 3
        imul    eax, ebx
        add     edx, eax
; pointer to pixel map
        mov     eax, [putimg.abs_cy]
        imul    eax, [Screen_Max_X]
        add     eax, [putimg.abs_cy]
        add     eax, [putimg.abs_cx]
        add     eax, [_WinMapAddress]
        xchg    eax, ebp
;--------------------------------------
        mov     ecx, [putimg.real_sx]
        add     ecx, [putimg.abs_cx]
        mov     [putimg.real_sx_and_abs_cx], ecx
        mov     ecx, [putimg.real_sy]
        add     ecx, [putimg.abs_cy]
        mov     [putimg.real_sy_and_abs_cy], ecx
;--------------------------------------
; get process number
        mov     ebx, [CURRENT_TASK]
        cmp     byte [ScreenBPP], 32
        je      put_image_end_32
;--------------------------------------
put_image_end_24:
        mov     edi, [putimg.real_sy]
        cmp     [_display.select_cursor], 0
        jne     put_image_end_24_new
;--------------------------------------
align 4
.new_line:
        mov     ecx, [putimg.real_sx]
;--------------------------------------
align 4
.new_x:
        push    [putimg.edi]
        mov     eax, [putimg.ebp+4]
        call    eax
        cmp     [ebp], bl
        jne     .skip
;--------------------------------------
        push    ecx

        neg     ecx
        add     ecx, [putimg.real_sx_and_abs_cx + 4]
        shl     ecx, 16
        add     ecx, [putimg.real_sy_and_abs_cy + 4]
        sub     ecx, edi

; check mouse area for putpixel
        call    check_mouse_area_for_putpixel
        pop     ecx
; store to real LFB
        mov     [LFB_BASE+edx], ax
        shr     eax, 16
        mov     [LFB_BASE+edx+2], al
;--------------------------------------
align 4
.skip:
        add     edx, 3
        inc     ebp
        dec     ecx
        jnz     .new_x

        add     esi, [putimg.line_increment]
        add     edx, [putimg.screen_newline];[BytesPerScanLine]
        add     ebp, [putimg.winmap_newline];[Screen_Max_X]

        cmp     [putimg.ebp], putimage_get1bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get2bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get4bpp
        jnz     @f
;--------------------------------------
align 4
.correct:
        mov     eax, [putimg.edi]
        mov     byte [eax], 80h
;--------------------------------------
align 4
@@:
        dec     edi
        jnz     .new_line
;--------------------------------------
align 4
.finish:
        add     esp, putimg.stack_data
        popad
        ret
;--------------------------------------
align 4
put_image_end_24_new:
;--------------------------------------
align 4
.new_line:
        mov     ecx, [putimg.real_sx]
;--------------------------------------
align 4
.new_x:
        push    [putimg.edi]
        mov     eax, [putimg.ebp+4]
        call    eax
        cmp     [ebp], bl
        jne     .skip
;--------------------------------------
        push    ecx
        mov     ecx, [putimg.real_sy_and_abs_cy + 4]
        sub     ecx, edi
;--------------------------------------
; check for Y
        cmp     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      .no_mouse_area

        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     .no_mouse_area

        rol     ecx, 16
        add     ecx, [putimg.real_sx_and_abs_cx + 4]
        sub     ecx, [esp]
;--------------------------------------
; check for X
        cmp     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      .no_mouse_area

        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     .no_mouse_area
;--------------------------------------
; check mouse area for putpixel
        call    check_mouse_area_for_putpixel_new.1
;--------------------------------------
align 4
.no_mouse_area:
        pop     ecx
; store to real LFB
        mov     [LFB_BASE+edx], ax
        shr     eax, 16
        mov     [LFB_BASE+edx+2], al
;--------------------------------------
align 4
.skip:
        add     edx, 3
        inc     ebp
        dec     ecx
        jnz     .new_x

        add     esi, [putimg.line_increment]
        add     edx, [putimg.screen_newline];[BytesPerScanLine]
        add     ebp, [putimg.winmap_newline];[Screen_Max_X]

        cmp     [putimg.ebp], putimage_get1bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get2bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get4bpp
        jnz     @f
;--------------------------------------
align 4
.correct:
        mov     eax, [putimg.edi]
        mov     byte [eax], 80h
;--------------------------------------
align 4
@@:
        dec     edi
        jnz     .new_line
        jmp     put_image_end_24.finish
;------------------------------------------------------------------------------
align 4
put_image_end_32:
        mov     edi, [putimg.real_sy]
        cmp     [_display.select_cursor], 0
        jne     put_image_end_32_new
;--------------------------------------
align 4
.new_line:
        mov     ecx, [putimg.real_sx]
;--------------------------------------
align 4
.new_x:
        push    [putimg.edi]
        mov     eax, [putimg.ebp+4]
        call    eax
        cmp     [ebp], bl
        jne     .skip
;--------------------------------------
        push    ecx

        neg     ecx
        add     ecx, [putimg.real_sx_and_abs_cx + 4]
        shl     ecx, 16
        add     ecx, [putimg.real_sy_and_abs_cy + 4]
        sub     ecx, edi

; check mouse area for putpixel
        call    check_mouse_area_for_putpixel
        pop     ecx
; store to real LFB
        mov     [LFB_BASE+edx], eax
;--------------------------------------
align 4
.skip:
        add     edx, 4
        inc     ebp
        dec     ecx
        jnz     .new_x

        add     esi, [putimg.line_increment]
        add     edx, [putimg.screen_newline];[BytesPerScanLine]
        add     ebp, [putimg.winmap_newline];[Screen_Max_X]

        cmp     [putimg.ebp], putimage_get1bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get2bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get4bpp
        jnz     @f
;--------------------------------------
align 4
.correct:
        mov     eax, [putimg.edi]
        mov     byte [eax], 80h
;--------------------------------------
align 4
@@:
        dec     edi
        jnz     .new_line
;--------------------------------------
align 4
.finish:
        add     esp, putimg.stack_data
        popad
        cmp     [SCR_MODE], dword 0x12
        jne     @f
        call    VGA__putimage
;--------------------------------------
align 4
@@:
        mov     [EGA_counter], 1
        ret
;--------------------------------------
align 4
put_image_end_32_new:
;--------------------------------------
align 4
.new_line:
        mov     ecx, [putimg.real_sx]
;--------------------------------------
align 4
.new_x:
        push    [putimg.edi]
        mov     eax, [putimg.ebp+4]
        call    eax
        cmp     [ebp], bl
        jne     .skip
;--------------------------------------
        push    ecx
        mov     ecx, [putimg.real_sy_and_abs_cy + 4]
        sub     ecx, edi
;--------------------------------------
; check for Y
        cmp     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      .no_mouse_area

        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     .no_mouse_area

        rol     ecx, 16
        add     ecx, [putimg.real_sx_and_abs_cx + 4]
        sub     ecx, [esp]
;--------------------------------------
; check for X
        cmp     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      .no_mouse_area

        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     .no_mouse_area
;--------------------------------------
; check mouse area for putpixel
        call    check_mouse_area_for_putpixel_new.1
;--------------------------------------
align 4
.no_mouse_area:
        pop     ecx
; store to real LFB
        mov     [LFB_BASE+edx], eax
;--------------------------------------
align 4
.skip:
        add     edx, 4
        inc     ebp
        dec     ecx
        jnz     .new_x

        add     esi, [putimg.line_increment]
        add     edx, [putimg.screen_newline];[BytesPerScanLine]
        add     ebp, [putimg.winmap_newline];[Screen_Max_X]

        cmp     [putimg.ebp], putimage_get1bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get2bpp
        jz      .correct
        cmp     [putimg.ebp], putimage_get4bpp
        jnz     @f
;--------------------------------------
align 4
.correct:
        mov     eax, [putimg.edi]
        mov     byte [eax], 80h
;--------------------------------------
align 4
@@:
        dec     edi
        jnz     .new_line
        jmp     put_image_end_32.finish
;------------------------------------------------------------------------------
align 4
__sys_putpixel:

; eax = x coordinate
; ebx = y coordinate
; ecx = ?? RR GG BB    ; 0x01000000 negation
                       ; 0x02000000 used for draw_rectangle without top line
                       ;            for example drawwindow_III and drawwindow_IV
; edi = 0x00000001 force

;;;        mov  [novesachecksum], dword 0
        pushad
        cmp     [Screen_Max_X], eax
        jb      .exit
        cmp     [Screen_Max_Y], ebx
        jb      .exit
        test    edi, 1           ; force ?
        jnz     .forced

; not forced:
        mov     edx, [d_width_calc_area + ebx*4]
        add     edx, [_WinMapAddress]
        movzx   edx, byte [eax+edx]
        cmp     edx, [CURRENT_TASK]
        jne     .exit
;--------------------------------------
align 4
.forced:
; check if negation
        test    ecx, 0x01000000
        jz      .noneg

        call    getpixel
        not     ecx

        rol     ecx, 8
        mov     cl, [esp+32-8+3]
        ror     ecx, 8
        mov     [esp+32-8], ecx
;--------------------------------------
align 4
.noneg:
; OK to set pixel
        call    dword [PUTPIXEL]; call the real put_pixel function
;--------------------------------------
align 4
.exit:
        popad
        ret
;-----------------------------------------------------------------------------
align 4
Vesa20_putpixel24:
; eax = x
; ebx = y
        mov     ecx, eax
        shl     ecx, 16
        mov     cx, bx

        imul    ebx, [BytesPerScanLine]  ; ebx = y * y multiplier
        lea     edi, [eax+eax*2]; edi = x*3
        mov     eax, [esp+32-8+4]
;--------------------------------------
; check mouse area for putpixel
        test    eax, 0x04000000
        jnz     @f
        call    check_mouse_area_for_putpixel
;--------------------------------------
align 4
@@:
; store to real LFB
        mov     [LFB_BASE+ebx+edi], ax
        shr     eax, 16
        mov     [LFB_BASE+ebx+edi+2], al
        ret
;-----------------------------------------------------------------------------
align 4
Vesa20_putpixel24_new:
; eax = x
; ebx = y
        mov     ecx, eax
        shl     ecx, 16
        mov     cx, bx

        imul    ebx, [BytesPerScanLine]  ; ebx = y * y multiplier
        lea     edi, [eax+eax*2]; edi = x*3
        mov     eax, [esp+32-8+4]
;--------------------------------------
; check mouse area for putpixel
        test    eax, 0x04000000
        jnz     @f
;--------------------------------------
; check for Y
        cmp     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      @f

        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     @f

        rol     ecx, 16
;--------------------------------------
; check for X
        cmp     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      @f

        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     @f

        call    check_mouse_area_for_putpixel_new.1
;--------------------------------------
align 4
@@:
; store to real LFB
        mov     [LFB_BASE+ebx+edi], ax
        shr     eax, 16
        mov     [LFB_BASE+ebx+edi+2], al
        ret
;-----------------------------------------------------------------------------
align 4
Vesa20_putpixel32:
; eax = x
; ebx = y
        mov     ecx, eax
        shl     ecx, 16
        mov     cx, bx

        imul    ebx, [BytesPerScanLine]  ; ebx = y * y multiplier
        lea     edi, [ebx+eax*4]; edi = x*4+(y*y multiplier)
        mov     eax, [esp+32-8+4]; eax = color
;--------------------------------------
; check mouse area for putpixel
        test    eax, 0x04000000
        jnz     @f
        call    check_mouse_area_for_putpixel
;--------------------------------------
align 4
@@:
        and     eax, 0xffffff
; store to real LFB
        mov     [LFB_BASE+edi], eax
        ret
;-----------------------------------------------------------------------------
align 4
Vesa20_putpixel32_new:
; eax = x
; ebx = y
        mov     ecx, eax
        shl     ecx, 16
        mov     cx, bx

        imul    ebx, [BytesPerScanLine]  ; ebx = y * y multiplier
        lea     edi, [ebx+eax*4]; edi = x*4+(y*y multiplier)
        mov     eax, [esp+32-8+4]; eax = color
;--------------------------------------
; check mouse area for putpixel
        test    eax, 0x04000000
        jnz     @f
;--------------------------------------
; check for Y
        cmp     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      @f

        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     @f

        rol     ecx, 16
;--------------------------------------
; check for X
        cmp     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      @f

        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     @f

        call    check_mouse_area_for_putpixel_new.1
;--------------------------------------
align 4
@@:
        and     eax, 0xffffff
; store to real LFB
        mov     [LFB_BASE+edi], eax
        ret
;-----------------------------------------------------------------------------
align 4
calculate_edi:
        mov     edi, ebx
        imul    edi, [Screen_Max_X]
        add     edi, ebx
        add     edi, eax
        ret
;-----------------------------------------------------------------------------
; DRAWLINE
;-----------------------------------------------------------------------------
align 4
__sys_draw_line:
; draw a line
; eax = HIWORD = x1
;       LOWORD = x2
; ebx = HIWORD = y1
;       LOWORD = y2
; ecx = color
; edi = force ?
        pusha

dl_x1 equ esp+20
dl_y1 equ esp+16
dl_x2 equ esp+12
dl_y2 equ esp+8
dl_dx equ esp+4
dl_dy equ esp+0

        xor     edx, edx   ; clear edx
        xor     esi, esi   ; unpack arguments
        xor     ebp, ebp
        mov     si, ax     ; esi = x2
        mov     bp, bx     ; ebp = y2
        shr     eax, 16    ; eax = x1
        shr     ebx, 16    ; ebx = y1
        push    eax        ; save x1
        push    ebx        ; save y1
        push    esi        ; save x2
        push    ebp        ; save y2
; checking x-axis...
        sub     esi, eax   ; esi = x2-x1
        push    esi        ; save y2-y1
        jl      .x2lx1     ; is x2 less than x1 ?
        jg      .no_vline  ; x1 > x2 ?
        mov     edx, ebp   ; else (if x1=x2)
        call    vline
        push    edx ; necessary to rightly restore stack frame at .exit
        jmp     .exit
;--------------------------------------
align 4
.x2lx1:
        neg     esi         ; get esi absolute value
;--------------------------------------
align 4
.no_vline:
; checking y-axis...
        sub     ebp, ebx    ; ebp = y2-y1
        push    ebp         ; save y2-y1
        jl      .y2ly1      ; is y2 less than y1 ?
        jg      .no_hline   ; y1 > y2 ?
        mov     edx, [dl_x2]; else (if y1=y2)
        call    hline
        jmp     .exit
;--------------------------------------
align 4
.y2ly1:
        neg     ebp         ; get ebp absolute value
;--------------------------------------
align 4
.no_hline:
        cmp     ebp, esi
        jle     .x_rules    ; |y2-y1| < |x2-x1|  ?
        cmp     [dl_y2], ebx; make sure y1 is at the begining
        jge     .no_reverse1
        neg     dword [dl_dx]
        mov     edx, [dl_x2]
        mov     [dl_x2], eax
        mov     [dl_x1], edx
        mov     edx, [dl_y2]
        mov     [dl_y2], ebx
        mov     [dl_y1], edx
;--------------------------------------
align 4
.no_reverse1:
        mov     eax, [dl_dx]
        cdq                 ; extend eax sing to edx
        shl     eax, 16     ; using 16bit fix-point maths
        idiv    ebp         ; eax = ((x2-x1)*65536)/(y2-y1)
;--------------------------------------
; correction for the remainder of the division
        shl     edx, 1
        cmp     ebp, edx
        jb      @f
        inc     eax
;--------------------------------------
align 4
@@:
;--------------------------------------
        mov     edx, ebp    ; edx = counter (number of pixels to draw)
        mov     ebp, 1 *65536; <<16   ; ebp = dy = 1.0
        mov     esi, eax    ; esi = dx
        jmp     .y_rules
;--------------------------------------
align 4
.x_rules:
        cmp     [dl_x2], eax ; make sure x1 is at the begining
        jge     .no_reverse2
        neg     dword [dl_dy]
        mov     edx, [dl_x2]
        mov     [dl_x2], eax
        mov     [dl_x1], edx
        mov     edx, [dl_y2]
        mov     [dl_y2], ebx
        mov     [dl_y1], edx
;--------------------------------------
align 4
.no_reverse2:
        xor     edx, edx
        mov     eax, [dl_dy]
        cdq                 ; extend eax sing to edx
        shl     eax, 16     ; using 16bit fix-point maths
        idiv    esi         ; eax = ((y2-y1)*65536)/(x2-x1)
;--------------------------------------
; correction for the remainder of the division
        shl     edx, 1
        cmp     esi, edx
        jb      @f
        inc     eax
;--------------------------------------
align 4
@@:
;--------------------------------------
        mov     edx, esi    ; edx = counter (number of pixels to draw)
        mov     esi, 1 *65536;<< 16   ; esi = dx = 1.0
        mov     ebp, eax    ; ebp = dy
;--------------------------------------
align 4
.y_rules:
        mov     eax, [dl_x1]
        mov     ebx, [dl_y1]
        shl     eax, 16
        shl     ebx, 16
;-----------------------------------------------------------------------------
align 4
.draw:
        push    eax ebx
;--------------------------------------
; correction for the remainder of the division
        test    ah, 0x80
        jz      @f
        add     eax, 1 shl 16
;--------------------------------------
align 4
@@:
;--------------------------------------
        shr     eax, 16
;--------------------------------------
; correction for the remainder of the division
        test    bh, 0x80
        jz      @f
        add     ebx, 1 shl 16
;--------------------------------------
align 4
@@:
;--------------------------------------
        shr     ebx, 16
        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 save to mouseunder area
        call    [putpixel]
        pop     ebx eax
        add     ebx, ebp     ; y = y+dy
        add     eax, esi     ; x = x+dx
        dec     edx
        jnz     .draw
; force last drawn pixel to be at (x2,y2)
        mov     eax, [dl_x2]
        mov     ebx, [dl_y2]
        and     ecx, 0xFBFFFFFF ;negate 0x04000000 save to mouseunder area
        call    [putpixel]
;--------------------------------------
align 4
.exit:
        add     esp, 6*4
        popa
;        call    [draw_pointer]
        ret
;------------------------------------------------------------------------------
align 4
hline:
; draw an horizontal line
; eax = x1
; edx = x2
; ebx = y
; ecx = color
; edi = force ?
        push    eax edx
        cmp     edx, eax   ; make sure x2 is above x1
        jge     @f
        xchg    eax, edx
;--------------------------------------
align 4
@@:
        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 save to mouseunder area
        call    [putpixel]
        inc     eax
        cmp     eax, edx
        jle     @b
        pop     edx eax
        ret
;------------------------------------------------------------------------------
align 4
vline:
; draw a vertical line
; eax = x
; ebx = y1
; edx = y2
; ecx = color
; edi = force ?
        push    ebx edx
        cmp     edx, ebx   ; make sure y2 is above y1
        jge     @f
        xchg    ebx, edx
;--------------------------------------
align 4
@@:
        and     ecx, 0xFBFFFFFF ;negate 0x04000000 save to mouseunder area
        call    [putpixel]
        inc     ebx
        cmp     ebx, edx
        jle     @b
        pop     edx ebx
        ret
;------------------------------------------------------------------------------
align 4
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
;--------------------------------------
align 4
; eax   cx
; ebx   cy
; ecx   xe
; edx   ye
; edi   color
vesa20_drawbar:
        pushad
        sub     esp, drbar.stack_data
        mov     [drbar.color], edi
        sub     edx, ebx
        jle     .exit       ;// mike.dld, 2005-01-29
        sub     ecx, eax
        jle     .exit       ;// mike.dld, 2005-01-29
        mov     [drbar.bar_sy], edx
        mov     [drbar.bar_sx], ecx
        mov     [drbar.bar_cx], eax
        mov     [drbar.bar_cy], ebx
        mov     edi, [TASK_BASE]
        add     eax, [edi-twdw + WDATA.box.left]; win_cx
        add     ebx, [edi-twdw + WDATA.box.top]; win_cy
        mov     [drbar.abs_cx], eax
        mov     [drbar.abs_cy], ebx
; real_sx = MIN(wnd_sx-bar_cx, bar_sx);
        mov     ebx, [edi-twdw + WDATA.box.width]; ebx = wnd_sx
; \begin{diamond}[20.08.2006]
; note that WDATA.box.width is one pixel less than real window x-size
        inc     ebx
; \end{diamond}[20.08.2006]
        sub     ebx, [drbar.bar_cx]
        ja      @f
;--------------------------------------
align 4
.exit:                       ;// mike.dld, 2005-01-29
        add     esp, drbar.stack_data
        popad
        xor     eax, eax
        inc     eax
        ret
;--------------------------------------
align 4
@@:
        cmp     ebx, [drbar.bar_sx]
        jbe     .end_x
        mov     ebx, [drbar.bar_sx]
;--------------------------------------
align 4
.end_x:
        mov     [drbar.real_sx], ebx
; real_sy = MIN(wnd_sy-bar_cy, bar_sy);
        mov     ebx, [edi-twdw + WDATA.box.height]; ebx = wnd_sy
; \begin{diamond}[20.08.2006]
        inc     ebx
; \end{diamond}
        sub     ebx, [drbar.bar_cy]
        ja      @f
        add     esp, drbar.stack_data
        popad
        xor     eax, eax
        inc     eax
        ret
;--------------------------------------
align 4
@@:
        cmp     ebx, [drbar.bar_sy]
        jbe     .end_y
        mov     ebx, [drbar.bar_sy]
;--------------------------------------
align 4
.end_y:
        mov     [drbar.real_sy], ebx
; line_inc_map
        mov     eax, [Screen_Max_X]
        sub     eax, [drbar.real_sx]
        inc     eax
        mov     [drbar.line_inc_map], eax
; line_inc_scr
        mov     eax, [drbar.real_sx]
        movzx   ebx, byte [ScreenBPP]
        shr     ebx, 3
        imul    eax, ebx
        neg     eax
        add     eax, [BytesPerScanLine]
        mov     [drbar.line_inc_scr], eax
; pointer to screen
        mov     edx, [drbar.abs_cy]
        imul    edx, [BytesPerScanLine]
        mov     eax, [drbar.abs_cx]
;     movzx   ebx, byte [ScreenBPP]
;     shr     ebx, 3
        imul    eax, ebx
        add     edx, eax
; pointer to pixel map
        mov     eax, [drbar.abs_cy]
        imul    eax, [Screen_Max_X]
        add     eax, [drbar.abs_cy]
        add     eax, [drbar.abs_cx]
        add     eax, [_WinMapAddress]
        xchg    eax, ebp
;--------------------------------------
        mov     ebx, [drbar.real_sx]
        add     ebx, [drbar.abs_cx]
        mov     [drbar.real_sx_and_abs_cx], ebx
        mov     ebx, [drbar.real_sy]
        add     ebx, [drbar.abs_cy]
        mov     [drbar.real_sy_and_abs_cy], ebx

        add     edx, LFB_BASE
;--------------------------------------
; get process number
        mov     ebx, [CURRENT_TASK]
        cmp     byte [ScreenBPP], 24
        jne     draw_bar_end_32
;--------------------------------------
align 4
draw_bar_end_24:
        mov     eax, [drbar.color] ;; BBGGRR00
        mov     bh, al             ;; bh  = BB
        shr     eax, 8             ;; eax = RRGG
; eax - color high   RRGG
; bl - process num
; bh - color low    BB
; ecx - temp
; edx - pointer to screen
; esi - counter
; edi - counter
        mov     esi, [drbar.real_sy]
        cmp     [_display.select_cursor], 0
        jne     draw_bar_end_24_new
;--------------------------------------
align 4
.new_y:
        mov     edi, [drbar.real_sx]
;--------------------------------------
align 4
.new_x:
        cmp     byte [ebp], bl
        jne     .skip
;--------------------------------------
        push    eax

        mov     ecx, [drbar.real_sx_and_abs_cx + 4]
        sub     ecx, edi
        shl     ecx, 16
        add     ecx, [drbar.real_sy_and_abs_cy + 4]
        sub     ecx, esi

        shl     eax, 8
        mov     al, bh
; check mouse area for putpixel
        call    check_mouse_area_for_putpixel
; store to real LFB
        mov     [edx], ax
        shr     eax, 16
        mov     [edx + 2], al
        pop     eax
;--------------------------------------
align 4
.skip:
; add pixel
        add     edx, 3
        inc     ebp
        dec     edi
        jnz     .new_x
; add line
        add     edx, [drbar.line_inc_scr]
        add     ebp, [drbar.line_inc_map]
; <Ivan 15.10.04> drawing gradient bars
        test    eax, 0x00800000
        jz      @f
        test    bh, bh
        jz      @f
        dec     bh
;--------------------------------------
align 4
@@:
; </Ivan 15.10.04>
        dec     esi
        jnz     .new_y
;--------------------------------------
align 4
.end:
        add     esp, drbar.stack_data
        popad
        xor     eax, eax
        ret
;--------------------------------------
align 4
draw_bar_end_24_new:
;--------------------------------------
align 4
.new_y:
        mov     edi, [drbar.real_sx]
;--------------------------------------
align 4
.new_x:
        cmp     byte [ebp], bl
        jne     .skip
;--------------------------------------
        mov     ecx, [drbar.real_sy_and_abs_cy]
        sub     ecx, esi
;--------------------------------------
; check for Y
        cmp     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      .no_mouse_area

        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     .no_mouse_area

        rol     ecx, 16
        add     ecx, [drbar.real_sx_and_abs_cx]
        sub     ecx, edi
;--------------------------------------
; check for X
        cmp     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      .no_mouse_area

        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     .no_mouse_area
;--------------------------------------
; check mouse area for putpixel
        push    eax

        shl     eax, 8
        mov     al, bh

        call    check_mouse_area_for_putpixel_new.1
; store to real LFB
        mov     [edx], ax
        shr     eax, 16
        mov     [edx + 2], al
        pop     eax
        jmp     .skip
; store to real LFB
;--------------------------------------
align 4
.no_mouse_area:
        mov     [edx], bh
        mov     [edx + 1], ax
;--------------------------------------
align 4
.skip:
; add pixel
        add     edx, 3
        inc     ebp
        dec     edi
        jnz     .new_x
; add line
        add     edx, [drbar.line_inc_scr]
        add     ebp, [drbar.line_inc_map]
; <Ivan 15.10.04> drawing gradient bars
        test    eax, 0x00800000
        jz      @f
        test    bh, bh
        jz      @f
        dec     bh
;--------------------------------------
align 4
@@:
; </Ivan 15.10.04>
        dec     esi
        jnz     .new_y
        jmp     draw_bar_end_24.end
;--------------------------------------
align 4
draw_bar_end_32:
        mov     eax, [drbar.color] ;; BBGGRR00
        mov     esi, [drbar.real_sy]
        cmp     [_display.select_cursor], 0
        jne     draw_bar_end_32_new
;--------------------------------------
align 4
.new_y:
        mov     edi, [drbar.real_sx]
;--------------------------------------
align 4
.new_x:
        cmp     byte [ebp], bl
        jne     .skip
;--------------------------------------
        push    eax

        mov     ecx, [drbar.real_sx_and_abs_cx + 4]
        sub     ecx, edi
        shl     ecx, 16
        add     ecx, [drbar.real_sy_and_abs_cy + 4]
        sub     ecx, esi

; check mouse area for putpixel
        call    check_mouse_area_for_putpixel
; store to real LFB
        mov     [edx], eax
        pop     eax
;--------------------------------------
align 4
.skip:
; add pixel
        add     edx, 4
        inc     ebp
        dec     edi
        jnz     .new_x
; add line
        add     edx, [drbar.line_inc_scr]
        add     ebp, [drbar.line_inc_map]
; <Ivan 15.10.04> drawing gradient bars
        test    eax, 0x80000000
        jz      @f
        test    al, al
        jz      @f
        dec     al
;--------------------------------------
align 4
@@:
; </Ivan 15.10.04>
        dec     esi
        jnz     .new_y
;--------------------------------------
align 4
.end:
        add     esp, drbar.stack_data
        popad
        cmp     [SCR_MODE], dword 0x12
        jne     @f
        call    VGA_draw_bar
;--------------------------------------
align 4
@@:
        xor     eax, eax
        mov     [EGA_counter], 1
        ret
;--------------------------------------
align 4
draw_bar_end_32_new:
;--------------------------------------
align 4
.new_y:
        mov     edi, [drbar.real_sx]
;--------------------------------------
align 4
.new_x:
        cmp     byte [ebp], bl
        jne     .skip
;--------------------------------------
        mov     ecx, [drbar.real_sy_and_abs_cy]
        sub     ecx, esi
;--------------------------------------
; check for Y
        cmp     cx, [Y_UNDER_subtraction_CUR_hot_y]
        jb      .no_mouse_area

        cmp     cx, [Y_UNDER_sub_CUR_hot_y_add_curh]
        jae     .no_mouse_area

        rol     ecx, 16
        add     ecx, [drbar.real_sx_and_abs_cx]
        sub     ecx, edi
;--------------------------------------
; check for X
        cmp     cx, [X_UNDER_subtraction_CUR_hot_x]
        jb      .no_mouse_area

        cmp     cx, [X_UNDER_sub_CUR_hot_x_add_curh]
        jae     .no_mouse_area
;--------------------------------------
; check mouse area for putpixel
        push    eax
        call    check_mouse_area_for_putpixel_new.1
        mov     [edx], eax
        pop     eax
        jmp     .skip
; store to real LFB
;--------------------------------------
align 4
.no_mouse_area:
        mov     [edx], eax
;--------------------------------------
align 4
.skip:
; add pixel
        add     edx, 4
        inc     ebp
        dec     edi
        jnz     .new_x
; add line
        add     edx, [drbar.line_inc_scr]
        add     ebp, [drbar.line_inc_map]
; <Ivan 15.10.04> drawing gradient bars
        test    eax, 0x80000000
        jz      @f
        test    al, al
        jz      @f
        dec     al
;--------------------------------------
align 4
@@:
; </Ivan 15.10.04>
        dec     esi
        jnz     .new_y
        jmp     draw_bar_end_32.end
;------------------------------------------------------------------------------
align 4
vesa20_drawbackground_tiled:
        pushad
; External loop for all y from start to end
        mov     ebx, [draw_data+32+RECT.top]    ; y start
;--------------------------------------
align 4
dp2:
        mov     ebp, [draw_data+32+RECT.left]   ; x start
; 1) Calculate pointers in WinMapAddress (does pixel belong to OS thread?) [ebp]
;                       and LFB data (output for our function) [edi]
        mov     eax, [BytesPerScanLine]
        mul     ebx
        xchg    ebp, eax
        add     ebp, eax
        add     ebp, eax
        add     ebp, eax
        cmp     [ScreenBPP], byte 24    ; 24 or 32 bpp ? - x size
        jz      @f
        add     ebp, eax
;--------------------------------------
align 4
@@:
        add     ebp, LFB_BASE
; ebp:=Y*BytesPerScanLine+X*BytesPerPixel+AddrLFB
        call    calculate_edi
        xchg    edi, ebp
        add     ebp, [_WinMapAddress]
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
;--------------------------------------
align 4
dp3:
        cmp     [ebp], dl
        jnz     nbgp
;--------------------------------------
        push    eax ecx

        mov     ecx, eax
        shl     ecx, 16
        add     ecx, ebx

        mov     eax, [esi]
        and     eax, 0xffffff
; check mouse area for putpixel
        call    [_display.check_mouse]
; store to real LFB
        mov     [edi], ax
        shr     eax, 16
        mov     [edi+2], al

        pop     ecx eax
;--------------------------------------
align 4
nbgp:
        add     esi, 3
        add     edi, 3
;--------------------------------------
align 4
@@:
        cmp     [ScreenBPP], byte 25    ; 24 or 32 bpp?
        sbb     edi, -1         ; +1 for 32 bpp
; I do not use 'inc eax' because this is slightly slower then 'add eax,1'
        add     ebp, edx
        add     eax, edx
        cmp     eax, [draw_data+32+RECT.right]
        ja      dp4
        sub     ecx, edx
        jnz     dp3
; next tile block on x-axis
        mov     ecx, [BgrDataWidth]
        sub     esi, ecx
        sub     esi, ecx
        sub     esi, ecx
        jmp     dp3
;--------------------------------------
align 4
dp4:
; next scan line
        inc     ebx
        cmp     ebx, [draw_data+32+RECT.bottom]
        jbe     dp2
        popad
        mov     [EGA_counter], 1
        cmp     [SCR_MODE], dword 0x12
        jne     @f
        call    VGA_drawbackground
;--------------------------------------
align 4
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
        div     dword [Screen_Max_X]
        push    eax     ; high
        xor     eax, eax
        div     dword [Screen_Max_X]
        push    eax     ; low
; the same for height
        mov     eax, [BgrDataHeight]
        dec     eax
        xor     edx, edx
        div     dword [Screen_Max_Y]
        push    eax     ; high
        xor     eax, eax
        div     dword [Screen_Max_Y]
        push    eax     ; low
; External loop for all y from start to end
        mov     ebx, [draw_data+32+RECT.top]    ; y start
        mov     ebp, [draw_data+32+RECT.left]   ; x start
; 1) Calculate pointers in WinMapAddress (does pixel belong to OS thread?) [ebp]
;                       and LFB data (output for our function) [edi]
        mov     eax, [BytesPerScanLine]
        mul     ebx
        xchg    ebp, eax
        add     ebp, eax
        add     ebp, eax
        add     ebp, eax
        cmp     [ScreenBPP], byte 24    ; 24 or 32 bpp ? - x size
        jz      @f
        add     ebp, eax
;--------------------------------------
align 4
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
;--------------------------------------
align 4
bgr_resmooth0:
        mov     ecx, [esp+8]
        mov     edx, [esp+4]
        mov     esi, [esp]
        push    edi
        mov     edi, bgr_cur_line
        call    smooth_line
;--------------------------------------
align 4
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
;--------------------------------------
align 4
bgr.no2nd:
        pop     edi
;--------------------------------------
align 4
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
;--------------------------------------
align 4
sdp3a:
        mov     eax, [_WinMapAddress]
        cmp     [ebp+eax], byte 1
        jnz     snbgp
        mov     eax, [bgr_cur_line+esi]
        test    ecx, ecx
        jz      .novert
        mov     ebx, [bgr_next_line+esi]
        call    [overlapping_of_points_ptr]
;--------------------------------------
align 4
.novert:
        push    ecx
        mov     ecx, [esp+20+4]        ;x
        shl     ecx, 16
        add     ecx, [esp+24+4]        ;y
; check mouse area for putpixel
        call    [_display.check_mouse]
; store to real LFB
        mov     [LFB_BASE+edi], ax
        shr     eax, 16
        mov     [LFB_BASE+edi+2], al
        pop     ecx
;--------------------------------------
align 4
snbgp:
        cmp     [ScreenBPP], byte 25
        sbb     edi, -4
        add     ebp, 1
        mov     eax, [esp+20]
        add     eax, 1
        mov     [esp+20], eax
        add     esi, 4
        cmp     eax, [draw_data+32+RECT.right]
        jbe     sdp3a
;--------------------------------------
align 4
sdp4:
; next y
        mov     ebx, [esp+24]
        add     ebx, 1
        mov     [esp+24], ebx
        cmp     ebx, [draw_data+32+RECT.bottom]
        ja      sdpdone
; advance edi, ebp to next scan line
        sub     eax, [draw_data+32+RECT.left]
        sub     ebp, eax
        add     ebp, [Screen_Max_X]
        add     ebp, 1
        sub     edi, eax
        sub     edi, eax
        sub     edi, eax
        cmp     [ScreenBPP], byte 24
        jz      @f
        sub     edi, eax
;--------------------------------------
align 4
@@:
        add     edi, [BytesPerScanLine]
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
        mov     eax, [draw_data+32+RECT.left]
        mov     [esp+20], eax
        test    ebx, ebx
        jz      sdp3
        cmp     ebx, -1
        jnz     bgr_resmooth0
        push    edi
        mov     esi, bgr_next_line
        mov     edi, bgr_cur_line
        mov     ecx, [Screen_Max_X]
        inc     ecx
        rep movsd
        jmp     bgr_resmooth1
;--------------------------------------
align 4
sdpdone:
        add     esp, 44
        popad
        mov     [EGA_counter], 1
        cmp     [SCR_MODE], dword 0x12
        jne     @f
        call    VGA_drawbackground
;--------------------------------------
align 4
@@:
        ret

uglobal
;--------------------------------------
align 4
bgr_cur_line    rd      1920    ; maximum width of screen
bgr_next_line   rd      1920
;--------------------------------------
endg
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
;--------------------------------------
align 4
@@:
        stosd
        mov     eax, [esp+20+8]
        add     eax, 1
        mov     [esp+20+8], eax
        cmp     eax, [draw_data+32+RECT.right]
        ja      @f
        add     ecx, [esp+36+8]
        mov     eax, edx
        adc     edx, [esp+40+8]
        sub     eax, edx
        lea     eax, [eax*3]
        sub     esi, eax
        jmp     smooth_line
;--------------------------------------
align 4
@@:
        mov     eax, [draw_data+32+RECT.left]
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
        add     dl, [BgrAuxTable+(eax+0x100)+esi]
        movzx   eax, ch
        sub     eax, ebx
        add     dh, [BgrAuxTable+(eax+0x100)+esi]
        ror     ecx, 16
        ror     edx, 16
        movzx   eax, cl
        movzx   ebx, dl
        sub     eax, ebx
        add     dl, [BgrAuxTable+(eax+0x100)+esi]
        pop     esi
        mov     eax, edx
        pop     edx
        ror     eax, 16
        pop     ecx
        ret
end if

iglobal
;--------------------------------------
align 4
overlapping_of_points_ptr       dd      overlapping_of_points
;--------------------------------------
endg
;------------------------------------------------------------------------------
align 4
init_background:
        mov     edi, BgrAuxTable
        xor     edx, edx
;--------------------------------------
align 4
.loop2:
        mov     eax, edx
        shl     eax, 8
        neg     eax
        mov     ecx, 0x200
;--------------------------------------
align 4
.loop1:
        mov     byte [edi], ah
        inc     edi
        add     eax, edx
        loop    .loop1
        add     dl, 4
        jnz     .loop2
        test    byte [cpu_caps+(CAPS_MMX/8)], 1 shl (CAPS_MMX mod 8)
        jz      @f
        mov     [overlapping_of_points_ptr], overlapping_of_points_mmx
;--------------------------------------
align 4
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
