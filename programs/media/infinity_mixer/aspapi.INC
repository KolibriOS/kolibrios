include '../../proc32.inc'

macro start_draw_window x,y,xsize,ysize,areacolor,caption;,capsize
{
	;pusha
	mov	eax, 12 		; function 12:tell os about windowdraw
	mov	ebx, 1			; 1, start of draw
	int	0x40
	; DRAW WINDOW
	mov	eax, 0			; function 0 : define and draw window
	mov	ebx, x*65536+xsize	; [x start] *65536 + [x size]
	mov	ecx, y*65536+ysize	; [y start] *65536 + [y size]
	mov	edx, areacolor		 ; color of work area RRGGBB
	or	edx, 0x34000000
	mov	edi, caption
;        mov     esi, 0x00334455          ; color of grab bar  RRGGBB
;        mov     edi, 0x00ddeeff          ; color of frames    RRGGBB
	int	0x40
	;popa
}

macro end_draw_window
{
	mov	eax, 12 		; end of redraw
	mov	ebx, 2
	int	0x40
}

macro change_window_place_size x,y,xsize,ysize
{
	mov	eax, 67 		 ; function 67 : replace and/or resize window
	mov	ebx, x
	mov	ecx, y
	mov	edx, xsize
	mov	esi, ysize
	int	0x40
}

proc draw_button stdcall, x:dword, y:dword, xsize:dword, ysize:dword, \
	id:dword, butcolor:dword, text:dword, textlen:byte, textcolor:dword

	pusha
	mov	ebx, dword [x]
	shl	ebx, 16
	add	ebx, dword [xsize]	  ; [x start] *65536 + [x size]

	mov	ecx, dword [y]
	shl	ecx, 16
	add	ecx, dword [ysize]	  ; [y start] *65536 + [y size]

	mov	edx, dword [id] 		; button id
	mov	esi, dword [butcolor]		; button color RRGGBB
	mov	eax, 8			; function 8 : define and draw button
	int	0x40

	mov	ebx, dword [x]
	add	ebx, 5
	shl	ebx, 16
	mov	eax, dword [ysize]
	sub	eax, 5
	shr	eax, 1
	add	ebx, eax
	add	ebx, dword [y]	;mov    ebx, (x+5)*65536+y+(ysize-5)/2  ; Draw button text

	mov	ecx, dword [textcolor]
	mov	edx, dword [text]
	xor	eax, eax
	mov	al,  byte [textlen]
	mov	esi, eax
	mov	eax, 4
	int	0x40
	popa
ret
endp

;macro outtextxy x,y,prompt,prompt_len,color
;{
proc outtextxy stdcall, x:dword, y:dword, prompt:dword, prompt_len:dword, color:dword
	pusha
	mov	eax, 4
	mov	ebx, dword [x]	       ; draw info text with function 4
	shl	ebx, 16
	or	ebx, dword [y]
	mov	ecx, dword [color]
	mov	edx, dword [prompt]
	mov	esi, dword [prompt_len]
	int	0x40
	popa
ret
endp
;}

proc bar x:dword, y:dword, xsize:dword, ysize:dword, color:dword
;macro bar x, y, xsize, ysize, color
;{
	pusha
	mov	eax, 13
	mov	ebx, [x]
	shl	ebx, 16
	add	ebx, [xsize]
	mov	ecx, [y]
	shl	ecx, 16
	add	ecx, [ysize]
	mov	edx, [color]
	;mov     ebx, x*65536+xsize
	;mov     ecx, y*65536+ysize
	;mov     edx, color

	int	0x40
	popa
ret
endp
;}


proc line x1:dword, y1:dword, x2:dword, y2:dword, color:dword
;macro line x1,y1,x2,y2,color
;{
	pusha
	mov	eax, 38
;        mov     ebx, x1*65536+x2
;        mov     ecx, y1*65536+y2
	mov	ebx, [x1]
	shl	ebx, 16
	add	ebx, [x2]
	mov	ecx, [y1]
	shl	ecx, 16
	add	ecx, [y2]

	mov	edx, [color]
	int	0x40
	popa
ret
endp
;}


;macro rectangle x,y,xsize,ysize,color
;{
;  x2=x+xsize
;  y2=y+ysize
;  line x,y,x2,y,color
;  line x,y,x,y2,color
;  line x,y2,x2,y2,color
;  line x2,y,x2,y2,color
;}

proc rectangle x:dword, y:dword, xsize:dword, ysize:dword, color:dword
	pusha
	mov	eax, [x]
	mov	ebx, eax
	add	ebx, [xsize]
	mov	ecx, [y]
	mov	edx, ecx
	add	edx, [ysize]
	mov	edi, [color]

	stdcall line, eax,ecx,ebx,ecx,edi
	stdcall line, eax,ecx,eax,edx,edi
	stdcall line, eax,edx,ebx,edx,edi
	stdcall line, ebx,ecx,ebx,edx,edi
	popa
ret
endp


proc putpixel stdcall, x:dword, y:dword, color:dword
;macro putpixel x,y,color
;{
	pusha
	mov	eax, 1
	mov	ebx, [x] ;x
	mov	ecx, [y] ;y
	mov	edx, [color] ;color
	int	0x40
	popa
ret
endp
;}

