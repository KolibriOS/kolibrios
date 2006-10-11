;**************************************************************************
;**********************DECODING BMP FILE(1,4,8,24 bits)*********************
;***************************************************************************
; BMPTOIMG -Convert BMP format TO IMG format
;***************************************************************************
bmptoimg:

    mov [bmp_load_area],esi
    mov [img_dest_area],edi
    xor eax,eax
    mov ax,word[esi+28]
    mov ebx,[esi+14]
    mov ecx,[esi+18]
    mov edx,[esi+22]
    mov [bmp_bits_per_pixel],ax
    mov [bmp_first_structure_size],ebx
    mov [Bmp_SizeY],edx
    mov [Bmp_SizeX],ecx

    xor eax,eax
    mov ax,[esi+28]
    mul  dword [esi+18]
    add  eax,31
    shr  eax,5
    mov  dword [bmptoimg_data_area_dwps],eax  ;dwps-doublewords per string
    shl  eax,2
    mov  dword [bmptoimg_data_area_bps],eax   ;bps-bytes per string

    cmp dword [esi+34],0
    jne  yespicsize  ;if picture size is defined
    mul dword [esi+22]
    mov dword [esi+34],eax

  yespicsize:

    mov  eax,[bmp_load_area]
    add  eax, [esi+10]	 ;how mach bytes to begin bitmap
    add  eax, [esi+34]	 ;size of bitmap in BMP file
    mov  dword [bmptoimg_data_area_eop],eax   ;eop-end of picture in file
    ;calculate bytes per string
    mov  eax, [esi+18]
    lea  eax,[eax+2*eax]   ;3x pixels in eax
    mov [bmp_bytes_per_string],eax

    mov esi,dword [bmptoimg_data_area_eop]
    sub esi,dword [bmptoimg_data_area_bps]

    mov ebp,[img_dest_area]
    mov edi,[img_dest_area]
    mov ebx,[bmp_load_area]
    add ebx, [bmp_first_structure_size]
    add ebx,14		;in ebx start of color table


    cmp [bmp_bits_per_pixel],24
    je	convert_to_24bpp

    cmp [bmp_bits_per_pixel],8
    je convert_to_8bpp

    cmp [bmp_bits_per_pixel],4
    je convert_to_4bpp

    cmp [bmp_bits_per_pixel],1
    je convert_to_1bpp

    jmp end_bmp

;--------------------------------------------------
;-----------Decoding 24 bit BMP file---------------
;--------------------------------------------------
convert_to_24bpp:

  mov ebx,[Bmp_SizeY]
  loop_convert_to_24bpp_y:

    mov edi,ebp
    mov  ecx,[bmptoimg_data_area_dwps]

     loop_convert_to_24bpp_x:

       mov edx,[esi]
       mov [edi],edx
       add esi,4
       add edi,4

     dec ecx
     jnz loop_convert_to_24bpp_x

    sub  esi,[bmptoimg_data_area_bps]
    sub  esi,[bmptoimg_data_area_bps]

    add  ebp,eax
    dec ebx
    jnz loop_convert_to_24bpp_y

    jmp  end_bmp
;-----------------------------------------------------
;--------------Decoding 8 bits BMP file---------------
;-----------------------------------------------------
 convert_to_8bpp:

    mov ebp,[Bmp_SizeY]
    loop_convert_8bpp_y:

      mov ecx,[bmptoimg_data_area_bps]
      push edi

      loop_convert_8bpp_x:

	 xor eax,eax
	 mov al,byte [esi]
	 call converttable
	 inc esi
	 add edi,3

      dec ecx
      jnz loop_convert_8bpp_x

      pop edi

    add edi,[bmp_bytes_per_string]
    sub esi,[bmptoimg_data_area_bps]
    sub esi,[bmptoimg_data_area_bps]

    dec ebp
    jnz loop_convert_8bpp_y

    jmp end_bmp
;-----------------------------------------------------
;--------------Decoding 4 bits BMP file---------------
;-----------------------------------------------------
 convert_to_4bpp:

    mov ebp,[Bmp_SizeY]
    loop_convert_4bpp_y:

      mov ecx,[bmptoimg_data_area_bps]
      push edi

      loop_convert_4bpp_x:

	 mov [Bmp_save1],ecx
	 xor eax,eax
	 mov al,byte [esi]
	 xor ecx,ecx
	 mov cl,al
	 shr al,4   ;first pixel in byte
	 and cl,0xf ;second pixel in byte
	 call converttable   ;draw first pixel of byte
	 mov eax,ecx	       ;move second pixel to register al and draw
	 add edi,3
	 call converttable   ;draw second pixel of byte
	 add edi,3
	 mov ecx,[Bmp_save1]
	 inc esi

      dec ecx
      jnz loop_convert_4bpp_x

      pop edi

    add edi,[bmp_bytes_per_string]
    sub esi,[bmptoimg_data_area_bps]
    sub esi,[bmptoimg_data_area_bps]

    dec ebp
    jnz loop_convert_4bpp_y

    jmp end_bmp
;-----------------------------------------------------
;---------------Decoding 1 bit BMP file---------------
;-----------------------------------------------------
 convert_to_1bpp:

    mov ebp,[Bmp_SizeY]
    loop_convert_1bpp_y:

      mov ecx,[bmptoimg_data_area_bps]
      push edi

      loop_convert_1bpp_x:

	 xor eax,eax
	 mov al,byte [esi]
	 mov [Bmp_save1],ecx
	 mov  ecx,eax
	 mov  edx,7
	 nextbit:
	    xor  eax,eax
	    bt	 ecx,edx
	    jnc  noaddelem
	    inc  eax
	    noaddelem:

	    push edx
	    call converttable
	    pop  edx

	    add  edi,3
	    dec  edx
	 jns nextbit
	 mov ecx,[Bmp_save1]
	 inc esi
      dec ecx
      jnz loop_convert_1bpp_x

      pop edi

    add edi,[bmp_bytes_per_string]
    sub esi,[bmptoimg_data_area_bps]
    sub esi,[bmptoimg_data_area_bps]

    dec ebp
    jnz loop_convert_1bpp_y

    jmp end_bmp
;-----------------------------------------------------
  converttable:
    shl  eax,2
    add  eax,ebx
    mov  edx, dword [eax]
    mov [edi],edx
    ret
;-----------------------------------------------------
; DATA AREA
bmptoimg_data_area_bps	    dd 0
bmptoimg_data_area_dwps     dd 0
bmptoimg_data_area_eop	    dd 0
bmp_load_area		    dd 0
img_dest_area		    dd 0
bmp_bits_per_pixel	    dw 0
bmp_first_structure_size    dd 0
bmp_bytes_per_string	    dd 0

end_bmp:

ret

;***************************************************************************
;*******************CODING BMP FILE(1,4,8,24 bits)**************************
;***************************************************************************

;-------------------------autor andrew_programmer---------------------------

coding_bmp:

	 mov [PointerToImage],ebx
	 mov [WhereCodingBMP],ecx
	 mov [BmpPalette],edx
	 mov [Bmp_SizeX],esi
	 mov [Bmp_SizeY],edi
	 ;**********************************************
	 ;******************1 bit BMP*******************
	 ;**********************************************
	 cmp eax,2
	 ja no_monohrom_colors
	 mov esi,[BmpPalette]
	 mov edi,[WhereCodingBMP]
	 add edi,54
	 mov eax,[esi]	 ;first color
	 mov ebx,[esi+4] ;second color
	 mov [edi],eax
	 mov [edi+4],ebx

	 ;coding image to bmp 1 bit format
	 mov esi,[PointerToImage]
	 mov edx,[WhereCodingBMP]
	 mov ebx,[Bmp_SizeX]
	 add ebx,31		   ;picture_size_x+31
	 shr ebx,5		   ;((picture_size_x)+31)/32
	 mov [Bmp_doublewords],ebx ;double words in string
	 shl ebx,2
	 mov [Bmp_bytes_per_string],ebx ;bytes per string
	 mov ecx,[Bmp_SizeY]
	 dec ecx
	 imul ebx,ecx
	 add edx,54+8
	 add edx,ebx		    ;in edx pointer to area for coding

	 mov ebp,[Bmp_bytes_per_string]

	 mov ebx,[Bmp_SizeY]
	 mov esi,[PointerToImage]
	 mov edi,edx

	 mov edx,[Bmp_SizeX]
	 lea edx,[edx+edx*2]

	 mov [Bmp_Counter],7
	 and [Bmp_Counter2],0
	 copy_lines_1:

	   push esi
	   mov ecx,[Bmp_bytes_per_string]
	   shl ecx,3	      ;(Bmp_bytes_per_string)*8

	   rep_movsb_1:

	      mov eax,[esi]
	      and eax,0xffffff
	      push esi
	      push ecx
	      push ebx
	      mov esi,[BmpPalette]
	      xor ecx,ecx
		find_color_in_palette_1:
		   mov ebx,[esi]
		   and ebx,0xffffff
		   cmp eax,ebx		 ;color fined ?
		   je color_fined_1
		   add esi,4
		   inc ecx
		   cmp ecx,256
		   jl find_color_in_palette_1
		   color_fined_1:
	      mov [Bmp_Counter3],ecx	 ;number color in palette
	      pop ebx
	      pop ecx
	      pop esi
	      mov eax,[Bmp_Counter3]

	      test eax,eax
	      jz no_change_bit_in_byte

	      push ecx
	      mov ecx,[Bmp_Counter]
	      bts [Bmp_Counter2],ecx
	      pop ecx

	      no_change_bit_in_byte:

	      dec [Bmp_Counter]
	      jns no_minus_in_counter

	      push eax
	      mov eax,[Bmp_Counter2]
	      mov [edi],al
	      inc edi
	      mov [Bmp_Counter],7
	      and [Bmp_Counter2],0     ;obnulyaem byte
	      pop eax

	      no_minus_in_counter:

	      add esi,3
	   dec ecx
	   jnz rep_movsb_1
	   pop esi

	 add esi,edx
	 sub edi,ebp
	 sub edi,ebp

	 dec ebx
	 jnz copy_lines_1

	 mov edi,[WhereCodingBMP]
	 mov ebx,[Bmp_bytes_per_string]
	 imul ebx,[Bmp_SizeY]
	 add ebx,(54+8)

	 ;crate list of bmp description
	 mov [edi],word 'BM'
	 mov [edi+2],ebx
	 mov [edi+10],dword 54+8 ;where bigin bmp imige
	 mov [edi+14],dword 40
	 mov edx,[Bmp_SizeX]
	 mov ecx,[Bmp_SizeY]
	 mov [edi+18],edx    ;picture size x
	 mov [edi+22],ecx    ;picture size y
	 mov [edi+26],word 1
	 mov [edi+28],word 1 ;bits per pixel
	 mov [edi+30],dword 0
	 mov edx,[Bmp_bytes_per_string]
	 imul edx,ecx
	 mov [edi+34],edx
	 mov [edi+38],dword 0
	 mov [edi+42],dword 0
	 mov [edi+46],dword 0

	 ret

	 no_monohrom_colors:

	 ;**********************************************
	 ;*****************4 bits BMP*******************
	 ;**********************************************
	 cmp eax,16
	 ja no_16_colors
	 ;copy 16 colors palette
	 mov esi,[BmpPalette]
	 mov edi,[WhereCodingBMP]
	 add edi,54
	 mov ecx,16
	 rep movsd

	 ;coding image to bmp 4 bits format
	 mov esi,[PointerToImage]
	 mov edx,[WhereCodingBMP]
	 mov ebx,[Bmp_SizeX]
	 shl ebx,2	       ;4*(picture_size_x)
	 add ebx,31	       ;4*(picture_size_x)+31
	 shr ebx,5	       ;(4*(picture_size_x)+31)/32
	 mov [Bmp_doublewords],ebx ;double words in string
	 shl ebx,2
	 mov [Bmp_bytes_per_string],ebx ;bytes per string
	 mov ecx,[Bmp_SizeY]
	 dec ecx
	 imul ebx,ecx
	 add edx,54+64
	 add edx,ebx

	 mov ebp,[Bmp_bytes_per_string]

	 mov ebx,[Bmp_SizeY]
	 mov [Bmp_Counter2],ebx

	 mov edi,edx

	 xor ebx,ebx
	 copy_lines_4:

	 mov ecx,[Bmp_bytes_per_string]
	 shl ecx,1

	 and [Bmp_Counter3],0
	 push esi

	  rep_movsb_4:
	  mov eax,[esi]
	  and eax,0xffffff

	    mov [Bmp_save1],esi
	    mov [Bmp_save2],ecx
	    mov [Bmp_save3],ebx
	    mov esi,[BmpPalette]
	    xor ecx,ecx
	     find_color_in_palette_:
	       mov ebx,[esi]
	       and ebx,0xffffff
	       cmp eax,ebx	     ;color fined ?
	       je color_fined_
		 add esi,4
		 inc ecx
	       cmp ecx,16
	       jl find_color_in_palette_
	       color_fined_:
	      mov [Bmp_Counter],ecx	;number color in palette
	    mov esi,[Bmp_save1]
	    mov ecx,[Bmp_save2]
	    mov ebx,[Bmp_save3]

	  xor eax,eax
	  mov eax,[Bmp_Counter]
	  shl bl,4
	  add bl,al

	   mov eax,[Bmp_Counter3]
	   and eax,1b
	   test eax,eax 	     ;next block ready ?
	   jz no_ready
	    mov [edi],bl	      ;4 bit color
	    inc edi
	   no_ready:

	  add esi,3
	  inc [Bmp_Counter3]

	  dec ecx
	  jnz rep_movsb_4

	 pop esi

	 add esi,[Bmp_SizeX]
	 add esi,[Bmp_SizeX]
	 add esi,[Bmp_SizeX]

	 sub edi,ebp
	 sub edi,ebp

	 dec [Bmp_Counter2]
	 jnz copy_lines_4

	 ;total size of bmp file
	 mov edi,[WhereCodingBMP]
	 mov ebx,[Bmp_bytes_per_string]
	 imul ebx,[Bmp_SizeY]
	 add ebx,(54+64)

	 ;crate list of bmp description
	 mov [edi],word 'BM'
	 mov [edi+2],ebx
	 mov [edi+10],dword 54+64
	 mov [edi+14],dword 40
	 mov edx,[Bmp_SizeX]
	 mov ecx,[Bmp_SizeY]
	 mov [edi+18],edx
	 mov [edi+22],ecx
	 mov [edi+26],word 1
	 mov [edi+28],word 4
	 mov [edi+30],dword 0
	 mov edx,[Bmp_bytes_per_string]
	 imul edx,ecx
	 mov [edi+34],edx
	 mov [edi+38],dword 0
	 mov [edi+42],dword 0
	 mov [edi+46],dword 0

	 ret
	 no_16_colors:

	 ;**********************************************
	 ;******************8 bits BMP******************
	 ;**********************************************

	 cmp eax,256
	 ja no_8_bits_per_pixel
	 ;copy palette
	 mov esi,[BmpPalette]
	 mov edi,[WhereCodingBMP]
	 add edi,54
	 mov ecx,256
	 rep movsd

	 ;coding image to bmp 8 bits format
	 mov esi,[PointerToImage]
	 mov edx,[WhereCodingBMP]
	 mov ebx,[Bmp_SizeX]
	 shl ebx,3	       ;8*(picture_size_x)
	 add ebx,31	       ;8*(picture_size_x)+31
	 shr ebx,5	       ;(8*(picture_size_x)+31)/32
	 mov [Bmp_doublewords],ebx ;double words in string
	 shl ebx,2
	 mov [Bmp_bytes_per_string],ebx ;bytes per string

	 mov ecx,[Bmp_SizeY]
	 dec ecx
	 imul ebx,ecx
	 add edx,(1024+54)
	 add edx,ebx	       ;in edx pointer to copy bitmap arrea

	 mov ebp,[Bmp_bytes_per_string]
	 shl ebp,1

	 mov ebx,[Bmp_SizeY]
	 mov edi,edx

	 copy_lines_8:

	 mov ecx,[Bmp_bytes_per_string]
	 mov [Bmp_save1],esi
	  rep_movsb_8:
	  mov eax,[esi]
	  and eax,0xffffff
	  push esi
	  push ecx
	  push ebx
	   mov esi,[BmpPalette]
	   xor ecx,ecx
	    find_color_in_palette:
	    mov ebx,[esi]
	    and ebx,0xffffff
	    cmp eax,ebx 	  ;color fined ?
	    je color_fined
	    add esi,4
	    inc ecx
	    cmp ecx,256
	    jl find_color_in_palette
	    color_fined:
	    mov [Bmp_Counter],ecx     ;number color in palette
	  pop ebx
	  pop ecx
	  pop esi
	  mov eax,[Bmp_Counter]
	  mov [edi],al		  ;8 bit color
	  add esi,3
	  inc edi

	  dec ecx
	  jnz rep_movsb_8

	 mov esi,[Bmp_save1]

	 add esi,[Bmp_SizeX]
	 add esi,[Bmp_SizeX]
	 add esi,[Bmp_SizeX]

	 sub edi,ebp
	 dec ebx
	 jnz copy_lines_8

	 ;total size of bmp file
	 mov edi,[WhereCodingBMP]
	 mov ebx,[Bmp_bytes_per_string]
	 imul ebx,[Bmp_SizeY]
	 add ebx,54+1024

	 ;crate list of bmp description
	 mov [edi],word 'BM'
	 mov [edi+2],ebx
	 mov [edi+10],dword 54+1024
	 mov [edi+14],dword 40
	 mov edx,[Bmp_SizeX]
	 mov ecx,[Bmp_SizeY]
	 mov [edi+18],edx
	 mov [edi+22],ecx
	 mov [edi+26],word 1
	 mov [edi+28],word 8
	 mov [edi+30],dword 0
	 mov edx,[Bmp_bytes_per_string]
	 imul edx,ecx
	 mov [edi+34],edx
	 mov [edi+38],dword 0
	 mov [edi+42],dword 0
	 mov [edi+46],dword 0

	 ret
	 no_8_bits_per_pixel:

	 ;**********************************************
	 ;*******************24 bit BMP*****************
	 ;**********************************************

	 cmp eax,256
	 jle no_32_bits_per_pixel
	 ;copy picture
	 mov esi,[PointerToImage]
	 mov edx,[WhereCodingBMP]
	 mov ebx,[Bmp_SizeX]
	 shl ebx,3	       ;8*(picture_size_x)
	 lea ebx,[ebx+ebx*2]   ;3*8*(picture_size_x)
	 add ebx,31	       ;3*8*(picture_size_x)+31
	 shr ebx,5	       ;(3*8*(picture_size_x)+31)/32
	 mov [Bmp_doublewords],ebx ;double words in string
	 shl ebx,2
	 mov [Bmp_bytes_per_string],ebx ;bytes per string
	 mov ecx,[Bmp_SizeY]
	 dec ecx
	 imul ebx,ecx
	 add edx,54
	 add edx,ebx	     ;in edx pointer to start of copy bit map

	 mov ebp,[Bmp_bytes_per_string]
	 shl ebp,1

	 mov ebx,[Bmp_SizeY]

	 mov esi,[PointerToImage]
	 mov edi,edx

	 mov edx,[Bmp_SizeX]
	 lea edx,[edx+edx*2]

	 copy_lines_24:

	   push esi
	   mov ecx,[Bmp_doublewords]
	   rep_movsb_24:
	      mov eax,[esi]
	      mov [edi],eax
	   add esi,4
	   add edi,4
	   dec ecx
	   jnz rep_movsb_24
	   pop esi

	 add esi,edx
	 sub edi,ebp
	 dec ebx
	 jnz copy_lines_24

	 ;total size of bmp fille
	 mov edi,[WhereCodingBMP]
	 mov ebx,[Bmp_bytes_per_string]
	 imul ebx,[Bmp_SizeY]
	 add ebx,54

	 ;write info to structure of bmp file
	 mov [edi],word 'BM'
	 mov [edi+2],ebx
	 mov [edi+10],dword 54	  ;where begin bmp imige
	 mov [edi+14],dword 40	  ;total size of structure number two
	 mov edx,[Bmp_SizeX]
	 mov ecx,[Bmp_SizeY]
	 mov [edi+18],edx
	 mov [edi+22],ecx
	 mov [edi+26],word 1
	 mov [edi+28],word 24	  ;bytes per pixel
	 mov [edi+30],dword 0
	 mov edx,[Bmp_bytes_per_string]
	 imul edx,ecx
	 mov [edi+34],edx	  ;size of bmp image
	 mov [edi+38],dword 0
	 mov [edi+42],dword 0
	 mov [edi+46],dword 0
	 ret
	 no_32_bits_per_pixel:


	 ret

PointerToImage	     dd 0
WhereCodingBMP	     dd 0
BmpPalette	     dd 0
Bmp_SizeX	     dd 0
Bmp_SizeY	     dd 0
Bmp_Counter	     dd 0
Bmp_Counter2	     dd 0
Bmp_Counter3	     dd 0
Bmp_save1	     dd 0
Bmp_save2	     dd 0
Bmp_save3	     dd 0
Bmp_bytes_per_string dd 0
Bmp_doublewords      dd 0