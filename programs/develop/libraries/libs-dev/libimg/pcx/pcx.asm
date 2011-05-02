;;================================================================================================;;
;;//// pcx.asm //// (c) dunkaist, 2010 ///////////////////////////////////////////////////////////;;
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

include 'pcx.inc'
;include '../../../../system/board/trunk/debug.inc'

;;================================================================================================;;
proc img.is.pcx _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in pcx format)                                      ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;

    push    ecx edi
    xor     eax,    eax

    mov     edi,    [_data]

    cmp     [edi+pcx_header.magic_number],  10
     jne    .is_not_pcx
    cmp     [edi+pcx_header.version],  5
     jne    .is_not_pcx
    cmp     [edi+pcx_header.encoding],  1
     jne    .is_not_pcx
    cmp     [edi+pcx_header.reserved],  0
     jne    .is_not_pcx

    add     edi,    pcx_header.filler
    xor     al,     al
    mov     ecx,    58
    cld
    repe    scasb
    test    ecx,    ecx
     jnz    .is_not_pcx

.is_pcx:
    inc     eax

.is_not_pcx:
    pop     edi ecx
    ret

endp

;;================================================================================================;;
proc img.decode.pcx _data, _length, _options ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in pcx format                  ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
nplanes             rd      1
xsize               rw      1
ysize               rw      1
bpl                 rw      1
total_bpl           rd      1
line_begin          rd      1
retvalue            rd      1                       ; 0 (error) or pointer to image 
endl

    pusha

    mov     esi,    [_data]
    movzx   eax,    byte[esi+pcx_header.nplanes]
    mov     [nplanes],  eax
    mov     bx,  word[esi+pcx_header.bpl]
    mov     [bpl],  bx
    mul     bx
    shl     eax,    16
    mov     ax,     dx
    ror     eax,    16
    mov     [total_bpl],    eax

    movzx   eax,    word[esi+pcx_header.xmax]
    inc     ax
    sub     ax,     word[esi+pcx_header.xmin]
    mov     [xsize],    ax

    movzx   ebx,    word[esi+pcx_header.ymax]
    inc     bx
    sub     bx,     word[esi+pcx_header.ymin]
    mov     [ysize],    bx


    cmp     [esi+pcx_header.bpp],   1
     jz     .monochrome
    cmp     byte[esi+pcx_header.nplanes],   3
     jnz    .indexed


  ._24bit:

      stdcall   img.create, eax, ebx, Image.bpp24
    mov     [retvalue], eax
    test    eax,    eax
     jz     .quit

    movzx   ebx,    [xsize]
    movzx   ecx,    [ysize]
    mov     edx,    [eax+Image.Data]

    rol     ebx,    16
    or      ebx,    ecx
    xor     ebx,    [edx]
    mov     [eax+Image.Checksum],   ebx


    mov     esi,    [_data]
    add     esi,    128
;    mov     edi,    [retvalue]
    mov     edi,    [eax+Image.Data]
    add     edi,    2
    mov     [line_begin],   edi
    mov     ebx,    [total_bpl]

  ._24bit.begin:
    mov     ax,     word[bpl]
  ._24bit.decode:
      call      get_byte
  ._24bit.write_sequence:
    mov     [edi],  dl
    dec     ax
    add     edi,    [nplanes]
    dec     dh
     jnz    ._24bit.write_sequence

    test    ax,     ax
     jz     ._24bit.end_color_line
     jmp    ._24bit.decode

 ._24bit.end_color_line:
    test    ebx,    ebx
     jz     ._24bit.end_full_line
    dec     [line_begin]
    mov     edi,    [line_begin]
     jmp    ._24bit.begin

  ._24bit.end_full_line:
    dec     word[ysize]
     jz     .quit
    mov     ebx,    [total_bpl]
    add     edi,    2
    mov     [line_begin],   edi
     jmp    ._24bit.begin


  .indexed:

      stdcall   img.create, eax, ebx, Image.bpp8
    mov     [retvalue], eax
    test    eax,    eax
     jz     .quit

    movzx   ebx,    [xsize]
    movzx   ecx,    [ysize]
    mov     edx,    [eax+Image.Data]

    rol     ebx,    16
    or      ebx,    ecx
    xor     ebx,    [edx]
    mov     [eax+Image.Checksum],   ebx

    mov     esi,    [_data]
    add     esi,    [_length]
    sub     esi,    768
    mov     edi,    [eax+Image.Palette]
    mov      cx,    256
  @@:
    mov     ebx,    [esi]
    bswap   ebx
    shr     ebx,    8
    mov     [edi],  ebx
    add     edi,    4
    add     esi,    3
    dec     cx
     jnz    @b

    mov     esi,    [_data]
    add     esi,    128
;    mov     edi,    [retvalue]
    mov     edi,    [eax+Image.Data]

  .indexed.begin:
    mov     ax,     word[bpl]
  .indexed.decode:
      call      get_byte
  .indexed.write_sequence:
    mov     [edi], dl
    inc     edi
    dec     ax
    dec     dh
     jnz    .indexed.write_sequence

    test    ax,     ax
     jz     .indexed.end_line
     jmp    .indexed.decode

  .indexed.end_line:
    dec     word[ysize]
     jz     .quit
     jmp    .indexed.begin


  .monochrome:

      stdcall   img.create, eax, ebx, Image.bpp1
    mov     [retvalue], eax
    test    eax,    eax
     jz     .quit

    movzx   ebx,    [xsize]
    movzx   ecx,    [ysize]
    mov     edx,    [eax+Image.Data]

    rol     ebx,    16
    or      ebx,    ecx
    xor     ebx,    [edx]
    mov     [eax+Image.Checksum],   ebx

    mov     edi,    [eax+Image.Palette]
    mov     [edi],  dword   0x00000000
    mov     [edi+4],    dword   0x00ffffff

    mov     esi,    [_data]
    add     esi,    128
;    mov     edi,    [retvalue]
    mov     edi,    [eax+Image.Data]


  .monochrome.begin:
    mov     ebx,    [total_bpl]
    mov     ax,     [xsize]

  .monochrome.decode:
      call      get_byte
  .monochrome.write_sequence:
    mov     [edi],  dl
    inc     edi
    cmp     ax,     8
     jng    .monochrome.is_last_byte_in_line
    sub     ax,     8
    dec     dh
     jnz    .monochrome.write_sequence
     jmp    .monochrome.decode

  .monochrome.is_last_byte_in_line:
    test    ebx,    ebx
     jng    @f
      call      get_byte
     jmp    .monochrome.is_last_byte_in_line
  @@:
    dec     word[ysize]
     jnz    .monochrome.begin
     jmp    .quit


  .quit:
    popa
    mov     eax,    [retvalue]
    ret

endp



;;================================================================================================;;
proc img.encode.pcx _img, _p_length, _options ;///////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in pcx format                                                     ;;
;;------------------------------------------------------------------------------------------------;;
;> _img = pointer to image                                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to encoded data                                                     ;;
;< _p_length = encoded data length                                                                ;;
;;================================================================================================;;
    xor eax, eax
    ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below are private procs you should never call directly from your code                          ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
proc get_byte

    mov     dh,     byte[esi]
    inc     esi
    cmp     dh,     0xC0
     jnb    .cycle1
    mov     dl,     dh
    mov     dh,     1
     jmp    .exit1
  .cycle1:
    and     dh,     0x3F
    mov     dl,     byte[esi]
    inc     esi
  .exit1:
    movzx   ecx,     dh
    sub     ebx,    ecx

    ret
endp
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;

;