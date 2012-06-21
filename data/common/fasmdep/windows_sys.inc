; Platform-specific procedures for Windows.

; Reallocate memory at pointer [start.buf] and size [start.allocated],
; new size is in eax.
realloc:
        push    eax
        stdcall [VirtualAlloc], 0, eax, MEM_COMMIT + MEM_RESERVE, PAGE_READWRITE
        pop     ecx
        test    eax, eax
        jz      nomemory
        add     [start.free], ecx
        xchg    ecx, [start.allocated]
        sub     [start.free], ecx
        push    esi edi
        mov     edi, eax
        xchg    eax, [start.buf]
        shr     ecx, 2
        jz      .nothing
        mov     esi, eax
        rep     movsd
        call    free_eax
.nothing:
        pop     edi esi
        ret

; Read the next portion of input data to [start.buf].
read:
        push    eax     ; reserve space for *lpNumberOfBytesRead
        mov     ecx, esp
        mov     eax, [start.buf]
        add     eax, [start.allocated]
        sub     eax, [start.free]
        stdcall [ReadFile], [start.in], eax, [start.free], ecx, 0
        test    eax, eax
        jz      @f
.nothing:
        pop     eax
        ret
@@:
; ERROR_BROKEN_PIPE and ERROR_MORE_DATA are normal codes when dealing with pipes
        call    [GetLastError]
        cmp     eax, ERROR_BROKEN_PIPE
        jz      .nothing
        cmp     eax, ERROR_MORE_DATA
        jz      .nothing
        pop     eax
        jmp     readerr

; Write output data: eax=pointer, edi=size.
write:
        push    eax
        mov     ecx, esp
        stdcall [WriteFile], [start.out], eax, edi, ecx, 0
        test    eax, eax
        pop     eax
        jz      writeerr
        cmp     eax, edi
        jnz     writeerr
        ret

; Parse command line, open input and output files.
get_params:
; 1. Get the command line split to argv[] array.
        call    [GetCommandLineW]
        push    eax     ; reserve space for *pNumArgs
        stdcall [CommandLineToArgvW], eax, esp
        pop     ebx     ; ebx = argc, eax = argv
        push    eax     ; save argument for [LocalFree]
; 2. Prepare for scanning, skipping argv[0].
        cmp     ebx, 1
        jbe     .noargs
        dec     ebx
        lea     esi, [eax+4]    ; skip argv[0]
        xor     edi, edi        ; no args parsed yet
; 3. Parse loop.
.parse:
; 3a. Get the next argument.
        lodsd
; 3b. Check whether it is a known option.
        cmp     dword [eax], '-' + 'e' * 65536
        jnz     @f
        cmp     word [eax+4], 0
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
        ja      .toomanyargs
; 3e. Some parameters of CreateFileW differ for input and output. Setup them.
        mov     ecx, GENERIC_WRITE
        mov     edx, CREATE_ALWAYS
        jz      @f
        add     ecx, ecx        ; GENERIC_READ
        inc     edx             ; OPEN_EXISTING
@@:
; 3f. Open/create the file, save the handle.
        stdcall [CreateFileW], eax, ecx, FILE_SHARE_READ+FILE_SHARE_DELETE, 0, edx, FILE_ATTRIBUTE_NORMAL, 0
        cmp     eax, INVALID_HANDLE_VALUE
        jz      .fileerr
        mov     [start.in+edi*4], eax
        inc     edi
.nextarg:
        dec     ebx
        jnz     .parse
.noargs:
; 4. End of command line reached. If input and/or output was not given, use defaults.
        test    edi, edi
        jnz     .hasinput
        stdcall [GetStdHandle], STD_INPUT_HANDLE
        mov     [start.in], eax
.hasinput:
        cmp     edi, 1
        ja      .hasoutput
        stdcall [GetStdHandle], STD_OUTPUT_HANDLE
        mov     [start.out], eax
.hasoutput:
; 5. Free memory allocated in step 1 and return.
        call    [LocalFree]
        ret
.toomanyargs:
        call    [LocalFree]
        jmp     information
.fileerr:
        call    [LocalFree]
        test    edi, edi
        jz      in_openerr
        jmp     out_openerr

; Exit, return code is in al.
exit:
        movzx   eax, al
        push    eax     ; save return code for [ExitProcess]
        mov     eax, [start.buf]
        test    eax, eax
        jz      @f
        call    free_eax
@@:
        stdcall [CloseHandle], [start.in]
        stdcall [CloseHandle], [start.out]
        call    [ExitProcess]

; Output the message given in esi to stderr.
sayerr:
        stdcall [GetStdHandle], STD_ERROR_HANDLE
        push    eax
        mov     ecx, esp
        movzx   edx, byte [esi-1]
        stdcall [WriteFile], eax, esi, edx, ecx, 0
        pop     ecx
        ret

; Helper procedure for realloc and exit.
free_eax:
        stdcall [VirtualFree], eax, 0, MEM_RELEASE
        ret

; Get environment variable esi (ebx-relative pointer) to the buffer,
; expanding it if needed.
get_environment_variable:
        lea     eax, [edi+ebx]
        lea     ecx, [esi+ebx]
        stdcall [GetEnvironmentVariableA], ecx, eax, [start.free]
; GetEnvironmentVariable returns one of following values:
; * if all is ok: number of characters copied to the buffer,
;       not including terminating zero
; * if buffer size is too small: number of characters required in the buffer,
;       including terminating zero
; * if environment variable not found: zero
; We treat the last case the same as first one.
        cmp     eax, [start.free]
        jae     .resize
        inc     eax     ; include terminating zero
        add     edi, eax
        sub     [start.free], eax
        mov     byte [edi+ebx-1], 0     ; force zero-terminated or empty string
        ret
.resize:
; esi can be inside the buffer or outside the buffer;
; we must not correct it in the first case,
; but advance relative to new value of ebx in the second one.
        mov     ecx, esi
        cmp     esi, [start.allocated]
        jb      @f
        add     esi, ebx
@@:
        stdcall alloc_in_buf, eax
        cmp     esi, ecx
        jz      get_environment_variable
        sub     esi, ebx
        jmp     get_environment_variable

; Test whether a file with name [.testname] exists.
; Returns eax<0 if not, nonzero otherwise.
test_file_exists:
        mov     eax, [start.testname]
        add     eax, ebx
        stdcall [GetFileAttributesA], eax
        inc     eax
        ret

; Imports
align 4
data import
library kernel32,'kernel32.dll',shell32,'shell32.dll'
import kernel32, \
        GetLastError, 'GetLastError', \
        ExitProcess, 'ExitProcess', \
        VirtualAlloc, 'VirtualAlloc', \
        VirtualFree, 'VirtualFree', \
        LocalFree, 'LocalFree', \
        GetStdHandle, 'GetStdHandle', \
        CreateFileW, 'CreateFileW', \
        GetFileAttributesA, 'GetFileAttributesA', \
        ReadFile, 'ReadFile', \
        WriteFile, 'WriteFile', \
        CloseHandle, 'CloseHandle', \
        GetCommandLineW, 'GetCommandLineW', \
        GetEnvironmentVariableA, 'GetEnvironmentVariableA'
import shell32, CommandLineToArgvW, 'CommandLineToArgvW'
end data
