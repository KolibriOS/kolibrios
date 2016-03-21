dialog_line:
	    mov [string_x],eax
	    mov [string_y],ebx
	    mov [MaxSizeString],ecx

	    and [position],0
	    mov eax,string_
	    mov ebx,64/4
	    next_byte_cleare:
	    mov [eax],dword 0
	    add eax,4
	    dec ebx
	    jnz next_byte_cleare
	    call print_line
	    call print_cursor

opros:
	mcall SF_SLEEP, 5 ;fix cpu use 100%
	mcall SF_WAIT_EVENT

	cmp eax,1 ;redraw window
	jne @f
		call dialog2_draw

		mov ebx,[string_x]
		shl ebx,16
		add ebx,[string_y]
		mov edx,string_
		mov esi,[MaxSizeString]
		mcall SF_DRAW_TEXT,,0
		jmp opros
	@@:
	cmp eax,3 ;press button
	je exit_cycle

	cmp eax,2 ;press key
	jne opros

	    mcall SF_GET_KEY
	    shr eax,8

	    cmp al,13
	    je exit_cycle
	    cmp al,8
	    je backspace
	    cmp al,176
	    je left
	    cmp al,179
	    je right
	    cmp al,32
	    je probel

	    inc [position]
	    inc [MaxSizeString]
	    call MinMaxCorrect
	    dec [MaxSizeString]
	    test ecx,ecx
	    jz no_maximum_position
	    jmp opros
	    no_maximum_position:


	    mov ebx,[position]
	    dec ebx
	    mov [string_+ebx],al

	    call print_line
	    call print_cursor
	    jmp  opros
  backspace:
	    dec [position]
	    call MinMaxCorrect
	    mov ebx,[position]
	    mov [string_+ebx],byte ' '

	    call print_line
	    call print_cursor
	    jmp opros

       left:
	    dec [position]
	    call MinMaxCorrect
	    call print_line
	    call print_cursor
	    jmp opros

      right:
	    inc [position]
	    call MinMaxCorrect
	    call print_line
	    call print_cursor
	    jmp opros

     probel:
	    mov ebx,[position]
	    mov [string_+ebx],byte ' '
	    inc [position]
	    call MinMaxCorrect
	    call print_line
	    call print_cursor
	    jmp opros

 exit_cycle:
	     inc [position]
	     mov ebx,[position]
	     mov [string_+ebx],byte 13
	     exit_opros:

	     call print_line
	    ret
;---------------------------------------------------------------
print_line:
	mov ebx,[string_x]
	mov ecx,[string_y]
	mov edx,[MaxSizeString]
	shl ebx,16
	shl ecx,16
	imul edx,6
	add ebx,edx
	add ebx,2
	add ecx,10
	mov edx,0xffffff
	mcall SF_DRAW_RECT
	mov ebx,[string_x]
	shl ebx,16
	add ebx,[string_y]
	mov edx,string_
	mov esi,[MaxSizeString]
	mcall SF_DRAW_TEXT,,0
	ret

print_cursor:
	mov ebx,[position]
	mov ecx,[string_y]
	imul ebx,6
	add ebx,[string_x]
	shl ebx,16
	shl ecx,16
	add ebx,2
	add ecx,8
	mcall SF_DRAW_RECT,,,0xff6c58
	ret
;----------------------------------------------------------
MinMaxCorrect:
	    mov ebx,[MaxSizeString]
	    xor ecx,ecx

	    cmp [position],ebx
	    jl no_maximum_length_string
	    mov [position],ebx
	    mov ecx,1	 ;maximul position
	    no_maximum_length_string:

	    cmp [position],0
	    jns no_minimum_length_string
	    and [position],0
	    no_minimum_length_string:

	    ret
;----------------------------------------------------------
string_       rb 65
position      dd 0
MaxSizeString dd 0
string_x      dd 0
string_y      dd 0

;----------------------------------------------------------
;input:
; eax - pointer to string
;output:
; eax - value
str_to_int:
	xor edx,edx
  cycle_value_:
	xor ebx,ebx
	mov bl,byte[eax]

	cmp bl,'0'
	jne no_0
	mov ebx,0
	jmp exit_v
	no_0:
	cmp bl,'1'
	jne no_1
	mov ebx,1
	jmp exit_v
	no_1:
	cmp bl,'2'
	jne no_2
	mov ebx,2
	jmp exit_v
	no_2:
	cmp bl,'3'
	jne no_3
	mov ebx,3
	jmp exit_v
	no_3:
	cmp bl,'4'
	jne no_4
	mov ebx,4
	jmp exit_v
	no_4:
	cmp bl,'5'
	jne no_5
	mov ebx,5
	jmp exit_v
	no_5:
	cmp bl,'6'
	jne no_6
	mov ebx,6
	jmp exit_v
	no_6:
	cmp bl,'7'
	jne no_7
	mov ebx,7
	jmp exit_v
	no_7:
	cmp bl,'8'
	jne no_8
	mov ebx,8
	jmp exit_v
	no_8:
	cmp bl,'9'
	jne no_9
	mov ebx,9
	jmp exit_v
	no_9:
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

;----------------------------------------------------------
align 4
dialog2_draw:
	mov	eax,80
	mov	ebx,100
	mov	ecx,150
	mov	edx,90
	mov esi,1
	call draw_volume_rectangle ; dialog 'file new'

	mov	eax,177
	mov	ebx,115
	mov	ecx,40
	mov	edx,14
	mov esi,4
	call draw_volume_rectangle

	mov	eax,177
	mov	ebx,115+24
	mov	ecx,40
	mov	edx,14
	mov esi,4
	call draw_volume_rectangle

	mov	eax,177
	mov	ebx,170
	mov	ecx,40
	mov	edx,15
	mov	esi,1
	call draw_volume_rectangle ; button [Ok]

	mov	eax,90
	mov	ebx,118
	mov	ecx,0xffffff
	mov	edx,new_text1
	mov	esi,14
	call print_text

	mov	eax,90
	mov	ebx,118+24
	mov	ecx,0xffffff
	mov	edx,new_text2
	mov	esi,14
	call print_text

	mov	eax,185
	mov	ebx,175
	mov	ecx,0xffffff
	mov	edx,ok_text
	mov	esi,2
	call print_text
	ret

length_number	dd 0