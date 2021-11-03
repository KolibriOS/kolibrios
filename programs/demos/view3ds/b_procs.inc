;init_envmap_cub2:
;        mov esi,envmap
;        mov edi,envmap_cub    ;take cubic env. map from
;        xor eax,eax           ;spherical env. map
;      @@:
;        add esi,eax
;        movsd
;        dec edi
;        dec esi
;        add esi,511*3
;        add eax,3
;        cmp eax,511*3
;        jl  @b
;ret
init_envmap_cub:         ; create 512x512 env map
.temp equ word [ebp-2]
         push     ebp
         mov      ebp,esp
         sub      esp,2
         mov      edi,envmap_cub
         fninit

         mov      cx,-256
    .ie_hor:
         mov      .temp,cx
         fild     .temp
         fabs
  ;      fmul     st,st0
  ;      fsqrt
         mov      .temp,255
         fisubr   .temp
         fmul     [env_const]
         fistp    .temp
         mov      ax,.temp

         or      ax,ax
         jge     .ie_ok1
         xor     ax,ax
         jmp     .ie_ok2
  .ie_ok1:
         cmp     ax,255
         jle     .ie_ok2
         mov     ax,255
  .ie_ok2:
         stosb
         stosb
         stosb

         inc     cx
         cmp     cx,256
         jne     .ie_hor

         mov     esp,ebp
         pop     ebp
ret

calc_one_col:
; procedure don't save registers !!!
; in -   st - dot_product
;        stack - other parameters
; out - eax - 0x00rrggbb
.dot_prd   equ dword[ebp+4]           ; dot product   - cos x  - not now
.min_col_r equ word[ebp+8]            ; minimum color - ambient
.min_col_g equ word[ebp+10]
.min_col_b equ word[ebp+12]
.max_col_r equ word[ebp+14]           ; maximum color - specular
.max_col_g equ word[ebp+16]
.max_col_b equ word[ebp+18]
.org_col_r equ word[ebp+20]           ; orginal color - diffuse
.org_col_g equ word[ebp+22]
.org_col_b equ word[ebp+24]
.n         equ word[ebp+26]           ; shines - not implemented
.temp      equ word[ebp-2]
.color_sum_r equ dword[ebp-6]
.color_sum_g equ dword[ebp-10]
.color_sum_b equ dword[ebp-14]
; color = ambient+cos(x)*diffuse+(cos(x)^n)*specular
         mov     ebp,esp
         sub     esp,14

         mov      ax,.min_col_r
         add      ax,.max_col_r
         add      ax,.org_col_r
         cwde
         mov      .color_sum_r,eax

         mov      ax,.min_col_g
         add      ax,.max_col_g
         add      ax,.org_col_g
         cwde
         mov      .color_sum_g,eax

         mov      ax,.min_col_b
         add      ax,.max_col_b
         add      ax,.org_col_b
         cwde
         mov      .color_sum_b,eax


;         fld     .dot_prd
       ;  fild    .n
       ;  fxch    st1
       ;  fabs
       ;  fyl2x                     ;
       ;  f2xm1
       ;  fld1
       ;  faddp        ; st =  dot_product ^ n

         fld     st            ; copy dot pr
         fmul    st,st0
         fmul    st,st0
         fmul    st,st0
         cmp     .n,255            ; .n = 255 -> spot light
         jne     @f
         fmul    st,st0
         fmul    st,st0
         fmul    st,st0
      @@:
         fld     st            ; st0=st1=dot_pr^n, st2=dot_pr
         fimul   .max_col_b
         fild    .org_col_b
         fmul    st,st3
         faddp                 ; st0=first piece of col, st1=dot_pr^n..
         fiadd   .min_col_b
         fimul   .max_col_b
         fidiv   .color_sum_b
         fistp   .temp
         movzx   eax,.temp
         shl     eax,16

         fld     st
         fimul   .max_col_g
         fild    .org_col_g
         fmul    st,st3
         faddp
         fiadd   .min_col_g
         fimul   .max_col_g
         fidiv   .color_sum_g
         fistp   .temp
         mov     ax,.temp
         mov     ah,al
         shl     eax,8

         fimul   .max_col_r
         fild    .org_col_r
         fmulp   st2,st
         faddp
         fiadd   .min_col_r
         fimul   .max_col_r
         fidiv   .color_sum_r
         fistp   .temp
         mov     ax,.temp          ;eax -  0xbbgg00rr
     ;   mov     ah,al
         ror     eax,16
         xchg    al,ah             ; eax - 0x00rrggbb
         mov     esp,ebp
ret 24

calc_bumpmap:            ; calculate random bumpmap
;--------------in edi _ pointer to TEX_X x TEX_Y bumpmap

         push    edi

         cmp     [bumps_flag],0
         je      .random_bump_map
         ; else bumps according to texture
         mov     esi,texmap
         mov     ecx,TEXTURE_SIZE
      @@:
         movzx   ax,byte[esi]
         movzx   bx,byte[esi+1]
         movzx   dx,byte[esi+2]
         add     ax,bx
         add     ax,dx
         cwd
         div     [i3]
         stosb
         add     esi,3
         loop    @b
         jmp     .blur_map
;         push    ecx
;         mov     eax,0x88888888
;         mov     ecx,16/4
;         rep     stosd
;         mov     eax,0xffffffff
;         mov     ecx,16/4
;         rep     stosd
;         pop     ecx
;         loop    @b
  .random_bump_map:

         mov     ecx,TEXTURE_SIZE
      @@:
         push    ecx
         xor     ecx,ecx
         mov     edx,255
         call    random
         stosb
         pop    ecx
         loop   @b

      .blur_map:
         pop    edi
         movzx  ecx,[bumps_deep_flag]
         inc    cx
      .blur:
         xor    esi,esi
         mov    edx,TEXTURE_SIZE
         xor    eax,eax
         xor    ebx,ebx
      @@:
         mov    ebp,esi
         dec    ebp
         and    ebp,TEXTURE_SIZE
         mov    al,byte[ebp+edi]

         mov    ebp,esi
         inc    ebp
         and    ebp,TEXTURE_SIZE
         mov    bl,byte[ebp+edi]
         add    eax,ebx

         mov    ebp,esi
         sub    ebp,TEX_X
         and    ebp,TEXTURE_SIZE
         mov    bl,byte[ebp+edi]
         add    eax,ebx

         mov    ebp,esi
         add    ebp,TEX_X
         and    ebp,TEXTURE_SIZE
         mov    bl,byte[ebp+edi]
         add    eax,ebx

         shr    eax,2
         mov    byte[esi+edi],al

         inc    esi
         dec    edx
         jnz    @b

         loop   .blur
ret
random:
;  in  - ecx - min
;        edx - max
;  out - eax - random number
         mov    bx,[rand_seed]
         add    bx,0x9248
         ror    bx,3
         mov    [rand_seed],bx

         mov    ax,dx
         sub    ax,cx
         mul    bx
         mov    ax,dx
         add    ax,cx
         cwde
ret

optimize_object1:        ; setting   point (0,0,0) in center of object
                         ; recalculate all coords , scale object,
                         ;the coords in <-1.0,1.0>
;in :  real_points - table filled of real float dd coordinates (x,y,z), end mark  dd -1
;                  _ global variable
;      points_count_var - dw integer variable with exactly points count
;                       - global variable
;      SIZE_X, SIZE_Y must be defined

.max   equ dword[ebp-4]
.min   equ dword[ebp-8]
.maxxx equ dword[ebp-12]
.center equ dword[ebp-16]

        mov     ebp,esp
        sub     esp,16
        fninit
        mov     .maxxx,0
        mov     ecx,3
        xor     ebx,ebx         ; ebx - x,y,z coord in real_points list
    .next_c:                    ; max/min/center x,y,z
        mov     edi,[points_ptr]      ; in real_point list minimum two points
        mov     edx,[points_count_var]
        fld     dword[edi+ebx]
        fst     .max
        fstp    .min
        add     edi,12
        dec     edx
    .next_coord:               ; next coord from real_points list
        fld     dword [edi+ebx]   ; real_points -> x,y,z
        fcom    .max    ; max_x,y,z
        fstsw   ax
        sahf
        jbe     @f             ; jmp less equal
        fstp    .max    ; new max_x,y,z
        jmp     .end_coords
     @@:
        fcom    .min    ; min_x,y,z
        fstsw   ax
        sahf
        jnbe    @f              ; jmp greater
        fst     .min   ; new min_x
    @@:
        ffree   st
    .end_coords:
        add     edi,12
   ;     cmp     dword[edi],-1   ; cmp with end mark
        dec      edx
        jnz     .next_coord
        ; ok after this we found max_coord and min_coord
        fld     .max          ; find center point
        fadd    .min
        fld1
        fld1
        faddp
        fdivp   st1,st        ; st0 - center coord
        fstp    .center

        fld     .max
        fsub    .center      ; st = .max - .center
        fcom    .maxxx      ; maximum of all .max
        fstsw   ax
        sahf
        jbe     @f          ; jmp lower
        fst     .maxxx      ; new maxx
    @@:
        ffree   st
        mov     edi,[points_ptr]
        mov     edx,[points_count_var]   ; substraction all coords - center point
    @@:
        fld     dword[edi+ebx]
        fsub    .center
        fstp    dword[edi+ebx]
        add     edi,12
;        cmp     dword[edi],-1
;        jne     @b
        dec      edx
        jnz      @b

        add     ebx,4               ; ebx - x,y,z cooficientes in list real_points
        dec     ecx
        jnz     .next_c

        fld     .maxxx
        mov     edi,[points_ptr]   ; create all coords in <-1.0,1.0>
        mov     ecx,[points_count_var]
     @@:
        fld     dword[edi]
        fdiv    .maxxx
        fstp    dword[edi]
        fld     dword[edi+4]
        fdiv    .maxxx
        fstp    dword[edi+4]
        fld     dword[edi+8]
        fdiv    .maxxx
        fstp    dword[edi+8]
        add     edi,12
        loop    @b
     ;   cmp     dword[edi],-1

     ;   jne     @b

        mov     esp,ebp
ret

generate_object:         ; generate node
.N         equ 32
.x         equ word[ebp-2]
.Ndiv2     equ word[ebp-10]
.MthickSqr equ dword[ebp-14]   ; diameter^2
.temp      equ dword[ebp-18]   ; variable for x <-1;1>
.Hthick    equ dword[ebp-22]
.cos_temp  equ dword[ebp-26]
.next_const  equ dword[ebp-30]
.a           equ dword[ebp-34]
.Pi2         equ       ebp-38



       mov      ebp,esp
       sub      esp,42

       mov      .Ndiv2,.N/2
       fninit
       fldpi
       fadd     st,st
       fst      dword[.Pi2]
       fidiv    .Ndiv2
       fst      .a                       ; .Ndiv2*.a=2Pi => .a=2pi/.Ndiv2

       fld      [.Mthick]           ; inside diameter, (outside daiameter = 1)
       fmul     st,st0
       fstp     .MthickSqr
       fld1

       fsub      [.Mthick]

       fst      .Hthick            ; Hthick = 1 - Mthick
       fld      st
       fadd     st,st
       faddp
       fstp     .next_const         ; next_const = Hthick * 3


    ;init triangles list
       mov      edi,[triangles_ptr]

       xor      esi,esi
       xor      eax,eax
       mov      ebx,.N+1
       mov      ecx,(.N*2)+2  ;--
       mov      edx,(.N*3)+3  ;---
       mov      [triangles_count_var],0
     .again_tri:
       stosd                             ; main wave
       mov      dword[edi],ebx
       inc      eax
       add      edi,4
       stosd
       stosd
       mov      dword[edi],ebx
       inc      ebx
       mov      dword[edi+4],ebx


       add      edi,8

       mov      dword[edi],ecx      ;----   ;     n2+2        ;    xor      ax,ax
       inc      ecx                        ;     n2+3        ;    mov      bx,.N+1
       mov      dword[edi+4],edx            ;                 ;    mov      cx,(.N*2)+2  ;--
       mov      dword[edi+8],ecx            ;     n3+3        ;    mov      dx,(.N*3)+3  ;---
       mov      dword[edi+12],edx            ;     n3+3        ;
       inc      edx                        ;                 ;
       mov      dword[edi+16],edx            ;     n2+3        ;
       mov      dword[edi+20],ecx           ;     n3+4
       add      edi,24              ;----

       dec      eax          ; border of wave
       dec      ebx
       dec      ecx
       dec      edx

       stosd                      ; first border
       inc      eax
       stosd
       mov      dword[edi],edx
       add      edi,4

       mov      dword[edi],edx
       add      edi,4
       stosd
       inc      edx
       mov      dword[edi],edx

       mov     dword[edi+4],ebx      ; second border
       mov     dword[edi+8],ecx
       inc     ebx
       mov     dword[edi+12],ebx

       mov     dword[edi+16],ebx
       mov     dword[edi+20],ecx
       inc     ecx
       mov     dword[edi+24],ecx
       add     edi,28

       add      [triangles_count_var],8     ;10
       inc      esi
       cmp      esi,.N
       jne      .again_tri

       add      eax,((.N+1)*3)+1
       add      ebx,((.N+1)*3)+1
       add      ecx,((.N+1)*3)+1
       add      edx,((.N+1)*3)+1
       xor      esi,esi
       cmp      eax,(.N*13)+13    ;;;(.N*23)+23                ; ax,(.N*13)+13
       jl      .again_tri

     ;  mov      dword[edi],-1          ;      <--- end mark not always in use

    ; init real points list
       mov      .x,-(.N/2)
       mov      edi,[points_ptr]
       lea      esi,[edi+(12*(.N+1))]
       mov      eax,[points_ptr]
       mov      ebx,eax
       add      eax,2*12*(.N+1)    ;---
       add      ebx,3*12*(.N+1)    ;---
       mov      [points_count_var],0


       .R_P4 equ edi+(4*12*(.N+1))
       .R_P5 equ edi+(5*12*(.N+1))
       .R_P6 equ edi+(6*12*(.N+1))
       .R_P7 equ edi+(7*12*(.N+1))

       .R_P8 equ edi+(8*12*(.N+1))
       .R_P9 equ edi+(9*12*(.N+1))
       .R_P10 equ edi+(10*12*(.N+1))
       .R_P11 equ edi+(11*12*(.N+1))

       .R_P12 equ edi+(12*12*(.N+1))
       .R_P13 equ edi+(13*12*(.N+1))
       .R_P14 equ edi+(14*12*(.N+1))
       .R_P15 equ edi+(15*12*(.N+1))

     @@:
       ; x coordinate
       fild     .x
       fld      st
;;       fmul     .a                  ; st = <-2pi;2pi>  when mul .a
       fidiv    .Ndiv2
       fst      .temp                 ; temporary x in <-1.0;1.0>

       fst      dword[edi]              ;x coordinate of point
       fst      dword[esi]
       fst      dword[eax]              ;--


       fst      dword[.R_P4]
       fst      dword[.R_P5]
       fst      dword[.R_P6]
       fst      dword[.R_P7]

       fst      dword[.R_P8]
       fst      dword[.R_P9]
       fst      dword[.R_P10]
       fst      dword[.R_P11]

       fst      dword[.R_P12]
       fst      dword[.R_P13]
       fst      dword[.R_P14]
       fst      dword[.R_P15]

       fstp     dword[ebx]  ;pop
      ;*******y coord               dword[offset + 4]
       fmul      .a      ;  st  = <-2pi;2pi>
       fsincos
       fmul     .next_const
       fst      dword[edi+4]            ; y coordinate of point
       fst      dword[esi+4]
       fst      dword[.R_P4+4]
       fst      dword[.R_P5+4]
       fld      .Hthick
       faddp
       fst      dword[.R_P6+4]
       fst      dword[.R_P7+4]
       fst      dword[eax+4]
       fst      dword[ebx+4]

       fchs
       fst      dword[.R_P10+4]
       fst      dword[.R_P11+4]
       fst      dword[.R_P14+4]
       fst      dword[.R_P15+4]
       fadd     .Hthick
       fadd     .Hthick
       fst      dword[.R_P8+4]
       fst      dword[.R_P9+4]
       fst      dword[.R_P12+4]
       fstp     dword[.R_P13+4]


       fmul     .Hthick
       fmul     .next_const
       fstp     .cos_temp           ; cos_temp = Hthick^2 * 3

     ;***************z coord
       fld      .temp
       fld      st
       fmul     st,st0                  ; z coords
       fchs
       fld1
       faddp
       fabs
       fsqrt
;       fld      st
;       fsub
       fld      st
       fsub     .cos_temp
       fst      dword[esi+8]
       fstp     dword[eax+8]            ;--
       fld      st
       fadd     .cos_temp
       fst      dword[.R_P9+8]
       fstp     dword[.R_P10+8]
       fchs
       fld      st
       fsub     .cos_temp
       fst      dword[.R_P6+8]
       fstp     dword[.R_P5+8]
       fadd     .cos_temp
       fst      dword[.R_P13+8]
       fstp     dword[.R_P14+8]

       fmul     [.Mthick]
       fmul     st,st0
       fchs
       fld      .MthickSqr
       faddp
       fabs
       fsqrt
       fld      st
       fsub     .cos_temp
       fst      dword[edi+8]            ;  z coordinate
       fstp     dword[ebx+8]            ;--
       fld      st
       fadd     .cos_temp
       fst      dword[.R_P8+8]
       fstp     dword[.R_P11+8]
       fchs
       fld      st
       fsub     .cos_temp
       fst      dword[.R_P7+8]
       fstp     dword[.R_P4+8]
       fadd     .cos_temp
       fst      dword[.R_P12+8]
       fstp     dword[.R_P15+8]

       add      edi,12
       add      esi,12
       add      eax,12                  ;--
       add      ebx,12                  ;---
       add      [points_count_var],24         ;16
       inc      .x
       cmp      .x,.N/2
       jng      @b
;       mov      dword[esi],-1       ; <-- end mark
       mov       [culling_flag],0
       mov     esp,ebp
ret
.Mthick dd  0.85     ; size-thickness

make_random_lights:
  .temp1  equ ebp-4
  .temp2  equ ebp-8     ;  - light vector generate variables
  .temp3  equ ebp-12
  .max equ 800
        RDTSC
        mov    [rand_seed],ax
        push    ebp
        mov     ebp,esp
        sub     esp,12
        mov     edi,lights
        fninit
        mov     dword[.temp2],.max
        mov     dword[.temp3],.max/2

    .again:
        xor     esi,esi
     @@:
        mov     edx,.max
        xor     ecx,ecx
        call    random
        sub     eax,.max/2
        mov     dword[.temp1],eax
        fild    dword[.temp1]
        fidiv   dword[.temp3]
        fstp    dword[edi+esi*4]
        inc     esi
        cmp     esi,2
        jne     @b

  .max1 equ 1000
        mov     dword[.temp2],.max1/2
        mov     edx,.max1
        xor     ecx,ecx
        call    random
        mov     dword[.temp1],eax
        fild    dword[.temp1]
        fchs
        fidiv   dword[.temp2]
        fstp    dword[edi+8]

        xor     esi,esi
     @@:
        mov     ecx,220            ; max colors and shine  , ecx = 200 - more bright shading
        mov     edx,255
        call    random
        mov     byte[edi+18+esi],al
        inc     esi
        cmp     esi,4
        jne     @b


        xor     esi,esi
     @@:
        mov     ecx,100              ; orginal colors
        movzx   edx,byte[edi+18+esi]
        call    random
        mov     byte[edi+12+esi],al
        inc     esi
        cmp     esi,3
        jne     @b

        xor     esi,esi
     @@:
        mov     ecx,1               ; min cols
        movzx   edx,byte[edi+12+esi]
        call    random
        mov     byte[edi+15+esi],al
        inc     esi
        cmp     esi,3
        jne     @b

        add     edi,LIGHT_SIZE ;22
        cmp     edi,lightsend     ; see file View3ds,asm
        jne     .again

        mov     esp,ebp
        pop     ebp
ret

generate_texture2:
.const equ 32
        mov     edi,texmap
        xor     bx,bx
     .next_line:
        xor     dx,dx
     .next2stripes:
        mov     eax,-1
        mov     ecx,(TEX_X/.const)*3/4
        rep     stosd
        mov     eax,0x00ff0000
        mov     ecx,(TEX_X/.const)
     @@:
        stosd
        dec     edi
        loop    @b
        inc     dx
        cmp     dx,.const/2
        jl      .next2stripes
        inc     bx
        cmp     bx,TEX_Y
        jl      .next_line
ret

blur_screen:    ;blur n times  ; blur or fire
;in - ecx  times count
;     ax - render mode

 .val      equ  dword[ebp-4]
 .dr_model equ  word[ebp-6]
 .fire     equ  dword[ebp-10]

if Ext>=SSE2
         push       ebp
         mov        ebp,esp
         sub        esp,10
      ;   xorps      xmm5,xmm5
      ;   or         edx,edx
      ;   jz         @f
         mov        .val,0x01010101
         movss      xmm5,.val
         shufps     xmm5,xmm5,0
      @@:
         mov        .dr_model,ax


     .again_blur:
         push       ecx
         mov        edi,[screen_ptr]
         movzx      ecx,word[size_x_var]  ;SIZE_X*3/4
         cmp         .dr_model,11
         jge        @f
         lea        ecx,[ecx*3+3]
         shr        ecx,2
       @@:

         xor        eax,eax
         rep        stosd
   if 1
         movzx      ebx,word[size_x_var]
         movzx      ecx,word[size_y_var]
         sub        ecx,3
         imul       ecx,ebx
         cmp        .dr_model,11  ; 32 bit per pix cause
         jge        @f
         lea        ecx,[ecx*3]
         shr        ecx,4
         lea        ebx,[ebx*3]
         jmp        .blr
       @@:

         shr        ecx,2
         shl        ebx,2
       ;  mov        ecx,(SIZE_X*(SIZE_Y-3))*3/16
     .blr:
     @@:
         push       ecx
         movups     xmm0,[edi+ebx]
         mov        ecx,edi
         sub        ecx,ebx
         movups     xmm1,[ecx]
         cmp        .dr_model,12
         jge        @f
         movups     xmm2,[edi-3]
         movups     xmm3,[edi+3]
         jmp        .f
        @@:
         movups     xmm2,[edi-4]
         movups     xmm3,[edi+4]
        .f:
         pavgb      xmm0,xmm1
         pavgb      xmm2,xmm3
         pavgb      xmm0,xmm2
         psubusb    xmm0,xmm5  ; importand if fire
         movups     [edi],xmm0
         add        edi,16
         add        esi,16
         pop        ecx
         loop       .blr
    end if
         xor        eax,eax
         movzx      ecx,word[size_x_var]
         cmp        .dr_model,11
         jge        @f
         lea        ecx,[ecx*3+3]
         shr        ecx,2
        @@:
    ;     mov        ecx,SIZE_X*3/4
         rep        stosd
         pop        ecx
         dec        ecx
         jnz       .again_blur
         mov        esp,ebp
         pop        ebp
end if

if Ext=SSE
         emms
         push       ebp
         mov        ebp,esp
         push       dword 0x01010101
         push       dword 0x01010101
         movq       mm4,[esp]
     .again_blur:
         push       ecx
         mov        edi,[screen_ptr]
         movzx      ecx,word[size_x_var]  ;SIZE_X*3/4
         lea        ecx,[ecx*3]
         shr        ecx,2
     ;    pxor       mm5,mm5
         xor        eax,eax
         rep        stosd
         movzx      ebx,word[size_x_var]
         movzx      ecx,word[size_y_var]
         sub        ecx,3
         imul       ecx,ebx
         lea        ecx,[ecx*3]
         shr        ecx,3
         lea        ebx,[ebx*3]
       ;  mov        ecx,(SIZE_X*(SIZE_Y-3))*3/8
     .blr:
     @@:
         movq       mm0,[edi+ebx]
         movq       mm1,[edi-ebx]
         movq       mm2,[edi-3]
         movq       mm3,[edi+3]

         pavgb      mm0,mm1
         pavgb      mm2,mm3
         pavgb      mm0,mm2

         psubusb    mm0,mm4  ; importand if fire

         movq       [edi],mm0
         add         edi,8
         add         esi,8

         loop        .blr

         xor        eax,eax
         mov        ecx,[size_x_var]
         lea        ecx,[ecx*3]
         shr        ecx,2
       ;  SIZE_X*3/4
         rep        stosd
         pop         ecx
         loop       .again_blur
         mov        esp,ebp
         pop        ebp
end if

if Ext=MMX
         emms
         push       ebp
         mov        ebp,esp
         push       dword 0x0
         push       dword 0x01010101
.again_blur:
         push       ecx
         mov        edi,[screen_ptr]
         mov        ecx,SIZE_X*3/4
         pxor       mm5,mm5
         xor        eax,eax
         rep        stosd

         mov        ecx,(SIZE_X*(SIZE_Y-3))*3/4
     .blr:
     @@:

         movd       mm0,[edi+SIZE_X*3]
         movd       mm1,[edi-SIZE_X*3]
         movd       mm2,[edi-3]
         movd       mm3,[edi+3]

         punpcklbw  mm0,mm5
         punpcklbw  mm1,mm5
         punpcklbw  mm2,mm5
         punpcklbw  mm3,mm5
         paddw      mm0,mm1
         paddw      mm0,mm2
         paddw      mm0,mm3
         psrlw      mm0,2

         packuswb   mm0,mm5
         psubusb    mm0,qword[esp]  ; importand if fire
         movd       eax,mm0
         stosd

         loop        .blr

         xor        eax,eax
         mov        ecx,SIZE_X*3/4
         rep        stosd
         pop         ecx
         loop       .again_blur
         mov        esp,ebp
         pop        ebp
end if
if  Ext=NON
      .blur:
         push   ecx
         xor    ecx,ecx
      .next_col_coof:
         xor    esi,esi
         xor    eax,eax
         xor    ebx,ebx
         mov    edi,SIZE_X*SIZE_Y
      .next:
         mov    ebp,esi
         dec    ebp

         cmp    ebp,SIZE_X*SIZE_Y-1   ; clipping
         jl     @f
         mov    ebp,SIZE_X*SIZE_Y-1
      @@:
         or     ebp,ebp
         jg     @f
         xor    ebp,ebp
      @@:
         lea    edx,[ebp*3+screen]
         mov    al,byte[edx+ecx]

         mov    ebp,esi
         inc    ebp
         cmp    ebp,SIZE_X*SIZE_Y-1   ; clipping
         jl     @f
         mov    ebp,SIZE_X*SIZE_Y-1
      @@:
         or     ebp,ebp
         jg     @f
         xor    ebp,ebp
      @@:
         lea    edx,[ebp*3+screen]
         mov    bl,byte[edx+ecx]
         add    eax,ebx

         mov    ebp,esi
         sub    ebp,SIZE_X
         cmp    ebp,SIZE_X*SIZE_Y-1   ; clipping
         jl     @f
         mov    ebp,SIZE_X*SIZE_Y-1
      @@:
         or     ebp,ebp
         jg     @f
         xor    ebp,ebp
      @@:
         lea    edx,[ebp*3+screen]
         mov    bl,byte[edx+ecx]
         add    eax,ebx

         mov    ebp,esi
         add    ebp,SIZE_X
         cmp    ebp,SIZE_X*SIZE_Y-1   ; clipping
         jl     @f
         mov    ebp,SIZE_X*SIZE_Y-1
      @@:
         or     ebp,ebp
         jg     @f
         xor    ebp,ebp
      @@:
         lea    edx,[ebp*3+screen]
         mov    bl,byte[edx+ecx]
         add    eax,ebx

         shr    eax,2
         lea    edx,[esi*3+screen]
         or     al,al
         jz     @f
         dec    al              ; not importand if fire
         mov    byte[edx+ecx],al
     @@:

         inc    esi
         dec    edi
         jnz    .next

         inc    ecx
         cmp    ecx,3
         jne    .next_col_coof
         pop    ecx
         dec     ecx
         jnz    .blur
end if
ret

mirror:                               ; mirror effect - loseless operation
; in ah - button id = 11, 12, 13
         mov      edi,[points_ptr]     ; one real point - triple float
         mov      esi,[points_normals_ptr]   ; one 3dvector - triple float dword x,y,z
         fninit
         mov      ecx,[points_count_var]

         cmp      ah,11
         je       @f
         cmp      ah,12
         je       .yn
         cmp      ah,13
         je       .zn

       @@:                              ; neg x
         fld      dword[edi]    ;x
         fchs
         fstp     dword[edi]    ;x
         fld      dword[esi]
         fchs
         fstp     dword[esi]
         add      edi,12
         add      esi,12
         loop     @b
         ret
       .yn:
         fld      dword[edi+4]  ;y
         fchs
         fstp     dword[edi+4]  ;y
         fld      dword[esi+4]
         fchs
         fstp     dword[esi+4]

         add      edi,12
         add      esi,12
         loop     .yn
         ret
      .zn:
         fld      dword[edi+8]    ;z
         fchs
         fstp     dword[edi+8]    ;z
         fld      dword[esi+8]
         fchs
         fstp     dword[esi+8]

         add      edi,12
         add      esi,12
         loop     .zn
ret

exchange:                             ; exchange some coords - loseless operation
         mov      edi,[points_ptr]     ; one real point - triple float
         mov      esi,[points_normals_ptr]  ; one 3dvector - triple float dword x,y,z
         fninit                       ; exchange both points and normal vactors coords/coofics
         mov      ecx,[points_count_var]

         cmp      [xchg_flag],1
         je       @f
         cmp      [xchg_flag],2
         je       .zx
         cmp      [xchg_flag],3
         je       .yz
       @@:
         fld      dword[edi]    ;x
         fld      dword[edi+4]  ;y
         fstp     dword[edi]    ;x
         fstp     dword[edi+4]  ;y
         fld      dword[esi]    ;x
         fld      dword[esi+4]  ;y
         fstp     dword[esi]    ;x
         fstp     dword[esi+4]  ;y

         add      esi,12
         add      edi,12
         loop     @b
         ret
       .zx:
         fld      dword[edi]    ;x
         fld      dword[edi+8]  ;z
         fstp     dword[edi]    ;x
         fstp     dword[edi+8]  ;z
         fld      dword[esi]    ;x
         fld      dword[esi+8]  ;y
         fstp     dword[esi]    ;x
         fstp     dword[esi+8]  ;y

         add      esi,12
         add      edi,12
         loop     .zx
         ret
      .yz:
         fld      dword[edi+8]    ;z
         fld      dword[edi+4]    ;y
         fstp     dword[edi+8]    ;z
         fstp     dword[edi+4]    ;y
         fld      dword[esi+8]    ;x
         fld      dword[esi+4]    ;y
         fstp     dword[esi+8]    ;x
         fstp     dword[esi+4]    ;y

         add      edi,12
         add      esi,12
         loop     .yz
ret

;#\\\\\\\\\\\\\\\\\\\\\\\\\comented///////////////////////////////
if 0
calc_attenuation_light:          ;; calculate point to spot_light distance
; spot light with attenuation    ;; and vector, normalize vector,
                                 ;; calc dot_pr and unlinear color according
                                 ;; to dot_product, write to color buff
.distance equ dword[ebp-4]       ;; color buff in bumpmap for save the mem
.temp_col equ word[ebp-6]
.vector   equ [ebp-20]
.spot_light_ptr equ dword [ebp-24]
        mov     ebp,esp
        sub     esp,24
        mov     edi,rotated_points_r  ;points_rotated
        mov     edx,point_normals_rotated
        mov     ecx,bumpmap        ; mem area with temp points color list
        xor     ax,ax              ; counter
        mov     esi,spot_light_params
        mov     .spot_light_ptr,esi
     .again_color:
        push    eax
        lea     ebx,.vector
        mov     esi,.spot_light_ptr ; calc vector fom light to every point
        call    make_vector_r
        ; ebx - ptr to result vector
        fld     dword [ebx]
        fmul    st, st
        fld     dword [ebx+4]
        fmul    st, st
        fld     dword [ebx+8]
        fmul    st, st
        faddp   st1, st
        faddp   st1, st
        fsqrt
        fstp    .distance
        push    edi
        mov     edi,ebx
        call    normalize_vector
        ; edi - normalized distance vector
        mov     esi,edx
        call    dot_product  ; esi first vector, edi second vector
        ; st0 - dot product
        fabs    ; why not ? - think about it
        pop     edi
        fldz
        fcomip  st1
        jbe     @f           ; st1>0
        mov     dword[ecx],0
        mov     word[ecx+4],0
        add     ecx,6
        ffree   st0
        jmp     .update_counters
  @@:
     ;   pop     edi

        ; calc color(with atenuation), write to buff
        ; buff - color of points list
        ; color = ambient+cos(x)*diffuse+(cos(x)^n)*specular

        push    edx
        push    edi

        push    ecx
        push    ebp

;        mov     eax,spot_light_params
        mov     eax,.spot_light_ptr
        movzx   dx,byte[eax+15]
        push    dx             ; shines
        movzx   dx,byte[eax+8] ; b
        push    dx             ; orginal col
        movzx   dx,byte[eax+7] ; g
        push    dx
        movzx   dx,byte[eax+6]   ; r
        push    dx
        movzx   dx,byte[eax+14] ; max col
        push    dx
        movzx   dx,byte[eax+13]
        push    dx
        movzx   dx,byte[eax+12]
        push    dx
        movzx   dx,byte[eax+11] ; min col
        push    dx
        movzx   dx,byte[eax+10]
        push    dx
        movzx   dx,byte[eax+9]
        push    dx
        push    eax            ; dot pr.  (in st0)
        call    calc_one_col
        ; eax - 0x00rrggbb
        ; brightness = 1 - (distance/light.fadezero)^fogness
        ; if brightness  < 0, then brightness = 0
        ; attenuetion equation taken from 3dica tutorial - 1/d^2 isn't perfect
        ; color = color * brightness  ;  fogness = <0.5,2.0>
        pop     ebp
        pop     ecx

        fld    .distance
        mov     esi,.spot_light_ptr
;        fidiv   word[spot_light_params+16]   ; fadezero
        fidiv   word[esi+16]   ; fadezero
      ;  fmul    st,st0       ; fogness = 2
        fabs                  ; to be sure
        fchs
        fld1
        faddp
        fld1
        fcomip  st1
        jnbe    @f
        ffree   st0
        fld1
      @@:
        fld     st            ; st - brightness
        ror     eax,16
        movzx   bx,al        ; al - r
        mov     .temp_col,bx
        fimul   .temp_col
        fistp   word[ecx]
        cmp     word[ecx],0
        jge     @f
        mov     word[ecx],0
    @@:
   ;     mov     edx,dword[spot_light_params+12]  ; max colors
        mov     edx,dword[esi+12]  ; max colors
        movzx   bx,dl         ; r max
        cmp     word[ecx],bx  ; choose the brightest for r, g, b
        jl      @f
        mov     word[ecx],bx
    @@:

        add     ecx,2
        fld     st
        ror     eax,16
        movzx   bx,ah        ; g
        mov     .temp_col,bx
        fimul   .temp_col
        fistp   word[ecx]
        cmp     word[ecx],0
        jg      @f
        mov     word[ecx],0
    @@:
        movzx   bx,dh        ; g max
        cmp     word[ecx],bx
        jle     @f
        mov     word[ecx],bx
    @@:

        add     ecx,2
        movzx   bx,al        ; b
        mov     .temp_col,bx
        fimul   .temp_col
        fistp   word[ecx]
        cmp     word[ecx],0
        jg      @f
        mov     word[ecx],0
    @@:
        shr     edx,16
        movzx   bx,dl        ; b max
        cmp     word[ecx],bx
        jle     @f
        mov     word[ecx],bx
    @@:
        add     ecx,2
;end if
;        ror     eax,16
;        movzx   bx,al
;        mov     word[ecx],bx
;        ror     eax,16
;        movzx   bx,ah
;        mov     word[ecx+2],bx
;        xor     ah,ah
;        mov     word[ecx+4],ax
;        add     ecx,6

        pop    edi
        pop    edx

    .update_counters:
        add     edx,12  ; normal_size
        add     edi,12   ;6   ; 3d point_coord_size

        pop     eax
        inc     ax
        cmp     ax,[points_count_var]
        jne     .again_color

        add     .spot_light_ptr,18
        cmp     .spot_light_ptr,spot_l_end
        jl      .again_color

        mov     esp,ebp
ret
end if
;#\\\\\\\\\\\\\\\\\\\\\\\\\comented////////////////////////////////////