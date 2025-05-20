macro buttabnum a0,a1,a2,a3,a4
{
	test [flags],(1 shl 5)
	jnz .prg
	test [flags],(1 shl 3)
	jnz .a1
if a4
	test [flags],(1 shl 4)
	jnz .a4
endf
	test [flags],(1 shl 11)
	jnz .a2
	test [flags],(1 shl 12)
	jnz .a3
	test [flags],(1 shl 9)
	jnz .prg
	mov eax,a0
	jmp but_table.to_opcode
		.a1:	mov eax,a1
		jmp but_table.to_opcode
		.a2:	mov eax,a2
		jmp but_table.to_opcode
		.a3:	mov eax,a3
		jmp but_table.to_opcode
	.prg:
	test [flags],(1 shl 8)
	jz @f
	add [dop8],a0
	jmp but_table.dop8
    @@: test [flags],(1 shl 9)
	jz @f
	mov al,a0
	jmp but_table.dop9
    @@:	movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	test [flags],(1 shl 3)
	jnz @f
	mov byte [eax],a0
	ret
    @@:	mov byte [eax],a1
	btr [flags],3
	ret
if a4
.a4:	mov eax,a4
	jmp but_table.to_opcode
endf	
}