;----------------------------------------------------------
;--------change size of memory which use program-----------
;----------------------------------------------------------
GetMemory:
	 pushad
	 mov eax,64
	 mov ebx,1
	 mov ecx,[Picture_SizeX]
	 mov edx,[Picture_SizeY]
	 imul ecx,edx
	 lea ecx,[ecx+ecx*2]
	 lea ecx,[ecx+ecx*4]		;(Picture_SizeX*Picture_SizeY*3)*5
	 add ecx,[ScreenPointer]
	 add ecx,(1200*1000)*3+50*(20*20*3)+500000+16000+0x4000
	 add ecx,[extended_memory]
	 mcall

	 test eax,eax
	 jz have_new_memory

	    mov esi,sound_havent_memory
	    call sound

	    jmp still

	 have_new_memory:
	 popad
	 ret
