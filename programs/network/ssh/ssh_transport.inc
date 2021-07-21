;    ssh_transport.inc - SSH transport layer
;
;    Copyright (C) 2016-2021 Jeffrey Amelynck
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


struct  ssh_packet_header
        packet_length   dd ?    ; The length of the packet in bytes, not including 'mac' or the
                                ; 'packet_length' field itself.
        padding_length  db ?    ; Length of 'random padding' (bytes).

        message_code    db ?    ; First byte of payload
ends

proc padding_zero

        xor     eax, eax
        ret

endp

proc ssh_recv_packet connection, flags

locals
        data_length     dd ?    ; Total length of packet without MAC
        socket_error    dd ?
endl

        DEBUGF  2, "> "
; Receive first block (Read length, padding length, message code)
        mov     ebx, [connection]
        mov     ecx, [ebx+ssh_connection.socketnum]
        mov     esi, [ebx+ssh_connection.rx_crypt_blocksize]
        lea     edx, [ebx+ssh_connection.rx_buffer]
        mov     edi, [flags]
        mcall   recv
        mov     [socket_error], ebx
        DEBUGF  1, "chunk = %u ", eax
        mov     ebx, [connection]
        cmp     eax, [ebx+ssh_connection.rx_crypt_blocksize]
        jne     .fail

; Decrypt first block
        cmp     [ebx+ssh_connection.rx_crypt_proc], 0
        je      @f
        pusha
        lea     esi, [ebx+ssh_connection.rx_buffer]
        stdcall [ebx+ssh_connection.rx_crypt_proc], [ebx+ssh_connection.rx_crypt_ctx_ptr], esi, esi
        popa
  @@:

; Check data length
        mov     esi, [ebx+ssh_connection.rx_buffer.packet_length]
        bswap   esi                                             ; convert length to little endian
        mov     [ebx+ssh_connection.rx_buffer.packet_length], esi
        DEBUGF  1, "packet length=%u ", esi
        cmp     esi, BUFFERSIZE
        ja      .fail                                           ; packet is too large

; Calculate amount of remaining data
        add     esi, 4                                          ; Packet length field itself is not included in the count
        sub     esi, [ebx+ssh_connection.rx_crypt_blocksize]    ; Already received this amount of data
        add     esi, [ebx+ssh_connection.rx_mac_length]
        jz      .got_all_data

; Receive remaining data
        lea     edx, [ebx+ssh_connection.rx_buffer]
        add     edx, [ebx+ssh_connection.rx_crypt_blocksize]
        mov     ecx, [ebx+ssh_connection.socketnum]
        mov     edi, [flags]
  .receive_loop:
        mcall   recv
        DEBUGF  1, "chunk = %u ", eax
        cmp     eax, 0
        jbe     .fail
        add     edx, eax
        sub     esi, eax
        jnz     .receive_loop

; Decrypt data
        mov     ebx, [connection]
        cmp     [ebx+ssh_connection.rx_crypt_proc], 0
        je      .decrypt_complete
        mov     ecx, [ebx+ssh_connection.rx_buffer.packet_length]
        add     ecx, 4                                          ; Packet_length field itself
        sub     ecx, [ebx+ssh_connection.rx_crypt_blocksize]    ; Already decrypted this amount of data
        jz      .decrypt_complete

        lea     esi, [ebx+ssh_connection.rx_buffer]
        add     esi, [ebx+ssh_connection.rx_crypt_blocksize]
  .decrypt_loop:
        pusha
        stdcall [ebx+ssh_connection.rx_crypt_proc], [ebx+ssh_connection.rx_crypt_ctx_ptr], esi, esi
        popa
        add     esi, [ebx+ssh_connection.rx_crypt_blocksize]
        sub     ecx, [ebx+ssh_connection.rx_crypt_blocksize]
        jnz     .decrypt_loop
  .decrypt_complete:

; Authenticate message
        cmp     [ebx+ssh_connection.rx_mac_proc], 0
        je      .mac_complete
        lea     esi, [ebx+ssh_connection.rx_seq]
        mov     ecx, [ebx+ssh_connection.rx_buffer.packet_length]
        add     ecx, 8                                          ; packet_length field itself + sequence number
        lea     eax, [ebx+ssh_connection.rx_mac_ctx]
        mov     edx, [ebx+ssh_connection.rx_buffer.packet_length]
        bswap   edx                                             ; convert length to big endian
        mov     [ebx+ssh_connection.rx_buffer.packet_length], edx
        stdcall [ebx+ssh_connection.rx_mac_proc], eax, esi, ecx
        mov     edx, [ebx+ssh_connection.rx_buffer.packet_length]
        bswap   edx                                             ; convert length to little endian
        mov     [ebx+ssh_connection.rx_buffer.packet_length], edx

        lea     esi, [ebx+ssh_connection.rx_mac_ctx]
        lea     edi, [ebx+ssh_connection.rx_buffer]
        add     edi, [ebx+ssh_connection.rx_buffer.packet_length]
        add     edi, 4
        mov     ecx, [ebx+ssh_connection.rx_mac_length]
        shr     ecx, 2
        repe cmpsd
        jne     .mac_failed
  .mac_complete:
        add     byte[ebx+ssh_connection.rx_seq+3], 1            ; Update sequence counter
        adc     byte[ebx+ssh_connection.rx_seq+2], 0
        adc     byte[ebx+ssh_connection.rx_seq+1], 0
        adc     byte[ebx+ssh_connection.rx_seq+0], 0

; Return useful data length to the caller via eax register
  .got_all_data:
        mov     eax, [ebx+ssh_connection.rx_buffer.packet_length]
        movzx   ebx, [ebx+ssh_connection.rx_buffer.padding_length]
        sub     eax, ebx
        DEBUGF  1, "useful data length=%u\n", eax
        ret

  .fail:
        DEBUGF  3, "ssh_recv_packet failed!\n"
        mov     eax, -1
        mov     ebx, [socket_error]
        ret

  .mac_failed:
        DEBUGF  3, "ssh_recv_packet MAC failed!\n"
        mov     eax, -2
        mov     ebx, [socket_error]
        ret

endp


proc ssh_send_packet connection, buf, payload_size, flags

locals
        packet_size    dd ?
endl
        DEBUGF  2, "< "

; Check how many bytes we should pad
        mov     eax, [payload_size]
        inc     eax                     ; padding length byte
        lea     edx, [eax+4]            ; total packet size (without padding and MAC)
        mov     [packet_size], edx

        mov     ecx, [connection]
        mov     ebx, [ecx+ssh_connection.tx_pad_size]
        dec     ebx
        and     edx, ebx
        neg     edx
        add     edx, [ecx+ssh_connection.tx_pad_size]
        add     edx, [ecx+ssh_connection.tx_pad_size]
        DEBUGF  1, "padding %u bytes ", edx
        add     [packet_size], edx      ; total packet size with padding

; Start building the packet
; First comes the packet length, in network byte order ofcourse.
        add     eax, edx
        DEBUGF  1, "total size: %u ", eax
        bswap   eax
        lea     edi, [ecx+ssh_connection.tx_buffer]
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
        mov     edx, [connection]
        call    [edx+ssh_connection.tx_pad_proc]
        mov     dword[edi], eax
        add     edi, ebx
; Then, do as many aligned writes as nescessary
        mov     ebx, [connection]
  @@:
        call    [ebx+ssh_connection.tx_pad_proc]
        stosd
        dec     esi
        jnz     @r

; Append the packet with Message Authentication Code
        mov     edx, [connection]
        cmp     [edx+ssh_connection.tx_mac_proc], 0
        je      .mac_complete
        DEBUGF  1, "MAC sequence number: 0x%x\n", [edx+ssh_connection.tx_seq]
        lea     esi, [edx+ssh_connection.tx_seq]
        mov     ecx, [packet_size]
        add     ecx, 4                                          ; Sequence number length
        lea     eax, [edx+ssh_connection.tx_mac_ctx]
        stdcall [edx+ssh_connection.tx_mac_proc], eax, esi, ecx

        lea     esi, [edx+ssh_connection.tx_mac_ctx]
        lea     edi, [edx+ssh_connection.tx_buffer]
        add     edi, [packet_size]
        mov     ecx, [edx+ssh_connection.tx_mac_length]
        shr     ecx, 2
        rep movsd
  .mac_complete:
        add     byte[edx+ssh_connection.tx_seq+3], 1            ; Update sequence counter
        adc     byte[edx+ssh_connection.tx_seq+2], 0
        adc     byte[edx+ssh_connection.tx_seq+1], 0
        adc     byte[edx+ssh_connection.tx_seq+0], 0

; Now, encrypt everything but MAC
        cmp     [edx+ssh_connection.tx_crypt_proc], 0
        je      .encrypt_complete
        lea     esi, [edx+ssh_connection.tx_buffer]
        mov     ecx, [packet_size]
  .encrypt_loop:
        pusha
        stdcall [edx+ssh_connection.tx_crypt_proc], [edx+ssh_connection.tx_crypt_ctx_ptr], esi, esi
        popa
        add     esi, [edx+ssh_connection.tx_crypt_blocksize]
        sub     ecx, [edx+ssh_connection.tx_crypt_blocksize]
        jnz     .encrypt_loop
  .encrypt_complete:

; Send the packet
        mov     ebx, [connection]
        mov     ecx, [ebx+ssh_connection.socketnum]
        lea     edx, [ebx+ssh_connection.tx_buffer]
        mov     esi, [packet_size]
        add     esi, [ebx+ssh_connection.tx_mac_length]
        mov     edi, [flags]
        mcall   send

        DEBUGF  1, "\n"

        ret

endp

