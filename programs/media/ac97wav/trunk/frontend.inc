thread:
        mov     edi, textbox_string
        or      ecx, -1
        xor     eax, eax
        repnz   scasb
        not     ecx
        dec     ecx
        mov     [textbox_position], cl
        call    draw_window
        call    main_loop
        mov     [status], ST_EXIT
        mcall   MF_EXIT

;---------------------------------------------------------------------

main_loop:
        cmp     [status], ST_PLAY
        je      @f
        mcall   MF_WAIT_EVENT
        jmp     .handle_event
     @@:
        call    draw_progress_bar
        mcall   MF_WAIT_EVENT_TIMEOUT, 80
     .handle_event:
        cmp     eax, EV_REDRAW
        je      redraw
        cmp     eax, EV_BUTTON
        je      button
        cmp     eax, EV_KEY
        je      key
        jmp     main_loop

 redraw:
        call    draw_window
        jmp     main_loop

 key:
        mcall   MF_GETKEY
        cmp     [textbox_active], 1
        jne     main_loop
        cmp     ah, 13
        je      .enter
        cmp     ah, 8
        je      .backspace
        movzx   ecx, [textbox_position]
        cmp     ecx, 255
        jae     .enter
        mov     [textbox_string + ecx], ah
        inc     [textbox_position]
        call    textbox_draw
        jmp     main_loop
  .enter:
        mov     [textbox_active], 0
        call    textbox_draw
        jmp     main_loop
  .backspace:
        movzx   ecx, [textbox_position]
        test    ecx, ecx
        jz      main_loop
        dec     [textbox_position]
        mov     [textbox_string + ecx - 1], byte 0
        call    textbox_draw
        jmp     main_loop

 button:
        mcall   MF_GETBUTTON
        cmp     ah, 0x10
        je      play_button
        cmp     ah, 0x11
        je      stop_button
        cmp     ah, 0x12
        je      decr_button
        cmp     ah, 0x13
        je      incr_button
        cmp     ah, 0x14
        je      volm_button
        cmp     ah, 0x15
        je      volp_button
        cmp     ah, 0x20
        je      activate_textbox
        cmp     ah, 0x30
        je      progressbar_click
        cmp     ah, 1
        jne     main_loop

;        mov     [status], ST_STOP
;        mcall   MF_DELAY, 40
        ret

    play_button:
        xor     eax, eax
        xchg    al, [textbox_active]
        cmp     al, 0
        je      @f
        call    textbox_draw
      @@:
        mov     [status], ST_PLAY
        jmp     main_loop
    stop_button:
        mov     [status], ST_STOP
        jmp     main_loop

    decr_button:
;        mov     [status], ST_STOP
;       @@:
;        mcall   5, 1
;        cmp     [status], ST_DONE
;        jne     @b
;        movzx   esi, [textbox_position]
;        add     esi, textbox_string
;       @@:
;        cmp     byte [esi], '/'
;        je      @f
;        dec     esi
;        jmp     @b
;       @@:
;        mov     byte [esi+1], 0
;        mov     [fileinfo.first_block], 0
;        mov     [fileinfo.dest], WAV_BUFFER1
;        mcall   58, fileinfo
;        add     ebx, WAV_BUFFER1
;        mov     esi, WAV_BUFFER1+8
;      .next_file:
;        cmp     ebx, esi
;        jbe     .fin
;        cmp     word [esi], "WA"
;        jne     .next_file
;        cmp     byte [esi+1], "V"
;        jne     .next_file
;      .fin:

        ;mov     eax, [fileinfo.first_block]
        ;cmp     eax, 1000
        ;jnl     @f
        ;mov     [fileinfo.first_block], 0
        ;jmp     main_loop
       ;@@:
        ;sub     [fileinfo.first_block], 1000
        ;jmp     main_loop

    incr_button:
        ;add     [fileinfo.first_block], 1000
        jmp     main_loop

    volm_button:
        inc     byte [volume]
        and     byte [volume], 0x1f
        jz      volp_button
        or      [volume], 0x10000000
        jmp     _print_volume
;        jmp     main_loop

    volp_button:
        dec     byte [volume]
        and     byte [volume], 0x1f
        jz      volm_button
        or      [volume], 0x10000000
;        jmp     main_loop

    _print_volume:
        movzx   eax, byte [volume]
        neg     eax
        add     eax, 31
        dps     "Volume: "
        dpd     eax
        newline
        jmp     main_loop

    activate_textbox:
        cmp     [status], ST_DONE
        jne     main_loop
        mov     [textbox_active], 1
        call    textbox_draw
        jmp     main_loop

    progressbar_click:
        ;print  "click on progress bar"
        cmp     [status], ST_DONE
        je      main_loop
        mcall   MF_GETMOUSE, MS_COORDS_WINDOW
        shr     eax, 16 ; get mouse.x
        sub     eax, 7
        jz      @f
        imul    [file_size]
        mov     ebx, 286
        div     ebx
     @@:
        ;dps "block: "
        ;dpd eax
        ;newline
        mov     [fileinfo.first_byte], eax
        call    draw_progress_bar
        jmp     main_loop
ret

;---------------------------------------------------------------------

PBAR_WIDTH = 286

draw_window:
        mcall   MF_DRAWSTATUS, DS_BEGIN

        mcall   MF_WINDOW, <100,299>, <100,72>, 0x03404040

        ; create six buttons
        mov     edi, 6
        mpack   ebx, 7, 45
        mpack   ecx, 24, 13
        mov     edx, 0x10
        mov     esi, 0xA0A0A0
      @@:
        mcall   MF_BUTTON
        add     ebx, 48 shl 16
        inc     edx
        dec     edi
        jnz     @b

        mcall   MF_TEXT, <8,8>, 0x10FFFFFF, header, header.size

        mcall   ,<13,28>, 0x404040, buttons_text, buttons_text.size
        sub     ebx, 0x00010001
        mov     ecx, 0xFFFFFF
        mcall

        call    draw_progress_bar
        call    textbox_draw

        mcall   MF_DRAWSTATUS, DS_END
ret

;---------------------------------------------------------------------

textbox_draw:
        mcall   MF_BUTTON, <7,285>, <55,10>, 0x60000020

        mov     edx, 0x808080
        cmp     [textbox_active], 1
        jne     @f
        mov     edx, 0xA0A0A0
    @@:
        mcall   MF_BAR, <7,286>, <55,11>

        movzx   esi, [textbox_position]
        mcall   MF_TEXT, <10,56>, 0x404040, textbox_string
ret

;---------------------------------------------------------------------

draw_progress_bar:
        pushad

        mov     eax, PBAR_WIDTH
        mul     [fileinfo.first_byte]
        div     [file_size]

        push    eax
        mcall   MF_BAR, <7,286>, <41,11>, 0x808080
        mcall   MF_BUTTON,     ,        , 0x60000030
        pop     eax

        mov     bx, ax
        mov     edx, 0xA0A0A0
        mcall   MF_BAR

        popad
ret

;---------------------------------------------------------------------

sz header,       "AC'97 WAV player -  all PCM audio"
sz buttons_text, " Play    Stop     <<      >>     Vol-    Vol+"

textbox_active    db 0
textbox_position  db 0;textbox_string.size-1
file_size dd 100
