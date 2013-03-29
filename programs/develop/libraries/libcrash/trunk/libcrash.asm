;    libcrash -- cryptographic hash functions
;
;    Copyright (C) 2012-2013 Ivan Baravy (dunkaist)
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.

format MS COFF

public @EXPORT as 'EXPORTS'

include '../../../../struct.inc'
include '../../../../proc32.inc'
include '../../../../macros.inc'
include '../../../../config.inc'
;include '../../../../debug.inc'

purge section,mov,add,sub
section '.flat' code readable align 16


include 'libcrash.inc'
include 'md4.asm'
include 'md5.asm'
include 'sha1.asm'
include 'sha224_256.asm'
include 'sha384_512.asm'


proc lib_init
	ret
endp


proc crash.hash  _hid, _hash, _data, _len, _callback, _msglen
locals
	hash_func	rd	1
	final		rd	1
	hi		rd	1
endl
	mov	eax, [_hid]
	imul	eax, sizeof.crash_item
	lea	eax, [crash._.table + eax]
	mov	[hi], eax

	mov	eax, [hi]
	mov	edx, [eax + crash_item.function]
	mov	esi, [eax + crash_item.init_val]
	mov	edi, [_hash]
	mov	[hash_func], edx
	mov	ecx, [hi]
	mov	ecx, [ecx + crash_item.len_in]
	rep	movsd

	mov	[final], 0
  .first:
	mov	eax, [_msglen]
	mov	ecx, [_len]
	add	[eax], ecx
	mov	esi, [_data]
	test	ecx, ecx
	jz	.callback
  .begin:
	mov	eax, [hi]
	mov	eax, [eax + crash_item.len_blk]
	sub	[_len], eax
	jnc	@f
	add	[_len], eax
	jmp	.endofblock
    @@:
	stdcall	[hash_func], [_hash], [_data]
	jmp	.begin
  .endofblock:
	cmp	[final], 1
	je	.quit

  .callback:
	call	[_callback]
	test	eax, eax
	jz	@f
	mov	[_len], eax
	jmp	.first
    @@:

	mov	edi, [_data]
	mov	ecx, [_len]
	rep	movsb
	mov	eax, [_msglen]
	mov	eax, [eax]
	mov	edx, [hi]
	mov	edx, [edx + crash_item.len_blk]
	sub	edx, 1
	and	eax, edx
	mov	edx, [hi]
	mov	ecx, [edx + crash_item.len_blk]
	sub	ecx, [edx + crash_item.len_size]
	sub	ecx, eax
	ja	@f
	add	ecx, [edx + crash_item.len_blk]
    @@:
	add	[_len], ecx
	mov	eax, [hi]
	mov	byte[edi], 0x80
	add	edi, 1
	sub	ecx, 1
	mov	al, 0
	rep	stosb
	push	ecx
	xor	eax, eax
	mov	ecx, [hi]
	mov	ecx, [ecx + crash_item.len_size]
	sub	ecx, 8			; FIXME for > 2^64 input length
	shr	ecx, 2
	rep	stosd
	pop	ecx
	mov	eax, [_msglen]
	mov	eax, [eax]
	mov	edx, 8
	mul	edx
	mov	ecx, [hi]
	cmp	[ecx + crash_item.endianness], LIBCRASH_ENDIAN_BIG
	jne	@f
	bswap	eax
	bswap	edx
	xchg	eax, edx
    @@:
	mov	dword[edi], eax
	mov	dword[edi + 4], edx
	mov	ecx, [hi]
	mov	eax, [ecx + crash_item.len_size]
	add	[_len], eax
	mov	[final], 1
	jmp	.first
  .quit:
	mov	eax, [hi]
	stdcall [eax + crash_item.postproc], [eax + crash_item.len_out], [_hash]
	ret
endp


proc crash._.md4_md5_postprocess _len_out, _hash
	ret
endp


proc crash._.sha1_224_256_postprocess _len_out, _hash
	mov	ecx, [_len_out]
	mov	esi, [_hash]
	mov	edi, esi
    @@:
	lodsd
	bswap	eax
	stosd
	dec	ecx
	jnz	@b
	ret
endp


proc crash._.sha384_512_postprocess _len_out, _hash
	mov	ecx, [_len_out]
	mov	esi, [_hash]
	mov	edi, esi
    @@:
	lodsd
	mov	ebx, eax
	lodsd
	bswap	eax
	bswap	ebx
	stosd
	mov	eax, ebx
	stosd
	dec	ecx
	jnz	@b
	emms
	ret
endp


proc crash.bin2hex _bin, _hex, _hid	; FIXME _hid param?
	mov	eax, [_hid]
	imul	eax, sizeof.crash_item
	mov	ecx, [crash._.table + eax + crash_item.len_out]
	mov	ebx, crash._.bin2hex_table
	mov	esi, [_bin]
	mov	edi, [_hex]
	shl	ecx, 2
  .next_byte:
	xor	eax, eax
	lodsb
	shl	eax, 4
	shr	al, 4
	xlatb
	xchg	al, ah
	xlatb
	stosw
	dec	ecx
	jnz	.next_byte
	xor	al, al
	stosb
	ret
endp


crash._.bin2hex_table	db	'0123456789abcdef'

; see crash_item struct for details
crash._.table		dd	crash.md4,	crash._.md4_init,    crash._.md4_md5_postprocess,       4,  4,  64,  8, 0
			dd	crash.md5,	crash._.md5_init,    crash._.md4_md5_postprocess,       4,  4,  64,  8, 0
			dd	crash.sha1,	crash._.sha1_init,   crash._.sha1_224_256_postprocess,  5,  5,  64,  8, 1
			dd	crash.sha256,	crash._.sha224_init, crash._.sha1_224_256_postprocess,  8,  7,  64,  8, 1
			dd	crash.sha256,	crash._.sha256_init, crash._.sha1_224_256_postprocess,  8,  8,  64,  8, 1
			dd	crash.sha512,	crash._.sha384_init, crash._.sha384_512_postprocess, 16, 12, 128, 16, 1
			dd	crash.sha512,	crash._.sha512_init, crash._.sha384_512_postprocess, 16, 16, 128, 16, 1

crash._.crc_init	dd	0xffffffff

crash._.md4_init:
crash._.md5_init:
crash._.sha1_init	dd	0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0

crash._.sha224_init	dd	0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939, 0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4

crash._.sha256_init	dd	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19

crash._.sha384_init	dq	0xcbbb9d5dc1059ed8, 0x629a292a367cd507, 0x9159015a3070dd17, 0x152fecd8f70e5939,\
				0x67332667ffc00b31, 0x8eb44a8768581511, 0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4

crash._.sha512_init	dq	0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,\
				0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179

crash._.sha256_table	dd	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,\
				0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,\
				0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,\
				0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,\
				0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,\
				0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,\
				0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,\
				0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2

crash._.sha512_table	dq	0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,\
				0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,\
				0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,\
				0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,\
				0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,\
				0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,\
				0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,\
				0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,\
				0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,\
				0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,\
				0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,\
				0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,\
				0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,\
				0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,\
				0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,\
				0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,\
				0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,\
				0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,\
				0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,\
				0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817


align 4
@EXPORT:

export                                   \
    lib_init        , 'lib_init'       , \
    crash.hash      , 'crash_hash'     , \
    crash.bin2hex   , 'crash_bin2hex'

section '.data' data readable writable align 16
crash._.crc32_table	rd	256
