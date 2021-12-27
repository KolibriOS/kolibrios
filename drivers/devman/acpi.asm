use32

format binary as ""

db 'MENUET01'
dd 1
dd start
dd i_end
dd mem
dd mem
dd 0
dd app_path

include '../../programs/macros.inc'
include '../../programs/proc32.inc'
include '../../programs/string.inc'

start:
	   stdcall string.concatenate, sz_dll, app_path
	   mov  eax, 68
	   mov  ebx, 21
	   mov  ecx, app_path
	   int  0x40

	   mov  eax, -1
	   int  0x40

align 4
app_path rb 2048
sz_dll db '.sys',0
i_end:
rb 128
mem:
