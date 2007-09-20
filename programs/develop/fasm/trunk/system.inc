; flat assembler
; Copyright (c) 1999-2007, Tomasz Grysztar
; All rights reserved.

struc FILEIO
{   .cmd            dd ?
    .offset         dd ?
                    dd ?
    .count          dd ?
    .buff           dd ?
                    db ?
    .name           dd ?
};

struc FILEINFO
{   .attr           dd ?
    .flags          dd ?
    .cr_time        dd ?
    .cr_date        dd ?
    .acc_time       dd ?
    .acc_date       dd ?
    .mod_time       dd ?
    .mod_date       dd ?
    .size           dd ?
}



;file_info_open:  dd 0,0,0xffffff,0x20000,0xf0000
fullpath_open:  ;  db '/RD/1/EXAMPLE.ASM'
		    times MAX_PATH db 0


;file_info_write: dd 1,0,0,0,0xf0000
fullpath_write:; db '/RD/1/EXAMPLE'
		    times MAX_PATH db 0

file_info_start:
    dd 7
    dd 0
    dd 0
    dd 0
    dd 0
fullpath_start:  ; db '/RD/1/EXAMPLE'
		    times MAX_PATH db 0

file_info_debug:
	dd	7
	dd	0
	dd	fullpath_start
	dd	0, 0
	db	'/SYS/DEVELOP/MTDBG',0

_ramdisk	 db '/RD/1/'
filepos 	 dd 0x0

; info by Privalov: starting from FASM 1.51
;   ~3/8 - additional memory
;   ~5/8 - main memory
init_memory:
	mcall	18, 16
	cmp	eax, 0xFFFFFFFF shr 9
	jbe	@f
	mov	eax, 0xFFFFFFFF shr 9
@@:
	shl	eax, 9
	lea	ecx, [eax+stacktop]
	mcall	64, 1
	mov	[memory_start], stacktop
	mov	[additional_memory_end], ecx
	sub	ecx, stacktop
	shr	ecx, 3
	lea	ecx, [ecx*5+stacktop]
	mov	[memory_end],ecx
	mov	[additional_memory],ecx
	ret

exit_program:
	cmp	[_mode],NORMAL_MODE
	jne	@f
	mcall	64, 1, stacktop
	jmp	still
@@:
	or	eax,-1
	mcall

make_timestamp:
	push	ebx
	mcall	26,9
	imul	eax,10
	pop	ebx
	ret

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
;       stc
	ret

alloc_handle:
        call    make_fullpaths
        mov     ebx, fileinfos+4
@@:
        cmp     dword [ebx], -1
        jz      .found
        add     ebx, 4+20+MAX_PATH
        cmp     ebx, fileinfos_end
        jb      @b
        stc
        ret
.found:
        and     dword [ebx+4], 0
        and     dword [ebx+8], 0
        push    esi edi ecx
        mov     esi, fullpath_open
        lea     edi, [ebx+20]
        mov     ecx, MAX_PATH
        rep     movsb
        pop     ecx edi esi
        ret	; CF=0

create:
        call    alloc_handle
        jc      .ret
        and     dword [ebx-4], 0
        mov     dword [ebx], 2
.ret:
        ret


open:
;        call    make_fullpaths

;;       mov     eax,fullpath_open
;;       DEBUGF  '"%s"\n',eax

;        mov     dword[file_info_open+8],-1
;        mcall   58,file_info_open
;        or      eax,eax                 ; found
;        jz      @f
;        cmp     eax,6
;        jne     file_error
;@@: mov [filesize],ebx
;        clc
;        ret
;file_error:
;        stc
;        ret

        call    alloc_handle
        jc      .ret
        mov     dword [ebx], 5
        and     dword [ebx+12], 0
        mov     dword [ebx+16], fileinfo
        mov     eax, 70
        push    ebx
        mcall
        pop     ebx
        test    eax, eax
        jnz     .fail
        mov     eax, [fileinfo.size]
        mov     [ebx-4], eax
        and     dword [ebx], 0
.ret:
        ret
.fail:
	or	dword [ebx], -1	; close handle
        stc
        ret

read:
;          pusha
;          mov     edi,edx
;          mov     esi,[filepos]
;          add     esi,0x20000
;          cld
;          rep     movsb
;          popa
;;         ret

        mov     [ebx+12], ecx
        mov     [ebx+16], edx
        push    ebx
        mov     eax, 70
        mcall
        xchg    eax, [esp]
        add     [eax+4], ebx
        adc     [eax+8], dword 0
        mov     ebx, eax
        pop     eax
        test    eax, eax
        jz      .ok
        cmp     eax, 6
        jz      .ok
        stc
.ok:
        ret

close:
        or      dword [ebx], -1
        ret


; ebx file handle
; ecx count of bytes to write
; edx pointer to buffer
write:
;        pusha
;        mov     [file_info_write+8],ecx
;        mov     [file_info_write+12],edx
;        mov     [filesize],edx
;        mov     eax,58
;        mov     ebx,file_info_write
;        mcall
;        popa
;        ret

        mov     [ebx+12], ecx
        mov     [ebx+16], edx
        push    ebx
        mov     eax, 70
        mcall
        xchg    eax, [esp]
        add     [eax+4], ebx
        adc     [eax+8], dword 0
        mov     ebx, eax
        pop     eax
        mov     byte [ebx], 3
        cmp     eax, 1
        cmc
        ret

make_fullpaths:
	pusha
	push	edx

	mov	esi,path	      ; open
;       DEBUGF  "   '%s'",esi
	mov	edi,fullpath_open
	cld
      newc1:
	movsb
	cmp	byte[esi],0;' '
	jne	newc1
	mov	esi,[esp]

	cmp	byte[esi],'/'
	jne	@f
	mov	edi,fullpath_open

    @@:
	lodsb
	stosb
	cmp	al,0
	jne	@b
;       mov     ecx,12
;       cld
;       rep     movsb
;       mov     byte[edi],0

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
	mov	edi,fullpath_write

    @@:
	lodsb
	stosb
	cmp	al,0
	jne	@b
;        mov     ecx,12
;        cld
;        rep     movsb
;        mov     byte[edi],0

	mov	esi,path	      ; start
	mov	edi,fullpath_start
	cld
      newc3:
	movsb
	cmp  byte[esi],0;' '
	jne  newc3
;        mov  esi,[esp]
	pop	esi

	cmp	byte[esi],'/'
	jne	@f
	mov	edi,fullpath_start

    @@:
	lodsb
	stosb
	cmp	al,0
	jne	@b
;        mov  ecx,12
;        cld
;        rep  movsb
;        mov  byte[edi],0

;        add  esp,4
	popa
	ret



lseek:
	cmp	al,0
	jnz	@f
	and	dword [ebx+4], 0
	and     dword [ebx+8], 0
    @@: cmp	al,2
	jnz	@f
	mov	eax, [ebx-4]
	mov	[ebx+4], eax
	and     dword [ebx+8], 0
    @@: add	dword [ebx+4], edx
        adc     dword [ebx+8], 0
	ret

display_character:
	pusha
	cmp	[_mode],NORMAL_MODE
	jne	@f
	cmp	dl,13
	jz	dc2
	cmp	dl,0xa
	jnz	dc1
	and	[textxy],0x0000FFFF
	add	[textxy], 7 shl 16 +53 and 0xFFFF0000 + 10
dc2:
  popa
	ret
dc1:
  mov eax,[textxy]
	cmp	ax,word[bottom_right]
	ja	dc2
	shr	eax,16
	cmp	ax,word[bottom_right+2]
	ja	dc2
	mov	[dc],dl
	mcall	4,[textxy],[sc.work_text],dc,1
	add	[textxy],0x00060000
	popa
	ret
@@:
  mov	eax,63
	mov	ebx,1
	mov	cl,dl
	mcall
	popa
	ret


display_string:
	pusha
@@:
  cmp	byte[esi],0
	je	@f
	mov	dl,[esi]
	call	display_character
	add	esi,1
	jmp	@b
@@:
  popa
	ret

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
	mov	dl,al
	add	dl,30h
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
	ret

display_user_messages:
;    push   [skinh]
;    pop    [textxy]
;    add    [textxy], 7 shl 16 +53
	mov	[displayed_count],0
	call	flush_display_buffer
	cmp	[displayed_count],1
	jb	line_break_ok
	je	make_line_break
	mov	ax,word[last_displayed]
	cmp	ax,0A0Dh
	je	line_break_ok
	cmp	ax,0D0Ah
	je	line_break_ok
make_line_break:
	mov	esi,lf
	call	display_string
line_break_ok:
	ret

display_block:
	pusha
@@: mov dl,[esi]
	call	display_character
	inc	esi
	loop	@b
	popa
	ret

fatal_error:
	mov	esi,error_prefix
	call	display_string
	pop	esi
	call	display_string
	mov	esi,error_suffix
	call	display_string
	mov	esi,lf
	call	display_string
	mov	al,0FFh
	jmp	exit_program

assembler_error:
	call	display_user_messages
	push	dword 0
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
	mov	dl,']'
	call	display_character
	pop	esi
	cmp	ebx,esi
	je	line_number_ok
	mov	dl,20h
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
	mov	dl,']'
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
	cmp	al,0Ah
	je	display_line_data
	cmp	al,0Dh
	je	display_line_data
	cmp	al,1Ah
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
	mov	esi,cr_lf
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

align 4
fileinfo FILEINFO

character   db ?,0
bytes_count dd ?

textxy	 dd 0x000500A0
dc	 db 0x0
filesize dd 0x0

displayed_count dd ?
last_displayed	rb 2

error_prefix	  db 'error: ',0
error_suffix	  db '.',0
line_data_start   db ':'
cr_lf		  db 0Dh,0Ah,0
line_number_start db ' [',0

macro dm string { db string,0 }
