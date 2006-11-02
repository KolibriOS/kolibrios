;
; EYES FOR MENUET
;
; Written by Nikita Lesnikov (nlo_one@mail.ru)
;
; Position of "eyes" is fixed. To close "eyes" just click on them.
;
; NOTE: quite big timeout is used to disable blinking when redrawing.
; If "eyes" blink on your system, enlarge the TIMEOUT. If not, you can
; decrease it due to more realistic movement.
;

TIMEOUT equ 5

; EXECUTABLE HEADER

use32

  org 0x0
  db "MENUET01"
  dd 0x01
  dd ENTRANCE
  dd EYES_END
  dd 0x3000
  dd 0x3000
  dd 0x0
  dd 0x0

include 'macros.inc'
ENTRANCE: ; start of code

; ==== main ====

call prepare_eyes

call shape_window

still:

call draw_eyes                   ; draw those funny "eyes"

mov eax,23                       ; wait for event with timeout
mov ebx,TIMEOUT
int 0x40

cmp eax,1                        ; redraw ?
jnz  no_draw
call redraw_overlap
no_draw:

cmp eax,2                        ; key ?
jz  key

cmp eax,3                        ; button ?
jz  button

jmp still                        ; loop

; EVENTS

key:
mov eax,2        ; just read and ignore
int 0x40
jmp still

button:          ; analyze button
mov eax,-1       ; this is button 1 - we have only one button :-)
int 0x40
jmp still

; -====- declarations -====-

imagedata equ EYES_END
skindata  equ EYES_END+925
winref    equ EYES_END+6325

; -====- shape -====-

shape_window:

mov eax,50                   ; set up shape reference area
mov ebx,0
mov ecx,winref
int 0x40

ret

; -====- redrawing -====-

draw_eyes:                   ; check mousepos to disable blinking

mov eax,37
xor ebx,ebx
int 0x40
cmp dword [mouse],eax
jne redraw_ok
ret
redraw_ok:
mov [mouse],eax

redraw_overlap:              ; label for redraw event (without checkmouse)

mov eax,12
mov ebx,1
int 0x40

xor eax,eax                  ; define window
mov ebx,[win_ebx]
mov ecx,[win_ecx]
xor edx,edx
xor esi,esi
xor edi,edi
int 0x40

mov eax,8                    ; define closebutton
mov ebx,60
mov ecx,45
mov edx,1
int 0x40

mov eax,7
mov ebx,skindata
mov ecx,60*65536+30
mov edx,15
int 0x40

mov eax,15
mov ebx,30
call draw_eye_point
add eax,30
call draw_eye_point

mov eax,12
mov ebx,2
int 0x40

ret

draw_eye_point:          ; draw eye point (EAX=X, EBX=Y)
pusha

mov ecx, [mouse]    ; ecx = mousex, edx = mousey
mov edx,ecx
shr ecx,16
and edx,0xFFFF

; ===> calculate position

push eax
push ebx
mov byte [sign1],0
mov esi, [win_ebx]
shr esi,16
add eax,esi
sub ecx,eax                 ; ECX=ECX-EAX (signed) , ECX=|ECX|
jnc abs_ok_1
neg ecx
mov byte [sign1],1
abs_ok_1:
mov [temp1],ecx
mov byte [sign2],0
mov esi,[win_ecx]
shr esi,16
add ebx,esi
sub edx,ebx                 ; EDX=EDX-EBX (signed) , EDX=|EDX|
jnc abs_ok_2
neg edx
mov byte [sign2],1
abs_ok_2:
mov [temp2],edx
pop ebx
pop eax

push eax                    ; ECX*=ECX
push edx
xor eax,eax
xor edx,edx
mov ax,cx
mul cx
shl edx,16
or  eax,edx
mov ecx,eax
pop edx
pop eax

push eax                    ; EDX*=EDX
push ecx
mov  ecx,edx
xor  eax,eax
xor  edx,edx
mov  ax,cx
mul  cx
shl  edx,16
or   eax,edx
mov  edx,eax
pop  ecx
pop  eax

push ebx
push ecx
push edx
push eax
mov  ebx,ecx                 ; EBX=ECX+EDX
add  ebx,edx
xor  edi,edi                 ; ESI=SQRT(EBX)
mov  ecx,edi
mov  edx,edi
inc  edi
mov  eax,edi
inc  edi
sqrt_loop:
add  ecx,eax
add  eax,edi
inc  edx
cmp  ecx,ebx
jbe  sqrt_loop
dec  edx
mov  esi,edx
mov  ax,si                   ; ESI=ESI/7
mov  dl,7
div  dl
and  ax,0xFF
mov  si,ax                   ; ESI ? 0 : ESI=1
jnz  nozeroflag1
mov  si,1
nozeroflag1:

pop eax
pop edx
pop ecx
pop ebx

push eax                     ; ECX=[temp1]/ESI
push edx
mov  eax,[temp1]
mov  dx,si
div  dl
mov  cl,al
and  ecx,0xFF
pop  edx
pop  eax

cmp  byte [sign1],1
je   subtract_1
add  eax,ecx                  ; EAX=EAX+ECX
jmp  calc_ok_1
subtract_1:
sub  eax,ecx                  ; EAX=EAX-ECX
calc_ok_1:

push eax                      ; EDX=[temp2]/ESI
push ecx
mov  eax,[temp2]
mov  dx,si
div  dl
mov  dl,al
and  dx,0xFF
pop  ecx
pop  eax

cmp  byte [sign2],1
je   subtract_2
add  ebx,edx                  ; EBX=EBX+EDX
jmp  calc_ok_2
subtract_2:
sub  ebx,edx                  ; EBX=EBX-EDX
calc_ok_2:

; <===

mov ecx,ebx         ; draw point
mov ebx,eax
mov eax,13
dec ecx
dec ecx
dec ebx
dec ebx
shl ecx,16
add ecx,4
shl ebx,16
add ebx,4
mov eax,13
xor edx,edx
int 0x40

popa
ret

; -====- working on images and window -====-

prepare_eyes:

;mov eax,6            ; load EYES.RAW
;mov ebx,graphix
;mov ecx,0x00000000
;mov edx,0xFFFFFFFF
;mov esi,imagedata
;int 0x40
;cmp eax,0xFFFFFFFF
;jnz filefound

;mov eax,-1           ; file not exists...
;int 0x40

;filefound:
mov esi,imagedata+25 ; transform grayscale to putimage format
mov edi,skindata
mov ecx,30
transform_loop:
push ecx
mov  ecx,30
lp1:
lodsb
stosb
stosb
stosb
loop lp1
sub esi,30
mov  ecx,30
lp2:
lodsb
stosb
stosb
stosb
loop lp2
pop  ecx
loop transform_loop

mov eax,14           ; calculating screen position
int 0x40
shr eax,1
mov ax,59
sub eax,30*65536
mov [win_ebx],eax
mov [win_ecx],dword 10*65536+44

mov esi,imagedata+25 ; calculate shape reference area
mov edi,winref
mov ecx,900          ; disable drag bar
mov al,0
rep stosb

mov ecx,30           ; calculate circles for eyes
shape_loop:
push ecx

call copy_line       ; duplicate (we have two eyes :)
sub  esi,30
call copy_line

pop  ecx
loop shape_loop

ret

copy_line:       ; copy single line to shape reference area
mov ecx,30
cpl_loop:
lodsb
cmp al,0xFF
jnz  set_one
mov al,0
jmp cpl_ok
set_one:
mov al,1
cpl_ok:
stosb
loop cpl_loop
ret

; DATA

; environment

win_ebx  dd     0x0
win_ecx  dd     0x0
mouse    dd     0xFFFFFFFF
;graphix  db     "EYES.RAW    "

; temporary storage for math routines

temp1    dd     0
temp2    dd     0
sign1    db     0
sign2    db     0

EYES_END: ; end of code
file "EYES.RAW"
