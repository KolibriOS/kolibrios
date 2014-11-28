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
ends

include 'malloc.inc'

proc syscall_int40
        int     0x40
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
; 2. Validate version of the init struct.
; If not known, say a debug message and die.
        mov     ebp, [reserved]
        cmp     [ebp+kernel_init_data.version], 1
        jnz     .version_mismatch
; 3. Setup common data based on the init struct.
        mov     eax, [ebp+kernel_init_data.stack_base]
        mov     FS_STACK_MIN, eax
        add     eax, [ebp+kernel_init_data.stack_size]
        mov     FS_STACK_MAX, eax
        mov     eax, [ebp+kernel_init_data.syscall_method]
        cmp     eax, 0x10000
        jae     @f
        mov     eax, syscall_int40
@@:
        mov     FS_SYSCALL_PTR, eax
; 4. Initialize the process heap.
        mov     eax, [ebp+kernel_init_data.exe_base]
        mov     edx, [eax+STRIPPED_PE_HEADER.SizeOfHeapReserve]
        cmp     word [eax], 'MZ'
        jnz     @f
        add     eax, [eax+IMAGE_DOS_HEADER.e_lfanew]
        mov     edx, [eax+IMAGE_NT_HEADERS.OptionalHeader.SizeOfHeapReserve]
@@:
        malloc_init
; ...TBD...
; Call exe entry point.
        mov     eax, [ebp+kernel_init_data.exe_base]
        mov     edx, [eax+STRIPPED_PE_HEADER.AddressOfEntryPoint]
        cmp     word [eax], 'MZ'
        jnz     @f
        mov     ecx, [eax+IMAGE_DOS_HEADER.e_lfanew]
        add     ecx, eax
        mov     edx, [ecx+IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint]
@@:
        add     edx, eax
        call    edx
; If exe entry point has returned control, die.
        mov     eax, -1
        call    FS_SYSCALL_PTR
.version_mismatch:
        mov     esi, version_mismatch_msg
        mov     eax, 63
        mov     ebx, 1
@@:
        mov     cl, [esi]
        test    cl, cl
        jz      @f
        int     0x40    ; can't use FS_SYSCALL_PTR here, it has not yet been set
        inc     esi
        jmp     @b
@@:
        mov     eax, -1
        int     0x40
.nothing:
        ret
endp

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

version_mismatch_msg    db      'Version mismatch between kernel and kolibri.dll',13,10,0

if FOOTERS
section '.data' data readable writable
malloc_magic    dd      ?
end if
