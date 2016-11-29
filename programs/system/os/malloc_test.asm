format PE console 4.0
entry start

include 'win32a.inc'
include '../../struct.inc'
include '../../proc32.inc'
include 'fpo.inc'

FS_ERRNO equ dword [errno]
ENOMEM = 12
include 'malloc.inc'

start:
        cinvoke fopen, logfile_name, logfile_mode
        mov     [logfile], eax
        mov     edx, 1 ;shl 25
        malloc_init
        call    run_test
        set_default_heap
        stdcall destroy_mspace, ebp
        cinvoke fclose, [logfile]
        ret

FS_SYSCALL_PTR:
        cmp     eax, 68
        jnz     unknown_syscall
        cmp     ebx, 12
        jz      syscall_malloc
        cmp     ebx, 13
        jz      syscall_free
        cmp     ebx, 20
        jz      syscall_realloc
        cmp     ebx, 26
        jz      syscall_trim

unknown_syscall:
        int3
        jmp     $

syscall_malloc:
        push    ecx edx
        invoke  VirtualAlloc, 0, ecx, MEM_COMMIT, PAGE_READWRITE
        pop     edx ecx
        ret
syscall_free:
        push    ecx edx
        invoke  VirtualFree, ecx, 0, MEM_RELEASE
        test    eax, eax
        jz      @f
        pop     edx ecx
        ret
@@:
        int3
        jmp     $
syscall_realloc:
        push    esi edi
        push    ecx edx
        mov     esi, edx
        call    syscall_malloc
        mov     edi, eax
        sub     esp, 1Ch
        mov     edx, esp
        invoke  VirtualQuery, esi, edx, 1Ch
        mov     ecx, [esp+0Ch]
        add     esp, 1Ch
        cmp     ecx, [esp+4]
        jb      @f
        mov     ecx, [esp+4]
@@:
        shr     ecx, 2
        push    esi edi
        rep movsd
        pop     edi ecx
        call    syscall_free
        mov     eax, edi
        pop     edx ecx
        pop     edi esi
        ret
syscall_trim:
        push    eax ecx edi
        lea     edi, [ecx+edx]
        mov     ecx, esi
        shr     ecx, 2
        xor     eax, eax
        rep stosd
        pop     edi ecx eax
        ret

macro next_random
{
        imul    edi, 1103515245
        add     edi, 12345
}

macro call_and_check_regs what
{
        push    ebx edi
        what
        cmp     edi, [esp]
        jnz     edi_destroyed
        cmp     ebx, [esp+4]
        jnz     ebx_destroyed
        add     esp, 8
}

get_malloc_size:
        and     eax, 1023
        jnz     @f
        next_random
        mov     eax, edi
        shr     eax, 16
        shl     eax, 8
@@:
        ret

get_and_validate_memory:
        xor     edx, edx
        div     esi
        mov     eax, [esp+edx*8+4]
        mov     ecx, [esp+edx*8+8]
        push    edi eax
        mov     edi, eax
        mov     al, [edi]
        repz scasb
        jnz     memory_destroyed
        pop     ecx edi
        ret

run_test:
; 65536 times run random operation.
; Randomly select malloc(random size from 1 to 1023 or from 256 to 16M),
; free(random of previously allocated areas),
; realloc(random of previously allocated areas, random size from 1 to 1023 or from 256 to 16M),
; realloc_in_place(<same as realloc>),
; memalign(random size from 1 to 1023 or from 256 to 16M, random power of 2 from 8 to 1024)
        mov     edi, 0x12345678
        xor     esi, esi        ; 0 areas allocated
        mov     ebx, 65536
.loop:
;       call    validate_release_chain
        next_random
        mov     eax, edi
        shr     eax, 16
        mov     ecx, eax
        shr     eax, 3
        and     ecx, 7
        jz      .memalign
        dec     ecx
        jz      .realloc_in_place
        dec     ecx
        jz      .realloc
        test    ebx, 64
        jz      .prefer_free
.prefer_malloc:
        dec     ecx
        jz      .free
        jmp     .malloc
.prefer_free:
        dec     ecx
        jnz     .free
.malloc:
        call    get_malloc_size
        jz      .loop
        push    eax
        call_and_check_regs <stdcall malloc,eax>
        pop     ecx
        pushad
        cinvoke fprintf, [logfile], malloc_str, ecx, eax
        popad
        test    eax, eax
        jz      generic_malloc_failure
        inc     esi
        push    ecx eax
        push    edi
        mov     edi, eax
        mov     eax, esi
        rep stosb
        pop     edi
        jmp     .common
.free:
        test    esi, esi
        jz      .loop
        call    get_and_validate_memory
        push    edx
        pushad
        cinvoke fprintf, [logfile], free_str, ecx
        popad
        call_and_check_regs <stdcall free,ecx>
;        call   validate_release_chain
        pop     edx
        dec     esi
        pop     eax ecx
        push    edi
        lea     edi, [esp+4]
@@:
        dec     edx
        js      @f
        xchg    eax, [edi]
        xchg    ecx, [edi+4]
        add     edi, 8
        jmp     @b
@@:
        pop     edi
        jmp     .common
.realloc:
        test    esi, esi
        jz      .loop
        call    get_and_validate_memory
        push    eax
        next_random
        mov     eax, edi
        shr     eax, 16
        call    get_malloc_size
        jnz     @f
        pop     eax
        jmp     .loop
@@:
        push    eax edx
        pushad
        cinvoke fprintf, [logfile], realloc_str1, ecx, eax
        popad
        call_and_check_regs <stdcall realloc,ecx,eax>
        pop     edx ecx
        pushad
        cinvoke fprintf, [logfile], realloc_str2, eax
        popad
        test    eax, eax
        jz      generic_malloc_failure
        push    ebx edi ecx
        mov     ebx, [esp+edx*8+20]
        mov     [esp+edx*8+16], eax
        mov     [esp+edx*8+20], ecx
        cmp     ebx, ecx
        jae     @f
        mov     ecx, ebx
@@:
        mov     edi, eax
        mov     eax, [esp+12]
        repz scasb
        jnz     memory_destroyed
        pop     ecx
        sub     ecx, ebx
        jbe     @f
        rep stosb
@@:
        pop     edi ebx eax
        jmp     .common
.realloc_in_place:
        test    esi, esi
        jz      .loop
        call    get_and_validate_memory
        push    eax
        next_random
        mov     eax, edi
        shr     eax, 16
        call    get_malloc_size
        jnz     @f
        pop     eax
        jmp     .loop
@@:
        push    eax edx
        pushad
        cinvoke fprintf, [logfile], realloc_in_place_str1, ecx, eax
        popad
        call_and_check_regs <stdcall realloc_in_place,ecx,eax>
        pushad
        cinvoke fprintf, [logfile], realloc_in_place_str2, eax
        popad
        pop     edx ecx
        test    eax, eax
        jnz     @f
        pop     eax
        jmp     .common
@@:
        cmp     [esp+edx*8+4], eax
        jnz     generic_malloc_failure
        push    ebx edi ecx
        mov     ebx, [esp+edx*8+20]
        mov     [esp+edx*8+20], ecx
        cmp     ebx, ecx
        jae     @f
        mov     ecx, ebx
@@:
        mov     edi, eax
        mov     eax, [esp+12]
        repz scasb
        jnz     memory_destroyed
        pop     ecx
        sub     ecx, ebx
        jbe     @f
        rep stosb
@@:
        pop     edi ebx eax
        jmp     .common
.memalign:
        call    get_malloc_size
        jz      .loop
        next_random
        mov     ecx, edi
        shr     ecx, 29
        mov     edx, 8
        shl     edx, cl
        push    eax edx
        pushad
        cinvoke fprintf, [logfile], memalign_str1, edx, eax
        popad
        call_and_check_regs <stdcall memalign, edx, eax>
        pushad
        cinvoke fprintf, [logfile], memalign_str2, eax
        popad
        dec     dword [esp]
        test    eax, [esp]
        jnz     memalign_invalid
        add     esp, 4
        pop     ecx
        test    eax, eax
        jz      generic_malloc_failure
        inc     esi
        push    ecx eax
        push    edi
        mov     edi, eax
        mov     eax, esi
        rep stosb
        pop     edi
.common:
        cinvoke fflush, [logfile]
        dec     ebx
        jnz     .loop
@@:
        dec     esi
        js      @f
        pop     eax ecx
        stdcall free, eax
        jmp     @b
@@:
        ret

generic_malloc_failure:
        mov     eax, 1
        int3
        jmp     $

memory_destroyed:
        mov     eax, 2
        int3
        jmp     $

edi_destroyed:
        mov     eax, 3
        int3
        jmp     $

ebx_destroyed:
        mov     eax, 4
        int3
        jmp     $

memalign_invalid:
        mov     eax, 5
        int3
        jmp     $

validate_release_chain:
        push    ebx ebp
        set_default_heap
        lea     ecx, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     eax, ecx
        mov     edx, [ecx+tchunk_release_fd]
@@:
        cmp     [edx+tchunk_release_bk], eax
        jnz     .fail
        cmp     edx, ecx
        jz      @f
        mov     eax, edx
        mov     edx, [edx+tchunk_release_fd]
        jmp     @b
@@:
        lea     eax, [ebp-3]
        add     eax, [ebp-4]
        cmp     eax, [ebp+malloc_state.top]
        jz      .ok
.chunk_loop:
        mov     ecx, [eax-4]
        test    ecx, CINUSE_BIT
        jnz     .next_chunk
        cmp     ecx, 0x100
        jb      .next_chunk
        mov     edx, ecx
        and     edx, not FLAG_BITS
        lea     edx, [eax+edx]
        cmp     [edx+tchunk_release_fd], edx
        jnz     @f
        cmp     [edx+tchunk_release_bk], edx
        jnz     .fail
        jmp     .next_chunk
@@:
        mov     ebx, [ebp+malloc_state.release_list]
@@:
        cmp     edx, ebx
        jz      .next_chunk
        mov     ebx, [ebx+tchunk_release_fd]
        cmp     ebx, [ebp+malloc_state.release_list]
        jnz     @b
        jmp     .fail
.next_chunk:
        and     ecx, not FLAG_BITS
        add     eax, ecx
        cmp     eax, [ebp+malloc_state.top]
        jb      .chunk_loop
        ja      .fail
.ok:
        pop     ebp ebx
        ret
.fail:
        int3
        jmp     $

align 4
data import
library kernel32,'kernel32.dll',msvcrt,'msvcrt.dll'
import kernel32,\
        VirtualAlloc, 'VirtualAlloc', \
        VirtualFree, 'VirtualFree', \
        VirtualQuery, 'VirtualQuery'
import msvcrt,\
        fopen,'fopen',\
        fclose,'fclose',\
        fprintf,'fprintf',\
        fflush,'fflush'
end data

malloc_str      db      'malloc(0x%X) = 0x%X',10,0
free_str        db      'free(0x%X)',10,0
realloc_str1    db      'realloc(0x%X,0x%X)',0
realloc_str2    db      ' = 0x%X',10,0
realloc_in_place_str1   db      'realloc_in_place(0x%X,0x%X)',0
realloc_in_place_str2   db      ' = 0x%X',10,0
memalign_str1   db      'memalign(0x%X,0x%X)',0
memalign_str2   db      ' = 0x%X',10,0

logfile_name    db      'test.log',0
logfile_mode    db      'w',0

align 4
logfile dd      ?
errno   dd      ?
default_heap    dd      ?
process_data    rd      1024
