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

        mov     ebx, [xsize]
        mov     ecx, [ysize]
        sub     cx, 35
        push    cx
        shl     ecx, 16
        pop     cx
        mov     edx, [colors.work_graph]
        mcall   38                              ; draw line

        mov     ecx, TOP_Y SHL 16 + TOP_Y
        mcall

        mov     edi, [window_open]
        cmp     [edi + window.type], WINDOWTYPE_CHANNEL
        jne     .not_channel

        mov     ebx, [xsize]
        sub     ebx, USERLIST_X + SCROLLBAR_WIDTH + 3
        push    bx
        shl     ebx, 16
        pop     bx
        mov     ecx, [ysize]
        add     ecx, TOP_Y SHL 16 -(15+20)
        mcall

        call    print_channel_list

  .not_channel:

        mov     edx, [edi + window.data_ptr]
        add     edx, window_data.text
        call    draw_channel_text

; editbox

        mov     eax, [ysize]
        sub     eax, 30 ;;;;;;
        mov     [edit1.top], eax

        mov     eax, [xsize]
        mov     [edit1.width], eax

        push    dword edit1
        call    [edit_box_draw]

; scrollbar

        mov     eax, [ysize]
        sub     eax, TOP_Y + 35 ;;;;
        mov     [scroll1.y_size], ax

        mov     eax, [xsize]
        sub     eax, SCROLLBAR_WIDTH
        mov     [scroll1.x_pos], ax

        mov     [scroll1.all_redraw], 1

        push    dword scroll1
        call    [scrollbar_v_draw]

        call    draw_windownames

        mcall   12, 2
        popa
        ret



print_channel_list:

        pusha

        mov     eax, 13                         ; draw rectangle (clear list)

        mov     ebx, [xsize]
        sub     ebx, USERLIST_X + SCROLLBAR_WIDTH
        shl     ebx, 16
        push    ebx
        mov     bx, USERLIST_X
        mov     ecx, [ysize]
        add     ecx, TEXT_Y shl 16 - (TEXT_Y + 35 + 10 )        ;;;;;
        mov     edx, [colors.work]
        mcall

        mov     eax, [scroll1.position]
        xor     edx, edx
        mov     ecx, MAX_NICK_LEN
        mul     ecx
        mov     edx, eax
        mov     eax, [window_open]
        add     edx, [eax + window.data_ptr]
        add     edx, window_data.names

        pop     ebx
        mov     bx, TEXT_Y
        mov     ecx, [colors.work_text]
        or      ecx, 0x80000000                 ; ASCIIZ string
        mov     eax, 4                          ; draw text

        mov     edi, [ysize]                    ; Calculate how many names will fit on screen
        sub     edi, TEXT_Y + 35 + 10           ;;;;;
  .loop:
        ; TODO: check if name is selected and highlight background color if so
        cmp     byte[edx], ' '                  ; end of list?
        je      .done
        mcall

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

        popa
        ret



draw_windownames:

        mov     eax, 8
        mov     ebx, 5 shl 16 + 120
        mov     ecx, 12 shl 16 + 12
        mov     edx, WINDOW_BTN_START
        mov     edi, windows
  .more_btn:
        mov     esi, [colors.work_button]
        cmp     [window_open], edi
        jne     @f
        not     esi
        and     esi, 0x00ffffff
      @@:
        mcall
        inc     edx
        add     ebx, 125 shl 16
        add     edi, sizeof.window
        cmp     [edi + + window.data_ptr], 0
        jne     .more_btn

        mov     eax, 4
        mov     ebx, 10 shl 16 + 15
        mov     ecx, [colors.work_button_text]
        or      ecx, 0x80000000         ; ASCIIZ string
        lea     edx, [windows + window.name]
        mov     esi, MAX_WINDOWS
  .more:
        mcall
        add     edx, sizeof.window
        cmp     byte[edx], 0
        je      .enough
        add     ebx, 125 shl 16
        dec     esi
        jnz     .more
  .enough:

        ret


