;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Hot Angles Config ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compile with FASM
; Version 0.3.9: Apr 11, 2020

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

;P.S.  оду требуетс€ оптимизаци€ и доработка (см. тему форума)
format binary as ""     ; Binary file format without extension
use32                   ; Tell compiler to use 32 bit instructions
org 0                   ; the base address of code, always 0x0

; Заголовок исполн€емого файла  олибри ќ—
db 'MENUET01'
dd 1
dd START
dd I_END
dd MEM
dd STACKTOP
dd 0, sys_path

; Ќачало области кода
include 'lang.inc'       ;поддержка €зыков при компил€ции
include '../../macros.inc'
include '../../proc32.inc'
include '../../dll.inc'
include '../../KOSfuncs.inc'
include '../../develop/libraries/box_lib/trunk/box_lib.mac' ;компоненты checkBox и editBox
include '../../load_lib.mac'      ;макрос дл€ загрузки библиотек
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

; ќбработка цикла событий
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
    invoke  edit_box_mouse, editRadius

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
        invoke  edit_box_key, editRadius

        cmp eax, 1
        je event_wait
        cmp word[hkSymbol], 0
        jne event_wait
        cmp al, 0
        jne .next
        mov byte[hkSymbol], ah

        push eax ecx esi            ;сохраняем сканкод клавиши
        shr eax, 16
        mov byte[keyscan], al
        mov ecx, 0
        mov cl, al ;byte[keyscan]
        mov esi, keystr.data
        call IntToStr

        ;считаем полученный размер строки
        xor ecx, ecx
        mov esi, keystr.data
       @@:
        cmp byte[esi], 0
        je @f
        inc esi
        inc ecx
        ;cmp ecx, 4
        ;je @f
        jmp @b
       @@:
        ;и сохраняем его
        mov dword[keystr.size], ecx

        pop esi ecx eax
        jmp red
.next:
        ;cmp al, 2
        ;jne @f
        ;cmp ah, 0
        ;jne @f
        ;считываем управляющие клавиши
        ;shr eax, 16
        ;bt eax, 0   ;left shift
        ;jne @f
        ;mov byte[hkSymbol], 2
        ;jmp red
@@:
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
    cmp  ah, 2         ;кнопка "ѕрименить"
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
    add eax, buttonModeY + 20 ;20 - это смещение всплывающего меню
    mov word[coordModeMenu.y], ax

    stdcall [ksubmenu_draw], [modeMenu], coordModeMenu
    pop ecx ebx eax
    jmp  event_wait
@@:
    cmp ah, 9
    jne @f
    mov word[hkSymbol], 0
    jmp red ;event_wait
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
fileName: db 'SETTINGS/HA.CFG', 0

;структура для создания/перезаписи файла
createFile:
  .func     dd 2    ;номер подфункции
  .re       dd 0, 0 ;зарезервировано
  .size     dd 0    ;сколько байт писать
  .data_adr dd 0    ;указатель на данные
  .path     db 0
  .path_adr dd fileName    ;путь

;строка параметров по умолчанию и её размер
defaultDataForConfig db 121,13,10,'/sys/HACONFIG',13,10,'/sys/HACONFIG',13,10,'/sys/HACONFIG',13,10,'/sys/HACONFIG',13,10,'Space',13,10,'57',13,10,'10',13,10,'s',13,10
defaultDataSize = 85 ;48+19+4


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

;обработчик кнопки "ѕрименить"
applyButton:
    push eax ebx ecx
        ;mov dword[createFile.data_adr], defaultDataForConfig
        ;mov dword[createFile.size], defaultDataSize

        ;вычислим размер файла конфигурации в байтах
        mov ecx, 9*2+1 ;1 байт на состояние активности и 9 пар переносов строк (символы 13, 10)
        add ecx, dword[editLU.size] ;плюс размер каждой строки в байтах
        add ecx, dword[editLD.size]
        add ecx, dword[editRU.size]
        add ecx, dword[editRD.size] ;dataBuffer.size4]
        add ecx, dword[editRadius.size]
        add ecx, dword[hkSize] ;размер имени активационной кнопки
        add ecx, dword[keystr.size] ;размер сканкода активационной кнопки
        inc ecx ; ещё 1 байт для хранения размера элементов

        ;cmp dword[keystr.size], 3
        ;jbe @f
        ;mcall -1
      ;@@:

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

        ;сохраняем кнопку активации
        call saveKey
        call addPerenos

        ;сохраняем сканкод кнопки
        mov esi, keystr.data    ;записываем строку keystr в файл
        xor ebx, ebx
      @@:
        cmp byte[esi], 0
        je @f
        mov bl, byte[esi]
        mov byte[eax], bl
        inc eax
        inc esi
        cmp esi, keystr.size-1
        jae @f
        jmp @b
      @@:
        call addPerenos

        ;сохраняем радиус
        ;!!! если радиус оставлен пустым, то нужно сохранять значение по умолчанию (сейчас пишется '-')
        mov esi, radiusBuffer.data
        mov ebx, editRadius.size ;radiusBuffer.size
        call copyParam
        call addPerenos

        ;добавляем размер элементов
        push ebx    ;сохраняем выбранный режим
        mov bl, byte[sizeStyle]
        mov byte[eax], bl
        pop ebx
        inc eax
        call addPerenos

        pop ebx esi


    mov ebx, createFile
    mcall 70
        cmp eax, 0
        je .end
        ;иначе здесь нужно вывести окно с ошибкой !!!
.end:
        mcall 68, 13, dword[createFile.data_adr]  ;освобождаем память под буфер
        call finishHotAnglesApp ;завершаем работу всех копий сервиса @HA
        call loadConfig         ;перезагружаем файл конфига
        call draw_window        ;перерисовываем окно
        call startHotAnglesApp  ;перезапускаем @HA
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
    .full_name   db   '/sys/'
    .file_name   db   '@HA', 0

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

   mov eax, dword[editRadius.size]
   mov dword[radiusBuffer.size], eax

   pop eax
   ret

fileAdr dd 0

loadConfig:
   push eax ebx ecx edx
   mcall 68, 27, fileName   ;загружаем конфигурационный файл в ќ«”
   mov dword[fileAdr], eax  ;сохраняем адрес блока памяти файла
   cmp eax, 0
   je loadConfig.default       ;если файла конфигурации нет, то ставим настройки по умолчанию
   ;иначе данные загружены в ќ«”, размер в edx
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
   cmp bl, 'y'             ;если y - то классический режим
   jne @f
   mov byte[mode], 49 ;'1'
   jmp loadConfig.modeEnd
@@:
   mov byte[mode], 48 ;'0' иначе по умолчанию отключено
   ;mov byte[mode], 49 ;'1' иначе по умолчанию включён классический режим работы
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

   call readOptKey        ;считываем клавишу
   call gotoEndString

   ;считываем сканкод клавиши
   call readScanKey
   call gotoEndString

   call readOptRadius     ;считываем значение радиуса для mode 3
   call gotoEndString
   call readOptSize       ;считываем размер элементов
   call gotoEndString

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
   call setMySize    ;применяем настройки размера элементов
   pop edx ecx ebx eax
   ret


setMySize:
   push eax
   ;выставляем mysize в зависимости от значения sizeStyle
   cmp byte[sizeStyle], '0'
   jb @f
   cmp byte[sizeStyle], '7'
   ja @f
   mov eax, 0                  ;если значение размера задано, то его и выставляем
   mov al, byte[sizeStyle]
   sub al, '0'
   jmp .end
@@:
   cmp byte[sizeStyle], 'a'
   je .auto
   ;cmp byte[sizeStyle], 's'
   ;je .system
.system:     ;иначе значение по умолчанию: s - системная
;   push ebx ecx
   ;!!!
;   mov eax, 9
;   mcall 48, 11   ;получаем системный размер шрифта
   ;eax - высота шрифта в пикселях
;   mov ecx, eax
;   push eax
;   mcall 48, 12, ecx  ;применяем размер шрифта
;   pop eax
;   pop ecx ebx
   ;temp fix -  делим на текущий размер шрифта and -1
;   push ebx edx
;   mov ebx, 9
   ;div ebx
;   pop edx ebx
   ;cmp eax, 0
   ;je .end
   ;dec eax
   mov eax, 0 ;xor eax, eax ;test temp
   jmp .end
.auto:       ;a - выставляем в зависимости от разрешения экрана
   ;!!!
   mcall 14
   push ebx
   xor ebx, ebx
   mov bx, ax
   shr eax, 16
   cmp ax, bx       ;получаем больший размер стороны
   jae .a
   mov ax, bx
.a:
   pop ebx
   cmp ax, 1919     ;размеры полученные выше от 0 и на 1 меньше
   jb .size0
   cmp ax, 2047
   jb .size1
   cmp ax, 4095
   jb .size2
   cmp ax, 8191
   jb .size3

   jmp .size4

.size0:
   mov eax, 0
   jmp .end
.size1:
   mov eax, 1
   jmp .end
.size2:
   mov eax, 2
   jmp .end
.size3:
   mov eax, 3
   jmp .end
.size4:
   mov eax, 4
   jmp .end
.size5:
   mov eax, 5
   jmp .end
.size6:
   mov eax, 6
   jmp .end
.size7:
   mov eax, 7
   jmp .end

;.default:
;   mov eax, 1
.end:
   mov byte[mysize], al ;mov dword[mysize], eax
   pop eax
   ret


readScanKey:
   ;пропускаем переносы строк в начале
   cmp eax, edx  ;если файл закончился, то
   ja .default   ;настройки по умолчанию
   cmp byte[eax], 10
   je @f
   cmp byte[eax], 0
   je @f
   cmp byte[eax], 13
   je @f

   push ecx ebx esi
   mov ecx, 0 ;xor ecx, ecx
   mov esi, keystr.data
.copy:
   mov bl, byte[eax]
   cmp bl, 0
   je .copyEnd
   cmp bl, 10
   je .copyEnd
   cmp bl, 13
   je .copyEnd
   cmp esi, keystr.size-1
   jae .copyEnd
   mov byte[esi], bl
   inc esi
   inc eax
   inc ecx
   jmp .copy
.copyEnd:
   mov byte[esi], 0
   mov dword[keystr.size], ecx
   pop esi ebx ecx

   jmp .end
@@:
   inc eax
   jmp readScanKey
.default:
   mov byte[keyscan], 57
   mov word[keystr.data], '57'
   mov byte[keystr.data+2], 0
   mov dword[keystr.size], 2
.end:
   ret

readOptKey:  ;1;3;4;5 - space or shift
   ;пропускаем переносы строк в начале
   cmp eax, edx  ;если файл закончился, то
   ja .default   ;настройки по умолчанию
   cmp byte[eax], 10
   je @f
   cmp byte[eax], 0
   je @f
   cmp byte[eax], 13
   je @f
   ;иначе читаем клавишу и выходим
   cmp dword[eax], 'Ctrl'
   jne .notCtrl
   mov byte[hkSymbol], 3
   mov dword[hkSize], 4
   jmp .end
.notCtrl:
   cmp dword[eax], 'Shif'
   jne .notShift
   mov byte[hkSymbol], 2
   mov dword[hkSize], 5
   jmp .end
.notShift:
   cmp dword[eax], 'Spac'
   jne .notSpace
   mov byte[hkSymbol], 32
   mov dword[hkSize], 5
   jmp .end
.notSpace:
   cmp word[eax], 'Al'
   jne .notAlt
   mov byte[hkSymbol], 1
   mov dword[hkSize], 3
   jmp .end
.notAlt:
   ;убедимся, что там 1 символ, а не строка
   cmp byte[eax+1], 10
   je .oneSymb
   cmp byte[eax+1], 0
   je .oneSymb
   cmp byte[eax+1], 13
   je .oneSymb
   jmp .default
.oneSymb:
   ;проверяем, что символ разрешённый  !!!

   ;добавляем символ как есть
   push ebx
   ;xor ebx, ebx
   mov bl, byte[eax]
   mov byte[hkSymbol], bl
   mov dword[hkSize], 1
   pop ebx

   jmp .end
@@:
   inc eax
   jmp readOptKey

.default:
   mov byte[hkSymbol], 32
   mov dword[hkSize], 5
.end:
   mov byte[hkSymbol+1], 0
   ret

;проходим до конца текущей строки
gotoEndString:
   cmp eax, edx  ;если файл закончился, то
   ja @f         ;выходим
   cmp byte[eax], 10
   je @f
   cmp byte[eax], 0
   je @f
   cmp byte[eax], 13
   je @f
   ;иначе переходим на следующий символ
   inc eax
   jmp gotoEndString
@@:
   ret

readOptRadius:
   ;пропускаем переносы строк в начале
   cmp eax, edx  ;если файл закончился, то
   ja .default   ;настройки по умолчанию
   cmp byte[eax], 10
   je @f
   cmp byte[eax], 0
   je @f
   cmp byte[eax], 13
   je @f

   ;иначе считываем значение
   push ebx ecx
   xor ecx, ecx
.readValue:
   mov bl, byte[eax]
   mov byte[radiusBuffer.data+ecx], bl
   inc ecx
   inc eax
   cmp eax, edx  ;если файл закончился, то выход
   ja .readEnd
   cmp byte[eax], 10
   je .readEnd
   cmp byte[eax], 0
   je .readEnd
   cmp byte[eax], 13
   je .readEnd
   cmp ecx, 6
   jae .readEnd
   jmp .readValue
.readEnd:
   mov byte[radiusBuffer.data+ecx], 0
   mov dword[radiusBuffer.size], ecx
   pop ecx ebx

   jmp .end
@@:
   inc eax
   jmp readOptRadius
.default:
   mov byte[radiusBuffer.data], '0'
   mov byte[radiusBuffer.data+1], 0
   mov dword[radiusBuffer.size], 1
.end:

   ret


readOptSize:
   ;пропускаем переносы строк в начале
   cmp eax, edx  ;если файл закончился, то
   ja .default   ;настройки по умолчанию
   cmp byte[eax], 10
   je @f
   cmp byte[eax], 0
   je @f
   cmp byte[eax], 13
   je @f

   ;проверяем, что это допустимое значение: 1-7, a, s
   cmp byte[eax], 'a'
   je .setValue
   cmp byte[eax], 's'
   je .setValue
   cmp byte[eax], '0'
   je .setValue
   cmp byte[eax], '1'
   je .setValue
   cmp byte[eax], '2'
   je .setValue
   cmp byte[eax], '3'
   je .setValue
   cmp byte[eax], '4'
   je .setValue
   cmp byte[eax], '5'
   je .setValue
   cmp byte[eax], '6'
   je .setValue
   cmp byte[eax], '7'
   je .setValue

   jmp .default    ;иначе ставим значение по умолчанию

.setValue:
   push ebx
   mov bl, byte[eax]
   mov byte[sizeStyle], bl
   pop ebx

   jmp .end
@@:
   inc eax
   jmp readOptSize
.default:
   mov byte[sizeStyle], 's'   ;системный
.end:
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

   mov eax, dword[radiusBuffer.size]
   mov dword[editRadius.size], eax
   mov dword[editRadius.pos], eax
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

 WindowsWidth  = 550
 WindowsHeight = 235
coord:      ;координаты окна приложения
 .y dw 0
 .x dw 0

draw_window:
                mcall   12, 1  ; Start window redraw

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

        ;рисуем кнопку и ее текст
        buttonAppleY = WindowsHeight-40
        mcall 8, <WindowsWidth-120-Otstup, 120>, <buttonAppleY, 27>, 2, 0x00AABBCC
        mcall 4, <WindowsWidth-120-Otstup+27, buttonAppleY+6>, 0x10FFFFFF, buttonText, buttonTextSize

        ;рисуем ещё 4 кнопки для диалогового окна выбора файла
        ;номера 4,5,6 и 7
        buttonW = 30
        buttonH = 21
        ;левый верхний угол
        mcall 8, <Otstup,buttonW>, <85,buttonH>, 4, 0x00AABBCC
        ;левый нижний угол
        mcall 8, <Otstup,buttonW>, <120,buttonH>, 5
        ;правый верхний угол
        mcall 8, <WindowsWidth-Otstup-buttonW,buttonW>, <85,buttonH>, 6
        ;правый нижний угол
        mcall 8, <WindowsWidth-Otstup-buttonW,buttonW>, <120,buttonH>, 7

        ;рисуем тексты для этих 4 кнопок
        mov     ecx, 0xFFFFFF              ;белый цвет текста
        mov     edx, commonButtonText      ;строка текста
        mov     esi, commonButtonTextSize  ;размер текста в символах
        ;координаты текста (x, y)
        deltaCommonTextX = 7
        deltaCommonTextY = 6
        mcall 4, <Otstup+deltaCommonTextX, 85+deltaCommonTextY>
        mcall 4, <Otstup+deltaCommonTextX, 120+deltaCommonTextY>
        mcall 4, <WindowsWidth-Otstup-buttonW+deltaCommonTextX, 85+deltaCommonTextY>
        mcall 4, <WindowsWidth-Otstup-buttonW+deltaCommonTextX, 120+deltaCommonTextY>

        ;delete mode button, if it exist
        mov edx, 0x80000008
        mcall 8
        ;draw button Mode
        buttonModeX = 30 ;433
        buttonModeY = 160 ;50
        buttonModeWidth = 300
        buttonModeHeight = 20
        buttonModeTextYoffset = 3
        buttonModeTextXoffset = 5
        mcall 8, <buttonModeX,buttonModeWidth>, <buttonModeY,buttonModeHeight>, 8, 0x00FFFFFF
        mcall 13, <buttonModeX+2+buttonModeWidth-20, 20-2>, <buttonModeY+1, buttonModeHeight-2>, 0xAABBCC
        ;draw text for button Mode
        mov     ebx, (buttonModeX+buttonModeTextXoffset) * 65536 + (buttonModeY+buttonModeTextYoffset)    ;(x, y)
        mov     ecx, 0x10000000
        call setTextModeMenu
        mov     esi, valueModeMenuSize
        mcall 4
        mov     ebx, (buttonModeX+buttonModeWidth-12) * 65536 + (buttonModeY+buttonModeTextYoffset+3)
        mov     ecx, 0x80FFFFFF
        mov     edx, symbolDownArrow
        mcall 4

        ;кнопка выбора горячей клавиши
        buttonHotKeyX = 350-10
        buttonHotKeyWidth = 90
        mov edx, 0x80000009    ;удаляем
        mcall 8
    cmp byte[mode], 49    ;рисуем выбор клавиши, только если mode > 1
    jbe @f
        ;рисуем новую
        push eax ebx ecx edx esi
        mov ebx, buttonHotKeyX*65536 + buttonHotKeyWidth  ;X + Width
        mov ecx, buttonModeY*65536 + buttonModeHeight ;Y + Height
        mov edx, 0x00000009    ;button id
        mov esi, 0x00FFFFFF    ;color button
        mcall 8
        pop esi edx ecx ebx eax
        ;текст подсказки для горячей клавиши
        mov     ebx, (buttonHotKeyX+buttonModeTextXoffset) * 65536 + (buttonModeY+buttonModeTextYoffset)    ;(x, y)
        mov     ecx, 0x90224466
        call setTextForHotKey
        mov     esi, valueModeMenuSize
        mcall 4
        ;заголовок
        push eax ebx ecx edx edi esi
        mov     ebx, buttonHotKeyX * 65536 + (buttonModeY-11)
        mov ecx, 0x80224466
        mov     edx, hkCaption
        mov     esi, 3
        mcall 4
        pop esi edi edx ecx ebx eax
    @@:

    ;отображаем настройку радиуса действия жеста, только если выбран режим 3
    cmp byte[mode], 51
    jne @f
        ;заголовок
        push eax ebx ecx edx edi esi
        mov     ebx, (buttonHotKeyX + buttonHotKeyWidth + 20) * 65536 + (buttonModeY-11)
        mov ecx, 0x80224466
        mov     edx, radiusCaption
        mov     esi, 3
        mcall 4
        pop esi edi edx ecx ebx eax
        invoke  edit_box_draw, editRadius
    @@:

        call draw_super_text

        LineLength = 62
        mov     ebx, Otstup * 65536 + 32    ; draw info text with function 4 (x, y)

        ;xor ecx, ecx
        ;mov cl, 1 ;byte[mysize]
        ;shl ecx, 24
        ;add ecx, 0x224466
        ;or ecx, 0x00224466  ;80224466
        mov     ecx, 0x10224466

        mov     edx, text
        mov     esi, LineLength
        mov     eax, 4

  .newline:                             ; text from the DATA AREA
        mcall
        add     ebx, 15
        add     edx, LineLength
        cmp     byte[edx], 0
        jne     .newline

        mcall   12, 2  ; End window redraw

        ret

hkSymbol: db 0, 0

setTextForHotKey:
   push eax
   mov al, byte[hkSymbol]
   cmp al, 0
   jne @f
      mov edx, hkHint
      mov dword[hkSize], 5 ;значение по умолчанию SPACE ;shift
      jmp setTextForHotKey.end
@@:
   cmp al, 1
   jne @f
      mov edx, hkAlt
      mov dword[hkSize], 3
      jmp setTextForHotKey.end
@@:
   cmp al, 2
   jne @f
      mov edx, hkShift
      mov dword[hkSize], 5
      jmp setTextForHotKey.end
@@:
   cmp al, 3
   jne @f
      mov edx, hkCtrl
      mov dword[hkSize], 4
      jmp setTextForHotKey.end
@@:
   cmp al, 32
   jne @f
      mov edx, hkSpace
      mov dword[hkSize], 5
      jmp setTextForHotKey.end
@@:
   cmp al, 96  ;'`'
   je  setTextForHotKey.symb
   cmp al, 126 ;'~'
   je  setTextForHotKey.symb
   cmp al, 48 ;'0'
   jb @f
   cmp al, 57 ;'9'
   ja @f
.symb:
      mov edx, hkSymbol
      mov dword[hkSize], 1
      jmp setTextForHotKey.end
@@:
   cmp al, 97  ;'a'
   jb @f
   cmp al, 122 ;'z'
   ja @f
      ;jmp setTextForHotKey.symb
      mov edx, hkSymbol
      mov dword[hkSize], 1
      jmp setTextForHotKey.end
@@:
   cmp al, 65  ;'A'
   jb @f
   cmp al, 90 ;'Z'
   ja @f
      ;jmp setTextForHotKey.symb
      mov edx, hkSymbol
      mov dword[hkSize], 1
      jmp setTextForHotKey.end
@@:
   mov word[hkSymbol], 0     ;если же попало что-то неизвестное, то сотрём его
   mov edx, hkHint
   mov dword[hkSize], 5 ;shift or space -  значение по умолчанию
.end:
   pop eax
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




saveKey:
   cmp word[hkSymbol], 0
   je .default
   cmp byte[hkSymbol], 1
   je .alt
   cmp byte[hkSymbol], 2
   je .shift
   cmp byte[hkSymbol], 3
   je .ctrl
   cmp byte[hkSymbol], 32
   je .space
.other:
   mov esi, hkSymbol
   mov byte[esi+1], 0
   mov dword[hkSize], 1
   jmp .end
.space:
   mov esi, hkSpace
   mov dword[hkSize], 5
   jmp .end
.ctrl:
   mov esi, hkCtrl
   mov dword[hkSize], 4
   jmp .end
.alt:
   mov esi, hkAlt
   mov dword[hkSize], 3
   jmp .end
.default:
.shift:
   mov esi, hkShift
   mov dword[hkSize], 5
.end:
   mov ebx, hkSize ;указатель на dword[hkSize]
   call copyParam
   ret





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
        option_box_mouse,       'option_box_mouse'

commonButtonText db '...', 0  ;текст для кнопок вызова OpenDial
commonButtonTextSize = 3

if lang eq ru  ;если язык сборки русский

text    db  "КЃ≠д®£га†вЃа Hot Angles ЃвҐ•з†•в І† ≠†бваЃ©™г †™в®Ґ≠ле г£ЂЃҐ. "
        db  "Н†бваЃ©в• ѓ†а†ђ•вал ѓЃ§ б•°п ®Ђ® Ѓв™Ђоз®в• нвг Ѓѓж®о.         "
        db  "                                                              ", 0
 hed:
title   db  "КЃ≠д®£га†вЃа гв®Ђ®вл Hot Angles", 0
buttonText db "Па®ђ•≠®вм", 0
buttonTextSize = 9

ch_text db 'В™Ђоз®вм/†™в®Ґ®аЃҐ†вм "Hot Angles"', 0

superText:
  .lu db 'КЃђ†≠§† Ђ•ҐЃ£Ѓ Ґ•ае≠•£Ѓ г£Ђ†: ', 0
  .ld db 'КЃђ†≠§† Ђ•ҐЃ£Ѓ ≠®¶≠•£Ѓ г£Ђ†:  ', 0
  .ru db 'КЃђ†≠§† ѓа†ҐЃ£Ѓ Ґ•ае≠•£Ѓ г£Ђ†:', 0
  .rd db 'КЃђ†≠§† ѓа†ҐЃ£Ѓ ≠®¶≠•£Ѓ г£Ђ†: ', 0
superTextSize = 30

valueModeMenuSize = 32
ModeMenu  db 'Р•¶®ђ а†°Ѓвл:',0
valueModeMenu0  db 'Ѓв™Ђоз•≠Ѓ                       ',0
valueModeMenu1  db '™Ђ†бб®з•б™®© а•¶®ђ              ',0
valueModeMenu2  db 'а•¶®ђ £Ѓапз•© ™Ђ†Ґ®и®           ',0
valueModeMenu3  db 'а•¶®ђ ¶•бв† ѓЃ ™Ђ†Ґ®и• †™в®Ґ†ж®®',0

hkCaption: db 'КЂ†Ґ®и†:',0
;hkHint: db 'Н†¶ђ®в• Ђо°го ™Ђ†Ґ®иг',0
radiusCaption: db 'Р†§®гб:',0

else  ;иначе английский текст

text    db  "This is a Hot Angles Configurator.                            "
        db  "Set the parameters bellow for yourself or disable this option."
        db  "                                                              ", 0
 hed:
title   db  "Hot Angles Configurator", 0
buttonText db "  Apply  ", 0
buttonTextSize = 9

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

hkCaption: db 'Key:',0
;hkHint: db 'press any key',0
radiusCaption: db 'Radius:',0

end if

hkAlt:   db 'Alt',0
hkShift: db 'Shift',0
hkCtrl:  db 'Ctrl',0
hkSpace: db 'Space',0
hkHint: db '?',0
hkSize: rd 1 ;размер строки с именем клавиши

draw_super_text:
   push eax ebx ecx edx edi esi

   mov ecx, 0
   mov ch, byte[mysize]
   shl ecx, 16
   add ecx, 0x224466

   mcall 4, <Otstup,74> ,,superText.lu,superTextSize
   mcall 4, <Otstup,109>,,superText.ld,superTextSize
   mcall 4, <290,74>    ,,superText.ru,superTextSize
   mcall 4, <290,109>   ,,superText.rd,superTextSize

   mov     ebx, Otstup * 65536 + (buttonModeY-11)
   or ecx, 0x80000000
   mov     edx, ModeMenu
   mov     esi, 3
   mcall 4

;   mov     ebx, buttonHotKeyX * 65536 + (buttonModeY-11)
;   or ecx, 0x80000000
;   mov     edx, hkCaption
;   mov     esi, 3
;   mcall 4

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


IntToStr:  ;in esi - строка; eсx - число
   push eax ecx edx esi
   or ecx, ecx
   jnz @f
   mov byte[esi], '0'
   inc esi
   jmp .end
@@:
   mov eax, ecx
   mov ecx, 10
.next:
   xor edx, edx
   div ecx
   add edx, '0'

   mov byte[esi], dl

   inc esi
   or eax, eax
   jz .end
   jmp .next
.end:
   mov byte[esi], 0   ;кладём завершающий ноль
   pop esi edx ecx eax
   call reverseString ;делаем реверс строки
   ret

;вход: esi - указатель на строку
reverseString:
   push eax esi edi

   mov al, byte[esi] ;если строка пустая то выходим реверс делать не надо
   cmp al, 0
   je reverseString.end

   mov edi, esi  ;иначе найдём позицию последнего символа строки и положим её в edi
.go_last_symbol:
   mov al, byte[edi+1]
   cmp al, 0
   je @f

   inc edi
   jmp reverseString.go_last_symbol
@@:
   push ebx
.rev:
   cmp esi, edi
   jae @f

   ;меняем местами символы в цикле
   mov al, byte[edi]
   mov bl, byte[esi]
   mov byte[edi], bl
   mov byte[esi], al

   inc esi
   dec edi
   jmp reverseString.rev
@@:
   pop ebx
.end:
   pop edi esi eax
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
.procinfo               dd pi
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
 
default_dir db '/sys',0 ;директория по умолчанию
communication_area_name: db 'FFFFFFFF_open_dialog',0
open_dialog_name:  db 'opendial',0
communication_area_default_path: db '/sys/File managers/',0
 
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


system_dir0 db '/sys/lib/'
lib0_name db 'proc_lib.obj',0
system_dir1 db '/sys/lib/'
lib1_name db 'kmenu.obj',0

symbolDownArrow db 25,0

;library structures
l_libs_start:
    lib0 l_libs lib0_name, file_name, system_dir0, import_proclib
    lib1 l_libs lib1_name, file_name, system_dir1, import_libkmenu
load_lib_end:


edMaxSize = 510 ;теперь с новым компонентом можно и 511 указать
edMax = 0 ;max, size, pos

align 4
editLU   edit_box 230-buttonW,Otstup+buttonW,85, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0x10000000, edMaxSize, dataBuffer.1, mouse_dd, 0, edMax, edMax  ;ed_focus
editLD   edit_box 230-buttonW,Otstup+buttonW,120, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0x10000000, edMaxSize, dataBuffer.2, mouse_dd, 0, edMax, edMax
editRU   edit_box 230-buttonW,290,85, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0x10000000, edMaxSize, dataBuffer.3, mouse_dd, 0, edMax, edMax
editRD   edit_box 230-buttonW,290,120, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0x10000000, edMaxSize, dataBuffer.4, mouse_dd, 0, edMax, edMax
editRadius edit_box 40,(buttonHotKeyX + buttonHotKeyWidth + 20),buttonModeY, 0xffffff, 0x6a9480, 0, 0xAABBCC, 0x10000000, 4, radiusBuffer.data, mouse_dd, ed_figure_only, edMax, edMax

align 4
import_proclib:
        OpenDialog_Init dd aOpenDialog_Init
        OpenDialog_Start dd aOpenDialog_Start
dd 0,0
        aOpenDialog_Init db 'OpenDialog_init',0
        aOpenDialog_Start db 'OpenDialog_start',0

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

sc  system_colors
mouse_dd  rd 1

sizeStyle: rb 1  ;размер текста и остальных элементов окна
mysize: rb 1     ;rd 1
radiusBuffer:    ;буфер для поля радиуса жеста
 .data: rb 6
 .size: rd 1
radiusBufferEnd:

button_press  rd 0     ;for kmenu

sys_path:      rb 4096
file_name:     rb 4096
plugin_path:   rb 4096
openfile_path: rb 4096
filename_area: rb 256
rb 1024
pi process_information

keyscan: rb 1
keystr:
  .data: rb 4 ;строка со значением сканкода активационной клавиши
  .size: rd 1 ;размер содержимого строки без последнего нуля

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
