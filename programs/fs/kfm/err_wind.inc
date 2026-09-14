read_folder_error:
    mov   [error_pointer],read_folder_name    ;read_folder.name
    jmp   error_window
read_folder_1_error:
    mov   [error_pointer],read_folder_1_name   ;read_folder_1.name
    jmp   error_window
error_window:
    mov   [file_system_error],eax
.red:
    call  draw_error_window
.still:
    mcall SF_WAIT_EVENT
    cmp   eax,EV_REDRAW
    je    .red
    cmp   eax,EV_BUTTON
    je    .button
    jmp   .still
.button:
    mcall SF_TERMINATE_PROCESS
    jmp   .still
;---------------------------------------------------------------------
draw_error_window:
    mcall SF_REDRAW, SSF_BEGIN_DRAW
    mcall SF_CREATE_WINDOW, [error_window_x] ,[error_window_y], 0x03D22F36
    mcall SF_DRAW_TEXT, <10,30>,0x90ffffff,[error_pointer]
    mcall SF_DRAW_TEXT, <10,50>, ,error_type
    mcall SF_DRAW_NUMBER, 0x800a0000,[file_system_error],<154,50>,0x10ffffff
    mcall SF_REDRAW, SSF_END_DRAW
    ret
;---------------------------------------------------------------------
start_error_window_thread:
    mov   [file_system_error],eax
    mcall SF_THREAD_INFO, procinfo, -1
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
    mcall SF_THREAD_CONTROL, SSF_CREATE_THREAD,thread_start,thread_stack
    ret
;---------------------------------------------------------------------
thread_start:
;    mov   [error_pointer],start_file_data.name
    jmp    error_window.red
;---------------------------------------------------------------------