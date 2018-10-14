;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Hot Angles ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compile with FASM
; Version 0.3.7: Oct 14, 2018

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

format binary as "" ;"kex"
use32
org 0x0    ; базовый адрес размещения кода, всегда 0x0

UNLOCKd = 0
LOCKd   = 1
SUPERLOCKd = 2

; заголовок
db 'MENUET01'     ;магический идентификатор
dd 0x01           ;версия
dd START_DEBUG    ;адрес точки старта программы
dd I_END          ;адрес конца, по факту размер файла программы
dd 0x100000       ;требуемое кол-во памяти для загрузки программы
dd 0x7fff0        ;начальное значение регистра esp - адрес конца области стэка так как стэк растет в сторону меньших адресов 
dd 0, 0           ;адрес строки параметров и адрес строки пути исполняемого файла


include '../../macros.inc'

START_DEBUG:
   call copyKill         ;простейшая защита от повторного запуска
   mcall  68, 11         ;инициализация кучи
   call loadConfig       ;загружаем конфигурацию приложения

   cmp byte[mode], 0     ;функция отключена - выход
   jne @f
   mcall -1
@@:
   ;режим жеста по горячей клавише?
   cmp byte[mode], 3 ;подписываемся на интересные нам события в зависимости от режима работы
   jne @f
   mov ebx, 00000100000000000000000000100010b    ;если да, то подписываемся на клавиши
   mcall 40
   jmp init03
@@:
   ;режим активации горячей клавишей?
   cmp byte[mode], 2 ;подписываемся на интересные нам события в зависимости от режима работы
   jne @f
   mov ebx, 00000100000000000000000000100010b    ;если да, то подписываемся на клавиши
   mcall 40
   jmp init02
@@:
   ; во всех остальных случаях - классический режим работы
   ;mode=1 классический режим работы - реакция при наведении в угол
   mov byte[mode], 1
   mov ebx, 00000100000000000000000000100000b
   mcall 40
   jmp init01



init02:
   mov dword[backAdr], mode02
   call setHotKey
mode02:
   mcall 10
   cmp eax, 2     ;обработка нажатия клавиши
   je hotkey

   cmp byte[keyState], 0
   je mode02
   cmp   eax, 6   ;обработка перемещений и нажатия мыши
   je    mouse

   jmp mode02


setHotKey:    ;установить горячую клавишу
   push eax ebx ecx edx
   xor ecx, ecx
   mov cl, byte[keyActivate]  ;нажатие горячей клавиши
   mov edx, 0x000
   mcall 66, 4, ecx, edx

   ;mov cl, byte[keyActivate]
   add cl, 128              ;отпускание горячей клавиши
   ;mov edx, 0x000
   mcall 66, 4, ecx, edx
   pop edx ecx ebx eax
   ret


hotkey:
   mcall 2
   cmp eax, 1     ;буфер клавиш пуст?
   je hotkey.end

   cmp al, 2      ;нас интересует только скан код горячей клавиши, иначе выходим
   jne hotkey.end

   cmp ah, byte[keyActivate]   ;если нажата наша горячая клавиша
   jne @f

   ;проверки для mode 3
   cmp byte[state], SUPERLOCKd
   je hotkey.end
   cmp byte[keyState], 1
   je hotkey.end

   push eax ebx
   mcall 37, 0 ;запоминаем текущие(начальные) координаты мыши
   ;mov dword[o.x], eax
   mov word[o.y], ax
   shr eax, 16
   mov word[o.x], ax
   pop ebx eax

   mov byte[keyState], 1
   mov byte[state], UNLOCKd
   jmp hotkey.end
@@:
   push edx
   xor edx, edx
   mov dl, byte[keyActivate]
   add dl, 128
   cmp ah, dl   ;если отпущена наша горячая клавиша
   jne @f       ;hotkey.end

   mov byte[keyState], 0
   mov byte[state], LOCKd
@@:
   pop edx
.end:
   jmp dword[backAdr]


init01:
   mov dword[backAdr], mode01
mode01:
  mov     eax, 10                 ; function 10 : wait until event
  mcall                           ; event type is returned in eax

  cmp   eax, 6           ;обработка перемещений и нажатия мыши
  je    mouse

  jmp mode01


mouse:
   push ebx
   mcall 37, 0   ;получаем текущие координаты мыши
   pop ebx

  cmp eax, 0 ;левый верхний угол
  jne @f
  ; -- здесь вызываем соотв. вызов для левого верхнего угла  (X=0, Y=0)
  push ecx
  mov ecx, 0
  call run_command
  pop ecx

  jmp dword[backAdr] ;finish
@@:
  call get_screen_size ;иначе обновляем размеры экрана

  cmp ax, word[screen.height]    ;Ymouse = Yscreen ?
  je Ytrue
  ;если Y не равны, то нас интересует Y=0 и X=ScreenWidth (правый верхний угол)

  cmp ax, 0
  jne unlockend
  shr eax, 16
  cmp ax, word[screen.width]
  jne unlockend
  ; -- здесь вызываем для правого верхенего угла
  push ecx
  mov ecx, 2
  call run_command
  pop ecx

  jmp dword[backAdr]
Ytrue:
  ;если Y равны, то нас интересует X=0 (левый нижний угол) или X=ScreenWidth (правый нижний)
  shr eax, 16
  cmp ax, 0
  jne @f
  ; -- X=0, Y = Height  (левый нижний угол)
  push ecx
  mov ecx, 1
  call run_command
  pop ecx
  jmp dword[backAdr]
@@:
  cmp ax, word[screen.width]
  jne unlockend
  ; -- (правый нижний угол)
  push ecx
  mov ecx, 3
  call run_command
  pop ecx

  jmp dword[backAdr]

unlockend:
  mov byte[state], UNLOCKd
  jmp dword[backAdr]



;получить размер экрана
get_screen_size:
   push eax
   mcall 14
   ;теперь в eax = [xsize]*65536 + [ysize]
   ;mov dword[screen], eax
   mov word[screen.height], ax
   shr eax, 16
   mov word[screen.width], ax
   pop eax
   ret





;запуск приложения: ecx - активный угол: lu=0, ld=1, ru=2, rd=3
run_command:
   cmp byte[state], UNLOCKd
   jne run_command.end

   push eax ebx ecx

   cmp ecx, 0
   jne @f
   mov eax, apps.path1
   cmp byte[eax], 0 ;если по адресу пусто (или лежал символ '-'), то адрес 0
   je .set0
   mov dword[struct70.path_adr], eax
   mov eax, apps.param1
   mov dword[struct70.param_adr], eax
   jmp .end_set_path
@@:
   cmp ecx, 1
   jne @f
   mov eax, apps.path2
   cmp byte[eax], 0 ;если по адресу пусто (или лежал символ '-'), то адрес 0
   je .set0
   mov dword[struct70.path_adr], eax
   mov eax, apps.param2
   mov dword[struct70.param_adr], eax
   jmp .end_set_path
@@:
   cmp ecx, 2
   jne @f
   mov eax, apps.path3
   cmp byte[eax], 0 ;если по адресу пусто (или лежал символ '-'), то адрес 0
   je .set0
   mov dword[struct70.path_adr], eax
   mov eax, apps.param3
   mov dword[struct70.param_adr], eax
   jmp .end_set_path
@@:
   cmp ecx, 3
   jne .set0 ;.end_set_path
   mov eax, apps.path4
   cmp byte[eax], 0 ;если по адресу пусто (или лежал символ '-'), то адрес 0
   je .set0
   mov dword[struct70.path_adr], eax
   mov eax, apps.param4
   mov dword[struct70.param_adr], eax
   jmp .end_set_path
.set0:
   ;mov dword[struct70.path_adr], 0
   ;mov dword[struct70.param_adr], 0
   jmp @f
.end_set_path:
   mcall 70, struct70
@@:
   mov byte[state], LOCKd
   ;mov byte[keyState], 0
   pop ecx ebx eax
.end:
   ret

struct70: ;Формат информационной структуры
  .func       dd 7 ; номер подфункции
  .mask       dd 0
  .param_adr  dd 0 ; указатель на ASCIIZ-строку с параметрами
  .other      dd 0, 0
  .path       db 0
  .path_adr   dd 0 ; указатель на строку с путём

;имя конфигурационного файла
fileName: db 'SETTINGS/HOTANGLES.CFG', 0


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

;пропускаем переносы в начале строки до первого адекватного символа
gotoStartString:
   cmp eax, edx  ;если файл закончился, то
   ja @f         ;выходим
   cmp byte[eax], 10
   je .next
   cmp byte[eax], 0
   je .next
   cmp byte[eax], 13
   je .next
   jmp @f ;если это другой символ, то выходим
.next:   ;переходим на следующий символ
   inc eax
   jmp gotoStartString
@@:
   ret


sep: db '|' ;символ разделитель строки пути + параметры запуска
readPath:
   push ebx esi edi
   mov bh, 0 ; 0 -  знак того, что параметры ещё не встречались
   mov byte[esi], 0   ;проинициализируем буферы-строки пустотой (на всякий случай)
   mov byte[edi], 0
@@:
   cmp eax, edx  ;если файл закончился, то
   ja @f         ;выходим
   mov bl, byte[eax]
   ;если очередной символ не конец строки
   cmp bl, 10
   je @f  ;иначе выход
   cmp bl, 0
   je @f
   cmp bl, 13
   je @f
   ;и не начало параметров
   cmp bl, byte[sep]
   jne .read
   ;если sep уже встречали ранее, то
   cmp bh, 0
   jne .read ;просто продолжаем считывать параметры дальше  (в них может быть что угодно)
   ;иначе кладём завершающий 0 и переходим к буферу с параметрами
   mov byte[esi], 0
   call trim ;ещё не забудем убрать лишние табуляции и пробелы на конце нашей строки
   mov esi, edi
   mov bh, 1
   inc eax
   jmp @b
.read: ;то записываем символ в наш буфер и переходим на следующий символ
   mov byte[esi], bl
   inc esi
   inc eax
   jmp @b
@@:
   mov byte[esi], 0 ;кладём завершающий ноль
   pop edi esi ebx
   ret

trim:  ;esi - последний нулевой символ строки
   ;push esi
   mov byte[esi], 0
   dec esi
   cmp byte[esi], 9   ;tab
   je trim
   cmp byte[esi], 32  ;space
   je trim
   inc esi
   ;pop esi
   ret


cfgFileAdr: dd 0     ;адрес конфигурационного файла в памяти

loadConfig:
   push eax ebx ecx edx
   mcall 68, 27, fileName   ;загружаем конфигурационный файл в ОЗУ
   mov dword[cfgFileAdr], eax
   cmp eax, 0
   je loadConfig.exit       ;если файла конфигурации нет, то завершаем работу приложения
   ;иначе данные загружены в ОЗУ, размер в edx
   cmp edx, 0               ;если файл пуст, в нём нет данных, то завершаем работу
   je loadConfig.exit

   add edx, eax             ;иначе кладём в edx - адрес конца файла

   mov byte[mode], 0
   cmp byte[eax], '0'       ;если параметр активности приложения имеет статус: не активно
   je loadConfig.exit       ;то завершаем работу приложения
   cmp byte[eax], 110 ;'n'
   je loadConfig.exit

   ;иначе выставляем режим
   cmp byte[eax], 121 ;'y'  ;классический режим
   jne @f
   mov byte[mode], 1
   jmp loadConfig.modeEnd
@@:
   cmp byte[eax], '1'       ;классический режим
   jne @f
   mov byte[mode], 1
   jmp loadConfig.modeEnd
@@:
   cmp byte[eax], '2'       ;режим активации по горячей клавише
   jne @f
   mov byte[mode], 2
   jmp loadConfig.modeEnd
@@:
   cmp byte[eax], '3'       ;режим жеста при зажатой клавише
   jne @f
   mov byte[mode], 3
   jmp loadConfig.modeEnd
@@:
   mov byte[mode], 0        ;во всех остальных случаях - отключено
   jmp loadConfig.exit
.modeEnd:

   ;идём до конца этой строки
   call gotoEndString
   ;находим начало следующей строки
   call gotoStartString

   push esi edi

   ;считываем путь для левого верхнего угла
   mov esi, apps.path1
   mov edi, apps.param1
   call readPath
   call gotoStartString  ;находим начало следующей строки

   ;считываем путь для левого нижнего угла
   mov esi, apps.path2
   mov edi, apps.param2
   call readPath
   call gotoStartString

   ;считываем путь для правого верхнего угла
   mov esi, apps.path3
   mov edi, apps.param3
   call readPath
   call gotoStartString

   ;считываем путь для правого нижнего угла
   mov esi, apps.path4
   mov edi, apps.param4
   call readPath
   call gotoStartString

   ;проверим строки на '-'
   call checkPathAdr

   pop edi esi

   ;пропускаем строку с именем клавиши
   call gotoEndString
   call gotoStartString

   ;считываем сканкод активационной клавиши
   push esi edi ecx
   mov esi, keystr
   mov edi, keystr
   call readPath
   ;преобразуем строку в число
   mov esi, keystr
   call strToInteger
   mov byte[keyActivate], cl
   pop ecx edi esi
   call gotoStartString

   ;считываем радиус
   push esi edi ecx
   mov esi, radstr
   mov edi, radstr
   call readPath
   ;преобразуем строку в число
   mov esi, radstr
   call strToInteger
   ;теперь ecx - значение радиуса
   ;!!! добавить проверку, чтобы число не было больше области экрана
   ;нас интересует квадрат числа
   push eax edx
   mov eax, ecx ;dword[radius2]
   mul ecx
   mov dword[radius2], eax
   pop edx eax

   pop ecx edi esi

   ;считывать размер элементов интерфейса нам не нужно (пока что этот параметр только для конфига)
   jmp .end
.exit:
   pop edx ecx ebx eax
   mcall -1 ;закрыть эту программу
.end:
   mov ecx, dword[cfgFileAdr]    ;если файл был загружен, то
   cmp ecx, 0
   je @f
   mcall 68, 13, ecx  ;выгружаем файл из памяти
@@:
   pop edx ecx ebx eax
   ret


checkPathAdr:
   push eax ebx
   xor eax, eax

   mov ebx, apps.path1
   cmp byte[ebx], '-'
   jne @f
   ;cmp byte[ebx+1], 0
   ;jne @f
   mov byte[ebx], 0
@@:
   mov ebx, apps.path2
   cmp byte[ebx], '-'
   jne @f
   ;cmp byte[ebx+1], 0
   ;jne @f
   mov byte[ebx], 0
@@:
   mov ebx, apps.path3
   cmp byte[ebx], '-'
   jne @f
   ;cmp byte[ebx+1], 0
   ;jne @f
   mov byte[ebx], 0
@@:
   mov ebx, apps.path4
   cmp byte[ebx], '-'
   jne @f
   ;cmp byte[ebx+1], 0
   ;jne @f
   mov byte[ebx], 0
@@:
   pop ebx eax
   ret

;преобразование строки в целое число
strToInteger: ;вход: esi - указатель на строку с десятичной записью числа, выход: eсx - число
   push eax ebx edx edi
   xor eax, eax  ;обнуляем eax
   xor ecx, ecx
   mov edi, esi
.get_last_symbol:
   ;читаем очередной символ
   mov al, byte[edi]

   ;если это ноль, то строка закончилась
   cmp al, 0
   je @f

   ;иначе переходим на следующий символ
   inc edi
   jmp .get_last_symbol
@@:   ;дошли до последнего символа и кладём адрес на него в edi
   dec edi

   cmp edi, esi    ;адрес последнего символа должен быть дальше адреса первого символа
   jae @f
   jmp .error  ;иначе ошибка
@@:
   ;теперь в edi последний символ, а esi указывает на начало строки (на первй символ строки)
   mov cl, byte[edi]
   sub cl, 48 ;0x30 ;'0'
   dec edi         ;переходим на след символ от конца
   mov ebx, 10     ;множитель степени
.tranform_cicle:
   cmp edi, esi        ;если больше нет символов строке, то число преобразовали - выходим
   jb .end

   mov eax, 0
   mov al, byte[edi]
   sub al, 48 ;'0'
   cmp al, 9 ;0x39           ;если число больше 9, то видимо в строке было что-то не так
   ja .error
   cmp al, 0 ;0x30           ;если число меньше 0, то видимо в строке было что-то не так
   jb .error
   mov edx, 0
   mul ebx

   add ecx, eax

   dec edi
   mov edx, 0
   mov eax, 10
   mul ebx
   mov ebx, eax
   jmp .tranform_cicle
.error:
   mov ecx, 0
.end:
   pop edi edx ebx eax
   ret


;Вынести код ниже в отдельный общий модуль
selfName db '@HOTANGLES',0
selfNameSize = 10 ;до 11 byte
;
compareBytes:
   push edi esi ecx ebx
   xor eax, eax
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


;MODE 3
init03:
   ;mov dword[radius2], 100; 10*10
   mov dword[backAdr], mode03
   call setHotKey
mode03:
   mcall 10
   cmp eax, 2     ;обработка нажатия клавиши
   je hotkey

   ;cmp byte[keyState], 0
   ;je mode03
   cmp   eax, 6   ;обработка перемещений и нажатия мыши
   je    mouse03

   jmp mode03

mouse03:
   cmp byte[keyState], 0     ;если клавиша не зажата, то обновим начальные координаты
   je mouse03.updateCoord

   cmp byte[state], LOCKd    ;если угол заблокирован, то обновим начальные координаты
   je mouse03.updateCoord

   cmp byte[state], SUPERLOCKd  ;если действие было выполнено и получили блокировку, то ждём отпускания клавиши активации
   je @f

   call checkPointInCircle   ;иначе если преодолели радиус активации, то вызываем действие нужного угла
   jmp @f
.updateCoord:
   push eax ebx
   mcall 37, 0 ;запоминаем текущие(начальные) координаты мыши
   ;mov dword[o.x], eax
   mov word[o.y], ax
   shr eax, 16
   mov word[o.x], ax
   pop ebx eax
@@:
   jmp dword[backAdr]


radius2 dd 0  ;квадрат радиуса для mode 3
o:            ;координаты начальной и текущей точки
 .x:   dw 0
 .y:   dw 0
 .cx:  dw 0
 .cy:  dw 0

;проверка выхода за пределы круга
; sqrt( (x2-x1)^2 + (y2-y1)^2 ) <= radius
; (x2-x1)^2 + (y2-y1)^2 <= radius^2
checkPointInCircle:
   push eax ebx ecx edx
   mcall 37, 0 ;получаем текущие координаты мыши
   ;mov dword[o.cx], eax
   mov word[o.cy], ax
   shr eax, 16
   mov word[o.cx], ax

   xor eax, eax
   xor edx, edx
   mov ax, word[o.x]
   mov dx, word[o.cx]
   cmp dx, ax
   jae @f
   sub ax, dx
   mov dx, ax
   jmp .xend
@@:
   sub dx, ax
.xend:
   mov eax, edx
   mul dx

   mov ebx, eax

   xor eax, eax
   xor edx, edx
   mov ax, word[o.y]
   mov dx, word[o.cy]
   cmp dx, ax
   jae @f
   sub ax, dx
   mov dx, ax
   jmp .yend
@@:
   sub dx, ax
.yend:
   mov eax, edx
   mul dx

   add ebx, eax

   cmp ebx, dword[radius2]
   jbe @f
   call activateCorner ;иначе активируем нужный угол
 @@:
   pop edx ecx ebx eax
   ret

;выяснить в сторону какого угла ушли и активировать его
activateCorner:
   push eax ecx
   xor eax, eax

   mov ax, word[o.cx]
   cmp ax, word[o.x]
   je activateCorner.end       ;если координата равна, то выходим (такого быть не должно)
   ja activateCorner.right

   ;иначе левая сторона
   mov ax, word[o.cy]
   cmp ax, word[o.y]
   je activateCorner.end       ;если координата равна, то выходим (такого быть не должно)
   jb activateCorner.leftup

   ;иначе левый нижний угол
   mov ecx, 1
   call run_command
   mov byte[keyState], 0
   mov byte[state], SUPERLOCKd
   jmp activateCorner.end

.leftup:
   ;левый верхний угол
   mov ecx, 0
   call run_command
   mov byte[keyState], 0
   mov byte[state], SUPERLOCKd
   jmp activateCorner.end

.right: ;правая сторона экрана
   mov ax, word[o.cy]
   cmp ax, word[o.y]
   je activateCorner.end       ;если координата равна, то выходим (такого быть не должно)
   jb activateCorner.rightup

   ;иначе правый нижний угол
   mov ecx, 3
   call run_command
   mov byte[keyState], 0
   mov byte[state], SUPERLOCKd
   jmp activateCorner.end

.rightup:
   ;правый верхний угол
   mov ecx, 2
   call run_command
   mov byte[keyState], 0
   mov byte[state], SUPERLOCKd
   jmp activateCorner.end

.end:
   pop ecx eax
   ret



keyActivate db 41 ;16 ;'Q' ;клавиша активации
keyState    db 0  ;0-клавиша не активна, 1 - клавиша активна

backAdr rd 1      ;адрес для возврата из проверки мыши
slotMax rd 1
selfPID rd 1
buf rb 1024

;структурка данных для хранения параметров экрана
screen:
  .width  rw 1
  .height rw 1

state: rb 1  ;состояние: 0=unlock, 1=lock, 2=superlock for mode 3
mode:  rb 1  ;режим работы приложения: 0=не работает, 1=классический, 2=по горячей клавише


keystr: rb 4 ;строка со значением сканкода активационной клавиши
radstr: rb 5 ;строка со значением радиуса
apps:
 .path1: rb 512
 .path2: rb 512
 .path3: rb 512
 .path4: rb 512
 .param1: rb 512
 .param2: rb 512
 .param3: rb 512
 .param4: rb 512
appsEnd:


I_END:
