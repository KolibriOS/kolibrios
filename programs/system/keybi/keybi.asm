; Keyboard indicators v0.2
; by Albom and IgorA

use32
 org 0
 db 'MENUET01'
 dd 1
 dd _start
 dd _end
 dd _memory
 dd stacktop
 dd 0
 dd sys_path

include '../../macros.inc'
include '../../proc32.inc'
include '../../develop/libraries/box_lib/load_lib.mac'
include '../../develop/libraries/box_lib/trunk/box_lib.mac'
;include 'mem.inc'
;include 'dll.inc'
include 'lang.inc'

@use_library ;_mem mem.Alloc,mem.Free,mem.ReAlloc, dll.Load

align 4
_start:
 load_libraries l_libs_start,l_libs_end
 mcall 48,3,sc,sizeof.system_colors
 mcall 40,0x27

 init_checkboxes2 check_boxes,check_boxes_end
 check_boxes_set_sys_color2 check_boxes,check_boxes_end,sc
 call _key_set

align 4
red_win:
 call draw_window


align 4
still:
 mcall 10

 cmp al,1 ;изм. положение окна
 jz red_win
 cmp al,2
 jz key
 cmp al,3
 jz button

 ;stdcall [check_box_mouse], ch1
 ;stdcall [check_box_mouse], ch2
 ;stdcall [check_box_mouse], ch3

 jmp still

;установить общесистемные "горячие клавиши"
align 4
_key_set:
mov eax, 66
mov ebx, 4
mov edx, 0
mov cl, 69
int 0x40

mov eax, 66
mov ebx, 4
mov edx, 0
mov cl, 58
int 0x40

mov eax, 66
mov ebx, 4
mov edx, 0
mov cl, 70
int 0x40

ret

align 4
draw_window:
pushad
 mcall 12,1
 xor eax,eax
 mov ebx,(10 shl 16)+100
 mov ecx,(10 shl 16)+75
 mov edx,[sc.work]
 or edx,(2 shl 24)+0x10000000+0x20000000
 mov edi,[sc.grab] ;[sc.frame]
 mov esi,[sc.grab]
 int 0x40

 mov eax,8
 mov ebx,(80 shl 16)+10
 mov cx,-15
 shl ecx,16
 mov cx,10
 mov edx,1
 mov esi,[sc.grab_button]
 int 0x40

 stdcall [check_box_draw], ch1
 stdcall [check_box_draw], ch2
 stdcall [check_box_draw], ch3
 mcall 12,2
popad
 ret

align 4
key:
 mcall 2
 call _indicators_check
 jmp still

align 4
_indicators_check:
pusha
 mov eax, 66
 mov ebx, 3
 int 40h

test_ins:
 test eax, 0x80
 jz @f
 bts dword[ch1.flags],1
 jmp test_caps
@@:
 btr dword[ch1.flags],1

test_caps:
 test eax, 0x40
 jz @f
 bts dword[ch2.flags],1
 jmp test_scroll
@@:
 btr dword[ch2.flags],1

test_scroll:
 test eax, 0x100
 jz @f
 bts dword[ch3.flags],1
 jmp test_ok
@@:
 btr dword[ch3.flags],1

test_ok:
 call draw_window
popa
ret

align 4
button:
 mcall 17
 cmp ah,1
 jne still
.exit:
 mcall -1


check_boxes:
ch1 check_box2 (5 shl 16)+15,(5 shl 16)+10,5, 0xffffff,0x8000,0xff,\
 txt_160,0+ch_flag_middle
ch2 check_box2 (5 shl 16)+15,(17 shl 16)+10,5, 0xffffff,0x8000,0xff,\
 txt_159,0+ch_flag_middle
ch3 check_box2 (5 shl 16)+15,(29 shl 16)+10,5, 0xffffff,0x8000,0xff,\
 txt_158,0+ch_flag_middle
check_boxes_end:

if lang eq it
	txt_160 db 'Bloc Num',0
	txt_159 db 'Bloc Maiusc',0
	txt_158 db 'Bloc Scorr',0
else
	txt_160 db 'Num',0
	txt_159 db 'Caps',0
	txt_158 db 'Scroll',0
end if
head_f_i:
head_f_l db 'Системная ошибка',0

system_dir_0 db '/sys/lib/'
lib_name_0 db 'box_lib.obj',0
err_msg_found_lib_0 db 'Не найдена библиотека box_lib.obj',0
err_msg_import_0 db 'Ошибка при импорте библиотеки box_lib',0

l_libs_start:
 lib_0 l_libs lib_name_0, sys_path, library_path, system_dir_0,\
 err_msg_found_lib_0,head_f_l,import_box_lib,err_msg_import_0,head_f_i
l_libs_end:

align 4
import_box_lib:
 ;init dd sz_init
 init_checkbox dd sz_init_checkbox
 check_box_draw dd sz_check_box_draw
 check_box_mouse dd sz_check_box_mouse
 dd 0,0
 ;sz_init db 'lib_init',0
 sz_init_checkbox db 'init_checkbox2',0
 sz_check_box_draw db 'check_box_draw2',0
 sz_check_box_mouse db 'check_box_mouse2',0

;mouse_dd dd 0x0
sc system_colors

_end:
align 32
 rb 2048
stacktop:
 sys_path rb 1024
 library_path rb 1024
_memory:
