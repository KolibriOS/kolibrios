format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _ksy_sound_load_block,4
;arg1 - blockptr
  mov   edx,ebx
  mov   eax,55
  xor   ebx,ebx
  mov   ecx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4
  
public_stdcall _ksy_sound_play_block,0
  mov   edx,ebx
  mov   eax,55 
  xor   ebx,ebx
  inc   ebx
  int   0x40
  mov   ebx,edx
  ret 
  
public_stdcall _ksy_sound_set_channels,4
;arg1 - channels
  push  ebx
  mov   eax,55
  mov   ebx,2
  xor   ecx,ecx
  mov   edx,[esp+8]
  int   0x40
  pop   ebx
  ret   4
  
public_stdcall _ksy_sound_set_data_size,4
;arg1 - data size
  push  ebx
  mov   eax,55
  mov   ebx,2
  xor   ecx,ecx
  inc   ecx
  mov   edx,[esp+8]
  int   0x40
  pop   ebx
  ret   4
  
public_stdcall _ksy_sound_set_frequency,4
;arg1 - frequency
  push  ebx
  mov   eax,55
  mov   ebx,2
  mov   ecx,2
  mov   edx,[esp+8]
  int   0x40
  pop   ebx
  ret   4
  
public_stdcall _ksy_sound_speaker_play,4
;arg1 - data
  mov   edx,ebx
  mov   eax,55
  mov   ebx,55
  mov   ecx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4