;/***

KolibriHeapInit = @@Kolibri@_HeapInit$qv

KolibriHeapAlloc = @@Kolibri@Alloc$qui

KolibriHeapReAlloc = @@Kolibri@ReAlloc$qpvui

KolibriHeapFree = @@Kolibri@Free$qpv

proc @MemoryHeap@mem_Init$qv uses ebx
  mov  eax,SF_SYS_MISC
  mov  ebx,SSF_HEAP_INIT
  int  0x40
  ret
endp

proc @MemoryHeap@mem_Alloc$qul uses ebx
  mov  eax,SF_SYS_MISC
  mov  ebx,SSF_MEM_ALLOC
  int  0x40
  ret
endp

proc @MemoryHeap@mem_ReAlloc$qulpv uses ebx
  mov  eax,SF_SYS_MISC
  mov  ebx,SSF_MEM_REALLOC
  mov  ecx,[esp+8]
  mov  edx,[esp+12]
  int  0x40
  ret
endp

proc @MemoryHeap@mem_Free$qpv uses ebx
  mov  eax,SF_SYS_MISC
  mov  ebx,SSF_MEM_FREE
  int  0x40
  ret
endp

;/**/
