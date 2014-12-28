;---------------------------------------------------------------------
load_file:
	mov	esi,path
	mov	edi,file_name
	call	copy_file_path

	mov	[fileinfo.subfunction],dword 5
	mov	[fileinfo.size],dword 0
	mov	[fileinfo.return],dword file_info
	mcall	70,fileinfo
	test	eax,eax
	jnz	.error

	mov	[fileinfo.subfunction],dword 0

	mov	ecx,[file_info+32]
	mov	[fileinfo.size],ecx
	mov	[img_size],ecx
	
	mcall	68,12
	test	eax,eax
	jz	memory_get_error

	mov	[fileinfo.return],eax
	mov	[image_file],eax

	mcall	70,fileinfo
	test	eax,eax
	jnz	.error
	ret
.error:
	mov	[N_error],2
	mov	[error_type],eax
	jmp	button.exit
;---------------------------------------------------------------------
copy_file_path:
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	mov	esi,edi
	dec	esi
	std
@@:
	lodsb
	cmp	al,'/'
	jnz	@b
	mov	edi,esi
	add	edi,2
	mov	esi,ebx
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	ret
;---------------------------------------------------------------------
load_and_convert_current_icon_set:
	call	load_file
	call	convert_icons
	mov	eax,[raw_pointer]
	ret
;---------------------------------------------------------------------
; load and convert icons	
;	mov	ebx,npc_miku_kick_icons_file_name
;	call	load_and_convert_current_icon_set
;	mov	[npc_miku_kick_icons],eax
;	DEBUGF	1, "npc_miku_kick_icons_file_name: %s\n",npc_miku_kick_icons_file_name
;	DEBUGF	1, "size x: %x\n",[eax+4]
;	DEBUGF	1, "size x: %x\n",[eax+8]
;	DEBUGF	1, "common depth: %x\n",[eax+12]
;	DEBUGF	1, "channel depth: %x\n",[eax+16]:2
;	DEBUGF	1, "channel value: %x\n",[eax+18]:2
;	DEBUGF	1, "palette offset: %x\n",[eax+20]
;	DEBUGF	1, "palette size: %x\n",[eax+24]
;	DEBUGF	1, "data offset: %x\n",[eax+28]
;	DEBUGF	1, "data size: %x\n",[eax+32]
;	DEBUGF	1, "transparency offset: %x\n",[eax+36]
;	DEBUGF	1, "transparency size: %x\n",[eax+40]
;---------------------------------------------------------------------
load_and_convert_all_icons:
; load and convert font icons	
	mov	ebx,font_icons_file_name
	call	load_and_convert_current_icon_set
	mov	[font_icons],eax
;	DEBUGF	1, "font_icons: %x\n",eax

; load and convert base icons	
	mov	ebx,base_icons_file_name
	call	load_and_convert_current_icon_set
	mov	[base_icons],eax
;	DEBUGF	1, "base_icons: %x\n",eax

; load and convert red brick
	mov	ebx,red_brick_icons_file_name	
	call	load_and_convert_current_icon_set
	mov	[red_brick_icons],eax
;	DEBUGF	1, "red_brick_icons: %x\n",eax

; load and convert white brick
	mov	ebx,white_brick_icons_file_name	
	call	load_and_convert_current_icon_set
	mov	[white_brick_icons],eax
;	DEBUGF	1, "white_brick_icons: %x\n",eax

; load and convert miku
	mov	ebx,npc_miku_icons_file_name
	call	load_and_convert_current_icon_set
	mov	[npc_miku_icons],eax
;	DEBUGF	1, "npc_miku_icons: %x\n",eax

; load and convert death
	mov	ebx,npc_death_icons_file_name
	call	load_and_convert_current_icon_set
	mov	[npc_death_icons],eax
;	DEBUGF	1, "npc_death_icons: %x\n",eax

; load and convert skeleton
	mov	ebx,npc_skeleton_icons_file_name
	call	load_and_convert_current_icon_set
	mov	[npc_skeleton_icons],eax
;	DEBUGF	1, "npc_skeleton_icons: %x\n",eax

; load and convert ifrit
	mov	ebx,npc_ifrit_icons_file_name
	call	load_and_convert_current_icon_set
	mov	[npc_ifrit_icons],eax
;	DEBUGF	1, "npc_ifrit_icons: %x\n",eax

; load and convert barret
	mov	ebx,npc_barret_icons_file_name
	call	load_and_convert_current_icon_set
	mov	[npc_barret_icons],eax
;	DEBUGF	1, "npc_barret_icons: %x\n",eax

	ret
;---------------------------------------------------------------------
load_sound:
	call	load_file
	mov	eax,[fileinfo.return]
	ret
;---------------------------------------------------------------------
load_all_sound_files:
; load background music
	mov	ebx,background_music_file_name
	call	load_sound
	mov	[background_music],eax
	
; load stone kick sound
	mov	ebx,stone_kick_sound_file_name
	call	load_sound
	mov	[stone_kick_sound],eax
	
; load menu sound
	mov	ebx,menu_sound_file_name
	call	load_sound
	mov	[menu_sound],eax

	ret
;---------------------------------------------------------------------