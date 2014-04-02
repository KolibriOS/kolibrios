;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

;-----------------------------------------------------------------------------
;**********************************************************
;  Непосредственная работа с устройством СD (ATAPI)
;**********************************************************
; Автор части исходного текста Кулаков Владимир Геннадьевич
; Адаптация, доработка и разработка Mario79,<Lrz>

; Максимальное количество повторений операции чтения
MaxRetr equ 10
; Предельное время ожидания готовности к приему команды
; (в тиках)
BSYWaitTime equ 1000  ;2
NoTickWaitTime equ 0xfffff
CDBlockSize equ 2048
;********************************************
;*        ЧТЕНИЕ СЕКТОРА С ПОВТОРАМИ        *
;* Многократное повторение чтения при сбоях *
;********************************************
ReadCDWRetr:
;-----------------------------------------------------------
; input  : eax = block to read
;          ebx = destination
;-----------------------------------------------------------
        pushad
        mov     eax, [CDSectorAddress]
        mov     ebx, [CDDataBuf_pointer]
        call    cd_calculate_cache
        xor     edi, edi
        add     esi, 8
        inc     edi
;--------------------------------------
align 4
.hdreadcache:
        cmp     [esi], eax      ; correct sector
        je      .yeshdcache

        add     esi, 8
        inc     edi
        dec     ecx
        jnz     .hdreadcache

        call    find_empty_slot_CD_cache ; ret in edi

        push    edi
        push    eax
        call    cd_calculate_cache_2
        shl     edi, 11
        add     edi, eax
        mov     [CDDataBuf_pointer], edi
        pop     eax
        pop     edi

        call    ReadCDWRetr_1
        cmp     [DevErrorCode], 0
        jne     .exit

        mov     [CDDataBuf_pointer], ebx
        call    cd_calculate_cache_1
        lea     esi, [edi*8+esi]
        mov     [esi], eax      ; sector number
;--------------------------------------
.yeshdcache:
        mov     esi, edi
        shl     esi, 11 ;9
        push    eax
        call    cd_calculate_cache_2
        add     esi, eax
        pop     eax
        mov     edi, ebx ;[CDDataBuf_pointer]
        mov     ecx, 512 ;/4
        cld
        rep movsd               ; move data
;--------------------------------------
.exit:
        popad
        ret
;-----------------------------------------------------------------------------
ReadCDWRetr_1:
        pushad
; Цикл, пока команда не выполнена успешно или не
; исчерпано количество попыток
        mov     ecx, MaxRetr
;--------------------------------------
align 4
@@NextRetr:
; Подать команду
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
;ReadCD:
        push    ecx
; Очистить буфер пакетной команды
        call    clear_packet_buffer
; Сформировать пакетную команду для считывания
; сектора данных
; Задать код команды Read CD
        mov     [PacketCommand], byte 0x28 ;0xBE
; Задать адрес сектора
        mov     ax, word [CDSectorAddress+2]
        xchg    al, ah
        mov     word [PacketCommand+2], ax
        mov     ax, word [CDSectorAddress]
        xchg    al, ah
        mov     word [PacketCommand+4], ax
; Задать количество считываемых секторов
        mov     [PacketCommand+8], byte 1
; Подать команду
        call    SendPacketDatCommand
        pop     ecx

        test    eax, eax
        jz      @@End_4

        or      ecx, ecx        ;{SPraid.simba} (for cd load)
        jz      @@End_4

        dec     ecx

        cmp     [timer_ticks_enable], 0
        jne     @f

        mov     eax, NoTickWaitTime
;--------------------------------------
align 4
.wait:
        dec     eax
        jz      @@NextRetr

        jmp     .wait
;--------------------------------------
align 4
@@:
        loop    @@NextRetr
;--------------------------------------
@@End_4:
        mov     dword [DevErrorCode], eax
        popad
        ret
;-----------------------------------------------------------------------------
; Универсальные процедуры, обеспечивающие выполнение
;             пакетных команд в режиме PIO
; Максимально допустимое время ожидания реакции
; устройства на пакетную команду (в тиках)
;-----------------------------------------------------------------------------
MaxCDWaitTime equ 1000 ;200 ;10 секунд
uglobal
; Область памяти для формирования пакетной команды
PacketCommand:
                 rb 12  ;DB 12 DUP (?)
; Адрес считываемого сектора данных
CDSectorAddress:   dd ?
; Время начала очередной операции с диском
TickCounter_1     dd 0
; Время начала ожидания готовности устройства
WURStartTime      dd 0
; указатель буфера для считывания
CDDataBuf_pointer dd 0
endg
;-----------------------------------------------------------------------------
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
; return eax DevErrorCode
;****************************************************
SendPacketDatCommand:
        xor     eax, eax
; Задать режим CHS
        mov     byte [ATAAddressMode], al
; Послать ATA-команду передачи пакетной команды
        mov     byte [ATAFeatures], al
        mov     byte [ATASectorCount], al
        mov     byte [ATASectorNumber], al
        ; Загрузить размер передаваемого блока
        mov     [ATAHead], al
        mov     [ATACylinder], CDBlockSize
        mov     [ATACommand], 0xA0
        call    SendCommandToHDD_1
        test    eax, eax
        jnz     @@End_8    ;закончить, сохранив код ошибки
; Ожидание готовности дисковода к приему
; пакетной команды
        mov     dx, [ATABasePortAddr]
        add     dx, 7    ;порт 1х7h
        mov     ecx, NoTickWaitTime
;--------------------------------------
align 4
@@WaitDevice0:
        cmp     [timer_ticks_enable], 0
        jne     @f

        dec     ecx
        jz      @@Err1_1

        jmp     .test
;--------------------------------------
align 4
@@:
        call    change_task
        ; Проверить время выполнения команды
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter_1]
        cmp     eax, BSYWaitTime
        ja      @@Err1_1   ;ошибка тайм-аута
        ; Проверить готовность
;--------------------------------------
align 4
.test:
        in      al, dx
        test    al, 0x80  ;состояние сигнала BSY
        jnz     @@WaitDevice0

        test    al, 1    ;состояние сигнала ERR
        jnz     @@Err6

        test    al, 0x8  ;состояние сигнала DRQ
        jz      @@WaitDevice0
; Послать пакетную команду
        cli
        mov     dx, [ATABasePortAddr]
        mov     ax, [PacketCommand]
        out     dx, ax
        mov     ax, [PacketCommand+2]
        out     dx, ax
        mov     ax, [PacketCommand+4]
        out     dx, ax
        mov     ax, [PacketCommand+6]
        out     dx, ax
        mov     ax, [PacketCommand+8]
        out     dx, ax
        mov     ax, [PacketCommand+10]
        out     dx, ax
        sti
; Ожидание готовности данных
        mov     dx, [ATABasePortAddr]
        add     dx, 7  ;порт 1х7h
        mov     ecx, NoTickWaitTime
;--------------------------------------
align 4
@@WaitDevice1:
        cmp     [timer_ticks_enable], 0
        jne     @f

        dec     ecx
        jz      @@Err1_1

        jmp     .test_1
;--------------------------------------
align 4
@@:
        call    change_task
        ; Проверить время выполнения команды
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter_1]
        cmp     eax, MaxCDWaitTime
        ja      @@Err1_1   ;ошибка тайм-аута
        ; Проверить готовность
;--------------------------------------
align 4
.test_1:
        in      al, dx
        test    al, 0x80  ;состояние сигнала BSY
        jnz     @@WaitDevice1

        test    al, 1    ;состояние сигнала ERR
        jnz     @@Err6_temp

        test    al, 0x8  ;состояние сигнала DRQ
        jz      @@WaitDevice1
; Принять блок данных от контроллера
        mov     edi, [CDDataBuf_pointer]
        ; Загрузить адрес регистра данных контроллера
        mov     dx, [ATABasePortAddr]
        ; Загрузить в счетчик размер блока в байтах
        xor     ecx, ecx
        mov     cx, CDBlockSize
        ; Вычислить размер блока в 16-разрядных словах
        shr     cx, 1 ;разделить размер блока на 2
        ; Принять блок данных
        cli
        cld
        rep insw
        sti
;--------------------------------------
; Успешное завершение приема данных
@@End_8:
        xor     eax, eax
        ret
;--------------------------------------
; Записать код ошибки
@@Err1_1:
        xor     eax, eax
        inc     eax
        ret
;--------------------------------------
@@Err6_temp:
        mov     eax, 7
        ret
;--------------------------------------
@@Err6:
        mov     eax, 6
        ret
;-----------------------------------------------------------------------------
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
        xor     eax, eax
; Задать режим CHS
        mov     byte [ATAAddressMode], al
; Послать ATA-команду передачи пакетной команды
        mov     byte [ATAFeatures], al
        mov     byte [ATASectorCount], al
        mov     byte [ATASectorNumber], al
        mov     word [ATACylinder], ax
        mov     byte [ATAHead], al
        mov     [ATACommand], 0xA0
        call    SendCommandToHDD_1
        test    eax, eax
        jnz     @@End_9  ;закончить, сохранив код ошибки
; Ожидание готовности дисковода к приему
; пакетной команды
        mov     dx, [ATABasePortAddr]
        add     dx, 7  ;порт 1х7h
;--------------------------------------
align 4
@@WaitDevice0_1:
        call    change_task
        ; Проверить время ожидания
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter_1]
        cmp     eax, BSYWaitTime
        ja      @@Err1_3   ;ошибка тайм-аута
        ; Проверить готовность
        in      al, dx
        test    al, 0x80  ;состояние сигнала BSY
        jnz     @@WaitDevice0_1

        test    al, 1    ;состояние сигнала ERR
        jnz     @@Err6_1

        test    al, 0x8  ;состояние сигнала DRQ
        jz      @@WaitDevice0_1
; Послать пакетную команду
;        cli
        mov     dx, [ATABasePortAddr]
        mov     ax, word [PacketCommand]
        out     dx, ax
        mov     ax, word [PacketCommand+2]
        out     dx, ax
        mov     ax, word [PacketCommand+4]
        out     dx, ax
        mov     ax, word [PacketCommand+6]
        out     dx, ax
        mov     ax, word [PacketCommand+8]
        out     dx, ax
        mov     ax, word [PacketCommand+10]
        out     dx, ax
;        sti
        cmp     [ignore_CD_eject_wait], 1
        je      @@clear_DEC
; Ожидание подтверждения приема команды
        mov     dx, [ATABasePortAddr]
        add     dx, 7  ;порт 1х7h
;--------------------------------------
align 4
@@WaitDevice1_1:
        call    change_task
        ; Проверить время выполнения команды
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter_1]
        cmp     eax, MaxCDWaitTime
        ja      @@Err1_3   ;ошибка тайм-аута
        ; Ожидать освобождения устройства
        in      al, dx
        test    al, 0x80  ;состояние сигнала BSY
        jnz     @@WaitDevice1_1

        test    al, 1    ;состояние сигнала ERR
        jnz     @@Err6_1

        test    al, 0x40  ;состояние сигнала DRDY
        jz      @@WaitDevice1_1
;--------------------------------------
@@clear_DEC:
        and     [DevErrorCode], 0
        popad
        ret
;--------------------------------------
; Записать код ошибки
@@Err1_3:
        xor     eax, eax
        inc     eax
        jmp     @@End_9
;--------------------------------------
@@Err6_1:
        mov     eax, 6
;--------------------------------------
@@End_9:
        mov     [DevErrorCode], eax
        popad
        ret
;-----------------------------------------------------------------------------
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
;* возвращен код ошибки в eax                       *
;****************************************************
SendCommandToHDD_1:
; Проверить значение кода режима
        cmp     [ATAAddressMode], 1
        ja      @@Err2_4
; Проверить корректность номера канала
        mov     bx, [ChannelNumber]
        cmp     bx, 1
        jb      @@Err3_4

        cmp     bx, 2
        ja      @@Err3_4
; Установить базовый адрес
        dec     bx
        shl     ebx, 2
        movzx   ebx, bx
        mov     eax, [cdpos]
        dec     eax
        shr     eax, 2
        imul    eax, sizeof.IDE_DATA
        add     eax, IDE_controller_1
        add     eax, ebx
        mov     ax, [eax+IDE_DATA.BAR0_val]
        mov     [ATABasePortAddr], ax
; Ожидание готовности HDD к приему команды
        ; Выбрать нужный диск
        mov     dx, [ATABasePortAddr]
        add     dx, 6   ;адрес регистра головок
        mov     al, [DiskNumber]
        cmp     al, 1   ;проверить номера диска
        ja      @@Err4_4

        shl     al, 4
        or      al, 10100000b
        out     dx, al
        ; Ожидать, пока диск не будет готов
        inc     dx
        mov     eax, [timer_ticks]
        mov     [TickCounter_1], eax
        mov     ecx, NoTickWaitTime
;--------------------------------------
align 4
@@WaitHDReady_2:
        cmp     [timer_ticks_enable], 0
        jne     @f

        dec     ecx
        jz      @@Err1_4

        jmp     .test
;--------------------------------------
align 4
@@:
        call    change_task
        ; Проверить время ожидания
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter_1]
        cmp     eax, BSYWaitTime ;300    ;ожидать 3 сек.
        ja      @@Err1_4   ;ошибка тайм-аута
;--------------------------------------
align 4
.test:
        in      al, dx ; Прочитать регистр состояния
        ; Проверить состояние сигнала BSY
        test    al, 0x80
        jnz     @@WaitHDReady_2
        ; Проверить состояние сигнала DRQ
        test    al, 0x8
        jnz     @@WaitHDReady_2
; Загрузить команду в регистры контроллера
        cli
        mov     dx, [ATABasePortAddr]
        inc     dx      ;регистр "особенностей"
        mov     al, [ATAFeatures]
        out     dx, al
        inc     dx      ;счетчик секторов
        mov     al, [ATASectorCount]
        out     dx, al
        inc     dx      ;регистр номера сектора
        mov     al, [ATASectorNumber]
        out     dx, al
        inc     dx      ;номер цилиндра (младший байт)
        mov     ax, [ATACylinder]
        out     dx, al
        inc     dx      ;номер цилиндра (старший байт)
        mov     al, ah
        out     dx, al
        inc     dx      ;номер головки/номер диска
        mov     al, [DiskNumber]
        shl     al, 4
        cmp     [ATAHead], 0xF ;проверить номер головки
        ja      @@Err5_4

        or      al, [ATAHead]
        or      al, 10100000b
        mov     ah, [ATAAddressMode]
        shl     ah, 6
        or      al, ah
        out     dx, al
; Послать команду
        mov     al, [ATACommand]
        inc     dx      ;регистр команд
        out     dx, al
        sti
;--------------------------------------
@@End_10:
        xor     eax, eax
        ret
;--------------------------------------
; Записать код ошибки
@@Err1_4:
        xor     eax, eax
        inc     eax
        ret
;--------------------------------------
@@Err2_4:
        mov     eax, 2
        ret
;--------------------------------------
@@Err3_4:
        mov     eax, 3
        ret
;--------------------------------------
@@Err4_4:
        mov     eax, 4
        ret
;--------------------------------------
@@Err5_4:
        mov     eax, 5
        ret
;-----------------------------------------------------------------------------
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
        mov     eax, [timer_ticks]
        mov     [WURStartTime], eax
; Очистить буфер пакетной команды
        call    clear_packet_buffer
; Сформировать команду TEST UNIT READY
        mov     [PacketCommand], word 0
; ЦИКЛ ОЖИДАНИЯ ГОТОВНОСТИ УСТРОЙСТВА
        mov     ecx, NoTickWaitTime
;--------------------------------------
align 4
@@SendCommand:
        ; Подать команду проверки готовности
        call    SendPacketNoDatCommand
        cmp     [timer_ticks_enable], 0
        jne     @f

        cmp     [DevErrorCode], 0
        je      @@End_11

        dec     ecx
        jz      .Error

        jmp     @@SendCommand
;--------------------------------------
align 4
@@:
        call    change_task
        ; Проверить код ошибки
        cmp     [DevErrorCode], 0
        je      @@End_11
        ; Проверить время ожидания готовности
        mov     eax, [timer_ticks]
        sub     eax, [WURStartTime]
        cmp     eax, MaxCDWaitTime
        jb      @@SendCommand
;--------------------------------------
.Error:
        ; Ошибка тайм-аута
        mov     [DevErrorCode], 1
;--------------------------------------
@@End_11:
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*            ЗАПРЕТИТЬ СМЕНУ ДИСКА              *
;* Входные параметры передаются через глобальные *
;* перменные:                                    *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
prevent_medium_removal:
        pusha
; Очистить буфер пакетной команды
        call    clear_packet_buffer
; Задать код команды
        mov     [PacketCommand], byte 0x1E
; Задать код запрета
        mov     [PacketCommand+4], byte 11b
; Подать команду
        call    SendPacketNoDatCommand
        mov     eax, ATAPI_IDE0_lock
        add     eax, [cdpos]
        dec     eax
        mov     [eax], byte 1
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*            РАЗРЕШИТЬ СМЕНУ ДИСКА              *
;* Входные параметры передаются через глобальные *
;* перменные:                                    *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
allow_medium_removal:
        pusha
; Очистить буфер пакетной команды
        call    clear_packet_buffer
; Задать код команды
        mov     [PacketCommand], byte 0x1E
; Задать код запрета
        mov     [PacketCommand+4], byte 0
; Подать команду
        call    SendPacketNoDatCommand
        mov     eax, ATAPI_IDE0_lock
        add     eax, [cdpos]
        dec     eax
        mov     [eax], byte 0
        popa
        ret
;-----------------------------------------------------------------------------
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
        call    clear_packet_buffer
; Сформировать команду START/STOP UNIT
        ; Задать код команды
        mov     [PacketCommand], word 0x1B
        ; Задать операцию загрузки носителя
        mov     [PacketCommand+4], word 00000011b
; Подать команду
        call    SendPacketNoDatCommand
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*         ИЗВЛЕЧЬ НОСИТЕЛЬ ИЗ ДИСКОВОДА         *
;* Входные параметры передаются через глобальные *
;* перменные:                                    *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
EjectMedium:
        pusha
; Очистить буфер пакетной команды
        call    clear_packet_buffer
; Сформировать команду START/STOP UNIT
        ; Задать код команды
        mov     [PacketCommand], word 0x1B
        ; Задать операцию извлечения носителя
        mov     [PacketCommand+4], word 00000010b
; Подать команду
        call    SendPacketNoDatCommand
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
;* Проверить событие нажатия кнопки извлечения   *
;*                     диска                     *
;* Входные параметры передаются через глобальные *
;* переменные:                                   *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
proc check_ATAPI_device_event_has_work?
        mov     eax, [timer_ticks]
        sub     eax, [timer_ATAPI_check]
        cmp     eax, 100
        jb      .no

        xor     eax, eax
        inc     eax
        ret
;--------------------------------------
.no:
        xor     eax, eax
        ret
endp
;-----------------------------------------------------------------------------
align 4
check_ATAPI_device_event:
        pusha
        mov     eax, [timer_ticks]
        sub     eax, [timer_ATAPI_check]
        cmp     eax, 100
        jb      .end_1

        pushfd
        mov     al, [DRIVE_DATA+1]
        and     al, 11b
        cmp     al, 10b
        jz      .ide3
;--------------------------------------
.ide2_1:
        mov     al, [DRIVE_DATA+1]
        and     al, 1100b
        cmp     al, 1000b
        jz      .ide2
;--------------------------------------
.ide1_1:
        mov     al, [DRIVE_DATA+1]
        and     al, 110000b
        cmp     al, 100000b
        jz      .ide1
;--------------------------------------
.ide0_1:
        mov     al, [DRIVE_DATA+1]
        and     al, 11000000b
        cmp     al, 10000000b
        jz      .ide0
;--------------------------------------
.ide7_1:
        mov     al, [DRIVE_DATA+6]
        and     al, 11b
        cmp     al, 10b
        jz      .ide7
;--------------------------------------
.ide6_1:
        mov     al, [DRIVE_DATA+6]
        and     al, 1100b
        cmp     al, 1000b
        jz      .ide6
;--------------------------------------
.ide5_1:
        mov     al, [DRIVE_DATA+6]
        and     al, 110000b
        cmp     al, 100000b
        jz      .ide5
;--------------------------------------
.ide4_1:
        mov     al, [DRIVE_DATA+6]
        and     al, 11000000b
        cmp     al, 10000000b
        jz      .ide4
;--------------------------------------
.ide11_1:
        mov     al, [DRIVE_DATA+11]
        and     al, 11b
        cmp     al, 10b
        jz      .ide11
;--------------------------------------
.ide10_1:
        mov     al, [DRIVE_DATA+11]
        and     al, 1100b
        cmp     al, 1000b
        jz      .ide10
;--------------------------------------
.ide9_1:
        mov     al, [DRIVE_DATA+11]
        and     al, 110000b
        cmp     al, 100000b
        jz      .ide9
;--------------------------------------
.ide8_1:
        mov     al, [DRIVE_DATA+11]
        and     al, 11000000b
        cmp     al, 10000000b
        jz      .ide8
;--------------------------------------
.end:
        popfd
        mov     eax, [timer_ticks]
        mov     [timer_ATAPI_check], eax
;--------------------------------------
.end_1:
        popa
        ret
;-----------------------------------------------------------------------------
.ide3:
        cli
        cmp     [ATAPI_IDE3_lock], 1
        jne     .ide2_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel2_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 1
        mov     [cdpos], 4
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide2_1
;-----------------------------------------------------------------------------
.ide2:
        cli
        cmp     [ATAPI_IDE2_lock], 1
        jne     .ide1_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel2_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 0
        mov     [cdpos], 3
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide1_1
;-----------------------------------------------------------------------------
.ide1:
        cli
        cmp     [ATAPI_IDE1_lock], 1
        jne     .ide0_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel1_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 1
        mov     [cdpos], 2
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide0_1
;-----------------------------------------------------------------------------
.ide0:
        cli
        cmp     [ATAPI_IDE0_lock], 1
        jne     .ide7_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel1_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 0
        mov     [cdpos], 1
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide7_1
;-----------------------------------------------------------------------------
.ide7:
        cli
        cmp     [ATAPI_IDE7_lock], 1
        jne     .ide6_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel4_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 1
        mov     [cdpos], 8
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide6_1
;-----------------------------------------------------------------------------
.ide6:
        cli
        cmp     [ATAPI_IDE6_lock], 1
        jne     .ide5_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel4_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 0
        mov     [cdpos], 7
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide5_1
;-----------------------------------------------------------------------------
.ide5:
        cli
        cmp     [ATAPI_IDE5_lock], 1
        jne     .ide4_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel3_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 1
        mov     [cdpos], 6
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide4_1
;-----------------------------------------------------------------------------
.ide4:
        cli
        cmp     [ATAPI_IDE4_lock], 1
        jne     .ide11_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel3_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 0
        mov     [cdpos], 5
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide11_1
;-----------------------------------------------------------------------------
.ide11:
        cli
        cmp     [ATAPI_IDE11_lock], 1
        jne     .ide10_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel6_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 1
        mov     [cdpos], 12
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide10_1
;-----------------------------------------------------------------------------
.ide10:
        cli
        cmp     [ATAPI_IDE10_lock], 1
        jne     .ide9_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel6_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 2
        mov     [DiskNumber], 0
        mov     [cdpos], 11
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide9_1
;-----------------------------------------------------------------------------
.ide9:
        cli
        cmp     [ATAPI_IDE9_lock], 1
        jne     .ide8_1

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel5_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 1
        mov     [cdpos], 10
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .ide8_1
;-----------------------------------------------------------------------------
.ide8:
        cli
        cmp     [ATAPI_IDE8_lock], 1
        jne     .end

        cmp     [cd_status], 0
        jne     .end

        mov     ecx, ide_channel5_mutex
        call    mutex_lock
        call    reserve_ok2
        mov     [ChannelNumber], 1
        mov     [DiskNumber], 0
        mov     [cdpos], 9
        call    GetEvent_StatusNotification
        cmp     [CDDataBuf+4], byte 1
        jne     @f

        call    .eject
;--------------------------------------
@@:
        call    syscall_cdaudio.free
        jmp     .end
;-----------------------------------------------------------------------------
.eject:
        call    clear_CD_cache
        call    allow_medium_removal
        mov     [ignore_CD_eject_wait], 1
        call    EjectMedium
        mov     [ignore_CD_eject_wait], 0
        ret
;-----------------------------------------------------------------------------
iglobal
timer_ATAPI_check dd 0
ATAPI_IDE0_lock db 0
ATAPI_IDE1_lock db 0
ATAPI_IDE2_lock db 0
ATAPI_IDE3_lock db 0
ATAPI_IDE4_lock db 0
ATAPI_IDE5_lock db 0
ATAPI_IDE6_lock db 0
ATAPI_IDE7_lock db 0
ATAPI_IDE8_lock db 0
ATAPI_IDE9_lock db 0
ATAPI_IDE10_lock db 0
ATAPI_IDE11_lock db 0
ignore_CD_eject_wait db 0
endg
;-----------------------------------------------------------------------------
;*************************************************
;* Получить сообщение о событии или состоянии    *
;*                  устройства                   *
;* Входные параметры передаются через глобальные *
;* переменные:                                   *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
GetEvent_StatusNotification:
        pusha
        mov     [CDDataBuf_pointer], CDDataBuf
; Очистить буфер пакетной команды
        call    clear_packet_buffer
; Задать код команды
        mov     [PacketCommand], byte 4Ah
        mov     [PacketCommand+1], byte 00000001b
; Задать запрос класса сообщений
        mov     [PacketCommand+4], byte 00010000b
; Размер выделенной области
        mov     [PacketCommand+7], byte 8h
        mov     [PacketCommand+8], byte 0h
; Подать команду
        call    SendPacketDatCommand
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
; прочитать информацию из TOC
;* Входные параметры передаются через глобальные *
;* переменные:                                   *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
Read_TOC:
        pusha
        mov     [CDDataBuf_pointer], CDDataBuf
; Очистить буфер пакетной команды
        call    clear_packet_buffer
; Сформировать пакетную команду для считывания
; сектора данных
        mov     [PacketCommand], byte 0x43
        ; Задать формат
        mov     [PacketCommand+2], byte 1
; Размер выделенной области
        mov     [PacketCommand+7], byte 0xFF
        mov     [PacketCommand+8], byte 0h
; Подать команду
        call    SendPacketDatCommand
        popa
        ret
;-----------------------------------------------------------------------------
;*************************************************
;* ОПРЕДЕЛИТЬ ОБЩЕЕ КОЛИЧЕСТВО СЕКТОРОВ НА ДИСКЕ *
;* Входные параметры передаются через глобальные *
;* переменные:                                   *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;*************************************************
;ReadCapacity:
;       pusha
;; Очистить буфер пакетной команды
;       call  clear_packet_buffer
;; Задать размер буфера в байтах
;       mov     [CDBlockSize],8
;; Сформировать команду READ CAPACITY
;       mov     [PacketCommand],word 25h
;; Подать команду
;       call    SendPacketDatCommand
;       popa
;       ret
;-----------------------------------------------------------------------------
clear_packet_buffer:
; Очистить буфер пакетной команды
        and     [PacketCommand], dword 0
        and     [PacketCommand+4], dword 0
        and     [PacketCommand+8], dword 0
        ret
;-----------------------------------------------------------------------------
