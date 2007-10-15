
pb_control_data_size              = 34
pb_redraw_all                     = 10000000b
pb_redraw_all_off                 = 01111111b
pb_special_progress_on            = 1b
pb_special_progress_off           = 11111110b

;****************************************************
;****************craete ProgressBar********************
;****************************************************
;IN
;pointer to parend
;pointer to ProgressBar's structure
;OUT
;pointer to initialized control
align 4

craete_progress_bar:

      mov ebx,[esp+4]
      mov eax,[esp+8]

      mov [PointerToStructureForProgressBar],eax

      mov eax,control_header_size+pb_control_data_size
      call craete_control

      ;set all EditBox's parameters in control
      mov [eax],dword progress_bar

      mov ecx,pb_control_data_size
      mov esi,[PointerToStructureForProgressBar]
      mov edi,eax
      add edi,control_header_size
      rep movsb

      call get_skin_height

      mov ebx,[PointerToStructureForProgressBar]
      mov ecx,[ebx+10]  ;x
      mov edx,[ebx+14]  ;y
      mov esi,[ebx+18]  ;size x
      mov edi,[ebx+22]  ;size y
      add ecx,border_width
      add edx,[skin_height]
      ;copy information to control
      mov [eax+24],ecx
      mov [eax+28],edx
      mov [eax+32],esi
      mov [eax+36],edi

      ret 8

;****************************************************
;*****************Draw ProgressBar********************
;****************************************************
;IN
;pointer to control of ProgressBar
;message
;OUT
;not returned value
align 4

progress_bar:

      ;get message
      mov eax,[esp+8]

      ;get pointer to control of button
      mov esi,[esp+4]
      mov [PointerForProgressBar],esi

      ;copy control to local control
      add esi,control_header_size
      mov edi,dword ProgressBar
      mov ecx,pb_control_data_size
      rep movsb


      push eax
      ;load coordinats and size from control
      mov eax,[PointerForProgressBar]
      mov ebx,[eax+24]           ;x
      mov ecx,[eax+28]           ;y
      mov edx,[eax+32]           ;size x
      mov esi,[eax+36]           ;size y
      ;set current coordinats and sizes in EeditBox
      mov [ProgressBar.x],ebx
      mov [ProgressBar.y],ecx
      mov [ProgressBar.sizex],edx
      mov [ProgressBar.sizey],esi

      pop eax


      cmp [eax],dword -1
      jne no_destroy_progress_bar

      jmp exit_progress_bar
      no_destroy_progress_bar:

      cmp [eax],dword 1
      jne no_redraw_all_progress_bar

         or [ProgressBar.type],pb_redraw_all

      no_redraw_all_progress_bar:

      cmp [eax],dword 3
      jne no_special_message_progress_bar

         or [ProgressBar.type],pb_special_progress_on

      no_special_message_progress_bar:

      xor eax,eax
      mov al,[ProgressBar.type]
      and al,pb_redraw_all
      test al,al
      jz no_draw_all_progress_bar

         and [ProgressBar.type],pb_redraw_all_off

          call draw_all_progress_bar

         jmp exit_progress_bar

      no_draw_all_progress_bar:

      xor eax,eax
      mov al,[ProgressBar.type]
      and al,pb_special_progress_on
      test al,al
      jz no_draw_progress_bar

         and [ProgressBar.type],pb_special_progress_off

         call draw_progress

      no_draw_progress_bar:

      exit_progress_bar:

      ret 8

