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

encoding_copyrect:

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
        lea     esi, [framebuffer_data+eax*3]           ; esi = framebuffer_data+([screen.width]*[src.y]+[src.x])*3

        movzx   eax, [rectangle.y]
        movzx   ebx, [screen.width]
        mul     ebx                                     ; [screen.width]*[rectangle.y]
        movzx   ebx, [rectangle.x]
        add     eax, ebx                                ; [screen.width]*[rectangle.y]+[rectangle.x]
        lea     edi, [framebuffer_data+eax*3]           ; edi = framebuffer_data+([screen.width]*[rectangle.y]+[rectangle.x])*3

        movzx   eax, [screen.width]
        sub     ax, [rectangle.width]
        lea     ebp, [eax*3]                            ; ebp = ([screen.width]-[rectangle.width])*3

        cmp     esi, edi
        ja      .copy

        ; source pixels come before destination in buffer, copy backwards
        movzx   eax, [rectangle.height]
        movzx   edx, [screen.width]
        mul     edx
        lea     eax, [eax*3-1]
        add     esi, eax
        add     edi, eax

        neg     ebp
        std
  .copy:

        movzx   edx, [rectangle.height]
        movzx   ecx, [rectangle.width]
        lea     ecx, [ecx*3]
  .lineloop:
        push    ecx
        rep     movsb
        pop     ecx

        add     esi, ebp
        add     edi, ebp
        dec     edx
        jnz     .lineloop

        pop     esi
        add     esi, 4
        cld
        jmp     next_rectangle