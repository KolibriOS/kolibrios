;
; TRIANGLE SPEED TEST 3
; 32 triangle draw use! 12300 triangle in sec on 800Mhz processor
;
; Pavlushin Evgeni 11.09.2004
; mail: waptap@mail.ru       site: www.deck4.narod.ru
;                                www.cyberdeck.fatal.ru

use32
               org     0x0
               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x100000                ; memory for app
               dd     0x100000                ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

SCREEN_X equ 320 ;800
SCREEN_Y equ 200 ;600

include 'lang.inc'
include 'ascl.inc'
include 'ascgl.inc'

START:
red:
    call draw_window

still:
    scevent red,key,button
    fps  290,8,cl_White,cl_Black

main_loop:
     random SCREEN_X,eax
     mov [@@tx1],eax
     random SCREEN_Y,eax
     mov [@@ty1],eax
     random SCREEN_X,eax
     mov [@@tx2],eax
     random SCREEN_Y,eax
     mov [@@ty2],eax
     random SCREEN_X,eax
     mov [@@tx3],eax
     random SCREEN_Y,eax
     mov [@@ty3],eax

     random 255,eax
     mov byte [@@rgb],al
     random 255,eax
     mov byte [@@rgb+1],al
     random 255,eax
     mov byte [@@rgb+2],al
     pushad
     call filled_triangle
     popad

     dec [count]  ;for max speed
     jnz xxx
     call outscr
     mov [count],100
xxx:
     jmp still

count dd 100

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

    xor eax,eax   ;Draw window
    mov ebx,100*65536+(SCREEN_X+19) ;x start*65536+x size
    mov ecx,100*65536+(SCREEN_Y+51) ;y start*65536+y size
    mov edx,0x33000000              ;0x33 use skinned window
    mov edi,header
    int 0x40

    mov eax,12  ;End
    mov ebx,2
    int 0x40
    ret

header db '3D TEST SAMPLE',0

outscr:

;outscrbuf
 mov ebx,scrbuf
 mov ecx,SCREEN_X*65536+SCREEN_Y
 mov edx,5*65536+22
 mov ax,7
 int 0x40

	ret

;filled trangle 32 bit draw procedure
;from NAAG3d demo

@@tx1  dd 0
@@ty1  dd 0
@@tx2  dd 0
@@ty2  dd 0
@@tx3  dd 0
@@ty3  dd 0
@@rgb  dd 0

@@dx12 dd 0
@@dx13 dd 0
@@dx23 dd 0

filled_triangle:
	mov eax,[@@ty1]
	cmp eax,[@@ty3]
	jle @@ok13

	xchg eax,[@@ty3]
	mov [@@ty1],eax

	mov eax,[@@tx1]
	xchg eax,[@@tx3]
	mov [@@tx1],eax
@@ok13:
	mov eax,[@@ty2]
	cmp eax,[@@ty3]
	jle @@ok23

	xchg eax,[@@ty3]
	mov [@@ty2],eax

	mov eax,[@@tx2]
	xchg eax,[@@tx3]
	mov [@@tx2],eax
@@ok23:
	mov eax,[@@ty1]
	cmp eax,[@@ty2]
	jle @@ok12

	xchg eax,[@@ty2]
	mov [@@ty1],eax

	mov eax,[@@tx1]
	xchg eax,[@@tx2]
	mov [@@tx1],eax
@@ok12:

	mov ebx,[@@ty2]
	sub ebx,[@@ty1]
	jnz @@make_d12

	mov [@@dx12],dword 0
	jmp @@done_d12
@@make_d12:
	mov eax,[@@tx2]
	sub eax,[@@tx1]
	shl eax,12 ;7
	cdq
	idiv ebx
	mov [@@dx12],eax			; dx12 = (x2-x1)/(y2-y1)
@@done_d12:

	mov ebx,[@@ty3]
	sub ebx,[@@ty1]
	jnz @@make_d13

	mov [@@dx13],dword 0
	jmp @@done_d13
@@make_d13:
	mov eax,[@@tx3]
	sub eax,[@@tx1]
	shl eax,12 ;7
	cdq
	idiv ebx
	mov [@@dx13],eax			; dx13 = (x3-x1)/(y3-y1)
@@done_d13:

	mov ebx,[@@ty3]
	sub ebx,[@@ty2]
	jnz @@make_d23

	mov [@@dx23],dword 0
	jmp @@done_d23
@@make_d23:
	mov eax,[@@tx3]
	sub eax,[@@tx2]
	shl eax,12 ;7
	cdq
	idiv ebx
	mov [@@dx23],eax			; dx23 = (x3-x2)/(y3-y2)
@@done_d23:

	mov eax,[@@tx1]
	shl eax,12 ;7
	mov ebx,eax

	mov ecx,[@@ty1]
 cmp ecx,[@@ty2]
 jge @@end_loop12

@@loop12:

 call flat_line

	add eax,[@@dx13]
	add ebx,[@@dx12]
	inc ecx
	cmp ecx,[@@ty2]
	jl  @@loop12
@@end_loop12:

 mov ecx,[@@ty2]
 cmp ecx,[@@ty3]
 jge @@end_loop23
	
	mov ebx,[@@tx2]
	shl ebx,12 ;7
@@loop23:

 call flat_line

	add eax,[@@dx13]
	add ebx,[@@dx23]
	inc ecx
	cmp ecx,[@@ty3]
	jl  @@loop23
@@end_loop23:

	ret

;flatline proc

flat_line:
 push eax
 push ebx
 push ecx

 or ecx,ecx
 jl @@quit
 cmp ecx,SCREEN_Y-1 ;199
 jg @@quit

 sar eax,12
 sar ebx,12

 or eax,eax
 jge @@ok1
 xor eax,eax
 jmp @@ok2
@@ok1:
 cmp eax,SCREEN_X-1 ;319
 jle @@ok2
 mov eax,SCREEN_X-1 ;319
@@ok2:
 or ebx,ebx
 jge @@ok3
 xor ebx,ebx
 jmp @@ok4
@@ok3:
 cmp ebx,SCREEN_X-1 ;319
 jle @@ok4
 mov ebx,SCREEN_X-1 ;319
@@ok4:
 cmp eax,ebx
 jl @@ok
 je @@quit

 xchg eax,ebx
@@ok:
 mov edi,ecx

; shl edi,6    ;for 320 speed+
; shl ecx,8
; add edi,ecx

 push eax
 mov eax,SCREEN_X
 mul edi
 mov edi,eax
 pop eax

 add edi,eax

 mov ebp,edi
 shl ebp,1
 add edi,ebp

 add edi,scrbuf

 mov ecx,ebx
 sub ecx,eax

lineout:
 mov eax,[@@rgb]
 mov byte [edi],al
 shr eax,8
 mov byte [edi+1],al
 shr eax,8
 mov byte [edi+2],al
 add edi,3
 dec ecx
 jnz lineout

@@quit:
 pop ecx
 pop ebx
 pop eax
 ret

scrbuf:
I_END:
