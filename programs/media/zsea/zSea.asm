;*****************************************************************************
; zSea - advanced image viewer for KolibriOS
; Copyright (c) 2008-2011, Marat Zakiyanov aka Mario79, aka Mario
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;	 * Redistributions of source code must retain the above copyright
;	   notice, this list of conditions and the following disclaimer.
;	 * Redistributions in binary form must reproduce the above copyright
;	   notice, this list of conditions and the following disclaimer in the
;	   documentation and/or other materials provided with the distribution.
;	 * Neither the name of the <organization> nor the
;	   names of its contributors may be used to endorse or promote products
;	   derived from this software without specific prior written permission.
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
;	v.1.0 rс4 25.09.2011
;******************************************************************************
	use32
	org 0x0
	db 'MENUET01'	; 8 byte id
	dd 0x01		; header version
	dd START	; start of code
	dd IM_END	; size of image
	dd I_END	; memory for app
	dd stacktop	; esp
	dd temp_area	; I_Param
	dd path		; APPLICATION PACH

include 'lang.inc'
;include 'macros.inc'
;include 'editbox_ex.mac'
;include 'proc32.inc'

include '../../macros.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../proc32.inc'

;include 'load_lib.mac'
include '../../develop/libraries/box_lib/load_lib.mac'
        @use_library    ;use load lib macros
;******************************************************************************

START:				; start of execution
	mcall	68, 11
	mcall	66, 1,1
	mcall 40, 0x27

	mcall 9, procinfo, -1
	mov	eax,[ebx+30]
	mov	[PID],eax
	xor	ecx,ecx
@@:
	inc	ecx
	mcall 9, procinfo
	mov	eax,[PID]
	cmp	eax,[ebx+30]
	jne	@r
	mov	[active_process],ecx

	mcall	48,4
	mov	[skin_height],eax
	
	mcall	68,12,1024
	mov	[menu_data_1.procinfo],eax
	mov	[menu_data_2.procinfo],eax
	mov	[menu_data_3.procinfo],eax
	mov	[menu_data_4.procinfo],eax
	mov	[menu_data_5.procinfo],eax
	mov	[menu_data_6.procinfo],eax

	mov	[load_directory_pointer],dir_header


	call	load_plugins


	call	load_buttons

;	call	init_data_OpenDialog

;init_OpenDialog	OpenDialog_data
	push    dword OpenDialog_data
	call    [OpenDialog_Init]

	call	get_filter_data

;-----------------------------------------------------
; check for parameters
	cmp	dword [temp_area],'BOOT'
	jne	.no_boot
.background:
	call	load_image
	cmp	[error_fs],0
	jnz	.exit
	call	convert

	call	background

.exit:
	call Set_ini
.exit_1:
	mov	ebx,18
	mov	edx,PID1
	mov	esi,7
.kill_successors:
	mov	ecx,[edx]
	add	edx,4
	test	ecx,ecx
	jz	@f
	mcall 18
@@:
	dec	esi
	jnz	.kill_successors
	
	mcall -1
;-----------------------------------------------------
 .no_boot:
	xor	eax,eax
	cmp	byte [temp_area],al
	jnz	@f
	mov	[file_name],eax
	jmp .no_param
@@:


	mov	edi,string	; clear string
	mov	ecx,256/4	;	length of a string
	xor	eax,eax	;	symbol <0>
	rep	stosd


	mov	edi,temp_area	; look for <0> in temp_area

	cmp	[edi],byte "\"
	jne	.continue
	cmp	[edi+1],byte "T"
	jne	@f
	mov	[bgrmode],dword 1
	jmp	.continue_1
@@:
	cmp	[edi+1],byte "S"
	jne	START.exit
	mov	[bgrmode],dword 2
.continue_1:
	add	edi,4
.continue:
	mov	esi,edi
	mov	ecx,257	;	strlen
	repne scasb
	lea		ecx, [edi-temp_area]

	mov	edi,string
	rep	movsb		; copy string from temp_area to "string" (filename)
	cmp	[temp_area],byte "\"
	je	START.background
	call	load_directory
	test	eax,eax
	jnz	@f
	call	load_image
	test	eax,eax
	jnz	@f
	call	convert
	jmp	.no_param
@@:
	mov	[load_directory_pointer],dir_header
	mov	[error_fs],eax
	call	convert.error
;-----------------------------------------------------
 .no_param:
;	or	ecx,-1		; get information about me
;	call getappinfo

;	mov edx,[process_info+30] ; теперь в edx наш идентификатор
;	mov ecx,eax

;	@@:
;	call getappinfo
;	cmp edx,[process_info+30]
;	je	@f	; если наш PID совпал с PID рассматриваемого процесса, мы нашли себя
;	dec ecx ; иначе смотрим следующий процесс
;	jne @b	; возвращаемся, если не все процессы рассмотрены
;	@@:

; теперь в ecx номер процесса
;	mov	[process],ecx
;---------------------------------------------------------------------
	cmp	[wnd_width],635
	jae	@f
	mov	[wnd_width],635
@@:
	cmp	[wnd_height],150
	jae	@f
	mov	[wnd_height],150
@@:
;	call draw_window
red:
;draw_still:
;	pusha
	call	get_window_param
	test	[window_status],10b
	jnz	red_1	;still
	test	[window_status],100b
	jnz	red_1
	test	[window_status],1b
	jnz	red_1
	mov esi,-1
	mov eax,procinfo
	mov eax,[eax+66]
	cmp	eax,150
;	cmp	[window_high],150
	jae	@f
	mov	esi,150
	mcall 67,-1,ebx,ebx
@@:
	mov edx,-1
	mov eax,procinfo
	mov eax,[eax+62]
	cmp	eax,635
;	cmp	[window_width],635
	jae	@f	;red_1
	mov	edx,635
	mcall 67,-1,ebx, ,ebx
@@:
;	mcall 67,-1,ebx
;	popa
;	xor esi,esi
red_1:
;	xor	eax,eax
;	mov	[scroll_bar_data_vertical.position],eax
;	mov	[scroll_bar_data_horizontal.position],eax

	call draw_window
	
	cmp	[redraw_wallpaper_flag],0
	je	still
	mov	[redraw_wallpaper_flag],0
	call	clear_thread
;	mcall 15,3
;	jmp  red_1
still:
	call	pause_cicle
	
	cmp	[RAW1_flag],1
	je	animation_handler
	
	mcall	48,4
	cmp	[skin_height],eax
	je	@f
	mov	[skin_height],eax
	call	convert.img_resolution_ok
	jmp	red_1
@@:
	mcall	10
.1:
	cmp	[open_file_flag],1
	je	kopen_1
	cmp	[sort_directory_flag],byte 1
	je	red_sort_directory
	cmp	[redraw_flag],byte 1
	je	redraw_window
	cmp	eax,1	; перерисовать окно ?
	je		red	; если да - на метку red
	cmp	eax,2	; нажата клавиша ?
	je		key	; если да - на key
	cmp	eax,3	; нажата кнопка ?
	je		button		; если да - на button
	cmp	eax,6
	je	mouse
;	cmp [redraw_wallpaper_flag],1
;	jne still
;	mov	[redraw_wallpaper_flag],0
;	mcall	15,3
	jmp	still	; если другое событие - в начало цикла

red_sort_directory:
	mov	[sort_directory_flag],byte 0
	jmp	red_1

redraw_window:
	mov	[redraw_flag],byte 0
	jmp	red_1
	
;---------------------------------------------------------------------
;	red:
;	test	dword [status], 4
;	jz	draw_still
;	mov	al,18
;	mov	ebx,3
;	mov	ecx,[process]
;	mcall	18,3,[active_process]
;	and	byte [status], not 4
;	jmp	still
;---------------------------------------------------------------------
button:			; button
	mov	eax,17		; get id
	mcall
	cmp	ah,1			; button id=1 ?
	je	START.exit
;	jne	.noclose
;
;	mov	eax,-1		; close this program
;	mcall
.noclose:
	cmp	ah,2
	je	slide_show.3	;still
	jmp	slide_show
;---------------------------------------------------------------------
pause_cicle:
	pusha
.start:
	mcall	9,procinfo,-1
	mov	eax,[procinfo+70] ;status of window
	test	eax,100b
	jne	@f
	popa
	ret
@@:
	mcall	10
	dec	eax
	jz	.redraw
	dec	eax
	jz	.key
	dec	eax
	jnz	.start	
.button:
	mcall	-1
.key:
	mcall	2
	jmp	.start
.redraw:
	call	draw_window
	jmp	.start
;---------------------------------------------------------------------
get_filter_data:
	mov	edi,Filter+4
	xor	eax,eax
	mov	ecx,10
	cld
@@:
	mov	esi,10
	sub	esi,ecx
	lea     esi,[esi+esi*2] ; x 3
	shl	esi,3  ; x 8
	add	esi,dword Convert_plugin_0.Assoc
	mov	esi,[esi]
	add	esi,4

	test	esi,esi
	jz	@f
	call	.start
	dec	ecx
	jnz	@r
@@:
	mov	[edi],byte 0
	mov	eax,Filter
	sub	edi,eax
	mov	[eax],edi

	ret
.start:
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@r
	cmp	[esi],ah
	jne	@r
	ret
;---------------------------------------------------------------------
kopen_1:

	mov	[open_file_flag],0
	call	load_directory
	test	eax,eax
	jz	kopen
.err:
	mov	[load_directory_pointer],dir_header
	mov	[error_fs],eax
	call	convert.error
	jmp	still	;red_1

kopen:
	cmp	[string],byte 0
	je	still

	mov	ecx,-1
	call	getappinfo

	call	load_image

	test	eax,eax
	jnz	kopen_1.err
	call	convert

.1:
	call	get_window_param
	test	[window_status],1b
	jz	red	;draw_still
	mov	[no_draw_window],1

;	push	edx
;	mov	edx,size_after_convert_2
;	call	write_memory_size
;	pop	edx

	call	draw_other

	mov	[no_draw_window],0
;	jmp	draw_still
	jmp	still
;---------------------------------------------------------------------
getappinfo:
	mov	eax,9
	mov	ebx,process_info
	mcall
	ret
;---------------------------------------------------------------------
get_window_param:
	mcall	9, procinfo, -1
	mov	eax,[ebx+34]
	mov	[window_start_x],eax
	mov	eax,[ebx+38]
	mov	[window_start_y],eax
	mov	eax,[ebx+66]   ;46]
	inc	eax
	mov	[window_high],eax
	mov	eax,[ebx+62]   ;42]
	inc	eax
	mov	[window_width],eax
	mov	eax,[ebx+70]
	mov	[window_status],eax
;	mcall	48,4
;	mov	[skin_high],eax
	ret
;---------------------------------------------------------------------
;write_memory_size:
;	pusha
;	mcall	9, procinfo, -1
;	mov	eax,[ebx+26]
;	mov	[edx],eax
;	popa
;	ret
;---------------------------------------------------------------------
include	'backgrnd.inc'
include	'draw_win.inc'
include	'full_win.inc'
include	'mouse.inc'
include	'key.inc'
include	'menu_key.inc'
include	'clr_bcgr.inc'
include	'w_error.inc'
include	'load.inc'
include	'animat.inc'
include	'draw_img.inc'
include	'convert.inc'
include	'zoom.inc'
include	'w_about.inc'
include	'win_file.inc'
include	'win_info.inc'
include	'win_bcgr.inc'
include	'win_sort.inc'
include	'win_opti.inc'
include	'libini.inc'
include	'../../../dll.inc'
include	'data.inc'
;---------------------------------------------------------------------
IM_END:
include	'dat_area.inc'
I_END:
