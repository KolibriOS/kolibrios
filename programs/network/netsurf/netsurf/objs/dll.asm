include 'proc32.inc'

macro __mov reg,a,b {       ; mike.dld
 if (~a eq)&(~b eq)
   mpack reg,a,b
 else if (~a eq)&(b eq)
   mov reg,a
 end if
} 

macro mcall a,b,c,d,e,f {   ; mike.dld, updated by Ghost for Fast System Calls
 __mov eax,a
 __mov ebx,b
 __mov ecx,c
 __mov edx,d
 __mov esi,e
 __mov edi,f
        int     0x40
}


format ELF
section '.text' executable

public dll_load
public mem_Free
public mem_Alloc
public mem_ReAlloc

;-----------------------------------------------------------------------------
proc dll_load, import_table:dword
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
proc dll.Init, dllentry:dword
	pushad
	mov	eax, mem_Alloc
	mov	ebx, mem_Free
	mov	ecx, mem_ReAlloc
	mov	edx, dll_load
	stdcall	[dllentry]
	popad
	ret
endp
;-----------------------------------------------------------------------------
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
	ret
endp
;-----------------------------------------------------------------------------
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
proc mem_Alloc, size
	push	ebx ecx
	mov	ecx, [size]
	mcall	68, 12
	pop	ecx ebx
	ret
endp
;-----------------------------------------------------------------------------
proc mem_ReAlloc, mptr, size
	push	ebx ecx edx
	mov	ecx, [size]
	or	ecx, ecx
	jz	@f
    @@:
	mov	edx, [mptr]
	or	edx, edx
	jz	@f
    @@:
	mcall	68, 20
	or	eax, eax
	jz	@f
    @@:
	pop	edx ecx ebx
	ret
endp
;-----------------------------------------------------------------------------
proc mem_Free, mptr
	push	ebx ecx
	mov	ecx,[mptr]
	or	ecx,ecx
	jz	@f
    @@:
	mcall	68, 13
	pop	ecx ebx
	ret
endp
;-----------------------------------------------------------------------------
