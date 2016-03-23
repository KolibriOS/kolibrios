macro instrument_set m_num, code
{
local no_instrument
cmp [number_menu],m_num
jne no_instrument
	mov	[Current_instrument],code
	call TakeButtonInstruments
	jmp end_menu
no_instrument:
}

;-----------------------------------------------------------
;-----------instruments of menu-----------------------------
;-----------------------------------------------------------
align 4
TakeInstruments:
;|||||||||||||||||||FILE||||||||||||||||||||||
	cmp	[number_panel],7
	jne	no_file

; <<< New >>>
	cmp	[number_menu],1
	jne	no_new

	call dialog2_draw

	mov	edx,1000000000000000000000000000000b+1000
	mcall SF_DEFINE_BUTTON,<177,40>,<115,14>,

	mov	edx,1000000000000000000000000000000b+1001
	mcall ,,<139,14>,

	mov	edx,1000000000000000000000000000000b+1002
	mcall ,,<170,15>

no_activate_space:
	mcall SF_SLEEP, 5 ;fix cpu use 100%
	mcall SF_WAIT_EVENT

	cmp eax,1 ;redraw window
	jne @f
		call dialog2_draw
		jmp no_activate_space
	@@:

	cmp eax,3 ;press button
	jne no_activate_space

	mcall SF_GET_BUTTON
	shr	eax,8

	cmp	eax,1000
	jl	no_activate_space

	cmp	eax,1000
	jne	no_picture_size_x

	mov	eax,180
	mov	ebx,119
	mov	ecx,5
	call	dialog_line

	mov	eax,string_
	mov	ebx,13
	call	find_symvol

	dec	eax
	mov	[length_number],eax
	mov	eax,string_
	call str_to_int

	mov	[Picture_SizeX],eax
	jmp	no_activate_space
;---------------------------------------------------------------------
no_picture_size_x:
	cmp	eax,1001
	jne	no_picture_size_y

	mov	eax,180
	mov	ebx,119+24
	mov	ecx,5
	call	dialog_line

	mov	eax,string_
	mov	ebx,13
	call	find_symvol

	dec	eax
	mov	[length_number],eax
	mov	eax,string_
	call str_to_int

	mov	[Picture_SizeY],eax
	jmp	no_activate_space
;---------------------------------------------------------------------
no_picture_size_y:
	cmp	eax,1002
	jne	no_activate_space

exit_new:
	mov	edx,11100000000000000000000000000000b+1002
	mcall SF_DEFINE_BUTTON,<177,40>,<170,15>

	mov	edx,11100000000000000000000000000000b+1001
	mcall	,,<139,14>

	mov	edx,11100000000000000000000000000000b+1000
	mcall	,,<115,14>

;get memory for new picture
	call GetMemory

	and	[save_flag],0

	call cleare_work_arrea
	call drawwin
	jmp	end_menu
;---------------------------------------------------------------------
no_new:
; <<< Open >>>
	cmp	[number_menu],2
	jne	no_open

;---------------------------------------------------------------------
	mov	[OpenDialog_data.type],0 ; open
	
	push    dword OpenDialog_data
	call    [OpenDialog_Start]
	
	cmp	[OpenDialog_data.status],1
	jne	end_menu
;---------------------------------------------------------------------
	mcall SF_SET_EVENTS_MASK,0x80000027 ;100111b

	call load_picture
	call drawwin
	jmp	end_menu
;---------------------------------------------------------------------
no_open:
; <<< Save >>>
	cmp	[number_menu],3
	jne	no_save_

	mov	al,[save_flag]
	test al,al
	jz end_menu
	jmp	save_enter
;---------------------------------------------------------------------
no_save_:
; <<< Save as >>>
	cmp	[number_menu],4
	jne	no_save

; savedialog drawwin,save1,save2,file_path
;---------------------------------------------------------------------
; replace extension to '.bmp'
	push	eax esi
	mov	esi,[OpenDialog_data.filename_area]
	xor	eax,eax
	cld
@@:
	lodsb
	test	eax,eax
	jnz	@r
	sub	esi,5
	cmp	[esi],byte '.'
	jne	@f
	mov	[esi],dword '.bmp'
@@:
	pop	esi eax
;---------------------------------------------------------------------
	mov	[OpenDialog_data.type],1 ; save
	stdcall [OpenDialog_Start],OpenDialog_data

	cmp	[OpenDialog_data.status],1
	jne	end_menu
;---------------------------------------------------------------------
save1:
save_enter:
	mcall SF_SET_EVENTS_MASK,0x80000027 ;100111b

	call analizing_picture_to_palette

	;eax => number of colors in picture
	mov	ebx,[PointerToPicture]
	mov	ecx,[PointerToEditBufer]
	mov	edx,[PointerToPalette]
	mov	esi,[Picture_SizeX]
	mov	edi,[Picture_SizeY]

	call	coding_bmp

	mov	eax,file_path
	mov	ecx,ebx
	mov	ebx,[PointerToEditBufer]

	call	save_file

	mov	[save_flag],1
	call drawwin
	jmp	end_menu
;---------------------------------------------------------------------
no_save:
; <<< Exit >>>
	cmp	[number_menu],5
	jne	no_exit_program

	mcall SF_TERMINATE_PROCESS
;---------------------------------------------------------------------
no_exit_program:
no_file:
;|||||||||||||||||||||||||||||EDIT|||||||||||||||||||||||||
	cmp	[number_panel],6
	jne	no_edit

; <<< Undo >>>
	cmp	[number_menu],1
	jne	no_undo

	cmp	[number_undo],2 ;2 - maximum undo count
	jge	end_menu

;rotate bufers -1
	inc	[number_undo]
	mov eax,[PointerToPicture]
	mov ebx,[PointerToCopyPicture]
	mov ecx,[PointerToCopyPicture2]
	mov [PointerToPicture],ebx
	mov [PointerToCopyPicture],ecx
	mov [PointerToCopyPicture2],eax

	call MovePictureToWorkScreen
	jmp end_menu
;---------------------------------------------------------------------
no_undo:
; <<< Redo >>>
	cmp [number_menu],2
	jne no_redo

	cmp [number_undo],0
	je end_menu

;rotate bufers +1
	dec [number_undo]
	mov eax,[PointerToPicture]
	mov ebx,[PointerToCopyPicture]
	mov ecx,[PointerToCopyPicture2]
	mov [PointerToPicture],ecx
	mov [PointerToCopyPicture],eax
	mov [PointerToCopyPicture2],ebx

	call MovePictureToWorkScreen
	jmp end_menu
;---------------------------------------------------------------------
no_redo:
; <<< Copy >>>
	cmp	[number_menu],3
	jne	no_copy

	cmp	[instrument_used],1
	jne	end_menu
	cmp	[Activate_instrument],0
	jne	end_menu

	mov	eax,[OldX]
	mov	ebx,[OldY]

	cmp	eax,[rectangular_shade_x]
	jl	no_remove_x_copy

	mov	ecx,[rectangular_shade_x]
	mov	[OldX],ecx			 ; OldX <-----> rectangulare_shade_x
	mov	[rectangular_shade_x],eax

no_remove_x_copy:
	cmp	ebx,[rectangular_shade_y]
	jl	no_remove_y_copy

	mov	ecx,[rectangular_shade_y]
	mov	[OldY],ecx			 ; OldY <-----> rectangulare_shade_y
	mov	[rectangular_shade_y],ebx

no_remove_y_copy:
	mov	eax,[OldX]
	mov	ebx,[OldY]
	mov	ecx,[rectangular_shade_x]
	mov	edx,[rectangular_shade_y]
	inc	eax
	inc	ebx
	dec	ecx
	dec	edx

	mov	[x],eax
	mov	[y],ebx
	mov	esi,eax
	mov	edi,ebx
	mov	[Dx_],1
	mov	[Dy_],1

	sub	ecx,eax
	jnz	no_signum_fill_r_x_copy

	mov	[Dx_],-1

no_signum_fill_r_x_copy:
	sub	edx,ebx
	jnz	no_signum_fill_r_y_copy

	mov	[Dy_],-1

no_signum_fill_r_y_copy:
	mov	ebx,[rectangular_shade_y]
	sub	ebx,edi

	mov	edx,[PointerToEditBufer]
	mov	[y],edi

loop_fill_rectangle_y_copy:
	mov	[x],esi

loop_fill_rectangle_x_copy:
	push	esi edi
	mov	eax,[PointerToPicture]
	mov	ebx,[Picture_SizeX]
	mov	esi,[x]
	mov	edi,[y]
	call	GetColorOfPixel
	mov	[edx],ax
	shr	eax,16
	mov	[edx+2],al
	pop	edi esi

	add	edx,3
	mov	eax,[x]
	add	eax,[Dx_]
	mov	[x],eax

	cmp	eax,[rectangular_shade_x]
	jl	loop_fill_rectangle_x_copy

	mov	eax,[y]
	add	eax,[Dy_]
	mov	[y],eax

	cmp	eax,[rectangular_shade_y]
	jl	loop_fill_rectangle_y_copy

	call	MovePictureToWorkScreen
	mov	[DrawSprite_flag],1
	jmp	end_menu
;---------------------------------------------------------------------
no_copy:
; <<< Paste >>>
	cmp	[number_menu],4
	jne	no_paste

	cmp	[instrument_used],1
	jne	no_paste

	cmp	[Activate_instrument],0
	jne	no_paste

	mov	eax,[OldX]
	mov	ebx,[OldY]

	cmp	eax,[rectangular_shade_x]
	jl	no_remove_x_paste

	mov	ecx,[rectangular_shade_x]
	mov	[OldX],ecx 		  ; OldX <-----> rectangulare_shade_x
	mov	[rectangular_shade_x],eax

no_remove_x_paste:
	cmp	ebx,[rectangular_shade_y]
	jl	no_remove_y_paste

	mov	ecx,[rectangular_shade_y]
	mov	[OldY],ecx 		  ; OldY <-----> rectangulare_shade_y
	mov	[rectangular_shade_y],ebx

no_remove_y_paste:
	mov	eax,[OldX]
	mov	ebx,[OldY]
	mov	ecx,[rectangular_shade_x]
	mov	edx,[rectangular_shade_y]
	inc	eax
	inc	ebx
	dec	ecx
	dec	edx

	mov	[x],eax
	mov	[y],ebx
	mov	esi,eax
	mov	edi,ebx
	mov	[Dx_],1
	mov	[Dy_],1

	sub	ecx,eax
	jnz	no_signum_fill_r_x_paste

	mov	[Dx_],-1

no_signum_fill_r_x_paste:
	sub	edx,ebx
	jnz	no_signum_fill_r_y_paste

	mov	[Dy_],-1

no_signum_fill_r_y_paste:
	mov	edx,[PointerToEditBufer]
	mov	[y],edi

loop_fill_rectangle_y_paste:
	mov	[x],esi

loop_fill_rectangle_x_paste:
	push	esi edi
	mov	ecx,[edx]
	and	ecx,0xffffff ;color
	mov	eax,[PointerToPicture]
	mov	ebx,[Picture_SizeX]
	mov	esi,[x]
	mov	edi,[y]
	call	PutPixel
	pop	edi esi

	add	edx,3

	mov	eax,[x]
	add	eax,[Dx_]
	mov	[x],eax
	cmp	eax,[rectangular_shade_x]
	jl	loop_fill_rectangle_x_paste

	mov	eax,[y]
	add	eax,[Dy_]
	mov	[y],eax
	cmp	eax,[rectangular_shade_y]
	jl	loop_fill_rectangle_y_paste

	call	MovePictureToWorkScreen
	mov	[Paste_flag],1
	jmp	end_menu
;---------------------------------------------------------------------
no_paste:
; <<< Cut >>>
	cmp	[number_menu],5
	jne	no_cut

	cmp	[instrument_used],1
	jne	no_cut

	cmp	[Activate_instrument],0
	jne	no_cut

	mov	eax,[OldX]
	mov	ebx,[OldY]

	cmp	eax,[rectangular_shade_x]
	jl	no_remove_x_cut

	mov	ecx,[rectangular_shade_x]
	mov	[OldX],ecx 		  ; OldX <-----> rectangulare_shade_x
	mov	[rectangular_shade_x],eax

no_remove_x_cut:
	cmp	ebx,[rectangular_shade_y]
	jl	no_remove_y_cut

	mov	ecx,[rectangular_shade_y]
	mov	[OldY],ecx 		  ; OldY <-----> rectangulare_shade_y
	mov	[rectangular_shade_y],ebx

no_remove_y_cut:
	mov	eax,[OldX]
	mov	ebx,[OldY]
	mov	ecx,[rectangular_shade_x]
	mov	edx,[rectangular_shade_y]
	inc	eax
	inc	ebx
	dec	ecx
	dec	edx

	mov	[x],eax
	mov	[y],ebx
	mov	esi,eax
	mov	edi,ebx
	mov	[Dx_],1
	mov	[Dy_],1

	sub	ecx,eax
	jnz	no_signum_fill_r_x

	mov	[Dx_],-1

no_signum_fill_r_x:
	sub	edx,ebx
	jnz	no_signum_fill_r_y

	mov	[Dy_],-1

no_signum_fill_r_y:
	mov	[y],edi

loop_fill_rectangle_y:
	mov	[x],esi

loop_fill_rectangle_x:
	push	esi edi
	mov	eax,[PointerToPicture]
	mov	ebx,[Picture_SizeX]
	mov	ecx,dword 0xffffff
	mov	esi,[x]
	mov	edi,[y]
	call	PutPixel
	pop	edi esi

	mov	eax,[x]
	add	eax,[Dx_]
	mov	[x],eax

	cmp	eax,[rectangular_shade_x]
	jl	loop_fill_rectangle_x

	mov	eax,[y]
	add	eax,[Dy_]
	mov	[y],eax

	cmp	eax,[rectangular_shade_y]
	jl	loop_fill_rectangle_y

	call	MovePictureToWorkScreen
	jmp	end_menu
;---------------------------------------------------------------------
no_cut:
; <<< Cleare all >>>
	cmp	[number_menu],6
	jne	no_cleare_all

	call	cleare_work_arrea
	call	MovePictureToWorkScreen
	jmp	end_menu
;---------------------------------------------------------------------
no_cleare_all:
; <<< To allocate all >>>
	cmp	[number_menu],7
	jne	end_menu ;no_to_allocate_all

	mov	[OldX],1
	mov	[OldY],1
	mov	eax,[Picture_SizeX]
	mov	ebx,[Picture_SizeY]
	dec	eax
	dec	ebx
	mov	[rectangular_shade_x],eax
	mov	[rectangular_shade_y],ebx
	mov	[instrument_used],1
	mov	[Activate_instrument],1
	mov	[Current_instrument],30
	mov	[crossing],0
;	call	TakeButtonInstruments
;	call	MovePictureToWorkScreen
	jmp	end_menu
;---------------------------------------------------------------------
;no_to_allocate_all:
no_edit:
;|||||||||||||||||||||||||INSTRUMENTS||||||||||||||||||||||
	cmp	[number_panel],5
	jne	no_instruments

	instrument_set  1,10 ; pensil
	instrument_set  2,11 ; draw brush
	instrument_set  3,12 ; spray
	instrument_set  4,13 ; lastik
	instrument_set  5,14 ; flood fill
	instrument_set  6,15 ; pipetka
	instrument_set  7,16 ; draw line
	instrument_set  8,17 ; draw rectangle
	instrument_set  9,18 ; draw circle
	instrument_set 10,19 ; draw ellips
	instrument_set 11,20 ; draw hard contour

no_instruments:

end_menu:
	and	[number_panel],0
	and	[number_menu],0
	jmp	still
;---------------------------------------------------------------------