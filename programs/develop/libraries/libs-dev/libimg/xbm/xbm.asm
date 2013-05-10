;;================================================================================================;;
;;//// xbm.asm //// (c) dunkaist, 2013 ///////////////////////////////////////////////////////////;;
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
proc img.is.xbm _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in xbm format)                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;

	push	ebx ecx esi edi
	xor	eax, eax

	mov	edi, [_data]
        mov     ecx, 4          ; BOM
        mov     al, '#'
        repne   scasb
        jne     .is_not_xbm
        dec     edi
        mov     esi, str_define
        mov     ecx, str_define.size
        repe    cmpsb
        jne     .is_not_xbm
        mov     esi, edi
        call    xbm._.skip_whitespace
        mov     edx, esi
        call    xbm._.wait_whitespace
        mov     ebx, esi
        sub     ebx, edx
        sub     ebx, 5
        cmp     word[esi - 6], '_w'
        jne     .is_not_xbm
        cmp     dword[esi - 4], 'idth'
        jne     .is_not_xbm
        call    xbm._.skip_whitespace
        call    xbm._.wait_whitespace
        call    xbm._.skip_whitespace
        mov     edi, str_define
        mov     ecx, str_define.size
        repe    cmpsb
        jne     .is_not_xbm
        call    xbm._.skip_whitespace
        mov     edi, edx
        mov     ecx, ebx
        repe    cmpsb
        jne     .is_not_xbm
        cmp     dword[esi], 'heig'
        jne     .is_not_xbm
        cmp     word[esi + 4], 'ht'
        jne     .is_not_xbm

  .is_xbm:
	pop	edi esi ecx ebx
        xor     eax, eax
	inc	eax
        ret
  .is_not_xbm:
	pop	edi esi ecx ebx
        xor     eax, eax
	ret
endp


;;================================================================================================;;
proc img.decode.xbm _data, _length, _options ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in xbm format                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
        width           rd 1
        height          rd 1
        counter         rd 1
	retvalue        rd 1    ; 0 (error) or pointer to image 
endl

	pusha

        mov     esi, [_data]
        add     esi, 5
        call    xbm._.wait_whitespace
        call    xbm._.skip_whitespace
        call    xbm._.wait_whitespace
        call    xbm._.skip_whitespace
        call    xbm._.read_number
        mov     [width], eax
        call    xbm._.skip_whitespace
        call    xbm._.wait_whitespace
        call    xbm._.skip_whitespace
        call    xbm._.wait_whitespace
        call    xbm._.skip_whitespace
        call    xbm._.read_number
        mov     [height], eax
        stdcall img.create, [width], [height], Image.bpp1
        test    eax, eax
        jz      .quit
        mov     [retvalue], eax

        mov     edi, [_data]
        mov     ecx, [_length]
        mov     eax, '{'
        repne   scasb
        mov     esi, edi

        mov     ebx, [retvalue]
        mov     eax, [ebx + Image.Palette]
        mov     dword[eax], 0xffffffff
        mov     dword[eax + 4], 0xff000000
        mov     ecx, [ebx + Image.Width]
        add     ecx, 7
        shr     ecx, 3
        imul    ecx, [ebx + Image.Height]
        mov     [counter], ecx
        mov     edi, [ebx + Image.Data]
    @@:
        call    xbm._.skip_whitespace
        call    xbm._.read_number
        mov     cl, al
        and     eax, 0x0f
        mov     al, byte[reverse_bits + eax]
        shl     eax, 4
        mov     edx, eax
        mov     al, cl
        shr     eax, 4
        mov     al, byte[reverse_bits + eax]
        or      eax, edx
        mov     byte[edi], al
        add     edi, 1
        sub     [counter], 1
        jnz     @b
  .quit:
	popa
	mov	eax, [retvalue]
	ret
endp


;;================================================================================================;;
proc img.encode.xbm _img, _common, _specific ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in xbm format                                                       ;;
;;------------------------------------------------------------------------------------------------;;
;> [_img]      = pointer to image                                                                 ;;
;> [_common]   = format independent options                                                       ;;
;> [_specific] = 0 / pointer to the structure of format specific options                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 / pointer to encoded data                                                              ;;
;< ecx = error code / the size of encoded data                                                    ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below are private procs you should never call directly from your code                          ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
proc xbm._.skip_whitespace
  .next_byte:
        movzx   eax, byte[esi]
        cmp     eax, 0x20
        jne     @f
        add     esi, 1
        jmp     .next_byte
    @@:
        cmp     eax, 0x09
        jne     @f
        add     esi, 1
        jmp     .next_byte
    @@:
        cmp     eax, 0x0a
        jne     @f
        add     esi, 1
        jmp     .next_byte
    @@:
        ret
endp


proc xbm._.wait_whitespace
    @@:
        movzx   eax, byte[esi]
        cmp     eax, 0x20
        je      @f
        cmp     eax, 0x09
        je      @f
        cmp     eax, 0x0a
        je      @f
        add     esi, 1
        jmp     @b
    @@:
        ret
endp


proc xbm._.read_number
        xor     eax, eax
        mov     ecx, eax
        mov     al, byte[esi]
        add     esi, 1
        sub     al, 0x30
        test    al, al
        jz     .octal_or_hex
  .decimal:
        mov     ebx, 10
        imul    ecx, ebx
        add     ecx, eax
        mov     al, byte[esi]
        add     esi, 1
        sub     al, 0x30
        cmp     al, 10
        jb      .decimal
        jmp     .done
  .octal_or_hex:
        mov     al, byte[esi]
        add     esi, 1
        sub     al, 0x30
        cmp     al, 'x' - 0x30
        je      .hex
  .octal:
        jmp     .done
  .hex:
        mov     al, byte[esi]
        add     esi, 1
        sub     al, 0x30
        jc      .done
        cmp     al, 9
        jna     @f
        sub     al, 0x30 - 9
        jc      .done
        cmp     al, 15
        ja      .done
    @@:
        shl     ecx, 4
        add     ecx, eax
        jmp     .hex
  .done:
        mov     eax, ecx
        ret
endp
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
reverse_bits    db 0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e, 0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f
sz str_define   , '#define'
sz str__width   , '_width'
sz str__height  , '_height'

