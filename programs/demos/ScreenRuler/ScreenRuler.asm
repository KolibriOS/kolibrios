;---------------------------------------------------------------------
;    Screen Ruler v1.0
;    Version for KolibriOS 2005-2023
;---------------------------------------------------------------------
; last update:  11.09.2023
; created by:   Subbotin Anton aka Spaceraven
;---------------------------------------------------------------------
        use32
        org     0x0
        db      'MENUET01'; 8 byte id
        dd      1         ; header version
        dd      START     ; program start
        dd      I_END     ; program image size
        dd      0x1000    ; required amount of memory
        dd      0x1000    ; esp
        dd      0, 0      ; no parameters, no path
;---------------------------------------------------------------------
        include 'lang.inc'
        include '..\..\macros.inc'

        delay   = 20
        magnify_width = 48
        magnify_height = 30
        magnify_halfwidth = magnify_width / 2
        magnify_halfheight = magnify_height / 2
        aim0 = (magnify_halfheight - 1) * 65536 + magnify_halfwidth - 1
        aim1 = (magnify_halfheight - 1) * 65536 + magnify_halfwidth + 1
        aim2 = (magnify_halfheight + 1) * 65536 + magnify_halfwidth - 1
        aim3 = (magnify_halfheight + 1) * 65536 + magnify_halfwidth + 1

;------------------------- Main cycle
START:
redraw:
        call    draw_window
still:
        call    draw_magnify
wtevent:
        mcall   23, delay ; wait here for event with timeout
        dec     eax
        js      still
        jz      redraw
        dec     eax
        jnz     button
; key in buffer
        mov     eax, 2
        mcall
        cmp     ah, 32
        jnz     wtevent
        mov     eax, [mouse_x]
        mov     [pix1_x], eax
        mov     eax, [mouse_y]
        mov     [pix1_y], eax

        jmp	    wtevent
;---------------------------------------------------------------------
button:
; we have only one button, close

        or      eax, -1
        mcall
;------------------------- Window draw
draw_window:
        mcall   12, 1

        mov     al, 48    ; function 48 : graphics parameters
        mov     bl, 4     ; subfunction 4 : get skin height
        mcall
					; DRAW WINDOW
        mov     ebx, 100*65536 + 4*magnify_width + 9
        lea     ecx, [eax + 100*65536 + 4*magnify_height + 128]
        mov     edx, 0x34000000         ; color of work area RRGGBB
        mov     edi, labelt             ; header
        xor     eax, eax                ; function 0 : define and draw window
        mcall
        mcall   71, 1, labelt
        mcall   12,2
        ret
;------------------------- Magnify draw
draw_magnify:
        mcall   9, procinfo, -1
        mov     eax, [procinfo+70] ;status of window
        test    eax,100b
        jne     .end

        mcall   14	; get screen size
        movzx   ecx, ax
        inc     ecx
        mov     [screen_size_y], ecx
        shr     eax, 16
        inc     eax
        mov     [screen_size_x], eax

        xor     ebx, ebx
        mcall   37        ; get mouse coordinates
        mov     ecx, eax
        shr     ecx, 16   ; ecx = x
        movzx   edx, ax   ; edx = y
        mov     [mouse_x], ecx
        mov     [mouse_y], edx
        add     ecx, magnify_halfwidth
        add     edx, magnify_halfheight
        mov     [magnify_area_end_x], ecx
        mov     [magnify_area_end_y], edx
        sub     ecx, magnify_width
        sub     edx, magnify_height
        mov     [magnify_area_start_x], ecx
        mov     [magnify_area_start_y], edx
.loop_y:
.loop_x:
        xor     eax, eax  ; assume black color for invalid pixels
        test    ecx, ecx
        js      .nopix
        cmp     ecx, [screen_size_x]
        jge     .nopix
        test    edx, edx
        js      .nopix
        cmp     edx, [screen_size_y]
        jge     .nopix
        mov     ebx, edx
        sub     ebx, [magnify_area_start_y]
        shl     ebx, 16
        mov     bx, cx
        sub     ebx, [magnify_area_start_x]
        cmp     ebx, aim0
        jz      .nopix
        cmp     ebx, aim1
        jz      .nopix
        cmp     ebx, aim2
        jz      .nopix
        cmp     ebx, aim3
        jz      .nopix

        mov     ebx, edx
        imul    ebx, [screen_size_x]
        add     ebx, ecx
        mcall   35        ; read pixel

