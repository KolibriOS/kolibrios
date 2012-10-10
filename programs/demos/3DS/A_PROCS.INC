do_sinus:
   .x	   equ	[ebp-8]
   .y	   equ	[ebp-12]
   .new_y  equ	[ebp-16]
   .temp   equ	[ebp-20]
   push    ebp
   mov	   ebp,esp
   sub	   esp,64
   mov	   dword .x,0
   mov	   dword .y,0
   mov	   esi,[screen_ptr]
   mov	   edi,[Zbuffer_ptr]
   push    edi
   ;  clear Zbuffer temporally used as image buffer
   mov	   ecx,SIZE_X*SIZE_Y
   xor	   eax,eax
   cld
   rep	   stosd
   pop	   edi
;   movzx   eax,[sinus_flag]
;   mov     edx,10
;   mul     edx
;   mov     [sin_amplitude],eax
;   mov     [sin_frq],eax
   fninit
;if Ext = SSE2
;   movups  xmm1,[const0123]   ; xmm1 - init values
;   mov     eax,0x000000ff
;   movd    xmm2,eax
;   shufps  xmm2,xmm2,0        ; xmm2 - mask value
;   mov     eax,4
;   movd    xmm3,eax
;   shufps  xmm3,xmm3,0
 .again:
if  0
   fild    dword .x
   fidiv   [sin_frq]
   fsin
   fimul   [sin_amplitude]
   fiadd   dword .y
   fistp   dword .new_y
else
   fild    dword .x
   fmul    [sin_frq]
   fistp   dword .temp
   mov	   eax, .temp
;   mov     bx, [angle_x]
;   add     bx, [angle_y]
;   movzx   ebx,bx
;   shr     ebx,1       ; change phase
;   add     eax,ebx


   and	   eax, 0x000000ff

;   cdq
 ;  mul     [sin_frq]
;   and      eax,0x000000ff
;   and     ax,0x00ff
;   cwde

   fld	   dword [sin_tab+eax*4]
   fimul   dword [sin_amplitude]
   fiadd   dword .y
   fistp   dword .new_y
end if
   mov	   eax,.new_y
   or	   eax,eax
   jl	   .skip
   cmp	   eax,SIZE_Y
   jg	   .skip
;   mov     edx,SIZE_X
;   mul     edx
   shl	   eax,9
   add	   eax,dword .x
   lea	   ebx,[eax*3]
   mov	   eax,[esi]
   mov	   [edi+ebx],eax
 .skip:
   add	   esi,3
   inc	   dword .x
   cmp	   dword .x,SIZE_X
   jl	   .again
   mov	   dword .x,0
   inc	   dword .y
   cmp	   dword .y,SIZE_Y
   jl	   .again

   ; copy from temporary buffer -> Zbuffer to screen
   mov	   esi,[Zbuffer_ptr]
   mov	   edi,[screen_ptr]
   mov	   ecx,SIZE_X*SIZE_Y*3/4
   cld
   rep	   movsd


   mov	   esp,ebp
   pop	   ebp
ret


draw_dots:
   mov	   esi,[points_translated_ptr]
   movzx   ecx,[points_count_var]
 .drw:
 @@:
   lodsd
   add	   esi,2	   ; skip z
   movzx   ebx,ax
   shr	   eax,16	   ; bx = x , ax = y
   or	   ax,ax
   jl	   @f
   or	   bx,bx
   jl	   @f
   cmp	   ax,SIZE_Y
   jge	   @f
   cmp	   bx,SIZE_X
   jge	   @f
   mov	   edx,SIZE_X	   ; SIZE_X not only power of 2   -> 256,512,...
   mul	   edx
   add	   eax,ebx
   mov	   edi,[screen_ptr]
   lea	   eax,[eax*3]
   add	   edi,eax
   xor	   eax,eax
   not	   eax
   stosd
 @@:
   loop    .drw

ret

do_emboss:
 ;  emboss -  after drawing all,
 ;  transfer screen buffer into bump map
 ;  and draw two bump triangles
 ; *************************************
	mov	esi,screen
	mov	edi,bumpmap2
	mov	ecx,TEXTURE_SIZE/3
	cld
if  Ext=NON
	xor	eax,eax
	xor	bh,bh
	xor	dh,dh
      @@:
	lodsb
	movzx	bx,al
	lodsb
	movzx	dx,al
	lodsb
	add	ax,bx
	add	ax,dx
      ;  cwd
      ;  div     [i3]
 ;;       push    ax
 ;;       pop     bx
 ;;       shr     bx,3
 ;;       shr     ax,2
 ;;       add     ax,bx

	lea	 eax,[eax*5]
	shr	 ax,4

	stosb
	loop	@b
else
	emms
	pxor	      mm1,mm1
	mov	      ebx,0x0000ffff
      @@:
	movd	      mm0,[esi]
	punpcklbw     mm0,mm1
	movq	      mm2,mm0
	psrlq	      mm2,16
	movq	      mm3,mm0
	psrlq	      mm3,32
	paddw	      mm0,mm2
	paddw	      mm0,mm3


	movd	      eax,mm0
	and	      eax,ebx
	lea	      eax,[eax*5]
	shr	      ax,4
	stosb
	add	      esi,3
	loop	      @b

end if
	push	ebp

	push	dword 0 	 ; env coords
	push	word 0
	push	word SIZE_X
	push	word SIZE_Y
	push	dword 0
	push	dword 0 	 ; bump coords
	push	word SIZE_X
	push	word SIZE_Y
	push	word 0
	mov	eax,SIZE_Y
	mov	ebx,SIZE_X*65536+0
	xor	ecx,ecx
	mov	edx,bumpmap2
	mov	esi,envmap
	mov	edi,screen
	call	bump_triangle

	push	dword SIZE_X shl 16 + SIZE_Y	   ; env coords
	push	word 0
	push	word SIZE_X
	push	word SIZE_Y
	push	word 0
	push	dword SIZE_X shl 16 + SIZE_Y	    ; bump coords
	push	word 0
	push	word SIZE_X
	push	word SIZE_Y
	push	word 0
	mov	eax,SIZE_Y
	mov	ebx,SIZE_X * 65536+0
	mov	ecx,SIZE_X shl 16 + SIZE_Y
	mov	edx,bumpmap2
	mov	esi,envmap
	mov	edi,screen
	call	bump_triangle

	pop	ebp
ret
;********************************EMBOSS DONE*******************************


generate_object2:  ; torus
;in  ax - figure number       2=torus, 3=loop, 4=loop
;locals
;   counter dw ?
;   sin     dd ?
;   cos     dd ?
;endl
.counter equ  word[ebp-2]
.sin	 equ  dword[ebp-6]
.cos	 equ  dword[ebp-10]
.sin2	 equ  dword[ebp-14]
.cos2	 equ  dword[ebp-18]
.piD180m3 equ dword[ebp-22]
.cD2	  equ word[ebp-24]
	push  ebp
	mov   ebp,esp
	sub   esp,24

	push  ax

	fninit
	mov	edi,[points_ptr]
	xor	eax,eax
				    ; init seed -> 4   3d points
	mov	dword[edi],-1.0     ; x
	add	edi,4
	stosd			    ; y
	stosd			    ; z
	mov	dword[edi],-0.9     ; x1
	mov	dword[edi+4],0.1    ; y1
	add	edi,8
	stosd			    ; z1
	mov	dword[edi],-0.8
	add	edi,4
	stosd
	stosd
	mov	dword[edi],-0.9     ; x3
	mov	dword[edi+4],-0.1   ; y3
	add	edi,8
	stosd			    ; z3
	mov	[points_count_var],4

	fld	[piD180]
	fidiv	[i3]
	fstp	.piD180m3
	mov	.cD2,5

	pop	ax
	mov	ecx,1
	mov	edx,9
      .next:			  ; calc angle and rotate seed 4 points
	mov	.counter,cx
	mov	ebx,[points_ptr]
	fld	.piD180m3
	fimul	.counter
	fld	st
	fsincos
	fstp	.sin
	fstp	.cos
	fadd	st,st0
	fsincos
	fstp	.sin2
	fstp	.cos2

      .rotor:			       ; next 4
	; rotary y
	fld	dword[ebx]	   ; x
	fld	.sin
	fmul	dword[ebx+8]	   ; z * sinbeta
	fchs
	fld	.cos
	fmul	dword[ebx]	   ; x * cosbeta
	faddp
	fstp	dword[edi]	   ; new x
	fmul	.sin		 ; old x * sinbeta
	fld	.cos
	fmul	dword[ebx+8]	   ; z * cosbeta
	faddp
	dec	dx
	or	dx,dx
	jnz	@f
;        mov     .counter,dx
	fld	st
	fidiv	[i3]
	faddp
    @@:
	fstp	dword[edi+8]	   ; new z
	fld	dword[ebx+4]
	or	dx,dx
	jnz	@f
  ;      fld1
  ;      faddp
;        fld     st
	fadd	st,st0
	fadd	st,st0
;        fxch
;        fimul   [i3]
;        fsin
;        faddp
	mov	dx,9
    @@:
	fstp	dword[edi+4]
	; rotary x
	cmp	al,3
	jl	.end_rot
	fld	dword[edi+4]	;y
	fld	.sin2
	fmul	dword[edi+8]	;z
	fld	.cos2
	fmul	dword[edi+4]	;y
	faddp
	fstp	dword[edi+4]	; new y
	fmul	.sin2	    ; sinbeta * old y
	fchs
	fld	.cos2
	fmul	dword[edi+8]
	faddp
	fstp	dword[edi+8]
	; rotary z
	cmp	al,4
	jl	.end_rot
	fld	dword[edi]	;x
	fld	.sin
	fmul	dword[edi+4]	;y
	fld	.cos
	fmul	dword[edi]	;x
	faddp
	fstp	dword[edi]	;new x
	fmul	.sin	   ; sinbeta * old x
	fchs
	fld	.cos
	fmul	dword[edi+4]	     ; cosbeta * y
	faddp
	fstp	dword[edi+4]	; new y



      .end_rot:

	add	edi,12
	add	ebx,12
	mov	esi,[points_ptr]
	add	esi,12*4
	cmp	ebx,esi
	jl	.rotor

	add	[points_count_var],4
	add	cx,18
	cmp	cx,(18*21*3)+1
	jle	.next

	mov	edi,[triangles_ptr]
	mov	ax,4
	mov	bx,4+4
	mov	[triangles_count_var],164*3   ;140

	mov	cx,80*3  ;68
      @@:
	stosw		      ;----
	mov	[edi],bx      ;    |
	add	edi,2	      ;    |
	inc	ax	      ;    |
	stosw		      ;    |repeat 4 times

	mov	[edi],bx      ;    |
	inc	bx
	add	edi,2
	stosw		      ;    |
	mov	[edi],bx      ;    |
	add	edi,2	      ;----
	loop	 @b


	mov	dword[edi],-1  ; < - end mark
	mov	  [culling_flag],0

	mov	esp,ebp
	pop	ebp

ret


generate_object3:  ; heart
;locals
;   counter dw ?
;   sin     dd ?
;   cos     dd ?
;endl
.counter equ  word[ebp-2]
.sin	 equ  dword[ebp-6]
.cos	 equ  dword[ebp-10]
.sin2	 equ  dword[ebp-14]
.cos2	 equ  dword[ebp-18]
.piD180m3 equ dword[ebp-22]
.cD2	  equ word[ebp-24]
	push  ebp
	mov   ebp,esp
	sub   esp,24

	fninit
	mov	edi,[points_ptr]
	xor	eax,eax
			       ; init seed -> eight   3d points
	mov	dword[edi],2.0
	add	edi,4
	stosd
	stosd

	mov	dword[edi],2.0
	mov	dword[edi+4],-0.5
	add	edi,8
	stosd

	mov	dword[edi],1.5
	mov	dword[edi+4],-1.5
	add	edi,8
	stosd
	mov	dword[edi],1.0
	mov	dword[edi+4],-2.0
	add	edi,8
	stosd

	stosd
	mov	dword[edi],-2.5
	add	edi,4
	stosd

	mov	[points_count_var],5

	mov	ecx,1
      .next:			  ; calc angle and rotate seed 4 points
	mov	.counter,cx
	mov	ebx,[points_ptr]
	fld	[piD180]
	fimul	.counter
	fsincos
	fstp	.sin
	fstp	.cos

      .rotor:			       ; next 4
	; rotary y
	fld	dword[ebx]	   ; x
	fld	.sin
	fmul	dword[ebx+8]	   ; z * sinbeta
	fchs
	fld	.cos
	fmul	dword[ebx]	   ; x * cosbeta
	faddp
	fidiv	[i3]
	fstp	dword[edi]	   ; new x
	fmul	.sin		   ; old x * sinbeta
	fld	.cos
	fmul	dword[ebx+8]	   ; z * cosbeta
	faddp
	fstp	dword[edi+8]	   ; new z

	fld	dword[ebx+4]   ;y
	fstp	dword[edi+4]


      .end_rot:

	add	edi,12
	add	ebx,12
	mov	esi,[points_ptr]
	add	esi,12*5
	cmp	ebx,esi  ;real_points + (12*5)
	jl	.rotor

	add	[points_count_var],5
	add	cx,18
	cmp	cx,(18*21)+1
	jle	.next
;last points

	xor	eax,eax

	mov	dword[edi],0.22
	mov	dword[edi+4],0.77
	mov	dword[edi+8],1.25
	add	edi,12

	mov	dword[edi],0.22
	mov	dword[edi+4],0.77
	mov	dword[edi+8],-1.25
	add	edi,12
	stosd

	add	[points_count_var],2

; init triangles list

	mov	edi,[triangles_ptr]
	mov	ax,5
	mov	bx,5+5
	mov	[triangles_count_var],204

	mov	cx,100
      @@:
	stosw		      ;----
	mov	[edi],bx      ;    |
	add	edi,2	      ;    |
	inc	ax	      ;    |
	stosw		      ;    |repeat

	mov	[edi],bx      ;    |
	inc	bx
	add	edi,2
	stosw		      ;    |
	mov	[edi],bx      ;    |
	add	edi,2	      ;----
	loop	 @b

	mov	ax,5
	mov	bx,[points_count_var]
	sub	bx,2
	mov	dl,2
    .nx:
	mov	cx,5
	add	[triangles_count_var],cx
    @@:
	stosw
	add	ax,5
	stosw
	mov	word[edi],bx
	add	edi,2
	loop	@b

	cmp	dl,1
	je	@f

	inc	bx
	jmp	.lab
     @@:
	dec	bx
     .lab:
	mov	cx,5
	add	[triangles_count_var],cx
     @@:
	stosw
	add	ax,5
	stosw
	mov	word[edi],bx
	add	edi,2
	loop	@b

	dec	dl
	or	dl,dl
	jnz	.nx

	sub	ax,25
	stosw
	sub	ax,50
	stosw
	mov	word[edi],bx
	add	edi,2

	stosw
	add	ax,50
	stosw
	inc	bx
	mov	word[edi],bx
	add	edi,2
	add	[triangles_count_var],2

	mov	dword[edi],-1  ; < - end mark
	mov	[culling_flag],0

	mov	esp,ebp
	pop	ebp

ret
