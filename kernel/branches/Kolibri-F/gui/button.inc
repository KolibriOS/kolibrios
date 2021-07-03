;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2016. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

button.MAX_BUTTONS = 4095

struct  SYS_BUTTON
        pslot           dw ?
        id_lo           dw ?
        left            dw ?
        width           dw ?
        top             dw ?
        height          dw ?
        id_hi           dw ?
                        dw ?
ends

;---------------------------------------------------------------
syscall_button: ;////////////// system function 8 //////////////
;---------------------------------------------------------------
;? Define/undefine GUI button object
;---------------------------------------------------------------
;; Define button:
;> ebx = pack[16(x), 16(width)]
;> ecx = pack[16(y), 16(height)]
;> edx = pack[8(flags), 24(button identifier)]
;>       flags bits:
;>          7 (31) = 0
;>          6 (30) = don't draw button
;>          5 (29) = don't draw button frame when pressed
;> esi = button color
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
;; Undefine button:
;> edx = pack[8(flags), 24(button identifier)]
;>       flags bits:
;>          7 (31) = 1
;---------------------------------------------------------------
        ; do we actually need to undefine the button?
        test    edx, 0x80000000
        jnz     .remove_button

        ; do we have free button slots available?
        mov     edi, [BTN_ADDR]
        mov     eax, [edi]
        cmp     eax, button.MAX_BUTTONS
        jge     .exit

        ; does it have positive size? (otherwise it doesn't have sense)
        or      bx, bx
        jle     .exit
        or      cx, cx
        jle     .exit

        ; make coordinates clientbox-relative
        push    eax
        mov     eax, [current_slot]
        rol     ebx, 16
        add     bx, word[eax + APPDATA.wnd_clientbox.left]
        rol     ebx, 16
        rol     ecx, 16
        add     cx, word[eax + APPDATA.wnd_clientbox.top]
        rol     ecx, 16
        pop     eax

        ; basic checks passed, define the button
        inc     eax
        mov     [edi], ax
        shl     eax, 4
        add     edi, eax
; NOTE: this code doesn't rely on SYS_BUTTON struct,
; please revise it, if you change something.
        mov     ax, word [current_slot_idx]
        stosw
        mov     ax, dx
        stosw               ; button id number: bits 0-15
        mov     eax, ebx
        rol     eax, 16
        stosd               ; x start | x size
        mov     eax, ecx
        rol     eax, 16
        stosd               ; y start | y size
        mov     eax, edx
        shr     eax, 16
        stosw               ; button id number: bits 16-31

        ; do we also need to draw the button?
        test    edx, 0x40000000
        jnz     .exit

        and     esi, 0xFFFFFF
        xor     edi, edi
        push    ebx ecx esi
        dec     cx
        dec     cx
        cmp     [buttontype], 1
        jnz     .draw
        cmp     cx, 65
        jnc     .draw

        ; calculate gradient data
        mov     eax, esi
        shl     eax, 8
        mov     edx, 3
.calculate:
        rol     eax, 8
        shl     al, 1
        jnc     @f
        neg     al
        jnz     @f
        mov     al, 64
@@:
        cmp     al, 65
        jc      @f
        mov     al, 64
@@:
        div     cl
        shl     ax, 8
        dec     edx
        jnz     .calculate
        mov     dl, cl
        dec     edx
        shr     edx, 1
        shr     eax, 8
        mov     edi, eax
        mul     edx
        add     esi, eax

.draw:  ; calculate window-relative coordinates
        movzx   ebp, cx
        dec     ebp
        shr     ebx, 16
        shr     ecx, 16
        mov     eax, [TASK_BASE]
        add     ebx, [eax - twdw + WDATA.box.left]
        add     ecx, [eax - twdw + WDATA.box.top]
        mov     eax, ebx
        inc     eax
        mov     edx, ebx
        add     dx, [esp+8]
        dec     edx
        mov     ebx, ecx
        mov     ecx, esi
        shr     ecx, 1
        and     cx, 7F7Fh
        push    esi
        mov     esi, edi
        xor     edi, edi
        call    hline   ; top border
        inc     ebx
        or      ecx, 808080h
        call    hline   ; top light line
        pop     ecx
        inc     ebx
.next_line:
        call    hline   ; button body
        inc     ebx
        sub     ecx, esi
        dec     ebp
        jnz     .next_line
        shr     ecx, 2
        and     cx, 3F3Fh
        mov     ebp, ecx
        shl     ecx, 1
        add     ecx, ebp
        call    hline   ; bottom dark line
        inc     ebx
        sub     ecx, ebp
        call    hline   ; bottom border
        pop     ecx
        shr     ecx, 1
        inc     edx
        push    edx
        mov     edx, ebx
        sub     bx, [esp+4]
        dec     edx
        inc     ebx
        cmp     [buttontype], 0
        jnz     @f
        dec     edx
        or      ecx, 808080h
        call    vline   ; left light line
        inc     edx
@@:
        and     ecx, 7F7F7Fh
        dec     eax
        call    vline   ; left border
        pop     eax
        call    vline   ; right border
        cmp     [buttontype], 0
        jnz     @f
        mov     ebp, ecx
        shr     ecx, 1
        and     cx, 7F7Fh
        add     ecx, ebp
        dec     eax
        inc     ebx
        dec     edx     ; avoid lines overflow
        call    vline   ; right dark line
@@:
        pop     ecx ebx
.exit:
        ret

; FIXME: mutex needed
.remove_button:
        and     edx, 0x00ffffff
        mov     edi, [BTN_ADDR]
        mov     ebx, [edi]
        inc     ebx
        imul    esi, ebx, sizeof.SYS_BUTTON
        add     esi, edi
        xor     ecx, ecx
        add     ecx, -sizeof.SYS_BUTTON
        add     esi, sizeof.SYS_BUTTON

.next_button:
        dec     ebx
        jz      .exit

        add     ecx, sizeof.SYS_BUTTON
        add     esi, -sizeof.SYS_BUTTON

        ; does it belong to our process?
        mov     ax, word [current_slot_idx]
        cmp     ax, [esi + SYS_BUTTON.pslot]
        jne     .next_button

        ; does the identifier match?
        mov     eax, dword[esi + SYS_BUTTON.id_hi - 2]
        mov     ax, [esi + SYS_BUTTON.id_lo]
        and     eax, 0x00ffffff
        cmp     edx, eax
        jne     .next_button

        ; okay, undefine it
        push    ebx
        mov     ebx, esi
        lea     eax, [esi + sizeof.SYS_BUTTON]
        call    memmove
        dec     dword[edi]
        add     ecx, -sizeof.SYS_BUTTON
        pop     ebx
        jmp     .next_button

;---------------------------------------------------------------
sys_button_activate_handler:
sys_button_deactivate_handler:
;---------------------------------------------------------------
;> eax = pack[8(process slot), 24(button id)]
;> ebx = pack[16(button x coord), 16(button y coord)]
;> cl = mouse button mask this system button was pressed with
;---------------------------------------------------------------
; find system button by specified process slot, id and coordinates
        push    ecx edx esi edi
        mov     edx, eax
        shr     edx, 24
        and     eax, 0x0ffffff
        mov     edi, [BTN_ADDR]
        mov     ecx, [edi]
        imul    esi, ecx, sizeof.SYS_BUTTON
        add     esi, edi
        inc     ecx
        add     esi, sizeof.SYS_BUTTON
.next_button:
        dec     ecx
        jz      .popexit
        add     esi, -sizeof.SYS_BUTTON

        ; does it belong to our process?
        cmp     dx, [esi + SYS_BUTTON.pslot]
        jne     .next_button

        ; does id match?
        mov     edi, dword[esi + SYS_BUTTON.id_hi - 2]
        mov     di, [esi + SYS_BUTTON.id_lo]
        and     edi, 0x0ffffff
        cmp     eax, edi
        jne     .next_button

        ; does coordinates match?
        mov     edi, dword[esi + SYS_BUTTON.left - 2]
        mov     di, [esi + SYS_BUTTON.top]
        cmp     ebx, edi
        jne     .next_button

        mov     eax, esi
        pop     edi esi edx ecx
        mov     ebx, dword[eax + SYS_BUTTON.id_hi - 2]

        ; display button border on press?
        bt      ebx, 29
        jc      .exit

        ; invert system button border
        pushad
        mov     esi, eax
        mov     edi, ebx
        movzx   ecx, [esi + SYS_BUTTON.pslot]
        shl     ecx, 5
        add     ecx, window_data
        mov     eax, dword[esi + SYS_BUTTON.left]
        mov     ebx, dword[esi + SYS_BUTTON.top]
        add     eax, [ecx + WDATA.box.left]
        add     ebx, [ecx + WDATA.box.top]
        mov     ecx, eax
        mov     edx, ebx
        bt      edi, 30
        jc      @f
        inc     ax
        inc     bx
        dec     cx
        dec     dx
@@:
        rol     eax, 16
        rol     ebx, 16
        add     ax, cx
        add     bx, dx
        mov     esi, 1000000h
        call    draw_rectangle.forced
        popad
.exit:
        ret
.popexit:
        pop     edi esi edx ecx
        ret

;---------------------------------------------------------------
sys_button_perform_handler:
;---------------------------------------------------------------
;> eax = pack[8(process slot), 24(button id)]
;> ebx = pack[16(button x coord), 16(button y coord)]
;> cl = mouse button mask this system button was pressed with
;---------------------------------------------------------------
        shl     eax, 8
        mov     al, cl
        movzx   ebx, byte[BTN_COUNT]
        mov     [BTN_BUFF + ebx * 4], eax
        inc     bl
        mov     [BTN_COUNT], bl
        ret
