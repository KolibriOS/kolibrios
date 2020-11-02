;/***

MenuetHeapInit = @@Kolibri@_HeapInit$qpvt1t1

MenuetHeapAlloc = @@Kolibri@Alloc$qui

MenuetHeapReAlloc = @@Kolibri@ReAlloc$qpvui

MenuetHeapFree = @@Kolibri@Free$qpv

MenuetHeapFreeAndThreadFinish = @Kolibri@_FreeAndThreadFinish$qpvpi

proc @Kolibri@_SetUseMemory$qui
  push ebx
  mov  eax,64
  mov  ebx,1
  mov  ecx,[esp+8]
  int  0x40
  pop  ebx
  test eax,eax
  jnz  .set_use_memory_nomem
  push ecx
  push dword [@Kolibri@_MenuetMemBlock]
  call @@MemoryHeap@ResizeBlock$q20MemoryHeap@TMemBlockpv
  add  esp,8
  mov  al,1
  ret
.set_use_memory_nomem:
  xor  al,al
  ret
endp

proc @Kolibri@_RecalculateUseMemory$qui
  mov  eax,dword [esp+4]
  mov  ecx,(U_END + 3) and not 3
  cmp  eax,ecx
  jna  .recalculate_use_memory_min
  push ebx
  sub  eax,ecx
  mov  ebx,6
  mul  ebx
  dec  ebx
  div  ebx
  add  eax,((U_END + 3) and not 3) + 3
  and  eax,not 3
  pop  ebx
  ret
.recalculate_use_memory_min:
  mov  eax,ecx
  ret
endp

proc @Kolibri@_FreeAndThreadFinish$qpvpi
  mov  ebx,1
  mov  ecx,[esp+8]
  jmp  .heap_free_tf_wait
.heap_free_tf_wait_loop:
  mov  eax,5
  int  0x40
  shl  ebx,1
  cmp  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jna  .heap_free_tf_wait
  mov  ebx,MENUET_MUTEX_MAX_TIME_WAIT
.heap_free_tf_wait:
  cmp  dword [ecx],0
  jnz  @Kolibri@ExitProcess$qv
  lock bts dword [@Kolibri@_MemHeapMutex],0
  jc   .heap_free_tf_wait_loop
  push dword [esp+4]
  push @Kolibri@_MenuetFreeSpace
  call @@MemoryHeap@Free$qr21MemoryHeap@TFreeSpacepv
  add  esp,8
  mov  byte [@Kolibri@_MemHeapMutex],0x40
  or   eax,-1
  int  0x40
endp

macro call func
{
  if func eq @MemoryHeap@_FirstNotZeroBit$qui
    bsf  eax,[esp]
  else if func eq @MemoryHeap@_CopyMemItemArray$quiuiui
    xchg edi,[esp]
    xchg esi,[esp+4]
    mov  ecx,[esp+8]
    cld
    sub  ecx,esi
    shr  ecx,2
    rep movs dword [edi],[esi]
    xchg edi,[esp]
    xchg esi,[esp+4]
  else
    call func
  end if
}

;/**/
