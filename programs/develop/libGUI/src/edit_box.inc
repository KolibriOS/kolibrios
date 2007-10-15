
eb_control_data_size    = 82

;****************************************************
;******************craete EditBox*********************
;****************************************************
;IN
;pointer to parend
;pointer to EditBox's structure
;OUT
;pointer to initialized control
align 4

craete_edit_box:

      mov ebx,[esp+4]
      mov eax,[esp+8]

      mov [PointerToStructureForEditBox],eax

      mov eax,control_header_size+eb_control_data_size
      call craete_control

      ;set all EditBox's parameters in control
      mov [eax],dword edit_box

      mov ecx,eb_control_data_size
      mov esi,[PointerToStructureForEditBox]
      mov edi,eax
      add edi,control_header_size
      rep movsb

      call get_skin_height

      mov ebx,[PointerToStructureForEditBox]
      mov ecx,[ebx+4]   ;x
      mov edx,[ebx+8]   ;y
      mov esi,[ebx]     ;size x
      mov edi,[ebx+70]  ;size y
      add ecx,border_width
      add edx,[skin_height]
      ;copy information to control
      mov [eax+24],ecx
      mov [eax+28],edx
      mov [eax+32],esi
      mov [eax+36],edi

      ret 8

;****************************************************
;******************Draw EditBox**********************
;****************************************************
;include 'editbox_mac.inc'
;IN
;pointer to control of EditBox
;message
;OUT
;not returned value
align 4

include 'editbox_mac.inc'

edit_box:
      ;;;;;;;;;;;;
      ;Structure from libGui асоциация структур
      ;;;;;;;;;;;;

      ;get message
      mov eax,[esp+8]

      ;get pointer to control of button
      mov esi,[esp+4]
      mov [PointerForEditBox],esi

      ;copy control to local control
      add esi,control_header_size
      mov edi,dword EditBox
      mov ecx,eb_control_data_size
      rep movsb

      ;;;;;;;;;;;;
      ;Structure from libGui асоциация структур
      ;;;;;;;;;;;;
      srt_ed_libgui

      push eax
      ;load coordinats and size from control
      mov eax,[PointerForEditBox]
      mov ebx,[eax+24]           ;x
      mov ecx,[eax+28]           ;y
      mov edx,[eax+32]           ;size x
      mov esi,[eax+36]           ;size y
      ;set current coordinats and sizes in EeditBox
      mov [EditBox.ed_left],ebx
      mov [EditBox.ed_top],ecx
      mov [EditBox.ed_width],edx
      mov [EditBox.ed_height],esi
      pop eax

      ;events of redraw all EditBox

      cmp [eax],dword 1
      jne .no_redraw_all_edit_box


       call .draw_border
       .draw_bg_cursor_text:

       call .check_offset

       call .draw_bg

       call .draw_shift
       .draw_cursor_text:

       test     word[EditBox.ed_flags],ed_focus
       je       @f
       call     .draw_cursor
     @@:
       call     .draw_text

      jmp .editbox_exit;exit_edit_box
      .no_redraw_all_edit_box:


      ;events of keys for EditBox

      cmp [eax],dword 2
      jne .no_keys_edit_box

       mov ebx,[eax+4]
       shl ebx,8
       mov eax,ebx

        test    word [EditBox.ed_flags],ed_focus
        je      .editbox_exit;exit_edit_box;@b

        call    .check_shift

        ;----------------------------------------------------------
        ;--- проверяем, что нажато --------------------------------
        ;----------------------------------------------------------

        use_key_process  backspase,delete,left,right,home,end,insert

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;Заглушка на обработку клавиш вверх и вниз т.е. при
        ;обнаружении этих кодов происходит выход из обработчика
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        srt_ed_libgui
        use_key_no_process   up,down,esc

        ;--- нажата другая клавиша ---

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;Проверка установлен ли флаг при котором нужно выводить
        ;только цифры в нужном боксе если такой необходимости нет
        ;нужно закоментировать макрос
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        use_key_figures_only

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;проверка на shift был ли нажат
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        are_key_shift_press

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; проверяем, находится ли курсор в конце + дальнейшая обработка
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        are_key_cur_end

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;Обработка клавиш insert,delete.backspase,home,end,left,right
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        use_work_key


      .no_keys_edit_box:

      ;events of mouse for EditBox

      cmp [eax],dword 6
      jne .editbox_exit

        mov esi,[eax+4]
        mov edi,[eax+8]
        mov ecx,[eax+12]
        mov [EditBox.mouseX],esi
        mov [EditBox.mouseY],edi
        mov [ButtonsOfMouse],ecx

        mov eax,ecx

        test    eax,1
        jnz     .mouse_left_button
        and     word [EditBox.ed_flags],ed_mouse_on_off
        jmp     .editbox_exit

       .mouse_left_button:

        mov eax,[EditBox.mouseX]
        shl eax,16

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;Функция обработки  мышки получение координат и проверка их + выделения
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        use_work_mouse ;scr_h,scr_w

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;Общие функции обработки
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        use_general_func

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;Функции для работы с key
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        use_key_func

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;Функции для работы с mouse
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        use_mouse_func ;scr_w

        ;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;Общий выход из editbox для всех функций и пост обработчиков
        ;;;;;;;;;;;;;;;;;;;;;;;;;;

      .editbox_exit:

      exit_edit_box:

      ;save resulatat of work in control
      mov esi,dword EditBox
      mov edi,[PointerForEditBox]
      add edi,control_header_size
      mov ecx,eb_control_data_size
      rep movsb

      ret 8

