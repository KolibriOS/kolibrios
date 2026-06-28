;;      Copyright (C) 2021-2026,  Mikhail Frolov aka Doczom

macro send_notify send_str{
      mov     dword[run_notify.message], send_str
      mcall   SF_FILE, run_notify
}

DRAW_TEXT_UTF8z = 0xB0000000
DRAW_TEXT_6x9x2 = 0x81000000

BTN_ID_SW_MODE  = 8 ; switch mode
BTN_ID_RUN_LOG  = 9
BTN_ID_SAVE_LOG = 10

        use32
        org    0

        db     'MENUET01'
        dd     1
        dd     START
        dd     I_END
        dd     MEM
        dd     STACKTOP
header.cmdline:
        dd     PATH
        dd     0

include '../../proc32.inc'
include '../../macros.inc'
include '../../KOSfuncs.inc'
include '../../dll.inc'

include 'parser.inc'
START:
        mcall   SF_SYS_MISC, SSF_HEAP_INIT

        stdcall dll.Load, @IMPORT
        or      eax, eax
        jnz     err_load_lib

        invoke OpenDialog_Init, OpenDialog_data
        ; init futex
        mcall   SF_FUTEX, SSF_CREATE, futex_cmd
        mov     [futex_handle], eax

        ; parse cmd_line
        cmp     byte[PATH], 0
        jz      @f
        call    parse_cmd
@@:
        mcall   SF_SET_EVENTS_MASK, EVM_REDRAW + EVM_BUTTON
        ;load driver
        mcall   SF_SYS_MISC, SSF_LOAD_DRIVER, [drv_name_ptr]
        test    eax, eax
        jnz     @f

        mcall   SF_SYS_MISC, SSF_LOAD_DRIVER_PE, [drv_path_ptr], 0
@@:
        mov     dword[drv_struct.handl], eax
        test    eax, eax ;
        jz      error_drv

main:
        mcall   SF_SYS_MISC, SSF_CONTROL_DRIVER, drv_struct
        cmp     dword[save_ptr], 0
        jz      still

        mcall   SF_CREATE_THREAD, 1, thread_auto_save, thread_auto_save.stack
        cmp     eax, -1
        jz      still

        mov     dword[PID_AUTO_SAVE], eax
still:          ;void main()
        call    draw

        mcall   SF_WAIT_EVENT_TIMEOUT, 200 ;2 second
        dec     eax      ; redraw request ?
        je      still
        cmp     eax, 2   ;button
        jz      button
        jmp     still

button:
        mcall   SF_GET_BUTTON

        cmp     eax, 1
        jz      still
        dec     ah
        je      exit
        ; switch window mode
        cmp     ah, BTN_ID_SW_MODE - 1
        jnz     .no_micro_info

        xor     byte[flag_micro_info], 1
        jmp     still
.no_micro_info:
        cmp     ah, BTN_ID_RUN_LOG - 1
        jnz     .no_log_button

        cmp     dword[text_log_butt], _start_log
        jnz     .log_stop

        cmp     dword[log_ptr], 0
        jnz     @f
        mcall   SF_SYS_MISC, SSF_MEM_ALLOC, 19 ; heared for Graph file
        test    eax, eax
        jz      .err_alloc_mem

        mov     [log_ptr], eax
        mov     dword[log_size], 19
        mov     edi, eax
        mov     eax,[drv_data.Tmax]
        mov     ebx, graph_start.new_data
        call    int_to_str
        mov     esi, graph_start
        ; set base value in log
        movsd
        movsd
        movsd
        movsd
        movsw
        movsb
@@:
        mcall   SF_CREATE_THREAD, 1, thread_timer, thread_timer.stack
        mov     dword[text_log_butt], _stop_log
        jmp     still
.log_stop:
        ; waking up the flow through the futex
        mov     byte[futex_cmd], 1  ; stop thread
        mcall   SF_FUTEX, SSF_WAKE, [futex_handle], 1
        mcall   SF_SYS_MISC, SSF_SWITCH_TASK ; in order for the message to arrive to the thread with a timer

        ; switch text for button
        mov     dword[text_log_butt], _start_log
        jmp     still
.err_alloc_mem:
        send_notify Error_text.alloc_1
        jmp     still

.no_log_button:
        cmp     ah, BTN_ID_SAVE_LOG - 1
        jnz     still

        cmp     dword[log_ptr], 0
        jnz     @f
        send_notify Error_text.save_log
        jmp     still
@@:
        cmp     byte[futex_cmd], 2 ; 2 - save file
        jnz     @f
        send_notify Error_text.save_log_1
        jmp     still
@@:
        ; start openDialog window
        invoke  OpenDialog_Start, OpenDialog_data

        cmp     [OpenDialog_data.status], 1
        jne     still

        ;mov      dword[log_path], openfile_path

        ; waking up the flow through the futex
        mov     byte[futex_cmd], 2  ; save_log
        mcall   SF_FUTEX, SSF_WAKE, [futex_handle], 1
        test    eax, eax
        jnz     still

        mcall   SF_CREATE_THREAD, 1, thread_timer, thread_timer.stack
        mcall   SF_SYS_MISC, SSF_SWITCH_TASK ; in order for the message to arrive to the thread with a timer
@@:
        mov     byte[futex_cmd], 3  ; save_log  & exit
        mcall   SF_FUTEX, SSF_WAKE, [futex_handle], 1
        test    eax, eax
        jz      @b
        mcall   SF_SYS_MISC, SSF_SWITCH_TASK ; in order for the message to arrive to the thread with a timer
        jmp     still

align 4
draw:
        mcall   SF_REDRAW, SSF_BEGIN_DRAW
        mcall   SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors

        mov     eax, dword[sc.work_graph]
        mov     dword[frame_struct.FR_OUTER_COLOR], eax
        mov     eax, dword[sc.work_light]
        mov     dword[frame_struct.FR_INNER_COLOR], eax
        mov     eax, dword[sc.work_text]
        mov     dword[frame_struct.FR_FORE_COLOR], eax
        mov     eax, dword[sc.work]
        mov     dword[frame_struct.FR_BACK_COLOR], eax


        mov     eax, SF_CREATE_WINDOW
        mov     ebx, 0x00ff0132
        mov     ecx, 0x00150114   ; link with  80 line
        mov     edx, 0x14000000
        add     edx, [sc.work]
        mov     edi, title
        mcall

        mcall   SF_SYS_MISC, SSF_CONTROL_DRIVER, drv_struct

        cmp     byte[flag_micro_info], 1
        mov     eax, SF_CHANGE_WINDOW
        mov     ebx, -1
        mov     ecx, ebx
        jz      draw.micro

        mov     esi,0x00000115     ; link with 58 line
        mov     edx,0x00000132
        mcall
        ; create button for switch window mode
        mov     eax, SF_DEFINE_BUTTON
        mov     ebx, 0x00fc0010
        mov     ecx, 0x001d0015
        mov     edx, BTN_ID_SW_MODE
        mov     esi, [sc.work_button]
        mcall

        ; print char on button
        mov     eax, SF_DRAW_TEXT
        mov     ebx, 0x01000020
        mov     ecx, DRAW_TEXT_6x9x2
        add     ecx, [sc.work_text]
        mov     edx, _up
        mcall

       ; create button for run/stop log
        mov     eax, SF_DEFINE_BUTTON
        mov     ebx, 0x00a00070
        mov     ecx, 0x004a0015
        mov     edx, BTN_ID_RUN_LOG
        mov     esi, [sc.work_button]
        mcall
        ; print text on button
        mov     eax, SF_DRAW_TEXT
        mov     ebx, 0x00a8004c
        mov     ecx, DRAW_TEXT_UTF8z
        add     ecx, [sc.work_text]
        mov     edx, [text_log_butt]
        mcall

        ; create button for save log in file
        mov     eax, SF_DEFINE_BUTTON
        mov     ebx, 0x00a00070
        mov     ecx, 0x00630015
        mov     edx, BTN_ID_SAVE_LOG
        mov     esi, [sc.work_button]
        mcall
        ; print text on button
        mov     eax, SF_DRAW_TEXT
        mov     ebx, 0x00a80065
        mov     ecx, DRAW_TEXT_UTF8z
        add     ecx, [sc.work_text]
        mov     edx, _save_log
        mcall

        ; "Tctl:"
        mov     ebx, 0x00150035
        mov     ecx, DRAW_TEXT_UTF8z
        add     ecx, [sc.work_text]
        mov     edx, _Tctl
        mcall

        ; "Tmax:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ; "Tcrit hyst:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ; "Tcrit:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ; "Tdie:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ; "Tccd1:"
        add     ebx, 0x2b
        add     edx, 7
        mcall

        ; "Tccd2:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ; "Tccd3:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ; "Tccd4:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ; "Tccd5:"
        mov     ebx, 0x009d00b4
        add     edx, 7
        mcall

        ; "Tccd6:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ; "Tccd7:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ; "Tccd8:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ; input driver data
        mov     eax, 0x004a0035
        mov     ebx, drv_data.Tctl  ;print driver data
        call    write_data

        add     eax, 0x15
        mov     byte[write_data.defaunt_color], 1
        mov     ebx, drv_data.Tmax
        call    write_data

        add     eax, 0x2a
        mov     byte[write_data.defaunt_color], 1
        mov     ebx, drv_data.Tcrit
        call    write_data

        add     eax, 0x40
        mov     ebx, drv_data.Tccd1
        call    write_data

        add     eax, 0x15
        add     ebx, 4
        call    write_data

        add     eax, 0x15
        add     ebx, 4
        call    write_data

        add     eax, 0x15
        add     ebx, 4
        call    write_data

        mov     eax, 0x00d000b4
        add     ebx, 4
        call    write_data

        add     eax, 0x15
        add     ebx, 4
        call    write_data

        add     eax, 0x15
        add     ebx, 4
        call    write_data

        add     eax, 0x15
        add     ebx, 4
        call    write_data

        mov     eax, 0x004a0089
        mov     ebx, drv_data.Tdie
        mov     byte[write_data.defaunt_color], 1
        call    write_data

        mov     eax, 0x004a005f
        mov     byte[write_data.defaunt_color], 1
        mov     ebx, drv_data.Tcrit_hyst
        call    write_data

        mov     dword[frame_struct.FR_WIDTH], 0x0f008a
        mov     dword[frame_struct.FR_HEIGHT], 0x280075
        mov     dword[frame_struct.FR_TEXT], frame_text_1
        invoke  frame_draw, frame_struct

        mov     dword[frame_struct.FR_WIDTH],0x0f0114
        mov     dword[frame_struct.FR_HEIGHT], 0xa70060
        mov     dword[frame_struct.FR_TEXT], frame_text_2
        invoke  frame_draw, frame_struct

        jmp     @f
.micro:
        mov     esi, 0x00000035
        mov     edx, 0x000000c0
        mcall

        assert  (BTN_ID_SW_MODE = 8)
        mov     eax, SF_DEFINE_BUTTON
        mov     ebx, 0x00a70010
        mov     ecx, 0x001d0015
        mov     edx, eax ; BTN_ID_SW_MODE
        mov     esi, [sc.work_button]
        mcall

        mov     eax, SF_DRAW_TEXT
        mov     ebx, 0x000a0020
        mov     ecx, DRAW_TEXT_UTF8z
        add     ecx, [sc.work_text]
        mov     edx, _Tctl
        mcall

        add     ebx, 0x00a00000
        mov     ecx, DRAW_TEXT_6x9x2
        add     ecx, [sc.work_text]
        mov     edx, _down
        mcall

        mov     eax, 0x004a0020
        mov     ebx, drv_data.Tctl
        call    write_data
@@:
        mcall SF_REDRAW, SSF_END_DRAW
        ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; procedure write_data(eax, ebx);
;; IN: eax=x*65536+y
;;     ebx = pointer on value
;;     [.defaunt_color] = 1 - set color, 0 - set default color
;; WARNING:  ecx register don`t save
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
write_data:
        push    eax  ;save koord
        push    ebx  ;save *value
        ;mov edx,[ebx]  ;edx=value
        mov     esi, ebx
        cmp     dword[ebx], -1   ; flag ziro data
        mov     ecx, DRAW_TEXT_UTF8z
        mov     ebx, eax
        jnz     @f
        ; write n/a
        add     ecx, [sc.work_text]
        mov     eax, SF_DRAW_TEXT
        add     ebx, (8*2) shl 16
        mov     edx, _NA
        mcall
        pop     ebx
        pop     eax
        ret
@@:
        ;set color
        push    eax
        push    edx
        push    ecx
        mov     dword[.color_text], 0x000000cf  ;blue
        cmp     byte[.defaunt_color], 1
        je      .end_set_color

        cmp     dword[drv_data.Tcrit], -1
        jz      .end_set_color
        cmp     dword[drv_data.Tcrit], 0
        jz      .end_set_color

        xor     edx, edx
        mov     eax, [esi]
        imul    eax, 10
        xor     edx, edx
        div     dword[drv_data.Tcrit]
        cmp     eax, 9 ; 90% * Tcrit
        jb      @f
        mov     dword[.color_text], 0x00d50000 ; red
        jmp     .end_set_color
@@:
        cmp     eax, 7 ; 70% * Tcrit
        jb      @f
        mov     dword[.color_text], 0x00f07000 ; orange
        jmp     .end_set_color
@@:
        mov     dword[.color_text], 0x0000a500 ; green
.end_set_color:
        pop     ecx
        pop     edx
        pop     eax
       ;write_value
; value / 1000 =value_in_1
;input value_in_1,koord_start
; if value >=100 then dot_kord=4 ; =0x20
;   if value >10 then  dot_kord=3; =0x18
;     dot_kord=2
;input ".", koord_start+dot_koord*size_w(char)
;value-value_in_1 = value_in_2
;input value_in_2, koord_start+dot_kord+1*size_w(char=8)
        push    eax ;save koord
        mov     eax, [esi] ; eax = value
        xor     edx, edx
        mov     ebx, 1000
        div     ebx

        xor     edi, edi
        cmp     eax, 100
        jae     .write

        mov     edi, 8 shl 16
        cmp     eax, 10
        jae     .write

        mov     edi, (2*8) shl 16
.write:
        mov     ecx, edx
        pop     edx     ; edx = koord
        push    ecx     ;save mask string

        push    edx ; save koord
        add     edx, edi
        mov     ecx, eax   ;ecx = value/1000
        mov     ebx, 0x80030000 ; mask sysfn47
        mov     eax, SF_DRAW_NUMBER
        mov     esi, 0x10000000
        add     esi, [.color_text]
        mcall

        pop  ebx ; ebx = koord
        add     ebx, (3*8) shl 16
        mov     eax, SF_DRAW_TEXT
        mov     ecx, DRAW_TEXT_UTF8z
        add     ecx, [.color_text]
        mov     edx, _dot_and_C
        mcall

        mov     eax, SF_DRAW_NUMBER
        mov     edx, ebx
        mov     esi, 0x10000000
        add     esi, [.color_text]
        mov     ebx, 0x00030000

        add     edx, 8 shl 16
        pop     ecx
        mcall

        pop     ebx
        pop     eax
        mov     byte[.defaunt_color], 0
        ret
.color_text:    rd 1 ; color input temp
.defaunt_color: rb 1 ; flag set color
;; end proc

error_drv:
       send_notify Error_text
exit:
        mov     byte[futex_cmd], 2
        mcall   SF_FUTEX, SSF_WAKE, [futex_handle], 1
        mcall   SF_SYS_MISC, SSF_SWITCH_TASK ; in order for the message to arrive to the thread with a timer
        ; destroy futex
        mcall   SF_FUTEX, SSF_DESTROY, [futex_handle]
        mcall   SF_SYS_MISC, SSF_MEM_FREE, [log_ptr]

        mcall   18, 18, dword[PID_AUTO_SAVE]
        mcall   SF_TERMINATE_PROCESS

err_load_lib:
        send_notify Error_text.load_lib
        mcall   SF_TERMINATE_PROCESS

align 4
index_item:
        dd    1

;eax = int   value / 1000
;ebx = *str
align 4
int_to_str:
        push    ecx edx esi
        mov     ecx, '0000'
        mov     [ebx], ecx
        mov     [ebx + 5], ecx

        xor     edx, edx
        mov     esi, 1000
        div     esi
        push    edx
        mov     esi, 10
        push    ebx
        add     ebx, 3
        and     eax, 0x3ff ; 1023
.loop:
        test    eax, eax
        jz      @f
        xor     edx, edx
        div     esi
        add     edx, '0'
        mov     byte[ebx], dl
        dec     ebx
        jmp     .loop
@@:
        pop     ebx
        add     ebx, 8;4
        pop     edx
        test    edx, edx
        jz      @f
        mov     eax, edx
        imul    eax, 10
        xor     edx, edx
        push    edx
        push    ebx
        jmp     .loop
@@:
        pop     esi edx ecx
        ret

thread_timer:
        mcall   SF_SET_EVENTS_MASK, 0x00 ; clear event mask
@@:
        mov     byte[futex_cmd], 0x00
        mcall   SF_FUTEX, SSF_WAIT, [futex_handle],[futex_cmd], 100*60 ; 1 min
        test    eax, eax
        jz      .check_cmd
        cmp     eax, -2
        je      .exit ; error
        ; realloc log
        mov     ecx, [log_size]
        mov     edx, [log_ptr]
        add     ecx, 20 ; size item data
        mcall   SF_SYS_MISC, SSF_MEM_REALLOC
        test    eax, eax
        jz      .err_alloc_2
        mov     [log_ptr], eax
        add     eax, [log_size]
        mov     [log_size], ecx;edx
        mov     edi, eax
        ; get new data temp
        push    edi
        mov     eax,[index_item]
        imul    eax,1000
        mov     ebx, graph_start.new_data
        call    int_to_str

        mov     ebx, [log_ptr]
        mov     eax, [graph_start.new_data]
        mov     [ebx + 2], eax   ; rewrite index x koord

        inc     dword[index_item]
        mov     eax,[drv_data.Tctl]
        mov     ebx, graph_start.new_data_2
        call    int_to_str
        pop     edi
        mov     esi, graph_start.new_data
        ; save data
        movsd
        movsd
        movsd
        movsd
        movsd

        cmp     dword[save_ptr], 0
        jz      .no_save_mode
        push    dword[log_path]
        mov     eax, [save_ptr]
        mov     [log_path], eax
        mcall   SF_FILE, file_log
        pop     dword[log_path]
.no_save_mode:
        jmp     @b
.err_alloc_2:
        send_notify  Error_text.alloc_2
        jmp     @b
.check_cmd:
        test    byte[futex_cmd], 0x02 ; 1- stop 2 - save in file
        jz      .exit
        mcall   SF_FILE, file_log
        test    byte[futex_cmd], 1
        jz      @b
.exit:
        mov     byte[futex_cmd], 0x00
        mcall   -1

thread_auto_save:
        mcall   SF_SET_EVENTS_MASK, 0  ;clear event mask
        ;calculate first data in file(4 value border graphic)
        mov     eax,[drv_data.Tmax]
        mov     ebx, graph_start_1.new_data
        call    int_to_str
        ;create file
        mcall   SF_FILE, .file
        mov     dword[.file], SSF_WRITE_FILE
        mov     dword[.log_size], 20
        mov     dword[.log_offset], 19
        mov     dword[.log_ptr], graph_start_1.new_data
@@:
        mcall   SF_SLEEP, 100*60
        ; add new item in file
        push    dword[.log_offset]
        mov     dword[.log_size], 4
        mov     dword[.log_offset], 2
        mov     eax,[.index_item]
        imul    eax,1000
        mov     ebx, graph_start_1.new_data
        call    int_to_str
        mcall SF_FILE, .file
        inc     dword[.index_item]
        pop     dword[.log_offset]
        mov     dword[.log_size], 20
        ; get string y koord
        mov     eax,[drv_data.Tctl]
        mov     ebx, graph_start_1.new_data_2
        call    int_to_str
        mcall SF_FILE, .file
        add     dword[.log_offset], 20
        jmp     @b
        mcall   -1
.index_item:    dd 1
.file:
                dd SSF_CREATE_FILE
.log_offset:    dd 0
                dd 0
.log_size:      dd 19
.log_ptr:       dd graph_start_1
                db 0
save_ptr:       dd 0 ; pointer to save file or zero
PID_AUTO_SAVE:  dd 0 ; for break this thread
;Data_program;
title       db 'AMDtemp',0
name_drv:   db 'k10temp: ',0
path_drv    db '/kolibrios/drivers/sensors/k10temp.sys',0
Error_text  db '"Error load driver\nk10temp.sys was not found or is faulty " -tdE ',0
.save_log:  db '"Error save log\nThe log has not been created" -tdE',0
.save_log_1:db '"Error save log\nThe log file is already saving" -tdE',0
.alloc_1:   db '"Error alloc memory for log" -tdE',0
.alloc_2:   db '"Error alloc memory for new koord" -tdE',0
.load_lib:  db '"Error load library" -tdE',0

_NA         db 'N/A',0
_dot_and_C  db '.   °C',0x00   ; 8x16 UTF-8
_down       db 0x1f,0x00        ; 6x9  CP866
_up         db 0x1e,0x00        ; 6x9  CP866


_Tctl       db 'Tctl: ',0
_Tmax       db 'Tmax: ',0
_Tcrit_hyst db 'Thyst:',0;'Tcrit hyst:',0
_Tcrit      db 'Tcrit:',0
_Tdie       db 'Tdie: ',0
_Tccd1      db 'Tccd1:',0
_Tccd2      db 'Tccd2:',0
_Tccd3      db 'Tccd3:',0
_Tccd4      db 'Tccd4:',0
_Tccd5      db 'Tccd5:',0
_Tccd6      db 'Tccd6:',0
_Tccd7      db 'Tccd7:',0
_Tccd8      db 'Tccd8:',0

_start_log:     db 'start logging',0
_stop_log:      db 'stop logging',0
_save_log:      db '  save log  ',0
text_log_butt:  dd _start_log

align 4
file_log:
                dd SSF_CREATE_FILE
                dd 0
                dd 0
log_size:       dd 0
log_ptr:        dd 0
                db 0
log_path:       dd openfile_path

futex_handle:   dd 0
futex_cmd:      dd 0 ;1- stop 2 - save in file & no exit 3 - save in file & exit

drv_path_ptr:   dd path_drv ; pointer to path on driver
drv_name_ptr:   dd name_drv
null_str:       db 0 ; string? length = 0

frame_text_1:   db 'General info',0
frame_text_2:   db 'Extended info',0

align 16
@IMPORT:

library box_lib         , 'box_lib.obj', \
        proc_lib        , 'proc_lib.obj'

import  proc_lib, \
        OpenDialog_Init     , 'OpenDialog_init',\
        OpenDialog_Start    , 'OpenDialog_start'

import  box_lib,\
        frame_draw      , 'frame_draw'

frame_struct:
                        dd      0;FR_STYLE     not using
.FR_WIDTH               dw      0x8a;FR_WIDTH
                        dw      0x0f;FR_LEFT start x
.FR_HEIGHT              dw      0x75;FR_HEIGHT
                        dw      0x28;FR_TOP  start y
.FR_OUTER_COLOR         dd      0xff;FR_OUTER_COLOR
.FR_INNER_COLOR         dd      0xff00;FR_INNER_COLOR
                        dd      00001b;FR_FLAGS
                        ;FR_CAPTION equ 00001b draw caption for this frame
.FR_TEXT                dd      0;FR_TEXT   pointer to text
                        dd      0;FR_TEXT_POSITION   0 - top text 1 - bottom text
                        dd      3;FR_FONT   text encoding
                        dd      0x0c;FR_FONT_HEIGHT
.FR_FORE_COLOR          dd      0xff;FR_FORE_COLOR     text color
.FR_BACK_COLOR          dd      0xffffff;FR_BACK_COLOR     text bg color

;;flag mode input data
flag_micro_info db 1
drv_data:
.Tctl           dd -1
.Tdie           dd -1
.Tccd1          dd -1
.Tccd2          dd -1
.Tccd3          dd -1
.Tccd4          dd -1
.Tccd5          dd -1
.Tccd6          dd -1
.Tccd7          dd -1
.Tccd8          dd -1

.Tmax           dd -1
.Tcrit          dd -1
.Tcrit_hyst     dd -1
.sizeof = $ - drv_data ;

align 4
drv_struct:
.handl          dd 0
                dd 0
                dd 0
                dd 0
                dd drv_data
                dd drv_data.sizeof;52 ; 13*4
align 4
run_notify:
                dd SSF_START_APP
                dd 0
.message:       dd Error_text
                dd 0
                dd 0
                db '/sys/@notify',0
align 4
graph_start_1:  db '0 0000 0 '  ; 9 byte
.new_data:      db '0000.0000 ' ;  10-19 byte  10 byte
.new_data_2:    db '0000.0000 ' ;  20-29 byte  10 byte
align 4
graph_start:    db '0 0000 0 '  ; 9 byte
.new_data:      db '0000.0000 ' ;  10-19 byte  10 byte
.new_data_2:    db '0000.0000 ' ;  20-29 byte  10 byte

; opendialog struct
align 4
OpenDialog_data:
.type                   dd 1 ;0 - open, 1 - save, 2 - select a directory
.procinfo               dd procinfo ;+4  for getting thread data
.com_area_name          dd communication_area_name ;+8
.com_area               dd 0 ;+12
.opendir_path           dd 0 ;+16
.dir_default_path       dd default_dir ;+20 path to directiry, for opendir_path=0 mode
.start_path             dd opendialog_path ;+24 path to opendialog
.draw_window            dd draw ;+28 ptr to redraw window function
.status                 dd 0 ;+32  0, 2 - exit or error 1 - user pressed "OK"
.openfile_path          dd openfile_path ;+36 ptr to buffer for file path
.filename_area          dd filename_area ;+40 ptr to buffer for file name
.filter_area            dd 0;Filter
.x:
.x_size                 dw 420 ;+48 ; Window X size
.x_start                dw 10 ;+50 ; Window X position
.y:
.y_size                 dw 320 ;+52 ; Window y size
.y_start                dw 10 ;+54 ; Window Y position

default_dir:     db '/sys',0 ; starting directory in opendial

communication_area_name:
        db 'FFFFFFFF_open_dialog',0
opendialog_path:
        db '/sys/File managers/opendial',0
filename_area:
        db 'temp1.grf',0
        rb 256

align 4
PATH:
        rb 256 ; buffer for command line. string for save log.
sc      system_colors
I_END:
   rd 256
STACKTOP:

        rb      512 ; 512 byte for stack
thread_timer.stack:
        rb      512 ; 512 byte for stack
thread_auto_save.stack:
procinfo process_information
openfile_path:
        rb 4096
MEM: