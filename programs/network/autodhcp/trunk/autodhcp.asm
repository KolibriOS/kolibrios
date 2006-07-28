;
; Automated dhcp client
;
; v 1.1
;
; by the hidden player
;

DEBUG equ 1
TIMEOUT equ 60 ; in seconds

use32

	       org    0x0

	       db     'MENUET01'	      ; 8 byte id
	       dd     0x01		      ; header version
	       dd     START		      ; start of code
	       dd     IM_END		      ; size of image
	       dd     I_END		      ; memory for app
	       dd     I_END		      ; esp
	       dd     0x0 , 0x0 	      ; I_Param , I_Icon

include 'macros.inc'

if DEBUG = 1
include 'debug.inc'
end if


START:				; start of execution

    mov     eax,40		   ; Report events
    mov     ebx,10000000b	   ; Only Stack
    int     0x40

    mov     eax,52		   ; first, enable the stack
    mov     ebx,2
    mov     ecx,0x00000383
    int     0x40

if DEBUG = 1
    newline
    dps  "DHCP: Stack Initialized"
    newline
end if

    mov     eax, 53		   ; then, read in the status
    mov     ebx, 255
    mov     ecx, 6
    int     0x40

    cmp     eax,0		   ; if eax is zero, no driver was found
    jne     @f

if DEBUG = 1
    dps  "DHCP: No Card detected"
    newline
end if

    jmp  close

   @@:
if DEBUG = 1
    dps  "DHCP: Detected card: "
    dph  eax
    newline
end if

    ; now that the stack is running, lets start the dhcp request

    ; First, open socket
    mov     eax, 53
    mov     ebx, 0
    mov     ecx, 68		    ; local port dhcp client
    mov     edx, 67		    ; remote port - dhcp server
    mov     esi, -1		    ; broadcast
    int     0x40

    mov     [socketNum], eax

if DEBUG = 1
    dps   "DHCP: Socket opened: "
    dpd   eax
    newline
end if

    ; Setup the first msg we will send
    mov     byte [dhcpMsgType], 0x01 ; DHCP discover
    mov     dword [dhcpLease], esi   ; esi is still -1 (-1 = forever)

;***************************************************************************
;   Function
;      buildRequest
;
;   Description
;      Creates a DHCP request packet.
;
;***************************************************************************
buildRequest:
    ; Clear dhcpMsg to all zeros
    xor     eax,eax
    mov     edi,dhcpMsg
    mov     ecx,512
    cld
    rep     stosb

    mov     edx, dhcpMsg

    mov     [edx], byte 0x01		    ; Boot request
    mov     [edx+1], byte 0x01		    ; Ethernet
    mov     [edx+2], byte 0x06		    ; Ethernet h/w len
    mov     [edx+4], dword 0x11223344	    ; xid
    mov     [edx+10], byte 0x80 	    ; broadcast flag set
    mov     [edx+236], dword 0x63538263     ; magic number

    ; option DHCP msg type
    mov     [edx+240], word 0x0135
    mov     al, [dhcpMsgType]
    mov     [edx+240+2], al

    ; option Lease time = infinity
    mov     [edx+240+3], word 0x0433
    mov     eax, [dhcpLease]
    mov     [edx+240+5], eax

;    ; option requested IP address
    mov     [edx+240+9], word 0x0432
;    mov     eax, [dhcpClientIP]
;    mov     [edx+240+11], eax

    ; option request list
    mov     [edx+240+15], word 0x0437
    mov     [edx+240+17], dword 0x0f060301

    ; Check which msg we are sending
    cmp     [dhcpMsgType], byte 0x01
    jne     br001

    ; "Discover" options
    ; end of options marker
    mov     [edx+240+21], byte 0xff

    mov     [dhcpMsgLen], dword 262
    jmp     ctr000

br001:
    ; "Request" options

    ; server IP
    mov     [edx+240+21], word 0x0436
    mov     eax, [dhcpServerIP]
    mov     [edx+240+23], eax

    ; end of options marker
    mov     [edx+240+27], byte 0xff

    mov     [dhcpMsgLen], dword 268

ctr000:

    ; write to socket ( send broadcast request )
    mov     eax, 53
    mov     ebx, 4
    mov     ecx, [socketNum]
    mov     edx, [dhcpMsgLen]
    mov     esi, dhcpMsg
    int     0x40

    ; Setup the DHCP buffer to receive response

    mov     eax, dhcpMsg
    mov     [dhcpMsgLen], eax	   ; Used as a pointer to the data

    ; now, we wait for data from remote

wait_for_data:
    mov     eax,23		   ; wait here for event   NOTE a TIME-OUT should be placed here
    mov     ebx,TIMEOUT*100
    int     0x40

    ; Any data in the UDP receive buffer?
    mov     eax, 53
    mov     ebx, 2
    mov     ecx, [socketNum]
    int     0x40

    cmp     eax, 0
    jne     ctr002

if DEBUG = 1
    dps  "DHCP: Timeout!"
    newline
end if

    jmp    close

    ; we have data - this will be the response
ctr002:

    mov     eax, 53
    mov     ebx, 3
    mov     ecx, [socketNum]
    int     0x40		; read byte - block (high byte)

    ; Store the data in the response buffer
    mov     eax, [dhcpMsgLen]
    mov     [eax], bl
    inc     dword [dhcpMsgLen]

    mov     eax, 53
    mov     ebx, 2
    mov     ecx, [socketNum]
    int     0x40		; any more data?

    cmp     eax, 0
    jne     ctr002		; yes, so get it

    ; depending on which msg we sent, handle the response
    ; accordingly.
    ; If the response is to a dhcp discover, then:
    ;  1) If response is DHCP OFFER then
    ;  1.1) record server IP, lease time & IP address.
    ;  1.2) send a request packet
    ;  2) else exit ( display error )
    ; If the response is to a dhcp request, then:
    ;  1) If the response is DHCP ACK then
    ;  1.1) extract the DNS & subnet fields. Set them in the stack
    ;  2) else exit ( display error )


    cmp     [dhcpMsgType], byte 0x01	; did we send a discover?
    je	    discover
    cmp     [dhcpMsgType], byte 0x03	; did we send a request?
    je	    request

    ; should never get here - we only send discover or request
    jmp     close

discover:

    call    parseResponse

    ; Was the response an offer? It should be
    cmp     [dhcpMsgType], byte 0x02
    jne     close		   ; NO - so quit

    ; send request
    mov     [dhcpMsgType], byte 0x03 ; DHCP request
    jmp     buildRequest

request:

    call    parseResponse

    ; Was the response an ACK? It should be
    cmp     [dhcpMsgType], byte 0x05
    jne     close		   ; NO - so quit

close:

    ; close socket
    mov     eax, 53
    mov     ebx, 1
    mov     ecx, [socketNum]
    int     0x40

if DEBUG = 1
    dps  "DHCP: Exiting"
    newline
end if

    mov     eax,-1		   ; at last, exit
    int     0x40


;***************************************************************************
;   Function
;      parseResponse
;
;   Description
;      extracts the fields ( client IP address and options ) from
;      a DHCP response
;      The values go into
;       dhcpMsgType,dhcpLease,dhcpClientIP,dhcpServerIP,
;       dhcpDNSIP, dhcpSubnet
;      The message is stored in dhcpMsg
;
;***************************************************************************
parseResponse:

if DEBUG = 1
    dps  "DHCP: Data received, parsing response"
    newline
end if

    mov     edx, dhcpMsg

    pusha

    mov     eax,52	    ; Set Client IP
    mov     ebx,3
    mov     ecx, [edx+16]
    int     0x40

if DEBUG = 1
    dps  "DHCP: Client: "

    xor   esi,esi
   .loop:

    pusha
    movzx eax,byte[edx+esi+16]
    call  debug_outdec
    popa

    inc   esi
    cmp   esi,4
    jne   .loop

    newline
end if

    popa

    ; Scan options

    add     edx, 240	    ; Point to first option

pr001:
    ; Get option id
    mov     al, [edx]
    cmp     al, 0xff	    ; End of options?
    je	    pr_exit

    cmp     al, 53	    ; Msg type is a single byte option
    jne     pr002

    mov     al, [edx+2]
    mov     [dhcpMsgType], al
    add     edx, 3
    jmp     pr001	    ; Get next option

pr002:
    ; All other (accepted) options are 4 bytes in length
    inc     edx
    movzx   ecx, byte [edx]
    inc     edx 	    ; point to data

    cmp     al, 54	    ; server id
    jne     pr0021
    mov     eax, [edx]	    ; All options are 4 bytes, so get it
    mov     [dhcpServerIP], eax
    jmp     pr003

pr0021:
    cmp     al, 51	    ; lease
    jne     pr0022

if DEBUG = 1
    pusha
    dps  "DHCP: lease:  "

    cmp  dword[edx],-1
    jne  no_lease_forever
    dps  "forever"
    jmp  lease_newline
   no_lease_forever:
    dpd  [edx]
   lease_newline:
    newline
    popa
end if

    jmp     pr003

pr0022:
    cmp     al, 1	    ; subnet mask
    jne     pr0023

    pusha
    mov     eax,52
    mov     ebx,12
    mov     ecx,[edx]
    int     0x40


if DEBUG = 1
    dps  "DHCP: Subnet: "

    xor   esi,esi
   .loop:

    pusha
    movzx eax,byte[edx+esi]
    call  debug_outdec
    popa

    inc   esi
    cmp   esi,4
    jne   .loop

    newline
end if

    popa

    jmp     pr003

pr0023:
    cmp     al, 6	    ; dns ip
    jne     pr0024

    pusha

    mov     eax,52
    mov     ebx,14
    mov     ecx,[edx]
    int     0x40


if DEBUG = 1
    dps  "DHCP: DNS IP: "

    xor   esi,esi
   .loop:

    pusha
    movzx eax,byte[edx+esi]
    call  debug_outdec
    popa

    inc   esi
    cmp   esi,4
    jne   .loop

    newline
end if

    popa

pr0024:
    cmp     al, 3	    ; gateway ip
    jne     pr003

    pusha

    mov     eax,52
    mov     ebx,11
    mov     ecx,[edx]
    int     0x40


if DEBUG = 1
    dps  "DHCP: Gateway:"

    xor   esi,esi
   .loop:

    pusha
    movzx eax,byte[edx+esi]
    call  debug_outdec
    popa

    inc   esi
    cmp   esi,4
    jne   .loop

    newline
end if

    popa

pr003:
    add     edx, ecx
    jmp     pr001

pr_exit:

if DEBUG = 1
    dps  "DHCP: Done"
    newline
end if

    jmp close


; DATA AREA

IM_END:

dhcpMsgType:	db  0
dhcpLease:	dd  0
;dhcpClientIP:   dd  0
dhcpServerIP:	dd  0

dhcpMsgLen:	dd  0
socketNum:	dd  0xFFFF
dhcpMsg:	rb  512

I_END: