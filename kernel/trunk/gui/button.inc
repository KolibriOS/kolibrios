;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

;==============================================================================
;///// public functions ///////////////////////////////////////////////////////
;==============================================================================

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

align 4
;------------------------------------------------------------------------------
syscall_button: ;///// system function 8 //////////////////////////////////////
;------------------------------------------------------------------------------
;? Define/undefine GUI button object
;------------------------------------------------------------------------------
;; Define button:
;> ebx = pack[16(x), 16(width)]
;> ecx = pack[16(y), 16(height)]
;> edx = pack[8(flags), 24(button identifier)]
;>       flags bits:
;>          7 (31) = 0
;>          6 (30) = don't draw button
;>          5 (29) = don't draw button frame when pressed
;> esi = button color
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;; Undefine button:
;> edx = pack[8(flags), 24(button identifier)]
;>       flags bits:
;>          7 (31) = 1
;------------------------------------------------------------------------------
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
        push    ebx ecx edx
        inc     eax
        mov     [edi], ax
        shl     eax, 4
        add     edi, eax
        ; NOTE: this code doesn't rely on SYS_BUTTON struct, please revise it
        ;       if you change something
        mov     ax, [CURRENT_TASK]
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
        pop     edx ecx ebx

        ; do we also need to draw the button?
        test    edx, 0x40000000
        jnz     .exit

        ; draw button body

        pushad

        ; calculate window-relative coordinates
        movzx   edi, cx
        shr     ebx, 16
        shr     ecx, 16
        mov     eax, [TASK_BASE]
        add     ebx, [eax - twdw + WDATA.box.left]
        add     ecx, [eax - twdw + WDATA.box.top]
        mov     eax, ebx
        shl     eax, 16
        mov     ax, bx
        add     ax, word[esp + 16]
        mov     ebx, ecx
        shl     ebx, 16
        mov     bx, cx

        ; calculate initial color
        mov     ecx, esi
        cmp     [buttontype], 0
        je      @f
        call    button._.incecx2

        ; set button height counter
    @@:
        mov     edx, edi

  .next_line:
        call    button._.button_dececx
        push    edi
        xor     edi, edi
;        call    [draw_line]
        call    __sys_draw_line
        pop     edi
        add     ebx, 0x00010001
        dec     edx
        jnz     .next_line

        popad

        ; draw button frame

        push    ebx ecx

        ; calculate window-relative coordinates
        shr     ebx, 16
        shr     ecx, 16
        mov     eax, [TASK_BASE]
        add     ebx, [eax - twdw + WDATA.box.left]
        add     ecx, [eax - twdw + WDATA.box.top]

        ; top border
        mov     eax, ebx
        shl     eax, 16
        mov     ax, bx
        add     ax, [esp + 4]
        mov     ebx, ecx
        shl     ebx, 16
        mov     bx, cx
        push    ebx
        xor     edi, edi
        mov     ecx, esi
        call    button._.incecx
;        call    [draw_line]
        call    __sys_draw_line

        ; bottom border
        movzx   edx, word[esp + 4 + 0]
        add     ebx, edx
        shl     edx, 16
        add     ebx, edx
        mov     ecx, esi
        call    button._.dececx
;        call    [draw_line]
        call    __sys_draw_line

        ; left border
        pop     ebx
        push    edx
        mov     edx, eax
        shr     edx, 16
        mov     ax, dx
        mov     edx, ebx
        shr     edx, 16
        mov     bx, dx
        add     bx, [esp + 4 + 0]
        pop     edx
        mov     ecx, esi
        call    button._.incecx
;        call    [draw_line]
        call    __sys_draw_line

        ; right border
        mov     dx, [esp + 4]
        add     ax, dx
        shl     edx, 16
        add     eax, edx
        add     ebx, 0x00010000
        mov     ecx, esi
        call    button._.dececx
;        call    [draw_line]
        call    __sys_draw_line

        pop     ecx ebx

  .exit:
        ret

; FIXME: mutex needed
syscall_button.remove_button:
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
        mov     ax, [CURRENT_TASK]
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

  .exit:
        ret

align 4
;------------------------------------------------------------------------------
sys_button_activate_handler: ;/////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> eax = pack[8(process slot), 24(button id)]
;> ebx = pack[16(button x coord), 16(button y coord)]
;> cl = mouse button mask this system button was pressed with
;------------------------------------------------------------------------------
        call    button._.find_button
        or      eax, eax
        jz      .exit

        mov     ebx, dword[eax + SYS_BUTTON.id_hi - 2]
        call    button._.negative_button

  .exit:
        ret

align 4
;------------------------------------------------------------------------------
sys_button_deactivate_handler: ;///////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> eax = pack[8(process slot), 24(button id)]
;> ebx = pack[16(button x coord), 16(button y coord)]
;> cl = mouse button mask this system button was pressed with
;------------------------------------------------------------------------------
        call    button._.find_button
        or      eax, eax
        jz      .exit

        mov     ebx, dword[eax + SYS_BUTTON.id_hi - 2]
        call    button._.negative_button

  .exit:
        ret

align 4
;------------------------------------------------------------------------------
sys_button_perform_handler: ;//////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
;> eax = pack[8(process slot), 24(button id)]
;> ebx = pack[16(button x coord), 16(button y coord)]
;> cl = mouse button mask this system button was pressed with
;------------------------------------------------------------------------------
        shl     eax, 8
        mov     al, cl
        movzx   ebx, byte[BTN_COUNT]
        mov     [BTN_BUFF + ebx * 4], eax
        inc     bl
        mov     [BTN_COUNT], bl
        ret

;==============================================================================
;///// private functions //////////////////////////////////////////////////////
;==============================================================================

;------------------------------------------------------------------------------
button._.find_button: ;////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Find system button by specified process slot, id and coordinates
;------------------------------------------------------------------------------
;> eax = pack[8(process slot), 24(button id)] or 0
;> ebx = pack[16(button x coord), 16(button y coord)]
;------------------------------------------------------------------------------
;< eax = pointer to SYS_BUTTON struct or 0
;------------------------------------------------------------------------------
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
        jz      .not_found

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

        ; okay, return it
        mov     eax, esi
        jmp     .exit

  .not_found:
        xor     eax, eax

  .exit:
        pop     edi esi edx ecx
        ret

;------------------------------------------------------------------------------
button._.dececx: ;/////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        sub     cl, 0x20
        jnc     @f
        xor     cl, cl
    @@:
        sub     ch, 0x20
        jnc     @f
        xor     ch, ch
    @@:
        rol     ecx, 16
        sub     cl, 0x20
        jnc     @f
        xor     cl, cl
    @@:
        rol     ecx, 16
        ret

;------------------------------------------------------------------------------
button._.incecx: ;/////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        add     cl, 0x20
        jnc     @f
        or      cl, -1
    @@:
        add     ch, 0x20
        jnc     @f
        or      ch, -1
    @@:
        rol     ecx, 16
        add     cl, 0x20
        jnc     @f
        or      cl, -1
    @@:
        rol     ecx, 16
        ret

;------------------------------------------------------------------------------
button._.incecx2: ;////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        add     cl, 0x14
        jnc     @f
        or      cl, -1
    @@:
        add     ch, 0x14
        jnc     @f
        or      ch, -1
    @@:
        rol     ecx, 16
        add     cl, 0x14
        jnc     @f
        or      cl, -1
    @@:
        rol     ecx, 16
        ret

;------------------------------------------------------------------------------
button._.button_dececx: ;//////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        cmp     [buttontype], 1
        jne     .finish

        push    eax
        mov     al, 1
        cmp     edi, 20
        jg      @f
        mov     al, 2

    @@:
        sub     cl, al
        jnc     @f
        xor     cl, cl
    @@:
        sub     ch, al
        jnc     @f
        xor     ch, ch
    @@:
        rol     ecx, 16
        sub     cl, al
        jnc     @f
        xor     cl, cl
    @@:
        rol     ecx, 16

        pop     eax

  .finish:
        ret

;------------------------------------------------------------------------------
button._.negative_button: ;////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? Invert system button border
;------------------------------------------------------------------------------
        ; if requested, do not display button border on press.
        test    ebx, 0x20000000
        jnz     .exit

        pushad

        xchg    esi, eax

        movzx   ecx, [esi + SYS_BUTTON.pslot]
        shl     ecx, 5
        add     ecx, window_data

        mov     eax, dword[esi + SYS_BUTTON.left]
        mov     ebx, dword[esi + SYS_BUTTON.top]
        add     eax, [ecx + WDATA.box.left]
        add     ebx, [ecx + WDATA.box.top]
        push    eax ebx
        pop     edx ecx
        rol     eax, 16
        rol     ebx, 16
        add     ax, cx
        add     bx, dx

        mov     esi, 0x01000000
        call    draw_rectangle.forced

        popad

  .exit:
        ret
