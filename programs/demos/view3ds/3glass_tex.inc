; Bilinear filtering, real Phongs shading and glass like parallel.
; Thanks to authors of 3dica tutorial.
; Implemented in FASM by Maciej Guba.
; http://macgub.co.pl

ROUND2 equ 10

glass_tex_tri:
;----Procedure render Phongs shaded triangle with z coord
;----interpolation ( Catmull alghoritm ), each pixel is -
;----covered by texture using bilinear filtering.--------
;----I normalize normal vector in every pixel -----------
;------------------in - eax - x1 shl 16 + y1 ------------
;---------------------- ebx - x2 shl 16 + y2 ------------
;---------------------- ecx - x3 shl 16 + y3 ------------
;---------------------- esi - pointer to stencil buffer--
;----------------------   filled with dd float variables-
;---------------------- edi - pointer to screen buffer---
;---------------------- edx - pointer to texture---------
;---------------------- xmm0 - 1st normal vector --------
;---------------------- xmm1 - 2cond normal vector ------
;---------------------- xmm2 - 3rd normal vector --------
;---------------------- xmm3 - normalized light vector --
;---------------------- xmm4 - lo -> hi z1, z2, z3 coords
;----------------------  as dwords floats ---------------
;---------------------- xmm5 - lo -> hi y_min, y_max, ---
;----------------------  x_min, x_max as dword integers -
;---------------------- xmm6 - lo -> hi tx1, ty1, tx2, --
;----------------------  ty2, tx3, ty3 as word, xres as--
;----------------------  dword integers------------------
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

  .cnv1   equ [ebp-192]  ; cur normal vectors
  .cnv2   equ [ebp-208]
  .x_res  equ [ebp-212]
  .ty3    equ [ebp-214]
  .tx3    equ [ebp-216]
  .ty2    equ [ebp-218]
  .tx2    equ [ebp-220]
  .ty1    equ [ebp-222]
  .tx1    equ [ebp-224]
  .dz12   equ [ebp-232]
  .dty12  equ [ebp-236]
  .dtx12  equ [ebp-240]
  .dz13   equ [ebp-248]
  .dty13  equ [ebp-252]
  .dtx13  equ [ebp-256]
  .dz23   equ [ebp-264]
  .dty23  equ [ebp-268]
  .dtx23  equ [ebp-272]
  .cz1    equ [ebp-280]
  .cty1   equ [ebp-284]
  .ctx1   equ [ebp-288]
  .cz2    equ [ebp-296]
  .cty2   equ [ebp-300]
  .ctx2   equ [ebp-304]
  .tx_ptr equ [ebp-308]


      emms
    ;  movd    .x_res,xmm7
 .sort3:                  ; sort triangle coordinates...
       cmp     ax,bx
       jle     .sort1
       xchg    eax,ebx
       shufps  xmm4,xmm4,11100001b
       shufps  xmm6,xmm6,11100001b
       movaps  xmm7,xmm0
       movaps  xmm0,xmm1
       movaps  xmm1,xmm7


 .sort1:
       cmp      bx,cx
       jle      .sort2
       xchg     ebx,ecx
       shufps   xmm4,xmm4,11011000b
       shufps   xmm6,xmm6,11011000b
       movaps   xmm7,xmm1
       movaps   xmm1,xmm2
       movaps   xmm2,xmm7

       jmp .sort3

 .sort2:
 ;  movq    .tx1,xmm6
 ;  pshufd  xmm6,xmm6,01001110b
 ;  movd    .tx3,xmm6
   movaps .tx1,xmm6
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
   mov      .tx_ptr,edx



       mov      bx,.y2       ; calc deltas
       sub      bx,.y1
       jnz      .rpt_dx12_make

       xorps    xmm7,xmm7
       mov      dword .dx12,0
       movaps   .dtx12,xmm7
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
       shufps   xmm6,xmm6,0
       movss    xmm5,.z2
       subss    xmm5,.z1
       divss    xmm5,xmm6
       movss    .dz12,xmm5

       movd     xmm0,.tx1
       movd     xmm2,.tx2
       pxor     xmm1,xmm1
       punpcklwd xmm0,xmm1
       punpcklwd xmm2,xmm1
       psubd     xmm2,xmm0
     ;  cvtdq2ps xmm0,xmm0
       cvtdq2ps xmm2,xmm2
;       movlps   .ctx1,xmm0
;       movlps   .ctx2,xmm2
     ;  subps    xmm2,xmm0
       divps    xmm2,xmm6
       movlps   .dtx12,xmm2

       movaps   xmm0,.2_nv
       subps    xmm0,.1_nv
       divps    xmm0,xmm6
       movaps   .dn12,xmm0


   .rpt_dx12_done:

       mov      bx,.y3       ; calc deltas
       sub      bx,.y1
       jnz      .rpt_dx13_make

       xorps    xmm7,xmm7
       mov      dword .dx13,0
       movaps   .dtx13,xmm7
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
       shufps   xmm6,xmm6,0

       movss    xmm5,.z3
       subss    xmm5,.z1
       divss    xmm5,xmm6
       movss    .dz13,xmm5

       movd     xmm0,.tx1
       movd     xmm2,.tx3
       pxor     xmm1,xmm1
       punpcklwd xmm0,xmm1
       punpcklwd xmm2,xmm1
       psubd    xmm2,xmm0
     ;  cvtdq2ps xmm0,xmm0
       cvtdq2ps xmm2,xmm2
     ;  subps    xmm2,xmm0
       divps    xmm2,xmm6
       movlps   .dtx13,xmm2



       movaps   xmm0,.3_nv
       subps    xmm0,.1_nv
       divps    xmm0,xmm6
       movaps   .dn13,xmm0

   .rpt_dx13_done:

       mov      bx,.y3       ; calc deltas
       sub      bx,.y2
       jnz      .rpt_dx23_make

       xorps    xmm7,xmm7
       mov      dword .dx23,0
       movaps   .dtx23,xmm7
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
       shufps   xmm6,xmm6,0
       movss    xmm5,.z3
       subss    xmm5,.z2
       divss    xmm5,xmm6
       movss    .dz23,xmm5

       movd     xmm0,.tx2
       movd     xmm2,.tx3
       pxor     xmm1,xmm1
       punpcklwd xmm0,xmm1
       punpcklwd xmm2,xmm1
       psubd     xmm2,xmm0
      ; cvtdq2ps xmm0,xmm0
       cvtdq2ps xmm2,xmm2
;       movlps   .ctx1,xmm0
;       movlps   .ctx2,xmm2
    ;   subps    xmm2,xmm0
       divps    xmm2,xmm6
       movlps   .dtx23,xmm2




       movaps   xmm0,.3_nv
       subps    xmm0,.2_nv
       divps    xmm0,xmm6
       movaps   .dn23,xmm0

   .rpt_dx23_done:

       movsx   eax,word .x1
       shl     eax,ROUND2
       mov     ebx,eax
       mov     edx,.z1
       movd    xmm1,.tx1
       pxor    xmm2,xmm2
       punpcklwd xmm1,xmm2
       cvtdq2ps  xmm1,xmm1

       mov     .cz1,edx
       mov     .cz2,edx
       movaps  xmm0,.1_nv
       movlps  .ctx1,xmm1
       movlps  .ctx2,xmm1
       movaps  .cnv1,xmm0
       movaps  .cnv2,xmm0

     ;  mov     edx,.dx13
     ;  cmp     edx,.dx12
     ;  jg      .second_cause

       movsx    ecx,word .y1
       cmp      cx,.y2

       jge      .rpt_loop1_end

    .rpt_loop1:
       pushad

       movaps   xmm2,.y_min
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
    ;   movlps   xmm3,.cz1 ; cz1, cz2 both
       movaps   xmm3,.ctx1
       movaps   xmm5,.ctx2
       movaps   xmm4,.l_v
       movd     xmm6,.x_res
       sar      ebx,ROUND2
       sar      eax,ROUND2
       mov      edx,.tx_ptr
       mov      edi,.screen

       mov      esi,.Zbuf

       call     glass_tex_line

       popad
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
     ;  movss    xmm2,.cz1
     ;  movss    xmm3,.cz2
       movaps   xmm2,.ctx1
       movaps   xmm3,.ctx2
       addps    xmm0,.dn13
       addps    xmm1,.dn12
       addps    xmm2,.dtx13
       addps    xmm3,.dtx12
       add      eax,.dx13
       add      ebx,.dx12

       movaps   .cnv1,xmm0
       movaps   .cnv2,xmm1
     ;  movss    .cz1,xmm2
     ;  movss    .cz2,xmm3
       movaps   .ctx1,xmm2
       movaps   .ctx2,xmm3
       add      ecx,1
       cmp      cx,.y2
       jl       .rpt_loop1


   ;    jmp  .rpt_loop2_end


   .rpt_loop1_end:
       movsx    ecx,word .y2
       cmp      cx,.y3
       jge      .rpt_loop2_end

       movsx    ebx,word .x2                    ; eax - cur x1
       shl      ebx,ROUND2                 ; ebx - cur x2
       push     dword .z2
       pop      dword .cz2
       movd     xmm1,.tx2
       pxor     xmm2,xmm2
       punpcklwd xmm1,xmm2
       cvtdq2ps xmm1,xmm1
       movlps   .ctx2,xmm1
       movaps   xmm0,.2_nv
       movaps   .cnv2,xmm0


     .rpt_loop2:
       pushad

       movaps   xmm2,.y_min
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
       movaps   xmm3,.ctx1
       movaps   xmm5,.ctx2
       movaps   xmm4,.l_v
       sar      ebx,ROUND2
       sar      eax,ROUND2
       mov      edx,.tx_ptr
       mov      edi,.screen
       mov      esi,.Zbuf
       movd     xmm6,.x_res
       call     glass_tex_line

       popad
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
   ;    movss    xmm2,.cz1
   ;    movss    xmm3,.cz2
       movaps   xmm2,.ctx1
       movaps   xmm3,.ctx2
       addps    xmm0,.dn13
       addps    xmm1,.dn23
    ;   addss    xmm2,.dz13
    ;   addss    xmm3,.dz23
       addps    xmm2,.dtx13
       addps    xmm3,.dtx23

       add      eax,.dx13
       add      ebx,.dx23

       movaps   .cnv1,xmm0
       movaps   .cnv2,xmm1
       movaps   .ctx1,xmm2
       movaps   .ctx2,xmm3

     ;  movss    .cz1,xmm2
     ;  movss    .cz2,xmm3

       add      ecx,1
       cmp      cx,.y3
       jl       .rpt_loop2

    .second_cause:  ;dx13 > dx12

    .rpt_loop2_end:

      add   esp,512
      pop   ebp

ret

glass_tex_line:
; in:
;    xmm0 - normal vector 1
;    xmm1 - normal vect 2
;    xmm3 - lo -> hi tx1, ty1, z1 coords as dwords float
;    xmm5 - lo -> hi tx2, ty2, z2 coords as dwords float
;    xmm2 - lo -> hi y_min, y_max, x_min, x_max
;           as dword integers
;    xmm4 - normalized light vector
;    eax - x1
;    ebx - x2
;    ecx - y
;    edi - screen buffer
;    esi - stencil buffer filled with dd floats
;    edx - texture pointer (handle)
;    xmm6 - lowest dword x_res as integer

   push  ebp
   mov   ebp,esp
   sub   esp,350
   sub   ebp,16
   and   ebp,0xfffffff0

 .n1 equ [ebp-16]
 .n2 equ [ebp-32]
 .lv equ [ebp-48]
 .lx1 equ [ebp-52]
 .lx2 equ [ebp-56]
; .z2 equ [ebp-60]
; .z1 equ [ebp-64]
 .screen equ [ebp-68]
 .zbuff  equ [ebp-72]
 .x_max  equ [ebp-74]
 .x_min  equ [ebp-76]
 .y_max  equ [ebp-78]
 .y_min  equ [ebp-80]
 .dn     equ [ebp-96]
 .x_res  equ [ebp-100]
 .y      equ [ebp-104]
 .cnv    equ [ebp-128]
 .z1     equ [ebp-136]
 .ty1    equ [ebp-140]
 .tx1    equ [ebp-144]
 .z2     equ [ebp-152]
 .ty2    equ [ebp-156]
 .tx2    equ [ebp-160]
 .cz     equ [ebp-168]
 .cty    equ [ebp-172]
 .ctx    equ [ebp-176]
 .dz     equ [ebp-184]
 .dty    equ [ebp-188]
 .dtx    equ [ebp-192]
 .yd     equ [ebp-196]
 .xd     equ [ebp-200]
 .yf     equ [ebp-204]
 .xf     equ [ebp-208]
 .w4     equ [ebp-212]
 .w3     equ [ebp-216]
 .w2     equ [ebp-220]
 .w1     equ [ebp-224]
 .p4     equ [ebp-228]
 .p3     equ [ebp-232]
 .p2     equ [ebp-236]
 .p1     equ [ebp-240]


 .tx_ptr equ [ebp-244]

  ;      movaps  xmm7,xmm3
  ;      movaps  xmm3,xmm5
  ;      movaps  xmm5,xmm7


        mov    .y,ecx
        packssdw xmm2,xmm2
    ;    movaps xmm7,xmm2
    ;    movhps xmm2,[the_zero]
    ;    pshuflw xmm2,xmm2,11111000b
    ;    pshufd  xmm2,xmm2,11111100b
   ;     movlps  xmm7,[the_zero]
    ;    pshufhw xmm7,xmm7,11111111b
     ;   movlps  xmm7,[the_zero]
     ;   psrldq  xmm7,4
     ;   por     xmm2,xmm7
        movq   .y_min,xmm2
        cmp    cx,.y_min
        jl     .end_line
        cmp    cx,.y_max
        jge    .end_line          ;

        cmp     eax,ebx
        je      .end_line
        jl      @f
        xchg    eax,ebx
        movaps  xmm7,xmm0
        movaps  xmm0,xmm1
        movaps  xmm1,xmm7
        movaps  xmm7,xmm3
        movaps  xmm3,xmm5
        movaps  xmm5,xmm7
   @@:

        cmp     ax,.x_max
        jge     .end_line
        cmp     bx,.x_min
        jle     .end_line
        movaps  .lv,xmm4
        movaps  .n1,xmm0
        movaps  .n2,xmm1
        mov     .lx1,eax
        mov     .lx2,ebx
        movaps  .tx1,xmm3
        movaps  .tx2,xmm5
        movd    .x_res,xmm6
        mov     .tx_ptr,edx
        sub     ebx,eax
        cvtsi2ss xmm7,ebx
        shufps  xmm7,xmm7,0
        subps   xmm1,xmm0
        divps   xmm1,xmm7
        movaps  .dn,xmm1
        subps   xmm5,xmm3
        divps   xmm5,xmm7
        movaps  .dtx,xmm5



        mov      ebx,.lx1
        cmp      bx,.x_min     ; clipping on function4
        jge      @f
        movzx    eax,word .x_min
        sub      eax,ebx
        cvtsi2ss xmm7,eax
        shufps   xmm7,xmm7,0
        mulps    xmm5,xmm7
        mulps    xmm1,xmm7
        addps    xmm5,.tx1
        addps    xmm1,.n1
        movsx    eax,word .x_min
        movaps   .tx1,xmm5
        movaps   .n1,xmm1
        mov      dword .lx1,eax

      @@:
        movzx   eax,word .x_max
        cmp     .lx2,eax
        jl      @f
        mov     .lx2,eax
      @@:
        mov     eax,.x_res
        mul     dword .y
        add     eax,.lx1
        shl     eax,2
        add     edi,eax
        add     esi,eax

        mov     ecx,.lx2
        sub     ecx,.lx1
   ;     movaps  xmm0,.n1
        movaps  xmm2,.tx1
      ;  xorps   xmm1,xmm1
align 16
   .ddraw:
     ;   movhlps  xmm7,xmm2
     ;   cmpnltss xmm7,dword[esi]
     ;   movd     eax,xmm7
     ;   or       eax,eax
     ;   jnz      .skip
        xorps    xmm5,xmm5
   ;     movhlps  xmm7,xmm2
   ;     movss    [esi],xmm7
        movaps   xmm7,.n1  ;xmm0
        mulps    xmm7,xmm7 ; normalize
        haddps   xmm7,xmm7
        haddps   xmm7,xmm7
        rsqrtps  xmm7,xmm7
        mulps    xmm7,.n1   ;xmm0
 ;       andps    xmm7,[abs_z_coof]
        movaps   .cnv,xmm7

        movaps   xmm6,xmm2
        minps    xmm6,[tex_m2]  ;    float  TEX_X-2,TEX_Y-2
        cvttps2dq xmm7,xmm6
        cvtdq2ps xmm4,xmm7
        subps    xmm6,xmm4
        movlps   .xf,xmm6
     ;    movaps   xmm5,.lv
        mov      eax,lights_aligned   ; global
     align 16
      .again_col:
        movaps   xmm0,[eax] ; calc multple lights
        mulps    xmm0,.cnv  ;.lv  ; last dword should be zeroed
        haddps   xmm0,xmm0
        haddps   xmm0,xmm0
   ;     andps    xmm0,[abs_val]  ;calc absolute value
if 1
        ; stencil
        movhlps   xmm6,xmm2
        movhlps   xmm4,xmm2
        addss     xmm6,[aprox]
        subss     xmm4,[aprox]
        cmpnltss  xmm6,dword[esi]
        cmpnltss  xmm4,dword[esi]
        xorps     xmm6,xmm4
        xorps     xmm4,xmm4
        movd      ebx,xmm6
        cmp       ebx,-1
        jne       .no_reflective
end if
        movaps    xmm4,xmm0
        mulps     xmm4,xmm4
        mulps     xmm4,xmm4
        mulps     xmm4,xmm4
        mulps     xmm4,xmm4
        mulps     xmm4,[eax+48]

     .no_reflective:
        maxps    xmm0,[the_zero]
 ;       movaps   xmm1,xmm0
        mulps    xmm0,[eax+16]
        addps    xmm4,xmm0
        addps    xmm4,[eax+32]
        maxps    xmm5,xmm4
        add      eax,64
        cmp      eax,lights_aligned_end
        jnz      .again_col
        minps    xmm5,[mask_255f]

          ; texture coords work
        movd     eax,xmm7
        psrldq   xmm7,4
        movd     ebx,xmm7
        shl      ebx,TEX_SHIFT
        add      eax,ebx
        lea      eax,[eax*3]
        add      eax,.tx_ptr
        mov      ebx,eax
        add      ebx,TEX_X*3
        movd     xmm7,[eax]
        movd     xmm6,[eax+3]
        movd     xmm4,[ebx]
        movd     xmm3,[ebx+3]
        punpcklbw xmm7,xmm6 ;xmm7 r1 r2 g1 g2 b1 b2
        punpcklbw xmm4,xmm3 ;xmm4 r3 r4 g3 g4 b3 b4
        punpcklwd xmm7,xmm4 ;xmm7 r1 r2 r3 r4 g1 g2 g3 g4 ...
        movdqa    xmm6,xmm7
        movdqa    xmm4,xmm7
        psrldq    xmm6,4
        psrldq    xmm4,8

        punpcklbw xmm7,[the_zero]  ; broadcasted 0
        punpcklbw xmm6,[the_zero]
        punpcklbw xmm4,[the_zero]
        punpcklwd xmm7,[the_zero]
        punpcklwd xmm6,[the_zero]
        punpcklwd xmm4,[the_zero]


     ; calc w .........
        movlps   xmm3,[the_one]  ;  broadcasted dword 1.0
        cvtdq2ps  xmm7,xmm7
        subps    xmm3,.xf
        cvtdq2ps  xmm6,xmm6
        movhps   xmm3,.xf
        cvtdq2ps  xmm4,xmm4
        movaps   xmm1,xmm3  ; 1-xf, 1-yf, xf, yf
        shufps   xmm3,xmm3,10001000b
        shufps   xmm1,xmm1,11110101b
        mulps    xmm3,xmm1

        mulps    xmm7,xmm3
        mulps    xmm6,xmm3
        mulps    xmm4,xmm3
        haddps   xmm7,xmm7  ; r
        haddps   xmm6,xmm6  ; g
        haddps   xmm4,xmm4  ; b
        haddps   xmm7,xmm7  ; r
        haddps   xmm6,xmm6  ; g
        haddps   xmm4,xmm4  ; b
        movlhps  xmm7,xmm6
        shufps   xmm7,xmm7,11101000b
        movlhps  xmm7,xmm4

        mulps    xmm5,xmm7
        cvtps2dq xmm5,xmm5
        psrld    xmm5,8
        movd     xmm6,[edi]
        packssdw xmm5,xmm5
        packuswb xmm5,xmm5
        paddusb  xmm5,xmm6
        movd     [edi],xmm5
     .skip:
        add      edi,4
        add      esi,4
        movaps   xmm0,.n1     ; cur normal
        addps    xmm0,.dn
        addps    xmm2,.dtx
        movaps   .n1,xmm0
        sub      ecx,1
        jnz      .ddraw

  .end_line:
        add      esp,350
        pop      ebp

ret
