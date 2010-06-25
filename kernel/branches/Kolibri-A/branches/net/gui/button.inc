;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


button._.MAX_BUTTONS = 4095


;==============================================================================
;///// public functions ///////////////////////////////////////////////////////
;==============================================================================

struc SYS_BUTTON
{
  .pslot  dw ?
  .id_lo  dw ?
  .left   dw ?
  .width  dw ?
  .top    dw ?
  .height dw ?
  .id_hi  dw ?
  .sizeof:
}
virtual at 0
  SYS_BUTTON SYS_BUTTON
end virtual

iglobal
  mx                dw 0x0 ; keeps the x mouse's position when it was clicked
  my                dw 0x0 ; keeps the y mouse's position when it was clicked
  bPressedMouseXY_B db 0x0
  btn_down_determ   db 0x0
endg

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
        cmp     eax, button._.MAX_BUTTONS
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
    @@: mov     edx, edi

  .next_line:
        call    button._.button_dececx
        push    edi
        xor     edi, edi
        call    [draw_line]
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
        call    [draw_line]

        ; bottom border
        movzx   edx, word[esp + 4 + 0]
        add     ebx, edx
        shl     edx, 16
        add     ebx, edx
        mov     ecx, esi
        call    button._.dececx
        call    [draw_line]

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
        call    [draw_line]

        ; right border
        mov     dx, [esp + 4]
        add     ax, dx
        shl     edx, 16
        add     eax, edx
        add     ebx, 0x00010000
        mov     ecx, esi
        call    button._.dececx
        call    [draw_line]

        pop     ecx ebx

  .exit:
        ret

; FIXME: mutex needed
syscall_button.remove_button:
        and     edx, 0x00ffffff
        mov     edi, [BTN_ADDR]
        mov     ebx, [edi]
        inc     ebx
        imul    esi, ebx, SYS_BUTTON.sizeof
        add     esi, edi
        xor     ecx, ecx
        add     ecx, -SYS_BUTTON.sizeof

  .next_button:
        dec     ebx
        jz      .exit

        add     ecx, SYS_BUTTON.sizeof
        add     esi, -SYS_BUTTON.sizeof

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
        lea     eax, [esi + SYS_BUTTON.sizeof]
        call    memmove
        dec     dword[edi]
        add     ecx, -SYS_BUTTON.sizeof
        pop     ebx
        jmp     .next_button

  .exit:
        ret

align 4
;------------------------------------------------------------------------------
check_buttons: ;///////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        cmp     byte[BTN_DOWN], 0    ; mouse buttons pressed
        jnz     @f
        mov     [bPressedMouseXY_B], 0
        ret

    @@: pushad
        xor     esi, esi
        mov     edi, [BTN_ADDR]
        mov     edx, [edi]
        test    edx, edx
        jne     @f
        popad
        ret

        ;here i catch the coordinates when the mouse's button is clicked
    @@: push    ax
        cmp     [bPressedMouseXY_B], 0 ; FALSE
        jnz     @f
        mov     [bPressedMouseXY_B], 1 ; TRUE - it was already clicked
        mov     ax, [MOUSE_X]
        mov     [mx], ax
        mov     ax, [MOUSE_Y]
        mov     [my], ax
    @@: pop     ax
        ;and it is only refreshed after the mouse's button release

        push    esi
        inc     edx
        push    edx

  .buttonnewcheck:
        pop     edx
        pop     esi
        inc     esi
        cmp     edx, esi
        jge     .bch

        popad
        ret

  .bch:
        push    esi
        push    edx
        mov     eax, esi
        shl     eax, 4
        add     eax, edi

        ; check that button is at top of windowing stack
        movzx   ebx, [eax + SYS_BUTTON.pslot]
        movzx   ecx, word[WIN_STACK + ebx * 2]
        cmp     ecx, [TASK_COUNT]
        jne     .buttonnewcheck

        ; check that button start is inside window x/y end
        shl     ebx, 5

        test    [ebx + window_data + WDATA.fl_wstate], WSTATE_MINIMIZED
        jnz     .buttonnewcheck

        movzx   edx, [eax + SYS_BUTTON.left]
        cmp     edx, [window_data + ebx + WDATA.box.width] ;ecx
        jge     .buttonnewcheck

        movzx   edx, [eax + SYS_BUTTON.top]
        cmp     edx, [window_data + ebx + WDATA.box.height] ;ecx
        jge     .buttonnewcheck

        ; check coordinates

        ; mouse x >= button x ?
        add     ebx, window_data
        mov     ecx, [ebx + WDATA.box.left]
        movzx   edx, [eax + SYS_BUTTON.left]
        add     edx, ecx
        mov     cx, [mx]   ;mov cx,[MOUSE_X]
        cmp     edx, ecx
        jg      .buttonnewcheck

        movzx   ebx, [eax + SYS_BUTTON.width]
        add     edx, ebx
        cmp     ecx, edx
        jg      .buttonnewcheck

        ; mouse y >= button y ?
        movzx   ebx, [eax + SYS_BUTTON.pslot]
        shl     ebx, 5
        add     ebx, window_data
        mov     ecx, [ebx + WDATA.box.top]
        movzx   edx, [eax + SYS_BUTTON.top]
        add     edx, ecx
        mov     cx, [my]  ;mov cx,[MOUSE_Y]
        cmp     edx, ecx
        jg      .buttonnewcheck

        movzx   ebx, [eax + SYS_BUTTON.height]
        add     edx, ebx
        cmp     ecx, edx
        jg      .buttonnewcheck

        ; mouse on button

        pop     edx
        pop     esi

        mov     ebx, dword[eax + SYS_BUTTON.id_hi - 2]     ; button id : bits 16-31
        mov     bx, [eax + SYS_BUTTON.id_lo]       ; button id : bits 00-16
        push    ebx

        mov     byte[MOUSE_DOWN], 1  ; no mouse down checks
        call    button._.negative_button

        pushad
        push    eax
        mov     al, [BTN_DOWN]
        mov     byte[btn_down_determ], al
        pop     eax

  .cbwaitmouseup:
        call    checkidle
        call    [draw_pointer]

        pushad
        call    stack_handler
        popad

        cmp     byte[BTN_DOWN], 0  ; mouse buttons pressed ?
        jnz     .cbwaitmouseup
        popad

        call    button._.negative_button
        mov     byte[MOUSE_BACKGROUND], 0  ; no mouse background
        mov     byte[DONT_DRAW_MOUSE], 0  ; draw mouse

        ; check coordinates
        pusha

        ; mouse x >= button x ?
        movzx   ebx, [eax + SYS_BUTTON.pslot]
        shl     ebx, 5
        add     ebx, window_data
        mov     ecx, [ebx + WDATA.box.left]
        movzx   edx, [eax + SYS_BUTTON.left]
        add     edx, ecx
        mov     cx, [MOUSE_X]
        cmp     edx, ecx
        jg      .no_on_button ;if we release the pointer out of the button area

        movzx   ebx, [eax + SYS_BUTTON.width]
        add     edx, ebx
        cmp     ecx, edx
        jg      .no_on_button

        ; mouse y >= button y ?
        movzx   ebx, [eax + SYS_BUTTON.pslot]
        shl     ebx, 5
        add     ebx, window_data
        mov     ecx, [ebx + WDATA.box.top]
        movzx   edx, [eax + SYS_BUTTON.top]
        add     edx, ecx
        mov     cx, [MOUSE_Y]
        cmp     edx, ecx
        jg      .no_on_button

        movzx   ebx, [eax + SYS_BUTTON.height]
        add     edx, ebx
        cmp     ecx, edx
        jg      .no_on_button

        popa

        mov     byte[BTN_COUNT], 1 ; no of buttons in buffer
        pop     ebx
        mov     [BTN_BUFF], ebx   ; lets put the button id in buffer
        push    ebx
        pusha
        jmp     .yes_on_button

  .no_on_button:
        mov     byte[BTN_COUNT], 0 ; no of buttons in buffer

  .yes_on_button:
        mov     byte[MOUSE_DOWN], 0 ; mouse down -> do not draw
        popa
        pop     ebx
        popa
        ret

;==============================================================================
;///// private functions //////////////////////////////////////////////////////
;==============================================================================

;------------------------------------------------------------------------------
button._.dececx: ;/////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        sub     cl, 0x20
        jnc     @f
        xor     cl, cl
    @@: sub     ch, 0x20
        jnc     @f
        xor     ch, ch
    @@: rol     ecx, 16
        sub     cl, 0x20
        jnc     @f
        xor     cl, cl
    @@: rol     ecx, 16
        ret

;------------------------------------------------------------------------------
button._.incecx: ;/////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        add     cl, 0x20
        jnc     @f
        or      cl, -1
    @@: add     ch, 0x20
        jnc     @f
        or      ch, -1
    @@: rol     ecx, 16
        add     cl, 0x20
        jnc     @f
        or      cl, -1
    @@: rol     ecx, 16
        ret

;------------------------------------------------------------------------------
button._.incecx2: ;////////////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
;------------------------------------------------------------------------------
        add     cl, 0x14
        jnc     @f
        or      cl, -1
    @@: add     ch, 0x14
        jnc     @f
        or      ch, -1
    @@: rol     ecx, 16
        add     cl, 0x14
        jnc     @f
        or      cl, -1
    @@: rol     ecx, 16
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

    @@: sub     cl, al
        jnc     @f
        xor     cl, cl
    @@: sub     ch, al
        jnc     @f
        xor     ch, ch
    @@: rol     ecx, 16
        sub     cl, al
        jnc     @f
        xor     cl, cl
    @@: rol     ecx, 16

        pop     eax

  .finish:
        ret

;------------------------------------------------------------------------------
button._.negative_button: ;////////////////////////////////////////////////////
;------------------------------------------------------------------------------
;? <description>
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
