virtual at 0
physical_mem_block:
  .start rd 1
  .size  rd 1
  .flags rd 1  ;0-free, pid-used.   
.sizeof:
end virtual
max_physical_mem_blocks = 24
uglobal
  num_physical_mem_blocks rd 1
  physical_mem_blocks rd 3*max_physical_mem_blocks
endg
Init_Physical_Memory_Manager:
  pushad
  mov  edi,physical_mem_blocks
  mov  ecx,3*max_physical_mem_blocks
  xor  eax,eax
  cld
  rep  stosd
  mov  dword [num_physical_mem_blocks],2
  mov  [physical_mem_blocks+physical_mem_block.start],0x60000
  mov  [physical_mem_blocks+physical_mem_block.size],0x20000 ;128Kb
  mov  [physical_mem_blocks+physical_mem_block.sizeof+physical_mem_block.start],0x780000
  mov  [physical_mem_blocks+physical_mem_block.sizeof+physical_mem_block.size],0x80000 ;512Kb
  popad
  ret
Insert_Block:
;input:
;  eax - handle
;output:
;  none
  push eax ecx esi edi
  sub  eax,[num_physical_mem_blocks]
  neg  eax
  mov  edi,physical_mem_block.sizeof
  imul eax,edi
  shr  eax,2
  mov  ecx,eax
  mov  esi,[num_physical_mem_blocks]
  imul esi,edi
  add  esi,physical_mem_blocks
  lea  edi,[esi+physical_mem_block.sizeof]
  std
  rep  movsd
  pop  edi esi ecx eax
  ret
Delete_Block:
;input:
;  eax - handle
;output:
;  none
  pushad
  mov  edi,eax
  sub  eax,[num_physical_mem_blocks]
  neg  eax
  dec  eax
  mov  esi,physical_mem_block.sizeof
  imul eax,esi
  imul edi,esi
  add  edi,physical_mem_blocks
  lea  esi,[edi+physical_mem_block.sizeof]
  mov  ecx,eax
  shr  ecx,2
  cld
  rep  movsd
  popad
  ret
Allocate_Physical_Block:
;input:
;  eax - size
;output:
;  eax - address or 0 if not enough memory.
  pushad
  cmp  [num_physical_mem_blocks],max_physical_mem_blocks
  jge  .error
  mov  ebx,eax
  xor  eax,eax
  mov  esi,physical_mem_blocks
.loop:
  cmp  dword [esi+physical_mem_block.flags],0
  jnz  .next
  cmp  [esi+physical_mem_block.size],ebx
  jg   .addblock
  jz   .noaddblock
.next:
  inc  eax
  add  esi,physical_mem_block.sizeof
  cmp  eax,[num_physical_mem_blocks]
  jl   .loop
.error:
  popad
  xor  eax,eax
  ret  
.noaddblock:
  mov  eax,[esi+physical_mem_block.start]
  mov  [esp+28],eax
  mov  eax,[0x3010]
  mov  eax,[eax+TASKDATA.pid]
  mov  [esi+physical_mem_block.flags],eax
  popad
  ret
.addblock:
  call Insert_Block
  inc  dword [num_physical_mem_blocks]
  mov  eax,[esi+physical_mem_block.start]
  mov  [esp+28],eax
  mov  ecx,[0x3010]
  mov  ecx,[ecx+TASKDATA.pid]
  mov  [esi+physical_mem_block.flags],ecx
  mov  ecx,[esi+physical_mem_block.size]
  mov  [esi+physical_mem_block.size],ebx
  sub  ecx,ebx
  mov  [esi+physical_mem_block.sizeof+physical_mem_block.size],ecx  
  add  ebx,[esi+physical_mem_block.start]
  mov  [esi+physical_mem_block.sizeof+physical_mem_block.start],ebx
  mov  dword [esi+physical_mem_block.sizeof+physical_mem_block.flags],0
  popad
  ret
Free_Physical_Block:
;input:
;  eax - address
;output:
;  none
  pushad
  test eax,eax
  jz   .ret
  mov  ebx,eax
  xor  eax,eax
  mov  esi,physical_mem_blocks
.loop:
  cmp  ebx,[esi+physical_mem_block.start]
  jz   .endloop
  inc  eax
  add  esi,physical_mem_block.sizeof
  cmp  eax,[num_physical_mem_blocks]
  jl   .loop
  jmp  .ret
.endloop:
  mov  dword [esi+physical_mem_block.flags],0
  test eax,eax
  jz   .no_union_previous
  cmp  dword [esi-physical_mem_block.sizeof+physical_mem_block.flags],0
  jnz  .no_union_previous
  mov  ebx,[esi-physical_mem_block.sizeof+physical_mem_block.start]
  add  ebx,[esi-physical_mem_block.sizeof+physical_mem_block.size]
  cmp  ebx,[esi+physical_mem_block.start]
  jnz  .no_union_previous
  mov  ebx,[esi+physical_mem_block.size]
  add  [esi-physical_mem_block.sizeof+physical_mem_block.size],ebx
  call Delete_Block
  dec  eax
  dec  [num_physical_mem_blocks]
.no_union_previous:
  inc  eax
  cmp  eax,[num_physical_mem_blocks]
  jge  .no_union_next
  cmp  dword [esi+physical_mem_block.sizeof+physical_mem_block.flags],0
  jnz  .no_union_next
  mov  ebx,[esi+physical_mem_block.start]
  add  ebx,[esi+physical_mem_block.size]
  cmp  ebx,[esi+physical_mem_block.sizeof+physical_mem_block.start]
  jnz  .no_union_next
  mov  ebx,[esi+physical_mem_block.sizeof+physical_mem_block.size]
  add  [esi+physical_mem_block.size],ebx
  call Delete_Block
  dec  [num_physical_mem_blocks]
.no_union_next:
.ret:
  popad
  ret
  
sys_allocate_physical_block:
;eax - subfunction number
  mov  eax,ebx
  call Allocate_Physical_Block
  mov  [esp+36],eax
  ret
sys_free_physical_block:
;eax - subfunction number
  mov  eax,ebx
  call Free_Physical_Block
  ret
sys_set_buffer:
  add  ecx,std_application_base_address
isys_set_buffer:                 ;for using in kernel
;eax - subfunction number
;ebx - physical address
;ecx - buffer start
;edx - buffer size
  lea  edi,[ebx+second_base_address]
  mov  esi,ecx
  mov  ecx,edx
  cld
  rep  movsb
  ret
sys_get_buffer:
  add  ecx,std_application_base_address
isys_get_buffer:                 ;for using in kernel
;eax - subfunction number
;ebx - physical address
;ecx - buffer start
;edx - buffer size
  mov  edi,ecx
  lea  esi,[ebx+second_base_address]
  mov  ecx,edx
  cld
  rep  movsb
  ret
sys_internal_services:
  cmp  eax,4
  jle  sys_sheduler
  cmp  eax,5
  jz   sys_allocate_physical_block
  cmp  eax,6
  jz   sys_free_physical_block
  cmp  eax,7
  jz   sys_set_buffer
  cmp  eax,8
  jz   sys_get_buffer
  ret