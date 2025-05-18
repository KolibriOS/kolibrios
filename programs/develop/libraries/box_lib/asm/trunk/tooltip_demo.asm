; Простой пример программы для KolibriOS
; озвучивает код нажатой клавиши
; - переделан как пример использования tooltip

use32 ; включить 32-битный режим ассемблера
org 0 ; адресация с нуля

db 'MENUET01' ; 8-байтный идентификатор MenuetOS
dd 1 ; версия заголовка (всегда 1)
dd START ; адрес первой команды
dd CODE_END ; размер программы
dd DATA_END ; количество памяти
dd STACK_END ; адрес вершины стэка
dd 0 ; адрес буфера для параметров
dd cur_dir_path      ; указатель на адрес, куда помещается строка, содержащая путь до программы в момент запуска.

include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../../../../KOSfuncs.inc'
include '../../../../../dll.inc'	; malloc fn
include '../../trunk/box_lib.mac'
include '../../../../../load_lib.mac'


;---------------------------------------------------------------------
;--- НАЧАЛО ПРОГРАММЫ ----------------------------------------------
;---------------------------------------------------------------------
; этот макрос обязателен для всех компонетов, использующих heap
; кроме того, обязательно имортировать lib_init - при импорте определяются
; функции хипа для библиотеки
@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load

START:
;---------------------------------------------------------------------
;--- ИНИЦИАЛИЗАЦИЯ ----------------------------------------
;---------------------------------------------------------------------
mcall	SF_SYS_MISC, SSF_HEAP_INIT

mcall SF_SET_EVENTS_MASK, $C0000027 ; маска событий - мышь только в активном окне

sys_load_library  lib_name, lib_path, sys_path, import_box_lib
test eax,eax
jz	@f
	mcall SF_TERMINATE_PROCESS
@@:


invoke tooltip_init, redbox_tt 	; only begin of list

red: ; перерисовать окно

call draw_window ; вызываем процедуру отрисовки окна

;---------------------------------------------------------------------
;--- ЦИКЛ ОБРАБОТКИ СОБЫТИЙ ----------------------------------------
;---------------------------------------------------------------------

still:
mcall SF_WAIT_EVENT_TIMEOUT, 5 ; ждать события не более чем 0.05с
test eax, eax ; нет событий - проверить рисование тултипов по таймеру
je yield
cmp eax,EV_REDRAW
je red ; если да - на метку red
cmp eax,EV_KEY
je key ; если да - на key
cmp eax,EV_BUTTON
je button ; если да - на button
cmp eax,EV_MOUSE
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
mcall SF_GET_KEY ; считать код символа (в ah)

jmp still ; вернуться к началу цикла

;---------------------------------------------------------------------

button:
mcall SF_GET_BUTTON ; получить идентификатор нажатой кнопки

cmp ah, 1 ; если НЕ нажата кнопка с номером 1,
jne still ; вернуться

pexit:
invoke tooltip_delete, redbox_tt	; освобождаем память
mcall SF_TERMINATE_PROCESS


;---------------------------------------------------------------------
;--- ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА ----------------------------------
;---------------------------------------------------------------------

draw_window:

mcall SF_REDRAW, SSF_BEGIN_DRAW

mcall SF_STYLE_SETTINGS, SSF_GET_COLORS, sc, sizeof.system_colors

mov edx, [sc.work] ; цвет фона
or edx, 0x33000000 ; и тип окна 3
mcall SF_CREATE_WINDOW, <200,300>, <200,150>, , ,title

; вывод квадратиков
mcall SF_DRAW_RECT, <60,50>, <50,50>, $FF0000
mcall SF_DRAW_RECT, <140,50>, <50,50>, $FF


mcall SF_REDRAW, SSF_END_DRAW

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

include '../../import.inc' ;import_box_lib


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
