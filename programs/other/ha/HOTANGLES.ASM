;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Hot Angles ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compile with FASM
; Version 0.3.5: Oct 5, 2018

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
   ;mov ecx, 0  ;строки для отладки и теста
   ;call run_command
   mov byte[keyState], 0
   mov byte[state], LOCKd
@@:
   pop edx
.end:
   jmp mode02


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

;   cmp byte[mode], 2     ;если режим второй
;   jne @f

   push eax ebx ecx

   cmp ecx, 0
   jne @f
   mov eax, dword[newData.lu]
   mov dword[struct70.path_adr], eax
   jmp .end_set_path
@@:
   cmp ecx, 1
   jne @f
   mov eax, dword[newData.ld]
   mov dword[struct70.path_adr], eax
   jmp .end_set_path
@@:
   cmp ecx, 2
   jne @f
   mov eax, dword[newData.ru]
   mov dword[struct70.path_adr], eax
   jmp .end_set_path
@@:
   cmp ecx, 3
   jne .end_set_path
   mov eax, dword[newData.rd]
   mov dword[struct70.path_adr], eax
   jmp .end_set_path
.end_set_path:

   ;параметры
   ;mov dword[struct.adr],

;если адрес 0, то ничего не делать
   cmp dword[struct70.path_adr], 0
   jne .next
   ;mov eax, testData.ld
   ;mov dword[struct70.path_adr], eax
   jmp @f
.next:
;конец кода для замены

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
  .path       db 0 ; "/rd/1/TINYPAD",0 ; путь
  .path_adr   dd 0 ;testData.lu

;имя конфигурационного файла
fileName: db 'SETTINGS/HOTANGLES.CFG', 0

loadConfig:
   push eax ebx ecx edx
   mcall 68, 27, fileName   ;загружаем конфигурационный файл в ОЗУ
   cmp eax, 0
   je loadConfig.exit       ;если файла конфигурации нет, то завершаем работу приложения
   ;иначе данные загружены в ОЗУ, размер в edx
   cmp edx, 0               ;если файл пуст, в нём нет данных, то завершаем работу
   je loadConfig.exit

   add edx, eax             ;иначе кладём в edx - адрес конца файла

   mov byte[mode], 0
   cmp byte[eax], '0' ;121 ;'y'  ;если параметр активности приложения имеет статус: не активно
   je loadConfig.exit      ;то завершаем работу приложения
   cmp byte[eax], 110 ;'n'
   je loadConfig.exit

   ;иначе выставляем режим
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
   mov byte[mode], 1        ;во всех остальных случаях - классический режим
.modeEnd:

   push edi
   push esi
   ; сохраняем в edi указатель на начало таблицы адресов наших команд
   mov edi, newData
   ; сохраняем адреса строк и добавляем 0 в конце
   mov esi, eax
.block:
   inc esi
   cmp byte[esi], 10         ;если очередной код символа 10 или 13, то пропускаем символы
   je loadConfig.propusk     ;до первого отличного от них
   cmp byte[esi], 13
   je loadConfig.propusk

   ; символ отличен от переноса строки и возврата каретки - если это на знак минус, то запоминаем его
   mov dword[edi], esi
   add edi, 4

   ;идём до конца этой строки: пока не встретим очередной 10, 13, 0 или file end
.while:
   inc esi
   cmp esi, edx           ;тут будет проблема - тк файл закончился, а нуля не было !!! исправить
   jae loadConfig.fileend
   cmp byte[esi], 10
   je loadConfig.ura
   cmp byte[esi], 0
   je loadConfig.ura
   cmp byte[esi], 13
   jne loadConfig.while
.ura:
   mov byte[esi], 0

   ;cmp byte[esi-1], '-'
   ;jne @f
   push esi  ;если это был знак минус, то меняем адрес на ноль
   mov esi, dword[edi-4]
   cmp byte[esi], '-'
   jne @f
   cmp byte[esi+1], 0
   jne @f
   mov dword[edi-4], 0
@@:
   pop esi

   cmp edi, newDataEnd ;newData.end      ;если вся таблица адресов заполнена, то выходим из цикла
   jb loadConfig.block
.fileend:

   pop esi
   pop edi

   jmp loadConfig.end

.propusk:
   mov byte[esi], 0
   jmp loadConfig.block
.exit:
   pop edx ecx ebx eax
   mcall -1 ;закрыть эту программу
.end:
   pop edx ecx ebx eax
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


keyActivate db 41 ;16 ;'Q' ;клавиша активации
keyState db 0 ;0-клавиша не активна, 1 - клавиша активна

backAdr rd 1 ;dd 0 ; адрес для возврата из проверки мыши

;slotMax dd 0
;selfPID dd 0
;buf db 1024 dup(0)
slotMax rd 1
selfPID rd 1
buf rb 1024

;структурка данных для хранения параметров экрана
screen:
  .width  rw 1 ;dw 0
  .height rw 1 ;dw 0

state: rb 1  ;состояние: 0=unlock, 1=lock
mode:  rb 1  ;режим работы приложения: 0=не работает, 1=классический, 2=по горячей клавише

newData:  ;табличка адресов командных строк
  .lu rd 1
  .ld rd 1
  .ru rd 1
  .rd rd 1
newDataEnd:

I_END:
