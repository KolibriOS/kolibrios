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


encoding_cursor:

        DEBUGF  1, "Set cursor: width=%u height=%u\n", [rectangle.width], [rectangle.height]

        mov     eax, [rectangle.width]
        mul     [rectangle.height]
        mov     ebx, eax
        add     ebx, 7
        shr     ebx, 3
        mov     ecx, BYTES_PER_PIXEL
        mul     ecx
        lea     ecx, [eax+ebx]

  @@:
        lea     eax, [esi+ecx]
        cmp     [datapointer], eax
        jae     @f
        call    read_data.more
        jmp     @b
  @@:

; TODO: chop larger cursor sizes to 32x32 ?
        cmp     [rectangle.width], 32
        ja      .fail
        cmp     [rectangle.height], 32
        ja      .fail

; Convert cursor image to 32*32 ARGB format
        mov     edi, cursor.image
        mov     edx, [rectangle.height]
        test    edx, edx
        jz      .zero_height
  .lineloop:
        mov     ecx, [rectangle.width]
        test    ecx, ecx
        jz      .zero_width
  .pixelloop:
        call    load_pixel
        stosd
        dec     ecx
        jnz     .pixelloop

  .zero_width:
        mov     ecx, 32
        sub     ecx, [rectangle.width]
        jz      @f
        xor     eax, eax
        rep stosd
  @@:
        dec     edx
        jnz     .lineloop

  .zero_height:
        mov     ecx, 32
        sub     ecx, [rectangle.height]
        jz      @f
        shl     ecx, 5  ; mul 32
        xor     eax, eax
        rep stosd
  @@:

        mov     edi, cursor.image+3
        mov     edx, [rectangle.height]
        test    edx, edx
        jz      .finish
  .zloop:
        mov     ecx, [rectangle.width]
        test    ecx, ecx
        jz      .finish
  .aloop:
        lodsb

        mov     bl, al
        shl     bl, 1
        salc
        mov     byte[edi], al
        add     edi, 4
        dec     ecx
        jz      .n

        shl     bl, 1
        salc
        mov     byte[edi], al
        add     edi, 4
        dec     ecx
        jz      .n

        shl     bl, 1
        salc
        mov     byte[edi], al
        add     edi, 4
        dec     ecx
        jz      .n

        shl     bl, 1
        salc
        mov     byte[edi], al
        add     edi, 4
        dec     ecx
        jz      .n

        shl     bl, 1
        salc
        mov     byte[edi], al
        add     edi, 4
        dec     ecx
        jz      .n

        shl     bl, 1
        salc
        mov     byte[edi], al
        add     edi, 4
        dec     ecx
        jz      .n

        shl     bl, 1
        salc
        mov     byte[edi], al
        add     edi, 4
        dec     ecx
        jz      .n

        shl     bl, 1
        salc
        mov     byte[edi], al
        add     edi, 4
        dec     ecx
        jz      .n
        jmp     .aloop

  .n:
        mov     eax, 32
        sub     eax, [rectangle.width]
        shl     eax, 2
        add     edi, eax
        dec     edx
        jnz     .zloop

  .finish:
        mov     eax, [rectangle.x]
        mov     [cursor.x], al
        mov     eax, [rectangle.y]
        mov     [cursor.y], al
        or      [work], WORK_CURSOR

        DEBUGF  1, "Set cursor succeeded\n"
        jmp     next_rectangle

  .fail:
        add     esi, ecx
        DEBUGF  2, "Set cursor failed!\n"
        jmp     next_rectangle
