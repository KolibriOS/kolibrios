MAX_SPHERES = 5
MAX_LIGHTS = 3
;ray

main_loop:
  xor  eax,eax	     ; y
.next_line:
  xor  ebx,ebx	     ; x

@@:
  push eax
  push ebx
  call find_intersection
  pop  ebx
  pop  eax
  add  ebx,1
  cmp  ebx,XRES
  jnz  @b

  add  eax,1
  cmp  eax,YRES
  jnz  .next_line

ret
init_calc:	   ; do some intial calculations

ret

find_intersection:  ;with for now single sphere
;   eax - y
;   ebx - x
   push ebp
   mov	ebp,esp
   sub	esp,128
   and	ebp,0xfffffff0
   .dz	  equ  dword[ebp-8]
   .dy	  equ  dword[ebp-12]
   .dx	  equ	    [ebp-16]
   .a	  equ  dword[ebp-20]
   .b	  equ  dword[ebp-24]
   .c	  equ  dword[ebp-28]
   .delta equ  dword[ebp-32]
   .iy	  equ  dword[ebp-36]
   .ix	  equ	    [ebp-40]
   .t1	  equ	    [ebp-44]
   .t2	  equ	    [ebp-48]
   .n	  equ	    [ebp-64]
   .sph_xyz	  equ	    [ebp-80]
   .one_write	  equ byte  [ebp-81] ;tells if sth written in 'nearest' data
   .sph_counter   equ dword [ebp-85]


   mov	    .iy,eax
   mov	    .ix,ebx

   mov	    .one_write,0
   xorps    xmm0,xmm0
   cvtpi2ps xmm0,.ix
   mov	    ecx,XRES
   cvtsi2ss xmm2,ecx
   shufps   xmm2,xmm2,0
   divps    xmm0,xmm2
   subps    xmm0,[camera]
   movaps   .dx,xmm0
   movaps   xmm1,xmm0
   mulps    xmm1,xmm0
   haddps   xmm1,xmm1
   haddps   xmm1,xmm1
   movss    .a,xmm1
   mov	    .sph_counter,0
 .next_sph:
   xorps    xmm5,xmm5
   movaps   xmm5,[camera]
   mov	    edx,.sph_counter
   shl	    edx,4
   add	    edx,sphere
   subps    xmm5,[edx]	;[sphere]  ;;[edx]
   mulps    xmm5,[float2]
   movaps   xmm0,.dx
   mulps    xmm5,xmm0
   haddps   xmm5,xmm5
   haddps   xmm5,xmm5
   movss    .b,xmm5


   movaps    xmm4,[edx]  ;[sphere]   ; [edx]
   mulps     xmm4,xmm4
 ;  haddps    xmm4,xmm4
 ;  haddps    xmm4,xmm4
   movaps    xmm5,[camera]
   mulps     xmm5,xmm5
   addps     xmm4,xmm5
   haddps    xmm4,xmm4
   haddps    xmm4,xmm4
 ;  addss     xmm4,xmm5
   movaps    xmm5,[edx]  ;;[sphere]  ;; [edx]
   mulps     xmm5,[camera]
   haddps    xmm5,xmm5
   haddps    xmm5,xmm5
   mulss     xmm5,[float2]
   subss     xmm4,xmm5
   mov	     ebx,.sph_counter
   shl	     ebx,2
   add	     ebx,sph_radius
   movss     xmm5,[ebx]  ;[R]  ; [ebx]
   mulss     xmm5,xmm5
   subss     xmm4,xmm5
   movss     .c,xmm4

   movss     xmm5,.b
   mulss     xmm5,xmm5
   mulss     xmm4,.a
   mulss     xmm4,[float4]
   subss     xmm5,xmm4
   movss     .delta,xmm5
   xorps     xmm6,xmm6
   cmpnltss  xmm5,xmm6
   movd      ecx,xmm5  ; ecx = -1 greater than 0.0
   cmp	     ecx,0
   jnz	     @f
   jmp	     .next_s   ; no intersection
 ;  add       .sph_counter,1
 ;  cmp       .sph_counter,MAX_SPHERES
 ;  jnz       .next_sph
 ;  jmp       .put_pixel


  @@:
   movss     xmm5,.delta
   sqrtss    xmm5,xmm5
   movss     xmm4,xmm5
   subss     xmm6,.b
   movss     xmm7,xmm6
   subss     xmm6,xmm5
   divss     xmm6,[float2]
   divss     xmm6,.a
   movss     .t1,xmm6
   addss     xmm4,xmm7
   divss     xmm4,[float2]
   divss     xmm4,.a
   movss     .t2,xmm4
   movss     xmm5,xmm4

   cmpnltss   xmm4,.t1
   movd      ecx,xmm4
   or	     ecx,ecx
   jne	     @f
   movss     xmm5,.t2
 @@:
   cmp	     .one_write,0   ; test if sth in 'nearest' data is written
   jz	     @f
   movss     xmm4,xmm5
   cmpnltss  xmm4,[smalest_t]
   movd      ecx,xmm4
   or	     ecx,ecx
   jz	    .next_s
 @@:
   movss     [smalest_t],xmm5
;   push      .a
;   pop       [smalest_a]
;   push      .b
;   pop       [smalest_b]
;   push      .c
;   pop       [smalest_c]
;   push      .delta
;   pop       [smalest_delta]
   movaps    xmm0,[edx]
   movaps    [nearest_sphere],xmm0
   push      dword[ebx]
   pop	     dword[nearest_radius]
   mov	     .one_write,1

 .next_s:
  add	    .sph_counter,1
  cmp	    .sph_counter,MAX_SPHERES
  jnz	    .next_sph

 .put_pixel:
   cmp	     .one_write,0
   je	     .end

   movss     xmm5,[smalest_t]
   shufps    xmm5,xmm5,0   ; calc and put pixel
   movaps    xmm6,.dx
   mulps     xmm6,xmm5
   movaps    xmm4,[camera]
   addps     xmm4,xmm6	; xmm4 - x,y,z on the sphere
   movaps    xmm7,xmm4
   subps     xmm4,[nearest_sphere]
 ;  movaps    xmm6,xmm7
 ;  addps     xmm6,[nearest_sphere]
 ;  movaps    .sph_xyz,xmm6

   movss     xmm0,[nearest_radius]
   shufps    xmm0,xmm0,0
   divps     xmm4,xmm0	; xmm4 - normal to surface vector
   movaps    xmm1,xmm4	; copy of normal in xmm1
   xor	     eax,eax
   xorps     xmm3,xmm3
   movss     xmm2,[light_factor]
   shufps    xmm2,xmm2,0

 .next_light:
 ;  push      eax
   mov	     ebx,eax
   shl	     ebx,4
   ;add       eax,light
   movaps    xmm5,[light+ebx];[light] ;; [eax]
   subps     xmm5,xmm7	; calc light unit vector
   movaps    xmm6,xmm5
   mulps     xmm5,xmm5
   haddps    xmm5,xmm5
   haddps    xmm5,xmm5
   sqrtss    xmm5,xmm5
   divps     xmm6,xmm5	; xmm6 - normalized light vector
  ; dot_product
   movaps    xmm4,xmm1	; xmm4 - normal to surface
   mulps     xmm4,xmm6
   haddps    xmm4,xmm4
   haddps    xmm4,xmm4
   shufps    xmm4,xmm4,0
;   movaps    xmm5,xmm4
;   mulps     xmm5,xmm5
;   mulps     xmm5,xmm5
;   mulps     xmm5,[color]
   mulps     xmm4,[lights_color+ebx]  ; xmm4 - computed col. light vector dep.
;   addps     xmm4,xmm5
   mulps     xmm4,xmm2
   addps     xmm3,xmm4
  ; pop       eax
   add	     eax,1
   cmp	     eax,MAX_LIGHTS
   jnz	     .next_light
if 0
   ; mix with texture
   movaps    xmm0,.sph_xyz
   movss     xmm1,[nearest_radius]
   shufps    xmm1,xmm1,0
   divps     xmm0,xmm1
   mulps     xmm0,[correct_tex] ; f64
 ;  addps     xmm0,[correct_tex]
   cvtss2si  eax,xmm0
   psrldq    xmm0,4
   cvtss2si  ebx,xmm0
   imul      ebx,[tex_x]
   add	     ebx,eax
   lea	     ebx,[ebx*3]
   add	     ebx,bitmap
   ;mov       eax,[ebx]
   movd      xmm1,[ebx]
   xorps     xmm7,xmm7
   punpcklbw xmm1,xmm7
   punpcklwd xmm1,xmm7
   cvtdq2ps  xmm1,xmm1
   mulps     xmm3,xmm1
   divps     xmm3,[float255]
 ;  divps     xmm1,[float2]
 ;  divps     xmm3,[float2]
 ;  addps     xmm3,xmm1
end if


   minps     xmm3,[float255]
   cvtps2dq  xmm3,xmm3
   packssdw  xmm3,xmm3
   packuswb  xmm3,xmm3




   mov	 edi,screen
   mov	 ecx,XRES
   imul  ecx,.iy
   add	 ecx,.ix
   lea	 ecx,[ecx*3]
   add	 edi,ecx
   movd  [edi],xmm3

.end:
   add	  esp,128
   pop	  ebp

ret





