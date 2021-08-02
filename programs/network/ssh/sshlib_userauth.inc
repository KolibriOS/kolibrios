;    ssh_userauth.inc - SSH user authentication
;
;    Copyright (C) 2021 Jeffrey Amelynck
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


proc sshlib_userauth_password con_ptr, username_sz, password_sz

; >> Request service (user-auth)

        DEBUGF  2, "SSH: Requesting service\n"

        stdcall sshlib_send_packet, [con_ptr], ssh_msg_request_service, ssh_msg_request_service.length, 0
        cmp     eax, 0
        jl      .err

; << Check for service acceptance

        stdcall sshlib_msg_handler, [con_ptr], 0
        cmp     eax, 0
        jl      .err

        mov     eax, [con_ptr]
        cmp     [eax + sshlib_connection.rx_buffer.message_code], SSH_MSG_SERVICE_ACCEPT
        jne     .err_proto

; >> Request user authentication

        DEBUGF  2, "SSH: User authentication\n"

        mcall   68, 12, 1024    ; FIXME: hardcoded size
        test    eax, eax
        jz      .err_nomem
        mov     edi, eax
        mov     ebx, eax
        mov     byte[edi], SSH_MSG_USERAUTH_REQUEST
        inc     edi

; Insert username
        stdcall sz_len, [username_sz]
        mov     ecx, eax
        mov     esi, [username_sz]
        bswap   eax
        stosd
        rep movsb

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

; Insert password
        stdcall sz_len, [password_sz]
        mov     ecx, eax
        mov     esi, [password_sz]
        bswap   eax
        stosd
        rep movsb

        sub     edi, ebx
        push    ebx
        stdcall sshlib_send_packet, [con_ptr], ebx, edi, 0

; Clear used buffer and free
        pop     edx
        mov     edi, edx
        push    eax
        mov     ecx, 1024/4     ; FIXME
        xor     eax, eax
        rep stosd
        mcall   68, 13, edx
        pop     eax

        cmp     eax, 0
        jl      .err

; << Check for userauth acceptance
  @@:
        stdcall sshlib_msg_handler, [con_ptr], 0
        cmp     eax, 0
        jl      .err

        mov     eax, [con_ptr]
        mov     al, [eax + sshlib_connection.rx_buffer.message_code]

        cmp     al, SSH_MSG_USERAUTH_BANNER
        je      @r      ; TODO

        cmp     al, SSH_MSG_USERAUTH_FAILURE
        je      .fail

        cmp     al, SSH_MSG_USERAUTH_SUCCESS
        jne     .err_proto

        xor     eax, eax
  .err:
        ret

  .fail:
        xor     eax, eax
        inc     eax
        ret

  .err_proto:
        mov     eax, SSHLIB_ERR_PROTOCOL
        ret

  .err_nomem:
        mov     eax, SSHLIB_ERR_NOMEM
        ret


endp


; Actually, string is \n and/or \0 terminated 0_o
proc sz_len uses ecx edi, string

        mov     edi, [string]
        mov     ecx, 256     ;;;;
        mov     al, 10
        repne scasb
        dec     edi
        sub     edi, [string]
        mov     eax, edi
        ret

endp