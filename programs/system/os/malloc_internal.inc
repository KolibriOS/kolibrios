MALLOC_ALIGNMENT = 8    ; must be power of two not less than 8, not greater than 4096
CHUNK_ALIGN_MASK = MALLOC_ALIGNMENT - 1

; -----------------------  Chunk representations ------------------------
;
;  (The following includes lightly edited explanations by Colin Plumb.)
;
;  The malloc_chunk declaration below is misleading (but accurate and
;  necessary).  It declares a "view" into memory allowing access to
;  necessary fields at known offsets from a given base.
;
;  Chunks of memory are maintained using a `boundary tag' method as
;  originally described by Knuth.  (See the paper by Paul Wilson
;  ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps for a survey of such
;  techniques.)  Sizes of free chunks are stored both in the front of
;  each chunk and at the end.  This makes consolidating fragmented
;  chunks into bigger chunks fast.  The head fields also hold bits
;  representing whether chunks are free or in use.
;
;  Here are some pictures to make it clearer.  They are "exploded" to
;  show that the state of a chunk can be thought of as extending from
;  the high 31 bits of the head field of its header through the
;  prev_foot and PINUSE_BIT bit of the following chunk header.
;
;  A chunk that's in use looks like:
;
;   chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;           | Size of previous chunk (if P = 0)                             |
;           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |P|
;         | Size of this chunk                                         1| +-+
;   mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;         |                                                               |
;         +-                                                             -+
;         |                                                               |
;         +-                                                             -+
;         |                                                               :
;         +-      size - sizeof(size_t) available payload bytes          -+
;         :                                                               |
; chunk-> +-                                                             -+
;         |                                                               |
;         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |1|
;       | Size of next chunk (may or may not be in use)               | +-+
; mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;
;    And if it's free, it looks like this:
;
;   chunk-> +-                                                             -+
;           | User payload (must be in use, or we would have merged!)       |
;           +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |P|
;         | Size of this chunk                                         0| +-+
;   mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;         | Next pointer                                                  |
;         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;         | Prev pointer                                                  |
;         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;         |                                                               :
;         +-      size - sizeof(struct chunk) unused bytes               -+
;         :                                                               |
; chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;         | Size of this chunk                                            |
;         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ |0|
;       | Size of next chunk (must be in use, or we would have merged)| +-+
; mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;       |                                                               :
;       +- User payload                                                -+
;       :                                                               |
;       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;                                                                     |0|
;                                                                     +-+
;  Note that since we always merge adjacent free chunks, the chunks
;  adjacent to a free chunk must be in use.
;
;  Given a pointer to a chunk (which can be derived trivially from the
;  payload pointer) we can, in O(1) time, find out whether the adjacent
;  chunks are free, and if so, unlink them from the lists that they
;  are on and merge them with the current chunk.
;
;  Chunks always begin on even word boundaries, so the mem portion
;  (which is returned to the user) is also on an even word boundary, and
;  thus at least double-word aligned.
;
;  The P (PINUSE_BIT) bit, stored in the unused low-order bit of the
;  chunk size (which is always a multiple of two words), is an in-use
;  bit for the *previous* chunk.  If that bit is *clear*, then the
;  word before the current chunk size contains the previous chunk
;  size, and can be used to find the front of the previous chunk.
;  The very first chunk allocated always has this bit set, preventing
;  access to non-existent (or non-owned) memory. If pinuse is set for
;  any given chunk, then you CANNOT determine the size of the
;  previous chunk, and might even get a memory addressing fault when
;  trying to do so.
;
;  The C (CINUSE_BIT) bit, stored in the unused second-lowest bit of
;  the chunk size redundantly records whether the current chunk is
;  inuse (unless the chunk is mmapped). This redundancy enables usage
;  checks within free and realloc, and reduces indirection when freeing
;  and consolidating chunks.
;
;  Each freshly allocated chunk must have both cinuse and pinuse set.
;  That is, each allocated chunk borders either a previously allocated
;  and still in-use chunk, or the base of its memory arena. This is
;  ensured by making all allocations from the `lowest' part of any
;  found chunk.  Further, no free chunk physically borders another one,
;  so each free chunk is known to be preceded and followed by either
;  inuse chunks or the ends of memory.
;
;  Note that the `foot' of the current chunk is actually represented
;  as the prev_foot of the NEXT chunk. This makes it easier to
;  deal with alignments etc but can be very confusing when trying
;  to extend or adapt this code.
;
;  The exceptions to all this are
;
;     1. The special chunk `top' is the top-most available chunk (i.e.,
;        the one bordering the end of available memory). It is treated
;        specially.  Top is never included in any bin, is used only if
;        no other chunk is available. In effect,
;        the top chunk is treated as larger (and thus less well
;        fitting) than any other available chunk.  The top chunk
;        doesn't update its trailing size field since there is no next
;        contiguous chunk that would have to index off it. However,
;        space is still allocated for it (TOP_FOOT_SIZE) to enable
;        separation or merging when space is extended.
;
;     2. Chunks allocated via mmap, have both cinuse and pinuse bits
;        cleared in their head fields.  Because they are allocated
;        one-by-one, each must carry its own prev_foot field, which is
;        also used to hold the offset this chunk has within its mmapped
;        region, which is needed to preserve alignment. Each mmapped
;        chunk is trailed by the first two fields of a fake next-chunk
;        for sake of usage checks.
;
struct malloc_chunk
prev_foot       dd      ?       ; size of previous chunk (if free)
head            dd      ?       ; size and inuse bits
data            rd      0       ; label for data (if busy)
fd              dd      ?       ; pointer to data in next malloc_chunk (if free)
bk              dd      ?       ; pointer to data in prev malloc_chunk (if free)
ends
mchunk_prev_foot = malloc_chunk.prev_foot - malloc_chunk.data
mchunk_head      = malloc_chunk.head      - malloc_chunk.data
mchunk_fd        = malloc_chunk.fd        - malloc_chunk.data
mchunk_bk        = malloc_chunk.bk        - malloc_chunk.data

; ------------------- Chunks sizes and alignments -----------------------
MCHUNK_SIZE = sizeof.malloc_chunk
if FOOTERS
CHUNK_OVERHEAD = 8
else
CHUNK_OVERHEAD = 4
end if
; MMapped chunks need a second word of overhead ...
MMAP_CHUNK_OVERHEAD = 8
; ... and additional padding for fake next-chunk at foot
MMAP_FOOT_PAD = 16
; The smallest size we can malloc is an aligned minimal chunk
MIN_CHUNK_SIZE = (MCHUNK_SIZE + CHUNK_ALIGN_MASK) and not CHUNK_ALIGN_MASK
; Bounds on request (not chunk) sizes.
MAX_REQUEST = (-MIN_CHUNK_SIZE) * 4
MIN_REQUEST = MIN_CHUNK_SIZE - CHUNK_OVERHEAD

; ------------------ Operations on head and foot fields -----------------
;  The head field of a chunk is or'ed with PINUSE_BIT when previous
;  adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
;  use, unless mmapped, in which case both bits are cleared.
;
;  FLAG4_BIT is not used by this malloc, but might be useful in extensions.
PINUSE_BIT = 1
CINUSE_BIT = 2
FLAG4_BIT = 4
INUSE_BITS = PINUSE_BIT + CINUSE_BIT
FLAG_BITS = PINUSE_BIT + CINUSE_BIT + FLAG4_BIT
; Head value for fenceposts
FENCEPOST_HEAD = 4 + INUSE_BITS

; ---------------------- Overlaid data structures -----------------------
;  When chunks are not in use, they are treated as nodes of either
;  lists or trees.
;
;  "Small"  chunks are stored in circular doubly-linked lists, and look
;  like this:
;
;    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Size of previous chunk                            |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;    `head:' |             Size of chunk, in bytes                         |P|
;      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Forward pointer to next chunk in list             |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Back pointer to previous chunk in list            |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Unused space (may be 0 bytes long)                .
;            .                                                               .
;            .                                                               |
;nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;    `foot:' |             Size of chunk, in bytes                           |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;
;  Larger chunks are kept in a form of bitwise digital trees (aka
;  tries) keyed on chunksizes.  Because malloc_tree_chunks are only for
;  free chunks greater than 256 bytes, their size doesn't impose any
;  constraints on user chunk sizes.  Each node looks like:
;
;    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Size of previous chunk                            |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;    `head:' |             Size of chunk, in bytes                         |P|
;      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Forward pointer to next chunk of same size        |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Back pointer to previous chunk of same size       |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Pointer to left child (child[0])                  |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Pointer to right child (child[1])                 |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Pointer to parent                                 |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             bin index of this chunk                           |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;            |             Unused space                                      .
;            .                                                               |
;nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;    `foot:' |             Size of chunk, in bytes                           |
;            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
;
;  Each tree holding treenodes is a tree of unique chunk sizes.  Chunks
;  of the same size are arranged in a circularly-linked list, with only
;  the oldest chunk (the next to be used, in our FIFO ordering)
;  actually in the tree.  (Tree members are distinguished by a non-null
;  parent pointer.)  If a chunk with the same size an an existing node
;  is inserted, it is linked off the existing node using pointers that
;  work in the same way as fd/bk pointers of small chunks.
;
;  Each tree contains a power of 2 sized range of chunk sizes (the
;  smallest is 0x100 <= x < 0x180), which is is divided in half at each
;  tree level, with the chunks in the smaller half of the range (0x100
;  <= x < 0x140 for the top nose) in the left subtree and the larger
;  half (0x140 <= x < 0x180) in the right subtree.  This is, of course,
;  done by inspecting individual bits.
;
;  Using these rules, each node's left subtree contains all smaller
;  sizes than its right subtree.  However, the node at the root of each
;  subtree has no particular ordering relationship to either.  (The
;  dividing line between the subtree sizes is based on trie relation.)
;  If we remove the last chunk of a given size from the interior of the
;  tree, we need to replace it with a leaf node.  The tree ordering
;  rules permit a node to be replaced by any leaf below it.
;
;  The smallest chunk in a tree (a common operation in a best-fit
;  allocator) can be found by walking a path to the leftmost leaf in
;  the tree.  Unlike a usual binary tree, where we follow left child
;  pointers until we reach a null, here we follow the right child
;  pointer any time the left one is null, until we reach a leaf with
;  both child pointers null. The smallest chunk in the tree will be
;  somewhere along that path.
;
;  The worst case number of steps to add, find, or remove a node is
;  bounded by the number of bits differentiating chunks within
;  bins. Under current bin calculations, this ranges from 6 up to 21
;  (for 32 bit sizes) or up to 53 (for 64 bit sizes). The typical case
;  is of course much better.
;
;  All large chunks which have been extended after last call to sys_trim
;  are kept in a single-linked list with head inside malloc_state.
;  For a chunk not in the list, release_fd/release_bk point to itself.
struct malloc_tree_chunk malloc_chunk
left_child      dd      ?       ; pointer to malloc_tree_chunk.data
right_child     dd      ?       ; pointer to malloc_tree_chunk.data
parent          dd      ?       ; pointer to malloc_tree_chunk.data
index           dd      ?
ends
tchunk_left_child  = malloc_tree_chunk.left_child  - malloc_chunk.data
tchunk_right_child = malloc_tree_chunk.right_child - malloc_chunk.data
tchunk_parent      = malloc_tree_chunk.parent      - malloc_chunk.data
tchunk_index       = malloc_tree_chunk.index       - malloc_chunk.data
tchunk_release_fd  = mchunk_prev_foot - 8
tchunk_release_bk  = tchunk_release_fd + 4

; ----------------------------- Segments --------------------------------
;
;  Each malloc space may include non-contiguous segments, held in a
;  list headed by an embedded malloc_segment record representing the
;  top-most space. Large chunks that are directly allocated by mmap are not
;  included in this list. They are instead independently created and
;  destroyed without otherwise keeping track of them.
;
;  Except for the top-most segment of an mstate, each segment record
;  is kept at the tail of its segment. Segments are added by pushing
;  segment records onto the list headed by &mstate.seg for the
;  containing mstate.
;
struct malloc_segment
base            dd      ?       ; base address
size            dd      ?       ; allocated size
next            dd      ?       ; pointer to next malloc_segment
ends

; ---------------------------- malloc_state -----------------------------
;   A malloc_state holds all of the bookkeeping for a space.
;   The main fields are:
;
;  Top
;    The topmost chunk of the currently active segment. Its size is
;    cached in topsize.  The actual size of topmost space is
;    topsize+TOP_FOOT_SIZE, which includes space reserved for adding
;    fenceposts and segment records if necessary when getting more
;    space from the system.
;
;  Designated victim (dv)
;    This is the preferred chunk for servicing small requests that
;    don't have exact fits.  It is normally the chunk split off most
;    recently to service another small request.  Its size is cached in
;    dvsize. The link fields of this chunk are not maintained since it
;    is not kept in a bin.
;
;  SmallBins
;    An array of bin headers for free chunks.  These bins hold chunks
;    with sizes less than MIN_LARGE_SIZE bytes. Each bin contains
;    chunks of all the same size, spaced 8 bytes apart.  To simplify
;    use in double-linked lists, each bin header acts as a malloc_chunk
;    pointing to the real first node, if it exists (else pointing to
;    itself).  This avoids special-casing for headers.  But to avoid
;    waste, we allocate only the fd/bk pointers of bins, and then use
;    repositioning tricks to treat these as the fields of a chunk.
;
;  TreeBins
;    Treebins are pointers to the roots of trees holding a range of
;    sizes. There are 2 equally spaced treebins for each power of two
;    from TREE_SHIFT to TREE_SHIFT+16. The last bin holds anything
;    larger.
;
;  Bin maps
;    There is one bit map for small bins ("smallmap") and one for
;    treebins ("treemap).  Each bin sets its bit when non-empty, and
;    clears the bit when empty.  Bit operations are then used to avoid
;    bin-by-bin searching -- nearly all "search" is done without ever
;    looking at bins that won't be selected.  The bit maps
;    conservatively use 32 bits per map word, even if on 64bit system.
;    For a good description of some of the bit-based techniques used
;    here, see Henry S. Warren Jr's book "Hacker's Delight" (and
;    supplement at http://hackersdelight.org/). Many of these are
;    intended to reduce the branchiness of paths through malloc etc, as
;    well as to reduce the number of memory locations read or written.
;
;  Segments
;    A list of segments headed by an embedded malloc_segment record
;    representing the initial space.
;
;  Magic tag
;    A cross-check field that should always hold same value as malloc_magic.
;
;  Max allowed footprint
;    The maximum allowed bytes to allocate from system (zero means no limit)
;
;  Flags
;    Bits recording whether to use locks
;
;  Statistics
;    Each space keeps track of current and maximum system memory.
;
;  Trim support
;    Fields holding the amount of unused topmost memory that should trigger
;    trimming, and a counter to force periodic scanning to release unused
;    non-topmost segments.
;
;  Locking
;    If USE_LOCKS is defined, the "mutex" lock is acquired and released
;    around every public call using this mspace.
;typedef struct malloc_segment  msegment;
;typedef struct malloc_segment* msegmentptr;
NSMALLBINS              =       32
NTREEBINS               =       32
SMALLBIN_SHIFT          =       3
SMALLBIN_WIDTH          =       1 shl SMALLBIN_SHIFT
TREEBIN_SHIFT           =       8
MIN_LARGE_SIZE          =       1 shl TREEBIN_SHIFT
assert MIN_LARGE_SIZE = NSMALLBINS * SMALLBIN_WIDTH
MAX_SMALL_SIZE          =       MIN_LARGE_SIZE - 1
MAX_SMALL_REQUEST       =       MAX_SMALL_SIZE - CHUNK_ALIGN_MASK - CHUNK_OVERHEAD

struct malloc_state
smallmap        dd      ?       ; bitmap for non-empty small bins
treemap         dd      ?       ; bitmap for non-empty tree bins
dvsize          dd      ?       ; designated victim size
topsize         dd      ?       ; size of free top area
dv              dd      ?       ; pointer to malloc_chunk.data of designated victim
top             dd      ?       ; pointer to malloc_chunk.data of top area
topmax          dd      ?       ; maximum value of top after prev release check
release_checks  dd      ?       ; counter before next release check
release_list    rd      2       ; list of malloc_tree_chunk with pages to release
mmap_list       rd      2       ; list of mmapped chunks
magic           dd      ?       ; for FOOTERS
smallbins       rd      NSMALLBINS*2    ; pointers to malloc_chunk.data
treebins        rd      NTREEBINS       ; pointers to malloc_tree_chunk.data
footprint       dd      ?
max_footprint   dd      ?
footprint_limit dd      ?       ; zero means no limit
mmap_threshold  dd      ?
segment_size    dd      ?
mflags          dd      ?
mutex           dd      ?
seg             malloc_segment
ends
; Bits for malloc_state.mflags
USE_LOCK_BIT = 2
;
TOP_FOOT_SIZE = (-8) and CHUNK_ALIGN_MASK + (sizeof.malloc_segment + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) and not CHUNK_ALIGN_MASK + MIN_CHUNK_SIZE
SYS_ALLOC_PADDING = TOP_FOOT_SIZE + MALLOC_ALIGNMENT

; in: ebp -> malloc_state
; out: eax -> malloc_segment
macro segment_holding address
{
        lea     eax, [ebp+malloc_state.seg]
@@:
        mov     ecx, address
        sub     ecx, [eax+malloc_segment.base]
        cmp     ecx, [eax+malloc_segment.size]
        jb      @f
        mov     eax, [eax+malloc_segment.next]
        test    eax, eax
        jnz     @b
@@:
}

; in: ebp -> malloc_state
macro lock_heap
{
local .done
        test    [ebp+malloc_state.mflags], USE_LOCK_BIT
        jz      .done
; TODO: use mutex when it will be available in kernel API
@@:
        mov     eax, 1
        xchg    [ebp+malloc_state.mutex], eax
        test    eax, eax
        jz      .done
        mov     eax, 68
        mov     ebx, 1
        call    FS_SYSCALL_PTR
        jmp     @b
.done:
}
; in: ebp -> malloc_state
macro unlock_heap
{
local .done
        test    [ebp+malloc_state.mflags], USE_LOCK_BIT
        jz      .done
; TODO: use mutex when it will be available in kernel API
        mov     [ebp+malloc_state.mutex], 0
.done:
}

; ---------------------------- Indexing Bins ----------------------------
MIN_SMALL_INDEX = MIN_CHUNK_SIZE shr SMALLBIN_SHIFT
; in: sizereg = size, must be >= 1 shl TREEBIN_SHIFT
; out: ecx = index, sizereg = size << leftshift_for_tree_index
macro compute_tree_index sizereg
{
        mov     ecx, NTREEBINS-1
        cmp     sizereg, 0xC000 shl TREEBIN_SHIFT
        jae     @f
        bsr     ecx, sizereg
        ror     sizereg, cl
        adc     ecx, ecx
        lea     sizereg, [(sizereg-1)*2]
        sub     ecx, TREEBIN_SHIFT*2
@@:
}
; ----------------------- Runtime Check Support -------------------------
macro mark_inuse_foot where
{
if FOOTERS
        mov     ebx, ebp
        xor     ebx, [malloc_magic]
        mov     [where+mchunk_prev_foot], ebx
end if
}

; ----------------------- Operations on smallbins -----------------------
; Link a free chunk into a smallbin
; in: ebp -> malloc_state, esi -> malloc_chunk.data, edx = size
macro insert_small_chunk_noshift
{
        bts     [ebp+malloc_state.smallmap], edx
        mov     ecx, [ebp+malloc_state.smallbins+edx*8+mchunk_fd]
        lea     edx, [ebp+malloc_state.smallbins+edx*8]
        mov     [ecx+mchunk_bk], esi
        mov     [edx+mchunk_fd], esi
        mov     [esi+mchunk_fd], ecx
        mov     [esi+mchunk_bk], edx
}
macro insert_small_chunk
{
        shr     edx, SMALLBIN_SHIFT
        insert_small_chunk_noshift
}

; Unlink a chunk from a smallbin
; in: ebp -> malloc_state, eax -> malloc_chunk.data, edx = size
macro unlink_small_chunk
{
        mov     ecx, [eax+mchunk_fd]
        mov     ebx, [eax+mchunk_bk]
        cmp     [ecx+mchunk_bk], eax
        jnz     heap_corrupted
        cmp     [ebx+mchunk_fd], eax
        jnz     heap_corrupted
        mov     [ecx+mchunk_bk], ebx
        mov     [ebx+mchunk_fd], ecx
        cmp     ecx, ebx
        jnz     @f
        shr     edx, SMALLBIN_SHIFT
        btr     [ebp+malloc_state.smallmap], edx
@@:
}

; Unlink the first chunk from a smallbin
; in: ebp -> malloc_state, edx -> list header, eax -> malloc_chunk.data
; in: ecx = smallbin index
macro unlink_first_small_chunk
{
        mov     ebx, [eax+mchunk_fd]
        cmp     [ebx+mchunk_bk], eax
        jnz     heap_corrupted
        mov     [ebx+mchunk_bk], edx
        mov     [edx+mchunk_fd], ebx
        cmp     edx, ebx
        jnz     @f
        btr     [ebp+malloc_state.smallmap], ecx
@@:
}

; Replace dv node, binning the old one
; Used only when dvsize known to be small
; in: ebp -> malloc_state, ecx = chunk size, edx -> malloc_chunk.data
macro replace_dv
{
        mov     esi, [ebp+malloc_state.dv]
        mov     [ebp+malloc_state.dv], edx
        mov     edx, [ebp+malloc_state.dvsize]
        mov     [ebp+malloc_state.dvsize], ecx
        shr     edx, SMALLBIN_SHIFT
        jz      @f
        insert_small_chunk_noshift ; ebp,esi,edx
@@:
}

; ------------------------- Operations on trees -------------------------
; Insert chunk into tree
; in: ebp -> malloc_state, esi -> malloc_chunk.data, edx = size
macro insert_large_chunk return_action
{
local .not_first, .loop, .exact_size
        mov     edi, edx
        compute_tree_index edx
        lea     ebx, [ebp+malloc_state.treebins+ecx*4]
        mov     [esi+tchunk_index], ecx
        mov     dword [esi+tchunk_left_child], 0
        mov     dword [esi+tchunk_right_child], 0
        bts     [ebp+malloc_state.treemap], ecx
        jc      .not_first
        mov     [ebx], esi
        mov     [esi+tchunk_parent], ebx
        mov     [esi+mchunk_fd], esi
        mov     [esi+mchunk_bk], esi
        return_action
.not_first:
        mov     ebx, [ebx]
.loop:
        mov     ecx, [ebx+mchunk_head]
        and     ecx, not FLAG_BITS
        cmp     ecx, edi
        jz      .exact_size
        xor     ecx, ecx
        add     edx, edx
        adc     ecx, ecx
        lea     ecx, [ebx+tchunk_left_child+ecx*4]
        cmp     dword [ecx], 0
        jz      @f
        mov     ebx, [ecx]
        jmp     .loop
@@:
        mov     [ecx], esi
        mov     [esi+tchunk_parent], ebx
        mov     [esi+mchunk_fd], esi
        mov     [esi+mchunk_bk], esi
        return_action
.exact_size:
        mov     edx, [ebx+mchunk_fd]
        mov     [edx+mchunk_bk], esi
        mov     [ebx+mchunk_fd], esi
        mov     [esi+mchunk_fd], edx
        mov     [esi+mchunk_bk], ebx
        mov     dword [esi+tchunk_parent], 0
        return_action
}

;  Unlink steps:
;
;  1. If x is a chained node, unlink it from its same-sized fd/bk links
;     and choose its bk node as its replacement.
;  2. If x was the last node of its size, but not a leaf node, it must
;     be replaced with a leaf node (not merely one with an open left or
;     right), to make sure that lefts and rights of descendents
;     correspond properly to bit masks.  We use the rightmost descendent
;     of x.  We could use any other leaf, but this is easy to locate and
;     tends to counteract removal of leftmosts elsewhere, and so keeps
;     paths shorter than minimally guaranteed.  This doesn't loop much
;     because on average a node in a tree is near the bottom.
;  3. If x is the base of a chain (i.e., has parent links) relink
;     x's parent and children to x's replacement (or null if none).

; in: ebp -> malloc_state, eax -> malloc_tree_chunk.data
; destroys ebx,edx,esi, keeps eax,ecx
macro unlink_large_chunk
{
local .last, .common1, .find_rightmost_child, .done, .advance_right, .remove_root
        mov     esi, [eax+tchunk_parent]
        cmp     [eax+mchunk_bk], eax
        jz      .last
        mov     edx, [eax+mchunk_fd]
        mov     ebx, [eax+mchunk_bk]
        cmp     [edx+mchunk_bk], eax
        jnz     heap_corrupted
        cmp     [ebx+mchunk_fd], eax
        jnz     heap_corrupted
        mov     [edx+mchunk_bk], ebx
        mov     [ebx+mchunk_fd], edx
        jmp     .common1
.last:
        mov     ebx, [eax+tchunk_right_child]
        lea     edx, [eax+tchunk_right_child]
        test    ebx, ebx
        jnz     .find_rightmost_child
        mov     ebx, [eax+tchunk_left_child]
        lea     edx, [eax+tchunk_left_child]
        test    ebx, ebx
        jz      .common1
.find_rightmost_child:
        cmp     dword [ebx+tchunk_right_child], 0
        jnz     .advance_right
        cmp     dword [ebx+tchunk_left_child], 0
        jz      @f
        lea     edx, [ebx+tchunk_left_child]
        mov     ebx, [ebx+tchunk_left_child]
        jmp     .find_rightmost_child
.advance_right:
        lea     edx, [ebx+tchunk_right_child]
        mov     ebx, [ebx+tchunk_right_child]
        jmp     .find_rightmost_child
@@:
        mov     dword [edx], 0
.common1:
        test    esi, esi
        jz      .done
        mov     edx, [eax+tchunk_index]
        cmp     [ebp+malloc_state.treebins+edx*4], eax
        jz      .remove_root
        xor     edx, edx
        cmp     [esi+tchunk_left_child], eax
        setnz   dl
        mov     [esi+tchunk_left_child+edx*4], ebx
        jmp     @f
.remove_root:
        mov     [ebp+malloc_state.treebins+edx*4], ebx
        test    ebx, ebx
        jnz     @f
        btr     [ebp+malloc_state.treemap], edx
@@:
        test    ebx, ebx
        jz      .done
        mov     [ebx+tchunk_parent], esi
        cmp     dword [eax+tchunk_left_child], 0
        jz      @f
        mov     edx, [eax+tchunk_left_child]
        mov     [ebx+tchunk_left_child], edx
        mov     [edx+tchunk_parent], ebx
@@:
        cmp     dword [eax+tchunk_right_child], 0
        jz      @f
        mov     edx, [eax+tchunk_right_child]
        mov     [ebx+tchunk_right_child], edx
        mov     [edx+tchunk_parent], ebx
@@:
.done:
}

; Relays to large vs small bin operations
; in: ebp -> malloc_state, esi -> malloc_chunk.data, edx = size
macro insert_chunk return_action
{
local .large
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     .large
        insert_small_chunk      ; ebp, esi, edx
        return_action
.large:
        insert_large_chunk return_action        ; ebp, esi, edx
}

; in: ebp -> malloc_state, eax -> malloc_chunk.data, edx = size
macro unlink_chunk
{
local .large, .common
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     .large
        unlink_small_chunk      ; ebp, eax, edx
        jmp     .common
.large:
        unlink_large_chunk      ; ebp, eax
.common:
}

; -----------------------  Direct-mmapping chunks -----------------------
; Since a system call is used in these functions anyway,
; the speed is not of primary value here.

;   Directly mmapped chunks are set up with an offset to the start of
;  the mmapped region stored in the prev_foot field of the chunk. This
;  allows reconstruction of the required argument to MUNMAP when freed,
;  and also allows adjustment of the returned chunk to meet alignment
;  requirements (especially in memalign).

; in: ebp -> malloc_state, ecx = nb
; out: eax -> allocated block on success, 0 on failure
; destroys: eax, ebx, ecx
assert MALLOC_ALIGNMENT <= 4096 ; it is important here
proc mmap_alloc
        add     ecx, 8*4 + CHUNK_ALIGN_MASK + 0xFFF
        jc      .error
        and     ecx, not 0xFFF
        cmp     [ebp+malloc_state.footprint_limit], 0
        jz      @f
        mov     eax, [ebp+malloc_state.footprint]
        add     eax, ecx
        jc      .error
        cmp     eax, [ebp+malloc_state.footprint_limit]
        ja      .error
@@:
        mov     eax, 68
        mov     ebx, 12
        call    FS_SYSCALL_PTR
        test    eax, eax
        jz      .error
        lea     ebx, [ebp+malloc_state.mmap_list]
        mov     edx, [ebp+malloc_state.mmap_list]
        cmp     [edx+4], ebx
        jnz     heap_corrupted
        mov     [ebx], eax
        mov     [edx+4], eax
        mov     [eax], edx
        mov     [eax+4], ebx
        add     eax, ((-16) and CHUNK_ALIGN_MASK) + 16
        lea     edx, [ecx-((-16) and CHUNK_ALIGN_MASK)-8-MMAP_FOOT_PAD]
        mov     dword [eax+mchunk_prev_foot], ((-16) and CHUNK_ALIGN_MASK) + 16
        mov     [eax+mchunk_head], edx
        mark_inuse_foot eax+edx
        mov     dword [eax+edx+mchunk_head], FENCEPOST_HEAD
        mov     dword [eax+edx+4+mchunk_head], 0
        add     ecx, [ebp+malloc_state.footprint]
        mov     [ebp+malloc_state.footprint], ecx
        cmp     ecx, [ebp+malloc_state.max_footprint]
        jb      @f
        mov     [ebp+malloc_state.max_footprint], ecx
@@:
        ret
.error:
        xor     eax, eax
        ret
endp

; in: ebp -> malloc_state, ecx = nb, edx -> old malloc_chunk.data
; in: ebx = can_move: if zero, fail unless realloc in place is possible
proc mmap_resize
        mov     eax, [edx+mchunk_head]
        cmp     ecx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      .error
        sub     eax, ecx
        jc      .realloc
        cmp     eax, 4
        jb      .realloc
        cmp     eax, 0x2000
        ja      .realloc
        mov     eax, edx
        ret
.realloc:
        test    ebx, ebx
        jz      .error
        sub     edx, [edx+mchunk_prev_foot]
        mov     eax, [edx]
        mov     ebx, [edx+4]
        cmp     [eax+4], edx
        jnz     heap_corrupted
        cmp     [ebx], edx
        jnz     heap_corrupted
        mov     [eax+4], ebx
        mov     [ebx], eax
        mov     eax, 68
        mov     ebx, 20
        add     ecx, 8*4 + CHUNK_ALIGN_MASK + 0xFFF
        and     ecx, not 0xFFF
        call    FS_SYSCALL_PTR
        test    eax, eax
        jz      .error
        mov     ebx, [eax]
        mov     [ebx+4], eax
        mov     ebx, [eax+4]
        mov     [ebx], eax
        add     eax, ((-16) and CHUNK_ALIGN_MASK)+16
        mov     edx, [eax+mchunk_head]
        add     edx, ((-16) and CHUNK_ALIGN_MASK)+8+MMAP_FOOT_PAD
        lea     ebx, [ecx-((-16) and CHUNK_ALIGN_MASK)-8-MMAP_FOOT_PAD]
        mov     [eax+mchunk_head], ebx
        mark_inuse_foot eax+ecx-((-16) and CHUNK_ALIGN_MASK)-8-MMAP_FOOT_PAD
        mov     dword [eax+ecx-((-16) and CHUNK_ALIGN_MASK)-8-MMAP_FOOT_PAD+mchunk_head], FENCEPOST_HEAD
        mov     dword [eax+ecx-((-16) and CHUNK_ALIGN_MASK)-8-MMAP_FOOT_PAD+4+mchunk_head], 0
        add     ecx, [ebp+malloc_state.footprint]
        sub     ecx, edx
        mov     [ebp+malloc_state.footprint], ecx
        cmp     ecx, [ebp+malloc_state.max_footprint]
        jb      @f
        mov     [ebp+malloc_state.max_footprint], ecx
@@:
        ret
.error:
        xor     eax, eax
        ret
endp

; -------------------------- mspace management --------------------------

; Initialize top chunk and its size
; in: eax -> malloc_state, ebx -> malloc_chunk.data for top, ecx = size
; ebx must be aligned to MALLOC_ALIGNMENT
macro init_top
{
        mov     [eax+malloc_state.top], ebx
        mov     [eax+malloc_state.topmax], ebx
        mov     [eax+malloc_state.topsize], ecx
        lea     edx, [ecx+PINUSE_BIT]
        mov     [ebx+mchunk_head], edx
        mov     dword [ebx+ecx+mchunk_head], TOP_FOOT_SIZE
}

; Same as init_top, but return the previous top
; in: ebp -> malloc_state, eax -> malloc_chunk.data for top, ecx = size
; out: edx -> malloc_chunk.data for old top
macro reinit_top
{
        lea     edx, [ecx+PINUSE_BIT]
        mov     [eax+mchunk_head], edx
        mov     dword [eax+ecx+mchunk_head], TOP_FOOT_SIZE
        mov     edx, [ebp+malloc_state.top]
        mov     [ebp+malloc_state.top], eax
        mov     [ebp+malloc_state.topmax], eax
        mov     [ebp+malloc_state.topsize], ecx
}

; Initialize bins for a new mstate that is otherwise zeroed out
; in: eax -> malloc_state
macro init_bins
{
        lea     ebx, [eax+malloc_state.smallbins]
        mov     edx, NSMALLBINS
@@:
        mov     [ebx+mchunk_fd], ebx
        mov     [ebx+mchunk_bk], ebx
        add     ebx, 8
        dec     edx
        jnz     @b
}

; Add a segment to hold a new noncontiguous region
; in: ebp -> malloc_state, eax = segment base, ecx = segment size
malloc_seg_chunk_size = (sizeof.malloc_segment + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) and not CHUNK_ALIGN_MASK
macro add_segment
{
local .nothing, .large
        push    edi
        push    eax ecx
; Ensure alignment for top
        add     eax, 8 + ((-8) and CHUNK_ALIGN_MASK)
        sub     ecx, TOP_FOOT_SIZE + ((-8) and CHUNK_ALIGN_MASK)
; reset top to new space
        reinit_top
        segment_holding edx
        mov     ecx, [eax+malloc_segment.base]
        add     ecx, [eax+malloc_segment.size]
        lea     ebx, [edx+MIN_CHUNK_SIZE]
        lea     eax, [ecx - malloc_seg_chunk_size - MALLOC_ALIGNMENT]
        cmp     eax, ebx
        jae     @f
        mov     eax, edx
@@:
; Set up segment record
        mov     dword [eax+mchunk_head], malloc_seg_chunk_size+PINUSE_BIT+CINUSE_BIT
        mark_inuse_foot eax+malloc_seg_chunk_size
; Push current record
        mov     ebx, [ebp+malloc_state.seg.base]
        mov     [eax+malloc_segment.base], ebx
        mov     ebx, [ebp+malloc_state.seg.size]
        mov     [eax+malloc_segment.size], ebx
        mov     ebx, [ebp+malloc_state.seg.next]
        mov     [eax+malloc_segment.next], ebx
        popd    [ebp+malloc_state.seg.size] [ebp+malloc_state.seg.base]
        mov     [ebp+malloc_state.seg.next], eax
; Insert trailing fenceposts
        mov     esi, edx
        sub     edx, eax
        lea     edi, [eax+malloc_seg_chunk_size+mchunk_head]
        mov     eax, FENCEPOST_HEAD
        sub     ecx, edi
        shr     ecx, 2
        rep stosd
; Insert the rest of old top into a bin as an ordinary free chunk
        neg     edx
        jz      .nothing
        and     dword [esi+edx+mchunk_head], not PINUSE_BIT
        lea     ecx, [edx+PINUSE_BIT]
        mov     [esi+mchunk_head], ecx
        mov     [esi+edx+mchunk_prev_foot], edx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      .small
        lea     ecx, [esi+edx]
        mov     [ecx+tchunk_release_fd], ecx
        mov     [ecx+tchunk_release_bk], ecx
        insert_large_chunk <jmp .nothing>       ; ebp, esi, edx
.small:
        insert_small_chunk      ; ebp, esi, edx
.nothing:
        pop     edi
}

; -------------------------- System allocation --------------------------

; Get memory from system using MMAP
; in: ebp -> malloc_state, esi = size

; Allocate a new segment and a chunk inside that segment.
; Algorithm of segment size calculation:
; 1. Calculate minimal size required to
;    successful subsequent allocation of a chunk.
; 2. Set maximum of it and malloc_state.segment_size as the new size.
; 3. If footprint limit is set and the selected size would overflow it,
;    decrease to the maximum allowed by the limit.
; 4. If the selected size is less than the minimal size, fail.
; 5. Call the kernel. If failed, retry several times halving
;    the requested size until either allocation succeeds or
;    the minimal size is reached; in the former case, try one last time
;    requesting the minimal size. If failed, pass the fail to the caller.
; 6. If allocation succeeded, add the final size to malloc_state.segment_size
;    for subsequent allocations.
proc sys_alloc
        call    trim_top
        mov     edx, esi
        add     edx, SYS_ALLOC_PADDING + 0xFFF
        jc      .fail
        and     edx, not 0xFFF
; edx = minimal memory required
        mov     ecx, [ebp+malloc_state.segment_size]
        cmp     ecx, edx
        ja      @f
        mov     ecx, edx
@@:
        mov     ebx, [ebp+malloc_state.footprint_limit]
        test    ebx, ebx
        jz      @f
        sub     ebx, [ebp+malloc_state.footprint]
        cmp     ecx, ebx
        jb      @f
        mov     ecx, ebx
@@:
        cmp     ecx, edx
        jb      .fail
.retry:
        mov     eax, 68
        mov     ebx, 12
        call    FS_SYSCALL_PTR
        test    eax, eax
        jnz     .ok
        cmp     ecx, edx
        jbe     .fail
        shr     ecx, 1
        cmp     ecx, edx
        jae     .retry
        mov     ecx, edx
        jmp     .retry
.fail:
        mov     FS_ERRNO, ENOMEM
        xor     eax, eax
        ret
.ok:
        add     [ebp+malloc_state.segment_size], ecx
        add     [ebp+malloc_state.footprint], ecx
        mov     ebx, [ebp+malloc_state.footprint]
        cmp     ebx, [ebp+malloc_state.max_footprint]
        jb      @f
        mov     [ebp+malloc_state.max_footprint], ebx
@@:
        push    esi
        add_segment     ; ebp, eax, ecx
        pop     esi
        mov     ecx, [ebp+malloc_state.topsize]
        sub     ecx, esi
        jbe     .fail
        mov     [ebp+malloc_state.topsize], ecx
        mov     eax, [ebp+malloc_state.top]
        add     [ebp+malloc_state.top], esi
        add     [ebp+malloc_state.topmax], esi
        lea     edx, [ecx+PINUSE_BIT]
        mov     [eax+esi+mchunk_head], edx
        lea     edx, [esi+PINUSE_BIT+CINUSE_BIT]
        mov     [eax+mchunk_head], edx
        mark_inuse_foot eax+esi
        ret
endp

; -----------------------  system deallocation --------------------------

proc chunk_trim
        add     edx, 0xFFF
        and     edx, not 0xFFF
        and     esi, not 0xFFF
        sub     esi, edx
        jbe     .nothing
        segment_holding edx
        mov     ecx, [eax+malloc_segment.base]
        mov     eax, 68
        mov     ebx, 26
        sub     edx, ecx
        call    FS_SYSCALL_PTR
.nothing:
        ret
endp

proc trim_top
        mov     edx, [ebp+malloc_state.top]
        xor     edx, [ebp+malloc_state.topmax]
        test    edx, not 0xFFF
        jz      .nothing
        push    esi
        mov     edx, [ebp+malloc_state.top]
        mov     esi, [ebp+malloc_state.topmax]
        mov     [ebp+malloc_state.topmax], edx
        add     esi, 0xFFF
        call    chunk_trim
        pop     esi
.nothing:
        ret
endp

proc sys_trim
        call    trim_top
        push    edi
        lea     edi, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     eax, [ebp+malloc_state.release_list]
        push    edi
.release_list:
        cmp     eax, [esp]
        jz      .release_done
        mov     edi, eax
        mov     esi, [eax+mchunk_prev_foot]
        sub     eax, [eax+mchunk_prev_foot]
        lea     edx, [eax+sizeof.malloc_tree_chunk-malloc_chunk.data]
        lea     esi, [eax+esi+tchunk_release_fd]
        call    chunk_trim
        mov     eax, [edi+tchunk_release_fd]
        mov     [edi+tchunk_release_fd], edi
        mov     [edi+tchunk_release_bk], edi
        jmp     .release_list
.release_done:
        mov     [ebp+malloc_state.release_list+0], eax
        mov     [ebp+malloc_state.release_list+4], eax
        pop     edi edi
        mov     [ebp+malloc_state.release_checks], RELEASE_CHECK_RATE
        ret
endp

macro trim_if_should
{
        dec     [ebp+malloc_state.release_checks]
        jnz     @f
        call    sys_trim
@@:
}

; ---------------------------- malloc ---------------------------

; allocate a large request from the best fitting chunk in a treebin
; if succeeded, unlock the heap and return
; if failed, go ahead
; in: ebp -> malloc_state, esi = nb
macro tmalloc_large_and_return
{
local .find_in_bin, .follow_right, .try_next_bin, .found_subtree, .found_match, .no_new_chunk, .follow_left, .nothing
        push    edi
macro ret \{
        pop     edi
        ret
\}
        push    0
        push    0
        mov     eax, esi
        compute_tree_index eax
; ecx = idx, eax = sizebits
        sub     [esp], esi
; edx = rsize
        cmp     [ebp+malloc_state.treebins+ecx*4], 0
        jz      .try_next_bin
; Traverse tree for this bin looking for node with size == nb
        mov     edi, [ebp+malloc_state.treebins+ecx*4]
        xor     ebx, ebx        ; The deepest untaken right subtree
.find_in_bin:
        mov     edx, [edi+mchunk_head]
        and     edx, not FLAG_BITS
        sub     edx, esi
        cmp     [esp], edx
        jbe     @f
        mov     [esp], edx
        mov     [esp+4], edi
        test    edx, edx
        jz      .found_match
@@:
        add     eax, eax
        jc      .follow_right
        cmp     dword [edi+tchunk_right_child], 0
        jz      @f
        mov     ebx, [edi+tchunk_right_child]
@@:
        cmp     dword [edi+tchunk_left_child], 0
        mov     edi, [edi+tchunk_left_child]
        jnz     .find_in_bin
        jmp     @f
.follow_right:
        cmp     dword [edi+tchunk_right_child], 0
        mov     edi, [edi+tchunk_right_child]
        jnz     .find_in_bin
@@:
        mov     edi, ebx        ; set t to least subtree holding sizes > nb
        test    ebx, ebx
        jnz     .found_subtree
        cmp     dword [esp+4], 0
        jnz     .found_match
.try_next_bin:
; set t to root of next non-empty treebin
        mov     eax, [ebp+malloc_state.treemap]
        shr     eax, cl
        shr     eax, 1
        jz      .nothing
        bsf     eax, eax
        add     eax, ecx
        mov     edi, [ebp+malloc_state.treebins+(eax+1)*4]
.found_subtree:
; find smallest of tree or subtree
        mov     edx, [edi+mchunk_head]
        and     edx, not FLAG_BITS
        sub     edx, esi
        cmp     [esp], edx
        jbe     @f
        mov     [esp], edx
        mov     [esp+4], edi
@@:
        cmp     dword [edi+tchunk_left_child], 0
        jnz     .follow_left
        cmp     dword [edi+tchunk_right_child], 0
        mov     edi, [edi+tchunk_right_child]
        jnz     .found_subtree
        cmp     dword [esp+4], 0
        jz      .nothing
.found_match:
; If dv is a better fit, return 0 so malloc will use it
        mov     eax, [ebp+malloc_state.dvsize]
        sub     eax, esi
        cmp     [esp], eax
        jae     .nothing
        mov     eax, [esp+4]
        cmp     dword [esp], NSMALLBINS shl SMALLBIN_SHIFT
        jae     @f
        mov     ecx, [esp]
        add     ecx, esi
        add     ecx, eax
        mov     ebx, [ecx+tchunk_release_fd]
        mov     edx, [ecx+tchunk_release_bk]
        cmp     [ebx+tchunk_release_bk], ecx
        jnz     heap_corrupted
        cmp     [edx+tchunk_release_fd], ecx
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_bk], edx
        mov     [edx+tchunk_release_fd], ebx
@@:
        mov     ecx, esi
        unlink_large_chunk      ; ebp, eax
        cmp     dword [esp], MIN_CHUNK_SIZE
        jb      .no_new_chunk
        lea     edx, [ecx+PINUSE_BIT+CINUSE_BIT]
        mov     [eax+mchunk_head], edx
        mark_inuse_foot eax+ecx
        lea     esi, [eax+ecx]
        mov     edx, [esp]
        lea     ecx, [edx+PINUSE_BIT]
        mov     [esi+mchunk_head], ecx
        mov     [esi+edx+mchunk_prev_foot], edx
        add     esp, 8
macro unlock_ret \{
        unlock_heap
        ret
\}
        insert_chunk unlock_ret ; ebp, esi, edx
purge unlock_ret
.no_new_chunk:
        add     ecx, [esp]
        lea     edx, [ecx+PINUSE_BIT+CINUSE_BIT]
        mov     [eax+mchunk_head], edx
        or      dword [eax+ecx+mchunk_head], PINUSE_BIT
        mark_inuse_foot eax+ecx
        add     esp, 8
        unlock_heap
        ret
.follow_left:
        mov     edi, [edi+tchunk_left_child]
        jmp     .found_subtree
.nothing:
        add     esp, 8
        pop     edi
purge ret
}

; allocate a small request from the best fitting chunk in a treebin
; in: ebp -> malloc_state, esi = size
macro tmalloc_small_and_return
{
local .follow_left, .found_best, .no_new_chunk
        bsf     ecx, [ebp+malloc_state.treemap]
        mov     eax, [ebp+malloc_state.treebins+ecx*4]
        mov     edx, eax
        mov     ecx, [eax+mchunk_head]
        and     ecx, not FLAG_BITS
        sub     ecx, esi
@@:
        cmp     dword [edx+tchunk_left_child], 0
        jnz     .follow_left
        cmp     dword [edx+tchunk_right_child], 0
        jz      .found_best
        mov     edx, [edx+tchunk_right_child]
        mov     ebx, [edx+mchunk_head]
        and     ebx, not FLAG_BITS
        sub     ebx, esi
        cmp     ecx, ebx
        jbe     @b
        mov     eax, edx
        mov     ecx, ebx
        jmp     @b
.follow_left:
        mov     edx, [edx+tchunk_left_child]
        mov     ebx, [edx+mchunk_head]
        and     ebx, not FLAG_BITS
        sub     ebx, esi
        cmp     ecx, ebx
        jbe     @b
        mov     eax, edx
        mov     ecx, ebx
        jmp     @b
.found_best:
        push    esi
        cmp     ecx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     @f
        add     esi, ecx
        add     esi, eax
        mov     ebx, [esi+tchunk_release_fd]
        mov     edx, [esi+tchunk_release_bk]
        cmp     [ebx+tchunk_release_bk], esi
        jnz     heap_corrupted
        cmp     [edx+tchunk_release_fd], esi
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_bk], edx
        mov     [edx+tchunk_release_fd], ebx
@@:
        unlink_large_chunk      ; ebp, eax
        pop     esi
        cmp     ecx, MIN_CHUNK_SIZE
        jb      .no_new_chunk
        lea     edx, [eax+esi]
        lea     ebx, [esi+PINUSE_BIT+CINUSE_BIT]
        mov     [eax+mchunk_head], ebx
        mark_inuse_foot edx
        lea     ebx, [ecx+PINUSE_BIT]
        mov     [edx+mchunk_head], ebx
        mov     [edx+ecx+mchunk_prev_foot], ecx
        replace_dv      ; ebp, ecx, edx
        unlock_heap
        ret
.no_new_chunk:
        lea     edx, [ecx+esi+PINUSE_BIT+CINUSE_BIT]
        add     ecx, esi
        mov     [eax+mchunk_head], edx
        or      dword [eax+ecx+mchunk_head], PINUSE_BIT
        mark_inuse_foot eax+ecx
        unlock_heap
        ret
}

; in: ebp -> malloc_state
; in: [bytes] = stack var with number of bytes to allocate
; out: eax -> allocated data
macro do_malloc
{
;
;     Basic algorithm:
;     If a small request (< 256 bytes minus per-chunk overhead):
;       1. If one exists, use a remainderless chunk in associated smallbin.
;          (Remainderless means that there are too few excess bytes to
;          represent as a chunk.)
;       2. If it is big enough, use the dv chunk, which is normally the
;          chunk adjacent to the one used for the most recent small request.
;       3. If one exists, split the smallest available chunk in a bin,
;          saving remainder in dv.
;       4. If it is big enough, use the top chunk.
;       5. If available, get memory from system and use it
;     Otherwise, for a large request:
;       1. Find the smallest available binned chunk that fits, and use it
;          if it is better fitting than dv chunk, splitting if necessary.
;       2. If better fitting than any binned chunk, use the dv chunk.
;       3. If request size >= mmap threshold, try to directly mmap this chunk.
;       4. If it is big enough, use the top chunk.
;       5. If available, get memory from system and use it
;
        lock_heap
        mov     eax, [bytes]
        cmp     [bytes], MAX_SMALL_REQUEST
        ja      .large
        mov     ecx, MIN_CHUNK_SIZE
        cmp     eax, MIN_REQUEST
        jb      @f
        lea     ecx, [eax + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK]
        and     ecx, not CHUNK_ALIGN_MASK
@@:
        mov     esi, ecx
        shr     ecx, SMALLBIN_SHIFT
        mov     eax, [ebp + malloc_state.smallmap]
        shr     eax, cl
        test    eax, 3
        jz      .small.remainder
; Remainderless fit to a smallbin.
        shr     eax, 1
        sbb     ecx, -1         ; Uses next bin if idx empty
        lea     edx, [ebp + malloc_state.smallbins + ecx*8]
        mov     eax, [ebp + malloc_state.smallbins + ecx*8 + mchunk_fd]
        unlink_first_small_chunk        ; ebp, edx, eax, ecx
        lea     edx, [ecx*SMALLBIN_WIDTH+PINUSE_BIT+CINUSE_BIT]
        mov     [eax+mchunk_head], edx
        or      dword [eax+ecx*SMALLBIN_WIDTH+mchunk_head], PINUSE_BIT
        mark_inuse_foot eax+ecx*SMALLBIN_WIDTH
        unlock_heap
        ret
.small.remainder:
        cmp     esi, [ebp+malloc_state.dvsize]
        jbe     .use_dv_chunk
        test    eax, eax
        jz      .small.try_split_large
        bsf     eax, eax
        add     ecx, eax
        lea     edx, [ebp + malloc_state.smallbins + ecx*8]
        mov     eax, [ebp + malloc_state.smallbins + ecx*8 + mchunk_fd]
        unlink_first_small_chunk        ; ebp, edx, eax, ecx
        lea     edx, [esi+PINUSE_BIT+CINUSE_BIT]
        mov     [eax+mchunk_head], edx
        lea     edx, [eax+esi]
        shl     ecx, SMALLBIN_SHIFT
        sub     ecx, esi
        mark_inuse_foot edx
        lea     ebx, [ecx+PINUSE_BIT]
        mov     [edx+mchunk_head], ebx
        mov     [edx+ecx+mchunk_prev_foot], ecx
        replace_dv      ; ebp, ecx, edx
        unlock_heap
        ret
.use_dv_chunk:
        mov     ecx, [ebp+malloc_state.dvsize]
        mov     eax, [ebp+malloc_state.dv]
        sub     ecx, esi
        cmp     [ebp+malloc_state.dvsize], NSMALLBINS shl SMALLBIN_SHIFT
        jb      @f
        cmp     ecx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     @f
        lea     edx, [eax+esi]
        add     edx, ecx
        mov     ebx, [edx+tchunk_release_fd]
        cmp     [ebx+tchunk_release_bk], edx
        jnz     heap_corrupted
        mov     ebx, [edx+tchunk_release_bk]
        cmp     [ebx+tchunk_release_fd], edx
        jnz     heap_corrupted
        mov     edx, [edx+tchunk_release_fd]
        mov     [ebx+tchunk_release_fd], edx
        mov     [edx+tchunk_release_bk], ebx
@@:
        cmp     ecx, MIN_CHUNK_SIZE
        jb      .no_new_chunk_dv
        lea     edx, [eax+esi]
        mov     [ebp+malloc_state.dv], edx
        mov     [ebp+malloc_state.dvsize], ecx
        lea     ebx, [ecx+PINUSE_BIT]
        mov     [edx+mchunk_head], ebx
        mov     [edx+ecx+mchunk_prev_foot], ecx
        lea     ebx, [esi+PINUSE_BIT+CINUSE_BIT]
        mov     [eax+mchunk_head], ebx
        mark_inuse_foot edx
        unlock_heap
        ret
.no_new_chunk_dv:
        add     ecx, esi
        mov     [ebp+malloc_state.dv], 0
        mov     [ebp+malloc_state.dvsize], 0
        lea     ebx, [ecx+PINUSE_BIT+CINUSE_BIT]
        mov     [eax+mchunk_head], ebx
        or      dword [eax+ecx+mchunk_head], PINUSE_BIT
        mark_inuse_foot eax+ecx
        unlock_heap
        ret
.small.try_split_large:
        cmp     [ebp+malloc_state.treemap], 0
        jz      .from_top_or_sys
        tmalloc_small_and_return
.large:
        cmp     eax, MAX_REQUEST
        jae     .fail
        lea     esi, [eax + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK]
        and     esi, not CHUNK_ALIGN_MASK
        cmp     [ebp+malloc_state.treemap], 0
        jz      .from_dv_top_or_sys
        tmalloc_large_and_return        ; can fail
.from_dv_top_or_sys:
        cmp     esi, [ebp+malloc_state.dvsize]
        jbe     .use_dv_chunk
.from_top_or_sys:
; Directly map large chunks
        cmp     esi, [ebp+malloc_state.mmap_threshold]
        jb      @f
        mov     ecx, esi
        call    mmap_alloc
        test    eax, eax
        jz      @f
        unlock_heap
        ret
@@:
        cmp     esi, [ebp+malloc_state.topsize]
        jae     .from_sys
        mov     eax, [ebp+malloc_state.top]
        mov     ecx, [ebp+malloc_state.topsize]
        lea     edx, [eax+esi]
        sub     ecx, esi
        mov     [ebp+malloc_state.top], edx
        cmp     edx, [ebp+malloc_state.topmax]
        jb      @f
        mov     [ebp+malloc_state.topmax], edx
@@:
        mov     [ebp+malloc_state.topsize], ecx
        lea     ebx, [ecx+PINUSE_BIT]
        mov     [edx+mchunk_head], ebx
        lea     ebx, [esi+PINUSE_BIT+CINUSE_BIT]
        mov     [eax+mchunk_head], ebx
        mark_inuse_foot edx
        unlock_heap
        ret
.from_sys:
        call    sys_alloc
        unlock_heap
        ret
.fail:
        mov     FS_ERRNO, ENOMEM
        xor     eax, eax
        unlock_heap
        ret
}

; ---------------------------- free ---------------------------

; in: ebp -> malloc_state
; in: [mem] = stack var with pointer to free
macro do_free
{
;     Consolidate freed chunks with preceeding or succeeding bordering
;     free chunks, if they exist, and then place in a bin.  Intermixed
;     with special cases for top, dv, mmapped chunks, and usage errors.
        cmp     [mem], 0
        jz      .nothing
if FOOTERS
        mov     ecx, [mem]
        mov     eax, [ecx+mchunk_head]
        and     eax, not FLAG_BITS
        mov     eax, [ecx+eax+mchunk_prev_foot]
        xor     eax, [malloc_magic]
        cmp     eax, ebp
        jnz     heap_corrupted
end if
        lock_heap
        mov     eax, [mem]
        mov     ecx, [eax+mchunk_head]
        mov     edx, ecx
        and     ecx, INUSE_BITS
assert PINUSE_BIT = 1
assert CINUSE_BIT = 2
; Possible values of ecx:
; 0 = mmapped chunk, should be rare
; 1 = illegal, trying to double-free
; 2 = previous chunk is not in use, merge backwards
; 3 = previous chunk is in use, no merge backwards
        and     edx, not FLAG_BITS
        cmp     ecx, 2
        ja      .see_forward
        jb      .free_mmapped
; turn CINUSE_BIT to PINUSE_BIT, increase chances to detect double-free
        dec     dword [eax+mchunk_head]
        cmp     dword [eax+mchunk_prev_foot], NSMALLBINS shl SMALLBIN_SHIFT
        jb      @f
        mov     ebx, [eax+tchunk_release_fd]
        mov     ecx, [eax+tchunk_release_bk]
        cmp     [ebx+tchunk_release_bk], eax
        jnz     heap_corrupted
        cmp     [ecx+tchunk_release_fd], eax
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_bk], ecx
        mov     [ecx+tchunk_release_fd], ebx
@@:
        mov     ecx, [eax+mchunk_prev_foot]
        add     edx, [eax+mchunk_prev_foot]
        sub     eax, [eax+mchunk_prev_foot]
        cmp     eax, [ebp+malloc_state.dv]
        jz      .append_dv
        push    edx
        mov     edx, ecx
        unlink_chunk    ; ebp, eax, edx
        pop     edx
.see_forward:
        mov     esi, eax
assert PINUSE_BIT = 1
        btr     dword [eax+edx+mchunk_head], 0
        jnc     heap_corrupted
        test    dword [eax+edx+mchunk_head], CINUSE_BIT
        jnz     .consolidated
.consolidate_forward:
        add     eax, edx
        cmp     eax, [ebp+malloc_state.top]
        jz      .prepend_top
        cmp     eax, [ebp+malloc_state.dv]
        jz      .prepend_dv
        push    esi edx
        mov     edx, [eax+mchunk_head]
        and     edx, not FLAG_BITS
        add     [esp], edx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     .consolidate_large
        unlink_small_chunk      ; ebp, eax, edx
        jmp     .consolidate_common
.consolidate_large:
        lea     ecx, [eax+edx]
        mov     ebx, [eax+edx+tchunk_release_fd]
        mov     edx, [eax+edx+tchunk_release_bk]
        cmp     [ebx+tchunk_release_bk], ecx
        jnz     heap_corrupted
        cmp     [edx+tchunk_release_fd], ecx
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_bk], edx
        mov     [edx+tchunk_release_fd], ebx
        unlink_large_chunk      ; ebp, eax
.consolidate_common:
        pop     edx esi
        cmp     esi, [ebp+malloc_state.dv]
        jz      .dv_consolidated
.consolidated:
        lea     ecx, [edx+PINUSE_BIT]
        mov     [esi+mchunk_head], ecx
        mov     [esi+edx+mchunk_prev_foot], edx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     .large
        insert_small_chunk      ; ebp, esi, edx
        unlock_heap
.nothing:
        ret
.large:
        lea     ecx, [esi+edx]
        lea     eax, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     ebx, [ebp+malloc_state.release_list]
        cmp     [ebx+tchunk_release_bk], eax
        jnz     heap_corrupted
        mov     [ecx+tchunk_release_fd], ebx
        mov     [ecx+tchunk_release_bk], eax
        mov     [ebx+tchunk_release_bk], ecx
        mov     [eax+tchunk_release_fd], ecx
@@:
        push    edi
macro trim_ret \{
        trim_if_should
        unlock_heap
        pop     edi
        ret
\}
        insert_large_chunk trim_ret     ; ebp, esi, edx
purge trim_ret
.dv_consolidated:
        mov     [esi+edx+mchunk_prev_foot], edx
        mov     [ebp+malloc_state.dvsize], edx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      @f
        lea     ecx, [esi+edx]
        lea     eax, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     ebx, [ebp+malloc_state.release_list]
        cmp     [ebx+tchunk_release_bk], eax
        jnz     heap_corrupted
        mov     [ecx+tchunk_release_fd], ebx
        mov     [ecx+tchunk_release_bk], eax
        mov     [ebx+tchunk_release_bk], ecx
        mov     [eax+tchunk_release_fd], ecx
@@:
assert PINUSE_BIT = 1
        inc     edx
        mov     [esi+mchunk_head], edx
        unlock_heap
        ret
.append_dv:
        mov     esi, eax
assert PINUSE_BIT = 1
        btr     dword [eax+edx+mchunk_head], 0
        jnc     heap_corrupted
        test    dword [eax+edx+mchunk_head], CINUSE_BIT
        jz      .consolidate_forward
        mov     [ebp+malloc_state.dvsize], edx
        lea     ecx, [edx+PINUSE_BIT]
        mov     [eax+mchunk_head], ecx
        mov     [eax+edx+mchunk_prev_foot], edx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      @f
        add     edx, eax
        lea     eax, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     ebx, [ebp+malloc_state.release_list]
        cmp     [ebx+tchunk_release_bk], eax
        jnz     heap_corrupted
        mov     [edx+tchunk_release_fd], ebx
        mov     [edx+tchunk_release_bk], eax
        mov     [ebx+tchunk_release_bk], edx
        mov     [eax+tchunk_release_fd], edx
@@:
        unlock_heap
        ret
.prepend_dv:
        add     edx, [ebp+malloc_state.dvsize]
        cmp     [ebp+malloc_state.dvsize], NSMALLBINS shl SMALLBIN_SHIFT
        jb      @f
        lea     ecx, [esi+edx]
        mov     eax, [esi+edx+tchunk_release_fd]
        mov     ebx, [esi+edx+tchunk_release_bk]
        cmp     [eax+tchunk_release_bk], ecx
        jnz     heap_corrupted
        cmp     [ebx+tchunk_release_fd], ecx
        jnz     heap_corrupted
        mov     [eax+tchunk_release_bk], ebx
        mov     [ebx+tchunk_release_fd], eax
@@:
        mov     [ebp+malloc_state.dvsize], edx
        mov     [ebp+malloc_state.dv], esi
        mov     [esi+edx+mchunk_prev_foot], edx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      @f
        lea     ecx, [esi+edx]
        lea     eax, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     ebx, [ebp+malloc_state.release_list]
        cmp     [ebx+tchunk_release_bk], eax
        jnz     heap_corrupted
        mov     [ecx+tchunk_release_fd], ebx
        mov     [ecx+tchunk_release_bk], eax
        mov     [ebx+tchunk_release_bk], ecx
        mov     [eax+tchunk_release_fd], ecx
@@:
assert PINUSE_BIT = 1
        inc     edx
        mov     [esi+mchunk_head], edx
        unlock_heap
        ret
.prepend_top:
        add     edx, [ebp+malloc_state.topsize]
        mov     [ebp+malloc_state.topsize], edx
        mov     [ebp+malloc_state.top], esi
assert PINUSE_BIT = 1
        inc     edx
        mov     [esi+mchunk_head], edx
        cmp     esi, [ebp+malloc_state.dv]
        jnz     @f
        mov     [ebp+malloc_state.dv], 0
        mov     [ebp+malloc_state.dvsize], 0
@@:
        trim_if_should
        unlock_heap
        ret
.free_mmapped:
        dec     ecx
        jz      heap_corrupted
        mov     ecx, eax
        add     edx, [eax+mchunk_prev_foot]
        add     edx, MMAP_FOOT_PAD-8
        test    edx, 0xFFF
        jnz     heap_corrupted
        sub     [ebp+malloc_state.footprint], edx
        sub     ecx, [ecx+mchunk_prev_foot]
        test    ecx, 0xFFF
        jnz     heap_corrupted
        mov     eax, [ecx]
        mov     ebx, [ecx+4]
        cmp     [eax+4], ecx
        jnz     heap_corrupted
        cmp     [ebx], ecx
        jnz     heap_corrupted
        mov     [eax+4], ebx
        mov     [ebx], eax
        mov     eax, 68
        mov     ebx, 13
        call    FS_SYSCALL_PTR
        unlock_heap
        ret
}

; in: ebp -> malloc_state
; in: [n_elements] = stack var with number of elements
; in: [elem_size] = stack var with element size
macro do_calloc malloc_call
{
        mov     eax, [n_elements]
        mul     [elem_size]
        test    edx, edx
        jnz     .fail
        mov     [n_elements], eax
        malloc_call
        test    eax, eax
        jz      .nothing
        test    dword [eax+mchunk_head], INUSE_BITS
        jz      .nothing
        push    eax edi
        mov     edi, eax
        mov     ecx, [n_elements]
        add     ecx, 3
        shr     ecx, 2
        xor     eax, eax
        rep stosd
        pop     edi eax
.nothing:
        ret
.fail:
        mov     FS_ERRNO, ENOMEM
        xor     eax, eax
        ret
}

; ------------ Internal support for realloc, memalign, etc --------------
; Try to realloc; only in-place unless can_move true
; in: ebp -> malloc_state
; in: [oldmem] = stack var with pointer to realloc
; in: [bytes] = stack var with number of bytes to allocate
macro try_realloc_chunk can_move
{
        lock_heap
        mov     eax, [oldmem]
        mov     ecx, MIN_CHUNK_SIZE
        cmp     [bytes], MIN_REQUEST
        jb      @f
        mov     ecx, [bytes]
        add     ecx, CHUNK_OVERHEAD + CHUNK_ALIGN_MASK
        and     ecx, not CHUNK_ALIGN_MASK
@@:
        mov     [bytes], ecx
        mov     edx, [eax+mchunk_head]
        and     edx, not FLAG_BITS
; Possible values of [eax+mchunk_head] and INUSE_BIT:
; 0 = mmapped chunk, should be rare
; 1 = illegal, trying to realloc already-freed chunk
; 2,3 = normal chunk
        test    dword [eax+mchunk_head], CINUSE_BIT
        jz      .mmapped
        test    dword [eax+edx+mchunk_head], PINUSE_BIT
        jz      heap_corrupted
        cmp     ecx, edx
        jbe     .realloc_decrease
        add     eax, edx
        cmp     eax, [ebp+malloc_state.top]
        jz      .move_top
        cmp     eax, [ebp+malloc_state.dv]
        jz      .extend_into_dv
        test    dword [eax+mchunk_head], CINUSE_BIT
        jnz     .failed
; extend into next free chunk
        push    edx
        mov     edx, [eax+mchunk_head]
        and     edx, not FLAG_BITS
        add     [esp], edx
        sub     [esp], ecx
        jb      .failed_pop
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     .large
        unlink_small_chunk      ; ebp, eax, edx
        jmp     .common
.large:
        cmp     dword [esp], NSMALLBINS shl SMALLBIN_SHIFT
        jae     @f
        lea     ecx, [eax+edx]
        mov     ebx, [eax+edx+tchunk_release_fd]
        mov     edx, [eax+edx+tchunk_release_bk]
        cmp     [ebx+tchunk_release_bk], ecx
        jnz     heap_corrupted
        cmp     [edx+tchunk_release_fd], ecx
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_bk], edx
        mov     [edx+tchunk_release_fd], ebx
@@:
        unlink_large_chunk      ; ebp, eax
.common:
        pop     edx
        mov     eax, [oldmem]
        mov     ecx, [bytes]
        cmp     edx, MIN_CHUNK_SIZE
        jae     .add_chunk
.no_new_chunk:
        add     edx, ecx
        mov     ebx, [eax+mchunk_head]
        and     ebx, PINUSE_BIT
        lea     ebx, [edx+ebx+CINUSE_BIT]
        mov     [eax+mchunk_head], ebx
        or      dword [eax+edx+mchunk_head], PINUSE_BIT
        mark_inuse_foot eax+edx
.nothing:
        unlock_heap
        ret
.realloc_decrease:
        test    dword [eax+edx+mchunk_head], CINUSE_BIT
        jz      .prepend_free
        sub     edx, ecx
        cmp     edx, MIN_CHUNK_SIZE
        jb      .nothing
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      .add_chunk
        lea     ebx, [eax+ecx]
        add     ebx, edx
        lea     eax, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     esi, [ebp+malloc_state.release_list]
        cmp     [esi+tchunk_release_bk], eax
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_fd], esi
        mov     [ebx+tchunk_release_bk], eax
        mov     [esi+tchunk_release_bk], ebx
        mov     [eax+tchunk_release_fd], ebx
        mov     eax, [oldmem]
.add_chunk:
        mov     ebx, [eax+mchunk_head]
        lea     esi, [eax+ecx]
        and     ebx, PINUSE_BIT
        lea     ebx, [ecx+ebx+CINUSE_BIT]
        mov     [eax+mchunk_head], ebx
        mark_inuse_foot esi
        lea     ecx, [edx+PINUSE_BIT]
        mov     [esi+mchunk_head], ecx
        mov     [esi+edx+mchunk_prev_foot], edx
        and     dword [esi+edx+mchunk_head], not PINUSE_BIT
        push    edi
macro unlock_ret \{
        pop     edi
        unlock_heap
        ret
\}
        insert_chunk unlock_ret ; ebp, esi, edx
purge unlock_ret
.prepend_free:
        add     eax, edx
        cmp     eax, [ebp+malloc_state.top]
        jz      .move_top
        cmp     eax, [ebp+malloc_state.dv]
        jz      .prepend_dv
        sub     edx, ecx
        push    edx
        mov     edx, [eax+mchunk_head]
        and     edx, not FLAG_BITS
        add     [esp], edx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     .prepend_old_large
        unlink_small_chunk      ; ebp, eax, edx
        jmp     .prepend_old_common
.prepend_old_large:
        add     edx, eax
        mov     ebx, [edx+tchunk_release_fd]
        mov     ecx, [edx+tchunk_release_bk]
        cmp     [ebx+tchunk_release_bk], edx
        jnz     heap_corrupted
        cmp     [ecx+tchunk_release_fd], edx
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_bk], ecx
        mov     [ecx+tchunk_release_fd], ebx
        unlink_large_chunk      ; ebp, eax
.prepend_old_common:
        pop     edx
        mov     esi, [oldmem]
        add     esi, [bytes]
        mov     [esi+edx+mchunk_prev_foot], edx
        lea     ecx, [edx+PINUSE_BIT]
        mov     [esi+mchunk_head], ecx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      .prepend_new_small
        lea     ecx, [esi+edx]
        lea     eax, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     ebx, [ebp+malloc_state.release_list]
        cmp     [ebx+tchunk_release_bk], eax
        jnz     heap_corrupted
        mov     [eax+tchunk_release_fd], ecx
        mov     [ebx+tchunk_release_bk], ecx
        mov     [ecx+tchunk_release_fd], ebx
        mov     [ecx+tchunk_release_bk], eax
        push    edi
macro after_insert \{
        pop     edi
        jmp     .prepend_new_common
\}
        insert_large_chunk after_insert ; ebp, esi, edx
purge after_insert
.prepend_new_small:
        insert_small_chunk      ; ebp, esi, edx
.prepend_new_common:
        mov     eax, [oldmem]
        mov     ecx, [bytes]
        mark_inuse_foot eax+ecx
        mov     ebx, [eax+mchunk_head]
        and     ebx, PINUSE_BIT
        lea     ebx, [ebx+ecx+CINUSE_BIT]
        mov     [eax+mchunk_head], ebx
        unlock_heap
        ret
.prepend_dv:
        add     edx, [ebp+malloc_state.dvsize]
        add     eax, [ebp+malloc_state.dvsize]
        sub     edx, ecx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      .move_dv
        cmp     [ebp+malloc_state.dvsize], NSMALLBINS shl SMALLBIN_SHIFT
        jb      @f
        cmp     [eax+tchunk_release_fd], eax
        jnz     .move_dv
@@:
        lea     ebx, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     esi, [ebp+malloc_state.release_list]
        cmp     [esi+tchunk_release_bk], ebx
        jnz     heap_corrupted
        mov     [eax+tchunk_release_fd], esi
        mov     [eax+tchunk_release_bk], ebx
        mov     [esi+tchunk_release_bk], eax
        mov     [ebx+tchunk_release_fd], eax
        jmp     .move_dv
.extend_into_dv:
        add     edx, [ebp+malloc_state.dvsize]
        sub     edx, ecx
        jb      .failed
        cmp     [ebp+malloc_state.dvsize], NSMALLBINS shl SMALLBIN_SHIFT
        jb      .move_dv
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     .move_dv
        add     eax, [ebp+malloc_state.dvsize]
        mov     ebx, [eax+tchunk_release_fd]
        mov     esi, [eax+tchunk_release_bk]
        cmp     [ebx+tchunk_release_bk], eax
        jnz     heap_corrupted
        cmp     [esi+tchunk_release_fd], eax
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_bk], esi
        mov     [esi+tchunk_release_fd], ebx
.move_dv:
        mov     eax, [oldmem]
        cmp     edx, MIN_CHUNK_SIZE
        jb      .dv_no_new_chunk
        mov     ebx, [eax+mchunk_head]
        and     ebx, PINUSE_BIT
        lea     ebx, [ecx+ebx+CINUSE_BIT]
        mov     [eax+mchunk_head], ebx
        add     ecx, eax
        mark_inuse_foot ecx
        lea     ebx, [edx+PINUSE_BIT]
        mov     [ecx+mchunk_head], ebx
        mov     [ecx+edx+mchunk_prev_foot], edx
        and     dword [ecx+edx+mchunk_head], not PINUSE_BIT
        mov     [ebp+malloc_state.dvsize], edx
        mov     [ebp+malloc_state.dv], ecx
        unlock_heap
        ret
.dv_no_new_chunk:
        add     edx, ecx
        mov     ebx, [eax+mchunk_head]
        and     ebx, PINUSE_BIT
        lea     ebx, [edx+ebx+CINUSE_BIT]
        mov     [eax+mchunk_head], ebx
        or      dword [eax+edx+mchunk_head], PINUSE_BIT
        mark_inuse_foot eax+edx
        mov     [ebp+malloc_state.dvsize], 0
        mov     [ebp+malloc_state.dv], 0
        unlock_heap
        ret
.move_top:
        add     edx, [ebp+malloc_state.topsize]
        sub     edx, ecx
        jbe     .failed
        mov     eax, [oldmem]
        mov     ebx, [eax+mchunk_head]
        and     ebx, PINUSE_BIT
        lea     ebx, [ecx+ebx+CINUSE_BIT]
        mov     [eax+mchunk_head], ebx
        add     ecx, eax
        mark_inuse_foot ecx
        mov     [ebp+malloc_state.top], ecx
        mov     [ebp+malloc_state.topsize], edx
assert PINUSE_BIT = 1
        inc     edx
        mov     [ecx+mchunk_head], edx
        unlock_heap
        ret
.mmapped:
        test    dword [eax+mchunk_head], PINUSE_BIT
        jnz     heap_corrupted
        mov     edx, eax
if can_move
        mov     ebx, 1
else
        xor     ebx, ebx
end if
        call    mmap_resize
        test    eax, eax
        jz      .failed
        unlock_heap
        ret
.failed_pop:
        pop     edx
.failed:
        unlock_heap
}

; ------------------ Exported realloc, memalign, etc --------------------

; in: ebp -> malloc_state
; in: [oldmem] = stack var with pointer to realloc
; in: [bytes] = stack var with number of bytes to allocate
macro do_realloc malloc_call, free_call
{
        cmp     [oldmem], 0
        jz      .malloc
        cmp     [bytes], 0
        jz      .free
        cmp     [bytes], MAX_REQUEST
        jae     .fail
if FOOTERS
        mov     ecx, [oldmem]
        mov     eax, [ecx+mchunk_head]
        and     eax, not FLAG_BITS
        mov     eax, [ecx+eax+mchunk_prev_foot]
        xor     eax, [malloc_magic]
        cmp     eax, ebp
        jnz     heap_corrupted
end if
        try_realloc_chunk 1     ; ebp, [oldmem], [bytes]
        sub     [bytes], CHUNK_OVERHEAD ; try_realloc_chunk has padded [bytes]
        malloc_call [bytes]
        test    eax, eax
        jz      .justret
        mov     esi, [oldmem]
        push    eax
        push    edi
        mov     ecx, [esi+mchunk_head]
        mov     edi, eax
        mov     edx, MMAP_CHUNK_OVERHEAD
        test    ecx, INUSE_BITS
        jz      @f
        mov     edx, CHUNK_OVERHEAD
@@:
        and     ecx, not FLAG_BITS
        sub     ecx, edx
        cmp     ecx, [bytes+8]
        jb      @f
        mov     ecx, [bytes+8]
@@:
        shr     ecx, 2
        rep movsd
        pop     edi
        free_call [oldmem+4]
        pop     eax
.justret:
        ret
.malloc:
        malloc_call [bytes]
        ret
.free:
        free_call [oldmem]
        xor     eax, eax
        ret
.fail:
        mov     FS_ERRNO, ENOMEM
        xor     eax, eax
        ret
}

; in: ebp -> malloc_state
; in: [oldmem] = stack var with pointer to realloc
; in: [bytes] = stack var with number of bytes to allocate
macro do_realloc_in_place
{
        cmp     [oldmem], 0
        jz      .fail
        cmp     [bytes], MAX_REQUEST
        jae     .fail
if FOOTERS
        mov     ecx, [oldmem]
        mov     eax, [ecx+mchunk_head]
        and     eax, not FLAG_BITS
        mov     eax, [ecx+eax+mchunk_prev_foot]
        xor     eax, [malloc_magic]
        cmp     eax, ebp
        jnz     heap_corrupted
end if
        try_realloc_chunk 0     ; ebp, [oldmem], [bytes]
.fail:
        xor     eax, eax
        ret
}

; in: ebp -> malloc_state
; in: [alignment] = stack var with required alignment
; in: [bytes] = stack var with number of bytes to allocate
macro do_memalign malloc_call
{
        mov     eax, [alignment]
        cmp     [alignment], MALLOC_ALIGNMENT
        jbe     .just_malloc
        lea     edx, [eax-1]
        cmp     eax, MIN_CHUNK_SIZE
        jb      .set_to_min_chunk_size
        test    eax, edx
        jnz     .set_align_pow2
.alignment_ok:
        add     eax, [bytes]
        jc      .too_large
        cmp     eax, MAX_REQUEST
        jae     .too_large
        mov     ecx, MIN_CHUNK_SIZE
        cmp     [bytes], MIN_REQUEST
        jb      @f
        mov     ecx, [bytes]
        add     ecx, CHUNK_OVERHEAD + CHUNK_ALIGN_MASK
        and     ecx, not CHUNK_ALIGN_MASK
@@:
        mov     [bytes], ecx
        add     ecx, [alignment]
        add     ecx, MIN_CHUNK_SIZE - CHUNK_OVERHEAD
        malloc_call ecx
        test    eax, eax
        jz      .nothing
        mov     esi, eax
        lock_heap
        mov     eax, esi
        mov     edx, [alignment]
        dec     edx
        test    esi, edx
        jz      .aligned
; Find an aligned spot inside chunk.  Since we need to give
; back leading space in a chunk of at least MIN_CHUNK_SIZE, if
; the first calculation places us at a spot with less than
; MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
; We've allocated enough total room so that this is always
; possible.
        not     edx
        lea     eax, [esi-1]
        add     eax, [alignment]
        and     eax, edx
        mov     edx, eax
        sub     edx, esi
        cmp     edx, MIN_CHUNK_SIZE
        jae     @f
        add     eax, [alignment]
        add     edx, [alignment]
@@:
        test    dword [esi+mchunk_head], INUSE_BITS
        jz      .unaligned_mmapped
; give back leader, use the rest
        mov     ecx, [esi+mchunk_head]
        and     ecx, not FLAG_BITS
        sub     ecx, edx
        mark_inuse_foot eax+ecx
        or      ecx, CINUSE_BIT
        mov     [eax+mchunk_head], ecx
        test    dword [esi+mchunk_head], PINUSE_BIT
        jnz     .insert_before
        mov     ebx, esi
        mov     edx, [esi+mchunk_prev_foot]
        sub     esi, [esi+mchunk_prev_foot]
        cmp     esi, [ebp+malloc_state.dv]
        jz      .append_dv
        push    eax
        mov     eax, esi
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     .append_large
        unlink_small_chunk      ; ebp, eax, edx
        jmp     .append_common
.append_large:
        mov     ecx, [ebx+tchunk_release_fd]
        mov     esi, [ebx+tchunk_release_bk]
        cmp     [ecx+tchunk_release_bk], ebx
        jnz     heap_corrupted
        cmp     [esi+tchunk_release_fd], ebx
        jnz     heap_corrupted
        mov     [ecx+tchunk_release_bk], esi
        mov     [esi+tchunk_release_fd], ecx
        unlink_large_chunk      ; ebp, eax
.append_common:
        mov     esi, eax
        mov     edx, [esp]
        pop     eax
        sub     edx, esi
.insert_before:
        lea     ecx, [edx+PINUSE_BIT]
        mov     [esi+mchunk_head], ecx
        mov     [eax+mchunk_prev_foot], edx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      .small_before
        lea     ebx, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     ecx, [ebp+malloc_state.release_list]
        cmp     [ecx+tchunk_release_bk], ebx
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_fd], eax
        mov     [ecx+tchunk_release_bk], eax
        mov     [eax+tchunk_release_fd], ecx
        mov     [eax+tchunk_release_bk], ebx
        push    edi
macro after_insert label \{
        pop     edi
        jmp     label
\}
        insert_large_chunk <after_insert .common_before>        ; ebp, esi, edx
.small_before:
        insert_small_chunk      ; ebp, esi, edx
.common_before:
.aligned:
; give back spare room at the end
        test    dword [eax+mchunk_head], INUSE_BITS
        jz      .done_after
        mov     ecx, [bytes]
        mov     edx, [eax+mchunk_head]
        mov     ebx, edx
        and     edx, not FLAG_BITS
        lea     esi, [eax+ecx]
        sub     edx, ecx
        test    dword [esi+edx+mchunk_head], CINUSE_BIT
        jz      @f
        cmp     edx, MIN_CHUNK_SIZE
        jb      .done_after
@@:
        and     ebx, INUSE_BITS
        add     ebx, ecx
        mov     [eax+mchunk_head], ebx
        mark_inuse_foot esi
        test    dword [esi+edx+mchunk_head], CINUSE_BIT
        jnz     .insert_after
        add     esi, edx
        cmp     esi, [ebp+malloc_state.dv]
        jz      .prepend_dv
        cmp     esi, [ebp+malloc_state.top]
        jz      .prepend_top
        mov     edx, [esi+mchunk_head]
        and     edx, not FLAG_BITS
        push    eax
        mov     eax, esi
        add     esi, edx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jae     .prepend_large
        unlink_small_chunk      ; ebp, eax, edx
        jmp     .prepend_common
.prepend_large:
        mov     ecx, [esi+tchunk_release_fd]
        mov     ebx, [esi+tchunk_release_bk]
        cmp     [ecx+tchunk_release_bk], esi
        jnz     heap_corrupted
        cmp     [ebx+tchunk_release_fd], esi
        jnz     heap_corrupted
        mov     [ecx+tchunk_release_bk], ebx
        mov     [ebx+tchunk_release_fd], ecx
        mov     ecx, esi
        unlink_large_chunk      ; ebp, eax
        mov     esi, ecx
.prepend_common:
        pop     eax
        mov     edx, esi
        mov     esi, [bytes]
        add     esi, eax
        sub     edx, esi
.insert_after:
        lea     ebx, [edx+PINUSE_BIT]
        mov     [esi+mchunk_head], ebx
        mov     [esi+edx+mchunk_prev_foot], edx
        and     dword [esi+edx+mchunk_head], not PINUSE_BIT
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      .small_after
        push    edi
        lea     ebx, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     ecx, [ebp+malloc_state.release_list]
        lea     edi, [esi+edx]
        cmp     [ecx+tchunk_release_bk], ebx
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_fd], edi
        mov     [ecx+tchunk_release_bk], edi
        mov     [edi+tchunk_release_fd], ecx
        mov     [edi+tchunk_release_bk], ebx
        insert_large_chunk <after_insert .done_after>   ; ebp, esi, edx
purge after_insert
.small_after:
        insert_small_chunk      ; ebp, esi, edx
.done_after:
        unlock_heap
        ret
.prepend_dv:
        sub     esi, edx
        add     edx, [ebp+malloc_state.dvsize]
        mov     [ebp+malloc_state.dv], esi
        lea     ecx, [edx+PINUSE_BIT]
        mov     [esi+mchunk_head], ecx
        mov     [esi+edx+mchunk_prev_foot], edx
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      .prepend_dv.norelease
        add     esi, edx
        cmp     [ebp+malloc_state.dvsize], NSMALLBINS shl SMALLBIN_SHIFT
        jb      @f
        cmp     [esi+tchunk_release_fd], esi
        jnz     .prepend_dv.norelease
@@:
        lea     ebx, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     ecx, [ebp+malloc_state.release_list]
        cmp     [ecx+tchunk_release_bk], ebx
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_fd], esi
        mov     [ecx+tchunk_release_bk], esi
        mov     [esi+tchunk_release_fd], ecx
        mov     [esi+tchunk_release_bk], ebx
.prepend_dv.norelease:
        mov     [ebp+malloc_state.dvsize], edx
        unlock_heap
        ret
.prepend_top:
        sub     esi, edx
        mov     [ebp+malloc_state.top], esi
        add     edx, [ebp+malloc_state.topsize]
        mov     [ebp+malloc_state.topsize], edx
assert PINUSE_BIT = 1
        inc     edx
        mov     [esi+mchunk_head], edx
        unlock_heap
        ret
.append_dv:
        mov     edx, eax
        sub     edx, esi
        cmp     edx, NSMALLBINS shl SMALLBIN_SHIFT
        jb      .append_dv.norelease
        cmp     [ebp+malloc_state.dvsize], NSMALLBINS shl SMALLBIN_SHIFT
        jb      @f
        cmp     [eax+tchunk_release_fd], eax
        jnz     .append_dv.norelease
@@:
        lea     ebx, [ebp+malloc_state.release_list-tchunk_release_fd]
        mov     ecx, [ebp+malloc_state.release_list]
        cmp     [ecx+tchunk_release_bk], ebx
        jnz     heap_corrupted
        mov     [ebx+tchunk_release_fd], eax
        mov     [ecx+tchunk_release_bk], eax
        mov     [eax+tchunk_release_fd], ecx
        mov     [eax+tchunk_release_bk], ebx
.append_dv.norelease:
        lea     ecx, [edx+PINUSE_BIT]
        mov     [ebp+malloc_state.dvsize], edx
        mov     [eax+mchunk_prev_foot], edx
        mov     [esi+mchunk_head], ecx
        jmp     .common_before
.unaligned_mmapped:
; For mmapped chunks, just adjust offset
        mov     ecx, [esi+mchunk_head]
        sub     ecx, edx
        add     edx, [esi+mchunk_prev_foot]
        mov     [eax+mchunk_prev_foot], edx
        mov     [eax+mchunk_head], ecx
        unlock_heap
        ret
.too_large:
        mov     FS_ERRNO, ENOMEM
        xor     eax, eax
.nothing:
        ret
.set_to_min_chunk_size:
        mov     eax, MIN_CHUNK_SIZE
        mov     [alignment], eax
        jmp     .alignment_ok
.set_align_pow2:
        bsr     ecx, eax
        mov     eax, 2
        shl     eax, cl
        mov     [alignment], eax
        jmp     .alignment_ok
.just_malloc:
        malloc_call [bytes]
        ret
}

macro do_malloc_trim
{
        ret
}

macro do_malloc_footprint
{
        mov     eax, [ebp+malloc_state.footprint]
        ret
}

macro do_malloc_max_footprint
{
        mov     eax, [ebp+malloc_state.max_footprint]
        ret
}

macro do_malloc_footprint_limit
{
        mov     eax, [ebp+malloc_state.footprint_limit]
        test    eax, eax
        jnz     @f
        dec     eax
@@:
        ret
}

macro do_malloc_set_footprint_limit
{
        mov     eax, [bytes]
        test    eax, eax
        jnz     @f
        inc     eax
@@:
        cmp     eax, -1
        jnz     @f
        inc     eax
@@:
        add     eax, 0xFFF
        and     eax, not 0xFFF
        mov     [ebp+malloc_state.footprint_limit], eax
        ret
}

macro do_mspace_mallopt
{
}

; ----------------------------- user mspaces ----------------------------

malloc_state_chunk_size = (sizeof.malloc_state + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) and not CHUNK_ALIGN_MASK
macro init_user_mstate
{
        mov     ebx, eax
        add     eax, 8 + ((-8) and CHUNK_ALIGN_MASK)
        mov     dword [eax+mchunk_head], malloc_state_chunk_size + INUSE_BITS
        mov     [eax+malloc_state.mmap_threshold], DEFAULT_MMAP_THRESHOLD
        mov     [eax+malloc_state.seg.base], ebx
        mov     [eax+malloc_state.seg.size], ecx
        mov     [eax+malloc_state.segment_size], ecx
        mov     [eax+malloc_state.footprint], ecx
        mov     [eax+malloc_state.max_footprint], ecx
        lea     ecx, [ecx+ebx+8-TOP_FOOT_SIZE]
if FOOTERS
        mov     ebx, [malloc_magic]
        mov     [eax+malloc_state.magic], ebx
end if
        mov     [eax+malloc_state.release_checks], RELEASE_CHECK_RATE
        init_bins       ; eax
        lea     ebx, [eax+malloc_state.release_list-tchunk_release_fd]
        mov     [eax+malloc_state.release_list+0], ebx
        mov     [eax+malloc_state.release_list+4], ebx
        lea     ebx, [eax+malloc_state.mmap_list]
        mov     [eax+malloc_state.mmap_list], ebx
        mov     [eax+malloc_state.mmap_list+4], ebx
        lea     ebx, [eax+malloc_state_chunk_size]
        sub     ecx, ebx
        init_top        ; eax, ebx, ecx
}

; in: [capacity] = stack var with number of bytes to allocate
; in: [locked] = stack var which is zero if locking is not needed
; out: eax -> allocated data
macro do_create_mspace
{
        mov     eax, 68
        mov     ebx, 12
        mov     ecx, [capacity]
        test    ecx, ecx
        jnz     @f
        mov     ecx, DEFAULT_MSPACE_SIZE
@@:
        add     ecx, 0xFFF
        and     ecx, not 0xFFF
        jz      .fail
        call    FS_SYSCALL_PTR
        test    eax, eax
        jz      .fail
        init_user_mstate
        and     [eax+malloc_state.mflags], not USE_LOCK_BIT
        cmp     [locked], 0
        jz      @f
        or      [eax+malloc_state.mflags], USE_LOCK_BIT
@@:
        ret
.fail:
        xor     eax, eax
        ret
}

; in: [msp] = stack var with mspace to free
macro do_destroy_mspace
{
        mov     edx, [msp]
if FOOTERS
        mov     ebx, [malloc_magic]
        cmp     [edx+malloc_state.magic], ebx
        jnz     heap_corrupted
end if
        add     edx, malloc_state.mmap_list
        push    edx
        mov     ecx, [edx]
        cmp     ecx, [esp]
        jz      .large_done
@@:
        mov     eax, 68
        mov     ebx, 13
        mov     edx, [ecx]
        call    FS_SYSCALL_PTR
        mov     ecx, edx
        cmp     edx, [esp]
        jnz     @b
.large_done:
        pop     edx
        add     edx, malloc_state.seg - malloc_state.mmap_list
.free_segments:
        mov     eax, 68
        mov     ebx, 13
        mov     ecx, [edx+malloc_segment.base]
        mov     edx, [edx+malloc_segment.next]
        call    FS_SYSCALL_PTR
        test    edx, edx
        jnz     .free_segments
        ret
}
