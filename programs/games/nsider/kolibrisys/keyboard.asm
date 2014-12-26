format COFF

include "../../../proc32.inc"

section '.text' code

public __ksys_get_key
public __ksys_set_keyboard_mode

align 4
proc __ksys_get_key stdcall

  mov   eax,2
  int   0x40
  ret

endp

align 4
proc __ksys_set_keyboard_mode stdcall, mode:dword

  mov   edx,ebx
  mov   eax,66 
  xor   ebx,ebx
  inc   ebx
  mov   ecx,[mode]
  mov   ebx,edx
  ret
endp