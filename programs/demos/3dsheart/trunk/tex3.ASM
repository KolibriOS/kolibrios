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
  mov ebp,esp

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

 ; or ax,ax
 ; jl .tt_end
  cmp ax,SIZE_Y
  jg .tt_end
  ror eax,16
 ; or ax,ax
 ; jl .tt_end
  cmp ax,SIZE_X
  jg .tt_end
  rol eax,16

 ; or bx,bx
 ; jl .tt_end
  cmp bx,SIZE_Y
  jg .tt_end
  ror ebx,16
 ; or bx,bx
 ; jl .tt_end
  cmp bx,SIZE_X
  jg .tt_end
  rol ebx,16

 ; or cx,cx
 ; jl .tt_end
  cmp cx,SIZE_Y
  jg .tt_end
  ror ecx,16
 ; or cx,cx
 ; jl .tt_end
  cmp cx,SIZE_X
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

  mov [.dx12],0
  mov [.tex_dx12],0
  mov [.tex_dy12],0
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

  movsx ebx,[.y3]
  sub bx,[.y1]
  jnz .tt_dx13_make

  mov [.dx13],0
  mov [.tex_dx13],0
  mov [.tex_dy13],0
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

  movsx ebx,[.y3]
  sub bx,[.y2]
  jnz .tt_dx23_make

  mov [.dx23],0
  mov [.tex_dx23],0
  mov [.tex_dy23],0
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

  movsx eax,[.x1]
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

  mov edx, [.scan_y2]
  sar edx, ROUND
  push dx
  mov edx, [.scan_x2]
  sar edx, ROUND
  push dx
  mov edx, [.scan_y1]
  sar edx, ROUND
  push dx
  mov edx, [.scan_x1]
  sar edx, ROUND
  push dx
  push [.tex_ptr]

  push cx
  mov edx,ebx
  sar edx,ROUND
  push dx
  mov edx,eax
  sar edx,ROUND
  push dx
  call textured_line

  pop ebp
  pop cx
  pop ebx
  pop eax
  pop edi

  mov edx, [.tex_dx13]
  add [.scan_x1], edx
  mov edx, [.tex_dx12]
  add [.scan_x2], edx
  mov edx, [.tex_dy13]
  add [.scan_y1], edx
  mov edx, [.tex_dy12]
  add [.scan_y2], edx

  add eax, [.dx13]
  add ebx, [.dx12]
  inc cx
  cmp cx,[.y2]
  jl .tt_loop1
.tt_loop1_end:


  mov cx,[.y2]
  cmp cx, [.y3]
  jge .tt_loop2_end

  movsx ebx,[.x2]
  shl ebx,ROUND

  movsx edx, word[.tex_x2]
  shl edx,ROUND
  mov [.scan_x2],edx
  movsx edx, word[.tex_y2]
  shl edx,ROUND
  mov [.scan_y2],edx

.tt_loop2:
  push edi
  push eax
  push ebx
  push cx
  push ebp

  mov edx, [.scan_y2]
  sar edx, ROUND
  push dx
  mov edx, [.scan_x2]
  sar edx, ROUND
  push dx
  mov edx, [.scan_y1]
  sar edx, ROUND
  push dx
  mov edx, [.scan_x1]
  sar edx, ROUND
  push dx
  push [.tex_ptr]

  push cx
  mov edx,ebx
  sar edx,ROUND
  push dx
  mov edx,eax
  sar edx,ROUND
  push dx
  call textured_line

  pop ebp
  pop cx
  pop ebx
  pop eax
  pop edi

  mov edx, [.tex_dx13]
  add [.scan_x1], edx
  mov edx, [.tex_dx23]
  add [.scan_x2], edx
  mov edx, [.tex_dy13]
  add [.scan_y1], edx
  mov edx, [.tex_dy23]
  add [.scan_y2], edx

  add eax, [.dx13]
  add ebx, [.dx23]
  inc cx
  cmp cx,[.y3]
  jl .tt_loop2
.tt_loop2_end:

.tt_end:
 mov esp,ebp
ret 12
.x1 dw ?
.y1 dw ?
.x2 dw ?
.y2 dw ?
.x3 dw ?
.y3 dw ?
.dx12 dd ?
.dx13 dd ?
.dx23 dd ?
.tex_dx12 dd ?
.tex_dy12 dd ?
.tex_dx13 dd ?
.tex_dy13 dd ?
.tex_dx23 dd ?
.tex_dy23 dd ?
.tex_ptr dd ?

.scan_x1 dd ?
.scan_y1 dd ?
.scan_x2 dd ?
.scan_y2 dd ?


textured_line:
;-----in -edi screen buffer pointer
;------------ stack:
  .x1 equ word [ebp+4]
  .x2 equ word [ebp+6]
  .y  equ word [ebp+8]

  .tex_ptr equ dword [ebp+10]
  .tex_x1 equ word [ebp+14]
  .tex_y1 equ word [ebp+16]
  .tex_x2 equ word [ebp+18]
  .tex_y2 equ word [ebp+20]

  mov ebp,esp

  mov ax,.y
  or ax,ax
  jl .tl_quit
  cmp ax,SIZE_Y
  jg .tl_quit

  mov ax,.x1
  cmp ax,.x2
  je .tl_quit
  jl .tl_ok

  xchg ax,.x2
  mov .x1,ax

  mov ax,.tex_x1
  xchg ax,.tex_x2
  mov .tex_x1,ax

  mov ax,.tex_y1
  xchg ax,.tex_y2
  mov .tex_y1,ax

 .tl_ok:
  mov ebx,edi
  movsx edi,.y
  mov eax,SIZE_X*3
  mul edi
  mov edi,eax
  movsx eax,.x1
  add edi,eax
  shl eax,1
  add edi,eax
  add edi,ebx

  mov cx,.x2
  sub cx,.x1
  movsx ecx,cx

  mov ax,.tex_x2
  sub ax,.tex_x1
  cwde
  shl eax,ROUND
  cdq
  idiv ecx
  mov [.tex_dx],eax	      ; tex_dx=(tex_x2-tex_x1)/(x2-x1)

  mov ax,.tex_y2
  sub ax,.tex_y1
  cwde
  shl eax,ROUND
  cdq
  idiv ecx
  mov [.tex_dy],eax		     ; tex_dy = (tex_y2-tex_y1)/(x2-x1)

  movsx eax,.tex_x1
  shl eax,ROUND
  movsx ebx,.tex_y1
  shl ebx,ROUND
  cld
 .tl_loop:
  mov edx,eax
  mov esi,ebx
  sar edx,ROUND
  sar esi,ROUND
 macro .fluent
 {
  push eax
  push edx
  mov eax,TEX_X*3
  mul esi
  mov esi,eax
  pop edx
  pop eax
 }
 macro .shift
 {
  shl esi,TEX_SHIFT
  lea esi,[esi*3]
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
  lea edx,[edx*3]
  add esi,edx
 ; shl edx,1
 ; add esi,edx
  add esi,.tex_ptr
  movsd
  dec edi
  add eax,[.tex_dx]
  add ebx,[.tex_dy]
  loop .tl_loop

 .tl_quit:
  mov esp,ebp
ret 18
  .tex_dx dd ?
  .tex_dy dd ?
