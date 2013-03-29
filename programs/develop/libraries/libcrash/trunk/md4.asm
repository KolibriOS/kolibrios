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

macro crash.md4.f b, c, d
{
	push	c
	xor	c, d
	and	b, c
	xor	b, d
	pop	c
}

macro crash.md4.g b, c, d
{
	push	c  d
	mov	edi, b
	and	b, c
	and	c, d
	and	d, edi
	or	b, c
	or	b, d
	pop	d  c
}

macro crash.md4.h b, c, d
{
	xor	b, c
	xor	b, d
}

macro crash.md4.round func, a, b, c, d, index, shift, ac
{
	push	b
	func	b, c, d
	lea	a, [a + b + ac]
	add	a, [esi + index*4]
	rol	a, shift
	pop	b
}


proc crash.md4 _md4, _data
	mov	edi, [_md4]
	mov	eax, [edi + 0x0]
	mov	ebx, [edi + 0x4]
	mov	ecx, [edi + 0x8]
	mov	edx, [edi + 0xc]

	crash.md4.round		crash.md4.f, eax, ebx, ecx, edx,  0,  3, 0x00000000
	crash.md4.round		crash.md4.f, edx, eax, ebx, ecx,  1,  7, 0x00000000
	crash.md4.round		crash.md4.f, ecx, edx, eax, ebx,  2, 11, 0x00000000
	crash.md4.round		crash.md4.f, ebx, ecx, edx, eax,  3, 19, 0x00000000
	crash.md4.round		crash.md4.f, eax, ebx, ecx, edx,  4,  3, 0x00000000
	crash.md4.round		crash.md4.f, edx, eax, ebx, ecx,  5,  7, 0x00000000
	crash.md4.round		crash.md4.f, ecx, edx, eax, ebx,  6, 11, 0x00000000
	crash.md4.round		crash.md4.f, ebx, ecx, edx, eax,  7, 19, 0x00000000
	crash.md4.round		crash.md4.f, eax, ebx, ecx, edx,  8,  3, 0x00000000
	crash.md4.round		crash.md4.f, edx, eax, ebx, ecx,  9,  7, 0x00000000
	crash.md4.round		crash.md4.f, ecx, edx, eax, ebx, 10, 11, 0x00000000
	crash.md4.round		crash.md4.f, ebx, ecx, edx, eax, 11, 19, 0x00000000
	crash.md4.round		crash.md4.f, eax, ebx, ecx, edx, 12,  3, 0x00000000
	crash.md4.round		crash.md4.f, edx, eax, ebx, ecx, 13,  7, 0x00000000
	crash.md4.round		crash.md4.f, ecx, edx, eax, ebx, 14, 11, 0x00000000
	crash.md4.round		crash.md4.f, ebx, ecx, edx, eax, 15, 19, 0x00000000

	crash.md4.round		crash.md4.g, eax, ebx, ecx, edx,  0,  3, 0x5a827999
	crash.md4.round		crash.md4.g, edx, eax, ebx, ecx,  4,  5, 0x5a827999
	crash.md4.round		crash.md4.g, ecx, edx, eax, ebx,  8,  9, 0x5a827999
	crash.md4.round		crash.md4.g, ebx, ecx, edx, eax, 12, 13, 0x5a827999
	crash.md4.round		crash.md4.g, eax, ebx, ecx, edx,  1,  3, 0x5a827999
	crash.md4.round		crash.md4.g, edx, eax, ebx, ecx,  5,  5, 0x5a827999
	crash.md4.round		crash.md4.g, ecx, edx, eax, ebx,  9,  9, 0x5a827999
	crash.md4.round		crash.md4.g, ebx, ecx, edx, eax, 13, 13, 0x5a827999
	crash.md4.round		crash.md4.g, eax, ebx, ecx, edx,  2,  3, 0x5a827999
	crash.md4.round		crash.md4.g, edx, eax, ebx, ecx,  6,  5, 0x5a827999
	crash.md4.round		crash.md4.g, ecx, edx, eax, ebx, 10,  9, 0x5a827999
	crash.md4.round		crash.md4.g, ebx, ecx, edx, eax, 14, 13, 0x5a827999
	crash.md4.round		crash.md4.g, eax, ebx, ecx, edx,  3,  3, 0x5a827999
	crash.md4.round		crash.md4.g, edx, eax, ebx, ecx,  7,  5, 0x5a827999
	crash.md4.round		crash.md4.g, ecx, edx, eax, ebx, 11,  9, 0x5a827999
	crash.md4.round		crash.md4.g, ebx, ecx, edx, eax, 15, 13, 0x5a827999

	crash.md4.round		crash.md4.h, eax, ebx, ecx, edx,  0,  3, 0x6ed9eba1
	crash.md4.round		crash.md4.h, edx, eax, ebx, ecx,  8,  9, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ecx, edx, eax, ebx,  4, 11, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ebx, ecx, edx, eax, 12, 15, 0x6ed9eba1
	crash.md4.round		crash.md4.h, eax, ebx, ecx, edx,  2,  3, 0x6ed9eba1
	crash.md4.round		crash.md4.h, edx, eax, ebx, ecx, 10,  9, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ecx, edx, eax, ebx,  6, 11, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ebx, ecx, edx, eax, 14, 15, 0x6ed9eba1
	crash.md4.round		crash.md4.h, eax, ebx, ecx, edx,  1,  3, 0x6ed9eba1
	crash.md4.round		crash.md4.h, edx, eax, ebx, ecx,  9,  9, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ecx, edx, eax, ebx,  5, 11, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ebx, ecx, edx, eax, 13, 15, 0x6ed9eba1
	crash.md4.round		crash.md4.h, eax, ebx, ecx, edx,  3,  3, 0x6ed9eba1
	crash.md4.round		crash.md4.h, edx, eax, ebx, ecx, 11,  9, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ecx, edx, eax, ebx,  7, 11, 0x6ed9eba1
	crash.md4.round		crash.md4.h, ebx, ecx, edx, eax, 15, 15, 0x6ed9eba1

	mov	edi, [_md4]
	add	[edi + 0x0], eax
	add	[edi + 0x4], ebx
	add	[edi + 0x8], ecx
	add	[edi + 0xc], edx
	add	esi, 64

	ret
endp

