;    ssh.asm - SSH client for KolibriOS
;
;    Copyright (C) 2015-2021 Jeffrey Amelynck
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

format binary as ""

__DEBUG__       = 1
__DEBUG_LEVEL__ = 3             ; 1: Everything, including sinsitive information, 2: Debugging, 3: Errors only

BUFFERSIZE      = 4096
MAX_BITS        = 8192

DH_PRIVATE_KEY_SIZE     = 256

use32

        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem+65536       ; required memory
        dd      mem+65536       ; stack pointer
        dd      params          ; parameters
        dd      0               ; path

include '../../macros.inc'
;include '../../struct.inc'
purge mov,add,sub
include '../../proc32.inc'
include '../../dll.inc'
include '../../debug-fdo.inc'
include '../../network.inc'
include '../../develop/libraries/libcrash/trunk/libcrash.inc'

include 'mcodes.inc'
include 'ssh_transport.inc'

include 'dh_gex.inc'

include 'mpint.inc'
include 'seed.inc'
include 'random.inc'

include 'aes256.inc'
include 'aes256-ctr.inc'
include 'aes256-cbc.inc'

include 'blowfish.inc'
include 'blowfish-ctr.inc'
include 'blowfish-cbc.inc'

include 'hmac_sha256.inc'
include 'hmac_sha1.inc'
include 'hmac_md5.inc'

; macros for network byte order
macro dd_n op {
   dd 0 or (((op) and 0FF000000h) shr 24) or \
           (((op) and 000FF0000h) shr  8) or \
           (((op) and 00000FF00h) shl  8) or \
           (((op) and 0000000FFh) shl 24)
}

macro dw_n op {
   dw 0 or (((op) and 0FF00h) shr 8) or \
           (((op) and 000FFh) shl 8)
}

proc dump_hex _ptr, _length
if __DEBUG_LEVEL__ <= 1
        pushad

        mov     esi, [_ptr]
        mov     ecx, [_length]
  .next_dword:
        lodsd
        bswap   eax
        DEBUGF  1,'%x', eax
        loop    .next_dword
        DEBUGF  1,'\n'

        popad
end if
        ret
endp

struct  ssh_connection

; Connection

        hostname                rb 1024

        socketnum               dd ?

        sockaddr                dw ?            ; Address family
        port                    dw ?
        ip                      dd ?
                                rb 10

; Encryption/Decryption

        rx_crypt_proc           dd ?
        tx_crypt_proc           dd ?
        rx_crypt_ctx_ptr        dd ?
        tx_crypt_ctx_ptr        dd ?
        rx_crypt_blocksize      dd ?
        tx_crypt_blocksize      dd ?

; Padding

;        rx_padsize              dd ?    ; = Max(8, rx_crypt_blocksize)
        tx_pad_size             dd ?    ; = Max(8, tx_crypt_blocksize)
        tx_pad_proc             dd ?

; Message authentication

        rx_mac_proc             dd ?
        tx_mac_proc             dd ?
        rx_mac_ctx              hmac_sha256_context
        tx_mac_ctx              hmac_sha256_context
        rx_mac_length           dd ?
        tx_mac_length           dd ?

; Buffers

        rx_seq                  dd ?            ; Packet sequence number for MAC
        rx_buffer               ssh_packet_header
                                rb BUFFERSIZE-sizeof.ssh_packet_header

        tx_seq                  dd ?            ; Packet sequence number for MAC
        tx_buffer               ssh_packet_header
                                rb BUFFERSIZE-sizeof.ssh_packet_header

        send_data               dw ?

; Output from key exchange
        dh_K                    dd ?            ; Shared Secret (Big endian)
                                rb MAX_BITS/8
        dh_K_length             dd ?            ; Length in little endian

        dh_H                    rb 32           ; Exchange Hash
        session_id_prefix       db ?
        session_id              rb 32
        rx_iv                   rb 32           ; Rx initialisation vector
        tx_iv                   rb 32           ; Tx initialisation vector
        rx_enc_key              rb 32           ; Rx encryption key
        tx_enc_key              rb 32           ; Tx encryption key
        rx_int_key              rb 32           ; Rx integrity key
        tx_int_key              rb 32           ; Tx integrity key

; Diffie Hellman
        dh_p                    dd ?
                                rb MAX_BITS/8
        dh_g                    dd ?
                                rb MAX_BITS/8
        dh_x                    dd ?
                                rb MAX_BITS/8
        dh_e                    dd ?
                                rb MAX_BITS/8
        dh_f                    dd ?
                                rb MAX_BITS/8

        dh_signature            dd ?
                                rb MAX_BITS/8

        temp_ctx                crash_ctx
        k_h_ctx                 crash_ctx

        mpint_tmp               dd ?
                                rb MAX_BITS/8

ends

start:
        mcall   68, 11          ; Init heap

        DEBUGF  2, "SSH: Loading libraries\n"
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

        DEBUGF  2, "SSH: Init PRNG\n"
        call    create_seed
        call    init_random

        DEBUGF  2, "SSH: Init Console\n"
        invoke  con_start, 1
        invoke  con_init, 80, 25, 80, 25, title

; Check for parameters TODO
;        cmp     byte[params], 0
;        jne     resolve

main:
        invoke  con_cls
; Welcome user
        invoke  con_write_asciiz, str1

prompt:
; write prompt
        invoke  con_write_asciiz, str2
; read string
        mov     esi, con.hostname
        invoke  con_gets, esi, 256
; check for exit
        test    eax, eax
        jz      done
        cmp     byte[esi], 10
        jz      done

resolve:
        mov     [con.sockaddr], AF_INET4
        mov     [con.port], 22 shl 8

; delete terminating '\n'
        mov     esi, con.hostname
  @@:
        lodsb
        cmp     al, ':'
        je      .do_port
        cmp     al, 0x20
        ja      @r
        mov     byte[esi-1], 0
        jmp     .done

  .do_port:
        xor     eax, eax
        xor     ebx, ebx
        mov     byte[esi-1], 0
  .portloop:
        lodsb
        cmp     al, 0x20
        jbe     .port_done
        sub     al, '0'
        jb      hostname_error
        cmp     al, 9
        ja      hostname_error
        lea     ebx, [ebx*4+ebx]
        shl     ebx, 1
        add     ebx, eax
        jmp     .portloop

  .port_done:
        xchg    bl, bh
        mov     [con.port], bx

  .done:

; resolve name
        push    esp     ; reserve stack place
        push    esp
        invoke  getaddrinfo, con.hostname, 0, 0
        pop     esi
; test for error
        test    eax, eax
        jnz     dns_error

        invoke  con_write_asciiz, str3
        invoke  con_write_asciiz, con.hostname

; write results
        invoke  con_write_asciiz, str8

; convert IP address to decimal notation
        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [con.ip], eax
        invoke  inet_ntoa, eax
; write result
        invoke  con_write_asciiz, eax
; free allocated memory
        invoke  freeaddrinfo, esi

        invoke  con_write_asciiz, str9

        mcall   40, EVM_STACK + EVM_KEY

; Create socket
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        jz      socket_err
        mov     [con.socketnum], eax

; Connect
        DEBUGF  2, "Connecting to server\n"
        mcall   connect, [con.socketnum], con.sockaddr, 18
        test    eax, eax
        jnz     socket_err

; Start calculating hash
        invoke  sha256_init, con.temp_ctx
; HASH: string  V_C, the client's version string (CR and NL excluded)
        invoke  sha256_update, con.temp_ctx, ssh_ident_ha, ssh_ident.length+4-2

; >> Send our identification string
        DEBUGF  2, "Sending ID string\n"
        mcall   send, [con.socketnum], ssh_ident, ssh_ident.length, 0
        cmp     eax, -1
        je      socket_err

; << Check protocol version of server
        mcall   recv, [con.socketnum], con.rx_buffer, BUFFERSIZE, 0
        cmp     eax, -1
        je      socket_err

        DEBUGF  2, "Received ID string\n"
        cmp     dword[con.rx_buffer], "SSH-"
        jne     proto_err
        cmp     dword[con.rx_buffer+4], "2.0-"
        jne     proto_err

; HASH: string  V_S, the server's version string (CR and NL excluded)
        lea     edx, [eax+2]
        sub     eax, 2
        bswap   eax
        mov     dword[con.rx_buffer-4], eax
        invoke  sha256_update, con.temp_ctx, con.rx_buffer-4, edx

; >> Key Exchange init
        mov     [con.rx_seq], 0
        mov     [con.tx_seq], 0
        mov     [con.rx_crypt_blocksize], 4             ; minimum blocksize
        mov     [con.tx_crypt_blocksize], 4
        mov     [con.rx_crypt_proc], 0
        mov     [con.tx_crypt_proc], 0
        mov     [con.rx_mac_proc], 0
        mov     [con.tx_mac_proc], 0
        mov     [con.rx_mac_length], 0
        mov     [con.tx_mac_length], 0
;        mov     [con.rx_padsize], 8                     ; minimum padsize
        mov     [con.tx_pad_size], 8
        mov     [con.tx_pad_proc], padding_zero

        DEBUGF  2, "Sending KEX init\n"
        mov     edi, ssh_kex.cookie
        call    MBRandom
        stosd
        call    MBRandom
        stosd
        call    MBRandom
        stosd
        call    MBRandom
        stosd
        stdcall ssh_send_packet, con, ssh_kex, ssh_kex.length, 0
        cmp     eax, -1
        je      socket_err

; HASH: string  I_C, the payload of the client's SSH_MSG_KEXINIT
        mov     eax, dword[con.tx_buffer+ssh_packet_header.packet_length]
        bswap   eax
        movzx   ebx, [con.tx_buffer+ssh_packet_header.padding_length]
        sub     eax, ebx
        dec     eax
        lea     edx, [eax+4]
        bswap   eax
        mov     dword[con.tx_buffer+1], eax
        invoke  sha256_update, con.temp_ctx, con.tx_buffer+1, edx

; << Check key exchange init of server
        stdcall ssh_recv_packet, con, 0
        cmp     eax, -1
        je      socket_err

        cmp     [con.rx_buffer.message_code], SSH_MSG_KEXINIT
        jne     proto_err
        DEBUGF  2, "Received KEX init\n"

        lea     esi, [con.rx_buffer+sizeof.ssh_packet_header+16]
        lodsd
        bswap   eax
        DEBUGF  1, "kex_algorithms: %s\n", esi
        add     esi, eax
        lodsd
        bswap   eax
        DEBUGF  1, "server_host_key_algorithms: %s\n", esi
        add     esi, eax
        lodsd
        bswap   eax
        DEBUGF  1, "encryption_algorithms_client_to_server: %s\n", esi
        add     esi, eax
        lodsd
        bswap   eax
        DEBUGF  1, "encryption_algorithms_server_to_client: %s\n", esi
        add     esi, eax
        lodsd
        bswap   eax
        DEBUGF  1, "mac_algorithms_client_to_server: %s\n", esi
        add     esi, eax
        lodsd
        bswap   eax
        DEBUGF  1, "mac_algorithms_server_to_client: %s\n", esi
        add     esi, eax
        lodsd
        bswap   eax
        DEBUGF  1, "compression_algorithms_client_to_server: %s\n", esi
        add     esi, eax
        lodsd
        bswap   eax
        DEBUGF  1, "compression_algorithms_server_to_client: %s\n", esi
        add     esi, eax
        lodsd
        bswap   eax
        DEBUGF  1, "languages_client_to_server: %s\n", esi
        add     esi, eax
        lodsd
        bswap   eax
        DEBUGF  1, "languages_server_to_client: %s\n", esi
        add     esi, eax
        lodsb
        DEBUGF  1, "KEX First Packet Follows: %u\n", al

; TODO: parse this structure and init procedures accordingly

; HASH: string I_S, the payload of the servers's SSH_MSG_KEXINIT
        mov     eax, dword[con.rx_buffer+ssh_packet_header.packet_length]
        movzx   ebx, [con.rx_buffer+ssh_packet_header.padding_length]
        sub     eax, ebx
        dec     eax
        lea     edx, [eax+4]
        bswap   eax
        mov     dword[con.rx_buffer+sizeof.ssh_packet_header-5], eax
        invoke  sha256_update, con.temp_ctx, con.rx_buffer+sizeof.ssh_packet_header-5, edx

; Exchange keys with the server

; TODO: host verification

        stdcall dh_gex
        test    eax, eax
        jnz     exit

; Set keys and initialize transport subroutines

        DEBUGF  2, "SSH: Setting encryption keys\n"

        stdcall aes256_ctr_init, con.rx_iv
        mov     [con.rx_crypt_ctx_ptr], eax

        stdcall aes256_set_encrypt_key, eax, con.rx_enc_key
        mov     [con.rx_crypt_proc], aes256_ctr_crypt
        mov     [con.rx_crypt_blocksize], AES256_BLOCKSIZE
;        mov     [con.rx_pad_size], AES256_BLOCKSIZE

        stdcall aes256_ctr_init, con.tx_iv
        mov     [con.tx_crypt_ctx_ptr], eax

        stdcall aes256_set_encrypt_key, eax, con.tx_enc_key
        mov     [con.tx_crypt_proc], aes256_ctr_crypt
        mov     [con.tx_crypt_blocksize], AES256_BLOCKSIZE

        mov     [con.tx_pad_size], AES256_BLOCKSIZE
        mov     [con.tx_pad_proc], MBRandom

        stdcall hmac_sha256_setkey, con.rx_mac_ctx, con.rx_int_key, SHA256_HASH_SIZE
        mov     [con.rx_mac_proc], hmac_sha256
        mov     [con.rx_mac_length], SHA256_HASH_SIZE

        stdcall hmac_sha256_setkey, con.tx_mac_ctx, con.tx_int_key, SHA256_HASH_SIZE
        mov     [con.tx_mac_proc], hmac_sha256
        mov     [con.tx_mac_length], SHA256_HASH_SIZE

; Re-seed RNG for padding bytes
        call    create_seed
        call    init_random

; TODO: erase all keys from memory and free the memory

; >> Request service (user-auth)

        DEBUGF  2, "SSH: Requesting service\n"

        stdcall ssh_send_packet, con, ssh_request_service, ssh_request_service.length, 0
        cmp     eax, -1
        je      socket_err

; << Check for service acceptance

        stdcall ssh_msg_handler, con, 0
        cmp     eax, -1
        je      socket_err

        cmp     [con.rx_buffer.message_code], SSH_MSG_SERVICE_ACCEPT
        jne     proto_err

; >> Request user authentication

        DEBUGF  2, "SSH: User authentication\n"

        mcall   68, 12, 1024    ; FIXME
        test    eax, eax
        jz      done            ; FIXME
        mov     edi, eax
        mov     ebx, eax
        mov     byte[edi], SSH_MSG_USERAUTH_REQUEST
        inc     edi

        ; Get username
        add     edi, 4
        invoke  con_write_asciiz, str12
        invoke  con_gets, edi, 256      ; FIXME
        test    eax, eax
        jz      done            ; FIXME

        mov     edx, eax
        mov     ecx, 256
        xor     al, al
        repne   scasb

        dec     edi             ; \0
        dec     edi             ; \n
        push    edi
        sub     edi, edx
        bswap   edi
        mov     [edx-4], edi
        pop     edi

        mov     dword[edi], 0x0e000000  ; 14 Bswapped
        mov     dword[edi+4], "ssh-"
        mov     dword[edi+8], "conn"
        mov     dword[edi+12], "ecti"
        mov     word[edi+16], "on"
        add     edi, 18

        mov     dword[edi], 0x08000000  ; 8 Bswapped
        mov     dword[edi+4], "pass"
        mov     dword[edi+8], "word"

        mov     byte[edi+12], 0         ; bool
        add     edi, 13

        ; Get password
        add     edi, 4
        invoke  con_write_asciiz, str13
        push    eax
        invoke  con_gets, edi, 256      ; FIXME
        test    eax, eax
        jz      done            ; FIXME

        mov     edx, eax
        mov     ecx, 256
        xor     al, al
        repne scasb

        dec     edi             ; \0
        dec     edi             ; \n
        push    edi
        sub     edi, edx
        bswap   edi
        mov     [edx-4], edi
        pop     edi
        sub     edi, ebx

        push    ebx
        stdcall ssh_send_packet, con, ebx, edi, 0

        ; Clear used buffer and free
        pop     edx
        mov     edi, edx
        push    eax
        mov     ecx, 1024/4     ; FIXME
        xor     eax, eax
        rep stosd
        mcall   68, 13, edx
        pop     eax

        cmp     eax, -1
        je      socket_err

        invoke  con_write_asciiz, str14

; << Check for userauth acceptance

        stdcall ssh_msg_handler, con, 0
        cmp     eax, -1
        je      socket_err

        cmp     [con.rx_buffer.message_code], SSH_MSG_USERAUTH_SUCCESS
        jne     proto_err

; >> Open channel

        DEBUGF  2, "SSH: Open channel\n"

        stdcall ssh_send_packet, con, ssh_channel_open, ssh_channel_open.length, 0
        cmp     eax, -1
        je      socket_err

; << Check for channel open confirmation

        stdcall ssh_msg_handler, con, 0
        cmp     eax, -1
        je      socket_err

        cmp     [con.rx_buffer.message_code], SSH_MSG_CHANNEL_OPEN_CONFIRMATION
        jne     proto_err

; >> Channel request: pty

        DEBUGF  2, "SSH: Request pty\n"

        stdcall ssh_send_packet, con, ssh_channel_request, ssh_channel_request.length, 0
        cmp     eax, -1
        je      socket_err

; << Check for channel request confirmation

        stdcall ssh_msg_handler, con, 0
        cmp     eax, -1
        je      socket_err

        cmp     [con.rx_buffer.message_code], SSH_MSG_CHANNEL_SUCCESS
        jne     proto_err

; >> Channel request: shell

        DEBUGF  2, "SSH: Request shell\n"

        stdcall ssh_send_packet, con, ssh_shell_request, ssh_shell_request.length, 0
        cmp     eax, -1
        je      socket_err

; << Check for channel request confirmation (FIXME: this may not be first packet!)

; TODO
;
;        stdcall ssh_msg_handler, con, 0
;        cmp     eax, -1
;        je      socket_err

;        cmp     [con.rx_buffer.message_code], SSH_MSG_CHANNEL_SUCCESS
;        jne     proto_err

; Launch network thread
        mcall   18, 7
        push    eax
        mcall   51, 1, thread, mem - 2048
        pop     ecx
        mcall   18, 3

mainloop:
        call    [con_get_flags]
        test    eax, 0x200                      ; con window closed?
        jnz     exit

        stdcall ssh_msg_handler, con, 0
        cmp     eax, 0
        jbe     closed

        cmp     [con.rx_buffer.message_code], SSH_MSG_CHANNEL_DATA
        jne     .dump

        mov     eax, dword[con.rx_buffer.message_code+5]
        bswap   eax
        DEBUGF  1, 'SSH: got %u bytes of data !\n', eax

        lea     esi, [con.rx_buffer.message_code+5+4]
        mov     ecx, eax
        lea     edi, [esi + eax]
        mov     byte [edi], 0
        invoke  con_write_asciiz, esi
        jmp     mainloop

  .dump:
        lea     esi, [con.rx_buffer]
        mov     ecx, eax
        pusha
@@:
        lodsb
        DEBUGF  1, "%x ", eax:2
        dec     ecx
        jnz     @r
        popa
        DEBUGF  1, "\n"
        jmp     mainloop


proto_err:
        mov     eax, con.rx_buffer
        int3

        DEBUGF  3, "SSH: protocol error\n"
        invoke  con_write_asciiz, str7
        jmp     prompt

socket_err:
        DEBUGF  3, "SSH: socket error %d\n", ebx
        invoke  con_write_asciiz, str6
        jmp     prompt

dns_error:
        DEBUGF  3, "SSH: DNS error %d\n", eax
        invoke  con_write_asciiz, str5
        jmp     prompt

hostname_error:
        invoke  con_write_asciiz, str10
        jmp     prompt

closed:
        invoke  con_write_asciiz, str11
        jmp     prompt

done:
        invoke  con_exit, 1
exit:
        DEBUGF  3, "SSH: Exiting\n"
        mcall   close, [con.socketnum]
        mcall   -1


thread:
        mcall   40, 0
  .loop:
        invoke  con_getch2
        mov     [ssh_channel_data+9], al
        stdcall ssh_send_packet, con, ssh_channel_data, ssh_channel_data.length, MSG_DONTWAIT

        invoke  con_get_flags
        test    eax, 0x200                      ; con window closed?
        jz      .loop
        mcall   -1


; Handle common messages and return from specific ones
proc ssh_msg_handler, con, flags

  .recv:
        stdcall ssh_recv_packet, [con], [flags]
        cmp     eax, -1
        je      .ret

        cmp     [con.rx_buffer.message_code], SSH_MSG_DISCONNECT
        je      .disc
        cmp     [con.rx_buffer.message_code], SSH_MSG_IGNORE
        je      .ign
        cmp     [con.rx_buffer.message_code], SSH_MSG_DEBUG
        je      .dbg
        cmp     [con.rx_buffer.message_code], SSH_MSG_GLOBAL_REQUEST
        je      .glob

  .ret:
        ret

  .disc:
        mov     eax, -1
        ret

  .ign:
        jmp     .recv

  .dbg:
  .glob:
        ; TODO

        jmp     .recv

endp

; data
title   db      'Secure Shell',0
str1    db      'SSH client for KolibriOS',10,10,\
                'Please enter URL of SSH server (hostname:port)',10,10,0
str2    db      '> ',0
str3    db      'Connecting to ',0
str4    db      10,0
str5    db      'Name resolution failed.',10,10,0
str6    db      'A socket error occured.',10,10,0
str7    db      'A protocol error occured.',10,10,0
str8    db      ' (',0
str9    db      ')',10,0
str10   db      'Invalid hostname.',10,10,0
str11   db      10,'Remote host closed the connection.',10,10,0
str12   db      'Login as: ',0
str13   db      'Password: ', 27, '[?25l', 27, '[30;40m', 0
str14   db      10, 27, '[?25h', 27, '[0m', 0

ssh_ident_ha:
        dd_n (ssh_ident.length-2)
ssh_ident:
        db "SSH-2.0-KolibriOS_SSH_0.04",13,10
  .length = $ - ssh_ident

ssh_kex:
        db SSH_MSG_KEXINIT
  .cookie:
        rd 4
  .kex_algorithms:
        dd_n .server_host_key_algorithms - .kex_algorithms - 4
        db "diffie-hellman-group-exchange-sha256" ; diffie-hellman-group-exchange-sha1
  .server_host_key_algorithms:
        dd_n .encryption_algorithms_client_to_server - .server_host_key_algorithms - 4
        db "ssh-rsa"                    ;,ssh-dss
  .encryption_algorithms_client_to_server:
        dd_n .encryption_algorithms_server_to_client - .encryption_algorithms_client_to_server - 4
        db "aes256-ctr"                 ;,aes256-cbc,aes256-cbc,rijndael-cbc@lysator.liu.se,aes192-ctr,aes192-cbc,aes128-ctr,aes128-cbc,blowfish-ctr,blowfish-cbc,3des-ctr,3des-cbc,arcfour256,arcfour128"
  .encryption_algorithms_server_to_client:
        dd_n .mac_algorithms_client_to_server - .encryption_algorithms_server_to_client - 4
        db "aes256-ctr"                 ;,aes256-cbc,aes256-cbc,rijndael-cbc@lysator.liu.se,aes192-ctr,aes192-cbc,aes128-ctr,aes128-cbc,blowfish-ctr,blowfish-cbc,3des-ctr,3des-cbc,arcfour256,arcfour128"
  .mac_algorithms_client_to_server:
        dd_n .mac_algorithms_server_to_client - .mac_algorithms_client_to_server - 4
        db "hmac-sha2-256"              ;,hmac-sha1,hmac-sha1-96,hmac-md5"
  .mac_algorithms_server_to_client:
        dd_n .compression_algorithms_client_to_server - .mac_algorithms_server_to_client - 4
        db "hmac-sha2-256"              ;,hmac-sha1,hmac-sha1-96,hmac-md5"
  .compression_algorithms_client_to_server:
        dd_n .compression_algorithms_server_to_client - .compression_algorithms_client_to_server - 4
        db "none"                       ;,zlib"
  .compression_algorithms_server_to_client:
        dd_n .languages_client_to_server - .compression_algorithms_server_to_client - 4
        db "none"                       ;,zlib"
  .languages_client_to_server:
        dd_n .languages_server_to_client - .languages_client_to_server - 4
        db ""
  .languages_server_to_client:
        dd_n .first_kex_packet_follows - .languages_server_to_client - 4
        db ""
  .first_kex_packet_follows:
        db 0
  .reserved:
        dd_n 0
  .length = $ - ssh_kex


ssh_gex_req:
        db SSH_MSG_KEX_DH_GEX_REQUEST
        dd_n 4096/4                      ; DH GEX min
        dd_n 4096/2                      ; DH GEX number of bits
        dd_n 4096                        ; DH GEX Max
  .length = $ - ssh_gex_req


ssh_new_keys:
        db SSH_MSG_NEWKEYS
  .length = $ - ssh_new_keys


ssh_request_service:
        db SSH_MSG_SERVICE_REQUEST
        dd_n 12                         ; String length
        db "ssh-userauth"               ; Service name
  .length = $ - ssh_request_service


ssh_request_userauth:
        db SSH_MSG_USERAUTH_REQUEST
        dd_n 9
        db "user123"                    ; user name in ISO-10646 UTF-8 encoding [RFC3629]
        dd_n 14
        db "ssh-connection"             ; service name in US-ASCII
        dd_n 8
        db "password"                   ; method name in US-ASCII: none, publickey, password, hostbased
        db 0                            ; bool: false
        dd_n 14
        db "pass123"
  .length = $ - ssh_request_userauth


ssh_channel_open:
        db SSH_MSG_CHANNEL_OPEN
        dd_n 7
        db "session"
        dd_n 0                          ; Sender channel
        dd_n 1024                       ; Initial window size
        dd_n 1024                       ; maximum packet size
  .length = $ - ssh_channel_open

ssh_channel_request:
        db SSH_MSG_CHANNEL_REQUEST
        dd_n 0                          ; Recipient channel
        dd_n 7
        db "pty-req"
        db 1                            ; Bool: want reply
        dd_n 5
        db "xterm"
        dd_n 80                         ; terminal width (rows)
        dd_n 25                         ; terminal height (rows)
        dd_n 0                          ; terminal width (pixels)
        dd_n 0                          ; terminal height (pixels)

        dd_n 0                          ; list of supported opcodes
  .length = $ - ssh_channel_request

ssh_shell_request:
        db SSH_MSG_CHANNEL_REQUEST
        dd_n 0                          ; Recipient channel
        dd_n 5
        db "shell"
        db 1                            ; Bool: want reply
  .length = $ - ssh_shell_request

ssh_channel_data:
        db SSH_MSG_CHANNEL_DATA
        dd_n 0                          ; Sender channel
        dd_n 1
        db ?
  .length = $ - ssh_channel_data


include_debug_strings

align 4
@IMPORT:

library network, 'network.obj', \
        console, 'console.obj', \
        libcrash, 'libcrash.obj'

import  network, \
        getaddrinfo, 'getaddrinfo', \
        freeaddrinfo, 'freeaddrinfo', \
        inet_ntoa, 'inet_ntoa'

import  console, \
        con_start, 'START', \
        con_init, 'con_init', \
        con_write_asciiz, 'con_write_asciiz', \
        con_exit, 'con_exit', \
        con_gets, 'con_gets', \
        con_cls, 'con_cls', \
        con_getch2, 'con_getch2', \
        con_set_cursor_pos, 'con_set_cursor_pos', \
        con_write_string, 'con_write_string', \
        con_get_flags,  'con_get_flags', \
        con_set_flags,  'con_set_flags'

import  libcrash, \
        sha256_init, 'sha256_init', \
        sha256_update, 'sha256_update', \
        sha256_final, 'sha256_final',\
        sha1_init, 'sha1_init', \
        sha1_update, 'sha1_update', \
        sha1_final, 'sha1_final', \
        md5_init, 'md5_init', \
        md5_update, 'md5_update', \
        md5_final, 'md5_final'

IncludeIGlobals

i_end:

IncludeUGlobals

params          rb 1024

con             ssh_connection

mem:
