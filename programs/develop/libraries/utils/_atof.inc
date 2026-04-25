;-----------------------------------------------------------------------------+
; Функция преобразования строки в вещественное число [ by ManHunter / PCL ]   |
;-----------------------------------------------------------------------------|
; Параметры:                                                                  |
;   lpStr - указатель на исходную строку в формате ASCIIZ                     |
;   lpResult - указатель на переменную-приемник значения                      |
; На выходе:                                                                  |
;   EAX = 1 - строка успешно преобразована                                    |
;   EAX = 0 - строка не может быть преобразована в число                      |
;-----------------------------------------------------------------------------+
proc    string2float lpStr:DWORD, lpResult:DWORD
    ; Локальные переменные
    locals
      dot    dd ?   ; Указатель на дробную часть
      exp    dd ?   ; Указатель на экспоненту
      digit  dd ?   ; Цифра
    endl
 
    pusha
 
    ; Проверка строки на валидность
    mov     [digit],1
 
    mov     [exp],0
    mov     [dot],0
    mov     esi,[lpStr]
    ; Минус или плюс может быть только в начале
    cmp     byte [esi],'-'
    je      @f
    cmp     byte [esi],'+'
    jne     .loc_chk_loop
@@:
    inc     esi
 
    ; После знака не может быть нуля
    cmp     byte [esi],0
    je      .loc_chk_error
.loc_chk_loop:
    ; В строке должны быть цифр, экспонента и не более одной точки
    lodsb
    or      al,al
    jz      .loc_chk_complete
    cmp     al,'e'
    je      .loc_chk_exp
    cmp     al,'E'
    je      .loc_chk_exp
    cmp     al,'.'
    je      .loc_chk_dot
    cmp     al,'0'
    jb      .loc_chk_error
    cmp     al,'9'
    ja      .loc_chk_error
    jmp     .loc_chk_loop
 
.loc_chk_dot:
    ; Точка в строке уже есть?
    cmp     [dot],0
    ; Строка имеет некорректный формат
    jne     .loc_chk_error
 
    ; Экспонента уже есть?
    cmp     [exp],0
    ; Строка имеет некорректный формат
    jne     .loc_chk_error
 
    ; Указатель на дробную часть
    mov     [dot],esi
 
    jmp     .loc_chk_loop
 
.loc_chk_exp:
    ; Экспонента уже есть?
    cmp     [exp],0
    ; Строка имеет некорректный формат
    jne     .loc_chk_error
 
    ; Указатель на начало экспоненты
    mov     [exp],esi
 
    ; Сразу после экспоненты не может быть нуля
    cmp     byte [esi],0
    je      .loc_chk_error
 
    ; После экспоненты может быть знак
    cmp     byte [esi],'-'
    je      @f
    cmp     byte [esi],'+'
    jne     .loc_chk_loop
@@:
    inc     esi
 
    ; Сразу после минуса не может быть нуля
    cmp     byte [esi],0
    je      .loc_chk_error
 
    ; Проверить следующий символ
    jmp     .loc_chk_loop
 
.loc_chk_error:
    ; Строка не является числом
    mov     [digit],0
    jmp     .loc_ret
 
.loc_chk_complete:
    ; Инициализация сопроцессора
    finit
 
    ; Начальное значение числа
    fldz
 
    ; Множитель и делитель
    mov     [digit],10
    fild    dword [digit]
 
    ; Запись значений до запятой
    mov     esi,[lpStr]
 
    ; В начале строки минус?
    cmp     byte [esi],'-'
    je      @f
    cmp     byte [esi],'+'
    jne     .loc_before_dot
@@:
    inc     esi
    ; Преобразование числа до запятой
.loc_before_dot:
    lodsb
    ; Конец строки?
    or      al,al
    jz      .loc_complete
 
    cmp     al,'.'
    je      .loc_complete_before_dot
    cmp     al,'e'
    je      .loc_exp
    cmp     al,'E'
    je      .loc_exp
 
    ; Очередная цифра
    sub     al,'0'
    movzx   eax,al
    mov     [digit],eax
 
    ; Записать
    fild    dword [digit]
    fxch    st2
    fmul    st0,st1
    fxch    st2
    fadd    st2,st0
 
    ffree   st0     ; Почистить стек
    fincstp
 
    jmp     .loc_before_dot
 
    ; Преобразование дробной части числа
.loc_complete_before_dot:
    ; Дробная часть есть?
    cmp     [dot],0
    je      .loc_complete_after_dot
 
    ; Экспонента есть?
    cmp     [exp],0
    je      @f
 
    ; Указатель на начало экспоненты
    mov     esi,[exp]
    jmp     .loc_start_after_dot
@@:
    ; Иначе перенести указатель на конец строки
    xor     ecx,ecx
    dec     ecx
    xor     eax,eax
    mov     edi,esi
    repne   scasb
 
    mov     esi,edi
 
.loc_start_after_dot:
    std
    dec     esi
    dec     esi
 
    ; Дробная часть
    fldz
    fxch    st1
.loc_after_dot:
    lodsb
    ; Конец дробной части?
    cmp     al,'.'
    je      .loc_complete_after_dot
 
    ; Очередная цифра
    sub     al,'0'
    movzx   eax,al
    mov     [digit],eax
 
    ; Записать
    fild    dword [digit]
    fadd    st2,st0
    fxch    st2
    fdiv    st0,st1
    fxch    st2
 
    ffree   st0     ; Почистить стек
    fincstp
 
    jmp     .loc_after_dot
 
.loc_complete_after_dot:
    ; Сбросить флаг направления
    cld
 
    ffree   st0     ; Почистить стек
    fincstp
 
    ; Сложить дробную и целую часть
    fadd    st1,st0
 
.loc_exp:
    ; Экспонента есть?
    cmp     [exp],0
    je      .loc_complete
 
    ; Получить значение экспоненты
    xor     ecx,ecx
 
    mov     esi,[exp]
    ; В начале строки минус?
    cmp     byte [esi],'-'
    je      @f
    cmp     byte [esi],'+'
    jne     .loc_start_exp
@@:
    inc     esi
.loc_start_exp:
    lodsb
    or      al,al
    jz      .loc_end_exp
 
    sub     al,'0'
    movzx   eax,al
    imul    ecx,10
    add     ecx,eax
 
    jmp     .loc_start_exp
.loc_end_exp:
 
    or      ecx,ecx
    jz      .loc_complete
 
    ffree   st0     ; Почистить стек
    fincstp
 
    mov     [digit],10
    fild    dword [digit]
 
    ; Делить или умножать?
    mov     esi,[exp]
    cmp     byte [esi],'-'
    je      .loc_exp_divide
 
.loc_exp_multiple:
    fmul    st1,st0
    loop    .loc_exp_multiple
    jmp     .loc_complete
 
.loc_exp_divide:
    fdiv    st1,st0
    loop    .loc_exp_divide
 
.loc_complete:
    ffree   st0     ; Почистить стек
    fincstp
 
    ; В начале строки минус?
    mov     esi,[lpStr]
    cmp     byte [esi],'-'
    jne     @f
 
    ; Изменить знак числа
    fchs
@@:
    ; Записать значение в ячейку памяти
    mov     eax,[lpResult]
    ; Если требуется повышенная точность, то приемник
    ; должен иметь размер QWORD, а следующую команду
    ; надо заменить на fstp qword [eax]
    fstp    tword [eax]
 
    ; Успешное преобразование
    mov     [digit],1
.loc_ret:
    popa
 
    ; Результат преобразования
    mov     eax,[digit]
 
    ret
endp