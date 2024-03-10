;    sshlib_connection.inc - SSH connection
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

; https://www.ietf.org/rfc/rfc4253.txt

proc sshlib_connect con_ptr, hostname_sz

locals
        socketnum       dd ?
        sockaddr        sockaddr_in
        ctx_ptr         dd ?
endl

        mov     edi, [con_ptr]
        lea     eax, [edi + sshlib_connection.part_ex_hash_ctx]
        mov     [ctx_ptr], eax

; Set default values in sockaddr struct
        mov     [sockaddr.sin_family], AF_INET4
        mov     [sockaddr.sin_port], 22 shl 8

; Parse hostname_sz
; Verify length, extract port number if given and copy base url to sshlib_connection struct
; Port number, if provided, will be written in sockaddr struct.
; Hostname ends with any character equal to 0x20 or lower

        mov     esi, [hostname_sz]
        lea     edi, [edi + sshlib_connection.hostname_sz]
        mov     ecx, MAX_HOSTNAME_LENGTH
  @@:
        dec     ecx
        jz      .err_hostname
        lodsb
        cmp     al, ':'
        je      .do_port
        stosb
        cmp     al, 0x20
        ja      @r
        mov     byte[edi-1], 0
        jmp     .hostname_ok

  .do_port:
        xor     eax, eax
        xor     ebx, ebx
        mov     byte[edi-1], 0
  .portloop:
        lodsb
        cmp     al, 0x20
        jbe     .port_done
        sub     al, '0'
        jb      .err_hostname
        cmp     al, 9
        ja      .err_hostname
        lea     ebx, [ebx*4+ebx]
        shl     ebx, 1
        add     ebx, eax
        jmp     .portloop
  .port_done:
        xchg    bl, bh
        mov     [sockaddr.sin_port], bx

  .hostname_ok:
; resolve name
        push    esp     ; reserve stack place
        push    esp
        mov     eax, [con_ptr]
        lea     eax, [eax+sshlib_connection.hostname_sz]
        invoke  getaddrinfo, eax, 0, 0
        pop     esi
; test for error
        test    eax, eax
        jnz     .err_hostname

; convert IP address to decimal notation
        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr.sin_addr], eax
        invoke  inet_ntoa, eax
; write result
        stdcall sshlib_callback_connecting, [con_ptr], eax
; free allocated memory
        invoke  freeaddrinfo, esi

; Create socket
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        jz      .err_sock
        mov     [socketnum], eax
        mov     ebx, [con_ptr]
        mov     [ebx + sshlib_connection.socketnum], eax

; Connect
        DEBUGF  2, "Connecting to server\n"
        lea     edx, [sockaddr]
        mcall   connect, [socketnum], , sizeof.sockaddr_in
        test    eax, eax
        jnz     .err_sock

; Start calculating hash
        invoke  sha2_256.init, [ctx_ptr]
; HASH: string  V_C, the client's version string (CR and NL excluded)
        invoke  sha2_256.update, [ctx_ptr], ssh_ident_ha, ssh_msg_ident.length+4-2

; >> Send our identification string
        DEBUGF  2, "Sending ID string\n"
        mcall   send, [socketnum], ssh_msg_ident, ssh_msg_ident.length, 0
        cmp     eax, -1
        je      .err_sock

; << Check protocol version of server
        mov     edx, [con_ptr]
        lea     edx, [edx + sshlib_connection.rx_buffer + 4]
        mcall   recv, [socketnum], , PACKETSIZE, 0
        cmp     eax, -1
        je      .err_sock

        DEBUGF  2, "Received ID string\n"
        cmp     dword[edx], "SSH-"
        jne     .err_proto
        cmp     dword[edx+4], "2.0-"
        jne     .err_proto

; HASH: string  V_S, the server's version string (CR and NL excluded)
        lea     ecx, [eax+2]
        sub     eax, 2
        bswap   eax
        sub     edx, 4
        mov     dword[edx], eax
        invoke  sha2_256.update, [ctx_ptr], edx, ecx

; >> Key Exchange init
        mov     eax, [con_ptr]
        mov     [eax + sshlib_connection.status], SSHLIB_CON_STAT_INIT

        mov     [eax + sshlib_connection.algo_kex], SSHLIB_ALGO_NONE
        mov     [eax + sshlib_connection.algo_hostkey], SSHLIB_ALGO_NONE
        mov     [eax + sshlib_connection.algo_crypt_rx], SSHLIB_ALGO_NONE
        mov     [eax + sshlib_connection.algo_crypt_tx], SSHLIB_ALGO_NONE
        mov     [eax + sshlib_connection.algo_mac_rx], SSHLIB_ALGO_NONE
        mov     [eax + sshlib_connection.algo_mac_tx], SSHLIB_ALGO_NONE
        mov     [eax + sshlib_connection.algo_compr_rx], SSHLIB_ALGO_NONE
        mov     [eax + sshlib_connection.algo_compr_tx], SSHLIB_ALGO_NONE

        mov     [eax + sshlib_connection.rx_mac_seqnr], 0
        mov     [eax + sshlib_connection.tx_mac_seqnr], 0
        mov     [eax + sshlib_connection.rx_crypt_blocksize], 4             ; minimum blocksize
        mov     [eax + sshlib_connection.tx_crypt_blocksize], 4
        mov     [eax + sshlib_connection.rx_crypt_proc], 0
        mov     [eax + sshlib_connection.tx_crypt_proc], 0
        mov     [eax + sshlib_connection.rx_mac_proc], 0
        mov     [eax + sshlib_connection.tx_mac_proc], 0
        mov     [eax + sshlib_connection.rx_mac_length], 0
        mov     [eax + sshlib_connection.tx_mac_length], 0
        mov     [eax + sshlib_connection.tx_pad_size], 8

        mov     [eax + sshlib_connection.rx_proc], sshlib_recv_packet_clear
        mov     [eax + sshlib_connection.tx_proc], sshlib_send_packet_clear

        DEBUGF  2, "Sending KEX init\n"
        mov     edi, ssh_msg_kex.cookie
        call    MBRandom
        stosd
        call    MBRandom
        stosd
        call    MBRandom
        stosd
        call    MBRandom
        stosd
        stdcall sshlib_send_packet, [con_ptr], ssh_msg_kex, ssh_msg_kex.length, 0
        cmp     eax, -1
        je      .err_sock

; HASH: string  I_C, the payload of the client's SSH_MSG_KEXINIT
        mov     esi, [con_ptr]
        mov     eax, [esi+sshlib_connection.tx_buffer.packet_length]
        bswap   eax
        movzx   ebx, [esi+sshlib_connection.tx_buffer.padding_length]
        sub     eax, ebx
        dec     eax
        lea     edx, [eax+4]
        bswap   eax
        lea     esi, [esi+sshlib_connection.tx_buffer+1]
        mov     dword[esi], eax
        invoke  sha2_256.update, [ctx_ptr], esi, edx

; << Check key exchange init of server
        stdcall sshlib_recv_packet, [con_ptr], 0
        cmp     eax, -1
        je      .err_sock

        mov     ebx, [con_ptr]
        cmp     [ebx + sshlib_connection.rx_buffer.message_code], SSH_MSG_KEXINIT
        jne     .err_proto
        DEBUGF  2, "Received KEX init\n"
        lea     esi, [ebx + sshlib_connection.rx_buffer + sizeof.ssh_packet_header + 16]

        DEBUGF  2, "kex_algorithm "
        stdcall sshlib_algo_find_match, ssh_msg_kex.kex_algorithms, algorithms_kex
        test    eax, eax
        jz      .err_no_algo
        mov     [ebx + sshlib_connection.algo_kex], eax

        DEBUGF  2, "server_host_key_algorithm "
        stdcall sshlib_algo_find_match, ssh_msg_kex.server_host_key_algorithms, algorithms_hostkey
        test    eax, eax
        jz      .err_no_algo
        mov     [ebx + sshlib_connection.algo_hostkey], eax

        DEBUGF  2, "encryption_algorithm_client_to_server "
        stdcall sshlib_algo_find_match, ssh_msg_kex.encryption_algorithms_client_to_server, algorithms_crypt
        test    eax, eax
        jz      .err_no_algo
        mov     [ebx + sshlib_connection.algo_crypt_tx], eax

        DEBUGF  2, "encryption_algorithm_server_to_client ",
        stdcall sshlib_algo_find_match, ssh_msg_kex.encryption_algorithms_server_to_client, algorithms_crypt
        test    eax, eax
        jz      .err_no_algo
        mov     [ebx + sshlib_connection.algo_crypt_rx], eax

        DEBUGF  2, "mac_algorithm_client_to_server "
        stdcall sshlib_algo_find_match, ssh_msg_kex.mac_algorithms_client_to_server, algorithms_mac
        test    eax, eax
        jz      .err_no_algo
        mov     [ebx + sshlib_connection.algo_mac_tx], eax

        DEBUGF  2, "mac_algorithm_server_to_client "
        stdcall sshlib_algo_find_match, ssh_msg_kex.mac_algorithms_server_to_client, algorithms_mac
        test    eax, eax
        jz      .err_no_algo
        mov     [ebx + sshlib_connection.algo_mac_rx], eax

        DEBUGF  2, "compression_algorithm_client_to_server "
        stdcall sshlib_algo_find_match, ssh_msg_kex.compression_algorithms_client_to_server, algorithms_compression
        test    eax, eax
        jz      .err_no_algo
        mov     [ebx + sshlib_connection.algo_compr_tx], eax

        DEBUGF  2, "compression_algorithm_server_to_client "
        stdcall sshlib_algo_find_match, ssh_msg_kex.compression_algorithms_server_to_client, algorithms_compression
        test    eax, eax
        jz      .err_no_algo
        mov     [ebx + sshlib_connection.algo_compr_rx], eax

        DEBUGF  2, "language_client_to_server "
        stdcall sshlib_algo_find_match, ssh_msg_kex.languages_client_to_server, languages

        DEBUGF  2, "language_server_to_client "
        stdcall sshlib_algo_find_match, ssh_msg_kex.languages_server_to_client, languages

        lodsb
        DEBUGF  2, "KEX First Packet Follows: %u\n", al

; HASH: string I_S, the payload of the servers's SSH_MSG_KEXINIT
        mov     esi, [con_ptr]
        mov     eax, [esi + sshlib_connection.rx_buffer.packet_length]
        movzx   ebx, [esi + sshlib_connection.rx_buffer.padding_length]
        sub     eax, ebx
        dec     eax
        lea     edx, [eax+4]
        bswap   eax
        lea     esi, [esi + sshlib_connection.rx_buffer+1]
        mov     dword[esi], eax
        invoke  sha2_256.update, [ctx_ptr], esi, edx

; Exchange keys with the server

        mov     ebx, [con_ptr]
        cmp     [ebx + sshlib_connection.algo_kex], SSHLIB_KEX_DH_SHA256        ; only kex algo supported for now
        jne     .err_no_algo

        stdcall sshlib_dh_gex, [con_ptr]
        test    eax, eax
        jnz     .err

; Set keys and initialize transport subroutines

        DEBUGF  2, "SSH: Setting encryption keys\n"

        mov     ebx, [con_ptr]

        cmp     [ebx + sshlib_connection.algo_crypt_rx], SSHLIB_CRYPT_AES256_CTR
        je      .rx_crypt_aes256_ctr
        cmp     [ebx + sshlib_connection.algo_crypt_rx], SSHLIB_CRYPT_AES256_CBC
        je      .rx_crypt_aes256_cbc
        cmp     [ebx + sshlib_connection.algo_crypt_rx], SSHLIB_CRYPT_CHACHA20_POLY1305
        je      .rx_crypt_poly1305_chacha20

        jmp     .err_proto

  .rx_crypt_aes256_ctr:
        lea     ecx, [ebx + sshlib_connection.rx_crypt_ctx]
        lea     edx, [ebx + sshlib_connection.rx_enc_key]
        lea     esi, [ebx + sshlib_connection.rx_iv]
        invoke  aes256ctr.init, ecx, edx, esi, 0
        push    [aes256ctr.update]
        pop     [ebx + sshlib_connection.rx_crypt_proc]
        mov     [ebx + sshlib_connection.rx_crypt_blocksize], 16        ; AES_BLOCKSIZE
        jmp     .have_rx_crypt

  .rx_crypt_aes256_cbc:
        lea     ecx, [ebx + sshlib_connection.rx_crypt_ctx]
        lea     edx, [ebx + sshlib_connection.rx_enc_key]
        lea     esi, [ebx + sshlib_connection.rx_iv]
        invoke  aes256cbc.init, ecx, edx, esi, 0
        push    [aes256cbc.update]
        pop     [ebx + sshlib_connection.rx_crypt_proc]
        mov     [ebx + sshlib_connection.rx_crypt_blocksize], 16        ; AES_BLOCKSIZE
        jmp     .have_rx_crypt

  .rx_crypt_poly1305_chacha20:
        mov     [ebx + sshlib_connection.rx_proc], sshlib_recv_packet_poly1305chacha20
        jmp     .have_rx_crypt_and_mac



  .have_rx_crypt:
        cmp     [ebx + sshlib_connection.algo_mac_rx], SSHLIB_MAC_HMAC_SHA2_256
        je      .rx_hmac_sha2_256
        cmp     [ebx + sshlib_connection.algo_mac_rx], SSHLIB_MAC_HMAC_SHA2_512
        je      .rx_hmac_sha2_512
        cmp     [ebx + sshlib_connection.algo_mac_rx], SSHLIB_MAC_HMAC_SHA2_256_ETM
        je      .rx_hmac_sha2_256_etm
        cmp     [ebx + sshlib_connection.algo_mac_rx], SSHLIB_MAC_HMAC_SHA2_512_ETM
        je      .rx_hmac_sha2_512_etm

        jmp     .err_proto

  .rx_hmac_sha2_256:
        push    [hmac_sha2_256.oneshot]
        pop     [ebx + sshlib_connection.rx_mac_proc]
        mov     [ebx + sshlib_connection.rx_mac_length], SHA2_256_LEN
        mov     [ebx + sshlib_connection.rx_proc], sshlib_recv_packet_hmac
        jmp     .have_rx_crypt_and_mac

  .rx_hmac_sha2_512:
        push    [hmac_sha2_512.oneshot]
        pop     [ebx + sshlib_connection.rx_mac_proc]
        mov     [ebx + sshlib_connection.rx_mac_length], SHA2_512_LEN
        mov     [ebx + sshlib_connection.rx_proc], sshlib_recv_packet_hmac
        jmp     .have_rx_crypt_and_mac

  .rx_hmac_sha2_256_etm:
        push    [hmac_sha2_256.oneshot]
        pop     [ebx + sshlib_connection.rx_mac_proc]
        mov     [ebx + sshlib_connection.rx_mac_length], SHA2_256_LEN
        mov     [ebx + sshlib_connection.rx_proc], sshlib_recv_packet_hmac_etm
        jmp     .have_rx_crypt_and_mac

  .rx_hmac_sha2_512_etm:
        push    [hmac_sha2_512.oneshot]
        pop     [ebx + sshlib_connection.rx_mac_proc]
        mov     [ebx + sshlib_connection.rx_mac_length], SHA2_512_LEN
        mov     [ebx + sshlib_connection.rx_proc], sshlib_recv_packet_hmac_etm
        jmp     .have_rx_crypt_and_mac


  .have_rx_crypt_and_mac:

        cmp     [ebx + sshlib_connection.algo_crypt_tx], SSHLIB_CRYPT_AES256_CTR
        je      .tx_crypt_aes256_ctr
        cmp     [ebx + sshlib_connection.algo_crypt_tx], SSHLIB_CRYPT_AES256_CBC
        je      .tx_crypt_aes256_cbc
        cmp     [ebx + sshlib_connection.algo_crypt_tx], SSHLIB_CRYPT_CHACHA20_POLY1305
        je      .tx_crypt_poly1305_chacha20

        jmp     .err_proto

  .tx_crypt_aes256_ctr:
        lea     ecx, [ebx + sshlib_connection.tx_crypt_ctx]
        lea     edx, [ebx + sshlib_connection.tx_enc_key]
        lea     esi, [ebx + sshlib_connection.tx_iv]
        invoke  aes256ctr.init, ecx, edx, esi, 0
        push    [aes256ctr.update]
        pop     [ebx + sshlib_connection.tx_crypt_proc]
        mov     [ebx + sshlib_connection.tx_crypt_blocksize], 16        ; AES_BLOCKSIZE
        mov     [ebx + sshlib_connection.tx_pad_size], 16               ; AES_BLOCKSIZE
        jmp     .have_tx_crypt

  .tx_crypt_aes256_cbc:
        lea     ecx, [ebx + sshlib_connection.tx_crypt_ctx]
        lea     edx, [ebx + sshlib_connection.tx_enc_key]
        lea     esi, [ebx + sshlib_connection.tx_iv]
        invoke  aes256cbc.init, ecx, edx, esi, 0
        push    [aes256cbc.update]
        pop     [ebx + sshlib_connection.tx_crypt_proc]
        mov     [ebx + sshlib_connection.tx_crypt_blocksize], 16        ; AES_BLOCKSIZE
        mov     [ebx + sshlib_connection.tx_pad_size], 16               ; AES_BLOCKSIZE
        jmp     .have_tx_crypt

  .tx_crypt_poly1305_chacha20:
        mov     [ebx + sshlib_connection.tx_proc], sshlib_send_packet_poly1305chacha20
        jmp     .have_tx_crypt_and_mac



  .have_tx_crypt:
        cmp     [ebx + sshlib_connection.algo_mac_tx], SSHLIB_MAC_HMAC_SHA2_256
        je      .tx_hmac_sha2_256
        cmp     [ebx + sshlib_connection.algo_mac_tx], SSHLIB_MAC_HMAC_SHA2_512
        je      .tx_hmac_sha2_512
        cmp     [ebx + sshlib_connection.algo_mac_tx], SSHLIB_MAC_HMAC_SHA2_256_ETM
        je      .tx_hmac_sha2_256_etm
        cmp     [ebx + sshlib_connection.algo_mac_tx], SSHLIB_MAC_HMAC_SHA2_512_ETM
        je      .tx_hmac_sha2_512_etm

        jmp     .err_proto

  .tx_hmac_sha2_256:
        push    [hmac_sha2_256.oneshot]
        pop     [ebx + sshlib_connection.tx_mac_proc]
        mov     [ebx + sshlib_connection.tx_mac_length], SHA2_256_LEN
        mov     [ebx + sshlib_connection.tx_proc], sshlib_send_packet_hmac
        jmp     .have_tx_crypt_and_mac

  .tx_hmac_sha2_512:
        push    [hmac_sha2_512.oneshot]
        pop     [ebx + sshlib_connection.tx_mac_proc]
        mov     [ebx + sshlib_connection.tx_mac_length], SHA2_512_LEN
        mov     [ebx + sshlib_connection.tx_proc], sshlib_send_packet_hmac
        jmp     .have_tx_crypt_and_mac

  .tx_hmac_sha2_256_etm:
        push    [hmac_sha2_256.oneshot]
        pop     [ebx + sshlib_connection.tx_mac_proc]
        mov     [ebx + sshlib_connection.tx_mac_length], SHA2_256_LEN
        mov     [ebx + sshlib_connection.tx_proc], sshlib_send_packet_hmac_etm
        jmp     .have_tx_crypt_and_mac

  .tx_hmac_sha2_512_etm:
        push    [hmac_sha2_512.oneshot]
        pop     [ebx + sshlib_connection.tx_mac_proc]
        mov     [ebx + sshlib_connection.tx_mac_length], SHA2_512_LEN
        mov     [ebx + sshlib_connection.tx_proc], sshlib_send_packet_hmac_etm
        jmp     .have_tx_crypt_and_mac


  .have_tx_crypt_and_mac:

; Re-seed RNG for padding bytes

        call    create_seed
        call    init_random

        xor     eax, eax
        ret

  .err_no_algo:
        mov     eax, SSHLIB_ERR_NO_ALGO
        ret

  .err_hostname:
        mov     eax, SSHLIB_ERR_HOSTNAME
        ret

  .err_sock:
        mov     eax, SSHLIB_ERR_SOCKET
        ret

  .err_proto:
        mov     eax, SSHLIB_ERR_PROTOCOL
        ret

  .err:
        ret

endp



proc sshlib_algo_find_match uses ebx ecx edx edi, client_str, algo_list

locals
        server_str      dd ?
        next_str        dd ?
        current         dd ?
endl

        lodsd
        mov     [server_str], esi
        bswap   eax
        lea     ecx, [esi + eax]
        mov     [next_str], ecx

        mov     edi, [client_str]
        mov     edx, dword[edi]
        bswap   edx
        add     edi, 4
        add     edx, edi        ; end of string

  .go:
        mov     [current], edi
  .cmp:
        cmp     esi, ecx
        jae     .end_of_s
        mov     al, byte[esi]
        inc     esi
  .cmp_1:
        cmp     edi, edx
        jae     .end_of_c
        mov     bl, byte[edi]
        inc     edi
  .cmp_2:
        cmp     al, bl
        jne     .mismatch

        cmp     al, ','
        jne     .cmp

; algo matches, print it to debug board
        DEBUGF  2, "= "
        mov     edi, [current]
  @@:
        cmp     edi, edx
        jae     @f
        mov     cl, byte[edi]
        cmp     cl, ','
        je      @f
        mcall   63, 1
        inc     edi
        jmp     @r
  @@:
;        mcall   63, 1, 10       ; print newline

; and now find it in algo list
        mov     esi, [algo_list]
  .algo_loop:
        mov     edi, [current]
        lodsd
        mov     ebx, eax        ; algo code
        test    eax, eax
        jz      .no_match

  .algo_charloop:
        lodsb
        test    al, al
        jz      .check_end
        cmp     al, byte[edi]
        jne     .next_algo
        inc     edi
        cmp     edi, edx
        jb      .algo_charloop
; we reached end of input, check end of algo token
        cmp     byte[esi], 0
        je      .algo_match
        jmp     .next_algo
; we reached end of algo token, check end of input
  .check_end:
        cmp     byte[edi], ','
        je      .algo_match

  .next_algo_loop:
        lodsb
  .next_algo:
        test    al, al
        jnz     .next_algo_loop
        jmp     .algo_loop

  .algo_match:
        mov     eax, ebx
        mov     esi, [next_str]
        DEBUGF  2," (%u)\n", eax
        ret

  .end_of_s:
        mov     al, ','
        jmp     .cmp_1

  .end_of_c:
        mov     bl, ','
        jmp     .cmp_2

  .mismatch:
; character mismatch, reset client str and go to next server token
        mov     edi, [current]
  @@:
        mov     al, byte[esi]
        inc     esi

        cmp     al, ','
        je      .cmp

        cmp     esi, ecx
        jb      @r

; end of server str, reset it and go to next client token
        mov     esi, [server_str]
  @@:
        mov     bl, byte[edi]
        inc     edi

        cmp     bl, ','
        je     .go

        cmp     edi, edx
        jb      @r

; end of client str, no match found
  .no_match:
        xor     eax, eax
        mov     esi, [next_str]
        DEBUGF  2," (%u)\n", eax
        ret

endp




; Handle common messages and return to caller for specific ones
proc sshlib_msg_handler, con_ptr, flags

  .recv:
; Send a window update if advertised window drops below half
        cmp     [ssh_chan.rcv_wnd], BUFFERSIZE/2
        ja      .no_wnd
        mov     eax, BUFFERSIZE
        bswap   eax
        mov     [ssh_msg_channel_window_adjust.wnd], eax
        stdcall sshlib_send_packet, [con_ptr], ssh_msg_channel_window_adjust, ssh_msg_channel_window_adjust.length, 0
        mov     [ssh_chan.rcv_wnd], BUFFERSIZE
  .no_wnd:

; Receive 1 SSH packet
        stdcall sshlib_recv_packet, [con_ptr], [flags]
        cmp     eax, 0
        jle     .ret

        mov     esi, [con_ptr]
        lea     esi, [esi + sshlib_connection.rx_buffer]
        mov     al, [esi + ssh_packet_header.message_code]
        add     esi, sizeof.ssh_packet_header

        cmp     al, SSH_MSG_DISCONNECT
        je      .disc
        cmp     al, SSH_MSG_IGNORE
        je      .ign
        cmp     al, SSH_MSG_DEBUG
        je      .dbg
        cmp     al, SSH_MSG_GLOBAL_REQUEST
        je      .glob_req
        cmp     al, SSH_MSG_CHANNEL_WINDOW_ADJUST
        je      .chan_win_adj
;        cmp     al, SSH_MSG_CHANNEL_REQUEST
;        je      .chan_req
        cmp     al, SSH_MSG_CHANNEL_EOF
        je      .chan_eof
        cmp     al, SSH_MSG_CHANNEL_CLOSE
        je      .chan_close

        DEBUGF  3, "SSH: Message type: %u\n", al

  .ret:
        ret

  .disc:
        DEBUGF  3, "SSH: Disconnect message received\n"
        mov     eax, SSHLIB_ERR_DISCONNECTING
        ret

  .ign:
        DEBUGF  3, "SSH: Ignore MSG received\n"
        jmp     .recv

  .dbg:
        DEBUGF  3, "SSH: Debug MSG received\n"
        ;TODO
        jmp     .recv

  .glob_req:
        add     esi, 4
        DEBUGF  3, "SSH: Global MSG received: %s\n", esi
        ;TODO
        jmp     .recv

  .chan_win_adj:
        mov     eax, dword[esi]
        bswap   eax
        mov     [ssh_chan.snd_wnd], eax
        ; TODO: validate channel number, act accordingly
        DEBUGF  3, "SSH: Channel %u window update received\n", eax
        jmp     .recv

  .chan_eof:
        mov     eax, dword[esi]
        bswap   eax
        ; TODO: validate channel number, act accordingly
        DEBUGF  3, "SSH: Channel %u EOF received\n", eax
        jmp     .recv

  .chan_close:
        mov     eax, dword[esi]
        bswap   eax
        ; TODO: validate channel number
        DEBUGF  3, "SSH: Channel %u close received\n", eax
        ; Reply with close message
        stdcall sshlib_send_packet, [con_ptr], ssh_msg_channel_close, ssh_msg_channel_close.length, 0
        xor     eax, eax
        ret

endp
