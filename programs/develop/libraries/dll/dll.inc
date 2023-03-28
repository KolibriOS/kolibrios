;
;  (C) KolibriOS team (original dll.inc)
;  (C) 2022, Edited by Coldy
;
;   This module based on original dll.inc.
;   
;   - Improved error handling. Now dll.Load return 0 is success guarantie
;     Also added corrsponding error codes if one of library or entry not found   
;


ERROR_LIBRARY_NOT_LOAD  =   0x100
ERROR_ENTRY_NOT_FOUND   =   0x101


;-----------------------------------------------------------------------------
; load one or more DLL file in COFF format and try to import functions by our list
; if first function in import list begins with 'lib_', call it as DLL initialization
; return eax = 1 as fail, if anyone of .obj file not found in /sys/lib
; return 0 if all fine or error code LIBRARY_NOT_LOAD or ENTRY_NOT_FOUND
; dirties all registers! eax, ebx, ecx, edx, esi, edi
proc dll.Load, import_table:dword
        mov     esi, [import_table]
  .next_lib:
        mov     edx, [esi]
        or      edx, edx
        jz      .exit
        push    esi
        mov     esi, [esi + 4]

        mov     edi, esi
        cmp     byte[esi], '/'
        jz      .load_lib

        mov     edi, s_libdir.fname
    @@:
        lodsb
        stosb
        or      al, al
        jnz     @b

        mov     edi, s_libdir
  .load_lib:
        mcall   68, 19, edi ;s_libdir
        or      eax, eax
        jz      .fail_load
        push eax
        stdcall dll.Link, eax, edx
        test  eax, eax
        jnz     .fail_link
        ;push   eax
        mov eax, [esp]
        mov     eax, [eax]
        cmp     dword[eax], 'lib_'
        pop     eax
        jnz     @f
        stdcall dll.Init, [eax + 4]
    @@:
        pop     esi
        add     esi, 8
        jmp     .next_lib
  .exit:
        xor     eax, eax
        ret
  .fail_load:
        add     esp, 4
        ;xor    eax, eax
        ;inc    eax
  mov eax, ERROR_LIBRARY_NOT_LOAD
  ret
  .fail_link:
  add   esp, 4
        ret
endp
;-----------------------------------------------------------------------------
; scans dll export table for a functions we want to import
; break scan on first unresolved import
; return value: 0 - success or ENTRY_NOT_FOUND
proc dll.Link, exp:dword, imp:dword
        ;push   eax
        mov     esi, [imp]
  ; Import table alreary checked in APP_STARTUP_THUNK
        ;test   esi, esi
        ;jz     .fail1;.done
  .next:
        lodsd
        test    eax, eax
        jz      .done
  mov ebx, eax
        stdcall dll.GetProcAddress, [exp], eax
        or      eax, eax
        jz      .fail ;.done
        mov     [esi - 4], eax
        jmp     .next
  ;  @@:
        ;mov    dword[esp], 0
  ;.fail1:
  ; No imports
  ;mov eax, BAD_IMAGE
  ;jmp .done
  .fail:
  mov [szEntryName],ebx
  mov eax, ERROR_ENTRY_NOT_FOUND
  .done:
        ;pop    eax  
        ret
endp
;-----------------------------------------------------------------------------
; calls lib_init with predefined parameters
; no return value
proc dll.Init, dllentry:dword
        pushad
        mov     eax, mem.Alloc
        mov     ebx, mem.Free
        mov     ecx, mem.ReAlloc
        mov     edx, dll.Load
        stdcall [dllentry]
        popad
        ret
endp
;-----------------------------------------------------------------------------
; scans export table for a sz_name function
; returns in eax function address or 0 if not found
proc dll.GetProcAddress, exp:dword, sz_name:dword
        mov     edx, [exp]
        xor     eax, eax
  .next:
        or      edx, edx
        jz      .end
        cmp     dword[edx], 0
        jz      .end
        stdcall strcmp, [edx], [sz_name]
        test    eax, eax
        jz      .ok
        add     edx, 8
        jmp     .next
  .ok:
        mov     eax, [edx + 4]
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
        push    esi edi
        mov     esi, [str1]
        mov     edi, [str2]
        xor     eax, eax
    @@:
        lodsb
        scasb
        jne     .fail
        or      al, al
        jnz     @b
        jmp     .ok
  .fail:
        or      eax, -1
  .ok:
        pop     edi esi
        ret
endp
;-----------------------------------------------------------------------------

s_libdir:
  db '/sys/lib/'
  .fname rb 32

szEntryName dd  0

;-----------------------------------------------------------------------------
proc mem.Alloc, size
        push    ebx ecx
        mov     ecx, [size]
        mcall   68, 12
        pop     ecx ebx
        ret
endp
;-----------------------------------------------------------------------------
proc mem.ReAlloc, mptr, size
        push    ebx ecx edx
        mov     ecx, [size]
        mov     edx, [mptr]
        mcall   68, 20
        pop     edx ecx ebx
        ret
endp
;-----------------------------------------------------------------------------
proc mem.Free, mptr
        push    ebx ecx
        mov     ecx,[mptr]
        mcall   68, 13
        pop     ecx ebx
        ret
endp
;-----------------------------------------------------------------------------
