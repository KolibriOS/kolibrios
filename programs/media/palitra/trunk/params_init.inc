    mov  edi,params         ; указатель на строку
    call str_len            ; получаем длину строки
    cmp  eax,0              ; стравниваем длину с нулём
    je   red                ; если параметров нет то уходим
    call    _read_params    ; иначе читаем параметры цвета (очень криво)
    xor  eax,eax            ; зануляем регистр
    mov  al,byte [params]   ; читаем параметр мода
    cmp  al,'N'             ; сравниваем с normal mode
    jne _no_normalmode      ; если нет то уходим на проверку дальше
    mov  [runmode],1        ; если да то устанавливаем его
    jmp  red                ; идём на отрисовку
  _no_normalmode:           ; если не normal mode то
    cmp  al,'H'             ; возможно hidden mode
    jne _no_hiddenmode      ; если не он то уходим дальше
    mov  [runmode],2        ; если он то устанавливаем что это он (нах?)
    call set_background     ; меняем фон
    jmp  bexit              ; и закрываем прогу
  _no_hiddenmode:           ; если не hidden mode
    cmp  al,'D'             ; возможно это color dialog mode
    jne _no_dialogmode      ; если всё таки не он то уходим
    mov  [runmode],3        ; иначе указываем что это таки он
    jmp  red                ; и уходим на выполнение
  _no_dialogmode:           ; если это ни один из заявленых модов
    mov  [runmode],1        ; то сбрасываем на дефолт
    jmp  red                ; и рисуем