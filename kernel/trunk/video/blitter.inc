;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2011-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


struct  BLITTER_BLOCK
        xmin            dd ?
        ymin            dd ?
        xmax            dd ?
        ymax            dd ?
ends


struct  BLITTER
        dc              RECT
        sc              RECT
        dst_x           dd ?            ;  32
        dst_y           dd ?            ;  36
        src_x           dd ?            ;  40
        src_y           dd ?            ;  44
        w               dd ?            ;  48
        h               dd ?            ;  52

        bitmap          dd ?            ;  56
        stride          dd ?            ;  60
ends



align 4
block_clip:
;esi= clip RECT ptr
;edi= RECT ptr
;return code:
;CF= 0 - draw, 1 - don't draw

        push    ebx

        mov     eax, [edi + RECT.left]
        mov     ebx, [edi + RECT.right]
        mov     ecx, [esi + RECT.left]    ;clip.left
        mov     edx, [esi + RECT.right]   ;clip.right

        cmp     eax, edx                ;left >= clip.right
        jge     .fail

        cmp     ebx, ecx                ;right < clip.left
        jl      .fail

        cmp     eax, ecx                ;left >= clip.left
        jge     @F

        mov     [edi + RECT.left], ecx
@@:
        cmp     ebx, edx                ;right <= clip.right
        jle     @f

        mov     [edi + RECT.right], edx
@@:
        mov     eax, [edi + RECT.top]
        mov     ebx, [edi + RECT.bottom]
        mov     ecx, [esi + RECT.top]     ;clip.top
        mov     edx, [esi + RECT.bottom]  ;clip.bottom

        cmp     eax, edx                ;top >= clip.bottom
        jge     .fail

        cmp     ebx, ecx                ;bottom < clip.top
        jl      .fail

        cmp     eax, ecx                ;top >= clip.top
        jge     @F

        mov     [edi + RECT.top], ecx
@@:
        cmp     ebx, edx                ;bottom <= clip.bottom
        jle     @f

        mov     [edi + RECT.bottom], edx
@@:
        pop     ebx
        clc
        ret
.fail:
        pop     ebx
        stc
        ret


align 4
blit_clip:

;return code:
;CF= 0 - draw, 1 - don't draw

.sx0   = 8
.sy0   = 12
.sx1   = 16
.sy1   = 20

.dx0   = 24
.dy0   = 28
.dx1   = 32
.dy1   = 36


        push    edi
        push    esi
        push    ebx
        sub     esp, 40

        mov     ebx, ecx
        mov     edx, [ecx + BLITTER.src_x]
        mov     [esp+.sx0], edx
        mov     eax, [ecx + BLITTER.src_y]
        mov     [esp+.sy0], eax
        add     edx, [ecx + BLITTER.w]
        add     eax, [ecx + BLITTER.h]
        mov     [esp + .sx1], edx
        mov     [esp + .sy1], eax

        lea     edi, [esp + .sx0]
        lea     esi, [ebx + BLITTER.sc]

        call    block_clip
        jc      .done

        mov     edi, [esp + .sx0]
        mov     edx, [ebx + BLITTER.dst_x]
        add     edx, edi
        sub     edx, [ebx + BLITTER.src_x]
        mov     [esp + .dx0], edx

        mov     ecx, [esp+.sy0]
        mov     eax, [ebx + BLITTER.dst_y]
        add     eax, ecx
        sub     eax, [ebx + BLITTER.src_y]
        mov     [esp + .dy0], eax

        sub     edx, edi
        add     edx, [esp + .sx1]
        mov     [esp + .dx1], edx

        sub     eax, ecx
        add     eax, [esp + .sy1]
        mov     [esp + .dy1], eax

        lea     edi, [esp + .dx0]
        lea     esi, [ebx + BLITTER.dc]
        call    block_clip
        jc      .done

        mov     edx, [esp + .dx0]
        mov     eax, [esp + .dx1]
        sub     eax, edx
        mov     [ebx + BLITTER.w], eax

        mov     eax, [esp + .dy0]
        mov     ecx, [esp + .dy1]
        sub     ecx, eax
        mov     [ebx + BLITTER.h], ecx

        mov     ecx, [ebx + BLITTER.src_x]
        add     ecx, edx
        sub     ecx, [ebx + BLITTER.dst_x]
        mov     [ebx + BLITTER.src_x], ecx

        mov     ecx, [ebx + BLITTER.src_y]
        add     ecx, eax
        sub     ecx, [ebx + BLITTER.dst_y]
        mov     [ebx + BLITTER.src_y], ecx
        mov     [ebx + BLITTER.dst_x], edx
        mov     [ebx + BLITTER.dst_y], eax
        clc
.done:
        add     esp, 40
        pop     ebx
        pop     esi
        pop     edi


purge .sx0
purge .sy0
purge .sx1
purge .sy1

purge .dx0
purge .dy0
purge .dx1
purge .dy1

        ret

align 4
blit_32:
        push    ebp
        push    edi
        push    esi
        push    ebx
virtual at sizeof.BLITTER
.position       dd      ? ; (x shl 16) + y
; ???
.extra_var1     dd      ?
.flags          dd      ?
.local_vars_size = $
end virtual
        sub     esp, .local_vars_size

        mov     [esp + .flags], ebx

        mov     eax, [current_slot]
        mov     eax, [eax + APPDATA.window]
        mov     ebx, [eax + WDATA.box.width]
        mov     edx, [eax + WDATA.box.height]
        inc     ebx
        inc     edx

        xor     eax, eax

        mov     [esp + BLITTER.dc.left], eax
        mov     [esp + BLITTER.dc.top], eax
        mov     [esp + BLITTER.dc.right], ebx
        mov     [esp + BLITTER.dc.bottom], edx

        mov     [esp + BLITTER.sc.left], eax
        mov     [esp + BLITTER.sc.top], eax
        mov     eax, [ecx+24]

        mov     [esp + BLITTER.sc.right], eax
        mov     eax, [ecx+28]

        mov     [esp + BLITTER.sc.bottom], eax

        mov     eax, [ecx]
        mov     [esp + BLITTER.dst_x], eax
        mov     eax, [ecx+4]
        mov     [esp + BLITTER.dst_y], eax

        mov     eax, [ecx+16]
        mov     [esp + BLITTER.src_x], eax
        mov     eax, [ecx+20]
        mov     [esp + BLITTER.src_y], eax
        mov     eax, [ecx+8]
        mov     [esp + BLITTER.w], eax
        mov     eax, [ecx+12]
        mov     [esp + BLITTER.h], eax


        mov     eax, [ecx+32]
        mov     [esp + BLITTER.bitmap], eax
        mov     eax, [ecx+36]
        mov     [esp + BLITTER.stride], eax

        mov     ecx, esp
        call    blit_clip
        jc      .L57

        mov     eax, [current_slot]
        mov     eax, [eax + APPDATA.window]

        mov     ebx, [esp + BLITTER.dst_x]
        mov     ebp, [esp + BLITTER.dst_y]
        add     ebx, [eax + WDATA.box.left]
        add     ebp, [eax + WDATA.box.top]

        test    [esp + .flags], BLIT_CLIENT_RELATIVE
        jz      .no_client_relative

        add     ebx, [eax + WDATA.clientbox.left]
        add     ebp, [eax + WDATA.clientbox.top]
.no_client_relative:

        mov     ecx, ebx
        add     ecx, [esp + BLITTER.w]
        shl     ecx, 16
        mov     cx, bp
        add     ecx, [esp + BLITTER.h]

        mov     eax, ebx
        shl     eax, 16
        mov     ax, bp
        mov     [esp + .position], eax

        mov     edi, ebp

;        imul    edi, [_display.pitch]
        mov     edi, [BPSLine_calc_area+edi*4]
;        imul    ebp, [_display.width]
        mov     ebp, [d_width_calc_area+ebp*4]

        add     ebp, ebx
        add     ebp, [_display.win_map]

        mov     eax, [esp + BLITTER.src_y]
        imul    eax, [esp + BLITTER.stride]
        mov     esi, [esp + BLITTER.src_x]
        lea     esi, [eax + esi*4]
        add     esi, [esp + BLITTER.bitmap]

        mov     eax, ecx
        mov     ecx, [esp + BLITTER.h]
        mov     edx, [esp + BLITTER.w]

        test    ecx, ecx    ;FIXME check clipping
        jz      .L57

        test    edx, edx
        jz      .L57

        cmp     [_display.bits_per_pixel], 32
        jne     .core_24

        lea     edi, [edi + ebx*4]

        mov     ebx, [current_slot_idx]
; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     .core_32.hardware_cursor
;--------------------------------------
.core_32.software_cursor:
align 4
.outer32:

align 4
.inner32:
        cmp     [ebp], bl
        jne     .skip
;--------------------------------------
        mov     eax, [esi]

        mov     ecx, [esp + .position]

; check mouse area for putpixel
        call    [_display.check_mouse]
;--------------------------------------
; store to real LFB
        mov     [LFB_BASE + edi], eax
;--------------------------------------
align 4
.skip:
        add     esi, 4
        add     edi, 4
        inc     ebp
        add     [esp + .position], 1 shl 16
        dec     edx
        jnz     .inner32

        add     esi, [esp + BLITTER.stride]
        add     edi, [_display.lfb_pitch]
        add     ebp, [_display.width]

        mov     edx, [esp + BLITTER.w]
        mov     eax, edx
        inc     [esp+.position]
        sub     ebp, edx
        shl     eax, 2
        sub     esi, eax
        sub     edi, eax
        shl     eax, 16-2
        sub     [esp + .position], eax
        dec     [esp + BLITTER.h]
        jnz     .outer32
        jmp     .done
.core_32.hardware_cursor:
align 4
.hw.outer32:
        xor     ecx, ecx

align 4
.hw.inner32:
        cmp     [ebp + ecx], bl
        jne     .hw.skip
        mov     eax, [esi + ecx*4]
        mov     [LFB_BASE + edi + ecx*4], eax

align 4
.hw.skip:
        inc     ecx
        dec     edx
        jnz     .hw.inner32

        add     esi, [esp + BLITTER.stride]
        add     edi, [_display.lfb_pitch]
        add     ebp, [_display.width]

        mov     edx, [esp + BLITTER.w]
        dec     [esp + BLITTER.h]
        jnz     .hw.outer32

.done:
;        call    [draw_pointer]
;        call    __sys_draw_pointer
.L57:
        add     esp, .local_vars_size
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret

.core_24:
        cmp     [_display.bits_per_pixel], 24
        jne     .core_16

        lea     ebx, [ebx + ebx*2]
        lea     edi, [LFB_BASE + edi + ebx]
        mov     ebx, [current_slot_idx]

align 4
.outer24:
        mov     [esp + .extra_var1], edi
        xor     ecx, ecx

align 4
.inner24:
        cmp     [ebp + ecx], bl           ; Does the process own this pixel?
        jne     .skip_1
;--------------------------------------
        push    eax
        mov     eax, [esi + ecx*4]

        lea     edi, [edi + ecx*2]

; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     .no_mouseunder_1
;--------------------------------------
align 4
        push    ecx

        mov     ecx, [esp+4]
        ror     ecx, 16
        sub     ecx, edx
        rol     ecx, 16
        sub     ecx, [esp + BLITTER.h + 8]

; check mouse area for putpixel
        call    [_display.check_mouse]
        pop     ecx
;--------------------------------------
align 4
.no_mouseunder_1:
        mov     [edi + ecx], ax
        shr     eax, 16
        mov     [edi + ecx+2], al

        pop     eax
;--------------------------------------
align 4
.skip_1:
        mov     edi, [esp + .extra_var1]
        inc     ecx
        dec     edx
        jnz     .inner24

        add     esi, [esp + BLITTER.stride]
        add     edi, [_display.lfb_pitch]
        add     ebp, [_display.width]

        mov     edx, [esp + BLITTER.w]
        dec     [esp + BLITTER.h]
        jnz     .outer24

        jmp     .done


.core_16:
        lea     edi, [LFB_BASE + edi + ebx*2]
        mov     ebx, [current_slot_idx]

  .outer16:
        mov     [esp + .extra_var1], edi
        xor     ecx, ecx

  .inner16:
        cmp     [ebp + ecx], bl                   ; Does the process own this pixel?
        jne     .skip_2
;--------------------------------------
        push    eax
        mov     eax, [esi + ecx*4]

; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        jne     .no_mouseunder_2
;--------------------------------------
        push    ecx

        mov     ecx, [esp+4]
        ror     ecx, 16
        sub     ecx, edx
        rol     ecx, 16
        sub     ecx, [esp + BLITTER.h + 8]

; check mouse area for putpixel
        call    [_display.check_mouse]
        pop     ecx
;--------------------------------------
  .no_mouseunder_2:
; convert to 16 bpp and store to LFB
        and     eax, 00000000111110001111110011111000b
        shr     ah, 2
        shr     ax, 3
        ror     eax, 8
        add     al, ah
        rol     eax, 8
        mov     [edi + ecx*2], ax
        pop     eax
;--------------------------------------
  .skip_2:
        mov     edi, [esp + .extra_var1]
        inc     ecx
        dec     edx
        jnz     .inner16

        add     esi, [esp + BLITTER.stride]
        add     edi, [_display.lfb_pitch]
        add     ebp, [_display.width]

        mov     edx, [esp + BLITTER.w]
        dec     [esp + BLITTER.h]
        jnz     .outer16

        jmp     .done

