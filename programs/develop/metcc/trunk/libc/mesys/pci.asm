format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_get_pci_version,0
  mov   edx,ebx
  mov   eax,62
  xor   ebx,ebx
  int   0x40
  movzx eax,ax
  mov   ebx,edx
  ret
  
public_stdcall _msys_get_last_pci_bus,0
  mov   edx,ebx
  mov   eax,62
  xor   ebx,ebx
  inc   ebx
  int   0x40
  movzx eax,al
  mov   ebx,edx
  ret  
  
public_stdcall _msys_get_pci_access_mechanism,0
  mov   edx,ebx
  mov   eax,62
  mov   ebx,2
  int   0x40
  movzx eax,al  
  mov   ebx,edx
  ret
  
public_stdcall _msys_pci_read_config_byte,16
;arg1 - bus
;arg2 - dev
;arg3 - fn
;arg4 - reg
  mov   edx,ebx
  mov   eax,62
  mov   bl,4
  mov   bh,[esp+4]
  mov   ch,[esp+8]
  shl   ch,3
  add   ch,[esp+12]
  mov   cl,[esp+16]
  int   0x40
  mov   ebx,edx
  ret   16
  
public_stdcall _msys_pci_read_config_word,16
;arg1 - bus
;arg2 - dev
;arg3 - fn
;arg4 - reg
  mov   edx,ebx
  mov   eax,62
  mov   bl,5
  mov   bh,[esp+4]
  mov   ch,[esp+8]
  shl   ch,3
  add   ch,[esp+12]
  mov   cl,[esp+16]
  int   0x40
  mov   ebx,edx
  ret   16
  
public_stdcall _msys_pci_read_config_dword,16
;arg1 - bus
;arg2 - dev
;arg3 - fn
;arg4 - reg
  mov   edx,ebx
  mov   eax,62
  mov   bl,6
  mov   bh,[esp+4]
  mov   ch,[esp+8]
  shl   ch,3
  add   ch,[esp+12]
  mov   cl,[esp+16]
  int   0x40
  mov   ebx,edx
  ret   16
  
public_stdcall _msys_pci_write_config_byte,20
;arg1 - bus
;arg2 - dev
;arg3 - fn
;arg4 - reg
;arg5 - value
  push  ebx
  mov   eax,62
  mov   bl,8
  mov   bh,[esp+8]
  mov   ch,[esp+12]
  shl   ch,3
  mov   ch,[esp+16]
  mov   cl,[esp+20]
  movzx edx,byte [esp+24]
  int   0x40
  pop   ebx
  ret   20
  
public_stdcall _msys_pci_write_config_word,20
;arg1 - bus
;arg2 - dev
;arg3 - fn
;arg4 - reg
;arg5 - value
  push  ebx
  mov   eax,62
  mov   bl,9
  mov   bh,[esp+8]
  mov   ch,[esp+12]
  shl   ch,3
  mov   ch,[esp+16]
  mov   cl,[esp+20]
  movzx edx,word [esp+24]
  int   0x40
  pop   ebx
  ret   20
  
public_stdcall _msys_pci_write_config_dword,20
;arg1 - bus
;arg2 - dev
;arg3 - fn
;arg4 - reg
;arg5 - value
  push  ebx
  mov   eax,62
  mov   bl,10
  mov   bh,[esp+8]
  mov   ch,[esp+12]
  shl   ch,3
  mov   ch,[esp+16]
  mov   cl,[esp+20]
  mov   edx,[esp+24]
  int   0x40
  pop   ebx
  ret   20         