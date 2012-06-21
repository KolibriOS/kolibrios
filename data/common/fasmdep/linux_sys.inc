; Platform-specific procedures for Linux.

; Reallocate memory at pointer [start.buf] and size [start.allocated],
; new size is in eax.
realloc:
        push    ebx esi edi
        mov     ecx, eax
        push    ebp
        kercall __NR_mmap2, 0, , PROT_READ+PROT_WRITE, MAP_PRIVATE+MAP_ANONYMOUS, -1, 0
        pop     ebp
        cmp     eax, 0xFFFFF000
        ja      nomemory
        add     [start.free], ecx
        xchg    ecx, [start.allocated]
        sub     [start.free], ecx
        mov     edi, eax
        xchg    eax, [start.buf]
        shr     ecx, 2
        jz      .nothing
        push    ecx
        mov     esi, eax
        rep     movsd
        pop     ecx
        shl     ecx, 2
        call    free_eax_ecx
.nothing:
        pop     edi esi ebx
        ret

; Read the next portion of input data to [start.buf].
read:
        mov     ecx, [start.buf]
        add     ecx, [start.allocated]
        mov     edx, [start.free]
        sub     ecx, edx
        kercall __NR_read, [start.in], ,
        test    eax, eax
        js      readerr
        ret

; Write output data: eax=pointer, edi=size.
write:
        mov     ecx, eax
        kercall __NR_write, [start.out], , edi
        cmp     eax, edi
        jnz     writeerr
        ret

; Parse command line, open input and output files.
get_params:
; 1. Initialize default streams: 0 for stdin, 1 for stdout.
        inc     [start.out]
; 2. Prepare for scanning, skipping argv[0].
        mov     ebx, [start.argc]
        cmp     ebx, 1
        jbe     .noargs
        dec     ebx
        lea     esi, [start.argv+4]     ; skip argv[0]
        xor     edi, edi        ; no args parsed yet
; 3. Parse loop.
.parse:
; 3a. Get the next argument.
        lodsd
; 3b. Check whether it is a known option.
        cmp     word [eax], '-e'
        jnz     @f
        cmp     byte [eax+2], 0
        jnz     @f
; 3c. If it is, modify flags and continue the loop.
        mov     [start.flags], 1        ; '-e' is given
        jmp     .nextarg
@@:
; 3d. Otherwise, it is a name of input or output file.
; edi keeps the count of names encountered before;
; edi = 0 means this is input file, edi = 1 - output file,
; otherwise this is third arg, which is an error
        cmp     edi, 1
        ja      information
; 3e. Some parameters of __NR_open differ for input and output. Setup them.
        mov     ecx, O_WRONLY+O_CREAT+O_TRUNC
        mov     edx, 0644o
        jz      @f
        mov     ecx, O_RDONLY
@@:
; 3f. Open/create the file, save the handle.
        push    ebx
        mov     ebx, eax
        kercall __NR_open
        pop     ebx
        test    eax, eax
        js      .fileerr
        mov     [start.in+edi*4], eax
        inc     edi
.nextarg:
        dec     ebx
        jnz     .parse
.noargs:
; 4. End of command line reached. Return.
        ret
.fileerr:
        test    edi, edi
        jz      in_openerr
        jmp     out_openerr

; Exit, return code is in al.
exit:
        movzx   ebx, al
        push    ebx
        mov     eax, [start.buf]
        test    eax, eax
        jz      @f
        mov     ecx, [start.allocated]
        call    free_eax_ecx
@@:
        kercall __NR_close, [start.in]
        kercall __NR_close, [start.out] 
        pop     ebx
        kercall __NR_exit

; Helper procedure for realloc and exit.
free_eax_ecx:
        mov     ebx, eax
        kercall __NR_munmap
        ret

; Output the message given in esi to stderr.
sayerr:
        movzx   edx, byte [esi-1]
        kercall __NR_write, 2, esi, 
        ret

; Get environment variable esi (ebx-relative pointer) to the buffer,
; expanding it if needed.
get_environment_variable:
        mov     ecx, [start.argc]
        lea     ecx, [start.argv+ecx*4+4]
.findvar:
        mov     edx, [ecx]
        add     ecx, 4
        test    edx, edx
        jz      .notfound
        push    esi
        add     esi, ebx
.comparename:
        lodsb
        cmp     al, [edx]
        jnz     @f
        inc     edx
        jmp     .comparename
@@:
        pop     esi
        test    al, al
        jnz     .findvar
        cmp     byte [edx], '='
        jnz     .findvar
        inc     edx
        xor     eax, eax
@@:
        inc     eax
        cmp     byte [edx+eax-1], 0
        jnz     @b
        stdcall alloc_in_buf, eax
@@:
        mov     al, [edx]
        inc     edx
        mov     [edi+ebx], al
        inc     edi
        test    al, al
        jnz     @b
        ret
.notfound:
        stdcall alloc_in_buf, 1
        mov     byte [edi+ebx], 0
        inc     edi
        ret

; Test whether a file with name [.testname] exists.
; Returns eax<0 if not, nonzero otherwise.
test_file_exists:
        push    ebx
        add     ebx, [start.testname]
        sub     esp, 800h
        kercall __NR_stat, , esp
        add     esp, 800h
        pop     ebx
        ret
