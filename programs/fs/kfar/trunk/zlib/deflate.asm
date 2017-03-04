; deflate.asm -- compress data using the deflation algorithm
; Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler
; For conditions of distribution and use, see copyright notice in zlib.inc

;  ALGORITHM

;      The "deflation" process depends on being able to identify portions
;      of the input text which are identical to earlier input (within a
;      sliding window trailing behind the input currently being processed).

;      The most straightforward technique turns out to be the fastest for
;      most input files: try all possible matches and select the longest.
;      The key feature of this algorithm is that insertions into the string
;      dictionary are very simple and thus fast, and deletions are avoided
;      completely. Insertions are performed at each input character, whereas
;      string matches are performed only when the previous match ends. So it
;      is preferable to spend more time in matches to allow very fast string
;      insertions and avoid deletions. The matching algorithm for small
;      strings is inspired from that of Rabin & Karp. A brute force approach
;      is used to find longer strings when a small match has been found.
;      A similar algorithm is used in comic (by Jan-Mark Wams) and freeze
;      (by Leonid Broukhis).
;         A previous version of this file used a more sophisticated algorithm
;      (by Fiala and Greene) which is guaranteed to run in linear amortized
;      time, but has a larger average cost, uses more memory and is patented.
;      However the F&G algorithm may be faster for some highly redundant
;      files if the parameter max_chain_length (described below) is too large.

;  ACKNOWLEDGEMENTS

;      The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
;      I found it in 'freeze' written by Leonid Broukhis.
;      Thanks to many people for bug reports and testing.

;  REFERENCES

;      Deutsch, L.P.,"DEFLATE Compressed Data Format Specification".
;      Available in http://tools.ietf.org/html/rfc1951

;      A description of the Rabin and Karp algorithm is given in the book
;         "Algorithms" by R. Sedgewick, Addison-Wesley, p252.

;      Fiala,E.R., and Greene,D.H.
;         Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595


deflate_copyright db ' deflate 1.2.8 Copyright 1995-2013 Jean-loup Gailly and Mark Adler ',0

;  If you use the zlib library in a product, an acknowledgment is welcome
;  in the documentation of your product. If for some reason you cannot
;  include such an acknowledgment, I would appreciate that you keep this
;  copyright string in the executable of your product.

; ===========================================================================
;  Function prototypes.

;enum block_state
need_more   equ 0 ;block not completed, need more input or more output
block_done  equ 1 ;block flush performed
finish_started equ 2 ;finish started, need only more output at next deflate
finish_done equ 3 ;finish done, accept no more input or output

; ===========================================================================
; Local data

NIL equ 0
; Tail of hash chains

TOO_FAR equ 4096
; Matches of length 3 are discarded if their distance exceeds TOO_FAR

; Values for max_lazy_match, good_match and max_chain_length, depending on
; the desired pack level (0..9). The values given below have been tuned to
; exclude worst case performance for pathological files. Better values may be
; found for specific files.

struct config_s ;config
	good_length dw ? ;uint_16 ;reduce lazy search above this match length
	max_lazy    dw ? ;uint_16 ;do not perform lazy search above this match length
	nice_length dw ? ;uint_16 ;quit search above this match length
	max_chain   dw ? ;uint_16
	co_func     dd ? ;compress_func
ends

align 16
configuration_table:
	config_s  0,   0,   0,    0, deflate_stored  ;store only
	config_s  4,   4,   8,    4, deflate_fast ;max speed, no lazy matches
if FASTEST eq 0
	config_s  4,   5,  16,    8, deflate_fast
	config_s  4,   6,  32,   32, deflate_fast
	config_s  4,   4,  16,   16, deflate_slow ;lazy matches
	config_s  8,  16,  32,   32, deflate_slow
	config_s  8,  16, 128,  128, deflate_slow
	config_s  8,  32, 128,  256, deflate_slow
	config_s 32, 128, 258, 1024, deflate_slow
	config_s 32, 258, 258, 4096, deflate_slow ;max compression
end if

; Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
; For deflate_fast() (levels <= 3) good is ignored and lazy has a different
; meaning.


EQUAL equ 0
; result of memcmp for equal strings

; rank Z_BLOCK between Z_NO_FLUSH and Z_PARTIAL_FLUSH
macro RANK f, reg
{
local .end0
	xor reg,reg
	cmp f,4
	jle .end0
		sub reg,9
	.end0:
	add reg,f
	add reg,f
}

; ===========================================================================
; Update a hash value with the given input byte
; IN  assertion: all calls to to UPDATE_HASH are made with consecutive
;    input characters, so that a running hash key can be computed from the
;    previous key instead of complete recalculation each time.

macro UPDATE_HASH s,h,c
{
push ebx ecx
	mov ebx,h
	mov ecx,[s+deflate_state.hash_shift]
	shl ebx,cl
	xor ebx,c
	and ebx,[s+deflate_state.hash_mask]
	mov h,ebx
pop ecx ebx
}

; ===========================================================================
; Insert string str in the dictionary and set match_head to the previous head
; of the hash chain (the most recent string with same hash key). Return
; the previous length of the hash chain.
; If this file is compiled with -DFASTEST, the compression level is forced
; to 1, and no hash chains are maintained.
; IN  assertion: all calls to to INSERT_STRING are made with consecutive
;    input characters and the first MIN_MATCH bytes of str are valid
;    (except for the last MIN_MATCH-1 bytes of the input file).

macro INSERT_STRING s, str, match_head
{
	mov eax,[s+deflate_state.window]
	add eax,str
	add eax,MIN_MATCH-1
	movzx eax,byte[eax]
	UPDATE_HASH s, [s+deflate_state.ins_h], eax
	mov eax,[s+deflate_state.ins_h]
	shl eax,1
	add eax,[s+deflate_state.head]
	movzx eax,word[eax]
	mov match_head,eax
push ebx
if FASTEST eq 0
	mov ebx,[s+deflate_state.w_mask]
	and ebx,str
	shl ebx,1
	add ebx,[s+deflate_state.prev]
	mov [ebx],ax

end if
	mov eax,[s+deflate_state.ins_h]
	shl eax,1
	add eax,[s+deflate_state.head]
	mov ebx,str
	mov [eax],bx
pop ebx
}

; ===========================================================================
; Initialize the hash table (avoiding 64K overflow for 16 bit systems).
; prev[] will be initialized on the fly.

macro CLEAR_HASH s
{
	;mov eax,[s+deflate_state.hash_size]
	;dec eax
	;shl eax,1
	;add eax,[s+deflate_state.head]
	;mov word[eax],NIL
	mov eax,[s+deflate_state.hash_size]
	;dec eax
	shl eax,1 ;sizeof(*s.head)
	stdcall zmemzero, [s+deflate_state.head], eax
}

align 4
proc deflateInit, strm:dword, level:dword
	stdcall deflateInit_, [strm], [level], ZLIB_VERSION, sizeof.z_stream
	ret
endp

; =========================================================================
;int (strm, level, version, stream_size)
;    z_streamp strm
;    int level
;    const char *version
;    int stream_size
align 4
proc deflateInit_, strm:dword, level:dword, version:dword, stream_size:dword
	stdcall deflateInit2_, [strm], [level], Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL,\
			Z_DEFAULT_STRATEGY, [version], [stream_size]
	; To do: ignore strm->next_in if we use it as window
	ret
endp

align 4
proc deflateInit2, strm:dword, level:dword, method:dword, windowBits:dword, memLevel:dword, strategy:dword
	stdcall deflateInit2_, [strm],[level],[method],[windowBits],[memLevel],\
		[strategy], ZLIB_VERSION, sizeof.z_stream
	ret
endp

; =========================================================================
;int (strm, level, method, windowBits, memLevel, strategy,
;                  version, stream_size)
;    z_streamp strm
;    int  level
;    int  method
;    int  windowBits
;    int  memLevel
;    int  strategy
;    const char *version
;    int stream_size
align 4
proc deflateInit2_ uses ebx ecx edx edi, strm:dword, level:dword, method:dword,\
	windowBits:dword, memLevel:dword, strategy:dword, version:dword, stream_size:dword
locals
	wrap dd 1 ;int
	overlay dd ? ;uint_16p
endl
	; We overlay pending_buf and d_buf+l_buf. This works since the average
	; output size for (length,distance) codes is <= 24 bits.

	mov eax,[version]
	cmp eax,Z_NULL
	je @f
	mov ebx,dword[ZLIB_VERSION]
	cmp dword[eax],ebx
	jne @f
	cmp dword[stream_size],sizeof.z_stream
	je .end0
	@@: ;if (..==0 || ..[0]!=..[0] || ..!=..)
		mov eax,Z_VERSION_ERROR
		jmp .end_f
	.end0:
	mov ebx,[strm]
	cmp ebx,Z_NULL
	jne @f ;if (..==0) return ..
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	@@:

	mov dword[ebx+z_stream.msg],Z_NULL
	cmp dword[ebx+z_stream.zalloc],0
	jne @f ;if (..==0)
if Z_SOLO eq 1
		mov eax,Z_STREAM_ERROR
		jmp .end_f
else
		mov dword[ebx+z_stream.zalloc],zcalloc
		mov dword[ebx+z_stream.opaque],0
end if
	@@:
	cmp dword[ebx+z_stream.zfree],0
	jne @f ;if (..==0)
if Z_SOLO eq 1
		mov eax,Z_STREAM_ERROR
		jmp .end_f
else
		mov dword[ebx+z_stream.zfree],zcfree
end if
	@@:

if FASTEST eq 1
	cmp dword[level],0
	je @f ;if (..!=0)
		mov dword[level],1
	@@:
else
	cmp dword[level],Z_DEFAULT_COMPRESSION
	jne @f ;if (..==..)
		mov dword[level],6
	@@:
end if

	cmp dword[windowBits],0
	jge @f ;if (..<0) ;suppress zlib wrapper
		mov dword[wrap],0
		neg dword[windowBits]
		jmp .end1
	@@:
if GZIP eq 1
	cmp dword[windowBits],15
	jle .end1 ;else if (..>15)
		mov dword[wrap],2 ;write gzip wrapper instead
		sub dword[windowBits],16
end if
	.end1:
	cmp dword[memLevel],1
	jl .end2
	cmp dword[memLevel],MAX_MEM_LEVEL
	jg .end2
	cmp dword[method],Z_DEFLATED
	jne .end2
	cmp dword[windowBits],8
	jl .end2
	cmp dword[windowBits],15
	jg .end2
	cmp dword[level],0
	jl .end2
	cmp dword[level],9
	jg .end2
	cmp dword[strategy],0
	jl .end2
	cmp dword[strategy],Z_FIXED
	jle @f
	.end2: ;if (..<.. || ..>.. || ..!=.. || ..<.. || ..>.. || ..<0 || ..>.. || ..<0 || ..>..)
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	@@:
	cmp dword[windowBits],8
	jne @f ;if (..==..)
		inc dword[windowBits] ;until 256-byte window bug fixed
	@@:
	ZALLOC ebx, 1, sizeof.deflate_state
	;eax = s
	cmp eax,Z_NULL
	jne @f ;if (..==0)
		mov eax,Z_MEM_ERROR
		jmp .end_f
	@@:
	mov edi,eax ;edi = s
	mov [ebx+z_stream.state],edi
	mov [edi+deflate_state.strm],ebx
	mov dword[edi+deflate_state.status],INIT_STATE ;to pass state test in deflateReset()

	mov eax,[wrap]
	mov [edi+deflate_state.wrap],eax
	mov [edi+deflate_state.gzhead],Z_NULL
	mov ecx,[windowBits]
	mov [edi+deflate_state.w_bits],ecx
	xor eax,eax
	inc eax
	shl eax,cl
	mov [edi+deflate_state.w_size],eax
	dec eax
	mov [edi+deflate_state.w_mask],eax

	mov ecx,[memLevel]
	add ecx,7
	mov [edi+deflate_state.hash_bits],ecx
	xor eax,eax
	inc eax
	shl eax,cl
	mov [edi+deflate_state.hash_size],eax
	dec eax
	mov [edi+deflate_state.hash_mask],eax
	add ecx,MIN_MATCH-1
	xor edx,edx
	mov eax,ecx
	mov ecx,MIN_MATCH
	div ecx
	mov [edi+deflate_state.hash_shift],eax

	ZALLOC ebx, [edi+deflate_state.w_size], 2 ;2*sizeof(Byte)
	mov [edi+deflate_state.window],eax
	ZALLOC ebx, [edi+deflate_state.w_size], 2 ;sizeof(Pos)
	mov [edi+deflate_state.prev],eax
	ZALLOC ebx, [edi+deflate_state.hash_size], 2 ;sizeof(Pos)
	mov [edi+deflate_state.head],eax

	mov dword[edi+deflate_state.high_water],0 ;nothing written to s->window yet

	mov ecx,[memLevel]
	add ecx,6
	xor eax,eax
	inc eax
	shl eax,cl
	mov [edi+deflate_state.lit_bufsize],eax ;16K elements by default

	ZALLOC ebx, eax, 4 ;sizeof(uint_16)+2
	mov [overlay],eax
	mov [edi+deflate_state.pending_buf],eax
	mov eax,[edi+deflate_state.lit_bufsize]
	imul eax,4 ;sizeof(uint_16)+2
	mov [edi+deflate_state.pending_buf_size],eax

	cmp dword[edi+deflate_state.window],Z_NULL
	je .end3
	cmp dword[edi+deflate_state.prev],Z_NULL
	je .end3
	cmp dword[edi+deflate_state.head],Z_NULL
	je .end3
	cmp dword[edi+deflate_state.pending_buf],Z_NULL
	jne @f
	.end3: ;if (..==0 || ..==0 || ..==0 || ..==0)
		mov dword[edi+deflate_state.status],FINISH_STATE
		ERR_MSG Z_MEM_ERROR
		mov [ebx+z_stream.msg],eax
		stdcall deflateEnd, ebx
		mov eax,Z_MEM_ERROR
		jmp .end_f
	@@:
	mov eax,[edi+deflate_state.lit_bufsize]
	add eax,[overlay]
	mov [edi+deflate_state.d_buf],eax
	mov eax,[edi+deflate_state.lit_bufsize]
	imul eax,3 ;1+sizeof(uint_16)
	add eax,[edi+deflate_state.pending_buf]
	mov [edi+deflate_state.l_buf],eax

	mov eax,[level]
	mov [edi+deflate_state.level],ax
	mov eax,[strategy]
	mov [edi+deflate_state.strategy],ax
	mov eax,[method]
	mov [edi+deflate_state.method],al

	stdcall deflateReset, ebx
.end_f:
	ret
endp

; =========================================================================
;int (strm, dictionary, dictLength)
;    z_streamp strm
;    const Bytef *dictionary
;    uInt  dictLength
align 4
proc deflateSetDictionary uses ebx ecx edx edi esi, strm:dword, dictionary:dword, dictLength:dword
locals
	wrap  dd ? ;int
	avail dd ? ;unsigned
	next  dd ? ;unsigned char*
endl
	mov ebx,[strm]
	cmp ebx,Z_NULL
	je @f
	mov edi,[ebx+z_stream.state]
	cmp edi,Z_NULL
	je @f
	cmp dword[dictionary],Z_NULL
	jne .end0 ;if (..==0 || ..==0 || ..==0)
	@@:
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end0:
	
	mov eax,[edi+deflate_state.wrap]
	mov [wrap],eax
	cmp dword[wrap],2
	je .end1
	cmp dword[edi+deflate_state.lookahead],0
	jne .end1
	cmp dword[wrap],1
	jne @f
	cmp dword[edi+deflate_state.status],INIT_STATE
	je @f
	.end1: ;if (..==.. || .. || (..==.. && ..!=..)) return ..
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	@@:

	; when using zlib wrappers, compute Adler-32 for provided dictionary
	cmp dword[wrap],1
	jne @f ;if (..==..)
		stdcall adler32, [ebx+z_stream.adler], [dictionary], [dictLength]
		mov [ebx+z_stream.adler],eax
	@@:
	mov dword[edi+deflate_state.wrap],0 ;avoid computing Adler-32 in read_buf

	; if dictionary would fill window, just replace the history
	mov eax,[edi+deflate_state.w_size]
	cmp [dictLength],eax
	jl .end2 ;if (..>=..)
		cmp dword[wrap],0
		jne @f ;if (..==0) ;already empty otherwise
			CLEAR_HASH edi
			mov dword[edi+deflate_state.strstart],0
			mov dword[edi+deflate_state.block_start],0
			mov dword[edi+deflate_state.insert],0
		@@:
		mov eax,[dictLength]
		sub eax,[edi+deflate_state.w_size]
		add [dictionary],eax ;use the tail
		mov eax,[edi+deflate_state.w_size]
		mov [dictLength],eax
	.end2:

	; insert dictionary into window and hash
	mov eax,[ebx+z_stream.avail_in]
	mov [avail],eax
	mov eax,[ebx+z_stream.next_in]
	mov [next],eax
	mov eax,[dictLength]
	mov [ebx+z_stream.avail_in],eax
	mov eax,[dictionary]
	mov [ebx+z_stream.next_in],eax
	stdcall fill_window, edi
	.cycle0: ;while (..>=..)
		mov ecx,[edi+deflate_state.lookahead]
		cmp ecx,MIN_MATCH
		jl .cycle0end
		mov esi,[edi+deflate_state.strstart]
		;esi = str
		sub ecx,MIN_MATCH-1
		.cycle1: ;do
			mov eax,[edi+deflate_state.window]
			add eax,esi
			add eax,MIN_MATCH-1
			movzx eax,byte[eax]
			UPDATE_HASH edi, [edi+deflate_state.ins_h], eax
if FASTEST eq 0
			mov edx,[edi+deflate_state.ins_h]
			shl edx,1
			add edx,[edi+deflate_state.head]
			movzx edx,word[edx] ;edx = s.head[s.ins_h]
			mov eax,esi
			and eax,[edi+deflate_state.w_mask]
			shl eax,1
			add eax,[edi+deflate_state.prev]
			mov [eax],dx
end if
			mov edx,[edi+deflate_state.ins_h]
			shl edx,1
			add edx,[edi+deflate_state.head]
			mov [edx],si ;s.head[s.ins_h] = str
			inc esi
			dec ecx
			jnz .cycle1 ;while (--..)
		mov [edi+deflate_state.strstart],esi
		mov [edi+deflate_state.lookahead],MIN_MATCH-1
		stdcall fill_window, edi
		jmp .cycle0
align 4
	.cycle0end:
	mov eax,[edi+deflate_state.strstart]
	add eax,[edi+deflate_state.lookahead]
	mov [edi+deflate_state.strstart],eax
	mov [edi+deflate_state.block_start],eax
	mov eax,[edi+deflate_state.lookahead]
	mov [edi+deflate_state.insert],eax
	mov dword[edi+deflate_state.lookahead],0
	mov eax,MIN_MATCH-1
	mov [edi+deflate_state.prev_length],eax
	mov [edi+deflate_state.match_length],eax
	mov dword[edi+deflate_state.match_available],0
	mov eax,[next]
	mov [ebx+z_stream.next_in],eax
	mov eax,[avail]
	mov [ebx+z_stream.avail_in],eax
	mov eax,[wrap]
	mov [edi+deflate_state.wrap],eax
	mov eax,Z_OK
.end_f:
	ret
endp

; =========================================================================
;int (strm)
;    z_streamp strm
align 4
proc deflateResetKeep uses ebx edi, strm:dword
	mov ebx,[strm]
	cmp ebx,Z_NULL
	je @f
	mov edi,[ebx+z_stream.state]
	cmp edi,Z_NULL
	je @f
	cmp dword[ebx+z_stream.zalloc],0
	je @f
	cmp dword[ebx+z_stream.zfree],0
	jne .end0 ;if (..==0 || ..==0 || ..==0 || ..==0)
	@@:
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end0:

	mov dword[ebx+z_stream.total_out],0
	mov dword[ebx+z_stream.total_in],0
	mov dword[ebx+z_stream.msg],Z_NULL ;use zfree if we ever allocate msg dynamically
	mov dword[ebx+z_stream.data_type],Z_UNKNOWN

	mov dword[edi+deflate_state.pending],0
	mov eax,[edi+deflate_state.pending_buf]
	mov [edi+deflate_state.pending_out],eax

	cmp dword[edi+deflate_state.wrap],0
	jge @f ;if (..<0)
		neg dword[edi+deflate_state.wrap] ;was made negative by deflate(..., Z_FINISH)
	@@:
	mov eax,BUSY_STATE
	cmp dword[edi+deflate_state.wrap],0
	je @f
		mov eax,INIT_STATE
	@@:
	mov dword[edi+deflate_state.status],eax
	stdcall adler32, 0, Z_NULL, 0
if GZIP eq 1
	cmp dword[edi+deflate_state.wrap],2
	jne @f
		xor eax,eax ;stdcall calc_crc32, 0, Z_NULL, 0
	@@:
end if
	mov dword[ebx+z_stream.adler],eax
	mov dword[edi+deflate_state.last_flush],Z_NO_FLUSH
	stdcall _tr_init, edi

	mov eax,Z_OK
.end_f:
	ret
endp

; =========================================================================
;int (strm)
;    z_streamp strm
align 4
proc deflateReset uses ebx, strm:dword
	mov ebx,[strm]
	stdcall deflateResetKeep, ebx
	cmp eax,Z_OK
	jne @f ;if (..==Z_OK)
		stdcall lm_init, [ebx+z_stream.state]
	@@:
	ret
endp

; =========================================================================
;int (strm, head)
;    z_streamp strm
;    gz_headerp head
align 4
proc deflateSetHeader uses ebx, strm:dword, head:dword
	mov ebx,[strm]
	cmp ebx,Z_NULL
	je @f
	mov ebx,[ebx+z_stream.state]
	cmp ebx,Z_NULL
	jne .end0
	@@: ;if (..==0 || ..==0) return ..
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end0:
	cmp dword[ebx+deflate_state.wrap],2
	je @f ;if (..!=..) return ..
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	@@:
	mov eax,[head]
	mov [ebx+deflate_state.gzhead],eax
	mov eax,Z_OK
.end_f:
	ret
endp

; =========================================================================
;int (strm, pending, bits)
;    unsigned *pending
;    int *bits
;    z_streamp strm
align 4
proc deflatePending uses ebx edi, strm:dword, pending:dword, bits:dword
	mov ebx,[strm]
	cmp ebx,Z_NULL
	je @f
	mov edi,[ebx+z_stream.state]
	cmp edi,Z_NULL
	jne .end0
	@@: ;if (..==0 || ..==0) return ..
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end0:
	cmp dword[pending],Z_NULL
	je @f ;if (..!=..)
		mov eax,[pending]
		mov ebx,[edi+deflate_state.pending]
		mov [eax],ebx
	@@:
	cmp dword[bits],Z_NULL
	je @f ;if (..!=..)
		mov eax,[bits]
		mov ebx,[edi+deflate_state.bi_valid]
		mov [eax],ebx
	@@:
	mov eax,Z_OK
.end_f:
	ret
endp

; =========================================================================
;int (strm, bits, value)
;    z_streamp strm
;    int bits
;    int value
align 4
proc deflatePrime uses ebx edi, strm:dword, bits:dword, value:dword
;    int put;

	mov ebx,[strm]
	cmp ebx,Z_NULL
	je @f
	mov edi,[ebx+z_stream.state] ;s = strm.state
	cmp edi,Z_NULL
	jne .end0
	@@: ;if (..==0 || ..==0) return ..
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end0:
;    if ((Bytef *)(s->d_buf) < s->pending_out + ((Buf_size + 7) >> 3))
;        return Z_BUF_ERROR;
;    do {
;        put = Buf_size - s->bi_valid;
;        if (put > bits)
;            put = bits;
;        s->bi_buf |= (uint_16)((value & ((1 << put) - 1)) << s->bi_valid);
;        s->bi_valid += put;
;        _tr_flush_bits(s);
;        value >>= put;
;        bits -= put;
;    } while (bits);
	mov eax,Z_OK
.end_f:
	ret
endp

; =========================================================================
;int (strm, level, strategy)
;    z_streamp strm
;    int level
;    int strategy
align 4
proc deflateParams uses ebx edi, strm:dword, level:dword, strategy:dword
locals
	co_func dd ?
	err dd Z_OK
endl

	mov ebx,[strm]
	cmp ebx,Z_NULL
	je @f
	mov edi,[ebx+z_stream.state] ;s = strm.state
	cmp edi,Z_NULL
	jne .end0
	@@: ;if (..==0 || ..==0) return ..
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end0:

if FASTEST eq 1
	cmp dword[level],0
	je @f ;if (..!=0)
		mov dword[level],1
	@@:
else
	cmp dword[level],Z_DEFAULT_COMPRESSION
	jne @f ;if (..==..)
		mov dword[level],6
	@@:
end if
	cmp dword[level],0
	jl @f
	cmp dword[level],9
	jg @f
	cmp dword[strategy],0
	jl @f
	cmp dword[strategy],Z_FIXED
	jle .end1
	@@: ;if (..<0 || ..>9 || ..<0 || ..>..)
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end1:
	movzx eax,word[edi+deflate_state.level]
	imul eax,sizeof.config_s
	add eax,configuration_table+config_s.co_func
	mov [co_func],eax

;    if ((strategy != s->strategy || co_func != configuration_table[level].func) &&
;        strm->total_in != 0) {
		; Flush the last buffer:
;        err = deflate(strm, Z_BLOCK);
;        if (err == Z_BUF_ERROR && s->pending == 0)
;            err = Z_OK;
;    }
;    if (s->level != level) {
;        s->level = level;
;        s->max_lazy_match   = configuration_table[level].max_lazy;
;        s->good_match       = configuration_table[level].good_length;
;        s->nice_match       = configuration_table[level].nice_length;
;        s->max_chain_length = configuration_table[level].max_chain;
;    }
	mov eax,[strategy]
	mov [edi+deflate_state.strategy],ax
	mov eax,[err]
.end_f:
	ret
endp

; =========================================================================
;int (strm, good_length, max_lazy, nice_length, max_chain)
;    z_streamp strm
;    int good_length
;    int max_lazy
;    int nice_length
;    int max_chain
align 4
proc deflateTune uses ebx, strm:dword, good_length:dword, max_lazy:dword,\
			nice_length:dword, max_chain:dword
	mov ebx,[strm]
	cmp ebx,Z_NULL
	je @f
	cmp dword[ebx+z_stream.state],Z_NULL
	jne .end0
	@@: ;if (..==0 || ..==0) return ..
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end0:
	mov ebx,[ebx+z_stream.state] ;s = strm.state
	mov eax,[good_length]
	mov [ebx+deflate_state.good_match],eax
	mov eax,[max_lazy]
	mov [ebx+deflate_state.max_lazy_match],eax
	mov eax,[nice_length]
	mov [ebx+deflate_state.nice_match],eax
	mov eax,[max_chain]
	mov [ebx+deflate_state.max_chain_length],eax
	mov eax,Z_OK
.end_f:
	ret
endp

; =========================================================================
; For the default windowBits of 15 and memLevel of 8, this function returns
; a close to exact, as well as small, upper bound on the compressed size.
; They are coded as constants here for a reason--if the #define's are
; changed, then this function needs to be changed as well.  The return
; value for 15 and 8 only works for those exact settings.

; For any setting other than those defaults for windowBits and memLevel,
; the value returned is a conservative worst case for the maximum expansion
; resulting from using fixed blocks instead of stored blocks, which deflate
; can emit on compressed data for some combinations of the parameters.

; This function could be more sophisticated to provide closer upper bounds for
; every combination of windowBits and memLevel.  But even the conservative
; upper bound of about 14% expansion does not seem onerous for output buffer
; allocation.

;uLong (z_streamp strm, uLong sourceLen)
align 4
proc deflateBound uses ebx edi, strm:dword, sourceLen:dword
locals
	complen dd ?
	wraplen dd ?
endl
;edi = s

	; conservative upper bound for compressed data
	mov ebx,[sourceLen]
	mov eax,ebx
	add eax,7
	shr eax,3
	add eax,ebx
	add ebx,63
	shr ebx,6
	lea eax,[eax+ebx+5]
	mov [complen],eax

	; if can't get parameters, return conservative bound plus zlib wrapper
	mov eax,[strm]
	cmp eax,Z_NULL
	je .end0
	mov edi,[eax+z_stream.state] ;s = strm.state
	cmp edi,Z_NULL
	jne @f
	.end0: ;if (..==0 || ..==0)
		mov eax,[complen]
		add eax,6
		jmp .end_f
	@@:

	; compute wrapper length
	mov ebx,[edi+deflate_state.wrap]
	cmp ebx,0
	je .end1
	cmp ebx,1
	je .end2
	cmp ebx,2
	je .end3
	jmp .end4
	.end1: ;raw deflate
		mov dword[wraplen],0
		jmp .end5
	.end2: ;zlib wrapper
		mov eax,[edi+deflate_state.strstart]
		neg eax
		sbb eax,eax
		and eax,4
		add eax,6
		mov [wraplen],eax
		jmp .end5
	.end3: ;gzip wrapper
		mov dword[wraplen],18
		cmp dword[edi+deflate_state.gzhead],Z_NULL ;user-supplied gzip header
		je .end5
		mov eax,[edi+deflate_state.gzhead]
		cmp dword[eax+gz_header.extra],0
		je @f
			mov eax,[edi+deflate_state.gzhead]
			mov eax,[eax+gz_header.extra_len]
			add dword[wraplen],eax
			add dword[wraplen],2
		@@:
		mov eax,[edi+deflate_state.gzhead]
		mov eax,[eax+gz_header.name]
		cmp eax,0
		je @f
		.cycle0: ;do
			inc dword[wraplen]
			movzx ebx,byte[eax]
			inc eax
			test ebx,ebx
			jne .cycle0
		@@:
		mov eax,[edi+deflate_state.gzhead]
		mov eax,[eax+gz_header.comment]
		cmp eax,0
		je @f
		.cycle1: ;do
			inc dword[wraplen]
			movzx ebx,byte[eax]
			inc eax
			test ebx,ebx
			jne .cycle1
		@@:
		mov eax,[edi+deflate_state.gzhead]
		cmp dword[eax+gz_header.hcrc],0
		je .end5
			add dword[wraplen],2
		jmp .end5
	.end4: ;for compiler happiness
		mov dword[wraplen],6
	.end5:

	; if not default parameters, return conservative bound
	cmp dword[edi+deflate_state.w_bits],15
	jne .end6
	cmp dword[edi+deflate_state.hash_bits],8+7
	je @f
	.end6: ;if (s->w_bits !=.. || s->hash_bits !=..)
		mov eax,[complen]
		add eax,[wraplen]
		jmp .end_f
	@@:

	; default settings: return tight bound for that case
	mov eax,[sourceLen]
	mov ebx,eax
	shr ebx,12
	add ebx,eax
	mov edi,eax
	shr edi,14
	add ebx,edi
	shr eax,25
	add ebx,[wraplen]
	lea eax,[eax+ebx+7]
.end_f:
	ret
endp

; =========================================================================
; Put a short in the pending buffer. The 16-bit value is put in MSB order.
; IN assertion: the stream state is correct and there is enough room in
; pending_buf.

;void (deflate_state *s, uInt b)
align 4
proc putShortMSB uses eax ebx ecx, s:dword, b:dword
	mov ebx,[s]
	mov ecx,[b]
	put_byte ebx, ch
	put_byte ebx, cl
	ret
endp

; =========================================================================
; Flush as much pending output as possible. All deflate() output goes
; through this function so some applications may wish to modify it
; to avoid allocating a large strm->next_out buffer and copying into it.
; (See also read_buf()).

;void (z_streamp strm)
align 16
proc flush_pending uses eax ebx ecx edx, strm:dword
;ecx - len
;edx - deflate_state *s
;ebx - strm
	mov ebx,[strm]
	mov edx,[ebx+z_stream.state]

	stdcall _tr_flush_bits, edx
	mov ecx,[edx+deflate_state.pending]
	mov eax,[ebx+z_stream.avail_out]
	cmp ecx,eax
	jbe @f ;if (..>..)
		mov ecx,eax
	@@:
	test ecx,ecx
	jz @f

	stdcall zmemcpy, [ebx+z_stream.next_out], [edx+deflate_state.pending_out], ecx
	add [ebx+z_stream.next_out],ecx
	add [edx+deflate_state.pending_out],ecx
	add [ebx+z_stream.total_out],ecx
	sub [ebx+z_stream.avail_out],ecx
	sub [edx+deflate_state.pending],ecx
	cmp dword[edx+deflate_state.pending],0
	jne @f ;if (..==0)
		mov eax,[edx+deflate_state.pending_buf]
		mov [edx+deflate_state.pending_out],eax
	@@:
	ret
endp

; =========================================================================
;int (strm, flush)
;    z_streamp strm
;    int flush
align 16
proc deflate uses ebx ecx edx edi esi, strm:dword, flush:dword
locals
	old_flush dd ? ;int ;value of flush param for previous deflate call
	val dd ?
endl
	mov ebx,[strm]
	cmp ebx,Z_NULL
	je @f
	mov edi,[ebx+z_stream.state] ;s = strm.state
	cmp edi,Z_NULL
	je @f
	cmp dword[flush],Z_BLOCK
	jg @f
	cmp dword[flush],0
	jge .end10 ;if (..==0 || ..==0 || ..>.. || ..<0)
	@@:
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end10:
	cmp dword[ebx+z_stream.next_out],Z_NULL
	je .beg0
	cmp dword[ebx+z_stream.next_in],Z_NULL
	jne @f
	cmp dword[ebx+z_stream.avail_in],0
	jne .beg0
	@@:
	cmp dword[edi+deflate_state.status],FINISH_STATE
	jne .end0
	cmp dword[flush],Z_FINISH
	je .end0
	.beg0: ;if (..==0 || (..==0 && ..!=0) || (..==.. && ..!=..))
		ERR_RETURN ebx, Z_STREAM_ERROR
		jmp .end_f
	.end0:
	cmp dword[ebx+z_stream.avail_out],0
	jne @f ;if (..==0)
		ERR_RETURN ebx, Z_BUF_ERROR
		jmp .end_f
	@@:

	mov dword[edi+deflate_state.strm],ebx ;just in case
	mov eax,[edi+deflate_state.last_flush]
	mov [old_flush],eax
	mov eax,[flush]
	mov [edi+deflate_state.last_flush],eax

	; Write the header
	cmp dword[edi+deflate_state.status],INIT_STATE
	jne .end2 ;if (..==..)
if GZIP eq 1
		cmp dword[edi+deflate_state.wrap],2
		jne .end1 ;if (..==..)
			xor eax,eax ;stdcall calc_crc32, 0, Z_NULL, 0
			mov [ebx+z_stream.adler],eax
			put_byte edi, 31
			put_byte edi, 139
			put_byte edi, 8
			cmp dword[edi+deflate_state.gzhead],Z_NULL
			jne .end3 ;if (..==0)
				put_byte edi, 0
				put_dword edi, 0
				xor cl,cl
				cmp word[edi+deflate_state.level],2
				jge @f
					mov cl,4
				@@:
				cmp word[edi+deflate_state.strategy],Z_HUFFMAN_ONLY
				jl @f
					mov cl,4
				@@:
				cmp word[edi+deflate_state.level],9
				jne @f
					mov cl,2
				@@: ;..==.. ? 2 : (..>=.. || ..<.. ? 4 : 0)
				put_byte edi, cl
				put_byte edi, OS_CODE
				mov dword[edi+deflate_state.status],BUSY_STATE
				jmp .end2
			.end3: ;else
				mov edx,[edi+deflate_state.gzhead]
				xor cl,cl
				cmp [edx+gz_header.text],0
				je @f
					inc cl
				@@:
				cmp [edx+gz_header.hcrc],0
				je @f
					add cl,2
				@@:
				cmp [edx+gz_header.extra],Z_NULL
				je @f
					add cl,4
				@@:
				cmp [edx+gz_header.name],Z_NULL
				je @f
					add cl,8
				@@:
				cmp [edx+gz_header.comment],Z_NULL
				je @f
					add cl,16
				@@:
				put_byte edi, cl
				mov ecx,[edx+gz_header.time]
				put_dword edi, ecx
				xor cl,cl
				cmp word[edi+deflate_state.level],2
				jge @f
					mov cl,4
				@@:
				cmp word[edi+deflate_state.strategy],Z_HUFFMAN_ONLY
				jl @f
					mov cl,4
				@@:
				cmp word[edi+deflate_state.level],9
				jne @f
					mov cl,2
				@@: ;..==.. ? 2 : (..>=.. || ..<.. ? 4 : 0)
				put_byte edi, cl
				mov ecx,[edx+gz_header.os]
				put_byte edi, cl
				cmp dword[edx+gz_header.extra],Z_NULL
				je @f ;if (..!=0)
					mov ecx,[edx+gz_header.extra_len]
					put_byte edi, cl
					put_byte edi, ch
				@@:
				cmp dword[edx+gz_header.hcrc],0
				je @f ;if (..)
					stdcall calc_crc32, [ebx+z_stream.adler],\
						[edi+deflate_state.pending_buf], [edi+deflate_state.pending]
					mov [ebx+z_stream.adler],eax
				@@:
				mov dword[edi+deflate_state.gzindex],0
				mov dword[edi+deflate_state.status],EXTRA_STATE
			jmp .end2
		.end1: ;else
end if
			mov edx,[edi+deflate_state.w_bits]
			sub edx,8
			shl edx,4
			add edx,Z_DEFLATED
			shl edx,8 ;edx = header
			;esi = level_flags

			mov esi,3
			cmp word[edi+deflate_state.strategy],Z_HUFFMAN_ONLY
			jge @f
			cmp word[edi+deflate_state.level],2
			jge .end30 ;if (..>=.. || ..<..)
			@@:
				xor esi,esi
				jmp .end4
			.end30:
			cmp word[edi+deflate_state.level],6
			jge @f ;else if (..<..)
				mov esi,1
				jmp .end4
			@@:
			;;cmp word[edi+deflate_state.level],6
			jne .end4 ;else if (..==..)
				mov esi,2
			.end4:
			shl esi,6
			or edx,esi
			cmp dword[edi+deflate_state.strstart],0
			je @f ;if (..!=0)
				or edx,PRESET_DICT
			@@:
			mov esi,edx
			mov eax,edx
			xor edx,edx
			mov ecx,31
			div ecx
			add esi,31
			sub esi,edx ;esi = header

			mov dword[edi+deflate_state.status],BUSY_STATE
			stdcall putShortMSB, edi, esi

			; Save the adler32 of the preset dictionary:
			cmp dword[edi+deflate_state.strstart],0
			je @f ;if (..!=0)
				mov ecx,[ebx+z_stream.adler]
				bswap ecx
				put_dword edi, ecx
			@@:
			stdcall adler32, 0,0,0
			mov [ebx+z_stream.adler],eax
	.end2:
if GZIP eq 1
	mov edx,[edi+deflate_state.gzhead]
	cmp dword[edi+deflate_state.status],EXTRA_STATE
	jne .end5 ;if (..==..)
		cmp dword[edx+gz_header.extra],Z_NULL
		je .end21 ;if (..!=..)
			mov esi,[edi+deflate_state.pending]
			;esi = beg ;start of bytes to update crc

			movzx ecx,word[edx+gz_header.extra_len]
align 4
			.cycle0: ;while (..<..)
			cmp dword[edi+deflate_state.gzindex],ecx
			jge .cycle0end
				mov eax,[edi+deflate_state.pending]
				cmp eax,[edi+deflate_state.pending_buf_size]
				jne .end24 ;if (..==..)
					mov dword[edx+gz_header.hcrc],0
					je @f
					cmp [edi+deflate_state.pending],esi
					jle @f ;if (.. && ..>..)
						mov ecx,[edi+deflate_state.pending]
						sub ecx,esi
						mov eax,[edi+deflate_state.pending_buf]
						add eax,esi
						stdcall calc_crc32, [ebx+z_stream.adler], eax, ecx
						mov [ebx+z_stream.adler],eax
					@@:
					stdcall flush_pending, ebx
					mov esi,[edi+deflate_state.pending]
					cmp esi,[edi+deflate_state.pending_buf_size]
					je .cycle0end ;if (..==..) break
				.end24:
				push ebx
					mov ebx,[edi+deflate_state.gzindex]
					add ebx,[edx+gz_header.extra]
					mov bl,[ebx]
					put_byte edi, bl
				pop ebx
				inc dword[edi+deflate_state.gzindex]
				jmp .cycle0
			.cycle0end:
			mov dword[edx+gz_header.hcrc],0
			je @f
			cmp [edi+deflate_state.pending],esi
			jle @f ;if (.. && ..>..)
				mov ecx,[edi+deflate_state.pending]
				sub ecx,esi
				mov eax,[edi+deflate_state.pending_buf]
				add eax,esi
				stdcall calc_crc32, [ebx+z_stream.adler], eax, ecx
				mov [ebx+z_stream.adler],eax
			@@:
			mov eax,[edx+gz_header.extra_len]
			cmp dword[edi+deflate_state.gzindex],eax
			jne .end5 ;if (..==..)
				mov dword[edi+deflate_state.gzindex],0
				mov dword[edi+deflate_state.status],NAME_STATE
			jmp .end5
		.end21: ;else
			mov dword[edi+deflate_state.status],NAME_STATE
	.end5:
	cmp dword[edi+deflate_state.status],NAME_STATE
	jne .end6 ;if (..==..)
		cmp dword[edx+gz_header.name],Z_NULL
		je .end22 ;if (..!=..)
			mov esi,[edi+deflate_state.pending]
			;esi = beg ;start of bytes to update crc

			.cycle1: ;do
				mov eax,[edi+deflate_state.pending]
				cmp eax,[edi+deflate_state.pending_buf_size]
				jne .end25 ;if (..==..)
					mov dword[edx+gz_header.hcrc],0
					je @f
					cmp [edi+deflate_state.pending],esi
					jle @f ;if (.. && ..>..)
						mov ecx,[edi+deflate_state.pending]
						sub ecx,esi
						mov eax,[edi+deflate_state.pending_buf]
						add eax,esi
						stdcall calc_crc32, [ebx+z_stream.adler], eax, ecx
						mov [ebx+z_stream.adler],eax
					@@:
					stdcall flush_pending, ebx
					mov esi,[edi+deflate_state.pending]
					cmp esi,[edi+deflate_state.pending_buf_size]
					jne .end25 ;if (..==..)
						mov dword[val],1
						jmp .cycle1end
				.end25:
				push ebx
					mov ebx,[edi+deflate_state.gzindex]
					add ebx,[edx+gz_header.name]
					movzx ebx,byte[ebx]
					mov [val],ebx
					inc dword[edi+deflate_state.gzindex]
					put_byte edi, bl
				pop ebx
				cmp dword[val],0
				jne .cycle1 ;while (val != 0)
			.cycle1end:
			mov dword[edx+gz_header.hcrc],0
			je @f
			cmp [edi+deflate_state.pending],esi
			jle @f ;if (.. && ..>..)
				mov ecx,[edi+deflate_state.pending]
				sub ecx,esi
				mov eax,[edi+deflate_state.pending_buf]
				add eax,esi
				stdcall calc_crc32, [ebx+z_stream.adler], eax, ecx
				mov [ebx+z_stream.adler],eax
			@@:
			cmp dword[val],0
			jne .end6 ;if (val == 0)
				mov dword[edi+deflate_state.gzindex],0
				mov dword[edi+deflate_state.status],COMMENT_STATE
			jmp .end6
		.end22: ;else
			mov dword[edi+deflate_state.status],COMMENT_STATE
	.end6:
	cmp dword[edi+deflate_state.status],COMMENT_STATE
	jne .end7 ;if (..==..)
		cmp dword[edx+gz_header.comment],Z_NULL
		je .end23 ;if (..!=..)
			mov esi,[edi+deflate_state.pending]
			;esi = beg ;start of bytes to update crc

			.cycle2: ;do
				mov eax,[edi+deflate_state.pending]
				cmp eax,[edi+deflate_state.pending_buf_size]
				jne .end26 ;if (..==..)
					mov dword[edx+gz_header.hcrc],0
					je @f
					cmp [edi+deflate_state.pending],esi
					jle @f ;if (.. && ..>..)
						mov ecx,[edi+deflate_state.pending]
						sub ecx,esi
						mov eax,[edi+deflate_state.pending_buf]
						add eax,esi
						stdcall calc_crc32, [ebx+z_stream.adler], eax, ecx
						mov [ebx+z_stream.adler],eax
					@@:
					stdcall flush_pending, ebx
					mov esi,[edi+deflate_state.pending]
					cmp esi,[edi+deflate_state.pending_buf_size]
					jne .end26 ;if (..==..)
						mov dword[val],1
						jmp .cycle2end
				.end26:
				push ebx
					mov ebx,[edi+deflate_state.gzindex]
					add ebx,[edx+gz_header.comment]
					movzx ebx,byte[ebx]
					mov [val],ebx
					inc dword[edi+deflate_state.gzindex]
					put_byte edi, bl
				pop ebx
				cmp dword[val],0
				jne .cycle2 ;while (val != 0)
			.cycle2end:
			mov dword[edx+gz_header.hcrc],0
			je @f
			cmp [edi+deflate_state.pending],esi
			jle @f ;if (.. && ..>..)
				mov ecx,[edi+deflate_state.pending]
				sub ecx,esi
				mov eax,[edi+deflate_state.pending_buf]
				add eax,esi
				stdcall calc_crc32, [ebx+z_stream.adler], eax, ecx
				mov [ebx+z_stream.adler],eax
			@@:
			cmp dword[val],0
			jne .end7 ;if (val == 0)
				mov dword[edi+deflate_state.status],HCRC_STATE
			jmp .end7
		.end23: ;else
			mov dword[edi+deflate_state.status],HCRC_STATE
	.end7:
	cmp dword[edi+deflate_state.status],HCRC_STATE
	jne .end8 ;if (..==..)
		cmp dword[edx+gz_header.hcrc],0
		je .end9 ;if (..)
			mov ecx,[edi+deflate_state.pending]
			add ecx,2
			cmp ecx,[edi+deflate_state.pending_buf_size]
			jbe @f ;if (..>..)
				stdcall flush_pending, ebx
			@@:
			mov ecx,[edi+deflate_state.pending]
			add ecx,2
			cmp ecx,[edi+deflate_state.pending_buf_size]
			ja .end8 ;if (..<=..)
				mov ecx,[ebx+z_stream.adler]
				put_byte edi, cl
				put_byte edi, ch
				xor eax,eax ;stdcall calc_crc32, 0, Z_NULL, 0
				mov [ebx+z_stream.adler],eax
				mov dword[edi+deflate_state.status],BUSY_STATE
			jmp .end8
		.end9: ;else
			mov dword[edi+deflate_state.status],BUSY_STATE
	.end8:
end if

	; Flush as much pending output as possible
	cmp dword[edi+deflate_state.pending],0
	je .end13 ;if (..!=0)
		stdcall flush_pending, ebx
		cmp dword[ebx+z_stream.avail_out],0
		jne @f ;if (..==0)
			; Since avail_out is 0, deflate will be called again with
			; more output space, but possibly with both pending and
			; avail_in equal to zero. There won't be anything to do,
			; but this is not an error situation so make sure we
			; return OK instead of BUF_ERROR at next call of deflate:

			mov dword[edi+deflate_state.last_flush],-1
			mov eax,Z_OK
			jmp .end_f
		; Make sure there is something to do and avoid duplicate consecutive
		; flushes. For repeated and useless calls with Z_FINISH, we keep
		; returning Z_STREAM_END instead of Z_BUF_ERROR.
align 4
	.end13:
	cmp dword[ebx+z_stream.avail_in],0
	jne @f
	RANK dword[old_flush],esi
	RANK dword[flush],eax
	cmp eax,esi
	jg @f
	cmp dword[flush],Z_FINISH
	je @f ;else if (..==0 && ..<=.. && ..!=..)
		ERR_RETURN ebx, Z_BUF_ERROR
		jmp .end_f
	@@:

	; User must not provide more input after the first FINISH:
	cmp dword[edi+deflate_state.status],FINISH_STATE
	jne @f
	cmp dword[ebx+z_stream.avail_in],0
	je @f ;if (..==.. && ..!=0)
		ERR_RETURN ebx, Z_BUF_ERROR
		jmp .end_f
	@@:

	; Start a new block or continue the current one.

	cmp dword[ebx+z_stream.avail_in],0
	jne @f
	cmp dword[edi+deflate_state.lookahead],0
	jne @f
	cmp dword[flush],Z_NO_FLUSH
	je .end11
	cmp dword[edi+deflate_state.status],FINISH_STATE
	je .end11
	@@: ;if (..!=0 || ..!=0 || (..!=.. && ..!=..))
		;edx = bstate
		cmp word[edi+deflate_state.strategy],Z_HUFFMAN_ONLY
		jne @f
			stdcall deflate_huff, edi, [flush]
			jmp .end20
		@@:
		cmp word[edi+deflate_state.strategy],Z_RLE
		jne @f
			stdcall deflate_rle, edi, [flush]
			jmp .end20
		@@:
		movzx eax,word[edi+deflate_state.level]
		imul eax,sizeof.config_s
		add eax,configuration_table+config_s.co_func
		stdcall dword[eax], edi, [flush]
		.end20:
		mov edx,eax

		cmp edx,finish_started
		je @f
		cmp edx,finish_done
		jne .end18
		@@: ;if (..==.. || ..==..)
			mov dword[edi+deflate_state.status],FINISH_STATE
		.end18:
		cmp edx,need_more
		je @f
		cmp edx,finish_started
		jne .end19
		@@: ;if (..==.. || ..==..)
			cmp dword[ebx+z_stream.avail_out],0
			jne @f ;if (..==0)
				mov dword[edi+deflate_state.last_flush],-1 ;avoid BUF_ERROR next call, see above
			@@:
			mov eax,Z_OK
			jmp .end_f
			; If flush != Z_NO_FLUSH && avail_out == 0, the next call
			; of deflate should use the same flush parameter to make sure
			; that the flush is complete. So we don't have to output an
			; empty block here, this will be done at next call. This also
			; ensures that for a very small output buffer, we emit at most
			; one empty block.

		.end19:
		cmp edx,block_done
		jne .end11 ;if (..==..)
			cmp dword[flush],Z_PARTIAL_FLUSH
			jne @f ;if (..==..)
				stdcall _tr_align, edi
				jmp .end16
			@@:
			cmp dword[flush],Z_BLOCK
			je .end16 ;else if (..!=..) ;FULL_FLUSH or SYNC_FLUSH
				stdcall _tr_stored_block, edi, 0, 0, 0
				; For a full flush, this empty block will be recognized
				; as a special marker by inflate_sync().

			cmp dword[flush],Z_FULL_FLUSH
			jne .end16 ;if (..==..)
				CLEAR_HASH edi ;forget history
				cmp dword[edi+deflate_state.lookahead],0
				jne .end16 ;if (..==0)
					mov dword[edi+deflate_state.strstart],0
					mov dword[edi+deflate_state.block_start],0
					mov dword[edi+deflate_state.insert],0
		.end16:
		stdcall flush_pending, ebx
		cmp dword[ebx+z_stream.avail_out],0
		jne .end11 ;if (..==0)
			mov dword[edi+deflate_state.last_flush],-1 ;avoid BUF_ERROR at next call, see above
			mov eax,Z_OK
			jmp .end_f
	.end11:
	cmp dword[ebx+z_stream.avail_out],0
	jg @f
		zlib_assert 'bug2' ;Assert(..>0)
	@@:

	cmp dword[flush],Z_FINISH
	je @f ;if (..!=0)
		mov eax,Z_OK
		jmp .end_f
	@@:
	cmp dword[edi+deflate_state.wrap],0
	jg @f ;if (..<=0)
		mov eax,Z_STREAM_END
		jmp .end_f
	@@:

	; Write the trailer
if GZIP eq 1
	cmp dword[edi+deflate_state.wrap],2
	jne @f ;if (..==..)
		mov ecx,[ebx+z_stream.adler]
		put_dword edi, ecx
		mov ecx,[ebx+z_stream.total_in]
		put_dword edi, ecx
		jmp .end17
	@@: ;else
end if
		mov ecx,[ebx+z_stream.adler]
		bswap ecx
		put_dword edi, ecx
	.end17:
	stdcall flush_pending, ebx
	; If avail_out is zero, the application will call deflate again
	; to flush the rest.

	cmp dword[edi+deflate_state.wrap],0
	jle @f ;if (..>0) ;write the trailer only once!
		neg dword[edi+deflate_state.wrap]
	@@:
	mov eax,Z_OK
	cmp dword[edi+deflate_state.pending],0
	jne .end_f
		mov eax,Z_STREAM_END
.end_f:
	ret
endp

; =========================================================================
;int (strm)
;    z_streamp strm
align 4
proc deflateEnd uses ebx ecx edx, strm:dword
	mov ebx,[strm]
	cmp ebx,Z_NULL
	je @f
	mov edx,[ebx+z_stream.state]
	cmp edx,Z_NULL
	jne .end0
	@@: ;if (..==0 || ..==0) return ..
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end0:

	mov ecx,[edx+deflate_state.status]
	cmp ecx,INIT_STATE
	je @f
	cmp ecx,EXTRA_STATE
	je @f
	cmp ecx,NAME_STATE
	je @f
	cmp ecx,COMMENT_STATE
	je @f
	cmp ecx,HCRC_STATE
	je @f
	cmp ecx,BUSY_STATE
	je @f
	cmp ecx,FINISH_STATE
	je @f ;if (..!=.. && ..!=.. && ..!=.. && ..!=.. && ..!=.. && ..!=.. && ..!=..)
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	@@:

	; Deallocate in reverse order of allocations:
	TRY_FREE ebx, dword[edx+deflate_state.pending_buf]
	TRY_FREE ebx, dword[edx+deflate_state.head]
	TRY_FREE ebx, dword[edx+deflate_state.prev]
	TRY_FREE ebx, dword[edx+deflate_state.window]

	ZFREE ebx, dword[ebx+z_stream.state]
	mov dword[ebx+z_stream.state],Z_NULL

	mov eax,Z_DATA_ERROR
	cmp ecx,BUSY_STATE
	je .end_f
		mov eax,Z_OK
.end_f:
	ret
endp

; =========================================================================
; Copy the source state to the destination state.
; To simplify the source, this is not supported for 16-bit MSDOS (which
; doesn't have enough memory anyway to duplicate compression states).

;int (dest, source)
;    z_streamp dest
;    z_streamp source
align 4
proc deflateCopy uses ebx edx edi esi, dest:dword, source:dword
;ebx = overlay ;uint_16p
;edi = ds ;deflate_state*
;esi = ss ;deflate_state*

	mov esi,[source]
	cmp esi,Z_NULL
	je @f
	mov edx,[dest]
	cmp edx,Z_NULL
	je @f
	mov esi,[esi+z_stream.state]
	cmp esi,Z_NULL
	jne .end0
	@@: ;if (..==0 || ..==0 || ..==0)
		mov eax,Z_STREAM_ERROR
		jmp .end_f
	.end0:

	stdcall zmemcpy, edx, [source], sizeof.z_stream

	ZALLOC edx, 1, sizeof.deflate_state
	cmp eax,0
	jne @f ;if (..==0) return ..
		mov eax,Z_MEM_ERROR
		jmp .end_f
	@@:
	mov edi,eax
	mov [edx+z_stream.state],eax
	stdcall zmemcpy, edi, esi, sizeof.deflate_state
	mov dword[edi+deflate_state.strm],edx

	ZALLOC edx, [edi+deflate_state.w_size], 2 ;2*sizeof.db
	mov dword[edi+deflate_state.window],eax
	ZALLOC edx, [edi+deflate_state.w_size], 2 ;sizeof.dw
	mov dword[edi+deflate_state.prev],eax
	ZALLOC edx, [edi+deflate_state.hash_size], 2 ;sizeof.dw
	mov dword[edi+deflate_state.head],eax
	ZALLOC edx, [edi+deflate_state.lit_bufsize], 4 ;sizeof.dw+2
	mov ebx,eax
	mov dword[edi+deflate_state.pending_buf],eax

	cmp dword[edi+deflate_state.window],Z_NULL
	je @f
	cmp dword[edi+deflate_state.prev],Z_NULL
	je @f
	cmp dword[edi+deflate_state.head],Z_NULL
	je @f
	cmp dword[edi+deflate_state.pending_buf],Z_NULL
	jne .end1
	@@: ;if (..==0 || ..==0 || ..==0 || ..==0)
		stdcall deflateEnd, edx
		mov eax,Z_MEM_ERROR
		jmp .end_f
	.end1:

	; following zmemcpy do not work for 16-bit MSDOS
	mov eax,[edi+deflate_state.w_size]
	shl eax,1 ;*= 2*sizeof.db
	stdcall zmemcpy, [edi+deflate_state.window], [esi+deflate_state.window], eax
	mov eax,[edi+deflate_state.w_size]
	shl eax,1 ;*= sizeof.dw
	stdcall zmemcpy, [edi+deflate_state.prev], [esi+deflate_state.prev], eax
	mov eax,[edi+deflate_state.hash_size]
	shl eax,1 ;*= sizeof.dw
	stdcall zmemcpy, [edi+deflate_state.head], [esi+deflate_state.head], eax
	stdcall zmemcpy, [edi+deflate_state.pending_buf], [esi+deflate_state.pending_buf], [edi+deflate_state.pending_buf_size]

	mov eax,[edi+deflate_state.pending_buf]
	add eax,[esi+deflate_state.pending_out]
	sub eax,[esi+deflate_state.pending_buf]
	mov [edi+deflate_state.pending_out],eax
	mov eax,[edi+deflate_state.lit_bufsize]
	shr eax,1 ;/=sizeof.uint_16
	add eax,ebx
	mov [edi+deflate_state.d_buf],eax
	mov eax,[edi+deflate_state.lit_bufsize]
	imul eax,3 ;*=1+sizeof.uint_16
	add eax,[edi+deflate_state.pending_buf]
	mov [edi+deflate_state.l_buf],eax

	mov eax,edi
	add eax,deflate_state.dyn_ltree
	mov [edi+deflate_state.l_desc.dyn_tree],eax
	add eax,deflate_state.dyn_dtree-deflate_state.dyn_ltree
	mov [edi+deflate_state.d_desc.dyn_tree],eax
	add eax,deflate_state.bl_tree-deflate_state.dyn_dtree
	mov [edi+deflate_state.bl_desc.dyn_tree],eax

	mov eax,Z_OK
.end_f:
	ret
endp

; ===========================================================================
; Read a new buffer from the current input stream, update the adler32
; and total number of bytes read.  All deflate() input goes through
; this function so some applications may wish to modify it to avoid
; allocating a large strm->next_in buffer and copying from it.
; (See also flush_pending()).

;int (strm, buf, size)
;    z_streamp strm
;    Bytef *buf
;    unsigned size
align 16
proc read_buf uses ebx ecx, strm:dword, buf:dword, size:dword
	mov ebx,[strm]
	mov eax,[ebx+z_stream.avail_in]

	cmp eax,[size]
	jbe @f ;if (..>..)
		mov eax,[size]
	@@:
	cmp eax,0
	jg @f
		xor eax,eax
		jmp .end_f ;if (..==0) return 0
	@@:

	sub [ebx+z_stream.avail_in],eax

	stdcall zmemcpy, [buf],[ebx+z_stream.next_in],eax
	mov ecx,[ebx+z_stream.state]
	cmp dword[ecx+deflate_state.wrap],1
	jne @f ;if (..==..)
		push eax
		stdcall adler32, [ebx+z_stream.adler], [buf], eax
		mov [ebx+z_stream.adler],eax
		pop eax
if GZIP eq 1
		jmp .end0
end if
	@@:
if GZIP eq 1
	cmp dword[ecx+deflate_state.wrap],2
	jne .end0 ;else if (..==..)
		push eax
		stdcall calc_crc32, [ebx+z_stream.adler], [buf], eax
		mov [ebx+z_stream.adler],eax
		pop eax
	.end0:
end if
	add [ebx+z_stream.next_in],eax
	add [ebx+z_stream.total_in],eax

.end_f:
	ret
endp

; ===========================================================================
; Initialize the "longest match" routines for a new zlib stream

;void (deflate_state *s)
align 16
proc lm_init uses eax ebx edi, s:dword
	mov edi,[s]
	mov eax,[edi+deflate_state.w_size]
	shl eax,1
	mov [edi+deflate_state.window_size],eax

	CLEAR_HASH edi

	; Set the default configuration parameters:

	movzx eax,word[edi+deflate_state.level]
	imul eax,sizeof.config_s
	add eax,configuration_table
	movzx ebx,word[eax+config_s.max_lazy]
	mov [edi+deflate_state.max_lazy_match],ebx
	movzx ebx,word[eax+config_s.good_length]
	mov [edi+deflate_state.good_match],ebx
	movzx ebx,word[eax+config_s.nice_length]
	mov [edi+deflate_state.nice_match],ebx
	movzx ebx,word[eax+config_s.max_chain]
	mov [edi+deflate_state.max_chain_length],ebx

	mov dword[edi+deflate_state.strstart],0
	mov dword[edi+deflate_state.block_start],0
	mov dword[edi+deflate_state.lookahead],0
	mov dword[edi+deflate_state.insert],0
	mov dword[edi+deflate_state.prev_length],MIN_MATCH-1
	mov dword[edi+deflate_state.match_length],MIN_MATCH-1
	mov dword[edi+deflate_state.match_available],0
	mov dword[edi+deflate_state.ins_h],0
if FASTEST eq 0
;if ASMV
;    call match_init ;initialize the asm code
;end if
end if
	ret
endp

;uInt (s, cur_match)
;    deflate_state *s
;    IPos cur_match ;current match
align 16
proc longest_match uses ebx ecx edx edi esi, s:dword, cur_match:dword
if FASTEST eq 0
; ===========================================================================
; Set match_start to the longest match starting at the given string and
; return its length. Matches shorter or equal to prev_length are discarded,
; in which case the result is equal to prev_length and match_start is
; garbage.
; IN assertions: cur_match is the head of the hash chain for the current
;   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
; OUT assertion: the match length is not greater than s->lookahead.

;#ifndef ASMV
; For 80x86 and 680x0, an optimized version will be provided in match.asm or
; match.S. The code will be functionally equivalent.
locals
	chain_length dd ? ;unsigned ;max hash chain length
	len        dd ? ;int ;length of current match
	strend     dd ? ;Bytef *
	best_len   dd ? ;int ;best match length so far
	nice_match dd ? ;int ;stop if match long enough
	limit      dd NIL ;IPos
	prev       dd ? ;Posf *
	wmask      dd ? ;uInt
endl
	mov edx,[s]
	mov eax,[edx+deflate_state.max_chain_length]
	mov [chain_length],eax
	mov edi,[edx+deflate_state.window]
	add edi,[edx+deflate_state.strstart]
	;edi - Bytef *scan ;current string
	;esi - Bytef *match ;matched string
	mov eax,[edx+deflate_state.prev_length]
	mov [best_len],eax
	mov eax,[edx+deflate_state.nice_match]
	mov [nice_match],eax

	MAX_DIST edx
	cmp [edx+deflate_state.strstart],eax
	jle @f
		mov ecx,[edx+deflate_state.strstart]
		sub ecx,eax
		mov [limit],ecx
	@@:
	; Stop when cur_match becomes <= limit. To simplify the code,
	; we prevent matches with the string of window index 0.
	mov eax,[edx+deflate_state.prev]
	mov [prev],eax
	mov eax,[edx+deflate_state.w_mask]
	mov [wmask],eax
	mov eax,edi
	add eax,MAX_MATCH ;-1 ???
	mov [strend],eax
	mov eax,[best_len]
	dec eax
	mov bx,[edi+eax]
	;bl - Byte scan_end1
	;bh - Byte scan_end

	; The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
	; It is easy to get rid of this optimization if necessary.

if MAX_MATCH <> 258
	cmp dword[edx+deflate_state.hash_bits],8
	jge @f
		zlib_assert 'Code too clever' ;Assert(..>=.. && ..==..)
	@@:
end if

	; Do not waste too much time if we already have a good match:
	mov eax,[edx+deflate_state.good_match]
	cmp [edx+deflate_state.prev_length],eax
	jl @f ;if (..>=..)
		shr dword[chain_length],2
	@@:
	; Do not look for matches beyond the end of the input. This is necessary
	; to make deflate deterministic.

	mov eax,[edx+deflate_state.lookahead]
	cmp dword[nice_match],eax
	jle @f ;if (..>..)
		mov [nice_match],eax
	@@:

	mov eax,[edx+deflate_state.window_size]
	sub eax,MIN_LOOKAHEAD
	cmp [edx+deflate_state.strstart],eax
	jle .cycle0
		zlib_assert 'need lookahead' ;Assert(..<=..)

align 4
	.cycle0: ;do
		mov eax,[edx+deflate_state.strstart]
		cmp [cur_match],eax
		jl @f
			zlib_assert 'no future' ;Assert(..<..)
		@@:
		mov esi,[edx+deflate_state.window]
		add esi,[cur_match]

		; Skip to next match if the match length cannot increase
		; or if the match length is less than 2.  Note that the checks below
		; for insufficient lookahead only occur occasionally for performance
		; reasons.  Therefore uninitialized memory will be accessed, and
		; conditional jumps will be made that depend on those values.
		; However the length of the match is limited to the lookahead, so
		; the output of deflate is not affected by the uninitialized values.

		mov eax,[best_len]
		dec eax
		cmp word[esi+eax],bx
		jne .cycle0cont
		mov al,byte[esi]
		cmp al,byte[edi]
		jne .cycle0cont
		inc esi
		mov al,byte[esi]
		cmp al,[edi+1]
		jne .cycle0cont ;if (..!=.. || ..!=.. || ..!=.. || ..!=..) continue

		; The check at best_len-1 can be removed because it will be made
		; again later. (This heuristic is not always a win.)
		; It is not necessary to compare scan[2] and match[2] since they
		; are always equal when the other bytes match, given that
		; the hash keys are equal and that HASH_BITS >= 8.

		add edi,2
		inc esi
		mov al,byte[edi]
		cmp al,byte[esi]
		je @f
			zlib_assert 'match[2]?' ;Assert(..==..)
		@@:

		; We check for insufficient lookahead only every 8th comparison;
		; the 256th check will be made at strstart+258.

		inc edi
		inc esi
		mov ecx,[strend]
		sub ecx,edi
		jz @f
			repe cmpsb
			dec edi
			dec esi
		@@:

		mov eax,[edx+deflate_state.window_size]
		dec eax
		add eax,[edx+deflate_state.window]
		cmp edi,eax
		jle @f
			zlib_assert 'wild scan' ;Assert(..<=..)
		@@:

		mov eax,MAX_MATCH
		add eax,edi
		sub eax,[strend]
		mov [len],eax
		mov edi,[strend]
		sub edi,MAX_MATCH

		mov eax,[best_len]
		cmp [len],eax
		jle .cycle0cont ;if (..>..)
			mov eax,[cur_match]
			mov [edx+deflate_state.match_start],eax
			mov eax,[len]
			mov [best_len],eax
			mov eax,[nice_match]
			cmp [len],eax
			jge .cycle0end ;if (..>=..) break
			mov eax,[best_len]
			dec eax
			mov bx,[edi+eax]

		.cycle0cont:
		mov eax,[cur_match]
		and eax,[wmask]
		shl eax,1
		add eax,[prev]
		movzx eax,word[eax] ;eax = prev[cur_match & wmask]
		mov [cur_match],eax
		cmp eax,[limit]
		jle .cycle0end
		dec dword[chain_length]
		jnz .cycle0
align 4
	.cycle0end: ;while (..>.. && ..!=0)

	mov eax,[edx+deflate_state.lookahead]
	cmp [best_len],eax
	jg @f ;if (..<=..)
		mov eax,[best_len]
	@@:	
;end if ;ASMV

else ;FASTEST

; ---------------------------------------------------------------------------
; Optimized version for FASTEST only
	mov edx,[s]

	; The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
	; It is easy to get rid of this optimization if necessary.

if MAX_MATCH <> 258
	cmp dword[edx+deflate_state.hash_bits],8
	jge @f
		zlib_assert 'Code too clever' ;Assert(..>=.. && ..==..)
	@@:
end if
	mov eax,[edx+deflate_state.window_size]
	sub eax,MIN_LOOKAHEAD
	cmp [edx+deflate_state.strstart],eax
	jle @f
		zlib_assert 'need lookahead' ;Assert(..<=..)
	@@:
	mov eax,[edx+deflate_state.strstart]
	cmp [cur_match],eax
	jl @f
		zlib_assert 'no future' ;Assert(..<..)
	@@:

	mov esi,[edx+deflate_state.window]
	mov edi,esi
	add esi,[cur_match]
	add edi,[edx+deflate_state.strstart]
	;edi = scan
	;esi = match

	; Return failure if the match length is less than 2:

	lodsw
	cmp ax,word[edi]
	je @f ;if (word[edi] != word[esi]) return 
		mov eax,MIN_MATCH-1
		jmp .end_f
	@@:

	; The check at best_len-1 can be removed because it will be made
	; again later. (This heuristic is not always a win.)
	; It is not necessary to compare scan[2] and match[2] since they
	; are always equal when the other bytes match, given that
	; the hash keys are equal and that HASH_BITS >= 8.

	add edi,2
	mov al,byte[edi]
	cmp al,byte[esi]
	je @f
		zlib_assert 'match[2]?' ;Assert(..==..)
	@@:

	; We check for insufficient lookahead only every 8th comparison;
	; the 256th check will be made at strstart+258.

	mov ebx,edi
	mov ecx,MAX_MATCH
align 4
	@@:
		lodsb
		scasb
		loope @b

	mov eax,[edx+deflate_state.window_size]
	dec eax
	add eax,[edx+deflate_state.window]
	cmp edi,eax
	jle @f
		zlib_assert 'wild scan' ;Assert(..<=..)
	@@:
	sub edi,ebx
	;edi = len

	cmp edi,MIN_MATCH
	jge @f ;if (..<..) 
		mov eax,MIN_MATCH-1
		jmp .end_f
	@@:
	mov eax,[cur_match]
	mov [edx+deflate_state.match_start],eax
	mov eax,[edx+deflate_state.lookahead]
	cmp edi,eax
	jg @f ;if (len <= s.lookahead) ? len : s.lookahead
		mov eax,edi
	@@:
end if ;FASTEST
.end_f:
	ret
endp

; ===========================================================================
; Check that the match at match_start is indeed a match.

;void (s, start, match, length)
;    deflate_state *s
;    IPos start, match
;    int length
align 4
proc check_match, s:dword, start:dword, p3match:dword, length:dword
if DEBUG eq 1
	; check that the match is indeed a match
;    if (zmemcmp(s->window + match,
;                s->window + start, length) != EQUAL) {
;        fprintf(stderr, " start %u, match %u, length %d\n",
;                start, match, length);
;        do {
;            fprintf(stderr, "%c%c", s->window[match++], s->window[start++]);
;        } while (--length != 0);
;        z_error("invalid match");
;    }
;    if (z_verbose > 1) {
;        fprintf(stderr,"\\[%d,%d]", start-match, length);
;        do { putc(s->window[start++], stderr); } while (--length != 0);
;    }
end if ;DEBUG
	ret
endp

; ===========================================================================
; Fill the window when the lookahead becomes insufficient.
; Updates strstart and lookahead.

; IN assertion: lookahead < MIN_LOOKAHEAD
; OUT assertions: strstart <= window_size-MIN_LOOKAHEAD
;    At least one byte has been read, or avail_in == 0; reads are
;    performed for at least two bytes (required for the zip translate_eol
;    option -- not supported here).

;void (deflate_state *s)
align 16
proc fill_window, s:dword
pushad
;esi = p, str, curr
;ebx = more ;Amount of free space at the end of the window.
	;Объем свободного пространства в конце окна.
;ecx = wsize ;uInt
;edx = s.strm
	mov edi,[s]
	cmp dword[edi+deflate_state.lookahead],MIN_LOOKAHEAD
	jl @f
		zlib_assert 'already enough lookahead' ;Assert(..<..)
	@@:

	mov ecx,[edi+deflate_state.w_size]
	mov edx,[edi+deflate_state.strm]
	.cycle0: ;do
		mov ebx,[edi+deflate_state.window_size]
		sub ebx,[edi+deflate_state.lookahead]
		sub ebx,[edi+deflate_state.strstart]

		; If the window is almost full and there is insufficient lookahead,
		; move the upper half to the lower one to make room in the upper half.

		;;MAX_DIST edi
		;;add eax,ecx
		mov eax,[edi+deflate_state.w_size]
		lea eax,[ecx+eax-MIN_LOOKAHEAD]
		cmp [edi+deflate_state.strstart],eax
		jb .end0 ;if (..>=..)
			push ecx
			mov eax,[edi+deflate_state.window]
			add eax,ecx
			stdcall zmemcpy, [edi+deflate_state.window], eax
			sub [edi+deflate_state.match_start],ecx
			sub [edi+deflate_state.strstart],ecx ;we now have strstart >= MAX_DIST
			sub [edi+deflate_state.block_start],ecx
			; Slide the hash table (could be avoided with 32 bit values
			; at the expense of memory usage). We slide even when level == 0
			; to keep the hash table consistent if we switch back to level > 0
			; later. (Using level 0 permanently is not an optimal usage of
			; zlib, so we don't care about this pathological case.)

			push ebx ecx
			;ebx = wsize
			;ecx = n
			mov ebx,ecx
			mov ecx,[edi+deflate_state.hash_size]
			mov esi,ecx
			shl esi,1
			add esi,[edi+deflate_state.head]
			.cycle1: ;do
				sub esi,2
				movzx eax,word[esi]
				mov word[esi],NIL
				cmp eax,ebx
				jl @f
					sub eax,ebx
					mov [esi],ax
				@@:
			loop .cycle1 ;while (..)
if FASTEST eq 0
			mov ecx,ebx
			mov esi,ecx
			shl esi,1
			add esi,[edi+deflate_state.prev]
			.cycle2: ;do
				sub esi,2
				movzx eax,word[esi]
				mov word[esi],NIL
				cmp eax,ebx
				jl @f
					sub eax,ebx
					mov [esi],ax
				@@:
				; If n is not on any hash chain, prev[n] is garbage but
				; its value will never be used.

			loop .cycle2 ;while (..)
end if
			pop ecx ebx
			add ebx,ecx
		.end0:
		cmp dword[edx+z_stream.avail_in],0
		je .cycle0end ;if (..==0) break

		; If there was no sliding:
		;    strstart <= WSIZE+MAX_DIST-1 && lookahead <= MIN_LOOKAHEAD - 1 &&
		;    more == window_size - lookahead - strstart
		; => more >= window_size - (MIN_LOOKAHEAD-1 + WSIZE + MAX_DIST-1)
		; => more >= window_size - 2*WSIZE + 2
		; In the BIG_MEM or MMAP case (not yet supported),
		;   window_size == input_size + MIN_LOOKAHEAD  &&
		;   strstart + s->lookahead <= input_size => more >= MIN_LOOKAHEAD.
		; Otherwise, window_size == 2*WSIZE so more >= 2.
		; If there was sliding, more >= WSIZE. So in all cases, more >= 2.

		cmp ebx,2
		jge @f
			zlib_assert 'more < 2' ;Assert(..>=..)
		@@:
		mov eax,[edi+deflate_state.window]
		add eax,[edi+deflate_state.strstart]
		add eax,[edi+deflate_state.lookahead]
		stdcall read_buf, edx, eax, ebx
		add [edi+deflate_state.lookahead],eax

		; Initialize the hash value now that we have some input:
		mov eax,[edi+deflate_state.lookahead]
		add eax,[edi+deflate_state.insert]
		cmp eax,MIN_MATCH
		jb .end1 ;if (..>=..)
			mov esi,[edi+deflate_state.strstart]
			sub esi,[edi+deflate_state.insert]
			;esi = str
			mov eax,[edi+deflate_state.window]
			add eax,esi
			mov [edi+deflate_state.ins_h],eax
			inc eax
			movzx eax,byte[eax]
            UPDATE_HASH edi, [edi+deflate_state.ins_h], eax
if MIN_MATCH <> 3
;            Call UPDATE_HASH() MIN_MATCH-3 more times
end if
			.cycle3: ;while (..)
			cmp dword[edi+deflate_state.insert],0
			je .end1
				mov eax,esi
				add eax,MIN_MATCH-1
				add eax,[edi+deflate_state.window]
				movzx eax,byte[eax]
				UPDATE_HASH edi, [edi+deflate_state.ins_h], eax
if FASTEST eq 0
				mov eax,[edi+deflate_state.ins_h]
				shl eax,1
				add eax,[edi+deflate_state.head]
				push ebx
				mov ebx,[edi+deflate_state.w_mask]
				and ebx,esi
				shl ebx,1
				add ebx,[edi+deflate_state.prev]
				mov ax,[eax]
				mov [ebx],ax
				pop ebx
end if
				mov eax,[edi+deflate_state.ins_h]
				shl eax,1
				add eax,[edi+deflate_state.head]
				mov [eax],si
				inc esi
				dec dword[edi+deflate_state.insert]
				mov eax,[edi+deflate_state.lookahead]
				add eax,[edi+deflate_state.insert]
				cmp eax,MIN_MATCH
				jb .end1 ;if (..<..) break
			jmp .cycle3
		.end1:
		; If the whole input has less than MIN_MATCH bytes, ins_h is garbage,
		; but this is not important since only literal bytes will be emitted.

		cmp dword[edi+deflate_state.lookahead],MIN_LOOKAHEAD
		jae .cycle0end
		cmp dword[edx+z_stream.avail_in],0
		jne .cycle0
align 4
	.cycle0end: ;while (..<.. && ..!=..)

	; If the WIN_INIT bytes after the end of the current data have never been
	; written, then zero those bytes in order to avoid memory check reports of
	; the use of uninitialized (or uninitialised as Julian writes) bytes by
	; the longest match routines.  Update the high water mark for the next
	; time through here.  WIN_INIT is set to MAX_MATCH since the longest match
	; routines allow scanning to strstart + MAX_MATCH, ignoring lookahead.

	mov eax,[edi+deflate_state.window_size]
	cmp [edi+deflate_state.high_water],eax
	jae .end2 ;if (..<..)
		mov esi,[edi+deflate_state.lookahead]
		add esi,[edi+deflate_state.strstart]
		;esi = curr

		cmp [edi+deflate_state.high_water],esi
		jae .end3 ;if (..<..)
			; Previous high water mark below current data -- zero WIN_INIT
			; bytes or up to end of window, whichever is less.

			mov eax,[edi+deflate_state.window_size]
			sub eax,esi
			cmp eax,WIN_INIT
			jbe @f ;if (..>..)
				mov eax,WIN_INIT
			@@:
			mov edx,[edi+deflate_state.window]
			add edx,esi
			stdcall zmemzero, edx, eax
			add eax,esi
			mov [edi+deflate_state.high_water],eax
			jmp .end2
		.end3: ;else if (..<..)
		mov eax,esi
		add eax,WIN_INIT
		cmp [edi+deflate_state.high_water],eax
		jae .end2
			; High water mark at or above current data, but below current data
			; plus WIN_INIT -- zero out to current data plus WIN_INIT, or up
			; to end of window, whichever is less.

			;eax = esi+WIN_INIT
			sub eax,[edi+deflate_state.high_water]
			mov edx,[edi+deflate_state.window_size]
			sub edx,[edi+deflate_state.high_water]
			cmp eax,edx ;if (..>..)
			jbe @f
				mov eax,edx
			@@:
			mov edx,[edi+deflate_state.window]
			add edx,[edi+deflate_state.high_water]
			stdcall zmemzero, edx, eax
			add [edi+deflate_state.high_water],eax
	.end2:

	mov eax,[edi+deflate_state.window_size]
	sub eax,MIN_LOOKAHEAD
	cmp [edi+deflate_state.strstart],eax
	jle @f
		zlib_assert 'not enough room for search' ;Assert(..<=..)
	@@:
popad
	ret
endp

; ===========================================================================
; Flush the current block, with given end-of-file flag.
; IN assertion: strstart is set to the end of the current match.

macro FLUSH_BLOCK_ONLY s, last
{
local .end0
	push dword last
	mov eax,[s+deflate_state.strstart]
	sub eax,[s+deflate_state.block_start]
	push eax
	xor eax,eax
	cmp [s+deflate_state.block_start],eax
	jl .end0
		mov eax,[s+deflate_state.block_start]
		add eax,[s+deflate_state.window]
	.end0:
	stdcall _tr_flush_block, s, eax
	mov eax,[s+deflate_state.strstart]
	mov [s+deflate_state.block_start],eax
	stdcall flush_pending, [s+deflate_state.strm]
;   Tracev((stderr,"[FLUSH]"));
}

; Same but force premature exit if necessary.
macro FLUSH_BLOCK s, last
{
local .end0
	FLUSH_BLOCK_ONLY s, last
	mov eax,[s+deflate_state.strm]
	cmp dword[eax+z_stream.avail_out],0
	jne .end0 ;if (..==0)
if last eq 1
		mov eax,finish_started
else
		mov eax,need_more
end if
		jmp .end_f
	.end0:
}

; ===========================================================================
; Copy without compression as much as possible from the input stream, return
; the current block state.
; This function does not insert new strings in the dictionary since
; uncompressible data is probably not useful. This function is used
; only for the level=0 compression option.
; NOTE: this function should be optimized to avoid extra copying from
; window to pending_buf.

;block_state (deflate_state *s, int flush)
align 4
proc deflate_stored uses ebx ecx edi, s:dword, flush:dword
; Stored blocks are limited to 0xffff bytes, pending_buf is limited
; to pending_buf_size, and each stored block has a 5 byte header:
	mov edi,[s]

	mov ecx,0xffff
	mov eax,[edi+deflate_state.pending_buf_size]
	sub eax,5
	cmp ecx,eax
	jle .cycle0 ;if (..>..)
		mov ecx,eax
	;ecx = max_block_size

	; Copy as much as possible from input to output:
align 4
	.cycle0: ;for (;;)
		; Fill the window as much as possible:
		cmp dword[edi+deflate_state.lookahead],1
		jg .end0 ;if (..<=..)
;            Assert(s->strstart < s->w_size+MAX_DIST(s) ||
;                   s->block_start >= (long)s->w_size, "slide too late");

			stdcall fill_window, edi
			cmp dword[edi+deflate_state.lookahead],0
			jne @f
			cmp dword[flush],Z_NO_FLUSH
			jne @f ;if (..==0 && ..==..)
				mov eax,need_more
				jmp .end_f
			@@:
			cmp dword[edi+deflate_state.lookahead],0
			je .cycle0end ;if (..==0) break ;flush the current block
		.end0:
		cmp dword[edi+deflate_state.block_start],0
		jge @f
			zlib_assert 'block gone' ;Assert(..>=0)
		@@:

		mov eax,[edi+deflate_state.lookahead]
		add [edi+deflate_state.strstart],eax
		mov dword[edi+deflate_state.lookahead],0

		; Emit a stored block if pending_buf will be full:
		mov ebx,[edi+deflate_state.block_start]
		add ebx,ecx
		cmp dword[edi+deflate_state.strstart],0
		je @f
		cmp [edi+deflate_state.strstart],ebx
		jl .end1
		@@: ;if (..==0 || ..>=..)
			; strstart == 0 is possible when wraparound on 16-bit machine
			mov eax,[edi+deflate_state.strstart]
			sub eax,ebx
			mov [edi+deflate_state.lookahead],eax
			mov [edi+deflate_state.strstart],ebx
			FLUSH_BLOCK edi, 0
		.end1:
		; Flush if we may have to slide, otherwise block_start may become
		; negative and the data will be gone:

		MAX_DIST edi
		mov ebx,[edi+deflate_state.strstart]
		sub ebx,[edi+deflate_state.block_start]
		cmp ebx,eax
		jl .cycle0 ;if (..>=..)
			FLUSH_BLOCK edi, 0
		jmp .cycle0
align 4
	.cycle0end:
	mov dword[edi+deflate_state.insert],0
	cmp dword[flush],Z_FINISH
	jne @f ;if (..==..)
		FLUSH_BLOCK edi, 1
		mov eax,finish_done
		jmp .end_f
	@@:
	mov eax,[edi+deflate_state.block_start]
	cmp [edi+deflate_state.strstart],eax
	jle @f ;if (..>..)
		FLUSH_BLOCK edi, 0
	@@:
	mov eax,block_done
.end_f:
	ret
endp

; ===========================================================================
; Compress as much as possible from the input stream, return the current
; block state.
; This function does not perform lazy evaluation of matches and inserts
; new strings in the dictionary only for unmatched strings or for short
; matches. It is used only for the fast compression options.

;block_state (s, flush)
;    deflate_state *s
;    int flush
align 4
proc deflate_fast uses ebx ecx edi, s:dword, flush:dword
locals
	bflush dd ? ;int  ;set if current block must be flushed
endl
;ecx = hash_head ;IPos ;head of the hash chain
	mov edi,[s]

	.cycle0: ;for (..)
	; Make sure that we always have enough lookahead, except
	; at the end of the input file. We need MAX_MATCH bytes
	; for the next match, plus MIN_MATCH bytes to insert the
	; string following the next match.

		cmp dword[edi+deflate_state.lookahead],MIN_LOOKAHEAD
		jge .end0 ;if (..<..)
			stdcall fill_window, edi
			cmp dword[edi+deflate_state.lookahead],MIN_LOOKAHEAD
			jge @f ;if (..<.. && ..==..)
			cmp dword[flush],Z_NO_FLUSH
			jne @f
				mov eax,need_more
				jmp .end_f
align 4
			@@:
			cmp dword[edi+deflate_state.lookahead],0
			je .cycle0end ;if (..==0) break ;flush the current block
align 4
		.end0:

		; Insert the string window[strstart .. strstart+2] in the
		; dictionary, and set hash_head to the head of the hash chain:

		mov ecx,NIL
		cmp dword[edi+deflate_state.lookahead],MIN_MATCH
		jl @f ;if (..>=..)
			INSERT_STRING edi, [edi+deflate_state.strstart], ecx
		@@:

		; Find the longest match, discarding those <= prev_length.
		; At this point we have always match_length < MIN_MATCH

		cmp ecx,NIL
		je @f
		MAX_DIST edi
		mov ebx,[edi+deflate_state.strstart]
		sub ebx,ecx
		cmp ebx,eax
		jg @f ;if (..!=0 && ..<=..)
			; To simplify the code, we prevent matches with the string
			; of window index 0 (in particular we have to avoid a match
			; of the string with itself at the start of the input file).

			stdcall longest_match, edi, ecx
			mov [edi+deflate_state.match_length],eax
			; longest_match() sets match_start
		@@:
		cmp dword[edi+deflate_state.match_length],MIN_MATCH
		jl .end1 ;if (..>=..)
			stdcall check_match, edi, [edi+deflate_state.strstart], [edi+deflate_state.match_start], [edi+deflate_state.match_length]

			mov eax,[edi+deflate_state.strstart]
			sub eax,[edi+deflate_state.match_start]
			mov ebx,[edi+deflate_state.match_length]
			sub ebx,MIN_MATCH
			_tr_tally_dist edi, eax, ebx, [bflush]

			mov eax,[edi+deflate_state.match_length]
			sub [edi+deflate_state.lookahead],eax

			; Insert new strings in the hash table only if the match length
			; is not too large. This saves time but degrades compression.

if FASTEST eq 0
			;;mov eax,[edi+deflate_state.match_length]
			cmp eax,[edi+deflate_state.max_insert_length]
			jg .end3
			cmp dword[edi+deflate_state.lookahead],MIN_MATCH
			jl .end3 ;if (..<=.. && ..>=..)
				dec dword[edi+deflate_state.match_length] ;string at strstart already in table
				.cycle1: ;do {
					inc dword[edi+deflate_state.strstart]
					INSERT_STRING edi, [edi+deflate_state.strstart], ecx
					; strstart never exceeds WSIZE-MAX_MATCH, so there are
					; always MIN_MATCH bytes ahead.

					dec dword[edi+deflate_state.match_length]
					cmp dword[edi+deflate_state.match_length],0
					jne .cycle1 ;while (..!=0)
				inc dword[edi+deflate_state.strstart]
				jmp .end2
			.end3: ;else
end if

				mov eax,[edi+deflate_state.match_length]
				add [edi+deflate_state.strstart],eax
				mov dword[edi+deflate_state.match_length],0
				mov eax,[edi+deflate_state.window]
				add eax,[edi+deflate_state.strstart]
				mov [edi+deflate_state.ins_h],eax
				inc eax
				movzx eax,byte[eax]
				UPDATE_HASH edi, [edi+deflate_state.ins_h], eax
if MIN_MATCH <> 3
;                Call UPDATE_HASH() MIN_MATCH-3 more times
end if
				; If lookahead < MIN_MATCH, ins_h is garbage, but it does not
				; matter since it will be recomputed at next deflate call.
			jmp .end2
		.end1: ;else
			; No match, output a literal byte
			mov eax,[edi+deflate_state.window]
			add eax,[edi+deflate_state.strstart]
			movzx eax,byte[eax]
			Tracevv eax,
			_tr_tally_lit edi, eax, [bflush]
			dec dword[edi+deflate_state.lookahead]
			inc dword[edi+deflate_state.strstart]
		.end2:
		cmp dword[bflush],0
		je .cycle0 ;if (..)
			FLUSH_BLOCK edi, 0
		jmp .cycle0
align 4
	.cycle0end:
	mov eax,[edi+deflate_state.strstart]
	cmp eax,MIN_MATCH-1
	jl @f
		mov eax,MIN_MATCH-1
	@@:
	mov [edi+deflate_state.insert],eax
	cmp dword[flush],Z_FINISH
	jne @f ;if (..==..)
		FLUSH_BLOCK edi, 1
		mov eax,finish_done
		jmp .end_f
	@@:
	cmp dword[edi+deflate_state.last_lit],0
	je @f ;if (..)
		FLUSH_BLOCK edi, 0
	@@:
	mov eax,block_done
.end_f:
	ret
endp

; ===========================================================================
; Same as above, but achieves better compression. We use a lazy
; evaluation for matches: a match is finally adopted only if there is
; no better match at the next window position.

;block_state (s, flush)
;    deflate_state *s
;    int flush
align 4
proc deflate_slow uses ebx ecx edx edi, s:dword, flush:dword
locals
	bflush dd ? ;int  ;set if current block must be flushed
endl
;ecx = hash_head ;IPos ;head of the hash chain
	mov edi,[s]

	; Process the input block.
	.cycle0: ;for (;;)
	; Make sure that we always have enough lookahead, except
	; at the end of the input file. We need MAX_MATCH bytes
	; for the next match, plus MIN_MATCH bytes to insert the
	; string following the next match.

		cmp dword[edi+deflate_state.lookahead],MIN_LOOKAHEAD
		jae .end0 ;if (..<..)
			stdcall fill_window, edi
			cmp dword[edi+deflate_state.lookahead],MIN_LOOKAHEAD
			jae @f ;if (..<.. && ..==..)
			cmp dword[flush],Z_NO_FLUSH
			jne @f
				mov eax,need_more
				jmp .end_f
align 4
			@@:
			cmp dword[edi+deflate_state.lookahead],0
			je .cycle0end ;if (..==0) break ;flush the current block
align 4
		.end0:

		; Insert the string window[strstart .. strstart+2] in the
		; dictionary, and set hash_head to the head of the hash chain:

		mov ecx,NIL
		cmp dword[edi+deflate_state.lookahead],MIN_MATCH
		jb @f ;if (..>=..)
			INSERT_STRING edi, [edi+deflate_state.strstart], ecx
		@@:

		; Find the longest match, discarding those <= prev_length.

		mov eax,[edi+deflate_state.match_length]
		mov [edi+deflate_state.prev_length],eax
		mov eax,[edi+deflate_state.match_start]
		mov [edi+deflate_state.prev_match],eax
		mov dword[edi+deflate_state.match_length],MIN_MATCH-1

		cmp ecx,NIL
		je .end1
		mov eax,[edi+deflate_state.prev_length]
		cmp eax,[edi+deflate_state.max_lazy_match]
		jae .end1
		MAX_DIST edi
		mov ebx,[edi+deflate_state.strstart]
		sub ebx,ecx
		cmp ebx,eax
		ja .end1 ;if (..!=0 && ..<.. && ..<=..)
			; To simplify the code, we prevent matches with the string
			; of window index 0 (in particular we have to avoid a match
			; of the string with itself at the start of the input file).

			stdcall longest_match, edi, ecx
			mov [edi+deflate_state.match_length],eax
			; longest_match() sets match_start

			cmp dword[edi+deflate_state.match_length],5
			ja .end1
			cmp word[edi+deflate_state.strategy],Z_FILTERED
if TOO_FAR <= 32767
			je @f
				cmp dword[edi+deflate_state.match_length],MIN_MATCH
				jne .end1
				mov eax,[edi+deflate_state.strstart]
				sub eax,[edi+deflate_state.match_start]
				cmp eax,TOO_FAR
				jbe .end1 ;if (..<=.. && (..==.. || (..==.. && ..>..)))
			@@:
else
			jne .end1 ;if (..<=.. && ..==..)
end if
				; If prev_match is also MIN_MATCH, match_start is garbage
				; but we will ignore the current match anyway.

				mov dword[edi+deflate_state.match_length],MIN_MATCH-1
		.end1:
		; If there was a match at the previous step and the current
		; match is not better, output the previous match:


		mov eax,[edi+deflate_state.prev_length]
		cmp eax,MIN_MATCH
		jb .end2
		cmp [edi+deflate_state.match_length],eax
		ja .end2 ;if (..>=.. && ..<=..)
			mov edx,[edi+deflate_state.strstart]
			add edx,[edi+deflate_state.lookahead]
			sub edx,MIN_MATCH
			;edx = max_insert
			; Do not insert strings in hash table beyond this.

			mov eax,[edi+deflate_state.strstart]
			dec eax
			stdcall check_match, edi, eax, [edi+deflate_state.prev_match], [edi+deflate_state.prev_length]

			mov eax,[edi+deflate_state.strstart]
			dec eax
			sub eax,[edi+deflate_state.prev_match]
			mov ebx,[edi+deflate_state.prev_length]
			sub ebx,MIN_MATCH
			_tr_tally_dist edi, eax, ebx, [bflush]

			; Insert in hash table all strings up to the end of the match.
			; strstart-1 and strstart are already inserted. If there is not
			; enough lookahead, the last two strings are not inserted in
			; the hash table.

			mov eax,[edi+deflate_state.prev_length]
			dec eax
			sub [edi+deflate_state.lookahead],eax
			sub dword[edi+deflate_state.prev_length],2
			.cycle1: ;do
				inc dword[edi+deflate_state.strstart]
				cmp [edi+deflate_state.strstart],edx
				ja @f ;if (..<=..)
					INSERT_STRING edi, [edi+deflate_state.strstart], ecx
				@@:
				dec dword[edi+deflate_state.prev_length]
				cmp dword[edi+deflate_state.prev_length],0
				jne .cycle1 ;while (..!=0)
			mov dword[edi+deflate_state.match_available],0
			mov dword[edi+deflate_state.match_length],MIN_MATCH-1
			inc dword[edi+deflate_state.strstart]

			cmp dword[bflush],0
			je .cycle0 ;if (..)
				FLUSH_BLOCK edi, 0
			jmp .cycle0
align 4
		.end2: ;else if (..)
		cmp dword[edi+deflate_state.match_available],0
		je .end3
			; If there was no match at the previous position, output a
			; single literal. If there was a match but the current match
			; is longer, truncate the previous match to a single literal.

			mov eax,[edi+deflate_state.strstart]
			dec eax
			add eax,[edi+deflate_state.window]
			movzx eax,byte[eax]
			Tracevv eax,
			_tr_tally_lit edi, eax, [bflush]
			cmp dword[bflush],0
			je @f ;if (..)
				FLUSH_BLOCK_ONLY edi, 0
			@@:
			inc dword[edi+deflate_state.strstart]
			dec dword[edi+deflate_state.lookahead]
			mov eax,[edi+deflate_state.strm]
			cmp dword[eax+z_stream.avail_out],0
			jne .cycle0 ;if (..==0) return ..
				mov eax,need_more
				jmp .end_f
align 4
		.end3: ;else
			; There is no previous match to compare with, wait for
			; the next step to decide.

			mov dword[edi+deflate_state.match_available],1
			inc dword[edi+deflate_state.strstart]
			dec dword[edi+deflate_state.lookahead]
		jmp .cycle0
align 4
	.cycle0end:
	cmp dword[flush],Z_NO_FLUSH
	jne @f
		zlib_assert 'no flush?' ;Assert (..!=..)
	@@:
	cmp dword[edi+deflate_state.match_available],0
	je @f ;if (..)
		mov eax,[edi+deflate_state.strstart]
		dec eax
		add eax,[edi+deflate_state.window]
		movzx eax,byte[eax]
		Tracevv eax,
		_tr_tally_lit edi, eax, [bflush]
		mov dword[edi+deflate_state.match_available],0
	@@:
	mov eax,[edi+deflate_state.strstart]
	cmp eax,MIN_MATCH-1
	jb @f
		mov eax,MIN_MATCH-1
	@@:
	mov [edi+deflate_state.insert],eax
	cmp dword[flush],Z_FINISH
	jne @f ;if (..==..)
		FLUSH_BLOCK edi, 1
		mov eax,finish_done
		jmp .end_f
	@@:
	cmp dword[edi+deflate_state.last_lit],0
	je @f ;if (..)
		FLUSH_BLOCK edi, 0
	@@:
	mov eax,block_done
.end_f:
	ret
endp

; ===========================================================================
; For Z_RLE, simply look for runs of bytes, generate matches only of distance
; one.  Do not maintain a hash table.  (It will be regenerated if this run of
; deflate switches away from Z_RLE.)

;block_state (s, flush)
;    deflate_state *s
;    int flush
align 4
proc deflate_rle uses ecx edx edi esi, s:dword, flush:dword
locals
	bflush dd ? ;int ;set if current block must be flushed
endl
	mov edx,[s]
align 4
	.cycle0: ;for (;;)
		; Make sure that we always have enough lookahead, except
		; at the end of the input file. We need MAX_MATCH bytes
		; for the longest run, plus one for the unrolled loop.
		cmp dword[edx+deflate_state.lookahead],MAX_MATCH
		jg .end0 ;if (..<=..)
			stdcall fill_window, edx
			cmp dword[edx+deflate_state.lookahead],MAX_MATCH
			jg @f
			cmp dword[flush],Z_NO_FLUSH
			jne @f ;if (..<=.. && ..==..)
				mov eax,need_more
				jmp .end_f
align 4
			@@:
			cmp dword[edx+deflate_state.lookahead],0
			je .cycle0end ;flush the current block
align 4
		.end0:

		; See how many times the previous byte repeats
		mov dword[edx+deflate_state.match_length],0
		cmp dword[edx+deflate_state.lookahead],MIN_MATCH
		jl .end1
		cmp dword[edx+deflate_state.strstart],0
		jle .end1 ;if (..>=.. && ..>..)
			mov esi,[edx+deflate_state.window]
			add esi,[edx+deflate_state.strstart]
			dec esi
			lodsb ;prev = *scan; ++scan
			mov edi,esi
			scasb
			jnz .end2
			scasb
			jnz .end2
			scasb
			jnz .end2 ;if (..==.. && ..==.. && ..==..)
				;edi = scan ;scan goes up to strend for length of run
				; al = prev ;byte at distance one to match
				;ecx = strend-scan
				mov ecx,MAX_MATCH-2
				repz scasb
				dec edi
				sub edi,[edx+deflate_state.window]
				sub edi,[edx+deflate_state.strstart]
				mov [edx+deflate_state.match_length],edi
				mov eax,[edx+deflate_state.lookahead]
				cmp [edx+deflate_state.match_length],eax
				jle .end2 ;if (..>..)
					mov [edx+deflate_state.match_length],eax
			.end2:
			mov eax,[edx+deflate_state.window_size]
			dec eax
			add eax,[edx+deflate_state.window]
			cmp edi,eax
			jle .end1
				zlib_assert 'wild scan' ;Assert(..<=..)
		.end1:

		; Emit match if have run of MIN_MATCH or longer, else emit literal
		cmp dword[edx+deflate_state.match_length],MIN_MATCH
		jl @f ;if (..>=..)
			push dword[edx+deflate_state.match_length]
			mov eax,[edx+deflate_state.strstart]
			dec eax
			stdcall check_match, edx, [edx+deflate_state.strstart], eax

			mov eax,[edx+deflate_state.match_length]
			sub eax,MIN_MATCH
			_tr_tally_dist edx, 1, eax, [bflush]

			mov eax,[edx+deflate_state.match_length]
			sub [edx+deflate_state.lookahead],eax
			add [edx+deflate_state.strstart],eax
			mov dword[edx+deflate_state.match_length],0
			jmp .end3
		@@: ;else
			; No match, output a literal byte
			mov eax,[edx+deflate_state.strstart]
			add eax,[edx+deflate_state.window]
			movzx eax,byte[eax]
			Tracevv eax,
			_tr_tally_lit edx, eax, [bflush]
			dec dword[edx+deflate_state.lookahead]
			inc dword[edx+deflate_state.strstart]
		.end3:
		cmp dword[bflush],0
		je .cycle0 ;if (..)
			FLUSH_BLOCK edx, 0
		jmp .cycle0
align 4
	.cycle0end:
	mov dword[edx+deflate_state.insert],0
	cmp dword[flush],Z_FINISH
	jne @f ;if (..==..)
		FLUSH_BLOCK edx, 1
		mov eax,finish_done
		jmp .end_f
	@@:
	cmp dword[edx+deflate_state.last_lit],0
	je @f ;if (..)
		FLUSH_BLOCK edx, 0
	@@:
	mov eax,block_done
.end_f:
	ret
endp

; ===========================================================================
; For Z_HUFFMAN_ONLY, do not look for matches.  Do not maintain a hash table.
; (It will be regenerated if this run of deflate switches away from Huffman.)

;block_state (s, flush)
;    deflate_state *s
;    int flush
align 4
proc deflate_huff uses ebx edi, s:dword, flush:dword
locals
	bflush dd ? ;int ;set if current block must be flushed
endl
	mov edi,[s]
align 4
	.cycle0: ;for (;;)
		; Make sure that we have a literal to write.
		cmp dword[edi+deflate_state.lookahead],0
		jne .end0 ;if (..==0)
			stdcall fill_window, edi
			cmp dword[edi+deflate_state.lookahead],0
			jne .end0 ;if (..==0)
				cmp dword[flush],Z_NO_FLUSH
				jne .cycle0end ;if (..==..)
					mov eax,need_more
					jmp .end_f
				;flush the current block
align 4
		.end0:

		; Output a literal byte
		mov dword[edi+deflate_state.match_length],0
		mov eax,[edi+deflate_state.strstart]
		add eax,[edi+deflate_state.window]
		movzx eax,byte[eax]
		Tracevv eax,
		_tr_tally_lit edi, eax, [bflush]
		dec dword[edi+deflate_state.lookahead]
		inc dword[edi+deflate_state.strstart]
		cmp dword[bflush],0
		je .cycle0 ;if (..)
			FLUSH_BLOCK edi, 0
		jmp .cycle0
align 4
	.cycle0end:
	mov dword[edi+deflate_state.insert],0
	cmp dword[flush],Z_FINISH
	jne @f ;if (..==..)
		FLUSH_BLOCK edi, 1
		mov eax,finish_done
		jmp .end_f
	@@:
	cmp dword[edi+deflate_state.last_lit],0
	je @f ;if (..)
		FLUSH_BLOCK edi, 0
	@@:
	mov eax,block_done
.end_f:
	ret
endp
