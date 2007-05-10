;
; 9 Ver Screen saver
; 5 Ver 24 bit texture.
; 23/8/2004
; Pavlushin Evgeni 3d cube screen saver
; mail: waptap@mail.ru       site: www.deck4.narod.ru
;
; This programm develop on sample text3d to Mikolaj Felix 25/5/2001
; mfelix@polbox.com
;

use32
               org     0x0
               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x200000                ; memory for app
               dd     0x200000                ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

include 'lang.inc'
include '..\..\..\macros.inc'
MAX_DEGS equ 512 ;quantity of angels 2^n.Minimize for speedup
MAX_POINTS equ 8 ;quantity of points
MAX_FACES equ 6  ;quantity of points

START:
    finit
    call draw_window
    call clear_screen
    call init_sincos

still:
    mov eax,23  ; wait for system event with 2 ms timeout
    mov ebx,2   ; wait 2 ms, then continue
    mcall

;    mov eax,11 ;If you want maximum speed! :)
;    mcall

    dec  eax                  ; window redraw request ?
    je   red
    dec  eax                  ; key in buffer ?
    je   key
    dec  eax                  ; button in buffer ?
    je   button

main_loop:
 call fade_texture

	mov esi,cube
	mov edi,cube_rotated
	mov ecx,MAX_POINTS*3
copy_object:
 fild word [esi]
 fstp dword [edi]
 add esi,2
 add edi,4
 dec ecx
 jnz copy_object

 mov esi,angle_x
 mov edi,cube_rotated
 mov ecx,MAX_POINTS
 call rotate_points
	
 mov esi,cube_rotated
 mov edi,coord2d
 mov ecx,MAX_POINTS
 call translate_points

 call draw_textured_faces
 call clear_screen_buffer

 add [angle_x],1
 add [angle_y],3
 add [angle_z],1

 jmp still

red:
     call draw_window
     jmp still
key:
     mov eax,2
     jmp exit
button:
     mov eax,17
     mcall
     cmp ah,1
     jne still
exit:
     mov eax,-1
     mcall

;Draw window
draw_window:
    mov eax,12  ;Start
    mov ebx,1
    mcall

    mov eax,0   ;Draw window
    mov ebx,0*65536+(799) ;x start*65536+x size
    mov ecx,0*65536+(599) ;y start*65536+y size
    mov edx,0x00000000         ;0x03 use skinned window
    mcall

    call clear_screen

    mov eax,12  ;End
    mov ebx,2
    mcall
    ret

head_label: db "3D TEXTURED CUBE THE LITTLE SCREEN SAVER FOR MENUETOS. USE "
            db "800x600 SCREEN MODE FROM VIEW. PRESS ANY KEY FOR EXIT"
hl_end:

;FADE IN FADE OUT TEXTURE

x_step db 0
x_num  db 1

fade_texture:
 mov ecx,0
loox:
 mov al,[file_texture+ecx]
 mov [texture+ecx],al
 inc ecx
 cmp ecx,128*128*3
 jnae loox

 mov ecx,0
loox2:
 mov al,[file_texture+ecx]
 cmp [x_step],al
 jae xxx
 sub al,[x_step]
 jmp nnn
xxx:
 mov al,0
nnn:
 mov [texture+ecx],al
 inc ecx
 cmp ecx,128*128*3
 jnae loox2

 cmp [x_step],255
 jne no_max
 mov [x_num],-1
no_max:
 cmp [x_step],0
 jne no_min
 mov [x_num],1
no_min:
 mov al,[x_num]
 add [x_step],al
 ret

; Clear Screen
clear_screen:
 mov eax,13
 mov ebx,0*65536+800
 mov ecx,0*65536+600
 mov edx,0
 mcall

 mov eax,4   ;Out Text
 mov ebx,8*65536+8          ;x start*65536+y start
 mov ecx,0x00ffffff         ;color White
 mov edx,head_label
 mov esi,hl_end-head_label
 mcall
 ret

clear_screen_buffer:
 mov ebx,scrbuf
 mov ecx,800*65536+(600-40)  ;sub 40 for antiflickering title
 mov edx,0*65536+40
 mov eax,7
 mcall

 mov eax,4   ;Out Text
 mov ebx,8*65536+580          ;x start*65536+y start
 mov ebp,[n_step]
 shl ebp,16
 sub ebx,ebp
 mov ecx,0x0000ff00         ;color White
 mov edx,move_text
 add edx,[step]
 mov esi,130 ;mt_end-move_text
 mcall

 inc [n_step]
 cmp [n_step],6
 jna t_ok
 mov [n_step],0
 inc [step]
 cmp [step],mt_end-move_text-130
 jng t_ok
 mov [step],0
t_ok:

 mov edi,scrbuf
 mov eax,0  ;black background
 mov ecx,800*600*3/4 ; 16000
 cld
 rep stosd
 ret

n_step dd 0
step dd 0

move_text: db "                                            "
           db "                                            "
           db "                                            "
           db "***** 3D TEXTURED CUBE THE LITTLE SCREEN SAVER FOR "
           db "MENUET OS. SET 800x600 SCREEN MODE FROM VIEW THIS SCREEN "
           db "SAVER *****                                        "
           db "SITE OF THE RUSSIAN DEVELOPERS TO MENUETOS: "
           db "www.menuet.narod.ru                         "
           db "RUSSIAN MENUET APLICATION ARCHIVE PAGE ON: "
           db "www.meosfiles.narod.ru                      "
           db "AUTOR OF THIS SCREEN SAVER Pavlushin Evgeni "
           db "MY SITE: www.deck4.narod.ru (Slow update)   "
           db "AND MAIL BOX: waptap@mail.ru                "
           db "THANK YOU FOR USE!                          "
           db "                                            "
           db "                                            "
           db "                                            "
mt_end:


;include graphlib.asm Mikolaj Felix 9/12/2000 mfelix@polbox.com
;Draw textured faces proc

@@rx1 dw 0 ;equ [bp-2]
@@ry1 dw 0 ;equ [bp-4]
@@rx2 dw 0 ;equ [bp-6]
@@ry2 dw 0 ;equ [bp-8]
@@rx3 dw 0 ;equ [bp-10]
@@ry3 dw 0 ;equ [bp-12]
@@rx4 dw 0 ;equ [bp-14]
@@ry4 dw 0 ;equ [bp-16]

draw_textured_faces:
	
 mov esi,link
 mov ecx,MAX_FACES
dtf_loop:
 push ecx

 xor ebx,ebx
 mov bl,byte [esi]		; point 1
 shl bx,2
 mov eax,dword [coord2d+bx] ;COPY 1 FOURANGLE POINT
 mov dword [@@rx1],eax

 xor ebx,ebx
 mov bl,byte [esi+1]		; point 2
 shl bx,2
 mov eax,dword [coord2d+bx] ;COPY 2 FOURANGLE POINT
 mov dword [@@rx2],eax

 xor ebx,ebx
 mov bl,byte [esi+2]		; point 3
 shl bx,2
 mov eax,dword [coord2d+bx] ;COPY 3 FOURANGLE POINT
 mov dword [@@rx3],eax

 xor bh,bh			; point 4
 mov bl,byte [esi+3]
 shl bx,2
 mov eax,dword [coord2d+bx] ;COPY 4 FOURANGLE POINT
 mov dword [@@rx4],eax

 mov ax,[@@ry1]
 sub ax,[@@ry3]
 mov bx,[@@rx2]
 sub bx,[@@rx1]
 imul bx
 shl edx,16
 mov dx,ax
 push edx
 mov ax,[@@rx1]
 sub ax,[@@rx3]
 mov bx,[@@ry2]
 sub bx,[@@ry1]
 imul bx
 shl edx,16
 mov dx,ax
 pop ebx
 sub ebx,edx			; normal_z = (y1-y3)*(x2-x1)-(x1-x3)*(y2-y1)
 or ebx,ebx
 jl dtf_next_face		; normal_z < 0	

 ; FIRST PICE OF FOUR ANGLE

 ; Set 3 triangel puts for texture ycoord*65536+xcoord
 mov dword [@@tex_x3],127*65536+127 ;3 point
 mov dword [@@tex_x2],0*65536+127   ;2 point
 mov dword [@@tex_x1],0*65536+0     ;1 point
 ; Set texture bitmap offset
 mov [@@tex_off],texture
 ; Set 3 triangel puts coordinates
 mov eax,dword [@@rx3]
 mov dword [@@x3],eax
 mov eax,dword [@@rx2]
 mov dword [@@x2],eax
 mov eax,dword [@@rx1]
 mov dword [@@x1],eax
 call textured_triangle

 ; SECOND PICE OF FOUR ANGLE

 ; Set 3 triangel puts for texture ycoord*65536+xcoord
 mov dword [@@tex_x3],127*65536+0   ;3 point
 mov dword [@@tex_x2],127*65536+127 ;2 point
 mov dword [@@tex_x1],0*65536+0     ;1 point
 ; Set texture bitmap offset
 mov [@@tex_off],texture
 ; Set 3 triangel puts coordinates
 mov eax,dword [@@rx4]
 mov dword [@@x3],eax
 mov eax,dword [@@rx3]
 mov dword [@@x2],eax
 mov eax,dword [@@rx1]
 mov dword [@@x1],eax
 call textured_triangle

dtf_next_face:
 add esi,4
 pop ecx
 dec ecx
 jnz dtf_loop
 ret

;include tex3.asm Mikolaj Felix 15/5/2001 mfelix@polbox.com

@@x1 dw 0 ;equ [bp+4]
@@y1 dw 0 ;equ [bp+6]
@@x2 dw 0 ;equ [bp+8]
@@y2 dw 0 ;equ [bp+10]
@@x3 dw 0 ;equ [bp+12]
@@y3 dw 0 ;equ [bp+14]

@@tex_off dd 0 ;equ [bp+16]
@@tex_x1 dw 0 ;equ [bp+18]
@@tex_y1 dw 0 ;equ [bp+20]
@@tex_x2 dw 0 ;equ [bp+22]
@@tex_y2 dw 0 ;equ [bp+24]
@@tex_x3 dw 0 ;equ [bp+26]
@@tex_y3 dw 0 ;equ [bp+28]

@@dx12 dw 0 ;equ [bp-2]
@@dx13 dw 0 ;equ [bp-4]
@@dx23 dw 0 ;equ [bp-6]

@@tex_dx12 dw 0 ;equ [bp-8]
@@tex_dy12 dw 0 ;equ [bp-10]
@@tex_dx13 dw 0 ;equ [bp-12]
@@tex_dy13 dw 0 ;equ [bp-14]
@@tex_dx23 dw 0 ;equ [bp-16]
@@tex_dy23 dw 0 ;equ [bp-18]

@@scan_x1 dw 0 ;equ [bp-20]
@@scan_y1 dw 0 ;equ [bp-22]
@@scan_x2 dw 0 ;equ [bp-24]
@@scan_y2 dw 0 ;equ [bp-26]


textured_triangle:

	mov ax,[@@y1]
	cmp ax,[@@y3]
	jb tt_check1

	xchg ax,[@@y3]
	mov [@@y1],ax

	mov ax,[@@x1]
	xchg ax,[@@x3]
	mov [@@x1],ax

	mov ax,[@@tex_y1]
	xchg ax,[@@tex_y3]
	mov [@@tex_y1],ax

	mov ax,[@@tex_x1]
	xchg ax,[@@tex_x3]
	mov [@@tex_x1],ax
tt_check1:
	mov ax,[@@y2]
	cmp ax,[@@y3]
	jb tt_check2

	xchg ax,[@@y3]
	mov [@@y2],ax

	mov ax,[@@x2]
	xchg ax,[@@x3]
	mov [@@x2],ax

	mov ax,[@@tex_y2]
	xchg ax,[@@tex_y3]
	mov [@@tex_y2],ax

	mov ax,[@@tex_x2]
	xchg ax,[@@tex_x3]
	mov [@@tex_x2],ax
tt_check2:
	mov ax,[@@y1]
	cmp ax,[@@y2]
	jb tt_check3

	xchg ax,[@@y2]
	mov [@@y1],ax

	mov ax,[@@x1]
	xchg ax,[@@x2]
	mov [@@x1],ax

	mov ax,[@@tex_y1]
	xchg ax,[@@tex_y2]
	mov [@@tex_y1],ax

	mov ax,[@@tex_x1]
	xchg ax,[@@tex_x2]
	mov [@@tex_x1],ax
tt_check3:

	mov bx,[@@y2]
	sub bx,[@@y1]
	jnz tt_dx12_make

	mov word [@@dx12],0
	mov word [@@tex_dx12],0
	mov word [@@tex_dy12],0
	jmp tt_dx12_done
tt_dx12_make:
	mov ax,[@@x2]
	sub ax,[@@x1]
	shl ax,7
	cwd
	idiv bx
	mov [@@dx12],ax			; dx12 = (x2-x1)/(y2-y1)

	mov ax,[@@tex_x2]
	sub ax,[@@tex_x1]
	shl ax,7
	cwd
	idiv bx
	mov [@@tex_dx12],ax		; tex_dx12 = (tex_x2-tex_x1)/(y2-y1)

	mov ax,[@@tex_y2]
	sub ax,[@@tex_y1]
	shl ax,7
	cwd
	idiv bx
	mov [@@tex_dy12],ax		; tex_dy12 = (tex_y2-tex_y1)/(y2-y1)
tt_dx12_done:

	mov bx,[@@y3]
	sub bx,[@@y1]
	jnz tt_dx13_make

	mov word [@@dx13],0
	mov word [@@tex_dx13],0
	mov word [@@tex_dy13],0
	jmp tt_dx13_done
tt_dx13_make:
	mov ax,[@@x3]
	sub ax,[@@x1]
	shl ax,7
	cwd
	idiv bx
	mov [@@dx13],ax			; dx13 = (x3-x1)/(y3-y1)

	mov ax,[@@tex_x3]
	sub ax,[@@tex_x1]
	shl ax,7
	cwd
	idiv bx
	mov [@@tex_dx13],ax		; tex_dx13 = (tex_x3-tex_x1)/(y3-y1)

	mov ax,[@@tex_y3]
	sub ax,[@@tex_y1]
	shl ax,7
	cwd
	idiv bx
	mov [@@tex_dy13],ax		; tex_dy13 = (tex_y3-tex_x1)/(y3-y1)
tt_dx13_done:

	mov bx,[@@y3]
	sub bx,[@@y2]
	jnz tt_dx23_make

	mov word [@@dx23],0
	mov word [@@tex_dx23],0
	mov word [@@tex_dy23],0
	jmp tt_dx23_done
tt_dx23_make:
	mov ax,[@@x3]
	sub ax,[@@x2]
	shl ax,7
	cwd
	idiv bx
	mov [@@dx23],ax			; dx23 = (x3-x2)/(y3-y2)

	mov ax,[@@tex_x3]
	sub ax,[@@tex_x2]
	shl ax,7
	cwd
	idiv bx
	mov [@@tex_dx23],ax		; tex_dx23 = (tex_x3-tex_x2)/(y3-y2)

	mov ax,[@@tex_y3]
	sub ax,[@@tex_y2]
	shl ax,7
	cwd
	idiv bx
	mov [@@tex_dy23],ax		; tex_dy23 = (tex_y3-tex_y2)/(y3-y2)
tt_dx23_done:


	mov ax,[@@x1]
	shl ax,7
	mov bx,ax
	mov cx,[@@y1]

	mov dx,[@@tex_x1]
	shl dx,7
	mov [@@scan_x1],dx
	mov [@@scan_x2],dx
	mov dx,[@@tex_y1]
	shl dx,7
	mov [@@scan_y1],dx
	mov [@@scan_y2],dx
tt_loop1:
 pushad
;	push ax
;	push bx
;	push cx

	mov dx,[@@scan_y2]
	shr dx,7
	mov [@@tex_ly2],dx ;push dx
	mov dx,[@@scan_x2]
	shr dx,7
	mov [@@tex_lx2],dx ;push dx
	mov dx,[@@scan_y1]
	shr dx,7
	mov [@@tex_ly1],dx ;push dx
	mov dx,[@@scan_x1]
	shr dx,7
	mov [@@tex_lx1],dx ;push dx
	mov ebp,dword [@@tex_off]  ;push word ptr @@tex_off
 mov [@@tex_loff],ebp

	mov [@@ly],cx ;push cx
	mov dx,bx
	shr dx,7
	mov [@@lx2],dx ;push dx
	mov dx,ax
	shr dx,7
	mov [@@lx1],dx ; push dx
	call textured_horizontal_line

;	pop cx
;	pop bx
;	pop ax
 popad

	mov dx,[@@tex_dx13]
	add [@@scan_x1],dx
	mov dx,[@@tex_dx12]
	add [@@scan_x2],dx
	mov dx,[@@tex_dy13]
	add [@@scan_y1],dx
	mov dx,[@@tex_dy12]
	add [@@scan_y2],dx

	add ax,[@@dx13]
	add bx,[@@dx12]
	inc cx
	cmp cx,[@@y2]
	jb tt_loop1

	mov bx,[@@x2]
	shl bx,7
	mov cx,[@@y2]

	mov dx,[@@tex_x2]
	shl dx,7
	mov [@@scan_x2],dx
	mov dx,[@@tex_y2]
	shl dx,7
	mov [@@scan_y2],dx
tt_loop2:

 pushad
;	push ax
;	push bx
;	push cx

	mov dx,[@@scan_y2]
	shr dx,7
	mov [@@tex_ly2],dx ;push dx
	mov dx,[@@scan_x2]
	shr dx,7
	mov [@@tex_lx2],dx ;push dx
	mov dx,[@@scan_y1]
	shr dx,7
	mov [@@tex_ly1],dx ;push dx
	mov dx,[@@scan_x1]
	shr dx,7
	mov [@@tex_lx1],dx ;push dx
	mov ebp,dword [@@tex_off]  ;push word ptr @@tex_off
 mov [@@tex_loff],ebp

	mov [@@ly],cx ;push cx
	mov dx,bx
	shr dx,7
	mov [@@lx2],dx ;push dx
	mov dx,ax
	shr dx,7
	mov [@@lx1],dx ; push dx
	call textured_horizontal_line

;	pop cx
;	pop bx
;	pop ax
 popad

	mov dx,[@@tex_dx13]
	add [@@scan_x1],dx
	mov dx,[@@tex_dx23]
	add [@@scan_x2],dx
	mov dx,[@@tex_dy13]
	add [@@scan_y1],dx
	mov dx,[@@tex_dy23]
	add [@@scan_y2],dx

	add ax,[@@dx13]
	add bx,[@@dx23]
	inc cx
	cmp cx,[@@y3]
	jb tt_loop2
	ret

@@lx1 dw 0 ;equ [bp+4]
@@lx2 dw 0 ;equ [bp+6]
@@ly dw 0 ;equ [bp+8]

@@tex_loff dd 0 ;equ [bp+10]
@@tex_lx1 dw 0 ;equ [bp+12]
@@tex_ly1 dw 0 ;equ [bp+14]
@@tex_lx2 dw 0 ;equ [bp+16]
@@tex_ly2 dw 0 ;equ [bp+18]

@@tex_ldx dw 0 ;equ [bp-2]
@@tex_ldy dw 0 ;equ [bp-4]

textured_horizontal_line:

	mov ax,[@@lx1]
	cmp ax,[@@lx2]
	je thl_quit
	jb thl_ok

	xchg ax,[@@lx2]
	mov [@@lx1],ax

	mov ax,[@@tex_lx1]
	xchg ax,[@@tex_lx2]
	mov [@@tex_lx1],ax

	mov ax,[@@tex_ly1]
	xchg ax,[@@tex_ly2]
	mov [@@tex_ly1],ax
thl_ok:

 ; Fast method
; mov edi,0
;	mov di,[@@ly]  ;edi = calculating start of line
;	mov ax,di
;	shl di,6  ;ly*64
;	shl ax,8  ;ly*256
;	add di,ax    ;di = (ly*64)+(ly*256)
;	add di,[@@lx1] ;di = ly*320+lx1
; mov eax,edi
; shl eax,1
; add edi,eax
; add edi,scrbuf

 ; Uneversal method
 movsx edi,[@@ly]
 mov eax,800   ;di = ly*320+lx1
 mul edi
 movsx ebx,[@@lx1]
 add eax,ebx ;[@@lx1]
 mov edi,3
 mul edi
 mov edi,eax
 add edi,scrbuf

	mov cx,[@@lx2]
	sub cx,[@@lx1]

	mov ax,[@@tex_lx2]
	sub ax,[@@tex_lx1]
	shl ax,7
	cwd
	idiv cx
	mov [@@tex_ldx],ax			; tex_dx = (tex_x2-tex_x1)/(x2-x1)

	mov ax,[@@tex_ly2]
	sub ax,[@@tex_ly1]
	shl ax,7
	cwd
	idiv cx
	mov [@@tex_ldy],ax			; tex_dy = (tex_y2-tex_y1)/(x2-x1)

	cld
	inc cx
	mov ax,[@@tex_lx1]
	shl ax,7
	mov bx,[@@tex_ly1]
	shl bx,7

thl_loop:
	mov dx,ax
	push bx

	and bx,0ff80h
	shr ax,7
	add bx,ax
 mov ebp,0
 mov bp,bx
 mov eax,ebp
 shl eax,1
 add ebp,eax
	add ebp,[@@tex_loff]

	mov al,byte [ebp+2]
	stosb
	mov al,byte [ebp+1]
	stosb
	mov al,byte [ebp]
	stosb

	pop bx
	mov ax,dx

	add ax,[@@tex_ldx]
	add bx,[@@tex_ldy]
	dec cx
	jnz thl_loop
thl_quit:
	ret

;include math3d_2.asm
; Mikolaj Felix 20/06/2001
; mfelix@polbox.com

;------------------------------------------------------------
;	ds:si	- offset to angles (int)
;	ds:di	- offset to array of 3d points
;	cx	- number of points
;------------------------------------------------------------

@@sin_x dd 0 ;equ dword ptr [bp-4]
@@cos_x dd 0 ;equ dword ptr [bp-8]
@@sin_y dd 0 ;equ dword ptr [bp-12]
@@cos_y dd 0 ;equ dword ptr [bp-16]
@@sin_z dd 0 ;equ dword ptr [bp-20]
@@cos_z dd 0 ;equ dword ptr [bp-24]

@@x equ dword [edi]
@@y equ dword [edi+4]
@@z equ dword [edi+8]

rotate_points:

	push edi
	mov edi,@@sin_x
	mov edx,3
rp_sin_cos:
	mov bx,word [esi]
	and bx,MAX_DEGS-1
	shl bx,2
	mov eax,dword [sintab+bx] 	
	mov dword [edi],eax
	mov eax,dword [costab+bx]
	mov dword [edi+4],eax
	add edi,8
	add esi,2
	dec edx
	jnz rp_sin_cos
	pop edi

rp_rotate:
	fld @@y
	fmul [@@cos_x]
	fld @@z
	fmul [@@sin_x]
	fsubp st1,st
	fld @@y
	fxch st1
	fstp @@y			; Yrotated = Cos (A)*Y - Sin (A)*Z
	fmul [@@sin_x]
	fld @@z
	fmul [@@cos_x]
	faddp st1,st
	fstp @@z			; Zrotated = Sin (A)*Y + Cos (A)*Z
	
	fld @@x
	fmul [@@cos_y]
	fld @@z
	fmul [@@sin_y]
	fsubp st1,st
	fld @@x
	fxch st1
	fstp @@x			; Xrotated = Cos (A)*X - Sin (A)*Z
	fmul [@@sin_y]
	fld @@z
	fmul [@@cos_y]
	faddp st1,st
	fstp @@z			; Zrotated = Sin (A)*X + Cos (A)*Z
	
	fld @@x
	fmul [@@cos_z]
	fld @@y
	fmul [@@sin_z]
	fsubp st1,st
	fld @@x
	fxch st1
	fstp @@x			; Xrotated = Cos (A)*X - Sin (A)*Y
	fmul [@@sin_z]
	fld @@y
	fmul [@@cos_z]
	faddp st1,st
	fstp @@y			; Yrotated = Sin (A)*X + Cos (A)*Y

	add edi,12
	dec ecx
	jnz rp_rotate
	ret
					
;------------------------------------------------------------
;	ds:si	- offset to array of 3d points
;	ds:di	- offset to 2d points
;	cx	- number of points
;------------------------------------------------------------

translate_points:

	fld dword [esi+8]
	fadd [perspective]

	fld dword [esi]
	fmul [perspective]
	fdiv st,st1
	fadd [correct_x]
	fistp word [edi]		; x2d = (x*persp)/(z+persp)+correct_x

	fld dword [esi+4]
	fmul [perspective]
	fdivrp st1,st
	fadd [correct_y]
	fistp word [edi+2]		; y2d = (y*persp)/(z+persp)+correct_y

	add esi,12
	add edi,4
 dec ecx
 jnz translate_points
 ret

; initalize SIN&COS table

@@temp1 dd 0 ;dword ptr [bp-4]
@@temp2 dd 0 ;dword ptr [bp-8]

init_sincos:
	mov [@@temp1],0
	fldpi
	mov [@@temp2],MAX_DEGS/2
	fidiv [@@temp2]
	fstp [@@temp2]

	xor edi,edi
	mov ecx,MAX_DEGS
isc_loop:
	fld [@@temp1]
	fld st0
	fld st0
	fsin
	fstp dword [sintab+edi]
	fcos
	fstp dword [costab+edi]
	fadd [@@temp2]
	fstp [@@temp1]

	add edi,4
	dec ecx
	jnz isc_loop
	ret

perspective dd 256.0
correct_x dd 400.0
correct_y dd 300.0

angle_x dw 0
angle_y dw 0
angle_z dw 0

file_texture:
file 'texture.raw'

cube	dw -50,-50,50, 50,-50,50, 50,50,50, -50,50,50	
    	dw -50,-50,-50, 50,-50,-50, 50,50,-50, -50,50,-50

link db 0,1,2,3, 5,4,7,6, 1,5,6,2, 4,0,3,7, 4,5,1,0, 3,2,6,7

sintab:
rd MAX_DEGS
costab:
rd MAX_DEGS

cube_rotated:
rd MAX_POINTS*3
coord2d:
rw MAX_POINTS*2
texture:
rb 128*128*3
scrbuf:

I_END: