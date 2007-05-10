macro wait time {
    mov     ebx,time
    mov     eax,5
    mcall
}

macro get_time_counter result {
    mov     eax,26
    mov     ebx,9
    mcall
    mov     result,eax
}

macro exit {
    or     eax,-1
    mcall
}