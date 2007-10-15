format ELF

section '.text' executable

public _ksys_wait_for_event_infinite
public _ksys_check_for_event
public _ksys_wait_for_event
public _ksys_set_wanted_events

_ksys_wait_for_event_infinite:

  mov  eax,10
  int  0x40

  ret
  
_ksys_check_for_event:

  mov  eax,11
  int  0x40

  ret
  
_ksys_wait_for_event:

;arg1 - time
  mov  eax,23
  mov  ebx,[esp+4]
  int  0x40

  ret 4 
  
_ksys_set_wanted_events:

;arg1 - flags
  mov  eax,40
  mov  ebx,[esp+4]
  int  0x40

  ret  4