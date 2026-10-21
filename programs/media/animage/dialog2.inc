align 4
dialog_new_picture:
	call drawwin
	call dialog2_draw

	mov	edx,0x40000000+1001
	mcall SF_DEFINE_BUTTON,<137,40>,<170,15>
	mov	edx,0x40000000+1002
	mcall ,<187,40>

no_activate_space:
	mcall SF_SLEEP, 5 ;fix cpu use 100%
	mcall SF_WAIT_EVENT

	cmp eax,EV_REDRAW
	jne @f
		call dialog2_draw
		jmp no_activate_space
	@@:
	cmp eax,EV_KEY
	jne @f
	    mcall SF_GET_KEY
	    cmp ah,13
	    je exit_cycle
		
		stdcall [edit_box_key],edit1
		stdcall [edit_box_key],edit2
	    jmp no_activate_space
	@@:
	cmp eax,EV_MOUSE
	jne @f
		stdcall [edit_box_mouse],edit1
		stdcall [edit_box_mouse],edit2
	    jmp no_activate_space
	@@:
	cmp eax,EV_BUTTON
	jne no_activate_space

	mcall SF_GET_BUTTON
	shr	eax,8

	cmp	eax,1001
	je	exit_cycle
	cmp	eax,1002
	je	exit_new
	jmp	no_activate_space

exit_cycle:
	stdcall str_len,[edit1.text]
	mov	[length_number],eax
	mov	eax,[edit1.text]
	call str_to_int
	cmp eax,1
	jb no_activate_space
	mov ebx,eax

	stdcall str_len,[edit2.text]
	mov	[length_number],eax
	mov	eax,[edit2.text]
	call str_to_int
	cmp eax,1
	jb no_activate_space

;get memory for new picture
	mov	[Picture_SizeX],ebx
	mov	[Picture_SizeY],eax
	call GetMemory
	and	[save_flag],0
	call cleare_work_arrea

exit_new:
	mov	edx,0xe0000000+1002
	mcall SF_DEFINE_BUTTON,<137,40>,<170,15>
	mov	edx,0xe0000000+1002
	mcall ,<187,40>

	call drawwin
	ret

align 4
string_       rb 20
mouse_dd dd 0

edit1 edit_box 40,187,115,   0xffffff,0xbbddff,0,0,0,5,string_,\
	mouse_dd,ed_focus,0
edit2 edit_box 40,187,115+24,0xffffff,0xbbddff,0,0,0,5,string_+10,mouse_dd,0,0
;----------------------------------------------------------
;input:
; eax - pointer to string
;output:
; eax - value
align 4
proc str_to_int uses ebx ecx edx
	xor edx,edx
  cycle_value_:
	movzx ebx,byte[eax]

	cmp bl,'0'
	jl @f
	cmp bl,'9'
	jg @f
	sub bl,'0'
	jmp exit_v
	@@:
	xor edx,edx
	jmp error
	exit_v:
	mov ecx,[length_number]
	dec ecx
	test ecx,ecx
	jz no_stepen

    next_mul:
	imul ebx,10
	dec ecx
	jne next_mul

	no_stepen:
	add edx,ebx

	inc eax
	dec [length_number]
	jnz cycle_value_
	error:
	mov eax,edx
	ret
endp
;----------------------------------------------------------
align 4
dialog2_draw:
	mov	eax,80
	mov	ebx,100
	mov	ecx,160
	mov	edx,90
	mov esi,1
	call draw_volume_rectangle ; dialog 'file new'

	stdcall [edit_box_draw],edit1
	stdcall [edit_box_draw],edit2

	mov	eax,137
	mov	ebx,170
	mov	ecx,40
	mov	edx,15
	mov	esi,1
	call draw_volume_rectangle ; button [Ok]

	mov	eax,187
	mov	ebx,170
	mov	ecx,40
	mov	edx,15
	mov	esi,1
	call draw_volume_rectangle ; button [Cancel]

	mov	esi,14
	mcall SF_DRAW_TEXT,(90 shl 16)+118,0x80000000,sz_Pict_size_X
	add ebx,24
	mcall ,,,sz_Pict_size_Y
	mcall ,(152 shl 16)+175,,sz_Ok
	mcall ,(191 shl 16)+175,,sz_Cancel
	ret

length_number	dd 0