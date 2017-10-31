;;================================================================================================;;
;;//// convert.asm //// (c) dunkaist, 2012 ///////////////////////////////////////////////////////;;
;;================================================================================================;;
;;                                                                                                ;;
;; This file is part of Common development libraries (Libs-Dev).                                  ;;
;;                                                                                                ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under the terms of the GNU ;;
;; Lesser General Public License as published by the Free Software Foundation, either version 2.1 ;;
;; of the License, or (at your option) any later version.                                         ;;
;;                                                                                                ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  ;;
;; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  ;;
;; Lesser General Public License for more details.                                                ;;
;;                                                                                                ;;
;; You should have received a copy of the GNU Lesser General Public License along with Libs-Dev.  ;;
;; If not, see <http://www.gnu.org/licenses/>.                                                    ;;
;;                                                                                                ;;
;;================================================================================================;;



;;================================================================================================;;
proc img.convert _src, _dst, _dst_type, _flags, _param                                            ;;
;;------------------------------------------------------------------------------------------------;;
;? convert _image                                                                                 ;;
;;------------------------------------------------------------------------------------------------;;
;> [_src]      = pointer to source image                                                          ;;
;> [_dst]      = pointer to destination image, or 0 to create a new one                           ;;
;> [_dst_type] = Image.Type of resulting image                                                    ;;
;> [_flags]    = see libimg.inc                                                                   ;;
;> [_param]    = depends on _flags fields, see libimg.inc                                         ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to converted image                                                           ;;
;< ecx = error code / undefined                                                                   ;;
;;================================================================================================;;
locals
        img     dd ?
        prev    dd ?
endl
        push    ebx esi edi
        mov     [img], 0
        mov     [prev], 0
        mov     ebx, [_src]
    @@:
        mov     eax, [ebx + Image.Previous]
        test    eax, eax
        jz      .loop
        mov     ebx, eax
        jmp     @b
  .loop:
        stdcall img.convert.layer, ebx, [_dst], [_dst_type], [_flags], [_param]
        test    eax, eax
        jz      .error
        cmp     [img], 0
        jnz     @f
        mov     [img], eax
    @@:
        mov     ecx, [prev]
        jecxz   @f
        mov     [ecx + Image.Next], eax
        mov     [eax + Image.Previous], ecx
    @@:
        mov     [prev], eax
        push    [ebx + Image.Flags]
        pop     [eax + Image.Flags]
        push    [ebx + Image.Delay]
        pop     [eax + Image.Delay]
        mov     ebx, [ebx + Image.Next]
        test    ebx, ebx
        jnz     .loop
        mov     eax, [img]
  .error:
        pop     edi esi ebx
        ret
endp


;;================================================================================================;;
proc img.convert.layer _src, _dst, _dst_type, _flags, _param                                      ;;
;;------------------------------------------------------------------------------------------------;;
;? convert _image layer                                                                           ;;
;;------------------------------------------------------------------------------------------------;;
;> [_src]      = pointer to source image                                                          ;;
;> [_dst]      = pointer to destination image, or 0 to create a new one                           ;;
;> [_dst_type] = Image.Type of resulting image                                                    ;;
;> [_flags]    = see libimg.inc                                                                   ;;
;> [_param]    = depends on _flags fields, see libimg.inc                                         ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to converted image                                                           ;;
;< ecx = error code / undefined                                                                   ;;
;;================================================================================================;;
locals
        fun     rd 1
endl
        push    ebx esi edi

        mov     ebx, [_src]
        mov     eax, [ebx + Image.Type]
        mov     esi, [img.convert.table + 4*eax]
  .next:
        lodsd
        test    eax, eax
        jnz     @f
        mov     ecx, LIBIMG_ERROR_BIT_DEPTH
        jmp     .exit
    @@:
        cmp     eax, [_dst_type]
        lodsd
        jnz     .next
        mov     [fun], eax

        mov     eax, [_dst]
        test    eax, eax
        jnz     @f
        stdcall img.create, [ebx + Image.Width], [ebx + Image.Height], [_dst_type]
        test    eax, eax
        jz      .exit
        mov     [_dst], eax
    @@:
        mov     edi, [eax + Image.Data]
        mov     esi, [ebx + Image.Data]
        mov     eax, [ebx + Image.Type]
        stdcall [fun], [_src], [_dst]
        mov     eax, [_dst]
  .exit:
        pop     edi esi ebx
        ret
endp

proc img._.convert.bpp8i_to_bpp24 _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]

        mov     ebx, [ebx + Image.Palette]
        sub     ecx, 1
        jz      .bpp8i.last
    @@:
        movzx   eax, byte[esi]
        add     esi, 1
        mov     eax, [ebx + eax*4]
        mov     [edi], eax
        add     edi, 3
        sub     ecx, 1
        jnz     @b
  .bpp8i.last:
        movzx   eax, byte[esi]
        mov     eax, [ebx + eax*4]
        mov     [edi], ax
        shr     eax, 16
        mov     [edi + 2], al
        ret
endp


proc img._.convert.bpp8i_to_bpp32 _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]

        mov     ebx, [ebx + Image.Palette]
    @@:
        movzx   eax, byte[esi]
        add     esi, 1
        mov     eax, [ebx + eax*4]
        mov     [edi], eax
        add     edi, 4
        dec     ecx
        jnz     @b
        ret
endp


proc img._.convert.bpp8g_to_bpp1 _src, _dst
        mov     eax, [_dst]
        mov     eax, [eax + Image.Palette]
        mov     dword[eax], 0x00000000
        mov     dword[eax + 4], 0x00ffffff
        mov     edx, [ebx + Image.Height]
  .bpp8g_to_bpp1.line:
        mov     ax, 0x0800
        mov     ecx, [ebx + Image.Width]
  .bpp8g_to_bpp1.pixel:
        shl     al, 1
        cmp     byte[esi], 0x7f
        cmc
        adc     eax, 0
        add     esi, 1
        dec     ah
        jnz     @f
        mov     byte[edi], al
        add     edi, 1
        mov     ax, 0x0800
    @@:
        dec     ecx
        jnz     .bpp8g_to_bpp1.pixel
        cmp     ah, 8
        je      @f
        mov     cl, ah
        shl     al, cl
        mov     byte[edi], al
        add     edi, 1
    @@:
        dec     edx
        jnz     .bpp8g_to_bpp1.line
        ret
endp

proc img._.convert.bpp8g_to_bpp24 _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]
    @@:
        mov     al, byte[esi]
        mov     byte[edi + 0], al
        mov     byte[edi + 1], al
        mov     byte[edi + 2], al
        add     esi, 1
        add     edi, 3
        sub     ecx, 1
        jnz     @b
        ret
endp


proc img._.convert.bpp24_to_bpp24 _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]
        lea     ecx, [ecx*3]
        mov     edx, ecx
        shr     ecx, 2
        rep     movsd
        mov     ecx, edx
        and     ecx, 3
        rep     movsb
        ret
endp


proc img._.convert.bpp24_to_bpp8g _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]
    @@:
        movzx   ebx, byte[esi + 0]
        movzx   eax, byte[esi + 1]
        add     ebx, eax
        movzx   eax, byte[esi + 2]
        add     eax, ebx
        mov     ebx, 3
        add     esi, 3
        div     bl
        mov     byte[edi], al
        add     edi, 1
        sub     ecx, 1
        jnz     @b
        ret
endp


proc img._.convert.bpp24_to_bpp32 _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]
    @@:
        lodsw
        ror     eax, 16
        lodsb
        rol     eax, 16
        stosd
        dec     ecx
        jnz     @b
        ret
endp


proc img._.convert.bpp32_to_bpp24 _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]
    @@:
        mov     eax, [esi]
        mov     [edi], ax
        shr     eax, 16
        mov     [edi + 2], al
        add     esi, 4
        add     edi, 3
        sub     ecx, 1
        jnz     @b
        ret
endp


proc img._.convert.bpp32_to_bpp32 _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]
        rep     movsd
        ret
endp


proc img._.convert.bpp15_to_bpp24 _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]

  .bpp15.intel: ; copypasted from do_rgb
        push    ebx ebp
        sub     ecx, 4
        jb      .bpp15.tail
align 16
  .bpp15.intel.loop:
repeat 2
        mov     ebx, [esi]
        mov     al, [esi]
        mov     ah, [esi + 1]
        add     esi, 4
        and     al, 0x1F
        and     ah, 0x1F shl 2
        mov     ebp, ebx
        mov     dl, al
        mov     dh, ah
        shr     al, 2
        shr     ah, 4
        shl     dl, 3
        shl     dh, 1
        and     ebp, 0x1F shl 5
        add     al, dl
        add     ah, dh
        shr     ebp, 2
        mov     [edi], al
        mov     [edi + 2], ah
        mov     eax, ebx
        mov     ebx, ebp
        shr     eax, 16
        shr     ebx, 5
        add     ebx, ebp
        mov     ebp, eax
        mov     [edi + 1], bl
        and     eax, (0x1F) or (0x1F shl 10)
        and     ebp, 0x1F shl 5
        lea     edx, [eax + eax]
        shr     al, 2
        mov     ebx, ebp
        shr     ah, 4
        shl     dl, 2
        shr     ebx, 2
        shr     ebp, 7
        add     al, dl
        add     ah, dh
        mov     [edi + 3], al
        add     ebx, ebp
        mov     [edi + 5], ah
        mov     [edi + 4], bl
        add     edi, 6
end repeat
        sub     ecx, 4
        jnb     .bpp15.intel.loop
  .bpp15.tail:
        add     ecx, 4
        jz      .bpp15.done
    @@:
        movzx   eax, word [esi]
        mov     ebx, eax
        add     esi, 2
        and     eax, (0x1F) or (0x1F shl 10)
        and     ebx, 0x1F shl 5
        lea     edx, [eax + eax]
        shr     al, 2
        mov     ebp, ebx
        shr     ebx, 2
        shr     ah, 4
        shl     dl, 2
        shr     ebp, 7
        add     eax, edx
        add     ebx, ebp
        mov     [edi], al
        mov     [edi + 1], bl
        mov     [edi + 2], ah
        add     edi, 3
        sub     ecx, 1
        jnz     @b
  .bpp15.done:
        pop     ebp ebx
        mov     eax, [_dst]
        jmp     .quit

  .bpp15.amd:
        push    ebx ebp
        sub     ecx, 4
        jb      .bpp15.tail
align 16
  .bpp15.amd.loop:
repeat 4
if (% mod 2) = 1
        mov     eax, dword[esi]
        mov     ebx, dword[esi]
else
        movzx   eax, word[esi]
        mov     ebx, eax
end if
        add     esi, 2
        and     eax, (0x1F) or (0x1F shl 10)
        and     ebx, 0x1F shl 5
        lea     edx, [eax + eax]
        shr     al, 2
        mov     ebp, ebx
        shr     ebx, 2
        shr     ah, 4
        shl     dl, 2
        shr     ebp, 7
        add     eax, edx
        add     ebx, ebp
        mov     [edi], al
        mov     [edi + 1], bl
        mov     [edi + 2], ah
        add     edi, 3
end repeat
        sub     ecx, 4
        jnb     .bpp15.amd.loop
        jmp     .bpp15.tail

  .quit:
        ret
endp


proc img._.convert.bpp16_to_bpp24 _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]
  .bpp16.intel:
        push    ebx ebp
        sub     ecx, 4
        jb      .bpp16.tail
align 16
  .bpp16.intel.loop:
repeat 2
        mov     ebx, [esi]
        mov     al, [esi]
        mov     ah, [esi + 1]
        add     esi, 4
        and     al, 0x1F
        and     ah, 0x1F shl 3
        mov     ebp, ebx
        mov     dl, al
        mov     dh, ah
        shr     al, 2
        shr     ah, 5
        shl     dl, 3
        and     ebp, 0x3F shl 5
        add     al, dl
        add     ah, dh
        shr     ebp, 3
        mov     [edi], al
        mov     [edi + 2], ah
        mov     eax, ebx
        mov     ebx, ebp
        shr     eax, 16
        shr     ebx, 6
        add     ebx, ebp
        mov     ebp, eax
        mov     [edi + 1], bl
        and     eax, (0x1F) or (0x1F shl 11)
        and     ebp, 0x3F shl 5
        mov     edx, eax
        shr     al, 2
        mov     ebx, ebp
        shr     ah, 5
        shl     dl, 3
        shr     ebx, 3
        shr     ebp, 9
        add     al, dl
        add     ah, dh
        mov     [edi + 3], al
        add     ebx, ebp
        mov     [edi + 5], ah
        mov     [edi + 4], bl
        add     edi, 6
end repeat
        sub     ecx, 4
        jnb     .bpp16.intel.loop
  .bpp16.tail:
        add     ecx, 4
        jz      .bpp16.done
    @@:
        movzx   eax, word[esi]
        mov     ebx, eax
        add     esi, 2
        and     eax, (0x1F) or (0x1F shl 11)
        and     ebx, 0x3F shl 5
        mov     edx, eax
        shr     al, 2
        mov     ebp, ebx
        shr     ebx, 3
        shr     ah, 5
        shl     dl, 3
        shr     ebp, 9
        add     eax, edx
        add     ebx, ebp
        mov     [edi], al
        mov     [edi + 1], bl
        mov     [edi + 2], ah
        add     edi, 3
        sub     ecx, 1
        jnz     @b
  .bpp16.done:
        pop     ebp ebx
        mov     eax, [_dst]
        jmp     .quit

  .bpp16.amd:
        push    ebx ebp
        sub     ecx, 4
        jb      .bpp16.tail
align 16
  .bpp16.amd.loop:
repeat 4
if (% mod 2) = 1
        mov     eax, dword[esi]
        mov     ebx, dword[esi]
else
        movzx   eax, word[esi]
        mov     ebx, eax
end if
        add     esi, 2
        and     eax, (0x1F) or (0x1F shl 11)
        and     ebx, 0x3F shl 5
        mov     edx, eax
        shr     al, 2
        mov     ebp, ebx
        shr     ebx, 3
        shr     ah, 5
        shl     dl, 3
        shr     ebp, 9
        add     eax, edx
        add     ebx, ebp
        mov     [edi], al
        mov     [edi + 1], bl
        mov     [edi + 2], ah
        add     edi, 3
end repeat
        sub     ecx, 4
        jnb     .bpp16.amd.loop
        jmp     .bpp16.tail

  .quit:
        ret
endp


proc img._.convert.bpp1_to_bpp24 _src, _dst
locals
        width   rd 1
        height  rd 1
endl
        push    [ebx + Image.Width]
        pop     [width]
        push    [ebx + Image.Height]
        pop     [height]
        mov     edx, [ebx + Image.Palette]
  .bpp1_to_bpp24.line:
        mov     ebx, [width]
  .bpp1_to_bpp24.byte:
        mov     ah, 8
        mov     al, byte[esi]
        add     esi, 1
  .bpp1_to_bpp24.bit:
        xor     ecx, ecx
        shl     al, 1
        adc     ecx, 0
        mov     ecx, [edx + 4*ecx]
        mov     word[edi], cx
        shr     ecx, 8
        mov     byte[edi + 2], ch
        add     edi, 3
        sub     ebx, 1
        jnz     @f
        sub     [height], 1
        jnz     .bpp1_to_bpp24.line
        jmp     .bpp1.done
    @@:
        sub     ah, 1
        jnz     .bpp1_to_bpp24.bit
        jmp     .bpp1_to_bpp24.byte
  .bpp1.done:
        ret
endp


proc img._.convert.bpp8a_to_bpp1 _src, _dst
        mov     eax, [_dst]
        mov     eax, [eax + Image.Palette]
        mov     dword[eax], 0x00000000
        mov     dword[eax + 4], 0x00ffffff
        mov     edx, [ebx + Image.Height]
  .bpp8a_to_bpp1.line:
        mov     ax, 0x0800
        mov     ecx, [ebx + Image.Width]
  .bpp8a_to_bpp1.pixel:
        shl     al, 1
        cmp     byte[esi], 0x7f
        cmc
        adc     eax, 0
        add     esi, 2
        dec     ah
        jnz     @f
        mov     byte[edi], al
        add     edi, 1
        mov     ax, 0x0800
    @@:
        dec     ecx
        jnz     .bpp8a_to_bpp1.pixel
        cmp     ah, 8
        je      @f
        mov     cl, ah
        shl     al, cl
        mov     byte[edi], al
        add     edi, 1
    @@:
        dec     edx
        jnz     .bpp8a_to_bpp1.line
        ret
endp


proc img._.convert.bpp8a_to_bpp24 _src, _dst
        mov     ecx, [ebx + Image.Width]
        imul    ecx, [ebx + Image.Height]
    @@:
        mov     al, byte[esi]
        mov     byte[edi + 0], al
        mov     byte[edi + 1], al
        mov     byte[edi + 2], al
        add     esi, 2
        add     edi, 3
        sub     ecx, 1
        jnz     @b
        ret
endp


img.convert.bpp8i.table:
        dd Image.bpp24, img._.convert.bpp8i_to_bpp24
        dd Image.bpp32, img._.convert.bpp8i_to_bpp32
        dd 0
img.convert.bpp24.table:
        dd Image.bpp24, img._.convert.bpp24_to_bpp24
        dd Image.bpp8g, img._.convert.bpp24_to_bpp8g
        dd Image.bpp32, img._.convert.bpp24_to_bpp32
        dd 0
img.convert.bpp32.table:
        dd Image.bpp24, img._.convert.bpp32_to_bpp24
        dd Image.bpp32, img._.convert.bpp32_to_bpp32
        dd 0
img.convert.bpp15.table:
        dd Image.bpp24, img._.convert.bpp15_to_bpp24
        dd 0
img.convert.bpp16.table:
        dd Image.bpp24, img._.convert.bpp16_to_bpp24
        dd 0
img.convert.bpp1.table:
        dd Image.bpp24, img._.convert.bpp1_to_bpp24
        dd 0
img.convert.bpp8g.table:
        dd Image.bpp24, img._.convert.bpp8g_to_bpp24
        dd Image.bpp1,  img._.convert.bpp8g_to_bpp1
        dd 0
img.convert.bpp2i.table:
        dd 0
img.convert.bpp4i.table:
        dd 0
img.convert.bpp8a.table:
        dd Image.bpp24, img._.convert.bpp8a_to_bpp24
        dd 0

img.convert.table:
        dd 0    ; no image type zero
        dd img.convert.bpp8i.table
        dd img.convert.bpp24.table
        dd img.convert.bpp32.table
        dd img.convert.bpp15.table
        dd img.convert.bpp16.table
        dd img.convert.bpp1.table
        dd img.convert.bpp8g.table
        dd img.convert.bpp2i.table
        dd img.convert.bpp4i.table
        dd img.convert.bpp8a.table
