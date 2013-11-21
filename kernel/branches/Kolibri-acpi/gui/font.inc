;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

;------------------------------------------------------------------------------
align 4
dtext_asciiz_esi:                          ; for skins title out
        push    eax
        xor     eax, eax
        inc     eax
        jmp     dtext.1
;------------------------------------------------------------------------------
align 4
dtext:
; ebx x & y
; ecx style ( 0xX0000000 ) & color ( 0x00RRGGBB )
; X = ABnnb:
; nn = font
; A = 0 <=> output esi characters; otherwise output ASCIIZ string
; B = 1 <=> fill background with color eax
; edx start of text
; edi 1 force or user area for redirect
        push    eax
        xor     eax, eax
;--------------------------------------
align 4
.1:
        pushad
        movsx   eax, bx         ; eax=y
        sar     ebx, 16         ; ebx=x
        xchg    eax, ebx        ; eax=x, ebx=y
        cmp     esi, 255
        jb      .loop

        mov     esi, 255
;--------------------------------------
align 4
.loop:
        test    ecx, ecx
        js      .test_asciiz

        dec     esi
        js      .end

        jmp     @f
;--------------------------------------
align 4
.test_asciiz:
        cmp     byte [edx], 0
        jz      .end

        cmp     byte [esp+28], 1
        jne     @f

        dec     esi
        js      .end
;--------------------------------------
align 4
@@:
        inc     edx
        pushad
        movzx   edx, byte [edx-1]
        test    ecx, 0x10000000
        jnz     .font2

        mov     esi, 9
        lea     ebp, [FONT_I+8*edx+edx]
;--------------------------------------
align 4
.symloop1:
        mov     dl, byte [ebp]
        or      dl, 1 shl 6
;--------------------------------------
align 4
.pixloop1:
        shr     dl, 1
        jz      .pixloop1end

        jnc     .nopix

        test    ecx, 0x08000000  ; redirect the output to the user area
        jz      @f

        call    draw_text_to_user_area
        jmp     .pixloop1cont
;--------------------------------------
align 4
@@:
        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 save to mouseunder area
;        call    [putpixel]
        call    __sys_putpixel
        jmp     .pixloop1cont
;--------------------------------------
align 4
.nopix:
        test    ecx, 0x40000000
        jz      .pixloop1cont

        push    ecx
        mov     ecx, [esp+4+20h+20h]

        test    ecx, 0x08000000  ; redirect the output to the user area
        jz      @f

        call    draw_text_to_user_area
        pop     ecx
        jmp     .pixloop1cont
;--------------------------------------
align 4
@@:
        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 save to mouseunder area
;        call    [putpixel]
        call    __sys_putpixel
        pop     ecx
;--------------------------------------
align 4
.pixloop1cont:
        inc     eax
        jmp     .pixloop1
;--------------------------------------
align 4
.pixloop1end:
        sub     eax, 6
        inc     ebx
        inc     ebp
        dec     esi
        jnz     .symloop1

        popad
        add     eax, 6
        jmp     .loop
;--------------------------------------
align 4
.font2:
        add     edx, edx
        lea     ebp, [FONT_II+4*edx+edx+1]
        push    9
        movzx   esi, byte [ebp-1]
;--------------------------------------
align 4
.symloop2:
        mov     dl, byte [ebp]
        push    esi
;--------------------------------------
align 4
.pixloop2:
        shr     dl, 1
        jnc     .nopix2

        test    ecx, 0x08000000  ; redirect the output to the user area
        jz      @f

        call    draw_text_to_user_area
        jmp     .pixloop2cont
;--------------------------------------
align 4
@@:
        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 save to mouseunder area
;        call    [putpixel]
        call    __sys_putpixel
        jmp     .pixloop2cont
;--------------------------------------
align 4
.nopix2:
        test    ecx, 0x40000000
        jz      .pixloop2cont

        push    ecx
        mov     ecx, [esp+12+20h+20h]

        test    ecx, 0x08000000  ; redirect the output to the user area
        jz      @f

        call    draw_text_to_user_area
        pop     ecx
        jmp     .pixloop2cont
;--------------------------------------
align 4
@@:
        and     ecx, 0xFBFFFFFF  ;negate 0x04000000 save to mouseunder area
;        call    [putpixel]
        call    __sys_putpixel
        pop     ecx
;--------------------------------------
align 4
.pixloop2cont:
        inc     eax
        dec     esi
        jnz     .pixloop2

        pop     esi
        sub     eax, esi
        inc     ebx
        inc     ebp
        dec     dword [esp]
        jnz     .symloop2

        pop     eax
        add     dword [esp+28], esi
        popad
        jmp     .loop
;--------------------------------------
align 4
.end:
        popad
        pop     eax
        ret
;------------------------------------------------------------------------------
; eax = x coordinate
; ebx = y coordinate
; ecx = ?? RR GG BB
; edi = user area
align 4
draw_text_to_user_area:
        pushad
        imul    ebx, [edi+0]
        add     eax, ebx
        shl     eax, 2
        add     eax, edi
        add     eax, 8
        and     ecx, 0xffffff
        or      ecx, 0xff000000  ; not transparent
        mov     [eax], ecx       ; store  pixel
        popad
        ret
;------------------------------------------------------------------------------
align 4
FONT_I:
  if lang eq sp
  file 'char_sp.mt'
  else if lang eq et
  file 'char_et.mt'
  else
  file 'char.mt'
  end if
;------------------------------------------------------------------------------
align 4
FONT_II:
  if lang eq sp
  file 'char2_sp.mt'
  else if lang eq et
  file 'char2_et.mt'
  else
  file 'char2.mt'
  end if
;------------------------------------------------------------------------------
