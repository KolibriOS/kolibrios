read_folder_error:
    mov   [error_pointer],read_folder_name    ;read_folder.name
    jmp   error_window
read_folder_1_error:
    mov   [error_pointer],read_folder_1_name   ;read_folder_1.name
    jmp   error_window
initiation_error:
    mov   [error_pointer],ini_file_name
    jmp   error_window
error_window:
    mov   [file_system_error],eax
.red:
    call  draw_error_window
.still:
    mcall 10
    cmp   eax,1
    je    .red
    cmp   eax,3
    je    .button
    jmp   .still
.button:
    mcall -1
    jmp   .still
;---------------------------------------------------------------------
draw_error_window:
    mcall 12, 1
    mcall 0,[error_window_x] ,[error_window_y], 0x03D22F36
    mcall 4,<10,30>,0x90ffffff,[error_pointer]
    mcall 4,<10,50>, ,error_type
    mcall 47,0xa0000,[file_system_error],<148,50>,0x10ffffff
    mcall 12, 2
    ret
;---------------------------------------------------------------------
start_error_window_thread:
    mov   [file_system_error],eax
    mcall 9, procinfo, -1
    mov   eax,[ebx+46]
    shr   eax,1
    add   eax,[ebx+38]
    sub   eax,ERR_WINH/2
    mov   [error_window_y+2],ax
    mov   eax,[ebx+42]
    shr   eax,1
    add   eax,[ebx+34]
    sub   eax,ERR_WINW/2
    mov   [error_window_x+2],ax
    mcall 51,1,thread_start,thread_stack
    ret
;---------------------------------------------------------------------
thread_start:
;    mov   [error_pointer],start_file_data.name
    jmp    error_window.red
;---------------------------------------------------------------------