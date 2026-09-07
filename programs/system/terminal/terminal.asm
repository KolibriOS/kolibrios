;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2014-2026. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  terminal for KolibriOS                                         ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format binary as ""

use32
        org     0x0

        db      'MENUET01'
        dd      0x1
        dd      START
        dd      I_END
        dd      IM_END+0x1000
        dd      IM_END+0x1000
        dd      0, 0

include '../../proc32.inc'
include '../../macros.inc'
include '../../dll.inc'
include '../../KOSfuncs.inc'
include '../../develop/libraries/box_lib/box_lib.mac'
include '../../../drivers/serial/common.inc'


START:

        mcall   SF_SYS_MISC, SSF_HEAP_INIT

        stdcall dll.Load, @IMPORT
        or      eax, eax
        jnz     exit

        call    serial_port_init
        test    eax, eax
        jnz     .drv_inited
        mov     [errormsg], err_serial_sys
        jmp     .drv_check_end
  .drv_inited:
        push    0
        stdcall serial_port_get_version, esp
        pop     eax
        shr     eax, 16
        cmp     ax, SERIAL_COMPATIBLE_API_VER
        jle     .drv_check_end
        mov     [errormsg], err_serial_ver
  .drv_check_end:

        mcall   SF_SET_EVENTS_MASK, EVM_MOUSE + EVM_MOUSE_FILTER + EVM_REDRAW + EVM_BUTTON + EVM_KEY

        invoke  init_checkbox, ch1

red_win:
        call draw_window

mainloop:
        mcall   SF_WAIT_EVENT

        dec     eax
        jz      red_win

        dec     eax
        jz      key

        dec     eax
        jz      button

        invoke  edit_box_mouse, edit1
        invoke  edit_box_mouse, edit2
        invoke  edit_box_mouse, edit3
        invoke  edit_box_mouse, edit4

        invoke  option_box_mouse, Option_boxs1
        invoke  option_box_mouse, Option_boxs2

        invoke  check_box_mouse, ch1

        jmp     mainloop

button:
        mcall   SF_GET_BUTTON

        cmp     ah, 0x10        ; connect button
        je      open_connection

        test    ah, ah
        jz      mainloop
exit:
        mcall   SF_TERMINATE_PROCESS

key:
        mcall   SF_GET_KEY

        cmp     ah, 13          ; enter key
        je      open_connection

        invoke  edit_box_key, edit1
        invoke  edit_box_key, edit2
        invoke  edit_box_key, edit3
        invoke  edit_box_key, edit4

        jmp     mainloop



draw_window:
; get system colors
        mcall   SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors

        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mov     edx, [sc.work]
        or      edx, 0x34000000
        xor     esi, esi
        mov     edi, str_title
        mcall   SF_CREATE_WINDOW, <50, 415>, <30, 195>

        mov     ebx, 5 shl 16 + 12
        mov     ecx, 0x90000000
        or      ecx, [sc.work_text]
        mov     edx, str_port
        mcall   SF_DRAW_TEXT
        add     ebx, 25
        mov     edx, str_speed
        mcall
        add     ebx, 25
        mov     edx, str_data
        mcall
        add     ebx, 25
        mov     edx, str_stop
        mcall

        mov     ebx, 195 shl 16 + 12
        mov     edx, str_parity
        mcall
        mov     ebx, 280 shl 16 + 12
        mov     edx, str_flow
        mcall

		edit_boxes_set_sys_color edit1,editboxes_end,sc
        invoke  edit_box_draw, edit1
        invoke  edit_box_draw, edit2
        invoke  edit_box_draw, edit3
        invoke  edit_box_draw, edit4

		option_boxes_set_sys_color sc, Option_boxs1
		option_boxes_set_sys_color sc, Option_boxs2
        invoke  option_box_draw, Option_boxs1
        invoke  option_box_draw, Option_boxs2

		check_boxes_set_sys_color2 ch1,ch1_end,sc ;set color
        invoke  check_box_draw, ch1

        mov     esi, [sc.work_button]
        mcall   SF_DEFINE_BUTTON, <280, 100>, <115, 22>, 0x10

        mov     ecx, 0x90000000
        or      ecx, [sc.work_button_text]
        mcall   SF_DRAW_TEXT, <315, 119>, , str_open


        mov     edx, [sc.work_graph]
        mcall   SF_DRAW_LINE, <0, 405>, <145, 145>

        mov     ecx, 0x90000000
        or      ecx, [sc.work_text]
        mcall   SF_DRAW_TEXT, <5, 150>, , [errormsg]

        mcall   SF_REDRAW, SSF_END_DRAW
        ret


open_connection:

        mov     [errormsg], err_none    ; clear previous error message

; Baud rate
        mov     esi, ed_speed
        call    str_to_uint
        test    eax, eax
        jnz     .speed_bad
        test    ebx, ebx
        jnz     .speed_ok
  .speed_bad:
        mov     [errormsg], err_conf
        jmp     red_win
  .speed_ok:
        mov     [port_conf + SP_CONF.baudrate], ebx

; Check number of data bits
        mov     al, [ed_data]
        cmp     al, '8'
        ja      .invalid_data
        cmp     al, '5'
        jb      .invalid_data
        jmp     .data_ok
  .invalid_data:
        mov     [errormsg], err_databits
        jmp     red_win
  .data_ok:
        sub     al, '0'
        mov     [port_conf + SP_CONF.word_size], al

; Check the parity type
        mov     bl, SERIAL_CONF_PARITY_NONE
        cmp     [option_group1], op1
        je      .parity_ok

        mov     bl, SERIAL_CONF_PARITY_ODD
        cmp     [option_group1], op2
        je      .parity_ok

        mov     bl, SERIAL_CONF_PARITY_EVEN
        cmp     [option_group1], op3
        je      .parity_ok

        mov     bl, SERIAL_CONF_PARITY_MARK
        cmp     [option_group1], op4
        je      .parity_ok

        mov     bl, SERIAL_CONF_PARITY_SPACE
        cmp     [option_group1], op5
        je      .parity_ok
        jmp     exit2                   ; something went terribly wrong
  .parity_ok:
        mov     [port_conf + SP_CONF.parity], bl

; Check number of stop bits
        mov     bl, SERIAL_CONF_STOP_BITS_1
        cmp     [ed_stop], '1'
        je      .stop_ok

        mov     bl, SERIAL_CONF_STOP_BITS_2
        cmp     [ed_stop], '2'
        je      .stop_ok

        mov     [errormsg], err_stopbits
        jmp     red_win
  .stop_ok:
        mov     [port_conf + SP_CONF.stop_bits], bl

; Check port id
        mov     esi, ed_port
        cmp     byte [esi], 0
        jz      .port_bad
        call    str_to_uint
        test    eax, eax
        jz      .port_ok
  .port_bad:
        mov     [errormsg], err_port
        jmp     red_win
  .port_ok:

        lea     ecx, [port_conf]
        lea     edx, [port_handle]
        stdcall serial_port_open, ebx, ecx, edx
        test    eax, eax
        jz      .opened

        mov     [errormsg], err_port
        cmp     eax, SERIAL_API_ERR_PORT_INVALID
        jz      red_win

        mov     [errormsg], err_reserve
        cmp     eax, SERIAL_API_ERR_PORT_BUSY
        jz      red_win

        mov     [errormsg], err_conf
        cmp     eax, SERIAL_API_ERR_CONF
        jz      red_win

        mov     [errormsg], err_unknown
        jmp     red_win

; Hide our GUI window and open the console
  .opened:
        mcall   SF_SET_EVENTS_MASK, 0           ; disable all events
        mcall   SF_CHANGE_WINDOW, 0, 0, 0, 0    ; hide window
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_REDRAW, SSF_END_DRAW

        invoke  con_start, 1
        invoke  con_init, 80, 25, 80, 25, str_title

console_loop:
        mcall   SF_SLEEP, 1     ; wait 10 ms

        invoke  con_get_flags
        test    eax, 0x200      ; con window closed?
        jnz     .close_port

  .tx_loop:
        invoke  con_kbhit
        test    eax, eax        ; did user press a key?
        jz      .rx_loop

        invoke  con_getch2      ; get the pressed key from buffer
        and     eax, 0xff
        mov     [port_buf], eax
        mov     [port_data_cnt], 1
        stdcall serial_port_write, [port_handle], port_buf, port_data_cnt

        test    [ch1.flags], ch_flag_en ; does user want us to echo locally?
        je      .tx_loop

        invoke  con_write_asciiz, port_buf   ; print the character
        jmp     .tx_loop

  .rx_loop:
        mov     [port_buf], 0
        mov     [port_data_cnt], 3
        stdcall serial_port_read, [port_handle], port_buf, port_data_cnt
        test    eax, eax
        jnz     console_loop    ; an error occured
        mov     ebx, [port_data_cnt]
        test    ebx, ebx
        jz      console_loop    ; no data yet
        invoke  con_write_asciiz, port_buf
        jmp     .rx_loop


  .close_port:
        stdcall serial_port_close, [port_handle]

exit2:
        mcall   SF_TERMINATE_PROCESS


str_to_uint:
; esi = source string
; eax = 0 if success
; ebx = result number
        xor     eax, eax
        xor     ebx, ebx
  .loop:
        lodsb
        test    al, al
        jz      .done
        sub     al, '0'
        jb      .done
        cmp     al, 9
        ja      .done
        lea     ebx, [ebx + 4 * ebx]
        shl     ebx, 1
        add     ebx, eax
        jmp     .loop
  .done:
        ret

;-------------------------
; DATA

align 16
@IMPORT:

library box_lib, 'box_lib.obj',\
        console, 'console.obj'

import  box_lib,\
        edit_box_draw,          'edit_box_draw',\
        edit_box_key,           'edit_box_key',\
        edit_box_mouse,         'edit_box_mouse',\
        init_checkbox,          'init_checkbox2',\
        check_box_draw,         'check_box_draw2',\
        check_box_mouse,        'check_box_mouse2',\
        option_box_draw,        'option_box_draw',\
        option_box_mouse,       'option_box_mouse'

import  console,\
        con_start,              'START',\
        con_init,               'con_init',\
        con_exit,               'con_exit',\
        con_getch2,             'con_getch2',\
        con_write_asciiz,       'con_write_asciiz',\
        con_get_flags,          'con_get_flags',\
        con_kbhit,              'con_kbhit'

edit1   edit_box 60, 112, 10, 0xffffff, 0x6f9480, 0, 0, 0x10000000, 5, ed_port, mouse_dd, ed_focus, 1, 1
edit2   edit_box 60, 112, 35, 0xffffff, 0x6a9480, 0, 0, 0x10000000, 7, ed_speed, mouse_dd, ed_figure_only, 4, 4
edit3   edit_box 60, 112, 60, 0xffffff, 0x6a9480, 0, 0, 0x10000000, 1, ed_data, mouse_dd, ed_figure_only, 1, 1
edit4   edit_box 60, 112, 85, 0xffffff, 0x6a9480, 0, 0, 0x10000000, 1, ed_stop, mouse_dd, ed_figure_only, 1, 1
editboxes_end:

ed_port         db "0",0,0,0,0,0,0
ed_speed        db "9600",0,0,0
ed_data         db "8",0
ed_stop         db "1",0

option_group1   dd op1
op1     option_box option_group1, 195, 30, 6, 12, 0xffffff, 0, 0, str_none, 4
op2     option_box option_group1, 195, 47, 6, 12, 0xffffff, 0, 0, str_odd, 3
op3     option_box option_group1, 195, 64, 6, 12, 0xffffff, 0, 0, str_even, 4
op4     option_box option_group1, 195, 81, 6, 12, 0xffffff, 0, 0, str_mark, 4
op5     option_box option_group1, 195, 98, 6, 12, 0xffffff, 0, 0, str_space, 5

option_group2   dd op6
op6     option_box option_group2, 280, 30, 6, 12, 0xffffff, 0, 0, str_none, 4
;op7     option_box option_group2, 270, 44, 6, 12, 0xffffff, 0, 0, str_xon, 8, 10b
;op8     option_box option_group2, 270, 61, 6, 12, 0xffffff, 0, 0, str_rts, 7, 1b
;op9     option_box option_group2, 270, 78, 6, 12, 0xffffff, 0, 0, str_dsr, 7

ch1     check_box2 5 shl 16 + 12, 119 shl 16 + 12, 5, 0xffffff, 0x000000, 0, str_echo, ch_flag_middle
ch1_end:

Option_boxs1    dd op1, op2, op3, op4, op5, 0
Option_boxs2    dd op6, 0 ;op7, op8, op9, 0

str_title       db 'Terminal', 0
str_port        db 'Serial port:', 0
str_speed       db 'Speed (baud):', 0
str_data        db 'Data bits:', 0
str_parity      db 'Parity:', 0
str_flow        db 'Flow control:', 0
str_stop        db 'Stop bits:', 0

str_open        db 'Open', 0

str_none        db 'None'
str_odd         db 'Odd'
str_even        db 'Even'
str_mark        db 'Mark'
str_space       db 'Space'
;str_xon         db 'XON/XOFF'
;str_rts         db 'RTS/CTS'
;str_dsr         db 'DSR/DTR'

str_echo        db 'Local echo', 0

errormsg        dd err_none
err_none        db 0
err_port        db 'Invalid serial port.', 0
err_reserve     db 'The port is already in use.', 0
err_conf        db 'Incorrect port setting.', 0
err_stopbits    db 'Invalid number of stop bits. Must be 1 or 2.', 0
err_databits    db 'Invalid number of data bits. Must be between 5 and 8.', 0
err_unknown     db 'An unknown error.', 0
err_serial_sys  db 'Error loading serial driver.', 0
err_serial_ver  db 'Incompatible serial driver version.', 0

port_conf:
        dd      port_conf_end - port_conf
        dd      9600
        db      8, SERIAL_CONF_STOP_BITS_1, SERIAL_CONF_PARITY_NONE, SERIAL_CONF_FLOW_CTRL_NONE
port_conf_end:

I_END:

mouse_dd        dd ?
echo            db ?
sc              system_colors
port_handle     dd ?
port_buf        dd ?
port_data_cnt   dd ?

IM_END:
