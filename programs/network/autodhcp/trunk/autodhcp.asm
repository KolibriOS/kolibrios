; Automated dhcp client
; v 1.3
;
; with thanks to authors of DHCP client for menuetos: Mike Hibbet
;
; by HidnPlayr & Derpenguin


TIMEOUT equ 60 ; in seconds
BUFFER	equ 1024
__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 1; 1 = all, 2 = errors

use32
	       org    0x0

	       db     'MENUET01'	      ; 8 byte id
	       dd     0x01		      ; header version
	       dd     START		      ; start of code
	       dd     IM_END		      ; size of image
	       dd     I_END		      ; memory for app
	       dd     I_END		      ; esp
	       dd     0x0 , 0x0 	      ; I_Param , I_Icon

;include 'macros.inc'
include 'eth.inc'
include 'debug-fdo.inc'


 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; CONFIGURATION FOR LINK-LOCAL                                                         ;
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
				       ;                                               ;
PROBE_WAIT	    equ 1	       ; second  (initial random delay)                ;
PROBE_MIN	    equ 1	       ; second  (minimum delay till repeated probe)   ;
PROBE_MAX	    equ 2	       ; seconds (maximum delay till repeated probe)   ;
PROBE_NUM	    equ 3	       ;         (number of probe packets)             ;
				       ;                                               ;
ANNOUNCE_NUM	    equ 2	       ;         (number of announcement packets)      ;
ANNOUNCE_INTERVAL   equ 2	       ; seconds (time between announcement packets)   ;
ANNOUNCE_WAIT	    equ 2	       ; seconds (delay before announcing)             ;
				       ;                                               ;
MAX_CONFLICTS	    equ 10	       ;         (max conflicts before rate limiting)  ;
				       ;                                               ;
RATE_LIMIT_INTERVAL equ 60	       ; seconds (delay between successive attempts)   ;
				       ;                                               ;
DEFEND_INTERVAL     equ 10	       ; seconds (min. wait between defensive ARPs)    ;
				       ;                                               ;
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


START:				       ; start of execution

    mov     eax,40		       ; Report events
    mov     ebx,10000000b	       ; Only Stack
    int     0x40

    mov     eax,52		       ; first, enable the stack (packet driver)
    mov     ebx,2
    mov     ecx,0x00000383
    int     0x40

    DEBUGF  1,"DHCP: Stack Initialized.\n"

    eth.status eax		       ; Read the Stack status
    test    eax,eax		       ; if eax is zero, no driver was found
    jnz     @f
    DEBUGF  1,"DHCP: No Card detected\n"
    jmp     close

   @@:
    DEBUGF  1,"DHCP: Detected card: %x\n",eax
   @@:
    eth.check_cable eax
    test    al,al
    jnz     @f
    DEBUGF  1,"DHCP: Ethernet Cable not connected\n"

    mov     eax,5
    mov     ebx,500		       ; loop until cable is connected (check every 5 sec)
    int     0x40

    jmp     @r

   @@:
    DEBUGF  1,"DHCP: Ethernet Cable status: %d\n",al

    eth.read_mac MAC
    DEBUGF  1,"DHCP: MAC address: %x-%x-%x-%x-%x-%x\n",[MAC]:2,[MAC+1]:2,[MAC+2]:2,[MAC+3]:2,[MAC+4]:2,[MAC+5]:2

;    jmp     apipa   ; comment this out if you want to skip DHCP and continue with link-local

;***************************************************************************
;
;  DHCP rubish starts here
;
;***************************************************************************



    eth.check_port 68,eax	       ; Check if port 68 is available
    cmp     eax,1
    je	    @f

    DEBUGF  1,"DHCP: Port 68 is already in use.\n"
    jmp     close

   @@:
    eth.open_udp 68,67,-1,[socketNum]  ; open socket (local,remote,ip,socket)
    DEBUGF  1,"DHCP: Socket opened: %d\n",eax
				       ; Setup the first msg we will send
    mov     byte [dhcpMsgType], 0x01   ; DHCP discover
    mov     dword [dhcpLease], esi     ; esi is still -1 (-1 = forever)

    mov     eax,26
    mov     ebx,9
    int     0x40
    imul    eax,100
    mov     [currTime],eax

;***************************************************************************
;   Function
;      buildRequest
;
;   Description
;      Creates a DHCP request packet.
;
;***************************************************************************
buildRequest:
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
    jne     br001

    mov     [edx+240+21], byte 0xff	    ; "Discover" options

    mov     [dhcpMsgLen], dword 262	    ; end of options marker
    jmp     ctr000

br001:					    ; "Request" options

    mov     [edx+240+21], word 0x0436	    ; server IP
    mov     eax, [dhcpServerIP]
    mov     [edx+240+23], eax

    mov     [edx+240+27], byte 0xff	    ; end of options marker

    mov     [dhcpMsgLen], dword 268

ctr000:

    eth.write_udp [socketNum],[dhcpMsgLen],dhcpMsg    ; write to socket ( send broadcast request )

    mov     eax, dhcpMsg		    ; Setup the DHCP buffer to receive response
    mov     [dhcpMsgLen], eax		    ; Used as a pointer to the data

    mov     eax,23			    ; wait here for event (data from remote)
    mov     ebx,TIMEOUT*10
    int     0x40

    eth.poll [socketNum]

    test    eax,eax
    jnz     ctr002

    DEBUGF  2,"DHCP: Timeout!\n"
    eth.close_udp [socketNum]
    jmp    apipa			    ; no server found, lets try zeroconf


ctr002: 				    ; we have data - this will be the response
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
    call     parseResponse

    cmp     [dhcpMsgType], byte 0x05	    ; Was the response an ACK? It should be
    jne     apipa			    ; NO - so we do zeroconf

close:
    DEBUGF  1,"DHCP: Exiting\n"

    mov     eax,-1			    ; at last, exit
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
    DEBUGF  1,"DHCP: Data received, parsing response\n"
    mov     edx, dhcpMsg

    pusha
    eth.set_IP [edx+16]
    mov     eax,[edx]
    mov     [dhcpClientIP],eax
    DEBUGF  1,"DHCP: Client: %u.%u.%u.%u\n",[edx+16]:1,[edx+17]:1,[edx+18]:1,[edx+19]:1
    popa

    add     edx, 240	    ; Point to first option

pr001:
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
    inc     edx
    movzx   ecx, byte [edx]
    inc     edx 	    ; point to data

    cmp     al, 54	    ; server id
    jne     pr0021
    mov     eax, [edx]	    ; All options are 4 bytes, so get it
    mov     [dhcpServerIP], eax
    DEBUGF  1,"DHCP: Server: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
    jmp     pr003

pr0021:
    cmp     al, 51	    ; lease
    jne     pr0022

    pusha
    DEBUGF  1,"DHCP: lease: "
    mov eax,[edx]
    bswap eax
    mov  [dhcpLease],eax
    cmp  dword[edx],-1	    ; i really don't know, how to test it
    jne  no_lease_forever
    DEBUGF  1,"forever\n"
    jmp  @f
   no_lease_forever:
    DEBUGF  1,"%d\n",eax
   @@:
    popa

    jmp     pr003

pr0022:
    cmp     al, 1	    ; subnet mask
    jne     pr0023

    pusha
    eth.set_SUBNET [edx]
    DEBUGF  1,"DHCP: Subnet: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
    popa

    jmp     pr003

pr0023:
    cmp     al, 3	    ; gateway ip
    jne     pr0024

    pusha
    eth.set_GATEWAY [edx]
    DEBUGF  1,"DHCP: Gateway: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
    popa


pr0024:
    cmp     al, 6	    ; dns ip
    jne     pr003

    pusha
    eth.set_DNS [edx]
    DEBUGF  1,"DHCP: DNS: %u.%u.%u.%u\n",[edx]:1,[edx+1]:1,[edx+2]:1,[edx+3]:1
    popa


pr003:
    add     edx, ecx
    jmp     pr001

pr_exit:

;    DEBUGF  1,"DHCP: Sending ARP probe\n"
;    eth.ARP_ANNOUNCE [dhcpClientIP]      ; send an ARP announc packet

    eth.get_GATEWAY eax 		  ; if gateway was not set, set it to the DHCP SERVER IP
    test  eax,eax
    jnz   close
    eth.set_GATEWAY [dhcpServerIP]
    jmp close

apipa:
    call random
    mov  ecx,0xfea9			 ; IP 169.254.0.0 link local net, see RFC3927
    mov  cx,ax
    eth.set_IP ecx			 ; mask is 255.255.0.0
    DEBUGF 1,"ZeroConf: Link Local IP assinged: 169.254.%u.%u\n",[generator+2]:1,[generator+3]:1
    eth.set_SUBNET 0xffff
    eth.set_GATEWAY 0x0
    eth.set_DNS 0x0

    mov   eax,5
    mov   ebx,PROBE_WAIT*100
    int   0x40

    xor esi,esi
   probe_loop:
    call  random		       ; create a pseudo random number in eax (seeded by MAC)

    cmp   al,PROBE_MIN*100	       ; check if al is bigger then PROBE_MIN
    jge   @f			       ; all ok
    add   al,(PROBE_MAX-PROBE_MIN)*100 ; al is too small
   @@:

    cmp   al,PROBE_MAX*100
    jle   @f
    sub   al,(PROBE_MAX-PROBE_MIN)*100
   @@:

    movzx ebx,al
    DEBUGF  1,"ZeroConf: Waiting %u0ms\n",ebx
    mov   eax,5
    int   0x40

    DEBUGF  1,"ZeroConf: Sending Probe\n"
;    eth.ARP_PROBE MAC2
    inc   esi

    cmp   esi,PROBE_NUM
    jl	  probe_loop

; now we wait further ANNOUNCE_WAIT seconds and send ANNOUNCE_NUM ARP announces. If any other host has assingnd
; IP within this time, we should create another adress, that have to be done later

    DEBUGF  1,"ZeroConf: Waiting %us\n",ANNOUNCE_WAIT
    mov   eax,5
    mov   ebx,ANNOUNCE_WAIT*100
    int   0x40

    xor   esi,esi
   announce_loop:

    DEBUGF  1,"ZeroConf: Sending Announce\n"
;    eth.ARP_ANNOUNCE MAC2

    inc   esi
    cmp   esi,ANNOUNCE_NUM
    je	  @f

    DEBUGF  1,"ZeroConf: Waiting %us\n",ANNOUNCE_INTERVAL
    mov   eax,5
    mov   ebx,ANNOUNCE_INTERVAL*100
    int   0x40

    jmp   announce_loop
   @@:
    jmp   close   ; we should, instead of closing, detect ARP conflicts and detect if cable keeps connected ;)

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

include_debug_strings ; ALWAYS present in data section

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