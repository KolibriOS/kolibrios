;-----------------------------------------------------------
;------------------SaveFonForSprite-------------------------
;-----------------------------------------------------------
SaveFonForSprite:

	mov edi,[PointerToSpriteBufer]
	mov ecx,[SpriteSizeX]
	mov edx,[Picture_SizeX]
	sub edx,[SpriteSizeX]
	mov esi,[SpriteCoordinatY]
	imul esi,[Picture_SizeX]
	add esi,[SpriteCoordinatX]
	lea edx,[edx+edx*2]
	lea esi,[esi+esi*2]
	add esi,[PointerToPicture]
	mov ebx,[SpriteSizeY]
	mov [counter],ecx

	next_line_sprite_save:
	mov ecx,[counter]

	next_pixel_sprite_save:
	mov eax,[esi]
	and eax,0xffffff
	mov [edi],ax
	shr eax,16
	mov [edi+2],al
	add esi,3
	add edi,3
	dec ecx
	jnz next_pixel_sprite_save

	add esi,edx
	dec ebx
	jnz next_line_sprite_save

	ret
;-----------------------------------------------------------
;-------------------PutFonForSprite-------------------------
;-----------------------------------------------------------
PutFonForSprite:

	mov esi,[PointerToSpriteBufer]
	mov ecx,[SpriteSizeX]
	mov edx,[Picture_SizeX]
	sub edx,[SpriteSizeX]
	mov edi,[SpriteOldCoordinatY]
	imul edi,[Picture_SizeX]
	add edi,[SpriteOldCoordinatX]
	lea edx,[edx+edx*2]
	lea edi,[edi+edi*2]
	add edi,[PointerToPicture]
	mov ebx,[SpriteSizeY]
	mov [counter],ecx

	next_line_sprite_put:
	mov ecx,[counter]

	next_pixel_sprite_put:
	mov eax,[esi]
	and eax,0xffffff
	mov [edi],ax
	shr eax,16
	mov [edi+2],al
	add esi,3
	add edi,3
	dec ecx
	jnz next_pixel_sprite_put

	add edi,edx
	dec ebx
	jnz next_line_sprite_put

	ret
;-----------------------------------------------------------
;------------------DrawFonForSprite-------------------------
;-----------------------------------------------------------
DrawSprite:

	mov esi,[PointerToEditBufer]
	mov ecx,[SpriteSizeX]
	mov edx,[Picture_SizeX]
	sub edx,[SpriteSizeX]
	mov edi,[SpriteCoordinatY]
	imul edi,[Picture_SizeX]
	add edi,[SpriteCoordinatX]
	lea edx,[edx+edx*2]
	lea edi,[edi+edi*2]
	add edi,[PointerToPicture]
	mov ebx,[SpriteSizeY]
	mov [counter],ecx

	next_line_sprite_draw:
	mov ecx,[counter]

	next_pixel_sprite_draw:
	mov eax,[esi]
	and eax,0xffffff
	mov [edi],ax
	shr eax,16
	mov [edi+2],al
	add esi,3
	add edi,3
	dec ecx
	jnz next_pixel_sprite_draw

	add edi,edx
	dec ebx
	jnz next_line_sprite_draw

	ret
