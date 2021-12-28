; Real Phong's shading implemented if flat assembler
; by Maciej Guba.
; http://macgub.co.pl

ROUND2 equ 10
real_phong_tri_z:
;----procedure render Phongs shaded triangle with z coord
;----interpolation ( Catmull alghoritm )-----------------
;----I normalize normal vector in every pixel -----------
;------------------in - eax - x1 shl 16 + y1 ------------
;---------------------- ebx - x2 shl 16 + y2 ------------
;---------------------- ecx - x3 shl 16 + y3 ------------
;---------------------- esi - pointer to Z-buffer filled-
;----------------------   with dd float variables--------
;---------------------- edi - pointer to screen buffer---
;---------------------- xmm0 - 1st normal vector --------
;---------------------- xmm1 - 2cond normal vector ------
;---------------------- xmm2 - 3rd normal vector --------
;---------------------- xmm3 - normalized light vector --
;---------------------- xmm4 - lo -> hi z1, z2, z3 coords
;----------------------  as dwords floats ---------------
;---------------------- xmm5 - lo -> hi y_min, y_max, ---
;----------------------  x_min, x_max as dword integers -
;---------------------- stack - no parameters -----------
;--------------------------------------------------------
;----------------- procedure don't save registers !! ----




  push  ebp
  mov   ebp,esp
  sub   esp,512
  sub   ebp,16
  and   ebp,0xfffffff0

  .1_nv equ [ebp-16]
  .2_nv equ [ebp-32]
  .3_nv equ [ebp-48]
  .l_v  equ [ebp-64]
  .z3   equ [ebp-72]
  .z2   equ [ebp-76]
  .z1   equ [ebp-80]
  .x1   equ [ebp-82]
  .y1   equ [ebp-84]
  .x2   equ [ebp-86]
  .y2   equ [ebp-88]
  .x3   equ [ebp-90]
  .y3   equ [ebp-92]
  .Zbuf equ [ebp-96]
  .x_max equ  [ebp-100]
  .x_min equ  [ebp-104]
  .y_max equ  [ebp-108]
  .y_min equ  [ebp-112]
  .screen equ [ebp-116]
  .dx12   equ [ebp-120]
  .dx13   equ [ebp-124]
  .dx23   equ [ebp-128]
  .dn12   equ [ebp-144]
  .dn13   equ [ebp-160]
  .dn23   equ [ebp-176]
  .dz12   equ [ebp-180]
  .dz13   equ [ebp-184]
  .dz23   equ [ebp-188]

  .cnv1   equ [ebp-208]  ; cur normal vectors
  .cnv2   equ [ebp-224]
  .cz2    equ [ebp-228]
  .cz1    equ [ebp-232]





 .sort3:                  ; sort triangle coordinates...
       cmp     ax,bx
       jle     .sort1
       xchg    eax,ebx
       shufps  xmm4,xmm4,11100001b
       movaps  xmm6,xmm0
       movaps  xmm0,xmm1
       movaps  xmm1,xmm6


 .sort1:
       cmp      bx,cx
       jle      .sort2
       xchg     ebx,ecx
       shufps   xmm4,xmm4,11011000b
       movaps   xmm6,xmm1
       movaps   xmm1,xmm2
       movaps   xmm2,xmm6

       jmp .sort3

 .sort2:

   movaps .z1,xmm4
   mov    .y1,eax
   mov    .y2,ebx
   mov    .y3,ecx

   movdqa   .y_min,xmm5
if 1                            ; check if at last only fragment
   packssdw xmm5,xmm5       ; of triangle is in visable area
   pshuflw  xmm5,xmm5,11011000b
   movdqu   xmm7,.y3
   movdqa   xmm6,xmm5
   pshufd   xmm5,xmm5,0  ; xmm5 lo-hi -> broadcasted y_min, x_min
   pshufd   xmm6,xmm6,01010101b ;xmm6 -> brd y_max x_max
   movdqa   xmm4,xmm7
   pcmpgtw  xmm7,xmm5
   pcmpgtw  xmm4,xmm6
   pxor     xmm7,xmm4
   pmovmskb eax,xmm7
   and      eax,0x00aaaaaa
   or       eax,eax
   jz       .rpt_loop2_end
end if
   movaps   .1_nv,xmm0
   movaps   .2_nv,xmm1
   movaps   .3_nv,xmm2
   movaps   .l_v,xmm3
   mov      .Zbuf,esi
   mov      .screen,edi



       mov      bx,.y2       ; calc deltas
       sub      bx,.y1
       jnz      .rpt_dx12_make

       xorps    xmm7,xmm7
       mov      dword .dx12,0
       mov      dword .dz12,0
       movaps   .dn12,xmm7
       jmp      .rpt_dx12_done

  .rpt_dx12_make:
       mov      ax,.x2
       sub      ax,.x1
       cwde
       movsx    ebx,bx
       shl      eax,ROUND2
       cdq
       idiv     ebx
       mov      .dx12,eax

       cvtsi2ss xmm6,ebx
       movss    xmm5,.z2
       subss    xmm5,.z1
       divss    xmm5,xmm6
       movss    .dz12,xmm5

       movaps   xmm0,.2_nv
       subps    xmm0,.1_nv
       shufps   xmm6,xmm6,0
       divps    xmm0,xmm6
       movaps   .dn12,xmm0


   .rpt_dx12_done:

       mov      bx,.y3       ; calc deltas
       sub      bx,.y1
       jnz      .rpt_dx13_make

       xorps    xmm7,xmm7
       mov      dword .dx13,0
       mov      dword .dz13,0
       movaps   .dn13,xmm7
       jmp      .rpt_dx13_done

  .rpt_dx13_make:
       mov      ax,.x3
       sub      ax,.x1
       cwde
       movsx    ebx,bx
       shl      eax,ROUND2
       cdq
       idiv     ebx
       mov      .dx13,eax

       cvtsi2ss xmm6,ebx
       movss    xmm5,.z3
       subss    xmm5,.z1
       divss    xmm5,xmm6
       movss    .dz13,xmm5

       movaps   xmm0,.3_nv
       subps    xmm0,.1_nv
       shufps   xmm6,xmm6,0
       divps    xmm0,xmm6
       movaps   .dn13,xmm0

   .rpt_dx13_done:

       mov      bx,.y3       ; calc deltas
       sub      bx,.y2
       jnz      .rpt_dx23_make

       xorps    xmm7,xmm7
       mov      dword .dx23,0
       mov      dword .dz23,0
       movaps   .dn23,xmm7
       jmp      .rpt_dx23_done

  .rpt_dx23_make:
       mov      ax,.x3
       sub      ax,.x2
       cwde
       movsx    ebx,bx
       shl      eax,ROUND2
       cdq
       idiv     ebx
       mov      .dx23,eax

       cvtsi2ss xmm6,ebx
       movss    xmm5,.z3
       subss    xmm5,.z2
       divss    xmm5,xmm6
       movss    .dz23,xmm5

       movaps   xmm0,.3_nv
       subps    xmm0,.2_nv
       shufps   xmm6,xmm6,0
       divps    xmm0,xmm6
       movaps   .dn23,xmm0

   .rpt_dx23_done:


       movsx   eax,word .x1
       shl     eax,ROUND2
       mov     ebx,eax
       mov     edx,.z1
       mov     .cz1,edx
       mov     .cz2,edx
       movaps  xmm0,.1_nv
       movaps  .cnv1,xmm0
       movaps  .cnv2,xmm0


       movsx    ecx,word .y1
       cmp      cx,.y2

       jge      .rpt_loop1_end

    .rpt_loop1:
       pushad

       movaps   xmm2,.y_min
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
       movlps   xmm3,.cz1
       movaps   xmm4,.l_v
       sar      ebx,ROUND2
       sar      eax,ROUND2
       mov      edi,.screen
       mov      esi,.Zbuf

       call     real_phong_line_z

       popad
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
       movss    xmm2,.cz1
       movss    xmm3,.cz2
       addps    xmm0,.dn13
       addps    xmm1,.dn12
       addss    xmm2,.dz13
       addss    xmm3,.dz12
       add      eax,.dx13
       add      ebx,.dx12

       movaps   .cnv1,xmm0
       movaps   .cnv2,xmm1
       movss    .cz1,xmm2
       movss    .cz2,xmm3

       add      ecx,1
       cmp      cx,.y2
       jl       .rpt_loop1





   .rpt_loop1_end:
       movsx    ecx,word .y2
       cmp      cx,.y3
       jge      .rpt_loop2_end

       movsx    ebx,word .x2                    ; eax - cur x1
       shl      ebx,ROUND2                 ; ebx - cur x2
       push     dword .z2
       pop      dword .cz2
       movaps   xmm0,.2_nv
       movaps   .cnv2,xmm0


     .rpt_loop2:
       pushad

       movaps   xmm2,.y_min
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
       movlps   xmm3,.cz1
       movaps   xmm4,.l_v
       sar      ebx,ROUND2
       sar      eax,ROUND2
       mov      edi,.screen
       mov      esi,.Zbuf

       call     real_phong_line_z

       popad
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
       movss    xmm2,.cz1
       movss    xmm3,.cz2
       addps    xmm0,.dn13
       addps    xmm1,.dn23
       addss    xmm2,.dz13
       addss    xmm3,.dz23
       add      eax,.dx13
       add      ebx,.dx23

       movaps   .cnv1,xmm0
       movaps   .cnv2,xmm1
       movss    .cz1,xmm2
       movss    .cz2,xmm3

       add      ecx,1
       cmp      cx,.y3
       jl       .rpt_loop2

    .rpt_loop2_end:

      add   esp,512
      pop   ebp

ret
real_phong_line_z:
; in:
;    xmm0 - normal vector 1
;    xmm1 - normal vect 2
;    xmm3 - lo -> hi z1, z2 coords as dwords floats
;    xmm2 - lo -> hi y_min, y_max, x_min, x_max
;           as dword integers
;    xmm4 - normalized light vector
;    eax - x1
;    ebx - x2
;    ecx - y
;    edi - screen buffer
;    esi - z buffer filled with dd floats

   push  ebp
   mov   ebp,esp
   sub   esp,160
   sub   ebp,16
   and   ebp,0xfffffff0

 .n1 equ [ebp-16]
 .n2 equ [ebp-32]
 .lv equ [ebp-48]
 .lx1 equ [ebp-52]
 .lx2 equ [ebp-56]
 .z2 equ [ebp-60]
 .z1 equ [ebp-64]
 .screen equ [ebp-68]
 .zbuff  equ [ebp-72]
 .x_max  equ [ebp-74]
 .x_min  equ [ebp-76]
 .y_max  equ [ebp-78]
 .y_min  equ [ebp-80]
 .dn     equ [ebp-96]
 .dz     equ [ebp-100]
 .y      equ [ebp-104]
 .cnv    equ [ebp-128]

        mov    .y,ecx
        packssdw xmm2,xmm2
        movq   .y_min,xmm2
        cmp    cx,.y_min
        jl     .end_rp_line
        cmp    cx,.y_max
        jge    .end_rp_line          ;

        cmp     eax,ebx
        je      .end_rp_line
        jl      @f
        xchg    eax,ebx
        movaps  xmm7,xmm0
        movaps  xmm0,xmm1
        movaps  xmm1,xmm7
        shufps  xmm3,xmm3,11100001b
   @@:

        cmp     ax,.x_max
        jge     .end_rp_line
        cmp     bx,.x_min
        jle     .end_rp_line
        movaps  .lv,xmm4
        movaps  .n1,xmm0
        movaps  .n2,xmm1
        mov     .lx1,eax
        mov     .lx2,ebx
        movlps  .z1,xmm3

        sub     ebx,eax
        cvtsi2ss xmm7,ebx
        shufps  xmm7,xmm7,0
        subps   xmm1,xmm0
        divps   xmm1,xmm7
        movaps  .dn,xmm1
        psrldq  xmm3,4
        subss   xmm3,.z1
        divss   xmm3,xmm7
        movss   .dz,xmm3



        mov      ebx,.lx1
        cmp      bx,.x_min     ; clipping on function4
        jge      @f
        movzx    eax,word .x_min
        sub      eax,ebx
        cvtsi2ss xmm7,eax
        shufps   xmm7,xmm7,0
        mulss    xmm3,xmm7
        mulps    xmm1,xmm7
        addss    xmm3,.z1
        addps    xmm1,.n1
        movsx    eax,word .x_min
        movss    .z1,xmm3
        movaps   .n1,xmm1
        mov      dword .lx1,eax

      @@:
        movzx   eax,word .x_max
        cmp     .lx2,eax
        jl      @f
        mov     .lx2,eax
      @@:
        movzx   eax,word[size_x_var]
        mul     dword .y
      ;  mov     edx,.x1
        add     eax,.lx1
        shl     eax,2
        add     edi,eax
        add     esi,eax

        mov     ecx,.lx2
        sub     ecx,.lx1
        movaps  xmm0,.n1
        movss   xmm2,.z1

   .ddraw:
        movss    xmm7,xmm2
        cmpnltss xmm7,dword[esi]
        movd     eax,xmm7
        or       eax,eax
        jnz      .skip
        movss    [esi],xmm2
        movaps   xmm7,xmm0
        mulps    xmm7,xmm7 ; normalize
        haddps   xmm7,xmm7
        haddps   xmm7,xmm7
        rsqrtps  xmm7,xmm7
        mulps    xmm7,xmm0
        movaps   .cnv,xmm7

        mov      edx,lights_aligned    ; lights - global variable
        xorps    xmm1,xmm1     ; instead global can be used .lv - light vect.
      @@:
        movaps    xmm6,[edx+16]
        movaps    xmm5,[edx]
        movaps    xmm3,[edx+48]
        andps     xmm5,[zero_hgst_dd]  ; global

        mulps    xmm5,.cnv  ;.lv  ; last dword should be zeroed
        haddps   xmm5,xmm5
        haddps   xmm5,xmm5
   ;     mulps    xmm5,[env_const2]
   ;     maxps    xmm5,[dot_min]
   ;     minps    xmm5,[dot_max]
        movaps   xmm7,xmm5
      ;  mulps    xmm7,[env_const2]
     ;   mulps    xmm7,[env_const2]
     ;   maxps    xmm7,[dot_min]
     ;   minps    xmm7,[dot_max]

        mulps    xmm7,xmm7
        mulps    xmm7,xmm7
        mulps    xmm5,xmm6
        mulps    xmm7,xmm7
        mulps    xmm7,xmm3

        addps    xmm5,xmm7
        minps    xmm5,[mask_255f]   ; global
        maxps    xmm1,xmm5
     ;   movq     xmm3,[edx+20]    ; minimal color
     ;   punpcklwd xmm3,[minimum0]
     ;   cvtdq2ps xmm3,xmm3
     ;   maxps    xmm1,xmm3
        add      edx,64
        cmp      edx,lights_aligned_end    ; global
        jnz      @b

        cvtps2dq xmm1,xmm1
        packssdw xmm1,xmm1
        packuswb xmm1,xmm1
        movd     [edi],xmm1
     .skip:
        add      edi,4
        add      esi,4
        addps    xmm0,.dn
        addss    xmm2,.dz
        sub      ecx,1
        jnz      .ddraw

  .end_rp_line:
        add      esp,160
        pop      ebp

ret
