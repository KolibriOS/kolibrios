$Revision: 431 $
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;**********************************************************
;  Непосредственная работа с устройством СD (ATAPI)
;**********************************************************
; Автор исходного текста  Кулаков Владимир Геннадьевич.
; Адаптация и доработка Mario79

;        Процедура для полного считывания всех
;           данных из сектора компакт-диска
; Автор текста программы Кулаков Владимир Геннадьевич.


; Максимальное количество повторений операции чтения
MaxRetr equ 3
; Предельное время ожидания готовности к приему команды
; (в тиках)
BSYWaitTime equ 1000  ;2

;*************************************************
;*      ПОЛНОЕ ЧТЕНИЕ СЕКТОРА КОМПАКТ-ДИСКА      *
;* Считываются данные пользователя, информация   *
;* субканала и контрольная информация            *
;* Входные параметры передаются через глобальные *
;* перменные:                                    *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале;           *
;* CDSectorAddress - адрес считываемого сектора. *
;* Данные считывается в массив CDDataBuf.        *
;*************************************************
ReadCD:
        pusha
; Задать размер сектора
        mov     [CDBlockSize],2048 ;2352
; Очистить буфер пакетной команды
        call  clear_packet_buffer
; Сформировать пакетную команду для считывания
; сектора данных
        ; Задать код команды Read CD
        mov     [PacketCommand],byte 0x28  ;0xBE
        ; Задать адрес сектора
        mov     AX,word [CDSectorAddress+2]
        xchg    AL,AH
        mov     word [PacketCommand+2],AX
        mov     AX,word [CDSectorAddress]
        xchg    AL,AH
        mov     word [PacketCommand+4],AX
;        mov   eax,[CDSectorAddress]
;        mov   [PacketCommand+2],eax
        ; Задать количество считываемых секторов
        mov     [PacketCommand+8],byte 1
        ; Задать считывание данных в полном объеме
;        mov     [PacketCommand+9],byte 0xF8
; Подать команду
        call    SendPacketDatCommand
;        call test_mario79
        popa
        ret

;********************************************
;*        ЧТЕНИЕ СЕКТОРА С ПОВТОРАМИ        *
;* Многократное повторение чтения при сбоях *
;********************************************
ReadCDWRetr:
        pusha
; Цикл, пока команда не выполнена успешно или не
; исчерпано количество попыток
        mov     ECX,MaxRetr
@@NextRetr:
; Подать команду
        call    ReadCD
        cmp     [DevErrorCode],0
        je      @@End_4
; Задержка на 2,5 секунды
        mov     EAX,[timer_ticks]
        add     EAX,250 ;50
@@Wait:
        call    change_task
        cmp     EAX,[timer_ticks]
        ja      @@Wait
        loop    @@NextRetr
;        call test_mario79
; Сообщение об ошибке
;        mov     SI,offset ErrS
;        call    FatalError
@@End_4:
        popa
        ret


;   Универсальные процедуры, обеспечивающие выполнение
;             пакетных команд в режиме PIO
;
; Автор текста программы Кулаков Владимир Геннадьевич.

; Максимально допустимое время ожидания реакции
; устройства на пакетную команду (в тиках)
MaxCDWaitTime equ 1000 ;200 ;10 секунд

; Область памяти для формирования пакетной команды
PacketCommand:   rb 12  ;DB 12 DUP (?)
; Область памяти для приема данных от дисковода
;CDDataBuf       DB 4096 DUP (0)
; Размер принимаемого блока данных в байтах
CDBlockSize     DW ?
; Адрес считываемого сектора данных
CDSectorAddress: DD ?
; Время начала очередной операции с диском
TickCounter_1 DD 0
; Время начала ожидания готовности устройства
WURStartTime DD 0
; указатель буфера для считывания
CDDataBuf_pointer dd 0

;****************************************************
;*    ПОСЛАТЬ УСТРОЙСТВУ ATAPI ПАКЕТНУЮ КОМАНДУ,    *
;* ПРЕДУСМАТРИВАЮЩУЮ ПЕРЕДАЧУ ОДНОГО СЕКТОРА ДАННЫХ *
;*     РАЗМЕРОМ 2048 БАЙТ ОТ УСТРОЙСТВА К ХОСТУ     *
;* Входные параметры передаются через глобальные    *
;* перменные:                                       *
;* ChannelNumber - номер канала;                    *
;* DiskNumber - номер диска на канале;              *
;* PacketCommand - 12-байтный командный пакет;      *
;* CDBlockSize - размер принимаемого блока данных.  *
;****************************************************
SendPacketDatCommand:
        pushad
; Задать режим CHS
        mov     [ATAAddressMode],0
; Послать ATA-команду передачи пакетной команды
        mov     [ATAFeatures],0
        mov     [ATASectorCount],0
        mov     [ATASectorNumber],0
        ; Загрузить размер передаваемого блока
        mov     AX,[CDBlockSize]
        mov     [ATACylinder],AX
        mov     [ATAHead],0
        mov     [ATACommand],0A0h
        call    SendCommandToHDD_1
;        call test_mario79
        cmp     [DevErrorCode],0 ;проверить код ошибки
        jne     @@End_8    ;закончить, сохранив код ошибки

; Ожидание готовности дисковода к приему
; пакетной команды
        mov     DX,[ATABasePortAddr]
        add     DX,7     ;порт 1х7h
@@WaitDevice0:
        call    change_task
        ; Проверить время выполнения команды
        mov     EAX,[timer_ticks]
        sub     EAX,[TickCounter_1]
        cmp     EAX,BSYWaitTime
        ja      @@Err1_1   ;ошибка тайм-аута
        ; Проверить готовность
        in      AL,DX
        test    AL,80h   ;состояние сигнала BSY
        jnz     @@WaitDevice0
        test    AL,1     ;состояние сигнала ERR
        jnz     @@Err6
        test    AL,08h   ;состояние сигнала DRQ
        jz      @@WaitDevice0
; Послать пакетную команду
        cli
        mov     DX,[ATABasePortAddr]
        mov     AX,[PacketCommand]
        out     DX,AX
        mov     AX,[PacketCommand+2]
        out     DX,AX
        mov     AX,[PacketCommand+4]
        out     DX,AX
        mov     AX,[PacketCommand+6]
        out     DX,AX
        mov     AX,[PacketCommand+8]
        out     DX,AX
        mov     AX,[PacketCommand+10]
        out     DX,AX
        sti
; Ожидание готовности данных
        mov     DX,[ATABasePortAddr]
        add     DX,7   ;порт 1х7h
@@WaitDevice1:
        call    change_task
        ; Проверить время выполнения команды
        mov     EAX,[timer_ticks]
        sub     EAX,[TickCounter_1]
        cmp     EAX,MaxCDWaitTime
        ja      @@Err1_1   ;ошибка тайм-аута
        ; Проверить готовность
        in      AL,DX
        test    AL,80h   ;состояние сигнала BSY
        jnz     @@WaitDevice1
        test    AL,1     ;состояние сигнала ERR
        jnz     @@Err6_temp
        test    AL,08h   ;состояние сигнала DRQ
        jz      @@WaitDevice1
        cli
; Принять блок данных от контроллера
        mov     EDI,[CDDataBuf_pointer] ;0x7000  ;CDDataBuf
        ; Загрузить адрес регистра данных контроллера
        mov     DX,[ATABasePortAddr] ;порт 1x0h
        ; Загрузить в счетчик размер блока в байтах
        mov     CX,[CDBlockSize]
        ; Вычислить размер блока в 16-разрядных словах
        shr     CX,1 ;разделить размер блока на 2
        ; Принять блок данных
        cld
        rep     insw
        sti
        ; Успешное завершение приема данных
        jmp @@End_8

; Записать код ошибки
@@Err1_1:
        mov     [DevErrorCode],1
        jmp @@End_8
@@Err6_temp:
        mov     [DevErrorCode],7
        jmp @@End_8
@@Err6:
        mov     [DevErrorCode],6

@@End_8:
        popad
        ret



;***********************************************
;*  ПОСЛАТЬ УСТРОЙСТВУ ATAPI ПАКЕТНУЮ КОМАНДУ, *
;*     НЕ ПРЕДУСМАТРИВАЮЩУЮ ПЕРЕДАЧИ ДАННЫХ    *
;* Входные параметры передаются через          *
;* глобальные перменные:                       *
;* ChannelNumber - номер канала;               *
;* DiskNumber - номер диска на канале;         *
;* PacketCommand - 12-байтный командный пакет. *
;***********************************************
SendPacketNoDatCommand:
        pushad
; Задать режим CHS
        mov     [ATAAddressMode],0
; Послать ATA-команду передачи пакетной команды
        mov     [ATAFeatures],0
        mov     [ATASectorCount],0
        mov     [ATASectorNumber],0
        mov     [ATACylinder],0
        mov     [ATAHead],0
        mov     [ATACommand],0A0h
        call    SendCommandToHDD_1
        cmp     [DevErrorCode],0 ;проверить код ошибки
        jne     @@End_9  ;закончить, сохранив код ошибки
; Ожидание готовности дисковода к приему
; пакетной команды
        mov     DX,[ATABasePortAddr]
        add     DX,7   ;порт 1х7h
@@WaitDevice0_1:
        call    change_task
        ; Проверить время ожидания
        mov     EAX,[timer_ticks]
        sub     EAX,[TickCounter_1]
        cmp     EAX,BSYWaitTime
        ja      @@Err1_3   ;ошибка тайм-аута
        ; Проверить готовность
        in      AL,DX
        test    AL,80h   ;состояние сигнала BSY
        jnz     @@WaitDevice0_1
        test    AL,1     ;состояние сигнала ERR
        jnz     @@Err6_1
        test    AL,08h   ;состояние сигнала DRQ
        jz      @@WaitDevice0_1
; Послать пакетную команду
;        cli
        mov     DX,[ATABasePortAddr]
        mov     AX,word [PacketCommand]
        out     DX,AX
        mov     AX,word [PacketCommand+2]
        out     DX,AX
        mov     AX,word [PacketCommand+4]
        out     DX,AX
        mov     AX,word [PacketCommand+6]
        out     DX,AX
        mov     AX,word [PacketCommand+8]
        out     DX,AX
        mov     AX,word [PacketCommand+10]
        out     DX,AX
;        sti
; Ожидание подтверждения приема команды
        mov     DX,[ATABasePortAddr]
        add     DX,7   ;порт 1х7h
@@WaitDevice1_1:
        call    change_task
        ; Проверить время выполнения команды
        mov     EAX,[timer_ticks]
        sub     EAX,[TickCounter_1]
        cmp     EAX,MaxCDWaitTime
        ja      @@Err1_3   ;ошибка тайм-аута
        ; Ожидать освобождения устройства
        in      AL,DX
        test    AL,80h   ;состояние сигнала BSY
        jnz     @@WaitDevice1_1
        test    AL,1     ;состояние сигнала ERR
        jnz     @@Err6_1
        test    AL,40h   ;состояние сигнала DRDY
        jz      @@WaitDevice1_1
        jmp @@End_9

; Записать код ошибки
@@Err1_3:
        mov     [DevErrorCode],1
        jmp @@End_9
@@Err6_1:
        mov     [DevErrorCode],6
@@End_9:
        popad
        ret

;****************************************************
;*          ПОСЛАТЬ КОМАНДУ ЗАДАННОМУ ДИСКУ         *
;* Входные параметры передаются через глобальные    *
;* переменные:                                      *
;* ChannelNumber - номер канала (1 или 2);          *
;* DiskNumber - номер диска (0 или 1);              *
;* ATAFeatures - "особенности";                     *
;* ATASectorCount - количество секторов;            *
;* ATASectorNumber - номер начального сектора;      *
;* ATACylinder - номер начального цилиндра;         *
;* ATAHead - номер начальной головки;               *
;* ATAAddressMode - режим адресации (0-CHS, 1-LBA); *
;* ATACommand - код команды.                        *
;* После успешного выполнения функции:              *
;* в ATABasePortAddr - базовый адрес HDD;           *
;* в DevErrorCode - ноль.                           *
;* При возникновении ошибки в DevErrorCode будет    *
;* возвращен код ошибки.                            *
;****************************************************
SendCommandToHDD_1:
        pushad
; Проверить значение кода режима
        cmp     [ATAAddressMode],1
        ja      @@Err2_4
; Проверить корректность номера канала
        mov     BX,[ChannelNumber]
        cmp     BX,1
        jb      @@Err3_4
        cmp     BX,2
        ja      @@Err3_4
; Установить базовый адрес
        dec     BX
        shl     BX,1
        movzx   ebx,bx
        mov     AX,[ebx+StandardATABases]
        mov     [ATABasePortAddr],AX
; Ожидание готовности HDD к приему команды
        ; Выбрать нужный диск
        mov     DX,[ATABasePortAddr]
        add     DX,6    ;адрес регистра головок
        mov     AL,[DiskNumber]
        cmp     AL,1    ;проверить номера диска
        ja      @@Err4_4
        shl     AL,4
        or      AL,10100000b
        out     DX,AL
        ; Ожидать, пока диск не будет готов
        inc     DX
;        mov  ecx,0xfff
        mov     eax,[timer_ticks]
        mov     [TickCounter_1],eax
@@WaitHDReady_2:
        call    change_task
        ; Проверить время ожидания
;        dec  ecx
;        cmp  ecx,0
;        je     @@Err1
        mov     eax,[timer_ticks]
        sub     eax,[TickCounter_1]
        cmp     eax,BSYWaitTime ;300    ;ожидать 3 сек.        
        ja      @@Err1_4   ;ошибка тайм-аута
        ; Прочитать регистр состояния
        in      AL,DX
        ; Проверить состояние сигнала BSY
        test    AL,80h
        jnz     @@WaitHDReady_2
        ; Проверить состояние сигнала DRQ
        test    AL,08h
        jnz     @@WaitHDReady_2
; Загрузить команду в регистры контроллера
        cli
        mov     DX,[ATABasePortAddr]
        inc     DX      ;регистр "особенностей"
        mov     AL,[ATAFeatures]
        out     DX,AL
        inc     DX      ;счетчик секторов
        mov     AL,[ATASectorCount]
        out     DX,AL
        inc     DX      ;регистр номера сектора
        mov     AL,[ATASectorNumber]
        out     DX,AL
        inc     DX      ;номер цилиндра (младший байт)
        mov     AX,[ATACylinder]
        out     DX,AL
        inc     DX      ;номер цилиндра (старший байт)
        mov     AL,AH
        out     DX,AL
        inc     DX      ;номер головки/номер диска
        mov     AL,[DiskNumber]
        shl     AL,4
        cmp     [ATAHead],0Fh ;проверить номер головки
        ja      @@Err5_4
        or      AL,[ATAHead]
        or      AL,10100000b
        mov     AH,[ATAAddressMode]
        shl     AH,6
        or      AL,AH
        out     DX,AL
; Послать команду
        mov     AL,[ATACommand]
        inc     DX      ;регистр команд
        out     DX,AL
        sti
; Сбросить признак ошибки
        mov     [DevErrorCode],0
        jmp @@End_10
; Записать код ошибки
@@Err1_4:
        mov     [DevErrorCode],1
        jmp @@End_10
@@Err2_4:
        mov     [DevErrorCode],2
        jmp @@End_10
@@Err3_4:
        mov     [DevErrorCode],3
        jmp @@End_10
@@Err4_4:
        mov     [DevErrorCode],4
        jmp @@End_10
@@Err5_4:
        mov     [DevErrorCode],5
; Завершение работы программы
@@End_10:
        sti
        popad 
        ret
        
;*************************************************
;*    ОЖИДАНИЕ ГОТОВНОСТИ УСТРОЙСТВА К РАБОТЕ    *
;* Входные параметры передаются через глобальные *
;* перменные:                                    *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
WaitUnitReady:
        pusha
; Запомнить время начала операции
        mov     EAX,[timer_ticks]
        mov     [WURStartTime],EAX
; Очистить буфер пакетной команды
        call  clear_packet_buffer
; Сформировать команду TEST UNIT READY
        mov     [PacketCommand],word 00h
; ЦИКЛ ОЖИДАНИЯ ГОТОВНОСТИ УСТРОЙСТВА
@@SendCommand:
        ; Подать команду проверки готовности
        call    SendPacketNoDatCommand
        call    change_task
        ; Проверить код ошибки
        cmp     [DevErrorCode],0
        je      @@End_11
        ; Проверить время ожидания готовности
        mov     EAX,[timer_ticks]
        sub     EAX,[WURStartTime]
        cmp     EAX,MaxCDWaitTime
        jb      @@SendCommand
        ; Ошибка тайм-аута
        mov     [DevErrorCode],1
@@End_11:
        popa
        ret

        
;*************************************************
;*         ЗАГРУЗИТЬ НОСИТЕЛЬ В ДИСКОВОД         *
;* Входные параметры передаются через глобальные *
;* перменные:                                    *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
LoadMedium:
        pusha
; Очистить буфер пакетной команды
        call  clear_packet_buffer
; Сформировать команду START/STOP UNIT
        ; Задать код команды
        mov     [PacketCommand],word 1Bh
        ; Задать операцию загрузки носителя
        mov     [PacketCommand+4],word 00000011b
; Подать команду
        call    SendPacketNoDatCommand
        popa
        ret

;*************************************************
;*         ИЗВЛЕЧЬ НОСИТЕЛЬ ИЗ ДИСКОВОДА         *
;* Входные параметры передаются через глобальные *
;* перменные:                                    *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
UnloadMedium:
        pusha
; Очистить буфер пакетной команды
        call  clear_packet_buffer
; Сформировать команду START/STOP UNIT
        ; Задать код команды
        mov     [PacketCommand],word 1Bh
        ; Задать операцию извлечения носителя
        mov     [PacketCommand+4],word 00000010b
; Подать команду
        call    SendPacketNoDatCommand
        popa
        ret

;*************************************************
;* ОПРЕДЕЛИТЬ ОБЩЕЕ КОЛИЧЕСТВО СЕКТОРОВ НА ДИСКЕ *
;* Входные параметры передаются через глобальные *
;* переменные:                                   *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
ReadCapacity:
        pusha
; Очистить буфер пакетной команды
        call  clear_packet_buffer
; Задать размер буфера в байтах
        mov     [CDBlockSize],8
; Сформировать команду READ CAPACITY
        mov     [PacketCommand],word 25h
; Подать команду
        call    SendPacketDatCommand
        popa
        ret

clear_packet_buffer:        
; Очистить буфер пакетной команды
        mov     [PacketCommand],dword 0
        mov     [PacketCommand+4],dword 0
        mov     [PacketCommand+8],dword 0
        ret

