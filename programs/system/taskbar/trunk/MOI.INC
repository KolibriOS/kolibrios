
load_ini:
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

	mov	[point2],52h ;3d language in Ctrl+Shift
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

	mov	[point2],16h
	mcall	70,get_ini
	xor	eax,eax
	mov	al,[inicont]
	sub	al,31h
	jns	applyKeymap
	ret
;-------------------------------
karu:
	mov	[draw_flag_certainly],1
	dec	[key_r2]
	jns	@f
	mov	[key_r2],1 ; Languages count - 1
@@:
	xor	eax,eax
	mov	al, [key_r2]
	mov	al, [lngcrc1+eax]
	dec	eax
	js	karu
	jmp	applyKeymap
;-------------------------------
alter:
	mov	[draw_flag_certainly],1
	xor	eax,eax
	mov	al, [key_r]
	mov	al, [altlng1+eax]
	dec	eax
	js	@f
applyKeymap:
	cmp	al, 9
	jnc	@f
	push	eax
	shl	eax,7
	lea	eax,[eax*2+eax]
	mov	[point],eax
	mcall	70,get_key
	mcall	21,2,1,current_keymap
	inc	ecx
	mcall	21,,,current_keymap_shift
	inc	ecx
	mcall	21,,,current_keymap_alt
	pop	edx
	inc	edx
	mcall	21,,9
@@:
	ret
;---------------------------------------------------------------
align 4
current_keymap		rb 128
current_keymap_shift	rb 128
current_keymap_alt	rb 128

get_ini:
	dd 0
point2	dd 0
	dd 0
	dd 1
	dd inicont
	db '/sys/settings/lang.ini',0

get_key:
	dd 0
point	dd 0
	dd 0
	dd 384
	dd current_keymap
	db '/sys/settings/keymap.key',0

key_r		db 0
key_r2		db 0
inicont		db 0
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
