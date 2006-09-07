format ELF
section '.text' executable
public _msys_sound_load_block
_msys_sound_load_block:
;arg1 - blockptr
  mov   edx,ebx
  mov   eax,55
  xor   ebx,ebx
  mov   ecx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4
  
public _msys_sound_play_block
_msys_sound_play_block:
  mov   edx,ebx
  mov   eax,55 
  xor   ebx,ebx
  inc   ebx
  int   0x40
  mov   ebx,edx
  ret 
  
public _msys_sound_set_channels
_msys_sound_set_channels:
;arg1 - channels
  push  ebx
  mov   eax,55
  mov   ebx,2
  xor   ecx,ecx
  mov   edx,[esp+8]
  int   0x40
  pop   ebx
  ret   4
  
public _msys_sound_set_data_size
_msys_sound_set_data_size:
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
  
public _msys_sound_set_frequency
_msys_sound_set_frequency:
;arg1 - frequency
  push  ebx
  mov   eax,55
  mov   ebx,2
  mov   ecx,2
  mov   edx,[esp+8]
  int   0x40
  pop   ebx
  ret   4
  
public _msys_sound_speaker_play
_msys_sound_speaker_play:
;arg1 - data
  mov   edx,ebx
  mov   eax,55
  mov   ebx,55
  mov   ecx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4