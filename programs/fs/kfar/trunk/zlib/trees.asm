; trees.asm -- output deflated data using Huffman coding
; Copyright (C) 1995-2012 Jean-loup Gailly
; detect_data_type() function provided freely by Cosmin Truta, 2006
; For conditions of distribution and use, see copyright notice in zlib.inc

;  ALGORITHM

;      The "deflation" process uses several Huffman trees. The more
;      common source values are represented by shorter bit sequences.

;      Each code tree is stored in a compressed form which is itself
; a Huffman encoding of the lengths of all the code strings (in
; ascending order by source values).  The actual code strings are
; reconstructed from the lengths in the inflate process, as described
; in the deflate specification.

;  REFERENCES

;      Deutsch, L.P.,"'Deflate' Compressed Data Format Specification".
;      Available in ftp.uu.net:/pub/archiving/zip/doc/deflate-1.1.doc

;      Storer, James A.
;          Data Compression:  Methods and Theory, pp. 49-50.
;          Computer Science Press, 1988.  ISBN 0-7167-8156-5.

;      Sedgewick, R.
;          Algorithms, p290.
;          Addison-Wesley, 1983. ISBN 0-201-06672-6.

; ===========================================================================
; Constants


MAX_BL_BITS equ 7
; Bit length codes must not exceed MAX_BL_BITS bits

END_BLOCK equ 256
; end of block literal code

REP_3_6     equ 16
; repeat previous bit length 3-6 times (2 bits of repeat count)

REPZ_3_10   equ 17
; repeat a zero length 3-10 times  (3 bits of repeat count)

REPZ_11_138 equ 18
; repeat a zero length 11-138 times  (7 bits of repeat count)

align 4
extra_lbits dd \ ;int [LENGTH_CODES] ;extra bits for each length code
	0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0

align 4
extra_dbits dd \ ;int [D_CODES] ;extra bits for each distance code
	0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13

align 4
extra_blbits dd \ ;int [BL_CODES] ;extra bits for each bit length code
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7

align 4
bl_order db 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15
; The lengths of the bit length codes are sent in order of decreasing
; probability, to avoid transmitting the lengths for unused bit length codes.


; ===========================================================================
; Local data. These are initialized only once.


DIST_CODE_LEN equ 512 ;see definition of array dist_code below

if GEN_TREES_H eq 1 ;| !(STDC)
; non ANSI compilers may not accept trees.inc

align 4
static_ltree rb sizeof.ct_data * (L_CODES+2)
; The static literal tree. Since the bit lengths are imposed, there is no
; need for the L_CODES extra codes used during heap construction. However
; The codes 286 and 287 are needed to build a canonical tree (see _tr_init
; below).

align 4
static_dtree rb sizeof.ct_data * D_CODES
; The static distance tree. (Actually a trivial tree since all codes use
; 5 bits.)

align 4
_dist_code rb DIST_CODE_LEN ;uch[]
; Distance codes. The first 256 values correspond to the distances
; 3 .. 258, the last 256 values correspond to the top 8 bits of
; the 15 bit distances.

align 4
_length_code rb MAX_MATCH-MIN_MATCH+1 ;uch[]
; length code for each normalized match length (0 == MIN_MATCH)

align 4
base_length rd LENGTH_CODES ;int[]
; First normalized length for each code (0 = MIN_MATCH)

align 4
base_dist rd D_CODES ;int[]
; First normalized distance for each code (0 = distance of 1)

else
include 'trees.inc'
end if ;GEN_TREES_H

struct static_tree_desc
	static_tree dd ? ;const ct_data * ;static tree or NULL
	extra_bits  dd ? ;const intf * ;extra bits for each code or NULL
	extra_base  dd ? ;int ;base index for extra_bits
	elems       dd ? ;int ;max number of elements in the tree
	max_length  dd ? ;int ;max bit length for the codes
ends

align 4
static_l_desc static_tree_desc static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_BITS

align 4
static_d_desc static_tree_desc static_dtree, extra_dbits, 0, D_CODES, MAX_BITS

align 4
static_bl_desc static_tree_desc 0, extra_blbits, 0, BL_CODES, MAX_BL_BITS

; ===========================================================================
; Local (static) routines in this file.


macro send_code s, c, tree
{
if DEBUG eq 1
;	if (z_verbose>2) fprintf(stderr,"\ncd %3d ",(c))
end if
push eax ebx
if c eq eax
else
	mov eax,c
end if
	imul eax,sizeof.ct_data
	add eax,tree
	movzx ebx,word[eax+Len]
	push ebx
	movzx ebx,word[eax+Code]
	push ebx
	stdcall send_bits, s ;tree[c].Code, tree[c].Len
pop ebx eax
}
; Send a code of the given tree[c] and tree must not have side effects

; ===========================================================================
; Output a short LSB first on the stream.
; IN assertion: there is enough room in pendingBuf.

macro put_short s, w
{
	mov eax,[s+deflate_state.pending]
	add eax,[s+deflate_state.pending_buf]
	mov word[eax],w
	add dword[s+deflate_state.pending],2
}

; ===========================================================================
; Send a value on a given number of bits.
; IN assertion: length <= 16 and value fits in length bits.

;void (s, value, length)
;    deflate_state* s
;    int value  ;value to send
;    int length ;number of bits
align 4
proc send_bits uses eax ecx edi, s:dword, value:dword, length:dword
;    Tracevv((stderr," l %2d v %4x ", length, value));
;if DEBUG eq 1
	mov eax,[length]
	cmp eax,0
	jle @f
	cmp eax,15
	jle .end1
	@@:
		zlib_assert 'invalid length' ;Assert(..>0 && ..<=15)
	.end1:
	mov edi,[s]
	;;add [edi+deflate_state.bits_sent],eax

	; If not enough room in bi_buf, use (valid) bits from bi_buf and
	; (16 - bi_valid) bits from value, leaving (width - (16-bi_valid))
	; unused bits in value.

	mov ecx,Buf_size
	sub ecx,eax
	cmp [edi+deflate_state.bi_valid],ecx
	jle @f ;if (..>..)
		mov eax,[value]
		mov ecx,[edi+deflate_state.bi_valid]
		shl eax,cl
		or [edi+deflate_state.bi_buf],ax
		mov cx,[edi+deflate_state.bi_buf]
		put_short edi, cx
		mov eax,[value]
		mov ecx,Buf_size
		sub ecx,[edi+deflate_state.bi_valid]
		sar eax,cl
		mov [edi+deflate_state.bi_buf],ax
		mov eax,[length]
		sub eax,Buf_size
		jmp .end0
	@@: ;else
		mov eax,[value]
		mov ecx,[edi+deflate_state.bi_valid]
		shl eax,cl
		or [edi+deflate_state.bi_buf],ax
		mov eax,[length]
	.end0:
	add [edi+deflate_state.bi_valid],eax
;else ;!DEBUG

;{ int len = length;
;  if (s->bi_valid > (int)Buf_size - len) {
;    int val = value;
;    s->bi_buf |= (uint_16)val << s->bi_valid;
;    put_short(s, s->bi_buf);
;    s->bi_buf = (uint_16)val >> (Buf_size - s->bi_valid);
;    s->bi_valid += len - Buf_size;
;  } else {
;    s->bi_buf |= (uint_16)(value) << s->bi_valid;
;    s->bi_valid += len;
;  }
;}
;end if ;DEBUG
	ret
endp

; the arguments must not have side effects

; ===========================================================================
; Initialize the various 'constant' tables.

;int static_init_done = 0

;void ()
align 4
proc tr_static_init
if GEN_TREES_H eq 1

;    int n      ;iterates over tree elements
;    int bits   ;bit counter
;    int length ;length value
;    int code   ;code value
;    int dist   ;distance index
;    uint_16 bl_count[MAX_BITS+1];
	; number of codes at each bit length for an optimal tree

;    if (static_init_done) return;

	; For some embedded targets, global variables are not initialized:
;if NO_INIT_GLOBAL_POINTERS
;    static_l_desc.static_tree = static_ltree;
;    static_l_desc.extra_bits = extra_lbits;
;    static_d_desc.static_tree = static_dtree;
;    static_d_desc.extra_bits = extra_dbits;
;    static_bl_desc.extra_bits = extra_blbits;
;end if

	; Initialize the mapping length (0..255) -> length code (0..28)
;    length = 0;
;    for (code = 0; code < LENGTH_CODES-1; code++) {
;        base_length[code] = length;
;        for (n = 0; n < (1<<extra_lbits[code]); n++) {
;            _length_code[length++] = (uch)code;
;        }
;    }
;    Assert (length == 256, "tr_static_init: length != 256");
	; Note that the length 255 (match length 258) can be represented
	; in two different ways: code 284 + 5 bits or code 285, so we
	; overwrite length_code[255] to use the best encoding:

;    _length_code[length-1] = (uch)code;

	; Initialize the mapping dist (0..32K) -> dist code (0..29)
;    dist = 0;
;    for (code = 0 ; code < 16; code++) {
;        base_dist[code] = dist;
;        for (n = 0; n < (1<<extra_dbits[code]); n++) {
;            _dist_code[dist++] = (uch)code;
;        }
;    }
;    Assert (dist == 256, "tr_static_init: dist != 256");
;    dist >>= 7; /* from now on, all distances are divided by 128 */
;    for ( ; code < D_CODES; code++) {
;        base_dist[code] = dist << 7;
;        for (n = 0; n < (1<<(extra_dbits[code]-7)); n++) {
;            _dist_code[256 + dist++] = (uch)code;
;        }
;    }
;    Assert (dist == 256, "tr_static_init: 256+dist != 512");

	; Construct the codes of the static literal tree
;    for (bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;
;    n = 0;
;    while (n <= 143) static_ltree[n++].Len = 8, bl_count[8]++;
;    while (n <= 255) static_ltree[n++].Len = 9, bl_count[9]++;
;    while (n <= 279) static_ltree[n++].Len = 7, bl_count[7]++;
;    while (n <= 287) static_ltree[n++].Len = 8, bl_count[8]++;
	; Codes 286 and 287 do not exist, but we must include them in the
	; tree construction to get a canonical Huffman tree (longest code
	; all ones)

;    gen_codes((ct_data *)static_ltree, L_CODES+1, bl_count);

	; The static distance tree is trivial:
;    for (n = 0; n < D_CODES; n++) {
;        static_dtree[n].Len = 5;
;        static_dtree[n].Code = bi_reverse((unsigned)n, 5);
;    }
;    static_init_done = 1;

if GEN_TREES_H eq 1
	call gen_trees_header
end if
end if ;(GEN_TREES_H) | !(STDC)
	ret
endp

; ===========================================================================
; Genererate the file trees.inc describing the static trees.

;#  define SEPARATOR(i, last, width) \
;      ((i) == (last)? "\n};\n\n" :    \
;       ((i) % (width) == (width)-1 ? ",\n" : ", "))

;void ()
align 4
proc gen_trees_header
;    FILE *header = fopen("trees.inc", "w");
;    int i;

;    Assert (header != NULL, "Can't open trees.inc");
;    fprintf(header,
;            "/* header created automatically with -DGEN_TREES_H */\n\n");

;    fprintf(header, "local const ct_data static_ltree[L_CODES+2] = {\n");
;    for (i = 0; i < L_CODES+2; i++) {
;        fprintf(header, "{{%3u},{%3u}}%s", static_ltree[i].Code,
;                static_ltree[i].Len, SEPARATOR(i, L_CODES+1, 5));
;    }

;    fprintf(header, "local const ct_data static_dtree[D_CODES] = {\n");
;    for (i = 0; i < D_CODES; i++) {
;        fprintf(header, "{{%2u},{%2u}}%s", static_dtree[i].Code,
;                static_dtree[i].Len, SEPARATOR(i, D_CODES-1, 5));
;    }

;    fprintf(header, "const uch ZLIB_INTERNAL _dist_code[DIST_CODE_LEN] = {\n");
;    for (i = 0; i < DIST_CODE_LEN; i++) {
;        fprintf(header, "%2u%s", _dist_code[i],
;                SEPARATOR(i, DIST_CODE_LEN-1, 20));
;    }

;    fprintf(header,
;        "const uch ZLIB_INTERNAL _length_code[MAX_MATCH-MIN_MATCH+1]= {\n");
;    for (i = 0; i < MAX_MATCH-MIN_MATCH+1; i++) {
;        fprintf(header, "%2u%s", _length_code[i],
;                SEPARATOR(i, MAX_MATCH-MIN_MATCH, 20));
;    }

;    fprintf(header, "local const int base_length[LENGTH_CODES] = {\n");
;    for (i = 0; i < LENGTH_CODES; i++) {
;        fprintf(header, "%1u%s", base_length[i],
;                SEPARATOR(i, LENGTH_CODES-1, 20));
;    }

;    fprintf(header, "local const int base_dist[D_CODES] = {\n");
;    for (i = 0; i < D_CODES; i++) {
;        fprintf(header, "%5u%s", base_dist[i],
;                SEPARATOR(i, D_CODES-1, 10));
;    }

;    fclose(header);
	ret
endp

; ===========================================================================
; Initialize the tree data structures for a new zlib stream.

;void (deflate_state* s)
align 4
proc _tr_init uses eax edi, s:dword
	mov edi,[s]
	call tr_static_init

	mov eax,edi
	add eax,deflate_state.dyn_ltree
	mov [edi+deflate_state.l_desc.dyn_tree],eax
	mov [edi+deflate_state.l_desc.stat_desc],static_l_desc

	add eax,deflate_state.dyn_dtree-deflate_state.dyn_ltree
	mov [edi+deflate_state.d_desc.dyn_tree],eax
	mov [edi+deflate_state.d_desc.stat_desc],static_d_desc

	add eax,deflate_state.bl_tree-deflate_state.dyn_dtree
	mov [edi+deflate_state.bl_desc.dyn_tree],eax
	mov [edi+deflate_state.bl_desc.stat_desc],static_bl_desc;

	mov word[edi+deflate_state.bi_buf],0
	mov dword[edi+deflate_state.bi_valid],0
if DEBUG eq 1
	mov dword[edi+deflate_state.compressed_len],0
	mov dword[edi+deflate_state.bits_sent],0
end if

	; Initialize the first block of the first file:
	stdcall init_block,edi
	ret
endp

; ===========================================================================
; Initialize a new block.

;void (deflate_state* s)
align 4
proc init_block uses eax ecx edi, s:dword
	mov edi,[s]

	; Initialize the trees.
	mov eax,edi
	add eax,deflate_state.dyn_ltree+Freq
	mov ecx,L_CODES
	@@:
		mov word[eax],0
		add eax,sizeof.ct_data
		loop @b
	mov eax,edi
	add eax,deflate_state.dyn_dtree+Freq
	mov ecx,D_CODES
	@@:
		mov word[eax],0
		add eax,sizeof.ct_data
		loop @b
	mov eax,edi
	add eax,deflate_state.bl_tree+Freq
	mov ecx,BL_CODES
	@@:
		mov word[eax],0
		add eax,sizeof.ct_data
		loop @b

	mov word[edi+sizeof.ct_data*END_BLOCK+deflate_state.dyn_ltree+Freq],1
	mov dword[edi+deflate_state.static_len],0
	mov dword[edi+deflate_state.opt_len],0
	mov dword[edi+deflate_state.matches],0
	mov dword[edi+deflate_state.last_lit],0
	ret
endp

SMALLEST equ 1
; Index within the heap array of least frequent node in the Huffman tree


; ===========================================================================
; Remove the smallest element from the heap and recreate the heap with
; one less element. Updates heap and heap_len.

macro pqremove s, tree, top
{
	mov eax,s
	add eax,deflate_state.heap+4*SMALLEST
	movzx top,word[eax]
push ebx
	mov ebx,[s+deflate_state.heap_len]
	mov ebx,[s+deflate_state.heap+4*ebx]
	mov [eax],ebx
	dec dword[s+deflate_state.heap_len]
pop ebx
	stdcall pqdownheap, s, tree, SMALLEST
}

; ===========================================================================
; Compares to subtrees, using the tree depth as tie breaker when
; the subtrees have equal frequency. This minimizes the worst case length.

macro smaller tree, n, m, depth, m_end
{
;if (..<.. || (..==.. && depth[n] <= depth[m]))
local .end0
	mov eax,n
	imul eax,sizeof.ct_data
	add eax,tree
	mov ax,word[eax+Freq]
	mov ebx,m
	imul ebx,sizeof.ct_data
	add ebx,tree
	cmp ax,word[ebx+Freq]
	jl .end0
	jne m_end
	mov eax,n
	mov al,byte[eax+depth]
	mov ebx,m
	cmp al,byte[ebx+depth]
	jg m_end
	.end0:
}

; ===========================================================================
; Restore the heap property by moving down the tree starting at node k,
; exchanging a node with the smallest of its two sons if necessary, stopping
; when the heap property is re-established (each father smaller than its
; two sons).

;void (s, tree, k)
;    deflate_state* s
;    ct_data* tree ;the tree to restore
;    int      k    ;node to move down
align 4
proc pqdownheap, s:dword, tree:dword, k:dword
pushad
	;ecx - v dw
	mov edi,[s]
	mov esi,[k]
	mov ecx,[edi+deflate_state.heap+4*esi]
	shl esi,1
	;esi = j ;left son of k
	.cycle0: ;while (..<=..)
		cmp esi,[edi+deflate_state.heap_len]
		jg .cycle0end
		; Set j to the smallest of the two sons:
		;;cmp esi,[edi+deflate_state.heap_len]
		jge .end1 ;if (..<.. &&
		lea edx,[edi+4*esi+deflate_state.heap]
		smaller [tree], dword[edx+4], dword[edx], edi+deflate_state.depth, .end1
			inc esi
		.end1:
		; Exit if v is smaller than both sons
		mov edx,[edi+deflate_state.heap+4*esi]
		smaller [tree], ecx, edx, edi+deflate_state.depth, .end2
			jmp .cycle0end ;break
		.end2:
		; Exchange v with the smallest son
		;;mov dx,[edi+deflate_state.heap+2*esi]
		mov eax,[k]
		mov [edi+deflate_state.heap+4*eax],edx
		mov [k],esi
		; And continue down the tree, setting j to the left son of k
		shl esi,1
		jmp .cycle0
align 4
	.cycle0end:
	mov eax,[k]
	mov [edi+deflate_state.heap+4*eax],ecx
popad
	ret
endp

; ===========================================================================
; Compute the optimal bit lengths for a tree and update the total bit length
; for the current block.
; IN assertion: the fields freq and dad are set, heap[heap_max] and
;    above are the tree nodes sorted by increasing frequency.
; OUT assertions: the field len is set to the optimal bit length, the
;     array bl_count contains the frequencies for each bit length.
;     The length opt_len is updated; static_len is also updated if stree is
;     not null.

;void (deflate_state* s, tree_desc* desc)
align 16
proc gen_bitlen, s:dword, desc:dword
locals
	tree  dd ? ;ct_data* ;= desc.dyn_tree
	max_code dd ? ;int   ;= desc.max_code
	stree dd ? ;ct_data* ;= desc.stat_desc.static_tree
	extra dd ? ;intf*    ;= desc.stat_desc.extra_bits
	base  dd ? ;int      ;= desc.stat_desc.extra_base
	max_length dd ? ;int ;= desc.stat_desc.max_length
	h     dd ? ;int ;heap index
	m     dd ? ;int ;iterate over the tree elements
	bits  dd ? ;int ;bit length
	xbits dd ? ;int ;extra bits
	f     dw ? ;uint_16 ;frequency
	overflow dd 0 ;int ;number of elements with bit length too large
endl
pushad
	mov edi,[s]
	mov edx,[desc]
	mov eax,[edx+tree_desc.dyn_tree]
	mov [tree],eax
	mov eax,[edx+tree_desc.max_code]
	mov [max_code],eax
	mov ebx,[edx+tree_desc.stat_desc]
	mov eax,[ebx+static_tree_desc.static_tree]
	mov [stree],eax
	mov eax,[ebx+static_tree_desc.extra_bits]
	mov [extra],eax
	mov eax,[ebx+static_tree_desc.extra_base]
	mov [base],eax
	mov eax,[ebx+static_tree_desc.max_length]
	mov [max_length],eax

	xor ecx,ecx
	.cycle0:
	cmp ecx,MAX_BITS
	jg .cycle0end ;for (..;..<=..;..)
		mov word[edi+deflate_state.bl_count+2*ecx],0
		inc ecx
		jmp .cycle0
align 4
	.cycle0end:

	; In a first pass, compute the optimal bit lengths (which may
	; overflow in the case of the bit length tree).

	mov eax,[edi+deflate_state.heap_max]
	mov eax,[edi+deflate_state.heap+4*eax]
	imul eax,sizeof.ct_data
	add eax,[tree]
	mov word[eax+Len],0 ;root of the heap

	mov eax,[edi+deflate_state.heap_max]
	inc eax
	mov [h],eax
	jmp @f
align 4
	.cycle1:
	inc dword[h]
	@@:
	cmp dword[h],HEAP_SIZE
	jge .cycle1end ;for (..;..<..;..)
		mov eax,[h]
		mov ecx,[edi+4*eax+deflate_state.heap]
		;ecx = n
		mov edx,[tree]
		movzx eax,word[edx+sizeof.ct_data*ecx+Dad]
		movzx eax,word[edx+sizeof.ct_data*eax+Len]
		inc eax
		mov [bits],eax ;bits = tree[tree[n].Dad].Len + 1
		cmp eax,[max_length]
		jle @f ;if (..>..)
			mov eax,[max_length]
			mov [bits],eax
			inc dword[overflow]
		@@:
		mov [edx+sizeof.ct_data*ecx+Len],ax
		; We overwrite tree[n].Dad which is no longer needed

		cmp ecx,[max_code]
		jg .cycle1 ;if (..>..) continue ;not a leaf node

		inc word[edi+2*eax+deflate_state.bl_count]
		mov dword[xbits],0
		cmp ecx,[base]
		jl @f ;if (..>=..)
			mov eax,ecx
			sub eax,[base]
			shl eax,2 ;*= sizeof.dd
			add eax,[extra]
			mov eax,[eax]
			mov [xbits],eax
		@@:
		movzx eax,word[edx+sizeof.ct_data*ecx+Freq]
		mov [f],ax
		mov esi,[bits]
		add esi,[xbits]
		imul eax,esi
		add [edi+deflate_state.opt_len],eax
		cmp dword[stree],0
		je .cycle1 ;if (..)
			movzx eax,word[f]
			mov esi,[stree]
			movzx esi,word[esi+sizeof.ct_data*ecx+Len]
			add esi,[xbits]
			imul eax,esi
			add [edi+deflate_state.static_len],eax
		jmp .cycle1
align 4
	.cycle1end:
	cmp dword[overflow],0
	je .end_f ;if (..==0) return

;    Trace((stderr,"\nbit length overflow\n"));
	; This happens for example on obj2 and pic of the Calgary corpus

	; Find the first bit length which could increase:
	.cycle2: ;do
		mov eax,[max_length]
		dec eax
		mov [bits],eax
		shl eax,1 ;*= sizeof.dw
		add eax,edi
		add eax,deflate_state.bl_count
		@@:
		cmp word[eax],0
		jne @f ;while (..==0) bits--
			dec dword[bits]
			sub eax,2
			jmp @b
align 4
		@@:
		dec word[eax]     ;move one leaf down the tree
		add word[eax+2],2 ;move one overflow item as its brother
		mov eax,[max_length]
		dec word[edi+deflate_state.bl_count+2*eax]
		; The brother of the overflow item also moves one step up,
		; but this does not affect bl_count[max_length]

		sub dword[overflow],2
		cmp dword[overflow],0
		jg .cycle2 ;while (..>0)

	; Now recompute all bit lengths, scanning in increasing frequency.
	; h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
	; lengths instead of fixing only the wrong ones. This idea is taken
	; from 'ar' written by Haruhiko Okumura.)

	mov eax,[max_length]
	mov [bits],eax
	.cycle3:
	cmp dword[bits],0
	je .end_f ;for (..;..!=0;..)
		mov eax,[bits]
		movzx ecx,word[edi+2*eax+deflate_state.bl_count]
		.cycle4: ;while (..!=0)
		test ecx,ecx
		jz .cycle4end
			dec dword[h]
			mov eax,[h]
			mov eax,[edi+4*eax+deflate_state.heap]
			mov [m],eax ;m = s.heap[--h]
			cmp eax,[max_code]
			jg .cycle4 ;if (..>..) continue
			mov esi,[m]
			imul esi,sizeof.ct_data
			add esi,edx ;esi = &tree[m]
			mov eax,[bits]
			cmp word[esi+Len],ax
			je @f ;if (..!=..)
;                Trace((stderr,"code %d bits %d->%d\n", m, tree[m].Len, bits));
				movzx ebx,word[esi+Len]
				sub eax,ebx
				movzx ebx,word[esi+Freq]
				imul eax,ebx ;eax = (bits - tree[m].Len) * tree[m].Freq
				add [edi+deflate_state.opt_len],eax
				mov eax,[bits]
				mov word[esi+Len],ax
			@@:
			dec ecx
			jmp .cycle4
align 4
		.cycle4end:
		dec dword[bits]
		jmp .cycle3
align 4
.end_f:
popad
	ret
endp

; ===========================================================================
; Generate the codes for a given tree and bit counts (which need not be
; optimal).
; IN assertion: the array bl_count contains the bit length statistics for
; the given tree and the field len is set for all tree elements.
; OUT assertion: the field code is set for all tree elements of non
;     zero code length.

;void (tree, max_code, bl_count)
;    ct_data *tree     ;the tree to decorate
;    int max_code      ;largest code with non zero frequency
;    uint_16p bl_count ;number of codes at each bit length
align 4
proc gen_codes uses eax ebx ecx edx edi, tree:dword, max_code:dword, bl_count:dword
locals
	u_code dw 0 ;uint_16 ;running code value
	bits   dd 1 ;int ;bit index
	next_code rw MAX_BITS+1 ;uint_16[] ;next code value for each bit length
endl
	; The distribution counts are first used to generate the code values
	; without bit reversal.
	mov ebx,ebp
	sub ebx,2*(MAX_BITS+1)

	.cycle0: ;for (..;..<=..;..)
	cmp dword[bits],MAX_BITS
	jg .cycle0end
		mov eax,[bits]
		dec eax
		shl eax,1
		add eax,[bl_count]
		mov ax,word[eax]
		add ax,[u_code]
		shl ax,1 ;ax = (u_code + bl_count[bits-1]) << 1
		mov [u_code],ax
		mov ecx,[bits]
		mov word[ebx+2*ecx],ax ;next_code[bits] = u_code
		inc dword[bits]
		jmp .cycle0
	.cycle0end:
	; Check that the bit counts in bl_count are consistent. The last code
	; must be all ones.

	mov eax,[bl_count]
	mov ax,word[eax+2*MAX_BITS]
	add ax,[u_code]
	dec ax
	cmp ax,(1 shl MAX_BITS)-1
	je @f
		zlib_assert 'inconsistent bit counts' ;Assert(..==..)
	@@:
;    Tracev((stderr,"\ngen_codes: max_code %d ", max_code));

	xor ecx,ecx ;n = 0
	.cycle1: ;for (..;..<=..;..)
	cmp ecx,[max_code]
	jg .cycle1end
		mov edx,sizeof.ct_data
		imul edx,ecx
		add edx,[tree] ;edx = &tree[n]
		movzx edi,word[edx+Len]
		cmp edi,0
		jne @f ;if (..==0) continue
			inc ecx
			jmp .cycle1
		@@:
		; Now reverse the bits
		movzx eax,word[ebx+2*edi]
		stdcall bi_reverse, eax, edi
		mov word[edx+Code],ax
		inc word[ebx+2*edi]

;        Tracecv(tree != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
;             n, (isgraph(n) ? n : ' '), len, tree[n].Code, next_code[len]-1));
		inc ecx
		jmp .cycle1
	.cycle1end:
	ret
endp

; ===========================================================================
; Construct one Huffman tree and assigns the code bit strings and lengths.
; Update the total bit length for the current block.
; IN assertion: the field freq is set for all tree elements.
; OUT assertions: the fields len and code are set to the optimal bit length
;     and corresponding code. The length opt_len is updated; static_len is
;     also updated if stree is not null. The field max_code is set.

;void (s, desc)
;    deflate_state* s
;    tree_desc *desc ;the tree descriptor
align 4
proc build_tree uses eax ebx ecx edx edi, s:dword, desc:dword
locals
	tree     dd  ? ;ct_data* ;= desc.dyn_tree
	stree    dd  ? ;ct_data* ;= desc.stat_desc.static_tree
	elems    dd  ? ;int      ;= desc.stat_desc.elems
	m        dd  ? ;int ;iterate over heap elements
	max_code dd -1 ;int ;largest code with non zero frequency
	node     dd  ? ;int ;new node being created
endl
	; Construct the initial heap, with least frequent element in
	; heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
	; heap[0] is not used.
	mov ebx,[desc]
	mov eax,[ebx+tree_desc.dyn_tree]
	mov [tree],eax
	mov ecx,[ebx+tree_desc.stat_desc]
	mov eax,[ecx+static_tree_desc.static_tree]
	mov [stree],eax
	mov ecx,[ecx+static_tree_desc.elems]
	mov [elems],ecx
	mov edi,[s]
	zlib_debug 'build_tree cycle0 ecx = %d',ecx

	mov dword[edi+deflate_state.heap_len],0
	mov dword[edi+deflate_state.heap_max],HEAP_SIZE

	mov edx,[tree]
	xor ecx,ecx
	.cycle0: ;for (..;..<..;..)
	cmp ecx,[elems]
	jge .cycle1
		cmp word[edx+Freq],0
		je @f ;if (..!=0)
			inc dword[edi+deflate_state.heap_len]
			mov eax,[edi+deflate_state.heap_len]
			mov [max_code],ecx
			mov dword[edi+deflate_state.heap+4*eax],ecx
			mov byte[edi+deflate_state.depth+ecx],0
			jmp .end0
align 4
		@@: ;else
			mov word[edx+Len],0
		.end0:
		add edx,sizeof.ct_data
		inc ecx
		jmp .cycle0

	; The pkzip format requires that at least one distance code exists,
	; and that at least one bit should be sent even if there is only one
	; possible code. So to avoid special checks later on we force at least
	; two codes of non zero frequency.

align 4
	.cycle1: ;while (..<..)
		cmp dword[edi+deflate_state.heap_len],2
		jge .cycle1end
		inc dword[edi+deflate_state.heap_len]
		xor eax,eax
		cmp dword[max_code],2
		jge @f
			inc dword[max_code]
			mov eax,[max_code]
		@@:
		mov ecx,[edi+deflate_state.heap_len]
		mov [edi+deflate_state.heap+4*ecx],eax
		mov [node],eax
		imul eax,sizeof.ct_data
		add eax,[tree]
		mov word[eax+Freq],1
		mov eax,[node]
		mov byte[edi+deflate_state.depth+eax],0
		dec dword[edi+deflate_state.opt_len]
		cmp dword[stree],0
		je .cycle1 ;if (..)
			mov eax,[node]
			imul eax,sizeof.ct_data
			add eax,[stree]
			movzx eax,word[eax+Len]
			sub [edi+deflate_state.static_len],eax
		; node is 0 or 1 so it does not have extra bits
		jmp .cycle1
align 4
	.cycle1end:
	mov eax,[max_code]
	mov [ebx+tree_desc.max_code],eax

	; The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
	; establish sub-heaps of increasing lengths:

	mov ecx,[edi+deflate_state.heap_len]
	sar ecx,1
	.cycle2: ;for (..;..>=..;..)
		cmp ecx,1
		jl .cycle2end
		stdcall pqdownheap, edi, [tree], ecx
		dec ecx
		jmp .cycle2
align 4
	.cycle2end:

	; Construct the Huffman tree by repeatedly combining the least two
	; frequent nodes.

	mov eax,[elems]
	mov [node],eax ;next internal node of the tree
	.cycle3: ;do
		pqremove edi, [tree], ecx ;n = node of least frequency
		movzx edx,word[eax]
		mov [m],edx ;m = node of next least frequency

		mov eax,[edi+deflate_state.heap_max]
		dec eax
		mov [edi+deflate_state.heap+4*eax],ecx ;keep the nodes sorted by frequency
		dec eax
		mov [edi+deflate_state.heap_max],eax
		mov [edi+deflate_state.heap+4*eax],edx

		; Create a new node father of n and m
		;;mov edx,[m]
		imul edx,sizeof.ct_data
		add edx,[tree]
		mov ax,word[edx+Freq]
		mov edx,[tree]
		add ax,word[edx+sizeof.ct_data*ecx+Freq]
		mov edx,[node]
		imul edx,sizeof.ct_data
		add edx,[tree]
		mov word[edx+Freq],ax

		mov eax,ecx
		add eax,edi
		mov al,byte[eax+deflate_state.depth]
		mov edx,[m]
		add edx,edi
		mov ah,byte[edx+deflate_state.depth]
		cmp al,ah
		jge @f ;if (al>=ah) al=al : al=ah
			mov al,ah
		@@:
		inc al
		mov edx,[node]
		add edx,edi
		mov byte[edx+deflate_state.depth],al

		mov eax,[node]
		mov edx,[m]
		imul edx,sizeof.ct_data
		add edx,[tree]
		mov [edx+Dad],ax
		mov edx,ecx
		imul edx,sizeof.ct_data
		add edx,[tree]
		mov [edx+Dad],ax
;if DUMP_BL_TREE eq 1
;        if (tree == s->bl_tree) {
;            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
;                    node, tree[node].Freq, n, tree[n].Freq, m, tree[m].Freq);
;        }
;end if
		; and insert the new node in the heap
		mov ecx,[node]
		mov [edi+deflate_state.heap+4*SMALLEST],ecx
		inc dword[node]
		stdcall pqdownheap, edi, [tree], SMALLEST
		cmp dword[edi+deflate_state.heap_len],2
		jge .cycle3 ;while (..>=..)

	mov ecx,[edi+deflate_state.heap+4*SMALLEST]
	dec dword[edi+deflate_state.heap_max]
	mov eax,[edi+deflate_state.heap_max]
	mov [edi+deflate_state.heap+4*eax],ecx

	; At this point, the fields freq and dad are set. We can now
	; generate the bit lengths.

	stdcall gen_bitlen, edi, [desc]

	; The field len is now set, we can generate the bit codes
	mov eax,edi
	add eax,deflate_state.bl_count
	stdcall gen_codes, [tree], [max_code], eax
	ret
endp

; ===========================================================================
; Scan a literal or distance tree to determine the frequencies of the codes
; in the bit length tree.

;void (s, tree, max_code)
;    deflate_state* s
;    ct_data *tree ;the tree to be scanned
;    int max_code  ;and its largest code of non zero frequency
align 4
proc scan_tree uses eax ebx ecx edi, s:dword, tree:dword, max_code:dword
locals
	n dd ? ;int ;iterates over all tree elements
	prevlen  dd -1 ;int ;last emitted length
	curlen    dd ? ;int ;length of current code
	nextlen   dd ? ;int ;= tree[0].Len ;length of next code
	count     dd 0 ;int ;repeat count of the current code
	max_count dd 7 ;int ;max repeat count
	min_count dd 4 ;int ;min repeat count
endl
	mov edi,[s]
	mov eax,[tree]
	movzx eax,word[eax+Len]
	mov [nextlen],eax
	test eax,eax
	jnz @f ;if (..==0)
		mov dword[max_count],138
		mov dword[min_count],3
	@@:
	mov eax,[max_code]
	inc eax
	imul eax,sizeof.ct_data
	add eax,[tree]
	mov word[eax+Len],0xffff ;guard

	xor ecx,ecx
align 4
	.cycle0:
		cmp ecx,[max_code]
		jg .cycle0end ;for (..;..<=..;..)
		mov eax,[nextlen]
		mov [curlen],eax
		inc ecx
		mov eax,sizeof.ct_data
		imul eax,ecx
		add eax,[tree]
		movzx eax,word[eax+Len]
		mov [nextlen],eax
		inc dword[count]
		mov ebx,[count]
		cmp ebx,[max_count]
		jge .end0
		mov eax,[nextlen]
		cmp [curlen],eax
		je .cycle0 ;if (..<.. && ..==..) continue
align 4
		.end0:
		cmp ebx,[min_count]
		jge .end1 ;else if (..<..)
			mov eax,[curlen]
			imul eax,sizeof.ct_data
			add eax,edi
			add word[eax+deflate_state.bl_tree+Freq],bx
			jmp .end4
align 4
		.end1:
		cmp dword[curlen],0
		je .end2 ;else if (..!=0)
			mov eax,[curlen]
			cmp eax,[prevlen]
			je @f ;if (..!=..)
				imul eax,sizeof.ct_data
				add eax,edi
				inc word[eax+deflate_state.bl_tree+Freq]
			@@:
			mov eax,REP_3_6
			imul eax,sizeof.ct_data
			add eax,edi
			inc word[eax+deflate_state.bl_tree+Freq]
			jmp .end4
align 4
		.end2:
		cmp ebx,10
		jg .end3 ;else if (..<=..)
			mov eax,REPZ_3_10
			imul eax,sizeof.ct_data
			add eax,edi
			inc word[eax+deflate_state.bl_tree+Freq]
			jmp .end4
align 4
		.end3: ;else
			mov eax,REPZ_11_138
			imul eax,sizeof.ct_data
			add eax,edi
			inc word[eax+deflate_state.bl_tree+Freq]
		.end4:
		mov eax,[curlen]
		mov [prevlen],eax
		xor eax,eax
		mov dword[count],eax
		cmp dword[nextlen],eax
		jne .end5 ;if (..==0)
			mov dword[max_count],138
			mov dword[min_count],3
			jmp .cycle0
align 4
		.end5:
		cmp eax,[nextlen]
		jne .end6 ;else if (..==..)
			mov dword[max_count],6
			mov dword[min_count],3
			jmp .cycle0
align 4
		.end6: ;else
			mov dword[max_count],7
			mov dword[min_count],4
		jmp .cycle0
align 4
	.cycle0end:
	ret
endp

; ===========================================================================
; Send a literal or distance tree in compressed form, using the codes in
; bl_tree.

;void (s, tree, max_code)
;    deflate_state* s
;    ct_data *tree ;the tree to be scanned
;    int max_code  ;and its largest code of non zero frequency
align 16
proc send_tree uses eax ebx ecx edi, s:dword, tree:dword, max_code:dword
locals
	n dd ? ;int ;iterates over all tree elements
	prevlen  dd -1 ;int ;last emitted length
	curlen    dd ? ;int ;length of current code
	nextlen   dd ? ;int ;= tree[0].Len ;length of next code
	count     dd 0 ;int ;repeat count of the current code
	max_count dd 7 ;int ;max repeat count
	min_count dd 4 ;int ;min repeat count
endl
	mov edi,[s]
	; *** tree[max_code+1].Len = -1 ;guard already set
	mov eax,[tree]
	movzx eax,word[eax+Len]
	mov [nextlen],eax
	xor ecx,ecx
	test eax,eax
	jnz .cycle0 ;if (..==0)
		mov dword[max_count],138
		mov dword[min_count],3
align 4
	.cycle0: ;for (..;..<=..;..)
	cmp ecx,[max_code]
	jg .cycle0end
		mov eax,[nextlen]
		mov [curlen],eax
		mov eax,ecx
		inc eax
		imul eax,sizeof.ct_data
		add eax,[tree]
		movzx eax,word[eax+Len]
		mov [nextlen],eax
		inc dword[count]
		mov ebx,[count]
		cmp ebx,[max_count]
		jge .end0
		mov eax,[nextlen]
		cmp [curlen],eax
		jne .end0 ;if (..<.. && ..==..)
			inc ecx
			jmp .cycle0 ;continue
align 4
		.end0:
		cmp ebx,[min_count]
		jge .end1 ;else if (..<..)
			@@: ;do
				mov ebx,edi
				add ebx,deflate_state.bl_tree
				send_code edi, [curlen], ebx
				dec dword[count]
				jnz @b ;while (..!=0)
			jmp .end4
align 4
		.end1:
		cmp dword[curlen],0
		je .end2 ;else if (..!=0)
			mov eax,[curlen]
			cmp eax,[prevlen]
			je @f ;if (..!=..)
				mov ebx,edi
				add ebx,deflate_state.bl_tree
				send_code edi, eax, ebx
				dec dword[count]
			@@:
			cmp dword[count],3
			jl @f
			cmp dword[count],6
			jle .end8
			@@:
				zlib_assert ' 3_6?' ;Assert(..>=.. && ..<=..)
			.end8:
			mov ebx,edi
			add ebx,deflate_state.bl_tree
			send_code edi, REP_3_6, ebx
			mov ebx,[count]
			sub ebx,3
			stdcall send_bits, edi, ebx, 2
			jmp .end4
		.end2:
		cmp ebx,10
		jg .end3 ;else if (..<=..)
			mov ebx,edi
			add ebx,deflate_state.bl_tree
			send_code edi, REPZ_3_10, ebx
			mov ebx,[count]
			sub ebx,3
			stdcall send_bits, edi, ebx, 3
			jmp .end4
		.end3: ;else
			mov ebx,edi
			add ebx,deflate_state.bl_tree
			send_code edi, REPZ_11_138, ebx
			mov ebx,[count]
			sub ebx,11
			stdcall send_bits, edi, ebx, 7
		.end4:
		mov eax,[curlen]
		mov [prevlen],eax
		xor eax,eax
		mov dword[count],eax
		cmp [nextlen],eax
		jne .end5 ;if (..==0)
			mov dword[max_count],138
			mov dword[min_count],3
			jmp .end7
		.end5:
		mov eax,[curlen]
		cmp eax,[nextlen]
		jne .end6 ;else if (..==..)
			mov dword[max_count],6
			mov dword[min_count],3
			jmp .end7
		.end6: ;else
			mov dword[max_count],7
			mov dword[min_count],4
		.end7:
		inc ecx
		jmp .cycle0
align 4
	.cycle0end:
	ret
endp

; ===========================================================================
; Construct the Huffman tree for the bit lengths and return the index in
; bl_order of the last bit length code to send.

;int (deflate_state* s)
align 16
proc build_bl_tree uses ebx ecx edi, s:dword
	;ebx - max_blindex ;index of last bit length code of non zero freq

	mov edi,[s]
	; Determine the bit length frequencies for literal and distance trees
	mov eax,edi
	add eax,deflate_state.dyn_ltree
	stdcall scan_tree, edi, eax, [edi+deflate_state.l_desc.max_code]
	add eax,deflate_state.dyn_dtree-deflate_state.dyn_ltree
	stdcall scan_tree, edi, eax, [edi+deflate_state.d_desc.max_code]

	; Build the bit length tree:
	add eax,deflate_state.bl_desc-deflate_state.dyn_dtree
	stdcall build_tree, edi, eax
	; opt_len now includes the length of the tree representations, except
	; the lengths of the bit lengths codes and the 5+5+4 bits for the counts.

	; Determine the number of bit length codes to send. The pkzip format
	; requires that at least 4 bit length codes be sent. (appnote.txt says
	; 3 but the actual value used is 4.)

	mov ebx,BL_CODES-1
	jmp @f
align 4
	.cycle0: ;for (..;..>=..;..)
		dec ebx
		@@:
		cmp ebx,3
		jl .cycle0end
		movzx ecx,byte[ebx+bl_order]
		movzx ecx,word[edi+sizeof.ct_data*ecx+deflate_state.bl_tree+Len]
		jecxz .cycle0
align 4
	.cycle0end:
	; Update opt_len to include the bit length tree and counts
	mov eax,ebx
	inc eax
	imul eax,3
	add eax,5+5+4
	add [edi+deflate_state.opt_len],eax
;    Tracev((stderr, "\ndyn trees: dyn %ld, stat %ld", s->opt_len, s->static_len));

	mov eax,ebx
	ret
endp

; ===========================================================================
; Send the header for a block using dynamic Huffman trees: the counts, the
; lengths of the bit length codes, the literal tree and the distance tree.
; IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.

;void (deflate_state* s, lcodes, dcodes, blcodes)
;    int lcodes, dcodes, blcodes ;number of codes for each tree
align 4
proc send_all_trees uses eax ebx ecx edi, s:dword, lcodes:dword, dcodes:dword, blcodes:dword
;ecx = index in bl_order
	cmp dword[lcodes],257
	jl @f
	cmp dword[dcodes],1
	jl @f
	cmp dword[blcodes],4
	jge .end0
	@@:
		zlib_assert 'not enough codes' ;Assert(..>=.. && ..>=.. && ..>=..)
	.end0:
	cmp dword[lcodes],L_CODES
	jg @f
	cmp dword[dcodes],D_CODES
	jg @f
	cmp dword[blcodes],BL_CODES
	jle .end1
	@@:
		zlib_assert 'too many codes' ;Assert(..<=.. && ..<=.. && ..<=..)
	.end1:
;    Tracev((stderr, "\nbl counts: "));
	mov edi,[s]
	mov eax,[lcodes]
	sub eax,257
	stdcall send_bits, edi, eax, 5 ;not +255 as stated in appnote.txt
	mov eax,[dcodes]
	dec eax
	stdcall send_bits, edi, eax, 5
	mov eax,[blcodes]
	sub eax,4
	stdcall send_bits, edi, eax, 4 ;not -3 as stated in appnote.txt
	xor ecx,ecx
	.cycle0:
		cmp ecx,[blcodes]
		jge .cycle0end ;for (..;..<..;..)
;        Tracev((stderr, "\nbl code %2d ", bl_order[ecx]));
		movzx eax,byte[ecx+bl_order]
		movzx eax,word[edi+sizeof.ct_data*eax+deflate_state.bl_tree+Len]
		stdcall send_bits, edi, eax, 3
		inc ecx
		jmp .cycle0
align 4
	.cycle0end:
;    Tracev((stderr, "\nbl tree: sent %ld", s->bits_sent));

	mov ebx,[lcodes]
	dec ebx
	mov eax,edi
	add eax,deflate_state.dyn_ltree
	stdcall send_tree, edi, eax, ebx ;literal tree
;    Tracev((stderr, "\nlit tree: sent %ld", s->bits_sent));

	mov ebx,[dcodes]
	dec ebx
	add eax,deflate_state.dyn_dtree-deflate_state.dyn_ltree
	stdcall send_tree, edi, eax, ebx ;distance tree
;    Tracev((stderr, "\ndist tree: sent %ld", s->bits_sent));
	ret
endp

; ===========================================================================
; Send a stored block

;void (s, buf, stored_len, last)
;    deflate_state* s
;    charf *buf     ;input block
;    ulg stored_len ;length of input block
;    int last       ;one if this is the last block for a file
align 4
proc _tr_stored_block uses eax edi, s:dword, buf:dword, stored_len:dword, last:dword
	mov edi,[s]
	mov eax,[last]
	add eax,STORED_BLOCK shl 1
	stdcall send_bits, edi, eax, 3 ;send block type
if DEBUG eq 1
	mov eax,[edi+deflate_state.compressed_len]
	add eax,3+7
	and eax,not 7
	mov [edi+deflate_state.compressed_len],eax
	mov eax,[stored_len]
	add eax,4
	shl eax,3
	add [edi+deflate_state.compressed_len],eax
end if
	stdcall copy_block, edi, [buf], [stored_len], 1 ;with header
	ret
endp

; ===========================================================================
; Flush the bits in the bit buffer to pending output (leaves at most 7 bits)

;void (deflate_state* s)
;align 4
;proc _tr_flush_bits, s:dword
;	stdcall bi_flush, [s]
;	ret
;endp

_tr_flush_bits equ bi_flush

; ===========================================================================
; Send one empty static block to give enough lookahead for inflate.
; This takes 10 bits, of which 7 may remain in the bit buffer.

;void (deflate_state* s)
align 4
proc _tr_align uses edi, s:dword
	mov edi,[s]
	stdcall send_bits, edi, STATIC_TREES shl 1, 3
	send_code edi, END_BLOCK, static_ltree
if DEBUG eq 1
	add [edi+deflate_state.compressed_len],10 ;3 for block type, 7 for EOB
end if
	stdcall bi_flush, edi
	ret
endp

; ===========================================================================
; Determine the best encoding for the current block: dynamic trees, static
; trees or store, and output the encoded block to the zip file.

;void (s, buf, stored_len, last)
;    deflate_state* s
;    charf *buf     ;input block, or NULL if too old
;    ulg stored_len ;length of input block
;    int last       ;one if this is the last block for a file
align 4
proc _tr_flush_block uses eax ebx edi, s:dword, buf:dword, stored_len:dword, last:dword
locals
	opt_lenb dd ? ;ulg
	static_lenb dd ? ;opt_len and static_len in bytes
	max_blindex dd 0 ;int ;index of last bit length code of non zero freq
endl
	; Build the Huffman trees unless a stored block is forced
	mov edi,[s]
	cmp word[edi+deflate_state.level],0
	jle .end0 ;if (..>0)

		; Check if the file is binary or text
		mov ebx,[edi+deflate_state.strm]
		cmp dword[ebx+z_stream.data_type],Z_UNKNOWN
		jne @f ;if (..==..)
			stdcall detect_data_type, edi
			mov [ebx+z_stream.data_type],eax
		@@:

		; Construct the literal and distance trees
		mov eax,edi
		add eax,deflate_state.l_desc
		stdcall build_tree, edi, eax
;        Tracev((stderr, "\nlit data: dyn %ld, stat %ld", s->opt_len, s->static_len));

		mov eax,edi
		add eax,deflate_state.d_desc
		stdcall build_tree, edi, eax
;        Tracev((stderr, "\ndist data: dyn %ld, stat %ld", s->opt_len, s->static_len));
		; At this point, opt_len and static_len are the total bit lengths of
		; the compressed block data, excluding the tree representations.

		; Build the bit length tree for the above two trees, and get the index
		; in bl_order of the last bit length code to send.

		stdcall build_bl_tree, edi
		mov [max_blindex],eax

		; Determine the best encoding. Compute the block lengths in bytes.
		mov eax,[edi+deflate_state.opt_len]
		add eax,3+7
		shr eax,3
		mov [opt_lenb],eax
		mov eax,[edi+deflate_state.static_len]
		add eax,3+7
		shr eax,3
		mov [static_lenb],eax

;        Tracev((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u ",
;                opt_lenb, s->opt_len, static_lenb, s->static_len, stored_len,
;                s->last_lit));

		cmp eax,[opt_lenb]
		ja .end1 ;if (..<=..)
			mov [opt_lenb],eax
		jmp .end1
	.end0: ;else
		cmp dword[buf],0
		jne @f
			zlib_assert 'lost buf' ;Assert(..!=0)
		@@:
		mov eax,[stored_len]
		add eax,5
		mov [static_lenb],eax
		mov [opt_lenb],eax ;force a stored block
	.end1:

if FORCE_STORED eq 1
	cmp dword[buf],0
	je .end2 ;if (..!=0) ;force stored block
else
	mov eax,[stored_len]
	add eax,4
	cmp eax,[opt_lenb]
	ja .end2
	cmp dword[buf],0
	je .end2 ;if (..<=.. && ..!=0)
		;4: two words for the lengths
end if
		; The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
		; Otherwise we can't have processed more than WSIZE input bytes since
		; the last block flush, because compression would have been
		; successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
		; transform a block into a stored block.

		stdcall _tr_stored_block, edi, [buf], [stored_len], [last]
		jmp .end4
	.end2:
if FORCE_STATIC eq 1
	cmp dword[static_lenb],0
	jl .end3 ;else if (..>=0) ;force static trees
else
	cmp word[edi+deflate_state.strategy],Z_FIXED
	je @f
	mov eax,[opt_lenb]
	cmp [static_lenb],eax
	je @f ;else if (..==.. || ..==..)
		jmp .end3
	@@:
end if
		mov eax,STATIC_TREES shl 1
		add eax,[last]
		stdcall send_bits, edi, eax, 3
		stdcall compress_block, edi, static_ltree, static_dtree
if DEBUG eq 1
		mov eax,[edi+deflate_state.static_len]
		add eax,3
		add [edi+deflate_state.compressed_len],eax
end if
		jmp .end4
	.end3: ;else
		mov eax,DYN_TREES shl 1
		add eax,[last]
		stdcall send_bits, edi, eax, 3
		mov eax,[max_blindex]
		inc eax
		push eax
		mov eax,[edi+deflate_state.d_desc.max_code]
		inc eax
		push eax
		mov eax,[edi+deflate_state.l_desc.max_code]
		inc eax
		stdcall send_all_trees, edi, eax ;, ..., ...
		mov eax,edi
		add eax,deflate_state.dyn_dtree
		push eax
		add eax,deflate_state.dyn_ltree-deflate_state.dyn_dtree
		stdcall compress_block, edi, eax ;, ...
if DEBUG eq 1
		mov eax,[edi+deflate_state.opt_len]
		add eax,3
		add [edi+deflate_state.compressed_len],eax
end if
	.end4:
;    Assert (s->compressed_len == s->bits_sent, "bad compressed size");
	; The above check is made mod 2^32, for files larger than 512 MB
	; and uLong implemented on 32 bits.

	stdcall init_block,edi

	cmp dword[last],0
	je @f ;if (..)
		stdcall bi_windup,edi
if DEBUG eq 1
		add [edi+deflate_state.compressed_len],7 ;align on byte boundary
end if
	@@:
;    Tracev((stderr,"\ncomprlen %lu(%lu) ", s->compressed_len>>3,
;           s->compressed_len-7*last));
	ret
endp

; ===========================================================================
; Save the match info and tally the frequency counts. Return true if
; the current block must be flushed.

;int (s, dist, lc)
;    deflate_state* s
;    unsigned dist ;distance of matched string
;    unsigned lc   ;match length-MIN_MATCH or unmatched char (if dist==0)
align 4
proc _tr_tally uses ebx edi, s:dword, dist:dword, lc:dword
	mov edi,[s]
	mov eax,[edi+deflate_state.last_lit]
	shl eax,1
	add eax,[edi+deflate_state.d_buf]
	mov ebx,[dist]
	mov word[eax],bx
	mov eax,[edi+deflate_state.last_lit]
	add eax,[edi+deflate_state.l_buf]
	mov ebx,[lc]
	mov byte[eax],bl
	inc dword[edi+deflate_state.last_lit]
	cmp dword[dist],0
	jne @f ;if (..==0)
		; lc is the unmatched char
		mov eax,[lc]
		inc word[edi+sizeof.ct_data*eax+deflate_state.dyn_ltree+Freq]
		jmp .end0
align 4
	@@: ;else
		inc dword[edi+deflate_state.matches]
		; Here, lc is the match length - MIN_MATCH
		dec dword[dist] ;dist = match distance - 1
		MAX_DIST edi
		cmp word[dist],ax
		jge @f
		cmp word[lc],MAX_MATCH-MIN_MATCH
		jg @f
		d_code [dist]
		cmp ax,D_CODES
		jl .end2
		@@:
			zlib_assert '_tr_tally: bad match' ;Assert(..<.. && ..<=.. && ..<..)
		.end2:
		mov eax,[lc]
		movzx eax,byte[eax+_length_code]
		inc word[edi+sizeof.ct_data*eax+deflate_state.dyn_ltree+sizeof.ct_data*(LITERALS+1)+Freq]
		d_code [dist]
		inc word[edi+sizeof.ct_data*eax+deflate_state.dyn_dtree+Freq]
	.end0:

if TRUNCATE_BLOCK eq 1
	; Try to guess if it is profitable to stop the current block here
	mov eax,[edi+deflate_state.last_lit]
	and eax,0x1fff
	jnz .end1
	cmp word[edi+deflate_state.level],2
	jle .end1 ;if (..==0 && ..>..)
	; Compute an upper bound for the compressed length
;        ulg out_length = (ulg)s->last_lit*8L;
;        ulg in_length = (ulg)((long)s->strstart - s->block_start);
;        int dcode;
;        for (dcode = 0; dcode < D_CODES; dcode++) {
;            out_length += (ulg)s->dyn_dtree[dcode].Freq *
;                (5L+extra_dbits[dcode]);
;        }
;        out_length >>= 3;
;        Tracev((stderr,"\nlast_lit %u, in %ld, out ~%ld(%ld%%) ",
;               s->last_lit, in_length, out_length,
;               100L - out_length*100L/in_length));
;        if (s->matches < s->last_lit/2 && out_length < in_length/2) return 1;
	.end1:
end if
	mov ebx,[edi+deflate_state.lit_bufsize]
	dec ebx
	xor eax,eax
	cmp [edi+deflate_state.last_lit],ebx
	sete al ;return (..==..)

	; We avoid equality with lit_bufsize because of wraparound at 64K
	; on 16 bit machines and because stored blocks are restricted to
	; 64K-1 bytes.
	ret
endp

; ===========================================================================
; Send the block data compressed using the given Huffman trees

;void (s, ltree, dtree)
;    deflate_state* s
;    ct_data *ltree ;literal tree
;    ct_data *dtree ;distance tree
align 4
proc compress_block uses eax edi, s:dword, ltree:dword, dtree:dword
locals
	dist  dd ? ;unsigned ;distance of matched string
	lc    dd ? ;int      ;match length or unmatched char (if dist == 0)
	lx    dd 0 ;unsigned ;running index in l_buf
	u_code dd ? ;unsigned ;the code to send
endl
	mov edi,[s]
	cmp dword[edi+deflate_state.last_lit],0
	je .end0 ;if (..!=0)
	.cycle0: ; do
		mov eax,[lx]
		shl eax,1
		add eax,[edi+deflate_state.d_buf]
		movzx eax,word[eax]
		mov [dist],eax
		mov eax,[lx]
		add eax,[edi+deflate_state.l_buf]
		movzx eax,byte[eax]
		mov [lc],eax
		inc dword[lx]
		cmp dword[dist],0
		jne @f ;if (..==0)
			send_code edi, [lc], [ltree] ;send a literal byte
;            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
			jmp .end1
		@@: ;else
			; Here, lc is the match length - MIN_MATCH
			mov eax,[lc]
			add eax,_length_code
			movzx eax,byte[eax]
			mov [u_code],eax
			add eax,LITERALS+1
			send_code edi, eax, [ltree] ;send the length code
			mov eax,[u_code]
			mov eax,[4*eax+extra_lbits]
			test eax,eax
			jz @f ;if (..!=0)
				push eax ;extra
				mov eax,[u_code]
				mov eax,[4*eax+base_length]
				sub [lc],eax
				stdcall send_bits, edi, [lc] ;, ... ;send the extra length bits
			@@:
			dec dword[dist] ;dist is now the match distance - 1
			d_code [dist]
			mov [u_code],eax
			cmp eax,D_CODES
			jl @f
				zlib_assert 'bad d_code' ;Assert(..<..)
			@@:
			send_code edi, [u_code], [dtree] ;send the distance code
			mov eax,[u_code]
			mov eax,[4*eax+extra_dbits]
			test eax,eax
			jz .end1 ;if (..!=0)
				push eax ;extra
				mov eax,[u_code]
				mov eax,[4*eax+base_dist]
				sub [dist],eax
				stdcall send_bits, edi, [dist] ;, ... ;send the extra distance bits
		.end1: ;literal or match pair ?

		; Check that the overlay between pending_buf and d_buf+l_buf is ok:
		mov eax,[lx]
		shl eax,1
		add eax,[edi+deflate_state.lit_bufsize]
		cmp [edi+deflate_state.pending],eax
		jl @f
			zlib_assert 'pendingBuf overflow' ;Assert(..<..)
		@@:
		mov eax,[edi+deflate_state.last_lit]
		cmp [lx],eax
		jb .cycle0 ;while (..<..)
align 4
	.end0:

	send_code edi, END_BLOCK, [ltree]
	ret
endp

; ===========================================================================
; Check if the data type is TEXT or BINARY, using the following algorithm:
; - TEXT if the two conditions below are satisfied:
;    a) There are no non-portable control characters belonging to the
;       "black list" (0..6, 14..25, 28..31).
;    b) There is at least one printable character belonging to the
;       "white list" (9 {TAB}, 10 {LF}, 13 {CR}, 32..255).
; - BINARY otherwise.
; - The following partially-portable control characters form a
;   "gray list" that is ignored in this detection algorithm:
;   (7 {BEL}, 8 {BS}, 11 {VT}, 12 {FF}, 26 {SUB}, 27 {ESC}).
; IN assertion: the fields Freq of dyn_ltree are set.

;int (deflate_state* s)
align 4
proc detect_data_type uses ebx ecx edi, s:dword
	; black_mask is the bit mask of black-listed bytes
	; set bits 0..6, 14..25, and 28..31
	; 0xf3ffc07f = binary 11110011111111111100000001111111
locals
	black_mask dd 0xf3ffc07f
endl
	mov edi,[s]

	; Check for non-textual ("black-listed") bytes.
	xor ecx,ecx
	mov ebx,edi
	add ebx,deflate_state.dyn_ltree+Freq
	.cycle0:
	cmp ecx,31
	jg .cycle0end ;for (..;..<=..;..,..)
		bt dword[black_mask],0
		jnc @f
		cmp word[ebx],0
		je @f ;if (..&.. && ..!=0)
			mov eax,Z_BINARY
			jmp .end_f
		@@:
		shr dword[black_mask],1
		add ebx,sizeof.ct_data
		inc ecx
		jmp .cycle0
	.cycle0end:

	; Check for textual ("white-listed") bytes.
	mov ebx,edi
	add ebx,deflate_state.dyn_ltree+Freq+9*sizeof.ct_data
	cmp word[ebx],0
	jne @f
	add ebx,sizeof.ct_data
	cmp word[ebx],0
	jne @f
	add ebx,3*sizeof.ct_data
	cmp word[ebx],0
	je .end0
	@@: ;if (..!=0 || ..!=0 || ..!= 0)
		mov eax,Z_TEXT
		jmp .end_f
	.end0:
	mov ecx,32
	mov ebx,edi
	add ebx,deflate_state.dyn_ltree+Freq+32*sizeof.ct_data
	.cycle1:
	cmp ecx,LITERALS
	jge .cycle1end ;for (..;..<..;..,..)
		cmp word[ebx],0
		je @f ;if (..!=0)
			mov eax,Z_TEXT
			jmp .end_f
		@@:
		add ebx,sizeof.ct_data
		inc ecx
		jmp .cycle1
	.cycle1end:

	; There are no "black-listed" or "white-listed" bytes:
	; this stream either is empty or has tolerated ("gray-listed") bytes only.

	mov eax,Z_BINARY
.end_f:
	ret
endp

; ===========================================================================
; Reverse the first len bits of a code, using straightforward code (a faster
; method would use a table)
; IN assertion: 1 <= len <= 15

;unsigned (code, len)
;    unsigned code ;the value to invert
;    int len       ;its bit length
align 4
proc bi_reverse uses ebx, p1code:dword, len:dword
	xor eax,eax
	@@: ;do
		mov ebx,[p1code]
		and ebx,1
		or eax,ebx
		shr dword[p1code],1
		shl eax,1
		dec dword[len]
		cmp dword[len],0
		jg @b ;while (..>..)
	shr eax,1
	ret
endp

; ===========================================================================
; Flush the bit buffer, keeping at most 7 bits in it.

;void (deflate_state* s)
align 4
proc bi_flush uses eax ecx edi, s:dword
	mov edi,[s]
	cmp dword[edi+deflate_state.bi_valid],16
	jne @f ;if (..==..)
		mov cx,[edi+deflate_state.bi_buf]
		put_short edi,cx
		mov word[edi+deflate_state.bi_buf],0
		mov dword[edi+deflate_state.bi_valid],0
		jmp .end0
	@@: ;else if (..>=..)
		cmp dword[edi+deflate_state.bi_valid],8
		jl .end0
		mov cl,byte[edi+deflate_state.bi_buf]
		put_byte edi,cl
		shr word[edi+deflate_state.bi_buf],8
		sub dword[edi+deflate_state.bi_valid],8
	.end0:
	ret
endp

; ===========================================================================
; Flush the bit buffer and align the output on a byte boundary

;void (deflate_state* s)
align 4
proc bi_windup uses eax ecx edi, s:dword
	mov edi,[s]
	cmp dword[edi+deflate_state.bi_valid],8
	jle @f ;if (..>..)
		mov cx,[edi+deflate_state.bi_buf]
		put_short edi, cx
		jmp .end0
	@@: ;else if (..>0)
		cmp dword[edi+deflate_state.bi_valid],0
		jle .end0
		mov cl,byte[edi+deflate_state.bi_buf]
		put_byte edi, cl
	.end0:
	mov word[edi+deflate_state.bi_buf],0
	mov dword[edi+deflate_state.bi_valid],0
if DEBUG eq 1
	mov eax,[edi+deflate_state.bits_sent]
	add eax,7
	and eax,not 7
	mov [edi+deflate_state.bits_sent],eax
end if
	ret
endp

; ===========================================================================
; Copy a stored block, storing first the length and its
; one's complement if requested.

;void (s, buf, len, header)
;    deflate_state* s
;    charf    *buf   ;the input data
;    unsigned len    ;its length
;    int      header ;true if block header must be written
align 4
proc copy_block uses eax ebx ecx edi esi, s:dword, buf:dword, len:dword, p4header:dword
	mov edi,[s]
	stdcall bi_windup,edi ;align on byte boundary

	cmp dword[p4header],0
	je @f ;if (..)
		mov ecx,[len]
		put_short edi, cx
		not cx
		put_short edi, cx
if DEBUG eq 1
		add dword[edi+deflate_state.bits_sent],2*16
end if
	@@:
if DEBUG eq 1
	mov ecx,[len]
	shl ecx,3
	add [edi+deflate_state.bits_sent],ecx
end if
	mov ecx,[len]
;	test ecx,ecx
;	jz .end_f
	mov esi,[buf]
	jmp .end0
align 4
	@@: ;while (len--)
		lodsb
		mov bl,al
		put_byte edi, bl
	.end0:
		loop @b
;	.end_f:
	ret
endp
