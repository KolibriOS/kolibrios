;========================================================================
;=                                                                      =
;=         Fast Hartley Transform  routine demo for KolibriOS           =
;=                                                                      =
;=       Copyright (C) 2010, Artem Jerdev <kolibri@jerdev.co.uk>        =
;=                                                                      =
;=             refer to wiki.kolibtios.org for all details              =
;=                                                                      =
;========================================================================




use32

	       org    0x0

	       db     'MENUET01'	      ; 8 byte id
	       dd     0x01		      ; header version
	       dd     START		      ; start of code
	       dd     I_END		      ; size of image
	       dd     0x100000		      ; memory for app
	       dd     0xbfffc		      ; esp
	       dd     0x0 , 0x0 	      ; I_Param , I_Icon


include 'macros.inc'
include 'debug.inc'
include 'FHT4i.inc'


START:				; start of execution

     call main


    mov  eax,-1 		; close this program
    int  0x40


;=============================================================
;Func: calculates a simple function
;      ff = (int)(500*exp(-t) * cos (2.5*t))
;               uses: eax, ebx
;------------
Func:

; 9    : {

; 10   :   double x,t;
; 11   :   int f;
; 12   :
; 13   :   x = (i < N2) ? i : i - NUM_POINTS;
	mov	eax, [ii]
	cmp	eax, 512
	jge	.index_negative
	jmp	.index_correct
.index_negative:
	sub	eax, 1024
.index_correct:
	mov	[temp], eax
;        fild    [temp]

; 14   :   t = x / 16.0;
; f2xm1 argument (abs) must be less than 1, so
	mov	[t_mod], eax
	and	[t_mod], 0x0F	; x % 16
	shr	eax, 4		; x / 16
	mov	[t_div], eax
	fild	[temp]

; 15   :   if (t<0) t = -t;
	fabs
exp_ok:
; 16   :   f = (int)(512*2^(-t) * cos (2.5*t));
	fchs
	f2xm1
	fmul	[f500]
	fstp	[tv93]
	fld	[f2_5]
	fmul	[tt]
	fcos
	fmul	[tv93]
	fstp	[tt]
	mov	bx, word[tt+6]
	shr	bx,4
	and	bx,0x07FF
	add	ax,bx
	shl	ax,4
	and	word[tt+6], 0x800F
	or	word[tt+6], ax
	fld	[tt]
	fstp   [ff]

; 17   :   return  f;
; 18   : }
	ret
;---------------------------------------------------------
;       test data filler
;
;       uses eax, ebx, ecx
FillData:
; 29   :    for (i=0; i<NUM_POINTS; i++)
; here : ecx = i
	xor	ecx, ecx
.funcloop:
; 30   :    {
; 31   :       ia[i] = Func(i);
	mov	[ii], ecx
	call	Func
	fld	[ff]
	fstp	qword [edx+ecx*8]
; 32   :    }
	inc	ecx
	cmp	ecx, [_in]	   ; ecx == N ?
	jne	.funcloop
	ret

;====================================================================
; main
;====================================================================
align 4

_ia	dd 0
_ii	dd 0
_ip	dd 0
_in	dd 0
_it	dd 0
;-----------------
main:
	 mov	eax, 68
	 mov	ebx, 11
	 int	0x40
	fninit
	 mov	cl,   2 	; power of 4
	 mov	byte[_ip], cl
	 mov	eax, 1
	 shl	eax, cl
	 shl	eax, cl
	 mov	[_in], eax
	 mov	dl, cl
	 call	CreateSinCosTable
	 mov	[_it], edx
	 mov	ecx, [_in]
	 shl	ecx, 3
	 mov	ebx, 12
	 mov	eax, 68
	 int	0x40
	 mov	[_ia], eax
	 mov	edx, eax

	call FillData

	cpuid
	rdtsc
	mov	[t_0], eax

	push	[_it]
	push	[_ia]
	push	[_ip]
	push	[_in]
;        call    FHT_4
	xor	eax, eax
	syscall

	add	esp, 16
	cpuid
	rdtsc
	mov	[t_1], eax

	sub	eax, [t_0]
	debug_print_hex eax
	print	'<- fht time'

	mov	edx, [_it]
	call	DestroySinCosTable
	mov	ecx, [_ia]
	mov	ebx, 13
	mov	eax, 68
	int	0x40
	ret


; ========================================================
; static data
;----------------

align 8

;f18     dq 0x4032000000000000
f256	dq 256.01f
f14_2	dq 14.2f
f500	dq 0x407f400000000000
f2_5	dq 0x4004000000000000
tt	dq ?
tv93	dq ?
t_div	dd ?
t_mod	dd ?
temp	dd ?
ff	dq ?	; return value (int)
ii	dd ?	; argument (int) = array index
t_1	dd ?
t_0	dd ?
fcontrol dw  0x0037f
title	db ' Fast Hartley Transform Test - A.Jerdev 2010'

I_END:
