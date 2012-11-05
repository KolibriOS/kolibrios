;
; Ghost Monitor - утилита (стресс) тестирования и мониторинга системы
; Copyright (C) 2005, 2006, 2007 Mihailov Ilia (ghost.nsk@mail.ru)
; All Right Reserved

; 0x00xx - Menuet 0.xx
; 0x0400 - Kolibi 4 (новое завершение процесса)
; 0x0510 - MSR -> множитель (Kolibri 0.5.1.0)
; 0x0530 - всего/свободно памяти
; 0x0580 - новое завершение процесса
OS_version	equ	0x0580
SMBus		fix	no ; yes

