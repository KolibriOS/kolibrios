;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2020-2020. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;      Version 2, or (at your option) any later version.       ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

unicode.utf8.decode:
        test    ecx, ecx
        jz      .error
        movzx   eax, byte[esi]
        test    al, al
        jns     .read1
        shl     al, 2
        jnc     .error
        shl     al, 1
        jnc     .read2
        shl     al, 1
        jnc     .read3
        shl     al, 1
        jnc     .read4
        jmp     .error
.read1:
        sub     ecx, 1
        add     esi, 1
        jmp     .done
.read2:
        cmp     ecx, 2
        jb      .error
        shl     eax, 5
        mov     al, [esi+1]
        shl     al, 1
        jnc     .error
        shl     al, 1
        jc      .error
        shr     eax, 2
        sub     ecx, 2
        add     esi, 2
        jmp     .done
.read3:
        cmp     ecx, 3
        jb      .error
        shl     eax, 4
        mov     al, [esi+1]
        shl     al, 1
        jnc     .error
        shl     al, 1
        jc      .error
        shl     eax, 6
        mov     al, [esi+2]
        shl     al, 1
        jnc     .error
        shl     al, 1
        jc      .error
        shr     eax, 2
        sub     ecx, 3
        add     esi, 3
        jmp     .done
.read4:
        cmp     ecx, 4
        jb      .error
        shl     eax, 3
        mov     al, [esi+1]
        shl     al, 1
        jnc     .error
        shl     al, 1
        jc      .error
        shl     eax, 6
        mov     al, [esi+2]
        shl     al, 1
        jnc     .error
        shl     al, 1
        jc      .error
        shl     eax, 6
        mov     al, [esi+3]
        shl     al, 1
        jnc     .error
        shl     al, 1
        jc      .error
        shr     eax, 2
        sub     ecx, 4
        add     esi, 4
        jmp     .done
.error:
.done:
        ret


unicode.cp866.encode:
        call    uni2ansi_char
        ret


unicode.utf16.encode:
        cmp     eax, 0x110000
        jae     .error
        cmp     eax, 0x10000
        jae     .write2
        cmp     eax, 0xe000
        jae     .write1
        cmp     eax, 0xd800
        jb      .write1
        jmp     .error
.write1:
        jmp     .done
.write2:
        sub     eax, 0x10000
        shl     eax, 6
        shr     ax, 6
        or      eax, 0xdc00d800
        jmp     .done
.done:
.error:
        ret
