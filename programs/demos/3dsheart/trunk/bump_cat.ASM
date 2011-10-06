;SIZE_X equ 350
;SIZE_Y equ 350
ROUND equ 8
;TEX_X equ 512
;TEX_Y equ 512
;TEXTURE_SIZE EQU (512*512)-1
;TEX_SHIFT EQU 9
CATMULL_SHIFT equ 8
;TEXTURE_SIZE EQU (TEX_X * TEX_Y)-1
;------- Big thanks to Majuma (www.majuma.xt.pl) for absolutely great---
;------- DOS 13h mode demos --------------------------------------------
;------- Procedure draws bump triangle using Catmull Z-buffer algorithm-
;------- (Z coordinate interpolation)-----------------------------------
bump_triangle_z:
;------------------in - eax - x1 shl 16 + y1 -----------
;---------------------- ebx - x2 shl 16 + y2 -----------
;---------------------- ecx - x3 shl 16 + y3 -----------
;---------------------- edx - pointer to bump map ------
;---------------------- esi - pointer to environment map
;---------------------- edi - pointer to screen buffer--
;---------------------- stack : bump coordinates--------
;----------------------         environment coordinates-
;----------------------         Z position coordinates--
;----------------------         pointer io Z buffer-----
;-- Z-buffer - filled with coordinates as dword --------
;-- (Z coor. as word) shl CATMULL_SHIFT ----------------
.b_x1	equ ebp+4   ; procedure don't save registers !!!
.b_y1	equ ebp+6   ; each coordinate as word
.b_x2	equ ebp+8
.b_y2	equ ebp+10
.b_x3	equ ebp+12
.b_y3	equ ebp+14
.e_x1	equ ebp+16
.e_y1	equ ebp+18
.e_x2	equ ebp+20
.e_y2	equ ebp+22
.e_x3	equ ebp+24
.e_y3	equ ebp+26
.z1	equ word[ebp+28]
.z2	equ word[ebp+30]
.z3	equ word[ebp+32]
.z_buff equ dword[ebp+34]	; pointer to Z-buffer


.t_bmap equ dword[ebp-4]	; pointer to bump map
.t_emap equ dword[ebp-8]	; pointer to e. map
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
.dz12  equ dword[ebp-44]

.dx13  equ dword[ebp-48]
.dbx13 equ dword[ebp-52]
.dby13 equ dword[ebp-56]
.dex13 equ dword[ebp-60]
.dey13 equ dword[ebp-64]
.dz13  equ dword[ebp-68]

.dx23  equ dword[ebp-72]
.dbx23 equ dword[ebp-76]
.dby23 equ dword[ebp-80]
.dex23 equ dword[ebp-84]
.dey23 equ dword[ebp-88]
.dz23  equ dword[ebp-92]

.cx1   equ dword[ebp-96]   ; current variables
.cx2   equ dword[ebp-100]
.cbx1  equ dword[ebp-104]
.cbx2  equ dword[ebp-108]
.cby1  equ dword[ebp-112]
.cby2  equ dword[ebp-116]
.cex1  equ dword[ebp-120]
.cex2  equ dword[ebp-124]
.cey1  equ dword[ebp-128]
.cey2  equ dword[ebp-132]
.cz1   equ dword[ebp-136]
.cz2   equ dword[ebp-140]

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
       mov     dx,.z1
       xchg    dx,.z2
       mov     .z1,dx
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
       mov     dx,.z2
       xchg    dx,.z3
       mov     .z2,dx
       jmp	.sort3
 .sort2:
       push	eax	; store triangle coords in variables
       push	ebx
       push	ecx

	 mov	  edx,80008000h  ; eax,ebx,ecx are ANDd together into edx which means that
	 and	  edx,ebx	 ; if *all* of them are negative a sign flag is raised
	 and	  edx,ecx
	 and	  edx,eax
	 test	  edx,80008000h  ; Check both X&Y at once
	 jne	  .loop23_done
    ;   mov     edx,eax         ; eax,ebx,ecx are ORd together into edx which means that
    ;   or      edx,ebx         ; if any *one* of them is negative a sign flag is raised
    ;   or      edx,ecx
    ;   test    edx,80000000h   ; Check only X
    ;   jne     .loop23_done

    ;   cmp     .x1,SIZE_X    ; {
    ;   jg      .loop23_done
    ;   cmp     .x2,SIZE_X     ; This can be optimized with effort
    ;   jg      .loop23_done
    ;   cmp     .x3,SIZE_X
    ;   jg      .loop23_done    ; {


       mov	bx,.y2	     ; calc delta 12
       sub	bx,.y1
       jnz	.bt_dx12_make
       mov	ecx,6
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

	mov	ax,.z2
	sub	ax,.z1
	cwde
	shl	eax,CATMULL_SHIFT
	cdq
	idiv	ebx
	push	eax
   .bt_dx12_done:

       mov	bx,.y3	     ; calc delta13
       sub	bx,.y1
       jnz	.bt_dx13_make
       mov	ecx,6
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

     ;  nop                     here bug ????!!!

       mov     ax,.z3
       sub     ax,.z1
       cwde
       shl     eax,CATMULL_SHIFT
       cdq
       idiv    ebx
  ;    mov    .dz13,eax
       push    eax
   .bt_dx13_done:

       mov	bx,.y3	     ; calc delta23
       sub	bx,.y2
       jnz	.bt_dx23_make
       mov	ecx,6
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

       mov     ax,.z3
       sub     ax,.z2
       cwde
       shl     eax,CATMULL_SHIFT
       cdq
       idiv    ebx
      ; mov     .dz23,eax
       push    eax
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

       movsx	eax,.z1
       shl	eax,CATMULL_SHIFT
      ;mov      .cz1,eax
      ;mov      .cz2,eax
       push	eax
       push	eax

       movsx	ecx,.y1
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

       mov	eax,.dz13
       add	.cz1,eax
       mov	eax,.dz12
       add	.cz2,eax

       inc	ecx
       cmp	cx,.y2
       jl	.loop12
    .loop12_done:

       movsx	ecx,.y2
       cmp	cx,.y3
       jge	.loop23_done

       movsx	eax,.z2
       shl	eax,CATMULL_SHIFT
       mov	.cz2,eax

       movsx	eax,.x2
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

       mov	eax,.dz13
       add	.cz1,eax
       mov	eax,.dz23
       add	.cz2,eax

       inc	ecx
       cmp	cx,.y3
       jl	.loop23
    .loop23_done:

       mov	esp,ebp
ret   34

.call_bump_line:

      ; push     ebp
      ; push     ecx
       pushad

       push	.cz1
       push	.cz2
       push	.z_buff
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

       call	bump_line_z

       popad
ret
bump_line_z:
;--------------in: eax - x1
;--------------    ebx - x2
;--------------    edi - pointer to screen buffer
;stack - another parameters :
.y	equ dword [ebp+4]
.bx1	equ dword [ebp+8]   ;   ---
.by1	equ dword [ebp+12]  ;       |
.bx2	equ dword [ebp+16]  ;       |
.by2	equ dword [ebp+20]  ;       |>   bump and env coords
.ex1	equ dword [ebp+24]  ;       |>   shifted shl ROUND
.ey1	equ dword [ebp+28]  ;       |
.ex2	equ dword [ebp+32]  ;       |
.ey2	equ dword [ebp+36]  ;   ---
.bmap	equ dword [ebp+40]
.emap	equ dword [ebp+44]
.z_buff equ dword [ebp+48]
.z2	equ dword [ebp+52]  ;   -- |>   z coords shifted
.z1	equ dword [ebp+56]  ;   --       shl  CATMULL_SHIFT

.x1	equ dword [ebp-4]
.x2	equ dword [ebp-8]
.dbx	equ dword [ebp-12]
.dby	equ dword [ebp-16]
.dex	equ dword [ebp-20]
.dey	equ dword [ebp-24]
.dz	equ dword [ebp-28]
.cbx	equ dword [ebp-32]
.cby	equ dword [ebp-36]
.cex	equ dword [ebp-40]
.cey	equ dword [ebp-44]
.cz	equ dword [ebp-48]
.czbuff equ dword [ebp-52]
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

	mov	edx,.z1
	xchg	edx,.z2
	mov	.z1,edx
  .bl_ok:
	push	eax
	push	ebx	      ;store x1, x2

	cmp	.x1,SIZE_X
	jge	.bl_end
	cmp	.x2,0
	jle	.bl_end

	mov	ebx,.x2
	sub	ebx,.x1

	mov	eax,.bx2       ; calc .dbx
	sub	eax,.bx1
	cdq
	idiv	ebx
	push	eax

	mov	eax,.by2       ; calc .dby
	sub	eax,.by1
	cdq
	idiv	ebx
	push	eax

	mov	eax,.ex2       ; calc .dex
	sub	eax,.ex1
	cdq
	idiv	ebx
	push	eax

	mov	eax,.ey2       ; calc .dey
	sub	eax,.ey1
	cdq
	idiv	ebx
	push	eax

	mov	eax,.z2        ; calc .dz
	sub	eax,.z1
	cdq
	idiv	ebx
	push	eax

	cmp	.x1,0	      ; set correctly begin variable
	jge	@f	      ; CLIPPING ON FUNCTION
			      ; cutting triangle exceedes screen
	mov	ebx,.x1
	neg	ebx
	imul	ebx	      ; eax = .dz * abs(.x1)
	add	.z1,eax
	mov	.x1,0

	mov	eax,.dbx
	imul	ebx
	add    .bx1,eax

	mov	eax,.dby
	imul	ebx
	add	.by1,eax

	mov	eax,.dex
	imul	ebx
	add	.ex1,eax

	mov	eax,.dey
	imul	ebx
	add	.ey1,eax
      @@:
	cmp	.x2,SIZE_X
	jl	@f
	mov	.x2,SIZE_X
      @@:
	mov	eax,SIZE_X	 ;calc memory begin in buffers
	mov	ebx,.y
	mul	ebx
	mov	ebx,.x1
	add	eax,ebx
	mov	ebx,eax
	lea	eax,[eax*3]
	add	edi,eax
	mov	esi,.z_buff	  ; z-buffer filled with dd variables
	shl	ebx,2
	add	esi,ebx

	mov	ecx,.x2
	sub	ecx,.x1
	; init current variables
	push	.bx1
	push	.by1
	push	.ex1
	push	.ey1
	push	.z1		 ; current z shl CATMULL_SHIFT
	push	esi
     .draw:
    ; if TEX = SHIFTING   ;bump drawing only in shifting mode
	mov	esi,.czbuff	 ; .czbuff current address in buffer
	mov	ebx,.cz 	 ; .cz - cur z position
	cmp	ebx,dword[esi]
	jge	.skip

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
	mov	ebx,.cz
	mov	esi,.czbuff
	mov	dword[esi],ebx
	jmp	.no_skip
     .skip:
	add	edi,3
     .no_skip:
	add	.czbuff,4
	mov	eax,.dbx
	add	.cbx,eax
	mov	eax,.dby
	add	.cby,eax
	mov	eax,.dex
	add	.cex,eax
	mov	eax,.dey
	add	.cey,eax
	mov	eax,.dz
	add	.cz,eax

	dec	ecx
	jnz	.draw
   ;   end if
  .bl_end:
	mov	esp,ebp
ret 56