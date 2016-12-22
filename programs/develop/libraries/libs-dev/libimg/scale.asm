;;================================================================================================;;
;;//// scale.asm //// (c) dunkaist, 2012 /////////////////////////////////////////////////////////;;
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
proc img.get_scaled_size _width, _height, _scale_type, _param1, _param2                           ;;
;;------------------------------------------------------------------------------------------------;;
;? calculate resulting width and height if image of _width and _height is scaled via _scale_type  ;;
;;------------------------------------------------------------------------------------------------;;
;> [_width]      = width of input image                                                           ;;
;> [_height]     = height of input image                                                          ;;
;> [_scale_type] = see libimg.inc (LIBIMG_SCALE_*)                                                ;;
;> [_param1]     = depends on _scale_type, see libimg.inc                                         ;;
;> [_param2]     = depends on _scale_type, see libimg.inc                                         ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = width                                                                                    ;;
;< ecx = height                                                                                   ;;
;;================================================================================================;;
        push    ebx esi edi
        mov     eax, [_scale_type]
        cmp     eax, LIBIMG_SCALE_FIT_MIN
        jz      .fit_min

  .fit_min:
        xor     edx, edx
        mov     eax, [_width]
        shl     eax, 1
        mov     ecx, [_param1]
        add     eax, ecx
        shl     eax, 15
        div     ecx
        mov     ebx, eax

        xor     edx, edx
        mov     eax, [_height]
        shl     eax, 1
        mov     ecx, [_param2]
        add     eax, ecx
        shl     eax, 15
        div     ecx

        cmp     eax, ebx
        ja      .fit_height
        jmp     .fit_width

        jmp     .quit
  .fit_max:
        jmp     .quit
  .fit_width:
        xor     edx, edx
        mov     eax, [_width]
        shl     eax, 16
        div     [_param1]
        mov     ecx, eax
        xor     edx, edx
        mov     eax, [_height]
        shl     eax, 16
        mov     ebx, ecx
        shr     ebx, 1
        add     eax, ebx
        div     ecx
        mov     ecx, eax
        mov     eax, [_param1]
        jmp     .quit
  .fit_height:
        xor     edx, edx
        mov     eax, [_height]
        shl     eax, 16
        div     [_param2]
        mov     ecx, eax
        xor     edx, edx
        mov     eax, [_width]
        shl     eax, 16
        mov     ebx, ecx
        shr     ebx, 1
        add     eax, ebx
        div     ecx
        mov     ecx, [_param2]
        jmp     .quit

  .quit:
        pop     edi esi ebx
        ret
endp


;;================================================================================================;;
proc img.scale _src, _crop_x, _crop_y, _crop_width, _crop_height, _dst, _scale, _inter, _param1, _param2 ;;
;;------------------------------------------------------------------------------------------------;;
;? scale _image                                                                                   ;;
;;------------------------------------------------------------------------------------------------;;
;> [_src]         = pointer to source image                                                       ;;
;> [_crop_x]      = left coord of cropping rect                                                   ;;
;> [_crop_y]      = top coord of cropping rect                                                    ;;
;> [_crop_width]  = width of cropping rect                                                        ;;
;> [_crop_height] = height of cropping rect                                                       ;;
;> [_dst]         = pointer to resulting image, 0 to create new one                               ;;
;> [_scale]       = scaling method, see libimg.inc (LIBIMG_SCALE_*)                               ;;
;> [_inter]       = interpolation algorithm, see libimg.inc (LIBIMG_INTER_*)                      ;;
;> [_param1]      = depends on _scale, see libimg.inc                                             ;;
;> [_param2]      = depends on _scale, see libimg.inc                                             ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to scaled image                                                              ;;
;< ecx = error code / undefined                                                                   ;;
;;================================================================================================;;
        push    ebx esi edi 0 0
        mov     ebx, [_src]
    @@:
        mov     eax, [ebx + Image.Previous]
        test    eax, eax
        jz      .loop
        mov     ebx, eax
        jmp     @b
  .loop:
        stdcall img.scale.layer, ebx, [_crop_x], [_crop_y], [_crop_width], [_crop_height], [_dst], [_scale], [_inter], [_param1], [_param2]
        test    eax, eax
        jz      .error
        cmp     dword[esp + 4], 0
        jnz     @f
        mov     [esp + 4], eax
    @@:
        mov     ecx, [esp]
        jecxz   @f
        mov     [ecx + Image.Next], eax
    @@:
        push    [ebx + Image.Flags]
        pop     [eax + Image.Flags]
        push    [ebx + Image.Delay]
        pop     [eax + Image.Delay]
        mov     [eax + Image.Previous], ecx
        mov     [esp], eax
        mov     ebx, [ebx + Image.Next]
        test    ebx, ebx
        jnz     .loop
  .quit:
        pop     eax eax edi esi ebx
        ret
  .error:
        pop     eax eax edi esi ebx
        ret
endp


;;================================================================================================;;
proc img.scale.layer _src, _crop_x, _crop_y, _crop_width, _crop_height, _dst, _scale, _inter, _param1, _param2 ;;
;;------------------------------------------------------------------------------------------------;;
;? scale _image layer                                                                             ;;
;;------------------------------------------------------------------------------------------------;;
;> [_src]         = pointer to source image                                                       ;;
;> [_crop_x]      = left coord of cropping rect                                                   ;;
;> [_crop_y]      = top coord of cropping rect                                                    ;;
;> [_crop_width]  = width of cropping rect                                                        ;;
;> [_crop_height] = height of cropping rect                                                       ;;
;> [_dst]         = pointer to resulting image, 0 to create new one                               ;;
;> [_scale]       = scaling method, see libimg.inc (LIBIMG_SCALE_*)                               ;;
;> [_inter]       = interpolation algorithm, see libimg.inc (LIBIMG_INTER_*)                      ;;
;> [_param1]      = depends on _scale, see libimg.inc                                             ;;
;> [_param2]      = depends on _scale, see libimg.inc                                             ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to scaled image                                                              ;;
;< ecx = error code / undefined                                                                   ;;
;;================================================================================================;;
locals
        src_type                rd 1
        src_data                rd 1
        dst_data                rd 1

        src_width_pixels        rd 1
        src_width_bytes         rd 1
        src_height_pixels       rd 1

        dst_width_pixels        rd 1
        dst_width_pixels_inv    rd 1
        dst_height_pixels       rd 1
        dst_height_pixels_inv   rd 1
        dst_width_bytes         rd 1
        bytes_per_pixel         rd 1

        crop_width_pixels_m1    rd 1
        crop_height_pixels_m1   rd 1
; bilinear
        src_x           rd 1
        src_y           rd 1
        src_base        rd 1
        dst_x           rd 1
        dst_y           rd 1
        rem_x           rd 1
        rem_y           rd 1
endl
        push    ebx esi edi
        mov     ebx, [_src]
        push    [ebx + Image.Width]
        pop     [src_width_pixels]
        push    [ebx + Image.Height]
        pop     [src_height_pixels]
        push    [ebx + Image.Type]
        pop     [src_type]
        push    [ebx + Image.Data]
        pop     [src_data]

        mov     eax, [src_type]
        mov     ecx, [src_width_pixels]
        mov     edx, [src_width_pixels]
        imul    edx, [_crop_y]
        add     edx, [_crop_x]
        cmp     eax, Image.bpp32
        jne     @f
        mov     [bytes_per_pixel], 4
        shl     ecx, 2
        shl     edx, 2
        jmp     .lab1
    @@:
        cmp     eax, Image.bpp24
        jne     @f
        mov     [bytes_per_pixel], 3
        lea     ecx, [ecx*3]
        lea     edx, [edx*3]
        jmp     .lab1
    @@:
        cmp     eax, Image.bpp8g
        jne     @f
        mov     [bytes_per_pixel], 1
        jmp     .lab1
    @@:
        mov     ecx, LIBIMG_ERROR_BIT_DEPTH
        jmp     .error
  .lab1:
        mov     [src_width_bytes], ecx
        add     [src_data], edx


        mov     eax, [_scale]
        cmp     eax, LIBIMG_SCALE_INTEGER
        je      .scale_type.integer
        cmp     eax, LIBIMG_SCALE_TILE
        je      .scale_type.tile
        cmp     eax, LIBIMG_SCALE_FIT_RECT
        je      .scale_type.fit_rect
        cmp     eax, LIBIMG_SCALE_FIT_WIDTH
        je      .scale_type.fit_width
        cmp     eax, LIBIMG_SCALE_FIT_HEIGHT
        je      .scale_type.fit_height
        cmp     eax, LIBIMG_SCALE_FIT_MAX
        je      .scale_type.fit_max
        cmp     eax, LIBIMG_SCALE_STRETCH
        je      .scale_type.stretch
        mov     ecx, LIBIMG_ERROR_SCALE
        jmp     .error

  .scale_type.integer:
        jmp     .integer
  .scale_type.tile:
        jmp     .tile
  .scale_type.fit_rect:
  .scale_type.fit_max:
  .scale_type.fit_width:
  .scale_type.fit_height:
        mov     eax, [_src]
        stdcall img.get_scaled_size, [eax + Image.Width], [eax + Image.Height], [_scale], [_param1], [_param2]
        mov     [_param1], eax
        mov     [_param2], ecx
  .scale_type.stretch:
        mov     eax, [_param1]
        mov     [dst_width_pixels], eax
        imul    eax, [bytes_per_pixel]
        mov     [dst_width_bytes], eax
        mov     ecx, [_param2]
        mov     [dst_height_pixels], ecx
        jmp     .define_inter
  .define_inter:
        mov     eax, [_inter]
        cmp     eax, LIBIMG_INTER_BILINEAR
        je      .bilinear
        mov     ecx, LIBIMG_ERROR_INTER
        jmp     .error


  .integer:
        mov     eax, [_param1]
        mov     ecx, [_crop_width]
        imul    ecx, eax
        mov     [dst_width_pixels], ecx
        mov     edx, [_crop_height]
        imul    edx, eax
        mov     [dst_height_pixels], edx

        mov     eax, [_dst]
        test    eax, eax
        jnz     @f
        stdcall img.create, [dst_width_pixels], [dst_height_pixels], [src_type]
        test    eax, eax
        jz      .error
        mov     [_dst], eax
        mov     edi, [_src]
        push    [edi + Image.Flags]
        pop     [eax + Image.Flags]
        push    [edi + Image.Delay]
        pop     [eax + Image.Delay]
        push    [edi + Image.Previous]
        pop     [eax + Image.Previous]
        push    [edi + Image.Next]
        pop     [eax + Image.Next]
    @@:
        mov     edi, [eax + Image.Data]
        mov     [dst_data], edi

        mov     esi, [src_data]
        mov     eax, [src_type]
        cmp     eax, Image.bpp8g
        je      .integer.bpp8g
        cmp     eax, Image.bpp24
        je      .integer.bpp24
        cmp     eax, Image.bpp32
        je      .integer.bpp32
        mov     ecx, LIBIMG_ERROR_BIT_DEPTH
        jmp     .error

  .integer.bpp8g:
        push    [dst_width_pixels]
        pop [dst_width_bytes]
        mov ecx, [_param1]
;   cmp ecx, 1
;   je  .error
  .integer.bpp8g.common:
        mov edx, ecx
        mov ebx, [_crop_height]
  .integer.bpp8g.common.line:
        push    ebx
        mov ebx, [_crop_width]
        @@:
        lodsb
        mov ecx, edx
        rep stosb
        dec ebx
        jnz @b
        push    esi
        mov esi, edi
        sub esi, [dst_width_bytes]
        mov ecx, edx
        dec ecx
        imul    ecx, [dst_width_bytes]
        mov eax, ecx
        shr ecx, 2
        and eax, 0x00000003
        rep movsd
        mov ecx, eax
        rep movsb
        pop esi
        mov eax, [src_width_pixels]
        sub eax, [_crop_width]
        add esi, eax
        pop ebx
        dec ebx
        jnz .integer.bpp8g.common.line
        mov eax, [_dst]
        jmp .quit

  .integer.bpp24:
        mov eax, [dst_width_pixels]
        lea eax, [eax*3]
        mov [dst_width_bytes], eax
        mov ecx, [_param1]
;   cmp ecx, 1
;   je  .error
  .integer.bpp24.common:
        mov edx, ecx
        mov ebx, [_crop_height]
  .integer.bpp24.common.line:
        push    ebx
        mov ebx, [_crop_width]
        @@:
        movsw
        movsb
        mov ecx, edx
        push    esi
        mov esi, edi
        sub esi, 3
        dec ecx
        lea ecx, [ecx*3]
        rep movsb
        pop esi
        dec ebx
        jnz @b
        push    esi
        mov esi, edi
        sub esi, [dst_width_bytes]
        mov ecx, edx
        dec ecx
        imul    ecx, [dst_width_bytes]
        mov eax, ecx
        shr ecx, 2
        and eax, 0x00000003
        rep movsd
        mov ecx, eax
        rep movsb
        pop esi
        mov eax, [src_width_pixels]
        sub eax, [_crop_width]
        lea eax, [eax*3]
        add esi, eax
        pop ebx
        dec ebx
        jnz .integer.bpp24.common.line
        mov eax, [_dst]
        jmp .quit

  .integer.bpp32:
        mov eax, [dst_width_pixels]
        shl eax, 2
        mov [dst_width_bytes], eax
        mov ecx, [_param1]
;   cmp ecx, 1
;   je  .error
  .integer.bpp32.common:
        mov edx, ecx
        mov ebx, [_crop_height]
  .integer.bpp32.common.line:
        push    ebx
        mov ebx, [_crop_width]
        @@:
        lodsd
        mov ecx, edx
        rep stosd
        dec ebx
        jnz @b
        push    esi
        mov esi, edi
        sub esi, [dst_width_bytes]
        mov ecx, edx
        dec ecx
        imul    ecx, [dst_width_bytes]
        shr ecx, 2
        rep movsd
        pop esi
        mov eax, [src_width_pixels]
        sub eax, [_crop_width]
        shl eax, 2
        add esi, eax
        pop ebx
        dec ebx
        jnz .integer.bpp32.common.line
        mov eax, [_dst]
        jmp .quit


  .tile:
        mov eax, [_param1]
        mov [dst_width_pixels], eax
        imul    eax, [bytes_per_pixel]
        mov [dst_width_bytes], eax
        mov eax, [_param2]
        mov [dst_height_pixels], eax

        mov eax, [_dst]
        test    eax, eax
        jnz @f
        stdcall img.create, [dst_width_pixels], [dst_height_pixels], [src_type]
        test    eax, eax
        jz  .error
        mov [_dst], eax
        mov     edi, [_src]
        push    [edi + Image.Flags]
        pop     [eax + Image.Flags]
        push    [edi + Image.Delay]
        pop     [eax + Image.Delay]
        push    [edi + Image.Previous]
        pop     [eax + Image.Previous]
        push    [edi + Image.Next]
        pop     [eax + Image.Next]
        @@:
        mov edi, [eax + Image.Data]
        mov [dst_data], edi

        mov esi, [src_data]
        mov eax, [_crop_height]
        cmp eax, [dst_height_pixels]
        jna @f
        mov eax, [dst_height_pixels]
        @@:
        push    eax
        mov ecx, [_crop_width]
        cmp ecx, [dst_width_pixels]
        jna @f
        mov ecx, [dst_width_pixels]
        @@:
        imul    ecx, [bytes_per_pixel]
        mov edx, ecx
        @@:
        mov ecx, edx
        rep movsb

        push    esi
        mov esi, edi
        sub esi, edx
        mov ecx, [dst_width_bytes]
        sub ecx, edx
        rep movsb
        pop esi

        mov ecx, [src_width_bytes]
        sub ecx, edx
        add esi, ecx
        dec eax
        jnz @b

        pop eax
        mov esi, [dst_data]
        mov ecx, [dst_height_pixels]
        sub ecx, eax
        imul    ecx, [dst_width_bytes]
        rep movsb

        mov eax, [_dst]
        jmp .quit


  .bilinear:
        mov eax, [_dst]
        test    eax, eax
        jnz @f
        stdcall img.create, [dst_width_pixels], [dst_height_pixels], [src_type]
        test    eax, eax
        jz  .error
        mov [_dst], eax
        mov     edi, [_src]
        push    [edi + Image.Flags]
        pop     [eax + Image.Flags]
        push    [edi + Image.Delay]
        pop     [eax + Image.Delay]
        push    [edi + Image.Previous]
        pop     [eax + Image.Previous]
        push    [edi + Image.Next]
        pop     [eax + Image.Next]
    @@:
        mov edi, [eax + Image.Data]
        mov [dst_data], edi

        push    [_crop_width]
        pop [crop_width_pixels_m1]
        sub [crop_width_pixels_m1], 1
        push    [_crop_height]
        pop [crop_height_pixels_m1]
        sub [crop_height_pixels_m1], 1

        mov eax, 0xffffffff
        xor edx, edx
        div [dst_width_pixels]
        mov [dst_width_pixels_inv], eax
        mov eax, 0xffffffff
        xor edx, edx
        div [dst_height_pixels]
        mov [dst_height_pixels_inv], eax

        mov eax, [src_type]
        cmp eax, Image.bpp8g
        je  .bilinear.bpp8g
        cmp eax, Image.bpp24
        je  .bilinear.bpp24
        cmp eax, Image.bpp32
        je  .bilinear.bpp32
        mov ecx, LIBIMG_ERROR_BIT_DEPTH
        jmp .error

  .bilinear.bpp8g:
        mov esi, [src_data]
        mov [dst_y], 0
        mov eax, 0  ; mov eax, [dst_y]
  .bilinear.bpp8g.line:
        mov esi, [src_data]
        mov [dst_x], 0
        imul    eax, [crop_height_pixels_m1]
        xor edx, edx
        div [dst_height_pixels]
        mov [rem_y], edx
        imul    eax, [src_width_bytes]
        add esi, eax
        mov [src_base], esi
        mov eax, 0  ; mov eax, [dst_x]

  .bilinear.bpp8g.pixel:
        mov esi, [src_base]

        imul    eax, [crop_width_pixels_m1]
        xor edx, edx
        div [dst_width_pixels]
        add esi, eax

        mov ax, word[esi]
        add esi, [src_width_pixels]
        mov bx, word[esi]
        mov esi, edx
        movzx   edx, ah
        and eax, 0x000000ff
        movzx   ecx, bh
        and ebx, 0x000000ff

        imul    edx, esi
        imul    ecx, esi
        neg esi
        add esi, [dst_width_pixels]
        imul    eax, esi
        imul    ebx, esi
        add eax, edx
        add ebx, ecx
        mov esi, [dst_width_pixels_inv]
        mul esi
        mov ecx, edx
        mov eax, ebx
        mul esi
        mov eax, edx

        mov edx, [rem_y]
        imul    eax, edx
        neg edx
        add edx, [dst_height_pixels]
        imul    ecx, edx
        add eax, ecx
        mul [dst_height_pixels_inv]
        mov byte[edi], dl
        add edi, 1

        add [dst_x], 1
        mov eax, [dst_x]
        cmp eax, [dst_width_pixels]
        jne .bilinear.bpp8g.pixel

        add [dst_y], 1
        mov eax, [dst_y]
        cmp eax, [dst_height_pixels]
        jne .bilinear.bpp8g.line

        mov eax, [_dst]
        jmp .quit


  .bilinear.bpp24:
        mov esi, [src_data]
        mov [dst_y], 0
        mov eax, 0  ; mov eax, [dst_y]
  .bilinear.bpp24.line:
        mov esi, [src_data]
        mov [dst_x], 0
        imul    eax, [crop_height_pixels_m1]
        xor edx, edx
        div [dst_height_pixels]
        mov [rem_y], edx
        imul    eax, [src_width_bytes]
        add esi, eax
        mov [src_base], esi
        mov eax, 0  ; mov eax, [dst_x]

  .bilinear.bpp24.pixel:
        mov esi, [src_base]

        imul    eax, [crop_width_pixels_m1]
        xor edx, edx
        div [dst_width_pixels]
        lea eax, [eax*3]
        add esi, eax

        mov [rem_x], edx
        sub esi, 1
        mov [src_x], esi

repeat 3
        mov edx, [rem_x]
        add [src_x], 1
        mov esi, [src_x]

        mov al, byte[esi]
        mov ah, byte[esi + 3]
        add esi, [src_width_bytes]
        movzx   ebx, byte[esi]
        movzx   ecx, byte[esi + 3]
        mov esi, edx
        movzx   edx, ah
        and eax, 0x000000ff

        imul    edx, esi
        imul    ecx, esi
        neg esi
        add esi, [dst_width_pixels]
        imul    eax, esi
        imul    ebx, esi
        add eax, edx
        add ebx, ecx
        mov esi, [dst_width_pixels_inv]
        mul esi
        mov ecx, edx
        mov eax, ebx
        mul esi
        mov eax, edx

        mov edx, [rem_y]
        imul    eax, edx
        neg edx
        add edx, [dst_height_pixels]
        imul    ecx, edx
        add eax, ecx
        mul [dst_height_pixels_inv]
        mov byte[edi], dl
        add edi, 1
end repeat

        add [dst_x], 1
        mov eax, [dst_x]
        cmp eax, [dst_width_pixels]
        jne .bilinear.bpp24.pixel

        add [dst_y], 1
        mov eax, [dst_y]
        cmp eax, [dst_height_pixels]
        jne .bilinear.bpp24.line

        mov eax, [_dst]
        jmp .quit

  .bilinear.bpp32:
        mov esi, [src_data]
        mov [dst_y], 0
        mov eax, 0  ; mov eax, [dst_y]
  .bilinear.bpp32.line:
        mov esi, [src_data]
        mov [dst_x], 0
        imul    eax, [crop_height_pixels_m1]
        xor edx, edx
        div [dst_height_pixels]
        mov [rem_y], edx
        imul    eax, [src_width_bytes]
        add esi, eax
        mov [src_base], esi
        mov eax, 0  ; mov eax, [dst_x]

  .bilinear.bpp32.pixel:
        mov esi, [src_base]

        imul    eax, [crop_width_pixels_m1]
        xor edx, edx
        div [dst_width_pixels]
        shl eax, 2
        add esi, eax

        mov [rem_x], edx
        sub esi, 1
        mov [src_x], esi

repeat 4
        mov edx, [rem_x]
        add [src_x], 1
        mov esi, [src_x]

        mov al, byte[esi]
        mov ah, byte[esi + 4]
        add esi, [src_width_bytes]
        movzx   ebx, byte[esi]
        movzx   ecx, byte[esi + 4]
        mov esi, edx
        movzx   edx, ah
        and eax, 0x000000ff

        imul    edx, esi
        imul    ecx, esi
        neg esi
        add esi, [dst_width_pixels]
        imul    eax, esi
        imul    ebx, esi
        add eax, edx
        add ebx, ecx
        mov esi, [dst_width_pixels_inv]
        mul esi
        mov ecx, edx
        mov eax, ebx
        mul esi
        mov eax, edx

        mov edx, [rem_y]
        imul    eax, edx
        neg edx
        add edx, [dst_height_pixels]
        imul    ecx, edx
        add eax, ecx
        mul [dst_height_pixels_inv]
        mov byte[edi], dl
        add edi, 1
end repeat

        add [dst_x], 1
        mov eax, [dst_x]
        cmp eax, [dst_width_pixels]
        jne .bilinear.bpp32.pixel

        add [dst_y], 1
        mov eax, [dst_y]
        cmp eax, [dst_height_pixels]
        jne .bilinear.bpp32.line

        mov eax, [_dst]
        jmp .quit


  .error:
            xor     eax, eax
  .quit:
            pop     edi esi ebx
            ret
endp

