
;button's types
bt_check_text_type                 = 1b
bt_check_picture_type              = 10b
bt_check_picture_text_type         = 100b
bt_no_draw                         = 1000b
bt_check_3D_type                   = 10000b
bt_check_full_redraw               = 10000000b
bt_full_redraw_off                 = 1111111b

;button's flags
bt_crossing                        = 1b
bt_crossing_pressing               = 11b
bt_crossing_pressing_off           = 11111100b

bt_control_data_size               = 50

;****************************************************
;******************craete Button**********************
;****************************************************
;IN
;pointer to parend
;pointer to Buttton's structure
;OUT
;pointer to initialized Button's structure
align 4

craete_button:

      mov ebx,[esp+4] ;parend
      mov eax,[esp+8] ;control

      mov [PointerToStructureForButton],eax

      mov eax,control_header_size+bt_control_data_size
      call craete_control

      ;set all button's parameters in control
      mov ecx,dword button
      mov [eax],dword button

      mov ecx,bt_control_data_size
      mov esi,[PointerToStructureForButton]
      mov edi,eax
      add edi,control_header_size
      rep movsb

      mov ebx,[PointerToStructureForButton]
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov cx,[ebx+2]   ;button x
      mov dx,[ebx+4]   ;button y
      mov si,[ebx+6]   ;button size x
      mov di,[ebx+8]   ;button size y

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
;*******************Draw Button**********************
;****************************************************
;IN
;pointer to control of button
;message
;OUT
;not returned value
align 4

button:

      ;get message
      mov eax,[esp+8]

      ;get pointer to control of button
      mov esi,[esp+4]
      mov [PointerForButton],esi
      ;copy control to local control
      mov edi,dword Button
      mov ecx,control_header_size+bt_control_data_size
      cld
      rep movsb


      cmp [eax],dword 1
      jne no_redraw_all_button

      or [Button.type],bt_check_full_redraw
      no_redraw_all_button:

      cmp [eax],dword 2
      jne no_keys_button

      jmp exit_button
      no_keys_button:

      cmp [eax],dword 3
      jne no_pressed_button_button

      jmp exit_button
      no_pressed_button_button:

      cmp [eax],dword 6
      jne no_mouse_button

      mov esi,[eax+4]
      mov edi,[eax+8]
      mov ecx,[eax+12]
      mov [Button.mouseX],si
      mov [Button.mouseY],di
      mov [ButtonsOfMouse],ecx

      no_mouse_button:


      cmp [eax],dword -1
      jne no_delete_button

      jmp exit_button
      no_delete_button:

      ;load coordinats and size from control
      mov eax,[PointerForButton]
      mov ebx,[eax+24]           ;x
      mov ecx,[eax+28]           ;y
      mov edx,[eax+32]           ;size x
      mov esi,[eax+36]           ;size y
      ;set current coordinats and sizes in zakladka
      mov [Button.x],bx
      mov [Button.y],cx
      mov [Button.width],dx
      mov [Button.height],si


      mov [Button.color2],0xffffff

      xor eax,eax
      mov al,[Button.type]
      and al,bt_check_full_redraw
      test al,al
      jz  no_redraw_button

      and [Button.type],bt_full_redraw_off

      mov esi,dword Button
      mov edi,[PointerForButton]
      add esi,control_header_size
      add edi,control_header_size
      mov ecx,2
      cld
      rep movsb

      ;if bit 3 is tru than no draw button
      mov al,[Button.type]
      and al,bt_no_draw
      test al,al
      jnz  no_draw_button

      ;generate colors tables
      mov eax,[Button.color1]
      mov ebx,0xffffff
      mov ecx,20
      mov edx,dword colors_table1
      call gradient

      xor eax,eax
      mov ebx,[Button.color1]
      mov ecx,20
      mov edx,dword colors_table2
      call gradient

      xor eax,eax
      mov al,[Button.type]

      and al,bt_check_3D_type
      test al,al
      jz no_3d_button

      ;draw gradient up rectangle
      mov eax,[Button.color1]
      mov ebx,[Button.color2]
      and eax,0xffffff
      and ebx,0xffffff
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov cx,[Button.x]
      mov dx,[Button.y]
      mov si,[Button.width]
      mov di,[Button.height]
      add cx,1
      add dx,2
      sub si,1
      sub di,3
      call rectangle_gradient_up

      mov eax,[Button.color1]
      mov [Line.color],eax

      ;draw left vertical line
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],3
      add [Line.y2],ecx
      call DrawLine

      ;draw right vertical line
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,esi
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],3
      add [Line.y2],ecx
      call DrawLine

      ;draw down 1 horizontal line
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add ebx,ecx
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],1
      sub esi,1
      add [Line.x2],esi
      call DrawLine

      ;draw up light horizontal rectangle
      xor ecx,ecx
      xor esi,esi
      mov si,[Button.height]
      mov di,[Button.width]
      mov eax,13
      mov bx,[Button.x]
      mov cx,[Button.y]
      mov edx,0xffffff
      add ebx,2
      add ecx,1
      shl ebx,16
      shl ecx,16
      sub edi,3
      add ebx,edi
      add ecx,2
      mcall

      ;draw left light vertical line
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]

      ;calculate color
      mov dx,[Button.height]
      sub edx,3+2
      mov esi,[pointer+edx*3]
      and esi,0xffffff
      add eax,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      mov [Line.color],esi; dword 0xffffff
      add [Line.y1],2
      sub ecx,3
      add [Line.y2],ecx
      call DrawLine

      ;draw right light vertical line
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]

      add ax,[Button.width]
      sub eax,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],2
      sub ecx,3
      add [Line.y2],ecx
      call DrawLine

      mov eax,[pointer+3*3]
      and eax,0xffffff
      mov [Button.color2],eax
      mov [Line.color],eax

      ;draw up light left vertical line 1
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],2
      call DrawLine

      ;draw up light right vertical line 1
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,esi
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],2
      call DrawLine

      ;draw up light left vertical line 2
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],1
      call DrawLine

      ;draw up light right vertical line 2
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,esi
      sub eax,1
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],1
      call DrawLine

      ;draw up light horizontal line
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],2
      sub esi,2
      add [Line.x2],esi
      call DrawLine

      mov eax,[pointer+3*3]
      mov ebx,[pointer+3*9]
      and eax,0xffffff
      and ebx,0xffffff
      call calculate_average_color

      mov [Pixel.color],eax

      ;(x,y)
      xor eax,eax
      xor ebx,ebx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;(x+size_x,y)
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.width]
      add eax,ecx
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;(x+1,y+1)
      xor eax,eax
      xor ebx,ebx
      mov ax,[Button.x]
      mov bx,[Button.y]
      add eax,1
      add ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;(x+size_x-1,y+1)
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.width]
      add eax,ecx
      sub eax,1
      add ebx,1
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;(x,y+size_y)
      xor eax,eax
      xor ebx,ebx
      mov ax,[Button.x]
      mov bx,[Button.y]
      add bx,[Button.height]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;(x+size_x,y+size_y)
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.width]
      add eax,ecx
      add bx,[Button.height]
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;(x+1,y+size_y-2)
      xor eax,eax
      xor ebx,ebx
      mov ax,[Button.x]
      mov bx,[Button.y]
      add eax,1
      add bx,[Button.height]
      sub ebx,2
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      ;(x+size_x-1,y+size_y-2)
      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.width]
      add eax,ecx
      sub eax,1
      add bx,[Button.height]
      sub ebx,2
      mov [Pixel.x],eax
      mov [Pixel.y],ebx
      call DrawPixel

      no_3d_button:

      ;****************************
      ;*********2D button**********
      ;****************************
      xor eax,eax
      mov al,[Button.type]
      and al,bt_check_3D_type
      test al,al
      jnz no_2D_button

      mov edx,[colors_table1+18]
      and edx,0xffffff

      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      xor edi,edi
      mov si,[Button.width]
      mov di,[Button.height]
      mov eax,13
      mov bx,[Button.x]
      mov cx,[Button.y]
      add ebx,1
      add ecx,1
      sub si,1
      sub di,1
      shl ebx,16
      shl ecx,16
      add ebx,esi
      add ecx,edi
      mcall

      mov eax,[colors_table1+15*3]
      and eax,0xffffff
      mov [Line.color],eax

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov si,[Button.width]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],esi
      call DrawLine

      mov eax,[colors_table2+8*3]
      and eax,0xffffff
      mov [Line.color],eax

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,esi
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add ebx,ecx
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],esi
      call DrawLine

      no_2D_button:

      no_draw_button:

      ;check drawing text on the button
      xor eax,eax
      mov al,[Button.type]

      and al,bt_check_text_type
      test al,al
      jz no_button_only_with_text

       call DrawStringOnTheButton
       jmp end_case_button
      no_button_only_with_text:

      ;check drawing picture on the button
      xor eax,eax
      mov al,[Button.type]
      and al,bt_check_picture_type
      test al,al
      jz no_button_only_with_picture

        call DrawPictureOnTheButton
        jmp end_case_button
      no_button_only_with_picture:


      xor eax,eax
      mov al,[Button.type]
      and al,bt_check_picture_text_type
      test al,al
      jz no_button_with_text_and_picture

      ;draw string in coordinats (textX,textY)
      xor eax,eax
      xor ebx,ebx
      mov ax,[Button.textX]
      mov bx,[Button.textY]
      add ax,[Button.x]
      add bx,[Button.y]
      mov [Font.x],eax
      mov [Font.y],ebx
      call DrawString

      ;draw picture in coordinats (imageX,imageY)
      xor eax,eax
      xor ebx,ebx
      mov ax,[Button.imageX]
      mov bx,[Button.imageY]
      add ax,[Button.x]
      add bx,[Button.y]
      mov [xo],eax
      mov [x],eax
      mov [y],ebx

      xor esi,esi
      xor edi,edi
      mov si,[Button.imageSizeX]
      mov di,[Button.imageSizeY]
      mov ebp,[Button.image]

      xor eax,eax
      inc eax

        loop_y_image_:

          mov ebx,[xo]
          mov [x],ebx

          xor esi,esi
          mov si,[Button.imageSizeX]

          loop_x_image_:

             mov edx,[ebp]
             and edx,0xffffff
             cmp edx,[Button.transparentColor]
             je no_draw_pixel_

               mov ebx,[x]
               mov ecx,[y]
               mcall
             no_draw_pixel_:

             add ebp,3
             add [x],1
          dec esi
          jnz  loop_x_image_

          add [y],1
        dec edi
        jnz loop_y_image_

      no_button_with_text_and_picture:

      no_redraw_button:

      end_case_button:

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor edx,edx
      xor esi,esi
      xor edi,edi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.width]
      mov dx,[Button.height]
      mov si,[Button.mouseX]
      mov di,[Button.mouseY]
      call CheckCrossingBox

      cmp eax,0xffffff
      jne no_crossing

        xor ebx,ebx
        mov bl,[Button.type]
        and bl,bt_check_3D_type
        test bl,bl
        jz button_active_2D

         push eax
           mov eax,[ButtonsOfMouse]

           cmp eax,1
           je button_pressed_3d_

             xor eax,eax
             mov al,[Button.type]
             and al,bt_no_draw
             test al,al
             jnz no_draw_active_button_3d_

               call Draw3DActiveButton

             no_draw_active_button_3d_:

             mov [Button.flag],byte bt_crossing
           jmp exit_button_pressed_3d_

           button_pressed_3d_:

           xor eax,eax
           mov al,[Button.type]
           and al,bt_no_draw
           test al,al
           jnz no_draw_pressed_button_3d_

            call DrawPressed3DButton

           no_draw_pressed_button_3d_:

           mov [Button.flag],byte bt_crossing_pressing

           exit_button_pressed_3d_:

         pop eax

         ;save flag
         mov esi,dword Button
         mov edi,[PointerForButton]
         add esi,control_header_size
         add edi,control_header_size
         mov ecx,2
         cld
         rep movsb
         jmp no_crossing

        button_active_2D:

         push eax
           mov eax,[ButtonsOfMouse]

           cmp eax,1
           je button_pressed_2d

           xor eax,eax
           mov al,[Button.type]
           and al,bt_no_draw
           test al,al
           jnz no_active_button_2d_

            call Draw2DActiveButton

           no_active_button_2d_:

           mov [Button.flag],byte bt_crossing
           jmp exit_button_pressed_2d
           button_pressed_2d:

           xor eax,eax
           mov al,[Button.type]
           and al,bt_no_draw
           test al,al
           jnz no_draw_pressed_button_2d_

            call DrawPressed2DButton

           no_draw_pressed_button_2d_:

           mov [Button.flag],byte bt_crossing_pressing
           exit_button_pressed_2d:

         pop eax

         ;save flag
         mov esi,dword Button
         mov edi,[PointerForButton]
         add esi,control_header_size
         add edi,control_header_size
         mov ecx,2
         cld
         rep movsb
      no_crossing:

      cmp eax,0xffffff
      je yes_crossing
      xor ebx,ebx
      mov bl,[Button.flag]
      and bl,bt_crossing
      test bl,bl
      jz yes_crossing

        xor ebx,ebx
        mov bl,[Button.type]
        and bl,bt_check_3D_type
        test bl,bl
        jz button_put_2D

         push eax

         xor eax,eax
         mov al,[Button.type]
         and al,bt_no_draw
         test al,al
         jnz no_draw_button_active_3d__

          call Put3DActiveButton
         no_draw_button_active_3d__:

         pop eax

         and [Button.flag],bt_crossing_pressing_off
         mov esi,dword Button
         mov edi,[PointerForButton]
         add edi,control_header_size
         add esi,control_header_size
         mov ecx,2
         cld
         rep movsb
         jmp yes_crossing

        button_put_2D:

        push eax

        xor eax,eax
        mov al,[Button.type]
        and al,bt_no_draw
        test al,al
        jnz no_draw_button_active_2d__

         call Put2DActiveButton

        no_draw_button_active_2d__:

        pop eax

        and [Button.flag],bt_crossing_pressing_off
        mov esi,dword Button
        mov edi,[PointerForButton]
        add esi,control_header_size
        add edi,control_header_size
        mov ecx,2
        cld
        rep movsb
      yes_crossing:

      exit_button:

      ret 8

DrawStringOnTheButton:

      ;xor ebx,ebx
      mov ebx,[Button.text]
      call GetLengthString
      imul eax,6  ;[Font.sizeX]
      xor ebx,ebx
      mov bx,[Button.width]
      shr eax,1
      shr ebx,1
      sub ebx,eax
      xor eax,eax
      mov ax,[Button.x]
      add eax,ebx
      mov [Font.x],eax
      xor ebx,ebx
      mov bx,[Button.height]
      shr ebx,1
      sub ebx,3;[Font.sizeY]
      xor eax,eax
      mov ax,[Button.y]
      add eax,ebx
      mov [Font.y],eax
      call DrawString

      ret

DrawPictureOnTheButton:

        xor esi,esi
        xor edi,edi
        xor eax,eax
        xor ebx,ebx
        mov ax,[Button.height]
        mov bx,[Button.width]
        mov di,[Button.imageSizeY]
        mov si,[Button.imageSizeX]
        shr ax,1
        shr bx,1
        shr si,1
        shr di,1
        sub ax,di
        sub bx,si
        add ax,[Button.y]
        add bx,[Button.x]
        mov [xo],ebx
        mov [x],ebx
        mov [y],eax

        xor esi,esi
        xor edi,edi
        mov si,[Button.imageSizeX]
        mov di,[Button.imageSizeY]
        mov ebp,[Button.image]

        xor eax,eax
        inc eax

        loop_y_image:

           mov ebx,[xo]
           mov [x],ebx

          xor esi,esi
          mov si,[Button.imageSizeX]

          loop_x_image:

             mov edx,[ebp]
             and edx,0xffffff
             cmp edx,[Button.transparentColor]
             je no_draw_pixel

               mov ebx,[x]
               mov ecx,[y]
               mcall
             no_draw_pixel:

             add ebp,3
             add [x],1
          dec esi
          jnz  loop_x_image

          add [y],1
        dec edi
        jnz loop_y_image

      ret


Draw3DActiveButton:

      ;call calculate_average_color
      mov [Line.color],0xffffff;eax

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],3
      sub ecx,1
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,esi
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],3
      sub ecx,1
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      add ebx,ecx
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],1
      xor ecx,ecx
      mov cx,[Button.width]
      sub ecx,1
      add [Line.x2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],1
      xor ecx,ecx
      mov cx,[Button.width]
      sub ecx,1
      add [Line.x2],ecx
      call DrawLine

      ret

Put3DActiveButton:

      mov eax,[Button.color1]
      mov ebx,0xffffff
      mov ecx,20
      mov edx,dword pointer
      call gradient

      mov eax,[pointer+3];[Button.color1]
      and eax,0xffffff
      mov [Line.color],eax

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],3
      sub ecx,1
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,esi
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],3
      sub ecx,1
      add [Line.y2],ecx
      call DrawLine


      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      add ebx,ecx
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],1
      xor ecx,ecx
      mov cx,[Button.width]
      sub ecx,1
      add [Line.x2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],1
      xor ecx,ecx
      mov cx,[Button.width]
      sub ecx,1
      add [Line.x2],ecx
      call DrawLine

      ret

DrawPressed3DButton:


      mov eax,[Button.color1]
      mov ebx,0xffffff
      mov ecx,20
      mov edx,dword pointer
      call gradient

      mov edx,[pointer+3]
      and edx,0xffffff
      mov [Line.color],edx

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],3
      sub ecx,1
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,esi
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y1],3
      sub ecx,1
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      add ebx,ecx
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],1
      xor ecx,ecx
      mov cx,[Button.width]
      sub ecx,1
      add [Line.x2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x1],1
      xor ecx,ecx
      mov cx,[Button.width]
      sub ecx,1
      add [Line.x2],ecx
      call DrawLine

      ret

Draw2DActiveButton:

      mov edx,[colors_table1]
      and edx,0xffffff
      ;mov [Button.color2],edx

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      mov [Line.color],edx
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov si,[Button.width]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],esi
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,esi
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add ebx,ecx
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],esi
      call DrawLine

      ret

Put2DActiveButton:

      mov edx,[pointer+15*3]
      and edx,0xffffff

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      mov [Line.color],edx
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov si,[Button.width]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],esi
      call DrawLine

      mov edx,[colors_table2+8*3]
      and edx,0xffffff

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,esi
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      mov [Line.color],edx
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add ebx,ecx
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],esi
      call DrawLine

      ret

DrawPressed2DButton:

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      and [Line.color],0
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      ;mov cx,[Button.height]
      mov si,[Button.width]
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],esi
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add eax,esi
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      mov [Line.color],0xffffff
      add [Line.y2],ecx
      call DrawLine

      xor eax,eax
      xor ebx,ebx
      xor ecx,ecx
      xor esi,esi
      mov ax,[Button.x]
      mov bx,[Button.y]
      mov cx,[Button.height]
      mov si,[Button.width]
      add ebx,ecx
      mov [Line.x1],eax
      mov [Line.x2],eax
      mov [Line.y1],ebx
      mov [Line.y2],ebx
      add [Line.x2],esi
      call DrawLine

      ret
