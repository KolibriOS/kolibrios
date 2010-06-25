;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                        ;;
;;  VGA.INC                                               ;;
;;                                                        ;;
;;  640x480 mode 0x12 VGA functions for MenuetOS          ;;
;;                                                        ;;
;;  Paul Butcher, paul.butcher@asa.co.uk                  ;;
;;                                                        ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



paletteVGA:

;16 colour palette
       mov    dx,0x3c8
       mov    al,0
       out    dx,al

       mov    ecx,16
       mov    dx,0x3c9
       xor    eax,eax

     palvganew:

       mov al,0
       test ah,4
       jz palvgalbl1
       add al,31
       test ah,8
       jz palvgalbl1
       add al,32
     palvgalbl1:
       out dx,al  ; red 0,31 or 63
       mov al,0
       test ah,2
       jz palvgalbl2
       add al,31
       test ah,8
       jz palvgalbl2
       add al,32
     palvgalbl2:
       out dx,al  ; blue 0,31 or 63
       mov al,0
       test ah,1
       jz palvgalbl3
       add al,31
       test ah,8
       jz palvgalbl3
       add al,32
     palvgalbl3:
       out dx,al  ; green 0,31 or 63
       add ah,1
       loop palvganew

       ret


vga_putimage:

 push ebp ;
 push esi ;
 push edi ;

 push eax ;
 push ebx ; +8 [ptrImage]
 push ecx ; +4 [BH]
 push edx ; +0 [xy]

 movzx eax,word [esp+2]   ; eax:=x
 movzx ebx,word [esp+0]   ; ebx:=y
 mov ecx,[0x3010]         ;
 add eax,[ecx-twdw]     ; eax+=Xwin
 add ebx,[ecx-twdw+4]   ; ebx+=Ywin
 mov ecx,ebx              ; ecx = y+Ywin
 mov edx,eax              ; edx = x+Xwin

 imul ebx, 640*4          ; (y+Ywin)*BytesPerScanLine
 shl eax,2                ; (x+Xwin)*BytesPerPixel
 add eax,ebx              ;
 mov edi,eax              ; store copy
        add eax,[0xfe80]         ; +AddrLFB
 ;entry point in LFB >> EAX:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel+AddrLFB

 shr edi,5                ; change from 4 to 1/8 BytesPerPixel
 add edi,0xa0000          ; + VGABasePtr
 ;VGA start address >> EDI:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel+AddrVGA

 mov ebx, [0xfe00]        ; ScreenXSize
 inc ebx                  ; +1
 imul ebx,ecx             ; *(y+Ywin)
 mov ebp, ebx             ;
 add ebp, edx             ; +(x+Xwin)
 add ebp, WinMapAddress   ; ebp:=(y+Ywin)*(ScreenXSize+1)+(x+Xwin)+AddrBuffer

 mov esi,[esp+8] ; esi:=AddrImg
 movzx ecx,word [esp+6] ; ecx:=B
 movzx ebx,word [esp+4] ; ebx:=H

 ; check limits while draw ?

 push ecx ; B
 push eax ; LFB address

 mov eax,[0x3010]
 mov ecx,[eax+draw_data-0x3000+0]
 cmp ecx,0
 jnz dbcblimitlset_vga

 mov ecx,[eax+draw_data-0x3000+4]
 cmp ecx,0
 jnz dbcblimitlset_vga

 mov ecx,[eax+draw_data-0x3000+8]
 cmp ecx,[0xfe00] ; ecx <> Screen X size
 jnz dbcblimitlset_vga

 mov ecx,[eax+draw_data-0x3000+12]
 cmp ecx,[0xfe04] ; ecx <> Screen Y size
 jnz dbcblimitlset_vga

 pop eax ; LFB address
 pop ecx ; B

 push dword 0

 jmp pimvga

      dbcblimitlset_vga:

 pop eax ; LFB address
 pop ecx ; B

 push dword 1

pimvga:
 push edi
 push esi
 push eax  ; LFB address
 push ecx  ; B
 push ebx  ; H
 push edx  ; x+Xwin

 mov ebx,[0x3010]
 mov bl,[ebx+0xe]
 mov bh,[esp+6*4]

 cld

 npvga:

   cmp bl,[ds:ebp]
   jnz impvgano

;   cmp bh,0
;   jz impvgayes

;   call voodoodbcplimit
;   jnz impvgano

; impvgayes:

   push eax  ; LFB address
   push ebx  ; app no.
   push ecx  ; B
   push edx  ; x+Xwin

   mov edx,[esi] ; color
   mov [eax],dx
   shr edx,16
   mov [eax+2],dl

   mov eax,[esi] ; color
   mov ecx,[esp] ; x+Xwin
   and ecx,0x07  ; modulo 8
   call setvgapixel ; eax=color, ecx=x%8, edi=VGA address

   pop edx
   pop ecx
   pop ebx
   pop eax

 impvgano:

   add esi,3 ; esi+=3 ptrImage+=3
   add eax,4 ; eax+=4 LFBaddr +=4
   inc ebp
   inc edx ; x+Xwin+n

   test edx,0x07  ; test modulo 8
   jnz impvgacont
   inc edi

        impvgacont:
   dec ecx ; B--
   jnz npvga

      pop edx
      pop ebx
      pop ecx
      pop eax
      pop esi
      pop edi

      add edi,640/8  ; add one VGA line
      add eax,640*4  ; add one LFB line

      sub ebp, ecx ;  -B
      add ebp, [0xfe00] ;
      inc ebp ; ptrBuffer:=ptrBuffer-B+Screen_Xsize+1

      push ecx
      lea ecx,[ecx+ecx*2] ;
      add esi,ecx ; ptrImage:=ptrImage+B*3
      pop ecx

      dec ebx ; H--
      jnz near pimvga

      add esp,4  ; jump saved limit byte
      pop edx
      pop ecx
      pop ebx
      pop eax
      pop edi
      pop esi
      pop ebp

      ret


VGA_putpixel:

 ; eax = x
 ; ebx = y

 mov     ecx,eax
 mov     eax, [esp+32-8+4] ; color

 imul    ebx, 640*4        ; y*BytesPerLine (Vesa2.0 32)
 lea     edx, [ebx+ecx*4]  ; + x*BytesPerPixel (Vesa2.0 32)

 mov     edi,edx
 add     edi, [0xfe80]     ; + LFB address
 mov     [edi], eax        ; write to LFB for Vesa2.0

 shr     edx,5             ; change BytesPerPixel to 1/8
 mov     edi,edx
 add     edi, 0x0a0000     ; address of pixel in VGA area

 and     ecx,0x07          ; bit no. (modulo 8)

setvgapixel:

 ; edi = address, eax = 24bit colour, ecx = bit no. (modulo 8)

 push   eax
 mov    ebx,eax            ; color

 ;mov    al,0x08
 ;mov    dx,0x03ce
 ;out    dx,al             ; select GDC bit mask register

 inc    cl
 mov    ax, 0x100
 shr    ax,cl
 mov    dx,0x03cf
 out    dx,al              ; set bit mask for pixel

 mov    dl,0
 mov    eax,ebx
 and    eax,0x000000ff     ; blue
 cmp    eax,85
 jle    p13green
 or     dl,0x01
 cmp    eax,170
 jle    p13green
 or     dl,0x08

p13green:
 and    ebx,0x0000ff00     ; green
 cmp    ebx,85*256
 jle    p13red
 or     dl,0x02
 cmp    ebx,170*256
 jle    p13red
 or     dl,0x08

p13red:
 pop    ebx
 and    ebx,0x00ff0000     ; red
 cmp    ebx,85*256*256
 jle    p13cont
 or     dl,0x04
 cmp    ebx,170*256*256
 jle    p13cont
 or     dl,0x08

p13cont:
 mov    al,[edi]           ; dummy read
 mov    [edi],dl

 ret


vga_drawbar:

     sub edx,ebx ; edx:=Yend-Ystart=H
     sub ecx,eax ; ecx:=Xend-Xstat=B

     push ebp ; +24
     push esi ; +20
     push edi ; +16
     push eax ; +12
     push ebx ; +8
     push ecx ; +4
     push edx ; +0

     mov ecx,[0x3010] ;
     add eax,[ecx-twdw] ; eax:=Xwin+x
     add ebx,[ecx-twdw+4] ; ebx:=Ywin+y
     mov ecx, eax ; ecx:=(x+Xwin)
     mov edx, ebx ; edx:=(y+Ywin)

     imul ebx, 640/8  ;
     mov edi, ebx ; edi:=BytesPerScanLine*(y+Ywin)
     shr eax, 3 ;
     add edi, eax ;  + (x+Xwin)*BytesPerPixel
     add edi,0xa0000  ; + VGAbaseaddress

     mov eax, [0xfe00] ; ScreenXSize
     inc eax ; +1
     imul eax,edx ; *(y+Ywin)
     mov ebp, eax ;
     add ebp, ecx ; +(x+Win)
     add ebp, WinMapAddress ; +AddrBuffer

     mov eax, [0xfe08]  ; BytesPerScanLine - LFB
     mul edx            ; *(y+Ywin)
     mov esi,eax
     add esi,ecx
     add esi,ecx
     add esi,ecx
     add esi,ecx        ; + 4*(x+Xwin)
     add esi,[0xfe80]   ; +AddrLFB

; edi:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel + AddrVGA
; esi:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel + AddrLFB
; ebp:=(y+Ywin)*(ScreenXSize+1)+(x+Xwin)+AddrBuffer

; x size

     mov eax,[esp+4] ; B [esp+4]
     mov ebx,[esp+0] ; H

     mov edx,[esp+16] ; color
     test edx,0x80000000
     jz nodbglvga

     ; no color glide for VGA - set to half glide
     shr ebx,1  ; H/2
     sub edx,ebx
     mov [esp+16],edx
     mov ebx,[esp+0] ; reset to H

   nodbglvga:
     ; check limits ?

     push eax
     push ecx

     mov eax,[0x3010]

     mov ecx,[eax+draw_data-0x3000+0]
     cmp ecx,0
     jnz dbcblimitlset_vga2

     mov ecx,[eax+draw_data-0x3000+4]
     cmp ecx,0
     jnz dbcblimitlset_vga2

     mov ecx,[eax+draw_data-0x3000+8]
     cmp ecx,[0xfe00]
     jnz dbcblimitlset_vga2

     mov ecx,[eax+draw_data-0x3000+12]
     cmp ecx,[0xfe04]
     jnz dbcblimitlset_vga2

     pop ecx
     pop eax

     push dword 0

     jmp dbnewpivga

   dbcblimitlset_vga2:

     pop ecx ; x+Xwin
     pop eax ; B

     push dword 1

   dbnewpivga:

     push eax; B
     push ebx ; H
     push edi
     push esi
     push ecx ; x+Xwin

     mov   ebx,[0x3010]
     movzx ebx,byte[ebx+0xe]

     cld

     dbnpvga:

       mov dl,[ds:ebp]

       cmp dl,bl
       jnz dbimpvgano

;       mov edx,[esp+5*4] ; check limit?
;       cmp edx,0
;       jz dbimpvgayes

;       call voodoodbcplimit
;       jnz  dbimpvgano

;     dbimpvgayes:

       push eax ; B
       push ebx
       push ecx ; x+Xwin

       mov eax,[esp+12+20+16+4] ; color
       mov ebx,eax

       mov [esi],bx    ; write LFB pixel
       shr  ebx,16
       mov  [esi+2],bl

       and ecx,0x07 ; modulo 8
       call setvgapixel ; eax=color, ecx=x%8, edi=VGA address

       pop ecx
       pop ebx
       pop eax

     dbimpvgano:

       add esi,4    ; ptrLFB+=4
       inc ebp  ; address buffer
       inc ecx  ; x posn++
       test ecx,0x07  ; test modulo 8
       jnz dbvgacont
       inc edi  ; VGA screen ptr++

     dbvgacont:
       dec eax  ; B--  NB ecx in Vesa20 fn?
       jnz dbnpvga

   dbnpvgad:

     pop ecx
     pop esi
     pop edi
     pop ebx
     pop eax

     add esi,[0xfe08]            ; ptrLFB+=BytesPerScanLine
     add edi,640/8               ; ptrScreen+=BytesPerScanLine

     add ebp,[0xfe00]            ;
     sub ebp, eax                ; was ecx in vesa20 fn?
     inc ebp                     ; ptrBuffer:=ptrBuffer-B+BytesPerLine+1

     dec ebx                     ; H--
     jz nodbnewpivga             ; H<>0

     jmp dbnewpivga

   nodbnewpivga:

     add esp,7*4   ; NB includes limit check flag
     ;pop ebx
     ;pop eax
     ;pop edi
     ;pop esi
     pop ebp

     ;pop edx
     ;pop ecx

     ret


vga_drawbackground_tiled:

     push ebp
     push eax
     push ebx
     push ecx
     push edx

     mov edx,dword [0x400000-8] ; B
     add edx,dword [WinMapAddress-8] ; +B
     add edx,dword [WinMapAddress-8] ; +B
     push edx

     mov eax,[draw_data+32+0] ; x start:=(x+Xwin)
     mov ebx,[draw_data+32+4] ; y start:=(y+Ywin)
     mov ecx,eax
     mov edx,ebx

     imul edx, 640*4          ; (y+Ywin)*BytesPerScanLine
     shl ecx,2                ; (x+Xwin)*BytesPerPixel
     add ecx,edx              ;
     mov ebp,ecx              ; store copy
     add ecx,[0xfe80]         ; +AddrLFB
  ;entry point in LFB >> ECX:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel+Addr

     shr ebp,5                ; change from 4 to 1/8 BytesPerPixel
     add ebp,0xa0000          ; + VGABasePtr
  ;VGA start address >> EBP:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel+AddrV


     call calculate_edi

   dp3vga:                            ; MAIN LOOP

     cmp [edi+WinMapAddress],byte 1     ; ptrBuffer^<>byte(1)
     je  ybgpvga

     jmp nbgpvga

   ybgpvga:

     push eax  ; x
     push ebx  ; y
     push ecx  ; LFB address

     mov ecx,dword [WinMapAddress-8]    ; B
     xor edx,edx                   ; edx:=0
     div ecx                       ; Xstart/B

     ; eax=Int(qn) edx:=Rem

     lea esi,[edx+edx*2]           ; esi:=edx*3

     mov ecx,dword [WinMapAddress-4]    ; ecx:=H
     mov eax,[esp+4]               ; eax:=Ystart
     xor edx,edx                   ;
     div ecx                       ; Ystart/H

     mov eax,edx                   ; eax:=Rem
     xor edx,edx                   ;
     mov ebx,[esp+12]              ; ebx:=B*3
     mul ebx                       ;
     add esi,eax                   ;

     mov eax,[esi+0x300000]        ; color
     and eax,0xffffff

     mov ecx, [esp]   ; LFB address
     mov ebx,eax      ; copy color
     mov [ecx],bx
     shr ebx,16
     mov [ecx+2],bl

     xchg edi, ebp
     mov  ecx,[esp+8]     ; x position
     and  ecx,0x07        ; x modulo 8
     call setvgapixel     ; eax=color, ecx=x%8, edi=VGA address
     xchg ebp, edi

     pop ecx
     pop ebx
     pop eax

   nbgpvga:

     inc eax                       ; x++
     cmp eax,[draw_data+32+8]         ; X > xend?
     jg  nodp3vga

     test eax,0x07                 ; x test modulo 8
     jnz hook1vga
     inc ebp                       ; VGA address++

   hook1vga:
     add ecx,4                     ; LFB address += 4
     inc edi                       ; ptrBuffer++
     add esi,3                     ; ptrImage+=3
     jmp dp3vga

   nodp3vga:

     mov eax,[draw_data+32+0]         ; x+Xwin
     inc ebx                       ; y position
     mov ecx,eax
     mov edx,ebx

     imul edx, 640*4          ; (y+Ywin)*BytesPerScanLine
     shl ecx,2                ; (x+Xwin)*BytesPerPixel
     add ecx,edx              ;
     mov ebp,ecx              ; store copy
     add ecx,[0xfe80]         ; +AddrLFB
    ;entry point in LFB >> ECX:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel+Addr


     shr ebp,5                ; change from 4 to 1/8 BytesPerPixel
     add ebp,0xa0000          ; + VGABasePtr
   ;VGA start address >> EBP:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel+AddrV


     call calculate_edi

     cmp ebx,[draw_data+32+12]         ; Y > yend
     jg  dp4vga

     jmp dp3vga

   dp4vga:

     add esp,4

     pop edx
     pop ecx
     pop ebx
     pop eax
     pop ebp

     ret

; ----------



vga_drawbackground_stretch:


     push ebp
     push eax
     push ebx
     push ecx
     push edx

     mov edx,dword [WinMapAddress-8] ; B
     add edx,dword [WinMapAddress-8] ; +B
     add edx,dword [WinMapAddress-8] ; +B
     push edx

     mov eax,[draw_data+32+0] ; x start:=(x+Xwin)
     mov ebx,[draw_data+32+4] ; y start:=(y+Ywin)
     mov ecx,eax
     mov edx,ebx

     imul edx, 640*4          ; (y+Ywin)*BytesPerScanLine
     shl ecx,2                ; (x+Xwin)*BytesPerPixel
     add ecx,edx              ;
     mov ebp,ecx              ; store copy
     add ecx,[0xfe80]         ; +AddrLFB
   ;entry point in LFB >> ECX:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel+Addr

     shr ebp,5                ; change from 4 to 1/8 BytesPerPixel
     add ebp,0xa0000          ; + VGABasePtr
   ;VGA start address >> EBP:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel+AddrV


     call calculate_edi

   sdp3vga:                            ; MAIN LOOP

     cmp [edi+WinMapAddress],byte 1     ; ptrBuffer^<>byte(1)
     je  sybgpvga

     jmp snbgpvga

   sybgpvga:

     push eax   ; x
     push ebx   ; y
     push ecx   ; LFB address

     mov   eax,dword [WinMapAddress-8]  ; B
     xor   edx,edx
     mov   ebx,[esp+8]             ; Xstart
     mul   ebx                     ; B*Xstart
     xor   edx,edx
     mov   ebx,[0xfe00]            ; x screen width
     div   ebx                     ; B*Xstart/xwidth
     lea   esi,[eax+eax*2]         ; *3
     mov   eax,dword [WinMapAddress-4]  ; H
     xor   edx,edx
     mov   ebx,[esp+4]             ; Ystart
     mul   ebx                     ; H*Ystart
     xor   edx,edx
     mov   ebx,[0xfe04]            ; y screen height
     div   ebx                     ; H*Ystart/yheight

     xor   edx,edx
     mov   ebx,[esp+12]             ; B*3
     mul   ebx                     ;
     add   esi,eax
     mov   eax,[esi+0x300000]      ; color
     and   eax,0xffffff

     mov   ecx, [esp]   ; LFB address
     mov   ebx,eax      ; copy color
     mov   [ecx],bx
     shr   ebx,16
     mov   [ecx+2],bl

     xchg  edi, ebp
     mov   ecx,[esp+8]     ; x position
     and   ecx,0x07        ; x modulo 8
     call  setvgapixel     ; eax=color, ecx=x%8, edi=VGA address
     xchg  ebp, edi        ; ebp+=3

     pop ecx
     pop ebx
     pop eax

   snbgpvga:

     inc eax                       ; x++
     cmp eax,[draw_data+32+8]         ; X > xend?
     jg  snodp3vga

     test eax,0x07                 ; x test modulo 8
     jnz shook1vga
     inc ebp                       ; VGA address++

   shook1vga:
     add ecx,4                     ; LFB address += 4
     inc edi                       ; ptrBuffer++
     add esi,3                     ; ptrImage+=3
     jmp sdp3vga

   snodp3vga:

     mov eax,[draw_data+32+0]         ; x+Xwin
     inc ebx                       ; y position
     mov ecx,eax
     mov edx,ebx

     imul edx, 640*4          ; (y+Ywin)*BytesPerScanLine
     shl ecx,2                ; (x+Xwin)*BytesPerPixel
     add ecx,edx              ;
     mov ebp,ecx              ; store copy
     add ecx,[0xfe80]         ; +AddrLFB
  ;entry point in LFB >> ECX:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel+Addr


     shr ebp,5                ; change from 4 to 1/8 BytesPerPixel
     add ebp,0xa0000          ; + VGABasePtr
          ;VGA start address >> EBP:=(y+Ywin)*BytesPerScanLine+X*BytesPerPixel+A


     call calculate_edi

     cmp ebx,[draw_data+32+12]        ; Y > yend
     jg  sdp4vga

     jmp sdp3vga

   sdp4vga:

     add esp,4

     pop edx
     pop ecx
     pop ebx
     pop eax
     pop ebp

     ret


