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


deflate_callback:
        mov     eax, [deflate_length]
        mov     ecx, [esp+8]
        mov     [ecx], eax
        mov     eax, [deflate_buffer]
        ret     8


encoding_ZRLE:

        DEBUGF  2, "ZRLE\n"
  @@:
        lea     eax, [esi+4]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

        lodsd
        bswap   eax                             ; number of packed bytes
        DEBUGF  2, "packed length=%u bytes\n", eax

  @@:
        push    eax
        add     eax, esi
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        pop     eax
        jmp     @b
  @@:
        pop     ecx
        lea     eax, [esi+ecx]
        push    eax                             ; end of data ptr

        cmp     [deflate_buffer], 0             ; first chunk of the stream?
        jne     @f
        lodsw                                   ; load zlib header
        cmp     ax, 0x9c78                      ; default compression?
        jne     .deflate_fail
        sub     ecx, 2                          ; subtract 2 byte header

;        invoke  deflate_init
;        mov     [deflate_str], eax
  @@:

        sub     ecx, 4                          ; subtract 4 byte check value trailer
        mov     [deflate_buffer], esi
        mov     [deflate_length], ecx

;        mov     eax, [deflate_str]
        push    eax                                             ; allocate place on stack
        invoke  deflate_unpack2, deflate_callback, 0, esp       ; unpack the data
        pop     ecx                                             ; get size in ecx
        test    eax, eax
        jz      .deflate_fail
        push    eax
        DEBUGF  2, "%u bytes unpacked succesfully\n", ecx
        test    ecx, ecx
        jz      .fail

        mov     esi, eax
        mov     [subrectangle.x], 0
        mov     [subrectangle.y], 0
  .tile:
        mov     eax, [rectangle.width]
        sub     eax, [subrectangle.x]
        cmp     eax, 64
        jb      @f
        mov     eax, 64
  @@:
        mov     [subrectangle.width], eax

        mov     edx, [rectangle.height]
        sub     edx, [subrectangle.y]
        cmp     edx, 64
        jb      @f
        mov     edx, 64
  @@:
        mov     [subrectangle.height], edx
        DEBUGF  1, "Subrectangle: x=%u y=%u width=%u height=%u ", \
        [subrectangle.x], [subrectangle.y], [subrectangle.width], [subrectangle.height]

; Calculate first pixel pos
        mov     eax, [rectangle.y]
        add     eax, [subrectangle.y]
        movzx   ebx, [screen.width]
        mul     ebx
        add     eax, [rectangle.x]
        add     eax, [subrectangle.x]
        lea     edi, [framebuffer+eax*3]

; Calculate offset between two rows of pixels
        movzx   eax, [screen.width]
        sub     eax, [subrectangle.width]
        lea     ebp, [eax*3]

        mov     edx, [subrectangle.height]

;;;;
        lodsb                   ; subencoding type
        DEBUGF  1, "encoding=%u\n", al
        test    al, al
        jz      .raw
        cmp     al, 1
        je      .solid
        cmp     al, 16
        jbe     .palette_packed
        cmp     al, 127
        je      .reuse_palette
        jb      .invalid
        and     al, 0x7f
        jz      .plain_rle
        cmp     al, 1
        je      .prle_reuse_palette

; Palette Run Length Encoded tile
        mov     [palettesize], al
        call    create_palette
  .prle_reuse_palette:
        DEBUGF  1, "Pallete RLE tile\n"

        mov     eax, 1
  .prle_line:
        mov     ebx, [subrectangle.width]
  .prle_pixel:
        dec     eax
        jz      .prle_reload

        mov     word[edi], cx
        rol     ecx, 16
        add     edi, 2
        mov     byte[edi], cl
        rol     ecx, 16
        inc     edi
        dec     ebx
        jnz     .prle_pixel

        add     edi, ebp
        dec     edx
        jnz     .prle_line
        jmp     .next_tile

  .prle_reload:
;;;
; load palette index and get color from palette
        xor     eax, eax
        lodsb
        push    ebx
        mov     bl, al
        and     al, 0x7f
        mov     ecx, [palette+eax*4]

; run length follows?
        xor     eax, eax
        test    bl, 0x80
        pop     ebx
        jz      .plength_ok

;;;

; load runlength
        push    ebx
        xor     eax, eax
        xor     ebx, ebx
  @@:
        lodsb
        cmp     al, 255
        jne     @f
        add     ebx, eax
        jmp     @b
  @@:
        add     eax, ebx
        pop     ebx
  .plength_ok:
        add     eax, 2
        jmp     .prle_pixel




; Run Length Encoded tile
  .plain_rle:

        DEBUGF  1, "Plain RLE tile\n"

        mov     eax, 1
  .rle_line:
        mov     ebx, [subrectangle.width]
  .rle_pixel:
        dec     eax
        jz      .rle_reload

        mov     word[edi], cx
        rol     ecx, 16
        add     edi, 2
        mov     byte[edi], cl
        rol     ecx, 16
        inc     edi
        dec     ebx
        jnz     .rle_pixel
        add     edi, ebp
        dec     edx
        jnz     .rle_line
        jmp     .next_tile

  .rle_reload:
; TODO: check for buffer underrun

        ; load pixel value
        call    load_cpixel
        mov     ecx, eax

        ; load length
        xor     eax, eax
        push    ebx
        xor     ebx, ebx
  @@:
        lodsb
        cmp     al, 255
        jne     @f
        add     ebx, eax
        jmp     @b
  @@:
        add     eax, ebx
        add     eax, 2
        pop     ebx
        jmp     .rle_pixel



  .reuse_palette:
        cmp     [palettesize], 1
        jne     .reuse_palette_
        mov     ecx, [palette]
        mov     eax, ecx
        shr     eax, 16
        jmp     .solid_line

; Palette packed tile
  .palette_packed:
        DEBUGF  1, "Palette packed tile\n"

        mov     [palettesize], al
        call    create_palette

  .reuse_palette_:
        cmp     [palettesize], 2
        je      .palette_1bit
        cmp     [palettesize], 4
        jbe     .palette_2bit
        jmp     .palette_4bit

  .palette_1bit:
        DEBUGF  1, "1-bit palette\n"
  .palette_1bit_line:
        mov     ebx, [subrectangle.width]
  .palette_1bit_byte:
        lodsb
        rol     al, 1
        mov     ecx, eax
        and     eax, 0x1
        mov     eax, [palette+4*eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_1bit_next_line

        rol     cl, 1
        mov     eax, ecx
        and     eax, 0x1
        mov     eax, [palette+4*eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_1bit_next_line

        rol     cl, 1
        mov     eax, ecx
        and     eax, 0x1
        mov     eax, [palette+4*eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_1bit_next_line

        rol     cl, 1
        mov     eax, ecx
        and     eax, 0x1
        mov     eax, [palette+4*eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_1bit_next_line

        rol     cl, 1
        mov     eax, ecx
        and     eax, 0x1
        mov     eax, [palette+4*eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_1bit_next_line

        rol     cl, 1
        mov     eax, ecx
        and     eax, 0x1
        mov     eax, [palette+4*eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_1bit_next_line

        rol     cl, 1
        mov     eax, ecx
        and     eax, 0x1
        mov     eax, [palette+4*eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_1bit_next_line

        rol     cl, 1
        and     ecx, 0x1
        mov     eax, [palette+4*ecx]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jnz     .palette_1bit_byte

  .palette_1bit_next_line:
        add     edi, ebp
        dec     edx
        jnz     .palette_1bit_line
        jmp     .next_tile



  .palette_2bit:
        DEBUGF  1, "2-bit palette\n"
  .palette_2bit_line:
        mov     ebx, [subrectangle.width]
  .palette_2bit_byte:
        lodsb
        mov     ecx, eax
        and     eax, 0xc0
        shr     eax, 4
        mov     eax, [palette+eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_2bit_next_line

        mov     eax, ecx
        and     eax, 0x30
        shr     eax, 2
        mov     eax, [palette+eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_2bit_next_line

        mov     eax, ecx
        and     eax, 0x0c
        mov     eax, [palette+eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_2bit_next_line

        and     ecx, 0x03
        mov     eax, [palette+4*ecx]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jnz     .palette_2bit_byte

  .palette_2bit_next_line:
        add     edi, ebp
        dec     edx
        jnz     .palette_2bit_line
        jmp     .next_tile




  .palette_4bit:
        DEBUGF  1, "4-bit palette\n"
  .palette_4bit_line:
        mov     ebx, [subrectangle.width]
  .palette_4bit_byte:
        lodsb
        mov     cl, al
        and     eax, 0xf0
        shr     eax, 2
        mov     eax, [palette+eax]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jz      .palette_4bit_next_line

        and     ecx, 0x0f
        shl     ecx, 2
        mov     eax, [palette+ecx]
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jnz     .palette_4bit_byte
  .palette_4bit_next_line:
        add     edi, ebp
        dec     edx
        jnz     .palette_4bit_line
        jmp     .next_tile


; RAW tile
  .raw:
        push    edx
        mov     eax, [subrectangle.width]
        mul     [subrectangle.height]
        lea     eax, [eax*3]
        pop     edx

; TODO: check for buffer underrun

        DEBUGF  1, "RAW tile\n"
  .raw_line:
        mov     ebx, [subrectangle.width]
  .raw_pixel:
        call    load_cpixel
        stosw
        shr     eax, 16
        stosb
        dec     ebx
        jnz     .raw_pixel
        add     edi, ebp
        dec     edx
        jnz     .raw_line
        jmp     .next_tile



; Single color tile
  .solid:
        DEBUGF  1, "Solid tile\n"

; TODO: check for buffer underrun

        call    load_cpixel
        mov     ecx, eax
        shr     ecx, 16

        mov     [palettesize], 1
        mov     [palette], eax

  .solid_line:
        mov     ebx, [subrectangle.width]
  .solid_pixel:
        stosw
        mov     [edi], cl
        inc     edi
        dec     ebx
        jnz     .solid_pixel
        add     edi, ebp
        dec     edx
        jnz     .solid_line
;        jmp     .next_tile


; Go to the next tile
  .next_tile:
        mov     eax, [subrectangle.x]
        add     eax, 64
        cmp     eax, [rectangle.width]
        jae     .next_row
        mov     [subrectangle.x], eax
        jmp     .tile
  .next_row:
        mov     [subrectangle.x], 0
        mov     eax, [subrectangle.y]
        add     eax, 64
        cmp     eax, [rectangle.height]
        jae     .done
        mov     [subrectangle.y], eax
        jmp     .tile

  .done:
        DEBUGF  1, "ZRLE complete!\n"
        pop     ecx
        mcall   68, 13                          ; free the unpacked data
        pop     esi
        jmp     next_rectangle

  .invalid:
        DEBUGF  2, "Invalid subencoding type!\n"
  .fail:
        DEBUGF  2, "ZRLE failed!\n"
        pop     ecx
        mcall   68, 13                          ; free unpacked data
        pop     esi
        jmp     next_rectangle


  .deflate_fail:
        DEBUGF  2,"Unpacking failed!\n"
        pop     esi
        jmp     next_rectangle