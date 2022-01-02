format ELF
;include "public_stdcall.inc"

public _ksys_get_process_table
public _ksys_kill_process

section '.text' executable

_ksys_get_process_table:
;arg1 - pointer to information
;arg2 - pid
    mov eax,9
    mov ebx,[esp+4]
    mov ecx,[esp+8]
    int 0x40
    ret 8

_ksys_kill_process:
;arg - pid
    mov eax, 18
    mov ebx, 18
    mov ecx,[esp+4]
    int 0x40
    ret 4
