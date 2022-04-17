  use32
  org    0

  db     'MENUET01'
  dd     1
  dd     START
  dd     I_END       ; а §¬Ґа Їа®Ја ¬¬л
  dd     MEM         ; Є®«ЁзҐбвў® Ї ¬пвЁ
  dd     STACKTOP
  dd     PATH
  dd     0
include '..\..\macros.inc'
include '..\..\KOSfuncs.inc'

START:
;;установка маска событий на получение переписовки и нажатия на кнопку
       mcall SF_SET_EVENTS_MASK,0x05
;load driver
       mcall SF_SYS_MISC, SSF_LOAD_DRIVER_PE, path_drv, 0
       mov dword[drv_struct.handl],eax
       test eax,eax ;проверка загрузки
       jz  error_drv
main:
       mcall SF_SYS_MISC, SSF_CONTROL_DRIVER, drv_struct
       cmp   byte[PATH], 0
       jz    still
       call add_file
still:           ;void main()
       call draw
;;ожидание события в течении 2000мс
       mcall SF_WAIT_EVENT_TIMEOUT,200 ;2 second
       dec eax                   ; redraw request ?
       je   still
       cmp eax,2             ;button
       jz button
       jmp still

button:
       mcall SF_GET_BUTTON

       cmp eax,1
       jz still
       dec ah
       je exit
;; id button micro_info = 8
       cmp ah,7 ;// ah-1=7
       jnz still
       xor byte[flag_micro_info],1
       jmp still

align 4
draw:
       mcall SF_REDRAW, SSF_BEGIN_DRAW
       mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors

       mov eax,SF_CREATE_WINDOW
       mov ebx,0x00ff013f
       mov ecx,0x001500cc
       mov edx,0x14000000
       add edx,[sc.work]
       mov edi,title
       mcall

       mcall SF_SYS_MISC, SSF_CONTROL_DRIVER, drv_struct

       cmp       byte[PATH], 0
       jz        @f
       dec       dword[update_flag]
       jnz       @f
       call      add_new_item
       mov       dword[update_flag], 30
@@:

       cmp byte[flag_micro_info],1
       mov eax,SF_CHANGE_WINDOW
       mov ebx,-1
       mov ecx,ebx;-1
       jz draw.micro

       mov esi,0x000000cc
       mov edx,0x0000013f
       mcall
;;создание кнопки переключения режима
       mov eax,SF_DEFINE_BUTTON
       mov ebx,0x01250010
       mov ecx,0x001d0015
       mov edx,0x00000008
       mov esi,[sc.work_button]
       mcall
;;вывод знака на кнопку
;;так как функция writeText не должна изменять регистры
;;присвоения в регистры eax и ecx происходят только 1 раз
;; Далее, так как текст выводится почти всегда с одинаковым
;; смещением вниз(равным 15)  дынные(строки) имеют одинаковый
;; размер , то используется инструкция add
       mov eax,SF_DRAW_TEXT
       mov ebx,0x01290020
       mov ecx,0x81000000
       add ecx,[sc.work_text]
       mov edx,_up
       mcall

       ;вывод "Tctl:"
       mov ebx,0x000a0020
       mov ecx,0x90000000
       add ecx,[sc.work_text]
       mov edx,_Tctl
       mcall

       ;вывод "Tmax:"
       add ebx,0x15
       add edx,7
       mcall

       ;вывод "Tcrit:"
       add ebx,0x2a
       add edx,7
       mcall

       ;вывод "Tccd1:"
       add ebx,0x19
       add edx,7
       mcall

       ;вывод "Tccd2:"
       add ebx,0x15
       add edx,7
       mcall

       ;вывод "Tccd3:"
       add ebx,0x15
       add edx,7
       mcall

       ;вывод "Tccd4:"
       add ebx,0x15
       add edx,7
       mcall

       ;вывод "Tccd5:"
       mov ebx,0x00a50078
       add edx,7
       mcall

       ;вывод "Tccd6:"
       add ebx,0x15
       add edx,7
       mcall

       ;вывод "Tccd7:"
       add ebx,0x15
       add edx,7
       mcall

       ;вывод "Tccd8:"
       add ebx,0x15
       add edx,7
       mcall

       ;вывод "Tdie:"
       mov ebx,0x00a5005f
       add edx,7
       mcall

       ;вывод "Tcrit hyst:"
       mov ebx,0x000a004a
       add edx,7
       mcall
;;;;;;input data driver;;;;;;;;;
       mov eax,0x004a0020
       mov ebx, drv_data.Tctl  ;вывод данных от драйвера
       call write_data

       add eax,0x15
       mov ebx, drv_data.Tmax
       call write_data

       add eax,0x2a
       mov ebx, drv_data.Tcrit
       call write_data

       add eax,0x19
       mov ebx, drv_data.Tccd1
       call write_data

       add eax,0x15
       add ebx,4
       call write_data

       add eax,0x15
       add ebx,4
       call write_data

       add eax,0x15
       add ebx,4
       call write_data

       mov eax,0x00E50078
       add ebx,4
       call write_data

       add eax,0x15
       add ebx,4
       call write_data

       add eax,0x15
       add ebx,4
       call write_data

       add eax,0x15
       add ebx,4
       call write_data

       mov eax,0x00E5005f
       mov ebx, drv_data.Tdie
       call write_data

       mov eax,0x007a004a
       mov ebx, drv_data.Tcrit_hyst
       call write_data
       jmp @f
.micro:
       mov esi,0x00000035
       mov edx,0x000000c0
       mcall

       mov eax,SF_DEFINE_BUTTON
       mov ebx,0x00a70010
       mov ecx,0x001d0015
       mov edx,eax;0x00000008
       mov esi,[sc.work_button]
       mcall

       mov eax,SF_DRAW_TEXT;4
       mov ebx,0x000a0020
       mov ecx,0x90000000
       add ecx,[sc.work_text]
       mov edx,_Tctl
       mcall

       add ebx,0x00a00000
       mov ecx,0x81000000
       add ecx,[sc.work_text]
       mov edx,_down
       mcall

       mov eax,0x004a0020
       mov ebx, drv_data.Tctl
       call write_data
@@:
       mcall SF_REDRAW, SSF_END_DRAW
       ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; procedure write_data(eax, ebx);
;; eax=x*65536+y
;; ebx=pointer on value
;; ecx register don`t save
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
write_data:
       push eax
       push ebx
       ;mov edx,[ebx]  ;edx=value
       mov esi,ebx
       cmp dword[ebx],-1   ; flag ziro data
       mov ecx,0x90000000
       mov ebx,eax
       jnz @f
       ;write n/a
       add ecx,[sc.work_text]
       mov eax,SF_DRAW_TEXT
       mov edx,_NA
       mcall
       pop ebx
       pop eax
       ret
@@:
       ;write_value
; value / 1000 =value_in_1
;input value_in_1,koord_start
; if value >=100 then dot_kord=4 ; =0x20
;   if value >10 then  dot_kord=3; =0x18
;     dot_kord=2
;input ".", koord_start+dot_koord*size_w(char)
;value-value_in_1 = value_in_2
;input value_in_2, koord_start+dot_kord+1*size_w(char=8)
       push eax
       mov eax,[esi]
       xor edx,edx
       mov ebx,1000
       div ebx
       cmp eax,100
       jae .dot_4
       cmp eax,10
       jae .dot_3       ; ----|
       mov edi,0x0c;0x10;     |
       jmp .write       ;     |
.dot_4:                 ;     |
       mov edi,0x1c;0x20;     |
       jmp .write       ;     |
.dot_3:                 ;   <-|
       mov edi,0x14;0x18
.write:
       mov ecx,edx
       pop edx
       push ecx

       mov ecx,eax
       mov ebx,0x80030000
       mov eax,SF_DRAW_NUMBER
       mov esi,0x10000000
       add esi,[sc.work_text]
       mcall
       shl edi,16
       mov eax,SF_DRAW_TEXT
       add edx,edi
       mov ebx,edx

       mov ecx,0x90000000
       add ecx,[sc.work_text]
       mov edx,_dot
       mcall
       mov eax,SF_DRAW_NUMBER
       mov edx,ebx
       mov esi,0x10000000
       add esi,[sc.work_text]
       mov ebx,0x00030000

       add edx,0x80000
       pop ecx
       mcall
       mov eax,SF_DRAW_TEXT
       mov ebx,edx
       mov ecx,0x80000000
       add ecx,[sc.work_text]
       add ebx,0x180000
       mov edx,_t
       mcall

       mov ecx,0x90000000
       add ecx,[sc.work_text]
       add ebx,0x60000
       mov edx,_C
       mcall
       pop ebx
       pop eax
       ret

error_drv:
       mcall SF_FILE,run_notify
exit:
       mcall SF_TERMINATE_PROCESS

align 4
add_file:
        mcall SF_FILE, graph_temp
        mov     dword[graph_temp], 3
        mov     dword[graph_temp.size], 10
        mov     dword[graph_temp.str], graph_start.new_data
        mov     eax,[drv_data.Tmax]
        mov     dword[graph_temp.index], 9
        mov     ebx, graph_start.new_data
        call    int_to_str
        mcall SF_FILE, graph_temp

        mov     dword[graph_temp.index], 19
        mov     dword[graph_temp.size], 20
        ret
align 4
index_item:
        dd    1
add_new_item:
        mov     eax,[index_item]
        imul    eax,1000
        mov     ebx, graph_start.new_data
        call    int_to_str
        mcall SF_FILE, graph_koord_x2 ;save x2=index_item
        inc     dword[index_item]

        mov     eax,[drv_data.Tctl]
        mov     ebx, graph_start.new_data_2
        call    int_to_str
        mcall SF_FILE, graph_temp
        add     dword[graph_temp.index], 20
        ret

;eax = int   value / 1000
;ebx = *str
; из за конкретики данного прилажения(а именно измерение температуры проца), сомниваюсь
; что потребуется больше 3 цифр на значение(ххх.ххх) так что будет костыль
align 4
int_to_str:
        push    ecx edx esi
        mov     ecx, '0000'
        mov     [ebx], ecx
        mov     [ebx+5], ecx

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
;Data_program;
title       db 'AMDtemp',0
path_drv    db '/kolibrios/drivers/sensors/k10temp.sys',0
Error_text  db '"Error load driver\nk10temp.sys was not found or is faulty " -tdE ',0
_NA         db 'N/A',0
_dot        db '.',0
_t          db 0x1d,0x00
_C          db 'C',0x00
_down       db 0x1f,0x00
_up         db 0x1e,0x00


_Tctl       db 'Tctl: ',0
_Tmax       db 'Tmax: ',0
_Tcrit      db 'Tcrit:',0
_Tccd1      db 'Tccd1:',0
_Tccd2      db 'Tccd2:',0
_Tccd3      db 'Tccd3:',0
_Tccd4      db 'Tccd4:',0
_Tccd5      db 'Tccd5:',0
_Tccd6      db 'Tccd6:',0
_Tccd7      db 'Tccd7:',0
_Tccd8      db 'Tccd8:',0
_Tdie       db 'Tdie: ',0
_Tcrit_hyst db 'Tcrit hyst:',0

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
                 dd Error_text
                 dd 0
                 dd 0
                 db '/sys/@notify',0 ,0,0,0 ;выравнивание

align 4
update_flag:    dd 30 ;1 minut
graph_start:    db '0 0000 0 '  ; 9 byte
.new_data:      db '0000.0000 ' ;  10-19 byte  10 byte
.new_data_2:    db '0000.0000 ' ;  20-29 byte  10 byte

align 4
graph_temp:
                dd 2
.index:         dd 0
                dd 0
.size:          dd 19 ;size 4 first value for Graph
.str:           dd graph_start ; заменить
                db 0
                dd PATH
align 4
graph_koord_x2:
                dd 3
                dd 2 ;index for rewrite 2 value
                dd 0
                dd 4
                dd graph_start.new_data ; заменить
                db 0
                dd PATH
align 4
PATH:
   rb 512 ; buffer for command line. string for save log.
sc      system_colors
I_END:
   rd 256
STACKTOP:
MEM: