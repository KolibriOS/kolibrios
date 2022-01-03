;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2014. All rights reserved.         ;;
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
include '../../develop/libraries/box_lib/trunk/box_lib.mac'


START:

        mcall   68, 11

        stdcall dll.Load, @IMPORT
        or      eax, eax
        jnz     exit

        mcall   40, EVM_MOUSE + EVM_MOUSE_FILTER + EVM_REDRAW + EVM_BUTTON + EVM_KEY

        invoke  init_checkbox, ch1

red_win:
        call draw_window

mainloop:
        mcall   10

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
        mcall   17

        cmp     ah, 0x10        ; connect button
        je      open_connection

        test    ah , ah
        jz      mainloop
exit:
        mcall   -1

key:
        mcall   2

        cmp     ah, 13          ; enter key
        je      open_connection

        invoke  edit_box_key, edit1
        invoke  edit_box_key, edit2
        invoke  edit_box_key, edit3
        invoke  edit_box_key, edit4

        jmp     mainloop



draw_window:
; get system colors
        mcall   48, 3, sc, 40

        mcall   12,1
        mov     edx, [sc.work]
        or      edx, 0x34000000
        xor     esi, esi
        mov     edi, str_title
        mcall   0, 50 shl 16 + 415, 30 shl 16 + 195

        mov     ebx, 5 shl 16 + 12
        mov     ecx, 0x90000000
        or      ecx, [sc.work_text]
        mov     edx, str_port
        mcall   4
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
        mcall   8, 280 shl 16 + 100, 115 shl 16 + 22, 0x10

        mov     ecx, 0x90000000
        or      ecx, [sc.work_button_text]
        mcall   4, 315 shl 16 + 119, , str_open


        mov     edx, [sc.work_graph]
        mcall   38, 0 shl 16 + 405, 145 shl 16 + 145

        mov     ecx, 0x90000000
        or      ecx, [sc.work_text]
        mcall   4, 5 shl 16 + 150, , [errormsg]

        mcall   12, 2
        ret


open_connection:

        mov     [errormsg], err_none    ; clear previous error message

; Read the serial port name, and convert it to a port number
        cmp     byte[ed_port+4], 0
        jne     .port_error
        mov     eax, dword[ed_port]
        or      eax, 0x20202020         ; convert to lowercase
        cmp     eax, 'com1'
        je      .com1
        cmp     eax, 'com2'
        je      .com2
        cmp     eax, 'com3'
        je      .com3
        cmp     eax, 'com4'
        je      .com4
  .port_error:
        mov     [errormsg], err_port
        jmp     red_win

  .com1:
        mov     [port], 0x3f8
        jmp     .port_ok
  .com2:
        mov     [port], 0x2f8
        jmp     .port_ok
  .com3:
        mov     [port], 0x3e8
        jmp     .port_ok
  .com4:
        mov     [port], 0x2e8
  .port_ok:

; reserve the com port so we can work with it
        xor     ebx, ebx
        movzx   ecx, [port]
        mov     edx, ecx
        add     edx, 7
        mcall   46
        test    eax, eax
        jz      .port_reserved
        mov     [errormsg], err_reserve
        jmp     red_win
  .port_reserved:

; disable com interrupts
; (We cannot receive them on the application level :( )
        mov     dx, [port]
        inc     dx
        mov     al, 0
        out     dx, al

; Set speed:
; Convert the ascii decimal number that user entered
; So we can do some math with it
        mov     esi, ed_speed
        xor     eax, eax
        xor     ebx, ebx
  .convert_loop:
        lodsb
        test    al, al
        jz      .convert_done
        sub     al, '0'
        jb      .invalid_speed
        cmp     al, 9
        ja      .invalid_speed
        lea     ebx, [ebx + 4*ebx]
        shl     ebx, 1
        add     ebx, eax
        jmp     .convert_loop
  .invalid_speed:
        call    free_port
        mov     [errormsg], err_speed
        jmp     red_win
  .convert_done:
        test    ebx, ebx
        jz      .invalid_speed

; We now have the speed setting in ebx
; calculate the divisor latch value as 115200/ebx
        xor     edx, edx
        mov     eax, 115200
        div     ebx
        test    edx, edx
        jnz     .invalid_speed
        cmp     eax, 0xffff
        ja      .invalid_speed
        mov     bx, ax

; enable Divisor latch
        mov     dx, [port]
        add     dx, 3
        mov     al, 1 shl 7     ; dlab bit
        out     dx, al

; Set divisor latch value
        mov     dx, [port]
        mov     al, bl
        out     dx, al
        inc     dx
        mov     al, bh
        out     dx, al

; Check the parity type
        xor     bl, bl
        cmp     [option_group1], op1    ; none
        je      .parity_ok

        mov     bl, 001b shl 3
        cmp     [option_group1], op2    ; odd
        je      .parity_ok

        mov     bl, 011b shl 3
        cmp     [option_group1], op3    ; even
        je      .parity_ok

        mov     bl, 101b shl 3
        cmp     [option_group1], op4    ; mark
        je      .parity_ok

        mov     bl, 111b shl 3
        cmp     [option_group1], op5    ; space
        je      .parity_ok
        jmp     exit2                   ; something went terribly wrong
  .parity_ok:

; Check number of stop bits
        cmp     [ed_stop], '1'
        je      .stop_ok
        cmp     [ed_stop], '2'
        jne     .invalid_stop
        or      bl, 1 shl 2     ; number of stop bits
        jmp     .stop_ok
  .invalid_stop:
        call    free_port
        mov     [errormsg], err_stopbits
        jmp     red_win
  .stop_ok:

; Check number of data bits
        mov     al, [ed_data]
        cmp     al, '8'
        ja      .invalid_data
        sub     al, '5'
        jae     .data_ok
  .invalid_data:
        call    free_port
        mov     [errormsg], err_databits
        jmp     red_win
  .data_ok:
        or      al, bl
; Program data bits, stop bits and parity in the UART
        mov     dx, [port]
        add     dx, 3           ; Line Control Register
        out     dx, al

; clear +  enable fifo (64 bytes), 1 byte trigger level
        mov     dx, [port]
        inc     dx
        inc     dx
        mov     al, 0x7 + 1 shl 5
        out     dx, al

; flow control
        mov     dx, [port]
        add     dx, 4
        mov     al, 0xb
        out     dx, al

; Hide our GUI window and open the console
        mcall   40, 0           ; disable all events
        mcall   67, 0, 0, 0, 0  ; hide window
        mcall   12, 1
        mcall   12, 2

        invoke  con_start, 1
        invoke  con_init, 80, 25, 80, 25, str_title

console_loop:
        mcall   5, 1            ; wait 10 ms

        invoke  con_get_flags
        test    eax, 0x200      ; con window closed?
        jnz     exit2

  .tx_loop:
        invoke  con_kbhit
        test    eax, eax        ; did user press a key?
        jz      .rx_loop

        invoke  con_getch2      ; get the pressed key from buffer
        mov     dx, [port]
        out     dx, al

        test    [ch1.flags], ch_flag_en ; does user want us to echo locally?
        je      .tx_loop

        and     eax, 0xff
        push    eax
        invoke  con_write_asciiz, esp   ; print the character
        pop     eax
        jmp     .tx_loop

  .rx_loop:
        mov     dx, [port]
        add     dx, 5           ; Line status register
        in      al, dx
        test    al, 1           ; Data ready?
        jz      console_loop

        mov     dx, [port]      ; Read character from buffer
        in      al, dx

        and     eax, 0xff
        push    eax
        invoke  con_write_asciiz, esp   ; print the character
        pop     eax

        jmp     .rx_loop


exit2:

        call    free_port
        mcall   -1      ; exit

free_port:

        xor     ebx, ebx
        inc     ebx
        movzx   ecx, [port]
        mov     edx, ecx
        add     edx, 7
        mcall   46

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

edit1   edit_box 60, 112, 10, 0xffffff, 0x6f9480, 0, 0, 0x10000000, 8, ed_port, mouse_dd, ed_focus, 4, 4
edit2   edit_box 60, 112, 35, 0xffffff, 0x6a9480, 0, 0, 0x10000000, 7, ed_speed, mouse_dd, ed_figure_only, 4, 4
edit3   edit_box 60, 112, 60, 0xffffff, 0x6a9480, 0, 0, 0x10000000, 1, ed_data, mouse_dd, ed_figure_only, 1, 1
edit4   edit_box 60, 112, 85, 0xffffff, 0x6a9480, 0, 0, 0x10000000, 1, ed_stop, mouse_dd, ed_figure_only, 1, 1
editboxes_end:

ed_port         db "COM1",0,0,0,0,0,0
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
err_speed       db 'Incorrect speed setting.', 0
err_stopbits    db 'Invalid number of stop bits. Must be 1 or 2.', 0
err_databits    db 'Invalid number of data bits. Must be between 5 and 8.', 0

I_END:

mouse_dd        dd ?
echo            db ?
port            dw ?
sc              system_colors

IM_END: