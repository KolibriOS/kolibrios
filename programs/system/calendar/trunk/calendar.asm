; CALENDAR FOR MENUET v1.0
; Written in pure assembler by Ivushkin Andrey aka Willow
;
;
; Created:      November 1, 2004
; Last changed: January 13, 2005
;
; COMPILE WITH FASM

WIN_X equ (150 shl 16+270)
WIN_Y equ (100 shl 16+300)

LINE1	  equ 27 shl 16+16
B_MONTH_X equ 10 shl 16+158
B_Y	  equ LINE1
B_MONTH   equ 63 shl 16+32

B_WBAR_X  equ 10 shl 16+250
B_WBAR_Y  equ 64 shl 16+20
B_WEEK	  equ 30 shl 16+70
B_WX_SHIFT equ 32 shl 16

B_DBAR_X  equ B_WBAR_X
B_DBAR_Y  equ 85 shl 16+190

B_DROP	  equ B_MONTH+16
B_DAYS_Y  equ 100
B_DAYS_SHIFT equ 30

B_YEAR_X  equ 173 shl 16+58
B_YEAR	  equ 188 shl 16+32

B_TODAY_X equ 25 shl 16
B_TODAY_Y equ 48 shl 16+10
B_TODAY   equ 30 shl 16+50

B_SPIN_WIDTH equ 13
B_SPIN_X  equ 234 shl 16+B_SPIN_WIDTH
B_SPIN	  equ 238 shl 16+32

B_DATE_X  equ 26 shl 16+60
B_DATE_Y  equ 275 shl 16+16
B_DATE_BSHIFT equ 80 shl 16
B_DATE	  equ 32 shl 16+280
B_DATE_SHIFT equ 80 shl 16

B_NS_X	  equ 185 shl 16+75
B_NS_Y	  equ 48 shl 16+10
B_NS	  equ 190 shl 16+50

FOCUSABLE equ 5
SKIP	  equ 1

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
include 'macros.inc' ; уменьшает размер программы
;include 'debug.inc'


macro  ShowFocus field,reg
{
   local  .nofocus, .exit
     cmp  [focus],field
     jne  .nofocus
   if reg eq
     mov  ecx,0x10e7c750;0x10ddeeff
   else
     mov  reg,0x10e7c750;0x10ddeeff
   end if
     jmp  .exit
   .nofocus:
   if reg eq
     mov  ecx,0x10000000
   else
     mov  reg,0x10000000
   end if
   .exit:
}

month_name:
if lang eq ru
     db   8
     db   'Январь  '
     db   'Февраль '
     db   'Март    '
     db   'Апрель  '
     db   'Май     '
     db   'Июнь    '
     db   'Июль    '
     db   'Август  '
     db   'Сентябрь'
     db   'Октябрь '
     db   'Ноябрь  '
     db   'Декабрь '
else if lang eq de
     db   9
     db   'Januar   '
     db   'Februar  '
     db   'M┴rz     '
     db   'April    '
     db   'Mai      '
     db   'Juni     '
     db   'Juli     '
     db   'August   '
     db   'September'
     db   'Oktober  '
     db   'November '
     db   'Dezember '
else if lang eq fr
     db   9
     db   'Janvier  '
     db   'Fevrier  '
     db   'Mars     '
     db   'Avril    '
     db   'Mai      '
     db   'Juin     '
     db   'Juliet   '
     db   'Aout     '
     db   'Septembre'
     db   'Octobre  '
     db   'Novembre '
     db   'Decembre '
else if lang eq fi
     db   9
     db   'Tammikuu '
     db   'Helmikuu '
     db   'Maaliskuu'
     db   'Huhtikuu '
     db   'Toukokuu '
     db   'Kes┴kuu  '
     db   'Hein┴kuu '
     db   'Elokuu   '
     db   'Syyskuu  '
     db   'Lokakuu  '
     db   'Marraskuu'
     db   'Joulukuu '
else if lang eq et
     db   9
     db   'Jaanuar  '
     db   'Veebruar '
     db   'Mфrts    '
     db   'Aprill   '
     db   'Mai      '
     db   'Juuni    '
     db   'Juuli    '
     db   'August   '
     db   'September'
     db   'Oktoober '
     db   'November '
     db   'Detsember'
else
     db   9
     db   'January  '
     db   'February '
     db   'March    '
     db   'April    '
     db   'May      '
     db   'June     '
     db   'July     '
     db   'August   '
     db   'September'
     db   'October  '
     db   'November '
     db   'December '
end if
spinner db '< >'
week_days:
if lang eq ru
     db   2
     db   1
     db   'Пн'
     db   'Вт'
     db   'Ср'
     db   'Чт'
     db   'Пт'
     db   'Сб'
     db   'Вс'
else if lang eq de
     db   2
     db   7
     db   'So'
     db   'Mo'
     db   'Di'
     db   'Mi'
     db   'Do'
     db   'Fr'
     db   'Sa'
else if lang eq fr
     db   3
     db   7
     db   'Dim'
     db   'Lun'
     db   'Mar'
     db   'Mer'
     db   'Jeu'
     db   'Ven'
     db   'Sam'
else if lang eq fi
     db   2
     db   7
     db   'Su'
     db   'Ma'
     db   'Ti'
     db   'Ke'
     db   'To'
     db   'Pe'
     db   'La'
else if lang eq et
     db   3
     db   7
     db   'Esm'
     db   'Tei'
     db   'Kol'
     db   'Nel'
     db   'Ree'
     db   'Lau'
     db   'P№h'
else
     db   3
     db   7
     db   'Sun'
     db   'Mon'
     db   'Tue'
     db   'Wen'
     db   'Thi'
     db   'Fri'
     db   'Sat'
end if

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

    call draw_window	; вызываем процедуру отрисовки окна

still:			; ГЛАВНЫЙ ЦИКЛ ПРОГРАММЫ

    mov  eax,10 	; функция 10 - ждать события
    int  0x40		; вызываем систему
  .evt:
    mov  ebp,[focus]
    cmp  eax,1		; перерисовать окно ?
    je	 red		; если да - на метку red
    cmp  eax,2		; нажата клавиша ?
    je	 key		; если да - на key
    cmp  eax,3		; нажата кнопка ?
    je	 button 	; если да - на button

    jmp  still		; если другое событие - в начало цикла

  key:			; нажата клавиша на клавиатуре
    mov  eax,2		; функция 2 - считать код символа
    int  0x40		; вызов системы
    cmp  ah,9
    jne  no_tab
  .tab:
    cmp  ebp,FOCUSABLE
    je	 foc_cycle
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
    cmp  ebp,4
    jne  no_spinner
    cmp  ah,176
    je	 year_dec
    cmp  ah,179
    je	 year_inc
  no_spinner:
    cmp  ebp,2
    jne  .nomonth
    cmp  ah,177
    je	 noclose.drop
    jmp  still
  .nomonth:
    cmp  ebp,3
    je	 noy_up.year_evt
    cmp  ebp,5
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

  button:		; нажата кнопка в окне программы
    mov  eax,17 	; 17 - получить идентификатор нажатой кнопки
    int  0x40		; вызов системы
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
    mov  [focus],5
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
    int  0x40		; вызов системы

  noclose:
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
    int  0x40
    dec  edx
    loop .bremove
    call draw_dropdown
    jmp  still
  no_dropdn:
    cmp  ah,3		; year -1
    jne  noy_dn
  year_dec:
    dec  [Year]
    mov  [focus],4
    jmp  upd
  noy_dn:
    cmp  ah,4		; year+1
    jne  noy_up
  year_inc:
    inc  [Year]
    mov  [focus],4
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


;   *********************************************
;   *******  ОПРЕДЕЛЕНИЕ И ОТРИСОВКА ОКНА *******
;   *********************************************

draw_window:

    mov  eax,12 		   ; функция 12: сообщить ОС об отрисовке окна
    mov  ebx,1			   ; 1 - начинаем рисовать
    int  0x40
				   ; СОЗДАЁМ ОКНО
    xor  eax,eax		   ; функция 0 : определить и отрисовать окно
    mov  ebx,WIN_X
  if SKIP eq 0
    mov  ecx,WIN_Y
  else
    mov  ecx,WIN_Y-15
  end if
    mov  edx,0x13aabbcc 	   ; цвет рабочей области  RRGGBB,8->color gl
    mov  edi,header              ; заголовок
    int  0x40
    call draw_week

    mov  eax,8
    mov  esi,0x05080d0
  if SKIP eq 0
    mov  ebx,B_DATE_X
    mov  ecx,B_DATE_Y
    mov  edx,eax
    int  0x40
    inc  edx
    add  ebx,B_DATE_BSHIFT
    int  0x40
    inc  edx
  else
    mov  edx,10
  end if
    or	 edx,1 shl 29+1 shl 30
    mov  ebx,B_NS_X
    mov  ecx,B_NS_Y
    int  0x40
    add  edx,1-1 shl 29
    mov  ebx,B_TODAY_X+8*(today_end-today_msg)
    mov  ecx,B_TODAY_Y
    int  0x40
    mov  ecx,B_Y
    mov  ebx,B_MONTH_X
    mov  edx,2
    int  0x40
    mov  ebx,B_SPIN_X
    inc  edx
    int  0x40
    add  ebx,B_SPIN_WIDTH shl 16
    inc  edx
    int  0x40
    call draw_days

    mov  eax,4			   ; функция 4 : написать в окне текст
    mov  ecx,0x10ddeeff 	   ; шрифт 1 и цвет ( 0xF0RRGGBB )

 if SKIP eq 0
    mov  ebx,B_DATE
    mov  edx,datebut
    mov  esi,9
    btc  ecx,28
    int  0x40
    add  ebx,B_DATE_SHIFT
    add  edx,esi
    int  0x40
 end if
    mov  edx,n_style
    mov  esi,ns_end-n_style
    mov  ebx,B_NS
    cmp  [new_style],1
    je	 .high
    mov  ecx,0xa0a0a0
    jmp  .int
  .high:
    mov  ecx,0xac0000;d048c8
  .int:
    int  0x40

    mov  ecx,0xd048c8
    mov  edx,today_msg
    mov  ebx,B_TODAY
    mov  esi,today_end-today_msg
    int  0x40

    mov  ebx,B_SPIN
    mov  edx,spinner
    mov  esi,3
    ShowFocus 4
    int  0x40

    mov  edx,[Month]
    movzx  esi,byte[month_name]
    imul edx,esi
    add  edx,month_name+1
    mov  ebx,B_MONTH
    ShowFocus 2
    int  0x40

    call draw_year
    mov  [dropped],0
    mov  eax,12 		   ; функция 12: сообщить ОС об отрисовке окна
    mov  ebx,2			   ; 2, закончили рисовать
    int  0x40
    ret 			   ; выходим из процедуры

draw_year:
    mcall 8,B_YEAR_X,B_Y,5,0x05080d0
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
    mov  ebx,B_MONTH_X
    mov  ecx,[esp+4]
    mov  esi,0x6f9fef
    mov  eax,8
    int  0x40
    shr  eax,1
    mov  ebx,[esp+8]
    xchg edx,[esp+12]
    movzx esi,byte[month_name]
    ShowFocus edi
    int  0x40
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
    mov  edx,0x90a0b0
    int  0x40
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
    int  0x40
    add  edx,esi
    add  ebx,B_WX_SHIFT
    pop  ecx
    loop .week
    ret

draw_days:
    mov  eax,13
    mov  ebx,B_DBAR_X
    mov  ecx,B_DBAR_Y
    mov  edx,0xe0e0e0
    int  0x40
    call count_days
    cmp  ecx,[day_sel]
    jae  .ok
    mov  [day_sel],ecx
  .ok:
    mov  [number],0
    mov  eax,47
    mov  edx,B_DAYS_Y
    mov  ebx,0x20001
    mov  edi,[firstday]
  .dayloop:
    push ecx
    movzx edx,dx
    mov  esi,edi
    shl  esi,21
    lea  edx,[edx+esi+30 shl 16]
    mov  ecx,edi
    add  cl,[week_days+1]
    cmp  ecx,7
    je	 .holiday
    mov  esi,0x10000000
    jmp  .noholiday
  .holiday:
    mov  esi,0x10cc1010
  .noholiday:
    mov  ecx,number
    inc  dword[ecx]
    pusha
    mov  ebx,edx
    mov  bx,20
    sub  ebx,3 shl 16
    shrd ecx,edx,16
    mov  cx,20
    sub  ecx,7 shl 16
    mov  edx,[number]
    cmp  edx,[day_sel]
    je	 .draw_sel
    mov  esi,0xe0e0e0
    jmp  .draw_but
  .draw_sel:
    mov  esi,0x5080d0
    cmp  [focus],5
    jne  .draw_but
    mov  esi,0xef7840;0xe26830
  .draw_but:
    add  edx,200+1 shl 29
    mov  eax,8
    int  0x40
    popa
    int  0x40
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

; Здесь находятся данные программы:

; интерфейс программы двуязычный - задайте язык в macros.inc
day_count db 3,0,3,2,3,2,3,3,2,3,2,3
Fkeys	  db 210,211,212,213,214,215,216,217,208,209,228,159

header:		 ; строка заголовка
if lang eq ru
     db   'КАЛЕНДАРЬ',0
else if lang eq ge
     db   'KALENDER',0
else if lang eq fr
     db   'CALENDRIER',0
else if lang eq et
     db   'KALENDER',0
else
     db   'CALENDAR',0
end if

if SKIP eq 0
datebut:
if lang eq ru
     db   '1-я дата '
     db   '2-я дата '
else if lang eq fr
     db   '1ere date'
     db   '2eme date'
else if lang eq ge
     db   ' Datum 1 '
     db   ' Datum 2 '
else if lang eq et
     db   'Kuupфev 1'
     db   'Kuupфev 2'
else
     db   '1st date '
     db   '2nd date '
end if
end if
n_style:
if lang eq ru
     db   'Новый стиль'
else if lang eq de
     db   'Neuer Stil'
else if lang eq fr
     db   'Nouveau'
else if lang eq et
     db   'Uus stiil'
else
     db   'New style'
end if
ns_end:
today_msg:
if lang eq ru
     db   'Сегодня'
else if lang eq ge
     db   'Heute'
else if lang eq fr
     db   "Aujourd'hui"
else if lang eq et
     db   'Tфna'
else
     db   'Today'
end if
today_end:
focus dd  3
new_style dd 1
dropped db 0

I_END:	; конец программы
firstday  dd ?
Year dd   ?
Month dd  ?
day_sel   dd ?
all_days  dd ?

datestr   dd  ?
leap_year dd ?
number	  dd ?
year_input dd ?
