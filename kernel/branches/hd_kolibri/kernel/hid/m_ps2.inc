$Revision: 431 $
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Номер принимаемого от мыши байта
MouseByteNumber_2 DB 0
; Трехбайтовая структура данных, передаваемая мышью
FirstByte_2       DB 0
SecondByte_2      DB 0
ThirdByte_2       DB 0
timer_ticks_ps2   dd 0

;**************************************
;* ОБРАБОТЧИК ПРЕРЫВАНИЯ ОТ МЫШИ PS/2 *
;**************************************
check_mouse_data_ps2:
        cmp     [ps2_mouse_detected],0
        je      @@EndMouseInterrupt_2
        call    Wait8042BufferEmpty ;очистка буфера
        in      AL,0x60              ;получить скэн-код
; Выбирать порядковый номер принимаемого байта
        cmp     [MouseByteNumber_2],0
        je      @@SaveFirstByte
        cmp     [MouseByteNumber_2],1
        je      @@SaveSecondByte
        cmp     [MouseByteNumber_2],2
        je      @@SaveThirdByte
        jmp     @@Error_2
; Записать первый байт посылки
@@SaveFirstByte:
        test    AL,1000b  ;первый байт посылки?
        jz      @@Error_2     ;сбой синхронизации
        mov     [FirstByte_2],AL
        inc     [MouseByteNumber_2]
        jmp     @@EndMouseInterrupt_2
; Записать второй байт посылки
@@SaveSecondByte:
        mov     [SecondByte_2],AL
        inc     [MouseByteNumber_2]
        jmp     @@EndMouseInterrupt_2
; Записать третий байт посылки
@@SaveThirdByte:
        mov     [ThirdByte_2],AL
        mov     [MouseByteNumber_2],0
; (пакет данных от мыши принят полностью)
; Записать новое значение байта состояния кнопок
        mov al,[FirstByte_2]   ;[0xfb01]
        and    eax,3
        mov    [BTN_DOWN],al
        mov    [mouse_active],1
; Вычислить новую X-координату курсора
        ; Занести в AX перемещение по X
        mov     AH,0   ;дублируем знак во все разряды AH
        mov     AL,[FirstByte_2]
        test    AL,10000b
        jz      @@M0
        mov     AH,0FFh
        ; Занести в AL младший байт
@@M0:
         mov     AL,[SecondByte_2]
        call     mouse_acceleration_ps2
        ; Вычислить новое значение координаты
        ; курсора по X
        add     AX,[MOUSE_X]    ;[XCoordinate]
        cmp     AX,0
        jge     @@M1
        mov     AX,0
        jmp     @@M2
@@M1:
        cmp     AX,[ScreenWidth]   ;ScreenLength
        jl      @@M2
        mov     AX,[ScreenWidth]   ;ScreenLength-1
        dec     ax
@@M2:
        mov     [MOUSE_X],AX     ;[XCoordinate]

; Вычисляем новую Y-координату курсора
        ; Занести в AX перемещение по Y
        mov     AH,0   ;дублируем знак во все разряды AH
        mov     AL,[FirstByte_2]
        test    AL,100000b
        jz      @@M3
        mov     AH,0FFh
        ; Занести в AL младший байт
@@M3:
        mov     AL,[ThirdByte_2]
        call     mouse_acceleration_ps2
        ; Вычислить новое значение координаты курсора
        ; по Y (Y-координата мыши PS/2 направлена
        ; противоположно экранной)
        neg     AX
        add     AX,[MOUSE_Y]   ;[YCoordinate]
        cmp     AX,0
        jge     @@M4
        mov     AX,0
        jmp     @@M5
@@M4:
        cmp     AX,[ScreenHeight]  ;ScreenHeigth
        jl      @@M5
        mov     AX,[ScreenHeight] ;ScreenHeigth-1
        dec     ax
@@M5:
        mov     [MOUSE_Y],AX     ;[YCoordinate]

; Показать курсор в новой позиции
        mov   eax,[timer_ticks]
        mov   [timer_ticks_ps2],eax
        jmp     @@EndMouseInterrupt_2

; Обнаружен сбой в порядке передачи информации от мыши
@@Error_2:
        mov   [MouseByteNumber_2],0
; Нормальное завершение прерывани
@@EndMouseInterrupt_2:
        call  ready_for_next_irq_1
        ret

mouse_acceleration_ps2:
        push    eax
        mov   eax,[timer_ticks]
        sub   eax,[timer_ticks_ps2]
        cmp   eax,[mouse_delay]
        pop    eax
        ja     @f
        imul  ax,[mouse_speed_factor]
@@:
        ret
;***********************************************
;*   ОЖИДАНИЕ ОЧИСТКИ ВХОДНОГО БУФЕРА I8042    *
;* При выходе из процедуры:                    *
;* флаг ZF установлен - нормальное завершение, *
;* флаг ZF сброшен - ошибка тайм-аута.         *
;***********************************************
Wait8042BufferEmpty:
;        push    CX
;        mov     CX,0FFFFh  ;задать число циклов ожидани
;@@kb:
;        in      AL,64h     ;получить статус
;        test    AL,10b     ;буфер i8042 свободен?
;        loopnz  @@kb       ;если нет, то цикл
;        pop     CX
        push ecx
        xor ecx,ecx
      @@:
        in     al,64h
        test    al,00000010b
        loopnz @b
        pop    ecx
        ;Если при выходе из подпрограммы сброшен
        ;флаг ZF - ошибка
        ret                   ;возврат в подпрограмму

;***************************************
;* ОЖИДАНИЕ ПОСТУПЛЕНИЯ ДАННЫХ ОТ МЫШИ *
;***************************************
WaitMouseData:
;        push    CX
;        mov     CX,0FFFFh  ;задать число циклов ожидани
;@@mouse:
;        in      AL,64h     ;опросить регистр статуса
;        test    AL,100000b ;данные поступили?
;        loopz   @@mouse    ;если нет, то цикл
;        pop     CX
        push ecx
        mov     ECX,0FFFFh
      @@:
        in     al,64h
        test    al,100000b
        loopz @b
        pop    ecx
        ;Если при выходе из подпрограммы установлен
        ;флаг ZF - ошибка
        ret

