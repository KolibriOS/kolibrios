;---------------------------------------------
;-----------panel BUTTONS---------------------
;---------------------------------------------
buttons:

	cmp [Current_instrument],21
	jne no_finish_instrument_button

	cmp [instrument_used],1
	jne no_finish_instrument_button

	cmp [Activate_instrument],0
	jne no_finish_instrument_button

	cmp [crossing],1
	jne no_finish_instrument_button

	mov [finishing_crossing],1
	call TakeButtonInstruments

	no_finish_instrument_button:

	mov eax,17
	mcall

	shr eax,8

	cmp eax,1
	jne no_exit

	mov eax,CursorsID
	call delete_cursors

	or eax,-1
	mcall

	no_exit:


	mov [Current_instrument],eax

	; set cursor for current instrument
	pushad

	mov eax,CursorsID
	mov ebx,[Current_instrument]

	call set_cursor
	popad



	cmp eax,10
	jl still

	;save instrumnets 10-40
	cmp eax,40
	jae no_save_last_instrument

	 mov [Last_instrument],eax

	no_save_last_instrument:

	cmp eax,10
	je still

	cmp eax,14
	je still

	cmp eax,11
	je still

	cmp eax,13
	je still

	cmp eax,12
	je still

	cmp eax,51
	jne no_palette_

	call TakeButtonInstruments
	jmp still

	no_palette_:

	cmp eax,17
	je still

	cmp eax,21
	jne no_allocation__

	and [Activate_instrument],0
	jmp still

	no_allocation__:

	cmp eax,20
	jne no_kontur__

	and [instrument_used],0
	jmp still

	no_kontur__:

	call TakeButtonInstruments

	jmp still