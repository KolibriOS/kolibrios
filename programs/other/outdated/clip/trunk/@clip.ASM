; <--- description --->
; compiler:     FASM
; name:         Clipboard server/daemon
; version:      0.2
; author:       barsuk

; <--- include all MeOS stuff --->
include "lang.inc"
include "..\..\..\..\macros.inc"

; <--- start of MenuetOS application --->
MEOS_APP_START

;define DEBUG TRUE
define DEBUG FALSE
include "bdebug.inc"

DEFAULT_SIZE = 0x10010	; 64K + size of headers
MAX_SIZE = 0x01000000	; 16 M
MAX_FORMAT = 16
DELAY = 10		; 1/10 sec between sending attempts
ATTEMPT = 5		; number of sending attempts


; <--- start of code --->
CODE

	; во-первых, надо убить все @clip, какие найдутся:
	; демон буфера обмена должен быть только один!
	; и им окажется самый молодой из демонов...

	mov	ecx, 2	; первый номер слота для проверки
	mov	ebp, info
	mov	ebx, ebp
	mov	edx, 18
.next_process:
	mov	eax, 9
	int	0x40
	cmp	dword [ebx + 10], '@CLI'
	jnz	.differ
	cmp	byte [ebx + 14], 'P'
	jnz	.differ
.similar:
	xchg	eax, edx
	mov	ebx, 2
	int	0x40
	mov	ebx, ebp
	xchg	eax, edx
.differ:
	inc	ecx
	cmp	ecx, eax
	jae	.done		; process not found
	jmp	.next_process

.done:
	mov	eax, 68 		; init heap
	mov	ebx, 11
	int	0x40

	mov	eax, 66 		; set hotkey: Ctrl-Alt-V
	mov	ebx, 4
	mov	ecx, 47
	mov	edx, 0x110		    ; 0x110
	int	0x40
print "set hot key (1 bad)"
pregs

	mov	ebp, DEFAULT_SIZE

	mov	eax, 68 		; get memory for buffer
	mov	ebx, 12
	mov	ecx, ebp
	int	0x40
	mov	esi, IPC_buffer
	mov	[esi], eax

	mov	eax, 60 		; define buffer for IPC
	mov	ebx, 1
	mov	ecx, [esi]
	mov	edx, ebp
	int	0x40

	mov	eax, 40
	mov	ebx, 01000010b		;
	int	0x40

wait_event:				; main cycle
	mov	eax, [IPC_buffer]
	mov	dword [eax], 0		; unlock buffer
	mov	dword [eax + 4], 8

	mov	eax, 10 		; wait
	int	0x40
print "event"
pregs

	;dec    eax
	;dec    eax
	cmp	eax, 2
	jnz	ipc_event

print "hotkey"

	mov	eax, 2
	int	0x40
pregs
	cmp	ah, 47		  ; "v" up
	jnz	wait_event

print "hotkey v up"
					; it is hotkey
	mov	eax, 1
	mov	ebx, eax
	call	find_format
pregs
	cmp	eax, -1
	jz	wait_event
	mov	esi, [data_buffer + eax * 4]
	mov	edi, [data_size + eax * 4]
	mov	ecx, 2
print "ping"
.next:
	mov	eax, 72
	movzx	edx, byte [esi]
	int	0x40			; here we should pause if eax = 1
pregs

	inc	esi
	dec	edi
	jnz	.next
	jmp	wait_event

ipc_event:

;       we get an IPC message

print "recv. "

	mov	eax, [IPC_buffer]
	mov	dword [eax], 1		; lock buffer
	mov	dword [eax + 4], 8

dph1   dword [eax]		       ; lock
dph1   dword [eax + 4]		       ; current used size
dph1   dword [eax + 8]		       ; pid
dph1   dword [eax + 12] 	       ; len
print ""
dph1   dword [eax + 16] 	       ; cmd << 16 | fmt
dph1   dword [eax + 20]
dph1   dword [eax + 24]

	mov	ebx, [eax + 8 + 4]
	cmp	ebx, 8
	jb	wait_event		; all my messages have 8-byte header
					; so ignore this one

	movzx	ecx, word [eax + 8 + 8] ; command
	cmp	ecx, 1
	jz	command_setsize
	cmp	ecx, 2
	jz	command_set
	cmp	ecx, 3
	jz	command_getsize
	cmp	ecx, 4
	jz	command_get
	cmp	ecx, 5
	jz	command_delete
	jmp	wait_event		; unrecognised command

command_setsize:

; the only thing we really need to do here is to grow IPC buffer
; no changes are done to real buffer of chosen format

; the bad thing is that IPC buffer grows and never becomes less.
; i want to fix this in the next version.

print "set size"

	mov	esi, [IPC_buffer_size]
	mov	ecx, [eax + 24] 	; size
	add	ecx, 0x18		; for headers
;pregs
	cmp	esi, ecx
	jae	wait_event
print "ipc buffer resize from esi to ecx"
pregs

	mov	ebp, MAX_SIZE	; sort of protection. forbid transfer of more
	cmp	ecx, ebp	; than 16,7 Mb of data through buffer
	jbe	not_much
	mov	ecx, ebp

not_much:
	xchg	eax, edi	; edx := [IPC_buffer]
	add	ecx, 0x18	; for headers

	mov	[IPC_buffer_size], ecx
	mov	eax, 68
	mov	ebx, 12
	int	0x40		; get memory
print "get mem for new buf, eax"
pregs
	mov	[IPC_buffer], eax

	mov	dword [eax + 4], 8
	mov	edx, ecx
	mov	ecx, eax
	mov	eax, 60
	mov	ebx, 1
	int	0x40		; make it IPC buffer

	mov	ecx, edi
	mov	eax, 68
	mov	ebx, 13
	int	0x40		; free old IPC buffer

	jmp	wait_event

command_set:
print "set"

; here we put the data transfered to us to one of internal buffers

;;;outs eax + 0x18

	movzx	eax, word [eax + 18]		; format id
	call	find_format
	cmp	eax, -1
	jz	new_format
	mov	edx, eax

	; free old buffer of this format

	mov	ecx, [data_buffer + edx * 4]
	mov	eax, 68
	mov	ebx, 13
	int	0x40
	jmp	set_buffer

new_format:
	mov	eax, data_count 	; allocate a buffer
	mov	edx, [eax]
	inc	dword [eax]		; no protection, no checks. very bad :(
set_buffer:
	mov	eax, [IPC_buffer]
	mov	ecx, dword [eax + 12]
	sub	ecx, 8			; get length of data itself
					; subtract size of my headers
;pregs
	mov	[data_size + edx * 4], ecx	; save length of data

	mov	eax, 68
	mov	ebx, 12
	int	0x40
	mov	[data_buffer + edx * 4], eax

; copy data from IPC to new buffer
	mov	esi, [IPC_buffer]
	mov	ebp, esi
	add	esi, 24 		; start of data
	mov	edi, eax
	rep	movsb			; oh, me knows that it's bad

	mov	eax, ebp
	movzx	ecx, word [eax + 18]		; format id
	mov	[data_format + edx * 4], ecx

		; debug
;       print   "set debug"
;       mov     eax, [data_buffer + edx * 4]
;       mov     ebx, [data_size + edx * 4]
;       mov     ecx, [data_format + edx * 4]
;       pregs

	jmp	wait_event

command_delete:
	movzx	eax, word [eax + 18]		; format id
	cmp	eax, -1
	jz	command_clear
	call	find_format
	cmp	eax, -1
	jz	wait_event
	mov	ecx, [data_buffer + eax * 4]
	mov	[data_format + eax * 4], 0
	mov	eax, 68
	mov	ebx, 13
	int	0x40
	jmp	wait_event

command_clear:
	mov	eax, 68
	mov	ebx, 13
	mov	edx, data_buffer
.next:
	mov	ecx, [edx]
	jecxz	.nofree
	int	0x40
.nofree:
	mov	[edx + data_size - data_buffer], 0
	add	edx, 4
	cmp	edx, data_format
	jnae	.next
	jmp	wait_event

command_getsize:

print "get size"

	; we should send reply, containing size of data with chosen format id
	movzx	eax, word [eax + 18]		; format id
	mov	esi, 4
	call	find_format
;pregs
	cmp	eax, -1
	jz	zero_size

	lea	edx, [data_size + eax * 4]
	mov	eax, [edx]
	jmp	send_getsize
zero_size:
	mov	edx, dw_zero	; send 0 to indicate that the buffer is empty
send_getsize:
	jmp	send_msg

command_get:
print "get"

; we need to send the data

	; [:||||:]

	movzx	eax, word [eax + 18]		; format id
	call	find_format
	cmp	eax, -1
	jz	wait_event
	mov	edi, eax			; а нефиг

	mov	edx, [data_buffer + edi * 4]
	mov	esi, [data_size + edi * 4]
	;jmp    send_msg

send_msg:

	; for debugging
;       mov     eax, 5
;       mov     ebx, DELAY*5
;       int     0x40

	mov	ebp, ATTEMPT		; number of attempts to send
	mov	eax, [IPC_buffer]
	mov	ecx, [eax + 8]		; sender PID
	mov	eax, 60
	mov	ebx, 2
	int	0x40
print "send data result eax"
pregs
	cmp	eax, 2
	jz	.wait
	cmp	eax, 3			; it is strange..
	jz	.wait			; maybe he needs time to resize his buf
	jmp	wait_event
.wait:
	dec	ebp
	jz	wait_event
	mov	eax, 5			; sleep a bit
	mov	ebx, DELAY
	int	0x40
	jmp	send_msg


find_format:			; small function returning number of format

;print "find format"
	push	ebx
	mov	ebx, eax	; format id
	and	ebx, 0xffff	; it is word
	xor	eax, eax
next:
	cmp	[data_format + eax * 4], ebx
	jz	found
	inc	eax
	cmp	eax, MAX_FORMAT
	jb	next
not_found:
	or	eax, -1
found:
;pregs
	pop	ebx
	ret

; <--- initialised data --->
DATA
	IPC_buffer	dd	0
	IPC_buffer_size dd	DEFAULT_SIZE		; initially 64K

	info		db	1024 dup(0)		; for process info

	data_buffer	dd	MAX_FORMAT dup(0)	; buffer for some format
							; (256 formats max)
	data_format	dd	MAX_FORMAT dup(0)	; format id
	data_size	dd	MAX_FORMAT dup(0)	; size of buffer
	data_count	dd	0			; number of formats used
	dw_zero 	dd	0			; used to tell that
							; we don't have a format

; <--- uninitialised data --->
UDATA


MEOS_APP_END
; <--- end of MenuetOS application --->
