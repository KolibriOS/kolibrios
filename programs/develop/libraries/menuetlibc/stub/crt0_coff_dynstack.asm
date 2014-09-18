CATCH_NULL_CALL = 0

format MS COFF
section '.text' code readable executable
public start
;EXTRN _edata
EXTRN ___menuet__app_param_area
EXTRN ___menuet__app_path_area
EXTRN ___crt1_startup
EXTRN ___memsize
start:
public ___menuet__app_header
public ___menuet__memsize
section '.A' code readable executable
___menuet__app_header:
 db 'MENUET01'
 dd 0x01
 dd do_start
; dd _edata
 dd 0
___menuet__memsize:
 dd ___memsize
 dd app_stack
 dd ___menuet__app_param_area
 dd ___menuet__app_path_area

do_start:
	push	68
	pop	eax
	push	11
	pop	ebx
	push	eax
	int	0x40
	pop	eax
	inc	ebx
	mov	ecx, 0x100000
	int	0x40
	lea	esp, [eax+ecx]
if CATCH_NULL_CALL
	mov	byte [0], 0xE9
	mov	dword [1], _libc_null_call-5
end if
 jmp ___crt1_startup

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
rb 0x100
app_stack:
