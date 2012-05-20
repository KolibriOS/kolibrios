
; flat assembler core
; Copyright (c) 1999-2012, Tomasz Grysztar.
; All rights reserved.

calculate_expression:
	mov	[current_offset],edi
	mov	[value_undefined],0
	cmp	byte [esi],0
	je	get_string_value
	cmp	byte [esi],'.'
	je	convert_fp
      calculation_loop:
	lods	byte [esi]
	cmp	al,1
	je	get_byte_number
	cmp	al,2
	je	get_word_number
	cmp	al,4
	je	get_dword_number
	cmp	al,8
	je	get_qword_number
	cmp	al,0Fh
	je	value_out_of_range
	cmp	al,10h
	je	get_register
	cmp	al,11h
	je	get_label
	cmp	al,')'
	je	expression_calculated
	cmp	al,']'
	je	expression_calculated
	cmp	al,'!'
	je	invalid_expression
	sub	edi,14h
	mov	ebx,edi
	sub	ebx,14h
	cmp	al,0E0h
	je	calculate_rva
	cmp	al,0E1h
	je	calculate_plt
	cmp	al,0D0h
	je	calculate_not
	cmp	al,083h
	je	calculate_neg
	mov	dx,[ebx+8]
	or	dx,[edi+8]
	cmp	al,80h
	je	calculate_add
	cmp	al,81h
	je	calculate_sub
	mov	ah,[ebx+12]
	or	ah,[edi+12]
	jz	absolute_values_calculation
	cmp	[error_line],0
	jne	absolute_values_calculation
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      absolute_values_calculation:
	cmp	al,90h
	je	calculate_mul
	cmp	al,91h
	je	calculate_div
	or	dx,dx
	jnz	invalid_expression
	cmp	al,0A0h
	je	calculate_mod
	cmp	al,0B0h
	je	calculate_and
	cmp	al,0B1h
	je	calculate_or
	cmp	al,0B2h
	je	calculate_xor
	cmp	al,0C0h
	je	calculate_shl
	cmp	al,0C1h
	je	calculate_shr
	jmp	invalid_expression
      expression_calculated:
	sub	edi,14h
	cmp	[value_undefined],0
	je	expression_value_ok
	xor	eax,eax
	mov	[edi],eax
	mov	[edi+4],eax
	mov	[edi+12],al
      expression_value_ok:
	ret
      get_byte_number:
	xor	eax,eax
	lods	byte [esi]
	stos	dword [edi]
	xor	al,al
	stos	dword [edi]
      got_number:
	mov	word [edi-8+8],0
	mov	byte [edi-8+12],0
	mov	dword [edi-8+16],0
	add	edi,0Ch
	jmp	calculation_loop
      get_word_number:
	xor	eax,eax
	lods	word [esi]
	stos	dword [edi]
	xor	ax,ax
	stos	dword [edi]
	jmp	got_number
      get_dword_number:
	movs	dword [edi],[esi]
	xor	eax,eax
	stos	dword [edi]
	jmp	got_number
      get_qword_number:
	movs	dword [edi],[esi]
	movs	dword [edi],[esi]
	jmp	got_number
      get_register:
	mov	byte [edi+9],0
	mov	byte [edi+12],0
	lods	byte [esi]
	mov	[edi+8],al
	mov	byte [edi+10],1
	xor	eax,eax
	mov	[edi+16],eax
	stos	dword [edi]
	stos	dword [edi]
	add	edi,0Ch
	jmp	calculation_loop
      get_label:
	xor	eax,eax
	mov	[edi+8],eax
	mov	[edi+12],al
	mov	[edi+20],eax
	lods	dword [esi]
	cmp	eax,0Fh
	jb	predefined_label
	je	reserved_word_used_as_symbol
	mov	ebx,eax
	mov	ax,[current_pass]
	mov	[ebx+18],ax
	or	byte [ebx+8],8
	test	byte [ebx+8],1
	jz	label_undefined
	cmp	ax,[ebx+16]
	je	label_defined
	test	byte [ebx+8],4
	jnz	label_out_of_scope
	test	byte [ebx+9],1
	jz	label_defined
	mov	eax,[ebx]
	sub	eax,dword [adjustment]
	stos	dword [edi]
	mov	eax,[ebx+4]
	sbb	eax,dword [adjustment+4]
	stos	dword [edi]
	mov	eax,dword [adjustment]
	or	eax,dword [adjustment+4]
	jz	got_label
	or	[next_pass_needed],-1
	jmp	got_label
      label_defined:
	mov	eax,[ebx]
	stos	dword [edi]
	mov	eax,[ebx+4]
	stos	dword [edi]
      got_label:
	mov	al,[ebx+11]
	mov	[edi-8+12],al
	mov	eax,[ebx+12]
	mov	[edi-8+8],eax
	mov	eax,[ebx+20]
	mov	[edi-8+16],eax
	add	edi,0Ch
	mov	al,[ebx+10]
	or	al,al
	jz	calculation_loop
	cmp	[size_override],-1
	je	calculation_loop
	cmp	[size_override],0
	je	check_size
	cmp	[operand_size],0
	jne	calculation_loop
	mov	[operand_size],al
	jmp	calculation_loop
      check_size:
	xchg	[operand_size],al
	or	al,al
	jz	calculation_loop
	cmp	al,[operand_size]
	jne	operand_sizes_do_not_match
	jmp	calculation_loop
      current_offset_label:
	mov	eax,[current_offset]
      make_current_offset_label:
	xor	edx,edx
	sub	eax,dword [org_origin]
	sbb	edx,dword [org_origin+4]
	stos	dword [edi]
	mov	eax,edx
	stos	dword [edi]
	mov	eax,[org_registers]
	stos	dword [edi]
	mov	al,[labels_type]
	mov	[edi-12+12],al
	mov	eax,[org_symbol]
	mov	[edi-12+16],eax
	add	edi,8
	jmp	calculation_loop
      org_origin_label:
	mov	eax,[org_start]
	jmp	make_current_offset_label
      counter_label:
	mov	eax,[counter]
      make_dword_label_value:
	stos	dword [edi]
	xor	eax,eax
	stos	dword [edi]
	add	edi,0Ch
	jmp	calculation_loop
      timestamp_label:
	call	make_timestamp
      make_qword_label_value:
	stos	dword [edi]
	mov	eax,edx
	stos	dword [edi]
	add	edi,0Ch
	jmp	calculation_loop
      predefined_label:
	or	eax,eax
	jz	current_offset_label
	cmp	eax,1
	je	counter_label
	cmp	eax,2
	je	timestamp_label
	cmp	eax,3
	je	org_origin_label
	mov	edx,invalid_value
	jmp	error_undefined
      label_out_of_scope:
	mov	edx,symbol_out_of_scope
	cmp	[error_line],0
	jne	error_undefined
	mov	[error_info],ebx
	jmp	error_undefined
      label_undefined:
	mov	edx,undefined_symbol
	cmp	[error_line],0
	jne	error_undefined
	mov	[error_info],ebx
      error_undefined:
	cmp	[current_pass],1
	ja	undefined_value
      force_next_pass:
	or	[next_pass_needed],-1
      undefined_value:
	mov	byte [edi+12],0
	or	[value_undefined],-1
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	add	edi,0Ch
	cmp	[error_line],0
	jne	calculation_loop
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],edx
	jmp	calculation_loop
      calculate_add:
	mov	ecx,[ebx+16]
	cmp	byte [edi+12],0
	je	add_values
	mov	ecx,[edi+16]
	cmp	byte [ebx+12],0
	je	add_values
	cmp	[error_line],0
	jne	add_values
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      add_values:
	mov	al,[edi+12]
	or	[ebx+12],al
	mov	[ebx+16],ecx
	mov	eax,[edi]
	add	[ebx],eax
	mov	eax,[edi+4]
	adc	[ebx+4],eax
	or	dx,dx
	jz	calculation_loop
	push	esi
	mov	esi,ebx
	lea	ebx,[edi+10]
	mov	cl,[edi+8]
	call	add_register
	lea	ebx,[edi+11]
	mov	cl,[edi+9]
	call	add_register
	pop	esi
	jmp	calculation_loop
      add_register:
	or	cl,cl
	jz	add_register_done
      add_register_start:
	cmp	[esi+8],cl
	jne	add_in_second_slot
	mov	al,[ebx]
	add	[esi+10],al
	jnz	add_register_done
	mov	byte [esi+8],0
	ret
      add_in_second_slot:
	cmp	[esi+9],cl
	jne	create_in_first_slot
	mov	al,[ebx]
	add	[esi+11],al
	jnz	add_register_done
	mov	byte [esi+9],0
	ret
      create_in_first_slot:
	cmp	byte [esi+8],0
	jne	create_in_second_slot
	mov	[esi+8],cl
	mov	al,[ebx]
	mov	[esi+10],al
	ret
      create_in_second_slot:
	cmp	byte [esi+9],0
	jne	invalid_expression
	mov	[esi+9],cl
	mov	al,[ebx]
	mov	[esi+11],al
      add_register_done:
	ret
      calculate_sub:
	xor	ah,ah
	mov	ah,[ebx+12]
	mov	al,[edi+12]
	or	al,al
	jz	sub_values
	cmp	al,ah
	jne	invalid_sub
	xor	ah,ah
	mov	ecx,[edi+16]
	cmp	ecx,[ebx+16]
	jne	invalid_sub
      sub_values:
	mov	[ebx+12],ah
	mov	eax,[edi]
	sub	[ebx],eax
	mov	eax,[edi+4]
	sbb	[ebx+4],eax
	or	dx,dx
	jz	calculation_loop
	push	esi
	mov	esi,ebx
	lea	ebx,[edi+10]
	mov	cl,[edi+8]
	call	sub_register
	lea	ebx,[edi+11]
	mov	cl,[edi+9]
	call	sub_register
	pop	esi
	jmp	calculation_loop
      invalid_sub:
	cmp	[error_line],0
	jne	sub_values
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
	jmp	sub_values
      sub_register:
	or	cl,cl
	jz	add_register_done
	neg	byte [ebx]
	jmp	add_register_start
      calculate_mul:
	or	dx,dx
	jz	mul_start
	cmp	word [ebx+8],0
	jne	mul_start
	mov	eax,[ebx]
	xchg	eax,[edi]
	mov	[ebx],eax
	mov	eax,[ebx+4]
	xchg	eax,[edi+4]
	mov	[ebx+4],eax
	mov	eax,[ebx+8]
	xchg	eax,[edi+8]
	mov	[ebx+8],eax
	mov	eax,[ebx+12]
	xchg	eax,[edi+12]
	mov	[ebx+12],eax
      mul_start:
	push	esi edx
	mov	esi,ebx
	xor	bl,bl
	bt	dword [esi+4],31
	jnc	mul_first_sign_ok
	not	dword [esi]
	not	dword [esi+4]
	add	dword [esi],1
	adc	dword [esi+4],0
	not	bl
      mul_first_sign_ok:
	bt	dword [edi+4],31
	jnc	mul_second_sign_ok
	not	dword [edi]
	not	dword [edi+4]
	add	dword [edi],1
	adc	dword [edi+4],0
	not	bl
      mul_second_sign_ok:
	cmp	dword [esi+4],0
	jz	mul_numbers
	cmp	dword [edi+4],0
	jnz	value_out_of_range
      mul_numbers:
	mov	eax,[esi+4]
	mul	dword [edi]
	or	edx,edx
	jnz	value_out_of_range
	mov	ecx,eax
	mov	eax,[esi]
	mul	dword [edi+4]
	or	edx,edx
	jnz	value_out_of_range
	add	ecx,eax
	jc	value_out_of_range
	mov	eax,[esi]
	mul	dword [edi]
	add	edx,ecx
	jc	value_out_of_range
	test	edx,1 shl 31
	jnz	value_out_of_range
	mov	[esi],eax
	mov	[esi+4],edx
	or	bl,bl
	jz	mul_ok
	not	dword [esi]
	not	dword [esi+4]
	add	dword [esi],1
	adc	dword [esi+4],0
      mul_ok:
	pop	edx
	or	dx,dx
	jz	mul_calculated
	cmp	word [edi+8],0
	jne	invalid_value
	cmp	byte [esi+8],0
	je	mul_first_register_ok
	mov	al,[edi]
	cbw
	cwde
	cdq
	cmp	edx,[edi+4]
	jne	value_out_of_range
	cmp	eax,[edi]
	jne	value_out_of_range
	imul	byte [esi+10]
	mov	dl,ah
	cbw
	cmp	ah,dl
	jne	value_out_of_range
	mov	[esi+10],al
	or	al,al
	jnz	mul_first_register_ok
	mov	[esi+8],al
      mul_first_register_ok:
	cmp	byte [esi+9],0
	je	mul_calculated
	mov	al,[edi]
	cbw
	cwde
	cdq
	cmp	edx,[edi+4]
	jne	value_out_of_range
	cmp	eax,[edi]
	jne	value_out_of_range
	imul	byte [esi+11]
	mov	dl,ah
	cbw
	cmp	ah,dl
	jne	value_out_of_range
	mov	[esi+11],al
	or	al,al
	jnz	mul_calculated
	mov	[esi+9],al
      mul_calculated:
	pop	esi
	jmp	calculation_loop
      calculate_div:
	push	esi edx
	mov	esi,ebx
	call	div_64
	pop	edx
	or	dx,dx
	jz	div_calculated
	cmp	byte [esi+8],0
	je	div_first_register_ok
	mov	al,[edi]
	cbw
	cwde
	cdq
	cmp	edx,[edi+4]
	jne	value_out_of_range
	cmp	eax,[edi]
	jne	value_out_of_range
	or	al,al
	jz	value_out_of_range
	mov	al,[esi+10]
	cbw
	idiv	byte [edi]
	or	ah,ah
	jnz	invalid_use_of_symbol
	mov	[esi+10],al
      div_first_register_ok:
	cmp	byte [esi+9],0
	je	div_calculated
	mov	al,[edi]
	cbw
	cwde
	cdq
	cmp	edx,[edi+4]
	jne	value_out_of_range
	cmp	eax,[edi]
	jne	value_out_of_range
	or	al,al
	jz	value_out_of_range
	mov	al,[esi+11]
	cbw
	idiv	byte [edi]
	or	ah,ah
	jnz	invalid_use_of_symbol
	mov	[esi+11],al
      div_calculated:
	pop	esi
	jmp	calculation_loop
      calculate_mod:
	push	esi
	mov	esi,ebx
	call	div_64
	mov	[esi],eax
	mov	[esi+4],edx
	pop	esi
	jmp	calculation_loop
      calculate_and:
	mov	eax,[edi]
	and	[ebx],eax
	mov	eax,[edi+4]
	and	[ebx+4],eax
	jmp	calculation_loop
      calculate_or:
	mov	eax,[edi]
	or	[ebx],eax
	mov	eax,[edi+4]
	or	[ebx+4],eax
	jmp	calculation_loop
      calculate_xor:
	mov	eax,[edi]
	xor	[ebx],eax
	mov	edx,[edi+4]
	xor	[ebx+4],edx
	cmp	[value_size],1
	je	xor_byte
	cmp	[value_size],2
	je	xor_word
	cmp	[value_size],4
	je	xor_dword
	cmp	[value_size],6
	je	xor_pword
      xor_calculated:
	jmp	calculation_loop
      prepare_xor_check:
	cmp	edx,-1
	je	xor_check_ready
	xor	edx,[ebx+4]
	xor	eax,[ebx]
	cmp	edx,-1
      xor_check_ready:
	ret
      xor_byte:
	call	prepare_xor_check
	jne	xor_calculated
	xor	edx,[ebx+4]
	jnz	xor_calculated
	cmp	eax,0FFFFFF80h
	jb	xor_calculated
	xor	eax,[ebx]
	cmp	eax,0FFh
	ja	xor_calculated
	mov	[ebx+4],edx
	and	dword [ebx],0FFh
	jmp	xor_calculated
      xor_word:
	call	prepare_xor_check
	jne	xor_calculated
	xor	edx,[ebx+4]
	jnz	xor_calculated
	cmp	eax,0FFFF8000h
	jb	xor_calculated
	xor	eax,[ebx]
	cmp	eax,0FFFFh
	ja	xor_calculated
	mov	[ebx+4],edx
	and	dword [ebx],0FFFFh
	jmp	xor_calculated
      xor_dword:
	call	prepare_xor_check
	jne	xor_calculated
	xor	edx,[ebx+4]
	jnz	xor_calculated
	cmp	eax,80000000h
	jb	xor_calculated
	mov	[ebx+4],edx
	jmp	xor_calculated
      xor_pword:
	cmp	edx,0FFFF8000h
	jae	xor_pword_check
	xor	edx,[ebx+4]
	xor	eax,[ebx]
	cmp	edx,0FFFF8000h
	jb	xor_calculated
      xor_pword_check:
	xor	edx,[ebx+4]
	cmp	edx,0FFFFh
	ja	xor_calculated
	and	dword [ebx+4],0FFFFh
	jmp	xor_calculated
      shr_negative:
	not	dword [edi]
	not	dword [edi+4]
	add	dword [edi],1
	adc	dword [edi+4],0
      calculate_shl:
	mov	eax,dword [edi+4]
	bt	eax,31
	jc	shl_negative
	or	eax,eax
	jnz	zero_value
	mov	ecx,[edi]
	cmp	ecx,64
	jae	zero_value
	mov	edx,[ebx+4]
	mov	eax,[ebx]
	cmp	ecx,32
	jae	shl_high
	shld	edx,eax,cl
	shl	eax,cl
	mov	[ebx],eax
	mov	[ebx+4],edx
	jmp	calculation_loop
      shl_high:
	sub	cl,32
	shl	eax,cl
	mov	[ebx+4],eax
	mov	dword [ebx],0
	jmp	calculation_loop
      shl_negative:
	not	dword [edi]
	not	dword [edi+4]
	add	dword [edi],1
	adc	dword [edi+4],0
      calculate_shr:
	mov	eax,dword [edi+4]
	bt	eax,31
	jc	shr_negative
	or	eax,eax
	jnz	zero_value
	mov	ecx,[edi]
	cmp	ecx,64
	jae	zero_value
	mov	edx,[ebx+4]
	mov	eax,[ebx]
	cmp	ecx,32
	jae	shr_high
	cmp	[value_size],1
	je	shr_byte
	cmp	[value_size],2
	je	shr_word
	cmp	[value_size],4
	je	shr_dword
	cmp	[value_size],6
	je	shr_pword
      shr_regular:
	shrd	eax,edx,cl
	shr	edx,cl
	mov	[ebx],eax
	mov	[ebx+4],edx
	jmp	calculation_loop
      shr_byte:
	cmp	edx,-1
	jne	shr_regular
	cmp	eax,0FFFFFF80h
	jb	shr_regular
	and	eax,0FFh
	xor	edx,edx
	jmp	shr_regular
      shr_word:
	cmp	edx,-1
	jne	shr_regular
	cmp	eax,0FFFF8000h
	jb	shr_regular
	and	eax,0FFFFh
	xor	edx,edx
	jmp	shr_regular
      shr_dword:
	cmp	edx,-1
	jne	shr_regular
	cmp	eax,80000000h
	jb	shr_regular
	xor	edx,edx
	jmp	shr_regular
      shr_pword:
	cmp	edx,0FFFF8000h
	jb	shr_regular
	and	edx,0FFFFh
	jmp	shr_regular
      shr_high:
	sub	cl,32
	cmp	[value_size],1
	je	shr_byte_32plus
	cmp	[value_size],2
	je	shr_word_32plus
	cmp	[value_size],4
	je	shr_dword_32plus
	cmp	[value_size],6
	je	shr_pword_32plus
      shr_32plus:
	shr	edx,cl
	mov	[ebx],edx
	mov	dword [ebx+4],0
	jmp	calculation_loop
      shr_byte_32plus:
	cmp	edx,-1
	jne	shr_32plus
	cmp	eax,-80h
	jb	shr_32plus
	jmp	zero_value
      shr_word_32plus:
	cmp	edx,-1
	jne	shr_32plus
	cmp	eax,-8000h
	jb	shr_32plus
	jmp	zero_value
      shr_dword_32plus:
	cmp	edx,-1
	jne	shr_32plus
	cmp	eax,-80000000h
	jb	shr_32plus
	jmp	zero_value
      shr_pword_32plus:
	cmp	edx,-8000h
	jb	shr_32plus
	and	edx,0FFFFh
	jmp	shr_32plus
      zero_value:
	mov	dword [ebx],0
	mov	dword [ebx+4],0
	jmp	calculation_loop
      calculate_not:
	cmp	word [edi+8],0
	jne	invalid_expression
	cmp	byte [edi+12],0
	je	not_ok
	cmp	[error_line],0
	jne	not_ok
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      not_ok:
	cmp	[value_size],1
	je	not_byte
	cmp	[value_size],2
	je	not_word
	cmp	[value_size],4
	je	not_dword
	cmp	[value_size],6
	je	not_pword
      not_qword:
	not	dword [edi]
	not	dword [edi+4]
	add	edi,14h
	jmp	calculation_loop
      not_byte:
	cmp	dword [edi+4],0
	jne	not_qword
	cmp	word [edi+2],0
	jne	not_qword
	cmp	byte [edi+1],0
	jne	not_qword
	not	byte [edi]
	add	edi,14h
	jmp	calculation_loop
      not_word:
	cmp	dword [edi+4],0
	jne	not_qword
	cmp	word [edi+2],0
	jne	not_qword
	not	word [edi]
	add	edi,14h
	jmp	calculation_loop
      not_dword:
	cmp	dword [edi+4],0
	jne	not_qword
	not	dword [edi]
	add	edi,14h
	jmp	calculation_loop
      not_pword:
	cmp	word [edi+6],0
	jne	not_qword
	not	dword [edi]
	not	word [edi+4]
	add	edi,14h
	jmp	calculation_loop
      calculate_neg:
	cmp	word [edi+8],0
	jne	invalid_expression
	cmp	byte [edi+12],0
	je	neg_ok
	cmp	[error_line],0
	jne	neg_ok
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      neg_ok:
	mov	eax,[edi]
	mov	edx,[edi+4]
	mov	dword [edi],0
	mov	dword [edi+4],0
	sub	[edi],eax
	sbb	[edi+4],edx
	add	edi,14h
	jmp	calculation_loop
      calculate_rva:
	cmp	word [edi+8],0
	jne	invalid_expression
	cmp	[output_format],5
	je	calculate_gotoff
	cmp	[output_format],4
	je	calculate_coff_rva
	cmp	[output_format],3
	jne	invalid_expression
	test	[format_flags],8
	jnz	pe64_rva
	mov	al,2
	bt	[resolver_flags],0
	jc	rva_type_ok
	xor	al,al
      rva_type_ok:
	cmp	byte [edi+12],al
	je	rva_ok
	cmp	[error_line],0
	jne	rva_ok
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      rva_ok:
	mov	byte [edi+12],0
	mov	eax,[code_start]
	mov	eax,[eax+34h]
	cdq
	sub	[edi],eax
	sbb	[edi+4],edx
	add	edi,14h
	jmp	calculation_loop
      pe64_rva:
	mov	al,4
	bt	[resolver_flags],0
	jc	pe64_rva_type_ok
	xor	al,al
      pe64_rva_type_ok:
	cmp	byte [edi+12],al
	je	pe64_rva_ok
	cmp	[error_line],0
	jne	pe64_rva_ok
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      pe64_rva_ok:
	mov	byte [edi+12],0
	mov	eax,[code_start]
	mov	edx,[eax+34h]
	mov	eax,[eax+30h]
	sub	[edi],eax
	sbb	[edi+4],edx
	add	edi,14h
	jmp	calculation_loop
      calculate_gotoff:
	test	[format_flags],8+1
	jnz	invalid_expression
      calculate_coff_rva:
	mov	dl,5
	cmp	byte [edi+12],2
	je	change_value_type
      incorrect_change_of_value_type:
	cmp	[error_line],0
	jne	change_value_type
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      change_value_type:
	mov	byte [edi+12],dl
	add	edi,14h
	jmp	calculation_loop
      calculate_plt:
	cmp	word [edi+8],0
	jne	invalid_expression
	cmp	[output_format],5
	jne	invalid_expression
	test	[format_flags],1
	jnz	invalid_expression
	mov	dl,6
	mov	dh,2
	test	[format_flags],8
	jz	check_value_for_plt
	mov	dh,4
      check_value_for_plt:
	mov	eax,[edi]
	or	eax,[edi+4]
	jnz	incorrect_change_of_value_type
	cmp	byte [edi+12],dh
	jne	incorrect_change_of_value_type
	mov	eax,[edi+16]
	cmp	byte [eax],80h
	jne	incorrect_change_of_value_type
	jmp	change_value_type
      div_64:
	xor	ebx,ebx
	cmp	dword [edi],0
	jne	divider_ok
	cmp	dword [edi+4],0
	jne	divider_ok
	cmp	[next_pass_needed],0
	je	value_out_of_range
	jmp	div_done
      divider_ok:
	bt	dword [esi+4],31
	jnc	div_first_sign_ok
	not	dword [esi]
	not	dword [esi+4]
	add	dword [esi],1
	adc	dword [esi+4],0
	not	bx
      div_first_sign_ok:
	bt	dword [edi+4],31
	jnc	div_second_sign_ok
	not	dword [edi]
	not	dword [edi+4]
	add	dword [edi],1
	adc	dword [edi+4],0
	not	bl
      div_second_sign_ok:
	cmp	dword [edi+4],0
	jne	div_high
	mov	ecx,[edi]
	mov	eax,[esi+4]
	xor	edx,edx
	div	ecx
	mov	[esi+4],eax
	mov	eax,[esi]
	div	ecx
	mov	[esi],eax
	mov	eax,edx
	xor	edx,edx
	jmp	div_done
      div_high:
	mov	eax,[esi+4]
	xor	edx,edx
	div	dword [edi+4]
	mov	ebx,[esi]
	mov	[esi],eax
	mov	dword [esi+4],0
	mov	ecx,edx
	mul	dword [edi]
      div_high_loop:
	cmp	ecx,edx
	ja	div_high_done
	jb	div_high_large_correction
	cmp	ebx,eax
	jae	div_high_done
      div_high_correction:
	dec	dword [esi]
	sub	eax,[edi]
	sbb	edx,[edi+4]
	jnc	div_high_loop
      div_high_done:
	sub	ebx,eax
	sbb	ecx,edx
	mov	edx,ecx
	mov	eax,ebx
	ret
      div_high_large_correction:
	push	eax edx
	mov	eax,edx
	sub	eax,ecx
	xor	edx,edx
	div	dword [edi+4]
	shr	eax,1
	jz	div_high_small_correction
	sub	[esi],eax
	push	eax
	mul	dword [edi+4]
	sub	dword [esp+4],eax
	pop	eax
	mul	dword [edi]
	sub	dword [esp+4],eax
	sbb	dword [esp],edx
	pop	edx eax
	jmp	div_high_loop
      div_high_small_correction:
	pop	edx eax
	jmp	div_high_correction
      div_done:
	or	bh,bh
	jz	remainder_ok
	not	eax
	not	edx
	add	eax,1
	adc	edx,0
      remainder_ok:
	or	bl,bl
	jz	div_ok
	not	dword [esi]
	not	dword [esi+4]
	add	dword [esi],1
	adc	dword [esi+4],0
      div_ok:
	ret
      convert_fp:
	inc	esi
	mov	word [edi+8],0
	mov	byte [edi+12],0
	mov	al,[value_size]
	cmp	al,2
	je	convert_fp_word
	cmp	al,4
	je	convert_fp_dword
	cmp	al,8
	je	convert_fp_qword
	jmp	invalid_value
      convert_fp_word:
	xor	eax,eax
	cmp	word [esi+8],8000h
	je	fp_word_store
	mov	bx,[esi+8]
	mov	ax,[esi+6]
	shl	ax,1
	shr	ax,6
	jnc	fp_word_ok
	inc	ax
	bt	ax,10
	jnc	fp_word_ok
	and	ax,1 shl 10 - 1
	inc	bx
	shr	ax,1
      fp_word_ok:
	add	bx,0Fh
	cmp	bx,01Fh
	jge	value_out_of_range
	cmp	bx,0
	jg	fp_word_exp_ok
	or	ax,1 shl 10
	mov	cx,bx
	neg	cx
	inc	cx
	cmp	cx,10
	ja	value_out_of_range
	xor	bx,bx
	shr	ax,cl
	jnc	fp_word_exp_ok
	inc	ax
	test	ax,1 shl 10
	jz	fp_word_exp_ok
	and	ax,1 shl 10 - 1
	inc	bx
      fp_word_exp_ok:
	shl	bx,10
	or	ax,bx
      fp_word_store:
	mov	bl,[esi+11]
	shl	bx,15
	or	ax,bx
	mov	[edi],eax
	xor	eax,eax
	mov	[edi+4],eax
	add	esi,13
	ret
      convert_fp_dword:
	xor	eax,eax
	cmp	word [esi+8],8000h
	je	fp_dword_store
	mov	bx,[esi+8]
	mov	eax,[esi+4]
	shl	eax,1
	shr	eax,9
	jnc	fp_dword_ok
	inc	eax
	bt	eax,23
	jnc	fp_dword_ok
	and	eax,1 shl 23 - 1
	inc	bx
	shr	eax,1
      fp_dword_ok:
	add	bx,7Fh
	cmp	bx,0FFh
	jge	value_out_of_range
	cmp	bx,0
	jg	fp_dword_exp_ok
	or	eax,1 shl 23
	mov	cx,bx
	neg	cx
	inc	cx
	cmp	cx,23
	ja	value_out_of_range
	xor	bx,bx
	shr	eax,cl
	jnc	fp_dword_exp_ok
	inc	eax
	test	eax,1 shl 23
	jz	fp_dword_exp_ok
	and	eax,1 shl 23 - 1
	inc	bx
      fp_dword_exp_ok:
	shl	ebx,23
	or	eax,ebx
      fp_dword_store:
	mov	bl,[esi+11]
	shl	ebx,31
	or	eax,ebx
	mov	[edi],eax
	xor	eax,eax
	mov	[edi+4],eax
	add	esi,13
	ret
      convert_fp_qword:
	xor	eax,eax
	xor	edx,edx
	cmp	word [esi+8],8000h
	je	fp_qword_store
	mov	bx,[esi+8]
	mov	eax,[esi]
	mov	edx,[esi+4]
	add	eax,eax
	adc	edx,edx
	mov	ecx,edx
	shr	edx,12
	shrd	eax,ecx,12
	jnc	fp_qword_ok
	add	eax,1
	adc	edx,0
	bt	edx,20
	jnc	fp_qword_ok
	and	edx,1 shl 20 - 1
	inc	bx
	shr	edx,1
	rcr	eax,1
      fp_qword_ok:
	add	bx,3FFh
	cmp	bx,7FFh
	jge	value_out_of_range
	cmp	bx,0
	jg	fp_qword_exp_ok
	or	edx,1 shl 20
	mov	cx,bx
	neg	cx
	inc	cx
	cmp	cx,52
	ja	value_out_of_range
	cmp	cx,32
	jbe	fp_qword_small_shift
	sub	cx,32
	mov	eax,edx
	xor	edx,edx
	shr	eax,cl
	jmp	fp_qword_shift_done
      fp_qword_small_shift:
	mov	ebx,edx
	shr	edx,cl
	shrd	eax,ebx,cl
      fp_qword_shift_done:
	mov	bx,0
	jnc	fp_qword_exp_ok
	add	eax,1
	adc	edx,0
	test	edx,1 shl 20
	jz	fp_qword_exp_ok
	and	edx,1 shl 20 - 1
	inc	bx
      fp_qword_exp_ok:
	shl	ebx,20
	or	edx,ebx
      fp_qword_store:
	mov	bl,[esi+11]
	shl	ebx,31
	or	edx,ebx
	mov	[edi],eax
	mov	[edi+4],edx
	add	esi,13
	ret
      get_string_value:
	inc	esi
	lods	dword [esi]
	mov	ecx,eax
	cmp	ecx,8
	ja	value_out_of_range
	mov	edx,edi
	xor	eax,eax
	stos	dword [edi]
	stos	dword [edi]
	mov	edi,edx
	rep	movs byte [edi],[esi]
	mov	edi,edx
	inc	esi
	mov	word [edi+8],0
	mov	byte [edi+12],0
	ret

