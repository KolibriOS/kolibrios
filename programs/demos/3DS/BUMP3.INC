;------- Big thanks to majuma (www.majuma.xt.pl) for absolutelly great--
;------- 13h mode demos ------------------------------------------------

bump_triangle:
;------------------in - eax - x1 shl 16 + y1 -----------
;---------------------- ebx - x2 shl 16 + y2 -----------
;---------------------- ecx - x3 shl 16 + y3 -----------
;---------------------- edx - pointer to bump map ------
;---------------------- esi - pointer to environment map
;---------------------- edi - pointer to screen buffer--
;---------------------- stack : bump coordinates--------
;----------------------         environment coordinates-
.b_x1 equ ebp+4     ; procedure don't save registers !!!
.b_y1 equ ebp+6     ; each coordinate as word
.b_x2 equ ebp+8
.b_y2 equ ebp+10
.b_x3 equ ebp+12
.b_y3 equ ebp+14
.e_x1 equ ebp+16
.e_y1 equ ebp+18
.e_x2 equ ebp+20
.e_y2 equ ebp+22
.e_x3 equ ebp+24
.e_y3 equ ebp+26

.t_bmap equ dword[ebp-4]
.t_emap equ dword[ebp-8]
.x1	equ word[ebp-10]
.y1	equ word[ebp-12]
.x2	equ word[ebp-14]
.y2	equ word[ebp-16]
.x3	equ word[ebp-18]
.y3	equ word[ebp-20]

.dx12  equ dword[ebp-24]
.dbx12 equ dword[ebp-28]
.dby12 equ dword[ebp-32]
.dex12 equ dword[ebp-36]
.dey12 equ dword[ebp-40]

.dx13  equ dword[ebp-44]
.dbx13 equ dword[ebp-48]
.dby13 equ dword[ebp-52]
.dex13 equ dword[ebp-56]
.dey13 equ dword[ebp-60]

.dx23  equ dword[ebp-64]
.dbx23 equ dword[ebp-68]
.dby23 equ dword[ebp-72]
.dex23 equ dword[ebp-76]
.dey23 equ dword[ebp-80]

.cx1   equ dword[ebp-84]   ; current variables
.cx2   equ dword[ebp-88]
.cbx1  equ dword[ebp-92]
.cbx2  equ dword[ebp-96]
.cby1  equ dword[ebp-100]
.cby2  equ dword[ebp-104]
.cex1  equ dword[ebp-108]
.cex2  equ dword[ebp-112]
.cey1  equ dword[ebp-116]
.cey2  equ dword[ebp-120]

       mov     ebp,esp
       push    edx	  ; store bump map
       push    esi	  ; store e. map
     ; sub     esp,120
 .sort3:		  ; sort triangle coordinates...
       cmp     ax,bx
       jle     .sort1
       xchg    eax,ebx
       mov     edx,dword[.b_x1]
       xchg    edx,dword[.b_x2]
       mov     dword[.b_x1],edx
       mov     edx,dword[.e_x1]
       xchg    edx,dword[.e_x2]
       mov     dword[.e_x1],edx
 .sort1:
       cmp	bx,cx
       jle	.sort2
       xchg	ebx,ecx
       mov	edx,dword[.b_x2]
       xchg	edx,dword[.b_x3]
       mov	dword[.b_x2],edx
       mov	edx,dword[.e_x2]
       xchg	edx,dword[.e_x3]
       mov	dword[.e_x2],edx
       jmp	.sort3
 .sort2:
       push	eax	; store triangle coords in variables
       push	ebx
       push	ecx

       mov     edx,eax	       ; eax,ebx,ecx are ORd together into edx which means that
       or      edx,ebx	       ; if any *one* of them is negative a sign flag is raised
       or      edx,ecx
       test    edx,80000000h   ; Check only X
       jne     .loop23_done

       cmp     .x1,SIZE_X    ; {
       jg      .loop23_done
       cmp     .x2,SIZE_X     ; This can be optimized with effort
       jg      .loop23_done
       cmp     .x3,SIZE_X
       jg      .loop23_done    ; {


       mov	bx,.y2	     ; calc delta 12
       sub	bx,.y1
       jnz	.bt_dx12_make
       mov	ecx,5
       xor	edx,edx
     @@:
       push	edx   ;dword 0
       loop	@b
       jmp	.bt_dx12_done
 .bt_dx12_make:
       mov	ax,.x2
       sub	ax,.x1
       cwde
       movsx	ebx,bx
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dx12,eax
       push	 eax

       mov	ax,word[.b_x2]
       sub	ax,word[.b_x1]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dbx12,eax
       push	 eax

       mov	ax,word[.b_y2]
       sub	ax,word[.b_y1]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dby12,eax
       push	 eax

       mov	ax,word[.e_x2]
       sub	ax,word[.e_x1]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dex12,eax
       push	 eax

       mov	ax,word[.e_y2]
       sub	ax,word[.e_y1]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dey12,eax
       push	 eax
   .bt_dx12_done:

       mov	bx,.y3	     ; calc delta13
       sub	bx,.y1
       jnz	.bt_dx13_make
       mov	ecx,5
       xor	edx,edx
     @@:
       push	edx   ;dword 0
       loop	@b
       jmp	.bt_dx13_done
 .bt_dx13_make:
       mov	ax,.x3
       sub	ax,.x1
       cwde
       movsx	ebx,bx
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dx13,eax
       push	 eax

       mov	ax,word[.b_x3]
       sub	ax,word[.b_x1]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dbx13,eax
       push	 eax

       mov	ax,word[.b_y3]
       sub	ax,word[.b_y1]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dby13,eax
       push	 eax

       mov	ax,word[.e_x3]
       sub	ax,word[.e_x1]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dex13,eax
       push	 eax

       mov	ax,word[.e_y3]
       sub	ax,word[.e_y1]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dey13,eax
       push	 eax
   .bt_dx13_done:

       mov	bx,.y3	     ; calc delta23
       sub	bx,.y2
       jnz	.bt_dx23_make
       mov	ecx,5
       xor	edx,edx
     @@:
       push	edx   ;dword 0
       loop	@b
       jmp	.bt_dx23_done
 .bt_dx23_make:
       mov	ax,.x3
       sub	ax,.x2
       cwde
       movsx	ebx,bx
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dx23,eax
       push	 eax

       mov	ax,word[.b_x3]
       sub	ax,word[.b_x2]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dbx23,eax
       push	 eax

       mov	ax,word[.b_y3]
       sub	ax,word[.b_y2]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dby23,eax
       push	 eax

       mov	ax,word[.e_x3]
       sub	ax,word[.e_x2]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dex23,eax
       push	 eax

       mov	ax,word[.e_y3]
       sub	ax,word[.e_y2]
       cwde
       shl	eax,ROUND
       cdq
       idiv	ebx
 ;      mov      .dey23,eax
       push	 eax

      ;  sub     esp,40
   .bt_dx23_done:

       movsx	eax,.x1
       shl	eax,ROUND
     ; mov      .cx1,eax
     ; mov      .cx2,eax
       push	eax
       push	eax

       movsx	eax,word[.b_x1]
       shl	eax,ROUND
     ; mov      .cbx1,eax
     ; mov      .cbx2,eax
       push	eax
       push	eax

       movsx	eax,word[.b_y1]
       shl	eax,ROUND
     ; mov      .cby1,eax
     ; mov      .cby2,eax
       push	eax
       push	eax

       movsx	eax,word[.e_x1]
       shl	eax,ROUND
      ;mov      .cex1,eax
      ;mov      .cex2,eax
       push	eax
       push	eax

       movsx	eax,word[.e_y1]
       shl	eax,ROUND
      ;mov      .cey1,eax
      ;mov      .cey2,eax
       push	eax
       push	eax

       movzx	ecx,.y1
       cmp	cx,.y2
       jge	.loop12_done
  .loop12:
       call	.call_bump_line

       mov	eax,.dx13
       add	.cx1,eax
       mov	eax,.dx12
       add	.cx2,eax

       mov	eax,.dbx13
       add	.cbx1,eax
       mov	eax,.dbx12
       add	.cbx2,eax
       mov	eax,.dby13
       add	.cby1,eax
       mov	eax,.dby12
       add	.cby2,eax

       mov	eax,.dex13
       add	.cex1,eax
       mov	eax,.dex12
       add	.cex2,eax
       mov	eax,.dey13
       add	.cey1,eax
       mov	eax,.dey12
       add	.cey2,eax

       inc	ecx
       cmp	cx,.y2
       jl	.loop12
   .loop12_done:
       movzx	ecx,.y2
       cmp	cx,.y3
       jge	.loop23_done

       movzx	eax,.x2
       shl	eax,ROUND
       mov	.cx2,eax

       movzx	eax,word[.b_x2]
       shl	eax,ROUND
       mov	.cbx2,eax

       movzx	eax,word[.b_y2]
       shl	eax,ROUND
       mov	.cby2,eax

       movzx	eax,word[.e_x2]
       shl	eax,ROUND
       mov	.cex2,eax

       movzx	eax,word[.e_y2]
       shl	eax,ROUND
       mov	.cey2,eax

     .loop23:
       call	.call_bump_line

       mov	eax,.dx13
       add	.cx1,eax
       mov	eax,.dx23
       add	.cx2,eax

       mov	eax,.dbx13
       add	.cbx1,eax
       mov	eax,.dbx23
       add	.cbx2,eax
       mov	eax,.dby13
       add	.cby1,eax
       mov	eax,.dby23
       add	.cby2,eax

       mov	eax,.dex13
       add	.cex1,eax
       mov	eax,.dex23
       add	.cex2,eax
       mov	eax,.dey13
       add	.cey1,eax
       mov	eax,.dey23
       add	.cey2,eax

       inc	ecx
       cmp	cx,.y3
       jl	.loop23
    .loop23_done:
       mov	esp,ebp
ret   24

.call_bump_line:

      ; push     ebp
      ; push     ecx
       pushad

       push	.t_emap
       push	.t_bmap
       push	.cey2
       push	.cex2
       push	.cey1
       push	.cex1
       push	.cby2
       push	.cbx2
       push	.cby1
       push	.cbx1
       push	ecx

       mov	eax,.cx1
       sar	eax,ROUND
       mov	ebx,.cx2
       sar	ebx,ROUND

       call	bump_line

       popad
ret
bump_line:
;--------------in: eax - x1
;--------------    ebx - x2
;--------------    edi - pointer to screen buffer
;stack - another parameters :
.y    equ dword [ebp+4]
.bx1  equ	[ebp+8]   ;   ---
.by1  equ dword [ebp+12]  ;       |
.bx2  equ	[ebp+16]  ;       |
.by2  equ dword [ebp+20]  ;       |>   bump and env coords
.ex1  equ	[ebp+24]  ;       |>   shifted shl ROUND
.ey1  equ dword [ebp+28]  ;       |
.ex2  equ	[ebp+32]  ;       |
.ey2  equ dword [ebp+36]  ;   ---
.bmap equ dword [ebp+40]
.emap equ dword [ebp+44]

.x1   equ dword [ebp-4]
.x2   equ dword [ebp-8]
.dbx  equ dword [ebp-12]
.dby  equ	[ebp-16]
.dex  equ dword [ebp-20]
.dey  equ	[ebp-24]
.cbx  equ dword [ebp-28]
.cby  equ	[ebp-32]
.cex  equ dword [ebp-36]
.cey  equ	[ebp-40]
	mov	ebp,esp

	mov	ecx,.y
	or	ecx,ecx
	jl	.bl_end
	cmp	ecx,SIZE_Y
	jge	.bl_end

	cmp	eax,ebx
	jl	.bl_ok
	je	.bl_end

	xchg	eax,ebx
if Ext = NON
	mov	edx,.bx1
	xchg	edx,.bx2
	mov	.bx1,edx
	mov	edx,.by1
	xchg	edx,.by2
	mov	.by1,edx

	mov	edx,.ex1
	xchg	edx,.ex2
	mov	.ex1,edx
	mov	edx,.ey1
	xchg	edx,.ey2
	mov	.ey1,edx
else
	movq	mm0,.bx1
	movq	mm1,.bx2
	movq	mm2,.ex1
	movq	mm3,.ex2
	movq	.bx2,mm0
	movq	.bx1,mm1
	movq	.ex1,mm3
	movq	.ex2,mm2
end if
  .bl_ok:
	push	eax
	push	ebx	      ;store x1, x2

	mov	eax,SIZE_X*3
	mov	ebx,.y
	mul	ebx
	mov	ecx,.x1
	lea	ecx,[ecx*3]
	add	eax,ecx
	add	edi,eax

	mov	ecx,.x2
	sub	ecx,.x1

	mov	eax,.bx2       ; calc .dbx
	sub	eax,.bx1
	cdq
	idiv	ecx
	push	eax

	mov	eax,.by2       ; calc .dby
	sub	eax,.by1
	cdq
	idiv	ecx
	push	eax

	mov	eax,.ex2       ; calc .dex
	sub	eax,.ex1
	cdq
	idiv	ecx
	push	eax

	mov	eax,.ey2       ; calc .dey
	sub	eax,.ey1
	cdq
	idiv	ecx
	push	eax

	push	dword .bx1
	push	.by1
	push	dword .ex1
	push	.ey1
     .draw:
    ; if TEX = SHIFTING   ;bump drawing only in shifting mode

	mov	eax,.cby
	sar	eax,ROUND
	shl	eax,TEX_SHIFT
	mov	esi,.cbx
	sar	esi,ROUND
	add	esi,eax

	mov	ebx,esi
	dec	ebx
	and	ebx,TEXTURE_SIZE
	add	ebx,.bmap
	movzx	eax,byte [ebx]

	mov	ebx,esi
	inc	ebx
	and	ebx,TEXTURE_SIZE
	add	ebx,.bmap
	movzx	ebx,byte [ebx]
	sub	eax,ebx

	mov	ebx,esi
	sub	ebx,TEX_X
	and	ebx,TEXTURE_SIZE
	add	ebx,.bmap
	movzx	edx,byte [ebx]

	mov	ebx,esi
	add	ebx,TEX_X
	and	ebx,TEXTURE_SIZE
	add	ebx,.bmap
	movzx	ebx,byte [ebx]
	sub	edx,ebx

	mov	ebx,.cex       ;.cex - current env map X
	sar	ebx,ROUND
	add	eax,ebx        ; eax - modified x coord

	mov	ebx,.cey       ;.cey - current  env map y
	sar	ebx,ROUND
	add	edx,ebx        ; edx - modified y coord

	or	eax,eax
	jl	.black
	cmp	eax,TEX_X
	jg	.black
	or	edx,edx
	jl	.black
	cmp	edx,TEX_Y
	jg	.black

	shl	edx,TEX_SHIFT
	add	edx,eax
	lea	edx,[edx*3]
	add	edx,.emap
	mov	eax,dword[edx]
	jmp	.put_pixel
     .black:
	xor	eax,eax
     .put_pixel:
	stosd
	dec	edi
;if Ext >= MMX
;        movq    mm0,.cby
;        movq    mm1,.cey
;        paddd   mm0,.dby
;        paddd   mm1,.dey
;        movq    .cby,mm0
;        movq    .cey,mm1
;else
	mov	eax,.dbx
	add	.cbx,eax
	mov	eax,.dby
	add	.cby,eax
	mov	eax,.dex
	add	.cex,eax
	mov	eax,.dey
	add	.cey,eax
;end if
	dec	ecx
	jnz	.draw
   ;   end if
  .bl_end:
	mov	esp,ebp
ret 44