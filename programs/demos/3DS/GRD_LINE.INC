;-procedure draws smooth shaded lines (I mean interpolation 24 bit--
;-color), with z coord interpolation--------------------------------
;-author: Maciej Guba (www.macgub.hekko.pl)-------------------------
;-in : -------------------------------------------------------------
;----- edi - pointer to screen buffer ------------------------------
;----- esi - pointer to Z buffer -----------------------------------
;------ constans : SIZE_X, SIZE_Y - screen width and height---------
;----------------- ROUND - fixed point shift------------------------
;------ other parameters via stack----------------------------------
smooth_line:
.x1  equ  ebp+4
.y1  equ  ebp+6
.z1  equ  ebp+8
.r1  equ  ebp+10
.g1  equ  ebp+12
.b1  equ  ebp+14
.x2  equ  ebp+16
.y2  equ  ebp+18
.z2  equ  ebp+20
.r2  equ  ebp+22
.g2  equ  ebp+24
.b2  equ  ebp+26


.line_lenght	equ ebp-2
.delta		equ ebp-6
.delta_x	equ ebp-10
.delta_y	equ ebp-14
.dr		equ ebp-18
.dg		equ ebp-22
.db		equ ebp-26
.dz		equ ebp-30
.cr		equ ebp-34
.cg		equ ebp-38
.cb		equ ebp-42
.cz		equ ebp-46

;.line_lenght    equ ebp-48
.screen 	equ ebp-52
.zbuffer	equ ebp-56
.ccoord 	equ ebp-60  ;current coordinate
.czbuf		equ ebp-64
.cscr		equ ebp-68
;.lasty          equ ebp-72
macro .update_cur_var
{
if Ext=NON
     mov	 ebx,[.dz]
     add	 [.cz],ebx
     mov	 ebx,[.dr]
     add	 [.cr],ebx
     mov	 ebx,[.dg]
     add	 [.cg],ebx
     mov	 ebx,[.db]
     add	 [.cb],ebx
elseif Ext=MMX
     movq	 mm0,[.cz]
     movq	 mm1,[.cg]
     paddd	 mm0,mm2 ;[.dz]
     paddd	 mm1,mm3 ;[.dg]
     movq	 [.cz],mm0
     movq	 [.cg],mm1
elseif Ext >= SSE2
     movups	 xmm1,[.cz]
     paddd	 xmm1,xmm0
     movups	 [.cz],xmm1
end if
}
macro .draw_pixel
{
    mov 	[esi],ebx	       ; actualize Z buffer
;if Ext=SSE2
;    movups      xmm2,[.cb]
   ; shufps      xmm1,xmm1,11000110b
   ; pand        xmm1,[.mask]
;    psrld       xmm2,ROUND
;    packssdw    xmm2,xmm2
;    packuswb    xmm2,xmm2
;    movss       [edi],xmm2
;else

    mov 	eax,[.cb]
    sar 	eax,ROUND
    mov 	[edi],al
;    and         eax,0x000000ff         ; clean unused bits
    mov 	ebx,[.cg]
    sar 	ebx,ROUND
    mov 	[edi+1],bl
;    mov         ah,bl
    mov 	edx,[.cr]
    sar 	edx,ROUND
    mov 	[edi+2],dl
;end if
;    shl         ebx,16
;    or          eax,ebx
;    mov         [edi],eax
}
macro .sort
{

if Ext >= MMX
    movq	mm0,[.x1]
    movq	mm1,[.x2]
    movq	[.x1],mm1
    movq	[.x2],mm0
else
    mov 	edx,[.x1]
    xchg	edx,[.x2]
    mov 	[.x1],edx
    mov 	edx,[.z1]
    xchg	edx,[.z2]
    mov 	[.z1],edx
end if
    mov 	edx,[.g1]
    xchg	edx,[.g2]
    mov 	[.g1],edx
}



    emms
    mov 	ebp,esp
    sub 	esp,128
    mov 	eax,[.x1]      ; check if parameters exceedes screen area
    mov 	ebx,[.x2]
    or		eax,ebx
    test	eax,80008000h
    jne 	.end_line
    cmp 	word[.x1],SIZE_X
    jg		.end_line
    cmp 	word[.x2],SIZE_X
    jg		.end_line
    cmp 	word[.y1],SIZE_Y
    jg		.end_line
    cmp 	word[.y2],SIZE_Y
    jg		.end_line


    mov 	[.screen],edi
    mov 	cx,[.x1]
    cmp 	cx,[.x2]
    je		.vertical_l
    mov 	cx,[.y1]
    cmp 	cx,[.y2]
    je		.horizontal_l
    mov 	ax,[.x1]
    sub 	ax,[.x2]
    cmp 	ax,0
    jg		@f
    neg 	ax    ; calc absolute value
 @@:
    mov 	[.delta_x],ax
    mov 	bx,[.y1]
    sub 	bx,[.y2]
    cmp 	bx,0
    jg		@f
    neg 	bx
 @@:
    mov 	[.delta_y],bx
    cmp 	ax,bx
    je		.deg45_l
    jl		.more_vertical_l
    jg		.more_horizon_l
    jmp 	.end_line
								  ;
.horizontal_l:
    mov 	ax,[.x1]
    mov 	bx,[.x2]
    cmp 	bx,ax
    jge 	@f

    .sort
@@:

    mov 	bx,[.x2]
    sub 	bx,[.x1]
    movsx	ebx,bx
    cmp 	ebx,0	 ;line lenght equql 0
    je		.end_line
    mov 	[.delta_x],ebx

    call	.calc_delta

    mov 	eax,SIZE_X
    movsx	ebx,word[.y1]
    mul 	ebx
    add 	esi,eax
    lea 	eax,[eax*3]
    add 	esi,eax
    add 	edi,eax
    movsx	eax,word[.x1]
    add 	esi,eax
    lea 	eax,[eax*3]
    add 	edi,eax
    add 	esi,eax

    mov 	ecx,[.delta_x]

    movsx	ebx,word[.r1]
    shl 	ebx,ROUND
    mov 	[.cr],ebx
    movsx	ebx,word[.g1]
    shl 	ebx,ROUND
    mov 	[.cg],ebx
    movsx	ebx,word[.b1]
    shl 	ebx,ROUND
    mov 	[.cb],ebx
    movsx	ebx,word[.z1]
    shl 	ebx,ROUND
    mov 	[.cz],ebx
.hdraw:
    mov 	ebx,[.cz]
    cmp 	[esi],ebx
    jle 	.skip

    .draw_pixel

.skip:
    add 	edi,3
    add 	esi,4

    .update_cur_var

    loop	.hdraw
    jmp 	.end_line

.vertical_l:
    mov 	ax,[.y1]
    cmp 	[.y2],ax
    jge 	@f

    .sort
@@:
    mov 	bx,[.y2]
    sub 	bx,[.y1]
    movsx	ebx,bx
    cmp 	ebx,0
    je		.end_line
    mov 	[.delta_y],ebx

    call	.calc_delta

    mov 	eax,SIZE_X
    movsx	ebx,word[.y1]
    mul 	ebx
    add 	esi,eax
    lea 	eax,[eax*3]
    add 	edi,eax
    add 	esi,eax
    movsx	eax,word[.x1]
    add 	esi,eax
    lea 	eax,[eax*3]
    add 	esi,eax
    add 	edi,eax

    mov 	ecx,[.delta_y]

    movsx	ebx,word[.r1]
    shl 	ebx,ROUND
    mov 	[.cr],ebx
    movsx	ebx,word[.g1]
    shl 	ebx,ROUND
    mov 	[.cg],ebx
    movsx	ebx,word[.b1]
    shl 	ebx,ROUND
    mov 	[.cb],ebx
    movsx	ebx,word[.z1]
    shl 	ebx,ROUND
    mov 	[.cz],ebx
.v_draw:
    mov 	ebx,[.cz]
    cmp 	[esi],ebx
    jle 	@f

    .draw_pixel

@@:
    add 	edi,SIZE_X*3
    add 	esi,SIZE_X*4

    .update_cur_var

    loop	.v_draw
    jmp 	.end_line
.deg45_l:
    mov 	word[.line_lenght],ax
    mov 	ax,[.x1]
    cmp 	[.x2],ax
    jge 	@f

    .sort
@@:
    mov 	bx,[.y2]
    sub 	bx,[.y1]
    movsx	ebx,bx
    cmp 	ebx,0
    je		.end_line
    mov 	[.delta_y],ebx
    mov 	bx,[.x2]
    sub 	bx,[.x1]
    movsx	ebx,bx
    mov 	[.delta_x],ebx

    call	.calc_delta

    mov 	eax,SIZE_X
    movsx	ebx,word[.y1] ;calc begin values in screen and Z buffers
    mul 	ebx
    lea 	ebx,[3*eax]
    add 	edi,ebx
    shl 	eax,2
    add 	esi,eax
    movsx	eax,word[.x1]
    lea 	ebx,[eax*3]
    add 	edi,ebx
    shl 	eax,2
    add 	esi,eax

    movzx	ecx,word[.line_lenght]

    movsx	ebx,word[.r1]
    shl 	ebx,ROUND
    mov 	[.cr],ebx
    movsx	ebx,word[.g1]
    shl 	ebx,ROUND
    mov 	[.cg],ebx
    movsx	ebx,word[.b1]
    shl 	ebx,ROUND
    mov 	[.cb],ebx
    movsx	ebx,word[.z1]
    shl 	ebx,ROUND
    mov 	[.cz],ebx
.d45_draw:
    mov 	ebx,[.cz]
    cmp 	[esi],ebx
    jle 	@f

    .draw_pixel

@@:
    cmp 	dword[.delta_y],0
    jl		@f
    add 	edi,SIZE_X*3+3
    add 	esi,SIZE_X*4+4
    jmp 	.d45_1
@@:
    sub 	edi,(SIZE_X*3)-3
    sub 	esi,(SIZE_X*4)-4
.d45_1:
    .update_cur_var

    loop	.d45_draw
    jmp 	.end_line

.more_vertical_l:
    mov 	word[.line_lenght],bx
    mov 	ax,[.y1]
    cmp 	[.y2],ax
    jge 	@f
    .sort
@@:
    mov 	bx,[.y2]
    sub 	bx,[.y1]
    movsx	ebx,bx
    cmp 	ebx,0
    je		.end_line   ;=======================
    mov 	[.delta_y],ebx

    mov 	ax,[.x2]
    sub 	ax,[.x1]
    cwde
    shl 	eax,ROUND
    cdq
    idiv	ebx
    mov 	[.delta],eax

    call	.calc_delta

    mov 	eax,SIZE_X
    movsx	ebx,word[.y1] ;calc begin values in screen and Z buffers
    mul 	ebx
    lea 	ebx,[3*eax]
    add 	esi,ebx
    add 	esi,eax
    add 	edi,ebx
    mov 	[.cscr],edi
    mov 	[.czbuf],esi

    movzx	ecx,word[.line_lenght]

    movsx	ebx,word[.r1]
    shl 	ebx,ROUND
    mov 	[.cr],ebx
    movsx	ebx,word[.g1]
    shl 	ebx,ROUND
    mov 	[.cg],ebx
    movsx	ebx,word[.b1]
    shl 	ebx,ROUND
    mov 	[.cb],ebx
    movsx	ebx,word[.z1]
    shl 	ebx,ROUND
    mov 	[.cz],ebx
    movsx	ebx,word[.x1]
    shl 	ebx,ROUND
    mov 	[.ccoord],ebx	 ; .ccoord -> x coordinate
.draw_m_v:
    mov 	edi,[.cscr]
    mov 	esi,[.czbuf]
    mov 	eax,[.ccoord]
    sar 	eax,ROUND
    lea 	ebx,[eax*3]
    add 	edi,ebx
    add 	esi,ebx
    add 	esi,eax
    mov 	ebx,[.cz]
    cmp 	[esi],ebx
    jle 	@f

    .draw_pixel

@@:
    mov 	eax,[.delta]
    add 	[.ccoord],eax
    add 	dword[.cscr],SIZE_X*3  ;
    add 	dword[.czbuf],SIZE_X*4
.d_m_v1:

    .update_cur_var

    dec 	ecx
    jnz 	.draw_m_v
    jmp 	.end_line

.more_horizon_l:
    mov 	word[.line_lenght],ax
    mov 	ax,[.x1]
    cmp 	[.x2],ax
    jge 	@f

    .sort
@@:
    mov 	bx,[.x2]
    sub 	bx,[.x1]
    movsx	ebx,bx
    cmp 	ebx,0;=======================
    je		.end_line
    mov 	[.delta_x],ebx

    mov 	ax,[.y2]
    sub 	ax,[.y1]
    cwde
    shl 	eax,ROUND
    cdq
    idiv	ebx
    mov 	[.delta],eax

    call	.calc_delta

 ;calc begin values in screen and Z buffers
    movsx	ebx,word[.x1]
    mov 	eax,ebx
    add 	esi,ebx
    lea 	ebx,[3*ebx]
    add 	esi,ebx
    add 	edi,ebx
    mov 	[.cscr],edi
    mov 	[.czbuf],esi

    movzx	ecx,word[.line_lenght]

    movsx	ebx,word[.r1]
    shl 	ebx,ROUND
    mov 	[.cr],ebx
    movsx	ebx,word[.g1]
    shl 	ebx,ROUND
    mov 	[.cg],ebx
    movsx	ebx,word[.b1]
    shl 	ebx,ROUND
    mov 	[.cb],ebx
    movsx	ebx,word[.z1]
    shl 	ebx,ROUND
    mov 	[.cz],ebx
    movsx	ebx,word[.y1]
    shl 	ebx,ROUND
    mov 	[.ccoord],ebx	 ; .ccoord -> y coordinate

.draw_m_h:
    mov 	edi,[.cscr]
    mov 	esi,[.czbuf]
    mov 	eax,[.ccoord]	 ; ccoord - cur y coordinate
    sar 	eax,ROUND
    mov 	ebx,SIZE_X
    mul 	ebx
    add 	esi,eax
    lea 	eax,[eax*3]
    add 	esi,eax
    add 	edi,eax
    mov 	ebx,[.cz]
    cmp 	[esi],ebx
    jle 	@f

    .draw_pixel

@@:
    mov 	eax,[.delta]
    add 	[.ccoord],eax
    add 	dword[.cscr],3	;
    add 	dword[.czbuf],4

    .update_cur_var

    dec 	ecx
    jnz 	.draw_m_h

.end_line:
     mov       esp,ebp
     ret       24

.calc_delta:
    mov 	ax,[.z2]
    sub 	ax,[.z1]
    cwde
    shl 	eax,ROUND
    cdq
    idiv	ebx
    mov 	[.dz],eax

    mov 	ax,[.r2]
    sub 	ax,[.r1]
    cwde
    shl 	eax,ROUND
    cdq
    idiv	ebx
    mov 	[.dr],eax

    mov 	ax,[.g2]
    sub 	ax,[.g1]
    cwde
    shl 	eax,ROUND
    cdq
    idiv	ebx
    mov 	[.dg],eax

    mov 	ax,[.b2]
    sub 	ax,[.b1]
    cwde
    shl 	eax,ROUND
    cdq
    idiv	ebx
    mov 	[.db],eax
if Ext=MMX | Ext = SSE
    movq	mm2,[.dz]
    movq	mm3,[.dg]
else if Ext >= SSE2
    movups	xmm0,[.dz]
end if
ret
;align 16
;.mask:
;           dq 0xffffffffffffffff
;           dq 0xffffffff00000000













