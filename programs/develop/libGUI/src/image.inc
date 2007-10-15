
im_control_data_size           = 26
im_redraw_all                  = 10000000b
im_redraw_all_off              = 01111111b
im_special_redraw_on           = 1b
im_special_redraw_off          = 11111110b

;****************************************************
;*******************craete Image**********************
;****************************************************
;IN
;pointer to parend
;pointer to Image's structure
;OUT
;pointer to initialized Image's structure
align 4

craete_image:

      mov ebx,[esp+4]
      mov eax,[esp+8]

      mov [PointerToStructureForImage],eax

      mov eax,control_header_size+im_control_data_size
      call craete_control

      ;set all image's parameters in control
      mov [eax],dword image

      mov ecx,im_control_data_size
      mov esi,[PointerToStructureForImage]
      mov edi,eax
      add edi,control_header_size
      rep movsb

      call get_skin_height

      mov ebx,[PointerToStructureForImage]
      mov ecx,[ebx+6]   ;x
      mov edx,[ebx+10]  ;y
      mov esi,[ebx+14]  ;size x
      mov edi,[ebx+18]  ;size y
      add ecx,border_width
      add edx,[skin_height]
      ;copy information to control
      mov [eax+24],ecx
      mov [eax+28],edx
      mov [eax+32],esi
      mov [eax+36],edi

      ret 8

;****************************************************
;********************Draw Image**********************
;****************************************************
;IN
;pointer to control of button
;message
;OUT
;not returned value
align 4

image:

      ;get message
      mov eax,[esp+8]

      ;get pointer to control of image
      mov esi,[esp+4]
      mov [PointerForImage],esi
      ;copy control to local control
      add esi,control_header_size
      mov edi,dword Image
      mov ecx,im_control_data_size
      rep movsb


      cmp [eax],dword 1
      jne no_redraw_all_image

      or [Image.type],im_redraw_all
      no_redraw_all_image:

      cmp [eax],dword 3
      jne no_special_message_image

       xor eax,eax
       mov al,[Image.type]
       and al,im_special_redraw_on
       test al,al
       jz no_activate_redraw_image

         and [Image.type],im_special_redraw_off
         or [Image.type],im_redraw_all
       no_activate_redraw_image:

      no_special_message_image:

      ;load coordinats and size from control
      mov eax,[PointerForImage]
      mov ebx,[eax+24]           ;x
      mov ecx,[eax+28]           ;y
      mov edx,[eax+32]           ;size x
      mov esi,[eax+36]           ;size y
      ;set current coordinats and sizes in zakladka
      mov [Image.x],ebx
      mov [Image.y],ecx
      mov [Image.sizex],edx
      mov [Image.sizey],esi

      xor eax,eax
      mov al,[Image.type]
      and al,im_redraw_all
      test al,al
      jz no_redraw_image_

       and [Image.type],im_redraw_all_off

       mov esi,dword Image
       mov edi,[PointerForImage]
       add edi,control_header_size
       mov ecx,2
       rep movsb

       mov eax,7
       mov ebx,[Image.pointer]
       mov ecx,[Image.sizex]
       shl ecx,16
       add ecx,[Image.sizey]
       mov edx,[Image.x]
       shl edx,16
       add edx,[Image.y]
       mcall

      no_redraw_image_:

      exit_image:

      ret 8
