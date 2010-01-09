use32

	org 0x0

	db 'MENUET01'
	dd 1, START, I_END, IM_END+0x1000, IM_END+0x1000, 0, 0

include '../proc32.inc'
include '../macros.inc'
include '../dll.inc'
include '../libio.inc'
include '../../../../../programs/develop/libraries/box_lib/trunk/box_lib.mac'

include '../network.inc'


filebuffer_size equ 4*4096 ; 16kb   (dont try to change it yet..)
TIMEOUT 	equ 500
buffer_len	equ 1500

AF_INET4	equ 2
IP_PROTO_UDP	equ 17

opcode_rrq	equ 1 shl 8
opcode_wrq	equ 2 shl 8
opcode_data	equ 3 shl 8
opcode_ack	equ 4 shl 8
opcode_error	equ 5 shl 8

; read/write request packet
;
;  2 bytes     string    1 byte     string   1 byte
;  ------------------------------------------------
; | Opcode |  Filename  |   0  |    Mode    |   0  |
;  ------------------------------------------------

; data packet
;
;  2 bytes     2 bytes      n bytes
;  ----------------------------------
; | Opcode |   Block #  |   Data     |
;  ----------------------------------

; acknowledgement packet
;
;  2 bytes     2 bytes
;  ---------------------
; | Opcode |   Block #  |
;  ---------------------

; error packet
;
;  2 bytes  2 bytes        string    1 byte
;  -----------------------------------------
; | Opcode |  ErrorCode |   ErrMsg   |   0  |
;  -----------------------------------------


START:

;;        mcall   68, 11

	stdcall dll.Load, @IMPORT
	or	eax, eax
	jnz	exit

stop_transfer:
	mcall	40, 00100111b

red_win:
	call draw_window

align 4
still:
	mcall	10

	dec	eax
	jz	red_win

	dec	eax
	jz	key

	dec	eax
	jz	button

	push	dword edit1
	call	[edit_box_mouse]

	push	dword edit2
	call	[edit_box_mouse]

	push	dword edit3
	call	[edit_box_mouse]

	push	dword edit4
	call	[edit_box_mouse]

	push	dword Option_boxs1
	call	[option_box_mouse]

	push	dword Option_boxs2
	call	[option_box_mouse]

	jmp	still

button:
	mcall	17

	cmp	ah,0x10
	je	start_transfer


	test	ah , ah
	jz	still

exit:	mcall	-1
key:
	mcall	2

	push	dword edit1
	call	[edit_box_key]

	push	dword edit2
	call	[edit_box_key]

	push	dword edit3
	call	[edit_box_key]

	push	dword edit4
	call	[edit_box_key]

	jmp still


align 4
draw_window:
	mcall	12,1
	mcall	0,(50*65536+400),(30*65536+180),0x34AABBCC,0x085080DD,str_title

	mcall	4,35*65536+10,0x80000000, str_server

	mov	ebx,5*65536+ 30
	mov	edx, str_source
	mcall

	mov	ebx,11*65536+ 50
	mov	edx, str_destination
	mcall

	mov	ebx,47*65536+72
	mov	edx, str_mode
	mcall

	mov	ebx,160*65536+72
	mov	edx, str_method
	mcall

	mov	ebx,270*65536+72
	mov	edx, str_blocksize
	mcall

	push	dword edit1
	call	[edit_box_draw]

	push	dword edit2
	call	[edit_box_draw]

	push	dword edit3
	call	[edit_box_draw]

	push	dword edit4
	call	[edit_box_draw]

	push	dword Option_boxs1
	call	[option_box_draw]

	push	dword Option_boxs2
	call	[option_box_draw]

	mcall	8,210*65536+170, 105*65536+16,0x00000010,0x085080DD

	mcall	4,260*65536+110, 0x80000000, str_transfer

	mcall	38,10*65536+380, 130*65536+130,0x00000000

	mcall	4,350*65536+137, 0x80000000, str_kb_s

;        mcall   47,1 shl 31 + 7 shl 16 + 1,kbps,305*65536+137,0x00000000

	mcall	4,20*65536+137, 0x80000000, [status]

;        mcall   47,1 shl 31 + 3 shl 16 + 1,done,25*65536+137,0x00000000

	mcall	12,2

	ret






start_transfer:

	; first, resolve the hostname

	push	esp	; reserve stack place

	push	esp	; fourth parameter
	push	0	; third parameter
	push	0	; second parameter
	push	SRV	; first parameter
	call	[getaddrinfo]

	pop	esi	; now we will have pointer to result in esi

; test for error
	test	eax, eax
	jnz	still

	mov	esi, [esi + addrinfo.ai_addr]
	mov	esi, [esi + sockaddr_in.sin_addr]
	mov	dword [IP], esi

	stdcall mem.Alloc, buffer_len
	test	eax, eax
	jz	stop_transfer
	mov	[packetbuff], eax

	invoke	file_open, local_addr, O_READ + O_WRITE + O_CREATE
	cmp	eax, 32
	jb	stop_transfer

	mov	[fh], eax

	mcall	socket, AF_INET4, IP_PROTO_UDP, 0		; socket_open
	cmp	eax, -1
	je	still

	mov	[socketnum], eax

	mcall	connect, [socketnum], sockaddr, sockaddr_len	; socket_connect
	cmp	eax, -1
	je	still

	mov	word [I_END], opcode_rrq
	cmp	[option_group2],op3		; method = get?
	jz	@f
	mov	word [I_END], opcode_wrq
       @@:

	xor	al , al
	mov	edi, remote_addr
	mov	ecx, 250
	repnz	scasb
	sub	edi, remote_addr
	mov	ecx, edi
	mov	edi, I_END+2
	mov	esi, remote_addr
	rep	movsb

	cmp	[option_group1], op1
	je	.ascii

	mov	esi, octet
	movsd
	movsb

	jmp	.send_request

      .ascii:

	mov	esi, netascii
	movsd
	movsd

      .send_request:

	xor	al, al
	stosb

	sub	edi, I_END
	mov	esi, edi
	mcall	send, [socketnum], I_END

	mov	[last_ack], 0

;        mcall   26, 9
;        mov     [last_time], eax

	mov	[status], str_transfering
	call	draw_window

	mcall	40, 10000101b

	cmp	[option_group2],op3		; method = get?
	jnz	send_data_loop

	invoke	file_truncate, [fh]

receive_data_loop:

	mcall	23, TIMEOUT

	dec	eax
	jz	.redraw

	dec	eax
	dec	eax
	jz	.btn

	mcall	recv, [socketnum], [packetbuff], buffer_len ; receive data

	mov	esi, [packetbuff]
	cmp	word[esi], opcode_data
	jne	.error

	mov	bx, [last_ack]
	inc	bx
	rol	bx, 8

	cmp	word [esi + 2], bx
	jne	.packet_got_lost

	inc	[last_ack]


	; now, we need to store the data

	add	esi, 4
	sub	eax, 4
	mov	ecx, eax
	invoke	file_write, [fh], esi ,ecx

	cmp	ecx, 512      ; full data packet?
	jge	.continue

	; last packet, or something else

	mov	[status], str_success

.kill_xfer:

	invoke	file_close, [fh]
	mcall	close, [socketnum]
	jmp	stop_transfer

.error:

	cmp	word[esi], opcode_error
	je	.decode_error

	jmp	.continue

.packet_got_lost:


.continue:

;        mcall   26, 9
;        mov     ebx, [last_time]
;        mov     [last_time], eax
;        xor     edx, edx
;        sub     eax, ebx
;        xchg    eax, ecx
;        div     ecx
;        mov     [kbps], eax
;        mcall   47,1 shl 31 + 7 shl 16 + 1,kbps,305*65536+137,0x40000000, 0x00ffffff

	mov	word [buffer], opcode_ack		 ; send ack
	mov	ax, [last_ack]
	rol	ax, 8
	mov	word [buffer+2], ax
	mcall	send, [socketnum], buffer, 4, 0

	jmp	receive_data_loop


.btn:
	mcall	17

	jmp	.kill_xfer

.redraw:
	call	draw_window
	jmp	receive_data_loop


.decode_error:
	movzx	esi, word[esi + 2]
	cmp	esi, 7
	cmovg	esi, [zero]

	mov	esi, dword [4*esi + error_crosslist]
	mov	[status], esi

	jmp	.kill_xfer



;--------------------------------


send_data_loop:

	mov	word[buffer], opcode_data

.read_chunk:

	inc	[last_ack]

	mov	ax, [last_ack]
	xchg	al, ah
	mov	word[buffer+2], ax

	invoke	file_read, [fh], buffer + 4, 512
	cmp	eax, -1
	je	.kill_xfer

	add	eax, 4
	mov	[packetsize], eax

.send_packet:
	mcall	send, [socketnum], buffer, [packetsize], 0	 ; send data

.loop:
	mcall	23, TIMEOUT

	dec	eax
	jz	.red

	dec	eax
	dec	eax
	jz	.btn

	mcall	recv, [socketnum], [packetbuff], buffer_len	     ; receive ack
	cmp	eax, -1
	je	.kill_xfer

	mov	esi, [packetbuff]

	cmp	word[esi], opcode_error
	je	.decode_error

	cmp	word[esi], opcode_ack
	jne	.send_packet

	mov	ax, [last_ack]
	xchg	al, ah
	cmp	word[esi+2], ax
	jne	.send_packet

	cmp	[packetsize], 512+4
	jne	.xfer_ok				      ; transfer is done



	jmp	.read_chunk

.red:
	call	draw_window
	jmp	.loop

.btn:
	mcall	17


.kill_xfer:
	mov	[status], str_fail

.xfer_done:
	invoke	file_close, [fh]
	mcall	close, [socketnum]
	jmp	stop_transfer


.xfer_ok:
	mov	[status], str_success
	jmp	.xfer_done


.decode_error:
	movzx	esi, word[esi + 2]
	cmp	esi, 7
	cmovg	esi, [zero]

	mov	esi, dword [4*esi + error_crosslist]
	mov	[status], esi

	jmp	.send_packet



;-------------------------
; DATA

socketnum      dd 0
kbps	       dd 0
done	       dd 0

sockaddr:
	dw AF_INET4
	dw 69 shl 8
IP	db 192,168,1,115
sockaddr_len = $ - sockaddr

align 16
@IMPORT:

library box_lib , 'box_lib.obj',\
	io_lib	, 'libio.obj',\
	network , 'network.obj'

import	box_lib 				,\
	edit_box_draw	 ,'edit_box'		,\
	edit_box_key	 ,'edit_box_key'	,\
	edit_box_mouse	 ,'edit_box_mouse'	,\
	version_ed	 ,'version_ed'		,\
	check_box_draw	 ,'check_box_draw'	,\
	check_box_mouse  ,'check_box_mouse'	,\
	version_ch	 ,'version_ch'		,\
	option_box_draw  ,'option_box_draw'	,\
	option_box_mouse ,'option_box_mouse'	,\
	version_op	 ,'version_op'

import	io_lib					,\
	file_find_first , 'file_find_first'	,\
	file_find_next	, 'file_find_next'	,\
	file_find_close , 'file_find_close'	,\
	file_size	, 'file_size'		,\
	file_open	, 'file_open'		,\
	file_read	, 'file_read'		,\
	file_write	, 'file_write'		,\
	file_seek	, 'file_seek'		,\
	file_tell	, 'file_tell'		,\
	file_eof?	, 'file_iseof'		,\
	file_seteof	, 'file_seteof' 	,\
	file_truncate	, 'file_truncate'	,\
	file_close	, 'file_close'

import	network 				,\
	inet_ntoa	, 'inet_ntoa'		,\
	getaddrinfo	, 'getaddrinfo' 	,\
	freeaddrinfo	, 'freeaddrinfo'



edit1 edit_box 300,80,5 ,0xffffff,0x6f9480,0,0,0,99 ,SRV,ed_focus,  13,13
edit2 edit_box 300,80,25,0xffffff,0x6a9480,0,0,0,99 ,remote_addr,ed_figure_only, 5,5
edit3 edit_box 300,80,45,0xffffff,0x6a9480,0,0,0,99 ,local_addr,ed_figure_only, 13,13
edit4 edit_box 40,340,68,0xffffff,0x6a9480,0,0,0,5 ,BLK,ed_figure_only, 3, 3

op1 option_box option_group1,80,68,6,12,0xffffff,0,0,netascii,octet-netascii
op2 option_box option_group1,80,85,6,12,0xFFFFFF,0,0,octet,get-octet

op3 option_box option_group2,210,68,6,12,0xffffff,0,0,get,put-get
op4 option_box option_group2,210,85,6,12,0xFFFFFF,0,0,put,BLK-put

option_group1	dd op1
option_group2	dd op3

Option_boxs1	dd op1,op2,0
Option_boxs2	dd op3,op4,0

str_title	db 'TFTP client for KolibriOS',0
str_server	db 'Server:',0
str_source	db 'Remote file:',0
str_destination db 'Local file:',0
str_mode	db 'Mode:',0
str_method	db 'Method:',0
str_blocksize	db 'Blocksize:',0
str_kb_s	db 'kb/s',0
str_complete	db '% complete',0
str_transfer	db 'Transfer',0
str_waiting	db 'Welcome!',0
str_transfering db 'Transfering...',0

str_success	db 'Tranfser completed sucessfully',0
str_fail	db 'Transfer failed!',0

str_error:
._0 db 'Not defined, see error message (if any).',0
._1 db 'File not found.',0
._2 db 'Access violation.',0
._3 db 'Disk full or allocation exceeded.',0
._4 db 'Illegal TFTP operation.',0
._5 db 'Unknown transfer ID.',0
._6 db 'File already exists.',0
._7 db 'No such user.',0


error_crosslist:
dd	str_error._0
dd	str_error._1
dd	str_error._2
dd	str_error._3
dd	str_error._4
dd	str_error._5
dd	str_error._6
dd	str_error._7



netascii db 'NetASCII'
octet	 db 'Octet'
get	 db 'GET'
put	 db 'PUT'

BLK	 db "512",0,0,0

last_ack dw ?

fh	 dd ?	; file handle

last_time dd ?

packetbuff	dd	?
packetsize	dd	?
status		dd	str_waiting
zero		dd	0

SRV db "192.168.1.115",0
times (SRV + 256 - $) db 0

remote_addr db "3.png",0
times (remote_addr + 256 - $) db 0

local_addr  db "/sys/test.png",0
times (local_addr + 256 - $) db 0

I_END:

buffer:
rb buffer_len

IM_END: