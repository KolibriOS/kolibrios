; The window: its colors, size and place, the panel and the tabs, then the
; panel of the open tab.

draw_window:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors

        mov     eax, [sc.work_button]   ; its channels high to low: al, ah, dl
        mov     edx, eax
        shr     edx, 16
        cmp     al, ah
        jae     @f
        xchg    al, ah
@@:
        cmp     al, dl
        jae     @f
        xchg    al, dl
@@:
        cmp     ah, dl
        jae     @f
        xchg    ah, dl

@@:
        movzx   ecx, dl                 ; blue, green, red
        shl     ecx, 16
        mov     cx, ax
        mov     [sc.work_button], ecx

        mov     dl, al                  ; the low ones half the high one
        shr     dl, 1
        movzx   ecx, dl                 ; all low, then green, red high
        imul    ecx, 0x10101
        mov     [green], ecx
        mov     byte [green + 1], al
        mov     [red], ecx
        mov     byte [red + 2], al

        mov     eax, [sc.work]          ; between the window and the panel
        mov     edx, [sc.work_light]
        call    mix
        mov     [mid], eax

        mcall   SF_GET_SCREEN_SIZE      ; half the screen, the kernel takes
        shr     eax, 1                  ; it the first time
        xchg    esi, eax
        mcall   SF_STYLE_SETTINGS, SSF_GET_SCREEN_AREA
        mov     edx, esi
        shr     edx, 16
        mov     ebp, MIN_W - 1
        call    .fit
        xchg    eax, ebx
        movzx   edx, si
        and     dh, 0x7F
        mov     ebp, MIN_H - 1
        call    .fit
        xchg    eax, ecx

        mov     edx, [sc.work]
        or      edx, WIN_STYLE
        mcall   SF_CREATE_WINDOW, , , , 0, sz_title

        mcall   SF_THREAD_INFO, pinfo, -1
        test    [pinfo.wnd_state], WND_ROLLED_UP
        jnz     .done
        test    [pinfo.wnd_state], WND_MAXIMIZED        ; the size it gets
        jnz     .layout

        mov     edx, [pinfo.box.width]  ; not smaller than the minimum
        mov     esi, [pinfo.box.height]
        cmp     edx, MIN_W - 1          ; the kernel keeps the size - 1
        jae     @f
        mov     edx, MIN_W - 1
@@:
        cmp     esi, MIN_H - 1
        jae     @f
        mov     esi, MIN_H - 1
@@:
        cmp     edx, [pinfo.box.width]
        jne     @f
        cmp     esi, [pinfo.box.height]
        je      .layout

@@:
        mcall   SF_CHANGE_WINDOW, -1, -1 ; it sends a new redraw
        jmp     .done

.layout:
        mov     eax, [pinfo.client_box.width]
        lea     ecx, [eax - PAD*2]
        mov     [area_w], ecx
        mov     [bar_right], eax
        mov     [bar_left], PAD

        mov     eax, [pinfo.client_box.height]
        sub     eax, PAD + BTN_H
        mov     [bar_y], eax
        sub     eax, AREA_Y + PAD
        mov     [area_h], eax

        mov     ebx, PAD shl 16
        mov     bx, word [area_w]
        mov     ecx, AREA_Y shl 16
        mov     cx, ax
        mov     edx, [sc.work_light]
        call    frame_rect

        mov     edx, [sc.work]          ; around the tabs and the panel: above,
        mov     ebx, [area_w]           ; below with the bar, left, right
        add     ebx, 2*PAD + 1
        mcall   SF_DRAW_RECT, , TOP

        mov     ecx, [bar_y]
        sub     ecx, PAD
        shl     ecx, 16
        mov     cx, 2*PAD + BTN_H + 1
        mcall   SF_DRAW_RECT

        mov     ecx, [area_h]
        add     ecx, (TOP shl 16) + TAB_H - 1
        mov     ebx, PAD
        mcall   SF_DRAW_RECT

        mov     ebx, [area_w]
        add     ebx, PAD
        shl     ebx, 16
        mov     bx, PAD + 1
        mcall   SF_DRAW_RECT

        mov     esi, [area_w]           ; the tabs as wide as the panel, the
        add     esi, NTABS - 1          ; room left shared alike
        xor     ebp, ebp
@@:
        call    .content
        sub     esi, eax
        inc     ebp
        cmp     ebp, NTABS
        jb      @b

        xchg    eax, esi
        xor     edx, edx
        mov     ecx, NTABS
        div     ecx
        xchg    eax, esi

        mov     ebx, PAD shl 16
        xor     ebp, ebp

.tab:
        call    .content
        add     eax, esi
        cmp     ebp, NTABS - 1          ; the last one to the end of the panel
        jne     @f
        mov     eax, [area_w]
        add     eax, PAD
        mov     edx, ebx
        shr     edx, 16
        sub     eax, edx

@@:
        mov     bx, ax
        call    .draw
        imul    eax, ebx, 1 shl 16
        lea     ebx, [ebx + eax - 1 shl 16]     ; neighbours share the border
        inc     ebp
        cmp     ebp, NTABS
        jb      .tab

        mov     eax, sz_shutdown        ; at the right end
        mov     edx, BTN_SHUTDOWN
        mov     esi, [sc.work_graph]
        call    bar_button

        imul    esi, [cur_tab], sizeof.TAB      ; the bar of the tab, then its
        movzx   esi, word [tabs + esi + TAB.bar]        ; panel
        call    bar_draw
        mov     eax, TAB.draw
        call    tab_call

.done:
        mcall   SF_REDRAW, SSF_END_DRAW
        ret

; eax = start shl 16 + end of the work area, edx = the size - 1 wanted,
; ebp = the least; out: eax = start shl 16 + size - 1 of the window in its
; middle
.fit:
        cmp     edx, ebp
        jae     @f
        mov     edx, ebp

@@:
        movzx   ecx, ax
        shr     eax, 16
        sub     ecx, eax
        cmp     edx, ecx
        jb      @f
        mov     edx, ecx

@@:
        sub     ecx, edx
        shr     ecx, 1
        add     eax, ecx
        shl     eax, 16
        mov     ax, dx
        ret

.draw:                                  ; the caption in the middle
        pusha
        mov     ecx, (TOP shl 16) + TAB_H
        mov     esi, [sc.work_dark]
        cmp     ebp, [cur_tab]
        jne     @f
        mov     esi, [sc.work_light]

@@:
        mov     edx, esi
        call    frame_rect

        lea     edx, [ebp + BTN_TAB + BT_HIDE + BT_NOFRAME]
        mcall   SF_DEFINE_BUTTON

        call    .content
        movzx   ecx, bx
        sub     ecx, eax
        shr     ecx, 1
        shl     ecx, 16
        add     ebx, ecx

        mov     ecx, TOP shl 16
        mov     eax, [sc.work_text]
        jmp     bar_text                ; as high as the bar items

; ebp = tab; out: edi -> its caption, eax = its width
.content:
        imul    edi, ebp, sizeof.TAB
        movzx   edi, word [tabs + edi + TAB.caption]
        mov     eax, edi
        call    text_width
        ret

