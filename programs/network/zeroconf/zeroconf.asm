;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2013. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  zeroconfig.asm - Zeroconfig service for KolibriOS              ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;    Some code contributed by Derpenguin                          ;;
;;                                                                 ;;
;;  DHCP code is based on that by Mike Hibbet                      ;;
;;      (DHCP client for menuetos)                                 ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

; CONFIGURATION

TIMEOUT                 = 3             ; in seconds
BUFFER                  = 1024          ; in bytes
DHCP_TRIES              = 3             ; number of times to try contacting DHCP server
__DEBUG__               = 1             ; enable/disable
__DEBUG_LEVEL__         = 2             ; 1 = all, 2 = errors

; CONFIGURATION FOR LINK-LOCAL

PROBE_WAIT              = 1             ; second  (initial random delay)
PROBE_MIN               = 1             ; second  (minimum delay till repeated probe)
PROBE_MAX               = 2             ; seconds (maximum delay till repeated probe)
PROBE_NUM               = 3             ;         (number of probe packets)

ANNOUNCE_NUM            = 2             ;         (number of announcement packets)
ANNOUNCE_INTERVAL       = 2             ; seconds (time between announcement packets)
ANNOUNCE_WAIT           = 2             ; seconds (delay before announcing)

MAX_CONFLICTS           = 10            ;         (max conflicts before rate limiting)

RATE_LIMIT_INTERVAL     = 60            ; seconds (delay between successive attempts)

DEFEND_INTERVAL         = 10            ; seconds (min. wait between defensive ARPs)

use32
        org     0x0

        db      'MENUET01'              ; 8 byte id
        dd      0x01                    ; header version
        dd      START                   ; start of code
        dd      IM_END                  ; size of image
        dd      (I_END+0x100)           ; memory for app
        dd      (I_END+0x100)           ; esp
        dd      0, 0                    ; I_Param, I_Path


include '../../proc32.inc'
include '../../macros.inc'
include '../../debug-fdo.inc'
include '../../network.inc'
include 'dhcp.inc'
include '../../dll.inc'


Ip2dword:
    push    edx

    ; This code validates if the query is an IP containing 4 numbers and 3 dots

    xor     al, al            ; make al (dot count) zero

   @@:
    cmp     byte[edx],'0'     ; check if this byte is a number, if not jump to no_IP
    jl      no_IP             ;
    cmp     byte[edx],'9'     ;
    jg      no_IP             ;

    inc     edx               ; the byte was a number, so lets check the next byte

    cmp     byte[edx],0       ; is this byte zero? (have we reached end of query?)
    jz      @f                ; jump to next @@ then
    cmp     byte[edx],':'
    jz      @f

    cmp     byte[edx],'.'     ; is this byte a dot?
    jne     @r                ; if not, jump to previous @@

    inc     al                ; the byte was a dot so increment al(dot count)
    inc     edx               ; next byte
    jmp     @r                ; lets check for numbers again (jump to previous @@)

   @@:                        ; we reach this when end of query reached
    cmp     al,3              ; check if there where 3 dots
    jnz     no_IP             ; if not, jump to no_IP

    ; The following code will convert this IP into a dword and output it in eax
    ; If there is also a port number specified, this will be returned in ebx, otherwise ebx is -1

    pop     esi               ; edx (query address) was pushed onto stack and is now popped in esi

    xor     edx, edx          ; result
    xor     eax, eax          ; current character
    xor     ebx, ebx          ; current byte

  .outer_loop:
    shl     edx, 8
    add     edx, ebx
    xor     ebx, ebx
  .inner_loop:
    lodsb
    test    eax, eax
    jz      .finish
    cmp     al, '.'
    jz      .outer_loop
    sub     eax, '0'
    imul    ebx, 10
    add     ebx, eax
    jmp     .inner_loop
  .finish:
    shl     edx, 8
    add     edx, ebx

    bswap   edx               ; we want little endian order

    ret

no_IP:
    pop     edx
    xor     edx, edx

    ret






START:
        mcall   40, EVM_STACK2

        DEBUGF  1,">Zero-config service loaded\n"

  .wait:
        mov     ebx, API_ETH + 0
        mov     bh, [device]
        mcall   76                              ; get MAC of ethernet interface 1
        cmp     eax, -1
        jne     .start

        mcall   10
        jmp     .wait

  .start:
        mov     word[MAC], bx
        mov     dword[MAC+2], eax
        DEBUGF  1,"->MAC: %x-%x-%x-%x-%x-%x\n", [MAC+0]:2, [MAC+1]:2, [MAC+2]:2, [MAC+3]:2, [MAC+4]:2, [MAC+5]:2

        mcall   40, EVM_STACK

        mcall   68, 11

        stdcall dll.Load,@IMPORT
        or      eax, eax
        jnz     try_dhcp

        invoke  ini.get_str, path, str_ipconfig, str_type, inibuf, 16, 0

        cmp     dword[inibuf], 'stat'
        jne     try_dhcp

        invoke  ini.get_str, path, str_ipconfig, str_ip, inibuf, 16, 0
        mov     edx, inibuf
        call    Ip2dword
        mcall   76, API_IPv4 + 3, edx

        invoke  ini.get_str, path, str_ipconfig, str_gateway, inibuf, 16, 0
        mov     edx, inibuf
        call    Ip2dword
        mcall   76, API_IPv4 + 9, edx

        invoke  ini.get_str, path, str_ipconfig, str_dns, inibuf, 16, 0
        mov     edx, inibuf
        call    Ip2dword
        mcall   76, API_IPv4 + 5, edx

        invoke  ini.get_str, path, str_ipconfig, str_subnet, inibuf, 16, 0
        mov     edx, inibuf
        call    Ip2dword
        mcall   76, API_IPv4 + 7, edx


        mcall   -1


try_dhcp:

        DEBUGF  1,"->Trying DHCP\n"

        mcall   75, 0, AF_INET4, SOCK_DGRAM, 0          ; open socket (parameters: domain, type, reserved)
        cmp     eax, -1
        je      error
        mov     [socketNum], eax

        DEBUGF  1,"->Socket %x opened\n", eax

        mcall   75, 2, [socketNum], sockaddr1, 18       ; bind socket to local port 68
        cmp     eax, -1
        je      error

        DEBUGF  1,"->Socket Bound to local port 68\n"

        mcall   75, 4, [socketNum], sockaddr2, 18       ; connect to 255.255.255.255 on port 67
        cmp     eax, -1
        je      error

        DEBUGF  1,"->Connected to 255.255.255.255 on port 67\n"

        mov     [dhcpMsgType], 0x01                     ; DHCP discover
        mov     [dhcpLease], esi                        ; esi is still -1 (-1 = forever)

        mcall   26, 9                                   ; Get system time
        imul    eax, 100
        mov     [currTime], eax

build_request:                                          ; Creates a DHCP request packet.

        mov     [tries], DHCP_TRIES

        DEBUGF  1,"->Building request\n"

        stdcall mem.Alloc, BUFFER
        mov     [dhcpMsg], eax
        test    eax, eax
        jz      dhcp_error

            ;;; todo: skip this bullcrap

        mov     edi, eax
        mov     ecx, BUFFER
        xor     eax, eax
        rep     stosb

            ;; todo: put this in a buffer instead of writing bytes and words!

        mov     edx, [dhcpMsg]

        ; Boot protocol legacy
        mov     [edx], byte 0x01                ; Boot request
        mov     [edx+1], byte 0x01              ; Ethernet
        mov     [edx+2], byte 0x06              ; Ethernet h/w len
        mov     [edx+4], dword 0x11223344       ; xid                 ;;;;;;;
        mov     eax, [currTime]
        mov     [edx+8], eax                    ; secs, our uptime
        mov     [edx+10], byte 0x80             ; broadcast flag set
        mov     eax, dword [MAC]                ; first 4 bytes of MAC
        mov     [edx+28],dword eax
        mov     ax, word [MAC+4]                ; last 2 bytes of MAC
        mov     [edx+32],word ax

        ; DHCP extension
        mov     [edx+236], dword 0x63538263     ; magic cookie
        mov     [edx+240], word 0x0135          ; option DHCP msg type
        mov     al, [dhcpMsgType]
        mov     [edx+240+2], al
        mov     [edx+240+3], word 0x0433        ; option Lease time = infinity
        mov     eax, [dhcpLease]
        mov     [edx+240+5], eax
        mov     [edx+240+9], word 0x0432        ; option requested IP address
        mov     eax, [dhcp.ip]
        mov     [edx+240+11], eax
        mov     [edx+240+15], word 0x0437       ; option request list
        mov     [edx+240+17], dword 0x0f060301

        cmp     [dhcpMsgType], byte 0x01        ; Check which msg we are sending
        jne     request_options

        mov     [edx+240+21], byte 0xff         ; end of options marker

        mov     [dhcpMsgLen], 262       ; length
        jmp     send_dhcpmsg

request_options:
        mov     [edx+240+21], word 0x0436       ; server IP
        mov     eax, [dhcpServerIP]
        mov     [edx+240+23], eax

        mov     [edx+240+27], byte 0xff         ; end of options marker

        mov     [dhcpMsgLen], 268       ; length

send_dhcpmsg:
        DEBUGF  1,"Sending DHCP discover/request\n"
        mcall   75, 6, [socketNum], [dhcpMsg], [dhcpMsgLen]     ; write to socket ( send broadcast request )
        mcall   23, TIMEOUT*100                                 ; wait for data

read_data:                                                      ; we have data - this will be the response
        mcall   75, 7, [socketNum], [dhcpMsg], BUFFER, MSG_DONTWAIT     ; read data from socket
        cmp     eax, -1
        jne     @f
        DEBUGF  1,"No answer from DHCP server\n"
        dec     [tries]
        jnz     send_dhcpmsg                    ; try again
        jmp     dhcp_error                      ; fail

  @@:
        DEBUGF  1,"->%d bytes received\n", eax
        mov     [dhcpMsgLen], eax

; depending on which msg we sent, handle the response
; accordingly.
; If the response is to a dhcp discover, then:
;  1) If response is DHCP OFFER then
;  1.1) record server IP, lease time & IP address.
;  1.2) send a request packet
; If the response is to a dhcp request, then:
;  1) If the response is DHCP ACK then
;  1.1) extract the DNS & subnet fields. Set them in the stack

        cmp     [dhcpMsgType], 0x01             ; did we send a discover?
        je      discover

        cmp     [dhcpMsgType], 0x03             ; did we send a request?
        je      request

        call    dhcp_end                        ; we should never reach here ;)
        jmp     exit

discover:
        call    parse_response

        cmp     [dhcpMsgType], 0x02             ; Was the response an offer?
        je      send_request

        call    dhcp_end
        jmp     link_local

send_request:
        DEBUGF  1, "Got offer, making request\n"
        mov     [dhcpMsgType], 0x03             ; make it a request
        jmp     build_request

request:
        call    parse_response

        cmp     [dhcpMsgType], 0x05             ; Was the response an ACK? It should be
        jne     read_data                       ; NO - read next packets

        DEBUGF  2, "Setting IP using DHCP\n"

        call    dhcp_end

        mov     ebx, API_IPv4 + 3
        mov     bh, [device]
        mcall   76, , [dhcp.ip]                 ; ip
        mov     bl, 5
        mcall   76, , [dhcp.dns]                ; dns
        mov     bl, 7
        mcall   76, , [dhcp.subnet]             ; subnet
        mov     bl, 9
        mcall   76, , [dhcp.gateway]            ; gateway

        jmp     exit

dhcp_end:
        mcall   close, [socketNum]
        stdcall mem.Free, [dhcpMsg]

        ret

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
parse_response:

        DEBUGF  1,"Data received, parsing response\n"
        mov     edx, [dhcpMsg]

        push    dword [edx+16]
        pop     [dhcp.ip]
        DEBUGF  1,"Client: %u.%u.%u.%u\n", [edx+16]:1, [edx+17]:1, [edx+18]:1, [edx+19]:1

; TODO: check if there really are options

        mov     al, 240                         ; Point to first option
        movzx   ecx, al

  .next_option:
        add     edx, ecx

        mov     al, [edx]                       ; get message identifier

        cmp     al, 0xff                        ; End of options?
        je      .done

        cmp     al, 0
        je      .pad

; TODO: check if we still are inside the buffer

        inc     edx
        movzx   ecx, byte [edx]                 ; get data length
        inc     edx                             ; point to data

        cmp     al, dhcp_msg_type               ; Msg type is a single byte option
        je      .msgtype

        cmp     al, dhcp_dhcp_server_id
        je      .server

        cmp     al, dhcp_address_time
        je      .lease

        cmp     al, dhcp_subnet_mask
        je      .subnet

        cmp     al, dhcp_router
        je      .router

        cmp     al, dhcp_domain_server
        je      .dns

        DEBUGF  1,"Unsupported DHCP option: %u\n", al

        jmp     .next_option

  .pad:
        xor     ecx, ecx
        inc     ecx
        jmp     .next_option

  .msgtype:
        mov     al, [edx]
        mov     [dhcpMsgType], al

        DEBUGF  1,"DHCP Msg type: %u\n", al
        jmp     .next_option                    ; Get next option

  .server:
        mov     eax, [edx]
        mov     [dhcpServerIP], eax
        DEBUGF  1,"Server: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
        jmp     .next_option

  .lease:
        pusha
        mov     eax,[edx]
        bswap   eax
        mov     [dhcpLease],eax
        DEBUGF  1,"lease: %d\n",eax
        popa
        jmp     .next_option

  .subnet:
        push    dword [edx]
        pop     [dhcp.subnet]
        DEBUGF  1,"Subnet: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
        jmp     .next_option

  .router:
        push    dword [edx]
        pop     [dhcp.gateway]
        DEBUGF  1,"Gateway: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
        jmp     .next_option

  .dns:
        push    dword [edx]
        pop     [dhcp.dns]
        DEBUGF  1,"DNS: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
        jmp     .next_option

  .done:
        ret



dhcp_error:
        call    dhcp_end

link_local:
        call    random
        mov     cx, ax
        shl     ecx, 16
        mov     cx, 0xfea9                              ; IP 169.254.0.0 link local net, see RFC3927
        mov     ebx, API_IPv4 + 3
        mov     bh, [device]
        mcall   76, , ecx                   ; mask is 255.255.0.0
        DEBUGF  2,"Link Local IP assigned: 169.254.%u.%u\n", [generator+0]:1, [generator+1]:1
        mov     bl, 7
        mcall   76, , 0xffff
        mov     bl, 9
        mcall   76, , 0x0
        mov     bl, 5
        mcall   76, , 0x0

        mcall   5, PROBE_WAIT*100

        xor     esi, esi
   probe_loop:
        call    random                                  ; create a pseudo random number in eax (seeded by MAC)

        cmp     al, PROBE_MIN*100                       ; check if al is bigger then PROBE_MIN
        jae     @f                                      ; all ok
        add     al, (PROBE_MAX-PROBE_MIN)*100           ; al is too small
   @@:

        cmp     al, PROBE_MAX*100
        jbe     @f
        sub     al, (PROBE_MAX-PROBE_MIN)*100
   @@:

        movzx   ebx,al
        DEBUGF  1,"Waiting %u0ms\n",ebx
        mcall   5

        DEBUGF  1,"Sending Probe\n"
        mov     ebx, API_ARP + 6
        mov     bh, [device]
        mcall   76
        inc     esi

        cmp     esi, PROBE_NUM
        jb      probe_loop

; now we wait further ANNOUNCE_WAIT seconds and send ANNOUNCE_NUM ARP announces. If any other host has assingned
; IP within this time, we should create another adress, that have to be done later

        DEBUGF  1,"Waiting %us\n", ANNOUNCE_WAIT
        mcall   5, ANNOUNCE_WAIT*100
        xor   esi, esi
   announce_loop:

        DEBUGF  1,"Sending Announce\n"
        mov     ebx, API_ARP + 6
        mov     bh, [device]
        mcall   76

        inc     esi
        cmp     esi,ANNOUNCE_NUM
        je      @f

        DEBUGF  1,"Waiting %us\n", ANNOUNCE_INTERVAL
        mcall   5, ANNOUNCE_INTERVAL*100
        jmp     announce_loop
   @@:


error:
        DEBUGF  2,"Socket error\n"
exit:   ; we should, instead of closing, detect ARP conflicts and detect if cable keeps connected ;)
        mcall   -1


random:  ; Pseudo random actually

        mov     eax, [generator]
        add     eax, -43ab45b5h
        ror     eax, 1
        bswap   eax
        xor     eax, dword[MAC]
        ror     eax, 1
        xor     eax, dword[MAC+2]
        mov     [generator], eax

        ret

; DATA AREA

align 16
@IMPORT:

library \
        libini,'libini.obj'

import  libini, \
        ini.get_str,'ini_get_str'

include_debug_strings

str_ip          db 'ip', 0
str_subnet      db 'subnet', 0
str_gateway     db 'gateway', 0
str_dns         db 'dns', 0
str_ipconfig    db 'ipconfig', 0
str_type        db 'type', 0


sockaddr1:

        dw AF_INET4
        dw 68 shl 8     ; local port
        dd 0            ; local IP

        rb 10


sockaddr2:

        dw AF_INET4
        dw 67 shl 8     ; destination port
        dd -1           ; destination IP

        rb 10

path            db  '/sys/network.ini'

IM_END:

device          db 1
inibuf          rb 16
tries           db ?

dhcpMsgType     db ?
dhcpLease       dd ?
dhcpServerIP    dd ?

dhcp:
.ip             dd ?
.subnet         dd ?
.dns            dd ?
.gateway        dd ?


dhcpMsgLen      dd ?
socketNum       dd ?

MAC             dp ?

currTime        dd ?
generator       dd ?

dhcpMsg         dd ?

I_END: