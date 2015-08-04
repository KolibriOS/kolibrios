;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2010-2015. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  VNC client for KolibriOS                                       ;;
;;                                                                 ;;
;;  Written by hidnplayr@kolibrios.org                             ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

pixel_to_24bpp:         ; returns in ecx, destroys eax, ebx
if BITS_PER_PIXEL = 8
; Convert pixel to 24BPP
        mov     bl, 85
        mov     al, [esi]
        shr     al, 6
        and     al, 3
        mul     bl
        mov     ch, al          ; blue
        mov     bl, 36
        mov     al, [esi]
        shr     al, 3
        and     al, 7
        mul     bl
        mov     cl, al          ; green
        mov     al, [esi]
        and     al, 7
        mul     bl
        shr     ecx, 8
        mov     cl, al          ; red
        inc     esi
else if BITS_PER_PIXEL = 16
        lodsw
        mov     cl, al
        shl     cl, 3
        and     cx, 0x00f8      ; blue
        shl     ecx, 16
        mov     cx, ax
        shl     cx, 5
        and     ch, 0xfc        ; green
        mov     cl, ah
        and     al, 0xf8        ; red
else
        xor     ecx, ecx
        mov     cx, [esi]
        shr     ecx, 8
        mov     cl, [esi+2]
        add     esi, 3
end if

        ret

encoding_RRE:

        DEBUGF  2,"RRE\n"

  @@:
        lea     eax, [esi+5]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

        lodsd
        bswap   eax
        mov     [subrectangles], eax

        call    pixel_to_24bpp

        movzx   eax, [screen.width]
        mul     [rectangle.y]                           ; [screen.width]*[rectangle.y]
        add     eax, [rectangle.x]                      ; [screen.width]*[rectangle.y]+[rectangle.x]
        lea     edi, [framebuffer_data+eax*3]           ; edi = framebuffer_data+([screen.width]*[rectangle.y]+[rectangle.x])*3

        movzx   eax, [screen.width]
        sub     eax, [rectangle.width]
        lea     ebp, [eax*3]                            ; ebp = ([screen.width]-[rectangle.width])*3

        push    edi
        mov     eax, ecx
        mov     edx, [rectangle.height]
  .lineloop:
        mov     ecx, [rectangle.width]
  .pixelloop:
        stosw
        rol     eax, 16
        stosb
        rol     eax, 16
        dec     ecx
        jnz     .pixelloop
        add     edi, ebp
        dec     edx
        jnz     .lineloop
        pop     edi

  .subrectangle:
  @@:
        lea     eax, [esi+9]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

        call    pixel_to_24bpp

        xor     eax, eax
        lodsw
        xchg    al, ah
        mov     [subrectangle.x], eax
        lodsw
        xchg    al, ah
        mov     [subrectangle.y], eax
        lodsw
        xchg    al, ah
        mov     [subrectangle.height], eax
        lodsw
        xchg    al, ah
        mov     [subrectangle.width], eax

        push    edi
        mov     eax, [rectangle.width]
        mul     [subrectangle.y]
        add     eax, [subrectangle.x]
        add     edi, eax

        mov     eax, ecx
        mov     edx, [subrectangle.height]
  .lineloop2:
        mov     ecx, [subrectangle.width]
  .pixelloop2:
        stosw
        rol     eax, 16
        stosb
        rol     eax, 16
        dec     ecx
        jnz     .pixelloop2
        add     edi, ebp
        dec     edx
        jnz     .lineloop2

        pop     edi

        dec     [subrectangles]
        jnz     .subrectangle
        jmp     next_rectangle