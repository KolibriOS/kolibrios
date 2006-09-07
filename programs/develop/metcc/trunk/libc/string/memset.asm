format ELF
section '.text' executable
public memset
memset:
  push	edi
  mov	edi,[esp+8]
  mov	eax,[esp+12]
  mov	ecx,[esp+16]
  jecxz  .no_set
  cld
  rep	stosb
.no_set:
  pop	edi
  ret
  