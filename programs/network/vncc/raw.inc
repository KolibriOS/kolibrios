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

encoding_raw:

        DEBUGF  1,"RAW\n"

        mov     eax, [rectangle.width]
        mov     ebx, [rectangle.height]
        mul     ebx
if BITS_PER_PIXEL = 16
        shl     eax, 1
else if BITS_PER_PIXEL = 24
        lea     eax, [eax*2+eax]
end if
  @@:
        push    eax
        add     eax, esi
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        pop     eax
        jmp     @b
  @@:
        pop     eax

        mov     eax, [rectangle.y]
        movzx   ebx, [screen.width]
        mul     ebx                                     ; [screen.width]*[rectangle.y]
        add     eax, [rectangle.x]                      ; [screen.width]*[rectangle.y]+[rectangle.x]
        lea     edi, [framebuffer_data+eax*3]           ; edi = framebuffer_data+([screen.width]*[rectangle.y]+[rectangle.x])*3

        movzx   eax, [screen.width]
        sub     eax, [rectangle.width]
        lea     ebp, [eax*3]                            ; ebp = ([screen.width]-[rectangle.width])*3

        mov     edx, [rectangle.height]

  .lineloop:
        mov     ecx, [rectangle.width]
if BITS_PER_PIXEL = 24
        lea     ecx, [ecx*2+ecx]
end if

if BITS_PER_PIXEL = 8
  .pixelloop:
        mov     bl, 85
        mov     al, [esi]
        shr     al, 6
        and     al, 3
        mul     bl
        stosb                   ; blue
        mov     bl, 36
        mov     al, [esi]
        shr     al, 3
        and     al, 7
        mul     bl
        stosb                   ; green
        mov     al, [esi]
        and     al, 7
        mul     bl
        stosb                   ; red
        inc     esi
        dec     ecx
        jnz     .pixelloop
else if BITS_PER_PIXEL = 16
  .pixelloop:
        lodsw
        mov     bx, ax
        shl     al, 3
        and     al, 0xf8
        stosb                   ; blue
        mov     ax, bx
        shr     ax, 3
        and     al, 0xfc
        stosb                   ; green
        mov     al, bh
        and     al, 0xf8
        stosb                   ; red
        dec     ecx
        jnz     .pixelloop
else if BITS_PER_PIXEL = 24
        rep movsb
end if

        add     edi, ebp
        dec     edx
        jnz     .lineloop
        jmp     next_rectangle
