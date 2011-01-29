help_start:
    mov   ebx,help_file_name
    mov   esi,path
    mov   edi,start_parameter ;file_name
    call  copy_path
    call  draw_window
    mov   [use_patch],1
    mov  esi,help_file_name-40
    jmp  file_no_folder