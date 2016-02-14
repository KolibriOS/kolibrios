;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2016. All rights reserved.    ;;
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

TIMEOUT                 = 5             ; in seconds
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

MAX_INTERFACES          = 8

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

struct  dhcp_msg
        op              db ?    ; Operation Code
        htype           db ?    ; Hardware type
        hlen            db ?    ; Hardware address length
        hops            db ?
        xid             dd ?    ; Transaction Identifier
        secs            dw ?    ; Seconds since boot
        flags           dw ?
        ciaddr          dd ?    ; Client IP address
        yiaddr          dd ?    ; "Your" IP address
        siaddr          dd ?    ; Server IP address
        giaddr          dd ?    ; Gateway IP address
        chaddr          rb 16   ; Client hardware address
        sname           rb 64   ; Server name
        file            rb 128  ; boot filename
        cookie          dd ?    ; Magic cookie (0x63538263)
        options         rb 512
ends

struct  interface
        number          dd ?
        state           dd ?    ; 0 - disconnected, 1 - connected
        mode            dd ?    ; 0 - disabled, 1 - static, 2 - dhcp, 3 - auto (zero config)
        tries           dd ?
        lease           dd ?
        ServerIP        dd ?
        ip              dd ?
        subnet          dd ?
        dns             dd ?
        gateway         dd ?
        socketNum       dd ?
        timeout         dd ?
        ip_conflicts    dd ?
ends

START:
        mcall   68, 11                  ; init heap

        stdcall dll.Load, @IMPORT       ; load libraries
        or      eax, eax
        jnz     exit_immediately

        DEBUGF  2, "Zero-config service loaded\n"

        mcall   40, EVM_STACK2          ; We only want low-level network events

; Set up interface list
        mov     edi, device_list
        xor     ebx, ebx
  @@:
        inc     ebx
        mov     eax, ebx
        stosd
        mov     ecx, sizeof.interface/4-1
        xor     eax,eax
        rep stosd
        cmp     ebx, MAX_INTERFACES
        jb      @b

        mov     ebp, device_list
mainloop:
        cmp     [ebp + interface.state], 0
        je      .link_up?
        jmp     .maintain_link

  .next:
        cmp     [ebp + interface.number], MAX_INTERFACES
        je      .wait
        add     ebp, sizeof.interface
        jmp     mainloop

  .wait:
        mcall   10                      ; Wait for event
        mov     ebp, device_list
        jmp     mainloop

  .link_up?:
        mov     bh, byte[ebp + interface.number]
        mov     bl, 0                   ; Get device type
        mcall   74
        cmp     eax, 1                  ; Ethernet
        jne     mainloop.next

        mov     bl, 10                  ; Get Link status
        mcall   74
        test    eax, eax
        jz      mainloop.next

        mov     [ebp + interface.state], 1

        call    create_str_ini_int

; Try to read settings from .ini file
        invoke  ini.get_str, ini_path, str_ini_int, str_ip_type, inibuf, 16, str_null
        test    eax, eax
        jz      @f
; If settings not found, use default settings from 'ip?' section
        mov     dword[str_ini_int], 'ip?'
  @@:

        mov     ebx, API_ETH + 0
        mov     bh, byte[ebp + interface.number]
        mcall   76                      ; get MAC of the ethernet interface
        mov     word[tx_msg.chaddr], bx
        mov     dword[tx_msg.chaddr+2], eax
        DEBUGF  1, "MAC: %x-%x-%x-%x-%x-%x\n", \
        [tx_msg.chaddr+0]:2, [tx_msg.chaddr+1]:2, [tx_msg.chaddr+2]:2, \
        [tx_msg.chaddr+3]:2, [tx_msg.chaddr+4]:2, [tx_msg.chaddr+5]:2

        invoke  ini.get_str, ini_path, str_ini_int, str_ip_type, inibuf, 16, str_null
        test    eax, eax
        jnz     .invalid
        mov     eax, dword[inibuf]
        or      eax, 0x20202020
        mov     [ebp + interface.mode], 0
        cmp     eax, 'disa'
        je      .next
        mov     [ebp + interface.mode], 1
        cmp     eax, 'stat'
        je      static
        mov     [ebp + interface.mode], 2
        cmp     eax, 'dhcp'
        je      dhcp
        mov     [ebp + interface.mode], 3
        cmp     eax, 'auto'
        je      dhcp

  .invalid:
        DEBUGF  2, "Invalid settings for interface: %s.\n", str_ini_int
        jmp     .next

  .maintain_link:

; Check for IP conflicts
        mov     ebx, API_ARP
        mov     bh, byte[ebp + interface.number]
        mov     bl, 7
        mcall   76                      ; Number of IP conflicts
        cmp     eax, [ebp + interface.ip_conflicts]
        je      @f
        mov     [ebp + interface.ip_conflicts], eax
        DEBUGF  2, "IP address conflict on interface %u\n", [ebp + interface.number]
        ; Notify user of the IP address conflict
        mov     [notify_struct.msg], str_conflict
        mcall   70, notify_struct
  @@:

; Check if device is still there
        mov     bh, byte[ebp + interface.number]
        mov     bl, 0                   ; Get device type
        mcall   74
        test    eax, eax                ; No device
        jz      .link_down

; Check if link is still there
        mov     bl, 10                  ; Get Link status
        mcall   74
        test    eax, eax
        jnz     .next

  .link_down:
        mov     [ebp + interface.state], 0

; Notify user that the link is down
        mov     [notify_struct.msg], str_disconnected
        mcall   70, notify_struct

; CHECKME: should we do this in kernel instead? Should we even do this at all?
        xor     ecx, ecx
        mov     ebx, API_IPv4 + 3
        mov     bh, byte[ebp + interface.number]
        mcall   76                      ; ip
        mov     bl, 5
        mcall   76                      ; dns
        mov     bl, 7
        mcall   76                      ; subnet
        mov     bl, 9
        mcall   76                      ; gateway

        jmp     .next

link_up:

; Read number of previous IP conflicts
        mov     ebx, API_ARP
        mov     bh, byte[ebp + interface.number]
        mov     bl, 7
        mcall   76
        mov     [ebp + interface.ip_conflicts], eax

; Notify user that the link is up and running
        mov     [notify_struct.msg], str_connected
        mcall   70, notify_struct

  .fail:
        mcall   40, EVM_STACK2
        jmp     mainloop.next

static:
        DEBUGF  1, "Applying Static IP settings\n"

        invoke  ini.get_str, ini_path, str_ini_int, str_ip, inibuf, 16, str_null
        mov     esi, inibuf
        call    ip_str_to_dword
        mov     ebx, API_IPv4 + 3       ; set IP
        mov     bh, byte[ebp + interface.number]
        mcall   76

        invoke  ini.get_str, ini_path, str_ini_int, str_subnet, inibuf, 16, str_null
        mov     esi, inibuf
        call    ip_str_to_dword
        mov     ebx, API_IPv4 + 7       ; set subnet
        mov     bh, byte[ebp + interface.number]
        mcall   76

        invoke  ini.get_str, ini_path, str_ini_int, str_gateway, inibuf, 16, str_null
        mov     esi, inibuf
        call    ip_str_to_dword
        mov     ebx, API_IPv4 + 9       ; set gateway
        mov     bh, byte[ebp + interface.number]
        mcall   76

  .dns:
        invoke  ini.get_str, ini_path, str_ini_int, str_dns, inibuf, 16, str_null
        mov     esi, inibuf
        call    ip_str_to_dword
        mov     ebx, API_IPv4 + 5       ; set DNS
        mov     bh, byte[ebp + interface.number]
        mcall   76

        jmp     link_up


dhcp:

        DEBUGF  2, "Trying to contact DHCP server\n"

        mcall   40, EVM_STACK

        mcall   75, 0, AF_INET4, SOCK_DGRAM, 0                          ; open socket (parameters: domain, type, reserved)
        cmp     eax, -1
        je      dhcp_error
        mov     [ebp + interface.socketNum], eax

        DEBUGF  1, "Socket %x opened\n", eax

        mcall   75, 2, [ebp + interface.socketNum], sock_local, 18      ; bind socket to local port 68
        cmp     eax, -1
        je      socket_error

        DEBUGF  1, "Socket Bound to local port 68\n"

        pushd   [ebp + interface.number]
        pushd   4                       ; length of option
        pushd   SO_BINDTODEVICE
        pushd   SOL_SOCKET
        mcall   75, 8, [ebp + interface.socketNum], esp
        add     esp, 16
        cmp     eax, -1
        je      socket_error

        DEBUGF  1, "Socket Bound to local interface %u\n", [ebp + interface.number]

        mcall   75, 4, [ebp + interface.socketNum], sock_remote, 18     ; connect to 255.255.255.255 on port 67
        cmp     eax, -1
        je      socket_error

        DEBUGF  1, "Connected to 255.255.255.255 on port 67\n"

        ; Read preferred IP address from settings file
        invoke  ini.get_str, ini_path, str_ini_int, str_ip, inibuf, 16, str_null
        mov     esi, inibuf
        call    ip_str_to_dword
        mov     [ebp + interface.ip], ecx

        call    random
        mov     [tx_msg.xid], eax                                       ; randomize session ID
        mov     [tx_msg_type], 1                                        ; DHCP discover

build_dhcp_packet:

        DEBUGF  1, "Building DHCP packet\n"

        mov     [ebp + interface.tries], DHCP_TRIES

        ; Boot protocol legacy
        mov     [tx_msg.op], 1                                          ; Boot request
        mov     [tx_msg.htype], 1                                       ; Ethernet
        mov     [tx_msg.hlen], 6                                        ; Ethernet address h/w len
        mov     [tx_msg.hops], 0
        mcall   26, 9                                                   ; Time since boot
        xor     edx, edx
        mov     ebx, 100
        div     ebx                                                     ; Divide by 100 to get number of seconds
        xchg    al, ah                                                  ; Convert to big endian
        mov     [tx_msg.secs], ax
        mov     [tx_msg.flags], 0

        ; DHCP extension
        mov     [tx_msg.cookie], 0x63538263                             ; magic cookie

        mov     word[tx_msg+240], 0x0135                                ; option DHCP msg type
        mov     al,[tx_msg_type]
        mov     [tx_msg+240+2], al

        mov     word[tx_msg+240+3], 0x0433                              ; option Lease time
        mov     dword[tx_msg+240+5], -1                                 ; infinite

        mov     word[tx_msg+240+9], 0x0432                              ; option requested IP address
        mov     eax,[ebp + interface.ip]
        mov     [tx_msg+240+11], eax

        mov     word[tx_msg+240+15], 0x0437                             ; option request list
        mov     dword[tx_msg+240+17], 0x0f060301

        cmp     [tx_msg_type], 1                                        ; Check which msg we are sending
        jne     .request

        mov     byte[tx_msg+240+21], 0xff                               ; end of options marker

        mov     [tx_msg_len], 262                                       ; length
        jmp     send_dhcp_packet

  .request:
        mov     word[tx_msg+240+21], 0x0436                             ; server IP
        mov     eax,[ebp + interface.ServerIP]
        mov     [tx_msg+240+23], eax

        mov     byte[tx_msg+240+27], 0xff                               ; end of options marker

        mov     [tx_msg_len], 268                                       ; length


send_dhcp_packet:
        DEBUGF  1, "Sending DHCP packet\n"
        lea     edx, [tx_msg]
        mcall   75, 6, [ebp + interface.socketNum], , [tx_msg_len]

; Wait for reply
        mcall   26, 9
        add     eax, TIMEOUT*100
        mov     [ebp + interface.timeout], eax
        mov     ebx, TIMEOUT*100
  .wait:
        mcall   23                                                      ; Wait for event with timeout
read_packet:                                                            ; we have data - this will be the response
        lea     edx, [rx_msg]
        mcall   75, 7, [ebp + interface.socketNum], , BUFFER, MSG_DONTWAIT    ; read data from socket
        cmp     eax, -1
        jne     .got_data

        mcall   26, 9
        mov     ebx, eax
        sub     ebx, [ebp + interface.timeout]
        ja      send_dhcp_packet.wait

        DEBUGF  1, "No answer from DHCP server\n"
        dec     [ebp + interface.tries]
        jnz     send_dhcp_packet
        jmp     dhcp_fail

  .got_data:
        DEBUGF  1, "%d bytes received\n", eax
        mov     [rx_msg_len], eax

; depending on which msg we sent, handle the response
; accordingly.
; If the response is to a dhcp discover, then:
;  1) If response is DHCP OFFER then
;  1.1) record server IP, lease time & IP address.
;  1.2) send a request packet
; If the response is to a dhcp request, then:
;  1) If the response is DHCP ACK then
;  1.1) extract the DNS & subnet fields. Set them in the stack

        cmp     [tx_msg_type], 1                ; did we send a discover?
        je      discover_sent
        cmp     [tx_msg_type], 3                ; did we send a request?
        je      request_sent
        jmp     exit_immediately

discover_sent:
        call    parse_dhcp_reply
        cmp     [rx_msg_type], 2                ; Was the response an offer?
        jne     read_packet

        DEBUGF  1, "Got offer, making request\n"
        mov     [tx_msg_type], 3                ; make it a request
        jmp     build_dhcp_packet

request_sent:
        call    parse_dhcp_reply
        cmp     [rx_msg_type], 5                ; Was the response an ACK? It should be
        jne     read_packet                     ; NO - read next packets

        DEBUGF  2, "IP address %u.%u.%u.%u assigned to network interface %u by DHCP\n",\
        [ebp+interface.ip+0]:1, [ebp+interface.ip+1]:1, [ebp+interface.ip+2]:1, [ebp+interface.ip+3]:1, [ebp + interface.number]:1

        mcall   close, [ebp + interface.socketNum]

        mov     ebx, API_IPv4 + 3
        mov     bh, byte[ebp + interface.number]
        mcall   76, , [ebp + interface.ip]            ; ip
        mov     bl, 7
        mcall   76, , [ebp + interface.subnet]        ; subnet
        mov     bl, 9
        mcall   76, , [ebp + interface.gateway]       ; gateway

        invoke  ini.get_str, ini_path, str_ini_int, str_dns_type, inibuf, 16, str_null
        test    eax, eax
        jnz     @f
        mov     eax, dword[inibuf]
        or      eax, 0x202020
        cmp     eax, 'stat'
        je      static.dns
  @@:
        mov     ebx, API_IPv4 + 5
        mov     bh, byte[ebp + interface.number]
        mcall   76, , [ebp + interface.dns]           ; dns

        jmp     link_up


parse_dhcp_reply:

        DEBUGF  1, "Parsing response\n"
        mov     [rx_msg_type], 0

; Verify if session ID matches
        mov     eax, [tx_msg.xid]
        cmp     [rx_msg.xid], eax
        jne     .done

        pushd   [rx_msg.yiaddr]
        pop     [ebp + interface.ip]
        DEBUGF  1, "Client: %u.%u.%u.%u\n", \
        [rx_msg.yiaddr]:1, [rx_msg.yiaddr+1]:1, [rx_msg.yiaddr+2]:1, [rx_msg.yiaddr+3]:1

; Verify magic cookie
        cmp     [rx_msg.cookie], 0x63538263
        jne     .done

; Parse the DHCP options
        lea     esi, [rx_msg]
        mov     ecx, 240                        ; point to the first option
  .next_option:
; TODO: check if we still are inside the buffer!
        add     esi, ecx

        lodsb                                   ; get message identifier
        mov     bl, al
        cmp     bl, 0xff                        ; End of options?
        je      .done
        test    bl, bl
        jz      .pad

        lodsb                                   ; load data length
        movzx   ecx, al
        cmp     bl, dhcp_msg_type               ; Msg type is a single byte option
        je      .msgtype
        cmp     bl, dhcp_dhcp_server_id
        je      .server
        cmp     bl, dhcp_address_time
        je      .lease
        cmp     bl, dhcp_subnet_mask
        je      .subnet
        cmp     bl, dhcp_router
        je      .router
        cmp     bl, dhcp_domain_server
        je      .dns

        DEBUGF  1, "Unsupported DHCP option: %u\n", bl
        jmp     .next_option

  .pad:
        xor     ecx, ecx
        inc     ecx
        jmp     .next_option

  .msgtype:
        mov     al, [esi]
        mov     [rx_msg_type], al

        DEBUGF  1, "DHCP Msg type: %u\n", al
        jmp     .next_option                    ; Get next option

  .server:
        pushd   [esi]
        pop     [ebp + interface.ServerIP]
        DEBUGF  1, "Server: %u.%u.%u.%u\n", [esi]:1, [esi+1]:1, [esi+2]:1, [esi+3]:1
        jmp     .next_option

  .lease:
        pusha
        mov     eax,[esi]
        bswap   eax
        mov     [ebp + interface.lease], eax
        DEBUGF  1, "Lease: %d\n", eax
        popa
        jmp     .next_option

  .subnet:
        pushd   [esi]
        pop     [ebp + interface.subnet]
        DEBUGF  1, "Subnet: %u.%u.%u.%u\n", [esi]:1, [esi+1]:1, [esi+2]:1, [esi+3]:1
        jmp     .next_option

  .router:
        pushd   [esi]
        pop     [ebp + interface.gateway]
        DEBUGF  1, "Gateway: %u.%u.%u.%u\n", [esi]:1, [esi+1]:1, [esi+2]:1, [esi+3]:1
        jmp     .next_option

  .dns:
        pushd   [esi]
        pop     [ebp + interface.dns]
        DEBUGF  1, "DNS: %u.%u.%u.%u\n", [esi]:1, [esi+1]:1, [esi+2]:1, [esi+3]:1
        jmp     .next_option

  .done:
        ret

exit_immediately:
        DEBUGF  2, "Zeroconf failed!\n"
        mcall   -1

socket_error:
        DEBUGF  2, "Socket error!\n"

dhcp_fail:
        mcall   close, [ebp + interface.socketNum]

dhcp_error:
        DEBUGF  2, "DHCP failed\n"
        cmp     [ebp + interface.mode], 3               ; zero config mode?
        jne     link_up

link_local:

; TODO: send ARP probes before setting the IP address in stack!

        call    random
        mov     cx, ax
        shl     ecx, 16
        mov     cx, 0xfea9                              ; IP 169.254.0.0 link local net, see RFC3927
        mov     ebx, API_IPv4 + 3
        mov     bh, byte[ebp + interface.number]
        mcall   76, , ecx                               ; mask is 255.255.0.0
        DEBUGF  2, "IP address 169.254.%u.%u assigned to network interface %u through Link-Local\n",\
        [generator+0]:1, [generator+1]:1, [ebp + interface.number]:1
        mov     bl, 7
        mcall   76, , 0xffff
        mov     bl, 9
        mcall   76, , 0x0
        mov     bl, 5
        mcall   76, , 0x0

        jmp     link_up


random:  ; Pseudo random actually

        mov     eax,[generator]
        add     eax, -43ab45b5h
        ror     eax, 1
        bswap   eax
        xor     eax, dword[tx_msg.chaddr]
        ror     eax, 1
        xor     eax, dword[tx_msg.chaddr+2]
        mov     [generator], eax

        ret



create_str_ini_int:
        mov     eax, [ebp + interface.number]
        mov     ebx, 10
        xor     edx, edx
        push    0
  @@:
        div     ebx
        add     dl, '0'
        push    edx
        test    eax, eax
        jnz     @r
  @@:
        mov     edi, str_ini_int+2
  @@:
        pop     eax
        stosb
        test    eax, eax
        jnz     @r

        ret



; In: esi = ptr to ASCIIZ IP address
; Out: ecx = IP (0 on error)

ip_str_to_dword:

        xor     ecx, ecx        ; end result
  .charloop:
        lodsb
        test    al, al
        jz      .finish
        cmp     al, '.'
        je      .dot
        sub     al, '0'
        jb      .fail
        cmp     al, 9
        ja      .fail
        mov     dl, cl
        shl     cl, 2
        jc      .fail
        add     cl, dl
        jc      .fail
        add     cl, cl
        jc      .fail
        add     cl, al
        jc      .fail
        jmp     .charloop
  .dot:
        shl     ecx, 8
        jc      .fail
        xor     cl, cl
        jmp     .charloop
  .finish:
        bswap   ecx             ; we want little endian order
        ret

  .fail:
        xor     ecx, ecx
        ret

; DATA AREA

align 16
@IMPORT:

library \
        libini,         'libini.obj'

import  libini, \
        ini.get_str,    'ini_get_str',\
        ini.set_str,    'ini_set_str'

include_debug_strings

str_ip          db 'ip', 0
str_subnet      db 'subnet', 0
str_gateway     db 'gateway', 0
str_dns         db 'dns', 0

str_ip_type     db 'ip_type', 0
str_dns_type    db 'dns_type', 0

str_ini_int     db 'ip1', 0
                rb 10

str_null        db 0

sock_local:
        dw AF_INET4
        dw 68 shl 8     ; local port
        dd 0            ; local IP
        rb 10


sock_remote:
        dw AF_INET4
        dw 67 shl 8     ; destination port
        dd -1           ; destination IP
        rb 10

notify_struct:
        dd 7            ; run application
        dd 0
 .msg   dd 0
        dd 0
        dd 0
        db '/sys/@notify', 0

str_connected           db '"You are now connected to the network." -N', 0
str_disconnected        db '"You are now disconnected from the network." -N', 0
str_conflict            db '"An IP address conflict has been detected on the network." -W', 0

ini_path                db '/sys/settings/network.ini',0

IM_END:

generator       dd ?

inibuf          rb 16

tx_msg_len      dd ?
rx_msg_len      dd ?
tx_msg_type     db ?
rx_msg_type     db ?
tx_msg          dhcp_msg
rx_msg          dhcp_msg

device_list     rd MAX_INTERFACES*sizeof.interface

I_END: