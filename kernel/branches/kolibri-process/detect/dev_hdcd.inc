;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 3881 $


;******************************************************
; поиск приводов HDD и CD
; автор исходного текста Кулаков Владимир Геннадьевич.
; адаптация и доработка Mario79
;******************************************************

;****************************************************
;*                 ПОИСК HDD и CD                   *
;****************************************************
        cmp     [ecx+IDE_DATA.ProgrammingInterface], 0
        je      EndFindHDD

FindHDD:
        push    ecx

        xor     ebx, ebx
        inc     ebx

        cmp     ecx, IDE_controller_2
        jne     @f

        add     bl, 5
        jmp     .find
@@:
        cmp     ecx, IDE_controller_3
        jne     .find

        add     bl, 10
;--------------------------------------
.find:

        mov     [ChannelNumber], 1
        mov     [DiskNumber], 0
        call    FindHDD_2

        mov     [DiskNumber], 1
        call    FindHDD_2

        inc     [ChannelNumber]

        mov     [DiskNumber], 0
        call    FindHDD_2

        mov     [DiskNumber], 1
        call    FindHDD_1

        pop     ecx
        jmp     EndFindHDD
;-----------------------------------------------------------------------------
FindHDD_2:
        call    FindHDD_1
        shl     byte [ebx+DRIVE_DATA], 2
        ret
;-----------------------------------------------------------------------------
FindHDD_1:
        DEBUGF  1, "K : Channel %d ",[ChannelNumber]:2
        DEBUGF  1, "Disk %d\n",[DiskNumber]:1
        push    ebx ecx
        call    ReadHDD_ID
        pop     ecx ebx
        cmp     [DevErrorCode], 0
        jne     .FindCD

        cmp     [Sector512+6], word 16
        ja      .FindCD

        cmp     [Sector512+12], word 255
        ja      .FindCD

        inc     byte [ebx+DRIVE_DATA]
        jmp     .Print_Device_Name
;--------------------------------------
.FindCD:
        push    ebx ecx
        call    DeviceReset
        pop     ecx ebx
        cmp     [DevErrorCode], 0
        jne     .end

        push    ebx ecx
        call    ReadCD_ID
        pop     ecx ebx
        cmp     [DevErrorCode], 0
        jne     .end

        add     [ebx+DRIVE_DATA], byte 2
;--------------------------------------
.Print_Device_Name: 
        pushad
        pushfd

        xor     ebx, ebx
        mov     bx, [ChannelNumber]
        dec     ebx
        shl     ebx, 1
        add     bl, [DiskNumber]
        shl     ebx, 1

        call    calculate_IDE_device_values_storage
;--------------------------------------
.copy_dev_name:
        mov     esi, Sector512+27*2
        mov     edi, dev_name
        mov     ecx, 20
        cld
;--------------------------------------
@@:
        lodsw
        xchg    ah, al
        stosw
        loop    @b

        DEBUGF 1, "K : Dev: %s \n", dev_name

        xor     eax, eax
        mov     ax, [Sector512+64*2]
        DEBUGF  1, "K : PIO possible modes %x\n", al

        mov     ax, [Sector512+51*2]
        mov     al, ah
        call    convert_Sector512_value
        DEBUGF  1, "K : PIO set mode %x\n", ah

        mov     ax, [Sector512+63*2]
        DEBUGF  1, "K : Multiword DMA possible modes %x\n", al

        mov     al, ah
        call    convert_Sector512_value
        DEBUGF  1, "K : Multiword DMA set mode %x\n", ah

        mov     ax, [Sector512+88*2]
        DEBUGF  1, "K : Ultra DMA possible modes %x\n", al

        mov     [ebx+IDE_DEVICE.UDMA_possible_modes], al

        mov     al, ah
        call    convert_Sector512_value
        DEBUGF  1, "K : Ultra DMA set mode %x\n", ah

        mov     [ebx+IDE_DEVICE.UDMA_set_mode], ah

        popfd
        popad
        ret
;--------------------------------------
.end:
        DEBUGF  1, "K : Device not found\n"
        ret
;-----------------------------------------------------------------------------
calculate_IDE_device_values_storage:
        cmp     ecx, IDE_controller_1
        jne     @f

        add     ebx, IDE_device_1
        jmp     .exit
;--------------------------------------
@@:
        cmp     ecx, IDE_controller_2
        jne     @f

        add     ebx, IDE_device_2
        jmp     .exit
;--------------------------------------
@@:
        add     ebx, IDE_device_3
;--------------------------------------
.exit:
        ret
;-----------------------------------------------------------------------------
convert_Sector512_value:
        mov     ecx, 8
        xor     ah, ah
;--------------------------------------
@@:
        test    al, 1b
        jnz     .end

        shr     al, 1
        inc     ah
        loop    @b

        xor     ah, ah
;--------------------------------------
.end:
        ret
;-----------------------------------------------------------------------------
; Адрес считываемого сектора в режиме LBA
uglobal
SectorAddress   dd ?
dev_name:
        rb 41
endg
;-----------------------------------------------------------------------------
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
        mov     [ATAAddressMode], 0
; Послать команду идентификации устройства
        mov     [ATAFeatures], 0
        mov     [ATAHead], 0
        mov     [ATACommand], 0xEC
        call    SendCommandToHDD
        cmp     [DevErrorCode], 0 ;проверить код ошибки
        jne     @@End  ;закончить, сохранив код ошибки

        mov     dx, [ATABasePortAddr]
        add     dx, 7    ;адрес регистра состояни
        mov     ecx, 0xffff
@@WaitCompleet:
        ; Проверить время выполнения команды
        dec     ecx
        jz      @@Error1  ;ошибка тайм-аута
        ; Проверить готовность
        in      al, dx
        test    al, 80h  ;состояние сигнала BSY
        jnz     @@WaitCompleet

        test    al, 1    ;состояние сигнала ERR
        jnz     @@Error6

        test    al, 08h  ;состояние сигнала DRQ
        jz      @@WaitCompleet
; Принять блок данных от контроллера
        mov     edi, Sector512
        mov     dx, [ATABasePortAddr];регистр данных
        mov     cx, 256  ;число считываемых слов
        rep insw         ;принять блок данных
        ret
; Записать код ошибки
@@Error1:
        mov     [DevErrorCode], 1
        ret
@@Error6:
        mov     [DevErrorCode], 6
@@End:
        ret
;-----------------------------------------------------------------------------
uglobal
; Стандартные базовые адреса каналов 1 и 2
StandardATABases dw ?, ? ; 1F0h, 170h
; Номер канала
ChannelNumber   dw ?
; Номер диска
DiskNumber      db ?
; Базовый адрес группы портов контроллера ATA
ATABasePortAddr dw ?
; Параметры ATA-команды
ATAFeatures     db ? ;особенности
ATASectorCount  db ? ;количество обрабатываемых секторов
ATASectorNumber db ? ;номер начального сектора
ATACylinder     dw ? ;номер начального цилиндра
ATAHead         db ? ;номер начальной головки
ATAAddressMode  db ? ;режим адресации (0 - CHS, 1 - LBA)
ATACommand      db ? ;код команды, подлежащей выполнению
; Код ошибки (0 - нет ошибок, 1 - превышен допустимый
; интервал ожидания, 2 - неверный код режима адресации,
; 3 - неверный номер канала, 4 - неверный номер диска,
; 5 - неверный номер головки, 6 - ошибка при выполнении
; команды)
DevErrorCode dd ?
endg
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
;* возвращен код ошибки.                            *
;****************************************************
SendCommandToHDD:
; Проверить значение кода режима
        cmp     [ATAAddressMode], 1
        ja      @@Err2
; Проверить корректность номера канала
        mov     bx, [ChannelNumber]
        cmp     bx, 1
        jb      @@Err3

        cmp     bx, 2
        ja      @@Err3
; Установить базовый адрес
        dec     bx
        shl     bx, 1
        movzx   ebx, bx
        mov     ax, [ebx+StandardATABases]
        mov     [ATABasePortAddr], ax
; Ожидание готовности HDD к приему команды
        ; Выбрать нужный диск
        mov     dx, [ATABasePortAddr]
        add     dx, 6   ;адрес регистра головок
        mov     al, [DiskNumber]
        cmp     al, 1   ;проверить номера диска
        ja      @@Err4

        shl     al, 4
        or      al, 10100000b
        out     dx, al
        ; Ожидать, пока диск не будет готов
        inc     dx
        mov     ecx, 0xfff
@@WaitHDReady:
        ; Проверить время ожидани
        dec     ecx
        jz      @@Err1
        ; Прочитать регистр состояни
        in      al, dx
        ; Проверить состояние сигнала BSY
        test    al, 80h
        jnz     @@WaitHDReady
        ; Проверить состояние сигнала DRQ
        test    al, 08h
        jnz     @@WaitHDReady
; Загрузить команду в регистры контроллера
        cli
        mov     dx, [ATABasePortAddr]
        inc     dx      ;регистр "особенностей"
        mov     al, [ATAFeatures]
        out     dx, AL
        inc     dx      ;счетчик секторов
        mov     al, [ATASectorCount]
        out     dx, AL
        inc     dx      ;регистр номера сектора
        mov     al, [ATASectorNumber]
        out     dx, AL
        inc     dx      ;номер цилиндра (младший байт)
        mov     ax, [ATACylinder]
        out     dx, AL
        inc     dx      ;номер цилиндра (старший байт)
        mov     al, AH
        out     dx, AL
        inc     dx      ;номер головки/номер диска
        mov     al, [DiskNumber]
        shl     al, 4
        cmp     [ATAHead], 0xF ;проверить номер головки
        ja      @@Err5

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
; Сбросить признак ошибки
        mov     [DevErrorCode], 0
        ret
; Записать код ошибки
@@Err1:
        mov     [DevErrorCode], 1
        ret
@@Err2:
        mov     [DevErrorCode], 2
        ret
@@Err3:
        mov     [DevErrorCode], 3
        ret
@@Err4:
        mov     [DevErrorCode], 4
        ret
@@Err5:
        mov     [DevErrorCode], 5
; Завершение работы программы
        ret
;-----------------------------------------------------------------------------
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
        mov     [ATAAddressMode], 0
; Послать команду идентификации устройства
        mov     [ATAFeatures], 0
        mov     [ATASectorCount], 0
        mov     [ATASectorNumber], 0
        mov     [ATACylinder], 0
        mov     [ATAHead], 0
        mov     [ATACommand], 0xA1
        call    SendCommandToHDD
        cmp     [DevErrorCode], 0;проверить код ошибки
        jne     @@End_1  ;закончить, сохранив код ошибки
; Ожидать готовность данных HDD
        mov     dx, [ATABasePortAddr]
        add     dx, 7  ;порт 1х7h
        mov     ecx, 0xffff
@@WaitCompleet_1:
        ; Проверить врем
        dec     ecx
        jz      @@Error1_1 ;ошибка тайм-аута
        ; Проверить готовность
        in      al, dx
        test    al, 80h  ;состояние сигнала BSY
        jnz     @@WaitCompleet_1

        test    al, 1    ;состояние сигнала ERR
        jnz     @@Error6_1

        test    al, 08h  ;состояние сигнала DRQ
        jz      @@WaitCompleet_1
; Принять блок данных от контроллера
        mov     edi, Sector512 ;offset Sector512
        mov     dx, [ATABasePortAddr];порт 1x0h
        mov     cx, 256;число считываемых слов
        rep insw
        ret
; Записать код ошибки
@@Error1_1:
        mov     [DevErrorCode], 1
        ret
@@Error6_1:
        mov     [DevErrorCode], 6
@@End_1:
        ret
;-----------------------------------------------------------------------------
;*************************************************
;*                СБРОС УСТРОЙСТВА               *
;* Входные параметры передаются через глобальные *
;* переменные:                                   *
;* ChannelNumber - номер канала (1 или 2);       *
;* DiskNumber - номер диска (0 или 1).           *
;*************************************************
DeviceReset:
; Проверить корректность номера канала
        mov     bx, [ChannelNumber]
        cmp     bx, 1
        jb      @@Err3_2

        cmp     bx, 2
        ja      @@Err3_2
; Установить базовый адрес
        dec     bx
        shl     bx, 1
        movzx   ebx, bx
        mov     dx, [ebx+StandardATABases]
        mov     [ATABasePortAddr], dx
; Выбрать нужный диск
        add     dx, 6   ;адрес регистра головок
        mov     al, [DiskNumber]
        cmp     al, 1   ;проверить номера диска
        ja      @@Err4_2

        shl     al, 4
        or      al, 10100000b
        out     dx, al
; Послать команду "Сброс"
        mov     al, 0x8
        inc     dx      ;регистр команд
        out     dx, al
        mov     ecx, 0x80000
@@WaitHDReady_1:
        ; Проверить время ожидани
        dec     ecx
        je      @@Err1_2 ;ошибка тайм-аута
        ; Прочитать регистр состояни
        in      al, dx
        ; Проверить состояние сигнала BSY
        test    al, 80h
        jnz     @@WaitHDReady_1
; Сбросить признак ошибки
        mov     [DevErrorCode], 0
        ret
; Обработка ошибок
@@Err1_2:
        mov     [DevErrorCode], 1
        ret
@@Err3_2:
        mov     [DevErrorCode], 3
        ret
@@Err4_2:
        mov     [DevErrorCode], 4
; Записать код ошибки
        ret
;-----------------------------------------------------------------------------
EndFindHDD:
