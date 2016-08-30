MAX_SPHERES = 4 ;8
MAX_LIGHTS = 6

main_loop:
  call normalize_surface
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

normalize_surface:
  movss   xmm0,[surface+8]
  movlhps xmm0,xmm0
  movlps  xmm0,[surface]
  movaps  xmm1,xmm0
  mulps   xmm0,xmm0
  haddps  xmm0,xmm0
  haddps  xmm0,xmm0
  sqrtss  xmm0,xmm0
  shufps  xmm0,xmm0,0
  divps   xmm1,xmm0
  movaps  xmm2,xmm1
  pslldq  xmm2,4
  psrldq  xmm2,4
  movaps  [surface_without_d],xmm2
  movlps  [surface],xmm1
  movhlps xmm1,xmm1
  movss   [surface+8],xmm1
ret

find_intersection:
;   eax - y
;   ebx - x
   push ebp
   mov	ebp,esp
   sub	esp,256
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
   .dx_sh	  equ	    [ebp-96]
   .a_sh	  equ	    [ebp-100]
   .b_sh	  equ	    [ebp-104]
   .c_sh	  equ	    [ebp-108]
   .delta_sh	  equ	    [ebp-112]
   .sph_counter_sh equ dword[ebp-116]
   .shadow_mark   equ dword [ebp-120]
   .nearest_surface equ     [ebp-144]
   .vd		  equ	    [ebp-148]	; denominator in plane inters. equation

   mov	    .iy,eax
   mov	    .ix,ebx

   mov	    .one_write,0
   mov	    .shadow_mark,0
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
 .next_sph:		      ; intersection with sphere
   movaps   xmm5,[camera]
   mov	    edx,.sph_counter
   shl	    edx,4
   add	    edx,sphere
   subps    xmm5,[edx]
   mulps    xmm5,[float2]
   mulps    xmm5,.dx
   haddps   xmm5,xmm5
   haddps   xmm5,xmm5
   movss    .b,xmm5


   movaps    xmm4,[edx]
   mulps     xmm4,xmm4
   movaps    xmm5,[camera]
   mulps     xmm5,xmm5
   addps     xmm4,xmm5
   haddps    xmm4,xmm4
   haddps    xmm4,xmm4
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
   movd      ecx,xmm5  ; ecx = -1 => greater than 0.0
   cmp	     ecx,0
   jnz	     @f
   jmp	     .next_s   ; no intersection


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
   maxss     xmm6,xmm4
   cmp	     .one_write,0   ; test if sth in 'nearest' data is written
   jz	     @f
   movss     xmm4,xmm6	;5
   cmpnltss  xmm4,[smalest_t]
   movd      ecx,xmm4
   or	     ecx,ecx
   jz	    .next_s
 @@:
   movss     [smalest_t],xmm6  ;5
   movaps    xmm0,[edx]
   movaps    [nearest_sphere],xmm0
   push      dword[ebx]
   pop	     dword[nearest_radius]
   mov	     .one_write,1	 ; one_write - object index -> 1 = sphere
 .next_s:
  add	    .sph_counter,1
  cmp	    .sph_counter,MAX_SPHERES
  jnz	    .next_sph

if 1
  movaps    xmm0,[surface_without_d]  ; find with plane intersection
  mulps     xmm0,[camera]	      ; only one surface is computed
  haddps    xmm0,xmm0
  haddps    xmm0,xmm0
  addss     xmm0,[surface+12]
  movaps    xmm1,[surface_without_d]
  mulps     xmm1,.dx
  haddps    xmm1,xmm1
  haddps    xmm1,xmm1
  xorps     xmm2,xmm2
  cmpnless  xmm2,xmm1
  movd	    ecx,xmm2
  cmp	    ecx,0  ;-1
  je	    .put_pixel ; denominator equal 'zero' - no intersection
  xorps     xmm2,xmm2  ; denominator > 0 -> inters. not in screen area
  movss     .vd,xmm1   ; write to memory this denom.
  divss     xmm0,xmm1
  subss     xmm2,xmm0
  cmp	    .one_write,0
  jz	    @f
  movss     xmm0,xmm2
  cmpnltss  xmm2,[smalest_t]
  movd	    ecx,xmm2
  cmp	    ecx,0
  je	    .put_pixel
 @@:
  movss     [smalest_t],xmm0
;  test      [smalest_t],0x80000000
;  jz	     @f
;  and	     [smalest_t],0x7fffffff
; @@:
  movaps    xmm2,[surface]
  movaps    .nearest_surface,xmm2
  mov	    .one_write,2      ; nearest object -> 2 = flat plane

end if

 .put_pixel:
   cmp	     .one_write,0  ; end if no intersection
   je	     .end

   movss     xmm5,[smalest_t]
   shufps    xmm5,xmm5,0   ; calc and put pixel
   movaps    xmm6,.dx
   mulps     xmm6,xmm5
   movaps    xmm4,[camera]
   addps     xmm4,xmm6	; xmm4 - x,y,z on the sphere or on surface
   movaps    .sph_xyz,xmm4
if 1
   cmp	     .one_write,2
   jne	     .shadow
;   movaps    xmm4,.sph_xyz
   movaps    xmm7,xmm4
   lea	     ebx,.nearest_surface
   movss     xmm4,[ebx+8]
   movlhps   xmm4,xmm4
   movlps    xmm4,[ebx]   ; xmm4 - normal to surface vector
   test      dword .vd,0x80000000
   jz	     @f
   andps     xmm4,[positive_mask]  ;0x7fffffff
 @@:

   jmp	     .calc_pix

end if
 .shadow:
if 1
; to find shadow intersect:
; P0 - point on sphere
; P1 - light
; with every other sphere in scene if any intersection occured -
;    - point is in shadow
 ; next_sph_shad:
   mov	    ecx,MAX_LIGHTS
 .next_light_sh:
   push     ecx
   shl	    ecx,4
   movaps   xmm0,[ecx+light]   ;xmm4  - point on nearest sphere
   subps    xmm0,xmm4
   movaps   .dx_sh,xmm0

   mulps    xmm0,xmm0
   haddps   xmm0,xmm0
   haddps   xmm0,xmm0
   movss    .a_sh,xmm0

   mov	    .sph_counter_sh,0
 .next_sph_sh:	    ; be sure you not intersect nearest sphere with itself
   movaps   xmm5,.sph_xyz  ;[light]
   mov	    edx,.sph_counter_sh
   shl	    edx,4
   add	    edx,sphere
   movaps   xmm7,[edx]
   cmpeqps  xmm7,[nearest_sphere]
   movmskps ecx,xmm7
   and	    ecx,0111b
   cmp	    ecx,0
   jne	     .next_s_sh

   subps    xmm5,[edx] ; [edx] - cur sph
   mulps    xmm5,[float2]
   mulps    xmm5,.dx_sh
   haddps   xmm5,xmm5
   haddps   xmm5,xmm5
   movss    .b_sh,xmm5

   movaps    xmm4,[edx]
   mulps     xmm4,xmm4
   movaps    xmm5,.sph_xyz
   mulps     xmm5,xmm5
   addps     xmm4,xmm5
   haddps    xmm4,xmm4
   haddps    xmm4,xmm4
   movaps    xmm5,.sph_xyz
   mulps     xmm5,[edx]
   haddps    xmm5,xmm5
   haddps    xmm5,xmm5
   mulss     xmm5,[float2]
   subss     xmm4,xmm5
   mov	     ebx,.sph_counter_sh
   shl	     ebx,2
   add	     ebx,sph_radius
   movss     xmm5,[ebx]
   mulss     xmm5,xmm5
   subss     xmm4,xmm5
   movss     .c_sh,xmm4

   movss     xmm5,.b_sh
   mulss     xmm5,xmm5
   mulss     xmm4,.a_sh
   mulss     xmm4,[float4]
   subss     xmm5,xmm4
   movss     .delta_sh,xmm5
   xorps     xmm6,xmm6
   cmpnltss  xmm5,xmm6
   movd      ecx,xmm5  ; ecx = -1 greater than 0.0
   cmp	     ecx,0
   jnz	     @f
   jmp	     .next_s_sh   ; no intersection
  @@:
   add	     .shadow_mark,1  ; mark ->point in shadow
   pop	     ecx
   sub	     ecx,1
   jnz	    .next_light_sh
   jmp	     .put_pix
  .next_s_sh:
   add	     .sph_counter_sh,1
   cmp	     .sph_counter_sh,MAX_SPHERES
   jnz	     .next_sph_sh
   pop	     ecx
   sub	     ecx,1
   jnz	     .next_light_sh

end if






 .put_pix:
   movaps    xmm4,.sph_xyz
   movaps    xmm7,xmm4
   subps     xmm4,[nearest_sphere]

   movss     xmm0,[nearest_radius]
   shufps    xmm0,xmm0,0
   divps     xmm4,xmm0	; xmm4 - normal to surface vector
 .calc_pix:  ; normal computed
   movaps    xmm1,xmm4	; copy of normal in xmm1
   xor	     eax,eax
   xorps     xmm3,xmm3
 ;  movss     xmm2,[light_factor] ; other model of lighting
 ;  shufps    xmm2,xmm2,0

 .next_light:
   mov	     ebx,eax
   shl	     ebx,4
   movaps    xmm5,[light+ebx]
   subps     xmm5,xmm7	; calc light unit vector
   movaps    xmm6,xmm5
   mulps     xmm5,xmm5
   haddps    xmm5,xmm5
   haddps    xmm5,xmm5
   sqrtss    xmm5,xmm5
   shufps    xmm5,xmm5,0
   divps     xmm6,xmm5	; xmm6 - normalized light vector
  ; dot_product
   movaps    xmm4,xmm1	; xmm4 - normal to surface
   mulps     xmm4,xmm6
   haddps    xmm4,xmm4
   haddps    xmm4,xmm4
   shufps    xmm4,xmm4,0
   mulps     xmm4,[lights_color+ebx]  ; xmm4 - computed col. light vector dep.

;   mulps     xmm4,xmm2  ; other model of lighting
;   addps     xmm3,xmm4

   maxps     xmm3,xmm4 ; will be this better ?

   add	     eax,1
   cmp	     eax,MAX_LIGHTS
   jnz	     .next_light

   minps     xmm3,[float255]
   cmp	     .shadow_mark,0
   je	     @f
   cvtsi2ss  xmm2,.shadow_mark
   shufps    xmm2,xmm2,0
   mulps     xmm2,[shadow_factor]
   subps     xmm3,xmm2
   xorps     xmm0,xmm0
   maxps     xmm3,xmm0
 @@:

   cvtps2dq  xmm3,xmm3
   packssdw  xmm3,xmm3
   packuswb  xmm3,xmm3
   paddusb   xmm3,[ambient_col]

if 1
   cmp	     .one_write,2
   jne	     .perspective
   movaps    xmm0,xmm3	      ; calc texture on plane
   movaps    xmm1,.sph_xyz
   mov	     ecx,XRES
   cvtsi2ss  xmm4,ecx
   shufps    xmm4,xmm4,0
   mulps     xmm1,xmm4
   cvtps2dq  xmm1,xmm1
   movd      ecx,xmm1
   test      ecx,0x8
   jz	     @f
   mov	     ecx,0xffffffff
   jmp	     .next_tex_test
 @@:
   xor	     ecx,ecx
 .next_tex_test:
   psrldq    xmm1,8
   movd      ebx,xmm1
   test      ebx,0x20
   jz	     @f
   mov	     ebx,0xffffffff
   jmp	     .set_tex
  @@:
   xor	     ebx,ebx
  .set_tex:
   xor	     ebx,ecx
   shr	     ebx,28
   mov	     bh,bl
   movd      xmm7,ebx
   paddusb   xmm3,xmm7

end if


.perspective:
   movaps xmm0,.sph_xyz   ; perspective correction
   subps  xmm0,[camera]
   movss  xmm1,[camera]
   movss  xmm2,xmm0
   movaps xmm4,xmm0
   shufps xmm4,xmm4,00000010b
   divss  xmm2,xmm4
   mulss  xmm2,[camera+8]
   subss  xmm1,xmm2 ; xmm1 - x

   movaps xmm2,xmm0
   shufps xmm2,xmm2,00000001b
   movaps xmm4,xmm0
   shufps xmm4,xmm4,00000010b
   divss  xmm2,xmm4
   mulss  xmm2,[camera+8]
   movss  xmm4,[camera+4]
   subss  xmm4,xmm2   ; xmm4 - y

   mov	    ebx,XRES
   cvtsi2ss xmm2,ebx
   mulss    xmm1,xmm2
   mulss    xmm4,xmm2
   cvtss2si ecx,xmm1
   mov	    .ix,ecx
   cvtss2si edx,xmm4
   mov	    .iy,edx

   mov	 edi,screen
   mov	 ecx,XRES
   imul  ecx,.iy
   add	 ecx,.ix
   lea	 ecx,[ecx*3]
   add	 edi,ecx
   movd  [edi],xmm3
.end:
   add	  esp,256
   pop	  ebp

ret





