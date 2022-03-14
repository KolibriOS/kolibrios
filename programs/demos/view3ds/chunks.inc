;=========================================================================
detect_chunks:
; make pivot table, sort, remove unused vertices, find chunks...
; in  - some global variables
; out:
;      ebx - chunks list ptr, every chunk as word
;      ecx - chunks number
;      esi - tri_ch    ; vertices with triangles list
;      edi - t_ptr     ; pointers to tri_ch list

    push ebp
    mov  ebp,esp
    sub  esp,60


   .tri_ch         equ  dword[ebp-4]    ; tri chunks list ptr
   .chunks         equ  dword[ebp-8]    ; chunks ptreach tri chunk No. as word
   .endt           equ  dword[ebp-12]   ;
   .t_ptr          equ  dword[ebp-16]   ; pointers list
   .tri_ch1        equ  dword[ebp-20]   ;
   .up             equ  dword[ebp-24]   ; upload ptr
   .chmr           equ  dword[ebp-28]   ; bit mark list if tri stored
   .str            equ  dword[ebp-32]   ; store ptr
   .ltch1          equ  dword[ebp-36]   ; end of tri_ch1 ptr
   .ch_cnt         equ  dword[ebp-40]
   .cntt           equ  dword[ebp-44]
   .cc             equ  dword[ebp-48]
   .lsparam        equ  dword[ebp-52]
   .fix_mark       equ  dword[ebp-56]
   .endVptr        equ  dword[ebp-60]

   ; some triangles have repeated indices of vertices
   ; check and remove such triangles
   call    remove_dead_tri

   mov     ecx,[triangles_count_var]
   shl     ecx,3
   lea     ecx,[ecx*3]
   add     ecx,100
   mov     eax,68
   mov     ebx,12
   int     0x40
   mov     .tri_ch,eax


   mov     ecx,[triangles_count_var]
   imul    ecx,[i12]
   add     ecx,32
   mov     eax,68
   mov     ebx,12
   int     0x40
   mov     .tri_ch1,eax

   mov     ecx,[points_count_var]
   shl     ecx,2
   add     ecx,1120
   mov     eax,68
   mov     ebx,12
   int     0x40
   mov     .t_ptr,eax

   mov     ecx,[triangles_count_var]
   shl     ecx,1
   add     ecx,20
   mov     eax,68
   mov     ebx,12
   int     0x40
   mov     .chunks,eax

   mov     ecx,[triangles_count_var]
   shr     ecx,3
   add     ecx,20
   mov     eax,68
   mov     ebx,12
   int     0x40
   mov     .chmr,eax     ; chunks mark if bit is set - tri was used

   mov     edi,eax
;   pxor    xmm0,xmm0
   mov     ecx,[triangles_count_var]
   shr     ecx,5
   inc     ecx
   xor     eax,eax
   cld
   rep     stosd
;   shr     ecx,7
;   inc     ecx
; @@:
;   movdqa  [edi],xmm0
;   add     edi,16
;   loop    @b


  mov      eax,[points_count_var]
  imul     eax,[i12]
  add      eax,[points_ptr]
  mov      .endVptr,eax

; make pivot table

   mov       edi,.tri_ch
   mov       esi,[triangles_ptr]
   xor       ecx,ecx
  @@:
   movd      xmm1,ecx
   movq      xmm0,[esi]
   pshufd    xmm1,xmm1,0
   movd      xmm2,[esi+8]
   punpckldq xmm0,xmm1
   punpckldq xmm2,xmm1
   movdqu   [edi],xmm0
   movq     [edi+16],xmm2
   add       esi,12
   add       edi,24
   inc       ecx
   cmp       ecx,[triangles_count_var]
   jnz       @b

;sort

   mov    ebx,.tri_ch
   mov    ecx,[triangles_count_var]
   lea    ecx,[ecx*3]

   mov    esi,ecx
   shl    esi,3
   add    esi,ebx
   mov   .endt,esi

 .ccc:                                ; ebx    - vert index
   mov    eax,[ebx+8]                 ; ebx+4  - tri index
   cmp    eax,[ebx]
   jge    .g
   movq   xmm0,[ebx+8]
   push   ebx
 .c:
   cmp    ebx,esi
   jae    .done
   cmp    ebx,.tri_ch
   jb     .done
   cmp    eax,[ebx]
   jae    .done
   movq   xmm7,[ebx]
   movq   [ebx+8],xmm7
   sub    ebx,8
   jnc    .c
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





  mov    ecx,[points_count_var]
  mov    esi,.tri_ch
  dec    ecx
 .ptC:
  mov     eax,[esi]
  add     esi,8
 .ptCff:
  cmp     esi,.endt
  jae     .dnC
  cmp    eax,[esi]
  je     @f
  lea    ebx,[eax+1]
  cmp    ebx,[esi]
  jne    .movt
  dec    ecx
  jz    .dnC            ; check done
 @@:
  jmp   .ptC

;  jmp  .dnC

 .movt:

  movd    xmm5,esi
  movd    xmm7,ebx
  mov     edi,[esi]
  sub     edi,ebx
  movd    xmm6,edi

 @@:
  cmp     esi,.endt
  jnb     @f
  sub     [esi],edi    ; fix .tri_ch pivot table list
  add     esi,8
  jmp     @b
 @@:


  ;shrink vert
  lea     ebx,[ebx*3]
  shl     ebx,2
  add     ebx,[points_ptr]
  imul    edi,[i12]
  add     edi,ebx

  cmp     edi,.endVptr           ; fix points_r  list
  ja      .dnV
 @@:
  movq    xmm0,[edi]
  movd    xmm1,[edi+8]
  movq    [ebx],xmm0
  movd    [ebx+8],xmm1
  add     edi,12
  add     ebx,12
  cmp     edi,.endVptr           ; fix point_r  list
  jna     @b

 .dnV:

; recalc tri  all indices above ebx - sub edi
  push   ecx

  mov    esi,[triangles_ptr]
  mov    ecx,[triangles_count_var]
  lea    ecx,[ecx*3]
  movd   edi,xmm6
  movd   ebx,xmm7
 .agT:
  cmp    [esi],ebx
  jb     @f
  sub     [esi],edi
 @@:
  add   esi,4
  loop  .agT

  pop   ecx

  movd  esi,xmm5

  sub    [points_count_var],edi

  dec   ecx

  jmp   .ptCff           ; again check sth found


 .dnC:      ; check done



  .do_ch:


;make t_ptr  - table with pointers/adresses


  mov    ecx,[points_count_var]
  mov    esi,.tri_ch
  mov    edi,.t_ptr
  mov    ebx,ecx

  mov    [edi],esi
  add    edi,4
  dec    ecx
  jz     .dn
 .pt:
  mov    eax,[esi]        ; [esi]   - vert ind
  add    esi,8
  cmp    eax,[esi]        ; [esi+4] - tri ind
  je     @f
  mov    [edi],esi
  add    edi,4
  dec    ecx
  jz     .dn
 @@:
  cmp   esi,.endt
  jb    .pt


 .dn:


 ; each dword position in .t_ptr list - adress of corresponding
 ; triangles indices, each triangles from such index contains this
 ; vertice


        mov     eax,[triangles_count_var]
        mov     .cntt,eax   ; temp help cnt
        xor     ecx,ecx
        mov     .cc,ecx
        mov     esi,[triangles_ptr]
        mov     edi,.tri_ch1
        imul    eax,[i12]
        add     eax,edi
        mov     .ltch1,eax    ; last

        mov     .up,esi
        mov     .str,edi
;        mov     edi,.tri_ch1
    .lb1:                     ; nx chunk
        cmp     edi,.ltch1
        jnb     .endl
        mov     edi,.tri_ch1
        mov     .str,edi
        mov     eax,.cc
        mov     edx,.cc
        inc     .cc
        cmp     edx,[triangles_count_var]
        jz      .endl
        shr     eax,3
        and     edx,111b
        add     eax,.chmr

        xor     ebx,ebx
        bt      [eax],edx           ; mark
        jc      @f                  ; tri was stored
        inc     ecx
        or      ebx,1b
        mov     esi,.up
        movdqu  xmm0,[esi]
        movdqu  [edi],xmm0
        add     .str,12
      @@:
        add     .up,12
        or      ebx,ebx
        jz      .lb1

    .lb2:
        mov     eax,[edi]
        mov     edx,[edi] ; edx - vert ind
        shl     eax,2
        add     eax,.t_ptr
        mov     eax,[eax]           ; [eax] -   t ptr
        or      eax,eax
        jz      .endl
    .nxt:

        mov     esi,[eax+4]
        mov     ebx,[eax+4]
        shr     esi,3
        and     ebx,111b
        add     esi,.chmr
        bts     [esi],ebx           ; mark
        jc      @f                  ; tri was stored
        dec     .cntt
        je      .endl
        mov     esi,[eax+4]         ; [eax+4] - tri ind
        add     esi,esi
        add     esi,.chunks
        mov     [esi],cx
        mov     esi,[eax+4]

        imul    esi,[i12]
        add     esi,[triangles_ptr]
        movups  xmm0,[esi]
        mov     esi,.str
        movups  [esi],xmm0
        add     .str,12
      @@:
        add     eax,8
        cmp     edx,[eax]
        je      .nxt
        add     edi,4

        cmp     edi,.str
        jne     .lb2
        jmp     .lb1

    .endl:

        mov     .ch_cnt,ecx


  .end:



;   mov     eax,68
;   mov     ebx,13
;   mov     ecx,.t_ptr
;   int     0x40

;   mov     eax,68
;   mov     ebx,13
;   mov     ecx,.tri_ch
;   int     0x40

   mov     eax,68
   mov     ebx,13
   mov     ecx,.tri_ch1
   int     0x40

   mov     eax,68
   mov     ebx,13
   mov     ecx,.chmr
   int     0x40
; for now free mem - cunks list - unused

;   mov     eax,68
;   mov     ebx,13
;   mov     ecx,.chunks
;   int     0x40



   mov    ebx,.chunks
   mov    ecx,.ch_cnt

   mov  esi,.tri_ch
   mov  edi,.t_ptr


   mov    esp,ebp
   pop    ebp
ret






