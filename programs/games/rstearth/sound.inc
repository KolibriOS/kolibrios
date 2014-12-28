;------------------------------------------------------------------------------
test_wav_file:
	mov	[sound_flag],1
	stdcall	_test_wav@4,dword [wav_for_test]	;letter_1a
	test	eax,eax
	jz	snd_test_wav_fail
	
;	or	eax,PCM_OUT
	or	eax,PCM_RING
	mov	[snd_format],eax

;	DEBUGF	1, "Test wav successful %x\n",eax

	call	find_data_offset
	
	mov	al,[sound_flag]
	test	al,al
	jnz	.exit
	
	mov	[sound_flag],2	; close thread
;--------------------------------------	
.exit:
	ret
;------------------------------------------------------------------------------
check_init_sound_busy:
	cmp	[init_sound_busy_flag],1
	jne	@f
	
	mcall	5,1
	mcall	68,1
	jmp	check_init_sound_busy
;--------------------------------------		
@@:
	mov	[init_sound_busy_flag],1
	ret
;------------------------------------------------------------------------------
initialize_sound_system:
	cmp	[sound_flag],1
	jne	sound_exit

	stdcall	_InitSound@4,version
	test	eax,eax
	jnz	snd_init_fail
	
;	DEBUGF	1, "Sound init successful %x\n",eax

	mov	eax,[version]
	and	eax,0xffff
	cmp	eax,SOUND_VERSION
	jne	snd_version_fail

;	DEBUGF	1, "Sound version successful %x\n",eax
;--------------------------------------	
	stdcall	_CreateBuffer@12,[snd_format],0,[hBuff_pointer]
	test	eax,eax
	jnz	snd_create_buffer_fail
	
;	DEBUGF	1, "Create Buffer successful %x\n",eax
	mov	eax,[hBuff_pointer]
	mov	ebx,[hBuff_buffer_size_pointer]
	stdcall	_GetBufferSize@8,[eax],ebx	;hBuff1_buffer_size
	
	test	eax,eax
	jnz	snd_get_buffer_size_fail
	
	mov	ebx,[hBuff_buffer_size_pointer]
	mov	eax,[ebx]
;	DEBUGF	1, "Get Buffer size successful size: %x\n",eax
	shr	eax,1
	mov	[ebx],eax
	
	mov	[init_sound_busy_flag],0
	ret
;------------------------------------------------------------------------------
sound_exit:
	mcall	-1
;------------------------------------------------------------------------------
find_data_offset:
	mov	esi,[wav_for_test]
;--------------------------------------
@@:
	cmp	[esi],dword 'data'
	je	@f

	inc	esi
	cmp	esi,[wav_for_test_end]
	jne	@b
	
;	DEBUGF	1, "find data offset fail\n"
	mov	[sound_flag],0
	ret
;--------------------------------------
@@:
	sub	esi,[wav_for_test]
	add	esi,4
	mov	[data_offset],esi
	ret
;------------------------------------------------------------------------------
snd_test_wav_fail:
;	DEBUGF	1, "Test wav fail %x\n",eax
	mov	[sound_flag],0
	ret
;------------------------------------------------------------------------------
snd_init_fail:
;	DEBUGF	1, "Sound init fail %x\n",eax
	jmp	disable_sound
;------------------------------------------------------------------------------
snd_version_fail:
;	DEBUGF	1, "Sound version fail %x\n",eax
	jmp	disable_sound
;------------------------------------------------------------------------------
snd_create_buffer_fail:
;	DEBUGF	1, "Create Buffer fail %x\n",eax
	jmp	disable_sound
;------------------------------------------------------------------------------
snd_get_buffer_size_fail:
;	DEBUGF	1, "Get Buffer size fail %x\n",eax
	jmp	disable_sound
;------------------------------------------------------------------------------
snd_play_buffer_fail:
;	DEBUGF	1, "Play Buffer fail\n"
	jmp	disable_sound
;------------------------------------------------------------------------------
snd_set_buffer_fail:
;	DEBUGF	1, "Set Buffer fail\n"
	jmp	disable_sound
;------------------------------------------------------------------------------
disable_sound:
	mov	[sound_flag],0
	jmp	sound_exit
;------------------------------------------------------------------------------
snd_background_music_thread_start:
	call	check_init_sound_busy
;	DEBUGF	1, "snd_background_music_thread_start\n"
	mov	[hBuff_pointer], dword hBuff1
	mov	[hBuff_buffer_size_pointer], dword hBuff1_buffer_size
	call	initialize_sound_system
;--------------------------------------	
.1:
	cmp	[music_flag],2
	je	sound_exit

	cmp	[music_flag],1
	je	.start
.2:	
	mcall	5,1
	mcall	68,1
	jmp	.1
;--------------------------------------		
.start:
	cmp	[init_sound_busy_flag],1
	je	.2

;	DEBUGF	1, "background_music Play Buffer start\n",eax
	stdcall _PlayBuffer@8, [hBuff1], 0
	test	eax,eax
	jnz	snd_play_buffer_fail
	
;	DEBUGF	1, "background_music Play Buffer successful EAX: %x\n",eax
;--------------------------------------		
.start_1:
	mov	eax,[background_music]
; check for correct data_offset
	push	eax
	add	eax,[data_offset]	
	sub	eax,4
	cmp	[eax],dword 'data'
	pop	eax
	je	@f

	mov	[wav_for_test],eax
	mov	ebx,eax
	add	ebx,1024
	mov	[wav_for_test_end],ebx
	call	find_data_offset
	cmp	[sound_flag],0
	jz	.1
;--------------------------------------
@@:
	add	eax,[data_offset]
	mov	ebx,[eax]
	add	eax,4
	mov	ebp,eax
	add	ebx,eax
;	DEBUGF	1, "start EAX: %x\n",eax
;--------------------------------------
.loop:	
	cmp	[music_flag],1
	je	@f

	stdcall _StopBuffer@4, [hBuff1]
;	DEBUGF	1, "Stop Buffer EAX: %x\n",eax
	jmp	.1
;--------------------------------------
@@:
; get event data from the driver
;	DEBUGF	1, "Get driver_notify_struct \n"
	push	ebx
	mcall	68,14,hBuff1_driver_notify_struct
	pop	ebx
;	DEBUGF	1, "Check to code \n"	
; check to code
	cmp	[hBuff1_driver_notify_struct], dword 0xFF000001
	jne 	@b
;	DEBUGF	1, "Check to stream \n"	
; check to stream
	mov	eax,[hBuff1_driver_notify_struct+8]
	cmp	eax,[hBuff1]
	jne	@b
;	DEBUGF	1, "Get buffer_offset \n"
; get buffer_offset
	mov	eax,[hBuff1_driver_notify_struct+12]
	mov	[hBuff1_buffer_offset],eax
;	DEBUGF	1, "src EBP: %x size EBX: %x\n",ebp,ebx
	stdcall _SetBuffer@16, [hBuff1], ebp, [hBuff1_buffer_offset], [hBuff1_buffer_size]	
	test	eax,eax
	jnz	snd_set_buffer_fail
	
;	DEBUGF	1, "Set Buffer successful EAX: %x\n",eax
	add	ebp,[hBuff1_buffer_size]
	add	ebp,[hBuff1_buffer_size]
	cmp	ebx,ebp
	jbe	.start_1
	
	sub	ebp,[hBuff1_buffer_size]
	jmp	.loop
;------------------------------------------------------------------------------
snd_kick_action:
	cmp	[sounds_flag],1
	jne	@f

	cmp	[snd_kick_flag],1
	je	@f

	mov	[snd_kick_flag],1
;--------------------------------------	
@@:
	ret
;------------------------------------------------------------------------------
snd_kick_action_thread_start:
	call	check_init_sound_busy
;	DEBUGF	1, "snd_kick_action_thread_start\n"
	mov	[hBuff_pointer], dword hBuff2
	mov	[hBuff_buffer_size_pointer], dword hBuff2_buffer_size
	call	initialize_sound_system
;--------------------------------------	
.1:
	cmp	[sounds_flag],2
	je	sound_exit

	cmp	[sounds_flag],0
	je	@f
	
	cmp	[snd_kick_flag],1
	je	.start
;--------------------------------------
@@:
	mcall	5,1
	mcall	68,1
	jmp	.1
;--------------------------------------
.start:
;	DEBUGF	1, "kick_action Play Buffer start\n"
	stdcall _PlayBuffer@8, [hBuff2], 0
	test	eax,eax
	jnz	snd_play_buffer_fail
	
;	DEBUGF	1, "kick_action Play Buffer successful EAX: %x\n",eax

	mov	eax,[sounds_sample]	;[stone_kick_sound]
; check for correct data_offset
	push	eax
	add	eax,[data_offset]	
	sub	eax,4
	cmp	[eax],dword 'data'
	pop	eax
	je	@f

	mov	[wav_for_test],eax
	mov	ebx,eax
	add	ebx,1024
	mov	[wav_for_test_end],ebx
	call	find_data_offset
	cmp	[sound_flag],0
	jz	.1
;--------------------------------------
@@:
	add	eax,[data_offset]
	mov	ebx,[eax]
	add	eax,4
	mov	ebp,eax
	add	ebx,eax
;	DEBUGF	1, "start EAX: %x\n",eax
;--------------------------------------
.loop:	
	cmp	[sounds_flag],1
	je	@f

	stdcall _StopBuffer@4, [hBuff2]
;	DEBUGF	1, "Stop Buffer EAX: %x\n",eax
	jmp	.1
;--------------------------------------
@@:
; get event data from the driver
;	DEBUGF	1, "Get driver_notify_struct \n"
	push	ebx
	mcall	68,14,hBuff2_driver_notify_struct
	pop	ebx
;	DEBUGF	1, "Check to code \n"	
; check to code
	cmp	[hBuff2_driver_notify_struct], dword 0xFF000001
	jne 	@b
;	DEBUGF	1, "Check to stream \n"	
; check to stream
	mov	eax,[hBuff2_driver_notify_struct+8]
	cmp	eax,[hBuff2]
	jne	@b
;	DEBUGF	1, "Get buffer_offset \n"
; get buffer_offset
	mov	eax,[hBuff2_driver_notify_struct+12]
	mov	[hBuff2_buffer_offset],eax
;	DEBUGF	1, "src EBP: %x size EBX: %x\n",ebp,ebx
	stdcall _SetBuffer@16, [hBuff2], ebp, [hBuff2_buffer_offset], [hBuff2_buffer_size]	
	test	eax,eax
	jnz	snd_set_buffer_fail
	
;	DEBUGF	1, "Set Buffer successful EAX: %x\n",eax
	add	ebp,[hBuff2_buffer_size]
	add	ebp,[hBuff2_buffer_size]
	cmp	ebx,ebp
	jbe	.exit
	
	sub	ebp,[hBuff2_buffer_size]
	jmp	.loop
;--------------------------------------	
.exit:
	stdcall _StopBuffer@4, [hBuff2]
	mov	[snd_kick_flag],0
	jmp	.1
;------------------------------------------------------------------------------