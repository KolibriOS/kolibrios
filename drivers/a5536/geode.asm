
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
       mov ebx, 16
	   mov ecx, sz_sound
       int 0x40
       test eax, eax
       jnz .exit

 	   mov eax, 68
	   mov ebx, 21
       mov ecx, sz_path
	   int 0x40

.exit:
	   mov eax, -1
	   int 0x40


sz_sound  db 'SOUND',0
sz_path   db '/rd/1/drivers/geode.drv',0

align 4
i_end:
rb 128
mem:
