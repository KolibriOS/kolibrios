;---------------------------------------------------------------------
;--------------------textured triangle procedure----------------------
;---------------------------------------------------------------------

tex_triangle:
;----------in - eax - x1 shl 16 + y1
;-------------- ebx - x2 shl 16 + y2
;---------------ecx - x3 shl 16 + y3
;---------------edx - nothing
;---------------esi - pointer to texture buffer
;---------------edi - pointer to screen buffer
;-------------stack - texture coordinates
.tex_x1 equ  ebp+4
.tex_y1 equ  ebp+6
.tex_x2 equ  ebp+8
.tex_y2 equ  ebp+10
.tex_x3 equ  ebp+12
.tex_y3 equ  ebp+14

.x1 equ ebp-2 ;dw ?
.y1 equ ebp-4 ;dw ?
.x2 equ ebp-6 ;dw ?
.y2 equ ebp-8 ;dw ?
.x3 equ ebp-10 ;dw ?
.y3 equ ebp-12 ;dw ?
.dx12 equ ebp-16 ;dd ?
.dx13 equ ebp-20 ;dd ?
.dx23 equ ebp-24 ;dd ?
.tex_dx12 equ ebp-28 ;dd ?
.tex_dy12 equ ebp-32 ;dd ?
.tex_dx13 equ ebp-36 ;dd ?
.tex_dy13 equ ebp-40 ;dd ?
.tex_dx23 equ ebp-44 ;dd ?
.tex_dy23 equ ebp-48 ;dd ?
.tex_ptr equ ebp-52 ;dd ?

.scan_x2 equ ebp-56 ;dd ?
.scan_y2 equ ebp-60 ;dd ?
.scan_x1 equ ebp-64 ;dd ?
.scan_y1 equ ebp-68 ;dd ?

  mov ebp,esp
  sub esp,68
;if  Ext = MMX
;  emms
;end if
  mov edx,dword[.tex_x1]		  ; check all parameters
  or dx,dx
  jl .tt_end
  cmp dx,TEX_X-1
  jg .tt_end
  shr edx,16
  or dx,dx
  jl .tt_end
  cmp dx,TEX_Y-1
  jg .tt_end

  mov edx,dword[.tex_x2]
  or dx,dx
  jl .tt_end
  cmp dx,TEX_X-1
  jg .tt_end
  shr edx,16
  or dx,dx
  jl .tt_end
  cmp dx,TEX_Y-1
  jg .tt_end

  mov edx,dword[.tex_x3]
  or dx,dx
  jl .tt_end
  cmp dx,TEX_X-1
  jg .tt_end
  shr edx,16
  cmp dx,TEX_Y-1
  jg .tt_end
  or dx,dx
  jl .tt_end

  mov edx,eax	 ; check X&Y triangle coordinate
  or edx,ebx
  or edx,ecx
  test edx,80008000h
  jne .tt_end
  mov  dx,[size_x_var]
 ; or ax,ax
 ; jl .tt_end
;  cmp ax,SIZE_Y
;  jg .tt_end
  ror eax,16
 ; or ax,ax
 ; jl .tt_end
  cmp ax,dx  ;SIZE_X
  jg .tt_end
  rol eax,16

 ; or bx,bx
 ; jl .tt_end
;  cmp bx,SIZE_Y
;  jg .tt_end
  ror ebx,16
 ; or bx,bx
 ; jl .tt_end
  cmp bx,dx ;SIZE_X
  jg .tt_end
  rol ebx,16

 ; or cx,cx
 ; jl .tt_end
;  cmp cx,SIZE_Y
;  jg .tt_end
  ror ecx,16
 ; or cx,cx
 ; jl .tt_end
  cmp cx,dx  ;SIZE_X
  jg .tt_end
  rol ecx,16		       ; uff.. parameters was checked

  cmp ax,bx			    ;sort all parameters
  jle .tt_sort1
  xchg eax,ebx
  mov edx,dword [.tex_x1]
  xchg edx,dword [.tex_x2]
  mov dword[.tex_x1],edx
.tt_sort1:
  cmp ax,cx
  jle .tt_sort2
  xchg eax,ecx
  mov edx,dword [.tex_x1]
  xchg edx,dword [.tex_x3]
  mov dword [.tex_x1],edx
.tt_sort2:
  cmp bx,cx
  jle .tt_sort3
  xchg ebx,ecx
  mov edx,dword [.tex_x2]
  xchg edx,dword [.tex_x3]
  mov dword [.tex_x2],edx
.tt_sort3:
  mov [.y1],ax	   ; and store to user friendly variables
  shr eax,16
  mov [.x1],ax
  mov [.y2],bx
  shr ebx,16
  mov [.x2],bx
  mov [.y3],cx
  shr ecx,16
  mov [.x3],cx
  mov [.tex_ptr],esi

  movsx ebx,word[.y2]
  sub bx,[.y1]
  jnz .tt_dx12_make

  mov dword[.dx12],0
  mov dword[.tex_dx12],0
  mov dword[.tex_dy12],0
  jmp .tt_dx12_done
.tt_dx12_make:
  mov ax,[.x2]
  sub ax,[.x1]
  cwde
  shl eax,ROUND
  cdq
  idiv ebx
  mov [.dx12],eax				      ; dx12 = (x2-x1)/(y2-y1)

  mov ax,word[.tex_x2]
  sub ax,word[.tex_x1]
  cwde
  shl eax,ROUND
  cdq
  idiv ebx
  mov [.tex_dx12],eax			  ; tex_dx12 = (tex_x2-tex_x1)/(y2-y1)

  mov ax,word[.tex_y2]
  sub ax,word[.tex_y1]
  cwde
  shl eax,ROUND
  cdq
  idiv ebx
  mov [.tex_dy12],eax			  ; tex_dy12 = (tex_y2-tex_y1)/(y2-y1)
.tt_dx12_done:

  movsx ebx,word[.y3]
  sub bx,word[.y1]
  jnz .tt_dx13_make

  mov dword [.dx13],0
  mov dword [.tex_dx13],0
  mov dword [.tex_dy13],0
  jmp .tt_dx13_done
.tt_dx13_make:
  mov ax,[.x3]
  sub ax,[.x1]
  cwde
  shl eax,ROUND
  cdq
  idiv ebx
  mov [.dx13],eax				  ; dx13 = (x3-x1)/(y3-y1)

  mov ax,word[.tex_x3]
  sub ax,word[.tex_x1]
  cwde
  shl eax,ROUND
  cdq
  idiv ebx
  mov [.tex_dx13],eax			    ; tex_dx13 = (tex_x3-tex_x1)/(y3-y1)

  mov ax,word[.tex_y3]
  sub ax,word[.tex_y1]
  cwde
  shl eax,ROUND
  cdq
  idiv ebx
  mov [.tex_dy13],eax			    ; tex_dy13 = (tex_y3-tex_y1)/(y3-y1)
.tt_dx13_done:

  movsx ebx,word[.y3]
  sub bx,word[.y2]
  jnz .tt_dx23_make

  mov dword [.dx23],0
  mov dword [.tex_dx23],0
  mov dword [.tex_dy23],0
  jmp .tt_dx23_done
.tt_dx23_make:
  mov ax,[.x3]
  sub ax,[.x2]
  cwde
  shl eax,ROUND
  cdq
  idiv ebx
  mov [.dx23],eax				  ; dx23 = (x3-x2)/(y3-y2)

  mov ax,word[.tex_x3]
  sub ax,word[.tex_x2]
  cwde
  shl eax,ROUND
  cdq
  idiv ebx
  mov [.tex_dx23],eax			 ; tex_dx23 = (tex_x3-tex_x2)/(y3-y2)

  mov ax,word[.tex_y3]
  sub ax,word[.tex_y2]
  cwde
  shl eax,ROUND
  cdq
  idiv ebx
  mov [.tex_dy23],eax			; tex_dy23 = (tex_y3-tex_y2)/(y3-y2)
.tt_dx23_done:

  movsx eax,word[.x1]
  shl eax,ROUND
  mov ebx,eax

  movsx edx, word[.tex_x1]
  shl edx,ROUND
  mov [.scan_x1],edx
  mov [.scan_x2],edx
  movsx edx, word[.tex_y1]
  shl edx,ROUND
  mov [.scan_y1],edx
  mov [.scan_y2],edx

  mov cx,[.y1]
  cmp cx, [.y2]
  jge .tt_loop1_end
.tt_loop1:
  push edi
  push eax
  push ebx
  push cx
  push ebp
;; Madis
;if Ext=MMX                          ; With MMX enabled it reverse light vectors ????
;  mov   dword[esp-8],ROUND
;  mov   dword[esp-4],0 ; Is this a bug? Explanation down 3 lines
;  movq  mm0,qword[.scan_y1]
;  movq  mm1,qword[.scan_y2]
;  psrad mm0,[esp-8] ;This instr. won't allow modifiers BYTE, WORD, etc.
;  psrad mm1,[esp-8] ;It always defaults to QWORD
;  packssdw mm0,mm1
;  movq  [esp-8],mm0
;  sub   esp,8
;else

   push     dword[.scan_y2]  ; now I push variables on stack without shifting
   push     dword[.scan_x2]
   push     dword[.scan_y1]
   push     dword[.scan_x1]

;end if



  push	dword[.tex_ptr]

  push	cx
  mov	edx,ebx
  sar	edx,ROUND
  push	dx
  mov	edx,eax
  sar	edx,ROUND
  push	dx
  call	textured_line

  pop	ebp
  pop	cx
  pop	ebx
  pop	eax
  pop	edi

  mov	edx, [.tex_dx13]
  add	[.scan_x1], edx
  mov	edx, [.tex_dx12]
  add	[.scan_x2], edx
  mov	edx, [.tex_dy13]
  add	[.scan_y1], edx
  mov	edx, [.tex_dy12]
  add	[.scan_y2], edx

  add	eax, [.dx13]
  add	ebx, [.dx12]
  inc	cx
  cmp	cx,[.y2]
  jl	.tt_loop1

.tt_loop1_end:


  mov	cx, [.y2]
  cmp	cx, [.y3]
  jge	.tt_loop2_end

  movsx ebx,word[.x2]
  shl	ebx,ROUND

  movsx edx, word[.tex_x2]
  shl	edx,ROUND
  mov	[.scan_x2],edx
  movsx edx, word[.tex_y2]
  shl	edx,ROUND
  mov	[.scan_y2],edx

.tt_loop2:
  push	edi
  push	eax
  push	ebx
  push	cx
  push	ebp
;; Madis
;if Ext=MMX
;  mov   dword[esp-8],ROUND
;  mov   dword[esp-4],0 ; Is this a bug? Explanation down 3 lines
;  movq  mm0,qword[.scan_y1]
;  movq  mm1,qword[.scan_y2]
;  psrad mm0,[esp-8] ;This instr. won't allow modifiers BYTE, WORD, etc.
;  psrad mm1,[esp-8] ;It always defaults to QWORD
;  packssdw mm0,mm1
;  movq  [esp-8],mm0
;  sub   esp,8
;else

;end if
  push	dword[.scan_y2]
  push	dword[.scan_x2]
  push	dword[.scan_y1]
  push	dword[.scan_x1]



  push	dword[.tex_ptr]

  push	cx
  mov	edx,ebx
  sar	edx,ROUND
  push	dx
  mov	edx,eax
  sar	edx,ROUND
  push	dx
  call	textured_line

  pop	ebp
  pop	cx
  pop	ebx
  pop	eax
  pop	edi

  mov	edx, [.tex_dx13]
  add	[.scan_x1], edx
  mov	edx, [.tex_dx23]
  add	[.scan_x2], edx
  mov	edx, [.tex_dy13]
  add	[.scan_y1], edx
  mov	edx, [.tex_dy23]
  add	[.scan_y2], edx

  add	eax, [.dx13]
  add	ebx, [.dx23]
  inc	cx
  cmp	cx,[.y3]
  jl	.tt_loop2

.tt_loop2_end:

.tt_end:
 mov	esp,ebp

ret 12


textured_line:
;-----in -edi screen buffer pointer
;------------ stack:
  .x1 equ word [ebp+4]
  .x2 equ word [ebp+6]
  .y  equ word [ebp+8]

  .tex_ptr equ dword [ebp+10]
  .tex_x1  equ	[ebp+14]
  .tex_y1  equ	[ebp+18]
  .tex_x2  equ	[ebp+22]
  .tex_y2  equ	[ebp+26]

  .tex_dx equ ebp-4 ;dd ?
  .tex_dy equ ebp-8 ;dd ?

  mov	ebp,esp
  sub	esp,8

  mov	ax,.y
  or	ax,ax
  jl	.tl_quit
  mov	dx,[size_y_var]
  cmp	ax,dx ;SIZE_Y
  jg	.tl_quit

  mov	ax,.x1
  cmp	ax,.x2
  je	.tl_quit
  jl	.tl_ok

  xchg	ax,.x2
  mov	.x1,ax

if Ext >= MMX
  movq	  mm0,.tex_x1
  movq	  mm1,.tex_x2
  movq	  .tex_x2,mm0
  movq	  .tex_x1,mm1

else

  mov	eax,.tex_x1
  xchg	eax,.tex_x2
  mov	.tex_x1,eax

  mov	eax,.tex_y1
  xchg	eax,.tex_y2
  mov	.tex_y1,eax

end if

 .tl_ok:

  mov	ebx,edi
  movsx edi,.y
  movzx eax,word[size_x_var]
  lea	eax,[eax*3]
;  mov   eax,SIZE_X*3
  mul	edi
  mov	edi,eax
  movsx eax,.x1
  add	edi,eax
  shl	eax,1
  add	edi,eax
  add	edi,ebx

  mov	cx,.x2
  sub	cx,.x1
  movsx ecx,cx

  mov	eax,.tex_x2
  sub	eax,.tex_x1
  cdq
  idiv	ecx
  mov	[.tex_dx],eax		; tex_dx=(tex_x2-tex_x1)/(x2-x1)

  mov	eax,.tex_y2
  sub	eax,.tex_y1
  cdq
  idiv	ecx
  mov	[.tex_dy],eax		       ; tex_dy = (tex_y2-tex_y1)/(x2-x1)

  mov	eax,.tex_x1
  mov	ebx,.tex_y1
  cld
 .tl_loop:
  mov	edx,eax    ; eax - cur x
  mov	esi,ebx    ; ebx - cur y
  shr	edx,ROUND
  shr	esi,ROUND
 macro .fluent
 {
  push	eax
  push	edx
  mov	eax,TEX_X*3
  mul	esi
  mov	esi,eax
  pop	edx
  pop	eax
 }
 macro .shift
 {
  shl	esi,TEX_SHIFT
  lea	esi,[esi*3]
  ;push edx
  ;mov edx,esi
  ;shl esi,1
  ;add esi,edx
  ;pop edx
 }
  if TEX = FLUENTLY
    .fluent
  end if
  if TEX = SHIFTING
    .shift
  end if
  lea	edx,[edx*3]
  add	esi,edx
 ; shl edx,1
 ; add esi,edx
  add	esi,.tex_ptr
  movsd
  dec	edi
  add	eax,[.tex_dx]
  add	ebx,[.tex_dy]
  loop .tl_loop

 .tl_quit:
  mov esp,ebp

ret 18+8
;  .tex_dx dd ?
;  .tex_dy dd ?
