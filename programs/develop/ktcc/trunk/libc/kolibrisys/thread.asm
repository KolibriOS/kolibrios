format ELF
include "public_stdcall.inc"
section '.text' executable
extrn malloc
public_stdcall _ksys_start_thread,12
;arg1 - proc
;arg2 - stacksize
;arg3 - pid
  push  dword [esp+8]
  call  malloc
  test  eax,eax
  jz    .no_mem
  push  ebx
  mov   edx,eax
  add   edx,[esp+12]
  mov   [edx-4],dword 0
  mov   ecx,[esp+8]
  mov   ebx,1
  mov   eax,51
  int   0x40
  mov   ebx,[esp+16]
  test  ebx,ebx
  jz    .no_val
  mov   [ebx],eax
.no_val:  
  mov   eax,edx
  sub   eax,[esp+12]
  pop   ebx
  ret   12
.no_mem:
  mov   ecx,[esp+12]
  mov   [ecx],eax
  ret   12