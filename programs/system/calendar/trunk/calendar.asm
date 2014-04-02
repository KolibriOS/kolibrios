; Calendar for KolibriOS
;
; v1.5 - time redesign by Heavyiron
; v1.2 - v1.55 - new desighn and functionality by Leency
; v1.1 - add change time support by DedOK 
; v1.0 - written in pure assembler by Ivushkin Andrey aka Willow
; also - diamond, spraid, fedesco
;
; Created: November 1, 2004


use32

  org	 0x0

  db	 'MENUET01'
  dd	 0x01
  dd	 START
  dd	 I_END
  dd	 0x1000
  dd	 0x1000
  dd	 0x0
  dd	 0x0
include '..\..\..\macros.inc'
include 'lang.inc'
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

macro DrawRect color1,color2,color3,color4 ; pizdec... but optimized well
{
	; top border-outer
	push ebx
	push ecx
	mov eax,13
	mov bx,DATE_BUTTON_WIDTH
	mov edx,color1
	mov cx,1
	mcall
	; left border-outer
	mov bx,1
	mov cx,DATE_BUTTON_HEIGHT
	mcall
	; top border-inner
	mov edx,color2
	add ebx,1 shl 16
	add ecx,1 shl 16
	mov bx,DATE_BUTTON_WIDTH-1
	mov cx,1
	; left border-inner
	mcall
	mov bx,1
	mov cx,DATE_BUTTON_HEIGHT-2
	mcall
	; inner
	mov edx,color3
	add ebx,1 shl 16
	add ecx,1 shl 16
	mov bx,DATE_BUTTON_WIDTH-4
	mov cx,DATE_BUTTON_HEIGHT-4
	mcall
	; bottom border-inner
	mov edx,color4
	add ebx,DATE_BUTTON_WIDTH shl 16
	sub ebx,4 shl 16
	mov bx,1
	mov cx,DATE_BUTTON_HEIGHT-3
	mcall
	; rgiht border-outer
	mov edx,color2
	add ebx,1 shl 16
	sub ecx,1 shl 16
	add cx,1
	mcall
	; bottom border-outer
	mov edx,color2
	pop ecx
	pop ebx
	add ecx,DATE_BUTTON_HEIGHT shl 16
	sub ecx,1 shl 16
	add ebx,1 shl 16
	mov cx,1
	mcall
	; left border-outer
	mov edx,color4
	add ebx,1 shl 16
	sub ecx,1 shl 16
	sub bx,2
	mcall 
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

START:
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
	mov eax,[Year]
	mov [curYear], eax
	mov eax,[Month]
	mov [curMonth], eax
	mov eax,[day_sel]
	mov [curDay], eax
  .no2000:
    jmp  upd
red:

    call define_window

still:

    mcall 23,50     ; wait here for event
  .evt:
    mov  ebp,[focus]
    cmp  eax,1
    je	 red
    cmp  eax,2
    je	 key
    cmp  eax,3
    je	 button

    call draw_clock

    jmp  still

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
    jmp  still
  .chk0:
    cmp  ecx,eax
    jle  still
    jmp  .ok

day_bounds db -1,0,7,0,-7,0,1,0 ; left,down,up,right

  button:
    mcall 17
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
    cmp  ah,1
    jne  noclose
  close:
    mcall -1		; clore programm

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
    jne  START
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
    jmp  START

additem:
    add  eax,1
    daa
    sub  ebx,1
    cmp  ebx,0
    jne  additem
    ret


;   *********************************************
;   *******          DRAW WINDOW          *******
;   *********************************************

draw_clock:

    mcall 3
    mov  ecx,eax
    mcall 47,0x00020100, ,195*65536+301,0x50CCCCCC,COL_TOOLBAR_BG

    shr  ecx,8
    add  edx,22*65536
    mcall

    shr  ecx,8
    add  edx,22*65536
    mcall
    ret

define_window:

    mcall 12,1
	mcall 48,5 ;get screen size
	mov ecx, ebx
	sub ecx, WIN_H
	shl ecx, 16
	add ecx, WIN_H
	mov ebx, eax
	sub eax, WIN_W
	shl ebx, 16
	add ebx, WIN_W	
    mcall 0,,,COL_WINDOW_BG, ,title ; define window
	mcall 12,2
	GetSkinHeight
	mov ecx, eax
	shl ecx, 16
	add ecx, 43
	mcall 13,B_WBAR_X, ,COL_TOOLBAR_BG ; draw toolbar background
	mcall 13,B_WBAR_X,BT_WBAR_Y,COL_TOOLBAR_BG ; draw toolbar background
	mcall 13,0*65536+B_WBAR_X,84*65536+199,0xE7E7E7
	
draw_window:

    call draw_week
    mcall 8,193*65536+8,287*65536+10,72,COL_TOOLBAR_BG
    mov  ebx,202*65536+8
    inc  edx ;73
    mcall
    mov  ebx,193*65536+8
    mov  ecx,311*65536+10
    inc  edx ;74
    mcall
    mov  ebx,202*65536+8
    inc  edx ;75
    mcall
    mov  ebx,215*65536+8
    mov  ecx,287*65536+10
    inc  edx ;76
    mcall
    mov  ebx,224*65536+8
    inc  edx ;77
    mcall
    mov  ebx,215*65536+8
    mov  ecx,311*65536+10
    inc  edx ;78
    mcall
    mov  ebx,224*65536+8
    inc  edx ;79
    mcall
    ;mov  ebx,237*65536+21
    ;mov  ecx,281*65536+35
    ;inc  edx ;80
    ;or   edx,0x40CCFF44
    ;mcall
    mov  ebx,25*65536+110
    mov  ecx,293*65536+22
    mov  esi,COL_TOOLBAR_BG
    mov  edx,81
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

	; draw text in window
    mcall 4,157*65536+301,0x80CCCCCC,sys_text
    mcall  ,211*65536+301,,separator
    mcall  ,233*65536+301
    mcall  ,185*65536+289, ,plus
    mcall  ,185*65536+313, ,minus
    mcall  , 35*65536+300,0x00CCCCCC,set_date_t,15 ;set date text


    mov  ecx,0x10ddeeff
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
    call draw_clock
    mov  [dropped],0
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
    mov  esi,0x10313138 ; COL_DATE_TEXT
    jmp  .noholiday
  .holiday:
    mov  esi,0x10cc1010
  .noholiday:
    mov  ecx,number
    inc  dword[ecx]
    pusha
    mov  ebx,edx
    mov  bx,DATE_BUTTON_WIDTH-1
    sub  ebx,8 shl 16
    shrd ecx,edx,16
    mov  cx,DATE_BUTTON_HEIGHT-1
    sub  ecx,12 shl 16
    mov  edx,[number]
    cmp  edx,[day_sel]
    je	 .draw_sel
    mov  esi,COL_DATE_BUTTONS
    jmp  .draw_but
.draw_sel:                                  ;draw selected button
	add  edx,1 shl 30
	add  edx,200+1 shl 29
	mcall 8
    cmp  [focus],4
    jne  .not_active
	DrawRect COL_DATE_ACTIVE_1,COL_DATE_ACTIVE_2,COL_DATE_ACTIVE_3,COL_DATE_ACTIVE_4
	jmp .out
.not_active:
	DrawRect COL_DATE_INACTIVE_1,COL_DATE_INACTIVE_2,COL_DATE_INACTIVE_3,COL_DATE_INACTIVE_4
	jmp .out
.draw_but:                                   ;draw non selected button
    add  edx,200+1 shl 29
    mcall 8
	mov eax,[Year]
	cmp [curYear],eax
	jne .out
	mov eax,[Month]
	cmp [curMonth],eax
	jne .out
	mov eax,[number]
	cmp [curDay],eax
	jne .out
	mov edx,0xff0000
	mov bx,DATE_BUTTON_WIDTH-2
	mov cx,2
	add ebx,1 shl 16
	add ecx,27 shl 16
	mcall 13
.out:
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
