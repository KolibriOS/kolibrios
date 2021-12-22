; flat assembler
; Copyright (c) 1999-2021, Tomasz Grysztar
; All rights reserved.

init_memory:

;	 mov ecx, 16*1024*1024
;
; allocate_memory:
	mcall	SF_SYSTEM, SSF_GET_FREE_RAM
	cmp	eax, 0x38000000 shr 9
	jbe	@f
	mov	eax, 0x38000000 shr 9
@@:
	shl	eax, 9
	xchg	eax, ecx
	mov	[memory_setting],ecx
	mcall	SF_SYS_MISC, SSF_MEM_ALLOC
	or	eax,eax
	jz	out_of_memory
	mov	[memblock], eax
	mov	[additional_memory],eax
	add	eax,[memory_setting]
	mov	[memory_end],eax
	mov	eax,[memory_setting]
	shr	eax,2
	add	eax,[additional_memory]
	mov	[additional_memory_end],eax
	mov	[memory_start],eax
	retn

exit_program:
	cmp	[_mode],NORMAL_MODE
	jne	@f
	mcall	SF_SYS_MISC, SSF_MEM_FREE, [memblock]
	mov	edi, file_IO_slots
	mov	ecx, (file_IO_end-file_IO_slots)/4
	or	eax, -1
	rep	stosd
	mov	esp,[processing_esp]
	ret;jmp     still
@@:
	or	eax,-1
	mcall

get_tickcount:
	push	ebx
	mcall	SF_SYSTEM_GET,SSF_TIME_COUNT
	imul	eax,10
	pop	ebx
	retn

macro BCDtoHEX al {
	aam	16
	aad	10 }

make_timestamp:
	mcall	SF_GET_SYS_DATE ; $00SSMMHH (BCD)
	mov	edx,eax
	shr	eax,16
	BCDtoHEX al
	push	eax		; SECONDS
	mov	al,dh
	BCDtoHEX al
	push	eax		; MINUTES
	mov	al,dl
	BCDtoHEX al
	push	eax		; HOURS
	mcall	SF_GET_SYS_DATE ; $00DDMMYY (BCD)
	mov	edx,eax
	shr	eax,16
	BCDtoHEX al
	push	eax		; DAY
	mov	al,dl
	BCDtoHEX al
	add	eax,2000
	push	eax		; YEAR
	mov	ecx,eax
	mov	al,dh
	BCDtoHEX al
	push	eax		; MONTH
; ecx: YEAR
; stack: MONTH, YEAR, DAY, HOURS, MINUTES, SECONDS, retaddr
	mov	eax,ecx
	sub	eax,1970
	mov	ebx,365
	mul	ebx
	mov	ebp,eax
	mov	eax,ecx
	sub	eax,1969
	shr	eax,2
	add	ebp,eax
	mov	eax,ecx
	xor	edx,edx
	sub	eax,1901
	mov	ebx,100
	div	ebx
	sub	ebp,eax
	mov	eax,ecx
	xor	edx,edx
	sub	eax,1601
	mov	ebx,400
	div	ebx
	add	ebp,eax
	pop	ecx
; ecx: MONTH
; stack: YEAR, DAY, HOURS, MINUTES, SECONDS, retaddr
	mov	eax,ecx
	dec	eax
	mov	ebx,30
	mul	ebx
	add	ebp,eax
	cmp	ecx,8
	jbe	months_correction
	mov	eax,ecx
	sub	eax,7
	shr	eax,1
	add	ebp,eax
	mov	ecx,8
      months_correction:
	mov	eax,ecx
	shr	eax,1
	add	ebp,eax
	cmp	ecx,2
	pop	ecx
; ecx: YEAR
; stack: DAY, HOURS, MINUTES, SECONDS, retaddr
	jbe	day_correction_ok
	sub	ebp,2
	test	ecx,11b
	jnz	day_correction_ok
	xor	edx,edx
	mov	eax,ecx
	mov	ebx,100
	div	ebx
	or	edx,edx
	jnz	day_correction
	mov	eax,ecx
	mov	ebx,400
	div	ebx
	or	edx,edx
	jnz	day_correction_ok
      day_correction:
	inc	ebp
      day_correction_ok:
	pop	eax
; eax: DAY
; stack: HOURS, MINUTES, SECONDS, retaddr
	dec	eax
	add	eax,ebp
	mov	ebx,24
	mul	ebx
	pop	ecx
; ecx: HOURS
; stack: MINUTES, SECONDS, retaddr
	add	eax,ecx
	mov	ebx,60
	mul	ebx
	pop	ecx
; ecx: MINUTES
; stack: SECONDS, retaddr
	add	eax,ecx
	mov	ebx,60
	mul	ebx
	pop	ecx
; ecx: SECONDS
; stack: retaddr
	add	eax,ecx
	retn

symbol_dump:

	push	edi
	mov	edx,[memory_end]
      symb_dump:
	cmp	edx,[labels_list]
	jbe	symbols_dumped
	sub	edx,LABEL_STRUCTURE_SIZE
	cmp	dword [edx+24],0
	je	symb_dump	; do not dump anonymous symbols
	test	byte [edx+8],1
	jz	symb_dump	; do not dump symbols that didn't get defined
	mov	ax,[current_pass]
	cmp	ax,[edx+16]
	jne	symb_dump
	test	byte [edx+8],4 or 2
	jnz	symb_dump	; do not dump assembly-time variables
				; do not dump variables defined with '='
	cmp	word [edx+12], 0
	jnz	symb_dump	; do not dump register-based variables

	mov	al, '0'
	stosb
	mov	al, 'x'
	stosb
	mov	eax, [edx+4]
	mov	ecx, 8
@@:
	rol	eax, 4
	test	al, 0xF
	loopz	@b
	jz	.nohigh
	inc	ecx
@@:
	push	eax
	and	al, 0xF
	cmp	al, 10
	sbb	al, 69h
	das
	stosb
	pop	eax
	rol	eax, 4
	loop	@b
	mov	eax, [edx]
	mov	ecx, 8
	jmp	.low
.nohigh:
	mov	eax, [edx]
	mov	ecx, 8
@@:
	rol	eax, 4
	test	al, 0xF
	loopz	@b
	inc	ecx
.low:
	push	eax
	and	al, 0xF
	cmp	al, 10
	sbb	al, 69h
	das
	stosb
	pop	eax
	rol	eax, 4
	loop	.low

	mov	al, ' '
	stosb

	mov	esi,[edx+24]
	movzx	ecx,byte [esi-1]
	rep	movsb

	mov	ax,0A0Dh
	stosw

	jmp	symb_dump

      symbols_dumped:
	mov	edx,dbgfilename
	push	esi edi
	mov	esi, outfile
	mov	edi, edx
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
	lea	ecx, [edi-1]
@@:
	dec	edi
	cmp	edi, edx
	jb	@f
	cmp	byte [edi], '/'
	jz	@f
	cmp	byte [edi], '.'
	jnz	@b
	mov	ecx, edi
@@:
	mov	dword [ecx], '.dbg'
	mov	byte [ecx+4], 0
	pop	edi esi
	call	create
	mov	edx,[esp]
	mov	ecx,edi
	sub	ecx,edx
	call	write
	call	close
	pop	edi

	retn

get_environment_variable:
	mov	ecx,[memory_end]
	sub	ecx,edi
	cmp	ecx,7
	jb	out_of_memory
	cmp	dword[esi],'INCL'
	jne	.finish
	mov	esi,_ramdisk
	mov	ecx,6
	cld
	rep	movsb
  .finish:
;	stc
	retn

alloc_handle:
; in:
; out: ebx = file handle
; on error: return to caller of caller with cf set
; preserves: esi, edi
	call	make_fullpaths
	mov	ebx, file_IO_slots+4
	jmp	check_file_IO_slot
      next_file_IO_slot:
	add	ebx, 4+20+MAX_PATH
	cmp	ebx, file_IO_end
	jae	send_error_to_caller_of_caller
      check_file_IO_slot:
	cmp	[ebx+FILEIO.cmd], -1
	jnz	next_file_IO_slot
	and	[ebx+FILEIO.offset], 0
	and	[ebx+FILEIO.flags], 0
      adapt_path:
	push	esi edi ecx
	mov	esi, fullpath_open
	lea	edi, [ebx+namedFILEIO.name]
	mov	ecx, MAX_PATH
      copy_path:
	lodsb
	cmp	al,'\'
	jne	path_char_ok
	mov	al,'/'
      path_char_ok:
	stosb
	or	al,al
	loopnz	copy_path
	pop	ecx edi esi
	jz	adapt_path.done
      send_error_to_caller_of_caller:
	stc
	pop	eax
      adapt_path.done:
	ret

create:
; in:
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	call	alloc_handle
	and	[ebx+FILEIO.filesize], 0
	mov	[ebx+FILEIO.cmd], SSF_CREATE_FILE
	retn


open:
; in:
; out: ebx = file handle, cf set on error
; preserves: esi, edi
	call	alloc_handle
	mov	[ebx+FILEIO.cmd], SSF_GET_INFO
	and	[ebx+FILEIO.count], 0
	mov	[ebx+FILEIO.buffer], fileinfo
	push	ebx
	mcall	SF_FILE
	pop	ebx
	test	eax, eax
	jnz	fail_close
	mov	eax, dword[fileinfo.size]
	mov	[ebx+FILEIO.filesize], eax
	and	[ebx+FILEIO.cmd], SSF_READ_FILE
	retn

fail_close:
	stc
close:
; in: ebx = file handle
; preserves: ebx, esi, edi, cf
	mov	[ebx+FILEIO.cmd], -1 ; close handle
	retn
write:
; in: ebx = file handle, edx - data, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	call	read_write
	mov	[ebx+FILEIO.cmd], SSF_WRITE_FILE
	jmp	read_write_check_S_OK
read:
; in: ebx = file handle, edx - buffer, ecx = number of bytes
; out: cf set on error
; preserves: ebx, esi, edi
	call	read_write
	cmp	eax, 6
	jz	read_write_OK
      read_write_check_S_OK:
	test	eax, eax
	jz	read_write_OK
	stc
	retn
      read_write:
	mov	[ebx+FILEIO.count], ecx
	mov	[ebx+FILEIO.buffer], edx
	push	ebx
	mcall	SF_FILE
	xchg	eax, [esp]
	add	[eax+FILEIO.offset], ebx
	adc	[eax+FILEIO.offshigh], 0
	mov	ebx, eax
	pop	eax
      read_write_OK:
	retn


make_fullpaths:
	pusha
	push	edx
	mov	esi,edx
	mov	ecx, MAX_PATH
      copy_edxpath:
	lodsb
	cmp	al,'\'
	jne	edxpath_char_ok
	mov	byte[esi-1],'/'
      edxpath_char_ok:
	or	al,al
	loopnz	copy_edxpath

	mov	esi,path	      ; open
;	DEBUGF	"   '%s'",esi
	mov	edi,fullpath_open
	cld
      newc1:
	movsb
	cmp	byte[esi],0;' '
	jne	newc1
	mov	esi,[esp]

	cmp	byte[esi],'/'
	je	absolute_path
	cmp	byte[esi],'\'
	jne	@F
    absolute_path:
	mov	edi,fullpath_open

    @@:
	lodsb
	stosb
	cmp	al,0
	jne	@b

	mov	esi,path	      ; write
	mov	edi,fullpath_write
	cld
      newc2:
	movsb
	cmp	byte[esi],0;' '
	jne	newc2
	mov	esi,[esp]

	cmp	byte[esi],'/'
	jne	@f
	cmp	byte[esi],'\'
	jne	@f
	mov	edi,fullpath_write

    @@:
	lodsb
	stosb
	cmp	al,0
	jne	@b

	mov	esi,path	      ; start
	mov	edi,file_io_start.path
	cld
      newc3:
	movsb
	cmp  byte[esi],0;' '
	jne  newc3
	pop	esi

	cmp	byte[esi],'/'
	jne	@f
	cmp	byte[esi],'\'
	jne	@f
	mov	edi,file_io_start.path

    @@:
	lodsb
	stosb
	cmp	al,0
	jne	@b
	popa
	retn
lseek:
; in: ebx = file handle, al = method, edx = delta offset
; out: cf set on error
; preserves: ebx, esi, edi
	cmp	al,FILE_BEGIN
	jnz	@f
	and	[ebx+FILEIO.offset], 0
	jmp	.common
    @@: cmp	al,FILE_END
	jnz	@f
	mov	eax, [ebx+FILEIO.filesize]
	mov	[ebx+FILEIO.offset], eax
	jmp	.common
    @@:
    .common:
	add	[ebx+FILEIO.offset], edx
	retn

display_character:
	pusha
	cmp	[_mode],NORMAL_MODE
	jne	@f
	cmp	al,$D
	jz	dc2
	cmp	al,$A
	jnz	dc1
	and	[textxy],0x0000FFFF
	add	[textxy], 8 shl 16 and 0xFFFF0000 + 18
	popa
	retn
dc1:
	mov	[dc],al
	mov	eax,[textxy]
	cmp	ax,word[bottom_right]
	ja	dc2
	shr	eax,16
	cmp	ax,word[bottom_right+2]
	ja	dc2
	mov	ecx,[sc.work_text]
	or	ecx,$10000000
	mcall	SF_DRAW_TEXT,[textxy],,dc,1
	add	[textxy],0x00080000
dc2:
	popa
	retn
@@:
	mov	cl,al
	mcall	SF_BOARD,SSF_DEBUG_WRITE
	popa
	retn


display_string:
; in:
;  esi - ASCIIZ string
; preserves: ebx, esi
	push	esi
@@:	lodsb
	test	al,al
	je	@f
	call	display_character
	jmp	@b
@@:
	pop	esi
	retn

display_number:
	push	ebx
	mov	ecx,1000000000
	xor	edx,edx
	xor	bl,bl
display_loop:
	div	ecx
	push	edx
	cmp	ecx,1
	je	display_digit
	or	bl,bl
	jnz	display_digit
	or	al,al
	jz	digit_ok
	not	bl
display_digit:
	add	al,'0'
	push	ebx ecx
	call	display_character
	pop	ecx ebx
digit_ok:
	mov	eax,ecx
	xor	edx,edx
	mov	ecx,10
	div	ecx
	mov	ecx,eax
	pop	eax
	or	ecx,ecx
	jnz	display_loop
	pop	ebx
	retn

display_user_messages:
;    push   [skinh]
;    pop    [textxy]
;    add    [textxy], 7 shl 16 +53
	mov	[displayed_count],0
	call	show_display_buffer
	cmp	[displayed_count],1
	jb	line_break_ok
	je	make_line_break
	mov	ax,word[last_displayed]
	cmp	ax,0A0Dh
	je	line_break_ok
	cmp	ax,0D0Ah
	je	line_break_ok
make_line_break:
	mov	esi,crlf
	call	display_string
line_break_ok:
	retn

display_block:
; in:
;  esi - string
;  ecx = string length
	push	esi
@@:	lodsb
	call	display_character
	loop	@b
	pop	esi
	retn

fatal_error:
; no return, trashes stack

	mov	esi,error_prefix
	call	display_string
	pop	esi
	call	display_string
	mov	esi,error_suffix
	call	display_string
	mov	esi,crlf
	call	display_string
	mov	al,0FFh
	jmp	exit_program

assembler_error:
	call	display_user_messages
	push	0
	mov	ebx,[current_line]
get_error_lines:
	push	ebx
	test	byte [ebx+7],80h
	jz	display_error_line
	mov	edx,ebx
find_definition_origin:
	mov	edx,[edx+12]
	test	byte [edx+7],80h
	jnz	find_definition_origin
	push	edx
	mov	ebx,[ebx+8]
	jmp	get_error_lines
display_error_line:
	mov	esi,[ebx]
	call	display_string
	mov	esi,line_number_start
	call	display_string
	mov	eax,[ebx+4]
	and	eax,7FFFFFFFh
	call	display_number
	mov	al,']'
	call	display_character
	pop	esi
	cmp	ebx,esi
	je	line_number_ok
	mov	al,' '
	call	display_character
	push	esi
	mov	esi,[esi]
	movzx	ecx,byte [esi]
	inc	esi
	call	display_block
	mov	esi,line_number_start
	call	display_string
	pop	esi
	mov	eax,[esi+4]
	and	eax,7FFFFFFFh
	call	display_number
	mov	al,']'
	call	display_character
line_number_ok:
	mov	esi,line_data_start
	call	display_string
	mov	esi,ebx
	mov	edx,[esi]
	call	open
	mov	al,2
	xor	edx,edx
	call	lseek
	mov	edx,[esi+8]
	sub	eax,edx
	push	eax
	xor	al,al
	call	lseek
	mov	ecx,[esp]
	mov	edx,[additional_memory]
	lea	eax,[edx+ecx]
	cmp	eax,[additional_memory_end]
	ja	out_of_memory
	call	read
	call	close
	pop	ecx
	mov	esi,[additional_memory]
get_line_data:
	mov	al,[esi]
	cmp	al,$A
	je	display_line_data
	cmp	al,$D
	je	display_line_data
	cmp	al,$1A
	je	display_line_data
	or	al,al
	jz	display_line_data
	inc	esi
	loop	get_line_data
display_line_data:
	mov	ecx,esi
	mov	esi,[additional_memory]
	sub	ecx,esi
	call	display_block
	mov	esi,crlf
	call	display_string
	pop	ebx
	or	ebx,ebx
	jnz	display_error_line
	mov	esi,error_prefix
	call	display_string
	pop	esi
	call	display_string
	mov	esi,error_suffix
	call	display_string
	jmp	exit_program