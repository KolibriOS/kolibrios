; <--- description --->
; compiler:     FASM 1.67.23
; name:         Mouse Emulation For KolibriOS
; version:      0.8
; last update:  03/10/2007
; written by:   Zlobny_Gluk
; e-mail:       umbrosia@land.ru


; <--- include all KolibriOS stuff --->
;include "lang.inc"
include "MACROS.INC"


; <--- start of KolibriOS application --->
MEOS_APP_START


; <--- start of code --->
CODE

  start:

  ;set an waiting mask
   mcall 40,10b


  wait_event_one:

    call checkspeed


    mov eax,66
    mov ebx,3
    int 0x40
    test eax,0x80
    jnz wait_event_two

    cmp ebp,1
    je unset_hotkeys


    mov eax,5
    mov ebx,10
    int 0x40

    jmp wait_event_one


  wait_event_two:
    cmp ebp,0
    je set_hotkeys
    mov ebp,1
    mov eax,10
    int 0x40

    cmp eax, 2			   ; if event == 2
    jmp key			   ; jump to key handler

    jmp wait_event_one		   ; else return to the start of main cycle



  key:				   ; key event handler
    mov eax, 2			   ; get key code
    int 0x40
    cmp al,2
    je mousewriteli
    jmp wait_event_one



  set_hotkeys:


    mov eax,66
    mov ebx,4
    mov cl,79d
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,4
    mov cl,80
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,4
    mov cl,81
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,4
    mov cl,75
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,4
    mov cl,76
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,4
    mov cl,77
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,4
    mov cl,71
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,4
    mov cl,72
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,4
    mov cl,73
    mov edx,0
    int 0x40


  mov ebp,1

  jmp wait_event_two


  unset_hotkeys:


    mov eax,66
    mov ebx,5
    mov cl,79
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,5
    mov cl,80
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,5
    mov cl,81
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,5
    mov cl,75
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,5
    mov cl,76
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,5
    mov cl,77
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,5
    mov cl,71
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,5
    mov cl,72
    mov edx,0
    int 0x40

    mov eax,66
    mov ebx,5
    mov cl,73
    mov edx,0
    int 0x40


    xor ebp,ebp

  jmp wait_event_one

checkspeed:
   mov eax,18
   mov ebx,19
   xor ecx,ecx
   int 0x40
   mov esi,eax
ret


  mouseread:

    ;numchecking
    mov eax,66
    mov ebx,3
    int 0x40
    test eax,0x80
    jz wait_event_one

    ;mousereading
    mov eax,37
    mov ebx,0
    int 0x40
    add edx,eax


  sravn:
    xor ebx,ebx
    xor edi,edi


    real:
    mov ebx,edx
    mov edi,ebx
    shr ebx,16 ; get x1
    shl edi,16 ; get y1
    shr edi,16

    nullli:
    add ebx,16
    cmp ebx,65535
    jg xmin
    sub ebx,15

    add edi,16
    cmp edi,65535
    jg ymin
    sub edi,15

    razr:
    mov eax,14
    int 0x40


    mov ecx,eax
    shr eax,16 ; get x2
    shl ecx,16 ; get y2
    shr ecx,16

    rightdownli:
    cmp eax,ebx
    jl xmax
    cmp ecx,edi
    jl ymax


  mousewrite:
    mov eax,18
    mov ebx,19
    mov ecx,4
    int 0x40
  jmp wait_event_one

  xmax:

    dec eax
    dec ebx
    dec edi
    shl eax,16
    add edi,eax
    mov edx,edi
    jmp sravn

  xmin:

    mov edx,edi
    jmp sravn


  ymax:

    dec ecx
    dec ebx

    shl ebx,16
    mov edi,ebx
    add edi,ecx
    mov edx,edi
    jmp sravn

  ymin:
    shl ebx,16
    mov edx,ebx
    shr ebx,16
    jmp sravn







  mousewriteli:
    cmp ah,79
    je keyone
    cmp ah,80
    je keytwo
    cmp ah,81
    je keythree
    cmp ah,75
    je keyfour
    cmp ah,77
    je keysix
    cmp ah,71
    je keyseven
    cmp ah,72
    je keyeight
    cmp ah,73
    je keynine
    cmp ah,76
    je keyfive

  jmp wait_event_one


;how_much_time:
;mov eax,26
;mov ebx,9
;int 0x40
;push eax
;ret


;oldkeyvsyakoe!
;incesi:
;inc esi
;ret

;nullesi:
;xor esi, esi
;ret


;oldkey:
;cmp ah, bh
;je incesi
;cmp ah, bh
;jne nullesi
;ret


keyone:

;call oldkey
xor edx,edx
call left
call down
xor ah,ah
mov bh,79
jmp mouseread

keytwo:

;call oldkey
xor edx,edx
call down
call down
xor ah,ah
mov bh,80
jmp mouseread

keythree:

;call oldkey
xor edx,edx
call right
call down
xor ah,ah
mov bh,81
jmp mouseread

keyfour:

;call oldkey
xor edx,edx
call left
call left
xor ah,ah
mov bh,75
jmp mouseread

keysix:

;call oldkey
xor edx,edx
call right
call right
xor ah,ah
mov bh,77
jmp mouseread

keyseven:

;call oldkey
xor edx,edx
call left
call up
xor ah,ah
mov bh,71
jmp mouseread

keyeight:

;call oldkey
xor edx,edx
call up
call up
xor ah,ah
mov bh,72
jmp mouseread

keynine:

;call oldkey
xor edx,edx
call right
call up
xor ah,ah
mov bh,73
jmp mouseread

keyfive:

 mcall 18, 19, 5, 1
 mov eax,5
 mov ebx,1
 int 0x40
 mcall 18, 19, 5, 0
jmp wait_event_one




left:
mov eax,esi

leftstart:
dec eax
sub edx,65536
cmp eax,0
ja leftstart
ret


right:
mov eax,esi

rightstart:
dec eax
add edx,65536
cmp eax,0
ja rightstart
ret


down:
mov eax,esi

downstart:
dec eax
add edx,1
cmp eax,0
ja downstart
ret


up:
mov eax,esi

upstart:
dec eax
sub edx,1
cmp eax,0
ja upstart
ret











; <--- initialised data --->
DATA


; <--- uninitialised data --->
UDATA


MEOS_APP_END
; <--- end of KolibriOS application --->
; ZG