list_draw:
    call    list_draw_ground
    mov     eax,[edit_cnt]
    cmp     eax,1
    jne     @f
    call    list_draw_win_text
    call    list_draw_win_hex
    jmp     list_draw_exit
  @@:
    cmp     eax,2
    jne     @f
    call    list_draw_btn_text
    call    list_draw_btn_hex
    jmp     list_draw_exit
  @@:
    cmp     eax,3
    jne     @f
    call    list_draw_gui_text
    call    list_draw_gui_hex
    jmp     list_draw_exit
  @@:
    call    list_draw_sup_text
    call    list_draw_sup_hex
  list_draw_exit:
    ret

list_draw_ground:
    ;; draw list face
    push    edi
    mov     edi,12
    mov     eax,13
    mov     ebx, 9 shl 16 + 214
    mov     ecx, 32 shl 16 + 19
    mov     edx,[sc.gui_face]
    draw_list_face:
      int     0x40
      add     ecx, 20 shl 16 + 0
      dec     edi
      cmp     edi,0
    jne     draw_list_face
    pop     edi

    ;; draw list line
    push    edi
    mov     edi,13
    mov     eax,38
    mov     ebx, 9 shl 16 + 221
    mov     ecx, 31 shl 16 + 31
    mov     edx,[sc.gui_frame]
    draw_list_line:
      int     0x40
      add     ecx, 20 shl 16 + 20
      dec     edi
      cmp     edi,0
    jne     draw_list_line
    pop     edi

    ;; draw hex sybbol
    push    esi
    mov     esi,12
    mov     eax,4
    mov     ecx,[sc.gui_intext]
    add     ecx,0xC0000000
    mov     ebx,164*65536 + 38
    mov     edx,text_hex
    mov     edi,[sc.gui_face]
    draw_list_hex:
      push    esi
      int     0x40
      pop     esi
      add     ebx,20
      dec     esi
      cmp     esi,0
    jne     draw_list_hex
    pop     esi

    ;; draw vert line
    mov     eax, 38
    mov     ebx, 8 shl 16 + 8
    mov     ecx, 32 shl 16 + 270
    mov     edx,[sc.gui_frame]
    int     0x40
    mov     ebx, 142 shl 16 + 142
    int     0x40
    mov     ebx, 222 shl 16 + 222
    int     0x40
    ret

list_draw_win_text:
    mov     eax,4
    mov     ecx,[sc.gui_text]
    add     ecx,0xC0000000
    mov     edi,[sc.gui_face]

    mov     edx,text_frame
    mov     ebx,16*65536 + 38
    int     0x40

    mov     edx,text_inframe
    add     ebx,20
    int     0x40

    mov     edx,text_face
    add     ebx,20
    int     0x40

    mov     edx,text_inface
    add     ebx,20
    int     0x40

    mov     edx,text_border
    add     ebx,20
    int     0x40

    mov     edx,text_inborder
    add     ebx,20
    int     0x40

    mov     edx,text_wtext
    add     ebx,20
    int     0x40

    mov     edx,text_graytext
    add     ebx,20
    int     0x40

    mov     edx,text_title
    add     ebx,20
    int     0x40

    mov     edx,text_body
    add     ebx,20
    int     0x40

    mov     edx,text_reserved
    add     ebx,20
    int     0x40

    mov     edx,text_reserved
    add     ebx,20
    int     0x40
    ret

list_draw_win_hex:
    mov     eax,47
    mov     ebx,256+8 shl 16
    mov     esi,[sc.gui_intext]

    mov     ecx,[sc.win_frame]
    mov     edx,170*65536 + 38
    int     0x40

    mov     ecx,[sc.win_inframe]
    add     edx,20
    int     0x40

    mov     ecx,[sc.win_face]
    add     edx,20
    int     0x40

    mov     ecx,[sc.win_inface]
    add     edx,20
    int     0x40

    mov     ecx,[sc.win_border]
    add     edx,20
    int     0x40

    mov     ecx,[sc.win_inborder]
    add     edx,20
    int     0x40

    mov     ecx,[sc.win_text]
    add     edx,20
    int     0x40

    mov     ecx,[sc.win_graytext]
    add     edx,20
    int     0x40

    mov     ecx,[sc.win_title]
    add     edx,20
    int     0x40

    mov     ecx,[sc.win_body]
    add     edx,20
    int     0x40

    mov     ecx,[sc.res_var_a]
    add     edx,20
    int     0x40

    mov     ecx,[sc.res_var_b]
    add     edx,20
    int     0x40
    ret

list_draw_btn_text:
    mov     eax,4
    mov     ecx,[sc.gui_text]
    add     ecx,0xC0000000
    mov     edi,[sc.gui_face]

    mov     edx,text_frame
    mov     ebx,16*65536 + 38
    int     0x40

    mov     edx,text_inframe
    add     ebx,20
    int     0x40

    mov     edx,text_fcframe
    add     ebx,20
    int     0x40

    mov     edx,text_face
    add     ebx,20
    int     0x40

    mov     edx,text_inface
    add     ebx,20
    int     0x40

    mov     edx,text_fcface
    add     ebx,20
    int     0x40

    mov     edx,text_text
    add     ebx,20
    int     0x40

    mov     edx,text_intext
    add     ebx,20
    int     0x40

    mov     edx,text_fctext
    add     ebx,20
    int     0x40

    mov     edx,text_reserved
    add     ebx,20
    int     0x40

    mov     edx,text_reserved
    add     ebx,20
    int     0x40

    mov     edx,text_reserved
    add     ebx,20
    int     0x40
    ret

list_draw_btn_hex:
    mov     eax,47
    mov     ebx,256+8 shl 16
    mov     esi,[sc.gui_intext]

    mov     ecx,[sc.btn_frame]
    mov     edx,170*65536 + 38
    int     0x40

    mov     ecx,[sc.btn_inframe]
    add     edx,20
    int     0x40

    mov     ecx,[sc.btn_fcframe]
    add     edx,20
    int     0x40

    mov     ecx,[sc.btn_face]
    add     edx,20
    int     0x40

    mov     ecx,[sc.btn_inface]
    add     edx,20
    int     0x40

    mov     ecx,[sc.btn_fcface]
    add     edx,20
    int     0x40

    mov     ecx,[sc.btn_text]
    add     edx,20
    int     0x40

    mov     ecx,[sc.btn_intext]
    add     edx,20
    int     0x40

    mov     ecx,[sc.btn_fctext]
    add     edx,20
    int     0x40

    mov     ecx,[sc.res_var_c]
    add     edx,20
    int     0x40

    mov     ecx,[sc.res_var_d]
    add     edx,20
    int     0x40

    mov     ecx,[sc.res_var_e]
    add     edx,20
    int     0x40
    ret


list_draw_gui_text:
    mov     eax,4
    mov     ecx,[sc.gui_text]
    add     ecx,0xC0000000
    mov     edi,[sc.gui_face]

    mov     edx,text_frame
    mov     ebx,16*65536 + 38
    int     0x40

    mov     edx,text_inframe
    add     ebx,20
    int     0x40

    mov     edx,text_fcframe
    add     ebx,20
    int     0x40

    mov     edx,text_face
    add     ebx,20
    int     0x40

    mov     edx,text_inface
    add     ebx,20
    int     0x40

    mov     edx,text_fcface
    add     ebx,20
    int     0x40

    mov     edx,text_text
    add     ebx,20
    int     0x40

    mov     edx,text_intext
    add     ebx,20
    int     0x40

    mov     edx,text_fctext
    add     ebx,20
    int     0x40

    mov     edx,text_select
    add     ebx,20
    int     0x40

    mov     edx,text_shadow
    add     ebx,20
    int     0x40

    mov     edx,text_reserved
    add     ebx,20
    int     0x40
    ret

list_draw_gui_hex:
    mov     eax,47
    mov     ebx,256+8 shl 16
    mov     esi,[sc.gui_intext]

    mov     ecx,[sc.gui_frame]
    mov     edx,170*65536 + 38
    int     0x40

    mov     ecx,[sc.gui_inframe]
    add     edx,20
    int     0x40

    mov     ecx,[sc.gui_fcframe]
    add     edx,20
    int     0x40

    mov     ecx,[sc.gui_face]
    add     edx,20
    int     0x40

    mov     ecx,[sc.gui_inface]
    add     edx,20
    int     0x40

    mov     ecx,[sc.gui_fcface]
    add     edx,20
    int     0x40

    mov     ecx,[sc.gui_text]
    add     edx,20
    int     0x40

    mov     ecx,[sc.gui_intext]
    add     edx,20
    int     0x40

    mov     ecx,[sc.gui_fctext]
    add     edx,20
    int     0x40

    mov     ecx,[sc.gui_select]
    add     edx,20
    int     0x40

    mov     ecx,[sc.gui_shadow]
    add     edx,20
    int     0x40

    mov     ecx,[sc.res_var_f]
    add     edx,20
    int     0x40
    ret


list_draw_sup_text:
    mov     eax,4
    mov     ecx,[sc.gui_text]
    add     ecx,0xC0000000
    mov     edi,[sc.gui_face]

    mov     edx,text_p_frame
    mov     ebx,16*65536 + 38
    int     0x40

    mov     edx,text_p_face
    add     ebx,20
    int     0x40

    mov     edx,text_p_text
    add     ebx,20
    int     0x40

    mov     edx,text_m_frame
    add     ebx,20
    int     0x40

    mov     edx,text_m_face
    add     ebx,20
    int     0x40

    mov     edx,text_m_text
    add     ebx,20
    int     0x40

    mov     edx,text_h_frame
    add     ebx,20
    int     0x40

    mov     edx,text_h_face
    add     ebx,20
    int     0x40

    mov     edx,text_h_text
    add     ebx,20
    int     0x40

    mov     edx,text_3dlight
    add     ebx,20
    int     0x40

    mov     edx,text_3ddark
    add     ebx,20
    int     0x40

    mov     edx,text_3dface
    add     ebx,20
    int     0x40
    ret

list_draw_sup_hex:
    mov     eax,47
    mov     ebx,256+8 shl 16
    mov     esi,[sc.gui_intext]

    mov     ecx,[sc.panel_frame]
    mov     edx,170*65536 + 38
    int     0x40

    mov     ecx,[sc.panel_body]
    add     edx,20
    int     0x40

    mov     ecx,[sc.panel_text]
    add     edx,20
    int     0x40

    mov     ecx,[sc.menu_frame]
    add     edx,20
    int     0x40

    mov     ecx,[sc.menu_body]
    add     edx,20
    int     0x40

    mov     ecx,[sc.menu_text]
    add     edx,20
    int     0x40

    mov     ecx,[sc.hint_frame]
    add     edx,20
    int     0x40

    mov     ecx,[sc.hint_body]
    add     edx,20
    int     0x40

    mov     ecx,[sc.hint_text]
    add     edx,20
    int     0x40

    mov     ecx,[sc.3d_light]
    add     edx,20
    int     0x40

    mov     ecx,[sc.3d_dark]
    add     edx,20
    int     0x40

    mov     ecx,[sc.3d_face]
    add     edx,20
    int     0x40
    ret