;/***

MenuetHeapInit = @@Menuet@_HeapInit$qpvt1t1

MenuetHeapAlloc = @@Menuet@Alloc$qui

MenuetHeapReAlloc = @@Menuet@ReAlloc$qpvui

MenuetHeapFree = @@Menuet@Free$qpv

MenuetHeapFreeAndThreadFinish = @Menuet@_FreeAndThreadFinish$qpvpi

define @Menuet@_SetUseMemory$qui
  push ebx
  mov  eax,64
  mov  ebx,1
  mov  ecx,[esp+8]
  int  0x40
  pop  ebx
  test eax,eax
  jnz  Menuet_set_use_memory_nomem
  push ecx
  push dword [@Menuet@_MenuetMemBlock]
  call @@MemoryHeap@ResizeBlock$q20MemoryHeap@TMemBlockpv
  add  esp,8
  mov  al,1
  ret
Menuet_set_use_memory_nomem:
  xor  al,al
  ret
enddef

define @Menuet@_RecalculateUseMemory$qui
  mov  eax,dword [esp+4]
  mov  ecx,(U_END + 3) and not 3
  cmp  eax,ecx
  jna  Menuet_recalculate_use_memory_min
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
Menuet_recalculate_use_memory_min:
  mov  eax,ecx
  ret
enddef

define @Menuet@_FreeAndThreadFinish$qpvpi
  mov  ebx,1
  mov  ecx,[esp+8]
  jmp  Menuet_heap_free_tf_wait
Menuet_heap_free_tf_wait_loop:
  mov  eax,5
  int  0x40
  shl  ebx,1
  cmp  ebx,MENUET_MUTEX_MAX_TIME_WAIT
  jna  Menuet_heap_free_tf_wait
  mov  ebx,MENUET_MUTEX_MAX_TIME_WAIT
Menuet_heap_free_tf_wait:
  cmp  dword [ecx],0
  jnz  @Menuet@ExitProcess$qv
  lock bts dword [@Menuet@_MemHeapMutex],0
  jc   Menuet_heap_free_tf_wait_loop
  push dword [esp+4]
  push @Menuet@_MenuetFreeSpace
  call @@MemoryHeap@Free$qr21MemoryHeap@TFreeSpacepv
  add  esp,8
  mov  byte [@Menuet@_MemHeapMutex],0x40
  or   eax,-1
  int  0x40
enddef

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
