  use32 	     ; включить 32-битный режим ассемблера
  org	 0x0	     ; адресация с нуля

  db	 'MENUET01'  ; 8-байтный идентификатор MenuetOS
  dd	 0x01	     ; версия заголовка (всегда 1)
  dd	 START	     ; адрес первой команды
  dd	 I_END	     ; размер программы
  dd	 0x1000      ; количество памяти
  dd	 0x1000      ; адрес вершины стэка
  dd	 0x0	     ; адрес буфера для параметров (не используется)
  dd	 0x0	     ; зарезервировано

include 'lang.inc'
include 'macros.inc' ; макросы облегчают жизнь ассемблерщиков!


START:
red:

    call draw_window

still:
    mcall 10		; функция 10 - ждать события

    cmp  eax,1		; перерисовать окно ?
    je	 red		; если да - на метку red
    cmp  eax,2		; нажата клавиша ?
    je	 key		; если да - на key
    cmp  eax,3		; нажата кнопка ?
    je	 button 	; если да - на button

    jmp  still		; если другое событие - в начало цикла

;---------------------------------------------------------------------

 key:		       ; нажата клавиша на клавиатуре
    mov   eax, 2
    int   0x40
    mov   edi, [index]
    cmp   edi, string1
    jz	  no_back

    cmp   ah, 8
    jnz   no_back

    dec   edi
    mov   byte [edi], ' '
    mov   [index], edi
    call  Draw_String
    jmp   still
no_back:
    cmp   ah, 13
    jz	  read_str

    cmp   edi, string1_end
    jae   still
    mov   al, ah
    stosb
    mov   [index], edi
    call  Draw_String
    jmp  still		; вернуться к началу цикла

read_str:
    dec   edi
    mov   esi, string1

    call  GetBase

    xor   ecx, ecx
    inc   ecx		; ecx = 1

make_bin:
    xor   eax, eax

next_digit:
    xor   edx, edx
    cmp   edi, esi
    jb	  .done

    mov   dl, [edi]
    cmp   dl, '-'
    jne   @f
    neg   eax
    jmp   .done
@@:
    cmp   dl, 'F'
    jbe   @f
    and   dl, 11011111b
@@:
    sub   dl, '0'
    cmp   dl, 9
    jbe   @f
    sub   dl, 'A'-'9'-1
@@:
    cmp   dl, bl
    jb	  @f
    ; Здесь обработать ошибку

    jmp   .done
@@:
    push  ecx
    xchg  eax, ecx
    mul   edx ;        edx:eax = eax * edx
    add   ecx, eax
    pop   eax
    mul   ebx
    xchg  eax, ecx
    dec   edi
    jmp   next_digit

.done:
   mov	  [num], eax

   jmp	  red

;---------------------------------------------------------------------

  button:
    mcall 17		; 17 - получить идентификатор нажатой кнопки
    cmp   ah, 1 	; если НЕ нажата кнопка с номером 1,
    jne   still 	;  вернуться

  .exit:
    mcall -1		; иначе конец программы

draw_window:
    mcall 12, 1
    mcall 0, 200*65536+300, 200*65536+120, 0x02AABBCC, 0x805080D0, 0x005080D0
    mcall 4, 8*65536+8, 0x10DDEEFF, header, header_e - header
    mcall  , 15*65536+100, 0, numstr, 7
    mcall  , 15*65536+72,   , hexstr, 4
    mcall  , 15*65536+44,   , decstr,
    mcall  , 15*65536+30,   , binstr,
    mcall  , 15*65536+58,   ,sdecstr, 10


    mcall 8, (300-19)*65536+ 12, 5*65536+ 12, 1, 0x6688DD

    mov    ecx, [num]

    mcall  47, 8*65536+256,,240*65536+72,0    ; 16-ная

    mcall    , 10*65536,   ,228*65536+44,     ; 10-ная

    mcall    , 8*65536+512,,240*65536+30,     ; 2-ная
    ror    ecx, 8
    mcall    , 8*65536+512,,(240-56)*65536+30,
    ror    ecx, 8
    mcall    , 8*65536+512,,(240-56*2)*65536+30,
    ror    ecx, 8
    mcall    , 8*65536+512,,(240-56*3)*65536+30,
    ror    ecx, 8
    mov    byte [minus], '+'
    jnc    @f
    mov    byte [minus], '-'
    neg    ecx
@@:
    mcall   ,  10*65536,,228*65536+58,	      ; 10-ная со знаком
    mcall  4, 222*65536+58, 0, minus, 1
    call Draw_String


    mcall 12, 2 		   ; функция 12: сообщить ОС об отрисовке окна

ret

;-------------------------------------------------
    Draw_String:
;-------------------------------------------------
    mov  eax, 13
    mov  ebx, 60*65536+ 6*33
    mov  ecx, 100*65536+ 9
    mov  edx, 0x02AABBCC
    int  0x40

    mov  eax,4
    mov  ebx,60*65536+100
    mov  ecx,0xA40473
    mov  edx,string1
    mov  esi,33
    int  0x40
ret

;-------------------------------------------------
    GetBase:
;-------------------------------------------------
    mov   ebx, 10
    cmp   edi, esi
    jb	  .done

    mov   al, [edi]
    cmp   al, 'H'
    jbe   @f
    and   al, 11011111b
@@:
    cmp   al, 'H'
    jne   @f
    mov   ebx, 16
    dec   edi
    jmp   .done

@@:
    cmp   al, 'D'
    jne   @f
    mov   ebx, 10
    dec   edi
    jmp   .done

@@:
    cmp   al, 'B'
    jne   .done
    mov   ebx, 2
    dec   edi

.done:
ret

;-------------------------------------------------
string1:
  db  34 dup(' ')
string1_end:
  index dd string1
num  dd  0


header db 'hex2dec2bin'
header_e:
 minus	db '-'
 numstr db 'Number:'
 hexstr db 'hex:'
 binstr db 'bin:'
 decstr db 'dec:'
sdecstr db 'signed dec:'

I_END:				   ; метка конца программы
