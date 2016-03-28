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
		;mov [crossing],2
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
	jae @f
		mov [Last_instrument],eax
	@@:

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
	jne @f
		call TakeButtonInstruments
		jmp still
	@@:

	cmp eax,17
	je still

	cmp eax,21
	je still

	cmp eax,20
	jne @f
		and [instrument_used],0
		jmp still
	@@:

	call TakeButtonInstruments
	jmp still