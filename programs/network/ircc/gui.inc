;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2020. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;   Written by hidnplayr@kolibrios.org                            ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


draw_window:    ; Completely redraw the window, recalculate all coordinates and sizes

        pusha

; get system colors
        mcall   48, 3, colors, 40
        call    set_edit_box_and_scrollbar_colors

        mcall   9, thread_info, -1                      ; get information about this thread
        mov     eax, [thread_info.box.width]            ; window xsize
        mov     ebx, [thread_info.box.height]           ; ysize
        mov     edx, [thread_info.client_box.width]     ; work area xsize
        mov     esi, [thread_info.client_box.height]    ; ysize
        sub     eax, edx
        sub     ebx, esi

        cmp     edx, WIN_MIN_X
        jae     .x_ok
        mov     edx, WIN_MIN_X
  .x_ok:
        mov     [xsize], edx
        add     edx, eax

        cmp     esi, WIN_MIN_Y
        jae     .y_ok
        mov     esi, WIN_MIN_Y
  .y_ok:
        mov     [ysize], esi
        add     esi, ebx
        mcall   67, -1, -1                              ; set the new sizes

        popa

  .dont_resize:

        pusha

        mcall   12, 1
        xor     eax, eax                                ; draw window
        mov     ebx, WIN_MIN_X+10
        mov     ecx, WIN_MIN_Y+30
        mov     edx, [colors.work]
        add     edx, 0x33000000
        mov     edi, str_programname
        mcall
                        
        test    [thread_info.wnd_state], 100b           ; skip if window is rolled up
        jne     .exit

        cmp     [window_active], 0
        je      .no_window

; calculate available space for textbox and coordinates for scrollbars
        mov     eax, [ysize]
        sub     eax, TOP_Y + INPUTBOX_HEIGHT - 1
        mov     [scroll2.y_size], ax
        mov     [scroll1.y_size], ax
        sub     eax, 4
        xor     edx, edx
        mov     ecx, FONT_HEIGHT
        div     ecx
        mov     [textbox_height], eax
        mov     [scroll2.cur_area], eax
        mov     [scroll1.cur_area], eax

        mov     eax, [xsize]
        sub     eax, SCROLLBAR_WIDTH
        mov     [scroll1.x_pos], ax
        mov     edi, [window_active]
        cmp     [edi + window.type], WINDOWTYPE_CHANNEL
        jne     @f
        sub     eax, USERLIST_WIDTH + SCROLLBAR_WIDTH + 2
  @@:
        mov     [scroll2.x_pos], ax
        sub     eax, 10
        xor     edx, edx
        mov     ecx, FONT_WIDTH
        div     ecx
        mov     [textbox_width], eax

; recalculate text line breaks (because height/width might have changed..)
; meanwhile, recalculate line number of current line
        mov     esi, [edi + window.text_print]
        mov     al, byte[esi]
        push    eax
        mov     byte[esi], 0
        push    esi

        mov     esi, [edi + window.text_start]
        call    text_insert_newlines
        mov     [edi + window.text_lines], edx
        mov     [edi + window.text_scanned], esi
        mov     [edi + window.text_line_print], edx

        pop     esi
        pop     eax
        mov     byte[esi], al

; and redraw the textbox (and scrollbar if needed)
        mov     [scroll2.all_redraw], 1
        call    draw_channel_text

; Draw userlist if active window is a channel
        mov     edi, [window_active]
        cmp     [edi + window.type], WINDOWTYPE_CHANNEL
        jne     .not_channel

        mov     [scroll1.all_redraw], 1
        call    draw_user_list

; draw a vertical separator line when there is no scrollbar
        cmp     [scroll2.all_redraw], 1
        jne     .not_channel

        mov     ebx, [xsize]
        sub     ebx, USERLIST_WIDTH + SCROLLBAR_WIDTH + 3
        push    bx
        shl     ebx, 16
        pop     bx
        mov     ecx, [ysize]
        add     ecx, TOP_Y shl 16 - (INPUTBOX_HEIGHT)
        mov     edx, [colors.work_graph]
        mcall   38
  .not_channel:
  .no_window:

; draw editbox
        mov     eax, [ysize]
        sub     eax, INPUTBOX_HEIGHT
        mov     [edit1.top], eax

        mov     eax, [xsize]
        mov     [edit1.width], eax

        invoke  edit_box_draw, edit1

; draw tabs
        call    draw_window_tabs
                
  .exit:
        mcall   12, 2
        popa
        ret



draw_user_list:

        pusha

        ; Do we need a scrollbar?
        mov     ebx, [window_active]
        mov     eax, [ebx + window.users]
        mov     [scroll1.max_area], eax
        cmp     [scroll1.cur_area], eax
        jae     .noscroll

        ; Is the current position greater then the max position?
        cmp     eax, [scroll1.position]
        ja      @f
        mov     [scroll1.position], eax
  @@:
        ; OK, draw the scrollbar
        invoke  scrollbar_draw, scroll1

        ; dont redraw scrollbar completely next time,
        ; unless draw_window asks us to by setting [scroll1.all_redraw] back to 1
        mov     [scroll1.all_redraw], 0
        jmp     .scroll_done

  .noscroll:
        mov     [scroll1.position], 0
  .scroll_done:

        ; draw an invisible button, where the usernames will go
        mov     ebx, [xsize]
        sub     ebx, USERLIST_WIDTH + SCROLLBAR_WIDTH
        shl     ebx, 16
        push    ebx
        mov     bx, USERLIST_WIDTH
        mov     ecx, [ysize]
        add     ecx, TEXT_Y shl 16 - (TEXT_Y + 16)
        push    ecx ebx
        mov     edx, WINDOW_BTN_LIST + 1 shl 29 + 1 shl 30
        mcall   8

        ; draw a filled rectangle to clear previously printed names
        pop     ebx ecx
        mov     edx, [colors.work]
        mcall   13

; now, draw the names according to the scrollbar position and window size
        mov     eax, [scroll1.position]
        xor     edx, edx
        mov     ecx, MAX_NICK_LEN
        mul     ecx
        mov     edx, eax
        mov     eax, [window_active]
        mov     ebp, [eax + window.selected]
        add     edx, [eax + window.data_ptr]
        sub     ebp, [scroll1.position]
        add     edx, window_data.names

        pop     ebx
        mov     bx, TEXT_Y
        mov     ecx, [colors.work_text]
        or      ecx, 0x90000000                 ; 8x16 font, zero terminated string
        mov     eax, 4                          ; draw text

        mov     edi, [textbox_height]           ; how many names will fit on screen
  .loop:
        cmp     byte[edx], 0                    ; end of list?
        je      .done

        dec     ebp                             ; is this name selected?
        jnz     .nothighlight
                                                ; yes, highlight it
        pusha
        mov     cx, bx
        mov     bx, USERLIST_WIDTH
        shl     ecx, 16
        mov     cx, FONT_HEIGHT
        mov     edx, 0x00000055                 ; blue!
        mcall   13
        popa

        mov     ecx, 0x9000ffff                 ; cyan!
        mcall

        mov     ecx, [colors.work_text]
        or      ecx, 0x90000000                 ; 8x16 font, zero terminated string
        jmp     .next

  .nothighlight:
        mcall

  .next:
        add     edx, MAX_NICK_LEN
        add     ebx, FONT_HEIGHT
        dec     edi
        jnz     .loop

  .done:
        popa

        ret


draw_window_tabs:

; Draw horizontal line

        mov     ebx, [xsize]
        mov     edx, [colors.work_graph]
        mov     ecx, TOP_Y SHL 16 + TOP_Y
        mcall   38

; Create the buttons

        mov     eax, 8
        mov     ebx, TAB_WIDTH
        mov     ecx, TOP_SPACE shl 16 + TAB_HEIGHT
        mov     edx, WINDOW_BTN_START
        mov     edi, windows
  .more_btn:
        mov     esi, [colors.work]
        cmp     [window_active], edi
        jne     @f
        ;not     esi
        ;and     esi, 0x00ffffff
        mov      esi,  [colors.work_light]  
      @@:
        mcall
        inc     edx
        add     ebx, (TAB_WIDTH + TAB_SPACE) shl 16
        add     edi, sizeof.window
        cmp     [edi + window.data_ptr], 0
        jne     .more_btn

; Draw the close window button
        mov     edi, [window_active]
        cmp     [edi + window.type], WINDOWTYPE_SERVER  ; dont let the user close server window
        je      @f

;        mov     eax, 8
        mov     ebx, [xsize]
        sub     ebx, SCROLLBAR_WIDTH
        shl     ebx, 16
        mov     bx, SCROLLBAR_WIDTH
        mov     ecx, TOP_SPACE shl 16 + TAB_HEIGHT - 1
        mov     edx, WINDOW_BTN_CLOSE
        mov     esi, 0x00aa0000         ; red !
        mcall

        pusha
        ; write closing cross
        mov     ebx, [xsize]
        sub     ebx, 9
        shl     ebx, 16
        add     ebx, TOP_SPACE+3
        mov     ecx, 0x80FFFfff
        mov     edx, closing_cross
        mcall   4
        popa
  @@:

; Draw the windownames onto the buttons

        mov     eax, 4
        mov     ebx, 5 shl 16 + TOP_SPACE + 4 ;;;;
        mov     esi, MAX_WINDOWS
        mov     edi, windows
  .more:
        mov     ecx, [colors.work_text]
        test    [edi + window.flags], FLAG_UPDATED
        jz      @f
        mov     ecx, 0x00aa0000         ; RED!
  @@:
        or      ecx, 0x80000000         ; ASCIIZ string
        lea     edx, [edi + window.name]
        mcall
        add     edi, sizeof.window      ; get ptr to next window
        cmp     [edi + window.data_ptr], 0
        je      .enough
        add     ebx, (TAB_WIDTH + TAB_SPACE) shl 16
        dec     esi
        jnz     .more
  .enough:

        ret



highlight_updated_tabs:
        mov     eax, 4
        mov     ebx, 5 shl 16 + TOP_SPACE + 4 ;;;;
        mov     ecx, 0x80aa0000
        mov     esi, MAX_WINDOWS
        mov     edi, windows
  .more_:
        test    [edi + window.flags], FLAG_UPDATED
        jz      .next
        lea     edx, [edi + window.name]
        mcall
  .next:
        add     edi, sizeof.window      ; get ptr to next window
        cmp     [edi + window.data_ptr], 0
        je      .enough_
        add     ebx, (TAB_WIDTH + TAB_SPACE) shl 16
        dec     esi
        jnz     .more_
  .enough_:

        ret
		
set_edit_box_and_scrollbar_colors:
        mov     eax, [colors.work]
        mov     [scroll1.bg_color], eax
        mov     [scroll2.bg_color], eax

        mov     eax, [colors.work_button]
        mov     [scroll1.front_color], eax
        mov     [scroll2.front_color], eax

        mov     eax, [colors.work_text]
        mov     [scroll1.line_color], eax
        mov     [scroll2.line_color], eax

        mov     [scroll1.type], 0               ; 0 = simple, 1 = skinned
        mov     [scroll2.type], 0

        ret