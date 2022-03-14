
;CATMULL_SHIFT equ 8
;TEXTURE_SIZE EQU (TEX_X * TEX_Y)-1
;ROUND equ 8
;Ext = NON
;MMX = 1
;NON = 0
;------- Big thanks to Majuma (www.majuma.xt.pl) for absolutely great---
;------- DOS 13h mode demos --------------------------------------------
;------- Procedure draws bump triangle with texture, I use -------------
;--------Catmull Z-buffer algorithm- (Z coordinate interpolation)-------
;--------I calc texture pixel by this way: col1*col2/256 ---------------
bump_tex_triangle_z:
;------------------in - eax - x1 shl 16 + y1 -----------
;---------------------- ebx - x2 shl 16 + y2 -----------
;---------------------- ecx - x3 shl 16 + y3 -----------
;---------------------- edx - pointer to bump map-------
;---------------------- esi - pointer to env map--------
;---------------------- edi - pointer to screen buffer--
;---------------------- stack : bump coordinates--------
;----------------------         environment coordinates-
;----------------------         Z position coordinates--
;----------------------         pointer to Z buffer-----
;----------------------         pointer to texture------
;----------------------         texture coordinates-----
;-- Z-buffer - filled with coordinates as dword --------
;-- (Z coor. as word) shl CATMULL_SHIFT ----------------



.t_y1   equ ebp+4   ; procedure don't save registers !!!
.t_x1   equ ebp+6   ; each coordinate as word
.e_y1   equ ebp+8              ; texture coords
.e_x1   equ ebp+10
.b_y1   equ ebp+12
.b_x1   equ ebp+14



.t_y2   equ ebp+16
.t_x2   equ ebp+18            ; b - bump map coords
.e_y2   equ ebp+20              ; texture coords
.e_x2   equ ebp+22
.b_y2   equ ebp+24
.b_x2   equ ebp+26




.t_y3   equ ebp+28              ; e - env map coords
.t_x3   equ ebp+30
.e_y3   equ ebp+32              ; texture coords
.e_x3   equ ebp+34
.b_y3   equ ebp+36
.b_x3   equ ebp+38

.z1     equ word[ebp+40]
.z2     equ word[ebp+42]
.z3     equ word[ebp+44]
.z_buff equ dword[ebp+46]       ; pointer to Z-buffer
.tex_ptr equ dword[ebp+50]      ; ptr to texture



.t_bmap equ dword[ebp-4]        ; pointer to bump map
.t_emap equ dword[ebp-8]        ; pointer to env map
.x1     equ word[ebp-10]
.y1     equ word[ebp-12]
.x2     equ word[ebp-14]
.y2     equ word[ebp-16]
.x3     equ word[ebp-18]
.y3     equ word[ebp-20]




.dx12   equ dword[ebp-24]
.dz12   equ      [ebp-28]
.dbx12  equ dword[ebp-32]
.dby12  equ      [ebp-36]
.dex12  equ dword[ebp-40]
.dey12  equ      [ebp-44]
.dtx12  equ dword[ebp-48]
.dty12  equ      [ebp-52]

.dx13  equ dword[ebp-52-4*1]
.dz13  equ      [ebp-52-4*2]
.dbx13 equ dword[ebp-52-4*3]
.dby13 equ      [ebp-52-4*4]
.dex13 equ dword[ebp-52-4*5]
.dey13 equ      [ebp-52-4*6]
.dtx13 equ dword[ebp-52-4*7]
.dty13 equ      [ebp-52-4*8]


.dx23  equ dword[ebp-(52+4*9)]
.dz23  equ      [ebp-(52+4*10)]
.dbx23 equ dword[ebp-(52+4*11)]
.dby23 equ      [ebp-(52+4*12)]
.dex23 equ dword[ebp-(52+4*13)]
.dey23 equ      [ebp-(52+4*14)]
.dtx23 equ dword[ebp-(52+4*15)]
.dty23 equ      [ebp-(52+4*16)]



.cx1   equ dword[ebp-(52+4*17)]         ; current variables
.cz1   equ      [ebp-(52+4*18)]
.cbx1  equ dword[ebp-(52+4*19)]
.cby1  equ      [ebp-(52+4*20)]
.cex1  equ dword[ebp-(52+4*21)]
.cey1  equ      [ebp-(52+4*22)]
.ctx1  equ dword[ebp-(52+4*23)]
.cty1  equ      [ebp-(52+4*24)]

.cx2   equ dword[ebp-(52+4*25)]
.cz2   equ      [ebp-(52+4*26)]
.cbx2  equ dword[ebp-(52+4*27)]
.cby2  equ      [ebp-(52+4*28)]
.cex2  equ dword[ebp-(52+4*29)]
.cey2  equ      [ebp-(52+4*30)]
.ctx2  equ dword[ebp-(52+4*31)]
.cty2  equ      [ebp-(52+4*32)]

if Ext >+ MMX
       emms
end if

       cld
       mov     ebp,esp
       push    edx        ; store bump map
       push    esi        ; store e. map
     ; sub     esp,120
 .sort3:                  ; sort triangle coordinates...
       cmp     ax,bx
       jle     .sort1
       xchg    eax,ebx
   if Ext >= MMX
       movq    mm0,[.t_y1]
       movq    mm1,[.t_y2]
       movq    [.t_y1],mm1
       movq    [.t_y2],mm0
    end if
       mov     edx,dword[.b_y1]
       xchg    edx,dword[.b_y2]
       mov     dword[.b_y1],edx
    if Ext = NON
       mov     edx,dword[.e_y1]
       xchg    edx,dword[.e_y2]
       mov     dword[.e_y1],edx
       mov     edx,dword[.t_y1]
       xchg    edx,dword[.t_y2]
       mov     dword[.t_y1],edx
    end if
       mov     dx,.z1
       xchg    dx,.z2
       mov     .z1,dx
 .sort1:
       cmp      bx,cx
       jle      .sort2
       xchg     ebx,ecx
     if Ext >= MMX
       movq    mm0,[.t_y3]
       movq    mm1,[.t_y2]
       movq    [.t_y3],mm1
       movq    [.t_y2],mm0
     end if
       mov      edx,dword[.b_y2]
       xchg     edx,dword[.b_y3]
       mov      dword[.b_y2],edx
     if Ext = NON
       mov      edx,dword[.e_y2]
       xchg     edx,dword[.e_y3]
       mov      dword[.e_y2],edx
       mov      edx,dword[.t_y2]
       xchg     edx,dword[.t_y3]
       mov      dword[.t_y2],edx
     end if
       mov     dx,.z2
       xchg    dx,.z3
       mov     .z2,dx
       jmp      .sort3
 .sort2:
       push     eax     ; store triangle coords in variables
       push     ebx
       push     ecx
         mov      edx,80008000h  ; eax,ebx,ecx are ANDd together into edx which means that
         and      edx,ebx        ; if *all* of them are negative a sign flag is raised
         and      edx,ecx
         and      edx,eax
         test     edx,80008000h  ; Check both X&Y at once
         jne      .loop23_done
    ;   mov     edx,eax         ; eax,ebx,ecx are ORd together into edx which means that
    ;   or      edx,ebx         ; if any *one* of them is negative a sign flag is raised
    ;   or      edx,ecx
    ;   test    edx,80000000h   ; Check only X
    ;   jne     .loop23_done

    ;   cmp     .x1,SIZE_X    ; {
    ;   jg      .loop23_done
    ;   cmp     .x2,SIZE_X     ; This can be optimized with effort
    ;   jg      .loop23_done
    ;   cmp     .x3,SIZE_X
    ;   jg      .loop23_done    ; {


       mov      bx,.y2       ; calc delta 12
       sub      bx,.y1
       jnz      .bt_dx12_make

       mov      ecx,8
       xor      edx,edx
     @@:
       push     edx
       loop     @b

       jmp      .bt_dx12_done
 .bt_dx12_make:
       movsx    ebx,bx
if Ext >= SSE2
       mov      eax,1 shl 15
       cdq
       idiv     ebx
       mov      ebx,eax


       mov      ax,.x2
       sub      ax,.x1
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax

       mov      ax,.z2
       sub      ax,.z1
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax

       sub       esp,4*6
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
       movlhps   xmm0,xmm0
       movdqu    xmm1,[.t_y1]
       movdqu    xmm2,[.t_y2]
       psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       movhlps   xmm4,xmm2
       movhlps   xmm5,xmm3
       punpcklwd xmm2,xmm3
       punpcklwd xmm4,xmm5
       psrad     xmm2,15 - ROUND
       psrad     xmm4,15 - ROUND
       movdqu    .dty12,xmm2
       movq      .dby12,xmm4
else

       mov      ax,.x2
       sub      ax,.x1
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dx12,eax
       push      eax

       mov     ax,.z2
       sub     ax,.z1
       cwde
       shl     eax,CATMULL_SHIFT
       cdq
       idiv    ebx
       push    eax

       mov      ax,word[.b_x2]
       sub      ax,word[.b_x1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dbx12,eax
       push      eax

       mov      ax,word[.b_y2]
       sub      ax,word[.b_y1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dby12,eax
       push      eax

       mov      ax,word[.e_x2]
       sub      ax,word[.e_x1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dex12,eax
       push      eax

       mov      ax,word[.e_y2]
       sub      ax,word[.e_y1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dey12,eax
       push      eax

       mov      ax,word[.t_x2]
       sub      ax,word[.t_x1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dtx12,eax
       push      eax

       mov      ax,word[.t_y2]
       sub      ax,word[.t_y1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dty12,eax
       push      eax
end if
   .bt_dx12_done:

       mov      bx,.y3       ; calc delta13
       sub      bx,.y1
       jnz      .bt_dx13_make
       mov      ecx,8
       xor      edx,edx
     @@:
       push     edx   ;dword 0
       loop     @b
       jmp      .bt_dx13_done
 .bt_dx13_make:
       movsx    ebx,bx

if Ext >= SSE2
       mov      eax,1 shl 15
       cdq
       idiv     ebx
       mov      ebx,eax


       mov      ax,.x3
       sub      ax,.x1
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax
    ;   mov      .dx12,eax

       mov      ax,.z3
       sub      ax,.z1
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax

       sub       esp,4*6
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
       movlhps   xmm0,xmm0
       movdqu    xmm1,[.t_y1]
       movdqu    xmm2,[.t_y3]
       psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       movhlps   xmm4,xmm2
       movhlps   xmm5,xmm3
       punpcklwd xmm2,xmm3
       punpcklwd xmm4,xmm5
       psrad     xmm2,15 - ROUND
       psrad     xmm4,15 - ROUND
       movdqu    .dty13,xmm2
       movq      .dby13,xmm4
else

       mov      ax,.x3
       sub      ax,.x1
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dx13,eax
       push      eax

       mov     ax,.z3
       sub     ax,.z1
       cwde
       shl     eax,CATMULL_SHIFT
       cdq
       idiv    ebx
  ;    mov    .dz13,eax
       push    eax


       mov      ax,word[.b_x3]
       sub      ax,word[.b_x1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dbx13,eax
       push      eax

       mov      ax,word[.b_y3]
       sub      ax,word[.b_y1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dby13,eax
       push      eax

       mov      ax,word[.e_x3]
       sub      ax,word[.e_x1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dex13,eax
       push      eax

       mov      ax,word[.e_y3]
       sub      ax,word[.e_y1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dey13,eax
       push      eax

       mov      ax,word[.t_x3]
       sub      ax,word[.t_x1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dtx13,eax
       push      eax

       mov      ax,word[.t_y3]
       sub      ax,word[.t_y1]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dty13,eax
       push      eax
end if
   .bt_dx13_done:

       mov      bx,.y3       ; calc delta23
       sub      bx,.y2
       jnz      .bt_dx23_make
       mov      ecx,8
       xor      edx,edx
     @@:
       push     edx   ;dword 0
       loop     @b
       jmp      .bt_dx23_done
 .bt_dx23_make:
       movsx    ebx,bx
if Ext >= SSE2

       mov      eax,1 shl 15
       cdq
       idiv     ebx
    ;   push     eax
       mov      ebx,eax


       mov      ax,.x3
       sub      ax,.x2
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax
    ;   mov      .dx12,eax

       mov      ax,.z3
       sub      ax,.z2
       cwde
       imul     ebx
       sar      eax,15 - ROUND
       push     eax

       sub       esp,4*6
       movd      xmm0,ebx
       pshuflw   xmm0,xmm0,0
       movlhps   xmm0,xmm0
       movdqu    xmm1,[.t_y2]
       movdqu    xmm2,[.t_y3]
       psubw     xmm2,xmm1
       movdqa    xmm3,xmm2
       pmullw    xmm2,xmm0
       pmulhw    xmm3,xmm0
       movhlps   xmm4,xmm2
       movhlps   xmm5,xmm3
       punpcklwd xmm2,xmm3
       punpcklwd xmm4,xmm5
       psrad     xmm2,15 - ROUND
       psrad     xmm4,15 - ROUND
       movdqu    .dty23,xmm2
       movq      .dby23,xmm4
else
       mov      ax,.x3
       sub      ax,.x2
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dx23,eax
       push      eax

       mov     ax,.z3
       sub     ax,.z2
       cwde
       shl     eax,CATMULL_SHIFT
       cdq
       idiv    ebx
     ; mov     .dz23,eax
       push    eax

       mov      ax,word[.b_x3]
       sub      ax,word[.b_x2]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dbx23,eax
       push      eax

       mov      ax,word[.b_y3]
       sub      ax,word[.b_y2]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dby23,eax
       push      eax

       mov      ax,word[.e_x3]
       sub      ax,word[.e_x2]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dex23,eax
       push      eax

       mov      ax,word[.e_y3]
       sub      ax,word[.e_y2]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dey23,eax
       push      eax


       mov      ax,word[.t_x3]
       sub      ax,word[.t_x2]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dtx23,eax
       push      eax

       mov      ax,word[.t_y3]
       sub      ax,word[.t_y2]
       cwde
       shl      eax,ROUND
       cdq
       idiv     ebx
 ;     mov      .dty23,eax
       push      eax
end if


   .bt_dx23_done:

       sub       esp,64

       movsx    eax,.x1
       shl      eax,ROUND
       mov      .cx1,eax
       mov      .cx2,eax
  ;     push     eax
  ;     push     eax

       movsx    ebx,word[.b_x1]
       shl      ebx,ROUND
       mov      .cbx1,ebx
       mov      .cbx2,ebx
      ; push     ebx
      ; push     ebx

       movsx    ecx,word[.b_y1]
       shl      ecx,ROUND
       mov      .cby1,ecx
       mov      .cby2,ecx
      ; push     ecx
      ; push     ecx

       movsx    edx,word[.e_x1]
       shl      edx,ROUND
       mov      .cex1,edx
       mov      .cex2,edx
    ;   push     edx
    ;   push     edx

       movsx    eax,word[.e_y1]
       shl      eax,ROUND
       mov      .cey1,eax
       mov      .cey2,eax
    ;   push     eax
    ;   push     eax

       movsx    ebx,.z1
       shl      ebx,CATMULL_SHIFT
       mov      .cz1,ebx
       mov      .cz2,ebx
   ;    push     ebx
   ;    push     ebx

      ; sub      esp,16
       movsx    ecx,word[.t_x1]
       shl      ecx,ROUND
       mov      .ctx1,ecx
       mov      .ctx2,ecx
       ;push     ecx
       ;push     ecx

       movsx    edx,word[.t_y1]
       shl      edx,ROUND
       mov      .cty1,edx
       mov      .cty2,edx
      ; push     edx
      ; push     edx

if 0 ;Ext >= SSE2
       movups  xmm0,.cby1
       movups  xmm1,.cty1
       movups  xmm2,.cby2
       movups  xmm3,.cty2
       movups  xmm4,.dby13
       movups  xmm5,.dty13
       movups  xmm6,.dby12
       movups  xmm7,.dty12
  ;     .scby1  equ [edi]
  ;     .scty1  equ [edi+16]
  ;     .scby2  equ [edi+32]
  ;     .scty2  equ [edi+48]
  ;     .sdby13 equ [edi+64]
  ;     .sdty13 equ [edi+80]
  ;     .sdby12 equ [edi+96]
  ;     .sdty12 equ [edi+128]
  ;     push    edi
       mov     edi,sse_repository
       movaps  .scby1,xmm0
       movaps  .scty1,xmm1
       movaps  .scby2,xmm2
       movaps  .scty2,xmm3
       movaps  .sdby13,xmm4
       movaps  .sdty13,xmm5
       movaps  .sdby12,xmm6
       movaps  .sdty12,xmm7
       pop     edi

end if

       movsx    ecx,.y1
       cmp      cx,.y2
       jge      .loop12_done
  .loop12:
       call     .call_line

if  Ext >= SSE2

       movups  xmm0,.cby1
       movups  xmm1,.cty1
       movups  xmm2,.cby2
       movups  xmm3,.cty2
       movups  xmm4,.dby13
       movups  xmm5,.dty13
       movups  xmm6,.dby12
       movups  xmm7,.dty12
       paddd   xmm0,xmm4
       paddd   xmm1,xmm5
       paddd   xmm2,xmm6
       paddd   xmm3,xmm7
       movups  .cby1,xmm0
       movups  .cty1,xmm1
       movups  .cby2,xmm2
       movups  .cty2,xmm3
end if

if  (Ext = MMX)| (Ext = SSE)
       movq     mm0,.cby2
       movq     mm1,.cby1
       movq     mm2,.cey2
       movq     mm3,.cey1
       movq     mm4,.cty1
       movq     mm5,.cty2
       movq     mm6,.cz1
       movq     mm7,.cz2
       paddd    mm0,.dby12
       paddd    mm1,.dby13
       paddd    mm2,.dey12
       paddd    mm3,.dey13
       paddd    mm4,.dty13
       paddd    mm5,.dty12
       paddd    mm6,.dz13
       paddd    mm7,.dz12
       movq     .cby2,mm0
       movq     .cby1,mm1
       movq     .cey1,mm3
       movq     .cey2,mm2
       movq     .cty1,mm4
       movq     .cty2,mm5
       movq     .cz1,mm6
       movq     .cz2,mm7
end if
if Ext = NON
       mov      edx,.dbx13
       add      .cbx1,edx
       mov      eax,.dbx12
       add      .cbx2,eax
       mov      ebx,.dby13
       add      .cby1,ebx
       mov      edx,.dby12
       add      .cby2,edx

       mov      eax,.dex13
       add      .cex1,eax
       mov      ebx,.dex12
       add      .cex2,ebx
       mov      edx,.dey13
       add      .cey1,edx
       mov      eax,.dey12
       add      .cey2,eax

       mov      eax,.dtx13
       add      .ctx1,eax
       mov      ebx,.dtx12
       add      .ctx2,ebx
       mov      edx,.dty13
       add      .cty1,edx
       mov      eax,.dty12
       add      .cty2,eax

       mov      eax,.dx13
       add      .cx1,eax
       mov      ebx,.dx12
       add      .cx2,ebx
       mov      ebx,.dz13
       add      .cz1,ebx
       mov      edx,.dz12
       add      .cz2,edx
end if
       inc      ecx
       cmp      cx,.y2
       jl       .loop12
    .loop12_done:

       movsx    ecx,.y2
       cmp      cx,.y3
       jge      .loop23_done


       movsx    eax,.z2
       shl      eax,CATMULL_SHIFT
       mov      .cz2,eax

       movsx    ebx,.x2
       shl      ebx,ROUND
       mov      .cx2,ebx

       movzx    edx,word[.b_x2]
       shl      edx,ROUND
       mov      .cbx2,edx

       movzx    eax,word[.b_y2]
       shl      eax,ROUND
       mov      .cby2,eax

       movzx    ebx,word[.e_x2]
       shl      ebx,ROUND
       mov      .cex2,ebx

       movzx    edx,word[.e_y2]
       shl      edx,ROUND
       mov      .cey2,edx

       movzx    eax,word[.t_x2]
       shl      eax,ROUND
       mov      .ctx2,eax

       movzx    ebx,word[.t_y2]
       shl      ebx,ROUND
       mov      .cty2,ebx

     .loop23:
       call     .call_line

if  Ext >= SSE2
;       fxrstor [sse_repository]
       movups  xmm0,.cby1
       movups  xmm1,.cty1
       movups  xmm2,.cby2
       movups  xmm3,.cty2
       movups  xmm4,.dby13
       movups  xmm5,.dty13
       movups  xmm6,.dby23
       movups  xmm7,.dty23
       paddd   xmm0,xmm4
       paddd   xmm1,xmm5
       paddd   xmm2,xmm6
       paddd   xmm3,xmm7
    ;   push    edi
    ;   mov     edi,sse_repository
    ;   paddd   xmm0,.sdby13
    ;   paddd   xmm1,.sdty13
    ;   paddd   xmm2,.sdby12
    ;   paddd   xmm3,.sdty12
    ;   pop     edi
       movups  .cby1,xmm0
       movups  .cty1,xmm1
       movups  .cby2,xmm2
       movups  .cty2,xmm3
end if


if  (Ext = MMX)| (Ext = SSE)
       movq     mm0,.cby2
       movq     mm1,.cby1
       movq     mm2,.cey2
       movq     mm3,.cey1
       movq     mm4,.cty1
       movq     mm5,.cty2
       movq     mm6,.cz1
       movq     mm7,.cz2
       paddd    mm0,.dby23
       paddd    mm1,.dby13
       paddd    mm2,.dey23
       paddd    mm3,.dey13
       paddd    mm4,.dty13
       paddd    mm5,.dty23
       paddd    mm6,.dz13
       paddd    mm7,.dz23
       movq     .cby2,mm0
       movq     .cby1,mm1
       movq     .cey2,mm2
       movq     .cey1,mm3
       movq     .cty1,mm4
       movq     .cty2,mm5
       movq     .cz1,mm6
       movq     .cz2,mm7
end if
If  Ext = NON
       mov      edx,.dbx13
       add      .cbx1,edx
       mov      eax,.dbx23
       add      .cbx2,eax
       mov      ebx,.dby13
       add      .cby1,ebx
       mov      edx,.dby23
       add      .cby2,edx

       mov      eax,.dex13
       add      .cex1,eax
       mov      ebx,.dex23
       add      .cex2,ebx
       mov      edx,.dey13
       add      .cey1,edx
       mov      eax,.dey23
       add      .cey2,eax

       mov      eax,.dx13
       add      .cx1,eax
       mov      ebx,.dx23
       add      .cx2,ebx
       mov      ebx,.dz13
       add      .cz1,ebx
       mov      edx,.dz23
       add      .cz2,edx

       mov      eax,.dtx13
       add      .ctx1,eax
       mov      ebx,.dtx23
       add      .ctx2,ebx
       mov      edx,.dty13
       add      .cty1,edx
       mov      eax,.dty23
       add      .cty2,eax
end if
       inc      ecx
       cmp      cx,.y3
       jl       .loop23
    .loop23_done:

       mov      esp,ebp
ret   50

.call_line:

       pushad
       push     dword .cty1
       push     .ctx1

       push     dword .cz1
       push     dword .cty2
       push     .ctx2

       push     dword .cz2

       push     dword .cey2
       push     .cex2
       push     dword .cby2
       push     .cbx2
       push     dword .cey1
       push     .cex1
       push     dword .cby1
       push     .cbx1


       push     .tex_ptr
       push     .z_buff
       push     .t_emap
       push     .t_bmap

       push     ecx

       mov      eax,.cx1
       sar      eax,ROUND
       mov      ebx,.cx2
       sar      ebx,ROUND

       call     bump_tex_line_z

       popad

ret
bump_tex_line_z:
;--------------in: eax - x1
;--------------    ebx - x2
;--------------    edi - pointer to screen buffer
;stack - another parameters :
.y      equ dword [ebp+4]
.bmap   equ dword [ebp+8]        ; bump map pointer
.emap   equ dword [ebp+12]       ; env map pointer
.z_buff equ dword [ebp+16]       ; z buffer
.tex_map equ dword [ebp+20]      ; texture pointer

.bx1    equ  [ebp+24]   ;   ---
.by1    equ  [ebp+28]  ;       |
.ex1    equ  [ebp+32]  ;       |
.ey1    equ  [ebp+36]  ;       |
.bx2    equ  [ebp+40]  ;       |
.by2    equ  [ebp+44]  ;       |>   b. map and e. map coords
.ex2    equ  [ebp+48]  ;       |>   shifted shl ROUND
.ey2    equ  [ebp+52]  ;   ---
.z2     equ  [ebp+56]
.tx2    equ  [ebp+60]
.ty2    equ  [ebp+64]
.z1     equ  [ebp+68]
.tx1    equ  [ebp+72]
.ty1    equ  [ebp+76]



.x1     equ [ebp-4]
.x2     equ [ebp-8]
.dbx    equ [ebp-12]
.dby    equ [ebp-16]
.dex    equ [ebp-20]
.dey    equ [ebp-24]
.dz     equ [ebp-28]
.dtx    equ [ebp-32]
.dty    equ [ebp-36]

.cbx    equ [ebp-40]
.cby    equ [ebp-44]
.cex    equ [ebp-48]
.cey    equ [ebp-52]
.cz     equ [ebp-56]
.czbuff equ [ebp-60]
.ctx    equ [ebp-64]
.cty    equ [ebp-68]
.c_scr  equ [ebp-72]

.temp1  equ        ebp-80
.temp2  equ        ebp-88
.temp3  equ        ebp-76
.temp4  equ        ebp-84
.temp5  equ        ebp-92

        mov     ebp,esp

        mov     ecx,.y
        or      ecx,ecx
        jl      .bl_end
        movzx   edx,word[size_y_var]
        cmp     ecx,edx  ;SIZE_Y
        jge     .bl_end

        cmp     eax,ebx
        jl      .bl_ok
        je      .bl_end


if Ext=NON
        mov     edx,.bx1
        xchg    edx,.bx2
        mov     .bx1,edx
        mov     edx,.by1
        xchg    edx,.by2
        mov     .by1,edx

        mov     edx,.ex1
        xchg    edx,.ex2
        mov     .ex1,edx
        mov     edx,.ey1
        xchg    edx,.ey2
        mov     .ey1,edx

        mov     edx,.tx1
        xchg    edx,.tx2
        mov     .tx1,edx
        mov     edx,.ty1
        xchg    edx,.ty2
        mov     .ty1,edx
end if
if Ext = MMX
        movq    mm0,.bx1
        movq    mm1,.bx2
        movq    mm2,.ex1
        movq    mm3,.ex2
        movq    mm4,.tx1
        movq    mm5,.tx2
        movq    .bx2,mm0
        movq    .bx1,mm1
        movq    .ex1,mm3
        movq    .ex2,mm2
        movq    .tx1,mm5
        movq    .tx2,mm4
end if
if Ext>=SSE
        movups xmm0,.bx1
        movups xmm1,.bx2
        movups .bx1,xmm1
        movups .bx2,xmm0
        movq    mm0,.tx1
        movq    mm1,.tx2
        movq    .tx1,mm1
        movq    .tx2,mm0
end if

        xchg    eax,ebx
        mov     edx,.z1
        xchg    edx,.z2
        mov     .z1,edx

  .bl_ok:

        push    eax
        push    ebx           ;store x1, x2
        movzx   ebx,word[size_x_var]
    ;    mov     eax,.x1
        cmp     dword .x1,ebx  ;dword .x1,SIZE_X
        jge     .bl_end
        cmp     dword .x2,0
        jle     .bl_end

        mov     ebx,.x2
        sub     ebx,.x1

       mov      eax,1 shl 15
       cdq
       idiv     ebx
       mov      ebx,eax


       mov      eax,.bx2
       sub      eax,.bx1
       sar      eax,ROUND
       imul     ebx
       sar      eax,15 - ROUND
       push     eax




       mov      eax,.by2
       sub      eax,.by1
       sar      eax,ROUND
       imul     ebx
       sar      eax,15 - ROUND
       push     eax



       mov      eax,.ex2
       sub      eax,.ex1
       sar      eax,ROUND
       imul     ebx
       sar      eax,15 - ROUND
       push     eax



       mov      eax,.ey2
       sub      eax,.ey1
       sar      eax,ROUND
       imul     ebx
       sar      eax,15 - ROUND
       push     eax


       mov      eax,.z2
       sub      eax,.z1
       sar      eax,ROUND
       imul     ebx
       sar      eax,15 - ROUND
       push     eax

       mov      eax,.tx2
       sub      eax,.tx1
       sar      eax,ROUND
       imul     ebx
       sar      eax,15 - ROUND
       push     eax


       mov      eax,.ty2
       sub      eax,.ty1
       sar      eax,ROUND
       imul     ebx
       sar      eax,15 - ROUND
       push     eax


        cmp     dword .x1,0         ; set correctly begin variable
        jge     @f            ; CLIPPING ON FUNCTION
                              ; cutting triangle exceedes screen
        mov     ebx,.x1
        neg     ebx

;if Ext >= SSE

;        cvtsi2ss xmm0,ebx
;        shufps   xmm0,xmm0,0
;        movups   xmm1,.dey
;        mulps    xmm1,xmm0
;        shufps   xmm1,xmm1,00011011b
;        movups   xmm2,.bx1
;        addps    xmm2,xmm1
;        movups   .bx1,xmm2

        mov     eax,.dz
        imul    ebx           ; eax = .dz * abs(.x1)
        add     .z1,eax
        mov     dword .x1,0

        mov     eax,.dbx
        imul    ebx
        add    .bx1,eax

        mov     eax,.dby
        imul    ebx
        add     .by1,eax

        mov     eax,.dex
        imul    ebx
        add     .ex1,eax

        mov     eax,.dey
        imul    ebx
        add     .ey1,eax

        mov     eax,.dtx
        imul    ebx
        add     .tx1,eax

        mov     eax,.dty
        imul    ebx
        add     .ty1,eax

      @@:
   ;     mov     ebx,.x2
        movzx   eax,word[size_x_var]
       ; cmp     dword .x2,SIZE_X
        cmp     dword .x2,eax  ; eax,ebx
        jl      @f
        mov     dword .x2,eax  ;SIZE_X
      @@:
        movzx   eax,word[size_x_var]  ;SIZE_X       ;calc memory begin in buffers
        mul     .y
        add     eax,.x1
        lea     esi,[4*eax]
        add     esi,.z_buff       ; z-buffer filled with dd variables
        lea     eax,[eax*3]
        add     edi,eax


        mov     ecx,.x2
        sub     ecx,.x1
        ; init current variables
        push    dword .bx1   ; current b, e and t shifted shl ROUND   .cbx
        push    dword .by1                                         ;  .cby
        push    dword .ex1                                         ;  .cex
        push    dword .ey1                                         ;  .cey

        push    dword .z1    ; current z shl CATMULL_SHIFT         ; .cz
        push    esi                                          ; .czbuff

        push    dword .tx1      ;         .ctx
        push    dword .ty1      ;         .cty
        push    edi       ;         .c_scr
if Ext = SSE2
        mov    eax,TEXTURE_SIZE
        movd   xmm1,eax
        shufps xmm1,xmm1,0
        push   dword  TEX_X
        push   dword  -TEX_X
        push   dword  1
        push   dword  -1
        movups xmm2,[esp]
        movd   xmm3,.bmap
        shufps xmm3,xmm3,0
end if

if Ext>=MMX
        movq    mm7,.cty
        movq    mm6,.cby
        movq    mm5,.cey
;        movq    mm4,.dtyq
;        movq    mm3,.dbyq
end if

     .draw:
    ; if TEX = SHIFTING   ;bump drawing only in shifting mode
        mov     esi,.czbuff      ; .czbuff current address in buffer
        mov     ebx,.cz          ; .cz - cur z position
        cmp     ebx,dword[esi]
        jge     .skip
if Ext=NON
        mov     eax,.cby
        shr     eax,ROUND
        mov     esi,.cbx
        shr     esi,ROUND
else
        movq    mm1,mm6
        psrld   mm1,ROUND
        movd    eax,mm1
        psrlq   mm1,32
        movd    esi,mm1
end if

        shl     eax,TEX_SHIFT
        add     esi,eax         ;-  ; esi - current bump map index

if Ext = SSE2

        movd    xmm0,esi
        shufps  xmm0,xmm0,0
        paddd   xmm0,xmm2
        pand    xmm0,xmm1
        paddd   xmm0,xmm3

        movd    ebx,xmm0
        movzx   eax,byte[ebx]
;
;        shufps  xmm0,xmm0,11100001b
        psrldq  xmm0,4
        movd    ebx,xmm0
        movzx   ebx,byte[ebx]
        sub     eax,ebx
;
;        shufps  xmm0,xmm0,11111110b
        psrldq  xmm0,4
        movd    ebx,xmm0
        movzx   edx, byte [ebx]
;
;        shufps  xmm0,xmm0,11111111b
        psrldq  xmm0,4
        movd    ebx,xmm0
        movzx   ebx, byte [ebx]
        sub     edx,ebx
;
else
;        mov     ebx,esi
;        dec     ebx
        lea     ebx,[esi-1]
        and     ebx,TEXTURE_SIZE
        add     ebx,.bmap
        movzx   eax,byte [ebx]

;        mov     ebx,esi
;        inc     ebx
        lea     ebx,[esi+1]
        and     ebx,TEXTURE_SIZE
        add     ebx,.bmap
        movzx   ebx,byte [ebx]
        sub     eax,ebx

;        mov     ebx,esi
;        sub     ebx,TEX_X
        lea     ebx,[esi-TEX_X]
        and     ebx,TEXTURE_SIZE
        add     ebx,.bmap
        movzx   edx,byte [ebx]

;        mov     ebx,esi
;        add     ebx,TEX_X
        lea     ebx,[esi+TEX_X]
        and     ebx,TEXTURE_SIZE
        add     ebx,.bmap
        movzx   ebx,byte [ebx]
        sub     edx,ebx
end if

     ;  eax - horizontal sub    modificated x coord
     ;  edx - vertical   sub    modificated y coord
if Ext=NON
        mov     ebx,.cex       ;.cex - current env map X
        shr     ebx,ROUND
        add     eax,ebx


        mov     ebx,.cey       ;.cey - current  env map y
        shr     ebx,ROUND
        add     edx,ebx

else
        movq    mm1,mm5        ; mm5 - copy of cur env coords
        psrld   mm1,ROUND
        movd    ebx,mm1
        psrlq   mm1,32
        add     eax,ebx
        movd    ebx,mm1
        add     edx,ebx
;        movq    qword[.temp1],mm3
;        add     eax,dword [.temp1]
;        add     edx,dword [.temp1+4]
end if

        or      eax,eax
        jl      .black
        cmp     eax,TEX_X
        jg      .black
        or      edx,edx
        jl      .black
        cmp     edx,TEX_Y
        jg      .black

        shl     edx,TEX_SHIFT   ; zaburzenie w emapie = zaburzenie w teksturze
        add     edx,eax         ; proponuje nie stawiac czarnego pixela tylko
        lea     esi,[edx*3]     ; niezaburzony.
        add     esi,.emap       ;
        lodsd

if Ext=NON
        mov     edx,.cty
        shr     edx,ROUND  ; sar

        mov     edi,.ctx
        shr     edi,ROUND  ; sar
else
        movq    mm1,mm7
        psrld   mm1,ROUND
        movd    edx,mm1
        psrlq   mm1,32
        movd    edi,mm1

end if

        shl     edx,TEX_SHIFT
        add     edi,edx
        and     edi,TEXTURE_SIZE
        lea     esi,[edi*3]
        add     esi,.tex_map

if Ext=NON
        mov     edx,eax
        lodsd
        push    ax
        mul     dl
        mov     dl,ah
        pop     ax
        shr     ax,8
        mul     dh
        mov     al,dl
        mov     edi,.c_scr
        stosw
        shr     edx,16
        shr     eax,16
        mul     dl
        shr     ax,8
        stosb
else
        movd       mm0,eax
        pxor       mm1,mm1
        punpcklbw  mm0,mm1
        movd       mm2,[esi]
        punpcklbw  mm2,mm1
        pmullw     mm0,mm2
        psrlw      mm0,8
        packuswb   mm0,mm1
        mov        edi,.c_scr
        movd       [edi],mm0

end if

        jmp     .actual_zbuff   ; actualize z buffer
     @@:
     .black:
        xor     eax,eax
        mov     edi,.c_scr
        stosd
     .actual_zbuff:
        mov     eax,.cz
        mov     edi,.czbuff
        stosd

      .skip:
        add     dword .czbuff,4
        add     dword .c_scr,3

if Ext=NON
        mov     eax,.dbx
        add     .cbx,eax
        mov     ebx,.dby
        add     .cby,ebx

        mov     edx,.dex
        add     .cex,edx
        mov     eax,.dey
        add     .cey,eax

        mov     ebx,.dtx
        add     .ctx,ebx
        mov     edx,.dty
        add     .cty,edx

else
        paddd   mm7,.dty
        paddd   mm6,.dby
        paddd   mm5,.dey
end if
        mov     eax,.dz
        add     .cz,eax

        dec     ecx
        jnz     .draw

  .bl_end:
        mov     esp,ebp
ret 76
