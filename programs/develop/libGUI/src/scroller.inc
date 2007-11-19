
;scroller's flags
sc_check_vertical                     = 00000001b
sc_check_horizontal                   = 00000010b
sc_check_init_child_buttons           = 00000100b
sc_active                             = 00001000b
sc_check_3D_type                      = 00010000b
sc_scroll_bar_move                    = 00100000b

sc_redraw                             = 10000000b
sc_redraw_off                         = 01111111b
sc_full_redraw                        = 10101000b
sc_scroll_bar_move_off                = 11011111b
sc_move_active_off                    = 11010111b

sc_first_child_button_pressed         = 00000001b
sc_first_child_button_pressed_off     = 11111110b
sc_second_child_button_pressed        = 00010000b
sc_second_child_button_pressed_off    = 11101111b

sc_control_data_size                  = 29
sc_child_button_type                  = 10001000b

;****************************************************
;******************Crate Scroller*********************
;****************************************************
;IN
;pointer to parend
;pointer to Scroller's structure
;OUT
;pointer to initialized Scroller's structure
align 4

craete_scroller:

      mov ebx,[esp+4]
      mov eax,[esp+8]

      mov [PointerToStructureForScroller],eax

      mov eax,control_header_size+sc_control_data_size
      call craete_control

     ;set all button's parameters in control
      mov [eax],dword scroller

      mov ecx,sc_control_data_size
      mov esi,[PointerToStructureForScroller]
      mov edi,eax
      add edi,control_header_size
      rep movsb

      mov ebx,[PointerToStructureForScroller]

      xor ecx,ecx
      mov cl,byte[ebx]
      and cl,sc_check_vertical
      jz type_horizontal_scroller

      ;if we here than scroller vertical
      mov esi,16         ;scroller size x
      xor edi,edi
      mov di,[ebx+5]    ;scroller size y
      jmp exit_type_scroller
      type_horizontal_scroller:

      xor esi,esi
      mov si,[ebx+5]    ;scroller size x
      mov edi,16        ;scroller size y
      exit_type_scroller:

      xor ecx,ecx
      xor edx,edx
      mov cx,[ebx+1]    ;scroller x
      mov dx,[ebx+3]    ;scroller y

      call get_skin_height
      add ecx,border_width
      add edx,[skin_height]
      ;copy information to control
      mov [eax+24],ecx
      mov [eax+28],edx
      mov [eax+32],esi
      mov [eax+36],edi

      ret 8

;****************************************************
;*******************Draw Scroller*********************
;****************************************************
;IN
;pointer to control of scroller
;message
;OUT
;not returned value
align 4

scroller:


      ;get message
      mov eax,[esp+8]

      ;copy message
      mov esi,eax
      mov edi,dword Message
      mov ecx,4
      rep movsd

      ;get pointer to control of scroller
      mov esi,[esp+4]
      mov [PointerForScroller],esi
      ;copy control to local control
      mov edi,dword Scroller
      mov ecx,control_header_size+sc_control_data_size
      rep movsb


      cmp [eax],dword 1
      jne no_redraw_all_scroller

      or [Scroller.type],sc_full_redraw
      no_redraw_all_scroller:

      cmp [eax],dword 2
      jne no_keys_scroller

      jmp exit_scroller
      no_keys_scroller:

      cmp [eax],dword 3
      jne no_pressed_button_scroller

      jmp exit_scroller
      no_pressed_button_scroller:

      cmp [eax],dword 6
      jne no_mouse_scroller

      mov esi,[eax+4]
      mov edi,[eax+8]
      mov ecx,[eax+12]
      mov [Scroller.mouseX],si
      mov [Scroller.mouseY],di
      mov [ButtonsOfMouse],ecx
      no_mouse_scroller:


      cmp [eax],dword -1
      jne no_delete_scroller

        mov eax,[Scroller.ChildButton1]
        call free

        mov eax,[Scroller.ChildButton2]
        call free

        ;push [Scroller.ChildButton1]
        ;call destroy_control

        ;push [Scroller.ChildButton2]
        ;call destroy_control

      jmp exit_scroller
      no_delete_scroller:

      ;load coordinats and size from control
      mov eax,[PointerForScroller]
      mov ebx,[eax+24]           ;x
      mov ecx,[eax+28]           ;y
      mov edx,[eax+32]           ;size x
      mov esi,[eax+36]           ;size y
      ;set current coordinats and sizes in scroller
      mov [Scroller.x],bx
      mov [Scroller.y],cx

      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_check_vertical
      test al,al
      jz set_for_horizontal

        mov [Scroller.length],si

      jmp exit_if_vertical
      set_for_horizontal:

        mov [Scroller.length],dx

      exit_if_vertical:

      ;checking to craete child buttons
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_check_init_child_buttons
      jnz child_buttons_craeted

         or [Scroller.type],sc_check_init_child_buttons

         mov [ChildButton.type],byte sc_child_button_type
         mov [ChildButton.width],16
         mov [ChildButton.height],16
         xor ebx,ebx
         xor ecx,ecx
         mov bx,[Scroller.x]
         mov cx,[Scroller.y]
         mov [ChildButton.x],bx
         mov [ChildButton.y],cx

         push ChildButton
         push [PointerForScroller]

         call craete_button

         mov [Scroller.ChildButton1],eax

         xor ebx,ebx
         xor ecx,ecx
         mov bx,[Scroller.x]
         mov cx,[Scroller.y]

         xor eax,eax
         mov al,[Scroller.type]
         and al,sc_check_vertical
         test al,al
         jz init_buttons_for_horizontal_scroller


           add cx,[Scroller.length]
           sub ecx,16


           jmp exit_if_init_buttons_for_horizontal_scroller
         init_buttons_for_horizontal_scroller:


           add bx,[Scroller.length]
           sub ebx,16

         exit_if_init_buttons_for_horizontal_scroller:

         mov [ChildButton.x],bx
         mov [ChildButton.y],cx

         push ChildButton
         push [PointerForScroller]

         call craete_button

         mov [Scroller.ChildButton2],eax

         mov esi,dword Scroller
         mov edi,[PointerForScroller]
         add esi,control_header_size
         add edi,control_header_size
         mov ecx,sc_control_data_size
         rep movsb

      child_buttons_craeted:

      ;checking for redraw all control
      ;if it is true than change koordinats of child buttons
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_redraw
      test al,al
      jz no_change_koordinats_of_child_buttons

         xor ebx,ebx
         xor ecx,ecx
         mov bx,[Scroller.x]
         mov cx,[Scroller.y]

         ;first child button
         mov eax,[Scroller.ChildButton1]
         mov [eax+24],ebx      ;x
         mov [eax+28],ecx      ;y

         xor eax,eax
         mov al,[Scroller.type]
         and al,sc_check_vertical
         test al,al
         jz change_koordinats_of_buttons_for_horizontal_scroller

           add cx,[Scroller.length]
           sub ecx,16

           jmp exit_if_change_koordinats_of_buttons_for_horizontal_scroller
         change_koordinats_of_buttons_for_horizontal_scroller:

           add bx,[Scroller.length]
           sub ebx,16

         exit_if_change_koordinats_of_buttons_for_horizontal_scroller:

         ;second child button
         mov eax,[Scroller.ChildButton2]
         mov [eax+24],ebx      ;x
         mov [eax+28],ecx      ;y

      no_change_koordinats_of_child_buttons:

      ;is a 3D scroller ?
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_check_3D_type
      test al,al
      jz no_3D_scroller


      mov eax,[Scroller.color1]
      mov ebx,0xffffff
      mov ecx,16
      mov edx,dword pointer
      call gradient

      mov eax,[pointer+9]
      and eax,0xffffff
      mov [Color],eax

      mov eax,[pointer]
      mov ebx,0xffffff
      call calculate_average_color
      mov [AveregeColor],eax

      ;is a vertical 3D scroller ?
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_check_vertical
      test al,al
      jz no_vertical_scroller_3D

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov cx,16
      mov dx,[Scroller.length]
      mov si,[Scroller.mouseX]
      mov di,[Scroller.mouseY]
      add ebx,17
      sub edx,17+17
      call CheckCrossingBox

      cmp eax,0xffffff
      jne no_crossing_vertical_scroller_3D

      or [Scroller.type],sc_active
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      no_crossing_vertical_scroller_3D:

      ;is a vertical 3D scroller active ?
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_active
      test al,al
      jz no_vertical_scroller_active

      ;calculate size of scroll bar
      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,16+16
      mov [v],eax
      fld [Scroller.size]
      fild [v]
      fmul st0,st1
      fistp [Scrollersize]
      fstp [v]

      cmp [Scrollersize],4
      jae no_minimum_size_vertical_3D_scroller

        mov [Scrollersize],4
      no_minimum_size_vertical_3D_scroller:

      ;if scroller is not active than use this is coordinats.
      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,17+17
      sub eax,[Scrollersize]
      mov [yo],eax
      fld [Scroller.pos]
      fild [yo]
      fmul st0,st1
      fistp [v]
      fstp [v2]

      xor eax,eax
      mov ax,[Scroller.y]
      add ax,17
      add eax,[v]
      mov [ScrollBar.y],eax

      ;check button of mouse
      mov eax,[ButtonsOfMouse]
      and eax,1b
      test al,al
      jz no_vertical_scroller_move
      ;calculate in pixels length of scroll bar

      xor eax,eax
      xor edx,edx
      xor ecx,ecx
      mov ax,[Scroller.y]
      mov cx,[Scroller.length]
      mov dx,[Scroller.mouseY]
      mov ebx,edx
      add ecx,eax
      add eax,17
      sub ecx,17
      mov esi,ecx

      sub ebx,eax
      jns no_minimum_scroller_y

      mov [Scroller.mouseY],ax
      no_minimum_scroller_y:

      sub ecx,edx
      jns no_maximum_scroller_y

      mov [Scroller.mouseY],si
      no_maximum_scroller_y:


      mov eax,[Scrollersize]
      shr eax,1
      xor ebx,ebx
      mov bx,[Scroller.mouseY]
      sub bx,[Scroller.y]
      sub ebx,17
      xor ecx,ecx
      mov cx,[Scroller.y]
      add cx,[Scroller.length]
      sub cx,17
      sub cx,[Scroller.mouseY]

      cmp ebx,eax
      jae no_correct_y
       xor edx,edx
       mov dx,[Scroller.y]
       add dx,17
       add edx,eax
      no_correct_y:

      cmp ecx,eax
      jae no_correct_y_
       xor edx,edx
       mov dx,[Scroller.y]
       add dx,[Scroller.length]
       sub dx,17
       sub edx,eax
      no_correct_y_:

      mov [ScrollBar.y],edx
      sub [ScrollBar.y],eax
      sub dx,[Scroller.y]
      sub edx,17
      sub edx,eax
      mov [v],edx

      shl eax,1
      xor ebx,ebx
      mov bx,[Scroller.length]
      sub ebx,17+17
      sub ebx,[Scrollersize]
      mov [v2],ebx

      fild [v2]
      fild [v]
      fdiv st0,st1
      fstp [Scroller.pos]
      fistp [v2]

      ;put 5 bit(scroll bar move)
      or [Scroller.type],sc_scroll_bar_move

      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      jmp no_vertical_scroller_active
      no_vertical_scroller_move:

      no_vertical_scroller_active:

      ;if bit 7 is a true than redraw all scroller.

      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_redraw
      test al,al
      jz no_redraw_all_vertical_scroller

      and [Scroller.type],sc_redraw_off
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      call DrawButtonsOfVertical3DScroller

      no_redraw_all_vertical_scroller:

      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_scroll_bar_move
      test al,al
      jz no_move_vertical_scroller_bar_3D

        xor eax,eax
        mov ax,[Scroller.x]
        mov ebx,[ScrollBar.y]
        mov ecx,[Scrollersize]
        call DrawVertical3DScrollBar

        and [Scroller.type],sc_scroll_bar_move_off
        mov esi,dword Scroller
        mov edi,[PointerForScroller]
        add esi,control_header_size
        add edi,control_header_size
        mov ecx,sc_control_data_size
        rep movsb

      no_move_vertical_scroller_bar_3D:

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov cx,16
      mov dx,[Scroller.length]
      mov si,[Scroller.mouseX]
      mov di,[Scroller.mouseY]
      add ebx,17
      sub edx,17+17
      call CheckCrossingBox

      cmp eax,0xffffff
      jne no_crossing_vertical_scroller_3D_

      or [Scroller.type],sc_active
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      jmp exit_crossing_vertical_scroller_3D
      no_crossing_vertical_scroller_3D_:

        ;check button of mouse
        mov eax,[ButtonsOfMouse]

        and eax,1b
        test al,al
        jnz mouse_button_pressed_vertical

          and [Scroller.type],sc_move_active_off
          mov esi,dword Scroller
          mov edi,[PointerForScroller]
          add esi,control_header_size
          add edi,control_header_size
          mov ecx,sc_control_data_size
          rep movsb
         mouse_button_pressed_vertical:

      exit_crossing_vertical_scroller_3D:


      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_active
      test al,al
      jz no_change_vertical_scroller_3D

        xor eax,eax
        xor ebx,ebx
        mov ax,[Scroller.x]
        mov bx,[Scroller.mouseX]

        sub eax,ebx
        jns no_signum_1
        neg eax
        no_signum_1:

        cmp eax,110
        jle no_change_bits_vertical_scroller_3D

        and [Scroller.type],sc_move_active_off
        no_change_bits_vertical_scroller_3D:

        mov esi,dword Scroller
        mov edi,[PointerForScroller]
        add esi,control_header_size
        add edi,control_header_size
        mov ecx,sc_control_data_size
        rep movsb

      no_change_vertical_scroller_3D:

      no_vertical_scroller_3D:

      ;****************Horinzontal scroller**************

      ;is a horizontal 3D scroller ?
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_check_horizontal
      test al,al
      jz no_horizontal_scroller_3D

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov cx,[Scroller.length]
      mov dx,16
      mov si,[Scroller.mouseX]
      mov di,[Scroller.mouseY]
      add eax,17
      sub ecx,17+17
      call CheckCrossingBox

      cmp eax,0xffffff
      jne no_crossing_horizontal_scroller_3D

      or [Scroller.type],sc_active
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      no_crossing_horizontal_scroller_3D:

      ;is a vertical 3D scroller active ?
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_active
      test al,al
      jz no_horizontal_scroller_active

      ;calculate size of scroll bar
      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,16+16
      mov [v],eax
      fld [Scroller.size]
      fild [v]
      fmul st0,st1
      fistp [Scrollersize]
      fstp [v]

      cmp [Scrollersize],4
      jae no_minimum_size_horizontal_3D_scroller

        mov [Scrollersize],4
      no_minimum_size_horizontal_3D_scroller:

      ;if scroller is not active than use this is coordinats.
      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,17+17
      sub eax,[Scrollersize]
      mov [xo],eax
      fld [Scroller.pos]
      fild [xo]
      fmul st0,st1
      fistp [v]
      fstp [v2]

      xor eax,eax
      mov ax,[Scroller.x]
      add ax,17
      add eax,[v]
      mov [ScrollBar.x],eax

      ;check button of mouse
      mov eax,[ButtonsOfMouse]

      and eax,1b
      test al,al
      jz no_horizontal_scroller_move
      ;calculate in pixels length of scroll bar

      xor eax,eax
      xor edx,edx
      xor ecx,ecx
      mov ax,[Scroller.x]
      mov cx,[Scroller.length]
      mov dx,[Scroller.mouseX]
      mov ebx,edx
      add ecx,eax
      add eax,17
      sub ecx,17
      mov esi,ecx

      sub ebx,eax
      jns no_minimum_scroller_x_

      mov [Scroller.mouseX],ax
      no_minimum_scroller_x_:

      sub ecx,edx
      jns no_maximum_scroller_x_

      mov [Scroller.mouseX],si
      no_maximum_scroller_x_:


      mov eax,[Scrollersize]
      shr eax,1
      xor ebx,ebx
      mov bx,[Scroller.mouseX]
      sub bx,[Scroller.x]
      sub ebx,17
      xor ecx,ecx
      mov cx,[Scroller.x]
      add cx,[Scroller.length]
      sub cx,17
      sub cx,[Scroller.mouseX]

      cmp ebx,eax
      jae no_correct_x_
       xor edx,edx
       mov dx,[Scroller.x]
       add dx,17
       add edx,eax
      no_correct_x_:

      cmp ecx,eax
      jae no_correct_x__
       xor edx,edx
       mov dx,[Scroller.x]
       add dx,[Scroller.length]
       sub dx,17
       sub edx,eax
      no_correct_x__:

      mov [ScrollBar.x],edx
      sub [ScrollBar.x],eax
      sub dx,[Scroller.x]
      sub edx,17
      sub edx,eax
      mov [v],edx

      xor ebx,ebx
      mov bx,[Scroller.length]
      sub ebx,17+17
      sub ebx,[Scrollersize]
      mov [v2],ebx

      fild [v2]
      fild [v]
      fdiv st0,st1
      fstp [Scroller.pos]
      fistp [v2]

      ;put 5 bit(scroll bar move)
      or [Scroller.type],sc_scroll_bar_move

      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      jmp no_horizontal_scroller_active
      no_horizontal_scroller_move:

      no_horizontal_scroller_active:

      ;if bit 7 is a true than redraw all scroller.

      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_redraw
      test al,al
      jz no_redraw_all_horizontal_scroller

      and [Scroller.type],sc_redraw_off
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      call DrawButtonsOfHorizontal3DScroller

      no_redraw_all_horizontal_scroller:

      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_scroll_bar_move
      test al,al
      jz no_move_horizontal_scroller_bar_3D

        xor ebx,ebx
        mov bx,[Scroller.y]
        mov eax,[ScrollBar.x]
        mov ecx,[Scrollersize]
        call DrawHorizontal3DScrollBar

        and [Scroller.type],sc_scroll_bar_move_off
        mov esi,dword Scroller
        mov edi,[PointerForScroller]
        add esi,control_header_size
        add edi,control_header_size
        mov ecx,sc_control_data_size
        rep movsb

      no_move_horizontal_scroller_bar_3D:

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov dx,16
      mov cx,[Scroller.length]
      mov si,[Scroller.mouseX]
      mov di,[Scroller.mouseY]
      add eax,17
      sub ecx,17+17
      call CheckCrossingBox

      cmp eax,0xffffff
      jne no_crossing_horizontal_scroller_3D_

      or [Scroller.type],sc_active
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      jmp exit_crossing_horizontal_scroller_3D
      no_crossing_horizontal_scroller_3D_:

        ;check button of mouse
        mov eax,[ButtonsOfMouse]

        and eax,1b
        test al,al
        jnz mouse_button_pressed_horizontal

          and [Scroller.type],sc_move_active_off
          mov esi,dword Scroller
          mov edi,[PointerForScroller]
          add esi,control_header_size
          add edi,control_header_size
          mov ecx,sc_control_data_size
          rep movsb
         mouse_button_pressed_horizontal:

      exit_crossing_horizontal_scroller_3D:


      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_active
      test al,al
      jz no_change_horizontal_scroller_3D

        xor eax,eax
        xor ebx,ebx
        mov ax,[Scroller.y]
        mov bx,[Scroller.mouseY]

        sub eax,ebx
        jns no_signum_1_x
        neg eax
        no_signum_1_x:

        cmp eax,110
        jle no_change_bits_horizontal_scroller_3D

        and [Scroller.type],sc_move_active_off
        no_change_bits_horizontal_scroller_3D:

        mov esi,dword Scroller
        mov edi,[PointerForScroller]
        add esi,control_header_size
        add edi,control_header_size
        mov ecx,sc_control_data_size
        rep movsb

      no_change_horizontal_scroller_3D:

      no_horizontal_scroller_3D:

      no_3D_scroller:

      ;****************************************************
      ;********************2D scrollers*********************
      ;****************************************************

      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_check_3D_type
      test al,al
      jnz no_2D_scroller

      mov eax,[Scroller.color1]
      mov ebx,0xffffff
      mov ecx,16
      mov edx,dword pointer
      call gradient

      mov eax,[pointer+9]
      and eax,0xffffff
      mov [Color],eax

      mov eax,[pointer]
      mov ebx,0xffffff
      call calculate_average_color
      mov [AveregeColor],eax

      ;is a vertical 2D scroller ?
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_check_vertical
      test al,al
      jz no_vertical_scroller_2D

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov cx,16
      mov dx,[Scroller.length]
      mov si,[Scroller.mouseX]
      mov di,[Scroller.mouseY]
      add ebx,17
      sub edx,17+17
      call CheckCrossingBox

      cmp eax,0xffffff
      jne no_crossing_vertical_scroller_2D

      or [Scroller.type],sc_active
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      no_crossing_vertical_scroller_2D:

      ;is a vertical 2D scroller active ?
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_active
      test al,al
      jz no_vertical_scroller_active_2D

      ;calculate size of scroll bar
      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,16+16
      mov [v],eax
      fld [Scroller.size]
      fild [v]
      fmul st0,st1
      fistp [Scrollersize]
      fstp [v]

      cmp [Scrollersize],4
      jae no_minimum_size_vertical_2D_scroller

        mov [Scrollersize],4
      no_minimum_size_vertical_2D_scroller:

      ;if scroller is not active than use this is coordinats.
       xor eax,eax
      mov ax,[Scroller.length]
      sub eax,17+17
      sub eax,[Scrollersize]
      mov [yo],eax
      fld [Scroller.pos]
      fild [yo]
      fmul st0,st1
      fistp [v]
      fstp [v2]

      xor eax,eax
      mov ax,[Scroller.y]
      add ax,16
      add eax,[v]
      mov [ScrollBar.y],eax

      ;check button of mouse
      mov eax,[ButtonsOfMouse]

      and eax,1b
      test al,al
      jz no_vertical_scroller_move_2D
      ;calculate in pixels length of scroll bar

      xor eax,eax
      xor edx,edx
      xor ecx,ecx
      mov ax,[Scroller.y]
      mov cx,[Scroller.length]
      mov dx,[Scroller.mouseY]
      mov ebx,edx
      add ecx,eax
      add eax,17
      sub ecx,17
      mov esi,ecx

      sub ebx,eax
      jns no_minimum_scroller_y_2D

      mov [Scroller.mouseY],ax
      no_minimum_scroller_y_2D:

      sub ecx,edx
      jns no_maximum_scroller_y_2D

      mov [Scroller.mouseY],si
      no_maximum_scroller_y_2D:


      mov eax,[Scrollersize]
      shr eax,1
      xor ebx,ebx
      mov bx,[Scroller.mouseY]
      sub bx,[Scroller.y]
      sub ebx,17
      xor ecx,ecx
      mov cx,[Scroller.y]
      add cx,[Scroller.length]
      sub cx,17
      sub cx,[Scroller.mouseY]

      cmp ebx,eax
      jae no_correct_y_2D_
       xor edx,edx
       mov dx,[Scroller.y]
       add dx,16
       add edx,eax
      no_correct_y_2D_:

      cmp ecx,eax
      jae no_correct_y_2D
       xor edx,edx
       mov dx,[Scroller.y]
       add dx,[Scroller.length]
       sub dx,16
       sub edx,eax
      no_correct_y_2D:

      mov [ScrollBar.y],edx
      sub [ScrollBar.y],eax
      sub dx,[Scroller.y]
      sub edx,16
      sub edx,eax
      mov [v],edx

      shl eax,1
      xor ebx,ebx
      mov bx,[Scroller.length]
      sub ebx,16+16
      sub ebx,[Scrollersize]
      mov [v2],ebx

      fild [v2]
      fild [v]
      fdiv st0,st1
      fstp [Scroller.pos]
      fistp [v2]

      ;put 5 bit(scroll bar move)
      or [Scroller.type],sc_scroll_bar_move

      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      jmp no_vertical_scroller_active_2D
      no_vertical_scroller_move_2D:

      no_vertical_scroller_active_2D:

      ;if bit 7 is a true than redraw all scroller.

      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_redraw
      test al,al
      jz no_redraw_all_vertical_scroller_2D

      and [Scroller.type],sc_redraw_off
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      call DrawButtonsOf2DVerticalScroller

      no_redraw_all_vertical_scroller_2D:

      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_scroll_bar_move
      test al,al
      jz no_move_vertical_scroller_bar_2D

        xor eax,eax
        mov ax,[Scroller.x]
        mov ebx,[ScrollBar.y]
        mov ecx,[Scrollersize]
        call DrawVertical2DScrollBar

        and [Scroller.type],sc_scroll_bar_move_off
        mov esi,dword Scroller
        mov edi,[PointerForScroller]
        add esi,control_header_size
        add edi,control_header_size
        mov ecx,sc_control_data_size
        rep movsb

      no_move_vertical_scroller_bar_2D:

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov cx,16
      mov dx,[Scroller.length]
      mov si,[Scroller.mouseX]
      mov di,[Scroller.mouseY]
      add ebx,17
      sub edx,17+17
      call CheckCrossingBox

      cmp eax,0xffffff
      jne no_crossing_vertical_scroller_2D_

      or [Scroller.type],sc_active
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      jmp exit_crossing_vertical_scroller_2D
      no_crossing_vertical_scroller_2D_:

        ;check button of mouse
        mov eax,[ButtonsOfMouse]

        and eax,1b
        test al,al
        jnz mouse_button_pressed_vertical_2D

          and [Scroller.type],sc_move_active_off
          mov esi,dword Scroller
          mov edi,[PointerForScroller]
          add esi,control_header_size
          add edi,control_header_size
          mov ecx,sc_control_data_size
          rep movsb
         mouse_button_pressed_vertical_2D:

      exit_crossing_vertical_scroller_2D:


      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_active
      test al,al
      jz no_change_vertical_scroller_2D

        xor eax,eax
        xor ebx,ebx
        mov ax,[Scroller.x]
        mov bx,[Scroller.mouseX]

        sub eax,ebx
        jns no_signum_1_y_2D
        neg eax
        no_signum_1_y_2D:

        cmp eax,110
        jle no_change_bits_vertical_scroller_2D

        and [Scroller.type],sc_move_active_off
        no_change_bits_vertical_scroller_2D:

        mov esi,dword Scroller
        mov edi,[PointerForScroller]
        add esi,control_header_size
        add edi,control_header_size
        mov ecx,sc_control_data_size
        rep movsb

      no_change_vertical_scroller_2D:

      no_vertical_scroller_2D:

      ;****************Horinzontal scroller**************

      ;is a horizontal 2D scroller ?
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_check_horizontal
      test al,al
      jz no_horizontal_scroller_2D

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov cx,[Scroller.length]
      mov dx,16
      mov si,[Scroller.mouseX]
      mov di,[Scroller.mouseY]
      add eax,17
      sub ecx,17+17
      call CheckCrossingBox

      cmp eax,0xffffff
      jne no_crossing_horizontal_scroller_2D

      or [Scroller.type],sc_active
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      no_crossing_horizontal_scroller_2D:

      ;is a vertical 3D scroller active ?
      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_active
      test al,al
      jz no_horizontal_scroller_active_2D

      ;calculate size of scroll bar
      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,16+16
      mov [v],eax
      fld [Scroller.size]
      fild [v]
      fmul st0,st1
      fistp [Scrollersize]
      fstp [v]

      cmp [Scrollersize],4
      jae no_minimum_size_horizontal_2D_scroller

        mov [Scrollersize],4
      no_minimum_size_horizontal_2D_scroller:

      ;if scroller is not active than use this is coordinats.
       xor eax,eax
      mov ax,[Scroller.length]
      sub eax,17+17
      sub eax,[Scrollersize]
      mov [yo],eax
      fld [Scroller.pos]
      fild [yo]
      fmul st0,st1
      fistp [v]
      fstp [v2]

      xor eax,eax
      mov ax,[Scroller.x]
      add ax,16
      add eax,[v]
      mov [ScrollBar.x],eax

      ;check button of mouse
      mov eax,[ButtonsOfMouse]

      and eax,1b
      test al,al
      jz no_horizontal_scroller_move_2D
      ;calculate in pixels length of scroll bar

      xor eax,eax
      xor edx,edx
      xor ecx,ecx
      mov ax,[Scroller.x]
      mov cx,[Scroller.length]
      mov dx,[Scroller.mouseX]
      mov ebx,edx
      add ecx,eax
      add eax,17
      sub ecx,17
      mov esi,ecx

      sub ebx,eax
      jns no_minimum_scroller_x_2D

      mov [Scroller.mouseX],ax
      no_minimum_scroller_x_2D:

      sub ecx,edx
      jns no_maximum_scroller_x_2D

      mov [Scroller.mouseX],si
      no_maximum_scroller_x_2D:


      mov eax,[Scrollersize]
      shr eax,1
      xor ebx,ebx
      mov bx,[Scroller.mouseX]
      sub bx,[Scroller.x]
      sub ebx,17
      xor ecx,ecx
      mov cx,[Scroller.x]
      add cx,[Scroller.length]
      sub cx,17
      sub cx,[Scroller.mouseX]

      cmp ebx,eax
      jae no_correct_x_2D
       xor edx,edx
       mov dx,[Scroller.x]
       add dx,16
       add edx,eax
      no_correct_x_2D:

      cmp ecx,eax
      jae no_correct_x__2D
       xor edx,edx
       mov dx,[Scroller.x]
       add dx,[Scroller.length]
       sub dx,16
       sub edx,eax
      no_correct_x__2D:

      mov [ScrollBar.x],edx
      sub [ScrollBar.x],eax
      sub dx,[Scroller.x]
      sub edx,16
      sub edx,eax
      mov [v],edx

      xor ebx,ebx
      mov bx,[Scroller.length]
      sub ebx,16+16
      sub ebx,[Scrollersize]
      mov [v2],ebx

      fild [v2]
      fild [v]
      fdiv st0,st1
      fstp [Scroller.pos]
      fistp [v2]

      ;put 5 bit(scroll bar move)
      or [Scroller.type],sc_scroll_bar_move

      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      jmp no_horizontal_scroller_active_2D
      no_horizontal_scroller_move_2D:

      no_horizontal_scroller_active_2D:

      ;if bit 7 is a true than redraw all scroller.

      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_redraw
      test al,al
      jz no_redraw_all_horizontal_scroller_2D

      and [Scroller.type],sc_redraw_off
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      call DrawButtonsOf2DHorizontalScroller

      no_redraw_all_horizontal_scroller_2D:

      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_scroll_bar_move
      test al,al
      jz no_move_horizontal_scroller_bar_2D

        xor ebx,ebx
        mov bx,[Scroller.y]
        mov eax,[ScrollBar.x]
        mov ecx,[Scrollersize]
        call DrawHorizontal2DScrollBar

        and [Scroller.type],sc_scroll_bar_move_off
        mov esi,dword Scroller
        mov edi,[PointerForScroller]
        add esi,control_header_size
        add edi,control_header_size
        mov ecx,sc_control_data_size
        rep movsb

      no_move_horizontal_scroller_bar_2D:

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov dx,16
      mov cx,[Scroller.length]
      mov si,[Scroller.mouseX]
      mov di,[Scroller.mouseY]
      add eax,17
      sub ecx,17+17
      call CheckCrossingBox

      cmp eax,0xffffff
      jne no_crossing_horizontal_scroller_2D_

      or [Scroller.type],sc_active
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,sc_control_data_size
      rep movsb

      jmp exit_crossing_horizontal_scroller_2D
      no_crossing_horizontal_scroller_2D_:

        ;check button of mouse
        mov eax,[ButtonsOfMouse]

        and eax,1b
        test al,al
        jnz mouse_button_pressed_horizontal_2D

          and [Scroller.type],sc_move_active_off
          mov esi,dword Scroller
          mov edi,[PointerForScroller]
          add esi,control_header_size
          add edi,control_header_size
          mov ecx,sc_control_data_size
          rep movsb
         mouse_button_pressed_horizontal_2D:

      exit_crossing_horizontal_scroller_2D:


      xor eax,eax
      mov al,[Scroller.type]
      and al,sc_active
      test al,al
      jz no_change_horizontal_scroller_2D

        xor eax,eax
        xor ebx,ebx
        mov ax,[Scroller.y]
        mov bx,[Scroller.mouseY]

        sub eax,ebx
        jns no_signum_1_x_2D
        neg eax
        no_signum_1_x_2D:

        cmp eax,110
        jle no_change_bits_horizontal_scroller_2D

        and [Scroller.type],sc_move_active_off
        no_change_bits_horizontal_scroller_2D:

        mov esi,dword Scroller
        mov edi,[PointerForScroller]
        add esi,control_header_size
        add edi,control_header_size
        mov ecx,sc_control_data_size
        rep movsb

      no_change_horizontal_scroller_2D:

      no_horizontal_scroller_2D:

      no_2D_scroller:

      ;checking to mouse events
      cmp [Message],dword 6
      jne no_send_message_to_child_buttons_of_scroller

      ;checking first child button
      mov eax,[Scroller.ChildButton1]
      mov ebx,[eax]

      push Message
      push eax
      call ebx

      mov eax,[Scroller.ChildButton1]

      xor ebx,ebx
      mov bl,[eax+45]
      and bl,bt_crossing
      test bl,bl
      jz no_crossing_first_child_button_of_scroller

         xor ebx,ebx
         mov bl,[eax+45]
         and bl,10b
         test bl,bl
         jz no_crossing_first_child_button_of_scroller

         or [Scroller.buttons_flags],sc_first_child_button_pressed
         or [Scroller.buttons_flags],10b

         xor eax,eax
         xor ebx,ebx
         mov ax,[Scroller.x]
         mov bx,[Scroller.y]
         mov [x],eax
         mov [y],ebx

         ;--------------------------------------
         ;---Checking stcroler to 2D or 3D types
         ;--------------------------------------
         xor eax,eax
         mov al,[Scroller.type]
         and al,sc_check_3D_type
         test al,al
         jnz type_3D_first_child_button_crossing

           ;if we here than scroller 2D type

           ;----------------------------------------------------
           ;---Chcecking scroller to horizontal or vertical types
           ;----------------------------------------------------

           call DrawPressedButtonOf2DScroller

           jmp else_type_3D_first_child_button_crossing

         type_3D_first_child_button_crossing:

           ;if we here than scroller 3D type

           call DrawPressedButtonOf3DScroller

         else_type_3D_first_child_button_crossing:

      jmp crossing_first_child_button_of_scroller
      no_crossing_first_child_button_of_scroller:

         and [Scroller.buttons_flags],sc_first_child_button_pressed_off

         xor eax,eax
         mov ax,[Scroller.buttons_flags]
         and ax,10b
         test ax,ax
         jz no_put_first_child_button

             ;--------------------------------------
             ;---Checking stcroler to 2D or 3D types
             ;--------------------------------------
             xor eax,eax
             mov al,[Scroller.type]
             and al,sc_check_3D_type
             test al,al
             jnz type_3D_first_child_button_not_crossing

                 ;if we here than scroller 2D type

                 ;----------------------------------------------------
                 ;---Chcecking scroller to horizontal or vertical types
                 ;----------------------------------------------------

                  xor eax,eax
                  mov al,[Scroller.type]
                  and al,sc_check_vertical
                  test al,al
                  jz type_horizontal_first_child_2D_button

                     ;put vertical first child button
                     call DrawButtonsOf2DVerticalScroller

                     jmp exit_else_type_horizontal_first_child_2D_button
                  type_horizontal_first_child_2D_button:

                     ;put horizontal first child button
                     call DrawButtonsOf2DVerticalScroller

                  exit_else_type_horizontal_first_child_2D_button:

                 jmp else_type_3D_first_child_button_not_crossing

             type_3D_first_child_button_not_crossing:

                  ;if we here than scroller 3D type

                 ;----------------------------------------------------
                 ;---Chcecking scroller to horizontal or vertical types
                 ;----------------------------------------------------

                  xor eax,eax
                  mov al,[Scroller.type]
                  and al,sc_check_vertical
                  test al,al
                  jz type_horizontal_first_child_3D_button

                     ;put vertical first child button
                     call DrawButtonsOfVertical3DScroller

                     jmp else_type_3D_first_child_button_not_crossing

                  type_horizontal_first_child_3D_button:

                     ;put horizontal first child button
                     call DrawButtonsOfHorizontal3DScroller

             else_type_3D_first_child_button_not_crossing:

             and [Scroller.buttons_flags],11111101b

         no_put_first_child_button:

      crossing_first_child_button_of_scroller:

      ;checking second child button
      mov eax,[Scroller.ChildButton2]
      mov ebx,[eax]

      push Message
      push eax
      call ebx

      mov eax,[Scroller.ChildButton2]

      xor ebx,ebx
      mov bl,[eax+45]
      and bl,bt_crossing
      test bl,bl
      jz no_crossing_second_child_button_of_scroller

         xor ebx,ebx
         mov bl,[eax+45]
         and bl,10b
         test bl,bl
         jz no_crossing_second_child_button_of_scroller

         or [Scroller.buttons_flags],sc_second_child_button_pressed
         or [Scroller.buttons_flags],100000b

         xor eax,eax
         xor ebx,ebx
         xor ecx,ecx
         mov ax,[Scroller.x]
         mov bx,[Scroller.y]
         mov [x],eax
         mov [y],ebx

         mov cx,[Scroller.length]
         sub ecx,16

         ;----------------------------------------------------
         ;---Chcecking scroller to horizontal or vertical types
         ;----------------------------------------------------
         xor eax,eax
         mov al,[Scroller.type]
         and al,sc_check_vertical
         test al,al
         jz calculate_pos_horizontal_second_child_button

            add [y],ecx

            jmp exit_else_calculate_pos_horizontal_second_child_button
         calculate_pos_horizontal_second_child_button:

            add [x],ecx

         exit_else_calculate_pos_horizontal_second_child_button:

         ;--------------------------------------
         ;---Checking stcroler to 2D or 3D types
         ;--------------------------------------
         xor eax,eax
         mov al,[Scroller.type]
         and al,sc_check_3D_type
         test al,al
         jnz type_3D_second_child_button_crossing

           ;if we here than scroller 2D type

           ;----------------------------------------------------
           ;---Chcecking scroller to horizontal or vertical types
           ;----------------------------------------------------

           call DrawPressedButtonOf2DScroller

           jmp else_type_3D_second_child_button_crossing

         type_3D_second_child_button_crossing:

           ;if we here than scroller 3D type

           call DrawPressedButtonOf3DScroller

         else_type_3D_second_child_button_crossing:

      jmp crossing_second_child_button_of_scroller
      no_crossing_second_child_button_of_scroller:

         and [Scroller.buttons_flags],sc_second_child_button_pressed_off

         xor eax,eax
         mov ax,[Scroller.buttons_flags]
         and ax,100000b
         test ax,ax
         jz no_put_second_child_button

             ;--------------------------------------
             ;---Checking stcroler to 2D or 3D types
             ;--------------------------------------
             xor eax,eax
             mov al,[Scroller.type]
             and al,sc_check_3D_type
             test al,al
             jnz type_3D_second_child_button_not_crossing

                 ;if we here than scroller 2D type

                 ;----------------------------------------------------
                 ;---Chcecking scroller to horizontal or vertical types
                 ;----------------------------------------------------

                  xor eax,eax
                  mov al,[Scroller.type]
                  and al,sc_check_vertical
                  test al,al
                  jz type_horizontal_second_child_2D_button

                     ;put vertical first child button
                     call DrawButtonsOf2DVerticalScroller

                     jmp exit_else_type_horizontal_second_child_2D_button
                  type_horizontal_second_child_2D_button:

                     ;put horizontal first child button
                     call DrawButtonsOf2DVerticalScroller

                  exit_else_type_horizontal_second_child_2D_button:

                 jmp else_type_3D_second_child_button_not_crossing

             type_3D_second_child_button_not_crossing:

                  ;if we here than scroller 3D type

                 ;----------------------------------------------------
                 ;---Chcecking scroller to horizontal or vertical types
                 ;----------------------------------------------------

                  xor eax,eax
                  mov al,[Scroller.type]
                  and al,sc_check_vertical
                  test al,al
                  jz type_horizontal_second_child_3D_button

                     ;put vertical first child button
                     call DrawButtonsOfVertical3DScroller

                     jmp else_type_3D_second_child_button_not_crossing

                  type_horizontal_second_child_3D_button:

                     ;put horizontal first child button
                     call DrawButtonsOfHorizontal3DScroller

             else_type_3D_second_child_button_not_crossing:

             and [Scroller.buttons_flags],11011111b

         no_put_second_child_button:

      crossing_second_child_button_of_scroller:

      no_send_message_to_child_buttons_of_scroller:

      exit_scroller:

      ;save resultat of work in control
      mov esi,dword Scroller
      mov edi,[PointerForScroller]
      mov ecx,sc_control_data_size
      add esi,control_header_size
      add edi,control_header_size
      rep movsb

      ret 8


DrawPressedButtonOf2DScroller:

      and [Line.color],0;eax

      ;draw horizontal up light line
      mov eax,[x]
      mov ebx,[y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      call DrawLine

      ;draw vertical up light line
      mov eax,[x]
      mov ebx,[y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],14
      call DrawLine

      ;mov eax,[colors_table2+8*3]
      ;and eax,0xffffff
      mov [Line.color],0xffffff ;eax

      ;draw vertical up dark line
      mov eax,[x]
      mov ebx,[y]
      add eax,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      call DrawLine

      ;draw horizontal up dark line
      mov eax,[x]
      mov ebx,[y]
      add ebx,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      call DrawLine

      ret

DrawPressedButtonOf3DScroller:

      and [Line.color],0;eax

      ;draw horizontal up light line
      mov eax,[x]
      mov ebx,[y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],16
      call DrawLine

      ;draw vertical up light line
      mov eax,[x]
      mov ebx,[y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],16
      call DrawLine

      ;mov eax,[colors_table2+8*3]
      ;and eax,0xffffff
      mov [Line.color],0xffffff ;eax

      ;draw vertical up dark line
      mov eax,[x]
      mov ebx,[y]
      add eax,16
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],16
      call DrawLine

      ;draw horizontal up dark line
      mov eax,[x]
      mov ebx,[y]
      add ebx,16
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],16
      call DrawLine

      ret

DrawButtonsOf2DHorizontalScroller:

       ;generate colors tables
      mov eax,[Scroller.color1]
      mov ebx,0xffffff
      mov ecx,20
      mov edx,dword colors_table1
      call gradient

      xor eax,eax
      mov ebx,[Scroller.color1]
      mov ecx,20
      mov edx,dword colors_table2
      call gradient

      ;draw vertical up rectangle
      xor ebx,ebx
      xor ecx,ecx
      mov eax,13
      mov bx,[Scroller.x]
      mov cx,[Scroller.y]
      shl ebx,16
      shl ecx,16
      add ebx,16
      add ecx,16
      mov edx,[Color]
      and edx,0xffffff
      mcall

      mov eax,[colors_table1+15*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw horizontal up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      call DrawLine

      ;draw vertical up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      call DrawLine

      mov eax,[colors_table2+8*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw vertical up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      call DrawLine

      ;draw horizontal up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],14
      call DrawLine

      and [Line.color],0

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],5
      add [Line.x2],9
      add [Line.y1],8
      add [Line.y2],4
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],6
      add [Line.x2],9
      add [Line.y1],8
      add [Line.y2],5
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],5
      add [Line.x2],9
      add [Line.y1],8
      add [Line.y2],12
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],6
      add [Line.x2],9
      add [Line.y1],8
      add [Line.y2],11
      call DrawLine

      ;draw vertical down rectangle
      xor ebx,ebx
      xor ecx,ecx
      mov eax,13
      mov edx,[Color]
      mov bx,[Scroller.x]
      mov cx,[Scroller.y]
      add bx,[Scroller.length]
      sub bx,16
      shl ebx,16
      shl ecx,16
      add ebx,16
      add ecx,16
      and edx,0xffffff
      mcall

      mov eax,[colors_table1+15*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw horizontal up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ax,[Scroller.length]
      sub eax,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      call DrawLine

      ;draw vertical up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ax,[Scroller.length]
      sub eax,16
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      call DrawLine

      mov eax,[colors_table2+8*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw vertical up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ax,[Scroller.length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      call DrawLine

      ;draw horizontal up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ax,[Scroller.length]
      sub eax,15
      add ebx,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      call DrawLine

      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,16
      mov [length],eax

      and [Line.color],0

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],12
      add [Line.x2],8
      add [Line.y1],8
      add [Line.y2],4
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],11
      add [Line.x2],8
      add [Line.y1],8
      add [Line.y2],5
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],12
      add [Line.x2],8
      add [Line.y1],8
      add [Line.y2],12
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],11
      add [Line.x2],8
      add [Line.y1],8
      add [Line.y2],11
      call DrawLine

      ret

DrawButtonsOf2DVerticalScroller:

      ;generate colors tables
      mov eax,[Scroller.color1]
      mov ebx,0xffffff
      mov ecx,20
      mov edx,dword colors_table1
      call gradient

      xor eax,eax
      mov ebx,[Scroller.color1]
      mov ecx,20
      mov edx,dword colors_table2
      call gradient

      ;draw vertical up rectangle
      mov eax,13
      mov edx,[Color]
      xor ebx,ebx
      xor ecx,ecx
      mov bx,[Scroller.x]
      mov cx,[Scroller.y]
      shl ebx,16
      shl ecx,16
      add ebx,16
      add ecx,16
      and edx,0xffffff
      mcall

      mov eax,[colors_table1+15*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw horizontal up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],14
      call DrawLine

      ;draw vertical up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],14
      call DrawLine

      mov eax,[colors_table2+8*3];[pointer]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw vertical up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      call DrawLine

      ;draw horizontal up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      call DrawLine

      and [Line.color],0

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],4
      add [Line.x2],8
      add [Line.y1],9
      add [Line.y2],5
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],4
      add [Line.x2],8
      add [Line.y1],10
      add [Line.y2],6
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],8
      add [Line.x2],12
      add [Line.y1],5
      add [Line.y2],9
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],8
      add [Line.x2],12
      add [Line.y1],6
      add [Line.y2],10
      call DrawLine

      ;draw vertical down rectangle
      xor ebx,ebx
      xor ecx,ecx
      mov eax,13
      mov edx,[Color]
      and edx,0xffffff
      mov bx,[Scroller.x]
      mov cx,[Scroller.y]
      add cx,[Scroller.length]
      sub cx,16
      shl ebx,16
      shl ecx,16
      add ebx,16
      add ecx,16
      mcall

      mov eax,[colors_table1+15*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw horizontal up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add bx,[Scroller.length]
      sub ebx,16
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      call DrawLine

      ;draw vertical up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add bx,[Scroller.length]
      sub ebx,16
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      call DrawLine

      mov eax,[colors_table2+8*3];[pointer]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw vertical up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add bx,[Scroller.length]
      sub ebx,16
      add eax,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      call DrawLine

      ;draw horizontal up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add bx,[Scroller.length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      call DrawLine

      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,16
      mov [length],eax

      and [Line.color],0

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],4
      add [Line.x2],8
      add [Line.y1],7
      add [Line.y2],11
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],4
      add [Line.x2],8
      add [Line.y1],6
      add [Line.y2],10
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],12
      add [Line.x2],8
      add [Line.y1],7
      add [Line.y2],11
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],12
      add [Line.x2],8
      add [Line.y1],6
      add [Line.y2],10
      call DrawLine

      ret

DrawButtonsOfHorizontal3DScroller:

       ;draw vertical up rectangle
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Rectangle.x],eax
      mov [Rectangle.y],ebx
      mov [Rectangle.width],16
      mov [Rectangle.height],16

      mov eax,[pointer]
      and eax,0xffffff
      mov [Rectangle.color],eax
      call RectangleContour

      ;draw gradient right
      mov eax,[Scroller.color1]
      mov ebx,dword 0xffffff
      and eax,0xffffff
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov cx,[Scroller.x]
      mov dx,[Scroller.y]
      mov si,(16-1)
      mov di,(16-2)
      add cx,1
      add dx,1
      call rectangle_gradient_up

      mov eax,[pointer+10*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw horizontal up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],14
      call DrawLine

      ;draw vertical up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],14
      call DrawLine

      mov eax,[Color]
      mov [Line.color],eax

      ;draw vertical up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,16
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],1
      add [Line.y2],14
      call DrawLine

      ;draw horizontal up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],14
      call DrawLine

      xor eax,eax
      mov eax,[AveregeColor]
      mov [Pixel.color],eax

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      mov ax,[Scroller.x]
      add eax,16
      mov [Pixel.x],eax
      call DrawPixel

      xor eax,eax
      mov ax,[Scroller.x]
      mov [Pixel.x],eax
      add [Pixel.y],16
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,16
      add ebx,16
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;----------------
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      add ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,15
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      add ebx,15
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov [Line.color],0

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],5
      add [Line.x2],9
      add [Line.y1],8
      add [Line.y2],4
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],6
      add [Line.x2],9
      add [Line.y1],8
      add [Line.y2],5
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],5
      add [Line.x2],9
      add [Line.y1],8
      add [Line.y2],12
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],6
      add [Line.x2],9
      add [Line.y1],8
      add [Line.y2],11
      call DrawLine

      ;draw vertical down rectangle
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ax,[Scroller.length]
      sub ax,16
      mov [Rectangle.x],eax
      mov [Rectangle.y],ebx
      mov [Rectangle.width],16
      mov [Rectangle.height],16

      mov eax,[pointer]
      and eax,0xffffff
      mov [Rectangle.color],eax
      call RectangleContour

      ;draw gradient right
      mov eax,[Scroller.color1]
      mov ebx,dword 0xffffff
      and eax,0xffffff
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov cx,[Scroller.x]
      mov dx,[Scroller.y]
      add cx,[Scroller.length]
      sub cx,16
      mov si,(16-1)
      mov di,(16-2)
      add cx,1
      add dx,1
      call rectangle_gradient_up

      mov eax,[pointer+10*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw horizontal up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ax,[Scroller.length]
      sub eax,15
      add ebx,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      call DrawLine

      ;draw vertical up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,1
      add ax,[Scroller.length]
      sub eax,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],14
      call DrawLine

      mov eax,[Color];[pointer]
      mov [Line.color],eax

      ;draw vertical up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ax,[Scroller.length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      call DrawLine

      ;draw horizontal up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ax,[Scroller.length]
      sub eax,15
      add ebx,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],14
      call DrawLine

      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,16
      mov [length],eax

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      add [Pixel.x],16
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,16
      add eax,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,16
      add ebx,16
      add eax,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;----------------
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,1
      add eax,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      add ebx,1
      add eax,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,15
      add eax,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      add ebx,15
      add eax,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,16
      mov [length],eax

      and [Line.color],0

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],12
      add [Line.x2],8
      add [Line.y1],8
      add [Line.y2],4
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],11
      add [Line.x2],8
      add [Line.y1],8
      add [Line.y2],5
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],12
      add [Line.x2],8
      add [Line.y1],8
      add [Line.y2],12
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],11
      add [Line.x2],8
      add [Line.y1],8
      add [Line.y2],11
      call DrawLine

      ret

DrawButtonsOfVertical3DScroller:

       ;draw vertical up rectangle
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Rectangle.x],eax
      mov [Rectangle.y],ebx
      mov [Rectangle.width],16
      mov [Rectangle.height],16

      mov eax,[pointer]
      and eax,0xffffff
      mov [Rectangle.color],eax
      call RectangleContour

      ;draw gradient right
      mov eax,[Scroller.color1]
      mov ebx,dword 0xffffff
      and eax,0xffffff
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov cx,[Scroller.x]
      mov dx,[Scroller.y]
      mov si,(16-1)
      mov di,(16-2)
      add cx,1
      add dx,1
      call rectangle_gradient_right

      mov eax,[pointer+10*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw horizontal up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],14
      call DrawLine

      ;draw vertical up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],14
      call DrawLine

      mov eax,[Color]
      mov [Line.color],eax

      ;draw vertical up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],1
      add [Line.y2],14
      call DrawLine

      ;draw horizontal up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],14
      call DrawLine

      xor eax,eax
      mov eax,[AveregeColor]
      mov [Pixel.color],eax

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      mov ax,[Scroller.x]
      add eax,16
      mov [Pixel.x],eax
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,16
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,16
      add ebx,16
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;----------------
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      add ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,15
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      add ebx,15
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      and [Line.color],0

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],4
      add [Line.x2],8
      add [Line.y1],9
      add [Line.y2],5
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],4
      add [Line.x2],8
      add [Line.y1],10
      add [Line.y2],6
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],8
      add [Line.x2],12
      add [Line.y1],5
      add [Line.y2],9
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],8
      add [Line.x2],12
      add [Line.y1],6
      add [Line.y2],10
      call DrawLine

      ;draw vertical down rectangle
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add bx,[Scroller.length]
      sub bx,16
      mov [Rectangle.x],eax
      mov [Rectangle.y],ebx
      mov [Rectangle.width],16
      mov [Rectangle.height],16

      mov eax,[pointer]
      and eax,0xffffff
      mov [Rectangle.color],eax
      call RectangleContour

      ;draw gradient right
      mov eax,[Scroller.color1]
      mov ebx,dword 0xffffff
      and eax,0xffffff
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov cx,[Scroller.x]
      mov dx,[Scroller.y]
      add dx,[Scroller.length]
      sub dx,(16-1)
      mov si,(16-1)
      mov di,(16-2)
      add cx,1
      call rectangle_gradient_right

      mov eax,[pointer+10*3]
      and eax,0xffffff
      mov [Line.color],eax

      ;draw horizontal up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add bx,[Scroller.length]
      sub ebx,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],1
      add [Line.x2],15
      call DrawLine

      ;draw vertical up light line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add bx,[Scroller.length]
      sub ebx,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],14
      call DrawLine

      mov eax,[Color];[pointer]
      mov [Line.color],eax

      ;draw vertical up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add bx,[Scroller.length]
      sub ebx,15
      add eax,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],14
      call DrawLine

      ;draw horizontal up dark line
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add bx,[Scroller.length]
      add eax,1
      sub ebx,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],14
      call DrawLine

      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,16
      mov [length],eax

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      mov ax,[Scroller.x]
      add eax,16
      mov [Pixel.x],eax
      call DrawPixel

      xor eax,eax
      mov ax,[Scroller.x]
      mov [Pixel.x],eax
      add [Pixel.y],16
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,16
      add ebx,16
      add ebx,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;----------------
      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,1
      add ebx,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      add ebx,1
      add ebx,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,1
      add ebx,15
      add ebx,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add eax,15
      add ebx,15
      add ebx,[length]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      xor eax,eax
      mov ax,[Scroller.length]
      sub eax,16
      mov [length],eax

      and [Line.color],0

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],4
      add [Line.x2],8
      add [Line.y1],7
      add [Line.y2],11
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],4
      add [Line.x2],8
      add [Line.y1],6
      add [Line.y2],10
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],12
      add [Line.x2],8
      add [Line.y1],7
      add [Line.y2],11
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      mov ax,[Scroller.x]
      mov bx,[Scroller.y]
      add ebx,[length]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],12
      add [Line.x2],8
      add [Line.y1],6
      add [Line.y2],10
      call DrawLine

      ret

;eax - x
;ebx - y
;ecx - size y in pixels
DrawVertical3DScrollBar:

      mov [x],eax
      mov [y],ebx
      mov [length2],ecx

      mov eax,[x]
      mov ebx,[y]
      mov ecx,[length2]
      mov edx,[pointer]
      and edx,0xffffff
      mov [Rectangle.x],eax
      mov [Rectangle.y],ebx
      mov [Rectangle.width],16
      mov [Rectangle.height],ecx
      mov [Rectangle.color],edx
      call RectangleContour

      ;draw gradient right
      mov eax,[Scroller.color1]
      mov ebx,dword 0xffffff
      and eax,0xffffff
      xor esi,esi
      xor edi,edi
      mov ecx,[x]
      mov edx,[y]
      add edx,1
      mov si,(16-1)
      mov edi,[length2]
      sub edi,2
      call rectangle_gradient_right

      ;------------------
      xor eax,eax
      mov eax,[AveregeColor]
      mov [Pixel.color],eax

      mov eax,[x]
      mov ebx,[y]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      add eax,16
      mov [Pixel.x],eax
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add ebx,[length2]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add eax,16
      add ebx,[length2]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add eax,1
      add ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add eax,15
      add ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add eax,1
      add ebx,[length2]
      sub ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add eax,16
      add ebx,[length2]
      sub eax,1
      sub ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      cmp [length2],12
      jl no_draw_reliev_vertical_3D

       mov ebx,[x]
       mov eax,[length2]
       shr eax,1
       add eax,[y]
       add ebx,5
       sub eax,3

       mov [Line.x1],ebx
       mov [Line.x2],ebx
       mov [Line.y1],eax
       mov [Line.y2],eax
       mov [Line.color],0xffffff
       add [Line.x2],6

       mov esi,4
       next_light_line_draw_r_vertical_3D:

         call DrawLine
         add [Line.y1],2
         add [Line.y2],2

       dec esi
       jnz  next_light_line_draw_r_vertical_3D

       mov ebx,[x]
       mov eax,[length2]
       shr eax,1
       add eax,[y]
       add ebx,6
       sub eax,2

       mov ecx,[Scroller.color1]
       mov [Line.x1],ebx
       mov [Line.x2],ebx
       mov [Line.y1],eax
       mov [Line.y2],eax
       mov [Line.color],ecx
       add [Line.x2],6

       mov esi,4
       next_dark_line_draw_r_vertical_3D:

         call DrawLine
         add [Line.y1],2
         add [Line.y2],2

       dec esi
       jnz  next_dark_line_draw_r_vertical_3D

      no_draw_reliev_vertical_3D:


      xor eax,eax
      mov ax,[Scroller.y]
      mov ebx,[y]
      add eax,17

      sub ebx,eax
      jz no_draw_up_rectangle_vertical_3D
      js no_draw_up_rectangle_vertical_3D

      sub ebx,1

      mov edi,ebx

      mov eax,0xe4e4e4
      mov ebx,0xffffff
      xor edx,edx
      mov ecx,[x]
      mov dx,[Scroller.y]
      add edx,17
      sub ecx,1
      mov esi,17
      call rectangle_gradient_right
      no_draw_up_rectangle_vertical_3D:

      xor eax,eax
      mov ax,[Scroller.y]
      add ax,[Scroller.length]
      mov ebx,[y]
      add ebx,[length2]
      sub eax,17

      sub eax,ebx
      jz no_draw_down_rectangle_vertical_3D
      js no_draw_down_rectangle_vertical_3D

      sub eax,1
      mov edi,eax

      mov eax,0xe4e4e4
      mov ebx,0xffffff
      mov ecx,[x]
      xor edx,edx
      mov edx,[y]
      add edx,[length2]
      add edx,1
      sub ecx,1
      mov esi,17
      call rectangle_gradient_right

      no_draw_down_rectangle_vertical_3D:

      ret

;eax - x
;ebx - y
;ecx - size y in pixels
DrawHorizontal3DScrollBar:

      mov [x],eax
      mov [y],ebx
      mov [length2],ecx

      mov eax,[x]
      mov ebx,[y]
      mov ecx,[length2]
      mov edx,[pointer]
      and edx,0xffffff
      mov [Rectangle.x],eax
      mov [Rectangle.y],ebx
      mov [Rectangle.width],ecx
      mov [Rectangle.height],16
      mov [Rectangle.color],edx
      call RectangleContour

      ;draw gradient right
      mov eax,[Scroller.color1]
      mov ebx,dword 0xffffff
      and eax,0xffffff
      xor esi,esi
      xor edi,edi
      mov ecx,[x]
      mov edx,[y]
      add ecx,1
      mov edi,(16-1)
      mov esi,[length2]
      sub esi,2
      call rectangle_gradient_up

      ;------------------
      xor eax,eax
      mov eax,[AveregeColor]
      mov [Pixel.color],eax

      mov eax,[x]
      mov ebx,[y]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov ebx,[y]
      add ebx,16
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add eax,[length2]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add ebx,16
      add eax,[length2]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add eax,1
      add ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add ebx,15
      add eax,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add ebx,1
      add eax,[length2]
      sub eax,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      mov eax,[x]
      mov ebx,[y]
      add ebx,15
      add eax,[length2]
      sub eax,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      cmp [length2],12
      jl no_draw_reliev_horizontal_3D

       mov ebx,[y]
       mov eax,[length2]
       shr eax,1
       add eax,[x]
       add ebx,5
       sub eax,3

       mov [Line.x1],eax
       mov [Line.x2],eax
       mov [Line.y1],ebx
       mov [Line.y2],ebx
       mov [Line.color],0xffffff
       add [Line.y2],6

       mov esi,4
       next_light_line_draw_r_horizontal_3D:

         call DrawLine
         add [Line.x1],2
         add [Line.x2],2

       dec esi
       jnz  next_light_line_draw_r_horizontal_3D

       mov ebx,[y]
       mov eax,[length2]
       shr eax,1
       add eax,[x]
       add ebx,6
       sub eax,2

       mov ecx,[Scroller.color1]
       mov [Line.x1],eax
       mov [Line.x2],eax
       mov [Line.y1],ebx
       mov [Line.y2],ebx
       mov [Line.color],ecx
       add [Line.y2],6

       mov esi,4
       next_dark_line_draw_r_horizontal_3D:

         call DrawLine
         add [Line.x1],2
         add [Line.x2],2

       dec esi
       jnz  next_dark_line_draw_r_horizontal_3D

      no_draw_reliev_horizontal_3D:


      xor eax,eax
      mov ax,[Scroller.x]
      mov ebx,[x]
      add eax,17

      sub ebx,eax
      jz no_draw_up_rectangle_horizontal_3D
      js no_draw_up_rectangle_horizontal_3D

      sub ebx,1

      mov esi,ebx

      mov eax,0xe4e4e4
      mov ebx,0xffffff
      xor ecx,ecx
      mov edx,[y]
      mov cx,[Scroller.x]
      add ecx,17
      sub edx,1
      mov edi,17
      call rectangle_gradient_up
      no_draw_up_rectangle_horizontal_3D:

      xor eax,eax
      mov ax,[Scroller.x]
      add ax,[Scroller.length]
      mov ebx,[x]
      add ebx,[length2]
      sub eax,17

      sub eax,ebx
      jz no_draw_down_rectangle_horizontal_3D
      js no_draw_down_rectangle_horizontal_3D

      sub eax,1
      mov esi,eax

      mov eax,0xe4e4e4
      mov ebx,0xffffff
      mov edx,[y]
      xor ecx,ecx
      mov ecx,[x]
      add ecx,[length2]
      add ecx,1
      sub edx,1
      mov edi,17
      call rectangle_gradient_up

      no_draw_down_rectangle_horizontal_3D:

      ret

;eax - x
;ebx - y
;ecx - size y in pixels
DrawVertical2DScrollBar:

      mov [x],eax
      mov [y],ebx
      mov [length2],ecx

      mov eax,13
      mov edx,[Color]
      mov ebx,[x]
      mov ecx,[y]
      shl ebx,16
      shl ecx,16
      add ebx,16
      add ecx,[length2]
      and edx,0xffffff
      mcall

     ;draw horizontal up light line
      mov eax,[x]
      mov ebx,[y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      mov ecx,[colors_table1+15*3]
      and ecx,0xffffff
      mov [Line.color],ecx
      call DrawLine

      ;draw vertical up light line
      mov eax,[x]
      mov ebx,[y]
      mov ecx,[length2]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      add [Line.y2],ecx
      call DrawLine

      ;draw vertical up dark line
      mov eax,[x]
      mov ebx,[y]
      add eax,15
      mov ecx,[length2]
      sub ecx,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],ecx
      mov ecx,[colors_table2+8*3];[pointer]
      and ecx,0xffffff
      mov [Line.color],ecx
      call DrawLine

      ;draw horizontal up dark line
      mov eax,[x]
      mov ebx,[y]
      mov ecx,[length2]
      add ebx,ecx
      sub ebx,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],15
      call DrawLine


      xor eax,eax
      mov ax,[Scroller.y]
      mov ebx,[y]
      add eax,16

      sub ebx,eax
      jz no_draw_up_rectangle_vertical_2D
      js no_draw_up_rectangle_vertical_2D

      mov edi,ebx

      xor ecx,ecx
      mov eax,13
      mov edx,0xe4e4e4
      mov ebx,[x]
      mov cx,[Scroller.y]
      add ecx,16
      shl ebx,16
      shl ecx,16
      add ebx,16
      add ecx,edi
      mcall

      no_draw_up_rectangle_vertical_2D:

      xor eax,eax
      mov ax,[Scroller.y]
      add ax,[Scroller.length]
      mov ebx,[y]
      add ebx,[length2]
      sub eax,16

      sub eax,ebx
      jz no_draw_down_rectangle_vertical_2D
      js no_draw_down_rectangle_vertical_2D

      ;sub eax,1
      mov edi,eax

      xor ecx,ecx
      mov eax,13
      mov edx,0xe4e4e4
      mov ebx,[x]
      mov ecx,[y]
      add ecx,[length2]
      shl ebx,16
      shl ecx,16
      add ebx,16
      add ecx,edi
      mcall

      no_draw_down_rectangle_vertical_2D:

      ret

;eax - x
;ebx - y
;ecx - size x in pixels
DrawHorizontal2DScrollBar:

      mov [x],eax
      mov [y],ebx
      mov [length2],ecx

      mov eax,13
      mov edx,[Color]
      mov ebx,[x]
      mov ecx,[y]
      shl ebx,16
      shl ecx,16
      add ebx,[length2]
      add ecx,16
      and edx,0xffffff
      mcall

     ;draw horizontal up light line
      mov eax,[x]
      mov ebx,[y]
      mov ecx,[length2]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      sub ecx,1
      add [Line.x2],ecx
      mov ecx,[colors_table1+15*3]
      and ecx,0xffffff
      mov [Line.color],ecx
      call DrawLine

      ;draw vertical up light line
      mov eax,[x]
      mov ebx,[y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      call DrawLine

      ;draw vertical up dark line
      mov eax,[x]
      mov ebx,[y]
      mov ecx,[length2]
      sub ecx,1
      add eax,ecx
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],15
      mov ecx,[colors_table2+8*3];[pointer]
      and ecx,0xffffff
      mov [Line.color],ecx
      call DrawLine

      ;draw horizontal up dark line
      mov eax,[x]
      mov ebx,[y]
      mov ecx,[length2]
      sub ecx,1
      add ebx,15
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],ecx
      call DrawLine


      xor eax,eax
      mov ax,[Scroller.x]
      mov ebx,[x]
      add eax,16

      sub ebx,eax
      jz no_draw_up_rectangle_horizontal_2D
      js no_draw_up_rectangle_horizontal_2D

      mov edi,ebx

      xor ecx,ecx
      xor ebx,ebx
      mov eax,13
      mov edx,0xe4e4e4
      mov bx,[Scroller.x]
      mov cx,[Scroller.y]
      add ebx,16
      shl ebx,16
      shl ecx,16
      add ecx,16
      add ebx,edi
      mcall

      no_draw_up_rectangle_horizontal_2D:

      xor eax,eax
      mov ax,[Scroller.x]
      add ax,[Scroller.length]
      mov ebx,[x]
      add ebx,[length2]
      sub eax,16

      sub eax,ebx
      jz no_draw_down_rectangle_horizontal_2D
      js no_draw_down_rectangle_horizontal_2D

      ;sub eax,1
      mov edi,eax

      xor ecx,ecx
      mov eax,13
      mov edx,0xe4e4e4
      mov ebx,[x]
      mov ecx,[y]
      add ebx,[length2]
      shl ebx,16
      shl ecx,16
      add ebx,edi
      add ecx,16
      mcall

      no_draw_down_rectangle_horizontal_2D:

      ret
