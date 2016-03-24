;------------------------------------------
;-------------KEYS-------------------------
;------------------------------------------
align 4
keys:
	mcall SF_GET_KEY
	shr eax,8

	cmp al,176
	jne @f
	sub [PosX],20
	jmp change_screen
@@:
	cmp al,179
	jne @f
	add [PosX],20
	jmp change_screen
@@:
	cmp al,177
	jne @f
	add [PosY],20
	jmp change_screen
@@:
	cmp al,178
	jne @f
	sub [PosY],20
	jmp change_screen
@@:
	cmp al,49 ;1
	jne @f
	mov [k],1
	jmp zoom_ch
@@:
	cmp al,50 ;2
	jne @f
	mov [k],2
	jmp zoom_ch
@@:
	cmp al,51 ;3
	jne @f
	mov [k],3
	jmp zoom_ch
@@:
	cmp al,52 ;4
	jne @f
	mov [k],4
	jmp zoom_ch
@@:
	cmp al,56 ;8
	jne @f
	mov [k],8
	jmp zoom_ch
@@:
	cmp al,48 ;0
	jne @f
	mov [k],16
zoom_ch:
	call scrollbar_move_00
	call drawwin
	jmp change_screen
@@:
	cmp al,255
	jne @f
	call analizing_picture_to_palette
	call drawwin
	jmp change_screen
@@:
	jmp still

	;cmp al,27 ;Esc
	;jne still
	;mov eax,CursorsID
	;call delete_cursors
close:
	mcall SF_TERMINATE_PROCESS

change_screen:
	call CalculatePositionScreen
	call MovePictureToWorkScreen
	call draw_scrollers
	call PrintMousePos
	jmp still