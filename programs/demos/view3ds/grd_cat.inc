ROUND equ 8
CATMULL_SHIFT equ 8
gouraud_triangle_z:

;----procedure drawing gouraud triangle with z coordinate
;----interpolation ( Catmull alghoritm )-----------------
;------------------in - eax - x1 shl 16 + y1 ------------
;---------------------- ebx - x2 shl 16 + y2 ------------
;---------------------- ecx - x3 shl 16 + y3 ------------
;---------------------- esi - pointer to Z-buffer--------
;---------------------- Z-buffer filled with dd variables
;---------------------- shifted CATMULL_SHIFT------------
;---------------------- edi - pointer to screen buffer---
;---------------------- stack : colors-------------------
;----------------- procedure don't save registers !!-----
.col1r equ ebp+4   ; each color as word
.col1g equ ebp+6   ; each z coordinate as word
.col1b equ ebp+8
.z1    equ ebp+10
.col2r equ ebp+12
.col2g equ ebp+14
.col2b equ ebp+16
.z2    equ ebp+18
.col3r equ ebp+20
.col3g equ ebp+22
.col3b equ ebp+24
.z3    equ ebp+26

.x1    equ word[ebp-2]
.y1    equ word[ebp-4]
.x2    equ word[ebp-6]
.y2    equ word[ebp-8]
.x3    equ word[ebp-10]
.y3    equ word[ebp-12]

.dx12  equ dword[ebp-16]
.dz12  equ dword[ebp-20]
.dc12r equ dword[ebp-24]
.dc12g equ dword[ebp-28]
.dc12b equ [ebp-32]

.dx13  equ dword[ebp-36]
.dz13  equ dword[ebp-40]
.dc13r equ dword[ebp-44]
.dc13g equ dword[ebp-48]
.dc13b equ [ebp-52]

.dx23  equ dword[ebp-56]
.dz23  equ dword[ebp-60]
.dc23r equ dword[ebp-64]
.dc23g equ dword[ebp-68]
.dc23b equ [ebp-72]

.zz1   equ dword[ebp-76]
.c1r   equ dword[ebp-80]
.c1g   equ dword[ebp-84]
.c1b   equ dword[ebp-88]
.zz2   equ dword[ebp-92]
.c2r   equ dword[ebp-96]
.c2g   equ dword[ebp-100]
.c2b   equ dword[ebp-104]
;.zz1   equ dword[ebp-100]
;.zz2   equ dword[ebp-104]

.c1bM equ [ebp-88]
.c2bM equ [ebp-104]
.c1rM equ [ebp-80]
.c2rM equ [ebp-96]
.dc23bM equ [ebp-72]
.dc13bM equ [ebp-52]
.dc12bM equ [ebp-32]
.dc12rM equ [ebp-24]
.dc13rM equ [ebp-44]
.dc23rM equ [ebp-64]
if Ext=MMX
      emms
end if

       mov     ebp,esp
     ;  sub     esp,84
 .sort3:                  ; sort triangle coordinates...
       cmp     ax,bx
       jle     .sort1
       xchg    eax,ebx
       mov     edx,dword[.col1r]
       xchg    edx,dword[.col2r]
       mov     dword[.col1r],edx
       mov     edx,dword[.col1b]
       xchg    edx,dword[.col2b]
       mov     dword[.col1b],edx
 .sort1:
       cmp      bx,cx
       jle      .sort2
       xchg     ebx,ecx
       mov      edx,dword[.col2r]
       xchg     edx,dword[.col3r]
       mov      dword[.col2r],edx
       mov      edx,dword[.col2b]
       xchg     edx,dword[.col3b]
       mov      dword[.col2b],edx
       jmp .sort3
 .sort2:
       push     eax          ; store in variables
       push     ebx
       push     ecx
         mov      edx,80008000h  ; eax,ebx,ecx are ANDd together into edx which means that
         and      edx,ebx        ; if *all* of them are negative a sign flag is raised
         and      edx,ecx
         and      edx,eax
         test     edx,80008000h  ; Check both X&Y at once
         jne      .gt_loop2_end

       mov      bx,.y2       ; calc deltas
       sub      bx,.y1
       jnz      .gt_dx12_make
      ; mov      .dx12,0
      ; mov      .dz12,0
      ; mov      .dc12r,0
      ; mov      .dc12g,0
      ; mov      .dc12b,0
       mov      ecx,5
     @@:
       push     dword 0
       loop     @b
       jmp      .gt_dx12_done
  .gt_dx12_make:

if Ext>= SSE2

       movsx    ebx,bx
       mov      eax,1 shl 15
       cdq
       idiv     ebx
    ;   push     eax
       mov      ebx,eax


       mov      ax,.x2
       sub      ax,.x1
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax
    ;   mov      .dx12,eax

       sub       esp,4*4
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
       movq      xmm1,[.col1r]
       movq      xmm2,[.col2r]
       psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       punpcklwd xmm2,xmm3
       psrad     xmm2,15 - ROUND
       pshufd    xmm2,xmm2,11000110b
       movdqu    .dc12b,xmm2
else
       mov      ax,.x2
       sub      ax,.x1
       cwde
       movsx    ebx,bx
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;      mov      .dx12,eax
       push      eax

       mov      ax,word[.z2]
       sub      ax,word[.z1]
       cwde
       shl      eax,CATMULL_SHIFT
       cdq
       idiv     ebx
       push     eax

       mov      ax,word[.col2r]
       sub      ax,word[.col1r]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
      ; mov      .dc12r,eax
       push       eax
       mov        ax,word[.col2g]
       sub        ax,word[.col1g]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
     ;  mov .dc12g,eax
       push     eax
       mov      ax,word[.col2b]        ;;---
       sub      ax,word[.col1b]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
      ; mov .dc12b,eax
       push     eax
end if
   .gt_dx12_done:

       mov      bx,.y3       ; calc deltas
       sub      bx,.y1
       jnz      .gt_dx13_make
      ; mov      .dx13,0
      ; mov      .dz13,0
      ; mov      .dc13r,0
      ; mov      .dc13g,0
      ; mov      .dc13b,0
       mov      ecx,5
     @@:
       push     dword 0
       loop     @b
       jmp      .gt_dx13_done
    .gt_dx13_make:

if Ext>= SSE2

       movsx    ebx,bx
       mov      eax,1 shl 15
       cdq
       idiv     ebx
       mov      ebx,eax


       mov      ax,.x3
       sub      ax,.x1
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax

       sub       esp,4*4
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
       movq      xmm1,[.col1r]
       movq      xmm2,[.col3r]
       psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       punpcklwd xmm2,xmm3
       psrad     xmm2,15 - ROUND
       pshufd    xmm2,xmm2,11000110b
       movdqu    .dc13b,xmm2
else

       mov      ax,.x3
       sub      ax,.x1
       cwde
       movsx    ebx,bx
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;      mov      .dx13,eax
       push      eax

       mov      ax,word[.z3]
       sub      ax,word[.z1]
       cwde
       shl      eax,CATMULL_SHIFT
       cdq
       idiv     ebx
       push     eax

       mov      ax,word[.col3r]
       sub      ax,word[.col1r]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
      ; mov      .dc13r,eax
       push       eax
       mov        ax,word[.col3g]
       sub        ax,word[.col1g]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
     ;  mov .dc13g,eax
       push     eax
       mov      ax,word[.col3b]
       sub      ax,word[.col1b]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
      ; mov .dc13b,eax
       push     eax
end if
   .gt_dx13_done:

       mov      bx,.y3       ; calc deltas
       sub      bx,.y2
       jnz      .gt_dx23_make
      ; mov      .dx23,0
      ; mov      .dz23,0
      ; mov      .dc23r,0
      ; mov      .dc23g,0
      ; mov      .dc23b,0
       mov      ecx,5
     @@:
       push     dword 0
       loop     @b
       jmp      .gt_dx23_done
    .gt_dx23_make:

if Ext>= SSE2

       movsx    ebx,bx
       mov      eax,1 shl 15
       cdq
       idiv     ebx
    ;   push     eax
       mov      ebx,eax

       mov      ax,.x3
       sub      ax,.x2
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax

       sub       esp,4*4
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
       movq      xmm1,[.col2r]
       movq      xmm2,[.col3r]
       psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       punpcklwd xmm2,xmm3
       psrad     xmm2,15 - ROUND
       pshufd    xmm2,xmm2,11000110b
       movdqu    .dc23b,xmm2
else


       mov      ax,.x3
       sub      ax,.x2
       cwde
       movsx    ebx,bx
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;      mov      .dx23,eax
       push      eax

       mov      ax,word[.z3]
       sub      ax,word[.z2]
       cwde
       shl      eax,CATMULL_SHIFT
       cdq
       idiv     ebx
       push     eax

       mov      ax,word[.col3r]
       sub      ax,word[.col2r]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
      ; mov     .dc23r,eax
       push     eax
       mov      ax,word[.col3g]
       sub      ax,word[.col2g]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
     ;  mov .dc23g,eax
       push     eax
       mov      ax,word[.col3b]
       sub      ax,word[.col2b]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
      ; mov .dc23b,eax
       push     eax
end if
   .gt_dx23_done:
       sub      esp,32

       movsx    eax,.x1                    ; eax - cur x1
       shl      eax,ROUND                  ; ebx - cur x2
       mov      ebx,eax
       movsx    edx,word[.z1]
       shl      edx,CATMULL_SHIFT
       mov      .zz1,edx
       mov      .zz2,edx
       movzx    edx,word[.col1r]
       shl      edx,ROUND
       mov      .c1r,edx
       mov      .c2r,edx
       movzx    edx,word[.col1g]
       shl      edx,ROUND
       mov      .c1g,edx
       mov      .c2g,edx
       movzx    edx,word[.col1b]
       shl      edx,ROUND
       mov      .c1b,edx
       mov      .c2b,edx
       mov      cx,.y1
       cmp      cx,.y2
       jge      .gt_loop1_end

    .gt_loop1:
       pushad
    ; macro .debug

       mov      edx,.c2r              ; c2r,c2g,c2b,c1r,c1g,c1b - current colors
       sar      edx,ROUND
       push     dx
       mov      edx,.c2g
       sar      edx,ROUND
       push     dx
       mov      edx,.c2b
       sar      edx,ROUND
       push     dx
       sar      ebx,ROUND    ; x2
       push     bx
       mov      edx,.c1r
       sar      edx,ROUND
       push     dx
       mov      edx,.c1g
       sar      edx,ROUND
       push     dx
       mov      edx,.c1b
       sar      edx,ROUND
       push     dx
       sar      eax,ROUND
       push     ax            ; x1
       push     cx            ; y
       push     .zz2
       push     .zz1
       call     gouraud_line_z

       popad

if Ext >= MMX
       movq     mm0,.c1bM
       paddd    mm0,qword .dc13bM
       movq     .c1bM,mm0
       movq     mm1,.c2bM
       paddd    mm1,qword .dc12bM
       movq     .c2bM,mm1

       movq     mm0,.c1rM
       paddd    mm0,qword .dc13rM
       movq     .c1rM,mm0
       movq     mm1,.c2rM
       paddd    mm1,qword .dc12rM
       movq     .c2rM,mm1
else
       mov      edx,.dc13r
       add      .c1r,edx
       mov      edx,.dc13g
       add      .c1g,edx
       mov      edx,.dc13b
       add      .c1b,edx
       mov      edx,.dc12r
       add      .c2r,edx
       mov      edx,.dc12g
       add      .c2g,edx
       mov      edx,.dc12b
       add      .c2b,edx

       mov      edx,.dz13
       add      .zz1,edx
       mov      edx,.dz12
       add      .zz2,edx
end if
       add      eax,.dx13
       add      ebx,.dx12
       inc      cx
       cmp      cx,.y2
       jl       .gt_loop1

   .gt_loop1_end:
       mov      cx,.y2
       cmp      cx,.y3
       jge      .gt_loop2_end

       movsx    ebx,.x2                    ; eax - cur x1
       shl      ebx,ROUND                  ; ebx - cur x2
       movsx    edx,word[.z2]
       shl      edx,CATMULL_SHIFT
       mov      .zz2,edx
       movzx    edx,word[.col2r]
       shl      edx,ROUND
       mov      .c2r,edx
       movzx    edx,word[.col2g]
       shl      edx,ROUND
       mov      .c2g,edx
       movzx    edx,word[.col2b]
       shl      edx,ROUND
       mov      .c2b,edx

    .gt_loop2:
       pushad
    ; macro .debug

       mov      edx,.c2r              ; c2r,c2g,c2b,c1r,c1g,c1b - current colors
       sar      edx,ROUND
       push     dx
       mov      edx,.c2g
       sar      edx,ROUND
       push     dx
       mov      edx,.c2b
       sar      edx,ROUND
       push     dx
       sar      ebx,ROUND    ; x2
       push     bx
       mov      edx,.c1r
       sar      edx,ROUND
       push     dx
       mov      edx,.c1g
       sar      edx,ROUND
       push     dx
       mov      edx,.c1b
       sar      edx,ROUND
       push     dx
       sar      eax,ROUND
       push     ax            ; x1
       push     cx            ; y
       push     .zz2
       push     .zz1
       call     gouraud_line_z

       popad

if Ext >= MMX
       movq     mm0,.c1bM
       paddd    mm0,qword .dc13bM
       movq     .c1bM,mm0
       movq     mm1,.c2bM
       paddd    mm1,qword .dc23bM
       movq     .c2bM,mm1

       movq     mm0,.c1rM
       paddd    mm0,qword .dc13rM
       movq     .c1rM,mm0
       movq     mm1,.c2rM
       paddd    mm1,qword .dc23rM
       movq     .c2rM,mm1
else
       mov      edx,.dc13r
       add      .c1r,edx
       mov      edx,.dc13g
       add      .c1g,edx
       mov      edx,.dc13b
       add      .c1b,edx
       mov      edx,.dc23r
       add      .c2r,edx
       mov      edx,.dc23g
       add      .c2g,edx
       mov      edx,.dc23b
       add      .c2b,edx
       mov      edx,.dz13
       add      .zz1,edx
       mov      edx,.dz23
       add      .zz2,edx
end if
       add      eax,.dx13
       add      ebx,.dx23
       inc      cx
       cmp      cx,.y3
       jl       .gt_loop2
   .gt_loop2_end:

       mov      esp,ebp
ret 24
gouraud_line_z:
;----------------- procedure drawing gouraud line
;----------------- with z coordinate interpolation
;----------------- esi - pointer to Z_buffer
;----------------- edi - pointer to screen buffer
;----------------- stack:
.z1  equ dword[ebp+4]   ; z coordiunate shifted left CATMULL_SHIFT
.z2  equ dword[ebp+8]
.y   equ word[ebp+12]
.x1  equ ebp+14

.c1b equ ebp+16
.c1g equ ebp+18
.c1r equ ebp+20
.x2  equ ebp+22
.c2b equ ebp+24
.c2g equ ebp+26
.c2r equ ebp+28

.dz   equ dword[ebp-4]
.dc_b equ dword[ebp-8]
.dc_g equ dword[ebp-12]
.dc_r equ dword[ebp-16]
.c_z  equ dword[ebp-20]
.cb   equ dword[ebp-24]
.cg   equ dword[ebp-28]
.cr   equ dword[ebp-32]
;.cg2  equ dword[ebp-36]


.crM  equ ebp-32
.cgM  equ ebp-28
.cbM  equ ebp-24

.dc_rM equ ebp-16
.dc_gM equ ebp-12
.dc_bM equ ebp-8
        mov       ebp,esp

        mov     ax,.y
        or      ax,ax
        jl      .gl_quit
        mov     bx,[size_y_var]
        dec     bx
        cmp     ax,bx ;SIZE_Y
        jge     .gl_quit

        mov     eax,dword[.x1]
        cmp     ax,word[.x2]
        je      .gl_quit
        jl      @f

        xchg    eax,dword[.x2]
        mov     dword[.x1],eax
        mov     eax,dword[.c1g]
        xchg    eax,dword[.c2g]
        mov     dword[.c1g],eax
        mov     eax,.z1
        xchg    eax,.z2
        mov     .z1,eax
   @@:
        mov     bx,[size_x_var]
        dec     bx
        cmp     word[.x1],bx  ;SIZE_X
        jge     .gl_quit
        cmp     word[.x2],0
        jle     .gl_quit

if 0
       mov     bx,word[.x2]      ; dz = z2-z1/x2-x1
       sub     bx,word[.x1]
       movsx   ebx,bx


       mov      eax,1 shl 15
       cdq
       idiv     ebx
       mov      ebx,eax


       mov      eax,.x3
       sub      eax,.x1
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax

       sub       esp,4*4
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
       movq      xmm1,[.col1r]
       movq      xmm2,[.col3r]
       psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       punpcklwd xmm2,xmm3
       psrad     xmm2,15 - ROUND
       pshufd    xmm2,xmm2,11000110b
       movdqu    .dc13b,xmm2


end if


        mov     eax,.z2
        sub     eax,.z1
        cdq
        mov     bx,word[.x2]      ; dz = z2-z1/x2-x1
        sub     bx,word[.x1]
        movsx   ebx,bx
        idiv    ebx
        push    eax


        mov      eax,1 shl 15
        cdq
        idiv     ebx
        mov      ebx,eax


        mov     ax,word[.c2b]
        sub     ax,word[.c1b]
        cwde
        imul     ebx
        sar      eax,15 - ROUND
        push     eax


        mov     ax,word[.c2g]
        sub     ax,word[.c1g]
        cwde
        imul     ebx
        sar      eax,15 - ROUND
        push     eax



        mov     ax,word[.c2r]
        sub     ax,word[.c1r]
        cwde
        imul     ebx
        sar      eax,15 - ROUND
        push     eax

        cmp     word[.x1],0     ; clipping on function
        jg      @f
        mov     eax,.dz
        movsx   ebx,word[.x1]
        neg     ebx
        imul    ebx
        add     .z1,eax
        mov     word[.x1],0

        mov     eax,.dc_r
        imul    ebx
        sar     eax,ROUND
        add     word[.c1r],ax

        mov     eax,.dc_g
        imul    ebx
        sar     eax,ROUND
        add     word[.c1g],ax

        mov     eax,.dc_b
        imul    ebx
        sar     eax,ROUND
        add     word[.c1b],ax

      @@:
        mov     bx,[size_x_var]
        dec     bx
        cmp     word[.x2],bx  ;SIZE_X
        jl      @f
        mov     word[.x2],bx  ;SIZE_X
     @@:
        sub     esp,16      ; calculate memory begin
        movzx   edx,word[size_x_var]  ;SIZE_X       ; in buffers
        movzx   eax,.y
        mul     edx
        movzx   edx,word[.x1]
        add     eax,edx
        push    eax
        lea     eax,[eax*3]
        add     edi,eax
        pop     eax
        shl     eax,2
        add     esi,eax

        mov     cx,word[.x2]
        sub     cx,word[.x1]
        movzx   ecx,cx
        mov     ebx,.z1          ; ebx - currrent z shl CATMULL_SIFT
;if Ext >= SSE
;        mov     .cz,edx
;end if
        mov     edx,.dz          ; edx - delta z
if Ext >= SSE2
        movq      xmm7,[.c1b]
        pshuflw   xmm7,xmm7,11000110b
        punpcklwd xmm7,[the_zero]
        pslld     xmm7,ROUND
        movdqu    xmm1,[.dc_rM]
end if
if Ext = NON
        movzx   eax,word[.c1r]
        shl     eax,ROUND
        mov     .cr,eax
        movzx   eax,word[.c1g]
        shl     eax,ROUND
        mov     .cg,eax
        movzx   eax,word[.c1b]
        shl     eax,ROUND
        mov     .cb,eax
end if
if (Ext = MMX) | (Ext=SSE)
;        mov     .c_z,edx
        movd    mm2,[.dc_bM]         ; delta color blue MMX
        movd    mm3,[.cbM]           ; current blue MMX
        movq    mm5,[.dc_rM]
        movq    mm4,[.crM]
        pxor    mm6,mm6
end if


      .ddraw:
;if Ext = MMX
;        movq    mm0,mm3
;        psrsq   mm0,32
;        movd    ebx,mm0
;end if
        cmp     ebx,dword[esi]   ; esi - z_buffer
        jge     @f               ; edi - Screen buffer
if Ext >= SSE2
        movdqa   xmm0,xmm7
        psrld    xmm0,ROUND
        packssdw xmm0,xmm0
        packuswb xmm0,xmm0
        movd     eax,xmm0
        stosw
        shr      eax,16
        stosb
end if
if (Ext=MMX) | (Ext=SSE)
        movq    mm0,mm3          ; mm0, mm1 - temp registers
        psrld   mm0,ROUND
        movq    mm1,mm4
        psrld   mm1,ROUND
        packssdw  mm1,mm0
        packuswb  mm1,mm6
;        movd     [edi],mm1
        movd      eax,mm1
        stosw
        shr       eax,16
        stosb
end if
if Ext=NON
        mov     eax,.cr
        sar     eax,ROUND
        stosb
        mov     eax,.cg
        sar     eax,ROUND
        stosb
        mov     eax,.cb
        sar     eax,ROUND
        stosb
end if
        mov     dword[esi],ebx
;if Ext = NON
        jmp     .no_skip
;end if
      @@:
        add     edi,3
      .no_skip:
        add     esi,4
;if Ext=NON
        add     ebx,edx
;end if
if Ext >=SSE2
        paddd   xmm7,xmm1
end if
if (Ext=MMX) | (Ext=SSE)
        paddd   mm3,mm2
        paddd   mm4,mm5
end if
if Ext = NON
        mov     eax,.dc_g
        add     .cg,eax
        mov     eax,.dc_b
        add     .cb,eax
        mov     eax,.dc_r
        add     .cr,eax
end if
        loop    .ddraw

   .gl_quit:
        mov       esp,ebp
ret 26
