macro .comment
{
init_envmap:	     ; create 512x512 env map
.temp equ word [ebp-2]
	 push	  ebp
	 mov	  ebp,esp
	 sub	  esp,2
	 mov	  edi,envmap
	 fninit

	 mov	  dx,-256
    .ie_ver:
	 mov	  cx,-256
    .ie_hor:
	 mov	  .temp,cx
	 fild	  .temp
	 fmul	  st,st0
	 mov	  .temp,dx
	 fild	  .temp
	 fmul	  st,st0
	 faddp
	 fsqrt
	 mov	  .temp,254
	 fisubr   .temp
	 fmul	  [env_const]
	 fistp	  .temp
	 mov	  ax,.temp

	 or	 ax,ax
	 jge	 .ie_ok1
	 xor	 ax,ax
	 jmp	 .ie_ok2
  .ie_ok1:
	 cmp	 ax,254
	 jle	 .ie_ok2
	 mov	 ax,254
  .ie_ok2:
	 push	 dx
	 mov	 bx,ax
	 mul	 [max_color_b]
	 shr	 ax,8
	 stosb
	 mov	 ax,bx
	 mul	 [max_color_g]
	 shr	 ax,8
	 stosb
	 mov	 ax,bx
	 mul	 [max_color_r]
	 shr	 ax,8
	 stosb
	 pop	 dx

	 inc	 cx
	 cmp	 cx,256
	 jne	 .ie_hor

	 inc	 dx
	 cmp	 dx,256
	 jne	 .ie_ver

	 mov	 esp,ebp
	 pop	 ebp
ret
}
calc_bumpmap:		 ; calculate random bumpmap
;--------------in edi _ pointer to TEX_X x TEX_Y bumpmap
	 push	 edi
	 mov	 ecx,TEXTURE_SIZE
      @@:
	 push	 ecx
	 xor	 ecx,ecx
	 mov	 edx,255
	 call	 random
	 stosb
	 pop	ecx
	 loop	@b

	 pop	edi
	 mov	ecx,4
      .blur:
	 xor	esi,esi
	 mov	edx,TEXTURE_SIZE
	 xor	eax,eax
	 xor	ebx,ebx
      @@:
	 mov	ebp,esi
	 dec	ebp
	 and	ebp,TEXTURE_SIZE
	 mov	al,byte[ebp+edi]

	 mov	ebp,esi
	 inc	ebp
	 and	ebp,TEXTURE_SIZE
	 mov	bl,byte[ebp+edi]
	 add	eax,ebx

	 mov	ebp,esi
	 sub	ebp,TEX_X
	 and	ebp,TEXTURE_SIZE
	 mov	bl,byte[ebp+edi]
	 add	eax,ebx

	 mov	ebp,esi
	 add	ebp,TEX_X
	 and	ebp,TEXTURE_SIZE
	 mov	bl,byte[ebp+edi]
	 add	eax,ebx

	 shr	eax,2
	 mov	byte[esi+edi],al

	 inc	esi
	 dec	edx
	 jnz	@b

	 loop	.blur
ret
random:
;  in  - ecx - min
;        edx - max
;  out - eax - random number
	 mov	bx,[rand_seed]
	 add	bx,0x9248
	 ror	bx,3
	 mov	[rand_seed],bx

	 mov	ax,dx
	 sub	ax,cx
	 mul	bx
	 mov	ax,dx
	 add	ax,cx
	 cwde
ret
rand_seed	dw	?
