;;      Copyright (C) 2021-2022, Michael Frolov aka Doczom

macro send_notify send_str{
      mov     dword[run_notify.message], send_str
      mcall   SF_FILE, run_notify
}

  use32
  org    0

  db     'MENUET01'
  dd     1
  dd     START
  dd     I_END
  dd     MEM
  dd     STACKTOP
  dd     PATH
  dd     0

include '..\..\proc32.inc'
include '..\..\macros.inc'
include '..\..\KOSfuncs.inc'
include '..\..\dll.inc'

include 'parser.inc'
START:
;init heap
        mcall   68, 11
; load lib
        stdcall dll.Load, @IMPORT
        or      eax, eax
        jnz     err_load_lib
;       init opendial
        invoke OpenDialog_Init, OpenDialog_data
; init futex
        mcall   SF_FUTEX, SSF_CREATE, futex_cmd
        mov     [futex_handle], eax
; parse cmd_line
        cmp     byte[PATH], 0
        jz      @f
        call    parse_cmd
@@:
;;установка маска событий на получение переписовки и нажатия на кнопку
        mcall   SF_SET_EVENTS_MASK, 0x05
;load driver
        mcall   SF_SYS_MISC, SSF_LOAD_DRIVER_PE, [drv_ptr], 0
        mov     dword[drv_struct.handl], eax
        test    eax, eax ;проверка загрузки
        jz      error_drv
main:
        mcall   SF_SYS_MISC, SSF_CONTROL_DRIVER, drv_struct
        cmp     dword[save_ptr], 0
        jz      still
        mcall   51, 1, thread_auto_save, thread_auto_save.stack
        cmp     eax, -1
        jz      still
        mov     dword[PID_AUTO_SAVE], eax
still:          ;void main()
        call    draw
;;ожидание события в течении 2000мс
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
;; id button micro_info = 8
        cmp     ah, 7 ;// ah-1=7
        jnz     .no_micro_info
        xor     byte[flag_micro_info], 1
        jmp     still
.no_micro_info:
; id button log = 9
        cmp     ah, 8 ; 9-1
        jnz     .no_log_button
        cmp     dword[text_log_butt], _start_log
        jnz     .log_stop

        cmp     dword[log_ptr], 0
        jnz     @f
        mcall   68, SSF_MEM_ALLOC, 19 ; начальные данные для Graph
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
        mcall   51, 1, thread_timer, thread_timer.stack
        mov     dword[text_log_butt], _stop_log
        jmp     still
.log_stop:
        ; пробуждаем поток через фьютекс
        mov     byte[futex_cmd], 1  ; stop thread
        mcall   SF_FUTEX, SSF_WAKE, [futex_handle], 1
        mcall   68, 1 ; переключаем потоки, для того, чтобы сообщение пришло потоку с таймером

        ; меняем текст в кнопке
        mov     dword[text_log_butt], _start_log
        jmp     still
.err_alloc_mem:
        send_notify Error_text.alloc_1
        jmp     still
.no_log_button:
; id button save = 10
        cmp     ah, 9 ; 10-1
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
        ; тут вызов функции openDialog
        invoke  OpenDialog_Start, OpenDialog_data

        cmp     [OpenDialog_data.status], 1
        jne     still

        ;mov      dword[log_path], openfile_path

        ; пробуждаем поток через фьютекс
        mov     byte[futex_cmd], 2  ; save_log
        mcall   SF_FUTEX, SSF_WAKE, [futex_handle], 1
        test    eax, eax
        jnz     still

        mcall   51, 1, thread_timer, thread_timer.stack
        mcall   68, 1 ; переключаем потоки, для того, чтобы сообщение пришло потоку с таймером
@@:
        mov     byte[futex_cmd], 3  ; save_log  & exit
        mcall   SF_FUTEX, SSF_WAKE, [futex_handle], 1
        test    eax, eax
        jz      @b
        mcall   68, 1 ; переключаем потоки, для того, чтобы сообщение пришло потоку с таймером
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
;;создание кнопки переключения режима
        mov     eax, SF_DEFINE_BUTTON
        mov     ebx, 0x00fc0010
        mov     ecx, 0x001d0015
        mov     edx, 0x00000008
        mov     esi, [sc.work_button]
        mcall
;;вывод знака на кнопку
;;так как функция writeText не должна изменять регистры
;;присвоения в регистры eax и ecx происходят только 1 раз
;; Далее, так как текст выводится почти всегда с одинаковым
;; смещением вниз(равным 15)  дынные(строки) имеют одинаковый
;; размер , то используется инструкция add
        mov     eax, SF_DRAW_TEXT
        mov     ebx, 0x01000020
        mov     ecx, 0x81000000
        add     ecx, [sc.work_text]
        mov     edx, _up
        mcall

       ;создание кнопки запуска/выключения лога
        mov     eax, SF_DEFINE_BUTTON
        mov     ebx, 0x00a00070
        mov     ecx, 0x004a0015
        mov     edx, 0x00000009 ;id button
        mov     esi, [sc.work_button]
        mcall
        ; вывод текста
        mov     eax, SF_DRAW_TEXT
        mov     ebx, 0x00a8004c
        mov     ecx, 0x90000000
        add     ecx, [sc.work_text]
        mov     edx, [text_log_butt]
        mcall

        ;создание кнопки сохранения лога в файл
        mov     eax, SF_DEFINE_BUTTON
        mov     ebx, 0x00a00070
        mov     ecx, 0x00630015
        mov     edx, 0x0000000a ; id=10
        mov     esi, [sc.work_button]
        mcall
        ; вывод текста
        mov     eax, SF_DRAW_TEXT
        mov     ebx, 0x00a80065
        mov     ecx, 0x90000000
        add     ecx, [sc.work_text]
        mov     edx, _save_log
        mcall

        ;вывод "Tctl:"
        mov     ebx, 0x00150035
        mov     ecx, 0x90000000
        add     ecx, [sc.work_text]
        mov     edx, _Tctl
        mcall

        ;вывод "Tmax:"
        add      ebx, 0x15
        add      edx, 7
        mcall

        ;вывод "Tcrit hyst:"
        ;mov ebx,0x0015005f
        add     ebx, 0x15
        add     edx, 7
        mcall

        ;вывод "Tcrit:"
        add     ebx, 0x15;0x2a
        add     edx, 7
        mcall

        ;вывод "Tdie:"
        ;mov ebx,0x00150089 ;0x00a5005f
        add      ebx, 0x15
        add      edx, 7
        mcall

        ;вывод "Tccd1:"
        add     ebx, 0x2b;0x19
        add     edx, 7
        mcall

        ;вывод "Tccd2:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ;вывод "Tccd3:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ;вывод "Tccd4:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ;вывод "Tccd5:"
        mov     ebx, 0x009d00b4
        add     edx, 7
        mcall

        ;вывод "Tccd6:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ;вывод "Tccd7:"
        add     ebx, 0x15
        add     edx, 7
        mcall

        ;вывод "Tccd8:"
        add     ebx, 0x15
        add     edx, 7
        mcall
;;;;;;input data driver;;;;;;;;;
        mov     eax, 0x004a0035  ;0x004a0020
        mov     ebx, drv_data.Tctl  ;вывод данных от драйвера
        call    write_data

        add     eax, 0x15
        mov     byte[write_data.defaunt_color], 1
        mov     ebx, drv_data.Tmax
        call    write_data

        add     eax, 0x2a
        mov     byte[write_data.defaunt_color], 1
        mov     ebx, drv_data.Tcrit
        call    write_data

        add     eax, 0x40;0x19
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

        mov     eax, SF_DEFINE_BUTTON
        mov     ebx, 0x00a70010
        mov     ecx, 0x001d0015
        mov     edx, eax;0x00000008
        mov     esi, [sc.work_button]
        mcall

        mov     eax, SF_DRAW_TEXT
        mov     ebx, 0x000a0020
        mov     ecx, 0x90000000
        add     ecx, [sc.work_text]
        mov     edx, _Tctl
        mcall

        add     ebx, 0x00a00000
        mov     ecx, 0x81000000
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
;; eax=x*65536+y
;; ebx = pointer on value
;; edx = 1 - set color, 0 - set defaunt color
;; ecx register don`t save
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
write_data:
        push    eax  ;save koord
        push    ebx  ;save *value
        ;mov edx,[ebx]  ;edx=value
        mov     esi, ebx
        cmp     dword[ebx], -1   ; flag ziro data
        mov     ecx, 0x90000000
        mov     ebx, eax
        jnz     @f
        ;write n/a
        add     ecx, [sc.work_text]
        mov     eax, SF_DRAW_TEXT
        add     ebx, 0x00100000
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
        jnz     @f
        ;mov  dword[.color_text], 0x000000cf  ;blue
        jmp     .end_set_color
@@:
        cmp     dword[drv_data.Tcrit], -1
        jz      .end_set_color
        cmp     dword[drv_data.Tcrit], 0
        jz      .end_set_color

        xor     edx, edx
        mov     eax, [esi]
        imul    eax, 10
        xor     edx, edx
        div     dword[drv_data.Tcrit]
        cmp     eax, 9
        jb      @f
        mov     dword[.color_text], 0x00d50000 ; red
        jmp     .end_set_color
@@:
        cmp     eax, 7
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
        cmp     eax, 100
        jae     .dot_4
        cmp     eax, 10
        jae     .dot_3    ; ----|
        mov     edi, 0x10 ;     |
        jmp     .write    ;     |
.dot_4:                   ;     |
        xor     edi, edi  ;     |
        jmp     .write    ;     |
.dot_3:                   ;   <-|
        mov     edi, 8
.write:
        mov     ecx, edx
        pop     edx     ; edx = koord
        push    ecx     ;save mask string

        shl     edi, 16
        push    edx ; save koord
        add     edx, edi
        mov     ecx, eax   ;ecx = value/1000
        mov     ebx, 0x80030000 ; ebx = mask sysfn47
        mov     eax, SF_DRAW_NUMBER
        mov     esi, 0x10000000
        add     esi, [.color_text]
        mcall

        pop  ebx ; ebx = koord
        add     ebx, 0x00180000 ; x + x_size_char*3
        mov     eax, SF_DRAW_TEXT
        mov     ecx, 0x90000000
        add     ecx, [.color_text]
        mov     edx, _dot
        mcall

        mov     eax, SF_DRAW_NUMBER
        mov     edx, ebx
        mov     esi, 0x10000000
        add     esi, [.color_text]
        mov     ebx, 0x00030000

        add     edx, 0x80000
        pop     ecx
        mcall
        mov     eax, SF_DRAW_TEXT
        mov     ebx, edx
        mov     ecx, 0x80000000
        add     ecx, [.color_text]
        add     ebx, 0x180000
        mov     edx, _t
        mcall

        mov     ecx, 0x90000000
        add     ecx, [.color_text]
        add     ebx, 0x60000
        mov     edx, _C
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
        mov      byte[futex_cmd], 2
        mcall    SF_FUTEX, SSF_WAKE, [futex_handle], 1
        mcall    68, 1 ; переключаем потоки, для того, чтобы сообщение пришло потоку с таймером
        ; destroy futex
        mcall    SF_FUTEX, SSF_DESTROY, [futex_handle]
        mcall    68, 13, [log_ptr] ; free page

        mcall    18, 18, dword[PID_AUTO_SAVE]
        mcall SF_TERMINATE_PROCESS

err_load_lib:
        send_notify Error_text.load_lib
        mcall SF_TERMINATE_PROCESS

align 4
index_item:
        dd    1

;eax = int   value / 1000
;ebx = *str
; из за конкретики данного прилажения(а именно измерение температуры проца), сомниваюсь
; что потребуется больше 3 цифр на значение(ххх.ххх) так что будет костыль
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
        mcall   40, 0x00 ; очищаем маску событий
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
        mov     ebx, SSF_MEM_REALLOC ; 20
        mcall   68
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
        mcall   70, file_log
        pop     dword[log_path]
.no_save_mode:
        jmp     @b
.err_alloc_2:
        send_notify  Error_text.alloc_2
        jmp     @b
.check_cmd:
        test    byte[futex_cmd], 0x02 ; 1- stop 2 - save in file
        jz      .exit
        mcall   70, file_log
        test    byte[futex_cmd], 1
        jz      @b
.exit:
        mov     byte[futex_cmd], 0x00
        mcall   -1

thread_auto_save:
        mcall   40, 0  ;clear event mask
        ;calculate first data in file(4 value border graphic)
        mov     eax,[drv_data.Tmax]
        mov     ebx, graph_start_1.new_data
        call    int_to_str
        ;create file
        mcall   70, .file
        mov     dword[.file], 3
        mov     dword[.log_size], 20
        mov     dword[.log_offset], 19
        mov     dword[.log_ptr], graph_start_1.new_data
@@:
        mcall   5, 100*60
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
                dd 2
.log_offset:    dd 0
                dd 0
.log_size:      dd 19
.log_ptr:       dd graph_start_1
                db 0
save_ptr:       dd 0 ; pointer to save file or zero
PID_AUTO_SAVE:  dd 0 ; for break this thread
;Data_program;
title       db 'AMDtemp',0
path_drv    db '/kolibrios/drivers/sensors/k10temp.sys',0
Error_text  db '"Error load driver\nk10temp.sys was not found or is faulty " -tdE ',0
.save_log:  db '"Error save log\nThe log has not been created" -tdE',0
.save_log_1:db '"Error save log\nThe log file is already saving" -tdE',0
.alloc_1:   db '"Error alloc memory for log" -tdE',0
.alloc_2:   db '"Error alloc memory for new koord" -tdE',0
.load_lib:  db '"Error load library" -tdE',0

_NA         db 'N/A',0
_dot        db '.',0
_t          db 0x1d,0x00
_C          db 'C',0x00
_down       db 0x1f,0x00
_up         db 0x1e,0x00


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

_start_log:     db 'start loging',0
_stop_log:      db 'stop loging ',0
_save_log:      db '  save log  ',0
text_log_butt:  dd _start_log

align 4
file_log:
                dd 2
                dd 0
                dd 0
log_size:       dd 0
log_ptr:        dd 0
                db 0
log_path:       dd openfile_path

futex_handle:     dd 0
futex_cmd:        dd 0 ;1- stop 2 - save in file & no exit 3 - save in file & exit

drv_ptr:          dd path_drv ; pointer to path on driver

frame_text_1:     db 'General info',0
frame_text_2:     db 'Extended info',0

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
                       dd      0;FR_STYLE     в коде либы не используется
.FR_WIDTH              dw      0x8a;FR_WIDTH
                       dw      0x0f;FR_LEFT start x
.FR_HEIGHT             dw      0x75;FR_HEIGHT
                       dw      0x28;FR_TOP  start y
.FR_OUTER_COLOR        dd      0xff;FR_OUTER_COLOR   внешний окрас линии  при флаге x000z
.FR_INNER_COLOR        dd      0xff00;FR_INNER_COLOR внутрений окрас линии при флаге  x000z
                       dd      00001b;FR_FLAGS
                       ;FR_CAPTION equ 00001b показывать заголовок или нет
                       ;FR_FILLED  equ 10000b закрашивает внутрению область цветом фона текста
.FR_TEXT               dd      0;FR_TEXT   тут и так понятно
                       dd      0;FR_TEXT_POSITION   0 - сверху текст 1 - снизу текст
                       dd      1;FR_FONT
                       dd      0x0c;FR_FONT_HEIGHT     смещение координаты y выводимого текста
.FR_FORE_COLOR         dd      0xff;FR_FORE_COLOR     цвет текста
.FR_BACK_COLOR         dd      0xffffff;FR_BACK_COLOR     фон символов текста

;;flag mode input data
flag_micro_info db 1
drv_data:
.Tctl            dd -1
.Tdie            dd -1
.Tccd1           dd -1
.Tccd2           dd -1
.Tccd3           dd -1
.Tccd4           dd -1
.Tccd5           dd -1
.Tccd6           dd -1
.Tccd7           dd -1
.Tccd8           dd -1

.Tmax            dd -1
.Tcrit           dd -1
.Tcrit_hyst      dd -1
.sizeof = $ - drv_data ;

align 4
drv_struct:
.handl           dd 0
                 dd 0
                 dd 0
                 dd 0
                 dd drv_data
                 dd drv_data.sizeof;52 ; 13*4
align 4
run_notify:
                 dd 7
                 dd 0
.message:        dd Error_text
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

;дынные для диалога открытия файлов
align 4
OpenDialog_data:
.type                   dd 1 ;0 - открыть, 1 - сохранить, 2 - выбрать директорию
.procinfo               dd procinfo ;+4  это для получения данных потока
.com_area_name          dd communication_area_name ;+8
.com_area               dd 0 ;+12     как я понял, сюда будет записан указатель на расшареную память
.opendir_path           dd 0 ;+16  путь к папке, которая будет при открытии компонента
.dir_default_path       dd default_dir ;+20 путь к папке, если opendir_path=0
.start_path             dd opendialog_path ;+24 путь к opendialog
.draw_window            dd draw ;+28 функция перерисовки окна, вызвавшего opendialog
.status                 dd 0 ;+32  0, 2 - выход или ошибка 1 - юзер нажал OK
.openfile_path          dd openfile_path ;+36 указатель на буфер для путь к открываемому файлу
.filename_area          dd filename_area ;+40 указатель на буфер для названия файла
.filter_area            dd 0;Filter  указатель на массив фильторов поиска
.x:
.x_size                 dw 420 ;+48 ; Window X size
.x_start                dw 10 ;+50 ; Window X position
.y:
.y_size                 dw 320 ;+52 ; Window y size
.y_start                dw 10 ;+54 ; Window Y position

;формат фильтров
;Filter:
;dd Filter.end - Filter.1
;.1:
;db 'ASM',0
;db 'INC',0
;db 'TXT',0
;.end:
;db 0


default_dir db '/sys',0 ;директория по умолчанию

communication_area_name:
        db 'FFFFFFFF_open_dialog',0
opendialog_path:
        db '/sys/File managers/opendial',0
filename_area:
        db 'temp1.grf',0
                rb 256

align 4
PATH:
   rb 512 ; buffer for command line. string for save log.
sc      system_colors
I_END:
   rd 256
STACKTOP:

        rb      512 ; 512 byte for stack
thread_timer.stack:
        rb      512 ; 512 byte for stack
thread_auto_save.stack:
;rb 1024
        procinfo process_information
        openfile_path:
                rb 4096
MEM: