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

iglobal

align 16
null_bytes:    times 64 db 0

endg


align 16
proc sshlib_recv_packet_poly1305chacha20 con_ptr, flags

locals
        data_length     dd ?    ; Total length of packet without MAC
        mac_otk         rb 64   ;256/8
        iv              rd 4
endl

        DEBUGF  3, "> "
; Receive first block (Read length)
        mov     ebx, [con_ptr]
        mov     ecx, [ebx+sshlib_connection.socketnum]
        mov     esi, 4
        lea     edx, [ebx+sshlib_connection.rx_buffer]
        mov     edi, [flags]
        mcall   recv
        cmp     eax, 0
        jle     .sock_fail
        sub     [ssh_chan.rcv_wnd], eax  ;;; FIXME
        DEBUGF  1, "chunk = %u ", eax
        mov     ebx, [con_ptr]
        cmp     eax, 4
        jne     .proto_fail     ; TODO: handle receives of 1, 2, and 3 bytes correctly

; Decrypt data length
        mov     ebx, [con_ptr]

        lea     eax, [iv]
        mov     dword[eax+0], 0
        mov     dword[eax+4], 0
        mov     dword[eax+8], 0
        push    [ebx+sshlib_connection.rx_mac_seqnr]
        pop     dword[eax+12]

        lea     ecx, [ebx+sshlib_connection.rx_crypt_ctx]
        lea     edx, [ebx+sshlib_connection.rx_enc_key+256/8]
        lea     esi, [ebx+sshlib_connection.rx_buffer]
        lea     edi, [data_length]
        invoke  chacha20_oneshot, ecx, edx, eax, 0, esi, 4, edi

        mov     eax, [data_length]
        bswap   eax
        mov     [data_length], eax
        DEBUGF  2, "decrypted packet length=%u\n", [data_length]

        cmp     eax, BUFFERSIZE-4-128/8
        ja      .proto_fail

; Receive remaining data
        lea     edx, [ebx+sshlib_connection.rx_buffer+4]
        mov     ecx, [ebx+sshlib_connection.socketnum]
        mov     edi, [flags]
        lea     esi, [eax + 128/8]                              ; We already got 4 bytes but they are not counted, MAC is also not counted so add that
  .recv_loop:
        DEBUGF  3, "want %u bytes.. ", esi
        mcall   recv
        cmp     eax, 0
        jle     .sock_fail
        sub     [ssh_chan.rcv_wnd], eax             ;;; FIXME
        DEBUGF  3, "got %u bytes\n", eax
        add     edx, eax
        sub     esi, eax
        jnz     .recv_loop

; Calculate the OTK
        mov     ebx, [con_ptr]
        lea     eax, [ebx+sshlib_connection.rx_crypt_ctx]
        lea     esi, [ebx+sshlib_connection.rx_enc_key]
        lea     edx, [mac_otk]
        lea     ecx, [iv]
        invoke  chacha20_oneshot, eax, esi, ecx, 0, null_bytes, 64, edx

; Calculate the MAC
        lea     esi, [ebx+sshlib_connection.rx_mac_ctx]
        lea     edi, [ebx+sshlib_connection.rx_buffer]
        mov     ecx, [data_length]
        add     ecx, 4
        lea     edx, [mac_otk]
        push    ecx
        invoke  poly1305_oneshot, esi, edi, ecx, edx, 256/8
        pop     ecx

; Compare in constant time
        add     edi, ecx
        cmpsd
        lahf
        mov     edx, eax
        cmpsd
        lahf
        and     edx, eax
        cmpsd
        lahf
        and     edx, eax
        cmpsd
        lahf
        and     eax, edx
        sahf
        jne     .mac_fail

; Decrypt the payload
        lea     eax, [ebx+sshlib_connection.rx_crypt_ctx]
        lea     edi, [ebx+sshlib_connection.rx_buffer+4]
        invoke  chacha20_update, eax, edi, [data_length], edi

; Put decrypted length in rx buffer
        push    [data_length]
        pop     dword[ebx+sshlib_connection.rx_buffer]

; Update sequence counter
        add     byte[ebx+sshlib_connection.rx_mac_seqnr+3], 1
        adc     byte[ebx+sshlib_connection.rx_mac_seqnr+2], 0
        adc     byte[ebx+sshlib_connection.rx_mac_seqnr+1], 0
        adc     byte[ebx+sshlib_connection.rx_mac_seqnr+0], 0

; Return useful data length to the caller via eax register
;;;  .packet_complete:
        mov     eax, [ebx+sshlib_connection.rx_buffer.packet_length]
        movzx   ebx, [ebx+sshlib_connection.rx_buffer.padding_length]
        sub     eax, ebx
        DEBUGF  1, "useful data length=%u\n", eax
        ret

  .sock_fail:
        DEBUGF  3, "ssh_recv_packet failed!\n"
        mov     eax, SSHLIB_ERR_SOCKET
        ret

  .mac_fail:
        DEBUGF  3, "ssh_recv_packet message authentication failed!\n"
        mov     eax, SSHLIB_ERR_MAC_VERIFY_FAIL
        xor     ebx, ebx
        ret

  .proto_fail:
        DEBUGF  3, "ssh_recv_packet protocol failure!\n"
        mov     eax, SSHLIB_ERR_PROTOCOL
        xor     ebx, ebx
        ret

endp


align 16
proc sshlib_send_packet_poly1305chacha20 con_ptr, buf, payload_size, flags

locals
        packet_size     dd ?
        mac_otk         rb 64   ;256/8
        iv              rd 4
endl
        DEBUGF  2, "< "

; Check how many bytes we should pad
        mov     eax, [payload_size]
        inc     eax                     ; padding length byte
        lea     edx, [eax+4]            ; total packet size without padding and MAC
        mov     [packet_size], edx

        mov     ecx, [con_ptr]
        mov     edx, eax
        mov     ebx, [ecx+sshlib_connection.tx_pad_size]
        dec     ebx
        and     edx, ebx
        neg     edx
        add     edx, [ecx+sshlib_connection.tx_pad_size]
        add     edx, [ecx+sshlib_connection.tx_pad_size]
        DEBUGF  2, "padding %u bytes ", edx
        add     [packet_size], edx      ; total packet size with padding, without MAC

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
;;; And the actual payload bytes
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
        mov     edx, [con_ptr]
        call    MBRandom
        mov     dword[edi], eax
        add     edi, ebx
; Then, do as many aligned writes as nescessary
        mov     ebx, [con_ptr]
  @@:
        call    MBRandom
        stosd
        dec     esi
        jnz     @r

; Encrypt data length
        lea     eax, [iv]
        mov     dword[eax+0], 0
        mov     dword[eax+4], 0
        mov     dword[eax+8], 0
        push    [ebx+sshlib_connection.tx_mac_seqnr]
        pop     dword[eax+12]

        lea     esi, [ebx+sshlib_connection.tx_crypt_ctx]
        lea     edx, [ebx+sshlib_connection.tx_enc_key+256/8]
        lea     edi, [ebx+sshlib_connection.tx_buffer]
        invoke  chacha20_oneshot, esi, edx, eax, 0, edi, 4, edi

; Calculate the OTK
        lea     eax, [iv]
        lea     edx, [ebx+sshlib_connection.tx_enc_key]
        lea     edi, [mac_otk]
        invoke  chacha20_oneshot, esi, edx, eax, 0, null_bytes, 64, edi

; Encrypt the payload
        lea     edi, [ebx+sshlib_connection.tx_buffer+4]
        mov     eax, [packet_size]
        sub     eax, 4
        invoke  chacha20_update, esi, edi, eax, edi

; Calculate the MAC
        lea     esi, [ebx+sshlib_connection.tx_mac_ctx]
        lea     edi, [ebx+sshlib_connection.tx_buffer]
        lea     edx, [mac_otk]
        invoke  poly1305_oneshot, esi, edi, [packet_size], edx, 256/8

; Write it to the send buffer
        add     edi, [packet_size]
        movsd
        movsd
        movsd
        movsd

; Update sequence counter
        add     byte[ebx+sshlib_connection.tx_mac_seqnr+3], 1
        adc     byte[ebx+sshlib_connection.tx_mac_seqnr+2], 0
        adc     byte[ebx+sshlib_connection.tx_mac_seqnr+1], 0
        adc     byte[ebx+sshlib_connection.tx_mac_seqnr+0], 0

; Send the packet
;        mov     ebx, [con_ptr]
        mov     ecx, [ebx+sshlib_connection.socketnum]
        lea     edx, [ebx+sshlib_connection.tx_buffer]
        mov     esi, [packet_size]
        add     esi, 16 ;[ebx+sshlib_connection.tx_mac_length]
        mov     edi, [flags]
        mcall   send

        DEBUGF  2, "\n"

        ret

endp

