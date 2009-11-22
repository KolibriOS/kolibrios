use32
; standard header
	db	'MENUET01'	; signature
	dd	1		; header version
	dd	start		; entry point
	dd	i_end		; initialized size
	dd	mem		; required memory
	dd	mem		; stack pointer
	dd	0		; parameters
	dd	0		; path


BUFFERSIZE	equ 1500
; useful includes
include '../macros.inc'
purge mov,add,sub
include '../proc32.inc'
include 'dll.inc'

include '../network.inc'

; entry point
start:
; load libraries
	stdcall dll.Load, @IMPORT
	test	eax, eax
	jnz	exit
; initialize console
	push	1
	call	[con_start]
	push	title
	push	-1
	push	-1
	push	-1
	push	-1
	call	[con_init]
; main loop
	push	str1
	call	[con_write_asciiz]
main:
; write prompt
	push	str2
	call	[con_write_asciiz]
; read string
	mov	esi, s
	push	256
	push	esi
	call	[con_gets]
; check for exit
	test	eax, eax
	jz	done
	cmp	byte [esi], 10
	jz	done
; delete terminating '\n'
	push	esi
@@:
	lodsb
	test	al, al
	jnz	@b
	mov	byte [esi-2], al
	pop	esi
; resolve name
	push	esp	; reserve stack place
	push	esp	; fourth parameter
	push	0	; third parameter
	push	0	; second parameter
	push	esi	; first parameter
	call	[getaddrinfo]
	pop	esi
; test for error
	test	eax, eax
	jnz	fail

; write results
	push	str3
	call	[con_write_asciiz]
;        mov     edi, esi

; convert IP address to decimal notation
	mov	eax, [esi+addrinfo.ai_addr]
	mov	eax, [eax+sockaddr_in.sin_addr]
	mov	[sockaddr1.ip], eax
	push	eax
	call	[inet_ntoa]
; write result
	push	eax
	call	[con_write_asciiz]
; free allocated memory
	push	esi
	call	[freeaddrinfo]

	push	str4
	call	[con_write_asciiz]

	mcall	socket, AF_INET4, IPPROTO_TCP, 0
	cmp	eax, -1
	jz	fail2
	mov	[socketnum], eax

	mcall	connect, [socketnum], sockaddr1, 18

	mcall	40, 1 shl 7 ; + 7
	call	[con_cls]

	mcall	51, 1, thread, mem - 2048

mainloop:
	mcall	10

	mcall	recv, [socketnum], buffer_ptr, BUFFERSIZE, 0
	cmp	eax, -1
	je	mainloop

	mov	esi, buffer_ptr
	mov	byte [esi + eax], 0

       @@:
	cmp	byte [esi], 0xff	; 'IAC' = Interpret As Command
	jne	@f
	; TODO: parse options, for now, we will reply with 'WONT' to everything
	mov	byte [esi + 1], 252	; WONT
	add	esi, 3			; a command is always 3 bytes
	jmp	@r

       @@:
	push	esi

	cmp	esi, buffer_ptr
	je	.nocommands

	mov	edx, buffer_ptr
	sub	esi, buffer_ptr
	xor	edi, edi
	mcall	send, [socketnum]

  .nocommands:
	call	[con_write_asciiz]
	jmp	mainloop


; write newline and continue main loop
	push	str4
@@:
	call	[con_write_asciiz]
	jmp	main
fail:
	push	str5
	jmp	@b
fail2:
	push	str6
	jmp	@b

done:
	push	1
	call	[con_exit]
exit:
	mcall	-1



thread:
	mcall	40, 0
	call	[con_getch2]
	mov	byte [send_data], al
	mcall	send, [socketnum], send_data, 1
	jmp	thread

; data
title	db	'Telnet',0
str1	db	'Telnet v0.1',10,' for KolibriOS # 1250 or later. ',10,10,0
str2	db	'> ',0
str3	db	'Connecting to: ',0
str4	db	10,0
str5	db	'Name resolution failed.',10,10,0
str6	db	'Could not open socket',10,10,0
str7	db	'Got data!',10,10,0

sockaddr1:
	dw AF_INET4
.port	dw 23 shl 8
.ip	dd 0
	rb 10



; import
align 4
@IMPORT:

library network, 'network.obj', console, 'console.obj'
import	network,	\
	getaddrinfo,	'getaddrinfo',	\
	freeaddrinfo,	'freeaddrinfo', \
	inet_ntoa,	'inet_ntoa'
import	console,	\
	con_start,	'START',	\
	con_init,	'con_init',	\
	con_write_asciiz,	'con_write_asciiz',	\
	con_exit,	'con_exit',	\
	con_gets,	'con_gets',\
	con_cls,	'con_cls',\
	con_getch2,	'con_getch2'
i_end:

socketnum	dd ?
buffer_ptr	rb BUFFERSIZE
send_data	rb 100

s	rb	256
align	4
rb	4096	; stack
mem:
