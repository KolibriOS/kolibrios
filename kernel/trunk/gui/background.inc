;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2021. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; Background graphics implementation

uglobal
;  bgrchanged  dd  0x0
align 4
bgrlockpid dd 0
bgrlock db 0
endg
;--------------------------------------
align 4
sys_background:
        cmp     ebx, 1                     ; BACKGROUND SIZE
        jnz     .nosb1
        test    ecx, ecx
        jz      .sbgrr

        test    edx, edx
        jz      .sbgrr
;--------------------------------------
align 4
@@:
;;Maxis use atomic bts for mutexes  4.4.2009
        bts     dword [bgrlock], 0
        jnc     @f
        call    change_task
        jmp     @b
;--------------------------------------
align 4
@@:
        mov     [BgrDataWidth], ecx
        mov     [BgrDataHeight], edx
;    mov   [bgrchanged],1

        pushad
; return memory for old background
        mov     eax, [img_background]
        cmp     eax, static_background_data
        jz      @f
        stdcall kernel_free, eax
;--------------------------------------
align 4
@@:
; calculate RAW size
        xor     eax, eax
        inc     eax
        cmp     [BgrDataWidth], eax
        jae     @f
        mov     [BgrDataWidth], eax
;--------------------------------------
align 4
@@:
        cmp     [BgrDataHeight], eax
        jae     @f
        mov     [BgrDataHeight], eax
;--------------------------------------
align 4
@@:
        mov     eax, [BgrDataWidth]
        imul    eax, [BgrDataHeight]
        lea     eax, [eax*3]
; it is reserved with aligned to the boundary of 4 KB pages,
; otherwise there may be exceptions a page fault for vesa20_drawbackground_tiled
; because the 32 bit read is used for  high performance: "mov eax,[esi]"
        shr     eax, 12
        inc     eax
        shl     eax, 12
        mov     [mem_BACKGROUND], eax
; get memory for new background
        stdcall kernel_alloc, eax
        test    eax, eax
        jz      .memfailed
        mov     [img_background], eax
        jmp     .exit
;--------------------------------------
align 4
.memfailed:
; revert to static monotone data
        mov     [img_background], static_background_data
        xor     eax, eax
        inc     eax
        mov     [BgrDataWidth], eax
        mov     [BgrDataHeight], eax
        mov     [mem_BACKGROUND], 4
;--------------------------------------
align 4
.exit:
        popad
        mov     [bgrlock], 0
;--------------------------------------
align 4
.sbgrr:
        ret
;--------------------------------------
align 4
.nosb1:
        cmp     ebx, 2                     ; SET PIXEL
        jnz     .nosb2

        mov     eax, [img_background]
        test    ecx, ecx
        jz      @f
        cmp     eax, static_background_data
        jz      .ret
;--------------------------------------
align 4
@@:
        mov     ebx, [mem_BACKGROUND]
        add     ebx, PAGE_SIZE-1
        and     ebx, -PAGE_SIZE
        sub     ebx, 4
        cmp     ecx, ebx
        ja      .ret

        mov     ebx, [eax+ecx]
        and     ebx, 0xFF000000;255*256*256*256
        and     edx, 0x00FFFFFF;255*256*256+255*256+255
        add     edx, ebx
        mov     [eax+ecx], edx
;--------------------------------------
align 4
.ret:
        ret
;--------------------------------------
align 4
.nosb2:
        cmp     ebx, 3                     ; DRAW BACKGROUND
        jnz     nosb3
;--------------------------------------
align 4
draw_background_temp:
        mov     [background_defined], 1
        call    force_redraw_background
;--------------------------------------
align 4
nosb31:
        ret
;--------------------------------------
align 4
nosb3:
        cmp     ebx, 4                     ; TILED / STRETCHED
        jnz     nosb4
        cmp     ecx, [BgrDrawMode]
        je      nosb41
        mov     [BgrDrawMode], ecx
;--------------------------------------
align 4
nosb41:
        ret
;--------------------------------------
align 4
nosb4:
        cmp     ebx, 5                     ; BLOCK MOVE TO BGR
        jnz     nosb5

; add check pointer
        stdcall is_region_userspace, ecx, esi
        jnz     .fin

        cmp     [img_background], static_background_data
        jnz     @f
        test    edx, edx
        jnz     .fin
        cmp     esi, 4
        ja      .fin
;--------------------------------------
align 4
@@:
  ; bughere
        mov     eax, ecx
        mov     ebx, edx
        add     ebx, [img_background];IMG_BACKGROUND
        mov     ecx, esi
        call    memmove
;--------------------------------------
align 4
.fin:
        ret
;---------------------------------------
align 4
nosb5:
        cmp     ebx, 6
        jnz     nosb6
;--------------------------------------
align 4
;;Maxis use atomic bts for mutex 4.4.2009
@@:
        bts     dword [bgrlock], 0
        jnc     @f
        call    change_task
        jmp     @b
;--------------------------------------
align 4
@@:
        mov     eax, [current_slot_idx]
        mov     [bgrlockpid], eax
        cmp     [img_background], static_background_data
        jz      .nomem
        stdcall user_alloc, [mem_BACKGROUND]
        mov     [esp + SYSCALL_STACK.eax], eax
        test    eax, eax
        jz      .nomem
        mov     ebx, eax
        shr     ebx, 12
        or      dword [page_tabs+(ebx-1)*4], MEM_BLOCK_DONT_FREE
        mov     esi, [img_background]
        shr     esi, 12
        mov     ecx, [mem_BACKGROUND]
        add     ecx, 0xFFF
        shr     ecx, 12
;--------------------------------------
align 4
.z:
        mov     eax, [page_tabs+ebx*4]
        test    al, 1
        jz      @f
        call    free_page
;--------------------------------------
align 4
@@:
        mov     eax, [page_tabs+esi*4]
        or      al, PG_UWR
        mov     [page_tabs+ebx*4], eax
        mov     eax, ebx
        shl     eax, 12
        invlpg  [eax]
        inc     ebx
        inc     esi
        loop    .z
        ret
;--------------------------------------
align 4
.nomem:
        and     [bgrlockpid], 0
        mov     [bgrlock], 0
;--------------------------------------
align 4
nosb6:
        cmp     ebx, 7
        jnz     nosb7
        cmp     [bgrlock], 0
        jz      .err
        mov     eax, [current_slot_idx]
        cmp     [bgrlockpid], eax
        jnz     .err
        mov     eax, ecx
        mov     ebx, ecx
        shr     eax, 12
        mov     ecx, [page_tabs+(eax-1)*4]
        test    cl, MEM_BLOCK_USED or MEM_BLOCK_DONT_FREE
        jz      .err
        jnp     .err
        push    eax
        shr     ecx, 12
        dec     ecx
;--------------------------------------
align 4
@@:
        and     dword [page_tabs+eax*4], 0
        mov     edx, eax
        shl     edx, 12
        push    eax
        invlpg  [edx]
        pop     eax
        inc     eax
        loop    @b
        pop     eax
        and     dword [page_tabs+(eax-1)*4], not MEM_BLOCK_DONT_FREE
        stdcall user_free, ebx
        mov     [esp + SYSCALL_STACK.eax], eax
        and     [bgrlockpid], 0
        mov     [bgrlock], 0
        ret
;--------------------------------------
align 4
.err:
        and     dword [esp + SYSCALL_STACK.eax], 0
        ret
;-------------------------------------
align 4
nosb7:
        cmp     ebx, 8
        jnz     nosb8

        mov     ecx, [current_slot]
        xor     eax, eax
        xchg    eax, [ecx + APPDATA.draw_bgr_x]
        mov     [esp + SYSCALL_STACK.eax], eax ; eax = [left]*65536 + [right]
        xor     eax, eax
        xchg    eax, [ecx + APPDATA.draw_bgr_y]
        mov     [esp + SYSCALL_STACK.ebx], eax ; ebx = [top]*65536 + [bottom]
        ret
;---------------------------------------
align 4
nosb8:
        cmp     ebx, 9
        jnz     .exit
; ecx = [left]*65536 + [right]
; edx = [top]*65536 + [bottom]
        mov     eax, [_display.width]
        mov     ebx, [_display.height]
; check [right]
        cmp     cx, ax
        jae     .exit
; check [left]
        ror     ecx, 16
        cmp     cx, ax
        jae     .exit
; check [bottom]
        cmp     dx, bx
        jae     .exit
; check [top]
        ror     edx, 16
        cmp     dx, bx
        jae     .exit

        movzx   eax, cx  ; [left]
        movzx   ebx, dx  ; [top]

        shr     ecx, 16 ; [right]
        shr     edx, 16 ; [bottom]

        mov     [background_defined], 1

        mov     [draw_data + sizeof.RECT + RECT.left], eax
        mov     [draw_data + sizeof.RECT + RECT.top], ebx

        mov     [draw_data + sizeof.RECT + RECT.right], ecx
        mov     [draw_data + sizeof.RECT + RECT.bottom], edx

        inc     [REDRAW_BACKGROUND]
        call    wakeup_osloop
;--------------------------------------
align 4
.exit:
        ret
;------------------------------------------------------------------------------


align 4
uglobal
  BG_Rect_X_left_right  dd   0x0
  BG_Rect_Y_top_bottom  dd   0x0
endg
;------------------------------------------------------------------------------


align 4
force_redraw_background:
        and     [draw_data + sizeof.RECT + RECT.left], 0
        and     [draw_data + sizeof.RECT + RECT.top], 0
        push    eax ebx
        mov     eax, [_display.width]
        mov     ebx, [_display.height]
        dec     eax
        dec     ebx
        mov     [draw_data + sizeof.RECT + RECT.right], eax
        mov     [draw_data + sizeof.RECT + RECT.bottom], ebx
        pop     ebx eax
        inc     [REDRAW_BACKGROUND]
        call    wakeup_osloop
        ret
;------------------------------------------------------------------------------


align 4
sys_getbackground: ; sysfn 39
        dec     ebx
        jnz     .nogb1
        ; sysfn 39.1:
        mov     eax, [BgrDataWidth]
        shl     eax, 16
        mov     ax, word [BgrDataHeight]
        mov     [esp + SYSCALL_STACK.eax], eax
        ret
;---------------------------------------
align 4
.nogb1:
        dec     ebx
        jnz     .nogb2
        ; sysfn 39.2:
        mov     eax, [img_background]
        test    ecx, ecx
        jz      @f
        cmp     eax, static_background_data
        jz      .ret
align 4
@@:
        mov     ebx, [mem_BACKGROUND]
        add     ebx, PAGE_SIZE-1
        and     ebx, -PAGE_SIZE
        sub     ebx, 4
        cmp     ecx, ebx
        ja      .ret

        mov     eax, [ecx+eax]

        and     eax, 0xFFFFFF
        mov     [esp + SYSCALL_STACK.eax], eax
align 4
.ret:
        ret
;------------------------------------------------------------------------------
align 4
        .x      dd ?
        .y      dd ?
        .w      dd ?
        .h      dd ?
        .subrect_startptr dd ?
        .subrect_bytes dd ?
align 4
.nogb2:
        dec     ebx
        jnz     .nogb3
        ; sysfn 39.3 read background subrect to buffer
        ; ecx - <x, y>
        ; edx - <w, h>
        ; esi - buffer of 0x00RRGGBB
        mov     eax, [img_background]
        cmp     eax, static_background_data
        jz      .ret_39_3
align 4
@@:
        movzx   eax, cx ; store y in eax
        mov     [.y], eax

        shr     ecx, 16 ; ecx = x
        mov     [.x], ecx

        imul    eax, [BgrDataWidth]
        add     eax, ecx
        imul    eax, 3
        mov     [.subrect_startptr], eax

        movzx   eax, dx ; store h in eax
        mov     [.h], eax

        shr     edx, 16 ; edx = w
        mov     [.w], edx

        imul    eax, edx
        mov     [.subrect_bytes], eax

        ; check bounds
        mov     ebx, [mem_BACKGROUND]
        add     ebx, PAGE_SIZE-1
        and     ebx, -PAGE_SIZE
        sub     ebx, 4
        add     eax, [.subrect_startptr]
        cmp     eax, ebx
        ja      .fail_39_3

        ; copy contents
        mov     edi, [img_background]
        xor     ecx, ecx ; ecx - row index
.copy_rect:
        cmp     ecx, [.h]
        jae     .end_copy_rect
        
        xor     edx, edx ; edx - column index
.copy_row:
        cmp     edx, [.w]
        jae     .end_copy_row

        mov     ebx, ecx
        imul    ebx, [BgrDataWidth]
        add     ebx, edx
        imul    ebx, 3
        add     ebx, [.subrect_startptr]
        mov     eax, [edi + ebx]
        mov     ebx, ecx
        imul    ebx, [.w]
        add     ebx, edx
        and     eax, 0xFFFFFF
        mov     [esi + ebx*4], eax       

        inc     edx
        jmp     .copy_row
.end_copy_row:        

        inc     ecx
        jmp     .copy_rect
.end_copy_rect:
        xor     eax, eax
        mov     [esp + SYSCALL_STACK.eax], eax
;--------------------------------------
align 4
.fail_39_3:
        mov     eax, -1
align 4
.ret_39_3:
        ret
;--------------------------------------
align 4
.nogb3:
        dec     ebx
        jnz     .nogb4
        ; sysfn 39.4:
        mov     eax, [BgrDrawMode]
;--------------------------------------
align 4
.nogb4:
        mov     [esp + SYSCALL_STACK.eax], eax
        ret
;-----------------------------------------------------------------------------


align 4
syscall_putarea_backgr: ; sysfn 25
;eax = 25
;ebx = pointer to bufer for img BBGGRRBBGGRR...
;ecx = [size x]*65536 + [size y]
;edx = [start x]*65536 + [start y]
        pushad
        mov     edi, ebx
        mov     eax, edx
        shr     eax, 16
        mov     ebx, edx
        and     ebx, 0xffff
        dec     eax
        dec     ebx
; eax - x, ebx - y
        mov     edx, ecx
        shr     ecx, 16
        and     edx, 0xffff
        mov     esi, ecx
; ecx - size x, edx - size y
        mov     ebp, edx
        
        lea     ebp, [ebp*4]
        imul    ebp, esi
        stdcall is_region_userspace, edi, ebp
        jnz     .exit
 
        mov     ebp, edx

        dec     ebp
        shl     ebp, 2

        imul    ebp, esi

        mov     esi, ecx
        dec     esi
        shl     esi, 2

        add     ebp, esi
        add     ebp, edi

        add     ebx, edx
;--------------------------------------
align 4
.start_y:
        push    ecx edx
;--------------------------------------
align 4
.start_x:
        push    eax ecx
        add     eax, ecx

        mov     ecx, [ebp]
        rol     ecx, 8
        test    cl, cl        ; transparensy = 0
        jz      .no_put

        xor     cl, cl
        ror     ecx, 8

        pushad
        mov     edx, [d_width_calc_area + ebx*4]
        add     edx, [_display.win_map]
        movzx   edx, byte [eax+edx]
        cmp     dl, byte 1
        jne     @f

        call    dword [PUTPIXEL]; eax - x, ebx - y
;--------------------------------------
align 4
@@:
        popad
;--------------------------------------
align 4
.no_put:
        pop     ecx eax

        sub     ebp, 4
        dec     ecx
        jnz     .start_x

        pop     edx ecx
        dec     ebx
        dec     edx
        jnz     .start_y

.exit:
        popad
        ret



