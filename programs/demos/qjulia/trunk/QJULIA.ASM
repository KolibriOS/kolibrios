; App written by randall ported to KolibriOS and MenuetOS64 by macgub (www.macgub.hekko.pl).
; Now it use static memory, it is mixed 32bit code and SSE2 instructions.

include '../../../macros.inc'

use32

	       org    0x0

	       db     'MENUET01'	      ; 8 byte id
	       dd     0x01		      ; header version
	       dd     START		      ; start of code
	       dd     IMG_END			; size of image
	       dd     I_END ;0x100000                ; memory for app
	       dd     I_END ;0xbffff                 ; esp
	       dd     0x0 , 0x0 	      ; I_Param , I_Icon

START:				; start of execution

     call draw_window
     call Main
     call draw_from_buffer

still:

;    mov  eax,23                ; wait here for event
;    mov  ebx,timeout
;    int  0x40
;    mov eax,11                 ; check for event no wait
    mov eax,10			; wait for event
    int 0x40

    cmp  eax,1			; redraw request ?
    je	 red
    cmp  eax,2			; key in buffer ?
    je	 key
    cmp  eax,3			; button in buffer ?
    je	 button

    jmp  noclose

  red:				; redraw
    call draw_window
    call draw_from_buffer
    jmp  still

  key:				; key
    mov  eax,2			; just read it and ignore
    int  0x40
    shr  eax,8
    cmp  eax, 27
    jne  still
    mov  eax, -1
    int  0x40


  button:			; button
    mov  eax,17 		; get id
    int  0x40

    cmp  ah,1			; button id=1 ?
    jne  noclose

    mov  eax,-1 		; close this program
    int  0x40
  noclose:

    jmp  still


draw_from_buffer:

	    mov eax,7
	    mov ebx,screen4
	    mov ecx,IMG_WIDTH*65536+IMG_HEIGHT
	    mov edx,0*65536+0
	    int 0x40
ret


;-------------------------------------------------------------------------------
; NAME:     shuf
;-------------------------------------------------------------------------------
macro	    shuf	d*,s*,z*,y*,x*,w* {
	    shufps	d,s,(z shl 6) or (y shl 4) or (x shl 2) or w
}
;-------------------------------------------------------------------------------
; NAME:     length3
; IN:       xmm0.xyz    input vector
; OUT:      xmm0.xyzw   vector length
;-------------------------------------------------------------------------------
macro	    length3	{
	    mulps	xmm0,xmm0
	    movaps	xmm1,xmm0
	    movaps	xmm2,xmm0
	    shufps	xmm0,xmm0,0x00
	    shufps	xmm1,xmm1,0x55
	    shufps	xmm2,xmm2,0xaa
	    addps	xmm0,xmm1
	    addps	xmm0,xmm2
	    sqrtps	xmm0,xmm0
}
;-------------------------------------------------------------------------------
; NAME:     length4
; IN:       xmm0.xyzw   input vector
; OUT:      xmm0.xyzw   vector length
;-------------------------------------------------------------------------------
macro	    length4	{
	    mulps	xmm0,xmm0
	    movaps	xmm1,xmm0
	    movaps	xmm2,xmm0
	    movaps	xmm3,xmm0
	    shufps	xmm0,xmm0,0x00
	    shufps	xmm1,xmm1,0x55
	    shufps	xmm2,xmm2,0xaa
	    shufps	xmm3,xmm3,0xff
	    addps	xmm0,xmm1
	    addps	xmm2,xmm3
	    addps	xmm0,xmm2
	    sqrtps	xmm0,xmm0
}
;-------------------------------------------------------------------------------
; NAME:     normalize3
; IN:       xmm0.xyz    input vector
; OUT:      xmm0.xyz    normalized vector
;-------------------------------------------------------------------------------
macro	    normalize3	{
	    movaps	xmm3,xmm0
	    mulps	xmm0,xmm0
	    movaps	xmm1,xmm0
	    movaps	xmm2,xmm0
	    shufps	xmm0,xmm0,0x00
	    shufps	xmm1,xmm1,0x55
	    shufps	xmm2,xmm2,0xaa
	    addps	xmm0,xmm1
	    addps	xmm0,xmm2
	    rsqrtps	xmm0,xmm0
	    mulps	xmm0,xmm3
}
;-------------------------------------------------------------------------------
; NAME:     cross
; IN:       xmm0.xyz    first input vector
; IN:       xmm1.xyz    second input vector
; OUT:      xmm0.xyz    cross product result
;-------------------------------------------------------------------------------
macro	    cross	{
	    movaps	xmm2,xmm0
	    movaps	xmm3,xmm1
	    shuf	xmm0,xmm0,3,0,2,1
	    shuf	xmm1,xmm1,3,1,0,2
	    shuf	xmm2,xmm2,3,1,0,2
	    shuf	xmm3,xmm3,3,0,2,1
	    mulps	xmm0,xmm1
	    mulps	xmm2,xmm3
	    subps	xmm0,xmm2
}
;-------------------------------------------------------------------------------
; NAME:     dot3
; IN:       xmm0.xyz    first input vector
; IN:       xmm1.xyz    second input vector
; OUT:      xmm0.xyzw   dot product result
;-------------------------------------------------------------------------------
macro	    dot3	{
	    mulps	xmm0,xmm1
	    movaps	xmm1,xmm0
	    movaps	xmm2,xmm0
	    shufps	xmm0,xmm0,0x00
	    shufps	xmm1,xmm1,0x55
	    shufps	xmm2,xmm2,0xaa
	    addps	xmm0,xmm1
	    addps	xmm0,xmm2
}
;-------------------------------------------------------------------------------
; NAME:     dot4
; IN:       xmm0.xyzw   first input vector
; IN:       xmm1.xyzw   second input vector
; OUT:      xmm0.xyzw   dot product result
;-------------------------------------------------------------------------------
macro	    dot4	{
	    mulps	xmm0,xmm1
	    movaps	xmm1,xmm0
	    movaps	xmm2,xmm0
	    movaps	xmm3,xmm0
	    shufps	xmm0,xmm0,0x00
	    shufps	xmm1,xmm1,0x55
	    shufps	xmm2,xmm2,0xaa
	    shufps	xmm3,xmm3,0xff
	    addps	xmm0,xmm1
	    addps	xmm2,xmm3
	    addps	xmm0,xmm2
}
;-------------------------------------------------------------------------------
; NAME:     qsq
; IN:       xmm0.xyzw   quaternion
; OUT:      xmm0.xyzw   squared quaternion
;-------------------------------------------------------------------------------
macro	    qsq 	{
	    movaps	xmm3,xmm0
	    movaps	xmm4,xmm0
	    movaps	xmm1,xmm0
	    dot3
	    shufps	xmm3,xmm3,0xff
	    movaps	xmm5,xmm3
	    mulps	xmm3,xmm3
	    subps	xmm3,xmm0   ; r.___w
	    mulps	xmm4,xmm5
	    addps	xmm4,xmm4   ; r.xyz_
	    andps	xmm3,dqword [g_ClearXYZ]
	    andps	xmm4,dqword [g_ClearW]
	    orps	xmm3,xmm4
	    movaps	xmm0,xmm3
}
;-------------------------------------------------------------------------------
; NAME:     qmul
; IN:       xmm0.xyzw   first quaternion
; IN:       xmm1.xyzw   second quaternion
; OUT:      xmm0.xyzw   first quaternion multiplied by second quaternion
;-------------------------------------------------------------------------------
macro	    qmul	{
	    movaps	xmm6,xmm0	; q1.xyzw
	    movaps	xmm7,xmm1	; q2.xyzw
	    dot3
	    movaps	xmm4,xmm6
	    movaps	xmm5,xmm7
	    shufps	xmm4,xmm4,0xff	; q1.wwww
	    shufps	xmm5,xmm5,0xff	; q2.wwww
	    movaps	xmm1,xmm4
	    mulps	xmm1,xmm5
	    subps	xmm1,xmm0
	    andps	xmm1,dqword [g_ClearXYZ]
	    movaps	[xxmm8],xmm1		       ; r.000w
	    movaps	xmm0,xmm6
	    movaps	xmm1,xmm7
	    cross
	    mulps	xmm7,xmm4
	    mulps	xmm6,xmm5
	    addps	xmm6,xmm7
	    addps	xmm0,xmm6
	    andps	xmm0,dqword [g_ClearW]
	    orps	xmm0,[xxmm8]
}
;-------------------------------------------------------------------------------
; NAME:     logss
; IN:       xmm0.x      function argument
; OUT:      xmm0.x      function result
;-------------------------------------------------------------------------------
macro	    logss	{
	    maxss	xmm0,[g_MinNormPos]
	    movss	xmm1,[g_1_0]
	    movd	edx,xmm0
	    andps	xmm0,dqword [g_InvMantMask]
	    orps	xmm0,xmm1
	    movaps	xmm4,xmm0
	    subss	xmm0,xmm1
	    addss	xmm4,xmm1
	    shr 	edx,23
	    rcpss	xmm4,xmm4
	    mulss	xmm0,xmm4
	    addss	xmm0,xmm0
	    movaps	xmm2,xmm0
	    mulss	xmm0,xmm0
	    sub 	edx,0x7f
	    movss	xmm4,[g_log_p0]
	    movss	xmm6,[g_log_q0]
	    mulss	xmm4,xmm0
	    movss	xmm5,[g_log_p1]
	    mulss	xmm6,xmm0
	    movss	xmm7,[g_log_q1]
	    addss	xmm4,xmm5
	    addss	xmm6,xmm7
	    movss	xmm5,[g_log_p2]
	    mulss	xmm4,xmm0
	    movss	xmm7,[g_log_q2]
	    mulss	xmm6,xmm0
	    addss	xmm4,xmm5
	    movss	xmm5,[g_log_c0]
	    addss	xmm6,xmm7
	    cvtsi2ss	xmm1,edx
	    mulss	xmm0,xmm4
	    rcpss	xmm6,xmm6
	    mulss	xmm0,xmm6
	    mulss	xmm0,xmm2
	    mulss	xmm1,xmm5
	    addss	xmm0,xmm2
	    addss	xmm0,xmm1
}

;-------------------------------------------------------------------------------
; NAME:     QJuliaDist
; IN:       xmm0.xyz    position
; OUT:      xmm0.xyzw   distance to the nearest point in quaternion julia set
;-------------------------------------------------------------------------------
align 64
QJuliaDist:
	    Z		equ ebp-16
	    Zp		equ ebp-32
	    NormZ	equ ebp-36
	    NormZp	equ ebp-40
	    push	ebp
	    mov 	ebp,esp
	    sub 	esp,64
	    ; init Z and Zp
	    andps	xmm0,dqword [g_ClearW]
	    shuf	xmm0,xmm0,0,3,2,1
	    movups	[Z],xmm0
	    movaps	xmm1,dqword [g_UnitW]
	    movups	[Zp],xmm1
	    ; iterate
	    mov 	ecx,10
.Iterate:
	    ; compute and update Zp
	    movups	xmm0,[Z]
	    movups	xmm1,[Zp]
	    qmul
	    addps	xmm0,xmm0
	    movups	[Zp],xmm0
	    ; compute and update Z
	    movups	xmm0,[Z]
	    qsq
	    addps	xmm0,dqword [g_Quat]
	    movups	[Z],xmm0
	    ; check if squared length of Z is greater than g_EscapeThreshold,
	    ; break the loop if it is
	    movaps	xmm1,xmm0
	    dot4
	    movss	xmm1,[g_EscapeThreshold]
	    cmpltss	xmm1,xmm0
	    movd	eax,xmm1
	    cmp 	eax,0xffffffff
	    je		.IterateEnd
	    ; continue the loop
	    sub 	ecx,1
	    test	ecx,ecx
	    jnz 	.Iterate
.IterateEnd:
	    movups	xmm0,[Z]
	    length4
	    movss	[NormZ],xmm0
	    movups	xmm0,[Zp]
	    length4
	    movss	[NormZp],xmm0
	    movss	xmm0,[NormZ]
	    logss
	    divss	xmm0,[NormZp]
	    mulss	xmm0,[NormZ]
	    mulss	xmm0,[g_0_5]
	    shufps	xmm0,xmm0,0x00
	    mov 	esp,ebp
	    pop 	ebp
	   ; restore     Z,Zp,NormZ,NormZp
	    ret
;-------------------------------------------------------------------------------
; NAME:     Map
; IN:       xmm0.xyz    position
; OUT:      xmm0.xyzw   distance to the nearest object from input position
; OUT:      eax         material ID of the nearest object
;-------------------------------------------------------------------------------
align 64
Map:
	  ;  P           equ ebp-16
	  ;  MinDist     equ ebp-32
	    MatID	equ ebp-40
	    push	ebp
	    mov 	ebp,esp
	    sub 	esp,128
	    movaps	[.P],xmm0
	    movaps	xmm0,dqword [g_255_0]
	    movaps	[.MinDist],xmm0
	    mov 	dword [MatID],0
	    ; QJulia
	    movaps	xmm0,[.P]
	    call	QJuliaDist
	    movaps	xmm1,xmm0
	    cmpltps	xmm1,[.MinDist]
	    movd	eax,xmm1
	    cmp 	eax,0xffffffff
	    jne 	@f
	    movaps	[.MinDist],xmm0
	    mov 	dword [MatID],4
@@:
	    ; sphere
	    movaps	xmm0,[.P]
	    subps	xmm0,dqword [g_UnitY]
	    length3
	    subps	xmm0,dqword [g_1_0]
	    movaps	xmm1,xmm0
	    cmpltps	xmm1,[.MinDist]
	    movd	eax,xmm1
	    cmp 	eax,0xffffffff
	    jne 	@f
	    ;movaps      [MinDist],xmm0
	    ;mov         dword [MatID],1
@@:
	    ; plane
	    movaps	xmm0,[.P]
	    movaps	xmm1,dqword [g_UnitY]
	    dot3
	    addps	xmm0,dqword [g_1_0]
	    movaps	xmm1,xmm0
	    cmpltps	xmm1,[.MinDist]
	    movd	eax,xmm1
	    cmp 	eax,0xffffffff
	    jne 	@f
	    movaps	[.MinDist],xmm0
	    mov 	dword [MatID],2
@@:
	    ; box
	    xorps	xmm0,xmm0
	    subps	xmm0,[.P]
	    maxps	xmm0,[.P]
	    subps	xmm0,dqword [g_BoxSize]
	    maxps	xmm0,dqword [g_0_0]
	    length3
	    subps	xmm0,dqword [g_BoxEdge]
	    movaps	xmm1,xmm0
	    cmpltps	xmm1,[.MinDist]
	    movd	eax,xmm1
	    cmp 	eax,0xffffffff
	    jne 	@f
	    ;movaps      [MinDist],xmm0
	    ;mov         dword [MatID],3
@@:
	    movaps	xmm0,[.MinDist]
	    mov 	eax,[MatID]
	    mov 	esp,ebp
	    pop 	ebp
	    ret
align 16
.P:
   rd 4
.MinDist:
   rd 4

       ;     restore     P,MinDist,MatID
;-------------------------------------------------------------------------------
; NAME:     CastRay
; IN:       xmm0.xyz    ray origin
; IN:       xmm1.xyz    ray direction
; OUT:      xmm0.xyzw   distance from ray orgin to the nearest intersected object
;                       or -1.0 if there is no intersection
;-------------------------------------------------------------------------------
align 64
CastRay:
     ;       RO          equ ebp-16
     ;       RD          equ ebp-32
     ;       T           equ ebp-48
	    MatID	equ ebp-52
	    push	ebp
	    mov 	ebp,esp
	    sub 	esp,128
	    ; init stack variables
	    movaps	[.RO],xmm0
	    movaps	[.RD],xmm1
	    xorps	xmm0,xmm0
	    movaps	[.T],xmm0
.March:
	    ; find distance to the nearest object
	    movaps	xmm0,[.RD]
	    mulps	xmm0,[.T]
	    addps	xmm0,[.RO]
	    call	Map
	    mov 	[MatID],eax
	    ; return if distance is less than g_HitDist
	    movaps	xmm1,xmm0
	    cmpltps	xmm1,dqword [g_HitDist]
	    movd	eax,xmm1
	    cmp 	eax,0xffffffff
	    je		.Hit
	    ; increment T with distance to the nearest object
	    movaps	xmm1,[.T]
	    addps	xmm1,xmm0
	    movaps	[.T],xmm1
	    ; continue loop only if distance is less than g_MaxDist
	    cmpltps	xmm1,dqword [g_MaxDist]
	    movd	eax,xmm1
	    cmp 	eax,0xffffffff
	    je		.March
	    xorps	xmm0,xmm0
	    subps	xmm0,dqword [g_1_0]
	    movaps	[.T],xmm0
	    mov 	dword [MatID],0
.Hit:
	    movaps	xmm0,[.T]
	    mov 	eax,[MatID]
	    mov 	esp,ebp
	    pop 	ebp
	    ret
align 16
.RO:
   rd 4
.RD:
   rd 4
.T:
   rd 4
      ;      restore     RO,RD,T,MatID
;-------------------------------------------------------------------------------
; NAME:     CastShadowRay
; IN:       xmm0.xyz    ray origin
; IN:       xmm1.xyz    ray direction
; OUT:      xmm0.xyzw   visibility factor [0.0, 1.0],
;                       0.0 means path is fully blocked,
;                       1.0 means path is fully clear
;-------------------------------------------------------------------------------
align 64
CastShadowRay:
	 ;   RO          equ ebp-16
	 ;   RD          equ ebp-32
	    R		equ ebp-48
	 ;   T           equ ebp-64
	    push	ebp
	    mov 	ebp,esp
	    sub 	esp,128
	    ; init stack variables
	    movaps	[.RO],xmm0
	    movaps	[.RD],xmm1
	    movaps	xmm0,dqword [g_0_01]
	    movaps	[.T],xmm0
	    movaps	xmm0,dqword [g_1_0]
	    movups	[R],xmm0
.March:
	    ; find distance to the nearest object
	    movaps	xmm0,[.RD]
	    mulps	xmm0,[.T]
	    addps	xmm0,[.RO]
	    call	Map
	    ; return 0.0 if distance is less than g_ShadowHitDist
	    movaps	xmm1,xmm0
	    cmpltps	xmm1,dqword [g_ShadowHitDist]
	    movd	eax,xmm1
	    cmp 	eax,0xffffffff
	    je		.Hit
	    ; compute R
	    movaps	xmm1,xmm0
	    rcpps	xmm2,[.T]
	    mulps	xmm1,xmm2
	    mulps	xmm1,dqword [g_16_0]
	    movups	xmm2,[R]
	    minps	xmm2,xmm1
	    movups	[R],xmm2
	    ; increment T with distance to the nearest object
	    movaps	xmm1,[.T]
	    addps	xmm1,xmm0
	    movaps	[.T],xmm1
	    ; continue loop only if distance is less than g_ShadowMaxDist
	    cmpltps	xmm1,dqword [g_ShadowMaxDist]
	    movd	eax,xmm1
	    cmp 	eax,0xffffffff
	    je		.March
	    ; return (R,R,R,R)
	    movups	xmm0,[R]
	    mov 	esp,ebp
	    pop 	ebp
	    ret
.Hit:
	    ; return (0,0,0,0)
	    xorps	xmm0,xmm0
	    mov 	esp,ebp
	    pop 	ebp
	    ret
	;    restore     RO,RD,R,T
align 16
.RO:
   rd 4
.RD:
   rd 4
.T:
   rd 4
;-------------------------------------------------------------------------------
; NAME:     ComputeNormal
; IN:       xmm0.xyz    position
; OUT:      xmm0.xyz    normal vector
;-------------------------------------------------------------------------------
align 64
ComputeNormal:
	    P		equ ebp-16
	    N		equ ebp-32
	    push	ebp
	    mov 	ebp,esp
	    sub 	esp,128
	    movups	[P],xmm0
	    ; compute x coordinate
	    addps	xmm0,dqword [g_NormalDX]
	    call	Map
	    movss	[N+0],xmm0
	    movups	xmm0,[P]
	    subps	xmm0,dqword [g_NormalDX]
	    call	Map
	    movss	xmm1,[N+0]
	    subss	xmm1,xmm0
	    movss	[N+0],xmm1
	    ; compute y coordinate
	    movups	xmm0,[P]
	    addps	xmm0,dqword [g_NormalDY]
	    call	Map
	    movss	[N+4],xmm0
	    movups	xmm0,[P]
	    subps	xmm0,dqword [g_NormalDY]
	    call	Map
	    movss	xmm1,[N+4]
	    subss	xmm1,xmm0
	    movss	[N+4],xmm1
	    ; compute z coordinate
	    movups	xmm0,[P]
	    addps	xmm0,dqword [g_NormalDZ]
	    call	Map
	    movss	[N+8],xmm0
	    movups	xmm0,[P]
	    subps	xmm0,dqword [g_NormalDZ]
	    call	Map
	    movss	xmm1,[N+8]
	    subss	xmm1,xmm0
	    movss	[N+8],xmm1
	    ; normalize
	    movups	xmm0,[N]
	    normalize3
	    mov 	esp,ebp
	    pop 	ebp
	    ret
	  ;  restore     P,N
;-------------------------------------------------------------------------------
; NAME:     Shade
; IN:       xmm0.xyz    position
; IN:       xmm1.xyz    normal vector
; IN:       edi         material ID
; OUT:      xmm0.xyz    color
;-------------------------------------------------------------------------------
align 64
Shade:
     ;       P           equ ebp-16
     ;       N           equ ebp-32
     ;       RGB         equ ebp-48
	    C		equ ebp-64
	    NdotL	equ ebp-80
	    L		equ ebp-96
     ;       AOScale     equ ebp-112
     ;       AO          equ ebp-128
	    Temp	equ ebp-144
	    Idx 	equ ebp-148
	    MatID	equ ebp-152
	    push	ebp
	    mov 	ebp,esp
	    sub 	esp,256
	    movaps	[.P],xmm0
	    movaps	[.N],xmm1
	    mov 	[MatID],edi
	    ;
	    ; AO
	    ;
	    xorps	xmm0,xmm0
	    movaps	[.AO],xmm0
	    movaps	xmm0,dqword [g_10_0]
	    movaps	[.AOscale],xmm0
	    mov 	dword [Idx],0
.AOLoop:
	    cvtsi2ss	xmm0,[Idx]
	    shufps	xmm0,xmm0,0x00
	    mulps	xmm0,xmm0
	    mulps	xmm0,dqword [g_0_015]
	    addps	xmm0,dqword [g_0_01]
	    movups	[Temp],xmm0
	    mulps	xmm0,[.N]
	    addps	xmm0,[.P]
	    call	Map
	    movups	xmm1,[Temp]
	    subps	xmm1,xmm0
	    mulps	xmm1,[.AOscale]
	    movaps	xmm0,[.AO]
	    addps	xmm0,xmm1
	    movaps	[.AO],xmm0
	    movaps	xmm0,[.AOscale]
	    mulps	xmm0,dqword [g_0_5]
	    movaps	[.AOscale],xmm0
	    add 	dword [Idx],1
	    cmp 	dword [Idx],5
	    jne 	.AOLoop
	    movaps	xmm0,[.AO]
	    maxps	xmm0,dqword [g_0_0]
	    minps	xmm0,dqword [g_1_0]
	    movaps	xmm1,dqword [g_1_0]
	    subps	xmm1,xmm0
	    movaps	[.AO],xmm1
	    ;
	    ; Material ID Switch
	    ;
	    mov 	edi,[MatID]
	    cmp 	edi,1
	    je		.Mat1
	    cmp 	edi,2
	    je		.Mat2
	    jmp 	.MatDef
.Mat1:
	    movaps	xmm0,dqword [g_Red]
	    movaps	[.RGB],xmm0
	    jmp 	.MatBreak
.Mat2:
	    movaps	xmm0,dqword [g_Green]
	    movaps	[.RGB],xmm0
	    jmp 	.MatBreak
.MatDef:
	    movaps	xmm0,dqword [g_1_0]
	    movaps	[.RGB],xmm0
.MatBreak:
	    ;
	    ; Light0 Contribution
	    ;
	    ; compute light vector and "N dot L" value
	    movaps	xmm0,dqword [g_L0Pos]
	    subps	xmm0,[.P]
	    normalize3
	    movups	[L],xmm0
	    movups	xmm1,[N]
	    dot3
	    movups	[NdotL],xmm0
	    ; cast shadow ray
	    movaps	xmm0,[.P]
	    movups	xmm1,[L]
	    call	CastShadowRay
	    movups	xmm1,[NdotL]
	    mulps	xmm0,xmm1
	    mulps	xmm0,dqword [g_0_6]
	    addps	xmm0,dqword [g_0_4]
	    maxps	xmm0,dqword [g_0_0]
	    mulps	xmm0,dqword [g_0_7]
	    mulps	xmm0,dqword [.RGB]
	    movups	[C],xmm0
	    ;
	    ; Light1 Contribution
	    ;
	    ; compute light vector and "N dot L" value
	    movaps	xmm0,dqword [g_L1Pos]
	    subps	xmm0,[.P]
	    normalize3
	    movups	[L],xmm0
	    movaps	xmm1,[.N]
	    dot3
	    movups	[NdotL],xmm0
	    ; cast shadow ray
	    movaps	xmm0,[.P]
	    movups	xmm1,[L]
	    call	CastShadowRay
	    movups	xmm1,[NdotL]
	    mulps	xmm0,xmm1
	    mulps	xmm0,dqword [g_0_6]
	    addps	xmm0,dqword [g_0_4]
	    maxps	xmm0,dqword [g_0_0]
	    mulps	xmm0,dqword [g_0_3]
	    mulps	xmm0,dqword [.RGB]
	    movups	xmm1,[C]
	    addps	xmm0,xmm1
	    mulps	xmm0,[.AO]
	    mov 	esp,ebp
	    pop 	ebp
	    ret
	 ;   restore     P,N,C,RGB,NdotL,L,AOScale,AO,Temp,Idx,MatID
align 16
.N:
    rd 4
.P:
    rd 4
.RGB:
    rd 4
.AOscale:
    rd 4
.AO:
    rd 4
;-------------------------------------------------------------------------------
; NAME:     ComputeColor
; IN:       xmm0.x      normalized x coordinate
; IN:       xmm0.y      normalized y coordinate
; OUT:      xmm0.xyz    pixel color
;-------------------------------------------------------------------------------
align 64
ComputeColor:
	;    X           equ ebp-32
	;    Y           equ ebp-48
	;    RD          equ ebp-64
	    P		equ ebp-80
	    N		equ ebp-96
	    MatID	equ ebp-100
	    push	ebp
	    mov 	ebp,esp
	    sub 	esp,128
	    ; save function parameters on the stack
	    shufps	xmm0,xmm0,0x00
	    shufps	xmm1,xmm1,0x00
	    movaps	[.X],xmm0
	    movaps	[.Y],xmm1
	    ; compute z axis
	    xorps	xmm0,xmm0
	    subps	xmm0,dqword [g_CamPos]
	    normalize3
	    movaps	xmm7,xmm0
	    ; compute x axis
	    movaps	xmm0,dqword [g_UnitY]
	    movaps	xmm1,xmm7
	    cross
	    normalize3
	    movaps	xmm6,xmm0
	    ; compute y axis
	    movaps	xmm0,xmm7
	    movaps	xmm1,xmm6
	    cross
	    normalize3
	    ; compute ray direction
	    mulps	xmm0,[.Y]
	    mulps	xmm6,[.X]
	    movaps	xmm1,xmm7
	    mulps	xmm1,dqword [g_0_5]
	    addps	xmm7,xmm1
	    addps	xmm0,xmm6
	    addps	xmm0,xmm7
	    normalize3
	    movaps	[.RD],xmm0
	    ; cast ray
	    movaps	xmm0,dqword [g_CamPos]
	    movaps	xmm1,[.RD]
	    call	CastRay
	    mov 	[MatID],eax
	    ; return if there is no intersection
	    movaps	xmm1,xmm0
	    cmpltps	xmm0,dqword [g_0_0]
	    movd	eax,xmm0
	    cmp 	eax,0xffffffff
	    je		.Return
	    ; compute intersection point
	    movaps	xmm0,[.RD]
	    mulps	xmm0,xmm1
	    addps	xmm0,dqword [g_CamPos]
	    movups	[P],xmm0
	    ; compute normal vector
	    call	ComputeNormal
	    movups	[N],xmm0
	    ; shade
	    mov 	edi,[MatID]
	    movups	xmm0,[P]
	    movups	xmm1,[N]
	    call	Shade
.Return:
	    mov 	esp,ebp
	    pop 	ebp
	    ret
       ;     restore     X,Y,RD,P,N,MatID
align 16
.X:
   rd 4
.Y:
   rd 4
.RD:
   rd 4
;-------------------------------------------------------------------------------
; NAME:     Main
; DESC:     Program entry point.
;-------------------------------------------------------------------------------
align 64
Main:
	    ImgPtr	equ ebp-8
	    Img8Ptr	equ ebp-16
	    X		equ ebp-20
	    Y		equ ebp-24
	    mov 	ebp,esp
	    sub 	esp,128
	    ;
	    ; generate floating point image
	    ;
	    mov 	dword [ImgPtr],screen16
	    mov 	ebx,screen16
	    ; begin loops
	    mov 	dword [Y],0
.LoopY:
	    mov 	dword [X],0
.LoopX:
	    ; compute normalized x coordinate [-1.777 , 1.777]
	    cvtsi2ss	xmm0,[X]
	    divss	xmm0,[g_ImgWidth]
	    subss	xmm0,[g_0_5]
	    addss	xmm0,xmm0
	    mov 	eax,1.777
	    movd	xmm2,eax
	    mulss	xmm0,xmm2
	    ; compute normalized y coordinate [-1.0 , 1.0]
	    cvtsi2ss	xmm1,[Y]
	    divss	xmm1,[g_ImgHeight]
	    subss	xmm1,[g_0_5]
	    addss	xmm1,xmm1
	    ; compute and write pixel color to the buffer
	    call	ComputeColor
	    movaps	[ebx],xmm0
	    ; advance pixel pointer
	    add 	ebx,16
	    ; continue .LoopX
	    add 	dword [X],1
	    cmp 	dword [X],IMG_WIDTH
	    jne 	.LoopX
	    ; continue .LoopY
	    add 	dword [Y],1
	    cmp 	dword [Y],IMG_HEIGHT
	    jne 	.LoopY
	    ;
	    ; convert image to 4 x 8 bpp
	    ;
	    mov 	dword [Img8Ptr],screen4
	    ; set dst and src pointers and loop counter
	    mov 	edi,screen4
	    mov 	esi,[ImgPtr]
	    mov 	ecx,IMG_WIDTH*IMG_HEIGHT
@@:
	    ; clamp to [0.0 ,1.0]
	    movaps	xmm0,[esi]
	    maxps	xmm0,dqword [g_0_0]
	    minps	xmm0,dqword [g_1_0]
	    ; convert from [0.0 ,1.0] to [0 ,255]
	    mulps	xmm0,dqword [g_255_0]
	    cvttps2dq	xmm0,xmm0
      ;      pshufb      xmm0,dqword [g_ImageConvMask]
	    ; set alpha to 0xff
	    movd	[edi],xmm0
	    or		dword [edi],0xff000000
	    ; continue loop
	    add 	esi,16
	    add 	edi,4
	    sub 	ecx,1
	    jnz 	@b
	    ;convert    to 24 bit per pixel
	    mov 	esi,screen4
	    mov 	edi,esi
	    mov 	ecx,IMG_WIDTH*IMG_HEIGHT
	    cld
       @@:
	    lodsd
	    mov 	ah,al
	    push	ax
	    ror 	eax,16
	    pop 	ax
	    stosd
	    dec 	edi
	    loop	@b

	    mov 	esp,ebp

ret
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:

    mcall 12, 1                                   ; function 12:tell os about windowdraw
    
	mcall 48, 4                                   ;get skin width
	lea	ecx, [50*65536+IMG_HEIGHT+4+eax]            ; [y start] *65536 + [y size] + [skin_height]
	mcall	0,<50,IMG_WIDTH+9>,,0x74000000,,labelt ;draw window

    mcall 12, 2                                   ; function 12:tell os about windowdraw

    ret



;-------------------------------------------------------------------------------
labelt:
 db  'qjulia',0
labelen:

align 4
IMG_WIDTH		= 800
IMG_HEIGHT		= 600
g_ImgWidth		dd 800.0
g_ImgHeight		dd 600.0
g_EscapeThreshold	dd 16.0
g_MinNormPos		dd 0x00800000
g_log_p0		dd -0.789580278884799154124
g_log_p1		dd 16.3866645699558079767
g_log_p2		dd -64.1409952958715622951
g_log_q0		dd -35.6722798256324312549
g_log_q1		dd 312.093766372244180303
g_log_q2		dd -769.691943550460008604
g_log_c0		dd 0.693147180559945

align 16
;g_ImageConvMask         db 8,4,0,12,12 dup 0x80
g_InvMantMask		dd 4 dup (not 0x7f800000)
g_0_0			dd 4 dup 0.0
g_0_5			dd 4 dup 0.5
g_1_0			dd 4 dup 1.0
g_255_0 		dd 4 dup 255.0
g_16_0			dd 4 dup 128.0
g_10_0			dd 4 dup 10.0
g_0_6			dd 4 dup 0.6
g_0_4			dd 4 dup 0.4
g_0_7			dd 4 dup 0.7
g_0_3			dd 4 dup 0.3
g_0_01			dd 4 dup 0.01
g_0_015 		dd 4 dup 0.015
g_CamPos		dd 1.2,1.4,1.2,1.0
g_UnitY 		dd 0.0,1.0,0.0,0.0
g_UnitW 		dd 0.0,0.0,0.0,1.0
g_HitDist		dd 4 dup 0.001
g_ShadowHitDist 	dd 4 dup 0.0005
g_ShadowMaxDist 	dd 4 dup 10.0
g_MaxDist		dd 4 dup 40.0
g_NormalDX		dd 0.001,0.0,0.0,0.0
g_NormalDY		dd 0.0,0.001,0.0,0.0
g_NormalDZ		dd 0.0,0.0,0.001,0.0
g_L0Pos 		dd 10.0,8.0,-6.0,1.0
g_L1Pos 		dd -12.0,19.0,6.0,1.0
g_Red			dd 1.0,1.0,1.0,1.0
g_Green 		dd 1.0,1.0,1.0,1.0
g_BoxSize		dd 1.5,1.0,1.5,1.0
g_BoxEdge		dd 4 dup 0.03
g_ClearXYZ		dd 0x00000000,0x00000000,0x00000000,0xffffffff
g_ClearW		dd 0xffffffff,0xffffffff,0xffffffff,0x00000000
g_Quat			dd 0.2,0.0,0.0,-1.0
IMG_END:
align 16
screen16		rb IMG_WIDTH*IMG_HEIGHT*16
align 16
screen4 		rb IMG_WIDTH*IMG_HEIGHT*4
align 16
xxmm8:
			rd 4
memStack:
			rd 65536
align 16
I_END:
