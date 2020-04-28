
GetStrLength:
; string:                   [in]  edi
; string length:            [out] ecx
;                           [unchanged] ebx, edx, esi, edi, ebp, esp
	or	ecx, -1
	xor	eax, eax
	cld
	repne scasb
	not	ecx
	sub	edi,ecx
	ret

GetProcAddress:
; hInstance (export table): [in]  esi
; proc name:                [in]  edi
; proc address:             [out] eax
;                           [unchanged] ebp, esp
	call	GetStrLength
	mov	edx, ecx
	mov	ebx, edi
	mov	eax, esi
.import_find:
	mov	esi, ebx
	mov	edi, [eax]
	test	edi, edi
	jz	.import_not_found
	repe cmpsb
	je	.import_found
	mov	ecx, edx
	add	eax, 8
	jmp	.import_find
.import_not_found:
	sub	eax, 4
.import_found:
	mov	eax, [eax+4]
	ret

ResolveImports:
; hInstance (export table): [in]  esi
; lib.lookup table:         [in]  ebp
; ?isResolved:              [out] eax
;                           [unchanged] esi, esp
.retFrom = 4
.import_loop:
	mov	edi, [ebp]
	test	edi, edi
	jz	.import_done
	push	esi
	call	GetProcAddress
	pop	esi
	test	eax, eax
	jz	.import_not_found
	mov	[ebp],eax
	add	ebp,4
	jmp	.import_loop
.import_not_found:
	mov	eax, sz_EPnotFound
	mov	ebx, sizeof.sz_EPnotFound
	mov	ecx, 3
	mov	edx, [ebp]
	call	ShowErrorWindow
	or	[LoadLibraries.errStatus+ResolveImports.retFrom],1
	mcall	SF_TERMINATE_PROCESS ;uncomment to stop after 1st error
	jmp	.import_loop
.import_done:
	ret

@copy_path_wo_pdname:
; library name:             [in]  edx
; library path:             [in]  esi
; current directory path:   [in]  edi
;                           [out] no
;                           [unchanged] ebx, edx, ebp, esp
	call	GetStrLength
	add	edi,ecx
	mov	al, '/'
	std
	repne scasb
	cld
	sub	edi,ecx
	add	ecx,2
	xchg	esi, edi
	mov	eax, ecx
	shr	ecx, 2
	rep movsd
	mov	ecx, eax
	and	ecx, 3
	rep movsb

	mov	esi, edx
	xchg	esi, edi
	call	GetStrLength
	xchg	esi, edi
	mov	eax, ecx
	shr	ecx, 2
	rep movsd
	mov	ecx, eax
	and	ecx, 3
	rep movsb
	ret

LoadLibrary:
; library name:             [in]  edx
; hInstance (export table): [out] eax
;                           [unchanged] edx, ebp, esp
	mov	edi, cur_dir_path
	mov	esi, library_path
	call	@copy_path_wo_pdname
	mcall	SF_SYS_MISC,SSF_LOAD_DLL,library_path
	test	eax, eax
	jnz	.ret
	lea	ecx, [edx-9]
	mcall	SF_SYS_MISC,SSF_LOAD_DLL,ecx   ; load of sys directory
.ret:
	ret

LoadLibraries:
; [in/out]    no
; [unchanged] esp
	push	0
	push	import.data
virtual at esp+4
  .errStatus dd ?
end virtual
.LoopLibraries:
	mov	eax, [esp]
	mov	ebp, [eax]
	test	ebp, ebp
	jz	.done
	mov	edx, [eax+4]
	call	LoadLibrary;edx=name
	test	eax, eax
	jz	.onError
	mov	esi,eax
	mov	eax, [esp]
	call	ResolveImports
	add	dword[esp],8
	jmp	.LoopLibraries
.onError:
	mov	eax, sz_cantLL
	mov	ebx, sizeof.sz_cantLL
	mov	ecx, 1
	or	[LoadLibraries.errStatus],1
	mcall	SF_TERMINATE_PROCESS ;uncomment to stop after 1st error
	jmp	.LoopLibraries
.done:
	test	[LoadLibraries.errStatus],1
	jz	.exit
	mcall	SF_TERMINATE_PROCESS
.exit:
	add	esp,8
	ret

ShowErrorWindow:
; error string: [in] edx
;               [unchanged] edx, ebp, esp
	mov	edi, [file_io_notify.flags]
	xor	ecx, [import_loader.state]
	test	ecx, ecx
	jz	.rns.FlagsInited
	cmp	ecx, 2
	jz	.rns.FlagsPreInited
.rns.FlagsPreInit:
	mov	esi, sz_system_error
	mov	ecx, sizeof.sz_system_error-1
	cld
	rep movsb
	or	[import_loader.state],1
	jmp	.rns.FlagsInit
.rns.FlagsPreInited:
	add	edi, sizeof.sz_system_error-1
.rns.FlagsInit:
	mov	esi, eax
	xchg	esi, edi
	call	GetStrLength
	dec	ecx
	xchg	esi, edi
	rep movsb
	xor	[import_loader.state],2
	jmp	.rns.FlagsDone
.rns.FlagsInited:
	lea	edi, [edi+ebx-1+sizeof.sz_system_error-1]
.rns.FlagsDone:
	mov	esi, edx
	xchg	esi, edi
	call	GetStrLength
	xchg	esi, edi
	rep movsb
	mcall	SF_FILE,file_io_notify
	ret