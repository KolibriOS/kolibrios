;405 412 586
;
;
bufer_size=1024*16+2
fichero=4
mcu_ptr=8
color_ptr=12
estado=16
color_c=17
nbits=color_c
idct=20
tmp_bits=24
actable=28
matriz_limit=32
sourcebits=36
sourcebits_index=40
sourcebits_limit=44
qt_ptrs=48
ht_dc_ptrs=64
ht_ac_ptrs=80
matrices=96
tmp_bufer=100
x_org=104
y_org=108
x_mcu=112
y_mcu=116
x_size=120
y_size=124
x_org2=128
y_org2=132
x_mcu2=136
y_mcu2=140
x_size2=144
y_size2=148
q_ptr=152
dc=164
position=204
draw_ptr=208
struct_size=212

jpeg_info:  ;fichero en eax
            ;retorna ebp
        xor ebp,ebp
        pushad
        mov ebp,esp
        mov ecx,6
        sub esp,ecx
        mov edi,esp
        call read
        pop dx
        cmp dx,0d8ffh
        je .l1
        mov esp,ebp
        popad
        ret
   .l1: push eax
        mov ecx,struct_size
        call mallocz
        mov [edi],ebp
        mov ebp,edi
        pop dword [ebp+fichero]
        pop ax
        pop cx
        jmp .l3
   .l2: mov ebx,[ebp+tmp_bufer]
        add ebx,[ebx-4]
        mov cx,[ebx-2]
        mov ax,[ebx-4]
   .l3: push .l2
        xchg cl,ch
        add cx,2
        cmp ch,3
        jnc .l4
        cmp al,0ffh
        jne eoi
        cmp ah,0dbh
        je dqt
        cmp ah,0c4h
        je dht
        cmp ah,0c0h
        je sof0
        cmp ah,0dah
        je sos
        cmp ah,0c2h
        je eoi
        cmp ah,0c9h
        je eoi
        cmp ah,0d9h
        je eoi
   .l4: lea edx,[ecx-4]
        xor ecx,ecx
        mov eax,[ebp+fichero]
        call skip
        mov ecx,4
        call READ
        cmp ecx,[edi-4]
        jne eoi
        ret

eoi:
      mov esp,[ebp]
      call jpeg_close
      popad
      xor ebp,ebp
      ret

jpeg_close:
      test ebp,ebp
      jz .l2
      pushad
      mov eax,[ebp+fichero]
      call close
      mov edi,[ebp+sourcebits]
      call free
      lea esi,[ebp+qt_ptrs]
      mov ecx,14
 .l1: mov edi,[esi]
      add esi,4
      call free
      loop .l1
      mov edi,ebp
      call free
      popad
 .l2: ret

dqt:  call READ
      mov esi,edi
      lea eax,[edi+ecx]
      push eax
 .l1: xor eax,eax
      lodsb
      cmp al,4
      jnc eoi
      lea ebx,[ebp+qt_ptrs+eax*4]
      test dword [ebx],-1
      jnz eoi
      mov ecx,64
      xor eax,eax
      sub esp,128
      mov edi,esp
 .l2: lodsb
      stosw
      loop .l2
      mov ecx,256
      call malloc
      mov [ebx],edi
      mov eax,esi
      mov esi,esp
      pushad
      mov ebp,zigzag
      fninit
      mov cl,64
      xor eax,eax
  .l3: fild word[esi]
      mov al,[ebp]
      and al,-2
      add ebp,2
      add esi,2
      mov ebx,eax
      and ebx,28
      fmul dword [ebx+k2]
      mov ebx,eax
      shr ebx,3
      and ebx,4+8+16
      fmul dword [ebx+k2]
      fstp dword [edi+eax]
      dec cl
      jnz .l3
      popad
      mov esi,eax
      add esp,128
      mov eax,[esp]
      sub eax,esi
      jc eoi
      cmp eax,4
      ja .l1
      jne eoi
      pop eax
      ret

sof0:  call READ
       cmp byte [edi],8
       jne eoi  ;precision
       mov ax,[edi+1]
       xchg al,ah
       mov [ebp+y_size],ax
       mov ax,[edi+3]
       xchg al,ah
       mov [ebp+x_size],ax
       mov al,[edi+5] ;ncomponentes
       mov cl,al
       mov [ebp+color_c],al
       mov edx,modes
       dec al
       jz .l1
       dec al
       dec al
       jnz eoi
       mov al,[edi+10]
       mov ah,[edi+13]
       cmp ax,1111h
       jne eoi
       mov al,[edi+7]
       add edx,16
       cmp al,11h
       je .l1
       add edx,16
       cmp al,21h
       je .l1
       add edx,16
       cmp al,22h
       jne eoi
  .l1: lea ebx,[ebp+q_ptr]
       lea esi,[ebp+qt_ptrs]
       mov [ebp+mcu_ptr],edx
  .l2: movzx eax,byte [edi+8]
       add edi,3
       cmp al,4
       jnc eoi
       lea eax,[eax*4+esi]
       mov [ebx],eax
       add ebx,16
       dec cl
       jnz .l2
       ret

READ:  mov eax,[ebp+fichero]
       mov edi,[ebp+tmp_bufer]
       movzx ecx,cx
       call mresize
       mov [ebp+tmp_bufer],edi
       jmp read

dht:    call READ
        mov esi,edi
        lea eax,[edi+ecx]
        push eax
   .l1: lodsb
        mov edi,esi
        mov ebx,3+16
        and bl,al
        cmp bl,al
        jne eoi
        shr bl,2
        and al,3
        or bl,al
        lea ebx,[ebp+ht_dc_ptrs+ebx*4]
        test dword [ebx],-1
        jnz eoi
        mov cl,15
        mov al,[edi]
   .l2: inc edi  ;calcular numero de codigos
        add al,[edi]
        jc eoi
        dec cl
        jnz .l2
        movzx ecx,al
        lea ecx,[ecx*4+2]
        call malloc
        mov [ebx],edi
        call arbol_hf
        mov eax,[esp]
        sub eax,ebx
        jc eoi
        mov esi,ebx
        cmp eax,4
        ja .l1
        jne eoi
        pop eax
        ret

arbol_hf:   ;esi=ht edi=memoria para el arbol
            ;retorna en ebx  el final de ht
            ;codigos: bits 0-3=nbits del siguiente numero
            ;         bits 4-7=numero de zeros
            ;         bits 8-14=longitud de este codigo o error si =127
            ;         bit 15=codigo/puntero
        push ebp
        lea ebx,[edi-2]
        add ebx,[ebx-2]
        mov word [ebx],-1 ;codigo de error si encontrado
        push ebx
        push esi
        lea ebx,[esi+16]
        mov ebp,esp
        xor ecx,ecx
        push edi
        push ecx
        add edi,2
        mov dx,1
        add dh,[esi]
        jz .l3
        jmp .l2
  .l1:  push edi
        push ecx
        add edi,2
  .l2:  inc cl
        cmp cl,dl
        jc .l1
        mov al,[ebx]
        inc ebx
        mov ah,128 ;marca de codigo
        or ah,dl
        cmp edi,[ebp+4]
        jnc .l5
        stosw
        cmp esp,ebp
        jnc .l5
        pop ecx
        pop esi
        lea eax,[edi-2]
        sub eax,esi
        mov [esi],ax
        dec dh
        jnz .l2 ;ncodigos
        mov esi,[ebp]
   .l3: inc esi
        inc dl
        cmp dl,17
        jnc .l4
        add dh,[esi]
        jz .l3
        mov [ebp],esi
        jmp .l2
   .l4: lea esp,[ebp+8]
        pop ebp
        ret
   .l5: mov ebp,[ebp+8]
        jmp eoi

sos:   sub ecx,4  ;a continuacion vienen los datos de la imagen
       call READ
       mov eax,[ebp+fichero]
       call ftell
       mov [ebp+position],edx
       mov esi,edi
       lea edi,[ebp+q_ptr]
       lodsb ;numero de componentes
       sub [ebp+color_c],al
       jnz eoi
       mov dh,al
  .l1: mov ebx,[edi]
       mov eax,[ebx]
       stosd
       lodsw
       mov cl,ah
       and eax,0f00h
       and ecx,0f0h
       shr eax,6
       shr ecx,2
       lea ebx,[ebp+ht_ac_ptrs+eax]
       mov eax,[ebx]
       lea ebx,[ebp+ht_dc_ptrs+ecx]
       mov ecx,[ebx]
       test eax,eax
       jz eoi
       test ecx,ecx
       jz eoi
       stosd
       mov eax,ecx
       stosd
       add edi,4
       dec dh
       jnz .l1
       mov edx,[ebp+mcu_ptr]
       cmp edx,modes
       jne .l2
       lea esi,[ebp+q_ptr]
       lea edi,[ebp+q_ptr+32]
       movsd
       movsd
       movsd
   .l2:
       mov esi,edx
       push dword [esi]
       pop dword [ebp+mcu_ptr]
       push dword [esi+4]
       pop dword[ebp+color_ptr]
       push dword [esi+12]
       pop dword [ebp+y_mcu]
       push dword [esi+8]
       pop dword [ebp+x_mcu]
       mov ecx,64*18
       call malloc
       mov [ebp+matrices],edi
       mov ecx,bufer_size
       call malloc
       mov [ebp+sourcebits],edi
       mov esp,[ebp]
       mov [esp+8],ebp
       popad
       ret

jpeg_display:
       test ebp,ebp
       jnz .inicio
       ret
      .inicio:
       pushad
       mov [ebp],esp
       mov eax,[ebp+fichero]
       mov edx,[ebp+position]
       call lseek
       mov edi,[ebp+sourcebits]
       add edi,bufer_size
       mov [ebp+sourcebits_index],edi
       sub edi,2
       mov [ebp+sourcebits_limit],edi
       mov edi,[ebp+matrices]
       mov [ebp+matriz_limit],edi
       xor eax,eax
       mov [esp+8],eax
       mov [ebp+estado],eax
       mov [ebp+tmp_bits],eax
       mov [ebp+dc],eax
       mov [ebp+dc+16],eax
       mov [ebp+dc+32],eax

       mov eax,[ebp+y_mcu]
       mov ecx,[ebp+y_org]
       sub ecx,eax
       mov [ebp+y_org2],ecx
       mov [ebp+y_mcu2],eax
       push dword [ebp+y_size]
       pop dword [ebp+y_size2]
  .l3: push dword [ebp+x_org]
       pop dword [ebp+x_org2]
       push dword [ebp+x_mcu]
       pop dword [ebp+x_mcu2]
       push dword [ebp+x_size]
       pop dword [ebp+x_size2]
       mov eax,[ebp+y_mcu2]
       add [ebp+y_org2],eax
       sub [ebp+y_size2],eax
       jnc .l4
       add eax,[ebp+y_size2]
       jnz .cont
       mov [esp+8],ebp
       popad
       ret
     .cont:
       mov dword [ebp+y_size2],0
       mov [ebp+y_mcu2],eax
      .l4:
        mov eax,[ebp+x_mcu2]
        sub [ebp+x_size2],eax
        jnc .l5
        add eax,[ebp+x_size2]
        jz .l3
        mov dword [ebp+x_size2],0
        mov [ebp+x_mcu2],eax
        call dword [ebp+mcu_ptr]
        mov eax,[ebp+x_mcu]
        mov ecx,[ebp+x_mcu2]
        mov edx,[ebp+y_mcu2]
        call recortar
        jmp .l6
      .l5:
        call dword [ebp+mcu_ptr]
        mov ecx,[ebp+x_mcu2]
        mov edx,[ebp+y_mcu2]
      .l6:
        mov eax,[ebp+x_org2]
        mov ebx,[ebp+y_org2]
        call dword [ebp+draw_ptr]
        add [ebp+x_org2],ecx
        mov ax,[ebp+estado]
        test al,15
        jz .l4
        cmp ah,8
        jnc .l4
        xor edx,edx
        mov [ebp+tmp_bits],edx
        mov [ebp+dc],edx
        mov [ebp+dc+16],edx
        mov [ebp+dc+32],edx
        add dword [ebp+sourcebits_index],2
        and word [ebp+estado],0c0h
        test al,32
        jz .l4
        jmp .l3

color100:
        push edi
    .l1: lodsw
        mov dl,ah
        mov ah,al
        stosw
        mov ah,dl
        stosb
        mov al,dl
        stosb
        stosw
        dec cl
        jnz .l1
        pop edi
        ret

color111:
        push edi
   .l1: lodsw
        mov bx,[esi+62]
        mov dx,[esi+126]
        xchg ah,bh
        xchg ah,dl
        xchg ah,bl
        stosw
        mov ax,bx
        stosw
        mov ax,dx
        stosw
        dec cl
        jnz .l1
        pop edi
        mov ecx,64*3
        jmp ybr_bgr

color411:
        push ebp
        push edi
        lea ebp,[esi+ecx*8]
     .l1: push ecx
        mov ax,[esi]
        mov cx,[ebp]
        mov dx,[ebp+64]
        add ebp,2
        xchg ch,dl
        mov bx,ax
        mov ah,cl
        mov bl,ch
        mov [edi],ax
        mov [edi+2],bx
        mov [edi+4],cx
        mov ax,[esi+8]
        mov bh,ah
        mov ah,cl
        mov [edi+48],ax
        mov [edi+48+2],bx
        mov [edi+48+4],cx
        mov ax,[esi+2]
        mov bx,ax
        mov ah,dl
        mov bl,dh
        mov [edi+6],ax
        mov [edi+2+6],bx
        mov [edi+4+6],dx
        mov ax,[esi+8+2]
        mov bh,ah
        mov ah,dl
        mov [edi+48+6],ax
        mov [edi+48+2+6],bx
        mov [edi+48+4+6],dx
        pop ecx
        add edi,12
        dec ecx
        add esi,4
        test cl,1
        jnz .l1
        add esi,64-8
        test cl,2
        jnz .l1
        sub esi,128-16
        add edi,48
        test cl,15
        jnz .l1
        add esi,64
        test cl,cl
        jnz .l1
        pop edi
        pop ebp
        mov ecx,64*4*3
        jmp ybr_bgr

color211:
        push ebp
        push edi
        lea ebp,[esi+ecx*4]
    .l1: push ecx
        mov ax,[esi]
        mov cx,[ebp]
        mov dx,[ebp+64]
        add ebp,2
        xchg ch,dl
        mov bx,ax
        mov ah,cl
        mov bl,ch
        mov [edi],ax
        mov [edi+2],bx
        mov [edi+4],cx
        mov ax,[esi+2]
        mov bx,ax
        mov ah,dl
        mov bl,dh
        mov [edi+6],ax
        mov [edi+2+6],bx
        mov [edi+4+6],dx
        pop ecx
        add edi,12
        dec cl
        add esi,4
        test cl,1
        jnz .l1
        add esi,64-8
        test cl,2
        jnz .l1
        sub esi,128-8
        test cl,cl
        jnz .l1
        pop edi
        pop ebp
        mov ecx,64*3*2
        jmp ybr_bgr


mcu411: lea ebx,[ebp+q_ptr]
        call hufdecode
        lea ebx,[ebp+q_ptr]
        call hufdecode
mcu211: lea ebx,[ebp+q_ptr]
        call hufdecode
mcu111: lea ebx,[ebp+q_ptr]
        call hufdecode
        lea ebx,[ebp+q_ptr+16]
        call hufdecode
mcu100: lea ebx,[ebp+q_ptr+32]
        call hufdecode
        mov esi,[ebp+matrices]
        mov dword [ebp+matriz_limit],esi
        mov ecx,32
        lea edi,[esi+64*6]
        jmp dword [ebp+color_ptr]

cargar_bits: ;edx=bits,cl=nbits,
             ;bp=data struct
             ;cr: cl,edx,eax,si
             ;ncr bx,bp,di,ch

        mov esi,[ebp+sourcebits_index]
        cmp esi,[ebp+sourcebits_limit]
        jnc .l6
        movzx eax,byte [esi]
        inc esi
        add cl,8
        cmp al,-1
        je .l2
        mov ah,al
        lodsb
        add cl,8
        cmp al,-1
        je .l2
   .l1: ror eax,cl
        or edx,eax
        mov [ebp+sourcebits_index],esi
        ret
   .l2: lodsb
        test al,al
        jnz .l3
        mov al,-1
        call .l1
        cmp cl,16
        jc cargar_bits
        ret
   .l3: sub esi,2
        sub cl,8
        sub al,0d0h
        cmp al,8
        jc .l4
        sub al,9
        mov al,63
        jz .l4
        mov al,127
    .l4: inc al
        or [ebp+estado],al
        movzx eax,ah
        jmp .l1
    .l5: mov [ebp+sourcebits_limit],edi
        mov word [edi],0d9ffh
        popad
        jmp cargar_bits
   .l6: ;read file
        pushad
        mov ecx,bufer_size-2
        mov edx,[ebp+sourcebits_limit]
        mov edi,[ebp+sourcebits]
        mov ax,[edx]
        sub edx,edi
        stosw
        sub esi,edx
        mov [ebp+sourcebits_index],esi
        cmp edx,ecx
        jne .l5
        mov eax,[ebp+fichero]
        call read
        lea ecx,[edi+ecx-2]
        mov [ebp+sourcebits_limit],ecx
        popad
        jmp cargar_bits


hufdecode:  ;si->dctable [bp+20]->actable di->outbufer edx->bits cl->bits en edx


            ;[bp+24]->sourcebits
            ;[bp+22]=outbufer+128
            ;[bx]       q ptr para aa&n
            ;[bx+2]     a ptr
            ;[bx+4]     d ptr
            ;[bx+8]     dc componente
        fninit
        push dword [ebx]
        mov cl,[ebp+nbits]
        mov edx,[ebp+tmp_bits]
        cmp cl,16
        jnc .l1
        call cargar_bits
    .l1: mov eax,[ebx+4]
        mov esi,[ebx+8]
        mov [ebp+actable],eax
        movzx eax,word [esi]
        add esi,2
   .l2: add edx,edx
        jnc .l3
        add esi,eax
   .l3: lodsw
        test ax,ax
        jns .l2
        ;codigo encontrado
        and ax,7f0fh
        mov edi,[ebp+matriz_limit] ;arrays
        sub cl,ah
        jns .l4
        fldz
   .error:
        xor ecx,ecx
        or byte [ebp+estado],32
        jmp .l12
   .l4: cmp cl,al
        jnc .l5
        push eax
        call cargar_bits
        pop eax
   .l5: sub cl,al
        mov ch,cl
        mov cl,al
        mov eax,edx
        shl edx,cl
        sar eax,17
        xor ax,8000h
        xor cl,15
        sar ax,cl
        mov cl,ch
        mov ch,2
        add ax,8000h ;incrementar si negativo
        adc ax,8000h
        add [ebx+12],ax
        fild word [ebx+12]
        push ecx
        mov ecx,64
        xor eax,eax
        add [ebp+matriz_limit],ecx
        rep stosd
        pop ecx
        sub edi,64*4
        mov ebx,[esp]
        fmul dword [ebx]
    .l6: cmp cl,16
        jnc .l7
        call cargar_bits
   .l7: mov esi,[ebp+actable]
        movzx eax,word[esi]
        add esi,2
   .l8: add edx,edx
        jnc .l9
        add esi,eax
   .l9: lodsw
        test ax,ax
        jns .l8
        ;codigo encontrado
        and ah,127
        xor ebx,ebx
        sub cl,ah
        js .error
        or bl,al
        jz .l12
        and al,0f0h
        shr al,3
        add ch,al
        js .error
        and bl,0fh
        jz .l11
        cmp cl,bl
        jnc .l10
        call cargar_bits
    .l10: sub cl,bl
        xchg bl,cl
        mov eax,edx
        shl edx,cl
        sar eax,17
        xor cl,15
        xor ax,8000h
        sar ax,cl
        add ax,8000h ;incrementar si negativo
        adc ax,8000h
        mov cl,bl
        mov bl,ch
        mov [ebp+tmp_bits],ax
        mov ax,[ebx+zigzag]
        mov ebx,[esp]
        fild word [ebp+tmp_bits]
        or [ebp+idct],ax
        and eax,11111100b
        fmul dword [ebx+eax]
        fstp dword [edi+eax]
  .l11: add ch,2
        jns .l6
  .l12: mov [ebp+nbits],cl
        mov [ebp+tmp_bits],edx
        xor ebx,ebx
        add esp,4
        xchg ebx,[ebp+idct]
        cmp ch,2
        je idctf1
        fstp dword [edi]
        test bh,0feh
        jnz idctf3
idctf2a: test bh,1
        mov esi,edi
        jz .l1
        test bl,1
        jnz idctf3
        push idctf2b
        jmp idctf3b
   .l1: call idctf3a
        mov cl,4
        call limit
        mov eax,[edi-8]
        mov edx,[edi-4]
        mov cl,7
   .l2: mov [edi],eax
        mov [edi+4],edx
        add edi,8
        dec cl
        jnz .l2
        ret

idctf1: fistp word[edi+64]
        mov ax,128
        add ax,[edi+64]
        jz .l2
        test ah,ah
        jz .l1
        mov al,-1
        js .l2
   .l1: mov ah,al
        stosw
        stosw
        mov eax,[edi-4]
        mov ecx,15
        rep stosd
   .l2: ret

idctf3: mov bl,8
        mov esi,edi
    .l1: rcr bh,1
        jc .l3
        mov eax,[esi]
        test eax,eax
        jz .l4
        mov cl,7
    .l2: add esi,32
        mov [esi],eax
        dec cl
        jnz .l2
        sub esi,32*7-4
        dec bl
        jnz .l1
        jmp .l5
   .l3: call idctf3b
   .l4: add esi,4
        dec bl
        jnz .l1
   .l5: mov esi,edi
        mov cl,8
   .l6: call idctf3a
        add esi,32
        add edi,16
        dec cl
        jnz .l6
        sub edi,128
        mov esi,edi
        mov cl,32
limit:  mov dx,[esi]
        mov bx,[esi+2]
        add esi,4
        add dx,128
        add bx,128
        test dh,dh
        mov ax,dx
        jz .l1
        mov al,0
        js .l1
        mov al,-1
   .l1: test bh,bh
        mov ah,bl
        jz .l2
        mov ah,0
        js .l2
        mov ah,-1
   .l2: stosw
        dec cl
        jnz limit
        ret

idctf2b:
        mov dl,8
  .l1:  fld dword[esi]
        add esi,32
        mov ax,128
        fistp word [edi]
        add ax,[edi]
        test ah,ah
        jz .l2
        mov al,0
        js .l2
        mov al,-1
  .l2:  mov ah,al
        stosw
        stosw
        stosw
        stosw
        dec dl
        jnz .l1
        ret

idctf3a: ;si(d float),di(w int) ncr
fld dword[esi+1*4]      ;f1                ;t21=f1+f7
fld st0
fld dword[esi+7*4]          ;f7
fadd st2,st0
fsubp st1,st0          ;t22=f1-f7
fld dword[esi+5*4]
fld st0           ;f5       ;t23=f5+f3
fld dword[esi+3*4]                   ;f3
fadd st2,st0
fsubp st1,st0          ;t20=f5-f3
fld st0
fadd st0,st3                ;t25=(t20+t22)*k2
fmul dword[k+4]                 ;k2 ;t25,t20,t23,t22,t21
fld st4                         ;t7=t21+t23
fadd st0,st3                        ;t7,t25,t20,t23,t22,t21
fld dword[k+12]                 ;k4     ;t6=k4*t20+t25-t7
fmulp st3,st0
fsub st2,st0
fld st1
faddp st3,st0                   ;t7,t25,t6,t23,t22,t21
fld st5                         ;t5=(t21-t23)*k1-t6
fsub st0,st4
fmul dword[k]          ;k1
fsub st0,st3
fstp st6               ;t7,t25,t6,t23,t22,t5
fstp st3                        ;t25,t6,t7,t22,t5
fxch st3
fmul dword[k+8]    ;k3      ;t4=k3*t22-t25+t5
fadd st0,st4                        ;t22*k3+t5,t6,t7,t25,t5
fsubrp st3,st0                  ;t6,t7,t4,t5
fld dword[esi]                       ;f0           ;t10=f0+f4
fst st5                         ;f0,t4,t5,t6,t7,f0
fld dword[esi+4*4]                   ;f4
fsub st6,st0                    ;t11=f0-f4
faddp st1,st0
fld st0               ;t10,t10,t6,t7,t4,t5,t11
fld dword[esi+2*4]                   ;f2      ;t13=f2+f6
fadd dword[esi+6*4]                  ;f6      ;t13,t10,t10,t6,t7,t4,t5,t11
fadd st2,st0                    ;t13,t10,t0,t6,t7,t4,t5,t11 ;t0=t10+t13
fsubp st1,st0                      ;t3,t0,t6,t7,t4,t5,t11 ;t3=t10-t13
fld st0                         ;p3=t3-t4
fsub st0,st5
fistp word [edi+3*2]        ;p3
fadd st0,st4                        ;p4=t3+t4
fld dword[esi+2*4]                   ;f2
fstp st5
fistp word [edi+4*2]        ;p4 ;t0,t6,t7,f2,t5,t11
fld st0                         ;p0=t0+t7
fsub st0,st3
fistp word [edi+7*2]        ;p7
fadd st0,st2                        ;p7=t0-t7
fistp word [edi]                     ;p0 ;t6,t7,f2,t5,t11
fld st2                         ;f2  ;f2,t6,t7,f2,t5,t11  ;t12=(f2-f6)*k1-t13
fld dword[esi+6*4]                   ;f6
fadd st4,st0                    ;f6,f2,t6,t7,t13,t5,t11
fsubp st1,st0
fmul dword[k]         ;k1
fsub st0,st3
fst st3                ;t12,t6,t7,t12,t5,t11
fadd st0,st5                        ;t1=t11+t12
fst st2                         ;t1,t6,t1,t12,t5,t11
fadd st0,st1                        ;p1=t1+t6
fistp word [edi+2]                   ;p1    ;t6,t1,t12,t5,t11
fsubp st1,st0                       ;p6=t1-t6
fistp word [edi+6*2]                 ;p6  ;t12,t5,t11
fsubp st2,st0                   ;t2=t11-t12  ;t5,t2
fld st0
fadd st0,st2                ;p2=t2+t5
fistp word [edi+2*2]                 ;p2
fsubp st1,st0                       ;p5=t2-t5   ;t5,t2
fistp word [edi+5*2]
ret             ;p5




idctf3b: ;si ncr
fld dword[esi+1*32]
fld st0          ;f1       ;t21=f1+f7
fld dword[esi+7*32]
fadd st2,st0     ;f7
fsubp st1,st0                       ;t22=f1-f7
fld dword[esi+5*32]
fld st0          ;f5       ;t23=f5+f3
fld dword[esi+3*32]                  ;f3
fadd st2,st0
fsubp st1,st0
fld st0  ;t20=f5-f3
fadd st0,st3                        ;t25=(t20+t22)*k2
fmul dword[k+4]                 ;k2 ;t25,t20,t23,t22,t21
fld st4                         ;t7=t21+t23
fadd st0,st3                        ;t7,t25,t20,t23,t22,t21
fld dword[k+12]                 ;k4     ;t6=k4*t20+t25-t7
fmulp st3,st0
fsub st2,st0
fld st1
faddp st3,st0                   ;t7,t25,t6,t23,t22,t21
fld st5                         ;t5=(t21-t23)*k1-t6
fsub st0,st4
fmul dword[k]          ;k1
fsub st0,st3
fstp st6               ;t7,t25,t6,t23,t22,t5
fstp st3
fxch st3               ;t25,t6,t7,t22,t5
fmul dword[k+8]                 ;k3      ;t4=k3*t22-t25+t5
fadd st0,st4                        ;t22*k3+t5,t6,t7,t25,t5
fsubrp st3,st0                  ;t6,t7,t4,t5
fld dword[esi]                       ;f0           ;t10=f0+f4
fst st5                         ;f0,t4,t5,t6,t7,f0
fld dword[esi+4*32]                  ;f4
fsub st6,st0                    ;t11=f0-f4
faddp st1,st0
fld st0               ;t10,t10,t6,t7,t4,t5,t11
fld dword[esi+2*32]                  ;f2      ;t13=f2+f6
fadd dword[esi+6*32]                 ;f6      ;t13,t10,t10,t6,t7,t4,t5,t11
fadd st2,st0                    ;t13,t10,t0,t6,t7,t4,t5,t11 ;t0=t10+t13
fsubp st1,st0                       ;t3,t0,t6,t7,t4,t5,t11 ;t3=t10-t13
fld st0                         ;p3=t3-t4
fsub st0,st5
fstp dword[esi+3*32]        ;p3
fadd st0,st4                        ;p4=t3+t4
fld dword[esi+2*32]                  ;f2
fstp st5
fstp dword[esi+4*32]        ;p4 ;t0,t6,t7,f2,t5,t11
fld st0
fsub st0,st3                ;p0=t0+t7
fstp dword[esi+7*32]                 ;p7
fadd st0,st2                        ;p7=t0-t7
fstp dword[esi]                      ;p0 ;t6,t7,f2,t5,t11
fld st2                         ;f2  ;f2,t6,t7,f2,t5,t11  ;t12=(f2-f6)*k1-t13
fld dword[esi+6*32]                  ;f6
fadd st4,st0                    ;f6,f2,t6,t7,t13,t5,t11
fsubp st1,st0
fmul dword[k]         ;k1
fsub st0,st3
fst st3                ;t12,t6,t7,t12,t5,t11
fadd st0,st5                        ;t1=t11+t12
fst st2                         ;t1,t6,t1,t12,t5,t11
fadd st0,st1                        ;p1=t1+t6
fstp dword[esi+1*32]                 ;p1    ;t6,t1,t12,t5,t11
fsubp st1,st0                       ;p6=t1-t6
fstp dword[esi+6*32]                 ;p6  ;t12,t5,t11
fsubp st2,st0
fld st0           ;t2=t11-t12  ;t5,t2
fadd st0,st2                        ;p2=t2+t5
fstp dword[esi+2*32]                 ;p2
fsubp st1,st0                       ;p5=t2-t5   ;t5,t2
fstp dword[esi+5*32]
ret             ;p5

ybr_bgr:  ;edi=bmp ecx=n_BYTES
	  ;retorna edi+=ecx
	pushad
	mov esi,edi
	add edi,ecx
	push edi
	mov edi,[colortabla]
   .l1: lodsw
	movzx ebx,ah
	movzx ebp,al
	movzx eax,al
	movzx ecx,byte[esi]
	lea ebx,[ebx*4+edi+1024]
	lea ecx,[ecx*4+edi]
	add eax,[ebx] ;cb   ;solo se usan 16 bits
	mov edx,[ebx+2]     ;pero el codigo de 32 bits es mas rapido
	mov ebx,[ecx] ;cr
	add eax,[ecx+2]
	add ebx,ebp ;b
	add edx,ebp ;r
	test ah,ah
	jz .l2
	mov al,0
	js .l2
	mov al,-1
   .l2: test dh,dh
	jz .l3
	mov dl,0
	js .l3
	mov dl,-1
   .l3: test bh,bh
	mov dh,al
	jz .l4
	mov bl,0
	js .l4
	mov bl,-1
   .l4: mov [esi-2],dx
	mov [esi],bl
	inc esi
	cmp esi,[esp]
	jc .l1
	pop edi
	popad
	ret

recortar:  ;edi=bufer eax=ancho en pixels (ecx,edx)tama¤o deseado
        pushad
        dec edx
        jz .l2
        lea ebx,[ecx*3]
        lea eax,[eax*3]
        lea esi,[edi+eax]
        add edi,ebx
        sub eax,ebx
  .l1:  mov ecx,ebx
        call movedata
        add esi,eax
        dec edx
        jnz .l1
   .l2: popad
        ret

;R = Y                    + 1.402  *(Cr-128)
;G = Y - 0.34414*(Cb-128) - 0.71414*(Cr-128)
;B = Y + 1.772  *(Cb-128)

colortabla: dd 0

colorprecalc: ;prepara la tabla para convertir ycb a rgb
	 mov ecx,1024*2
	 call malloc
	 mov [colortabla],edi
	 fninit
	 fld dword [.k+4]
	 fld dword [.k]
	 mov dl,0
	 call .l1
	 fld dword [.k+12]
	 fld dword[.k+8]
    .l1: mov cx,-128
    .l2: mov [edi],ecx
	 inc ecx
	 fild word[edi]
	 fld st0
	 fmul st0,st2
	 fistp word[edi]
	 fmul st0,st2
	 fistp word[edi+2]
	 add edi,4
	 inc dl
	 jnz .l2
	 ret

     .k: dd 1.402,-0.71414,-0.34414,+1.772
