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
	    mov eax,10
	    mcall

	    cmp eax,1
	    je exit_cycle

	    cmp eax,2
	    jne opros

	    mov eax,2
	    mcall
	    shr eax,8

	    cmp eax,13
	    je exit_cycle
	    cmp eax,8
	    je backspace
	    cmp eax,176
	    je left
	    cmp eax,179
	    je right
	    cmp eax,32
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
	     mov eax,13
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
	     mcall
	     mov eax,4
	     mov ebx,[string_x]
	     shl ebx,16
	     add ebx,[string_y]
	     mov ecx,0
	     mov edx,string_
	     mov esi,[MaxSizeString]
	     mcall
	     ret

print_cursor:
	     mov eax,13
	     mov ebx,[position]
	     mov ecx,[string_y]
	     imul ebx,6
	     add ebx,[string_x]
	     shl ebx,16
	     shl ecx,16
	     add ebx,2
	     add ecx,8
	     mov edx,0xff6c58
	     mcall
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
;**********************************************************
;----------------------------------------------------------
  value:
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
length_number	dd 0