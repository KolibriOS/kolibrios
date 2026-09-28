;---------------------------------------------------------------------
;out:
;  esi = fb_left.folder_data + 304 *
;      * (fb_left.start_draw_line + fb_left.start_draw_cursor_line / fb_left.line_size_y)
align 4
calculate_left_data_1:
    movzx eax,[fb_left.start_draw_cursor_line]
    xor  edx,edx
    movzx  ebx,[fb_left.line_size_y]
    div  ebx
    mov  esi,[fb_left.start_draw_line]
    add  esi,eax
    imul esi,304
    add  esi,[fb_left.folder_data]
    ret
;---------------------------------------------------------------------
;out:
;  esi = fb_right.folder_data + 304 *
;      * (fb_right.start_draw_line + fb_right.start_draw_cursor_line / fb_right.line_size_y)
align 4
calculate_right_data_1:
    movzx eax,[fb_right.start_draw_cursor_line]
    xor  edx,edx
    movzx  ebx,[fb_right.line_size_y]
    div  ebx
    mov  esi,[fb_right.start_draw_line]
    add  esi,eax
    imul esi,304
    add  esi,[fb_right.folder_data]
    ret
;---------------------------------------------------------------------
align 4
proc mark_file_clear_all uses eax ecx edi, tmem:dword, tsize:dword
    xor  al,al
    mov  edi,[tmem]
    mov  ecx,edi
    add  ecx,[tsize]
    add  edi,32+299
    cld
@@:
    stosb
    add  edi,304-1
    cmp  edi,ecx
    jl   @b
    ret
endp
;---------------------------------------------------------------------