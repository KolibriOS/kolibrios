; System allocator.
; Based on dlmalloc 2.8.6.
; dlmalloc is written by Doug Lea and released to the public domain.

; Algorithms are the same as in dlmalloc, with the following differences:
; * segment management uses large segments,
;   since segments can never be merged;
; * top chunk is usually large, so the code tries mmap
;   for chunks with size >= mmap_threshold before allocating from top;
; * there is additional bookkeeping for releasing physical memory
;   instead of relying on unmapping entire segments:
;   tree chunks have additional field in the end,
;   all recently expanded tree chunks are linked in one list for sys_trim;
; * there is an additional list of all mmapped chunks,
;   so that mspace_destroy can free everything, including mmapped chunks;
; * realloc and memalign can give back a space before a free chunk
;   (extending that chunk) even if a space is less than minimal chunk size.

; Statistics:
;   Alignment: 8 bytes
;   Minimum overhead per allocated chunk: 4 or 8 bytes,
;     depending on whether FOOTERS is defined.
;   Minimum allocated size: 16 bytes (including overhead)
; See details at http://gee.cs.oswego.edu/dl/html/malloc.html.

; The KolibriOS kernel provides functions similar to mmap/mremap/munmap,
; they are used as base for allocations.

FOOTERS = 0
;  If true, provide extra checking and dispatching by placing
;  information in the footers of allocated chunks. This adds
;  space and time overhead, but can be useful for debugging.

DEFAULT_MMAP_THRESHOLD = 256*1024
;  The request size threshold for using MMAP to directly service a
;  request. Requests of at least this size that cannot be allocated
;  using already-existing space will be serviced via mmap.  (If enough
;  normal freed space already exists it is used instead.)  Using mmap
;  segregates relatively large chunks of memory so that they can be
;  individually obtained and released from the host system. A request
;  serviced through mmap is never reused by any other request (at least
;  not directly; the system may just so happen to remap successive
;  requests to the same locations).  Segregating space in this way has
;  the benefits that: Mmapped space can always be individually released
;  back to the system, which helps keep the system level memory demands
;  of a long-lived program low.  Also, mapped memory doesn't become
;  `locked' between other chunks, as can happen with normally allocated
;  chunks, which means that even trimming via malloc_trim would not
;  release them.  However, it has the disadvantage that the space
;  cannot be reclaimed, consolidated, and then used to service later
;  requests, as happens with normal chunks.  The advantages of mmap
;  nearly always outweigh disadvantages for "large" chunks, but the
;  value of "large" may vary across systems.  The default is an
;  empirically derived value that works well in most systems. You can
;  disable mmap by setting to 0xFFFFFFFF.

RELEASE_CHECK_RATE = 64
;  The number of consolidated frees between checks to release
;  unused segments when freeing. When using non-contiguous segments,
;  especially with multiple mspaces, checking only for topmost space
;  doesn't always suffice to trigger trimming. To compensate for this,
;  free() will, with a period of MAX_RELEASE_CHECK_RATE (or the
;  current number of segments, if greater) try to release unused
;  segments to the OS when freeing chunks that result in
;  consolidation. The best value for this parameter is a compromise
;  between slowing down frees with relatively costly checks that
;  rarely trigger versus holding on to unused memory. To effectively
;  disable, set to MAX_SIZE_T. This may lead to a very slight speed
;  improvement at the expense of carrying around more memory.

DEFAULT_MSPACE_SIZE = 1024*1024

include 'malloc_internal.inc'

prologue@proc equ fpo_prologue
epilogue@proc equ fpo_epilogue

;  void* create_mspace(size_t capacity, int locked)
;  create_mspace creates and returns a new independent space with the
;  given initial capacity, or, if 0, the default mspace size.  It
;  returns null if there is no system memory available to create the
;  space.  If argument locked is non-zero, the space uses a separate
;  lock to control access. The capacity of the space will grow
;  dynamically as needed to service mspace_malloc requests.
proc create_mspace stdcall uses ebx, capacity, locked
        do_create_mspace
endp

;  void destroy_mspace(mspace msp)
;  destroy_mspace destroys the given space, and attempts to return all
;  of its memory back to the system, returning the total number of
;  bytes freed. After destruction, the results of access to all memory
;  used by the space become undefined.
proc destroy_mspace stdcall uses ebx, msp
        do_destroy_mspace
endp


macro set_default_heap
{
        mov     ebp, [default_heap]
.got_mspace:
}

macro set_explicit_heap
{
        mov     ebp, [msp]
}

macro mspace_adapter common_label
{
        mov     eax, [esp]
        mov     [esp], ebp
        mov     ebp, [esp+4]
        mov     [esp+4], eax
        push    ebx
        push    esi
        jmp     common_label
}

;  void* malloc(size_t bytes)
;  Returns a pointer to a newly allocated chunk of at least n bytes, or
;  null if no space is available, in which case errno is set to ENOMEM
;  on ANSI C systems.
;
;  If n is zero, malloc returns a minimum-sized chunk. (The minimum
;  size is 16 bytes on most 32bit systems, and 32 bytes on 64bit
;  systems.)  Note that size_t is an unsigned type, so calls with
;  arguments that would be negative if signed are interpreted as
;  requests for huge amounts of space, which will often fail. The
;  maximum supported value of n differs across systems, but is in all
;  cases less than the maximum representable value of a size_t.
align 16
proc malloc stdcall uses ebp ebx esi, bytes
        set_default_heap
        do_malloc
endp

;  void free(void* mem)
;  Releases the chunk of memory pointed to by mem, that had been previously
;  allocated using malloc or a related routine such as realloc.
;  It has no effect if mem is null. If mem was not malloced or already
;  freed, free(mem) will by default cause the current program to abort.
align 16
proc free stdcall uses ebp ebx esi, mem
        set_default_heap
        do_free
endp

;  void* calloc(size_t n_elements, size_t elem_size);
;  Returns a pointer to n_elements * elem_size bytes, with all locations
;  set to zero.
align 16
proc calloc stdcall, n_elements, elem_size
        do_calloc <stdcall malloc,eax>
endp

;  void* realloc(void* oldmem, size_t bytes)
;  Returns a pointer to a chunk of size bytes that contains the same data
;  as does chunk oldmem up to the minimum of (bytes, oldmem's size) bytes, or null
;  if no space is available.
;
;  The returned pointer may or may not be the same as oldmem. The algorithm
;  prefers extending oldmem in most cases when possible, otherwise it
;  employs the equivalent of a malloc-copy-free sequence.
;
;  If oldmem is null, realloc is equivalent to malloc.
;
;  If space is not available, realloc returns null, errno is set (if on
;  ANSI) and oldmem is NOT freed.
;
;  if bytes is for fewer bytes than already held by oldmem, the newly unused
;  space is lopped off and freed if possible.  realloc with a size
;  argument of zero (re)allocates a minimum-sized chunk.
;
;  The old unix realloc convention of allowing the last-free'd chunk
;  to be used as an argument to realloc is not supported.
align 16
proc realloc stdcall uses ebp ebx esi, oldmem, bytes
        set_default_heap
if used mspace_realloc
        do_realloc <stdcall mspace_malloc,ebp,>, <stdcall mspace_free,ebp,>
else
        do_realloc <stdcall malloc,>, <stdcall free,>
end if
endp

;  void* realloc_in_place(void* oldmem, size_t bytes)
;  Resizes the space allocated for oldmem to size bytes, only if this can be
;  done without moving oldmem (i.e., only if there is adjacent space
;  available if bytes is greater than oldmem's current allocated size, or bytes is
;  less than or equal to oldmem's size). This may be used instead of plain
;  realloc if an alternative allocation strategy is needed upon failure
;  to expand space; for example, reallocation of a buffer that must be
;  memory-aligned or cleared. You can use realloc_in_place to trigger
;  these alternatives only when needed.
;
;  Returns oldmem if successful; otherwise null.
align 16
proc realloc_in_place stdcall uses ebp ebx esi, oldmem, bytes
        set_default_heap
        do_realloc_in_place
endp

;  void* memalign(size_t alignment, size_t bytes);
;  Returns a pointer to a newly allocated chunk of bytes argument, aligned
;  in accord with the alignment argument.
;
;  The alignment argument should be a power of two. If the argument is
;  not a power of two, the nearest greater power is used.
;  8-byte alignment is guaranteed by normal malloc calls, so don't
;  bother calling memalign with an argument of 8 or less.
;
;  Overreliance on memalign is a sure way to fragment space.
align 16
proc memalign stdcall uses ebp ebx esi, alignment, bytes
        set_default_heap
if used mspace_memalign
        do_memalign <stdcall mspace_malloc,ebp,>
else
        do_memalign <stdcall malloc,>
end if
endp

;  void* mspace_malloc(mspace msp, size_t bytes)
;  mspace_malloc behaves as malloc, but operates within
;  the given space.
align 16
proc mspace_malloc ;stdcall uses ebp ebx esi, msp, bytes
;       set_explicit_heap
;       do_malloc
        mspace_adapter malloc.got_mspace
endp

;  void mspace_free(mspace msp, void* mem)
;  mspace_free behaves as free, but operates within
;  the given space.
align 16
proc mspace_free ;stdcall uses ebp ebx esi, msp, mem
;       set_explicit_heap
;       do_free
        mspace_adapter free.got_mspace
endp

;  void* mspace_calloc(mspace msp, size_t n_elements, size_t elem_size)
;  mspace_calloc behaves as calloc, but operates within
;  the given space.
align 16
proc mspace_calloc stdcall, msp, n_elements, elem_size
        do_calloc <stdcall mspace_malloc,[msp+4],eax>
endp

;  void* mspace_realloc(mspace msp, void* oldmem, size_t bytes)
;  mspace_realloc behaves as realloc, but operates within
;  the given space.
align 16
proc mspace_realloc ;stdcall uses ebp ebx esi, msp, oldmem, bytes
;       set_explicit_heap
;       do_realloc <stdcall mspace_malloc,ebp,>, <stdcall mspace_free,ebp,>
        mspace_adapter realloc.got_mspace
endp

;  void* mspace_realloc_in_place(mspace msp, void* oldmem, size_t bytes)
align 16
proc mspace_realloc_in_place ;stdcall uses ebp ebx esi, msp, oldmem, bytes
;       set_explicit_heap
;       do_realloc_in_place
        mspace_adapter realloc_in_place.got_mspace
endp

;  void* mspace_memalign(mspace msp, size_t alignment, size_t bytes)
;  mspace_memalign behaves as memalign, but operates within
;  the given space.
align 16
proc mspace_memalign ;stdcall uses ebp ebx esi, msp, alignment, bytes
;       set_explicit_heap
;       do_memalign <stdcall mspace_malloc,ebp,>
        mspace_adapter memalign.got_mspace
endp

assert MALLOC_ALIGNMENT >= 8
assert MALLOC_ALIGNMENT and (MALLOC_ALIGNMENT - 1) = 0
assert MCHUNK_SIZE and (MCHUNK_SIZE - 1) = 0
; in: edx = initial size of the process heap
macro malloc_init
{
if FOOTERS
        mov     eax, 26
        mov     ebx, 9
        call    FS_SYSCALL_PTR
        xor     eax, 0x55555555
        or      eax, 8
        and     eax, not 7
        mov     [malloc_magic], eax
end if
        stdcall create_mspace, edx, 1
        mov     [default_heap], eax
}

proc heap_corrupted
        sub     esp, 400h
        mov     eax, 9
        mov     ebx, esp
        or      ecx, -1
        call    FS_SYSCALL_PTR
        lea     esi, [ebx+10]
        lea     edx, [ebx+10+11]
        mov     eax, 63
        mov     ebx, 1
        mov     cl, '['
        call    FS_SYSCALL_PTR
@@:
        mov     cl, [esi]
        test    cl, cl
        jz      @f
        call    FS_SYSCALL_PTR
        inc     esi
        cmp     esi, edx
        jb      @b
@@:
        mov     esi, heap_corrupted_msg
@@:
        mov     cl, [esi]
        inc     esi
        test    cl, cl
        jz      @f
        mov     eax, 63
        mov     ebx, 1
        call    FS_SYSCALL_PTR
        jmp     @b
@@:
        or      eax, -1
        or      ebx, -1
        call    FS_SYSCALL_PTR
endp

heap_corrupted_msg      db      '] Heap corrupted, aborting',13,10,0
