;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


include "skindata.inc"

;skin_data = 0x00778000
;------------------------------------------------------------------------------
align 4
read_skin_file:
        stdcall load_file, ebx
        test    eax, eax
        jz      .notfound

        cmp     dword [eax], 'SKIN'
        jnz     .noskin

        xchg    eax, [skin_data]
        test    eax, eax
        jz      @f

        stdcall kernel_free, eax
@@:
        call    parse_skin_data
        xor     eax, eax
        ret
;--------------------------------------
align 4
.notfound:
        xor     eax, eax
        inc     eax
        ret
;--------------------------------------
align 4
.noskin:
        stdcall kernel_free, eax
        mov     eax, 2
        ret
;------------------------------------------------------------------------------
struct  SKIN_HEADER
        ident           dd ?
        version         dd ?
        params          dd ?
        buttons         dd ?
        bitmaps         dd ?
ends

struct  SKIN_PARAMS
        skin_height     dd ?
        margin.right    dw ?
        margin.left     dw ?
        margin.bottom   dw ?
        margin.top      dw ?
        colors.inner    dd ?
        colors.outer    dd ?
        colors.frame    dd ?
        colors_1.inner  dd ?
        colors_1.outer  dd ?
        colors_1.frame  dd ?
        dtp.size        dd ?
        dtp.data        rb 40
ends

struct  SKIN_BUTTONS
        type    dd ?
; position
        left    dw ?
        top     dw ?
; size
        width   dw ?
        height  dw ?
ends

struct  SKIN_BITMAPS
        kind    dw ?
        type    dw ?
        data    dd ?
ends
;------------------------------------------------------------------------------
align 4
load_default_skin:
        mov     [_skinh], 22
        mov     ebx, _skin_file_default
        call    read_skin_file
        ret
;------------------------------------------------------------------------------
align 4
parse_skin_data:
        mov     ebp, [skin_data]
        cmp     [ebp+SKIN_HEADER.ident], 'SKIN'
        jne     .exit

        mov     edi, skin_udata
        mov     ecx, (skin_udata.end-skin_udata)/4
        xor     eax, eax
        cld
        rep stosd

        mov     ebx, [ebp+SKIN_HEADER.params]
        add     ebx, [skin_data]
        mov     eax, [ebx+SKIN_PARAMS.skin_height]
        mov     [_skinh], eax
        mov     eax, [ebx+SKIN_PARAMS.colors.inner]
        mov     [skin_active.colors.inner], eax
        mov     eax, [ebx+SKIN_PARAMS.colors.outer]
        mov     [skin_active.colors.outer], eax
        mov     eax, [ebx+SKIN_PARAMS.colors.frame]
        mov     [skin_active.colors.frame], eax
        mov     eax, [ebx+SKIN_PARAMS.colors_1.inner]
        mov     [skin_inactive.colors.inner], eax
        mov     eax, [ebx+SKIN_PARAMS.colors_1.outer]
        mov     [skin_inactive.colors.outer], eax
        mov     eax, [ebx+SKIN_PARAMS.colors_1.frame]
        mov     [skin_inactive.colors.frame], eax
        lea     esi, [ebx+SKIN_PARAMS.dtp.data]
        mov     edi, common_colours
        mov     ecx, [ebx+SKIN_PARAMS.dtp.size]
        and     ecx, 127
        rep movsb
        mov     eax, dword[ebx+SKIN_PARAMS.margin.right]
        mov     dword[_skinmargins+0], eax
        mov     eax, dword[ebx+SKIN_PARAMS.margin.bottom]
        mov     dword[_skinmargins+4], eax

        mov     ebx, [ebp+SKIN_HEADER.bitmaps]
        add     ebx, [skin_data]
;--------------------------------------
align 4
.lp1:
        cmp     dword[ebx], 0
        je      .end_bitmaps
        movzx   eax, [ebx+SKIN_BITMAPS.kind]
        movzx   ecx, [ebx+SKIN_BITMAPS.type]
        dec     eax
        jnz     .not_left
        xor     eax, eax
        mov     edx, skin_active.left.data
        or      ecx, ecx
        jnz     @f
        mov     edx, skin_inactive.left.data
;--------------------------------------
align 4
@@:
        jmp     .next_bitmap
;--------------------------------------
align 4
.not_left:
        dec     eax
        jnz     .not_oper
        mov     esi, [ebx+SKIN_BITMAPS.data]
        add     esi, [skin_data]
        mov     eax, [esi+0]
        neg     eax
        mov     edx, skin_active.oper.data
        or      ecx, ecx
        jnz     @f
        mov     edx, skin_inactive.oper.data
;--------------------------------------
align 4
@@:
        jmp     .next_bitmap
;--------------------------------------
align 4
.not_oper:
        dec     eax
        jnz     .not_base
        mov     eax, [skin_active.left.width]
        mov     edx, skin_active.base.data
        or      ecx, ecx
        jnz     @f
        mov     eax, [skin_inactive.left.width]
        mov     edx, skin_inactive.base.data
;--------------------------------------
align 4
@@:
        jmp     .next_bitmap
;--------------------------------------
align 4
.not_base:
        add     ebx, 8
        jmp     .lp1
;--------------------------------------
align 4
.next_bitmap:
        mov     ecx, [ebx+SKIN_BITMAPS.data]
        add     ecx, [skin_data]
        mov     [edx+4], eax
        mov     eax, [ecx+0]
        mov     [edx+8], eax
        add     ecx, 8
        mov     [edx+0], ecx
        add     ebx, 8
        jmp     .lp1
;--------------------------------------
align 4
.end_bitmaps:
        mov     ebx, [ebp+SKIN_HEADER.buttons]
        add     ebx, [skin_data]
;--------------------------------------
align 4
.lp2:
        cmp     dword[ebx], 0
        je      .end_buttons
        mov     eax, [ebx+SKIN_BUTTONS.type]
        dec     eax
        jnz     .not_close
        mov     edx, skin_btn_close
        jmp     .next_button
;--------------------------------------
align 4
.not_close:
        dec     eax
        jnz     .not_minimize
        mov     edx, skin_btn_minimize
        jmp     .next_button
;--------------------------------------
align 4
.not_minimize:
        add     ebx, 12
        jmp     .lp2
;--------------------------------------
align 4
.next_button:
        movsx   eax, [ebx+SKIN_BUTTONS.left]
        mov     [edx+SKIN_BUTTON.left], eax
        movsx   eax, [ebx+SKIN_BUTTONS.top]
        mov     [edx+SKIN_BUTTON.top], eax
        movsx   eax, [ebx+SKIN_BUTTONS.width]
        mov     [edx+SKIN_BUTTON.width], eax
        movsx   eax, [ebx+SKIN_BUTTONS.height]
        mov     [edx+SKIN_BUTTON.height], eax
        add     ebx, 12
        jmp     .lp2
;--------------------------------------
align 4
.end_buttons:
.exit:
        ret
;------------------------------------------------------------------------------
align 4
drawwindow_IV_caption:

        mov     ebp, skin_active
        or      al, al
        jnz     @f
        mov     ebp, skin_inactive
;--------------------------------------
align 4
@@:
        mov     esi, [esp+4]
        mov     eax, [esi+WDATA.box.width]   ; window width
        mov     edx, [ebp+SKIN_DATA.left.left]
        shl     edx, 16
        mov     ecx, [ebp+SKIN_DATA.left.width]
        shl     ecx, 16
        add     ecx, [_skinh]

        mov     ebx, [ebp+SKIN_DATA.left.data]
        or      ebx, ebx
        jz      @f
        call    sys_putimage.forced
;--------------------------------------
align 4
@@:
        mov     esi, [esp+4]
        mov     eax, [esi+WDATA.box.width]
        sub     eax, [ebp+SKIN_DATA.left.width]
        sub     eax, [ebp+SKIN_DATA.oper.width]
        cmp     eax, [ebp+SKIN_DATA.base.left]
        jng     .non_base
        xor     edx, edx
        mov     ecx, [ebp+SKIN_DATA.base.width]
        jecxz   .non_base
        div     ecx

        inc     eax

        mov     ebx, [ebp+SKIN_DATA.base.data]
        mov     ecx, [ebp+SKIN_DATA.base.width]
        shl     ecx, 16
        add     ecx, [_skinh]
        mov     edx, [ebp+SKIN_DATA.base.left]
        sub     edx, [ebp+SKIN_DATA.base.width]
        shl     edx, 16
;--------------------------------------
align 4
.baseskinloop:
        shr     edx, 16
        add     edx, [ebp+SKIN_DATA.base.width]
        shl     edx, 16

        push    eax ebx ecx edx

        or      ebx, ebx
        jz      @f
        call    sys_putimage.forced
;--------------------------------------
align 4
@@:
        pop     edx ecx ebx eax

        dec     eax
        jnz     .baseskinloop
;--------------------------------------
align 4
.non_base:

        mov     esi, [esp+4]
        mov     edx, [esi+WDATA.box.width]
        sub     edx, [ebp+SKIN_DATA.oper.width]
        inc     edx
        shl     edx, 16
        mov     ebx, [ebp+SKIN_DATA.oper.data]

        mov     ecx, [ebp+SKIN_DATA.oper.width]
        shl     ecx, 16
        add     ecx, [_skinh]

        or      ebx, ebx
        jz      @f
        call    sys_putimage.forced
;--------------------------------------
align 4
@@:
        ret
;------------------------------------------------------------------------------
align 4
drawwindow_IV:
;param1 - aw_yes
        pusha

        push    edx

        mov     edi, edx

        mov     ebp, skin_active
        cmp     byte [esp+32+4+4], 0
        jne     @f
        mov     ebp, skin_inactive
;--------------------------------------
align 4
@@:
        mov     eax, [edi+WDATA.box.left]
        shl     eax, 16
        mov     ax, word [edi+WDATA.box.left]
        add     ax, word [edi+WDATA.box.width]
        mov     ebx, [edi+WDATA.box.top]
        shl     ebx, 16
        mov     bx, word [edi+WDATA.box.top]
        add     bx, word [edi+WDATA.box.height]
        mov     esi, [ebp+SKIN_DATA.colors.outer]
        or      esi, 1 shl 25 ; 0x02000000 used for draw_rectangle without top line
        ror     ebx, 16
        add     ebx, [_skinh]
        sub     bx, 1
        rol     ebx, 16
        call    draw_rectangle
        mov     ecx, 3
;--------------------------------------
align 4
_dw3l:
        add     eax, 1*65536-1
        add     ebx, 0*65536-1
        test    ax, ax
        js      no_skin_add_button
        test    bx, bx
        js      no_skin_add_button
        mov     esi, [ebp+SKIN_DATA.colors.frame];[edi+24]
        or      esi, 1 shl 25; 0x02000000 used for draw_rectangle without top line
        call    draw_rectangle
        dec     ecx
        jnz     _dw3l
        mov     esi, [ebp+SKIN_DATA.colors.inner]
        or      esi, 1 shl 25; 0x02000000 used for draw_rectangle without top line
        add     eax, 1*65536-1
        add     ebx, 0*65536-1
        test    ax, ax
        js      no_skin_add_button
        test    bx, bx
        js      no_skin_add_button
        test    [edi + WDATA.fl_wstate], WSTATE_ROLLEDUP
        jnz     @f
        call    draw_rectangle
;--------------------------------------
align 4
@@:
        mov     eax, [skin_data]
        cmp     [eax], dword 'SKIN'
        je      @f
        xor     eax, eax
        xor     ebx, ebx
        mov     esi, [esp]
        mov     ecx, [esi+WDATA.box.width]
        inc     ecx
        mov     edx, [_skinh]
        mov     edi, [common_colours+4]; standard grab color
;        call    [drawbar]
        call    vesa20_drawbar
        jmp     draw_clientbar
;--------------------------------------
align 4
@@:
        mov     al, [esp+32+4+4]
        call    drawwindow_IV_caption
;--------------------------------------
align 4
draw_clientbar:
        mov     esi, [esp]

        mov     edx, [esi+WDATA.box.top]                    ; WORK AREA
        add     edx, 21+5
        mov     ebx, [esi+WDATA.box.top]
        add     ebx, [esi+WDATA.box.height]
        cmp     edx, ebx
        jg      _noinside2
        mov     eax, 5
        mov     ebx, [_skinh]
        mov     ecx, [esi+WDATA.box.width]
        mov     edx, [esi+WDATA.box.height]
        sub     ecx, 4
        sub     edx, 4
        mov     edi, [esi+WDATA.cl_workarea]
        test    edi, 0x40000000
        jnz     _noinside2
;        call    [drawbar]
        call    vesa20_drawbar
;--------------------------------------
align 4
_noinside2:
        mov     eax, [skin_data]
        cmp     [eax], dword 'SKIN'
        jne     no_skin_add_button
;* close button
        mov     edi, [BTN_ADDR]
        movzx   eax, word [edi]
        cmp     eax, 1000
        jge     no_skin_add_button
        inc     eax
        mov     [edi], ax

        shl     eax, 4
        add     eax, edi

        mov     bx, [CURRENT_TASK]
        mov     [eax], bx

        add     eax, 2      ; save button id number
        mov     bx, 1
        mov     [eax], bx
        add     eax, 2      ; x start
        xor     ebx, ebx
        cmp     [skin_btn_close.left], 0
        jge     _bCx_at_right
        mov     ebx, [esp]
        mov     ebx, [ebx+WDATA.box.width]
        inc     ebx
;--------------------------------------
align 4
_bCx_at_right:
        add     ebx, [skin_btn_close.left]
        mov     [eax], bx
        add     eax, 2      ; x size
        mov     ebx, [skin_btn_close.width]
        dec     ebx
        mov     [eax], bx
        add     eax, 2      ; y start
        mov     ebx, [skin_btn_close.top]
        mov     [eax], bx
        add     eax, 2      ; y size
        mov     ebx, [skin_btn_close.height]
        dec     ebx
        mov     [eax], bx
;* minimize button
        mov     edi, [BTN_ADDR]
        movzx   eax, word [edi]
        cmp     eax, 1000
        jge     no_skin_add_button
        inc     eax
        mov     [edi], ax

        shl     eax, 4
        add     eax, edi

        mov     bx, [CURRENT_TASK]
        mov     [eax], bx

        add     eax, 2      ; save button id number
        mov     bx, 65535;999
        mov     [eax], bx
        add     eax, 2      ; x start
        xor     ebx, ebx
        cmp     [skin_btn_minimize.left], 0
        jge     _bMx_at_right
        mov     ebx, [esp]
        mov     ebx, [ebx+WDATA.box.width]
        inc     ebx
;--------------------------------------
align 4
_bMx_at_right:
        add     ebx, [skin_btn_minimize.left]
        mov     [eax], bx
        add     eax, 2      ; x size
        mov     ebx, [skin_btn_minimize.width]
        dec     ebx
        mov     [eax], bx
        add     eax, 2      ; y start
        mov     ebx, [skin_btn_minimize.top]
        mov     [eax], bx
        add     eax, 2      ; y size
        mov     ebx, [skin_btn_minimize.height]
        dec     ebx
        mov     [eax], bx
;--------------------------------------
align 4
no_skin_add_button:
        pop     edi
        popa
        ret     4
;------------------------------------------------------------------------------
