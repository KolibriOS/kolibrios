;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Hot Angles Config ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compile with FASM
; Version 0.3.6: Oct 9, 2018

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
include '../../develop/libraries/box_lib/load_lib.mac'      ;макрос для загрузки библиотек
@use_library

KMENUITEM_NORMAL    equ 0
KMENUITEM_SUBMENU   equ 1
KMENUITEM_SEPARATOR equ 2

Otstup = 30

START:
    call copyKill        ;простейшая защита от запуска второй копии приложения
    mcall  68, 11        ;инициализация кучи

    ;mcall 48,3,sc,sizeof.system_colors ;получить системные цвета
    mov  eax,48          ;получить системные цвета
    mov  ebx,3
    mov  ecx, sc
    mov  edx, sizeof.system_colors
    mcall

    stdcall dll.Load, @IMPORT
    or      eax, eax
    jnz     exit

    mov  eax,40          ;установить маску для ожидаемых событий
    mov  ebx,0x27        ;система будет реагировать только на сообщение о перерисовке, нажата кнопка, определённая ранее, событие от мыши (что-то случилось - нажатие на кнопку мыши или перемещение; сбрасывается при прочтении)
    mcall

    load_libraries l_libs_start,load_lib_end ;загрузка библиотек(и)
    stdcall [OpenDialog_Init],OpenDialog_data ;подготовка диалога

    ;mode menu
    stdcall [kmenu_init], sc    ;kmenu initialisation
    stdcall [ksubmenu_new]
    mov [modeMenu], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, valueModeMenu0, 110
    stdcall [ksubmenu_add], [modeMenu], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, valueModeMenu1, 111
    stdcall [ksubmenu_add], [modeMenu], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, valueModeMenu2, 112
    stdcall [ksubmenu_add], [modeMenu], eax
    stdcall [kmenuitem_new], KMENUITEM_NORMAL, valueModeMenu3, 113
    stdcall [ksubmenu_add], [modeMenu], eax
        stdcall [kmenuitem_new], KMENUITEM_SUBMENU, valueModeMenu1, [modeMenu]

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
    ;stdcall [kmainmenu_dispatch_cursorevent], [modeMenu] ;проверка мыши для комбобокса

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
    jne  @f ;butend
    call applyButton   ;вызов нашего обработчика нажатия кнопки
    jmp event_wait
@@:
    cmp ah, 4
    jne @f
    mov byte[editID], 4
    call but_open_dlg
    jmp  event_wait
@@:
    cmp ah, 5
    jne @f
    mov byte[editID], 5
    call but_open_dlg
    jmp  event_wait
@@:
    cmp ah, 6
    jne @f
    mov byte[editID], 6
    call but_open_dlg
    jmp  event_wait
@@:
    cmp ah, 7
    jne @f
    mov byte[editID], 7
    call but_open_dlg
    jmp  event_wait
@@:
    cmp ah, 8
    jne @f
    push eax ebx ecx
    mcall 9, pi, -1 ;get window coord

                mov eax, dword[pi+34]
                add eax, buttonModeX ;+ 5
                mov word[coordModeMenu.x], ax

                mov eax, dword[pi+38]
                add eax, buttonModeY + 17 ;20 ;42
                mov word[coordModeMenu.y], ax

                stdcall [ksubmenu_draw], [modeMenu], coordModeMenu
    pop ecx ebx eax
    jmp  event_wait
@@:
    cmp ah, 110
    jne @f
    mov byte[mode], 48 ;'0'
    jmp  event_wait
@@:
    cmp ah, 111
    jne @f
    mov byte[mode], 49 ;'1'
    jmp  event_wait
@@:
    cmp ah, 112
    jne @f
    mov byte[mode], 50 ;'2'
    jmp  event_wait
@@:
    cmp ah, 113
    jne @f
    mov byte[mode], 51 ;'3'
    jmp  event_wait
@@:
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

   cmp ecx, 0          ;пуста ли строка?
   jne copyParam.copy  ;если нет, то переходим к копированию содержимого строки (к  циклу)

   mov byte[eax], '-'  ;если же пусто, то заменяем символом минус '-'
   inc eax
   ;jmp copyParam.copyEnd ;и выходим
   ;далее в начале цикла произойдёт сравнение и выход

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

        ;если заданы пустые строки, то мы автоматически добавляем знак минус, значит размер +1 байт
        cmp dword[editLU.size], 0
        jne @f
        inc ecx
@@:
        cmp dword[editLD.size], 0
        jne @f
        inc ecx
@@:
        cmp dword[editRU.size], 0
        jne @f
        inc ecx
@@:
        cmp dword[editRD.size], 0
        jne @f
        inc ecx
@@:

        ;теперь размер файла конфигурации в ecx
        mov dword[createFile.size], ecx ;положим размер байт в структуру создания/перезаписи файла
        mcall 68, 12, ecx ;выделяем блок памяти под буфер содержимого
        mov dword[createFile.data_adr], eax ;сохраняем адрес буфера в структуру

        ;заполним буфер содержимого:
        ;cmp byte[mode], 0
        ;jne @f
        ;@@:
        ;

        push ebx    ;сохраняем выбранный режим
        mov bl, byte[mode]
        mov byte[eax], bl
        pop ebx

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
        call finishHotAnglesApp ;завершаем работу всех копий сервиса @HOTANGLES
        call loadConfig         ;перезагружаем файл конфига
        call draw_window        ;перерисовываем окно
        call startHotAnglesApp  ;перезапускаем @HOTANGLES
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


finishHotAnglesApp:
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
   jne finishHotAnglesApp.next   ;если не совпало, то переходим на следующий слот

   mcall 18, 2, ecx               ;иначе завершаем слот
.next:
   inc  ecx
   cmp ecx, dword[slotMax]
   ja @f
   jmp @b
@@:
   pop edi esi ecx ebx eax
   ret

startHotAnglesApp:
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

   push ebx
   mov bl, byte[eax]
   cmp bl, 48          ; если значение выходит за допустимые пределы, то проверяем на 'n' (для совместимости)
   jb loadConfig.checkN
   cmp bl, 51
   ja loadConfig.checkN

   mov byte[mode], bl  ;иначе устанавливаем этот режим как есть
   jmp loadConfig.modeEnd   ;и идём дальше
.checkN:
   cmp bl, 'n'             ;если n - то режим "отключено"
   jne @f
   mov byte[mode], 48 ;'0'
   jmp loadConfig.modeEnd
@@:
   mov byte[mode], 49 ;'1' иначе по умолчанию включён классический режим работы
.modeEnd:
   pop ebx
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

        ;удаляем ещё 4 кнопки если они есть
        mov edx, 0x80000004
        mcall 8
        mov edx, 0x80000005
        mcall 8
        mov edx, 0x80000006
        mcall 8
        mov edx, 0x80000007
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
        mov     esi, buttonTextSize  ;размер текста в символах
        mcall 4

        ;рисуем ещё 4 кнопки для диалогового окна выбора файла
        ;номера 4,5,6 и 7
        buttonH = 14
        buttonW = 30
        ;левый верхний угол
        mov ebx, Otstup*65536 + buttonW ;координата X, ширина кнопки
        mov ecx, 85*65536 + buttonH   ;координата Y, высота кнопки
        mov edx, 0x00000004       ;номер кнопки
        mov esi, 0x00AABBCC       ;цвет кнопки
        mcall 8
        ;левый нижний угол
        mov ebx, Otstup*65536 + buttonW ;координата X, ширина кнопки
        mov ecx, 120*65536 + buttonH   ;координата Y, высота кнопки
        mov edx, 0x00000005       ;номер кнопки
        mov esi, 0x00AABBCC       ;цвет кнопки
        mcall 8
        ;правый верхний угол
        mov ebx, (WindowsWidth-Otstup-buttonW)*65536 + buttonW ;координата X, ширина кнопки
        mov ecx, 85*65536 + buttonH   ;координата Y, высота кнопки
        mov edx, 0x00000006       ;номер кнопки
        mov esi, 0x00AABBCC       ;цвет кнопки
        mcall 8
        ;правый нижний угол
        mov ebx, (WindowsWidth-Otstup-buttonW)*65536 + buttonW ;координата X, ширина кнопки
        mov ecx, 120*65536 + buttonH   ;координата Y, высота кнопки
        mov edx, 0x00000007       ;номер кнопки
        mov esi, 0x00AABBCC       ;цвет кнопки
        mcall 8

        ;рисуем тексты для этих 4 кнопок
        mov     ecx, 0xFFFFFF              ;белый цвет текста
        mov     edx, commonButtonText      ;строка текста
        mov     esi, commonButtonTextSize  ;размер текста в символах
        ;координаты текста (x, y)
        deltaCommonTextX = 7
        deltaCommonTextY = 2
        mov     ebx, (Otstup+deltaCommonTextX) * 65536 + (85 + deltaCommonTextY)
        mcall 4
        mov     ebx, (Otstup+deltaCommonTextX) * 65536 + (120 + deltaCommonTextY)
        mcall 4
        mov     ebx, (WindowsWidth-Otstup-buttonW+deltaCommonTextX) * 65536 + (85 + deltaCommonTextY)
        mcall 4
        mov     ebx, (WindowsWidth-Otstup-buttonW+deltaCommonTextX) * 65536 + (120 + deltaCommonTextY)
        mcall 4

        ;delete mode button, if it exist
        mov edx, 0x80000008
        mcall 8
        ;draw button Mode
        buttonModeX = 30 ;433
        buttonModeY = 160 ;50
        buttonModeWidth = 221
        buttonModeHeight = 20
        buttonModeTextXoffset = 5
        mov ebx, buttonModeX*65536 + buttonModeWidth  ;X + Width
        mov ecx, buttonModeY*65536 + buttonModeHeight ;Y + Height
        mov edx, 0x00000008    ;button id
        mov esi, 0x00FFFFFF    ;color button
        mcall 8
        mov ebx, (buttonModeX+2+buttonModeWidth-20)*65536 + (20-2)  ;X + Width
        mov ecx, (buttonModeY+1)*65536 + buttonModeHeight-2   ;Y + Height
        mov edx, 0xAABBCC
        mcall 13
        ;draw text for button Mode
        mov     ebx, (buttonModeX+buttonModeTextXoffset) * 65536 + (buttonModeY+6)    ;(x, y)
        mov     ecx, 0x000000
        call setTextModeMenu
        mov     esi, valueModeMenuSize
        mcall 4
        mov     ebx, (buttonModeX+buttonModeWidth-12) * 65536 + (buttonModeY+6)
        mov     ecx, 0x80FFFFFF
        mov     edx, symbolDownArrow
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


setTextModeMenu:
   cmp byte[mode], 48
   jne @f
        mov edx, valueModeMenu0
        ret
@@:
   cmp byte[mode], 49
   jne @f
        mov edx, valueModeMenu1
        ret
@@:
   cmp byte[mode], 50
   jne @f
        mov edx, valueModeMenu2
        ret
@@:
   cmp byte[mode], 51
   jne @f
        mov edx, valueModeMenu3
        ret
@@:
   mov edx, valueModeMenu1
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

commonButtonText db '...', 0  ;текст для кнопок вызова OpenDial
commonButtonTextSize = 3

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

valueModeMenuSize = 32
ModeMenu  db 'ђҐ¦Ё¬ а Ў®вл:',0
valueModeMenu0  db '®вЄ«озҐ­®                       ',0
valueModeMenu1  db 'Є« ббЁзҐбЄЁ© аҐ¦Ё¬              ',0
valueModeMenu2  db 'аҐ¦Ё¬ Ј®апзҐ© Є« ўЁиЁ           ',0
valueModeMenu3  db 'аҐ¦Ё¬ ¦Ґбв  Ї® Є« ўЁиҐ  ЄвЁў жЁЁ',0

head_f_i:
        head_f_l  db '‘ЁбвҐ¬­ п ®иЁЎЄ ',0
        err_message_found_lib0 db 'ЌҐ ­ ©¤Ґ­  ЎЁЎ«Ё®вҐЄ  ',39,'proc_lib.obj',39,0
        err_message_import0 db 'ЋиЁЎЄ  ЇаЁ Ё¬Ї®авҐ ЎЁЎ«Ё®вҐЄЁ ',39,'proc_lib.obj',39,0
        err_message_found_lib1 db 'ЌҐ ­ ©¤Ґ­  ЎЁЎ«Ё®вҐЄ  ',39,'kmenu.obj',39,0
        err_message_import1 db 'ЋиЁЎЄ  ЇаЁ Ё¬Ї®авҐ ЎЁЎ«Ё®вҐЄЁ ',39,'kmenu',39,0

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

valueModeMenuSize = 32
ModeMenu  db 'Mode:',0
valueModeMenu0  db 'disabled                        ',0
valueModeMenu1  db 'classic mode                    ',0
valueModeMenu2  db 'hot key mode                    ',0
valueModeMenu3  db 'activation key gesture mode     ',0

head_f_i:
        head_f_l  db 'System error',0
        err_message_found_lib0 db 'Could not find library ',39,'proc_lib.obj',39,0
        err_message_import0 db 'Error importing library ',39,'proc_lib.obj',39,0
        err_message_found_lib1 db 'Could not find library ',39,'kmenu.obj',39,0
        err_message_import1 db 'Error importing library ',39,'kmenu',39,0

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

   mov     ebx, Otstup * 65536 + (buttonModeY-11)
   or ecx, 0x80000000
   mov     edx, ModeMenu
   mov     esi, 3
   mcall 4

   pop esi edi edx ecx ebx eax
   ret

editID db 0
initEBX:
   cmp byte[editID], 4
   jne @f
   mov ebx, dataBuffer.1
   ret
@@:
   cmp byte[editID], 5
   jne @f
   mov ebx, dataBuffer.2
   ret
@@:
   cmp byte[editID], 6
   jne @f
   mov ebx, dataBuffer.3
   ret
@@:
   cmp byte[editID], 7
   jne @f
   mov ebx, dataBuffer.4
   ret
@@:
   ret
setRESULT:
   cmp byte[editID], 4
   jne @f
   mov dword[dataBuffer.size1], ecx
   mov dword[editLU.size], ecx
   mov dword[editLU.pos], ecx
   ret
@@:
   cmp byte[editID], 5
   jne @f
   mov dword[dataBuffer.size2], ecx
   mov dword[editLD.size], ecx
   mov dword[editLD.pos], ecx
   ret
@@:
   cmp byte[editID], 6
   jne @f
   mov dword[dataBuffer.size3], ecx
   mov dword[editRU.size], ecx
   mov dword[editRU.pos], ecx
   ret
@@:
   cmp byte[editID], 7
   jne @f
   mov dword[dataBuffer.size4], ecx
   mov dword[editRD.size], ecx
   mov dword[editRD.pos], ecx
   ret
@@:
   ret
copyPath:
   push eax ebx ecx edx ;copy file name path
        mov eax, openfile_path ;dword[OpenDialog_data.openfile_path]
        call initEBX ;mov ebx, dataBuffer.1 ;.data
        mov ecx, 0
      @@:
        mov dl, byte[eax]
        cmp dl, 0 ;byte[eax], 0
        je @f
        mov byte[ebx], dl
        inc eax
        inc ebx
        inc ecx
        jmp @b
      @@:
        mov byte[ebx], 0
        call setRESULT
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
                call copyPath
                jmp .end_open
        @@:
                ;код при не удачном открытии диалога
                ;...
        .end_open:
        popad
        ret
 
;данные для диалога открытия файлов
align 4
OpenDialog_data:
.type                   dd 0 ;0 - открыть, 1 - сохранить, 2 - выбрать директорию
.procinfo               dd procinfo
.com_area_name          dd communication_area_name ;+8
.com_area               dd 0           ;+12
.opendir_path           dd plugin_path ;+16
.dir_default_path       dd default_dir ;+20
.start_path             dd file_name   ;+24 путь к диалогу открытия файлов
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
 
default_dir db '/rd/1',0 ;директория по умолчанию
communication_area_name: db 'FFFFFFFF_open_dialog',0
open_dialog_name:  db 'opendial',0
communication_area_default_path: db '/rd/1/File managers/',0
 
Filter:
dd Filter.end - Filter.1
.1:
;db 'KEX',0
.end:
db 0

mode db 49 ;'1'
modeMenu dd 0
coordModeMenu:
  .x dw 100
  .y dw 200


align 4
proclib_import: ;описание экспортируемых функций
        OpenDialog_Init dd aOpenDialog_Init
        OpenDialog_Start dd aOpenDialog_Start
dd 0,0
        aOpenDialog_Init db 'OpenDialog_init',0
        aOpenDialog_Start db 'OpenDialog_start',0
 
system_dir0 db '/sys/lib/'
lib0_name db 'proc_lib.obj',0
lib1_name db 'kmenu.obj',0

symbolDownArrow db 25,0

;library structures
l_libs_start:
    lib0 l_libs lib0_name, sys_path, file_name, system_dir0, err_message_found_lib0, head_f_l, proclib_import,err_message_import0, head_f_i
    lib1 l_libs lib1_name, sys_path, file_name, system_dir0, err_message_found_lib1, head_f_l, import_libkmenu,err_message_import1,head_f_i
load_lib_end:


edMaxSize = 510 ;теперь с новым компонентом можно и 511 указать
edMax = 0 ;max, size, pos

align 4
editLU   edit_box 180-buttonW,Otstup+buttonW,85, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0, edMaxSize, dataBuffer.1, mouse_dd, 0, edMax, edMax  ;ed_focus
editLD   edit_box 180-buttonW,Otstup+buttonW,120, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0, edMaxSize, dataBuffer.2, mouse_dd, 0, edMax, edMax
editRU   edit_box 180-buttonW,240,85, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0, edMaxSize, dataBuffer.3, mouse_dd, 0, edMax, edMax
editRD   edit_box 180-buttonW,240,120, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0, edMaxSize, dataBuffer.4, mouse_dd, 0, edMax, edMax

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

mouse_dd  dd ?
sc  system_colors

button_press  rd 0     ;for kmenu

sys_path:      rb 4096
file_name:     rb 4096
plugin_path:   rb 4096
openfile_path: rb 4096
filename_area: rb 256
rb 1024
procinfo process_information
pi rb 1024


dataBuffer:
.1: rb 512
.2: rb 512
.3: rb 512
.4: rb 512
.size1: rd 1
.size2: rd 1
.size3: rd 1
.size4: rd 1
;255+255+2 = 512
;два запасных байта необходимы для того что бы не пепереписать следующией байты, в конце буфера 0
dataBufferEnd:


I_END:
        rb 4096
align 16
STACKTOP:

MEM:
