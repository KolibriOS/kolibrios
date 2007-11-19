
 nm_control_data_size          = 22
 nm_redraw_all                 = 10000000b
 nm_redraw_off                 = 01111111b
 ;nm_special_redraw_on          = 00000100b
 ;nm_special_redraw_off         = 11111011b
 nm_integer_type               = 00000010b
 nm_no_show_number             = 00000001b

;****************************************************
;********************craete Number**********************
;****************************************************
;IN
;pointer to parend
;pointer to Number's structure
;OUT
;pointer to initialized Number's structure
align 4

craete_number:

      mov ebx,[esp+4]
      mov eax,[esp+8]

      mov [PointerToStructureForNumber],eax

      mov eax,control_header_size+nm_control_data_size
      call craete_control

      ;set all image's parameters in control
      mov [eax],dword number

      mov ecx,nm_control_data_size
      mov esi,[PointerToStructureForNumber]
      mov edi,eax
      add edi,control_header_size
      rep movsb

      call get_skin_height

      mov ebx,[PointerToStructureForNumber]
      mov ecx,[ebx+6]   ;x
      mov edx,[ebx+10]  ;y
      add ecx,border_width
      add edx,[skin_height]
      ;copy information to control
      mov [eax+24],ecx
      mov [eax+28],edx

      ret 8

;****************************************************
;********************Draw Number*********************
;****************************************************
;IN
;pointer to control of number
;message
;OUT
;not returned value
align 4

number:

      ;get message
      mov eax,[esp+8]

      ;get pointer to control of image
      mov esi,[esp+4]
      mov [PointerForNumber],esi
      ;copy control to local control
      add esi,control_header_size
      mov edi,dword Number
      mov ecx,nm_control_data_size
      rep movsb


      cmp [eax],dword 1
      jne no_redraw_all_number

      xor ebx,ebx
      mov bl,[Number.type]
      and bl,nm_no_show_number
      test bl,bl
      jnz no_redraw_all_number

        or [Number.type],nm_redraw_all

      no_redraw_all_number:

      cmp [eax],dword 3
      jne no_special_message_for_number

      xor ebx,ebx
      mov bl,[Number.type]
      and bl,activate_trap
      test bl,bl
      jz no_special_message_for_number

        or [Number.type],nm_redraw_all
        and [Number.type],deactivate_trap

      no_special_message_for_number:

      cmp [eax],dword -1
      jne no_destroy_control_of_number

         jmp exit_number
      no_destroy_control_of_number:

      ;load coordinats and size from control
      mov eax,[PointerForNumber]
      mov ebx,[eax+24]           ;x
      mov ecx,[eax+28]           ;y
      ;set current coordinats and sizes in zakladka
      mov [Number.x],ebx
      mov [Number.y],ecx

      xor eax,eax
      mov al,[Number.type]
      and al,nm_redraw_all
      test al,al
      jz no_redraw_number_

       and [Number.type],nm_redraw_off

       mov esi,dword Number
       mov edi,[PointerForNumber]
       add edi,control_header_size
       mov ecx,2
       rep movsb

       ;get standart colors table
       mov eax,48
       mov ebx,3
       mov ecx,dword pointer2
       mov edx,40
       mcall

       xor eax,eax
       mov al,[Number.type]
       and al,nm_integer_type
       test al,al
       jnz no_integer_number

         mov eax,47
         mov ebx,[Number.parameters]
         mov ecx,[Number.number]
         mov edx,[Number.x]
         shl edx,16
         add edx,[Number.y]
         mov esi,[Number.color]
         mov edi,[pointer2+8]
         mcall

         jmp no_redraw_number_
       no_integer_number:
      ;---------------------------------------

      mov ebx,[Number.parameters]
      mov ecx,ebx
      shr ebx,16      ;format for integer
      and ecx,0xffff  ;format for float

      ;get integer part of float number
      mov eax,1
      mov edi,ecx
      pow_10__:

        ;eax=eax*10
        lea eax,[eax+eax*4]
        shl eax,1

      dec edi
      jnz pow_10__
      mov edx,eax

      ;check for signum
      mov [v2],0
      fld [Number.number]
      fcom [v2]
      fstsw ax
      sahf
      jae no_signum_float_number

         fabs
         mov [signum_float_number],byte 1

      no_signum_float_number:
      fstp [Number.number]

      mov [v2],edx  ;v2=10^ecx
      fild [v2]
      fld [Number.number]
      fmul st0,st1
      fistp [v]
      fstp st0

      mov esi,edx
      mov eax,[v]
      cdq
      idiv esi
      mov [v],eax

      and [integer_part],0
      and [tochnost1],0

      ;save v in v2
      mov eax,[v]
      mov [v2],eax

      mov [Number.flag],0
      mov esi,ebx
      get_next_razryd_of_number_integer:

          mov eax,1

          mov edi,esi
          pow_10_integer:

            ;eax=eax*10
            lea eax,[eax+eax*4]
            shl eax,1

          dec edi
          jnz pow_10_integer

          mov edi,eax
          mov eax,[v]
          cdq
          idiv edi
          and eax,0xf

          test eax,eax
          jz no_save_pos_for_integer

           cmp [Number.flag],0
           jne no_save_pos_for_integer

              mov [Number.flag],1b
              mov [tochnost1],esi
              add [tochnost1],1

          no_save_pos_for_integer:

          imul eax,edi
          sub [v],eax
          add [integer_part],eax

      dec esi
      jnz get_next_razryd_of_number_integer
      mov eax,[v]
      add [integer_part],eax

      cmp [tochnost1],0
      jnz all_ok_with_tochnost

          mov [tochnost1],1
      all_ok_with_tochnost:

      ;get float part of float number
      mov eax,[v2]
      mov [v],eax

      mov eax,1
      mov edi,ecx
      pow_10_float__:

        ;eax=eax*10
        lea eax,[eax+eax*4]
        shl eax,1

      dec edi
      jnz pow_10_float__

      mov [v2],eax  ;v2=10^ecx

      fild [v2]
      fild [v]
      fld [Number.number]
      fsub st0,st1
      fmul st0,st2
      fistp [v]
      fstp st0
      fstp st0

      and [float_part],0

      mov esi,ecx
      get_next_razryd_of_number_float:

          mov eax,1

          mov edi,esi
          pow_10_float:

            ;eax=eax*10
            lea eax,[eax+eax*4]
            shl eax,1

          dec edi
          jnz pow_10_float

          mov edi,eax
          mov eax,[v]
          cdq
          idiv edi
          and eax,0xf
          imul eax,edi
          sub [v],eax
          add [float_part],eax

      dec esi
      jnz get_next_razryd_of_number_float
      mov eax,[v]
      add [float_part],eax

      mov [tochnost2],ecx
      cmp [signum_float_number],byte 1
      jne no_draw_signum_for_float_number

           ;draw signum
           mov eax,4
           mov ebx,[Number.x]
           mov ecx,[Number.color]
           mov edx,dword signum
           mov esi,1
           mov edi,[pointer2+8]
           shl ebx,16
           add ebx,[Number.y]
           mcall

           add [Number.x],6
           mov [signum_float_number],0
      no_draw_signum_for_float_number:

      ;draw integer part of float number
      mov eax,47
      mov ebx,[tochnost1]
      mov ecx,[integer_part]
      mov edx,[Number.x]
      mov esi,[Number.color]
      mov edi,[pointer2+8]
      shl edx,16
      shl ebx,16
      add edx,[Number.y]
      mcall

      mov edx,[tochnost1]
      add edx,1
      lea edx,[edx+edx*2]
      shl edx,1        ;edx=edx*6
      add edx,[Number.x]
      mov [x],edx

      ;draw float part of float number
      mov eax,47
      mov ebx,[tochnost2]
      mov ecx,[float_part]
      mov esi,[Number.color]
      mov edi,[pointer2+8]
      shl edx,16
      shl ebx,16
      add edx,[Number.y]
      mcall

      ;draw point betwen integer part of number and float part of number
      mov eax,4
      mov ebx,[x]
      mov ecx,[Number.color]
      mov edx,dword point
      mov esi,1
      mov edi,[pointer2+8]
      sub ebx,6
      shl ebx,16
      add ebx,[Number.y]
      mcall

      no_redraw_number_:
      ;-----------------------------------------

      exit_number:

      ret 8
