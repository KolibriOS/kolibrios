pgalloc:
; in: ecx=size
; out: eax=pointer or NULL
        push    ebx
        push    68
        pop     eax
        push    12
        pop     ebx
        mcall
        pop     ebx
        ret

pgfree:
; in: ecx=pointer
; destroys eax
        push    ebx
        push    68
        pop     eax
        push    13
        pop     ebx
        mcall
        pop     ebx
        ret

pgrealloc:
; in: ecx=size, edx=pointer
; out: eax=pointer
        push    ebx
        push    68
        pop     eax
        push    20
        pop     ebx
        mcall
        pop     ebx
        ret

xpgalloc:
; in: ecx=size
; out: eax=pointer or NULL
        call    pgalloc
.common:
        test    eax, eax
        jnz     @f
        call    SayNoMem
        xor     eax, eax
@@:
        ret

xpgrealloc:
; in: edx=pointer, ecx=new size
; out: eax=pointer or NULL
        call    pgrealloc
        jmp     xpgalloc.common

get_error_msg:
; in: eax=error code
; out: eax=pointer to message (in static buffer)
        push    esi edi
        mov     edi, error_msg
        cmp     eax, 11
        ja      .no1
        mov     esi, [errors1+eax*4]
        jmp     .copy
.no1:
        cmp     eax, 30
        jb      .no2
        cmp     eax, 32
        ja      .no2
        mov     esi, [errors2+(eax-30)*4]
.copy:
        lodsb
        stosb
        test    al, al
        jnz     .copy
.ret:
        mov     eax, error_msg
        pop     edi esi
        ret
.no2:
        mov     esi, aUnknownError
        push    eax
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     eax
        push    edx ecx
        test    eax, eax
        jns     @f
        mov     byte [edi], '-'
        inc     edi
        neg     eax
@@:
        xor     edx, edx
        mov     ecx, 10
        div     ecx
        add     edx, '0'
        mov     byte [edi], dl
        inc     edi
        test    eax, eax
        jnz     @b
        pop     ecx edx
        stosb
        jmp     .ret

libini_alloc:
        push    ecx
        mov     ecx, [esp+8]
        call    xpgalloc
        pop     ecx
        ret     4
libini_free:
        push    ecx
        mov     ecx, [esp+8]
        call    pgfree
        pop     ecx
        ret     4
libini_realloc:
        push    ecx edx
        mov     edx, [esp+8+4]
        mov     ecx, [esp+8+8]
        call    xpgrealloc
        pop     edx ecx
        ret     8

libini_dllload:
        push    esi
        mov     esi, [esp+8]
.lib:
        lodsd
        test    eax, eax
        jz      .ret
        push    eax
        lodsd
        xchg    esi, [esp]
        xor     ebp, ebp        ; no version control
        call    load_dll_and_import
        pop     esi
        test    eax, eax
        jz      .lib
.ret:
        pop     esi
        ret     4

load_dll_and_import:
        cmp     byte [eax], '/'
        jz      .do
        push    esi
        mov     edi, saved_file_name
        push    edi
        mov     esi, standard_dll_path
        mov     ecx, standard_dll_path_size
        rep     movsb
        mov     esi, eax
        mov     ecx, 1024-standard_dll_path_size
@@:
        lodsb
        stosb
        test    al, al
        loopnz  @b
        pop     eax
        pop     esi
        jz      .do
        push    eax
        mov     edi, aFileNameTooBig
.sayerr:
        push    dword aCannotLoadDLL
        push    edi
        mov     eax, esp
        push    dword aOk
        push    esp
        push    1
        push    eax
        push    3
        push    -1
        push    -1
        push    dword aError
        call    SayErr
        add     esp, 16
        xor     eax, eax
        inc     eax
        ret
.do:
        push    eax
        mov     ecx, eax
        push    68
        pop     eax
        push    19
        pop     ebx
        mcall
        mov     edi, aInvalidDLL
        test    eax, eax
        jz      .sayerr
; initialize import
        mov     edi, aMissingExport
        mov	edx, eax
.import_loop:
        lodsd
        test    eax, eax
        jz      .import_done
        call    .find_exported_function
        jc      .sayerr
        mov     [esi-4], eax
        jmp     .import_loop
.import_done:
; check version
        test    ebp, ebp
        jz      .version_ok
        mov     edi, aIncompatibleVersion
        mov     eax, aVersion
        call    .find_exported_function
        jc      .sayerr
        cmp     ax, bp
        jb      .sayerr
        shr     eax, 16
        cmp     eax, ebp
        ja      .sayerr
.version_ok:
; initialize library
        mov     eax, aStart
        call    .find_exported_function
        jc      @f
        push    1       ; DLL_ENTRY
        call    eax
.ret0:
        pop     eax
        xor     eax, eax
        ret
@@:
        mov     eax, aLibInit
        call    .find_exported_function
        jc      .ret0
        mov     esi, eax
        mov     eax, libini_alloc
        mov     ebx, libini_free
        mov     ecx, libini_realloc
        mov     edx, libini_dllload
        call    esi
        mov     edi, aInitFailed
        test    eax, eax
        jnz     .sayerr
        jmp     .ret0

.find_exported_function:
        push    edx
.import_loop_i:
        mov     ebx, [edx]
        test    ebx, ebx
        jz      .import_notfound
        push    eax
@@:
        mov     cl, [eax]
        cmp     cl, [ebx]
        jnz     .import_find_next
        test    cl, cl
        jz      .import_found
        inc     eax
        inc     ebx
        jmp     @b
.import_find_next:
        pop     eax
        add     edx, 8
        jmp     .import_loop_i
.import_found:
        pop     eax
        mov     eax, [edx+4]
        pop     edx
        ret
.import_notfound:
        pop     edx
        stc
        ret
