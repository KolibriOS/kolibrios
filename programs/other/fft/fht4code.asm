;           Fast Hartley Transform routine
;           Copyright (C) 1999, 2004, 2010
;          Artem Jerdev  artem@jerdev.co.uk
;
; free KolibriOS version - not to be ported to other OSes
; ==========================================================


; global constants
align 8
_r	dq	1.41421356237309504880169	; = sqrt(2)
_r2	dq	0.70710678118654752440084	; = sqrt(2)/2
_c1	dq	0.92387953251128675612818	; = cos(pi/8)
_s1	dq	0.38268343236508977172846	; = sin(pi/8)

;=================================================================
; parameter1:
; -- reg dl (bits[3:0])   = Power_of_4
; returns:
; -- reg edx = _CosTable address (4k-aligned)
; assumes:     _SinTable  = _CosTable + (N/2)*8
; user heap has to be initialized
; destroys:
; -- eax, ebx, ecx
;; ==========================
align 4
CreateSinCosTable:
	xor	eax,  eax
	inc	eax
	mov	cl,  dl
	and	cl,  15
	shl	eax, cl
	shl	eax, cl
	mov	ecx, eax		; now ecx = N
	shl	ecx, 3
	mov	ebx, 12
	mov	eax, 68
	int	0x40			; getmem(N*sizeof(double))

	mov	edx, eax		; edx = _CosTable
	shr	ecx, 1
	mov	ebx, eax
	add	ebx, ecx		; ebx = _SinTable
	shr	ecx, 3
	push	ecx			; [esp] = ecx = N/2

	xor	eax,  eax
	fldpi
	fidiv	dword[esp]		; st : dx = 2*pi/N
	pop	ecx
	fldz				; st : 0, dx
.loop:
	fld	st0			; st : x, x, dx
	FSINCOS 			; st : cos, sin, x, dx
	fstp	qword [edx+eax*8]	; st : sin, x, dx
	fstp	qword [ebx+eax*8]	; st : x, dx
	fadd	st0, st1		; st : x+dx, dx

	inc	eax
	cmp	eax, ecx
	jne	.loop
	fstp	st0			; st : dx
	fstp	st0			; st : <empty>
ret

;=================================================================
; parameter1:
; -- reg edx = _CosTable address
; destroys:
; -- eax, ebx, ecx
;; ==========================
align 4
DestroySinCosTable:
	mov	ecx, edx
	mov	ebx, 13
	mov	eax, 68
	int	0x40			; free(SinCosTable)
ret

;=================================================================
; parameter1:
; -- reg  dl (bits[3:0])   = Power_of_4
; -- reg edx && (-16) = 4k-aligned data array address
; returns:
; -- edx = Power_of_4
; -- ecx = N
; destroys:
; -- eax, ebx, ecx, edx, esi
;; ==========================
align 4
BitInvert:
	mov	esi, edx
	and	esi, 0xFFFFFFF0
	and	edx, 0x0F
	push	edx
	mov	cl, dl
	xor	eax, eax
	inc	eax
	shl	eax, cl
	shl	eax, cl
	push	eax
	xor	ecx, ecx		; index term
.newterm:
	inc	ecx
	cmp	ecx, [esp]		; N
	jge	.done

	xor	eax, eax
	mov	edx, ecx
	xor	bl, bl

.do_invert:
	inc	bl
	cmp	bl, byte[esp+4] ; Power_of_4
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
	pop	ecx
	pop	edx
	ret

;=================================================================


;=================================================================
; stdcall parameters:
; -- [esp+4]  = N
; -- [esp+8]  = 4k-aligned data array  address
; returns:
; -- nothing
; destroys:
; -- ebx, esi
;; ==========================
align 4
step1:
	mov	ebx, [esp+8]
	mov	esi, [esp+4]
	shl	esi, 3
	add	esi, ebx

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

;=================================================================
; SSE3 version:	Step1
;
;==========================

align 4
step1_sse:
	mov	ebx, [esp+8]
	mov	esi, [esp+4]
	shl	esi, 3
	add	esi, ebx

.loop:
	movddup     xmm0, [ebx]     ;   xmm0: f0 ; f0
	movddup     xmm1, [ebx+8]   ;   xmm1: f1 ; f1
	addsubpd    xmm0, xmm1      ;   xmm0: t1 ; t2   ( + - )
    movddup     xmm1, [ebx+16]  ;   xmm1: f2 ; f2
    movddup     xmm2, [ebx+24]  ;   xmm2: f3 ; f3
	addsubpd    xmm1, xmm2      ;   xmm1: t3 ; t4   ( + - )

    movddup     xmm2, xmm0      ;   xmm2: t2 ; t2
    movddup     xmm3, xmm1      ;   xmm3: t4 ; t4
	addsubpd    xmm2, xmm3      ;   xmm2: 2+4; 2-4  
    shufpd      xmm2, xmm2, 1   ;   xmm2: 2-4; 2+4
    movapd      [ebx+16], xmm2

    shufpd      xmm0, xmm0, 1   ;   xmm0: t2 ; t1
    shufpd      xmm1, xmm1, 1   ;   xmm1: t4 ; t3
    movddup     xmm2, xmm0      ;   xmm2: t1 ; t1
    movddup     xmm3, xmm1      ;   xmm3: t3 ; t3
	addsubpd    xmm2, xmm3      ;   xmm2: 1+3; 1-3  
    shufpd      xmm2, xmm2, 1   ;   xmm2: 1-3; 1+3
    movapd      [ebx], xmm2

	add	ebx, 32
	cmp	ebx, esi
	jnz	.loop
ret

;       local stack definitions
;===========================================================================
_t0	equ	dword [esp]
_t1	equ	dword[esp+4]
_t2	equ	dword[esp+8]
_t3	equ	dword[esp+12]
_t4	equ	dword[esp+16]
_t5	equ	dword[esp+20]
_t6	equ	dword[esp+24]
_t7	equ	dword[esp+28]
_t8	equ	dword[esp+32]
_t9	equ	dword[esp+36]

_l1   equ	dword[esp+40]
_l2   equ	dword[esp+44]
_l3   equ	dword[esp+48]
_l4   equ	dword[esp+52]
_l5   equ	dword[esp+56]
_l6   equ	dword[esp+60]
_l7   equ	dword[esp+64]
_l8   equ	dword[esp+68]
_l9   equ	dword[esp+72]
_l0   equ	dword[esp+76]
_d1   equ	dword[esp+80]
_d2   equ	dword[esp+84]
_d3   equ	dword[esp+88]
_d4   equ	dword[esp+92]
_d5   equ	dword[esp+96]
_d6   equ	dword[esp+100]
_j5   equ	dword[esp+104]
_jj   equ	dword[esp+108]
_end_of_array	equ	dword[esp+112]
_step		equ	word [esp+116]


;=================================================================
; cdecl parameters:
; -- [ebp+8]   = N
; -- [ebp+12]  = 4k-aligned data array  address
; returns:
; -- nothing
; destroys:
; -- eax, ebx
; locals:
; -- 10 stack-located dwords (_t0 ... _t9)
;; ==========================
align 4
step2:
	push	ebp
	mov	ebp, esp
	sub	esp, 40
	mov	ebx, [ebp+12]
	mov	eax, [ebp+ 8]
	shl	eax, 3
	add	eax, ebx

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
	fld	[_r]
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
	fld	[_r2]
	fmul	st2, st0
	fmulp	st1, st0	; st : t9, t6
	fld	qword[ebx+8*3]
	fld	st0
	fadd	st0, st2	; st : t1, f[l5], t9, t6
	fstp	_t1
	fsub	st0, st1
	fstp	_t2
	fstp	_t9	; (t9 never used)
	fstp	_t6		; st : <empty>

	fld	[_c1]
	fld	[_s1]
	fld	qword[ebx+8*5]
	fld	qword[ebx+8*7]
	fld	st3		; st: c1, f[l6], f[l2], s1, c1
	fmul	st0, st2	; st: f_2*c, f_6, f_2, s, c
	fld	st1		; st: f_6, f_2*c, f_6, f_2, s, c
	fmul	st0, st4	; st: f_6*s, f_2*c, f_6, f_2, s, c
	faddp	st1, st0	; st: t5, f_6, f_2, s, c
	fstp	_t5		; st: f_6, f_2, s, c
	fld	st3		; st: c, f_6, f_2, s, c
	fmul	st0, st1
	fld	st3
	fmul	st0, st3	; st: f_2*s, f_6*c, f_6, f_2, s, c
	fsubp	st1, st0	; st: t8, f_6, f_2, s, c
	fstp	_t8		; st: f_6, f_2, s, c
	fstp	st0		; st: f_2, s, c
	fstp	st0		; st: s, c

	fld	qword[ebx+8*13]
	fld	qword[ebx+8*15]
	fld	st3		; st: c1, f[l8], f[l4], s1, c1
	fmul	st0, st1
	fld	st3
	fmul	st0, st3	; st: f_4*s, f_8*c, f_8, f_4, s, c
	faddp	st1, st0	; st: t7, f_8, f_4, s, c
	fld	_t5		; st: t5, t7, f_8, f_4, s, c
	fsub	st0, st1	; st: t4, t7, f_8, f_4, s, c
	fstp	_t4
	fstp	_t7		; st: f_8, f_4, s, c
	fld	st3		; st: c, f_8, f_4, s, c
	fmul	st0, st2
	fld	st3
	fmul	st0, st2	; st: f_8*s, f_4*c, f_8, f_4, s, c
	fsubp	st1, st0	; st:-t0, f_8, f_4, s, c
	fchs
	fld	_t8
	fchs			; st:-t8, t0, f_8, f_4, s, c
	fsub	st0, st1	; st: t3, t0, f_8, f_4, s, c
	fstp	_t3
	fstp	_t0		; st: f_8, f_4, s, c
	fstp	st0		; st: f_4, s, c
	fstp	st0		; st: s, c
	fstp	st0		; st: c
	fstp	st0		; st: <empty>

	fld	_t1
	fld	_t4
	fld	st1
	fsub	st0, st1
	fstp	qword[ebx+8*11] ; f[l7] = t1-t4
	faddp	st1, st0
	fstp	qword[ebx+8*3]	; f[l5] = t1+t4
	fld	_t2
	fld	_t3
	fld	st1
	fsub	st0, st1
	fstp	qword[ebx+8*15] ; f[l8]
	faddp	st1, st0
	fstp	qword[ebx+8*7]	; f[l6]

	fld	_t6
	fld	qword[ebx+8]
	fld	st1
	fsub	st0, st1
	fxch	st1
	faddp	st2, st0	; st : t2, t1
	fld	_t8
	fsub	_t0
	fld	_t5
	fadd	_t7		; st : t4, t3, t2, t1

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

	mov	esp, ebp
	pop	ebp
ret




;=================================================================
; cdecl parameters:
; -- [ebp+8]   = N
; -- [ebp+12]  = p
; -- [ebp+16]  = 4k-aligned data array  address
; -- [ebp+20]  = 4k-aligned SinCosTable address
; returns:
; -- nothing
; destroys:
; -- all GPRegs
; locals:
; -- 120 stack-located dwords (_t0 ... _t9, _l0..._step)
;; ==========================
align 4
step3:
	push	ebp
	mov	ebp, esp
	sub	esp, 120
; 283  : {


; 293  :   for (l=3; l<=p; l++)
	mov	cx, 0x0200
.newstep:
	inc	ch
	cmp	ch, byte[ebp+12]
	jg	.done
	mov	_step, cx

; 294  :   {
; 295  :     d1 = 1 << (l + l - 3);

	mov	cl, ch
	add	cl, cl
	sub	cl, 3
	mov	edx, 1
	shl	edx, cl
	mov	_d1, edx

; 296  :     d2 = d1 << 1;
	shl	edx, 1
	mov	_d2, edx
	mov	eax, edx

; 297  :     d3 = d2 << 1;
	shl	edx, 1
	mov	_d3, edx

; 298  :     d4 = d2 + d3;
	add	eax, edx
	mov	_d4, eax

; 299  :     d5 = d3 << 1;
	shl	edx, 1
	mov	_d5, edx
	shl	edx, 3
	mov	_d6, edx	; d6 = d5*8 to simplify index operations

; 339  :         j5 = N / d5;   ; moved out of internal loop
	mov	cl, [ebp+12]
	sub	cl, ch
	add	cl, cl
	mov	edx, 1
	shl	edx, cl
	mov	_j5, edx

; 300  :
; 301  :     for (j=0; j<N; j+=d5)
	mov	ebx, [ebp+16]
	mov	esi, [ebp+8]
	shl	esi, 3
	add	esi, ebx
	mov	_end_of_array, esi

.next_j:

; {
; t1 = f[j] + f[j+d2];
	mov	eax, _d2
	fld	qword[ebx]
	fld	qword[ebx+eax*8]
	fld	st1
	fadd	st0, st1
	fstp	_t1

; t2 = f[j] - f[j+d2];
	fsubp	st1, st0
	fstp	_t2

; t3 = f[j+d3] + f[j+d4];
	mov	edi, _d3
	fld	qword[ebx+edi*8]
	mov	edx, _d4
	fld	qword[ebx+edx*8]
	fld	st1
	fsub	st0, st1		; st : t4, f4, f3
	fxch	st1			; st : f4, t4, f3

; t4 = f[j+d3] - f[j+d4];
	faddp	st2, st0		; st : t4, t3

; f[j+d4] = t2 - t4;
; f[j+d3] = t2 + t4;
	fld	_t2
	fld	st0
	fsub	st0, st2		; st : f4, t2, t4, t3
	fstp	qword[ebx+edx*8]	; st : t2, t4, t3
	fadd	st0, st1		; st : f3, t4, t3
	fstp	qword[ebx+edi*8]	; st : t4, t3

; f[j+d2] = t1 - t3;
; f[j]    = t1 + t3;
	fld	_t1
	fst	st1
	fsub	st0, st2		; st : f2, t1, t3
	fstp	qword[ebx+eax*8]	; st : t1, t3
	fadd	st0, st1		; st : f0, t3
	fstp	qword[ebx]		; st : t3
	fstp	st0

; jj = j + d1;     / ??
	mov	edi, _d1
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
	fld	[_r]
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
	mov	_jj, ecx
.next_k:
	inc	ecx
	cmp	ecx, _d1
	jge	.done_k
; {
	mov	eax, _d2	; the sector increment
; l1 = j  + k;
	mov	edx, ecx
	mov	_l1, edx	; [ebx+edx*8] --> f[j+k]
; l2 = l1 + d2;
	add	edx, eax
	mov	_l2, edx
; l3 = l1 + d3;
	add	edx, eax
	mov	_l3, edx
; l4 = l1 + d4;
	add	edx, eax
	mov	_l4, edx

; l5 = j  + d2 - k;
	mov	edx, eax
	sub	edx, ecx
	mov	_l5, edx
; l6 = l5 + d2;
	add	edx, eax
	mov	_l6, edx
; l7 = l5 + d3;
	add	edx, eax
	mov	_l7, edx
; l8 = l5 + d4;
	add	edx, eax
	mov	_l8, edx


; 340  :         j5 *= k;       // add-substituted multiplication
	mov	eax, _jj
	add	eax, _j5
	mov	_jj, eax

; c1 = C[jj];
; s1 = S[jj];
	mov	edi, [ebp+20]
	fld	qword[edi+eax*8]
	mov	esi, [ebp+8]
	shl	esi, 2
	add	esi, edi
	fld	qword[esi+eax*8]	; st : s1, c1

; t5 = f[l2] * c1 + f[l6] * s1;
; t8 = f[l6] * c1 - f[l2] * s1;
	mov	edx, _l6
	fld	qword[ebx+edx*8]
	mov	edx, _l2
	fld	st0
	fmul	st0, st2
	fxch	st1
	fmul	st0, st3
	fld	qword[ebx+edx*8]	; st : f[l2], f[l6]*c, f[l6]*s, s, c
	fmul	st4, st0
	fmulp	st3, st0		; st : f[l6]*c, f[l6]*s, f[l2]*s, f[l2]*c
	fsub	st0, st2		; st :   t8,    f[l6]*s, f[l2]*s, f[l2]*c
	fstp	_t8
	faddp	st2, st0		; st :  f[l2]*s, t5
	fstp	st0			; st :  t5
	fstp	_t5			; st :  <empty>

; c2 = C[2*jj];
; s2 = S[2*jj];
	shl	eax, 1
	fld	qword[edi+eax*8]
	fld	qword[esi+eax*8]	; st : s2, c2

; t6 = f[l3] * c2 + f[l7] * s2;
; t9 = f[l7] * c2 - f[l3] * s2;
	mov	edx, _l7
	fld	qword[ebx+edx*8]
	mov	edx, _l3
	fld	st0
	fmul	st0, st2
	fxch	st1
	fmul	st0, st3
	fld	qword[ebx+edx*8]	; st : f[l3], f[l7]*c, f[l7]*s, s, c
	fmul	st4, st0
	fmulp	st3, st0		; st : f[l7]*c, f[l7]*s, f[l3]*s, f[l3]*c
	fsub	st0, st2		; st :   t9,    f[l7]*s, f[l3]*s, f[l3]*c
	fstp	_t9
	faddp	st2, st0		; st :  f[l2]*s, t6
	fstp	st0			; st :  t6
	fstp	_t6			; st :  <empty>

; c3 = C[3*jj];
; s3 = S[3*jj];
	add	eax, _jj
	fld	qword[edi+eax*8]
	fld	qword[esi+eax*8]	; st : s3, c3

; t7 = f[l4] * c3 + f[l8] * s3;
; t0 = f[l8] * c3 - f[l4] * s3;
	mov	edx, _l8
	fld	qword[ebx+edx*8]
	mov	edx, _l4
	fld	st0
	fmul	st0, st2
	fxch	st1
	fmul	st0, st3
	fld	qword[ebx+edx*8]	; st : f[l4], f[l8]*c, f[l8]*s, s, c
	fmul	st4, st0
	fmulp	st3, st0		; st : f[l8]*c, f[l8]*s, f[l4]*s, f[l4]*c
	fsub	st0, st2		; st :   t9,    f[l8]*s, f[l4]*s, f[l4]*c
	fstp	_t0
	faddp	st2, st0		; st : f[l2]*s, t7
	fstp	st0			; st :  t7
	fstp	_t7			; st :  <empty>

; t1 = f[l5] - t9;
; t2 = f[l5] + t9;
	mov	eax, _l5
	fld	qword [ebx+eax*8]
	fld	_t9
	fld	st0
	fadd	st0, st2
	fstp	_t2
	fsubp	st1, st0
	fstp	_t1

; t3 = - t8  - t0;
	fld	_t8
	fadd	_t0
	fchs
	fstp	_t3
; t4 =   t5  - t7;
	fld	_t5
	fsub	_t7
	fstp	_t4

; f[l5] = t1 + t4;
	fld	_t1
	fld	_t4
	fld	st0
	fadd	st0, st2
	fstp	qword [ebx+eax*8]
; f[l7] = t1 - t4;
	mov	eax, _l7
	fsubp	st1, st0
	fstp	qword [ebx+eax*8]

; f[l6] = t2 + t3;
	mov	eax, _l6
	fld	_t2
	fld	_t3
	fld	st0
	fadd	st0, st2
	fstp	qword [ebx+eax*8]
; f[l8] = t2 - t3;
	mov	eax, _l8
	fsubp	st1, st0
	fstp	qword [ebx+eax*8]

; t1 = f[l1] + t6;
	mov	eax, _l1
	fld	qword [ebx+eax*8]
	fld	_t6
	fld	st0
	fadd	st0, st2
	fstp	_t1
; t2 = f[l1] - t6;
	fsubp	st1, st0
	fstp	_t2

; t3 =    t8 - t0;
	fld	_t8
	fsub	_t0
	fstp	_t3
; t4 =    t5 + t7;
	fld	_t5
	fadd	_t7
	fstp	_t4

; f[l1] = t1 + t4;
	mov	eax, _l1
	fld	_t1
	fld	_t4
      fld     st0
	fadd	st0, st2
	fstp	qword [ebx+eax*8]
; f[l3] = t1 - t4;
	mov	eax, _l3
	fsubp	st1, st0
	fstp	qword [ebx+eax*8]

; f[l2] = t2 + t3;
	mov	eax, _l2
	fld	_t2
	fld	_t3
	fld	st0
	fadd	st0, st2
	fstp	qword [ebx+eax*8]
; f[l4] = t2 - t3;
	mov	eax, _l4
	fsubp	st1, st0
	fstp	qword [ebx+eax*8]

; 374  :       }
	jmp	.next_k

.done_k:
; 375  :     }
	add	ebx, _d6	; d6 = d5*8
	cmp	ebx, _end_of_array
	jb	.next_j

; 376  :   }
	mov	cx, _step
	jmp	.newstep
.done:
	mov	esp, ebp
	pop	ebp
; 377  : }
	ret


		;=========== Step3 ends here ===========


; =================================================================

;=================================================================
; parameters:
; -- [ebp+8]   = N
; -- [ebp+12]  = p
; -- [ebp+16]  = 4k-aligned data array  address
; -- [ebp+20]  = 4k-aligned SinCosTable address
; returns:
; -- nothing
; destroys:
; -- all GPRegs
;; ==========================

align 4

FHT_4:

	push	ebp
	mov	ebp, esp
	mov	edx, [ebp+16]
	add	edx, [ebp+12]
	call BitInvert
	push	dword[ebp+16]
	push	dword[ebp+8]
	call	step1
	call	step2
	pop	edx		; N
	pop	ecx		; a
	push	dword[ebp+20]	; t
	push	ecx
	push	dword[ebp+12]	; p
	push	edx		; N
	call	step3
	mov	esp, ebp
	pop	ebp

ret
