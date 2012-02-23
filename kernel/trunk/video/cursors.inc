;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


LOAD_FROM_FILE  equ 0
LOAD_FROM_MEM   equ 1
LOAD_INDIRECT   equ 2
LOAD_SYSTEM     equ 3

struct  BITMAPINFOHEADER
        Size                    dd ?
        Width                   dd ?
        Height                  dd ?
        Planes                  dw ?
        BitCount                dw ?
        Compression             dd ?
        SizeImage               dd ?
        XPelsPerMeter           dd ?
        YPelsPerMeter           dd ?
        ClrUsed                 dd ?
        ClrImportant            dd ?
ends

align 4
proc init_cursor stdcall, dst:dword, src:dword
           locals
             rBase    dd ?
             pQuad    dd ?
             pBits    dd ?
             pAnd     dd ?
             width    dd ?
             height   dd ?
             counter  dd ?
           endl

        mov     esi, [src]
        add     esi, [esi+18]
        mov     eax, esi

        cmp     [esi+BITMAPINFOHEADER.BitCount], 24
        je      .img_24
        cmp     [esi+BITMAPINFOHEADER.BitCount], 8
        je      .img_8
        cmp     [esi+BITMAPINFOHEADER.BitCount], 4
        je      .img_4

.img_2:
        add     eax, [esi]
        mov     [pQuad], eax
        add     eax, 8
        mov     [pBits], eax
        add     eax, 128
        mov     [pAnd], eax
        mov     eax, [esi+4]
        mov     [width], eax
        mov     ebx, [esi+8]
        shr     ebx, 1
        mov     [height], ebx

        mov     edi, [dst]
        add     edi, 32*31*4
        mov     [rBase], edi

        mov     esi, [pQuad]
.l21:
        mov     ebx, [pBits]
        mov     ebx, [ebx]
        bswap   ebx
        mov     eax, [pAnd]
        mov     eax, [eax]
        bswap   eax
        mov     [counter], 32
@@:
        xor     edx, edx
        shl     eax, 1
        setc    dl
        dec     edx

        xor     ecx, ecx
        shl     ebx, 1
        setc    cl
        mov     ecx, [esi+ecx*4]
        and     ecx, edx
        and     edx, 0xFF000000
        or      edx, ecx
        mov     [edi], edx

        add     edi, 4
        dec     [counter]
        jnz     @B

        add     [pBits], 4
        add     [pAnd], 4
        mov     edi, [rBase]
        sub     edi, 128
        mov     [rBase], edi
        sub     [height], 1
        jnz     .l21
        ret

.img_4:
        add     eax, [esi]
        mov     [pQuad], eax
        add     eax, 64
        mov     [pBits], eax
        add     eax, 0x200
        mov     [pAnd], eax
        mov     eax, [esi+4]
        mov     [width], eax
        mov     ebx, [esi+8]
        shr     ebx, 1
        mov     [height], ebx

        mov     edi, [dst]
        add     edi, 32*31*4
        mov     [rBase], edi

        mov     esi, [pQuad]
        mov     ebx, [pBits]
.l4:
        mov     eax, [pAnd]
        mov     eax, [eax]
        bswap   eax
        mov     [counter], 16
@@:
        xor     edx, edx
        shl     eax, 1
        setc    dl
        dec     edx

        movzx   ecx, byte [ebx]
        and     cl, 0xF0
        shr     ecx, 2
        mov     ecx, [esi+ecx]
        and     ecx, edx
        and     edx, 0xFF000000
        or      edx, ecx
        mov     [edi], edx

        xor     edx, edx
        shl     eax, 1
        setc    dl
        dec     edx

        movzx   ecx, byte [ebx]
        and     cl, 0x0F
        mov     ecx, [esi+ecx*4]
        and     ecx, edx
        and     edx, 0xFF000000
        or      edx, ecx
        mov     [edi+4], edx

        inc     ebx
        add     edi, 8
        dec     [counter]
        jnz     @B

        add     [pAnd], 4
        mov     edi, [rBase]
        sub     edi, 128
        mov     [rBase], edi
        sub     [height], 1
        jnz     .l4
        ret
.img_8:
        add     eax, [esi]
        mov     [pQuad], eax
        add     eax, 1024
        mov     [pBits], eax
        add     eax, 1024
        mov     [pAnd], eax
        mov     eax, [esi+4]
        mov     [width], eax
        mov     ebx, [esi+8]
        shr     ebx, 1
        mov     [height], ebx

        mov     edi, [dst]
        add     edi, 32*31*4
        mov     [rBase], edi

        mov     esi, [pQuad]
        mov     ebx, [pBits]
.l81:
        mov     eax, [pAnd]
        mov     eax, [eax]
        bswap   eax
        mov     [counter], 32
@@:
        xor     edx, edx
        shl     eax, 1
        setc    dl
        dec     edx

        movzx   ecx, byte [ebx]
        mov     ecx, [esi+ecx*4]
        and     ecx, edx
        and     edx, 0xFF000000
        or      edx, ecx
        mov     [edi], edx

        inc     ebx
        add     edi, 4
        dec     [counter]
        jnz     @B

        add     [pAnd], 4
        mov     edi, [rBase]
        sub     edi, 128
        mov     [rBase], edi
        sub     [height], 1
        jnz     .l81
        ret
.img_24:
        add     eax, [esi]
        mov     [pQuad], eax
        add     eax, 0xC00
        mov     [pAnd], eax
        mov     eax, [esi+BITMAPINFOHEADER.Width]
        mov     [width], eax
        mov     ebx, [esi+BITMAPINFOHEADER.Height]
        shr     ebx, 1
        mov     [height], ebx

        mov     edi, [dst]
        add     edi, 32*31*4
        mov     [rBase], edi

        mov     esi, [pAnd]
        mov     ebx, [pQuad]
.row_24:
        mov     eax, [esi]
        bswap   eax
        mov     [counter], 32
@@:
        xor     edx, edx
        shl     eax, 1
        setc    dl
        dec     edx

        mov     ecx, [ebx]
        and     ecx, 0x00FFFFFF
        and     ecx, edx
        and     edx, 0xFF000000
        or      edx, ecx
        mov     [edi], edx
        add     ebx, 3
        add     edi, 4
        dec     [counter]
        jnz     @B

        add     esi, 4
        mov     edi, [rBase]
        sub     edi, 128
        mov     [rBase], edi
        sub     [height], 1
        jnz     .row_24
        ret
endp

align 4
proc set_cursor stdcall, hcursor:dword
        mov     eax, [hcursor]
        cmp     [eax+CURSOR.magic], 'CURS'
        jne     .fail
;           cmp [eax+CURSOR.size], CURSOR_SIZE
;           jne .fail
        mov     ebx, [current_slot]
        xchg    eax, [ebx+APPDATA.cursor]
        ret
.fail:
        mov     eax, [def_cursor]
        mov     ebx, [current_slot]
        xchg    eax, [ebx+APPDATA.cursor]
        ret
endp

; param
;  eax= pid
;  ebx= src
;  ecx= flags

create_cursor:
.src     equ esp
.flags   equ esp+4
.hcursor equ esp+8

        sub     esp, 4         ;space for .hcursor
        push    ecx
        push    ebx

        mov     ebx, eax
        mov     eax, sizeof.CURSOR
        call    create_kernel_object
        test    eax, eax
        jz      .fail

        mov     [.hcursor], eax

        xor     ebx, ebx
        mov     [eax+CURSOR.magic], 'CURS'
        mov     [eax+CURSOR.destroy], destroy_cursor
        mov     [eax+CURSOR.hot_x], ebx
        mov     [eax+CURSOR.hot_y], ebx

        stdcall kernel_alloc, 0x1000
        test    eax, eax
        jz      .fail

        mov     edi, [.hcursor]
        mov     [edi+CURSOR.base], eax

        mov     esi, [.src]
        mov     ebx, [.flags]
        cmp     bx, LOAD_INDIRECT
        je      .indirect

        movzx   ecx, word [esi+10]
        movzx   edx, word [esi+12]
        mov     [edi+CURSOR.hot_x], ecx
        mov     [edi+CURSOR.hot_y], edx

        stdcall init_cursor, eax, esi

        mov     ecx, [.hcursor]
        lea     ecx, [ecx+CURSOR.list_next]
        lea     edx, [_display.cr_list.next]

        pushfd
        cli
        list_add ecx, edx   ;list_add_tail(new, head)
        popfd

        mov     eax, [.hcursor]
.check_hw:
        cmp     [_display.init_cursor], 0
        je      .fail

        push    eax
        call    [_display.init_cursor]
        add     esp, 4

        mov     eax, [.hcursor]
.fail:
        add     esp, 12
        ret
.indirect:
        shr     ebx, 16
        movzx   ecx, bh
        movzx   edx, bl
        mov     [eax+CURSOR.hot_x], ecx
        mov     [eax+CURSOR.hot_y], edx

        xchg    edi, eax
        mov     ecx, 1024
        cld
        rep movsd
        jmp     .check_hw

align 4
proc load_cursor stdcall, src:dword, flags:dword
           locals
             handle  dd ?
           endl

        xor     eax, eax
        cmp     [create_cursor], eax
        je      .fail2

        mov     [handle], eax
        cmp     word [flags], LOAD_FROM_FILE
        jne     @F

        stdcall load_file, [src]
        test    eax, eax
        jz      .fail
        mov     [src], eax
@@:
        push    ebx
        push    esi
        push    edi

        mov     eax, [CURRENT_TASK]
        shl     eax, 5
        mov     eax, [CURRENT_TASK+eax+4]
        mov     ebx, [src]
        mov     ecx, [flags]
        call    create_cursor    ;eax, ebx, ecx
        mov     [handle], eax

        cmp     word [flags], LOAD_FROM_FILE
        jne     .exit
        stdcall kernel_free, [src]
.exit:
        pop     edi
        pop     esi
        pop     ebx
.fail:
        mov     eax, [handle]
.fail2:
        ret
endp

align 4
proc delete_cursor stdcall, hcursor:dword
           locals
             hsrv       dd ?
             io_code    dd ?
             input      dd ?
             inp_size   dd ?
             output     dd ?
             out_size   dd ?
           endl

        mov     esi, [hcursor]
        cmp     [esi+CURSOR.magic], 'CURS'
        jne     .fail

        mov     ebx, [CURRENT_TASK]
        shl     ebx, 5
        mov     ebx, [CURRENT_TASK+ebx+4]
        cmp     ebx, [esi+CURSOR.pid]
        jne     .fail

        mov     ebx, [current_slot]
        cmp     esi, [ebx+APPDATA.cursor]
        jne     @F
        mov     eax, [def_cursor]
        mov     [ebx+APPDATA.cursor], eax
@@:
        mov     eax, [hcursor]
        call    [eax+APPOBJ.destroy]
.fail:
        ret
endp

; param
;  eax= cursor

align 4
destroy_cursor:

        push    eax
        stdcall kernel_free, [eax+CURSOR.base]

        mov     eax, [esp]
        lea     eax, [eax+CURSOR.list_next]

        pushfd
        cli
        list_del eax
        popfd

        pop     eax
        call    destroy_kernel_object
        ret

align 4
select_cursor:
        mov     eax, [esp+4]
        mov     [_display.cursor], eax
        ret     4

align 4
proc restore_24 stdcall, x:dword, y:dword

        push    ebx

        mov     ebx, [cur_saved_base]
        mov     edx, [cur.h]
        test    edx, edx
        jz      .ret

        push    esi
        push    edi

        mov     esi, cur_saved_data
        mov     ecx, [cur.w]
        lea     ecx, [ecx+ecx*2]
        push    ecx
@@:
        mov     edi, ebx
        add     ebx, [BytesPerScanLine]

        mov     ecx, [esp]
        rep movsb
        dec     edx
        jnz     @B

        pop     ecx
        pop     edi
        pop     esi
.ret:
        pop     ebx
        ret
endp

align 4
proc restore_32 stdcall, x:dword, y:dword

        push    ebx

        mov     ebx, [cur_saved_base]
        mov     edx, [cur.h]
        test    edx, edx
        jz      .ret

        push    esi
        push    edi

        mov     esi, cur_saved_data
@@:
        mov     edi, ebx
        add     ebx, [BytesPerScanLine]

        mov     ecx, [cur.w]
        rep movsd
        dec     edx
        jnz     @B

        pop     edi
.ret:
        pop     esi
        pop     ebx
        ret
endp

align 4
proc move_cursor_24 stdcall, hcursor:dword, x:dword, y:dword
           locals
             h      dd ?
             _dx     dd ?
             _dy     dd ?
           endl

        mov     esi, [hcursor]
        mov     ecx, [x]
        mov     eax, [y]
        mov     ebx, [BytesPerScanLine]

        xor     edx, edx
        sub     ecx, [esi+CURSOR.hot_x]
        lea     ebx, [ecx+32-1]
        mov     [x], ecx
        sets    dl
        dec     edx
        and     ecx, edx      ;clip x to 0<=x
        mov     [cur.left], ecx
        mov     edi, ecx
        sub     edi, [x]
        mov     [_dx], edi

        xor     edx, edx
        sub     eax, [esi+CURSOR.hot_y]
        lea     edi, [eax+32-1]
        mov     [y], eax
        sets    dl
        dec     edx
        and     eax, edx      ;clip y to 0<=y
        mov     [cur.top], eax
        mov     edx, eax
        sub     edx, [y]
        mov     [_dy], edx

        mul     dword [BytesPerScanLine]
        lea     edx, [LFB_BASE+ecx*3]
        add     edx, eax
        mov     [cur_saved_base], edx

        cmp     ebx, [Screen_Max_X]
        jbe     @F
        mov     ebx, [Screen_Max_X]
@@:
        cmp     edi, [Screen_Max_Y]
        jbe     @F
        mov     edi, [Screen_Max_Y]
@@:
        mov     [cur.right], ebx
        mov     [cur.bottom], edi

        sub     ebx, [x]
        sub     edi, [y]
        inc     ebx
        inc     edi

        mov     [cur.w], ebx
        mov     [cur.h], edi
        mov     [h], edi

        mov     eax, edi
        mov     edi, cur_saved_data
@@:
        mov     esi, edx
        add     edx, [BytesPerScanLine]
        mov     ecx, [cur.w]
        lea     ecx, [ecx+ecx*2]
        rep movsb
        dec     eax
        jnz     @B

;draw cursor
        mov     ebx, [cur_saved_base]
        mov     eax, [_dy]
        shl     eax, 5
        add     eax, [_dx]

        mov     esi, [hcursor]
        mov     esi, [esi+CURSOR.base]
        lea     edx, [esi+eax*4]
.row:
        mov     ecx, [cur.w]
        mov     esi, edx
        mov     edi, ebx
        add     edx, 32*4
        add     ebx, [BytesPerScanLine]
.pix:
        lodsd
        test    eax, 0xFF000000
        jz      @F
        mov     [edi], ax
        shr     eax, 16
        mov     [edi+2], al
@@:
        add     edi, 3
        dec     ecx
        jnz     .pix

        dec     [h]
        jnz     .row
        ret
endp


align 4
proc move_cursor_32 stdcall, hcursor:dword, x:dword, y:dword
           locals
             h      dd ?
             _dx     dd ?
             _dy     dd ?
           endl

        mov     esi, [hcursor]
        mov     ecx, [x]
        mov     eax, [y]

        xor     edx, edx
        sub     ecx, [esi+CURSOR.hot_x]
        lea     ebx, [ecx+32-1]
        mov     [x], ecx
        sets    dl
        dec     edx
        and     ecx, edx      ;clip x to 0<=x
        mov     [cur.left], ecx
        mov     edi, ecx
        sub     edi, [x]
        mov     [_dx], edi

        xor     edx, edx
        sub     eax, [esi+CURSOR.hot_y]
        lea     edi, [eax+32-1]
        mov     [y], eax
        sets    dl
        dec     edx
        and     eax, edx      ;clip y to 0<=y
        mov     [cur.top], eax
        mov     edx, eax
        sub     edx, [y]
        mov     [_dy], edx

        mul     dword [BytesPerScanLine]
        lea     edx, [LFB_BASE+eax+ecx*4]
        mov     [cur_saved_base], edx

        cmp     ebx, [Screen_Max_X]
        jbe     @F
        mov     ebx, [Screen_Max_X]
@@:
        cmp     edi, [Screen_Max_Y]
        jbe     @F
        mov     edi, [Screen_Max_Y]
@@:
        mov     [cur.right], ebx
        mov     [cur.bottom], edi

        sub     ebx, [x]
        sub     edi, [y]
        inc     ebx
        inc     edi

        mov     [cur.w], ebx
        mov     [cur.h], edi
        mov     [h], edi

        mov     eax, edi
        mov     edi, cur_saved_data
@@:
        mov     esi, edx
        add     edx, [BytesPerScanLine]
        mov     ecx, [cur.w]
        rep movsd
        dec     eax
        jnz     @B

;draw cursor
        mov     ebx, [cur_saved_base]
        mov     eax, [_dy]
        shl     eax, 5
        add     eax, [_dx]

        mov     esi, [hcursor]
        mov     esi, [esi+CURSOR.base]
        lea     edx, [esi+eax*4]
.row:
        mov     ecx, [cur.w]
        mov     esi, edx
        mov     edi, ebx
        add     edx, 32*4
        add     ebx, [BytesPerScanLine]
.pix:
        lodsd
        test    eax, 0xFF000000
        jz      @F
        mov     [edi], eax
@@:
        add     edi, 4
        dec     ecx
        jnz     .pix

        dec     [h]
        jnz     .row
        ret
endp


align 4
get_display:
        mov     eax, _display
        ret

align 4
init_display:

        xor     eax, eax
        mov     edi, _display

        mov     [edi+display_t.init_cursor], eax
        mov     [edi+display_t.select_cursor], eax
        mov     [edi+display_t.show_cursor], eax
        mov     [edi+display_t.move_cursor], eax
        mov     [edi+display_t.restore_cursor], eax

        lea     ecx, [edi+display_t.cr_list.next]
        mov     [edi+display_t.cr_list.next], ecx
        mov     [edi+display_t.cr_list.prev], ecx

        cmp     [SCR_MODE], word 0x13
        jbe     .fail

        test    word [SCR_MODE], 0x4000
        jz      .fail

        mov     ebx, restore_32
        mov     ecx, move_cursor_32
        movzx   eax, byte [ScreenBPP]
        cmp     eax, 32
        je      @F

        mov     ebx, restore_24
        mov     ecx, move_cursor_24
        cmp     eax, 24
        jne     .fail
@@:
        mov     [_display.select_cursor], select_cursor
        mov     [_display.move_cursor], ecx
        mov     [_display.restore_cursor], ebx

        stdcall load_cursor, def_arrow, dword LOAD_FROM_MEM
        mov     [def_cursor], eax
        ret
.fail:
        xor     eax, eax
        mov     [_display.select_cursor], eax
        mov     [_display.move_cursor], eax
        ret


align 4
def_arrow:
  file 'arrow.cur'

