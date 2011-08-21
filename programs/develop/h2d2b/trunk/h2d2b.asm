;;      h2d2b v0.3 system colors by Leency  ;;
;;      21.08.2011                          ;;

;;      hex2dec2bin 0.2 by Alexei Ershov    ;;
;;      16.11.2006                          ;;

use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,0,0

include '../../../proc32.inc'
include '../../../macros.inc' ; макросы облегчают жизнь ассемблерщиков!


start:
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
    mcall
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
   mov	  [num], eax	; сохраняем введенное число
   xor	  eax, eax
   mov	  al, ' '
   mov	  edi, string1
   mov	  [index], edi
   mov	  ecx, string1_end-string1
   rep	  stosb 	; стираем введенную строку
   jmp	  red

;---------------------------------------------------------------------

  button:
    mcall 17		; 17 - получить идентификатор нажатой кнопки
    cmp   ah, 1 	; если НЕ нажата кнопка с номером 1,
    jne   @f
    mcall -1
@@:
    cmp   ah, 2
    jne   @f
    shl   [num], 1
    jmp   red
@@:
    cmp   ah, 3
    jne   @f
    shr   [num], 1
    jmp   red
@@:
    cmp   ah, 4
    jne   still
    sar   [num], 1
    jmp   red

;------------------------------------------------
    draw_window:
;------------------------------------------------
	mcall	48, 3, sys_colors, 40

    mcall 12, 1
	mov	edx, 0x14000000
	or	edx, [sys_colors.work]
	;mov	esi, 0x80000000
	;or	esi, [sys_colors.grab_text]
    mcall 0, 200*65536+300, 200*65536+175, ,,title

	
    mcall  8, 15      *65536+ 38, 90*65536+ 15, 2, [sys_colors.work_button] ; кнопка shl
    mcall  ,		       ,110*65536+ 15,	,	   ; кнопка sal
    mcall  , (300-53)*65536+ 38, 90*65536+ 15, 3,	   ; кнопка shr
    mcall  ,		       ,110*65536+ 15, 4,	   ; кнопка sar

	mov	ecx, 0x80000000
	or	ecx, [sys_colors.work_text]
    mcall  4, 15*65536+30,   , binstr,
    mcall  , 15*65536+44,   , decstr,
    mcall  , 15*65536+58,   ,sdecstr,
    mcall  , 15*65536+72,   , hexstr,
    mcall  , 15*65536+150,  , numstr,

	mov	ecx, 0x80000000
	or	ecx, [sys_colors.work_button_text]
    mcall  ,	   26*65536+94, 	, shlstr,3
    mcall  , (300-42)*65536+94, 	, shrstr,
    mcall  ,	   26*65536+114,	, salstr,
    mcall  , (300-42)*65536+114,	, sarstr,
    mov    ecx, [num]

	
    mcall  47, 8*65536+256,,240*65536+72,[sys_colors.work_text]    ; 16-ная
    mcall    , 10*65536,   ,228*65536+44,     ; 10-ная
    mcall    , 8*65536+512,,240*65536+30,     ; 2-ная
    ror    ecx, 8
    mcall    ,		  ,,(240-56)*65536+30,
    ror    ecx, 8
    mcall    ,		  ,,(240-56*2)*65536+30,
    ror    ecx, 8
    mcall    ,		  ,,(240-56*3)*65536+30,
    ror    ecx, 8
    mov    [minus], '+'
    jnc    @f
    mov    [minus], '-'
    neg    ecx
@@:
    mcall   ,  10*65536,,228*65536+58,	      ; 10-ная со знаком
    mcall  4, 222*65536+58, 0, minus, 1
    mcall 38, 15*65536+300-15, 137*65536+137, [sys_colors.work_graph]
    call   Draw_String
    mcall 12, 2 		   ; функция 12: сообщить ОС об отрисовке окна

ret

;-------------------------------------------------
    Draw_String:
;-------------------------------------------------

     mcall   4, 60*65536+150, 0x40A40473, \
	     string1,string1_end-string1,0x00AABBCC
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
  num	dd  0


 title db 'hex2dec2bin 0.3',0
 minus	db '-',0
 numstr db 'Number:',0
 hexstr db 'hex:',0
 binstr db 'bin:',0
 decstr db 'dec:',0
sdecstr db 'signed dec:',0
 shlstr db 'shl',0
 salstr db 'sal',0
 shrstr db 'shr',0
 sarstr db 'sar',0

i_end:
 sys_colors		system_colors
 rb 0x400					;stack
 
e_end:				   ; метка конца программы
