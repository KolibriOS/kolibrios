;
; нарисовать прямоугольник
; x1,y1 ---------|
; |              |
; |              |
; |-------------x2,y2
;
  macro rect x1, y1, x2, y2, color
   {
     pushad

     mov edx, color
     ; ------------
     mov eax, 38
     mov ebx, x1
     shl ebx, 16
     add ebx, x2
     mov ecx, y1
     shl ecx, 16
     add ecx, y1
     int 40h

     ; ------------
     mov eax, 38
     mov ebx, x1
     shl ebx, 16
     add ebx, x2
     mov ecx, y2
     shl ecx, 16
     add ecx, y2
     int 40h
     ; |
     ; |
     ; |
     mov eax, 38
     mov ebx, x1
     shl ebx, 16
     add ebx, x1
     mov ecx, y1
     shl ecx, 16
     add ecx, y2
     int 40h
     ; |
     ; |
     ; |
     mov eax, 38
     mov ebx, x2
     shl ebx, 16
     add ebx, x2
     mov ecx, y1
     shl ecx, 16
     add ecx, y2
     int 40h

     popad
   }

;
;   Вывод на экран буфера со строками
;

scbuff db 512*128 dup 0
; 512 - длина строки
; 128 - количество строк
;
ind db 0   ; Текущий индекс
;
;
x_s dw 15   ;  Координаты левого верхнего угла  FIXIT
y_s dw 38  ;

; Высота строки 
SH = 10
              
xlen dw 512   ; длина строки
;ylen dw 128   ; количество строк
ylen dw 40

;Последние 4 байта в строке обозначают цвет

  printbuff:
    pushad

    ;
    ; Нарисуем белый прямоугольник
    ;
    mov eax, 13
    mov ebx, 15*65536+480
    mov ecx, 31*65536+418
    mov edx, 0FFFFFFh
    int 40h



    xor ecx, ecx ; Счетчик
    ;xor eax, eax
    xor ebx, ebx
    ;xor edx, edx

  pb_loop:
    xor edx, edx
    xor eax, eax
    mov dl, [ind]     ;  Индекс
    mov ax, [ylen]
    ;mul edx
    cmp ecx, eax
    ja pr_end
    ;
    ;
    add edx, ecx    ;Прибавим счетчик
    xor eax, eax
    mov ax, [ylen]
    cmp edx, eax    ;Индекс меньше количества строк
    jna @f           ;<=
    sub edx, eax    ;Если больше, считаем с начала
    dec edx
  @@:
    ;
    mov bx, [x_s]    ; Координата X
    shl ebx, 16      ;
    push ecx
    mov eax, SH       ; Вычисляем местоположение строки
    imul eax, ecx     ;
    ;mov ecx, eax
    ;xor eax, eax

    ;mov ax , [x_s]
    xor ecx, ecx
    mov cx, [y_s]
    add ecx, eax
    add ebx, ecx    ; Координата Y
    ;
    xor eax, eax
    mov ax, [xlen]  ;длина строки
    imul edx, eax         ;Умножаем счетчик на длину строки, получаем смещение относительно начала буфера
    ;mov edx, eax
    add edx, scbuff

    xor eax, eax
    mov ax, [xlen]
    sub eax, 4
    xor ecx, ecx
    mov ecx, dword [edx+eax]  ; Последние 4 Байта с цветом
    or ecx, 0x80000000        ; Выводить ASCIIZ
    mov eax, 4
   mov esi, -1               ; For Menuet
    int 40h
    pop ecx
    inc ecx
    jmp pb_loop


  pr_end:
    popad
  ret

;
;  Отладочные сообщения
;
  macro write_debug str 
    {
     local ..string, ..label, ..end, ..loop, ..fin
     jmp ..label

     ..string db str, 0

    ..label:
     pushad
     xor eax, eax
     xor ebx, ebx
     xor ecx, ecx
     xor edx, edx

     mov bl, [ind]
     mov ax, [xlen]
     imul ebx, eax 
     add ebx, scbuff

   ..loop:
     mov dl, [..string+ecx]
     cmp dl, 0
     jz ..end
     mov [ebx+ecx], dl
     inc ecx
     jmp ..loop
     
   ..end:
     mov [ebx+ecx], dl
     xor ebx, ebx
     mov bl, [ind]
     cmp bx, [ylen]
     jz @f
     inc bl
     jmp ..fin
   @@:
    xor bl, bl

   ..fin:
     mov [ind], bl
     call printbuff
     ;call draw_window
     popad
    }

;
;  Кнопка
;
  macro draw_button x, y, xlen, ylen, id, str
   {
    pushad
    local ..string, ..label 
    jmp ..label
    ..string db str, 0

    ..label:
    mov eax, 8
    mov ebx, x*65536+xlen
    mov ecx, y*65536+ylen
    mov edx, id
    mov esi, 0x4466aa
    int 40h

    mov eax, 4
    mov ebx, (x+5)*65536+y+ylen/2-3
    xor ecx, ecx
    or ecx, 0x80FFFFFF
    mov edx, ..string
    int 40h

    popad
   }

;
; Отладочный вывод данных
;
  macro data_debug str, rg
    {
     pushad
     local ..string, ..end, ..loop, ..strend, ..fin, ..label
     jmp ..label
     ..string db str, 20h, 0, 0, 0, 0, 0, 0, 0, 0, 0
     ..strend:

     ..label:
     ;xor eax, eax
     ;xor ebx, ebx
     xor ecx, ecx
     xor edx, edx

     mov eax, rg
     lea ebx, [..strend-9]
     call int2str

     xor eax, eax
     xor ebx, ebx

     mov bl, [ind]
     mov ax, [xlen]
     imul ebx, eax 
     add ebx, scbuff

   ..loop:
     mov dl, [..string+ecx]
     cmp dl, 0
     jz ..end
     mov [ebx+ecx], dl
     inc ecx
     jmp ..loop
     
   ..end:
     mov [ebx+ecx], dl
     xor ebx, ebx
     mov bl, [ind]
     cmp bx, [ylen]
     jz @f
     inc bl
     jmp ..fin
    @@:
     xor bl, bl

    ..fin:
     mov [ind], bl



     call printbuff
     ;call draw_window

     popad
    }

; <--EAX
; -->[ebx]
;
  int2str:
    pushf
    ;push ebx
    push ecx
    push edx
    push esi

    ;xor ebx, ebx
    xor ecx, ecx

   i_loop:
    mov edx, eax
    push ecx
    shl ecx, 2
    mov esi, 28
    sub esi, ecx
    mov ecx, esi
    shr edx, cl
    pop ecx
    and dl, 0fh    ;Отделить младшие 4 бита
    cmp dl, 0Ah    ;Буква
    jnc @f
    or dl, 0x30
    jmp i_n1

  @@:
    sub dl, 9
    or dl, 0x40
  i_n1:
    mov [ebx+ecx], dl
    inc ecx
    cmp ecx, 8
    jnz i_loop

    
    pop esi
    pop edx
    pop ecx
    ;pop ebx
    popf
  ret

;
;
CP866 db 'АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюя'

;
;  Перекодировка из cp1251 в cp866
;  
;  [eax] <-- Null-terminated string
; 

  win2dos:
    pushad
    pushf

    xor ebx, ebx
    xor ecx, ecx
    ;xor edx, edx

  w2d_loop:
    mov bl, [eax+ecx]
    cmp bl, 0
    jz  w2d_end
    cmp bl, 0A8h              ; Ё
    jz  w2d_yo1
    cmp bl, 0B8h              ; ё
    jz  w2d_yo2
    cmp bl, 0C0h              ; Русская буква
    jnc w2d_rchar
    inc ecx
    jmp w2d_loop

  w2d_yo1:
    mov [eax+ecx], byte 0F0h
    inc ecx
    jmp w2d_loop

  w2d_yo2:
    mov [eax+ecx], byte 0F1h
    inc ecx
    jmp w2d_loop

  w2d_rchar:
    sub bl, 0C0h
    mov bl, [CP866+ebx]
    mov [eax+ecx], bl
    inc ecx
    jmp w2d_loop


  w2d_end:

    popf
    popad
  ret


CP1251 db '└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀рстуфхцчшщъыьэюяЁёЄєЇїЎў°∙·√№¤■ '

;
; Перекодировка из CP866 в CP1251
; [eax] <-- Null termainated string
;

   dos2win:
     pushf
     pushad

     xor ebx, ebx
     xor ecx, ecx

     dec ecx

   d2w_loop:
     inc ecx
     mov bl, [eax+ecx]
     cmp bl, 0
     jz  d2w_end
     cmp bl, 80h
     jnc d2w_rchar
     
     ;inc ecx
     jmp d2w_loop

   d2w_yo1:
     mov byte [eax+ecx], 0A8h
     ;inc ecx
     jmp d2w_loop

   d2w_yo2:
     mov byte [eax+ecx], 0B8h
     ;inc ecx
     jmp d2w_loop

   d2w_rchar:
     cmp bl, 0B0h
     jnc d2w_rchar2
     
     sub bl, 80h
     mov bl, [CP1251+ebx]
     mov [eax+ecx], bl
     jmp d2w_loop

   d2w_rchar2:
     cmp bl, 0E0h
     jc d2w_loop
     cmp bl, 0F0h
     jz d2w_yo1
     cmp bl, 0F1h
     jz d2w_yo2
     cmp bl, 0F2h
     jnc d2w_loop
     add bl, 10h
     mov [eax+ecx], bl
     jmp d2w_loop


     d2w_end:

     popad
     popf
   ret


;
;  Для вывода сообщений
;  [eax] <-- null-terminated string
;   ebx <-- color
;
  writemsg:
    pushad
     push ebx
     push eax

     xor eax, eax
     xor ebx, ebx

     mov bl, [ind]
     mov ax, [xlen]
     imul ebx, eax 
     add ebx, scbuff

     pop eax
     mov edx, eax
     call strlen
     mov ecx, eax
     mov eax, edx
     call strcpy

     mov [ebx+ecx], byte 0x00

     xor eax, eax
     mov ax, [xlen]
     sub eax, 4
     pop ecx
     mov dword [ebx+eax], ecx  ; Последние 4 Байта с цветом


     xor ebx, ebx
     mov bl, [ind]
     cmp bx, [ylen]
     jz @f
     inc bl
     jmp ..fin
   @@:
    xor bl, bl

   ..fin:
     mov [ind], bl

     call printbuff
     ;call draw_window

    popad
  ret

;
;



; <--EAX       Перевод из 16 в 10 форму
; -->[ebx]
; --> eax длина
  int2strd:
    pushf
    ;push ebx
    push ecx
    push edx
    push esi
    ;push edi

    ;xor ebx, ebx
    xor ecx, ecx
    mov esi, 10
    

   id_loop:
    xor edx, edx
    
    div esi

    and dl, 0fh    ;Отделить младшие 4 бита
    or dl, 0x30
    
    mov [ebx+ecx], dl
    cmp eax, 10
    inc ecx
    jnc id_loop

    and al, 0fh    ;Отделить младшие 4 бита
    or al, 0x30

    mov [ebx+ecx], al
    ;mov [ebx+ecx+1], byte 0
    inc ecx
    mov eax, ecx
    ; перевернуть полученную строку
    ;
    ;xor edx, edx

    ;mov esi, 2
    ;div esi
    shr eax, 1

    xor edx, edx

   id_loop2:
    cmp eax, 0
    jz id_end


    mov dl, [ebx+eax-1]
    lea esi, [ebx+ecx]
    sub esi, eax
    mov dh, [esi]

    mov [ebx+eax-1], dh
    mov [esi], dl

    dec eax
    jmp id_loop2




    id_end:
    mov eax, ecx

    ;pop edi
    pop esi
    pop edx
    pop ecx
    ;pop ebx
    popf
  ret

;
;
;
x_bb dw 550                ;  Координаты левого верхнего угла 
y_bb dw 30                ;
;
bb_width dw 100           ; Ширина кнопок
bb_height dw 15          ; высота кнопок
;
bb_dist  dw 10           ; Расстояние между кнопками

; Идентификаторы кнопок начиная с 100
;
;  Для КЛ
;
;
  buttonbox:
    pushad
    pushf
 
    xor ecx, ecx
    xor eax, eax
    xor ebx, ebx

  bb_loop:

    ; Проверяем первый байт уина, если 0, кнопку рисовать не надо
    mov ebx, NAME_LEN
    imul ebx, ecx

    mov al, [names+ebx]
    cmp al, 0
    jz bb_end
    ; рисуем кнопку
    ; Цвет зависит от статуса UIN
    mov ebx, 4
    imul ebx, ecx
    mov eax, [stats+ebx]     ; В старшем слове дополнительный статус
    cmp ax, -1
    jz bb_offline
    cmp ax, 1
    jz bb_away
    cmp ax, 2
    jz bb_dnd
    cmp ax, 4
    jz bb_na
    cmp ax, 10h
    jz bb_bisy
    cmp ax, 20h
    jz bb_free4chat
    cmp ax, 100h
    jz bb_invis
    ; Online
    mov esi, 0x4466AA
    jmp bb_dr

  bb_offline:
    mov esi, 0xBEBEBE
    jmp bb_dr

  bb_away:
    mov esi, 0x14dcc3
    jmp bb_dr

  bb_dnd:
    mov esi, 0x14dcc3
    jmp bb_dr

  bb_na:
    mov esi, 0x14dcc3
    jmp bb_dr

  bb_bisy:
    mov esi, 0x14dcc3
    jmp bb_dr

  bb_free4chat:
    mov esi, 0x2233FF
    jmp bb_dr

  bb_invis:
    mov esi, 0xD0D0D0

  bb_dr:

    mov bx, [x_bb]
    shl ebx, 16
    mov bx, [bb_width]
    ;push ecx
    mov edx, ecx
    lea edx, [edx+100]      ; ID
    mov edi, ecx
    mov cx, [y_bb]
    movzx eax, [bb_height]
    add ax, [bb_dist]
    imul eax, edi
    add ecx, eax
    shl ecx, 16
    mov cx, [bb_height]
    mov eax, 8
    int 40h
     
    ;
    ; Надпись на кнопке
    ;
    add ebx, 50000h     ; Смещение от левого края
    shr ecx, 16
    movzx eax, [bb_height]
    shr eax, 1          ;  /2
    sub eax, 3          ; смещение вверх от центра
    add ecx, eax        ; + половина размера кнопки
    mov bx, cx
    mov ecx, 0x80FFFFFF ; Цвет
    mov edx, names
    mov esi, NAME_LEN
    imul esi, edi
    add edx, esi
    ;mov esi, -1
    mov eax, 4
    int 40h

    mov ecx, edi
    inc ecx
    cmp ecx, UINS
    ja bb_end
    jmp bb_loop


  bb_end:
    popf
    popad
  ret
;
; Массив с UIN
;
UIN_LEN = 11            ; Длина
UINS    = 15            ; Количество
;
uins db UIN_LEN*UINS dup 0
; 
; массив со статусами
;
stats dd UINS dup -1
;
; Массив с именами
;
NAME_LEN = 30

names db NAME_LEN*UINS dup 0

;
;
U1 db '123456789',0
U2 db '123456789',0
U3 db '123456789',0
;
; Загрузка массива UIN
;
  loaduin:
    pushad
    mov eax, U1
    mov ebx, uins
    mov ecx, 9
    call strcpy

    mov ebx, names
    call strcpy

    mov eax, U2
    mov ebx, uins+UIN_LEN
    mov ecx, 9
    call strcpy

    mov ebx, names+NAME_LEN
    call strcpy

    mov eax, U3
    mov ebx, uins+UIN_LEN*2
    mov ecx, 9
    call strcpy

    mov ebx, names+NAME_LEN*2
    call strcpy


    popad
  ret

;
; Функция для загрузки КЛ никами и статусами
; eax    <--   Номер уина по порядку в массиве uins, начиная с 0
; [ebx]  <--   указатель на null-terminated строку с новым именем
; ecx    <--   Новый статус
;                                  
   loadbb:
     pushad
     pushf
     ;
     ; Проверяем номер
     ;
     cmp eax, UINS
     jnc loadbb_end        ;>=


     ;
     ; Удаляем кнопку
     ;
     push ecx
     push ebx
     push eax

     lea edx, [eax+100]
     or edx, 0x80000000
     mov eax, 8
     int 40h

     ;
     ; Сохраняем новый статус в массиве статусов
     ;
     pop eax
     mov edx, 4                 ; DWORD
     imul edx, eax
     mov [stats+edx], ecx
     ;
     ; Сохраняем новое имя
     ;
     mov edi, eax               ; edi <-- Uin number
     pop eax                    ; <-- offset of string
     mov ebx, eax
     call strlen
     mov ecx, eax               ; Len


     mov eax, ebx               ;Source
     mov edx, NAME_LEN
     imul edx, edi
     lea ebx, [names+edx]       ; Dest
     call strcpy
     mov [names+edx+ecx], byte 0

     xchg edi, eax              ; eax - счетик, edi - указатель на строку
     
     pop ecx
     push edi
     ; рисуем кнопку
     ; Цвет зависит от статуса UIN
    
     cmp cx, -1
     jz l_offline
     cmp cx, 1
     jz l_away
     cmp cx, 2
     jz l_dnd
     cmp cx, 4
     jz l_na
     cmp cx, 10h
     jz l_bisy
     cmp cx, 20h
     jz l_free4chat
     cmp cx, 100h
     jz l_invis
     ; Online
     mov esi, 0x4466AA
     jmp l_dr

   l_offline:
     mov esi, 0xBEBEBE
     jmp l_dr

   l_away:
     mov esi, 0x14dcc3
     jmp l_dr

   l_dnd:
     mov esi, 0x14dcc3
     jmp l_dr

   l_na:
     mov esi, 0x14dcc3
     jmp l_dr

   l_bisy:
     mov esi, 0x14dcc3
     jmp l_dr

   l_free4chat:
     mov esi, 0x2233FF
     jmp l_dr

   l_invis:
     mov esi, 0xD0D0D0

   l_dr:

     mov bx, [x_bb]
     shl ebx, 16
     mov bx, [bb_width]
     ;push ecx
     mov edx, eax
     lea edx, [edx+100]      ; ID
     mov edi, eax
     mov cx, [y_bb]
     movzx eax, [bb_height]
     add ax, [bb_dist]
     imul eax, edi
     add ecx, eax
     shl ecx, 16
     mov cx, [bb_height]
     mov eax, 8
     int 40h
     
    ;
    ; Надпись на кнопке
    ;
    add ebx, 50000h     ; Смещение от левого края
    shr ecx, 16
    movzx eax, [bb_height]
    shr eax, 1          ;  /2
    sub eax, 3          ; смещение вверх от центра
    add ecx, eax        ; + половина размера кнопки
    mov bx, cx
    mov ecx, 0x80FFFFFF ; Цвет
    pop edx
   mov esi, -1
    mov eax, 4
    int 40h



   loadbb_end:

     popf
     popad
   ret