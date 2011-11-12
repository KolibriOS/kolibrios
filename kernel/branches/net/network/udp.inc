;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2010. All rights reserved.    ;;
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


align 4
uglobal
        UDP_PACKETS_TX          rd  MAX_IP
        UDP_PACKETS_RX          rd  MAX_IP
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
        mov     ecx, 2*MAX_IP
        rep     stosd
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

        DEBUGF  1,"UDP_input, size:%u\n", ecx

; First validate, checksum

        neg     [esi + UDP_header.Checksum]     ; substract checksum from 0
        jz      .no_checksum                    ; if checksum is zero, it is considered valid and we continue processing
                                                ; otherwise, we will re-calculate the checksum and add it to this value, thus creating 0 when it is correct

        push    esi
        UDP_checksum (edi), (edi+4)
        pop     edx
        jnz     .checksum_mismatch

  .no_checksum:
        DEBUGF  1,"UDP Checksum is correct\n"

        ; Convert port numbers to intel format

        rol     [edx + UDP_header.DestinationPort], 8
        rol     [edx + UDP_header.SourcePort], 8
        rol     [edx + UDP_header.Length], 8

        ; Look for a socket where
        ; IP Packet UDP Destination Port = local Port
        ; IP Packet SA = Remote IP

        mov     si, [edx + UDP_header.DestinationPort]
        mov     cx, [edx + UDP_header.SourcePort]
        mov     edi, [edi + 4]                          ; ipv4 source address
        mov     eax, net_sockets

  .next_socket:
        mov     eax, [eax + SOCKET.NextPtr]
        or      eax, eax
        jz      .dump

        cmp     [eax + SOCKET.Domain], AF_INET4
        jne     .next_socket

        cmp     [eax + SOCKET.Protocol], IP_PROTO_UDP
        jne     .next_socket

        cmp     [eax + UDP_SOCKET.LocalPort], si
        jne     .next_socket

        DEBUGF  1,"using socket: %x\n", eax

        ;;; TODO: when packet is processed, check more sockets!

        cmp     [eax + IP_SOCKET.RemoteIP], 0xffffffff
        je      @f
        cmp     [eax + IP_SOCKET.RemoteIP], edi
        jne     .next_socket
       @@:

        cmp     [eax + UDP_SOCKET.firstpacket], 0
        je      .updateport

        cmp     [eax + UDP_SOCKET.RemotePort], cx
        jne     .dump

        push    ebx
        lea     ebx, [eax + SOCKET.lock]
        call    wait_mutex
        pop     ebx

  .updatesock:
        inc     [UDP_PACKETS_RX]
        DEBUGF  1,"Found valid UDP packet for socket %x\n", eax
        lea     esi, [edx + sizeof.UDP_header]
        movzx   ecx, [edx + UDP_header.Length]
        sub     ecx, sizeof.UDP_header

        jmp     SOCKET_input

  .updateport:
        push    ebx
        lea     ebx, [eax + SOCKET.lock]
        call    wait_mutex
        pop     ebx

        DEBUGF  1,"Changing remote port to: %u\n", cx
        mov     [eax + UDP_SOCKET.RemotePort], cx
        inc     [eax + UDP_SOCKET.firstpacket]

        jmp     .updatesock


  .checksum_mismatch:

        DEBUGF  2,"UDP_Handler - checksum mismatch\n"

  .dump:
        call    kernel_free
        add     esp, 4 ; pop (balance stack)
        DEBUGF  2,"UDP_Handler - dumping\n"

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

        DEBUGF  1,"UDP_output: socket:%x, bytes: %u, data ptr: %x\n", eax, ecx, esi

        mov     dx, [eax + UDP_SOCKET.RemotePort]
        DEBUGF  1,"remote port: %u\n", dx
        rol     dx, 8
        rol     edx, 16
        mov     dx, [eax + UDP_SOCKET.LocalPort]
        DEBUGF  1,"local port: %u\n", dx
        rol     dx, 8

        mov     ebx, [eax + IP_SOCKET.LocalIP]
        mov     eax, [eax + IP_SOCKET.RemoteIP]

        mov     di, IP_PROTO_UDP shl 8 + 128
        sub     esp, 8                                          ; Data ptr and data size will be placed here
        add     ecx, sizeof.UDP_header

;;; TODO: fragment id
        push    edx esi
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
        rep     movsd
        mov     ecx, [esp]
        and     ecx, 3
        rep     movsb
        pop     ecx edi

        pop     dword [edi + UDP_header.SourcePort]

; Checksum
        mov     esi, edi
        mov     [edi + UDP_header.Checksum], 0
        UDP_checksum (edi-4), (edi-8)                           ; TODO: fix this, IPv4 packet could have options..

        inc     [UDP_PACKETS_TX]

        DEBUGF  1,"Sending UDP Packet to device %x\n", ebx

        call    [ebx + NET_DEVICE.transmit]
        ret

  .fail:
        DEBUGF  1,"UDP_output: failed\n"
        add     esp, 4+4+8
        xor     eax, eax
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
UDP_API:

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
