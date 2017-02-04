; deflate.inc -- internal compression state
; Copyright (C) 1995-2012 Jean-loup Gailly
; For conditions of distribution and use, see copyright notice in zlib.inc

; WARNING: this file should *not* be used by applications. It is
; part of the implementation of the compression library and is
; subject to change. Applications should only use zlib.inc.

include 'zutil.inc'

; ===========================================================================
; Internal compression state.


LENGTH_CODES equ 29
; number of length codes, not counting the special END_BLOCK code

LITERALS equ 256
; number of literal bytes 0..255

L_CODES equ (LITERALS+1+LENGTH_CODES)
; number of Literal or Length codes, including the END_BLOCK code

D_CODES  equ 30
; number of distance codes

BL_CODES equ 19
; number of codes used to transfer the bit lengths

HEAP_SIZE equ (2*L_CODES+1)
; maximum heap size

MAX_BITS equ 15
; All codes must not exceed MAX_BITS bits

Buf_size equ 16
; size of bit buffer in bi_buf

INIT_STATE   equ 42
EXTRA_STATE  equ 69
NAME_STATE   equ 73
COMMENT_STATE equ 91
HCRC_STATE   equ 103
BUSY_STATE   equ 113
FINISH_STATE equ 800
; Stream status

; Data structure describing a single value and its code string.
struct ct_data
	fc dw ? ;union
		;uint_16 freq ;frequency count
		;uint_16 code ;bit string
	dale dw ? ;union
		;uint_16 dad  ;father node in Huffman tree
		;uint_16 len  ;length of bit string
ends

Freq equ ct_data.fc ;.freq
Code equ ct_data.fc ;.code
Dad  equ ct_data.dale ;.dad
Len  equ ct_data.dale ;.len

struct tree_desc
	dyn_tree  dd ? ;ct_data * ;the dynamic tree
	max_code  dd ? ;int ;largest code with non zero frequency
	stat_desc dd ? ;static_tree_desc * ;the corresponding static tree
ends

; A Pos is an index in the character window. We use short instead of int to
; save space in the various tables. IPos is used only for parameter passing.

struct deflate_state ;internal_state
	strm   dd ? ;z_streamp ;pointer back to this zlib stream
	status dd ? ;int ;as the name implies
	pending_buf dd ? ;Bytef *;output still pending
	pending_buf_size dd ? ;ulg ;size of pending_buf
	pending_out dd ? ;Bytef * ;next pending byte to output to the stream
	pending dd ? ;uInt ;nb of bytes in the pending buffer
	wrap    dd ? ;int ;bit 0 true for zlib, bit 1 true for gzip
	gzhead  dd ? ;gz_headerp ;gzip header information to write
	gzindex dd ? ;uInt ;where in extra, name, or comment
	method  db ? ;Byte ;can only be DEFLATED
		rb 3 ;for align
	last_flush dd ? ;int ;value of flush param for previous deflate call

; used by deflate.asm:

	w_size dd ? ;uInt ;LZ77 window size (32K by default)
	w_bits dd ? ;uInt ;log2(w_size)  (8..16)
	w_mask dd ? ;uInt ;w_size - 1

	window dd ? ;Bytef *
	; Sliding window. Input bytes are read into the second half of the window,
	; and move to the first half later to keep a dictionary of at least wSize
	; bytes. With this organization, matches are limited to a distance of
	; wSize-MAX_MATCH bytes, but this ensures that IO is always
	; performed with a length multiple of the block size. Also, it limits
	; the window size to 64K, which is quite useful on MSDOS.
	; To do: use the user input buffer as sliding window.

	window_size dd ? ;ulg
	; Actual size of window: 2*wSize, except when the user input buffer
	; is directly used as sliding window.

	prev dd ? ;Posf *
	; Link to older string with same hash index. To limit the size of this
	; array to 64K, this link is maintained only for the last 32K strings.
	; An index in this array is thus a window index modulo 32K.

	head      dd ? ;Posf * ;Heads of the hash chains or NIL.

	ins_h     dd ? ;uInt ;hash index of string to be inserted
	hash_size dd ? ;uInt ;number of elements in hash table
	hash_bits dd ? ;uInt ;log2(hash_size)
	hash_mask dd ? ;uInt ;hash_size-1

	hash_shift dd ? ;uInt
	; Number of bits by which ins_h must be shifted at each input
	; step. It must be such that after MIN_MATCH steps, the oldest
	; byte no longer takes part in the hash key, that is:
	;   hash_shift * MIN_MATCH >= hash_bits

	block_start dd ? ;long
	; Window position at the beginning of the current output block. Gets
	; negative when the window is moved backwards.

	match_length dd ? ;uInt ;length of best match
	prev_match   dd ? ;IPos ;previous match
	match_available dd ? ;int ;set if previous match exists
	strstart     dd ? ;uInt ;start of string to insert
	match_start  dd ? ;uInt ;start of matching string
	lookahead    dd ? ;uInt ;number of valid bytes ahead in window

	prev_length dd ? ;uInt
	; Length of the best match at previous step. Matches not greater than this
	; are discarded. This is used in the lazy match evaluation.

	max_chain_length dd ? ;uInt
	; To speed up deflation, hash chains are never searched beyond this
	; length.  A higher limit improves compression ratio but degrades the
	; speed.

	max_lazy_match dd ? ;uInt
	; Attempt to find a better match only when the current match is strictly
	; smaller than this value. This mechanism is used only for compression
	; levels >= 4.

	level dw ? ;int ;compression level (1..9)
		rb 2 ;for align
	strategy dw ? ;int ;favor or force Huffman coding
		rb 2 ;for align

	good_match dd ? ;uInt
	; Use a faster search when the previous match is longer than this

	nice_match dd ? ;int ;Stop searching when current match exceeds this

; used by trees.asm:
	; Didn't use ct_data typedef below to suppress compiler warning
	dyn_ltree rb sizeof.ct_data * HEAP_SIZE ;literal and length tree
	dyn_dtree rb sizeof.ct_data * (2*D_CODES+1) ;distance tree
	bl_tree   rb sizeof.ct_data * (2*BL_CODES+1) ;Huffman tree for bit lengths

	l_desc  tree_desc ;desc. for literal tree
	d_desc  tree_desc ;desc. for distance tree
	bl_desc tree_desc ;desc. for bit length tree

	bl_count rw MAX_BITS+1 ;uint_16[]
	; number of codes at each bit length for an optimal tree

	heap     rd 2*L_CODES+1 ;int[] ;heap used to build the Huffman trees
	heap_len dd ? ;int ;number of elements in the heap
	heap_max dd ? ;int ;element of largest frequency
	; The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
	; The same heap array is used to build all trees.

	depth rb ((2*L_CODES+1)+3) and (not 3) ;uch[]
	; Depth of each subtree used as tie breaker for trees of equal frequency

	l_buf dd ? ;uchf * ;buffer for literals or lengths

	lit_bufsize dd ? ;uInt
	; Size of match buffer for literals/lengths.  There are 4 reasons for
	; limiting lit_bufsize to 64K:
	;   - frequencies can be kept in 16 bit counters
	;   - if compression is not successful for the first block, all input
	;     data is still in the window so we can still emit a stored block even
	;     when input comes from standard input.  (This can also be done for
	;     all blocks if lit_bufsize is not greater than 32K.)
	;   - if compression is not successful for a file smaller than 64K, we can
	;     even emit a stored file instead of a stored block (saving 5 bytes).
	;     This is applicable only for zip (not gzip or zlib).
	;   - creating new Huffman trees less frequently may not provide fast
	;     adaptation to changes in the input data statistics. (Take for
	;     example a binary file with poorly compressible code followed by
	;     a highly compressible string table.) Smaller buffer sizes give
	;     fast adaptation but have of course the overhead of transmitting
	;     trees more frequently.
	;   - I can't count above 4

	last_lit dd ? ;uInt ;running index in l_buf

	d_buf dd ? ;uint_16p
	; Buffer for distances. To simplify the code, d_buf and l_buf have
	; the same number of elements. To use different lengths, an extra flag
	; array would be necessary.

	opt_len dd ? ;ulg ;bit length of current block with optimal trees
	static_len dd ? ;ulg ;bit length of current block with static trees
	matches dd ? ;uInt ;number of string matches in current block
	insert  dd ? ;uInt ;bytes at end of window left to insert

if DEBUG eq 1
	;compressed_len dd ? ;ulg ;total bit length of compressed file mod 2^32
	;bits_sent      dd ? ;ulg ;bit length of compressed data sent mod 2^32
end if

	bi_buf dw ? ;uint_16
		rb 2 ;for align
	; Output buffer. bits are inserted starting at the bottom (least
	; significant bits).

	bi_valid dd ? ;int
	; Number of valid bits in bi_buf.  All bits above the last valid bit
	; are always zero.

	high_water dd ? ;ulg
	; High water mark offset in window for initialized bytes -- bytes above
	; this are set to zero in order to avoid memory check warnings when
	; longest match routines access bytes past the input.  This is then
	; updated to the new high water mark.
ends

deflate_state.max_insert_length equ deflate_state.max_lazy_match
; Insert new strings in the hash table only if the match length is not
; greater than this length. This saves time but degrades compression.
; max_insert_length is used only for compression levels <= 3.

; Output a byte on the stream.
; IN assertion: there is enough room in pending_buf.

macro put_byte s, c
{
	mov eax,[s+deflate_state.pending]
	add eax,[s+deflate_state.pending_buf]
	mov byte[eax],c
	inc dword[s+deflate_state.pending]
}
macro put_dword s, d
{
	mov eax,[s+deflate_state.pending]
	add eax,[s+deflate_state.pending_buf]
	mov dword[eax],d
	add dword[s+deflate_state.pending],4
}

MIN_LOOKAHEAD equ (MAX_MATCH+MIN_MATCH+1)
; Minimum amount of lookahead, except at the end of the input file.
; See deflate.asm for comments about the MIN_MATCH+1.

macro MAX_DIST s
{
	mov eax,[s+deflate_state.w_size]
	sub eax,MIN_LOOKAHEAD
}
; In order to simplify the code, particularly on 16 bit machines, match
; distances are limited to MAX_DIST instead of WSIZE.

WIN_INIT equ MAX_MATCH
; Number of bytes after end of data in window to initialize in order to avoid
; memory checker errors from longest match routines

macro d_code dist
{
;if (dist < 256) _dist_code[dist]
;else _dist_code[ 256+(dist>>7) ]
local .end0
if dist eq eax
else
	mov eax,dist
end if
	cmp eax,256
	jb .end0
		shr eax,7
		add eax,256
	.end0:
	movzx eax,byte[eax+_dist_code]
}
; Mapping from a distance to a distance code. dist is the distance - 1 and
; must not have side effects. _dist_code[256] and _dist_code[257] are never
; used.

macro _tr_tally_lit s, c, flush
{
if DEBUG eq 0
; Inline versions of _tr_tally for speed:
if c eq eax
else
	mov eax,c
end if
	push ecx
	mov ecx,[s+deflate_state.last_lit]
	shl ecx,1
	add ecx,[s+deflate_state.d_buf]
	mov word[ecx],0
	mov ecx,[s+deflate_state.last_lit]
	add ecx,[s+deflate_state.l_buf]
	mov byte[ecx],al
	inc dword[s+deflate_state.last_lit]
	and eax,0xff
	inc word[s+sizeof.ct_data*eax+deflate_state.dyn_ltree+Freq]
	xor eax,eax
	mov ecx,[s+deflate_state.lit_bufsize]
	dec ecx
	cmp [s+deflate_state.last_lit],ecx
	sete al ;flush = (..==..)
	mov flush, eax
	pop ecx
else
	stdcall _tr_tally, s, 0, c
	mov flush, eax
end if
}
macro _tr_tally_dist s, distance, length, flush
{
if DEBUG eq 0
	push ecx

    ;s.d_buf[s.last_lit] = dist
	mov ecx,[s+deflate_state.last_lit]
	shl ecx,1
	add ecx,[s+deflate_state.d_buf]
if distance eq eax
	mov [ecx],ax
else
	mov word[ecx],distance
end if

    ;s.l_buf[s.last_lit++] = len
	mov ecx,[s+deflate_state.last_lit]
	add ecx,[s+deflate_state.l_buf]
if length eq eax
	mov [ecx],al
else if length eq ebx
	mov [ecx],bl
else
	... ;mov byte[ecx],length
end if
	inc dword[s+deflate_state.last_lit]

	;dist--
if distance eq eax
else
	mov eax,distance
end if
	dec eax

	;s.dyn_ltree[_length_code[len]+LITERALS+1].Freq++
	movzx ecx,byte[ecx]
	movzx ecx,byte[ecx+_length_code]
	inc word[s+sizeof.ct_data*ecx+deflate_state.dyn_ltree+sizeof.ct_data*(LITERALS+1)+Freq]

	;s.dyn_dtree[d_code(dist)].Freq++
	d_code eax
	inc word[s+sizeof.ct_data*eax+deflate_state.dyn_dtree+Freq]

	;flush = (s.last_lit == s.lit_bufsize-1)
	mov ecx,[s+deflate_state.lit_bufsize]
	dec ecx
	xor eax,eax
	cmp [s+deflate_state.last_lit],ecx
	sete al
	mov flush,eax
	pop ecx
else
	stdcall _tr_tally, s, distance, length
	mov flush, eax
end if
}
