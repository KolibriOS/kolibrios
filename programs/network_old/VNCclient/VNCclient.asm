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

    org     0x0

    db	   'MENUET01'	     ; 8 byte id
    dd	   0x01 	     ; header version
    dd	   START	     ; start of code
    dd	   I_END	     ; size of image
    dd	   IM_END	     ; memory for app
    dd	   IM_END	     ; esp
    dd	   0x0 , 0x0	     ; I_Param , I_Icon

__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 1

STRLEN = 64	 ; password and server max length
xpos = 4	 ; coordinates of image
ypos = 22	 ;

TIMEOUT = 60	 ; timeout in seconds
BUFFER	= 1500	 ; Buffer size for DNS

include '..\..\macros.inc'
include 'fdo.inc'
include 'ETH.INC'
include 'logon.inc'
include 'raw.inc'
include 'copyrect.inc'
include 'thread.inc'

START:				       ; start of execution

    call    red_logon

    mov     eax,40		       ; Report events
    mov     ebx,00000000b	       ; Only Stack
    mcall

    mov     eax,67		       ; resize the window (hide it)
    xor     ebx,ebx
    mov     ecx,ebx
    mov     edx,ebx
    mov     esi,ebx
    mcall

    mov     eax,51
    mov     ebx,1
    mov     ecx,thread_start
    mov     edx,thread_stack
    mcall

    DEBUGF 1,'Thread created: %u\n',eax

   @@:
    mov     eax,5
    mov     ebx,10
    mcall

    cmp     byte[thread_ready],0
    je	    @r

    mov     eax,40		   ; report events
    mov     ebx,100111b 	   ; mouse, button, key, redraw
    mcall

    mov     eax,67		   ; resize the window
    mov     ebx,10
    mov     ecx,10
    mov     edx,dword[framebuffer]
    bswap   edx
    movzx   esi,dx
    shr     edx,16
    add     edx,2*xpos
    add     esi,ypos+xpos
    mcall

  mainloop:
    eth.socket_status [socket],eax
    cmp     al,TCB_CLOSE_WAIT
    je	    close

    mov     eax,23		   ; wait for event with timeout
    mov     ebx,50		   ; 0,5 s
    mcall

    cmp     eax,1
    je	    redraw
    cmp     eax,2		   ; key
    je	    key
    cmp     eax,3		   ; button
    je	    button
    cmp     eax,6		   ; mouse
    je	    mouse

    call    drawbuffer

    jmp     mainloop

  key:
    DEBUGF 1,'Sending key event\n'

    mov     eax,2
    mcall
    mov     byte[keyevent.key+3],ah

    eth.write_tcp [socket],8,keyevent

    jmp     mainloop

  mouse:
    DEBUGF 1,'Sending mouse event\n'

    mov     eax,37
    mov     ebx,1
    mcall

    sub     eax,xpos*65536+ypos
    bswap   eax
    mov     word[pointerevent.x],ax
    shr     eax,16
    mov     word[pointerevent.y],ax

    mov     eax,37
    mov     ebx,2
    mcall

    test    al,00000010b	    ; test if right button was pressed  (bit 1 in kolibri)
    jz	    @f
    add     al,00000010b	    ; in RFB protocol it is bit 2, so if we add bit 2 again, we'll get bit 3 and bit 1 will remain the same
   @@:

    mov     byte[pointerevent.mask],al

    eth.write_tcp [socket],6,pointerevent

    jmp     mainloop

  redraw:

    DEBUGF 1,'Drawing window\n'

    mcall 12, 1

    mov     eax,0		      ; draw window
    mov     ebx,dword[framebuffer]
    bswap   ebx
    movzx   ecx,bx
    shr     ebx,16
    add     ebx,2*xpos
    add     ecx,ypos+xpos
    mov     edx,0xffffff
    mcall

    mov     eax,4		      ; label
    mov     ebx,9*65536+8
    mov     ecx,0x10ffffff
    mov     edx,name
    mov     esi,[name_length]
    bswap   esi
    mcall

    call    drawbuffer

    mcall 12, 2

    jmp     mainloop

  drawbuffer:

    mov     eax,7
    mov     ebx,framebuffer_data
    mov     ecx,dword[screen]
    mov     edx,xpos*65536+ypos
    mcall

    ret


  button:			  ; button
    mov     eax,17		  ; get id
    mcall

  close:
    call    read_data
;    eth.close_tcp [socket]             ; We're done, close the socket ;;; BUG WHEN CLOSING SOCKET !!
    DEBUGF 1,'Socket closed\n'

    mov     eax,-1
    mcall

    no_rfb:
    DEBUGF 1,'This is no vnc server!\n'
    jmp     close

    invalid_security:
    DEBUGF 1,'Security error: %s\n',receive_buffer+5
    jmp     close


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
.depth		   db 15      ; depth
.big_endian	   db 0       ; big-endian flag
.true_color	   db 1       ; true-colour flag
.red_max	   db 0,31    ; red-max
.green_max	   db 0,31    ; green-max
.blue_max	   db 0,31    ; blue-max
.red_shif	   db 0       ; red-shift
.green_shift	   db 5       ; green-shift
.blue_shift	   db 10      ; blue-shift
		   rb 3       ; padding

pixel_format8	   db 0       ; setPixelformat
		   rb 3       ; padding
.bpp		   db 8       ; bits per pixel
.depth		   db 6       ; depth
.big_endian	   db 0       ; big-endian flag
.true_color	   db 1       ; true-colour flag
.red_max	   db 0,3     ; red-max
.green_max	   db 0,3     ; green-max
.blue_max	   db 0,3     ; blue-max
.red_shif	   db 0       ; red-shift
.green_shift	   db 2       ; green-shift
.blue_shift	   db 4       ; blue-shift
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

thread_ready	   db 0

dnsMsg:
receive_buffer	   rb 5*1024*1024 ; 5 mb buffer for received data (incoming frbupdate etc)
framebuffer_data   rb 1024*768*3  ; framebuffer

thread_stack	   rb 0x1000

IM_END:


