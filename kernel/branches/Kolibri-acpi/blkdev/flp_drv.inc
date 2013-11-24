;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2013. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;**********************************************************
;  Непосредственная работа с контроллером гибкого диска
;**********************************************************
; Автор исходного текста  Кулаков Владимир Геннадьевич.
; Адаптация и доработка Mario79

;give_back_application_data:  ; переслать приложению
;     mov edi,[TASK_BASE]
;     mov edi,[edi+TASKDATA.mem_start]
;     add edi,ecx
give_back_application_data_1:
        mov     esi, FDD_BUFF;FDD_DataBuffer  ;0x40000
        mov     ecx, 128
        cld
        rep movsd
        ret

;take_data_from_application:   ; взять из приложени
;     mov esi,[TASK_BASE]
;     mov esi,[esi+TASKDATA.mem_start]
;     add esi,ecx
take_data_from_application_1:
        mov     edi, FDD_BUFF;FDD_DataBuffer  ;0x40000
        mov     ecx, 128
        cld
        rep movsd
        ret

; Коды завершения операции с контроллером (FDC_Status)
FDC_Normal         equ 0 ;нормальное завершение
FDC_TimeOut        equ 1 ;ошибка тайм-аута
FDC_DiskNotFound   equ 2 ;в дисководе нет диска
FDC_TrackNotFound  equ 3 ;дорожка не найдена
FDC_SectorNotFound equ 4 ;сектор не найден

; Максимальные значения координат сектора (заданные
; значения соответствуют параметрам стандартного
; трехдюймового гибкого диска объемом 1,44 Мб)
MAX_Track   equ 79
MAX_Head    equ  1
MAX_Sector  equ 18

uglobal
; Счетчик тиков таймера
TickCounter dd ?
; Код завершения операции с контроллером НГМД
FDC_Status  DB ?
; Флаг прерывания от НГМД
FDD_IntFlag DB ?
; Момент начала последней операции с НГМД
FDD_Time    DD ?
; Номер дисковода
FDD_Type    db 0
; Координаты сектора
FDD_Track   DB ?
FDD_Head    DB ?
FDD_Sector  DB ?

; Блок результата операции
FDC_ST0 DB ?
FDC_ST1 DB ?
FDC_ST2 DB ?
FDC_C   DB ?
FDC_H   DB ?
FDC_R   DB ?
FDC_N   DB ?
; Счетчик повторения операции чтени
ReadRepCounter  DB ?
; Счетчик повторения операции рекалибровки
RecalRepCounter DB ?
endg
; Область памяти для хранения прочитанного сектора
;FDD_DataBuffer:  times 512 db 0   ;DB 512 DUP (?)
fdd_motor_status db 0
timer_fdd_motor  dd 0

;*************************************
;* ИНИЦИАЛИЗАЦИЯ РЕЖИМА ПДП ДЛЯ НГМД *
;*************************************
Init_FDC_DMA:
        pushad
        mov     al, 0
        out     0x0c, al; reset the flip-flop to a known state.
        mov     al, 6           ; mask channel 2 so we can reprogram it.
        out     0x0a, al
        mov     al, [dmamode]; 0x46 -> Read from floppy - 0x4A Write to floppy
        out     0x0b, al
        mov     al, 0
        out     0x0c, al; reset the flip-flop to a known state.
        mov     eax, 0xD000
        out     0x04, al; set the channel 2 starting address to 0
        shr     eax, 8
        out     0x04, al
        shr     eax, 8
        out     0x81, al
        mov     al, 0
        out     0x0c, al; reset flip-flop
        mov     al, 0xff;set count (actual size -1)
        out     0x5, al
        mov     al, 0x1;[dmasize]       ;(0x1ff = 511 / 0x23ff =9215)
        out     0x5, al
        mov     al, 2
        out     0xa, al
        popad
        ret

;***********************************
;* ЗАПИСАТЬ БАЙТ В ПОРТ ДАННЫХ FDC *
;* Параметры:                      *
;* AL - выводимый байт.            *
;***********************************
FDCDataOutput:
;       DEBUGF 1,'K : FDCDataOutput(%x)',al
;        pusha
        push    eax ecx edx
        mov     AH, AL    ;запомнить байт в AH
; Сбросить переменную состояния контроллера
        mov     [FDC_Status], FDC_Normal
; Проверить готовность контроллера к приему данных
        mov     DX, 3F4h  ;(порт состояния FDC)
        mov     ecx, 0x10000 ;установить счетчик тайм-аута
@@TestRS:
        in      AL, DX    ;прочитать регистр RS
        and     AL, 0C0h  ;выделить разряды 6 и 7
        cmp     AL, 80h   ;проверить разряды 6 и 7
        je      @@OutByteToFDC
        loop    @@TestRS
; Ошибка тайм-аута
;       DEBUGF 1,' timeout\n'
        mov     [FDC_Status], FDC_TimeOut
        jmp     @@End_5
; Вывести байт в порт данных
@@OutByteToFDC:
        inc     DX
        mov     AL, AH
        out     DX, AL
;        DEBUGF 1,' ok\n'
@@End_5:
;        popa
        pop     edx ecx eax
        ret

;******************************************
;*   ПРОЧИТАТЬ БАЙТ ИЗ ПОРТА ДАННЫХ FDC   *
;* Процедура не имеет входных параметров. *
;* Выходные данные:                       *
;* AL - считанный байт.                   *
;******************************************
FDCDataInput:
        push    ECX
        push    DX
; Сбросить переменную состояния контроллера
        mov     [FDC_Status], FDC_Normal
; Проверить готовность контроллера к передаче данных
        mov     DX, 3F4h  ;(порт состояния FDC)
        mov     ecx, 0x10000 ;установить счетчик тайм-аута
@@TestRS_1:
        in      AL, DX    ;прочитать регистр RS
        and     AL, 0C0h  ;выдлить разряды 6 и 7
        cmp     AL, 0C0h  ;проверить разряды 6 и 7
        je      @@GetByteFromFDC
        loop    @@TestRS_1
; Ошибка тайм-аута
;       DEBUGF 1,'K : FDCDataInput: timeout\n'
        mov     [FDC_Status], FDC_TimeOut
        jmp     @@End_6
; Ввести байт из порта данных
@@GetByteFromFDC:
        inc     DX
        in      AL, DX
;       DEBUGF 1,'K : FDCDataInput: %x\n',al
@@End_6:
        pop     DX
        pop     ECX
        ret

;*********************************************
;* ОБРАБОТЧИК ПРЕРЫВАНИЯ ОТ КОНТРОЛЛЕРА НГМД *
;*********************************************
FDCInterrupt:
;       dbgstr 'FDCInterrupt'
; Установить флаг прерывания
        mov     [FDD_IntFlag], 1
        mov     al, 1
        ret

;*******************************************
;* ОЖИДАНИЕ ПРЕРЫВАНИЯ ОТ КОНТРОЛЛЕРА НГМД *
;*******************************************
WaitFDCInterrupt:
        pusha
; Сбросить байт состояния операции
        mov     [FDC_Status], FDC_Normal
; Обнулить счетчик тиков
        mov     eax, [timer_ticks]
        mov     [TickCounter], eax
; Ожидать установки флага прерывания НГМД
@@TestRS_2:
        call    change_task
        cmp     [FDD_IntFlag], 0
        jnz     @@End_7           ;прерывание произошло
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter]
        cmp     eax, 200;50 ;25   ;5 ;ожидать 5 тиков
        jb      @@TestRS_2
;        jl      @@TestRS_2
; Ошибка тайм-аута
;       dbgstr 'WaitFDCInterrupt: timeout'
        mov     [FDC_Status], FDC_TimeOut
@@End_7:
        popa
        ret

;*********************************
;* ВКЛЮЧИТЬ МОТОР ДИСКОВОДА "A:" *
;*********************************
FDDMotorON:
;       dbgstr 'FDDMotorON'
        pusha
;        cmp     [fdd_motor_status],1
;        je      fdd_motor_on
        mov     al, [flp_number]
        cmp     [fdd_motor_status], al
        je      fdd_motor_on
; Произвести сброс контроллера НГМД
        mov     DX, 3F2h;порт управления двигателями
        mov     AL, 0
        out     DX, AL
; Выбрать и включить мотор дисковода
        cmp     [flp_number], 1
        jne     FDDMotorON_B
;        call    FDDMotorOFF_B
        mov     AL, 1Ch   ; Floppy A
        jmp     FDDMotorON_1
FDDMotorON_B:
;        call    FDDMotorOFF_A
        mov     AL, 2Dh   ; Floppy B
FDDMotorON_1:
        out     DX, AL
; Обнулить счетчик тиков
        mov     eax, [timer_ticks]
        mov     [TickCounter], eax
; Ожидать 0,5 с
@@dT:
        call    change_task
        mov     eax, [timer_ticks]
        sub     eax, [TickCounter]
        cmp     eax, 50 ;10
        jb      @@dT
; Read results of RESET command
        push    4
;       DEBUGF 1,'K : floppy reset results:'
@@:
        mov     al, 8
        call    FDCDataOutput
        call    FDCDataInput
;       DEBUGF 1,' %x',al
        call    FDCDataInput
;       DEBUGF 1,' %x',al
        dec     dword [esp]
        jnz     @b
;       DEBUGF 1,'\n'
        pop     eax
        cmp     [flp_number], 1
        jne     fdd_motor_on_B
        mov     [fdd_motor_status], 1
        jmp     fdd_motor_on
fdd_motor_on_B:
        mov     [fdd_motor_status], 2
fdd_motor_on:
        call    save_timer_fdd_motor
        popa
        ret

;*****************************************
;*  СОХРАНЕНИЕ УКАЗАТЕЛЯ ВРЕМЕНИ         *
;*****************************************
save_timer_fdd_motor:
        mov     eax, [timer_ticks]
        mov     [timer_fdd_motor], eax
        ret

;*****************************************
;*  ПРОВЕРКА ЗАДЕРЖКИ ВЫКЛЮЧЕНИЯ МОТОРА  *
;*****************************************
proc check_fdd_motor_status_has_work?
        cmp     [fdd_motor_status], 0
        jz      .no
        mov     eax, [timer_ticks]
        sub     eax, [timer_fdd_motor]
        cmp     eax, 500
        jb      .no
.yes:
        xor     eax, eax
        inc     eax
        ret
.no:
        xor     eax, eax
        ret
endp

align 4
check_fdd_motor_status:
        cmp     [fdd_motor_status], 0
        je      end_check_fdd_motor_status_1
        mov     eax, [timer_ticks]
        sub     eax, [timer_fdd_motor]
        cmp     eax, 500
        jb      end_check_fdd_motor_status
        call    FDDMotorOFF
        mov     [fdd_motor_status], 0
end_check_fdd_motor_status_1:
end_check_fdd_motor_status:
        ret

;**********************************
;* ВЫКЛЮЧИТЬ МОТОР ДИСКОВОДА      *
;**********************************
FDDMotorOFF:
;       dbgstr 'FDDMotorOFF'
        push    AX
        push    DX
        cmp     [flp_number], 1
        jne     FDDMotorOFF_1
        call    FDDMotorOFF_A
        jmp     FDDMotorOFF_2
FDDMotorOFF_1:
        call    FDDMotorOFF_B
FDDMotorOFF_2:
        pop     DX
        pop     AX
        ; сброс флагов кеширования в связи с устареванием информации
        or      [floppy_media_flags+0], FLOPPY_MEDIA_NEED_RESCAN
        or      [floppy_media_flags+1], FLOPPY_MEDIA_NEED_RESCAN
        ret

FDDMotorOFF_A:
        mov     DX, 3F2h;порт управления двигателями
        mov     AL, 0Ch ; Floppy A
        out     DX, AL
        ret

FDDMotorOFF_B:
        mov     DX, 3F2h;порт управления двигателями
        mov     AL, 5h ; Floppy B
        out     DX, AL
        ret

;*******************************
;* РЕКАЛИБРОВКА ДИСКОВОДА "A:" *
;*******************************
RecalibrateFDD:
;       dbgstr 'RecalibrateFDD'
        pusha
        call    save_timer_fdd_motor
; Сбросить флаг прерывания
        mov     [FDD_IntFlag], 0
; Подать команду "Рекалибровка"
        mov     AL, 07h
        call    FDCDataOutput
        mov     AL, 00h
        call    FDCDataOutput
; Ожидать завершения операции
        call    WaitFDCInterrupt
        cmp     [FDC_Status], 0
        jne     .fail
; Read results of RECALIBRATE command
;       DEBUGF 1,'K : floppy recalibrate results:'
        mov     al, 8
        call    FDCDataOutput
        call    FDCDataInput
;       DEBUGF 1,' %x',al
        call    FDCDataInput
;       DEBUGF 1,' %x',al
;       DEBUGF 1,'\n'
.fail:
        call    save_timer_fdd_motor
        popa
        ret

;*****************************************************
;*                    ПОИСК ДОРОЖКИ                  *
;* Параметры передаются через глобальные переменные: *
;* FDD_Track - номер дорожки (0-79);                 *
;* FDD_Head - номер головки (0-1).                   *
;* Результат операции заносится в FDC_Status.        *
;*****************************************************
SeekTrack:
;       dbgstr 'SeekTrack'
        pusha
        call    save_timer_fdd_motor
; Сбросить флаг прерывания
        mov     [FDD_IntFlag], 0
; Подать команду "Поиск"
        mov     AL, 0Fh
        call    FDCDataOutput
        ; Передать байт номера головки/накопител
        mov     AL, [FDD_Head]
        shl     AL, 2
        call    FDCDataOutput
        ; Передать байт номера дорожки
        mov     AL, [FDD_Track]
        call    FDCDataOutput
; Ожидать завершения операции
        call    WaitFDCInterrupt
        cmp     [FDC_Status], FDC_Normal
        jne     @@Exit
; Сохранить результат поиска
        mov     AL, 08h
        call    FDCDataOutput
        call    FDCDataInput
        mov     [FDC_ST0], AL
        call    FDCDataInput
        mov     [FDC_C], AL
; Проверить результат поиска
        ; Поиск завершен?
        test    [FDC_ST0], 100000b
        je      @@Err
        ; Заданный трек найден?
        mov     AL, [FDC_C]
        cmp     AL, [FDD_Track]
        jne     @@Err
        ; Номер головки совпадает с заданным?
; The H bit (Head Address) in ST0 will always return a "0" (c) 82077AA datasheet,
; description of SEEK command. So we can not verify the proper head.
;        mov     AL, [FDC_ST0]
;        and     AL, 100b
;        shr     AL, 2
;        cmp     AL, [FDD_Head]
;        jne     @@Err
        ; Операция завершена успешно
;        dbgstr 'SeekTrack: FDC_Normal'
        mov     [FDC_Status], FDC_Normal
        jmp     @@Exit
@@Err:  ; Трек не найден
;       dbgstr 'SeekTrack: FDC_TrackNotFound'
        mov     [FDC_Status], FDC_TrackNotFound
@@Exit:
        call    save_timer_fdd_motor
        popa
        ret

;*******************************************************
;*               ЧТЕНИЕ СЕКТОРА ДАННЫХ                 *
;* Параметры передаются через глобальные переменные:   *
;* FDD_Track - номер дорожки (0-79);                   *
;* FDD_Head - номер головки (0-1);                     *
;* FDD_Sector - номер сектора (1-18).                  *
;* Результат операции заносится в FDC_Status.          *
;* В случае успешного выполнения операции чтения       *
;* содержимое сектора будет занесено в FDD_DataBuffer. *
;*******************************************************
ReadSector:
;       dbgstr 'ReadSector'
        pushad
        call    save_timer_fdd_motor
; Сбросить флаг прерывания
        mov     [FDD_IntFlag], 0
; Установить скорость передачи 500 Кбайт/с
        mov     AX, 0
        mov     DX, 03F7h
        out     DX, AL
; Инициализировать канал прямого доступа к памяти
        mov     [dmamode], 0x46
        call    Init_FDC_DMA
; Подать команду "Чтение данных"
        mov     AL, 0E6h ;чтение в мультитрековом режиме
        call    FDCDataOutput
        mov     AL, [FDD_Head]
        shl     AL, 2
        call    FDCDataOutput
        mov     AL, [FDD_Track]
        call    FDCDataOutput
        mov     AL, [FDD_Head]
        call    FDCDataOutput
        mov     AL, [FDD_Sector]
        call    FDCDataOutput
        mov     AL, 2   ;код размера сектора (512 байт)
        call    FDCDataOutput
        mov     AL, 18 ;+1; 3Fh  ;число секторов на дорожке
        call    FDCDataOutput
        mov     AL, 1Bh ;значение GPL
        call    FDCDataOutput
        mov     AL, 0FFh;значение DTL
        call    FDCDataOutput
; Ожидаем прерывание по завершении операции
        call    WaitFDCInterrupt
        cmp     [FDC_Status], FDC_Normal
        jne     @@Exit_1
; Считываем статус завершения операции
        call    GetStatusInfo
        test    [FDC_ST0], 11011000b
        jnz     @@Err_1
;        dbgstr 'ReadSector: FDC_Normal'
        mov     [FDC_Status], FDC_Normal
        jmp     @@Exit_1
@@Err_1:
;       dbgstr 'ReadSector: FDC_SectorNotFound'
        mov     [FDC_Status], FDC_SectorNotFound
@@Exit_1:
        call    save_timer_fdd_motor
        popad
        ret

;*******************************************************
;*   ЧТЕНИЕ СЕКТОРА (С ПОВТОРЕНИЕМ ОПЕРАЦИИ ПРИ СБОЕ)  *
;* Параметры передаются через глобальные переменные:   *
;* FDD_Track - номер дорожки (0-79);                   *
;* FDD_Head - номер головки (0-1);                     *
;* FDD_Sector - номер сектора (1-18).                  *
;* Результат операции заносится в FDC_Status.          *
;* В случае успешного выполнения операции чтения       *
;* содержимое сектора будет занесено в FDD_DataBuffer. *
;*******************************************************
ReadSectWithRetr:
        pusha
; Обнулить счетчик повторения операции рекалибровки
        mov     [RecalRepCounter], 0
@@TryAgain:
; Обнулить счетчик повторения операции чтени
        mov     [ReadRepCounter], 0
@@ReadSector_1:
        call    ReadSector
        cmp     [FDC_Status], 0
        je      @@Exit_2
        cmp     [FDC_Status], 1
        je      @@Err_3
        ; Троекратное повторение чтени
        inc     [ReadRepCounter]
        cmp     [ReadRepCounter], 3
        jb      @@ReadSector_1
        ; Троекратное повторение рекалибровки
        call    RecalibrateFDD
        call    SeekTrack
        inc     [RecalRepCounter]
        cmp     [RecalRepCounter], 3
        jb      @@TryAgain
@@Exit_2:
        popa
        ret
@@Err_3:
        popa
        ret

;*******************************************************
;*               ЗАПИСЬ СЕКТОРА ДАННЫХ                 *
;* Параметры передаются через глобальные переменные:   *
;* FDD_Track - номер дорожки (0-79);                   *
;* FDD_Head - номер головки (0-1);                     *
;* FDD_Sector - номер сектора (1-18).                  *
;* Результат операции заносится в FDC_Status.          *
;* В случае успешного выполнения операции записи       *
;* содержимое FDD_DataBuffer будет занесено в сектор.  *
;*******************************************************
WriteSector:
;       dbgstr 'WriteSector'
        pushad
        call    save_timer_fdd_motor
; Сбросить флаг прерывания
        mov     [FDD_IntFlag], 0
; Установить скорость передачи 500 Кбайт/с
        mov     AX, 0
        mov     DX, 03F7h
        out     DX, AL
; Инициализировать канал прямого доступа к памяти
        mov     [dmamode], 0x4A
        call    Init_FDC_DMA
; Подать команду "Запись данных"
        mov     AL, 0xC5 ;0x45  ;запись в мультитрековом режиме
        call    FDCDataOutput
        mov     AL, [FDD_Head]
        shl     AL, 2
        call    FDCDataOutput
        mov     AL, [FDD_Track]
        call    FDCDataOutput
        mov     AL, [FDD_Head]
        call    FDCDataOutput
        mov     AL, [FDD_Sector]
        call    FDCDataOutput
        mov     AL, 2   ;код размера сектора (512 байт)
        call    FDCDataOutput
        mov     AL, 18; 3Fh  ;число секторов на дорожке
        call    FDCDataOutput
        mov     AL, 1Bh ;значение GPL
        call    FDCDataOutput
        mov     AL, 0FFh;значение DTL
        call    FDCDataOutput
; Ожидаем прерывание по завершении операции
        call    WaitFDCInterrupt
        cmp     [FDC_Status], FDC_Normal
        jne     @@Exit_3
; Считываем статус завершения операции
        call    GetStatusInfo
        test    [FDC_ST0], 11000000b ;11011000b
        jnz     @@Err_2
        mov     [FDC_Status], FDC_Normal
        jmp     @@Exit_3
@@Err_2:
        mov     [FDC_Status], FDC_SectorNotFound
@@Exit_3:
        call    save_timer_fdd_motor
        popad
        ret

;*******************************************************
;*   ЗАПИСЬ СЕКТОРА (С ПОВТОРЕНИЕМ ОПЕРАЦИИ ПРИ СБОЕ)  *
;* Параметры передаются через глобальные переменные:   *
;* FDD_Track - номер дорожки (0-79);                   *
;* FDD_Head - номер головки (0-1);                     *
;* FDD_Sector - номер сектора (1-18).                  *
;* Результат операции заносится в FDC_Status.          *
;* В случае успешного выполнения операции записи       *
;* содержимое FDD_DataBuffer будет занесено в сектор.  *
;*******************************************************
WriteSectWithRetr:
        pusha
; Обнулить счетчик повторения операции рекалибровки
        mov     [RecalRepCounter], 0
@@TryAgain_1:
; Обнулить счетчик повторения операции чтени
        mov     [ReadRepCounter], 0
@@WriteSector_1:
        call    WriteSector
        cmp     [FDC_Status], 0
        je      @@Exit_4
        cmp     [FDC_Status], 1
        je      @@Err_4
        ; Троекратное повторение чтени
        inc     [ReadRepCounter]
        cmp     [ReadRepCounter], 3
        jb      @@WriteSector_1
        ; Троекратное повторение рекалибровки
        call    RecalibrateFDD
        call    SeekTrack
        inc     [RecalRepCounter]
        cmp     [RecalRepCounter], 3
        jb      @@TryAgain_1
@@Exit_4:
        popa
        ret
@@Err_4:
        popa
        ret

;*********************************************
;* ПОЛУЧИТЬ ИНФОРМАЦИЮ О РЕЗУЛЬТАТЕ ОПЕРАЦИИ *
;*********************************************
GetStatusInfo:
        push    AX
        call    FDCDataInput
        mov     [FDC_ST0], AL
        call    FDCDataInput
        mov     [FDC_ST1], AL
        call    FDCDataInput
        mov     [FDC_ST2], AL
        call    FDCDataInput
        mov     [FDC_C], AL
        call    FDCDataInput
        mov     [FDC_H], AL
        call    FDCDataInput
        mov     [FDC_R], AL
        call    FDCDataInput
        mov     [FDC_N], AL
        pop     AX
        ret

; Interface for disk subsystem.
; Assume fixed capacity for 1.44M.
FLOPPY_CAPACITY = 2880  ; in sectors

iglobal
align 4
floppy_functions:
        dd      .size
        dd      0       ; no close() function
        dd      0       ; no closemedia() function
        dd      floppy_querymedia
        dd      floppy_read
        dd      floppy_write
        dd      0       ; no flush() function
        dd      0       ; no adjust_cache_size() function
.size = $ - floppy_functions
endg

uglobal
floppy_media_flags      rb      2
n_sector    dd 0  ; temporary save for sector value
flp_number  db 0  ; 1- Floppy A, 2-Floppy B
old_track   db 0  ; old value track
flp_label   rb 15*2 ; Label and ID of inserted floppy disk
align 4
; Hardware does not allow to work with two floppies in parallel,
; so there is one mutex guarding access to any floppy.
floppy_mutex    MUTEX
endg
; Meaning of bits in floppy_media_flags
FLOPPY_MEDIA_PRESENT = 1        ; media was present when last asked
FLOPPY_MEDIA_NEED_RESCAN = 2    ; media was possibly changed, need to rescan
FLOPPY_MEDIA_LABEL_CHANGED = 4  ; temporary state

iglobal
floppy1_name    db      'fd',0
floppy2_name    db      'fd2',0
endg

; This function is called in boot process.
; It creates filesystems /fd and/or /fd2, if the system has one/two floppy drives.
proc floppy_init
        mov     ecx, floppy_mutex
        call    mutex_init
; First floppy is present if [DRIVE_DATA] and 0xF0 is nonzero.
        test    byte [DRIVE_DATA], 0xF0
        jz      .no1
        stdcall disk_add, floppy_functions, floppy1_name, 1, DISK_NO_INSERT_NOTIFICATION
.no1:
; Second floppy is present if [DRIVE_DATA] and 0x0F is nonzero.
        test    byte [DRIVE_DATA], 0x0F
        jz      .no2
        stdcall disk_add, floppy_functions, floppy2_name, 2, DISK_NO_INSERT_NOTIFICATION
.no2:
        ret
endp

; Returns information about disk media.
; Floppy drives do not support insert notifications,
; DISK_NO_INSERT_NOTIFICATION is set,
; the disk subsystem calls this function before each filesystem operation.
; If the media has changed, return error for the first call as signal
; to finalize work with old media and the true geometry for the second call.
; Assume that media is (possibly) changed anytime when motor is off.
proc floppy_querymedia
  virtual at esp+4
    .userdata dd ?
    .info dd ?
  end virtual
; 1. Acquire the global lock.
        mov     ecx, floppy_mutex
        call    mutex_lock
        mov     edx, [.userdata]        ; 1 for /fd, 2 for /fd2
; 2. If the media was reported and has been changed, forget it and report an error.
        mov     al, [floppy_media_flags+edx-1]
        and     al, FLOPPY_MEDIA_PRESENT + FLOPPY_MEDIA_NEED_RESCAN
        cmp     al, FLOPPY_MEDIA_PRESENT + FLOPPY_MEDIA_NEED_RESCAN
        jnz     .not_reported
.no_media:
        mov     [floppy_media_flags+edx-1], 0
.return_no_media:
        mov     ecx, floppy_mutex
        call    mutex_unlock
        mov     eax, DISK_STATUS_NO_MEDIA
        retn    8
.not_reported:
; 3. If we are in the temporary state LABEL_CHANGED, this is the second call
; after intermediate DISK_STATUS_NO_MEDIA due to media change;
; clear the flag and return the current geometry without rereading the bootsector.
        cmp     [floppy_media_flags+edx-1], FLOPPY_MEDIA_LABEL_CHANGED
        jz      .report_geometry
; 4. Try to read the bootsector.
        mov     [flp_number], dl
        mov     [FDC_Status], 0
        call    floppy_read_bootsector
; 5. If reading bootsector failed, assume that media is not present.
        mov     edx, [.userdata]
        cmp     [FDC_Status], 0
        jnz     .no_media
; 6. Check whether the previous status is "present". If not, go to 10.
        push    esi edi
        imul    edi, edx, 15
        add     edi, flp_label-15
        mov     esi, FDD_BUFF+39
        mov     ecx, 15
        test    [floppy_media_flags+edx-1], FLOPPY_MEDIA_PRESENT
        jz      .set_label
; 7. Compare the old label with the current one.
        rep cmpsb
; 8. If the label has not changed, go to 11.
        jz      .ok
; 9. If the label has changed, store it, enter temporary state LABEL_CHANGED
; and report DISK_STATUS_NO_MEDIA.
;       dbgstr 'floppy label changed'
        add     esi, ecx
        add     edi, ecx
        mov     ecx, 15
        sub     esi, ecx
        sub     edi, ecx
        rep movsb
        mov     [floppy_media_flags+edx-1], FLOPPY_MEDIA_LABEL_CHANGED
        pop     edi esi
        jmp     .return_no_media
.set_label:
; 10. The previous state was "not present". Copy the label.
        rep movsb
.ok:
        pop     edi esi
.report_geometry:
; 11. Fill DISKMEDIAINFO structure.
        mov     ecx, [.info]
        and     [ecx+DISKMEDIAINFO.Flags], 0
        mov     [ecx+DISKMEDIAINFO.SectorSize], 512
        mov     dword [ecx+DISKMEDIAINFO.Capacity], FLOPPY_CAPACITY
        and     dword [ecx+DISKMEDIAINFO.Capacity+4], 0
; 12. Update state: media is present, data are actual.
        mov     [floppy_media_flags+edx-1], FLOPPY_MEDIA_PRESENT
; 13. Release the global lock and return successful status.
        mov     ecx, floppy_mutex
        call    mutex_unlock
        xor     eax, eax
        retn    8
endp

proc floppy_read_bootsector
        pushad
        mov     [FDD_Track], 0; Цилиндр
        mov     [FDD_Head], 0; Сторона
        mov     [FDD_Sector], 1; Сектор
        call    FDDMotorON
        call    RecalibrateFDD
        cmp     [FDC_Status], 0
        jne     .nothing
        call    SeekTrack
        cmp     [FDC_Status], 0
        jne     .nothing
        call    ReadSectWithRetr
.nothing:
        popad
        ret
endp

read_chs_sector:
        call    calculate_chs
        call    ReadSectWithRetr
        ret

save_chs_sector:
        call    calculate_chs
        call    WriteSectWithRetr
        ret

calculate_chs:
        mov     bl, [FDD_Track]
        mov     [old_track], bl
        mov     ebx, 18
        xor     edx, edx
        div     ebx
        inc     edx
        mov     [FDD_Sector], dl
        mov     edx, eax
        shr     eax, 1
        and     edx, 1
        mov     [FDD_Track], al
        mov     [FDD_Head], dl
        mov     dl, [old_track]
        cmp     dl, [FDD_Track]
        je      no_seek_track_1
        call    SeekTrack
no_seek_track_1:
        ret

; Writes one or more sectors to the device.
proc floppy_write
        mov     dl, 1
        jmp     floppy_read_write
endp

; Reads one or more sectors from the device.
proc floppy_read
        mov     dl, 0
endp

; Common part of floppy_read and floppy_write.
proc floppy_read_write userdata:dword, buffer:dword, start_sector:qword, numsectors_ptr:dword
virtual at ebp-8
.sectors_todo   dd      ?
.operation      db      ?
end virtual
        push    edx             ; save operation code to [.operation]
; 1. Get number of sectors to read/write
; and zero number of sectors that were actually read/written.
        mov     eax, [numsectors_ptr]
        push    dword [eax]     ; initialize [.sectors_todo]
        and     dword [eax], 0
        push    ebx esi edi     ; save used registers to be stdcall
; 2. Acquire the global lock.
        mov     ecx, floppy_mutex
        call    mutex_lock
; 3. Set floppy number for this operation.
        mov     edx, [userdata]
        mov     [flp_number], dl
; 4. Read/write sector-by-sector.
.operation_loop:
; 4a. Check that the sector is inside the media.
        cmp     dword [start_sector+4], 0
        jnz     .end_of_media
        mov     eax, dword [start_sector]
        cmp     eax, FLOPPY_CAPACITY
        jae     .end_of_media
; 4b. For read operation, call read_chs_sector and then move data from FDD_BUFF to [buffer].
; For write operation, move data from [buffer] to FDD_BUFF and then call save_chs_sector.
        cmp     [.operation], 0
        jz      .read
        mov     esi, [buffer]
        mov     edi, FDD_BUFF
        mov     ecx, 512/4
        rep movsd
        mov     [buffer], esi
        call    save_chs_sector
        jmp     @f
.read:
        call    read_chs_sector
        mov     esi, FDD_BUFF
        mov     edi, [buffer]
        mov     ecx, 512/4
        rep movsd
        mov     [buffer], edi
@@:
; 4c. If there was an error, propagate it to the caller.
        cmp     [FDC_Status], 0
        jnz     .fail
; 4d. Otherwise, increment number of sectors processed and continue the loop.
        mov     eax, [numsectors_ptr]
        inc     dword [eax]
        inc     dword [start_sector]
        dec     [.sectors_todo]
        jnz     .operation_loop
; 5. Release the global lock and return with the correct status.
        push    0
.return:
        mov     ecx, floppy_mutex
        call    mutex_unlock
        pop     eax
        pop     edi esi ebx     ; restore used registers to be stdcall
        ret     ; this translates to leave/retn N and purges local variables
.fail:
        push    -1
        jmp     .return
.end_of_media:
        push    DISK_STATUS_END_OF_MEDIA
        jmp     .return
endp
