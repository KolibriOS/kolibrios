format ELF
include "public_stdcall.inc"
section '.text' executable
public_stdcall _msys_wait_for_event_infinite,0
  mov  eax,10
  int  0x40
  ret
  
public_stdcall _msys_check_for_event,0
  mov  eax,11
  int  0x40
  ret
  
public_stdcall _msys_wait_for_event,4
;arg1 - time
  mov  edx,ebx
  mov  eax,23
  mov  ebx,[esp+4]
  int  0x40
  mov  ebx,edx
  ret  4
  
public_stdcall _msys_set_wanted_events,4
;arg1 - flags
  mov  edx,ebx
  mov  eax,40
  mov  ebx,[esp+4]
  int  0x40
  mov  ebx,edx
  ret  4