; Cb-n#%li.-# @l$i Lkbnbe
format PE GUI 4.0 at 400000h
section '.text' code readable executable
entry start
i40_nt:
	jmp	i40_9x
start:
	xor	ebx, ebx
	call	[GetVersion]
	shr	eax, 31
	mov	[bIs9x], al
; get default heap
	call	[GetProcessHeap]
	mov	[hHeap], eax
	push	261
	push	startcurdir
	push	ebx
	call	[GetModuleFileNameA]
	xchg	ecx, eax
	inc	ecx
	inc	ecx
	lea	edi, [ecx+startcurdir-1]
	mov	al, '\'
	std
	repnz	scasb
	cld
	mov	byte [edi+2], bl
	mov	esi, startcurdir
	mov	edi, esi
	xor	ecx, ecx
	dec	ecx
	mov	al, 0
	repnz	scasb
	not	ecx
	dec	ecx
	mov	edi, win32_path
	push	edi
	rep	movsb
	mov	al, '\'
	cmp	byte [edi-1], al
	jz	@f
	stosb
@@:	mov	esi, inifilename
	mov	ecx, inifilenamesize
	rep	movsb
; parse command line
	call	[GetCommandLineA]
	xchg	eax, esi
	mov	edi, inname
	call	getfilename
	mov	edi, inname
	call	getfilename
	jc	no_file_given
	cmp	byte [esi], bl
	jz	file_known
	mov	[parameters], esi
	jmp	file_known
no_file_given:
	mov	[inname], bl
	push	comdlg32_name
	call	[LoadLibraryA]
	test	eax, eax
	jz	@f
	push	eax
	push	aGetOpenFileNameA
	push	eax
	call	[GetProcAddress]
	test	eax, eax
	jz	@f
	push	ofn
	call	eax
	test	eax, eax
	jz	@f
	call	[FreeLibrary]
	jmp	file_known
@@:
	push	ebx
	call	[ExitProcess]
file_known:
; TLS data
	mov	eax, [tls_index]
	mov	ecx, [fs:2Ch]
	mov	ebp, [ecx+eax*4]
; save registers
	mov	[ebp+tls._cs], cs
	mov	[ebp+tls._ds], ds
	mov	[ebp+tls._fs], fs
	mov	[ebp+tls._esp], esp
	mov	[ebp+tls._eip], exception
	mov	eax, 1000h
	call	malloc_big
	mov	edi, eax
; test for server
	push	seh
	push	dword [fs:ebx]
	mov	[fs:ebx], esp
	xor	eax, eax
server_test:
	div	edx
	pop	dword [fs:ebx]
	pop	esi
	test	eax, eax
	jz	server
	mov	[ebp+tls.cur_slot], eax
	mov	[hSharedData], ecx
	mov	[hSharedMutex], edx
	push	edi
	push	user32_thunks
	push	user32_name
	call	init_dll
	push	gdi32_thunks
	push	gdi32_name
	call	init_dll
	pop	edi
	push	edi
	call	[lstrlenA]
	inc	eax
	push	eax
	push	eax
	call	malloc
	pop	ecx
	mov	[ebp+tls.cur_dir], eax
	push	edi
	xchg	eax, edi
	xchg	eax, esi
	rep	movsb
	call	free_big
	call	map_shared_data
	push	bgr_mutex_name
	push	ebx
	push	ebx
	call	[CreateMutexA]
	mov	[hBgrMutex], eax
	push	ebx
	push	ebx
	push	3	; OPEN_EXISTING
	push	ebx
	push	1	; FILE_SHARE_READ
	push	80000000h	; GENERIC_READ
	push	inname
	call	[CreateFileA]
	inc	eax
	jnz	infileopened
	mov	esi, fileopenerr
fail:
	push	10h
	push	ebx
fail2:
	push	esi
	push	ebx
	cmp	[bInitialized], 0
	jnz	@f
	mov	eax, [esi-4]
loadfailed:
	div	edx
@@:
	call	[MessageBoxA]
	call	free_ldt
	push	ebx
	call	[ExitProcess]
infileopened:
	dec	eax
	xchg	eax, edi
	push	eax
	mov	eax, esp
	push	ebx
	push	eax
	push	36
	push	header
	push	edi
	call	[ReadFile]
	test	eax, eax
	pop	eax
	mov	esi, filereaderr
	jz	fail
	cmp	eax, 36
	jnz	fail
	cmp     [header], 'KPCK'
	jnz     .program_not_packed
        mov	eax, [header+4]
	call	malloc_big
        test    eax, eax
        mov     esi, memerr
        jz      fail
        push    eax
        push    eax
        push    ebx
        push    edi
        call    [GetFileSize]
        mov     [limit], eax
	call	malloc_big
        test    eax, eax
        jz      fail
        push    eax
        push    ebx
        push    ebx
        push    ebx
        push    edi
        call    [SetFilePointer]
        push    eax
        mov     eax, esp
        push    ebx
        push    eax
        push    [limit]
        push    dword [esp+16]
        push    edi
        call    [ReadFile]
        test    eax, eax
        pop     eax
        mov     esi, filereaderr
        jz      fail
        cmp     eax, [limit]
        jnz     fail
        pop     esi
        push    esi
        mov     eax, [esi+4]
        mov     [limit], eax
        call    unpack
        push    esi
        call    free_big
        pop     edx
        mov     esi, notexe
        cmp     dword [edx], 'MENU'
        jnz     fail
        cmp     word [edx+4], 'ET'
        jnz     fail
        mov     ax, word [edx+6]
        sub     ax, '00'
        xchg    al, ah
        cmp     ax, 1
        ja      fail
        push    edi
        mov     esi, edx
        mov     edi, header
        mov     ecx, 9
        rep     movsd
        jz      @f
        mov     eax, [edx+18h]
        mov     [header+1Ch], eax
        mov     eax, [edx+14h]
        shr     eax, 1
        sub     eax, 10h
        mov     [header+18h], eax
        mov     [header+20h], ebx
@@:
        push    edx
        push    40h     ; PAGE_EXECUTE_READWRITE
        push    1000h   ; MEM_COMMIT
        push    dword [edx+14h]
        push    ebx
        call    [VirtualAlloc]
        pop     edx
        test    eax, eax
        mov     esi, memerr
        jz      fail
        mov     [base], eax
        mov     edi, eax
        mov     esi, edx
        mov     ecx, [limit]
        mov     eax, ecx
        shr     ecx, 2
        rep     movsd
        mov     ecx, eax
        and     ecx, 3
        rep     movsb
        jmp     .program_packed_common
.program_not_packed:
	mov	esi, notexe
	cmp	[header], 'MENU'
	jnz	fail
	cmp	word [header+4], 'ET'
	jnz	fail
	mov	ax, word [header+6]
	sub	ax, '00'
	xchg	al, ah
	cmp	ax, 1
	ja	fail
	jz	@f
	mov	eax, [header+18h]
	mov	[header+1Ch], eax
	mov	eax, [header+14h]
	shr	eax, 1
	sub	eax, 10h
	mov	[header+18h], eax
	mov	[header+20h], ebx
@@:
; hmm... Menuet/Kolibri seems to ignore app_i_end field in case of running from ramdisk (fn 19)
; but depend on app_i_end in case of running from fn 58

; so force read all file
	push	ebx
	push	edi
	call	[GetFileSize]
	mov	[header+10h], eax
	mov	eax, [header+14h]
	cmp	eax, [header+10h]
	jb	fail
	push	40h	; PAGE_EXECUTE_READWRITE
	push	1000h	; MEM_COMMIT
	push	eax
	push	ebx
	call	[VirtualAlloc]
	test	eax, eax
	mov	esi, memerr
	jz	fail
	mov	[base], eax
	push	ebx
	push	ebx
	push	ebx
	push	edi
	call	[SetFilePointer]
	push	eax
	mov	eax, esp
	push	ebx
	push	eax
	push	[header+10h]
	push	[base]
	push	edi
	call	[ReadFile]
	test	eax, eax
	pop	eax
	mov	esi, filereaderr
	jz	fail
	push	edi
.program_packed_common:
	call	[CloseHandle]
	mov	esi, [parameters]
	mov	edi, esi
	test	esi, esi
	jz	no_params
	mov	eax, [header+1Ch]
	test	eax, eax
	jz	no_params
	mov	edx, eax
	add	eax, 256
	cmp	eax, [header+14h]
	mov	esi, params_err
	ja	fail
	mov	esi, edi
	mov	ecx, 256
	xor	eax, eax
	repnz	scasb
	neg	cl
	mov	edi, edx
	add	edi, [base]
	rep	movsb
no_params:
; read ini file client settings
; disks
	push	512
	push	ramdisk_path
	push	default_ramdisk
	push	ramdisk_keyname
	push	aDisk
	call	[GetPrivateProfileStringA]
	mov	edi, hd_partitions_num
hdloop:
	push	win32_path
	push	ebx
	push	hdxn
	push	aDisk
	call	[GetPrivateProfileIntA]
	stosd
	test	eax, eax
	jz	.cont
	push	eax
	shl	eax, 9		; *512
	push	eax
	call	malloc
	mov	[edi-4+hd_partitions_array-hd_partitions_num], eax
	pop	ecx
	xchg	esi, eax
	xor	eax, eax
	inc	eax
.partitions:
	push	eax ecx
	push	eax
	push	hdpart
	push	converted_path
	call	[wsprintfA]
	add	esp, 12
	mov     byte [esi+511], 0
	push	win32_path
	push	511
	push	esi
	push	null_string
	push	converted_path
	push	aDisk
	call	[GetPrivateProfileStringA]
	test	eax, eax
	jnz	@f
	push	10h
	push	converted_path
	mov	esi, no_partition
	jmp	fail2
@@:
        push    esi
        call    [lstrlenA]
        cmp     eax, 10
        jbe     @f
        lea     eax, [eax+esi-9]
        cmp     byte [eax], ','
        jnz     @f
        cmp     dword [eax+1], 'read'
        jnz     @f
        cmp     dword [eax+5], 'only'
        jnz     @f
        mov     byte [eax], 0
        mov     byte [esi+511], 1
@@:
	add	esi, 512
	pop	ecx eax
	inc	eax
	dec     ecx
	jnz 	.partitions
.cont:
	inc	[hdxn+2]
	inc	[hdpart+2]
	cmp	edi, hd_partitions_num+4*4
	jnz	hdloop
	mov	esi, converted_path
; read fonts
	push	win32_path
	push	512
	push	esi
	push	null_string
	push	aFont1
	push	aMain
	call	[GetPrivateProfileStringA]
	call	getfilemap
	mov	[char_mt], eax
	push	win32_path
	push	512
	push	esi
	push	null_string
	push	aFont2
	push	aMain
	call	[GetPrivateProfileStringA]
	call	getfilemap
	mov	[char2_mt], eax
	push	win32_path
	push	ebx
	push	aSetBgr
	push	aQuestions
	call	[GetPrivateProfileIntA]
	mov	[SetBgrQuestion], eax
; read skin
	push	win32_path
	push	512
	push	esi
	push	null_string
	push	aSkin
	push	aMain
	call	[GetPrivateProfileStringA]
	call	getfilemap
	xchg	eax, edi
	cmp	dword [edi], 'KPCK'
	jnz	@f
	mov	eax, [edi+4]
	call	malloc_big
	mov	esi, memerr
	test	eax, eax
	jz	fail
	push	eax
	push	eax
	push	edi
	call	unpack
	push	edi
	call	[UnmapViewOfFile]
	pop	edi
	inc	ebx
@@:
	cmp	dword [edi], 'SKIN'	; ident
	mov	esi, skinfileerr
	jnz	fail
	cmp	dword [edi+4], 1	; version
	jnz	fail
; skin parameters
	mov	esi, edi
	add	esi, [esi+8]		; parameters offset
	mov	ecx, 9
	push	edi
	mov	edi, _skinh
	rep	movsd
	pop	edi
	mov	ecx, common_colors
	mov	edx, 127
	call	get_wnd_colors
	test	al, al
	jnz	@f
	lodsd
	and	eax, edx
	push	eax
	xchg	eax, ecx
	push	edi
	mov	edi, common_colors
	push	edi
	rep	movsb
	pop	ecx
	pop	edi
	pop	edx
	call	set_wnd_colors
@@:
; skin bitmaps
	mov	esi, edi
	add	esi, [esi+16]
skinbmploop:
	cmp	dword [esi], 0
	jz	skinbmploopend
	movzx	eax, word [esi]
	movzx	ecx, word [esi+2]
	mov	edx, [esi+4]
	add	esi, 8
	add	edx, edi
	lea	eax, [eax*2+ecx-1]
; convert bmp data to Win32 DIB
	push	eax edx
	mov	eax, [edx]
	add	eax, 3
	and	al, not 3
	mul	dword [edx+4]
	imul	eax, 3
	add	eax, 40
	push	eax
	push	8	; HEAP_ZERO_MEMORY
	push	[hHeap]
	call	[HeapAlloc]
	pop	edx
	mov	dword [eax], 40		; biSize
	mov	ecx, [edx]
	mov	[eax+4], ecx		; biWidth
	mov	ecx, [edx+4]
	mov	[eax+8], ecx		; biHeight
	mov	dword [eax+12], 180001h	; biPlanes, biBitCount
	push	esi edi
	lea	edi, [eax+40]
	lea	esi, [edx+8]
	mov	ecx, [edx+4]
	push	eax
	mov	eax, [edx]
	imul	eax, ecx
	add	esi, eax
	add	esi, eax
	add	esi, eax
.x1:
	push	ecx
	mov	ecx, [edx]
	add	ecx, ecx
	add	ecx, [edx]
	sub	esi, ecx
	push	esi
	rep	movsb
	add	edi, 3
	and	edi, not 3
	pop	esi
	pop	ecx
	loop	.x1
	pop	edx
	pop	edi esi
	pop	eax
	dec	eax
	jnz	@f
; inactive left
	mov	[left1_bmp], edx
	jmp	skinbmploop
@@:
	dec	eax
	jnz	@f
; active left
	mov	[left_bmp], edx
	jmp	skinbmploop
@@:
	dec	eax
	jnz	@f
; inactive oper
	mov	[oper1_bmp], edx
	jmp	skinbmploop
@@:
	dec	eax
	jnz	@f
; active oper
	mov	[oper_bmp], edx
	jmp	skinbmploop
@@:
	dec	eax
	jnz	@f
; inactive base
	mov	[base1_bmp], edx
	jmp	skinbmploop
@@:
	dec	eax
	jnz	skinbmploop
; active base
	mov	[base_bmp], edx
	jmp	skinbmploop
skinbmploopend:
; skin buttons
	mov	esi, edi
	add	esi, [esi+12]
skinbuttonsloop:
	lodsd
	test	eax, eax
	jz	skinbuttonsloopend
	mov	edx, skin_btn_close
	dec	eax
	jz	.button
	mov	edx, skin_btn_minimize
	dec	eax
	jz	.button
	lodsd
	lodsd
	jmp	skinbuttonsloop
.button:
	mov	ecx, 4
@@:
	lodsw
	cwde
	mov	[edx], eax
	add	edx, 4
	loop	@b
	jmp	skinbuttonsloop
skinbuttonsloopend:
	dec	ebx
	jz	.mem
	xor	ebx, ebx
	push	edi
	call	[UnmapViewOfFile]
	jmp	@f
.mem:
	push	edi
	call	free_big
@@:
; sound volume
	push	win32_path
	push	10
	push	aSoundVol
	push	aSetup
	call	[GetPrivateProfileIntA]
	and	al, 0x7F
	mov	[sound_vol], al
; direct screen access parameters
	push	win32_path
	push	32
	push	aColorDepth
	push	aDirectScreenAccess
	call	[GetPrivateProfileIntA]
	test	eax, eax
	jz	@f
	cmp	eax, 24
	jz	@f
	cmp	eax, 32
	jz	@f
	mov	esi, aInvalidColorDepth
	jmp	fail
@@:
	mov	[ColorDepth], eax
	push	win32_path
	push	200
	push	aInvalidateTime
	push	aDirectScreenAccess
	call	[GetPrivateProfileIntA]
	mov	[InvalidateTime], eax
	mov	[DSA], ebx
	push	DSACritSect
	call	[InitializeCriticalSection]
	cmp	[ColorDepth], ebx
	jz	@f
	push	4	; PAGE_READWRITE
	push	2000h	; MEM_RESERVE
	push	1000000h
	push	ebx
	call	[VirtualAlloc]
	mov	esi, memerr
	test	eax, eax
	jz	fail
	mov	[DSA], eax
@@:
; parse path
        mov     eax, [header+20h]
        test    eax, eax
        jz      path_done
        cmp     eax, [header+14h]
        jae     path_done
;        jb      @f
;        push    30h
;        push    aWarning
;        push    aPathInvalid
;        push    0
;        call    [MessageBoxA]
;        jmp     path_done
;@@:
        push    0
        push    startcurdir
        push    261
        push    inname
        call    [GetFullPathNameA]
; test for /rd/1
        push    ramdisk_path
        call    [lstrlenA]
        push    eax
        push    eax
        push    ramdisk_path
        push    eax
        push    startcurdir
        push    1
        push    800h
        call    [CompareStringA]
        cmp     eax, 2
        pop     eax
        jz      .ramdisk
; test for /hdx/y
        xor     ecx, ecx
.hdxloop:
        push    ecx
        mov     esi, [hd_partitions_array+ecx*4]
        mov     edi, [hd_partitions_num+ecx*4]
        test    edi, edi
        jz      .hdxcont
.hdyloop:
        push    esi
        call    [lstrlenA]
        push    eax
        push    eax
        push    esi
        push    eax
        push    startcurdir
        push    1
        push    800h
        call    [CompareStringA]
        cmp     eax, 2
        pop     eax
        jz      .hdxy
        add     esi, 512
        dec     edi
        jnz     .hdyloop
.hdxcont:
        pop     ecx
        inc     ecx
        cmp     ecx, 4
        jb      .hdxloop
        mov     esi, aPathUnknown
        jmp     fail
.ramdisk:
        push    eax
        mov     edi, [header+20h]
        add     edi, [base]
        mov     eax, '/RD/'
        stosd
        mov     ax, '1/'
        stosw
        jmp     .common
.hdxy:
        pop     ecx
        sub     esi, [hd_partitions_array+ecx*4]
        shr     esi, 9
        inc     esi
        push    eax
        mov     edi, [header+20h]
        add     edi, [base]
        push    esi
        push    ecx
        push    hdxy_str
        push    edi
        call    [wsprintfA]
        add     esp, 10h
        add     edi, eax
.common:
        pop     eax
        lea     esi, [startcurdir+eax]
.l:
        lodsb
        cmp     al, '\'
        jnz     @f
        mov     al, '/'
@@:
        stosb
        test    al, al
        jnz     .l
        mov	eax, [header+20h]
        add	eax, [base]
        push	eax
        push	eax
        call	[CharToOemA]
path_done:
; create window
;        push    0
;        push    16
;        push    16
;        push    1
;        push    1
;        push    400000h
;        call    [LoadImageA]
;        push    eax             ; hIconSm
        push    ebx             ; hIconSm
	push	classname	; lpszClassName
	push	ebx		; lpszMenuName
	push	ebx		; hbrBackground
	push	32512
	push	ebx
	call	[LoadCursorA]
	mov     [hArrow], eax
	push	eax		; hCursor
;	push	ebx		; hIcon
        push    1
        push    400000h
        call    [LoadIconA]
        push    eax             ; hIcon
	push	400000h		; hInstance
	push	ebx		; cbWndExtra
	push	ebx		; cbClsExtra
	push	wndproc		; lpfnWndProc
	push	3		; style = CS_HREDRAW or CS_VREDRAW
	push    48              ; cbSize
	push	esp
	call	[RegisterClassExA]
	add	esp, 48
	push	ebx		; lpParam
	push	400000h		; hInstance
	push	ebx		; hMenu
	push	ebx		; hWndParent
	mov	eax, 80000000h	; CW_USEDEFAULT
	push	eax		; nHeight
	push	eax		; nWidth
	push	eax		; y
	push	eax		; x
	push	eax		; dwStyle = WS_POPUP
;	push	ebx		; lpWindowName
	mov	esi, inname
	mov	edx, esi
@@:
	lodsb
	cmp	al, 0
	jz	.done
	cmp	al, '\'
	jz	.x
	cmp     al, '/'
	jz      .x
	cmp	al, ':'
	jnz	@b
.x:	mov	edx, esi
	jmp	@b
.done:
	dec	esi
	cmp	byte [esi-1], '.'
	jnz	@f
	dec	esi
	mov	byte [esi], 0
@@:
	push	edx
	mov	[process_name], edx
	push	classname	; lpClassName
	push	ebx		; dwExStyle
	call	[CreateWindowExA]
	mov	[ebp+tls.hWnd], eax
	mov	[ebp+tls.bActive], 1
	mov	[ebp+tls.bFirstMouseMove], 1
	test	eax, eax
	mov	esi, createwnderr
	jz	fail
	call	get_cur_slot_ptr
	mov	[edi+shared_data_struc.hWnd-shared_data_struc.threads], eax
	cmp	[edi+shared_data_struc.thread_id-shared_data_struc.threads], 2
	jnz	.notfirst
	mov	esi, [shared_data]
	cmp	[esi+shared_data_struc.vk], 0
	jnz	.workarea_vk
	push	ebx
	lea	eax, [esi+shared_data_struc.workarea_left]
	push	eax
	push	ebx
	push	30h	; SPI_GETWORKAREA
	call	[SystemParametersInfoA]
	dec     [esi+shared_data_struc.workarea_right]
	dec     [esi+shared_data_struc.workarea_bottom]
	jmp	.workarea_set
.workarea_vk:
	push	esi
	call	get_screen_size
	pop	esi
	inc	ebx
	mov	word [esi+shared_data_struc.workarea_bottom], bx
	shr	ebx, 10h
	inc	ebx
	mov	[esi+shared_data_struc.workarea_right], ebx
	xor	ebx, ebx
.workarea_set:
.notfirst:
	push	newprg_section_name
	push	1000h
	push	ebx
	push	4
	push	ebx
	push	-1
	call	[CreateFileMappingA]
	push	eax
	mov	esi, shared_section_create_err
	test	eax, eax
	jz	fail
	push	ebx
	push	ebx
	push	ebx
	push	2
	push	eax
	call	[MapViewOfFile]
	pop	ecx
	push	eax
	push	ecx
	call	[CloseHandle]
	pop	eax
	push	eax
	cmp	word [eax], 0x201
	jnz	@f
	mov	ecx, [ebp+tls.hWnd]
	mov	[eax+2], ecx
	mov	byte [eax+1], 3
@@:
	call	[UnmapViewOfFile]
; allocate LDT selectors
; data segment
	mov	esi, selector_data
	mov	eax, [base]
	mov	[esi+2], ax
	shr	eax, 10h
	mov	[esi+4], al
	mov	[esi+7], ah
	mov	eax, [header+14h]
	dec	eax
	mov	[limit], eax
	mov     [fn9limit], eax
	call	get_cur_slot_ptr
	mov	[edi+24], eax
	shr	eax, 0Ch
	mov	[esi], ax
	shr	eax, 10h
	or	al, 11000000b
	mov	[esi+6], al
	mov	byte [esi+5], 11110010b
	lea	edi, [esi+8]
; code segment
	movsd
	movsd
	mov	byte [esi+5], 11111010b
	cmp	[bIs9x], 0
	jnz	alloc_ldt_9x
	push	ntdll_name
	call	[GetModuleHandleA]
	push	aNtSetLdtEntries
	push	eax
	call	[GetProcAddress]
	mov	[NtSetLdtEntries], eax
	push	dword [esi-4]
	push	dword [esi-8]
	push	17h
	push	dword [esi+4]
	push	dword [esi]
	push	0Fh
	call	eax
	mov	esi, ldterr
	test	eax, eax
	js	fail
	mov	eax, [DSA]
	test	eax, eax
	jz	@f
	push	ebx
	push	ebx
	push	ebx
	mov	edx, eax
	mov	dx, (11000000b shl 8) + 11110010b
	ror	edx, 16
	xchg	dl, dh
	ror	edx, 8
	push	edx
	shl	eax, 16
	mov	ax, 0FFFh
	push	eax
	push	1Fh
	call	[NtSetLdtEntries]
	test	eax, eax
	js	fail
	mov	[_gs], 1Fh
@@:
dorunklbr:
; jump to program code
	mov	eax, [header+18h]
	mov	[ebp+tls._esp], eax
	mov	eax, [header+0Ch]
	mov	[ebp+tls._eip], eax
	push	3200h		; eflags
	xor	eax, eax
	push	eax
	push	eax
	push	eax
	push	eax
	push	eax
	push	eax
	push	eax
	push	eax
; Kolibri process was successfully created, notify parent
	call	get_cur_slot_ptr
	mov	[edi+shared_data_struc.win32_stack-shared_data_struc.threads], esp
	mov	[bInitialized], 1
notify_parent:
	div	edx
	jmp	i40_done

alloc_ldt_9x:
	mov	eax, r0p
	call	CallRing0
; patch int40
	add	edi, (40h-9)*8
	mov	eax, i40_9x
	mov	[edi], ax
	mov	word [edi+2], cs
	shr	eax, 16
	mov	[edi+6], ax
	mov	word [edi+4], 1110111100000000b
	xor	ebx, ebx
	jmp	dorunklbr
free_ldt:
	cmp	[bIs9x], 0
	jnz	@f
.ret:	ret
@@:
	cmp	[temp_cs], 0
	jz	.ret
	mov	eax, fl0p

CallRing0:
	call	acquire_shared		; int 9 is global resource
	sidt	[idtr]
	mov	edi, dword [idtr+2]
	add	edi, 8*9
	push	dword [edi]
	push	dword [edi+4]
	mov	[edi], ax
	mov	word [edi+2], 28h
;	mov	word [edi+4], 0xEE00
;	shr	eax, 16
;	mov	[edi+6], ax
	mov	[edi+4], eax
	mov	word [edi+4], 0xEE00
	int	9
	pop	dword [edi+4]
	pop	dword [edi]
	call	release_shared
	ret

r0p:
	int	20h	; VMMCall Get_Cur_VM_Handle
	dw	1
	dw	1
	push	0
	push	1
	push	dword [esi]
	push	dword [esi+4]
	push	ebx
	int	20h	; VMMCall _Allocate_LDT_Selector
	dw	78h
	dw	1
	add	esp, 14h
	mov	[klbr_cs], ax
	push	0
	push	1
	push	dword [esi-8]
	push	dword [esi-4]
	push	ebx
	int	20h	; VMMCall _Allocate_LDT_Selector
	dw	78h
	dw	1
	add	esp, 14h
	mov	[klbr_ds], ax
	mov	eax, [DSA]
	test	eax, eax
	jz	@f
	push	0
	push	1
	mov	edx, eax
	mov	dx, (11000000b shl 8) + 11110010b
	ror	edx, 16
	xchg	dl, dh
	ror	edx, 8
	shl	eax, 16
	mov	ax, 0FFFh
	push	eax
	push	edx
	push	ebx
	int	20h	; VMMCall _Allocate_LDT_Selector
	dw	78h
	dw	1
	add	esp, 14h
	mov	[_gs], ax
@@:
	push	0
	push	1
	mov	eax, temp_code
	mov	ecx, eax
	shl	eax, 16
	add	eax, temp_code_size-1
	push	eax
	mov	eax, ecx
	and	eax, 0xFF000000
	add	eax, 0000000011111011b shl 8
	shr	ecx, 16
	mov	al, cl
	push	eax
	push	ebx
	int	20h	; VMMCall _Allocate_LDT_Selector
	dw	78h
	dw	1
	add	esp, 14h
	mov	[temp_cs], ax
	mov	[temp_cs2], ax
	push	0
	push	1
	mov	eax, temp_stack
	mov	ecx, eax
	shl	eax, 16
	add	eax, temp_stack_size-1
	push	eax
	mov	eax, ecx
	and	eax, 0xFF000000
	add	eax, 0000000011110011b shl 8
	shr	ecx, 16
	mov	al, cl
	push	eax
	push	ebx
	int	20h	; VMMCall _Allocate_LDT_Selector
	dw	78h
	dw	1
	add	esp, 14h
	mov	[temp_ss], ax
;	mov	eax, 40h
;	mov	cx, [_cs]
;	mov	edx, i40_9x
;	int	20h	; VMMCall Set_PM_Int_Vector
;	dw	45h
;	dw	1
;	xor	ecx, ecx
;	xor	edx, edx
;	int	20h	; VMMCall Get_PM_Int_Vector
;	dw	44h
;	dw	1
	iret
fl0p:
	int	20h	; VMMCall Get_Cur_VM_Handle
	dw	1
	dw	1
	movzx	eax, [klbr_cs]
	call	free_selector
	movzx	eax, [klbr_ds]
	call	free_selector
	movzx	eax, [temp_cs]
	call	free_selector
	movzx	eax, [temp_ss]
	call	free_selector
	xor	ebx, ebx
	iret
sl0p:
	int	20h	; VMMCall Get_Cur_VM_Handle
	dw	1
	dw	1
	push	0
	push	dword [esi]
	push	dword [esi+4]
	push	ebx
	movzx	eax, [klbr_cs]
	push	eax
	int	20h	; VMMCall _SetDescriptor
	dw	7Ch
	dw	1
	push	0
	push	dword [esi-8]
	push	dword [esi-4]
	push	ebx
	movzx	eax, [klbr_ds]
	push	eax
	int	20h	; VMMCall _SetDescriptor
	dw	7Ch
	dw	1
	add	esp, 40
	iret
rdmsrp:
; rdmsr may throw exception
	mov	esi, .exception_struc
	int	20h	; VMMCall Install_Exception_Handler
	dw	0EFh
	dw	1
	xor	ebx, ebx	; assume OK
.start_eip:
	rdmsr
.end_eip:
	mov	esi, .exception_struc
	int	20h	; VMMCall Remove_Exception_Handler
	dw	0F0h
	dw	1
	iret
.exception_struc:
	dd	0
	dd	.start_eip
	dd	.end_eip
	dd	.exception_handler
.exception_handler:
	inc	ebx
	jmp	.end_eip

free_selector:
	push	0
	push	eax
	push	ebx
	int	20h	; VMMCall _Free_LDT_Selector
	dw	79h
	dw	1
	add	esp, 12
	ret

seh:
	mov	eax, [esp+12]
	add	dword [eax+0xB8], 2
	xor	eax, eax
	ret

ofn_hook:
	cmp	dword [esp+8], 2	; WM_DESTROY
	jnz	@f
	push	260
	mov	eax, converted_path
	mov	[parameters], eax
	push	eax
	push	23
	push	dword [esp+12+4]
	push	user32_name
	call	[GetModuleHandleA]
	push	GetDlgItemTextA_thunk+2
	push	eax
	call	[GetProcAddress]
	call	eax
@@:
	xor	eax, eax
	ret	10h

getfilename:
@@:
	lodsb
	cmp	al, 0
	jz	.not
	cmp	al, ' '
	jbe	@b
	cmp	al, '"'
	setz	dl
	jz	.loo
	dec	esi
.loo:
	lodsb
	cmp	al, 0
	jz	.end
	cmp	al, ' '
	ja	@f
	test	dl, 1
	jz	.end
@@:	cmp	al, '"'
	jnz	@f
	test	dl, 1
	jnz	.end_quote
@@:	stosb
	jmp	.loo
.end_quote:
	lodsb
.end:
	or	al, al
	jnz	@f
	dec	esi
@@:	mov	al, 0
	stosb
	clc
	ret
.not:
	stc
	ret

map_shared_data:
	push	0
	push	0
	push	0
	push	2
	push	[hSharedData]
	call	[MapViewOfFile]
	mov	[shared_data], eax
	ret

acquire_shared:
	pushad
	push	-1
	push	[hSharedMutex]
	call	[WaitForSingleObject]
	popad
	ret
release_shared:
	pushad
	push	[hSharedMutex]
	call	[ReleaseMutex]
	popad
	ret

get_cur_slot_ptr_server:
	push	eax
	mov	eax, [cur_slot]
@@:
	call	get_slot_ptr
	pop	eax
	ret
get_cur_slot_ptr:
	push	eax
	mov	eax, [ebp+tls.cur_slot]
	jmp	@b
get_slot_ptr:
	mov	edi, [shared_data]
	shl	eax, 6
	lea	edi, [eax+edi+shared_data_struc.threads]
	ret

read_color:
	push	esi
	mov	ecx, 6
	xor	edx, edx
.l:
	lodsb
	cmp	al, 0
	jz	.d
	or	al, 20h
	sub	al, '0'
	cmp	al, 10
	jb	@f
	sub	al, 'a'-10-'0'
@@:
	shl	edx, 4
	or	dl, al
	loop	.l
.d:
	pop	esi
	xchg	eax, edx
	ret

i40_9x:
; set Win32 context
	push	eax ecx
	mov	eax, [cs:tls_index]
	shl	eax, 2
	add	eax, [fs:2Ch]
	mov	eax, [cs:eax]
	mov	ds, [cs:eax+tls._ds]
	mov	es, [eax+tls._ds]
;	mov	fs, [_fs]
	mov	ecx, [esp+8]	; eip
	dec	ecx
	dec	ecx
	mov	[eax+tls._eip], ecx
	mov	ecx, [esp+16]	; eflags
	mov	ss, [eax+tls._ds]
	xchg	esp, [eax+tls._esp]
	push	ecx
	add	[eax+tls._esp], 20
	mov	eax, [eax+tls._esp]
	add	eax, [base]
	mov	ecx, [eax-20]
	mov	eax, [eax-16]
	popfd

exception:
	pushfd
	cld
; test for page fault in direct screen area
	push	ebp eax
	mov	eax, [tls_index]
	mov	ebp, [fs:2Ch]
	mov	ebp, [ebp+eax*4]
	mov	eax, [ebp+tls.saved_fs0]
	mov	[fs:0], eax
	mov	eax, [ebp+tls.saved_fs4]
	mov	[fs:4], eax
	cmp	[ebp+tls.exc_code], 0C0000005h
	jnz	noaccvio
	mov	eax, [ebp+tls.exc_data]
	sub	eax, [DSA]
	cmp	eax, 0FFFFFFh
	ja	noaccvio
; handle page fault in direct screen area
	pop	eax ebp
	pushad
	mov	ebp, [tls_index]
	shl	ebp, 2
	add	ebp, [fs:2Ch]
	mov	ebp, [ebp]
	push	DSACritSect
	call	[EnterCriticalSection]
	cmp	[bHaveDSA], 0
	jnz	dsafail
	call	get_screen_size
	mov	eax, ebx
	shr	eax, 16
	movzx	ebx, bx
	inc	eax
	inc	ebx
	mov	edi, eax
	mul	ebx
	mul	[ColorDepth]
	shr	eax, 3
	add	eax, 0xFFF
	and	eax, not 0xFFF
	mov	ecx, [ebp+tls.exc_data]
	sub	ecx, [DSA]
	cmp	ecx, eax
	jb	@f
dsafail:
	push	DSACritSect
	call	[LeaveCriticalSection]
	push	40h
	push	0
	push	DSAErr
	push	0
mbni:
	call	[MessageBoxA]
	popad
	push	ebp eax
	mov	ebp, [tls_index]
	shl	ebp, 2
	add	ebp, [fs:2Ch]
	mov	ebp, [ebp]
	jmp	notint40
@@:
	push	4
	push	1000h
	push	eax
	push	[DSA]
	call	[VirtualAlloc]
; get screen data
	push	ebp
	push	0
	call	[GetDC]
	push	eax
	xchg	eax, ebp
	call	[CreateCompatibleDC]
	xchg	eax, esi
	push	ebx
	push	edi
	push	ebp
	call	[CreateCompatibleBitmap]
	push	eax
	push	esi
	call	[SelectObject]
	push	eax
	xor	eax, eax
	push	0xCC0020
	push	eax
	push	eax
	push	ebp
	push	ebx
	push	edi
	push	eax
	push	eax
	push	esi
	call	[BitBlt]
	push	esi
	call	[SelectObject]
	push	ebp
	xchg	eax, ebp
	xor	eax, eax
; now esi=hDC, ebp=hBitmap
	push	eax	; biClrImportant
	push	eax	; biClrUsed
	push	eax	; biYPelsPerMeter
	push	eax	; biXPelsPerMeter
	push	eax	; biSizeImage
	push	eax	; biCompression
	push	1	; biPlanes
	mov	ecx, [ColorDepth]
	mov	[esp+2], cx	; biBitColor
	neg	ebx
	push	ebx	; biHeight
	neg	ebx
	push	edi	; biWidth
	push	40	; biSize
	mov	ecx, esp
	push	eax
	push	ecx
	push	[DSA]
	push	ebx
	push	eax
	push	ebp
	push	esi
	call	[GetDIBits]
	add	esp, 40
	push	ebp
	call	[DeleteObject]
	push	esi
	call	[DeleteDC]
	push	0
	call	[ReleaseDC]
	mov	[bHaveDSA], 1
	push	eax
	push	esp
	push	0
	push	0
	push	DSAFreeThread
	push	10000h
	push	0
	call	[CreateThread]
	pop	eax
	push	DSACritSect
	call	[LeaveCriticalSection]
	pop	ebp
	mov	ebp, [tls_index]
	shl	ebp, 2
	add	ebp, [fs:2Ch]
	mov	ebp, [ebp]
	jmp	i40_done
noaccvio:
; test for int40
	mov	eax, [ebp+tls._eip]
	cmp	eax, [limit]
	jae	notint40
	add	eax, [base]
	cmp	word [eax], 0x40CD
	jz	int40
notint40:

	pop	eax
	push	esi
	sub	esp, 400h
	mov	esi, esp
	push	dword [esi+408h]
	push	[ebp+tls._eip]
	push	dword [esi+404h]
	push	[ebp+tls._esp]
	push	edi
	push	dword [esi+400h]
	push	edx
	push	ecx
	push	ebx
	push	eax
	push	excstr
	push	esi
	call	[wsprintfA]
	push	0
	push	exceptionstr
	push	esi
	push	0
	call	[MessageBoxA]
lock	dec	[NumThreads]
	jnz	.et
	call	free_ldt
	push	0
	call	[ExitProcess]
.et:
	push	0
	call	[ExitThread]

int40:
	add	[ebp+tls._eip], 2
	pop	eax ebp
	pushad
safe_to_suspend:
	mov	ebp, [tls_index]
	shl	ebp, 2
	add	ebp, [fs:2Ch]
	mov	ebp, [ebp]
	inc	eax
	cmp	eax, num_i40_fns
	push    eax     ; emulate ret addr for not_supported_i40_fn
	jae	not_supported_i40_fn
	pop     eax
	call	[i40fns + eax*4]
i40_done:
	cmp	[NumThreads], 1
	jnz	i40_done_mt
	mov	eax, [ebp+tls._esp]
	mov	[klbr_esp], eax
	mov	eax, [ebp+tls._eip]
	mov	[jmp_klbr_eip], eax
	lea	eax, [esp+24h]
	mov	[ebp+tls._esp], eax
	mov	[ebp+tls._eip], exception
	mov	eax, [fs:0]
	mov	[ebp+tls.saved_fs0], eax
	mov	eax, [fs:4]
	mov	[ebp+tls.saved_fs4], eax
	popad
	popfd
	mov	ss, [klbr_ds]
	mov	esp, [klbr_esp]
	mov	es, [klbr_ds]
;	mov	fs, [klbr_null]
;	mov	gs, [klbr_null]
	mov	gs, [_gs]
	mov	ds, [klbr_ds]
i40_done_jmp1:
	jmp	[cs:jmp_klbr]
i40_done_mt:
	mov	eax, [ebp+tls._esp]
	mov	[esp+12], eax
	mov	ecx, [ebp+tls._eip]
	xchg	[fs:0], ecx
	mov	[ebp+tls.saved_fs0], ecx
	movzx	ecx, [klbr_cs]
	xchg	[fs:4], ecx
	mov	[ebp+tls.saved_fs4], ecx
	lea	eax, [esp+24h]
	mov	[ebp+tls._esp], eax
	mov	[ebp+tls._eip], exception
	popad
	popfd
	mov	ss, [klbr_ds]
	mov	esp, [ds:esp-24h+12]
	mov	es, [klbr_ds]
;	mov	fs, [klbr_null]
;	mov	gs, [klbr_null]
	mov	gs, [_gs]
	mov	ds, [klbr_ds]
i40_done_jmp2:
	jmp	fword [fs:0]

not_supported_i40_fn:
	sub	esp, 200h-4
	mov	esi, esp
	push	dword [esi+200h+20h]
	push	[ebp+tls._eip]
	push	dword [esi+200h+8]
	push	[ebp+tls._esp]
	push	dword [esi+200h]
	push	dword [esi+200h+4]
	push	dword [esi+200h+14h]
	push	dword [esi+200h+18h]
	push	dword [esi+200h+10h]
	push	dword [esi+200h+1Ch]
	push	notsupportedmsg
	push	esi
	call	[wsprintfA]
	push	0
	push	nsm
	push	esi
	push	0
	call	[MessageBoxA]
i40_terminate:
lock	dec	[NumThreads]
	jnz	.thread
	call	free_ldt
	push	0
	call	[ExitProcess]
.thread:
	push	0
	call	[ExitThread]

align 4
i40fns	dd	i40_terminate		; -1
	dd	i40_draw_window		; 0
	dd	i40_put_pixel		; 1
	dd	i40_getkey		; 2
	dd	i40_get_sys_time	; 3
	dd	i40_writetext		; 4
	dd	i40_delay		; 5
	dd	i40_read_floppy_file	; 6
	dd	i40_putimage		; 7
	dd	i40_define_button	; 8
	dd	i40_get_process_info	; 9
	dd	i40_wait_event		; 10
	dd	i40_check_event		; 11
	dd	i40_redraw_status	; 12
	dd	i40_drawrect		; 13
	dd	i40_get_screen_size	; 14
	dd	i40_set_background	; 15
	dd	not_supported_i40_fn	; 16
	dd	i40_getbutton		; 17
	dd	i40_sys_service		; 18
	dd	not_supported_i40_fn	; 19
	dd	not_supported_i40_fn	; 20
	dd	i40_sys_setup		; 21
	dd	not_supported_i40_fn	; 22
	dd	i40_wait_event_timeout	; 23
	dd	not_supported_i40_fn	; 24
	dd	not_supported_i40_fn	; 25
	dd	i40_getsetup		; 26
	dd	not_supported_i40_fn	; 27
	dd	not_supported_i40_fn	; 28
	dd	i40_get_sys_date	; 29
	dd	i40_current_folder	; 30
	dd	not_supported_i40_fn	; 31
	dd	i40_delete_ramdisk_file	; 32
	dd	i40_write_ramdisk_file	; 33
	dd	not_supported_i40_fn	; 34
	dd	i40_screen_getpixel	; 35
	dd	i40_screen_getarea	; 36
	dd	i40_read_mouse_pos	; 37
	dd	i40_draw_line		; 38
	dd	i40_get_background	; 39
	dd	i40_set_event_mask	; 40
	dd	not_supported_i40_fn	; 41
	dd	not_supported_i40_fn	; 42
	dd	not_supported_i40_fn	; 43
	dd	not_supported_i40_fn	; 44
	dd	not_supported_i40_fn	; 45
	dd	i40_reserve_free_ports	; 46
	dd	i40_display_number	; 47
	dd	i40_display_settings	; 48
	dd	not_supported_i40_fn	; 49
	dd	i40_set_window_shape	; 50
	dd	i40_create_thread	; 51
	dd	not_supported_i40_fn	; 52
	dd	not_supported_i40_fn	; 53
	dd	not_supported_i40_fn	; 54
	dd	i40_sound_interface	; 55
	dd	not_supported_i40_fn	; 56
	dd	not_supported_i40_fn	; 57
	dd	i40_file_system		; 58
	dd	not_supported_i40_fn	; 59
	dd	i40_ipc			; 60
	dd	i40_direct_scr_access	; 61
	dd	i40_pci			; 62
	dd	i40_debug_board		; 63
	dd	i40_resize_app_memory	; 64
	dd	i40_putimage_palette	; 65
	dd	i40_process_def		; 66
	dd	i40_move_resize		; 67
	dd	i40_sys_services	; 68
	dd	i40_debug_services	; 69
	dd	i40_file_system_lfn	; 70
	dd	i40_window_settings	; 71
num_i40_fns = ($ - i40fns)/4

getfilemap:
; in: esi->filename
; out: eax->mapped file
	push	esi
	sub	esp, 200h
	cmp	word [esi+1], ':\'
	jz	.fullpath
	mov	edi, esp
	push	esi
	mov	esi, startcurdir
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
	pop	esi
	dec	edi
	mov	al, '\'
	cmp	byte [edi-1], al
	jz	@f
	stosb
@@:
	lodsb
	stosb
	test	al, al
	jnz	@b
	mov	esi, esp
.fullpath:
	push	ebx
	push	ebx
	push	3	; OPEN_EXISTING
	push	ebx
	push	1	; FILE_SHARE_READ
	push	80000000h	; GENERIC_READ
	push	esi
	call	[CreateFileA]
	add	esp, 200h
	pop	esi
	inc	eax
	jz	.failed
	dec	eax
	xchg	eax, edi
	push	ebx
	push	ebx
	push	ebx
	push	2	; PAGE_READONLY
	push	ebx
	push	edi
	call	[CreateFileMappingA]
	test	eax, eax
	jz	.failed
	push	edi
	xchg	eax, edi
	call	[CloseHandle]
	push	ebx
	push	ebx
	push	ebx
	push	4	; FILE_MAP_READ
	push	edi
	call	[MapViewOfFile]
	test	eax, eax
	jz	.failed
	push	eax
	push	edi
	call	[CloseHandle]
	pop	eax
	ret
.failed:
	push	ebx
	push	filereaderr
	jmp	fail2

DSAFreeThread:
	push	[InvalidateTime]
	call	[Sleep]
	push	DSACritSect
	call	[EnterCriticalSection]
	push	4000h
	push	0
	push	[DSA]
	call	[VirtualFree]
	mov	[bHaveDSA], 0
	push	DSACritSect
	call	[LeaveCriticalSection]
	ret

virtual at 0
button_desc:
	.next	dd	?	; must be 1st dword
	.id	dd	?
	.xsize	dw	?
	.xstart	dw	?
	.ysize	dw	?
	.ystart	dw	?
	.color	dd	?
	.size = $
end virtual

test_maximized:
	sub	esp, 40
	push	44
	push	esp
	push	[ebp+tls.hWnd]
	call	[GetWindowPlacement]
	mov	eax, [esp+8]	; showCmd
	add	esp, 44
	cmp	eax, 3	; SW_SHOWMAXIMIZED
	ret

wndproc:
; LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	push	ebp
	mov	ebp, [tls_index]
	mov	eax, [fs:2Ch]
	mov	ebp, [eax+ebp*4]
	mov	eax, [esp+8+4]
	cmp	eax, 0xF	; WM_PAINT
	jz	wmpaint
	dec	eax
;	jz	wmcreate
	dec	eax
	jz	wmdestroy
	dec	eax
	jz	wmmove
	dec	eax
	dec	eax
	jz	wmsize
	dec	eax
	jz	wmactivate
	cmp	eax, 0x1A-6
	jz	wmsettingchange
	cmp     eax, 0x20-6
	jz      wmsetcursor
	cmp	eax, 0x24-6
	jz	wmgetminmaxinfo
	sub	eax, 0x84-6
	jz	wmnchittest
	cmp	eax, 0xA1-0x84
	jz	wmnclbuttondown
	cmp	eax, 0xA3-0x84
	jz	wmnclbuttondblclk
	sub	eax, 0x100-0x84	; WM_KEYDOWN
	jz	wmkeydown
	dec	eax
	jz	wmkeyup
	dec	eax
	jz	wmchar
	dec	eax
	dec	eax
 	jz	wmsyskeydown
 	dec     eax
 	jz      wmsyskeyup
	sub	eax, 0x200-0x105	; WM_MOUSEMOVE
	jz	wmmousemove
	dec	eax
	jz	wmlbuttondown
	dec	eax
	jz	wmlbuttonup
	dec	eax
	dec	eax
	jz	wmrbuttondown
	dec	eax
	jz	wmrbuttonup
	cmp	eax, 0x20A-0x205
	jz	wmmousewheel
	cmp	eax, 0x214-0x205
	jz	wmsizing
	sub	eax, 0x400-0x205	; WM_USER
	jz	wm_ipc
	cmp	eax, 0xC000-0x400
	jz	vk_mouse
	dec	eax
	jz	wm_debug1
	dec	eax
	jz	wm_debug2
wmdef:
	pop	ebp
	jmp	[DefWindowProcA]
wmsettingchange:
	call	del_background
@@:
	test	[ebp+tls.message_mask], 10h
	jz	wmdef
	mov	[ebp+tls.translated_msg_code], 5
	push	0
	push	0
	push	0
	push	dword [esp+20]
	call	[PostMessageA]
	jmp	wmdef
wmactivate:
	mov	eax, [shared_data]
	mov	ecx, [ebp+tls.cur_slot]
	inc	ecx
	cmp	word [esp+12+4], 0
	jz	.inact1
	mov	[eax+shared_data_struc.active_process], ecx
	jmp	.cmn1
.inact1:
	call	acquire_shared
	cmp	[eax+shared_data_struc.active_process], ecx
	jnz	@f
	mov	[eax+shared_data_struc.active_process], 1
@@:	call	release_shared
.cmn1:
	mov	al, byte [ebp+tls.color_main+3]
	and	al, 0Fh
	cmp	al, 3
	jz	.setactive
	cmp	al, 4
	jnz	wmdef
.setactive:
	mov	al, [esp+12+4]
	mov	[ebp+tls.bActive], al	; 0/1/2
wndproc_update_wnd:
	mov	[ebp+tls.curdraw], 0
	push	0
	push	0
	push	[ebp+tls.hWnd]
	call	[InvalidateRect]
	jmp	wmdef
wmpaint:
	push	esi
	push	edi
	sub	esp, 0x40
	push	esp
	push	[ebp+tls.hWnd]
	call	[BeginPaint]
;	test	[ebp+tls.message_mask], 1
;	jz	@f
;	mov	[ebp+tls.translated_msg_code], 1
@@:	xchg	eax, edi
	cmp	[ebp+tls.curdraw], 0
	mov	[ebp+tls.curdraw], 1
	jz	.nopaint
	call	draw_window_base
.nopaint:
	push	esp
	push	[ebp+tls.hWnd]
	call	[EndPaint]
	add	esp, 40h
	pop	edi
	pop	esi
	pop	ebp
	xor	eax, eax
	ret	10h
wmdestroy:
	push	0
	call	[PostQuitMessage]
@@:
	xor	eax, eax
	pop	ebp
	ret	10h
wmnclbuttondown:
	call	test_maximized
	jnz	wmdef
	push    [ebp+tls.hWnd]
	call    [SetForegroundWindow]
	jmp	@b
;wmwindowposchanging:
;        call    test_maximized
;        jnz     @b
;        mov     eax, [esp+0x10+4]
;        or      byte [eax+18h], 2       ; SWP_NOMOVE
;        jmp     @b
wmnchittest:
; for window type 1 always return HTCLIENT
	mov	cl, byte [ebp+tls.color_main+3]
	and	cl, 0x0F
	cmp	cl, 0x01
	jz	.client
	mov	ax, [esp+0x10+4]	; x
	sub	ax, [ebp+tls.x_start]
	mov	dx, [esp+0x12+4]	; y
	sub	dx, [ebp+tls.y_start]
; test for caption
        push    eax
        mov     eax, [_skinh]
        cmp     cl, 0x03
        jz      @f
        mov     al, 21
@@:
	cmp	dx, ax
	pop     eax
	jae	.nocaption
; check for buttons
	push	esi
	call	find_button
	test	esi, esi
	pop	esi
	jnz	.button
.caption:
	push	2		; HTCAPTION
	pop	eax
	pop	ebp
	ret	10h
.button:
.client:
	push	1		; HTCLIENT
	jmp	.ret
.nocaption:
; do not resize window with type 0
	jecxz	.client
; do not resize window with type 4
	cmp	ecx, 0x04000000
	jz	.client
; do not resize maximized window
	push	eax edx
	call	test_maximized
	pop	edx eax
	jz	.client
	sub	dx, [ebp+tls.y_size]
	neg	dx
	cmp	dx, 7
	jbe	.bottomall
	cmp	ax, 7
	jbe	.left
	sub	ax, [ebp+tls.x_size]
	neg	ax
	cmp	ax, 7
	ja	.client
	push	11		; HTRIGHT
	jmp	.ret
.left:
	push	10		; HTLEFT
.ret:
	pop	eax
	pop	ebp
	ret	10h
.bottomall:
	cmp	ax, 7
	jbe	.bottomleft
	sub	ax, [ebp+tls.x_size]
	neg	ax
	cmp	ax, 7
	ja	.bottom
	push	17		; HTBOTTOMRIGHT
	jmp	.ret
.bottomleft:
	push	16		; HTBOTTOMLEFT
	jmp	.ret
.bottom:
	push	15		; HTBOTTOM
	jmp	.ret
wmsetcursor:
        cmp     [ebp+tls.hCursor], 0
        jz      wmdef
        push    [ebp+tls.hCursor]
        call    [SetCursor]
        push    1
        pop     eax
        pop     ebp
        ret     10h
wmnclbuttondblclk:
	mov	al, byte [ebp+tls.color_main+3]
	and	al, 0xF
	jz	.nomaximize
	cmp	al, 1
	jz	.nomaximize
	cmp	al, 4
	jz	.nomaximize
	call	test_maximized
	mov	eax, 3	; SW_MAXIMIZED
	jnz	@f
	mov	al, 1	; SW_SHOWNORMAL
@@:
	push	eax
	push	[ebp+tls.hWnd]
	call	[ShowWindow]
	push	1
	push	0
	push	[ebp+tls.hWnd]
	call	[InvalidateRect]
.nomaximize:
	xor	eax, eax
	pop	ebp
	ret	10h
wmmove:
	mov	ax, [esp+0x10+4]
	mov	[ebp+tls.x_start], ax
	mov	ax, [esp+0x12+4]
	mov	[ebp+tls.y_start], ax
;	jmp	wndproc_update_wnd
	xor	eax, eax
	pop	ebp
	ret	10h
wmsize:
	mov	ax, [esp+0x10+4]
	mov	[ebp+tls.x_size], ax
	mov	ax, [esp+0x12+4]
	mov	[ebp+tls.y_size], ax
;	jmp	wndproc_update_wnd
	xor	eax, eax
	pop	ebp
	ret	10h
wmsizing:
	mov	eax, [esp+0x14]
	mov	ecx, [eax]
	mov	[ebp+tls.x_start], cx
	mov	ecx, [eax+4]
	mov	[ebp+tls.y_start], cx
	mov	ecx, [eax+8]
	sub	ecx, [eax]
	mov	[ebp+tls.x_size], cx
	mov	ecx, [eax+12]
	sub	ecx, [eax+4]
	mov	[ebp+tls.y_size], cx
;	push	0
;	push	0
;	push	[ebp+tls.hWnd]
;	call	[InvalidateRect]
	xor	eax, eax
	inc	eax
	pop	ebp
	ret	10h
wmsyskeydown:
;	test	byte [esp+16+3+4], 20h	; Alt pressed?
;	jnz	wmdef
        cmp     byte [esp+16+2+4], 3Eh  ; Alt+F4?
        jz      wmdestroy
wmkeydown:
	movzx	eax, byte [esp+16+2+4]
	test	eax, eax
	jnz	@f
	mov	al, 1Ch		; <Enter>
@@:
	cmp	[ebp+tls.usescancode], 0
	jnz	.putkeycode
; ignore keys-modifiers
	cmp	al, 2Ah
	jz	.ret
	cmp	al, 36h
	jz	.ret
	cmp	al, 38h
	jz	.ret
	cmp	al, 1Dh
	jz	.ret
	cmp	al, 3Ah
	jz	.ret
	cmp	al, 45h
	jz	.ret
	cmp	al, 46h
	jz	.ret
; translate NumPad keys
	test	byte [esp+14h+3], 1
	jnz	.nonumpad
	mov	cl, '*'
	cmp	al, 55
	jz	@f
	cmp	al, 71
	jb	.nonumpad
	cmp	al, 83
	ja	.nonumpad
	mov	cl, [numlock_map+eax-71]
@@:
	push	eax
	push	ecx
	sub	esp, 100h
	push	esp
	call	[GetKeyboardState]
	mov	al, [esp+0x90]	; VK_NUMLOCK
	add	esp, 100h
	test	al, 1
	pop	ecx
	pop	eax
	jnz	.put_cl
.nonumpad:
	mov	cl, [keymap+eax]
	push	eax
	push	ecx
	push	0x11	; VK_CONTROL
	call	[GetAsyncKeyState]
	test	ax, ax
	jns	@f
	sub	byte [esp], 60h
@@:
	push	0x10	; VK_SHIFT
	call	[GetAsyncKeyState]
	test	ax, ax
	jns	@f
	pop	ecx
	pop	eax
	mov	cl, [keymap_shift+eax]
	push	eax
	push	ecx
@@:
	push	0x12	; VK_MENU
	call	[GetAsyncKeyState]
	test	ax, ax
	pop	ecx
	pop	eax
	jns	@f
	mov	cl, [keymap_alt+eax]
@@:
.put_cl:
	xchg	eax, ecx
.putkeycode:
	movzx	ecx, [ebp+tls.keybuflen]
	inc	cl
	jz	.ret
; test for extended key (0xE0 prefix)
	test	byte [esp+14h+3], 1	; lParam+3
	jz	.noext
	cmp     [ebp+tls.usescancode], 0
	jz      .noext
	mov	[ebp+tls.keybuflen], cl
	mov	[ebp+tls.keybuffer+ecx-1], 0xE0
	inc	cl
	jz	.ret
.noext:
	mov	[ebp+tls.keybuflen], cl
	mov	[ebp+tls.keybuffer+ecx-1], al
	test	[ebp+tls.message_mask], 2
	jz	@f
	mov	[ebp+tls.translated_msg_code], 2
@@:
.ret:
wmchar:
	xor	eax, eax
	pop	ebp
	ret	10h
wmkeyup:
wmsyskeyup:
	cmp	[ebp+tls.usescancode], 0
	jz	wmkeydown.ret
	mov	al, [esp+16+2+4]
	or	al, 80h
	jmp	wmkeydown.putkeycode
;wmchar:
;	cmp	[usescancode], 0
;	jnz	wmkeydown.ret
;	mov	al, [esp+12]
;	jmp	wmkeydown.putkeycode
wmlbuttondown:
	push	esi
	push	1
	jmp	@f
wmrbuttondown:
	push	esi
	push	2
@@:
	call	capture1
	mov	ax, [esp+0x10+12]	; x
	mov	dx, [esp+0x12+12]	; y
	call	find_button
	pop	eax
	test	esi, esi
	jnz	.onbutton
	test	[ebp+tls.message_mask], 20h
	jz	@f
	mov	[ebp+tls.translated_msg_code], 6
@@:
.done:
	pop	esi
	pop	ebp
	xor	eax, eax
	ret	10h
.onbutton:
	or	[ebp+tls.current_buttons], al
	cmp	[ebp+tls.original_buttons], 0
	jnz	@f
	mov	[ebp+tls.original_buttons], al
@@:
	mov	[ebp+tls.active_button], esi
; don't highlight button if bit 29 is set
	test	[esi+button_desc.id], 20000000h
	jnz	.done
; highlight - negate border
	call	negate_button_border
	jmp	.done
wmrbuttonup:
	push	-3
	jmp	@f
wmlbuttonup:
	push	-2
@@:
	call	capture2
	pop	eax
	cmp	[ebp+tls.active_button], 0
	jz	wmrbuttondown.nobutton
	and	[ebp+tls.current_buttons], al
	jnz	wmrbuttondown.nobutton
	push	esi
	xor	esi, esi
	xchg	esi, [ebp+tls.active_button]
	test	byte [esi+button_desc.id+3], 20h
	jnz	@f
	call	negate_button_border
@@:
; minimize button - special handler (see event.inc)
	cmp	word [esi+button_desc.id], 0FFFFh
	jz	.minimize
	test	[ebp+tls.message_mask], 4
	jz	@f
	mov	[ebp+tls.translated_msg_code], 3
@@:
	test	[ebp+tls.message_mask], 20h
	jz	@f
	mov	[ebp+tls.translated_msg_code], 86h
@@:
	mov	[ebp+tls.bFirstMouseMove], 1
	movzx	ecx, [ebp+tls.butbuflen]
	inc	cl
	jz	@f
	mov	[ebp+tls.butbuflen], cl
	mov	eax, [esi+button_desc.id]
	shl	eax, 8
	mov	al, [ebp+tls.original_buttons]
	mov	[ebp+tls.butbuffer+ecx*4-4], eax
@@:
	mov	[ebp+tls.original_buttons], 0
.done:
	pop	esi
.ret:
	xor	eax, eax
	pop	ebp
	ret	10h
.minimize:
	call	minimize_window
	jmp	.done
wmrbuttondown.nobutton:
wmmousemove:
	cmp	[ebp+tls.bFirstMouseMove], 0
	mov	[ebp+tls.bFirstMouseMove], 0
	jnz	wmdef
vk_mouse:
; N.B. Due of current implementation of buttons in the kernel
; mouse events are NOT processed when any button is active!
	cmp	[ebp+tls.active_button], 0
	jnz	wmlbuttonup.ret
	test	[ebp+tls.message_mask], 20h
	jz	wmlbuttonup.ret
	mov	[ebp+tls.translated_msg_code], 6
	jmp	wmlbuttonup.ret
wmmousewheel:
	movsx	eax, word [esp+0xE+4]
	sub	[ebp+tls.scroll], eax
	jmp	vk_mouse
wm_ipc:
	test	[ebp+tls.message_mask], 40h
	jz	wmlbuttonup.ret
	mov	[ebp+tls.translated_msg_code], 7
	jmp	wmlbuttonup.ret
wm_debug1:
	test	byte [ebp+tls.message_mask+1], 1
	jz	.failed2
	push	edi
	call	get_cur_slot_ptr
	mov	edi, [edi+shared_data_struc.debugger_mem-shared_data_struc.threads]
	test	edi, edi
	jz	.failed
	add	edi, [base]
	mov	eax, [edi]
	mov	ecx, [edi+4]
	sub	eax, ecx
	cmp	eax, 12
	jl	.failed
	add	dword [edi+4], 12
	lea	edi, [edi+ecx+8]
	xor	eax, eax
	inc	eax
	stosd
	push	edi
	mov	eax, [esp+0xC+12]
	call	get_slot_ptr
	mov	eax, [edi]
	pop	edi
	stosd
	mov	eax, [esp+0x10+8]
; translate Win32 exception code to x86 exception vector
	cmp	eax, 0x80000004
	jz	.singlestep
	xor	ecx, ecx
	push	edi
	mov	edi, exccode2number-5
.1:
	add	edi, 5
	cmp	eax, [edi]
	jnz	.2
	mov	cl, [edi+4]
	jmp	.3
.2:
	cmp	dword [edi], ecx
	jnz	.1
	mov	cl, 0xD		; translate unrecognized codes to #GP
.3:
	pop	edi
	jmp	.4
.singlestep:
	push	ebx
	mov	ecx, [edi-4]
	call	find_debuggee
	mov	ecx, ebx
	pop	ebx
	jecxz	.failed
	sub	esp, 0xB2*4
	push	1001Fh
	push	esp
	push	dword [ecx+12]
	call	[GetThreadContext]
	mov	ecx, [esp+0x14]		; DR6
	mov	byte [edi-8], 3		; signal #DB
	add	esp, 0xB3*4
.4:
	mov	[edi], ecx
.written:
	pop	edi
	mov	[ebp+tls.translated_msg_code], 9
	jmp	wmlbuttonup.ret
.failed:
	pop	edi
.failed2:
	push	40h
	push	0
	push	aFailedToDeliverDebugMessage
	push	[ebp+tls.hWnd]
	call	[MessageBoxA]
	jmp	wmlbuttonup.ret
wm_debug2:
	test	byte [ebp+tls.message_mask+1], 1
	jz	wm_debug1.failed2
	push	edi
	call	get_cur_slot_ptr
	mov	edi, [edi+shared_data_struc.debugger_mem-shared_data_struc.threads]
	test	edi, edi
	jz	wm_debug1.failed
	add	edi, [base]
	mov	eax, [edi]
	mov	ecx, [edi+4]
	sub	eax, ecx
	cmp	eax, 8
	jl	wm_debug1.failed
	add	dword [edi+4], 8
	lea	edi, [edi+ecx+8]
	push	2
	pop	eax
	stosd
	push	edi
	mov	eax, [esp+0xC+12]
	call	get_slot_ptr
	mov	eax, [edi]
	pop	edi
	stosd
; delete this item from debuggees list
	lea	ecx, [ebp+tls.debuggees]
@@:
	mov	edx, [ecx]
	test	edx, edx
	jz	wm_debug1.written
	cmp	dword [edx+4], eax
	jz	.found
	mov	ecx, edx
	jmp	@b
.found:
	push	dword [edx]
	push	ecx
	push	edx
	call	free
	pop	ecx
	pop	dword [ecx]
	jmp	wm_debug1.written

wmgetminmaxinfo:
	mov	ecx, [shared_data]
	cmp	[ecx+shared_data_struc.vk], 0
	jnz	@f
	sub	esp, 10h
	mov	eax, esp
	push	0
	push	eax
	push	0
	push	30h	; SPI_GETWORKAREA
	call	[SystemParametersInfoA]
	mov	eax, [esp+20+10h]	; lParam
	mov	ecx, esp
	mov	edx, [ecx]
	mov	[eax+10h], edx
	mov	edx, [ecx+4]
	mov	[eax+14h], edx
	mov	edx, [ecx+8]
	sub	edx, [ecx]
	mov	[eax+8], edx
	mov	edx, [ecx+12]
	sub	edx, [ecx+4]
	mov	[eax+0Ch], edx
	add	esp, 10h
	jmp	.ret
@@:
	call	acquire_shared
	mov	eax, [esp+20]	; lParam
	mov	edx, [ecx+shared_data_struc.workarea_left]
	mov	[eax+10h], edx
	mov	edx, [ecx+shared_data_struc.workarea_top]
	mov	[eax+14h], edx
	mov	edx, [ecx+shared_data_struc.workarea_right]
	sub	edx, [ecx+shared_data_struc.workarea_left]
	mov	[eax+8], edx
	mov	edx, [ecx+shared_data_struc.workarea_bottom]
	sub	edx, [ecx+shared_data_struc.workarea_top]
	mov	[eax+0Ch], edx
	call	release_shared
.ret:
	xor	eax, eax
	pop	ebp
	ret	10h

find_button:
	mov	esi, [ebp+tls.buttons]
.loop:
	test	esi, esi
	jz	.done
	push	eax
	sub	ax, [esi+button_desc.xstart]
	cmp	ax, [esi+button_desc.xsize]
	pop	eax
	jae	.cont
	push	edx
	sub	dx, [esi+button_desc.ystart]
	cmp	dx, [esi+button_desc.ysize]
	pop	edx
	jb	.done
.cont:
	mov	esi, [esi]
	jmp	.loop
.done:
	ret

negate_button_border:
	push	edi
	push	[ebp+tls.hWnd]
	call	[GetDC]
	xchg	eax, edi
	push	6	; R2_NOT
	push	edi
	call	[SetROP2]
	push	eax
	movzx	eax, [esi+button_desc.xstart]
	movzx	edx, [esi+button_desc.ystart]
; point 4
	push	edx
	push	eax
; point 3
	mov	ecx, edx
	add	cx, [esi+button_desc.ysize]
	push	ecx
	push	eax
; point 2
	push	ecx
	mov	ecx, eax
	add	cx, [esi+button_desc.xsize]
	push	ecx
; point 1
	push	edx
	push	ecx
; point 0
	push	edx
	push	eax
; Polyline
	mov	eax, esp
	push	5
	push	eax
	push	edi
	call	[Polyline]
	add	esp, 5*8
	push	edi
	call	[SetROP2]
	push	edi
	push	[ebp+tls.hWnd]
	call	[ReleaseDC]
	pop	edi
	ret

draw_border:
	mov	eax, [ebp+tls.color_border]
	shr	eax, 1
	and	eax, 0x007F7F7F
	cmp	byte [esp+4], 0
	jz	@f
	mov	eax, [skin_active_outer]
	cmp	[ebp+tls.bActive], 0
	jnz	@f
	mov	eax, [skin_passive_outer]
@@:
	call	create_select_pen
	push	eax
	xor	esi, esi
	call	rect_wnd
	call	select_delete
	mov	eax, [ebp+tls.color_border]
	cmp	byte [esp+4], 0
	jz	@f
	mov	eax, [skin_active_frame]
	cmp	[ebp+tls.bActive], 0
	jnz	@f
	mov	eax, [skin_passive_frame]
@@:
	call	create_select_pen
	push	eax
@@:
	inc	esi
	call	rect_wnd
	cmp	esi, 3
	jnz	@b
	call	select_delete
	mov	eax, [ebp+tls.color_border]
	shr	eax, 1
	and	eax, 0x007F7F7F
	cmp	byte [esp+4], 0
	jz	@f
	mov	eax, [skin_active_inner]
	cmp	[ebp+tls.bActive], 0
	jnz	@f
	mov	eax, [skin_passive_inner]
@@:
	call	create_select_pen
	push	eax
	inc	esi
	call	rect_wnd
	call	select_delete
	ret	4

rect_wnd:
	movzx	ecx, [ebp+tls.y_size]
	dec	ecx
	sub	ecx, esi
	movzx	edx, [ebp+tls.x_size]
	dec	edx
	sub	edx, esi
; point 5
	push	esi
	push	esi
; point 4
	push	esi
	push	edx
; point 3
	push	ecx
	push	edx
; point 2
	push	ecx
	push	esi
; point 1
	push	esi
	push	esi
	mov	eax, esp
	push	5
	push	eax
	push	edi
	call	[Polyline]
	add	esp, 40
	ret

rectangle_gradient:
; in: edi=hDC
; stack:
; [esp+4] = xstart
; [esp+8] = ystart
; [esp+12] = xend
; [esp+16] = yend (end is excluded)
; [esp+20] = color
; [esp+24] = color_delta (if gradient specified)
	test	byte [esp+20+3], 80h
	jnz	.dograd
; no gradient
	mov	eax, [esp+20]
	call	convert_color
	push	eax
	call	[CreateSolidBrush]
	push	eax
	push	dword [esp+4+16]
	push	dword [esp+8+12]
	push	dword [esp+12+8]
	push	dword [esp+16+4]
	mov	ecx, esp
	push	eax
	push	ecx
	push	edi
	call	[FillRect]
	add	esp, 10h
	call	[DeleteObject]
.done:
	ret	24
.dograd:
; gradient
	mov	esi, [esp+8]
.gradloop:
	cmp	esi, [esp+16]
	jae	.done
	mov	eax, [esp+20]
	call	create_select_pen
	push	eax
	push	0
	push	esi
	push	dword [esp+12+4]
	push	edi
	call	[MoveToEx]
	push	esi
	push	dword [esp+8+12]
	push	edi
	call	[LineTo]
	call	select_delete
	inc	esi
;	mov	eax, [esp+24]
;	sub	[esp+20], eax
	test	byte [esp+27], 80h
	jnz	.signed
	mov	al, [esp+24]
	sub	[esp+20], al
	jnb	@f
	add	[esp+20], al
@@:	mov	al, [esp+25]
	sub	[esp+21], al
	jnb	@f
	add	[esp+21], al
@@:	mov	al, [esp+26]
	sub	[esp+22], al
	jnb	@f
	add	[esp+22], al
@@:	jmp	.gradloop
.signed:
	mov	al, [esp+24]
	add	[esp+20], al
	jnb	@f
	sub	[esp+20], al
@@:	mov	al, [esp+25]
	add	[esp+21], al
	jnb	@f
	sub	[esp+21], al
@@:	mov	al, [esp+26]
	add	[esp+22], al
	jnb	@f
	sub	[esp+22], al
@@:	jmp	.gradloop

create_select_pen:
	call	convert_color
	push	eax
	push	1
	push	0
	call	[CreatePen]
	push	eax
	push	edi
	call	[SelectObject]
	ret
select_delete:
	push	dword [esp+4]
	push	edi
	call	[SelectObject]
	push	eax
	call	[DeleteObject]
	ret	4

malloc:
	push	dword [esp+4]
	push	0
	push	[hHeap]
	call	[HeapAlloc]
	ret	4
free:
	push	dword [esp+4]
	push	0
	push	[hHeap]
	call	[HeapFree]
	ret	4
malloc_big:
	push	4	; PAGE_READWRITE
	push	1000h	; MEM_COMMIT
	push	eax
	push	0
	call	[VirtualAlloc]
	ret
free_big:
	push	8000h	; MEM_RELEASE
	push	0
	push	dword [esp+12]
	call	[VirtualFree]
	ret	4

capture1:
	inc	[bCaptured]
	cmp	[bCaptured], 1
	jnz	@f
	push	[ebp+tls.hWnd]
	call	[SetCapture]
@@:	ret
capture2:
	cmp	[bCaptured], 0
	jz	@f
	dec	[bCaptured]
	jnz	@f
	call	[ReleaseCapture]
@@:	ret

server_fail:
	push	10h
	push	0
	push	esi
	push	0
	call	init_MessageBox
	call	[MessageBoxA]
	push	0
	call	[ExitProcess]

server_exists:
	mov	esi, vkerr
	cmp	[vk], 0
	jnz	server_fail
	mov	al, 1
	mov	edx, [newprg_section]
lock	xchg	[edx], al
	test	al, al
	jz	@f
	push	edx
	push	200
	call	[Sleep]
	pop	edx
	dec	edi
	jnz	server_exists
@@:
	mov	esi, inname
	lea	edi, [edx+8]
	mov	ecx, 108h/4
	rep	movsd
	mov	esi, [parameters]
	test	esi, esi
	jnz	@f
	mov	esi, converted_path
@@:	mov	ecx, 100h/4
	rep	movsd
	mov	byte [edx+1], 1
	mov	esi, edx
	push	50
	pop	edi
@@:
	push	200
	call	[Sleep]
	cmp	byte [esi+1], 3
	jz	@f
	dec	edi
	jnz	@b
	push	ebx
	call	[ExitProcess]
@@:
	push	esi
	push	user32_thunks
	push	user32_name
	call	init_dll
	pop	esi
	push	dword [esi+2]
	call	[SetForegroundWindow]
	mov	word [esi], bx
	push	ebx
	call	[ExitProcess]

newprg_request:
	lea	edi, [eax+8]
	lea	esi, [eax+110h]
	call	acquire_shared
	call	new_kolibri_process_with_default_curdir
	call	release_shared
	mov	eax, [newprg_section]
	mov	byte [eax+1], 2
	jmp	debugloop

server:
	push	edi
	call	free_big
	sub	esp, 44h
	push	esp
	call	[GetStartupInfoA]
	mov	eax, [esp+8]
	test	eax, eax
	jz	@f
	cmp	dword [eax], 'Koli'
	jnz	@f
	cmp	dword [eax+4], 'briD'
	jnz	@f
	mov	[vk], 1
@@:
	push	newprg_section_name
	push	1000h
	push	ebx
	push	4
	push	ebx
	push	-1
	call	[CreateFileMappingA]
	mov	esi, shared_section_create_err
	test	eax, eax
	jz	server_fail
	xchg	eax, edi
	call	[GetLastError]
	push	eax
	push	ebx
	push	ebx
	push	ebx
	push	2
	push	edi
	call	[MapViewOfFile]
	mov	[newprg_section], eax
	test	eax, eax
	jz	server_fail
;	push	edi
;	call	[CloseHandle]
	pop	eax
	cmp	eax, 183
	jz	server_exists
	mov	ecx, [esp+2Ch]
	add	esp, 44h
	test	cl, cl
	js	@f
	cmp	[vk], 0
	jnz	@f
; We were created without STARTF_FORCEOFFFEEDBACK flag.
; Rerun self. This has two goals: first, this avoids "hour glass" cursor,
; second, if GetOpenFileNameA was used, it didn't cleanup all resources,
; but new process will run in clean environment.
	push	[newprg_section]
	call	[UnmapViewOfFile]
	push	edi
	call	[CloseHandle]
	mov	[bDontDebug], 1
	call	create_child
	push	ebx
	call	[ExitProcess]
@@:
if 0
	push	ebx
	push	ebx
	push	ebx
	call	[GetCurrentThreadId]
	push	eax
	call	[PostThreadMessageA]
	sub	esp, 40h
	mov	eax, esp
	push	ebx
	push	ebx
	push	ebx
	push	eax
	call	[GetMessageA]
	add	esp, 40h
end if
; create shared data
	push	ebx
	push	shared_section_size
	push	ebx
	push	4
	push	ebx
	push	-1
	call	[CreateFileMappingA]
	test	eax, eax
	jz	server_fail
	mov	[hSharedData], eax
	push	ebx
	push	ebx
	push	ebx
	call	[CreateMutexA]
	mov	esi, shared_mutex_create_err
	test	eax, eax
	jz	server_fail
	mov	[hSharedMutex], eax
	call	map_shared_data
	xor	ecx, ecx
	inc	ecx
	mov	dword [eax], ecx	; 1 process (OS/IDLE)
	mov	dword [eax+4], ecx	; 1 process
	mov	[eax+shared_data_struc.active_process], ecx
	mov	[eax+shared_data_struc.thread_id], ecx	; PID=1
	mov	[eax+shared_data_struc.sound_dma], ecx
	mov	[eax+shared_data_struc.fat32part], ecx
	mov	[eax+shared_data_struc.hd_base], cl
	mov     [eax+shared_data_struc.mouse_delay], 10
	mov     [eax+shared_data_struc.mouse_speed_factor], 3
	xchg	eax, esi
	mov	al, [vk]
	or	[esi+shared_data_struc.vk], al
	mov	[esi+shared_data_struc.pci_data_init], bl
; read ini file server settings
	mov	edi, aSetup
	push	win32_path
	push	ebx
	push	aSoundFlag
	push	edi
	call	[GetPrivateProfileIntA]
	mov	[esi+shared_data_struc.sound_flag], al
	push	win32_path
	push	1
	push	aSysLang
	push	edi
	call	[GetPrivateProfileIntA]
	mov	[esi+shared_data_struc.syslang], eax
	push	win32_path
	push	1
	push	aKeyboard
	push	edi
	call	[GetPrivateProfileIntA]
	mov	[esi+shared_data_struc.keyboard], ax
	mov	ebp, esi
	mov	ecx, 1028
	sub	esp, ecx
	mov	esi, esp
	mov	edi, aMain
	push    win32_path
	push    ecx
	push    esi
        push    null_string
        push    aAllowReadMSR
        push    edi
        call    [GetPrivateProfileStringA]
        cmp     byte [esi], 't'
        setz    [ebp+shared_data_struc.bAllowReadMSR]
	push	win32_path
	push	1028
	push	esi
	push	null_string
	push	aAllowReadPCI
	push	edi
	call	[GetPrivateProfileStringA]
	cmp	byte [esi], 't'
	setz	[ebp+shared_data_struc.bAllowReadPCI]
	setz	byte [ebp+shared_data_struc.pci_access_enabled]
	push	win32_path
	push	1028
	push	esi
	push	null_string
	push	aKeepLoadedDriver
	push	edi
	call	[GetPrivateProfileStringA]
	cmp	byte [esi], 't'
	setz	[keep_loaded_driver]
	push	win32_path
	push	1028
	push	esi
	push	null_string
	push	aEnablePorts
	push	edi
	call	[GetPrivateProfileStringA]
; parse EnablePorts parameter
	or	eax, -1
	lea	edi, [ebp+shared_data_struc.DisabledPorts]
	mov	ecx, 1000h
	rep	stosd
ParseEnablePorts:
	lodsb
	test	al, al
	jz	.done
	cmp	al, 0x20
	jbe	ParseEnablePorts
	call	read_hex
	cmp	al, '-'
	jz	.minus
	mov	edx, ecx
	shr	ecx, 3
	and	edx, 7
	btr	dword [ebp+shared_data_struc.DisabledPorts+ecx], edx
.x1:	test	al, al
	jz	.done
	cmp	al, 0x20
	jbe	ParseEnablePorts
.err:
	mov	esi, EnablePortsSyntaxErr
	jmp	server_fail
.minus:
	push	ecx
	lodsb
	call	read_hex
	cmp	ecx, [esp]
	jb	.err
	push	eax
@@:
	mov	eax, ecx
	shr	ecx, 3
	mov	edx, eax
	and	edx, 7
	btr	dword [ebp+shared_data_struc.DisabledPorts+ecx], edx
	test	eax, eax
	jz	@f
	lea	ecx, [eax-1]
	cmp	ecx, [esp+4]
	jae	@b
@@:
	pop	eax
	pop	ecx
	jmp	.x1
.done:
	add	esp, 1028
	xor	eax, eax
	cmp	[bIs9x], al
	jnz	.skipload
	cmp     [ebp+shared_data_struc.bAllowReadMSR], bl
	jnz     .load
	cmp	[ebp+shared_data_struc.bAllowReadPCI], bl
	jnz	.load
	mov	ecx, 2000h
	lea	edi, [ebp+shared_data_struc.DisabledPorts]
	mov	al, -1
	repz	scasb
	jz	.skipload
.load:
; load driver kiw0.sys
; note that this must execute after all work with ini-file
; because win32_path is overwritten
	call	load_kiw0
.skipload:
	call	create_child
debugloop:
	mov	eax, [newprg_section]
	cmp	byte [eax+1], 1
	jz	newprg_request
	push	500	; wait a half of second
	push	debugevent
	call	[WaitForDebugEvent]
	test	eax, eax
	jz	debugloop
; get hProcess
	mov	eax, [debugevent+4]
	mov	ecx, [pids]
@@:	cmp	[ecx+4], eax
	jz	@f
	mov	ecx, [ecx]
	jmp	@b
@@:	mov	eax, [ecx+8]
	mov	[hProcess], eax
; parse debug event
	mov	eax, [debugevent]	; dwDebugEventCode
	dec	eax	; EXCEPTION_DEBUG_EVENT = 1
	jz	exceptionevent
	dec	eax	; CREATE_THREAD_DEBUG_EVENT = 2
	jz	threadcreated
	dec	eax	; CREATE_PROCESS_DEBUG_EVENT = 3
	jz	processcreated
	dec	eax	; EXIT_THREAD_DEBUG_EVENT = 4
	jz	threadexited
	dec	eax	; EXIT_PROCESS_DEBUG_EVENT = 5
	jz	exited
debugcont:
	push	10002h	; DBG_CONTINUE
dodebugcont:
	push	[debugevent+8]
	push	[debugevent+4]
	call	[ContinueDebugEvent]
	jmp	debugloop
exited:
; delete Win32 pid and tid
	mov	eax, [debugevent+4]
	mov	ecx, pids
	call	delete_id
	call	find_tid
	jecxz	@f
	call	on_thread_exited
	mov	eax, [debugevent+8]
	mov	ecx, tids
	call	delete_id
@@:
; if all processes are done, exit
	dec	[num_kolibri_proc]
	jnz	debugcont
	jmp	server_done
threadcreated:
	mov	eax, [debugevent+12]
	mov	[hThread], eax
	mov	eax, [debugevent+8]
	mov	[dwThreadId], eax
	call	alloc_thread
	mov	eax, [debugevent+16]
	mov	ecx, [cur_tid_ptr]
	mov	[ecx+16], eax
	mov	[ecx+20], ebx
	jmp	debugcont
processcreated:
	call	find_tid
	test	ecx, ecx
	jz	debugcont
	push	[debugevent+12]
	call	[CloseHandle]
	mov	eax, [debugevent+24h]
	mov	ecx, [cur_tid_ptr]
	mov	[ecx+16], eax
	jmp	debugcont
threadexited:
	call	find_tid
	test	ecx, ecx
	jz	debugcont
	cmp	[cur_slot], -1
	jz	@f
	call	on_thread_exited
@@:
	mov	eax, [debugevent+8]
	mov	ecx, tids
	call	delete_id
	jmp	debugcont
exceptionevent:
	call	find_tid
	test	eax, eax
	jz	debugcont
; special handling of #PF exceptions in shared memory areas
	cmp	[debugevent+12], 0xC0000005
	jnz	.nopf
	mov	ecx, [debugevent+36]
	call	get_cur_slot_ptr_server
	mov	edi, [edi+shared_data_struc.shmem_list-shared_data_struc.threads]
.scanaddr:
	test	edi, edi
	jz	.nopf
	cmp	ecx, [edi+shmem_proc_descr.ptr]
	jb	@f
	cmp	ecx, [edi+shmem_proc_descr.end]
	jb	.pfshared
@@:
	mov	edi, [edi+shmem_proc_descr.next]
	jmp	.scanaddr
.pfshared:
; this is really exception in shared area
	mov	esi, [edi+shmem_proc_descr.item]
	mov	eax, [esi+shmem_item.pOwner]
	cmp	eax, ebx
	jz	.pfsh_noowner
	call	shmem_load
.pfsh_noowner:
	mov	edx, [edi+shmem_proc_descr.end]
	mov	ecx, [edi+shmem_proc_descr.ptr]
	sub	edx, ecx
	push	ecx edx
	push	eax
	push	esp
	push	4	; PAGE_READWRITE
	push	edx
	push	ecx
	push	[hProcess]
	call	[VirtualProtectEx]
	pop	eax
	pop	edx ecx
	push	ecx edx
	push	ebx
	push	edx
	push	[esi+shmem_item.ptr]
	push	ecx
	push	[hProcess]
	call	[WriteProcessMemory]
	pop	edx ecx
	cmp	dword [debugevent+32], ebx
	jz	.pfsh_read
	cmp	[edi+shmem_proc_descr.access], ebx
	jz	.nopf
	mov	[esi+shmem_item.pOwner], edi
	mov	eax, [hProcess]
	mov	[esi+shmem_item.hOwner], eax
	jmp	debugcont
.pfsh_read:
	push	eax
	push	esp
	push	2	; PAGE_READONLY
	push	edx
	push	ecx
	push	[hProcess]
	call	[VirtualProtectEx]
	pop	eax
	jmp	debugcont
.nopf:
; first exception is int3 in loader code
; ignore all exceptions before executing our code
; (there is one exception, debugging int3, in ntdll loader code,
;  this exception must be continued as handled)
	mov	edi, context
	push	edi
	push	[hThread]
	mov	dword [edi], 1000Fh
	call	[GetThreadContext]
	add	edi, 0xB8
; breakpoints int3 (0xCC): decrement EIP (incremented by Windows)
	cmp	[debugevent+12], 0x80000003
	jnz	@f
	dec	dword [edi]
@@:
; single-step exceptions: restore TF flag (cleared by Windows)
	mov	dx, cs
	mov	eax, [edi]
	mov	ecx, [cur_tid_ptr]
	cmp	[debugevent+12], 0x80000004
	jnz	.noss
	cmp	word [edi+4], dx
	jnz	.set_tf
	cmp	eax, exception+1
	jz	@f
.set_tf:
	or	byte [edi+8+1], 1
@@:
	cmp	[ecx+52], ebx
	mov	[ecx+52], ebx
	jnz	x
	cmp	word [edi+4], dx
	jnz	.noss
	cmp	eax, i40_done_jmp1
	jz	.skipnext
	cmp	eax, i40_done_jmp2
	jnz	@f
.skipnext:
	inc	dword [ecx+52]
@@:
	cmp	eax, exception+1
	jz	x
	cmp	eax, i40_done
	jb	.noss
	cmp	eax, not_supported_i40_fn
	jb	x
.noss:
	mov	[ecx+52], ebx
	mov	esi, tls_index
	push	eax
	push	esp
	push	4
	push	esi
	push	esi
	push	[hProcess]
	call	[ReadProcessMemory]
	mov	eax, [cur_tid_ptr]
	mov	eax, [eax+16]
	add	eax, 2Ch
	mov	ecx, esp
	push	ebx
	push	ecx
	sub	ecx, 4
	push	4
	push	ecx
	push	eax
	push	[hProcess]
	call	[ReadProcessMemory]
	pop	eax
	pop	ecx
	test	eax, eax
	jz	debugcont
	mov	ecx, [esi]
	cmp	ecx, -1
	jz	debugcont
	lea	eax, [eax+ecx*4]
	push	eax
	mov	ecx, esp
	push	ebx
	push	ecx
	sub	ecx, 4
	push	4
	push	ecx
	push	eax
	push	[hProcess]
	call	[ReadProcessMemory]
	pop	eax
	pop	ecx
; now eax -> TLS data
	xchg	eax, esi
	push	eax
	push	esp
	push	24
	push	_cs
	push	esi
	push	[hProcess]
	call	[ReadProcessMemory]
	pop	eax
	mov	ax, [_cs]
	test	ax, ax
	jz	debugcont
; test for exceptions in Kolibri code
	cmp	word [context+0xBC], ax
	jz	process_exception
; debugged process?
	mov	edx, [cur_tid_ptr]
	mov	edi, [edx+20]
	test	edi, edi
	jz	.nodebuggee
; yes
; int40?
	cmp	[debugevent+12], 0xC0000005
	jnz	.exception2dbg
	push	edx edi
	push	ebx
	mov	ecx, esp
	push	ebx
	push	esp
	push	4
	push	ecx
	push	base
	call	get_cur_slot_ptr_server
	push	[edi+shared_data_struc.win32_hBaseProcess-shared_data_struc.threads]
	call	[ReadProcessMemory]
	lea	ecx, [esp+4]
	push	esp
	mov	eax, [ecx]
	push	2
	add	eax, [context+0xB8]
	push	ecx
	push	eax
	push	[edi+shared_data_struc.win32_hBaseProcess-shared_data_struc.threads]
	call	[ReadProcessMemory]
	pop	eax
	pop	ecx
	pop	edi edx
	cmp	al, 2
	jnz	.exception2dbg
	cmp	cx, 0x40CD
	jz	.nodebuggee
; suspend current thread and notify debugger
.exception2dbg:
	push	dword [edx+8]
	call	[SuspendThread]
	push	context
	push	[hThread]
	call	[SetThreadContext]
	mov	eax, [edi+12]
	call	get_slot_ptr
	push	[debugevent+12]
	push	[cur_slot]
	push	401h
	push	[edi+shared_data_struc.hWnd-shared_data_struc.threads]
	call	init_MessageBox
	call	[PostMessageA]
	jmp	debugcont
.nodebuggee:
; set Win32 context
	mov	word [context+0xBC], ax
	mov	ax, [_ds]
	mov	word [context+0x98], ax
	mov	word [context+0x94], ax
	mov	word [context+0xC8], ax
	mov	ax, [_fs]
	mov	word [context+0x90], ax
;	mov	word [context+0x8C], 0
	mov	eax, [_eip]
	xchg	eax, [context+0xB8]
	mov	[_eip], eax
	mov	eax, [_esp]
	xchg	eax, [context+0xC4]
	mov	[_esp], eax
	mov	eax, [debugevent+12]
	mov	[exc_code], eax
	mov	eax, [debugevent+36]
	mov	[exc_data], eax
	push	eax
	push	esp
	push	24
	push	_cs
	push	esi
	push	[hProcess]
	call	[WriteProcessMemory]
	pop	eax
x:
	push	context
	push	[hThread]
	call	[SetThreadContext]
	jmp	debugcont
process_exception:
	mov	eax, [context+0xB8]
	cmp	eax, server_test
	jnz	.no_server_test
	mov	eax, [debugevent+4]
	mov	ecx, [pids]
@@:	cmp	[ecx+4], eax
	jz	@f
	mov	ecx, [ecx]
	jmp	@b
@@:
	mov	edi, [ecx+12]
	push	ebx
	push	1000h
	push	edi
	push	[context+0x9C]
	push	[hProcess]
	call	[WriteProcessMemory]
	push	edi
	call	free_big
	mov	eax, [cur_slot]
	mov	[context+0xB0], eax
	mov	eax, context+0xAC	; ecx
	mov	ecx, [hSharedData]
	call	DuplicateMyHandle
	mov	eax, context+0xA8	; edx
	mov	ecx, [hSharedMutex]
	call	DuplicateMyHandle
	jmp	add2
.no_server_test:
	cmp	eax, server_new_thread
	jnz	@f
	mov	eax, [debugevent+8]
	mov	[dwThreadId], eax
	call	new_kolibri_thread
	mov	eax, [cur_slot]
	mov	[context+0xAC], eax
	jmp	add2
@@:
	cmp	eax, server_run_prg
	jnz	@f
; create new process
	push	ebx
	push	4096
	push	process_curdir
	push	process_curdir
	push	[hProcess]
	call	[ReadProcessMemory]
	mov	eax, [context+0x9C]
	mov	edx, converted_path
	mov	edi, edx	; edi=name
	call	read_asciz
	mov	eax, [context+0xA0]
	xor	esi, esi	; esi=params
	test	eax, eax
	jz	.x
	mov	edx, win32_path
	mov	esi, edx
	call	read_asciz
.x:
	mov	eax, [cur_tid_ptr]
	mov	[parent_tid_ptr], eax
	push	2	; dwOptions = DUPLICATE_SAME_ACCESS
	push	ebx	; bInheritHandle
	push	ebx	; dwDesiredAccess
	push	context+0xAC	; lpTargetHandle
	push	[hProcess]
	push	eax
	call	new_kolibri_process
	pop	ecx
	mov	edx, [context+0xB0]	; flags for 70.7
	mov	[ecx+32], edx
	mov	[ecx+36], ebx
	mov	[ecx+40], ebx
;	mov	[context+0xB0], eax
	mov	[ecx+28], eax
	push	dword [ecx+24]
	call	[GetCurrentProcess]
	push	eax
	call	[DuplicateHandle]
	jmp	add2
@@:
	cmp	eax, server_get_run_result
	jnz	@f
	mov	esi, [cur_tid_ptr]
	push	dword [esi+24]
	call	[CloseHandle]
	mov	eax, [esi+28]
	mov	[context+0xB0], eax
	mov	eax, context+0xAC
	mov	ecx, [esi+36]
	call	DuplicateMyHandle
	mov	eax, context+0xA8
	mov	ecx, [esi+40]
	call	DuplicateMyHandle
	mov	eax, [esi+44]
	mov	[context+0xA0], eax
	mov	eax, [esi+48]
	mov	[context+0x9C], eax
	jmp	add2
@@:
	cmp	eax, set_wnd_colors
	jnz	@f
	mov	ecx, [context+0xA8]
	and	ecx, 7Fh
	push	ebx
	push	ecx
	push	common_colors
	push	[context+0xAC]
	push	[hProcess]
	call	[ReadProcessMemory]
	mov	[bCommonColorsSet], 1
add2:
	add	[context+0xB8], 2
	jmp	x
@@:
	cmp	eax, notify_parent
	jnz	nonotifyparent
	mov	eax, [cur_tid_ptr]
	mov	edi, [eax+20]
	test	edi, edi
	jz	add2
	test	byte [edi+32], 1
	jz	@f
	push	[hThread]
	call	[SuspendThread]
	mov	eax, [hProcess]
	mov	[edi+36], eax
	mov	eax, [hThread]
	mov	[edi+40], eax
	mov	eax, [context+0xB4]
	mov	[edi+44], eax
	mov	eax, [context+0xC4]
	mov	[edi+48], eax
	jmp	setparev
@@:
	mov	dword [eax+20], ebx
setparev:
	push	dword [edi+24]
	call	[SetEvent]
	jmp	add2
nonotifyparent:
	cmp	eax, loadfailed
	jnz	@f
	mov	ecx, [context+0xB0]
	mov	eax, [cur_tid_ptr]
	mov	edi, [eax+20]
	test	edi, edi
	jz	add2
	mov	[edi+28], ecx
	jmp	@b
@@:
	cmp	eax, get_wnd_colors
	jnz	@f
	mov	ecx, [context+0xA8]
	and	ecx, 7Fh
	push	ebx
	push	ecx
	push	common_colors
	push	[context+0xAC]
	push	[hProcess]
	call	[WriteProcessMemory]
	mov	al, [bCommonColorsSet]
	mov	byte [context+0xB0], al
	jmp	add2
@@:
	cmp	eax, set_button_style
	jnz	@f
	mov	ecx, [context+0xAC]
	cmp	cl, [buttontype]
	jz	add2
	mov	[buttontype], cl
	call	update_buttontype
	jmp	add2
@@:
	cmp	eax, server_send_ipc
	jnz	no_server_send_ipc
; find target slot
	mov	eax, [context+0xAC]	; ecx
	mov	esi, [shared_data]
	mov	ecx, [esi]
	add	esi, shared_data_struc.threads
@@:
	cmp	[esi], eax
	jz	@f
	add	esi, 64
	loop	@b
	mov	[context+0xB0], 4	; no such PID
	jmp	.done
@@:
	mov	eax, [esi+4]
	test	eax, eax
	jnz	@f
	mov	[context+0xB0], 1	; no IPC memory
	jmp	.done
@@:
	push	-1
	mov	ecx, esp
	push	ebx
	push	4
	push	ecx
	push	eax
	push	dword [esi+12]
	call	[ReadProcessMemory]
	pop	ecx
	jecxz	@f
	mov	[context+0xB0], 2	; IPC blocked
	jmp	.done
@@:
	push	ebx
	mov	eax, esp
	push	ebx
	push	4
	push	eax
	mov	eax, [esi+4]
	add	eax, 4
	push	eax
	push	dword [esi+12]
	call	[ReadProcessMemory]
	pop	eax
	mov	ecx, [esi+8]
	sub	ecx, 8
	sub	ecx, eax
	sub	ecx, [context+0xA0]	; esi = message length
	jns	@f
	mov	[context+0xB0], 3	; buffer overflow
	jmp	.done
@@:
	mov	edi, eax
	add	eax, 8
	add	eax, [context+0xA0]
	push	eax
	mov	eax, esp
	push	ebx
	push	4
	push	eax
	mov	eax, [esi+4]
	add	eax, 4
	push	eax
	push	dword [esi+12]
	call	[WriteProcessMemory]
	pop	eax
	add	edi, [esi+4]	; edi = pointer to place for our message
; message header: dd source_pid, dd size
	push	[context+0xA0]
	push	edi
	call	get_cur_slot_ptr_server
	mov	eax, [edi]
	pop	edi
	push	eax
	mov	eax, esp
	push	ebx
	push	8
	push	eax
	push	edi
	push	dword [esi+12]
	call	[WriteProcessMemory]
	pop	eax
	pop	eax
; now read message from source process and write it to target
	push	eax
	call	malloc
	xchg	eax, ebp
	push	ebx
	push	[context+0xA0]
	push	ebp
	push	[context+0xA8]
	push	[hProcess]
	call	[ReadProcessMemory]
	add	edi, 8
	push	ebx
	push	[context+0xA0]
	push	ebp
	push	edi
	push	dword [esi+12]
	call	[WriteProcessMemory]
	push	ebp
	call	free
	mov     [context+0xB0], ebx	; success
; now notify window of target handle
;	push	0
;	push	0
;	push	400h		; WM_USER
;	push	dword [esi+20]
;	call	[PostMessageA]
; let source thread to notify target window
	mov	eax, [esi+20]
	mov	[context+0xAC], eax
.done:
	jmp	add2
no_server_send_ipc:
	cmp	eax, server_convert
	jnz	no_server_convert
	mov	eax, context+0xB0	; eax
	mov	ecx, [eax]
	call	DuplicateMyHandle
	jmp	add2
no_server_convert:
	cmp	eax, ..server_create_shmem
	jnz	no_server_create_shmem
	sub	esp, 32
	mov	eax, esp
	push	ebx
	push	32
	push	eax
	push	dword [context+0xAC]	; ecx
	push	[hProcess]
	call	[ReadProcessMemory]
	test	eax, eax
	jnz	@f
.invparam:
	push	33	; E_PARAM
	pop	edx
.ret:
	add	esp, 32
	mov	dword [context+0xB0], edx	; eax
	jmp	add2
@@:
; scan for shared memory area with requested name
	mov	edx, [shmem_list]
.scan:
	mov	esi, esp
	cmp	edx, shmem_list - shmem_item.next
	jz	.new
	mov	edi, edx
	push	32
	pop	ecx
@@:
	lodsb
	scasb
	jnz	@f
	test	al, al
	loopnz	@b
@@:
	jz	.found
	mov	edx, [edx+shmem_item.next]
	jmp	.scan
.new:
; requested section was not found, create new if needed
	push	5	; E_NOTFOUND
	pop	edx
	mov	al, byte [context+0xA0]	; esi
	and	al, 0xC
	jz	.ret
	jp	.invparam
; create
	mov	eax, [context+0xA8]	; edx
	test	eax, eax
	jz	.invparam
	call	malloc_big
	push	30	; E_NOMEM
	pop	edx
	test	eax, eax
	jz	.ret
	push	eax
	push	shmem_item.sizeof
	call	malloc
	test	eax, eax
	jnz	@f
	call	free_big
	push	30
	pop	edx
	jmp	.ret
@@:
	mov	edi, eax
	push	32/4
	pop	ecx
	rep	movsd
	mov	ecx, [shmem_list]
	mov	[eax+shmem_item.next], ecx
	mov	[ecx+shmem_item.prev], eax
	mov	[eax+shmem_item.prev], shmem_list - shmem_item.next
	mov	[shmem_list], eax
	mov	[eax+shmem_item.refs], ebx
	pop	[eax+shmem_item.ptr]
	push	[context+0xA8]
	pop	[eax+shmem_item.size]
	mov	[eax+shmem_item.hOwner], ebx
	mov	[eax+shmem_item.pOwner], ebx
	mov	esi, eax
	mov	eax, [context+0xA0]	; esi
	and	eax, 1
	mov	[esi+shmem_item.access], eax
	jmp	.created
.found:
	mov	esi, edx
	push	10	; E_ACCESS
	pop	edx
	mov	al, byte [context+0xA0]	; esi
	and	al, 0xC
	jz	@f
	jp	.invparam
	cmp	al, 8
	jz	.ret
@@:
	test	byte [context+0xA0], 1
	jz	.created
	cmp	[esi+shmem_item.access], ebx
	jz	.ret
.created:
	inc	[esi+shmem_item.refs]
; section ok, now create descriptor for address space in target process
	push	shmem_proc_descr.sizeof
	call	malloc
	test	eax, eax
	jnz	@f
	push	[esi+shmem_item.next]
	pop	[shmem_list]
	push	esi
	push	[esi+shmem_item.ptr]
	call	free_big
	call	free
	push	30
	pop	edx
	jmp	.ret
@@:
	mov	[eax+shmem_proc_descr.item], esi
	mov	[eax+shmem_proc_descr.ptr], ebx
	mov	[eax+shmem_proc_descr.end], ebx
	xor	edx, edx
	test	byte [context+0xA0], 1
	jz	@f
	inc	edx
@@:
	cmp	[esi+shmem_item.refs], 1
	jnz	@f
	mov	dl, 1
@@:
	mov	[eax+shmem_proc_descr.access], edx
; no need to synchronize - only server uses this list
	call	get_cur_slot_ptr_server
	push	[edi+shared_data_struc.shmem_list-shared_data_struc.threads]
	pop	[eax+shmem_proc_descr.next]
	mov	[edi+shared_data_struc.shmem_list-shared_data_struc.threads], eax
; all is OK, return to caller
	mov	[context+0xB0], ebx	; eax
	mov	eax, [esi+shmem_item.size]
	mov	[context+0xAC], eax	; ecx
	add	eax, 0xFFF
	and	eax, not 0xFFF
	cmp	[esi+shmem_item.refs], 1
	jnz	@f
	xor	eax, eax
@@:
	mov	[context+0xA8], eax	; edx
	add	esp, 32
	jmp	add2
no_server_create_shmem:
	cmp	eax, ..server_notify_shmem
	jnz	no_server_notify_shmem
	call	get_cur_slot_ptr_server
	mov	edi, [edi+shared_data_struc.shmem_list-shared_data_struc.threads]
	push	[context+0xB0]	; eax
	pop	[edi+shmem_proc_descr.ptr]
	mov	eax, [edi+shmem_proc_descr.item]
	mov	eax, [eax+shmem_item.size]
	add	eax, 0xFFF
	and	eax, not 0xFFF
	add	eax, [edi+shmem_proc_descr.ptr]
	mov	[edi+shmem_proc_descr.end], eax
	jmp	add2
no_server_notify_shmem:
	cmp	eax, ..server_destroy_shmem
	jnz	no_server_destroy_shmem
	sub	esp, 32
	mov	eax, esp
	push	ebx
	push	32
	push	eax
	push	dword [context+0xAC]	; ecx
	push	[hProcess]
	call	[ReadProcessMemory]
	mov	[context+0xAC], ebx
	test	eax, eax
	jz	.ret
	call	get_cur_slot_ptr_server
	lea	ebp, [edi+shared_data_struc.shmem_list-shared_data_struc.threads - shmem_proc_descr.next]
.scan:
	mov	edx, [ebp+shmem_proc_descr.next]
	test	edx, edx
	jz	.ret
	mov	esi, [edx+shmem_proc_descr.item]
	mov	edi, esp
	push	32
	pop	ecx
@@:
	lodsb
	scasb
	jnz	@f
	test	al, al
	loopnz	@b
@@:
	jz	.found
	mov	ebp, edx
	jmp	.scan
.found:
	push	[edx+shmem_proc_descr.next]
	pop	[ebp+shmem_proc_descr.next]
	push	[edx+shmem_proc_descr.ptr]
	pop	[context+0xAC]		; ecx
	mov	esi, [edx+shmem_proc_descr.item]
	push	edx
	call	free
	dec	[esi+shmem_item.refs]
	jnz	.ret
	call	shmem_free_item
.ret:
	add	esp, 32
	jmp	add2
no_server_destroy_shmem:
	cmp	eax, i40_sys_service.server_terminate
	jz	server_done
no_server_terminate:
if idletime_via_ring0
	cmp	eax, i40_sys_service.idlecount_init
	jnz	@f
	push	eax
	push	esp
	push	ebx
	push	[cur_slot]
	push	idlecount_thread
	push	2000h
	push	ebx
	call	[CreateThread]
	pop	eax
	mov	eax, [shared_data]
	mov	[eax+shared_data_struc.idlecount], 1000
	mov	[eax+shared_data_struc.b9xPerfInited], 1
	jmp	add2
@@:
end if
cont_nh:
; ignore int3 breaks (continue as handled)
	cmp	[debugevent+12], 80000003h	; EXCEPTION_BREAKPOINT
	jz	debugcont
if 1
; ignore first-chance exceptions (continue as not handled)
        cmp     dword [debugevent+0x5C], ebx
        jnz     .first_chance
	mov	eax, context
	int3
	mov	eax, [context+0xB8]
	mov	edi, eeeeip+7
	std
	mov	ecx, 8
@@:
	mov	edx, eax
	and	al, 0xF
	cmp	al, 10
	sbb	al, 69h
	das
	stosb
	mov	eax, edx
	shr	eax, 4
	loop	@b
	cld
	call	init_MessageBox
	push	ebx
	push	ebx
	push	eee
	push	ebx
	call	[MessageBoxA]
.first_chance:
end if
	push	80010001h	; DBG_EXCEPTION_NOT_HANDLED
	jmp	dodebugcont

find_tid:
; get hThread
	mov	[hThread], ebx
	mov	[cur_slot], ebx
	mov	eax, [debugevent+8]
	mov	ecx, [tids]
@@:	jecxz	.ret
	cmp	[ecx+4], eax
	jz	@f
	mov	ecx, [ecx]
	jmp	@b
@@:	mov	eax, [ecx+12]
	mov	[cur_slot], eax
	mov	eax, [ecx+8]
	mov	[hThread], eax
	mov	[cur_tid_ptr], ecx
.ret:
	ret

read_asciz:
; in: eax=client pointer, edx->buffer
	push	eax
	push	edx
	push	eax
	push	esp
	push	260
	push	edx
	push	eax
	push	[hProcess]
	call	[ReadProcessMemory]
	test	eax, eax
	pop	ecx
	pop	edx
	pop	eax
	jnz	@f
	xor	ecx, ecx
@@:	mov	byte [edx+ecx], bl
	ret

create_child:
	mov	edi, inname
	mov	esi, [parameters]

new_kolibri_process_with_default_curdir:
	mov	dword [process_curdir], '/rd/'
	mov	word [process_curdir+4], '1'
	mov	[parent_tid_ptr], ebx

new_kolibri_process:
; in: edi=pointer to process name, esi=pointer to parameters
; create command line
	mov	[process_name], edi
	push	esi
	push	edi
	mov	edi, cmdline
	mov	al, '"'
	stosb
	push	2000
	push	edi
	push	ebx
	call	[GetModuleFileNameA]
	add	edi, eax
	mov	ax, '" '
	stosw
	mov	al, '"'
	stosb
	pop	esi
	push	esi
	call	[lstrlenA]
	xchg	eax, ecx
	rep	movsb
	mov	al, '"'
	stosb
	mov	al, ' '
	stosb
	pop	esi
	test	esi, esi
	jz	@f
	push	esi
	call	[lstrlenA]
	xchg	eax, ecx
	rep	movsb
@@:
	xor	eax, eax
	stosb
; create process
	push	[hThread]
	push	pinfo
	push	sinfo
	push	ebx
	push	ebx
	push	ebx
	cmp	[bDontDebug], bl
	jnz	@f
	pop	ebx
	push	2	; DEBUG_ONLY_THIS_PROCESS
@@:
	push	ebx
	push	ebx
	push	ebx
	push	cmdline
	push	ebx
	call	[CreateProcessA]
	test	eax, eax
	jnz	@f
	call	init_MessageBox
	push	ebx
	push	ebx
	push	cpe
	push	ebx
	call	[MessageBoxA]
	pop	eax
	xor	eax, eax
	dec	eax
	ret
@@:
	cmp	[bDontDebug], bl
	jz	@f
	pop	eax
	ret
@@:
	mov	eax, pids
@@:	mov	ecx, [eax]
	jecxz	@f
	xchg	eax, ecx
	jmp	@b
@@:	push	eax
	push	16
	call	malloc
	pop	ecx
	mov	[ecx], eax
	mov	[eax], ebx
	mov	ecx, [dwProcessId]
	mov	[eax+4], ecx
	mov	ecx, [hProcess]
	mov	[eax+8], ecx
	push	eax
	mov	eax, 1000h
	call	malloc_big
	pop	ecx
	mov	[ecx+12], eax
	mov	edi, eax
	mov	esi, process_curdir
	mov	ecx, 1000h/4
	rep	movsd
	call	alloc_thread
	call	new_kolibri_thread
	push	eax
	add	edi, 8
	mov	esi, [process_name]
	mov	edx, esi
.1:
	lodsb
	cmp	al, '\'
	jnz	@f
	mov	edx, esi
@@:	cmp	al, 0
	jnz	.1
	mov	esi, edx
	mov	ecx, 11
	push	ecx edi
	mov	al, ' '
	rep	stosb
	pop	edi ecx
	push	edi
.s:
	lodsb
	test	al, al
	jz	.d
	cmp	al, '.'
	jnz	@f
	mov	edi, [esp]
	add	edi, 8
	mov	cl, 3
	jmp	.s
@@:
	cmp	al, 'a'
	jb	@f
	cmp	al, 'z'
	ja	@f
	and	al, not 20h
@@:	stosb
	loop	.s
.d:
	pop	edi
	add	edi, 11
	mov	byte [edi], 0
	pop	eax
	pop	[hThread]
	inc	[num_kolibri_proc]
	ret
alloc_thread:
	mov	eax, tids
@@:	mov	ecx, [eax]
	jecxz	@f
	xchg	eax, ecx
	jmp	@b
@@:	push	eax
	push	56
	call	malloc
	pop	ecx
	mov	[ecx], eax
	mov	[eax], ebx
	mov	ecx, [dwThreadId]
	mov	[eax+4], ecx
	mov	ecx, [hThread]
	mov	[eax+8], ecx
	or	dword [eax+12], -1
	mov	ecx, [parent_tid_ptr]
	mov	[eax+20], ecx
	mov	[eax+52], ebx
	mov	[cur_tid_ptr], eax
	push	ecx
	push	ebx	; lpName
	push	ebx	; bInitialState
	push	ebx	; bManualReset
	push	ebx	; lpEventAttributes
	call	[CreateEventA]
	pop	ecx
	jecxz	@f
	mov	[ecx+24], eax
@@:
	ret
new_kolibri_thread:
; find free slot
	mov	edi, [shared_data]
	inc	dword [edi+4]
	mov	ecx, [edi]
	add	edi, shared_data_struc.threads
	xor	edx, edx
@@:
	cmp	dword [edi], 0
	jz	@f
	inc	edx
	add	edi, 64
	loop	@b
	mov	ecx, [shared_data]
	inc	dword [ecx]
@@:
	mov	eax, [cur_tid_ptr]
	mov	[eax+12], edx
	mov	[cur_slot], edx
	mov	eax, [max_pid]
	inc	eax
	mov	[max_pid], eax
	stosd
	push	eax
	xor	eax, eax
	stosd
	stosd
	mov	eax, [hProcess]
	stosd
	mov	eax, [dwThreadId]
	stosd
	push	edi
	add	edi, 20
	mov	eax, [hThread]
	stosd
	xor	eax, eax
	stosd
	stosd
	stosd
	stosd
	stosd
	pop	edi
	pop	eax
	ret

delete_id:
@@:	mov	edx, [ecx]
	cmp	[edx+4], eax
	jz	@f
	mov	ecx, edx
	jmp	@b
@@:
	push	dword [edx]
	push	ecx
	push	edx
	push	ebx
	push	[hHeap]
;	push	dword [edx+8]
;	call	[CloseHandle]
	call	[HeapFree]
	pop	ecx
	pop	dword [ecx]
.ret:
	ret

on_thread_exited:
	mov	ecx, [cur_tid_ptr]
; send notification message to debugger, if it is present
	mov	edi, [ecx+20]
	test	edi, edi
	jz	@f
	push	ecx
	mov	eax, [edi+12]
	call	get_slot_ptr
	push	ebx
	push	[cur_slot]
	push	402h
	push	[edi+shared_data_struc.hWnd-shared_data_struc.threads]
	call	init_MessageBox
	call	[PostMessageA]
	pop	ecx
@@:
; terminate all debuggees, if are
	mov	esi, tids
@@:
	mov	esi, [esi]
	test	esi, esi
	jz	@f
	cmp	[esi+20], ecx
	jnz	@b
	push	ecx
	push	ebx
	push	dword [esi+8]
	call	[TerminateThread]
	pop	ecx
	jmp	@b
@@:
; free all shared memory
	call	get_cur_slot_ptr_server
	mov	edi, [edi+shared_data_struc.shmem_list-shared_data_struc.threads]
.freeshared:
	test	edi, edi
	jz	.doneshared
	push	[edi+shmem_proc_descr.next]
	mov	esi, [edi+shmem_proc_descr.item]
	dec	[esi+shmem_item.refs]
	jz	.freephys
	cmp	[esi+shmem_item.pOwner], edi
	jnz	.nextshared
	call	shmem_load
	jmp	.nextshared
.freephys:
	call	shmem_free_item
.nextshared:
	push	edi
	call	free
	pop	edi
	jmp	.freeshared
.doneshared:
; create thread to do rest of job (part which must be done with SharedDataMutex acquired)
; it is not safe to acquire_shared right here, because of possible deadlock
	push	eax
	push	esp
	push	ebx
	push	[cur_slot]
	push	on_thread_exited_thread
	push	10000h
	push	ebx
	call	[CreateThread]
	pop	eax
	ret

on_thread_exited_thread:
	call	acquire_shared
	mov	eax, [shared_data]
	dec	dword [eax+4]
	mov	eax, [esp+4]
	call	get_slot_ptr
	and	dword [edi], 0
; avoid problems with @panel
	mov	eax, '    '
	add	edi, 28
	stosd
	stosd
	stosd
	call	release_shared
	push	0
	call	[ExitThread]

if idletime_via_ring0
idlecount_thread:
	mov	eax, .count
	call	CallRing0
.workloop:
	mov	esi, eax
	push	1000
	call	[Sleep]
	mov	eax, .count
	call	CallRing0
	sub	esi, eax
;	add	esi, 1000
;	jc	@f
;	mov	esi, 1000
;@@:
	neg	esi
	cmp	esi, 1000
	jb	@f
	mov	esi, 1000
@@:
	mov	ecx, [shared_data]
	mov	[ecx+shared_data_struc.idlecount], esi
	jmp	.workloop
.count:
	push	edi
;	xor	edx, edx
	int	0x20	; VMMCall Get_Sys_Thread_Handle
	dw	10Ah
	dw	1
	push	edi
	int	0x20	; VMMCall _GetThreadExecTime
	dw	106h
	dw	1
	pop	edi
;	int	0x20	; VMMCall Get_Sys_Thread_Handle
;	dw	10Ah
;	dw	1
;@@:
;	int	0x20	; VMMCall Get_Next_Thread_Handle
;	dw	113h
;	dw	1
;	int	0x20	; VMMCall Test_Sys_Thread_Handle
;	dw	10Bh
;	dw	1
;	jz	@f
;	push	edi
;	int	0x20	; VMMCall _GetThreadExecTime
;	dw	106h
;	dw	1
;	add	edx, eax
;	pop	eax
;	jmp	@b
;@@:
;	mov	eax, edx
	pop	edi
	iret
end if

update_buttontype:
	mov	esi, [pids]
@@:
	test	esi, esi
	jz	.done
	push	0
	push	1
	push	buttontype
	push	buttontype
	push	dword [esi+8]
	call	[WriteProcessMemory]
	mov	esi, [esi]
	jmp	@b
.done:
	ret

init_background:
	push	-1
	push	[hBgrMutex]
	call	[WaitForSingleObject]
	cmp	[bgr_section], 0
	jnz	.ret
	push	ebx
	call	get_screen_size
	movzx	eax, bx
	shr	ebx, 16
	inc	eax
	inc	ebx
	mul	ebx
	imul	eax, 3
	pop	ebx
	add	eax, 20h
	push	bgr_section_name
;	push	0
	push	eax
	push	0
	push	4
	push	0
	push	-1
	call	[CreateFileMappingA]
	test	eax, eax
	jz	.ret
	xchg	eax, esi
	call	[GetLastError]
	push	eax
	push	0
	push	0
	push	0
	push	2
	push	esi
	call	[MapViewOfFile]
	push	eax
;	push	esi
;	call	[CloseHandle]
	pop	eax
	mov	[bgr_section], eax
	test	eax, eax
	xchg	eax, edi
	pop	eax
	jz	.ret
	cmp	eax, 183
	jz	.ret
; init background data
	call	get_screen_size
	xor	eax, eax
	shld	eax, ebx, 16
	inc	eax
	stosd
	mov	ax, bx
	inc	eax
	stosd
	mov	byte [edi], 2
	add	edi, 8
	xor	ebx, ebx
;	cmp	byte [esp+4], 0
;	jz	read_bgr
	jmp	read_bgr
.ret:
	push	[hBgrMutex]
	call	[ReleaseMutex]
	ret	4

read_bgr:
; read and parse desktop background to edi (=[bgr_section]+10h)
;	call	[GetDesktopWindow]
	xor	eax, eax
	push	eax
	push	eax
	call	[GetDC]
	push	eax
	push	eax
	call	[CreateCompatibleDC]
	xchg	eax, esi
	push	dword [edi-0Ch]
	push	dword [edi-10h]
	push	dword [esp+8]
	call	[CreateCompatibleBitmap]
	push	eax
	push	esi
	call	[SelectObject]
	push	eax
	push	0xCC0020
	push	ebx
	push	ebx
	push	dword [esp+16]
	push	dword [edi-0Ch]
	push	dword [edi-10h]
	push	ebx
	push	ebx
	push	esi
	call	[BitBlt]
	push	dword [esp+4]
	call	[PaintDesktop]
	push	0x660046
	push	ebx
	push	ebx
	push	dword [esp+16]
	push	dword [edi-0Ch]
	push	dword [edi-10h]
	push	ebx
	push	ebx
	push	esi
	call	[BitBlt]
	push	0x660046
	push	ebx
	push	ebx
	push	esi
	push	dword [edi-0Ch]
	push	dword [edi-10h]
	push	ebx
	push	ebx
	push	dword [esp+36]
	call	[BitBlt]
	push	0x660046
	push	ebx
	push	ebx
	push	dword [esp+16]
	push	dword [edi-0Ch]
	push	dword [edi-10h]
	push	ebx
	push	ebx
	push	esi
	call	[BitBlt]
	push	esi
	call	[SelectObject]
	push	ebp
	xchg	eax, ebp
; now esi=hDC, ebp=hBitmap
	push	ebx	; biClrImportant
	push	ebx	; biClrUsed
	push	ebx	; biYPelsPerMeter
	push	ebx	; biXPelsPerMeter
	push	ebx	; biSizeImage
	push	ebx	; biCompression
	push	200001h	; biBitCount, biPlanes
	push	dword [edi-0Ch]	; biHeight
	push	dword [edi-10h]	; biWidth
	push	40	; biSize
	mov	ecx, esp
	push	ebx
	push	ecx
	mov	eax, [edi-0Ch]
	mul	dword [edi-10h]
	shl	eax, 2
	call	malloc_big
	push	eax
	push	dword [edi-0Ch]
	push	ebx
	push	ebp
	push	esi
	xchg	eax, edi
	call	[GetDIBits]
	add	esp, 40
	push	ebp
	call	[DeleteObject]
	pop	ebp
	push	esi
	call	[DeleteDC]
	pop	eax
	pop	ecx
	push	eax
	push	ecx
	call	[ReleaseDC]
	mov	esi, [bgr_section]
	mov	eax, [esi]	; width
	mov	ecx, [esi+4]	; height
	add	esi, 10h
	xchg	esi, edi
; esi=source, edi=destination
	push	eax
	mul	ecx
	shl	eax, 2
	add	esi, eax
	pop	edx
.1:
	push	ecx
	mov	ecx, edx
	neg	ecx
	lea	esi, [esi+ecx*4]
	neg	ecx
	push	esi
.2:
	lodsd
;	call	convert_color
	stosd
	dec	edi
	loop	.2
	pop	esi
	pop	ecx
	loop	.1
	push	esi
	call	free_big
	push	[hBgrMutex]
	call	[ReleaseMutex]
	ret	4

del_background:
	call	[GetTickCount]
	mov	ecx, [shared_data]
	cmp	eax, [ecx+shared_data_struc.dwNewBgrTime]
	jb	.ret
	add	eax, 3000
	mov	[ecx+shared_data_struc.dwNewBgrTime], eax
	xor	eax, eax
lock	xchg	eax, [bgr_section]
	test	eax, eax
	jz	.ret
	push	eax
	call	[UnmapViewOfFile]
.ret:
	ret

read_hex:
	xor	ecx, ecx
.l:
	cmp	al, '0'
	jb	.done
	cmp	al, '9'
	jbe	.digit
	cmp	al, 'A'
	jb	.done
	cmp	al, 'F'
	jbe	.digit2
	cmp	al, 'a'
	jb	.done
	cmp	al, 'f'
	jbe	.digit3
.done:
	ret
.digit3:
	sub	al, 0x20
.digit2:
	sub	al, 'A'-'0'-10
.digit:
	sub	al, '0'
	movzx	eax, al
	shl	ecx, 4
	add	ecx, eax
	cmp	ecx, 0x10000
	jae	ParseEnablePorts.err
	lodsb
	jmp	.l

send_driver_request:
	xor	ebx, ebx
	push	ebx
	push	ebx
	push	3
	push	ebx
	push	ebx
	push	0xC0000000
	push	kiw0
	call	[CreateFileA]
	inc	eax
	jz	.ret
	dec	eax
	push	eax
	push	eax
	mov	ecx, esp
	push	ebx	; lpOverlapped
	push	ecx	; lpBytesReturned
	push	dword [ecx+8+20]	; nOutBufferSize
	push	dword [ecx+8+16]	; lpOutBuffer
	push	dword [ecx+8+12]	; nInBufferSize
	push	dword [ecx+8+8]	; lpInBuffer
	push	dword [ecx+8+4]	; dwIoControlCode
	push	eax
	call	[DeviceIoControl]
	pop	ecx
	pop	ecx
	push	eax
	push	ecx
	call	[CloseHandle]
	pop	eax
.ret:
	ret	20

driver_via_scm = 0
REQUIRED_DRIVER_VERSION = 1

load_kiw0:
; check whether driver with required version is already loaded
	push	eax
	mov	eax, esp
	push	4
	push	eax
	push	ebx
	push	ebx
	push	0x222008
	call	send_driver_request
	test	eax, eax
	pop	edi
	jz	.load
if driver_via_scm
	push	3	; dwDesiredAccess = SC_MANAGER_CONNECT+SC_MANAGER_CREATE_SERVICE
	cmp	edi, REQUIRED_DRIVER_VERSION
	jnz	.open
	pop	eax
	cmp	[keep_loaded_driver], 0
	jnz	.noopen
	push	1	; dwDesiredAccess = SC_MANAGER_CONNECT
.open:
	mov	esi, DrvLoadErr
	push	ebx	; lpDatabaseName
	push	ebx	; lpMachineName
	call	[OpenSCManagerA]
	test	eax, eax
	jz	server_fail
	mov	[hSCManager], eax
	push	10030h
	push	kiw0_drivername
	push	[hSCManager]
	call	[OpenServiceA]
	test	eax, eax
	jz	server_fail
	mov	[hService], eax
.noopen:
	cmp	edi, REQUIRED_DRIVER_VERSION
	jz	.driverok
; driver is loaded, but has incorrect version
; try to unload and load new driver
	call	unload_kiw0
	jmp	@f

.load:
	mov	esi, DrvLoadErr
	push	2	; dwDesiredAccess = SC_MANAGER_CREATE_SERVICE
	push	ebx	; lpDatabaseName
	push	ebx	; lpMachineName
	call	[OpenSCManagerA]
	test	eax, eax
	jz	server_fail
	mov	[hSCManager], eax
@@:
	mov	edi, win32_path
	push	edi
	push	edi
	call	[lstrlenA]
	lea	edi, [edi+eax+1-inifilenamesize]
	push	esi
	mov	esi, kiw0filename
	mov	ecx, kiw0filenamesize
	rep	movsb
	pop	esi
	pop	edi
	push	ebx	; lpPassword
	push	ebx	; lpServiceStartName
	push	ebx	; lpDependencies
	push	ebx	; lpdwTagId
	push	ebx	; lpLoadOrderGroup
	push	edi	; lpBinaryPathName
	push	ebx	; dwErrorControl = SERVICE_ERROR_IGNORE
	push	3	; dwStartType = SERVICE_DEMAND_START
	push	1	; dwServiceType = SERVICE_KERNEL_DRIVER
	push	10030h	; dwDesiredAccess = SERVICE_START or SERVICE_STOP or DELETE
	push	kiw0_username	; lpDisplayName
	push	kiw0_drivername	; lpServiceName
	push	[hSCManager]
	call	[CreateServiceA]
;	test	eax, eax
;	jnz	.cont
;	call	[GetLastError]
;	cmp	eax, 431h	; ERROR_SERVICE_EXISTS
;	jnz	server_fail
;	push	10030h
;	push	kiw0_drivername
;	push	[hSCManager]
;	call	[OpenServiceA]
	test	eax, eax
	jz	server_fail
.cont:
	mov	[hService], eax
	push	ebx	; lpServiceArgVectors
	push	ebx	; dwNumServiceArgs
	push	eax
	call	[StartServiceA]
	test	eax, eax
	jz	server_fail
.driverok:
	ret

unload_kiw0:
	sub	esp, 20h
	push	esp
	push	1	; SERVICE_CONTROL_STOP
	push	[hService]
	call	[ControlService]
	add	esp, 20h
	push	[hService]
	call	[DeleteService]
	push	[hService]
	call	[CloseServiceHandle]
	ret

server_done:
	cmp	[hService], 0
	jz	.skip_drv
	cmp	[keep_loaded_driver], 0
	jnz	.skip_drv
	call	unload_kiw0
	push	[hSCManager]
	call	[CloseServiceHandle]
.skip_drv:

else
	cmp	edi, REQUIRED_DRIVER_VERSION
	jz	load_kiw0.driverok
	call	unload_kiw0
load_kiw0.load:
        mov     esi, DrvLoadErr
        push    eax
        mov     eax, esp
        xor     ecx, ecx
        push    ecx     ; lpdwDisposition
        push    eax     ; phkResult
        push    ecx     ; lpSecurityAttributes
        push    6       ; samDesired = KEY_SET_VALUE | KEY_CREATE_SUB_KEY
        push    ecx     ; dwOptions
        push    ecx     ; lpClass
        push    ecx     ; Reserved
        push    DrvKey  ; lpSubKey
        push    0x80000002      ; hKey = HKEY_LOCAL_MACHINE
        call    [RegCreateKeyExA]
        test    eax, eax
        jnz     server_fail
        push    esi
        mov     esi, win32_path
        push    esi
        call    [lstrlenA]
        lea     esi, [esi+eax-1]
        lea     edi, [esi+4]
        mov     ecx, eax
        push    edi
        std
        rep     movsb
        cld
        mov     dword [edi-3], '\??\'
        pop     edi
        sub     edi, inifilenamesize-2
        mov     esi, kiw0filename
        mov     ecx, kiw0filenamesize
        rep     movsb
        sub     edi, win32_path+1
        mov     [drvpathlen], edi
        pop     esi
        mov     edi, DrvKeyValues
.write_values:
        push    dword [edi+12]  ; cbData
        push    dword [edi+8]   ; lpData
        push    dword [edi+4]   ; dwType
        push    0               ; Reserved
        push    dword [edi]     ; lpValueName
        push    dword [esp+20]  ; hKey
        call    [RegSetValueExA]
        test    eax, eax
        jz      @f
        call    [RegCloseKey]
.del_fail:
        push    DrvKey
        push    0x80000002
        call    [RegDeleteKeyA]
        jmp     server_fail
@@:
        add     edi, 16
        cmp     dword [edi], 0
        jnz     .write_values
        call    [RegCloseKey]
; NtLoadDriver and NtUnloadDriver require SeLoadPrivilege enabled.
; But I found that if user has this privilege, then it is already enabled
; (unlike things like SeShutdownPrivilege - in such cases there must be
;  additional code with AdjustTokenPrivileges(OpenProcessToken(...),LookupPrivilegeValue(...),...))
	push	ntdll_name
	call	[GetModuleHandleA]
	push	aNtLoadDriver
	push	eax
	call	[GetProcAddress]
	push    DrvKeySys
	call    eax
	test    eax, eax
	js      .del_fail
load_kiw0.driverok:
	mov	[bDriverLoaded], 1
	ret

unload_kiw0:
; Unload and delete driver kiw0.sys
; 1. Unload
        push    ntdll_name
        call    [GetModuleHandleA]
        push    aNtUnloadDriver
        push    eax
        call    [GetProcAddress]
        push    DrvKeySys
        call    eax
; 2. When the kernel loads driver, it (kernel) creates auxiliary reg keys
; in HKLM\System\CurrentControlSet\Enum\
; (for legacy drivers such as kiw0, this is Root\LEGACY_KIW0\<instance>)
; To delete this key and possibly reenumerate, call umpnpmgr.DeleteServicePlugPlayRegKeys
; In Win2k the library umpnpmgr.dll doesn't export this function,
; but under Win2k there is impossible to delete this key, because it is still opened by the kernel
        mov     esi, aCannotLoadDll
        push    umpnpmgr_name
        call    [LoadLibraryA]
        test    eax, eax
        jz      .nodll
        push    eax
        push    umpnpmgr_uninst
        push    eax
        call    [GetProcAddress]
        test    eax, eax
        jz      @f
        push    kiw0_unicode
        call    eax
@@:
        call    [FreeLibrary]
.nodll:
; 3. Delete main registry key, HKLM\System\CurrentControlSet\Services\kiw0
;    (and created by the kernel subkey Enum)
        push    DrvKeyEnum
        push    0x80000002
        call    [RegDeleteKeyA]
        push    DrvKey
        push    0x80000002
        call    [RegDeleteKeyA]
.ret:
	ret

server_done:
	cmp	[bDriverLoaded], 0
	jz	.skip_drv
	cmp	[keep_loaded_driver], 0
	jnz	.skip_drv
	call	unload_kiw0
.skip_drv:
end if

	cmp	[bIs9x], 0
	jz	server_done_perf
	mov	eax, [shared_data]
	cmp	[eax+shared_data_struc.b9xPerfInited], 0
	jz	server_done_perf
if ~idletime_via_ring0
	push	eax
	push	esp	; phkResult
	push	1	; samDesired = KEY_QUERY_VALUE
	push	ebx	; ulOptions
	push	perfend	; lpSubKey
	push	80000006h	; hKey = HKEY_DYN_DATA
	call	[RegOpenKeyExA]
	pop	esi
	test	eax, eax
	jnz	server_done_perf
	push	eax
	mov	eax, esp
	push	4
	push	esp	; lpcbData
	push	eax	; lpData
	push	ebx	; lpType
	push	ebx	; lpReserved
	push	perfval	; lpValueName
	push	esi	; hKey
	call	[RegQueryValueExA]
	pop	ecx
	pop	ecx
	push	esi
	call	[RegCloseKey]
end if
server_done_perf:
        push	ebx
        call	[ExitProcess]

PlaySoundA_delayed_imp:
	push	winmm_name
	call	[LoadLibraryA]
	test	eax, eax
	jz	.fail
	push	eax
	push	aPlaySoundA
	push	eax
	call	[GetProcAddress]
	pop	ecx
	test	eax, eax
	jz	.fail_free
	mov	[PlaySoundA], eax
	jmp	eax
.fail_free:
	push	ecx
	call	[FreeLibrary]
.fail:
	mov	[PlaySoundA], @f
@@:
	xor	eax, eax
	ret	12

init_dll:
	push	dword [esp+4]
	call	[LoadLibraryA]
	xchg	edi, eax
	mov	esi, [esp+8]
@@:
	lodsd
	test	eax, eax
	jz	@f
	add	eax, 0x400002
	push	eax
	push	edi
	call	[GetProcAddress]
	mov	[esi-4], eax
	jmp	@b
@@:
	ret	8

init_MessageBox:
	cmp	[MessageBoxA], rva MessageBoxA_thunk
	jnz	@f
	push	user32_thunks
	push	user32_name
	call	init_dll
@@:
	ret

DuplicateMyHandle:
	jecxz	@f
	push	2	; DUPLICATE_SAME_ACCESS
	push	ebx
	push	ebx
	push	eax
	push	[hProcess]
	push	ecx
	call	[GetCurrentProcess]
	push	eax
	call	[DuplicateHandle]
	ret
@@:
	mov	[eax], ecx
	ret

shmem_load:
	mov	edx, [eax+shmem_proc_descr.end]
	mov	ecx, [eax+shmem_proc_descr.ptr]
	sub	edx, ecx
	push	eax ecx edx
	push	eax
	push	esp
	push	2	; PAGE_READONLY
	push	edx
	push	ecx
	push	[esi+shmem_item.hOwner]
	call	[VirtualProtectEx]
	pop	eax
	pop	edx ecx eax
	push	ebx
	push	edx
	push	[esi+shmem_item.ptr]
	push	ecx
	push	[esi+shmem_item.hOwner]
	call	[ReadProcessMemory]
	mov	[esi+shmem_item.hOwner], ebx
	mov	[esi+shmem_item.pOwner], ebx
	ret

shmem_free_item:
	mov	eax, [esi+shmem_item.next]
	mov	ecx, [esi+shmem_item.prev]
	mov	[eax+shmem_item.prev], ecx
	mov	[ecx+shmem_item.next], eax
	push	[esi+shmem_item.ptr]
	call	free_big
	push	esi
	call	free
	ret

include 'i40emul.inc'

section '.rdata' data readable

data import
macro thunk a {
a#_thunk:dw 0
db `a,0}
	dd	0,0,0, rva kernel32_name, rva kernel32_thunks
;	dd	0,0,0, rva user32_name, rva user32_thunks
;	dd	0,0,0, rva gdi32_name, rva gdi32_thunks
;	dd	0,0,0, rva comdlg32_name, rva comdlg32_thunks
	dd	0,0,0, rva advapi32_name, rva advapi32_thunks
;	dd	0,0,0, rva winmm_name, rva winmm_thunks
	dd	0,0,0,0,0
kernel32_name db 'kernel32.dll',0
user32_name db 'user32.dll',0
gdi32_name db 'gdi32.dll',0
ntdll_name db 'ntdll.dll',0
comdlg32_name db 'comdlg32.dll',0
advapi32_name db 'advapi32.dll',0
winmm_name db 'winmm.dll',0
;winsock_name db 'wsock32.dll',0
kernel32_thunks:
CreateFileA	dd	rva CreateFileA_thunk
CloseHandle	dd	rva CloseHandle_thunk
CreateFileMappingA dd	rva CreateFileMappingA_thunk
OpenFileMappingA dd	rva OpenFileMappingA_thunk
MapViewOfFile	dd	rva MapViewOfFile_thunk
UnmapViewOfFile	dd	rva UnmapViewOfFile_thunk
ReadFile	dd	rva ReadFile_thunk
WriteFile	dd	rva WriteFile_thunk
GetFileSize	dd	rva GetFileSize_thunk
SetEndOfFile    dd      rva SetEndOfFile_thunk
VirtualAlloc	dd	rva VirtualAlloc_thunk
VirtualFree	dd	rva VirtualFree_thunk
VirtualProtect	dd	rva VirtualProtect_thunk
VirtualProtectEx dd	rva VirtualProtectEx_thunk
SetFilePointer	dd	rva SetFilePointer_thunk
ExitProcess	dd	rva ExitProcess_thunk
ExitThread	dd	rva ExitThread_thunk
CreateProcessA	dd	rva CreateProcessA_thunk
CreateThread	dd	rva CreateThread_thunk
TerminateThread	dd	rva TerminateThread_thunk
GetCommandLineA dd	rva GetCommandLineA_thunk
ReadProcessMemory dd	rva ReadProcessMemory_thunk
WriteProcessMemory dd	rva WriteProcessMemory_thunk
WaitForDebugEvent dd	rva WaitForDebugEvent_thunk
ContinueDebugEvent dd	rva ContinueDebugEvent_thunk
SuspendThread	dd	rva SuspendThread_thunk
ResumeThread	dd	rva ResumeThread_thunk
GetThreadContext dd	rva GetThreadContext_thunk
SetThreadContext dd	rva SetThreadContext_thunk
GetProcessHeap	dd	rva GetProcessHeap_thunk
HeapAlloc	dd	rva HeapAlloc_thunk
HeapReAlloc	dd	rva HeapReAlloc_thunk
HeapFree	dd	rva HeapFree_thunk
Sleep		dd	rva Sleep_thunk
GetLocalTime	dd	rva GetLocalTime_thunk
SetFileTime     dd      rva SetFileTime_thunk
GetCurrentDirectoryA dd	rva GetCurrentDirectoryA_thunk
SetCurrentDirectoryA dd	rva SetCurrentDirectoryA_thunk
GetTickCount	dd	rva GetTickCount_thunk
GetCurrentProcess dd	rva GetCurrentProcess_thunk
GetPrivateProfileStringA dd rva GetPrivateProfileStringA_thunk
GetPrivateProfileIntA dd rva GetPrivateProfileIntA_thunk
lstrcpyA	dd	rva lstrcpyA_thunk
lstrcpynA       dd      rva lstrcpynA_thunk
lstrcatA	dd	rva lstrcatA_thunk
lstrlenA	dd	rva lstrlenA_thunk
lstrcmpA        dd      rva lstrcmpA_thunk
GetFileAttributesA dd	rva GetFileAttributesA_thunk
SetFileAttributesA dd   rva SetFileAttributesA_thunk
GetModuleFileNameA dd	rva GetModuleFileNameA_thunk
GetLastError	dd	rva GetLastError_thunk
CreateMutexA	dd	rva CreateMutexA_thunk
CreateEventA	dd	rva CreateEventA_thunk
SetEvent	dd	rva SetEvent_thunk
DuplicateHandle	dd	rva DuplicateHandle_thunk
WaitForSingleObject dd	rva WaitForSingleObject_thunk
ReleaseMutex	dd	rva ReleaseMutex_thunk
GetVersion	dd	rva GetVersion_thunk
GetModuleHandleA dd	rva GetModuleHandleA_thunk
GetProcAddress	dd	rva GetProcAddress_thunk
GetCurrentThreadId dd	rva GetCurrentThreadId_thunk
GetStartupInfoA	dd	rva GetStartupInfoA_thunk
FindFirstFileA	dd	rva FindFirstFileA_thunk
FindNextFileA	dd	rva FindNextFileA_thunk
FindClose	dd	rva FindClose_thunk
FileTimeToDosDateTime dd rva FileTimeToDosDateTime_thunk
DeleteFileA	dd	rva DeleteFileA_thunk
DeviceIoControl dd	rva DeviceIoControl_thunk
MultiByteToWideChar dd  rva MultiByteToWideChar_thunk
FileTimeToSystemTime dd rva FileTimeToSystemTime_thunk
SystemTimeToFileTime dd rva SystemTimeToFileTime_thunk
GetFullPathNameA dd     rva GetFullPathNameA_thunk
CompareStringA  dd      rva CompareStringA_thunk
GlobalMemoryStatus dd   rva GlobalMemoryStatus_thunk
InitializeCriticalSection dd rva InitializeCriticalSection_thunk
EnterCriticalSection dd rva EnterCriticalSection_thunk
LeaveCriticalSection dd rva LeaveCriticalSection_thunk
CreateDirectoryA dd     rva CreateDirectoryA_thunk
RemoveDirectoryA dd     rva RemoveDirectoryA_thunk
LoadLibraryA    dd      rva LoadLibraryA_thunk
FreeLibrary     dd      rva FreeLibrary_thunk
	dw	0
thunk CreateFileA
thunk CloseHandle
thunk CreateFileMappingA
thunk OpenFileMappingA
thunk MapViewOfFile
thunk UnmapViewOfFile
thunk ReadFile
thunk WriteFile
thunk GetFileSize
thunk SetEndOfFile
thunk VirtualAlloc
thunk VirtualFree
thunk VirtualProtect
thunk VirtualProtectEx
thunk SetFilePointer
thunk ExitProcess
thunk ExitThread
thunk CreateProcessA
thunk CreateThread
thunk TerminateThread
thunk GetCurrentProcess
thunk GetCommandLineA
thunk ReadProcessMemory
thunk WriteProcessMemory
thunk WaitForDebugEvent
thunk ContinueDebugEvent
thunk SuspendThread
thunk ResumeThread
thunk GetThreadContext
thunk SetThreadContext
thunk GetProcessHeap
thunk HeapAlloc
thunk HeapReAlloc
thunk HeapFree
thunk Sleep
thunk GetLocalTime
thunk SetFileTime
thunk GetCurrentDirectoryA
thunk SetCurrentDirectoryA
thunk GetTickCount
thunk GetPrivateProfileStringA
thunk GetPrivateProfileIntA
thunk lstrcpyA
thunk lstrcpynA
thunk lstrcatA
thunk lstrlenA
thunk lstrcmpA
thunk GetFileAttributesA
thunk SetFileAttributesA
thunk GetModuleFileNameA
thunk GetLastError
thunk CreateMutexA
thunk CreateEventA
thunk SetEvent
thunk DuplicateHandle
thunk WaitForSingleObject
thunk ReleaseMutex
thunk GetVersion
thunk GetModuleHandleA
thunk GetProcAddress
thunk GetCurrentThreadId
thunk GetStartupInfoA
thunk FindFirstFileA
thunk FindNextFileA
thunk FindClose
thunk CharToOemA
thunk OemToCharA
thunk FileTimeToDosDateTime
thunk DeleteFileA
thunk DeviceIoControl
thunk MultiByteToWideChar
thunk FileTimeToSystemTime
thunk SystemTimeToFileTime
thunk GetFullPathNameA
thunk CompareStringA
thunk GlobalMemoryStatus
thunk InitializeCriticalSection
thunk EnterCriticalSection
thunk LeaveCriticalSection
thunk CreateDirectoryA
thunk RemoveDirectoryA
thunk LoadLibraryA
thunk FreeLibrary
aNtSetLdtEntries db 'NtSetLdtEntries',0
if ~driver_via_scm
aNtLoadDriver   db 'NtLoadDriver',0
aNtUnloadDriver db 'NtUnloadDriver',0
end if
align 4
advapi32_thunks:
if ~driver_via_scm
RegCreateKeyExA dd      rva RegCreateKeyExA_thunk
RegDeleteKeyA   dd      rva RegDeleteKeyA_thunk
end if
RegOpenKeyExA	dd	rva RegOpenKeyExA_thunk
RegCloseKey	dd	rva RegCloseKey_thunk
RegQueryValueExA dd	rva RegQueryValueExA_thunk
RegSetValueExA	dd	rva RegSetValueExA_thunk
OpenSCManagerA	dd	rva OpenSCManagerA_thunk
CreateServiceA	dd	rva CreateServiceA_thunk
OpenServiceA	dd	rva OpenServiceA_thunk
StartServiceA	dd	rva StartServiceA_thunk
ControlService	dd	rva ControlService_thunk
DeleteService	dd	rva DeleteService_thunk
CloseServiceHandle dd	rva CloseServiceHandle_thunk
	dw	0
if ~driver_via_scm
thunk RegCreateKeyExA
thunk RegDeleteKeyA
end if
thunk RegOpenKeyExA
thunk RegCloseKey
thunk RegQueryValueExA
thunk RegSetValueExA
thunk OpenSCManagerA
thunk CreateServiceA
thunk OpenServiceA
thunk StartServiceA
thunk ControlService
thunk DeleteService
thunk CloseServiceHandle
;winmm_thunks:
;PlaySoundA	dd	rva PlaySoundA_thunk
;	dw	0
;thunk PlaySoundA
end data

aGetOpenFileNameA db	'GetOpenFileNameA',0
aPlaySoundA	db	'PlaySoundA',0

align 4
;data resource from 'klbrico.res'
;end data
data resource
rsrcdata:
; only icon resource from file 'KlbrInWin.ico'
; for graphics thanks to goglus, Leency, Heavyiron
iconfile equ 'KlbrInWin.ico'

virtual at 0
; load .ico header
file iconfile:0,6
load .idReserved word from 0
load .idType word from 2
load .idCount word from 4
if (.idReserved <> 0) | (.idType <> 1)
error invalid icon file
end if
end virtual

; root resource directory
	dd	0, 0, 0
	dw	0, 2	; 2 entries by id
	dd	3, (.icon1 - rsrcdata) or 80000000h	; entry 1: RT_ICON
	dd	14, (.gicon1 - rsrcdata) or 80000000h	; entry 2: RT_GROUP_ICON
; level-1 resource directory for RT_ICON
.icon1:
	dd	0, 0, 0
	dw	0, .idCount	; .idCount entries by id
repeat .idCount
	dd	%, ((.icon2 - rsrcdata) + 18h*(%-1)) or 80000000h
end repeat
; level-1 resource directory for RT_GROUP_ICON
.gicon1:
	dd	0, 0, 0
	dw	0, 1	; 1 entry by id
	dd	1, (.gicon2 - rsrcdata) or 80000000h
; level-2 resource directories for RT_ICON
.icon2:
repeat .idCount
	dd	0, 0, 0
	dw	0, 1	 ; 1 entry by id
	dd	0, (.icon3 - rsrcdata) + 10h*(%-1)
end repeat
; level-2 resource directory for RT_GROUP_ICON
.gicon2:
	dd	0, 0, 0
	dw	0, 1	; 1 entry by id
	dd	0, (.gicon3 - rsrcdata)
; leaf entries for RT_ICON
.icon3:
.a = rva .icons
repeat .idCount
virtual at 0
file iconfile:6+16*(%-1)+8,4
load .dwBytesInRes dword from 0
end virtual
	dd	.a, .dwBytesInRes, 0, 0
.a = (.a + .dwBytesInRes + 3) and not 3
end repeat
; leaf entry for RT_GROUP_ICON
.gicon3:
	dd	rva .gicon, .gicon_end - .gicon, 0, 0
; icon data
.icons:
repeat .idCount
virtual at 0
file iconfile:6+16*(%-1)+8,8
load .dwBytesInRes dword from 0
load .dwImageOffset dword from 4
end virtual
	file	iconfile:.dwImageOffset,.dwBytesInRes
while .dwBytesInRes and 3
.dwBytesInRes = .dwBytesInRes + 1
	db	0
end while
end repeat
.gicon:
	dw	0, 1, .idCount	; .idCount images
repeat .idCount
	file	iconfile:6+16*(%-1),12
	dw	%
end repeat
.gicon_end:
end data

data 9
	dd	tls_init_start
	dd	tls_init_end
	dd	tls_index
	dd	0
	dd	0
	dd	0
end data

virtual at 0
tls:
	._cs	dw	?
	._ds	dw	?
	._esp	dd	?
	._eip	dd	?
	._fs	dw	?
		dw	?	; align
	.exc_code dd	?
	.exc_data dd	?
	.message_mask	dd	?
	.lpShapeData	dd	?
	.scale		dd	?
	.curdraw	db	?

	.uninit_size = .size - $

	.showwnd	db	?
	.bFirstMouseMove db	?
	.bActive	db	?
	.hWnd		dd	?
	.hCursor        dd      ?
	.buttons	dd	?
	.x_size		dw	?
	.x_start	dw	?
	.y_size		dw	?
	.y_start	dw	?
	.client_left    dd      ?
	.client_top     dd      ?
	.client_width   dd      ?
	.client_height  dd      ?
	.color_main	dd	?
	.color_capt	dd	?
	.color_border	dd	?
	.caption        dd      ?
	.debuggees	dd	?
	.translated_msg_code db ?
	.usescancode	db	?
	.keybuflen	db	?
	.butbuflen	db	?
	.keybuffer	rb	0x100
	.butbuffer	rd	0x100
	.active_button	dd	?
	.cur_slot	dd	?
	.saved_fs0	dd	?
	.saved_fs4	dd	?
	.prev_snd_block	dd	?
	.cur_dir	dd	?
	.scroll		dd	?
	.original_buttons db	?
	.current_buttons db	?
			dw	?
	.size = $
end virtual

	align	4
ofn_arg_template:
	dw	1,-1	; dlgVer,signature
	dd	0	; helpId
	dd	0	; exStyle
	dd	56000444h	; style
	dw	2	; cDlgItems
	dw	0,0,275,28	; x,y,cx,cy
	dw	0,0,0	; menu,windowClass,title
	dw	8	; pointsize
	dd	0	; weight,italic,charset
	du	'MS Sans Serif',0
	align	4
	dd	0	; helpId
	dd	0	; exStyle
	dd	50010000h	; style
	dw	5,12,45,9	; x,y,cx,cy
	dw	-1	; id
	dw	0
	dw	-1,82h	; windowClass
	du	"Parameters:",0
	dw	0
	align	4
	dd	0
	dd	204h
	dd	50010080h
	dw	54,10,218,12
	dw	23
	dw	0
	dw	-1,81h
	dw	0
	dw	0

align 4
_1193180 dd	1193180
_100	dd	100

kontrOctave:
; note that values 0, D,E,F must not be used, but 0 is used (e.g. by icon2)
	dw	0xC3FB, 0x4742, 0x4342, 0x3F7C, 0x3BEC, 0x388F, 0x3562, 0x3264
	dw	0x2F8F, 0x2CE4, 0x2A5F, 0x2802, 0x25BF, 0xFDA, 0, 0x19

dir0:
	db	'HARDDISK   ',10h
	db	'RAMDISK    ',10h
dir1	db	'FIRST      ',10h

path_begin:
	db	1,2,'RD'
	db	1,7,'RAMDISK'
	db	2,2,'FD'
	db	2,11,'FLOPPYDI.SK'
	db	4,3,'HD0'
	db	5,3,'HD1'
	db	6,3,'HD2'
	db	7,3,'HD3'
	db	3,2,'HD'
	db	3,8,'HARDDISK'
	db	0


; align 4
; winsock_imports:
; WSAStartup	dd	WSAStartup_name
; WSACleanup	dd	WSACleanup_name
; socket		dd	socket_name
; closesocket	dd	closesocket_name
	; dd	0

; WSAStartup_name	db	'WSAStartup',0
; WSACleanup_name	db	'WSACleanup',0
; socket_name	db	'socket',0
; closesocket_name db	'closesocket',0

ofn_title db 'Select KolibriOS executable',0
	dd	-10
fileopenerr db 'Cannot open input file',0
	dd	-31
filereaderr db 'Input file read error',0
	dd	-31
notexe	db 'Not KolibriOS executable!',0
	dd	-7
params_err db 'Parameters pointer is outside used memory!',0
	dd	-30
memerr	db 'Not enough memory',0
	dd	-30
ldterr	db 'Cannot allocate LDT selectors',0
idt_err	db 'IDT limit too small',0
exceptionstr db 'Exception',0
excstr	db 'Emulated process has caused an exception and will be terminated.',13,10
	db 'Registers:',13,10
	db 'EAX=%08X EBX=%08X ECX=%08X EDX=%08X',13,10
	db 'ESI=%08X EDI=%08X ESP=%08X EBP=%08X',13,10
	db 'EIP=%08X EFLAGS=%08X',0
nsm	db 'Unsupported system function',0
notsupportedmsg db 'Emulated process has called unknown system function and will be terminated.',13,10
	db 'Registers:',13,10
	db 'EAX=%08X EBX=%08X ECX=%08X EDX=%08X',13,10
	db 'ESI=%08X EDI=%08X ESP=%08X EBP=%08X',13,10
	db 'EIP=%08X EFLAGS=%08X',0
cpe db 'Cannot create process',0
aConfirm	db	'Подтверждение',0
BgrQuestionText	db	'Программа хочет установить фон рабочего стола.',13,10
		db	'Установить как постоянный?',0
BgrFileErrorMsg	db	'Cannot create background image file',0
		dd	-1
skinfileerr	db	'Invalid skin file',0
vkerr		db	'A running instance of KlbrInWin already exists, cannot continue',0
		dd	-1
no_partition	db	'Partition is not defined',0
EnablePortsSyntaxErr db	'EnablePorts parameter: syntax error',0
DrvLoadErr	db	'Cannot load driver',0
DrvOpenErr	db	'Cannot send command to driver',0
PortsRangeErr	db	'Sysfunction 46: invalid ports range',0
PortsNotEnabledErr db	'Sysfunction 46: attempt to allocate not enabled ports',0
PortsUsedErr	db	'Sysfunction 46: attempt to allocate already used ports',0
PortsNotUsedErr	db	'Sysfunction 46: attempt to free ports which were not allocated',0

;aPathInvalid    db      'Path pointer is outside used memory and will be ignored',0
		dd	-2
aPathUnknown    db      'Win32 path to program cannot be written as Kolibri path!',0

aReadMSRDisabled db     'Emulated process tries to read MSR, and this is disabled in ini-file.',0
aNoMsr          db      'Emulated process has tried to read invalid MSR and will be terminated',0
aInvFn64Call    db      'Function 64 has been called after heap initialization, will fail.',0
aHeapNotInited  db      'Attempt to work with uninitialized heap!',0
aInternalError  db      'Internal error',0
aMallocFailed   db      'Memory request failed!',0
aFreeInvalid    db      'Attempt to free/realloc not allocated block!',0
aCannotLoadDll  db      'Cannot load DLL',0
aUnknownReloc   db      'Unknown relocation type',0
aExportsNotFound db     'DLL export table was not found!',0
aInvCursorData  db      'Invalid cursor data',0
aOnlyOneCursor  db      'Cursor data must contain only one cursor',0
aInvCursorDim   db      'Cursor must be of size 32*32 pixels',0
aCursorFailed   db      'Cursor creation failed',0
aCursorLimitExceeded db 'Cursors limit exceeded',0
aInvalidCursor  db      'Invalid handle for delete_cursor!',0
aSound          db      'SOUND',0
aInfinity       db      'INFINITY',0
aUnknownDriver  db      'Attempt to load unknown driver will fail',0
aCannotGetPci	db	'Cannot get PCI BIOS parameters',0
;aPciDisabled	db	'Emulated process tries to enable PCI access, and this is disabled in ini-file.',0
		dd	-1
aInvalidColorDepth db	'Invalid ColorDepth parameter in ini-file',0
DSAErr		db	'Access to DirectScreenArea outside real screen data causes an exception...',0
DSADisabled	db	'The program has called sysfunction 61 (Direct Screen Access parameters),',10
		db	'but Direct Screen Access is disabled in ini-file. The program will be terminated :(',0
aFailedToDeliverDebugMessage db	'Failed to deliver debug message',0
aInvalidDataForDR db	'Invalid data for 69.9, returning an error',0
aCannotDestroyShMem db	'Attempt to close not opened shared memory area',0
;aWinsockInitErr	db	'Cannot initialize Winsock DLL!',0
;aSocketErr	db	'Cannot allocate socket!',0

inifilename db 'KlbrInWin.ini'
null_string db 0
inifilenamesize = $ - inifilename
kiw0filename    db 'kiw0.sys',0
kiw0filenamesize = $ - kiw0filename
kiw0_username	db	'KlbrInWin ring-0 component',0
kiw0_drivername	db	'kiw0',0
kiw0		db	'\\.\kiw0',0
if ~driver_via_scm
DrvKey          db      'SYSTEM\CurrentControlSet\Services\kiw0',0
DrvKeyEnum      db      'SYSTEM\CurrentControlSet\Services\kiw0\Enum',0
align 4
DrvKeySys:
        dw      DrvKeySysLen-2, DrvKeySysLen
        dd      @f
@@      du      '\REGISTRY\MACHINE\SYSTEM\CurrentControlSet\Services\kiw0',0
DrvKeySysLen = $ - @b
aDisplayName    db      'DisplayName',0
aType           db      'Type',0
aStart          db      'Start',0
addr3           dd      3       ; SERVICE_DEMAND_START
aErrorControl   db      'ErrorControl',0
aImagePath      db      'ImagePath',0

umpnpmgr_name   db      'umpnpmgr.dll',0
umpnpmgr_uninst db      'DeleteServicePlugPlayRegKeys',0
kiw0_unicode    du      'kiw0',0
end if

default_ramdisk db 'A:\',0
ramdisk_keyname db 'RamDisk',0
aDisk db 'Disk',0
aMain db 'Main',0
aFont1 db 'Font1',0
aFont2 db 'Font2',0
aSkin db 'Skin',0
aQuestions db 'Questions',0
aSetBgr db 'SetBgr',0
aSetup db 'sys_setup',0
aSoundFlag db 'sound_flag',0
aSoundVol db 'sound_vol',0
aSysLang db 'syslang',0
aKeyboard db 'keyboard',0
aEnablePorts db 'EnablePorts',0
aAllowReadMSR db 'AllowReadMSR',0
aAllowReadPCI db 'AllowReadPCI',0
aKeepLoadedDriver db 'KeepLoadedDriver',0
aDirectScreenAccess db 'DirectScreenAccess',0
aColorDepth db 'ColorDepth',0
aInvalidateTime db 'DSAInvalidateTime',0

classname db 'KolibriInWin_WndClass',0
	dd	-30
createwnderr db 'Cannot create window!',0

	dd	-30
shared_section_size = 8000h
shared_section_create_err db 'Cannot create section for shared data!',0
shared_mutex_create_err db 'Cannot create mutex for shared data!',0
virtual at 0
shared_data_struc:
	.alloc_threads	dd	?
	.num_threads	dd	?
	.vk		db	?
	.bAllowReadMSR  db      ?
	.b9xPerfInited  db	?
if idletime_via_ring0
	.idlecount	dd	?
end if
; \begin{sys_setup}
	.sound_flag	db	?
	.syslang	dd	?
	.midi_base	dw	?
	.cd_base	db	?
	.hd_base	db	?
	.sb16		dd	?
	.wss		dd	?
	.fat32part	dd	?
	.sound_dma	dd	?
	.lba_read_enabled dd	?
	.pci_access_enabled dd	?
	.keyboard	dw	?
	.mouse_speed_factor dw  ?
	.mouse_delay    dd      ?
; \end{sys_setup}
	.pci_data_init	db	?	; initialized?
	.bAllowReadPCI	db	?
	.curport	dw	?
	.cursocket	dd	?
	.pci_bios_mj	db	?	; major PCI BIOS version
	.pci_bios_mn	db	?	; minor PCI BIOS version
	.pci_bios_lb	db	?	; last PCI bus
	.pci_bios_pc	db	?	; PCI characteristics
	.workarea_left	dd	?
	.workarea_top	dd	?
	.workarea_right	dd	?
	.workarea_bottom dd	?
	.dwNewBgrTime	dd	?
	.msg_board_count	dd	?
	.msg_board_data		rb	512
	.active_process	dd	?
	.cpuspeed	dd	?
	.DisabledPorts	rb	2000h
	.UsedIoMap	rb	2000h
num_cursors = 63        ; exclude standard arrow cursor, it is handled separately
        .cursors        rd      num_cursors*2
.threads:
; rept .alloc_threads
	.thread_id	dd	?		; 0 for free slot
	.thread_ipc_mem dd	?
	.thread_ipc_size dd	?
	.win32_hBaseProcess dd	?	; this is handle for debugger!
	.win32_dwThreadId dd	?
	.hWnd		dd	?
	.limit		dd	?
	.name		rb	12
	.win32_hThread	dd	?	; this is handle for debugger!
	.debugger_mem	dd	?
	.win32_stack	dd	?
	.shmem_list	dd	?	; head of L1-list of shmem_proc_descr
	rd 2
end virtual

bgr_section_name	db	'KolibriInWin_background',0
bgr_section_size	=	0x160000+0x10
bgr_mutex_name		db	'KolibriInWin_bgrmtx',0
bgrkeyname		db	'Control Panel\Desktop',0
bgrstylevalue		db	'WallpaperStyle',0
bgrtilevalue		db	'TileWallpaper',0
bgrtempfilename		db	'klbrbgr.bmp',0
bgrfilename		db	'klbr_bgr.bmp',0

newprg_section_name	db	'KolibriInWin_newprg',0

keycpu		db	'HARDWARE\DESCRIPTION\System\CentralProcessor\0',0
keymhz		db	'~MHz',0

aIdentifier	db	'Identifier',0
aConfigurationData db	'Configuration Data',0

perfstart	db	'PerfStats\StartStat',0
perfget		db	'PerfStats\StatData',0
perfend		db	'PerfStats\StopStat',0
perfval		db	'KERNEL\CPUUsage',0
aPerfInitFailed	db	'Failed to init performance counter',0

exccode2number:
	dd	0xC0000094	; EXCEPTION_INT_DIVIDE_BY_ZERO
	db	0		; #DE
;	dd	0x80000004	; EXCEPTION_SINGLE_STEP (handled separately)
;	db	1		; #DB
	dd	0x80000003	; EXCEPTION_BREAKPOINT
	db	0xD		; #GP (yes, in Kolibri it's #GP, not #BP)
	dd	0xC0000095	; EXCEPTION_INT_OVERFLOW
	db	4		; #OF
	dd	0xC000008C	; EXCEPTION_ARRAY_BOUNDS_EXCEEDED
	db	5		; #BR
	dd	0xC000001D	; EXCEPTION_ILLEGAL_INSTRUCTION
	db	6		; #UD
	dd	0xC0000096	; EXCEPTION_PRIV_INSTRUCTION
	db	0xD		; #GP
	dd	0xC0000005	; EXCEPTION_ACCESS_VIOLATION
	db	0xE		; #PF
	dd	0x80000002	; EXCEPTION_DATATYPE_MISALIGNMENT
	db	0x11		; #AC
	dd	0xC000008D	; EXCEPTION_FLT_DENORMAL_OPERAND
	db	0x10		; #MF
	dd	0xC000008E	; EXCEPTION_FLT_DIVIDE_BY_ZERO
	db	0x10		; #MF
	dd	0xC000008F	; EXCEPTION_FLT_INEXACT_RESULT
	db	0x10		; #MF
	dd	0xC0000090	; EXCEPTION_FLT_INVALID_OPERATION
	db	0x10		; #MF
	dd	0xC0000091	; EXCEPTION_FLT_OVERFLOW
	db	0x10		; #MF
	dd	0xC0000092	; EXCEPTION_FLT_STACK_CHECK
	db	0x10		; #MF
	dd	0xC0000093	; EXCEPTION_FLT_UNDERFLOW
	db	0x10		; #MF
	dd	0

section '.data' data readable writable

user32_thunks:
MessageBoxA	dd	rva MessageBoxA_thunk
wsprintfA	dd	rva wsprintfA_thunk
GetDC		dd	rva GetDC_thunk
ReleaseDC	dd	rva ReleaseDC_thunk
LoadIconA       dd      rva LoadIconA_thunk
LoadCursorA	dd	rva LoadCursorA_thunk
LoadImageA      dd      rva LoadImageA_thunk
RegisterClassExA dd	rva RegisterClassExA_thunk
CreateWindowExA	dd	rva CreateWindowExA_thunk
MoveWindow	dd	rva MoveWindow_thunk
ShowWindow	dd	rva ShowWindow_thunk
DefWindowProcA	dd	rva DefWindowProcA_thunk
BeginPaint	dd	rva BeginPaint_thunk
EndPaint	dd	rva EndPaint_thunk
GetMessageA	dd	rva GetMessageA_thunk
PeekMessageA	dd	rva PeekMessageA_thunk
TranslateMessage dd	rva TranslateMessage_thunk
DispatchMessageA dd	rva DispatchMessageA_thunk
FillRect	dd	rva FillRect_thunk
PostQuitMessage	dd	rva PostQuitMessage_thunk
GetDesktopWindow dd	rva GetDesktopWindow_thunk
GetAsyncKeyState dd	rva GetAsyncKeyState_thunk
GetKeyboardState dd	rva GetKeyboardState_thunk
SetCapture	dd	rva SetCapture_thunk
ReleaseCapture	dd	rva ReleaseCapture_thunk
GetCursorPos	dd	rva GetCursorPos_thunk
SetCursorPos	dd	rva SetCursorPos_thunk
InvalidateRect	dd	rva InvalidateRect_thunk
ValidateRect	dd	rva ValidateRect_thunk
SetWindowRgn	dd	rva SetWindowRgn_thunk
EnumThreadWindows dd	rva EnumThreadWindows_thunk
PostMessageA	dd	rva PostMessageA_thunk
SendMessageTimeoutA dd	rva SendMessageTimeoutA_thunk
GetDlgItemTextA	dd	rva GetDlgItemTextA_thunk
PaintDesktop	dd	rva PaintDesktop_thunk
SystemParametersInfoA dd rva SystemParametersInfoA_thunk
GetWindowRect	dd	rva GetWindowRect_thunk
GetWindowPlacement dd	rva GetWindowPlacement_thunk
;BringWindowToTop dd	rva BringWindowToTop_thunk
PostThreadMessageA dd	rva PostThreadMessageA_thunk
CharToOemA	dd	rva CharToOemA_thunk
OemToCharA	dd	rva OemToCharA_thunk
IsWindowVisible dd      rva IsWindowVisible_thunk
CreateIconFromResourceEx dd rva CreateIconFromResourceEx_thunk
CreateIconIndirect dd   rva CreateIconIndirect_thunk
SetCursor       dd      rva SetCursor_thunk
DestroyCursor   dd      rva DestroyCursor_thunk
SetForegroundWindow dd  rva SetForegroundWindow_thunk
	dw	0
thunk MessageBoxA
thunk wsprintfA
thunk GetDC
thunk ReleaseDC
thunk CreateCompatibleDC
thunk LoadIconA
thunk LoadCursorA
thunk LoadImageA
thunk RegisterClassExA
thunk CreateWindowExA
thunk MoveWindow
thunk ShowWindow
thunk DefWindowProcA
thunk BeginPaint
thunk EndPaint
thunk GetMessageA
thunk PeekMessageA
thunk TranslateMessage
thunk DispatchMessageA
thunk PostQuitMessage
thunk GetDesktopWindow
thunk GetPixel
thunk SetPixel
thunk GetAsyncKeyState
thunk GetKeyboardState
thunk SetCapture
thunk ReleaseCapture
thunk GetCursorPos
thunk SetCursorPos
thunk InvalidateRect
thunk ValidateRect
thunk SetWindowRgn
thunk PostMessageA
thunk SendMessageTimeoutA
thunk EnumThreadWindows
thunk GetDlgItemTextA
thunk PaintDesktop
thunk SystemParametersInfoA
thunk GetWindowRect
thunk GetWindowPlacement
;thunk BringWindowToTop
thunk PostThreadMessageA
thunk IsWindowVisible
thunk CreateIconFromResourceEx
thunk CreateIconIndirect
thunk SetCursor
thunk DestroyCursor
thunk SetForegroundWindow
gdi32_thunks:
SetDIBitsToDevice dd	rva SetDIBitsToDevice_thunk
GetDIBits	dd	rva GetDIBits_thunk
CreatePen	dd	rva CreatePen_thunk
SelectObject	dd	rva SelectObject_thunk
DeleteObject	dd	rva DeleteObject_thunk
CreateSolidBrush dd	rva CreateSolidBrush_thunk
CreateBitmap	dd	rva CreateBitmap_thunk
CreateCompatibleDC dd	rva CreateCompatibleDC_thunk
CreateCompatibleBitmap dd rva CreateCompatibleBitmap_thunk
BitBlt		dd	rva BitBlt_thunk
MoveToEx	dd	rva MoveToEx_thunk
LineTo		dd	rva LineTo_thunk
GetDeviceCaps	dd	rva GetDeviceCaps_thunk
GetPixel	dd	rva GetPixel_thunk
SetPixel	dd	rva SetPixel_thunk
SetROP2		dd	rva SetROP2_thunk
Polyline	dd	rva Polyline_thunk
ExtCreateRegion	dd	rva ExtCreateRegion_thunk
DeleteDC	dd	rva DeleteDC_thunk
	dw	0
thunk SetDIBitsToDevice
thunk GetDIBits
thunk CreatePen
thunk SelectObject
thunk DeleteObject
thunk CreateSolidBrush
thunk FillRect
thunk BitBlt
thunk CreateBitmap
thunk CreateCompatibleBitmap
thunk MoveToEx
thunk LineTo
thunk GetDeviceCaps
thunk SetROP2
thunk Polyline
thunk ExtCreateRegion
thunk DeleteDC
;comdlg32_thunks:
;GetOpenFileNameA dd	rva GetOpenFileNameA_thunk
;	dw	0
;thunk GetOpenFileNameA

	align 4
ofn:
	dd	76	; lStructSize
	dd	0	; hWndOwner
	dd	ofn_arg_template	; hInstance
	dd	0	; lpstrFilter
	dd	0	; lpstrCustomFilter
	dd	0	; nMaxCustFilter
	dd	0	; nFilterIndex
	dd	inname	; lpstrFile
	dd	100h	; nMaxFile
	dd	0	; lpstrFileTitle
	dd	0	; nMaxFileTitle
	dd	0	; lpstrInitialDir
	dd	ofn_title	; lpstrTitle
	dd	818A4h	; flags
	dw	0	; nFileOffset
	dw	0	; nFileExtension
	dd	0	; lpstrDefExt
	dd	0	; lCustData
	dd	ofn_hook	; lpfnHook
	dd	0	; lpTemplateName

align 4
PlaySoundA	dd	PlaySoundA_delayed_imp

NumThreads	dd	1

virtual at 0
shmem_item:
.name		rb	32
.next		dd	?
.prev		dd	?
.refs		dd	?
.ptr		dd	?
.size		dd	?
.access		dd	?
.hOwner		dd	?
.pOwner		dd	?
.sizeof = $
end virtual

virtual at 0
shmem_proc_descr:
.next		dd	?
.item		dd	?
.ptr		dd	?
.end		dd	?
.access		dd	?
.sizeof = $
end virtual

shmem_list	dd	shmem_list - shmem_item.next
		dd	shmem_list - shmem_item.next

DrvKeyValues:
        dd      aDisplayName, 1, kiw0_username, kiw0_drivername-kiw0_username-1
        dd      aType, 4, DrvKeyValues+4, 4
        dd      aStart, 4, addr3, 4
        dd      aErrorControl, 4, DrvKeyValues+4, 4
        dd      aImagePath, 1, win32_path, ?
drvpathlen = $-4
        dd      0

keymfa		db	'HARDWARE\DESCRIPTION\System\MultifunctionAdapter\'
idxmfa		db	'0'
		db	0

hdxn	db	'hd0n',0
hdpart	db	'hd0_%d',0
hdxy_str db     '/hd%d/%d/',0

bInitialized	db	0
bCaptured	db	0

label jmp_klbr fword
jmp_klbr_eip dd 0
klbr_cs	dw	0Fh
klbr_ds	dw	17h
klbr_null dw	0
label jmp_temp_int33 fword
	dd	0
temp_cs	dw	0
label jmp_temp_int1A fword
	dd	temp_code_int1A - temp_code
temp_cs2 dw	0

eee db 'exception in debuggee at '
eeeeip db '00000000'
db 0

; data for int40 emulating code - initialized
; from kernel.asm
keymap:
	db	'6',27,'1234567890-=',8,9	; 0x00
	db	'qwertyuiop[]',13,'~as'		; 0x10
	db	'dfghjkl;',39,96,0,'\zxcv'	; 0x20
	db	'bnm,./',0,'45 @23456'		; 0x30
	db	'7890123',180,178,184,'6',176,'7',179,'8',181	; 0x40
	db	177,183,185,182,'AB<D',255,'FGHIJKL'	; 0x50
	db	'MNOPQRSTUVWXYZAB'		; 0x60
	db	'CDEFGHIJKLMNOPQR'		; 0x70
keymap_shift:
	db	'6',27,'!@#$%^&*()_+',8,9	; 0x00
	db	'QWERTYUIOP{}',13,'~AS'		; 0x10
	db	'DFGHJKL:"~',0,'|ZXCV'		; 0x20
	db	'BNM<>?',0,'45 @23456'		; 0x30
	db	'7890123',180,178,184,'6',176,'7',179,'8',181	; 0x40
	db	177,183,185,182,'AB>D',255,'FGHIJKL'	; 0x50
	db	'MNOPQRSTUVWXYZAB'		; 0x60
	db	'CDEFGHIJKLMNOPQR'		; 0x70
keymap_alt:
	db	' ',27,' @ $  {[]}\ ',8,9	; 0x00
	db	'            ',13,'   '		; 0x10
	db	'          ',0,'     '		; 0x20
	db	'      ',0,'4',0,'       '	; 0x30
	db	'       ',180,178,184,'6',176,'7',179,'8',181	; 0x40
	db	177,183,185,182,'ABCD',255,'FGHIJKL'	; 0x50
	db	'MNOPQRSTUVWXYZAB'		; 0x60
	db	'CDEFGHIJKLMNOPQR'		; 0x70

numlock_map	db	'789-456+1230.'

version_inf:
	db	0,7,1,0		; emulate Kolibri 0.7.1.0
	db	3		; UID_KlbrInWin
	dd	945		; emulate revision 945
				; (last change: functions 68.22 and 68.23)
version_end:

bCommonColorsSet db	0

bHaveDSA	db	0

vk	db	0

tls_index	dd	-1

max_pid dd 1
num_kolibri_proc dd 0

window_topleft:
        dd      1, 21   ; type 1
        dd      0, 0    ; no drawn window
        dd      5, 20   ; type 2
        dd      5, ?    ; skinned
	dd	5, ?	; skinned fixed-size

buttontype db 1

bgr_bmp_header:
	db	'B','M'
	dd	?	; size
	dd	0
	dd	36h
	dd	28h
	dd	?	; width
	dd	?	; height
	dw	1
	dw	24
	dd	0
	dd	?	; size
	dd	0,0
	dd	0,0

wave_block_begin:
	db	'RIFF'
	dd	?
	db	'WAVEfmt '
	dd	10h
	dw	1,1
wave_r	dd	22050
	dd	22050
	dw	1,8
	db	'data'
;	dd	?
wbb_size = $ - wave_block_begin

; note that all uninitialized variables are set to 0 by Windows
sinfo	dd	44h
	rb	28h
	dd	80h
	rb	14h

tls_init_start:
	times 24 db 0
	dd	7	; message_mask
	dd	0	; lpShapeData
	dd	1	; scale
	db	1	; curdraw
	times tls.uninit_size db ?
tls_init_end:

bDontDebug db	?
keep_loaded_driver	db	?

align 4
bgr_section	dd	?
hBgrMutex	dd	?
;dwNewBgrTime	dd	?

SetBgrQuestion	dd	?

newprg_section	dd	?

hArrow          dd      ?

bIs9x	db	?
bDriverLoaded	db	?
heap_status     db      ?

align 4
inname	rb	256
header	rd	9
base            dd      ?
limit           dd      ?
fn9limit        dd      ?
heap_start      dd      ?
heap_control_block dd   ?
heap_region_size dd     ?
heap_critical_sec rb    0x18
DSACritSect	rb	0x18
selector_data   rb      8
selector_code   rb      8
NtSetLdtEntries dd      ?
idtr            dp      ?
pinfo:
hProcess	dd	?
hThread		dd	?
dwProcessId	dd	?
dwThreadId	dd	?
cur_slot	dd	?
cur_tid_ptr	dd	?
parent_tid_ptr	dd	?

debugevent rd	18h
tids dd ?
pids dd ?

_cs	dw	?
_ds	dw	?
_esp	dd	?
_eip	dd	?
_fs	dw	?
_gs	dw	?
exc_code dd	?
exc_data dd	?
klbr_esp dd	?

temp_ss		dw	?
temp_stack_size = 0x1000
temp_stack	rb	temp_stack_size

parameters dd ?

startcurdir rb 261

sound_vol	db	?

align 4
context	rd	0xB3

; data for int40 emulating code - uninitialized
hHeap	dd	?

hSharedData dd	?
hSharedMutex dd	?
shared_data dd ?

_skinh		dd	?
margins		rw	4	; right:left:bottom:top
skin_active_inner dd	?
skin_active_outer dd	?
skin_active_frame dd	?
skin_passive_inner dd	?
skin_passive_outer dd	?
skin_passive_frame dd	?

common_colors	rb	128

left_bmp dd	?
oper_bmp dd	?
base_bmp dd	?
left1_bmp dd	?
oper1_bmp dd	?
base1_bmp dd	?

skin_btn_close:
.left		dd	?
.top		dd	?
.width		dd	?
.height		dd	?
skin_btn_minimize:
.left		dd	?
.top		dd	?
.width		dd	?
.height		dd	?

char_mt		dd	?
char2_mt	dd	?

process_name	dd	?

ramdisk_path	rb	512
converted_path	rb	512
win32_path	rb	512

hd_partitions_num	rd	4
hd_partitions_array	rd	4

cmdline		rb	2000
process_curdir	rb	4096	; protected by the same mutex as for shared data

if driver_via_scm
hSCManager	dd	?
hService	dd	?
end if

ColorDepth	dd	?
InvalidateTime	dd	?
DSA		dd	?

;WinSockDLL	dd	?

align 4
unpack.p	rd	unpack.LZMA_BASE_SIZE + (unpack.LZMA_LIT_SIZE shl (unpack.lc+unpack.lp))
unpack.code_	dd	?
unpack.range	dd	?
unpack.rep0	dd	?
unpack.rep1	dd	?
unpack.rep2	dd	?
unpack.rep3	dd	?
unpack.previousByte db	?
