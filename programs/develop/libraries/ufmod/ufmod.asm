; UFMOD.ASM
; ---------
; uFMOD public source code release. Provided as-is.

SOUND_VERSION     equ 100h ; required Infinity sound driver version
FSOUND_Block      equ 10
FSOUND_BlockSize  equ 1024 ; 1 << FSOUND_Block

if DEBUG

; Debug messages:
sDBGMSG1 db "uFMOD: XM track loaded",13,10,0
sDBGMSG2 db "uFMOD: Infinity driver loaded",13,10,0
sDBGMSG3 db "uFMOD: Buffer created",13,10,0
sDBGMSG4 db "uFMOD: Sound buffer destroyed",13,10,0
sDBGMSG5 db "uFMOD: Infinity version: ",0

; DEBUG board: print a string.
DBG_print_s:
; EDX = msg (trailing 0 is required!)
	pushad
DBG_print_s_loop:
	mov eax,63
	mov ebx,1
	mov cl,[edx]
	test cl,cl
	jz DBG_print_s_R
	int 40h
	inc edx
	jmp DBG_print_s_loop
DBG_print_s_R:
	popad
	ret

; DEBUG board: print the hex value in EAX.
DBG_print_x:
; EAX = val
	pushad
	mov esi,eax
	mov edx,OFFSET MixBuf
	mov ecx,7
	mov DWORD PTR [edx+8],0A0Dh
print_eax_loop:
	mov eax,esi
	and al,0Fh
	cmp al,10
	sbb al,69h
	das
	mov [edx+ecx],al
	shr esi,4
	dec ecx
	jns print_eax_loop
	call DBG_print_s
	popad
	ret
endif ; DEBUG

if RAMP_STRONG
	volumerampsteps   equ 128
	volumeramps_pow   equ 7
endif

if RAMP_WEAK
	volumerampsteps   equ 16
	volumeramps_pow   equ 4
endif

if RAMP_NONE
	volumerampsteps   equ 64
	volumeramps_pow   equ 6
endif

XM_MEMORY                 equ 1
XM_FILE                   equ 2
XM_NOLOOP                 equ 8
XM_SUSPENDED              equ 16
FMUSIC_ENVELOPE_SUSTAIN   equ 2
FMUSIC_ENVELOPE_LOOP      equ 4
FMUSIC_FREQ               equ 1
FMUSIC_VOLUME             equ 2
FMUSIC_PAN                equ 4
FMUSIC_TRIGGER            equ 8
FMUSIC_VOLUME_OR_FREQ     equ 3
FMUSIC_VOLUME_OR_PAN      equ 6
FMUSIC_VOL_OR_FREQ_OR_TR  equ 11
FMUSIC_FREQ_OR_TRIGGER    equ 9
NOT_FMUSIC_TRIGGER        equ 0F7h
NOT_FMUSIC_TRIGGER_OR_FRQ equ 0F6h

; FMUSIC_XMCOMMANDS enum:
FMUSIC_XM_PORTAUP         equ 1
FMUSIC_XM_PORTADOWN       equ 2
FMUSIC_XM_PORTATO         equ 3
FMUSIC_XM_VIBRATO         equ 4
FMUSIC_XM_PORTATOVOLSLIDE equ 5
FMUSIC_XM_VIBRATOVOLSLIDE equ 6
FMUSIC_XM_TREMOLO         equ 7
FMUSIC_XM_SETPANPOSITION  equ 8
FMUSIC_XM_SETSAMPLEOFFSET equ 9
FMUSIC_XM_VOLUMESLIDE     equ 10
FMUSIC_XM_PATTERNJUMP     equ 11
FMUSIC_XM_SETVOLUME       equ 12
FMUSIC_XM_PATTERNBREAK    equ 13
FMUSIC_XM_SPECIAL         equ 14
FMUSIC_XM_SETSPEED        equ 15
FMUSIC_XM_SETGLOBALVOLUME equ 16
FMUSIC_XM_GLOBALVOLSLIDE  equ 17
FMUSIC_XM_KEYOFF          equ 20
FMUSIC_XM_PANSLIDE        equ 25
FMUSIC_XM_MULTIRETRIG     equ 27
FMUSIC_XM_TREMOR          equ 29
FMUSIC_XM_EXTRAFINEPORTA  equ 33

; FMUSIC_XMCOMMANDSSPECIAL enum:
FMUSIC_XM_FINEPORTAUP      equ 1
FMUSIC_XM_FINEPORTADOWN    equ 2
FMUSIC_XM_SETGLISSANDO     equ 3
FMUSIC_XM_SETVIBRATOWAVE   equ 4
FMUSIC_XM_SETFINETUNE      equ 5
FMUSIC_XM_PATTERNLOOP      equ 6
FMUSIC_XM_SETTREMOLOWAVE   equ 7
FMUSIC_XM_SETPANPOSITION16 equ 8
FMUSIC_XM_RETRIG           equ 9
FMUSIC_XM_NOTECUT          equ 12
FMUSIC_XM_NOTEDELAY        equ 13
FMUSIC_XM_PATTERNDELAY     equ 14

if AC97SND_ON

	file_read:
	; buf  in EAX
	; size in EDX
		push ebx
		push esi
		push edi
		push ebp
		xchg eax,edi
	file_read_begin:
		test edx,edx
		jg file_read_chk_cache
	file_read_done:
		pop ebp
		pop edi
		pop esi
		pop ebx
		ret
		; *** CHECK IN THE CACHE
	file_read_chk_cache:
		mov ebp,OFFSET file_struct
		mov esi,[ebp-24]
		sub esi,[ebp+28] ; cache_offset
		js file_read_cache_done
		mov ecx,8192
		sub ecx,esi
		jle file_read_cache_done
		add esi,OFFSET MixBuf
		sub edx,ecx
		jns file_read_do_cache
		add ecx,edx
	file_read_do_cache:
		add [ebp-24],ecx
		rep movsb
		test edx,edx
		jle file_read_done ; data read from the cache (no need to access the FS)
	file_read_cache_done:
		; *** FS BATCH READ OPERATION
		mov eax,[ebp-24]
		mov ecx,edx
		add ecx,eax
		and ecx,0FFFFE000h
		sub ecx,eax
		jle file_read_fs_done ; Too few data requested for a FS batch operation
		sub edx,ecx
		mov [ebp+4],eax  ; file offset
		mov [ebp+12],ecx ; NumberOfBytesToRead
		mov [ebp+16],edi ; lpBuffer
		mov ebx,ebp
		add edi,ecx
		push 70
		add [ebp-24],ecx
		pop eax
		int 40h
	file_read_fs_done:
		; *** UPDATE THE CACHE
		mov ecx,[ebp-24]
		and ecx,0FFFFE000h
		mov [ebp+4],ecx           ; file offset
		mov [ebp+28],ecx          ; cache_offset
		mov DWORD PTR [ebp+12],8192 ; NumberOfBytesToRead
		mov DWORD PTR [ebp+16],OFFSET MixBuf ; lpBuffer
		mov ebx,ebp
		push 70
		pop eax
		int 40h
		jmp file_read_begin

	if INFO_API_ON
		PUBLIC _uFMOD_GetTitle
		_uFMOD_GetTitle:
			mov eax,OFFSET szTtl
			ret
	endif

else
		uFMOD_mem dd mem_open, mem_read
	if XM_FILE_ON
		uFMOD_fs  dd file_open, file_read
	endif

	szInfinity db "INFINITY",0

	if JUMP_TO_PAT_ON

		; Jump to the given pattern
	PUBLIC _uFMOD_Jump2Pattern
	_uFMOD_Jump2Pattern:
		mov eax,[esp+4]
		mov ecx,OFFSET _mod+36
		movzx eax,ax
		and DWORD PTR [ecx+FMUSIC_MODULE.nextrow-36],0
		cmp ax,[ecx+FMUSIC_MODULE.numorders-36]
		sbb edx,edx
		and eax,edx
		mov [ecx+FMUSIC_MODULE.nextorder-36],eax
		ret
	endif

	if VOL_CONTROL_ON

		; Set global volume [0: silence, 25: max. volume]
		vol_scale dw 1     ; -45 dB
	        	  dw 130   ; -24
			  dw 164   ; -23
		          dw 207   ; -22
		          dw 260   ; -21
		          dw 328   ; -20
	        	  dw 413   ; -19
		          dw 519   ; -18
		          dw 654   ; -17
	        	  dw 823   ; -16
		          dw 1036  ; -15
		          dw 1305  ; -14
	        	  dw 1642  ; -13
		          dw 2068  ; -12
		          dw 2603  ; -11
	        	  dw 3277  ; -10
		          dw 4125  ; -9
		          dw 5193  ; -8
		          dw 6538  ; -7
		          dw 8231  ; -6
	        	  dw 10362 ; -5
		          dw 13045 ; -4
		          dw 16423 ; -3
	        	  dw 20675 ; -2
		          dw 26029 ; -1
		          dw 32768 ; 0 dB
		PUBLIC _uFMOD_SetVolume
		_uFMOD_SetVolume:
			mov eax,[esp+4]
			cmp eax,25
			jna get_vol_scale
			push 25
			pop eax
		get_vol_scale:
			mov ax,[vol_scale+eax*2]
			mov [ufmod_vol],eax
			ret
	endif

	if PAUSE_RESUME_ON

			; Pause the currently playing song.
		PUBLIC _uFMOD_Pause
		_uFMOD_Pause:
			mov al,1
			jmp $+4

			; Resume the currently paused song.
		PUBLIC _uFMOD_Resume
		_uFMOD_Resume:
			xor eax,eax
			mov BYTE PTR [ufmod_pause_],al
			ret
	endif

	if INFO_API_ON

		; Return currently playing signal stats:
		;    lo word : RMS volume in R channel
		;    hi word : RMS volume in L channel
		PUBLIC _uFMOD_GetStats
		_uFMOD_GetStats:
			push 4
			jmp _uFMOD_InfoApi

		; Return currently playing row and order:
		;    lo word : row
		;    hi word : order
		PUBLIC _uFMOD_GetRowOrder
		_uFMOD_GetRowOrder:
			push 8
			jmp _uFMOD_InfoApi

		; Return the time in milliseconds since the song was started.
		PUBLIC _uFMOD_GetTime
		_uFMOD_GetTime:
			push 0
		_uFMOD_InfoApi:
			pop edx
			mov eax,[time_ms+edx]
			ret
	endif

	; ***********************
	; * XM_MEMORY CALLBACKS *
	; ***********************
	mem_read:
	; buf in EAX
	; size in EDX
		push edi
		push esi
		xchg eax,edi ; buf
		mov esi,OFFSET mmf
		lodsd
		mov ecx,edx
		add edx,[esi]
		cmp edx,eax
		jl copy
		sub eax,[esi]
		xchg eax,ecx
	copy:
		test ecx,ecx
		jle mem_read_R
		lodsd
		add eax,[esi]
		mov [esi-4],edx
	mem_do_copy:
		mov dl,[eax]
		mov [edi],dl
		inc eax
		inc edi
		dec ecx
		jnz mem_do_copy
	mem_read_R:
		pop esi
		pop edi
	if INFO_API_ON
		PUBLIC _uFMOD_GetTitle
		_uFMOD_GetTitle:
			mov eax,OFFSET szTtl
	endif
	mem_open:
		ret

	; *********************
	; * XM_FILE CALLBACKS *
	; *********************
	if XM_FILE_ON
		file_open:
		; pszName in ESI
			; Prepare the FILE struct for subsecuent I/O:
			lea eax,[ebp+8]  ; file_struct
			xor edx,edx
			mov [eax],edx    ;  +0 subfunction: 0 = read
			mov [eax+8],edx  ;  +8 Reserved
			                 ; +12 NumberOfBytesToRead
			                 ; +16 lpBuffer
			push -1
			push 1
			mov [eax+20],dl  ; +20 db 0
			mov [eax+21],esi ; +21 lpFileName
			pop DWORD PTR [eax+28] ; cache_offset
			pop DWORD PTR [eax-28] ; [mmf] = maximum size
			ret

		file_read:
		; buf  in EAX
		; size in EDX
			push ebx
			push esi
			push edi
			push ebp
			xchg eax,edi
		file_read_begin:
			test edx,edx
			jg file_read_chk_cache
		file_read_done:
			pop ebp
			pop edi
			pop esi
			pop ebx
			ret
			; *** CHECK IN THE CACHE
		file_read_chk_cache:
			mov ebp,OFFSET file_struct
			mov esi,[ebp-24]
			sub esi,[ebp+28] ; cache_offset
			js file_read_cache_done
			mov ecx,8192
			sub ecx,esi
			jle file_read_cache_done
			add esi,OFFSET MixBuf
			sub edx,ecx
			jns file_read_do_cache
			add ecx,edx
		file_read_do_cache:
			add [ebp-24],ecx
			rep movsb
			test edx,edx
			jle file_read_done ; data read from the cache (no need to access the FS)
		file_read_cache_done:
			; *** FS BATCH READ OPERATION
			mov eax,[ebp-24]
			mov ecx,edx
			add ecx,eax
			and ecx,0FFFFE000h
			sub ecx,eax
			jle file_read_fs_done ; Too few data requested for a FS batch operation
			sub edx,ecx
			mov [ebp+4],eax  ; file offset
			mov [ebp+12],ecx ; NumberOfBytesToRead
			mov [ebp+16],edi ; lpBuffer
			mov ebx,ebp
			add edi,ecx
			push 70
			add [ebp-24],ecx
			pop eax
			int 40h
		file_read_fs_done:
			; *** UPDATE THE CACHE
			mov ecx,[ebp-24]
			and ecx,0FFFFE000h
			mov [ebp+4],ecx           ; file offset
			mov [ebp+28],ecx          ; cache_offset
			mov DWORD PTR [ebp+12],8192 ; NumberOfBytesToRead
			mov DWORD PTR [ebp+16],OFFSET MixBuf ; lpBuffer
			mov ebx,ebp
			push 70
			pop eax
			int 40h
			jmp file_read_begin
	endif

endif ; AC97SND_ON = 0

uFMOD_lseek:
; pos  in EAX
; org  in ECX
; !org in Z
	mov edx,OFFSET mmf+4
	jz mem_ok
	add eax,[edx]
mem_ok:
	test eax,eax
	js mem_seek_R
	cmp eax,[edx-4]
	ja mem_seek_R
	mov [edx],eax
mem_seek_R:
	ret

; Dynamic memory allocation
alloc:
; EAX: how many bytes to allocate
	add eax,3
	and eax,-4
	jle alloc_error2
	mov ecx,OFFSET ufmod_heap
alloc_lookup:
	cmp DWORD PTR [ecx],0
	je do_alloc
	mov ecx,[ecx]
	cmp [ecx+4],eax
	jl alloc_lookup
	sub [ecx+4],eax
	mov eax,[ecx+4]
	lea eax,[eax+ecx+8]
	ret
do_alloc:
	add eax,8
	push ebx
	push edi
	mov ebx,eax
	add ebx,8191
	neg eax
	and ebx,-8192
	push ecx
	add eax,ebx
	xchg eax,edi
	push 12
	push 68
	mov ecx,ebx
	pop eax
	pop ebx
	int 40h
	; Test for error condition
	test eax,eax
	pop ebx
	mov edx,edi ; free space
	jz alloc_error1
	mov [ebx],eax
	mov edi,eax
	lea eax,[eax+edx+8]
	mov [edi+4],edx
	pop edi
	pop ebx
	ret
alloc_error1:
	pop edi
	pop ebx
alloc_error2:
	xor eax,eax
	pop edx ; EIP
	pop ebx
	leave
_alloc_R:
	ret

; Starts playing a song.
PUBLIC _uFMOD_LoadSong
_uFMOD_LoadSong:

	; *** FREE PREVIOUS TRACK, IF ANY
	call _uFMOD_StopSong

	if AC97SND_ON
		mov ecx,[esp+4]
		push ebx
		push esi
		push edi
		push ebp
		mov ebp,OFFSET uFMOD_fopen
		; Prepare the FILE struct for subsecuent I/O:
		lea eax,[ebp+8]  ; file_struct
		xor edx,edx
		mov [eax],edx    ;  +0 subfunction: 0 = read
		mov [eax+8],edx  ;  +8 Reserved
		                 ; +12 NumberOfBytesToRead
		                 ; +16 lpBuffer
		mov [eax+20],dl  ; +20 db 0
		mov [eax+21],ecx ; +21 lpFileName
		push -1
		push 1
		mov DWORD PTR [ebp+4],OFFSET file_read ; uFMOD_fread
		pop DWORD PTR [eax+28]          ; cache_offset
		mov [eax-24],edx
		pop DWORD PTR [eax-28]          ; [mmf] = maximum size
	else
		mov eax,[esp+8]  ; param
		mov ecx,[esp+12] ; fdwSong
		mov edx,[esp+4]  ; lpXM
		test edx,edx
		jz _alloc_R
		; *** SET I/O CALLBACKS
		push ebx
		push esi
		push edi
		push ebp
		mov ebp,OFFSET uFMOD_fopen
		mov [ebp-20],eax ; mmf
		xor eax,eax
		mov [ebp-16],eax ; mmf+4
		test cl,XM_MEMORY
		mov esi,OFFSET uFMOD_mem
	if XM_FILE_ON
		jnz set_callbacks
		test cl,XM_FILE
		lea esi,[esi+(uFMOD_fs-uFMOD_mem)]
	endif
		jz goto_StopSong
	set_callbacks:
	if NOLOOP_ON
		test cl,XM_NOLOOP
		setnz [ebp-24] ; ufmod_noloop
	endif
	if PAUSE_RESUME_ON
		and cl,XM_SUSPENDED
		mov [ebp-23],cl ; ufmod_pause_
	endif
		mov edi,ebp ; uFMOD_fopen
		movsd
		movsd
		mov esi,edx ; uFMOD_fopen:lpXM <= ESI
	if VOL_CONTROL_ON
		cmp [ebp-4],eax ; ufmod_vol
		jne play_vol_ok
		mov WORD PTR [ebp-4],32768
	play_vol_ok:
	endif
		xor edi,edi
		; *** INIT THE INFINITY DRIVER
		lea eax,[edi+68]
		lea ebx,[edi+16]
		mov ecx,OFFSET szInfinity
		int 40h
		test eax,eax
		mov [hSound],eax
		jz goto_StopSong
	if DEBUG
		mov edx,OFFSET sDBGMSG2
		call DBG_print_s
	endif
		; *** CHECK THE DRIVER VERSION
		push edi ; ver = 0
		push esp ; &ver
		mov edx,esp
		push 4   ; .out_size
		push edx ; .output = &&ver
		push edi ; .inp_size
		push edi ; .input
		push edi ; .code   = SRV_GETVERSION
		push eax ; .handle = [hSound]
		lea ebx,[edi+17]
		lea eax,[edi+68]
		mov ecx,esp
		int 40h
		add esp,28
		pop eax ; ver
	if DEBUG
		mov edx,OFFSET sDBGMSG5
		call DBG_print_s
		call DBG_print_x
	endif
		shr eax,16
		cmp eax,SOUND_VERSION
		ja _uFMOD_StopSong+4 ; obsolete program version (Hint: try adjusting SOUND_VERSION!)
		; *** ALLOCATE A HEAP OBJECT
		lea eax,[edi+68]
		lea ebx,[edi+11]
		int 40h
		test eax,eax
		jz goto_StopSong
		; *** LOAD THE TRACK
		mov [ebp-12],esi ; mmf+8 <= pMem
	if XM_FILE_ON
		call DWORD PTR [ebp] ; uFMOD_fopen
	endif

	endif ; AC97SND_ON = 0

	call LoadXM
	test eax,eax
goto_StopSong:
	jz _uFMOD_StopSong+4
if DEBUG
	mov edx,OFFSET sDBGMSG1
	call DBG_print_s
endif

	if AC97SND_ON
	else
		xor edi,edi
		; *** CREATE THE PCM BUFFER
		push edi        ; size (default is 64Kb)
		push PCM_format ; format: 16-bit / stereo / sampling rate
		mov edx,esp
		push OFFSET hBuff
		mov eax,esp
		push 4              ; .out_size
		push eax            ; .output = &&hBuff
		push 8              ; .inp_size
		push edx            ; .input
		push 1              ; .code   = SND_CREATE_BUFF
		push DWORD PTR [hSound] ; .handle
		lea eax,[edi+68]
		lea ebx,[edi+17]
		mov ecx,esp
		int 40h
		pop esi ; <- hSound
		add esp,32
		test eax,eax
		jnz _uFMOD_StopSong+4 ; buffer not created
	if DEBUG
		mov edx,OFFSET sDBGMSG3
		call DBG_print_s
	endif
		xchg eax,esi ; return the driver handle
	endif ; AC97SND_ON = 0

	; *** ENABLE PCM OUTPUT
	mov [SW_Exit],eax
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

; Stop the currently playing song, if any, and free all resources allocated for that song.
PUBLIC _uFMOD_StopSong
_uFMOD_StopSong:
	push ebx
	push esi
	push edi
	push ebp
; _uFMOD_StopSong+4
	xor edi,edi
	mov ebp,OFFSET ufmod_heap
	; *** DISABLE PCM OUTPUT
	mov [ebp+16],edi ; SW_Exit

	if AC97SND_ON
	else
		; *** STOP AND DESTROY THE PCM BUFFER
		mov eax,[ebp+12] ; hBuff
		test eax,eax
		jz SND_buffer_free
		push eax             ; buffer
		mov edx,esp
		push edi             ; .out_size
		push edi             ; .output
		push 4               ; .inp_size
		push edx             ; .input
		push 11              ; .code   = SND_STOP
		push DWORD PTR [ebp+8]   ; .handle = [hSound]
		lea eax,[edi+68]
		lea ebx,[edi+17]
		mov ecx,esp
		int 40h
		mov DWORD PTR [esp+4],2  ; .code = SND_DESTROY_BUFF
		lea eax,[edi+68]
		int 40h
		add esp,28
	if DEBUG
		mov edx,OFFSET sDBGMSG4
		call DBG_print_s
	endif
	SND_buffer_free:
		mov [ebp+12],edi ; hBuff
	endif ; AC97SND_ON = 0

	; *** FREE THE HEAP
	mov esi,[ebp]    ; ufmod_heap
heapfree:
	test esi,esi
	jz free_R
	mov ecx,esi
	mov esi,[esi]
	lea eax,[edi+68]
	lea ebx,[edi+13]
	int 40h
	jmp heapfree
free_R:
	xor eax,eax

	if AC97SND_ON
	else
		if INFO_API_ON
			; *** CLEAR THE INFO API DATA
			lea ecx,[eax+4]
			mov edi,OFFSET time_ms
			rep stosd
		endif
	endif ; AC97SND_ON = 0

	mov DWORD PTR [ebp],eax ; ufmod_heap
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

PUBLIC _uFMOD_WaveOut
_uFMOD_WaveOut:
	push edi
	push ebp
	xor edi,edi

	if AC97SND_ON
		; *** PCM OUTPUT ENABLED?
		cmp DWORD PTR [SW_Exit],edi
		lea eax,[edi+1] ; return error if output not enabled
		je _uFMOD_WaveOut_R
		; *** COMPUTE THE NUMBER OF FREE BLOCKS AVAILABLE
		lea ecx,[esp+12]    ; &hBuff
		push edi            ; space = 0
		mov edx,esp
		push 4              ; .out_size
		push edx            ; .output = &space
		push 4              ; .inp_size
		push ecx            ; .input
		push 17             ; .code   = SND_GETFREESPACE
		mov ecx,[ecx]
		push DWORD PTR [hSound] ; .handle
		mov [hBuff],ecx
	else
		mov ebp,OFFSET hSound
		; *** PCM OUTPUT ENABLED?
		cmp [ebp+8],edi ; SW_Exit
		lea eax,[edi+1] ; return error if output not enabled
		je _uFMOD_WaveOut_R
		; *** COMPUTE THE NUMBER OF FREE BLOCKS AVAILABLE
		push edi         ; space = 0
		mov edx,esp
		push 4           ; .out_size
		lea ecx,[ebp+4]  ; &hBuff
		push edx         ; .output = &space
		push 4           ; .inp_size
		push ecx         ; .input
		push 17          ; .code   = SND_GETFREESPACE
		push DWORD PTR [ebp] ; .handle = [hSound]
	endif ; AC97SND_ON = 0

	lea ebx,[edi+17]
	lea eax,[edi+68]
	mov ecx,esp
	int 40h
	add esp,24
	pop edi ; <- space
	shr edi,FSOUND_Block+2  ; / (FSOUND_BlockSize * 4)
	jz _uFMOD_WaveOut_R     ; no free blocks available
_uFMOD_WaveOut_loop:
	call uFMOD_do_WaveOut
	neg eax
	dec edi
	ja _uFMOD_WaveOut_loop
_uFMOD_WaveOut_R:
	pop ebp
	pop edi
	ret

uFMOD_do_WaveOut:
	mov ecx,FSOUND_BlockSize*2
	push ebx
	push esi
	push edi
	mov edi,OFFSET MixBuf
	xor eax,eax
	push edi ; mixbuffer <= MixBuf
	push edi ; <- MixPtr
	; MIXBUFFER CLEAR
	mov esi,OFFSET _mod+36
	rep stosd

	if AC97SND_ON
	else
		if PAUSE_RESUME_ON
			cmp [ufmod_pause_],al
			xchg eax,ebp
			jne do_swfill
		endif
	endif ; AC97SND_ON = 0

	mov ebp,FSOUND_BlockSize
	; UPDATE MUSIC
	mov ebx,[esi+FMUSIC_MODULE.mixer_samplesleft-36]
fill_loop_1:
	test ebx,ebx
	jnz mixedleft_nz
	; UPDATE XM EFFECTS
	cmp [esi+FMUSIC_MODULE.tick-36],ebx ; new note
	mov ecx,[esi+FMUSIC_MODULE.pattern-36]
	jne update_effects
	dec ebx
	; process any rows commands to set the next order/row
	mov edx,[esi+FMUSIC_MODULE.nextorder-36]
	mov eax,[esi+FMUSIC_MODULE.nextrow-36]
	mov [esi+FMUSIC_MODULE.nextorder-36],ebx
	test edx,edx
	mov [esi+FMUSIC_MODULE.nextrow-36],ebx
	jl fill_nextrow
	mov [esi+FMUSIC_MODULE.order-36],edx
fill_nextrow:
	test eax,eax
	jl update_note
	mov [esi+FMUSIC_MODULE.row-36],eax
update_note:
	; mod+36 : ESI
	call DoNote
if ROWCOMMANDS_ON
	cmp DWORD PTR [esi+FMUSIC_MODULE.nextrow-36],-1
	jne inc_tick
endif
	mov eax,[esi+FMUSIC_MODULE.row-36]
	inc eax
	; if end of pattern
	; "if(mod->nextrow >= mod->pattern[mod->orderlist[mod->order]].rows)"
	cmp ax,[ebx]
	jl set_nextrow
	mov edx,[esi+FMUSIC_MODULE.order-36]
	movzx ecx,WORD PTR [esi+FMUSIC_MODULE.numorders-36]
	inc edx
	xor eax,eax
	cmp edx,ecx
	jl set_nextorder

	if AC97SND_ON
	else
		; We've reached the end of the order list. So, stop playing, unless looping is enabled.
		if NOLOOP_ON
			cmp [ufmod_noloop],al
			je set_restart
			pop eax ; skip MixPtr
			pop edx ; skip mixbuffer
			pop edi
			inc eax ; end of loop reached while XM_NOLOOP flag is enabled
			pop esi
			pop ebx
			ret
		set_restart:
		endif
	endif ; AC97SND_ON = 0

	movzx edx,WORD PTR [esi+FMUSIC_MODULE.restart-36]
	cmp edx,ecx
	sbb ecx,ecx
	and edx,ecx
set_nextorder:
	mov [esi+FMUSIC_MODULE.nextorder-36],edx
set_nextrow:
	mov [esi+FMUSIC_MODULE.nextrow-36],eax
	jmp inc_tick
update_effects:
	; mod+36 : ESI
	call DoEffs
inc_tick:
	mov eax,[esi+FMUSIC_MODULE.speed-36]
	mov ebx,[esi+FMUSIC_MODULE.mixer_samplespertick-36]
	inc DWORD PTR [esi+FMUSIC_MODULE.tick-36]
if PATTERNDELAY_ON
	add eax,[esi+FMUSIC_MODULE.patterndelay-36]
endif
	cmp [esi+FMUSIC_MODULE.tick-36],eax
	jl mixedleft_nz
if PATTERNDELAY_ON
	and DWORD PTR [esi+FMUSIC_MODULE.patterndelay-36],0
endif
	and DWORD PTR [esi+FMUSIC_MODULE.tick-36],0
mixedleft_nz:
	mov edi,ebp
	cmp ebx,edi
	jae fill_ramp
	mov edi,ebx
fill_ramp:
	pop edx  ; <- MixPtr
	sub ebp,edi
	lea eax,[edx+edi*8]
	push eax ; MixPtr += (SamplesToMix<<3)
	; tail    : [arg0]
	; len     : EDI
	; mixptr  : EDX
	; _mod+36 : ESI
	call Ramp

	if AC97SND_ON
	else
		if INFO_API_ON
			lea eax,[edi+edi*4]
			cdq
			shl eax,2
			mov ecx,FSOUND_MixRate/50
			div ecx
			; time_ms += SamplesToMix*FSOUND_OOMixRate*1000
			add [time_ms],eax
		endif
	endif ; AC97SND_ON = 0

	sub ebx,edi ; MixedLeft -= SamplesToMix
	test ebp,ebp
	jnz fill_loop_1
	mov [esi+FMUSIC_MODULE.mixer_samplesleft-36],ebx ; <= MixedLeft

	if AC97SND_ON
	else
		if INFO_API_ON
			mov ecx,[esi + FMUSIC_MODULE.row-36]
			or ecx,[esi + FMUSIC_MODULE.order-2-36]
			mov DWORD PTR [s_row],ecx
		endif
	endif ; AC97SND_ON = 0

do_swfill:
	; *** CLIP AND COPY BLOCK TO OUTPUT BUFFER
	pop eax ; skip MixPtr
	pop esi ; <- mixbuffer

	if AC97SND_ON
	else
		if INFO_API_ON
			; ebx : L channel RMS volume
			; ebp : R channel RMS volume
			xor ebx,ebx
		endif
	endif ; AC97SND_ON = 0

	mov edi,esi
	mov ecx,FSOUND_BlockSize*2
	align 4
fill_loop_2:
	lodsd

	if AC97SND_ON
		mov ebx,eax
		cdq
		xor eax,edx
		sub eax,edx
		mov ebp,255*volumerampsteps/2
		xor edx,edx
		div ebp
		cmp edx,255*volumerampsteps/4
		sbb eax,-1
		cmp eax,8000h
		sbb edx,edx
		not edx
		or eax,edx
		sar ebx,31
		and eax,7FFFh
		xor eax,ebx
		sub eax,ebx
	else
		if INFO_API_ON
			push edi
			cdq
			mov edi,eax
			push esi
			xor eax,edx
			mov esi,255*volumerampsteps/2
			sub eax,edx
			xor edx,edx
			div esi
			cmp edx,255*volumerampsteps/4
			pop esi
			sbb eax,-1
			cmp eax,8000h
			sbb edx,edx
			not edx
			or eax,edx
			sar edi,31
			and eax,7FFFh
		if VOL_CONTROL_ON
			mul DWORD PTR [ufmod_vol]
			shr eax,15
		endif
			; sum. the L and R channel RMS volume
			ror ecx,1
			sbb edx,edx
			and edx,eax
			add ebp,edx ; += |vol|
			rol ecx,1
			sbb edx,edx
			not edx
			and edx,eax
			add ebx,edx ; += |vol|
			xor eax,edi
			sub eax,edi
			pop edi
		else
			mov ebx,eax
			cdq
			xor eax,edx
			sub eax,edx
			mov ebp,255*volumerampsteps/2
			xor edx,edx
			div ebp
			cmp edx,255*volumerampsteps/4
			sbb eax,-1
			cmp eax,8000h
			sbb edx,edx
			not edx
			or eax,edx
			sar ebx,31
			and eax,7FFFh
		if VOL_CONTROL_ON
			mul DWORD PTR [ufmod_vol]
			shr eax,15
		endif
			xor eax,ebx
			sub eax,ebx
		endif
	endif ; AC97SND_ON = 0

	dec ecx
	stosw
	jnz fill_loop_2

	if AC97SND_ON
	else
		if INFO_API_ON
			shr ebp,FSOUND_Block      ; R_vol / FSOUND_BlockSize
			shl ebx,16-FSOUND_Block   ; (L_vol / FSOUND_BlockSize) << 16
			mov bx,bp
			mov DWORD PTR [L_vol],ebx
		endif
	endif ; AC97SND_ON = 0

	; *** DISPATCH DATA TO THE AC97 DRIVER
	push FSOUND_BlockSize*4 ; size
	push OFFSET MixBuf      ; &src
	push DWORD PTR [hBuff]  ; buffer
	mov edx,esp
	push ecx                ; .out_size
	push ecx                ; .output
	push 12                 ; .inp_size
	push edx                ; .input
	push 9                  ; .code = SND_OUT
	push DWORD PTR [hSound] ; .handle
	lea eax,[ecx+68]
	lea ebx,[ecx+17]
	mov ecx,esp
	int 40h
	add esp,36
	pop edi
	pop esi
	pop ebx
	ret
