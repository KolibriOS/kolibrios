format PE DLL GUI 0.8 at 7FF00000h
entry start
include '../../struct.inc'
include '../../proc32.inc'
include 'fpo.inc'
include 'export.inc'
include 'pe.inc'
section '.text' code readable executable

FS_STACK_MAX equ dword [fs:4]
FS_STACK_MIN equ dword [fs:8]
FS_SELF_PTR equ dword [fs:0x18]
FS_PROCESS_DATA equ dword [fs:0x30]
FS_ERRNO equ dword [fs:0x34]
FS_SYSCALL_PTR equ dword [fs:0xC0]

ENOMEM = 12

DLL_PROCESS_DETACH = 0
DLL_PROCESS_ATTACH = 1
DLL_THREAD_ATTACH = 2
DLL_THREAD_DETACH = 3

SYSCALL_METHOD_I40 = 1
SYSCALL_METHOD_SYSENTER = 2
SYSCALL_METHOD_SYSCALL = 3

; Pointer to this structure is passed as the third argument
; to 'start' procedure by the kernel.
struct kernel_init_data
version         dw      ?
flags           dw      ?
syscall_method  dd      ?
; either one of SYSCALL_METHOD_xxx or pointer to procedure
exe_base        dd      ?
stack_base      dd      ?
stack_size      dd      ?
exe_path        dd      ?
command_line    dd      ?
environment     dd      ?
ends

include 'malloc.inc'
include 'peloader.inc'
include 'cmdline.inc'

proc syscall_int40
        int     0x40
        ret
endp

proc syscall_sysenter
        push    ebp
        mov     ebp, esp
        push    @f
        sysenter
@@:
        pop     edx
        pop     ecx
        ret
endp

proc syscall_syscall
        push    ecx
        syscall
        pop     ecx
        ret
endp

proc kercall
        jmp     FS_SYSCALL_PTR
endp

prologue@proc equ fpo_prologue
epilogue@proc equ fpo_epilogue

proc start stdcall, dll_base, reason, reserved
locals
exe_base dd ?
exe_path_size dd ?
endl
; 1. Do nothing unless called by the kernel for DLL_PROCESS_ATTACH.
        cmp     [reason], DLL_PROCESS_ATTACH
        jnz     .nothing
; 2. Initialize process.
; 2a. Validate version of the init struct.
; If not known, say a debug message and die.
        mov     ebp, [reserved]
        mov     esi, [dll_base]
        cmp     [ebp+kernel_init_data.version], 1
        jnz     .version_mismatch
; 2b. Get the system call code.
; Note: relocations have not been fixed yet,
; so we cannot use absolute addresses, only RVAs.
        mov     eax, [ebp+kernel_init_data.syscall_method]
        cmp     eax, 0x10000
        jae     .syscall_absolute
        dec     eax
        mov     edx, rva syscall_int40
        cmp     eax, num_syscall_methods
        jae     @f
        mov     edx, [esi+eax*4+rva syscall_methods]
@@:
        lea     eax, [edx+esi]
.syscall_absolute:
        mov     FS_SYSCALL_PTR, eax
; 2c. Fixup relocations so that we can use absolute offsets instead of RVAs
; in rest of code.
; Note: this uses syscalls, so this step should be done after
; configuring FS_SYSCALL_PTR at step 2b.
        push    kolibri_dll
        call    fixup_pe_relocations
        pop     ecx
        jc      .die
; 2d. Allocate process data.
        mov     eax, 68
        mov     ebx, 12
        mov     ecx, 0x1000
        call    FS_SYSCALL_PTR
        mov     FS_PROCESS_DATA, eax
; 2e. Initialize process heap.
        mov     eax, [ebp+kernel_init_data.exe_base]
        mov     [exe_base], eax
        mov     edx, [eax+STRIPPED_PE_HEADER.SizeOfHeapReserve]
        cmp     word [eax], 'MZ'
        jnz     @f
        add     eax, [eax+IMAGE_DOS_HEADER.e_lfanew]
        mov     edx, [eax+IMAGE_NT_HEADERS.OptionalHeader.SizeOfHeapReserve]
@@:
        malloc_init
; 2f. Copy rest of init struct and free memory.
; Parse command line to argc/argv here and move arguments to the heap
; in order to save memory: init struct and heap use different pages,
; but typically data from init struct are far from the entire page,
; so moving it to heap does not increase actual physical heap size
; and allows to free init struct.
        mov     eax, [ebp+kernel_init_data.stack_base]
        mov     FS_STACK_MIN, eax
        add     eax, [ebp+kernel_init_data.stack_size]
        mov     FS_STACK_MAX, eax
        mov     eax, [ebp+kernel_init_data.exe_path]
@@:
        inc     eax
        cmp     byte [eax-1], 0
        jnz     @b
        sub     eax, [ebp+kernel_init_data.exe_path]
        mov     [exe_path_size], eax
        mov     esi, [ebp+kernel_init_data.command_line]
        xor     edx, edx
        xor     edi, edi
        call    parse_cmdline
        inc     ebx ; argv[0] = exe path
.argc equ dll_base
.argv equ reason
.envp equ reserved
        mov     [.argc], ebx
        sub     esi, [ebp+kernel_init_data.command_line]
        lea     esi, [esi+(ebx+1)*4]
        add     esi, [exe_path_size]
        stdcall malloc, esi
        mov     [.argv], eax
        mov     edx, eax
        lea     edi, [eax+ebx*4]
        mov     esi, [ebp+kernel_init_data.exe_path]
        mov     [edx], edi
        add     edx, 4
        mov     ecx, [exe_path_size]
        rep movsb
        mov     esi, [ebp+kernel_init_data.command_line]
        call    parse_cmdline
        and     dword [edx], 0 ; argv[argc] = NULL
        and     [.envp], 0
        mov     eax, 68
        mov     ebx, 13
        mov     ecx, ebp
        call    FS_SYSCALL_PTR
; 3. Configure modules: main EXE and possible statically linked DLLs.
        mov     esi, [exe_base]
        mov     eax, [.argv]
        pushd   [eax]
        call    fixup_pe_relocations
        pop     ecx
        jc      .die
; 4. Call exe entry point.
        mov     edx, [esi+STRIPPED_PE_HEADER.AddressOfEntryPoint]
        cmp     word [esi], 'MZ'
        jnz     @f
        mov     ecx, [esi+IMAGE_DOS_HEADER.e_lfanew]
        add     ecx, esi
        mov     edx, [ecx+IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint]
@@:
        add     edx, esi
        add     esp, fpo_localsize+4
        call    edx
; If exe entry point has returned control, die.
        jmp     .die
.version_mismatch:
        lea     eax, [esi + rva syscall_int40]
        mov     FS_SYSCALL_PTR, eax
        add     esi, rva msg_version_mismatch
        call    sys_msg_board_str
.die:
        or      eax, -1
        call    FS_SYSCALL_PTR
.nothing:
        ret
endp

proc sys_msg_board_str
        push    eax ebx
@@:
        push    ecx
        mov     cl, [ecx]
        test    cl, cl
        jz      @f
        mov     eax, 63
        mov     ebx, 1
        call    FS_SYSCALL_PTR
        pop     ecx
        inc     ecx
        jmp     @b
@@:
        pop     ecx ebx eax
        ret
endp

align 4
syscall_methods dd rva syscall_int40, rva syscall_sysenter, rva syscall_syscall
num_syscall_methods = ($ - syscall_methods) / 4

align 4
data export
export 'kolibri.dll' \
        , kercall, 'kercall' \
        , malloc, 'malloc' \
        , free, 'free' \
        , calloc, 'calloc' \
        , realloc, 'realloc' \
        , realloc_in_place, 'realloc_in_place' \
        , memalign, 'memalign' \
        , create_mspace, 'create_mspace' \
        , destroy_mspace, 'destroy_mspace' \
        , mspace_malloc, 'mspace_malloc' \
        , mspace_free, 'mspace_free' \
        , mspace_calloc, 'mspace_calloc' \
        , mspace_realloc, 'mspace_realloc' \
        , mspace_realloc_in_place, 'mspace_realloc_in_place' \
        , mspace_memalign, 'mspace_memalign' \

end data

kolibri_dll             db      'kolibri.dll',0

msg_version_mismatch    db      'S : Version mismatch between kernel and kolibri.dll',13,10,0
msg_bad_relocation1     db      'S : Bad relocation type in ',0
msg_newline             db      13,10,0
msg_relocated1          db      'S : fixups for ',0
msg_relocated2          db      ' applied',13,10,0

if FOOTERS
section '.data' data readable writable
malloc_magic    dd      ?
end if
