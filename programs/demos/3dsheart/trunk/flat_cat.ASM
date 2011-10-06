CATMULL_SHIFT equ 16
fill_Z_buffer:
	mov	eax,0x70000000
	mov	edi,Z_buffer
	mov	ecx,SIZE_X*SIZE_Y
	rep	stosd
ret
flat_triangle_z:
;  procedure drawing triangle with Z cordinate interpolation ------
;  (Catmull  alghoritm)--------------------------------------------
;  ----------------in - eax - x1 shl 16 + y1 ----------------------
;  -------------------- ebx - x2 shl 16 + y2 ----------------------
;  -------------------- ecx - x3 shl 16 + y3 ----------------------
;  -------------------- edx - color 0x00RRGGBB --------------------
;  -------------------- esi - pointer to Z-buffer -----------------
;  -------------------- edi - pointer to screen buffer-------------
;  -------------------- stack : z coordinates
;  -------------------- Z-buffer : each z variable as dword
;  -------------------- (Z coor. as word) shl CATMULL_SHIFT
.z1	equ word[ebp+4]
.z2	equ word[ebp+6]    ; each z coordinate as word integer
.z3	equ word[ebp+8]

.col	equ dword[ebp-4]
.x1	equ word[ebp-6]
.y1	equ word[ebp-8]
.x2	equ word[ebp-10]
.y2	equ word[ebp-12]
.x3	equ word[ebp-14]
.y3	equ word[ebp-16]

.dx12	equ dword[ebp-20]
.dz12	equ dword[ebp-24]
.dx13	equ dword[ebp-28]
.dz13	equ dword[ebp-32]
.dx23	equ dword[ebp-36]
.dz23	equ dword[ebp-40]
.zz1	equ dword[ebp-44]
.zz2	equ dword[ebp-48]

	mov	ebp,esp
	push	edx	       ; store edx in variable .col
      .sort2:
	cmp	ax,bx
	jle	.sort1
	xchg	eax,ebx
	mov	dx,.z1
	xchg	dx,.z2
	mov	.z1,dx
      .sort1:
	cmp	bx,cx
	jle	.sort3
	xchg	ebx,ecx
	mov	dx,.z2
	xchg	dx,.z3
	mov	.z2,dx
	jmp	.sort2
      .sort3:
	push	eax	       ; store triangle coordinates in user friendly variables
	push	ebx
	push	ecx
	 mov	  edx,80008000h  ; eax,ebx,ecx are ANDd together into edx which means that
	 and	  edx,ebx	 ; if *all* of them are negative a sign flag is raised
	 and	  edx,ecx
	 and	  edx,eax
	 test	  edx,80008000h  ; Check both X&Y at once
	 jne	  .ft_loop2_end
       ;  cmp      ax,SIZE_Y
       ;  jle      @f
       ;  cmp      bx,SIZE_Y
       ;  jle      @f
       ;  cmp      cx,SIZE_Y
       ;  jge      @f
       ;  ror      eax,16
       ;  ror      ebx,16
       ;  ror      ecx,16
       ;  cmp      ax,SIZE_X
       ;  jle      @f
       ;  cmp      bx,SIZE_X
       ;  jle      @f
       ;  cmp      cx,SIZE_X
       ;  jle      @f
       ;  jmp      .ft_loop2_end
       ;@@:
	sub	esp,32

	mov	bx,.y2	     ; calc delta 12
	sub	bx,.y1
	jnz	.ft_dx12_make
	mov	.dx12,0
	mov	.dz12,0
	jmp	.ft_dx12_done
   .ft_dx12_make:
	mov	ax,.x2
	sub	ax,.x1
	cwde
	movsx	ebx,bx
	shl	eax,ROUND
	cdq
	idiv	ebx
	mov	.dx12,eax

	mov	ax,.z2
	sub	ax,.z1
	cwde
	shl	eax,CATMULL_SHIFT
	cdq
	idiv	ebx
	mov	.dz12,eax
  .ft_dx12_done:
	mov	bx,.y3	     ; calc delta 13
	sub	bx,.y1
	jnz	.ft_dx13_make
	mov	.dx13,0
	mov	.dz13,0
	jmp	.ft_dx13_done
   .ft_dx13_make:
	mov	ax,.x3
	sub	ax,.x1
	cwde
	movsx	ebx,bx
	shl	eax,ROUND
	cdq
	idiv	ebx
	mov	.dx13,eax

	mov	ax,.z3
	sub	ax,.z1
	cwde
	shl	eax,CATMULL_SHIFT
	cdq
	idiv	ebx
	mov	.dz13,eax
    .ft_dx13_done:
	mov	bx,.y3	     ; calc delta 23
	sub	bx,.y2
	jnz	.gt_dx23_make
	mov	.dx23,0
	mov	.dz23,0
	jmp	.gt_dx23_done
   .gt_dx23_make:
	mov	ax,.x3
	sub	ax,.x2
	cwde
	movsx	ebx,bx
	shl	eax,ROUND
	cdq
	idiv	ebx
	mov	.dx23,eax

	mov	ax,.z3
	sub	ax,.z2
	cwde
	shl	eax,CATMULL_SHIFT
	cdq
	idiv	ebx
	mov	.dz23,eax
    .gt_dx23_done:

	movsx	edx,.z1
	shl	edx,CATMULL_SHIFT
	mov	.zz1,edx
	mov	.zz2,edx
	movsx	eax,.x1
	shl	eax,ROUND    ; eax - x1
	mov	ebx,eax      ; ebx - x2
	mov	cx,.y1
	cmp	cx,.y2
	jge	.ft_loop1_end
     .ft_loop1:

	pushad

	push	.col
	push	cx	    ; y
	sar	ebx,ROUND
	push	bx	    ; x2
	sar	eax,ROUND
	push	ax	    ; x1
	push	.zz2	    ; z2 shl CATMULL_SHIFT
	push	.zz1	    ; z1 shl CATMULL_SHIFT
	call	flat_line_z

	popad

	add	eax,.dx13
	add	ebx,.dx12
	mov	edx,.dz13
	add	.zz1,edx
	mov	edx,.dz12
	add	.zz2,edx
	inc	cx
	cmp	cx,.y2
	jl	.ft_loop1
    .ft_loop1_end:

	movsx	edx,.z2
	shl	edx,CATMULL_SHIFT
	mov	.zz2,edx
	movsx	ebx,.x2
	shl	ebx,ROUND
	mov	cx,.y2
	cmp	cx,.y3
	jge	.ft_loop2_end
     .ft_loop2:
	pushad

	push	.col
	push	cx
	sar	ebx,ROUND
	push	bx
	sar	eax,ROUND
	push	ax	    ; x1
	push	.zz2	    ; z2 shl CATMULL_SHIFT
	push	.zz1	    ; z1 shl CATMULL_SHIFT
	call	flat_line_z

	popad

	add	eax,.dx13
	add	ebx,.dx23
	mov	edx,.dz13
	add	.zz1,edx
	mov	edx,.dz23
	add	.zz2,edx
	inc	cx
	cmp	cx,.y3
	jl	.ft_loop2
     .ft_loop2_end:

	mov esp,ebp
ret 6

flat_line_z:
;----------------
;-------------in edi - pointer to screen buffer ----------------------------------
;--------------- esi - pointer to z-buffer (each Z varible dword)-----------------
;----------stack - (each z coordinate shifted shl CATMULL_SHIFT)------------------
.z1 equ dword [ebp+4]
.z2 equ dword [ebp+8]
.x1 equ word  [ebp+12]
.x2 equ word  [ebp+14]
.y  equ word  [ebp+16]
.col equ dword [ebp+18]

.dz equ dword [ebp-4]

     mov	ebp,esp
;;     sub        esp,4
     mov	ax,.y
     or 	ax,ax
     jl 	.fl_quit
     cmp	ax,SIZE_Y-1
     jg 	.fl_quit

 ;    cmp        .x1,0
 ;    jge        .fl_ok1
 ;    cmp        .x2,0
 ;    jl         .fl_quit
 ;  .fl_ok1:
 ;    cmp        .x1,SIZE_X
 ;    jle        .fl_ok2
 ;    cmp        .x2,SIZE_X
 ;    jg         .fl_quit
 ;  .fl_ok2:
     mov	ax,.x1
     cmp	ax,.x2
     je 	.fl_quit
     jl 	.fl_ok

     xchg	ax,.x2
     mov	.x1,ax
     mov	edx,.z1
     xchg	edx,.z2
     mov	.z1,edx
   .fl_ok:
     cmp	.x1,SIZE_X-1
     jg 	.fl_quit
     cmp	.x2,0
     jle	.fl_quit

     mov	eax,.z2
     sub	eax,.z1
     cdq
     mov	bx,.x2
     sub	bx,.x1
     movsx	ebx,bx
     idiv	ebx
;;     mov        .dz,eax                ; calculated delta - shifted .dz
     push	eax

     cmp	.x1,0
     jge	@f
     movsx	ebx,.x1
     neg	ebx
     imul	ebx
     add	.z1,eax
     mov	.x1,0
    @@:
     cmp	.x2,SIZE_X
     jl 	@f
     mov	.x2,SIZE_X
    @@:
     mov	edx,SIZE_X
     movsx	eax,.y
     mul	edx		     ; edi = edi + (SIZE_X * y + x1)*3
     movsx	edx,.x1
     add	eax,edx
     push	eax
     lea	eax,[eax*3]
     add	edi,eax 	     ; esi = esi + (SIZE_X * y + x1)*4
     pop	eax
     shl	eax,2
     add	esi,eax

     mov	cx,.x2
     sub	cx,.x1
     movzx	ecx,cx

     mov	eax,.col
     mov	ebx,.z1  ; ebx : curr. z
     mov	edx,.dz
   .ddraw:
     ;pushad
     ;show
     ;popad
     cmp	ebx,dword[esi]
     jge	.skip
     stosd
     dec	edi
     mov	dword[esi],ebx
     jmp	.no_skip
   .skip:
     add	edi,3
   .no_skip:
     add	esi,4
     add	ebx,edx
     loop	.ddraw

   .fl_quit:
     mov   esp,ebp
ret 18
