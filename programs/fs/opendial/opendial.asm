;*****************************************************************************
; Open Dialog - for Kolibri OS
; Copyright (c) 2009, 2010, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;        * Redistributions of source code must retain the above copyright
;          notice, this list of conditions and the following disclaimer.
;        * Redistributions in binary form must reproduce the above copyright
;          notice, this list of conditions and the following disclaimer in the
;          documentation and/or other materials provided with the distribution.
;        * Neither the name of the <organization> nor the
;          names of its contributors may be used to endorse or promote products
;          derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Marat Zakiyanov ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************

	use32
	org	0x0

	db 'MENUET01'
	dd 0x01
	dd START
	dd IM_END
	dd I_END
	dd stacktop
	dd param
	dd path

include '../../macros.inc'
include '../../develop/libraries/box_lib/load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
;include 'macros.inc'
;include 'load_lib.mac'
;include 'box_lib.mac'
@use_library

x_minimal_size equ 350
y_minimal_size equ 250
;---------------------------------------------------------------------
;---------------------------------------------------------------------
START:
	mcall	68,11
	mcall	66,1,1
	mcall	40,0x27
	call	get_communication_area
	
	call	get_active_pocess

load_libraries	l_libs_start,end_l_libs
	test	eax,eax
	jnz	button.exit
; initialize sort
	push	dword 1
	call	dword [sort_init]
; unpack deflate
	mov	eax,[unpack_DeflateUnpack2]
	mov	[deflate_unpack],eax

	mov	esi,start_pach
	mov	edi,previous_dir_path
	call	copy_dir_name.1
	
	call	load_root_directory
	call	load_start_directory
	call	sort_directory
	call	load_icons
	call	convert_icons
	call	load_ini
	call	calc_ini
	jmp	red_1
;---------------------------------------------------------------------
red:
	call	control_minimal_window_size
red_1:
	call	draw_window
;---------------------------------------------------------------------
still:
	mcall	10
	cmp	eax,1
	je	red
	cmp	eax,2
	je	key
	cmp	eax,3
	je	button
	cmp	eax,6
	je	mouse
	jmp	still
;---------------------------------------------------------------------
control_minimal_window_size:
	pusha
	call	get_window_param
	test	[window_status],10b
	jnz	.end	;red_1
	test	[window_status],100b
	jnz	.end	;red_1
	test	[window_status],1b
	jnz	.end	;red_1
	mov	esi,-1
	mov	eax,procinfo
	mov	eax,[eax+46]
	cmp	eax,dword y_minimal_size ;200
	jae	@f
	mov	esi,dword y_minimal_size ;200
	mcall	67,-1,ebx,ebx
@@:
	mov	edx,-1
	mov	eax,procinfo
	mov	eax,[eax+42]
	cmp	eax,dword x_minimal_size ;300
	jae	@f
	mov	edx,dword x_minimal_size ;300
	mcall	67,-1,ebx,,ebx
@@:
.end:
	popa
	ret
;---------------------------------------------------------------------
key:
	mov	al,[focus_pointer]
	test	al,al
	jne	key_ASCII
	mcall	2
	xor	ebx,ebx
	cmp	[extended_key],1
	je	.extended_key
	test	al,al
	jnz	still
	cmp	ah,0xE0
	jne	@f
	mov	[extended_key],1
	jmp	still
@@:
	cmp	ah,72	; arrow	up
	je	.2
	cmp	ah,80	; arrow	down
	je	.1
	cmp	ah,28	; Enter
	je	.7
	cmp	ah,1	; Esc
	je	button.exit
	cmp	ah,14	; Backspace
	je	button.exit_dir
	cmp	ah,187	; F1
	je	select_disk
	cmp	ah,188	; F2
	je	select_sort
	cmp	ah,189	; F3
	je	select_filter
	cmp	ah,19	; R
	je	button.reload_dir
	cmp	ah,42
	je	key_shift_up
	cmp	ah,54
	je	key_shift_up
	cmp	ah,170
	je	key_shift_down
	cmp	ah,182
	je	key_shift_down
	cmp	ah,29
	je	key_ctrl_up
	cmp	ah,157
	je	key_ctrl_down
	cmp	ah,56
	je	key_alt_up
	cmp	ah,184
	je	key_alt_down
	cmp	ah,206	; NumPad+ Up
	je	NumPad_plus_Up
	cmp	ah,202	; NumPad- Up
	je	NumPad_minus_Up
	cmp	ah,183	; NumPad* Up
	je	NumPad_invert_Up
	cmp	ah,158
	je	symbol_a_up
	cmp	ah,15	; Tab down
	je	change_focus_area_press_Tab_key
	cmp	ah,143	; Tab up
	je	change_focus_area_check_Tab_key
	jmp	still
.extended_key:
	mov	[extended_key],0
	cmp	ah,80	; arrow down
	je	.1
	cmp	ah,72	; arrow up
	je	.2
	cmp	ah,81	; PageDown
	je	.3
	cmp	ah,73	; PageUp
	je	.4
	cmp	ah,71	; Home
	je	.5
	cmp	ah,79	; End
	je	.6
	cmp	ah,28	; Enter
	je	.7
	cmp	ah,82	; Insert
	je	.8
	
	cmp	ah,29
	je	key_ctrl_up
	cmp	ah,157
	je	key_ctrl_down
	cmp	ah,56
	je	key_alt_up
	cmp	ah,184
	je	key_alt_down
	jmp	still
;---------------------------------
.11:
	inc	ebx	; 11
;---------------------------------
.10:
	inc	ebx	; 10
;---------------------------------
.9:
	inc	ebx	; 9
;---------------------------------
.8:
	inc	ebx	; 8
;---------------------------------
.7:
	inc	ebx	; 7
;---------------------------------
.6:
	inc	ebx	; 6
;---------------------------------
.5:
	inc	ebx	; 5
;---------------------------------
.4:
	inc	ebx	; 4
;---------------------------------
.3:
	inc	ebx	; 3
;---------------------------------
.2:
	inc	ebx	; 2
;---------------------------------
.1:
	inc	ebx	; 1
;---------------------------------
	call	.key_action
	
;	movzx	ecx,word [file_browser_data_1.start_draw_cursor_line]
;	mcall	47,0x80000,,<50,0>,0x40000000,0xffffff
;	movzx	ecx,word [file_browser_data_1.size_y]
;	mcall	47,0x80000,,<150,0>,0x40000000,0xffffff

	mov	eax,file_browser_data_1.mouse_keys_delta
	cmp	[eax],dword 3
	jne	still
	xor	ebx,ebx
	mov	[eax],ebx
	call	load_next_dir
	jmp	still
;-------------------------------------------------------
.key_action:
	mov	[file_browser_data_1.key_action],ebx

	push	dword file_browser_data_1
	call	[FileBrowser_key]

	cmp	[file_browser_data_1.draw_scroll_bar],0
	je	@f
	call	draw_scrollbar1
	mov	[file_browser_data_1.draw_scroll_bar],0
@@:
	ret
;---------------------------------------------------------------------
change_focus_area_Tab_key_ASCII:
	xor	eax,eax
	inc	eax
	mov	[Tab_key_block],al
	jmp	change_focus_area
;---------------------------------------------------------------------
change_focus_area_press_Tab_key:
	cmp	[open_dialog_type],1
	jne	still
	mov	al,[Tab_key_block]
	test	al,al
	jnz	still
	xor	eax,eax
	inc	eax
	mov	[Tab_key],al
	jmp	still
;---------------------------------------------------------------------
change_focus_area_check_Tab_key:
	cmp	[open_dialog_type],1
	jne	still
	xor	eax,eax
	mov	[Tab_key_block],al
	mov	al,[Tab_key]
	test	al,al
	jz	still
	xor	eax,eax
	mov	[Tab_key],al
;---------------------------------------------------------------------
change_focus_area:
	mov	al,[focus_pointer]
	inc	al
	and	al,1
	mov	[focus_pointer],al
.1:
	mov	edi,edit1
	test	al,al
	jne	@f
	mov	[file_browser_data_1.select_panel_counter],1
	and	[edi+44],dword 0xFFFFFFFD	; ed_focus
	mov	[edi+12],dword 0xffffff	; color white
	call	draw_draw_file_browser1
	mcall	66,1,1
	jmp	still
@@:
	mov	[file_browser_data_1.select_panel_counter],0
	or	[edi+44],dword ed_focus
	mov	[edi+12],dword 0xffffb0	; color yellow
	call	draw_draw_file_browser1
	mcall	66,1,0
	jmp	still
;---------------------------------------------------------------------
key_ASCII:
	mcall	2
	cmp	ah,9
	je	change_focus_area_Tab_key_ASCII
	cmp	ah,13
	je	.13
	cmp	ah,27
	je	button.exit
	push	dword name_editboxes
	call	[edit_box_key]
	jmp	still
.13:
;	cmp [open_dialog_type],2	; Select dir
;	je	file_no_folder
;	cmp	[open_dialog_type],1	; Save file
;	jne	user_selected_name_action	; load_dir
;	inc	[open_dialog_type]
	jmp	file_no_folder
;.load_dir:
;	mov	[file_browser_data_1.select_panel_counter],1
;	xor	eax,eax
;	mov	[focus_pointer],al
;	mcall	66,1,1

;	xor	eax,eax
;	mov	esi,dir_path
;	cld
;@@:
;	lodsb
;	test	al,al
;	jne	@r
;	sub	esi,2
;	cmp	[esi],byte '/'
;	jne	@f
;	xor	eax,eax
;	mov	[esi],al
;@@:
;	call	load_next_dir.1
;	jmp	still
;---------------------------------------------------------------------
user_selected_name_action:
	mov	eax,[communication_area]
	test	eax,eax
	jnz	@f
	call	control_minimal_window_size
	call	draw_window
	ret
@@:
	add	eax,16	;12
;copy_path	user_selected_name,dir_path,eax,0
	mov	esi,dir_path
	mov	edi,eax
	call	copy_dir_name
	mov	[edi-1],byte '/'
	mov	esi,user_selected_name
	call	copy_dir_name
	
	mov	eax,[communication_area]
	mov	[eax],word 1
	jmp	button.exit
;---------------------------------------------------------------------	
select_disk:
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
	je	select_filter.1

	cmp	eax,2
	je	select_sort.1

	cmp	[menu_data_1.click],dword 1
	jne	still

	cmp	[menu_data_1.cursor_out],dword 0
	jne	analyse_out_menu_1
	jmp	still
;---------------------------------------------------------------------
select_sort:
	call	check_alt
.1:
	xor	eax,eax
	mov	[menu_data_2.ret_key],eax

	push	dword menu_data_2
	call	[menu_bar_activate]

	call	clear_control_key_flag

	mov	eax,[menu_data_2.ret_key]
	mov	[menu_data_2.ret_key],dword 0
	cmp	eax,1
	je	select_disk.1

	cmp	eax,2
	je	select_filter.1


	cmp	[menu_data_2.click],dword 1
	jne	still

	cmp	[menu_data_2.cursor_out],dword 0
	jne	analyse_out_menu_2
	jmp	still
;---------------------------------------------------------------------
select_filter:
	call	check_alt
.1:
	xor	eax,eax
	mov	[menu_data_3.ret_key],eax

	push	dword menu_data_3
	call	[menu_bar_activate]

	call	clear_control_key_flag

	mov	eax,[menu_data_3.ret_key]
	mov	[menu_data_3.ret_key],dword 0
	cmp	eax,1
	je	select_sort.1

	cmp	eax,2
	je	select_disk.1


	cmp	[menu_data_3.click],dword 1
	jne	still

	cmp	[menu_data_3.cursor_out],dword 0
	jne	analyse_out_menu_3
	jmp	still
;---------------------------------------------------------------------
symbol_a_up:
NumPad_plus_Up:
	call	check_ctrl
	jmp	key.9
;---------------------------------------
NumPad_minus_Up:
	call	check_ctrl
	jmp	key.10
;---------------------------------------
NumPad_invert_Up:
	call	check_ctrl
	jmp	key.11
;---------------------------------------	
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
;---------------------------------------	
check_ctrl:
	xor	eax,eax
	mov	al,[ctrl_flag]
	test	eax,eax
	jz	@f
	xor	ebx,ebx
	ret
@@:
	add	esp,4
	jmp	still
;---------------------------------------------------------------------
clear_control_key_flag:
	xor	eax,eax
	mov	[shift_flag],al
	mov	[ctrl_flag],al
	mov	[alt_flag],al
	ret
;---------------------------------------------------------------------
key_shift_up:
	mov	[shift_flag],1
	jmp	still
;---------------------------------------------------------------------
key_shift_down:
	mov	[shift_flag],0
	jmp	still
;---------------------------------------------------------------------
key_ctrl_up:
	mov	[ctrl_flag],1
	jmp	still
;---------------------------------------------------------------------
key_ctrl_down:
	mov	[ctrl_flag],0
	jmp	still
;---------------------------------------------------------------------
key_alt_up:
	mov	[alt_flag],1
	jmp	still
;---------------------------------------------------------------------
key_alt_down:
	mov	[alt_flag],0
	jmp	still
;---------------------------------------------------------------------
button:
	mcall	17
	cmp	ah,6
	je	.reload_dir_1
	cmp	ah,4
	je	.open_dir_or_file
	cmp	ah,3
	je	.exit
	cmp	ah,2
	je	.exit_dir
	cmp	ah,1
	jne	still
.exit:
	mov	eax,[communication_area]
	test	eax,eax
	jz	@f
	cmp	[eax],word 1
	je	@f
	mov	[eax],word 3
@@:
	mov	eax,[N_error]
	test	eax,eax
	jz	@f
	call	start_error_window_thread
@@:
	call	get_window_param
	mov	ebx,[communication_area]
	mov	ecx,procinfo
;	mov	eax,[window_x]
	mov	eax,[ecx+34]
	shl	eax,16
	add	eax,[ecx+42]
	mov	[ebx+4],eax
;	mov	eax,[window_y]
	mov	eax,[ecx+38]
	shl	eax,16
	add	eax,[ecx+46]
	mov	[ebx+8],eax

	mcall	-1
;---------------------------------------------------------------------
.reload_dir:
	call	check_ctrl
.reload_dir_1:
	call	load_next_dir.1
	jmp	still
;---------------------------------------------------------------------
.exit_dir:
	call	load_next_dir.exit_dir
	jmp	still
;---------------------------------------------------------------------
.open_dir_or_file:
	cmp	[open_dialog_type],2	;Select	dir
	je	file_no_folder
	
	cmp	[open_dialog_type],1	;Save file
	jne	@f
	mov	al,[focus_pointer]
	test	al,al
	jne	file_no_folder	
@@:
	xor	ebx,ebx
	jmp	key.7
;---------------------------------------------------------------------
thread_start:
	mov	eax,[N_error]
	cmp	al,1
	jne	@f
	mov	[N_error],load_ini_error_type
	mov	[error_path],file_name
	jmp	.error_type
@@:
	cmp	al,2
	jne	@f
	mov	[N_error],load_icons_error_type
	mov	[error_path],file_name
	jmp	.error_type
@@:
	cmp	al,3
	jne	@f
	mov	[N_error],memory_free_error_type
	xor	eax,eax
	mov	[error_path],eax
	mov	[error_type],eax
	jmp	.red
@@:
	cmp	al,4
	jne	@f
	mov	[N_error],memory_get_error_type
	xor	eax,eax
	mov	[error_path],eax
	mov	[error_type],eax
	jmp	.red
@@:
	cmp	al,5
	jne	@f
	mov	[N_error],load_directory_error_type
	mov	[error_path],dir_path
	jmp	.error_type
@@:
	cmp	al,6
	jne	.button
	mov	[N_error],convert_icons_error_type
	mov	[error_path],file_name
	xor	eax,eax
	mov	[error_type],eax
	jmp	.red
.error_type:
	mov	eax,[error_type]
	shl	eax,2
	add	eax,error_fs_text_pointers
	mov	eax,[eax]
	mov	[error_type],eax
.red:
	call	draw_error_window
.still:
	mcall	10
	cmp	eax,1
	je	.red
	cmp	eax,2
	je	.key
	cmp	eax,3
	je	.button
	jmp	.still
.key:
	mcall	2
	jmp	.still
.button:
	mcall	-1
	jmp	.still
;---------------------------------------------------------------------
draw_error_window:
	mcall	12,1
	mcall	0,[error_window_x],[error_window_y],0x03ff0000
	call	type_title
	mcall	4,<10,30>,0x90ffffff,[N_error]
	mov	eax,[error_path]
	test	eax,eax
	jz	@f
	mcall	4,<10,50>,,[error_path]
@@:
	mov	eax,[error_type]
	test	eax,eax
	jz	@f
	mcall	4,<10,70>,,[error_type]
@@:
	mcall	12,2
	ret
;---------------------------------------------------------------------
start_error_window_thread:
	mcall	9,procinfo,-1
	mov	eax,[ebx+46]
	shr	eax,1
	add	eax,[ebx+38]
	sub	eax,40
	mov	[error_window_y+2],ax
	mov	eax,[ebx+42]
	shr	eax,1
	add	eax,[ebx+34]
	sub	eax,125
	mov	[error_window_x+2],ax
	mcall	51,1,thread_start,thread_stack
	ret
;---------------------------------------------------------------------
mouse:
	mcall	18,7
	cmp	[active_process],eax
	jne	still

	mcall	37,7
	mov	[mouse_scroll_data],eax

	mcall	37,1
	mov	[mouse_position],eax

	cmp	[scroll_bar_data_vertical.delta2],0
	jne	.scrollbar

	mov	[file_browser_data_1.select_flag],0

	push	dword file_browser_data_1
	call	[FileBrowser_mouse]

	mov	eax,file_browser_data_1.mouse_keys_delta
	cmp	[eax],dword 3
	jne	.check_focus	; scrollbar
	mov	[eax],dword 0
	call	load_next_dir
	jmp	still
;---------------------------------------------------
.check_focus:
	mov	ebx,[file_browser_data_1.select_flag]
	test	ebx,ebx
	jz	.scrollbar	;@f
	mov	al,[focus_pointer]
	test	al,al
	jz	.scrollbar
	xor	eax,eax
	mov	[focus_pointer],al
	jmp	change_focus_area.1
;---------------------------------------------------
.scrollbar:
	mov	eax,[scroll_bar_data_vertical.max_area]
	cmp	eax,[scroll_bar_data_vertical.cur_area]
	jbe	.menu_bar	;still
	
	push	dword scroll_bar_data_vertical
	call	[scrollbar_ver_mouse]
	
	cmp	[scroll_bar_data_vertical.redraw],0
	je	.menu_bar	;still
	mov	[scroll_bar_data_vertical.redraw],0
.draw:
	call	draw_draw_file_browser2
	jmp	still

;---------------------------------------------------
.menu_bar:
	cmp	[scroll_bar_data_vertical.delta2],0
	jne	still

.menu_bar_1:
	call	.set_mouse_flag
@@:
	push	dword menu_data_1
	call	[menu_bar_mouse]

	cmp	[menu_data_1.click],dword 1
	jne	.menu_bar_2

	cmp	[menu_data_1.cursor_out],dword 0
	jne	analyse_out_menu_1
	jmp	.menu_bar_1
;--------------------------------------------
.menu_bar_2:
	push	dword menu_data_2
	call	[menu_bar_mouse]

	cmp	[menu_data_2.click],dword 1
	jne	.menu_bar_3

	cmp	[menu_data_2.cursor_out],dword 0
	jne	analyse_out_menu_2
	jmp	.menu_bar_1
;---------------------------------------------------
.menu_bar_3:
	push	dword menu_data_3
	call	[menu_bar_mouse]

	cmp	[menu_data_3.click],dword 1
	jne	.check_editboxes

	cmp	[menu_data_3.cursor_out],dword 0
	jne	analyse_out_menu_3
	jmp	.menu_bar_1
;---------------------------------------------------
.check_editboxes:
	cmp	[open_dialog_type],1
	jne	.check_scroll_event
	mov	eax,[edit1+44]
	and	eax,10b
	push	dword name_editboxes
	call	[edit_box_mouse]
	mov	ebx,[edit1+44]
	and	ebx,10b
	cmp	eax,ebx
	je	.check_scroll_event
	mov	al,[focus_pointer]
	test	al,al
	jnz	.check_scroll_event
	xor	eax,eax
	test	ebx,10b
	jz	@f
	inc	eax
@@:
	mov	[focus_pointer],al
	jmp	change_focus_area.1
;---------------------------------------------------
.check_scroll_event:
	mov	eax,[mouse_position]
	xor	ebx,ebx
	mov	bx,ax	; EBX mouse y
	shr	eax,16	; EAX mouse x
	
	mov	cx,[file_browser_data_1.start_x]
	mov	dx,[file_browser_data_1.start_y]

	cmp	ax,cx
	jb	.mouse_next	; min x

	cmp	bx,dx
	jb	.mouse_next	; min y

	add	cx,[file_browser_data_1.size_x]
	cmp	ax,cx	
	ja	.mouse_next	; max x

	add	dx,[file_browser_data_1.size_y]
	cmp	bx,dx
	ja	.mouse_next	; max y

	xor	ecx,ecx
	xor	ebx,ebx
	mov	eax,[mouse_scroll_data]
	test	eax,eax
	jz	.mouse_next
	test	ax,0x8000
	jnz	.decr
	shr	eax,16
	test	ax,0x8000
	jnz	.decr_1

	mov	cx,[mouse_scroll_data.vertical]
	test	ecx,ecx
	jnz	@f
	mov	cx,[mouse_scroll_data.horizontal]
	test	ecx,ecx
	jz	.mouse_next
@@:
	mov	ebx,1
@@:
	push	ebx ecx
	call	key.key_action
	pop	ecx ebx
	dec	ecx
	jnz	@r
	jmp	still
;----------------------------------------
.decr:
	mov	bx,[mouse_scroll_data.vertical]
	jmp	@f
.decr_1:
	mov	bx,[mouse_scroll_data.horizontal]
@@:
	mov	ecx,0xffff
	sub	ecx,ebx
	inc	ecx
	mov	ebx,2
@@:
	push	ebx ecx
	call	key.key_action
	pop	ecx ebx
	dec	ecx
	jnz	@r
	jmp	still
;---------------------------------------------------
.mouse_next:
	jmp	still
;---------------------------------------------------------------------
.set_mouse_flag:
	xor	eax,eax
	inc	eax
	mov	[menu_data_1.get_mouse_flag],eax
	mov	[menu_data_2.get_mouse_flag],eax
	ret
;---------------------------------------------------------------------
analyse_out_menu_1:
; Available disks
	mov	eax,[menu_data_1.cursor_out]
	dec	eax
	imul	esi,eax,10
	add	esi,retrieved_devices_table
	mov	edi,dir_path
	call	copy_dir_name
	call	load_next_dir.1
	jmp	still
;---------------------------------------------------------------------
analyse_out_menu_2:
; Sort
	mov	eax,[menu_data_2.cursor_out]
	xor	ebx,ebx
	cmp	eax,dword 1
	je	.1
	cmp	eax,dword 2
	je	.2
	cmp	eax,dword 3
	je	.3
	cmp	eax,dword 4
	je	.4
	jmp	still
.4:
	add	ebx,2
.3:
	add	ebx,2
.2:
	add	ebx,2
.1:
	mov	[sort_type],ebx
	call	sort_directory
	call	draw_draw_file_browser1
	jmp	still
;---------------------------------------------------------------------
analyse_out_menu_3:
; Filter
	mov	eax,[menu_data_3.cursor_out]
	cmp	eax,dword 1
	jne	@f
	mov	[filter_flag],0
	call	load_next_dir.1
	jmp	still
@@:
	cmp	eax,dword 2
	jne	still
	mov	[filter_flag],1
	call	load_next_dir.1
	jmp	still	
;---------------------------------------------------------------------
get_communication_area:
	xor	eax,eax
	mov	al,[param]
	test	eax,eax
	jz	@f
	mcall	68,22,param,,0x01
	mov	[communication_area],eax
	movzx	ebx,word [eax+2]
	mov	[open_dialog_type],ebx
	mov	ebx,[eax+4]
	cmp	bx,word x_minimal_size ;300
	jb	@f
	mov	[window_x],ebx
	mov	ebx,[eax+8]
	cmp	bx,word y_minimal_size ;200
	jb	@f
	mov	[window_y],ebx
@@:
	ret
;---------------------------------------------------------------------
load_start_directory:
	mov	eax,[communication_area]	
	test	eax,eax
	jz	.1
	movzx	ebx,word [eax]
	test	eax,eax
	jz	.1
	add	eax,16	;12 ;4
	mov	esi,eax
	push	esi
	mov	esi,[communication_area]
	add	esi,3840 ;4096-256
	mov	eax,[esi]
	test	eax,eax
	jnz	@f
	mov	esi,example_name_temp
@@:
	mov	edi,user_selected_name
	call	copy_dir_name
	pop	esi
	jmp	.2
.1:
	mov	esi,start_pach
.2:
	mov	edi,dir_path
	call	copy_dir_name
	
;	call	load_directory
;	mov	eax,[N_error]
;	test	eax,eax
;	jnz	button.exit
.3:
	call	load_directory
	mov	eax,[N_error]
	test	eax,eax
	jz	@f
	call	error_handler
	jmp	.3
@@:
	ret
;---------------------------------------------------------------------
load_next_dir:
	mov	ebx,[file_browser_data_1.selected_BDVK_adress]
	add	ebx,40
	test	[ebx-40],byte 0x10
	jz	file_no_folder
	cmp	[ebx],word '..'
	jne	@f
	cmp	[ebx+2],byte 0
	je	.exit_dir
@@:
	mov	esi,dir_path
	call	copy_dir_path

@@:
.1:
	call	load_directory
	mov	eax,[N_error]
	test	eax,eax
	jz	@f
	call	error_handler
	jmp	.1
@@:
	call	sort_directory

	mov	ebx,[scroll_bar_data_vertical.x]
	inc	ebx
	mov	ecx,[scroll_bar_data_vertical.y]
	inc	ecx
	mcall	13,,,0xcccccc
	mov	edi,edit1
	xor	eax,eax
	mov	[edi+44],eax
	mov	[edi+12],dword 0xffffff	; color white
	call	draw_draw_file_browser1
	ret
.exit_dir:
	mov	esi,dir_path
	call	copy_exit_dir
	jmp	.1
;---------------------------------------------------------------------
error_handler:
.red:
	call	.draw_window
;------------------------------------
.still:
	mcall	10
	cmp	eax,1
	je	.red
	cmp	eax,2
	je	.key
	cmp	eax,3
	je	.button
	jmp	.still
;------------------------------------
.draw_window:
	xor	eax,eax
	inc	eax
	mov	[error_window],al
	call	control_minimal_window_size
	call	draw_window
	xor	eax,eax
	mov	[error_window],al
	ret
;------------------------------------
.key:
	mcall	2
	xor	ebx,ebx
	cmp	[extended_key],1
	je	.extended_key
	test	al,al
	jnz	.still
	cmp	ah,0xE0
	jne	@f
	mov	[extended_key],1
	jmp	.still
@@:
	cmp	ah,129	; Esc
	je	.exit
	jmp	.still
.extended_key:
	jmp	.still
;------------------------------------
.button:
	mcall	17
	cmp	ah,5
	je	.exit
	cmp	ah,1
	jne	.still
	xor	eax,eax
	mov	[N_error],eax
	jmp	button.exit
;------------------------------------
.exit:
	mov	esi,previous_dir_path
	mov	edi,dir_path
	call	copy_dir_name.1
	mov	esi,start_pach
	mov	edi,previous_dir_path
	call	copy_dir_name.1
	ret
;---------------------------------------------------------------------
file_no_folder:
	mov	esi,dir_path
	mov	edi,file_name
	call	copy_dir_name
	push	ebx
	mov	al,[focus_pointer]
	test	al,al
	je	@f
	mov	ebx,user_selected_name
@@:
	cmp	[open_dialog_type],2
	je	@f
	mov	esi,file_name
	call	copy_dir_path
@@:
	mov	eax,[communication_area]
	test	eax,eax
	jnz	@f
	call	control_minimal_window_size
	call	draw_window
	pop	ebx
	ret
@@:
	mov	edi,eax
	add	edi,16	;12
	mov	esi,file_name	
	call	copy_dir_name
	
	pop	esi
	mov	al,[focus_pointer]
	test	al,al
	jz	@f
	mov	esi,user_selected_name
@@:
	mov	edi,[communication_area]
	add	edi,3840 ;4096-256
	call	copy_dir_name
	
	mov	eax,[communication_area]
	mov	[eax],word 1
	jmp	button.exit
;---------------------------------------------------------------------
load_root_directory:
	mov	esi,root_pach
	mov	edi,dir_path
	call	copy_dir_name
	call	load_directory
	mov	eax,[N_error]
	test	eax,eax
	jnz	button.exit

	mov	eax,[dirinfo.return]
	mov	[root_folder_area],eax
	mov	eax,[eax+4]
	mov	[root_folder_block],eax

	xor	eax,eax
	mov	[dirinfo.return],eax
	mov	[file_browser_data_1.folder_data],eax
	mov	[temp_counter_1],eax	;0

	mov	[retrieved_devices_table_counter],eax	;0
.start_temp_counter_1:
	imul	esi,[temp_counter_1],304
	add	esi,[root_folder_area]
	add	esi,32+40
	mov	edi,dir_path+1
	mov	[edi-1],byte '/'
	call	copy_dir_name
	call	load_directory
	mov	eax,[N_error]
	test	eax,eax
	jnz	button.exit

	mov	eax,[dirinfo.return]
	mov	[root1_folder_area],eax
	mov	eax,[eax+4]
	test	eax,eax
	jz	.continue
	mov	[root1_folder_block],eax
	
	mov	ebp,0
.start_copy_device_patch:
	imul	edi,[retrieved_devices_table_counter],10
	add	edi,retrieved_devices_table
	mov	[edi],byte '/'
	inc	edi
	imul	esi,[temp_counter_1],304
	add	esi,[root_folder_area]
	add	esi,32+40

	call	copy_dir_name

	imul	esi,ebp,304
	add	esi,[root1_folder_area]
	add	esi,32+40
	mov	[edi-1],byte '/'

	call	copy_dir_name

	inc	[retrieved_devices_table_counter]
	inc	ebp
	cmp	ebp,[root1_folder_block]
	jb	.start_copy_device_patch
.continue:
	inc	[temp_counter_1]
	mov	eax,[temp_counter_1]
	cmp	eax,[root_folder_block]
	jb	.start_temp_counter_1

	cmp	[root_folder_area],dword 0
	je	@f
	mcall	68,13,[root_folder_area]
	test	eax,eax
	jz	memory_free_error
@@:

	xor	ecx,ecx
	mov	edi,menu_text_area_1_1	;.1
@@:
	imul	esi,ecx,10
	add	esi,retrieved_devices_table
	call	copy_dir_name
	inc	ecx
	cmp	ecx,[retrieved_devices_table_counter]
	jb	@b
	mov	[menu_data_1.text_end],edi
	xor	eax,eax
	mov	[edi],eax
	ret
;---------------------------------------------------------------------
memory_free_error:
	mov	[N_error],3
	jmp	button.exit
;---------------------------------------------------------------------
memory_get_error:
	mov	[N_error],4
	jmp	button.exit
;---------------------------------------------------------------------
type_title:
	mov	ecx,[open_dialog_type]
	shl	ecx,2
	add	ecx,open_dialog_title_pointer
	mov	ecx,[ecx]
	test	ecx,ecx
	jz	@f
	mcall	71,1,; title ;;param ;file_name ;dir_path
@@:
	ret
;---------------------------------------------------------------------
draw_window:

	mcall	12,1

;	mcall	0,<10,420>,<10,320>,0x63AABBCC,
	xor	esi,esi
	mcall	0,[window_x],[window_y],0x63AABBCC,

;	mov	ecx,[communication_area]
;	add	ecx,4096+4+4
	call	type_title
	call	get_window_param
	
	mov	eax,[procinfo+70] ;status of window
	test	eax,100b
	jne	.end
	
	mov	eax,[window_high]
	sub	eax,25+45
	mov	[file_browser_data_1.size_y],ax
	mov	[scroll_bar_data_vertical.size_y],ax
	
	mov	eax,[window_width]
	sub	eax,10+20
	mov	[file_browser_data_1.size_x],ax
	add	ax,10
	mov	[scroll_bar_data_vertical.start_x],ax
	
	
	mcall	13,[window_width],45,0xcccccc

	push	ecx
	rol	ecx,16
	add	cx,[file_browser_data_1.size_y]
	add	cx,45
	ror	ecx,16
	mov	cx,25
	mcall
	pop	ecx
	add	ecx,45 shl 16
	mov	cx,[file_browser_data_1.size_y]
	mov	bx,10
	mcall
	mov	bx,[file_browser_data_1.size_x]
	add	bx,10
	shl	ebx,16
	mov	bx,20
	mcall

	cmp	[error_window],0
	je	@f
	call	draw_for_fs_errors
	jmp	.1
@@:
	call	draw_draw_file_browser1
.1:
	push	dword menu_data_1
	call	[menu_bar_draw]
	push	dword menu_data_2
	call	[menu_bar_draw]
	push	dword menu_data_3
	call	[menu_bar_draw]

	mov	ebx,[file_browser_data_1.x]
	mov	ax,bx
	shl	eax,16
	add	ebx,eax
	mov	eax,50
	mov	bx,ax
	shl	eax,16
	sub	ebx,eax
	mov	ecx,26 shl 16+15

	mcall	8,,,2,0xffffff

	pusha
	shr	ecx,16
	mov	bx,cx
	add	ebx,20 shl 16+2
	mcall	4,,0x90000000,message_ExitDir_button
	add	ebx,4
	mcall
	add	ebx,4
	mcall
	popa

	push	ebx
	sub	ebx,70 shl 16
	mov	bx,60
	mcall	8,,,6

	shr	ecx,16
	mov	bx,cx
	add	ebx,5 shl 16+4
	mcall	4,,0x90000000,message_ReloadDir_button
	pop	ebx

	mov	ebx,[file_browser_data_1.x]
	
	mov	ax,bx
	shl	eax,16
	add	ebx,eax
	mov	eax,55
	mov	bx,ax
	shl	eax,16
	sub	ebx,eax

	mov	ecx,[file_browser_data_1.y]
	mov	ax,cx
	add	eax,3
	shl	eax,16
	add	ecx,eax
	mov	cx,15

	mcall	8,,,3

	pusha

	shr	ecx,16
	mov	bx,cx
	add	ebx,6 shl 16+ 4
	mcall	4,,0x90000000,message_cancel_button
	popa

	sub	ebx,65 shl 16
	mcall	8,,,4
	
	shr	ecx,16
	mov	bx,cx
	add	ebx,12 shl 16+4
	
	mov	edx,[open_dialog_type]
	shl	edx,2
	add	edx,message_open_dialog_button
	mov	edx,[edx]
	
	cmp	[open_dialog_type],2	; Select dir
	jne	@f
	sub	ebx,5 shl 16
@@:
	
	mcall	4,,0x90000000	;message_open_button
	
;	mcall	47,0x80000,[file_browser_data_1.ini_file_start],<250,0>,0x0
;	mcall	4,<3,420>,0,fb_extension_start,3
.end:
	mcall	12,2

	ret
;---------------------------------------------------------------------
draw_for_fs_errors:
	call	draw_dir_path

	mov	ebx,[file_browser_data_1.x]
	mov	ecx,[file_browser_data_1.y]
	mcall	13,,,[file_browser_data_1.background_color]
	push	ebx ecx
	add	ebx,10 shl 16
	sub	ebx,20
	add	ecx,10 shl 16
	sub	ecx,20
	mov	edx,0xff0000
	mcall

	shr	ecx,16
	mov	bx,cx
	add	ebx,5 shl 16+15
	mcall	4,,0x90ffffff,load_directory_error_type

	add	ebx,20
	mcall	4,,,dir_path	

	mov	eax,[error_type]
	shl	eax,2
	add	eax,error_fs_text_pointers
	mov	edx,[eax]
	add	ebx,20
	mcall	4

	pop	ecx ebx

	mov	ebx,[file_browser_data_1.x]
	mov	ax,bx
	shr	eax,1
	shl	eax,16
	add	ebx,eax
	mov	eax,50
	mov	bx,ax
	shr	eax,1
	shl	eax,16
	sub	ebx,eax

	mov	ecx,[file_browser_data_1.y]
	mov	ax,cx
	sub	eax,40
	shl	eax,16
	add	ecx,eax
	mov	cx,15

	mcall	8,,,5,0xffffff

	shr	ecx,16
	mov	bx,cx
	add	ebx,4 shl 16+4
	mcall	4,,0x90000000,message_cancel_button


	ret
;---------------------------------------------------------------------	
draw_file_name:
	mov	esi,user_selected_name
	cld
@@:
	lodsb
	test	al,al
	jne	@r
	sub	esi,user_selected_name
	mov	eax,esi
	dec	eax
	
	mov	edi,edit1
	mov	[edi+48],eax ;ed_size
	mov	[edi+52],eax ;ed_pos
;--------------------------------------
	mov	eax,[file_browser_data_1.x]
	mov	ebx,eax
	shr	ebx,16
	and	eax,0xffff
	sub	eax,200
	mov	[edi],eax
	add	ebx,70
	mov	[edi+4],ebx
	
	mov	eax,[file_browser_data_1.y]
	mov	ebx,eax
	shr	ebx,16
	and	eax,0xffff
	add	eax,ebx
	add	eax,5
	mov	[edi+8],eax
	
	push	dword name_editboxes
	call	[edit_box_draw]	
	
	mov	bx,[file_browser_data_1.start_x]
	add	bx,5
	shl	ebx,16
	mov	bx,[file_browser_data_1.start_y]
	add	bx,[file_browser_data_1.size_y]
	add	bx,9
	mcall	4,,0x80000000,message_file_name
	ret
;---------------------------------------------------------------------
draw_dir_path:
	mov	eax,[file_browser_data_1.x]
	mov	ebx,eax
	shr	ebx,16
	add	ebx,3
	and	eax,0xffff
	sub	eax,5
	
	mov	[PathShow_data_1.area_size_x],ax
	mov	[PathShow_data_1.start_x],bx
;--------------------------------------
; top line
	mov	ebx,[file_browser_data_1.x]
	mcall	13,,<7,1>,0x0
; down line
	push	ebx ecx
	mcall	,,<21,1>,
	pop	ecx ebx
; left line	
	push	ebx
	mov	bx,1
	mov	cx,15
	mcall
	pop	ebx
; right line
	mov	ax,bx
	shr	ebx,16
	add	bx,ax
	dec	ebx
	shl	ebx,16
	mov	bx,1
	mcall	13
;--------------------------------------	
	mov	ebx,[file_browser_data_1.x]
	sub	ebx,2
	add	ebx,1 shl 16
	mcall	13,,<8,13>,0xffffff
;--------------------------------------
; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]
	
; draw for PathShow
	push	dword PathShow_data_1
	call	[PathShow_draw]
	
	ret
	
;draw_dir_path_1:
;	mov	ebx,[file_browser_data_1.x]
;	mcall	13,,<7,15>,0xffffb0
;	mov	bx,10
;	add	ebx,4 shl 16
;	mcall	4,,0xC0000000,dir_path,,0xffffb0
;	ret
;---------------------------------------------------------------------
draw_draw_file_browser1:
	call	draw_dir_path
	cmp	[open_dialog_type],1
	jne	@f	
	call	draw_file_name
@@:
	xor	eax,eax
	inc	eax
	mov	[file_browser_data_1.all_redraw],eax
	mov	[scroll_bar_data_vertical.all_redraw],eax
	
	push	dword file_browser_data_1
	call	[FileBrowser_draw]
	
	
	call	prepare_scrollbar_data

	call	draw_scrollbar

	xor	eax,eax
	mov	[file_browser_data_1.all_redraw],eax
	mov	[scroll_bar_data_vertical.all_redraw],eax
	ret
;---------------------------------------------------------------------
draw_draw_file_browser2:
	mov	eax,2
	mov	[file_browser_data_1.all_redraw],eax

	call	get_scrollbar_data

	push	dword file_browser_data_1
	call	[FileBrowser_draw]

	xor	eax,eax
	mov	[file_browser_data_1.all_redraw],eax
	ret
;---------------------------------------------------------------------
draw_scrollbar1:
	mov	eax,[file_browser_data_1.start_draw_line]
	mov	[scroll_bar_data_vertical.position],eax

	call	draw_scrollbar

	ret
;---------------------------------------------------------------------
draw_scrollbar:
	mov	eax,[scroll_bar_data_vertical.max_area]
	cmp	eax,[scroll_bar_data_vertical.cur_area]
	jbe	@f
	cmp	[scroll_bar_data_vertical.cur_area],0
	je	@f
	push	dword scroll_bar_data_vertical
	call	[scrollbar_ver_draw]
@@:
	ret
;---------------------------------------------------------------------
get_scrollbar_data:
	mov	eax,[scroll_bar_data_vertical.position]
	mov	[file_browser_data_1.start_draw_line],eax
	ret
;---------------------------------------------------------------------
prepare_scrollbar_data:
	mov	eax,[file_browser_data_1.folder_block]
	mov	[scroll_bar_data_vertical.max_area],eax
	mov	eax,[file_browser_data_1.max_panel_line]
	mov	[scroll_bar_data_vertical.cur_area],eax
	ret
;---------------------------------------------------------------------
get_active_pocess:
;	mcall	9,procinfo,-1
;	mov	eax,[ebx+30]
;	mov	[PID],eax
;	xor	ecx,ecx
;@@:
;	inc	ecx
;	mcall	9,procinfo
;	mov	eax,[PID]
;	cmp	eax,[ebx+30]
;	jne	@r
;	mov	[active_process],ecx

	mcall	9,procinfo,-1
	mov	ecx,[ebx+30]	; PID
	mcall	18,21
	mov	[active_process],eax	; WINDOW SLOT
	mov	ebx,[communication_area]	
	test	ebx,ebx
	jz	.1
	mov	[ebx+12],eax	; WINDOW SLOT to com. area
.1:
	ret
;---------------------------------------------------------------------
get_window_param:
	mcall	9,procinfo,-1
	mov	eax,[ebx+66]
	inc	eax
	mov	[window_high],eax
	mov	eax,[ebx+62]
	inc	eax
	mov	[window_width],eax
	mov	eax,[ebx+70]
	mov	[window_status],eax
	ret
;---------------------------------------------------------------------
convert_icons:
	xor	eax,eax
	mov	[return_code],eax
;	mov	eax,image_file
	push	image_file
	call	[cnv_png_import.Start]

	mov	ecx,[image_file]
	mcall	68,13,
	test	eax,eax
	jz	memory_free_error

	cmp	[return_code],dword 0
	je	@f
	mov	[N_error],6
	jmp	button.exit
@@:

	mov	ebx,[raw_pointer]
	mov	eax,[ebx+4]
; set of icon size x
	mov	[file_browser_data_1.icon_size_x],ax
; mov eax,[ebx+8]
; set of icon size y
	mov	[file_browser_data_1.icon_size_y],ax
	inc	ax
	mov	[file_browser_data_1.line_size_y],ax
	mov	eax,[ebx+12]
; set of RAW resolution to pixel
	mov	[file_browser_data_1.resolution_raw],eax

	mov	eax,[ebx+20]
	add	eax,ebx
; set RAW palette,use else resolution 8bit or less
	mov	[file_browser_data_1.palette_raw],eax

	mov	eax,[ebx+28]
	add	eax,ebx
; set RAW area for icon
	mov	[file_browser_data_1.icon_raw_area],eax
	ret
;---------------------------------------------------------------------
calc_ini:
	mov	eax,[image_file]
	mov	[file_browser_data_1.ini_file_start],eax
	add	eax,[img_size]
	mov	[file_browser_data_1.ini_file_end],eax
	ret
;---------------------------------------------------------------------
load_ini:
	mov	ebx,ini_file_name
	mov	esi,path
	mov	edi,file_name
	call	copy_file_path

	mov	[fileinfo.subfunction],dword 5
	mov	[fileinfo.size],dword 0
	mov	[fileinfo.return],dword file_info
	mcall	70,fileinfo
	test	eax,eax
	jnz	.error

	mov	[fileinfo.subfunction],dword 0

	mov	ecx,[file_info+32]
	mov	[fileinfo.size],ecx
	mov	[img_size],ecx
	
	mcall	68,12
	test	eax,eax
	jz	memory_get_error

	mov	[fileinfo.return],eax
	mov	[image_file],eax

	mcall	70,fileinfo
	test	eax,eax
	jnz	.error
	ret
.error:
	mov	[N_error],1
	mov	[error_type],eax
	jmp	button.exit
;---------------------------------------------------------------------
load_icons:
	mov	ebx,icons_file_name_2
	mov	esi,path
	mov	edi,file_name
	call	copy_file_path

	mov	[fileinfo.subfunction],dword 5
	mov	[fileinfo.size],dword 0
	mov	[fileinfo.return],dword file_info
	mcall	70,fileinfo
	test	eax,eax
	jz	@f
	
	mov	ebx,icons_file_name
	mov	esi,path
	mov	edi,file_name
	call	copy_file_path

	mov	[fileinfo.subfunction],dword 5
	mov	[fileinfo.size],dword 0
	mov	[fileinfo.return],dword file_info
	mcall	70,fileinfo
	test	eax,eax
	jnz	.error
@@:
	mov	[fileinfo.subfunction],dword 0

	mov	ecx,[file_info+32]
	mov	[fileinfo.size],ecx
	mov	[img_size],ecx
	
	mcall	68,12
	test	eax,eax
	jz	memory_get_error

	mov	[fileinfo.return],eax
	mov	[image_file],eax

	mcall	70,fileinfo
	test	eax,eax
	jnz	.error
	ret
.error:
	mov	[N_error],2
	mov	[error_type],eax
	jmp	button.exit
;---------------------------------------------------------------------
sort_directory:
	mov	eax,[file_browser_data_1.folder_data]
	mov	ebx,[eax+4]	; number of files
	add	eax,32
	cmp	[eax+40],word '..'
	jne	@f
	cmp	[eax+40+2],byte 0
	jne	@f
	dec	ebx
	add	eax,304
@@:
	push	dword [sort_type]	; sort mode
	push	ebx	; number of files
	push	eax	; data files
	call	[sort_dir]
	ret
;--------------------------------------------------------------------
load_directory:
	xor	eax,eax
	mov	[N_error],eax
	cmp	[file_browser_data_1.folder_data],eax
	je	@f
	mcall	68,13,[file_browser_data_1.folder_data]
	test	eax,eax
	jz	memory_free_error

@@:
	mov	[dirinfo.size],dword 0
	mov	[dirinfo.return],dir_header
	mcall	70,dirinfo
	test	eax,eax
	jz	@f
;	mov	esi,previous_dir_path
;	mov	edi,dir_path
;	call	copy_dir_name.1
;	mcall	70,dirinfo
;	test	eax,eax
;	jz	@f	
	xor	ebx,ebx
	mov	[file_browser_data_1.folder_data],ebx
	jmp	.error
@@:
	
	mov	ecx,[dir_header.totl_blocks]
	mov	[dirinfo.size],ecx
	imul	ecx,304
	add	ecx,32
	mcall	68,12
	test	eax,eax
	jz	memory_get_error

	mov	[dirinfo.return],eax
	mov	[file_browser_data_1.folder_data],eax

	mcall	70,dirinfo
	test	eax,eax
	jnz	.error
	
; test for empty directory
	mov	eax,[dirinfo.return]
	mov	eax,[eax+4]
	test	eax,eax
	jz	@f
	
	call	delete_point_dir
	call	files_name_normalize
	call	check_filter
	call	prepare_extension_and_mark
	call	clear_data_fb_and_sb
@@:
	ret

.error:
	mov	[N_error],5
	mov	[error_type],eax
	ret
;---------------------------------------------------------------------
clear_data_fb_and_sb:
	xor	eax,eax
	mov	[file_browser_data_1.start_draw_cursor_line],ax
	mov	[file_browser_data_1.start_draw_line],eax
	mov	[scroll_bar_data_vertical.position],eax	
	ret
;---------------------------------------------------------------------
check_filter:
	cmp [open_dialog_type],2	; Select dir
	je	.1
	xor	eax,eax
	mov	al,[filter_flag]
	test	eax,eax
	jz	@f

	mov	eax,[communication_area]
	test	eax,eax
	jz	@f
	mov	eax,[eax+4096]
	test	eax,eax
	jz	@f
.1:
	call	delete_unsupported_BDFE
@@:
	ret
;---------------------------------------------------------------------
delete_unsupported_BDFE:
	mov	ebx,[file_browser_data_1.folder_data]
	add	ebx,4
	xor	ecx,ecx
	dec	ecx
	
	mov	eax,[file_browser_data_1.folder_data]
	add	eax,32+40
	sub	eax,304
.start:
	inc	ecx
	add	eax,304
.1:
	cmp	[ebx],ecx
	je	.end
	cmp	[eax],byte '.'
	jne	@f
	cmp	[eax+1],byte 0
	je	.delete
@@:
	test	[eax-40],byte 0x10
	jnz	.start
	
	cmp	[open_dialog_type],2	; Select dir
	je	.delete
	
	push	eax ebx
	mov	esi,eax
	call	search_expansion
	test	eax,eax
	pop	ebx eax
	jnz	.delete
	
	push	eax ebx ecx esi
	mov	edi,[communication_area]
	add	edi,4100
	call	compare_expansion
	test	eax,eax
	pop	esi ecx ebx eax
	jz	.start
	
;-------------------------------------------
.delete:
	dec	dword [ebx]
	mov	esi,[ebx]
	sub	esi,ecx

	push	ecx
	mov	ecx,esi
	imul	ecx,304/4
	mov	edi,eax
	sub	edi,40
	mov	esi,edi
	add	esi,304
	cld
	rep	movsd
	pop	ecx
	
	jmp	.1
.end:
	ret
;---------------------------------------------------------------------
search_expansion:
	mov	edi,esi
	xor	eax,eax
@@:
	cld
	lodsb
	test	eax,eax
	jnz	@b
	mov	ebx,esi
	dec	esi
@@:
	std
	lodsb
	cmp	esi,edi
	jb	.end_err
	cmp	al,'.'
	jne	@b
	
	add	esi,2
	sub	ebx,esi
	mov	[expansion_length],ebx
	cld
	xor	eax,eax
	ret
	
.end_err:
	cld
	xor	eax,eax
	inc	eax
	ret
;---------------------------------------------------------------------
compare_expansion:
	mov	ebx,[edi]
	add	ebx,edi
	add	edi,3
.start:
	cmp	ebx,edi
	jb	.end_err
	mov	ecx,[expansion_length]
	inc	edi
	
	push	esi edi
@@:
	cld
	lodsb
	xchg	esi,edi
	shl	eax,8
	lodsb
	xchg	esi,edi
	call	char_todown
	xchg	al,ah
	call	char_todown
	cmp	al,ah
	jne	@f
	dec	ecx
	jnz	@b
	jmp	.end
@@:
	pop	edi esi
	jmp	.start
.end:
	pop	edi esi
	xor	eax,eax
	ret
	
.end_err:
	xor	eax,eax
	inc	eax
	ret
;---------------------------------------------------------------------
prepare_extension_and_mark:
	mov	esi,[dirinfo.return]
	mov	ebp,[esi+4]
	add	esi,32+40
.start:
	push	esi
	call	search_extension_start
	mov	eax,esi
	pop	esi
	sub	eax,esi
	sub	ebx,esi
	shl	eax,16
	mov	ax,bx
	mov	[esi+300-40],eax
	mov	[esi+299-40],byte 0
	add	esi,304
	dec	ebp
	jnz	.start
	ret
;---------------------------------------------------------------------
search_extension_start:
	mov	edx,esi
	xor	eax,eax
	cld
@@:
	lodsb
	test	eax,eax
	jnz	@b
	dec	esi
	dec	edx
	push	esi
	std
@@:
	lodsb
	cmp	esi,edx
	je	.end
	cmp	al,'.'
	jnz	@b
	add	esi,2
	cld
	pop	ebx
	ret
.end:
	cld
	pop	esi
	mov	ebx,esi
	ret
;---------------------------------------------------------------------
delete_point_dir:
	mov	eax,[dirinfo.return]
	cmp	[eax+32+40],byte '.'
	jne	@f
	cmp	[eax+32+40+1],byte 0
	jne	@f
	mov	edi,eax
	add	edi,32
	mov	esi,edi
	add	esi,304
	mov	ecx,[eax+4]
	dec	ecx
	mov	[eax+4],ecx
	imul	ecx,304
	shr	ecx,2
	cld
	rep	movsd
@@:
	ret
;---------------------------------------------------------------------
files_name_normalize:
	mov	esi,[dirinfo.return]
	mov	ebp,[esi+4]
	add	esi,32+40
.start:
	push	esi
	mov	al,[esi]
	call	char_toupper
	mov	[esi],al
@@:
	inc	esi
	mov	al,[esi]
	test	al,al
	jz	@f
	call	char_todown
	mov	[esi],al
	jmp	@b
@@:
	pop	esi
	add	esi,304
	dec	ebp
	jnz	.start
	ret
;---------------------------------------------------------------------
char_toupper:
; convert character to uppercase,using cp866 encoding
; in: al=symbol
; out: al=converted symbol
	cmp	al,'a'
	jb	.ret
	cmp	al,'z'
	jbe	.az
	cmp	al,' '
	jb	.ret
	cmp	al,'à'
	jb	.rus1
	cmp	al,'ï'
	ja	.ret
; 0xE0-0xEF -> 0x90-0x9F
	sub	al,'à'-''
.ret:
	ret
.rus1:
; 0xA0-0xAF -> 0x80-0x8F
.az:
	and	al,not	0x20
	ret
;---------------------------------------------------------------------
char_todown:
; convert character to uppercase,using cp866 encoding
; in: al=symbol
; out: al=converted symbol
	cmp	al,'A'
	jb	.ret
	cmp	al,'Z'
	jbe	.az
	cmp	al,'€'
	jb	.ret
	cmp	al,''
	jb	.rus1
	cmp	al,'Ÿ'
	ja	.ret
; 0x90-0x9F -> 0xE0-0xEF
	add	al,'à'-''
.ret:
	ret
.rus1:
; 0x80-0x8F -> 0xA0-0xAF
.az:
	add	al,0x20
	ret
;---------------------------------------------------------------------
copy_file_path:
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	mov	esi,edi
	dec	esi
	std
@@:
	lodsb
	cmp	al,'/'
	jnz	@b
	mov	edi,esi
	add	edi,2
	mov	esi,ebx
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	ret
;---------------------------------------------------------------------
copy_dir_path:
	mov	ecx,esi
	inc	ecx
	inc	ecx
	xor	eax,eax
	cld
@@:
	lodsb
	test	eax,eax
	jnz	@b

	cmp	ecx,esi
	jb	@f
	dec	esi
@@:
	mov	[esi-1],byte '/'
	mov	edi,esi
	mov	esi,ebx
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	ret
;---------------------------------------------------------------------
copy_exit_dir:
	mov	ebx,esi
	inc	ebx
	xor	eax,eax
	cld
@@:
	lodsb
	test	eax,eax
	jnz	@b
	sub	esi,2
	std
@@:
	lodsb
	cmp	al,'/'
	jnz	@b
	xor	eax,eax
	cmp	ebx,esi
	jb	@f
	inc	esi
@@:
	mov	[esi+1],al
	cld
	ret
;---------------------------------------------------------------------
copy_dir_name:
	push	esi edi
	mov	esi,edi
	mov	edi,previous_dir_path
	call	.1
	pop	edi esi
.1:
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	ret
;---------------------------------------------------------------------
;---------------------------------------------------------------------

;plugins_directory	db 'plugins/',0
plugins_directory	db 0

system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_CnvPNG	db '/sys/lib/cnv_png.obj',0
system_dir_Sort		db '/sys/lib/sort.obj',0
system_dir_UNPACK	db '/sys/lib/archiver.obj',0

ihead_f_i:
ihead_f_l	db 'System	error',0

er_message_found_lib	db 'box_lib.obj - Not found!',0
er_message_import	db 'box_lib.obj - Wrong import!',0

er_message_found_lib2	db 'cnv_png.obj - Not found!',0
er_message_import2	db 'cnv_png.obj - Wrong import!',0

err_message_found_lib3	db 'sort.obj - Not found!',0
err_message_import3	db 'sort.obj - Wrong import!',0

err_message_found_lib4	db 'archiver.obj - Not found!',0
err_message_import4	db 'archiver.obj - Wrong import!',0

align	4
l_libs_start:
library01	l_libs	system_dir_Boxlib+9,path,file_name,system_dir_Boxlib,\
er_message_found_lib,ihead_f_l,Box_lib_import,er_message_import,ihead_f_i,plugins_directory

library02	l_libs	system_dir_CnvPNG+9,path,file_name,system_dir_CnvPNG,\
er_message_found_lib2,ihead_f_l,cnv_png_import,er_message_import2,ihead_f_i,plugins_directory

library03	l_libs	system_dir_Sort+9,path,file_name,system_dir_Sort,\
err_message_found_lib3,ihead_f_l,Sort_import,err_message_import3,ihead_f_i,plugins_directory

library04	l_libs	system_dir_UNPACK+9,path,file_name,system_dir_UNPACK,\
err_message_found_lib4,ihead_f_l,UNPACK_import,err_message_import4,ihead_f_i,plugins_directory

end_l_libs:

;---------------------------------------------------------------------
align	4
UNPACK_import:
;unpack_Version			dd aUnpack_Version
;unpack_PluginLoad		dd aUnpack_PluginLoad	
;unpack_OpenFilePlugin		dd aUnpack_OpenFilePlugin
;unpack_ClosePlugin		dd aUnpack_ClosePlugin
;unpack_ReadFolder		dd aUnpack_ReadFolder	
;unpack_SetFolder		dd aUnpack_SetFolder
;unpack_GetFiles		dd aUnpack_GetFiles
;unpack_GetOpenPluginInfo	dd aUnpack_GetOpenPluginInfo
;unpack_Getattr			dd aUnpack_Getattr
;unpack_Open			dd aUnpack_Open
;unpack_Read			dd aUnpack_Read
;unpack_Setpos			dd aUnpack_Setpos
;unpack_Close			dd aUnpack_Close
;unpack_DeflateUnpack		dd aUnpack_DeflateUnpack
unpack_DeflateUnpack2		dd aUnpack_DeflateUnpack2
	dd 0
	dd 0

;aUnpack_Version		db 'version',0
;aUnpack_PluginLoad		db 'plugin_load',0
;aUnpack_OpenFilePlugin		db 'OpenFilePlugin',0
;aUnpack_ClosePlugin		db 'ClosePlugin',0
;aUnpack_ReadFolder		db 'ReadFolder',0
;aUnpack_SetFolder		db 'SetFolder',0
;aUnpack_GetFiles		db 'GetFiles',0
;aUnpack_GetOpenPluginInfo	db 'GetOpenPluginInfo',0
;aUnpack_Getattr		db 'getattr',0
;aUnpack_Open			db 'open',0
;aUnpack_Read			db 'read',0
;aUnpack_Setpos			db 'setpos',0
;aUnpack_Close			db 'close',0
;aUnpack_DeflateUnpack		db 'deflate_unpack',0
aUnpack_DeflateUnpack2		db 'deflate_unpack2',0

;---------------------------------------------------------------------
;---------------------------------------------------------------------
align	4
Sort_import:
sort_init	dd aSort_init
sort_version	dd aSort_version
sort_dir	dd aSort_SortDir
sort_strcmpi	dd aSort_strcmpi
	dd 0
	dd 0

aSort_init	db 'START',0
aSort_version	db 'version',0
aSort_SortDir	db 'SortDir',0
aSort_strcmpi	db 'strcmpi',0

;---------------------------------------------------------------------
align	4
cnv_png_import:
.Start		dd aCP_Start
.Version	dd aCP_Version
.Check		dd aCP_Check
.Assoc		dd aCP_Assoc
	dd 0
	dd 0
aCP_Start	db 'START',0
aCP_Version	db 'version',0
aCP_Check	db 'Check_Header',0
aCP_Assoc	db 'Associations',0
;---------------------------------------------------------------------
align	4
Box_lib_import:	
;init_lib	dd a_init
;version_lib	dd a_version


edit_box_draw		dd aEdit_box_draw
edit_box_key		dd aEdit_box_key
edit_box_mouse		dd aEdit_box_mouse
;version_ed		dd aVersion_ed

;check_box_draw	dd aCheck_box_draw
;check_box_mouse	dd aCheck_box_mouse
;version_ch		dd aVersion_ch

;option_box_draw	dd aOption_box_draw
;option_box_mouse	dd aOption_box_mouse
;version_op		dd aVersion_op

scrollbar_ver_draw	dd aScrollbar_ver_draw
scrollbar_ver_mouse	dd aScrollbar_ver_mouse
;scrollbar_hor_draw	dd aScrollbar_hor_draw
;scrollbar_hor_mouse	dd aScrollbar_hor_mouse
;version_scrollbar	dd aVersion_scrollbar

;dinamic_button_draw	dd aDbutton_draw
;dinamic_button_mouse	dd aDbutton_mouse
;version_dbutton	dd aVersion_dbutton

menu_bar_draw		dd aMenu_bar_draw
menu_bar_mouse		dd aMenu_bar_mouse
menu_bar_activate	dd aMenu_bar_activate
;version_menu_bar	dd aVersion_menu_bar

FileBrowser_draw	dd aFileBrowser_draw
FileBrowser_mouse	dd aFileBrowser_mouse
FileBrowser_key		dd aFileBrowser_key
;Version_FileBrowser	dd aVersion_FileBrowser

PathShow_prepare	dd sz_PathShow_prepare
PathShow_draw		dd sz_PathShow_draw
;Version_path_show	dd szVersion_path_show
	dd 0
	dd 0

;a_init			db 'lib_init',0
;a_version		db 'version',0

aEdit_box_draw		db 'edit_box',0
aEdit_box_key		db 'edit_box_key',0
aEdit_box_mouse		db 'edit_box_mouse',0
;aVersion_ed		db 'version_ed',0

;aCheck_box_draw	db 'check_box_draw',0
;aCheck_box_mouse	db 'check_box_mouse',0
;aVersion_ch		db 'version_ch',0

;aOption_box_draw	db 'option_box_draw',0
;aOption_box_mouse	db 'option_box_mouse',0
;aVersion_op		db 'version_op',0

aScrollbar_ver_draw	db 'scrollbar_v_draw',0
aScrollbar_ver_mouse	db 'scrollbar_v_mouse',0
;aScrollbar_hor_draw	db 'scrollbar_h_draw',0
;aScrollbar_hor_mouse	db 'scrollbar_h_mouse',0
;aVersion_scrollbar	db 'version_scrollbar',0

;aDbutton_draw		db 'dbutton_draw',0
;aDbutton_mouse		db 'dbutton_mouse',0
;aVersion_dbutton	db 'version_dbutton',0

aMenu_bar_draw		db 'menu_bar_draw',0
aMenu_bar_mouse		db 'menu_bar_mouse',0
aMenu_bar_activate	db 'menu_bar_activate',0
;aVersion_menu_bar	db 'version_menu_bar',0

aFileBrowser_draw	db 'FileBrowser_draw',0
aFileBrowser_mouse	db 'FileBrowser_mouse',0
aFileBrowser_key	db 'FileBrowser_key',0
;aVersion_FileBrowser	db 'version_FileBrowser',0

sz_PathShow_prepare	db 'PathShow_prepare',0
sz_PathShow_draw	db 'PathShow_draw',0
;szVersion_path_show	db 'version_PathShow',0
;---------------------------------------------------------------------
;---------------------------------------------------------------------
align	4
window_high			dd 0
window_width			dd 0
window_status			dd 0

active_process			dd 0
PID				dd 0
sort_type			dd 2
root_folder_area		dd 0
root_folder_block		dd 0
root1_folder_area		dd 0
root1_folder_block		dd 0
temp_counter_1			dd 0
retrieved_devices_table_counter	dd 0
communication_area		dd 0
open_dialog_type		dd 0

open_dialog_title_pointer:
	dd title_0
	dd title_1
	dd title_2
	dd 0
	
message_open_dialog_button:
	dd message_0
	dd message_1
	dd message_2
	dd 0
;---------------------------------------------------------------------
expansion_length	dd 0
;---------------------------------------------------------------------
N_error			dd 0
error_type		dd 0
error_path		dd 0
error_window_x:		dd 100 shl 16+250
error_window_y:		dd 100 shl 16+120
;---------------------------------------------------------------------
mouse_scroll_data:
.vertical	dw 0
.horizontal	dw 0

mouse_position:
.y	dw 0
.x	dw 0
;---------------------------------------------------------------------
; not	change	this	section!!!
; start	section
;---------------------------------------------------------------------
align	4
image_file	dd 0
raw_pointer	dd 0
return_code	dd 0
img_size	dd 0
deflate_unpack	dd 0
raw_pointer_2	dd 0	;+20
;---------------------------------------------------------------------
; end	section
;---------------------------------------------------------------------
align	4
fileinfo:
.subfunction	dd 5
.Offset		dd 0
.Offset_1	dd 0
.size		dd 0
.return		dd file_info
		db 0
.name:		dd file_name
;---------------------------------------------------------------------
align	4
dirinfo:
.subfunction	dd 1
.start		dd 0
.flags		dd 0
.size		dd 0
.return		dd 0
		db 0
.name:		dd dir_path
;---------------------------------------------------------------------
align	4
dir_header:
.version	dd 0	;+0
.curn_blocks	dd 0	;+4
.totl_blocks	dd 0	;+8
.other	rb	20
;---------------------------------------------------------------------
load_ini_error_type:
	db 'Error loading INI file',0

load_icons_error_type:
	db 'Error loading of icons file',0

memory_free_error_type:
	db 'Error of free memory',0

memory_get_error_type:
	db 'Memory allocation error',0

load_directory_error_type:
	db 'Error loading directory',0

convert_icons_error_type:
	db 'Unsupported or corrupt data for icons file',0
;---------------------------------------------------------------------
align	4
error_fs_text_pointers:
	dd error_fs_text_0
	dd error_fs_text_1
	dd error_fs_text_2
	dd error_fs_text_3
	dd error_fs_text_4
	dd error_fs_text_5
	dd error_fs_text_6
	dd error_fs_text_7
	dd error_fs_text_8
	dd error_fs_text_9
	dd error_fs_text_10
	dd error_fs_text_11

error_fs_text_0:	db '0 - Success full',0
error_fs_text_1:	db '1 - Base and/or partition of a hard disk is not defined',0
error_fs_text_2:	db '2 - Function is not supported for the given file system',0
error_fs_text_3:	db '3 - Unknown file system',0
error_fs_text_4:	db '4 - Reserved, is never returned in the current implementation',0
error_fs_text_5:	db '5 - File not found',0
error_fs_text_6:	db '6 - End of file, EOF',0
error_fs_text_7:	db '7 - Pointer lies outside of application memory',0
error_fs_text_8:	db '8 - Disk is full',0
error_fs_text_9:	db '9 - FAT table is destroyed',0
error_fs_text_10:	db '10 - Access denied',0
error_fs_text_11:	db '11 - Device error',0
;---------------------------------------------------------------------

extended_key	db 0

shift_flag	db 0
ctrl_flag	db 0
alt_flag	db 0

error_window	db 0

Tab_key		db 0
Tab_key_block	db 0

filter_flag	db 1

focus_pointer	db 0
;---------------------------------------------------------------------
start_pach:
	db '/rd/1',0

root_pach:
	db '/',0

icons_file_name_2	db 'buttons/'
icons_file_name		db 'z_icons.png',0
ini_file_name		db 'icons.ini',0
;---------------------------------------------------------------------

message:
	db 'Press any key...',0

message_cancel_button:
	db 'Cancel',0

message_ReloadDir_button:
	db 'Refresh',0

message_ExitDir_button:
	db '^',0

message_file_name:
	db 'File name:',0
	
message_0:
	db 'Open',0
message_1:
	db 'Save',0
message_2:
	db 'Select',0
	
	
title_0:
	db 'Open Dialog',0
title_1:
	db 'Save Dialog',0
title_2:
	db 'Select Dir',0
;---------------------------------------------------------------------
align 4
menu_data_1:
.type:		dd 0   ;+0
.x:
.size_x 	dw 80  ;+4
.start_x	dw 10	;+6
.y:
.size_y 	dw 15	;+8
.start_y	dw 26  ;+10
.text_pointer:	dd menu_text_area_1  ;0 ;+12
.pos_pointer:	dd menu_text_area_1_1 ;0 ;+16
.text_end	dd menu_text_area_1_1 ;0 ;+20
.ret_key	dd 0  ;+24
.mouse_keys	dd 0  ;+28
.x1:
.size_x1	dw 80  ;+32
.start_x1	dw 10	;+34
.y1:
.size_y1	dw 100	 ;+36
.start_y1	dw 41  ;+38
.bckg_col	dd 0xffffff  ;0xe5e5e5 ;+40
.frnt_col	dd 0xff ;+44
.menu_col	dd 0xeef0ff ;0xffffff ;+48
.select 	dd 0 ;+52
.out_select	dd 0 ;+56
.buf_adress	dd 0 ;+60
.procinfo	dd procinfo ;+64
.click		dd 0 ;+68
.cursor 	dd 0 ;+72
.cursor_old	dd 0 ;+76
.interval	dd 16 ;+80
.cursor_max	dd 0 ;+84
.extended_key	dd 0 ;+88
.menu_sel_col	dd 0x00cc00 ;+92
.bckg_text_col	dd 0 ; +96
.frnt_text_col	dd 0xffffff ;+100
.mouse_keys_old dd 0 ;+104
.font_height	dd 8 ;+108
.cursor_out	dd 0 ;+112
.get_mouse_flag dd 0 ;+116
;---------------------------------------------------------------------
menu_text_area_1:
db 'Select Disk',0
;---------------------------------------------------------------------
align 4
menu_data_2:
.type:		dd 0   ;+0
.x:
.size_x 	dw 30  ;+4
.start_x	dw 95	;+6
.y:
.size_y 	dw 15	;+8
.start_y	dw 26  ;+10
.text_pointer:	dd menu_text_area_2  ;0 ;+12
.pos_pointer:	dd menu_text_area_2.1 ;0 ;+16
.text_end	dd menu_text_area_2.end ;0 ;+20
.ret_key	dd 0  ;+24
.mouse_keys	dd 0  ;+28
.x1:
.size_x1	dw 30  ;+32
.start_x1	dw 95	;+34
.y1:
.size_y1	dw 100	 ;+36
.start_y1	dw 41  ;+38
.bckg_col	dd 0xffffff ; 0xe5e5e5 ;+40
.frnt_col	dd 0xff ;+44
.menu_col	dd 0xeef0ff  ;0xffffff ;+48
.select 	dd 0 ;+52
.out_select	dd 0 ;+56
.buf_adress	dd 0 ;+60
.procinfo	dd procinfo ;+64
.click		dd 0 ;+68
.cursor 	dd 0 ;+72
.cursor_old	dd 0 ;+76
.interval	dd 16 ;+80
.cursor_max	dd 0 ;+84
.extended_key	dd 0 ;+88
.menu_sel_col	dd 0x00cc00 ;+92
.bckg_text_col	dd 0 ; +96
.frnt_text_col	dd 0xffffff ;+100
.mouse_keys_old dd 0 ;+104
.font_height	dd 8 ;+108
.cursor_out	dd 0 ;+112
.get_mouse_flag dd 0 ;+116
;---------------------------------------------------------------------
menu_text_area_2:
db 'Sort',0
.1:
db 'Name',0
db 'Type',0
db 'Date',0
db 'Size',0
.end:
db 0
;---------------------------------------------------------------------
align 4
menu_data_3:
.type:		dd 0   ;+0
.x:
.size_x 	dw 45  ;+4
.start_x	dw 130	 ;+6
.y:
.size_y 	dw 15	;+8
.start_y	dw 26  ;+10
.text_pointer:	dd menu_text_area_3  ;0 ;+12
.pos_pointer:	dd menu_text_area_3.1 ;0 ;+16
.text_end	dd menu_text_area_3.end ;0 ;+20
.ret_key	dd 0  ;+24
.mouse_keys	dd 0  ;+28
.x1:
.size_x1	dw 95  ;+32
.start_x1	dw 130	 ;+34
.y1:
.size_y1	dw 100	 ;+36
.start_y1	dw 41  ;+38
.bckg_col	dd 0xffffff ; 0xe5e5e5 ;+40
.frnt_col	dd 0xff ;+44
.menu_col	dd 0xeef0ff  ;0xffffff ;+48
.select 	dd 0 ;+52
.out_select	dd 0 ;+56
.buf_adress	dd 0 ;+60
.procinfo	dd procinfo ;+64
.click		dd 0 ;+68
.cursor 	dd 0 ;+72
.cursor_old	dd 0 ;+76
.interval	dd 16 ;+80
.cursor_max	dd 0 ;+84
.extended_key	dd 0 ;+88
.menu_sel_col	dd 0x00cc00 ;+92
.bckg_text_col	dd 0 ; +96
.frnt_text_col	dd 0xffffff ;+100
.mouse_keys_old dd 0 ;+104
.font_height	dd 8 ;+108
.cursor_out	dd 0 ;+112
.get_mouse_flag dd 0 ;+116
;---------------------------------------------------------------------
menu_text_area_3:
db 'Filter',0
.1:
db '*.* - show all',0
db 'Only supported',0
.end:
db 0
;---------------------------------------------------------------------

align 4
scroll_bar_data_vertical:
.x:
.size_x 	dw 15 ;+0
.start_x	dw 500 ;+2
.y:
.size_y 	dw 300 ;+4
.start_y	dw 45 ;+6
.btn_high	dd 15 ;+8
.type		dd 2  ;+12
.max_area	dd 10  ;+16
.cur_area	dd 2  ;+20
.position	dd 0  ;+24
.bckg_col	dd 0xeeeeee ;+28
.frnt_col	dd 0xbbddff ;+32 ;0x8aeaa0
.line_col	dd 0  ;+36
.redraw 	dd 0  ;+40
.delta		dw 0  ;+44
.delta2 	dw 0  ;+46
.run_x:
.r_size_x	dw 0  ;+48
.r_start_x	dw 0  ;+50
.run_y:
.r_size_y	dw 0 ;+52
.r_start_y	dw 0 ;+54
.m_pos		dd 0 ;+56
.m_pos_2	dd 0 ;+60
.m_keys 	dd 0 ;+64
.run_size	dd 0 ;+68
.position2	dd 0 ;+72
.work_size	dd 0 ;+76
.all_redraw	dd 0 ;+80
.ar_offset	dd 1 ;+84
;---------------------------------------------------------------------
align 4
file_browser_data_1:
.type				dd 0 ;+0
.x:
.size_x 			dw 400 ;+4
.start_x			dw 10 ;+6
.y:
.size_y 			dw 550 ;+8
.start_y			dw 45 ;+10
.icon_size_y			dw 16 ; +12
.icon_size_x			dw 16 ; +14
.line_size_x			dw 0 ; +16
.line_size_y			dw 18 ; +18
.type_size_x			dw 0 ; +20
.size_size_x			dw 0 ; +22
.date_size_x			dw 0 ; +24
.attributes_size_x		dw 0 ; +26
.icon_assoc_area		dd 0 ; +28
.icon_raw_area			dd 0 ; +32
.resolution_raw 		dd 0 ; +36
.palette_raw			dd 0 ; +40
.directory_path_area		dd 0 ; +44
.file_name_area 		dd 0 ; +48
.select_flag			dd 0 ; +52
.background_color		dd 0xffffff ; +56
.select_color			dd 0xbbddff ; +60
.seclect_text_color		dd 0 ; +64
.text_color			dd 0 ; +68
.reduct_text_color		dd 0xff0000 ; +72
.marked_text_color		dd 0 ; +76
.max_panel_line 		dd 0 ; +80
.select_panel_counter		dd 1 ; +84
.folder_block			dd 0 ; +88
.start_draw_line		dd 0 ; +92
.start_draw_cursor_line 	dw 0 ; +96 ; pixels
.folder_data			dd 0 ; +98
.temp_counter			dd 0 ; +102
.file_name_length		dd 0 ; +106
.marked_file			dd 0 ; +110
.extension_size 		dd 0 ; +114
.extension_start		dd 0 ; +118
.type_table			dd features_table ; +122
.ini_file_start 		dd 0 ; +126
.ini_file_end			dd 0 ; +130
.draw_scroll_bar		dd 0 ; +134
.font_size_y			dw 9 ; +138
.font_size_x			dw 6 ; +140
.mouse_keys			dd 0 ; +142
.mouse_keys_old 		dd 0 ; +146
.mouse_pos			dd 0 ; +150
.mouse_keys_delta		dd 0 ; +154
.mouse_key_delay		dd 50 ; +158
.mouse_keys_tick		dd 0 ; +162
.start_draw_cursor_line_2	dw 0 ;+166
.all_redraw			dd 0 ;+168
.selected_BDVK_adress		dd 0 ;+172
.key_action			dd 0 ;+176
.name_temp_area 		dd name_temp_area ;+180
.max_name_temp_size		dd 0 ;+184
.display_name_max_length	dd 0 ;+188
.draw_panel_selection_flag	dd 0 ;+192
.mouse_pos_old			dd 0 ;+196
.marked_counter 		dd 0 ;+200
;---------------------------------------------------------------------
PathShow_data_1:
.type			dd 0	;+0
.start_y		dw 11	;+4
.start_x		dw 10	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw 200	;+10
.font_number		dd 0	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0x0	;+20
.background_color	dd 0x0	;+24
.text_pointer		dd dir_path	;+28
.work_area_pointer	dd text_work_area	;+32
.temp_text_length	dd 0	;+36
;---------------------------------------------------------------------
; for EDITBOX
align	4
name_editboxes:
edit1	edit_box 200,10,7,0xffffff,0xbbddff,0,0,0,4095,user_selected_name,mouse_dd,,0
name_editboxes_end:

;mouse_flag:	dd 0x0

mouse_dd rd	1
;---------------------------------------------------------------------
window_x:
.x_size		dw 420
.x_start	dw 10
window_y:
.y_size		dw 320
.y_start	dw 10
;---------------------------------------------------------------------
features_table:
.type_table:
	db '<DIR> '
;---------------------------------------------------------------------
.size_table:
	db '1023b '
;---------------------------------------------------------------------
.date_table:
	db '00.00.00 00:00 '
;---------------------------------------------------------------------
.year_table:
	db '    '
;---------------------------------------------------------------------
example_name_temp:	
	db 'temp1.asm',0
;---------------------------------------------------------------------
IM_END:
menu_text_area_1_1:
rb 256
;---------------------------------------------------------------------
	rb 1024
stacktop:
;---------------------------------------------------------------------
; window error message
	rb 1024
thread_stack:
;---------------------------------------------------------------------
retrieved_devices_table:
	rb 200
;---------------------------------------------------------------------
name_temp_area:
	rb 256
;---------------------------------------------------------------------
user_selected_name:
	rb 256
;---------------------------------------------------------------------
param:
	rb 256
;---------------------------------------------------------------------
path:
	rb 4096
;---------------------------------------------------------------------
file_name:
	rb 4096
;---------------------------------------------------------------------
previous_dir_path:
	rb 4096
;---------------------------------------------------------------------
dir_path:
	rb 4096
;---------------------------------------------------------------------
text_work_area:
	rb 1024
;---------------------------------------------------------------------
procinfo:
process_info:
	rb 1024
;----------------------
file_info:
	rb 40
I_END:
