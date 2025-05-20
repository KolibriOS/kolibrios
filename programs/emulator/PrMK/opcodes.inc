 align 4

OPCODEtable:

dd _00,_00,_00,_00,_00,_00,_00,_00 ;00-07
dd _00,_00,_0A,_0B,_0C,_0D,_0E,_0F ;08-0f
dd _10,_11,_12,_13,_14,_15,_16,_17 ;10-17
dd _18,_19,_1A,_1B,_1C,_1D,_1E,_1f ;18-1f
dd _20,_21,_22,_23,_24,_25,_1f,_1f ;20-27
dd _1f,_1f,_1f,_1f,_1f,_1f,_1f,_1f ;28-2f
dd _1f,_31,_32,_1f,_34,_35,_36,_1f ;30-37
dd _1f,_1f,_1f,_3B,_1f,_1f,_1f,_1f ;38-3f
times 16 dd _40 ;40-4f
dd _50,_51,_52,_53,_54,_1f,_1f,_57 ;50-57
dd _58,_59,_5A,_5B,_5C,_5D,_5E,_1f ;58-5f
times 16 dd _60 ;60-6f
times 16 dd _70 ;70-7f
times 16 dd _80 ;80-8f
times 16 dd _90 ;90-9f
times 16 dd _A0 ;a0-af
times 16 dd _B0 ;b0-bf
times 16 dd _C0 ;c0-cf
times 16 dd _D0 ;d0-df
times 16 dd _E0 ;e0-ef
times 16 dd _1f ;f0-ff


move1 db 0
move2 db 0
move1dot db 0

_1f: ret

;*************************************

_3B:
	mcall	3
	shl	eax,16
	shr	eax,16
	mov	[perem],eax
	fild	[perem]
	mcall	26,9
	shl	eax,16
	shr	eax,18
	mov	[perem],eax
	fild	[perem]
	fcom	st1
	jnc	@f
	fdivrp
	mov	[perem],10
	fild	[perem]
	fdivp
	jmp	.1
   @@:	fdivp
   .1:	fld	qword [S.Z]
	fstp	qword [S.T]
	fld	qword [S.Y]
	fstp	qword [S.Z]
	fld	qword [S.X]
	fst	qword [S.Y]
	fstp	qword [S.X1]
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],4
	ret
	
_36:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.Y]
	fld	qword [S.X]
	fst	qword [S.X1]

	fcom	st1
	fstsw	ax
	sahf
	jc	@f
	fstp	qword [S.X]
	fstp	st0
	jmp	.1
  @@:	fstp	st0
	fstp	qword [S.X]
  .1:
	fld	qword [S.Z]
	fstp	qword [S.Y]
	fld	qword [S.T]
	fstp	qword [S.Z]
;	fldz
;	fstp	qword [S.T]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	ret

_35:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	fldz
	fcomp	st1
	fstsw	ax
	sahf
	jz	@f
	jc	@f
	frndint
	fcom	qword [S.X]
	fstsw	ax
	sahf
	jnc	.0
	fld1
	faddp
	jmp	.0
  @@:	frndint
	fcom	qword [S.X]
	fstsw	 ax
	sahf
	jc	.0
	jz	.0
	fld1
	fsubp
  .0:	fld	qword [S.X]
	fsubrp
	fst	qword [S.X]
	call	ftos
	call	draw_string
	call	draw_stack
	bts	[flags],10
	btr	[flags],6
	ret


_34:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	fldz
	fcomp	st1
	fstsw	ax
	sahf
	jz	@f
	jc	@f
	frndint
	fcom	qword [S.X]
	fstsw	ax
	sahf
	jnc	.0
	fld1
	faddp
	jmp	.0
  @@:	frndint
	fcom	qword [S.X]
	fstsw	 ax
	sahf
	jc	.0
	jz	.0
	fld1
	fsubp
  .0:	fst	qword [S.X]
	call	ftos
	call	draw_string
	call	draw_stack
	bts	[flags],10
	btr	[flags],6
	ret

_32:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fstp	qword [S.X1]
	fldz
	fcomp	qword [S.X]
	fstsw	 ax
	sahf
	fld1
	jz	@f
	jc	@f
	fchs
  @@:	fst	qword [S.X]
	call	ftos
	call	draw_string
	call	draw_stack
	bts	[flags],10
	btr	[flags],6
	ret

_31:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	fabs
	fst	qword [S.X]
	call	ftos
	call	draw_string
	call	draw_stack
	bts	[flags],10
	btr	[flags],6
	ret

;************************************
_E0:
	;x=0 ;0!=x
	inc	[schk]
  @@:	fldz
	fcomp	qword [S.X]
	fstsw	 ax
	sahf
	jnz	@f
	ret
  @@:	sub	eax,OPCODEtable
	shr	eax,2
	sub	eax,0xE0;90
	shl	eax,3
	add	eax,R.0
	fld	qword [eax]
	cmp	ebx,6
	jge	@f
	fld1
	cmp	ebx,4
	jge	.2
	fsubrp
	jmp	@f
  .2:	faddp
  @@:	fst	qword [eax]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	mov	[schk],al
	ret

_D0:
	inc	[schk]

	sub	eax,OPCODEtable
	shr	eax,2
	sub	eax,0xD0
	mov	ebx,eax
	shl	eax,3
	add	eax,R.0
	fld	qword [eax]
	cmp	ebx,6
	ja	@f
	fld1
	cmp	ebx,4
	jge	.2
	fsubrp
	jmp	@f
  .2:	faddp
  @@:	frndint
	fst	qword [eax]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	shl	eax,31-4
	shr	eax,31-4
	shl	eax,3
	add	eax,R.0

	fld	qword [eax]

	fld	qword [S.Z]
	fstp	qword [S.T]
	fld	qword [S.Y]
	fstp	qword [S.Z]
	fld	qword [S.X]
	fst	qword [S.Y]
	fstp	qword [S.X1]

	fstp	qword [S.X]

	call	draw_registers
	call	draw_stack
	bts	[flags],10
	btr	[flags],6
	ret

_C0:
	;x<0 ;0<x
	inc	[schk]
  @@:	fldz
	fcomp	qword [S.X]
	fstsw	 ax
	sahf
	jz	@f
	jc	@f
	ret
  @@:	sub	eax,OPCODEtable
	shr	eax,2
	sub	eax,0xC0
	shl	eax,3
	add	eax,R.0
	fld	qword [eax]
	cmp	ebx,6
	jge	@f
	fld1
	cmp	ebx,4
	jge	.2
	fsubrp
	jmp	@f
  .2:	faddp
  @@:	fst	qword [eax]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	mov	[schk],al
	ret

_B0:
	inc	[schk]

	sub	eax,OPCODEtable
	shr	eax,2
	sub	eax,0xB0
	shl	eax,3
	add	eax,R.0
	fld	qword [eax]
	cmp	ebx,6
	jge	@f
	fld1
	cmp	ebx,4
	jge	.2
	fsubrp
	jmp	@f
  .2:	faddp
  @@:	fst	qword [eax]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	mov	ecx,16
	xor	edx,edx
	div	ecx
	mov	eax,edx
	shl	eax,3
	add	eax,R.0
	fld	qword [S.X]
	fstp	qword [eax]

	bts	[flags],10
	btr	[flags],6
	ret

_A0:
	inc	[schk]
	mov	cl,[schk]

	sub	eax,OPCODEtable
	shr	eax,2
	sub	eax,0xA0
	shl	eax,3
	add	eax,R.0
	fld	qword [eax]
	cmp	ebx,6
	jge	@f
	fld1
	cmp	ebx,4
	jge	.2
	fsubrp
	jmp	@f
  .2:	faddp
  @@:	fst	qword [eax]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	mov	[schk],al

	mov	al,[RS+3]
	mov	[RS+4],al
	mov	al,[RS+2]
	mov	[RS+3],al
	mov	al,[RS+1]
	mov	[RS+2],al
	mov	al,[RS+0]
	mov	[RS+1],al

	mov	[RS+0],cl
	bts	[flags],10
	btr	[flags],6
	ret

_90:
	;x>=0 ;0>x
	inc	[schk]
  @@:	fldz
	fcomp	qword [S.X]
	fstsw	 ax
	sahf
	jp	@f
	jz	@f
	jc	@f
	jmp	.1
  @@:	ret
  .1:	sub	eax,OPCODEtable
	shr	eax,2
	sub	eax,0x90
	shl	eax,3
	add	eax,R.0
	fld	qword [eax]
	cmp	ebx,6
	jge	@f
	fld1
	cmp	ebx,4
	jge	.2
	fsubrp
	jmp	@f
  .2:	faddp
  @@:	fst	qword [eax]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	mov	[schk],al
	ret

_80:
	inc	[schk]
	sub	eax,OPCODEtable
	shr	eax,2
	sub	eax,0x80
	shl	eax,3
	add	eax,R.0
	fld	qword [eax]
	cmp	ebx,6
	jge	@f
	fld1
	cmp	ebx,4
	jge	.2
	fsubrp
	jmp	@f
  .2:	faddp
  @@:	fst	qword [eax]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	mov	[schk],al
	ret

_70:	;x!=0 ;x=0
	inc	[schk]
  @@:	fldz
	fcomp	qword [S.X]
	fstsw	 ax
	sahf
	jz	@f
	ret
  @@:	sub	eax,OPCODEtable
	shr	eax,2
	sub	eax,0x70
	mov	ebx,eax
	shl	eax,3
	add	eax,R.0
	fld	qword [eax]
	cmp	ebx,6
	jge	@f
	fld1
	cmp	ebx,4
	jge	.2
	fsubrp
	jmp	@f
  .2:	faddp
  @@:	fst	qword [eax]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	mov	[schk],al
	ret

_5D:
	inc	[schk]
	fld	qword [R.0]
	fld1
	fsubp
	fst	qword [R.0]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	test	eax,eax
	jz	@f
	movzx	eax,[schk]
	add	eax,PMEM
	mov	al,[eax]
	mov	[schk],al
	jmp	.1
  @@:	inc	[schk]
  .1:	call	draw_registers
	bts	[flags],10
	btr	[flags],6
	ret

_5B:
	inc	[schk]
	fld	qword [R.1]
	fld1
	fsubp
	fst	qword [R.1]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	test	eax,eax
	jz	@f
	movzx	eax,[schk]
	add	eax,PMEM
	mov	al,[eax]
	mov	[schk],al
	jmp	.1
  @@:	inc	[schk]
  .1:	call	draw_registers
	bts	[flags],10
	btr	[flags],6
	ret


_5A:
	inc	[schk]
	fld	qword [R.3]
	fld1
	fsubp
	fst	qword [R.3]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	test	eax,eax
	jz	@f
	movzx	eax,[schk]
	add	eax,PMEM
	mov	al,[eax]
	mov	[schk],al
	jmp	.1
  @@:	inc	[schk]
  .1:	call	draw_registers
	bts	[flags],10
	btr	[flags],6
	ret


_58:	;F,L2
	inc	[schk]
	fld	qword [R.2]
	fld1
	fsubp
	fst	qword [R.2]
	fabs
	fistp	[perem]
	mov	eax,[perem]
	test	eax,eax
	jz	@f
	movzx	eax,[schk]
	add	eax,PMEM
	mov	al,[eax]
	mov	[schk],al
	jmp	.1
  @@:	inc	[schk]
  .1:	call	draw_registers
	bts	[flags],10
	btr	[flags],6
	ret



_5E:	;x=0
;       bt      [flags],0
;       jnc     @f
	inc	[schk]
  @@:	fldz
	fcomp	qword [S.X]
	fstsw	 ax
	sahf
	jnz	@f
	inc	[schk]
	ret
  @@:	movzx	eax, byte [schk]
	add	eax,PMEM
	mov	al,[eax]
	mov	[schk],al
	ret

_5C:	;x<0
;       bt      [flags],0
;       jnc     @f
	inc	[schk]
  @@:	fldz
	fcomp	qword [S.X]
	fstsw	 ax
	sahf
	jc	@f
	jz	@f
	inc	[schk]
	ret
  @@:	movzx	eax, byte [schk]
	add	eax,PMEM
	mov	al,[eax]
	mov	[schk],al
	ret

_59:	;x>=0
;       bt      [flags],0
;       jnc     @f
	inc	[schk]
  @@:	fldz
	fcomp	qword [S.X]
	fstsw	 ax
	sahf
	jp	@f
	jz	@f
	jc	@f
	jmp	.1
  @@:	inc	[schk]
	ret
  .1:	movzx	eax, byte [schk]
	add	eax,PMEM
	mov	al,[eax]
	mov	[schk],al
	ret

_57:  ;x!=0
;       bt      [flags],0
;       jnc     @f
	inc	[schk]
  @@:	fldz
	fcomp	qword [S.X]
	fstsw	 ax
	sahf
	jz	@f
	inc	[schk]
	ret
  @@:	movzx	eax, byte [schk]
	add	eax,PMEM
	mov	al,[eax]
	mov	[schk],al
	ret

_54:
	inc	[schk]
	ret

_53:	bt	[flags],0
	jnc	@f
	inc	[schk]
	mov	al,[RS+3]
	mov	[RS+4],al
	mov	al,[RS+2]
	mov	[RS+3],al
	mov	al,[RS+1]
	mov	[RS+2],al
	mov	al,[RS+0]
	mov	[RS+1],al
	mov	al,[schk]
	inc	al
	mov	[RS+0],al
	movzx	eax,[schk]
	add	eax,PMEM
	mov	al,[eax]
	mov	[schk],al
	bts	[flags],10
	btr	[flags],6
	ret
  @@:	bts	[flags],0
	movzx	eax,[schk]
	add	eax,PMEM
	movzx	eax,byte [eax]
	shl	eax,2
	add	eax,OPCODEtable
	mov	ebx,[eax]
	call	ebx
	btr	[flags],0
	call	draw_schk
	call	draw_prog
ret

_52:	inc	[schk]
	bt	[flags],0
	jnc	@f
	mov	al,[RS+0]
	mov	[schk],al
	mov	al,[RS+1]
	mov	[RS+0],al
	mov	al,[RS+2]
	mov	[RS+1],al
	mov	al,[RS+3]
	mov	[RS+2],al
	mov	al,[RS+4]
	mov	[RS+3],al
	mov	byte [RS+4],0
	ret
  @@:	mov	[schk],0
	call draw_prog
	call draw_schk
ret


_51:
;       bt      [flags],0
;       jnc     @f
	inc	[schk]
  @@:	movzx	eax, byte [schk]
	add	eax,PMEM
	mov	al,[eax]
	mov	[schk],al
	ret


_50:
	btc	[flags],0
	jc	@f
	ret
  @@:	inc	[schk]
	fld	qword [S.X]
	call	ftos
	call	draw_string
	call	draw_registers
	bts	[flags],10
	btr	[flags],6
	ret

_60:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	sub	eax,OPCODEtable
	shr	eax,2
	sub	eax,0x60
	shl	eax,3
	add	eax,R.0
	fld	qword [eax]
	fld	qword [S.Z]
	fstp	qword [S.T]
	fld	qword [S.Y]
	fstp	qword [S.Z]
	fld	qword [S.X]
	fstp	qword [S.Y]
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	call	draw_registers
	bts	[flags],10
	btr	[flags],6
	btr	[flags],12
	btr	[flags],3
	ret

_40:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	sub	eax,OPCODEtable
	shr	eax,2
	sub	eax,0x40
	shl	eax,3
	add	eax,R.0
	fld	qword [S.X]
	fstp	qword [eax]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	call	draw_registers
	bts	[flags],10
	btr	[flags],6
	btr	[flags],11
	btr	[flags],3
	ret

_25:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X1]
	fld	qword [S.X]
	fstp	qword [S.X1]
	fld	qword [S.Y]
	fstp	qword [S.X]
	fld	qword [S.Z]
	fstp	qword [S.Y]
	fld	qword [S.T]
	fstp	qword [S.Z]
	fstp	qword [S.T]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_24:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	fld	qword [S.Y]
	call	FpuXexpY
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_23:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	fld1
	fdivrp
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_22:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	fmul	st,st
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_21:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	fsqrt
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret


_20:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fstp	qword [S.X1]
	fld	qword [S.Z]
	fstp	qword [S.T]
	fld	qword [S.Y]
	fstp	qword [S.Z]
	fld	qword [S.X]
	fstp	qword [S.Y]
	fldpi
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_1E:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	call	FpuTan
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret


_1D:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	call	FpuCos
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_1C:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	call	FpuSin
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_1B:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	call	FpuArctan
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_1A:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	call	FpuArccos
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret
_19:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	call	FpuArcsin
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret


_18:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	fldln2
	fxch
	fyl2x
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_17:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	fldlg2
	fxch
	fyl2x
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret


_16:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fst	qword [S.X1]
	call	FpuEexpX
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_15:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	mov	[perem],10
	fild	[perem]
	fld	qword [S.X]
	fst	qword [S.X1]
	call	FpuXexpY
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_14:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.Y]
	fld	qword [S.X]
	fst	qword [S.Y]
	fstp	qword [S.X1]
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	ret
_13:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.Y]
	fld	qword [S.X]
	fdivp
	fld	qword [S.X]
	fstp	qword [S.X1]
	fstp	qword [S.X]
	fld	qword [S.Z]
	fstp	qword [S.Y]
	fld	qword [S.T]
	fstp	qword [S.Z]
;	fldz
;	fstp	qword [S.T]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	ret
_12:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.Y]
	fld	qword [S.X]
	fmulp
	fld	qword [S.X]
	fstp	qword [S.X1]
	fstp	qword [S.X]
	fld	qword [S.Z]
	fstp	qword [S.Y]
	fld	qword [S.T]
	fstp	qword [S.Z]
;	fldz
;	fstp	qword [S.T]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	ret
_11:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.Y]
	fld	qword [S.X]
	fsubp
	fld	qword [S.X]
	fstp	qword [S.X1]
	fstp	qword [S.X]
	fld	qword [S.Z]
	fstp	qword [S.Y]
	fld	qword [S.T]
	fstp	qword [S.Z]
;	fldz
;	fstp	qword [S.T]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	ret

_10:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.X]
	fld	qword [S.Y]
	faddp
	fld	qword [S.X]
	fstp	qword [S.X1]
	fstp	qword [S.X]
	fld	qword [S.Z]
	fstp	qword [S.Y]
	fld	qword [S.T]
	fst	qword [S.Z]
;	fldz
;	fstp	qword [S.T]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	ret

_0F:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	fld	qword [S.Z]
	fstp	qword [S.T]
	fld	qword [S.Y]
	fstp	qword [S.Z]
	fld	qword [S.X]
	fstp	qword [S.Y]
	fld	qword [S.X1]
	fstp	qword [S.X]
	call	draw_stack
	fld	qword [S.X]
	call	ftos
	call	draw_string
	bts	[flags],10
	btr	[flags],6
	btr	[flags],3
	ret

_0E:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:
;       call    stof
;       fstp    qword [S.X]
	fld	qword [S.Z]
	fstp	qword [S.T]
	fld	qword [S.Y]
	fstp	qword [S.Z]
	fld	qword [S.X]
	fstp	qword [S.Y]
	call	draw_string
;       call    stof
;       fstp    qword [S.X]
	call	draw_stack
	bts	[flags],10
	bts	[flags],7
	btr	[flags],6
	ret
_0D:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	bt	[flags],3
	jc	.F
	fldz
	fstp	qword	[S.X]
	mov	esi,string_zero
	mov	edi,string
	mov	ecx,13
	cld
	@@:	movsb
	loop @b
	mov	[move1],0
	mov	[move2],0
	mov	[move1dot],0
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
	btr	[flags],6
	btr	[flags],10
	btr	[flags],11
	btr	[flags],4
	btr	[flags],8
	btr	[flags],9
	ret
	.F:
	btr	[flags],3
	ret
_0C:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	btc	[flags],6
	btr	[flags],10
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
	cmp byte [string+12],0x20
	jne @f
	mov word [string+11],0x3030
	@@:
	cmp byte [string+1],"0"
	je @f
	ret
  @@:	cmp word [string+2],"  "
	je @f
	cmp word [string+2],". "
	je @f
	ret
  @@:	mov dword [string]," 1  "
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
	ret
_0B:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	jmp	.0
	;       test [flags],(1 shl 7)
	jz @f
	call null.null
	jmp .0
  @@:	test [flags],(1 shl 10)
	jz .0
	call null
  .0:;  fld     qword [S.X]
;       call    ftos
	bt [flags],6
	jc	@f
	cmp byte [string],'-'
	jne .sign_1
	mov byte [string],' '
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
	ret
	.sign_1:
	mov byte [string],'-'
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
	ret
		@@:
	cmp byte [string+10],'-'
	jne .sign_2
	mov byte [string+10],' '
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
	ret
	.sign_2:
	mov byte [string+10],'-'
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
	ret

_0A:
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	test [flags],(1 shl 7)
	jz @f
	call null.null
	jmp .0
  @@:	test [flags],(1 shl 10)
	jz .0
	call null
  .0:	cmp [move1],8
	jge .nop
	cmp [move1],0
	jne	@f
	mov [move1],1
	mov [move1dot],1
	mov word [string+1],'0.'
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
  .nop: ret
		@@:
	cmp [move1dot],1
	je .nop
	movzx esi,[move1]
	mov byte [esi+string+1],'.'
	mov [move1dot],1
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
	ret

_00:
	sub	eax,OPCODEtable
	shr	eax,2
	add	eax,0x30
	push	eax
	bt	[flags],0
	jnc	@f
	inc	[schk]
  @@:	test [flags],(1 shl 7)
	jz @f
	call null.null
	jmp .0
  @@:	test [flags],(1 shl 10)
	jz .0
	call null
  .0:	pop ebx
	test [flags],(1 shl 6)
	jnz	@f
	cmp [move1],8
	jge .nop
	movzx esi,[move1]
	movzx edi,[move1dot]
	add esi,edi
	mov byte [esi+string+1],bl
	inc [move1]
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
	ret
		@@:
	cmp [move2],0
	jne @f
	mov byte [string+12],bl
	inc [move2]
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
	ret
	@@:
	mov al, [string+12]
	mov [string+11],al
	mov byte [string+12],bl
	call	draw_string
	call	stof
	fstp	qword [S.X]
	call	draw_stack
 .nop:	ret

null:	fld	qword [S.Z]
	fstp	qword [S.T]
	fld	qword [S.Y]
	fstp	qword [S.Z]
	fld	qword [S.X]
	fstp	qword [S.Y]
.null:	call	null_string
	btr	[flags],10
	btr	[flags],7
ret





null_string:
	mov	esi,string_zero
	mov	edi,string
	mov	ecx,13
	cld
	@@:	movsb
	loop @b
	mov	[move1],0
	mov	[move2],0
	mov	[move1dot],0
ret

