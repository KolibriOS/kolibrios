include "..\..\develop\info3ds\info_fun_float.inc"

align 4
proc @DoubleToStr$qduso uses esi edi
  cld
  lea  esi,[esp+12]
  mov  edi,Data_Double
  movsd
  movsd
  mov  ax,[esp+20]
  mov [NumberSymbolsAD],ax
  call DoubleFloat_to_String
  cmp dword[esp+24],0
  je @f
  call String_crop_0
  @@:
  mov eax,Data_String
  ret
endp

align 4
proc @StrToDouble$qpc uses esi edi
  cld
  mov edi,Data_String
  mov esi,[esp+12]
  mov ecx,32
  repnz movsb
  call String_to_DoubleFloat
  fld qword[Data_Double]
  ret
endp

;input:
; *str - указатель на строку, число должно быть в 10 или 16 ричном виде
;output:
; eax - число
align 4
proc @StrToInt$qpc uses ebx esi
	xor eax,eax
	xor ebx,ebx
	mov esi,[esp+12]

	;на случай если перед числом находятся пробелы
	@@:
	cmp byte[esi],' '
	jne @f
		inc esi
		jmp @b
	@@:

	;определение отрицательных чисел
	xor ecx,ecx
	inc ecx
	cmp byte[esi],'-'
	jne @f
		dec ecx
		inc esi
	@@:

	cmp word[esi],'0x'
	je .load_digit_16

	.load_digit_10: ;считывание 10-тичных цифр
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'9'
		jg @f
			sub bl,'0'
			imul eax,10
			add eax,ebx
			inc esi
			jmp .load_digit_10
	jmp @f

	.load_digit_16: ;считывание 16-ричных цифр
		add esi,2
	.cycle_16:
		mov bl,byte[esi]
		cmp bl,'0'
		jl @f
		cmp bl,'f'
		jg @f
		cmp bl,'9'
		jle .us1
			cmp bl,'A'
			jl @f ;отсеиваем символы >'9' и <'A'
		.us1: ;составное условие
		cmp bl,'F'
		jle .us2
			cmp bl,'a'
			jl @f ;отсеиваем символы >'F' и <'a'
			sub bl,32 ;переводим символы в верхний регистр, для упрощения их последущей обработки
		.us2: ;составное условие
			sub bl,'0'
			cmp bl,9
			jle .cor1
				sub bl,7 ;convert 'A' to '10'
			.cor1:
			shl eax,4
			add eax,ebx
			inc esi
			jmp .cycle_16
	@@:
	or ecx,ecx ;если число отрицательное
	jnz @f
		sub ecx,eax
		mov eax,ecx
	@@:
	ret
endp

align 4
proc @strlen$qpxc uses edi
  cld
  mov  edi,[esp+8]
  mov  ecx,-1
  xor  al,al
  repnz scasb
  not  ecx
  lea  eax,[ecx-1]
  ret
endp

align 4
proc @strcpy$qpcpxc uses esi edi
  cld
  mov  edi,[esp+16]
  mov  ecx,-1
  mov  esi,edi
  xor  al,al
  repnz scasb
  not  ecx
  mov  edi,[esp+12]
  mov  edx,ecx
  mov  eax,edi
  shr  ecx,2
  rep movsd
  mov  ecx,edx
  and  ecx,3
  rep movsb
  ret
endp

align 4
proc @strncpy$qpcpxcl uses esi edi
  cld
  mov  edi,[esp+16]
  mov  ecx,[esp+20]
  jecxz @f
  mov  esi,edi
  xor  al,al
  repnz scasb
  not  ecx
  mov  edi,[esp+12]
  mov  edx,ecx
  mov  eax,edi
  shr  ecx,2
  rep movsd
  mov  ecx,edx
  and  ecx,3
  rep movsb
@@:
  ret
endp

align 4
proc @memcmp$qpxvt1l uses esi edi
	cld
	mov	ecx,[esp+20]
	mov	edi,[esp+16]
	mov	esi,[esp+12]
	repe  cmpsb
	movzx eax,byte[esi-1]
	movzx edx,byte[edi-1]
	sub	eax,edx
	ret
endp

align 4
proc @memcpy$qpvpxvui uses esi edi
  cld
  mov  edi,[esp+12]
  mov  eax,edi
  mov  ecx,[esp+20]
  mov  esi,[esp+16]
  mov  edx,ecx
  shr  ecx,2
  rep movsd
  mov  ecx,edx
  and  ecx,3
  rep movsb
  ret
endp

align 4
proc @memset$qpvcui uses edi
  cld
  mov  edi,[esp+8]
  mov  al,[esp+12]
  mov  ah,al
  mov  dx,ax
  shl  eax,16
  mov  ax,dx
  mov  ecx,[esp+16]
  mov  edx,ecx
  shr  ecx,2
  rep stosd
  mov  ecx,edx
  and  ecx,3
  rep stosb
  mov  eax,[esp+4]
  ret
endp

align 4
proc @strcmp$qpxct1 uses esi edi
	cld
	xor	eax,eax
	mov	edi,[esp+16]
	mov	esi,edi
	mov	ecx,-1
	repne scasb
	not	ecx
	mov	edi,esi
	mov	esi,[esp+12]
	repe  cmpsb
	mov	al,[esi-1]
	movzx edx,byte[edi-1]
	sub	eax,edx
	ret
endp

align 4
proc @strchr$qpxci uses edi
	mov	edi,[esp+8]
	mov	edx,edi
	mov	ecx,-1
	xor	eax,eax
	cld
	repne scasb
	not	ecx
	mov	edi,edx
	mov	al,[esp+12]
	repne scasb
	jnz	@f
	lea	eax,[edi-1]
	ret
@@:
	xor	eax,eax
	ret
endp

align 4
proc @strstr$qpxct1 uses ebx esi edi
	xor  eax,eax
	mov  esi,[esp+16]
	test esi,esi
	je   .not_f
	mov  edi,[esp+20]
	test edi,edi
	je   .not_f
	mov  edx,edi
	mov	 ecx,-1
	cld
	repne scasb
	not	 ecx
	mov  ebx,ecx ;ebx = strlen(str2)
align 4
.cycle:
	cmp  byte[esi],0
	je   .not_f
	mov  edi,edx
	push esi
	mov  ecx,ebx
	repe cmpsb
	pop  esi
	inc  esi
	cmp  byte[edi-1],0
	jne  .cycle

	lea  eax,[esi-1]
.not_f:
        ret
endp

align 4
proc __ftol
  sub  esp,12
  wait
  fstcw word [esp+8]
  wait
  mov  al,[esp+9]
  or  byte [esp+9],0x0c
  fldcw word [esp+8]
  fistp qword [esp]
  mov  [esp+9],al
  fldcw word [esp+8]
  mov  eax,[esp]
  mov  edx,[esp+4]
  add  esp,12
  ret
endp

align 4
proc @floor$qd
  fld  qword [esp+4]
  mov  ax,[esp+10]
  shl  ax,1
  cmp  ax,0x8680
  ja  .floor_end
  mov  ch,4
  sub  esp,2
  wait
  fstcw word [esp]
  mov  ax,0xf3ff
  wait
  mov  dx,[esp]
  and  ax,dx
  or  ah,ch
  mov  [esp],ax
  fldcw word [esp]
  frndint
  mov  [esp],dx
  fldcw word [esp]
  add  esp,2
.floor_end:
  ret
endp

align 4
proc @MessageBox$qpc
	mov eax,[esp+4]
	notify_window_run eax
	ret
endp