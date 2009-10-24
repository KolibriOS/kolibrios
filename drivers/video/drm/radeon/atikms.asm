
use32

db 'MENUET01'
dd 1
dd start
dd i_end
dd mem
dd mem
dd 0
dd 0

start:
	   mov eax, 68
	   mov ebx, 21
           mov ecx, sz_kms
           mov edx, sz_mode
	   int 0x40

	   mov eax, -1
	   int 0x40

sz_kms   db '/rd/1/drivers/atikms.dll',0
sz_mode  db '-m 1024x768  -l/hd0/2/atikms.log',0

align 4
i_end:
rb 16
mem:
