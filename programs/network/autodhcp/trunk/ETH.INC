;
; ETH.INC
;
; made by hidnplayr (hidnplayr@gmail.com) for KolibriOS
;
; The given code before every macro is only a simple example
;
;
; HISTORY
;
; v1.0: august 2006  original release
; v1.1: december 2006 bugfixes and improvements
; v1.2: februari 2007 more bugfixes and improvements

macro mov arg1,arg2 {
    if arg1 eq arg2
    else
    mov arg1,arg2
    end if
}

TCB_LISTEN = 1
TCB_SYN_SENT = 2
TCB_SYN_RECEIVED = 3
TCB_ESTABLISHED = 4
TCB_FIN_WAIT_1 = 5
TCB_FIN_WAIT_2 = 6
TCB_CLOSE_WAIT = 7
TCB_CLOSING = 8
TCB_LAST_ASK = 9
TCB_TIME_WAIT = 10
TCB_CLOSED = 11

PASSIVE = 0
ACTIVE = 1

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

macro eth.get_IP IP {
    mov  ebx,1
    mov  eax,52
    mcall

    mov  IP ,eax
}

macro eth.get_GATEWAY GATEWAY {
    mov  ebx,9
    mov  eax,52
    mcall
    mov GATEWAY ,eax
}

macro eth.get_SUBNET SUBNET {
    mov  ebx,10
    mov  eax,52
    mcall
    mov SUBNET ,eax
}

macro eth.get_DNS DNS {
    mov  ebx,13
    mov  eax,52
    mcall
    mov  DNS ,eax
}

macro eth.set_IP IP {
    mov  ecx,IP
    mov  ebx,3
    mov  eax,52
    mcall
}

macro eth.set_GATEWAY GATEWAY {
    mov  ecx,GATEWAY
    mov  ebx,11
    mov  eax,52
    mcall
}

macro eth.set_SUBNET SUBNET {
    mov  ecx,SUBNET
    mov  ebx,12
    mov  eax,52
    mcall
}

macro eth.set_DNS DNS {
    mov  ecx,DNS
    mov  ebx,14
    mov  eax,52
    mcall
}

macro eth.set_network_drv conf {
    mov  ecx,conf
    mov  ebx,2
    mov  eax,52
    mcall
}

macro eth.open_udp local,remote,ip,socket {
    mov  ecx, local
    mov  edx, remote
    mov  esi, ip
    mov  ebx, 0
    mov  eax, 53
    mcall

    mov  socket,eax
}

macro eth.close_udp socket {
    mov  ecx, socket
    mov  ebx, 1
    mov  eax, 53
    mcall
}

macro eth.poll socket {
    mov  ecx, socket
    mov  ebx, 2
    mov  eax, 53
    mcall
}

macro eth.read_byte socket, result {
    mov  ecx, socket
    mov  ebx, 3
    mov  eax, 53
    mcall

    mov  result,bl
}

macro eth.read_packet socket, result, buffersize {
    mov  esi, buffersize
    mov  edx, result
    mov  ecx, socket
    mov  ebx, 11
    mov  eax, 53
    mcall
}

macro eth.write_udp socket,length,msg,verify {
    mov  ecx, socket
    mov  edx, length
    mov  esi, msg
    mov  ebx, 4
    mov  eax, 53
    mcall

    if verify eq 1
    call verifysend
    end if

}

verifysend:
    test eax,eax
    jnz  @f
    ret
@@:
    pusha
    mov  eax,5
    mov  ebx,100
    mcall
    popa
    mcall
ret

macro eth.open_tcp local,remote,ip,passive,socket {

    mov  ecx, local
    mov  edx, remote
    mov  esi, ip
    mov  edi, passive	   ; 0 = PASSIVE open
    mov  ebx, 5
    mov  eax, 53
    mcall

    mov  socket,eax
}

macro eth.socket_status socket,result {
    mov  ecx, socket
    mov  ebx, 6
    mov  eax, 53
    mcall

    mov  result,eax
}

macro eth.write_tcp socket,length,msg,verify {
    mov  ecx, socket
    mov  edx, length
    mov  esi, msg
    mov  ebx, 7
    mov  eax, 53
    mcall

    if verify eq 1
    call verifysend
    end if
}

macro eth.read_mac mac {
    mov  eax, 52
    mov  ebx, 15
    xor  ecx, ecx
    pusha
    mcall
    mov  dword[mac],eax
    popa
    add  cl, 4
    mcall
    mov  word[mac+4],ax

}

macro eth.close_tcp socket {
    mov  ecx, socket
    mov  ebx, 8
    mov  eax, 53
    mcall
}

macro eth.check_port port,result {
    mov  ecx, port
    mov  ebx, 9
    mov  eax, 53
    mcall

    mov  result,eax
}

macro eth.check_cable result {
    mov  ebx, 10
    mov  eax, 53
    mcall

    mov  result,eax
}

macro eth.status status {
    mov  ebx, 255
    mov  ecx, 6
    mov  eax, 53
    mcall

    mov  status,eax
}

macro eth.search_port port,result {
    mov  edx,port
   @@:
    inc  edx
    eth.check_port edx,eax
    cmp  eax,0
    je	 @r
    mov  result,edx
}

macro eth.ARP_PROBE address{

    mov  edx,address
    mov  eax,52
    mov  ebx,16
    xor  ecx,ecx
    mcall

}


macro eth.ARP_ANNOUNCE address{

    mov  edx,address
    mov  eax,52
    mov  ebx,16
    xor  ecx,ecx
    inc  ecx
    mcall

}

macro eth.read_data socket,dest,endptr,bufferl {
local .getdata,.loop,.end
    mov     eax, dest
    mov     endptr, eax

.getdata:
    cmp     endptr, bufferl
    jg	    .end

    eth.read_packet socket, endptr, 0
    add     endptr,eax

    test    eax, eax
    jnz     .getdata

    xor     edx, edx
.loop:
    eth.poll socket

    test    eax, eax
    jnz     .getdata

    mov     eax,5
    mov     ebx,1
    mcall

    inc     edx
    cmp     edx,30
    jl	    .loop

.end:
}

macro eth.wait_for_data socket,TIMEOUT,abort {
    mov   edx,TIMEOUT

   @@:
    eth.poll socket

    cmp   eax,0
    jne   @f

    dec   edx
    jz	  abort

    mov   eax,5 			      ; wait here for event
    mov   ebx,10
    mcall

    jmp   @r
   @@:

}


; The function 'resolve' resolves the address in edx and puts the resulting IP in eax.
; When the input is an IP-adress, the function will output this IP in eax.
; If something goes wrong, the result in eax should be 0
;
; example:
;
; resolve query1,IP,PORT
; resolve '192.168.0.1',IP,PORT
; resolve query2,IP,PORT
;
; query1 db 'www.google.com',0
; query2 db '49.78.84.45',0
; IP     dd ?
; PORT   dd ?

macro resolve query,result {

if    query eqtype 0
 mov   edx,query
else
 local ..string, ..label
 jmp   ..label
 ..string db query,0
 ..label:
 mov   edx,..string
end   if

call  __resolve

mov   result,eax

}

if used __resolve

__resolve:

if __DEBUG__ eq 1
DEBUGF 1,'DNS: Resolving started\n'
end if

    ; This code validates if the query is an IP containing 4 numbers and 3 dots


    push    edx 	      ; push edx (query address) onto stack
    xor     al, al	      ; make al (dot count) zero

   @@:
    cmp     byte[edx],'0'     ; check if this byte is a number, if not jump to no_IP
    jl	    no_IP	      ;
    cmp     byte[edx],'9'     ;
    jg	    no_IP	      ;

    inc     edx 	      ; the byte was a number, so lets check the next byte

    cmp     byte[edx],0       ; is this byte zero? (have we reached end of query?)
    jz	    @f		      ; jump to next @@ then
    cmp     byte[edx],':'
    jz	    @f

    cmp     byte[edx],'.'     ; is this byte a dot?
    jne     @r		      ; if not, jump to previous @@

    inc     al		      ; the byte was a dot so increment al(dot count)
    inc     edx 	      ; next byte
    jmp     @r		      ; lets check for numbers again (jump to previous @@)

   @@:			      ; we reach this when end of query reached
    cmp     al,3	      ; check if there where 3 dots
    jnz     no_IP	      ; if not, jump to no_IP (this is where the DNS will take over)

    ; The following code will convert this IP into a dword and output it in eax
    ; If there is also a port number specified, this will be returned in ebx, otherwise ebx is -1

    pop     esi 	      ; edx (query address) was pushed onto stack and is now popped in esi

    xor     edx, edx	      ; result
    xor     eax, eax	      ; current character
    xor     ebx, ebx	      ; current byte

.outer_loop:
    shl     edx, 8
    add     edx, ebx
    xor     ebx, ebx
.inner_loop:
    lodsb
    test    eax, eax
    jz	    .finish
    cmp     al, '.'
    jz	    .outer_loop
    sub     eax, '0'
    imul    ebx, 10
    add     ebx, eax
    jmp     .inner_loop
.finish:
    shl     edx, 8
    add     edx, ebx

    bswap   edx 	      ; we want little endian order
    mov     eax, edx

    ret


no_IP:

    pop     edx

    ; The query is not an IP address, we will send the query to a DNS server and hope for answer ;)
if __DEBUG__ eq 1
    DEBUGF 1,'DNS: The query is no ip, Building request string from:%u\n',edx
end if

    ; Build the request string
    mov     eax, 0x00010100
    mov     [dnsMsg], eax
    mov     eax, 0x00000100
    mov     [dnsMsg+4], eax
    mov     eax, 0x00000000
    mov     [dnsMsg+8], eax

    ; domain name goes in at dnsMsg+12
    mov     esi, dnsMsg + 12			 ; location of label length
    mov     edi, dnsMsg + 13			 ; label start
    mov     ecx, 12				 ; total string length so far

td002:
    mov     [esi], byte 0
    inc     ecx

td0021:
    mov     al, [edx]

    cmp     al, 0
    je	    td001				 ; we have finished the string translation

    cmp     al, '.'
    je	    td004				 ; we have finished the label

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

    mov     [dnsMsgLen], ecx			 ; We'll need the length of the message when we send it
    ; Now, lets send this and wait for an answer

    eth.search_port 1024,edx			 ; Find a free port starting from 1025 and store in edx
    eth.get_DNS esi				 ; Read DNS IP from stack into esi
    eth.open_udp edx,53,esi,[socketNum] 	     ; First, open socket
if __DEBUG__ eq 1
    DEBUGF 1,'DNS: Socket opened: %u (port %u)\n',[socketNum],ecx
end if
    eth.write_udp [socketNum],[dnsMsgLen],dnsMsg     ; Write to socket ( request DNS lookup )
if __DEBUG__ eq 1
    DEBUGF 1,'DNS: Data written, length:%u offset:%u\n',[dnsMsgLen],dnsMsg
    DEBUGF 1,'DNS: Waiting for data: (timeout is %us)\n',TIMEOUT
end if
    eth.wait_for_data [socketNum],TIMEOUT,abort  ; Now, we wait for data from remote
    eth.read_data dword[socketNum],dnsMsg,dword[dnsMsgLen],dnsMsg+BUFFER      ; Read the data into the buffer
if __DEBUG__ eq 1
    DEBUGF 1,'Data received, offset:%u buffer size:%u length:%u\n',dnsMsg,BUFFER,esi-dnsMsg
end if
    eth.close_udp [socketNum]			     ; We're done, close the socket
if __DEBUG__ eq 1
    DEBUGF 1,'Closed Socket\n'
end if

    ; Now parse the message to get the host IP. Man, this is complicated. It's described in RFC 1035
    ; 1) Validate that we have an answer with > 0 responses
    ; 2) Find the answer record with TYPE 0001 ( host IP )
    ; 3) Finally, copy the IP address to the display
    ; Note: The response is in dnsMsg, the end of the buffer is pointed to by [dnsMsgLen]

    mov     esi, dnsMsg

    mov     al, [esi+2] 			 ; Is this a response to my question?
    and     al, 0x80
    cmp     al, 0x80
    jne     abort
if __DEBUG__ eq 1
    DEBUGF 1,'DNS: It was a response to my question\n'
end if

    mov     al, [esi+3] 			 ; Were there any errors?
    and     al, 0x0F
    cmp     al, 0x00
    jne     abort

if __DEBUG__ eq 1
    DEBUGF 1,'DNS: There were no errors\n'
end if

    mov     ax, [esi+6] 			 ; Is there ( at least 1 ) answer?
    cmp     ax, 0x00
    je	    abort

    ; Header validated. Scan through and get my answer
    add     esi, 12				 ; Skip to the question field
    call    skipName				 ; Skip through the question field
    add     esi, 4				 ; skip past the questions qtype, qclass

ctr002z:
    ; Now at the answer. There may be several answers, find the right one ( TYPE = 0x0001 )
    call    skipName
    mov     ax, [esi]
    cmp     ax, 0x0100				 ; Is this the IP address answer?
    jne     ctr002c
    add     esi, 10				 ; Yes! Point eax to the first byte of the IP address
    mov     eax,[esi]

    ret


ctr002c:					 ; Skip through the answer, move to the next
    add     esi, 8
    movzx   eax, byte [esi+1]
    mov     ah, [esi]
    add     esi, eax
    add     esi, 2

    cmp     esi, [dnsMsgLen]			 ; Have we reached the end of the msg? This is an error condition, should not happen
    jl	    ctr002z				 ; Check next answer

abort:
if __DEBUG__ eq 1
    DEBUGF 1,'DNS: Something went wrong, aborting\n'
end if
    xor     eax,eax

    ret


skipName:
    ; Increment esi to the first byte past the name field
    ; Names may use compressed labels. Normally do.
    ; RFC 1035 page 30 gives details
    mov     al, [esi]
    cmp     al, 0
    je	    sn_exit
    and     al, 0xc0
    cmp     al, 0xc0
    je	    sn001

    movzx   eax, byte [esi]
    inc     eax
    add     esi, eax
    jmp     skipName

sn001:
    add     esi, 2				 ; A pointer is always at the end
    ret

sn_exit:
    inc     esi
    ret

dnsMsgLen:	dd 0
socketNum:	dd 0xFFFF

if ~defined dnsMsg
dnsMsg: 	rb BUFFER
end if

end if




