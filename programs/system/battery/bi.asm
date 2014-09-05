; Battery Indicator v0.ALPHA by Gluk
include "../../macros.inc"
MEOS_APP_START
CODE

init:
	mov cl,48d			; setting waitingmask
	mov edx,0x110
	mcall 66,4

mov word[vminor],0
mov dx,0x5300
xor ebx,ebx
mov eax,49
jnc @f
mov word[vminor],ax
@@:
mov dx,0x5308
mov bx,1
mov cx,bx
mov eax,49
int 0x40
mov dx,0x530E
xor bx,bx
mov cx,0x0102
mov eax,49
int 0x40
mov dx,0x530D
mov bx,1
mov cx,bx
mov eax,49
int 0x40
mov dx,0x530F
mov bx,1
mov cx,bx
mov eax,49
int 0x40
	xor ebx,ebx
	mov bx,[bid]			; find a bid
	dec ebx
	@@:
	mov eax,49
	mov dx,530Ah
	inc ebx
	int 0x40
	jc @b
       ;cmp cl,0xff
       ;je @b
	finded:
	mov [bid],bx
	;mcall 49,0,0,5310h

	mcall 9,streaminfo,-1		; get process data
	mov ecx,dword[streaminfo+30d]
	mcall 18,21
	mov [slotid],eax

	mov eax,48			; get system colors
	mov ebx,3
	mov ecx,sc
	mov edx,sizeof.system_colors
	mcall
initend:

  redraw:
   call     draw_window

wait_event:				; main cycle
	call redata
	mov ebx,[waiting]
	mcall 23
	cmp eax,0
	jz wait_event
	dec eax
	jz redraw
	dec eax
	jz key
	dec eax
	jz button
jmp wait_event

button:
	mcall 17
	@@:		;1:
	dec ah
	;jnz @f
	;        or eax,-1
	;        mcall
	;@@:             ;2
	dec ah
	jnz @f
		mov eax,dword[whatview]
		mov ebx,views_end-3
		add eax,4
		cmp eax,ebx
		jb allok1
			mov eax,views_start
		allok1:
		mov [whatview],eax
		mov eax,[eax]
		mov [viewer],eax
	@@:
jmp wait_event

redata:
	call getdata
	mov al,[window]
	cmp al,0
	jz @f
		call regraph
		call rebutton
	@@:
ret

key:				; key event handler
    mov al,2			; get key code
    mcall
    cmp al,2
    jne wait_event
	    call rewindow
jmp wait_event

  draw_window:
	mov eax,12
	mov ebx,1
	mcall

	mov al,byte[window]
	cmp al,0
	jz nowindow

	mcall 48,5
	sub eax,[winotstx]
	sub eax,[winsizex]
	shl eax,16
	sub ebx,[winotsty]
	sub ebx,[winsizey]
	shl ebx,16
	mov ecx,ebx
	mov ebx,eax

	xor eax,eax			; create and draw the window
	add ebx,dword[winsizex]
	dec ebx 			; (window_c)*65536+(window_s)
	add ecx,dword[winsizey]
	dec ecx
	mov edx,[sc.work]	       ; work area color
	or edx, 0x61000000		; & window type 1
	int 0x40

	mov eax,13d			;printing pryamougolniks
	push eax			;pipka out
		mov eax,[winsizex]
		xor edx,edx
		mov ebx,3
		div ebx
		mov ebx,eax
     ;                   pusha
			;mov [nowpoint],0
			;in: ebx - piplenght
      ;                  mov eax,0
			;mov edi, nowpoint
       ;                 add edi,winform
	;                mov ecx,ebx
	 ;               rep stosb
	  ;              mov eax,1
			;mov edi, nowpoint
	   ;             add edi,winform
	    ;            mov ecx,ebx
	     ;           rep stosb
	      ;          mov eax,0
			;mov edi, nowpoint
	       ;         add edi,winform
		;        mov ecx,ebx
		 ;       rep stosb
		  ;      popa
		mov ecx,ebx
		shl ebx,16
	pop eax
	add ebx,ecx
	mov ecx,[winsizey]
	shr ecx,4		;div 16 ;)
	xor edx,edx
	int 0x40

	add ebx,65536-2 		;pipka in
	add ecx,65536-1
	mov edx,[sc]
	int 0x40

	mov ebx,[winsizex]		;korpus out
	mov ecx,[winsizey]
	shr ecx,4
	mov edx,ecx
	shl ecx,16
	add ecx,[winsizey]
	sub ecx,edx
	xor edx,edx
	int 0x40

	add ebx,65536-2 		;korpus in
	add ecx,65536-2
	mov edx,[sc]
	int 0x40

	mov edx,[winborts]
	mov edi,edx
	shl edx,16
	add ebx,edx
	sub ebx,edi
	sub ebx,edi			;black contur
	add ecx,65536-3
	sub ecx,[winknopy]
	xor edx,edx
	int 0x40

	add ebx,65536-2 		;working area
	add ecx,65536-2
	mov edx,[sc.work]
	int 0x40
	mov [winworkx],ebx
	mov [winworky],ecx

	call redata
	nowindow:
	mov eax,12			;finish drawing
	mov ebx,2
	int 0x40
ret

rebutton:
	mov eax,8
	mov edx,0x80000002
	int 0x40

	mov ebx,[winsizex]
	add ebx,65536-3
	mov edi,[winknopy]
	mov ecx,[winsizey]
	sub ecx,edi
	sub ecx,2
	shl ecx,16
	add ecx,edi
	mov edx,0x00000002
	mov esi,[sc.work_button]
	int 0x40

	mov [wintextx],3
	sub edi,[winfonty]
	shr edi,1
	mov ebx,edi
	add ebx,[winsizey]
	sub ebx,[winknopy]
	mov [wintexty],ebx
	call dword[viewer]
ret


rewindow:
	mov al,byte[window]
	cmp al,1
	jne @f
		mcall 67,1,1,0,0
		mov byte[window],0
		mcall 40,2
		jmp endrew
	@@:
		mcall 48,5
		sub eax,[winotstx]
		sub eax,[winsizex]
		sub ebx,[winotsty]
		sub ebx,[winsizey]
		mov ecx,ebx
		mov ebx,eax

		mov eax,67
		mov edx,[winsizex]
		dec edx
		mov esi,[winsizey]
		dec esi
		int 0x40

		mov byte[window],1
		mov ecx,[slotid]
		mcall 18,3

		mcall 40,7
	endrew:
	call draw_window
ret

regraph:
	mov eax,13
	mov ebx,[winworkx]		   ;working area
	mov ecx,[winworky]
	mov edx,[sc.work]
	int 0x40

	mov ebx,[winworky]
	shl ebx,16
	shr ebx,16
	sub ebx,[wingotst]
	sub ebx,[wingotst]
	mov eax,ebx
	shr ebx,3		;div 8 ;)
	shl eax,29
	shr eax,32		;ostatok
	add eax,[wingotst]
	cmp eax,ebx
	jb @f
		inc ebx
	@@:
	mov [winlines],ebx
	mov ecx,[delenia]
	@@:
		push ecx
		mov edi,ecx

		mov edx,[wingotst]
		mov ebx,[winworkx]
		shl edx,16
		add ebx,edx
		shr edx,16
		sub ebx,edx
		sub ebx,edx

		mov ecx,[winworky]
		mov eax,[winworky]
		shl eax,16
		add ecx,eax

		push edx
		mov eax,[winlines]
		mul edi
		pop edx
		add edx,eax

		shl edx,16
		sub ecx,edx

		shr ecx,16
		inc ecx
		shl ecx,16
		add ecx,[winlines]
		sub ecx,1 ;promezhutki
		mov edx,[sc.work_graph]
		mov eax,13
		int 0x40

		pop ecx
	loop @b
endreg:
ret

getdata:
	xor ecx,ecx
	xor edx,edx

;HERE YOU MAY GET A PERCENTAGE AND REMAINING TIME FOR BATTERY UNIT [bid], AND PUT THEY INTO [gotperc] AND [gottime]
	mov bx,[bid]
	mov eax,49
	mov dx,530Ah
	int 0x40
	mov [gotperc],cl
	mov [gottime],dx
;/HERE

	xor eax,eax
	mov al,[gotperc]
	cmp al,0
	jne @f
		mov [delenia],1
		ret
	@@:
	cmp al,100
	jb @f
		mov [delenia],8
		ret
	@@:
	shl eax,3
	mov ebx,100
	xor edx,edx
	div ebx
	inc eax
	mov [delenia],eax
ret

viewers:
	time:
		xor edx,edx
		mov dx,[gottime]
		cmp dx,0xffff
		jne @f
			mov eax,4
			mov ebx,[wintextx]
			shl ebx,16
			add ebx,[wintexty]
			mov ecx,0x00000000
			mov edx,simbols
			mov esi,3
			add ecx,[sc.work_button_text]
			int 0x40
		ret
		@@:
		shl edx,17
		shr edx,31
		mov eax,4
		mov ebx,[wintextx]
		add ebx,2*8
		shl ebx,16
		add ebx,[wintexty]
		mov ecx,0x00000000
		add edx,simbols
		mov esi,1
		add ecx,[sc.work_button_text]
		int 0x40
      ;12345678901234567890123456789012
		mov eax,47
		mov bl,2	;cifr
		mov bh,0
		shl ebx,16
		mov bl,0	;ecx is chislo
		mov bh,0
		xor ecx,ecx
		mov cx,[gottime]
		shl ecx,18
		shr ecx,18
		mov edx,[wintextx]
		shl edx,16
		add edx,[wintexty]
		mov esi,0x10000000
		add esi,[sc.work_button_text]
		int 0x40
	ret
	percent:
		mov dl,[gotperc]
		cmp dl,0xff
		jne @f
			mov eax,4
			mov ebx,[wintextx]
			shl ebx,16
			add ebx,[wintexty]
			mov ecx,0x00000000
			mov edx,simbols
			mov esi,4
			add ecx,[sc.work_button_text]
			int 0x40
		ret
		@@:
		mov eax,4
		mov ebx,[wintextx]
		add ebx,3*8
		shl ebx,16
		add ebx,[wintexty]
		mov ecx,0x00000000
		mov edx,simbols
		add edx,3
		mov esi,1
		add ecx,[sc.work_button_text]
		int 0x40

		mov eax,47
		mov bl,3	;cifr
		mov bh,0
		shl ebx,16
		mov bl,0	;ecx is chislo
		mov bh,0
		xor ecx,ecx
		mov cl,[gotperc]
		mov edx,[wintextx]
		shl edx,16
		add edx,[wintexty]
		mov esi,0x10000000
		add esi,[sc.work_button_text]
		int 0x40
	ret
; <--- initialised data --->
DATA

bid dw 8000h

viewer dd percent
whatview dd views_start
views_start:
	dd percent
	dd time
views_end:

simbols db 'smh%??m???%'

waiting dd 1000
watchings:
	window db 1
	winotstx dd 7
	winotsty dd 7
	winsizex dd 48
	winsizey dd 64
	winborts dd 1
	winfonty dd 9
	winknopy dd 10
	wingotst dd 2
; <--- uninitialised data --->
UDATA
vminor dw ?
sc system_colors
streaminfo rb 1024
winform rb 1024
slotid dd ?

gotperc db ?
gottime dw ?
delenia dd ?

uwatchings:
	winhomex dd ?
	winhomey dd ?
	winworkx dd ?		;cx*65536+sx
	winworky dd ?		;cy*65536+sy
	winlines dd ?
	wintextx dd ?
	wintexty dd ?
MEOS_APP_END