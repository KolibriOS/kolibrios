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
  dd I_END
  dd 0x3000
  dd 0x3000
  dd 0x0
  dd 0x0

include 'macros.inc'
ENTRANCE: ; start of code

; ==== main ====
prepare_eyes:

mov esi,imagedata    ; transform grayscale to putimage format
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

mov esi,imagedata    ; calculate shape reference area
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

; -====- shape -====-

shape_window:

mov eax,50                   ; set up shape reference area
xor ebx,ebx
mov ecx,winref
int 0x40

call draw_window

still:

call draw_eyes                   ; draw those funny "eyes"

_wait:
mov eax,23                       ; wait for event with timeout
mov ebx,TIMEOUT
int 0x40
        dec     eax
        jz      redraw
        dec     eax
        jz      key
        dec     eax
        jnz     still
button:
        or      eax, -1
        int     0x40
key:
        mov     al, 2
        int     0x40
        jmp     still
redraw:
        call    draw_window
        call    redraw_eyes
        jmp     _wait

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

redraw_eyes:
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
ret

draw_window:

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

mov eax,12
mov ebx,2
int 0x40

ret

draw_eye_point:          ; draw eye point (EAX=X, EBX=Y)
pusha

        movzx   ecx, word [mouse+2] ; ecx = mousex, esi = mousey
        movzx   esi, word [mouse]

; ===> calculate position

push eax
push ebx
mov byte [sign1],0
mov edx, [win_ebx]
shr edx,16
add eax,edx
sub ecx,eax                 ; ECX=ECX-EAX (signed) , ECX=|ECX|
jnc abs_ok_1
neg ecx
mov byte [sign1],1
abs_ok_1:
        push    ecx         ; save x distance
mov byte [sign2],0
mov edx,[win_ecx]
shr edx,16
add ebx,edx
sub esi,ebx                 ; EDX=EDX-EBX (signed) , EDX=|EDX|
jnc abs_ok_2
neg esi
mov byte [sign2],1
abs_ok_2:
mov [temp2],esi

; ESI = ECX*ECX+ESI*ESI
        imul    ecx, ecx
        imul    esi, esi
        add     esi, ecx

xor  ecx,ecx                 ; EDX=SQRT(EBX)
xor  edx,edx
mov  eax,1
sqrt_loop:
; in this moment ecx=edx*edx, eax=1+2*edx
add  ecx,eax
inc  eax
inc  eax
inc  edx
cmp  ecx,esi
jbe  sqrt_loop
dec  edx
mov  eax,edx                   ; EDX=EDX/7
mov  dl,7
div  dl
and  eax,0xFF
mov  edx,eax                   ; EDX ? 0 : EDX=1
jnz  nozeroflag1
inc  edx
nozeroflag1:

        pop     eax             ; EAX = x distance
                                ; ECX=EAX/EDX
div  dl
movzx ecx,al
pop  ebx
pop  eax

        cmp     byte [sign1], 0
        jz      @f
        neg     ecx
@@:
        add     eax, ecx

push eax                      ; ESI=[temp2]/EDX
mov  eax,[temp2]
div  dl
movzx esi,al
pop  eax

        cmp     byte [sign2], 0
        jz      @f
        neg     esi
@@:
        add     ebx, esi

; <===

; draw point
        lea     ecx, [ebx-2]
        lea     ebx, [eax-2]
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

copy_line:       ; copy single line to shape reference area
mov ecx,30
cpl_loop:
lodsb
; input is image: 0xFF = white pixel, 0 = black pixel
; output is membership boolean: 0 = pixel no, 1 = pixel ok
inc eax
stosb
loop cpl_loop
ret

; DATA

; environment

win_ebx  dd     0x0
win_ecx  dd     0x0
mouse    dd     0xFFFFFFFF

EYES_END: ; end of code
imagedata:
; texture is 900 bytes starting from 25th
file "eyes.raw":25,900
I_END:

; temporary storage for math routines

sign1   db      ?
sign2   db      ?
align 4
temp2   dd      ?

skindata rb     60*30*3
winref  rb      45*60
