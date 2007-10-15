format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _ksys_dga_get_resolution,16
;arg1 - *xres
;arg2 - *yres
;arg3 - *bpp
;arg4 - *bpscan
  mov   edx,ebx
  
  mov   eax,61
  mov   ebx,1
  int   0x40
  mov   ebx,[esp+8]
  mov   [ebx],ax
  mov   word [ebx+2],0
  shr   eax,16
  mov   ebx,[esp+4]
  mov   [ebx],eax
  
  mov   eax,61
  mov   ebx,2
  int   0x40
  mov   ebx,[esp+12]
  mov   [ebx],eax
  
  mov   eax,61
  mov   ebx,3
  int   0x40
  mov   ebx,[esp+16]
  mov   [ebx],eax
  
  mov   ebx,edx
  ret   16