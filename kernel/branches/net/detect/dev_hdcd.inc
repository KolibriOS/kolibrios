;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


;******************************************************
; поиск приводов HDD и CD
; автор исходного текста Кулаков Владимир Геннадьевич.
; адаптация и доработка Mario79
;******************************************************

;****************************************************
;*                 ПОИСК HDD и CD                   *
;****************************************************
FindHDD:
        mov     [ChannelNumber],1
        mov     [DiskNumber],0
        call    FindHDD_3
;        mov     ax,[Sector512+176]
;        mov     [DRIVE_DATA+6],ax
;        mov     ax,[Sector512+126]
;        mov     [DRIVE_DATA+8],ax
;        mov     ax,[Sector512+128]
;        mov     [DRIVE_DATA+8],ax
        mov     [DiskNumber],1
        call    FindHDD_3
;        mov     al,[Sector512+176]
;        mov     [DRIVE_DATA+7],al
        inc     [ChannelNumber]
        mov     [DiskNumber],0
        call    FindHDD_3
;        mov     al,[Sector512+176]
;        mov     [DRIVE_DATA+8],al
        mov     [DiskNumber],1
        call    FindHDD_1
;        mov     al,[Sector512+176]
;        mov     [DRIVE_DATA+9],al

        jmp     EndFindHDD

FindHDD_1:
        call    ReadHDD_ID
        cmp     [DevErrorCode],0
        jne     FindHDD_2
        cmp     [Sector512+6],word 16
        ja      FindHDD_2
        cmp     [Sector512+12],word 255
        ja      FindHDD_2
        inc     byte [DRIVE_DATA+1]
        jmp     FindHDD_2_2
   FindHDD_2:
        call    DeviceReset
        cmp     [DevErrorCode],0
        jne     FindHDD_2_2
        call    ReadCD_ID
        cmp     [DevErrorCode],0
        jne      FindHDD_2_2
        inc     byte [DRIVE_DATA+1]
        inc     byte [DRIVE_DATA+1]
   FindHDD_2_2:
        ret

FindHDD_3:
        call    FindHDD_1
        shl     byte [DRIVE_DATA+1],2
        ret


; Адрес считываемого сектора в режиме LBA
SectorAddress   DD ?

;*************************************************
;*     ЧТЕНИЕ ИДЕНТИФИКАТОРА ЖЕСТКОГО ДИСКА      *
;* Входные параметры передаются через глобальные *
;* переменные:                                   *
;* ChannelNumber - номер канала (1 или 2);       *
;* DiskNumber - номер диска на канале (0 или 1). *
;* Идентификационный блок данных считывается     *
;* в массив Sector512.                           *
;*************************************************
ReadHDD_ID:
; Задать режим CHS
        mov     [ATAAddressMode],0
; Послать команду идентификации устройства
        mov     [ATAFeatures],0
        mov     [ATAHead],0
        mov     [ATACommand],0ECh
        call    SendCommandToHDD
        cmp     [DevErrorCode],0 ;проверить код ошибки
        jne     @@End  ;закончить, сохранив код ошибки
        mov     DX,[ATABasePortAddr]
        add     DX,7     ;адрес регистра состояни
        mov  ecx,0xffff
@@WaitCompleet:
        ; Проверить время выполнения команды
        dec  ecx
        cmp  ecx,0
        je     @@Error1   ;ошибка тайм-аута
        ; Проверить готовность
        in      AL,DX
        test    AL,80h   ;состояние сигнала BSY
        jnz     @@WaitCompleet
        test    AL,1     ;состояние сигнала ERR
        jnz     @@Error6
        test    AL,08h   ;состояние сигнала DRQ
        jz      @@WaitCompleet
; Принять блок данных от контроллера
;        mov     AX,DS
;        mov     ES,AX
        mov     EDI,Sector512  ;offset Sector512
        mov     DX,[ATABasePortAddr] ;регистр данных
        mov     CX,256   ;число считываемых слов
        rep     insw     ;принять блок данных
        jmp @@End
; Записать код ошибки
@@Error1:
        mov     [DevErrorCode],1
        jmp @@End
@@Error6:
        mov     [DevErrorCode],6
@@End:  ret



; Стандартные базовые адреса каналов 1 и 2
StandardATABases DW 1F0h, 170h
; Номер канала
ChannelNumber   DW ?
; Номер диска
DiskNumber      DB ?
; Базовый адрес группы портов контроллера ATA
ATABasePortAddr DW ?
; Параметры ATA-команды
ATAFeatures     DB ? ;особенности
ATASectorCount  DB ? ;количество обрабатываемых секторов
ATASectorNumber DB ? ;номер начального сектора
ATACylinder     DW ? ;номер начального цилиндра
ATAHead         DB ? ;номер начальной головки
ATAAddressMode  DB ? ;режим адресации (0 - CHS, 1 - LBA)
ATACommand      DB ? ;код команды, подлежащей выполнению
; Код ошибки (0 - нет ошибок, 1 - превышен допустимый
; интервал ожидания, 2 - неверный код режима адресации,
; 3 - неверный номер канала, 4 - неверный номер диска,
; 5 - неверный номер головки, 6 - ошибка при выполнении
; команды)
DevErrorCode DB ?

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
SendCommandToHDD:
; Проверить значение кода режима
        cmp     [ATAAddressMode],1
        ja      @@Err2
; Проверить корректность номера канала
        mov     BX,[ChannelNumber]
        cmp     BX,1
        jb      @@Err3
        cmp     BX,2
        ja      @@Err3
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
        ja      @@Err4
        shl     AL,4
        or      AL,10100000b
        out     DX,AL
        ; Ожидать, пока диск не будет готов
        inc     DX
        mov  ecx,0xfff
;        mov     eax,[timer_ticks]
;        mov     [TickCounter_1],eax
@@WaitHDReady:
        ; Проверить время ожидани
        dec  ecx
        cmp  ecx,0
        je     @@Err1
;        mov     eax,[timer_ticks]
;        sub     eax,[TickCounter_1]
;        cmp     eax,300    ;ожидать 300 тиков
;        ja      @@Err1   ;ошибка тайм-аута
        ; Прочитать регистр состояни
        in      AL,DX
        ; Проверить состояние сигнала BSY
        test    AL,80h
        jnz     @@WaitHDReady
        ; Проверить состояние сигнала DRQ
        test    AL,08h
        jnz     @@WaitHDReady
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
        ja      @@Err5
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
        jmp @@End_2
; Записать код ошибки
@@Err1: mov     [DevErrorCode],1
        jmp @@End_2
@@Err2: mov     [DevErrorCode],2
        jmp @@End_2
@@Err3: mov     [DevErrorCode],3
        jmp @@End_2
@@Err4: mov     [DevErrorCode],4
        jmp @@End_2
@@Err5: mov     [DevErrorCode],5
; Завершение работы программы
@@End_2:
        ret

;*************************************************
;*     ЧТЕНИЕ ИДЕНТИФИКАТОРА УСТРОЙСТВА ATAPI    *
;* Входные параметры передаются через глобальные *
;* перменные:                                    *
;* ChannelNumber - номер канала;                 *
;* DiskNumber - номер диска на канале.           *
;* Идентификационный блок данных считывается     *
;* в массив Sector512.                           *
;*************************************************
ReadCD_ID:
; Задать режим CHS
        mov     [ATAAddressMode],0
; Послать команду идентификации устройства
        mov     [ATAFeatures],0
        mov     [ATASectorCount],0
        mov     [ATASectorNumber],0
        mov     [ATACylinder],0
        mov     [ATAHead],0
        mov     [ATACommand],0A1h
        call    SendCommandToHDD
        cmp     [DevErrorCode],0 ;проверить код ошибки
        jne     @@End_1  ;закончить, сохранив код ошибки
; Ожидать готовность данных HDD
        mov     DX,[ATABasePortAddr]
        add     DX,7   ;порт 1х7h
        mov  ecx,0xffff
@@WaitCompleet_1:
        ; Проверить врем
        dec  ecx
        cmp  ecx,0
        je     @@Error1_1  ;ошибка тайм-аута
        ; Проверить готовность
        in      AL,DX
        test    AL,80h   ;состояние сигнала BSY
        jnz     @@WaitCompleet_1
        test    AL,1     ;состояние сигнала ERR
        jnz     @@Error6_1
        test    AL,08h   ;состояние сигнала DRQ
        jz      @@WaitCompleet_1
; Принять блок данных от контроллера
;        mov     AX,DS
;        mov     ES,AX
        mov     EDI,Sector512  ;offset Sector512
        mov     DX,[ATABasePortAddr] ;порт 1x0h
        mov     CX,256 ;число считываемых слов
        rep     insw
        jmp @@End_1
; Записать код ошибки
@@Error1_1:
        mov     [DevErrorCode],1
        jmp @@End_1
@@Error6_1:
        mov     [DevErrorCode],6
@@End_1:
        ret

;*************************************************
;*                СБРОС УСТРОЙСТВА               *
;* Входные параметры передаются через глобальные *
;* переменные:                                   *
;* ChannelNumber - номер канала (1 или 2);       *
;* DiskNumber - номер диска (0 или 1).           *
;*************************************************
DeviceReset:
; Проверить корректность номера канала
        mov     BX,[ChannelNumber]
        cmp     BX,1
        jb      @@Err3_2
        cmp     BX,2
        ja      @@Err3_2
; Установить базовый адрес
        dec     BX
        shl     BX,1
        movzx   ebx,bx
        mov     DX,[ebx+StandardATABases]
        mov     [ATABasePortAddr],DX
; Выбрать нужный диск
        add     DX,6    ;адрес регистра головок
        mov     AL,[DiskNumber]
        cmp     AL,1    ;проверить номера диска
        ja      @@Err4_2
        shl     AL,4
        or      AL,10100000b
        out     DX,AL
; Послать команду "Сброс"
        mov     AL,08h
        inc     DX      ;регистр команд
        out     DX,AL
        mov     ecx,0x80000
@@WaitHDReady_1:
        ; Проверить время ожидани
        dec     ecx
        cmp     ecx,0
        je      @@Err1_2 ;ошибка тайм-аута
        ; Прочитать регистр состояни
        in      AL,DX
        ; Проверить состояние сигнала BSY
        test    AL,80h
        jnz     @@WaitHDReady_1
; Сбросить признак ошибки
        mov     [DevErrorCode],0
        jmp @@End_3
; Обработка ошибок
@@Err1_2: mov     [DevErrorCode],1
        jmp @@End_3
@@Err3_2: mov     [DevErrorCode],3
        jmp @@End_3
@@Err4_2: mov     [DevErrorCode],4
; Записать код ошибки
@@End_3:
        ret

EndFindHDD:

