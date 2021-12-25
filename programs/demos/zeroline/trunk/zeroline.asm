;------------------------------------------------------------------------------
;   @SS - screensaver
;------------------------------------------------------------------------------
; last update:	30/03/2012
; changed by:	Marat Zakiyanov aka Mario79, aka Mario
; changes:	The program uses only 4 Kb memory is now.
;		Code refactoring. Using transparent cursor.
;		Fix bug - using lots of buttons from f.8.
;---------------------------------------------------------------------
;   SCREENSAVER APPLICATION by lisovin@26.ru
;
;   Compile with FASM for Menuet
;
;------------------------------------------------------------------------------
	use32
	org 0x0

	db 'MENUET01'	; 8 byte id
	dd 0x01 	; header version
	dd START	; start of code
	dd IM_END	; size of image
	dd I_END	; memory for app
	dd stack_top	; esp
	dd I_Param	; boot parameters
	dd 0x0		; path
;------------------------------------------------------------------------------
include '..\..\..\macros.inc'
;include   'debug.inc'
;------------------------------------------------------------------------------
align 4
START:
	mcall	68,11
	mcall	40, EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE
; boot parameters
	cmp [I_Param], dword '@ss'
	setz [screensaver]
	cmp	[I_Param],dword 'ZERO'
	jne	@f
	mov	[type],dword 0
;--------------------------------------
align 4
@@:
	cmp	[I_Param],dword 'ASSM'
	jne	bgr_changed
	mov	[type],dword 24
;------------------------------------------------------------------------------
align 4
bgr_changed:

	mcall	14
	mov	[y_max],ax
	shr	eax,16
	mov	[x_max],ax
	shl	eax,16
	mov	[top_right_corner],eax
	
	call create_ss
;------------------------------------------------------------------------------
align 4
still:
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
key:
	mcall			; eax = 2
	jmp	still
;------------------------------------------------------------------------------
align 4
create_ss:
	mcall	40,100010b
;set_new_cursor_skin - transparent cursor
	mcall	68,12,32*32*4	; get memory for own cursor area

	push	eax
	mov	ecx,eax
	mcall	37,4,,2 	; load own cursor

	mov	ecx,eax
	mcall	37,5		; set own cursor

	pop	ecx
	mcall	68,13	; free own cursor area

	cmp	[type],dword 0
	je	drawsswin

	cmp	[type],dword 24
	je	asminit

	mov	dword [delay],1
	mov	[lx1],10	 ; for "draw line"
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
	mov	dword [delay],1 ;25 - old value
;--------------------------------------
align 4
asminit1:
	mov	eax,[fileinfo.point]
	mov	[stringstart],eax
	mov	dword [stringlen],1
;--------------------------------------
align 4
newpage:
	mov	[stringpos],16
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
	;mcall	13 ;Leency - use transparent background
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
    cmp     [screensaver], 0
    jz      @f
    mcall   70, f70
@@:
	cmp	[type],dword 24
	jne	@f
	mcall	68,13,[fileinfo.point]
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
	add	[stringpos],16
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
	mov	ebx,[stringlen]
	shl	ebx,19
	mov	bx,[stringpos]
	mov	edx,[stringstart]
	add	edx,[stringlen]
	dec	edx
	mcall	4,,0x104ba010,,1
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
align 4 	; DATA AREA
type	dd 12
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

stringpos	dw 16
params		db 0	;if bit 0 set-ssaver works if bit 1 set-setup works

fileinfo:
	dd 0
	dd 0
	dd 0
.size:	dd 0
.point: dd 0
	db '/sys/macros.inc',0

f70:    ; run
        dd 7, 0, 0, 0, 0
        db '/sys/@SS',0

screensaver db ?

;-------------------------------
IM_END: 	; UNINITIALIZED DATA
top_right_corner	rd 1
align 4
lcolor	dd ?
x_max	dw ?	; размеры экрана
y_max	dw ?
I_Param:
fileinfo_buffer:
	rb 40
;-------------------------------
	rb 512
stack_top:
I_END:
