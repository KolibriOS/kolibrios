;
;   RDsave для Kolibri (0.6.5.0 и старше)
;   
;   Mario79 2005
;   Heavyiron 12.02.2007
;
;   Компилировать FASM'ом
;
;---------------------------------------------------------------------
include 'lang.inc'
include '..\..\..\macros.inc'

appname equ 'RDsave '
version equ '1.2'
  
  use32              ; включить 32-битный режим ассемблера
  org    0x0         ; адресация с нуля

  db     'MENUET01'  ; 8-байтный идентификатор MenuetOS
  dd     0x01        ; версия заголовка (всегда 1)
  dd     START       ; адрес первой команды
  dd     I_END       ; размер программы
  dd     0x2000      ; количество памяти
  dd     0x2000      ; адрес вершины стэка
  dd     0x0         ; адрес буфера для параметров (не используется)
  dd     0x0         ; зарезервировано

include '..\..\develop\editbox\editbox.inc'
use_edit_box procinfo,22,5
al equ eax      ; \ decrease kpack'ed size
purge mov       ; /

;---------------------------------------------------------------------
;---  НАЧАЛО ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------

START:
   mov eax, 40
   mov ebx, 100111b
   mcall
red:                    ; перерисовать окно
    call draw_window    ; вызываем процедуру отрисовки окна

;---------------------------------------------------------------------
;---  ЦИКЛ ОБРАБОТКИ СОБЫТИЙ  ----------------------------------------
;---------------------------------------------------------------------

still:
    mcall 10

    dec  eax             ; перерисовать окно?
    jz   red             ; если да - на метку red
    dec  eax 
    jz   key
    dec  eax
    jz   button

mouse:
        mouse_edit_box editbox
        jmp     still
    
button:
    mov  al,17           ; получить идентификатор нажатой кнопки
    mcall

    cmp  ah,1            ; кнопка с id=1("закрыть")?
    jne  noclose
    or   eax,-1          ; функция -1: завершить программу
    mcall

noclose:
        push    eax
        call    clear_err
        pop     eax
        push    16
        xor     ebx, ebx
        inc     ebx     ; 16.1 = save to /FD/1
        cmp     ah, 2
        je      doit
        inc     ebx     ; 16.2 = save to /FD/2
        cmp     ah, 3
        je      doit
        pop     ebx
        push    18
        mov     bl, 6   ; 18.6 = save to specified folder
        mov     ecx, path3
        cmp     ah, 4
        je      doit
        mov     ecx, path4
doit:
        pop     eax
        mcall
        call    check_for_error
        jmp     still

key:         
    mov  al,2
    mcall
    key_edit_box editbox
    jmp  still


check_for_error:                      ;Обработчик ошибок
        mov     ecx, [sc.work_text]
        mov     edx, ok
        test    eax, eax
        jz      print
        mov     ecx, 0xdd2222
        add     edx, error3 - ok
        dec     eax
        dec     eax
        jz      print
        add     edx, error5 - error3
        dec     eax
        dec     eax
        jz      print
        add     edx, error8 - error5
        dec     eax
        dec     eax
        dec     eax
        jz      print
        add     edx, error9 - error8
        dec     eax
        jz      print
        add     edx, error10 - error9
        dec     eax
        jz      print
        add     edx, error11 - error10
        dec     eax
        jz      print
        add     edx, aUnknownError - error11

 print:
    mov eax,4                              ;надписи
    mov ebx,20 shl 16 + 148
    or  ecx,0x80000000
    mcall
    ret

clear_err:
    mov eax,13
    mov ebx,15 shl 16 + 240
    mov ecx,145 shl 16 +15
    mov edx,[sc.work]
    mcall
    ret

;---------------------------------------------------------------------
;---  ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА  ----------------------------------
;---------------------------------------------------------------------

draw_window:

   mov  eax,48
   mov  ebx,3
   mov  ecx,sc
   mov  edx,sizeof.system_colors
   mcall

   mov eax,12                            ; функция 12: сообщить ОС об отрисовке окна
   mov bl,1                              ; 1 - начинаем рисовать
   mcall

                                         ; СОЗДАЁМ ОКНО
   xor eax,eax                           ; функция 0 : определить и отрисовать окно
   mov ebx,200 shl 16 + 270              ; [x старт] *65536 + [x размер]
   mov ecx,200 shl 16 + 190              ; [y старт] *65536 + [y размер]
   mov edx,[sc.work]                     ; цвет рабочей области  RRGGBB,8->color gl
   or  edx,0x34000000
   mov edi,title                        ; ЗАГОЛОВОК ОКНА
   mcall

draw_edit_box editbox,use_f9,procinfo   ;рисование edit box'ов

   mov al,13                            ;отрисовка теней кнопок
   mov ebx,194 shl 16 + 60
   mov ecx,34 shl 16 +15
   mov edx,0x444444
   mcall

   add ecx,20 shl 16
   mcall

   add ecx,20 shl 16
   mcall

   add ecx,40 shl 16
   mcall

   mov eax,8                             ;отрисовка кнопок
   sub ebx,4 shl 16
   sub ecx,4 shl 16
   mov edx,5
   mov esi,[sc.work_button]
   mcall

   sub ecx,40 shl 16
   dec edx
   mcall

   sub ecx,20 shl 16
   dec edx
   mcall

   sub ecx,20 shl 16
   dec edx
   mcall

   mov al,4                              ;надписи
   mov ebx,45 shl 16 + 12
   mov ecx,[sc.work_text]
   or  ecx,0x80000000
   mov edx,label1
   mcall

   mov ebx,150 shl 16 + 35
   mov edx,path1
   mcall

   add ebx,20
   mov edx,path2
   mcall

   mov ebx,75 shl 16 + 75
   mov edx,path3
   mcall

   mov ebx,30 shl 16 + 97
   mov edx,label2
   mcall

   mov ebx,40 shl 16 + 135
   mov edx,label3
   mcall

   mov ecx,[sc.work_button_text]
   or  ecx,0x80000000
   mov ebx,195 shl 16 + 35
   mov edx,save
   mcall

   add ebx,20
   mcall

   add ebx,20
   mcall

   add ebx,40
   mcall

   mov al,12                            ; функция 12: сообщить ОС об отрисовке окна
   mov ebx,2                            ; 2, закончили рисовать
   mcall

   ret                                  ; выходим из процедуры


;---------------------------------------------------------------------
;---  ДАННЫЕ ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------

title db appname,version,0

editbox:
edit1 edit_box 170,10,113,0xffffff,0xaabbcc,0,0,0,512,path4,ed_focus,26,26
editbox_end:

if lang eq ru
save    db 'Сохранить',0
label1  db 'Выберите один из вариантов:',0
label2  db 'Или введите полный путь к файлу:',0
label3  db 'Все папки должны существовать',0
ok      db 'RAM-диск сохранен успешно',0
error3  db 'Неизвестная файловая система',0
error5  db 'Несуществующий путь',0
error8  db 'Нет места на диске',0
error9  db 'Таблица FAT разрушена',0
error10 db 'Доступ запрещен',0
error11 db 'Ошибка устройства',0
aUnknownError db 'Неизвестная ошибка',0

else
save    db '  Save',0
label1  db 'Select one of the variants:',0
label2  db '   Or enter full path to file:',0
label3  db '    All folders must exist',0
ok      db 'RAM-drive was saved successfully',0
error3  db 'Unknown file system',0
error5  db 'Incorrect path',0
error8  db 'Disk is full',0
error9  db 'FAT table corrupted',0
error10 db 'Access denied',0
error11 db 'Device error',0
aUnknownError db 'Unknown error',0

end if

mouse_flag: dd 0x0

path1   db '/fd/1/',0
path2   db '/fd/2/',0
path3   db '/hd0/1/kolibri.img',0
path4   db '/hd0/1/kolibri/kolibri.img',0  ;для резервного сохранения

;---------------------------------------------------------------------
I_END:                             ; метка конца программы

rb 514

sc     system_colors

procinfo process_information


