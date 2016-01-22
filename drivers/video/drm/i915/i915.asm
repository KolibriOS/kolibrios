
use32

db 'MENUET01'
dd 1
dd start
dd i_end
dd mem
dd mem
dd cmdline
dd path

SRV_CMDLINE equ 4

start:
       mov  eax, 68
       mov  ebx, 16
       mov  ecx, sz_display
       int  0x40
       test eax, eax
       jz   .load

       xor  ebx, ebx

       push ebx            ;.out_size
       push ebx            ;.output
       push 4              ;.inp_size
       push cmdline        ;.input
       push SRV_CMDLINE    ;.code
       push eax            ;.handle

       mov  eax, 68
       mov  ebx, 17
       mov  ecx, esp        ;[ioctl]
       int  0x40

       mov  eax, -1
       int  0x40

.load:
       xor  eax, eax
       mov  ecx, 1024
       mov  edi, path
	   cld
	   repne scasb
       dec  edi
       mov  [edi], dword '.dll'
       mov  [edi+4], al
       mov  eax, 68
       mov  ebx, 21
       mov  ecx, path
       mov  edx, cmdline
       int  0x40

       mov  eax, -1
       int  0x40

sz_display db 'DISPLAY',0

align 4
i_end:
cmdline  rb 256
path	 rb 1024
	 rb 16	   ; stack
mem:
