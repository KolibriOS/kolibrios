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

encoding_CopyRect:

        DEBUGF  1,"CopyRect\n"

  @@:
        lea     eax, [esi+4]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

        push    esi

        movzx   eax, word[esi+2]                        ; [src.y]
        xchg    al, ah
        movzx   ebx, [screen.width]
        mul     ebx                                     ; [screen.width]*[src.y]
        movzx   ebx, word[esi]                          ; [src.x]
        xchg    bl, bh
        add     eax, ebx                                ; [screen.width]*[src.y]+[src.x]
        lea     esi, [framebuffer+eax*3]                ; esi = framebuffer_data+([screen.width]*[src.y]+[src.x])*3

        mov     eax, [rectangle.y]
        movzx   ebx, [screen.width]
        mul     ebx                                     ; [screen.width]*[rectangle.y]
        add     eax, [rectangle.x]                      ; [screen.width]*[rectangle.y]+[rectangle.x]
        lea     edi, [framebuffer+eax*3]                ; edi = framebuffer_data+([screen.width]*[rectangle.y]+[rectangle.x])*3

        movzx   eax, [screen.width]
        sub     eax, [rectangle.width]
        lea     ebp, [eax*3]                            ; ebp = ([screen.width]-[rectangle.width])*3

        cmp     esi, edi
        ja      .not_backwards

; Source pixels come before destination in buffer, copy backwards.
        DEBUGF  1,"backwards\n"
        mov     eax, [rectangle.height]
        dec     eax
        movzx   edx, [screen.width]
        mul     edx
        add     eax, [rectangle.width]
        lea     eax, [eax*3-1]                          ; eax = (([rectangle.height]-1)*[screen.width]+[rectangle.width])*3-1
        add     esi, eax
        add     edi, eax
        neg     ebp
        std
  .not_backwards:

; Do the actual copying
        mov     eax, [rectangle.width]
        lea     eax, [eax*3]
        mov     ebx, eax
        and     eax, 11b
        shr     ebx, 2
        mov     edx, [rectangle.height]
  .lineloop:
        mov     ecx, eax
        rep movsb
        mov     ecx, ebx
        rep movsd
        add     esi, ebp
        add     edi, ebp
        dec     edx
        jnz     .lineloop

        pop     esi
        add     esi, 4
        cld
        jmp     next_rectangle