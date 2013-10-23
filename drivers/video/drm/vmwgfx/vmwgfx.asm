
use32

db 'MENUET01'
dd 1
dd start
dd i_end
dd mem
dd mem
dd cmdline
dd path

start:
           mov eax, 68
           mov ebx, 16
           mov ecx, sz_display
           int 0x40
           test eax, eax
           jnz .done             ; FIXME parse command line and
                                 ;       call service

	   xor eax, eax
	   mov ecx, 1024
	   mov edi, path
	   cld
	   repne scasb
	   dec edi
	   mov [edi], dword '.dll'
	   mov [edi+4], al
	   mov eax, 68
	   mov ebx, 21
	   mov ecx, path
	   mov edx, cmdline
	   int 0x40
.done:
	   mov eax, -1
	   int 0x40

sz_display db 'DISPLAY',0

align 4
i_end:
cmdline  rb 256
path	 rb 1024
	 rb 16	   ; stack
mem:
