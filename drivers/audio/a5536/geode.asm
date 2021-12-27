use32
db 'MENUET01'
dd 1, start, i_end, mem, mem, 0, selfpath

start:
;------ strcat(selfpath, '.drv')
		mov  eax, selfpath
	@@:
		inc  eax
		cmp  [eax], byte 0
		jne  @b
		mov  [eax], dword '.sys'

;------ writing some info
		mov  edx, info_msg 
		call debug_string
		mov  cl, 13 ; line break symbol
		int  40h

;------ init driver		
		mov  eax, 68
		mov  ebx, 16
		mov  ecx, sz_sound
		int  40h
		test eax, eax
		jnz  .exit

		mov  eax, 68
		mov  ebx, 21
		mov  ecx, selfpath
		int  40h

.exit:
		mov  eax, -1
		int  40h
		
debug_string:
		mov  eax,63
		mov  ebx,1
	@@:
		mov  cl,[edx]
		test cl,cl
		jz   @f
		int  40h
		inc  edx
		jmp  @b
	@@:
		ret

sz_sound  db 'SOUND',0
info_msg  db 'Trying to load the driver: '
selfpath  rb 4096

align 4
i_end:
rb 128
mem:
