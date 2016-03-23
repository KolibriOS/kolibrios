;---------------------------------------------
;-----------panel BUTTONS---------------------
;---------------------------------------------
buttons:

	cmp [Current_instrument],21
	jne @f
	cmp [instrument_used],1
	jne @f
	cmp [Activate_instrument],0
	jne @f
	cmp [crossing],1
	jne @f
		mov [finishing_crossing],1
		call TakeButtonInstruments
		jmp still
	@@:

	mcall SF_GET_BUTTON

	shr eax,8

	cmp eax,1
	jne no_exit
		mov eax,CursorsID
		call delete_cursors
		mcall SF_TERMINATE_PROCESS
	no_exit:

	mov [Current_instrument],eax
	call button_handler_main_menu

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