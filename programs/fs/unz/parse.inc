; ret:
; 0 - not found param
; 1 - found param
; 2 - error parsing

proc getLastParam	;последний параметр пишет в fInp
	xor	al, al		;to end string
	mov	edi, params
	mov	ecx, 4096
	repne	scasb
	dec	edi
	dec	edi
	test	ecx, ecx
	jz	errorParsing

	mov	eax, ecx       ; - spaces
	mov	ecx, 4096
	sub	ecx, eax
	mov	al, ' '
	std
	repe	scasb
	cld
	inc	edi
	inc	ecx

	cmp	edi, params
	je	.noparams

	cmp	[edi], byte '"'
	je	.quotation

	mov	al, ' ' 	;find previous space or begin string
	std
	repne	scasb
	cld
	inc	edi
	inc	edi

	mov	esi, edi	;copy
	mov	ebx, edi
	mov	edi, fInp

	xor	ecx, ecx
     @@:lodsb
	cmp	al, ' '
	je	@f
	test	al, al
	je	@f
	stosb
	inc	ecx
	jmp	@b
     @@:
	mov	[edi], byte 0
	inc	edi

	mov	edi, ebx	;clear
	mov	al, ' '
	rep stosb

	jmp	.end
 .quotation:
	dec	edi
	mov	al, '"'
	std
	repne	scasb
	cld
	add	edi, 2
	cmp	[edi-1], byte '"'
	jne	.err

	mov	esi, edi
	mov	ebx, edi
	mov	edi, fInp
	xor	ecx, ecx
     @@:lodsb
	cmp	al, '"'
	je	@f
	test	al, al
	je	@f
	stosb
	inc	ecx
	jmp	@b
     @@:
	mov	[edi], byte 0
	inc	edi

	mov	edi, ebx
	dec	edi
	mov	al, ' '
	add	ecx, 2
	rep stosb

.end:
	mov	eax, 1
	ret
.err:
	mov	eax, 2
	ret
.noparams:
	xor	eax, eax
endp

;заполняе pathOut
proc getParamOutPath
locals
retrn rd 0
endl
	mov	[retrn], 0
	mov	ebx, params

  .find_o:
	cmp	[ebx], word '-o'
	jne	.find_o2

	mov	edx, ebx
	lea	esi, [ebx+3]

   @@:				;skiping spaces
	cmp	esi, params+4096
	jae	.errorParsing
	cmp	byte[esi], ' '
	jne	@f
	inc	esi
	jmp	@b
   @@:
				;copying        ; -o "file"
	mov	ecx, 3
	cmp	byte[esi], '"'
	jne	._notspace
	inc	esi
	add	ecx, 2
	mov	edi, pathOut
	mov	[retrn], 1
    @@: lodsb
	cmp	al, '"'
	je	.clear
	stosb
	inc	ecx
	cmp	esi, params+1024
	jae	errorParsing
	jmp	@b

     ._notspace:		;copying        ; -o file
	mov	edi, pathOut
    @@: lodsb
	stosb
	inc	ecx
	cmp	al, ' '
	je	.clear
	cmp	esi, params+1024
	jae	errorParsing
	jmp	@b

     .clear:			;cleaning param
	mov	[edi], byte 0
	inc	edi
	mov	[retrn], 1
      @@:
	mov	byte[edx], ' '
	inc	edx
	loop	@b
	jmp	@f

  .find_o2:
	inc	ebx
	cmp	ebx, params+1024
	jae	@f
	cmp	byte[ebx], 0
	je	@f
	jmp	.find_o
  @@:



	mov	eax, [retrn]
	ret
.errorParsing:
	mov	eax, 2
	ret
endp


proc getParam key:word
locals
retrn rd 0
endl
	mov	[retrn], 0
	mov	ebx, params

  .find_o:
	mov	ax, [key]
	cmp	[ebx], ax
	jne	.find_o2

	mov	edx, ebx
	lea	esi, [ebx+3]

   @@:				;skiping spaces
	cmp	esi, params+4096
	jae	.errorParsing
	cmp	byte[esi], ' '
	jne	@f
	inc	esi
	jmp	@b
   @@:

				;variant1: copying   ; -f "file"
	mov	ecx, 3
	cmp	byte[esi], '"'
	jne	._notspace
	inc	esi
	add	ecx, 2
	mov	edi, [endPointer]
	mov	[retrn], 1
    @@: lodsb
	cmp	al, '"'
	je	.clear
	stosb
	inc	ecx
	cmp	esi, params+4096
	jae	errorParsing
	jmp	@b

     ._notspace:		;variant2: copying    ; -f file
	mov	edi, [endPointer]
    @@: lodsb
	stosb
	inc	ecx
	cmp	al, ' '
	je	.clear
	cmp	esi, params+4096
	jae	errorParsing
	jmp	@b


     .clear:			;cleaning param
	mov	[edi], byte 0
	inc	edi
	mov	[endPointer], edi
	mov	[retrn], 1
      @@:
	mov	byte[edx], ' '
	inc	edx
	loop	@b

	jmp	@f

  .find_o2:
	inc	ebx
	cmp	ebx, params+4096
	jae	@f
	cmp	byte[ebx], 0
	je	@f
	jmp	.find_o
  @@:



	mov	eax, [retrn]
	ret
.errorParsing:
	mov	eax, 2
	ret
endp
