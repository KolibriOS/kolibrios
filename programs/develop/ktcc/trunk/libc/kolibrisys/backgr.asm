format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _ksys_set_background_size,8
;arg1 - xsize
;arg2 - ysize
  push  ebx
  mov   ecx,[esp+8]
  mov   edx,[esp+12]
  mov   eax,15
  mov   ebx,1
  int   0x40
  pop   ebx
  ret   8
public_stdcall _ksys_write_background_mem,8
;arg1 - pos
;arg2 - color
  push  ebx
  mov   eax,15
  mov   ebx,2
  mov   ecx,[esp+8]
  mov   edx,[esp+12]
  int   0x40
  pop   ebx
  ret   8
public_stdcall _ksys_draw_background,0
  mov   edx,ebx
  mov   eax,15
  mov   ebx,3
  int   0x40
  mov   ebx,edx
  ret    
public_stdcall _ksys_set_background_draw_type,4
;arg1 - type
  mov   edx,ebx
  mov   eax,15
  mov   ebx,4
  mov   ecx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4
public_stdcall _ksys_background_blockmove,12
;arg1 - source
;arg2 - position in dest
;arg3 - size
  push  ebx esi
  mov   eax,15
  mov   ebx,5
  mov   ecx,[esp+12]
  mov   edx,[esp+16]
  mov   esi,[esp+20]
  int   0x40
  pop   esi ebx
  ret   12