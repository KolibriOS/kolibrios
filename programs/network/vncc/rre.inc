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

load_pixel:                     ; returns in eax

if BITS_PER_PIXEL = 8

        xor     eax, eax
        lodsb
        mov     eax, [lut_8bpp+eax*4]

else if BITS_PER_PIXEL = 16

        push    ebx
        lodsw
        mov     bx, ax
        shl     bx, 5
        and     bh, 0xfc        ; green

        mov     bl, ah
        and     bl, 0xf8        ; red
        shl     ebx, 8

        mov     bl, al
        shl     bl, 3
        and     bl, 0xf8        ; blue
        mov     eax, ebx
        pop     ebx

else if BITS_PER_PIXEL = 24

        mov     eax, [esi]
        and     eax, 0x00ffffff
        add     esi, 3

else if BITS_PER_PIXEL = 32

        lodsd

end if

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

; Calculate first pixel pos
        movzx   eax, [screen.width]
        mul     [rectangle.y]                           ; [screen.width]*[rectangle.y]
        add     eax, [rectangle.x]                      ; [screen.width]*[rectangle.y]+[rectangle.x]
        lea     edi, [framebuffer+eax*3]                ; edi = framebuffer_data+([screen.width]*[rectangle.y]+[rectangle.x])*3

; Calculate offset between two rows of pixels
        movzx   eax, [screen.width]
        sub     eax, [rectangle.width]
        lea     ebp, [eax*3]                            ; ebp = ([screen.width]-[rectangle.width])*3

; Get background color
        call    load_pixel

; Draw background rectangle
        push    edi
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
        push    edi
  @@:
        lea     eax, [esi+8+BYTES_PER_PIXEL]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

; Get subrectangle color
        call    load_pixel
        push    eax

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
        pop     eax
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

