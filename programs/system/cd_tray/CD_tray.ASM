;
; Управление лотками дисковых приводов ATAPI
; 22.07.2007 Mario79 исходный вариант
;---------------------------------------------------------------------

  use32 	 ; включить 32-битный режим ассемблера
  org	  0x0	      ; адресация с нуля

  db	 'MENUET01'  ; 8-байтный идентификатор MenuetOS
  dd	 0x01	      ; версия заголовка (всегда 1)
  dd	 START	       ; адрес первой команды
  dd	 I_END	       ; размер программы
  dd	 0x11000      ; количество памяти
  dd	 0x11000      ; адрес вершины стэка
  dd	 0x0	     ; адрес буфера для параметров (не используется)
  dd	 0x0	     ; зарезервировано

include 'MACROS.INC' ; макросы облегчают жизнь ассемблерщиков!

;---------------------------------------------------------------------
;---  НАЧАЛО ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------

START:

red:		; перерисовать окно
    call draw_window	; вызываем процедуру отрисовки окна

;---------------------------------------------------------------------
;---  ЦИКЛ ОБРАБОТКИ СОБЫТИЙ  ----------------------------------------
;---------------------------------------------------------------------

still:
    mcall 10	    ; функция 10 - ждать события

    cmp  eax,1	      ; перерисовать окно ?
    je	   red	      ; если да - на метку red
    cmp  eax,2	      ; нажата клавиша ?
    je	   key	      ; если да - на key
    cmp  eax,3	      ; нажата кнопка ?
    je	   button     ; если да - на button

    jmp  still	      ; если другое событие - в начало цикла


;---------------------------------------------------------------------


  key:		  ; нажата клавиша на клавиатуре
    mcall 2	   ; функция 2 - считать код символа (в ah)

    jmp  still	      ; вернуться к началу цикла

;---------------------------------------------------------------------

  button:
    mcall 17	    ; 17 - получить идентификатор нажатой кнопки
    cmp   ah,2
    jne   b3
    mov   dword [load_tray.name],cd0
    jmp   b5.1
  b3:
    cmp   ah,3
    jne   b4
    mov   dword [load_tray.name],cd1
    jmp   b5.1
  b4:
    cmp   ah,4
    jne   b5
    mov   dword [load_tray.name],cd2
    jmp   b5.1
  b5:
    cmp   ah,5
    jne   b6
    mov   dword [load_tray.name],cd3
  .1:  
     mcall 70, load_tray
    jmp   red
  b6:
    cmp   ah,6
    jne   b7
    mov   dword [eject_tray.name],cd0
    jmp   b9.1
  b7:
    cmp   ah,7
    jne   b8
    mov   dword [eject_tray.name],cd1
    jmp   b9.1
  b8:
    cmp   ah,8
    jne   b9
    mov   dword [eject_tray.name],cd2
    jmp   b9.1
  b9:
    cmp   ah,9
    jne   b1
    mov   dword [eject_tray.name],cd3
  .1:  
     mcall 70, eject_tray
    jmp   red
  b1:
    cmp   ah, 1     ; если НЕ нажата кнопка с номером 1,
    jne   still     ;  вернуться

  .exit:
    mcall -1	    ; иначе конец программы



;---------------------------------------------------------------------
;---  ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА  ----------------------------------
;---------------------------------------------------------------------

draw_window:
    mcall 12, 1 	   ; функция 12: сообщить ОС об отрисовке окна
		   ; 1 - начинаем рисовать
		   ; СОЗДАиМ ОКНО

    mcall 0, <100,230>, <100,110>, 0x03AABBCC, 0x805080D0, 0x005080D0
    mcall 71, 1 ,header
    mcall 8,<15,42>,<40,20>,2,0xaaaaaa
    call  draw_buttons
    mov   edx,6
    add   ecx,30 shl 16
    mcall
    call  draw_buttons
    
    mcall 4, <25,25>, 0x80ffffff,text1
    mov   edx,text2
    
    add   ebx,3 shl 16+20
    mcall
    mov   edx,text3
    add   ebx,30
    sub   ebx,3 shl 16
    mcall
    
    mcall 12, 2 	   ; функция 12: сообщить ОС об отрисовке окна
		   ; 2, закончили рисовать

    ret 	       ; выходим из процедуры

draw_buttons:
    pusha
    add   ebx,50 shl 16
    inc   edx
    mcall 
    add   ebx,50 shl 16
    inc   edx
    mcall 
    add   ebx,50 shl 16
    inc   edx
    mcall 
    popa
    ret
;---------------------------------------------------------------------
;---  ДАННЫЕ ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------
header db ' ATAPI Device Tray Control',0
text3 db 'eject   eject   eject   eject',0
text2 db 'load    load    load    load',0
text1 db '/cd0/   /cd1/   /cd2/   /cd3/',0
cd0:  db '/cd0/1/',0
cd1:  db '/cd1/1/',0
cd2:  db '/cd2/1/',0
cd3:  db '/cd3/1/',0
;---------------------------------------------------------------------
load_tray:
    .subfunction dd   11
    .rezerv   dd   0
    .rezerv1  dd   0
    .rezerv2  dd   0
    .rezerv3  dd   0
    db 0
    .name: dd cd0

eject_tray:
    .subfunction dd   10
    .rezerv   dd   0
    .rezerv1  dd   0
    .rezerv2  dd   0
    .rezerv3  dd   0
    db 0
    .name: dd cd0
    
I_END:			 ; метка конца программы
