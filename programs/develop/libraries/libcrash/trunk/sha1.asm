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

proc crash.sha1.f
	push	ebx ecx edx
	xor	ecx, edx
	and	ebx, ecx
	xor	ebx, edx
	mov	esi, ebx
	pop	edx ecx ebx
	ret
endp

proc crash.sha1.g
	push	ebx ecx edx
	xor	ebx, ecx
	xor	ebx, edx
	mov	esi, ebx
	pop	edx ecx ebx
	ret
endp

proc crash.sha1.h
	push	ebx ecx edx
	mov	esi, ebx
	and	ebx, ecx
	and	ecx, edx
	and	esi, edx
	or	ebx, ecx
	or	esi, ebx
	pop	edx ecx ebx
	ret
endp

macro crash.sha1.round f, k, c
{
	mov	esi, eax
	rol	esi, 5
	mov	[temp], esi
	call	f

	add	esi, edi
	add	[temp], esi
	mov	esi, [w + (c)*4]
	add	esi, k
	add	[temp], esi

	mov	edi, edx
	mov	edx, ecx
	mov	ecx, ebx
	rol	ecx, 30
	mov	ebx, eax
	mov	eax, [temp]
}


proc crash.sha1 _sha1, _data
locals
	temp	 rd 1
	w	 rd 80
endl
	lea	edi, [w]
	xor	ecx, ecx
    @@:
	mov	eax, [esi]
	add	esi, 4
	bswap	eax
	mov	[edi], eax
	add	edi, 4
	add	ecx, 1
	cmp	ecx, 16
	jne	@b
    @@:
	mov	eax, [w + (ecx -  3)*4]
	xor	eax, [w + (ecx -  8)*4]
	xor	eax, [w + (ecx - 14)*4]
	xor	eax, [w + (ecx - 16)*4]
	rol	eax, 1
	mov	[w + ecx*4], eax
	add	ecx, 1
	cmp	ecx, 80
	jne	@b

	mov	edi, [_sha1]
	mov	eax, [edi + 0x00]
	mov	ebx, [edi + 0x04]
	mov	ecx, [edi + 0x08]
	mov	edx, [edi + 0x0c]
	mov	edi, [edi + 0x10]

	push	esi

repeat 20
	crash.sha1.round	crash.sha1.f, 0x5a827999, %-1
end repeat

repeat 20
	crash.sha1.round	crash.sha1.g, 0x6ed9eba1, %-1+20
end repeat

repeat 20
	crash.sha1.round	crash.sha1.h, 0x8f1bbcdc, %-1+40
end repeat

repeat 20
	crash.sha1.round	crash.sha1.g, 0xca62c1d6, %-1+60
end repeat

	pop	esi

	mov	[temp], edi
	mov	edi, [_sha1]
	add	[edi + 0x00], eax
	add	[edi + 0x04], ebx
	add	[edi + 0x08], ecx
	add	[edi + 0x0c], edx
	mov	eax, [temp]
	add	[edi + 0x10], eax

	ret
endp

