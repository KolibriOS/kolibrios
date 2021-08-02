;    sshlib_channel.inc - SSH channel
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


proc sshlib_chan_open con_ptr; Channel struct ptr?!

; >> Open channel

        DEBUGF  2, "SSH: Open channel\n"

        mov     [ssh_chan.rcv_wnd], BUFFERSIZE
        mov     [ssh_chan.snd_wnd], 0
        stdcall sshlib_send_packet, [con_ptr], ssh_msg_channel_open, ssh_msg_channel_open.length, 0
        cmp     eax, 0
        jl      .err

; << Check for channel open confirmation

        stdcall sshlib_msg_handler, [con_ptr], 0
        cmp     eax, 0
        jl      .err

        mov     esi, [con_ptr]
        cmp     [esi + sshlib_connection.rx_buffer.message_code], SSH_MSG_CHANNEL_OPEN_CONFIRMATION
        jne     .err_proto

; >> Channel request: pty

        DEBUGF  2, "SSH: Request pty\n"

        stdcall sshlib_send_packet, [con_ptr], ssh_msg_channel_request, ssh_msg_channel_request.length, 0
        cmp     eax, 0
        jl      .err

; << Check for channel request confirmation

        stdcall sshlib_msg_handler, [con_ptr], 0
        cmp     eax, 0
        jl      .err

        mov     esi, [con_ptr]
        cmp     [esi + sshlib_connection.rx_buffer.message_code], SSH_MSG_CHANNEL_SUCCESS
        jne     .err_proto

; >> Channel request: shell

        DEBUGF  2, "SSH: Request shell\n"

        stdcall sshlib_send_packet, [con_ptr], ssh_msg_shell_request, ssh_msg_shell_request.length, 0
        cmp     eax, 0
        jl      .err

; << Check for channel request confirmation

; TODO: timeout
  .wait_success:
        stdcall sshlib_msg_handler, [con_ptr], 0
        cmp     eax, 0
        jl      .err

        mov     esi, [con_ptr]
        cmp     [esi + sshlib_connection.rx_buffer.message_code], SSH_MSG_CHANNEL_SUCCESS
        jne     .wait_success

        xor     eax, eax
  .err:
        ret

  .err_proto:
        mov     eax, SSHLIB_ERR_PROTOCOL
        ret

endp