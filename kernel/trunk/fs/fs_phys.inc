hd_phys_read:
;eax - sector number
;ebx - destination
  pushad
  call  wait_for_hd_idle
  popad
  push  edx
  push  eax
  cli
  xor   eax,eax
  mov   edx,[hdbase]
  inc   edx
  out   dx,al
  inc   edx
  inc   eax
  out   dx,al
  inc   edx
;write sector number.  
  mov   eax,[esp]
  out   dx,al
  shr   eax,8
  inc   edx
  out   dx,al
  shr   eax,8
  inc   edx
  out   dx,al
  shr   eax,8
  inc   edx
  and   al,1+2+4+8
  add   al,byte [hdid] ;+0 or +16
  or    al,32+64+128
  out   dx,al
  inc   edx
  mov   al,0x20
  out   dx,al
  sti
  
  call  wait_for_sector_buffer
  cmp   [hd_error],0
  jnz   hd_read_error
  cli
  push  edi
  mov   edi,ebx
  mov   ecx,256
  mov   edx,[hdbase]
  cld
  rep   insw
  pop   edi
  sti
  pop   edx
  pop   eax
  ret
  
hd_phys_write:
;eax - sector number
;ebx - destination
  cmp   eax,[partition_start]
  jb    .ret
  cmp   eax,[partition_end]
  ja    .ret
  pushad
  call  wait_for_hd_idle
  popad
  push  edx
  push  eax
  cli
  xor   eax,eax
  mov   edx,[hdbase]
  inc   edx
  out   dx,al
  inc   edx
  inc   eax
  out   dx,al
;write sector number  
  inc   edx
  mov   eax,[esp]
  out   dx,al
  shr   eax,8
  inc   edx
  out   dx,al
  shr   eax,8
  inc   edx
  out   dx,al
  shr   eax,8
  inc   edx
  and   al,1+2+4+8
  add   al,byte [hdid] ;+0 or +16
  or    al,32+64+128
  out   dx,al
  
  inc   edx
  mov   al,0x30
  out   dx,al
  sti
  
  call  wait_for_sector_buffer
  cmp   [hd_error],0
  jnz   hd_write_error
  cli
  push  esi
  mov   esi,ebx
  mov   ecx,256
  mov   edx,[hdbase]
  cld
  rep   outsw
  pop   esi
  sti
  pop   edx
  pop   eax
.ret:  
  ret