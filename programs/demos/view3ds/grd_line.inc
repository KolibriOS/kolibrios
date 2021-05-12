; Line drawing procedure by Pablo Reda.
; Gouraud extension by Maciej Guba.
;****************************************************
line_grd:  ; eax=x1 ebx=y1 ecx=x2 edx=y2
                 ; xmm0 - color1 argb as 4 dword float
                 ; xmm1 - color2 argb as 4 dword float
                 ; xmm2 - z1 as dword  float
                 ; xmm3 - z2 as dword  float
                 ; mm0  - Z buffer
                 ; mm1  - screen buffer
                 ; mm2  - screen width


        cmp    ebx,edx
      ;  je     horizontal_grd
        jg     .noswap
        xchg   eax,ecx
        xchg   ebx,edx
        movaps xmm7,xmm0
        movaps xmm0,xmm1
        movaps xmm1,xmm7
        movaps xmm7,xmm2
        movaps xmm2,xmm3
        movaps xmm3,xmm7
.noswap:

        cvtsi2ss xmm7,eax
        cvtsi2ss xmm6,ebx
        cvtsi2ss xmm5,ecx
        cvtsi2ss xmm4,edx
        subss    xmm7,xmm5
        subss    xmm6,xmm4
        andps    xmm7,[abs_mask]  ; 0x7fff
        andps    xmm6,[abs_mask]  ; 0x7fff
        maxss    xmm7,xmm6   ; xmm7 - delta
        shufps   xmm7,xmm7,0
        movaps   xmm6,xmm3
        rcpps    xmm7,xmm7
        subss    xmm6,xmm2
        mulss    xmm6,xmm7   ; xmm6 - delta z
        movaps   xmm5,xmm1
        subps    xmm5,xmm0
        mulps    xmm5,xmm7   ; xmm5 - delta col argb


        shl eax,16
        shl ecx,16
        sub eax,ecx
        push ebx
        push edx
        sub ebx,edx
        add ebx,1
        cdq
        idiv ebx
        mov esi,eax
        add ecx,$7fff
        pop ebx
        pop edx
.lineas:
        mov eax,ecx
        add eax,esi
        push ebx
        push eax
        push edx
        shr ecx,16
        shr eax,16
        call horizontal_grd
        pop edx
        pop ecx
        pop ebx
        add ebx,1
        cmp ebx,edx
        jle .lineas
        ret

horizontal_grd:  ; eax=x1 ebx=y1 ecx=x2
        cmp ecx,eax
        jg .m
        xchg ecx,eax
.m:
        sub ecx,eax
        jnz .nn
        add ecx,1
.nn:
        movd  edx,mm2
        imul  ebx,edx
        add   eax,ebx
        movd  edi,mm1
        lea   edi,[edi+eax*4]
        push  eax
        push  ebx
        movd  ebx,mm0
        lea   ebx,[ebx+eax*4]
.l:
      ;  movss    xmm4,xmm2     ; xmm2 - cur z
      ;  cmpnltss xmm4,[ebx]
      ;  movd     eax,xmm4
      ;  or       eax,eax
      ;  jnz      @f
        comiss   xmm2,[ebx]
        jng      @f
        movaps   xmm4,xmm0    ; xmm0 - cur col
        cvtps2dq xmm4,xmm4
        packssdw xmm4,xmm4
        packuswb xmm4,xmm4
        movd     [edi],xmm4
      @@:
        addss xmm2,xmm6
        addps xmm0,xmm5
        add   edi,4
        sub   ecx,1
        jnz   .l
        pop   ebx
        pop   eax
        ret
