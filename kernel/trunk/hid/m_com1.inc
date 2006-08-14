; Номер принимаемого от мыши байта
MouseByteNumber DB 0
; Трехбайтовая структура данных, передаваемая мышью
FirstByte       DB 0
SecondByte      DB 0
ThirdByte       DB 0
timer_ticks_com dd 0
;***************************************
;* НОВЫЙ ОБРАБОТЧИК ПРЕРЫВАНИЯ ОТ МЫШИ *
;***************************************
check_mouse_data_com1:
;        cmp     [com1_mouse_detected],0
;        je      @@EndMouseInterrupt
; Проверить наличие данных
        mov     DX,3F8h  ;[COMPortBaseAddr]
        add     DX,5         ;xFDh
        in      AL,DX
        test    AL,1   ;Данные готовы?
        jz      @@Error
; Ввести данные
        mov     DX,3F8h   ;[COMPortBaseAddr]   ;xF8h
        in      AL,DX
; Сбросить старший незначащий бит
        and     AL,01111111b

; Определить порядковый номер принимаемого байта
        cmp     [MouseByteNumber],0
        je      @@FirstByte
        cmp     [MouseByteNumber],1
        je      @@SecondByte
        cmp     [MouseByteNumber],2
        je      @@ThirdByte
        jmp     @@Error

; Сохранить первый байт данных
@@FirstByte:
        test    AL,1000000b       ;Первый байт посылки?
        jz      @@Error
        mov     [FirstByte],AL
        inc     [MouseByteNumber] ;увеличить счетчик
        jmp     @@EndMouseInterrupt
; Сохранить второй байт данных
@@SecondByte:
        test    AL,1000000b
        jnz     @@Error
        mov     [SecondByte],AL
        inc     [MouseByteNumber] ;увеличить счетчик
        jmp     @@EndMouseInterrupt
; Сохранить третий байт данных
@@ThirdByte:
        test    AL,1000000b
        jnz     @@Error
        mov     [ThirdByte],AL    ;увеличить счетчик
        mov     [MouseByteNumber],0
; (Пакет данных от мыши принят полностью).
; Записать новое значение состояния кнопок мыши
        mov al,[FirstByte]   ;[0xfb01]
        mov ah,al
        shr al,3
        and al,2
        shr ah,5
        and ah,1
        add al,ah
        mov    [0xfb40],al
        mov    [mouse_active],1
; Прибавить перемещение по X к координате X
        mov     AL,[FirstByte]
        shl     AL,6
        or      AL,[SecondByte]
        cbw
        call    mouse_acceleration_com1
        add     AX,[0xFB0A]    ;[XCoordinate]
        ; Курсор не должен выходить за левую или
        ; правую границу экрана
        js      @@X1
        cmp     AX,[0xFE00]   ;ScreenLength
        jb      @@X2
        ; Установить координату X по правой границе
        mov     AX,[0xFE00]   ;ScreenLength-1
        dec     ax
        jmp     @@X2
@@X1:
       ; Установить координату X по левой границе
        xor     AX,AX
@@X2:
        mov     [0xFB0A],AX     ;[XCoordinate]
        ; Прибавить перемещение по Y к координате Y
        mov     AL,[FirstByte]
        and     AL,00001100b
        shl     AL,4
        or      AL,[ThirdByte]
        cbw
        call    mouse_acceleration_com1
        add     AX,[0xFB0C]   ;[YCoordinate]
        ; Курсор не должен выходить за верхнюю или
        ; нижнюю границу экрана
        js      @@Y1
        cmp     AX,[0xFE04]  ;ScreenHeigth
        jb      @@Y2
        ; Установить координату X по нижней границе
        mov     AX,[0xFE04] ;ScreenHeigth-1
        dec     ax
        jmp     @@Y2
@@Y1:
       ; Установить координату X по верхней границе
        xor     AX,AX
@@Y2:
        mov     [0xFB0C],AX     ;[YCoordinate]
        mov   eax,[timer_ticks]
        mov   [timer_ticks_com],eax
        jmp     @@EndMouseInterrupt

@@Error:
; Произошел сбой в порядке передачи информации от
; мыши, обнулить счетчик байтов пакета данных
        mov     [MouseByteNumber],0
@@EndMouseInterrupt:
        call  ready_for_next_irq
        ret

mouse_acceleration_com1:
        push    eax
        mov   eax,[timer_ticks]
        sub   eax,[timer_ticks_com]
        cmp   eax,[mouse_delay]
        pop    eax
        ja     @f
        imul  ax,[mouse_speed_factor]
@@:
        ret
