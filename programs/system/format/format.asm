;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Formatting Disk Utility ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compile with FASM
; FORMAT ver. Oct 16, 2018

; Copyright (c) 2018, Efremenkov Sergey aka TheOnlyMirage
; All rights reserved.
; Redistribution and use in source and binary forms, with or without modification,
; are permitted provided that the following conditions are met:
;    * Redistributions of source code must retain the above copyright notice, this
;    list of conditions and the following disclaimer.
;    * Redistributions in binary form must reproduce the above copyright  notice,
;    this list of conditions and the following disclaimer in the documentation and/or
;    other materials provided with the distribution.
;    * Neither the name of the <organization> nor the names of its contributors may
;    be used to endorse or promote products derived from this software without
;    specific prior written permission.

; THE SOFTWARE IS PROVIDED УAS ISФ, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
; INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
; PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
; OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
; SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
; --------------------------------------------------------------------------------------
format binary as ""

use32
org 0

db 'MENUET01'
dd 1
dd START
dd I_END
dd MEM
dd STACKTOP
dd 0, 0

include 'lang.inc'
include '../../macros.inc'
include '../../proc32.inc'
include '../../dll.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac' ;компоненты checkBox и editBox
include '../../develop/libraries/box_lib/load_lib.mac'      ;макрос дл€ загрузки библиотек
@use_library

START:
   mcall 48,3,sc,sizeof.system_colors ;получить системные цвета

   stdcall dll.Load, @IMPORT
   or      eax, eax
   jnz     exit

   load_libraries l_libs_start,load_lib_end ;загрузка библиотек(и)
   stdcall [OpenDialog_Init],OpenDialog_data ;подготовка диалога

   mov  eax,40          ;установить маску дл€ ожидаемых событий
   mov  ebx,0x27        ;система будет реагировать только на сообщение о перерисовке,нажата кнопка, определЄнна€ ранее, событие от мыши (что-то случилось - нажатие на кнопку мыши или перемещение; сбрасываетс€ при прочтении)
   mcall

   mov ecx,[sc.work_text]
   and ecx, 0x9FFFFFFF
   or  ecx,0x90000000
   ;mov ecx, 0x90000000
   mov dword[editLU.text_color], ecx
   mov dword[editLD.text_color], ecx
   mov dword[editRU.text_color], ecx
   mov dword[editRD.text_color], ecx
   mov dword[editMBR.text_color], ecx

   invoke init_checkbox, ch1
   invoke init_checkbox, ch2

   call    draw_window             ; draw the window

event_wait:
        mov     eax, 10                 ; function 10 : wait until event
        mcall                           ; event type is returned in eax

        cmp     eax, 1                  ; Event redraw request ?
        je      red                     ; Expl.: there has been activity on screen and
                                        ; parts of the applications has to be redrawn.

        cmp     eax, 2                  ; Event key in buffer ?
        je      key                     ; Expl.: User has pressed a key while the
                                        ; app is at the top of the window stack.

        cmp     eax, 3                  ; Event button in buffer ?
        je      button                  ; Expl.: User has pressed one of the
                                        ; applications buttons.


        invoke check_box_mouse, ch1  ;проверка чек бокса
        invoke check_box_mouse, ch2

        invoke  edit_box_mouse, editLU   ;проверка событий мыши дл€ editBox'ов
    invoke  edit_box_mouse, editLD
    invoke  edit_box_mouse, editRU
    invoke  edit_box_mouse, editRD
    invoke  edit_box_mouse, editMBR

        jmp     event_wait


red:                                    ; Redraw event handler
        call    draw_window             ; We call the window_draw function and
        jmp     event_wait              ; jump back to event_wait

key:
        mcall 2
        invoke  edit_box_key, editLU
        invoke  edit_box_key, editLD
        invoke  edit_box_key, editRU
        invoke  edit_box_key, editRD
        invoke  edit_box_key, editMBR
        jmp     event_wait              ; Just read the key, ignore it and jump to event_wait.

button:
        mcall 17

        cmp     ah,1                    ; button id=1 ?
        jne     noclose
exit:
        mcall -1  ;close this program

noclose:

   cmp ah, 2
   jne no_format
   call format_action
   jmp     event_wait
no_format:
    cmp ah, 3
   jne no_browser
   call brouse_action
   jmp     event_wait
no_browser:
        jmp     event_wait              ; This is for ignored events, useful at development

brouse_action:
   call but_open_dlg
   ret
format_action:
   ret


delta = 50
dy = 15

draw_window:
   mcall  12, 1

   ;удал€ем кнопки, если есть
   mov edx, 0x80000002
   mcall 8
   mov edx, 0x80000003
   mcall 8

        mov     eax, 0                  ; function 0 : define and draw window
        mov     ebx, 100 * 65536 + (290+delta)  ; [x start] *65536 + [x size]
        mov     ecx, 100 * 65536 + (310+dy)  ; [y start] *65536 + [y size]
        mov     edx, 0x14ffffff         ; color of work area RRGGBB
                                        ; 0x02000000 = window type 4 (fixed size, skinned window)
        mov     esi, 0x808899ff         ; color of grab bar  RRGGBB
                                        ; 0x80000000 = color glide
        mov     edi, title
        mcall


        mov ebx, (290+delta-Otstup-130)*65536+130
        mov ecx, (270+dy)*65536+(20+3)
        mov edx, 0x00000002 ;2
        mov esi, 0xAABBCC ;4466AA
        mcall 8

        mov ebx, (290+delta-Otstup-50-2)*65536+(50+2)
        mov ecx, (210+dy)*65536+21 ;14
        mov edx, 0x00000003  ;3
        mov esi, 0xAABBCC ;D7D7D7 ;4466AA
        mcall 8

       invoke check_box_draw, ch1  ;рисование чекбоксов
       invoke check_box_draw, ch2

       invoke  edit_box_draw, editMBR     ;рисование edit box'ов
       invoke  edit_box_draw, editLU
       invoke  edit_box_draw, editLD
       invoke  edit_box_draw, editRU
       invoke  edit_box_draw, editRD

        call draw_super_text

        mcall 12, 2
        ret



Otstup = 30

;цвет 0x224466 заменЄн 0x90000000 и в финале на 0
ch1 check_box2 Otstup shl 16 + 12, (170+dy) shl 16 + 12, 6, 0xFFFFFFFF, 0xAABBCC, 0, ch_text.1, 100b ; 110b = ch_flag_en and ch_flag_middl
ch2 check_box2 Otstup shl 16 + 12, (190+dy) shl 16 + 12, 6, 0xFFFFFFFF, 0xAABBCC, 0, ch_text.2, 100b

if lang eq ru  ;если €зык сборки русский

title   db  "Formatting Disk Utility", 0

ch_text:        ;сопровождающий текст дл€ чек боксов
.1 db 'ПЃЂ≠Ѓ• ® §ЃЂ£Ѓ• дЃађ†в®аЃҐ†≠®•',0
.2 db 'СЃІ§†вм І†£агІЃз≠л© §®б™, І†ѓ®бм MBR:',0

text:
  .volume db 'М•в™† вЃђ†:', 0
  .fs     db 'Ф†©ЂЃҐ†п б®бв•ђ†:', 0
  .disk   db 'Еђ™Ѓбвм:', 0
  .unit   db 'Р†Іђ•а ™Ђ†бв•а†:', 0
  .option db 'П†а†ђ•вал:', 0
  .format db 'ФЃађ†в®аЃҐ†вм', 0
  .browse db 'О°ІЃа', 0

head_f_i:
        head_f_l  db 'С®бв•ђ≠†п Ѓи®°™†',0
        err_message_found_lib0 db 'Н• ≠†©§•≠† °®°Ђ®Ѓв•™† ',39,'proc_lib.obj',39,0
        err_message_import0 db 'Ои®°™† ѓа® ®ђѓЃав• °®°Ђ®Ѓв•™® ',39,'proc_lib.obj',39,0
        err_message_found_lib1 db 'Н• ≠†©§•≠† °®°Ђ®Ѓв•™† ',39,'kmenu.obj',39,0
        err_message_import1 db 'Ои®°™† ѓа® ®ђѓЃав• °®°Ђ®Ѓв•™® ',39,'kmenu',39,0

else  ;иначе английский текст

title   db  "Formatting Disk Utility", 0

ch_text:        ;сопровождающий текст дл€ чек боксов
.1 db 'Full and long disk formatting',0
.2 db 'Create startup disk, write MBR:',0

text:
  .volume db 'Volume Label:', 0
  .fs     db 'File System:', 0
  .disk   db 'Capacity:', 0
  .unit   db 'Allocation unit size:', 0
  .option db 'Options:', 0
  .format db 'Format', 0
  .browse db 'Browse', 0

head_f_i:
  head_f_l  db 'System error',0
  err_message_found_lib0 db 'Could not find library ',39,'proc_lib.obj',39,0
  err_message_import0 db 'Error importing library ',39,'proc_lib.obj',39,0
  err_message_found_lib1 db 'Could not find library ',39,'kmenu.obj',39,0
  err_message_import1 db 'Error importing library ',39,'kmenu',39,0

end if


draw_super_text:
   push eax ebx ecx edx edi esi
   mov     ebx, Otstup * 65536 + 49-6    ; draw info text with function 4 (x, y)
   mov     ecx, 0x90000000 ;0x90224466 ;0x224466
   mov eax, text.volume
   mov     edx, eax
   mov     esi, 13
   mcall 4

   mov     ebx, Otstup * 65536 + 99-6
   mov     edx, text.fs
   mov     esi, 12
   mcall 4

   mov     ebx, (Otstup+80+30+delta) * 65536 + 99-6
   mov     edx, text.unit
   mov     esi, 21
   mcall 4

   mov     ebx, (Otstup+80+30+delta) * 65536 + 49-6
   mov     edx, text.disk
   mov     esi, 9
   mcall 4

   mov     ebx, Otstup * 65536 + (151-6+dy)
   mov     edx, text.option
   mov     esi, 8
   mcall 4

   ;buttons text
   mov     ebx, (290+delta-Otstup-130+10+2) * 65536 + (277-3+dy)
   mov     ecx, 0x90FFFFFF;0xFFFFFF
   mov eax, text.format
   mov     edx, eax
   mov     esi, 6
   mcall 4

   mov     ebx, (290+delta-Otstup-52+6) * 65536 + (213+dy)  ;-3
   ;mov     ecx, 0xFFFFFF
   mov eax, text.browse
   mov     edx, eax
   mov     esi, 6
   mcall 4

   pop esi edi edx ecx ebx eax
   ret

Buf:
  .1 db 'NONAME18',0,0
  .2 db 'FAT',0,0 ;100 dup(0)
  .3 db 'hd0 [FAT32: 4Gb]',0,0 ;100 dup(0)
  .4 db '4096',0,0 ;100 dup(0)
  .5 db '/rd/1/format/fat32mbr.bin', 0, 0
     rb 256



align 16
@IMPORT:
 
library box_lib, 'box_lib.obj'
 
import  box_lib,\
        edit_box_draw,          'edit_box',\
        edit_box_key,           'edit_box_key',\
        edit_box_mouse,         'edit_box_mouse',\
        init_checkbox,          'init_checkbox2',\
        check_box_draw,         'check_box_draw2',\
        check_box_mouse,        'check_box_mouse2',\
        option_box_draw,        'option_box_draw',\
        option_box_mouse,       'option_box_mouse'

copyPath:
   push eax ebx ecx edx ;copy file name path
        mov eax, openfile_path ;dword[OpenDialog_data.openfile_path]
        mov ebx, Buf.5
        mov ecx, 0
      @@:
        mov dl, byte[eax]
        cmp dl, 0
        je @f
        mov byte[ebx], dl
        inc eax
        inc ebx
        inc ecx
        jmp @b
      @@:
        mov byte[ebx], 0
        ;mov dword[Buf.size5], ecx
        mov dword[editMBR.size], ecx
        mov dword[editMBR.pos], ecx
   pop edx ecx ebx eax
   ret

align 4
but_open_dlg:
        pushad
        copy_path open_dialog_name,communication_area_default_path,file_name,0
        mov [OpenDialog_data.type],0
        stdcall[OpenDialog_Start], OpenDialog_data
        cmp [OpenDialog_data.status],2
        je @f
        cmp [OpenDialog_data.status],0 ;пользователь нажал Cancel?
        je .end_open
                ;код при удачном открытии диалога
                ;...
                call copyPath
                jmp .end_open
        @@:
                ;код при не удачном открытии диалога
                ;...
        .end_open:
        popad
        ret

;данные дл€ диалога открыти€ файлов
align 4
OpenDialog_data:
.type                   dd 0 ;0 - открыть, 1 - сохранить, 2 - выбрать директорию
.procinfo               dd procinfo
.com_area_name          dd communication_area_name ;+8
.com_area               dd 0           ;+12
.opendir_path           dd plugin_path ;+16
.dir_default_path       dd default_dir ;+20
.start_path             dd file_name   ;+24 путь к диалогу открыти€ файлов
.draw_window            dd draw_window ;+28
.status                 dd 0           ;+32
.openfile_path          dd openfile_path ;+36 путь к открываемому файлу
.filename_area          dd filename_area ;+40
.filter_area            dd Filter
.x:
.x_size                 dw 420 ;+48 ; Window X size
.x_start                dw 10  ;+50 ; Window X position
.y:
.y_size                 dw 320 ;+52 ; Window y size
.y_start                dw 10  ;+54 ; Window Y position
 
default_dir db '/rd/1',0 ;директори€ по умолчанию
communication_area_name: db 'FFFFFFFF_open_dialog',0
open_dialog_name:  db 'opendial',0
communication_area_default_path: db '/rd/1/File managers/',0
 
Filter:
dd Filter.end - Filter.1
.1:
;db 'BIN',0
.end:
db 0


;описание экспортируемых функций
align 4
import_libkmenu:
        kmenu_init      dd akmenu_init
        kmainmenu_draw  dd akmainmenu_draw
        kmainmenu_dispatch_cursorevent dd akmainmenu_dispatch_cursorevent
        ksubmenu_new    dd aksubmenu_new
        ksubmenu_delete dd aksubmenu_delete
        ksubmenu_draw   dd aksubmenu_draw
        ksubmenu_add    dd aksubmenu_add
        kmenuitem_new   dd akmenuitem_new
        kmenuitem_delete dd akmenuitem_delete
        kmenuitem_draw  dd akmenuitem_draw
dd 0,0
        akmenu_init     db 'kmenu_init',0
        akmainmenu_draw db 'kmainmenu_draw',0
        akmainmenu_dispatch_cursorevent db 'kmainmenu_dispatch_cursorevent',0
        aksubmenu_new   db 'ksubmenu_new',0
        aksubmenu_delete db 'ksubmenu_delete',0
        aksubmenu_draw  db 'ksubmenu_draw',0
        aksubmenu_add   db 'ksubmenu_add',0
        akmenuitem_new  db 'kmenuitem_new',0
        akmenuitem_delete db 'kmenuitem_delete',0
        akmenuitem_draw  db 'kmenuitem_draw',0

align 4
proclib_import:
        OpenDialog_Init dd aOpenDialog_Init
        OpenDialog_Start dd aOpenDialog_Start
dd 0,0
        aOpenDialog_Init db 'OpenDialog_init',0
        aOpenDialog_Start db 'OpenDialog_start',0
 
system_dir0 db '/sys/lib/'
lib0_name db 'proc_lib.obj',0
lib1_name db 'kmenu.obj',0

;symbolDownArrow db 25,0

l_libs_start:
    lib0 l_libs lib0_name, sys_path, file_name, system_dir0, err_message_found_lib0, head_f_l, proclib_import,err_message_import0, head_f_i
    lib1 l_libs lib1_name, sys_path, file_name, system_dir0, err_message_found_lib1, head_f_l, import_libkmenu,err_message_import1,head_f_i
load_lib_end:

;размеры: 80 и 120
editLU edit_box 120,Otstup,60,0xffffff,0x6a9480,0,0xAABBCC,0,8,Buf.1, mouse_dd, 0,8,8
editLD edit_box 120,Otstup,110,0xffffff,0x6a9480,0,0xAABBCC,0,3,Buf.2, mouse_dd, 0,3,3
editRU edit_box 120,Otstup+80+30+delta,60,0xffffff,0x6a9480,0,0xAABBCC,0,16,Buf.3, mouse_dd, 0,16,16
editRD edit_box 120,Otstup+80+30+delta,110,0xffffff,0x6a9480,0,0xAABBCC,0,4,Buf.4, mouse_dd, 0,4,4
editMBR edit_box 290+delta-Otstup-52-Otstup-20,Otstup+20,210+dy,0xffffff,0x6a9480,0,0xAABBCC,0,255,Buf.5, mouse_dd, 0,25,25

data_of_code dd 0

sc  system_colors
mouse_dd  rd 1

sys_path:      rb 4096
file_name:     rb 4096
plugin_path:   rb 4096
openfile_path: rb 4096
filename_area: rb 256
rb 1024
procinfo process_information

I_END:
        rb 256 ;4096
STACKTOP:
MEM:
