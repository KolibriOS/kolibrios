;    ssh.asm - SSH client for KolibriOS
;
;    Copyright (C) 2015-2016 Jeffrey Amelynck
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
__DEBUG_LEVEL__ = 1

BUFFERSIZE      = 4096
MAX_BITS        = 8192

DH_PRIVATE_KEY_SIZE     = 256

use32

        db      'MENUET01'      ; signature
        dd      1               ; header version
        dd      start           ; entry point
        dd      i_end           ; initialized size
        dd      mem+4096        ; required memory
        dd      mem+4096        ; stack pointer
        dd      hostname        ; parameters
        dd      0               ; path

include '../../macros.inc'
purge mov,add,sub
include '../../proc32.inc'
include '../../dll.inc'
include '../../debug-fdo.inc'
include '../../network.inc'
;include '../../develop/libraries/libcrash/trunk/libcrash.inc'

include 'mcodes.inc'
include 'ssh_transport.inc'
include 'dh_gex.inc'

include 'mpint.inc'
include 'random.inc'
include 'aes256.inc'
include 'aes256-ctr.inc'
include 'aes256-cbc.inc'
include '../../fs/kfar/trunk/kfar_arc/sha256.inc'

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

start:
        mcall   68, 11          ; Init heap

        DEBUGF  1, "SSH: Loading libraries\n"
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

        DEBUGF  1, "SSH: Init PRNG\n"
        call    init_random

        DEBUGF  1, "SSH: Init Console\n"
        invoke  con_start, 1
        invoke  con_init, 80, 25, 80, 25, title

; Check for parameters
        cmp     byte[hostname], 0
        jne     resolve

main:
        invoke  con_cls
; Welcome user
        invoke  con_write_asciiz, str1

prompt:
; write prompt
        invoke  con_write_asciiz, str2
; read string
        mov     esi, hostname
        invoke  con_gets, esi, 256
; check for exit
        test    eax, eax
        jz      done
        cmp     byte[esi], 10
        jz      done

resolve:
        mov     [sockaddr1.port], 22 shl 8

; delete terminating '\n'
        mov     esi, hostname
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
        lea     ebx, [ebx*4 + ebx]
        shl     ebx, 1
        add     ebx, eax
        jmp     .portloop

  .port_done:
        xchg    bl, bh
        mov     [sockaddr1.port], bx

  .done:

; resolve name
        push    esp     ; reserve stack place
        push    esp
        invoke  getaddrinfo, hostname, 0, 0
        pop     esi
; test for error
        test    eax, eax
        jnz     dns_error

        invoke  con_cls
        invoke  con_write_asciiz, str3
        invoke  con_write_asciiz, hostname

; write results
        invoke  con_write_asciiz, str8

; convert IP address to decimal notation
        mov     eax, [esi+addrinfo.ai_addr]
        mov     eax, [eax+sockaddr_in.sin_addr]
        mov     [sockaddr1.ip], eax
        invoke  inet_ntoa, eax
; write result
        invoke  con_write_asciiz, eax
; free allocated memory
        invoke  freeaddrinfo, esi

        invoke  con_write_asciiz, str9

        mcall   40, EVM_STACK + EVM_KEY
        invoke  con_cls

; Create socket
        mcall   socket, AF_INET4, SOCK_STREAM, 0
        cmp     eax, -1
        jz      socket_err
        mov     [socketnum], eax

; Connect
        mcall   connect, [socketnum], sockaddr1, 18
        test    eax, eax
        jnz     socket_err

; Start calculating hash meanwhile
        call    sha256_init
; HASH: string  V_C, the client's version string (CR and NL excluded)
        mov     esi, ssh_ident_ha
        mov     edx, ssh_ident.length+4-2
        call    sha256_update

; Send our identification string
        DEBUGF  1, "Sending ID string\n"
        mcall   send, [socketnum], ssh_ident, ssh_ident.length, 0
        cmp     eax, -1
        je      socket_err

; Check protocol version of server
        mcall   recv, [socketnum], rx_buffer, BUFFERSIZE, 0
        cmp     eax, -1
        je      socket_err

        DEBUGF  1, "Received ID string\n"
        cmp     dword[rx_buffer], "SSH-"
        jne     proto_err
        cmp     dword[rx_buffer+4], "2.0-"
        jne     proto_err

; HASH: string  V_S, the server's version string (CR and NL excluded)
        lea     edx, [eax+2]
        sub     eax, 2
        bswap   eax
        mov     [rx_buffer-4], eax
        mov     esi, rx_buffer-4
        call    sha256_update

; Key Exchange init
        DEBUGF  1, "Sending KEX init\n"
        mov     edi, ssh_kex.cookie
        call    MBRandom
        stosd
        call    MBRandom
        stosd
        call    MBRandom
        stosd
        call    MBRandom
        stosd
        stdcall ssh_send_packet, [socketnum], ssh_kex, ssh_kex.length, 0
        cmp     eax, -1
        je      socket_err

; HASH: string  I_C, the payload of the client's SSH_MSG_KEXINIT
        mov     eax, [tx_buffer+ssh_header.length]
        bswap   eax
        movzx   ebx, [tx_buffer+ssh_header.padding]
        sub     eax, ebx
        dec     eax
        lea     edx, [eax+4]
        bswap   eax
        mov     [tx_buffer+1], eax
        mov     esi, tx_buffer+1
        call    sha256_update

; Check key exchange init of server
        stdcall ssh_recv_packet, [socketnum], rx_buffer, BUFFERSIZE, 0
        cmp     eax, -1
        je      socket_err

        cmp     [rx_buffer+ssh_header.message_code], SSH_MSG_KEXINIT
        jne     proto_err
        DEBUGF  1, "Received KEX init\n"

        lea     esi, [rx_buffer+sizeof.ssh_header+16]
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

        ; TODO

; HASH: string I_S, the payload of the servers's SSH_MSG_KEXINIT
        mov     eax, [rx_buffer+ssh_header.length]
        movzx   ebx, [rx_buffer+ssh_header.padding]
        sub     eax, ebx
        dec     eax
        lea     edx, [eax+4]
        bswap   eax
        mov     [rx_buffer+sizeof.ssh_header-5], eax
        mov     esi, rx_buffer+sizeof.ssh_header-5
        call    sha256_update

; Exchange keys with the server
        stdcall dh_gex
        test    eax, eax
        jnz     exit

; Set keys
        DEBUGF  1, "SSH: Init encryption\n"
        stdcall aes256_cbc_init, rx_iv
        mov     [rx_context], eax
        stdcall aes256_set_encrypt_key, [rx_context], rx_enc_key
        mov     [decrypt_proc], aes256_cbc_decrypt
        mov     [rx_blocksize], 32

        DEBUGF  1, "SSH: Init decryption\n"
        stdcall aes256_cbc_init, tx_iv
        mov     [tx_context], eax
        stdcall aes256_set_decrypt_key, [tx_context], tx_enc_key
        mov     [encrypt_proc], aes256_cbc_encrypt
        mov     [tx_blocksize], 32

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

        stdcall ssh_recv_packet, [socketnum], rx_buffer, BUFFERSIZE, 0
        cmp     eax, -1
        je      closed

        DEBUGF  1, 'SSH: got %u bytes of data !\n', eax

        mov     esi, rx_buffer
        mov     ecx, eax
        pusha
@@:
        lodsb
        DEBUGF  1, "%x ", eax:2
        dec     ecx
        jnz     @r
        popa
        lea     edi, [esi + eax]
        mov     byte [edi], 0
        invoke  con_write_asciiz, esi
        jmp     mainloop

proto_err:
        DEBUGF  1, "SSH: protocol error\n"
        invoke  con_write_asciiz, str7
        jmp     prompt

socket_err:
        DEBUGF  1, "SSH: socket error %d\n", ebx
        invoke  con_write_asciiz, str6
        jmp     prompt

dns_error:
        DEBUGF  1, "SSH: DNS error %d\n", eax
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
        DEBUGF  1, "SSH: Exiting\n"
        mcall   close, [socketnum]
        mcall   -1


thread:
        mcall   40, 0
  .loop:
        invoke  con_getch2
        mov     [send_data], ax
        xor     esi, esi
        inc     esi
        test    al, al
        jnz     @f
        inc     esi
  @@:
        stdcall ssh_send_packet, [socketnum], send_data, 0

        invoke  con_get_flags
        test    eax, 0x200                      ; con window closed?
        jz      .loop
        mcall   -1

; data
title   db      'Secure Shell',0
str1    db      'SSH client for KolibriOS',10,10,\
                'Please enter URL of SSH server (host:port)',10,10,0
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

sockaddr1:
        dw AF_INET4
  .port dw 0
  .ip   dd 0
        rb 10

ssh_ident_ha:
        dd_n (ssh_ident.length-2)
ssh_ident:
        db "SSH-2.0-KolibriOS_SSH_0.01",13,10
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
        db "aes256-cbc"                 ;,aes256-ctr,aes256-cbc,rijndael-cbc@lysator.liu.se,aes192-ctr,aes192-cbc,aes128-ctr,aes128-cbc,blowfish-ctr,blowfish-cbc,3des-ctr,3des-cbc,arcfour256,arcfour128"
  .encryption_algorithms_server_to_client:
        dd_n .mac_algorithms_client_to_server - .encryption_algorithms_server_to_client - 4
        db "aes256-cbc"                 ;,aes256-ctr,aes256-cbc,rijndael-cbc@lysator.liu.se,aes192-ctr,aes192-cbc,aes128-ctr,aes128-cbc,blowfish-ctr,blowfish-cbc,3des-ctr,3des-cbc,arcfour256,arcfour128"
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
        dd_n 128                ; DH GEX min
        dd_n 256                ; DH GEX number of bits
        dd_n 512                ; DH GEX Max
  .length = $ - ssh_gex_req


ssh_new_keys:
        db SSH_MSG_NEWKEYS
  .length = $ - ssh_new_keys


include_debug_strings


; import
align 4
@IMPORT:

library network, 'network.obj', \
        console, 'console.obj';, \
;        libcrash, 'libcrash.obj'

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
        con_get_flags,  'con_get_flags'

;import  libcrash, \
;        crash.hash, 'crash_hash'

IncludeIGlobals

i_end:

decrypt_proc    dd dummy_encrypt
encrypt_proc    dd dummy_encrypt
rx_blocksize    dd 4
tx_blocksize    dd 4
rx_context      dd ?
tx_context      dd ?

IncludeUGlobals

socketnum       dd ?
rx_packet_length dd ?   ;;;;;
rx_buffer:      rb BUFFERSIZE+1
tx_buffer:      rb BUFFERSIZE+1

send_data       dw ?

hostname        rb 1024

; Diffie Hellman variables
dh_p            dd ?
                rb MAX_BITS/8
dh_g            dd ?
                rb MAX_BITS/8
dh_x            dd ?
                rb MAX_BITS/8
dh_e            dd ?
                rb MAX_BITS/8
dh_f            dd ?
                rb MAX_BITS/8

dh_signature    dd ?
                rb MAX_BITS/8

; Output from key exchange
dh_K            dd ?            ; Shared Secret (Big endian)
                rb MAX_BITS/8
  .length       dd ?            ; Length in little endian

dh_H            rb 32           ; Exchange Hash
session_id      rb 32
rx_iv           rb 32           ; Rx initialisation vector
tx_iv           rb 32           ; Tx initialisation vector
rx_enc_key      rb 32           ; Rx encryption key
tx_enc_key      rb 32           ; Tx encryption key
rx_int_key      rb 32           ; Rx integrity key
tx_int_key      rb 32           ; Tx integrity key

; Temporary values      ; To be removed
mpint_tmp       rb MPINT_MAX_LEN+4

mem:
