;
; application : Flag - Polonia in Tertio Millenium - wavy shading rotary area
; compiler    : FASM
; system      : MenuetOS
; author      : macgub
; email       : macgub3@wp.pl
; web         : www.menuet.xt.pl
; Fell free to use this intro in your own distribution of MenuetOS.
SIZE_X equ 220
SIZE_Y equ 260
TIMEOUT equ 1
ROUND equ 12
points_count equ 50
triangles_count equ 54

use32

	org    0x0

	db     'MENUET01'	; 8 byte id
	dd     0x01	   ; header version
	dd     START	    ; start of code
	dd     I_END	    ; size of image
	dd     I_END	    ; memory for app
	dd     I_END	    ; esp
	dd     0x0 , 0x0	; I_Param , I_Icon

START:	  ; start of execution

     call draw_window
 ;    call generate_map

still:

    mov  eax,23   ; wait here for event with timeout
    mov  ebx,TIMEOUT
    cmp [speed_flag],0xff
    jne speed_skip
    mov eax,11
   speed_skip:
    int  0x40

    cmp  eax,1	 ; redraw request ?
    je	red
    cmp  eax,2	 ; key in buffer ?
    je	key
    cmp  eax,3	 ; button in buffer ?
    je	button

    jmp  noclose

  red:	  ; redraw
    call draw_window
    jmp  noclose

  key:	  ; key
    mov  eax,2	 ; just read it and ignore
    int  0x40
    jmp  noclose

  button:   ; button
    mov  eax,17   ; get id
    int  0x40

    cmp  ah,1	; button id=1 ?
    jne shad_button
    mov  eax,-1   ; close this program
    int  0x40
    shad_button:
    cmp ah,2
    jne speed_button
    not [shad_flag]	    ; set shadow / flag mode
    speed_button:
    cmp ah,3
    jne noclose
    not [speed_flag]
  noclose:

  call calculate_angle	 ; calculates sinus and cosinus
  call generate_map
  call copy_points
  call rotate_points
  call translate_points  ; translate from 3d to 2d
  call clrscr		 ; clear the screen
  call sort_triangles
  call draw_triangles	 ; draw all triangles from the list

  mov eax,7	     ; put image
  mov ebx,screen
  mov ecx,SIZE_X shl 16 + SIZE_Y
  mov edx,5 shl 16 + 20
  int 0x40

  jmp  still
generate_map:
  finit
  mov edi,points
  xor ebx,ebx	   ;z
 again_gen1:
  mov eax,70	       ;x
 again_gen:
  mov word[edi],ax
  mov word[edi+4],bx
  fild word[edi]
  fidiv [i20]
  fadd [current_angle]
  fsin
  fimul [i20]
  fiadd [i75]
  fistp word [edi+2]
;  fild word[edi]       ;another map generation
;  fisub [i100]
;  fidiv [i75]
;  fmul st,st0
;  fild word[edi+4]
;  fisub [i50]
;  fidiv [i20]
;  fmul st,st0
;  faddp
;  fsqrt
;  fadd [current_angle]
;  fsin
;  fimul [i20]
;  fiadd [i75]
;  fistp word[edi+2]

  add ax,10
  add edi,6
  cmp ax,170
  jne again_gen
  add bx,20
  cmp bx,100
  jne again_gen1
  mov dword[edi],0xffffffff
ret
i20 dw 20
i50 dw 50
i75 dw 75
i100 dw 100

sort_triangles:
  mov esi,triangles
  mov edi,triangles_with_z
  mov ebp,points_rotated

 make_triangle_with_z:		  ;makes list with triangles and z position
  xor eax,eax
  mov ax,word[esi]
  shl eax,1
  mov ebx,eax
  shl eax,1
  add eax,ebx
  push ebp
  add ebp,eax
  xor ecx,ecx
  mov cx,word[ebp+4]
  pop ebp

  xor eax,eax
  mov ax,word[esi+2]
  shl eax,1
  mov ebx,eax
  shl eax,1
  add eax,ebx
  push ebp
  add ebp,eax
  add cx,word[ebp+4]
  pop ebp

  xor eax,eax
  mov ax,word[esi+4]
  shl eax,1
  mov ebx,eax
  shl eax,1
  add eax,ebx
  push ebp
  add ebp,eax
  add cx,word[ebp+4]
  pop ebp

  mov ax,cx
  cwd
  idiv [i3]
  cld
  movsd      ; store vertex coordinates
  movsw
  stosw      ; middle vertex coordinate  'z' in triangles_with_z list
  cmp dword[esi],0xffffffff
  jne make_triangle_with_z
  movsd   ; copy end mark

;macro sort

  mov [sort_flag],1
 next_booble:
  mov esi,triangles_with_z   ;sort list triangles_with_z booble metod
  cmp [sort_flag],0
  je end_sort
  mov [sort_flag],0
 check_and_check:
 ; cmp dword[esi],0xffffffff
 ; je next_booble
  cmp dword[esi+8],0xffffffff
  je next_booble
  mov ax,word[esi+6]
  cmp ax,word[esi+14]
  jge no_chg_pos
  mov eax,dword[esi]
  mov ebx,dword[esi+4]
  xchg eax,dword[esi+8]
  xchg ebx,dword[esi+12]
  mov dword[esi],eax
  mov dword[esi+4],ebx		     ; sort_flag=1 if change occured
  mov [sort_flag],1
 no_chg_pos:
  add esi,8
  jmp check_and_check	  ;check end mark end if greater
 end_sort:

 ; translate triangles_with_z to sorted_triangles
  mov esi,triangles_with_z
  mov edi,sorted_triangles
 again_copy:
  movsd
  movsw
  add esi,2
  cmp dword[esi],0xffffffff
  jne again_copy
  movsd  ; copy end mark too
ret
sort_flag db 0
clrscr:
  mov edi,screen
  mov ecx,SIZE_X*SIZE_Y*3/4
  xor eax,eax
  cld
  rep stosd
ret
calculate_angle:
  finit
  fldpi
  fidiv [i180]
  fimul [angle_counter]
  fst [current_angle]
  fld st
  fidiv [i2]
  fsincos
  fstp [singamma]
  fstp [cosgamma]
  fsincos
  fstp [sinbeta]
  fstp [cosbeta]
  inc [angle_counter]
  cmp [angle_counter],360
  jne end_calc_angle
  mov [angle_counter],0
 end_calc_angle:
ret
i180 dw 90
i2 dw 2
rotate_points:
  finit 			  ; y axle rotate
  mov ebx,points_rotated
 again_r:
  mov ax,word[ebx] ;x
  sub ax,[xo]
  mov [xsub],ax
  mov ax,word[ebx+4] ;z
  sub ax,[zo]
  mov [zsub],ax
  fld [sinbeta]
  fimul [zsub]
  fchs
  fld [cosbeta]
  fimul [xsub]
  faddp
  fiadd [xo]
  fistp word[ebx]  ;x
  fld [sinbeta]
  fimul [xsub]
  ;fchs
  fld [cosbeta]
  fimul [zsub]
  faddp
  fiadd [zo]
  fistp word[ebx+4] ;z

  mov ax,word[ebx+2] ;y                  ; z axle rotate
  sub ax,[yo]
  mov [ysub],ax
  mov ax,word[ebx]  ;x
  sub ax,[xo]
  mov [xsub],ax
  fld [singamma]
  fimul[ysub]
  fld [cosgamma]
  fimul [xsub]
  faddp
  fiadd [xo]
  fistp word[ebx] ;x
  fld [cosgamma]
  fimul [ysub]
  fld [singamma]
  fimul [xsub]
  fchs
  faddp
  fiadd [yo]
  fistp word[ebx+2] ;y

  add ebx,6
  cmp dword[ebx],0xffffffff
  jne again_r
ret
xsub dw ?
ysub dw ?
zsub dw ?
draw_triangles:
  mov [tr_counter],1
  mov ebp,points_rotated
;  mov esi,triangles
  mov esi,sorted_triangles
 again_dts:
  xor eax,eax
  mov ax,word[esi]
  shl eax,1
  mov [dtpom],eax
  shl eax,1
  add eax,[dtpom]
  push ebp
  add ebp,eax
  mov ax,word[ebp]
  mov [xx1],ax
  mov ax,word[ebp+2]
  mov [yy1],ax
  mov ax,word[ebp+4]
  mov [zz1],ax
  pop ebp

  xor eax,eax
  mov ax,word[esi+2]
  shl eax,1
  mov [dtpom],eax
  shl eax,1
  add eax,[dtpom]
  push ebp
  add ebp,eax
  mov ax,word[ebp]
  mov [xx2],ax
  mov ax,word[ebp+2]
  mov [yy2],ax
  mov ax,word[ebp+4]
  mov [zz2],ax
  pop ebp

  xor eax,eax
  mov ax,word[esi+4]
  shl eax,1
  mov [dtpom],eax
  shl eax,1
  add eax,[dtpom]
  push ebp
  add ebp,eax
  mov ax,word[ebp]
  mov [xx3],ax
  mov ax,word[ebp+2]
  mov [yy3],ax
  mov ax,word[ebp+4]
  mov [zz3],ax
  pop ebp
  push ebp
  push esi

macro set_flag
{
  mov edx,0x00ffffff
  inc [tr_counter]
  cmp [tr_counter],triangles_count/2
  jl skip_red
 set_red:
  mov edx,0x00ff0000
 skip_red:
}

  mov ax,[zz1]
  add ax,[zz2]
  add ax,[zz3]
  cwd
  idiv [i3]
  sub ax,100 ;77
;  shl ax,1
  neg al
  xor edx,edx
  mov dh,al		     ;set color according to z position
  mov dl,al
;  push dx
;  shl edx,8
;  pop dx

  cmp [shad_flag],0
  je skip_col
  set_flag
 skip_col:
  mov ax,[xx1]
  shl eax,16
  mov ax,[yy1]
  mov bx,[xx2]
  shl ebx,16
  mov bx,[yy2]
  mov cx,[xx3]
  shl ecx,16
  mov cx,[yy3]
  mov edi,screen
  call draw_triangle
  pop esi
  pop ebp

  add esi,6
  cmp dword[esi],0xffffffff
  jne again_dts
ret
i3 dw 3
tr_counter dw 0
dtpom dd ?
xx1 dw ?
yy1 dw ?
zz1 dw ?
xx2 dw ?
yy2 dw ?
zz2 dw ?
xx3 dw ?
yy3 dw ?
zz3 dw ?
translate_points:
  finit
  mov ebx,points_rotated
 again_trans:
  fild word[ebx+4] ;z1
  fmul [sq]
  fld st
  fiadd word[ebx]  ;x1
  fistp word[ebx]
  fchs
  fiadd word[ebx+2] ;y1
  fistp word[ebx+2] ;y1

  add ebx,6
  cmp dword[ebx],0xffffffff
  jne again_trans
ret
copy_points:
  mov esi,points
  mov edi,points_rotated
  mov ecx,points_count*3+2
  cld
  rep movsw
ret

draw_triangle:
;----------in - eax - x1 shl 16 + y1
;------------- -ebx - x2 shl 16 + y2
;---------------ecx - x3 shl 16 + y3
;---------------edx - color 0x00rrggbb
;---------------edi - pointer to screen buffer
 @ch3:
  cmp ax,bx
  jg @ch1
 @ch4:	  ; sort parameters
  cmp bx,cx
  jg @ch2
  jle @chEnd
 @ch1:
  xchg eax,ebx
  jmp @ch4
 @ch2:
  xchg ebx,ecx
  jmp @ch3
 @chEnd:
  mov [@y1],ax	; ....and store to user friendly  variables
  mov [@y2],bx
  mov [@y3],cx
  shr eax,16
  shr ebx,16
  shr ecx,16
  mov [@x1],ax
  mov [@x2],bx
  mov [@x3],cx
  mov [@col],edx

  cmp [@y1],0
  jl @end_triangle
  cmp [@y2],0
  jl @end_triangle
  cmp [@y3],0
  jl @end_triangle
  cmp [@x1],0
  jl @end_triangle
  cmp [@x2],0
  jl @end_triangle
  cmp [@x3],0
  jl @end_triangle
  cmp [@y1],SIZE_Y
  jg @end_triangle
  cmp [@y2],SIZE_Y
  jg @end_triangle
  cmp [@y3],SIZE_Y
  jg @end_triangle
  cmp [@x1],SIZE_X
  jg @end_triangle
  cmp [@x2],SIZE_X
  jg @end_triangle
  cmp [@x3],SIZE_X
  jg @end_triangle

  neg ax       ; calculate delta 12
  add ax,bx
  cwde
  shl eax,ROUND
  cdq
  mov bx,[@y2]
  mov cx,[@y1]
  sub ebx,ecx
  cmp ebx,0
  jne @noZero1
  mov [@dx12],0
  jmp @yesZero1
 @noZero1:
  idiv ebx
  mov  [@dx12],eax
 @yesZero1:

  mov ax,[@x3]	      ; calculate delta 13
  sub ax,[@x1]
  cwde
  shl eax,ROUND
  cdq
  xor ebx,ebx
  xor ecx,ecx
  or bx,[@y3]
  or cx,[@y1]
  sub ebx,ecx
  cmp ebx,0
  jne @noZero2
  mov [@dx13],0
  jmp @yesZero2
 @noZero2:
  idiv ebx
  mov [@dx13],eax
 @yesZero2:

  mov ax,[@x3]	   ; calculate delta 23 [dx23]
  sub ax,[@x2]
  cwde
  shl eax,ROUND
  cdq
  xor ebx,ebx
  xor ecx,ecx
  or bx,[@y3]
  or cx,[@y2]
  sub ebx,ecx
  cmp ebx,0
  jne @noZero3
  mov [@dx23],0
  jmp @yesZero3
 @noZero3:
  idiv ebx
  mov [@dx23],eax
 @yesZero3:


  xor eax,eax	 ;eax - xk1
  or ax,[@x1]
  shl eax,ROUND
  mov ebx,eax	  ; ebx - xk2
  xor esi,esi	    ; esi - y
  or si,[@y1]
 @next_line1:
  mov ecx,eax	  ; ecx - x11
  sar ecx,ROUND
  mov edx,ebx	   ;edx - x12
  sar edx,ROUND
  cmp ecx,edx
  jle @nochg
  xchg ecx,edx
 @nochg:
  pusha
  mov ebx,ecx
  sub edx,ecx
  mov ecx,edx
  mov edx,esi
  mov eax,[@col]
  call @horizontal_line
  popa
  add eax,[@dx13]
  add ebx,[@dx12]
  inc esi
  cmp si,[@y2]
  jl @next_line1

  xor esi,esi
  or si,[@y2]
  xor ebx,ebx
  mov bx,[@x2]
  shl ebx,ROUND
 @next_line2:
  mov ecx,eax
  sar ecx,ROUND
  mov edx,ebx
  sar edx,ROUND
  cmp ecx,edx
  jle @nochg1
  xchg ecx,edx
 @nochg1:
  pusha
  mov eax,[@col]
  mov ebx,ecx
  sub edx,ecx
  mov ecx,edx
  mov edx,esi
  call @horizontal_line
  popa
  add eax,[@dx13]
  add ebx,[@dx23]
  inc esi
  cmp si,[@y3]
  jl @next_line2
  @end_triangle:
ret
@col dd ?
@y1 dw ?
@x1 dw ?
@y2 dw ?
@x2 dw ?
@y3 dw ?
@x3 dw ?
@dx12 dd ?
@dx13 dd ?
@dx23 dd ?

@horizontal_line:
;---------in
;---------eax - color of line,  0x00RRGGBB
;---------ebx - x1 - x position of line begin
;---------ecx - lenght of line
;---------edx - y position of line
;---------edi - pointer to buffer
  jcxz @end_hor_l
  push eax
  mov eax,SIZE_X*3
  mul edx
  add edi,eax	    ; calculate line begin adress
  add edi,ebx
  shl ebx,1
  add edi,ebx
  pop eax
  cld
 @ddraw:
  push eax
  stosw
  shr eax,16
  stosb
  pop eax
  loop @ddraw
  @end_hor_l:
ret




;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:


    mov  eax,12      ; function 12:tell os about windowdraw
    mov  ebx,1	    ; 1, start of draw
    int  0x40

       ; DRAW WINDOW
    mov  eax,0	    ; function 0 : define and draw window
    mov  ebx,120*65536+SIZE_X+20    ; [x start] *65536 + [x size]
    mov  ecx,120*65536+SIZE_Y+55    ; [y start] *65536 + [y size]
    mov  edx,0x34000000     ; color of work area RRGGBB,8->color gl
    mov  edi,win_title	; color of frames    RRGGBB
    int  0x40

      ; flag color button
    mov  eax,8	    ; function 8 : define and draw button
    mov  ebx,(SIZE_X-52)*65536+40    ; [x start] *65536 + [x size]
    mov  ecx,5*65536+12     ; [y start] *65536 + [y size]
    mov  edx,3	    ; button id
    mov  esi,0x888888	  ; button color RRGGBB
    int  0x40
       ; speed button
    sub  ebx,48*65536    ; [x start] *65536 + [x size]
    dec  edx    ; button id
    int  0x40
	
	mov  eax, 4
	mov  ebx, (SIZE_X-100+6)*65536+8
	mov  ecx, 0x80EEEeee
	mov  edx, btn_title
	int  0x40

    mov  eax,12      ; function 12:tell os about windowdraw
    mov  ebx,2	    ; 2, end of draw
    int  0x40

    ret


; DATA AREA
angle_counter dw 0
sq dd 0.707
xo dw 110 ;87
zo dw 0
yo dw 125
shad_flag db 0
speed_flag db 0

triangles:
dw 0,1,10, 10,11,1, 1,2,11, 11,12,2, 2,3,12, 12,13,3, 3,4,13, 13,14,4, 4,5,14
dw 14,15,5, 5,6,15, 15,16,6, 6,7,16, 16,17,7, 7,8,17, 17,18,8, 8,9,18, 18,19,9
dw 10,11,20, 20,21,11, 11,12,21, 21,22,12, 12,13,22, 22,23,13, 13,14,23
dw 23,24,14, 14,15,24, 24,25,15, 15,16,25, 25,26,16, 16,17,26, 26,27,17
dw 17,18,27, 27,28,18, 18,19,28, 28,29,19, 20,21,30, 30,31,21, 21,22,31
dw 31,32,22, 22,23,32, 32,33,23, 23,24,33, 33,34,24, 24,25,34, 34,35,25
dw 25,26,35, 35,36,26, 26,27,36, 36,37,27, 27,28,37, 37,38,28, 28,29,38
dw 38,39,29
 dd 0xffffffff ;<- end marker



win_title    db   '3d wavy rotaring area',0
btn_title db 'Color   Speed',0

sinbeta rd 1
cosbeta rd 1
singamma rd 1
cosgamma rd 1
current_angle rd 1

points rw points_count*3 + 2
points_rotated rw points_count*3 + 2
triangles_with_z rw triangles_count*4 + 2 ; triangles triple dw + z position
sorted_triangles rw triangles_count*3 + 2
screen rb SIZE_X * SIZE_Y * 3	; screen buffer
memStack rb 1000 ;memory area for stack
I_END:




