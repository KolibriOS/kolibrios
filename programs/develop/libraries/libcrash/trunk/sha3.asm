;    libcrash -- cryptographic hash functions
;
;    Copyright (C) 2013 Ivan Baravy (dunkaist)
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

macro keccak_rol_xor nd, ncl, ncr
{
	movq	mm0, [C + 8*(ncl)]
	movq	mm1, mm0
	psllq	mm0, 1
	psrlq	mm1, 63
	por	mm0, mm1
	pxor	mm0, [C + 8*(ncr)]
	movq	[D + 8*(nd)], mm0
}

proc keccak_theta
locals
	C	rq 5
	D	rq 5
endl

repeat 5
	movq	mm0, [edi + 8*(%-1 +  0)]
	pxor	mm0, [edi + 8*(%-1 +  5)]
	pxor	mm0, [edi + 8*(%-1 + 10)]
	pxor	mm0, [edi + 8*(%-1 + 15)]
	pxor	mm0, [edi + 8*(%-1 + 20)]
	movq	[C + 8*(%-1)], mm0
end repeat

	keccak_rol_xor	0, 1, 4
	keccak_rol_xor	1, 2, 0
	keccak_rol_xor	2, 3, 1
	keccak_rol_xor	3, 4, 2
	keccak_rol_xor	4, 0, 3

repeat 5
	movq	mm1, [D + 8*(%-1)]
	movq	mm0, mm1
	pxor	mm0, [edi + 8*(%-1 +  0)]
	movq	[edi + 8*(%-1 +  0)], mm0
	movq	mm0, mm1
	pxor	mm0, [edi + 8*(%-1 +  5)]
	movq	[edi + 8*(%-1 +  5)], mm0
	movq	mm0, mm1
	pxor	mm0, [edi + 8*(%-1 + 10)]
	movq	[edi + 8*(%-1 + 10)], mm0
	movq	mm0, mm1
	pxor	mm0, [edi + 8*(%-1 + 15)]
	movq	[edi + 8*(%-1 + 15)], mm0
	movq	mm0, mm1
	pxor	mm0, [edi + 8*(%-1 + 20)]
	movq	[edi + 8*(%-1 + 20)], mm0
end repeat

	ret
endp


proc keccak_pi
	movq	mm1, [edi + 8*1]
	movq	mm0, [edi + 8*6]
	movq	[edi + 8*1], mm0
	movq	mm0, [edi + 8*9]
	movq	[edi + 8*6], mm0
	movq	mm0, [edi + 8*22]
	movq	[edi + 8*9], mm0
	movq	mm0, [edi + 8*14]
	movq	[edi + 8*22], mm0
	movq	mm0, [edi + 8*20]
	movq	[edi + 8*14], mm0
	movq	mm0, [edi + 8*2]
	movq	[edi + 8*20], mm0
	movq	mm0, [edi + 8*12]
	movq	[edi + 8*2], mm0
	movq	mm0, [edi + 8*13]
	movq	[edi + 8*12], mm0
	movq	mm0, [edi + 8*19]
	movq	[edi + 8*13], mm0
	movq	mm0, [edi + 8*23]
	movq	[edi + 8*19], mm0
	movq	mm0, [edi + 8*15]
	movq	[edi + 8*23], mm0
	movq	mm0, [edi + 8*4]
	movq	[edi + 8*15], mm0
	movq	mm0, [edi + 8*24]
	movq	[edi + 8*4], mm0
	movq	mm0, [edi + 8*21]
	movq	[edi + 8*24], mm0
	movq	mm0, [edi + 8*8]
	movq	[edi + 8*21], mm0
	movq	mm0, [edi + 8*16]
	movq	[edi + 8*8], mm0
	movq	mm0, [edi + 8*5]
	movq	[edi + 8*16], mm0
	movq	mm0, [edi + 8*3]
	movq	[edi + 8*5], mm0
	movq	mm0, [edi + 8*18]
	movq	[edi + 8*3], mm0
	movq	mm0, [edi + 8*17]
	movq	[edi + 8*18], mm0
	movq	mm0, [edi + 8*11]
	movq	[edi + 8*17], mm0
	movq	mm0, [edi + 8*7]
	movq	[edi + 8*11], mm0
	movq	mm0, [edi + 8*10]
	movq	[edi + 8*7], mm0
	movq	[edi + 8*10], mm1

	ret
endp


proc keccak_chi

	mov	eax, 0xffffffff
	movd	mm0, eax
	movq	mm2, mm0
	punpckldq	mm2, mm0

repeat 5
	movq	mm6, [edi + 8*(0 + 5*(%-1))]
	movq	mm7, [edi + 8*(1 + 5*(%-1))]

	movq	mm0, [edi + 8*(0 + 5*(%-1))]
	movq	mm1, mm7
	pandn	mm1, mm2
	pand	mm1, [edi + 8*(2 + 5*(%-1))]
	pxor	mm0, mm1
	movq	[edi + 8*(0 + 5*(%-1))], mm0

	movq	mm0, [edi + 8*(1 + 5*(%-1))]
	movq	mm1, [edi + 8*(2 + 5*(%-1))]
	pandn	mm1, mm2
	pand	mm1, [edi + 8*(3 + 5*(%-1))]
	pxor	mm0, mm1
	movq	[edi + 8*(1 + 5*(%-1))], mm0

	movq	mm0, [edi + 8*(2 + 5*(%-1))]
	movq	mm1, [edi + 8*(3 + 5*(%-1))]
	pandn	mm1, mm2
	pand	mm1, [edi + 8*(4 + 5*(%-1))]
	pxor	mm0, mm1
	movq	[edi + 8*(2 + 5*(%-1))], mm0

	movq	mm0, [edi + 8*(3 + 5*(%-1))]
	movq	mm1, [edi + 8*(4 + 5*(%-1))]
	pandn	mm1, mm2
	pand	mm1, mm6
	pxor	mm0, mm1
	movq	[edi + 8*(3 + 5*(%-1))], mm0

	movq	mm0, [edi + 8*(4 + 5*(%-1))]
	movq	mm1, mm6
	pandn	mm1, mm2
	pand	mm1, mm7
	pxor	mm0, mm1
	movq	[edi + 8*(4 + 5*(%-1))], mm0
end repeat
	ret
endp


macro keccak_rol_mov n, c
{
	movq	mm0, [edi + 8*(n)]
	movq	mm1, mm0
	psllq	mm0, (c)
	psrlq	mm1, (64-(c))
	por	mm0, mm1
	movq	[edi + 8*(n)], mm0
}

proc keccak_permutation

repeat 24
	stdcall	keccak_theta

	keccak_rol_mov	 1,  1
	keccak_rol_mov	 2, 62
	keccak_rol_mov	 3, 28
	keccak_rol_mov	 4, 27
	keccak_rol_mov	 5, 36
	keccak_rol_mov	 6, 44
	keccak_rol_mov	 7,  6
	keccak_rol_mov	 8, 55
	keccak_rol_mov	 9, 20
	keccak_rol_mov	10,  3
	keccak_rol_mov	11, 10
	keccak_rol_mov	12, 43
	keccak_rol_mov	13, 25
	keccak_rol_mov	14, 39
	keccak_rol_mov	15, 41
	keccak_rol_mov	16, 45
	keccak_rol_mov	17, 15
	keccak_rol_mov	18, 21
	keccak_rol_mov	19,  8
	keccak_rol_mov	20, 18
	keccak_rol_mov	21,  2
	keccak_rol_mov	22, 61
	keccak_rol_mov	23, 56
	keccak_rol_mov	24, 14

	stdcall	keccak_pi
	stdcall	keccak_chi

	movq	mm0, [edi + 8*(0)]
	pxor	mm0, [crash._.sha3_round + 8*(%-1)]
	movq	[edi + 8*(0)], mm0
end repeat

	ret
endp


proc crash.sha3_224 _hash, _data
	mov	edi, [_hash]

repeat 18
	movq	mm0, [esi + 8*(%-1)]
	pxor	mm0, [edi + 8*(%-1)]
	movq	[edi + 8*(%-1)], mm0
end repeat

	stdcall	keccak_permutation

	add	esi, 144
	ret
endp


proc crash.sha3_256 _hash, _data
	mov	edi, [_hash]

repeat 17
	movq	mm0, [esi + 8*(%-1)]
	pxor	mm0, [edi + 8*(%-1)]
	movq	[edi + 8*(%-1)], mm0
end repeat

	stdcall	keccak_permutation

	add	esi, 136
	ret
endp


proc crash.sha3_384 _hash, _data
	mov	edi, [_hash]

repeat 13
	movq	mm0, [esi + 8*(%-1)]
	pxor	mm0, [edi + 8*(%-1)]
	movq	[edi + 8*(%-1)], mm0
end repeat

	stdcall	keccak_permutation

	add	esi, 104
	ret
endp


proc crash.sha3_512 _hash, _data
	mov	edi, [_hash]

repeat 9
	movq	mm0, [esi + 8*(%-1)]
	pxor	mm0, [edi + 8*(%-1)]
	movq	[edi + 8*(%-1)], mm0
end repeat

	stdcall	keccak_permutation

	add	esi, 72
	ret
endp

