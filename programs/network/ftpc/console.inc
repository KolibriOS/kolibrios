;;================================================================================================;;
console: ;////////////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Console-specific functions - initialization, clear screen,                                     ;;
;? .get_cmd - Takes user command as input from the console                                        ;;
;? .server_addr - Gets server address from user in the form address:port                          ;;
;? .get_username/.get_pass - Takes username/password as input from the user                       ;;
;;------------------------------------------------------------------------------------------------;;
;>                                                                                                ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;

  dd .init
  dd .server_addr
  dd .get_username
  dd .get_cmd
  dd .print
  dd .set_flags
  dd .list
  dd .progress
  dd .error

  .init:
; load console library
        stdcall dll.Load, @IMPORT_CONSOLE
; initialize console
        invoke  con_start, 1
        invoke  con_init, 120, 43, 120, 300, str_title
        invoke  con_cls
; Welcome user
        invoke  con_write_asciiz, str_welcome
        ret

  .server_addr:
        mov     [initial_login], 1
        invoke  con_cls
        invoke  con_set_flags, 0x07
; ask for server addr
        invoke  con_write_asciiz, str_srv_addr
; write prompt (in green color)
        invoke  con_set_flags, 0x0a
        invoke  con_write_asciiz, str_prompt
; read string
        invoke  con_gets, param_server_addr, 256
; check for exit
        test    eax, eax
        jz      .exit
        cmp     byte [param_server_addr], 10
        jz      .exit

  .port:
        invoke  con_write_asciiz, str_port
        invoke  con_gets, param_port, 256

  ; read username
  .get_username:
        invoke  con_set_flags, 0x0a
        invoke  con_write_asciiz, str_user
        invoke  con_gets, param_user, 256

  ; read password
  .get_pass:
        invoke  con_write_asciiz, str_pass
        invoke  con_set_flags, 0x00             ; black text on black background for password
        invoke  con_gets, param_password, 256
        invoke  con_set_flags, 0x0a

        cmp     [initial_login], 1
        jne     arg_handler.copy_user
        mov     [initial_login], 0

  ; get initial path
  .get_path:
        invoke  con_write_asciiz, str_path
        invoke  con_gets, param_path, 256
        invoke  con_write_asciiz, str_newline

        jmp     arg_handler.connect

  .get_cmd:
        ; write prompt
        invoke  con_write_asciiz, str_prompt
        ; read string
        invoke  con_gets, buf_cmd, 256

        ; print a newline and reset the color back to grey
        invoke  con_write_asciiz, str_newline
        invoke  con_set_flags, 0x07

        jmp     wait_for_usercommand.parse_cmd

  .print:
        pushad

        invoke  con_write_asciiz, [esp+36]
        mov     esi, [esp+36]
        mov     ecx, -1
    @@:
        inc     ecx
        lodsb
        test    al, al
        jnz     @b
        ; write to log file
        mov     eax, [esp+36]
        call    write_to_file

      @@:
        popad
        ret     4

  .set_flags:
        invoke  con_set_flags, [esp+4]
        ret     4

  .list:
        invoke  con_write_asciiz, buf_buffer2
        jmp     data_loop

  .progress: ; edx = no. of bytes transferred
        mov     eax, edx
        mov     edi, str_bytes_done
        call    dword_ascii
        mov     byte[edi],0
        icall   eax, interface_addr, interface.print, str_downloaded, str_bytes_done, str_bytes
        ret

  .error:
        invoke  con_getch2
        jmp     .server_addr

  .exit:
        invoke  con_exit, 1
        jmp     exit


align 4
@IMPORT_CONSOLE:

library console, 'console.obj'

import  console, \
        con_start,          'START', \
        con_init,           'con_init', \
        con_write_asciiz,   'con_write_asciiz', \
        con_exit,           'con_exit', \
        con_gets,           'con_gets', \
        con_cls,            'con_cls', \
        con_getch2,         'con_getch2', \
        con_set_cursor_pos, 'con_set_cursor_pos', \
        con_write_string,   'con_write_string', \
        con_get_flags,      'con_get_flags', \
        con_set_flags,      'con_set_flags'