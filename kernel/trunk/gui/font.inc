;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


dtext:
; edx -> string
; esi = number of characters
; ebx = output coordinates XXXXYYYY h
; ecx = char color and flags flRRGGBB h
; fl = ZBFFRSSS b
; Z=1: edx -> zero terminated string, esi = ?
; B=1: fill background with color eax
; R=1: edi -> user area for redirect
; FF=3: UTF-8 8x16, FF=2: UTF-16LE 8x16
; FF=1: cp866 8x16, FF=0: cp866 6x9
; SSS = (font multiplier)-1
; edi=1: force output
; flag CF=1 means that we deal with asciiz but need to draw no more than esi symbols
        clc
._:
        push    ecx edx
        setc    dl
        and     eax, 0xFFFFFF
        and     ecx, 1 shl 31
        shl     edx, 29
        or      eax, ecx
        or      eax, edx
        pop     edx ecx
        bt      ecx, 30
        jc      @f
        or      eax, 1 shl 30
@@:
        pushd   0 0 0 eax
        movsx   eax, bx
        sar     ebx, 16
        push    eax ebx edi
        bt      ecx, 27
        jc      .redirect
        mov     ebp, [_display.width]
        xor     edi, edi
        jmp     @f

.ret:
        add     esp, 28
        ret

.redirect:
        mov     ebp, [edi]
        add     edi, 8
@@:
        shl     ebp, 2
        imul    eax, ebp
        shl     ebx, 2
        add     eax, ebx
        js      .ret
        add     edi, eax
        mov     eax, ecx
        mov     ebx, ecx
        test    ecx, ecx
        jns     @f
        mov     esi, 256
@@:
        shr     ecx, 24
        and     cl, 7
        inc     ecx
        push    ebp ecx esi
        mov     esi, edx
        or      eax, 0xFF000000
        bt      ebx, 27
        jc      .bufferReady
        mov     eax, 9
        test    ebx, 0x30000000
        jz      @f
        add     eax, 7
@@:
        imul    eax, ecx
        mov     [esp+32], eax
        imul    ebp, eax
        stdcall kernel_alloc, ebp
        mov     ecx, ebp
        shr     ecx, 2
        mov     [esp+36], eax
        sub     edi, eax
        mov     edx, eax
        mov     eax, [esp+24]
        test    eax, 1 shl 30
        jz      @f
        or      eax, -1
        mov     [esp+28], edi
        jmp     .1
@@:
        and     eax, 0x00ffffff
.1:
        mov     edi, edx
        rep stosd
        mov     edi, edx
        mov     eax, ebx
        and     eax, 0xFFFFFF
.bufferReady:
        mov     ebp, eax
        mov     eax, [esp+8]
        shr     eax, 2
        sub     eax, [esp+16]
        shr     eax, 3
        xor     edx, edx
        div     dword[esp+4]
        mov     ecx, [esp]
        cmp     eax, ecx
        jnc     @f
        mov     [esp], eax
@@:
        xor     edx, edx
        bt      ebx, 29
        jc      @f
        bt      ebx, 28
        jc      .draw866toUni
        shl     eax, 3
        mov     bx, 6
        div     bx
        xor     edx, edx
        inc     eax
        mov     [esp], ecx
        cmp     eax, ecx
        jnc     .draw866
        mov     [esp], eax
        jmp     .draw866

@@:
        bt      ebx, 28
        jc      .drawUTF8

; ebp = font color
; esi -> string
; edi -> buffer

; Stack map:
; char counter +0
fontMultiplier = 4
widthX = 8
; edi +12
; X +16
; Y +20
; internal flags & background +24
deltaToScreen = 28
; temp buffer height +32
; temp buffer pointer +36

.drawUTF16:
        dec     dword [esp]
        js      .done
        movzx   ebx, word [esi]
        test    dword [esp + 24], 1 shl 31
        jnz     @f
        test    dword [esp + 24], 1 shl 29
        jz      .u16Still
@@:
        test    ebx, ebx
        jz      .done
.u16Still:
        inc     esi
        inc     esi
        cmp     bx, 1419
        jc      @f
        xor     ebx, ebx
@@:
        pushd   esi edi 16
        shl     ebx, 4
        add     ebx, fontUni
        mov     esi, [esp+12+fontMultiplier]
        call    drawChar
        imul    esi, 8*4
        pop     edi
        pop     edi
        add     edi, esi
        pop     esi
        jmp     .drawUTF16

.drawUTF8:
        dec     dword [esp]
        js      .done
        xor     eax, eax
        call    utf8to16
        test    dword [esp + 24], 1 shl 31
        jnz     @f
        test    dword [esp + 24], 1 shl 29
        jz      .u8Still
@@:
        test    eax, eax
        jz      .done
.u8Still:
        cmp     eax, 1419
        jc      @f
        xor     eax, eax
@@:
        shl     eax, 4
        lea     ebx, [eax+fontUni]
        pushd   esi edi 16
        mov     esi, [esp+12+fontMultiplier]
        call    drawChar
        imul    esi, 8*4
        pop     edi
        pop     edi
        add     edi, esi
        pop     esi
        jmp     .drawUTF8

.draw866:
        dec     dword [esp]
        js      .done
        movzx   ebx, byte [esi]
        test    dword [esp + 24], 1 shl 31
        jnz     @f
        test    dword [esp + 24], 1 shl 29
        jz      .866Still
@@:
        test    ebx, ebx
        jz      .done
.866Still:
        inc     esi
        pushd   esi edi 9
        lea     ebx, [ebx*8+ebx+font1]
        mov     esi, [esp+12+fontMultiplier]
        call    drawChar
        imul    esi, 6*4
        pop     edi
        pop     edi
        add     edi, esi
        pop     esi
        jmp     .draw866

.draw866toUni:
        dec     dword [esp]
        js      .done
        movzx   eax, byte [esi]
        test    dword [esp + 24], 1 shl 31
        jnz     @f
        test    dword [esp + 24], 1 shl 29
        jz      .8662uStill
@@:
        test    eax, eax
        jz      .done
.8662uStill:
        call    ansi2uni_char
        shl     eax, 4
        lea     ebx, [eax+fontUni]
        inc     esi
        pushd   esi edi 16
        mov     esi, [esp+12+fontMultiplier]
        call    drawChar
        imul    esi, 8*4
        pop     edi
        pop     edi
        add     edi, esi
        pop     esi
        jmp     .draw866toUni

.done:
        mov     ecx, edi
        pop     eax eax eax esi edx ebx ebp ebp ebp
        mov     edi, [esp]
        test    edi, edi
        jnz     @f
        pop     eax
        ret

@@: ; redraw from buffer to screen
        push    eax
        sub     ecx, edi
        shr     ecx, 2
        add     edx, ecx
        inc     ecx
        push    ecx
        push    edi
.drawPicture:
        mov     eax, -1
        repz scasd
        jecxz   @f
        mov     eax, edx
        sub     eax, ecx
        push    ecx
        mov     ecx, [edi-4]
        xchg    esi, edi
        call    __sys_putpixel
        xchg    esi, edi
        pop     ecx
        jmp     .drawPicture

@@:
        pop     edi
        mov     ecx, [esp]
        add     edi, [esp+4]
        inc     ebx
        push    edi
        dec     ebp
        jnz     .drawPicture
        add     esp, 12
        call    kernel_free
        ret

; scaling/smoothing algorithm
drawChar:
; ebp = font color
; esi = font multiplier
; edi -> buffer
; ebx -> char data
        mov     dl, [ebx]
.raw:
        bsf     eax, edx
        jz      .nextRaw
        imul    eax, esi
        shl     eax, 2
        push    edi
        add     edi, eax
        mov     ecx, esi
        dec     esi
        jnz     .square
        mov     [edi], ebp
        inc     esi
        cmp     [fontSmoothing], 0
        jz      .nextPixel
.checkLeftSM:     ; smoothing
        bsf     eax, edx
        dec     eax
        js      .checkRightSM
        bt      [ebx], eax
        jc      .checkRightSM
        dec     eax
        js      .checkLeftDownSM
        bt      [ebx], eax
        jc      .checkRightSM
.checkLeftDownSM:
        inc     eax
        bt      [ebx+1], eax
        jnc     .checkLeftUpSM
        inc     eax
        bt      [ebx+1], eax
        jnc     @f
        bt      [ebx-1], eax
        jc      .checkRightSM
        dec     eax
        dec     eax
        js      @f
        bt      [ebx+1], eax
        jnc     @f
        inc     eax
.checkLeftUpSM:
        bt      [ebx-1], eax
        jnc     .checkRightSM
        inc     eax
        bt      [ebx-1], eax
        jnc     @f
        bt      [ebx+1], eax
        jc      .checkRightSM
        dec     eax
        dec     eax
        js      @f
        bt      [ebx-1], eax
        jc      .checkRightSM
@@:
        mov     ecx, [esp+20+deltaToScreen]
        mov     eax, [edi-4]
        test    ecx, ecx
        jz      @f
        pusha
        lea     ebx, [edi+ecx-4]
        shr     ebx, 2
        call    syscall_getpixel
        popa
@@:
        push    ebx edx
        mov     ebx, ebp
        xor     ecx, ecx
        cmp     [fontSmoothing], 1
        jnz     .subpixelLeft
        call    antiAliasing
        jmp     @f

.subpixelLeft:
        mov     cl, bl
        lea     edx, [ecx*8+ecx]
        lea     edx, [ecx*2+edx]
        mov     cl, al
        lea     ecx, [ecx*4+ecx]
        add     edx, ecx
        shr     edx, 4
        mov     al, dl

        xor     ecx, ecx
        mov     cl, ah
        lea     edx, [ecx*8+ecx]
        lea     edx, [ecx*2+edx]
        mov     cl, bh
        lea     ecx, [ecx*4+ecx]
        add     edx, ecx
        shr     edx, 4
        mov     ah, dl

        rol     eax, 16
        rol     ebx, 16
        xor     ecx, ecx
        mov     cl, al
        mov     edx, ecx
        shl     ecx, 3
        sub     ecx, edx
        mov     dl, bl
        add     ecx, edx
        shr     ecx, 3
        mov     al, cl
        rol     eax, 16
@@:
        mov     [edi-4], eax
        pop     edx ebx
.checkRightSM:
        bsf     eax, edx
        inc     eax
        bt      [ebx], eax
        jc      .nextPixel
        inc     eax
        bt      [ebx], eax
        jc      .nextPixel
        dec     eax
.checkRightDownSM:
        bt      [ebx+1], eax
        jnc     .checkRightUpSM
        dec     eax
        bt      [ebx+1], eax
        jnc     @f
        bt      [ebx-1], eax
        jc      .nextPixel
        inc     eax
        inc     eax
        bt      [ebx+1], eax
        jnc     @f
        dec     eax
.checkRightUpSM:
        bt      [ebx-1], eax
        jnc     .nextPixel
        dec     eax
        bt      [ebx-1], eax
        jnc     @f
        bt      [ebx+1], eax
        jc      .nextPixel
        inc     eax
        inc     eax
        bt      [ebx-1], eax
        jc      .nextPixel
@@:
        mov     ecx, [esp+20+deltaToScreen]
        mov     eax, [edi+4]
        test    ecx, ecx
        jz      @f
        pusha
        lea     ebx, [edi+ecx+4]
        shr     ebx, 2
        call    syscall_getpixel
        popa
@@:
        push    ebx edx
        mov     ebx, ebp
        xor     ecx, ecx
        cmp     [fontSmoothing], 1
        jnz     .subpixelRight
        call    antiAliasing
        jmp     @f

.subpixelRight:
        mov     cl, al
        mov     edx, ecx
        shl     ecx, 3
        sub     ecx, edx
        mov     dl, bl
        add     ecx, edx
        shr     ecx, 3
        mov     al, cl

        xor     ecx, ecx
        mov     cl, ah
        lea     edx, [ecx*8+ecx]
        lea     edx, [ecx*2+edx]
        mov     cl, bh
        lea     ecx, [ecx*4+ecx]
        add     edx, ecx
        shr     edx, 4
        mov     ah, dl

        rol     ebx, 16
        rol     eax, 16
        xor     ecx, ecx
        mov     cl, bl
        lea     edx, [ecx*8+ecx]
        lea     edx, [ecx*2+edx]
        mov     cl, al
        lea     ecx, [ecx*4+ecx]
        add     edx, ecx
        shr     edx, 4
        mov     al, dl
        rol     eax, 16
@@:
        mov     [edi+4], eax
        pop     edx ebx
        jmp     .nextPixel

.square:    ; scaling
        mov     eax, esi
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        add     edi, [esp+20+widthX]
        dec     ecx
        jnz     .square
        inc     esi
        mov     edi, [esp]
.checkLeft:
        bsf     eax, edx
        dec     eax
        js      .checkRight
        bt      [ebx], eax
        jc      .checkRight
.checkLeftDown:
        bt      [ebx+1], eax
        jnc     .checkLeftUp
        mov     ecx, eax
        inc     eax
        bt      [ebx+1], eax
        jc      @f
        bt      [ebx-1], eax
        jnc     .downRightLow
        bt      [ebx-2], eax
        jc      .downRightLow
        dec     eax
        bt      [ebx-1], eax
        jc      .downRightLow
        dec     eax
        js      .downRightHigh
        bt      [ebx-2], eax
        jc      .downRightLow
        jmp     .downRightHigh

@@:
        bt      [ebx-1], eax
        jc      .checkLeftUp
        dec     eax
        dec     eax
        js      .downRightLow
        bt      [ebx+1], eax
        jc      .checkLeftUp
.downRightLow:
        imul    ecx, esi
        shl     ecx, 2
        add     edi, ecx
        dec     esi
        mov     eax, [esp+20+widthX]
        imul    eax, esi
        add     edi, eax
        add     edi, 4
        mov     ecx, esi
        dec     ecx
.drawDownRight:
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        sub     edi, [esp+20+widthX]
        add     edi, 4
        dec     ecx
        jns     .drawDownRight
        inc     esi
        mov     edi, [esp]
        jmp     .checkLeftUp

.downRightHigh:
        imul    ecx, esi
        shl     ecx, 2
        add     edi, ecx
        dec     esi
        mov     eax, [esp+20+widthX]
        imul    eax, esi
        add     edi, eax
        add     edi, 4
        mov     ecx, esi
        dec     ecx
.drawDownRightHigh:
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        sub     edi, [esp+20+widthX]
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        sub     edi, [esp+20+widthX]
        add     edi, 4
        dec     ecx
        jns     .drawDownRightHigh
        inc     esi
        mov     edi, [esp]
.checkLeftUp:
        bsf     eax, edx
        dec     eax
        bt      [ebx-1], eax
        jnc     .checkRight
        mov     ecx, eax
        inc     eax
        bt      [ebx-1], eax
        jc      @f
        bt      [ebx+1], eax
        jnc     .upRightLow
        bt      [ebx+2], eax
        jc      .upRightLow
        dec     eax
        bt      [ebx+1], eax
        jc      .upRightLow
        dec     eax
        js      .upRightHigh
        bt      [ebx+2], eax
        jc      .upRightLow
        jmp     .upRightHigh

@@:
        bt      [ebx+1], eax
        jc      .checkRight
        dec     eax
        dec     eax
        js      .upRightLow
        bt      [ebx-1], eax
        jc      .checkRight
.upRightLow:
        imul    ecx, esi
        shl     ecx, 2
        add     edi, ecx
        add     edi, 4
        mov     ecx, esi
        dec     ecx
        dec     ecx
.drawUpRight:
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        add     edi, [esp+20+widthX]
        add     edi, 4
        dec     ecx
        jns     .drawUpRight
        mov     edi, [esp]
        jmp     .checkRight

.upRightHigh:
        imul    ecx, esi
        shl     ecx, 2
        add     edi, ecx
        add     edi, 4
        mov     ecx, esi
        dec     ecx
        dec     ecx
.drawUpRightHigh:
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        add     edi, [esp+20+widthX]
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        add     edi, [esp+20+widthX]
        add     edi, 4
        dec     ecx
        jns     .drawUpRightHigh
        mov     edi, [esp]
.checkRight:
        bsf     eax, edx
        inc     eax
        bt      [ebx], eax
        jc      .nextPixel
.checkRightDown:
        bt      [ebx+1], eax
        jnc     .checkRightUp
        mov     ecx, eax
        dec     eax
        bt      [ebx+1], eax
        jc      @f
        bt      [ebx-1], eax
        jnc     .downLeftLow
        bt      [ebx-2], eax
        jc      .downLeftLow
        inc     eax
        bt      [ebx-1], eax
        jc      .downLeftLow
        inc     eax
        bt      [ebx-2], eax
        jc      .downLeftLow
        jmp     .downLeftHigh

@@:
        bt      [ebx-1], eax
        jc      .checkRightUp
        inc     eax
        inc     eax
        bt      [ebx+1], eax
        jc      .checkRightUp
.downLeftLow:
        imul    ecx, esi
        shl     ecx, 2
        add     edi, ecx
        dec     esi
        mov     eax, [esp+20+widthX]
        imul    eax, esi
        add     edi, eax
        mov     ecx, esi
        dec     ecx
.drawDownLeft:
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        sub     edi, [esp+20+widthX]
        dec     ecx
        jns     .drawDownLeft
        inc     esi
        mov     edi, [esp]
        jmp     .checkRightUp

.downLeftHigh:
        imul    ecx, esi
        shl     ecx, 2
        add     edi, ecx
        dec     esi
        mov     eax, [esp+20+widthX]
        imul    eax, esi
        add     edi, eax
        mov     ecx, esi
        dec     ecx
.drawDownLeftHigh:
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        sub     edi, [esp+20+widthX]
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        sub     edi, [esp+20+widthX]
        dec     ecx
        jns     .drawDownLeftHigh
        inc     esi
        mov     edi, [esp]
.checkRightUp:
        bsf     eax, edx
        inc     eax
        bt      [ebx-1], eax
        jnc     .nextPixel
        mov     ecx, eax
        dec     eax
        bt      [ebx-1], eax
        jc      @f
        bt      [ebx+1], eax
        jnc     .upLeftLow
        bt      [ebx+2], eax
        jc      .upLeftLow
        inc     eax
        bt      [ebx+1], eax
        jc      .upLeftLow
        inc     eax
        bt      [ebx+2], eax
        jc      .upLeftLow
        jmp     .upLeftHigh

@@:
        bt      [ebx+1], eax
        jc      .nextPixel
        inc     eax
        inc     eax
        bt      [ebx-1], eax
        jc      .nextPixel
.upLeftLow:
        imul    ecx, esi
        shl     ecx, 2
        add     edi, ecx
        mov     ecx, esi
        dec     ecx
        dec     ecx
.drawUpLeft:
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        add     edi, [esp+20+widthX]
        dec     ecx
        jns     .drawUpLeft
        jmp     .nextPixel

.upLeftHigh:
        imul    ecx, esi
        shl     ecx, 2
        add     edi, ecx
        mov     ecx, esi
        dec     ecx
        dec     ecx
.drawUpLeftHigh:
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        add     edi, [esp+20+widthX]
        mov     eax, ecx
@@:
        mov     [edi+eax*4], ebp
        dec     eax
        jns     @b
        add     edi, [esp+20+widthX]
        dec     ecx
        jns     .drawUpLeftHigh
.nextPixel:
        bsf     eax, edx
        btr     edx, eax
        pop     edi
        jmp     .raw

.nextRaw:
        inc     ebx
        mov     eax, [esp+16+widthX]
        imul    eax, esi
        add     edi, eax
        dec     dword [esp+4]
        jnz     drawChar
        ret

antiAliasing:
        mov     bp, 3
@@:
        mov     cl, al
        mov     dl, bl
        lea     ecx, [ecx*2+ecx]
        add     ecx, edx
        shr     ecx, 2
        mov     al, cl
        ror     eax, 8
        ror     ebx, 8
        dec     bp
        jnz     @b
        ror     eax, 8
        ror     ebx, 8
        mov     ebp, ebx
        ret

iglobal
fontSmoothing   db  2   ; = 0, 1 or 2
fontSize        db  0   ; user mode setting
font1:
  if lang eq sp
  file 'gui/char_sp.mt'
  else if lang eq et
  file 'gui/char_et.mt'
  else
  file 'gui/char.mt'
  end if
fontUni:
file 'gui/charUni.mt'
endg
