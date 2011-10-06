;
;   application : WAVED 3D FORM SIN(SQRT(x*x+y*y))
;   author      : macgub
;   email       : macgub3@wp.pl
;   system      : MenuetOS
;   compiler    : fasm
;
;
TIMEOUT EQU 2
use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x100000                ; memory for app
               dd     0x7fff0                 ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

START:                          ; start of execution

     call draw_window

still:

    mov  eax,23                 ; wait here for event
    mov  ebx,TIMEOUT
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    jmp  noclose

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40
    jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose

    mov  eax,-1                 ; close this program
    int  0x40
  noclose:

    call calc_deg

;    mov eax,13    ;clear screen
;    mov ebx,5*65536 + 517
;    mov ecx,20*65536 + 345
;    xor edx,edx
;    int 0x40

    mov edi,the_mem        ;clear buffer
    mov ecx,512*384*3/4
    xor eax,eax
    cld
    rep stosd

    mov eax,1    ;main loop
    oop:
    mov ebx,1
    oop1:
     push eax
     push ebx
     mov [x],eax
     mov [z],ebx
     call fun
     add [y],260
     add [x],40
     call rotateY
     call put3dpixel
     pop ebx
     pop eax
    add ebx,4
    cmp ebx,201
    jne oop1
    inc eax
    cmp eax,300
    jne oop

    mov eax,7    ;put image
    mov ebx,the_mem
    mov ecx,512*65536+384
    mov edx,0
    int 0x40

    jmp  still
;-------------------PROCEDURES-------------------------------
calc_deg:
 cmp [deg_counter], 360
 jne go_deg
 mov [deg_counter],0
 go_deg:
 fldpi
 fidiv [deg_div]
 fimul [deg_counter]
 fst [current_deg]
 fsincos
 fstp [cosbeta]
 fstp [sinbeta]
 inc [deg_counter]
 ret
;-----------------
rotateY:
 mov eax,[z]
 sub eax,[zo]
 mov [subz],eax
 mov eax,[x]
 sub eax,[xo]
 mov [subx],eax

 fld [sinbeta]
 fimul [subz]
 fchs
 fld [cosbeta]
 fimul[subx]
 faddp
 fiadd [xo]
 fistp [x]

 fld [sinbeta]
 fimul [subx]
 fld [cosbeta]
 fimul [subz]
 faddp
 fiadd [zo]
 fistp [z]
 ret
;-----------------------------
fun:
 finit
 fild [x]
 fisub [i150]
 fidiv [xdiv]
 fmul st,st0
 fld st0
 fild [z]
 fisub [i100]
 fidiv [zdiv]
 fmul st,st0
 faddp
 fsqrt

 fld [current_deg]
 fimul [cur_deg_mul]
 faddp
 fsin
 fimul [ymul]
 fistp [y]
 ret
;--------------------------
put3dpixel:
 fild [z]
 fmul [sq]
 fchs
 fiadd [y]
 fistp [y]
 fild [z]
 fmul [sq]
 fiadd [x]
 fistp [x]
 ;mov eax,1
 ;mov ebx,[x]
 ;mov ecx,[y]
 ;mov edx,[col]
 ;int 0x40
 mov eax,[y]    ; calculating pixel position in buffer
 dec eax
 shl eax,9
 add eax,[x]
 mov ebx,eax
 shl eax,1
 add eax,ebx
 add eax,the_mem
 mov ebx,[col]
 or [eax],ebx
 ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:


    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

	mov  eax,48
	mov  ebx,4
	int  0x40
	mov  esi, eax
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+512+9         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+384+4         ; [y start] *65536 + [y size]
	add  ecx, esi
	mov  edx,0x74000000 	  	   ; color of work area RRGGBB,8->color gl
    mov  edi,labelt
	int  0x40

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret

; DATA AREA
deg_div dd 90
deg_counter dd ?
col dd 0x00ffffff
sq dd 0.707    ;sin pi/4
cosbeta dd ?
sinbeta dd ?
x dd ?
y dd ?
z dd ?
xo dd 200           ;axle variables
;yo dd 110
zo dd 80
xdiv dd 20
zdiv dd 20
ymul dd 20
subx dd ?
;suby dd ?
subz dd ?
current_deg dd ? ;real
cur_deg_mul dd 12
i100 dd 100
i150 dd 150
labelt:
     db   'Waved 3d form',0
the_mem:
I_END:




