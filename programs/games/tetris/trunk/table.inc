;--------------------------------------------------------------
; DRAW_TABLE
;--------------------------------------------------------------
draw_table:	mov esi,table_tetris+LEN_X*BORDER_TOP+BORDER_LEFT

		mov ebx,X_LOCATION*65536+ADOBE_SIZE
		mov ecx,Y_LOCATION*65536+ADOBE_SIZE
		mov edi,LEN_Y-BORDER_TOP-BORDER_BOTTOM
y_draw: 	push edi

		mov edi,LEN_X-BORDER_LEFT-BORDER_RIGHT
x_draw: 	push edi
		mov ax,13
		movzx edx,byte [esi]
		mov edx,[color_table+edx*4]
		int 0x40
		call draw_frames
		inc esi
		add ebx,65536*ADOBE_SIZE
		pop edi
		dec edi
		jnz x_draw

		add esi,BORDER_LEFT+BORDER_RIGHT
		mov ebx,X_LOCATION*65536+ADOBE_SIZE
		add ecx,65536*ADOBE_SIZE
		pop edi
		dec edi
		jnz y_draw

		ret

;--------------------------------------------------------------

clear_table_tetris:
		cld
		mov al,1
		mov edi,table_tetris
		mov ecx,LEN_X*BORDER_TOP
		rep stosb

		mov edx,LEN_Y-BORDER_TOP-BORDER_BOTTOM
adr300: 	mov cl,BORDER_LEFT
		rep stosb
		dec ax	;AL=0
		mov cl,LEN_X-BORDER_LEFT-BORDER_RIGHT
		rep stosb
		inc ax	;AL=1
		mov cl,BORDER_RIGHT
		rep stosb
		dec dx
		jne adr300

		mov ecx,LEN_X*BORDER_BOTTOM
		rep stosb
		ret
