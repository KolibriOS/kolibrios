;
;    HTTPC.ASM
;
;    Compile with FASM for Menuet ( v1.40 for DOS )
;
;    This program implements a very simple web browser
;
;    Version 0.4    2nd December 2003 Mike Hibbett
;    Enabled lowercase/uppcase characters in URL
;    Version 0.3    30th November 2003 Mike Hibbett
;    Fixed bug with tcp socket opne - uses unique port
;    Version 0.2    27th November 2003 Mike Hibbett
;    Added user entry of url, and implements url -> IP address
;    resolution through DNS
;
;    Version 0.1  Ville Mikael Turjanmaa
;    Original version


; Enabling debugging puts stuff to the debug board
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
;include "DEBUG.INC"

URLMAXLEN       equ     50  ; maximum length of url string

; Memory usage
; webpage source file at 0x10000
; decoded text page at   0x20000
; text attribute         0x30000  (1 = normal, 2 = bold, 128+ = link)

START:                              ; start of execution

;dps <"Program started",13,10>

    mov     eax,40                          ; Report events
    mov     ebx,10000111b                   ; Stack 8 + defaults
    int     0x40

    call    draw_window

still:
    mov     eax,23                 ; wait here for event
    mov     ebx,1
    int     0x40

    cmp     eax,1                  ; redraw request ?
    je      red
    cmp     eax,2                  ; key in buffer ?
    je      key
    cmp     eax,3                  ; button in buffer ?
    je      button

    ; Get the web page data from the remote server
    call    read_incoming_data

    mov     eax,[status]
    mov     [prev_status],eax

    mov     eax,53
    mov     ebx,6
    mov     ecx,[socket]
    int     0x40

    mov     [status],eax

    cmp     [prev_status],4
    jge     no_send
    cmp     [status],4
    jne     no_send

    mov     [onoff],1

    call    send_request

no_send:
    call    print_status

    cmp     [prev_status],4
    jne     no_close
    cmp     [status],4   ; connection closed by server
    jbe     no_close     ; respond to connection close command
                         ; draw page

    call    read_incoming_data

    mov     eax,53
    mov     ebx,8
    mov     ecx,[socket]
    int     0x40

    call    draw_page

    mov     [onoff],0

no_close:
    jmp     still

red:                    ; redraw
    call    draw_window
    jmp     still

key:                    ; key
    mov     eax,2       ; just read it and ignore
    int     0x40
    shr     eax,8
    cmp     eax,184
    jne     no_down
    cmp     [display_from],25
    jb      no_down
    sub     [display_from],25
    call    display_page

no_down:
    cmp     eax,183
    jne     no_up
    add     [display_from],25
    call    display_page

no_up:
    jmp     still

button:                 ; button
;dps <"Button pressed",13,10>
    mov     eax,17      ; get id
    int     0x40
    cmp     ah,1                   ; button id=1 ?
    jne     noclose

;dps "Closing socket before exit... "

    mov     eax, 53
    mov     ebx, 8
    mov     ecx, [socket]
    int     0x40

;dpd eax
;dps <13,10>

exit:
    or      eax,-1                 ; close this program
    int     0x40

noclose:
    cmp     ah,31
    jne     noup
    sub     [display_from],20
    call    display_page
    jmp     still

noup:
    cmp     ah,32
    jne     nodown
    add     [display_from],20
    call    display_page
    jmp     still

nodown:
    cmp     ah, 10              ; Enter url
    jne     nourl

    mov     [addr],dword document_user
    mov     [ya],dword 38
    mov     [len],dword URLMAXLEN

    mov     ecx,[len]
    mov     edi,[addr]
    mov     al,' '
    rep     stosb

    call    print_text

    mov     edi,[addr]

f11:
    mov     eax,10
    int     0x40
    cmp     eax,2 ; key?
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
    cmp     eax, dword 10
    je      retkey
    cmp     eax, dword 13
    je      retkey

    cmp     eax,dword 31
    jbe     f11

; Removed in v0.4
;    cmp     eax,dword 95
;    jb      keyok
;    sub     eax,32

keyok:
    mov     [edi],al

    call    print_text

    add     edi,1
    mov     esi,[addr]
    add     esi,URLMAXLEN
    cmp     esi,edi
    jnz     f11

    jmp  still

retkey:
    mov     ah, 22   ; start load

nourl:
    call    socket_commands     ; opens or closes the connection
    jmp     still


;****************************************************************************
;    Function
;       send_request
;
;   Description
;       Transmits the GET request to the server.
;       This is done as GET then URL then HTTP/1.0',13,10,13,10 in 3 packets
;
;****************************************************************************
send_request:
    pusha
    mov     eax,53     ; 'GET '
    mov     ebx,7
    mov     ecx,[socket]
    mov     edx,4
    mov     esi,string0
    int     0x40

    mov     edx,0

next_edx:
    ; Determine the length of the url to send in the GET request
    inc     edx
    cmp     [edx+document],byte ' '
    jne     next_edx

    mov     eax,53     ; document_user
    mov     ebx,7
    mov     ecx,[socket]
    mov     esi,document
    int     0x40

    mov     eax,53     ; ' HTTP/1.0 .. '
    mov     ebx,7
    mov     ecx,[socket]
    mov     edx,stringh_end-stringh
    mov     esi,stringh
    int     0x40

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

    mov     eax,26
    mov     ebx,9
    int     0x40

    cmp     eax,[nextupdate]
    jb      status_return

    add     eax,25

    mov     [nextupdate],eax

    mov     eax,13
    mov     ebx,5*65536+100
    mov     ecx,[winys]
    shl     ecx,16
    add     ecx,-18*65536+10
    mov     edx,0xffffff
    int     0x40

    mov     eax,47
    mov     ebx,3*65536
    mov     ecx,[status]
    mov     edx,12*65536-18
    add     edx,[winys]
    mov     esi,0x000000
    int     0x40

    mov     eax,47
    mov     ebx,6*65536
    mov     ecx,[pos]
    mov     edx,40*65536-18
    add     edx,[winys]
    mov     esi,0x000000
    int     0x40

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
    cmp     [onoff],1
    je      rid
    ret

rid:
    mov     ecx,-1

newbyteread:
    mov     eax, 53
    mov     ebx, 2
    mov     ecx, [socket]
    int     0x40

    cmp     eax,0
    je      no_more_data

read_more:
    mov     eax, 53
    mov     ebx, 3
    mov     ecx, [socket]
    int     0x40

yesm:
    inc     [pos]
    mov     ecx,[pos]
    mov     [0x10000+ecx],bl

    call    print_status

    cmp     eax,0
    jne     read_more

    mov     eax,5
    mov     ebx,50
    int     0x40

    jmp     newbyteread

no_more_data:
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
    pusha
    mov     esi,0
    mov     [command_on_off],0

newlettercheck:
    movzx   eax,byte [esi+0x10000]
    cmp     al,'<'
    jne     no_c_on
    mov     [command_on_off],1

no_c_on:
    cmp     al,'>'
    jne     no_c_off
    mov     [command_on_off],0

no_c_off:
    cmp     [command_on_off],0
    je      no_lower_case

    cmp     eax,96
    jg      no_lower_case
    cmp     eax,65
    jb      no_lower_case
    add     eax,32

no_lower_case:
    mov     [esi+0x10000],al
    inc     esi
    cmp     esi,[pos]
    jbe     newlettercheck
    mov     edi,0x30000
    mov     ecx,0x10000
    mov     al,0
    cld
    rep     stosb
    mov     [text_type],1
    mov     [command_on_off],0

    mov     esi,0
    mov     ecx,[pos]

    ; search for double lf

find_dlf:
    cmp     [0x10000+esi-4],dword 0x0d0a0d0a
    je      found_dlf
    cmp     [0x10000+esi-4],dword 0x0a0d0a0d
    je      found_dlf
    cmp     [0x10000+esi-2],word 0x0d0d
    je      found_dlf
    cmp     [0x10000+esi-2],word 0x0a0a
    je      found_dlf

    cmp     esi,5500
    je      found_dlf

    inc     esi

    jmp     find_dlf

found_dlf:
newbyte:
    mov     al,[esi+0x10000]
    cmp     al,'<'
    jne     no_command_on
    mov     [command_on_off],1

no_command_on:
    cmp     al,'>'
    jne     no_command_off
    mov     [command_on_off],0
    jmp     byte_done

no_command_off:
    mov     eax,[esi+0x10000]    ; <!--
    cmp     eax,'<!--'
    jne     no_com2_start
    mov     [com2],1

no_com2_start:
    mov     ax,[esi+0x10000]    ; -->
    cmp     ax,'->'
    jne     no_com2_end
    mov     [com2],0
    inc     esi
    jmp     byte_done

no_com2_end:
    mov     eax,[esi+0x10000]    ; <script
    cmp     eax,'<scr'
    jne     no_script_start
    mov     [script],1

no_script_start:
    mov     eax,[esi+0x10000]    ; /script>
    cmp     eax,'</sc'
    jne     no_script_end
    mov     [script],0
    inc     esi
    jmp     byte_done

no_script_end:
    cmp     [command_on_off],0
    jne     no_print

    cmp     [com2],0
    jne     no_print

    cmp     [script],0
    jne     no_print

    mov     al,[esi+0x10000] ; &
    cmp     al,'&'
    jne     no_nbsp

newsps:
    inc     esi
    mov     al,[esi+0x10000] ;
    cmp     al,';'
    jne     newsps
    jmp     byte_done

no_nbsp:
    cmp     al,13
    jne     no_lb
    jmp     byte_done

no_lb:
    cmp     al,10
    jne     no_lf
    jmp     byte_done

no_lf:
    mov     ebx,[pagey]
    imul    ebx,[pagexs]
    add     ebx,[pagex]
    add     ebx,0x20000
    and     eax,0xff
    cmp     eax,31
    jbe     byte_done
    cmp     [lastletter],al
    jne     letter_ok
    cmp     al,' '
    je      byte_done

letter_ok:
    mov     [ebx],al
    mov     dl,[text_type]
    mov     [ebx+0x10000],dl
    mov     [pageyinc],0
    mov     [lastletter],al

    inc     [pagex]

    mov     ebx,[pagex]
    cmp     ebx,[pagexs]
    jb      byte_done
    mov     [pagex],0
    inc     [pagey]

    jmp     byte_done

no_print:
    ; HTML -COMMAND

    mov     ax,[esi+0x10000]    ; b> bold
    cmp     ax,'b>'
    jne     no_bold_start
    mov     [text_type],2

no_bold_start:
    mov     eax,[esi+0x10000]    ; /b bold end
    cmp     eax,'</b>'
    jne     no_bold_end
    mov     [text_type],1
    add     esi,2

no_bold_end:
    mov     ax,[esi+0x10000]    ; <a
    cmp     ax,'a '
    jne     no_link_start
    mov     [text_type],128
    add     esi,2

no_link_start:
    mov     ax,[esi+0x10000]    ; /a
    cmp     ax,'/a'
    jne     no_link_end2
    mov     [text_type],1
    add     esi,0

no_link_end2:
    mov     ax,[esi+0x10000]
    cmp     ax,'br'
    jne     no_br
    call    linefeed
    inc     esi

no_br:
    mov     ax,[esi+0x10000]
    cmp     ax,'td'
    jne     no_td
    call    linefeed
    inc     esi

no_td:
    mov     eax,[esi+0x10000]
    cmp     eax,'tabl'
    jne     no_table
    call    linefeed
    add     esi,3

no_table:
byte_done:
    inc     esi
    cmp     esi,[pos]
    jbe     newbyte

    mov     [display_from],0
    call    display_page

    popa
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
    cmp     [pageyinc],2
    jge     nolf

    mov     [pagex],0
    inc     [pagey]
    inc     [pageyinc]

nolf:
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
    pusha

    mov     eax,0
    mov     ebx,0

newpxy:
    push    eax
    push    ebx

    mov     eax,13       ; background for letter
    mov     ebx,[esp+4]
    imul    ebx,6
    add     ebx,[dpx]
    shl     ebx,16
    add     ebx,6
    mov     ecx,[esp+0]
    imul    ecx,10
    add     ecx,[dpy]
    shl     ecx,16
    add     ecx,10
    mov     edx,0xffffff
    int     0x40

    mov     eax,4
    mov     ebx,[esp+4]
    imul    ebx,6
    add     ebx,[dpx]
    shl     ebx,16

    mov     bx,[esp+0]
    imul    bx,10
    add     bx,word [dpy]

    mov     esi,[esp]
    imul    esi,[pagexs]
    add     esi,[esp+4]

    mov     edx,[display_from]
    imul    edx,[pagexs]
    add     edx,0x20000
    add     edx,esi

    movzx   ecx,byte [edx+0x10000]
    cmp     ecx,1
    jne     noecx1
    mov     ecx,0x000000

noecx1:
    movzx   ecx,byte [edx+0x10000]
    cmp     ecx,2
    jne     noecx2
    mov     ecx,0xff0000

noecx2:
    cmp     ecx,128
    jne     noecx128
    mov     ecx,0x0000ff

noecx128:
    mov     esi,1

    int     0x40

    pop     ebx
    pop     eax

    inc     eax
    cmp     eax,[pagexs]
    jb      newpxy
    mov     eax,0
    inc     ebx
    cmp     ebx,30
    jb      newpxy

    popa
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
    cmp     ah,22       ; open socket
    jnz     tst3

    ; Clear all page memory
    mov     edi,0x10000
    mov     ecx,0x30000
    mov     al,0
    cld
    rep     stosb

    ; Parse the entered url
    call    parse_url

    ; Get a free port number
        mov         ecx, 1000           ; local port starting at 1000
getlp1:
        inc         ecx
        push    ecx
        mov         eax, 53
        mov         ebx, 9
        int     0x40
        pop         ecx
        cmp         eax, 0                      ; is this local port in use?
        jz              getlp1              ; yes - so try next

    mov     eax,53
    mov     ebx,5
    mov     edx,80
    mov     esi,dword [server_ip]
    mov     edi,1
    int     0x40
    mov     [socket], eax

    mov     [pos],0
    mov     [pagex],0
    mov     [pagey],0
    mov     [pagexs],80
    mov     [command_on_off],0

    ret

tst3:
    cmp     ah,24     ; close socket
    jnz     no_24

    mov     eax,53
    mov     ebx,8
    mov     ecx,[socket]
    int     0x40

    call    draw_page

    ret

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
    mov     al, ' '
    mov     edi, document
    mov     ecx,URLMAXLEN
    rep     stosb
    mov     edi, webAddr
    mov     ecx,URLMAXLEN
    rep     stosb
    mov     al, '/'
    mov     [document], al

    mov     esi, document_user
    ; remove any leading protocol text
    mov     ecx, URLMAXLEN
    mov     ax, '//'

pu_000:
    cmp     [esi], byte ' '     ; end of text?
    je      pu_002              ; yep, so not found
    cmp     word [esi], ax
    je      pu_001              ; Found it, so esi+2 is start
    inc     esi
    loop    pu_000

pu_002:
    ; not found, so reset esi to start
    mov     esi, document_user -2

pu_001:
    add     esi, 2

    mov     ebx, esi    ; save address of start of web address
    mov     edi, document_user + URLMAXLEN  ; end of string

    ; look for page delimiter - it's a '/' character
pu_003:
    cmp     [esi], byte ' '     ; end of text?
    je      pu_004              ; yep, so none found
    cmp     esi, edi            ; end of string?
    je      pu_004              ; yep, so none found
    cmp     [esi], byte '/'     ; delimiter?
    je      pu_005              ; yep - process it
    inc     esi
    jmp     pu_003

pu_005:
    ; copy page to document address
    ; esi = delimiter
    push    esi
    inc     esi
    mov     ecx, edi            ; end of document_user
    mov     edi, document
    cld

pu_006:
    movsb
    cmp     esi, ecx
    je      pu_007              ; end of string?
    cmp     [esi], byte ' '     ; end of text
    je      pu_007
    jmp     pu_006

pu_007:
    pop     esi                 ; point esi to '/' delimiter

pu_004:
    ; copy web address to webAddr
    ; start in ebx, end in esi-1
    mov     ecx, esi
    mov     esi, ebx
    mov     edi, webAddr
    cld

pu_008:
    movsb
    cmp     esi, ecx
    je      pu_009
    jmp     pu_008

pu_009:
    ; For debugging, display resulting strings

if DEBUGGING_STATE = DEBUGGING_ENABLED
    mov     esi, document_user
    call    debug_print_string
    mov     esi, webAddr
    call    debug_print_string
    mov     esi, document
    call    debug_print_string
end if

    ; Look up the ip address, or was it specified?
    mov     al, [webAddr]
    cmp     al, '0'
    jb      pu_010              ; Resolve address
    cmp     al, '9'
    ja      pu_010              ; Resolve address


if DEBUGGING_STATE = DEBUGGING_ENABLED
    mov     esi, str2       ; print gotip
    call    debug_print_string
end if

    ; Convert address
    mov     esi,webAddr-1
    mov     edi,server_ip
    xor     eax,eax
ip1:
    inc     esi
    cmp     [esi],byte '0'
    jb      ip2
    cmp     [esi],byte '9'
    jg      ip2
    imul    eax,10
    movzx   ebx,byte [esi]
    sub     ebx,48
    add     eax,ebx
    jmp     ip1
ip2:
    mov     [edi],al
    xor     eax,eax
    inc     edi
    cmp     edi,server_ip+3
    jbe     ip1

    jmp     pu_011

pu_010:

if DEBUGGING_STATE = DEBUGGING_ENABLED
    mov     esi, str1       ; print resolving
    call    debug_print_string
end if

    ; Resolve Address
    call    translateData       ; Convert domain & DNS IP address
    call    resolveDomain       ; get ip address

if DEBUGGING_STATE = DEBUGGING_ENABLED
    mov     esi, str3
    call    debug_print_string
end if

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


    mov     eax, 0x00010100
    mov     [dnsMsg], eax
    mov     eax, 0x00000100
    mov     [dnsMsg+4], eax
    mov     eax, 0x00000000
    mov     [dnsMsg+8], eax

    ; domain name goes in at dnsMsg+12
    mov     esi, dnsMsg + 12        ; location of label length
    mov     edi, dnsMsg + 13        ; label start
    mov     edx, webAddr
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
        mov         ecx, 1000           ; local port starting at 1000
getlp:
        inc         ecx
        push    ecx
        mov         eax, 53
        mov         ebx, 9
        int     0x40
        pop         ecx
        cmp         eax, 0                      ; is this local port in use?
        jz              getlp               ; yes - so try next

    ; First, open socket
    mov     eax, 53
    mov     ebx, 0
    mov     edx, 53    ; remote port - dns
    mov     esi, dword [dns_ip]
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

if DEBUGGING_STATE = DEBUGGING_ENABLED
    mov     esi, str4
    call    debug_print_string
end if

    ; 1) Validate that we have an answer with > 0 responses
    ; 2) Find the answer record with TYPE 0001 ( host IP )
    ; 3) Finally, copy the IP address to the display
    ; Note: The response is in dnsMsg
    ;       The end of the buffer is pointed to by [dnsMsgLen]

    ; Clear the IP address text
    mov     [server_ip], dword 0

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

if DEBUGGING_STATE = DEBUGGING_ENABLED
    pusha
    mov     esi, str4
    call    debug_print_string
    popa
end if

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
    mov     [server_ip], eax
    ret


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

    mov     dl, ah

    ; close socket
    mov     eax, 53
    mov     ebx, 1
    mov     ecx, [socketNum]
    int     0x40

    cmp     dl, 1
    je      exit

    mov     [socketNum], dword 0xFFFF
    mov     [server_ip], dword 0

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



if DEBUGGING_STATE = DEBUGGING_ENABLED

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
    push    esi
    mov     cl, '"'
    mov     eax,63
    mov     ebx, 1
    int     0x40
    pop     esi

dps_000:
    mov     cl, [esi]
    cmp     cl, 0
    je      dps_exit
    cmp     cl, ' '
    je      dps_exit
    jmp     dps_001

dps_exit:
    mov     cl, '"'
    mov     eax,63
    mov     ebx, 1
    int     0x40
    mov     cl, 13
    mov     eax,63
    mov     ebx, 1
    int     0x40
    mov     cl, 10
    mov     eax,63
    mov     ebx, 1
    int     0x40
    ret

dps_001:
    mov     eax,63
    mov     ebx, 1
    push    esi
    int     0x40

    pop     esi
    inc     esi
    jmp     dps_000
end if


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
    mov     eax,13
    mov     ebx,30*65536+URLMAXLEN*6  ; 50 should really be [len], and 103 [xa]
    mov     ecx,[ya]
    shl     ecx,16
    mov     cx,9
    mov     edx,0xFFFFFF
    int     0x40

    ; write text
    mov     eax,4
    mov     ebx,30*65536
    add     ebx,[ya]
    mov     ecx,0x000000
    mov     edx,[addr]
    mov     esi,URLMAXLEN
    int     0x40

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
    mov     ebx,50*65536+550          ; [x start] *65536 + [x size]
    mov     ecx,50*65536+400          ; [y start] *65536 + [y size]
    mov     edx,0x03ffffff            ; color of work area RRGGBB,8->color gl
    mov     esi,0x805080d0            ; color of grab bar  RRGGBB,8->color gl
    mov     edi,0x005080d0            ; color of frames    RRGGBB
    int     0x40

                                   ; WINDOW LABEL
    mov     eax,4                     ; function 4 : write text to window
    mov     ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov     ecx,0x00ddeeff            ; color of text RRGGBB
    mov     edx,labelt                ; pointer to text beginning
    mov     esi,labellen-labelt       ; text length
    int     0x40


    mov     esi, URLMAXLEN            ; URL
    mov     eax,4                     ; function 4 : write text to window
    mov     ebx,30*65536+38           ; [x start] *65536 + [y start]
    mov     ecx,0x000000              ; color of text RRGGBB
    mov     edx,document_user         ; pointer to text beginning
    int     0x40

    mov     eax,38
    mov     ebx,5*65536+545
    mov     ecx,60*65536+60
    mov     edx,0x000000
    int     0x40

    mov     eax,38
    mov     ebx,5*65536+545
    mov     ecx,[winys]
    shl     ecx,16
    add     ecx,[winys]
    sub     ecx,26*65536+26
    mov     edx,0x000000
    int     0x40
                                   ; RELOAD
    mov     eax,8                     ; function 8 : define and draw button
    mov     ebx,388*65536+50          ; [x start] *65536 + [x size]
    mov     ecx,34*65536+14           ; [y start] *65536 + [y size]
    mov     edx,22                    ; button id
    mov     esi,0x5588dd              ; button color RRGGBB
    int     0x40

                                   ; URL
    mov     eax,8                     ; function 8 : define and draw button
    mov     ebx,10*65536+12          ; [x start] *65536 + [x size]
    mov     ecx,34*65536+12           ; [y start] *65536 + [y size]
    mov     edx,10                    ; button id
    mov     esi,0x5588dd              ; button color RRGGBB
    int     0x40

                                   ; STOP
    mov     eax,8                     ; function 8 : define and draw button
    mov     ebx,443*65536+50          ; [x start] *65536 + [x size]
    mov     ecx,34*65536+14           ; [y start] *65536 + [y size]
    mov     edx,24                    ; button id
    mov     esi,0x5588dd              ; button color RRGGBB
    int     0x40

                                   ; BUTTON TEXT
    mov     eax,4                     ; function 4 : write text to window
    mov     ebx,390*65536+38          ; [x start] *65536 + [y start]
    mov     ecx,0xffffff              ; color of text RRGGBB
    mov     edx,button_text           ; pointer to text beginning
    mov     esi,20                    ; text length
    int     0x40

    call    display_page

    mov     eax,12                    ; function 12:tell os about windowdraw
    mov     ebx,2                     ; 2, end of draw
    int     0x40

    ret


if DEBUGGING_STATE = DEBUGGING_ENABLED
str1:       db  "Resolving...",0
str3:       db  "Resolved",0
str2:       db  "GotIP",0
str4:       db  "GotResponse",0
end if

button_text     db      ' RELOAD    STOP       '
dpx             dd      25  ; x - start of html page in pixels in window
dpy             dd      65  ; for y
lastletter      db      0
pageyinc        dd      0
display_from    dd      20
pos             dd      0x0
pagex           dd      0x0
pagey           dd      0x0
pagexs          dd      80
command_on_off  dd      0x0
text_type       db      1
com2            dd      0x0
script          dd      0x0
socket          dd      0x0

addr            dd  0x0
ya              dd  0x0
len             dd  0x00

labelt:         db      'HTTPC - PgUp/PgDown'
labellen:

server_ip:      db      207,44,212,20
dns_ip:         db      194,145,128,1
webAddr:        times 128 db ' '
document_user:  db      'Click on the button to the left to enter a URL',0
times  100      db      0x0
document:       db      '/'
times  100      db      ' '

string0:        db      'GET '

stringh:        db      ' HTTP/1.0',13,10,13,10
stringh_end:

status          dd      0x0
prev_status     dd      0x0

onoff           dd      0x0

nextupdate:     dd      0
winys:          dd      400

dnsMsgLen:      dd 0
socketNum:      dd 0xFFFF
dnsMsg:
I_END:
