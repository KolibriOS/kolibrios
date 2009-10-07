;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 593 $


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
     mov esi,FDD_BUFF   ;FDD_DataBuffer  ;0x40000
     xor ecx,ecx
     mov cx,128
     cld
     rep movsd
     ret

;take_data_from_application:   ; взять из приложени
;     mov esi,[TASK_BASE]
;     mov esi,[esi+TASKDATA.mem_start]
;     add esi,ecx
take_data_from_application_1:
     mov edi,FDD_BUFF   ;FDD_DataBuffer  ;0x40000
     xor ecx,ecx
     mov cx,128
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
        mov al,0
        out 0x0c,al     ; reset the flip-flop to a known state.
        mov al,6                ; mask channel 2 so we can reprogram it.
        out 0x0a,al
        mov al,[dmamode] ; 0x46 -> Read from floppy - 0x4A Write to floppy
        out 0x0b,al
        mov al,0
        out 0x0c,al     ; reset the flip-flop to a known state.
        mov eax,0xD000
        out 0x04,al     ; set the channel 2 starting address to 0
        shr eax,8
        out 0x04,al
        shr eax,8
        out 0x81,al
        mov al,0
        out 0x0c, al    ; reset flip-flop
        mov al, 0xff    ;set count (actual size -1)
        out 0x5, al
        mov al,0x1  ;[dmasize]       ;(0x1ff = 511 / 0x23ff =9215)
        out 0x5,al
        mov al,2
        out 0xa,al
        popad
        ret

;***********************************
;* ЗАПИСАТЬ БАЙТ В ПОРТ ДАННЫХ FDC *
;* Параметры:                      *
;* AL - выводимый байт.            *
;***********************************
FDCDataOutput:
;        pusha
        push eax ecx edx
        mov     AH,AL     ;запомнить байт в AH
; Сбросить переменную состояния контроллера
        mov     [FDC_Status],FDC_Normal
; Проверить готовность контроллера к приему данных
        mov     DX,3F4h   ;(порт состояния FDC)
        mov     ecx, 0x10000 ;установить счетчик тайм-аута
@@TestRS:
        in      AL,DX     ;прочитать регистр RS
        and     AL,0C0h   ;выделить разряды 6 и 7
        cmp     AL,80h    ;проверить разряды 6 и 7
        je      @@OutByteToFDC
        loop    @@TestRS
; Ошибка тайм-аута
        mov     [FDC_Status],FDC_TimeOut
        jmp @@End_5
; Вывести байт в порт данных
@@OutByteToFDC:
        inc     DX
        mov     AL,AH
        out     DX,AL
@@End_5:
;        popa
        pop edx ecx eax
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
        mov     [FDC_Status],FDC_Normal
; Проверить готовность контроллера к передаче данных
        mov     DX,3F4h   ;(порт состояния FDC)
        xor     CX,CX     ;установить счетчик тайм-аута
@@TestRS_1:
        in      AL,DX     ;прочитать регистр RS
        and     AL,0C0h   ;выдлить разряды 6 и 7
        cmp     AL,0C0h   ;проверить разряды 6 и 7
        je      @@GetByteFromFDC
        loop    @@TestRS_1
; Ошибка тайм-аута
        mov     [FDC_Status],FDC_TimeOut
        jmp @@End_6
; Ввести байт из порта данных
@@GetByteFromFDC:
        inc     DX
        in      AL,DX
@@End_6:  pop     DX
        pop     ECX
        ret

;*********************************************
;* ОБРАБОТЧИК ПРЕРЫВАНИЯ ОТ КОНТРОЛЛЕРА НГМД *
;*********************************************
FDCInterrupt:
; Установить флаг прерывани
        mov     [FDD_IntFlag],1
        ret


;******************************************
;* УСТАНОВИТЬ НОВЫЙ ОБРАБОТЧИК ПРЕРЫВАНИЙ *
;*             НГМД                       *
;******************************************
SetUserInterrupts:
         mov     [fdc_irq_func],FDCInterrupt
         ret

;*******************************************
;* ОЖИДАНИЕ ПРЕРЫВАНИЯ ОТ КОНТРОЛЛЕРА НГМД *
;*******************************************
WaitFDCInterrupt:
        pusha
; Сбросить байт состояния операции
        mov     [FDC_Status],FDC_Normal
; Сбросить флаг прерывани
        mov     [FDD_IntFlag],0
; Обнулить счетчик тиков
        mov     eax,[timer_ticks]
        mov     [TickCounter],eax
; Ожидать установки флага прерывания НГМД
@@TestRS_2:
        cmp     [FDD_IntFlag],0
        jnz     @@End_7           ;прерывание произошло
        call    change_task
        mov     eax,[timer_ticks]
        sub     eax,[TickCounter]
        cmp     eax,50  ;25   ;5 ;ожидать 5 тиков
        jb      @@TestRS_2
;        jl      @@TestRS_2
; Ошибка тайм-аута
        mov     [FDC_Status],FDC_TimeOut
;        mov   [flp_status],0
@@End_7:  popa
        ret

;*********************************
;* ВКЛЮЧИТЬ МОТОР ДИСКОВОДА "A:" *
;*********************************
FDDMotorON:
        pusha
;        cmp     [fdd_motor_status],1
;        je      fdd_motor_on
        mov     al,[flp_number]
        cmp     [fdd_motor_status],al
        je      fdd_motor_on
; Произвести сброс контроллера НГМД
        mov     DX,3F2h ;порт управления двигателями
        mov     AL,0
        out     DX,AL
; Выбрать и включить мотор дисковода
        cmp     [flp_number],1
        jne     FDDMotorON_B
;        call    FDDMotorOFF_B
        mov     AL,1Ch    ; Floppy A
        jmp     FDDMotorON_1
FDDMotorON_B:
;        call    FDDMotorOFF_A
        mov     AL,2Dh    ; Floppy B
FDDMotorON_1:
        out     DX,AL
; Обнулить счетчик тиков
        mov     eax,[timer_ticks]
        mov     [TickCounter],eax
; Ожидать 0,5 с
@@dT:
        call    change_task
        mov     eax,[timer_ticks]
        sub     eax,[TickCounter]
        cmp     eax,50  ;10
        jb      @@dT
        cmp     [flp_number],1
        jne     fdd_motor_on_B
        mov     [fdd_motor_status],1
        jmp     fdd_motor_on
fdd_motor_on_B:
        mov     [fdd_motor_status],2
fdd_motor_on:
        call    save_timer_fdd_motor
        popa
        ret

;*****************************************
;*  СОХРАНЕНИЕ УКАЗАТЕЛЯ ВРЕМЕНИ         *
;*****************************************
save_timer_fdd_motor:
        mov     eax,[timer_ticks]
        mov     [timer_fdd_motor],eax
        ret

;*****************************************
;*  ПРОВЕРКА ЗАДЕРЖКИ ВЫКЛЮЧЕНИЯ МОТОРА  *
;*****************************************
align 4
check_fdd_motor_status:
        cmp     [fdd_motor_status],0
        je      end_check_fdd_motor_status_1
        mov     eax,[timer_ticks]
        sub     eax,[timer_fdd_motor]
        cmp     eax,500
        jb      end_check_fdd_motor_status
        call    FDDMotorOFF
        mov     [fdd_motor_status],0
end_check_fdd_motor_status_1:
        mov     [flp_status],0
end_check_fdd_motor_status:
        ret

;**********************************
;* ВЫКЛЮЧИТЬ МОТОР ДИСКОВОДА      *
;**********************************
FDDMotorOFF:
        push    AX
        push    DX
        cmp     [flp_number],1
        jne     FDDMotorOFF_1
        call    FDDMotorOFF_A
        jmp     FDDMotorOFF_2
FDDMotorOFF_1:
        call    FDDMotorOFF_B
FDDMotorOFF_2:
        pop     DX
        pop     AX
        ; сброс флагов кеширования в связи с устареванием информации
        mov    [root_read],0
        mov    [flp_fat],0
        ret

FDDMotorOFF_A:
        mov     DX,3F2h ;порт управления двигателями
        mov     AL,0Ch  ; Floppy A
        out     DX,AL
        ret

FDDMotorOFF_B:
        mov     DX,3F2h ;порт управления двигателями
        mov     AL,5h  ; Floppy B
        out     DX,AL
        ret

;*******************************
;* РЕКАЛИБРОВКА ДИСКОВОДА "A:" *
;*******************************
RecalibrateFDD:
        pusha
        call    save_timer_fdd_motor
; Подать команду "Рекалибровка"
        mov     AL,07h
        call    FDCDataOutput
        mov     AL,00h
        call    FDCDataOutput
; Ожидать завершения операции
        call    WaitFDCInterrupt
;        cmp    [FDC_Status],0
;        je    no_fdc_status_error
;        mov   [flp_status],0
;no_fdc_status_error:
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
        pusha
        call    save_timer_fdd_motor
; Подать команду "Поиск"
        mov     AL,0Fh
        call    FDCDataOutput
        ; Передать байт номера головки/накопител
        mov     AL,[FDD_Head]
        shl     AL,2
        call    FDCDataOutput
        ; Передать байт номера дорожки
        mov     AL,[FDD_Track]
        call    FDCDataOutput
; Ожидать завершения операции
        call    WaitFDCInterrupt
        cmp     [FDC_Status],FDC_Normal
        jne     @@Exit
; Сохранить результат поиска
        mov     AL,08h
        call    FDCDataOutput
        call    FDCDataInput
        mov     [FDC_ST0],AL
        call    FDCDataInput
        mov     [FDC_C],AL
; Проверить результат поиска
        ; Поиск завершен?
        test    [FDC_ST0],100000b
        je      @@Err
        ; Заданный трек найден?
        mov     AL,[FDC_C]
        cmp     AL,[FDD_Track]
        jne     @@Err
        ; Номер головки совпадает с заданным?
        mov     AL,[FDC_ST0]
        and     AL,100b
        shr     AL,2
        cmp     AL,[FDD_Head]
        jne     @@Err
        ; Операция завершена успешно
        mov     [FDC_Status],FDC_Normal
        jmp @@Exit
@@Err:  ; Трек не найден
        mov     [FDC_Status],FDC_TrackNotFound
;        mov   [flp_status],0
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
        pushad
        call    save_timer_fdd_motor
; Установить скорость передачи 500 Кбайт/с
        mov     AX,0
        mov     DX,03F7h
        out     DX,AL
; Инициализировать канал прямого доступа к памяти
        mov     [dmamode],0x46
        call    Init_FDC_DMA
; Подать команду "Чтение данных"
        mov     AL,0E6h  ;чтение в мультитрековом режиме
        call    FDCDataOutput
        mov     AL,[FDD_Head]
        shl     AL,2
        call    FDCDataOutput
        mov     AL,[FDD_Track]
        call    FDCDataOutput
        mov     AL,[FDD_Head]
        call    FDCDataOutput
        mov     AL,[FDD_Sector]
        call    FDCDataOutput
        mov     AL,2    ;код размера сектора (512 байт)
        call    FDCDataOutput
        mov     AL,18  ;+1; 3Fh  ;число секторов на дорожке
        call    FDCDataOutput
        mov     AL,1Bh  ;значение GPL
        call    FDCDataOutput
        mov     AL,0FFh ;значение DTL
        call    FDCDataOutput
; Ожидаем прерывание по завершении операции
        call    WaitFDCInterrupt
        cmp     [FDC_Status],FDC_Normal
        jne     @@Exit_1
; Считываем статус завершения операции
        call    GetStatusInfo
        test    [FDC_ST0],11011000b
        jnz     @@Err_1
        mov     [FDC_Status],FDC_Normal
        jmp @@Exit_1
@@Err_1:  mov     [FDC_Status],FDC_SectorNotFound
;        mov   [flp_status],0
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
        mov     [RecalRepCounter],0
@@TryAgain:
; Обнулить счетчик повторения операции чтени
        mov     [ReadRepCounter],0
@@ReadSector_1:
        call    ReadSector
        cmp     [FDC_Status],0
        je      @@Exit_2
        cmp     [FDC_Status],1
        je      @@Err_3
        ; Троекратное повторение чтени
        inc     [ReadRepCounter]
        cmp     [ReadRepCounter],3
        jb      @@ReadSector_1
        ; Троекратное повторение рекалибровки
        call    RecalibrateFDD
        call    SeekTrack
        inc     [RecalRepCounter]
        cmp     [RecalRepCounter],3
        jb      @@TryAgain
;        mov   [flp_status],0
@@Exit_2:
        popa
        ret
@@Err_3:
        mov   [flp_status],0
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
        pushad
        call    save_timer_fdd_motor
; Установить скорость передачи 500 Кбайт/с
        mov     AX,0
        mov     DX,03F7h
        out     DX,AL
; Инициализировать канал прямого доступа к памяти
        mov     [dmamode],0x4A
        call    Init_FDC_DMA
; Подать команду "Запись данных"
        mov     AL,0xC5  ;0x45  ;запись в мультитрековом режиме
        call    FDCDataOutput
        mov     AL,[FDD_Head]
        shl     AL,2
        call    FDCDataOutput
        mov     AL,[FDD_Track]
        call    FDCDataOutput
        mov     AL,[FDD_Head]
        call    FDCDataOutput
        mov     AL,[FDD_Sector]
        call    FDCDataOutput
        mov     AL,2    ;код размера сектора (512 байт)
        call    FDCDataOutput
        mov     AL,18; 3Fh  ;число секторов на дорожке
        call    FDCDataOutput
        mov     AL,1Bh  ;значение GPL
        call    FDCDataOutput
        mov     AL,0FFh ;значение DTL
        call    FDCDataOutput
; Ожидаем прерывание по завершении операции
        call    WaitFDCInterrupt
        cmp     [FDC_Status],FDC_Normal
        jne     @@Exit_3
; Считываем статус завершения операции
        call    GetStatusInfo
        test    [FDC_ST0],11000000b  ;11011000b
        jnz     @@Err_2
        mov     [FDC_Status],FDC_Normal
        jmp @@Exit_3
@@Err_2:  mov     [FDC_Status],FDC_SectorNotFound
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
        mov     [RecalRepCounter],0
@@TryAgain_1:
; Обнулить счетчик повторения операции чтени
        mov     [ReadRepCounter],0
@@WriteSector_1:
        call    WriteSector
        cmp     [FDC_Status],0
        je      @@Exit_4
        cmp     [FDC_Status],1
        je      @@Err_4
        ; Троекратное повторение чтени
        inc     [ReadRepCounter]
        cmp     [ReadRepCounter],3
        jb      @@WriteSector_1
        ; Троекратное повторение рекалибровки
        call    RecalibrateFDD
        call    SeekTrack
        inc     [RecalRepCounter]
        cmp     [RecalRepCounter],3
        jb      @@TryAgain_1
@@Exit_4:
        popa
        ret
@@Err_4:
        mov   [flp_status],0
        popa
        ret

;*********************************************
;* ПОЛУЧИТЬ ИНФОРМАЦИЮ О РЕЗУЛЬТАТЕ ОПЕРАЦИИ *
;*********************************************
GetStatusInfo:
        push    AX
        call    FDCDataInput
        mov     [FDC_ST0],AL
        call    FDCDataInput
        mov     [FDC_ST1],AL
        call    FDCDataInput
        mov     [FDC_ST2],AL
        call    FDCDataInput
        mov     [FDC_C],AL
        call    FDCDataInput
        mov     [FDC_H],AL
        call    FDCDataInput
        mov     [FDC_R],AL
        call    FDCDataInput
        mov     [FDC_N],AL
        pop     AX
        ret

