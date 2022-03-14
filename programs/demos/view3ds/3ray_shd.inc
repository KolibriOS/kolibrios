; Ray casted shadows
; by Maciej Guba.
; http://macgub.co.pl


ROUND2 equ 10
ray_shad:
;--- Procedure render triangle with ray casted shadow ---
;--- effect. Calc intersection with all triangles in ----
;--- everypixel. Its not real time process, especially --
;--- when many triangles are computed. ------------------
;------in - eax - x1 shl 16 + y1 ------------------------
;---------- ebx - x2 shl 16 + y2 ------------------------
;---------- ecx - x3 shl 16 + y3 ------------------------
;---------- edx - ptr to fur coords struct --------------
;---------- esi - pointer to stencil / Z-buffer, filled -
;--------------   with dword float variables, it masks --
;--------------   'Z' position (coord) of every front ---
;--------------   pixel. --------------------------------
;---------- edi - pointer to screen buffer --------------
;---------- xmm0 - 1st normal vector --------------------
;---------- xmm1 - 2cond normal vector ------------------
;---------- xmm2 - 3rd normal vector --------------------
;---------- xmm3 - --------------------------------------
;---------- xmm4 - lo -> hi z1, z2, z3 coords -----------
;---------------   as dwords floats ---------------------
;---------- xmm5 - lo -> hi y_min, y_max, x_min, x_max --
;---------------   as dword integers --------------------
;-----------mm7  - current triangle index ---------------
;---------------------- stack - no parameters -----------
;--------------------------------------------------------
;----------------- procedure don't save registers !! ----

  push  ebp
  mov   ebp,esp
  sub   esp,1024
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
  .x_max  equ [ebp-100]
  .x_min  equ [ebp-104]
  .y_max  equ [ebp-108]
  .y_min  equ [ebp-112]
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
  .cnv1   equ [ebp-208] ; current normal vectors
  .cnv2   equ [ebp-240]
  .cz2    equ [ebp-244]
  .cz1    equ [ebp-248]
  .tri_no equ [ebp-252]


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
   movd     .tri_no,mm7
   movaps   .1_nv,xmm0
   movaps   .2_nv,xmm1
   movaps   .3_nv,xmm2
 ;  movaps   .l_v,xmm3
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
       rcpss    xmm6,xmm6
       subss    xmm5,.z1
       mulss    xmm5,xmm6
       movss    .dz12,xmm5

       shufps   xmm6,xmm6,0
       movaps   xmm0,.2_nv
       subps    xmm0,.1_nv
       mulps    xmm0,xmm6
       movaps   .dn12,xmm0
;       subps    xmm3,xmm0
;       mulps    xmm3,xmm6

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
       rcpss    xmm6,xmm6
       subss    xmm5,.z1
       mulss    xmm5,xmm6
       movss    .dz13,xmm5

       movaps   xmm0,.3_nv
       subps    xmm0,.1_nv
       shufps   xmm6,xmm6,0
       mulps    xmm0,xmm6
       movaps   .dn13,xmm0

 ;      mulps    xmm0,xmm6

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
       rcpss    xmm6,xmm6
       subss    xmm5,.z2
       mulss    xmm5,xmm6
       movss    .dz23,xmm5

       movaps   xmm0,.3_nv
       subps    xmm0,.2_nv
       shufps   xmm6,xmm6,0
       mulps    xmm0,xmm6
       movaps   .dn23,xmm0
  ;     mulps    xmm0,xmm6

   .rpt_dx23_done:

       movsx   eax,word .x1
       shl     eax,ROUND2
       mov     ebx,eax
       mov     ecx,.z1
       mov     .cz1,ecx
       mov     .cz2,ecx
       movaps  xmm0,.1_nv
       movaps  .cnv1,xmm0
       movaps  .cnv2,xmm0
       mov      edi,.screen
       mov      esi,.Zbuf
       movsx    ecx,word .y1
       cmp      cx,.y2

       jge      .rpt_loop1_end

    .rpt_loop1:
       pushad

       movaps   xmm2,.y_min
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
       movlps   xmm3,.cz1
    ;   movaps   xmm4,.l_v
       sar      ebx,ROUND2
       sar      eax,ROUND2
       movd     mm7,.tri_no

       call     ray_shd_l

       popad
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
       ; fur x,y
       movss    xmm2,.cz1
       movss    xmm3,.cz2
       shufps   xmm4,xmm4,01001110b
       addps    xmm0,.dn13
       addps    xmm1,.dn12
       addss    xmm2,.dz13
       addss    xmm3,.dz12


       add      eax,.dx13
       add      ebx,.dx12

       shufps   xmm4,xmm4,01001110b
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

       movsx    ebx,word .x2               ; eax - cur x1
       shl      ebx,ROUND2                 ; ebx - cur x2
       push     dword .z2
       pop      dword .cz2
       movaps   xmm0,.2_nv
       movaps   .cnv2,xmm0

       mov      edi,.screen
       mov      esi,.Zbuf


     .rpt_loop2:
       pushad
       movaps   xmm2,.y_min
       movaps   xmm0,.cnv1
       movaps   xmm1,.cnv2
       movlps   xmm3,.cz1
   ;    movaps   xmm4,.l_v
       sar      ebx,ROUND2
       sar      eax,ROUND2
       movd     mm7,.tri_no

       call     ray_shd_l

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
       addps    xmm4,xmm6

       movaps   .cnv1,xmm0
       movaps   .cnv2,xmm1
       movss    .cz1,xmm2
       movss    .cz2,xmm3

       add      ecx,1
       cmp      cx,.y3
       jl       .rpt_loop2

    .rpt_loop2_end:

      add   esp,1024
      pop   ebp



ret

ray_shd_l:
; in:
;    xmm0 - normal vector 1
;    xmm1 - normal vect 2
;    xmm3 - lo -> hi z1, z2 coords as dwords floats
;    xmm2 - lo -> hi y_min, y_max, x_min, x_max
;           as dword integers
;    xmm4 - ----
;    mm7  - current triangle index
;    eax  - x1
;    ebx  - x2
;    ecx  - y
;    edx  - -----
;    edi  - screen buffer
;    esi  - z buffer / stencil buffer filled with dd floats

   push  ebp
   mov   ebp,esp
   sub   esp,270
   sub   ebp,16
   and   ebp,0xfffffff0

 .n1  equ [ebp-16]
 .n2  equ [ebp-32]
 .lv  equ [ebp-48]
 .lx1 equ [ebp-52]
 .lx2 equ [ebp-56]
 .z2  equ [ebp-60]
 .z1  equ [ebp-64]
 .screen    equ [ebp-68]
 .zbuff     equ [ebp-72]
 .x_max     equ [ebp-74]
 .x_min     equ [ebp-76]
 .y_max     equ [ebp-78]
 .y_min     equ [ebp-80]
 .dn        equ [ebp-96]
 .dz        equ [ebp-100]
 .y         equ [ebp-104]
 .startx    equ [ebp-108]
 .cnv       equ [ebp-128]
 .Rlen      equ [ebp-128-16]
 .r1        equ [ebp-128-32]
 .vect_t    equ [ebp-128-48]
 .cur_tri    equ [ebp-128-64]
; .p3t       equ [ebp-128-80]
 .nray      equ [ebp-128-96]
 .final_col equ [ebp-128-112]
 .aabb_mask equ dword[ebp-128-112-4]

        mov     .y,ecx
        movdqa   xmm4,xmm2
        packssdw xmm2,xmm2
        movq    .y_min,xmm2
        cmp     cx,.y_min
        jl      .end_rp_line
        cmp     cx,.y_max
        jge     .end_rp_line          ;
        cmp     eax,ebx
        je      .end_rp_line
        jl      @f
        xchg    eax,ebx
        movaps  xmm7,xmm0
        movaps  xmm0,xmm1
        movaps  xmm1,xmm7
        shufps  xmm3,xmm3,11100001b
    @@:
        movd    .cur_tri,mm7
  ;      sub     .cur_tri,dword 1
        cmp     ax,.x_max
        jge     .end_rp_line
        cmp     bx,.x_min
        jle     .end_rp_line
     ;   movaps  .lv,xmm4
        andps    xmm0,[zero_hgst_dd]
        andps    xmm1,[zero_hgst_dd]
        movaps  .n1,xmm0
        movaps  .n2,xmm1
        mov     .lx1,eax
;        mov     .startx,eax
        mov     .lx2,ebx
        movlps  .z1,xmm3

        sub     ebx,eax
        cvtsi2ss xmm7,ebx
        rcpss   xmm7,xmm7
        shufps  xmm7,xmm7,0
        subps   xmm1,xmm0
        mulps   xmm1,xmm7
        movaps  .dn,xmm1
        shufps  xmm3,xmm3,11111001b
        subss   xmm3,.z1
        mulss   xmm3,xmm7
        movss   .dz,xmm3

        subps   xmm6,xmm5
        mulps   xmm6,xmm7

        mov      ebx,.lx1
        cmp      bx,.x_min     ; clipping on function4
        jge      @f
        movzx    eax,word .x_min
        sub      eax,ebx
        cvtsi2ss xmm7,eax
        shufps   xmm7,xmm7,0
        mulss    xmm3,xmm7
        mulps    xmm1,xmm7
        mulps    xmm6,xmm7
        addss    xmm3,.z1
        addps    xmm1,.n1
        addps    xmm6,xmm5
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
        movzx   eax,word[xres_var]
        mul     dword .y
        add     eax,.lx1
        mov     .zbuff,esi
        mov     .screen,edi
        shl     eax,2
        add     edi,eax
        add     esi,eax
        mov     ecx,.lx2
        sub     ecx,.lx1

        movd       xmm0,[vect_x]
        punpcklwd  xmm0,[the_zero]
        cvtdq2ps   xmm0,xmm0
        movaps     .vect_t,xmm0


   .ddraw:

        xorps    xmm0,xmm0
        movss    xmm2,.z1
        movss    xmm5,.z1
        movaps   .final_col,xmm0
        addss    xmm2,[f1]
        subss    xmm5,[f1]
        cmpnltss xmm2,dword[esi]
        cmpnltss xmm5,dword[esi]
        pxor     xmm2,xmm5
        movd     eax,xmm2
        or       eax,eax
        jz       .skips

        movaps   xmm7,.n1
        andps    xmm7,[zero_hgst_dd]
        mulps    xmm7,xmm7 ; normalize
        haddps   xmm7,xmm7
        haddps   xmm7,xmm7
        rsqrtps  xmm7,xmm7
        mulps    xmm7,.n1
        movaps   .cnv,xmm7
        mov      ebx,point_light_coords
        mov      edx,lights_aligned
        xor      eax,eax
  .nx_light:
   pushad
   cvtsi2ss xmm0,.lx1
   cvtsi2ss xmm1,.y
   movss    xmm2,.z1
   movlhps  xmm0,xmm1
   shufps   xmm0,xmm2,11001000b
   subps    xmm0,[ebx] ; xmm0 - ray end, -> current vertex
   movaps   xmm3,[ebx]
   andps    xmm0,[zero_hgst_dd]
   movaps   xmm1,xmm0
   mulps    xmm0,xmm0
   haddps   xmm0,xmm0
   haddps   xmm0,xmm0
   sqrtps   xmm0,xmm0
   movss    .Rlen,xmm0
   rcpps    xmm0,xmm0
   mulps    xmm0,xmm1    ; xmm0 - normalized ray vector
   andps    xmm0,[zero_hgst_dd]
   movaps   .nray,xmm0
   movaps   .r1,xmm3   ; ray orgin
 if 0
   movaps   xmm1,xmm3
   call     calc_bounding_box

   mov      .aabb_mask,eax
end if
   mov      edi,[triangles_ptr]
   xor      ecx,ecx
 .nx_tri:  ; next triangle
 ;  mov     eax,.lx1
 ;  cmp     eax,.startx
 ;  je      @f          ; prevent artifact borders on tri
 ;  cmp     eax,.lx2    ; NOT work as I want !!
;   je      @f

   cmp     ecx,.cur_tri ; prevent self shadowing
   je      .skipp
  @@:
 if 0
   mov     edi,ecx
   imul    edi,[i12]
   add     edi,[triangles_ptr]
   mov     eax,[edi]
   mov     ebx,[edi+4]
   mov     edx,[edi+8]
   imul    eax,[i12]
   imul    ebx,[i12]
   imul    edx,[i12]
   add     eax,[points_ptr]
   add     ebx,[points_ptr]
   add     edx,[points_ptr]
   movups   xmm2,[eax]
   movups   xmm3,[ebx]
   movups   xmm4,[edx]
   andps    xmm2,[sign_mask]
   andps    xmm3,[sign_mask]
   andps    xmm4,[sign_mask]
   movmskps ebx,xmm4
   cmpeqps  xmm2,xmm3
   cmpeqps  xmm3,xmm4
   andps    xmm2,xmm3
   movmskps eax,xmm2
   and      eax,111b
   and      ebx,111b
   cmp      eax,111b
   jne      @f
   bt       .aabb_mask,ebx
   jnc      .skipp
  @@:
end if
   mov     edi,ecx
   imul    edi,[i12]
   add     edi,[triangles_ptr]
   mov     eax,[edi]
   mov     ebx,[edi+4]
   mov     edx,[edi+8]
   imul    eax,[i12]
   imul    ebx,[i12]
   imul    edx,[i12]
   add     eax,[points_rotated_ptr]
   add     ebx,[points_rotated_ptr]
   add     edx,[points_rotated_ptr]
   movups   xmm2,[eax]
   movups   xmm3,[ebx]
   movups   xmm4,[edx]
   addps    xmm2,.vect_t
   addps    xmm3,.vect_t
   addps    xmm4,.vect_t


;intersect_tri: procs header
; in:
;     xmm0 - ray direction  ; should be normalized
;     xmm1 - ray orgin
;     xmm2 - tri vert1
;     xmm3 - tri vert2
;     xmm4 - tri vert3
;     if  eax = 1 - intersction with edge
;        xmm6 - edge lenght
;     if  eax = 0 - intersect with ray (classic)
; out:
;     eax  = 1 - intersection occured
;     xmm0 - float lo -> hi = t, v, u, ...

   movss     xmm6,.Rlen
   movaps    xmm0,.nray
   movaps    xmm1,.r1
   subss     xmm6,[the_one]
   mov       eax,1
   push      ecx
   call      intersect_tri
   pop       ecx
   cmp       eax,1
   je        .inter
 .skipp:
 .skp:
   inc       ecx
   cmp       ecx,[triangles_count_var]
   jnz       .nx_tri
;   jz        .do_process
;   comiss  xmm0,.Rlen
;   jl      .inter

   popad
 .do_process:
        movaps    xmm5,.nray  ;[edx]
        andps     xmm5,[zero_hgst_dd]  ; global
        mulps    xmm5,.cnv  ;.lv   ; last dword should be zeroed
 ;       andps     xmm5,[sign_z]   ; global
        haddps   xmm5,xmm5
        haddps   xmm5,xmm5
        andps     xmm5,[abs_mask]  ; global
        movaps   xmm7,xmm5
        mulps    xmm7,xmm7
        mulps    xmm7,xmm7
        mulps    xmm5,[edx+16]
        mulps    xmm7,xmm7
        mulps    xmm7,xmm7
        mulps    xmm7,[edx+48]
        addps    xmm5,xmm7
        minps    xmm5,[mask_255f]  ; global
        maxps    xmm5,.final_col     ;  addps  maxps
        movaps   .final_col,xmm5
        jmp     .nx_loop
  .inter:

        popad
      .nx_loop:
      ;  add      edx,64    ; unncomment to achive 3 lights
      ;  add      ebx,16
      ;  cmp      edx,lights_aligned_end    ; global
      ;  jnz      .nx_light

        movaps    xmm1,.final_col
        cvtps2dq  xmm1,xmm1
        packssdw  xmm1,xmm1
        packuswb  xmm1,xmm1
        movd      [edi],xmm1
  .skips:
        movaps   xmm0,.n1
        movss    xmm2,.z1
        add      edi,4
        add      esi,4
        add      dword .lx1,1
        addps    xmm0,.dn
        addss    xmm2,.dz
        movaps   .n1,xmm0
        movss    .z1,xmm2
        dec      ecx
        jnz      .ddraw
  .end_rp_line:
        add      esp,270
        pop      ebp

ret
