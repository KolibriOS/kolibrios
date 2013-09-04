; Calendar for KolibriOS
;
; v1.35 - code update, redesign by Leency
; v1.1 - add change time support by DedOK 
; v1.0 - written in pure assembler by Ivushkin Andrey aka Willow
; also - diamond, spraid, fedesco
;
; Created: November 1, 2004


use32		     ; включить 32-битный режим ассемблера

  org	 0x0	     ; адресация с нуля

  db	 'MENUET01'  ; 8-байтный идентификатор MenuetOS
  dd	 0x01	     ; версия заголовка (всегда 1)
  dd	 start	     ; адрес метки, с которой начинается выполнение программ
  dd	 I_END	     ; размер программы
  dd	 0x1000      ; количество памяти
  dd	 0x1000      ; адрес вершины стэка
  dd	 0x0	     ; адрес буфера для строки параметров (не используется)
  dd	 0x0	     ; зарезервировано
include 'lang.inc'
include '..\..\..\macros.inc' ; уменьшает размер программы
include 'data.inc'



macro ShowFocus field,reg
{
   local  .nofocus, .exit
     cmp  [focus],field
     jne  .nofocus
   if reg eq
     mov  ecx,COL_ACTIVE_TEXT
   else
     mov  reg,COL_ACTIVE_TEXT
   end if
     jmp  .exit
   .nofocus:
   if reg eq
     mov  ecx,COL_DROPDOWN_T
   else
     mov  reg,COL_DROPDOWN_T
   end if
   .exit:
}

macro GetSkinHeight
{
	mov  eax,48
	mov  ebx,4
	int 0x40
}


str2int:
    xor  eax,eax
    lodsb
    mov  ebx,eax
    shr  ebx,4
    and  eax,0xf
    imul ebx,10
    add  al,bl
    ret

start:
    mcall 29
    mov  [datestr],eax
    mov  esi,datestr
    call str2int
    add  eax,1900
    mov  [Year],eax
    call str2int
    dec  eax
    mov  [Month],eax
    call str2int
    mov  [day_sel],eax
    test byte[esi],0
    jnz  .no2000
    add  [Year],100
  .no2000:
    jmp  upd		; здесь начинается выполнение программы
red:			; перерисовать окно

    call draw_window

still:			; ГЛАВНЫЙ ЦИКЛ ПРОГРАММЫ

    mcall 23,50     ; wait here for event
  .evt:
    mov  ebp,[focus]
    cmp  eax,1		; перерисовать окно ?
    je	 red		; если да - на метку red
    cmp  eax,2		; нажата клавиша ?
    je	 key		; если да - на key
    cmp  eax,3		; нажата кнопка ?
    je	 button 	; если да - на button

    call draw_clock

    jmp  still		; если другое событие - в начало цикла

  key:
    mcall 2		; get pressed key
    cmp  ah,9
    jne  no_tab
  .tab:
    cmp  ebp,FOCUSABLE
    JAE	 foc_cycle
    inc  [focus]
  upd:
    call calculate
    jmp  red
  foc_cycle:
    mov  [focus],2
    jmp  upd
  no_tab:
    push eax
    shr  eax,8
    mov  ecx,12
    mov  edi,Fkeys
    repne scasb
    pop  eax
    jnz  .noFkey
    sub  edi,Fkeys+1
    mov  [Month],edi
    jmp  upd
  .noFkey:
    cmp  ebp,2
    jne  .nomonth
    cmp  ah,177
    je	 noclose.drop
    jmp  still
  .nomonth:
    cmp  ebp,3
    je	 noy_up.year_evt
    cmp  ebp,4
    jne  still
    mov  ebx,[day_sel]
    cmp  ah,176 	; left arrow
    jb	 still
    cmp  ah,179
    ja	 still
    shr  eax,8
    sub  eax,176
    movsx ecx,byte[day_bounds+eax*2]
    movzx eax,byte[day_bounds+eax*2+1]
    add  ecx,ebx
    test eax,eax
    jz	 .chk0
    cmp  ecx,eax
    ja	 still
  .ok:
    mov  [day_sel],ecx
    call draw_days
    jmp  still		; вернуться к началу цикла
  .chk0:
    cmp  ecx,eax
    jle  still
    jmp  .ok

day_bounds db -1,0,7,0,-7,0,1,0 ; left,down,up,right

  button:
    mcall 17 	; 17 - получить идентификатор нажатой кнопки
    movzx ebx,ah
    cmp  ah,200
    jbe  nodayselect
    sub  ah,200
    mov  byte[day_sel],ah
    cmp  ebp,5
    jne  .redraw
    call draw_days
    jmp  still
  .redraw:
    mov  [focus],4
    jmp  red
  nodayselect:
    cmp  ah,100
    jb	 no_list
    sub  ah,100
    mov  byte[Month],ah
    mov  [focus],2
    jmp  upd
  no_list:
    cmp  ah,1		; идентификатор == 1 ?
    jne  noclose	; если нет - иди вперёд на noclose
  close:
    or	 eax,-1 	; выход из программы
    mcall		; вызов системы

  noclose:

    cmp  ah,72
    je	 plus_he

    cmp  ah,73
    je	 plus_hd

    cmp  ah,74
    je	 minus_he

    cmp  ah,75
    je	 minus_hd

    cmp  ah,76
    je	 plus_me

    cmp  ah,77
    je	 plus_md

    cmp  ah,78
    je	 minus_me

    cmp  ah,79
    je	 minus_md

    cmp  ah,80
    je	 reset

    cmp  ah,81
    je	 set_date

    cmp  ah,2		; drop down list
    jne  no_dropdn
  .drop:
    mov  [focus],2
    cmp  [dropped],al	; ==0
    jne  red
    call draw_window
    mov  edx,1 shl 31+231
    mov  ecx,31
    mov  eax,8
  .bremove:
    mcall
    dec  edx
    loop .bremove
    call draw_dropdown
    jmp  still
  no_dropdn:
    cmp  ah,3		; year -1
    jne  noy_dn
  year_dec:
    dec  [Year]
    mov  [focus],3
    jmp  upd
  noy_dn:
    cmp  ah,4		; year+1
    jne  noy_up
  year_inc:
    inc  [Year]
    mov  [focus],3
    jmp  upd
  noy_up:
    cmp  ah,5
    jne  noy_click
    mov  [focus],3
    call draw_window
  .still:
    mcall 10
    cmp  eax,2
    jne  still.evt
    mcall 2
  .year_evt:
    cmp  ah,176
    je	 year_dec
    cmp  ah,179
    je	 year_inc
	
    mov  ebx,10
    cmp  ah,9
    je	 key.tab
    cmp  ah,8		; backspace
    jne  .nobsp
    mov  eax,[Year]
    xor  edx,edx
    div  ebx
  .ch_year:
    mov  [Year],eax
    call draw_year
    jmp  .still
  .nobsp:
    cmp  ah,13		; enter
    je	 upd
    cmp  ah,182
    jne  .noclear	; del
    xor  eax,eax
    jmp  .ch_year
  .noclear:
    cmp  ah,48
    jb	 .still
    cmp  ah,57
    ja	 .still
    cmp  [Year],1000
    jae  .still
    shr  eax,8
    lea  ecx,[eax-48]
    mov  eax,[Year]
    imul eax,ebx
    add  eax,ecx
    jmp  .ch_year
  noy_click:
    cmp  ah,10
    jne  start
    xor  [new_style],1
    jmp  upd


reset:
    mcall 3
    mov  ecx,eax
    shl  ecx,16
    shr  ecx,16
    mcall 22,0x00000000 
    jmp  still

plus_hd:
    mcall 3
    mov  ecx,eax
    add  ecx,1
    mcall 22,0x00000000 
    jmp  still

plus_he:
    mcall 3
    mov  ecx,eax
    add  ecx,16
    mcall 22,0x00000000 
    jmp  still

minus_hd:
    mcall 3
    mov  ecx,eax
    sub  ecx,1
	mcall 22,0x00000000
    jmp  still

minus_he:
    mcall 3
    mov  ecx,eax
    sub  ecx,16
	mcall 22,0x00000000 
    jmp  still

plus_md:
    mcall 3
    mov  ecx,eax
    add  ecx,256
	mcall 22,0x00000000 
    jmp  still

plus_me:
    mcall 3
    mov  ecx,eax
    add  ecx,4096
    mcall 22,0x00000000 
    jmp  still

minus_md:
    mcall 3
    mov  ecx,eax
    sub  ecx,256
    mcall 22,0x00000000
    jmp  still

minus_me:
    mcall 3
    mov  ecx,eax
    sub  ecx,4096
    mcall 22,0x00000000
    jmp  still

set_date:
    mov  eax,0x00000000
    mov ebx,[day_sel]
    call additem
    shl  eax,8
    mov  ebx,[Month]
    add  ebx,1
    call additem
    shl  eax,8
    mov  ebx,[Year]
    call additem
    mov  ecx,eax
    mcall 22,1
    jmp  still

additem:
    add  eax,1
    daa
    sub  ebx,1
    cmp  ebx,0
    jne  additem
    ret


;   *********************************************
;   *******  ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА *******
;   *********************************************

draw_clock:

    mcall 3
    mov  ecx,eax
    mcall 47,0x00020100, ,205*65536+280,0x50000000,COL_WINDOW_BG

    shr  ecx,8
    add  edx,20*65536
    mcall

    shr  ecx,8
    add  edx,20*65536
    mcall
    ret

draw_window:

    mcall 12,1
    mcall 0,WIN_X,WIN_Y-15,COL_WINDOW_BG, ,title ; define window
	GetSkinHeight
	mov ecx, eax
	shl ecx, 16
	add ecx, 43
	mcall 13,B_WBAR_X, ,COL_TOOLBAR_BG ; draw toolbar background
    call draw_week

    mcall 8,205*65536+7,290*65536+10,72,COL_TIME_BUTTONS

    mov  ebx,212*65536+7
    inc  edx
    mcall

    mov  ebx,205*65536+7
    mov  ecx,300*65536+10
    inc  edx
    mcall

    mov  ebx,212*65536+7
    inc  edx
    mcall

    mov  ebx,225*65536+7
    mov  ecx,290*65536+10
    inc  edx
    mcall

    mov  ebx,232*65536+7
    inc  edx
    mcall

    mov  ebx,225*65536+7
    mov  ecx,300*65536+10
    inc  edx
    mcall

    mov  ebx,232*65536+7
    inc  edx
    mcall

    mov  ebx,244*65536+14
    mov  ecx,290*65536+20
    inc  edx
    mcall

    mov  ebx,14*65536+110
    mov  ecx,285*65536+22
    mov  esi,COL_DATE_BUTTONS
    inc  edx
    mcall

    mov  esi,COL_MONTH_YEAR_B   ; new style
    mov  edx,10
    or	 edx,1 shl 29+1 shl 30
    mov  ebx,B_NS_X
    mov  ecx,B_NS_Y
    mcall
    add  edx,1-1 shl 29
    mov  ebx,B_TODAY_X+8*(today_end-today_msg)
    mov  ecx,B_TODAY_Y
    mcall
    mov  ecx,B_Y
    mov  ebx,B_MONTH_X
    mov  edx,2
    mcall
    mov  ebx,B_SPIN_X ; <
    inc  edx
    mcall
    add  ebx,61 shl 16 ; >
    inc  edx
    mcall
    call draw_days

	; функция 4 : написать в окне текст
    mcall 4,162*65536+280,0x800000ff,sys_text
    mcall  ,180*65536+302,0x800000ff,minus
    mcall  ,180*65536+292,0x80ff0000,plus
    mcall  , 24*65536+292,0x00000000,set_date_t,15 ;set date text

    mov  ecx,0x10ddeeff 	   ; шрифт 1 и цвет ( 0xF0RRGGBB )

    mov  edx,n_style
    mov  esi,ns_end-n_style
    mov  ebx,B_NS
    cmp  [new_style],1
    je	 .high
    mov  ecx,0xa0a0a0
    jmp  .int
  .high:
    mov  ecx,COL_NEW_STYLE_T
  .int:
    mcall

    mov  ecx,COL_GO_TODAY_T
    mov  edx,today_msg
    mov  ebx,B_TODAY
    mov  esi,today_end-today_msg
    mcall

    mov  ebx,B_SPIN
    mov  edx,spinner
    mov  esi,12
	mov  ecx,COL_DROPDOWN_T
    mcall

    mov  edx,[Month]
    movzx  esi,byte[month_name]
    imul edx,esi
    add  edx,month_name+1
    mov  ebx,B_MONTH
    ShowFocus 2
    mcall

    call draw_year
    mov  [dropped],0
    mcall 12,2
    ret

draw_year:
    mcall 8,B_YEAR_X,B_Y,5,COL_MONTH_YEAR_B
    ShowFocus 3,esi
    mcall 47,0x40001,Year,B_YEAR
    ret

draw_dropdown:
    mov  [dropped],1
    push [Month]
    pop  [focus]
    add  [focus],100
    mov  ecx,12
    mov  edx,100
    push dword month_name+1
    push dword B_DROP
    push dword B_Y+16 shl 16
  .ddd_loop:
    mov  edi,edx
    push ecx
    mov  esi,COL_DROPDOWN_BG
    mcall 8,B_MONTH_X,[esp+4]
    shr  eax,1
    mov  ebx,[esp+8]
    xchg edx,[esp+12]
    movzx esi,byte[month_name]
    ShowFocus edi
    mcall
    add  edx,esi
    xchg edx,[esp+12]
    add  dword[esp+8],16
    add  dword[esp+4],16 shl 16
    inc  edx
    pop  ecx
    loop .ddd_loop
    add  esp,12
    ret

draw_week:
    mov  eax,13
    mov  ebx,B_WBAR_X
    mov  ecx,B_WBAR_Y
    mov  edx,COL_WEEKDAY_BG
    mcall
    movzx esi,byte[week_days]
    movzx edi,byte[week_days+1]
    mov  ebx,B_WEEK
    mov  ecx,7
    mov  edx,week_days+2
    mov  eax,4
  .week:
    push ecx
    cmp  ecx,edi
    je	 .holiday
    mov  ecx,0x10000000
    jmp  .noholiday
  .holiday:
    mov  ecx,0x10cc1010
  .noholiday:
    mcall
    add  edx,esi
    add  ebx,B_WX_SHIFT
    pop  ecx
    loop .week
    ret

draw_days:
    call count_days
    cmp  ecx,[day_sel]
    jae  .ok
    mov  [day_sel],ecx
  .ok:
    mov  [number],0
    mov  eax,47
    mov  edx,B_DAYS_Y
    mov  ebx,0x10001
    mov  edi,[firstday]
  .dayloop:
    push ecx
    movzx edx,dx
    mov  esi,edi
    shl  esi,21
    lea  edx,[edx+esi+34 shl 16]
    mov  ecx,edi
    add  cl,[week_days+1]
    cmp  ecx,7
    je	 .holiday
    mov  esi,0x10000000 ; COL_DATE_TEXT
    jmp  .noholiday
  .holiday:
    mov  esi,0x10cc1010
  .noholiday:
    mov  ecx,number
    inc  dword[ecx]
    pusha
    mov  ebx,edx
    mov  bx,31           ; width
    sub  ebx,8 shl 16
    shrd ecx,edx,16
    mov  cx,29           ; height
    sub  ecx,12 shl 16
    mov  edx,[number]
    cmp  edx,[day_sel]
    je	 .draw_sel
    mov  esi,COL_DATE_BUTTONS
    jmp  .draw_but
  .draw_sel:
    mov  esi,COL_DATE_INACTIV
    cmp  [focus],4
    jne  .draw_but
    mov  esi,COL_DATE_ACTIVE
  .draw_but:
    add  edx,200+1 shl 29
    mcall 8
	mov    eax, [number]
    xor    edx, edx
    mov    ecx, 10
    div    ecx
    mov    [remainder], edx
    mov    [quotient],  eax
	popa
	
	;first number
	mov ecx,quotient
    mcall 
	add edx,1 shl 16
	mcall
	sub edx,1 shl 16
	
	;second number
	mov ecx,remainder
	add edx,9 shl 16
    mcall 
	add edx,1 shl 16
	mcall
	sub edx,10 shl 16
	
	
    pop  ecx
    inc  edi
    cmp  edi,7
    jne  .nowrap
    xor  edi,edi
    add  dx,B_DAYS_SHIFT
  .nowrap:
    loop .eloop
    jmp  .ex
  .eloop:
    jmp  .dayloop
  .ex:
    ret

count_days:    ; ecx -days in month
    call is_leap_year
    mov  ecx,[Month]
    mov  eax,1
    movzx ecx,byte[day_count+ecx]
    add  ecx,28
    cmp  eax,[leap_year]
    jne  .noleap
    cmp  eax,[Month]
    jne  .noleap
    inc  ecx
  .noleap:
    mov  [day_bounds+3],cl
    mov  [day_bounds+7],cl
    ret

is_leap_year:
    mov  [leap_year],0
    mov  eax,[Year]
    mov  bl,100
    div  bl	     ; ah=Year mod 100, al=Year%100
    test ah,ah
    jz	.century
    shr  ax,8	     ; ax - last 2 digits
  .century:
    test al,11b
    jnz  .noleap
    inc  [leap_year]
  .noleap:
    ret

calculate:
    mov  ebx,[Year]
    mov  eax,[Month]
    sub  eax,2
    jge  .nojf
    dec  ebx
    add  eax,12
  .nojf:
    add  eax,4
    xor  edx,edx
    mov  ecx,153
    imul cx
    mov  ecx,5
    div  cx
    inc  eax
    mov  ecx,365
    imul ecx,ebx
    add  eax,ecx
    mov  ecx,ebx
    shr  ecx,2
    add  eax,ecx
    dec  eax
    cmp  [new_style],0
    je	 .nonew
    add  eax,2
    xchg eax,ebx
    mov  ecx,100
    xor  edx,edx
    div  cx
    sub  ebx,eax
    shr  eax,2
    add  ebx,eax
  .nonew:
    add  ebx,5
    mov  eax,ebx
    xor  edx,edx
    movzx ebx,byte[week_days+1]
    sub  eax,ebx
    inc  eax
    mov  ebx,7
    div  bx
    mov  [firstday],edx
    ret

I_END:

firstday  dd ?
Year dd   ?
Month dd  ?
day_sel   dd ?

datestr   dd  ?
leap_year dd ?
number	  dd ?
