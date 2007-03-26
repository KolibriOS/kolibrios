$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  VGA.INC                                                     ;;
;;                                                              ;;
;;  640x480 mode 0x12 VGA functions for MenuetOS                ;;
;;                                                              ;;
;;  Paul Butcher, paul.butcher@asa.co.uk                        ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



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
;       mov    dx, 3ceh
;       mov    ax, 0005h
;       out    dx, ax
       ret

palette320x200:

       mov   edx,0x3c8
       xor   eax, eax
       out   dx,al
       mov   ecx,256
       mov   edx,0x3c9
       xor   eax,eax

     palnew:
       mov   al,0
       test  ah,64
       jz    pallbl1
       add   al,21
     pallbl1:
       test  ah,128
       jz    pallbl2
       add   al,42
     pallbl2:
       out   dx,al
       mov   al,0
       test  ah,8
       jz    pallbl3
       add   al,8
     pallbl3:
       test  ah,16
       jz    pallbl4
       add   al,15
     pallbl4:
       test  ah,32
       jz    pallbl5
       add   al,40
     pallbl5:
       out   dx,al
       mov   al,0
       test  ah,1
       jz    pallbl6
       add   al,8
     pallbl6:
       test  ah,2
       jz    pallbl7
       add   al,15
     pallbl7:
       test  ah,4
       jz    pallbl8
       add   al,40
     pallbl8:
       out   dx,al
       add   ah,1
       loop  palnew

       ret

uglobal
  novesachecksum dd 0x0
  EGA_counter    db  0
  VGA_drawing_screen db 0
  VGA_8_pixels:
     rb 16
  temp:
     .cx dd 0
endg

checkVga_N13:

        cmp    [SCR_MODE],dword 0x13
        jne   @f

;      cnvl:
        pushad
        cmp   [EGA_counter],1
        je   novesal
        mov    ecx,[MOUSE_X]
        cmp    ecx,[novesachecksum]
        jne    novesal
        popad
      @@:
        ret

      novesal:
        mov    [novesachecksum],ecx
        mov    ecx,0
        movzx  eax,word [0xfb0c]
        cmp    eax,100
        jge    m13l3
        mov    eax,100
      m13l3:
        cmp    eax,480-100
        jbe    m13l4
        mov    eax,480-100
      m13l4:
        sub    eax,100
        imul   eax,640*4
        add    ecx,eax
        movzx  eax,word [MOUSE_X]
        cmp    eax,160
        jge    m13l1
        mov    eax,160
      m13l1:
        cmp    eax,640-160
        jbe    m13l2
        mov    eax,640-160
      m13l2:
        sub    eax,160
        shl    eax,2
        add    ecx,eax
        mov    esi,[LFBAddress]
        add    esi,ecx
        mov    edi,VGABasePtr
        mov    edx,200
        mov    ecx,320
        cld
     m13pix:
        lodsd
        cmp    eax,0
        je    .save_pixel
        push   eax
        mov    ebx,eax
        and    eax,(128+64+32)      ; blue
        shr    eax,5
        and    ebx,(128+64+32)*256  ; green
        shr    ebx,8+2
        add    eax,ebx
        pop    ebx
        and    ebx,(128+64)*256*256 ; red
        shr    ebx,8+8
        add    eax,ebx
     .save_pixel:
        stosb
        loop   m13pix
        mov    ecx,320
        add    esi,4*(640-320)
        dec    edx
        jnz    m13pix
        mov    [EGA_counter],0
        popad
        ret

VGA_drawbackground:
; draw all
        cmp    [SCR_MODE],dword 0x12
        jne     .end
        pushad
        mov    esi,[LFBAddress]
        mov    edi,VGABasePtr
        mov    ebx,640/32  ; 640*480/(8*4)
        mov    edx,480
     @@:
        push   ebx edx esi edi
        shl    edx,9
        lea    edx,[edx+edx*4]
        add    esi,edx
        shr    edx,5
        add    edi,edx
        call   VGA_draw_long_line
        pop    edi esi edx ebx
        dec    edx
        jnz    @r
        call   VGA_draw_long_line_1
        popad
    .end:
        ret

VGA_draw_long_line:
        mov    dx,3ceh
        mov    ax,0ff08h
        cli
        out    dx, ax
        mov    ax,0005h
        out    dx, ax
    m12pix:
        call   VGA_draw_32_pixels
        dec    ebx
        jnz    m12pix
        mov    dx,3c4h
        mov    ax,0ff02h
        out    dx,ax
        mov    dx,3ceh
        mov    ax,0205h
        out    dx,ax
        mov    dx,3ceh
        mov    al,08h
        out    dx,al
        sti
        ret

VGA_draw_32_pixels:
        xor    eax,eax
        mov    ebp,VGA_8_pixels
        mov    [ebp],eax
        mov    [ebp+4],eax
        mov    [ebp+8],eax
        mov    [ebp+12],eax
        mov    ch,4
    .main_loop:
        mov    cl,8
    .convert_pixels_to_VGA:
        lodsd   ; eax = 24bit colour
        cmp    eax,0
        je     .end
        rol    eax,8
        mov    al,ch
        ror    eax,8
        mov    ch,1
        dec    cl
        shl    ch,cl
        cmp    al,85
        jbe     .p13green
        or     [ebp],ch
        cmp    al,170
        jbe     .p13green
        or     [ebp+12],ch
    .p13green:
        cmp    ah,85
        jbe     .p13red
        or     [ebp+4],ch
        cmp    ah,170
        jbe     .p13red
        or     [ebp+12],ch
    .p13red:
        shr    eax,8
        cmp    ah,85
        jbe     .p13cont
        or     [ebp+8],ch
        cmp    ah,170
        jbe     .p13cont
        or     [ebp+12],ch
    .p13cont:
        ror    eax,8
        mov    ch,ah
        inc    cl
    .end:
        dec    cl
        jnz    .convert_pixels_to_VGA
        inc    ebp
        dec    ch
        jnz    .main_loop
        push   esi
        sub    ebp,4
        mov    esi,ebp
        mov    dx, 3c4h
        mov    ah, 1h
    @@:
        mov    al, 02h
        out    dx,ax
        xchg   ax,bp
        lodsd
        mov    [edi],eax
        xchg   ax,bp
        shl    ah, 1
        cmp    ah, 10h
        jnz    @r
        add    edi,4
        pop    esi
        ret

VGA_putpixel:
 ; eax = x
 ; ebx = y
        mov     ecx,eax
        mov     eax, [esp+32-8+4] ; color
        shl        ebx,9
        lea        ebx,[ebx+ebx*4] ; умножение на 5
        lea     edx, [ebx+ecx*4]  ; + x*BytesPerPixel (Vesa2.0 32)
        mov     edi,edx
        add     edi, [LFBAddress]     ; + LFB address
        mov     [edi], eax        ; write to LFB for Vesa2.0
        shr     edx,5             ; change BytesPerPixel to 1/8
        mov     edi,edx
        add     edi, VGABasePtr     ; address of pixel in VGA area
        and     ecx,0x07          ; bit no. (modulo 8)
        pushfd
        ; edi = address, eax = 24bit colour, ecx = bit no. (modulo 8)
        xor    edx,edx
        cmp    eax,0
        je     .p13cont
        cmp    al,85
        jbe    .p13green
        or     dl,0x01
        cmp    al,170
        jbe    .p13green
        or     dl,0x08
.p13green:
        cmp    ah,85
        jbe    .p13red
        or     dl,0x02
        cmp    ah,170
        jbe    .p13red
        or     dl,0x08
.p13red:
        shr    eax,8
        cmp    ah,85
        jbe    .p13cont
        or     dl,0x04
        cmp    ah,170
        jbe    .p13cont
        or     dl,0x08
.p13cont:
        ror    edx,8
        inc    cl
        xor    eax,eax
        inc    ah
        shr    ax,cl
        mov    dx,3cfh
        cli
        out    dx,al
        mov    al,[edi]           ; dummy read
        rol    edx,8
        mov    [edi],dl
        popfd
;.end:
        ret

VGA__putimage:
; ecx = size [x|y]
; edx = coordinates [x|y]
        cmp    [SCR_MODE],dword 0x12
        jne     @f
         pushad
         rol  edx,16
         movzx eax,dx
         rol  edx,16
         movzx ebx,dx
         movzx edx,cx
         rol   ecx,16
         movzx ecx,cx
         call  VGA_draw_bar_1
         popad
@@:
         ret

VGA_draw_bar:
; eax   cx
; ebx   cy
; ecx   xe
; edx   ye
        cmp    [SCR_MODE],dword 0x12
        jne     @f
         pushad
         sub   ecx,eax
         sub   edx,ebx
         and   eax,0xffff
         and   ebx,0xffff
         and   ecx,0xffff
         and   edx,0xffff
         call  VGA_draw_bar_1
         popad
@@:
         ret

VGA_draw_bar_1:
        mov     [temp.cx],eax
        mov     eax, [TASK_BASE]
        add     ebx, [eax-twdw + 4]
        mov     eax, [eax-twdw + 0]
        add     eax, [temp.cx]
        and     eax,0xfff8
        shl     ebx,9
        lea     ebx,[ebx+ebx*4] ; умножение на 5
        lea     ebx, [ebx+eax*4]  ; + x*BytesPerPixel (Vesa2.0 32)
        mov     esi,ebx
        add     esi, [LFBAddress]     ; + LFB address
        shr     ebx,5             ; change BytesPerPixel to 1/8
        mov     edi,ebx
        add     edi, VGABasePtr   ; address of pixel in VGA area
        mov     ebx,ecx
        shr     ebx,5
        inc     ebx
.main_loop:
        call   VGA_draw_long_line_1
        dec    edx
        jnz    .main_loop
        call   VGA_draw_long_line_1
        ret

VGA_draw_long_line_1:
        push   ebx edx esi edi
        shl    edx,9
        lea    edx,[edx+edx*4]
        add    esi,edx
        shr    edx,5
        add    edi,edx
        call   VGA_draw_long_line
        pop    edi esi edx ebx
        ret


