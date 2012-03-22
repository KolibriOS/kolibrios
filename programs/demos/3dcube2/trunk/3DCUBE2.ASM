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
include '..\..\..\macros.inc'
;purge mov
include 'ascl.inc'
include 'ascgl.inc'

START:
init_sin_cos:
	finit
	fldz
	xor edi,edi
	mov ecx,512
isc_make:
	fld st0
	fsincos
	fmul [fixed_point_const]
	fistp word [cos_table+edi]
	fmul [fixed_point_const]
	fistp word [sin_table+edi]

	fadd [inc_angle]

	add edi,2
	loop isc_make
	fstp st0
red:
    call draw_window

still:
    mov eax,11
    mcall

    dec eax
;    cmp  eax,1                  ; window redraw request ?
    jz   red
    dec eax
;    cmp  eax,2                  ; key in buffer ?
    jz   key
    dec eax
;    cmp  eax,3                  ; button in buffer ?
    jz   button

    fps  220,8,cl_White,cl_Black

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

key:
     mov eax,2
     mcall
     jmp still
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
    mcall 12, 1 ;Start window redraw

	mcall 48, 4
	
	lea	ecx, [100*65536+SCREEN_Y+4+eax]; [y start] *65536 + [y size] + [skin_height]
	xor	eax, eax   ;Draw window
    mov ebx,100*65536+(SCREEN_X+9)  ;x start*65536+x size
    mov edx,0x54000000         ;0x03 use skinned window
    mov edi,title
    mcall
	
    mcall 12, 2 ;End window redraw
	
    ret

title db '3D Cube Sample',0


; Draw faces procedure

draw_faces:

	mov esi,link
	mov ecx,MAX_TRIANGLES
df_draw:
	push ecx
	mov ecx,3
	mov edi,@@tx1 ;bp
df_get_point:
	movzx	ebx, byte [esi]
	movzx	eax, word [object_translated + ebx*4]
	stosd
	movzx	eax, word [object_translated + ebx*4 + 2]
	stosd
	inc esi
	dec ecx
	jnz df_get_point

	mov eax,[@@ty1]
	sub eax,[@@ty3]
	mov ebx,[@@tx2]
	sub ebx,[@@tx1]
	imul ebx
	push eax
	mov eax,[@@tx1]
	sub eax,[@@tx3]
	mov ebx,[@@ty2]
	sub ebx,[@@ty1]
	imul ebx
	pop ebx
	sub ebx,eax
	jge df_next

	movzx	eax, byte [esi]
	
	mov  	[@@xcol], eax

	call filled_triangle
df_next:
	inc esi
	pop ecx
	dec ecx
	jnz df_draw
	ret

;modify
;include graphlib.asm

clear_screen_buffer:

;outscrbuf
 mcall 48, 4
 mov ebx,scrbuf
 mov ecx,SCREEN_X*65536+SCREEN_Y
 lea edx,[5*65536+eax]
 mov eax,7
 mcall

;White background
 mov edi,scrbuf
 mov ecx,(SCREEN_X*SCREEN_Y*3)/4
 mov eax,0xfffffff0
 cld
 rep stosd

	ret

;include triangle.asm
; Mikolaj Felix 14/5/2001
; mfelix@polbox.com

;filled trangle procedure

align 4
@@tx1  dd 0
@@ty1  dd 0
@@tx2  dd 0
@@ty2  dd 0
@@tx3  dd 0
@@ty3  dd 0
@@xcol dd 0

@@dx12 dd 0
@@dx13 dd 0
@@dx23 dd 0

filled_triangle:

 mov eax,[@@xcol]  ;trnsforming color
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

	mov eax,[@@ty1]
	cmp eax,[@@ty3]
	jb ft_check1

	xchg eax,[@@ty3]
	mov [@@ty1],eax

	mov eax,[@@tx1]
	xchg eax,[@@tx3]
	mov [@@tx1],eax
ft_check1:
	mov eax,[@@ty2]
	cmp eax,[@@ty3]
	jb ft_check2

	xchg eax,[@@ty3]
	mov [@@ty2],eax

	mov eax,[@@tx2]
	xchg eax,[@@tx3]
	mov [@@tx2],eax
ft_check2:
	mov eax,[@@ty1]
	cmp eax,[@@ty2]
	jb ft_check3

	xchg eax,[@@ty2]
	mov [@@ty1],eax

	mov eax,[@@tx1]
	xchg eax,[@@tx2]
	mov [@@tx1],eax
ft_check3:

	mov ebx,[@@ty2]
	sub ebx,[@@ty1]
	jnz ft_dx12_make

	mov [@@dx12],dword 0
	jmp ft_dx12_done
ft_dx12_make:
	mov eax,[@@tx2]
	sub eax,[@@tx1]
	shl eax,7
	cdq
	idiv ebx
	mov [@@dx12],eax			; dx12 = (x2-x1)/(y2-y1)
ft_dx12_done:

	mov ebx,[@@ty3]
	sub ebx,[@@ty1]
	jnz ft_dx13_make

	mov [@@dx13],dword 0
	jmp ft_dx13_done
ft_dx13_make:
	mov eax,[@@tx3]
	sub eax,[@@tx1]
	shl eax,7
	cdq
	idiv ebx
	mov [@@dx13],eax			; dx13 = (x3-x1)/(y3-y1)
ft_dx13_done:

	mov ebx,[@@ty3]
	sub ebx,[@@ty2]
	jnz ft_dx23_make

	mov [@@dx23],dword 0
	jmp ft_dx23_done
ft_dx23_make:
	mov eax,[@@tx3]
	sub eax,[@@tx2]
	shl eax,7
	cdq
	idiv ebx
	mov [@@dx23],eax			; dx23 = (x3-x2)/(y3-y2)
ft_dx23_done:

	mov eax,[@@tx1]
	shl eax,7
	mov ebx,eax

	mov ecx,[@@ty1]
ft_loop1:

 pushad

	mov [@@ly],ecx
	mov edx,ebx
	shr edx,7
	mov [@@lx2],edx
	mov edx,eax
	shr edx,7
	mov [@@lx1],edx
	mov eax,[@@xcol]
 mov [@@lcol],eax
 call horizontal_line

 popad

	add eax,[@@dx13]
	add ebx,[@@dx12]
	inc ecx
	cmp ecx,[@@ty2]
	jb ft_loop1

	
	mov ebx,[@@tx2]
	shl ebx,7
	mov ecx,[@@ty2]
ft_loop2:

 pushad

	mov [@@ly],ecx
	mov edx,ebx
	shr edx,7
	mov [@@lx2],edx
	mov edx,eax
	shr edx,7
	mov [@@lx1],edx
 mov eax,[@@xcol]
 mov [@@lcol],eax
	call horizontal_line

 popad

	add eax,[@@dx13]
	add ebx,[@@dx23]
	inc ecx
	cmp ecx,[@@ty3]
	jb ft_loop2

	ret

;horizontal line subproc

align 4
@@lx1  dd 0
@@lx2  dd 0
@@ly   dd 0
@@lcol dd 0

@@rgb  dd 0

horizontal_line:

 mov ecx,[@@lx1]
 sub ecx,[@@lx2]
 ja  x12
 je  ext
; ret
 neg ecx
 mov edi,3
 jmp xx
x12:
 mov edi,-3
 jmp xx
ext:
 mov ecx,-1 ;1
; sub ebp,3
xx:
 mov eax,[@@ly]
 mov ebx,SCREEN_X ;320
 mul ebx
 add eax,[@@lx1]
 lea ebp,[eax*3-3]	; for delete white dots
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
	mov ebx, [esi]
	and ebx,511
	mov ax,word [sin_table+ebx*2]
	mov word [edi],ax
	mov ax,word [cos_table+ebx*2]
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
 mcall
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

	movzx ebx,word [esi+4]
 mov ax,[my]
 cmp ax,0
 jng no_m
 shl ax,3
 add bx,ax
no_m:
 add bx,256  ; Z factor (zoom)

 	movsx eax,word [esi]
	shl eax,8
	cdq
	idiv ebx
	add eax,(SCREEN_X/2) ;160         ;X factor (center X)
	stosw

	movsx eax,word [esi+2]
	shl eax,8
	cdq
	idiv ebx
	add eax,(SCREEN_Y/2) ;100         ;Y factor (center Y)
	stosw

	add esi,6
	dec ecx
	jnz translate_points
	ret

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