;
; 3D POLYGONAL CUBE - ASCL
;
; Pavlushin Evgeni
; mail: waptap@mail.ru       site: www.deck4.narod.ru
;
; Create on base 3D test sample
; Mikolaj Felix mfelix@polbox.com
;

use32
               org     0x0
               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x30000                  ; memory for app
               dd     0x30000                  ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

MAX_POINTS equ 8
MAX_TRIANGLES equ 12
SCREEN_X equ 320
SCREEN_Y equ 200

include 'lang.inc'
include 'ascl.inc'
include 'ascgl.inc'
include 'macros.inc'
START:
    call draw_window
   	call init_sin_cos

still:
;    mov eax,23  ; wait for system event with 10 ms timeout
;    mov ebx,1   ; wait 10 ms, then continue
;    int  0x40

    mov eax,11
    int 0x40

    dec eax
;    cmp  eax,1                  ; window redraw request ?
    jz   red
    dec eax
;    cmp  eax,2                  ; key in buffer ?
    jz   key
    dec eax
;    cmp  eax,3                  ; button in buffer ?
    jz   button

    fps  230,8,cl_White,cl_Black

main_loop:

	mov esi,object
	mov edi,object_rotated
	mov ecx,MAX_POINTS*3
	cld
	rep movsw

	mov esi,angle_x
	mov edi,object_rotated
	mov ecx,MAX_POINTS
	call rotate_points

	mov esi,object_rotated
	mov edi,object_translated
	mov ecx,MAX_POINTS
	call translate_points

	call draw_faces

	call clear_screen_buffer

	add [angle_x],2
	add [angle_y],3
	add [angle_z],1

     jmp still

red:
     call draw_window
     jmp still
key:
     mov eax,2
     int 0x40
     jmp still
button:
     mov eax,17
     int 0x40
     cmp ah,1
     jne still
exit:
     mov eax,-1
     int 0x40

;Draw window
draw_window:
    mov eax,12  ;Start
    mov ebx,1
    int 0x40

    mov eax,0   ;Draw window
    mov ebx,100*65536+(SCREEN_X+9) ;x start*65536+x size
    mov ecx,100*65536+(SCREEN_Y+26) ;y start*65536+y size
    mov edx,0x13000000         ;0x03 use skinned window
    mov edi,header
    int 0x40

    mov eax,12  ;End
    mov ebx,2
    int 0x40
    ret

header db '3D TEST SAMPLE FOR MENUETOS',0


; Draw faces procedure

draw_faces:

	mov esi,link
	mov ecx,MAX_TRIANGLES
df_draw:
	push ecx
	mov ecx,3
	mov edi,@@tx1 ;bp
df_get_point:
	xor bh,bh
	mov bl,byte [esi]
	shl bx,2
	mov ax,word [object_translated+bx]
	mov word [edi],ax
	mov ax,word [object_translated+bx+2]
	mov word [edi+2],ax
	inc esi
	add edi,4
	dec ecx
	jnz df_get_point

	mov ax,[@@ty1]
	sub ax,[@@ty3]
	mov bx,[@@tx2]
	sub bx,[@@tx1]
	imul bx
	shl edx,16
	mov dx,ax
	push edx
	mov ax,[@@tx1]
	sub ax,[@@tx3]
	mov bx,[@@ty2]
	sub bx,[@@ty1]
	imul bx
	shl edx,16
	mov dx,ax
	pop ebx
	sub ebx,edx
	or ebx,ebx
	jge df_next

	xor ah,ah
	mov al,byte [si]
	
 mov  [@@xcol],ax

	call filled_triangle
df_next:
	inc si
	pop ecx
	dec ecx
	jnz df_draw
	ret

;modify
;include graphlib.asm

clear_screen_buffer:

;outscrbuf
 mov ebx,scrbuf
 mov ecx,SCREEN_X*65536+SCREEN_Y
 mov edx,5*65536+22
 mov ax,7
 int 0x40

;White background
 mov edi,scrbuf
 mov ecx,(SCREEN_X*SCREEN_Y*3)/4
 mov eax,0xffffffff
 cld
 rep stosd

	ret

;include triangle.asm
; Mikolaj Felix 14/5/2001
; mfelix@polbox.com

;filled trangle procedure

@@tx1  dw 0
@@ty1  dw 0
@@tx2  dw 0
@@ty2  dw 0
@@tx3  dw 0
@@ty3  dw 0
@@xcol dw 0

@@dx12 dw 0
@@dx13 dw 0
@@dx23 dw 0

filled_triangle:

 mov ax,[@@xcol]  ;trnsforming color
 mov bl,al        ;byte bbbggrrx
 mov dl,al        ;to 3 byte
 mov dh,al        ;bbbxxxxx ggxxxxxx rrxxxxxx
 and dh,00000001b

 and al,11100000b
 and bl,00011000b
 and dl,00000110b
 shl bl,3
 shl dl,5

 cmp dh,1
 jne no_bitup
 or  al,00011111b
 or  bl,00111111b
 or  dl,00111111b
no_bitup:

 shl eax,8 ;puck colors
 mov al,bl
 shl eax,8
 mov al,dl
 mov dword [@@rgb],eax
 mov eax,0 ; for 16 bit instructions

	mov ax,[@@ty1]
	cmp ax,[@@ty3]
	jb ft_check1

	xchg ax,[@@ty3]
	mov [@@ty1],ax

	mov ax,[@@tx1]
	xchg ax,[@@tx3]
	mov [@@tx1],ax
ft_check1:
	mov ax,[@@ty2]
	cmp ax,[@@ty3]
	jb ft_check2

	xchg ax,[@@ty3]
	mov [@@ty2],ax

	mov ax,[@@tx2]
	xchg ax,[@@tx3]
	mov [@@tx2],ax
ft_check2:
	mov ax,[@@ty1]
	cmp ax,[@@ty2]
	jb ft_check3

	xchg ax,[@@ty2]
	mov [@@ty1],ax

	mov ax,[@@tx1]
	xchg ax,[@@tx2]
	mov [@@tx1],ax
ft_check3:

	mov bx,[@@ty2]
	sub bx,[@@ty1]
	jnz ft_dx12_make

	mov [@@dx12],word 0
	jmp ft_dx12_done
ft_dx12_make:
	mov ax,[@@tx2]
	sub ax,[@@tx1]
	shl ax,7
	cwd
	idiv bx
	mov [@@dx12],ax			; dx12 = (x2-x1)/(y2-y1)
ft_dx12_done:

	mov bx,[@@ty3]
	sub bx,[@@ty1]
	jnz ft_dx13_make

	mov [@@dx13],word 0
	jmp ft_dx13_done
ft_dx13_make:
	mov ax,[@@tx3]
	sub ax,[@@tx1]
	shl ax,7
	cwd
	idiv bx
	mov [@@dx13],ax			; dx13 = (x3-x1)/(y3-y1)
ft_dx13_done:

	mov bx,[@@ty3]
	sub bx,[@@ty2]
	jnz ft_dx23_make

	mov [@@dx23],word 0
	jmp ft_dx23_done
ft_dx23_make:
	mov ax,[@@tx3]
	sub ax,[@@tx2]
	shl ax,7
	cwd
	idiv bx
	mov [@@dx23],ax			; dx23 = (x3-x2)/(y3-y2)
ft_dx23_done:

	mov ax,[@@tx1]
	shl ax,7
	mov bx,ax

	mov cx,[@@ty1]
ft_loop1:

 pushad

	mov [@@ly],cx
	mov dx,bx
	shr dx,7
	mov [@@lx2],dx
	mov dx,ax
	shr dx,7
	mov [@@lx1],dx
	mov ax,[@@xcol]
 mov [@@lcol],ax
 call horizontal_line

 popad

	add ax,[@@dx13]
	add bx,[@@dx12]
	inc cx
	cmp cx,[@@ty2]
	jb ft_loop1

	
	mov bx,[@@tx2]
	shl bx,7
	mov cx,[@@ty2]
ft_loop2:

 pushad

	mov [@@ly],cx
	mov dx,bx
	shr dx,7
	mov [@@lx2],dx
	mov dx,ax
	shr dx,7
	mov [@@lx1],dx
 mov ax,[@@xcol]
 mov [@@lcol],ax
	call horizontal_line

 popad

	add ax,[@@dx13]
	add bx,[@@dx23]
	inc ecx
	cmp cx,[@@ty3]
	jb ft_loop2

	ret

;horizontal line subproc

@@lx1  dw 0
@@lx2  dw 0
@@ly   dw 0
@@lcol dw 0

@@rgb  dd 0

horizontal_line:

 mov ecx,0
 mov cx,[@@lx1]
 cmp cx,[@@lx2]
 ja  x12
 je  ext
; ret
 mov cx,[@@lx2]
 sub cx,[@@lx1]
 mov edi,3
 jmp xx
x12:
 mov cx,[@@lx1]
 sub cx,[@@lx2]
 mov edi,-3
 jmp xx
ext:
 mov ecx,-1 ;1
; sub ebp,3
xx:
 mov eax,0
 mov ax,[@@ly]
 mov ebx,SCREEN_X ;320
 mul ebx
 mov ebp,0
 mov bp,[@@lx1] ;for correct 16 bit size
 add eax,ebp
 mov ebx,3
 mul ebx
 mov ebp,eax
 sub ebp,3   ;for delete  white dots
 add ecx,2
loo:

 mov eax,dword [@@rgb]
 mov bl,al
 shr eax,8 ;puck colors

 mov byte [scrbuf+ebp],ah
 mov byte [scrbuf+ebp+1],al
 mov byte [scrbuf+ebp+2],bl
 add ebp,edi
 dec ecx
 jnz loo

	ret

;include fixed3d.asm
; Mikolaj Felix 25/5/2001
; mfelix@polbox.com

;------------------------------------------------------------
;	ds:si	- offset to angles
;	ds:di	- offset to 3d points
;	cx	- number of points
;------------------------------------------------------------

@@sin_x dw 0
@@cos_x dw 0
@@sin_y dw 0
@@cos_y dw 0
@@sin_z dw 0
@@cos_z dw 0

@@px equ word [edi]
@@py equ word [edi+2]
@@pz equ word [edi+4]

rotate_points:

 push edi
	mov edi,@@sin_x
	mov edx,3
rp_sin_cos:
	mov bx,word [esi]
	and bx,511
	shl bx,1
	mov ax,word [sin_table+bx]
	mov word [edi],ax
	mov ax,word [cos_table+bx]
	mov word [edi+2],ax

	add esi,2
	add edi,4
	dec edx
	jnz rp_sin_cos
	pop edi

rp_rotate:

	; rotate around x-axis

	mov ax,@@py
	imul [@@cos_x]
	mov bx,ax
	mov si,dx

	mov ax,@@pz
	imul [@@sin_x]
	sub bx,ax
	sbb si,dx
	shrd bx,si,14
	push bx

	mov ax,@@py
	imul [@@sin_x]
	mov bx,ax
	mov si,dx

	mov ax,@@pz
	imul [@@cos_x]
	add bx,ax
	adc si,dx
	shrd bx,si,14

	pop @@py
	mov @@pz,bx

	; rotate around y-axis

	mov ax,@@px
	imul [@@cos_y]
	mov bx,ax
	mov si,dx

	mov ax,@@pz
	imul [@@sin_y]
	sub bx,ax
	sbb si,dx
	shrd bx,si,14
	push bx

	mov ax,@@px
	imul [@@sin_y]
	mov bx,ax
	mov si,dx

	mov ax,@@pz
	imul [@@cos_y]
	add bx,ax
	adc si,dx
	shrd bx,si,14

	pop @@px
	mov @@pz,bx

	; rotate around z-axis

	mov ax,@@px
	imul [@@cos_z]
	mov bx,ax
	mov si,dx

	mov ax,@@py
	imul [@@sin_z]
	sub bx,ax
	sbb si,dx
	shrd bx,si,14
	push bx

	mov ax,@@px
	imul [@@sin_z]
	mov bx,ax
	mov si,dx

	mov ax,@@py
	imul [@@cos_z]
	add bx,ax
	adc si,dx
	shrd bx,si,14

	pop @@px
	mov @@py,bx

	add edi,6
	dec ecx
	jnz rp_rotate

	ret

;------------------------------------------------------------
;	ds:si	- offset to 3d points
;	es:di	- offset to 2d points
;	cx	- number of points
;------------------------------------------------------------

 mx dw 0
 my dw 0

translate_points:
 pushad
 mov eax,37
 mov ebx,1
 int 0x40
 mov ebx,eax
 shr eax,16
 and ebx,0xffff
 cmp ax,SCREEN_X
 jna x_n
 mov ax,0 ;SCREEN_X
x_n:
 cmp bx,SCREEN_Y
 jna y_n
 mov bx,0 ;SCREEN_Y
y_n:
 mov [mx],ax
 mov [my],bx
 popad

 mov ebx,0  ;?
	mov bx,word [esi+4]
 mov ax,[my]
 cmp ax,0
 jng no_m
 shl ax,3
 add bx,ax
no_m:
 add bx,256  ; Z factor (zoom)

 mov eax,0  ;?
	mov ax,word [esi]
	shl ax,8
	cwd
	idiv bx; bx
	add ax,(SCREEN_X/2) ;160         ;X factor (center X)
	stosw

 mov eax,0  ;?
	mov ax,word [esi+2]
	shl ax,8
	cwd
	idiv bx
	add ax,(SCREEN_Y/2) ;100         ;Y factor (center Y)
	stosw

	add esi,6
	dec ecx
	jnz translate_points
	ret

init_sin_cos:
	finit
	fldz
	fstp [temp]
	xor edi,edi
	mov ecx,512
isc_make:
	fld [temp]
	fld st0
	fld st0
	fsin
	fmul [fixed_point_const]
	fistp word [sin_table+edi]
	fcos
	fmul [fixed_point_const]
	fistp word [cos_table+edi]

	fadd [inc_angle]
	fstp [temp]

	add edi,2
	loop isc_make
	ret

temp dd 0

fixed_point_const dd 16384.0
inc_angle dd 0.01227184630309			; pi/256

angle_x dw 0
angle_y dw 0
angle_z dw 0

object	dw -50,-50,-50, 50,-50,-50, 50,50,-50, -50,50,-50
      	dw -50,-50, 50, 50,-50, 50, 50,50, 50, -50,50, 50

link:
 db  0,1,2,10000011b, 0,2,3,10000011b ;purpure   side
 db  5,4,7,00000111b, 5,7,6,00000111b ;soft-red  side
 db  1,5,6,00011000b, 1,6,2,00011000b ;soft-lime side
 db  4,0,3,11100001b, 4,3,7,11100001b ;soft-blue side
 db  4,5,1,00011111b, 1,0,4,00011111b ;yellow    side
 db  3,2,6,00000000b, 3,6,7,00000000b ;black     side

sin_table:
rw 512
cos_table:
rw 512

object_rotated:
rw MAX_POINTS*3
object_translated:
rw MAX_POINTS*2

scrbuf:
I_END: