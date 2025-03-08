;  256b intro by baze/3SC for Syndeecate 2001
;  loveC: thanks, Serzh: eat my socks dude ;]
;  e-mail: baze@stonline.sk, web: www.3SC.sk

;  Menuet port by VT


use32
	org  0
	db   'MENUET01'
	dd   1,START,image_end,memory_end,stacktop,0,0

include '../../macros.inc'
include '../../KOSfuncs.inc'
include 'lang.inc'

if lang eq ru_RU
	title db 'Труба - FPU',0
else
	title db 'Tube - FPU',0
end if

SCREEN_W equ (640-10) ;10 px for borders
SCREEN_H equ (400-10)

align 4
START:
	call draw_window
	call init_tube
	push ebx

still:
	pop  ebx
	call MAIN
	push ebx

	mcall SF_WAIT_EVENT_TIMEOUT,1

	cmp  eax,EV_REDRAW
	jne  no_red
	call draw_window
	jmp  still
 no_red:

	or   eax,eax
	jz   still

	mcall SF_TERMINATE_PROCESS

EYE     equ EYE_P-4


align 4
MAIN:
;edx - coord y
;ebp - coord x
;edi - pixel buffer
	add    ebx,10 shl 8
	mov    edi,PIXBUF
	fadd   dword [TEXUV-4]
	push   edi
	mov    edx,-SCREEN_H/2

TUBEY:
	mov    ebp,-SCREEN_W/2

TUBEX:
	mov    esi,TEXUV
	fild   word [EYE]
	mov    [esi],ebp
	fild   word [esi]
	mov    [esi],edx
	fild   word [esi]
	mov    cl,2

ROTATE:
	fld    st3
	fsincos
	fld    st2
	fmul   st0,st1
	fld    st4
	fmul   st0,st3
	fsubp  st1,st0
	fxch   st3
	fmulp  st2,st0
	fmulp  st3,st0
	faddp  st2,st0
	fxch   st2
	loop   ROTATE

	fld    st1
	db 0xdc,0xc8 ;fmul   st0,st ?
	fld    st1
	db 0xdc,0xc8 ;fmul   st0,st ?
	faddp  st1,st0
	fsqrt
	
	fdivp  st3,st0
	fpatan
	fimul  word [esi-4]
	fistp  word [esi]
	fimul  word [esi-4]
	fistp  word [esi+1]
	mov    esi,[esi]

	lea    eax,[ebx+esi]
	add    al,ah
	and    al,64
	mov    al,-5
	jz     STORE_1

	shl    esi,2
	lea    eax,[ebx+esi]
	sub    al,ah
	mov    al,-16
	jns    STORE_1

	shl    esi,1
	mov    al,-48

STORE_1:
; add    al,[ebx+esi+0x80000]
	add    [edi],al
	inc    edi
	inc    ebp
	cmp    ebp,SCREEN_W/2

EYE_P:
	jnz    TUBEX
	inc    edx
	cmp    edx,SCREEN_H/2
	jnz    TUBEY

	call   display_image

	pop    esi
	mov    ecx,SCREEN_H*SCREEN_W

align 4
BLUR:
	inc    esi
	sar    byte [esi],2
	loop   BLUR
	ret


align 4
display_image:
	pusha

	mov esi,PIXBUF
	mov edi,buf2
align 4
newp:
	movzx edx,byte [esi]
	shl edx,4

	mov [edi],edx

	add edi,3
	inc esi

	cmp esi,PIXBUF+SCREEN_W*SCREEN_H
	jbe newp

	xor edx,edx
	mcall SF_PUT_IMAGE,buf2,<SCREEN_W,SCREEN_H>

	popa
	ret


align 4
draw_window:
	pusha

	mcall SF_REDRAW, SSF_BEGIN_DRAW
	mcall SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
	add eax,SCREEN_H
	lea ecx,[100*65536+4+eax]
	mcall SF_CREATE_WINDOW,100*65536+SCREEN_W+9,, 0x74000000,,title
	mcall SF_REDRAW, SSF_END_DRAW
	popa
	ret

align 4
	db 41,0,0xC3,0x3C
TEXUV:
	rd 1

align 4
init_tube:
	mov ecx,256

PAL1:
	mov edx,3C8h
	mov eax,ecx
	inc edx
	sar al,1
	js PAL2
	mul al
	shr ax,6

PAL2:
	mov al,0
	jns PAL3
	sub al,cl
	shr al,1
	shr al,1

PAL3:
	mov ebx,ecx
	mov [ebx+buf1],bh
	loop PAL1
	mov  ecx,256

TEX:
	mov bx,cx
	add ax,cx
	rol ax,cl
	mov dh,al
	sar dh,5
	adc dl,dh
	adc dl,[ebx+255+buf1]
	shr dl,1
	mov [ebx+buf1],dl
	not bh
	mov [ebx+buf1],dl
	loop TEX

	fninit
	fldz

	ret

align 4
image_end:
PIXBUF rb SCREEN_W*SCREEN_H
buf1 rb SCREEN_W*SCREEN_H
buf2 rb SCREEN_W*SCREEN_H*3
	rb	1024
align 4
stacktop:
memory_end:
