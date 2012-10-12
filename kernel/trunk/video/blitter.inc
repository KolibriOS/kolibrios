;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2011-2012. All rights reserved. ;;
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
;eax= 0 - draw, 1 - don't draw

        push    ebx

        mov     eax, [edi+RECT.left]
        mov     ebx, [edi+RECT.right]
        mov     ecx, [esi+RECT.left]    ;clip.left
        mov     edx, [esi+RECT.right]   ;clip.right

        cmp     eax, edx                ;left >= clip.right
        jge     .fail

        cmp     ebx, ecx                ;right < clip.left
        jl      .fail

        cmp     eax, ecx                ;left >= clip.left
        jae     @F

        mov     eax, ecx
@@:
        mov     [edi+RECT.left], eax

        cmp     ebx, edx                ;right <= clip.right
        jle     @f
        mov     ebx, edx
@@:
        mov     [edi+RECT.right], ebx

        mov     eax, [edi+RECT.top]
        mov     ebx, [edi+RECT.bottom]
        mov     ecx, [esi+RECT.top]     ;clip.top
        mov     edx, [esi+RECT.bottom]  ;clip.bottom

        cmp     eax, edx                ;top >= clip.bottom
        jge     .fail

        cmp     ebx, ecx                ;bottom < clip.top
        jl      .fail

        cmp     eax, ecx                ;top >= clip.top
        jae     @F

        mov     eax, ecx
@@:
        mov     [edi+RECT.top], eax

        cmp     ebx, edx                ;bottom <= clip.bottom
        jle     @f
        mov     ebx, edx
@@:
        mov     [edi+RECT.bottom], ebx
        pop     ebx
        xor     eax, eax
        ret
.fail:
        pop     ebx
        mov     eax, 1
        ret


align 4
blit_clip:

.sx0   equ 8
.sy0   equ 12
.sx1   equ 16
.sy1   equ 20

.dx0   equ 24
.dy0   equ 28
.dx1   equ 32
.dy1   equ 36


        push    edi
        push    esi
        push    ebx
        sub     esp, 40

        mov     ebx, ecx
        mov     edx, [ecx+BLITTER.src_x]
        mov     [esp+.sx0], edx
        mov     eax, [ecx+BLITTER.src_y]
        mov     [esp+.sy0], eax
        add     edx, [ecx+BLITTER.w]
        add     eax, [ecx+BLITTER.h]
        mov     [esp+.sx1], edx
        mov     [esp+.sy1], eax

        lea     edi, [esp+.sx0]
        lea     esi, [ebx+BLITTER.sc]

        call    block_clip
        test    eax, eax
        mov     esi, 1
        jnz     .done

        mov     edi, [esp+.sx0]
        mov     edx, [ebx+BLITTER.dst_x]
        add     edx, edi
        sub     edx, [ebx+BLITTER.src_x]
        mov     [esp+.dx0], edx

        mov     ecx, [esp+.sy0]
        mov     eax, [ebx+BLITTER.dst_y]
        add     eax, ecx
        sub     eax, [ebx+BLITTER.src_y]
        mov     [esp+.dy0], eax

        sub     edx, edi
        add     edx, [esp+.sx1]
        mov     [esp+.dx1], edx

        sub     eax, ecx
        add     eax, [esp+.sy1]
        mov     [esp+.dy1], eax

        lea     edi, [esp+.dx0]
        lea     esi, [ebx+BLITTER.dc]
        call    block_clip
        test    eax, eax
        mov     esi, 1
        jnz     .done

        mov     edx, [esp+.dx0]
        mov     eax, [esp+.dx1]
        sub     eax, edx
        mov     [ebx+BLITTER.w], eax

        mov     eax, [esp+.dy0]
        mov     ecx, [esp+.dy1]
        sub     ecx, eax
        mov     [ebx+BLITTER.h], ecx

        mov     ecx, [ebx+BLITTER.src_x]
        add     ecx, edx
        sub     ecx, [ebx+BLITTER.dst_x]
        mov     [ebx+BLITTER.src_x], ecx

        mov     ecx, [ebx+BLITTER.src_y]
        add     ecx, eax
        sub     ecx, [ebx+BLITTER.dst_y]
        mov     [ebx+BLITTER.src_y], ecx
        mov     [ebx+BLITTER.dst_x], edx
        mov     [ebx+BLITTER.dst_y], eax
        xor     esi, esi
.done:
        mov     eax, esi
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
        sub     esp, 72

        mov     eax, [TASK_BASE]
        mov     ebx, [eax-twdw + WDATA.box.width]
        mov     edx, [eax-twdw + WDATA.box.height]
        inc     ebx
        inc     edx

        xor     eax, eax

        mov     [esp+BLITTER.dc.left], eax
        mov     [esp+BLITTER.dc.top], eax
        mov     [esp+BLITTER.dc.right], ebx
        mov     [esp+BLITTER.dc.bottom], edx

        mov     [esp+BLITTER.sc.left], eax
        mov     [esp+BLITTER.sc.top], eax
        mov     eax, [ecx+24]

        mov     [esp+BLITTER.sc.right], eax
        mov     eax, [ecx+28]

        mov     [esp+BLITTER.sc.bottom], eax

        mov     eax, [ecx]
        mov     [esp+BLITTER.dst_x], eax
        mov     eax, [ecx+4]
        mov     [esp+BLITTER.dst_y], eax

        mov     eax, [ecx+16]
        mov     [esp+BLITTER.src_x], eax
        mov     eax, [ecx+20]
        mov     [esp+BLITTER.src_y], eax
        mov     eax, [ecx+8]
        mov     [esp+BLITTER.w], eax
        mov     eax, [ecx+12]
        mov     [esp+BLITTER.h], eax


        mov     eax, [ecx+32]
        mov     [esp+56], eax
        mov     eax, [ecx+36]
        mov     [esp+60], eax

        mov     ecx, esp
        call    blit_clip
        test    eax, eax
        jne     .L57

        mov     eax, [TASK_BASE]

        mov     ebx, [esp+BLITTER.dst_x]
        mov     ebp, [esp+BLITTER.dst_y]
        add     ebx, [eax-twdw + WDATA.box.left]
        add     ebp, [eax-twdw + WDATA.box.top]

        mov     ecx, ebx
        add     ecx, [esp+BLITTER.w]
        shl     ecx, 16
        mov     cx, bp
        add     ecx, [esp+BLITTER.h]

        mov     edi, ebp

;        imul    edi, [_display.pitch]
        mov     edi, [BPSLine_calc_area+edi*4]
;        imul    ebp, [_display.width]
        mov     ebp, [d_width_calc_area+ebp*4]

        add     ebp, ebx
        add     ebp, [_WinMapAddress]

        mov     eax, [esp+BLITTER.src_y]
        imul    eax, [esp+BLITTER.stride]
        mov     esi, [esp+BLITTER.src_x]
        lea     esi, [eax+esi*4]
        add     esi, [esp+BLITTER.bitmap]

        mov     eax, ecx
        mov     ecx, [esp+BLITTER.h]
        mov     edx, [esp+BLITTER.w]

        test    ecx, ecx    ;FIXME check clipping
        jz      .L57

        test    edx, edx
        jz      .L57

        cmp     [_display.bpp], 32
        jne     .core_24

        lea     edi, [edi+ebx*4]

        mov     ebx, [CURRENT_TASK]
align 4
.outer32:
        xor     ecx, ecx

align 4
.inner32:
        cmp     [ebp+ecx], bl
        jne     .skip
;--------------------------------------
        push    eax
        mov     eax, [esi+ecx*4]

; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        je      @f
        cmp     [_display.select_cursor], 0
        jne     .no_mouseunder
;--------------------------------------
align 4
@@:
        push    ecx

        mov     ecx, [esp+4]
        ror     ecx, 16
        sub     ecx, edx
        rol     ecx, 16
        sub     ecx, [esp+BLITTER.h + 8]

; check mouse area for putpixel
        call    [_display.check_mouse]
        pop     ecx
;--------------------------------------
align 4
.no_mouseunder:
; store to real LFB
        mov     [LFB_BASE+edi+ecx*4], eax
        pop     eax
;--------------------------------------
align 4
.skip:
        inc     ecx
        dec     edx
        jnz     .inner32

        add     esi, [esp+BLITTER.stride]
        add     edi, [_display.pitch]
        add     ebp, [_display.width]

        mov     edx, [esp+BLITTER.w]
        dec     [esp+BLITTER.h]
        jnz     .outer32

.done:
;        call    [draw_pointer]
;        call    __sys_draw_pointer
.L57:
        add     esp, 72
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret

.core_24:
        lea     ebx, [ebx+ebx*2]
        lea     edi, [LFB_BASE+edi+ebx]
        mov     ebx, [CURRENT_TASK]

align 4
.outer24:
        mov     [esp+64], edi
        xor     ecx, ecx

align 4
.inner24:
        cmp     [ebp+ecx], bl
        jne     .skip_1
;--------------------------------------
        push    eax
        mov     eax, [esi+ecx*4]

        lea     edi, [edi+ecx*2]

; check for hardware cursor
        cmp     [_display.select_cursor], select_cursor
        je      @f
        cmp     [_display.select_cursor], 0
        jne     .no_mouseunder_1
;--------------------------------------
align 4
@@:
        push    ecx

        mov     ecx, [esp+4]
        ror     ecx, 16
        sub     ecx, edx
        rol     ecx, 16
        sub     ecx, [esp+BLITTER.h + 8]

; check mouse area for putpixel
        call    [_display.check_mouse]
        pop     ecx
;--------------------------------------
align 4
.no_mouseunder_1:
        mov     [edi+ecx], ax
        shr     eax, 16
        mov     [edi+ecx+2], al

        pop     eax
;--------------------------------------
align 4
.skip_1:
        mov     edi, [esp+64]
        inc     ecx
        dec     edx
        jnz     .inner24

        add     esi, [esp+BLITTER.stride]
        add     edi, [_display.pitch]
        add     ebp, [_display.width]

        mov     edx, [esp+BLITTER.w]
        dec     [esp+BLITTER.h]
        jnz     .outer24

        jmp     .done
