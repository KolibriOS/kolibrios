;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;                                                                 ;;
;;         GNU GENERAL PUBLIC LICENSE                              ;;
;;          Version 2, June 1991                                   ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


draw_window:

        pusha

        mcall   9, thread_info, -1              ; get current window size
        mov     eax, dword[thread_info+42]      ; window xsize
        mov     ebx, dword[thread_info+46]      ; ysize
        mov     edx, dword[thread_info+62]      ; work area xsize
        mov     esi, dword[thread_info+66]      ; ysize
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
        mcall   67, -1, -1                      ; set the new sizes

        mcall   12, 1
        xor     eax, eax                        ; draw window
        mov     ebx, WIN_MIN_X
        mov     ecx, WIN_MIN_Y
        mov     edx, [colors.work]
        add     edx, 0x33000000
        mov     edi, str_programname
        mcall
        mcall   12, 2 ;; when do we actually need this??

        mov     ebx, [xsize]
        mov     ecx, [ysize]
        sub     cx, 15 ;;;;
        push    cx
        shl     ecx, 16
        pop     cx
        mov     edx, [colors.work_graph]
        mcall   38                              ; draw line

        mov     ecx, TOP_Y SHL 16 + TOP_Y
        mcall

        mov     edi, [window_active]
        cmp     [edi + window.type], WINDOWTYPE_CHANNEL
        jne     .not_channel

        ; draw a vertical separator line
        mov     ebx, [xsize]
        sub     ebx, USERLIST_X + SCROLLBAR_WIDTH + 3
        push    bx
        shl     ebx, 16
        pop     bx
        mov     ecx, [ysize]
        add     ecx, TOP_Y SHL 16 -(15) ;;;;
        mcall

        call    redraw_channel_list

  .not_channel:
        mov     edx, [edi + window.data_ptr]
        add     edx, window_data.text
        call    draw_channel_text

; editbox

        mov     eax, [ysize]
        sub     eax, 12 ;;;;;;
        mov     [edit1.top], eax

        mov     eax, [xsize]
        mov     [edit1.width], eax

        push    dword edit1
        call    [edit_box_draw]

; tabs
        call    draw_windowtabs

        popa
        ret



redraw_channel_list:

; First, calculate scrollbar

        mov     ebx, [window_active]
        mov     eax, [ebx + window.users]       ; number of users in the open window
        mov     [scroll1.max_area], eax

        mov     eax, [ysize]
        sub     eax, TOP_Y + 15 ;;;;
        push    eax
        mov     [scroll1.y_size], ax

        mov     eax, [xsize]
        sub     eax, SCROLLBAR_WIDTH
        mov     [scroll1.x_pos], ax

        pop     eax                             ; scrollbar height
        xor     edx, edx
        mov     ecx, 10
        div     ecx
        mov     [scroll1.cur_area], eax

        ; Do we need a scrollbar?
        cmp     eax, [scroll1.max_area]
        jae     .noscroll

        ; Is the current position greater then the max position?
        cmp     eax, [scroll1.position]
        ja      @f
        mov     [scroll1.position], eax
  @@:

        ; OK, draw the scrollbar
        mov     [scroll1.all_redraw], 1

        push    dword scroll1
        call    [scrollbar_v_draw]

        jmp     print_channel_list

  .noscroll:
        mov     [scroll1.position], 0



print_channel_list:

        pusha

; first, draw an invisible button
        mov     ebx, [xsize]
        sub     ebx, USERLIST_X + SCROLLBAR_WIDTH
        shl     ebx, 16
        push    ebx
        mov     bx, USERLIST_X
        mov     ecx, [ysize]
        add     ecx, TEXT_Y shl 16 - (TEXT_Y + 15)        ;;;;;   + 10???
        push    ecx ebx
        mov     edx, WINDOW_BTN_LIST + 1 shl 29 + 1 shl 30
        mcall   8

; draw rectangle to clear previously printed names
        pop     ebx ecx
        mov     edx, [colors.work]
        mcall   13

; now draw the names according to the scrollbar position and window size
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
        or      ecx, 0x80000000                 ; ASCIIZ string
        mov     eax, 4                          ; draw text

        mov     edi, [ysize]                    ; Calculate how many names will fit on screen
        sub     edi, TEXT_Y + 15 ;+ 10           ;;;;;
  .loop:
        cmp     byte[edx], 0                    ; end of list?
        je      .done

        dec     ebp                             ; is this name selected?
        jnz     .nothighlight
                                                ; yes, highlight it
        pusha
        mov     cx, bx
        mov     bx, USERLIST_X
        shl     ecx, 16
        mov     cx, 10 - 1
        mov     edx, 0x00000055                 ; blue!
        mcall   13
        popa

        mov     ecx, 0x8000ffff                 ; cyan!
        mcall

        mov     ecx, [colors.work_text]
        or      ecx, 0x80000000                 ; ASCIIZ string
        jmp     .next

  .nothighlight:
        mcall

  .next:
        add     edx, MAX_NICK_LEN               ; next name
        add     ebx, 10                         ; height distance between lines
        sub     edi, 10
        ja      .loop

  .done:
        popa

        ret




draw_channel_text:

        pusha

        mov     eax, 4                  ; draw text
        mov     ebx, TEXT_X shl 16 + TEXT_Y
        mov     ecx, 12                 ; 12 lines max ?
        mov     esi, [textbox_width]

  .dct:
        pusha
        mov     cx, bx
        shl     ecx, 16
        mov     cx, 9                   ; character height
        mov     eax, 13                 ; draw rectangle
        mov     ebx, TEXT_X shl 16
        mov     bx, word[textbox_width]
        imul    bx, 6                   ; character width
        mov     edx, [colors.work]
        mcall
        popa

        push    ecx
        mov     ecx, [colors.work_text]
        cmp     word[edx], '* '
        jne     .no_red
        mov     ecx, 0x00aa0000
        jmp     .draw
  .no_red:

        cmp     word[edx], '**'
        jne     .no_light_blue
        cmp     byte[edx+2], '*'
        jne     .no_light_blue
        mov     ecx, 0x000000aa
        jmp     .draw
  .no_light_blue:

        cmp     byte[edx], '#'
        jne     .no_blue
        mov     ecx, 0x0000aa00
;        jmp     .draw
  .no_blue:

  .draw:
        mcall
        add     edx, [textbox_width]
        add     ebx, 10                 ; height distance between lines

        pop     ecx
        loop    .dct

        mov     eax, [window_active]
        and     [eax + window.flags], not FLAG_UPDATED  ; clear the 'window is updated' flag

        popa
        ret



draw_windowtabs:

; Create the buttons

        mov     eax, 8
        mov     ebx, 5 shl 16 + 120
        mov     ecx, 12 shl 16 + 12
        mov     edx, WINDOW_BTN_START
        mov     edi, windows
  .more_btn:
        mov     esi, [colors.work_button]
        cmp     [window_active], edi
        jne     @f
        not     esi
        and     esi, 0x00ffffff
      @@:
        mcall
        inc     edx
        add     ebx, 125 shl 16
        add     edi, sizeof.window
        cmp     [edi + window.data_ptr], 0
        jne     .more_btn

; Draw the windownames onto the buttons

        mov     eax, 4
        mov     ebx, 10 shl 16 + 15
        mov     esi, MAX_WINDOWS
        mov     edi, windows
  .more:
        mov     ecx, [colors.work_button_text]
        test    [edi + window.flags], FLAG_UPDATED
        jz      @f
        mov     ecx, 0x00aa0000 ; RED!
  @@:
        or      ecx, 0x80000000         ; ASCIIZ string
        lea     edx, [edi + window.name]
        mcall
        add     edi, sizeof.window      ; get ptr to next window
        cmp     [edi + window.data_ptr], 0
        je      .enough
        add     ebx, 125 shl 16
        dec     esi
        jnz     .more
  .enough:

; Draw the close window button

        mov     edi, [window_active]
        cmp     [edi + window.type], WINDOWTYPE_SERVER  ; dont let the user close server window
        je      @f

        mov     eax, 8
        mov     ebx, [xsize]
        sub     ebx, 12
        shl     ebx, 16
        mov     bx, 12
        mov     ecx, 6 shl 16 + 12
        mov     edx, WINDOW_BTN_CLOSE
;        mov     esi, [colors.work_button]
        mov     esi, 0x00aa0000         ; red !
        mcall

  @@:

        ret


