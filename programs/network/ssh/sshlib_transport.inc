;    sshlib_transport.inc - SSH transport layer
;
;    Copyright (C) 2016-2024 Jeffrey Amelynck
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.


align 16
sshlib_recv_packet:     ; con_ptr, flags

        mov     eax, [esp+4]
        jmp     [eax+sshlib_connection.rx_proc]

align 16
sshlib_send_packet:     ; con_ptr, buf, payload_size, flags

        mov     eax, [esp+4]
        jmp     [eax+sshlib_connection.tx_proc]


align 16
proc sshlib_recv_packet_clear con_ptr, flags

locals
        data_length     dd ?    ; Total length of packet without MAC
endl

        DEBUGF  3, "> "
; Receive first block (Read length, padding length, message code)
        mov     ebx, [con_ptr]
        mov     ecx, [ebx + sshlib_connection.socketnum]
        mov     esi, 4
        lea     edx, [ebx + sshlib_connection.rx_buffer]
        mov     edi, [flags]
        mcall   recv
        cmp     eax, 0
        jle     .sock_fail
        sub     [ssh_chan.rcv_wnd], eax  ;;; FIXME
        DEBUGF  1, "chunk = %u ", eax
        mov     ebx, [con_ptr]
        cmp     eax, 4
        jne     .proto_fail     ; TODO: handle receives of 1, 2, and 3 bytes correctly

; Check data length
        mov     esi, [ebx + sshlib_connection.rx_buffer.packet_length]
        bswap   esi                                             ; convert length to little endian
        mov     [ebx + sshlib_connection.rx_buffer.packet_length], esi
        DEBUGF  1, "packet length=%u ", esi
        cmp     esi, BUFFERSIZE
        ja      .proto_fail                                     ; packet is too large
        test    ecx, ecx
        jz      .proto_fail

; Receive remaining data
        lea     edx, [ebx + sshlib_connection.rx_buffer]
        add     edx, 4
        mov     ecx, [ebx + sshlib_connection.socketnum]
        mov     edi, [flags]
  .receive_loop:
        DEBUGF  3, "want %d bytes.. ", esi
        mcall   recv
        cmp     eax, 0
        jle     .sock_fail
        sub     [ssh_chan.rcv_wnd], eax             ;;; FIXME
        DEBUGF  3, "got %d bytes\n", eax
        add     edx, eax
        sub     esi, eax
        jnz     .receive_loop
  .packet_complete:

; Return useful data length to the caller via eax register
        mov     ebx, [con_ptr]
        mov     eax, [ebx + sshlib_connection.rx_buffer.packet_length]
        movzx   ebx, [ebx + sshlib_connection.rx_buffer.padding_length]
        sub     eax, ebx

; Update sequence counter
        mov     ebx, [con_ptr]
        add     byte[ebx + sshlib_connection.rx_mac_seqnr+3], 1
        adc     byte[ebx + sshlib_connection.rx_mac_seqnr+2], 0
        adc     byte[ebx + sshlib_connection.rx_mac_seqnr+1], 0
        adc     byte[ebx + sshlib_connection.rx_mac_seqnr+0], 0

        DEBUGF  1, "useful data length=%u\n", eax
        ret

  .sock_fail:
        DEBUGF  3, "ssh_recv_packet failed!\n"
        mov     eax, SSHLIB_ERR_SOCKET
        ret

  .proto_fail:
        DEBUGF  3, "ssh_recv_packet protocol failure!\n"
        mov     eax, SSHLIB_ERR_PROTOCOL
        xor     ebx, ebx
        ret

endp


align 16
proc sshlib_send_packet_clear con_ptr, buf, payload_size, flags

locals
        packet_size    dd ?
endl
        DEBUGF  2, "< "

; Check how many bytes we should pad
        mov     eax, [payload_size]
        inc     eax                     ; padding length byte
        lea     edx, [eax+4]            ; total packet size (without padding and MAC)
        mov     [packet_size], edx

        mov     ecx, [con_ptr]
        mov     ebx, [ecx+sshlib_connection.tx_pad_size]
        dec     ebx
        and     edx, ebx
        neg     edx
        add     edx, [ecx+sshlib_connection.tx_pad_size]
        add     edx, [ecx+sshlib_connection.tx_pad_size]
        DEBUGF  1, "padding %u bytes ", edx
        add     [packet_size], edx      ; total packet size with padding

; Start building the packet
; First comes the packet length, in network byte order ofcourse.
        add     eax, edx
        DEBUGF  2, "total size: %u ", eax
        bswap   eax
        lea     edi, [ecx+sshlib_connection.tx_buffer]
        stosd
; Then the padding length
        mov     al, dl
        stosb
; And the actual payload bytes
        mov     esi, [buf]
        mov     ecx, [payload_size]
        rep movsb

; Append the packet with #edx padding bytes.
; Since we must pad at least 8 bytes, we can always use DWORD writes.
; First do an (unaligned) write exactly following the data
        dec     edx
        mov     esi, edx
        shr     esi, 2          ; number dwords
        mov     ebx, edx
        and     ebx, 3
        inc     ebx             ; number bytes in first write (1-4)
        mov     dword[edi], 0
        add     edi, ebx
; Then, do as many aligned writes as nescessary
        xor     eax, eax
  @@:
        stosd
        dec     esi
        jnz     @r

; Send the packet
        mov     ebx, [con_ptr]
        mov     ecx, [ebx + sshlib_connection.socketnum]
        lea     edx, [ebx + sshlib_connection.tx_buffer]
        mov     esi, [packet_size]
        mov     edi, [flags]
        mcall   send

; Update sequence counter
        mov     ebx, [con_ptr]
        add     byte[ebx + sshlib_connection.tx_mac_seqnr+3], 1
        adc     byte[ebx + sshlib_connection.tx_mac_seqnr+2], 0
        adc     byte[ebx + sshlib_connection.tx_mac_seqnr+1], 0
        adc     byte[ebx + sshlib_connection.tx_mac_seqnr+0], 0

        DEBUGF  2, "\n"

        ret

endp

