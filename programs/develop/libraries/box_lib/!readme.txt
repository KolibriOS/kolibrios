Руководство программиста по использованию макросов для универсальной загрузки библиотеки/библиотек
от 6 июля 2009г.

Copyright (c) 2009, <Lrz>
All rights reserved.

        Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
Neither the name of the <organization> nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY Alexey Teplov aka <Lrz> ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE MPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE  DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
        LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************Изменения:

Доработан макрос, убраны ограничения при его использовании. Вывод сообщений об ошибках производиться в отдельном потоке. При пакетной обработке загрузки библиотек, в начале формируется все сообщения от библиотек, которые загружены с ошибками, и затем произведен вывод на экран отдельным потоком со всем списком ошибок.Введение:

        В последнее время наметилась тенденция в переносе основных, базовых блоков (компонентов) в библиотеки. Для разработчика это является очень удобно, т.к. сроки разработки программы значительно сокращаются. Макрос load_lib.mac разрабатывался как универсальный для загрузки любого количества библиотек. Особенностью его является то, что если библиотека не может быть найдена по указанному пути, или существует ошибка при импорте, то тогда, формируется сообщение и выводиться в окне информация об ошибке. Поиск библиотеки происходит по 2-м направлениям. В текущей папке, откуда стартовала программа и в системной папке (по указанному пути + название библиотеки). 
Структура макросов:

        Файл load_lib.mac состоит из 5 основных макросов.
Для загрузки одной библиотеки существуют макросы первой группы, назовем, ее группа А.  Для загрузки от 2-х и более библиотек созданы макросы группы B. Название этих макросов:

A:
sys_load_library
load_library
B:
sys_load_libraries
load_libraries

        Для макросов группы А необходимо в качестве параметров указать следующие опции:
library_name, cur_dir_path, library_path, system_path, err_message_found_lib, head_f_l, myimport, err_message_import, head_f_i . Синтаксически правильно использовать следующию запись для правильного разворачивания макроса:

sys_load_library library_name, cur_dir_path, library_path, system_path, err_message_found_lib, head_f_l, myimport, err_message_import, head_f_i

или 

load_library library_name, cur_dir_path, library_path, system_path, err_message_found_lib, head_f_l, myimport, err_message_import, head_f_i

        Разница между этими двумя макросами только в порядке проверки пути к библиотеке.
sys_load_library - в первую очередь проверяется значение указанное в system_path, т.е. на этом месте где находится system_path должен быть помещен адрес записи. 

Всегда соблюдать последовательность в имени.
system_path      db '/sys/lib/'
library_name     db 'box_lib.obj',0     ; такая запись сделана из экономии места

Если есть желание разъединить, то нужно использовать следующую конструкцию
system_path      db '/sys/lib/box_lib.obj',0
... любая последовательность других команд и определений.
library_name     db 'box_lib.obj',0 


А load_library - в первую очередь проверяет текущую папку, т.е. использует этот путь для поиска библиотеки.

library_name - имя библиотеки обычно в данных определяется как 
library_name     db 'box_lib.obj',0

Иногда, возникает необходимость загрузки библиотеки с папки, которая находиться ниже уровнем директории, с которой была запущена программа. Допустим, следующее:


Необходимая нам библиотека расположена в папке ff2, для того, что бы макрос загрузил библиотеку из этой папки, нам нужно сделать следующее:

Обращаю внимание, что короткая запись в этом случае невозможна, и нужно определить следующие пути полностью.
system_path      db '/sys/lib/tread_lib.obj',0
;... любая последовательность других команд и определений.
library_name     db 'ff2/tread_lib.obj',0
- именно такое определение имени, позволит динамически сформировать путь до нашей бибилотеки.

cur_dir_path - первоначально должен быть определен в заголовке программы, как:

use32                   ; транслятор, использующий 32 разрядных команды
    org 0x0             ; базовый адрес кода, всегда 0x0
    db 'MENUET01'       ; идентификатор исполняемого файла (8 байт)
    dd 0x1              ; версия формата заголовка исполняемого файла
    dd start            ; адрес, на который система передаёт управление
                        ; после загрузки приложения в память
    dd i_end            ; размер приложения
    dd mem              ; Объем используемой памяти, для стека отведем 0х100 байт и выровним на грницу 4 байта
    dd mem              ; расположим позицию стека в области памяти, сразу за телом программы. Вершина стека в диапазоне памяти, указанном выше
    dd 0x0              ; указатель на строку с параметрами.
    dd cur_dir_path     ; указатель на адрес, куда помещается строка, содержащая путь до программы в момент запуска.
а затем в секции DATA программы 

cur_dir_path    rb 4096 ; значение 4096 может быть и меньше, но должно вмещать полностью путь до библиотеки. Т.е. это буфер, в котором формируется путь при запуске программы.

library_path - начало буфера, в котором будет сформирован путь полученный при запуске программы с именем библиотеки. 
library_path    rb 4096

system_path - путь до библиотеки с именем библиотеки. Предполагаем, что, тут указан полный путь до библиотеки.
Всегда соблюдать последовательность в имени.
system_path      db '/sys/lib/'
library_name     db 'box_lib.obj',0     ; такая запись сделана из экономии места

Если есть желание разъединить, то нужно использовать следующую конструкцию
system_path      db '/sys/lib/box_lib.obj',0
... любая последовательность других команд и определений.
library_name     db 'box_lib.obj',0 

err_message_found_lib - строка, которая будет в сформированном окне, если библиотека не будет найдена.

err_message_found_lib   db 'Sorry I cannot load library box_lib.obj',0

head_f_l -  заголовок окна, при возникновении ошибки - библиотека не найдена.
head_f_l        db 'System error',0

myimport -  указатель на импорт функций из библиотеки.
myimport:   

edit_box_draw   dd      aEdit_box_draw
edit_box_key    dd      aEdit_box_key
edit_box_mouse  dd      aEdit_box_mouse
version_ed      dd      aVersion_ed

check_box_draw  dd      aCheck_box_draw
check_box_mouse dd      aCheck_box_mouse
version_ch      dd      aVersion_ch

option_box_draw  dd      aOption_box_draw
option_box_mouse dd      aOption_box_mouse
version_op       dd      aVersion_op

                dd      0
                dd      0

aEdit_box_draw  db 'edit_box',0
aEdit_box_key   db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
aVersion_ed     db 'version_ed',0

aCheck_box_draw  db 'check_box_draw',0
aCheck_box_mouse db 'check_box_mouse',0
aVersion_ch      db 'version_ch',0

aOption_box_draw  db 'option_box_draw',0
aOption_box_mouse db 'option_box_mouse',0
aVersion_op       db 'version_op',0

err_message_import - строка, которая будет в сформированном окне, если при импорте функций произошла ошибка.

err_message_import      db 'Error on load import library box_lib.obj',0

head_f_i - заголовок окна, при возникновении ошибки - ошибка импорта функций.
head_f_i        db 'System error',0

        После того, как макрос будет раскрыт и отработает свою часть кода, можно узнать об успешности или не успешности загрузки, импорте библиотеки. В регистре еax формируется 0 при успешной загрузке и импорте, или -1, если на любом из этих этапов возникла ошибка. При возникновении ошибки рекомендуется завершить выполнение программы.
        cmp     eax,-1
        jz      exit

Группа макросов B

        Основным отличием макросов группы А, от группы B является блочная (пакетная) обработка загрузки большого количестве библиотек. Так же, больше информации можно получить после обработки пакета. Так, при пакетной обработке получаем код возврата, который содержит 2 типа кодов ошибок:
Не нашли либу
Не смогли импортировать функции.

B:
sys_load_libraries
load_libraries

        Для макросов группы B необходимо в качестве параметров указать следующие опции:
начало блока данных структур и конец load_libraries l_libs_start,end_l_libs, где 

l_libs_start:
library01  l_libs boxlib_name, path, file_name, system_dir, \
er_message_found_lib, ihead_f_l, myimport, er_message_import, ihead_f_i

library02  l_libs plugin_BMP_name, path, file_name, system_dir1,\
er_message_found_lib2, ihead_f_l, myimport, er_message_import2, ihead_f_i
end_l_libs:

Вот такая запись
library01  l_libs boxlib_name, path, file_name, system_dir, \
er_message_found_lib, ihead_f_l, myimport, er_message_import, ihead_f_i
раскрывается в следующее:

.library_name   dd library_name
.cur_dir_path   dd cur_dir_path
.library_path   dd library_path
.system_path    dd system_path
.err_message_found_lib   dd err_message_found_lib
.head_f_l       dd head_f_l
.my_import      dd my_import
.err_message_import      dd err_message_import
.head_f_i       dd head_f_i
;выше полностью соответствует значениям для параметров группы макросов А.
.adr_load_lib   dd 0x0          ; адрес загруженной библиотеки 
.status_lib     dd 0x0          ;status of load library - статус коды могут принимать значение 0 - успешно, 0х1 - ошибка поиска библиотеки, 0х2 - ошибка импорта функций.

Если нужно узнать программе, как загрузилась библиотека, используем следующую проверку:

;проверка на сколько удачно загрузилась наша библиотека
        mov     ebp,library01 - метка структуры
        cmp     dword [ebp+ll_struc_size-4],0 ; тут проверяем код статуса возврата
        jnz     exit ;если не 0, то уходим.


;получение адреса загруженной библиотеки
        mov     ebp,library01 - метка структуры
        cmp     dword [ebp+ll_struc_size-4],0 ; тут проверяем код статуса возврата
        jnz     exit ;если не 0, то уходим.
        mov     ebp, dword [ebp+ll_struc_size-8] - в ebp адрес начала.

Макорос @use_library

Этот макрос представляет вызываемые процедуры, которые используются для работы групп макросов А и B. Данный макрос располагается в секции дата. Использование данного макроса нужно для загрузки библиотек но его можно заменять на @use_library_mem.


Макорос @use_library_mem mem_alloc,mem_free,mem_realloc,dll_load

Этот макрос использует макрос @use_library, но в отличие от него он позволяет для функций с именами 'lib_init' задавать 4 параметра. В даных параметрах могут быть указатели на функции для работы с памятью, которые могут быть нужны для использования внутри библиотеки.


Как я могу использовать макрос загрузки библиотеки/библиотек в своей программе? 

Общий шаблон для использования библиотеки такой:

use32                ; транслятор, использующий 32 разрядных команды
    org 0x0                ; базовый адрес кода, всегда 0x0
    db 'MENUET01'        ; идентификатор исполняемого файла (8 байт)
    dd 0x1                ; версия формата заголовка исполняемого файла
    dd start                ; адрес, на который система передаёт управление
                        ; после загрузки приложения в память
    dd i_end                ; размер приложения
    dd mem                  ; Объем используемой памяти, для стека отведем 0х100 байт и выровним на грницу 4 байта
    dd mem                  ; расположим позицию стека в области памяти, сразу за телом программы. Вершина стека в диапазоне памяти, указанном выше
    dd 0x0              ; указатель на строку с параметрами.
    dd cur_dir_path
include 'macros.inc'
include 'box_lib.mac'
include 'load_lib.mac'
        @use_library    ;use load lib macros
start:
;universal load library/librarys
sys_load_library  library_name, cur_dir_path, library_path, system_path, \
err_message_found_lib, head_f_l, myimport, err_message_import, head_f_i
;if return code =-1 then exit, else nornary work
        cmp     eax,-1
        jz      exit
        mcall   40,0x27         ;установить маску для ожидаемых событий
red_win:
    call draw_window            ;первоначально необходимо нарисовать окно
align 4
still:                          ;основной обработчик
        mcall   10              ;Ожидать события
        dec  eax
        jz   red_win
        dec  eax
        jz   key
        dec  eax
        jz   button

        push    dword edit1
        call    [edit_box_mouse]

        push    dword edit2
        call    [edit_box_mouse]

        push    dword check1
        call    [check_box_mouse]

        push    dword check2
        call    [check_box_mouse]

        push    dword Option_boxs
        call    [option_box_mouse]

        push    dword Option_boxs2
        call    [option_box_mouse]

        jmp still    ;если ничего из перечисленного то снова в цикл
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
button:
        mcall   17      ;получить идентификатор нажатой клавиши
        test ah,ah      ;если в ah 0, то перейти на обработчик событий still
        jz  still
exit:   mcall   -1
key:
        mcall   2       ;загрузим значение 2 в регистор eax и получим код нажатой клавиши

        push    dword edit1
        call    [edit_box_key]

        push    dword edit2
        call    [edit_box_key]

        jmp still

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
align 4
draw_window:            ;рисование окна приложения
        mcall   12,1
        mcall   0,(50*65536+390),(30*65536+200),0x33AABBCC,0x805080DD,hed

        push    dword edit1
        call    [edit_box_draw]

        push    dword edit2
        call    [edit_box_draw]

        push    dword check1
        call    [check_box_draw]

        push    dword check2
        call    [check_box_draw]

        push    dword Option_boxs
        call    [option_box_draw]        

        push    dword Option_boxs2
        call    [option_box_draw]

        mcall   12,2
    ret
;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;DATA данные
;Всегда соблюдать последовательность в имени.
system_path      db '/sys/lib/'
library_name     db 'box_lib.obj',0
; Если есть желание разъединить, то нужно использовать следующию конструкцию
;system_path      db '/sys/lib/box_lib.obj',0
;... любая последовательность других команд и определений.
;library_name     db 'box_lib.obj',0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

err_message_found_lib   db 'Sorry I cannot load library box_lib.obj',0
head_f_i:
head_f_l        db 'System error',0
err_message_import      db 'Error on load import library box_lib.obj',0

myimport:   

edit_box_draw   dd      aEdit_box_draw
edit_box_key    dd      aEdit_box_key
edit_box_mouse  dd      aEdit_box_mouse
version_ed      dd      aVersion_ed

check_box_draw  dd      aCheck_box_draw
check_box_mouse dd      aCheck_box_mouse
version_ch      dd      aVersion_ch

option_box_draw  dd      aOption_box_draw
option_box_mouse dd      aOption_box_mouse
version_op       dd      aVersion_op

                dd      0
                dd      0

aEdit_box_draw  db 'edit_box',0
aEdit_box_key   db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
aVersion_ed     db 'version_ed',0

aCheck_box_draw  db 'check_box_draw',0
aCheck_box_mouse db 'check_box_mouse',0
aVersion_ch      db 'version_ch',0

aOption_box_draw  db 'option_box_draw',0
aOption_box_mouse db 'option_box_mouse',0
aVersion_op       db 'version_op',0




check1 check_box 10,45,6,12,0x80AABBCC,0,0,check_text,14,ch_flag_en
check2 check_box 10,60,6,12,0x80AABBCC,0,0,check_text2,15

edit1 edit_box 350,3,5,0xffffff,0x6f9480,0,0xAABBCC,0,308,hed,ed_focus,hed_end-hed-1,hed_end-hed-1
edit2 edit_box 350,3,25,0xffffff,0x6a9480,0,0,0,99,ed_buffer,ed_figure_only

op1 option_box option_group1,10,90,6,12,0xffffff,0,0,op_text.1,op_text.e1-op_text.1
op2 option_box option_group1,10,105,6,12,0xFFFFFF,0,0,op_text.2,op_text.e2-op_text.2
op3 option_box option_group1,10,120,6,12,0xffffff,0,0,op_text.3,op_text.e3-op_text.3
op11 option_box option_group2,120,90,6,12,0xffffff,0,0,op_text.1,op_text.e1-op_text.1
op12 option_box option_group2,120,105,6,12,0xffffff,0,0,op_text.2,op_text.e2-op_text.2
op13 option_box option_group2,120,120,6,12,0xffffff,0,0,op_text.3,op_text.e3-op_text.3

option_group1   dd op1  ;указатели, они отображаются по умолчанию, когда выводится 
option_group2   dd op12 ;приложение
Option_boxs     dd  op1,op2,op3,0
Option_boxs2    dd  op11,op12,op13,0
hed db   'BOXs load from lib <Lrz> date 27.04.2009',0
hed_end:
rb  256
check_text db 'First checkbox'
check_text2 db 'Second checkbox'
op_text:                ; Сопровождающий текст для чек боксов
.1 db 'Option_Box #1' 
.e1:
.2 db 'Option_Box #2'
.e2:
.3 db 'Option_Box #3'
.e3:
ed_buffer       rb 100
;-----------------------
;sc      system_colors
p_info  process_information
cur_dir_path    rb 4096
library_path    rb 4096
i_end:
rb 1024
mem: