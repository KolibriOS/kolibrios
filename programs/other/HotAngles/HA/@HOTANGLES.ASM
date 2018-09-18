;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Hot Angles ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Compile with FASM
; Version 0.1: Sep 18, 2018

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
LOCKd = 1

; заголовок
db 'MENUET01'     ;магический идентификатор
dd 0x01           ;версия
dd START_DEBUG          ;адрес точки старта программы
dd I_END          ;адрес конца, по факту размер файла программы
dd 0x100000       ;требуемое кол-во памяти для загрузки программы
dd 0x7fff0        ;начальное значение регистра esp - адрес конца области стэка так как стэк растет в сторону меньших адресов 
dd 0, 0           ;адрес строки параметров и адрес строки пути исполняемого файла


include '../../../macros.inc'

START_DEBUG:
   call copyKill         ;простейшая защита от повторного запуска
   mcall  68, 11         ;инициализация кучи
   call loadConfig       ;загружаем конфигурацию приложения
   mov ebx, 00000100000000000000000000100000b ;подписываемся на интересные нам события
   mcall 40

START:
  mov     eax, 10                 ; function 10 : wait until event
  mcall                           ; event type is returned in eax

  cmp   eax, 6           ;обработка перемещений и нажатия мыши
  je    mouse

  jmp START

mouse:
  ;основной цикл программы
  call get_mouse_pos ;получаем текущие координаты мыши

  cmp eax, 0 ;левый верхний угол
  jne @f
  ; -- здесь вызываем соотв. вызов для левого верхнего угла  (X=0, Y=0)
  push ecx
  mov ecx, 0
  call run_command
  pop ecx

  jmp START ;finish
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

  jmp START
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
  jmp START
@@:
  cmp ax, word[screen.width]
  jne unlockend
  ; -- (правый нижний угол)
  push ecx
  mov ecx, 3
  call run_command
  pop ecx

  jmp START

unlockend:
  mov byte[state], UNLOCKd
  jmp START


;структурка данных для хранения параметров экрана
screen:
   .width  dw 0
   .height dw 0
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


;получить позицию мыши
get_mouse_pos:
   ;push eax
   push ebx
   mcall 37, 0
   pop ebx
   ;pop eax
   ret


;запуск приложения: ecx - активный угол: lu=0, ld=1, ru=2, rd=3
run_command:
   cmp byte[state], UNLOCKd
   jne run_command.end

   push eax
   push ebx
   push ecx


   cmp ecx, 0
   jne @f
   mov eax, dword[newData.lu] ;testData.lu
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

;этот код заменить - если адрес 0, то ничего не делать
   cmp dword[struct70.path_adr], 0
   jne .next
   mov eax, testData.ld    ;
   mov dword[struct70.path_adr], eax
.next:
;конец кода для замены

   mcall 70, struct70
   mov byte[state], LOCKd

   pop ecx
   pop ebx
   pop eax
.end:
   ret

struct70: ;Формат информационной структуры
  .func       dd 7 ; номер подфункции
  .mask       dd 0
  .param_adr  dd 0 ; указатель на ASCIIZ-строку с параметрами
  .other      dd 0, 0
  .path       db 0 ; "/rd/1/TINYPAD",0 ; путь
  .path_adr   dd 0 ;testData.lu ;0


state: db 0 ; 0=unlock, 1=lock

testData:
  .lu db '/rd/1/SHELL', 0
  .ld db '/rd/1/RUN', 0
  .ru db "/rd/1/TINYPAD", 0
  .rd db '/rd/1/File Managers/EOLITE', 0

newData:  ;табличка адресов командных строк
  .lu dd 0
  .ld dd 0
  .ru dd 0
  .rd dd 0
newDataEnd:

;имя конфигурационного файла
fileName: db 'SETTINGS/HOTANGLES.CFG', 0  ;'ha.cfg', 0

loadConfig:
   push eax
   push ebx
   push ecx
   push edx
   mcall 68, 27, fileName   ;загружаем конфигурационный файл в ОЗУ
   cmp eax, 0
   je loadConfig.exit       ;если файла конфигурации нет, то завершаем работу приложения
   ;иначе данные загружены в ОЗУ, размер в edx
   cmp edx, 0               ;если файл пуст, в нём нет данных, то завершаем работу
   je loadConfig.exit

   add edx, eax             ;иначе кладём в edx - адрес конца файла

   cmp byte[eax], 121 ;'y'  ;если параметр активности приложения имеет статус: не активно
   jne loadConfig.exit      ;то завершаем работу приложения

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

   ; символ отличен от переноса строки и возврата каретки - запоминаем его
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
   pop edx
   pop ecx
   pop ebx
   pop eax
   mcall -1 ;закрыть эту программу
.end:
   pop edx
   pop ecx
   pop ebx
   pop eax
   ret




;Вынести код ниже в отдельный общий модуль
selfName db '@HOTANGLES',0
selfNameSize = 10 ;до 11 byte
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



I_END:
