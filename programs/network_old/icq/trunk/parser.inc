;
; Структура для функции 70
;

struc sinfo
{
	.subfnc_name     dd 0
	.pos_in_file     dd 0
	.reserved        dd 0
	.bytes_to_read   dd 0
	.pbuffer	 dd 0
	.null            db 0
	.pname           dd 0
}




;
;  таблицы значений 
;
;  +----+-------------+-----------------+
;  | in | Variable    | Variable        |
;  | de | name        | string          |
;  | x  |             |                 |
;  |    |             |                 |
;  +----+-------------+-----------------+
;  |    |             |                 |
;  |    |             |                 |
;  | 1  | UIN         | 'XXXXX..XX',0   |
;  |    |             |                 |
;  +----+-------------+-----------------+
;  |                  |                 |
;
;  загрузка int пока не реализована
;
;  +----+-------------+-----------------+
;  | in | Variable    | Variable        |
;  | de | name        | int             |
;  | x  |             |                 |
;  |    |             |                 |
;  +----+-------------+-----------------+
;  |    |             |                 |
;  |    |             |                 |
;  | 1  |    BUFFSIZE |  XXXXXXXX       |
;  |    |             |                 |
;  +----+-------------+-----------------+
;  |                  |                 |
;
;


TABLE_SIZE	equ 16
VNAME_LEN	equ 8
VAR_LEN         equ 16

;
; Описание таблицы значений

virtual at 0
  vartable:
	.uin db VAR_LEN dup ?
	.pass db VAR_LEN dup ?
	.icqip db VAR_LEN dup ?
 	


end virtual


;
; Коды ошибок файловой системы
;

FIO_SUCCESS		equ 0
FIO_UNSUPPORTED		equ 2
FIO_UNKNOWNFS		equ 3
FIO_FILENOTFOUND	equ 5
FIO_EOF			equ 6
FIO_BADPOINTER		equ 7
FIO_DISKFULL		equ 8
FIO_FATDAMAGED		equ 9
FIO_DENIED		equ 10
FIO_ERRORDEVICE		equ 11


IOBUFF_SIZE	equ 128




;
; макрос для поиска элемента в строке 
; возвращает в eax номер элемента или -1 если не найден
macro findchar string, len, char
{
   local ..fc_endstr, ..fc_end

   push ebx
   push ecx
   push edi


   mov edi, string
   mov ecx, len
   mov ebx, ecx
   cld
   mov al, char
   repne scasb
   jcxz ..fc_endstr

   sub ebx, ecx  ; Номер =
   mov eax, ebx
   jmp ..fc_end

 ..fc_endstr:
   mov eax, -1


 ..fc_end:
   pop edi
   pop ecx
   pop ebx

}

;
; Макрос для поиска элемента строки, отличающегося от 
; заданного

macro findother string, len, char
{
   local ..fc_endstr, ..fc_end

   push ebx
   push ecx
   push edi


   mov edi, string
   mov ecx, len
   mov ebx, ecx
   cld
   mov al, char
   repe scasb    ; Если символ не char - выходим
   jcxz ..fc_endstr   ; строка из char 

   sub ebx, ecx   ; в ebx - номер элемента отличного от char
   mov eax, ebx
   jmp ..fc_end

 ..fc_endstr:
   mov eax, -1


 ..fc_end:
   pop edi
   pop ecx
   pop ebx
}

;
; Макрос для копирования строк
;
macro mstrcpy from, to, leng
{

   push ecx
   push esi
   push edi

   mov ecx, leng
   mov esi, from
   mov edi, to
   cld
   rep movsb

   pop edi
   pop esi
   pop ecx

}


;
; Инициализирует таблицы 
;

;  inittables:
;
;
;   mstrcpy name1, nvtable, VNAME_LEN
;   mstrcpy name2, (nvtable + NAME_LEN), VNAME_LEN
;   mstrcpy name3, (nvtable + NAME_LEN * 2), VNAME_LEN
;
;
;
;  ret   


;                 
; заполняет таблицы значениями
; IN eax - ASCIIZ имя файла
; OUT eax - результат чтения
; если результат -1, формат файла неправильный
;
  parseconf:
   push edi
   push esi
   ;push eax
   push ebx
   push ecx
   push edx

   mov [strnum], dword 0
   
   ;
   ; Прочитать построчно конфиг
   ; если строка начинается с ;, #  - комментарий
   ; Формат UIN="1234567890"
   ; PASS="******" и т.д.

   ;
   ; Сброс смещения
   mov [shift], dword 0

   mov esi, eax
   
 pc_still:

   mov edx, esi
   mov ecx, IOBUFF_SIZE
   mov ebx, iobuff

   call getstr

   inc [strnum]

   push eax

   ;
   ;Проверка полученной строки
   ;
   movzx eax, byte [iobuff]

   test eax, eax
   jz pc_next

   cmp al, '#'
   jz pc_next

   cmp al, ';'
   jz pc_next

   ;
   ; Найти имя переменной 
   ;
   findother iobuff, ebx, ' '
   cmp eax, -1
   jz pc_next
    
   mov [stnpos], eax  ;начало имени

   ;
   ; найти =
   ;
   mov ecx, ebx   ; Искать от найденного символа
   sub ecx, eax   ;

   mov edi, iobuff
   add edi, eax

   findchar edi, ecx, '='

   cmp eax, -1
   jz pc_badformat

   mov edi, [stnpos]
   add eax, edi   ; в eax - смещение от начала строки
   mov [eqpos], eax

   mov ecx, ebx
   sub ecx, eax

   ;
   ; проверить "
   ;
   mov dl, [iobuff + eax]
   cmp dl, '"'
   jnz pc_badformat
   ;
   ; найти закрывающую "
   ;
   mov edi, iobuff
   add edi, eax

   inc edi

   findchar edi, ecx, '"'

   cmp eax, -1
   jz pc_badformat

   inc eax

   mov edx, [eqpos]
   add eax, edx
   mov [edvpos], eax

   ;
   ; Установить значение
   ;
   ; Добавить завершающие 0

   mov eax, [stnpos]
   dec eax


   mov ebx, [eqpos]
   mov ecx, ebx
   dec ecx            ; количество символов до =
   inc ebx            ; Пропустить "

   mov [iobuff + ecx], byte 0

   mov edx, [edvpos]
   dec edx

   mov [iobuff + edx], byte 0

   lea eax, [iobuff + eax]
   lea ebx, [iobuff + ebx]
   call setavar

   jmp pc_next




 pc_badformat:
   pop eax

   mov ebx, [strnum]
   jmp pc_err


 pc_next:
   pop eax

   cmp eax, FIO_EOF
   jz pc_eof
   cmp eax, FIO_SUCCESS
   jnz pc_err

 jmp pc_still



 pc_eof:
 pc_err:
   pop edx
   pop ecx
   pop ebx
   ;pop eax
   pop esi
   pop edi
  ret

; Поиск в таблице переменной и установка её значения
; IN eax - название переменной указатель на ASCIIZ
; IN ebx - значение переменной указатель на ASCIIZ
; OUT eax -результат 0 = OK, -1 = нет в таблице переменных
; OUT заполняет глобальную таблицу
  setavar:
   ;push ebx
   push ecx
   push edx
   push esi
   push edi
   push ebx

   ;
   ; Определить длину строки - название переменной
   ;
   mov edi, eax
   push eax

   mov ecx, VNAME_LEN

   xor eax, eax ;Ищем \0
   cld
   repne scasb

   mov eax, VNAME_LEN
   sub eax, ecx       ; В ecx - остаток до максимального размера строки
   mov ebx, eax


   pop eax            
   ;
   ; Искать в таблице подходящее имя
   ;
   xor edx, edx         ;index

 sv_next:
   mov ecx, ebx
   push eax
   mov esi, eax
   mov edi, nvtable
   mov eax, edx
   imul eax, VNAME_LEN   ;offset
   add edi, eax
   pop eax
   cld
  
   repe cmpsb
   jz sv_match

 sv_inc:
   inc edx
   cmp edx, TABLE_SIZE
   jae  sv_fail
   jmp sv_next

 sv_match:
   cmp ebx, VNAME_LEN    ;В ebx - длина исходной строчки
   jz sv_match2

   push eax
   mov edi, nvtable
   mov eax, edx
   imul eax, VNAME_LEN   ;offset
   add edi, eax
   pop eax

   cmp [edi + ebx], byte 0 ; Если имя в таблице короче максимального,
   jnz sv_inc                  ; заканчивается 0

 sv_match2:
   pop edi                    ; 
   push edi
   ;
   ; Определять длину строки - переменная
   ;
   xor eax, eax
   mov ecx, VAR_LEN
   cld
   repne scasb

   mov eax, VAR_LEN
   sub eax, ecx
   mov ecx, eax
   mov ebx, eax

   ;
   ; Копировать переменную в таблицу
   ;

   pop esi
   push esi

   mov eax, VAR_LEN
   imul eax, edx
   mov edi, vtable
   add edi, eax
   cld
   rep movsb

   ;
   ; Если строка короче поля в таблице, записать в конец 0
   ;
   cmp ebx, VAR_LEN
   jz sv_end
   mov [edi + ebx], byte 0



 sv_end:
   xor eax, eax
   jmp sv_fin

 sv_fail:
   mov eax, -1

 sv_fin:

   pop ebx
   pop edi
   pop esi
   pop edx
   pop ecx
   ;pop ebx
  ret





;
; Чтение ASCIIZ строки из файла
;  IN ebx - указатель на буфер
;  ecx - размер буфера
;  edx - указатель на строку имя файла
;  OUT ebx - длина строки
;  eax - результат чтения

  getstr:
   ;push eax
   ;push ebx
   push ecx
   push edx
   push esi

   ;xor edx, edx
   xor esi, esi

 gs_read:
   ;
   ;  Заполняем структуру
   ;
   mov [finfo.subfnc_name], 0
   mov eax, [shift]
   mov [finfo.pos_in_file], eax
   mov [finfo.bytes_to_read], ecx
   mov [finfo.pbuffer], ebx
   mov [finfo.pname], edx

   push ebx

   ;
   ; Читем
   ;
   ;mov eax, 70
   ;mov ebx, finfo
   ;int 40h
   mcall 70, finfo

   mov ecx, ebx    ; В ebx количество прочтенных байт

   pop ebx

   ;
   ; Проверить результат чтения - если не EOF и 0, 
   ; выходим
   cmp eax, FIO_EOF
   jz gs_loop
   cmp eax, 0
   jz gs_loop

   jmp gs_ok


   ;
   ; Обработка полученного блока
   ;
 gs_loop:
   mov dl, [ebx + esi]
   cmp dl, 0Ah          ;cr
   jz gs_cr
   inc esi
   cmp esi, ecx
   jnb gs_err 
   jmp gs_loop

 gs_err:
   ;
   ; В буфере нет символа переноса строки, т.е. строка слишком длинная
   ; Отбрасываем все до ближайшего символа переноса строки
   ; если конец файла  - выходим
   cmp eax, FIO_EOF
   jz gs_endf
   add [shift], ecx
   jmp gs_read


 gs_endf:
   xor ebx, ebx
   jmp gs_ok

 gs_cr:
   ;
   ; Сбросить результат чтения
   ;
   xor eax, eax

   mov dl, [ebx + esi - 1]
   cmp dl, 0Dh ;le
   jz gs_le

   mov [ebx + esi], byte 0
   mov ebx, esi


   inc esi
   add [shift], esi

   jmp gs_ok

 gs_le:
   mov [ebx + esi - 1], byte 0
   mov [ebx + esi], byte 0
   lea ebx, [esi - 1]

   inc esi
   add [shift], esi

 gs_ok:


   pop esi
   pop edx
   pop ecx
   ;pop ebx
   ;pop eax
  ret


;
; Функция для вывода загруженной инфы 
;

 showcfg:
   push eax
   push ebx
   push ecx
   push edx
   push edi


   xor edx, edx ; счетчик

  sc_loop:

   cmp edx, TABLE_SIZE
   jnb sc_end

   ;
   ; Скопировать в буфер имя и значение переменной
   ;
   mov eax, VNAME_LEN
   imul eax, edx
   lea eax, [nvtable + eax]

   mov cl, [eax]
   cmp cl, byte 0
   jz sc_next

   push eax
   call strlen

   mov ecx, eax
   pop eax

   mov ebx, cfgbuff

   call strcpy

   mov [cfgbuff + ecx], ':'

   lea ebx, [cfgbuff + ecx + 1]

   mov eax, VAR_LEN
   imul eax, edx
   lea eax, [vtable + eax]

   push eax
   call strlen

   mov ecx, eax
   pop eax

   call strcpy

   mov [ebx + ecx], byte 0

   mov eax, cfgbuff
   xor ebx, ebx
   call writemsg

  sc_next:

   inc edx

  jmp sc_loop



  sc_end:
   pop edi
   pop edx
   pop ecx
   pop ebx
   pop eax

 ret

