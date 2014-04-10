draw_edit:
    pusha

    mov     eax , 13
    mov     ebx , 75 shl 16 + 116
    mov     ecx , 10 shl 16 + 15
    mov     edx , [sc.gui_face]
    int     0x40

    mov     eax , 38
    mov     ebx , 74 shl 16 + 191
    mov     ecx , 9  shl 16 + 9
    mov     edx , [sc.3d_light]
    int     0x40
    mov     ecx , 25 shl 16 + 25
    int     0x40
    mov     ecx , 8  shl 16 + 8
    mov     edx , [sc.gui_inframe]
    int     0x40
    mov     ecx , 26 shl 16 + 26
    int     0x40

    mov     eax , 38
    mov     ebx , 74 shl 16 + 74
    mov     ecx , 9  shl 16 + 25
    mov     edx , [sc.3d_light]
    int     0x40
    mov     ebx , 191 shl 16 + 191
    int     0x40
    mov     edx , [sc.gui_inframe]
    mov     ebx , 192 shl 16 + 192
    int     0x40
    mov     ebx , 73 shl 16 + 73
    int     0x40

    mov     eax,4
    mov     ecx,[sc.gui_intext]
    add     ecx,0xC0000000
    mov     edi,[sc.gui_face]

    mov     edx,[edit_cnt]
    cmp     edx,1
    jne     @f
    mov     edx,edit_win
    mov     ebx,115*65536 + 14
    jmp     draw_edit_exit
  @@:
    cmp     edx,2
    jne     @f
    mov     edx,edit_btn
    mov     ebx,115*65536 + 14
    jmp     draw_edit_exit
  @@:
    cmp     edx,3
    jne     @f
    mov     edx,edit_gui
    mov     ebx,112*65536 + 14
    jmp     draw_edit_exit
  @@:
    mov     edx,edit_cld
    mov     ebx,112*65536 + 14
  draw_edit_exit:
    int     0x40
    popa
ret
