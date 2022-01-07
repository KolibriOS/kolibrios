;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Formatting Disk Utility ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compile with FASM
; FORMAT ver. Oct 19, 2018

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

KMENUITEM_NORMAL equ 0
KMENUITEM_SUBMENU equ 1
KMENUITEM_SEPARATOR equ 2

include 'lang.inc'
include '../../macros.inc'
include '../../proc32.inc'
include '../../KOSfuncs.inc'
include '../../dll.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac' ;for uses checkBox and editBox
include '../../load_lib.mac'
@use_library

START:
   mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors ;get system colors

   stdcall dll.Load, @IMPORT
   or      eax, eax
   jnz     exit

   load_libraries l_libs_start,load_lib_end
   stdcall [OpenDialog_Init],OpenDialog_data

   ;set mask for events:
   ;сообщение о перерисовке,нажата кнопка, определЄнна€ ранее, событие от мыши (что-то случилось - нажатие на кнопку мыши или перемещение; сбрасываетс€ при прочтении)
   mcall SF_SET_EVENTS_MASK, 0x27

   mov ecx,[sc.work_text]
   and ecx,0x9FFFFFFF
   or  ecx,0x90000000

   call initBuf

   mov dword[editLabel.text_color], ecx
   mov dword[editMBR.text_color], ecx

   invoke init_checkbox, ch1
   invoke init_checkbox, ch2

   stdcall [kmenu_init], sc
   stdcall [ksubmenu_new]
   mov [kmFS], eax

   stdcall [kmenuitem_new], KMENUITEM_NORMAL, kmNone, 110
   stdcall [ksubmenu_add], [kmFS], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, kmFat16, 111
   stdcall [ksubmenu_add], [kmFS], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, kmFat32, 112
   stdcall [ksubmenu_add], [kmFS], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, kmNTFS, 113
   stdcall [ksubmenu_add], [kmFS], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, kmExt2, 114
   stdcall [ksubmenu_add], [kmFS], eax
   mov byte[kmID], 0

   stdcall [kmenuitem_new], KMENUITEM_SUBMENU, kmFat16, [kmFS]


   stdcall [ksubmenu_new]
   mov [kmUnit], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, unittext.0, 120
   stdcall [ksubmenu_add], [kmUnit], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, unittext.1, 121
   stdcall [ksubmenu_add], [kmUnit], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, unittext.2, 122
   stdcall [ksubmenu_add], [kmUnit], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, unittext.3, 123
   stdcall [ksubmenu_add], [kmUnit], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, unittext.4, 124
   stdcall [ksubmenu_add], [kmUnit], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, unittext.5, 125
   stdcall [ksubmenu_add], [kmUnit], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, unittext.6, 126
   stdcall [ksubmenu_add], [kmUnit], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, unittext.7, 127
   stdcall [ksubmenu_add], [kmUnit], eax
   mov byte[kmUnitID], 0
   stdcall [kmenuitem_new], KMENUITEM_SUBMENU, unittext.0, [kmUnit]

   call getDeviceList

   call  draw_window

event_wait:
   mcall SF_WAIT_EVENT

   cmp     eax, 1       ; Event redraw
   je      redraw

   cmp     eax, 2       ; Event key in buffer ?
   je      key

   cmp     eax, 3       ; Event button in buffer ?
   je      button

   cmp     eax, 6       ; Event mouse in buffer ?
   je      mouse

   jmp  event_wait


redraw:
   call    draw_window
   jmp     event_wait

key:
   mcall SF_GET_KEY
   invoke  edit_box_key, editLabel
   invoke  edit_box_key, editMBR
   jmp     event_wait

button:
    mcall SF_GET_BUTTON

    cmp ah, 1         ;Close button
    jne @f
exit:
    mcall SF_TERMINATE_PROCESS
@@:
   cmp ah, 2          ;Format button
   jne @f
   call butFormat
   jmp event_wait
@@:
   cmp ah, 3          ;Brouse button
   jne @f
   call butBrouse
   jmp event_wait
@@:
   cmp ah, 4          ;Select FS button
   jne @f
   call butSelectFS
   jmp event_wait
@@:
   cmp ah, 5          ;Select unit size button
   jne @f
   call butUnit
   jmp event_wait
@@:
   cmp ah, 6          ;Select Device button
   jne @f
   call butDevice
   jmp event_wait
@@:
   cmp ah, 110        ;kmenu list FS
   jb @f
   cmp ah, 114
   ja @f
   sub ah, 110
   mov byte[kmID], ah
   jmp redraw
@@:
   cmp ah, 120        ;kmenu Unit Size
   jb @f
   cmp ah, 127
   ja @f
   sub ah, 120
   mov byte[kmUnitID], ah
   jmp redraw
@@:
   cmp ah, 130        ;kmenu Device
   jb @f
   cmp ah, 250
   ja @f
   sub ah, 130
   mov byte[kmDeviceID], ah
   jmp redraw
@@:
   jmp event_wait

mouse:
   invoke check_box_mouse, ch1  ;проверка чек бокса
   invoke check_box_mouse, ch2

   invoke  edit_box_mouse, editLabel   ;проверка событий мыши дл€ editBox'ов
   invoke  edit_box_mouse, editMBR

   stdcall [kmainmenu_dispatch_cursorevent], [kmFS]
   jmp event_wait

butBrouse:
   call but_open_dlg
   ret

butFormat:
   ret

butSelectFS:
   push eax ebx ecx
   mcall SF_THREAD_INFO, pi, -1 ;get window coord

   mov eax, dword[pi+34]
   add eax, Otstup
   mov word[coord.x], ax

   mov eax, dword[pi+38]
   add eax, 129
   mov word[coord.y], ax

   stdcall [ksubmenu_draw], [kmFS], coord
   pop ecx ebx eax
   ret

butUnit:
   push eax ebx ecx
   mcall SF_THREAD_INFO, pi, -1

   mov eax, dword[pi+34]
   add eax, Otstup+80+30+delta
   mov word[coordUnit.x], ax

   mov eax, dword[pi+38]
   add eax, 129
   mov word[coordUnit.y], ax

   stdcall [ksubmenu_draw], [kmUnit], coordUnit
   pop ecx ebx eax
   ret

butDevice:
   push eax ebx ecx
   mcall SF_THREAD_INFO, pi, -1

   mov eax, dword[pi+34]
   add eax, Otstup+80+30+delta
   mov word[coordDevice.x], ax

   mov eax, dword[pi+38]
   add eax, 79
   mov word[coordDevice.y], ax

   stdcall [ksubmenu_draw], [kmDevice], coordDevice
   pop ecx ebx eax
   ret


delta = 50
dy = 15 + 40
warning_title: db 'Warning!',0
draw_warningWindow:
   ret

draw_window:
   mcall  SF_REDRAW, SSF_BEGIN_DRAW

        mov     eax, SF_CREATE_WINDOW           ; define and draw window
        mov     ebx, 100 * 65536 + (290+delta)  ; [x start] *65536 + [x size]
        mov     ecx, 100 * 65536 + (310+dy)  ; [y start] *65536 + [y size]
        mov     edx,[sc.work]    ;0x14FFFFFF
        add     edx, 0x14000000         ; color of work area RRGGBB
                                        ; 0x02000000 = window type 4 (fixed size, skinned window)
        mov     esi, 0x808899ff         ; color of grab bar  RRGGBB
                                        ; 0x80000000 = color glide
        mov     edi, title
        mcall


        mov esi, 0xAABBCC
        mcall SF_DEFINE_BUTTON, (290+delta-Otstup-130+10)*65536+130, (270+dy)*65536+(20+3), 2

        inc edx
        mcall , (290+delta-Otstup-50-2)*65536+(50+2), (210+dy)*65536+21 ;14

        ;button select FS
        inc edx
        mov esi, 0xFFFFFF
        mcall , Otstup*65536+120, (110)*65536+(21)

        ;button select unit size
        inc edx
        mcall , (Otstup+80+30+delta)*65536+120, (110)*65536+(21)

        ;button select device
        inc edx
        mcall , (Otstup+80+30+delta)*65536+120, (60)*65536+(21)

       invoke check_box_draw, ch1  ;рисование чекбоксов
       invoke check_box_draw, ch2

       invoke  edit_box_draw, editMBR     ;рисование edit box'ов
       invoke  edit_box_draw, editLabel

       call draw_super_text


        mov     ecx,[sc.work]
        mov     dword [frame_data.font_backgr_color],ecx
        push    dword frame_data
        invoke  frame_draw

        mov     ecx,[sc.work]
        mov     dword [frame_data2.font_backgr_color],ecx
        push    dword frame_data2
        invoke  frame_draw

        mcall SF_REDRAW, SSF_END_DRAW
        ret



Otstup = 30

;цвет 0x224466 заменЄн 0x90000000 и в финале на 0
ch1 check_box2 Otstup shl 16 + 12, (170+dy) shl 16 + 12, 6, 0xFFFFFFFF, 0xAABBCC, 0, ch_text.1, 100b ; 110b = ch_flag_en and ch_flag_middl
ch2 check_box2 Otstup shl 16 + 12, (190+dy) shl 16 + 12, 6, 0xFFFFFFFF, 0xAABBCC, 0, ch_text.2, 100b


browse db '...', 0

if lang eq ru  ;RU language

title   db  "Formatting Disk Utility", 0

ch_text:       ;text for CheckBoxs
.1 db 'ПЃЂ≠Ѓ• ® §ЃЂ£Ѓ• дЃађ†в®аЃҐ†≠®•',0
.2 db 'СЃІ§†вм І†£агІЃз≠л© §®б™, І†ѓ®бм MBR:',0

text:
  .volume db 'М•в™† вЃђ†:', 0
  .fs     db 'Ф†©ЂЃҐ†п б®бв•ђ†:', 0
  .disk   db 'Н†™Ѓѓ®в•Ђм:', 0 ;'Еђ™Ѓбвм:', 0
  .unit   db 'Р†Іђ•а ™Ђ†бв•а†:', 0
  .option db 'П†а†ђ•вал:', 0
  .format db 'ФЃађ†в®аЃҐ†вм', 0

unittext:
  .0 db '512 °†©в', 0
  .1 db '1024 °†©в', 0
  .2 db '2048 °†©в', 0
  .3 db '4096 °†©в', 0
  .4 db '8192 °†©в', 0
  .5 db '16 К°', 0
  .6 db '32 К°', 0
  .7 db '64 К°', 0

else           ;EN language

title   db  "Formatting Disk Utility", 0

ch_text:
.1 db 'Full and long disk formatting',0
.2 db 'Create startup disk, write MBR:',0

text:
  .volume db 'Volume Label:', 0
  .fs     db 'File System:', 0
  .disk   db 'Storage device:', 0 ;'Capacity:', 0
  .unit   db 'Allocation unit size:', 0
  .option db 'Options:', 0
  .format db 'Format', 0

unittext:
  .0 db '512 bytes', 0
  .1 db '1024 bytes', 0
  .2 db '2048 bytes', 0
  .3 db '4096 bytes', 0
  .4 db '8192 bytes', 0
  .5 db '16 Kb', 0
  .6 db '32 Kb', 0
  .7 db '64 Kb', 0

end if



root_path: db "/", 0

align 4
maxDeviceCount = 250-130
read_folder_struct:
   .subfunction dd  1
   .start       dd  0 ;start block
   .encoding    dd  3 ;1-cp866, 2-UTF-16LE, 3-utf8
   .count       dd  maxDeviceCount ;count blocks
   .return      dd  0 ;адрес пам€ти дл€ получаемого блока Ѕƒ¬  с заголовком
   .name        db  0
   .path_adr    dd  root_path
noneDevice: db '-', 0
;adrDevice: dd 0

getDeviceList:
   push eax ebx ecx esi
   stdcall [ksubmenu_new]
   mov [kmDevice], eax
   sizeBDVK = 560   ;304 ;

   mcall SF_SYS_MISC, SSF_HEAP_INIT

   mcall SF_SYS_MISC, SSF_MEM_ALLOC, sizeBDVK*maxDeviceCount+32
   mov dword[read_folder_struct.return], eax
   mcall SF_FILE, read_folder_struct
   cmp eax, 0
   je .next
   cmp eax, 6
   je .next
   jmp .none
.next:
   mov eax, dword[read_folder_struct.return]
   mov esi, deviceAdrStr
   add eax, 32
   mov ecx, 130
@@:
   cmp ebx, 0
   je @f
   push eax ebx ecx
   add eax, 40
   ;mov [adrDevice], eax
   mov dword[esi], eax
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, [esi], ecx ; [adrDevice], ecx
   stdcall [ksubmenu_add], [kmDevice], eax
   pop ecx ebx eax
   add esi, 4
   inc ecx
   dec ebx
   add eax, sizeBDVK
   cmp ecx, 250
   ja @f
   jmp @b
.none:
   stdcall [kmenuitem_new], KMENUITEM_NORMAL, noneDevice, 130
   mov dword[esi], noneDevice
   stdcall [ksubmenu_add], [kmDevice], eax
@@:
   mov byte[kmDeviceID], 0
   stdcall [kmenuitem_new], KMENUITEM_SUBMENU, unittext.0, [kmDevice]
   pop esi ecx ebx eax
   ret


draw_super_text:
   push eax ebx ecx edx edi esi
   mov     ebx, Otstup * 65536 + 49-6    ; draw info text with function 4 (x, y)
   mov     ecx, 0x90000000 ;0x90224466 ;0x224466
   mov eax, text.volume
   mov     edx, eax
   mov     esi, 13
   mcall SF_DRAW_TEXT

   mov     ebx, Otstup * 65536 + 99-6
   mov     edx, text.fs
   mov     esi, 12
   mcall SF_DRAW_TEXT

   mov     ebx, (Otstup+80+30+delta) * 65536 + 99-6
   mov     edx, text.unit
   mov     esi, 21
   mcall SF_DRAW_TEXT

   mov     ebx, (Otstup+80+30+delta) * 65536 + 49-6
   mov     edx, text.disk
   mov     esi, 9
   mcall SF_DRAW_TEXT

;   mov     ebx, Otstup * 65536 + (151-6+dy)
;   mov     edx, text.option
;   mov     esi, 8
;   mcall SF_DRAW_TEXT

   mov     ebx, (Otstup+80+30+delta +5) * 65536 + (110+3)
   mov dl, byte[kmUnitID]
   cmp dl, 0
   jne @f
   mov     edx, unittext.0
   jmp .printUnit
@@:
   cmp dl, 1
   jne @f
   mov     edx, unittext.1
   jmp .printUnit
@@:
   cmp dl, 2
   jne @f
   mov     edx, unittext.2
   jmp .printUnit
@@:
   cmp dl, 3
   jne @f
   mov     edx, unittext.3
   jmp .printUnit
@@:
   cmp dl, 4
   jne @f
   mov     edx, unittext.4
   jmp .printUnit
@@:
   cmp dl, 5
   jne @f
   mov     edx, unittext.5
   jmp .printUnit
@@:
   cmp dl, 6
   jne @f
   mov     edx, unittext.6
   jmp .printUnit
@@:
   cmp dl, 7
   jne @f
   mov     edx, unittext.7
   jmp .printUnit
@@:
   mov byte[kmUnitID], 0
   mov     edx, unittext.0
.printUnit:
   mcall SF_DRAW_TEXT

   mov     ebx, (Otstup+5) * 65536 + (110+3)
   mov dl, byte[kmID]
   cmp dl, 0
   jne @f
   mov     edx, kmNone
   jmp .printFS
@@:
   cmp dl, 1
   jne @f
   mov     edx, kmFat16
   jmp .printFS
@@:
   cmp dl, 2
   jne @f
   mov     edx, kmFat32
   jmp .printFS
@@:
   cmp dl, 3
   jne @f
   mov     edx, kmNTFS
   jmp .printFS
@@:
   cmp dl, 4
   jne @f
   mov     edx, kmExt2
   jmp .printFS
@@:
   mov byte[kmID], 0
   mov     edx, kmNone
.printFS:
   ;mov     esi, 8
   mcall SF_DRAW_TEXT

   ;button device
   mov     ebx, (Otstup+80+30+delta +5) * 65536 + (60+3)
   mov edx, dword[kmDeviceID]
   shl edx, 2
   add edx, deviceAdrStr
   mov edx, dword[edx]
   ;call setCurrentDeviceInEDX
   mcall SF_DRAW_TEXT

   ;buttons text
   mov     ebx, (290+delta-Otstup-130+10+2+10) * 65536 + (277-3+dy)
   mov     ecx, 0x90FFFFFF
   mov eax, text.format
   mov     edx, eax
   mov     esi, 6
   mcall SF_DRAW_TEXT

   mov     ebx, (290+delta-Otstup-52+6+8) * 65536 + (213+dy)
   ;mov     ecx, 0xFFFFFF
   mov eax, browse ;text.browse
   mov     edx, eax
   mov     esi, 6
   mcall SF_DRAW_TEXT

   pop esi edi edx ecx ebx eax
   ret



Buf:
  .1 db 'NONAME18',0,0
  ;.3 db 'hd0 [4Gb]',0,0 ;100 dup(0)
  .5 rb 512 ;db '/sys/format/fat32mbr.bin', 0, 0

initBuf:
   push eax ecx
   ;buf.1 - label
   mov dword[Buf.1], 'NONA'
   mov dword[Buf.1+4], 'ME18'
   mov word[Buf.1+8], 0

   ;buf.5 - full name for file mbr
   mov eax, Buf.5
   mov ecx, 512/4
@@:
   mov dword[eax], 4
   add eax, 4
   dec ecx
   cmp ecx, 0
   je @f
   jmp @b
@@:

   pop ecx eax
   ret



copyPath:
   push eax ebx ecx edx ;copy file name path
        mov eax, openfile_path
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
 
default_dir db '/sys',0 ;директори€ по умолчанию
communication_area_name: db 'FFFFFFFF_open_dialog',0
open_dialog_name:  db 'opendial',0
communication_area_default_path: db '/sys/File managers/',0
 
Filter:
dd Filter.end - Filter.1
.1:
;db 'BIN',0
.end:
db 0


align 16
@IMPORT:
library box_lib, 'box_lib.obj'
 
import  box_lib,\
        edit_box_draw,          'edit_box_draw',\
        edit_box_key,           'edit_box_key',\
        edit_box_mouse,         'edit_box_mouse',\
        init_checkbox,          'init_checkbox2',\
        check_box_draw,         'check_box_draw2',\
        check_box_mouse,        'check_box_mouse2',\
        option_box_draw,        'option_box_draw',\
        option_box_mouse,       'option_box_mouse',\
        frame_draw,             'frame_draw'

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
import_proclib:
        OpenDialog_Init dd aOpenDialog_Init
        OpenDialog_Start dd aOpenDialog_Start
dd 0,0
        aOpenDialog_Init db 'OpenDialog_init',0
        aOpenDialog_Start db 'OpenDialog_start',0
 
system_dir0 db '/sys/lib/'
lib0_name db 'proc_lib.obj',0
lib1_name db 'kmenu.obj',0


frame_data:
.type                   dd 0 ;+0
.x:
.x_size                 dw 290+delta-2*(Otstup-10) ;+4
.x_start                dw Otstup-10 ;+6
.y:
.y_size                 dw 80+20 ;+8
.y_start                dw 151-6+dy ;+10
.ext_fr_col             dd 0x888888 ;+12
.int_fr_col             dd 0xffffff ;+16
.draw_text_flag         dd 1 ;+20
.text_pointer           dd text.option ;+24
.text_position          dd 0 ;+28
.font_number            dd 1;0 ;+32
.font_size_y            dd 9 ;+36
.font_color             dd 0x000000 ;+40
.font_backgr_color      dd 0xFFFFFF ;dddddd ;+44

frame_data2:
.type                   dd 0
.x:
.x_size                 dw 290+delta-2*(Otstup-10)
.x_start                dw Otstup-10
.y:
.y_size                 dw 110
.y_start                dw Otstup+5
.ext_fr_col             dd 0x888888
.int_fr_col             dd 0xffffff
.draw_text_flag         dd 0;1
.text_pointer           dd 0 ;text.option
.text_position          dd 0
.font_number            dd 0
.font_size_y            dd 9
.font_color             dd 0x0
.font_backgr_color      dd 0xdddddd

;symbolDownArrow db 25,0

kmNone: db 'None', 0  ;only MBR or ZeroDestroy
;kmFat12: db 'FAT12', 0
kmFat16: db 'FAT16', 0
kmFat32: db 'FAT32', 0
;kmExtFat: db 'EXTFAT', 0
kmNTFS: db 'NTFS', 0
kmExt2: db 'EXT2', 0
;kmExt3: db 'EXT3', 0
;kmExt4: db 'EXT4', 0
;kmXFS: db 'XFS', 0

l_libs_start:
    lib0 l_libs lib0_name, file_name, system_dir0, import_proclib
    lib1 l_libs lib1_name, file_name, system_dir0, import_libkmenu
load_lib_end:

;размеры: 80 и 120
editLabel edit_box 120,Otstup,60,0xffffff,0x6a9480,0,0xAABBCC,0,8,Buf.1, mouse_dd, 0,8,8
;editRU edit_box 120,Otstup+80+30+delta,60,0xffffff,0x6a9480,0,0xAABBCC,0,16,Buf.3, mouse_dd, 0,16,16
editMBR edit_box 290+delta-Otstup-52-Otstup-20,Otstup+20,210+dy,0xffffff,0x6a9480,0,0xAABBCC,0,255,Buf.5, mouse_dd, 0,0,0 ;25,25

data_of_code dd 0

sc  system_colors
mouse_dd  rd 1

coord:
  .x: rw 1
  .y: rw 1

coordUnit:
  .x: rw 1
  .y: rw 1

coordDevice:
  .x: rw 1
  .y: rw 1

kmDeviceID: rd 1
kmDevice: rd 1

kmUnitID: rd 1
kmUnit: rd 1

kmID: rb 1 ;номер выбранного пункта
kmFS: rd 1
mbr: rb 512

sys_path:      rb 4096
file_name:     rb 4096
plugin_path:   rb 4096
openfile_path: rb 4096
filename_area: rb 256
rb 1024
procinfo process_information
pi rb 1024

deviceAdrStr: rd maxDeviceCount

I_END:
        rb 256
align 4
STACKTOP:
MEM:
