ROUND2 equ 10

stencil_tri:
;  procedure calculate triangle in stencil buffer
;  ----------------in - eax - x1 shl 16 + y1 ----------------------
;  -------------------- ebx - x2 shl 16 + y2 ----------------------
;  -------------------- ecx - x3 shl 16 + y3 ----------------------
;  -------------------- esi - pointer to s-buffer -----------------
;  -------------------- xmm0 - lo -> hi z1, z2, z3 as dword float

.x1     equ [ebp-2]
.y1     equ [ebp-4]
.x2     equ [ebp-6]
.y2     equ [ebp-8]
.x3     equ [ebp-10]
.y3     equ [ebp-12]

.dx12   equ dword[ebp-20]
.dz12   equ dword[ebp-24]
.dx13   equ dword[ebp-28]
.dz13   equ dword[ebp-32]
.dx23   equ dword[ebp-36]
.dz23   equ dword[ebp-40]
.zz2    equ [ebp-44]
.zz1    equ [ebp-48]
.z3     equ [ebp-56]
.z2     equ [ebp-60]
.z1     equ [ebp-64]
;.s_buff equ [ebp-68]

       push    ebp
       mov     ebp,esp
     ;  sub     esp,128
     ;  and     ebp,0xfffffff0
      .sort2:
        cmp     ax,bx
        jle     .sort1
        xchg    eax,ebx
        shufps  xmm0,xmm0,11100001b
      .sort1:
        cmp     bx,cx
        jle     .sort3
        xchg    ebx,ecx
        shufps  xmm0,xmm0,11011000b
        jmp     .sort2
      .sort3:
      ;  mov     .y1,eax            ; store triangle coordinates in user friendly variables
      ;  mov     .y2,ebx
      ;  mov     .y3,ecx
        push     eax
        push     ebx
        push     ecx
        sub      esp,60


    ;   mov     edx,100.11
    ;   movd    xmm0,edx
    ;   shufps  xmm0,xmm0,11100000b

        movups  .z1,xmm0
    ;    mov     dword .z1,edx
    ;    mov     .z2,edx
    ;    mov     .z3,edx
 ;       mov     .s_buff,esi

         mov      edx,80008000h  ; eax,ebx,ecx are ANDd together into edx which means that
         and      edx,ebx        ; if *all* of them are negative a sign flag is raised
         and      edx,ecx
         and      edx,eax
         test     edx,80008000h  ; Check both X&Y at once
         jne      .loop2_end

        mov     bx,.y2       ; calc delta 12
        sub     bx,.y1
        jnz     .dx12_make
        mov     .dx12,0
        mov     .dz12,0
        jmp     .dx12_done
   .dx12_make:
        mov     ax,.x2
        sub     ax,.x1
        cwde
        movsx   ebx,bx
        shl     eax,ROUND2
        cdq
        idiv    ebx
        mov     .dx12,eax

        movss    xmm1,.z2
        cvtsi2ss xmm2,ebx
        subss    xmm1,.z1
        divss    xmm1,xmm2
        movss    .dz12,xmm1
     ;  mov      .dz12, dword 0.11

  .dx12_done:
        mov     bx,.y3       ; calc delta 13
        sub     bx,.y1
        jnz     .dx13_make
        mov     .dx13,0
        mov     .dz13,0
        jmp     .dx13_done
   .dx13_make:
        mov     ax,.x3
        sub     ax,.x1
        cwde
        movsx   ebx,bx
        shl     eax,ROUND2
        cdq
        idiv    ebx
        mov     .dx13,eax

        movss    xmm1,.z3
        cvtsi2ss xmm2,ebx
        subss    xmm1,.z1
        divss    xmm1,xmm2
        movss    .dz13,xmm1
    ;    mov  .dz13, dword 0.11

    .dx13_done:
        mov     bx,.y3       ; calc delta 23
        sub     bx,.y2
        jnz     .dx23_make
        mov     .dx23,0
        mov     .dz23,0
        jmp     .dx23_done
   .dx23_make:
        mov     ax,.x3
        sub     ax,.x2
        cwde
        movsx   ebx,bx
        shl     eax,ROUND2
        cdq
        idiv    ebx
        mov     .dx23,eax

        movss    xmm1,.z3
        cvtsi2ss xmm2,ebx
        subss    xmm1,.z2
        divss    xmm1,xmm2
        movss    .dz23,xmm1

    .dx23_done:
        mov     edx,.z1

        mov     .zz1,edx
        mov     .zz2,edx
        movsx   eax,word .x1
        shl     eax,ROUND2    ; eax - cur x1
        mov     ebx,eax       ; ebx - cur x2

        mov     cx,.y1
        cmp     cx,.y2
        jge     .loop1_end
     .loop1:

        pushad
        sar     ebx,ROUND2
        sar     eax,ROUND2
     ;   movq    xmm0,.zz1
        movlps  xmm0,.zz1
   ;     mov     edx,0.11
   ;     movd    xmm0,edx
   ;     shufps  xmm0,xmm0,11100000b
;        mov     esi,.s_buff

        call    stencil_line

        popad
        add     eax,.dx13
        add     ebx,.dx12

        movss   xmm1,.zz1
        movss   xmm2,.zz2
        addss   xmm1,.dz13
        addss   xmm2,.dz12
        movss   .zz1,xmm1
        movss   .zz2,xmm2

        add     cx,1
        cmp     cx,.y2
        jl      .loop1

    .loop1_end:

        mov     edx,.z2
        mov     .zz2,edx
        movsx   ebx,word .x2
        shl     ebx,ROUND2

        mov     cx,.y2
        cmp     cx,.y3
        jge     .loop2_end
     .loop2:
        pushad

        sar     ebx,ROUND2
        sar     eax,ROUND2
        movlps  xmm0,.zz1
;        mov     esi,.s_buff


        call    stencil_line

        popad

        add     eax,.dx13
        add     ebx,.dx23

        movss   xmm1,.zz1
        movss   xmm2,.zz2
        addss   xmm1,.dz13
        addss   xmm2,.dz23
        movss   .zz1,xmm1
        movss   .zz2,xmm2


        add     cx,1
        cmp     cx,.y3
        jl      .loop2
     .loop2_end:

        mov  esp,ebp
        pop  ebp

ret

stencil_line:
;----------------------------------------------------
;-------------in xmm0 - lo -> hi z1, z2
;--------------- eax - x1 ---------------------------
;--------------- ebx - x2 ---------------------------
;--------------- ecx - y-----------------------------
;--------------- esi - pointer to s-buffer


.dz equ  [ebp-4]
.z2 equ  [ebp-8]
.z1 equ  [ebp-12]
.x2 equ  [ebp-16]
.x1 equ  [ebp-20]
.s_buf  equ [ebp-24]
;     cmp        eax,ebx
;     je  @f
;     int3
;   @@:
     push       ebp
     mov        ebp,esp
     sub        esp,64
  ;   cmp        eax,0
  ;   jg         @f
  ;
  ; @@:
     or         cx,cx
     jle        .l_quit

     cmp        cx,[size_y_var]
     jge        .l_quit

     movzx      ecx,cx
     mov        .s_buf,esi
     cmp        eax,ebx
     je         .l_quit
     jl         .l_ok

     xchg       eax,ebx
     shufps     xmm0,xmm0,11100001b
   .l_ok:

     cmp        ax,[size_x_var]
     jge        .l_quit
     cmp        bx,0
     jle        .l_quit

     movlps     .z1,xmm0
     mov        .x1,eax
     mov        .x2,ebx

     sub        ebx,eax
     movss      xmm0,.z2
     cvtsi2ss   xmm1,ebx
     subss      xmm0,.z1
     divss      xmm0,xmm1
     movss      .dz,xmm0

     movzx      edx,word[size_x_var]
     cmp        eax,1
     jge        @f
     mov        eax,.x1
     neg        eax
     cvtsi2ss   xmm2,eax
     mulss      xmm2,.dz
     addss      xmm2,.z1
     movss      .z1,xmm2
     mov        dword .x1,0
     movzx      edx,word[size_x_var]
     sub        edx,1
   @@:
     cmp        .x2,edx
     jl         @f
     mov        .x2,edx

    @@:
 ;   mov         eax,.x1
 ;   cmp         .x2,eax
 ;   je          .l_quit
     movzx      edx,word[size_x_var]
     mov        esi,.s_buf
     mov        eax,ecx  ; y
     mul        edx
     add        eax,.x1

     shl        eax,2
     add        esi,eax

     mov        ecx,.x2
     sub        ecx,.x1
     movss      xmm2,.z1   ; cz
   .ccalc:
   ;  movss     xmm1,xmm2
   ;  cmpltss   xmm1,dword[esi]
   ;  movd      eax,xmm1
   ;  cmp       eax,-1
     comiss    xmm2,[esi]
     ja        @f
     movss     dword[esi],xmm2
    @@:
     add       esi,4
     addss     xmm2,.dz
;     sub       ecx,1
;     jnz       .ccalc
    loop       .ccalc
  .l_quit:
     mov       esp,ebp
     pop       ebp
ret
