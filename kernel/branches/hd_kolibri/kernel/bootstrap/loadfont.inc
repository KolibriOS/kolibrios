; LOAD FONTS I and II

        mov   [CURRENT_TASK],dword 1
        mov   [TASK_COUNT],dword 1
        mov   [TASK_BASE],dword TASK_DATA

	pushad
	push	eax
	mov		eax,char  - std_application_base_address
	call	file_system_lfn
	mov		eax,char2  - std_application_base_address
	call	file_system_lfn
	pop	eax
	popad


;        mov   esi,char
;        xor   ebx,ebx
;        mov   ecx,2560;26000
;        mov   edx,FONT_I
;        call  fs_RamdiskRead

;        mov   esi,char2
;        xor   ebx,ebx
;        mov   ecx,2560;26000
;        mov   edx,FONT_II
;        call  fs_RamdiskRead

        mov   esi,boot_fonts
        call  boot_log