draw_all_progress_bar:

         mov eax,[ProgressBar.color3]
         mov ebx,dword 0xffffff
         mov ecx,20
         mov edx,dword colors_table1
         call gradient

         mov eax,13
         mov ebx,[ProgressBar.x]
         mov ecx,[ProgressBar.y]
         mov edx,[ProgressBar.color1]
         shl ebx,16
         shl ecx,16
         add ebx,[ProgressBar.sizex]
         add ecx,[ProgressBar.sizey]
         mcall

         mov eax,[ProgressBar.x]
         mov ebx,[ProgressBar.y]
         add eax,[ProgressBar.sizex]
         add ebx,[ProgressBar.sizey]
         mov [v],eax    ;v=x+sizex
         mov [v2],ebx   ;v2=y+sizey

         ;dark lines
         mov eax,[ProgressBar.color3]
         and eax,0xffffff
         mov [Line.color],eax
         mov [Pixel.color],eax

         ;(x+1,y)-(x+sizex-1,y)
         mov eax,[ProgressBar.x]
         mov ebx,[ProgressBar.y]
         mov ecx,[v]
         add eax,1
         sub ecx,1
         mov [Line.x1],eax
         mov [Line.x2],ecx
         mov [Line.y1],ebx
         mov [Line.y2],ebx
         call DrawLine

         ;(x+sizex,y+1)-(x+sizex,y+sizey-1)
         mov eax,[v]
         mov ebx,[v2]
         mov ecx,[ProgressBar.y]
         add ecx,1
         sub ebx,1
         mov [Line.x1],eax
         mov [Line.x2],eax
         mov [Line.y1],ecx
         mov [Line.y2],ebx
         call DrawLine

         ;(x+1,y+sizey)-(x+sizex-1,y+sizey)
         mov eax,[ProgressBar.x]
         mov ebx,[v]
         mov ecx,[v2]
         add eax,1
         sub ebx,1
         mov [Line.x1],eax
         mov [Line.x2],ebx
         mov [Line.y1],ecx
         mov [Line.y2],ecx
         call DrawLine

         ;(x,y+1)-(x,y_sizey-1)
         mov eax,[ProgressBar.x]
         mov ebx,[ProgressBar.y]
         mov ecx,[v2]
         add ebx,1
         sub ecx,1
         mov [Line.x1],eax
         mov [Line.x2],eax
         mov [Line.y1],ebx
         mov [Line.y2],ecx
         call DrawLine

         ;(x+1,y+1)-(x+1,y+2)
         mov eax,[ProgressBar.x]
         mov ebx,[ProgressBar.y]
         mov ecx,ebx
         add eax,1
         add ebx,1
         add ecx,2
         mov [Line.x1],eax
         mov [Line.x2],eax
         mov [Line.y1],ebx
         mov [Line.y2],ecx
         call DrawLine

         ;(x+2,y+1)
         mov eax,[ProgressBar.x]
         mov ebx,[ProgressBar.y]
         add eax,2
         add ebx,1
         mov [Pixel.x],eax
         mov [Pixel.y],ebx
         call DrawPixel

         ;(x+1,y+sizey-1)-(x+1,y+sizey-2)
         mov eax,[ProgressBar.x]
         mov ebx,[v2]
         mov ecx,ebx
         add eax,1
         sub ebx,1
         sub ecx,2
         mov [Line.x1],eax
         mov [Line.x2],eax
         mov [Line.y1],ebx
         mov [Line.y2],ecx
         call DrawLine

         ;(x+2,y+sizey-1)
         mov eax,[ProgressBar.x]
         mov ebx,[v2]
         add eax,2
         sub ebx,1
         mov [Pixel.x],eax
         mov [Pixel.y],ebx
         call DrawPixel

         ;(x+sizex-1,y+1)-(x+sizex-1,y+2)
         mov eax,[v]
         mov ebx,[ProgressBar.y]
         mov ecx,ebx
         sub eax,1
         add ebx,1
         add ecx,2
         mov [Line.x1],eax
         mov [Line.x2],eax
         mov [Line.y1],ebx
         mov [Line.y2],ecx
         call DrawLine

         ;(x+sizex-2,y+1)
         mov eax,[v]
         mov ebx,[ProgressBar.y]
         sub eax,2
         add ebx,1
         mov [Pixel.x],eax
         mov [Pixel.y],ebx
         call DrawPixel

         ;(x+sizex-1,y+sizey-2)-(x+sizex-1,y+sizey-1)
         mov eax,[v]
         mov ebx,[v2]
         mov ecx,ebx
         sub eax,1
         sub ebx,2
         sub ecx,1
         mov [Line.x1],eax
         mov [Line.x2],eax
         mov [Line.y1],ebx
         mov [Line.y2],ecx
         call DrawLine

         ;(x+sizex-2,y+sizey-1)
         mov eax,[v]
         mov ebx,[v2]
         sub eax,2
         sub ebx,1
         mov [Pixel.x],eax
         mov [Pixel.y],ebx
         call DrawPixel

         mov eax,[colors_table1+10*3]
         and eax,0xffffff
         mov [Line.color],eax
         mov [Pixel.color],eax

         ;(x+3,y+1)-(x+sizex-3,y+1)
         mov eax,[ProgressBar.x]
         mov ebx,[v]
         mov ecx,[ProgressBar.y]
         add ecx,1
         add eax,3
         sub ebx,3
         mov [Line.x1],eax
         mov [Line.x2],ebx
         mov [Line.y1],ecx
         mov [Line.y2],ecx
         call DrawLine

         ;(x+2,y+2)
         mov eax,[ProgressBar.x]
         mov ebx,[ProgressBar.y]
         add eax,2
         add ebx,2
         mov [Pixel.x],eax
         mov [Pixel.y],ebx
         call DrawPixel

         ;(x+sizex-1,y+3)-(x+sizex-1,y+sizey-3)
         mov eax,[v]
         mov ebx,[v2]
         mov ecx,[ProgressBar.y]
         sub eax,1
         sub ebx,3
         add ecx,3
         mov [Line.x1],eax
         mov [Line.x2],eax
         mov [Line.y1],ecx
         mov [Line.y2],ebx
         call DrawLine

         ;(x+sizex-2,y+2)
         mov eax,[v]
         mov ebx,[ProgressBar.y]
         sub eax,2
         add ebx,2
         mov [Pixel.x],eax
         mov [Pixel.y],ebx
         call DrawPixel

         ;(x+3,y+sizey-1)-(x+sizex-3,y+sizey-1)
         mov eax,[ProgressBar.x]
         mov ebx,[v]
         mov ecx,[v2]
         sub ebx,3
         sub ecx,1
         add eax,3
         mov [Line.x1],eax
         mov [Line.x2],ebx
         mov [Line.y1],ecx
         mov [Line.y2],ecx
         call DrawLine

         ;(x+sizex-2,y+sizey-2)
         mov eax,[v]
         mov ebx,[v2]
         sub eax,2
         sub ebx,2
         mov [Pixel.x],eax
         mov [Pixel.y],ebx
         call DrawPixel

         ;(x+1,y+3)-(x+1,y+sizey-3)
         mov eax,[ProgressBar.x]
         mov ebx,[ProgressBar.y]
         mov ecx,[v2]
         add eax,1
         add ebx,3
         sub ecx,3
         mov [Line.x1],eax
         mov [Line.x2],eax
         mov [Line.y1],ebx
         mov [Line.y2],ecx
         call DrawLine

         ;(x+2,y+sizey-2)
         mov eax,[ProgressBar.x]
         mov ebx,[v2]
         add eax,2
         sub ebx,2
         mov [Pixel.x],eax
         mov [Pixel.y],ebx
         call DrawPixel

         ret

draw_progress:

         cmp [ProgressBar.progress],dword 0.02
         jae min_size_ok

         mov [ProgressBar.progress],dword 0.02
         min_size_ok:

         cmp [ProgressBar.progress],dword 1.0
         jle max_size_ok

         mov [ProgressBar.progress],dword 1.0
         max_size_ok:

         mov eax,[ProgressBar.sizex]
         sub eax,6
         mov [v2],eax

         fld [ProgressBar.progress]
         fild [v2]
         fmul st0,st1
         fistp [v3]
         fstp st0

         mov eax,[ProgressBar.color2]
         mov ebx,0xffffff
         mov ecx,[ProgressBar.x]
         mov edx,[ProgressBar.y]
         mov esi,[v3]
         mov edi,[ProgressBar.sizey]
         sub edi,4
         shr edi,1
         add ecx,3
         add edx,1

         call rectangle_gradient_up

         mov eax,[ProgressBar.color2]
         mov ebx,0xffffff
         mov ecx,[ProgressBar.x]
         mov edx,[ProgressBar.y]
         mov esi,[v3]
         mov edi,[ProgressBar.sizey]
         sub edi,4
         shr edi,1
         add ecx,3
         add edx,2
         add edx,edi

         call rectangle_gradient_down

         ret