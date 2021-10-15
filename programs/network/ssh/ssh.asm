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

__DEBUG__               = 1
__DEBUG_LEVEL__         = 2             ; 1: Everything, including sensitive information, 2: Debugging, 3: Errors only

BUFFERSIZE              = 64*1024       ; Must be at least 32K according rfc4253#section-6.1
PACKETSIZE              = 32*1024       ; Must be at least 32K according rfc4253#section-6.1
MAX_BITS                = 8192

DH_PRIVATE_KEY_SIZE     = 256
MAX_INPUT_LENGTH        = 255
MAX_USERNAME_LENGTH     = 256
MAX_PASSWORD_LENGTH     = 256
MAX_HOSTNAME_LENGTH     = 4096
MAX_PUBLIC_KEY_SIZE     = 4096

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
include '../../develop/libraries/libcrash/libcrash.inc'

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

macro str string {
    local .start, .stop

    dd_n (.stop-.start)

    .start db string
    .stop:
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

macro DEBUGM l, s, m {
if __DEBUG__
        DEBUGF  l, s
  if l >=__DEBUG_LEVEL__
        stdcall mpint_print, m
  end if
end if
}

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

include 'sshlib.inc'

include 'sshlib_mcodes.inc'
include 'sshlib_transport.inc'
include 'sshlib_connection.inc'
include 'sshlib_dh_gex.inc'
include 'sshlib_host.inc'
include 'sshlib_channel.inc'
include 'sshlib_userauth.inc'

include 'encodings.inc'         ; Unfortunately, we dont have UTF-8 capable console yet :(

start:
        mcall   68, 11          ; Init heap

        DEBUGF  2, "SSH: Loading libraries\n"
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     main.fail

        DEBUGF  2, "SSH: Init PRNG\n"
        call    create_seed
        call    init_random

        DEBUGF  2, "SSH: Init Console\n"
        invoke  con_start, 1
        invoke  con_init, 80, 25, 80, 250, title

        cmp     byte[params], 0
        jne     main.connect

main:
        invoke  con_cls
; Welcome user
        invoke  con_write_asciiz, str1a
  .prompt:
        invoke  con_write_asciiz, str1b
; Reset window title
        invoke  con_set_title, title
; Write prompt
        invoke  con_write_asciiz, str2
; read string
        mov     esi, params
        invoke  con_gets, esi, MAX_HOSTNAME_LENGTH
; check for exit
        test    eax, eax
        jz      .done
        cmp     byte[esi], 10
        jz      .done

  .connect:
        stdcall sshlib_connect, ssh_con, params
        cmp     eax, 0
        jg      .prompt
        jl      .error

  .login:
        mcall   68, 12, (MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH)
        test    eax, eax
        jz      .done   ; ERR_NOMEM
        mov     esi, eax
        lea     edi, [eax + MAX_USERNAME_LENGTH]

; Get username
        invoke  con_write_asciiz, str12
        invoke  con_gets, esi, MAX_USERNAME_LENGTH
        test    eax, eax
;;        jz      .con_closed_must_clear

; Get password
        invoke  con_write_asciiz, str13a
        invoke  con_gets, edi, MAX_PASSWORD_LENGTH
        test    eax, eax
;;        jz      .con_closed_must_clear
        invoke  con_write_asciiz, str13b

; Authenticate
        stdcall sshlib_userauth_password, ssh_con, esi, edi
; Clear and free username and password
  .clear:
        push    eax
        mov     edx, edi
        xor     eax, eax
        mov     ecx, (MAX_USERNAME_LENGTH + MAX_PASSWORD_LENGTH)/4
        rep     stosd
        mcall   68, 13, edx
        pop     eax

        cmp     eax, 0
        jg      .login          ; Authentication failed
        jl      .error          ; An error occured

; Open a channel
        stdcall sshlib_chan_open, ssh_con
        cmp     eax, 0
        jg      .prompt         ; Authentication failed
        jl      .error          ; An error occured

; Start console input handler thread without deactivating the current window
; Get active window ID
        mcall   18, 7
        push    eax
; Create thread
        mcall   51, 1, con_in_thread, mem - 2048
; Activate window with given ID
        pop     ecx
        mcall   18, 3

  .loop:
        invoke  con_get_flags
        test    eax, 0x200                      ; console window closed?
        jnz     .con_closed

        stdcall sshlib_msg_handler, ssh_con, 0
        cmp     eax, 0
        jle     .check_err

        cmp     [ssh_con.rx_buffer.message_code], SSH_MSG_CHANNEL_DATA
        jne     .dump

        mov     eax, dword[ssh_con.rx_buffer.message_code+5]
        bswap   eax
        DEBUGF  1, 'SSH: got %u bytes of data !\n', eax

        lea     esi, [ssh_con.rx_buffer.message_code+5+4]
        lea     edx, [esi+eax]
        lea     edi, [ssh_con.rx_buffer]
  @@:
        call    get_byte_utf8
        stosb
        cmp     esi, edx
        jb      @r
        xor     al, al
        stosb

        lea     esi, [ssh_con.rx_buffer]
        DEBUGF  3, 'SSH msg: %s\n', esi

        invoke  con_write_asciiz, esi
        jmp     .loop

  .dump:
        DEBUGF  3, "SSH: Unsupported message: "
        lea     esi, [ssh_con.rx_buffer.message_code]
        mov     ecx, eax
        pusha
  @@:
        lodsb
        DEBUGF  3, "%x ", eax:2
        dec     ecx
        jnz     @r
        popa
        DEBUGF  3, "\n"
        jmp     .loop

  .check_err:
        jz      .err_conn_closed
        cmp     ebx, EWOULDBLOCK
        je      .loop
        jmp     .err_sock

  .con_closed:
        ; Send close message on the active channel
        stdcall sshlib_send_packet, ssh_con, ssh_msg_channel_close, ssh_msg_channel_close.length, 0
        jmp     .done

  .error:

; TODO: proper cleanup after error

        cmp     eax, SSHLIB_ERR_NOMEM
        je      .done
        cmp     eax, SSHLIB_ERR_SOCKET
        je      .err_sock
        cmp     eax, SSHLIB_ERR_PROTOCOL
        je      .err_proto
        cmp     eax, SSHLIB_ERR_HOSTNAME
        je      .err_hostname
        cmp     eax, SSHLIB_ERR_HKEY_VERIFY_FAIL
        je      .err_hostkey_fail
        cmp     eax, SSHLIB_ERR_HKEY_SIGNATURE
        je      .err_hostkey_signature
        cmp     eax, SSHLIB_ERR_HKEY_PUBLIC_KEY
        je      .err_hostkey

        jmp     .done


  .err_proto:
;        lea     eax, [ssh_con.rx_buffer]
;        int3
        invoke  con_write_asciiz, str7
        jmp     .prompt

  .err_sock:
        invoke  con_write_asciiz, str6

        mov     eax, str14
        cmp     ebx, ETIMEDOUT
        je      .err_sock_detail
        mov     eax, str15
        cmp     ebx, ECONNREFUSED
        je      .err_sock_detail
        mov     eax, str16
        cmp     ebx, ECONNRESET
        je      .err_sock_detail
        mov     eax, str17
  .err_sock_detail:
        invoke  con_write_asciiz, eax
        jmp     .prompt

  .err_hostname:
        invoke  con_write_asciiz, str10
        jmp     .prompt

  .err_conn_closed:
        invoke  con_write_asciiz, str11
        jmp     .prompt

  .err_hostkey:
        invoke  con_write_asciiz, str19
        jmp     .prompt

  .err_hostkey_signature:
        invoke  con_write_asciiz, str20
        jmp     .prompt

  .err_hostkey_fail:
        invoke  con_write_asciiz, str21
        jmp     .prompt

  .done:
        invoke  con_exit, 1
  .exit:
        DEBUGF  3, "SSH: Exiting\n"
        mcall   close, [ssh_con.socketnum]
  .fail:
        mcall   -1


proc sshlib_callback_connecting, con_ptr, connstring_sz

        invoke  con_write_asciiz, str3
        mov     eax, [con_ptr]
        lea     eax, [eax+sshlib_connection.hostname_sz]
        invoke  con_write_asciiz, eax
        invoke  con_write_asciiz, str8
        invoke  con_write_asciiz, [connstring_sz]
        invoke  con_write_asciiz, str9

        ret
endp


proc sshlib_callback_hostkey_problem, con_ptr, problem_type, hostkey_sz

        cmp     [problem_type], SSHLIB_HOSTKEY_PROBLEM_UNKNOWN
        je      .unknown
        cmp     [problem_type], SSHLIB_HOSTKEY_PROBLEM_MISMATCH
        je      .mismatch

        mov     eax, -1
        ret

  .unknown:
        invoke  con_write_asciiz, str22
        jmp     .ask

  .mismatch:
        invoke  con_write_asciiz, str23
;        jmp     .ask
  .ask:
        invoke  con_write_asciiz, str24a
        invoke  con_write_asciiz, [hostkey_sz]
        invoke  con_write_asciiz, str24b
  .getansw:
        invoke  con_getch2
        or      al, 0x20        ; convert to lowercase
        cmp     al, 'a'
        je      .accept
        cmp     al, 'c'
        je      .once
        cmp     al, 'x'
        je      .refuse
        jmp     .getansw

  .accept:
        mov     eax, SSHLIB_HOSTKEY_ACCEPT
        ret
  .once:
        mov     eax, SSHLIB_HOSTKEY_ONCE
        ret
  .refuse:
        mov     eax, SSHLIB_HOSTKEY_REFUSE
        ret

endp



align 16
con_in_thread:

  .loop:
; TODO: check if channel is still open somehow

        invoke  con_get_input, keyb_input, MAX_INPUT_LENGTH
        test    eax, eax
        jz      .no_input

        mov     ecx, eax
        mov     esi, keyb_input
        mov     edi, ssh_msg_channel_data.data
        call    recode_to_utf8

        lea     eax, [edi - ssh_msg_channel_data.data]
        lea     ecx, [edi - ssh_msg_channel_data]
        bswap   eax
        mov     [ssh_msg_channel_data.len], eax
        stdcall sshlib_send_packet, ssh_con, ssh_msg_channel_data, ecx, 0
        cmp     eax, 0
        jle     .exit

  .no_input:
        invoke  con_get_flags
        test    eax, 0x200                      ; con window closed?
        jz      .loop

  .exit:
        mcall   -1


; data
title   db 'Secure Shell',0
str1a   db 'SSHv2 client for KolibriOS',10,0
str1b   db 10,'Please enter URL of SSH server (hostname:port)',10,0
str2    db '> ',0
str3    db 'Connecting to ',0
str4    db 10,0
str6    db 10, 27, '[2J',27,'[mA network error has occured.',10,0
str7    db 10, 27, '[2J',27,'[mAn SSH protocol error has occured.',10,0
str8    db ' (',0
str9    db ')',10,0
str10   db 'Host does not exist.',10,10,0
str11   db 10, 27, '[2J',27,'[mThe remote host closed the connection.',10,0
str12   db 'Login as: ',0
str13a  db 'Password: ', 27, '[?25l', 27, '[30;40m', 0
str13b  db 10, 27, '[?25h', 27, '[0m', 27, '[2J', 0
str14   db 'The connection timed out',10,0
str15   db 'The connection was refused',10,0
str16   db 'The connection was reset',10,0
str17   db 'No details available',10,0
;str18   db 'User authentication failed',10,0;;;;
str19   db "The remote host's public key is invalid.", 10, 0
str20   db "The remote host's signature is invalid.", 10, 0
str21   db "The remote host failed to verify it's own public key.", 10, 0
str22   db "The host key for the server was not found in the cache.", 10
        db "There is no guarantee to the servers identity !",10, 0

str23   db "The host key provided by the host does not match the cached one.", 10
        db "This may indicate that the remote server has been compromised!", 10, 0

str24a  db 10, "The remote host key is: ", 10, 0
str24b  db 10, 10, "If you trust this host, press A to accept and store the (new) key.", 10
        db "Press C to connect to the host but don't store the (new) key.", 10
        db "Press X to abort.", 10, 0


ssh_ident_ha:
        dd_n (ssh_msg_ident.length-2)
ssh_msg_ident:
        db "SSH-2.0-KolibriOS_SSH_0.09",13,10
  .length = $ - ssh_msg_ident


ssh_msg_kex:
        db SSH_MSG_KEXINIT
  .cookie:
        rd 4
  .kex_algorithms:
        str "diffie-hellman-group-exchange-sha256" ; diffie-hellman-group-exchange-sha1
  .server_host_key_algorithms:
        str "rsa-sha2-512,rsa-sha2-256,ssh-rsa"                    ;,ssh-dss
  .encryption_algorithms_client_to_server:
        str "aes256-ctr"                 ;,aes256-cbc,aes256-cbc,rijndael-cbc@lysator.liu.se,aes192-ctr,aes192-cbc,aes128-ctr,aes128-cbc,blowfish-ctr,blowfish-cbc,3des-ctr,3des-cbc,arcfour256,arcfour128"
  .encryption_algorithms_server_to_client:
        str "aes256-ctr"                 ;,aes256-cbc,aes256-cbc,rijndael-cbc@lysator.liu.se,aes192-ctr,aes192-cbc,aes128-ctr,aes128-cbc,blowfish-ctr,blowfish-cbc,3des-ctr,3des-cbc,arcfour256,arcfour128"
  .mac_algorithms_client_to_server:
        str "hmac-sha2-256"              ;,hmac-sha1,hmac-sha1-96,hmac-md5"
  .mac_algorithms_server_to_client:
        str "hmac-sha2-256"              ;,hmac-sha1,hmac-sha1-96,hmac-md5"
  .compression_algorithms_client_to_server:
        str "none"                       ;,zlib"
  .compression_algorithms_server_to_client:
        str "none"                       ;,zlib"
  .languages_client_to_server:
        str ""
  .languages_server_to_client:
        str ""
  .first_kex_packet_follows:
        db 0
  .reserved:
        dd_n 0
  .length = $ - ssh_msg_kex


ssh_msg_gex_req:
        db SSH_MSG_KEX_DH_GEX_REQUEST
        dd_n 4096/4                      ; DH GEX min
        dd_n 4096/2                      ; DH GEX number of bits
        dd_n 4096                        ; DH GEX Max
  .length = $ - ssh_msg_gex_req


ssh_msg_new_keys:
        db SSH_MSG_NEWKEYS
  .length = $ - ssh_msg_new_keys


ssh_msg_request_service:
        db SSH_MSG_SERVICE_REQUEST
        str "ssh-userauth"              ; Service name
  .length = $ - ssh_msg_request_service


ssh_msg_channel_open:
        db SSH_MSG_CHANNEL_OPEN
        str "session"
        dd_n 0                          ; Sender channel
        dd_n BUFFERSIZE                 ; Initial window size
        dd_n PACKETSIZE                 ; maximum packet size
  .length = $ - ssh_msg_channel_open


ssh_msg_channel_close:
        db SSH_MSG_CHANNEL_CLOSE
        dd_n 0                          ; Sender channel
  .length = $ - ssh_msg_channel_close


ssh_msg_channel_request:
        db SSH_MSG_CHANNEL_REQUEST
        dd_n 0                          ; Recipient channel
        str "pty-req"
        db 1                            ; Bool: want reply
        str "xterm"
        dd_n 80                         ; terminal width (rows)
        dd_n 25                         ; terminal height (rows)
        dd_n 80*8                       ; terminal width (pixels)
        dd_n 25*16                      ; terminal height (pixels)

        dd_n 0                          ; list of supported opcodes
  .length = $ - ssh_msg_channel_request


ssh_msg_shell_request:
        db SSH_MSG_CHANNEL_REQUEST
        dd_n 0                          ; Recipient channel
        str "shell"
        db 1                            ; Bool: want reply
  .length = $ - ssh_msg_shell_request


ssh_msg_channel_data:
        db SSH_MSG_CHANNEL_DATA
        dd_n 0                          ; Sender channel
  .len  dd ?
  .data rb 4*MAX_INPUT_LENGTH + 1


ssh_msg_channel_window_adjust:
        db SSH_MSG_CHANNEL_WINDOW_ADJUST
        dd_n 0                          ; Sender channel
  .wnd  dd ?
  .length = $ - ssh_msg_channel_window_adjust


include_debug_strings

align 4
@IMPORT:

library network, 'network.obj', \
        console, 'console.obj', \
        libcrash, 'libcrash.obj', \
        libini, 'libini.obj'

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
        con_get_flags, 'con_get_flags', \
        con_set_title, 'con_set_title', \
        con_get_input, 'con_get_input'

import  libcrash, \
        sha2_512_init, 'sha2_512_init', \
        sha2_512_update, 'sha2_512_update', \
        sha2_512_finish, 'sha2_512_finish',\
        sha2_256_init, 'sha2_256_init', \
        sha2_256_update, 'sha2_256_update', \
        sha2_256_finish, 'sha2_256_finish',\
        sha1_init, 'sha1_init', \
        sha1_update, 'sha1_update', \
        sha1_finish, 'sha1_finish', \
        md5_init, 'md5_init', \
        md5_update, 'md5_update', \
        md5_finish, 'md5_finish'

import  libini, \
        ini_get_str, 'ini_get_str', \
        ini_set_str, 'ini_set_str'

IncludeIGlobals

i_end:

IncludeUGlobals

params          rb MAX_HOSTNAME_LENGTH

ssh_con         sshlib_connection
ssh_chan        sshlib_channel

keyb_input      rb MAX_INPUT_LENGTH

mem:
