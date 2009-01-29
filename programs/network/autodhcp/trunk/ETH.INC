;
; ETH.INC
;
; made by hidnplayr (hidnplayr@kolibrios.org) for KolibriOS
;
; The given code before every macro is only a simple example
;
;
; HISTORY
;
; v1.0: august 2006  original release
; v1.1: december 2006 bugfixes and improvements
; v1.2: february 2007 more bugfixes and improvements

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



Ip2dword:
    push    edx

    ; This code validates if the query is an IP containing 4 numbers and 3 dots

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
    jnz     no_IP	      ; if not, jump to no_IP

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

    ret

no_IP:
    pop     edx
    xor     edx, edx

    ret




