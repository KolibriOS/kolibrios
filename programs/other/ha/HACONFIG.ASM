;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Hot Angles Config ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compile with FASM
; Version 0.3.1: Sep 23, 2018

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

; THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
; INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
; PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
; OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
; SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
; --------------------------------------------------------------------------------------

;P.S. Коду требуется оптимизация и доработка (см. тему форума)
format binary as ""     ; Binary file format without extension
use32                   ; Tell compiler to use 32 bit instructions
org 0                   ; the base address of code, always 0x0

; Заголовок исполняемого файла Колибри ОС
db 'MENUET01'
dd 1
dd START
dd I_END
dd MEM
dd STACKTOP
dd 0, 0

; Начало области кода
include 'lang.inc'       ;поддержка языков при компиляции
include '../../macros.inc'
include '../../proc32.inc'
include '../../dll.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac' ;компоненты checkBox и editBox

Otstup = 30

START:
    call copyKill        ;простейшая защита от запуска второй копии приложения
    mcall  68, 11        ;инициализация кучи

    mov  eax,48          ;получить системные цвета
    mov  ebx,3
    mov  ecx, sc
    mov  edx, sizeof.system_colors
    mcall


    stdcall dll.Load, @IMPORT
    or      eax, eax
    jnz     exit

    invoke  init_checkbox, ch1

    mov  eax,40          ;установить маску для ожидаемых событий
    mov  ebx,0x27        ;система будет реагировать только на сообщение о перерисовке, нажата кнопка, определённая ранее, событие от мыши (что-то случилось - нажатие на кнопку мыши или перемещение; сбрасывается при прочтении)
    mcall

    call  loadConfig     ;загружаем конфигурацию из файла, если нет - то выставляем параметры по умолчанию
    call  draw_window    ;отрисовать окно приложения

; Обработка цикла событий
event_wait:
    mov     eax, 10      ; function 10 : wait until event
    mcall                ; event type is returned in eax

    cmp     eax, 1       ; Event redraw request ?
    je      red          ; Expl.: there has been activity on screen and
                         ; parts of the applications has to be redrawn.

    cmp     eax, 2       ; Event key in buffer ?
    je      key          ; Expl.: User has pressed a key while the
                         ; app is at the top of the window stack.

    cmp     eax, 3       ; Event button in buffer ?
    je      button       ; Expl.: User has pressed one of the
                         ; applications buttons.

    invoke  edit_box_mouse, editLU   ;проверка событий мыши для editBox'ов
    invoke  edit_box_mouse, editLD
    invoke  edit_box_mouse, editRU
    invoke  edit_box_mouse, editRD
    invoke  check_box_mouse, ch1     ;проверка чек бокса

    jmp     event_wait


red:                     ;обработка события перерисовки окна
   call    draw_window
   jmp     event_wait

key:                                    ; Keypress event handler
        mov     eax, 2                  ; The key is returned in ah. The key must be
        mcall                           ; read and cleared from the system queue.

        invoke  edit_box_key, editLU
        invoke  edit_box_key, editLD
        invoke  edit_box_key, editRU
        invoke  edit_box_key, editRD
        jmp     event_wait              ; Just read the key, ignore it and jump to event_wait.

button:                                 ; Buttonpress event handler
    mov     eax,17                  ; The button number defined in window_draw
    mcall                           ; is returned to ah.

    cmp     ah,1                    ; button id=1 ?
    jne     noclose
exit:
    mov     eax,-1                  ; Function -1 : close this program
    mcall
noclose:
    cmp  ah, 2         ;кнопка "Применить"
    jne  butend
    call applyButton   ;вызов нашего обработчика нажатия кнопки
butend:
    jmp  event_wait    ;this is for ignored events, useful at development



;имя конфигурационного файла
fileName: db 'SETTINGS/HOTANGLES.CFG', 0

;структура для создания/перезаписи файла
createFile:
  .func     dd 2    ;номер подфункции
  .re       dd 0, 0 ;зарезервировано
  .size     dd 0    ;сколько байт писать
  .data_adr dd 0    ;указатель на данные
  .path     db 0
  .path_adr dd fileName    ;путь

;строка параметров по умолчанию и её размер
defaultDataForConfig db 121,13,10,'/rd/1/HACONFIG',13,10,'/rd/1/HACONFIG',13,10,'/rd/1/HACONFIG',13,10,'/rd/1/HACONFIG',13,10
defaultDataSize = 48+19


copyParam:  ;копирует параметр(команду запуска) в буфер (см. дальше)
   push esi ebx ecx
   mov ecx, dword[ebx] ;читаем размер строки
.copy:  ;копируем символы
   cmp ecx, 0
   je copyParam.copyEnd
   mov bl, byte[esi]
   mov byte[eax], bl
   dec ecx
   inc eax
   inc esi
   jmp copyParam.copy
.copyEnd:
   pop ecx ebx esi
   ret


addPerenos:  ;добавляет перенос строки в буфер
   mov byte[eax], 13
   inc eax
   mov byte[eax], 10
   inc eax
   ret

;обработчик кнопки "Применить"
applyButton:
    push eax ebx ecx
        ;mov dword[createFile.data_adr], defaultDataForConfig
        ;mov dword[createFile.size], defaultDataSize

        ;вычислим размер файла конфигурации в байтах
        mov ecx, 5*2+1 ;1 байт на состояние активности и 5 пар переносов строк (символы 13, 10)
        add ecx, dword[editLU.size] ;плюс размер каждой строки в байтах
        add ecx, dword[editLD.size]
        add ecx, dword[editRU.size]
        add ecx, dword[editRD.size] ;dataBuffer.size4]
        ;теперь размер файла конфигурации в ecx
        mov dword[createFile.size], ecx ;положим размер байт в структуру создания/перезаписи файла
        mcall 68, 12, ecx ;выделяем блок памяти под буфер содержимого
        mov dword[createFile.data_adr], eax ;сохраняем адрес буфера в структуру

        ;заполним буфер содержимого:
        mov byte[eax], 110; 'n' ;по умолчанию флаг снят
        cmp word[ch1.flags], 110b ;ch_flag_en and ch_flag_middle ;выставлен checkBox или нет?
        jne applyButton.no
        mov byte[eax], 121 ;иначе кладём 'y'
.no:
        inc eax
        call addPerenos

        push esi ebx
        call UpdateEditsSizes ;обновить размеры текста в edit'ах

        mov esi, dataBuffer.1     ;editLU.text
        mov ebx, dataBuffer.size1 ;editLU.size
        call copyParam
        call addPerenos     ;добавляем перенос строки

        mov esi, dataBuffer.2
        mov ebx, dataBuffer.size2
        call copyParam
        call addPerenos     ;добавляем перенос строки

        mov esi, dataBuffer.3
        mov ebx, dataBuffer.size3
        call copyParam
        call addPerenos     ;добавляем перенос строки

        mov esi, dataBuffer.4
        mov ebx, dataBuffer.size4
        call copyParam
        call addPerenos     ;добавляем перенос строки

        pop ebx esi


    mov ebx, createFile
    mcall 70
        cmp eax, 0
        je .end
        ;иначе здесь нужно вывести окно с ошибкой !!!
.end:
        mcall 68, 13, dword[createFile.data_adr]  ;освобождаем память под буфер
        call restartHotAnglesApp ;перезапускаем @HOTANGLES
        pop ecx ebx eax
    ret






compareBytes2:
   push edi esi ecx ebx
   xor eax, eax
   mov ecx, 10  ;max размер строк 11
@@:
   mov bl, byte[edi]
   cmp bl, byte[esi]
   jne compareBytes.no
   inc edi
   inc esi
   cmp ecx, 0
   je @f
   dec ecx
   jmp @b
.no:
   mov eax, 1
@@:
   pop ebx ecx esi edi
   ret


restartHotAnglesApp:
   ;находим все копии приложения по имени и завершаем их
   push eax ebx ecx esi edi
   ;указатели, которые никогда не меняются:
   mov esi, startAppData.file_name       ;первая строка - искомое имя приложения
   mov edi, buf            ;вторая строка - имя текущего слота
   add edi, 10

   mov ecx, 1
@@:
   mcall 9, buf, ecx              ;получаем данные очередного слота
   mov dword[slotMax], eax

   call compareBytes2 ;сравниваем имя
   cmp eax, 0
   jne restartHotAnglesApp.next   ;если не совпало, то переходим на следующий слот

   ;push eax ebx ecx
   mcall 18, 2, ecx               ;иначе завершаем слот
   ;pop ecx ebx eax

.next:
   inc  ecx
   cmp ecx, dword[slotMax]
   ja @f
   jmp @b
@@:
   pop edi esi ecx ebx eax

   ;запускаем приложение заново
   push eax ebx
   mcall 70, startAppData
   cmp eax, 0
   ja @f       ;если eax>0, то успешно запущено
   ;!!! иначе тут печатаем ошибку
@@:
   pop ebx eax
   ret

startAppData:
    .subfunction dd   7
    .flags       dd   0
    .param       dd   0
    .rezerv      dd   0, 0
    .full_name   db   '/rd/1/'
    .file_name   db   '@HOTANGLES', 0
    ;db 0
    ;.name: dd HotAnglesAppName
;HotAnglesAppName db '@HOTANGLES', 0


UpdateEditsSizes:
   push eax
   mov eax, dword[editLU.size]
   mov dword[dataBuffer.size1], eax

   mov eax, dword[editLD.size]
   mov dword[dataBuffer.size2], eax

   mov eax, dword[editRU.size]
   mov dword[dataBuffer.size3], eax

   mov eax, dword[editRD.size]
   mov dword[dataBuffer.size4], eax

   pop eax
   ret

fileAdr dd 0

loadConfig:
   push eax ebx ecx edx
   mcall 68, 27, fileName   ;загружаем конфигурационный файл в ОЗУ
   mov dword[fileAdr], eax  ;сохраняем адрес блока памяти файла
   cmp eax, 0
   je loadConfig.default       ;если файла конфигурации нет, то ставим настройки по умолчанию
   ;иначе данные загружены в ОЗУ, размер в edx
   cmp edx, 0               ;если файл пуст, в нём нет данных, то ставим настройки по умолчанию
   je loadConfig.default
.start:
   add edx, eax             ;кладём в edx - адрес конца конфиг.данных

.chStart:
   cmp byte[eax], 121 ;'y'  ;выставляем checkBox - параметр активности приложения
   jne loadConfig.no
.yes:
   mov word[ch1.flags], 110b ;ch_flag_en and ch_flag_middle
   jmp loadConfig.chEnd
.no:
   mov word[ch1.flags], 100b ;ch_flag_middle
.chEnd:
   ;jmp loadConfig.end   ;команда для удобства отладки и поиска ошибок

   ;копируем содержимое строк в буфер
   push edi esi ecx ebx eax
   ;инициализируем указатели на адреса буфера строки и её размера
   mov edi, dataBuffer.1
   mov esi, dataBuffer.size1
   ;ecx - счётчик байт в строке
.block:
   xor ecx, ecx ;обнуляем счётчик
   inc eax
   cmp byte[eax], 10         ;если очередной код символа 10 или 13, то пропускаем символы
   je loadConfig.block       ;до первого отличного от них
   cmp byte[eax], 13
   je loadConfig.block

   ; символ отличен от переноса строки и возврата каретки - начинаем копировать отсюда в буфер
   mov bl, byte[eax]
   mov byte[edi], bl
   inc edi
   inc eax
   inc ecx ;увеличиваем счётчик символов в строке

   ;идём до конца этой строки: пока не встретим очередной 10, 13, 0 или file end
.while:     ;и копируем всё в буфер
   cmp eax, edx           ;проверяем закончился ли файл, а конца строки не было !!!
   ja loadConfig.ura
   cmp byte[eax], 10
   je loadConfig.ura
   cmp byte[eax], 0
   je loadConfig.ura
   cmp byte[eax], 13
   je loadConfig.ura

   mov bl, byte[eax]
   mov byte[edi], bl
   inc edi
   inc eax
   inc ecx
   cmp ecx, 511
   jae loadConfig.ura
   jmp loadConfig.while
.ura:
   mov byte[edi], 0      ;кладём завершающий 0 в строку
   mov dword[esi], ecx   ;сохраняем размер строки
   add esi, 4            ;переходим на размер следующего буфера
   sub edi, ecx          ;переходим на следующий буфер
   add edi, 512
   ;xor ecx, ecx          ;обнуляем размер строки

   cmp edi, dataBuffer.size1 ;если это был последний буфер (и мы вышли за адреса размеров буферов), то выходим
   jb loadConfig.block

   pop eax ebx ecx esi edi

   jmp loadConfig.end
.default:
   mov eax, defaultDataForConfig
   mov edx, defaultDataSize
   jmp loadConfig.start
.end:
   mov ecx, dword[fileAdr]    ;если файл был загружен, то
   cmp ecx, 0
   je @f
   mcall 68, 13, ecx  ;выгружаем файл из памяти
@@:
   call updateParams ;применяем настройки каждого буфера
   pop edx ecx ebx eax
   ret


updateParams: ;max, size, pos
   push eax
   mov eax, dword[dataBuffer.size1]
   mov dword[editLU.size], eax
   mov dword[editLU.pos], eax

   mov eax, dword[dataBuffer.size2]
   mov dword[editLD.size], eax
   mov dword[editLD.pos], eax

   mov eax, dword[dataBuffer.size3]
   mov dword[editRU.size], eax
   mov dword[editRU.pos], eax

   mov eax, dword[dataBuffer.size4]
   mov dword[editRD.size], eax
   mov dword[editRD.pos], eax
   pop eax
   ret



selfName db 'HACONFIG', 0
selfNameSize = 8 ;до 11 byte
;
compareBytes:
   push edi esi ecx ebx
   mov eax, 0 ;xor eax, eax
   mov ecx, selfNameSize    ;max размер строк 11
@@:
   mov bl, byte[edi]
   cmp bl, byte[esi]
   jne compareBytes.no
   inc edi
   inc esi
   cmp ecx, 0
   je @f
   dec ecx
   jmp @b
.no:
   mov eax, 1
@@:
   pop ebx ecx esi edi
   ret
;
slotMax dd 0
selfPID dd 0
buf db 1024 dup(0)
copyKill:
   push eax ebx ecx esi edi

   ;сперва прочтём свою информацию
   mcall 9, buf, -1
   mov eax, dword[buf+30]
   mov dword[selfPID], eax

   ;указатели, которые никогда не меняются:
   mov esi, selfName       ;первая строка - имя текущего приложения
   mov edi, buf            ;вторая строка - имя текущего слота
   add edi, 10

   mov ecx, 1
@@:
   mcall 9, buf, ecx
   mov dword[slotMax], eax

   ;если это мы сами, то пропускаем проверку
   mov eax, dword[buf+30]
   cmp eax, dword[selfPID]
   je copyKill.propusk

   call compareBytes   ;сравниваем 11 байт строк, результат в eax

   cmp eax, 0
   je copyKill.selfKill

.propusk:
   inc ecx
   cmp ecx, dword[slotMax]
   ja @f
   jmp @b

.selfKill:
   pop edi esi ecx ebx eax
   mcall -1
   ret
@@:
   pop edi esi ecx ebx eax
   ret

 WindowsWidth  = 450
 WindowsHeight = 195
coord:      ;координаты окна приложения
 .y dw 0
 .x dw 0

draw_window:
        ;удаляем кнопку, если есть
        mov edx, 0x80000002
        mcall 8

        mov     eax, 12                 ; function 12: tell os about windowdraw
        mov     ebx, 1                  ; 1, start of draw
        mcall

        ;отобразим окно по центру экрана для этого:
        mcall 14 ;получим размеры экрана
        mov dword[coord], eax       ;положим размеры экрана в координаты
        shr word[coord.x], 1        ;вернее положим их половину
        shr word[coord.y], 1

        ;вычисляем координаты окна, чтобы окно было по середине экрана
        sub word[coord.x], WindowsWidth/2
        sub word[coord.y], WindowsHeight/2

        mov     eax, 0                  ; function 0 : define and draw window
        mov     ebx, 0                  ; [x start] *65536 + [x size]
        mov bx, word[coord.x]
        shl  ebx, 16
        mov bx, WindowsWidth

        mov ecx, 0
        mov     cx, word[coord.y]       ; [y start] *65536 + [y size]
        shl ecx, 16
        mov cx, WindowsHeight

        mov     edx, 0x14ffffff         ; color of work area RRGGBB
                                        ; 0x02000000 = window type 4 (fixed size, skinned window)
        mov     esi, 0x808899ff         ; color of grab bar  RRGGBB
                                        ; 0x80000000 = color glide
        mov     edi, title
        mcall

        invoke  edit_box_draw, editLU  ;рисование edit box'ов
        invoke  edit_box_draw, editLD
        invoke  edit_box_draw, editRU
        invoke  edit_box_draw, editRD
 
        invoke  check_box_draw, ch1    ;рисование чекбоксов

        ;рисуем кнопку
        mov ebx, (WindowsWidth-80-Otstup)*65536 + 80 ;координата X, ширина кнопки
        mov ecx, 160*65536 + 20   ;координата Y, высота кнопки
        mov edx, 0x00000002 ;2    ;номер кнопки
        mov esi, 0x00AABBCC       ;цвет кнопки
        mcall 8
        ;рисуем текст кнопки
        mov     ebx, (WindowsWidth-80-Otstup+25-buttonTextXoffset) * 65536 + 166    ;координаты текста (x, y)
        mov     ecx, 0xFFFFFF     ;белый цвет текста
        mov     edx, buttonText
        mov     esi, buttonTextSize ;5            ;размер текста в символах
        mcall 4

        call draw_super_text

        LineLength = 62
        mov     ebx, 40 * 65536 + 36    ; draw info text with function 4 (x, y)
        mov     ecx, 0x224466
        mov     edx, text
        mov     esi, LineLength
        mov     eax, 4

  .newline:                             ; text from the DATA AREA
        mcall
        add     ebx, 12
        add     edx, LineLength
        cmp     byte[edx], 0
        jne     .newline

        mov     eax, 12                 ; function 12:tell os about windowdraw
        mov     ebx, 2                  ; 2, end of draw
        mcall

        ret




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


;DATA данные check_box использует системные цвета

;координаты (x,y) = (30, 160) размер чекбокса 12, цвет текста 0x224466
ch1     check_box2 30 shl 16 + 12, 160 shl 16 + 12, 6, 0xFFFFFFFF, 0xAABBCC, 0x224466, ch_text, 110b ; ch_flag_en and ch_flag_middle


if lang eq ru  ;если язык сборки русский

text    db  "Љ®­дЁЈга в®а Hot Angles ®вўҐз Ґв §  ­ бва®©Єг  ЄвЁў­ле гЈ«®ў. "
        db  "Ќ бва®©вҐ Ї а ¬Ґвал Ї®¤ бҐЎп Ё«Ё ®вЄ«озЁвҐ нвг ®ЇжЁо.         "
        db  "                                                              ", 0
 hed:
title   db  "Љ®­дЁЈга в®а гвЁ«Ёвл Hot Angles", 0
buttonText db "ЏаЁ¬Ґ­Ёвм", 0
buttonTextSize = 9
buttonTextXoffset = 12

ch_text db '‚Є«озЁвм/ ЄвЁўЁа®ў вм "Hot Angles"', 0

superText:
  .lu db 'Љ®¬ ­¤  «Ґў®Ј® ўҐае­ҐЈ® гЈ« : ', 0
  .ld db 'Љ®¬ ­¤  «Ґў®Ј® ­Ё¦­ҐЈ® гЈ« :  ', 0
  .ru db 'Љ®¬ ­¤  Їа ў®Ј® ўҐае­ҐЈ® гЈ« :', 0
  .rd db 'Љ®¬ ­¤  Їа ў®Ј® ­Ё¦­ҐЈ® гЈ« : ', 0
superTextSize = 30

else  ;иначе английский текст

text    db  "This is a Hot Angles Configurator.                            "
        db  "Set the parameters bellow for yourself or disable this option."
        db  "                                                              ", 0
 hed:
title   db  "Hot Angles Configurator", 0
buttonText db "Apply", 0
buttonTextSize = 5
buttonTextXoffset = 0

ch_text db 'Activate "Hot Angles"',0       ;сопровождающий текст для чек бокса

superText:
  .lu db 'Left up angle command:   ', 0
  .ld db 'Left down angle command: ', 0
  .ru db 'Right up angle command:  ', 0
  .rd db 'Right down angle command:', 0
superTextSize = 25

end if


draw_super_text:
   push eax ebx ecx edx edi esi
   mov     ebx, Otstup * 65536 + 74    ; draw info text with function 4 (x, y)
   mov     ecx, 0x224466
   mov eax, superText.lu
   mov     edx, eax
   mov     esi, superTextSize
   mcall 4

   mov     ebx, Otstup * 65536 + 109
   mov     edx, superText.ld
   mov     esi, superTextSize
   mcall 4

   mov     ebx, 240 * 65536 + 74
   mov     edx, superText.ru
   mov     esi, superTextSize
   mcall 4

   mov     ebx, 240 * 65536 + 109
   mov     edx, superText.rd
   mov     esi, superTextSize
   mcall 4

   pop esi edi edx ecx ebx eax
   ret


edMaxSize = 510 ;теперь с новым компонентом можно и 511 указать
edMax = 0 ;max, size, pos

align 4
editLU   edit_box 180,Otstup,85, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0, edMaxSize, dataBuffer.1, mouse_dd, 0, edMax, edMax  ;ed_focus
editLD   edit_box 180,Otstup,120, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0, edMaxSize, dataBuffer.2, mouse_dd, 0, edMax, edMax
editRU   edit_box 180,240,85, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0, edMaxSize, dataBuffer.3, mouse_dd, 0, edMax, edMax
editRD   edit_box 180,240,120, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0, edMaxSize, dataBuffer.4, mouse_dd, 0, edMax, edMax

dataBuffer:
.1: rb 512
.2: rb 512
.3: rb 512
.4: rb 512
.size1: rd 1
.size2: rd 1
.size3: rd 1
.size4: rd 1
;255+255+2
;два запасных байта необходимы для того что бы не пепереписать следующией байты, в конце буфера 0
dataBufferEnd:

mouse_dd  dd ?
sc  system_colors

I_END:
        rb 4096
align 16
STACKTOP:

MEM:
