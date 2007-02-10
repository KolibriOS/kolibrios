; Automated dhcp client
; v 1.3
;
; with thanks to authors of DHCP client for menuetos: Mike Hibbet
;
; by HidnPlayr & Derpenguin

use32
	       org    0x0

	       db     'MENUET01'	    ; 8 byte id
	       dd     0x01		    ; header version
	       dd     START		    ; start of code
	       dd     IM_END		    ; size of image
	       dd     I_END		    ; memory for app
	       dd     I_END		    ; esp
	       dd     0x0 , 0x0 	    ; I_Param , I_Icon

; CONFIGURATION


TIMEOUT 	    equ 60		    ; in seconds
BUFFER		    equ 1024		    ; in bytes
__DEBUG__	    equ 1		    ; enable/disable
__DEBUG_LEVEL__     equ 1		    ; 1 = all, 2 = errors

; CONFIGURATION FOR LINK-LOCAL

PROBE_WAIT	    equ 1		    ; second  (initial random delay)
PROBE_MIN	    equ 1		    ; second  (minimum delay till repeated probe)
PROBE_MAX	    equ 2		    ; seconds (maximum delay till repeated probe)
PROBE_NUM	    equ 3		    ;         (number of probe packets)

ANNOUNCE_NUM	    equ 2		    ;         (number of announcement packets)
ANNOUNCE_INTERVAL   equ 2		    ; seconds (time between announcement packets)
ANNOUNCE_WAIT	    equ 2		    ; seconds (delay before announcing)

MAX_CONFLICTS	    equ 10		    ;         (max conflicts before rate limiting)

RATE_LIMIT_INTERVAL equ 60		    ; seconds (delay between successive attempts)

DEFEND_INTERVAL     equ 10		    ; seconds (min. wait between defensive ARPs)

include 'macros.inc'
include 'eth.inc'
include 'debug-fdo.inc'
include 'dhcp.inc'
include 'events.inc'
include 'common.inc'


START:					    ; start of execution

    set_event_mask	event_network
    eth.set_network_drv 0x00000383

    DEBUGF  1,"Stack Initialized.\n"

    eth.status eax			    ; Read the Stack status
    test    eax,eax			    ; if eax is zero, no driver was found
    jnz     @f
    DEBUGF  1,"No Card detected\n"
    jmp     close

   @@:
    DEBUGF  1,"Detected card: %x\n",eax
   @@:
    eth.check_cable eax
    test    al,al
    jnz     @f
    DEBUGF  1,"Ethernet Cable not connected\n"
    wait 500				    ; loop until cable is connected (check every 5 sec)
    jmp     @r

   @@:
    DEBUGF  1,"Ethernet Cable status: %d\n",al

    eth.read_mac MAC
    DEBUGF  1,"MAC address: %x-%x-%x-%x-%x-%x\n",[MAC]:2,[MAC+1]:2,[MAC+2]:2,[MAC+3]:2,[MAC+4]:2,[MAC+5]:2

    eth.check_port 68,eax		    ; Check if port 68 is available
    cmp     eax,1
    je	    @f

    DEBUGF  1,"Port 68 is already in use!\n"
    jmp     close

   @@:
    eth.open_udp 68,67,-1,[socketNum]	    ; open socket (local,remote,ip,socket)
    DEBUGF  1,"Socket opened: %d\n",eax
					    ; Setup the first msg we will send
    mov     byte [dhcpMsgType], 0x01	    ; DHCP discover
    mov     dword [dhcpLease], esi	    ; esi is still -1 (-1 = forever)

    get_time_counter eax
    imul    eax,100
    mov     [currTime],eax

buildRequest:				    ; Creates a DHCP request packet.
    xor     eax,eax			    ; Clear dhcpMsg to all zeros
    mov     edi,dhcpMsg
    mov     ecx,BUFFER
    cld
    rep     stosb

    mov     edx, dhcpMsg

    mov     [edx], byte 0x01		    ; Boot request
    mov     [edx+1], byte 0x01		    ; Ethernet
    mov     [edx+2], byte 0x06		    ; Ethernet h/w len
    mov     [edx+4], dword 0x11223344	    ; xid
    mov     eax,[currTime]
    mov     [edx+8], eax		    ; secs, our uptime
    mov     [edx+10], byte 0x80 	    ; broadcast flag set
    mov     eax, dword [MAC]		    ; first 4 bytes of MAC
    mov     [edx+28],dword eax
    mov     ax, word [MAC+4]		    ; last 2 bytes of MAC
    mov     [edx+32],word ax
    mov     [edx+236], dword 0x63538263     ; magic number
    mov     [edx+240], word 0x0135	    ; option DHCP msg type
    mov     al, [dhcpMsgType]
    mov     [edx+240+2], al
    mov     [edx+240+3], word 0x0433	    ; option Lease time = infinity
    mov     eax, [dhcpLease]
    mov     [edx+240+5], eax
    mov     [edx+240+9], word 0x0432	    ; option requested IP address
    mov     eax, [dhcpClientIP]
    mov     [edx+240+11], eax
    mov     [edx+240+15], word 0x0437	    ; option request list
    mov     [edx+240+17], dword 0x0f060301

    cmp     [dhcpMsgType], byte 0x01	    ; Check which msg we are sending
    jne     request_options

    mov     [edx+240+21], byte 0xff	    ; "Discover" options

    mov     [dhcpMsgLen], dword 262	    ; end of options marker
    jmp     send_request

request_options:
    mov     [edx+240+21], word 0x0436	    ; server IP
    mov     eax, [dhcpServerIP]
    mov     [edx+240+23], eax

    mov     [edx+240+27], byte 0xff	    ; end of options marker

    mov     [dhcpMsgLen], dword 268

send_request:
    eth.write_udp [socketNum],[dhcpMsgLen],dhcpMsg ; write to socket ( send broadcast request )

    mov     eax, dhcpMsg		    ; Setup the DHCP buffer to receive response
    mov     [dhcpMsgLen], eax		    ; Used as a pointer to the data

    mov     eax,23			    ; wait here for event (data from remote)
    mov     ebx,TIMEOUT*10
    int     0x40

    eth.poll [socketNum]

    test    eax,eax
    jnz     read_data

    DEBUGF  2,"Timeout!\n"
    eth.close_udp [socketNum]
    jmp    apipa			    ; no server found, lets try zeroconf


read_data:				    ; we have data - this will be the response
    eth.read_packet [socketNum], dhcpMsg, BUFFER
    mov     [dhcpMsgLen], eax
    eth.close_udp [socketNum]

    ; depending on which msg we sent, handle the response
    ; accordingly.
    ; If the response is to a dhcp discover, then:
    ;  1) If response is DHCP OFFER then
    ;  1.1) record server IP, lease time & IP address.
    ;  1.2) send a request packet
    ; If the response is to a dhcp request, then:
    ;  1) If the response is DHCP ACK then
    ;  1.1) extract the DNS & subnet fields. Set them in the stack

    cmp     [dhcpMsgType], byte 0x01	    ; did we send a discover?
    je	    discover
    cmp     [dhcpMsgType], byte 0x03	    ; did we send a request?
    je	    request

    jmp     close			    ; really unknown, what we did

discover:
    call    parseResponse

    cmp     [dhcpMsgType], byte 0x02	    ; Was the response an offer?
    jne     apipa			    ; NO - so we do zeroconf
    mov     [dhcpMsgType], byte 0x03	    ; DHCP request
    jmp     buildRequest

request:
    call    parseResponse

    cmp     [dhcpMsgType], byte 0x05	    ; Was the response an ACK? It should be
    jne     apipa			    ; NO - so we do zeroconf

    jmp     close

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
    DEBUGF  1,"Data received, parsing response\n"
    mov     edx, dhcpMsg

    pusha
    eth.set_IP [edx+16]
    mov     eax,[edx]
    mov     [dhcpClientIP],eax
    DEBUGF  1,"Client: %u.%u.%u.%u\n",[edx+16]:1,[edx+17]:1,[edx+18]:1,[edx+19]:1
    popa

    add     edx, 240			    ; Point to first option
    xor     ecx, ecx

next_option:
    add     edx, ecx
pr001:
    mov     al, [edx]
    cmp     al, 0xff			    ; End of options?
    je	    pr_exit

    cmp     al, dhcp_msg_type		    ; Msg type is a single byte option
    jne     @f

    mov     al, [edx+2]
    mov     [dhcpMsgType], al
    add     edx, 3
    jmp     pr001			    ; Get next option

@@:
    inc     edx
    movzx   ecx, byte [edx]
    inc     edx 			    ; point to data

    cmp     al, dhcp_dhcp_server_id	    ; server ip
    jne     @f
    mov     eax, [edx]
    mov     [dhcpServerIP], eax
    DEBUGF  1,"Server: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
    jmp     next_option

@@:
    cmp     al, dhcp_address_time
    jne     @f

    pusha
    mov     eax,[edx]
    bswap   eax
    mov     [dhcpLease],eax
    DEBUGF  1,"lease: %d\n",eax
    popa

    jmp     next_option

@@:
    cmp     al, dhcp_subnet_mask
    jne     @f

    pusha
    eth.set_SUBNET [edx]
    DEBUGF  1,"Subnet: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
    popa

    jmp     next_option

@@:
    cmp     al, dhcp_router
    jne     @f

    pusha
    eth.set_GATEWAY [edx]
    DEBUGF  1,"Gateway: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
    popa

    jmp     next_option


@@:
    cmp     al, dhcp_domain_server
    jne     next_option

    pusha
    eth.set_DNS [edx]
    DEBUGF  1,"DNS: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
    popa

    jmp     next_option

pr_exit:

;    DEBUGF  1,"Sending ARP announce\n"
;    eth.ARP_ANNOUNCE [dhcpClientIP]         ; send an ARP announce packet

    jmp close

apipa:
    call random
    mov  ecx,0xfea9			    ; IP 169.254.0.0 link local net, see RFC3927
    mov  cx,ax
    eth.set_IP ecx			    ; mask is 255.255.0.0
    DEBUGF 1,"Link Local IP assinged: 169.254.%u.%u\n",[generator+2]:1,[generator+3]:1
    eth.set_SUBNET 0xffff
    eth.set_GATEWAY 0x0
    eth.set_DNS 0x0

    wait PROBE_WAIT*100

    xor esi,esi
   probe_loop:
    call  random			    ; create a pseudo random number in eax (seeded by MAC)

    cmp   al,PROBE_MIN*100		    ; check if al is bigger then PROBE_MIN
    jge   @f				    ; all ok
    add   al,(PROBE_MAX-PROBE_MIN)*100	    ; al is too small
   @@:

    cmp   al,PROBE_MAX*100
    jle   @f
    sub   al,(PROBE_MAX-PROBE_MIN)*100
   @@:

    movzx ebx,al
    DEBUGF  1,"Waiting %u0ms\n",ebx
    wait  ebx

    DEBUGF  1,"Sending Probe\n"
;    eth.ARP_PROBE MAC
    inc   esi

    cmp   esi,PROBE_NUM
    jl	  probe_loop

; now we wait further ANNOUNCE_WAIT seconds and send ANNOUNCE_NUM ARP announces. If any other host has assingned
; IP within this time, we should create another adress, that have to be done later

    DEBUGF  1,"Waiting %us\n",ANNOUNCE_WAIT
    wait ANNOUNCE_WAIT*100
    xor   esi,esi
   announce_loop:

    DEBUGF  1,"Sending Announce\n"
;    eth.ARP_ANNOUNCE MAC

    inc   esi
    cmp   esi,ANNOUNCE_NUM
    je	  @f

    DEBUGF  1,"Waiting %us\n",ANNOUNCE_INTERVAL
    wait  ANNOUNCE_INTERVAL*100
    jmp   announce_loop
   @@:
    ; we should, instead of closing, detect ARP conflicts and detect if cable keeps connected ;)

close:
    DEBUGF  1,"Exiting\n"
    exit				    ; at last, exit


random:

    mov   eax,[generator]
    add   eax,-43ab45b5h
    ror   eax,1
    bswap eax
    xor   eax,dword[MAC]
    ror   eax,1
    xor   eax,dword[MAC+2]
    mov   [generator],eax

ret

; DATA AREA

include_debug_strings

IM_END:

dhcpClientIP	dd  0
dhcpMsgType	db  0
dhcpLease	dd  0
dhcpServerIP	dd  0

dhcpMsgLen	dd  0
socketNum	dd  0

MAC		rb  6
currTime	dd  0
renewTime	dd  0
generator	dd  0

dhcpMsg 	rb  BUFFER
I_END: