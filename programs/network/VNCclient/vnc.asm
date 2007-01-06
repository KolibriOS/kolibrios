;
;
; VNC Client for kolibrios by hidnplayr
;
;
; WORK IN PROGRESS...
;
; FEEL FREE TO CONTRIBUTE !
;
; hidnplayr@gmail.com
;

use32

		org	0x0

		db	'MENUET00'		; 8 byte id
		dd	38			; required os
		dd	START			; program start
		dd	I_END			; program image size
		dd	IM_END			; required amount of memory
		dd	0			; reserved=no extended header

__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 1
STRLEN = 64
xpos = 4
ypos = 22

TIMEOUT = 60	 ; timeout in seconds
BUFFER	= 512	 ; Buffer size for DNS

include 'fdo.inc'
include 'ETH.INC'
include 'logon.inc'
include 'raw.inc'
include 'copyrect.inc'


START:				       ; start of execution
    call    red_logon

    mov     eax,40		       ; Report events
    mov     ebx,10000000b	       ; Only Stack
    int     0x40

    mov     eax,67		       ; resize the window (hide it)
    xor     ebx,ebx
    mov     ecx,ebx
    mov     edx,ebx
    mov     esi,ebx
    int     0x40

    resolve first,[server_ip]	       ; the input window putted the server @ 'first', resolve it into a real ip
    mov     [server_port],5900	       ; no port input for now, only standard port 5900

    DEBUGF  1,'connecting to %u.%u.%u.%u:%u\n',1[server_ip],1[server_ip+1],1[server_ip+2],1[server_ip+3],4[server_port]
    eth.search_port 1000,edx					  ; Find a free port starting from 1001 and store in edx
    eth.open_tcp edx,[server_port],[server_ip],1,[socket]	  ; open socket
    DEBUGF 1,'Socket opened: %u (port %u)\n',[socket],ecx

    call    read_data
    cmp     dword[receive_buffer+1],'RFB '
    jne     no_rfb
    eth.write_tcp [socket],12,handshake
    DEBUGF 1,'Sending handshake: protocol version\n'

    call    read_data
    mov     eax,receive_buffer+1
    mov     eax,[eax]
    bswap   eax
    cmp     eax,0
    je	    invalid_security
    cmp     eax,1
    je	    no_security
    cmp     eax,2
    je	    vnc_security

    jmp     close

   vnc_security:
    mov     byte[mode],1
    call    red_logon

   no_security:
    eth.write_tcp [socket],1,shared
    DEBUGF 1,'Sending handshake: shared session?\n'

    eth.wait_for_data [socket],TIMEOUT*10,close
    eth.read_data [socket],framebuffer,[datapointer],IM_END-receive_buffer ; now the server should send init message
    DEBUGF 1,'Serverinit: bpp:%u depth:%u bigendian:%u truecolor:%u\n',1[pixelformat.bpp],1[pixelformat.depth],1[pixelformat.big_endian],1[pixelformat.true_color]
    mov     eax,dword[framebuffer]
    bswap   eax
    mov     dword[screen],eax

    eth.write_tcp [socket],20,pixel_format32
    DEBUGF 1,'Sending pixel format\n'
    call    read_data

;    eth.write_tcp [socket],8,encodings
;    DEBUGF 1,'Sending encoding info\n'
;    call    read_data

    mov     eax,dword[framebuffer.width]
    mov     dword[fbur.width],eax

    mov     eax,40		   ; report events
    mov     ebx,10100111b	   ; stack, mouse, button, key, redraw
    int     0x40

    mov     eax,67		   ; resize the window
    mov     ebx,10
    mov     ecx,10
    mov     edx,dword[framebuffer]
    bswap   edx
    movzx   esi,dx
    shr     edx,16
    add     edx,2*xpos
    add     esi,ypos+xpos
    int     0x40

;    mov     byte[fbur.inc],0       ; request a framebufferupdate
;    eth.write_tcp [socket],10,fbur

  mainloop:
    eth.socket_status [socket],eax
    cmp     al,TCB_CLOSE_WAIT
    je	    close

    mov     eax,23		   ; wait for event with timeout
    mov     ebx,50		   ; 0,5 s
    int     0x40

    cmp     eax,1		   ; redraw
    je	    redraw
    cmp     eax,2		   ; key
    je	    key
    cmp     eax,3		   ; button
    je	    button
    cmp     eax,6		   ; mouse
    je	    mouse
    cmp     eax,8
    je	    network

    ; request an FRB update
    jmp     mainloop


   network:
    call    read_data		   ; Read the data into the buffer

    mov     eax,[datapointer]	   ; at least 2 bytes should be received
    sub     eax,receive_buffer
    cmp     eax,1
    jle     mainloop

    DEBUGF 1,'Data received, %u bytes\n',eax

    cmp     byte[receive_buffer],0
    je	    framebufferupdate

    cmp     byte[receive_buffer],1
    je	    setcolourmapentries

    cmp     byte[receive_buffer],2
    je	    bell

    cmp     byte[receive_buffer],3
    je	    servercuttext

    jmp     mainloop


   framebufferupdate:
    DEBUGF 1,'Framebufferupdate!\n'
    mov     di,word[receive_buffer+2]
    bswap   edi
    shr     edi,16
    mov     esi,receive_buffer+4

   rectangle_loop:
    mov     edx,[esi]
    bswap   edx
    mov     ebx,edx
    shr     edx,16
    mov     [frame.x],dx
    mov     [frame.y],bx
    add     esi,4
    mov     ecx,[esi]
    bswap   ecx
    mov     eax,ecx
    shr     ecx,16
    mov     [frame.width],cx
    mov     [frame.height],ax
    add     esi,4
    mov     eax,[esi]
    add     esi,4

    DEBUGF 1,'screen: width=%u height=%u\nframe: width=%u height=%u x=%u y=%u\n',2[screen.width],2[screen.height],2[frame.width],2[frame.height],2[frame.x],2[frame.y]

    cmp     eax,0
    je	    encoding_raw

    cmp     eax,1
    je	    encoding_copyrect

    cmp     eax,2
    je	    encoding_RRE

    cmp     eax,5
    je	    encoding_hextile

    cmp     eax,16
    je	    encoding_ZRLE

    DEBUGF 1,'FRAME: unknown encoding\n'
    jmp     mainloop

   next_rectangle:
    dec     di
    pusha
    call    drawbuffer
    popa
    cmp     di,0
    jg	    rectangle_loop
    jmp     mainloop

  encoding_RRE:
    DEBUGF 1,'FRAME: RRE\n'

    jmp     next_rectangle

  encoding_hextile:
    DEBUGF 1,'FRAME: hextile\n'

    jmp     next_rectangle

  encoding_ZRLE:
    DEBUGF 1,'FRAME: ZRLE\n'

    jmp     next_rectangle


  setcolourmapentries:

    DEBUGF 1,'Server sended an SetColourMapEntries message\n'

    jmp     mainloop


  bell:
    mov     eax,55
    mov     ebx,eax
    mov     esi,beep
    int     0x40

    jmp     mainloop


  servercuttext:

    jmp mainloop


  key:
    DEBUGF 1,'Sending key event\n'

    mov     eax,2
    int     0x40
    mov     byte[keyevent.key+3],ah

;   eth.write_tcp [socket],8,keyevent

    cmp     ah,13
    jne     @f

    mov     byte[fbur.inc],1
    eth.write_tcp [socket],10,fbur
    jmp     mainloop
  @@:

    cmp     ah,30
    jne     @f

    mov     byte[fbur.inc],12
    eth.write_tcp [socket],10,fbur
    jmp     mainloop
  @@:

    mov     byte[fbur.inc],0
    eth.write_tcp [socket],10,fbur

    jmp     mainloop

  mouse:
    DEBUGF 1,'Sending mouse event\n'

    mov     eax,37
    mov     ebx,1
    int     0x40

    sub     eax,xpos*65536+ypos
    bswap   eax
    mov     word[pointerevent.x],ax
    shr     eax,16
    mov     word[pointerevent.y],ax

    mov     eax,37
    mov     ebx,2
    int     0x40

    cmp     al,2
    jne     @f	      ; in kolibri right click is 2 (decimal), in RFB protocol it is bit 2 (counting from 0)
    mov     al,100b
   @@:

    mov     byte[pointerevent.mask],al

    eth.write_tcp [socket],6,pointerevent

    jmp     mainloop

  redraw:

    DEBUGF 1,'Drawing window\n'

    mov     eax,12
    mov     ebx,1
    int     0x40

    mov     eax,0		      ; draw window
    mov     ebx,dword[framebuffer]
    bswap   ebx
    movzx   ecx,bx
    shr     ebx,16
    add     ebx,2*xpos
    add     ecx,ypos+xpos
    mov     edx,0x03ffffff
    mov     esi,0x80555599
    mov     edi,0x00ffffff
    int     0x40

    mov     eax,4		      ; label
    mov     ebx,9*65536+8
    mov     ecx,0x10ffffff
    mov     edx,name
    mov     esi,[name_length]
    bswap   esi
    int     0x40

    call    drawbuffer

    mov     eax,12
    mov     ebx,2
    int     0x40

    jmp     mainloop

  drawbuffer:

    mov     eax,7
    mov     ebx,framebuffer_data
    mov     ecx,dword[screen]
    mov     edx,xpos*65536+ypos
    int     0x40

    ret


  button:			  ; button
    mov     eax,17		  ; get id
    int     0x40

  close:
    call    read_data
;    eth.close_tcp [socket]             ; We're done, close the socket ;;; BUG WHEN CLOSING SCOKET !!
    DEBUGF 1,'Socket closed\n'

    mov     eax,-1
    int     0x40

    no_rfb:
    DEBUGF 1,'This is no vnc server!\n'
    jmp     close

    invalid_security:
    DEBUGF 1,'Security error: %s\n',receive_buffer+5
    jmp     close

read_data:
    eth.read_data [socket],receive_buffer,[datapointer],IM_END-receive_buffer
ret

; DATA AREA

include_debug_strings	 ; ALWAYS present in data section

handshake	   db 'RFB 003.003',0x0a
shared		   db 0
beep		   db 0x85,0x25,0x85,0x40,0

pixel_format32	   db 0       ; setPixelformat
		   rb 3       ; padding
.bpp		   db 32      ; bits per pixel
.depth		   db 32      ; depth
.big_endian	   db 0       ; big-endian flag
.true_color	   db 1       ; true-colour flag
.red_max	   db 0,255   ; red-max
.green_max	   db 0,255   ; green-max
.blue_max	   db 0,255   ; blue-max
.red_shif	   db 0       ; red-shift
.green_shift	   db 8       ; green-shift
.blue_shift	   db 16      ; blue-shift
		   rb 3       ; padding

pixel_format16	   db 0       ; setPixelformat
		   rb 3       ; padding
.bpp		   db 16      ; bits per pixel
.depth		   db 16      ; depth
.big_endian	   db 0       ; big-endian flag
.true_color	   db 1       ; true-colour flag
.red_max	   db 0,32    ; red-max
.green_max	   db 0,32    ; green-max
.blue_max	   db 0,64    ; blue-max
.red_shif	   db 0       ; red-shift
.green_shift	   db 5       ; green-shift
.blue_shift	   db 10      ; blue-shift
		   rb 3       ; padding

pixel_format8	   db 0       ; setPixelformat
		   rb 3       ; padding
.bpp		   db 8       ; bits per pixel
.depth		   db 8       ; depth
.big_endian	   db 0       ; big-endian flag
.true_color	   db 1       ; true-colour flag
.red_max	   db 0,7     ; red-max
.green_max	   db 0,7     ; green-max
.blue_max	   db 0,3     ; blue-max
.red_shif	   db 0       ; red-shift
.green_shift	   db 3       ; green-shift
.blue_shift	   db 6       ; blue-shift
		   rb 3       ; padding

encodings	   db 2       ; setEncodings
		   rb 1       ; padding
		   db 1,0     ; number of encodings
		   db 0,0,0,0 ; raw encoding        (DWORD, Big endian order)
		   db 1,0,0,0 ; Copyrect encoding

fbur		   db 3       ; frame buffer update request
.inc		   db 0       ; incremental
.x		   dw 0
.y		   dw 0
.width		   dw 0
.height 	   dw 0

keyevent	   db 4       ; keyevent
.down		   db 0       ; down-flag
		   dw 0       ; padding
.key		   dd 0       ; key

pointerevent	   db 5       ; pointerevent
.mask		   db 0       ; button-mask
.x		   dw 0       ; x-position
.y		   dw 0       ; y-position

I_END:

framebuffer:
.width		   dw ?
.height 	   dw ?
pixelformat:
.bpp		   db ?
.depth		   db ?
.big_endian	   db ?
.true_color	   db ?
.red_max	   dw ?
.green_max	   dw ?
.blue_max	   dw ?
.red_shift	   db ?
.green_shift	   db ?
.blue_shift	   db ?
.padding	   rb 3
name_length	   dd ?
name		   rb 256

server_ip	   dd 0
server_port	   dd 0
socket		   dd 0
datapointer	   dd 0

frame:
.width		   dw 0
.height 	   dw 0
.x		   dw 0
.y		   dw 0

screen:
.height 	   dw 0
.width		   dw 0

dnsMsg:
receive_buffer	   rb 5*1024*1024 ; 5 mb buffer for received data (incoming frbupdate etc)
framebuffer_data   rb 1024*768*3  ; framebuffer

IM_END:


