;------------------------------------------------------------------------------
;   @SS - screensaver
;------------------------------------------------------------------------------
; last update:  30/03/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      The program uses only 4 Kb memory is now.
;               Code refactoring. Using transparent cursor.
;               Fix bug - using lots of buttons from f.8.
;---------------------------------------------------------------------
;   SCREENSAVER APPLICATION by lisovin@26.ru
;
;   Compile with FASM for Menuet
;
;------------------------------------------------------------------------------
	use32
	org 0x0

	db 'MENUET01'	; 8 byte id
	dd 0x01		; header version
	dd START	; start of code
	dd IM_END	; size of image
	dd I_END	; memory for app
	dd stack_top	; esp
	dd 0x0		; I_Param
	dd 0x0		; path
;------------------------------------------------------------------------------
include 'lang.inc'
include '..\..\..\macros.inc'
;include   'debug.inc'
;------------------------------------------------------------------------------
align 4
START:
	mcall	68,11
	mcall	40,110010b
;------------------------------------------------------------------------------
align 4
bgr_changed:

	mcall	14
	mov	[y_max],ax
	shr	eax,16
	mov	[x_max],ax
	shl	eax,16
	mov	[top_right_corner],eax
;------------------------------------------------------------------------------
align 4
still:
	movzx	ebx,[time]
	imul	ebx,60*100
	mcall	23		; ждём события в течение [time] минут
	test	eax,eax
	jz	create_ss_thread

	cmp	al,2		; key in buffer?
	jz	key

	cmp	al,5		; background redraw?
	jz	bgr_changed
; mouse event
	mcall	37,2		; проверим кнопки
	and	al,3
	cmp	al,3		; нажаты обе кнопки мыши?
	jnz	still

	mcall	37,0		; проверим координаты
	cmp	[top_right_corner],eax
	jnz	still
;------------------------------------------------------------------------------
align 4
create_setup:
	test	[params],2
	jnz	still           ; окно настроек уже создано
	mcall	51,1,sthread,sthread_stack_top
	or	[params],2
	jmp	still
;------------------------------------------------------------------------------
align 4
key:
	mcall                   ; eax = 2
	jmp	still
;------------------------------------------------------------------------------
align 4
create_ss_thread:
	test	[params],3
	jnz	still
	call	create_ss
	jmp	still
;------------------------------------------------------------------------------
align 4
create_ss:
	mcall	51,1,thread,thread_stack_top
	or	[params],1
	ret
;------------------------------------------------------------------------------
align 4
thread:
	mcall	40,100010b
;set_new_cursor_skin - transparent cursor
	mcall	68,12,32*32*4	; get memory for own cursor area

	push	eax
	mov	ecx,eax
	mcall	37,4,,2		; load own cursor

	mov	ecx,eax
	mcall	37,5		; set own cursor

	pop	ecx
	mcall	68,13	; free own cursor area

	cmp	[type],dword 0
	je	drawsswin

	cmp	[type],dword 24
	je	asminit

	mov	dword [delay],1
	mov	[lx1],10         ; for "draw line"
	mov	[lx2],40
	mov	[ly1],50
	mov	[ly2],100
	mov	[addx1],1
	mov	[addx2],1
	mov	[addy1],1
	mov	[addy2],1
	jmp	drawsswin
;--------------------------------------
align 4	
asminit:	; for "assembler" - assembler sources demo
; get size of file
	mov	[fileinfo],dword 5
	mov	[fileinfo.point],dword fileinfo_buffer
	mcall	70,fileinfo
	test	eax,eax
	jnz	.no_file
; get memory for file
	mov	ecx,[fileinfo_buffer+32]
	mov	[fileinfo.size],ecx
	mcall	68,12
	mov	[fileinfo.point],eax
; load file
	mov	[fileinfo],dword 0
	mcall	70,fileinfo
	test	eax,eax
	jz	@f
	mcall	68,13,[fileinfo.point]
;--------------------------------------
align 4	
.no_file:
	mov	[type],dword 0
	jmp	drawsswin
;--------------------------------------
align 4	
@@:
	mov	dword [delay],1	;25 - old value
;--------------------------------------
align 4	
asminit1:
	mov	eax,[fileinfo.point]
	mov	[stringstart],eax
	mov	dword [stringlen],1
;--------------------------------------
align 4	
newpage:
	mov	word [stringpos],10
;--------------------------------------
align 4	
drawsswin:
	xor	eax,eax
	movzx	ebx,[x_max]
	movzx	ecx,[y_max]
	inc	ebx
	inc	ecx
	mcall	,,,0x01000000

	xor	edx,edx
	mcall	13
;--------------------------------------
align 4		
tstill:
	mcall	23,[delay]
	test	eax,eax
	jnz	thr_end

	cmp	[type],dword 0
	je	tstill

	cmp	[type],dword 24
	je	drawssasm

	call	draw_line
	jmp	tstill
;--------------------------------------
align 4		
thr_end:
	cmp	[type],dword 24
	jne	@f
	mcall	68,13,[fileinfo.point]
;--------------------------------------
align 4		
@@:
	and	[params], not 1
	or	eax,-1
	mcall
;------------------------------------------------------------------------------
align 4
drawssasm:
	mov	edi,[stringstart]
	add	edi,[stringlen]
	dec	edi

	mov	eax,edi
	sub	eax,[fileinfo.point]

	cmp	eax,[fileinfo.size]
	ja	asminit1

	cmp	word [edi],0x0a0d
	je	addstring

	cmp	byte [edi],0x0a
	jne	noaddstring

	dec	edi
;--------------------------------------
align 4	
addstring:
	add	word [stringpos],10
	add	edi,2
	mov	[stringstart],edi
	mov	dword [stringlen],1
	mov	ax,[stringpos]
	cmp	ax,[y_max]
	jb	tstill
	jmp	newpage
;--------------------------------------
align 4	
noaddstring:
	mov	ebx,10*65536
	mov	bx,[stringpos]
	mcall	4,,0x104ba010,[stringstart],[stringlen]
	inc	dword [stringlen]
	cmp	[edi],byte ' '
	je	drawssasm
	jmp	tstill
;------------------------------------------------------------------------------
align 4
draw_line:
	movzx	esi,[x_max]
	movzx	edi,[y_max]

	mov	eax,[addx1]
	add	[lx1],eax
	mov	eax,[addy1]
	add	[ly1],eax

	mov	eax,[addx2]
	add	[lx2],eax
	mov	eax,[addy2]
	add	[ly2],eax
	
	cmp	[lx1],1
	jge	dl1
	mov	[addx1],1
;--------------------------------------
align 4	
dl1:
	cmp	[lx2],1
	jge	dl2

	mov	[addx2],1
;--------------------------------------
align 4	
dl2:
	cmp	[lx1],esi
	jbe	dl3

	mov	[addx1],0xffffffff
;--------------------------------------
align 4	
dl3:
	cmp	[lx2],esi
	jbe	dl4

	mov	[addx2],0xffffffff
;--------------------------------------
align 4	
dl4:
	cmp	[ly1],1
	jge	dl5
	mov	[addy1],1
;--------------------------------------
align 4	
dl5:
	cmp	[ly2],2
	jge	dl6

	mov	[addy2],1
;--------------------------------------
align 4	
dl6:
	cmp	[ly1],edi
	jbe	dl7

	mov	[addy1],-1
;--------------------------------------
align 4	
dl7:
	cmp	[ly2],edi
	jbe	dl8

	mov	[addy2],-1
;--------------------------------------
align 4	
dl8:
	mov	eax,[lx2]
	cmp	[lx1],eax
	jz	dnol
	
	mov	bx,word [lx1]
	shl	ebx,16
	mov	bx,word [lx2]

	mov	cx,word [ly1]
	shl	ecx,16
	mov	cx,word [ly2]
	
	mov	edx,[lcolor]
	and	edx,0xffffff
	mcall	38
;--------------------------------------
align 4	
dnol:
	add	[lcolor],0x010201
	ret
;------------------------------------------------------------------------------
align 4
sthread:		; start of execution
     call sdraw_window
;--------------------------------------
align 4	
sstill:
	mcall	10	; wait here for event
	dec	eax	; redraw request ?
	je	sthread

	dec	eax	; key in buffer ?
	jne	sbutton

	mcall	2
	jmp	snoclose	;sstill
;------------------------------------------------------------------------------
align 4
sbutton:		; button
	mcall	17	; get id

	cmp	ah,1	; button id=1 ?
	jne	snoclose

	and	[params],not 2
	mov	eax,-1	; close this program
	mcall
;--------------------------------------
align 4	
snoclose:
	cmp	ah,7
	jne	nosetfl

	xor	[params],1
	call	drawflag
	call	drawtype
	call	drawtime
	jmp	sstill
;--------------------------------------
align 4	
nosetfl:
	test	[params],1
	jnz	sstill

	cmp	ah,2
	jne	notypedown

	mov	eax,[type]
	test	eax,eax
	je	sstill

	sub	eax,12
	jmp	typeupdn
;--------------------------------------
align 4
notypedown:
	cmp	ah,3
	jne	notypeup

	mov	eax,[type]
	cmp	eax,24
	jae	sstill

	add	eax,12
	jmp	typeupdn
;--------------------------------------
align 4
notypeup:
	cmp	ah,4
	jne	notimedown

	mov	al,[time]
	cmp	al,1
	jbe	sstill

	dec	eax
;	das
	jmp	timeupdn
;--------------------------------------
align 4
notimedown:
	cmp	ah,5
	jne	notimeup

	mov	al,[time]
	cmp	al,59	; 0x59
	jae	sstill

	inc	eax
;	daa
	jmp	timeupdn
;--------------------------------------
align 4
notimeup:
	cmp	ah,6
	jne	noshow

	mcall	5,10
	call	create_ss
;--------------------------------------
align 4
noshow:
	jmp	sstill
;--------------------------------------
align 4
timeupdn:
	mov	[time],al
	call	drawtime
	jmp	sstill
;--------------------------------------
align 4
typeupdn:
	mov	[type],eax
	call	drawtype
	jmp	sstill
;------------------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
align 4
sdraw_window:
	mcall	12,1

	xor	eax,eax		; function 0 : define and draw window
	xor	esi,esi
	mcall	,<100,215>,<100,70>,0x13400088,,title

	mcall	8,<47,10>,<31,10>,2,0x702050
    
	push	ebx
	add	ebx,13*65536
	mov	edi,ebx
	inc	edx
	mcall
	pop	ebx
    
	add	ecx,15*65536
	inc	edx
	mcall

	mov	ebx,edi
	inc	edx
	mcall

	inc	edx
	mcall	,<160,40>,<28,14>

	mcall	4,<15,33>,0x80ffffff,setuptext	; write text to window

	add	ebx,15
	add	edx,10
	mcall

	mcall	,<169,32>,,buttext

	call	drawtype
	call	drawtime
	call	drawflag

	mcall	12,2
	ret
;------------------------------------------------------------------------------
align 4
drawtype:
	mov	edx,0xffffff
	test	[params], 1
	jz	noblue

	mov	edx,0x4e00e7
;--------------------------------------
align 4
noblue:
	mcall	13,<80,75>,<30,12>
	xor	ecx,ecx
	mov	edx,typetext
	add	edx,[type]
	mcall	4,<82,32>,,,12
	ret
;------------------------------------------------------------------------------
align 4
drawtime:
	mov	edx,0xffffff
	test	[params], 1
	jz	noblue1

	mov	edx,0x4e00e7
;--------------------------------------
align 4
noblue1:
	mcall	13,<80,15>,<45,12>
	xor	esi,esi
	movzx	ecx,byte [time]
	mcall	47,0x00020000,,<82,47>
	ret
;------------------------------------------------------------------------------
align 4
drawflag:
	mcall	8,,,0x80000007	; before we need delete button
; otherwise, a few hours later the application will spend all buttons of system
	mcall	,<150,10>,<45,10>,7,0xe0e0e0	; then create button
	
	mov	edx,flag
	bt	dword [params],0
	jc	setf

	inc	edx
;--------------------------------------
align 4
setf:
	xor	ecx,ecx
	mcall	4,<153,47>,,,1
	ret
;------------------------------------------------------------------------------
align 4
; DATA AREA
buttext		db 'SHOW',0
flag		db 'V '
title		db 'SCREENSAVER SETUP',0
setuptext	db 'TYPE: < >',0
		db 'TIME: < >     MINUTES    NEVER',0
typetext	db 'BLACK SCREENCOLOR LINES ASSEMBLER   '
type	dd 12
time	db 15	; время до запуска заставки в минутах
delay	dd 100

lx1	dd 10
lx2	dd 40

ly1	dd 50
ly2	dd 100

addx1	dd 1
addx2	dd 1

addy1	dd 1
addy2	dd 1

stringlen	dd 1
stringstart	dd 0
stringpos	dw 10

params	db 0	;if bit 0 set-ssaver works if bit 1 set-setup works

fileinfo:
	dd 0
	dd 0
	dd 0
.size:	dd 0
.point:	dd 0
	db '/sys/macros.inc',0
;------------------------------------------------------------------------------
align 4
IM_END:
; UNINITIALIZED DATA:

lcolor	dd ?
x_max	dw ?	; размеры экрана
y_max	dw ?

top_right_corner	rd 1
;------------------------------------------------------------------------------
align 4
fileinfo_buffer:
	rb 40
;------------------------------------------------------------------------------
align 4
	rb 512
sthread_stack_top:
;------------------------------------------------------------------------------
align 4
	rb 512
thread_stack_top:
;------------------------------------------------------------------------------
align 4
	rb 512
stack_top:
I_END:
;------------------------------------------------------------------------------
