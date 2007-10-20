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



