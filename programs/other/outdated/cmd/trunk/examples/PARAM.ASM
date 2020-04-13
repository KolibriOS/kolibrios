use32

   org 0x0

   db 'MENUET01'
   dd 0x01
   dd START
   dd I_END
   dd 0x100000
   dd 0x7fff0
   dd I_PARAM              ; указатель на параметры

include "CMDIPC.INC"       ; Подключить файл CMDIPC.INC

START:
 call initipc              ; инициализировать для работы с CMD

 mov eax,47                ; вывести сообщение
 mov ebx,mess
 call print

 call eol                  ; пропустить строку
 call eol

 cmp [I_PARAM],byte 0      ; проверить, есть ли параметры
 jz noparam

 mov eax,43
 mov ebx,mess1
 call print

 call eol

 mov eax,30                ; вывести параметры
 mov ebx,I_PARAM
 call print

 jmp end1                  ; переход в конец программы

noparam:
 mov eax,40                ; вывести сообщение о том, что нет параметров
 mov ebx,mess2
 call print

end1:
 jmp endipc               ; завершить программу

mess db 'PARAM.ASM - Test params in IPC programs for CMD'

mess1 db 'This program was started with this params: '
mess2 db 'This program was started without params!'

I_PARAM db 0

I_END: