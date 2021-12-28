

CATMULL_SHIFT  equ 8
ROUND equ 8
;NON=0
;MMX=1
;SSE=2
;SSE2=3

;Ext=SSE2


;TEX_SIZE=0x3fff
;SIZE_X equ 512
;SIZE_Y equ 512
;ROUND = 8
 ; TEX_SHIFT equ 6
 ; TEXTURE_SIZE = 0xFFFFF

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

   .z3      equ [ebp+26]
   .col3b   equ [ebp+24]
   .col3g   equ [ebp+22]
   .col3r   equ [ebp+20]

   .z2      equ [ebp+18]
   .col2b   equ [ebp+16]
   .col2g   equ [ebp+14]
   .col2r   equ [ebp+12]

   .z1      equ [ebp+10]
   .col1b   equ [ebp+8]
   .col1g   equ [ebp+6]
   .col1r   equ [ebp+4]

; local variables:

   .tex_ptr   equ dword[ebp-4]
   .z_ptr     equ dword[ebp-8]
   .scr_buff  equ dword[ebp-12]

   .x1        equ  word[ebp-14] ;dw ? ;equ word[ebp-10]
   .y1        equ  word[ebp-16] ;dw ? ;equ word[ebp-12]
   .x2        equ  word[ebp-18] ;dw ? ;equ word[ebp-14]
   .y2        equ  word[ebp-20] ;dw ? ;equ word[ebp-16]
   .x3        equ  word[ebp-22] ;dw ? ;equ word[ebp-18]
   .y3        equ  word[ebp-24] ;dw ? ;equ word[ebp-20]

   .dx12      equ  dword[ebp-28] ;dd ?
   .tex_dy12  equ       [ebp-32] ;dd ?
   .tex_dx12  equ       [ebp-36] ;dd ?
   .dz12      equ  dword[ebp-40] ;dd ?
   .dc12r     equ       [ebp-44] ;dd ?
   .dc12g     equ  dword[ebp-48] ;dd ?
   .dc12b     equ       [ebp-52] ;dd ?

   .dx23      equ  dword[ebp-56] ;dd ?
   .tex_dy23  equ       [ebp-60] ;dd ?
   .tex_dx23  equ       [ebp-64] ;dd ?
   .dz23      equ  dword[ebp-68] ;dd ?
   .dc23r     equ       [ebp-72] ;dd ?
   .dc23g     equ  dword[ebp-76] ;dd ?
   .dc23b     equ       [ebp-80] ;dword[ebp-8]dd ?

   .dx13      equ  dword[ebp-84] ;dd ?
   .tex_dy13  equ       [ebp-88] ;dd ?
   .tex_dx13  equ       [ebp-92] ;dd ?
   .dz13      equ  dword[ebp-96] ;dd ?
   .dc13r     equ       [ebp-100] ;dd ?
   .dc13g     equ  dword[ebp-104] ;dd ?
   .dc13b     equ       [ebp-108] ;dd ?

   .scan_y1   equ       [ebp-112] ;dd ?
   .scan_x1   equ       [ebp-116] ;dd ?
   .zz1       equ  dword[ebp-120] ;dw ?
   .cur1r     equ       [ebp-124] ;dw ?
   .cur1g     equ       [ebp-128] ;dw ?
   .cur1b     equ       [ebp-132] ;dw ?

   .scan_y2   equ       [ebp-136] ;dd ?
   .scan_x2   equ       [ebp-140] ;dd ?
   .zz2       equ       [ebp-144] ;dw ?
   .cur2r     equ       [ebp-148] ;dw ?
   .cur2g     equ       [ebp-152] ;dw ?
   .cur2b     equ       [ebp-156] ;dw ?


       mov    ebp,esp

         push    edx esi edi

         mov      edx,80008000h  ; eax,ebx,ecx are ANDd together into edx which means that
         and      edx,ebx        ; if *all* of them are negative a sign flag is raised
         and      edx,ecx
         and      edx,eax
         test     edx,80008000h  ; Check both X&Y at once
         jne      .loop2_end

   .sort3:
       cmp     ax,bx
       jle     .sort1
       xchg    eax,ebx
if Ext>=MMX
       movq    mm0, .col1r       ; exchange r, g, b, z
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
       cmp      bx,cx
       jle      .sort2
       xchg    ebx,ecx

if Ext>=MMX
       movq    mm0, .col2r       ; exchange r, g, b, z
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

       push     eax ebx ecx         ; store in variables
  ;     push     ebx
  ;     push     ecx

;****************** delta computng zone **************
;+++++++++ first zone
       mov      bx,.y2       ; calc delta12
       sub      bx,.y1
       jnz      .dx12_make
       mov      ecx,7
     @@:
       push     dword 0
       loop     @b
       jmp      .dx12_done
  .dx12_make:
     ;  sub      esp,7*4

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

       sub       esp,6*4
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
    ;   pshufd    xmm0,xmm0,0
       movlhps   xmm0,xmm0
       movq      xmm1,.col1r
       movq      xmm2,.col2r
       movhps    xmm1,.tex_x1
       movhps    xmm2,.tex_x2
       psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       movhlps   xmm4,xmm2
       movhlps   xmm5,xmm3
       punpcklwd xmm2,xmm3
       punpcklwd xmm4,xmm5
       psrad     xmm2,15 - ROUND
       psrad     xmm4,15 - ROUND
       pshufd    xmm2,xmm2,11000110b
       movdqu    .dc12b,xmm2
   ;    punpcklwd xmm4,xmm5
    ;   psrad     xmm4,15 - ROUND
       movq      .tex_dx12,xmm4


;+++++++++++++++++ second zone +++++++++++++
   .dx12_done:

       mov      bx,.y3       ; calc delta23
       sub      bx,.y2
       jnz      .dx23_make
       mov      ecx,7
    @@:
       push     dword 0
       loop     @b
       jmp      .dx23_done

  .dx23_make:
       movsx    ebx,bx
       mov      eax,1 shl 15
       cdq
       idiv     ebx
       mov      ebx,eax


       mov      ax,.x3
       sub      ax,.x2
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax

       sub       esp,6*4
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
       movlhps   xmm0,xmm0
       movq      xmm1,.col2r
       movq      xmm2,.col3r
       movhps    xmm1,.tex_x2
       movhps    xmm2,.tex_x3
       psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       movhlps   xmm4,xmm2
       movhlps   xmm5,xmm3
       punpcklwd xmm2,xmm3
       punpcklwd xmm4,xmm5
       psrad     xmm2,15 - ROUND
       psrad     xmm4,15 - ROUND
       pshufd    xmm2,xmm2,11000110b
       movdqu    .dc23b,xmm2
       movq      .tex_dx23,xmm4


   .dx23_done:
;++++++++++++++++++third zone++++++++++++++++++++++++
       mov      bx,.y3       ; calc delta13
       sub      bx,.y1
       jnz      .dx13_make
       mov      ecx,7
     @@:
       push     dword 0
       loop     @b
       jmp      .dx13_done
  .dx13_make:
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

       sub       esp,6*4
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
       movlhps   xmm0,xmm0
       movq      xmm1,.col1r
       movq      xmm2,.col3r
       movhps    xmm1,.tex_x1
       movhps    xmm2,.tex_x3
       psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       movhlps   xmm4,xmm2
       movhlps   xmm5,xmm3
       punpcklwd xmm2,xmm3
       punpcklwd xmm4,xmm5
       psrad     xmm2,15 - ROUND
       psrad     xmm4,15 - ROUND
       pshufd    xmm2,xmm2,11000110b
       movdqu    .dc13b,xmm2
       movq      .tex_dx13,xmm4
   .dx13_done:

; <<<<<<<  ::delta zone end+++++++++++++++++++++ >>>>>>>>

       sub      esp,(12*4)

       movsx    eax,.x1                    ; eax - cur x1
       shl      eax,ROUND                  ; ebx - cur x2
       mov      ebx,eax


       movzx    edi,word .tex_x1
       shl      edi,ROUND
       mov      .scan_x1,edi
       mov      .scan_x2,edi
    ;   push     edi
    ;   push     edi
       movzx    edx,word .tex_y1
       shl      edx,ROUND
  ;     push     edx
  ;     push     edx
       mov      .scan_y1,edx
       mov      .scan_y2,edx

       movsx    edx,word .z1
       shl      edx,CATMULL_SHIFT
   ;    push     edx
   ;    push     edx
       mov      .zz1,edx
       mov      .zz2,edx

       movzx    edi,word .col1r
       shl      edi,ROUND
       mov      .cur1r,edi
       mov      .cur2r,edi
       movzx    esi,word .col1g
       shl      esi,ROUND
       mov      .cur1g,esi
       mov      .cur2g,esi
       movzx    edx,word .col1b
       shl      edx,ROUND
       mov      .cur1b,edx
       mov      .cur2b,edx


       mov      cx,.y1
       cmp      cx,.y2
       jge      .loop1_end
    .loop_1:

       pushad

       push     .tex_ptr
       push     .scr_buff
       push     .z_ptr
       push     cx

       push     dword .zz2

       push     dword .cur2b
       push     dword .cur2g
       push     dword .cur2r
       push     dword .scan_x2
       push     dword .scan_y2

       push      .zz1

       push     dword .cur1b
       push     dword .cur1g
       push     dword .cur1r
       push     dword .scan_x1
       push     dword .scan_y1

       sar      eax,ROUND
       sar      ebx,ROUND
       call     horizontal_tex_grd_line


       popad


       movups   xmm0,.cur1b
       movups   xmm1,.dc13b
       movups   xmm2,.cur2b
       movups   xmm3,.dc12b
       movq     mm2,.scan_x1
       movq     mm5,.scan_x2
       paddd    xmm0,xmm1
       paddd    xmm2,xmm3
       paddd    mm2,.tex_dx13
       paddd    mm5,.tex_dx12
       movq     .scan_x1,mm2
       movq     .scan_x2,mm5
       movups   .cur1b,xmm0
       movups   .cur2b,xmm2


       add      eax,.dx13
       add      ebx,.dx12
       inc      cx
       cmp      cx,.y2
       jl      .loop_1
 .loop1_end:
       movzx    ecx,.y2
       cmp      cx,.y3
       jge      .loop2_end

       movsx    ebx,.x2                    ; eax - cur x1
       shl      ebx,ROUND                  ; ebx - cur x2

       movsx    edx,word .z2
       shl      edx,CATMULL_SHIFT
;       mov      .zz1,edx
       mov      .zz2,edx

       movzx    edi,word .col2r
       shl      edi,ROUND
   ;    mov      .cur1r,edi
       mov      .cur2r,edi
       movzx    esi,word .col2g
       shl      esi,ROUND
   ;    mov      .cur1g,esi
       mov      .cur2g,esi
       movzx    edx,word .col2b
       shl      edx,ROUND
   ;    mov      .cur1b,edx
       mov      .cur2b,edx

       movzx    edi,word .tex_x2
       shl      edi,ROUND
 ;      mov      .scan_x1,edi
       mov      .scan_x2,edi
       movzx    edx,word .tex_y2
       shl      edx,ROUND
 ;      mov      .scan_y1,edx
       mov      .scan_y2,edx

  .loop_2:
       pushad

       push     .tex_ptr
       push     .scr_buff
       push     .z_ptr
       push     cx

       push     dword .zz2

       push     dword .cur2b
       push     dword .cur2g
       push     dword .cur2r
       push     dword .scan_x2
       push     dword .scan_y2

       push      .zz1

       push     dword .cur1b
       push     dword .cur1g
       push     dword .cur1r
       push     dword .scan_x1
       push     dword .scan_y1

       sar      eax,ROUND
       sar      ebx,ROUND
       call     horizontal_tex_grd_line

       popad


       movups   xmm0,.cur1b
       movups   xmm1,.dc13b
       movups   xmm2,.cur2b
       movups   xmm3,.dc23b
       movq     mm2,.scan_x1
       movq     mm5,.scan_x2
       paddd    xmm0,xmm1
       paddd    xmm2,xmm3
       paddd    mm2,.tex_dx13
       paddd    mm5,.tex_dx23
       movq     .scan_x1,mm2
       movq     .scan_x2,mm5
       movups   .cur1b,xmm0
       movups   .cur2b,xmm2

       add      eax,.dx13
       add      ebx,.dx23
       inc      cx
       cmp      cx,.y3
       jl       .loop_2

.loop2_end:
       mov      esp,ebp
ret 36
horizontal_tex_grd_line:
;in:
; eax : x1, ebx : x2

.tex_ptr  equ [ebp+62]
.screen   equ [ebp+58]
.z_buffer equ [ebp+54]
.y        equ [ebp+52]

.z2     equ [ebp+48]
.b2     equ [ebp+44]
.g2     equ [ebp+40]
.r2     equ [ebp+36]
.tex_x2 equ [ebp+32]
.tex_y2 equ [ebp+28]


.z1     equ [ebp+24]
.b1     equ [ebp+20]
.g1     equ [ebp+16]
.r1     equ [ebp+12]
.tex_x1 equ [ebp+8]
.tex_y1 equ [ebp+4]


.x1 equ  word[ebp-2]
.x2 equ  word[ebp-4]
.dz equ dword[ebp-8]
.db equ [ebp-12]
.dg equ dword[ebp-16]
.dr equ [ebp-20]
.dtex_x equ dword[ebp-24]
.dtex_y equ [ebp-28]

        mov     ebp,esp


        mov     cx,word .y
        or      cx,cx
        jl      .quit_l

        cmp     cx,word[size_y_var]  ;SIZE_Y
        jge     .quit_l

        cmp     ax,bx
        je      .quit_l
        jl      @f

        xchg    eax,ebx


       movdqu  xmm0,.tex_y1
       movdqu  xmm1,.tex_y2
       movdqu  .tex_y1,xmm1
       movdqu  .tex_y2,xmm0
       movq    xmm4,.b1    ; x, z
       movq    xmm5,.b2
       movq    .b1,xmm5
       movq    .b2,xmm4


    @@:
        or      bx,bx
        jle     .quit_l
        cmp     ax,word[size_x_var]  ;SIZE_X
        jge     .quit_l

        push    ax
        push    bx
if 1
        mov     bx,.x2
        sub     bx,.x1

        movsx    ebx,bx
        mov      eax,1 shl 15
        cdq
        idiv     ebx
        mov      ebx,eax


        mov     eax,.z2            ; delta zone************
        sub     eax,.z1
        imul    ebx
        sar     eax,15
        push    eax  ; .dz

        mov     eax,.b2
        sub     eax,.b1
        imul    ebx
        sar     eax,15
        push    eax

        mov     eax,.g2
        sub     eax,.g1
        imul    ebx
        sar     eax,15
        push    eax  ; .dz

        mov     eax,.r2
        sub     eax,.r1
        imul    ebx
        sar     eax,15
        push    eax

        mov     eax,.tex_x2
        sub     eax,.tex_x1
        imul    ebx
        sar     eax,15
        push    eax

        mov     eax,.tex_y2
        sub     eax,.tex_y1
        imul    ebx
        sar     eax,15
        push    eax


end if
if 0
       sub       esp,6*4
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
       movlhps   xmm0,xmm0
       movdqu    xmm1,.tex_y1
       movdqu    xmm2,.tex_y2
       movq      xmm3,.b1
       movq      xmm4,.b2
       psubd     xmm4,xmm3
       psubd     xmm2,xmm1
       packssdw  xmm2,xmm4
  ;     packlssdw  xmm2,xmm2
   ;    movlhps   xmm2,xmm4


   ;    psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       movhlps   xmm4,xmm2
       movhlps   xmm5,xmm3
       punpcklwd xmm2,xmm3
       punpcklwd xmm4,xmm5
       psrad     xmm2,15 - ROUND
       psrad     xmm4,15 - ROUND
   ;    pshufd    xmm2,xmm2,11000110b
       movdqu    .dtex_y,xmm2
       movq      .db,xmm4


end if

        cmp     .x1,0
        jg      @f

        mov     eax,.dz     ; clipping
        movsx   ebx,.x1
        neg     ebx
        imul    ebx
        add     .z1,eax
        mov     .x1,0

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
        movsx     edx,word[size_x_var]  ;SIZE_X
        cmp       .x2,dx
        jl        @f
        mov       .x2,dx
    @@:
; calc line addres begin in screen and Z buffer
        movsx     eax,word .y
        mul       edx
        movsx     edx,.x1
        add       eax,edx

        mov       esi,eax
        shl       esi,2
        add       esi,.z_buffer

        lea       eax,[eax*3]
        mov       edi,.screen
        add       edi,eax

        mov       cx,.x2
        sub       cx,.x1
        movzx     ecx,cx

; init current variables
        movdqu    xmm0,.r1
        movdqu    xmm1,.dr
        pxor      xmm2,xmm2
        movq      xmm4,.dtex_y
        movq      xmm5,.tex_y1

        mov       ebx,.z1
      .ddraw:
        cmp       ebx,dword[esi]
        jge       @f
        movdqa    xmm6,xmm5
        psrld     xmm6,ROUND
        movd      eax,xmm6
        psrldq    xmm6,4
        movd      edx,xmm6
        shl       eax,TEX_SHIFT

   ; calc texture pixel mem addres

       add       eax,edx
       and       eax,TEXTURE_SIZE   ; cutting
       lea       eax,[3*eax]
       add       eax,.tex_ptr
       mov       dword[esi],ebx

       movd      xmm7,[eax]
       punpcklbw xmm7,xmm2
       movdqa    xmm3,xmm0   ; calc col
       psrld     xmm3,ROUND  ;
       packssdw  xmm3,xmm3
       pmullw    xmm7,xmm3
       psrlw     xmm7,8
       packuswb  xmm7,xmm7
       movd      [edi],xmm7

       mov        dword[esi],ebx
     @@:
       add        edi,3
       add        esi,4
       add        ebx,.dz
       paddd      xmm5,xmm4
       paddd      xmm0,xmm1

       loop     .ddraw

  .quit_l:

        mov     esp,ebp
ret 42+20  ; horizontal line
;the_zero:
;size_y_var:
;size_x_var:

