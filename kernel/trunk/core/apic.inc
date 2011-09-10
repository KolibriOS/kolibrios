;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
rerouteirqs:

        cli
        mov     al,0x11         ;  icw4, edge triggered
        out     0x20,al
        out     0xA0,al

        mov     al,0x20         ;  generate 0x20 +
        out     0x21,al
        mov     al,0x28         ;  generate 0x28 +
        out     0xA1,al

        mov     al,0x04         ;  slave at irq2
        out     0x21,al
        mov     al,0x02         ;  at irq9
        out     0xA1,al

        mov     al,0x01         ;  8086 mode
        out     0x21,al
        out     0xA1,al

        mov     al,255          ; mask all irq's
        out     0xA1,al
        out     0x21,al

        mov     al,255          ; mask all irq's
        out     0xA1,al
        out     0x21,al
        ret


align 4
;proc enable_irq stdcall, irq_line:dword
enable_irq:                                ; FIXME make fastcall
       mov ebx, [esp+4]  ;irq_line
       mov edx, 0x21
       cmp ebx, 8
       jb @F
       mov edx, 0xA1
       sub ebx,8
@@:
       in al,dx
       btr eax, ebx
       out dx, al
       ret 4


align 4
;proc irq_eoi fastcall, irq_line:dword
irq_eoi:
       cmp cl, 8
       mov al, 0x20
       jb @f
       out 0xa0, al
@@:
       out 0x20, al
       ret
