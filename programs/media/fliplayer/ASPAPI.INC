macro start_draw_window x,y,xsize,ysize,areacolor,caption
{
	mov	eax, 12 		; function 12:tell os about windowdraw
	mov	ebx, 1			; 1, start of draw
	int	0x40
	; DRAW WINDOW
	mov	eax, 0			; function 0 : define and draw window
	mov	ebx, x*65536+xsize	; [x start] *65536 + [x size]
	mov	ecx, y*65536+ysize	; [y start] *65536 + [y size]
	mov	edx,areacolor		; color of work area RRGGBB
        mov     edi,caption             ; window title
	int	0x40
}

macro end_draw_window
{
	mov	eax, 12 		; end of redraw
	mov	ebx, 2
	int	0x40
}

macro draw_button x,y,xsize,ysize,id,butcolor,text,textlen,textcolor
{
	mov	eax, 8			; function 8 : define and draw button
	mov	ebx, (x)*65536+xsize	  ; [x start] *65536 + [x size]
	mov	ecx, (y)*65536+ysize	  ; [y start] *65536 + [y size]
	mov	edx, id 		; button id
	mov	esi, butcolor		; button color RRGGBB
	int	0x40

	mov	ebx, (x+5)*65536+y+(ysize-5)/2	; Draw button text
	mov	ecx, textcolor
	mov	edx, text
	xor	eax, eax
	mov	al,  textlen
	mov	esi, eax
	mov	eax, 4
	int	0x40
}

macro outtextxy x,y,prompt,prompt_len,color
{
	mov	ebx, x*65536+y	       ; draw info text with function 4
	mov	ecx, color
	mov	edx, prompt
	xor	eax, eax
	mov	al,  prompt_len
	mov	esi, eax
	mov	eax, 4
	int	0x40
}

macro bar x,y,xsize,ysize,color
{
  mov eax, 13
  mov ebx, x*65536+xsize
  mov ecx, y*65536+ysize
  mov edx, color
  int 0x40
}

macro line x1,y1,x2,y2,color
{
  mov eax, 38
  mov ebx, x1*65536+x2
  mov ecx, y1*65536+y2
  mov edx, color
  int 0x40
}

macro rectangle x,y,xsize,ysize,color
{
  x2=x+xsize
  y2=y+ysize
  line x,y,x2,y,color
  line x,y,x,y2,color
  line x,y2,x2,y2,color
  line x2,y,x2,y2,color
}

macro putpixel x,y,color
{
  mov eax, 1
  mov ebx, x
  mov ecx, y
  mov edx, color
  int 0x40
}

macro lights_on cns
{
   mov ax, cns
   test ax, 0x40
   jz @f
   push ax
   bar 532, 53, 10, 11, 0x0000FF00
   pop ax
 @@:
   test ax, 0x80
   jz @f
   push ax
   bar 500, 53, 10, 11, 0x0000FF00
   pop ax
 @@:
   test ax, 0x100
   jz @f
   bar 565, 53, 10, 11, 0x0000FF00
 @@:
}

out_symbol:
	;mov	ebx, x*65536+y	       ; draw info text with function 4
	;mov	ecx, color
	;mov	edx, prompt
	mov	esi, 1
	mov	eax, 4
	int	0x40
  ret

