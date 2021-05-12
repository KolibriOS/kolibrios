;===============================================================
do_edges_list:
    push    ebp
    mov     ebp,esp
    sub     esp,32

    .ed_cnt   equ [ebp-4]
    .edd_ptr  equ [ebp-8]
    .counter  equ [ebp-12]


    mov     ebx,[edges_ptr]
    mov     eax,[triangles_ptr]
    mov     ecx,[triangles_count_var]
  @@:
    movdqu  xmm0,[eax]
    movq    [ebx],xmm0
    pshufd  xmm0,xmm0,11001001b
    movq    [ebx+8],xmm0
    pshufd  xmm0,xmm0,11001001b
    movq    [ebx+16],xmm0
    add     ebx,24
    add     eax,12
    loop    @b


    mov     ebx,[edges_ptr]
    mov     ecx,[triangles_count_var]
    lea     ecx,[ecx*3]
  .mxd:
    mov     eax,[ebx]
    cmp     eax,[ebx+4]
    jl      @f
    movq    xmm0,[ebx]
    pshufd  xmm0,xmm0,11100001b
    movq    [ebx],xmm0
  @@:
    add     ebx,8
    loop    .mxd


   ; insert sort
   mov    ebx,[edges_ptr]
   mov    ecx,[triangles_count_var]
   lea    ecx,[ecx*3]

   mov    esi,ecx
   shl    esi,3
   add    esi,ebx

 .ccc:
   mov    eax,[ebx+8]
   cmp    eax,[ebx]
   jge    .g
   movq   xmm0,[ebx+8]
   push   ebx
 .c:
   cmp    ebx,esi
   jge    .done
   cmp    ebx,[edges_ptr]
   jl     .done
   cmp    eax,[ebx]
   jge    .done
   movq   xmm7,[ebx]
   movq   [ebx+8],xmm7
   sub    ebx,8
   jnz    .c
   add    ebx,8
 .done:
   movq   [ebx+8],xmm0
 .p:
   pop    ebx
 .g:
   add    ebx,8
   dec    ecx
   cmp    ecx,1
   jnz    .ccc


  ; insert sort again
   mov    ebx,[edges_ptr]
   mov    ecx,[triangles_count_var]
   lea    ecx,[ecx*3]
   mov    esi,ecx
   shl    esi,3
   add    esi,ebx

 .count:
   push   ecx
   push   ebx
   xor    ecx,ecx
   mov    eax,[ebx]          ; count
 .aa:
   inc    ecx
   add    ebx,8
   cmp    ebx,esi
   jge    .br         ; break
   cmp    eax,[ebx]
   je     .aa
   mov    .counter,ecx
   pop    ebx
   mov    edi,ebx
   sub    edi,8
   mov    edx,[ebx+8]
 .ccc2:

   cmp    ebx,esi
   jbe    @f
   add    esp,4
   jmp    .ff
 @@:
   mov    eax,[ebx+12]
   mov    edx,[ebx+8]
   cmp    eax,[ebx+4]
   jge    .gg2
   movq   xmm0,[ebx+8]
   push   ebx
 .c2:
   cmp    eax,[ebx+4]
   jge    .done2
   movq   xmm7,[ebx]
   movq   [ebx+8],xmm7

   sub    ebx,8
   cmp    ebx,edi
   jz     @f
   cmp    [ebx+8],edx
   jz     .c2
 @@:
 .done2:
   add    ebx,8
   movq   [ebx],xmm0

 .p2:
   pop    ebx
 .gg2:
   add    ebx,8
   dec    ecx
   cmp    ecx,1
   jnz    .ccc2

   pop    ecx
   sub    ecx,.counter
   add    ebx,8
   ja     .count

   jmp    .ff
 .br: ;break
   add   esp,8
 .ff:

   ; count edges
   mov    ecx,0
   mov    edx,[triangles_count_var]
   lea    edx,[edx*3]
   mov    ebx,[edges_ptr]
;   mov    esi,edx
;   shl    esi,3
;   add    esi,[edges_ptr]
 .nx:
   movq    xmm0,[ebx]
   add     ebx,8
;   cmp     ebx,esi
;   jae     @f
   movq    xmm1,[ebx]
; @@:
   pcmpeqd xmm0,xmm1
   pmovmskb eax,xmm0
   and     eax,0xff
   cmp     eax,0xff
   jz      @f
   inc     ecx
  @@:
   dec     edx
   jnz     .nx


    mov     .ed_cnt,ecx
    lea     ecx,[ecx*3]
    shl     ecx,2
    add     ecx,65536
    mov     ebx,12
    mov     eax,68
    mov     edx,.edd_ptr
    int     0x40                   ;  -> allocate memory to triangles
    mov     .edd_ptr, eax   ;  -> eax = pointer to allocated mem



   mov      ebx,[edges_ptr]
   mov      ecx,[triangles_count_var]
   lea      ecx,[ecx*3]
  .seek:
   movq     xmm0,[ebx]
   movq     xmm1,[ebx+8]
   pcmpeqd  xmm1,xmm0
   pmovmskb edx,xmm1
   and      edx,0xff
   cmp      edx,0xff
   je       @f
   movq     [eax],xmm0
   add      eax,8
 @@:
   add      ebx,8
   loop     .seek

   mov       eax,68
   mov       ebx,13
   mov       ecx,[edges_ptr]
   int       0x40          ; release old edges ptr


   mov      eax,.edd_ptr
   mov      ecx,.ed_cnt
   mov      [edges_ptr],eax
   mov      [edges_count],ecx

   mov      esp,ebp
   pop      ebp
ret

;=======================


do_sinus:
   .x      equ  [ebp-8]
   .y      equ  [ebp-12]
   .new_y  equ  [ebp-16]
   .temp   equ  [ebp-20]
   push    ebp
   mov     ebp,esp
   sub     esp,64
   mov     dword .x,0
   mov     dword .y,0
   mov     esi,[screen_ptr]
   mov     edi,[Zbuffer_ptr]
   push    edi
   ;  clear Zbuffer temporally used as image buffer
   movzx   ecx,word[size_x_var]
   movzx   eax,word[size_y_var]
   imul    ecx,eax  ;SIZE_X*SIZE_Y
   xor     eax,eax
   cld
   rep     stosd
   pop     edi
;   movzx   eax,[sinus_flag]
;   mov     edx,10
;   mul     edx
;   mov     [sin_amplitude],eax
;   mov     [sin_frq],eax
   fninit
;if Ext = SSE2
;   movups  xmm1,[const0123]   ; xmm1 - init values
;   mov     eax,0x000000ff
;   movd    xmm2,eax
;   shufps  xmm2,xmm2,0        ; xmm2 - mask value
;   mov     eax,4
;   movd    xmm3,eax
;   shufps  xmm3,xmm3,0
 .again:
if  0
   fild    dword .x
   fidiv   [sin_frq]
   fsin
   fimul   [sin_amplitude]
   fiadd   dword .y
   fistp   dword .new_y
else
   fild    dword .x
   fmul    [sin_frq]
   fistp   dword .temp
   mov     eax, .temp
;   mov     bx, [angle_x]
;   add     bx, [angle_y]
;   movzx   ebx,bx
;   shr     ebx,1       ; change phase
;   add     eax,ebx


   and     eax, 0x000000ff

;   cdq
 ;  mul     [sin_frq]
;   and      eax,0x000000ff
;   and     ax,0x00ff
;   cwde

   fld     dword [sin_tab+eax*4]
   fimul   dword [sin_amplitude]
   fiadd   dword .y
   fistp   dword .new_y
end if
   mov     eax,.new_y
   or      eax,eax
   jl      .skip
   movzx   ebx,word[size_y_var]
   cmp     eax,ebx  ;SIZE_Y
   jg      .skip
   movzx   edx,word[size_x_var]
   mul     edx
;   shl     eax,9
   add     eax,dword .x

   lea     ebx,[eax*3]
   cmp     [dr_flag],12 ; 32 bit col cause
   jl      @f
   add     ebx,eax
  @@:
   mov     eax,[esi]
   mov     [edi+ebx],eax
 .skip:
   add     esi,3
   cmp     [dr_flag],12
   jl      @f
   inc     esi
  @@:
   inc     dword .x
   movzx   edx,word[size_x_var]
   cmp     dword .x,edx  ;SIZE_X
   jl      .again
   mov     dword .x,0
   inc     dword .y
   movzx   edx,word[size_y_var]
   cmp     dword .y,edx   ;SIZE_Y
   jl      .again

   ; copy from temporary buffer -> Zbuffer to screen
   mov     esi,[Zbuffer_ptr]
   mov     edi,[screen_ptr]
   movzx   ecx,word[size_x_var]
   movzx   eax,word[size_y_var]
   imul    ecx,eax
   cmp     [dr_flag],12
   jge     @f
   lea     ecx,[ecx*3]
   shr     ecx,2
 ;  mov     ecx,SIZE_X*SIZE_Y*3/4
  @@:
   cld
   rep     movsd


   mov     esp,ebp
   pop     ebp
ret


draw_dots:
   mov     esi,[points_translated_ptr]
   mov     ecx,[points_count_var]
 .drw:
 @@:
   lodsd
   add     esi,2           ; skip z
   movzx   ebx,ax
   shr     eax,16          ; bx = x , ax = y
   or      ax,ax
   jl      @f
   or      bx,bx
   jl      @f
   cmp     ax,[size_y_var]  ;SIZE_Y
   jge     @f
   cmp     bx,[size_x_var]  ;SIZE_X
   jge     @f
   movzx   edx,word[size_x_var]  ;SIZE_X      ; SIZE_X not only power of 2   -> 256,512,...
   mul     edx
   add     eax,ebx
   mov     edi,[screen_ptr]
   lea     eax,[eax*3]
   add     edi,eax
   xor     eax,eax
   not     eax
   stosd
 @@:
   loop    .drw

ret

do_emboss:   ; sse2 version only
if Ext >= SSE2
 movzx ecx,[bumps_deep_flag]
 inc   ecx
 call  blur_screen    ;blur n times

 mov   eax,[size_y_var]  ;load both x, y
 mov   ebx,eax
 shr   ebx,16
 cwde
 mul   ebx
 mov   ecx,eax
 sub   ecx,ebx
 sub   ecx,ebx
 mov   esi,[screen_ptr]
 mov   edi,[Zbuffer_ptr]
 cmp   [dr_flag],12
 jge   @f
 lea   ebx,[ebx*3]
 jmp   .f
@@:
 shl   ebx,2
.f:
 mov   edx,esi
 add   esi,ebx
 lea   ebx,[ebx+esi]
 pxor  xmm0,xmm0
 push  eax
.emb:
 cmp   [dr_flag],12
 jge   @f
 movlps     xmm1,[esi+3]
 movhps     xmm1,[esi+6]
 movlps     xmm2,[esi-3]
 movhps     xmm2,[esi]
 movlps     xmm3,[ebx]
 movhps     xmm3,[ebx+3]
 movlps     xmm4,[edx]
 movhps     xmm4,[edx+3]
 jmp        .ff
@@:
 movlps     xmm1,[esi+4]
 movhps     xmm1,[esi+8]
 movlps     xmm2,[esi-4]
 movhps     xmm2,[esi]
 movlps     xmm3,[ebx]
 movhps     xmm3,[ebx+4]
 movlps     xmm4,[edx]
 movhps     xmm4,[edx+4]
.ff:
 punpcklbw  xmm1,xmm0
 punpcklbw  xmm2,xmm0
 punpcklbw  xmm3,xmm0
 punpcklbw  xmm4,xmm0
 psubsw     xmm1,xmm2
 paddw      xmm1,[emboss_bias]
 psubsw     xmm3,xmm4
 paddw      xmm3,[emboss_bias]
 pmulhw     xmm1,xmm3
 movaps      xmm7,xmm1
 movaps      xmm6,xmm1
 psrlq       xmm7,2*8
 psrlq       xmm6,4*8
 pmaxsw      xmm1,xmm7
 pmaxsw      xmm1,xmm6

if 0
 movaps      xmm7,xmm3
 movaps      xmm6,xmm3
 psrlq       xmm7,2*8
 psrlq       xmm6,4*8
 pmaxsw      xmm3,xmm7
 pmaxsw      xmm3,xmm6
end if
 pmaxsw      xmm1,xmm3

 movd        eax,xmm1
 movzx       eax,al
; cmp         [dr_flag],12
; je          @f
 lea         eax,[eax*3+envmap_cub]
; jmp         .fff
;@@:

 mov         eax,[eax]
 mov        [edi],eax  ;xmm1
 psrldq     xmm1,8
 movd       eax,xmm1
 movzx      eax,al
 lea        eax,[eax*3+envmap_cub]
 mov        eax,[eax]
 mov        [edi+4],eax

 cmp    [dr_flag],12
 jl     @f
 add    esi,2
 add    ebx,2
 add    edx,2
@@:

 add    edi,8
 add    esi,6
 add    ebx,6
 add    edx,6
 sub    ecx,2
 jnc    .emb


 pop    ecx  ;,eax
 mov    edi,[screen_ptr]
 mov    esi,[Zbuffer_ptr]
 cmp    [dr_flag],12
 jge    .e
@@:
 movsd
 dec    edi
 loop   @b
.e:
 rep    movsd

end if

ret

;align 16
; emboss_bias:
;    dw 128, 128, 128, 128, 128, 128, 128, 128

if 0  ; old emb proc

 ;  emboss -  after drawing all,
 ;  transfer screen buffer into bump map
 ;  and draw two bump triangles
 ; *************************************
        mov     esi,screen
        mov     edi,bumpmap2
        mov     ecx,TEXTURE_SIZE/3
        cld
if  Ext=NON
        xor     eax,eax
        xor     bh,bh
        xor     dh,dh
      @@:
        lodsb
        movzx   bx,al
        lodsb
        movzx   dx,al
        lodsb
        add     ax,bx
        add     ax,dx
      ;  cwd
      ;  div     [i3]
 ;;       push    ax
 ;;       pop     bx
 ;;       shr     bx,3
 ;;       shr     ax,2
 ;;       add     ax,bx

        lea      eax,[eax*5]
        shr      ax,4

        stosb
        loop    @b
else
        emms
        pxor          mm1,mm1
        mov           ebx,0x0000ffff
      @@:
        movd          mm0,[esi]
        punpcklbw     mm0,mm1
        movq          mm2,mm0
        psrlq         mm2,16
        movq          mm3,mm0
        psrlq         mm3,32
        paddw         mm0,mm2
        paddw         mm0,mm3


        movd          eax,mm0
        and           eax,ebx
        lea           eax,[eax*5]
        shr           ax,4
        stosb
        add           esi,3
        loop          @b

end if
        push    ebp

        push    dword 0          ; env coords
        push    word 0
        push    word SIZE_X
        push    word SIZE_Y
        push    dword 0
        push    dword 0          ; bump coords
        push    word SIZE_X
        push    word SIZE_Y
        push    word 0
        mov     eax,SIZE_Y
        mov     ebx,SIZE_X*65536+0
        xor     ecx,ecx
        mov     edx,bumpmap2
        mov     esi,envmap
        mov     edi,screen
        call    bump_triangle

        push    dword SIZE_X shl 16 + SIZE_Y       ; env coords
        push    word 0
        push    word SIZE_X
        push    word SIZE_Y
        push    word 0
        push    dword SIZE_X shl 16 + SIZE_Y        ; bump coords
        push    word 0
        push    word SIZE_X
        push    word SIZE_Y
        push    word 0
        mov     eax,SIZE_Y
        mov     ebx,SIZE_X * 65536+0
        mov     ecx,SIZE_X shl 16 + SIZE_Y
        mov     edx,bumpmap2
        mov     esi,envmap
        mov     edi,screen
        call    bump_triangle

        pop     ebp
ret
end if
;********************************EMBOSS DONE*******************************


generate_object2:  ; torus
;in  ax - figure number       2=torus, 3=loop, 4=loop
;locals
;   counter dw ?
;   sin     dd ?
;   cos     dd ?
;endl
.counter equ  word[ebp-2]
.sin     equ  dword[ebp-6]
.cos     equ  dword[ebp-10]
.sin2    equ  dword[ebp-14]
.cos2    equ  dword[ebp-18]
.piD180m3 equ dword[ebp-22]
.cD2      equ word[ebp-24]
        push  ebp
        mov   ebp,esp
        sub   esp,24

        push  ax

        fninit
        mov     edi,[points_ptr]
        xor     eax,eax
                                    ; init seed -> 4   3d points
        mov     dword[edi],-1.0     ; x
        add     edi,4
        stosd                       ; y
        stosd                       ; z
        mov     dword[edi],-0.9     ; x1
        mov     dword[edi+4],0.1    ; y1
        add     edi,8
        stosd                       ; z1
        mov     dword[edi],-0.8
        add     edi,4
        stosd
        stosd
        mov     dword[edi],-0.9     ; x3
        mov     dword[edi+4],-0.1   ; y3
        add     edi,8
        stosd                       ; z3
        mov     [points_count_var],4

        fld     [piD180]
        fidiv   [i3]
        fstp    .piD180m3
        mov     .cD2,5

        pop     ax
        mov     ecx,1
        mov     edx,9
      .next:                      ; calc angle and rotate seed 4 points
        mov     .counter,cx
        mov     ebx,[points_ptr]
        fld     .piD180m3
        fimul   .counter
        fld     st
        fsincos
        fstp    .sin
        fstp    .cos
        fadd    st,st0
        fsincos
        fstp    .sin2
        fstp    .cos2

      .rotor:                          ; next 4
        ; rotary y
        fld     dword[ebx]         ; x
        fld     .sin
        fmul    dword[ebx+8]       ; z * sinbeta
        fchs
        fld     .cos
        fmul    dword[ebx]         ; x * cosbeta
        faddp
        fstp    dword[edi]         ; new x
        fmul    .sin             ; old x * sinbeta
        fld     .cos
        fmul    dword[ebx+8]       ; z * cosbeta
        faddp
        dec     dx
        or      dx,dx
        jnz     @f
;        mov     .counter,dx
        fld     st
        fidiv   [i3]
        faddp
    @@:
        fstp    dword[edi+8]       ; new z
        fld     dword[ebx+4]
        or      dx,dx
        jnz     @f
  ;      fld1
  ;      faddp
;        fld     st
        fadd    st,st0
        fadd    st,st0
;        fxch
;        fimul   [i3]
;        fsin
;        faddp
        mov     dx,9
    @@:
        fstp    dword[edi+4]
        ; rotary x
        cmp     al,3
        jl      .end_rot
        fld     dword[edi+4]    ;y
        fld     .sin2
        fmul    dword[edi+8]    ;z
        fld     .cos2
        fmul    dword[edi+4]    ;y
        faddp
        fstp    dword[edi+4]    ; new y
        fmul    .sin2       ; sinbeta * old y
        fchs
        fld     .cos2
        fmul    dword[edi+8]
        faddp
        fstp    dword[edi+8]
        ; rotary z
        cmp     al,4
        jl      .end_rot
        fld     dword[edi]      ;x
        fld     .sin
        fmul    dword[edi+4]    ;y
        fld     .cos
        fmul    dword[edi]      ;x
        faddp
        fstp    dword[edi]      ;new x
        fmul    .sin       ; sinbeta * old x
        fchs
        fld     .cos
        fmul    dword[edi+4]         ; cosbeta * y
        faddp
        fstp    dword[edi+4]    ; new y



      .end_rot:

        add     edi,12
        add     ebx,12
        mov     esi,[points_ptr]
        add     esi,12*4
        cmp     ebx,esi
        jl      .rotor

        add     [points_count_var],4
        add     cx,18
        cmp     cx,(18*21*3)+1
        jle     .next

        mov     edi,[triangles_ptr]
        mov     eax,4
        mov     ebx,4+4
        mov     [triangles_count_var],160*3    ;164*3   ;140

        mov     ecx,80*3  ;68
      @@:
        stosd                 ;----
        mov     [edi],ebx      ;    |
        add     edi,4         ;    |
        inc     eax            ;    |
        stosd                 ;    |repeat 4 times

        mov     [edi],ebx      ;    |
        inc     ebx
        add     edi,4
        stosd                 ;    |
        mov     [edi],ebx      ;    |
        add     edi,4         ;----
        loop     @b


        mov     dword[edi],-1  ; < - end mark
        mov       [culling_flag],0

        mov     esp,ebp
        pop     ebp

ret
generate_object3:  ; heart
;locals
;   counter dw ?
;   sin     dd ?
;   cos     dd ?
;endl
.counter equ  word[ebp-2]
.sin     equ  dword[ebp-6]
.cos     equ  dword[ebp-10]
.sin2    equ  dword[ebp-14]
.cos2    equ  dword[ebp-18]
.piD180m3 equ dword[ebp-22]
.cD2      equ word[ebp-24]
        push  ebp
        mov   ebp,esp
        sub   esp,24

        fninit
        mov     edi,[points_ptr]
        xor     eax,eax
                               ; init seed -> eight   3d points
        mov     dword[edi],2.0
        add     edi,4
        stosd
        stosd

        mov     dword[edi],2.0
        mov     dword[edi+4],-0.5
        add     edi,8
        stosd

        mov     dword[edi],1.5
        mov     dword[edi+4],-1.5
        add     edi,8
        stosd
        mov     dword[edi],1.0
        mov     dword[edi+4],-2.0
        add     edi,8
        stosd

        stosd
        mov     dword[edi],-2.5
        add     edi,4
        stosd

        mov     [points_count_var],5

        mov     ecx,1
      .next:                      ; calc angle and rotate seed 4 points
        mov     .counter,cx
        mov     ebx,[points_ptr]
        fld     [piD180]
        fimul   .counter
        fsincos
        fstp    .sin
        fstp    .cos

      .rotor:                          ; next 4
        ; rotary y
        fld     dword[ebx]         ; x
        fld     .sin
        fmul    dword[ebx+8]       ; z * sinbeta
        fchs
        fld     .cos
        fmul    dword[ebx]         ; x * cosbeta
        faddp
        fidiv   [i3]
        fstp    dword[edi]         ; new x
        fmul    .sin               ; old x * sinbeta
        fld     .cos
        fmul    dword[ebx+8]       ; z * cosbeta
        faddp
        fstp    dword[edi+8]       ; new z

        fld     dword[ebx+4]   ;y
        fstp    dword[edi+4]


      .end_rot:

        add     edi,12
        add     ebx,12
        mov     esi,[points_ptr]
        add     esi,12*5
        cmp     ebx,esi  ;real_points + (12*5)
        jl      .rotor

        add     [points_count_var],5
        add     cx,18
        cmp     cx,(18*21)+1
        jle     .next
;last points

        xor     eax,eax

        mov     dword[edi],0.22
        mov     dword[edi+4],0.77
        mov     dword[edi+8],1.25
        add     edi,12

        mov     dword[edi],0.22
        mov     dword[edi+4],0.77
        mov     dword[edi+8],-1.25
        add     edi,12
        stosd

        add     [points_count_var],2

; init triangles list

        mov     edi,[triangles_ptr]
        mov     eax,5
        mov     ebx,5+5
        mov     [triangles_count_var],200  ;204

        mov     ecx,100
      @@:
        stosd                 ;----
        mov     [edi],ebx      ;    |
        add     edi,4         ;    |
        inc     eax            ;    |
        stosd                 ;    |repeat

        mov     [edi],ebx      ;    |
        inc     ebx
        add     edi,4
        stosd                 ;    |
        mov     [edi],ebx      ;    |
        add     edi,4         ;----
        loop     @b

        mov     eax,5
        mov     ebx,[points_count_var]
        sub     ebx,2
        mov     dl,2
    .nx:
        mov     ecx,5
        add     [triangles_count_var],ecx
    @@:
        stosd
        add     eax,5
        stosd
        mov     dword[edi],ebx
        add     edi,4
        loop    @b

        cmp     dl,1
        je      @f

        inc     ebx
        jmp     .lab
     @@:
        dec     ebx
     .lab:
        mov     ecx,5
        add     [triangles_count_var],ecx
     @@:
        stosd
        add     eax,5
        stosd
        mov     dword[edi],ebx
        add     edi,4
        loop    @b

        dec     dl
        or      dl,dl
        jnz     .nx

        sub     eax,25
        stosd
        sub     eax,50
        stosd
        mov     dword[edi],ebx
        add     edi,4

        stosd
        add     eax,50
        stosd
        inc     ebx
        mov     dword[edi],ebx
        add     edi,4
        add     [triangles_count_var],2

        mov     dword[edi],-1  ; < - end mark
        mov     [culling_flag],0

        mov     esp,ebp
        pop     ebp

ret



