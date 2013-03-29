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

macro chn x, y, z
{
	mov	eax, [y]
	xor	eax, [z]
	and	eax, [x]
	xor	eax, [z]
}

macro maj x, y, z
{
	mov	eax, [x]
	xor	eax, [y]
	and	eax, [z]
	mov	ecx, [x]
	and	ecx, [y]
	xor	eax, ecx
}

macro Sigma0 x
{
	mov	eax, x
	mov	ecx, eax
	ror	ecx, 2
	ror	eax, 13
	xor	eax, ecx
	mov	ecx, x
	ror	ecx, 22
	xor	eax, ecx
}

macro Sigma1 x
{
	mov	eax, x
	mov	ecx, eax
	ror	ecx, 6
	ror	eax, 11
	xor	eax, ecx
	mov	ecx, x
	ror	ecx, 25
	xor	eax, ecx
}

macro sigma0 x
{
	mov	eax, x
	mov	ecx, eax
	ror	ecx, 7
	ror	eax, 18
	xor	eax, ecx
	mov	ecx, x
	shr	ecx, 3
	xor	eax, ecx
}

macro sigma1 x
{
	mov	eax, x
	mov	ecx, eax
	ror	ecx, 17
	ror	eax, 19
	xor	eax, ecx
	mov	ecx, x
	shr	ecx, 10
	xor	eax, ecx
}

macro recalculate_w n
{
	mov	edx, [w + ((n-2) and 15)*4]
	sigma1	edx
	add	eax, [w + ((n-7) and 15)*4]
	push	eax
	mov	edx, [w + ((n-15) and 15)*4]
	sigma0	edx
	pop	ecx
	add	eax, ecx
	add	[w + (n)*4], eax
}

macro crash.sha256.round a, b, c, d, e, f, g, h, k
{
	mov	ebx, [h]
	mov	edx, [e]
	Sigma1	edx

	add	ebx, eax
	chn	e, f, g

	add	ebx, eax
	add	ebx, [k]
	add	ebx, edi

	add	[d], ebx

	mov	edx, [a]
	Sigma0	edx
	add	ebx, eax
	maj	a, b, c
	add	eax, ebx
	mov	[h], eax
}


macro crash.sha256.round_1_16 a, b, c, d, e, f, g, h, n
{

	mov	eax, [esi + (n)*4]
	bswap	eax

	mov	dword[w + (n)*4], eax
	mov	edi, eax
	crash.sha256.round a, b, c, d, e, f, g, h, (crash._.sha256_table + (n)*4)
}

macro crash.sha256.round_17_64 a, b, c, d, e, f, g, h, n, rep_num
{
	recalculate_w n
	mov	edi, [w + (n)*4]
	crash.sha256.round a, b, c, d, e, f, g, h, (crash._.sha256_table + (n+16*rep_num)*4)
}


proc crash.sha256 _sha256, _data
locals
	w	rd 64
	A	rd 1
	B	rd 1
	C	rd 1
	D	rd 1
	E	rd 1
	F	rd 1
	G	rd 1
	H	rd 1
endl
	mov	edi, [_sha256]
	mov	eax, [edi + 0x00]
	mov	[A], eax
	mov	eax, [edi + 0x04]
	mov	[B], eax
	mov	eax, [edi + 0x08]
	mov	[C], eax
	mov	eax, [edi + 0x0c]
	mov	[D], eax
	mov	eax, [edi + 0x10]
	mov	[E], eax
	mov	eax, [edi + 0x14]
	mov	[F], eax
	mov	eax, [edi + 0x18]
	mov	[G], eax
	mov	eax, [edi + 0x1c]
	mov	[H], eax

	crash.sha256.round_1_16		A, B, C, D, E, F, G, H,  0
	crash.sha256.round_1_16		H, A, B, C, D, E, F, G,  1
	crash.sha256.round_1_16		G, H, A, B, C, D, E, F,  2
	crash.sha256.round_1_16		F, G, H, A, B, C, D, E,  3
	crash.sha256.round_1_16		E, F, G, H, A, B, C, D,  4
	crash.sha256.round_1_16		D, E, F, G, H, A, B, C,  5
	crash.sha256.round_1_16		C, D, E, F, G, H, A, B,  6
	crash.sha256.round_1_16		B, C, D, E, F, G, H, A,  7
	crash.sha256.round_1_16		A, B, C, D, E, F, G, H,  8
	crash.sha256.round_1_16		H, A, B, C, D, E, F, G,  9
	crash.sha256.round_1_16		G, H, A, B, C, D, E, F, 10
	crash.sha256.round_1_16		F, G, H, A, B, C, D, E, 11
	crash.sha256.round_1_16		E, F, G, H, A, B, C, D, 12
	crash.sha256.round_1_16		D, E, F, G, H, A, B, C, 13
	crash.sha256.round_1_16		C, D, E, F, G, H, A, B, 14
	crash.sha256.round_1_16		B, C, D, E, F, G, H, A, 15

repeat 3
	crash.sha256.round_17_64	A, B, C, D, E, F, G, H,  0, %
	crash.sha256.round_17_64	H, A, B, C, D, E, F, G,  1, %
	crash.sha256.round_17_64	G, H, A, B, C, D, E, F,  2, %
	crash.sha256.round_17_64	F, G, H, A, B, C, D, E,  3, %
	crash.sha256.round_17_64	E, F, G, H, A, B, C, D,  4, %
	crash.sha256.round_17_64	D, E, F, G, H, A, B, C,  5, %
	crash.sha256.round_17_64	C, D, E, F, G, H, A, B,  6, %
	crash.sha256.round_17_64	B, C, D, E, F, G, H, A,  7, %
	crash.sha256.round_17_64	A, B, C, D, E, F, G, H,  8, %
	crash.sha256.round_17_64	H, A, B, C, D, E, F, G,  9, %
	crash.sha256.round_17_64	G, H, A, B, C, D, E, F, 10, %
	crash.sha256.round_17_64	F, G, H, A, B, C, D, E, 11, %
	crash.sha256.round_17_64	E, F, G, H, A, B, C, D, 12, %
	crash.sha256.round_17_64	D, E, F, G, H, A, B, C, 13, %
	crash.sha256.round_17_64	C, D, E, F, G, H, A, B, 14, %
	crash.sha256.round_17_64	B, C, D, E, F, G, H, A, 15, %
end repeat

	mov	edi, [_sha256]
	mov	eax, [A]
	add	[edi + 0x00], eax
	mov	eax, [B]
	add	[edi + 0x04], eax
	mov	eax, [C]
	add	[edi + 0x08], eax
	mov	eax, [D]
	add	[edi + 0x0c], eax
	mov	eax, [E]
	add	[edi + 0x10], eax
	mov	eax, [F]
	add	[edi + 0x14], eax
	mov	eax, [G]
	add	[edi + 0x18], eax
	mov	eax, [H]
	add	[edi + 0x1c], eax
	add	esi, 64

	ret
endp

