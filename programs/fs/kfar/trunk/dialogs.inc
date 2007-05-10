; int __stdcall GenericBox(DLGTEMPLATE* dlg, void* DlgProc);
;       int __stdcall DlgProc(DLGTEMPLATE* dlg, int msg, int param1, int param2);

virtual at 0
dlgtemplate:
; Флаги:
; бит 0: использовать стандартные цвета диалога
; бит 1: использовать стандартные цвета предупреждения/ошибки
; (если любой из этих битов установлен, поля main_color,border_color,header_color
;  игнорируются)
.flags          dd      ?
.x              dd      ?
.y              dd      ?
.width          dd      ?
.height         dd      ?
.border_size_x  dd      ?
.border_size_y  dd      ?
.title          dd      ?
.main_color     db      ?
.border_color   db      ?
.header_color   db      ?
                db      ?       ; align
.size = $
end virtual

GenericBox:
        pushad
        mov     ebx, [esp+20h+4]
; center window if required
        push    [ebx+dlgtemplate.x]
        push    [ebx+dlgtemplate.y]
        cmp     [ebx+dlgtemplate.x], -1
        jnz     @f
        mov     eax, [cur_width]
        sub     eax, [ebx+dlgtemplate.width]
        shr     eax, 1
        mov     [ebx+dlgtemplate.x], eax
@@:
        cmp     [ebx+dlgtemplate.y], -1
        jnz     @f
        mov     eax, [cur_height]
        sub     eax, [ebx+dlgtemplate.height]
        shr     eax, 1
        mov     [ebx+dlgtemplate.y], eax
@@:
; some checks
        mov     eax, [ebx+dlgtemplate.x]
        cmp     eax, 1
        jl      .sizeerr
        add     eax, [ebx+dlgtemplate.width]
        cmp     eax, [cur_width]
        jge     .sizeerr
        mov     eax, [ebx+dlgtemplate.y]
        cmp     eax, 1
        jl      .sizeerr
        add     eax, [ebx+dlgtemplate.height]
        cmp     eax, [cur_height]
        jge     .sizeerr
        cmp     [ebx+dlgtemplate.border_size_x], 1
        jl      .sizeerr
        cmp     [ebx+dlgtemplate.border_size_y], 1
        jge     .sizeok
.sizeerr:
        pop     [ebx+dlgtemplate.y]
        pop     [ebx+dlgtemplate.x]
        popad
        or      eax, -1
        ret     8
.sizeok:
; set color if required
        test    byte [ebx+dlgtemplate.flags], 1
        jz      @f
        mov     edi, dialog_colors
        jmp     .setcolor
@@:
        test    byte [ebx+dlgtemplate.flags], 2
        jz      @f
        mov     edi, warning_colors
.setcolor:
        mov     al, [edi + dialog_main_color-dialog_colors]
        mov     [ebx+dlgtemplate.main_color], al
        mov     al, [edi + dialog_border_color-dialog_colors]
        mov     [ebx+dlgtemplate.border_color], al
        mov     al, [edi + dialog_header_color-dialog_colors]
        mov     [ebx+dlgtemplate.header_color], al
@@:
; allocate memory for data under dialog
; for 'No memory' dialog use static data area
        mov     ebp, nomem_dlgsavearea
        cmp     ebx, nomem_dlgdata
        jz      .allocated
        mov     eax, [ebx+dlgtemplate.width]
        add     eax, [ebx+dlgtemplate.border_size_x]
        add     eax, [ebx+dlgtemplate.border_size_x]
        inc     eax
        inc     eax
        mov     edx, [ebx+dlgtemplate.height]
        add     edx, [ebx+dlgtemplate.border_size_y]
        add     edx, [ebx+dlgtemplate.border_size_y]
        inc     edx
        mul     edx
        lea     ecx, [eax*2]
        call    xpgalloc
        test    eax, eax
        jnz     @f
        pop     [ebx+dlgtemplate.y]
        pop     [ebx+dlgtemplate.x]
        popad
        or      eax, -1
        ret     8
@@:
        mov     ebp, eax
.allocated:
; save data
        mov     eax, [ebx+dlgtemplate.y]
        add     eax, [ebx+dlgtemplate.height]
        add     eax, [ebx+dlgtemplate.border_size_y]
        inc     eax
        push    eax
        mov     eax, [ebx+dlgtemplate.x]
        add     eax, [ebx+dlgtemplate.width]
        add     eax, [ebx+dlgtemplate.border_size_x]
        inc     eax
        inc     eax
        push    eax
        mov     eax, [ebx+dlgtemplate.y]
        sub     eax, [ebx+dlgtemplate.border_size_y]
        push    eax
        mov     eax, [ebx+dlgtemplate.x]
        sub     eax, [ebx+dlgtemplate.border_size_x]
        push    eax
        call    save_console_data
; draw shadow
        mov     eax, [ebx+dlgtemplate.x]
        sub     eax, [ebx+dlgtemplate.border_size_x]
        ja      @f
        xor     eax, eax
@@:
        push    eax     ; save real window left
        inc     eax
        inc     eax
        mov     edx, [ebx+dlgtemplate.y]
        sub     edx, [ebx+dlgtemplate.border_size_y]
        ja      @f
        xor     edx, edx
@@:
        push    edx     ; save real window top
        inc     edx
        call    get_console_ptr
        mov     ecx, [ebx+dlgtemplate.y]
        add     ecx, [ebx+dlgtemplate.height]
        add     ecx, [ebx+dlgtemplate.border_size_y]
        inc     ecx
        cmp     ecx, [cur_height]
        jb      @f
        mov     ecx, [cur_height]
@@:
        sub     ecx, edx
        mov     edx, ecx
        mov     ecx, [ebx+dlgtemplate.x]
        add     ecx, [ebx+dlgtemplate.width]
        add     ecx, [ebx+dlgtemplate.border_size_x]
        inc     ecx
        inc     ecx
        cmp     ecx, [cur_width]
        jb      @f
        mov     ecx, [cur_width]
@@:
        sub     ecx, eax
        mov     eax, ecx
.shadow_loop:
        mov     ecx, eax
        push    edi
.sl1:
        inc     edi
        test    byte [edi], 0x0F
        jnz     @f
        or      byte [edi], 8
@@:
        and     byte [edi], 0x0F
        inc     edi
        loop    .sl1
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        dec     edx
        jnz     .shadow_loop
; draw area background
        pop     edx
        pop     eax
        call    get_console_ptr
        mov     ecx, [ebx+dlgtemplate.x]
        add     ecx, [ebx+dlgtemplate.width]
        add     ecx, [ebx+dlgtemplate.border_size_x]
        cmp     ecx, [cur_width]
        jb      @f
        mov     ecx, [cur_width]
@@:
        sub     ecx, eax
        mov     esi, ecx
        mov     ecx, [ebx+dlgtemplate.y]
        add     ecx, [ebx+dlgtemplate.height]
        add     ecx, [ebx+dlgtemplate.border_size_y]
        cmp     ecx, [cur_height]
        jb      @f
        mov     ecx, [cur_height]
@@:
        sub     ecx, edx
        mov     edx, ecx
        mov     al, ' '
        mov     ah, [ebx+dlgtemplate.border_color]
.1:
        mov     ecx, esi
        push    edi
        rep     stosw
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        dec     edx
        jnz     .1
; draw border
        mov     eax, [ebx+dlgtemplate.x]
        dec     eax
        mov     edx, [ebx+dlgtemplate.y]
        dec     edx
        call    get_console_ptr
        mov     edx, [ebx+dlgtemplate.height]
        inc     edx
        inc     edx
        mov     ah, [ebx+dlgtemplate.border_color]
        push    ebx
        mov     ebx, [ebx+dlgtemplate.width]
        inc     ebx
        inc     ebx
        call    draw_border
        pop     ebx
; draw header
        mov     esi, [ebx+dlgtemplate.title]
        test    esi, esi
        jz      .noheader
        cmp     byte [esi], 0
        jz      .noheader
        push    esi
@@:     lodsb
        test    al, al
        jnz     @b
        mov     eax, esi
        pop     esi
        sub     eax, esi
        inc     eax     ; eax = длина заголовка + 2
        mov     ecx, [ebx+dlgtemplate.width]
        cmp     eax, ecx
        jbe     .fullhea
        sub     ecx, 5
        jb      .noheader
        xor     edx, edx
        jmp     .drawhea
.fullhea:
        mov     edx, ecx
        sub     edx, eax
        shr     edx, 1
.drawhea:
        mov     eax, [ebx+dlgtemplate.x]
        add     eax, edx
        mov     edx, [ebx+dlgtemplate.y]
        dec     edx
        call    get_console_ptr
        mov     ah, [ebx+dlgtemplate.header_color]
        mov     al, ' '
        stosw
.2:
        dec     ecx
        jz	.3
        lodsb
        test    al, al
        jz      .4
        stosw
        jmp     .2
.3:
        mov     al, '.'
        stosw
        stosw
        stosw
.4:
        mov     al, ' '
        stosw
.noheader:
; draw window background
        mov     eax, [ebx+dlgtemplate.x]
        mov     edx, [ebx+dlgtemplate.y]
        call    get_console_ptr
        mov     ah, [ebx+dlgtemplate.main_color]
        mov     al, ' '
        mov     edx, [ebx+dlgtemplate.height]
@@:
        mov     ecx, [ebx+dlgtemplate.width]
        push    edi
        rep     stosw
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        dec     edx
        jnz     @b
; send redraw message
        mov     eax, [esp+28h+8]
        push    ebx ebp
        push    0
        push    0
        push    1
        push    ebx
        call    eax
        call    draw_image
        pop     ebp ebx
; message loop
.event:
        push    10
        pop     eax
        mcall
        dec     eax
        jz      .redraw
        dec     eax
        jz      .key
        or      eax, -1
        mcall
.redraw:
        push    ebx ebp
        call    draw_window
        pop     ebp ebx
        jmp     .event
.key:
        mov     al, 2
        mcall
        shr     eax, 8
        cmp     al, 0xE0
        jnz     @f
        mov     [bWasE0], 1
        jmp     .event
@@:
        xchg    ah, [bWasE0]
        cmp     al, 0x1D
        jz      .ctrl_down
        cmp     al, 0x9D
        jz      .ctrl_up
        cmp     al, 0x2A
        jz      .lshift_down
        cmp     al, 0xAA
        jz      .lshift_up
        cmp     al, 0x36
        jz      .rshift_down
        cmp     al, 0xB6
        jz      .rshift_up
        cmp     al, 0x38
        jz      .alt_down
        cmp     al, 0xB8
        jz      .alt_up
        mov     ecx, [esp+28h+8]
        push    ebx ebp
        push    0
        push    eax
        push    2
        push    ebx
        call    ecx
        pop     ebp ebx
        test    eax, eax
        jz      .event
        mov     [esp+8+28], eax
        jmp     .exit
.ctrl_down:
        test    ah, ah
        jnz     .rctrl_down
        or      [ctrlstate], 4
        jmp     .event
.rctrl_down:
        or      [ctrlstate], 8
        jmp     .event
.ctrl_up:
        test    ah, ah
        jnz     .rctrl_up
        and     [ctrlstate], not 4
        jmp     .event
.rctrl_up:
        and     [ctrlstate], not 8
        jmp     .event
.lshift_down:
        test    ah, ah
        jnz     @f
        or      [ctrlstate], 1
@@:     jmp     .event
.lshift_up:
        test    ah, ah
        jnz     @b
        and     [ctrlstate], not 1
        jmp     @b
.rshift_down:
        or      [ctrlstate], 2
        jmp     .event
.rshift_up:
        and     [ctrlstate], not 2
        jmp     .event
.alt_down:
        test    ah, ah
        jnz     .ralt_down
        or      [ctrlstate], 0x10
        jmp     .event
.ralt_down:
        or      [ctrlstate], 0x20
        jmp     .event
.alt_up:
        test    ah, ah
        jnz     .ralt_up
        and     [ctrlstate], not 0x10
        jmp     .event
.ralt_up:
        and     [ctrlstate], not 0x20
        jmp     .event
.exit:
; restore data
        mov     eax, [ebx+dlgtemplate.y]
        add     eax, [ebx+dlgtemplate.height]
        add     eax, [ebx+dlgtemplate.border_size_y]
        inc     eax
        push    eax
        mov     eax, [ebx+dlgtemplate.x]
        add     eax, [ebx+dlgtemplate.width]
        add     eax, [ebx+dlgtemplate.border_size_x]
        inc     eax
        inc     eax
        push    eax
        mov     eax, [ebx+dlgtemplate.y]
        sub     eax, [ebx+dlgtemplate.border_size_y]
        push    eax
        mov     eax, [ebx+dlgtemplate.x]
        sub     eax, [ebx+dlgtemplate.border_size_x]
        push    eax
        call    restore_console_data
        call    draw_keybar
        cmp     ebx, nomem_dlgdata
        jz      @f
        mov     ecx, ebp
        call    pgfree
@@:
        pop     [ebx+dlgtemplate.y]
        pop     [ebx+dlgtemplate.x]
        or      [cursor_x], -1
        or      [cursor_y], -1
        call    draw_image
        popad
        ret     8

save_console_data:
        cmp     dword [esp+4], 0
        jge     @f
        and     dword [esp+4], 0
@@:
        cmp     dword [esp+8], 0
        jge     @f
        and     dword [esp+8], 0
@@:
        mov     eax, [esp+12]
        cmp     eax, [cur_width]
        jbe     @f
        mov     eax, [cur_width]
@@:
        sub     eax, [esp+4]
        ja      @f
        ret     16
@@:
        mov     [esp+12], eax
        mov     eax, [esp+16]
        cmp     eax, [cur_height]
        jbe     @f
        mov     eax, [cur_height]
@@:
        sub     eax, [esp+8]
        ja      @f
        ret     16
@@:
        mov     [esp+16], eax
        mov     eax, [esp+4]
        mov     edx, [esp+8]
        call    get_console_ptr
        mov     esi, edi
        mov     edi, ebp
.l:
        mov     ecx, [esp+12]
        push    esi
        shr     ecx, 1
        rep     movsd
        adc     ecx, ecx
        rep     movsw
        pop     esi
        add     esi, [cur_width]
        add     esi, [cur_width]
        dec     dword [esp+16]
        jnz     .l
        ret     16

restore_console_data:
        cmp     dword [esp+4], 0
        jge     @f
        and     dword [esp+4], 0
@@:
        cmp     dword [esp+8], 0
        jge     @f
        and     dword [esp+8], 0
@@:
        mov     eax, [esp+12]
        cmp     eax, [cur_width]
        jbe     @f
        mov     eax, [cur_width]
@@:
        sub     eax, [esp+4]
        ja      @f
        ret     16
@@:
        mov     [esp+12], eax
        mov     eax, [esp+16]
        cmp     eax, [cur_height]
        jbe     @f
        mov     eax, [cur_height]
@@:
        sub     eax, [esp+8]
        ja      @f
        ret     16
@@:
        mov     [esp+16], eax
        mov     eax, [esp+4]
        mov     edx, [esp+8]
        call    get_console_ptr
        mov     esi, ebp
.l:
        mov     ecx, [esp+12]
        push    edi
        shr     ecx, 1
        rep     movsd
        adc     ecx, ecx
        rep     movsw
        pop     edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        dec     dword [esp+16]
        jnz     .l
        ret     16

; int __stdcall menu(void* variants, const char* title, unsigned flags);
; variants указывает на текущий элемент в двусвязном линейном списке
menu:
        pop     eax
        push    [cur_height]
        push    [cur_width]
        push    0
        push    0
        push    eax

; int __stdcall menu_centered_in(unsigned left, unsigned top, unsigned width, unsigned height,
;       void* variants, const char* title, unsigned flags);
menu_centered_in:
        pushad
        mov     ecx, 56
; 36 bytes for dlgtemplate + additional:
; +36: dd cur_variant
; +40: dd num_variants
; +44: dd begin_variant
; +48: dd end_variant
; +52: dd cur_variant_idx
        call    xpgalloc
        test    eax, eax
        jnz     @f
.ret_bad:
        popad
        or      eax, -1
        ret     28
@@:
        mov     ebx, eax
        mov     eax, 1
        test    byte [esp+20h+28], 1
        jz      @f
        mov     al, 3
@@:
        mov     [ebx+dlgtemplate.border_size_x], eax
        inc     eax
        shr     eax, 1
        mov     [ebx+dlgtemplate.border_size_y], eax
; Находим ширину и высоту окна
        xor     eax, eax
        xor     ecx, ecx
        mov     esi, [esp+20h+20]
        mov     [ebx+36], esi
        and     dword [ebx+52], 0
@@:
        cmp     dword [esi+4], 0
        jz      .find_width
        mov     esi, [esi+4]
        inc     dword [ebx+52]
        jmp     @b
.find_width:
        mov     [ebx+44], esi
        add     esi, 8
        push    esi
        xor     edx, edx
.fw1:
        cmp     byte [esi], '&'
        jnz     @f
        mov     dl, 1
@@:
        inc     esi
        cmp     byte [esi-1], 0
        jnz     .fw1
        sub     esi, [esp]
        sub     esi, edx
        dec     esi
        cmp     eax, esi
        ja      @f
        mov     eax, esi
@@:
        inc     ecx
        pop     esi
        mov     esi, [esi-8]
        test    esi, esi
        jnz     .find_width
        add     eax, 3
        add     eax, [ebx+dlgtemplate.border_size_x]
        add     eax, [ebx+dlgtemplate.border_size_x]
        cmp     eax, [cur_width]
        jb      @f
        mov     eax, [cur_width]
@@:
        sub     eax, [ebx+dlgtemplate.border_size_x]
        sub     eax, [ebx+dlgtemplate.border_size_x]
        mov     [ebx+dlgtemplate.width], eax
        mov     [ebx+dlgtemplate.height], ecx
        mov     [ebx+40], ecx
        sub     eax, [esp+20h+12]
        neg     eax
        sar     eax, 1
        add     eax, [esp+20h+4]
        cmp     eax, [ebx+dlgtemplate.border_size_x]
        jge     @f
        mov     eax, [ebx+dlgtemplate.border_size_x]
@@:
        push    eax
        add     eax, [ebx+dlgtemplate.width]
        add     eax, [ebx+dlgtemplate.border_size_x]
        cmp     eax, [cur_width]
        jbe     @f
        pop     eax
        mov     eax, [cur_width]
        sub     eax, [ebx+dlgtemplate.width]
        sub     eax, [ebx+dlgtemplate.border_size_x]
        push    eax
@@:
        pop     [ebx+dlgtemplate.x]
        sub     ecx, [esp+20h+16]
        neg     ecx
        sar     ecx, 1
        add     ecx, [esp+20h+8]
        cmp     ecx, [ebx+dlgtemplate.border_size_y]
        jge     @f
        mov     ecx, [ebx+dlgtemplate.border_size_y]
@@:
        push    ecx
        add     ecx, [ebx+dlgtemplate.height]
        add     ecx, [ebx+dlgtemplate.border_size_y]
        cmp     ecx, [cur_height]
        jbe     @f
        pop     ecx
        mov     ecx, [cur_height]
        sub     ecx, [ebx+dlgtemplate.height]
        sub     ecx, [ebx+dlgtemplate.border_size_y]
        push    ecx
@@:
        pop     [ebx+dlgtemplate.y]
        mov     eax, [cur_height]
        sub     eax, 6
        cmp     [ebx+dlgtemplate.height], eax
        jbe     .small_height
        mov     [ebx+dlgtemplate.height], eax
        mov     [ebx+dlgtemplate.y], 3
.small_height:
        mov     ecx, [ebx+dlgtemplate.height]
        mov     eax, [ebx+36]
        mov     [ebx+44], eax
        dec     ecx
        jz      .skip
        push    ecx
@@:
        cmp     dword [eax+4], 0
        jz      @f
        mov     eax, [eax+4]
        loop    @b
@@:
        mov     [ebx+44], eax
        pop     ecx
.loop:
        mov     eax, [eax]
        loop    .loop
.skip:
        mov     [ebx+48], eax
        mov     eax, [esp+20h+24]
        mov     [ebx+dlgtemplate.title], eax
        mov     al, [menu_normal_color]
        mov     [ebx+dlgtemplate.main_color], al
        mov     al, [menu_border_color]
        mov     [ebx+dlgtemplate.border_color], al
        mov     al, [menu_header_color]
        mov     [ebx+dlgtemplate.header_color], al
        push    MenuDlgProc
        push    ebx
        call    GenericBox
        mov     [esp+28], eax
        mov     ecx, ebx
        call    pgfree
        popad
        ret     28

MenuDlgProc:
        mov     eax, [esp+8]
        cmp     al, 1
        jz      .draw
        cmp     al, 2
        jz      .key
        ret     16
.draw:
        call    .dodraw
        ret     16
.prev:
        mov     eax, [ebx+36]
        cmp     dword [eax+4], 0
        jz      .end
        call    .line_prev
.posret:
        mov     [ebx+36], eax
.redraw:
        call    .dodraw
        call    draw_image
        xor     eax, eax
        ret     16
.next:
        mov     eax, [ebx+36]
        cmp     dword [eax], 0
        jz      .home
        call    .line_next
        jmp     .posret
.pgdn:
        mov     eax, [ebx+36]
        mov     ecx, [ebx+dlgtemplate.height]
.pgdnl:
        cmp     dword [eax], 0
        jz      .posret
        call    .line_next
        loop    .pgdnl
        jmp     .posret
.key:
        mov     al, [esp+12]
        cmp     al, 0x48
        jz      .prev
        cmp     al, 0x4B
        jz      .prev
        cmp     al, 0x4D
        jz      .next
        cmp     al, 0x50
        jz      .next
        cmp     al, 0x1C
        jz      .enter
        cmp     al, 1
        jz      .esc
        cmp     al, 0x47
        jz      .home
        cmp     al, 0x4F
        jz      .end
        cmp     al, 0x51
        jz      .pgdn
        cmp     al, 0x49
        jz      .pgup
        mov     edx, [ebx+36]
@@:
        cmp     dword [edx+4], 0
        jz      @f
        mov     edx, [edx+4]
        jmp     @b
@@:
.l:
        lea     esi, [edx+7]
@@:
        inc     esi
        cmp     byte [esi], 0
        jz      .n
        cmp     byte [esi], '&'
        jnz     @b
        movzx   ecx, byte [esi+1]
        cmp     [ascii2scan+ecx], al
        jnz     .n
        mov     eax, edx
        ret     16
.n:
        mov     edx, [edx]
        test    edx, edx
        jnz     .l
.ret:
        xor     eax, eax
        ret     16
.pgup:
        mov     eax, [ebx+36]
        mov     ecx, [ebx+dlgtemplate.height]
.pgupl:
        cmp     dword [eax+4], 0
        jz      .posret
        call    .line_prev
        loop    .pgupl
        jmp     .posret
.home:
        mov     eax, [ebx+36]
@@:
        cmp     dword [eax+4], 0
        jz      @f
        mov     eax, [eax+4]
        jmp     @b
@@:
        mov     [ebx+44], eax
        push    eax
        mov     ecx, [ebx+dlgtemplate.height]
        dec     ecx
        jz      .h1
.h2:
        mov     eax, [eax]
        loop    .h2
.h1:
        mov     [ebx+48], eax
        pop     eax
        and     dword [ebx+52], 0
        jmp     .posret
.end:
        mov     eax, [ebx+36]
@@:
        cmp     dword [eax], 0
        jz      @f
        mov     eax, [eax]
        jmp     @b
@@:
        mov     [ebx+48], eax
        push    eax
        mov     ecx, [ebx+dlgtemplate.height]
        dec     ecx
        jz      .e1
.e2:
        mov     eax, [eax+4]
        loop    .e2
.e1:
        mov     [ebx+44], eax
        mov     eax, [ebx+40]
        dec     eax
        mov     [ebx+52], eax
        pop     eax
        jmp     .posret
.esc:
        or      eax, -1
        ret     16
.enter:
        mov     eax, [ebx+36]
        ret     16

.line_prev:
        cmp     eax, [ebx+44]
        jnz     @f
        mov     edx, [ebx+44]
        mov     edx, [edx+4]
        mov     [ebx+44], edx
        mov     edx, [ebx+48]
        mov     edx, [edx+4]
        mov     [ebx+48], edx
@@:
        mov     eax, [eax+4]
        dec     dword [ebx+52]
        ret
.line_next:
        cmp     eax, [ebx+48]
        jnz     @f
        mov     edx, [ebx+44]
        mov     edx, [edx]
        mov     [ebx+44], edx
        mov     edx, [ebx+48]
        mov     edx, [edx]
        mov     [ebx+48], edx
@@:
        mov     eax, [eax]
        inc     dword [ebx+52]
        ret

.dodraw:
        mov     eax, [ebx+dlgtemplate.x]
        mov     edx, [ebx+dlgtemplate.y]
        call    get_console_ptr
        mov     esi, [ebx+44]
.0:
        xor     edx, edx
        mov     ah, [menu_selected_color]
        cmp     esi, [ebx+36]
        jz      @f
        mov     ah, [menu_normal_color]
@@:
        push    edi
        mov     ecx, [ebx+dlgtemplate.width]
        mov     al, ' '
        stosw
        dec     ecx
        stosw
        dec     ecx
        dec     ecx
        push    esi
        add     esi, 8
@@:
        lodsb
        test    al, al
        jz      @f
        cmp     al, '&'
        jnz     .noamp
        test    dl, dl
        jnz     .noamp
        mov     dl, 1
        lodsb
        push    eax
        mov     ah, [menu_selected_highlight_color]
        push    ecx
        mov     ecx, [esp+8]
        cmp     ecx, [ebx+36]
        pop     ecx
        jz      .amp1
        mov     ah, [menu_highlight_color]
.amp1:
        stosw
        pop     eax
        jmp     .amp2
.noamp:
        stosw
.amp2:
        loop    @b
        mov     al, ' '
        cmp     byte [esi], 0
        jnz     .1
        lodsb
        jmp     .1
@@:
        mov     al, ' '
.1:
        stosw
        mov     al, ' '
        rep     stosw
        pop     esi edi
        add     edi, [cur_width]
        add     edi, [cur_width]
        cmp     esi, [ebx+48]
        jz      @f
        mov     esi, [esi]
        test    esi, esi
        jnz     .0
@@:
; Линейка прокрутки
        mov     ecx, [ebx+dlgtemplate.height]
        cmp     ecx, [ebx+40]
        jz      .noscrollbar
        sub     ecx, 2
        jbe     .noscrollbar
        mov     eax, [ebx+52]
        mul     ecx
        div     dword [ebx+40]
        push    eax
        mov     eax, [ebx+dlgtemplate.x]
        add     eax, [ebx+dlgtemplate.width]
        mov     edx, [ebx+dlgtemplate.y]
        call    get_console_ptr
        pop     edx
        inc     edx
        mov     al, 0x1E
        mov     ah, [menu_scrollbar_color]
        mov     [edi], ax
        add     edi, [cur_width]
        add     edi, [cur_width]
.2:
        mov     al, 0xB2
        dec     edx
        jz      @f
        mov     al, 0xB0
@@:
        mov     [edi], ax
        add     edi, [cur_width]
        add     edi, [cur_width]
        loop    .2
        mov     al, 0x1F
        stosw
.noscrollbar:
        ret

virtual at 0
dlgitemtemplate:
; Элементы:
;       1 = статический текст
;       2 = кнопка
;       3 = поле редактирования
.type           dd      ?
.x1             dd      ?
.y1             dd      ?
.x2             dd      ?
.y2             dd      ?
; Данные:
; для текста: const char* data - ASCIIZ-строка
; для кнопки: const char* data - заголовок
; для редактора: struct {unsigned maxlength; unsigned pos; unsigned start;
;                        char data[maxlength+1];}* data;
.data           dd      ?
.flags          dd      ?
; Флаги:
;       0 = выравнивание влево
;       1 = выравнивание по центру
;       2 = выравнивание вправо
;       4 = элемент имеет фокус ввода
;       8 = элемент может иметь фокус ввода
;       10h: для кнопки = кнопка по умолчанию (Enter на не-кнопке)
;            для поля ввода = данные были модифицированы
.size = $
end virtual
;       struct DLGDATA
;       {
;               DLGTEMPLATE dialog;     /* window description */
;               void* DlgProc;          /* dialog procedure */
; /* int __stdcall DlgProc(DLGDATA* dlg, int msg, int param1, int param2); */
;               void* user_data;        /* arbitrary user data */
;               unsigned num_items;     /* number of items in the following array */
;               DLGITEMTEMPLATE items[]; /* array of dialog items */
;       }
; int __stdcall DialogBox(DLGDATA* dlg);
DialogBox:
        push    ManagerDlgProc
        push    dword [esp+8]
        call    GenericBox
        ret     4

ManagerDlgProc:
        mov     ebp, ebx
        mov     eax, [esp+8]
        dec     eax
        jz      .draw
        dec     eax
        jz      .key
        xor     eax, eax
        ret     16
.draw:
        call    .dodraw
        ret     16
.key:
; find item with focus
        add     ebx, dlgtemplate.size+12
        mov     ecx, [ebx-4]
        jecxz   .nobtns
@@:
        test    [ebx+dlgitemtemplate.flags], 4
        jnz     @f
        add     ebx, dlgitemtemplate.size
        loop    @b
@@:
.nobtns:
        mov     al, [esp+12]
        cmp     al, 1
        jz      .esc
        cmp     al, 0x1C
        jz      .enter
        cmp     al, 0xF
        jz      .tab
        cmp     al, 0x48
        jz      .up
        cmp     al, 0x50
        jz      .down
        jecxz   @f
        cmp     [ebx+dlgitemtemplate.type], 3
        jz      .key_edit
@@:
        cmp     al, 0x4B
        jz      .left
        cmp     al, 0x4D
        jz      .right
.ret0:
        xor     eax, eax
        ret     16
.esc:
        or      eax, -1
        ret     16
.enter:
        cmp     [ebx+dlgitemtemplate.type], 2
        jnz     @f
.enter_found:
        mov     eax, ebx
        ret     16
@@:
        lea     ebx, [ebp+dlgtemplate.size+12]
        mov     ecx, [ebx-4]
.enter_find:
        cmp     [ebx+dlgitemtemplate.type], 2
        jnz     @f
        test    [ebx+dlgitemtemplate.flags], 0x10
        jnz     .enter_found
@@:
        add     ebx, dlgitemtemplate.size
        loop    .enter_find
        jmp     .enter_found
.tab:
        test    [ctrlstate], 3
        jnz     .shift_tab
.right:
.down:
        jecxz   .ret0
        and     byte [ebx+dlgitemtemplate.flags], not 4
        dec     ecx
        jz      .find_first_btn
@@:
        add     ebx, dlgitemtemplate.size
        test    [ebx+dlgitemtemplate.flags], 8
        jnz     .btn_found
        loop    @b
.find_first_btn:
        lea     ebx, [ebp+dlgtemplate.size+12]
@@:
        test    [ebx+dlgitemtemplate.flags], 8
        jnz     .btn_found
        add     ebx, dlgitemtemplate.size
        jmp     @b
.btn_found:
        or      byte [ebx+dlgitemtemplate.flags], 4
.ret_draw:
        mov     ebx, ebp
        call    .dodraw
        call    draw_image
        xor     eax, eax
        ret     16
.shift_tab:
.left:
.up:
        jecxz   .ret0
        and     byte [ebx+dlgitemtemplate.flags], not 4
        sub     ecx, [ebp+dlgtemplate.size+8]
        neg     ecx
        jz      .find_last_btn
@@:
        sub     ebx, dlgitemtemplate.size
        test    [ebx+dlgitemtemplate.flags], 8
        loopz   @b
        jnz     .btn_found
.find_last_btn:
        mov     ebx, [ebp+dlgtemplate.size+8]
        imul    ebx, dlgitemtemplate.size
        lea     ebx, [ebx+ebp+dlgtemplate.size+12]
@@:
        sub     ebx, dlgitemtemplate.size
        test    [ebx+dlgitemtemplate.flags], 8
        jz      @b
        jmp     .btn_found
.key_edit:
; обработка клавиш в поле ввода
        test    al, 0x80
        jnz     .ret0
        or      [ebx+dlgitemtemplate.flags], 0x10
        mov     edx, [ebx+dlgitemtemplate.data]
        cmp     al, 0x4B
        jz      .editor_left
        cmp     al, 0x4D
        jz      .editor_right
        cmp     al, 0x47
        jz      .editor_home
        cmp     al, 0x4F
        jz      .editor_end
        cmp     al, 0x0E
        jz      .editor_backspace
        cmp     al, 0x53
        jnz     .editor_char
.editor_del:
        mov     ecx, [edx+4]
        lea     edi, [ecx+edx+12]
        lea     esi, [edi+1]
        cmp     byte [edi], 0
        jz      .ret_test
        jmp     .copy_and_ret_test
.editor_left:
        mov     ecx, [edx+4]
        jecxz   @f
        dec     dword [edx+4]
@@:     jmp     .ret_test
.editor_right:
        mov     ecx, [edx+4]
        cmp     byte [edx+ecx+12], 0
        jz      @b
        inc     dword [edx+4]
        jmp     @b
.editor_home:
        and     dword [edx+4], 0
        jmp     @b
.editor_end:
        lea     edi, [edx+12]
        xor     eax, eax
        or      ecx, -1
        repnz   scasb
        not     ecx
        dec     ecx
        mov     [edx+4], ecx
.ret_test:
        mov     eax, [edx+4]
        cmp     [edx+8], eax
        jl      .ret_test.l1
        mov     [edx+8], eax
        jmp     .ret_test.l2
.ret_test.l1:
        add     eax, [ebx+dlgitemtemplate.x1]
        sub     eax, [ebx+dlgitemtemplate.x2]
        cmp     [edx+8], eax
        jge     .ret_test.l2
        mov     [edx+8], eax
.ret_test.l2:
        jmp     .ret_draw
.editor_backspace:
        mov     ecx, [edx+4]
        jecxz   .ret_test
        dec     dword [edx+4]
        lea     esi, [edx+ecx+12]
        lea     edi, [esi-1]
.copy_and_ret_test:
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        jmp     .ret_test
.editor_char:
        test    [ctrlstate], 0x3C
        jnz     .ret_draw
; query keyboard layout
        pushad
        push    26
        pop     eax
        push    2
        pop     ebx
        xor     ecx, ecx
        cmp     [ctrlstate], 1
        sbb     ecx, -2
        mov     edx, layout
        mcall
        popad
; translate scancode to ASCII
        movzx   eax, al
        movzx   eax, byte [layout+eax]
        push    eax
; insert entered symbol
        xor     eax, eax
        lea     edi, [edx+12]
        or      ecx, -1
        repnz   scasb
        not     ecx
        pop     eax
        cmp     ecx, [edx]
        ja      .ret_test       ; buffer capacity exceeded
        lea     edi, [edx+ecx+12-1]
        mov     esi, [edx+4]
        lea     esi, [edx+esi+12]
@@:
        mov     cl, [edi]
        mov     [edi+1], cl
        dec     edi
        cmp     edi, esi
        jae     @b
        mov     [esi], al
        inc     dword [edx+4]
@@:     jmp     .ret_test

.dodraw:
        or      [cursor_x], -1
        or      [cursor_y], -1
        add     ebx, dlgtemplate.size+8
        mov     ecx, [ebx]
        add     ebx, 4
        jecxz   .done_draw
.draw_loop:
        push    ecx
        mov     eax, [ebx+dlgitemtemplate.type]
        dec     eax
        jz      .draw_text
        dec     eax
        jz      .draw_button
        dec     eax
        jnz     .draw_loop_continue
        call    draw_editbox
        jmp     .draw_loop_continue
.draw_button:
        call    draw_button
        jmp     .draw_loop_continue
.draw_text:
        call    draw_static_text
.draw_loop_continue:
        pop     ecx
        add     ebx, dlgitemtemplate.size
        loop    .draw_loop
.done_draw:
        ret

draw_static_text:
; рисуем статический текст
        mov     ah, [dialog_main_color]
        test    byte [ebp+dlgtemplate.flags], 2
        jz      draw_text
        mov     ah, [warning_main_color]
draw_text:
; определяем длину строки
        mov     esi, [ebx+dlgitemtemplate.data]
draw_text_esi:
        test    esi, esi
        jz      .ret
        or      ecx, -1
@@:
        inc     ecx
        cmp     byte [ecx+esi], 0
        jnz     @b
; в ecx длина строки
        push    eax
        xor     eax, eax
        mov     edx, [ebx+dlgitemtemplate.x2]
        sub     edx, [ebx+dlgitemtemplate.x1]
        inc     edx
        cmp     ecx, edx
        jae     .text_draw
        mov     al, byte [ebx+dlgitemtemplate.flags]
        and     al, 3
        jz      .text_align_left
        cmp     al, 1
        jz      .text_align_center
; текст выровнен вправо
        mov     eax, edx
        sub     eax, ecx
        jmp     .text_draw
.text_align_center:
        mov     eax, edx
        sub     eax, ecx
        shr     eax, 1
        jmp     .text_draw
.text_align_left:
        xor     eax, eax
.text_draw:
        push    ecx
        push    eax
        mov     eax, [ebx+dlgitemtemplate.x1]
        add     eax, [ebp+dlgtemplate.x]
        push    edx
        mov     edx, [ebx+dlgitemtemplate.y1]
        add     edx, [ebp+dlgtemplate.y]
        call    get_console_ptr
        pop     edx
        pop     ecx
        mov     ah, [esp+5]
        mov     al, ' '
        rep     stosw
        pop     ecx
        cmp     ecx, edx
        jbe     .text_copy
        cmp     [ebx+dlgitemtemplate.type], 3
        jnz     @f
        mov     ecx, edx
        jmp     .text_copy
@@:
        cmp     edx, 3
        jb      .ret
        mov     al, '.'
        stosw
        stosw
        stosw
        add     esi, ecx
        mov     ecx, edx
        sub     ecx, 3
        sub     esi, ecx
.text_copy:
        jecxz   .ret
@@:
        lodsb
        stosw
        loop    @b
.ret:
        mov     eax, [ebp+dlgtemplate.x]
        mov     edx, [ebp+dlgtemplate.y]
        add     eax, [ebx+dlgitemtemplate.x2]
        inc     eax
        add     edx, [ebx+dlgitemtemplate.y1]
        mov     ecx, edi
        call    get_console_ptr
        xchg    ecx, edi
        sub     ecx, edi
        shr     ecx, 1
        pop     eax
        mov     al, ' '
        rep     stosw
        ret

draw_button:
        mov     ecx, dialog_colors
        test    byte [ebp+dlgtemplate.flags], 2
        jz      @f
        mov     ecx, warning_colors
@@:
        mov     ah, [dialog_normal_btn_color-dialog_colors+ecx]
        test    [ebx+dlgitemtemplate.flags], 4
        jz      @f
        mov     ah, [dialog_selected_btn_color-dialog_colors+ecx]
@@:
        jmp     draw_text

draw_editbox:
        mov     edx, [ebx+dlgitemtemplate.data]
        test    [ebx+dlgitemtemplate.flags], 4
        jz      @f
        mov     eax, [ebx+dlgitemtemplate.x1]
        add     eax, [edx+4]
        sub     eax, [edx+8]
        add     eax, [ebp+dlgtemplate.x]
        mov     [cursor_x], eax
        mov     eax, [ebx+dlgitemtemplate.y1]
        add     eax, [ebp+dlgtemplate.y]
        mov     [cursor_y], eax
@@:
        mov     ecx, dialog_colors
        test    byte [ebp+dlgtemplate.flags], 2
        jz      @f
        mov     ecx, warning_colors
@@:
        mov     ah, [dialog_edit_color-dialog_colors+ecx]
        test    [ebx+dlgitemtemplate.flags], 10h
        jnz     @f
        mov     ah, [dialog_unmodified_edit_color-dialog_colors+ecx]
@@:
        mov     esi, [ebx+dlgitemtemplate.data]
        add     esi, [edx+8]
        add     esi, 12
        jmp     draw_text_esi

; void __stdcall SayNoMem(void);
SayNoMem:
        or      dword [nomem_dlgdata+4], -1
        or      dword [nomem_dlgdata+8], -1
        push    nomem_dlgdata
        call    DialogBox
        ret

; int __stdcall SayErr(const char* title, int x, int y,
;                       int num_strings, const char* strings[],
;                       int num_buttons, const char* buttons[]);
; may be x=-1 and/or y=-1
SayErr:
        push    2
        jmp     @f

; int __stdcall Message(const char* title, int x, int y,
;                       int num_strings, const char* strings[],
;                       int num_buttons, const char* buttons[]);
Message:
        push    1
@@:
; [esp+4] = title
; [esp+8] = x
; [esp+12] = y
; [esp+16] = num_strings
; [esp+20] = strings
; [esp+24] = num_buttons
; [esp+28] = buttons
        pop     eax
        pushad
        mov     ecx, [esp+4+16]
        add     ecx, [esp+4+24]
        imul    ecx, dlgitemtemplate.size
        add     ecx, dlgtemplate.size+12
        call    xpgalloc
        test    eax, eax
        jnz     @f
        popad
        or      eax, -1
        ret     28
@@:
        mov     ebx, eax
        mov     edi, eax
        mov     eax, [esp+28]
        stosd                           ; dlgtemplate.flags
        mov     eax, [esp+32+8]
        stosd                           ; dlgtemplate.x
        mov     eax, [esp+32+12]
        stosd                           ; dlgtemplate.y
; calculate width
        mov     ecx, [esp+32+16]
        mov     esi, [esp+32+20]
        xor     edx, edx
.calcwidth:
        lodsd
@@:
        inc     eax
        cmp     byte [eax-1], 0
        jnz     @b
        sub     eax, [esi-4]
        inc     eax
        cmp     edx, eax
        ja      @f
        mov     edx, eax
@@:
        loop    .calcwidth
        mov     ecx, [esp+32+24]
        mov     esi, [esp+32+28]
        xor     ebp, ebp
.calcwidth2:
        lodsd
@@:
        inc     eax
        cmp     byte [eax-1], 0
        jnz     @b
        sub     eax, [esi-4]
        inc     eax
        add     ebp, eax
        loop    .calcwidth2
        inc     ebp
        inc     ebp
        cmp     edx, ebp
        ja      @f
        mov     edx, ebp
@@:
        mov     eax, [cur_width]
        sub     eax, 8
        cmp     edx, eax
        jb      @f
        mov     edx, eax
@@:
        mov     eax, edx
        stosd                           ; dlgtemplate.width
        mov     eax, [esp+32+16]
        inc     eax
        stosd                           ; dlgtemplate.height
        mov     eax, 3
        stosd                           ; dlgtemplate.border_size_x
        mov     al, 2
        stosd                           ; dlgtemplate.border_size_y
        mov     eax, [esp+32+4]
        stosd                           ; dlgtemplate.title
        xor     eax, eax
        stosd                           ; (ignored)
        stosd                           ; DlgProc
        stosd                           ; userdata
        mov     eax, [esp+32+16]
        add     eax, [esp+32+24]
        stosd                           ; num_items
; fill strings
        xor     ecx, ecx
        mov     esi, [esp+32+20]
@@:
        mov     eax, 1
        stosd                           ; dlgitemtemplate.type
        dec     eax
        stosd                           ; dlgitemtemplate.x1
        mov     eax, ecx
        stosd                           ; dlgitemtemplate.y1
        lea     eax, [edx-1]
        stosd                           ; dlgitemtemplate.x2
        mov     eax, ecx
        stosd                           ; dlgitemtemplate.y2
        movsd                           ; dlgitemtemplate.data
        mov     eax, 1
        stosd                           ; dlgitemtemplate.flags
        inc     ecx
        cmp     ecx, [esp+32+16]
        jb      @b
; fill buttons
        mov     ecx, [esp+32+24]
        mov     esi, [esp+32+28]
        sub     edx, ebp
        jc      .big
        shr     edx, 1
        inc     edx
        jmp     .fillbtns
.big:
        xor     edx, edx
.fillbtns:
        mov     eax, 2
        stosd                           ; dlgitemtemplate.type
        mov     eax, edx
        stosd                           ; dlgitemtemplate.x1
        mov     eax, [ebx+dlgtemplate.height]
        dec     eax
        stosd                           ; dlgitemtemplate.y1
        push    eax
        lodsd
        sub     eax, edx
@@:
        inc     edx
        cmp     byte [eax+edx-1], 0
        jnz     @b
        mov     eax, edx
        inc     edx
        stosd                           ; dlgitemtemplate.x2
        pop     eax
        stosd                           ; dlgitemtemplate.y2
        mov     eax, [esi-4]
        stosd                           ; dlgitemtemplate.data
        mov     eax, 9
        cmp     ecx, [esp+32+24]
        jnz     @f
        or      al, 4
@@:
        stosd                           ; dlgitemtemplate.flags
        loop    .fillbtns
        push    ebx
        call    DialogBox
        cmp     eax, -1
        jz      @f
        sub     eax, ebx
        sub     eax, dlgtemplate.size+12
        xor     edx, edx
        mov     ecx, dlgitemtemplate.size
        div     ecx
        sub     eax, [esp+32+16]
@@:
        mov     [esp+28], eax
        mov     ecx, ebx
        call    pgfree
        popad
        ret     28
