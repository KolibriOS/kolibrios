x3d equ 0
y3d equ 2
z3d equ 4
vec_x equ 0
vec_y equ 4
vec_z equ 8
; 3d point - triple integer word coordinate
; vector   - triple float dword coordinate
;----------------------in: --------------------------------
;------------------------ esi - pointer to 1st 3d point ---
;------------------------ edi - pointer to 2nd 3d point ---
;------------------------ ebx - pointer to result vector --
;---------------------- out : none ------------------------
if 0
make_vector:
	fninit
	fild	word[edi+x3d]		     ;edi+x3d
	fisub	word[esi+x3d]		     ;esi+x3d
	fstp	dword[ebx+vec_x]

	fild	word[edi+y3d]
	fisub	word[esi+y3d]
	fstp	dword[ebx+vec_y]

	fild	word[edi+z3d]
	fisub	word[esi+z3d]
	fstp	dword[ebx+vec_z]

ret
end if
make_vector_r:
	fninit
	fld	dword[edi]		  ;edi+x3d
	fsub	dword[esi]		  ;esi+x3d
	fstp	dword[ebx+vec_x]

	fld	dword[edi+4]
	fsub	dword[esi+4]
	fstp	dword[ebx+vec_y]

	fld	dword[edi+8]
	fsub	dword[esi+8]
	fstp	dword[ebx+vec_z]

ret
;---------------------- in: -------------------------------
;--------------------------- esi - pointer to 1st vector --
;--------------------------- edi - pointer to 2nd vector --
;--------------------------- ebx - pointer to result vector
;---------------------- out : none
cross_product:
	fninit
	fld	dword [esi+vec_y]
	fmul	dword [edi+vec_z]
	fld	dword [esi+vec_z]
	fmul	dword [edi+vec_y]
	fsubp	;st1 ,st
	fstp	dword [ebx+vec_x]

	fld	dword [esi+vec_z]
	fmul	dword [edi+vec_x]
	fld	dword [esi+vec_x]
	fmul	dword [edi+vec_z]
	fsubp	;st1 ,st
	fstp	dword [ebx+vec_y]

	fld	dword [esi+vec_x]
	fmul	dword [edi+vec_y]
	fld	dword [esi+vec_y]
	fmul	dword [edi+vec_x]
	fsubp	;st1 ,st
	fstp	dword [ebx+vec_z]
ret
;----------------------- in: ------------------------------
;---------------------------- edi - pointer to vector -----
;----------------------- out : none
normalize_vector:
	fninit
	fld	dword [edi+vec_x]
	fmul	st, st
	fld	dword [edi+vec_y]
	fmul	st, st
	fld	dword [edi+vec_z]
	fmul	st, st
	faddp	st1, st
	faddp	st1, st
	fsqrt

	ftst
	fstsw ax
	sahf
	jnz	@f

	fst	dword [edi+vec_x]
	fst	dword [edi+vec_y]
	fstp	dword [edi+vec_z]
	ret
      @@:
	fld st
	fld st
	fdivr dword [edi+vec_x]
	fstp  dword [edi+vec_x]
	fdivr dword [edi+vec_y]
	fstp  dword [edi+vec_y]
	fdivr dword [edi+vec_z]
	fstp  dword [edi+vec_z]
ret
;------------------in: -------------------------
;------------------ esi - pointer to 1st vector
;------------------ edi - pointer to 2nd vector
;------------------out: ------------------------
;------------------ st0 - dot-product
dot_product:
	fninit
	fld	dword [esi+vec_x]
	fmul	dword [edi+vec_x]
	fld	dword [esi+vec_y]
	fmul	dword [edi+vec_y]
	fld	dword [esi+vec_z]
	fmul	dword [edi+vec_z]
	faddp
	faddp
ret

; DOS version Coded by Mikolaj Felix aka Majuma
; mfelix@polbox.com
; www.majuma.xt.pl
; into FASM translation by Macgub
init_sincos_tab:
.counter   equ	dword [ebp-4]  ; cur angle

     push	ebp
     mov	ebp,esp

     xor	eax,eax
     push	eax	       ; init .counter
     mov	edi,cos_tab
     mov	esi,sin_tab
     mov	ecx,256
     fninit

     fld	.counter
  @@:
     fld	st
     fsincos
     fstp	dword [edi]
     fstp	dword [esi]
;     fadd       [piD180]
     fadd	[piD128]
     add	esi,4
     add	edi,4
     loop	@b
     ffree	st

     mov	esp,ebp
     pop	ebp
ret
;------
; esi - offset (pointer) to angles, edi offset to 3x3 matrix
make_rotation_matrix:
   .sinx   equ dword[ebp-4]
   .cosx   equ dword[ebp-8]
   .siny   equ dword[ebp-12]
   .cosy   equ dword[ebp-16]
   .sinz   equ dword[ebp-20]
   .cosz   equ dword[ebp-24]
     push      ebp
     mov       ebp,esp
     sub       esp,24

     movzx     ebx,word[esi]
     shl       ebx,2
     mov       eax,dword[sin_tab+ebx]
     mov       .sinx,eax
     mov       edx,dword[cos_tab+ebx]
     mov       .cosx,edx

     movzx     ebx,word[esi+2]
     shl       ebx,2
     mov       eax,dword[sin_tab+ebx]
     mov       .siny,eax
     mov       edx,dword[cos_tab+ebx]
     mov       .cosy,edx

     movzx     ebx,word[esi+4]
     shl       ebx,2
     mov       eax,dword[sin_tab+ebx]
     mov       .sinz,eax
     mov       edx,dword[cos_tab+ebx]
     mov       .cosz,edx

     fninit
     fld       .cosy
     fmul      .cosz
     fstp      dword[edi]

     fld       .sinx
     fmul      .siny
     fmul      .cosz
     fld       .cosx
     fmul      .sinz
     fchs
     faddp
     fstp      dword[edi+12]

     fld       .cosx
     fmul      .siny
     fmul      .cosz
     fld       .sinx
     fmul      .sinz
     faddp
     fstp      dword[edi+24]

     fld       .siny
     fmul      .sinz
     fstp      dword[edi+4]

     fld       .sinx
     fmul      .siny
     fmul      .sinz
     fld       .cosx
     fmul      .cosz
     faddp
     fstp      dword[edi+16]

     fld       .cosx
     fmul      .siny
     fmul      .sinz
     fld       .sinx
     fchs
     fmul      .cosz
     faddp
     fstp      dword[edi+28]

     fld       .siny
     fchs
     fstp      dword[edi+8]

     fld       .cosy
     fmul      .sinx
     fstp      dword[edi+20]

     fld       .cosx
     fmul      .cosy
     fstp      dword[edi+32]

     mov       esp,ebp
     pop       ebp
ret
;---------------------
;  in:  esi - ptr to points(normals], each point(normal) coeficient as dword
;       edi - ptr to rotated points(normals)
;       ebx - ptr to 3x3 (9 dwords, 36 bytes) rotation matrix
;       ecx - number of points(normals)
rotary:
if Ext<SSE
    fninit
 .again:

    fld     dword[esi]
    fmul    dword[ebx]
    fld     dword[esi+4]
    fmul    dword[ebx+12]
    faddp
    fld     dword[esi+8]
    fmul    dword[ebx+24]
    faddp
    fstp    dword[edi]


    fld     dword[esi+4]
    fmul    dword[ebx+16]
    fld     dword[esi]
    fmul    dword[ebx+4]
    faddp
    fld     dword[esi+8]
    fmul    dword[ebx+28]
    faddp
    fstp    dword[edi+4]


    fld     dword[esi+8]
    fmul    dword[ebx+32]
    fld     dword[esi]
    fmul    dword[ebx+8]
    fld     dword[esi+4]
    fmul    dword[ebx+20]
    faddp
    faddp
    fstp    dword[edi+8]


    add     esi,12
    add     edi,12
    loop    .again
    mov     [edi],dword -1
else
;   Copyright (C) 1999-2001  Brian Paul
;   Copyright (C)            Maciej Guba
;---------------------
;  in:  esi - ptr to points(normals], each point(normal) coeficient as dword
;       edi - ptr to rotated points(normals)
;       ebx - ptr to 3x3 (9 dwords, 36 bytes) rotation matrix
;       ecx - number of points(normals)
;align 32
    movups   xmm4,[ebx]
    movups   xmm5,[ebx+12]
    movups   xmm6,[ebx+24]
;align 32
  .again:
    movss    xmm0,dword[esi]
    shufps   xmm0,xmm0,0
    mulps    xmm0,xmm4

    movss    xmm1,dword[esi+4]
    shufps   xmm1,xmm1,0
    mulps    xmm1,xmm5

    movss    xmm2,dword[esi+8]
    shufps   xmm2,xmm2,0
    mulps    xmm2,xmm6

    addps    xmm0,xmm1
    addps    xmm0,xmm2

    movups   [edi],xmm0

    add      esi,12
    add      edi,12
    dec      ecx
    jne      .again
    mov      [edi],dword -1
end if
ret
;----------------------------------------------
;  esi - pointer to 3x3 matrix
add_scale_to_matrix:
     fninit
     fld     [rsscale]
     fld     dword[esi] 	   ;-----
     fmul    st,st1
     fstp    dword[esi]
     fld     dword[esi+12]	     ; x scale
     fmul    st,st1
     fstp    dword[esi+12]
     fld     dword[esi+24]
     fmul    st,st1
     fstp    dword[esi+24]	   ;------

     fld     dword[esi+4]	   ;-----
     fmul    st,st1
     fstp    dword[esi+4]
     fld     dword[esi+16]	      ; y scale
     fmul    st,st1
     fstp    dword[esi+16]
     fld     dword[esi+28]
     fmul    st,st1
     fstp    dword[esi+28]	   ;------


     fld     dword[esi+8]	   ;-----
     fmul    st,st1
     fstp    dword[esi+8]
     fld     dword[esi+20]		; z scale
     fmul    st,st1
     fstp    dword[esi+20]
     fld     dword[esi+32]
     fmulp    st1,st
     fstp    dword[esi+32]	   ;------

ret

;in   esi - offset to 3d points  (point as 3 dwords float)
;     edi - offset to 2d points  ( as 3 words integer)
;     ecx - number of points
translate_points:  ; just convert into integer; z coord still needed
    fninit
  .again:
    fld    dword[esi+8]
 ;   fmul   [rsscale]
    fist   word[edi+4]

    fisub  [zobs]
    fchs

    fld    dword[esi]
;    fmul   [rsscale]
    fisub  [xobs]
    fimul  [zobs]
    fdiv   st0,st1

    fiadd  [xobs]
    fiadd  [vect_x]
    fistp  word[edi]

    fld    dword[esi+4]
;    fmul   [rsscale]
    fisub  [yobs]
    fimul  [zobs]
    fdivrp  ;   st0,st1

    fiadd  [yobs]
    fiadd  [vect_y]
    fistp  word[edi+2]

    add    esi,12
    add    edi,6
    dec    ecx
    jnz    .again

ret
