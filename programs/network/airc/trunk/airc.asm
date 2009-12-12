;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                   ;;
;;    IRC CLIENT for KolibriOS                       ;;
;;                                                   ;;
;;    License: GPL / See file COPYING for details    ;;
;;    Copyright 2004 (c) Ville Turjanmaa             ;;
;;    Copyright 2009 (c) CleverMouse                 ;;
;;                                                   ;;
;;    Compile with FASM for Kolibri                  ;;
;;                                                   ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

version equ '0.6'


;__DEBUG__ equ 1
;__DEBUG_LEVEL__ equ 1

use32

		org	0x0

		db	'MENUET01'		; 8 byte id
		dd	0x01			; required os
		dd	START			; program start
		dd	initialized_size	; program image size
		dd	0x100000		; required amount of memory
		dd	0x100000
		dd	0,0

include "../../../macros.inc"
include "../../../proc32.inc"
include "../../../develop/libraries/network/network.inc"
include "dll.inc"
;include "fdo.inc"
include "eth.inc"
;include "lang.inc"

; connection statuses
STATUS_DISCONNECTED = 0 ; disconnected
STATUS_RESOLVING    = 1 ; resolving server name
STATUS_CONNECTING   = 2 ; connecting to server
STATUS_CONNECTED    = 3 ; connected
; where to display status
STATUS_X = 25 + 22*6
STATUS_Y = 183 + 7*12

; supported encodings
CP866 = 0
CP1251 = 1
UTF8 = 2
; where to display encoding
ENCODING_X = 25 + 15*6
ENCODING_Y = 183 + 3*12

def_server_name db      'kolibrios.org',0                 ; default server name

user_nick	dd	12				  ; length
		db	'kolibri_user           '	  ; string
user_nick_max = $ - user_nick - 4

user_real_name	dd	14				  ; length
		db	'KolibriOS User         '	  ; string
user_real_name_max = $ - user_real_name - 4


START:				; start of execution

    stdcall dll.Load, @IMPORT
    test eax,eax
    jnz  exit

    mov  eax,40
    mov  ebx,11000111b
    mcall
    mcall 60, 1, ipcbuf, ipcbuf.size
    mcall 9, 0xe0000, -1
    mov  eax,[ebx+process_information.PID]
    mov  [main_PID],eax

	mov	esi, def_server_name
	mov	edi, irc_server_name
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b

    mov  edi,I_END
    mov  ecx,60*120
    mov  al,32
    cld
    rep  stosb

    mov  eax,[rxs]
    imul eax,11
    mov  [pos],eax

    mov  ebp,0
    mov  edx,I_END

redraw: 			; redraw
    call draw_window		; at first, draw the window

still:

    mov  eax,10                 ; wait here for event
    mcall

    dec  eax                    ; redraw
    je	 redraw
    dec  eax			; key
    je	 main_window_key
    dec  eax			; button
    je	 button
    cmp  al,4
    jz   ipc

    call process_network_event

    cmp  [I_END+120*60],byte 1
    jne  no_main_update
    mov  [I_END+120*60],byte 0
    mov  edx,I_END
    call draw_channel_text
  no_main_update:

    call print_channel_list

    jmp  still

button: 			; button

    mov  eax,17 		; get id
    mcall

    cmp  ah,1			; close program
    jne  noclose
exit:
    or   eax,-1
    mcall
  noclose:
    cmp  ah,21
    jne  no_change_encoding
    cmp  byte[edx-1],0
    jnz  still
    mov  eax,[encoding]
    inc  eax
    mov  edx,msgbox_struct
    mov  byte[edx],al
    mov  byte[edx-1],1 ; msgbox is running
    push mb_stack
    push edx
    call [mb_create]
    push msgbox_func_array
    call [mb_setfunctions]
    jmp  still
  no_change_encoding:

    call socket_commands

    jmp  still

ipc:
    mov  edx,msgbox_struct
    cmp  byte[edx-1],0
    jz   @f
    mov  byte[edx-1],0
    mov  al,[edx]
    dec  eax
    mov  byte[encoding],al
    call update_encoding
    jmp  ipc_done
@@:
    call process_command
ipc_done:
    mov  dword [ipcbuf+4], 8
    jmp  still

main_window_key:

    mov  eax,2
    mcall

    shr  eax,8

    cmp  eax,8
    jne  no_bks2
    cmp  [xpos],0
    je	 still
    dec  [xpos]
    call print_entry
    jmp  still
   no_bks2:

    cmp  eax,20
    jbe  no_character2
    mov  ebx,[xpos]
    mov  [send_string+ebx],al
    inc  [xpos]
    cmp  [xpos],80
    jb	 noxposdec
    mov  [xpos],79
  noxposdec:
    call print_entry
    jmp  still
  no_character2:

    cmp  eax,13
    jne  no_send2
    cmp  [xpos],0
    je	 no_send2
    cmp  [send_string],byte '/'   ; server command
    jne  no_send2
    call process_command
    jmp  still
  no_send2:

    jmp  still


socket_commands:

    cmp  ah,22	     ; connect
    jnz  tst3

; ignore if status is not "disconnected"
	cmp	[status], STATUS_DISCONNECTED
	jnz	.nothing

; start name resolving
	inc	[status]	; was STATUS_DISCONNECTED, now STATUS_RESOLVING
	push	gai_reqdata
	push	ip_list
	push	0
	push	0
	push	irc_server_name
	call	[getaddrinfo_start]
	test	eax, eax
	jns	getaddrinfo_done
	call	update_status
.nothing:
	ret

  tst3:


    cmp  ah,23	      ; write userinfo
    jnz  tst4

; ignore if status is not "connected"
	cmp	[status], STATUS_CONNECTED
	jnz	.nothing

; create packet in packetbuf
	mov	edi, packetbuf
	mov	edx, edi
	mov	esi, string0
	mov	ecx, string0l-string0
	rep	movsb
	mov	esi, user_real_name+4
	mov	ecx, [esi-4]
	rep	movsb
	mov	al, 13
	stosb
	mov	al, 10
	stosb
	mov	esi, string1
	mov	ecx, string1l-string1
	rep	movsb
	mov	esi, user_nick+4
	mov	ecx, [esi-4]
	rep	movsb
	mov	al, 13
	stosb
	mov	al, 10
	stosb
; send packet
	xchg	edx, edi
	sub	edx, edi
	mov	esi, edi
	mcall	53, 7, [socket]
    .nothing:
	ret

  tst4:


    cmp  ah,24	   ; close socket
    jz   disconnect
  no_24:


    ret

getaddrinfo_done:
; The address resolving is done.
; If eax is zero, address is resolved, otherwise there was some problems.
	test	eax, eax
	jz	.good
.disconnect:
; Change status to "disconnected" and return.
	and	[status], 0
	call	update_status
	ret
.good:
; We got a list of IP addresses. Try to connect to first of them.
	mov	eax, [ip_list]
	mov	esi, [eax + addrinfo.ai_addr]
	mov	esi, [esi + sockaddr_in.sin_addr]
	push	eax
	call	[freeaddrinfo]
	mcall	53, 5, 0, 6667, , 1
	cmp	eax, -1
	jz	.disconnect
; Socket has been opened. Save handle and change status to "connecting".
	mov	[socket], eax
	inc	[status]	; was STATUS_RESOLVING, now STATUS_CONNECTING
	call	update_status
	ret

process_network_event:
; values for status: 0, 1, 2, 3
	mov	eax, [status]
	dec	eax
; 0 = STATUS_DISCONNECTED - do nothing
; (ignore network events if we are disconnected from network)
	js	.nothing
; 1 = STATUS_RESOLVING
	jz	.resolving
; 2 = STATUS_CONNECTING
	dec	eax
	jz	.connecting
; 3 = STATUS_CONNECTED
	jmp	.connected
.resolving:
; We are inside address resolving. Let the network library work.
	push	ip_list
	push	gai_reqdata
	call	[getaddrinfo_process]
; Negative returned value means that the resolving is not yet finished,
; and we continue the loop without status change.
; Zero and positive values are handled by getaddrinfo_done.
	test	eax, eax
	jns	getaddrinfo_done
.nothing:
	ret
.connecting:
; We are connecting to the server, and socket status has changed.
	mcall	53, 6, [socket]
; Possible values for status: SYN_SENT=2, SYN_RECEIVED=3, ESTABLISHED=4, CLOSE_WAIT=7
; First two mean that we are still connecting, and we must continue wait loop
; without status change.
; Last means that server has immediately closed the connection,
; and status becomes "disconnected".
	cmp	eax, 4
	jb	.nothing
	jz	.established
	and	[status], 0
	call	update_status
; close socket
	mcall	53, 8
	ret
.established:
; The connection has been established, change status from "connecting" to "connected".
	inc	[status]
	call	update_status
; Fall through to .connected, because some data can be already in buffer.
.connected:
	call	read_incoming_data
; Handle closing socket by the server.
	mcall	53, 6, [socket]
	cmp	eax, 4
	jnz	disconnect
	ret

disconnect:
; Release all allocated resources.
; Exact actions depend on current status.
	mov	eax, [status]
	dec	eax
; 0 = STATUS_DISCONNECTED - do nothing
	js	.nothing
; 1 = STATUS_RESOLVING
	jz	.resolving
; 2 = STATUS_CONNECTING, 3 = STATUS_CONNECTED
; In both cases we should close the socket.
	mcall	53, 8, [socket]
	jmp	.disconnected
.resolving:
; Let the network library handle abort of resolving process.
	push	gai_reqdata
	call	[getaddrinfo_abort]
.disconnected:
; In all cases, set status to "disconnected".
	and	[status], 0
	call	update_status
.nothing:
	ret

msgbox_notify:
	inc	byte [msgbox_running]
	mcall	60,2,[main_PID],0,1
	ret

print_channel_list:

    pusha

    mov  eax,13
    mov  ebx,415*65536+6*13
    mov  ecx,27*65536+12*10
    mov  edx,0xffffff
    mcall

    mov  eax,4
    mov  ebx,415*65536+27
    mov  ecx,[index_list_1]
    mov  edx,channel_list+32
  newch:
    movzx esi,byte [edx+31]
    and  esi,0x1f
    mcall
    add  edx,32
    add  ebx,12
    cmp  edx,channel_list+32*10
    jbe  newch

  no_channel_list:

    popa

    ret


print_user_list:

    pusha

  newtry:

    mov  edx,ebp
    imul edx,120*80
    add  edx,120*60+8+I_END
    cmp  [edx],byte 1
    je	 nonp

    mov  edx,ebp
    imul edx,120*80
    add  edx,120*70+I_END
    mov  edi,edx

    mov  eax,[edx-8]
    mov  ebx,[edx-4]
    add  ebx,edx
    sub  ebx,3
    inc  eax
    dec  edx
  newnss:
    inc  edx
    dec  eax
    jz	 startuu
  asdf:
    cmp  [edx],word '  '
    jne  nodouble
    inc  edx
  nodouble:
    cmp  [edx],byte ' '
    je	 newnss
    inc  edx
    cmp  edx,ebx
    jbe  asdf
    dec  dword [edi-8]

    popa
    ret

  startuu:

    cmp  [edx],byte ' '
    jne  startpr
    inc  edx
  startpr:

    pusha
    mov  eax,13
    mov  ebx,415*65536+6*13
    mov  ecx,27*65536+12*10
    mov  edx,0xffffff
    mcall
    popa

    mov  eax,4
    mov  ebx,415*65536+27

    mov  ebp,0
  newuser:

    mov  esi,0
  newusers:
    cmp  [edx+esi],byte ' '
    je	 do_print
    inc  esi
    cmp  esi,20
    jbe  newusers
  do_print:

    mov  ecx,[index_list_1]
    cmp  [edx],byte '@'
    jne  no_op
    mov  ecx,[index_list_2]
  no_op:

    mcall

    inc  ebp
    cmp  ebp,10
    je	 nonp

    add  ebx,12

    add  edx,esi

    inc  edx
    cmp  [edx],byte ' '
    jne  newuser
    inc  edx
    jmp  newuser

  nonp:

    popa

    ret


start_user_list_at dd 0x0

recode_to_cp866:
	rep	movsb
	ret

recode_to_cp1251:
	xor	eax, eax
	jecxz	.nothing
  .loop:
	lodsb
	cmp	al,0x80
	jb	@f
	mov	al,[cp866_table-0x80+eax]
    @@: stosb
	loop	.loop
  .nothing:
	ret

recode_to_utf8:
	jecxz	.nothing
  .loop:
	lodsb
	cmp	al, 0x80
	jb	.single_byte
	and	eax, 0x7F
	mov	ax, [utf8_table+eax*2]
	stosw
	loop	.loop
	ret
  .single_byte:
	stosb
	loop	.loop
  .nothing:
	ret

recode:
	mov	eax, [encoding]
	jmp	[recode_proc+eax*4]

process_command:

    pusha

    mov  eax,[xpos]
    mov  [send_string+eax+0],byte 13
    mov  [send_string+eax+1],byte 10

    mov  eax,[rxs]
    imul eax,11
    mov  [pos],eax
    mov  eax,[send_to_channel]
    imul eax,120*80
    add  eax,I_END
    mov  [text_start],eax

    cmp  [send_string],byte '/'   ; server command
    je	 server_command

; Ignore data commands when not connected.
	cmp	[status], STATUS_CONNECTED
	jnz	sdts_ret

    mov  bl,13
    call print_character
    mov  bl,10
    call print_character
    mov  bl,'<'
    call print_character

    mov  esi,user_nick+4
    mov  ecx,[user_nick]
  newnp:
    mov  bl,[esi]
    call print_character
    inc  esi
    loop newnp

    mov  bl,'>'
    call print_character
    mov  bl,' '
    call print_character

    mov  ecx,[xpos]
    mov  esi,send_string
  newcw:
    mov  bl,[esi]
    call print_character
    inc  esi
    loop newcw

    mov  eax,dword [send_to_channel]
    shl  eax,5
    add  eax,channel_list
    mov  esi,eax

    mov  edi,send_string_header+8
    movzx ecx,byte [eax+31]
    cld
    rep  movsb

    mov  [edi],word ' :'

    mov   esi, send_string_header
    mov   ecx,10
    movzx ebx,byte [eax+31]
    add   ecx,ebx

    mov   edi, packetbuf
    rep   movsb

    mov  esi,send_string
    mov  ecx,[xpos]
    inc  ecx

	call	recode

	mov	esi, packetbuf
	mov	edx, edi
	sub	edx, esi
	mcall	53, 7, [socket]

	mov	[xpos], 0
    jmp  sdts_ret

  server_command:

    cmp  [send_string+1],dword 'anic'
    jne  no_set_nick

    mov  ecx,[xpos]
    sub  ecx,7
    cmp  ecx,user_nick_max
    jb   @f
    mov  ecx,user_nick_max
  @@:
    mov  [user_nick],ecx

    mov  esi,send_string+7
    mov  edi,user_nick+4
    cld
    rep  movsb

    pusha
    mov  edi,text+70*1+15
    mov  al,32
    mov  ecx,15
    cld
    rep  stosb
    popa

    mov  esi,user_nick+4
    mov  edi,text+70*1+15
    mov  ecx,[esi-4]
    cld
    rep  movsb

    mov  [xpos],0
    call draw_window

    popa
    ret

  no_set_nick:

    cmp  [send_string+1],dword 'area'
    jne  no_set_real_name

    mov  ecx,[xpos]
    sub  ecx,7
    cmp  ecx,user_real_name_max
    jb   @f
    mov  ecx,user_real_name_max
  @@:
    mov  [user_real_name],ecx

    mov  esi,send_string+7
    mov  edi,user_real_name+4
    cld
    rep  movsb

    pusha
    mov  edi,text+70*0+15
    mov  al,32
    mov  ecx,15
    cld
    rep  stosb
    popa

    mov  esi,user_real_name+4
    mov  edi,text+70*0+15
    mov  ecx,[esi-4]
    rep  movsb

    mov  [xpos],0
    call draw_window

    popa
    ret

  no_set_real_name:

    cmp  [send_string+1],dword 'aser'
    jne  no_set_server

    mov  ecx,[xpos]
    sub  ecx,7

    mov  esi,send_string+7
    mov  edi,irc_server_name
    rep  movsb
    mov  al,0
    stosb

    pusha
    mov  edi,text+70*2+15
    mov  al,32
    mov  ecx,15
    cld
    rep  stosb
    popa

    mov  ecx,[xpos]
    sub  ecx,7
    mov  esi,send_string+7
    mov  edi,text+70*2+15
    rep  movsb

    mov  [xpos],0
    call draw_window

    popa
    ret

   no_set_server:

; All other commands require a connection to the server.
	cmp	[status], STATUS_CONNECTED
	jnz	sdts_ret


    cmp  [send_string+1],dword 'quer'
    jne  no_query_create

    mov  edi,I_END+120*80
    mov  eax,1 ; create channel window - search for empty slot
   newse2:
    mov  ebx,eax
    shl  ebx,5
    cmp  dword [channel_list+ebx],dword '    '
    je	 free_found2
    add  edi,120*80
    inc  eax
    cmp  eax,[max_windows]
    jb	 newse2

  free_found2:

    mov  edx,send_string+7

    mov  ecx,[xpos]
    sub  ecx,7
    mov  [channel_list+ebx+31],cl

    call create_channel_name

    push edi
    push eax
    mov  [edi+120*60+8],byte 1 ; query window
    mov  al,32
    mov  ecx,120*60
    cld
    rep  stosb
    pop  eax
    pop  edi

    ; eax has the free position
;    mov  [thread_screen],edi
    call create_channel_window

    mov  [xpos],0

    popa
    ret

  no_query_create:


    mov  esi, send_string+1
    mov  ecx, [xpos]
    inc  ecx
    mov  edi, packetbuf
    call recode
    mov  esi, packetbuf
    mov  edx, edi
    sub  edx, esi

    mov  eax, 53      ; write server command
    mov  ebx, 7
    mov  ecx, [socket]
    mcall

  send_done:

    mov  [xpos],0

    cmp  [send_string+1],dword 'quit'
    jne  no_quit_server
    mov  eax,5
    mov  ebx,200
    mcall

    mov  eax, 53      ; close socket
    mov  ebx, 8
    mov  ecx, [socket]
    mcall

    mov  ecx,[max_windows]
    mov  edi,I_END
  newclose:
    mov  [edi+120*60+4],byte  1
    call notify_channel_thread
    add  edi,120*80
    loop newclose

    popa
    ret

  no_quit_server:

  sdts_ret:

    popa
    ret

get_next_byte:
; Load next byte from the packet, translating to cp866 if necessary
; At input esi = pointer to data, edx = limit of data
; Output is either (translated) byte in al with CF set or CF cleared.
	mov	eax, [encoding]
	jmp	[get_byte_table+eax*4]

get_byte_cp866:
	cmp	esi, edx
	jae	.nothing
	lodsb
.nothing:
	ret

get_byte_cp1251:
	cmp	esi, edx
	jae	.nothing
	lodsb
	cmp	al, 0x80
	jb	@f
	and	eax, 0x7F
	mov	al, [cp1251_table+eax]
@@:
	stc
.nothing:
	ret

get_byte_utf8:
; UTF8 decoding is slightly complicated.
; One character can occupy one or more bytes.
; The boundary in packets theoretically can be anywhere in data,
; so this procedure keeps internal state between calls and handles
; one byte at a time, looping until character is read or packet is over.
; Globally, there are two distinct tasks: decode byte sequence to unicode char
; and convert this unicode char to our base encoding (that is cp866).
; 1. Check that there are data.
	cmp	esi, edx
	jae	.nothing
; 2. Load byte.
	lodsb
	movzx	ecx, al
; 3. Bytes in an UTF8 sequence can be of any of three types.
; If most significant bit is cleared, sequence is one byte and usual ASCII char.
; First byte of a sequence must be 11xxxxxx, other bytes are 10yyyyyy.
	and	al, 0xC0
	jns	.single_byte
	jp	.first_byte
; 4. This byte is not first in UTF8 sequence.
; 4a. Check that the sequence was started. If no, it is invalid byte
; and we simply ignore it.
	cmp	[utf8_bytes_rest], 0
	jz	get_byte_utf8
; 4b. Otherwise, it is really next byte and it gives some more bits of char.
	mov	eax, [utf8_char]
	shl	eax, 6
	lea	eax, [eax+ecx-0x80]
; 4c. Decrement number of bytes rest in the sequence.
; If it goes to zero, character is read, so return it.
	dec	[utf8_bytes_rest]
	jz	.got_char
	mov	[utf8_char], eax
	jmp	get_byte_utf8
; 5. If the byte is first in UTF8 sequence, calculate the number of leading 1s
; - it equals total number of bytes in the sequence; some other bits rest for
; leading bits in the character.
.first_byte:
	mov	eax, -1
@@:
	inc	eax
	add	cl, cl
	js	@b
	mov	[utf8_bytes_rest], eax
	xchg	eax, ecx
	inc	ecx
	shr	al, cl
	mov	[utf8_char], eax
	jmp	get_byte_utf8
; 6. If the byte is ASCII char, it is the character.
.single_byte:
	xchg	eax, ecx
.got_char:
; We got the character, now abandon a possible sequence in progress.
	and	[utf8_bytes_rest], 0
; Now second task. The unicode character is in eax, and now we shall convert it
; to cp866.
	cmp	eax, 0x80
	jb	.done
; 0x410-0x43F -> 0x80-0xAF, 0x440-0x44F -> 0xE0-0xEF, 0x401 -> 0xF0, 0x451 -> 0xF1
	cmp	eax, 0x401
	jz	.YO
	cmp	eax, 0x451
	jz	.yo
	cmp	eax, 0x410
	jb	.unrecognized
	cmp	eax, 0x440
	jb	.part1
	cmp	eax, 0x450
	jae	.unrecognized
	sub	al, (0x40-0xE0) and 0xFF
	ret
.part1:
	sub	al, 0x10-0x80
.nothing:
.done:
	ret
.unrecognized:
	mov	al, '?'
	stc
	ret
.YO:
	mov	al, 0xF0
	stc
	ret
.yo:
	mov	al, 0xF1
	stc
	ret

read_incoming_data:
	pusha
.packetloop:
.nextpacket:
	mcall	53, 11, [socket], packetbuf, 1024
	test	eax, eax
	jz	.nothing
	mov	esi, edx	; esi = pointer to data
	add	edx, eax	; edx = limit of data
.byteloop:
	call	get_next_byte
	jnc	.nextpacket
	cmp	al, 10
	jne	.no_start_command
	mov	[cmd], 1
.no_start_command:
	cmp	al, 13
	jne	.no_end_command
	mov	ebx, [cmd]
	mov	byte [ebx+command-2], 0
	call	analyze_command
	mov	edi, command
	mov	ecx, 250
	xor	eax, eax
	rep	stosb
	mov	[cmd], eax
	mov	al, 13
.no_end_command:
	mov	ebx, [cmd]
	cmp	ebx, 512
	jge	@f
	mov	[ebx+command-2], al
	inc	[cmd]
@@:
	jmp	.byteloop
.nothing:
	popa
	ret


create_channel_name:

    pusha

  search_first_letter:
    cmp  [edx],byte ' '
    jne  first_letter_found
    inc  edx
    jmp  search_first_letter
  first_letter_found:

    mov  esi,edx
    mov  edi,channel_list
    add  edi,ebx
    mov  ecx,30
    xor  eax,eax
  newcase:
    mov  al,[esi]
    cmp  eax,'a'
    jb	 nocdec
    cmp  eax,'z'
    jg	 nocdec
    sub  al,97-65
  nocdec:
    mov  [edi],al
    inc  esi
    inc  edi
    loop newcase

    popa

    ret


create_channel_window:

    pusha

    mov  [cursor_on_off],0

;    mov  [thread_nro],eax

    mov  edx,[thread_stack]
    sub  edx,8
    mov  [edx],eax
    mov  [edx+4],edi
    mov  eax,51
    mov  ebx,1
    mov  ecx,channel_thread
    mcall
    mov  [edi+120*60+12], eax

    add  [thread_stack],0x4000
;    add  [thread_screen],120*80

    popa

    ret


print_entry:

    pusha

    mov  eax,13
    mov  ebx,8*65536+6*80
    mov  ecx,151*65536+13
    mov  edx,0xffffff
    mcall

    mov  eax,4
    mov  ebx,8*65536+154
    mov  ecx,0x000000
    mov  edx,send_string
    mov  esi,[xpos]
    mcall

    popa

; Fall through to draw_cursor.
;    ret

draw_cursor:

    pusha

    mov  eax,9
    mov  ebx,0xe0000
    mov  ecx,-1
    mcall

    cmp  ax,word [0xe0000+4]
    setnz dl
    movzx edx,dl
    neg edx
    and edx,0xffffff
;    jne  no_blink

;    call print_entry

    mov  ebx,[xpos]
    imul ebx,6
    add  ebx,8
    mov  cx,bx
    shl  ebx,16
    mov  bx,cx
    mov  ecx,151*65536+163
    mov  eax,38
    mcall

    popa

    ret

;  no_blink:
;
;    mov  eax,13
;    mov  ebx,8*65536+6*60
;    mov  ecx,151*65536+13
;    mov  edx,0xffffff
;    mcall

    popa

    ret





set_channel:

    pusha

    ; UPPER / LOWER CASE CHECK

    mov  esi,eax
    mov  edi,channel_temp
    mov  ecx,40
    xor  eax,eax
  newcase2:
    mov  al,[esi]
    cmp  eax,'#'
    jb	 newcase_over2
    cmp  eax,'a'
    jb	 nocdec2
    cmp  eax,'z'
    jg	 nocdec2
    sub  al,97-65
  nocdec2:
    mov  [edi],al
    inc  esi
    inc  edi
    loop newcase2
  newcase_over2:
    sub  edi,channel_temp
    mov  [channel_temp_length],edi

    mov  eax,channel_temp

    mov  [text_start],I_END+120*80
    mov  ebx,channel_list+32
    mov  eax,[eax]

    mov  edx,[channel_temp_length]

  stcl1:
    cmp  dl,[ebx+31]
    jne  notfound

    pusha
    xor  eax,eax
    xor  edx,edx
    mov  ecx,0
  stc4:
    mov  dl,[ebx+ecx]
    mov  al,[channel_temp+ecx]
    cmp  eax,edx
    jne  notfound2
    inc  ecx
    cmp  ecx,[channel_temp_length]
    jb	 stc4
    popa

    jmp  found

  notfound2:
    popa

  notfound:
    add  [text_start],120*80
    add  ebx,32
    cmp  ebx,channel_list+19*32
    jb	 stcl1

    mov  [text_start],I_END

  found:

    popa

    ret


channel_temp:	      times   100   db	 0
channel_temp_length   dd      0x0



print_nick:

    pusha

    mov  eax,command+1
    mov  dl,'!'
    call print_text

    popa
    ret


analyze_command:

    pusha

    mov  [text_start],I_END
    mov  ecx,[rxs]
    imul ecx,11
    mov  [pos],ecx

;    mov  bl,13
;  call print_character
;    mov  bl,10
;  call print_character

;    mov  ecx,[cmd]
;    sub  ecx,2
;    mov  esi,command+0
;  newcmdc:
;    mov  bl,[esi]
;  call print_character
;    inc  esi
;    loop newcmdc

    mov   edx,I_END
;  call  draw_channel_text

;    cmp  [cmd],20
;    jge  cmd_len_ok
;
;    mov  [cmd],0
;
;    popa
;    ret


  cmd_len_ok:

    cmp  [command],dword 'PING'  ; ping response
    jne  no_ping_responce

    call print_command_to_main

    mov  [command],dword 'PONG'

    call print_command_to_main

    mov  eax,4
    mov  ebx,100*65536+3
    mov  ecx,0xffffff
    mov  edx,command
    mov  esi,[cmd]
    mov  [command+esi-1],word '**'
;    mcall

    mov  eax,53
    mov  ebx,7
    mov  ecx,[socket]
    mov  edx,[cmd]
    mov  esi,command
    mov  word [esi+edx-2], 0x0a0d
    mcall

    popa
    ret

  no_ping_responce:

    mov  eax,[rxs]
    imul eax,11
    mov  [pos],eax

    mov  [command],byte '<'

    mov  eax,command
    mov  ecx,100
   new_blank:
    cmp  [eax],byte ' '
    je	 bl_found
    inc  eax
    loop new_blank
    mov  eax,50
  bl_found:

    inc  eax
    mov  [command_position],eax

    mov  esi,eax
    mov  edi,irc_command
    mov  ecx,8
    cld
    rep  movsb


    cmp  [irc_command],'PRIV'  ; message to channel
    jne  no_privmsg

    ; compare nick

    mov  eax,[command_position]
    add  eax,8
    call compare_to_nick
    cmp  [cresult],0
    jne  no_query_msg
    mov  eax,command+1
  no_query_msg:
    call set_channel

    mov  ecx,100 ; [cmd]
    mov  eax,command+10
  acl3:
    cmp  [eax],byte ':'
    je	 acl4
    inc  eax
    loop acl3
    mov  eax,10
  acl4:
    inc  eax

    cmp  [eax+1],dword 'ACTI'
    jne  no_action
    push eax
    mov  eax,action_header_short
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    call print_text
    mov  bl,' '
    call print_character
    pop  eax
    add  eax,8
    mov  dl,0
    call print_text
    call notify_channel_thread
    popa
    ret

  no_action:

    push eax
    mov  bl,10
    call print_character
    mov  eax,command
    mov  dl,'!'
    call print_text
    mov  bl,'>'
    call print_character
    mov  bl,' '
    call print_character
    pop  eax

    mov  dl,0
    call print_text
    call notify_channel_thread

    popa
    ret

  no_privmsg:


    cmp  [irc_command],'PART'	 ; channel leave
    jne  no_part

    ; compare nick

    mov  eax,command+1
    call compare_to_nick
    cmp  [cresult],0
    jne  no_close_window

    mov  eax,[command_position]
    add  eax,5
    call set_channel

    mov  edi,[text_start]
    mov  [edi+120*60+4],byte 1
    call notify_channel_thread

    popa
    ret

  no_close_window:

    mov  eax,[command_position]
    add  eax,5
    call set_channel

    mov  eax,action_header_red
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    mov  cl,' '
    call print_text
    mov  eax,has_left_channel
    mov  dl,0
    call print_text
    mov  eax,[command_position]
    add  eax,5
    mov  dl,' '
    call print_text
    call notify_channel_thread

    popa
    ret

  no_part:


    cmp  [irc_command],'JOIN'	 ; channel join
    jne  no_join

    ; compare nick

    mov  eax,command+1
    call compare_to_nick
    cmp  [cresult],0
    jne  no_new_window

    mov  edi,I_END+120*80
    mov  eax,1 ; create channel window - search for empty slot
   newse:
    mov  ebx,eax
    shl  ebx,5
    cmp  dword [channel_list+ebx],dword '    '
    je	 free_found
    add  edi,120*80
    inc  eax
    cmp  eax,[max_windows]
    jb	 newse

  free_found:

    mov  edx,[command_position]
    add  edx,6

    push eax
    push edx
    mov  ecx,0
   finde:
    inc  ecx
    inc  edx
    movzx eax,byte [edx]
    cmp  eax,'#'
    jge  finde
    mov  [channel_list+ebx+31],cl
    pop  edx
    pop  eax

    call create_channel_name

    push edi
    push eax
    mov  [edi+120*60+8],byte 0 ; channel window
    mov  al,32
    mov  ecx,120*60
    cld
    rep  stosb
    pop  eax
    pop  edi

    ; eax has the free position
;    mov  [thread_screen],edi
    call create_channel_window

  no_new_window:

    mov  eax,[command_position]
    add  eax,6
    call set_channel

    mov  eax,action_header_blue
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    mov  cl,' '
    call print_text

    mov  eax,joins_channel
    mov  dl,0
    call print_text

    mov  eax,[command_position]
    add  eax,6
    mov  dl,0
    call print_text
    call notify_channel_thread

    popa
    ret

  no_join:


    cmp  [irc_command],'NICK'	   ; nick change
    jne  no_nick_change

	add	[command_position], 6
; test for change of my nick
	mov	esi, command+1
	mov	edi, user_nick+4
	mov	ecx, [edi-4]
	repz	cmpsb
	jnz	.notmy
	cmp	byte [esi], '!'
	jnz	.notmy
; yes, this is my nick, set to new
	mov	esi, [command_position]
	or	ecx, -1
	mov	edi, esi
	xor	eax, eax
	repnz	scasb
	not	ecx
	dec	ecx
	cmp	ecx, user_nick_max
	jb	@f
	mov	ecx, user_nick_max
@@:
	mov	edi, user_nick+4
	mov	[edi-4], ecx
	rep	movsb

	mov	edi, text+70*1+15
	mov	al, ' '
	mov	cl, 15
	push	edi
	rep	stosb
	pop	edi
	mov	esi, user_nick+4
	mov	ecx, [esi-4]
	cmp	ecx, 15
	jb	@f
	mov	ecx, 15
@@:
	rep	movsb
	mov	[xpos], 0
	call	draw_window
.notmy:
; replace nick in all lists of users
	mov	ebx, I_END + 120*70
.channels:
	mov	esi, ebx
	mov	edx, [esi-4]
	add	edx, esi
.nicks:
	mov	edi, command+1
	cmp	byte [esi], '@'
	jnz	@f
	inc	esi
@@:
	cmp	esi, edx
	jae	.srcdone
	lodsb
	cmp	al, ' '
	jz	.srcdone
	scasb
	jz	@b
@@:
	cmp	esi, edx
	jae	.nextchannel
	lodsb
	cmp	al, ' '
	jnz	@b
.nextnick:
	cmp	esi, edx
	jae	.nextchannel
	lodsb
	cmp	al, ' '
	jz	.nextnick
	dec	esi
	jmp	.nicks
.srcdone:
	cmp	byte [edi], '!'
	jnz	.nextnick
; here we have esi -> end of nick which must be replaced to [command_position]+6
	lea	edx, [edi-command-1]
	sub	esi, edx
	or	ecx, -1
	xor	eax, eax
	mov	edi, [command_position]
	repnz	scasb
	not	ecx
	dec	ecx
	push	ecx
	cmp	ecx, edx
	jb	.decrease
	jz	.copy
.increase:
; new nick is longer than the old
	push	esi
	lea	edi, [ebx+120*10]
	lea	esi, [edi+edx]
	sub	esi, ecx
	mov	ecx, esi
	sub	ecx, [esp]
	dec	esi
	dec	edi
	std
	rep	movsb
	cld
	pop	esi
	jmp	.copy
.decrease:
; new nick is shorter than the old
	push	esi
	lea	edi, [esi+ecx]
	add	esi, edx
	lea	ecx, [ebx+120*10]
	sub	ecx, edi
	rep	movsb
	pop	esi
.copy:
; copy nick
	mov	edi, esi
	dec	edi
	mov	esi, [command_position]
	pop	ecx
	sub	edx, ecx
	sub	[ebx-4], edx
	rep	movsb
	mov	al, ' '
	stosb
.nextchannel:
	add	ebx, 120*80
	cmp	ebx, I_END + 120*70 + 120*80*19
	jb	.channels

    mov  [text_start],I_END
    add  [text_start],120*80

 new_all_channels3:

    mov  eax,action_header_short
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    call print_text
    mov  eax,is_now_known_as
    mov  dl,0
    call print_text
    mov  eax,[command_position]
    mov  dl,0
    call print_text
    call notify_channel_thread

    add  [text_start],120*80
    cmp  [text_start],I_END+120*80*20
    jb	 new_all_channels3

    popa
    ret

  no_nick_change:


     cmp  [irc_command],'KICK'	    ; kick
     jne  no_kick

    mov  [text_start],I_END
    add  [text_start],120*80

    mov  eax,[command_position]
    add  eax,5
    call set_channel

; new_all_channels4:

    mov  eax,action_header_short
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    call print_text
     mov  eax,kicked
     mov  dl,0
    call print_text
    mov  eax,[command_position]
    add  eax,5
    mov  dl,0
    call print_text
    call notify_channel_thread

;    add  [text_start],120*80
;    cmp  [text_start],I_END+120*80*20
;    jb   new_all_channels4

    popa
    ret

  no_kick:




    cmp  [irc_command],'QUIT'	 ; irc quit
    jne  no_quit

    mov  [text_start],I_END
    add  [text_start],120*80

 new_all_channels2:

    mov  eax,action_header_red
    mov  dl,0
    call print_text
    mov  eax,command+1
    mov  dl,'!'
    call print_text
    mov  eax,has_quit_irc
    mov  dl,0
    call print_text
    call notify_channel_thread

    add  [text_start],120*80
    cmp  [text_start],I_END+120*80*20
    jb	 new_all_channels2

    popa
    ret

  no_quit:


    cmp  [irc_command],dword 'MODE'  ; channel mode change
    jne  no_mode

    mov  [text_start],I_END
    add  [text_start],120*80

    mov  eax,[command_position]
    add  eax,5
    call set_channel

 new_all_channels:

    mov  eax,action_header_short
    mov  dl,0
    call print_text

    call print_nick

    mov  eax,sets_mode
    mov  dl,0
    call print_text

    mov  eax,[command_position]
    add  eax,5
    mov  dl,0
    call print_text
    call notify_channel_thread

;    add  [text_start],120*80
;    cmp  [text_start],I_END+120*80*20
;    jb   new_all_channels

    popa
    ret

  no_mode:


    cmp  [irc_command],dword '353 '  ; channel user names
    jne  no_user_list

    mov  eax,[command_position]
   finde2:
    inc  eax
    cmp  [eax],byte '#'
    jne  finde2
    call set_channel

   finde3:
    inc  eax
    cmp  [eax],byte ':'
    jne  finde3

    pusha
    cmp  [user_list_pos],0
    jne  no_clear_user_list
    mov  edi,[text_start]
    add  edi,120*70
    mov  [edi-8],dword 0
    mov  [edi-4],dword 0
    mov  al,32
    mov  ecx,1200
    cld
    rep  stosb
  no_clear_user_list:
    popa

    push eax

    mov  esi,eax
    inc  esi
    mov  edi,[text_start]
    add  edi,120*70
    add  edi,[user_list_pos]
    mov  edx,edi
    mov  ecx,command
    add  ecx,[cmd]
    sub  ecx,[esp]
    sub  ecx,3
    and  ecx,0xfff
    cld
    rep  movsb

    pop  eax
    mov  ebx,command
    add  ebx,[cmd]
    sub  ebx,eax
    sub  ebx,2
    mov  [edx+ebx-1],dword '    '

    add  [user_list_pos],ebx

    mov  eax,[user_list_pos]
    mov  ebx,[text_start]
    add  ebx,120*70
    mov  [ebx-4],eax
    call notify_channel_thread

    popa
    ret

  user_list_pos dd 0x0

  no_user_list:


    cmp  [irc_command],dword '366 '  ; channel user names end
    jne  no_user_list_end

    mov  [user_list_pos],0

    popa
    ret

  no_user_list_end:

    mov  [command],byte '-'
    call print_command_to_main

    popa

    ret


cresult db 0

compare_to_nick:

; input  : eax = start of compare
; output : [cresult] = 0 if match, [cresult]=1 if no match


    pusha

    mov  esi,eax
    mov  edi,0

  new_nick_compare:

    mov  bl,byte [esi]
    mov  cl,byte [user_nick+4+edi]

    cmp  bl,cl
    jne  nonickm

    add  esi,1
    add  edi,1

    cmp  edi,[user_nick]
    jb	 new_nick_compare

    movzx eax,byte [esi]
    cmp  eax,40
    jge  nonickm

    popa
    mov  [cresult],0
    ret

  nonickm:

    popa
    mov  [cresult],1
    ret





print_command_to_main:

    pusha

    mov  [text_start],I_END
    mov  ecx,[rxs]
    imul ecx,11
    mov  [pos],ecx

    mov  bl,13
    call print_character
    mov  bl,10
    call print_character

    mov  ecx,[cmd]
    sub  ecx,2
    mov  esi,command
   newcmdc2:
    mov  bl,[esi]
    call print_character
    inc  esi
    loop newcmdc2

    mov   edx,I_END
    call  draw_channel_text

    popa

    ret




print_text:

    pusha

    mov  ecx,command-2
    add  ecx,[cmd]

  ptr2:
    mov  bl,[eax]
    cmp  bl,dl
    je	 ptr_ret
    cmp  bl,0
    je	 ptr_ret
    call print_character
    inc  eax
    cmp  eax,ecx
    jbe  ptr2

  ptr_ret:

    mov  eax,[text_start]
    mov  [eax+120*60],byte 1

    popa
    ret


cp1251_table:
  db '?','?','?','?','?','?','?','?' , '?','?','?','?','?','?','?','?' ; 8
  db '?','?','?','?','?',$F9,'?','?' , '?','?','?','?','?','?','?','?' ; 9
  db '?',$F6,$F7,'?',$FD,'?','?','?' , $F0,'?',$F2,'?','?','?','?',$F4 ; A
  db $F8,'?','?','?','?','?','?',$FA , $F1,$FC,$F3,'?','?','?','?',$F5 ; B
  db $80,$81,$82,$83,$84,$85,$86,$87 , $88,$89,$8A,$8B,$8C,$8D,$8E,$8F ; C
  db $90,$91,$92,$93,$94,$95,$96,$97 , $98,$99,$9A,$9B,$9C,$9D,$9E,$9F ; D
  db $A0,$A1,$A2,$A3,$A4,$A5,$A6,$A7 , $A8,$A9,$AA,$AB,$AC,$AD,$AE,$AF ; E
  db $E0,$E1,$E2,$E3,$E4,$E5,$E6,$E7 , $E8,$E9,$EA,$EB,$EC,$ED,$EE,$EF ; F

;    0   1   2   3   4   5   6   7     8   9   A   B   C   D   E   F

utf8_table:
	times 80h dw 0x98C3	; default placeholder
; 0x80-0xAF -> 0x90D0-0xBFD0
repeat 0x30
store byte 0xD0 at utf8_table+2*(%-1)
store byte 0x90+%-1 at utf8_table+2*%-1
end repeat
; 0xE0-0xEF -> 0x80D1-0x8FD1
repeat 0x10
store byte 0xD1 at utf8_table+2*(0xE0-0x80+%-1)
store byte 0x80+%-1 at utf8_table+2*(0xE0-0x80+%)-1
end repeat
; 0xF0 -> 0x81D0, 0xF1 -> 0x91D1
store dword 0x91D181D0 at utf8_table+2*(0xF0-0x80)

cp866_table:
  db $C0,$C1,$C2,$C3,$C4,$C5,$C6,$C7 , $C8,$C9,$CA,$CB,$CC,$CD,$CE,$CF ; 8
  db $D0,$D1,$D2,$D3,$D4,$D5,$D6,$D7 , $D8,$D9,$DA,$DB,$DC,$DD,$DE,$DF ; 9
  db $E0,$E1,$E2,$E3,$E4,$E5,$E6,$E7 , $E8,$E9,$EA,$EB,$EC,$ED,$EE,$EF ; A
  db '?','?','?','?','?','?','?','?' , '?','?','?','?','?','?','?','?' ; B
  db '?','?','?','?','?','?','?','?' , '?','?','?','?','?','?','?','?' ; C
  db '?','?','?','?','?','?','?','?' , '?','?','?','?','?','?','?','?' ; D
  db $F0,$F1,$F2,$F3,$F4,$F5,$F6,$F7 , $F8,$F9,$FA,$FB,$FC,$FD,$FE,$FF ; E
  db $A8,$B8,$AA,$BA,$AF,$BF,$A1,$A2 , $B0,$95,$B7,'?',$B9,$A4,'?','?' ; F

;    0   1   2   3   4   5   6   7     8   9   A   B   C   D   E   F

print_character:

    pusha

    cmp  bl,13	   ; line beginning
    jne  nobol
    mov  ecx,[pos]
    add  ecx,1
  boll1:
    sub  ecx,1
    mov  eax,ecx
    xor  edx,edx
    mov  ebx,[rxs]
    div  ebx
    cmp  edx,0
    jne  boll1
    mov  [pos],ecx
    jmp  newdata
  nobol:

    cmp  bl,10	   ; line down
    jne  nolf
   addx1:
    add  [pos],dword 1
    mov  eax,[pos]
    xor  edx,edx
    mov  ecx,[rxs]
    div  ecx
    cmp  edx,0
    jnz  addx1
    mov  eax,[pos]
    jmp  cm1
  nolf:
  no_lf_ret:


    cmp  bl,15	  ; character
    jbe  newdata

    mov  eax,[irc_data]
    shl  eax,8
    mov  al,bl
    mov  [irc_data],eax

    mov  eax,[pos]
    call draw_data

    mov  eax,[pos]
    add  eax,1
  cm1:
    mov  ebx,[scroll+4]
    imul ebx,[rxs]
    cmp  eax,ebx
    jb	 noeaxz

    mov  esi,[text_start]
    add  esi,[rxs]

    mov  edi,[text_start]
    mov  ecx,ebx
    cld
    rep  movsb

    mov  esi,[text_start]
    mov  ecx,[rxs]
    imul ecx,61
    add  esi,ecx

    mov  edi,[text_start]
    mov  ecx,[rxs]
    imul ecx,60
    add  edi,ecx
    mov  ecx,ebx
    cld
    rep  movsb

    mov  eax,ebx
    sub  eax,[rxs]
  noeaxz:
    mov  [pos],eax

  newdata:

    mov  eax,[text_start]
    mov  [eax+120*60],byte 1

    popa
    ret

notify_channel_thread:
	pusha
	mov	eax, [text_start]
	mov	ecx, [eax+120*60+12]
	mcall	60, 2, , 0, 1
	popa
	ret


draw_data:

    pusha

    and  ebx,0xff
    add  eax,[text_start]
    mov  [eax],bl

    popa
    ret



draw_window:

    pusha

    mov  eax,12
    mov  ebx,1
    mcall

    xor  eax,eax		   ; draw window
    mov  ebx,5*65536+499
    mov  ecx,5*65536+381
    mov  edx,[wcolor]
    add  edx,0x14ffffff
    mov  edi,title
    mcall

    mov  eax,8                     ; button: change encoding
    mov  ebx,(ENCODING_X-2)*65536+38
    mov  ecx,(ENCODING_Y-2)*65536+12
    mov  edx,21
    mov  esi,[main_button]
    mcall

;    mov  eax,8			   ; button: open socket
    mov  ebx,43*65536+22
    mov  ecx,241*65536+10
;    mov  edx,22
    inc  edx
    mcall

    ;mov  eax,8			   ; button: send userinfo
    mov  ebx,180*65536+22
    mov  ecx,241*65536+10
;    mov  edx,23
    inc  edx
    mcall

    ;mov  eax,8			   ; button: close socket
    mov  ebx,317*65536+22
    mov  ecx,241*65536+10
;    mov  edx,24
    inc  edx
    mcall

    mov  eax,38 		   ; line
    mov  ebx,5*65536+494
    mov  ecx,148*65536+148
    mov  edx,[main_line]
    mcall
    add  ecx,1*65536+1

    mov  eax,38 		   ; line
    mov  ebx,5*65536+494
    mov  ecx,166*65536+166
    mcall
    add  ecx,1*65536+1

    mov  eax,38 		   ; line
    mov  ebx,410*65536+410
    mov  ecx,22*65536+148
    mcall
    add  ebx,1*65536+1

    mov  ebx,25*65536+183	   ; info text
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,70
  newline:
    mov  eax,4
    mcall
    add  ebx,12
    add  edx,70
    cmp  [edx],byte 'x'
    jne  newline

    mov  edx,I_END		  ; text from server
    call draw_channel_text

    call print_entry

    mov  eax,12
    mov  ebx,2
    mcall

    popa

    ret

update_status:
	pusha
	mov	esi, [status]
	mov	edi, text + 7*70 + 22
	mov	ecx, status_text_len
	push	ecx
	imul	esi, ecx
	add	esi, status_text
	mov	edx, edi
	rep	movsb
	pop	esi
	mcall	4, STATUS_X*65536+STATUS_Y, 0x40000000, , , 0xFFFFFF
	popa
	ret

update_encoding:
	pusha
	mov	edx, 21
	mcall	8	; delete button
	mov	esi, [main_button]
	mcall	, <(ENCODING_X-2),38>, <(ENCODING_Y-2),12>	; recreate it
	mov	esi, [encoding]
	mov	edi, text + 3*70 + 15
	mov	ecx, encoding_text_len
	push	ecx
	imul	esi, ecx
	add	esi, encoding_text
	mov	edx, edi
	rep	movsb
	pop	esi
	mcall	4, ENCODING_X*65536+ENCODING_Y, 0
	popa
	ret

main_line    dd 0x000000
main_button  dd 0x6565cc


text:

db '   Real name : KolibriOS User  - change with eg /areal Jill User      '
db '   Nick      : kolibri_user    - change with eg /anick Jill           '
db '   Server    : kolibrios.org   - change with eg /aserv irc.by         '
db '   Encoding  : UTF-8                                                  '
db '                                                                      '
db '        1) Connect             2) Send userinfo       3) Disconnect   '
db '                                                                      '
db '   Connection status: disconnected                                    '
db '                                                                      '
db '   Commands after established connection:                             '
db '                                                                      '
db '   /join #ChannelName         - eg /join #general                     '
db '   /part #ChannelName         - eg /part #windows                     '
db '   /query Nickname            - eg /query Mary                        '
db '   /quit                      - Quit server and Close socket          '
db 'x' ; <- END MARKER, DONT DELETE

status_text:
db	'disconnected            '
db      'resolving server name...'
db	'connecting...           '
db	'connected               '
status_text_len = 24

encoding_text:
db	'CP866 '
db	'CP1251'
db	'UTF-8 '
encoding_text_len = 6

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;                        CHANNEL THREADS
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



channel_thread:

;    mov   ebp,[thread_nro]
    pop   ebp
    pop   edx

    mov   eax,ebp
    shl   eax,14
    add   eax,0x80000
    mov   esp,eax

;    mov   edi,ebp	 ; clear thread memory
;    imul  edi,120*80
;    add   edi,I_END
;    mov   ecx,120*80
;    mov   al,32
;    cld
;    rep   stosb

; Create IPC buffer in the stack.
	push	eax
	push	eax
	push	eax
	push	8
	push	0
	mov	ecx, esp
	push	edx
	mcall	60, 1, , 20
	pop	edx
	mcall	40, 1100111b

;    mov   edx,[thread_screen]

  thread_redraw:
    call  thread_draw_window
    call  draw_channel_text
    call  print_user_list
    call  print_entry

  w_t:

    mov  esi,ebp
    imul esi,120*80
    add  esi,I_END
    cmp  [esi+120*60+4],byte 1
    jne  no_channel_leave
    mov  [esi+120*60+4],byte 0
    mov  edi,ebp
    shl  edi,5
    mov  dword [channel_list+edi],dword '    '
    mov  byte  [channel_list+edi+31],byte 1
    mov  eax,-1
    mcall
  no_channel_leave:

    mcall 10
    dec   eax
    jz    thread_redraw
    dec   eax
    jz    thread_key
    dec   eax
    jz    thread_end
    cmp   al,4
    jz    thread_ipc
    call  check_mouse
    jmp   w_t
  thread_end:
    mov   eax,17
    mcall
    mov   eax,ebp
    imul  eax,120*80
    add   eax,I_END
    cmp   [eax+120*60+8],byte 0 ; channel window
    je	  not_close
    mov   eax,ebp
    shl   eax,5
    add   eax,channel_list
    mov   [eax],dword '    '
    mov   [eax+31],byte 1
    mov   eax,-1
    mcall
  not_close:
    mov   [text_start],eax
    mov   eax,nocl
  newcc:
    mov   bl,[eax]
    call  print_character
    inc   eax
    cmp   [eax],byte 0
    jne   newcc
    call  draw_channel_text
    jmp   w_t
   nocl:   db  13,10,'To exit channel, use PART or QUIT command.',0
  thread_ipc:
    mov   byte [esp+4], 8 ; erase message from IPC buffer
   no_end:

    cmp   [edx+120*60],byte 1
    jne   no_update
    mov   [edx+120*60],byte 0
    call  draw_channel_text
  no_update:

    call  print_user_list

  nopri2:

    jmp   w_t



check_mouse:

    pusha

    mov  eax,37
    mov  ebx,1
    mcall

    mov  ebx,eax
    shr  eax,16
    and  ebx,0xffff

    cmp  eax,420
    jb	 no_mouse
    cmp  eax,494
    jg	 no_mouse

    cmp  ebx,145
    jg	 no_mouse
    cmp  ebx,23
    jb	 no_mouse


    cmp  ebx,100
    jb	 no_plus
    mov  eax,ebp
    imul eax,120*80
    add  eax,120*70+I_END
    inc  dword [eax-8]
    call print_user_list
    mov  eax,5
    mov  ebx,8
    mcall
    jmp  no_mouse
  no_plus:

    cmp  ebx,80
    jg	 no_mouse
    mov  eax,ebp
    imul eax,120*80
    add  eax,120*70+I_END
    cmp  dword [eax-8],dword 0
    je	 no_mouse
    dec  dword [eax-8]
    call print_user_list
    mov  eax,5
    mov  ebx,8
    mcall

  no_minus:

  no_mouse:

    popa

    ret




thread_key:

    mov  eax,2
    mcall

    shr  eax,8

    cmp  eax,8
    jne  no_bks
    cmp  [xpos],0
    je	 w_t
    dec  [xpos]
    call print_entry
    jmp  w_t
   no_bks:

    cmp  eax,20
    jbe  no_character
    mov  ebx,[xpos]
    mov  [send_string+ebx],al
    inc  [xpos]
    cmp  [xpos],80
    jb	 xpok
    mov  [xpos],79
  xpok:
    call print_entry
    jmp  w_t
  no_character:

    cmp  eax,13
    jne  no_send
    cmp  [xpos],0
    je	 no_send
    mov  dword [send_to_channel],ebp
    pusha
    mcall 60,2,[main_PID],0,1
  wait_for_sending:
    mov  eax,5
    mov  ebx,1
    mcall
    cmp  dword [ipcbuf+4],8
    jne	 wait_for_sending
    popa
    call draw_channel_text
    call print_entry
    jmp  w_t
  no_send:

    jmp  w_t






draw_channel_text:

    pusha

    mov   eax,4
    mov   ebx,10*65536+26
    mov   ecx,12
    mov   esi,[rxs]
  dct:
    pusha
    mov   cx,bx
    shl   ecx,16
    mov   cx,9
    mov   eax,13
    mov   ebx,10*65536
    mov   bx,word [rxs]
    imul  bx,6
    mov   edx,0xffffff
    mcall
    popa
    push  ecx
    mov   eax,4
    mov   ecx,0
    cmp   [edx],word '* '
    jne   no_red
    mov   ecx,0x0000ff
   no_red:
    cmp   [edx],word '**'
    jne   no_light_blue
    cmp   [edx+2],byte '*'
    jne   no_light_blue
    mov   ecx,0x0000ff
  no_light_blue:
    cmp   [edx],byte '#'
    jne   no_blue
    mov   ecx,0x0000ff
  no_blue:
    mcall
    add   edx,[rxs]
    add   ebx,10
    pop   ecx
    loop  dct

    popa
    ret





thread_draw_window:

    pusha

    mov  eax,12
    mov  ebx,1
    mcall

    mov  ebx,ebp		   ; draw window
    shl  ebx,16+4
    xor  eax,eax
    mov  ecx,ebx
    mov  bx,499
    mov  cx,170

    mov  edx,[wcolor]
    add  edx,0x03ffffff
    mov  esi,0x80555599
    mov  edi,0x00ffffff

    mcall

    mov  eax,ebp		   ; label
    add  eax,48
    mov  [labelc+14],al
    mov  eax,ebp
    shl  eax,5
    add  eax,channel_list
    mov  esi,eax
    mov  edi,labelc+17
    movzx ecx,byte [eax+31]
    cld
    rep   movsb

    mov  esi,17 		   ; print label
    movzx ebx,byte [eax+31]
    add  esi,ebx
    mov  eax,4
    mov  ebx,9*65536+8
    mov  ecx,0x00ffffff
    mov  edx,labelc
    mcall

    mov  eax,38 		   ; line
    mov  ebx,5*65536+494
    mov  ecx,148*65536+148
    mov  edx,[channel_line_sun]
    mcall
    add  ecx,1*65536+1
    mov  edx,[channel_line_shadow]
    mcall


    ;mov  eax,38 		   ; line
    mov  ebx,410*65536+410
    mov  ecx,22*65536+148
    mov  edx,[channel_line_sun]
    mcall
    add  ebx,1*65536+1
    mov  edx,[channel_line_shadow]
    mcall

    mov  eax,12
    mov  ebx,2
    mcall

    popa

    ret


; DATA AREA

socket	dd  0x0

bgc  dd  0x000000
     dd  0x000000
     dd  0x00ff00
     dd  0x0000ff
     dd  0x005500
     dd  0xff00ff
     dd  0x00ffff
     dd  0x770077

tc   dd  0xffffff
     dd  0xff00ff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff

channel_line_sun    dd 0x9999ff
channel_line_shadow dd 0x666699

cursor_on_off  dd  0x0

max_windows    dd  20

thread_stack   dd  0x9fff0
;thread_nro     dd 1
;thread_screen  dd I_END+120*80*1

action_header_blue  db	10,'*** ',0
action_header_red   db	10,'*** ',0

action_header_short db	10,'* ',0

has_left_channel db  ' has left ',0
joins_channel	 db  ' has joined ',0
is_now_known_as  db  ' is now known as ',0
has_quit_irc	 db  ' has quit IRC',0
sets_mode	 db  ' sets mode ',0
kicked		 db  ' kicked from ',0

index_list_1	 dd  0x0000bb
index_list_2	 dd  0x0000ff

posx		 dd  0x0
incoming_pos	 dd  0x0
incoming_string: times 128 db 0

pos	     dd  0x0

text_start   dd  I_END
irc_data     dd  0x0
print	     db  0x0
cmd	     dd  0x0
rxs	     dd  66

res:	     db  0,0
command:     times  600  db 0x0

nick	     dd  0,0,0
irc_command  dd  0,0

command_position  dd 0x0
counter 	  dd  0
send_to_server	  db 0

channel_list:	  times 32*20 db 32
send_to_channel   dd 0x0

send_string_header:  db     'privmsg #eax :'
		     times  100  db  0x0

send_string:	     times  100  db  0x0
xpos	     dd  0

string0:     db  'USER guest ser1 ser2 :'
string0l:
string1:     db  'nick '
string1l:

attribute   dd	0
scroll	    dd	1
	    dd	12

numtext     db	'                     '

wcolor	    dd	0x000000

labelc	    db	'AIRC - WINDOW X: #xxx                 '
title	    db	'IRC client ',version,0

ipcbuf:
	dd	0
	dd	8
	dd	?
	dd	?
	db	?
.size = $

align 4
@IMPORT:

library network, 'network.obj', msgbox, 'msgbox.obj'
import  network, \
	getaddrinfo_start,	'getaddrinfo_start',	\
	getaddrinfo_process,	'getaddrinfo_process',	\
	getaddrinfo_abort,	'getaddrinfo_abort',	\
	freeaddrinfo,		'freeaddrinfo'
import	msgbox, mb_create, 'mb_create', mb_setfunctions, 'mb_setfunctions'

msgbox_running	db	?	; must be the byte before msgbox_struct
				; look to the handler of button 21
msgbox_struct:
.default:
	dw	?	; default button, will be filled with current encoding
	db	'Encoding',0
	db	'Select encoding for all messages:',0
	db	'CP866',0
	db	'CP1251',0
	db	'UTF-8',0
	db	0

align 4
status		dd	STATUS_DISCONNECTED
encoding	dd	UTF8
recode_proc	dd	recode_to_cp866, recode_to_cp1251, recode_to_utf8
get_byte_table	dd	get_byte_cp866, get_byte_cp1251, get_byte_utf8
msgbox_func_array:
times 3		dd	msgbox_notify
initialized_size:

main_PID	dd	?	; identifier of main thread
utf8_bytes_rest	dd	?	; bytes rest in current UTF8 sequence
utf8_char	dd	?	; first bits of current UTF8 character
gai_reqdata	rb	32	; buffer for getaddrinfo_start/process
ip_list		dd	?	; will be filled as pointer to addrinfo list
irc_server_name	rb	256	; buffer for irc_server_name
packetbuf	rb	1024	; buffer for packets to server
mb_stack	rb	1024	; stack for messagebox thread

;;
;;   Channel data at I_END
;;
;;   120*80 * channel window (1+)
;;
;;      At         Size
;;
;;      00      ,  120*60   window text 120 characters per row
;;  120*60      ,  1        text is updated
;;  120*60+4    ,  1        close yourself
;;  120*60+8    ,  1        0 = channel window  :  1 = private chat
;;  120*60+12    , 4        identifier of the thread
;;  120*61      ,  256      channel name
;;  120*61+254  ,  254      channel entry text from user
;;  120*61+255  ,  1        length of entry text
;;  120*69+248  ,  4        display names from n:th name
;;  120*69+252  ,  4        length of names string
;;  120*70      ,  1200     names separated with space
;;
I_END:
