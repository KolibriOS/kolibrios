;-----------------------------------------------------------
;SaveFonForSprite:
; PointerToPicture -> PointerToSpriteBufer
;
;PutFonForSprite:
; PointerToSpriteBufer -> PointerToPicture
;
;DrawSprite:
; PointerToEditBufer -> PointerToPicture
;-----------------------------------------------------------

align 4
SaveFonForSprite:
	mov ecx,[SpriteSizeX]
	or ecx,ecx
	jz .end_f
	mov edx,[Picture_SizeX]
	sub edx,ecx
	mov esi,[SpriteCoordinatY]
	imul esi,[Picture_SizeX]
	add esi,[SpriteCoordinatX]
	lea edx,[edx+edx*2]
	lea esi,[esi+esi*2]
	add esi,[PointerToPicture]
	mov edi,[PointerToSpriteBufer]
	mov ebx,[SpriteSizeY]
	mov [counter],ecx

	.next_line_sprite_save:
	mov ecx,[counter]
align 4
	@@:
	movsw
	movsb
	dec ecx
	jnz @b

	add esi,edx
	dec ebx
	jnz .next_line_sprite_save
.end_f:
	ret
;-----------------------------------------------------------

align 4
PutFonForSprite:
	mov ecx,[SpriteSizeX]
	or ecx,ecx
	jz .end_f
	mov edx,[Picture_SizeX]
	sub edx,ecx
	mov edi,[SpriteOldCoordinatY]
	imul edi,[Picture_SizeX]
	add edi,[SpriteOldCoordinatX]
	lea edx,[edx+edx*2]
	lea edi,[edi+edi*2]
	add edi,[PointerToPicture]
	mov esi,[PointerToSpriteBufer]
	mov ebx,[SpriteSizeY]
	mov [counter],ecx

	.next_line_sprite_put:
	mov ecx,[counter]
align 4
	@@:
	movsw
	movsb
	dec ecx
	jnz @b

	add edi,edx
	dec ebx
	jnz .next_line_sprite_put
.end_f:
	ret
;-----------------------------------------------------------

align 4
DrawSprite:
	mov ecx,[SpriteSizeX]
	or ecx,ecx
	jz .end_f
	mov edx,[Picture_SizeX]
	sub edx,ecx
	mov edi,[SpriteCoordinatY]
	imul edi,[Picture_SizeX]
	add edi,[SpriteCoordinatX]
	lea edx,[edx+edx*2]
	lea edi,[edi+edi*2]
	add edi,[PointerToPicture]
	mov esi,[PointerToEditBufer]
	mov ebx,[SpriteSizeY]
	mov [counter],ecx

	.next_line_sprite_draw:
	mov ecx,[counter]
align 4
	@@:
	movsw
	movsb
	dec ecx
	jnz @b

	add edi,edx
	dec ebx
	jnz .next_line_sprite_draw
.end_f:
	ret