get_byte_value:
	mov	[value_size],1
	mov	[size_override],-1
	call	calculate_expression
	mov	eax,[edi+16]
	mov	[symbol_identifier],eax
	mov	[value_type],0
	cmp	word [edi+8],0
	jne	invalid_value
	cmp	byte [edi+12],0
	je	check_byte_value
	cmp	[error_line],0
	jne	check_byte_value
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      check_byte_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	or	edx,edx
	jz	byte_positive
	cmp	edx,-1
	jne	range_exceeded
	cmp	eax,-80h
	jb	range_exceeded
	ret
      byte_positive:
	cmp	eax,100h
	jae	range_exceeded
      return_byte_value:
	ret
      range_exceeded:
	xor	eax,eax
	xor	edx,edx
	cmp	[error_line],0
	jne	return_byte_value
	mov	ecx,[current_line]
	mov	[error_line],ecx
	mov	[error],value_out_of_range
	ret
get_word_value:
	mov	[value_size],2
	mov	[size_override],-1
	call	calculate_expression
	cmp	word [edi+8],0
	jne	invalid_value
	mov	eax,[edi+16]
	mov	[symbol_identifier],eax
	mov	al,[edi+12]
	mov	[value_type],al
	cmp	al,2
	jb	check_word_value
	cmp	[error_line],0
	jne	check_word_value
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      check_word_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	or	edx,edx
	jz	word_positive
	cmp	edx,-1
	jne	range_exceeded
	cmp	eax,-8000h
	jb	range_exceeded
	ret
      word_positive:
	cmp	eax,10000h
	jae	range_exceeded
	ret
get_dword_value:
	mov	[value_size],4
	mov	[size_override],-1
	call	calculate_expression
	cmp	word [edi+8],0
	jne	invalid_value
	mov	eax,[edi+16]
	mov	[symbol_identifier],eax
	mov	al,[edi+12]
	mov	[value_type],al
	cmp	al,4
	jne	check_dword_value
	mov	[value_type],2
	mov	eax,[edi]
	cdq
	cmp	edx,[edi+4]
	jne	range_exceeded
	ret
      check_dword_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	or	edx,edx
	jz	dword_positive
	cmp	edx,-1
	jne	range_exceeded
	bt	eax,31
	jnc	range_exceeded
      dword_positive:
	ret
get_pword_value:
	mov	[value_size],6
	mov	[size_override],-1
	call	calculate_expression
	mov	eax,[edi+16]
	mov	[symbol_identifier],eax
	cmp	word [edi+8],0
	jne	invalid_value
	mov	al,[edi+12]
	mov	[value_type],al
	cmp	al,4
	jne	check_pword_value
	cmp	[error_line],0
	jne	check_pword_value
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      check_pword_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	cmp	edx,10000h
	jge	range_exceeded
	cmp	edx,-8000h
	jl	range_exceeded
	ret
get_qword_value:
	mov	[value_size],8
	mov	[size_override],-1
	call	calculate_expression
	cmp	word [edi+8],0
	jne	invalid_value
	mov	eax,[edi+16]
	mov	[symbol_identifier],eax
	mov	al,[edi+12]
	mov	[value_type],al
      check_qword_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	ret
get_count_value:
	mov	[value_size],8
	mov	[size_override],-1
	call	calculate_expression
	cmp	word [edi+8],0
	jne	invalid_value
	mov	al,[edi+12]
	or	al,al
	jz	check_count_value
	cmp	[error_line],0
	jne	check_count_value
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_use_of_symbol
      check_count_value:
	mov	eax,[edi]
	mov	edx,[edi+4]
	or	edx,edx
	jnz	invalid_count_value
	ret
      invalid_count_value:
	cmp	[error_line],0
	jne	zero_count
	mov	eax,[current_line]
	mov	[error_line],eax
	mov	[error],invalid_value
      zero_count:
	xor	eax,eax
	ret
get_value:
	mov	[operand_size],0
	lods	byte [esi]
	call	get_size_operator
	cmp	al,'('
	jne	invalid_value
	mov	al,[operand_size]
	cmp	al,1
	je	value_byte
	cmp	al,2
	je	value_word
	cmp	al,4
	je	value_dword
	cmp	al,6
	je	value_pword
	cmp	al,8
	je	value_qword
	or	al,al
	jnz	invalid_value
      value_qword:
	call	get_qword_value
	ret
      value_pword:
	call	get_pword_value
	movzx	edx,dx
	ret
      value_dword:
	call	get_dword_value
	xor	edx,edx
	ret
      value_word:
	call	get_word_value
	xor	edx,edx
	movzx	eax,ax
	ret
      value_byte:
	call	get_byte_value
	xor	edx,edx
	movzx	eax,al
	ret
get_address_word_value:
	mov	[address_size],2
	mov	[value_size],2
	jmp	calculate_address
get_address_dword_value:
	mov	[address_size],4
	mov	[value_size],4
	jmp	calculate_address
get_address_qword_value:
	mov	[address_size],8
	mov	[value_size],8
	jmp	calculate_address
get_address_value:
	mov	[address_size],0
	mov	[value_size],8
      calculate_address:
	cmp	byte [esi],'.'
	je	invalid_address
	call	calculate_expression
	mov	eax,[edi+16]
	mov	[address_symbol],eax
	mov	al,[edi+12]
	mov	[value_type],al
	cmp	al,6
	je	special_address_type_32bit
	cmp	al,5
	je	special_address_type_32bit
	ja	invalid_use_of_symbol
	test	al,1
	jnz	invalid_use_of_symbol
	or	al,al
	jz	address_size_ok
	shl	al,5
	jmp	address_symbol_ok
      special_address_type_32bit:
	mov	al,40h
      address_symbol_ok:
	mov	ah,[address_size]
	or	[address_size],al
	shr	al,4
	or	ah,ah
	jz	address_size_ok
	cmp	al,ah
	je	address_size_ok
	cmp	ax,0804h
	jne	address_sizes_do_not_agree
	cmp	[value_type],2
	ja	value_type_correction_ok
	mov	[value_type],2
      value_type_correction_ok:
	mov	eax,[edi]
	cdq
	cmp	edx,[edi+4]
	je	address_size_ok
	cmp	[error_line],0
	jne	address_size_ok
	mov	ecx,[current_line]
	mov	[error_line],ecx
	mov	[error],value_out_of_range
      address_size_ok:
	xor	ebx,ebx
	xor	ecx,ecx
	mov	cl,[value_type]
	shl	ecx,16
	mov	ch,[address_size]
	cmp	word [edi+8],0
	je	check_immediate_address
	mov	al,[edi+8]
	mov	dl,[edi+10]
	call	get_address_register
	mov	al,[edi+9]
	mov	dl,[edi+11]
	call	get_address_register
	mov	ax,bx
	shr	ah,4
	shr	al,4
	or	bh,bh
	jz	check_address_registers
	or	bl,bl
	jz	check_address_registers
	cmp	al,ah
	jne	invalid_address
      check_address_registers:
	or	al,ah
	mov	ah,[address_size]
	and	ah,0Fh
	jz	address_registers_sizes_ok
	cmp	al,ah
	jne	address_sizes_do_not_match
      address_registers_sizes_ok:
	cmp	al,4
	je	sib_allowed
	cmp	al,8
	je	sib_allowed
	cmp	al,0Fh
	je	check_ip_relative_address
	or	cl,cl
	jz	check_word_value
	cmp	cl,1
	je	check_word_value
	jmp	invalid_address
      address_sizes_do_not_match:
	cmp	al,0Fh
	jne	invalid_address
	mov	al,bh
	and	al,0Fh
	cmp	al,ah
	jne	invalid_address
      check_ip_relative_address:
	cmp	bh,0F4h
	je	check_dword_value
	cmp	bh,0F8h
	jne	invalid_address
	mov	eax,[edi]
	cdq
	cmp	edx,[edi+4]
	jne	range_exceeded
	ret
      get_address_register:
	or	al,al
	jz	address_register_ok
	cmp	dl,1
	jne	scaled_register
	or	bh,bh
	jnz	scaled_register
	mov	bh,al
      address_register_ok:
	ret
      scaled_register:
	or	bl,bl
	jnz	invalid_address
	mov	bl,al
	mov	cl,dl
	jmp	address_register_ok
      sib_allowed:
	or	bh,bh
	jnz	check_index_with_base
	cmp	cl,3
	je	special_index_scale
	cmp	cl,5
	je	special_index_scale
	cmp	cl,9
	je	special_index_scale
	cmp	cl,2
	jne	check_index_scale
	cmp	bl,45h
	jne	special_index_scale
	cmp	[code_type],64
	je	special_index_scale
	cmp	[segment_register],4
	jne	special_index_scale
	cmp	[value_type],0
	jne	check_index_scale
	mov	al,[edi]
	cbw
	cwde
	cmp	eax,[edi]
	jne	check_index_scale
	cdq
	cmp	edx,[edi+4]
	jne	check_immediate_address
      special_index_scale:
	mov	bh,bl
	dec	cl
      check_immediate_address:
	mov	al,[address_size]
	and	al,0Fh
	cmp	al,2
	je	check_word_value
	cmp	al,4
	je	check_dword_value
	cmp	al,8
	je	check_qword_value
	or	al,al
	jnz	invalid_value
	cmp	[code_type],64
	jne	check_dword_value
	jmp	check_qword_value
      check_index_with_base:
	cmp	cl,1
	jne	check_index_scale
	cmp	bl,44h
	je	swap_base_with_index
	cmp	bl,84h
	je	swap_base_with_index
	cmp	[code_type],64
	je	check_for_rbp_base
	cmp	bl,45h
	jne	check_for_ebp_base
	cmp	[segment_register],3
	je	swap_base_with_index
	jmp	check_immediate_address
      check_for_ebp_base:
	cmp	bh,45h
	jne	check_immediate_address
	cmp	[segment_register],4
	jne	check_immediate_address
      swap_base_with_index:
	xchg	bl,bh
	jmp	check_immediate_address
      check_for_rbp_base:
	cmp	bh,45h
	je	swap_base_with_index
	cmp	bh,85h
	je	swap_base_with_index
	jmp	check_immediate_address
      check_index_scale:
	test	cl,not 1111b
	jnz	invalid_address
	mov	al,cl
	dec	al
	and	al,cl
	jz	check_immediate_address
	jmp	invalid_address
calculate_relative_offset:
	cmp	[value_undefined],0
	jne	relative_offset_ok
	test	bh,bh
	setne	ch
	cmp	bx,word [org_registers]
	je	origin_registers_ok
	xchg	bh,bl
	xchg	ch,cl
	cmp	bx,word [org_registers]
	jne	invalid_value
      origin_registers_ok:
	cmp	cx,word [org_registers+2]
	jne	invalid_value
	add	eax,dword [org_origin]
	adc	edx,dword [org_origin+4]
	sub	eax,edi
	sbb	edx,0
	mov	bl,[value_type]
	or	bl,bl
	je	relative_offset_ok
	test	bl,1
	jnz	invalid_use_of_symbol
	mov	ecx,[address_symbol]
	mov	[symbol_identifier],ecx
	cmp	bl,6
	je	plt_relative_offset
	cmp	bl,[labels_type]
	jne	invalid_use_of_symbol
	mov	[value_type],0
	cmp	ecx,[org_symbol]
	je	relative_offset_ok
	mov	[value_type],3
      relative_offset_ok:
	ret
      plt_relative_offset:
	mov	[value_type],7
	ret

calculate_logical_expression:
	xor	al,al
  calculate_embedded_logical_expression:
	mov	[logical_value_wrapping],al
	call	get_logical_value
      logical_loop:
	cmp	byte [esi],'|'
	je	logical_or
	cmp	byte [esi],'&'
	je	logical_and
	ret
      logical_or:
	inc	esi
	or	al,al
	jnz	logical_value_already_determined
	push	eax
	call	get_logical_value
	pop	ebx
	or	al,bl
	jmp	logical_loop
      logical_and:
	inc	esi
	or	al,al
	jz	logical_value_already_determined
	push	eax
	call	get_logical_value
	pop	ebx
	and	al,bl
	jmp	logical_loop
      logical_value_already_determined:
	push	eax
	call	skip_logical_value
	jc	invalid_expression
	pop	eax
	jmp	logical_loop
  get_logical_value:
	xor	al,al
      check_for_negation:
	cmp	byte [esi],'~'
	jne	negation_ok
	inc	esi
	xor	al,-1
	jmp	check_for_negation
      negation_ok:
	push	eax
	mov	al,[esi]
	cmp	al,'{'
	je	logical_expression
	cmp	al,0FFh
	je	invalid_expression
	cmp	al,88h
	je	check_for_defined
	cmp	al,89h
	je	check_for_used
	cmp	al,'0'
	je	given_false
	cmp	al,'1'
	je	given_true
	call	get_value
	mov	bl,[value_type]
	push	eax edx ebx
	mov	al,[esi]
	or	al,al
	jz	logical_number
	cmp	al,0Fh
	je	logical_number
	cmp	al,'}'
	je	logical_number
	cmp	al,'&'
	je	logical_number
	cmp	al,'|'
	je	logical_number
	inc	esi
	mov	[compare_type],al
	call	get_value
	pop	ebx
	cmp	[next_pass_needed],0
	jne	values_ok
	cmp	bl,[value_type]
	jne	invalid_use_of_symbol
      values_ok:
	pop	ecx ebx
	cmp	[compare_type],'='
	je	check_equal
	cmp	[compare_type],'>'
	je	check_greater
	cmp	[compare_type],'<'
	je	check_less
	cmp	[compare_type],0F1h
	je	check_not_equal
	cmp	[compare_type],0F2h
	je	check_not_less
	cmp	[compare_type],0F3h
	je	check_not_greater
	jmp	invalid_expression
      check_equal:
	cmp	eax,ebx
	jne	return_false
	cmp	edx,ecx
	jne	return_false
	jmp	return_true
      check_greater:
	cmp	edx,ecx
	jl	return_true
	jg	return_false
	cmp	eax,ebx
	jb	return_true
	jae	return_false
      check_less:
	cmp	edx,ecx
	jl	return_false
	jg	return_true
	cmp	eax,ebx
	jbe	return_false
	ja	return_true
      check_not_less:
	cmp	edx,ecx
	jl	return_true
	jg	return_false
	cmp	eax,ebx
	jbe	return_true
	ja	return_false
      check_not_greater:
	cmp	edx,ecx
	jl	return_false
	jg	return_true
	cmp	eax,ebx
	jb	return_false
	jae	return_true
      check_not_equal:
	cmp	eax,ebx
	jne	return_true
	cmp	edx,ecx
	jne	return_true
	jmp	return_false
      logical_number:
	pop	ebx edx eax
	or	bl,bl
	jnz	invalid_expression
	or	eax,edx
	jnz	return_true
	jmp	return_false
      check_for_defined:
	or	bl,-1
	lods	word [esi]
	cmp	ah,'('
	jne	invalid_expression
      check_expression:
	lods	byte [esi]
	or	al,al
	jz	defined_string
	cmp	al,'.'
	je	defined_fp_value
	cmp	al,')'
	je	expression_checked
	cmp	al,'!'
	je	invalid_expression
	cmp	al,0Fh
	je	check_expression
	cmp	al,10h
	je	defined_register
	cmp	al,11h
	je	check_if_symbol_defined
	cmp	al,80h
	jae	check_expression
	movzx	eax,al
	add	esi,eax
	jmp	check_expression
      defined_register:
	inc	esi
	jmp	check_expression
      defined_fp_value:
	add	esi,12
	jmp	expression_checked
      defined_string:
	lods	dword [esi]
	add	esi,eax
	inc	esi
	jmp	expression_checked
      check_if_symbol_defined:
	lods	dword [esi]
	cmp	eax,-1
	je	invalid_expression
	cmp	eax,0Fh
	jb	check_expression
	je	reserved_word_used_as_symbol
	test	byte [eax+8],4
	jnz	no_prediction
	test	byte [eax+8],1
	jz	symbol_predicted_undefined
	mov	cx,[current_pass]
	sub	cx,[eax+16]
	jz	check_expression
	cmp	cx,1
	ja	symbol_predicted_undefined
	or	byte [eax+8],40h+80h
	jmp	check_expression
      no_prediction:
	test	byte [eax+8],1
	jz	symbol_undefined
	mov	cx,[current_pass]
	sub	cx,[eax+16]
	jz	check_expression
	jmp	symbol_undefined
      symbol_predicted_undefined:
	or	byte [eax+8],40h
	and	byte [eax+8],not 80h
      symbol_undefined:
	xor	bl,bl
	jmp	check_expression
      expression_checked:
	mov	al,bl
	jmp	logical_value_ok
      check_for_used:
	lods	word [esi]
	cmp	ah,2
	jne	invalid_expression
	lods	dword [esi]
	cmp	eax,0Fh
	jb	invalid_use_of_symbol
	je	reserved_word_used_as_symbol
	inc	esi
	test	byte [eax+8],8
	jz	not_used
	mov	cx,[current_pass]
	sub	cx,[eax+18]
	jz	return_true
	cmp	cx,1
	ja	not_used
	or	byte [eax+8],10h+20h
	jmp	return_true
      not_used:
	or	byte [eax+8],10h
	and	byte [eax+8],not 20h
	jmp	return_false
      given_false:
	inc	esi
      return_false:
	xor	al,al
	jmp	logical_value_ok
      given_true:
	inc	esi
      return_true:
	or	al,-1
	jmp	logical_value_ok
      logical_expression:
	lods	byte [esi]
	mov	dl,[logical_value_wrapping]
	push	edx
	call	calculate_embedded_logical_expression
	pop	edx
	mov	[logical_value_wrapping],dl
	push	eax
	lods	byte [esi]
	cmp	al,'}'
	jne	invalid_expression
	pop	eax
      logical_value_ok:
	pop	ebx
	xor	al,bl
	ret

skip_symbol:
	lods	byte [esi]
	or	al,al
	jz	nothing_to_skip
	cmp	al,0Fh
	je	nothing_to_skip
	cmp	al,1
	je	skip_instruction
	cmp	al,2
	je	skip_label
	cmp	al,3
	je	skip_label
	cmp	al,20h
	jb	skip_assembler_symbol
	cmp	al,'('
	je	skip_expression
	cmp	al,'['
	je	skip_address
      skip_done:
	clc
	ret
      skip_label:
	add	esi,2
      skip_instruction:
	add	esi,2
      skip_assembler_symbol:
	inc	esi
	jmp	skip_done
      skip_address:
	mov	al,[esi]
	and	al,11110000b
	cmp	al,60h
	jb	skip_expression
	cmp	al,70h
	ja	skip_expression
	inc	esi
	jmp	skip_address
      skip_expression:
	lods	byte [esi]
	or	al,al
	jz	skip_string
	cmp	al,'.'
	je	skip_fp_value
	cmp	al,')'
	je	skip_done
	cmp	al,']'
	je	skip_done
	cmp	al,'!'
	je	skip_expression
	cmp	al,0Fh
	je	skip_expression
	cmp	al,10h
	je	skip_register
	cmp	al,11h
	je	skip_label_value
	cmp	al,80h
	jae	skip_expression
	movzx	eax,al
	add	esi,eax
	jmp	skip_expression
      skip_label_value:
	add	esi,3
      skip_register:
	inc	esi
	jmp	skip_expression
      skip_fp_value:
	add	esi,12
	jmp	skip_done
      skip_string:
	lods	dword [esi]
	add	esi,eax
	inc	esi
	jmp	skip_done
      nothing_to_skip:
	dec	esi
	stc
	ret
