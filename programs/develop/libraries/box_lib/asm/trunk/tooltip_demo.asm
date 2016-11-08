; Простой пример программы для KolibriOS
; озвучивает код нажатой клавиши
; - переделан как пример использования tooltip

use32 ; включить 32-битный режим ассемблера
org 0x0 ; адресация с нуля

db 'MENUET01' ; 8-байтный идентификатор MenuetOS
dd 0x01 ; версия заголовка (всегда 1)
dd START ; адрес первой команды
dd CODE_END ; размер программы
dd DATA_END ; количество памяти
dd STACK_END ; адрес вершины стэка
dd 0x0 ; адрес буфера для параметров
dd cur_dir_path      ; указатель на адрес, куда помещается строка, содержащая путь до программы в момент запуска.

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../../../../KOSfuncs.inc'
include '../../../../../dll.inc'	; malloc fn
include '../../trunk/box_lib.mac'
include '../../load_lib.mac'
;include 'proc32.inc'
;include 'macros.inc'
;include 'KOSfuncs.inc'

;---------------------------------------------------------------------
;--- НАЧАЛО ПРОГРАММЫ ----------------------------------------------
;---------------------------------------------------------------------
; этот макрос обязателен для всех компонетов, использующих heap
; кроме того, обязательно имортировать lib_init - при импорте определяются
; функции хипа для библиотеки
@use_library_mem mem.Alloc,mem.Free,mem.ReAlloc,dll.Load

START:
;---------------------------------------------------------------------
;--- ИНИЦИАЛИЗАЦИЯ ----------------------------------------
;---------------------------------------------------------------------
;mov eax, mem_alloc
;mov [mem.alloc], eax
;mov eax, mem_realloc
;mov [mem.realloc], eax
;mov eax, mem_free
;mov [mem.free], eax
mcall	68, 11

mcall 40, $C0000027 ; маска событий - мышь только в активном окне

sys_load_library  lib_name, cur_dir_path, lib_path, sys_path, \
	e_notfound_lib, head_f_l, myimport, e_import, head_f_i
test eax,eax
jz	@f
	mcall -1 ; alarm exit
@@:


invoke tooltip_init, redbox_tt 	; only begin of list

red: ; перерисовать окно

call draw_window ; вызываем процедуру отрисовки окна

;---------------------------------------------------------------------
;--- ЦИКЛ ОБРАБОТКИ СОБЫТИЙ ----------------------------------------
;---------------------------------------------------------------------

still:
mcall 23, 5 ; функция 23 - ждать события Не более чем 0.05с
test eax, eax ; нет событий - проверить рисование тултипов по таймеру
je yield
cmp eax,1 ; перерисовать окно ?
je red ; если да - на метку red
cmp eax,2 ; нажата клавиша ?
je key ; если да - на key
cmp eax,3 ; нажата кнопка ?
je button ; если да - на button
cmp eax,6 ; событие мыши
je mouse ; если да - на mouse

jmp still ; если другое событие - в начало цикла


;---------------------------------------------------------------------
yield:
invoke tooltip_test_show, redbox_tt
jmp still ; вернуться к началу цикла

mouse:
invoke tooltip_mouse, redbox_tt
jmp still ; вернуться к началу цикла

key: ; нажата клавиша на клавиатуре
mcall 2 ; функция 2 - считать код символа (в ah)

jmp still ; вернуться к началу цикла

;---------------------------------------------------------------------

button:
mcall 17 ; 17 - получить идентификатор нажатой кнопки

cmp ah, 1 ; если НЕ нажата кнопка с номером 1,
jne still ; вернуться

pexit:
invoke tooltip_delete, redbox_tt	; освобождаем память
mcall -1 ; иначе конец программы


;---------------------------------------------------------------------
;--- ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА ----------------------------------
;---------------------------------------------------------------------

draw_window:

mcall 12, 1 ; функция 12: сообщить ОС о начале отрисовки

mcall 48, 3, sc,sizeof.system_colors

mov edx, [sc.work] ; цвет фона
or edx, 0x33000000 ; и тип окна 3
mcall 0, <200,300>, <200,150>, , ,title

; вывод квадратиков
mcall 13, <60,50>, <50,50>, $FF0000
mcall 13, <140,50>, <50,50>, $FF


mcall 12, 2 ; функция 12.2, закончили рисовать

ret ; выходим из процедуры


CODE_END: ; метка конца программы; --------------------------------------------;

; ---------------------------------------------------------------------------- ;
;---------------------------------------------------------------------
;--- ДАННЫЕ ПРОГРАММЫ ----------------------------------------------
;---------------------------------------------------------------------

sys_path	db '/sys/lib/'
;sys_path	db '/tmp0/1/'
lib_name    db 'box_lib.obj',0
cur_dir_path    rb 4096
lib_path    rb 4096

e_notfound_lib    db 'Sorry I cannot load library box_lib.obj',0

head_f_i:
head_f_l    db 'System error',0
e_import    db 'Error on load import library box_lib.obj',0

myimport:
				dd sz_lib_init ;функция запускается макросом 1 раз при подключении 
;библиотеки, потому в программе метка на нее не нужна
tooltip_init  	dd sz_tooltip_init
tooltip_delete	dd sz_tooltip_delete
tooltip_test_show	dd sz_tooltip_test_show
tooltip_mouse	dd sz_tooltip_mouse
get_font_size	dd sz_get_font_size
    dd    0
    dd    0

sz_lib_init 			db 'lib_init',0
sz_tooltip_init			db 'tooltip_init', 0
sz_tooltip_delete		db 'tooltip_delete', 0
sz_tooltip_test_show	db 'tooltip_test_show', 0
sz_tooltip_mouse		db 'tooltip_mouse', 0
sz_get_font_size		db 'get_font_size', 0


;tooltip txt, next, zone_x, zone_w, zone_y, zone_h, col_txt, col_bkg, tm_wait
redbox_tt    tooltip redboxtxt, blubox_tt, 60, 50, 50, 50, 0, $FFF473, 100
blubox_tt    tooltip bluboxtxt, 0, 140, 50, 50, 50, $110000FF, $FFF473, 100

redboxtxt	db 'Red Box Tooltip', 13, 'May be multilined', 13, 13, 'Even with empty lines', 0
bluboxtxt	db 'Blue Box Tooltip', 0

sc system_colors

title db 'Toooltip demo',0

; stack----------------------------------------------------------------------- ;
	   rb 4096
STACK_END  dd ?

DATA_END: ; метка конца данных программы; ------------------------------------ ;
