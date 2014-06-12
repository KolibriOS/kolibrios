CATCH_NULL_CALL = 0

format ELF
section '.text' executable
public start
EXTRN edata
EXTRN __menuet__app_param_area
EXTRN __menuet__app_path_area
EXTRN __crt1_startup
start:
public __menuet__app_header
public __menuet__memsize
__menuet__app_header:
 db 'MENUET01'
 dd 0x01
if CATCH_NULL_CALL
 dd do_start
else
 dd __crt1_startup
end if
 dd edata
__menuet__memsize:
 dd 0x400000
 dd app_stack
 dd __menuet__app_param_area
 dd __menuet__app_path_area

if CATCH_NULL_CALL
do_start:
	mov	byte [0], 0xE9
	mov	dword [1], _libc_null_call-5
 call __crt1_startup
; Handle exit if __crt1_startup returns (shouldn't happen)
 mov eax,-1
 int 0x40
end if

if CATCH_NULL_CALL
EXTRN __libc_null_call

_libc_null_call:
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    call __libc_null_call
    mov eax,-1
    int 0x40
end if

section '.bss' writeable
rd 4096*4
app_stack:
