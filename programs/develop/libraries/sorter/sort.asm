format MS COFF
public EXPORTS
section '.flat' code readable align 16
; void __stdcall START(dword state);
DLL_ENTRY equ 1
DLL_EXIT equ -1
START:
	cmp	dword [esp+4], DLL_ENTRY
	jnz	.ret
	push	edi
        mov     edi, tolower_table
        push    'A'
        pop     ecx
        xor     eax, eax
@@:
        stosb
        inc     eax
        loop    @b
        add     al, 0x20
        mov     cl, 'Z'-'A'+1
@@:
        stosb
        inc     eax
        loop    @b
        sub     al, 0x20
        mov     cl, 'А'-'Z'-1
@@:
        stosb
        inc     eax
        loop    @b
        add     al, 0x20
        mov     cl, 'Р'-'А'
@@:
        stosb
        inc     eax
        loop    @b
        add     al, 0x30
        mov     cl, 'Я'-'Р'+1
@@:
        stosb
        inc     eax
        loop    @b
        sub     al, 0x50
        mov     cl, 256-'Я'-1
@@:
        stosb
        inc     eax
        loop    @b
        mov     byte [edi-256+'Ё'], 'ё'
        pop	edi
.ret:
	ret	4

; enum sort_mode
; { SORT_BY_NAME = 0, SORT_BY_NAME_REV = 1,
;   SORT_BY_EXT = 2, SORT_BY_EXT_REV = 3,
;   SORT_BY_MODIFIED = 4, SORT_BY_MODIFIED_REV = 5,
;   SORT_BY_SIZE = 6, SORT_BY_SIZE_REV = 7,
;   SORT_BY_CREATED = 10, SORT_BY_CREATED_REV = 11,
;   SORT_BY_ACCESSED = 12, SORT_BY_ACCESSED_REV = 13 };
; int __stdcall sort_dir(BDFE* folder_data, unsigned num_folder_entries, sort_mode mode);
; return: 0 = ok, 1 = error
sort_dir:
        cmp     dword [esp+12], num_compare_fns
        jae     .error
        push    ebx
        mov     eax, ptr_table + 0x1FFF
        mov     edx, ptr_table
        and     eax, not 0xFFF
        mov     ecx, [esp+4+8]  ; num_folder_entries
        sub     eax, edx
        shl     ecx, 3
        jz      .done
        cmp     eax, ecx
        jae     .memok
        push    68
        pop     eax
        push    12
        pop     ebx
        int     0x40
        test    eax, eax
        jz      .error2
        mov     edx, eax
.memok:
        shr     ecx, 3
        mov     eax, edx
        mov     ebx, [esp+4+4]  ; folder_data
        push    ecx
.init_ptr:
        mov     [eax], ebx
        mov     [eax+4], eax
        add     eax, 8
        add     ebx, 304
        sub     ecx, 1
        jnz     .init_ptr
        pop     ecx
        mov     ebx, [esp+4+12] ; mode
        mov     ebx, [compare_fns+ebx*4]
        test    ebx, ebx
        jz      .done_free
        push    esi edi ecx
        call    sort
        pop     ecx
        mov     ebx, edx
        sub     esp, 304
.moveloop:
        mov     esi, [ebx]
        test    esi, esi
        jz      .movenext
        cmp     [ebx+4], ebx
        jz      .movenext
        mov     esi, [ebx]
        mov     edi, esp
        push    ecx
        mov     ecx, 304/4
        rep     movsd
        mov     edi, [ebx]
        mov     eax, [ebx+4]
.z:
        mov     esi, [eax]
        mov     dword [eax], 0
        push    esi
        mov     ecx, 304/4
        rep     movsd
        pop     edi
        mov     eax, [eax+4]
        cmp     eax, ebx
        jnz     .z
        lea     esi, [esp+4]
        mov     ecx, 304/4
        rep     movsd
        pop     ecx
.movenext:
        add     ebx, 8
        sub     ecx, 1
        jnz     .moveloop
        add     esp, 304
        pop     edi esi
.done_free:
        cmp     edx, ptr_table
        jz      .done
        push    68
        pop     eax
        push    13
        pop     ebx
        mov     ecx, edx
        int     0x40
.done:
        pop     ebx
        xor     eax, eax
        ret     12
.error2:
        pop     ebx
.error:
        push    1
        pop     eax
        ret     12

; Сортировка qword'ов в количестве ecx по адресу edx, функция сравнения в ebx
; Разрушает eax, ecx, esi, edi
sort:
;        jecxz   .done  ; checked by caller
        mov     eax, ecx
@@:
        push    eax
        call    .restore
        pop     eax
        sub     eax, 1
        jnz     @b
@@:
        cmp     ecx, 1
        jz      .done
        mov     esi, 1
        mov     edi, ecx
        call    .exchange
        dec     ecx
        mov     eax, 1
        call    .restore
        jmp     @b
.done:
        ret

.exchange:
        push    eax ecx
        mov     eax, [edx+esi*8-8]
        mov     ecx, [edx+edi*8-8]
        mov     [edx+esi*8-8], ecx
        mov     [edx+edi*8-8], eax
        mov     eax, [edx+esi*8-4]
        mov     ecx, [edx+edi*8-4]
        mov     [edx+esi*8-4], ecx
        mov     [edx+edi*8-4], eax
        pop     ecx eax
        ret

.restore:
        lea     esi, [eax+eax]
        cmp     esi, ecx
        ja      .doner
        push    esi
        mov     esi, [edx+esi*8-8]
        mov     edi, [edx+eax*8-8]
        call    ebx
        pop     esi
        ja      .need_xchg
        cmp     esi, ecx
        jae     .doner
        push    esi
        mov     esi, [edx+esi*8]
        mov     edi, [edx+eax*8-8]
        call    ebx
        pop     esi
        jbe     .doner
.need_xchg:
        cmp     esi, ecx
        jz      .do_xchg
        push    esi
        mov     edi, [edx+esi*8-8]
        mov     esi, [edx+esi*8]
        call    ebx
        pop     esi
        sbb     esi, -1
.do_xchg:
        mov     edi, eax
        call    .exchange
        mov     eax, esi
        jmp     .restore
.doner:
        ret

compare_name:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        add     esi, 40
        add     edi, 40
        jmp     strcmpi
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_name_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        add     esi, 40
        add     edi, 40
        xchg    esi, edi
        jmp     strcmpi
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

strcmpi:
        push    eax
@@:
        lodsb
        call    match_symbol
        jnz     .ret
        inc     edi
        test    al, al
        jnz     @b
.ret:
        pop     eax
        ret

compare_ext:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        add     esi, 40
        add     edi, 40
        push    esi edi
        call    seek_ext
        xchg    esi, edi
        call    seek_ext
        xchg    esi, edi
        call    strcmpi
        jnz     .ret
        pop     edi esi
        jmp     strcmpi
.ret:
        pop     edi esi
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_ext_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        add     esi, 40
        add     edi, 40
        push    esi edi
        call    seek_ext
        xchg    esi, edi
        call    seek_ext
        call    strcmpi
        jnz     .ret
        pop     edi esi
        xchg    esi, edi
        jmp     strcmpi
.ret:
        pop     edi esi
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

seek_ext:
        push    eax
        xor     eax, eax
.l:
        inc     esi
        cmp     byte [esi-1], '.'
        jnz     @f
        mov     eax, esi
@@:
        cmp     byte [esi-1], 0
        jnz     .l
        test    eax, eax
        jnz     @f
        lea     eax, [esi-1]
@@:
        mov     esi, eax
        pop     eax
        ret

compare_modified:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    edi
        mov     edi, [edi+28]
        cmp     edi, [esi+28]
        pop     edi
        jnz     @f
        push    edi
        mov     edi, [edi+24]
        cmp     edi, [esi+24]
        pop     edi
        jnz     @f
        add     esi, 40
        add     edi, 40
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_modified_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    esi
        mov     esi, [esi+28]
        cmp     esi, [edi+28]
        pop     esi
        jnz     @f
        push    esi
        mov     esi, [esi+24]
        cmp     esi, [edi+24]
        pop     esi
        jnz     @f
        add     esi, 40
        add     edi, 40
        xchg    esi, edi
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_size:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    edi
        mov     edi, [edi+36]
        cmp     edi, [esi+36]
        pop     edi
        jnz     @f
        push    edi
        mov     edi, [edi+32]
        cmp     edi, [esi+32]
        pop     edi
        jnz     @f
        add     esi, 40
        add     edi, 40
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_size_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    esi
        mov     esi, [esi+36]
        cmp     esi, [edi+36]
        pop     esi
        jnz     @f
        push    esi
        mov     esi, [esi+32]
        cmp     esi, [edi+32]
        pop     esi
        jnz     @f
        add     esi, 40
        add     edi, 40
        xchg    esi, edi
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_unordered:
        cmp     esi, edi
        ret
compare_unordered_rev:
        cmp     edi, esi
        ret

compare_created:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    edi
        mov     edi, [edi+12]
        cmp     edi, [esi+12]
        pop     edi
        jnz     @f
        push    edi
        mov     edi, [edi+8]
        cmp     edi, [esi+8]
        pop     edi
        jnz     @f
        add     esi, 40
        add     edi, 40
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_created_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    esi
        mov     esi, [esi+12]
        cmp     esi, [edi+12]
        pop     esi
        jnz     @f
        push    esi
        mov     esi, [esi+8]
        cmp     esi, [edi+8]
        pop     esi
        jnz     @f
        add     esi, 40
        add     edi, 40
        xchg    esi, edi
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_accessed:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    edi
        mov     edi, [edi+20]
        cmp     edi, [esi+20]
        pop     edi
        jnz     @f
        push    edi
        mov     edi, [edi+16]
        cmp     edi, [esi+16]
        pop     edi
        jnz     @f
        add     esi, 40
        add     edi, 40
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

compare_accessed_rev:
        test    byte [esi], 10h
        jnz     .1dir
        test    byte [edi], 10h
        jnz     .greater
.eq1:
        push    esi
        mov     esi, [esi+20]
        cmp     esi, [edi+20]
        pop     esi
        jnz     @f
        push    esi
        mov     esi, [esi+16]
        cmp     esi, [edi+16]
        pop     esi
        jnz     @f
        add     esi, 40
        add     edi, 40
        xchg    esi, edi
        jmp     strcmpi
@@:
        ret
.greater:
        test    esi, esi
        ret
.1dir:
        test    byte [edi], 10h
        jnz     .eq1
.less:
        xor     edi, edi
        stc
        ret

match_symbol:
; in: al,[edi]=symbols
; out: flags as 'cmp al,[edi]'
        push    eax ecx
        movzx   ecx, al
        mov     al, [tolower_table+ecx]
        movzx   ecx, byte [edi]
        cmp     al, [tolower_table+ecx]
        pop     ecx eax
        ret

align 4
compare_fns     dd      compare_name
                dd      compare_name_rev
                dd      compare_ext
                dd      compare_ext_rev
                dd      compare_modified
                dd      compare_modified_rev
                dd      compare_size
                dd      compare_size_rev
;                dd      compare_unordered
;                dd      compare_unordered_rev
                dd      0
                dd      0
                dd      compare_created
                dd      compare_created_rev
                dd      compare_accessed
                dd      compare_accessed_rev
num_compare_fns = ($ - compare_fns) / 4

EXPORTS:
	dd	szStart,	START
	dd	szVersion,	0x00010001
	dd	szSortDir,	sort_dir
	dd	szStrcmpi,	strcmpi
	dd	0
szStart		db	'START',0
szVersion       db      'version',0
szSortDir	db	'SortDir',0
szStrcmpi	db	'strcmpi',0

section '.data' data readable writable align 16
tolower_table	rb	256
; ptr_table must be last variable
ptr_table       rb      0x1000
