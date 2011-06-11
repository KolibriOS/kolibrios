select_menu:
	call	check_alt
.1:
	xor	eax,eax
	mov	[menu_data_1.ret_key],eax

	push	dword menu_data_1
	call	[menu_bar_activate]

	call	clear_control_key_flag

	mov	eax,[menu_data_1.ret_key]
	mov	[menu_data_1.ret_key],dword 0
	cmp	eax,1
	je	.6

	cmp	eax,2
	je	.2

	cmp	[menu_data_1.click],dword 1
	jne	still

	cmp	[menu_data_1.cursor_out],dword 0
	jne	analyse_out_menu_1
	jmp	still
;---------------------------------------------------------------------
.2:
	xor	eax,eax
	mov	[menu_data_2.ret_key],eax

	push	dword menu_data_2
	call	[menu_bar_activate]

	call	clear_control_key_flag

	mov	eax,[menu_data_2.ret_key]
	mov	[menu_data_2.ret_key],dword 0
	cmp	eax,1
	je	.1

	cmp	eax,2
	je	.3


	cmp	[menu_data_2.click],dword 1
	jne	still

	cmp	[menu_data_2.cursor_out],dword 0
	jne	analyse_out_menu_2
	jmp	still
;---------------------------------------------------------------------
.3:
	xor	eax,eax
	mov	[menu_data_3.ret_key],eax

	push	dword menu_data_3
	call	[menu_bar_activate]

	call	clear_control_key_flag

	mov	eax,[menu_data_3.ret_key]
	mov	[menu_data_3.ret_key],dword 0
	cmp	eax,1
	je	.2

	cmp	eax,2
	je	.4


	cmp	[menu_data_3.click],dword 1
	jne	still

	cmp	[menu_data_3.cursor_out],dword 0
	jne	analyse_out_menu_3
	jmp	still
;---------------------------------------------------------------------
.4:
	xor	eax,eax
	mov	[menu_data_4.ret_key],eax

	push	dword menu_data_4
	call	[menu_bar_activate]

	call	clear_control_key_flag

	mov	eax,[menu_data_4.ret_key]
	mov	[menu_data_4.ret_key],dword 0
	cmp	eax,1
	je	.3

	cmp	eax,2
	je	.5


	cmp	[menu_data_4.click],dword 1
	jne	still

	cmp	[menu_data_4.cursor_out],dword 0
	jne	analyse_out_menu_4
	jmp	still
;---------------------------------------------------------------------
.5:
	xor	eax,eax
	mov	[menu_data_5.ret_key],eax

	push	dword menu_data_5
	call	[menu_bar_activate]

	call	clear_control_key_flag

	mov	eax,[menu_data_5.ret_key]
	mov	[menu_data_5.ret_key],dword 0
	cmp	eax,1
	je	.4

	cmp	eax,2
	je	.6


	cmp	[menu_data_5.click],dword 1
	jne	still

	cmp	[menu_data_5.cursor_out],dword 0
	jne	analyse_out_menu_5
	jmp	still
;---------------------------------------------------------------------
.6:
	xor	eax,eax
	mov	[menu_data_6.ret_key],eax

	push	dword menu_data_6
	call	[menu_bar_activate]

	call	clear_control_key_flag

	mov	eax,[menu_data_6.ret_key]
	mov	[menu_data_6.ret_key],dword 0
	cmp	eax,1
	je	.5

	cmp	eax,2
	je	.1


	cmp	[menu_data_6.click],dword 1
	jne	still

	cmp	[menu_data_6.cursor_out],dword 0
	jne	analyse_out_menu_6
	jmp	still
;---------------------------------------------------------------------
clear_control_key_flag:
	push	eax
	xor	eax,eax
	mov	[shift_flag],al
	mov	[ctrl_flag],al
	mov	[alt_flag],al
	pop	eax
	ret
;---------------------------------------------------------------------
check_alt:
	xor	eax,eax
	mov	al,[alt_flag]
	test	eax,eax
	jz	@f
	xor	ebx,ebx
	ret
@@:
	add	esp,4
	jmp	still
;---------------------------------------------------------------------