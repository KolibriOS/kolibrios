CATCH_NULL_CALL = 0

format MS COFF
section '.text' code readable executable
public start
;EXTRN _edata
EXTRN ___menuet__app_param_area
EXTRN ___menuet__app_path_area
EXTRN ___crt1_startup
start:
public ___menuet__app_header
public ___menuet__memsize
section '.A' code readable executable
___menuet__app_header:
 db 'MENUET01'
 dd 0x01
if CATCH_NULL_CALL
 dd do_start
else
 dd ___crt1_startup
end if
; dd _edata
 dd 0
___menuet__memsize:
 dd 0x400000
 dd app_stack
 dd ___menuet__app_param_area
 dd ___menuet__app_path_area

if CATCH_NULL_CALL
do_start:
	mov	byte [0], 0xE9
	mov	dword [1], _libc_null_call-5
 call ___crt1_startup
; Handle exit if __crt1_startup returns (shouldn't happen)
 mov eax,-1
 int 0x40
end if

if CATCH_NULL_CALL
EXTRN ___libc_null_call

_libc_null_call:
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    call ___libc_null_call
    mov eax,-1
    int 0x40
end if

section '.bss' readable writeable
rd 0x20000
app_stack:
