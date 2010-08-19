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
include '../../../../system/board/trunk/debug.inc'

;;================================================================================================;;
proc img.is.pcx _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in Targa format)                                    ;;
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
    cmp     [esi+pcx_header.reserved],  0
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
;? Decode data into image if it contains correctly formed raw data in Targa format                ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
locals
;  IMGwidth      dd ?
;  IMGheight     dd ?
;  IMGbpp        dd ?
buf                 rb      1
nplanes             rd      1
xsize               rw      1
ysize               rw      1
stxsize             rw      1
stysize             rw      1
total_bpl           rd      1
total_bpl_tmp       rd      1
line_begin          rd      1
retvalue            rd      1

endl

    pusha

    mov     esi,    [_data]

    cmp     [esi+pcx_header.bpp],   1
     jz     monochrome
    cmp     byte[esi+pcx_header.nplanes],   3
     jnz    indexed



  _24bit:
    xor     eax,   eax
    mov     al,    byte[esi+pcx_header.nplanes]
    mov     [nplanes],  eax
    mul     word[esi+pcx_header.bpl]
    mov     [total_bpl],    eax

    movzx   eax,    word[esi+pcx_header.xmax]
    inc     ax
    sub     ax,     word[esi+pcx_header.xmin]
    mov     [xsize],    ax

    movzx   ebx,    word[esi+pcx_header.ymax]
    inc     bx
    sub     bx,     word[esi+pcx_header.ymin]
    mov     [ysize],    bx

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
    mov     edi,    [retvalue]
    mov     edi,    [edi+Image.Data]
    add     edi,    2
    mov     [line_begin],   edi
    mov     ebx,    [total_bpl]

  .begin:
    mov     eax,    [_data]
    mov     ax,     word[eax+pcx_header.bpl]
  .decode:
    mov     dl,     byte[esi]
    inc     esi
    mov     [buf],  dl
    and     dl,     0xC0
    cmp     dl,     0xC0
     jne    @f
    mov     dl,     byte[buf]
    and     dl,     0x3F
    mov     dh,     [esi]
    inc     esi

  .write_sequence:
    mov     [edi], dh
    dec     ax
    dec     ebx
    add     edi,    [nplanes]
    dec     dl
    test    dl,     dl
     jnz    .write_sequence

    test    ax,     ax
     jz     .end_color_line
     jmp    .decode
  @@:
    mov     dl,     byte[buf]
    mov     [edi],  dl
    add     edi, [nplanes]
    dec     ebx
    dec     ax
     jz     .end_color_line
     jmp    .decode

 .end_color_line:
    test    ebx,    ebx
     jz     .end_full_line
    dec     [line_begin]
    mov     edi,    [line_begin]
     jmp    .begin

  .end_full_line:
    dec     word[ysize]
     jz     .quit
    mov     ebx,    [total_bpl]
    add     edi,    2
    mov     [line_begin],   edi
     jmp    .begin

  .quit:
    popa
    mov     eax,    [retvalue]
    ret

  indexed:

    xor     eax,   eax
    mov     al,    byte[esi+pcx_header.nplanes]
    mov     [nplanes],  eax
    mul     word[esi+pcx_header.bpl]
    mov     [total_bpl],    eax

    movzx   eax,    word[esi+pcx_header.xmax]
    inc     ax
    sub     ax,     word[esi+pcx_header.xmin]
    mov     [xsize],    ax

    movzx   ebx,    word[esi+pcx_header.ymax]
    inc     bx
    sub     bx,     word[esi+pcx_header.ymin]
    mov     [ysize],    bx

      stdcall   img.create, eax, ebx, Image.bpp8
    mov     [retvalue], eax
    test    eax,    eax
     jz     .quit

    mov     esi,    [_data]
    add     esi,    [_length]
    sub     esi,    768
    mov     edi,    [eax+Image.Palette]
    mov     ecx,    256
  @@:
    mov     ebx,    [esi]
    and     ebx,    0x00ffffff
    bswap   ebx
    shr     ebx,    8
    mov     [edi],  ebx
    add     edi,    4
    add     esi,    3
    dec     ecx
     jnz    @b

    movzx   ebx,    [xsize]
    movzx   ecx,    [ysize]
    mov     edx,    [eax+Image.Data]

    rol     ebx,    16
    or      ebx,    ecx
    xor     ebx,    [edx]
    mov     [eax+Image.Checksum],   ebx


    mov     esi,    [_data]
    add     esi,    128
    mov     edi,    [retvalue]
    mov     edi,    [edi+Image.Data]

  .begin:
    mov     eax,    [_data]
    mov     ax,     word[eax+pcx_header.bpl]
  .decode:
    mov     dl,     byte[esi]
    inc     esi
    mov     [buf],  dl
    and     dl,     0xC0
    cmp     dl,     0xC0
     jne    @f
    mov     dl,     [buf]
    and     dl,     0x3F
    mov     dh,     [esi]
    inc     esi

  .write_sequence:
    mov     [edi], dh
    inc     edi
    dec     ax
    dec     dl
     jnz    .write_sequence

    test    ax,     ax
     jz     .end_line
     jmp    .decode
  @@:
    mov     dl,     byte[buf]
    mov     [edi],  dl
    inc     edi
    dec     ax
     jz     .end_line
     jmp    .decode

  .end_line:
    dec     word[ysize]
     jz     .quit
     jmp    .begin

  .quit:
    popa
    mov     eax,    [retvalue]
    ret


  monochrome:

    xor     eax,    eax
    mov     ax,     word[esi+pcx_header.bpl]
    mov     [total_bpl],    eax

    movzx   eax,    word[esi+pcx_header.xmax]
    inc     ax
    sub     ax,     word[esi+pcx_header.xmin]
    mov     [xsize],    ax

    movzx   ebx,    word[esi+pcx_header.ymax]
    inc     bx
    sub     bx,     word[esi+pcx_header.ymin]
    mov     [ysize],    bx

      stdcall   img.create, eax, ebx, Image.bpp8
    mov     [retvalue], eax
    test    eax,    eax
     jz     .quit

    mov     edi,    [eax+Image.Palette]
    mov     [edi],  dword   0x00000000
    mov     [edi+4],    dword   0x00ffffff

    movzx   ebx,    [xsize]
    movzx   ecx,    [ysize]
    mov     edx,    [eax+Image.Data]

    rol     ebx,    16
    or      ebx,    ecx
    xor     ebx,    [edx]
    mov     [eax+Image.Checksum],   ebx


    mov     esi,    [_data]
    add     esi,    128
    mov     edi,    [retvalue]
    mov     edi,    [edi+Image.Data]

  .begin:
    mov     eax,    [total_bpl]
    mov     [total_bpl_tmp],    eax
    mov     ax,     [xsize]

  .decode:

    mov     dh,     byte[esi]
    inc     esi
    mov     [buf],  dh
    and     dh,     0xC0
    cmp     dh,     0xC0
     je    .cycle1
    mov     dh,     1
    mov     dl,     [buf]
     jmp    .exit1
  .cycle1:
    mov     dh,     [buf]
    and     dh,     0x3F
    mov     dl,     byte[esi]
    inc     esi
  .exit1:
    push    eax
    xor     eax,    eax
    mov     al,     dh
    sub     [total_bpl_tmp],    eax
    pop     eax


  .write_sequence:
    mov     ecx,    7
  .go:
    bt      edx,    ecx
     jnc    @f
    mov     [edi],  byte    0x01
     jmp    .later
  @@:
    mov     [edi],  byte    0x00
  .later:
    inc     edi
    dec     ax
     jnz    .lol
  @@:
    cmp     [total_bpl_tmp],    0
     jng    @f

    mov     dh,     byte[esi]
    inc     esi
    mov     [buf],  dh
    and     dh,     0xC0
    cmp     dh,     0xC0
     je    .cycle2
    mov     dh,     1
    mov     dl,     [buf]
     jmp    .exit2
  .cycle2:
    mov     dh,     [buf]
    and     dh,     0x3F
    mov     dl,     byte[esi]
    inc     esi
  .exit2:
    push    eax
    xor     eax,    eax
    mov     al,     dh
    sub     [total_bpl_tmp],    eax
    pop     eax

     jmp    @b
  @@:
    dec     word[ysize]
     jnz    .begin
     jmp    .quit
  .lol:
    dec     ecx
    cmp     ecx,    -1
     jne    .go
    dec     dh
     jnz    .write_sequence
     jmp    .decode

  .quit:
    popa
    mov     eax,    [retvalue]
    ret

endp



;;================================================================================================;;
proc img.encode.pcx _img, _p_length, _options ;///////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in Targa format                                                     ;;
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

;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;

;