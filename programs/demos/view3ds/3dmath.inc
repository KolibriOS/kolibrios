x3d equ 0
y3d equ 2
z3d equ 4
vec_x equ 0
vec_y equ 4
vec_z equ 8

if 0  ; Ext >= SSE3
calc_bounding_box:
; in:
;      xmm0  -  normal vector of ray
;      xmm1  -  light origin
; out:
;      eax - axis aligned bounding boxes bit mask

       .rmx     equ [ebp-36]
       .nray    equ [ebp-64]
       .origin  equ [ebp-80]
       .dirfrac equ [ebp-96]
       .nrayr   equ [ebp-112]
       .originr equ [ebp-128]
       .tmin    equ [ebp-132]
       .tmax    equ [ebp-136]


       push    ebp
       mov     ebp,esp
       and     ebp,-16
       sub     esp,160

       movss     xmm5,[rsscale]
       shufps    xmm5,xmm1,0
       movd      xmm2,[vect_x]
       punpcklwd xmm2,[the_zero]
       cvtdq2ps  xmm2,xmm2
       subps     xmm1,xmm2
       movaps    .origin,xmm1
       mulps     xmm0,xmm5
       movaps    .nray,xmm0

       mov     esi,matrix
       lea     edi,.rmx
       call    reverse_mx_3x3

;  in:  esi - ptr to points(normals], each point(normal) coeficient as dword
;       edi - ptr to rotated points(normals)
;       ebx - ptr to 3x3 (9 dwords, 36 bytes) rotation matrix
;       ecx - number of points(normals)

    ; reverse transform
       lea    esi,.nray
       lea    edi,.nrayr
       lea    ebx,.rmx
       mov    ecx,1
       call   rotary

       lea    esi,.origin
       lea    edi,.originr
       lea    ebx,.rmx
       mov    ecx,1
       call   rotary

       xor      ecx,ecx
       mov      ebx,aabb1
       xor      eax,eax
       rcpps    xmm7,.nrayr
       movaps   .dirfrac,xmm7

    .nx_aabb:
       movaps   xmm5,[ebx]
       movaps   xmm6,[ebx]
       minps    xmm5,[the_zero]
       maxps    xmm6,[the_zero]
 ;  xmm5 - lb corner of AABB with minimal coordinates
 ;  xmm6 - rt cor.   of AABB wit maximum coords
       subps    xmm5,.originr
       subps    xmm6,.originr
       mulps    xmm5,.dirfrac   ; xmm5 - tx1, ty1
       mulps    xmm6,.dirfrac   ; xmm6 - tx2, ty2
       movaps   xmm1,xmm6
       movaps   xmm2,xmm6


       minps    xmm1,xmm5
       maxps    xmm2,xmm5

       movaps   xmm5,xmm1
       movaps   xmm6,xmm2
       shufps   xmm5,xmm5,11100001b
       shufps   xmm6,xmm6,11100001b
       maxss    xmm1,xmm5  ;t min
       minss    xmm2,xmm6  ;t max
       comiss   xmm2,xmm1
       jb       .no_inter
    .yes:
       bts      eax,ecx
     .no_inter:
       add      ebx,16
       inc      ecx
       cmp      ecx,8
       jne      .nx_aabb

; out: eax - bit mask
       add      esp,160
       pop      ebp
ret
end if

reverse_mx_3x3:
; esi - source matrix
; edi - desired reversed matrix

  push  ebp
  mov   ebp,esp
  sub   esp,4
  .det  equ  ebp-4

  fninit
  fld  dword[esi]
  fmul dword[esi+16]
  fmul dword[esi+32]
  fld  dword[esi+12]
  fmul dword[esi+28]
  fmul dword[esi+8]
  faddp
  fld  dword[esi+24]
  fmul dword[esi+4]
  fmul dword[esi+20]
  faddp
  fld  dword[esi]
  fmul dword[esi+28]
  fmul dword[esi+20]
  fchs
  faddp
  fld  dword[esi+24]
  fmul dword[esi+16]
  fmul dword[esi+8]
  fchs
  faddp
  fld  dword[esi+12]
  fmul dword[esi+4]
  fmul dword[esi+32]
  fchs
  faddp
  fstp dword[.det]
  cmp  dword[.det],0
  jne  @f
  int3
 @@:
 ; fld1
 ; fdiv dword[.det]
 ; fstp dword[.det]

  fld  dword[esi+16]
  fmul dword[esi+32]
  fld  dword[esi+20]
  fmul dword[esi+28]
  fchs
  faddp
  fdiv dword[.det]
  fstp dword[edi]

  fld  dword[esi+8]
  fmul dword[esi+28]
  fld  dword[esi+4]
  fmul dword[esi+32]
  fchs
  faddp
  fdiv dword[.det]
  fstp dword[edi+4]

  fld  dword[esi+4]
  fmul dword[esi+20]
  fld  dword[esi+8]
  fmul dword[esi+16]
  fchs
  faddp
  fdiv dword[.det]
  fstp dword[edi+8]

  fld  dword[esi+20]
  fmul dword[esi+24]
  fld  dword[esi+12]
  fmul dword[esi+32]
  fchs
  faddp
  fdiv dword[.det]
  fstp dword[edi+12]

  fld  dword[esi]
  fmul dword[esi+32]
  fld  dword[esi+8]
  fmul dword[esi+24]
  fchs
  faddp
  fdiv dword[.det]
  fstp dword[edi+16]

  fld  dword[esi+8]
  fmul dword[esi+12]
  fld  dword[esi]
  fmul dword[esi+20]
  fchs
  faddp
  fdiv dword[.det]
  fstp dword[edi+20]

  fld  dword[esi+12]
  fmul dword[esi+28]
  fld  dword[esi+16]
  fmul dword[esi+24]
  fchs
  faddp
  fdiv dword[.det]
  fstp dword[edi+24]

  fld  dword[esi+4]
  fmul dword[esi+24]
  fld  dword[esi]
  fmul dword[esi+28]
  fchs
  faddp
  fdiv dword[.det]
  fstp dword[edi+28]

  fld  dword[esi]
  fmul dword[esi+16]
  fld  dword[esi+4]
  fmul dword[esi+12]
  fchs
  faddp
  fdiv dword[.det]
  fstp dword[edi+32]


  mov  esp,ebp
  pop  ebp
ret
; 3d point - triple integer word coordinate
; vector   - triple float dword coordinate
;----------------------in: --------------------------------
;------------------------ esi - pointer to 1st 3d point ---
;------------------------ edi - pointer to 2nd 3d point ---
;------------------------ ebx - pointer to result vector --
;---------------------- out : none ------------------------

make_vector_r:
   if Ext < SSE
        fninit
        fld     dword[edi]                ;edi+x3d
        fsub    dword[esi]                ;esi+x3d
        fstp    dword[ebx+vec_x]

        fld     dword[edi+4]
        fsub    dword[esi+4]
        fstp    dword[ebx+vec_y]

        fld     dword[edi+8]
        fsub    dword[esi+8]
        fstp    dword[ebx+vec_z]
    else
        movups  xmm0,[esi]
        movups  xmm1,[edi]
        subps   xmm1,xmm0
        movlps  [ebx],xmm1
        movhlps  xmm1,xmm1
        movss   [ebx+8],xmm1
     end if

ret
;---------------------- in: -------------------------------
;--------------------------- esi - pointer to 1st vector --
;--------------------------- edi - pointer to 2nd vector --
;--------------------------- ebx - pointer to result vector
;---------------------- out : none
cross_product:
        fninit
        fld     dword [esi+vec_y]
        fmul    dword [edi+vec_z]
        fld     dword [esi+vec_z]
        fmul    dword [edi+vec_y]
        fsubp   ;st1 ,st
        fstp    dword [ebx+vec_x]

        fld     dword [esi+vec_z]
        fmul    dword [edi+vec_x]
        fld     dword [esi+vec_x]
        fmul    dword [edi+vec_z]
        fsubp   ;st1 ,st
        fstp    dword [ebx+vec_y]

        fld     dword [esi+vec_x]
        fmul    dword [edi+vec_y]
        fld     dword [esi+vec_y]
        fmul    dword [edi+vec_x]
        fsubp   ;st1 ,st
        fstp    dword [ebx+vec_z]
ret
cross_aligned:
; params as above cross_p
      movaps  xmm0,[esi]
      movaps  xmm1,[esi]
      movaps  xmm2,[edi]
      movaps  xmm3,[edi]
      shufps  xmm0,xmm0,00001001b
      shufps  xmm1,xmm1,00010010b
      shufps  xmm2,xmm2,00010010b
      shufps  xmm3,xmm3,00001001b
      mulps   xmm0,xmm2
      mulps   xmm1,xmm3
      subps   xmm0,xmm1
      movaps  [ebx],xmm0
ret
;----------------------- in: ------------------------------
;---------------------------- edi - pointer to vector -----
;----------------------- out : none
normalize_vector:
if Ext >= SSE2
        movups  xmm0,[edi]
        andps   xmm0,[zero_hgst_dd]
        movups  xmm1,xmm0
        mulps   xmm0,xmm0
        movhlps xmm2,xmm0
        addps   xmm0,xmm2
        movaps  xmm2,xmm0
        shufps  xmm2,xmm2,11100101b
        addps   xmm0,xmm2
        shufps  xmm0,xmm0,0
;        haddps  xmm0,xmm0
;        haddps  xmm0,xmm0
        rsqrtps xmm0,xmm0
        mulps   xmm0,xmm1
        movlps  [edi],xmm0
        movhlps xmm0,xmm0
        movss   [edi+8],xmm0
else

        fninit
        fld     dword [edi+vec_x]
        fmul    st, st
        fld     dword [edi+vec_y]
        fmul    st, st
        fld     dword [edi+vec_z]
        fmul    st, st
        faddp   st1, st
        faddp   st1, st
        fsqrt

        ftst
        fstsw ax
        sahf
        jnz     @f

        fst     dword [edi+vec_x]
        fst     dword [edi+vec_y]
        fstp    dword [edi+vec_z]
        ret
      @@:
        fld st
        fld st
        fdivr dword [edi+vec_x]
        fstp  dword [edi+vec_x]
        fdivr dword [edi+vec_y]
        fstp  dword [edi+vec_y]
        fdivr dword [edi+vec_z]
        fstp  dword [edi+vec_z]
end if
ret
;------------------in: -------------------------
;------------------ esi - pointer to 1st vector
;------------------ edi - pointer to 2nd vector
;------------------out: ------------------------
;------------------ st0 - dot-product
dot_product:
        fninit
;if Ext >=SSE3
;        movups  xmm0,[esi]
;        movups  xmm1,[edi]
;        andps   xmm0,[zero_hgst_dd]
;        mulps   xmm0,xmm1
;        haddps  xmm0,xmm0
;        haddps  xmm0,xmm0
;        movss   [esp-4],xmm0
;        fld     dword[esp-4]
;else
        fld     dword [esi+vec_x]
        fmul    dword [edi+vec_x]
        fld     dword [esi+vec_y]
        fmul    dword [edi+vec_y]
        fld     dword [esi+vec_z]
        fmul    dword [edi+vec_z]
        faddp
        faddp
;end if
ret

; DOS version Coded by Mikolaj Felix aka Majuma
; mfelix@polbox.com
; www.majuma.xt.pl
; into FASM translation by Macgub
init_sincos_tab:
.counter   equ  dword [ebp-4]  ; cur angle

     push       ebp
     mov        ebp,esp

     xor        eax,eax
     push       eax            ; init .counter
     mov        edi,cos_tab
     mov        esi,sin_tab
     mov        ecx,256
     fninit

     fld        .counter
  @@:
     fld        st
     fsincos
     fstp       dword [edi]
     fstp       dword [esi]
;     fadd       [piD180]
     fadd       [piD128]
     add        esi,4
     add        edi,4
     loop       @b
     ffree      st

     mov        esp,ebp
     pop        ebp
ret
;------
; esi - offset (pointer) to angles, edi offset to 3x3 matrix
make_rotation_matrix:
   .sinx   equ dword[ebp-4]
   .cosx   equ dword[ebp-8]
   .siny   equ dword[ebp-12]
   .cosy   equ dword[ebp-16]
   .sinz   equ dword[ebp-20]
   .cosz   equ dword[ebp-24]
     push      ebp
     mov       ebp,esp
     sub       esp,24

     movzx     ebx,word[esi]
     shl       ebx,2
     mov       eax,dword[sin_tab+ebx]
     mov       .sinx,eax
     mov       edx,dword[cos_tab+ebx]
     mov       .cosx,edx

     movzx     ebx,word[esi+2]
     shl       ebx,2
     mov       eax,dword[sin_tab+ebx]
     mov       .siny,eax
     mov       edx,dword[cos_tab+ebx]
     mov       .cosy,edx

     movzx     ebx,word[esi+4]
     shl       ebx,2
     mov       eax,dword[sin_tab+ebx]
     mov       .sinz,eax
     mov       edx,dword[cos_tab+ebx]
     mov       .cosz,edx

     fninit
     fld       .cosy
     fmul      .cosz
     fstp      dword[edi]

     fld       .sinx
     fmul      .siny
     fmul      .cosz
     fld       .cosx
     fmul      .sinz
     fchs
     faddp
     fstp      dword[edi+12]

     fld       .cosx
     fmul      .siny
     fmul      .cosz
     fld       .sinx
     fmul      .sinz
     faddp
     fstp      dword[edi+24]

     fld       .cosy
     fmul      .sinz
     fstp      dword[edi+4]

     fld       .sinx
     fmul      .siny
     fmul      .sinz
     fld       .cosx
     fmul      .cosz
     faddp
     fstp      dword[edi+16]

     fld       .cosx
     fmul      .siny
     fmul      .sinz
     fld       .sinx
     fchs
     fmul      .cosz
     faddp
     fstp      dword[edi+28]

     fld       .siny
     fchs
     fstp      dword[edi+8]

     fld       .cosy
     fmul      .sinx
     fstp      dword[edi+20]

     fld       .cosx
     fmul      .cosy
     fstp      dword[edi+32]

     mov       esp,ebp
     pop       ebp
ret
;---------------------
;  in:  esi - ptr to points(normals], each point(normal) coeficient as dword
;       edi - ptr to rotated points(normals)
;       ebx - ptr to 3x3 (9 dwords, 36 bytes) rotation matrix
;       ecx - number of points(normals)
rotary:
if Ext<SSE
    fninit
 .again:

    fld     dword[esi]
    fmul    dword[ebx]
    fld     dword[esi+4]
    fmul    dword[ebx+12]
    faddp
    fld     dword[esi+8]
    fmul    dword[ebx+24]
    faddp
    fstp    dword[edi]


    fld     dword[esi+4]
    fmul    dword[ebx+16]
    fld     dword[esi]
    fmul    dword[ebx+4]
    faddp
    fld     dword[esi+8]
    fmul    dword[ebx+28]
    faddp
    fstp    dword[edi+4]


    fld     dword[esi+8]
    fmul    dword[ebx+32]
    fld     dword[esi]
    fmul    dword[ebx+8]
    fld     dword[esi+4]
    fmul    dword[ebx+20]
    faddp
    faddp
    fstp    dword[edi+8]


    add     esi,12
    add     edi,12
    loop    .again
    mov     [edi],dword -1
else
;   Copyright (C) 1999-2001  Brian Paul
;   Copyright (C)            Maciej Guba
;---------------------
;  in:  esi - ptr to points(normals], each point(normal) coeficient as dword
;       edi - ptr to rotated points(normals)
;       ebx - ptr to 3x3 (9 dwords, 36 bytes) rotation matrix
;       ecx - number of points(normals)
;align 32
    movups   xmm4,[ebx]
 ;   lddqu    xmm4,[ebx]   ; I tried sse3 :D
    movups   xmm5,[ebx+12]
    movups   xmm6,[ebx+24]
;align 32
  .again:
    movss    xmm0,dword[esi]
    shufps   xmm0,xmm0,0
    mulps    xmm0,xmm4

    movss    xmm1,dword[esi+4]
    shufps   xmm1,xmm1,0
    mulps    xmm1,xmm5

    movss    xmm2,dword[esi+8]
    shufps   xmm2,xmm2,0
    mulps    xmm2,xmm6

    addps    xmm0,xmm1
    addps    xmm0,xmm2

    movups   [edi],xmm0

    add      esi,12
    add      edi,12
;    dec      ecx
;    jne      .again
   loop     .again
    mov      [edi],dword -1
end if
ret
;----------------------------------------------
;  esi - pointer to 3x3 matrix
add_scale_to_matrix:
  if Ext>SSE
     movss   xmm0,[rsscale]
     shufps  xmm0,xmm0,0
     movups  xmm1,[esi]
     movups  xmm2,[esi+16]
     movss   xmm3,[esi+32]
     mulps   xmm1,xmm0
     mulps   xmm2,xmm0
     mulss   xmm3,xmm0
     movups  [esi],xmm1
     movups  [esi+16],xmm2
     movss   [esi+32],xmm3
  else
     fninit
     fld     [rsscale]
     fld     dword[esi]            ;-----
     fmul    st,st1
     fstp    dword[esi]
     fld     dword[esi+12]           ; x scale
     fmul    st,st1
     fstp    dword[esi+12]
     fld     dword[esi+24]
     fmul    st,st1
     fstp    dword[esi+24]         ;------

     fld     dword[esi+4]          ;-----
     fmul    st,st1
     fstp    dword[esi+4]
     fld     dword[esi+16]            ; y scale
     fmul    st,st1
     fstp    dword[esi+16]
     fld     dword[esi+28]
     fmul    st,st1
     fstp    dword[esi+28]         ;------


     fld     dword[esi+8]          ;-----
     fmul    st,st1
     fstp    dword[esi+8]
     fld     dword[esi+20]              ; z scale
     fmul    st,st1
     fstp    dword[esi+20]
     fld     dword[esi+32]
     fmulp    st1,st
     fstp    dword[esi+32]         ;------
   end if
ret

;in   esi - offset to 3d points  (point as 3 dwords float)
;     edi - offset to 2d points  ( as 3 words integer)
;     ecx - number of points
translate_points:  ; just convert into integer; z coord still needed
  if Ext < SSE
    fninit
  else
 ;   movaps  xmm1,[vect_x]
  end if

  .again:
  if   0
    fld    dword[esi+8]
 ;   fmul   [rsscale]
    fist   word[edi+4]

    fisub  [zobs]
    fchs

    fld    dword[esi]
;    fmul   [rsscale]
    fisub  [xobs]
    fimul  [zobs]
    fdiv   st0,st1

    fiadd  [xobs]
    fiadd  [vect_x]
    fistp  word[edi]

    fld    dword[esi+4]
;    fmul   [rsscale]
    fisub  [yobs]
    fimul  [zobs]
    fdivrp  ;   st0,st1

    fiadd  [yobs]
    fiadd  [vect_y]
    fistp  word[edi+2]
   end if
  if Ext>=SSE2
    movups   xmm0,[esi]
    cvtps2dq xmm0,xmm0
    packssdw xmm0,xmm0
    paddw    xmm0,[vect_x]
    movd     [edi],xmm0
  ;  psrldq   xmm0,4
  ;  movd     eax,xmm0
    pextrw   eax,xmm0,6
    mov      [edi+4],ax
   else

   ; cvtps2dq xmm0,xmm0
   ; packsdw xmm0,xmm0
   ; movq     [edi]
    fld    dword[esi]
    fiadd  word[vect_x]
    fistp  word[edi]
    fld    dword[esi+4]
    fiadd  [vect_y]
    fistp  word[edi+2]
    fld    dword[esi+8]
    fistp  word[edi+4]
  end if

    add    esi,12
    add    edi,6
    loop    .again

ret
