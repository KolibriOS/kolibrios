
; clipboard client module by barsuk.
; version 0.2
; see readme.txt

macro CLIP_BUFFER buf_name, buf_size
{
#buf_name:
.size	dd	buf_size
.sys1	dd	?
.sys2	dd	?
.data	db	buf_size dup(?)
}

_ipc_send:
; internally used routine
; esi->buffer, edx = byte count

	push	ebx
	push	ecx
	push	ebp

	mov	ebp, ATTEMPT	; number of attempts to send
	xchg	edx, esi
.send_again:
	mov	eax, 5
	mov	ebx, SEND_DELAY 		
	int	0x40			

	mov	eax, 60
	mov	ebx, 2
	mov	ecx, [clipboard_init.clipserv_pid]
	int	0x40
print "send result eax"
pregs

	or	eax, eax
	jz	.ok
	cmp	eax, 2
	jz	.wait
	jmp	.err
.wait:
	dec	ebp
	jz	.err
	jmp	.send_again
.ok:
	mov	eax, 1
	jmp	.exit
.err:
	xor	eax, eax
.exit:
	pop	ebp
	pop	ecx
	pop	ebx
	xchg	edx, esi
	ret

_ipc_recv:
; used internally.
; edx = default event mask of thread(!!!)
; esi -> CLIP_buffer
; result: eax == 1 - suc, 0 - err

	push	ebx
	push	ecx

	mov	dword [esi + 4], 0	; unlock buffer
	mov	dword [esi + 8], 0	; 

	push	edx
	mov	eax, 60			; register buffer
	mov	ebx, 1
	lea	ecx, [esi + 4]
	mov	edx, [esi]
	add	edx, 8
;print "register buffer"
;pregs
	int	0x40
	pop	edx

.wait:
	mov	eax, 40			; listen for IPC
	mov	ebx, 01000000b
	int	0x40

	mov	eax, 23			; wait for IPC event
	mov	ebx, RECV_DELAY		; not too long
	int	0x40
	cmp	eax, 7
	jnz	.err

	mov	eax, [clipboard_init.clipserv_pid]	; not message from 
	cmp	eax, [esi + 4]				; daemon, so ignore
	jnz	.wait

	mov	dword [esi + 4], 1		; lock so it will not be spoiled

print "cli recv"
dph1	[esi]
dph1	[esi+4]
dph1	[esi+8]
dph1	[esi+12]
dph1	[esi+16]
dph1	[esi+20]
print ""

	mov	eax, 40			; restore event mask
	mov	ebx, edx
	int	0x40
	mov	eax, 1
	jmp	.exit
.err:
	xor	eax, eax
.exit:
	pop	ecx
	pop	ebx
	ret


clipboard_init:
; action: define IPC buffer and find clipserv process
; output: eax == 0 - error, eax == 1 - success

	push	ebx
	push	ecx
	push	edx

;	mov	eax, 60			; define buffer for IPC
;	mov	ebx, 1
;	mov	ecx, .IPC_real_buffer
;	mov	edx, IPC_buffer_size
;	int	0x40

;print "buffer defined"

;	mov	[.IPC_buffer], ecx

	push	ebp
	mov	ebp, 1    
.next_process:
	mov	eax, 9
	mov	ebx, .process_info
	mov	ecx, ebp
	int	0x40
	mov	ecx, eax     
	mov	ebx, .process_info + 10
	mov	eax, [ebx]
	cmp	eax, '@CLI'
	jnz	.differ
	mov	al, [ebx + 4]
	cmp	al, 'P'
	jnz	.differ
	jmp	.similar

;	mov	edx, .clipserv_name
;.compare:          
;	mov	al, [edx]
;	cmp	al, 0
;	jz	.similar
;	cmp	al, [ebx]
;	jnz	.differ 
;	inc	edx    
;	inc	ebx
;	jmp	.compare
.differ:
	inc	ebp
	cmp	ebp, ecx
	jae	.err		; process not found 
	jmp	.next_process
.similar:
;print "found server"
	mov	eax, dword [.process_info + 30]
	mov	[.clipserv_pid], eax

	mov	eax, 1
	jmp	.exit
.err:
;print "no server"
	xor	eax, eax
.exit:
	pop	ebp
	pop	edx
	pop	ecx
	pop	ebx
	ret
.clipserv_pid	dd	0
.process_info	db	1024 dup(0)
;;.clipserv_name	db	'@clip',0 
;.IPC_buffer	dd	.IPC_real_buffer	; sorry
;.IPC_real_buffer	db	IPC_buffer_size dup(0)

clipboard_write:
; action: put to clipboard data in format (ax) from buffer
; input: esi -> CLIP_buffer, ax = format id
; output: eax == 0 - error, eax == 1 - success

	push	ecx
	push	edx
	push	edi

	mov	edx, [esi]		; CLIP_buffer.size

	push	edx
	push	esi
	mov	[.msg_set_size + 2], ax
	mov	word [esi + 4 + 2], ax	; format id to low word of sys1
	mov	dword [.msg_set_size + 8], edx
	mov	esi, .msg_set_size
	mov	edx, 12
	call	_ipc_send
	pop	esi
	pop	edx
	or	eax, eax
	jz	.err

	mov	word [esi + 4], 2	; COMMAND_SET to high word of sys1
	add	esi, 4			; esi->buffer.sys1
	add	edx, 8
	call	_ipc_send
	sub	esi, 4
	or	eax, eax
	jz	.err

	mov	eax, 1
print "write success"
	jmp	.exit
.err:
print "write failed"
	xor	eax, eax
.exit:
	pop	edi
	pop	edx
	pop	ecx
	ret   	

.msg_set_size	dw	1
		dw	1
		dd	0
		dd	0

clipboard_read:
; esi -> CLIP_buffer, ax = format id
; edx - маска событий по умолчанию
; result: eax = 1 - success, 0 - general failure, 
; -1 - buffer too small
; edx = size of data

	push	ebx
	push	ecx
	push	ebp
	push	esi
	push	edi

	mov	ebp, edx

	; get size

	mov	edi, esi
	mov	esi, .msg_get_size
	mov	word [esi], 3
	mov	[esi + 2], ax
	mov	[.msg_get_buf + 2], ax
	mov	edx, 8
	call	_ipc_send
	or	eax, eax	
	jz	.err

	;;mov	edx, DEFAULT_MASK
	mov	edx, ebp
	mov	esi, .buf_for_size
	mov	dword [esi], 4
	call	_ipc_recv
	or	eax, eax
	jz	.err

	mov	eax, [esi + 12]		; space we need
	mov	edx, [edi]		; space we have
print "read size eax"
pregs
	mov	ecx, eax
	cmp	eax, edx
	ja	.size
	or	eax, eax
	jz	.err
	mov	ebx, eax

	mov	esi, .msg_get_buf
	mov	word [esi], 4
	mov	edx, 8
	call	_ipc_send
	or	eax, eax
	jz	.err

;print "send fuck"

	mov	edx, ebp
	mov	esi, edi
	call	_ipc_recv
	or	eax, eax
	jz	.err

print "read get data"

	mov	edx, ebx
	mov	eax, 1
print "read ok"
	jmp	.exit		; i'm an idiot. Never will I code at night again
				; i put jz instead of jmp.

.size:
print "buffer small"
	mov	edx, eax
	or	eax, -1
	jmp	.exit
.err:
print "read error"
	xor	eax, eax
.exit:
	pop	edi
	pop	esi
	pop	ebp
	pop	ecx
	pop	ebx
	ret

.msg_get_size	dw	0
		dw	1
		dd	0
.msg_get_buf	dw	0
		dw	1
		dd	0
.buf_for_size	dd	0
		dd	0
		dd	0
		dd	0


clipboard_delete:
; ax = format id to delete
; result: eax = 0 error, = 1 ok

	push	edx
	push	esi

	mov	esi, .msg_del
	mov	word [esi], 5
	mov	[esi + 2], ax
	mov	edx, 8

	call	_ipc_send 

	pop	esi
	pop	edx

	ret
.msg_del	dw	0
		dw	1
		dd	0

