macro wait time {
    mov     ebx,time
    mov     eax,5
    int     0x40
}

macro get_time_counter result {
    mov     eax,26
    mov     ebx,9
    int     0x40
    mov     result,eax
}

macro exit {
    mov     eax,-1
    int     0x40
}