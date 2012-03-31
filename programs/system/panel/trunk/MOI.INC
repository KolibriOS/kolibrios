;------------------------------------------------------------------------------	
align 4
load_ini:
	mov	[point2],16h

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[firini],al
	mov	[point2],32h

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[lngcrc1],al
	mov	[point2],42h

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[lngcrc2],al
	mov	[point2],52h

	mcall  70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[lngcrc3],al
	mov	[point2],06eh

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[altlng1],al
	mov	[point2],07eh

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[altlng2],al
	mov	[point2],08eh

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[altlng3],al
	mov	[point2],09eh

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[altlng4],al
	mov	[point2],0aeh

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[altlng5],al
	mov	[point2],0beh

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[altlng6],al
	mov	[point2],0ceh

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[altlng7],al
	mov	[point2],0deh

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[altlng8],al
	mov	[point2],0eeh

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[altlng9],al
	mov	[point2],0feh

	mcall	70,get_ini

	mov	al,[inicont]
	sub	al,30h
	mov	[altlng0],al
	ret
;------------------------------------------------------------------------------	
align 4
fir_lng:
	mov	al,[firini]
	sub	al,1
	mov	dx,384
	mul	dx
	mov	[point],eax
	mcall	70,get_key

	mcall	21,2,1,current_keymap

	inc	ecx
	mcall	21,,,current_keymap_shift

	inc	ecx
	mcall	21,,,current_keymap_alt

	movzx	edx,[firini]
	mcall	21,,9
	ret
;------------------------------------------------------------------------------	
align 4
karu:
	add	[key_r2],1
	cmp	[key_r2],3
	jb	.altes

	mov	[key_r2],0
;--------------------------------------
align 4
.altes:
	cmp	[key_r2],0
	je	.klt_1

	cmp	[key_r2],1
	je	.klt_2

	cmp	[key_r2],2
	je	.klt_3

	jmp	still
;--------------------------------------
align 4
.klt_1:
	mov	al,[lngcrc1]
	sub	al,1
	mov	[key_r],al
	jmp	alted
;--------------------------------------
align 4
.klt_2:
	mov	al,[lngcrc2]
	sub	al,1
	mov	[key_r],al
	jmp	alted
;--------------------------------------
align 4
.klt_3:
	mov	al,[lngcrc3]
	sub	al,1
	mov	[key_r],al
	jmp	alted
;------------------------------------------------------------------------------	
align 4
alter:
	cmp	[key_r],0
	je	.alt_1

	cmp	[key_r],1
	je	.alt_2

	cmp	[key_r],2
	je	.alt_3

	cmp	[key_r],3
	je	.alt_4

	cmp	[key_r],4
	je	.alt_5

	cmp	[key_r],5
	je	.alt_6

	cmp	[key_r],6
	je	.alt_7
;--------------------------------------
align 4
.alt_1:
	mov	al,[altlng1]
	cmp	al,0
	je	.exit

	sub	al,1
	mov	[key_r],al
	jmp	alted
;--------------------------------------
align 4
.alt_2:
	mov	al,[altlng2]
	cmp	al,0
	je	.exit

	sub	al,1
	mov	[key_r],al
	jmp	alted
;--------------------------------------
align 4
.alt_3:
	mov	al,[altlng3]
	cmp	al,0
	je	.exit

	sub	al,1
	mov	[key_r],al
	jmp	alted
;--------------------------------------
align 4
.alt_4:
	mov	al,[altlng4]
	cmp	al,0
	je	.exit

	sub	al,1
	mov	[key_r],al
	jmp	alted
;--------------------------------------
align 4
.alt_5:
	mov	al,[altlng5]
	cmp	al,0
	je	.exit

	sub	al,1
	mov	[key_r],al
	jmp	alted
;--------------------------------------
align 4
.alt_6:
	mov	al,[altlng6]
	cmp	al,0
	je	.exit

	sub	al,1
	mov	[key_r],al
	jmp	alted
;--------------------------------------
align 4
.alt_7:
	mov	al,[altlng7]
	cmp	al,0
	je	.exit

	sub	al,1
	mov	[key_r],al
	jmp	alted
;--------------------------------------
align 4
.exit:
	ret
;------------------------------------------------------------------------------	
align 4
alted:
	cmp	[key_r],0
	je	alt_1

	cmp	[key_r],1
	je	alt_2

	cmp	[key_r],2
	je	alt_3

	cmp	[key_r],3
	je	alt_4

	cmp	[key_r],4
	je	alt_5

	cmp	[key_r],5
	je	alt_6

	cmp	[key_r],6
	je	alt_7

	ret
;------------------------------------------------------------------------------	
align 4
alt_1:
	mov	[point],0	; english
	mcall	70,get_key

	mcall	21,2,1,current_keymap

	inc	ecx
	mcall	21,,,current_keymap_shift
	
	mcall	21,,9,1	; 1 - english
	ret
;------------------------------------------------------------------------------	
align 4
alt_2:
	mov	[point],384	;1536	; finnish
	mcall	70,get_key

	mcall	21,2,1,current_keymap

	inc	ecx
	mcall	21,,,current_keymap_shift

	mcall	21,,9,2	; 2 - finnish
	ret
;------------------------------------------------------------------------------	
align 4
alt_3:
	mov	[point],768	; german
	mcall	70,get_key

	mcall	21,2,1,current_keymap

	inc	ecx
	mcall	21,,,current_keymap_shift

	mcall	21,,9,3	; 3 - german
	ret
;------------------------------------------------------------------------------	
align 4
alt_4:
	mov	[point],1152	;384	; russian
	mcall	70,get_key

	mcall	21,2,1,current_keymap

	inc	ecx
	mcall	21,,,current_keymap_shift
	
	mcall	21,,9,4	; 4 - russian
	ret
;------------------------------------------------------------------------------	
align 4
alt_5:
	mov	[point],1536	;1152	;french
	mcall	70,get_key

	mcall	21,2,1,current_keymap

	inc	ecx
	mcall	21,,,current_keymap_shift

	inc	ecx
	mcall	21,,,current_keymap_alt

	mcall	21,,9,5	; 5 - french
	ret
;------------------------------------------------------------------------------	
align 4
alt_6:
	mov	[point],1920	; estonian
	mcall	70,get_key

	mcall	21,2,1,current_keymap

	inc	ecx
	mcall	21,,,current_keymap_shift
	
	mcall	21,,9,6	; 6 - estonian
	ret
;------------------------------------------------------------------------------	
align 4
alt_7:
	mov	[point],2304	; ukrainian
	mcall	70,get_key

	mcall	21,2,1,current_keymap

	inc	ecx
	mcall	21,,,current_keymap_shift

	mcall	21,,9,7 ; 7 - ukrainian
	ret
;------------------------------------------------------------------------------	
align 4
current_keymap:
	rb 128
;------------------------------------------------------------------------------	
align 4
current_keymap_shift:
	rb 128
;------------------------------------------------------------------------------	
align 4
current_keymap_alt:
	rb 128
;------------------------------------------------------------------------------	
key_r		db 0
key_r2		db 0
key_r3		db 0
inicont		db 0
firini		db 0
lngcrc1		db 0
lngcrc2		db 0
lngcrc3		db 0
altlng1		db 0
altlng2		db 0
altlng3		db 0
altlng4		db 0
altlng5		db 0
altlng6		db 0
altlng7		db 0
altlng8		db 0
altlng9		db 0
altlng0		db 0
;------------------------------------------------------------------------------	
align 4
get_key:	dd 0
point		dd 0
		dd 0
		dd 384
		dd current_keymap
		db 0
		dd key_file
;------------------------------------------------------------------------------	
align 4
get_ini:	dd 0
point2		dd 0
		dd 0
		dd 1
		dd inicont
		db 0
		dd ini_file
;------------------------------------------------------------------------------
key_file: db	  '/rd/1/keymap.key',0
ini_file: db	  '/rd/1/lang.ini',0