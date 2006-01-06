; Номер принимаемого от мыши байта
MouseByteNumber_1 DB 0
; Трехбайтовая структура данных, передаваемая мышью
FirstByte_1       DB 0
SecondByte_1      DB 0
ThirdByte_1       DB 0

;***************************************
;* НОВЫЙ ОБРАБОТЧИК ПРЕРЫВАНИЯ ОТ МЫШИ *
;***************************************
check_mouse_data_com2:
        cmp     [com2_mouse_detected],0
        je      @@EndMouseInterrupt_1
; Проверить наличие данных
        mov     DX,2F8h  ;[COMPortBaseAddr]
        add     DX,5         ;xFDh
        in      AL,DX
        test    AL,1   ;Данные готовы?
        jz      @@Error_1
; Ввести данные
        mov     DX,2F8h   ;[COMPortBaseAddr]   ;xF8h
        in      AL,DX
; Сбросить старший незначащий бит
        and     AL,01111111b

; Определить порядковый номер принимаемого байта
        cmp     [MouseByteNumber_1],0
        je      @@FirstByte_1
        cmp     [MouseByteNumber_1],1
        je      @@SecondByte_1
        cmp     [MouseByteNumber_1],2
        je      @@ThirdByte_1
        jmp     @@Error_1

; Сохранить первый байт данных
@@FirstByte_1:
        test    AL,1000000b       ;Первый байт посылки?
        jz      @@Error_1
        mov     [FirstByte_1],AL
        inc     [MouseByteNumber_1] ;увеличить счетчик
        jmp     @@EndMouseInterrupt_1
; Сохранить второй байт данных
@@SecondByte_1:
        test    AL,1000000b
        jnz     @@Error_1
        mov     [SecondByte_1],AL
        inc     [MouseByteNumber_1] ;увеличить счетчик
        jmp     @@EndMouseInterrupt_1
; Сохранить третий байт данных
@@ThirdByte_1:
        test    AL,1000000b
        jnz     @@Error_1
        mov     [ThirdByte_1],AL    ;увеличить счетчик
        mov     [MouseByteNumber_1],0
; (Пакет данных от мыши принят полностью).
; Записать новое значение состояния кнопок мыши
        mov al,[FirstByte_1]   ;[0xfb01]
        mov ah,al
        shr al,3
        and al,2
        shr ah,5
        and ah,1
        add al,ah
        mov    [0xfb40],al
        mov    [mouse_active],1
; Прибавить перемещение по X к координате X
        mov     AL,[FirstByte_1]
        shl     AL,6
        or      AL,[SecondByte_1]
        cbw
         shl  ax,1
        add     AX,[0xFB0A]    ;[XCoordinate]
        ; Курсор не должен выходить за левую или
        ; правую границу экрана
        js      @@X1_1
        cmp     AX,[0xFE00]   ;ScreenLength
        jb      @@X2_1
        ; Установить координату X по правой границе
        mov     AX,[0xFE00]   ;ScreenLength-1
        dec     ax
        jmp     @@X2_1
@@X1_1:
       ; Установить координату X по левой границе
        xor     AX,AX
@@X2_1:
        mov     [0xFB0A],AX     ;[XCoordinate]
        ; Прибавить перемещение по Y к координате Y
        mov     AL,[FirstByte_1]
        and     AL,00001100b
        shl     AL,4
        or      AL,[ThirdByte_1]
        cbw
         shl  ax,1
        add     AX,[0xFB0C]   ;[YCoordinate]
        ; Курсор не должен выходить за верхнюю или
        ; нижнюю границу экрана
        js      @@Y1_1
        cmp     AX,[0xFE04]  ;ScreenHeigth
        jb      @@Y2_1
        ; Установить координату X по нижней границе
        mov     AX,[0xFE04] ;ScreenHeigth-1
        dec     ax
        jmp     @@Y2_1
@@Y1_1:
       ; Установить координату X по верхней границе
        xor     AX,AX
@@Y2_1:
        mov     [0xFB0C],AX     ;[YCoordinate]
        jmp     @@EndMouseInterrupt_1

@@Error_1:
; Произошел сбой в порядке передачи информации от
; мыши, обнулить счетчик байтов пакета данных
        mov     [MouseByteNumber_1],0
@@EndMouseInterrupt_1:
        call  ready_for_next_irq
        ret

