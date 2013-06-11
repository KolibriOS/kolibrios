;
;   Vertical Refresh Rate programm
;
;   Author:  Trans <<< 13 >>>
;   Date:    February-May 2003 (09.05.2003)
;   Version: 2.0
;   Last Modification: 30.07.2003
;   Compile with FASM for Menuet
;
use32

        org    0x0

        db     'MENUET01'   ; 8 byte id
        dd     0x01         ; header version
        dd     START        ; start of code
        dd     I_END        ; size of image
        dd     0x5000       ; memory for app
        dd     0x4ff0       ; esp
        dd     0x0 , 0x0    ; I_Param , I_Icon

include '..\..\..\macros.inc'
include 'lang.inc'
START:    ; start of execution

    mov eax,14
    mcall
    mov ebx,eax
    shr ebx,16
    mov [oldX],ebx
    shl eax,16
    shr eax,16
    mov [oldY],eax

; Test on installed video driver
    mov eax,21
    mov ebx,13
    mov ecx,1
    mov edx,drvinfo
    mcall
    cmp eax,0FFFFFFFFh ; = -1 - error or not installed
    jne vrr_00
    call warning_info
    retn
;

vrr_00:
    call get_vert_rate
    mov eax,[currvm]
    mov [oldvm],eax
    call get_pid
red:
    call draw_window  ; at first, draw the window

still:

    mov  eax,10   ; check here for event
    mcall

    cmp  eax,1   ; redraw request ?
    je  red
    cmp  eax,2   ; key in buffer ?
    je  key
    cmp  eax,3   ; button in buffer ?
    je  button
    call get_pid
    jmp  still

  key:    ; key
    mov  al,2   ; just read it
    mcall
    cmp ah,'1'
    jne key_loc_00
    call safekey
    jmp still
key_loc_00:
    cmp ah,'2'
    jne key_loc_01
    call safekey
    jmp still
key_loc_01:
    cmp ah,'3'
    jne key_loc_02
    call safekey
    jmp still
key_loc_02:
    cmp ah,'4'
    jne key_loc_03
    call safekey
    jmp still
key_loc_03:
    cmp ah,'5'
    jne key_loc_04
    call safekey
    jmp still
key_loc_04:
    cmp ah,'-'
    jne key_loc_05
    xor eax,eax
    call inc_dec_rate
    jmp still
key_loc_05:
    cmp ah,'+'
    jne key_loc_06
    xor eax,eax
    inc eax
    call inc_dec_rate
    jmp  still
key_loc_06:
    cmp ah,'r'         ;Return in last stable mode
    jne key_loc_07
    call restore_mode
    jmp red
key_loc_07:
    cmp ah,13          ;Apply select mode
    jne still
    xor eax,eax
    mov ax,[vmselect]
    cmp al,0
    je still
    xor ebx,ebx
    mov bl,al
    dec bl
    shl bx,1      ; ebx=(ebx-1)*2
    xor edx,edx
    mov dx,[vidmode+ebx]
    mov ecx,ebx
    shl ebx,2
    add ebx,ecx   ; ebx=ebx*5
    shr ax,8
    dec ax
    shl ax,1
    add ebx,eax
    ror edx,16
    mov dx,[_m1+ebx]
    rol edx,16
    call set_my_mode
    call protect_and_return
    xor ax,ax
    mov [vmselect],ax
    jmp red

button:   ; button
    mov  al,17   ; get id
    mcall

    cmp  ah,1   ; button id=1 ?
    jne  noclose
    mov  eax,-1           ; close this program
    mcall
  noclose:
    cmp ah,2              ;'+' screen width
    jne button_loc_01
    xor eax,eax
    inc eax
    call inc_dec_rate
    jmp still
button_loc_01:
    cmp ah,3              ;'-' screen width
    jne button_loc_02
    xor eax,eax
    call inc_dec_rate
    jmp still
button_loc_02:
    cmp ah,4              ; Ok
    jne button_loc_03
    mov ah,13
    jmp key_loc_07
button_loc_03:
    cmp ah,5              ; Cancel
    jne button_loc_04
    mov [vmselect],word 00h
    jmp red
button_loc_04:
    cmp ah,6              ; Return
    jne button_loc_05
    mov ah,'r'
    jmp key_loc_06
button_loc_05:
    cmp ah,7              ; Default
    jne button_loc_06
    call restore_mode
button_loc_06:
    jmp  still


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

dw_continue:

    mov  eax,12      ; function 12:tell os about windowdraw
    mov  ebx,1      ; 1, start of draw
    mcall

       ; DRAW WINDOW
    mov  eax,0      ; function 0 : define and draw window
    mov  ebx,100*65536+400    ; [x start] *65536 + [x size]
    mov  ecx,100*65536+200    ; [y start] *65536 + [y size]
    mov  edx,0x140020C0;0x00000040 ; color of work area RRGGBB,8->color glide
    mov  edi,title
    mcall

       ; BUTTONS
    mov eax,8
    mov edx,0
    mov ebx,330*65536+20
    mov ecx,84*65536+48
    mov dl,2
    mov  esi,0x5599cc     ; button color RRGGBB
    mcall               ; Button '+'Width
    add ebx,30*65536
    mov dl,3
    mcall               ; Button '-'Width
    mov ebx,22*65536+85
    mov ecx,170*65536+15
    inc dl ;dl=4
    mcall               ; Button 'Ok'
    add ebx,90*65536
    inc dl ;dl=5
    mcall               ; Button 'Cancel'
    add ebx,90*65536
    inc dl ;dl=6
    mcall               ; Button 'Return'
    add ebx,90*65536
    inc dl ;dl=7
    mcall               ; Button 'Default'

    call draw_face

    mov  eax,12      ; function 12:tell os about windowdraw
    mov  ebx,2      ; 2, end of draw
    mcall

    ret

;------------Subfunctions-----------

restore_mode:
    push eax
    push ebx
    push edx
    mov eax,21
    mov ebx,13
    mov ecx,4
    mcall
    pop edx
    pop ecx
    pop eax
    retn



; IN: edx = RefRate*65536+No.VideoMode
set_my_mode:
    push ecx
    push ebx
    push edx
    mov eax,[currvm]
    mov [oldvm],eax
    mov [currvm],edx
    pop edx
    push edx
    mov eax,21
    mov ebx,13
    mov ecx,3
    mcall
    mcall 5,50
    mcall 15,3
    pop edx
    pop ebx
    pop ecx
    retn

; IN: eax = 0/1  -  -/+ 1Hz
inc_dec_rate:
    push ebx
    push ecx
    push edx
    mov edx,eax
    mov eax,21
    mov ebx,13
    mov ecx,5
    mcall
    pop edx
    pop ecx
    pop ebx
    retn

get_pid:
    mov eax,9
    mov ebx,buffer
    xor ecx,ecx
    dec ecx
    mcall
    mov [totp],eax
    mov eax,[ebx+30]
    mov [mypid],eax
    mov ax,[ebx+4]
    mov [mypno],ax
    retn

get_vert_rate:
    xor eax,eax
    mov ebx,eax
    mov ecx,eax
    mov al,21
    mov bl,13
    mov cl,2
    mcall
    mov [initrr],ebx
    mov [refrate],ebx
    ror ecx,16
    mov cx,bx
    rol ecx,16
    mov [currvm],ecx
    retn

get_initial_videomode:
    retn


draw_table:
    mov eax,13
    mov ebx,9*65536+303
    mov ecx,59*65536+87
    xor edx,edx
    mcall
    mov ebx,10*65536+300
    mov ecx,60*65536+24
    mov edx,00FF00FFh
    mcall
    mov ebx,10*65536+36
    mov ecx,72*65536+72
    mov edx,0000FFFFh
    mcall
    mov eax,38
    mov edx,00FFFFFFh
    mov ebx,10*65536+310
    mov edi,60*65536+60
    mov esi,12*65536+12
    xor ecx,ecx
    mov cl,8
dt_loc_hor_line:
    push ecx
    mov ecx,edi
    mcall
    add edi,esi
    pop ecx
    loop dt_loc_hor_line
    mov ebx,10*65536+10
    mov edi,60*65536+144
    mov esi,66*65536+66
    mov ecx,edi
    mcall
    add ebx,36*65536+36
    xor ecx,ecx
    mov cl,5
dt_loc_vert_line:
    push ecx
    mov ecx,edi
    mcall
    add ebx,esi
    pop ecx
    loop dt_loc_vert_line
    mov eax,4
    mov ebx,52*65536+75
    mov ecx,000000FFh
    mov edx,_m1280x1024
    mov esi,9
    mcall
    add edx,9
    add ebx,66*65536
    mcall
    add edx,9
    add ebx,66*65536
    mcall
    add edx,9
    add ebx,66*65536
    mcall
    xor eax,eax
    mov ebx,eax
    mov ecx,eax
    mov al,47
    inc ebx
    shl ebx,16
    inc ecx
    mov edi,ecx
    mov edx,22*65536+86
    mov esi,00FF0000h
    mov ecx,5
dt_loc_00:
    push ecx
    mov ecx,edi
    mcall
    inc edi
    add dx,12
    pop ecx
    loop dt_loc_00
    xor ecx,ecx
    inc ecx
    mov edi,ecx
    mov edx,76*65536+63
    mov esi,000000FFh
    mov ecx,4
dt_loc_01:
    push ecx
    mov ecx,edi
    mcall
    inc edi
    add edx,66*65536
    pop ecx
    loop dt_loc_01
    mov eax,4
    mov ebx,16*65536+63
    mov ecx,000000FFh
    mov edx,_mk
    mov esi,4
    mcall
    shl ecx,16
    add bx,12
    add edx,4
    mcall
    retn

;IN: ah=keycode
safekey:
    sub ah,30h
    push bx
    mov bx,word [vmselect]
    cmp bx,0
    jnz sk_loc_00
    cmp ah,5
    je sk_loc_01
    mov bl,ah
    mov [vmselect],bx
    jmp sk_loc_01
sk_loc_00:
    push esi
    push edx
    push ecx
    push eax
    mov bh,ah
    xor edx,edx
    mov esi,_m1
    mov al,bl
    dec al
    xor ah,ah
    mov cx,10
    mul cx
    xor ecx,ecx
    mov cx,ax
    xor ax,ax
    mov al,bh
    dec al
    shl ax,1
    add cx,ax
    add esi,ecx
    lodsw
    cmp ax,0
    jnz sk_loc_02
    xor eax,eax
    mov bh,ah
sk_loc_02:
    mov [vmselect],bx
    pop eax
    pop ecx
    pop edx
    pop esi
sk_loc_01:
    call draw_window
    pop bx
    retn

; IN: ebx=Xstart*65536+Xend
;     ecx=Ystart*65536+Yend
;     edx=color
draw_rect:
    push eax
    push ebx
    push ecx
    push edx
    push edi
    xor eax,eax
    mov al,38
    push ecx
    mov edi,ecx
    shr edi,16
    mov cx,di
    mcall
    pop ecx
    push ecx
    mov edi,ecx
    ror ecx,16
    mov cx,di
    mcall
    pop ecx
    push ebx
    mov edi,ebx
    shr edi,16
    mov bx,di
    mcall
    pop ebx
    mov edi,ebx
    ror ebx,16
    mov bx,di
    mcall
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    retn

;
; OUT: eax = 0 - no event
protect_and_return:
    push ebx
    push ecx
    xor eax,eax
    mov al,5
    xor ebx,ebx
    mov bx,300
    mcall
    call get_pid
    xor eax,eax
    mov ebx,eax
    mov ecx,eax
    mov al,18
    mov ebx,3
    mov cx,[mypno]
    mcall
    pop ecx
    pusha
    call draw_window
    popa
    xor eax,eax
    mov al,5
    xor ebx,ebx
    mov bx,300
    mcall
    xor eax,eax
    mov al,11
    mcall
    cmp eax,1
    jne par_loc_00
    pusha
    call draw_window
    popa
par_loc_00:
    xor eax,eax
    mov ebx,eax
    mov al,23
    mov bx,700
    mcall
    cmp eax,0
    jnz par_loc_02
; mov [ftr_eax],eax
    mov edx,[oldvm]
    call set_my_mode
par_loc_02:
    pop ebx
    retn

debug_ftr:
;    xor eax,eax
;    mov ebx,eax
;    mov al,47
;    mov bl,8
;    shl ebx,16
;    mov bh,1
;    mov ecx,[ftr_eax]
;    mov edx,20*65536+180
;    mov esi,00FFFFFFh
;    mcall
;    mov ecx,[ftr_ebx]
;    add edx,54*65536
;    mcall
    retn

print_cur_vm:
    mov eax,4
    mov ebx,20*65536+40
    mov ecx,0000FF00h
    mov edx,curmode
    mov esi,cmlen
    mcall
    mov al,14
    mcall
    mov esi,00FFFFFFh
    mov edi,eax
    shr eax,16
    xor ecx,ecx
    mov cx,ax
    inc ecx
    xor ebx,ebx
    mov bl,4
    shl ebx,16
    mov edx,104*65536+40
    mov eax,47
    mcall
    add edx,30*65536
    mov cx,di
    inc ecx
    mcall
    add edx,30*65536
    mov ecx,[initrr]
    sub ebx,1*65536
    mcall
    mov al,4
    mov ebx,200*65536+40
    mov ecx,0000FF00h
    mov edx,selmode
    mov esi,cmlen
    mcall
    mov ax,[vmselect]
    cmp ax,0
    jz pcv_loc_00
    push eax
    xor eax,eax
    mov al,13
    mov ebx,284*65536+54
    mov ecx,40*65536+10
    mov edx,000020C0h
    mcall
    pop eax
    push eax
    xor ecx,ecx
    dec al
    mov cl,al
    shl cx,3
    add cl,al   ; cx=(al-1)*9
    mov edx,_m1280x1024
    add edx,ecx
    xor eax,eax
    mov al,4
    mov esi,9
    mov ebx,284*65536+40
    mov ecx,00ff0000h
    mcall
    pop eax
    cmp ah,0
    jz pcv_loc_00
    push esi
    push edx
    push ecx
    push eax
    xor eax,eax
    mov al,13
    mov ebx,344*65536+18
    mov ecx,40*65536+10
    mov edx,000020C0h
    mcall
    pop eax
    push eax
    mov bx,ax
    xor edx,edx
    mov esi,_m1
    mov al,bl
    dec al
    xor ah,ah
    mov cx,10
    mul cx
    xor ecx,ecx
    mov cx,ax
    xor ax,ax
    mov al,bh
    dec al
    shl ax,1
    add cx,ax
    add esi,ecx
    lodsw
    xor ecx,ecx
    mov cx,ax
    xor ebx,ebx
    mov bl,3
    shl ebx,16
    mov edx,344*65536+40
    xor eax,eax
    mov al,47
    mov esi,00ff0000h
    mcall
    pop eax
    pop ecx
    pop edx
    pop esi
 pcv_loc_00:
     retn

print_all_herz:
        push esi
        push edi
        push eax
        push ebx
        push ecx
        push edx
        cld
        mov esi,_m1
        mov ebx,(10+36+26)*65536+86
        mov edx,66*65536
        xor ecx,ecx
        mov cl,4
pah_loc_00:
        push ecx
        push edx
        push ebx
        mov cl,5
        xor edx,edx
        mov dl,12
pah_loc_01:
        lodsw
        cmp ax,00h
        jnz pah_loc_02
        call print_noherz
        jmp pah_loc_03
pah_loc_02:
        call print_herz
pah_loc_03:
        add ebx,edx
        loop pah_loc_01
        pop ebx
        pop edx
        add ebx,edx
        pop ecx
        loop pah_loc_00
        pop edx
        pop ecx
        pop ebx
        pop eax
        pop edi
        pop esi
        retn

; IN: ebx=X*65536+Y - coordinate
print_noherz:
        push eax
        push ebx
        push ecx
        push edx
        push esi
        xor eax,eax
        mov al,4
        mov ecx,00FFFFFFh
        mov edx,noherz
        xor esi,esi
        mov si,3
        mcall
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        retn

; IN: eax=numer_of_herz
;     ebx=X*65536+Y
print_herz:
        push eax
        push ebx
        push ecx
        push edx
        push esi
        mov edx,ebx
        xor ebx,ebx
        mov bl,3
        shl ebx,16
        mov ecx,eax
        mov esi,00FFFFFFh
        xor eax,eax
        mov al,47
        mcall
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        retn

get_pixelclock:
        retn

 ; light version of function
calc_refrate:
        retn

rect_select:
        mov ax,[vmselect]
;     mov [ftr_ebx],eax
        cmp ax,00h
        je rs_loc_00
        cmp ah,0
        jne rs_loc_01
        dec al
        mov cx,66
        mul cx
        add ax,46
        mov bx,ax
        shl ebx,16
        add ax,66
        mov bx,ax
        mov ecx,60*65536+144
        mov edx,00ff0000h
        call draw_rect
        retn
rs_loc_01:
        push ax
        xor ah,ah
        dec al
        xor ebx,ebx
        mov bx,66
        mul bx
        add ax,46
        mov bx,ax
        shl ebx,16
        add ax,66
        mov bx,ax
        pop ax
        xchg ah,al
        xor ah,ah
        dec al
        xor ecx,ecx
        mov cx,12
        mul cx
        add ax,84
        mov cx,ax
        shl ecx,16
        add ax,12
        mov cx,ax
        mov edx,00ff0000h
        call draw_rect
rs_loc_00:
        retn

draw_face:
        call draw_table
;
;
        mov ebx,320*65536+390
        mov ecx,66*65536+144
        mov edx,0000FF00h
        call draw_rect
        mov ebx,10*65536+390
        mov ecx,27*65536+55
        call draw_rect
        add ebx,2*65536
        sub bx,2
        add ecx,2*65536
        sub cx,2
        call draw_rect
        mov ebx,10*65536+390
        mov ecx,155*65536+193
        call draw_rect
        add ebx,2*65536
        sub bx,2
        add ecx,2*65536
        sub cx,2
        call draw_rect
        xor eax,eax
        mov al,13
        mov ebx,182*65536+36
        mov ecx,26*65536+5
        mov edx,000020C0h
        mcall
        mov ebx,173*65536+54
        mov ecx,153*65536+7
        mcall
        mov ebx,337*65536+36
        mov ecx,62*65536+10
        mcall
        mov al,4
        shr ecx,16
        mov bx,cx
        add ebx,3*65536
        mov ecx,00FF0000h
        mov edx,width
        mov esi,5
        mcall
        xor ecx,ecx
        add edx,5
        xor esi,esi
        inc esi
        mov ebx,335*65536+104
        mcall
        add ebx,36*65536
        inc edx
        mcall
        mov edx,tmode
        mov ecx,00FF0000h
        mov ebx,182*65536+24
        mov esi,6
        mcall
        mov edx,actions
        mov ebx,173*65536+152
        mov esi,9
        mcall
        xor ecx,ecx
        mov edx,button1
        mov ebx,59*65536+174
        mov esi,2
        mcall
        add edx,esi
        mov esi,6
        add ebx,78*65536
        mcall
        add edx,esi
        add ebx,90*65536
        mcall
        add edx,esi
        mov esi,7
        add ebx,87*65536
        mcall
        call rect_select
;        call debug_ftr
        call print_cur_vm
        call print_all_herz
        retn

warning_info:
        call warning_window
        call warning_loop
        retn

warning_window:
        mov  eax,12      ; function 12:tell os about windowdraw
        mov  ebx,1      ; 1, start of draw
        mcall
   ; DRAW WARNING WINDOW
        xor  eax,eax      ; function 0 : define and draw window
        mov ebx,[oldX]
        shr ebx,1
        sub ebx,200
        shl ebx,16
        mov bx,400
;        mov  ecx,100*65536+200    ; [y start] *65536 + [y size]
        mov ecx,[oldY]
        shr ecx,1
        sub ecx,100
        shl ecx,16
        mov cx,200
        mov  edx,0x13808080     ; color of work area RRGGBB,8->color glide
        mov  edi,title
        mcall

   ; WARNING TEXT
        mov  eax,4      ; function 4 : write text to window
        mov  ebx,(200-(len_warn00/2)*6)*65536+60    ; [x start] *65536 + [y
   ;]
        mov  ecx,0xf0ff0000     ; color of text RRGGBB
        mov  edx,warn00        ; pointer to text beginning
        mov  esi,len_warn00     ; text length
;        mcall
;        inc  ebx
        mcall
        add  ebx,1*65536
        mcall
        mov  ebx,(200-(len_warn01/2)*6)*65536+100
        mov  edx,warn01
        mov  esi,len_warn01
        mcall
        mov  edx,button1
        add  ecx,0ffffh
        mov  ebx,(200-6)*65536+(160-4)
        mov  esi,2
        mcall
        mov  eax,12      ; function 12:tell os about windowdraw
        mov  ebx,2      ; 2, end of draw
        mcall
        retn

warning_loop:
        mov  eax,5
        mov ebx,13
        mcall
        mov eax,11
        mcall
        cmp  eax,1      ; redraw request ?
        je  warning_red
        cmp  eax,2      ; key in buffer ?
        je  warning_key
        cmp  eax,3      ; button in buffer ?
        je  warning_button
        mov  eax,4
        mov  ebx,(200-(len_warn01/2)*6)*65536+100
        mov  ecx,[blinkcol]
        sub cl,12
        dec cl
        dec cl
        dec cl
        dec cl
        mov [blinkcol],ecx
        mov ch,0f0h
        shl ecx,16
        mov  edx,warn01
        mov  esi,len_warn01
        mcall
        sub ebx,1*65536
        mcall
        jmp  warning_loop
  warning_red:      ; redraw
        call warning_window
        jmp  warning_loop
  warning_key:      ; key
        mov  eax,2      ;  read key
        mcall
        cmp ah,01h
        jne warning_loop
        xor eax,eax
        dec eax         ; Terminate application
        mcall
        jmp warning_loop
  warning_button:   ; button
        mov  eax,17     ; get id
        mcall
        cmp  ah,1   ; button id=1 ?
        jne  warning_loop
        xor eax,eax
        dec eax         ; close this program
        mcall
        jmp warning_loop
        retn

;------------DATA AREA---------------

oldX       dd ?
oldY       dd ?
initvm     dd ?
currvm     dd 0
oldvm      dd 0
refrate    dd 0
initrr     dd 0
mypid      dd ?
mypno      dw ?
totp       dd ?
vmselect   dw 0
ftr_eax    dd ?
ftr_ebx    dd ?
blinkcol   dd 0ffh

;  db 0,0,0,0,0,0,0,0
;_m1        dw 0,0,0,0,0
;_m2        dw 0,0,0,0,0
;_m3        dw 0,0,0,0,0
;_m4        dw 0,0,0,0,0

if lang eq it
	title     db   'Vertical Refresh Rate v2.0 (C) 2003 TRANS',0

	_m1280x1024 db '1280x1024'
	_m1024x768  db '1024x768 '
	_m800x600   db ' 800x600 '
	_m640x480   db ' 640x480 '
	_mk         db 'Key1Key2'

	curmode     db 'Modalita intera: '
					db '    x    x   Hz'
	cmlen=$-curmode
	selmode     db ' Select mode: '
	selcans     db '----x----x---Hz'
	noherz      db '---'
	width       db 'Width',11h,10h
	tmode       db ' Modalita '
	actions     db ' Azioni '
	button1     db 'Ok'      ;len=2
	button2     db 'Cancella'  ;len=6
	button3     db 'Torna'  ;len=6
	button4     db 'Default' ;len=7

	strt  db 'LAUNCHER    '

	warn00      db ' A T T E N Z I O N E ! '
	len_warn00=$-warn00
	warn01      db 'D R I V E R  V I D E O  N O N  I N S T A L L A T O'
	len_warn01=$-warn01
else
	title     db   'Vertical Refresh Rate v2.0 (C) 2003 TRANS',0

	_m1280x1024 db '1280x1024'
	_m1024x768  db '1024x768 '
	_m800x600   db ' 800x600 '
	_m640x480   db ' 640x480 '
	_mk         db 'Key1Key2'

	curmode     db 'Current mode: '
					db '    x    x   Hz'
	cmlen=$-curmode
	selmode     db ' Select mode: '
	selcans     db '----x----x---Hz'
	noherz      db '---'
	width       db 'Width',11h,10h
	tmode       db ' Mode '
	actions     db ' Actions '
	button1     db 'Ok'      ;len=2
	button2     db 'Cancel'  ;len=6
	button3     db 'Return'  ;len=6
	button4     db 'Default' ;len=7

	strt  db 'LAUNCHER    '

	warn00      db ' W  A  R  N  I  N  G  ! '
	len_warn00=$-warn00
	warn01      db 'V i d e o  D r i v e r  N O T  I n s t a l l e d'
	len_warn01=$-warn01
end if

drvinfo:   ; 512 bytes driver info area
; +0   - Full driver name
; +32  - Driver version
; +64  - Word List of support video modes (max 32 positions)
; +128 - 5 words list of support vertical rate to each present mode
      org $+32
drvver:
      org $+32
vidmode:
      org $+64
_m1:
      org drvinfo+200h

buffer:
I_END:
