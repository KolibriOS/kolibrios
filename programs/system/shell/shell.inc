

SC_OK	= 0
SC_EXIT = 1
SC_PUTC = 2
SC_PUTS = 3
SC_GETC = 4
SC_GETS = 5
SC_CLS	= 6
SC_PID = 7
SC_PING = 8

SHM_WRITE =  0x01
SHM_OPEN_ALWAYS = 0x04

;============================

align 4
sc_name    rb 64
sc_pid	   dd 0
sc_buffer  dd 0
sc_process dd 0

;============================

if used _sc_pid2name
align 4
_sc_pid2name:

 push esp
 push ebx

 xor ecx, ecx
 mov eax, [sc_pid]
 mov ebx, 10
@@:
 xor edx, edx
 div ebx
 push edx
 inc ecx
 test eax, eax
 jnz @b

mov edi, sc_name

@@:
 pop eax
 add al, '0'
 stosb
 loop @b


 mov al, '-'
 stosb
 mov al, 'S'
 stosb
 mov al, 'H'
 stosb
 mov al, 'E'
 stosb
 mov al, 'L'
 stosb
 mov al, 'L'
 stosb
 mov al,  0
 stosb


 pop ebx
 pop esp

 ret
end if

;============================

if used _sc_init
align 4
; void __stdcall sc_init();
_sc_init:

 push esp
 push ebx

 mov eax, 68
 mov ebx, 11
 int 0x40

 mov eax, 68 ; выделить память
 mov ebx, 12
 mov ecx, 1024
 int 0x40

 mov [sc_process], eax

 mov eax, 9 ; получить информацию о текущем процессе
 mov ebx, [sc_process]
 mov ecx, -1
 int 0x40

 mov dword eax, [ebx+30] ; получаем PID текщего процесса
 mov [sc_pid], eax

 mov eax, 68 ; освободить память
 mov ebx, 13
 mov ecx, [sc_process]
 int 0x40

 call _sc_pid2name

 mov eax, 68 ; открыть именованную область
 mov ebx, 22
 mov dword ecx, sc_name
 mov edx, 4096
 mov esi, SHM_OPEN_ALWAYS or SHM_WRITE
 int 0x40

 mov [sc_buffer], eax

 pop ebx
 pop esp

 ret
end if

;============================

if used _sc_puts
align 4
; void __stdcall sc_puts(char *str);
_sc_puts:

 push esp
 push ebx

 mov esi, [esp+12]
 mov edi, [sc_buffer]
 mov al, SC_PUTS
 stosb

@@:
 lodsb
 stosb
 test al, al
 jnz @b

 mov ebx, [sc_buffer]

@@:
 mov byte dl, [ebx]
 test dl, dl
 jz @f
 push ebx
 mov eax, 5
 mov ebx, 5
 int 0x40
 pop ebx
 jmp @b

@@:

 pop ebx
 pop esp
 ret 4
end if

;============================

if used _sc_exit
align 4
; void __stdcall sc_exit();
_sc_exit:
 push ebx
 push esp

 mov ebx, [sc_buffer]
 mov byte [ebx], SC_EXIT

@@:
 mov byte dl, [ebx]
 test dl, dl
 jz @f
 push ebx
 mov eax, 5
 mov ebx, 5
 int 0x40
 pop ebx
 jmp @b

@@:
 mov eax, 68 ;закрыть именованную область
 mov ebx, 23
 mov dword ecx, sc_name
 int 0x40

 pop esp
 pop ebx
 ret
end if


;============================

if used _sc_gets
align 4
; void __stdcall sc_gets(char *str);
_sc_gets:

 push esp
 push ebx

 mov edi, [esp+12]

 mov ebx, [sc_buffer]
 mov byte [ebx], SC_GETS

@@:
 mov byte dl, [ebx]
 test dl, dl
 jz @f
 push ebx
 mov eax, 5
 mov ebx, 5
 int 0x40
 pop ebx
 jmp @b

@@:


 mov esi, [sc_buffer]
 inc esi

@@:
 lodsb
 stosb
 test al, al
 jnz @b

 pop ebx
 pop esp
 ret 4
end if

;============================

if used _sc_pid
_sc_pid:
;int __stdcall sc_pid (void);
	push	ebx ecx

	mov	ecx, [sc_buffer]
	mov	byte [ecx], SC_PID

@@:
	mov	eax, 5
	mov	ebx, 5
	int	0x40

	cmp	byte [ecx], 0
	je	@f
	call	_sc_ping
	test	eax, eax
	jnz	.err

@@:
	mov	eax, [ecx+1]
	pop	ecx ebx
	ret

.err:
	pop	ecx ebx
	xor	eax, eax
	dec	eax
	ret
end if

;============================

if used _sc_ping
_sc_ping:
;int __stdcall sc_ping (void);
	push	ebx ecx

	mov	ecx, [sc_buffer]
	mov	byte [ecx], SC_PING

	mov	eax, 5
	mov	ebx, 200
	int	0x40

	xor	eax, eax
	cmp	byte [ecx], 0
	je	@f
	dec	eax

@@:
	pop	ecx ebx
	ret
end if
