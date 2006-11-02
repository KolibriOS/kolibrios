;    Пример реализации генератора MD5 - хеша
;
;    MD5 Generator
;    
;    Автор: Hex
;    Сайт: www.mestack.narod.ru
;    Идея, реализация и отладка.
;
;    Автор: Halyavin
;    Сайт: www.shade.msu.ru/~msu-se/home.html
;    Доработка, отладка и оптимизация.
;
;    Компилируеться Fasm'ом для МенуэтОС
include 'lang.inc'
macro diff16 title,l2
 {
  local s,d,l1
  s = l2
  display title,': 0x'
  repeat 8
   d = 48 + s shr ((8-%) shl 2) and $0F
   if d > 57
    d = d + 65-57-1
   end if
   display d
  end repeat
  display 13,10
 }    

use32

                org     0x0

  db     'MENUET01'  ; 8-байтный идентификатор MenuetOS
  dd     0x01        ; версия заголовка (всегда 1)
  dd     START       ; адрес первой команды
  dd     I_END       ; размер программы
  dd     0x100000    ; количество памяти
  dd     0x100000    ; адрес вершины стэка
  dd     0x0         ; адрес буфера для параметров (не используется)
  dd     0x0         ; зарезервировано

START:                                  ; Начало выполнения программы

        call draw_window            ; Сперва перерисуем окно

still:

    mov  eax,23                 ; Ожидаем событий
    mov  ebx,1
    int  0x40

    cmp  eax,1                  ; Запрос на перерисовку ?
    jz   red
    cmp  eax,2                  ; нажата клавиши ?
    jz   key
    cmp  eax,3                  ; нажата кнопка ?
    jz   button

    jmp  still

red:
    call draw_window
    jmp  still

key:
    mov  eax,2
    int  0x40
    jmp  still

button:
    mov  eax,17
    int  0x40

    cmp  ah,1                  ;  id кнопки = 1 ?
    jnz  noclose
    mov  eax,-1
    int  0x40

  noclose:

    cmp  ah,2                  ; Генерировать?
    je   procMD5hash

    jmp  still


procMD5hash:        

    ; phase I - padding
    mov    edi,ptBuffer
    mov    eax,[dtBufferLength]

    inc    eax
    add    edi,eax
    mov     byte [edi-1],0x80

    xor    edx,edx

    mov    ebx,64
    div    ebx

    neg    edx
    add    edx,64

    cmp    edx,8
    jae    @f

    add    edx,64
    
@@:    mov    ecx,edx
    xor    al,al
    rep    stosb

    mov    eax,[dtBufferLength]

    inc    edx
    add    [dtBufferLength],edx

    xor    edx,edx

    mov    ebx,8
    mul    ebx

    mov    [edi-8],eax
    mov    [edi-4],edx

    mov    edx,[dtBufferLength]

    mov    edi,ptBuffer

    ; phase II - chaining variables initialization
    mov     dword [dtA],067452301h
    mov     dword [dtB],0efcdab89h
    mov     dword [dtC],098badcfeh
    mov     dword [dtD],010325476h
       
    mov    esi,ptMD5Result

hashloop:
;diff16 "hashloop",hashloop
    mov    eax,[dtA]
    mov    [dta],eax
    mov    eax,[dtB]
    mov    [dtb],eax
    mov    eax,[dtC]
    mov    [dtc],eax
    mov    eax,[dtD]
    mov    [dtd],eax
    
macro ff dta,dtb,dtc,dtd,data,shift,cc
{
    mov     eax,dtb
    mov     ebx,dtc
    mov     ecx,dtd
    
    and     ebx,eax
    not     eax
    and     eax,ecx
    or      eax,ebx
    
    add     eax,dta
    add     eax,data
    add     eax,cc
    rol     eax,shift
    add     eax,dtb
    mov     dta,eax
}
macro gg dta,dtb,dtc,dtd,data,shift,cc
{
    mov     eax,dtb
    mov     ebx,dtc
    mov     ecx,dtd
    
    and     eax,ecx
    not     ecx
    and     ecx,ebx
    or      eax,ecx
    
    add     eax,dta
    add     eax,data
    add     eax,cc
    rol     eax,shift
    add     eax,dtb
    mov     dta,eax
}
macro hh dta,dtb,dtc,dtd,data,shift,cc
{
    mov     eax,dtb
    mov     ebx,dtc
    mov     ecx,dtd
    
    xor     eax,ebx
    xor     eax,ecx
    
    add     eax,dta
    add     eax,data
    add     eax,cc
    rol     eax,shift
    add     eax,dtb
    mov     dta,eax
}
macro ii dta,dtb,dtc,dtd,data,shift,cc
{
    mov     eax,dtb
    mov     ebx,dtc
    mov     ecx,dtd
    
    not     ecx
    or      eax,ecx
    xor     eax,ebx
    
    add     eax,dta
    add     eax,data
    add     eax,cc
    rol     eax,shift
    add     eax,dtb
    mov     dta,eax
}        
    ; round 1
    ff [dta],[dtb],[dtc],[dtd],dword [edi+00*4],07,0xd76aa478
    ff [dtd],[dta],[dtb],[dtc],dword [edi+01*4],12,0xe8c7b756
    ff [dtc],[dtd],[dta],[dtb],dword [edi+02*4],17,0x242070db
    ff [dtb],[dtc],[dtd],[dta],dword [edi+03*4],22,0xc1bdceee
    ff [dta],[dtb],[dtc],[dtd],dword [edi+04*4],07,0xf57c0faf
    ff [dtd],[dta],[dtb],[dtc],dword [edi+05*4],12,0x4787c62a
    ff [dtc],[dtd],[dta],[dtb],dword [edi+06*4],17,0xa8304613
    ff [dtb],[dtc],[dtd],[dta],dword [edi+07*4],22,0xfd469501
    ff [dta],[dtb],[dtc],[dtd],dword [edi+08*4],07,0x698098d8
    ff [dtd],[dta],[dtb],[dtc],dword [edi+09*4],12,0x8b44f7af
    ff [dtc],[dtd],[dta],[dtb],dword [edi+10*4],17,0xffff5bb1
    ff [dtb],[dtc],[dtd],[dta],dword [edi+11*4],22,0x895cd7be
    ff [dta],[dtb],[dtc],[dtd],dword [edi+12*4],07,0x6b901122
    ff [dtd],[dta],[dtb],[dtc],dword [edi+13*4],12,0xfd987193
    ff [dtc],[dtd],[dta],[dtb],dword [edi+14*4],17,0xa679438e
    ff [dtb],[dtc],[dtd],[dta],dword [edi+15*4],22,0x49b40821
    ; round 2
    gg [dta],[dtb],[dtc],[dtd],dword [edi+01*4],05,0xf61e2562
    gg [dtd],[dta],[dtb],[dtc],dword [edi+06*4],09,0xc040b340
    gg [dtc],[dtd],[dta],[dtb],dword [edi+11*4],14,0x265e5a51
    gg [dtb],[dtc],[dtd],[dta],dword [edi+00*4],20,0xe9b6c7aa
    gg [dta],[dtb],[dtc],[dtd],dword [edi+05*4],05,0xd62f105d
    gg [dtd],[dta],[dtb],[dtc],dword [edi+10*4],09,0x02441453
    gg [dtc],[dtd],[dta],[dtb],dword [edi+15*4],14,0xd8a1e681
    gg [dtb],[dtc],[dtd],[dta],dword [edi+04*4],20,0xe7d3fbc8
    gg [dta],[dtb],[dtc],[dtd],dword [edi+09*4],05,0x21e1cde6
    gg [dtd],[dta],[dtb],[dtc],dword [edi+14*4],09,0xc33707d6
    gg [dtc],[dtd],[dta],[dtb],dword [edi+03*4],14,0xf4d50d87
    gg [dtb],[dtc],[dtd],[dta],dword [edi+08*4],20,0x455a14ed
    gg [dta],[dtb],[dtc],[dtd],dword [edi+13*4],05,0xa9e3e905
    gg [dtd],[dta],[dtb],[dtc],dword [edi+02*4],09,0xfcefa3f8
    gg [dtc],[dtd],[dta],[dtb],dword [edi+07*4],14,0x676f02d9
    gg [dtb],[dtc],[dtd],[dta],dword [edi+12*4],20,0x8d2a4c8a
    ; round 3
    hh [dta],[dtb],[dtc],[dtd],dword [edi+05*4],04,0xfffa3942
    hh [dtd],[dta],[dtb],[dtc],dword [edi+08*4],11,0x8771f681
    hh [dtc],[dtd],[dta],[dtb],dword [edi+11*4],16,0x6d9d6122
    hh [dtb],[dtc],[dtd],[dta],dword [edi+14*4],23,0xfde5380c
    hh [dta],[dtb],[dtc],[dtd],dword [edi+01*4],04,0xa4beea44
    hh [dtd],[dta],[dtb],[dtc],dword [edi+04*4],11,0x4bdecfa9
    hh [dtc],[dtd],[dta],[dtb],dword [edi+07*4],16,0xf6bb4b60
    hh [dtb],[dtc],[dtd],[dta],dword [edi+10*4],23,0xbebfbc70
    hh [dta],[dtb],[dtc],[dtd],dword [edi+13*4],04,0x289b7ec6
    hh [dtd],[dta],[dtb],[dtc],dword [edi+00*4],11,0xeaa127fa
    hh [dtc],[dtd],[dta],[dtb],dword [edi+03*4],16,0xd4ef3085
    hh [dtb],[dtc],[dtd],[dta],dword [edi+06*4],23,0x04881d05
    hh [dta],[dtb],[dtc],[dtd],dword [edi+09*4],04,0xd9d4d039
    hh [dtd],[dta],[dtb],[dtc],dword [edi+12*4],11,0xe6db99e5
    hh [dtc],[dtd],[dta],[dtb],dword [edi+15*4],16,0x1fa27cf8
    hh [dtb],[dtc],[dtd],[dta],dword [edi+02*4],23,0xc4ac5665
    ; round 4
    ii [dta],[dtb],[dtc],[dtd],dword [edi+00*4],06,0xf4292244
    ii [dtd],[dta],[dtb],[dtc],dword [edi+07*4],10,0x432aff97
    ii [dtc],[dtd],[dta],[dtb],dword [edi+14*4],15,0xab9423a7
    ii [dtb],[dtc],[dtd],[dta],dword [edi+05*4],21,0xfc93a039
    ii [dta],[dtb],[dtc],[dtd],dword [edi+12*4],06,0x655b59c3
    ii [dtd],[dta],[dtb],[dtc],dword [edi+03*4],10,0x8f0ccc92
    ii [dtc],[dtd],[dta],[dtb],dword [edi+10*4],15,0xffeff47d
    ii [dtb],[dtc],[dtd],[dta],dword [edi+01*4],21,0x85845dd1
    ii [dta],[dtb],[dtc],[dtd],dword [edi+08*4],06,0x6fa87e4f
    ii [dtd],[dta],[dtb],[dtc],dword [edi+15*4],10,0xfe2ce6e0
    ii [dtc],[dtd],[dta],[dtb],dword [edi+06*4],15,0xa3014314
    ii [dtb],[dtc],[dtd],[dta],dword [edi+13*4],21,0x4e0811a1
    ii [dta],[dtb],[dtc],[dtd],dword [edi+04*4],06,0xf7537e82
    ii [dtd],[dta],[dtb],[dtc],dword [edi+11*4],10,0xbd3af235
    ii [dtc],[dtd],[dta],[dtb],dword [edi+02*4],15,0x2ad7d2bb
    ii [dtb],[dtc],[dtd],[dta],dword [edi+09*4],21,0xeb86d391
    
    mov    eax,[dta]
    add    [dtA],eax
    mov    eax,[dtb]
    add    [dtB],eax
    mov    eax,[dtc]
    add    [dtC],eax
    mov    eax,[dtd]
    add    [dtD],eax

    add    edi,64

    sub    edx,64
    jnz    hashloop

    ; phase IV - results

    mov    ecx,4
    mov     esi,ptMD5Result

@@:    mov    eax,[esi]
    xchg    al,ah
    rol    eax,16
    xchg    al,ah
    mov    [esi],eax

    add    esi,4
    loop    @b

translate:
;diff16 "translate",translate
    mov     esi,ptMD5Result-5
    mov     edi,hexresult
    mov     ecx,16
@@:
    test    ecx,3
    jnz     .nojmp
    add     esi,8
.nojmp:
    xor     eax,eax
    mov     al,byte [esi]
    mov     edx,eax
    shr     eax,4
    mov     bl,byte [table+eax]
    mov     [edi],bl
    inc     edi
    and     edx,15
    mov     bl,byte [table+edx]
    mov     [edi],bl
    dec     esi
    inc     edi
    loop    @b
    
    mov    esi,hexresult
     
        mov     [text], esi
    mov     eax,32
        mov     [textlen], eax
    call    draw_window

    jmp     still

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov eax,12               ; function 12:tell os about windowdraw
    mov ebx,1                      ; 1, start of draw
    int 0x40
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+230         ; [x start] *65536 + [x size]
    mov  ecx,60*65536+100          ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB
    mov  esi,0x80aabbcc            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x00aabbcc            ; color of frames    RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; color of text RRGGBB
    mov  edx,labeltext             ; pointer to text beginning
    mov  esi,lte-labeltext         ; text length
    int  0x40
                   ; Рисуем кнопку для генерации
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,20*65536+80           ; [x start] *65536 + [x size]
    mov  ecx,34*65536+14           ; [y start] *65536 + [y size]
    mov  edx,2                     ; button id
    mov  esi,0x5588dd              ; button color RRGGBB
    int  0x40
    
                                   ; Название на кнопку
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,23*65536+38           ; [x start] *65536 + [y start]
    mov  ecx,0x000000              ; color of text RRGGBB
    mov  edx,gen_txt               ; pointer to text beginning
    mov  esi,gen_len-gen_txt       ; text length
    int  0x40

    mov  eax,4               ; draw info text with function 4
    mov  ebx,20*65536+70
    mov  ecx,0x000000
    mov  edx,[text]
    xor  eax,eax
    mov  al, [textlen]
    mov  esi,eax
    mov  eax,4
    int  0x40

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret

;Область данных

labeltext: db 'MD5 Generator'
lte:

text:  dd 0
textlen: dd 0

gen_txt: db 'Сгенерировать'
gen_len: 

InputMD5Rez:  dd 0
InputMD5Rezlen:

ptBuffer:  db '123' ;Заменить на генерируемое слово
rb 61
dtBufferLength: dd 3 ;Размер ptBuffer

ptMD5Result:

dtA:  dd 0
dtB:  dd 0
dtC:  dd 0
dtD:  dd 0

dta:  dd 0
dtb:  dd 0
dtc:  dd 0
dtd:  dd 0

x: dd 0
s: dd 0
t: dd 0

table: db '0123456789abcdef'
hexresult db 32

I_END:
