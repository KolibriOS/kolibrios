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

pixel_to_24bpp:         ; returns in ecx

        push    eax
  @@:
        lea     eax, [esi+BYTES_PER_PIXEL]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

if BITS_PER_PIXEL = 8

        push    ebx

        mov     bl, 36
        mov     al, [esi]
        and     al, 7
        mul     bl
        mov     ch, al          ; red

        mov     al, [esi]
        shr     al, 3
        and     al, 7
        mul     bl
        mov     cl, al          ; green

        mov     bl, 85
        mov     al, [esi]
        shr     al, 6
        and     al, 3
        mul     bl
        shl     ecx, 8
        mov     cl, al          ; blue

        inc     esi
        pop     ebx

else if BITS_PER_PIXEL = 16

        lodsw
        mov     cl, ah
        and     al, 0xf8        ; red

        mov     cx, ax
        shl     cx, 5
        and     ch, 0xfc        ; green
        shl     ecx, 8

        mov     cl, al
        shl     cl, 3
        and     cx, 0x00f8      ; blue

else

        xor     ecx, ecx
        mov     cx, [esi]
        shl     ecx, 8
        mov     cl, [esi+2]
        add     esi, 3

end if
        pop     eax

        ret

encoding_RRE:

        DEBUGF  1,"RRE\n"

  @@:
        lea     eax, [esi+4+BYTES_PER_PIXEL]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

        lodsd
        bswap   eax
        mov     [subrectangles], eax

        DEBUGF  1, "%u subrectangles\n", eax

; Get background color
        call    pixel_to_24bpp

; Calculate first pixel pos
        movzx   eax, [screen.width]
        mul     [rectangle.y]                           ; [screen.width]*[rectangle.y]
        add     eax, [rectangle.x]                      ; [screen.width]*[rectangle.y]+[rectangle.x]
        lea     edi, [framebuffer_data+eax*3]           ; edi = framebuffer_data+([screen.width]*[rectangle.y]+[rectangle.x])*3

; Calculate offset between two rows of pixels
        movzx   eax, [screen.width]
        sub     eax, [rectangle.width]
        lea     ebp, [eax*3]                            ; ebp = ([screen.width]-[rectangle.width])*3

; Draw background rectangle
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

; Any subrectangles at all?
        cmp     [subrectangles], 0
        je      next_rectangle

  .subrectangle:
  @@:
        lea     eax, [esi+8+BYTES_PER_PIXEL]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

; Get subrectangle color
        call    pixel_to_24bpp

; Get coordinates
        xor     eax, eax
        lodsw
        xchg    al, ah
        mov     [subrectangle.x], eax
        lodsw
        xchg    al, ah
        mov     [subrectangle.y], eax
        lodsw
        xchg    al, ah
        mov     [subrectangle.width], eax
        lodsw
        xchg    al, ah
        mov     [subrectangle.height], eax
        DEBUGF  1, "Subrectangle: x=%u y=%u width=%u height=%u\n", \
        [subrectangle.x], [subrectangle.y], [subrectangle.width], [subrectangle.height]

; Calculate pos of first pixel
        push    edi
        movzx   eax, [screen.width]
        mul     [subrectangle.y]
        add     eax, [subrectangle.x]
        lea     eax, [eax*3]
        add     edi, eax

; Calculate offset between two rows of pixels
        movzx   eax, [screen.width]
        sub     eax, [subrectangle.width]
        lea     ebp, [eax*3]                            ; ebp = ([screen.width]-[rectangle.width])*3

; Draw the subrectangle
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