;****************************************************************
;***************Elements of menegment for Controls***************
;****************************************************************

;**********************************************************
;********************craete control*************************
;**********************************************************

;IN
;eax - ctructure's size
;ebx - Parend(pointer to structure for parend)
;OUT
;eax - pointer to new control

craete_control:

      test eax,eax
      jnz no_null_size

        mov eax,1
      no_null_size:

      call malloc
      mov [ReturnControl],eax

      push ebx  ;parend
      push eax  ;control
      call  AddObject

      mov eax,[ReturnControl]

      ;information about craete control output to debug board
      call new_line
      mov [s_p],s_craete
      call print_string
      debug_print_dec [ReturnControl]
      mov [s_p],s_parend
      call print_string
      debug_print_dec dword[eax+20]
      mov [s_p],s_last
      call print_string
      debug_print_dec dword[eax+4]
      ;--------------------------------

      ret

AddObject:

      mov ecx,[esp+4] ;control
      mov edx,[esp+8] ;parend

      mov eax,[edx+12]  ;eax=parend->child_fd
      test eax,eax
      jnz else_parend_add_object

        add [ControlID],1
        mov esi,[ControlID]

        mov [edx+12],ecx   ;Parend->child_fd=control
        mov [edx+16],ecx   ;Parend->child_bk=control

        mov [ecx+20],edx   ;control->parend=Parend
        mov [ecx+4],edx    ;control->ctrl_fd=Parend
        mov [ecx+40],esi   ;control->ctrl_ID=ID

      jmp exit_if_parend_add_object
      else_parend_add_object:

        add [ControlID],1
        mov esi,[ControlID]

        mov edi,[edx+16]   ;last_control=Parend->child_bk
        mov [edx+16],ecx   ;Parend->child_bk=control

        mov [edi+8],ecx    ;last_control->ctrl_bk=control
        mov [ecx+4],edi    ;control->ctrl_fd=last_control;
        mov [ecx+20],edx   ;control->parend=Parend;
        mov [ecx+40],esi   ;control->ctrl_ID=ID

      exit_if_parend_add_object:

      ret 8


;************************************************************
;***********************Destroy control**********************
;************************************************************

;Delete Control.
;IN
;pointer to control for delete
;OUT
;not returned value
align 4

destroy_control:

      push eax
      push ebx
      push ecx
      push edx
      push esi
      push edi
      push ebp

      mov eax,[esp+28+4]

      mov ebx,[eax+20] ;get parend for control

      ;checking pointer of control to 0
      test eax,eax
      jz exit_destroy_control

      ;checking parend to main parend(parend of window)
      ;if parend equ main parend than exit
      test ebx,ebx
      jz exit_destroy_control

      mov ecx,[ebx+12] ;Parend->child_fd
      test ecx,ecx
      jz exit_destroy_control

         mov ecx,[ebx+12]    ;Parend->child_fd
         mov edx,[ebx+16]    ;Parend->child_bk

         cmp ecx,edx
         jne no_if_parend_child_fd_child_bk

            cmp ecx,eax
            jne no_if_parend_child_fd_control

              mov [ebx+12],dword 0  ;Parend->child_fd=0
              mov [ebx+16],dword 0  ;Parend->child_bk=0

              ;send message to control for deleting
              push eax

              mov [Message],dword -1
              mov ebx,[eax] ;load .... for control
              push Message
              push eax
              call ebx

              pop eax

               push eax
               call free
               pop eax

               call new_line
               mov [s_p],s_destroy
               call print_string
               debug_print_dec eax
               mov [s_p],s_1
               call print_string

              jmp exit_destroy_control
            no_if_parend_child_fd_control:

         no_if_parend_child_fd_child_bk:
      ;----------------------------------------------
        mov esi,ecx    ;seek_control=Parend->child_fd

        while_seek_control:
        cmp esi,0
        je exit_destroy_control
        ;*******************************************************************
            cmp esi,eax
            jne no_if_seek_control_control
            ;***************************************************************
                mov edi,[esi+8]
                test edi,edi
                jz no_if_seek_control_ctrl_bk
                ;***********************************************************
                    mov ecx,[esi+4]
                    mov edx,[esi+8]
                    mov [edx+4],ecx

                    cmp ecx,ebx
                    je no_if_last_control_parend
                    ;*******************************************************
                       mov [ecx+8],edx
                       jmp else_seek_control_ctrl_bk
                    ;*******************************************************
                    no_if_last_control_parend:

                       mov [ebx+12],edx

                  jmp else_seek_control_ctrl_bk
                  ;*********************************************************
                no_if_seek_control_ctrl_bk:
                ;**********************************************************

                    mov ecx,[esi+4]      ;last_control=seek_control->ctrl_fd
                    mov [ecx+8],dword 0   ;last_control->ctrl_bk=0
                    mov [ebx+16],ecx     ;Parend->child_bk=last_control

                else_seek_control_ctrl_bk:
                ;**********************************************************

                 ;send message to control for deleting
                 push eax

                 mov [Message],dword -1
                 mov ebx,[eax] ;load .... for control
                 push Message
                 push eax
                 call ebx

                 pop eax

                 push eax
                 call free
                 pop eax

                 call new_line
                 mov [s_p],s_destroy
                 call print_string
                 debug_print_dec eax
                 mov [s_p],s_2
                 call print_string

                 jmp exit_destroy_control

            no_if_seek_control_control:
            ;****************************************************************

            mov edi,[esi+8]   ;exchange_control=seek_control->ctrl_bk
            mov esi,edi       ;seek_control=exchange_control

        jmp  while_seek_control
        ;*******************************************************************
      exit_destroy_control:

      pop ebp
      pop edi
      pop esi
      pop edx
      pop ecx
      pop ebx
      pop eax

      ret 4

;***********************************************************
;*****************Send message to Control*******************
;***********************************************************

;IN
;pointer to message
;pointer to parend(structure)
;OUT
;not returned value
align 4

send_message:

      push eax
      push ebx
      push ecx
      push edx
      push esi
      push edi
      push ebp

      ;get pointer to parend
      mov eax,[esp+28+4]
      mov [Parend],eax

      ;get message
      mov esi,[esp+28+8]
      mov edi,dword Message
      mov ecx,4
      rep movsd

      ;load event type
      mov eax,[Message]


      ;check for event type and get parameters for Control
      dec eax
      jnz no_window_redraw

        mov ebx,[Parend]
        mov esi,[ebx+12]  ;get pointer to first child control of parend
        mov ebx,esi

        send_message_for_redraw_to_next_control:

          ;if esi=0 than exit
          test esi,esi
          jz exit_check_event_type

            push ebx
            push esi

            mov eax,ebx
            mov ebx,[eax] ;load .... for control
            push Message
            push eax
            call ebx

            pop esi
            pop ebx

            mov esi,[ebx+8]
            mov ebx,esi

          jmp send_message_for_redraw_to_next_control

      no_window_redraw:

      dec eax
      jnz no_keys


        ;if have active control than send message
        ;checking for active control
      mov ebx,[ActiveControlForKeys]
      test ebx,ebx
      jz exit_check_event_type

        mov eax,[ActiveControlForKeys]
        mov ebx,[eax] ;load proc for control
        push Message
        push eax
        call ebx

      jmp exit_check_event_type
      no_keys:

      dec eax
      jnz no_special_system_message

        mov ebx,[Parend];
        mov esi,[ebx+12]
        mov ebx,esi

        send_special_message_to_next_control:

          ;if esi=0 than exit
          test esi,esi
          jz exit_check_event_type

            push ebx
            push esi

            mov eax,ebx
            mov ebx,[eax] ;load .... for control
            push Message
            push eax
            call ebx

            pop esi
            pop ebx

            mov esi,[ebx+8]
            mov ebx,esi

          jmp send_special_message_to_next_control

      no_special_system_message:

      ;*************************************************
      ;****************Mouse send message***************
      ;*************************************************

      cmp eax,3
      jne no_mouse

        ;check childrens of parend for crossing
        mov ebx,[Parend]
        mov esi,[ebx+12]    ;esi =first child control
        mov ebx,esi         ;ebx =first child control

          mov eax,[Message+12]
          and eax,1b
          test eax,eax
          jz left_button_of_mouse_not_pressed

                mov eax,[ActiveControl]
                test eax,eax
                jz havent_active_control

                        mov eax,[ActiveControl]
                        mov ebx,[eax] ;load proc for control
                        push Message
                        push eax
                        call ebx

                        jmp exit_check_event_type
                havent_active_control:

          jmp exit_if_left_button_of_mouse__pressed
          left_button_of_mouse_not_pressed:

                mov eax,[ActiveControl]
                test eax,eax
                jz havent_active_control_

                        mov [ActiveControl],dword 0

                havent_active_control_:

          exit_if_left_button_of_mouse__pressed:


        next_conrol_check_for_crossing_with_mouse:

          test esi,esi
          jz exit_next_conrol_check_for_crossing_with_mouse

          mov [PointerToControl],ebx


          push ebx
          push esi

          mov eax,[ebx+28]    ;eax = y coordinat of control
          mov [y_ctl],eax
          mov eax,[ebx+24]    ;eax = x coordinat of control

          mov ecx,[ebx+32]    ;ecx = size x of control
          mov edx,[ebx+36]    ;edx = size y of control
          mov esi,[Message+4] ;mouse x
          mov edi,[Message+8] ;mouse y
          mov ebx,[y_ctl]

          call CheckCrossingBox

          cmp eax,0xffffff
          jne no_activate_control

            mov eax,[Message+12]
            and eax,1b
            test eax,eax
            jz no_activate_control

               mov eax,[PointerToControl]
               mov [ActiveControl],eax
               mov [ActiveControlForKeys],eax

          no_activate_control:

            mov eax,[PointerToControl]
            mov ebx,[eax] ;load proc for control
            push Message
            push eax
            call ebx

          pop esi
          pop ebx

          mov esi,[ebx+8] ;16 but not 8
          mov ebx,esi

        jmp next_conrol_check_for_crossing_with_mouse

        exit_next_conrol_check_for_crossing_with_mouse:

      no_mouse:

      exit_check_event_type:

      pop ebp
      pop edi
      pop esi
      pop edx
      pop ecx
      pop ebx
      pop eax

      ret 8


new_line:

      pushad

      mov eax,63
      mov ebx,1
      xor ecx,ecx
      mov cl,13
      int 0x40

      mov eax,63
      mov ebx,1
      xor ecx,ecx
      mov cl,10
      int 0x40

      popad

      ret

print_string:

      pushad

      xor esi,esi

      next_symbol_print:

      mov edi,[s_p]
      add edi,esi

      xor edx,edx
      mov dl,byte[edi]
      test dl,dl
      jz exit_print_str

      mov eax,63
      mov ebx,1
      xor ecx,ecx
      mov cl,byte[edi]
      int 0x40

      inc esi
      jmp next_symbol_print

      exit_print_str:

      popad


      ret

s_p dd 0
s_craete  db 'craete ',0
s_destroy db 'destroy ',0
s_parend  db ' parend ',0
s_last    db ' ctrl_fd ',0
s_next    db ' ctrl_bk ',0
s_1       db ' 1',0
s_2       db ' 2',0