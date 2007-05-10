;
;    DHCP Client
;
;    Compile with FASM for Menuet
;

include 'lang.inc'
include '..\..\..\macros.inc'

use32

               org    0x0

	       db     'MENUET01'	    ; 8 byte id
	       dd     0x01		    ; header version
	       dd     START		    ; start of code
	       dd     I_END		    ; size of image
	       dd     I_END+0x8000	    ; memory for app
	       dd     I_END+0x8000	    ; esp
	       dd     0x0 , 0x0 	    ; I_Param , I_Icon


START:                                      ; start of execution
    mov     eax,40                          ; Report events
    mov     ebx,10000111b                   ; Stack 8 + defaults
    int     0x40

red:                            ; redraw
    call    draw_window                     ; at first, draw the window

still:
    mov     eax,10                          ; wait here for event
    mcall

    cmp     eax,1                           ; redraw request ?
    jz      red
    cmp     eax,2                           ; key in buffer ?
    jz      key
    cmp     eax,3                           ; button in buffer ?
    jz      button

    jmp     still
key:                            ; Keys are not valid at this part of the
    mov     eax,2               ; loop. Just read it and ignore
    mcall
    jmp     still

button:                         ; button
    mov     eax,17              ; get id
    mcall

    cmp     ah,1                ; button id=1 ?
    jnz     noclose

    ; close socket before exiting
    mov     eax, 53
    mov     ebx, 1
    mov     ecx, [socketNum]
    mcall

    mov     eax,0xffffffff      ; close this program
    mcall

noclose:
    cmp     ah,3                ; Resolve address?
    jnz     still

    call    draw_window

    call    contactDHCPServer

    jmp     still


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
    mov     edx, dhcpMsg

    mov     eax, [edx+16]
    mov     [dhcpClientIP], eax

    ; Scan options

    add     edx, 240        ; Point to first option

pr001:
    ; Get option id
    mov     al, [edx]
    cmp     al, 0xff        ; End of options?
    je      pr_exit

    cmp     al, 53          ; Msg type is a single byte option
    jne     pr002

    mov     al, [edx+2]
    mov     [dhcpMsgType], al
    add     edx, 3
    jmp     pr001           ; Get next option

pr002:
    ; All other (accepted) options are 4 bytes in length
    inc     edx
    movzx   ecx, byte [edx]
    inc     edx             ; point to data

    cmp     al, 54          ; server id
    jne     pr0021
    mov     eax, [edx]      ; All options are 4 bytes, so get it
    mov     [dhcpServerIP], eax
    jmp     pr003

pr0021:
    cmp     al, 51          ; lease
    jne     pr0022
    mov     eax, [edx]      ; All options are 4 bytes, so get it
    mov     [dhcpLease], eax
    jmp     pr003

pr0022:
    cmp     al, 1           ; subnet mask
    jne     pr0023
    mov     eax, [edx]      ; All options are 4 bytes, so get it
    mov     [dhcpSubnet], eax
    jmp     pr003

pr0023:
    cmp     al, 6           ; dns ip
    jne     pr0024
    mov     eax, [edx]      ; All options are 4 bytes, so get it
    mov     [dhcpDNSIP], eax

pr0024:
    cmp     al, 3           ; gateway ip
    jne     pr003
    mov     eax, [edx]      ; All options are 4 bytes, so get it
    mov     [dhcpGateway], eax

pr003:
    add     edx, ecx
    jmp     pr001

pr_exit:
    ret


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

    mov     [edx], byte 0x01                ; Boot request
    mov     [edx+1], byte 0x01              ; Ethernet
    mov     [edx+2], byte 0x06              ; Ethernet h/w len
    mov     [edx+4], dword 0x11223344       ; xid
    mov     [edx+10], byte 0x80             ; broadcast flag set
    mov     [edx+236], dword 0x63538263     ; magic number

    ; option DHCP msg type
    mov     [edx+240], word 0x0135
    mov     al, [dhcpMsgType]
    mov     [edx+240+2], al

    ; option Lease time = infinity
    mov     [edx+240+3], word 0x0433
    mov     eax, [dhcpLease]
    mov     [edx+240+5], eax

    ; option requested IP address
    mov     [edx+240+9], word 0x0432
    mov     eax, [dhcpClientIP]
    mov     [edx+240+11], eax

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
    jmp     br_exit

br001:
    ; "Request" options

    ; server IP
    mov     [edx+240+21], word 0x0436
    mov     eax, [dhcpServerIP]
    mov     [edx+240+23], eax

    ; end of options marker
    mov     [edx+240+27], byte 0xff

    mov     [dhcpMsgLen], dword 268

br_exit:
    ret



;***************************************************************************
;   Function
;      contactDHCPServer
;
;   Description
;       negotiates settings with a DHCP server
;
;***************************************************************************
contactDHCPServer:
    ; First, open socket
    mov     eax, 53
    mov     ebx, 0
    mov     ecx, 68                 ; local port dhcp client
    mov     edx, 67                 ; remote port - dhcp server
    mov     esi, 0xffffffff         ; broadcast
    mcall

    mov     [socketNum], eax

    ; Setup the first msg we will send
    mov     [dhcpMsgType], byte 0x01 ; DHCP discover
    mov     [dhcpLease], dword 0xffffffff
    mov     [dhcpClientIP], dword 0
    mov     [dhcpServerIP], dword 0

    call    buildRequest

ctr000:
    ; write to socket ( send broadcast request )
    mov     eax, 53
    mov     ebx, 4
    mov     ecx, [socketNum]
    mov     edx, [dhcpMsgLen]
    mov     esi, dhcpMsg
    mcall

    ; Setup the DHCP buffer to receive response

    mov     eax, dhcpMsg
    mov     [dhcpMsgLen], eax      ; Used as a pointer to the data

    ; now, we wait for
    ; UI redraw
    ; UI close
    ; or data from remote

ctr001:
    mov     eax,10                 ; wait here for event
    mcall

    cmp     eax,1                  ; redraw request ?
    je      ctr003
    cmp     eax,2                  ; key in buffer ?
    je      ctr004
    cmp     eax,3                  ; button in buffer ?
    je      ctr005


    ; Any data in the UDP receive buffer?
    mov     eax, 53
    mov     ebx, 2
    mov     ecx, [socketNum]
    mcall

    cmp     eax, 0
    je      ctr001

    ; we have data - this will be the response
ctr002:
    mov     eax, 53
    mov     ebx, 3
    mov     ecx, [socketNum]
    mcall                ; read byte - block (high byte)

    ; Store the data in the response buffer
    mov     eax, [dhcpMsgLen]
    mov     [eax], bl
    inc     dword [dhcpMsgLen]

    mov     eax, 53
    mov     ebx, 2
    mov     ecx, [socketNum]
    mcall                ; any more data?

    cmp     eax, 0
    jne     ctr002              ; yes, so get it

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


    cmp     [dhcpMsgType], byte 0x01    ; did we send a discover?
    je      ctr007
    cmp     [dhcpMsgType], byte 0x03    ; did we send a request?
    je      ctr008

    ; should never get here - we only send discover or request
    jmp     ctr006

ctr007:
    call    parseResponse

    ; Was the response an offer? It should be
    cmp     [dhcpMsgType], byte 0x02
    jne     ctr006                  ; NO - so quit

    ; send request
    mov     [dhcpMsgType], byte 0x03 ; DHCP request
    call    buildRequest
    jmp     ctr000

ctr008:
    call    parseResponse

    ; Was the response an ACK? It should be
    cmp     [dhcpMsgType], byte 0x05
    jne     ctr006                  ; NO - so quit

    ; Set or display addresses here...

ctr006:
    ; close socket
    mov     eax, 53
    mov     ebx, 1
    mov     ecx, [socketNum]
    mcall

    mov     [socketNum], dword 0xFFFF

    call    draw_window

    jmp     ctr001

ctr003:                         ; redraw
    call    draw_window
    jmp     ctr001

ctr004:                         ; key
    mov     eax,2               ; just read it and ignore
    mcall
    jmp     ctr001

ctr005:                         ; button
    mov     eax,17              ; get id
    mcall

    ; close socket
    mov     eax, 53
    mov     ebx, 1
    mov     ecx, [socketNum]
    mcall

    mov     [socketNum], dword 0xFFFF

    call    draw_window                     ; at first, draw the window

    ret




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


; Pass in the IP address in edi
; row to display in [ya]
drawIP:
;    mov     edi,hostIP
    mov     ecx, edi
    add     ecx, 4
    mov     edx,[ya]
    add     edx, 97*65536
    mov     esi,0x00ffffff
    mov     ebx,3*65536

ipdisplay:
    mov     eax,47
    push    ecx
    movzx   ecx,byte [edi]
    mcall
    pop     ecx
    add     edx,6*4*65536
    inc     edi
    cmp     edi,ecx
    jb      ipdisplay
    ret


drawDHMS:

    mov     eax,[edi]
    bswap   eax

    mov     esi,dhms
    mov     ecx,16
    mov     edi,text+40*4+12
    cmp     eax,0xffffffff
    jne     nforever
    mov     esi,forever
    cld
    rep     movsb
    ret
   nforever:
    cld
    rep     movsb

    mov     ecx,28
    xor     edx,edx
    mov     ebx,60
    div     ebx
    call    displayDHMS
    xor     edx,edx
    div     ebx
    call    displayDHMS
    xor     edx,edx
    mov     ebx,24
    div     ebx
    call    displayDHMS
    mov     edx,eax
    call    displayDHMS

    ret


displayDHMS:

    pusha
    mov     eax,47
    mov     ebx,3*65536
    mov     edx,ecx
    imul    edx,6
    shl     edx,16
    add     edx,1*65536+99
    mov     ecx,[esp+20]
    mov     esi,0xffffff
    mcall
    popa
    sub     ecx,4
    ret


draw_window:

    mov     eax,12                    ; function 12:tell os about windowdraw
    mov     ebx,1                     ; 1, start of draw
    mcall
                                      ; DRAW WINDOW
    mov     eax,0                     ; function 0 : define and draw window
    mov     ebx,100*65536+300         ; [x start] *65536 + [x size]
    mov     ecx,100*65536+156         ; [y start] *65536 + [y size]
    mov     edx,0x13224466            ; color of work area RRGGBB
    mov     edi,title                 ; WINDOW LABEL
    mcall
                                      
    mov     eax,8                     ; Resolve
    mov     ebx,20*65536+90
    mov     ecx,127*65536+15
    mov     edx,3
    mov     esi,0x557799
    mcall

    ; Pass in the IP address in edi
    ; row to display in [ya]
    mov     edi, dhcpClientIP
    mov     eax, 35
    mov     [ya], eax
    call    drawIP
    mov     edi, dhcpGateway
    mov     eax, 35 + 16
    mov     [ya], eax
    call    drawIP
    mov     edi, dhcpSubnet
    mov     eax, 35 + 32
    mov     [ya], eax
    call    drawIP
    mov     edi, dhcpDNSIP
    mov     eax, 35 + 48
    mov     [ya], eax
    call    drawIP
    mov     edi, dhcpLease
    call    drawDHMS

    ; Re-draw the screen text
    cld
    mov     eax,4
    mov     ebx,25*65536+35           ; draw info text with function 4
    mov     ecx,0xffffff
    mov     edx,text
    mov     esi,40
newline:
    mcall
    add     ebx,16
    add     edx,40
    cmp     [edx],byte 'x'
    jnz     newline


    mov     eax,12                    ; function 12:tell os about windowdraw
    mov     ebx,2                     ; 2, end of draw
    mcall

    ret



; DATA AREA

ya              dd  0x0

text:
    db 'Client IP :    .   .   .                '
    db 'Gateway IP:    .   .   .                '
    db 'Subnet    :    .   .   .                '
    db 'DNS IP    :    .   .   .                '
    db 'Lease Time:    d   h   m   s            '
    db '                                        '
    db ' SEND REQUEST                           '
    db 'x <- END MARKER, DONT DELETE            '


dhms      db   '   d   h   m   s'
forever   db   'Forever         '

title   db   'DHCP Client Test',0

dhcpMsgType:    db  0
dhcpLease:      dd  0
dhcpClientIP:   dd  0
dhcpServerIP:   dd  0
dhcpDNSIP:      dd  0
dhcpSubnet:     dd  0
dhcpGateway:    dd  0

dhcpMsgLen:     dd  0
socketNum:      dd  0xFFFF
dhcpMsg:
I_END:




