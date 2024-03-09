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
proc sshlib_recv_packet_hmac_etm con_ptr, flags

locals
        data_length     dd ?    ; Total length of packet without MAC
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

        mov     eax, [ebx+sshlib_connection.rx_buffer.packet_length]
        bswap   eax
        mov     [data_length], eax
        DEBUGF  2, "decrypted packet length=%u\n", [data_length]

        add     eax, [ebx + sshlib_connection.rx_mac_length]
        cmp     eax, BUFFERSIZE-4
        ja      .proto_fail

; Receive remaining data
        lea     edx, [ebx+sshlib_connection.rx_buffer+4]
        mov     ecx, [ebx+sshlib_connection.socketnum]
        mov     edi, [flags]
        mov     esi, eax
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

; Authenticate message
        mov     ebx, [con_ptr]
        lea     esi, [ebx + sshlib_connection.rx_mac_seqnr]
        mov     ecx, [data_length]
        add     ecx, 8                                          ; packet_length field itself + sequence number
        lea     eax, [ebx + sshlib_connection.rx_mac_ctx]
        lea     edx, [ebx + sshlib_connection.rx_int_key]
        stdcall [ebx + sshlib_connection.rx_mac_proc], eax, esi, ecx, edx, SHA2_256_LEN

        lea     esi, [ebx + sshlib_connection.rx_mac_ctx]
        lea     edi, [ebx + sshlib_connection.rx_buffer+4]
        add     edi, [data_length]
        mov     ecx, [ebx + sshlib_connection.rx_mac_length]
        shr     ecx, 2
        repe cmpsd     ; TODO: constant time
        jne     .mac_fail

; Decrypt the payload
        lea     eax, [ebx+sshlib_connection.rx_crypt_ctx]
        lea     edi, [ebx+sshlib_connection.rx_buffer+4]
        stdcall [ebx + sshlib_connection.rx_crypt_proc], eax, edi, [data_length], edi

; Update sequence counter
        add     byte[ebx+sshlib_connection.rx_mac_seqnr+3], 1
        adc     byte[ebx+sshlib_connection.rx_mac_seqnr+2], 0
        adc     byte[ebx+sshlib_connection.rx_mac_seqnr+1], 0
        adc     byte[ebx+sshlib_connection.rx_mac_seqnr+0], 0

; Return useful data length to the caller via eax register
        mov     eax, [data_length]
        mov     [ebx+sshlib_connection.rx_buffer.packet_length], eax
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
proc sshlib_send_packet_hmac_etm con_ptr, buf, payload_size, flags

locals
        padded_size     dd ?    ; payload with padding (without length field or MAC)
endl
        DEBUGF  2, "< "

; Check how many bytes we should pad
        mov     eax, [payload_size]
        inc     eax                     ; padding length byte

        mov     ecx, [con_ptr]
        mov     edx, eax
        mov     ebx, [ecx + sshlib_connection.tx_pad_size]
        dec     ebx
        and     edx, ebx
        neg     edx
        add     edx, [ecx + sshlib_connection.tx_pad_size]
        add     edx, [ecx + sshlib_connection.tx_pad_size]
        DEBUGF  2, "padding %u bytes ", edx
        add     eax, edx
        mov     [padded_size], eax      ; total packet size with padding, without MAC

; Start building the packet
; First comes the packet length, in network byte order ofcourse.
        DEBUGF  2, "total size: %u ", eax
        bswap   eax
        lea     edi, [ecx + sshlib_connection.tx_buffer]
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
        call    MBRandom
        mov     dword[edi], eax
        add     edi, ebx
; Then, do as many aligned writes as nescessary
  @@:
        call    MBRandom
        stosd
        dec     esi
        jnz     @r

; Encrypt the payload
        mov     ebx, [con_ptr]
        lea     esi, [ebx + sshlib_connection.tx_crypt_ctx]
        lea     edi, [ebx + sshlib_connection.tx_buffer+4]
        stdcall [ebx + sshlib_connection.tx_crypt_proc], esi, edi, [padded_size], edi

; Append the packet with Message Authentication Code
;        mov     ebx, [con_ptr]
        DEBUGF  1, "MAC sequence number: 0x%x\n", [ebx + sshlib_connection.tx_mac_seqnr]
        lea     esi, [ebx + sshlib_connection.tx_mac_seqnr]
        mov     ecx, [padded_size]
        add     ecx, 8                                          ; Sequence number length + packet length field
        lea     eax, [ebx + sshlib_connection.tx_mac_ctx]
        lea     edx, [ebx + sshlib_connection.tx_int_key]
        stdcall [ebx + sshlib_connection.tx_mac_proc], eax, esi, ecx, edx, SHA2_256_LEN

        mov     ebx, [con_ptr]
        lea     esi, [ebx + sshlib_connection.tx_mac_ctx]
        lea     edi, [ebx + sshlib_connection.tx_buffer+4]
        add     edi, [padded_size]
        mov     ecx, [ebx + sshlib_connection.tx_mac_length]
        shr     ecx, 2
        rep movsd

; Update sequence counter
        add     byte[ebx+sshlib_connection.tx_mac_seqnr+3], 1
        adc     byte[ebx+sshlib_connection.tx_mac_seqnr+2], 0
        adc     byte[ebx+sshlib_connection.tx_mac_seqnr+1], 0
        adc     byte[ebx+sshlib_connection.tx_mac_seqnr+0], 0

; Send the packet
;        mov     ebx, [con_ptr]
        mov     ecx, [ebx+sshlib_connection.socketnum]
        lea     edx, [ebx+sshlib_connection.tx_buffer]
        mov     esi, [padded_size]
        add     esi, 4
        add     esi, [ebx+sshlib_connection.tx_mac_length]
        mov     edi, [flags]
        mcall   send

        DEBUGF  2, "\n"

        ret

endp

