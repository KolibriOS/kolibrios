
proc dll.Load, import_table:dword
		mov	esi,[import_table]
  .next_lib:	mov	edx,[esi]
		or	edx,edx
		jz	.exit
		push	esi
		mov	esi,[esi+4]
		mov	edi,s_libdir.fname
	    @@: lodsb
		stosb
		or	al,al
		jnz	@b
		mcall	68,19,s_libdir
		or	eax,eax
		jz	.fail
		stdcall dll.Link,eax,edx
		push	eax
		mov	eax, [eax]
		cmp	dword [eax], 'lib_'
		pop	eax
		jnz	@f
		stdcall dll.Init,[eax+4]
	    @@:
		pop	esi
		add	esi,8
		jmp	.next_lib
  .exit:	xor	eax,eax
		ret
  .fail:	add	esp,4
		xor	eax,eax
		inc	eax
		ret
endp

proc dll.Link, exp:dword,imp:dword
		push	eax
		mov	esi,[imp]
		test	esi,esi
		jz	.done
  .next:	lodsd
		test	eax,eax
		jz	.done
		stdcall dll.GetProcAddress,[exp],eax
		or	eax,eax
		jz	@f
		mov	[esi-4],eax
		jmp	.next
	    @@: mov	dword[esp],0
  .done:	pop	eax
		ret
endp

proc dll.Init, dllentry:dword
		pushad
		mov	eax,mem.Alloc
		mov	ebx,mem.Free
		mov	ecx,mem.ReAlloc
		mov	edx,dll.Load
		stdcall [dllentry]
		popad
		ret
endp

proc dll.GetProcAddress, exp:dword,sz_name:dword
		mov	edx,[exp]
		xor	eax,eax
  .next:	or	edx,edx
		jz	.end
		cmp	dword[edx],0
		jz	.end
		stdcall strcmp,[edx],[sz_name]
		test	eax,eax
		jz	.ok
		add	edx,8
		jmp	.next
  .ok:		mov	eax,[edx+4]
  .end: 	ret
endp

proc strcmp, str1:dword,str2:dword
		push	esi edi
		mov	esi,[str1]
		mov	edi,[str2]
		xor	eax,eax
	    @@: lodsb
		scasb
		jne	.fail
		or	al,al
		jnz	@b
		jmp	.ok
  .fail:	or	eax,-1
  .ok:		pop	edi esi
		ret
endp

; void* __stdcall mem.Alloc(unsigned size);
mem.Alloc:
	push	ebx ecx
	mov	ecx, [esp+12]
	mcall	68, 12
	pop	ecx ebx
	ret	4

; void* __stdcall mem.ReAlloc(void* mptr, unsigned size);
mem.ReAlloc:
	push	ebx ecx edx
	mov	edx, [esp+16]
	mov	ecx, [esp+20]
	mcall	68, 20
	pop	edx ecx ebx
	ret	8

; void __stdcall mem.Free(void* mptr);
mem.Free:
	push	ebx ecx
	mov	ecx, [esp+12]
	mcall	68, 13
	pop	ecx ebx
	ret	4

s_libdir:
  db '/sys/lib/'
  .fname rb 32
