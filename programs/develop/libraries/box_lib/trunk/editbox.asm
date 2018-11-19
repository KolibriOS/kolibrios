macro use_key_no_process  up,down,esc,enter,tab,numl,capsl,scrolll,pgup,pgdown
{
if up eq
else
        cmp     ah,178
        jz      edit_box.editbox_exit
end if
if down eq
else
        cmp     ah,177
        jz      edit_box.editbox_exit
end if
if esc eq
else
        cmp     ah,27   ;ESC - клавиша ))
        jz      edit_box.editbox_exit
end if
if enter eq
else
        cmp     ah,13   ;ENTER - клавиша ))
        jz      edit_box.editbox_exit
end if
if tab eq
else
        cmp     ah,9   ;TAB - клавиша ))
        jz      edit_box.editbox_exit
end if
if numl eq
else
        cmp     ah,4   ;Num Lock - клавиша ))
        jz      edit_box.editbox_exit
end if
if capsl eq
else
        cmp     ah,2   ;Caps Lock - клавиша ))
        jz      edit_box.editbox_exit
end if
if scrolll eq
else
        cmp     ah,1   ;Scroll Lock - клавиша ))
        jz      edit_box.editbox_exit
end if
if pgup eq
else
        cmp     ah,184 ;Page Up - клавиша ))
        jz      edit_box.editbox_exit
end if
if pgdown eq
else
        cmp     ah,183 ;Page Dwon - клавиша ))
        jz      edit_box.editbox_exit
end if
}

SCAN_LWIN_RELEASE = 0xDB
SCAN_RWIN_RELEASE = 0xDC

align 16
edit_box:
.draw:
        pushad
        mov     edi,[esp+36]
        and     dword ed_text_color,17FFFFFFh
        mov     ecx,ed_text_color
        mov     ebx,ecx
        shr     ecx,28
        shl     ebx,4
        shr     ebx,28
        inc     ebx
        mov     eax,6
        jecxz   @f
        mov     al, 8
@@:
        mul     bl
        mov     ed_char_width,eax
        mov     al, 9
        jecxz   @f
        mov     al, 16
@@:
        mul     bl
        add     eax,4
        mov     ed_height,eax
        call    .draw_border
.draw_bg_cursor_text:
        call    .check_offset
        call    .draw_bg
        call    .draw_shift
.draw_cursor_text:
        call    .draw_text
        test    word ed_flags,ed_focus
        jz      .editbox_exit
        call    .draw_cursor
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Общий выход из editbox для всех функций и пост обработчиков;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.editbox_exit:
        popad
        ret 4

;==========================================================
;=== обработка клавиатуры =================================
;==========================================================
align 16
edit_box_key:
        pushad
        mov     edi,[esp+36]
        test    word ed_flags,ed_focus ; если не в фокусе, выходим
        jz      edit_box.editbox_exit
        test    word ed_flags,ed_mouse_on or ed_disabled
        jnz     edit_box.editbox_exit
;--------------------------------------
; this code for Win-keys, works with
; kernel SVN r.3356 or later
        push    eax
        push    ebx
        mcall   SF_KEYBOARD,SSF_GET_CONTROL_KEYS
        test    ax,0x200        ; LWin
        jnz     .win_key_pressed
        test    ax,0x400        ; RWin
        jz      @f
.win_key_pressed:
        pop     ebx
        pop     eax
        jmp     edit_box.editbox_exit

@@:     pop     ebx
        pop     eax
;--------------------------------------
;Проверка нажат shift ?
        call    edit_box_key.check_shift_ctrl_alt
;----------------------------------------------------------
;--- проверяем, что нажато --------------------------------
;----------------------------------------------------------
        cmp     ah,8
        jz      edit_box_key.backspace
        cmp     ah,0xb6
        jz      edit_box_key.delete
        cmp     ah,176
        jz      edit_box_key.left
        cmp     ah,179
        jz      edit_box_key.right
        cmp     ah,180
        jz      edit_box_key.home
        cmp     ah,181
        jz      edit_box_key.end
        cmp     ah,185  ;insert
        jz      edit_box_key.insert

; get scancode in ah
        ror     eax,8
; check for ctrl+ combinations
        test    word ed_flags,ed_ctrl_on
        jz      @f
        cmp     ah,45 ; Ctrl + X
        je      edit_box_key.ctrl_x
        cmp     ah,46 ; Ctrl + C
        je      edit_box_key.ctrl_c
        cmp     ah,47 ; Ctrl + V
        je      edit_box_key.ctrl_v
        cmp     ah,30 ; Ctrl + A
        je      edit_box_key.ctrl_a
@@:
        cmp     ah,SCAN_LWIN_RELEASE
        jz      edit_box.editbox_exit
        cmp     ah,SCAN_RWIN_RELEASE
        jz      edit_box.editbox_exit
; restore ascii code
        rol     eax,8
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Заглушка на обработку клавиш вверх и вниз т.е. при обнаружении этих кодов происходит выход из обработчика
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
use_key_no_process   up,down,esc,enter,tab,numl,capsl,scrolll,pgup,pgdown
;--- нажата другая клавиша ---
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Проверка установлен ли флаг при котором нужно выводить только цифры в нужном боксе
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        test    word ed_flags,ed_figure_only  ; только цифры?
        jz      @f
        cmp     ah,'0'
        jb      edit_box.editbox_exit
        cmp     ah,'9'
        ja      edit_box.editbox_exit
@@:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;проверка на shift, был ли нажат
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        test    word ed_flags,ed_shift_on
        je      @f
; edx = ed_size, ecx = ed_pos
        push    eax
        mov     edx,ed_size
        mov     ecx, ed_pos
        pusha
; clear input area
        mov     ebp,ed_color
        movzx   ebx, word ed_shift_pos
        call    edit_box_key.sh_cl_
        mov     ebp,ed_size
        call    edit_box_key.clear_bg
        popa
        call    edit_box_key.del_char
        mov     ebx,ed_size
        sub     bx,ed_shift_pos
        mov     ed_size,ebx
        pop     eax
@@:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; проверяем, находится ли курсор в конце + дальнейшая обработка
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov     ecx,ed_size
        mov     edx, ed_max
        test    word ed_flags,ed_insert
        jne     @f
        cmp     ecx,edx
        jae     edit_box.editbox_exit
@@:     mov     ebx, ed_pos
        cmp     ebx,edx
        jnl     edit_box.editbox_exit
        mov     ecx,ed_size
        push    edi eax
        mov     ebp,edi
        mov     esi,ed_text
        add     esi,ecx
        mov     edi,esi
        cmp     ecx,ebx
        je      edit_box_key.In_k
        test    dword bp_flags,ed_insert
        jne     edit_box_key.ins_v
        pusha
        mov     edi,ebp
        mov     ebp,ed_size
        call    edit_box_key.clear_bg
        popa
        sub     ecx,ebx
        inc     edi
        std
        inc     ecx
@@:
        lodsb
        stosb
        loop    @b
edit_box_key.In_k:
        cld
        pop     eax
        mov     al,ah
        stosb
        pop     edi
        inc     dword ed_size
        inc     dword ed_pos
        call    edit_box_key.draw_all2
        jmp     edit_box_key.shift

;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Обработка клавиш insert,delete,backspace,home,end,left,right
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
edit_box_key.insert:
        test    word ed_flags,ed_insert
        je      @f
        and     word ed_flags,ed_insert_cl
        jmp     edit_box.editbox_exit

@@:     or      word ed_flags,ed_insert
        jmp     edit_box.editbox_exit

edit_box_key.ins_v:
        dec     dword bp_size
        sub     esi,ecx
        add     esi,ebx
        mov     edi,esi
        pusha
        mov     edi,ebp
        mov     ebp,ed_pos
        call    edit_box_key.clear_bg
        popa
        jmp     edit_box_key.In_k

edit_box_key.delete:
        mov     edx,ed_size
        mov     ecx,ed_pos
        cmp     edx,ecx
        jg      edit_box_key.bac_del
        test    word ed_flags,ed_shift_on
        jne     edit_box_key.del_bac
        popad
        ret     4

edit_box_key.bac_del:
        call    edit_box_key.del_char
        jmp     edit_box_key.draw_all

edit_box_key.backspace:
        test    word ed_flags,ed_shift_on
        jne     edit_box_key.delete
        mov     ecx,ed_pos
        test    ecx,ecx
        jnz     edit_box_key.del_bac
        popad
        ret     4

edit_box_key.del_bac:
        mov     edx,ed_size
        cmp     edx,ecx ;if ed_pos=ed_size
        je      @f
        dec     ecx
        call    edit_box_key.del_char
@@:     test    word ed_flags,ed_shift_on
        jne     edit_box_key.bac_del
        dec     dword ed_pos
edit_box_key.draw_all:
        push    edit_box_key.shift
        test    word ed_flags,ed_shift_on
        je      @f
        movzx   eax, word ed_shift_pos
        mov     ebx,ed_size
        sub     ebx,eax
        mov     ed_size,ebx
        mov     ebp,ed_color
        call    edit_box.clear_cursor
        call    edit_box.check_offset
        and     word ed_flags,ed_shift_cl
        jmp     edit_box.draw_bg

@@:     dec     dword ed_size
edit_box_key.draw_all2:
        and     word ed_flags,ed_shift_cl
        mov     ebp,ed_color
        call    edit_box.clear_cursor
        call    edit_box.check_offset
        mov     ebp,ed_size
        jmp     edit_box_key.clear_bg

;--- нажата клавиша left ---
edit_box_key.left:
        mov     ebx,ed_pos
        test    ebx,ebx
        jz      edit_box_key.sh_st_of
        or      word ed_flags,ed_left_fl
        call    edit_box_key.sh_first_sh
        dec     dword ed_pos
        call    edit_box.draw_bg
        call    edit_box.draw_shift
        call    edit_box_key.sh_enable
        jmp     edit_box.draw_cursor_text

;--- нажата клавиша right ---
edit_box_key.right:
        mov     ebx,ed_pos
        cmp     ebx,ed_size
        je      edit_box_key.sh_st_of
        and     word ed_flags,ed_right_fl
        call    edit_box_key.sh_first_sh
        inc     dword ed_pos
        call    edit_box.draw_bg
        call    edit_box.draw_shift
        call    edit_box_key.sh_enable
        jmp     edit_box.draw_cursor_text

edit_box_key.home:
        mov     ebx,ed_pos
        test    ebx,ebx
        jz      edit_box_key.sh_st_of
        call    edit_box_key.sh_first_sh
        xor     eax,eax
        mov     ed_pos,eax
        call    edit_box.draw_bg
        call    edit_box.draw_shift
        call    edit_box_key.sh_home_end
        jmp     edit_box.draw_cursor_text

;--- нажата клавиша end ---
edit_box_key.end:
        mov     ebx,ed_pos
        cmp     ebx,ed_size
        je      edit_box_key.sh_st_of
        call    edit_box_key.sh_first_sh
        mov     eax,ed_size
        mov     ed_pos,eax
        call    edit_box.draw_bg
        call    edit_box.draw_shift
        call    edit_box_key.sh_home_end
        jmp     edit_box.draw_cursor_text
;----------------------------------------
StrInsert:
; SizeOf(TmpBuf) >= StrLen(Src) + StrLen(Dst) + 1
Dst    equ [esp + 16] ; - destination buffer
Src    equ [esp + 12] ; - source to insert from
Pos    equ [esp + 8] ; - position for insert
DstMax equ [esp + 4]  ; - maximum Dst length(exclude terminating null)
SrcCount equ [esp - 4]
DstCount equ [esp - 8]
TmpBuf   equ [esp - 12]  ; - temporary buffer
        mov    edi, Src
        mov    ecx, -1
        xor    eax, eax
        repne scasb
        mov    eax, -2
        sub    eax, ecx
        mov    SrcCount, eax
        mov    edi, Dst
        add    edi, Pos
        mov    ecx, -1
        xor    eax, eax
        repne scasb
        mov    eax, -2
        sub    eax, ecx
        inc    eax
        mov    DstCount, eax
        mov    ecx, eax
        add    ecx, SrcCount
        add    ecx, Pos
        mcall   SF_SYS_MISC,SSF_MEM_ALLOC
        mov    TmpBuf, eax
        mov    esi, Dst
        mov    edi, TmpBuf
        mov    ecx, Pos
        mov    edx, ecx
        rep movsb
        mov    esi, Src
        mov    edi, TmpBuf
        add    edi, Pos
        mov    ecx, SrcCount
        add    edx, ecx
        rep movsb
        mov    esi, Pos
        add    esi, Dst
        mov    ecx, DstCount
        add    edx, ecx
        rep movsb
        mov    esi, TmpBuf
        mov    edi, Dst
; ecx = MIN(edx, DstSize)
        cmp    edx, DstMax
        sbb    ecx, ecx
        and    edx, ecx
        not    ecx
        and    ecx, DstMax
        add    ecx, edx
        mov    eax, ecx ; return total length
        rep movsb
        mov    ecx, TmpBuf
        mcall   SF_SYS_MISC,SSF_MEM_FREE
        ret    16
restore Dst
restore Src
restore Pos
restore DstSize
restore TmpBuf
restore SrcCount
restore DstCount
;----------------------------------------
edit_box_key.ctrl_x:
        test   word ed_flags,ed_shift_on
        jz     edit_box.editbox_exit
        push    dword 'X'  ; this value need below to determine which action is used
        jmp     edit_box_key.ctrl_c.pushed

edit_box_key.ctrl_c:
        test   word ed_flags,ed_shift_on
        jz     edit_box.editbox_exit
        push    dword 'C'  ; this value need below to determine which action is used
.pushed:
; add memory area
        mov     ecx,ed_size
        add     ecx,3*4
        mcall   SF_SYS_MISC,SSF_MEM_ALLOC
; building the clipboard slot header
        xor     ecx,ecx
        mov     [eax+4],ecx ; type 'text'
        inc     ecx
        mov     [eax+8],ecx ; cp866 text encoding
        mov     ecx,ed_pos
        movzx   ebx,word ed_shift_pos
        sub     ecx,ebx
.abs: ; make ecx = abs(ecx)
	       neg     ecx
	       jl	     .abs
        add     ecx,3*4
        mov     [eax],ecx
        sub     ecx,3*4
        mov     edx,ed_pos
        movzx   ebx,word ed_shift_pos
        cmp     edx,ebx
        jle     @f
        mov     edx,ebx
@@:
; copy data
        mov     esi,ed_text
        add     esi,edx
        push    edi
        mov     edi,eax
        add     edi,3*4
        cld
        rep     movsb
        pop     edi
; put slot to the kernel clipboard
        mov     edx,eax
        mov     ecx,[edx]
        push    eax
        mcall   SF_CLIPBOARD,SSF_WRITE_CB
        pop     ecx
; remove unnecessary memory area
        mcall   SF_SYS_MISC,SSF_MEM_FREE
.exit:
        pop     eax        ; determine current action (ctrl+X or ctrl+C)
        cmp     eax, 'X'
        je      edit_box_key.delete
        jmp     edit_box.editbox_exit

edit_box_key.ctrl_v:
        mcall   SF_CLIPBOARD,SSF_GET_SLOT_COUNT
; no slots of clipboard ?
        test    eax,eax
        jz      .exit
; main list area not found ?
        inc     eax
        test    eax,eax
        jz      .exit
        sub     eax,2
        mov     ecx,eax
        mcall   SF_CLIPBOARD,SSF_READ_CB
; main list area not found ?
        inc     eax
        test    eax,eax
        jz      .exit
; error ?
        sub     eax,2
        test    eax,eax
        jz      .exit
        inc     eax
; check contents of container
        mov     ebx,[eax+4]
; check for text
        test    ebx,ebx
        jnz     .no_valid_text
        mov     ebx,[eax+8]
; check for cp866
        cmp     bl,1
        jnz     .no_valid_text
; if something selected then need to delete it
        test   word ed_flags,ed_shift_on
        jz     .selected_done
        push   eax; dummy parameter ; need to
        push   dword .selected_done ; correctly return
        pushad                      ; from edit_box_key.delete
        jmp    edit_box_key.delete
.selected_done:
        mov     ecx,[eax]
        sub     ecx,3*4
; in ecx size of string to insert
        add     ecx,ed_size
        mov     edx,ed_max
        cmp     ecx,edx
        jb      @f
        mov     ecx,edx
@@:
        mov     esi,eax
        add     esi,3*4
        push    eax edi
;---------------------------------------;
        mov     ed_size,ecx

        push   dword ed_text ; Dst
        push   esi           ; Src
        push   dword ed_pos  ; Pos in Dst
        push   dword ed_max  ; DstMax
        call   StrInsert
;---------------------------------------;
;        mov     edi,ed_text
;        cld
;@@:
;        lodsb
;        cmp     al,0x0d ; EOS (end of string)
;        je      .replace
;        cmp     al,0x0a ; EOS (end of string)
;        jne     .continue
;.replace:
;        mov     al,0x20 ; space
;.continue:
;        stosb
;        dec     ecx
;        jnz     @b
        pop     edi eax
.no_valid_text:
; remove unnecessary memory area
        mov     ecx,eax
        mcall   SF_SYS_MISC,SSF_MEM_FREE
.exit:
        jmp     edit_box.draw_bg_cursor_text

edit_box_key.ctrl_a:
        mov     eax,ed_size
        mov     ed_pos,eax
        xor     eax,eax
        mov     ed_shift_pos,eax
        or      word ed_flags,ed_shift_bac+ed_shift_on
        jmp     edit_box.draw_bg_cursor_text

;==========================================================
;=== обработка мыши =======================================
;==========================================================
;save for stdcall ebx,esi,edi,ebp
align 16
edit_box_mouse:
        pushad
        mov     edi,[esp+36]
        test    word ed_flags,ed_disabled
        jnz     edit_box.editbox_exit

;----------------------------------------------------------
;--- получаем состояние кнопок мыши -----------------------
;----------------------------------------------------------
        mcall   SF_MOUSE_GET,SSF_BUTTON
;----------------------------------------------------------
;--- проверяем состояние ----------------------------------
;----------------------------------------------------------
        test    eax,1
        jnz     edit_box_mouse.mouse_left_button
        and     word ed_flags,ed_mouse_on_off
        mov     ebx,ed_mouse_variable
        push    0
        pop     dword [ebx]
        jmp     edit_box.editbox_exit

.mouse_left_button:
;----------------------------------------------------------
;--- блокировка от фокусировки в других боксах при попадании на них курсора
;----------------------------------------------------------
        mov     eax,ed_mouse_variable
        push    dword [eax]
        pop     eax
        test    eax,eax
        jz      @f
        cmp     eax,edi
        je      @f
        jmp     edit_box_mouse._blur
;----------------------------------------------------------
;--- получаем координаты мыши относительно 0 т.е всей области экрана
;----------------------------------------------------------
@@:
        mcall   SF_MOUSE_GET,SSF_WINDOW_POSITION
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Функция обработки  мышки получение координат и проверка их + выделения
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Не удерживаем ли мы клавишу мышки, перемещая курсор?
        test    word ed_flags,ed_mouse_on
        jne     edit_box_mouse.mouse_wigwag
; проверяем, попадает ли курсор в edit box
        mov     ebx,ed_top
        cmp     ax,bx
        jl      edit_box_mouse._blur
        add     ebx,ed_height
        cmp     ax,bx
        jg      edit_box_mouse._blur
        shr     eax,16
        mov     ebx,ed_left
        cmp     ax,bx
        jl      edit_box_mouse._blur
        add     ebx,ed_width
        cmp     ax,bx
        jg      edit_box_mouse._blur
; изменяем позицию курсора
        push    eax
        mov     ebp,ed_color
        call    edit_box.clear_cursor
        pop     eax
edit_box_mouse._mvpos:
        xor     edx,edx
        sub     eax,ed_left
        div     word ed_char_width
        add     eax,ed_offset
        cmp     eax,ed_size
        jna     edit_box_mouse._mshift
        mov     eax,ed_size
edit_box_mouse._mshift:
; секция обработки shift и выделения по shift
        test    word ed_flags,ed_shift_bac
        je      @f
        mov     ebp,ed_color
        movzx   ebx, word ed_shift_pos
        push    eax
        call    edit_box_key.sh_cl_
        and     word ed_flags,ed_shift_bac_cl
        pop     eax
@@:
        test    word ed_flags,ed_mouse_on
        jne     @f
        mov     ed_shift_pos,ax
        or      word  ed_flags,ed_mouse_on
        mov     ed_pos,eax
        mov     ebx,ed_mouse_variable
        push    edi
        pop     dword [ebx]
        bts     word ed_flags,1
        call    edit_box.draw_bg
        jmp     edit_box_mouse.m_sh

@@:     cmp     ax,ed_shift_pos
        je      edit_box.editbox_exit
        mov     ed_pos,eax
        call    edit_box.draw_bg
        mov     ebp,shift_color
        movzx   ebx, word ed_shift_pos
        call    edit_box_key.sh_cl_
        or      word ed_flags,ed_mous_adn_b
edit_box_mouse.m_sh:
        call    edit_box.draw_text
        call    edit_box.draw_cursor
; процедура установки фокуса
        jmp     edit_box_mouse.drc

edit_box_mouse._blur:
        test    word ed_flags,ed_always_focus
        jne     edit_box.editbox_exit
        btr     word ed_flags,1 ; если не в фокусе, выходим
        jnc     edit_box.editbox_exit
        mov     ebp,ed_color
        call    edit_box.clear_cursor
edit_box_mouse.drc:
        call    edit_box.draw_border
        jmp     edit_box.editbox_exit

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Общие функции обработки
;----------------------------------------------------------
;--- процедура прорисовки выделенной части ----------------
;----------------------------------------------------------
edit_box.draw_shift:
        test    word ed_flags,ed_shift_bac ;установка флага, выделенной области
        jz      @f
        mov     ebp,shift_color
        movzx   ebx, word ed_shift_pos
        call    edit_box_key.sh_cl_
@@:     ret
;----------------------------------------------------------
;--- процедура прорисовки текста --------------------------
;----------------------------------------------------------
edit_box.draw_text:
        call    edit_box.get_n
        mov     esi,ed_size
        sub     esi,ed_offset
        cmp     eax,esi
        jae     @f
        mov     esi,eax
@@:
        test    esi,esi
        jz      @f
        mov     eax,SF_DRAW_TEXT
        mov     ebx,ed_left
        add     ebx,2
        shl     ebx,16
        add     ebx,ed_top
        add     ebx,3
        mov     ecx,ed_text_color
        test    dword ed_flags,ed_pass
        jnz     .password
        mov     edx,ed_text
        add     edx,ed_offset
        mcall
@@:
        ret

.password:
        mov     ebp,esi
        mov     esi,1
        mov     edx,txt_pass
@@:
        mcall
        rol     ebx,16
        add     ebx,ed_char_width
        rol     ebx,16
        dec     ebp
        jnz     @b
        ret

txt_pass db '*'
;----------------------------------------------------------
;--- процедура прорисовки фона ----------------------------
;----------------------------------------------------------
edit_box.draw_bg:
        mov     ebx,ed_left
        inc     ebx
        shl     ebx,16
        add     ebx,ed_width
        dec     ebx
        mov     edx,ed_color
        test    word ed_flags, ed_disabled
        jz      edit_box.draw_bg_eax
        mov     edx, 0xCACACA   ; TODO: add disabled_color field to editbox struct
edit_box.draw_bg_eax:
        mov     ecx,ed_top
        inc     ecx
        shl     ecx,16
        add     ecx,ed_height
        mcall   SF_DRAW_RECT
        ret

;----------------------------------------------------------
;--- процедура получения количества символов в текущей ширине компонента
;----------------------------------------------------------
edit_box.get_n:
        mov     eax,ed_width
        sub     eax,4
        xor     edx,edx
        div     word ed_char_width
        ret

;----------------------------------------------------------
;------------------ Draw Cursor Procedure -----------------
;----------------------------------------------------------
; in: ebp = Color
edit_box.clear_cursor:
        movzx   ebx, word cl_curs_x
        cmp     ebx, ed_left ;попадает ли курсор текстовое поле?
        jle     @f
        mov     edx, ebp
        movzx   ecx, word cl_curs_y
        cmp     ecx, ed_top
        jg      edit_box.draw_curs
@@:
        ret

edit_box.draw_cursor:
        mov     edx, ed_text_color
        mov     eax, ed_pos
        sub     eax, ed_offset
        mul     dword ed_char_width
        mov     ebx, eax
        add     ebx, ed_left
        inc     ebx
        mov     ecx, ed_top
        add     ecx, 2
        mov     cl_curs_x, bx
        mov     cl_curs_y, cx
edit_box.draw_curs:
        mov     eax, ebx
        shl     ebx, 16
        or      ebx, eax
        mov     eax, ecx
        shl     ecx, 16
        or      ecx, eax
        add     ecx, ed_height
        sub     ecx, 3
        mcall   SF_DRAW_LINE
        ret

;----------------------------------------------------------
;--- процедура рисования рамки ----------------------------
;----------------------------------------------------------
edit_box.draw_border:
        test    word ed_flags,ed_focus
        mov     edx,ed_focus_border_color
        jne     @f
        mov     edx,ed_blur_border_color
@@:
        mov     ebx,ed_left
        mov     ecx,ebx
        shl     ebx,16
        add     ebx,ecx
        add     ebx,ed_width
        mov     ecx,ed_top
        mov     esi,ecx
        shl     ecx,16
        add     ecx,esi
        mcall   SF_DRAW_LINE ; top
        mov     esi,ecx
        inc     ecx
        add     ecx,ed_height
        mov     ebp,ecx
        shl     ecx,16
        mov     cx,bp
        mcall   ; bottom
        mov     cx,si
        mov     ebp,ebx
        sub     ebx,ed_width
        mcall   ; left
        mov     ebx,ebp
        shl     ebx,16
        mov     bx,bp
        mcall   ; right
        ret

;----------------------------------------------------------
;--- проверка, зашел ли курсор за границы и, если надо, ---
;--- изменяем смещение ------------------------------------
;--- если смещение было, установка флага ed_offset_cl, иначе,
; если ничего не изменилось, то выставление ed_offset_fl
; в общей битовой матрице состояния компонентов word ed_flags
;----------------------------------------------------------
edit_box.check_offset:
        pushad
        mov     ecx,ed_pos
        mov     ebx,ed_offset
        cmp     ebx,ecx
        ja      edit_box.sub_8
        push    ebx
        call    edit_box.get_n
        pop     ebx
        mov     edx,ebx
        add     edx,eax
        inc     edx     ;необходимо для нормального положения курсора в крайней левой позиции
        cmp     edx,ecx
        ja      @f
        mov     edx,ed_size
        cmp     edx,ecx
        je      edit_box.add_end
        sub     edx,ecx
        cmp     edx,8
        jbe     edit_box.add_8
        add     ebx,8
        jmp     edit_box.chk_d

@@:     or      word ed_flags,ed_offset_fl
        popad
        ret

edit_box.sub_8:
        test    ecx,ecx
        jz      @f
        sub     ebx,8   ;ebx=ed_offset
        ja      edit_box.chk_d
@@:
        xor     ebx,ebx
        jmp     edit_box.chk_d

edit_box.add_end:
        sub     edx,eax
        mov     ebx,edx
        jmp     edit_box.chk_d

edit_box.add_8:
        add     ebx,edx
edit_box.chk_d:
        mov     ed_offset,ebx
        call    edit_box.draw_bg
        and     word ed_flags,ed_offset_cl
        popad
        ret

align 4
proc edit_box_set_text, edit:dword, text:dword
        pushad
        mov     edi,[edit]
        mov     ecx,ed_max
        inc     ecx
        mov     edi,[text]
        xor     al,al
        cld
        repne scasb
        mov     ecx,edi
        mov     edi,[edit]
        mov     esi,[text]
        sub     ecx,esi
        dec     ecx
        mov     ed_size,ecx
        mov     ed_pos,ecx
        and     word ed_flags,ed_shift_cl
        mov     edi,ed_text
        repne movsb
        mov     byte[edi],0
        popad
        ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Функции для работы с key
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;Обработка Shift для снятия выделения неизвестной области
edit_box_key.shift:
        call    edit_box.draw_bg
        test    word ed_flags,ed_shift
        je      edit_box_key.f_exit
        mov     ebp,shift_color
        or      word ed_flags,ed_shift_bac ;установка флага, выделенной области
        movzx   ebx, word ed_shift_pos
        call    edit_box_key.sh_cl_
        jmp     edit_box.draw_cursor_text

edit_box_key.f_exit:
        call    edit_box.check_offset
        and     word ed_flags,ed_shift_cl
        call    edit_box_key.enable_null
        jmp     edit_box.draw_cursor_text

edit_box_key.sh_cl_:
;обработка очистки, при левом - правом движении выделения
;для обработки снятия выделения
;входные параметры ebp=color ebx=ed_shift_pos
        mov     eax,ed_pos
        cmp     eax,ebx
        jae     edit_box_key.sh_n
        push    eax  ;меньшее в eax
        push    ebx  ;большее
        jmp     edit_box_key.sh_n1

edit_box_key.sh_n:
        push    ebx
        push    eax
edit_box_key.sh_n1:
        call    edit_box.check_offset
        call    edit_box.get_n
        mov     edx,eax ;size of ed_box
        mov     ecx,ed_offset
        add     eax,ecx ;eax = w_off= ed_offset+width
        mov     edx,eax ;save
        pop     ebx     ;большее
        pop     eax     ;меньшее
        cmp     eax,ecx         ;сравнение меньшего с offset.
        jae     edit_box_key.f_f            ;если больше
        xor     eax,eax
        cmp     edx,ebx         ;cравним размер w_off с большим
        jnb     @f
        mov     ebx,edx
@@:
        sub     ebx,ecx
        jmp     edit_box_key.nxt_f

edit_box_key.f_f:
        sub     eax,ecx
        cmp     edx,ebx         ;cравним размер w_off с большим
        jle     @f
        sub     ebx,ecx
        sub     ebx,eax
        jmp     edit_box_key.nxt_f

@@:     mov     ebx,edx
        sub     ebx,ecx
        sub     ebx,eax
edit_box_key.nxt_f:
        mul     dword ed_char_width
        xchg    eax,ebx
        mul     dword ed_char_width
        add     ebx,ed_left
        inc     ebx
        shl     ebx,16
        inc     eax
        mov     bx, ax
        mov     edx,ebp ;shift_color
        call    edit_box.draw_bg_eax
        jmp     edit_box_key.enable_null

;Установка- снятие выделения в один символ
edit_box_key.drw_sim:
        mov     eax,ed_pos
        call    edit_box_key.draw_rectangle
        jmp     edit_box_key.enable_null

;Функция установки выделения при движении влево и вправо и нажатии shift
edit_box_key.draw_wigwag:
        mov     ebp,shift_color
        call    edit_box.clear_cursor
        or      word ed_flags,ed_shift_bac ;установка флага выделенной области
        mov     ebp,shift_color
        mov     eax,ed_pos
        test    word ed_flags,ed_left_fl
        jnz     edit_box_key.draw_rectangle
        dec     eax
        jmp     edit_box_key.draw_rectangle

;Функция удаления выделения при движении влево и вправо и нажатии shift
edit_box_key.draw_wigwag_cl:
        mov     ebp,ed_color
        call    edit_box.clear_cursor
        mov     ebp,ed_color
        mov     eax,ed_pos
        test    word ed_flags,ed_left_fl
        jnz     edit_box_key.draw_rectangle
        dec     eax
        jmp     edit_box_key.draw_rectangle

;входной параметр ebx - ed_pos
edit_box_key.sh_first_sh:
        test    word ed_flags,ed_shift
        je      @f
        mov     ed_shift_pos_old,bx
        test    word ed_flags,ed_shift_on
        jne     @f
        mov     ed_shift_pos,bx
        or      word ed_flags,ed_shift_on
@@:     ret
;Обработка крайних положений в editbox при нажатом shift
;производит снятие выделения, если нет shift
;иначе вообще выходит
edit_box_key.sh_st_of:
        test    word ed_flags,ed_shift
        jne     @f
        test    word ed_flags,ed_shift_bac
        je      @f
        call    edit_box.draw_bg
        mov     ebp,ed_color
        movzx   ebx, word ed_shift_pos
        call    edit_box_key.sh_cl_  ;очистка выделеного фрагмента
        and     word ed_flags,ed_shift_cl ; очистка от того, что убрали выделение
        jmp     edit_box.draw_cursor_text

@@:     and     word ed_flags,ed_shift_off
        popad
        ret     4
;проверка состояния shift, был ли он нажат раньше?
edit_box_key.sh_enable:
        test    word ed_flags,ed_shift
        jne     edit_box_key.sh_ext_en ;нарисовать закрашенный прямоугольник
        test    word ed_flags,ed_shift_bac
        je      @f
        call    edit_box.check_offset
        mov     ebp,ed_color
        movzx   ebx, word ed_shift_pos
        call    edit_box_key.sh_cl_  ;очистка выделенного фрагмента
        call    edit_box_key.draw_wigwag_cl
        and     word ed_flags,ed_shift_cl ; 1вар не нужно
        ret

@@:     mov     ebp,ed_color
        call    edit_box.clear_cursor
        jmp     edit_box.check_offset

edit_box_key.sh_ext_en:
        call    edit_box.check_offset
        test    word ed_flags,ed_offset_fl
        je      @f
;Рисование закрашенных прямоугольников и их очистка
        movzx   eax, word ed_shift_pos
        mov     ebx,ed_pos
        movzx   ecx, word ed_shift_pos_old
;проверка и рисование закрашенных областей
        cmp     eax,ecx
        je      edit_box_key.1_shem
        jb      edit_box_key.smaller
        cmp     ecx,ebx
        ja      edit_box_key.1_shem
        call    edit_box_key.draw_wigwag_cl ;clear
        jmp     edit_box_key.sh_e_end

edit_box_key.smaller:
        cmp     ecx,ebx
        jb      edit_box_key.1_shem
        call    edit_box_key.draw_wigwag_cl ;clear
        jmp     edit_box_key.sh_e_end

edit_box_key.1_shem:
        call    edit_box_key.draw_wigwag
edit_box_key.sh_e_end:
        and     word ed_flags,ed_shift_off
        ret

@@:     mov     ebp,shift_color
        movzx   ebx, word ed_shift_pos
        call    edit_box_key.sh_cl_
        jmp     edit_box_key.sh_e_end
;функция для обработки shift при нажатии home and end
edit_box_key.sh_home_end:
        mov     ebp,ed_color
        call    edit_box.clear_cursor
        test    word ed_flags,ed_shift_bac
        je      @f
        mov     ebp,ed_color
        movzx   ebx, word ed_shift_pos_old
        call    edit_box_key.sh_cl_
@@:
        test    word ed_flags,ed_shift
        je      edit_box_key.sh_exit_ ;выйти
        mov     ebp,shift_color
        movzx   ebx, word ed_shift_pos
        call    edit_box_key.sh_cl_
        or      word ed_flags,ed_shift_bac ;установка флага, выделенной области
        jmp     edit_box_key.sh_e_end

edit_box_key.sh_exit_:
        call    edit_box.draw_bg
        jmp     edit_box.check_offset

;функция внесения 0 по адресу ed_size+1
edit_box_key.enable_null:
        pusha
        mov     eax,ed_size
        mov     ebx,ed_text
        test    eax,eax
        add     eax,ebx
        jne     @f
        inc     eax
@@:     xor     ebx,ebx
        mov     [eax],bl
        popad
        ret

;- удаление символа
;Входные данные edx=ed_size;ecx=ed_pos
edit_box_key.del_char:
        mov     esi,ed_text
        test    word ed_flags,ed_shift_on
        je      @f
        movzx   eax, word ed_shift_pos
        mov     ebx,esi
        cmp     eax,ecx
        jae     edit_box_key.dh_n
        mov     ed_pos,eax      ;чтобы не было убегания курсора
        mov     ebp,ecx
        sub     ebp,eax
        add     ebx,eax  ;eax меньше
        sub     edx,ecx
        add     esi,ecx
        mov     ed_shift_pos,bp
        jmp     edit_box_key.del_ch_sh

edit_box_key.dh_n:
        mov     ebp,eax
        sub     ebp,ecx
        add     ebx,ecx
        sub     edx,eax
        add     esi,eax
        mov     ed_shift_pos,bp
        jmp     edit_box_key.del_ch_sh

@@:     add     esi,ecx ;указатель + смещение к реальному буферу
        mov     ebx,esi
        inc     esi
        cld
        sub     edx,ecx
edit_box_key.del_ch_sh:
        push    edi
        mov     edi,ebx
@@:
        lodsb
        stosb
        dec     edx
        jns     @b
        pop     edi
        ret
;вычислить закрашиваемую область
;соглашение в ebp - передается ed_size
edit_box_key.clear_bg:
        call    edit_box.get_n  ;получить размер в символах ширины компонента
        push    eax
        mov     ebx,ed_offset
        add     eax,ebx ;eax = w_off= ed_offset+width
        mov     ebx,ebp ;ed_size
        cmp     eax,ebx
        jb      @f
        mov     eax,ed_pos
        sub     ebx,eax
        mov     ecx,ed_offset
        sub     eax,ecx
        jmp     edit_box_key.nxt

@@:     mov     ebx,ed_pos
        push    ebx
        sub     eax,ebx
        mov     ebx,eax ;It is not optimal
        pop     eax     ;ed_pos
        mov     ecx,ed_offset
        sub     eax,ecx
edit_box_key.nxt:
        mov     ebp,eax  ;проверка на выход закрашиваемой области за пределы длины
        add     ebp,ebx
        pop     edx
        cmp     ebp,edx
        je      @f
        inc     ebx
@@:
        mul     dword ed_char_width
        xchg    eax,ebx
        mul     dword ed_char_width
        add     ebx,ed_left
        inc     ebx
        shl     ebx,16
        inc     eax
        mov     bx, ax
        mov     edx,ed_color
        jmp     edit_box.draw_bg_eax

;;;;;;;;;;;;;;;;;;;
;;; Обработка примитивов
;;;;;;;;;;;;;;;;;;;;
;Нарисовать прямоугольник, цвет передается в ebp
;входные параметры:
;eax=dword ed_pos
;ebp=-цвет ed_color or shift_color
edit_box_key.draw_rectangle:
        sub     eax,ed_offset
        mul     dword ed_char_width
        add     eax,ed_left
        inc     eax
        shl     eax,16
        add     eax,ed_char_width
        mov     ebx,eax
        mov     edx,ebp
        jmp     edit_box.draw_bg_eax

;;;;;;;;;;;;;;;;;;
;;Проверка нажат ли shift
;;;;;;;;;;;;;;;;;;
edit_box_key.check_shift_ctrl_alt:
        pusha
        mcall   SF_KEYBOARD,SSF_GET_CONTROL_KEYS
        test    al,11b
        je      @f
        or      word ed_flags,ed_shift   ;установим флаг Shift
@@:
        and     word ed_flags,ed_ctrl_off ; очистим флаг Ctrl
        test    al,1100b
        je      @f
        or      word ed_flags,ed_ctrl_on   ;установим флаг Ctrl
@@:
        and     word ed_flags,ed_alt_off ; очистим флаг Alt
        test    al,110000b
        je      @f
        or      word ed_flags,ed_alt_on   ;установим флаг Alt
@@:
        popad
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Функции для работы с mouse
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
edit_box_mouse.mouse_wigwag:
        push    eax
        call    edit_box.draw_bg
        call    edit_box.draw_shift
        pop     eax
        or      word ed_flags,ed_shift_bac+ed_shift_on+ed_shift
;Обработка положения выделенного текста, когда происходит выход за пределы editbox
        test    eax,eax
        js      edit_box_mouse.mleft
        shr     eax,16
        sub     eax,ed_left
        jc      edit_box_mouse.mleft
        cmp     ed_width,eax
        jc      edit_box_mouse.mright
        xor     edx,edx
        div     word ed_char_width
;Обработка положения выделенного текста, в пределах области editbox
;Получили координаты в eax мышки, т.е. куда она переместилась
;Рисование закрашенных прямоугольников и их очистка
        add     eax,ed_offset
        cmp     eax,ed_size
        ja      edit_box_mouse.mwigvag
edit_box_mouse.mdraw:
        mov     ed_pos,eax
;Рисование закрашенных прямоугольников и их очистка
        movzx   ecx, word ed_shift_pos
        movzx   ebx, word ed_shift_pos_old
        mov     ed_shift_pos_old,ax
;проверка и рисование закрашенных областей
        cmp     ecx,ebx
        je      edit_box_mouse.m1_shem  ;движения не было ранее
        jb      edit_box_mouse.msmaller ;было движение ->
        cmp     ebx,eax
        ja      edit_box_mouse.m1_shem  ;было движение <-
        je      edit_box_mouse.mwigvag
        mov     ebp,ed_color
        call    edit_box_key.sh_cl_     ;очистить область c ed_pos ed_shift_pos_old
        jmp     edit_box_mouse.mwigvag

edit_box_mouse.msmaller:
        cmp     ebx,eax
        jb      edit_box_mouse.m1_shem
        mov     ebp,ed_color
        call    edit_box_key.sh_cl_
        jmp     edit_box_mouse.mwigvag

edit_box_mouse.m1_shem:
        mov     ebp,shift_color
        mov     ebx,ecx
        call    edit_box_key.sh_cl_
edit_box_mouse.mwigvag:
        and     word ed_flags,ed_shift_mcl
        jmp     edit_box.draw_cursor_text

edit_box_mouse.mleft:
        mov     eax,ed_pos
        cmp     eax,0
        jbe     edit_box_mouse.mwigvag
        dec     eax
        call    edit_box.check_offset
        push    eax
        movzx   ebx, word ed_shift_pos
        mov     ebp,shift_color
        call    edit_box_key.sh_cl_
        pop     eax
        jmp     edit_box_mouse.mdraw

edit_box_mouse.mright:
        mov     eax,ed_pos
        mov     ebx,ed_size
        cmp     eax,ebx
        jae     edit_box_mouse.mwigvag
        inc     eax
        call    edit_box.check_offset
        movzx   ebx, word ed_shift_pos
        mov     ebp,shift_color
        push    eax
        call    edit_box_key.sh_cl_
        pop     eax
        jmp     edit_box_mouse.mdraw


ed_struc_size=84
