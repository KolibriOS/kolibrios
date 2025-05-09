SYS_COL  = 0xe6e6e6
BT_COL   = 0xcccccc
STR_COL  = 0x595959 ;0x000000

WIN_X = 320
WIN_Y = 300
WIN_W = 390
WIN_H = 230

PAD = 28 ; padding between editboxes

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
        mcall   SF_CHANGE_WINDOW, WIN_X, WIN_Y, WIN_W, WIN_H ; resize to login gui window size

  .redraw:
        call    .draw
        jmp     .still
		
  .draw_editboxes:
        stdcall [edit_box_draw], edit_usr
        stdcall [edit_box_draw], edit_pass
        stdcall [edit_box_draw], edit_server
        stdcall [edit_box_draw], edit_port
        stdcall [edit_box_draw], edit_path
		ret


  align 4
  .draw:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, 40
        edit_boxes_set_sys_color edit_usr,editboxes_end,sc
        mov     edx, 0x34000000
        or      edx, [sc.work]
        mcall   SF_CREATE_WINDOW, <WIN_X,WIN_W>, <WIN_Y,WIN_H>, , 0x805080DD, str_title

		call    .draw_editboxes

        ; draw "connect" button
        mcall   SF_DEFINE_BUTTON, <162,65>, <150,25>, 2, [sc.work_button]

        ; draw strings
        mov     ecx, 0x90000000
        or      ecx, [sc.work_text]
        mcall   SF_DRAW_TEXT, <3,      8>, , gui_str_usr
        mcall    , <3,PAD*1+8>, , gui_str_pass
        mcall    , <3,PAD*2+8>, , gui_str_server
        mcall    , <3,PAD*3+8>, , gui_str_port
        mcall    , <3,PAD*4+8>, , gui_str_path
		
		mov     ecx, 0x90000000
        or      ecx, [sc.work_button_text]
        mcall    , <167,155>, , gui_str_connect
        
		mcall    , <3,185>, 0xb0ff0000, [str_error_addr]
		add     ebx, 1*65536
		mcall
        mov     [str_error_addr], gui_str_null ; reset error string address

        mcall   SF_REDRAW, SSF_END_DRAW
        ret

  align 4
  .still:
        mcall    SF_WAIT_EVENT
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
        mcall   SF_GET_BUTTON

        dec     ah
        jz      .exit

        dec     ah   ; 'Connect' button clicked
        jz      .connect

        jmp     .still

  .key:
        mcall   SF_GET_KEY

        cmp     ah,13
        je      .connect

        cmp     ah,9
        je      .tab

        stdcall [edit_box_key], edit_usr
        stdcall [edit_box_key], edit_pass
        stdcall [edit_box_key], edit_server
        stdcall [edit_box_key], edit_port
        stdcall [edit_box_key], edit_path

        jmp     .still

  .connect:
        mov     esi, 555
        or      [show_log],0
        jnz     @f
        sub     esi, 160
  @@:
        mcall   SF_CHANGE_WINDOW, 35, 20, 830 ; resize to main gui window's coordinates
        cmp     [param_server_addr], 0
        jne     gui.main
        
        mov     [str_error_addr], gui_str_no_srvr
        jmp     .redraw
 
  .error:
        mov     [str_error_addr], gui_str_error
        jmp     .server_addr
    
  .exit:
        jmp     gui.exit

  .tab:
        test word [edit_usr.flags],ed_focus
		je @f
		sub     word [edit_usr.flags],ed_focus
		add     word [edit_pass.flags],ed_focus
		jmp     .tab_end
  @@:
        test word [edit_pass.flags],ed_focus
		je @f
		sub     word [edit_pass.flags],ed_focus
		add     word [edit_server.flags],ed_focus
		jmp     .tab_end       
  @@:
        test word [edit_server.flags],ed_focus
		je @f
		sub     word [edit_server.flags],ed_focus
		add     word [edit_port.flags],ed_focus
		jmp     .tab_end   		
  @@:
        test word [edit_port.flags],ed_focus
		je @f
		sub     word [edit_port.flags],ed_focus
		add     word [edit_path.flags],ed_focus
		jmp     .tab_end
  @@:
        test word [edit_path.flags],ed_focus
		je @f
		sub     word [edit_path.flags],ed_focus
		add     word [edit_usr.flags],ed_focus
		jmp     .tab_end
  @@:
		add     word [edit_usr.flags],ed_focus
		
	.tab_end:	
	    call    .draw_editboxes
        jmp     .still

;=========================================================
;DATA AREA
;=========================================================
		
gui_str_connect db 'Connect',0
gui_str_usr     db 'Username:',0
gui_str_pass    db 'Password:',0
gui_str_server  db 'Server:',0
gui_str_port    db 'Port:',0
gui_str_path    db 'Path:',0
gui_str_error   db 'Error! Check log file for details',0
gui_str_no_srvr db 'Error! Please set server address',0
gui_str_null    db ' ',0

str_error_addr  dd gui_str_null

; login window components
edit_usr    edit_box 300,75,5,      0xffffff,0x94AECE,0,0xAABBCC,0x10000000,64,param_user,mouse_dd,ed_focus,9,9
edit_pass   edit_box 300,75,PAD+5,  0xffffff,0x94AECE,0,0xAABBCC,0x10000000,99,param_password,mouse_dd,ed_pass
edit_server edit_box 300,75,PAD*2+5,0xffffff,0x94AECE,0,0xAABBCC,0x10000000,1024,param_server_addr,mouse_dd,0,13,13
edit_port   edit_box 50, 75,PAD*3+5,0xffffff,0x94AECE,0,0xAABBCC,0x10000000,99,param_port,mouse_dd,ed_figure_only
edit_path   edit_box 300,75,PAD*4+5,0xffffff,0x94AECE,0,0xAABBCC,0x10000000,99,param_path,mouse_dd,0
editboxes_end:

mouse_dd rd 1
