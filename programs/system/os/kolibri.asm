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

include 'sync.inc'
include 'malloc.inc'
include 'peloader.inc'
include 'modules.inc'
include 'cmdline.inc'
include 'thread.inc'

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
; 2d. Initialize process heap.
        mov     eax, [ebp+kernel_init_data.exe_base]
        mov     edx, [eax+STRIPPED_PE_HEADER.SizeOfHeapReserve]
        cmp     word [eax], 'MZ'
        jnz     @f
        add     eax, [eax+IMAGE_DOS_HEADER.e_lfanew]
        mov     edx, [eax+IMAGE_NT_HEADERS.OptionalHeader.SizeOfHeapReserve]
@@:
        malloc_init
; 2e. Allocate and fill MODULE structs for main exe and kolibri.dll.
        mov     eax, [ebp+kernel_init_data.exe_path]
@@:
        inc     eax
        cmp     byte [eax-1], 0
        jnz     @b
        sub     eax, [ebp+kernel_init_data.exe_path]
        push    eax
        add     eax, sizeof.MODULE
        stdcall malloc, eax
        test    eax, eax
        jz      .die
        mov     ebx, eax
        stdcall malloc, sizeof.MODULE + kolibri_dll.size
        test    eax, eax
        jz      .die
        mov     edx, modules_list
        mov     [edx+MODULE.next], ebx
        mov     [ebx+MODULE.next], eax
        mov     [eax+MODULE.next], edx
        mov     [edx+MODULE.prev], eax
        mov     [eax+MODULE.prev], ebx
        mov     [ebx+MODULE.prev], edx
        push    esi
        mov     esi, kolibri_dll
        mov     ecx, kolibri_dll.size
        lea     edi, [eax+MODULE.path]
        rep movsb
        pop     esi
        call    init_module_struct
        mov     eax, ebx
        mov     esi, [ebp+kernel_init_data.exe_path]
        pop     ecx
        lea     edi, [ebx+MODULE.path]
        rep movsb
        mov     esi, [ebp+kernel_init_data.exe_base]
        call    init_module_struct
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
        stdcall malloc, esi
        test    eax, eax
        jz      .die
        mov     [.argv], eax
        mov     edx, eax
        lea     edi, [eax+(ebx+1)*4]
        mov     eax, [modules_list + MODULE.next]
        add     eax, MODULE.path
        mov     [edx], eax
        add     edx, 4
        mov     esi, [ebp+kernel_init_data.command_line]
        call    parse_cmdline
        and     dword [edx], 0 ; argv[argc] = NULL
        and     [.envp], 0
        mov     eax, 68
        mov     ebx, 13
        mov     ecx, ebp
        call    FS_SYSCALL_PTR
; 2g. Initialize mutex for list of MODULEs.
        mov     ecx, modules_mutex
        call    mutex_init
; 2h. For console applications, call console.dll!con_init with default parameters.
        mov     eax, [modules_list + MODULE.next]
        mov     esi, [eax+MODULE.base]
        mov     al, [esi+STRIPPED_PE_HEADER.Subsystem]
        cmp     byte [esi], 'M'
        jnz     @f
        mov     eax, [esi+3Ch]
        mov     al, byte [esi+eax+IMAGE_NT_HEADERS.OptionalHeader.Subsystem]
@@:
        cmp     al, IMAGE_SUBSYSTEM_WINDOWS_CUI
        jnz     .noconsole
        stdcall dlopen, console_dll, 0
        test    eax, eax
        jz      .noconsole
        stdcall dlsym, eax, con_init_str
        test    eax, eax
        jz      .noconsole
        mov     edx, [modules_list + MODULE.next]
        stdcall eax, -1, -1, -1, -1, [edx+MODULE.filename]
.noconsole:
; 3. Configure modules: main EXE and possible statically linked DLLs.
        mov     eax, [modules_list + MODULE.next]
        mov     esi, [eax+MODULE.base]
        add     eax, MODULE.path
        push    eax
        call    fixup_pe_relocations
        pop     ecx
        jc      .die
        mutex_lock modules_mutex
        mov     esi, [modules_list + MODULE.next]
        call    resolve_pe_imports
        mov     ebx, eax
        mutex_unlock modules_mutex
        test    ebx, ebx
        jnz     .die
; 4. Call exe entry point.
        mov     esi, [esi+MODULE.base]
        mov     edx, [esi+STRIPPED_PE_HEADER.AddressOfEntryPoint]
        cmp     byte [esi], 'M'
        jnz     @f
        mov     ecx, [esi+IMAGE_DOS_HEADER.e_lfanew]
        add     ecx, esi
        mov     edx, [ecx+IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint]
@@:
        add     edx, esi
        pop     ecx
        mov     [process_initialized], 1
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
        , dlopen, 'dlopen' \
        , dlclose, 'dlclose' \
        , dlsym, 'dlsym' \
        , create_thread, 'create_thread' \
        , exit_thread, 'exit_thread' \

end data

kolibri_dll             db      '/rd/1/lib/kolibri.dll',0
.size = $ - kolibri_dll

console_dll             db      'console.dll',0
con_init_str            db      'con_init',0

msg_version_mismatch    db      'S : Version mismatch between kernel and kolibri.dll',13,10,0
msg_bad_relocation      db      'Bad relocation type in ',0
msg_newline             db      13,10,0
msg_relocated1          db      'S : fixups for ',0
msg_relocated2          db      ' applied',13,10,0
msg_noreloc1            db      'Module ',0
msg_noreloc2            db      ' is not at preferred base and has no fixups',0
loader_debugboard_prefix db     'S : ',0
notify_program          db      '/rd/1/@notify',0
msg_cannot_open         db      'Cannot open ',0
msg_paths_begin         db      ' in any of '

module_path1    db      '/rd/1/lib/'
.size = $ - module_path1
                        db      ', '
module_path2    db      '/kolibrios/lib/'
.size = $ - module_path2
                        db      ', ',0
msg_export_name_not_found       db      'Exported function ',0
msg_export_ordinal_not_found    db      'Exported ordinal #',0
msg_export_not_found    db      ' not found in module ',0
msg_unknown             db      '<unknown>',0
msg_invalid_forwarder   db      'Invalid forwarded export in module ',0

section '.data' data readable writable
if FOOTERS
malloc_magic    dd      ?
end if
default_heap    dd      ?
modules_list    rd      2
modules_mutex   MUTEX
process_initialized     db      ?
