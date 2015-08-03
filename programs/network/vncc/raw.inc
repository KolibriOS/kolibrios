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

        movzx   eax, [rectangle.width]
        movzx   ebx, [rectangle.height]
        mul     ebx
        add     eax, esi
  @@:
        cmp     [datapointer], eax
        jae     @f
        push    eax
        call    read_data.more
        pop     eax
        jmp     @b
  @@:

        movzx   eax, [rectangle.y]
        movzx   ebx, [screen.width]
        mul     ebx                                     ; [screen.width]*[rectangle.y]
        movzx   ebx, [rectangle.x]
        add     eax, ebx                                ; [screen.width]*[rectangle.y]+[rectangle.x]
        lea     edi, [framebuffer_data+eax*3]           ; edi = framebuffer_data+([screen.width]*[rectangle.y]+[rectangle.x])*3

        movzx   eax, [screen.width]
        sub     ax, [rectangle.width]
        lea     ebp, [eax*3]                            ; ebp = ([screen.width]-[rectangle.width])*3

        mov     bl, 85

        movzx   edx, [rectangle.height]

  .lineloop:
        movzx   ecx, [rectangle.width]

  .pixelloop:
        mov     al, [esi]
        shr     al, 4
        and     al, 3
        mul     bl
        stosb

        mov     al, [esi]
        shr     al, 2
        and     al, 3
        mul     bl
        stosb

        mov     al, [esi]
        and     al, 3
        mul     bl
        stosb

        inc     esi
        dec     ecx
        jnz     .pixelloop

        add     edi, ebp
        dec     edx
        jnz     .lineloop
        jmp     next_rectangle



