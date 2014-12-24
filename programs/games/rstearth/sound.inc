;------------------------------------------------------------------------------
; init.obj 
;	_InitSound@4		Инит,
;	_CreateBuffer@12	создать буфер,
;	_DestroyBuffer@4	удалить буфер
;
; sndgetsize.obj
;	_GetBufferSize@8	взять размер буфера,
;	_GetBufferFree@8	WTF???
;
; setbuf.obj
;	_SetBuffer@16		установить буфер,
;	_PlayBuffer@8		играть буфер,
;
; stopbuf.obj
;	_StopBuffer@4		остановить буфер,
;------------------------------------------------------------------------------
initialize_sound_system:
	mov	[sound_flag],1

	stdcall	_InitSound@4,version
	cmp	eax,-1
	je	snd_init_fail
	
;	DEBUGF	1, "Sound init successful %x\n",eax
	mov	eax,[version]
	and	eax,0xffff
	cmp	eax,SOUND_VERSION
	jne	snd_version_fail

;	DEBUGF	1, "Sound version successful %x\n",eax

	stdcall	_test_wav@4,dword [wav_for_test]	;letter_1a
	test	eax,eax
	jz	snd_test_wav_fail
	
	or	eax,PCM_OUT
	mov	[snd_format],eax

;	DEBUGF	1, "Test wav successful %x\n",eax

	call	find_data_offset
	
	mov	al,[sound_flag]
	test	al,al
	jz	.exit
	
	stdcall	_CreateBuffer@12,[snd_format],0,hBuff1
	cmp	eax,-1
	je	snd_create_buffer_fail
	
;	DEBUGF	1, "Create Buffer 1 successful %x\n",eax

	stdcall	_CreateBuffer@12,[snd_format],0,hBuff2
	cmp	eax,-1
	je	snd_create_buffer_fail

;	DEBUGF	1, "Create Buffer 1 successful %x\n",eax
;--------------------------------------
.exit:
	ret
;------------------------------------------------------------------------------
find_data_offset:
	mov	esi,[wav_for_test]	;letter_1a
;--------------------------------------
@@:
	cmp	[esi],dword 'data'
	je	@f

	inc	esi
	cmp	esi,[wav_for_test_end]	;letter_1a + letter_2b
	jne	@b
	
;	DEBUGF	1, "find data offset fail\n"
	mov	[sound_flag],0
	ret
;--------------------------------------
@@:
	sub	esi,[wav_for_test]	;letter_1a
	add	esi,4
	mov	[data_offset],esi
	ret
;------------------------------------------------------------------------------
wave_out:
; in:
; eax - adress of wav sample
; ebp - hBuff	; hardware buffer
	pusha
;	DEBUGF	1, "Wave out\n"
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
	jz	.exit
;--------------------------------------
@@:
	add	eax,[data_offset]
	mov	ebx,[eax]
	add	eax,4
;	DEBUGF	1, "src EAX: %x size EBX: %x\n",eax,ebx
;	stdcall	_WaveOut@12,[hBuff],eax,ebx
	stdcall	_WaveOut@12,ebp,eax,ebx
;	DEBUGF	1, "return EAX: %x\n",eax
	cmp	eax,-1
	popa
	je	snd_wave_out_fail
	
	ret
;--------------------------------------
.exit:
	popa
	ret
;------------------------------------------------------------------------------
snd_init_fail:
;	DEBUGF	1, "Sound init fail %x\n",eax
	mov	[sound_flag],0
	ret
;------------------------------------------------------------------------------
snd_version_fail:
;	DEBUGF	1, "Sound version fail %x\n",eax
	mov	[sound_flag],0
	ret
;------------------------------------------------------------------------------
snd_test_wav_fail:
;	DEBUGF	1, "Test wav fail %x\n",eax
	mov	[sound_flag],0
	ret
;------------------------------------------------------------------------------
snd_create_buffer_fail:
;	DEBUGF	1, "Create Buffer fail %x\n",eax
	mov	[sound_flag],0
	ret
;------------------------------------------------------------------------------
snd_wave_out_fail:
;	DEBUGF	1, "Wave Out fail\n"
	mov	[sound_flag],0
	ret
;------------------------------------------------------------------------------
snd_background_music_thread_start:
	cmp	[music_flag],1
	je	.start
	
	mcall	5,50
	mcall	68,1
	jmp	snd_background_music_thread_start
;--------------------------------------		
.start:
	mov	eax,[background_music]
;	call	wave_out
;	pusha
;	DEBUGF	1, "Wave out\n"
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
	jz	snd_background_music_thread_start	;.exit
;--------------------------------------
@@:
	add	eax,[data_offset]
	mov	ebx,[eax]
	add	eax,4
	mov	ebp,eax
;	DEBUGF	1, "src EAX: %x size EBX: %x\n",eax,ebx
;--------------------------------------
.loop:	
	cmp	[music_flag],0
	je	snd_background_music_thread_start
;	DEBUGF	1, "src EAX: %x size EBX: %x\n",eax,ebx
;	stdcall	_WaveOut@12,[hBuff],eax,ebx
	stdcall	_WaveOut@12,[hBuff1],ebp,8192
;	DEBUGF	1, "return EAX: %x\n",eax
	cmp	eax,-1
;	popa
	je	snd_wave_out_fail
	
	add	ebp,8192*2
	cmp	ebx,ebp
	jae	.start
	
	sub	ebp,8192
	jmp	.loop
;--------------------------------------
;.exit:
;	popa
;	jmp	snd_background_music_thread_start
;	mcall	-1
;------------------------------------------------------------------------------
snd_kick_action:
	cmp	[sounds_flag],1
	jne	@f

	cmp	[snd_kick_flag],1
	je	@f

	mcall	51,1,snd_kick_action_thread_start,snd_kick_action_thread_stack
;--------------------------------------	
@@:
	ret
;------------------------------------------------------------------------------
snd_kick_action_thread_start:
	mov	[snd_kick_flag],1
	mov	eax,[stone_kick_sound]
	mov	ebp,[hBuff2]
	call	wave_out
	mov	[snd_kick_flag],0
	mcall	-1
;------------------------------------------------------------------------------