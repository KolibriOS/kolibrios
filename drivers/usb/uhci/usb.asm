
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
	   mov ecx, sz_usb
	   int 0x40

	   mov eax, -1
	   int 0x40

sz_usb	 db '/rd/1/drivers/usb.drv',0

align 4
i_end:
rb 128
mem:
