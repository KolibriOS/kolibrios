  use32
  org    0

  db     'MENUET01'
  dd     1
  dd     START
  dd     I_END       ; а §¬Ґа Їа®Ја ¬¬л
  dd     MEM         ; Є®«ЁзҐбвў® Ї ¬пвЁ
  dd     STACKTOP
  dd     0
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
       jnz  still
       jmp error_drv
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
draw:
       mcall SF_REDRAW, SSF_BEGIN_DRAW
       mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sc,sizeof.system_colors

       mov eax,SF_CREATE_WINDOW
       mov ebx,0x00ff013f
       mov ecx,0x001500cc
       mov edx,0x14000000
       add edx,[sc.work]
       mov edi,title
       mcall

       mcall SF_SYS_MISC, SSF_CONTROL_DRIVER, drv_struct

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
       mov ebx,drv_data.Tctl  ;вывод данных от драйвера
       call write_data

       add eax,0x15
       mov ebx,drv_data.Tmax
       call write_data

       add eax,0x2a
       mov ebx,drv_data.Tcrit
       call write_data

       add eax,0x19
       mov ebx,drv_data.Tccd1
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
       mov ebx,drv_data.Tdie
       call write_data

       mov eax,0x007a004a
       mov ebx,drv_data.Tcrit_hyst
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
       mov ebx,drv_data.Tctl
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

drv_struct:
.handl           dd 0
                 dd 0
                 dd 0
                 dd 0
                 dd drv_data
                 dd drv_data.sizeof;52 ; 13*4

run_notify:
                 dd 7
                 dd 0
                 dd Error_text
                 dd 0
                 dd 0
                 db '/sys/@notify',0

sc      system_colors
I_END:
   rd 256
STACKTOP:
MEM: