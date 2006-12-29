;-----------------------------------------------------------
;-----------instruments of menu-----------------------------
;-----------------------------------------------------------
TakeInstruments:

	 ;|||||||||||||||||||FILE||||||||||||||||||||||
	 cmp [number_panel],7
	 jne no_file

	 ;<<<<<NEW>>>>>>>
	 cmp [number_menu],1
	 jne no_new

	 mov eax,80
	 mov ebx,100
	 mov ecx,150
	 mov edx,90
	 mov  esi,1
	 call draw_volume_rectangle

	 mov eax,177
	 mov ebx,115
	 mov ecx,40
	 mov edx,14
	 mov  esi,4
	 call draw_volume_rectangle

	 mov eax,177
	 mov ebx,115+24
	 mov ecx,40
	 mov edx,14
	 mov  esi,4
	 call draw_volume_rectangle

	 mov eax,175
	 mov ebx,170
	 mov ecx,40
	 mov edx,15
	 mov  esi,1
	 call draw_volume_rectangle

	 mov eax,90
	 mov ebx,118
	 mov ecx,0xffffff
	 mov edx,new_text1
	 mov esi,14
	 call print_text

	 mov eax,90
	 mov ebx,118+24
	 mov ecx,0xffffff
	 mov edx,new_text2
	 mov esi,14
	 call print_text

	 mov eax,185
	 mov ebx,175
	 mov ecx,0xffffff
	 mov edx,ok_text
	 mov esi,2
	 call print_text

	 mov eax,8
	 mov ebx,177*65536+40
	 mov ecx,115*65536+14
	 mov edx,1000000000000000000000000000000b
	 add edx,1000
	 int 0x40

	 mov eax,8
	 mov ebx,177*65536+40
	 mov ecx,139*65536+14
	 mov edx,1000000000000000000000000000000b
	 add edx,1001
	 int 0x40

	 mov eax,8
	 mov ebx,175*65536+40
	 mov ecx,170*65536+15
	 mov edx,1000000000000000000000000000000b
	 add edx,1002
	 int 0x40

	 no_activate_space:

	 mov eax,10
	 int 0x40

	 cmp eax,1
	 je exit_new

	 cmp eax,3
	 jne no_activate_space

	 mov eax,17
	 int 0x40
	 shr eax,8

	 cmp eax,1000
	 jl no_activate_space

	 cmp eax,1000
	 jne no_picture_size_x

	 mov eax,180
	 mov ebx,119
	 mov ecx,5
	 call dialog_line

	 mov eax,string_
	 mov ebx,13
	 call find_symvol

	 dec eax
	 mov [length_number],eax
	 mov eax,string_
	 call value

	 mov [Picture_SizeX],eax

	 jmp no_activate_space

	 no_picture_size_x:

	 cmp eax,1001
	 jne no_picture_size_y

	 mov eax,180
	 mov ebx,119+24
	 mov ecx,5
	 call dialog_line

	 mov eax,string_
	 mov ebx,13
	 call find_symvol

	 dec eax
	 mov [length_number],eax
	 mov eax,string_
	 call value

	 mov [Picture_SizeY],eax

	 jmp no_activate_space

	 no_picture_size_y:

	 cmp eax,1002
	 jne no_activate_space

	 exit_new:

	 mov eax,8
	 mov ebx,175*65536+40
	 mov ecx,170*65536+15
	 mov edx,11100000000000000000000000000000b
	 add edx,1002
	 int 0x40

	 mov eax,8
	 mov ebx,177*65536+40
	 mov ecx,139*65536+14
	 mov edx,11100000000000000000000000000000b
	 add edx,1001
	 int 0x40

	 mov eax,8
	 mov ebx,177*65536+40
	 mov ecx,115*65536+14
	 mov edx,11100000000000000000000000000000b
	 add edx,1000
	 int 0x40

	 ;get memory for new picture
	 mov eax,[Picture_SizeX]
	 mov ebx,[Picture_SizeY]

	 imul eax,ebx
	 lea eax,[eax+eax*2]

	 mov ebx,[ScreenPointer]
	 add ebx,(1200*1000*3)+50*(20*20*3)+500000

	 mov [PointerToPicture],ebx
	 mov [PointerToCopyPicture],ebx
	 mov [PointerToCopyPicture2],ebx
	 mov [PointerToEditBufer],ebx
	 mov [PointerToSpriteBufer],ebx

	 add [PointerToCopyPicture],eax

	 add [PointerToCopyPicture2],eax
	 add [PointerToCopyPicture2],eax

	 add [PointerToEditBufer],eax
	 add [PointerToEditBufer],eax
	 add [PointerToEditBufer],eax

	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax

	 call GetMemory

	 and [save_flag],0
	 and [number_panel],0
	 and [number_menu],0

	 call cleare_work_arrea
	 call drawwin

	 jmp still
	 no_new:

	 ;<<<<<OPEN>>>>>>>
	cmp [number_menu],2
	 jne no_open

	 opendialog drawwin,open_1,open_2,file_path
	 open_1:

	 mov eax,40
	 mov ebx,100111b
	 int 0x40

	 mov eax,file_path
	 mov ebx,[ScreenPointer]
	 add ebx,0x10000

	 call load_heading_of_file

	 mov esi,[ScreenPointer]
	 add esi,0x10000
	 ;-------------is this BMP file ?----------------
	 xor eax,eax
	 mov ax,[esi]
	 mov [type],ax

	 cmp [type],word 'BM'
	 jne no_bmp_file

	 xor eax,eax
	 xor ebx,ebx
	 mov eax,[esi+18]
	 mov ebx,[esi+22]
	 mov [Picture_SizeX],eax
	 mov [Picture_SizeY],ebx

	 jmp exit_type

	 no_bmp_file:

	 ;-------------is this GIF file ?----------------
	 xor eax,eax
	 mov ax,[esi]
	 mov [type],ax

	 cmp [type],'GI'
	 jne no_gif_file

	 add esi,6
	 xor eax,eax
	 xor ebx,ebx
	 mov ax,word[esi]
	 mov bx,word[esi+2]
	 mov [Picture_SizeX],eax
	 mov [Picture_SizeY],ebx

	 jmp exit_type

	 no_gif_file:

	 jmp no_unpakcing_file

	 exit_type:

	 ;----------------------------------------------------------
	 ;Get momory for unpacking picture and for picture's bufers
	 ;----------------------------------------------------------
	 mov eax,[Picture_SizeX]
	 mov ebx,[Picture_SizeY]

	 imul eax,ebx
	 lea eax,[eax+eax*2]

	 mov ebx,[ScreenPointer]
	 add ebx,(1200*1000*3)+50*(20*20*3)+500000

	 mov [PointerToPicture],ebx
	 mov [PointerToCopyPicture],ebx
	 mov [PointerToCopyPicture2],ebx
	 mov [PointerToEditBufer],ebx
	 mov [PointerToSpriteBufer],ebx

	 add [PointerToCopyPicture],eax

	 add [PointerToCopyPicture2],eax
	 add [PointerToCopyPicture2],eax

	 add [PointerToEditBufer],eax
	 add [PointerToEditBufer],eax
	 add [PointerToEditBufer],eax

	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax
	 add [PointerToSpriteBufer],eax

	 call GetMemory
	 ;----------------------------------------------------------
	 ;--------------------Load file in memory-------------------
	 ;----------------------------------------------------------

	 mov eax,file_path
	 mov ebx,[PointerToCopyPicture]
	 add ebx,1000

	 call load_file

	 ;----------------------------------------------------------
	 ;-------------------Unpacking picture----------------------
	 ;----------------------------------------------------------
	 mov esi,[PointerToCopyPicture]
	 add esi,1000
	 mov edi,[PointerToPicture]
	 mov eax,[ScreenPointer]

	 cmp [type],'BM'
	 jne no_unpakcing_bmp_file
	    ;BMP DECODER
	    call bmptoimg
	    mov [save_flag],1
	 no_unpakcing_bmp_file:


	 cmp [type],'GI'
	 jne no_unpakcing_file
	   ;GIF DECODER
	   sub edi,12
	   call ReadGIF
	   mov [save_flag],1
	 no_unpakcing_file:

	 call load_icons
	 call drawwin

	 open_2:

	 mov [Scroll1CoordinatX],9
	 mov [Scroll2CoordinatY],89
	 and [number_panel],0
	 and [number_menu],0

	 jmp still

	 no_open:

	 ;<<<<<<<<<<<SAVE>>>>>>>>>>>>
	 cmp [number_menu],3
	 jne no_save_

	 mov al,[save_flag]

	 test al,al
	 jz no_save_file

	 jmp save_enter

	 no_save_file:

	 and [number_panel],0
	 and [number_menu],0

	 jmp still

	 no_save_:

	 ;<<<<<<<<<SAVE AS>>>>>>>>>>>
	 cmp [number_menu],4
	 jne no_save

	 savedialog drawwin,save1,save2,file_path

	 save1:

	 save_enter:
	 mov eax,40
	 mov ebx,100111b
	 int 0x40

	 call analizing_picture_to_palette
	 ;eax => number of colors in picture
	 mov ebx,[PointerToPicture]
	 mov ecx,[PointerToEditBufer]
	 mov edx,[PointerToPalette]
	 mov esi,[Picture_SizeX]
	 mov edi,[Picture_SizeY]

	 call coding_bmp

	 mov eax,file_path
	 mov ecx,ebx
	 mov ebx,[PointerToEditBufer]
	 push eax
	 push ebx
	 push ecx

	 call save_file

	 mov [save_flag],1
	 and [number_panel],0
	 and [number_menu],0
	 call drawwin

	 jmp still

	 save2:

	 and [number_panel],0
	 and [number_menu],0

	 jmp still
	 no_save:

	 ;<<<<<<EXIT>>>>>>>
	 cmp [number_menu],5
	 jne no_exit_program

	 mov eax,-1
	 int 0x40

	 no_exit_program:

	 no_file:

	 ;|||||||||||||||||||||||||||||EDIT|||||||||||||||||||||||||

	 ;<<<<<<<<<<UNDO>>>>>>>>>>
	 cmp [number_panel],6
	 jne no_edit

	 cmp [number_menu],1
	 jne no_undo

	 cmp [number_undo],1
	 jne no_one____

	 mov esi,[PointerToCopyPicture]

	 no_one____:

	 cmp [number_undo],2
	 jne no_two____

	 mov esi,[PointerToCopyPicture2]

	 no_two____:

	 mov edi,[PointerToPicture]
	 mov ecx,[Picture_SizeX]
	 imul ecx,[Picture_SizeY]
	 lea ecx,[ecx+ecx*2]
	 add ecx,4
	 shr ecx,2
	 cld
	 rep movsd
	 call MovePictureToWorkScreen

	 dec [number_undo]
	 jnz no_null_undo

	 mov [number_undo],1

	 no_null_undo:

	 and [number_panel],0
	 and [number_menu],0
	 jmp still
	 no_undo:

	 ;<<<<<<<<<<COPY>>>>>>>>>>

	 cmp [number_menu],2
	 jne no_copy

	 cmp [instrument_used],1
	 jne no_copy
	 cmp [Activate_instrument],0
	 jne no_copy

	mov eax,[OldX]
	mov ebx,[OldY]

	cmp eax,[rectangular_shade_x]
	jl no_remove_x_copy

	mov ecx,[rectangular_shade_x]
	mov [OldX],ecx			  ; OldX <-----> rectangulare_shade_x
	mov [rectangular_shade_x],eax

	no_remove_x_copy:

	cmp ebx,[rectangular_shade_y]
	jl no_remove_y_copy

	mov ecx,[rectangular_shade_y]
	mov [OldY],ecx			  ; OldY <-----> rectangulare_shade_y
	mov [rectangular_shade_y],ebx

	no_remove_y_copy:

	 mov eax,[OldX]
	 mov ebx,[OldY]
	 mov ecx,[rectangular_shade_x]
	 mov edx,[rectangular_shade_y]
	 inc eax
	 inc ebx
	 dec ecx
	 dec edx

	 mov [x],eax
	 mov [y],ebx
	 mov esi,eax
	 mov edi,ebx
	 mov [Dx_],1
	 mov [Dy_],1

	 sub ecx,eax
	 jnz no_signum_fill_r_x_copy

	 mov [Dx_],-1

	 no_signum_fill_r_x_copy:

	 sub edx,ebx
	 jnz no_signum_fill_r_y_copy

	 mov [Dy_],-1

	 no_signum_fill_r_y_copy:

	 mov ebx,[rectangular_shade_y]
	 sub ebx,edi

	 mov edx,[PointerToEditBufer]
	 mov [y],edi
	 loop_fill_rectangle_y_copy:

	 mov [x],esi
	 loop_fill_rectangle_x_copy:

	 push esi
	 push edi

	 mov eax,[PointerToPicture]
	 mov ebx,[Picture_SizeX]
	 mov esi,[x]
	 mov edi,[y]
	 call GetColorOfPixel

	 mov [edx],ax
	 shr eax,16
	 mov [edx+2],al

	 pop edi
	 pop esi

	 add edx,3

	 mov eax,[x]
	 add eax,[Dx_]
	 mov [x],eax

	 cmp eax,[rectangular_shade_x]
	 jl loop_fill_rectangle_x_copy

	 mov eax,[y]
	 add eax,[Dy_]
	 mov [y],eax

	 cmp eax,[rectangular_shade_y]
	 jl loop_fill_rectangle_y_copy

	 call MovePictureToWorkScreen

	 and [number_panel],0
	 and [number_menu],0
	 mov [DrawSprite_flag],1

	 jmp still

	 no_copy:

	 ;<<<<<<<<<<PASTE>>>>>>>>
	 cmp [number_menu],3
	 jne no_paste

	 cmp [instrument_used],1
	 jne no_paste

	 cmp [Activate_instrument],0
	 jne no_paste

	 mov eax,[OldX]
	 mov ebx,[OldY]

	 cmp eax,[rectangular_shade_x]
	 jl no_remove_x_paste

	 mov ecx,[rectangular_shade_x]
	 mov [OldX],ecx 		   ; OldX <-----> rectangulare_shade_x
	 mov [rectangular_shade_x],eax

	 no_remove_x_paste:

	 cmp ebx,[rectangular_shade_y]
	 jl no_remove_y_paste

	 mov ecx,[rectangular_shade_y]
	 mov [OldY],ecx 		   ; OldY <-----> rectangulare_shade_y
	 mov [rectangular_shade_y],ebx

	 no_remove_y_paste:

	 mov eax,[OldX]
	 mov ebx,[OldY]
	 mov ecx,[rectangular_shade_x]
	 mov edx,[rectangular_shade_y]
	 inc eax
	 inc ebx
	 dec ecx
	 dec edx

	 mov [x],eax
	 mov [y],ebx
	 mov esi,eax
	 mov edi,ebx
	 mov [Dx_],1
	 mov [Dy_],1

	 sub ecx,eax
	 jnz no_signum_fill_r_x_paste

	 mov [Dx_],-1

	 no_signum_fill_r_x_paste:

	 sub edx,ebx
	 jnz no_signum_fill_r_y_paste

	 mov [Dy_],-1

	 no_signum_fill_r_y_paste:

	 mov edx,[PointerToEditBufer]
	 mov [y],edi

	 loop_fill_rectangle_y_paste:

	 mov [x],esi
	 loop_fill_rectangle_x_paste:

	 push esi
	 push edi

	 mov ecx,[edx]
	 and ecx,0xffffff ;color

	 mov eax,[PointerToPicture]
	 mov ebx,[Picture_SizeX]
	 mov esi,[x]
	 mov edi,[y]
	 call PutPixel

	 pop edi
	 pop esi

	 add edx,3

	 mov eax,[x]
	 add eax,[Dx_]
	 mov [x],eax
	 cmp eax,[rectangular_shade_x]
	 jl loop_fill_rectangle_x_paste

	 mov eax,[y]
	 add eax,[Dy_]
	 mov [y],eax
	 cmp eax,[rectangular_shade_y]

	 jl loop_fill_rectangle_y_paste

	 call MovePictureToWorkScreen
	 and [number_panel],0
	 and [number_menu],0
	 mov [Paste_flag],1

	 jmp still

	 no_paste:

	 ;<<<<<<<<<<CUT>>>>>>>>>>
	 cmp [number_menu],4
	 jne no_cut

	 cmp [instrument_used],1
	 jne no_cut

	 cmp [Activate_instrument],0
	 jne no_cut

	 mov eax,[OldX]
	 mov ebx,[OldY]

	 cmp eax,[rectangular_shade_x]
	 jl no_remove_x_cut

	 mov ecx,[rectangular_shade_x]
	 mov [OldX],ecx 		   ; OldX <-----> rectangulare_shade_x
	 mov [rectangular_shade_x],eax

	 no_remove_x_cut:

	 cmp ebx,[rectangular_shade_y]
	 jl no_remove_y_cut

	 mov ecx,[rectangular_shade_y]
	 mov [OldY],ecx 		   ; OldY <-----> rectangulare_shade_y
	 mov [rectangular_shade_y],ebx

	 no_remove_y_cut:

	 mov eax,[OldX]
	 mov ebx,[OldY]
	 mov ecx,[rectangular_shade_x]
	 mov edx,[rectangular_shade_y]
	 inc eax
	 inc ebx
	 dec ecx
	 dec edx

	 mov [x],eax
	 mov [y],ebx
	 mov esi,eax
	 mov edi,ebx
	 mov [Dx_],1
	 mov [Dy_],1

	 sub ecx,eax
	 jnz no_signum_fill_r_x

	 mov [Dx_],-1

	 no_signum_fill_r_x:

	 sub edx,ebx
	 jnz no_signum_fill_r_y

	 mov [Dy_],-1

	 no_signum_fill_r_y:

	 mov [y],edi
	 loop_fill_rectangle_y:

	 mov [x],esi
	 loop_fill_rectangle_x:

	 push esi
	 push edi

	 mov eax,[PointerToPicture]
	 mov ebx,[Picture_SizeX]
	 mov ecx,dword 0xffffff
	 mov esi,[x]
	 mov edi,[y]
	 call PutPixel

	 pop edi
	 pop esi

	 mov eax,[x]
	 add eax,[Dx_]
	 mov [x],eax

	 cmp eax,[rectangular_shade_x]
	 jl loop_fill_rectangle_x

	 mov eax,[y]
	 add eax,[Dy_]
	 mov [y],eax

	 cmp eax,[rectangular_shade_y]
	 jl loop_fill_rectangle_y

	 call MovePictureToWorkScreen

	 and [number_panel],0
	 and [number_menu],0

	 jmp still
	 no_cut:

	 ;<<<<<<CLEARE ALL>>>>>>
	 cmp [number_menu],5
	 jne no_cleare_all

	 call cleare_work_arrea

	 call MovePictureToWorkScreen

	 and [number_panel],0
	 and [number_menu],0

	 jmp still
	 no_cleare_all:
	 ;<<<<<<TO ALLOCATE ALL>>>>>>

	 cmp [number_menu],6
	 jne no_to_allocate_all

	 mov [OldX],1
	 mov [OldY],1
	 mov eax,[Picture_SizeX]
	 mov ebx,[Picture_SizeY]
	 dec eax
	 dec ebx
	 mov [rectangular_shade_x],eax
	 mov [rectangular_shade_y],ebx
	 mov [instrument_used],1
	 mov [Activate_instrument],1
	 mov [Current_instrument],30
	 mov [crossing],0
	 and [number_panel],0
	 and [number_menu],0
	 ;call TakeButtonInstruments
	 ;call MovePictureToWorkScreen
	 jmp still

	 no_to_allocate_all:

	 no_edit:


	 jmp still

	 ret
