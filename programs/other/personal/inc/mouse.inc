mouse_local:
    ;.................................................................................................
    ; ѕолучаем координаты мыши относительно окна
    ;.................................................................................................
    mov     eax,37                        ; ‘ункци€ 37 - работа с мышью
    mov     ebx,1                         ; нам нужны глобальные координаты
    int     0x40                          ; eax = x*65536 + y, (x,y)=координаты курсора мыши
    mov     ecx,eax                       ;
    shr     ecx,16                        ; ecx = x+1
    movzx   edx,ax                        ; edx = y+1
    dec     ecx                           ; ecx = x
    dec     edx                           ; edx = y
    mov     [mouse_x],ecx                 ; mouse_x = x
    mov     [mouse_y],edx                 ; mouse_y = y
    ret                                   ; ¬озвращаем управление
;end_mouse_local

mouse_left:
    pusha
    xor     ecx,ecx
    mov     eax,37
    mov     ebx,2
    int     0x40
    cmp     al,1
    jne     @f
    mov     ecx,1
  @@:
    mov     [mouse_l],ecx
    popa
    ret
;end