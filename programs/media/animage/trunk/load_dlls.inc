; load dll from file to memory
; IN
; eax - pointer to full dll path(path to dll +name_of_dll)
; OUT
; eax- handle to table of export of dll

load_dll:

    mov ecx,eax
    mov eax,68
    mov ebx,19
    int 0x40

    ret