.nopix:
        push    ecx edx
        sub     ecx, [magnify_area_start_x]
        sub     edx, [magnify_area_start_y]
        mov     ebx, ecx
        shl     ebx, 2+16
        mov     bl, 4
        mov     ecx, edx
        shl     ecx, 2+16
        mov     cl, 4
        mov     edx, eax
        mcall   13        ; draw rectangle 8x8
        pop     edx ecx

        inc     ecx
        cmp     ecx, [magnify_area_end_x]
        jnz     .loop_x
        mov     ecx, [magnify_area_start_x]
        inc     edx
        cmp     edx, [magnify_area_end_y]
        jnz     .loop_y

;------------------------- Measure labels draw
        mov     eax, 4
        mov     ebx, 8*65536 + 124
        mov     ecx, 11110000111100001111000011110000b
        mov     edx, start_pix
        xor     edi, edi
        mcall   4
        add     ebx, 20
        mov     edx, end_pix
        mcall   4
        add     ebx, 20
        mov     edx, measure_x
        mcall   4
        add     ebx, 20
        mov     edx, measure_y
        mcall   4
        add     ebx, 20
        mov     edx, measure_d
        mcall   4
        add     ebx, 20
        mov     edx, inf
        mcall   4

        mov     ebx, 0x80040000
        mov     ecx, [mouse_x]
        mov     edx, 12*8*65536 + 144
        mov     esi, 0x50FFFFFF
        xor     edi, edi
        mcall   47
        sub     ecx, [pix1_x]
        jns     .sign1
        neg     ecx
.sign1:
        mov     edx, 14*8*65536 + 164
        mov     [dist_x], ecx
        mcall   47
        mov     ecx, [mouse_y]
        mov     edx, 18*8*65536 + 144
        mcall   47
        sub     ecx, [pix1_y]
        jns     .sign2
        neg     ecx
.sign2:
        mov     [dist_y], ecx
        mov     edx, 14*8*65536 + 184
        mcall   47
        mov     ecx, [pix1_y]
        mov     edx, 18*8*65536 + 124
        mcall   47
        mov     ecx, [pix1_x]
        mov     edx, 12*8*65536 + 124
        mcall   47

        mov     eax, [dist_x]
        mov     ebx, eax
        mul     bx
        mov     cx, dx
        shl     ecx, 16
        mov     cx, ax
        mov     eax, [dist_y]
        mov     ebx, eax
        mul     bx
        mov     si, dx
        shl     esi, 16
        mov     si, ax
        add     ecx, esi
        mov     [diag_l], ecx
        finit
        fild    [diag_l]
        fsqrt
        fistp   [diag_l]

        mov     ebx, 0x80040000
        mov     ecx, [diag_l]
        mov     edx, 12*8*65536 + 204
        mov     esi, 0x50FFFFFF
        xor     edi, edi
        mcall   47

.end:
        ret

;------------------------- Data area
        if      lang eq ru
labelt:
        db      3, 'Измеритель', 0
start_pix:
        db      'Пиксель 1 (    ,     )', 0
end_pix:
        db      'Пиксель 2 (    ,     )', 0
measure_x:
        db      'Дистанция x (    )', 0
measure_y:
        db      'Дистанция y (    )', 0
measure_d:
        db      'Диагональ (    )', 0
inf:
        db      'Нажмите пробел', 0

        else
labelt:
        db      3, 'Ruler', 0
start_pix:
        db      'Pixel 1   (    ,     )', 0
end_pix:
        db      'Pixel 2   (    ,     )', 0
measure_x:
        db      'Distance x  (    )', 0
measure_y:
        db      'Distance y  (    )', 0
measure_d:
        db      'Diagonal  (    )', 0
inf:
        db      'Press Space', 0

         end if
I_END:
        align   4
        magnify_area_start_x  dd ?
        magnify_area_start_y  dd ?
        magnify_area_end_x  dd ?
        magnify_area_end_y  dd ?
        screen_size_x   dd ?
        screen_size_y   dd ?
        mouse_x         dd ?
        mouse_y         dd ?
        pix1_x          dd 0
        pix1_y          dd 0
        dist_x          dd 0
        dist_y          dd 0
        diag_l          dd 0
;---------------------------------------------------------------------
procinfo:
        rb 1024
;---------------------------------------------------------------------