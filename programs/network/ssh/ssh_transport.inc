;    ssh_transport.inc - SSH transport layer
;
;    Copyright (C) 2016 Jeffrey Amelynck
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

struct  ssh_header
        length          dd ?
        padding         db ?
        message_code    db ?
ends

proc dummy_encrypt _key, _in, _out

        ret
endp

proc ssh_recv_packet sock, buf, size, flags

locals
        bufferptr       dd ?
        remaining       dd ?
        padding         dd ?
endl

        DEBUGF  1, "ssh_recv_packet\n"
; Receive first block (Read length, padding length, message code)
        mcall   recv, [sock], [buf], [rx_blocksize], [flags]
        DEBUGF  1, "chunk = %u\n", eax
        cmp     eax, [rx_blocksize]
        jne     .fail       ;;;;

;        stdcall [decrypt_proc], [rx_context], [buf], [buf]

        mov     ebx, [buf]
        movzx   eax, [ebx+ssh_header.padding]
        mov     [padding], eax
        mov     eax, [ebx+ssh_header.length]
        bswap   eax                                             ; length to little endian
        mov     [ebx+ssh_header.length], eax
        DEBUGF  1, "ssh_recv_packet length = %u\n", eax

        cmp     eax, [size]
        ja      .fail       ;;;;

        sub     eax, [rx_blocksize]
        add     eax, 4
        mov     [remaining], eax
        add     ebx, [rx_blocksize]
        mov     [bufferptr], ebx
  .receive_loop:
        mcall   recv, [sock], [bufferptr], [remaining], 0
        DEBUGF  1, "chunk = %u\n", eax
        cmp     eax, 0
        jbe     .fail
        add     [bufferptr], eax
        sub     [remaining], eax
        ja      .receive_loop

;  .decrypt_loop:
;        stdcall [decrypt_proc], [rx_context], [buf], [buf]
;        ja      .decrypt_loop

;  .hmac_loop:
; TODO
;        ja      .hmac_loop

; Return usefull data length in eax
        mov     eax, [buf]
        movzx   ebx, [eax+ssh_header.padding]
        mov     eax, [eax+ssh_header.length]
        sub     eax, ebx
        DEBUGF  1, "ssh_recv_packet complete, usefull data length=%u\n", eax
        ret

  .fail:
        DEBUGF  1, "ssh_recv_packet failed!\n"
        mov     eax, -1
        ret

endp


proc ssh_send_packet sock, buf, payloadsize, flags

locals
        size    dd ?
endl
        DEBUGF  1, "ssh_send_packet: size=%u\n", [payloadsize]

        mov     eax, [payloadsize]
        inc     eax             ; padding length byte

        lea     edx, [eax+4]    ; total packet size (without padding)
        mov     [size], edx
        mov     ebx, [tx_blocksize]
        dec     ebx
        and     edx, ebx
        neg     edx
        add     edx, [tx_blocksize]
        cmp     edx, 4          ; minimum padding size
        jae     @f
        add     edx, [tx_blocksize]
  @@:
        DEBUGF  1, "Padding %u bytes\n", edx
        add     [size], edx

        add     eax, edx
        DEBUGF  1, "Total size: %u\n", eax
        bswap   eax
        mov     edi, tx_buffer
        stosd
        mov     al, dl
        stosb
        mov     esi, [buf]
;        cmp     esi, edi
;        je      @f
        mov     ecx, [payloadsize]
        rep movsb
;  @@:

        mov     ebx, edx
        mov     esi, edx
        and     ebx, 3
        jz      @f
        call    MBRandom
        mov     dword[edi], eax
        add     edi, ebx
  @@:

        shr     esi, 2
  @@:
        call    MBRandom
        stosd
        dec     esi
        jnz     @r

        mcall   send, [sock], tx_buffer, [size], [flags]

        ret

endp