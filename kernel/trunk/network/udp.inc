;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved.    ;;
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

$Revision: 2995 $


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


;-----------------------------------------------------------------
;
; UDP_init
;
;  This function resets all UDP variables
;
;-----------------------------------------------------------------
macro   UDP_init {

        xor     eax, eax
        mov     edi, UDP_PACKETS_TX
        mov     ecx, 2*NET_DEVICES_MAX
        rep stosd
}


macro   UDP_checksum    IP1, IP2  { ; esi = ptr to udp packet, ecx = packet size, destroys: ecx, edx

; Pseudoheader
        mov     edx, IP_PROTO_UDP

        add     dl, [IP1+1]
        adc     dh, [IP1+0]
        adc     dl, [IP1+3]
        adc     dh, [IP1+2]

        adc     dl, [IP2+1]
        adc     dh, [IP2+0]
        adc     dl, [IP2+3]
        adc     dh, [IP2+2]

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


;-----------------------------------------------------------------
;
; UDP_input:
;
;  Called by IPv4_input,
;  this procedure will inject the udp data diagrams in the application sockets.
;
;  IN:   [esp]  = Pointer to buffer
;       [esp+4] = size of buffer
;       ebx = ptr to device struct
;       ecx = UDP Packet size
;       esi = ptr to UDP header
;       edi = ptr to ipv4 source and dest address
;
;  OUT: /
;
;-----------------------------------------------------------------
align 4
UDP_input:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_input: size=%u\n", ecx

        ; First validate, checksum

        neg     [esi + UDP_header.Checksum]     ; substract checksum from 0
        jz      .no_checksum                    ; if checksum is zero, it is considered valid

        ; otherwise, we will re-calculate the checksum and add it to this value, thus creating 0 when it is correct

        UDP_checksum (edi), (edi+4)
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
        mov     edi, [edi + 4]                          ; ipv4 source address
        mov     eax, net_sockets

  .next_socket:
        mov     eax, [eax + SOCKET.NextPtr]
        or      eax, eax
        jz      .dump_

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

        ;;; TODO: when packet is processed, check more sockets!

;        cmp     [eax + IP_SOCKET.RemoteIP], 0xffffffff
;        je      @f
;        cmp     [eax + IP_SOCKET.RemoteIP], edi
;        jne     .next_socket
;       @@:
;
; FIXME: UDP should check remote IP, but not under all circumstances!

        cmp     [eax + UDP_SOCKET.firstpacket], 0
        je      .updateport

        cmp     [eax + UDP_SOCKET.RemotePort], cx
        jne     .dump

        pusha
        lea     ecx, [eax + SOCKET.mutex]
        call    mutex_lock
        popa

  .updatesock:
        call    NET_ptr_to_num4
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
        inc     [eax + UDP_SOCKET.firstpacket]

        jmp     .updatesock

  .dump_:

        pusha
        mov     ecx, socket_mutex
        call    mutex_unlock
        popa

        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_input: no socket found\n"

        jmp     .dump

  .checksum_mismatch:
        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_input: checksum mismatch\n"

  .dump:
        call    kernel_free
        add     esp, 4 ; pop (balance stack)
        DEBUGF  DEBUG_NETWORK_VERBOSE,"UDP_input: dumping\n"

        ret




;-----------------------------------------------------------------
;
; UDP_output
;
; IN: eax = socket pointer
;     ecx = number of bytes to send
;     esi = pointer to data
;
;-----------------------------------------------------------------

align 4
UDP_output:

        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_output: socket=%x bytes=%u data_ptr=%x\n", eax, ecx, esi

        mov     dx, [eax + UDP_SOCKET.RemotePort]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "UDP_output: remote port=%x, ", dx    ; FIXME: find a way to print big endian values with debugf
        rol     edx, 16
        mov     dx, [eax + UDP_SOCKET.LocalPort]
        DEBUGF  DEBUG_NETWORK_VERBOSE, "local port=%x\n", dx

        sub     esp, 8                                          ; Data ptr and data size will be placed here
        push    edx esi
        mov     ebx, [eax + SOCKET.device]
        mov     edx, [eax + IP_SOCKET.LocalIP]
        mov     eax, [eax + IP_SOCKET.RemoteIP]
        mov     di, IP_PROTO_UDP shl 8 + 128
        add     ecx, sizeof.UDP_header
        call    IPv4_output
        jz      .fail
        mov     [esp + 8], eax                                  ; pointer to buffer start
        mov     [esp + 8 + 4], edx                              ; buffer size

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



;---------------------------------------------------------------------------
;
; UDP_API
;
; This function is called by system function 75
;
; IN:  subfunction number in bl
;      device number in bh
;      ecx, edx, .. depends on subfunction
;
; OUT:
;
;---------------------------------------------------------------------------

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
