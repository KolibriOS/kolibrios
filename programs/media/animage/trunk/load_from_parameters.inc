;loading file from parameters if parametrs <> 0
;IN
; eax- pointer to parameters
; ebx- pointer to path of file
;OUT
;value not returned

check_loading_from_parameters:

     cmp [eax],byte 0
     jz no_parameters

     ;copy file name
     mov esi,eax
     mov edi,ebx
     mov ecx,256
     rep movsb

     ;load file in memory
     
     call load_picture

     no_parameters:
     
     ret


; load picture from file to memory

load_picture:

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
	 jne no_bmp_file_1

	 xor eax,eax
	 xor ebx,ebx
	 mov eax,[esi+18]
	 mov ebx,[esi+22]
	 mov [Picture_SizeX],eax
	 mov [Picture_SizeY],ebx

	 jmp exit_type_1

	 no_bmp_file_1:

	 ;-------------is this GIF file ?----------------
	 xor eax,eax
	 mov ax,[esi]
	 mov [type],ax

	 cmp [type],'GI'
	 jne no_gif_file_1

	 add esi,6
	 xor eax,eax
	 xor ebx,ebx
	 mov ax,word[esi]
	 mov bx,word[esi+2]
	 mov [Picture_SizeX],eax
	 mov [Picture_SizeY],ebx

	 jmp exit_type_1

	 no_gif_file_1:

	 jmp no_unpakcing_file_1

	 exit_type_1:

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
	 jne no_unpakcing_bmp_file_1
	    ;BMP DECODER
	    call bmptoimg
	    mov [save_flag],1
	 no_unpakcing_bmp_file_1:


	 cmp [type],'GI'
	 jne no_unpakcing_file_1
	   ;GIF DECODER
	   sub edi,8
	   call ReadGIF
	   mov [save_flag],1
	 no_unpakcing_file_1:

	 call MovePictureToWorkScreen

	 mov [Scroll1CoordinatX],9
	 mov [Scroll2CoordinatY],89    
     ret
