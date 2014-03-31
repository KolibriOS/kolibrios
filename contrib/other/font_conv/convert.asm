use16
org $100

;==============================================================================

macro fopen { ; DX = filename
  mov	ax,$3D00
  int	$21
}

macro fclose { ; BX = file handle
  mov	ah,$3E
  int	$21
}

macro fread { ; BX = file handle, CX = count, DX = buffer
  mov	ah,$3F
  int	$21
}

macro fwrite { ; BX = file handle, CX = count, DX = buffer
  mov	ah,$40
  int	$21
}

;==============================================================================

; /////////////// char.mt ///////////////

	mov	dx,_font1_in
		fopen
	jc	exit
	xchg	ax,bx
	mov	cx,$5000
	mov	dx,_buf
		fread
	jc	exit
		fclose
;---------------------------------------
	mov	bx,_buf+8
	xor	si,si
	mov	di,_buf
	mov	cl,1
	xor	al,al
	cld
    @@:
	cmp	byte[bx+si],$20
	je	.sp
	or	al,cl
    .sp:
	inc	si
	shl	cl,1
	cmp	cl,00100000b
	jne	@b
	mov	cl,1
	stosb
	xor	al,al
	add	si,3
	cmp	si,8*9
	jb	@b
	xor	si,si
	add	bx,8*10
	cmp	di,_buf+$900
	jb	@b
;---------------------------------------
	mov	dx,_font1_out
	xor	cx,cx
	mov	ah,$3C ; create
	int	$21
	jc	exit
	xchg	ax,bx
	mov	cx,$900
	mov	dx,_buf
		fwrite
	jc	exit
		fclose

; /////////////// char2.mt ///////////////

	mov	dx,_font2_in
		fopen
	jc	exit
	xchg	ax,bx
	mov	cx,$6500
	mov	dx,_buf
		fread
	jc	exit
		fclose
;---------------------------------------
	mov	bx,_buf+10
	mov	di,_buf
	cld
newchar:
; width of character
	mov	al, 8
	cmp	byte [bx-4], ' '
	jz	@f
	mov	al, [bx-4]
	sub	al, '0'-1
    @@:
    	stosb
; character itself
	xor	si, si
charloop:
	xor	al, al
	mov	cl, 1
    @@:
	cmp	byte[bx+si],$20
	je	.sp2
	or	al,cl
    .sp2:
	inc	si
	shl	cl,1
	cmp	cl,10000000b
	jne	@b
	stosb
	add	si,3
	cmp	si,10*9
	jb	charloop
	add	bx,10*10
	cmp	di,_buf+$A00
	jb	newchar
;---------------------------------------
	mov	dx,_font2_out
	xor	cx,cx
	mov	ah,$3C ; create
	int	$21
	jc	exit
	xchg	ax,bx
	mov	cx,$A00
	mov	dx,_buf
		fwrite
	jc	exit
		fclose

  exit:
	ret

;==============================================================================

_font1_in db 'char.txt',0
_font1_out db 'char.mt',0
_font2_in db 'char2.txt',0
_font2_out db 'char2.mt',0

_buf: rb $6500