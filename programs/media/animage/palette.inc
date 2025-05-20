;-----------------------------------------------------------
;-------analizing picture to palette------------------------
;-----------------------------------------------------------
analizing_picture_to_palette:
	mov	eax,[Picture_SizeX]
	mov	ebx,[Picture_SizeY]
	imul	eax,ebx
	mov	ecx,eax
	mov	edx,[PointerToPicture]
	mov	esi,1 ;counter colors in palette

;put firs color of picture how one color in palette
	mov	ebp,[PointerToPalette]
	mov	eax,[edx]
	and	eax,0xffffff
	mov	[ebp],eax

analizing_to_palette:
	mov	eax,[edx]
	and	eax,0xffffff
	mov	edi,esi
	mov	ebp,[PointerToPalette]

next_color_in_palette:
	mov	ebx,[ebp]
	and	ebx,0xffffff
	cmp	ebx,eax	    ;this is color have in palette
	je	exit_loop_palette

	add	ebp,4
	dec	edi
	jnz	next_color_in_palette

	inc	esi
	cmp	esi,256 ;256 colors
	ja	more_than_256_colors

	mov	ebp,[PointerToPalette]
	mov	ebx,esi
	dec	ebx
	shl	ebx,2
	add	ebp,ebx
	mov	[ebp],ax
	shr	eax,16
	mov	[ebp+2],al

exit_loop_palette:
	add	edx,3
	dec	ecx
	jnz	analizing_to_palette

more_than_256_colors:
	mov	eax,esi
	ret