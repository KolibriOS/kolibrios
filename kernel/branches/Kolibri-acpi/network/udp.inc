;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  UDP.INC                                                        ;;
;;                                                                 ;;
;;  Part of the tcp/ip network stack for KolibriOS                 ;;
;;                                                                 ;;
;;    Written by hidnplayr@kolibrios.org                           ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


struct  UDP_header

        SourcePort              dw  ?
        DestinationPort         dw  ?
        Length                  dw  ?  ; Length of (UDP Header + Data)
        Checksum                dw  ?

ends


uglobal
align 4

        UDP_PACKETS_TX          rd  NET_DEVICES_MAX
        UDP_PACKETS_RX          rd  NET_DEVICES_MAX

endg


;-----------------------------------------------------------------;
;                                                                 ;
; UDP_init: This function resets all UDP variables                ;
;                                                                 ;
;-----------------------------------------------------------------;
macro   UDP_init {

        xor     eax, eax
        mov     edi, UDP_PACKETS_TX
        mov     ecx, 2*NET_DEVICES_MAX
        rep stosd
}


macro   UDP_checksum    IP1, IP2  { ; esi = ptr to udp packet, ecx = packet size, destroys: ecx, edx

; Pseudoheader
        mov     edx, IP_PROTO_UDP

        add     dl, byte[IP1+1]
        adc     dh, byte[IP1+0]
        adc     dl, byte[IP1+3]
        adc     dh, byte[IP1+2]

        adc     dl, byte[IP2+1]
        adc     dh, byte[IP2+0]
        adc     dl, byte[IP2+3]
        adc     dh, byte[IP2+2]

        adc     dl, cl ; byte[esi+UDP_header.Length+1]
        adc     dh, ch ; byte[esi+UDP_header.Length+0]

; Done with pseudoheader, now do real header
        adc     dl, byte[esi+UDP_header.SourcePort+1]
        adc     dh, byte[esi+UDP_header.SourcePort+0]

        adc     dl, byte[esi+UDP_header.DestinationPort+1]
        adc     dh, byte[esi+UDP_header.DestinationPort+0]

        adc     dl, byte[esi+UDP_header.Length+1]
        adc     dh, byte[esi+UDP_header.Length+0]

        adc     edx, 0

; Done with header, now do data
        push    esi
        movzx   ecx, [esi+UDP_header.Length]
        rol     cx , 8
        sub     cx , sizeof.UDP_header
        add     esi, sizeof.UDP_header

        call    checksum_1
        call    checksum_2
        pop     esi

        add     [esi+UDP_header.Checksum], dx   ; this final instruction will set or clear ZF :)

}


;-----------------------------------------------------------------;
;                                                                 ;
; UDP_input: Inject the UDP data in the application sockets.      ;
;                                                                 ;
;   IN: [esp] = ptr to buffer                                     ;
;       ebx = ptr to device struct                                ;
;       ecx = UDP packet size                                     ;
;       edx = ptr to IPv4 header                                  ;
;       esi = ptr to UDP packet data                              ;
;       edi = interface number*4                                  ;
;                                                                 ;
;  OUT: /                                                         ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
UDP_input:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_input: size=%u\n", ecx

        ; First validate, checksum

        neg     [esi + UDP_header.Checksum]     ; substract checksum from 0
        jz      .no_checksum                    ; if checksum is zero, it is considered valid

        ; otherwise, we will re-calculate the checksum and add it to this value, thus creating 0 when it is correct

        mov     eax, edx
        UDP_checksum (eax+IPv4_header.SourceAddress), (eax+IPv4_header.DestinationAddress)
        jnz     .checksum_mismatch

  .no_checksum:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_input: checksum ok\n"

        ; Convert length to little endian

        rol     [esi + UDP_header.Length], 8

        ; Look for a socket where
        ; IP Packet UDP Destination Port = local Port
        ; IP Packet SA = Remote IP

        pusha
        mov     ecx, socket_mutex
        call    mutex_lock
        popa

        mov     cx, [esi + UDP_header.SourcePort]
        mov     dx, [esi + UDP_header.DestinationPort]
        mov     eax, net_sockets
  .next_socket:
        mov     eax, [eax + SOCKET.NextPtr]
        or      eax, eax
        jz      .unlock_dump

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     .next_socket

        cmp     [eax + SOCKET.Protocol], IP_PROTO_UDP
        jne     .next_socket

        cmp     [eax + UDP_SOCKET.LocalPort], dx
        jne     .next_socket

        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_input: socket=%x\n", eax

        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

        ;;; TODO: when packet is processed, check more sockets?!

; FIXME: check remote IP if possible
;
;        cmp     [eax + IP_SOCKET.RemoteIP], 0xffffffff
;        je      @f
;        cmp     [eax + IP_SOCKET.RemoteIP],
;        jne     .next_socket
;       @@:

        cmp     [eax + UDP_SOCKET.RemotePort], 0
        je      .updateport

        cmp     [eax + UDP_SOCKET.RemotePort], cx
        jne     .dump

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        popa

  .updatesock:
        inc     [UDP_PACKETS_RX + edi]

        movzx   ecx, [esi + UDP_header.Length]
        sub     ecx, sizeof.UDP_header
        add     esi, sizeof.UDP_header

        jmp     SOCKET_input

  .updateport:
        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        popa

        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_input: new remote port=%x\n", cx ; FIXME: find a way to print big endian values with debugf
        mov     [eax + UDP_SOCKET.RemotePort], cx
        jmp     .updatesock

  .unlock_dump:
        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_input: no socket found\n"
        jmp     .dump

  .checksum_mismatch:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_input: checksum mismatch\n"

  .dump:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_input: dumping\n"
        call    NET_BUFF_free
        ret



;-----------------------------------------------------------------;
;                                                                 ;
; UDP_output: Create an UDP packet.                               ;
;                                                                 ;
;  IN:  eax = socket pointer                                      ;
;       ecx = number of bytes to send                             ;
;       esi = pointer to data                                     ;
;                                                                 ;
; OUT:  eax = -1 on error                                         ;
;                                                                 ;
;-----------------------------------------------------------------;

align 4
UDP_output:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_output: socket=%x bytes=%u data_ptr=%x\n", eax, ecx, esi

        mov     dx, [eax + UDP_SOCKET.RemotePort]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_output: remote port=%x, ", dx    ; FIXME: find a way to print big endian values with debugf
        rol     edx, 16
        mov     dx, [eax + UDP_SOCKET.LocalPort]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "local port=%x\n", dx

        sub     esp, 4                                          ; Data ptr will be placed here
        push    edx esi
        mov     ebx, [eax + IP_SOCKET.device]
        mov     edx, [eax + IP_SOCKET.LocalIP]
        mov     edi, [eax + IP_SOCKET.RemoteIP]
        mov     al, [eax + IP_SOCKET.ttl]
        mov     ah, IP_PROTO_UDP
        add     ecx, sizeof.UDP_header
        call    IPv4_output
        jz      .fail
        mov     [esp + 8], eax                                  ; pointer to buffer start

        mov     [edi + UDP_header.Length], cx
        rol     [edi + UDP_header.Length], 8

        pop     esi
        push    edi ecx
        sub     ecx, sizeof.UDP_header
        add     edi, sizeof.UDP_header
        shr     ecx, 2
        rep movsd
        mov     ecx, [esp]
        and     ecx, 3
        rep movsb
        pop     ecx edi

        pop     dword [edi + UDP_header.SourcePort]

; Checksum
        mov     esi, edi
        mov     [edi + UDP_header.Checksum], 0
        UDP_checksum (edi-4), (edi-8)                           ; FIXME: IPv4 packet could have options..

        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_output: sending with device %x\n", ebx
        call    [ebx + NET_DEVICE.transmit]
        test    eax, eax
        jnz     @f
        call    NET_ptr_to_num4
        inc     [UDP_PACKETS_TX + edi]
       @@:

        ret

  .fail:
        DEBUGF  DEBUG_NETWORK_ERROR, "UDP_output: failed\n"
        add     esp, 4+4+8
        or      eax, -1
        ret




;-----------------------------------------------------------------;
;                                                                 ;
; UDP_connect                                                     ;
;                                                                 ;
;   IN: eax = socket pointer                                      ;
;                                                                 ;
;  OUT: eax = 0 on success                                        ;
;       eax = -1 on error                                         ;
;       ebx = error code on error                                 ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
UDP_connect:

        test    [eax + SOCKET.state], SS_ISCONNECTED
        jz      @f
        call    UDP_disconnect
  @@:

        push    eax edx
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        pop     edx eax

; Fill in local IP
        cmp     [eax + IP_SOCKET.LocalIP], 0
        jne     @f
        push    [IP_LIST + 4]                                   ; FIXME: use correct local IP
        pop     [eax + IP_SOCKET.LocalIP]

; Fill in remote port and IP, overwriting eventually previous values
        pushw   [edx + 2]
        pop     [eax + UDP_SOCKET.RemotePort]

        pushd   [edx + 4]
        pop     [eax + IP_SOCKET.RemoteIP]

; Find a local port, if user didnt define one
        cmp     [eax + UDP_SOCKET.LocalPort], 0
        jne     @f
        call    SOCKET_find_port
       @@:

        push    eax
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_unlock
        pop     eax

        call    SOCKET_is_connected

        xor     eax, eax
        ret


;-----------------------------------------------------------------;
;                                                                 ;
; UDP_disconnect                                                  ;
;                                                                 ;
;   IN: eax = socket pointer                                      ;
;                                                                 ;
;  OUT: eax = socket pointer                                      ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
UDP_disconnect:

        ; TODO: remove the pending received data

        call    SOCKET_is_disconnected

        ret





;-----------------------------------------------------------------;
;                                                                 ;
; UDP_api: This function is called by system function 76          ;
;                                                                 ;
;  IN: bl = subfunction number in bl                              ;
;      bh = device number in bh                                   ;
;      ecx, edx, .. depends on subfunction                        ;
;                                                                 ;
; OUT: depends on subfunction                                     ;
;                                                                 ;
;-----------------------------------------------------------------;
align 4
UDP_api:

        movzx   eax, bh
        shl     eax, 2

        test    bl, bl
        jz      .packets_tx     ; 0
        dec     bl
        jz      .packets_rx     ; 1

  .error:
        mov     eax, -1
        ret

  .packets_tx:
        mov     eax, [UDP_PACKETS_TX + eax]
        ret

  .packets_rx:
        mov     eax, [UDP_PACKETS_RX + eax]
        ret
