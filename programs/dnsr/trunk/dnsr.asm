;
;    DNS Domain name -> IP lookup
;
;    Compile with FASM for Menuet
;


; If you like, you camd change the DNS server default by changing the
; IP address in the dnsServer string.


; Enabling debugging puts the received response to the
; debug board
DEBUGGING_ENABLED           equ     1
DEBUGGING_DISABLED          equ     0
DEBUGGING_STATE             equ     DEBUGGING_DISABLED


use32

                org     0x0

                db      'MENUET00'              ; 8 byte id
                dd      38                      ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x100000                ; required amount of memory
                dd      0x00000000              ; reserved=no extended header

include 'lang.inc'
include 'macros.inc'

START:                                      ; start of execution
    mov     eax,40                          ; Report events
    mov     ebx,10000111b                   ; Stack 8 + defaults
    int     0x40

    mov     dword [prompt], p1
    mov     dword [promptlen], p1len - p1   ; 'waiting for command'

    call    draw_window                     ; at first, draw the window

still:
    mov     eax,10                          ; wait here for event
    int     0x40

    cmp     eax,1                           ; redraw request ?
    jz      red
    cmp     eax,2                           ; key in buffer ?
    jz      key
    cmp     eax,3                           ; button in buffer ?
    jz      button

    jmp     still

red:                            ; redraw
    call    draw_window
    jmp     still

key:                            ; Keys are not valid at this part of the
    mov     eax,2               ; loop. Just read it and ignore
    int     0x40
    jmp     still

button:                         ; button
    mov     eax,17              ; get id
    int     0x40

    cmp     ah,1                ; button id=1 ?
    jnz     noclose

    ; close socket before exiting
    mov     eax, 53
    mov     ebx, 1
    mov     ecx, [socketNum]
    int     0x40

    mov     eax,0xffffffff      ; close this program
    int     0x40

noclose:
    cmp     ah,3                ; Resolve address?
    jnz     noresolve

    mov     dword [prompt], p5
    mov     dword [promptlen], p5len - p5   ; display 'Resolving'
    call    draw_window

    call    translateData       ; Convert domain & DNS IP address

    call    resolveDomain

    jmp     still


noresolve:
    cmp     ah,4
    jz      f1                  ; Enter domain name
    cmp     ah,5
    jz      f2                  ; enter DNS Server IP
    jmp     still


f1:
    mov     [addr],dword query
    mov     [ya],dword 35
    jmp     rk

f2:
    mov     [addr],dword dnsServer
    mov     [ya],dword 35+16

rk:
    mov     ecx,26
    mov     edi,[addr]
    mov     al,' '
    rep     stosb

    call    print_text

    mov     edi,[addr]

f11:
    mov     eax,10
    int     0x40
    cmp     eax,2
    jz      fbu
    jmp     still

fbu:
    mov     eax,2
    int     0x40  ; get key
    shr     eax,8
    cmp     eax,8
    jnz     nobs
    cmp     edi,[addr]
    jz      f11
    sub     edi,1
    mov     [edi],byte ' '
    call    print_text
    jmp     f11

nobs:
    cmp     eax,dword 31
    jbe     f11
    cmp     eax,dword 95
    jb      keyok
    sub     eax,32

keyok:
    mov     [edi],al

    call    print_text

    add     edi,1
    mov     esi,[addr]
    add     esi,26
    cmp     esi,edi
    jnz     f11

    jmp  still



print_text:
    mov     eax,13
    mov     ebx,103*65536+26*6
    mov     ecx,[ya]
    shl     ecx,16
    mov     cx,8
    mov     edx,0x224466
    int     0x40

    mov     eax,4
    mov     ebx,103*65536
    add     ebx,[ya]
    mov     ecx,0xffffff
    mov     edx,[addr]
    mov     esi,26
    int     0x40

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
;      The DNS ip, in dnsServer, is converted and stored in dnsIP
;
;***************************************************************************
translateData:

    ; first, get the IP address of the DNS server
    ; Then, build up the request string.

    xor     eax, eax
    mov     dh, 10
    mov     dl, al
    mov     [dnsIP], eax

    mov     esi, dnsServer
    mov     edi, dnsIP

    mov     ecx, 4

td003:
    lodsb
    sub     al, '0'
    add     dl, al
    lodsb
    cmp     al, '.'
    je      ipNext
    cmp     al, ' '
    je      ipNext
    mov     dh, al
    sub     dh, '0'
    mov     al, 10
    mul     dl
    add     al, dh
    mov     dl, al
    lodsb
    cmp     al, '.'
    je      ipNext
    cmp     al, ' '
    je      ipNext
    mov     dh, al
    sub     dh, '0'
    mov     al, 10
    mul     dl
    add     al, dh
    mov     dl, al
    lodsb

ipNext:
    mov     [edi], dl
    inc     edi
    mov     dl, 0
    loop    td003

    ; Build the request string


    mov     eax, 0x00010100
    mov     [dnsMsg], eax
    mov     eax, 0x00000100
    mov     [dnsMsg+4], eax
    mov     eax, 0x00000000
    mov     [dnsMsg+8], eax

    ; domain name goes in at dnsMsg+12
    mov     esi, dnsMsg + 12        ; location of label length
    mov     edi, dnsMsg + 13        ; label start
    mov     edx, query
    mov     ecx, 12                  ; total string length so far

td002:
    mov     [esi], byte 0
    inc     ecx

td0021:
    mov     al, [edx]
    cmp     al, ' '
    je      td001                   ; we have finished the string translation
    cmp     al, '.'                 ; we have finished the label
    je      td004

    inc     byte [esi]
    inc     ecx
    mov     [edi], al
    inc     edi
    inc     edx
    jmp     td0021

td004:
    mov     esi, edi
    inc     edi
    inc     edx
    jmp     td002



    ; write label len + label text

td001:
    mov     [edi], byte 0
    inc     ecx
    inc     edi
    mov     [edi], dword 0x01000100
    add     ecx, 4

    mov     [dnsMsgLen], ecx

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
 mov     ecx, 1000  ; local port starting at 1000
getlp:
 inc     ecx
 push ecx
 mov     eax, 53
 mov     ebx, 9
 int     0x40
 pop     ecx
 cmp     eax, 0   ; is this local port in use?
 jz  getlp      ; yes - so try next

    ; First, open socket
    mov     eax, 53
    mov     ebx, 0
    mov     edx, 53    ; remote port - dns
    mov     esi, [dnsIP]
    int     0x40

    mov     [socketNum], eax

    ; write to socket ( request DNS lookup )
    mov     eax, 53
    mov     ebx, 4
    mov     ecx, [socketNum]
    mov     edx, [dnsMsgLen]
    mov     esi, dnsMsg
    int     0x40

    ; Setup the DNS response buffer

    mov     eax, dnsMsg
    mov     [dnsMsgLen], eax

    ; now, we wait for
    ; UI redraw
    ; UI close
    ; or data from remote

ctr001:
    mov     eax,10                 ; wait here for event
    int     0x40

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
    int     0x40

    cmp     eax, 0
    je      ctr001

    ; we have data - this will be the response
ctr002:
    mov     eax, 53
    mov     ebx, 3
    mov     ecx, [socketNum]
    int     0x40                ; read byte - block (high byte)

    ; Store the data in the response buffer
    mov     eax, [dnsMsgLen]
    mov     [eax], bl
    inc     dword [dnsMsgLen]


if DEBUGGING_STATE = DEBUGGING_ENABLED
    call debug_print_rx_ip
