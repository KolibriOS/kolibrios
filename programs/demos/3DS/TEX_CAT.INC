;TEX_X = 512
;TEX_Y = 512
;ROUND equ 8
;SIZE_X = 512
;SIZE_Y = 512
;TEX_SHIFT = 9
CATMULL_SHIFT equ 8

;------------------------------------------------------------------------
;- Procedure drawing textured triangle using Catmull Z-buffer algorithm -
;------------------------------------------------------------------------
tex_triangle_z:
;----------in - eax - x1 shl 16 + y1
;-------------- ebx - x2 shl 16 + y2
;---------------ecx - x3 shl 16 + y3
;---------------edx - pointer to Z-buffer
;---------------esi - pointer to texture buffer
;---------------edi - pointer to screen buffer
;-------------stack - texture coordinates
;------------------ - z coordinates
.tex_x1 equ  ebp+4
.tex_y1 equ  ebp+6
.tex_x2 equ  ebp+8
.tex_y2 equ  ebp+10
.tex_x3 equ  ebp+12
.tex_y3 equ  ebp+14
.z1	equ  word[ebp+16]
.z2	equ  word[ebp+18]
.z3	equ  word[ebp+20]

.tex_ptr equ dword[ebp-4]	 ; pointer to texture
.z_ptr	 equ dword[ebp-8]	 ; pointer to z-buffer
.x1	 equ word[ebp-10]
.y1	 equ word[ebp-12]
.x2	 equ word[ebp-14]
.y2	 equ word[ebp-16]
.x3	 equ word[ebp-18]
.y3	 equ word[ebp-20]

.dx12	  equ dword[ebp-24]
.tex_dx12 equ dword[ebp-28]
.tex_dy12 equ dword[ebp-32]
.dz12	  equ dword[ebp-36]

.dx13	  equ dword[ebp-40]
.tex_dx13 equ dword[ebp-44]
.tex_dy13 equ dword[ebp-48]
.dz13	  equ dword[ebp-52]

.dx23	  equ dword[ebp-56]
.tex_dx23 equ dword[ebp-60]
.tex_dy23 equ dword[ebp-64]
.dz23	  equ dword[ebp-68]

.scan_x1  equ dword[ebp-72]
.scan_x2  equ dword[ebp-76]
.scan_y1  equ dword[ebp-80]
.scan_y2  equ dword[ebp-84]
.cz1	  equ dword[ebp-88]
.cz2	  equ dword[ebp-92]

	mov	ebp,esp
	push	esi		  ; store memory pointers
	push	edx
.tt_sort3:
	cmp	ax,bx			  ;sort all parameters
	jle	.tt_sort1
	xchg	eax,ebx
	mov	edx,dword [.tex_x1]
	xchg	edx,dword [.tex_x2]
	mov	dword[.tex_x1],edx
	mov	dx,.z1
	xchg	dx,.z2
	mov	.z1,dx
.tt_sort1:
	cmp	bx,cx
	jle	.tt_sort2
	xchg	ebx,ecx
	mov	edx,dword [.tex_x2]
	xchg	edx,dword [.tex_x3]
	mov	dword [.tex_x2],edx
	mov	dx,.z2
	xchg	dx,.z3
	mov	.z2,dx
	jmp	.tt_sort3
.tt_sort2:

	push	eax	; and store to user friendly variables
	push	ebx
	push	ecx

	mov	 edx,80008000h	; eax,ebx,ecx are ANDd together into edx which means that
	and	 edx,ebx	; if *all* of them are negative a sign flag is raised
	and	 edx,ecx
	and	 edx,eax
	test	 edx,80008000h	; Check both X&Y at once
	jne	 .tt_loop2_end
    ;    cmp      ax,SIZE_Y
    ;    jl       @f
    ;    cmp      bx,SIZE_Y
    ;    jl       @f
    ;    cmp      cx,SIZE_Y
    ;    jl       @f
	ror	 eax,16
	ror	 ebx,16
	ror	 ecx,16
    ;    cmp      ax,SIZE_X
    ;    jl       @f
    ;    cmp      bx,SIZE_X
    ;    jl       @f
    ;    cmp      cx,SIZE_X
    ;    jl       @f
    ;    jmp      .tt_loop2_end
     @@:
	mov	 eax,dword[.tex_x1] ; texture coords must be in [0..TEX_X(Y)]
	mov	 ebx,dword[.tex_x2]
	mov	 ecx,dword[.tex_x3]
	mov	 edx,eax
	or	 edx,ebx
	or	 edx,ecx
	test	 edx,80008000h
	jne	 .tt_loop2_end
	cmp	 ax,TEX_X
	jge	 .tt_loop2_end
	cmp	 bx,TEX_X
	jge	 .tt_loop2_end
	cmp	 cx,TEX_X
	jge	 .tt_loop2_end
	ror	 eax,16
	ror	 ebx,16
	ror	 ecx,16
	cmp	 ax,TEX_Y
	jge	 .tt_loop2_end
	cmp	 bx,TEX_Y
	jge	 .tt_loop2_end
	cmp	 cx,TEX_Y
	jge	 .tt_loop2_end


	movsx	 ebx,.y2    ; calc delta
	sub	 bx,.y1
	jnz	 .tt_dx12_make
	xor	 edx,edx
	mov	 ecx,4
    @@:
	push	 edx
	loop	 @b
	jmp	 .tt_dx12_done
    .tt_dx12_make:
	mov	 ax,.x2
	sub	 ax,.x1
	cwde
	shl	 eax,ROUND
	cdq
	idiv	 ebx
       ; mov      .dx12,eax                      ; dx12 = (x2-x1)/(y2-y1)
	push	 eax

	mov	 ax,word[.tex_x2]
	sub	 ax,word[.tex_x1]
	cwde
	shl	 eax,ROUND
	cdq
	idiv	 ebx
       ; mov     [.tex_dx12],eax               ; tex_dx12 = (tex_x2-tex_x1)/(y2-y1)
	push	 eax

	mov	 ax,word[.tex_y2]
	sub	 ax,word[.tex_y1]
	cwde
	shl	 eax,ROUND
	cdq
	idiv	 ebx
     ;   mov    [.tex_dy12],eax              ; tex_dy12 = (tex_y2-tex_y1)/(y2-y1)
	push	 eax

	mov	 ax,.z2
	sub	 ax,.z1
	cwde
	shl	 eax,CATMULL_SHIFT
	cdq
	idiv	 ebx
	push	 eax
   .tt_dx12_done:

	movsx	 ebx,.y3    ; calc delta
	sub	 bx,.y1
	jnz	 .tt_dx13_make
	xor	 edx,edx
	mov	 ecx,4
    @@:
	push	 edx
	loop	 @b
	jmp	 .tt_dx13_done
    .tt_dx13_make:
	mov	 ax,.x3
	sub	 ax,.x1
	cwde
	shl	 eax,ROUND
	cdq
	idiv	 ebx
       ; mov      .dx12,eax                      ; dx13 = (x3-x1)/(y3-y1)
	push	  eax

	mov	 ax,word[.tex_x3]
	sub	 ax,word[.tex_x1]
	cwde
	shl	 eax,ROUND
	cdq
	idiv	 ebx
       ; mov     [.tex_dx12],eax               ; tex_dx13 = (tex_x3-tex_x1)/(y3-y1)
	push	 eax

	mov	 ax,word[.tex_y3]
	sub	 ax,word[.tex_y1]
	cwde
	shl	 eax,ROUND
	cdq
	idiv	 ebx
     ;   mov    [.tex_dy12],eax              ; tex_dy13 = (tex_y3-tex_y1)/(y3-y1)
	push	 eax

	mov	 ax,.z3
	sub	 ax,.z1
	cwde
	shl	 eax,CATMULL_SHIFT
	cdq
	idiv	 ebx
	push	 eax
   .tt_dx13_done:

	mov	 bx,.y3    ; calc delta
	sub	 bx,.y2
	jnz	 .tt_dx23_make
	xor	 edx,edx
	mov	 ecx,4
    @@:
	push	 edx
	loop	 @b
	jmp	 .tt_dx23_done
    .tt_dx23_make:
	mov	 ax,.x3
	sub	 ax,.x2
	cwde
	shl	 eax,ROUND
	cdq
	movzx	 ebx,bx
	idiv	 ebx
       ; mov      .dx23,eax                      ; dx23 = (x3-x2)/(y3-y2)
	push	  eax

	mov	 ax,word[.tex_x3]
	sub	 ax,word[.tex_x2]
	cwde
	shl	 eax,ROUND
	cdq
	idiv	 ebx
       ; mov     [.tex_dx23],eax               ; tex_dx23 = (tex_x3-tex_x2)/(y3-y2)
	push	 eax

	mov	 ax,word[.tex_y3]
	sub	 ax,word[.tex_y2]
	cwde
	shl	 eax,ROUND
	cdq
	idiv	 ebx
     ;   mov    [.tex_dy23],eax              ; tex_dy23 = (tex_y3-tex_y2)/(y3-y2)
	push	 eax

	mov	 ax,.z3
	sub	 ax,.z2
	cwde
	shl	 eax,CATMULL_SHIFT
	cdq
	idiv	 ebx
	push	 eax
   .tt_dx23_done:

	movsx	 eax,.x1     ;eax - cur x1
	shl	 eax,ROUND   ;ebx - cur x2
	mov	 ebx,eax

	movsx	 edx, word[.tex_x1]
	shl	 edx,ROUND
      ;  mov [.scan_x1],edx
      ;  mov [.scan_x2],edx
	push	 edx
	push	 edx
	movsx	 edx, word[.tex_y1]
	shl	 edx,ROUND
       ; mov [.scan_y1],edx
       ; mov [.scan_y2],edx
	push	 edx
	push	 edx
	movsx	 edx,.z1
	shl	 edx,CATMULL_SHIFT
	push	 edx
	push	 edx
	mov	 cx,.y1
	cmp	 cx,.y2
	jge	 .tt_loop1_end

   .tt_loop1:
	pushad

	push	.z_ptr
	push	.cz1	  ; z coords shifted shl catmull_shift
	push	.cz2
	push	.scan_y2
	push	.scan_x2
	push	.scan_y1
	push	.scan_x1
	push	esi		  ;[.tex_ptr]

	push	cx
	sar	ebx,ROUND
	push	bx
	sar	eax,ROUND
	push	ax
	call	textured_line_z

	popad
	mov	edx,.dz13
	add	.cz1,edx
	mov	edx,.dz12
	add	.cz2,edx

	mov	edx, .tex_dx13
	add	.scan_x1, edx
	mov	edx, .tex_dx12
	add	.scan_x2, edx
	mov	edx, .tex_dy13
	add	.scan_y1, edx
	mov	edx, .tex_dy12
	add	.scan_y2, edx

	add	eax, .dx13
	add	ebx, .dx12
	inc	cx
	cmp	cx,.y2
	jl	.tt_loop1

  .tt_loop1_end:


	mov	cx,.y2
	cmp	cx,.y3
	jge	.tt_loop2_end

	movsx	ebx,.x2
	shl	ebx,ROUND
	movsx	edx,.z2
	shl	edx,CATMULL_SHIFT
	mov	.cz2,edx
	movzx	edx, word [.tex_x2]
	shl	edx,ROUND
	mov	.scan_x2,edx
	movzx	edx, word[.tex_y2]
	shl	edx,ROUND
	mov	.scan_y2,edx

.tt_loop2:

	pushad

	push	.z_ptr
	push	.cz1	  ; z coords shifted shl catmull_shift
	push	.cz2

	push	.scan_y2
	push	.scan_x2
	push	.scan_y1
	push	.scan_x1
	push	esi		  ;[.tex_ptr]

	push	cx
	sar	ebx,ROUND
	push	bx
	sar	eax,ROUND
	push	ax
	call	textured_line_z

	popad


	mov	edx,.dz13
	add	.cz1,edx
	mov	edx,.dz23
	add	.cz2,edx

	mov	edx, .tex_dx13
	add	.scan_x1, edx
	mov	edx, .tex_dx23
	add	.scan_x2, edx
	mov	edx, .tex_dy13
	add	.scan_y1, edx
	mov	edx, .tex_dy23
	add	.scan_y2, edx

	add	eax, .dx13
	add	ebx, .dx23
	inc	cx
	cmp	cx,.y3
	jl	.tt_loop2

.tt_loop2_end:

.tt_end:
	mov esp,ebp
ret 18

textured_line_z:
;-----in -edi screen buffer pointer
;------------ stack:
  .x1 equ word [ebp+4]
  .x2 equ word [ebp+6]
  .y  equ word [ebp+8]

  .tex_ptr equ dword [ebp+10]
  .tex_x1  equ ebp+14
  .tex_y1  equ ebp+18
  .tex_x2  equ ebp+22
  .tex_y2  equ ebp+26
  .z2	   equ dword [ebp+30]	  ;z1, z2  coords shifted shl CATMULL_SHIFT
  .z1	   equ dword [ebp+34]
  .z_ptr   equ dword [ebp+38]

  .tex_dy  equ dword [ebp-4]
  .tex_dx  equ dword [ebp-8]
  .dz	   equ dword [ebp-12]
  .cz	   equ dword [ebp-16]
  .c_tex_x equ dword [ebp-20]  ; current tex x
  .m_sft1 equ ebp-28
  .m_sft2 equ ebp-32
;  .c_tex_xM equ ebp+14
  .tex_dxM  equ ebp-8

	mov	ebp,esp

	mov	ax,.y
	or	ax,ax
	jl	.tl_quit
	mov	bx,[size_y_var]
	dec	bx
	cmp	ax,bx  ;SIZE_Y
	jge	.tl_quit

	mov	ax,.x1
	cmp	ax,.x2
	je	.tl_quit
	jl	.tl_ok

	xchg	ax,.x2	  ; sort params
	mov	.x1,ax
if Ext >= MMX
	movq	mm0,[.tex_x1]
	movq	mm1,[.tex_x2]
	movq	[.tex_x2],mm0
	movq	[.tex_x1],mm1

else
	mov	eax,dword[.tex_x1]
	xchg	eax,dword[.tex_x2]
	mov	dword[.tex_x1],eax

	mov	eax,dword[.tex_y1]
	xchg	eax,dword[.tex_y2]
	mov	dword[.tex_y1],eax

end if

	mov	eax,.z1
	xchg	eax,.z2
	mov	.z1,eax

     .tl_ok:
	mov	cx,[size_x_var]
	dec	cx
	cmp	.x1,cx	 ;SIZE_X
	jge	.tl_quit
	cmp	.x2,0
	jle	.tl_quit

	mov	bx,.x2
	sub	bx,.x1
	movsx	ebx,bx

	mov	eax,dword[.tex_y2]	 ; calc .dty
	sub	eax,dword[.tex_y1]
	cdq
	idiv	ebx
	push	eax

	mov	eax,dword[.tex_x2]	 ; calc .dtx
	sub	eax,dword[.tex_x1]
	cdq
	idiv	ebx
	push	eax

	mov	eax,.z2       ; calc .dz
	sub	eax,.z1
	cdq
	idiv	ebx
	push	eax

	cmp    .x1,0	      ; clipping
	jg     @f

	movsx	ebx,.x1
	neg	ebx
	imul	ebx	      ; eax = .dz * abs(.x1)
	add	.z1,eax
	mov	.x1,0

	mov	eax,.tex_dy
	imul	ebx
	add	dword[.tex_y1],eax

	mov	eax,.tex_dx
	imul	ebx
	add	dword[.tex_x1],eax

      @@:
	cmp	.x2,cx ;SIZE_X
	jl	@f
	mov	.x2,cx	;SIZE_X
      @@:

	movsx	ebx,.y	      ; calc mem begin in buffers
	movzx	eax,word[size_x_var]  ;SIZE_X
	mul	ebx
	movsx	ebx,.x1
	add	eax,ebx
	mov	ebx,eax

	lea	eax,[eax*3]
	add	edi,eax 	      ; edi - scr buff
	shl	ebx,2
	add	.z_ptr,ebx	      ; z buffer pointer

	mov	cx,.x2
	sub	cx,.x1
	movzx	ecx,cx

;if Ext >= MMX
;        movq    mm0,[.tex_x1]
;        movq    mm4,mm0
;        movq    mm1,qword[.tex_dxM]
;        mov     ebx,.z1
;        mov     eax,.dz
;else
	mov	eax,dword[.tex_x1]
	mov	ebx,dword[.tex_y1]
	push	.z1	      ; .cz
	push	eax	      ;.c_tex_x
;end if
	mov	edx,.z_ptr

     .tl_loop:

;if Ext >= MMX
;        cmp     ebx,[edx]                       ;  ebx - current z
;        jge     @f
;        movq    mm2,mm0
;        psrad   mm2,ROUND
;        movq    mm3,mm2
;        psrlq   mm2,32-TEX_SHIFT
;        paddd   mm3,mm2
;        movd    esi,mm3
;        mov     dword[edx],ebx    ; renew z buffer
;else
							;  eax - temp
	mov	eax,.cz 			;  ebx - cur tex y shl ROUND
	cmp	eax,[edx]			;  ecx - l.lenght
	jge	@f	   ; ebx - cur tex_y ;  edx - temp
	mov	esi,ebx 		;  edi - scr buff
	sar	esi,ROUND		;  esi - tex_ptr temp
	shl	esi,TEX_SHIFT		;  .z_ptr - cur pointer to z buff
	mov	eax,.c_tex_x		;  .cz - cur z coord shl CATMULL_SHIFT
	sar	eax,ROUND
	add	esi,eax
	mov	eax,.cz
	mov	dword[edx],eax	  ; renew z buffer
;end if
	and	esi,TEXTURE_SIZE
	lea	esi,[esi*3]
	add	esi,.tex_ptr
	movsd
	dec	edi
	jmp	.no_skip
      @@:
	add	edi,3
     .no_skip:
	add	edx,4
;if Ext >= MMX
;        add     ebx,eax
;        paddd   mm0,mm1
;else
	mov	eax,.dz
	add	.cz,eax
	mov	eax,.tex_dx
	add	.c_tex_x,eax
	add	ebx,.tex_dy
;end if
	loop	.tl_loop
 .tl_quit:

	mov	esp,ebp

ret 30+8

