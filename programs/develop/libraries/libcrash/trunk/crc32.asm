proc crash.crc32 _crc, _data, _len, _callback, _msglen
  .begin:
	mov	ecx, [_len]
	test	ecx, ecx
	jz	.callback
	mov	eax, [_crc]
	mov	eax, [eax]
	mov	esi, [_data]
  .head:
	mov	ebx, [_data]
	and	ebx, 3
	jz	.body
    @@:
	dec	ecx
	js	.done
	movzx	edx, al
	xor	dl, byte[esi]
	add	esi, 1
	shr	eax, 8
	xor	eax, [crash._.crc32_table + edx*4]
	dec	ebx
	jnz	@b
  .body:
	mov	ebx, ecx
	and	ecx, 15
	shr	ebx, 4
	test	ebx, ebx
	jz	.tail
    @@:
repeat 4
	mov	edx, eax
	mov	eax, [esi]
	add	esi, 4
	xor	eax, edx
repeat 4
	movzx	edx, al
	shr	eax, 8
	xor	eax, [crash._.crc32_table + edx*4]
end repeat
end repeat
	dec	ebx
	jnz	@b
  .tail:
	test	ecx, ecx
	jz	.done
    @@:
	movzx	edx, al
	xor	dl, byte[esi]
	add	esi, 1
	shr	eax, 8
	xor	eax, [crash._.crc32_table + edx*4]
	dec	ecx
	jnz	@b
  .done:
	mov	ecx, [_crc]
	mov	[ecx], eax
  .callback:
	mov	eax, [_callback]
	test	eax, eax
	jz	@f
	call	eax
	mov	[_len], eax
	test	eax, eax
	jnz	.begin
    @@:
	mov	ecx, [_crc]
	mov	eax, [ecx]
	xor	eax, 0xffffffff
	bswap	eax
	mov	[ecx], eax

	ret
endp
