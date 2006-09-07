format ELF
section '.text' executable
public _msys_wait_for_event_infinite
_msys_wait_for_event_infinite:
  mov  eax,10
  int  0x40
  ret
  
public _msys_check_for_event
_msys_check_for_event:
  mov  eax,11
  int  0x40
  ret
  
public _msys_wait_for_event
_msys_wait_for_event:
;arg1 - time
  mov  edx,ebx
  mov  eax,23
  mov  ebx,[esp+4]
  int  0x40
  mov  ebx,edx
  ret  4
  
public _msys_set_wanted_events
_msys_set_wanted_events:
;arg1 - flags
  mov  edx,ebx
  mov  eax,40
  mov  ebx,[esp+4]
  int  0x40
  mov  ebx,edx
  ret  4