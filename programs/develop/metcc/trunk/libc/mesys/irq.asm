format ELF
section '.text' executable
public _msys_get_irq_owner
_msys_get_irq_owner:
;arg1 - irq
  mov   edx,ebx
  mov   eax,41
  mov   ebx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4
  
public _msys_get_data_read_by_irq
_msys_get_data_read_by_irq:
;arg1 - irq
;arg2 - *size
;arg3 - data
  mov   edx,ebx
  mov   eax,42
  mov   ebx,[esp+4]
  int   0x40
  cmp   ecx,2
  jz    .not_an_owner
  push  ecx
  mov   ecx,[esp+16]
  test  ecx,ecx
  jz    .ignore_data
  mov   [ecx],bl
.ignore_data:    
  mov   ecx,[esp+12]
  mov   [ecx],eax
  pop   eax
  mov   ebx,edx
  ret   12
.not_an_owner:
  mov   eax,2
  mov   ebx,edx
  ret
  
public _msys_send_data_to_device
_msys_send_data_to_device:
;arg1 - port
;arg2 - data
  mov   edx,ebx
  mov   eax,63
  mov   ebx,[esp+8]
  mov   ecx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   8 
  
public _msys_receive_data_from_device
_msys_receive_data_from_device:  
;arg1 - port
;arg2 - data
  mov   edx,ebx
  mov   eax,43
  mov   ecx,[esp+4]
  add   ecx,0x80000000
  int   0x40
  mov   ecx,[esp+8]
  mov   [ecx],bl
  mov   ebx,edx
  ret   8
  
public _msys_program_irq
_msys_program_irq:
;arg1 - intrtable
;arg2 - irq
  mov   edx,ebx
  mov   eax,44
  mov   ebx,[esp+4]
  mov   ecx,[esp+8]
  int   0x40
  mov   ebx,edx
  ret   8
  
public _msys_reserve_irq
_msys_reserve_irq:
;arg1 - irq
  mov   edx,ebx
  mov   eax,45
  xor   ebx,ebx
  mov   ecx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4
  
public _msys_free_irq
_msys_free_irq:
;arg1 - irq
  mov   edx,ebx
  mov   eax,45
  xor   ebx,ebx
  inc   ebx
  mov   ecx,[esp+4]
  int   0x40
  mov   ebx,edx
  ret   4
  
public _msys_reserve_port_area
_msys_reserve_port_area:
;arg1 - start
;arg2 - end
  push  ebx
  mov   eax,46    
  xor   ebx,ebx
  mov   ecx,[esp+8]
  mov   edx,[esp+12]
  int   0x40
  pop   ebx
  ret   8
  
public _msys_free_port_area
_msys_free_port_area:
;arg1 - start
;arg2 - end
  push  ebx
  mov   eax,46
  xor   ebx,ebx
  inc   ebx
  mov   ecx,[esp+8]
  mov   edx,[esp+12]
  int   0x40
  pop   ebx
  ret   8
  