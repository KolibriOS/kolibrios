;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


struc COM_MOUSE_DATA {
; Номер принимаемого от мыши байта
        .MouseByteNumber        db      ?
; Трехбайтовая структура данных, передаваемая мышью
        .FirstByte              db      ?
        .SecondByte             db      ?
        .ThirdByte              db      ?
        .timer_ticks_com        dd      ?
}
virtual at 0
 COM_MOUSE_DATA COM_MOUSE_DATA
end virtual

uglobal
com1_mouse COM_MOUSE_DATA
com2_mouse COM_MOUSE_DATA
endg

;***************************************
;* НОВЫЙ ОБРАБОТЧИК ПРЕРЫВАНИЯ ОТ МЫШИ *
;***************************************
check_mouse_data_com:
; in: esi -> COM_MOUSE_DATA struc, dx = base port (xF8h)
        add     edx, 5          ; xFDh
        in      al, dx
        test    al, 1           ; Данные готовы?
        jz      .Error
; Ввести данные
        sub     edx, 5
        in      al, dx
; Сбросить старший незначащий бит
        and     al, 01111111b

; Определить порядковый номер принимаемого байта
        cmp     [esi+COM_MOUSE_DATA.MouseByteNumber], 2
        ja      .Error
        jz      .ThirdByte
        jp      .SecondByte
; Сохранить первый байт данных
.FirstByte:
        test    al, 1000000b    ; Первый байт посылки?
        jz      .Error
        mov     [esi+COM_MOUSE_DATA.FirstByte], al
        inc     [esi+COM_MOUSE_DATA.MouseByteNumber]
        jmp     .EndMouseInterrupt
; Сохранить второй байт данных
.SecondByte:
        test    al, 1000000b
        jnz     .Error
        mov     [esi+COM_MOUSE_DATA.SecondByte], al
        inc     [esi+COM_MOUSE_DATA.MouseByteNumber]
        jmp     .EndMouseInterrupt
; Сохранить третий байт данных
.ThirdByte:
        test    al, 1000000b
        jnz     .Error
        mov     [esi+COM_MOUSE_DATA.ThirdByte], al
        mov     [esi+COM_MOUSE_DATA.MouseByteNumber], 0
; (Пакет данных от мыши принят полностью).
; Записать новое значение состояния кнопок мыши
        mov     al, [esi+COM_MOUSE_DATA.FirstByte]
        mov     ah, al
        shr     al, 3
        and     al, 2
        shr     ah, 5
        and     ah, 1
        add     al, ah
        mov     [BTN_DOWN], al
        mov     [mouse_active], 1
; Прибавить перемещение по X к координате X
        mov     al, [esi+COM_MOUSE_DATA.FirstByte]
        shl     al, 6
        or      al, [esi+COM_MOUSE_DATA.SecondByte]
        call    mouse_acceleration_com
; mouse acceleration
        mov     ecx, [timer_ticks]
        sub     ecx, [esi+COM_MOUSE_DATA.timer_ticks_com]
        cmp     ecx, [mouse_delay]
        ja      @f
        imul    ax, [mouse_speed_factor]
@@:
        add     ax, [MOUSE_X]
; Курсор не должен выходить за левую или правую границу экрана
        js      .x1
        cmp     ax, [Screen_Max_X]
        jb      .x2
; Установить координату X по правой границе
        mov     ax, [Screen_Max_X]
        jmp     .x2
.x1:
; Установить координату X по левой границе
        xor     eax, eax
.x2:
        mov     [MOUSE_X], ax
; Прибавить перемещение по Y к координате Y
        mov     al, [esi+COM_MOUSE_DATA.FirstByte]
        and     al, 00001100b
        shl     al, 4
        or      al, [esi+COM_MOUSE_DATA.ThirdByte]
        call    mouse_acceleration_com
        add     ax, [MOUSE_Y]
; Курсор не должен выходить за верхнюю или нижнюю границу экрана
        js      .y1
        cmp     ax, [Screen_Max_Y]
        jb      .y2
; Установить координату Y по нижней границе
        mov     ax, [Screen_Max_Y]
        jmp     .y2
.y1:
; Установить координату Y по верхней границе
        xor     eax, eax
.y2:
        mov     [MOUSE_Y], ax
        mov     eax, [timer_ticks]
        mov     [esi+COM_MOUSE_DATA.timer_ticks_com], eax
        jmp     .EndMouseInterrupt

.Error:
; Произошел сбой в порядке передачи информации от
; мыши, обнулить счетчик байтов пакета данных
        mov     [esi+COM_MOUSE_DATA.MouseByteNumber],0
.EndMouseInterrupt:
        jmp     ready_for_next_irq

mouse_acceleration_com:
        cbw
        mov     ecx, [timer_ticks]
        sub     ecx, [esi+COM_MOUSE_DATA.timer_ticks_com]
        cmp     ecx, [mouse_delay]
        ja      @f
        imul    ax, [mouse_speed_factor]
@@:
        ret
