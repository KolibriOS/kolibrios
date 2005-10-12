;**************************************************
;*      ПОИСК МЫШИ ПО ПОСЛЕДОВАТЕЛЬНЫМ ПОРТАМ     *
;* Процедура подготавливает глобальные переменные *
;* COMPortNum и COMPortBaseAddr для подпрограммы  *
;* установки обработчика прерывания               *
;**************************************************
; Автор исходного текста Кулаков Владимир Геннадьевич.
; Адаптация и доработка Mario79

MSMouseSearch:
        pusha
; ПОИСК МЫШИ ЧЕРЕЗ COM-ПОРТЫ
MouseSearch:
        ; Устанавливаем скорость
        ; приема/передачи 1200 бод
        mov     DX,[COMPortBaseAddr]
        add     DX,3
        in      AL,DX
        or      AL,80h  ;установить бит DLAB
        out     DX,AL
        mov     DX,[COMPortBaseAddr]
        mov     AL,60h  ;1200 бод
        out     DX,AL
        inc     DX
        mov     AL,0
        out     DX,AL
        ; Установить длину слова 7 бит, 1 стоповый бит,
        ; четность не контролировать
        mov     DX,[COMPortBaseAddr]
        add     DX,3
        mov     AL,00000010b
        out     DX,AL
        ; Запретить все прерывания
        mov     DX,[COMPortBaseAddr]
        inc     DX
        mov     AL,0
        out     DX,AL
; Проверить, что устройство подключено и является
; мышью типа MSMouse
        ; Отключить питание мыши и прерывания
        mov     DX,[COMPortBaseAddr]
        add     DX,4    ;регистр управления модемом
        mov     AL,0    ;сбросить DTR, RTS и OUT2
        out     DX,AL
        ; Ожидать 5 "тиков" (0,2 с)
        mov  ecx,0xffff
dT_1:
        dec  ecx
        cmp ecx,0
        jne dT_1
       mov  ecx,0xffff 
        ; Включить питание мыши
        mov     AL,11b  ;установить DTR и RTS
        out     DX,AL
        ; Очистить регистр данных 
        mov     DX,[COMPortBaseAddr]
        in      AL,DX
; Цикл опроса порта 
WaitData:
        ; Ожидать еще 10 "тиков"
         dec  ecx
         cmp  ecx,0
         je     NoMouse
        ; Проверить наличие идентификационного байта
        mov     DX,[COMPortBaseAddr]
        add     DX,5
        in      AL,DX
        test    AL,1   ;Данные готовы?
        jz      WaitData
        ; Ввести данные
        mov     DX,[COMPortBaseAddr]
        in      AL,DX
        ; Устройство является мышью?
        cmp     AL,'M'
        je EndDetect
NoMouse:
        inc     [COMPortNum]
        cmp     [COMPortNum],1
        ja      EndDetect
        sub     [COMPortBaseAddr],100h
        jmp     MouseSearch

iglobal
COMPortBaseAddr dw 3F8h
COMPortNum dw 0
endg

iglobal
boot_setmouse_type  db   'No COM mouse, set PS2',0
                    db   'Detected - COM1 mouse',0
                    db   'Detected - COM2 mouse',0
 
endg

EndDetect:
        cmp     [COMPortNum],0  ;номер порта меньше 0?
        jl      ComMouseNotFound ;мышь не найдена
        cmp     [COMPortNum],1  ;номер порта больше 1?
        ja      ComMouseNotFound ;мышь не найдена
        mov     ax,[COMPortNum]
        add     al,2
        jmp     set_detecting_mouse
 ComMouseNotFound:
        mov al,1
 set_detecting_mouse:
        mov   [0xF604],al
        dec   al
        movzx eax,al
        imul  eax,22
        mov   esi,boot_setmouse_type
        add   esi,eax
        call  boot_log
        popa


