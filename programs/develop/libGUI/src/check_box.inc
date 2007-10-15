
cb_control_data_dize = 44

;****************************************************
;****************craete CheckBox**********************
;****************************************************
;IN
;pointer to parend
;pointer to CheckBox's structure
;OUT
;pointer to initialized control
align 4

craete_check_box:

      mov ebx,[esp+4]
      mov eax,[esp+8]

      mov [PointerToStructureForCheckBox],eax

      mov eax,control_header_size+cb_control_data_dize
      call craete_control

      ;set all CheckBox's parameters in control
      mov [eax],dword check_box

      mov ecx,cb_control_data_dize
      mov esi,[PointerToStructureForCheckBox]
      mov edi,eax
      add edi,control_header_size
      rep movsb

      call get_skin_height

      mov ebx,[PointerToStructureForCheckBox]
      xor ecx,ecx
      xor edx,edx
      mov cx,[ebx+2]   ;CheckBox x
      mov dx,[ebx+4]   ;CheckBox y
      mov esi,[ebx+10] ;CheckBox size
      add ecx,border_width
      add edx,[skin_height]
      ;copy information to control
      mov [eax+24],ecx
      mov [eax+28],edx
      mov [eax+32],esi
      mov [eax+36],esi

      ret 8

;****************************************************
;*****************Draw CheckBox**********************
;****************************************************
;IN
;pointer to control of CheckBox
;message
;OUT
;not returned value
align 4

check_box:

      ;get message
      mov eax,[esp+8]

      ;get pointer to control of CheckBox
      mov esi,[esp+4]
      mov [PointerForCheckBox],esi

      ;copy information from control to structure
      add esi,control_header_size
      mov edi,dword CheckBox
      mov ecx,control_header_size+cb_control_data_dize
      rep movsb

      push eax
      ;load coordinats and size from control
      mov eax,[PointerForCheckBox]
      mov ebx,[eax+24]           ;x
      mov ecx,[eax+28]           ;y
      mov edx,[eax+32]           ;size
      ;set current coordinats and sizes in CheckBox
      mov [CheckBox.ch_left],bx
      mov [CheckBox.ch_top],cx
      mov [CheckBox.ch_size],edx
      pop eax

      cmp [eax],dword 1
      jne no_redraw_all_check_box

       mov eax,13
       mov bx,[CheckBox.ch_left]
       mov cx,[CheckBox.ch_top]
       shl ebx,16
       shl ecx,16
       add ebx,[CheckBox.ch_size]
       add ecx,[CheckBox.ch_size]
       mov edx,[CheckBox.ch_border_color]
        mcall

       mov edx,[CheckBox.ch_color]
       add ebx,1 shl 16 - 2
       add ecx,1 shl 16 - 2
        mcall

       mov eax,[CheckBox.ch_size]
       mov ebx,eax
       mov ecx,3
       shr ebx,1
       cdq
       idiv ecx
       mov [CheckBox.ch_size_2],bx
       mov [CheckBox.ch_size_3],ax

       test word [CheckBox.ch_flags],2
       jz   @f
       call draw_ch
    @@:

        movzx ebx,word[CheckBox.ch_left]
        add ebx,[CheckBox.ch_size]
        add ebx,[CheckBox.ch_text_margin]
        shl   ebx,16
        mov   bx,[CheckBox.ch_top]
        add   ebx,[CheckBox.ch_size]
        add   ebx,(-9+1)
        mov   ecx,[CheckBox.ch_text_color]

        mov   edx,[CheckBox.ch_text_ptr]
        movzx esi,word [CheckBox.ch_text_length]

        mov   eax,4
        mcall

      jmp exit_check_box
      no_redraw_all_check_box:


      cmp [eax],dword 6
      jne no_mouse_check_box

      mov esi,[eax+4]
      mov edi,[eax+8]
      mov ebx,[eax+12]   ;buttons of mouse state
      mov [CheckBox.mouseX],esi
      mov [CheckBox.mouseY],edi
      mov eax,ebx

      test eax,eax
      jnz  @f
      btr  word [CheckBox.ch_flags],2
      jmp exit_check_box
@@:
      bts  word [CheckBox.ch_flags],2
      jc   .mouse_end
      movzx  esi,word [CheckBox.ch_text_length]

      imul esi,6
      add  esi,[CheckBox.ch_text_margin]

      mov eax,[CheckBox.mouseX]
      shl eax,16
      add eax,[CheckBox.mouseY]

      xor ebx,ebx

      movzx ebx,word[CheckBox.ch_top]
      cmp   ax,bx
      jl   .mouse_end
      add   ebx,[CheckBox.ch_size]
      cmp   ax,bx
      jg   .mouse_end

      shr   eax,16
      movzx ebx,word[CheckBox.ch_left]
      cmp   ax,bx
      jl   .mouse_end
      add   ebx,[CheckBox.ch_size]
      add   ebx,esi
      cmp   ax,bx
      jg   .mouse_end

      bts  word[CheckBox.ch_flags],1
      jc   @f

      call draw_ch

      jmp  .mouse_end
@@:
      btr   word[CheckBox.ch_flags],1
      call clear_ch

      .mouse_end:

      no_mouse_check_box:

      exit_check_box:

      ;save resultat of work in control
      mov ecx,cb_control_data_dize           ;save in control only flags
      mov esi,dword CheckBox
      mov edi,[PointerForCheckBox]
      add edi,control_header_size
      cld
      rep movsb

      ret 8

;ch_text_margin=4
;ch_size=11

clear_ch:
        mov   edx,[CheckBox.ch_color]
        jmp   @f

draw_ch:
        mov   edx,[CheckBox.ch_border_color]
@@:

        movzx ebx,word[CheckBox.ch_left]
        add   bx,[CheckBox.ch_size_3]
        shl   ebx,16
        mov   bx,[CheckBox.ch_size_2]
        ;mov   bp,bx
        ;push        bx
        movzx ecx,word [CheckBox.ch_top]
        mov   eax,13
        add   cx,[CheckBox.ch_size_3]
        shl   ecx,16
        ;mov   cx,bp
        mov cx,[CheckBox.ch_size_2]
        mcall

        ret
