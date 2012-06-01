; version: 0.5
; last update:  07/10/2010
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      reducing the size of the binary code,
;               program uses far less memory while running
;               (>0x7000, the old version used >0x100000),
;               process only net event at start with parameter
;-----------------------------------------------------------
; version 0.3 -0.4 
; written by:   CleverMouse
;
;-----------------------------------------------------------
; wget 0.2 by barsuk
; based on Menuet Httpc


; Enabling debugging puts stuff to the debug board
DEBUGGING_ENABLED	equ 1
DEBUGGING_DISABLED	equ 0
DEBUGGING_STATE		equ DEBUGGING_ENABLED

use32
	org	0x0
	db 'MENUET01'	; header
	dd 0x01		; header version
	dd START	; entry point
	dd IM_END	; image size
	dd I_END	;0x100000 ; required memory
	dd stacktop	; esp
	dd params	; I_PARAM
	dd 0x0		; I_Path

include	'lang.inc'
include	'../../../macros.inc'
include	"../../../proc32.inc"
include	"dll.inc"
include	"debug.inc"

URLMAXLEN	equ 256	; maximum length of url string

primary_buffer_size	equ 4096

; Memory usage
; webpage headers at buf_headers

START:	; start of execution
;dps	<"Program started",13,10>
; prepare webAddr area	
	mov	al,' '
	mov	edi,webAddr
	mov	ecx,URLMAXLEN
	cld
	rep	stosb
	xor	eax,eax
	stosb
; prepare document area	
	mov	al,'/'
	mov	edi,document
	cld
	stosb
	mov	al,' '
	mov	ecx,URLMAXLEN-1
	rep	stosb

; create local heap
	mcall	68,11

	call	load_settings
	cmp	[params],byte 0
	jz	prepare_event	;red

	mcall	40,10000000b ; only net event!!!

; we have an url
	mov	edi,document_user
	mov	al,' '
	mov	ecx,URLMAXLEN
	rep	stosb
	
	mov	esi,params
	mov	edi,document_user

.copy_param:
	mov	al,[esi]
	cmp	al,0
	jz	.done

	cmp	al,' '
	jz	.done_inc

	mov	[edi],al
	inc	esi
	inc	edi
	jmp	.copy_param

.done_inc:
; url is followed by shared memory name.
	inc	esi
.done:
	mov	[shared_name],esi

	mov	ah,22	; strange way to tell that socket should be opened...
	call	socket_commands

	jmp	still

prepare_event:
; Report events
; Stack 8 + defaults
	mcall	40,10000111b

red:	; redraw
	call	draw_window

still:
	mcall	23,1	; wait here for event
	cmp	eax,1	; redraw request ?
	je	red

	cmp	eax,2	; key in buffer ?
	je	key

	cmp	eax,3	; button in buffer ?
	je	button

; Get the web page data from the remote server
	call	read_incoming_data
	mov	eax,[status]
	mov	[prev_status],eax
	mcall	53,6,[socket]
	mov	[status],eax

	cmp	[prev_status],4
	jge	no_send

	cmp	[status],4
	jne	no_send

	mov	[onoff],1
	call	send_request

no_send:
	call	print_status

	cmp	[prev_status],4
	jne	no_close
	cmp	[status],4	; connection closed by server
	jbe	no_close	; respond to connection close command
; draw page
	call	read_incoming_data
	mcall	53,8,[socket]
	call	draw_page
	mov	[onoff],0

no_close:
	jmp	still

key:	; key
	mcall	2	; just read it and ignore
	shr	eax,8
	cmp	eax,184
	jne	no_down
	cmp	[display_from],25
	jb	no_down
	sub	[display_from],25
	call	display_page

no_down:
	cmp	eax,183
	jne	no_up
	add	[display_from],25
	call	display_page

no_up:
	jmp	still

button:	; button
;dps	<"Button pressed",13,10>
	mcall	17	; get id
	cmp	ah,26
	je	save
	cmp	ah,1	; button id=1 ?
	jne	noclose
;	dps	"Closing socket before exit... "

close_end_exit:

;dpd	eax
;dps	<13,10>

exit:
	or	eax,-1	; close this program
	mcall

save:
dps	"saving"
newline
	mcall	70,fileinfo
;pregs
	jmp	still

noclose:
	cmp	ah,31
	jne	noup
	sub	[display_from],20
	call	display_page
	jmp	still

noup:
	cmp	ah,32
	jne	nodown
	add	[display_from],20
	call	display_page
	jmp	still

nodown:
	cmp	ah,10	; Enter url
	jne	nourl

	mov	[addr],dword document_user
	mov	[ya],dword 38
	mov	[len],dword URLMAXLEN

	mov	ecx,URLMAXLEN
	mov	edi,[addr]
	mov	al,' '
	rep	stosb

	call	print_text

	mov	edi,[addr]

f11:
	mcall	10
	cmp	eax,2	; key?
	jz	fbu
	jmp	still

fbu:
	mcall	2	; get key
	shr	eax,8
	cmp	eax,8
	jnz	nobs
	cmp	edi,[addr]
	jz	f11
	sub	edi,1
	mov	[edi],byte ' '
	call	print_text
	jmp	f11

nobs:
	cmp	eax,10
	je	retkey
	cmp	eax,13
	je	retkey

	cmp	eax,31
	jbe	f11

; Removed in v0.4
;	cmp	eax,95
;	jb	keyok
;	sub	eax,32

keyok:
	mov	[edi],al
	call	print_text
	add	edi,1
	mov	esi,[addr]
	add	esi,URLMAXLEN
	cmp	esi,edi
	jnz	f11
	jmp	still

retkey:
	mov	ah,22	; start load

nourl:
	call	socket_commands	; opens or closes the connection
	jmp	still

;****************************************************************************
;    Function
;       send_request
;
;   Description
;       Transmits the GET request to the server.
;       This is done as GET then URL then HTTP/1.1',13,10,13,10 in 3 packets
;
;****************************************************************************
send_request:
	pusha
	mov	esi,string0
	mov	edi,request
	movsd
; If proxy is used, make absolute URI - prepend http://<host>
	cmp	[proxyAddr],byte 0
	jz	.noproxy
	mov	dword [edi],'http'
	mov	[edi+4],byte ':'
	mov	[edi+5],word '//'
	add	edi,7
	mov	esi,webAddr

.copy_host_loop:
	lodsb
	cmp	al,' '
	jz	.noproxy
	stosb
	jmp	.copy_host_loop

.noproxy:
	xor	edx,edx ; 0

.next_edx:
; Determine the length of the url to send in the GET request
	mov	al,[edx+document]
	cmp	al,' '
	je	.document_done
	mov	[edi],al
	inc	edi
	inc	edx
	jmp	.next_edx

.document_done:
	mov	esi,stringh
	mov	ecx,stringh_end-stringh
	rep	movsb
	xor	edx,edx	; 0

.webaddr_next:
	mov	al,[webAddr + edx]
	cmp	al,' '
	je	.webaddr_done
	mov	[edi],al
	inc	edi
	inc	edx
	jmp	.webaddr_next

.webaddr_done:
	cmp	[proxyUser],byte 0
	jz	@f
	call	append_proxy_auth_header
@@:
	mov	esi,connclose
	mov	ecx,connclose_end-connclose
	rep	movsb

	pusha	
	mov	eax,63
	mov	ebx,1
	mov	edx,request
@@:
	mov	cl,[edx]
	cmp	edx,edi
	jz	@f
	mcall
	inc	edx
	jmp	@b
@@:
	popa

	mov	edx,edi
	sub	edx,request
;;;;now write \r\nConnection: Close \r\n\r\n
	mcall	53,7,[socket],,request	;' HTTP/1.1 .. '
	popa
	ret

;****************************************************************************
;    Function
;       print_status
;
;   Description
;       displays the socket/data received status information
;
;****************************************************************************
print_status:
	pusha
	mcall	26,9
	cmp	eax,[nextupdate]
	jb	status_return

	add	eax,25
	mov	[nextupdate],eax

	mov	ecx,[winys]
	shl	ecx,16
	add	ecx,-18*65536+10
	mcall	13,<5,100>,,0xffffff

	mov	edx,12*65536-18
	add	edx,[winys]
	xor	esi,esi
	mcall	47,<3,0>,[status],,

	mov	edx,40*65536-18
	add	edx,[winys]
	mcall	,<6,0>,[pos]

status_return:
	popa
	ret

;****************************************************************************
;    Function
;       read_incoming_data
;
;   Description
;       receive the web page from the server, storing it without processing
;
;****************************************************************************
read_incoming_data:
	cmp	[onoff],1
	je	rid
	ret

rid:
	push	esi
	push	edi
dps	"rid"
newline

newbyteread:
	;call	print_status
	mcall	53,2,[socket]
	cmp	eax,0
	je	no_more_data

	mcall	53,11,[socket],primary_buf,primary_buffer_size
	
;dps	"part "
;dph	eax
;newline
	mov	edi,[pos]
	add	[pos],eax
	push	eax
	mcall	68,20,[pos],[buf_ptr]
	mov	[buf_ptr],eax
	add	edi,eax
	mov	esi,primary_buf
	pop	ecx	; number of recently read bytes
	lea	edx,[ecx - 3]
	rep	movsb
	
no_more_data:
	mcall	53,6,[socket]
	cmp	eax,4
	jne	no_more_data.finish
	jmp	newbyteread

.finish:
;dps	"finish	"
;pregs
	call	parse_result
	mov	ecx,[shared_name]
	test	ecx, ecx
	jz	@f
	cmp	[ecx],byte 0
	jnz	save_in_shared
@@:

	mcall	70,fileinfo
;dps	"saving "
;pregs
;	jmp	close_end_exit
	pop	edi
	pop	esi
; if called from command line, then exit
	cmp	[params],byte 0
	jnz	exit
	ret
	
save_in_shared:
	mov	esi,1	; SHM_OPEN+SHM_WRITE
	mcall	68,22
	test	eax,eax
	jz	save_in_shared_done

	sub	edx,4
	jbe	save_in_shared_done

	mov	ecx,[final_size]
	cmp	ecx,edx
	jb	@f

	mov	ecx,edx
@@:
	mov	[eax],ecx
	lea	edi,[eax+4]
	mov	esi,[final_buffer]
	mov	edx,ecx
	shr	ecx,2
	rep	movsd
	mov	ecx,edx
	and	ecx,3
	rep	movsb

save_in_shared_done:
	pop	edi
	pop	esi
	jmp	exit
	
; this function cuts header, and removes chunk sizes if doc is chunked
; in: buf_ptr, pos; out: buf_ptr, pos.
	
parse_result:
; close socket
	mcall	53,8,[socket]
	
dps	"close	socket: "
dph	eax
newline
	mov	edi,[buf_ptr]
	mov	edx,[pos]
	mov	[buf_size],edx
;	mcall	70,fileinfo_tmp
dps	"pos = "
dph	edx
newline

; first, find end of headers
.next_byte:
	cmp	[edi],dword 0x0d0a0d0a	; мне лень читать стандарт, пусть будут оба варианта
	je	.end_of_headers
	cmp	[edi],dword 0x0a0d0a0d
	je	.end_of_headers
	inc	edi
	dec	edx
	jne	.next_byte
; no end of headers. it's an error. let client see all those headers.
	ret

.end_of_headers:
; here we look at headers and search content-length or transfer-encoding headers
;dps	"eoh "
;newline
	sub	edi,[buf_ptr]
	add	edi,4
	mov	[body_pos],edi	; store position where document body starts
	mov	[is_chunked],0
; find content-length in headers
; not good method, but should work for 'Content-Length:'
	mov	esi,[buf_ptr]
	mov	edi,s_contentlength
	mov	ebx,[body_pos]
	xor	edx,edx	; 0
.cl_next:
	mov	al,[esi]
	cmp	al,[edi + edx]
	jne	.cl_fail
	inc	edx
	cmp	edx,len_contentlength
	je	.cl_found
	jmp	.cl_incr
.cl_fail:
	xor	edx,edx	; 0
.cl_incr:
	inc	esi
	dec	ebx
	je	.cl_error
	jmp	.cl_next
.cl_error:
;pregs
;newline
;dph	esi
;dps	" content-length not found "
; find 'chunked'
; да, я копирую код, это ужасно, но мне хочется, чтобы поскорее заработало
; а там уж отрефакторю
	mov	esi,[buf_ptr]
	mov	edi,s_chunked
	mov	ebx,[body_pos]
	xor	edx,edx	; 0

.ch_next:
	mov	al,[esi]
	cmp	al,[edi + edx]
	jne	.ch_fail
	inc	edx
	cmp	edx,len_chunked
	je	.ch_found
	jmp	.ch_incr

.ch_fail:
	xor	edx,edx	; 0

.ch_incr:
	inc	esi
	dec	ebx
	je	.ch_error
	jmp	.ch_next

.ch_error:
; if neither of the 2 headers is found, it's an error
;dps	"transfer-encoding: chunked not found "
	mov	eax,[pos]
	sub	eax,[body_pos]
	jmp	.write_final_size

.ch_found:
	mov	[is_chunked],1
	mov	eax,[body_pos]
	add	eax,[buf_ptr]
	sub	eax,2
	mov	[prev_chunk_end],eax
	jmp	parse_chunks
	
.cl_found:	
	call	read_number	; eax = number from *esi

.write_final_size:
	mov	[final_size],eax	; if this works, i will b very happy...
	
	mov	ebx,[pos]	; we well check if it is right
	sub	ebx,[body_pos]

;dps	"check cl eax==ebx "
;pregs

; everything is ok, so we return
	mov	eax,[body_pos]
	mov	ebx,[buf_ptr]
	add	ebx,eax
	mov	[final_buffer],ebx
;	mov	ebx,[pos]
;	sub	ebx,eax
;	mov	[final_size],ebx
	ret
	
parse_chunks:
;dps	"parse chunks"
;newline
	; we have to look through the data and remove sizes of chunks we see
	; 1. read size of next chunk
	; 2. if 0, it's end. if not, continue.
	; 3. make a good buffer and copy a chunk there
	xor	eax,eax
	mov	[final_buffer],eax	; 0
	mov	[final_size],eax	; 0
	
.read_size:
	mov	eax,[prev_chunk_end]
	mov	ebx,eax
	sub	ebx,[buf_ptr]
	mov	edx,eax
;dps	"rs "
;pregs
	cmp	ebx,[pos]
	jae	chunks_end	; not good
	
	call	read_hex	; in: eax=pointer to text. out:eax=hex number,ebx=end of text.
	cmp	eax,0
	jz	chunks_end

	add	ebx,1
	mov	edx,ebx	; edx = size of size of chunk
	
	add	ebx,eax
	mov	[prev_chunk_end],ebx
	
;dps	"sz "
;pregs
; do copying: from buf_ptr+edx to final_buffer+prev_final_size count eax
; realloc final buffer
	push	eax
	push	edx
	push	dword [final_size]
	add	[final_size],eax
	mcall	68,20,[final_size],[final_buffer]
	mov	[final_buffer],eax
;dps	"re "
;pregs
	pop	edi
	pop	esi
	pop	ecx
;	add	[pos],ecx
	add	edi,[final_buffer]
;dps	"cp "
;pregs
	rep	movsb
	jmp	.read_size
	
chunks_end:
; free old buffer
dps	"chunks end"
newline
	mcall	68,13,[buf_ptr]
; done!
	ret

; reads content-length from [edi+ecx], result in eax
read_number:
	push	ebx
	xor	eax,eax
	xor	ebx,ebx

.next:
	mov	bl,[esi]
;dph	ebx
	cmp	bl,'0'
	jb	.not_number
	cmp	bl,'9'
	ja	.not_number
	sub	bl,'0'
	shl	eax,1
	lea	eax,[eax + eax * 4]	; eax *= 10
	add	eax,ebx

.not_number:
	cmp	bl,13
	jz	.done
	inc	esi
	jmp	.next

.done:
	pop	ebx
;newline
;dps	"strtoint eax "
;pregs
	ret
	
; reads hex from eax,result in eax,end of text in ebx
read_hex:
	add	eax,2
	mov	ebx,eax
	mov	eax,[ebx]
	mov	[deba],eax
;	pushf
;	pushad
;	mov	edx,deba
;	call	debug_outstr
;	popad
;	popf
	xor	eax,eax
	xor	ecx,ecx
.next:
	mov	cl,[ebx]
	inc	ebx
	
	cmp	cl,0x0d
	jz	.done
;dph	ebx
	or	cl,0x20
	sub	cl,'0'
	jb	.bad

	cmp	cl,0x9
	jbe	.adding

	sub	cl,'a'-'0'-10
	cmp	cl,0x0a
	jb	.bad

	cmp	cl,0x0f
	ja	.bad

.adding:
	shl	eax,4
	or	eax,ecx
;	jmp	.not_number
;.bad:
.bad:
	jmp	.next
.done:
;newline
;dps	"hextoint eax "
;pregs
	ret

;****************************************************************************
;    Function
;       draw_page
;
;   Description
;       parses the web page data, storing displayable data at 0x20000
;       and attributes at 0x30000. It then calls display_page to render
;       the data
;
;****************************************************************************
draw_page:
	ret

;****************************************************************************
;    Function
;       linefeed
;
;   Description
;
;
;****************************************************************************
linefeed:
	ret

;****************************************************************************
;    Function
;       display_page
;
;   Description
;       Renders the text decoded by draw_page
;
;****************************************************************************
display_page:
	ret

;****************************************************************************
;    Function
;       socket_commands
;
;   Description
;       opens or closes the socket
;
;****************************************************************************
socket_commands:
	cmp	ah,22	; open socket
	jnz	tst3

dps	"opening socket"
newline
; Clear all page memory
	xor	eax,eax
	mov	[prev_chunk_end],eax	; 0
	cmp	[buf_ptr],eax	; 0
	jz	no_free

	mcall	68,13,[buf_ptr]	; free	buffer

no_free:
	xor	eax,eax
	mov	[buf_size],eax	; 0
; Parse the entered url
	call	parse_url
; Get a free port number
	mov	ecx,1000	; local port starting at 1000

getlp1:
	inc	ecx
	push	ecx
	mcall	53,9
	pop	ecx
	cmp	eax,0	; is this local port in use?
	jz	getlp1	; yes - so try next

	mov	edx,80
	cmp	[proxyAddr],byte 0
	jz	sc000
	mov	edx,[proxyPort]
sc000:
	mcall	53,5,,,[server_ip],1
	mov	[socket],eax
	mov	[pagexs],80
	
	push	eax
	xor	eax,eax ; 0
	mov	[pos],eax
	mov	[pagex],eax
	mov	[pagey],eax
	mov	[command_on_off],eax
	mov	[is_body],eax
	pop	eax
	ret

tst3:
	cmp	ah,24	; close	socket
	jnz	no_24

	mcall	53,8,[socket]
	call	draw_page
no_24:
	ret

;****************************************************************************
;    Function
;       parse_url
;
;   Description
;       parses the full url typed in by the user into a web address ( that
;       can be turned into an IP address by DNS ) and the page to display
;       DNS will be used to translate the web address into an IP address, if
;       needed.
;       url is at document_user and will be space terminated.
;       web address goes to webAddr and is space terminated.
;       ip address goes to server_ip
;       page goes to document and is space terminated.
;
;       Supported formats:
;       <protocol://>address<page>
;       <protocol> is optional, removed and ignored - only http supported
;       <address> is required. It can be an ip address or web address
;       <page> is optional and must start with a leading / character
;
;****************************************************************************
parse_url:
; First, reset destination variables
	cld
	mov	al,' '
	mov	edi,document
	mov	ecx,URLMAXLEN
	rep	stosb
	mov	edi,webAddr
	mov	ecx,URLMAXLEN
	rep	stosb

	mov	al,'/'
	mov	[document],al

	mov	esi,document_user
; remove any leading protocol text
	mov	ecx,URLMAXLEN
	mov	ax,'//'

pu_000:
	cmp	[esi],byte ' '	; end of text?
	je	pu_002		; yep, so not found
	cmp	[esi],ax
	je	pu_001	; Found it, so esi+2 is start
	inc	esi
	loop	pu_000

pu_002:
; not found, so reset esi to start
	mov	esi,document_user-2

pu_001:
	add	esi,2
	mov	ebx,esi	; save address of start of web address
	mov	edi,document_user + URLMAXLEN	; end of string
; look for page delimiter - it's a '/' character
pu_003:
	cmp	[esi],byte ' '	; end of text?
	je	pu_004		; yep, so none found
	cmp	esi,edi		; end of string?
	je	pu_004		; yep, so none found
	cmp	[esi],byte '/'	; delimiter?
	je	pu_005		; yep - process it
	inc	esi
	jmp	pu_003

pu_005:
; copy page to document address
; esi = delimiter
	push	esi
	mov	ecx,edi		; end of document_user
	mov	edi,document
	cld

pu_006:
	movsb
	cmp	esi,ecx
	je	pu_007		; end of string?
	cmp	[esi],byte ' '	; end of text
;	je	pu_007		; дзен-ассемблер
;	jmp	pu_006		; не надо плодить сущности по напрасну
	jne	pu_006

pu_007:
	pop	esi	; point esi to '/' delimiter

pu_004:
; copy web address to webAddr
; start in ebx,end in esi-1
	mov	ecx,esi
	mov	esi,ebx
	mov	edi,webAddr
	cld

pu_008:
	movsb
	cmp	esi,ecx
;	je	pu_009		; дзен-ассемблер
;	jmp	pu_008		; не надо плодить сущности по напрасну
	jne	pu_008

pu_009:
; For debugging, display resulting strings
if	DEBUGGING_STATE = DEBUGGING_ENABLED
	mov	esi,document_user
	call	debug_print_string
	mov	esi,webAddr
	call	debug_print_string
	mov	esi,document
	call	debug_print_string
end	if
; Look up the ip address, or was it specified?
	mov	al,[proxyAddr]
	cmp	al,0
	jnz	pu_015
	mov	al,[webAddr]
pu_015:
	cmp	al,'0'
	jb	pu_010	; Resolve address
	cmp	al,'9'
	ja	pu_010	; Resolve address

if	DEBUGGING_STATE = DEBUGGING_ENABLED
	mov	esi,str2	; print	gotip
	call	debug_print_string
end	if
; Convert address
; If proxy is given, get proxy address instead of server
	mov	esi,proxyAddr-1
	cmp	byte [esi+1],0
	jnz	pu_020
	mov	esi,webAddr-1

pu_020:
	mov	edi,server_ip
	xor	eax,eax

ip1:
	inc	esi
	cmp	[esi],byte '0'
	jb	ip2
	cmp	[esi],byte '9'
	jg	ip2
	imul	eax,10
	movzx	ebx,byte [esi]
	sub	ebx,48
	add	eax,ebx
	jmp	ip1

ip2:
	mov	[edi],al
	xor	eax,eax
	inc	edi
	cmp	edi,server_ip+3
	jbe	ip1
	jmp	pu_011

pu_010:
if	DEBUGGING_STATE = DEBUGGING_ENABLED
	mov	esi,str1	; print	resolving
	call	debug_print_string
end	if
; Resolve Address
	call	translateData	; Convert domain & DNS IP address
	call	resolveDomain	; get ip address

if	DEBUGGING_STATE = DEBUGGING_ENABLED
	mov	esi,str3
	call	debug_print_string
end	if

pu_011:
; Done
	ret

;***************************************************************************
;   Function
;      translateData
;
;   Description
;      Coverts the domain name and DNS IP address typed in by the user into
;      a format suitable for the IP layer.
;
;    The ename, in query, is converted and stored in dnsMsg
;
;***************************************************************************
translateData:
    ; first, get the IP address of the DNS server
    ; Then, build up the request string.

    ; Build the request string
	mov	eax,0x00010100
	mov	[dnsMsg],eax
	mov	eax,0x00000100
	mov	[dnsMsg+4],eax
	mov	eax,0x00000000
	mov	[dnsMsg+8],eax
; domain name goes in at dnsMsg+12
	mov	esi,dnsMsg +12	;location of label length
	mov	edi,dnsMsg + 13	;label start
	mov	edx,proxyAddr
	cmp	byte [edx],0
	jnz	td000
	mov	edx,webAddr

td000:
	mov	ecx,12	; total string length so far

td002:
	mov	[esi],byte 0
	inc	ecx

td0021:
	mov	al,[edx]
	cmp	al,' '
	je	td001	; we have finished the string translation

	cmp	al,0
	je	td001

	cmp	al,'.'	; we have finished the label
	je	td004

	inc	byte [esi]
	inc	ecx
	mov	[edi],al
	inc	edi
	inc	edx
	jmp	td0021

td004:
	mov	esi,edi
	inc	edi
	inc	edx
	jmp	td002

; write label len + label text
td001:
	mov	[edi],byte 0
	inc	ecx
	inc	edi
	mov	[edi],dword 0x01000100
	add	ecx,4
	mov	[dnsMsgLen],ecx
	ret

;***************************************************************************
;   Function
;      resolveDomain
;
;   Description
;       Sends a question to the dns server
;       works out the IP address from the response from the DNS server
;
;***************************************************************************
resolveDomain:
; Get a free port number
	mov	ecx,1000	; local port starting at 1000
getlp:
	inc	ecx
	push	ecx
	mcall	53,9
	pop	ecx
	cmp	eax,0	; is this local port in use?
	jz	getlp	; yes - so try next

; Get DNS IP
	mcall	52,13
	mov	esi,eax
; First, open socket
	mov	edx,53	; remote port - dns
;	mov	esi,dword [dns_ip]
	xor	ebx,ebx	; 0
	mcall	53
	mov	[socketNum],eax
; write to socket ( request DNS lookup )
	mcall	53,4,[socketNum],[dnsMsgLen],dnsMsg
; Setup the DNS response buffer
	mov	eax,dnsMsg
	mov	[dnsMsgLen],eax
; now, we wait for
; UI redraw
; UI close
; or data from remote

ctr001:
	mcall	10	; wait here for event
	cmp	eax,1	; redraw request ?
	je	ctr003

	cmp	eax,2	; key in buffer ?
	je	ctr004

	cmp	eax,3	; button in buffer ?
	je	ctr005
; Any data in the UDP receive buffer?
	mcall	53,2,[socketNum]
	cmp	eax,0
	je	ctr001

; we have data - this will be the response
ctr002:
	mcall	53,3,[socketNum]	; read byte - block (high byte)
; Store the data in the response buffer
	mov	eax,[dnsMsgLen]
	mov	[eax],bl
	inc	dword [dnsMsgLen]

	mcall	53,2,[socketNum]	; any more data?
	cmp	eax,0
	jne	ctr002	; yes, so get it

; close	socket
	mcall	53,1,[socketNum]
	mov	[socketNum],dword  0xFFFF
; Now parse the message to get the host IP
; Man, this is complicated. It's described in
; RFC 1035
if	DEBUGGING_STATE = DEBUGGING_ENABLED
	mov	esi,str4
	call	debug_print_string
end	if

    ; 1) Validate that we have an answer with > 0 responses
    ; 2) Find the answer record with TYPE 0001 ( host IP )
    ; 3) Finally, copy the IP address to the display
    ; Note: The response is in dnsMsg
    ;       The end of the buffer is pointed to by [dnsMsgLen]

; Clear the IP address text
	mov	[server_ip],dword  0
	mov	esi,dnsMsg
; Is this a response to my question?
	mov	al,[esi+2]
	and	al,0x80
	cmp	al,0x80
	jne	ctr002a
; Were there any errors?
	mov	al,[esi+3]
	and	al,0x0F
	cmp	al,0x00
	jne	ctr002a
; Is there ( at least 1 ) answer?
	mov	ax,[esi+6]
	cmp	ax,0x00
	je	ctr002a
; Header valdated. Scan through and get my answer
if	DEBUGGING_STATE = DEBUGGING_ENABLED
	pusha
	mov	esi,str4
	call	debug_print_string
	popa
end	if
	add	esi,12	; Skip to the question field
; Skip through the question field
	call	skipName
	add	esi,4	; skip past the questions qtype, qclass

ctr002z:
; Now at the answer. There may be several answers,
; find the right one ( TYPE = 0x0001 )
	call	skipName
	mov	ax,[esi]
	cmp	ax,0x0100	; Is this the IP address answer?
	jne	ctr002c
; Yes! Point esi to the first byte of the IP address
	add	esi,10
	mov	eax,[esi]
	mov	[server_ip],eax
	ret

ctr002c:	; Skip through the answer, move to the next
	add	esi,8
	movzx	eax,byte [esi+1]
	mov	ah,[esi]
	add	esi,eax
	add	esi,2
; Have we reached the end of the msg?
; This is an error condition, should not happen
	cmp	esi,[dnsMsgLen]
	jl	ctr002z	; Check next answer
	jmp	ctr002a	; abort

ctr002a:
	jmp	ctr001

ctr003:	; redraw
	call	draw_window
	jmp	ctr001

ctr004:	; key
	mcall	2	; just read it and ignore
	jmp	ctr001

ctr005:	; button
	mcall	17	; get id
	mov	dl,ah

	; close	socket
	mcall	53,1,[socketNum]
	cmp	dl,1
	je	exit

	mov	[socketNum],dword  0xFFFF
	mov	[server_ip],dword  0
	ret

;***************************************************************************
;   Function
;      skipName
;
;   Description
;       Increment esi to the first byte past the name field
;       Names may use compressed labels. Normally do.
;       RFC 1035 page 30 gives details
;
;***************************************************************************
skipName:
	mov	al,[esi]
	cmp	al,0
	je	sn_exit
	and	al,0xc0
	cmp	al,0xc0
	je	sn001

	movzx	eax,byte [esi]
	inc	eax
	add	esi,eax
	jmp	skipName

sn001:
	add	esi,2	; A pointer is always at the end
	ret

sn_exit:
	inc	esi
	ret

;***************************************************************************
;   Function
;       load_settings
;
;   Description
;       Load settings from configuration file network.ini
;
;***************************************************************************
load_settings:
	stdcall	dll.Load,@IMPORT
	test	eax,eax
	jnz	ls001
	invoke	ini.get_str,inifile,sec_proxy,key_proxy,proxyAddr,256,proxyAddr
	invoke	ini.get_int,inifile,sec_proxy,key_proxyport,80
	mov	[proxyPort],eax
	invoke	ini.get_str,inifile,sec_proxy,key_user,	proxyUser,256,proxyUser
	invoke	ini.get_str,inifile,sec_proxy,key_password,proxyPassword,256,proxyPassword
ls001:
	ret

;***************************************************************************
;   Function
;       append_proxy_auth_header
;
;   Description
;       Append header to HTTP request for proxy authentification
;
;***************************************************************************
append_proxy_auth_header:
	mov	esi,proxy_auth_basic
	mov	ecx,proxy_auth_basic_end - proxy_auth_basic
	rep	movsb
; base64-encode string <user>:<password>
	mov	esi,proxyUser

apah000:
	lodsb
	test	al,al
	jz	apah001
	call	encode_base64_byte
	jmp	apah000

apah001:
	mov	al,':'
	call	encode_base64_byte
	mov	esi,proxyPassword

apah002:
	lodsb
	test	al,al
	jz	apah003
	call	encode_base64_byte
	jmp	apah002

apah003:
	call	encode_base64_final
	ret

encode_base64_byte:
	inc	ecx
	shl	edx,8
	mov	dl,al
	cmp	ecx,3
	je	ebb001
	ret

ebb001:
	shl	edx,8
	inc	ecx

ebb002:
	rol	edx,6
	xor	eax,eax
	xchg	al,dl
	mov	al,[base64_table+eax]
	stosb
	loop	ebb002
	ret

encode_base64_final:
	mov	al,0
	test	ecx,ecx
	jz	ebf000
	call	encode_base64_byte
	test	ecx,ecx
	jz	ebf001
	call	encode_base64_byte
	mov	byte [edi-2],'='

ebf001:
	mov	byte [edi-1],'='

ebf000:
	ret

if	DEBUGGING_STATE = DEBUGGING_ENABLED

;****************************************************************************
;    Function
;       debug_print_string
;
;   Description
;       prints a string to the debug board, in quotes
;
;       esi holds ptr to msg to display, which is space or 0 terminated
;
;       Nothing preserved; I'm assuming a pusha/popa is done before calling
;
;****************************************************************************
debug_print_string:
	push	esi
	mov	cl,'"'
	mcall	63,1
	pop	esi

dps_000:
	mov	cl,[esi]
	cmp	cl,0
	je	dps_exit

	cmp	cl,' '
	je	dps_exit
	jmp	dps_001

dps_exit:
	mov	cl,'"'
	mcall	63,1
	mov	cl,13
	mcall	
	mov	cl,10
	mcall
	ret

dps_001:
	push	esi
	mcall	63,1
	pop	esi
	inc	esi
	jmp	dps_000
end	if

;****************************************************************************
;    Function
;       print_text
;
;   Description
;       display the url (full path) text
;
;****************************************************************************
print_text:
; Draw a bar to blank out previous text
	mcall	13, <30,520>, <[ya], 9>,0xFFFFFF
; write text
	mcall	4, <30,[ya]>, 0,[addr],URLMAXLEN
	ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:

	mcall	12,1 ; start window redraw

;	cmp	[params],byte 0
;	jz	.noret

;.noret:
; DRAW	WINDOW
	mcall	0,<50,570>,<350,200>,0x14ffffff,0,title
; eax	function 4: write text to window
; ebx	[x start] *65536 + [y start]
; ecx	color of text RRGGBB
; edx	pointer to text beginning
; esi	max lenght
	xor	ecx,ecx
	mcall	4,<30,38>,,document_user,URLMAXLEN

;	xor	edx,edx
;	mcall	38,<5,545>,<60,60>

;	mov	ecx,[winys]
;	shl	ecx,16
;	add	ecx,[winys]
;	sub	ecx,26*65536+26
;	mcall	38,<5,545>

; RELOAD
	mcall	8,<388,50>,<54,14>,22,0x5588dd
; URL
	mcall	,<10,12>,<34,12>,10
; STOP
	mcall	,<443,50>,<54,14>,24
; SAVE
	mcall	,<498,50>,,26
; BUTTON TEXT
	mcall	4,<390,58>,0xffffff,button_text,30
	call	display_page

	mcall	12,2 ; end window redraw
	ret
;-----------------------------------------------------------------------------
; Data area
;-----------------------------------------------------------------------------
align	4
@IMPORT:

library	libini,'libini.obj'

import	libini, \
	ini.get_str,'ini_get_str', \
	ini.get_int,'ini_get_int'

;---------------------------------------------------------------------
fileinfo	dd 2,0,0
final_size	dd 0
final_buffer	dd 0
		db '/rd/1/.download',0
	
body_pos	dd 0

;fileinfo_tmp	dd 2,0,0
buf_size	dd 0
buf_ptr		dd 0
;		db	'/rd/1/1',0

deba		dd 0
		db 0
;---------------------------------------------------------------------
if	DEBUGGING_STATE = DEBUGGING_ENABLED
str1:		db "Resolving...",0
str3:		db "Resolved",0
str2:		db "GotIP",0
str4:		db "GotResponse",0
end	if
;---------------------------------------------------------------------
button_text	db ' RELOAD    STOP     SAVE      '
dpx		dd 25	; x - start of html page in pixels in window
dpy		dd 65	; for	y
lastletter	db 0
pageyinc	dd 0
display_from	dd 20
pos		dd 0x0
pagex		dd 0x0
pagey		dd 0x0
pagexs		dd 80
command_on_off	dd 0x0
text_type	db 1
com2		dd 0x0
script		dd 0x0
socket		dd 0x0

addr		dd 0x0
ya		dd 0x0
len		dd 0x00

title		db 'Network Downloader',0

server_ip:	db 207,44,212,20
;dns_ip:	db 194,145,128,1
;---------------------------------------------------------------------
;webAddr:
;times URLMAXLEN db ' '
;db	0

;document:	db '/'
;times URLMAXLEN-1 db ' '
;---------------------------------------------------------------------
s_contentlength	db 'Content-Length:'
len_contentlength = 15

s_chunked	db 'Transfer-Encoding: chunked'
len_chunked	= $ - s_chunked

is_body		dd 0	; 0 if headers, 1 if content
is_chunked	dd 0
prev_chunk_end	dd 0
cur_chunk_size	dd 0

string0:	db 'GET '

stringh:        db ' HTTP/1.1',13,10,'Host: '
stringh_end:
proxy_auth_basic:	db 13,10,'Proxy-Authorization: Basic '
proxy_auth_basic_end:
connclose:	db 13,10,'User-Agent: Kolibrios Downloader',13,10,'Connection: Close',13,10,13,10
connclose_end:

base64_table	db 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'
		db '0123456789+/'

inifile		db '/sys/network/zeroconf.ini',0

sec_proxy:
key_proxy	db 'proxy',0
key_proxyport	db 'port',0
key_user	db 'user',0
key_password	db 'password',0


proxyPort	dd 80

shared_name	dd 0

;yandex:	db 'menuetos.net'
;yandex_end:

status		dd 0x0
prev_status	dd 0x0

onoff		dd 0x0

nextupdate:	dd 0
winys:		dd 400

dnsMsgLen:	dd 0
socketNum:	dd 0xFFFF
;---------------------------------------------------------------------
document_user:	db 'Click on the button to the left to enter a URL',0
;---------------------------------------------------------------------
IM_END:
	rb URLMAXLEN-(IM_END - document_user)
;---------------------------------------------------------------------
align 4
document:
	rb URLMAXLEN
;---------------------------------------------------------------------
align 4
webAddr:
	rb URLMAXLEN+1
;---------------------------------------------------------------------
align 4
primary_buf:
	rb primary_buffer_size
;---------------------------------------------------------------------
align 4
params:		; db 1024 dup(0)
	rb 1024
;---------------------------------------------------------------------
align 4
request:	; db 256 dup(0)
	rb 256
;---------------------------------------------------------------------
align 4
proxyAddr:	; db 256 dup(0)
	rb 256
;---------------------------------------------------------------------
align 4
proxyUser:	; db 256 dup(0)
	rb 256
;---------------------------------------------------------------------
align 4
proxyPassword:	; db 256 dup(0)
	rb 256
;---------------------------------------------------------------------
align 4
dnsMsg:
	rb 4096
;	rb 0x100000
;---------------------------------------------------------------------
align 4
	rb 4096
stacktop:
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------