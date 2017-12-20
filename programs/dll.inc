;-----------------------------------------------------------------------------
; load one or more DLL file in COFF format and try to import functions by our list
; if first function in import list begins with 'lib_', call it as DLL initialization
; return eax = 1 as fail, if anyone of .obj file not found in /sys/lib
; return 0 if all fine, but 0 not garantees in succesfull import - see dll.Link comment
; dirties all registers! eax, ebx, ecx, edx, esi, edi
proc dll.Load, import_table:dword
	mov	esi, [import_table]
  .next_lib:
	mov	edx, [esi]
	or	edx, edx
	jz	.exit
	push	esi
	mov	esi, [esi + 4]
	mov	edi, s_libdir.fname
    @@:
	lodsb
	stosb
	or	al, al
	jnz	@b
	mcall	68, 19, s_libdir
	or	eax, eax
	jz	.fail
	stdcall	dll.Link, eax, edx
	push	eax
	mov	eax, [eax]
	cmp	dword[eax], 'lib_'
	pop	eax
	jnz	@f
	stdcall	dll.Init, [eax + 4]
    @@:
	pop	esi
	add	esi, 8
	jmp	.next_lib
  .exit:
	xor	eax, eax
	ret
  .fail:
	add	esp, 4
	xor	eax, eax
	inc	eax
	ret
endp
;-----------------------------------------------------------------------------
; scans dll export table for a functions we want to import
; break scan on first unresolved import
; no return value
proc dll.Link, exp:dword, imp:dword
	push	eax
	mov	esi, [imp]
	test	esi, esi
	jz	.done
  .next:
	lodsd
	test	eax, eax
	jz	.done
	stdcall	dll.GetProcAddress, [exp], eax
	or	eax, eax
	jz	@f
	mov	[esi - 4], eax
	jmp	.next
    @@:
	mov	dword[esp], 0
  .done:
	pop	eax
	ret
endp
;-----------------------------------------------------------------------------
; calls lib_init with predefined parameters
; no return value
proc dll.Init, dllentry:dword
	pushad
	mov	eax, mem.Alloc
	mov	ebx, mem.Free
	mov	ecx, mem.ReAlloc
	mov	edx, dll.Load
	stdcall	[dllentry]
	popad
	ret
endp
;-----------------------------------------------------------------------------
; scans export table for a sz_name function
; returns in eax function address or 0 if not found
proc dll.GetProcAddress, exp:dword, sz_name:dword
	mov	edx, [exp]
	xor	eax, eax
  .next:
	or	edx, edx
	jz	.end
	cmp	dword[edx], 0
	jz	.end
	stdcall	strcmp, [edx], [sz_name]
	test	eax, eax
	jz	.ok
	add	edx, 8
	jmp	.next
  .ok:
	mov	eax, [edx + 4]
  .end:
	cmp eax, -1
	jnz @f
	xor eax, eax
  @@:
	ret
endp
;-----------------------------------------------------------------------------
; compares strings
; returns eax = 0 if equal, -1 otherwise
proc strcmp, str1:dword, str2:dword
	push	esi edi
	mov	esi, [str1]
	mov	edi, [str2]
	xor	eax, eax
    @@:
	lodsb
	scasb
	jne	.fail
	or	al, al
	jnz	@b
	jmp	.ok
  .fail:
	or	eax, -1
  .ok:
	pop	edi esi
	ret
endp
;-----------------------------------------------------------------------------
s_libdir:
  db '/sys/lib/'
  .fname rb 32
;-----------------------------------------------------------------------------
proc mem.Alloc, size
	push	ebx ecx
	mov	ecx, [size]
	mcall	68, 12
	pop	ecx ebx
	ret
endp
;-----------------------------------------------------------------------------
proc mem.ReAlloc, mptr, size
	push	ebx ecx edx
	mov	ecx, [size]
	mov	edx, [mptr]
	mcall	68, 20
	pop	edx ecx ebx
	ret
endp
;-----------------------------------------------------------------------------
proc mem.Free, mptr
	push	ebx ecx
	mov	ecx,[mptr]
	mcall	68, 13
	pop	ecx ebx
	ret
endp
;-----------------------------------------------------------------------------
