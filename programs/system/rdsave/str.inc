; by ManHunter / PCL 
; http://www.manhunter.ru 
;-----------------------------------------------------
; Функция получения длины строки (Fast)
;-----------------------------------------------------
; lpStr - указатель на строку ASCIIZ
; На выходе: EAX - длина строки без учета завершающего
; нулевого байта
;-----------------------------------------------------
;proc    _lstrlen lpStr:DWORD
;        mov     eax, [lpStr]
;        sub     eax, 4
;@@:
;        add     eax, 4
;        cmp     byte [eax], 0
;        je      .szlen_lb1
;        cmp     byte [eax+1], 0
;        je      .szlen_lb2
;        cmp     byte [eax+2], 0
;        je      .szlen_lb3
;        cmp     byte [eax+3], 0
;        jne     @b
;        sub     eax, [lpStr]
;        add     eax, 3
;        ret
;.szlen_lb3:
;        sub     eax, [lpStr]
;        add     eax, 2
;        ret
;.szlen_lb2:
;        sub     eax, [lpStr]
;        add     eax, 1
;        ret
;.szlen_lb1:
;        sub     eax, [lpStr]
;        ret
;endp

;----------------------------------------------------- 
; Функция получения длины строки 
;----------------------------------------------------- 
; lpStr - указатель на строку ASCIIZ 
; На выходе: EAX - длина строки без учета завершающего 
; нулевого байта 
;-----------------------------------------------------
proc _lstrlen lpStr:DWORD
        mov     eax, [lpStr]
@@:     inc     eax
        cmp     byte [eax], 0
        jne     @b
        sub     eax, [lpStr]
        ret
endp
;----------------------------------------------------- 
; Функция быстрого слияния двух строк 
; используются функции _lstrlen, _lstrcpy 
;----------------------------------------------------- 
; lpDst - указатель на исходную строку ASCIIZ 
; lpSrc - указатель на добавляемую строку ASCIIZ 
;----------------------------------------------------- 
proc    _lstrcat lpDst:DWORD, lpSrc:DWORD 
        pusha 
  
        stdcall _lstrlen,[lpDst] 
        add     eax,[lpDst] 
  
        stdcall _lstrcpy,eax,[lpSrc] 
  
        popa 
        ret 
endp 

;----------------------------------------------------- 
; Функция быстрого копирования строки 
; используются функции _lstrlen, _memcopy 
;----------------------------------------------------- 
; lpDst - указатель на приемник 
; lpSrc - указатель на строку ASCIIZ 
;----------------------------------------------------- 
proc    _lstrcpy lpDst:DWORD, lpSrc:DWORD 
        pusha 
  
        stdcall _lstrlen,[lpSrc] 
        inc     eax 
        stdcall _memcopy,[lpDst],[lpSrc],eax 
  
        popa 
        ret 
endp 

;----------------------------------------------------- 
; Функция быстрого копирования участка памяти 
;----------------------------------------------------- 
; lpDst - указатель на приемник 
; lpSrc - указатель на источник 
; dSize - размер копируемого блока 
;----------------------------------------------------- 
proc    _memcopy lpDst:DWORD, lpSrc:DWORD, dSize:DWORD 
        pusha 
  
        ; Установить указатели на источник и приемник 
        cld 
        mov     edi,[lpDst] 
        mov     esi,[lpSrc] 
  
        mov     ecx,[dSize] 
        push    ecx 
        ; Разделить на 4 и получить длину в DWORD 
        shr     ecx,2 
        ; Скопировать основную часть строки DWORD'ами 
        rep     movsd 
        pop     ecx 
        ; Получить остаток от деления на 4 
        and     ecx,3 
        ; Скопировать остаток строки байтами 
        rep     movsb 
  
        popa 
        ret 
endp 
