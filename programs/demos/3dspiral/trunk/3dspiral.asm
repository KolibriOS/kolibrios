;
;   application     :  3d shaking waved spiral
;   compilator      :  fasm
;   system          :  KolibriOS
;   author          :  macgub
;   email           :  macgub3@wp

timeout equ 3
maxx equ 616	    ; window size
maxy equ 420
use32

	       org    0x0

	       db     'MENUET01'	      ; 8 byte id
	       dd     0x01		      ; header version
	       dd     START		      ; start of code
	       dd     I_END		      ; size of image
	       dd     0x100000		      ; memory for app
	       dd     0xbffff		      ; esp
	       dd     0x0 , 0x0 	      ; I_Param , I_Icon

START:				; start of execution

     call draw_window

still:

;    mov  eax,23                 ; wait here for event
;    mov  ebx,timeout
;    int  0x40
    mov eax,11			 ; check for event no wait
    int 0x40

    cmp  eax,1			; redraw request ?
    je	 red
    cmp  eax,2			; key in buffer ?
    je	 key
    cmp  eax,3			; button in buffer ?
    je	 button

    jmp  noclose

  red:				; redraw
    call draw_window
    jmp  still

  key:				; key
    mov  eax,2			; just read it and ignore
    int  0x40
    shr  eax,8
    cmp  eax, 27
    jne  still
    mov  eax, -1
    int  0x40


  button:			; button
    mov  eax,17 		; get id
    int  0x40

    cmp  ah,1			; button id=1 ?
    jne  noclose

    mov  eax,-1 		; close this program
    int  0x40
  noclose:

; mov eax,13
; mov ebx,20*65536+maxx-25
; mov ecx,20*65536+maxy-25
; xor edx,edx
; int 0x40

 mov edi,screen_buf
 mov ecx,maxx*maxy*3/4
 xor eax,eax
 cld
 rep stosd


 call calc_deg
 mov [z],0
 mov [sin_counter],0
 finit
oopz:
 mov [x],0
 push [z]
 call calc_sin_variable
oop:
  push [x]
;  call getcol  ;(x,z)
  call fun			 ; calculates y and y1
;  call rotateY
  mov eax,[sin_variable]
  add eax,[vector_x]			 ;  vector_x
  add [x],eax
  mov eax,[vector_y]
  add [y],eax			  ;  vector_y
  add [y1],eax
  call point_perspective
  call draw_point_3d
 pop [x]
 inc [x]
 mov eax,[x]
 cmp eax,[loop_counter]
 jne oop
 inc [sin_counter]
 pop [z]
 inc [z]
 cmp [z],200
 jne oopz

 mov eax,7
 mov ebx,screen_buf
 mov ecx,maxx*65536+maxy
 mov edx,0*65536+0
 int 0x40

 call set_elipse_dim
 call set_vectors

jmp still
;-----------------++++++PROCEDURES
getcol:
 mov eax,[x_resolution]
 mul [z]
 add eax,[x]
 mov ebx,eax
 mov eax,35
 int 0x40
 mov [col],eax
ret

set_vectors:
 cmp [vector_x],55
 jne vec1
 mov [vector_dir_x],1
 vec1:
 cmp [vector_x],250
 jne vec2
 mov [vector_dir_x],0
 vec2:
 cmp [vector_dir_x],1
 jne vec3
 inc [vector_x]
 jmp end_x
 vec3:
 dec [vector_x]
 end_x:
 cmp [vector_y],195
 jne vec4
 mov [vector_dir_y],1
 vec4:
 cmp [vector_y],205
 jne vec5
 mov [vector_dir_y],0
 vec5:
 cmp [vector_dir_y],1
 jne vec6
 inc [vector_y]
 ret
 vec6:
 dec [vector_y]
ret
set_elipse_dim:
 cmp [b],60
 jne go11
 mov [elipse_dir],0
 go11:
 cmp [b],10
 jne go12
 mov [elipse_dir],1
 go12:
 cmp [elipse_dir],1
 jne go13
 inc [b]
 dec [a]
 mov eax,[a]
 mov [xo],eax
 shl eax,1
 inc eax
 mov [loop_counter],eax
 ret
 go13:
 dec [b]
 inc [a]
 mov eax,[a]
 mov [xo],eax
 shl eax,1
 inc eax
 mov [loop_counter],eax
ret

calc_deg:
 cmp [deg_counter], 360
 jne go_deg
 mov [deg_counter],0
 go_deg:
 fldpi
 fidiv [deg_div]
 fimul [deg_counter]
 fstp [current_deg]
; fsincos
; fstp [cosbeta]
; fstp [sinbeta]
 inc [deg_counter]
 ret

;rotateY:
; mov eax,[z]
; sub eax,[zoo]
; mov [subz],eax
; mov eax,[x]
; sub eax,[xoo]
; mov [subx],eax
;
; fld [sinbeta]
; fimul [subz]
; fchs
; fld [cosbeta]
; fimul[subx]
; faddp
; fiadd [xoo]
; fistp [x]

; fld [sinbeta]
; fimul [subx]
; fld [cosbeta]
; fimul [subz]
; faddp
; fiadd [zoo]
; fistp [z]
; finit

; ret

point_perspective:
  mov eax,[x]
  sub eax,[xobs]
  mov [xobssub],eax
  mov eax,[z]
  sub eax,[zobs]
  mov [zobssub],eax

  mov eax,[y]
  sub eax,[yobs]
  mov [yobssub],eax
  mov eax,[y1]
  sub eax,[yobs]
  mov [y1obssub],eax

  finit
  fild [xobssub]
  fidiv [zobssub]
  fimul [zobs]
  fchs
  fiadd [xobs]
  fistp [x]
  fild [yobssub]
  fidiv [zobssub]
  fimul [zobs]
  fchs
  fiadd [yobs]
  fistp [y]

;  mov eax,[xobssub]
; idiv [zobssub]
;; mov eax,edx
; imul [zobs]
; neg eax
;  add eax,[xobs]
;  mov [x],eax
;  mov eax,[yobssub]
;  idiv [zobssub]
;;  mov eax,edx
;  imul [zobs]
;  neg eax
;  add eax,[yobs]
;  mov [y],eax

  fild [y1obssub]
  fidiv [zobssub]
  fimul [zobs]
  fchs
  fiadd [yobs]
  fistp [y1]
ret
calc_sin_variable:
		     ;calculate sinus variable
 fldpi
 fidiv [sin_gran]
 fimul [sin_counter]
 fadd  [current_deg]
 fsin
 fimul [sin_mul]
 fistp [sin_variable]
ret

fun:
; finit
 fild [x]
 fisub [xo]
; fchs
; faddp
 fild [a]
 fdivp st1,st
 fmul st,st0
 fchs
 fld1
 faddp
 fsqrt
 fimul [b]
 fld st
 fchs
 fiadd [yo]
 fistp [y]
 fiadd [yo]
 fistp [y1]
ret
draw_point_3d:
 mov eax,[z]
 imul [sq]
 shr eax,10
 mov ebx,eax
 neg eax
 push eax
 add eax,[y]
 mov [y],eax
 pop eax
 add eax,[y1]
 mov [y1],eax
 mov eax,ebx
 add eax,[x]
 mov [x],eax
 ;mov eax,1
 ;mov ebx,[x]
 ;mov ecx,[y]
 ;mov edx,[col]
 ;int 0x40
 ;mov ecx,[y1]
 ;int 0x40
 mov eax,maxx
 mul [y]
 add eax,[x]
 mov ebx,eax
 shl ebx,1
 add eax,ebx
 add eax,screen_buf
 mov ebx,[col]
 or [eax],ebx
 mov eax,maxx
 mul [y1]
 add eax,[x]
 mov ebx,eax
 shl ebx,1
 add eax,ebx
 add eax,screen_buf
 mov ebx,[col]
 or [eax],ebx
ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:

	mov  eax,12                   ; function 12:tell os about windowdraw
    mov  ebx,1                    ; 1, start of draw; 2 - end
    int  0x40
 
	mov eax, 48                   ; get skin height
	mov ebx, 4
	int  0x40
	
	lea  ecx,[eax + (100 shl 16) + maxy + 4]
    mov  ebx,100*65536+maxx+9  ; [x start] *65536 + [x size]
    mov  edx,0x74000000           ; skinned window, not resizable
    mov  edi,labelt               ; window title
    mov  eax,0                    ; function 0 : define and draw window
    int  0x40

    mov  eax,12
    mov  ebx,2
    int  0x40
 
    ret
	
	
x_resolution dd 800
vector_x dd 200
vector_y dd 200
vector_dir_x db 1
vector_dir_y db 1
elipse_dir db 1

deg_counter dd ?   ; rotation variables
deg_div dd 20
current_deg dd ?
;cosbeta dd ?
;sinbeta dd ?
;zoo dd 100            ; rotation axle
;xoo dd 40
;yoo dd 20
;subx dd ?
;suby dd ?
;subz dd ?

xobs dd maxx/2	  ; 320     observer variables
yobs dd maxy/2	  ; 175
zobs dd -200
xobssub dd ?
yobssub dd ?
y1obssub dd ?
zobssub dd ?

sin_variable dd ?
sin_mul dd 60
sin_gran dd 30
sin_counter dd	0
sq dd 724	   ; round( (sqrt2)/2*1024 )
z dd ?
x dd ?
y dd ?
y1 dd ?
xo dd 70	   ; center point  , (loop counter-1)/2
yo dd 20
a dd 70 	   ; vertical half-axle
b dd 20 	   ; horizontal half-axle
loop_counter dd 141 ; axle granularity
col dd 0x00ffffff

labelt:
     db   ' 3D shaking waved spiral',0
labellen:
screen_buf:

I_END:




