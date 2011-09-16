

CATMULL_SHIFT  equ 8
ROUND equ 8
;NON=0
;MMX=1
;Ext=MMX
;TEX_SIZE=0x3fff
;SIZE_X equ 512
;SIZE_Y equ 512
;ROUND = 8
;TEX_SHIFT equ 6

; procedure drawing textured triangle with Gouraud shading
; Z-buffer alghoritm included, Z coord interpolation ----
; I set the color by this way -- (col1 * col2)/256 ------
;------------------in - eax - x1 shl 16 + y1 ------------
;---------------------- ebx - x2 shl 16 + y2 ------------
;---------------------- ecx - x3 shl 16 + y3 ------------
;---------------------- esi - pointer to Z-buffer--------
;---------------------- edx - pointer to texture---------
;---------------------- Z-buffer filled with dd variables
;---------------------- shifted CATMULL_SHIFT------------
;---------------------- edi - pointer to screen buffer---
;---------------------- stack : colors-------------------



tex_plus_grd_triangle:
; parameters :
   .tex_y3  equ [ebp+38]   ; 36 bytes through stack
   .tex_x3  equ [ebp+36]
   .tex_y2  equ [ebp+34]
   .tex_x2  equ [ebp+32]
   .tex_y1  equ [ebp+30]
   .tex_x1  equ [ebp+28]

   .z3	    equ [ebp+26]
   .col3b   equ [ebp+24]
   .col3g   equ [ebp+22]
   .col3r   equ [ebp+20]

   .z2	    equ [ebp+18]
   .col2b   equ [ebp+16]
   .col2g   equ [ebp+14]
   .col2r   equ [ebp+12]

   .z1	    equ [ebp+10]
   .col1b   equ [ebp+8]
   .col1g   equ [ebp+6]
   .col1r   equ [ebp+4]

; local variables:

   .tex_ptr   equ dword[ebp-4]
   .z_ptr     equ dword[ebp-8]
   .scr_buff  equ dword[ebp-12]

   .x1	      equ  word[ebp-14] ;dw ? ;equ word[ebp-10]
   .y1	      equ  word[ebp-16] ;dw ? ;equ word[ebp-12]
   .x2	      equ  word[ebp-18] ;dw ? ;equ word[ebp-14]
   .y2	      equ  word[ebp-20] ;dw ? ;equ word[ebp-16]
   .x3	      equ  word[ebp-22] ;dw ? ;equ word[ebp-18]
   .y3	      equ  word[ebp-24] ;dw ? ;equ word[ebp-20]

   .dx12      equ  dword[ebp-28] ;dd ?
   .tex_dx12  equ  dword[ebp-32] ;dd ?
   .tex_dy12  equ	[ebp-36] ;dd ?
   .dz12      equ  dword[ebp-40] ;dd ?
   .dc12r     equ	[ebp-44] ;dd ?
   .dc12g     equ  dword[ebp-48] ;dd ?
   .dc12b     equ	[ebp-52] ;dd ?

   .dx23      equ  dword[ebp-56] ;dd ?
   .tex_dx23  equ  dword[ebp-60] ;dd ?
   .tex_dy23  equ	[ebp-64] ;dd ?
   .dz23      equ  dword[ebp-68] ;dd ?
   .dc23r     equ	[ebp-72] ;dd ?
   .dc23g     equ  dword[ebp-76] ;dd ?
   .dc23b     equ	[ebp-80] ;dword[ebp-8]dd ?

   .dx13      equ  dword[ebp-84] ;dd ?
   .tex_dx13  equ  dword[ebp-88] ;dd ?
   .tex_dy13  equ	[ebp-92] ;dd ?
   .dz13      equ  dword[ebp-96] ;dd ?
   .dc13r     equ	[ebp-100] ;dd ?
   .dc13g     equ  dword[ebp-104] ;dd ?
   .dc13b     equ	[ebp-108] ;dd ?

   .scan_x1   equ  dword[ebp-112] ;dd ?
   .scan_y1   equ	[ebp-116] ;dd ?
   .zz1       equ  dword[ebp-120] ;dw ?
   .cur1r     equ	[ebp-124] ;dw ?
   .cur1g     equ  dword[ebp-128] ;dw ?
   .cur1b     equ	[ebp-132] ;dw ?

   .scan_x2   equ  dword[ebp-136] ;dd ?
   .scan_y2   equ	[ebp-140] ;dd ?
   .zz2       equ  dword[ebp-144] ;dw ?
   .cur2r     equ	[ebp-148] ;dw ?
   .cur2g     equ  dword[ebp-152] ;dw ?
   .cur2b     equ	[ebp-156] ;dw ?


       mov    ebp,esp

   ;    mov     .tex_ptr,edx
   ;    mov     .z_ptr,esi
   ;    mov     .scr_buff,edi
	 push	 edx esi edi
;         push    esi
;         push    edi
	 mov	  edx,80008000h  ; eax,ebx,ecx are ANDd together into edx which means that
	 and	  edx,ebx	 ; if *all* of them are negative a sign flag is raised
	 and	  edx,ecx
	 and	  edx,eax
	 test	  edx,80008000h  ; Check both X&Y at once
	 jne	  .loop2_end

   .sort3:
       cmp     ax,bx
       jle     .sort1
       xchg    eax,ebx
if Ext>=MMX
       movq    mm0, .col1r	 ; exchange r, g, b, z
       movq    mm1, .col2r
       movq    .col1r ,mm1
       movq    .col2r ,mm0
else
       mov     edx,dword .col1r   ; exchange both r and g
       xchg    edx,dword .col2r
       mov     dword .col1r ,edx

       mov     edx,dword .col1b   ; b and z
       xchg    edx,dword .col2b
       mov     dword .col1b ,edx
end if

       mov     edx,dword .tex_x1
       xchg    edx,dword .tex_x2
       mov     dword .tex_x1 ,edx

 .sort1:
       cmp	bx,cx
       jle	.sort2
       xchg    ebx,ecx

if Ext>=MMX
       movq    mm0, .col2r	 ; exchange r, g, b, z
       movq    mm1, .col3r
       movq    .col3r ,mm0
       movq    .col2r ,mm1
else

       mov     edx,dword .col2r  ; r, g
       xchg    edx,dword .col3r
       mov     dword .col2r,edx

       mov     edx,dword .col2b  ; b, z
       xchg    edx,dword .col3b
       mov     dword .col2b,edx
end if

       mov     edx,dword .tex_x2
       xchg    edx,dword .tex_x3
       mov     dword .tex_x2,edx

       jmp .sort3

 .sort2:

       push	eax ebx ecx	    ; store in variables
  ;     push     ebx
  ;     push     ecx

;****************** delta computng zone **************
;+++++++++ first zone
       mov	bx,.y2	     ; calc delta12
       sub	bx,.y1
       jnz	.dx12_make
       mov	ecx,7
     @@:
       push	dword 0
       loop	@b
       jmp	.dx12_done
  .dx12_make:


       mov	ax,.x2
       sub	ax,.x1
       cwde
       movsx	ebx,bx
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dx12,eax
       push	 eax

if  0  ; Ext=SSE
       movd	 mm0,.col1r    ; 2 words  r, g
       pxor	 mm1,mm1
       punpcklwd mm0,mm1
       cvtpi2ps  xmm0,mm0
       movlhps	 xmm0,xmm0
       movd	 mm0,.col1g    ; 2 words  b, z
       punpcklwd mm0,mm1
       cvtpi2ps  xmm0,mm0
       ; xmm0=four float double words
       divss	 xmm0,.pack3
       ;convert and insert mm0 to lower xmm1     ..
end if

       mov	ax,word .tex_x2
       sub	ax,word .tex_x1
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .tex_dx12r,eax
       push	  eax

       mov	  ax,word .tex_y2
       sub	  ax,word .tex_y1
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .tex_dx12,eax
       push	eax

       mov	ax,word .z2
       sub	ax,word .z1
       cwde
       shl	eax,CATMULL_SHIFT
       cdq
       idiv	ebx
 ;      mov      .dz12,eax
       push	eax	  ; .dza12

       mov	ax,word .col2r
       sub	ax,word .col1r
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dc12r,eax
       push	  eax

       mov	  ax,word .col2g
       sub	  ax,word .col1g
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dc12g,eax
       push	eax

       mov	ax,word .col2b	       ;;---
       sub	ax,word .col1b
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .dc12b,eax
       push	eax

;+++++++++++++++++ second zone +++++++++++++
   .dx12_done:

       mov	bx,.y3	     ; calc delta23
       sub	bx,.y2
       jnz	.dx23_make
       mov	ecx,7
    @@:
       push	dword 0
       loop	@b
       jmp	.dx23_done

  .dx23_make:
       mov	ax,.x3
       sub	ax,.x2
       cwde
       movsx	ebx,bx
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dx23,eax
       push	 eax

       mov	ax,word .tex_x3
       sub	ax,word .tex_x2
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .tex_dx23,eax
       push	  eax

       mov	  ax,word .tex_y3
       sub	  ax,word .tex_y2
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .tex_dy23,eax
       push	eax

       mov	ax,word .z3
       sub	ax,word .z2
       cwde				   ;
       shl	eax,CATMULL_SHIFT	   ;  2222222
       cdq				   ;  2     2
       idiv	ebx			   ;        2
;       mov      .dz23,eax                 ;       2
       push	eax	  ; .dza12         ;      2
					   ;     2
       mov	ax,word .col3r		   ;    2
       sub	ax,word .col2r		   ;   2222222
       cwde				   ;  second delta
       shl	eax,ROUND		   ;
       cdq				   ;
       idiv	ebx			   ;
;       mov      .dc23r,eax                 ;
       push	  eax

       mov	  ax,word .col3g
       sub	  ax,word .col2g
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .dc23g,eax
       push	eax

       mov	ax,word .col3b	       ;;---
       sub	ax,word .col2b
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .dc23b,eax
       push	eax

   .dx23_done:
;++++++++++++++++++third zone++++++++++++++++++++++++
       mov	bx,.y3	     ; calc delta13
       sub	bx,.y1
       jnz	.dx13_make
       mov	ecx,7
     @@:
       push	dword 0
       loop	@b
       jmp	.dx13_done
  .dx13_make:
       mov	ax,.x3
       sub	ax,.x1
       cwde
       movsx	ebx,bx
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .dx13,eax
       push	 eax

       mov	ax,word .tex_x3 	   ; triangle b
       sub	ax,word .tex_x1
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .tex_dx13r,eax
       push	  eax

       mov	  ax,word .tex_y3
       sub	  ax,word .tex_y1
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .tex_dy13,eax
       push	eax

       mov	ax,word .z3
       sub	ax,word .z1	       ;    333333333
       cwde			       ;   3        3
       shl	eax,CATMULL_SHIFT      ;            3
       cdq			       ;            3
       idiv	ebx		       ;            3
;       mov      .dz13,eax             ;            3
       push	eax	  ; .dza12     ;            3
				       ;            3
       mov	ax,word .col3r	       ;    3333333333
       sub	ax,word .col1r	       ;            3
       cwde			       ;            3
       shl	eax,ROUND	       ;            3
       cdq			       ;            3
       idiv	ebx		       ;            3
  ;    mov      .dc13r,eax             ;    3       3
       push	  eax		      ;     33333333

       mov	  ax,word .col3g
       sub	  ax,word .col1g
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dc13g,eax
       push	eax

       mov	ax,word .col3b	       ;;---
       sub	ax,word .col1b
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
;       mov      .dc13b,eax
       push	eax

   .dx13_done:

; <<<<<<<  ::delta zone end+++++++++++++++++++++ >>>>>>>>
       sub	esp,55	 ;(12*4)

       movsx	eax,.x1 		   ; eax - cur x1
       shl	eax,ROUND		   ; ebx - cur x2
       mov	ebx,eax
       movsx	edx,word .z1
       shl	edx,CATMULL_SHIFT
       mov	.zz1,edx
       mov	.zz2,edx

       movzx	edi,word .col1r
       shl	edi,ROUND
       mov	.cur1r,edi
       mov	.cur2r,edi
       movzx	esi,word .col1g
       shl	esi,ROUND
       mov	.cur1g,esi
       mov	.cur2g,esi
       movzx	edx,word .col1b
       shl	edx,ROUND
       mov	.cur1b,edx
       mov	.cur2b,edx

       movzx	edi,word .tex_x1
       shl	edi,ROUND
       mov	.scan_x1,edi
       mov	.scan_x2,edi
       movzx	edx,word .tex_y1
       shl	edx,ROUND
       mov	.scan_y1,edx
       mov	.scan_y2,edx

       mov	cx,.y1
       cmp	cx,.y2
       jge	.loop1_end
    .loop_1:
    ;   push     eax ebx ebp
       pushad

       push	.tex_ptr
       push	.scr_buff
       push	.z_ptr
       push	cx

       push	.zz2

       push	.scan_x2
       push	dword .scan_y2
       push	dword .cur2r
       push	.cur2g
       push	dword .cur2b

       push	.zz1

       push	.scan_x1
       push	dword .scan_y1
       push	dword .cur1r
       push	.cur1g
       push	dword .cur1b

       sar	eax,ROUND
       sar	ebx,ROUND
       call	horizontal_tex_grd_line

      ; pop      ebp ebx eax
       popad

if (Ext = MMX)|(Ext=SSE)
       movq	mm0,.cur1b
       movq	mm1,.cur1r
       movq	mm2,.scan_y1
       movq	mm3,.cur2b
       movq	mm4,.cur2r
       movq	mm5,.scan_y2
       paddd	mm0,.dc13b
       paddd	mm1,.dc13r
       paddd	mm2,.tex_dy13
       paddd	mm3,.dc12b
       paddd	mm4,.dc12r
       paddd	mm5,.tex_dy12
       movq	.cur1b,mm0
       movq	.cur1r,mm1
       movq	.scan_y1,mm2
       movq	.cur2b,mm3
       movq	.cur2r,mm4
       movq	.scan_y2,mm5
end if
if Ext >= SSE2
       movups	xmm0,.cur1b
       movups	xmm1,.dc13b
       movups	xmm2,.cur2b
       movups	xmm3,.dc12b
       movq	mm2,.scan_y1
       movq	mm5,.scan_y2
       paddd	xmm0,xmm1
       paddd	xmm2,xmm3
       paddd	mm2,.tex_dy13
       paddd	mm5,.tex_dy12
       movq	.scan_y1,mm2
       movq	.scan_y2,mm5
       movups	.cur1b,xmm0
       movups	.cur2b,xmm2
end if

if Ext = NON
       mov	edx,.dc13b
       add	.cur1b,edx
       mov	esi,.dc13g
       add	.cur1g,esi
       mov	edi,.dc13r
       add	.cur1r,edi
       mov	edx,.dz13
       add	.zz1,edx
       mov	edx,.tex_dx13
       add	.scan_x1,edx
       mov	esi,.tex_dy13
       add	.scan_y1,esi

       mov	edi,.dc12b
       add	.cur2b,edi
       mov	esi,.dc12g
       add	.cur2g,esi
       mov	edx,.dc12r
       add	.cur2r,edx
       mov	edi,.tex_dx12
       add	.scan_x2,edi
       mov	esi,.tex_dy12
       add	.scan_y2,esi
       mov	edx,.dz12
       add	.zz2,edx
end if
       add	eax,.dx13
       add	ebx,.dx12
       inc	cx
       cmp	cx,.y2
       jl      .loop_1
 .loop1_end:
       movzx	ecx,.y2
       cmp	cx,.y3
       jge	.loop2_end

       movsx	ebx,.x2 		   ; eax - cur x1
       shl	ebx,ROUND		   ; ebx - cur x2

       movsx	edx,word .z2
       shl	edx,CATMULL_SHIFT
;       mov      .zz1,edx
       mov	.zz2,edx

       movzx	edi,word .col2r
       shl	edi,ROUND
   ;    mov      .cur1r,edi
       mov	.cur2r,edi
       movzx	esi,word .col2g
       shl	esi,ROUND
   ;    mov      .cur1g,esi
       mov	.cur2g,esi
       movzx	edx,word .col2b
       shl	edx,ROUND
   ;    mov      .cur1b,edx
       mov	.cur2b,edx

       movzx	edi,word .tex_x2
       shl	edi,ROUND
 ;      mov      .scan_x1,edi
       mov	.scan_x2,edi
       movzx	edx,word .tex_y2
       shl	edx,ROUND
 ;      mov      .scan_y1,edx
       mov	.scan_y2,edx

  .loop_2:
       pushad

       push	.tex_ptr
       push	.scr_buff
       push	.z_ptr
       push	cx

       push	.zz2

       push	.scan_x2
       push	dword .scan_y2
       push	dword .cur2r
       push	.cur2g
       push	dword .cur2b

       push	.zz1

       push	.scan_x1
       push	dword .scan_y1
       push	dword .cur1r
       push	.cur1g
       push	dword .cur1b

       sar	eax,ROUND
       sar	ebx,ROUND
       call	horizontal_tex_grd_line

       popad

if (Ext = MMX)|(Ext=SSE)
       movq	mm0,.cur1b
       movq	mm1,.cur1r
       movq	mm2,.scan_y1
       movq	mm3,.cur2b
       movq	mm4,.cur2r
       movq	mm5,.scan_y2
       paddd	mm0,.dc13b
       paddd	mm1,.dc13r
       paddd	mm2,.tex_dy13
       paddd	mm3,.dc23b
       paddd	mm4,.dc23r
       paddd	mm5,.tex_dy23
       movq	.cur1b,mm0
       movq	.cur1r,mm1
       movq	.scan_y1,mm2
       movq	.cur2b,mm3
       movq	.cur2r,mm4
       movq	.scan_y2,mm5
end if
if Ext >= SSE2
       movups	xmm0,.cur1b
       movups	xmm1,.dc13b
       movups	xmm2,.cur2b
       movups	xmm3,.dc23b
       movq	mm2,.scan_y1
       movq	mm5,.scan_y2
       paddd	xmm0,xmm1
       paddd	xmm2,xmm3
       paddd	mm2,.tex_dy13
       paddd	mm5,.tex_dy23
       movq	.scan_y1,mm2
       movq	.scan_y2,mm5
       movups	.cur1b,xmm0
       movups	.cur2b,xmm2
end if
if Ext = NON
       mov	edx,.dc13b
       add	.cur1b,edx
       mov	esi,.dc13g
       add	.cur1g,esi
       mov	edi,.dc13r
       add	.cur1r,edi
       mov	edx,.tex_dx13
       add	.scan_x1,edx
       mov	esi,.tex_dy13
       add	.scan_y1,esi
       mov	edx,.dz13
       add	.zz1,edx

       mov	edi,.dc23b
       add	.cur2b,edi
       mov	esi,.dc23g
       add	.cur2g,esi
       mov	edx,.dc23r
       add	.cur2r,edx
       mov	edi,.tex_dx23
       add	.scan_x2,edi
       mov	esi,.tex_dy23
       add	.scan_y2,esi
       mov	edx,.dz23
       add	.zz2,edx
end if
       add	eax,.dx13
       add	ebx,.dx23
       inc	cx
       cmp	cx,.y3
       jl	.loop_2

.loop2_end:
       mov	esp,ebp
ret 36
horizontal_tex_grd_line:
;in:
; eax : x1, ebx : x2

.tex_ptr  equ [ebp+62]
.screen   equ [ebp+58]
.z_buffer equ [ebp+54]
.y	  equ [ebp+52]

.z2	equ [ebp+48]
.tex_x2 equ [ebp+44]
.tex_y2 equ [ebp+40]
.r2	equ [ebp+36]
.g2	equ [ebp+32]
.b2	equ [ebp+28]

.z1	equ [ebp+24]
.tex_x1 equ [ebp+20]
.tex_y1 equ [ebp+16]
.r1	equ [ebp+12]
.g1	equ [ebp+8]
.b1	equ [ebp+4]

.x1 equ  word[ebp-2]
.x2 equ  word[ebp-4]
.dz equ dword[ebp-8]
.db equ dword[ebp-12]
.dg equ dword[ebp-16]
.dr equ dword[ebp-20]
.dtex_x equ dword[ebp-24]
.dtex_y equ dword[ebp-28]

.c_ty equ [ebp-32]
.c_tx equ [ebp-36]
.cb  equ  [ebp-40]
.cg  equ  [ebp-44]
.cr  equ  [ebp-48]
.t_col equ [ebp-52]

.dtex_yM equ qword[ebp-28]
.drM equ qword[ebp-20]
.dbM equ qword[ebp-12]

	mov	ebp,esp
  ;      sub     esp,30

	mov	cx,word .y
	or	cx,cx
	jl	.quit_l

	cmp	cx,SIZE_Y
	jge	.quit_l

	cmp	ax,bx
	je	.quit_l
	jl	@f

	xchg	eax,ebx

if Ext=NON
	mov	ecx,dword .r1
	xchg	ecx, .r2
	mov	dword .r1, ecx

	mov	ecx,dword .g1
	xchg	ecx, .g2
	mov	dword .g1, ecx

	mov	ecx,dword .b1
	xchg	ecx, .b2
	mov	dword .b1, ecx

	mov	ecx,dword .tex_x1
	xchg	ecx, .tex_x2
	mov	dword .tex_x1, ecx

	mov	ecx,dword .tex_y1
	xchg	ecx, .tex_y2
	mov	dword .tex_y1, ecx

	mov	ecx,dword .z1
	xchg	ecx, .z2
	mov	dword .z1, ecx
end if
if (Ext=MMX)
	movq	mm0,.b1        ; b, g
	movq	mm1,.b2
	movq	.b1, mm1
	movq	.b2, mm0
	movq	mm2,.r1        ; r, y
	movq	mm3,.r2
	movq	.r1,mm3
	movq	.r2,mm2
	movq	mm4,.tex_x1    ; x, z
	movq	mm5,.tex_x2
	movq	.tex_x1,mm5
	movq	.tex_x2,mm4

end if
if Ext>=SSE
       movups  xmm0,.b1
       movups  xmm1,.b2
       movups  .b1,xmm1
       movups  .b2,xmm0
       movq    mm4,.tex_x1    ; x, z
       movq    mm5,.tex_x2
       movq    .tex_x1,mm5
       movq    .tex_x2,mm4
end if

    @@:
	or	bx,bx
	jle	.quit_l
	cmp	ax,SIZE_X
	jge	.quit_l

	push	ax
	push	bx

	mov	eax,.z2 	   ; delta zone************
	sub	eax,.z1
	cdq
	mov	bx,.x2
	sub	bx,.x1
	movsx	ebx,bx
	idiv	ebx
	push	eax  ; .dz

	mov	eax,.b2
	sub	eax,.b1
	cdq
	idiv	ebx
	push	eax  ; .db

	mov	eax,.g2
	sub	eax,.g1
	cdq
	idiv	ebx
	push	eax  ; .dg

	mov	eax,.r2
	sub	eax,.r1
	cdq
	idiv	ebx
	push	eax  ; .dr

	mov	eax,.tex_x2
	sub	eax,.tex_x1
	cdq
	idiv	ebx
	push	eax  ; .dtex_x

	mov	eax,.tex_y2
	sub	eax,.tex_y1
	cdq
	idiv	ebx
	push	eax  ; .dtey_x

	cmp	.x1,0
	jg	@f

	mov	eax,.dz     ; clipping
	movsx	ebx,.x1
	neg	ebx
	imul	ebx
	add	.z1,eax
	mov	.x1,0

	mov    eax,.dr
	imul   ebx
	add    .r1,eax
;if  Ext=NON
	mov    eax,.dg
	imul   ebx
	add    .g1,eax

	mov    eax,.db
	imul   ebx
	add    .b1,eax

	mov    eax,.dtex_x
	imul   ebx
	add    .tex_x1,eax

	mov    eax,.dtex_y
	imul   ebx
	add    .tex_y1,eax
   @@:
	mov	  edx,SIZE_X
	cmp	  .x2,dx
	jl	  @f
	mov	  .x2,dx
    @@:
; calc line addres begin in screen and Z buffer
	movsx	  eax,word .y
	mul	  edx
	movsx	  edx,.x1
	add	  eax,edx

	mov	  esi,eax
	shl	  esi,2
	add	  esi,.z_buffer

	lea	  eax,[eax*3]
	mov	  edi,.screen
	add	  edi,eax

	mov	  cx,.x2
	sub	  cx,.x1
	movzx	  ecx,cx

; init current variables
	push	  dword .tex_y1
;if  Ext=NON
	push	  dword .tex_x1

	push	  dword .b1
	push	  dword .g1
	push	  dword .r1

if Ext>=MMX
	movq	  mm4,.cr  ; lo -> r,g
	movq	  mm6,.cb  ; hi -> b, tex_x
	pxor	  mm0,mm0
end if
	mov	  ebx,.z1
      .ddraw:
	cmp	  ebx,dword[esi]
	jge	  @f
	mov	  eax,.c_ty
;  if ROUND<TEX_SHIFT
;        shl        eax,TEX_SHIFT-ROUND
;  end if
;  if ROUND>TEX_SHIFT
;        shr        eax,ROUND-TEX_SHIFT
;  end if
	shr	  eax,ROUND
	shl	  Eax,TEX_SHIFT
	mov	  edx,.c_tx   ; calc texture pixel mem addres
	shr	  edx,ROUND
	add	  eax,edx
	and	  eax,TEXTURE_SIZE   ; cutting
	lea	  eax,[3*eax]
	add	  eax,.tex_ptr
	mov	  dword[esi],ebx
if    Ext = NON
	mov	  eax,dword[eax]
 ;      mov        .tex_col,eax
	push	  ax
	shl	  eax,8
	pop	  ax
	mov	  edx,.cr
	sar	  edx,ROUND
	mul	  dl	     ; al*dl
	shr	  ax,8
	stosb
	ror	  eax,16
	push	  ax
	mov	  edx,.cg
	sar	  edx,ROUND
	mul	  dl
	shr	  ax,8
	stosb
	pop	  ax
	shr	  ax,8
	mov	  edx,.cb
	sar	  edx,ROUND
	mul	  dl
	shr	  ax,8
	stosb
	jmp	  .no_skip
else
	movd	  mm1,[eax]
	punpcklbw mm1,mm0
	movq	  mm3,mm4  ;.cr     ; lo -> r,g
	movq	  mm5,mm6  ;.cb     ; lo -> b,tex_x
	psrld	  mm3,ROUND  ;
	psrld	  mm5,ROUND  ;
	packssdw  mm3,mm5
	pmullw	  mm1,mm3
	psrlw	  mm1,8
	packuswb  mm1,mm0
	movd	  [edi],mm1
end if
       mov	  dword[esi],ebx
if Ext = NON
       jmp	  .no_skip
end if
     @@:
       add	  edi,3
     .no_skip:
       add	  esi,4
       add	  ebx,.dz

       mov	  eax,.dtex_x
       add	  .c_tx, eax
       mov	  edx,.dtex_y
       add	  .c_ty, edx
if Ext=NON
       mov	  eax,.dr
       add	  .cr,eax
       mov	  edx,.dg
       add	  .cg,edx
       mov	  eax,.db
       add	  .cb,eax

else
	paddd	   mm4,.drM
	paddd	   mm6,.dbM
;;        paddd      mm7,.dtex_y      ; mm4 - b, g
;;        movq       .c_tx,mm7
				    ; mm6 - r, x
end if				    ; mm7 - y, x

       dec	ecx
       jnz	.ddraw

  .quit_l:

	mov	esp,ebp
ret 42+20  ; horizontal line

