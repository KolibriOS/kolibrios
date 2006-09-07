format ELF
section '.text' executable
public memcpy
public memmove
memcpy:
memmove:
  push	esi edi
  mov	edi,[esp+12]
  mov	esi,[esp+16]
  mov	ecx,[esp+20]
  jecxz .no_copy
  cld
  rep	movsb
.no_copy:
  pop	edi esi
  ret