end if

    mov     eax, 53
    mov     ebx, 2
    mov     ecx, [socketNum]
    int     0x40                ; any more data?

    cmp     eax, 0
    jne     ctr002              ; yes, so get it

    ; close socket
    mov     eax, 53
    mov     ebx, 1
    mov     ecx, [socketNum]
    int     0x40

    mov     [socketNum], dword 0xFFFF

    ; Now parse the message to get the host IP
    ; Man, this is complicated. It's described in
    ; RFC 1035

    ; 1) Validate that we have an answer with > 0 responses
    ; 2) Find the answer record with TYPE 0001 ( host IP )
    ; 3) Finally, copy the IP address to the display
    ; Note: The response is in dnsMsg
    ;       The end of the buffer is pointed to by [dnsMsgLen]

    ; Clear the IP address text
    mov     [hostIP], dword 0

    mov     esi, dnsMsg

    ; Is this a response to my question?
    mov     al, [esi+2]
    and     al, 0x80
    cmp     al, 0x80
    jne     ctr002a

    ; Were there any errors?
    mov     al, [esi+3]
    and     al, 0x0F
    cmp     al, 0x00
    jne     ctr002a

    ; Is there ( at least 1 ) answer?
    mov     ax, [esi+6]
    cmp     ax, 0x00
    je      ctr002a

    ; Header validated. Scan through and get my answer

    add     esi, 12             ; Skip to the question field

    ; Skip through the question field
    call    skipName
    add     esi, 4              ; skip past the questions qtype, qclass

ctr002z:
    ; Now at the answer. There may be several answers,
    ; find the right one ( TYPE = 0x0001 )
    call    skipName
    mov     ax, [esi]
    cmp     ax, 0x0100          ; Is this the IP address answer?
    jne     ctr002c

    ; Yes! Point esi to the first byte of the IP address
    add     esi, 10

    mov     eax, [esi]
    mov     [hostIP], eax
    jmp     ctr002a             ; And exit...


ctr002c:                        ; Skip through the answer, move to the next
    add     esi, 8
    movzx   eax, byte [esi+1]
    mov     ah, [esi]
    add     esi, eax
    add     esi, 2

    ; Have we reached the end of the msg?
    ; This is an error condition, should not happen
    cmp     esi, [dnsMsgLen]
    jl      ctr002z             ; Check next answer
    jmp     ctr002a             ; abort


ctr002a:
    mov     dword [prompt], p4  ; Display IP address
    mov     dword [promptlen], p4len - p4
    call    draw_window

    jmp     ctr001

ctr003:                         ; redraw
    call    draw_window
    jmp     ctr001

ctr004:                         ; key
    mov     eax,2               ; just read it and ignore
    int     0x40
    jmp     ctr001

ctr005:                         ; button
    mov     eax,17              ; get id
    int     0x40

    ; close socket
    mov     eax, 53
    mov     ebx, 1
    mov     ecx, [socketNum]
    int     0x40

    mov     [socketNum], dword 0xFFFF
    mov     [hostIP], dword 0

    mov     dword [prompt], p1
    mov     dword [promptlen], p1len - p1   ; 'waiting for command'

    call    draw_window                     ; at first, draw the window

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
    mov     al, [esi]
    cmp     al, 0
    je      sn_exit
    and     al, 0xc0
    cmp     al, 0xc0
    je      sn001

    movzx   eax, byte [esi]
    inc     eax
    add     esi, eax
    jmp     skipName

sn001:
    add     esi, 2                          ; A pointer is always at the end
    ret

sn_exit:
    inc     esi
    ret


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:
    mov     eax,12                    ; function 12:tell os about windowdraw
    mov     ebx,1                     ; 1, start of draw
    int     0x40
                                      ; DRAW WINDOW
    mov     eax,0                     ; function 0 : define and draw window
    mov     ebx,100*65536+300         ; [x start] *65536 + [x size]
    mov     ecx,100*65536+140         ; [y start] *65536 + [y size]
    mov     edx,0x03224466            ; color of work area RRGGBB
    mov     esi,0x00334455            ; color of grab bar  RRGGBB,8->color gl
    mov     edi,0x00ddeeff            ; color of frames    RRGGBB
    int     0x40
                                      ; WINDOW LABEL
    mov     eax,4                     ; function 4 : write text to window
    mov     ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov     ecx,0x00ffffff            ; color of text RRGGBB
    mov     edx,labelt                ; pointer to text beginning
    mov     esi,labellen-labelt       ; text length
    int     0x40

    mov     eax,8                     ; Resolve
    mov     ebx,20*65536+190
    mov     ecx,79*65536+15
    mov     edx,3
    mov     esi,0x557799
    int     0x40

    mov     eax,8
    mov     ebx,270*65536+10
    mov     ecx,34*65536+10
    mov     edx,4
    mov     esi,0x557799
    int     0x40

    mov     eax,8
    mov     ebx,270*65536+10
    mov     ecx,50*65536+10
    mov     edx,5
    mov     esi,0x557799
    int     0x40

    ; Copy the file name to the screen buffer
    ; file name is same length as IP address, to
    ; make the math easier later.
    cld
    mov     esi,query
    mov     edi,text+13
    mov     ecx,26
    rep     movsb


    ; copy the IP address to the screen buffer
    mov     esi,dnsServer
    mov     edi,text+40+13
    mov     ecx,26
    rep     movsb

    ; copy the prompt to the screen buffer
    mov     esi,[prompt]
    mov     edi,text+200
    mov     ecx,[promptlen]
    rep     movsb

    ; Re-draw the screen text
    cld
    mov     ebx,25*65536+35           ; draw info text with function 4
    mov     ecx,0xffffff
    mov     edx,text
    mov     esi,40

newline:
    mov     eax,4
    int     0x40
    add     ebx,16
    add     edx,40
    cmp     [edx],byte 'x'
    jnz     newline


    ; Write the host IP, if we have one
    mov     eax, [hostIP]
    cmp     eax, 0
    je      dw001

    ; We have an IP address... display it
    mov     edi,hostIP
    mov     edx,97*65536+115
    mov     esi,0x00ffffff
    mov     ebx,3*65536

ipdisplay:
    mov     eax,47
    movzx   ecx,byte [edi]
    int     0x40
    add     edx,6*4*65536
    inc     edi
    cmp     edi,hostIP+4
    jb      ipdisplay

dw001:
    mov     eax,12                    ; function 12:tell os about windowdraw
    mov     ebx,2                     ; 2, end of draw
    int     0x40

    ret


if DEBUGGING_STATE = DEBUGGING_ENABLED
;****************************************************************************
;    Function
;       debug_print_string
;
;   Description
;       prints a string to the debug board
;
;       esi holds ptr to msg to display
;
;       Nothing preserved; I'm assuming a pusha/popa is done before calling
;
;****************************************************************************
debug_print_string:
    mov     cl, [esi]
    cmp     cl, 0
    jnz     dps_001
    ret

dps_001:
    mov     eax,63
    mov     ebx, 1
    push    esi
    int 0x40

    inc   word [ind]
    mov  ax, [ind]
    and ax, 0x1f
    cmp  ax, 0
    jne  ds1

    mov   cl, 13
    mov     eax,63
    mov     ebx, 1
    int 0x40
    mov   cl, 10
    mov     eax,63
    mov     ebx, 1
    int 0x40


ds1:
    pop     esi
    inc     esi
    jmp     debug_print_string


ind: dw 0
; This is used for translating hex to ASCII for display or output
hexchars db '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
IP_STR              db  'xx',0


debug_print_rx_ip:
    pusha
    mov     edi, IP_STR

    xor     eax, eax
    mov     al, bl
    shr     al, 4
    mov     ah, [eax + hexchars]
    mov     [edi], ah
    inc     edi

    xor     eax, eax
    mov     al, bl
    and     al, 0x0f
    mov     ah, [eax + hexchars]
    mov     [edi], ah
    mov     esi, IP_STR

    call    debug_print_string
    popa
    ret
end if


; DATA AREA

addr            dd  0x0
ya              dd  0x0

text:
    db 'Host name  : xxxxxxxxxxxxxxx            '
    db 'DNS server : xxx.xxx.xxx.xxx            '
    db '                                        '
    db '     RESOLVE ADDRESS                    '
    db '                                        '
    db '                                        '
    db 'x <- END MARKER, DONT DELETE            '


labelt:
    db   'DNS Client'
labellen:


prompt: dd 0
promptlen: dd 0


p1:             db 'Waiting for Command        '
p1len:

p4:             db 'IP Address:    .   .   .   '
p4len:

p5:             db 'Resolving...               '
p5len:


dnsServer       db  '194.145.128.1              ' ; iolfree.ie DNS
query           db  'WWW.MENUETOS.ORG           '

hostIP:         dd 0
dnsIP:          dd 0
dnsMsgLen:      dd 0
socketNum:      dd 0xFFFF
dnsMsg:
I_END:
