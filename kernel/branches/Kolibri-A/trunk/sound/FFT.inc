;           Fast Hartley Transform routine
;           Copyright (C) 1999, 2004, 2010
;          Artem Jerdev  artem@jerdev.co.uk
;
; free KolibriOS version - not to be ported to other OSes
; ==========================================================

Power_of_4	equ	5
NumPoints	equ	1024
N_2		equ	NumPoints / 2
N_4		equ	NumPoints / 4

;=================================================================
; global constants
align 8
_root		dq	1.41421356237309504880169	; = sqrt(2)
_root2	dq	0.70710678118654752440084	; = sqrt(2)/2
_c1		dq	0.92387953251128675612818	; = cos(pi/8)
_s1		dq	0.38268343236508977172846	; = sin(pi/8)
_dx		dq	0.00613592315154296875		; pi/512

;[_CosTable]	dd	0 ; N_2 elements
;[_SinTable]	dd	0 ; N_2 elements

;; ==========================
align 4
MakeSinCosTable:
	mov	ebx, [_Sines]
	mov	ecx, [_Cosins]
	xor	eax,  eax
	fld	[_dx]			; st : dx
	fldz				; st : 0, dx
.loop:
	fld	st0			; st : x, x, dx
	FSINCOS 			; st : cos, sin, x, dx
	fstp	qword [ecx+eax*8]	; st : sin, x, dx
	fstp	qword [ebx+eax*8]	; st : x, dx
	fadd	st0, st1		; st : x+dx, dx

	inc	eax
	cmp	eax, N_2
	jne	.loop
	fstp	st0			; st : dx
	fstp	st0			; st : <empty>
	ret

; ================================================================
align 4
BitInvert:
	mov	esi, [x]	; array of qwords
	xor	ecx, ecx	; index term
.newterm:
	inc	ecx
	cmp	ecx, NumPoints
	jge	.done

	xor	eax, eax
	mov	edx, ecx
	xor	bl, bl

.do_invert:
	inc	bl
	cmp	bl, Power_of_4
	jg	.switch

	mov	bh, dl
	and	bh,  3
	shl	eax, 2
	or	al, bh
	shr	edx, 2
	jmp	.do_invert

.switch:
	cmp	eax, ecx
	jle	.newterm

	fld	qword [esi+eax*8]
	fld	qword [esi+ecx*8]
	fstp	qword [esi+eax*8]
	fstp	qword [esi+ecx*8]
	jmp	.newterm

.done:
	ret

;=================================================================
align 4

step1:
	mov	esi, [x]
	mov	ebx, esi
	add	esi, NumPoints*8

.loop:
	fld	qword[ebx]
	fld	qword[ebx+8]
	fld	st1
	fsub	st0, st1	; st : t2, f[i+1], f[i]
	fxch	st1		; st : f[i+1], t2, f[i]
	faddp	st2, st0	; st : t2, t1
	fld	qword[ebx+16]
	fld	qword[ebx+24]
	fld	st1		; st : f[i+2], f[i+3], f[i+2], t2, t1
	fadd	st0, st1	; st : t3, f[i+3], f[i+2], t2, t1
	fxch	st2		; st : f[i+2], f[i+3], t3, t2, t1
	fsub	st0, st1	; st : t4, f[i+3], t3, t2, t1
	fstp	st1		; st : t4, t3, t2, t1
	fld	st2		; st : t2, t4, t3, t2, t1
	fadd	st0, st1	; st : t2+t4, t4, t3, t2, t1
	fstp	qword[ebx+16]	; st : t4, t3, t2, t1
	fsubp	st2, st0	; st : t3, t2-t4, t1
	fld	st2		; st : t1, t3, t2-t4, t1
	fadd	st0, st1	; st : t1+t3, t3, t2-t4, t1
	fstp	qword[ebx]	; st : t3, t2-t4, t1
	fsubp	st2, st0	; st : t2-t4, t1-t3
	fstp	qword[ebx+24]	; st : t1-t3
	fstp	qword[ebx+8]	; st : <empty>

	add	ebx, 32
	cmp	ebx, esi
	jnz	.loop

	ret


;
;===========================================================================
step2:				; Step2
;===========================================================================

	mov	eax, [_f]
	mov	ebx, eax
	add	eax, NumPoints*8

.loop_i:

; -- quad subelements  +0, +4, +8 and +12 (simpliest operations)
	fld	qword[ebx]
	fld	qword[ebx+8*4]
	fld	st0
	fadd	st0, st2	; st : t1, f_4, f_0
	fxch	st1
	fsubp	st2, st0	; st : t1, t2
	fld	qword[ebx+8*8]
	fld	qword[ebx+8*12]
	fld	st0
	fadd	st0, st2	; st : t3, f_12, t1, t2
	fxch	st1
	fsubp	st2, st0	; st : t3, t4, t1, t2
	; ------
	fld	st2		; st : t1, t3, t4, t1, t2
	fadd	st0, st1
	fstp	qword[ebx]	; st : t3, t4, t1, t2
	fsub	st0, st2	; st : t3-t1, t4, t1, t2
	fchs			; st : t1-t3, t4, t1, t2
	fstp	qword[ebx+8*4]	; st : t4, t1, t2
	fst	st1		; st : t4, t4, t2
	fadd	st0, st2	; st : t2+t4, t4, t2
	fstp	qword[ebx+8*8]	; st : t4, t2
	fsubp	st1, st0	; st : t2-t4
	fstp	qword[ebx+8*12] ; st : <empty>

; -- even subelements  +2, +6, +10 and +14 (2 multiplications needed)
	fld	qword[ebx+8*2]
	fld	qword[ebx+8*6]
	fld	[_root]
	fmul	st1, st0	; st : r, t2, t1
	fld	qword[ebx+8*10]
	fxch	st1		; st : r, t3, t2, t1
	fmul	qword[ebx+8*14] ; st : t4, t3, t2, t1
	; ------
	fld	st3		; st : t1, t4, t3, t2, t1
	fadd	st0, st3	;
	fadd	st0, st2	;
	fst	qword[ebx+8*2]	; store f[i+8] = t1+t2+t3
	fsub	st0, st3	;
	fsub	st0, st3	;
	fstp	qword[ebx+8*10] ; store f[i+10]= t1-t2+t3
	fld	st3		; st : t1, t4, t3, t2, t1
	fsub	st0, st2	;
	fsub	st0, st1	;
	fst	qword[ebx+8*14] ; store f[i+14]= t1-t3-t4
	fadd	st0, st1	;
	faddp	st1, st0	; st : t1-t3+t4, t3, t2, t1
	fstp	qword[ebx+8*6]	; store f[i+6]
	fstp	st0		; st : t2, t1
	fstp	st0		; st : t1
	fstp	st0		; st : <empty>

; -- odd subelements
	fld	qword[ebx+8*9]
	fld	qword[ebx+8*11]
	fld	st1
	fsub	st0, st1
	fxch	st1
	faddp	st2, st0	; st : (f[l3]-f[l7]), (f[l3]+f[l7])
	fld	[_root2]
	fmul	st2, st0
	fmulp	st1, st0	; st : t9, t6
	fld	qword[ebx+8*3]
	fld	st0
	fadd	st0, st2	; st : t1, f[l5], t9, t6
	fstp	[_t1]
	fsub	st0, st1
	fstp	[_t2]
	fstp	[_t9]	; (t9 never used)
	fstp	[_t6]		; st : <empty>

	fld	[_c1]
	fld	[_s1]
	fld	qword[ebx+8*5]
	fld	qword[ebx+8*7]
	fld	st3		; st: c1, f[l6], f[l2], s1, c1
	fmul	st0, st2	; st: f_2*c, f_6, f_2, s, c
	fld	st1		; st: f_6, f_2*c, f_6, f_2, s, c
	fmul	st0, st4	; st: f_6*s, f_2*c, f_6, f_2, s, c
	faddp	st1, st0	; st: t5, f_6, f_2, s, c
	fstp	[_t5]		; st: f_6, f_2, s, c
	fld	st3		; st: c, f_6, f_2, s, c
	fmul	st0, st1
	fld	st3
	fmul	st0, st3	; st: f_2*s, f_6*c, f_6, f_2, s, c
	fsubp	st1, st0	; st: t8, f_6, f_2, s, c
	fstp	[_t8]		; st: f_6, f_2, s, c
	fstp	st0		; st: f_2, s, c
	fstp	st0		; st: s, c

	fld	qword[ebx+8*13]
	fld	qword[ebx+8*15]
	fld	st3		; st: c1, f[l8], f[l4], s1, c1
	fmul	st0, st1
	fld	st3
	fmul	st0, st3	; st: f_4*s, f_8*c, f_8, f_4, s, c
	faddp	st1, st0	; st: t7, f_8, f_4, s, c
	fld	[_t5]		; st: t5, t7, f_8, f_4, s, c
	fsub	st0, st1	; st: t4, t7, f_8, f_4, s, c
	fstp	[_t4]
	fstp	[_t7]		; st: f_8, f_4, s, c
	fld	st3		; st: c, f_8, f_4, s, c
	fmul	st0, st2
	fld	st3
	fmul	st0, st2	; st: f_8*s, f_4*c, f_8, f_4, s, c
	fsubp	st1, st0	; st:-t0, f_8, f_4, s, c
	fchs
	fld	[_t8]
	fchs			; st:-t8, t0, f_8, f_4, s, c
	fsub	st0, st1	; st: t3, t0, f_8, f_4, s, c
	fstp	[_t3]
	fstp	[_t0]		; st: f_8, f_4, s, c
	fstp	st0		; st: f_4, s, c
	fstp	st0		; st: s, c
	fstp	st0		; st: c
	fstp	st0		; st: <empty>

	fld	[_t1]
	fld	[_t4]
	fld	st1
	fsub	st0, st1
	fstp	qword[ebx+8*11] ; f[l7] = t1-t4
	faddp	st1, st0
	fstp	qword[ebx+8*3]	; f[l5] = t1+t4
	fld	[_t2]
	fld	[_t3]
	fld	st1
	fsub	st0, st1
	fstp	qword[ebx+8*15] ; f[l8]
	faddp	st1, st0
	fstp	qword[ebx+8*7]	; f[l6]

	fld	[_t6]
	fld	qword[ebx+8]
	fld	st1
	fsub	st0, st1
	fxch	st1
	faddp	st2, st0	; st : t2, t1
	fld	[_t8]
	fsub	[_t0]
	fld	[_t5]
	fadd	[_t7]		; st : t4, t3, t2, t1

	fld	st3
	fsub	st0, st1
	fstp	qword[ebx+8*9]	; f[l3] = t1-t4
	fadd	st0, st3
	fstp	qword[ebx+8]	; f[l1] = t1+t4
	fld	st1		; st : t2, t3, t2, t1
	fsub	st0, st1	; f[l4] = t2-t3
	fstp	qword[ebx+8*13] ; st : t3, t2, t1
	faddp	st1, st0	; st : t2+t3, t1
	fstp	qword[ebx+8*5]	; f[l2] = t2+t3
	fstp	st0		; st : <empty>

	add	ebx, 16*8
	cmp	ebx, eax
	jb	.loop_i

	ret

align 8 	; shared local vars
_t0	dq	0
_t1	dq	0
_t2	dq	0
_t3	dq	0
_t4	dq	0
_t5	dq	0
_t6	dq	0
_t7	dq	0
_t8	dq	0
_t9	dq	0



;===================================================================
step3:
;===================================================================

; 283  : {


; 293  :   for (l=3; l<=p; l++)
	mov	cx, 0x0200
.newstep:
	inc	ch
	cmp	ch, Power_of_4
	jg	.done
	mov	[.step], cx

; 294  :   {
; 295  :     d1 = 1 << (l + l - 3);

	mov	cl, ch
	add	cl, cl
	sub	cl, 3
	mov	edx, 1
	shl	edx, cl
	mov	[.d1], edx

; 296  :     d2 = d1 << 1;
	shl	edx, 1
	mov	[.d2], edx
	mov	eax, edx

; 297  :     d3 = d2 << 1;
	shl	edx, 1
	mov	[.d3], edx

; 298  :     d4 = d2 + d3;
	add	eax, edx
	mov	[.d4], eax

; 299  :     d5 = d3 << 1;
	shl	edx, 1
	mov	[.d5], edx
	shl	edx, 3
	mov	[.d6], edx	; d6 = d5*8 to simplify index operations

; 339  :         j5 = N / d5;   ; moved out of internal loop
	mov	cl, Power_of_4
	sub	cl, ch
	add	cl, cl
	mov	edx, 1
	shl	edx, cl
	mov	[.j5], edx

; 300  :
; 301  :     for (j=0; j<N; j+=d5)
	mov	esi, [_f]
	mov	ebx, esi
	add	esi, NumPoints*8
	mov	[.end_of_array], esi

.next_j:

; {
; t1 = f[j] + f[j+d2];
	mov	eax, [.d2]
	fld	qword[ebx]
	fld	qword[ebx+eax*8]
	fld	st1
	fadd	st0, st1
	fstp	[_t1]

; t2 = f[j] - f[j+d2];
	fsubp	st1, st0
	fstp	[_t2]

; t3 = f[j+d3] + f[j+d4];
	mov	edi, [.d3]
	fld	qword[ebx+edi*8]
	mov	edx, [.d4]
	fld	qword[ebx+edx*8]
	fld	st1
	fsub	st0, st1		; st : t4, f4, f3
	fxch	st1			; st : f4, t4, f3

; t4 = f[j+d3] - f[j+d4];
	faddp	st2, st0		; st : t4, t3

; f[j+d4] = t2 - t4;
; f[j+d3] = t2 + t4;
	fld	[_t2]
	fld	st0
	fsub	st0, st2		; st : f4, t2, t4, t3
	fstp	qword[ebx+edx*8]	; st : t2, t4, t3
	fadd	st0, st1		; st : f3, t4, t3
	fstp	qword[ebx+edi*8]	; st : t4, t3

; f[j+d2] = t1 - t3;
; f[j]    = t1 + t3;
	fld	[_t1]
	fst	st1
	fsub	st0, st2		; st : f2, t1, t3
	fstp	qword[ebx+eax*8]	; st : t1, t3
	fadd	st0, st1		; st : f0, t3
	fstp	qword[ebx]		; st : t3
	fstp	st0

; jj = j + d1;     / ??
	mov	edi, [.d1]
	shl	edi, 3		; = d1*8
	mov	edx, edi
	mov	eax, edi
	add	eax, eax	; eax = d2*8
	shl	edx, 2		; = d3*8
	add	edi, ebx	; now [edi] points to f[jj]
	add	edx, edi	; and [edx] points to f[jj+d3]

; t1 = f[jj];
	fld	qword [edi]	; st : t1
; t3 = f[jj+d3];
	fld	qword [edx]	; st : t3, t1

; t2 = f[jj+d2] * r;
	fld	qword [edi+eax]
	fld	[_root]
	fmul	st1, st0	; st : r,  t2, t3, t1
; t4 = f[jj+d4] * r
	fmul	qword [edx+eax] ; st : t4, t2, t3, t1

; f[jj]    = t1 + t2 + t3;
	fld	st3		; st : t1, t4, t2, t3, t1
	fadd	st0, st3
	fadd	st0, st2
	fstp	qword [edi]

; f[jj+d2] = t1 - t3 + t4;
	fld	st3
	fsub	st0, st3	; st : (t1-t3), t4, t2, t3, t1
	fld	st0
	fadd	st0, st2	; st : f2, (t1-t3), t4, t2, t3, t1
	fstp	qword [edi+eax]
; f[jj+d4] = t1 - t3 - t4;
	fsub	st0, st1	; st : f4, t4, t2, t3, t1
	fstp	qword [edx+eax]

; f[jj+d3] = t1 - t2 + t3;
	fstp	st0		; st : t2, t3,  t1
	fsubp	st1, st0	; st : (t3-t2), t1
	faddp	st1, st0	; st : f3
	fstp	qword [edx]

; for (k=1; k<d1; k++)
	xor	ecx, ecx	; ecx = k
	mov	[.jj], ecx
.next_k:
	inc	ecx
	cmp	ecx, [.d1]
	jge	.done_k
; {
	mov	eax, [.d2]	; the sector increment
; l1 = j  + k;
	mov	edx, ecx
	mov	[.l1], edx	; [ebx+edx*8] --> f[j+k]
; l2 = l1 + d2;
	add	edx, eax
	mov	[.l2], edx
; l3 = l1 + d3;
	add	edx, eax
	mov	[.l3], edx
; l4 = l1 + d4;
	add	edx, eax
	mov	[.l4], edx

; l5 = j  + d2 - k;
	mov	edx, eax
	sub	edx, ecx
	mov	[.l5], edx
; l6 = l5 + d2;
	add	edx, eax
	mov	[.l6], edx
; l7 = l5 + d3;
	add	edx, eax
	mov	[.l7], edx
; l8 = l5 + d4;
	add	edx, eax
	mov	[.l8], edx


; 340  :         j5 *= k;       // add-substituted multiplication
	mov	eax, [.jj]
	add	eax, [.j5]
	mov	[.jj], eax

; c1 = C[jj];
; s1 = S[jj];
	mov	edi, [_Cosins]
	fld	qword[edi+eax*8]
	mov	esi, [_Sines]
	fld	qword[esi+eax*8]	; st : s1, c1

; t5 = f[l2] * c1 + f[l6] * s1;
; t8 = f[l6] * c1 - f[l2] * s1;
	mov	edx, [.l6]
	fld	qword[ebx+edx*8]
	mov	edx, [.l2]
	fld	st0
	fmul	st0, st2
	fxch	st1
	fmul	st0, st3
	fld	qword[ebx+edx*8]	; st : f[l2], f[l6]*c, f[l6]*s, s, c
	fmul	st4, st0
	fmulp	st3, st0		; st : f[l6]*c, f[l6]*s, f[l2]*s, f[l2]*c
	fsub	st0, st2		; st :   t8,    f[l6]*s, f[l2]*s, f[l2]*c
	fstp	[_t8]
	faddp	st2, st0		; st :  f[l2]*s, t5
	fstp	st0			; st :  t5
	fstp	[_t5]			; st :  <empty>

; c2 = C[2*jj];
; s2 = S[2*jj];
	shl	eax, 1
	fld	qword[edi+eax*8]
	fld	qword[esi+eax*8]	; st : s2, c2

; t6 = f[l3] * c2 + f[l7] * s2;
; t9 = f[l7] * c2 - f[l3] * s2;
	mov	edx, [.l7]
	fld	qword[ebx+edx*8]
	mov	edx, [.l3]
	fld	st0
	fmul	st0, st2
	fxch	st1
	fmul	st0, st3
	fld	qword[ebx+edx*8]	; st : f[l3], f[l7]*c, f[l7]*s, s, c
	fmul	st4, st0
	fmulp	st3, st0		; st : f[l7]*c, f[l7]*s, f[l3]*s, f[l3]*c
	fsub	st0, st2		; st :   t9,    f[l7]*s, f[l3]*s, f[l3]*c
	fstp	[_t9]
	faddp	st2, st0		; st :  f[l2]*s, t6
	fstp	st0			; st :  t6
	fstp	[_t6]			; st :  <empty>

; c3 = C[3*jj];
; s3 = S[3*jj];
	add	eax, [.jj]
	fld	qword[edi+eax*8]
	fld	qword[esi+eax*8]	; st : s3, c3

; t7 = f[l4] * c3 + f[l8] * s3;
; t0 = f[l8] * c3 - f[l4] * s3;
	mov	edx, [.l8]
	fld	qword[ebx+edx*8]
	mov	edx, [.l4]
	fld	st0
	fmul	st0, st2
	fxch	st1
	fmul	st0, st3
	fld	qword[ebx+edx*8]	; st : f[l4], f[l8]*c, f[l8]*s, s, c
	fmul	st4, st0
	fmulp	st3, st0		; st : f[l8]*c, f[l8]*s, f[l4]*s, f[l4]*c
	fsub	st0, st2		; st :   t9,    f[l8]*s, f[l4]*s, f[l4]*c
	fstp	[_t0]
	faddp	st2, st0		; st : f[l2]*s, t7
	fstp	st0			; st :  t7
	fstp	[_t7]			; st :  <empty>

; t1 = f[l5] - t9;
; t2 = f[l5] + t9;
	mov	eax, [.l5]
	fld	qword [ebx+eax*8]
	fld	[_t9]
	fld	st0
	fadd	st0, st2
	fstp	[_t2]
	fsubp	st1, st0
	fstp	[_t1]

; t3 = - t8  - t0;
	fld	[_t8]
	fadd	[_t0]
	fchs
	fstp	[_t3]
; t4 =   t5  - t7;
	fld	[_t5]
	fsub	[_t7]
	fstp	[_t4]

; f[l5] = t1 + t4;
	fld	[_t1]
	fld	[_t4]
	fld	st0
	fadd	st0, st2
	fstp	qword [ebx+eax*8]
; f[l7] = t1 - t4;
	mov	eax, [.l7]
	fsubp	st1, st0
	fstp	qword [ebx+eax*8]

; f[l6] = t2 + t3;
	mov	eax, [.l6]
	fld	[_t2]
	fld	[_t3]
	fld	st0
	fadd	st0, st2
	fstp	qword [ebx+eax*8]
; f[l8] = t2 - t3;
	mov	eax, [.l8]
	fsubp	st1, st0
	fstp	qword [ebx+eax*8]

; t1 = f[l1] + t6;
	mov	eax, [.l1]
	fld	qword [ebx+eax*8]
	fld	[_t6]
	fld	st0
	fadd	st0, st2
	fstp	[_t1]
; t2 = f[l1] - t6;
	fsubp	st1, st0
	fstp	[_t2]

; t3 =    t8 - t0;
	fld	[_t8]
	fsub	[_t0]
	fstp	[_t3]
; t4 =    t5 + t7;
	fld	[_t5]
	fadd	[_t7]
	fstp	[_t4]

; f[l1] = t1 + t4;
	mov	eax, [.l1]
	fld	[_t1]
	fld	[_t4]
      fld     st0
	fadd	st0, st2
	fstp	qword [ebx+eax*8]
; f[l3] = t1 - t4;
	mov	eax, [.l3]
	fsubp	st1, st0
	fstp	qword [ebx+eax*8]

; f[l2] = t2 + t3;
	mov	eax, [.l2]
	fld	[_t2]
	fld	[_t3]
	fld	st0
	fadd	st0, st2
	fstp	qword [ebx+eax*8]
; f[l4] = t2 - t3;
	mov	eax, [.l4]
	fsubp	st1, st0
	fstp	qword [ebx+eax*8]

; 374  :       }
	jmp	.next_k

.done_k:
; 375  :     }
	add	ebx, [.d6]	; d6 = d5*8
	cmp	ebx, [.end_of_array]
	jb	.next_j

; 376  :   }
	mov	cx, [.step]
	jmp	.newstep
.done:

; 377  : }
	ret

align 4
.l1   dd	0
.l2   dd	0
.l3   dd	0
.l4   dd	0
.l5   dd	0
.l6   dd	0
.l7   dd	0
.l8   dd	0
.l9   dd	0
.l0   dd	0
.d1   dd	0
.d2   dd	0
.d3   dd	0
.d4   dd	0
.d5   dd	0
.d6   dd	0
.j5   dd	0
.jj   dd	0
.end_of_array	dd	  0
.step dw	0

align 8

		;=========== Step3 ends here ===========





; =================================================================
;	syscall entry
;
_f  dd ?
_N  dd 1024	; number of points

_a		dd ?		; initial   data array
x      	dd 0    	; tranformed (float) data array
_Cosins 	dd 0
_Sines	dd 0

FFT4:
	or	al, al
	jnz	.trans
	mov	cl, Power_of_4
	mov	eax, 1
	shl	eax, cl
	shl	eax, cl
	mov	[_N], eax
	shl	eax, 2	; size of Sine table in bytes
	add	eax, ebx
	mov 	[_Sines], ebx
	mov	[_Cosins], eax
	cpuid
	rdtsc
	mov	[.time], eax
	call	MakeSinCosTable
	cpuid
	rdtsc
	sub	eax, [.time]
	ret
.trans:
	mov	[x], 	ebx
	mov	[_f], ebx
	cli	;-----
	cpuid
	rdtsc
	mov	[.time], eax
	call BitInvert
	call	step1
	call	step2
	call	step3
	cpuid
	rdtsc
	sti	;----
	sub	eax, [.time]
	ret

.time	dd	0
