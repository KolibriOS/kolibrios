; Files *.asc routines by Maciej Guba
; Thanks to Reverend for integer/float/ascii conversion examples
read_asc:
    mov  eax,[fptr]
 .find_vert:
    cmp  dword[eax],'Vert'
    je	 @f
    inc  eax
    jmp  .find_vert
  @@:
    add   eax,4
    cmp   dword[eax],'ices'
    jne   .find_vert
    add   eax,3
  @@:
    inc   eax
    cmp   byte[eax],'0' 	  ; search end of ascii number of vertices string
    jb	  @b
    cmp   byte[eax],'9'
    ja	  @b
;    eax - start ascii number
  @@:
    inc    eax
    cmp    byte[eax],'0'
    jb	  .convert1
    cmp    byte[eax],'9'
    ja	  .convert1
    jmp   @b
  .convert1:
    dec   eax
    mov   ebx,eax
    push  eax
    call  ascii_to_integer
    mov   [points_count_var],edx
    pop   eax

  @@:
    inc   eax
    cmp   dword[eax],'Face'
    jne   @b
    add   eax,3
  @@:
    inc   eax
    cmp   byte[eax],'0'
    jb	  @b
    cmp   byte[eax],'9'
    ja	  @b
   ;    eax - start ascii number
  @@:
    inc    eax
    cmp    byte[eax],'0'
    jb	  .convert2
    cmp    byte[eax],'9'
    ja	  .convert2
    jmp   @b
   ;  eax - end ascii number
  .convert2:
    dec   eax
    mov   ebx,eax
    push  eax
    call  ascii_to_integer
    mov   [triangles_count_var],edx
    pop   eax

  @@:
    inc   eax
    cmp   dword[eax],'Vert'
    jnz   @b
    inc   eax

    mov   edi,[points_ptr]
    xor   ebx,ebx
   .decode_vertices:
    push   ebx
  @@:
    inc   eax
    cmp   dword[eax],'Vert'
    jne   @b
    xor   ecx,ecx

  .decode_coord:
    push  ecx
  @@:
    inc   eax
    mov   dl,byte[eax]
    cmp   dl,byte[XYZpartices+ecx]
    jne   @b
  @@:
    inc   eax
    cmp   byte[eax],'.'
    je	  .readF
    cmp   byte[eax],'-'
    je	  .readF
    cmp   byte[eax],'0'
    jb	  @b
    cmp   byte[eax],'9'
    ja	  @b
.readF: ; read float
    mov   esi,eax
    push  eax
    push  ecx

    call   atof   ; st0 - desired dword float

    pop   ecx
    pop   eax

    fstp  dword[edi]
    add   edi,4

    pop   ecx
    inc   ecx
    cmp   ecx,3
    jne   .decode_coord
    pop   ebx
    inc   ebx
    cmp   ebx,[points_count_var]
    jne   .decode_vertices
    mov   dword[edi],-1



    mov   esi,eax
   @@:
    inc   esi
    cmp   dword[esi],'Face'
    jne   @b
    xor   edx,edx
    mov   edi,[triangles_ptr]
    cld
 .decode_face:

    push  edx
   @@:
    inc   esi
    cmp   dword[esi],'Face'
    jne   @b
 @@:
    inc   esi
    cmp   byte[esi],'0'    ; face number start
    jb	  @b
    cmp   byte[esi],'9'
    ja	  @b
  @@:
    inc    esi
    cmp    byte[esi],'0'
    jb	  @f
    cmp    byte[esi],'9'   ; face number end
    ja	  @f
    jmp   @b
  @@:
    xor ecx,ecx
 .next_vertex_number:

    push  ecx
  @@:
    inc   esi
    cmp   byte[esi],'0'
    jb	  @b
    cmp   byte[esi],'9'
    ja	  @b
    ;    eax - start ascii number
  @@:
    inc    esi
    cmp    byte[esi],'0'
    jb	  @f
    cmp    byte[esi],'9'
    ja	  @f
    jmp   @b
   ;  eax - end ascii number
  @@:
    dec   esi
    mov   ebx,esi
    push  esi
    call  ascii_to_integer
    mov   eax,edx

    stosd
    pop   esi
    add   esi,4

    pop   ecx
    inc   ecx
    cmp   ecx,3
    jne   .next_vertex_number
    pop   edx
    inc   edx
    cmp   edx,[triangles_count_var]
    jne   .decode_face
    mov   dword[edi],-1 ;dword[triangles+ebx+2],-1   ; end mark
    mov   eax,1  ;-> mark if ok
ret

ascii_to_integer:
; in ---  [ebx] -> end of ascii string
; out --  edx  -> desired number
    xor    edx,edx
    xor    ecx,ecx
  .again:
    movzx  eax,byte[ebx]
    sub    al,'0'
    cwde
    push   edx
    mul    dword[convert_muler+ecx]
    pop    edx
    add    edx,eax
    dec    ebx
    cmp    byte[ebx],'0'
    jb	   .end
    cmp    byte[ebx],'9'
    ja	   .end
    add    ecx,4
    jmp    .again
  @@:

  .end:
ret

;===============================================================================
;       ASCII to float conversion procedure
;
;       input:
;       esi - pointer to string
;
;       output:
;       st0 - number changed into float
;
;===============================================================================

atof:
	.string  equ ebp-4

	push	ebp
	mov	ebp,esp
	sub	esp,32
	push	eax ecx esi
	mov	[.string],esi
	fninit
	fldz
	fldz

	cld
	cmp	byte [esi], '-'
	jnz	@F
	inc	esi
    @@:
	xor	eax, eax
	align	4
  .loop.integer_part:
	lodsb
	cmp	al, '.'
	jz	.mantisa
	cmp	al,'0'
	jb	.exit
	cmp	al,'9'
	ja	.exit
	fimul	[i10]
	sub	al, '0'
	push	eax
	fiadd	dword [esp]
	add	esp, 4
	jmp	.loop.integer_part

  .mantisa:
	xor	 ecx, ecx
	xor	 eax, eax
	cld
	fxch	 st1
     @@:

	lodsb
	cmp	al,'0'
	jb	.exit
	cmp	al,'9'
	ja	.exit
	cmp	ecx,7*4
	je	.exit		; max 7 digits in mantisa
	sub	al,'0'
	push	eax
	fild	dword[esp]
	fidiv	dword[convert_muler+4+ecx]
	faddp
	add	esp,4
	add	ecx,4
	jmp	@b
  .exit:
	faddp

	mov	eax, [.string]
	cmp	byte [eax], '-'
	jnz	@F
	fchs
    @@:
	cld
	stc				; always returns no error
	pop	esi ecx eax
	mov	esp,ebp
	pop	ebp
	ret


itoa:		   ; unsigned dword integer to ascii procedure
;     in   eax - variable
;          esi - Pointer to ascii string
;   out    esi - desired ascii string
;          edi - end of ascii string  - ptr to memory
 .temp_string  equ dword[ebp-36]
 .ptr	       equ dword[ebp-40]
 .var	       equ dword[ebp-44]
	   push  ecx
	   push  ebp
	   mov	 ebp,esp
	   sub	 esp,64
	   mov	 .var,eax
	   mov	 eax,-1
	   lea	 edi,.temp_string
	   cld
	   mov	 ecx,9
	   rep	 stosd	    ; make floor


	   mov	 .ptr,esi
	   lea	 edi,.temp_string
	   add	 edi,34
	   std
	   xor	 eax,eax
	   stosb	       ; mark begin
	   mov	 eax,.var
	   mov	 esi,10
	@@:
	   xor	 edx,edx
	   div	 esi
	   xchg  eax,edx
	   add	 al,'0'
	   stosb
	   xchg  eax,edx
	   or	 eax,eax
	   jnz	 @b
	   stosb		     ; mark end

	   lea	 esi,.temp_string
	   cld
	@@:
	   lodsb
	   or	al,al
	   jnz	@b

	   mov	edi,.ptr
	 @@:
	   lodsb
	   stosb
	   or	al,al
	   jnz	@b

	   mov	esp,ebp
	   pop	ebp
	   pop	ecx
ret
if 1
ftoa_mac:
;   in  :  esi - pointer to  dword float
;          edi - pointer to ascii string
  .ptr_f     equ dword[ebp-4]
  .sign      equ dword[ebp-8]	;  0 ->   less than zero, 1 - otherwise
  .ptr_ascii equ dword[ebp-12]
  .integer   equ dword[ebp-20]
  .fraction  equ dword[ebp-28]
  .status_orginal equ word[ebp-32]
  .status_changed equ word[ebp-34]
	 push	  ecx
	 push	  ebp
	 mov	  ebp,esp
	 sub	  esp,64
	fninit
	fnstcw	.status_orginal
	mov	ax, .status_orginal
	or	ax, 0000110000000000b
	mov	.status_changed, ax
	fldcw	.status_changed
;       --------------------------------
;       check if signed
	xor	eax, eax
	fld	dword[esi]
	fst	.sign
	test	.sign, 80000000h
	setz	al
	mov	.sign, eax

	 mov	   .ptr_f,esi
	 mov	   .ptr_ascii,edi
	 fabs
	 fld	   st0
	 frndint
	 fist	   .integer
	 fsubp	   st1, st0

	 mov	   eax,.integer
	 mov	   esi,.ptr_ascii
	 call	   itoa
      ;  edi   ->   ptr to end of ascii string
	 dec	   edi
	 mov	   al,'.'
	 stosb

	mov	ecx, 6	 ; max 6 digits in fraction part
    .loop:
	fimul	[i10]
	fld	st0
	frndint
	fist	.fraction
	fsubp	st1, st0
	mov	esi,edi
	mov	eax,.fraction
	add	al,'0'
	stosb
	ftst
	fnstsw	ax
	test	ax, 0100000000000000b
	jz	@F
	test	ax, 0000010100000000b
	jz	.finish
    @@:
	loop	.loop
if 0
	fldcw	.status_orginal
	fimul	[i10]
	fist	.fraction
    ;    mov     esi,edi
	mov	eax,.fraction
	add	al,'0'
	stosb
    ;    call    itoa
;       --------------------------------
;       restore previous values
    .finish:
      ;  fstp    st0
	 ffree	   st
	 mov	   eax,.fraction
	 mov	   esi,edi
;         call      itoa

	 add	   al,'0'
	 stosb
end if
 .finish:
  ffree st
	 cmp	   .sign,0
	 jnz	   @f
	 mov	   esi,.ptr_ascii
	 dec	   esi
	 mov	   byte[esi],'-'
       @@:
	 mov	   esp,ebp
	 pop	   ebp
	 pop	   ecx

ret
end if
if 0
;===============================================================================
;       float to ASCII conversion procedure
;
;       input:
;       buffer - pointer to memory where output will be saved
;       precision - number of digits after dot
;
;       output:
;       no immediate output
;
;       notes:
;       separate integer and mantisa part with dot '.'
;       so GOOD   123.456
;          WRONG  123,456
;
;       coded by Reverend // HTB + RAG
;===============================================================================
proc	ftoa			buffer, precision
locals
  status_original	      dw ?
  status_changed	      dw ?
  integer		      dd ?
  mantisa		      dd ?
  signed		      dd ?
endl
	push	eax ecx;edi ecx
;       --------------------------------
;       set correct precision
	mov	eax, [precision]
	cmp	eax, 51
	jb	@F
	mov	eax, 51
    @@:
	mov	[precision], eax
;       --------------------------------
;       change control wortd of fpu to prevent rounding
	fnstcw	[status_original]
	mov	ax, [status_original]
	or	ax, 0000110000000000b
	mov	[status_changed], ax
	fldcw	[status_changed]
;       --------------------------------
;       check if signed
	xor	eax, eax
	fst	[signed]
	test	[signed], 80000000h
	setnz	al
	mov	[signed], eax
;       --------------------------------
;       cut integer and mantisa separately
	fld	st0
	fld	st0			; st0 = x, st1 = x
	frndint
	fist	[integer]		; st0 = x, st1 = x
	fabs
	fsubp	st1, st0		; st0 = mantisa(x)
;       --------------------------------
;       save integer part in buffer
    ;    mov     edi, [buffer]
	mov	esi,[buffer]
      ;  push    [signed]
      ;  push    edi
      ;  push    10
      ;  push    [integer]
	mov	eax,[integer]
	call	itoa
 ;       add     edi, eax
	mov	al, '.'
	stosb
	mov	esi,edi
;       --------------------------------
;       save mantisa part in buffer
	mov	ecx, [precision]
	dec	ecx
    .loop:
	fimul	[i10]
	fld	st0
	frndint
	fist	[mantisa]
	fsubp	st1, st0
;        push    0
;        push    edi
;        push    10
;        push    [mantisa]
	mov	esi,edi
	mov	eax,[mantisa]
	call	itoa
      ;  add     edi, eax
	ftst
	fnstsw	ax
	test	ax, 0100000000000000b
	jz	@F
	test	ax, 0000010100000000b
	jz	.finish
    @@:
	loop	.loop
	fldcw	[status_original]
	fimul	[i10]
	fist	[mantisa]
;        push    0
;        push    edi
;        push    10
;        push    [mantisa]
	mov	esi,edi
	mov	eax,[mantisa]
	call	itoa
;       --------------------------------
;       restore previous values
    .finish:
	fstp	st0
	cmp	[signed],1
	jnz   @f
	mov   byte[buffer],'-'
      @@:
	stc
	pop	ecx eax   ;edi eax
	ret
endp
end if
if 0
write_asc:
    .counter equ dword[ebp-4]
	push  ebp
	mov   ebp,esp
	sub   esp,64
	fninit
	mov   edi,asc_file_buffer
	mov   esi,asc_main_header
	cld
      @@:
	lodsb
	cmp   al,1
	jz    @f
	stosb
	jmp   @b
      @@:

	mov   esi,asc_info_header
      @@:
	lodsb
	cmp   al,1
	jz    @f
	stosb
	jmp   @b
     @@:
	push   esi  ; -> position in header info
	movzx  eax,[points_count_var]
	mov    esi,edi
	call   itoa		 ; unsigned dword integer to ascii procedure
	pop   esi
	inc   esi
      @@:
	lodsb
	cmp   al,1
	jz    @f
	stosb
	jmp   @b
      @@:
	push  esi
	movzx eax,[triangles_count_var]
	mov   esi,edi
	call  itoa
	pop   esi
	inc   esi
      @@:
	lodsb
	cmp   al,1
	jz    @f
	stosb
	jmp   @b
      @@:
;=============================================================
;================vertex list parser===========================
;=============================================================

       xor   ecx,ecx
     .again_vertex:
       push  ecx
       mov   esi,asc_one_vertex_formula
     @@:
       lodsb
       cmp   al,1
       jz    @f
       stosb
       jmp   @b
     @@:
       mov   eax,ecx
 ;      push  ecx
       push  esi
       mov   esi,edi
       call  itoa
       pop   esi
;       pop   ecx
       inc   esi
       xor   ebx,ebx
    .next_vertex_coef:
       push  ebx
     @@:
       lodsb
       cmp   al,1
       jz    @f
       stosb
       jmp   @b
     @@:
    ; int3
       push  esi
       lea   esi,[ecx*3]
       shl   esi,2
       add   esi,points_r
       add   esi,ebx
     ;  int3
       call  ftoa_mac
     ;  std
     ;  fld     dword[esi]


     ;  pushad
     ;  stdcall ftoa, edi, 30
     ;  popad
     ;  add   edi,20


       pop   esi
       pop   ebx
       add   ebx,4
       cmp   ebx,12
       jnz   .next_vertex_coef
     @@:
       lodsb
       cmp   al,1
       jz    @f
       stosb
       jmp   @b
     @@:
       pop   ecx
       inc   ecx
       cmp   cx,[points_count_var]
       jnz   .again_vertex


   ;   mov  edi,[temp_edi]


       mov  esi,asc_face_list_header
      @@:
	lodsb
	cmp   al,1	; all face header
	jz    @f
	stosb
	jmp   @b
      @@:
	   ;=====================================
	   ; ==============face list parser======
	   ;=====================================
       xor   ecx,ecx
     .again_face:
       push  ecx
       mov   .counter,ecx
       mov   esi,asc_one_face_formula
     @@:
       lodsb
       cmp   al,1
       jz    @f
       stosb
       jmp   @b
     @@:
       mov   eax,ecx
       push  esi
       mov   esi,edi
       call  itoa
       pop   esi
       inc   esi
     @@:
       lodsb
       cmp   al,1
       jz    @f
       stosb
       jmp   @b
     @@:

       xor    ebx,ebx
      .next_face_index:
	push   ebx
	mov    ecx,.counter
	lea    ecx,[ecx*3]
	add    ecx,ecx
	movzx  eax,word[triangles+ecx+ebx]
	push   esi
	mov    esi,edi
	call   itoa
	pop    esi
       @@:
	lodsb
	cmp   al,1
	jz    @f
	stosb
	jmp   @b
      @@:
	pop    ebx
	add    ebx,2
	cmp    ebx,6
	jnz    .next_face_index

;        push   esi
	mov   esi,asc_material
     @@:
	lodsb
	cmp   al,1
	jz    @f
	stosb
	jmp   @b
     @@:
;        pop   esi

	 pop	ecx
	inc    ecx
	cmp    cx,[triangles_count_var]
	jnz    .again_face

	; write file
    sub  edi,asc_file_buffer
  ;  mov  [file_buffer+2],edi
    mov  [FileSize],edi

    invoke  CreateFile,asc_file_name, GENERIC_WRITE, 0, 0,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
    mov  [hfile],eax
    invoke  WriteFile,eax,asc_file_buffer,[FileSize], byteswritten, 0
    invoke  CloseHandle,[hfile]

    mov  esp,ebp
    pop  ebp
ret
end if
if 0
asc_file_buffer:
	      rd 65536
ascii_string  rb 50
ftoa_muler dd 1000000000
file_size dd ?
file_handle dd ?
end if

if 0
convert_muler:
dd 1, 10, 100, 1000, 10000
XYZpartices:
db 'X','Y','Z'
i10 dw 10
points_count_var dd ?
triangles_count_var dd ?
points rb 100
triangles rb 100
asc_file:

file "2TORUS.ASC"
end if