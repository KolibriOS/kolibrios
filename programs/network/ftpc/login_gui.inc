SYS_COL  = 0xe6e6e6
BT_COL   = 0xcccccc
STR_COL  = 0x595959 ;0x000000

;;================================================================================================;;
login_gui: ;//////////////////////////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Login GUI-specific functions                                                                   ;;
;;------------------------------------------------------------------------------------------------;;
;> none                                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;< none                                                                                           ;;
;;================================================================================================;;

    ; TODO: print error strings (wrong user, pass, etc.)

  .server_addr:
        mov     [initial_login], 1

  .get_username:
        ; in case of error when either login_gui.server_addr or 
        ; login_gui.get_username is called, should resize window
        mcall   67, 320, 300, 390, 200 ; resize to login gui window size

  .redraw:
        call    .draw
        jmp     .still


  align 4
  .draw:
        mcall   12, 1
        mcall   0, <320,390>, <300,200>, 0x34000000+SYS_COL, 0x805080DD, hed

        stdcall [edit_box_draw], edit_usr
        stdcall [edit_box_draw], edit_pass
        stdcall [edit_box_draw], edit_server
        stdcall [edit_box_draw], edit_port
        stdcall [edit_box_draw], edit_path

        ; draw "connect" button
        mcall   8, <162,65>, <140,25>, 2, BT_COL

        ; draw strings
        mcall   4, <3,5>, 0xb0000000, gui_str_usr
        mcall   4, <3,25>, 0xb0000000, gui_str_pass
        mcall   4, <3,45>, 0xb0000000, gui_str_server
        mcall   4, <3,65>, 0xb0000000, gui_str_port
        mcall   4, <3,85>, 0xb0000000, gui_str_path
        mcall   4, <167,145>, 0xb0000000+STR_COL, gui_str_connect
        mcall   4, <3,115>, 0xb0ff0000, [str_error_addr]
        mov     [str_error_addr], gui_str_null ; reset error string address

        mcall   12, 2
        ret

  align 4
  .still:
        mcall    10     ; wait for event
        dec      eax
        jz       .redraw
        dec      eax
        jz       .key
        dec      eax
        jz       .button

        stdcall [edit_box_mouse], edit_usr
        stdcall [edit_box_mouse], edit_pass
        stdcall [edit_box_mouse], edit_server
        stdcall [edit_box_mouse], edit_port
        stdcall [edit_box_mouse], edit_path

        jmp     .still

  .button:
        mcall   17

        dec     ah
        jz      .exit

        dec     ah   ; 'Connect' button clicked
        jz      gui.main

        jmp     .still

  .key:
        mcall   2

        stdcall [edit_box_key], edit_usr
        stdcall [edit_box_key], edit_pass
        stdcall [edit_box_key], edit_server
        stdcall [edit_box_key], edit_port
        stdcall [edit_box_key], edit_path

        jmp     .still

  .error:
        mov     [str_error_addr], gui_str_error
        jmp     .server_addr
    
  .exit:
        jmp     gui.exit


gui_str_connect db 'Connect',0
gui_str_usr     db 'Username:',0
gui_str_pass    db 'Password:',0
gui_str_server  db 'Server:',0
gui_str_port    db 'Port:',0
gui_str_path    db 'Path:',0
gui_str_error   db 'ERROR! Check log file for details',0
gui_str_null    db ' ',0

str_error_addr  dd gui_str_null

; login window components
edit_usr edit_box 300,75,5, 0xffffff,0x6f9480,0,0xAABBCC,0,99,param_user,mouse_dd,ed_focus
edit_pass edit_box 300,75,25,0xffffff,0x6a9480,0,0xAABBCC,0,99,param_password,mouse_dd,ed_pass
edit_server edit_box 300,75,45,0xffffff,0x6a9480,0,0xAABBCC,0,99,param_server_addr,mouse_dd,0
edit_port edit_box 50,75,65,0xffffff,0x6a9480,0,0xAABBCC,0,99,param_port,mouse_dd,ed_figure_only
edit_path edit_box 300,75,85,0xffffff,0x6a9480,0,0xAABBCC,0,99,param_path,mouse_dd,0

mouse_dd rd 1