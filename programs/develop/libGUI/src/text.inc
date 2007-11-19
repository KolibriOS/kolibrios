
tx_control_data_size            = 22
tx_no_show_text                 = 00000001b
tx_redraw_all                   = 10000000b
tx_redraw_all_off               = 01111111b
tx_special_redraw_on            = 00000100b
tx_special_redraw_off           = 11111011b

;****************************************************
;********************craete Text**********************
;****************************************************
;IN
;pointer to parend
;pointer to Text's structure
;OUT
;pointer to initialized Texr's structure
align 4

craete_text:

      mov ebx,[esp+4]
      mov eax,[esp+8]

      mov [PointerToStructureForText],eax

      mov eax,control_header_size+tx_control_data_size
      call craete_control

      ;set all image's parameters in control
      mov [eax],dword text

      mov ecx,tx_control_data_size
      mov esi,[PointerToStructureForText]
      mov edi,eax
      add edi,control_header_size
      rep movsb

      call get_skin_height

      mov ebx,[PointerToStructureForText]
      mov ecx,[ebx+6]   ;x
      mov edx,[ebx+10]  ;y
      mov esi,[ebx+14]  ;length x
      imul esi,6
      add ecx,border_width
      add edx,[skin_height]
      ;copy information to control
      mov [eax+24],ecx
      mov [eax+28],edx
      mov [eax+32],esi
      mov [eax+36],dword 9

      ret 8

;****************************************************
;********************Draw Text**********************
;****************************************************
;IN
;pointer to control of text
;message
;OUT
;not returned value
align 4

text:

      ;get message
      mov eax,[esp+8]

      ;get pointer to control of image
      mov esi,[esp+4]
      mov [PointerForText],esi
      ;copy control to local control
      add esi,control_header_size
      mov edi,dword Text
      mov ecx,tx_control_data_size
      rep movsb


      cmp [eax],dword 1
      jne no_redraw_all_text

      xor ebx,ebx
      mov bl,[Text.type]
      and bl,tx_no_show_text
      test bl,bl
      jnz no_redraw_all_text

        or [Text.type],tx_redraw_all

      no_redraw_all_text:

      cmp [eax],dword 3
      jne no_special_message_for_text

      xor ebx,ebx
      mov bl,[Text.type]
      and bl,tx_special_redraw_on
      test bl,bl
      jz no_special_message_for_text

        or [Text.type],tx_redraw_all
        and [Text.type],tx_special_redraw_off

      no_special_message_for_text:

      ;load coordinats and size from control
      mov eax,[PointerForText]
      mov ebx,[eax+24]           ;x
      mov ecx,[eax+28]           ;y
      ;set current coordinats and sizes in zakladka
      mov [Text.x],ebx
      mov [Text.y],ecx

      ;get standart colors table
        mov     eax,48
        mov     ebx,3
        mov     ecx,dword pointer2
        mov     edx,40
        mcall

      xor eax,eax
      mov al,[Text.type]
      and al,tx_redraw_all
      test al,al
      jz no_redraw_text_

       and [Text.type],tx_redraw_all_off

       mov esi,dword Text
       mov edi,[PointerForText]
       add edi,control_header_size
       mov ecx,2
       rep movsb

       mov eax,4
       mov ebx,[Text.x]
       shl ebx,16
       add ebx,[Text.y]
       mov ecx,[Text.color]
       mov edx,[Text.pointer]
       mov esi,[Text.length]
       mov edi,[pointer2+8]
       mcall

      no_redraw_text_:

      exit_text:

      ret 8
